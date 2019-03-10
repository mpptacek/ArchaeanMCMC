"""
Identifies the optimal filter space for a given reconstruction system,
given a set of good elements
"""
import progressbar
import numpy as np

import clib.HL888 as HL38

from . import Database
from . import Rombinatorics
from . import MetaStat
from . import Util

MAX_LEN = 14
DEBUG_LOG = False

def Find(recon_system, good_elements_list):
    """
    Returns the optimal filter space for a given system.
    """
    print("OPTIMISING FILTER SPACE FOR: "+str(recon_system))
    #Get the igneous database
    HL38.LoadModule("CommonDBs")
    ign = Database.LoadHL38("keller_unfiltered")

    #Generate all possible ratios
    candidate_list = Rombinatorics.Gen_NoRepeats(good_elements_list, 1)
    
    #Calculate the MI of every reconstructor-candidate ratio pair
    n_r = len(recon_system)
    n_c = len(candidate_list)
    mi = {}
    print("COMPUTING MUTUAL INFORMATION")
    p_bar = progressbar.ProgressBar(max_value=n_r * n_c)
    for r in range(n_r):
        r_name = recon_system[r]
        mi[r_name] = []
        [rA, rB] = Util.DecomposeR(r_name)
        reconstructor_val = ign[rA] / ign[rB]
        filter_r = np.isfinite(reconstructor_val)
        for c in range(n_c):
            c_name = candidate_list[c][0]
            [cA, cB] = Util.DecomposeR(c_name)
            candidate_val = ign[cA] / ign[cB]
            filter_c = np.isfinite(candidate_val)
            filter_rc = np.bitwise_and(filter_r, filter_c)
            calc_mi = MetaStat.MutualInformation(reconstructor_val[filter_rc], candidate_val[filter_rc], 20)
            mi[r_name].append((calc_mi, c_name))
            p_bar.update(r * n_c + c)
    p_bar.finish()

    #Sort by MI for each reconstruction ratio separately
    for mi_list in mi.values():
        mi_list.sort(key=lambda x: x[0], reverse=True)
    
    if DEBUG_LOG:
        for r_sys in recon_system:
            print(mi[r_sys])

    #Pick the best ratios for each reconstruction system, in turn, such that elements aren't repeated
    r_list = []
    elements_remaining = set(good_elements_list)
    
    def RatioGood(r):
        [A, B] = Util.DecomposeR(r)
        if A not in elements_remaining:
            return False
        if B not in elements_remaining:
            return False
        return True

    def AddRatio2FilterSpace(r):
        r_list.append(r)
        [A, B] = Util.DecomposeR(r)
        if A in elements_remaining:
            elements_remaining.remove(A)
        if B in elements_remaining:
            elements_remaining.remove(B)
    #Include the original reconstruction ratios in the filter
    for ratio in recon_system:
        AddRatio2FilterSpace(ratio)

    while elements_remaining:
        #Stop if filter is getting too long
        if len(r_list) > MAX_LEN:
            print("EXCEEDED MAX FILTER SIZE")
            break
        #Stop if we've exhausted all lists
        valid_sys = 0
        for recon_r in recon_system:
            if mi[recon_r]:
                valid_sys += 1
        if valid_sys == 0:
            print("EXHAUSTED VALID SYSTEMS LIST")
            break
               
        #Add top candidate from each reconstruction ratio to the filter list
        #Make sure elements are not repeated
        for recon_r in recon_system:
            top_pick = mi[recon_r][0]
            if RatioGood(top_pick[1]):
                AddRatio2FilterSpace(top_pick[1])
            mi[recon_r] = mi[recon_r][1:]

    if not elements_remaining:
        print("EXHAUSTED ALL ELEMENTS!")

    print("FOUND OPTIMAL SPACE: "+str(r_list))
    return r_list
