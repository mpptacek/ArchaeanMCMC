"""
Implementation of the cached bootstrap loader
"""

from .Util import *

#Persistent bootstrap cache
_bootStr= {}

def IsCached(sys):
    """
    Verifies whether a boostrap is in the persistent cache
    """
    if sys in _bootStr:
        return True
    return False

def Cache(sys, reconManager):
    """
    Generate a bootstrap and store it in the persistent cache
    """
    [A, B] = DecomposeR(sys)
    _bootStr[sys]= reconManager.GenerateBootstrap(A, B)

def NukeCache():
    """
    Clear bootstrap cache (necessary if we changed shale databases or filters)
    """
    global _bootStr
    _bootStr= {}

def GetCached(sys, reconManager):
    """
    Attempts to retrieve a single bootstrap from the cache.
    If it doesn't exist, generates a new one.
    """
    if not IsCached(sys):
        Cache(sys, reconManager)
    return _bootStr[sys]

def ReadCache(sys):
    """
    Retrieves a single bootstrap from the cache.
    """
    if not IsCached(sys):
        raise AssertionError("BOOTSTRAP CACHE MISS FOR SYSTEM "+str(sys))
    return _bootStr[sys]
