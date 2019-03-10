#pragma once
#include "utils.h"
#include "RockDatabase.h"

struct ModuleCoordinator;

struct Module {
	friend ModuleCoordinator;

	virtual void Exec() = 0;

	virtual bool LoadOnly() { return false; };

	Module();
	virtual ~Module();

protected:
	RockDatabase& db(const std::string&) const;

private:
	std::string sid;
};

struct ModuleCoordinator {
	void ProcessAll();
	void Initialise();

	static ModuleCoordinator& Get();

	void Register(Module*);
	void Unregister(Module*);

	void RegisterConstructor(const std::string&, Module*(*fptr)());
	void RegisterDependency(const std::string&, const std::string&);

	static RockDatabase& RequestDB(const std::string&);
	static void LoadModuleIfRequired(const std::string&);
private:
	void LoadModuleIfRequired(Module * i);
	std::set<Module*> loadedModules;
	std::set<Module*> iqs;
	std::map<std::string, RockDatabase> dbs;
	std::map<std::string, std::vector<std::string>> deps;
	std::map<std::string, Module*(*)()> cs;
	std::map<std::string, Module*> ms;
};

template<typename T>
Module* MCONSTRUCTOR() {
	return (Module*)(new T());
};

template<typename T>
struct ModuleInitiator {
	ModuleInitiator(const std::string& S) {
		ModuleCoordinator::Get().RegisterConstructor(S, &MCONSTRUCTOR<T>);
	};
};

template<typename T>
struct ModuleDependencyMarker {
	ModuleDependencyMarker(const std::string& MODULE, const std::string& DEP) {
		ModuleCoordinator::Get().RegisterDependency(MODULE, DEP);
	};
};

//Register an executable module (executes multiple times)
#define REGISTER_MODULE(id) struct id : public Module { virtual void Exec() override; }; static ModuleInitiator<id> zmi_##id(#id)

//Register a database module (only executes once)
#define REGISTER_DB_MODULE(id) struct id : public Module { virtual void Exec() override; virtual bool LoadOnly() override final { return true; }; }; static ModuleInitiator<id> zmi_##id(#id)

//Register a 'dependency' - i.e. one module always has to load before another
#define MODULE_DEPENDENCY(id, dep) static ModuleDependencyMarker<id> zmi_dm_##id##dep(#id,#dep)

