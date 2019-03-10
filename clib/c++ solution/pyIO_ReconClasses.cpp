#include "stdafx.h"
#include "pyLib.h"
#include "reconManager.h"

//Define common reconstruction-related types that can be passed to the Python interface
#ifdef PYTHON_LIB
PYTHON_LINK_EXEC(pyIO_ReconClasses) {
	using namespace boost::python;
	class_<MixState<2>>("MixState<2>")
		.def("__getitem__", &MixState<2>::get_endmember_value)
		.def("__setitem__", &MixState<2>::set_endmember_value);

	class_<MixState<3>>("MixState<3>")
		.def("__getitem__", &MixState<3>::get_endmember_value)
		.def("__setitem__", &MixState<3>::set_endmember_value);

	class_<MixState<4>>("MixState<4>")
		.def("__getitem__", &MixState<4>::get_endmember_value)
		.def("__setitem__", &MixState<4>::set_endmember_value);

	class_< std::vector<double> >("vectorDouble")
		.def(vector_indexing_suite< std::vector<double> >());

	class_< std::vector<std::string> >("vectorString")
		.def(vector_indexing_suite< std::vector<std::string> >());

	class_<RockSample>("RockSample")
		.def_readwrite("Age", &RockSample::Age)
		.def_readwrite("Al2O3", &RockSample::Aluminium)
		.def_readwrite("Ba", &RockSample::Ba)
		.def_readwrite("CaO", &RockSample::Calcium)
		.def_readwrite("Ce", &RockSample::Ce)
		.def_readwrite("Co", &RockSample::Co)
		.def_readwrite("compositeN", &RockSample::compositeN)
		.def_readwrite("Cr", &RockSample::Cr)
		.def_readwrite("Cu", &RockSample::Cu)
		.def_readwrite("Dy", &RockSample::Dy)
		.def_readwrite("FeO", &RockSample::Effective_FeO)
		.def_readwrite("Er", &RockSample::Er)
		.def_readwrite("Era", &RockSample::Era)
		.def_readwrite("Eu", &RockSample::Eu)
		.def_readwrite("Gd", &RockSample::Gd)
		.def_readwrite("Hf", &RockSample::Hf)
		.def_readwrite("Ho", &RockSample::Ho)
		.def_readwrite("La", &RockSample::La)
		.def_readwrite("Lu", &RockSample::Lu)
		.def_readwrite("MgO", &RockSample::Magnesium)
		.def_readwrite("MnO", &RockSample::Manganese)
		.def_readwrite("Nb", &RockSample::Nb)
		.def_readwrite("Nd", &RockSample::Nd)
		.def_readwrite("Ni", &RockSample::Ni)
		.def_readwrite("Pb", &RockSample::Pb)
		.def_readwrite("Pd", &RockSample::Pd)
		.def_readwrite("P2O5", &RockSample::Phosphorous)
		.def_readwrite("K2O", &RockSample::Potassium)
		.def_readwrite("Pr", &RockSample::Pr)
		.def_readwrite("Pt", &RockSample::Pt)
		.def_readwrite("Rb", &RockSample::Rb)
		.def_readwrite("RockName", &RockSample::RockName)
		.def_readwrite("RockType", &RockSample::RockType)
		.def_readwrite("Sc", &RockSample::Sc)
		.def_readwrite("SiO2", &RockSample::Silicate)
		.def_readwrite("Sm", &RockSample::Sm)
		.def_readwrite("NaO2", &RockSample::Sodium)
		.def_readwrite("Sr", &RockSample::Sr)
		.def_readwrite("Ta", &RockSample::Ta)
		.def_readwrite("Tb", &RockSample::Tb)
		.def_readwrite("Th", &RockSample::Th)
		.def_readwrite("TiO2", &RockSample::Titanium)
		.def_readwrite("Tl", &RockSample::Tl)
		.def_readwrite("Tm", &RockSample::Tm)
		.def_readwrite("U", &RockSample::U)
		.def_readwrite("V", &RockSample::V)
		.def_readwrite("Y", &RockSample::Y)
		.def_readwrite("Yb", &RockSample::Yb)
		.def_readwrite("Zn", &RockSample::Zn)
		.def_readwrite("Zr", &RockSample::Zr);

	class_<WRB_Result>("WRB_Result")
		.def_readwrite("bestFit", &WRB_Result::bestFit)
		.def_readwrite("stdError", &WRB_Result::stdError)
		.def("Percentile975", &WRB_Result::Percentile975)
		.def("Percentile025", &WRB_Result::Percentile025);

	class_<DiscreteFunction>("DiscreteFunction")
		.def("__call__", &DiscreteFunction::operator())
		.def("GenerateXList", &DiscreteFunction::GenerateXList)
		.def("GenerateYList", &DiscreteFunction::GenerateYList)
		.def("AddNewPoint", &DiscreteFunction::AddNewPoint)
		.def("Finalise",&DiscreteFunction::Finalise)
		.def("PrintToFile", &DiscreteFunction::PrintToFile)
		.def("Reserve", &DiscreteFunction::Reserve)
		.def("FirstX", &DiscreteFunction::FirstX)
		.def("LastX", &DiscreteFunction::LastX)
		.def("FirstY", &DiscreteFunction::FirstY)
		.def("LastY", &DiscreteFunction::LastY);

	class_<ReconManager>("ReconManager", boost::python::init<boost::python::dict, const std::string&>())
		.def("GenerateAllBootstraps", &ReconManager::GenerateAllBootstraps)
		.def("ResetAllBootstraps", &ReconManager::ResetAllBootstraps)
		.def("AddBootstrap", &ReconManager::AddBootstrap)
		.def("GenerateBootstrap", &ReconManager::GenerateBootstrap)
		.def("DataCountForBootstrap", &ReconManager::DataCountForBootstrap)
		.def("RunReconstruction", &ReconManager::RunReconstruction)
		.def("GetEndmemberName", &ReconManager::GetEndmemberName)
		.def("GetEndmemberCount",&ReconManager::GetEndmemberCount)
		.def("ForwardModelCalc", static_cast<std::vector<double>(ReconManager::*)(double, boost::python::list)const>(&ReconManager::ForwardModelCalc))
		.def("ForwardModelCalc", static_cast<double(ReconManager::*)(double, boost::python::list, const std::string&)const>(&ReconManager::ForwardModelCalc))
		.def("GetNearestValidTime", &ReconManager::GetNearestValidTime);

	class_<SingleTimeState>("SingleTimeState")
		.def("__getitem__", &SingleTimeState::get_endmember_value)
		.def("__setitem__", &SingleTimeState::set_endmember_value)
		.def_readwrite("best", &SingleTimeState::best)
		.def_readwrite("endmemberP025", &SingleTimeState::endmemberP025)
		.def_readwrite("endmemberP975", &SingleTimeState::endmemberP975);

	class_<SingleTimeState::RatioSystem>("STS___RatioSystem")
		.def_readwrite("cA", &SingleTimeState::RatioSystem::cA)
		.def_readwrite("cB", &SingleTimeState::RatioSystem::cB)
		.def_readwrite("bootR", &SingleTimeState::RatioSystem::bootR)
		.def_readwrite("bestR", &SingleTimeState::RatioSystem::bestR)
		.def_readwrite("mcmcR", &SingleTimeState::RatioSystem::mcmcR);
};

//General Python-exported functions
namespace {
	boost::python::dict GetEndmemberRocks(const ReconManager& RM, double t) {
		RM.E->RecalculateForTime(t);
		auto e = RM.E->ExportSamples();

		boost::python::dict d;
		for (size_t i = 0; i < RM.GetEndmemberCount(); ++i) {
			d[RM.GetEndmemberName(i)] = e[i].DataAsCSV(NullFilter());
		};
		return d;
	};
	PYTHON_LINK_FUNCTION(GetEndmemberRocks);

	//Requests HL38 to load a module
	void LoadModule(const std::string& ID) {
		ModuleCoordinator::LoadModuleIfRequired(ID);
	};
	PYTHON_LINK_FUNCTION(LoadModule);

	//Returns a HL38 database in CSV format
	//If the database does not exist, will return an empty CSV
	std::string DatabaseAsCSV(const std::string& ID) {
		return ModuleCoordinator::RequestDB(ID).DataAsCSV(NullFilter());
	};
	PYTHON_LINK_FUNCTION(DatabaseAsCSV);
};

#endif
