"""
An applet to evaluate many potential candidates for mixing spaces and rank them according to error propagation characteristics
"""

import os
import itertools
import pickle
import datetime

import clib.HL888 as HL38

from Croc import *

PLOT_SPACES = False

def grouper(iterable, n, fillvalue=None):
    "Collect data into fixed-length chunks or blocks"
    # grouper('ABCDEFG', 3, 'x') --> ABC DEF Gxx"
    args = [iter(iterable)] * n
    return itertools.zip_longest(*args, fillvalue=fillvalue)

class MiniBootstrap:
    def __init__(self, archaError, modernError):
        self.a_err = archaError
        self.m_err = modernError
    def stdError(self, t):
        if t > 2500:
            return self.a_err
        else:
            return self.m_err

def PrepC(fR, sys_list):
    """
    Sets up the Python-side MCMC config
    """
    c = Recon.HL38Config(Recon.EndmemberType.Dual,
                         Recon.ReconType.MCMC,
                         Recon.EndmemberConfig.KMF, 500.0)
    c.ReconSystems(sys_list).FilterResult(fR)
    return c

def PrepRM(fR, sys_list):
    """
    Sets up the C++ MCMC config
    """
    return PrepC(fR, sys_list).InitReconManager(False)

def InverseModelCalc(RM, rVals, rErrors, t):
    """
    Calculate the best-fitting mixture for a given list of ratio values.
    For this, we invoke HL38's single-timestep MCMC functionality, but
    instead of generating the shale bootstraps for data, we load in our own
    (with the given values).
    """
    #Delete previous bootstraps from the RM
    RM.ResetAllBootstraps()
    #Load our own bootstraps into the RM
    for rVal, rErr in zip(rVals, rErrors):
        b = HL38.WRB_Result()
        b.bestFit.AddNewPoint(t - 50.0, rVal)
        b.bestFit.AddNewPoint(t, rVal)
        b.bestFit.AddNewPoint(t + 50.0, rVal)
        b.stdError.AddNewPoint(t - 50.0, rErr)
        b.stdError.AddNewPoint(t, rErr)
        b.stdError.AddNewPoint(t + 50.0, rErr)
        b.bestFit.Finalise()
        b.stdError.Finalise()
        RM.AddBootstrap(b)

    #Run the single time-step reconstruction
    return HL38.MCMC_SingleTimeTest_WRB(RM, t)

def EvalMixSpace(fR, b_cache, ratio_list):
    """
    Evaluates the quality of a given mixing space, returns a rating string
    """
    if ratio_list is None:
        return ""
    #Define our target points as the proportions from the Science paper,
    #and also Tang et al's proportions.
    #Endmember type: DUAL, Endmember script: KMF
    CENTRAL_MIX = [[[0.15, 0.27, 0.58], [0.15, 0.70, 0.15]], #Archaean proportions
                   [[0.0, 0.28, 0.72]]]                      #Modern proportions

    #Prepare results string
    buffer = ""
    for ratio in ratio_list:
        buffer += ratio
        if ratio != ratio_list[-1]:
            buffer += "-"
    buffer += ","

    #Load cached bootstraps into a separate list, outside the RM
    bList = []
    for rName in ratio_list:
        bList.append(b_cache[rName])

    #Run for both Archaean and modern time periods
    HL38.LoadModule("CommonDBs")
    RM = PrepRM(fR, ratio_list)
    for t,targetList in zip([3500.0, 0.0],
                            CENTRAL_MIX):
        #Load error value
        errVals = []
        for bootstrap in bList:
            errVals.append(bootstrap.stdError(t))

        bestList = []
        varsList = []

        #Scan error points
        for targetPoint in targetList:
            #Calculate values of central point
            cPoint = list(RM.ForwardModelCalc(t, targetPoint))

            #Run the MCMC reconstruction at this central point
            result = InverseModelCalc(RM, cPoint, errVals, t)

            #Get the endmember mistfit & variances
            bestList.append([])
            varsList.append([])
            for i in range(3):
                bestList[-1].append(result.best[i] - targetPoint[i])
                varsList[-1].append(result.endmemberP975[i] - result.endmemberP025[i])

        #Find maximum misfit
        misfitMax = bestList[0]
        errorMax = varsList[0]
        for misfit, error in zip(bestList, varsList):
            for i in range(3):
                misfitMax[i] = max(misfit[i], misfitMax[i])
                errorMax[i] = max(error[i], errorMax[i])

        #Log results
        for misfit in misfitMax:
            buffer += str(misfit)
            buffer += ","
        for error in errorMax:
            buffer += str(error)
            buffer += ","

    #Plot mixing spaces too, if asked for
    if PLOT_SPACES:
        Recon.Visualiser(PrepC()).CompareArchaModern()

    #Return
    return buffer+"\n"

#Evaluate all mixing spaces
if __name__ == '__main__':
    N = 2
    REPEAT_ELEMENTS = False
    PROCESSOR_COUNT = 2

    #Define suitable ratios
    good_elements = SciConstants.ReconElements()

    #Load shales database
    filterResult = Database.LoadFilteredLOF()
             
    #Find all mixing spaces to check
    if REPEAT_ELEMENTS:
        rList = Rombinatorics.Gen_ElementRepeats(good_elements, N)
    else:
        rList = Rombinatorics.Gen_NoRepeats(good_elements, N)
    nTotal = len(rList)
    
    #If there already is a bootcache file, load from that first
    bcache_file = "scan_boot_cache.xsb"
    if os.access(bcache_file, os.R_OK):
        bfile = open(bcache_file, "rb")
        bootcache = pickle.load(bfile)
        print("LOADED " + str(len(bootcache)) + " BOOTSTRAPS FROM FILE.")
        bfile.close()
    else:
        bootcache = {}
    #Find any remaining ratios to bootstrap and run them
    r_set = set()
    for system in rList:
        for ratio in system:
            r_set.add(ratio)
    for ratio in bootcache:
        if ratio in r_set:
            r_set.remove(ratio)
    cRM = PrepRM(filterResult, ["Ni/Co"])
    cSize = len(r_set)
    if cSize > 0:
        print("NOW CACHING "+str(cSize)+" BOOTSTRAPS...")
        for ratio, it in zip(list(r_set), range(cSize)):
            print("T: "+str(datetime.datetime.now()))
            print("BOOTSTRAP "+str(ratio)+": "+str(it)+"/"+str(cSize)+" ("+str(100.0 * float(it)/float(cSize))+"%)")
            [A, B] = DecomposeR(ratio)
            boot = cRM.GenerateBootstrap(A, B)
            bootcache[ratio] = MiniBootstrap(boot.stdError.LastY(), boot.stdError.FirstY()) #Load mini python bootstraps instead of full-sized C++ ones, so we can easily pass them to other processes afterwards!
        wbfile = open(bcache_file, "wb")
        pickle.dump(bootcache, wbfile)
        wbfile.close()
    print("BOOTCACHE READY, MAIN LOOP INITIATE")

    #Create log file name
    if REPEAT_ELEMENTS:
        mode_string = "repeatEl"
    else:
        mode_string = "norepeat"
    fname = "mixSpacesMCMCR_v5_"+mode_string+"_N"+str(N)+".csv"

    #Open (or create) the log file
    if os.access(fname, os.W_OK):
        sio2Log = open(fname, "a")
    else:
        sio2Log = open(fname, "w")
        sio2Log.writelines(["System,archK,archM,archF,archK_2SE,archM_2SE,archF_2SE,mdrn_K,mdrn_M,mdrn_F,mdrnK_2SE,mdrnM_2SE,mdrnF_2SE\n"])
        sio2Log.flush()
        os.fsync(sio2Log)

    #Scan everything
    if PROCESSOR_COUNT == 1:
        #Laptop version
        for system,it in zip(rList,range(nTotal)):
            print("T: "+str(datetime.datetime.now()))
            print("SYS["+str(N)+"] "+str(it)+"/"+str(nTotal)+" ("+str(100.0 * float(it)/float(nTotal))+"%)")
            bf = EvalMixSpace(filterResult, bootcache, system)
            sio2Log.writelines(bf)
            sio2Log.flush()
            os.fsync(sio2Log)
    else:
        #Cluster version
        CLUSTER_BATCH_SIZE = 100
        from multiprocessing import Pool
        pool = Pool(PROCESSOR_COUNT)
        system_iterator = iter(rList)
        batch_count = int(nTotal / (PROCESSOR_COUNT * CLUSTER_BATCH_SIZE)) + 1
        for sys_list, it in zip(grouper(system_iterator, PROCESSOR_COUNT * CLUSTER_BATCH_SIZE),
                                range(batch_count)):
            print("T: "+str(datetime.datetime.now()))
            print("BATCH "+str(it)+"/"+str(batch_count)+" ("+str(100.0 * float(it)/float(batch_count))+"%)")
            #print("B-CONT: "+str(sys_list))
            buffer_lines = pool.starmap(EvalMixSpace, zip(itertools.repeat(filterResult),
                                                          itertools.repeat(bootcache),
                                                          sys_list), CLUSTER_BATCH_SIZE)
            for bf in buffer_lines:
                sio2Log.writelines(bf)
            sio2Log.flush()
            os.fsync(sio2Log)
    
    #We've made it
    print("TASK COMPLETE.")
