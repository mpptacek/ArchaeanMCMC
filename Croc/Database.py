"""
Functionality related to handling HL38 and pandas databases
"""

from io import StringIO
import pandas
import numpy as np

import clib.HL888 as HL38

from . import FilterLOF
from . import Bootstrap
from .Util import *

def BaseFilterSpace():
    """
    Returns the common filter space.
    """
    return ["Sm/MnO", "La/Th", "La/Sc", "Ce/Zr",
            "Al2O3/TiO2", "Ce/Co", "CaO/TiO2",
            "CaO/Al2O3", "Ni/Y", "Th/Y",
            "Th/TiO2", "La/Cr", "Sc/Cr", "Pb/Sc"]

def ShaleElements():
    """
    List of elements present in our shale databases
    """
    return ["SiO2", "TiO2", "CaO", "Al2O3",
            "MnO", "Cu", "Pb", "Hf", "Zr",
            "Nb", "Ta", "Y", "La", "Ce",
            "Pr", "Nd", "Sm", "Eu", "Gd",
            "Tb", "Dy", "Ho", "Er", "Tm",
            "Yb", "Lu", "Th", "Ni", "Co",
            "Cr", "Sc", "Zn", "V"]


def IsRatioGood(ratio):
    """
    Tests that both elements in the input ratio exist in our shale database
    """
    if '/' not in ratio:
        return False
    [A, B] = DecomposeR(ratio)
    if A not in ShaleElements():
        return False
    if B not in ShaleElements():
        return False
    return True

def LoadRandomSubset(dataframe, factor):
    """
    Loads a random subset from a dataframe that is 0 - 100% of the original size. Factor should be in [0, 1].
    """
    random_filter = np.random.random_sample((len(dataframe.index))) < factor
    return dataframe[random_filter]

def LoadHL38(dbName):
    """
    Load a HL38 database into a pandas dataframe
    """
    return pandas.read_csv(StringIO(HL38.DatabaseAsCSV(dbName)), na_values=['','x'])

def ReadFile(fname):
    """
    Load the unfiltered geological shale database
    """
    return pandas.read_csv("db/ortes/"+fname+".csv", encoding='utf-8', na_values=['','x'])

def GenerateSynthetic(suffix=""):
    """
    Generate a new synthetic shale database (with no filter applied)
    """
    Bootstrap.NukeCache()
    stepTime= 1
    dbSynth = pandas.read_csv(StringIO(HL38.GenerateSyntheticDatabase_Dual(stepTime)))
    return FilterResult(dbSynth, dbSynth, "GEN_SYNTH_DATA_STEPTIME-"+str(stepTime)+"-"+suffix)

def LoadOptimaLOF(reconSpace):
    """
    Finds the optimal filter space for a given reconstruction space,
    and uses it to filter the shale database
    """
    from . import FilterSpaceOptimiser
    from . import SciConstants
    return LoadFilteredLOF(FilterSpaceOptimiser.Find(reconSpace, SciConstants.ReconElements()))

def LoadFilteredLOF(filterSpace=None):
    """
    Run the LOF filter on the shale database
    """
    #Reset any existing bootstraps (since they will have been based on the previous filter)
    Bootstrap.NukeCache()

    FilterShales= filterSpace is not None
    #Access the igneous database
    HL38.LoadModule("CommonDBs")
    dbIgn= pandas.read_csv(StringIO(HL38.DatabaseAsCSV("keller_unfiltered")), na_values=['','x'])

    #Construct a LOF filter, apply it to the shale database        
    dbShale= ReadFile("all")
    filterApplied= ""
    if FilterShales:
        print("Applying filter for system: "+str(filterSpace))
        filterApplied+= MultiCompose(filterSpace)
        lofFilter= FilterLOF.Apply(dbShale, dbIgn, filterSpace)
        passrate= np.sum(lofFilter) / len(dbShale.index)
        print("Filter applied: pass rate is "+str(100*passrate)+"%")
        dbShale_f= dbShale[lofFilter]
    else:
        dbShale_f = dbShale

    return FilterResult(dbShale, dbShale_f, filterApplied)

class FilterResult:
    """
    Convenience class to store a filtered shale database
    """
    def __init__(self, db_shale_orig, db_shale_filtered, filters_applied=""):
        self.dbShale = db_shale_filtered
        self.filterApplied = filters_applied
        self.dbShale0 = db_shale_orig
