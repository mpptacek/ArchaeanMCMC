"""
An applet to visualise the effects of LOF filtering on the shale database in 2D
Code follows after FilterLOF.Apply, with the exception of the end of the function
"""

from matplotlib import pyplot as plt
import numpy as np

from Croc import *

TAKE_LOGS = True

if __name__ == "__main__":
    HL38.LoadModule("CommonDBs")
    while True:
        print("VisualiseLOF: Enter ratios to filter by...")
        l = InputRatioList(Database.IsRatioGood)

        Axes = l
        #unfilteredDB= Database.LoadRandomSubset(Database.LoadHL38("keller"), 0.1)
        unfilteredDB = Database.ReadFile("all")
        alpha = 0.05
    
        #Reconstruct element names from ratio names
        el_list= []
        for r in Axes:
            el_list.extend(DecomposeR(r))

        #Get matrices for the tested and training datasets
        N= 50
        print("VisualiseLOF: Normalise & take logarithms")
        db_tested = unfilteredDB
        db_train = Database.LoadHL38("keller_unfiltered")
        dTest  = FilterLOF.LogRatioMatrix(db_tested,el_list, stripNaNs=False)
        dTrain = FilterLOF.LogRatioMatrix(db_train, el_list, stripNaNs=True)

        #Compute extents of training data for each dimension, renormalise all data
        dTrain, midpts, extents = FilterLOF.Renormalise(dTrain)
        dTest = FilterLOF.Renormalise(dTest, midpts, extents)[0]
 
        #Fill in missing data
        print("VisualiseLOF: Interpolate missing values")
        dTest= FilterLOF.FillNaNs(dTrain, dTest, N)

        print("VisualiseLOF: Compute testing set factors")
        LOF_test= FilterLOF.ComputeLOF(dTest, dTrain, N)
        #print("VisualiseLOF: Compute training set factors")
        #LOF_ref= FilterLOF.ComputeLOF2(dTrain, N)

        #Prepare the rejections filter
        lofFilter= np.full(dTest.shape[0],True,dtype=np.bool)

        #Find cutoff LOF value for the training set
        #c = np.percentile(LOF_ref,100*(1-alpha))
        #passing_samples= (LOF_test < c)
        #lofFilter= (lofFilter & passing_samples)
        #print("VisualiseLOF: Evaluate with c="+str(c))

        #Apply filter
        #filteredShales = unfilteredDB[lofFilter]

        #Compute coordinates
        [elXa, elXb] = DecomposeR(Axes[0])
        [elYa, elYb] = DecomposeR(Axes[1])
        unfilteredX = db_tested[elXa] / db_tested[elXb]
        unfilteredY = db_tested[elYa] / db_tested[elYb]
        bgX = db_train[elXa] / db_train[elXb]
        bgY = db_train[elYa] / db_train[elYb]

        if TAKE_LOGS:
            unfilteredX = np.log(unfilteredX)
            unfilteredY = np.log(unfilteredY)
            bgX = np.log(bgX)
            bgY = np.log(bgY)

        #Use filter to generate colours
        #unfilteredClr = np.zeros_like(unfilteredX)
        #unfilteredClr[lofFilter] = 1.0
        #unfilteredClr = np.log(LOF_test)
        unfilteredClr = LOF_test

        #Plot the figure!
        if TAKE_LOGS:
            plt.xlabel("ln("+Axes[0]+")")
            plt.ylabel("ln("+Axes[1]+")")
        else:
            plt.xlabel(Axes[0])
            plt.ylabel(Axes[1])

        plt.scatter(bgX, bgY, c ='grey')
        mappable = plt.scatter(unfilteredX, unfilteredY, c = unfilteredClr)

        plt.colorbar(mappable)
        #plt.plot(unfilteredX[ lofFilter], unfilteredY[ lofFilter], "b.")
        #plt.plot(unfilteredX[~lofFilter], unfilteredY[~lofFilter], ".", color='grey')
        plt.gcf().set_size_inches(12,10)
        plt.show()
        print("")
