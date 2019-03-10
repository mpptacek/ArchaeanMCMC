CC=gcc -flto -O3 -march=native
DEFINES= PYTHON_LIB
INC_DEFINES=$(DEFINES:%=-D%)
CFLAGS= -std=c++11 -fPIC -shared -fpermissive -w $(INC_DEFINES)

TARGET = ./../HL888.so

INCLUDE = /mnt/c/Users/Matous/Documents/c++/boost_1_66_0_unix /usr/include/python3.6m
INC_PARAMS=$(INCLUDE:%=-I%)
ODIR=x64/obj

LIB_PATH = /mnt/c/Users/Matous/Documents/c++/boost_1_66_0_unix/stage/lib
LIBS=-lm -lstdc++ -lboost_python3
R_PATH = /mnt/c/Users/Matous/Documents/c++/boost_1_66_0_unix/stage/lib

_DEPS = Analysis.h csvParser.h csvWriter.h MemberOffset.h Model.h module.h moduleCommon.h pyLib.h reconCommon.h reconEndmembers.h reconManager.h reconResultsProcessors.h RockDatabase.h RockDatabaseFilter.h RockSample.h SpecialisedRockDatabase.h stdafx.h utils.h WRB.h

_OBJ = CommonDBs.o csvParser.o MCMCRecon.o MemberOffset.o module.o moduleCommon.o pyLib.o pyIO_ReconClasses.o reconCommon.o reconEndmembers.o reconManager.o reconResultsProcessors.o RockDatabase.o RockSample.o SpecialisedRockDatabase.o stdafx.o utils.o WRB.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INC_PARAMS)

HyperLemming888: $(OBJ)
	$(CC) -o $(TARGET) $^ $(CFLAGS) -L$(LIB_PATH) $(LIBS) -Wl,-rpath,$(R_PATH)
	
.PHONY: clean

clean: 
	$(RM) $(TARGET) $(ODIR)/*.o
	