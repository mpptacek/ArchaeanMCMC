"""
(Very messy) code to plot and evaluate ratio mixing spaces
"""

import numpy as np
from .Util import *

def GenIntermediateMix(P0A, P0B, P1A, P1B, res=50):
    """
    Plots one set of guidelines on the ternary diagram
    """
    pts= []
    pts.append(P0A/P0B)
    step = 1.0 / float(res)
    for x in frange(0.0, 1.0, step):
        PA= (1.0 - x)*P0A + x*P1A
        PB= (1.0 - x)*P0B + x*P1B
        pts.append(PA/PB)
    pts.append(P1A/P1B)

    return np.array(pts).T

def Plot(plt, r0A, r0B, r1A, r1B, endLabels):
    """
    Creates a ternary plot of a N-ratio mixing space
    """
    N = len(r0A)
    endPts = [x for x in range(N)]

    #Stack X and Y coordinates together
    ptsA0 = np.array(list(r0A)).reshape((N, 1))
    ptsA1 = np.array(list(r1A)).reshape((N, 1))
    ptsB0 = np.array(list(r0B)).reshape((N, 1))
    ptsB1 = np.array(list(r1B)).reshape((N, 1))
    ptsA = np.hstack([ptsA0, ptsA1])
    ptsB = np.hstack([ptsB0, ptsB1])
    pts = ptsA / ptsB

    #Generate lines representing the boundaries of the mixing space
    edgeLines = []
    edgeLines.append(GenIntermediateMix(ptsA[0], ptsB[0], ptsA[1], ptsB[1]))
    edgeLines.append(GenIntermediateMix(ptsA[1], ptsB[1], ptsA[2], ptsB[2]))
    edgeLines.append(GenIntermediateMix(ptsA[2], ptsB[2], ptsA[0], ptsB[0]))

    #Generate intermediate mixing lines
    intermediateLines= []
    for x in frange(0.0, 1.0, 0.1):
        ptsA_1 = x*ptsA[0] + (1-x)*ptsA[1]
        ptsB_1 = x*ptsB[0] + (1-x)*ptsB[1]
        ptsA_2 = x*ptsA[0] + (1-x)*ptsA[2]
        ptsB_2 = x*ptsB[0] + (1-x)*ptsB[2]
        intermediateLines.append(GenIntermediateMix(ptsA_1, ptsB_1, ptsA_2, ptsB_2))
    for x in frange(0.0, 1.0, 0.1):
        ptsA_1 = x*ptsA[2] + (1-x)*ptsA[1]
        ptsB_1 = x*ptsB[2] + (1-x)*ptsB[1]
        ptsA_2 = x*ptsA[2] + (1-x)*ptsA[0]
        ptsB_2 = x*ptsB[2] + (1-x)*ptsB[0]
        intermediateLines.append(GenIntermediateMix(ptsA_1, ptsB_1, ptsA_2, ptsB_2))
    for x in frange(0.0, 1.0, 0.1):
        ptsA_1 = x*ptsA[1] + (1-x)*ptsA[2]
        ptsB_1 = x*ptsB[1] + (1-x)*ptsB[2]
        ptsA_2 = x*ptsA[1] + (1-x)*ptsA[0]
        ptsB_2 = x*ptsB[1] + (1-x)*ptsB[0]
        intermediateLines.append(GenIntermediateMix(ptsA_1, ptsB_1, ptsA_2, ptsB_2))
       
    #Set diagram extents
    x_b = pts[:,0]
    y_b = pts[:,1]
    x_rng = max(x_b) - min(x_b)
    y_rng = max(y_b) - min(y_b)
    
    plt.xlim(min(x_b) - 0.1*x_rng, max(x_b) + 0.1*x_rng)
    plt.ylim(min(y_b) - 0.1*y_rng, max(y_b) + 0.1*y_rng)

    #Plot all lines
    for iL in intermediateLines:
        plt.plot(iL[0], iL[1],"0.5")
    for eL in edgeLines:
        plt.plot(eL[0], eL[1],"w")
    #Plot labels
    for i in endPts:
        plt.plot(pts[i][0], pts[i][1], "o", label=endLabels[i])

def Show3D(aS, endNames, reconSystems):
    """
    Visualize a 3D mixing space using mayavi
    Input 0: SingleTimeState object from HL38
    Input 1: List of endmember names
    Input 2: List of ratio names
    """
    N = len(aS[0].cA)

    #Stack X and Y coordinates together
    ptsA0 = np.array(list(aS[0].cA)).reshape((N, 1))
    ptsA1 = np.array(list(aS[1].cA)).reshape((N, 1))
    ptsA2 = np.array(list(aS[2].cA)).reshape((N, 1))
    ptsB0 = np.array(list(aS[0].cB)).reshape((N, 1))
    ptsB1 = np.array(list(aS[1].cB)).reshape((N, 1))
    ptsB2 = np.array(list(aS[2].cB)).reshape((N, 1))
    ptsA = np.hstack([ptsA0, ptsA1, ptsA2])
    ptsB = np.hstack([ptsB0, ptsB1, ptsB2])

    #Generate lines representing the boundaries of the mixing space
    edgeLines = []
    edgeLines.append(GenIntermediateMix(ptsA[0], ptsB[0], ptsA[1], ptsB[1]))
    edgeLines.append(GenIntermediateMix(ptsA[0], ptsB[0], ptsA[2], ptsB[2]))
    edgeLines.append(GenIntermediateMix(ptsA[0], ptsB[0], ptsA[3], ptsB[3]))
    edgeLines.append(GenIntermediateMix(ptsA[1], ptsB[1], ptsA[2], ptsB[2]))
    edgeLines.append(GenIntermediateMix(ptsA[1], ptsB[1], ptsA[3], ptsB[3]))
    edgeLines.append(GenIntermediateMix(ptsA[2], ptsB[2], ptsA[3], ptsB[3]))

    #Get MCMC states
    mcmcX= np.array(aS[0].mcmcR)
    mcmcY= np.array(aS[1].mcmcR)
    mcmcZ= np.array(aS[2].mcmcR)

    #Compute ranges
    #It's ugly, but it works...
    minX = None
    maxX = None
    minY = None
    maxY = None
    minZ = None
    maxZ = None
    for i in range(4):
        cX = aS[0].cA[i] / aS[0].cB[i]
        cY = aS[1].cA[i] / aS[1].cB[i]
        cZ = aS[2].cA[i] / aS[2].cB[i]
        if minX is None:
            minX = cX
            maxX = cX
            minY = cY
            maxY = cY
            minZ = cZ
            maxZ = cZ
        else:
            minX = min(minX, cX)
            minY = min(minY, cY)
            minZ = min(minZ, cZ)
            maxX = max(maxX, cX)
            maxY = max(maxY, cY)
            maxZ = max(maxZ, cZ)
    SFACTOR = 0.1
    rngX = SFACTOR * (maxX - minX)
    rngY = SFACTOR * (maxY - minY)
    rngZ = SFACTOR * (maxZ - minZ)
        
    """
    minX = minX - SFACTOR * rngX
    maxX = maxX + SFACTOR * rngX
    minY = minY - SFACTOR * rngY
    maxY = maxY + SFACTOR * rngY
    minZ = minZ - SFACTOR * rngZ
    maxZ = maxZ + SFACTOR * rngZ
    """

    #Do the 3D
    #We have to do ugly rescaling of the coordinates here otherwise scales become incredibly wrong 
    from mayavi import mlab
    mlab.figure('3D MixSpace')
    for eL in edgeLines:
        mlab.plot3d((eL[0]-minX)/rngX, (eL[1]-minY)/rngY, (eL[2]-minZ)/rngZ)
    mlab.points3d((mcmcX-minX)/rngX, (mcmcY-minY)/rngY, (mcmcZ-minZ)/rngZ,
                  scale_factor=0.05)
    for i in range(4):
        cX = aS[0].cA[i] / aS[0].cB[i]
        cY = aS[1].cA[i] / aS[1].cB[i]
        cZ = aS[2].cA[i] / aS[2].cB[i]
        mlab.points3d((cX-minX)/rngX, (cY-minY)/rngY, (cZ-minZ)/rngZ,
                      color=(1,0,0), scale_factor=0.2)
        mlab.text3d((cX-minX)/rngX, (cY-minY)/rngY, (cZ-minZ)/rngZ,
                    endNames[i], scale=0.4)

    mlab.points3d((aS[0].bootR-minX)/rngX, (aS[1].bootR-minY)/rngY, (aS[2].bootR-minZ)/rngZ,
                  color=(0,0,1), scale_factor=0.1)
    mlab.text3d((aS[0].bootR-minX)/rngX, (aS[1].bootR-minY)/rngY, (aS[2].bootR-minZ)/rngZ,
                "Boot", scale=0.2)

    #mlab.axes(extent=[minX, maxX, minY, maxY, minZ, maxZ], xlabel=reconSystems[0], ylabel=reconSystems[1], zlabel=reconSystems[2])
    mlab.axes(extent =[0, 1 / SFACTOR, 0, 1 / SFACTOR, 0, 1 / SFACTOR], xlabel=reconSystems[0], ylabel=reconSystems[1], zlabel=reconSystems[2])
    mlab.show()

def Area(List):
    """
    Compute the area of a self-intersecting 2D polygon with the [x, y] vertices supplied in a list
    """
    complex_list = []
    for v in List:
        complex_list.append(complex(v[0], v[1]))
    return AreaC(complex_list)

def AreaC(IN):
    """
    From https://codegolf.stackexchange.com/questions/47638/area-of-a-self-intersecting-polygon
    """
    def I(s, a, b=1j):
        """
        uhhh... it's complicated
        """
        c, d = s
        d -= c
        c -= a
        e = (d*b.conjugate()).imag
        return e * (0 <= (b*c.conjugate()).imag * e <= e*e) and \
               [a + (d*c.conjugate()).imag * b/e] or []

    E = lambda p: list(zip(p, p[1:] + p))
    S = sorted

    P = E(IN)

    return sum(
        (t - b) * (r - l) / 2

        for l, r in E(S(
            i.real for a, b in P for e in P for i in I(e, a, b - a)
        ))[:-1]

        for b, t in E(S(
            ((i + j).conjugate()).imag for e in P for i in I(e, l) for j in I(e, r)
        ))[::2]
    )
    