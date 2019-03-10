"""
An applet to generate a reconstruction based on user input
"""

from Croc import *

if __name__ == '__main__':
    recon_type = Recon.ReconType.MCMC
    print("Use komatiites (Y/N)?")
    useK = (input().upper() == "Y")
    if useK:
        print("Use tholeiitic endmembers (Y/N)?")
        useT = (input().upper() == "Y")
        if useT:
            endmember_config = Recon.EndmemberConfig.QUARTUS
        else:
            endmember_config = Recon.EndmemberConfig.KMF
    else:
        endmember_config = Recon.EndmemberConfig.MF

    while True:
        print("ENTER RATIOS: (or Q to QUIT)")
        l = InputRatioList(Database.IsRatioGood)
        c = Recon.HL38Config(Recon.EndmemberType.Bootstrap, recon_type, endmember_config, 400.0)
        c.FilterResult(Database.LoadOptimaLOF(l)).ReconSystems(l)
        c.EndmemberWidths(1000.0,450.0)
        Recon.Visualiser(c, True).PlotBootstraps().CompareArchaModern().TimelineRecon()
