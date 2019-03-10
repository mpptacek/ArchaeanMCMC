"""
Ratio Combinatoric functions.
"""
import itertools
from .Util import ComposeR, DecomposeR

def pair_iterator(lst):
    """
    Iterator that goes through a list two elements at a time.
    """
    temp = iter(lst)
    return zip(temp, temp)

def HasLargerThan(el, lst):
    """
    Return whether the iterable 'lst' (assumed to be sorted)
    contains a member larger than 'el.'
    """
    for member in lst:
        if member > el:
            return True
    return False

def GetIdxFirstLargerThan(el, lst):
    """
    Return the index of the first member
    in iterable 'lst' (assumed to be sorted)
    which is larger than 'el.'
    """
    return lst.index(GetFirstLargerThan(el, lst))

def GetFirstLargerThan(el, lst):
    """
    Return the first member in iterable 'lst' (assumed to be sorted)
    which is larger than 'el.'
    """
    for member in lst:
        if member > el:
            return member
    raise ValueError("NO MEMBERS LARGER THAN "+str(el)+" IN LIST "+str(lst))

def Gen_NoRepeats(good_elements, N):
    """
    Returns all the feasible mixing systems with N ratios.
    """
    print("CONSIDERING ELEMENTS LIST: "+str(good_elements))
    print("N = "+str(N))
    c_list = []

    el_list = good_elements
    el_list.sort()
    last_el = el_list[-1]
    lst2_el = el_list[-2]

    #Generate first solution
    stripped = el_list.copy()
    sln = []
    for idx in range(0, 2*N, 2):
        rA = el_list[idx]
        rB = el_list[idx + 1]
        sln.append((rA, rB))
        stripped.remove(rA)
        stripped.remove(rB)
    #Add to candidate solutions list
    #print(str(sln)+"|||"+str(stripped))
    c_list.append(sln.copy())
    #Keep advancing solution until no more advances possible
    #Advance by finding first advancable element, stripping off elements along the way
    while sln:
        #print("IT:"+str(sln)+"|||"+str(stripped))
        if HasLargerThan(sln[-1][1], stripped):
            tuple_idx = 1 #Advance denominator
        elif HasLargerThan(sln[-1][0], stripped):
            tuple_idx = 0 #Advance nominator
        else: 
            #Can no longer advance either nominator or denominator
            #Let's strip off more elements, then!
            stripped.append(sln[-1][0])
            stripped.append(sln[-1][1])
            stripped.sort()
            sln.pop()
            continue
        #Otherwise, advance the selected element by one
        curr_el = sln[-1][tuple_idx]
        #print("ADV "+curr_el)
        if tuple_idx == 1:
            indx_el = GetIdxFirstLargerThan(curr_el, stripped)
            advc = stripped[indx_el]
            stripped.pop(indx_el)
            stripped.append(sln[-1][1])
            stripped.sort()
            sln[-1] = (sln[-1][0], advc) #Advanced denominator
        else:
            stripped.append(sln[-1][0])
            stripped.append(sln[-1][1])
            stripped.sort()
            indx_el = GetIdxFirstLargerThan(curr_el, stripped)
            sln[-1] = (stripped[indx_el], stripped[indx_el + 1]) #Advanced nominator
            stripped.pop(indx_el)
            stripped.pop(indx_el) #Pop BOTH of the elements we've just added
        #Fill in the rest of the solution
        abort = False
        while len(sln) < N:
            last_nmtr = sln[-1][0]
            if not (HasLargerThan(last_nmtr, stripped)
                    and HasLargerThan(GetFirstLargerThan(last_nmtr, stripped), stripped)):
                #We're definitely out of elements now!
                abort = True
                break
            i_nxt_ntr = GetIdxFirstLargerThan(last_nmtr, stripped)
            rA = stripped[i_nxt_ntr]
            rB = stripped[i_nxt_ntr + 1]
            sln.append((rA, rB))
            stripped.pop(i_nxt_ntr)
            stripped.pop(i_nxt_ntr)
        if abort:
            continue
        #Log this solution and move on to the next one (if possible)
        #print(str(sln)+"|||"+str(stripped))
        c_list.append(sln.copy())

    print("FOUND "+str(len(c_list))+" UNIQUE SYSTEMS.")

    #Translate solutions to be ratios instead of tuples
    sys_list = []
    for system in c_list:
        sys_list.append([])
        for tpl in system:
            sys_list[-1].append(ComposeR(tpl[0], tpl[1]))
    return sys_list

def Gen_ElementRepeats(good_elements, N):
    """
    Returns all the feasible mixing systems with N ratios.
    Allows duplicated elements, but not reciprocal ratios.
    """
    print("CONSIDERING ELEMENTS LIST: "+str(good_elements))
    print("N = "+str(N))
    c_list = []
    #Go through all possible combinations of elements
    for el_list in itertools.combinations(good_elements, 2 * N):
        r_list = []
        #Generate all possible combinations of ratios
        for c_el_list in itertools.combinations(el_list, 2):
            for a, b in pair_iterator(c_el_list):
                r_list.append(ComposeR(a, b))
        #Pick N ratios from the list of possible ratios
        for c_r_list in itertools.combinations(r_list, N):
            c_list.append(list(c_r_list))

    print("FOUND "+str(len(c_list))+" UNIQUE SYSTEMS.")
    return c_list
