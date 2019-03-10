"""
Main reconstruction functionality
"""

import gc
import enum
from io import StringIO
import pandas
import numpy as np
import matplotlib.pyplot as plt

import clib.HL888 as HL38

from . import Bootstrap
from . import MixingSpace
from . import SciConstants
from .Util import *

class ReconType(enum.Enum):
    """
    Enum to select which reconstruction algorithm we want
    """
    MCMC = 1
    Matrix = 2

class EndmemberType(enum.Enum):
    """
    Enum to select which style of endmember estimation we require
    """
    Dual = 1
    Continuous = 2
    ModernOnly = 3
    Exponential = 4
    FuturePast = 5
    Bootstrap = 6

class EndmemberConfig(enum.Enum):
    """
    Enum to select the script for endmember configuration
    """
    MF = 1
    KMF = 2
    QUARTUS = 3
    QUINTUS = 5

class HL38Config:
    """
    Reconstruction configuration object.
    Can be passed to the C++ engine to initialise a ReconstructionManager.
    """
    def __init__(self, endmembers, recon, end_config, bootWidth):
        self.ageBinWidth = None
        self.kernelWidth = None
        self.bootstrapWidth = bootWidth
        self.endmembers = endmembers
        self.recon = recon
        self.endScript = end_config
        self.detailedRatioPrint = []
        self.filterList = []
        self.reconSystems = []
        self.DB = None
        self.DB0 = None
        self.DBstr = None

    def CountReconSystems(self):
        """
        Return the number of ratios used by the current configuration.
        """
        return len(self.reconSystems)

    def EndmemberWidths(self, agebinW = None, kernelW = None):
        """
        Configure the various kernels required by certain endmember types
        """
        self.ageBinWidth = agebinW
        self.kernelWidth = kernelW
        return self

    def UseDetailedRatioPrinter(self, rList):
        """
        Print detailed confidence interval statistics for the ratios
        given in 'rList.'
        """
        self.detailedRatioPrint = rList
        return self

    def FilterResult(self, filterResult):
        """
        Set the FilterResult object which will be used as a input
        for the sedimentary database in the reconstruction.
        """
        self.Database(filterResult.dbShale)
        self.AppliedFilter(filterResult.filterApplied)
        self.DB0 = filterResult.dbShale0
        return self

    def AppliedFilter(self, filterList):
        """
        Set what elements were used to filter the shale database.
        """
        self.filterList = filterList
        return self

    def ReconSystems(self, reconSystems):
        """
        Set which proxy ratios will be used in the reconstruction.
        """
        self.reconSystems = reconSystems
        return self

    def Database(self, DB):
        """
        Set the raw sedimentary database to be used in the reconstruction.
        """
        self.DB = DB
        self.DB0 = DB
        #If no filename is provided (such as here), this returns a memory-mapped file
        self.DBstr = DB.to_csv(na_rep='x') 
        return self

    def ToFilename(self, prefix):
        """
        Convenience function for generating systematic filenames.
        """
        config = self.ToDict()
        f = "V8"+prefix+"_RECON_"+MultiCompose(self.reconSystems)
        f += "_bootWRB"+config["BootstrapKernelWidth"]+"Myr_"
        f += config["endmemberMode"]+"_"+config["reconMode"]+"_"+config["endmemberScript"]
        if self.filterList:
            f += "_FILTER_"+self.filterList
        if self.ageBinWidth is not None:
            f += "_AGEBIN_"+str(self.ageBinWidth)
        if self.kernelWidth is not None:
            f += "_KWIDTH_"+str(self.kernelWidth)
        return f

    def ToDict(self):
        """
        Create the parameter dict that can be passed to the C++ reconstruction engine.
        """
        confdict= {"rA": [], "rB": [], "r":[]}
        mapEndmemberMode = {EndmemberType.Dual : "Dual",
                            EndmemberType.Continuous : "Continuous",
                            EndmemberType.ModernOnly : "ModernOnly",
                            EndmemberType.Exponential : "Exponential",
                            EndmemberType.FuturePast : "FuturePast",
                            EndmemberType.Bootstrap : "Bootstrap"}
        mapReconMode = {ReconType.MCMC : "MCMC",
                        ReconType.Matrix : "Matrix"}
        mapEndmemberScript = {EndmemberConfig.MF : "MF",
                              EndmemberConfig.KMF : "KMF",
                              EndmemberConfig.QUARTUS : "QUARTUS",
                              EndmemberConfig.QUINTUS : "QUINTUS"}
        
        confdict["endmemberMode"] = mapEndmemberMode[self.endmembers]
        confdict["reconMode"] = mapReconMode[self.recon]
        confdict["endmemberScript"] = mapEndmemberScript[self.endScript]
        confdict["BootstrapKernelWidth"]= str(self.bootstrapWidth)
        if self.ageBinWidth is not None:
            confdict["AgeBinWidth"]= str(self.ageBinWidth)
        if self.kernelWidth is not None:
            confdict["KernelWidth"] = str(self.kernelWidth)
        if self.reconSystems is not None:
            for r in self.reconSystems:
                [A, B] = DecomposeR(r)
                confdict["rA"].append(A)
                confdict["rB"].append(B)
                confdict["r"].append(r)
        if self.detailedRatioPrint:
            confdict["detailedRatioPrinter"] = []
            for r in self.detailedRatioPrint:
                confdict["detailedRatioPrinter"].append(r)
        return confdict

    def InitReconManager(self, prepareBootstraps = True):
        """
        Set up the C++ reconstruction manager object, caching bootstrap results for future use.
        Return None if the shale database does not contain sufficient data.
        """
        RM = HL38.ReconManager(self.ToDict(), self.DBstr)
        if prepareBootstraps:
            for r in self.reconSystems:
                if not Bootstrap.IsCached(r):
                    [A, B] = DecomposeR(r)
                    if RM.DataCountForBootstrap(A, B) < 10:
                        print("INSUFFICIENT DATA TO CONSTRUCT ALL BOOTSTRAPS")
                RM.AddBootstrap(Bootstrap.GetCached(r, RM))
        return RM

class Visualiser:
    """
    Holds common state for visualising reconstructions.
    Must be initialised with a HL38Config object.
    """
    def __init__(self, config, plotVector= False):
        self.config = config
        self.plotVector = plotVector
        self.RM= self.config.InitReconManager()

    def TimelineRecon(self):
        """
        Run the C++ reconstruction procedure for the entirety of Earth history, save csv file, generate figure with error bars
        """
        recon = self.RM.RunReconstruction()
        endNames = [self.RM.GetEndmemberName(idx) for idx in range(self.RM.GetEndmemberCount())]
        #Generate the filename
        fname= self.config.ToFilename("b")
        #Store the result as a csv file
        wfile= open("cout/"+fname+".csv", "w")
        wfile.write(recon)
        #Create a figure from the result file
        dbRecon= pandas.read_csv(StringIO(recon))
        #Stacked K, M, F endmember percentages through time
        #Include error bars!
        endColours = ['#91FFBAFF', '#FFE391FF', '#A3C5F7FF']
        FSIZE = 16
        plt.rc('font', size=FSIZE)
        plt.rc('axes', titlesize=FSIZE)
        plt.rc('axes', labelsize=FSIZE)
        plt.rc('xtick', labelsize=FSIZE)
        plt.rc('ytick', labelsize=FSIZE) 
        plt.rc('legend', fontsize=FSIZE)
        #plt.title(fname)
        if len(self.config.DB.index) > 0:
            t= dbRecon["TIME(/MYR)"]
            acc = np.zeros_like(t)
            #Plot best fits
            for eM_name, eM_colour in zip(endNames, endColours):
                eM_dat = dbRecon[eM_name]
                plt.fill_between(t, acc, acc + eM_dat, label=eM_name, color=eM_colour)
                acc = acc + eM_dat
            #Plot SiO2
            plt.plot(dbRecon["TIME(/MYR)"], dbRecon["SiO2"], 'k--', label='SiO_2')
            #Plot confidence intervals (skip last error since everything sums to 100%)
            acc = np.zeros_like(t)
            for eM_name in endNames[:-1]:
                eM_dat_p025 = dbRecon["ERR_"+eM_name+"025"]
                eM_dat_p975 = dbRecon["ERR_"+eM_name+"975"]
                plt.fill_between(t, acc + eM_dat_p025, acc + eM_dat_p975, color='#00538728')
                acc = acc + dbRecon[eM_name]
        plt.ylim(0, 100)
        plt.xlabel("Time (/Ma)")
        plt.xlim(0, 3500)
        plt.ylabel("Endmember Proportion, %")
        #plt.legend()
        plt.gca().invert_xaxis()
        #Write the figure
        self.WritePlot(fname, 8, 6)
        return self

    def CompareArchaModern(self):
        """
        Create a figure comparing the Archaean and Modern MCMC states
        """
        if self.config.CountReconSystems() < 2:
            return self
        #Generate filename
        fname= self.config.ToFilename("c")
        plt.suptitle(fname)
        #Subfigure: Archaean
        plt.subplot(121)
        self.SubplotMCMCStates(self.RM.GetNearestValidTime(3500.0, False), False, True)
        #Subfigure: Modern
        plt.subplot(122)
        self.SubplotMCMCStates(self.RM.GetNearestValidTime(0.0, True), True, False)
        #Write the figure
        self.WritePlot(fname, 20, 8)
        return self

    def PlotBootstraps(self):
        """
        Create a figure showing the first two bootstraps
        """
        if self.config.CountReconSystems() < 2:
            return self
        subplots = [121, 122]
        indices = [x for x in range(len(subplots))]

        #Get (or generate) the bootstraps we will plot
        #Then, convert them to python-legible arrays
        pX = []
        pbest = []
        perr = []
        for idx in indices:
            b = Bootstrap.GetCached(self.config.reconSystems[idx], self.RM)
            pX.append(np.array(b.bestFit.GenerateXList()))
            pbest.append(np.array(b.bestFit.GenerateYList()))
            perr.append(np.array(b.stdError.GenerateYList()))
    
        #Generate the filename
        fname= self.config.ToFilename("d")

        #Init figure
        dbShale0 = self.config.DB0
        dbShale= self.config.DB
        plt.suptitle(fname)

        #Plot both bootstraps
        for idx, subp in zip(indices, subplots):
            plt.subplot(subp)
            [rA, rB] = DecomposeR(self.config.reconSystems[idx])
            temp_limit= dbShale0[rA]/dbShale0[rB]
            temp_limit= temp_limit[np.isfinite(temp_limit)]
            limit_top= np.nanpercentile(temp_limit, 95)
            plt.plot(dbShale0["Age"], dbShale0[rA]/dbShale0[rB], '.', color='lightgrey')
            plt.plot(dbShale["Age"], dbShale[rA]/dbShale[rB], '.',color='blueviolet')
            if len(dbShale.index) > 0:
                plt.plot(pX[idx], pbest[idx], linewidth=3.0, color='black')
                plt.plot(pX[idx], pbest[idx]+2*perr[idx],'--', color='red')
                plt.plot(pX[idx], pbest[idx]-2*perr[idx],'--', color='red')
            plt.xlabel("Time (/Ma)")
            plt.ylabel(self.config.reconSystems[idx])
            plt.ylim(0,limit_top)
            plt.gca().invert_xaxis()

        #Write the figure
        self.WritePlot(fname, 20, 8)
        return self

    def WritePlot(self, fname, size_x= 10, size_y= 8):
        """
        Plot a figure, setting the size, using the correct extension, and handling memory leaks
        """
        #Figure out extension
        if self.plotVector:
            plotExt = "svg"
        else:
            plotExt = "png"

        #Write the figure
        plt.gcf().set_size_inches(size_x, size_y)
        plt.savefig("cout/"+fname+"."+plotExt)

        #Fix memory leaks
        plt.close(plt.gcf())
        gc.collect()

    def SubplotMCMCStates(self, t, plotModern, plotArcha):
        """
        Draw one set of MCMC states using the stateful pyplot interface
        """
        #Run the C++ engine
        aS = HL38.MCMC_SingleTimeTest_WRB(self.RM, t)
        endNames = [self.RM.GetEndmemberName(idx) for idx in range(self.RM.GetEndmemberCount())]
        plt.gca().set_facecolor('k')
        X = np.array(aS[0].mcmcR)
        Y = np.array(aS[1].mcmcR)
        mask = np.bitwise_and(np.isfinite(X), np.isfinite(Y))
        X = X[mask]
        Y = Y[mask]
        plt.hist2d(X, Y, bins=80, cmin=1)
        MixingSpace.Plot(plt, aS[0].cA, aS[0].cB, aS[1].cA, aS[1].cB, endNames)

        plt.plot(aS[0].bestR, aS[1].bestR, "+", label="BEST") #Best-fitting reconstruction
        plt.plot(aS[0].bootR, aS[1].bootR, "x", label="BOOT") #Modern-day value given by bootstrap
        #Plot modern estimates (if requested)
        if plotModern:
            if self.config.endScript == EndmemberConfig.KMF:
                bestR0 = (SciConstants.bestfitK*aS[0].cA[0]
                          + SciConstants.bestfitM*aS[0].cA[1]
                          + SciConstants.bestfitF*aS[0].cA[2]) / (SciConstants.bestfitK*aS[0].cB[0]
                                                                  + SciConstants.bestfitM*aS[0].cB[1]
                                                                  + SciConstants.bestfitF*aS[0].cB[2])
                bestR1 = (SciConstants.bestfitK*aS[1].cA[0]
                          + SciConstants.bestfitM*aS[1].cA[1]
                          + SciConstants.bestfitF*aS[1].cA[2]) / (SciConstants.bestfitK*aS[1].cB[0]
                                                                  + SciConstants.bestfitM*aS[1].cB[1]
                                                                  + SciConstants.bestfitF*aS[1].cB[2])
                plt.plot(bestR0, bestR1, "*", label="TiIso") #Modern-day value given by forward model, using KMF proportions from the Science paper
            [r1A, r1B] = DecomposeR(self.config.reconSystems[0])
            [r2A, r2B] = DecomposeR(self.config.reconSystems[1])
            TM_R0 = SciConstants.TaylorMcLennan[r1A] / SciConstants.TaylorMcLennan[r1B]
            TM_R1 = SciConstants.TaylorMcLennan[r2A] / SciConstants.TaylorMcLennan[r2B]
            RG_R0 =     SciConstants.RudnickGao[r1A] /     SciConstants.RudnickGao[r1B]
            RG_R1 =     SciConstants.RudnickGao[r2A] /     SciConstants.RudnickGao[r2B]
            plt.plot(TM_R0, TM_R1, "8", label="T&M'85") #Taylor & McLennan estimate for modern-day upper continental crust
            plt.plot(RG_R0, RG_R1, "8", label="R&G'03") #Rudnick & Gao estimate for modern-day upper continental crust
        #Plot Archaean estimates (if requested)
        if plotArcha:
            if self.config.endScript == EndmemberConfig.KMF:
                bestR0 = (SciConstants.bestfitKArch*aS[0].cA[0]
                          + SciConstants.bestfitMArch*aS[0].cA[1]
                          + SciConstants.bestfitFArch*aS[0].cA[2]) / (SciConstants.bestfitKArch*aS[0].cB[0]
                                                                      + SciConstants.bestfitMArch*aS[0].cB[1]
                                                                      + SciConstants.bestfitFArch*aS[0].cB[2])
                bestR1 = (SciConstants.bestfitKArch*aS[1].cA[0]
                          + SciConstants.bestfitMArch*aS[1].cA[1]
                          + SciConstants.bestfitFArch*aS[1].cA[2]) / (SciConstants.bestfitKArch*aS[1].cB[0]
                                                                      + SciConstants.bestfitMArch*aS[1].cB[1]
                                                                      + SciConstants.bestfitFArch*aS[1].cB[2])
                plt.plot(bestR0, bestR1, "*", label="TiIso")
        plt.xlabel(self.config.reconSystems[0])
        plt.ylabel(self.config.reconSystems[1])
        plt.legend()

    def Show3DMixSpace(self, t):
        """
        Use mayavi to display a 3D mixing space of the current reconstruction.
        Only works if there are four endmembers.
        """
        aS = HL38.MCMC_SingleTimeTest_WRB(self.RM, t)
        endNames = [self.RM.GetEndmemberName(idx) for idx in range(self.RM.GetEndmemberCount())]

        MixingSpace.Show3D(aS, endNames, self.config.reconSystems) 
