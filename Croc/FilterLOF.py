"""
Implements the Local Outlier Filter algorithm.
"""
import numpy as np
from . import MetaStat
from .Util import *

_debugVisualise= False

def Renormalise(X, midpts=None, extents=None):
    """
    Normalises every dimension in data matrix to roughly [-1,1] (except extreme outliers!)
    """
    if (extents is None) or (midpts is None):
        extents = 2 * np.std(X, axis=0, keepdims=True)
        midpts = np.mean(X, axis=0, keepdims=True)
    return (X - midpts) / extents, midpts, extents

def FillNaNs(Xtrain, Xtest, M):
    """
    Fill in missing values with averages of M nearest neighbours
     1) Find the nearest training neighbour of each tested data point
     2) Find the average distance, in each dimension separately, from the nearest training neighbour
        to its N nearest training neighbours.
     3) Substitute the missing values with those local average distances, for missing dimensions only.
    """
    idx = MetaStat.NearestAB_N_NeighbourIdx_IgnoreNans(Xtrain, Xtest, M)
    meanAxisDist = np.average(Xtrain[idx], axis=1)
    return np.where(np.isnan(Xtest), meanAxisDist, Xtest)

def LogRatioMatrix(db, el_list, stripNaNs=False):
    """
    Create a stack of log ratios, given a list of elements and a database to pull samples from
    """

    #Stack all our elements in a single matrix
    el_stack = []
    for el in el_list:
        el_stack.append(db[el].values.astype(np.float))
    dstack = np.vstack(el_stack)

    #Compute ratios, take logs
    b= []
    i= 0
    while i+1 < dstack.shape[0]:
        #Avoid division by zero, AND zero values in the logs!
        b.append(np.divide(dstack[i], dstack[i+1], out=np.full_like(dstack[i], np.nan), where=np.bitwise_and(dstack[i+1] != 0, dstack[i] != 0)))
        i+= 2
    
    if stripNaNs:
        #Strip all observations with NaNs via masking
        return np.ma.compress_cols(np.ma.masked_invalid(np.log(b))).T 
    else:
        #Simply return the elementwise log
        return np.log(b).T

def ComputeLOF(dTest, dTrain, k):
    """
    Compute LOFs for all entries in matrix A by checking them against the densities of entries in B
    Return a length (n, ) numpy array where the nth value is the LOF of the nth point in A
    Note that this algorithm assumes there are no duplicate entries in any of the databases
    """

    #Find distances to all points in the training set, for the test and train set respectively
    print("  Compute distance matrix...")
    dist_AB = MetaStat.AB_NeighbourDistances_IgnoreNans(dTrain, dTest)  #dim: (n_test, n_trai)
    dist_BB = MetaStat.AB_NeighbourDistances_IgnoreNans(dTrain, dTrain) #dim: (n_trai, n_trai)
    
    #Find indices of k nearest neighbours (in dTrain) for every point in dTest & dTrain
    idxKNN_AB = dist_AB.argsort(axis=1)[:, 1:k+1]
    idxKNN_BB = dist_BB.argsort(axis=1)[:, 1:k+1]

    #Find distance to k-th nearest neighbour (in dTrain) for all points in dTrain
    distk_BB = MetaStat.DistanceToIdx(dTrain, dTrain, idxKNN_BB[:, k-1]) #dim: (n_trai, 1)

    #Replace distance with local reachability distance
    #To accomplish this, look at the distance to the kth neighbour of SECOND point, not the first!
    print("  Evaluate local reachability distance...")
    dist_AB = np.where(dist_AB.T > distk_BB, dist_AB.T, distk_BB).T  
    dist_BB = np.where(dist_BB.T > distk_BB, dist_BB.T, distk_BB).T  
    
    #Compute the local reachability density (LRD) for every training point
    #To do this, first add up reachability distances for all k-nearest training neighbours,
    #and then reciprocate and normalise
    print("  Prepare LRD tables...")
    lrd = np.zeros((dTrain.shape[0], 1), np.float32)
    for i in range(k):
        idx = idxKNN_BB[:, i]
        lrd += dist_BB[np.arange(0, dTrain.shape[0]), idx].reshape(dTrain.shape[0], 1)
    np.reciprocal(lrd, lrd) 
    lrd *= float(k)

    #Compute the sum of the k-nearest neigbours' LRDs for every tested point
    sumLRD = np.zeros((dTest.shape[0], 1), np.float32)
    for i in range(k):
        #"For every tested point, look at the index of the ith nearest neighbour, get its LRD,
        #then add it into the accumulator."
        sumLRD += lrd[idxKNN_AB[:, i]]

    #Compute LOF for every tested point
    print("  Estimate LOF...")
    LOF = np.zeros((dTest.shape[0], 1))
    for i in range(k):
        idx = idxKNN_AB[:, i]
        LOF += dist_AB[np.arange(0, dTest.shape[0]), idx].reshape(dTest.shape[0], 1)
    LOF *= sumLRD / (k * k)
    #Return LOFs array
    return LOF.reshape(dTest.shape[0])

def ComputeLOF2(dTrain, k):
    """
    Same thing as ComputeLOF, but for training data ONLY
    """
    print("  Compute distance matrix...")
    dist = MetaStat.AB_NeighbourDistances_IgnoreNans(dTrain, dTrain)
    idxKNN = dist.argsort(axis=1)[:, 1:k+1]
    distk = MetaStat.DistanceToIdx(dTrain, dTrain, idxKNN[:, k-1])
    print("  Evaluate local reachability distance...")
    dist = np.where(dist > distk, dist, distk)
    print("  Prepare LRD tables...") 
    lrd= np.zeros((dTrain.shape[0], 1), np.float32)
    for i in range(k):
        idx = idxKNN[:, i]
        lrd += dist[np.arange(0, dTrain.shape[0]), idx].reshape(dTrain.shape[0], 1)
    np.reciprocal(lrd, lrd) 
    lrd *= float(k)
    sumLRD= np.zeros((dTrain.shape[0], 1), np.float32)
    for i in range(k):
        sumLRD+= lrd[idxKNN[:, i]]
    print("  Estimate LOF...")
    LOF = np.zeros((dTrain.shape[0], 1))
    for i in range(k):
        idx = idxKNN[:, i]
        LOF += dist[np.arange(0, dTrain.shape[0]), idx].reshape(dTrain.shape[0], 1)
    LOF *= sumLRD / (k * k)
    return LOF.reshape(dTrain.shape[0])

def Apply(db_tested, db_train, sysnames, alpha=0.05, N=50):
    """
    Inputs: A data frame of objects to be tested, a training data set, and a list of ratios to use
    Outputs: An array of booleans of indices in the test data frame which have passed the filter
    """

    #Reconstruct element names from ratio names
    el_list= []
    for r in sysnames:
        el_list.extend(DecomposeR(r))

    #Get matrices for the tested and training datasets
    print("ApplyLOF: Normalise & take logarithms")
    dTest  = LogRatioMatrix(db_tested,el_list, stripNaNs=False)
    dTrain = LogRatioMatrix(db_train, el_list, stripNaNs=True)

    #Compute extents of training data for each dimension, renormalise all data
    dTrain, midpts, extents = Renormalise(dTrain)
    dTest = Renormalise(dTest, midpts, extents)[0]
 
    #Fill in missing data
    print("ApplyLOF: Interpolate missing values")
    dTest= FillNaNs(dTrain, dTest, N)

    print("ApplyLOF: Compute testing set factors")
    LOF_test= ComputeLOF(dTest, dTrain, N)
    print("ApplyLOF: Compute training set factors")
    LOF_ref= ComputeLOF2(dTrain, N)

    #Prepare the rejections filter
    Filter= np.full(dTest.shape[0],True,dtype=np.bool)

    #Find cutoff LOF value for the training set
    c = np.percentile(LOF_ref,100*(1-alpha))
    passing_samples= (LOF_test < c)
    Filter= (Filter & passing_samples)
    print("ApplyLOF: Evaluate with c="+str(c))

    # DEBUG: Plot results with mayavi
    if _debugVisualise:
        from mayavi import mlab
        dTest[np.isinf(dTest)]= 1000
        LOF_test[np.isinf(LOF_test)]= 1000
        mlab.figure('RSV ')
        mlab.points3d(dTrain.T[-3], dTrain.T[-2], dTrain.T[-1], LOF_ref, scale_mode='none', scale_factor=0.01)
        mlab.axes()
        mlab.xlabel(sysnames[-3])
        mlab.ylabel(sysnames[-2])
        mlab.zlabel(sysnames[-1])
        mlab.show()
        mlab.figure('RSV ')
        mlab.points3d(dTrain.T[-6], dTrain.T[-5], dTrain.T[-4], LOF_ref, scale_mode='none', scale_factor=0.01)
        mlab.axes()
        mlab.xlabel(sysnames[-6])
        mlab.ylabel(sysnames[-5])
        mlab.zlabel(sysnames[-4])
        mlab.show()

    #Return boolean mask for test data frame
    return Filter
