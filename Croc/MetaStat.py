"""
NumPy functionality to compute distance-related functions on large matrices.
"""
import numpy as np
import sklearn.metrics

def NearestAB_N_NeighbourIdx_IgnoreNans(A, B, N):
    """
    Find the indices of the N nearest neighbours in A for every point in B
    Result is a 2D array of observation indices for A, of shape (#observations in B, N)
    Result[:,0] represents the nearest neighbour indices, and Result[:,N-1] the Nth neighbour
    """
    (n_trn, n_dim1)= A.shape
    (n_tst, n_dim2)= B.shape
    assert n_dim1 == n_dim2, "Matrices A and B have incompatible dimensions!"
    assert N < n_trn, "N too large for number of training points!"
    n_dim= n_dim1
    acc= np.zeros((n_tst, n_trn), dtype=np.float32)
    incr= np.empty_like(acc).T
    #In order to reduce memory consumption, do this one dimension at a time, via an accumulator matrix
    #Also do this in 32 bit precision (since numbers are already normalized to a reasonable range)
    for D in range(n_dim):
        incr= A.astype(np.float32)[:,D].reshape(n_trn,1) - B.astype(np.float32)[:,D].reshape(n_tst,1).T
        np.square(incr,incr)
        incr[np.isnan(incr)]= 0
        acc+= incr.T
    np.sqrt(acc,acc)
    #Returning [:, 0:N] would mean that the nearest neighbour for P would be P itself - obviously wrong! 
    return acc.argsort(axis=1)[:,1:N + 1]

def AB_NeighbourDistances_IgnoreNans(A, B):
    """
    Find the distance to every point in A for every point in B
    Result is a 2D array of observation indices for A, of shape (#observations in B, #observations in A)
    """
    (n_trn, n_dim1)= A.shape
    (n_tst, n_dim2)= B.shape
    assert n_dim1 == n_dim2, "Matrices A and B have incompatible dimensions!"
    n_dim= n_dim1
    acc= np.zeros((n_tst, n_trn),dtype=np.float32)
    incr= np.empty_like(acc).T
    #In order to reduce memory consumption, do this one dimension at a time, via an accumulator matrix
    #Also do this in 32 bit precision (since numbers are already normalized to a reasonable range)
    for D in range(n_dim):
        incr= A.astype(np.float32)[:,D].reshape(n_trn,1) - B.astype(np.float32)[:,D].reshape(n_tst,1).T
        np.square(incr,incr)
        incr[np.isnan(incr)]= 0
        acc+= incr.T
    np.sqrt(acc,acc)
    return acc

def DistanceToIdx(A, B, idx):
    """
    For the ith point in B, find the distance to the point in A addressed by the ith index in array 'idx'
    """
    C= A[idx]
    return np.sqrt(np.sum(np.square(C-B),axis=1,keepdims=True))

def MutualInformation(x, y, bins):
    """
    Calculates the MI of a set of two-dimensional vectors, separated into the arrays x and y.
    Discretises the distribution into an amount of bins.
    """
    hist_xy, x_edges, y_edges = np.histogram2d(x, y, bins)
    return sklearn.metrics.mutual_info_score(None, None, hist_xy)
