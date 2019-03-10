"""
Assorted utility functions useful throughout the project.
Mostly involve ratio strings
"""

def frange(start, stop, step = 1.0):
    """
    A floating-point equivalent of the range() generator
    """
    i = start
    while i < stop:
        yield i
        i += step

def DecomposeR(ab):
    """
    Decompose a "a/b" string into ["a","b"]
    """
    r= ab.split('/',1)
    return [r[0], r[1]]

def ComposeR(a,b):
    """
    Compose a "a/b" string from "a", "b"
    """
    return a+"/"+b

def MultiCompose(L):
    """
    Compose a filename-friendly multiratio string from a list of strings
    """
    fstr= ""
    for s in L:
        if fstr:
            fstr+= "-"
        templist= s.split('/',1)
        fstr+= templist[0]+templist[1]
    return fstr

def GenerateList(elList):
    """
    Given a list of elements, generate a list of all the possible ratios that can be generated from those elements
    """
    r= []
    for a in elList:
        for b in elList:
            if a != b:
                if ComposeR(a,b) not in r:
                    r.append(ComposeR(b,a))
    return r


def FlipR(ab):
    """
    Flip a string ratio, from "A/B" to "B/A"
    """
    [A, B] = DecomposeR(ab)
    return ComposeR(B, A)

def Dict2CSV(fname, Dict):
    """
    Write a dictionary to a csv
    """
    wfile = open(fname+".csv","w")
    wfile.write("KEY, VAL \n")
    for k, v in Dict.items():
        wfile.write(str(k)+","+str(v)+"\n")

def RandomElement(List):
    """
    Select a random element from a list
    """
    from random import randint
    return List[randint(0, len(List) - 1)]

def InputRatioList(test_function):
    """
    Input a ratio list from the console.
    Exists if input was 'Q'
    Perform error-checking to ensure it only contains valid elements.
    """
    while True:
        l= []
        inList = input().split(" ")
        if inList[0].upper() == "Q":
            quit()
        allGood= True
        for inS in inList:
            if test_function(inS):
                l.append(inS)
            else:
                print("BAD RATIO "+inS)
                allGood= False
        if allGood:
            break
        else:
            print("ENTER NEW INPUT:")
    return l
