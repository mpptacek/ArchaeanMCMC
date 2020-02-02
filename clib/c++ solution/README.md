#### C++ library accompanying the manuscript "Chemical Evolution of the Crust from an Inversion of the Composition of Terrigenous Sediments and Implications for the Geotherm of Archaean Continents."
#### M. P. Ptacek, N. Dauphas, N. D. Greber
#### Origins Lab, University of Chicago
#### March 2019

### PREREQUISITES:
- The Boost Python library (https://www.boost.org/doc/libs/1_69_0/libs/python/doc/html/index.html), built with Python 3 support
- Python 3 C++ development headers (https://www.python.org/)
- For Windows only: dirent-windows (available at https://github.com/tronkko/dirent)

### TO BUILD ON WINDOWS:
- Open solution file in Visual Studio
- Within the Solution Explorer, right-click on 'HL888', then navigate to 'VC++ Directories"
- Change all three entries within 'Include Directories' to point to the correct locations on your system
- Also change both entries within 'Library Directories'
- Build the project

### TO BUILD ON LINUX:
- Open makefile
- Edit variable 'INCLUDE' to point to the correct include directories
- Edit variable 'LIB_PATH' to point to the correct library directories
- Do either of:
  - If you have installed lboost_python3 in a non-standard location, update variable R_PATH to point to that location
  - If your lboost_python3 is in a standard location, remove the fragment '-Wl,-rpath,$(R_PATH)' from the end of line 27
-Build using 'make'

### TO BUILD IN A CUSTOM ENVIRONMENT:
- Ensure your compiler can find all necessary libraries
- Define PYTHON_LIB in the preprocessor
- Build all files, and link into a Python shared object

### ON THE STRUCTURE OF THIS LIBRARY:
All Python-facing functions are defined within the file 'pyIO_ReconClasses.cpp' - the Boost.Python library is used to provide a very lightweight wrapper around the underlying C++ objects. The most interesting code is found within 'MCMCRecon.cpp,' which implements the core of the MCMC algorithm. The file 'reconEndmembers.cpp' is also noteworthy, as it defines the reconstruction endmembers. Finally, 'WRB.cpp' implements the exponential weighted-average model, and the bootstrap algorithm.
