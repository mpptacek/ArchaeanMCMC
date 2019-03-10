Computer code accompanying the manuscript "Chemical Evolution of the Crust from an Inversion of the Composition of Terrigenous Sediments and Implications for the Geotherm of Archaean Continents."
M. P. Ptacek, N. Dauphas, N. D. Greber
Origins Lab, University of Chicago
March 2019

STRUCTURE OF THIS REPOSITORY:
 The top-level directory contains four subfolders, five Python "apps," two Visual Studio solution files, and this readme file.
 Explanation of the directories:
 - "clib" contains the HL888 C++ library that implements the MCMC algorithm, both as a compiled shared object and as source code.
 - "cout" will contain outputs from the Python apps.
 - "Croc" contains the Python geoanalytical library functions called by the apps, such as LOF filtering and database management.
 - "db" contains the geochemical databases.

 Explanation of the Python apps:
 - "appMixSpaceScan.py" calculates error propagation metrics for large numbers of geochemical ratio combinations. It was used to select the combinations of ratios best-suited for our MCMC reconstruction. It outputs a csv file containing performance metrics for each scanned ratio. The input parameters must be modified in the source code, rather than being supplied on the command-line.
 - "appMultiRecon.py" is a command-line application that our applies the main reconstruction methodology described in our manuscript and produces reconstructions of the ancient Earth's crust. These are written to the "cout" folder.
 - "appVisualiseLOF.py" visualises the results of applying the LOF filter to the OrTeS database.
 - "appVisualiseMixSpace.py" visualises a two-ratio mixing space during two time periods (the Archaean and present-day), then writes these images to the "cout" directory.
 - "appVisualiseMixSpace3D.py" visualises a three-ratio mixing space in an interactive graphic window.

DATA SOURCES FOR THIS APPLICATION:
  Three separate data sources are required to execute the code in this repository. These correspond to the three subdirectories within the 'db' folder:
  - 'keller_web' must contain the Keller & Schoene 2012 database in csv format
  - 'ortes' must contain the OrTeS database as a file named "all.csv"
  - 'PetDB_NoOcean' must contain our custom query of the PetDB database in csv format
  
  These databases are not distributed with this repository. If you want to run these programs yourself, please see the associated manuscript on how to acquire these data sources.
 