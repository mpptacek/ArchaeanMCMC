#include "stdafx.h"
#include "module.h"

Module::Module() {
	ModuleCoordinator::Get().Register(this);
};

Module::~Module() {
	ModuleCoordinator::Get().Unregister(this);
};

RockDatabase & Module::db(const std::string & S) const {
	return ModuleCoordinator::RequestDB(S);
};

void ModuleCoordinator::LoadModuleIfRequired(Module* i) {
	if (loadedModules.count(i) > 0) {
		return;
	}
	for (std::string dsid : deps[i->sid]) {
		LoadModuleIfRequired(ms[dsid]);
	};
	if (i->LoadOnly()) {
		std::cout << "  MODULE: " << i->sid << std::endl;
		i->Exec();
		loadedModules.insert(i);
	};
};

void ModuleCoordinator::Initialise() {
	//Construct
	for (auto i : cs) {
		ms[i.first] = (*i.second)();
		ms[i.first]->sid = i.first;
	};
	std::cout << "MODULE MANAGER INITIALISED." << std::endl;
};

void ModuleCoordinator::ProcessAll() {
	//Initialise
	Initialise();
	//List modules
	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	std::cout << "MC: AVAILABLE MODULES:" << std::endl;
	for (Module* i : iqs) {
		if (!i->LoadOnly()) {
			std::cout << "--" << i->sid << std::endl;
		};
	};
	//Execute modules
	while (true) {
		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		std::cout << "ENTER MODULE TO EXECUTE: (OR Q TO EXIT)" << std::endl;
		std::string in;
		std::cin >> in;
		if (in == "Q" || in == "q") {
			std::cout << "TERMINATING PROCESS" << std::endl;
			break;
		}
		if ((ms.count(in) == 0) || (ms[in]->LoadOnly())) {
			std::cout << "CANNOT EXECUTE MODULE" << std::endl;
		} else {
			Module* m = ms[in];
			LoadModuleIfRequired(m);
			m->Exec();
		};
	};
};

ModuleCoordinator & ModuleCoordinator::Get() {
	static ModuleCoordinator IC;
	return IC;
};

void ModuleCoordinator::Register(Module * i) {
	iqs.insert(i);
};

void ModuleCoordinator::Unregister(Module * i) {
	iqs.erase(i);
};

void ModuleCoordinator::RegisterConstructor(const std::string& sid, Module*(*fptr)()) {
	cs[sid] = fptr;
};

void ModuleCoordinator::RegisterDependency(const std::string & module, const std::string & dep) {
	deps[module].push_back(dep);
};

RockDatabase & ModuleCoordinator::RequestDB(const std::string & S) {
	return Get().dbs[S];
};

void ModuleCoordinator::LoadModuleIfRequired(const std::string & S) {
	//Loads a given database module, if it hasn't been loaded already
	auto it = Get().ms.find(S);
	if (it == Get().ms.end()) {
		throw std::runtime_error("ModuleCoordinator::LoadModuleIfRequired| CANNOT LOAD MODULE '" + S+"'");
	};
	Get().LoadModuleIfRequired(it->second);
};

#ifdef PYTHON_LIB
#include "pyLib.h"
PYTHON_LINK_EXEC(moduleCoordinator_Initialise) {
	ModuleCoordinator::Get().Initialise();
};
#endif