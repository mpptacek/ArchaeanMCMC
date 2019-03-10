#include "stdafx.h"
#include "pyLib.h"

#ifdef PYTHON_LIB

//This function is called when the Python library is imported, after C++ static initialisation completes
BOOST_PYTHON_MODULE(HL888) {
	PythonRegistry::Get().ExecAll();
};

PythonRegistry& PythonRegistry::Get() {
	static PythonRegistry singleton;
	return singleton;
}

void PythonRegistry::Register(const std::string & s , PY_INIT_FPTR f) {
	sname.push_back(s);
	init.push_back(f);
};

void PythonRegistry::ExecAll() {
	std::cout << "PYTHON LINK INITIATED." << std::endl;
	for (size_t i = 0; i < init.size(); ++i) {
		std::cout << "LINKING " << sname[i] << std::endl;
		init[i]();
	};
	std::cout << "PYTHON LINK SUCCESSFUL." << std::endl;
};

#endif


