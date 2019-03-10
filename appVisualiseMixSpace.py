"""
An applet to visualise mixing spaces for the Archaean and Modern periods
"""
from Croc import *

if __name__ == '__main__':
    while True:
        print("ENTER RATIOS: (or Q to QUIT)")
        l = InputRatioList(Database.IsRatioGood)

        c = Recon.HL38Config(Recon.EndmemberType.Dual, Recon.ReconType.MCMC, Recon.EndmemberConfig.KMF, 401.0)
        c.FilterResult(Database.LoadOptimaLOF(l)).ReconSystems(l)
        Recon.Visualiser(c, True).CompareArchaModern()
