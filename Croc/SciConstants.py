"""
Store scientific constants
"""

#Modern-age proportions from the Science paper
bestfitK= 0.00
bestfitM= 0.28
bestfitF= 0.72

#Archaean proportions from the Science paper
bestfitKArch= 0.15
bestfitMArch= 0.27
bestfitFArch= 0.58

#pg46 in Taylor & McLennan - their estimate for composition of the upper continental crust
TaylorMcLennan= {"SiO2":66, "TiO2": 0.5, "Al2O3": 15.2, "CaO":4.2, "MnO": 0.077,"Sc":11, "V":60, "Cr":35, "Co":10, "Ni":20, "Cu":25, "Zn":71, "Rb":112, "Sr":350, "Y":22, "Zr":190,"Nb":25, "Mo":1.5,
                 "La":30, "Ce":64, "Pr":7.1, "Nd":26, "Sm":4.5, "Eu":0.88, "Gd":3.8, "Tb":0.64, "Dy":3.5, "Ho":0.8, "Er":2.3, "Tm":0.33, "Yb":2.2, "Lu":0.32, "Hf":5.8, "Ta":2.2, "Pb":20, "Th":10.7}
#Treatise, 2003, Rudnick & Gao
RudnickGao= {"SiO2":66.62, "TiO2": 0.64, "Al2O3": 15.4, "CaO":3.59, "MnO": 0.1,"Sc":14, "V":97, "Cr":92, "Co":17.3, "Ni":47, "Cu":28, "Zn":67, "Rb":84, "Sr":320, "Y":21, "Zr":193,"Nb":12, "Mo":1.1,
             "La":31, "Ce":63, "Pr":7.1, "Nd":27, "Sm":4.7, "Eu":1.0, "Gd":4.0, "Tb":0.7, "Dy":3.9, "Ho":0.83, "Er":2.3, "Tm":0.3, "Yb":2.0, "Lu":0.31, "Hf":5.3, "Ta":0.9, "Pb":17, "Th":10.5}

def ReconElements():
    """
    Elements ideal for reconstruction (by the criteria outlined in the 2018 paper)
    """
    return ['Sc', 'Cr', 'Co', 'Ni', 'Al2O3', 'Zr',
            'Hf', 'Nb', 'Ta', 'TiO2', 'La', 'Ce',
            'Yb', 'Y', 'Th', 'Pb']
