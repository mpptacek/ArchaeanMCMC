"""
aefgagea
"""
from Croc import *

if __name__ == '__main__':
    fR = Database.LoadFilteredLOF()
    while True:
        print("ENTER RATIOS: (or Q to QUIT)")
        l = InputRatioList(Database.IsRatioGood)

        if len(l) < 3:
            print("AT LEAST THREE ENDMEMBERS ARE REQUIRED")
            continue

        c = Recon.HL38Config(Recon.EndmemberType.Dual, Recon.ReconType.MCMC, Recon.EndmemberConfig.QUARTUS, 401.0)
        c.FilterResult(Database.LoadOptimaLOF(l))
        c.ReconSystems(l)
        Recon.Visualiser(c).Show3DMixSpace(0)
