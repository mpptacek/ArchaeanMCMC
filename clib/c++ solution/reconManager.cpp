#include "stdafx.h"
#include "reconManager.h"

MemberOffset<RockSample, double> ReconManager::TranslateOffset(const std::string & sysName) {
	return MemberOffset<RockSample, double>((MemberOffsetBase)RockSample::allElements[sysName]);
};

WRB_Result ReconManager::GenerateBootstrapIMPL(const MemberOffset<RockSample, double>& A, const MemberOffset<RockSample, double>& B) {
	//Strip NaNs
	auto ageList = shales.ExtractList(OFF(RockSample::Age));
	auto AList = shales.ExtractList(A);
	auto BList = shales.ExtractList(B);
	std::vector<double> ageListFiltered, AListFiltered, BListFiltered;
	for (size_t j = 0; j < ageList.size(); ++j) {
		if (std::isfinite(ageList[j]) && std::isfinite(AList[j]) && std::isfinite(BList[j])) {
			ageListFiltered.push_back(ageList[j]);
			AListFiltered.push_back(AList[j]);
			BListFiltered.push_back(BList[j]);
		};
	};
	//Bootstrap
	return WRB_Bootstrap(ageListFiltered, AListFiltered, BListFiltered, kernelWidth);
};

void ReconManager::AddRatio(const std::string & RNAME, MemberOffset<RockSample, double> NOM, MemberOffset<RockSample, double> DNM) {
	nameR.push_back(RNAME);
	Nmntr.push_back(NOM);
	Dmntr.push_back(DNM);
};

size_t ReconManager::CountRatios() const {
	return nameR.size();
};

void ReconManager::AddBootstrap(const WRB_Result & r) {
	bestF.push_back(r.bestFit);
	errMF.push_back(r.stdError);
};

size_t ReconManager::DataCountForBootstrap(const std::string & A, const std::string & B) {
	//Returns how many datapoints our shale database contains for a given ratio (after stripping NaNs)
	auto a = TranslateOffset(A);
	auto b = TranslateOffset(B);

	//Strip NaNs
	auto ageList = shales.ExtractList(OFF(RockSample::Age));
	auto AList = shales.ExtractList(a);
	auto BList = shales.ExtractList(b);
	std::vector<double> ageListFiltered, AListFiltered, BListFiltered;
	//Count non-NAN, complete values in database
	size_t count = 0;
	for (size_t j = 0; j < ageList.size(); ++j) {
		bool test = (std::isfinite(ageList[j]) && std::isfinite(AList[j]) && std::isfinite(BList[j]));
		if (test) {
			++count;
		};
	};

	return count;
};

WRB_Result ReconManager::GenerateBootstrap(const std::string & A, const std::string & B) {
	return GenerateBootstrapIMPL(TranslateOffset(A), TranslateOffset(B));
};

void ReconManager::GenerateAllBootstraps() {
	for (size_t i = 0; i < CountRatios(); ++i) {
		AddBootstrap(GenerateBootstrapIMPL(Nmntr[i], Dmntr[i]));
	};
};

void ReconManager::ResetAllBootstraps() {
	bestF.clear();
	errMF.clear();
};

ReconManager::ReconManager(DenseStringMap conf, const std::string & DB) : kernelWidth(StringToData<double>(conf["BootstrapKernelWidth"][0])), initConfig(conf) {
	//Load shale database (either from standard folder, or from supplied string)
	StandardGeochemDatabase db_shales;
	db_shales.SetName("Filtered global shales");
	if (DB == "") {
		db_shales.ParseDirectory("filt-shales-A");
	} else {
		db_shales.ParseString(DB);
	};
	shales.Merge(db_shales);

	//Parse our configuration list and define our system
	if (!conf["rA"].empty()) {
		for (size_t i = 0; i < conf["rA"].size(); ++i) {
			std::string strA = conf["rA"][i];
			std::string strB = conf["rB"][i];
			AddRatio(strA + "/" + strB, TranslateOffset(strA), TranslateOffset(strB));
		};
	};

	//Load databases required for endmember computation
	ModuleCoordinator::LoadModuleIfRequired("CommonDBs");
	parsedDB_Keller = &ModuleCoordinator::RequestDB("keller");
	parsedDB_nomorb = &ModuleCoordinator::RequestDB("ign_nomorb");

	//Select endmembers, based on the configuration file
	std::string endMode = conf["endmemberMode"][0];
	std::string endScript = conf["endmemberScript"][0];
	if (endMode == "Dual") {
		E = new DualEndmembers(endScript, *parsedDB_Keller, *parsedDB_nomorb);
	} else if (endMode == "Continuous") {
		double ABwidth = StringToData<double>(conf["AgeBinWidth"][0]);
		E = new ContinuousEndmembers(endScript, *parsedDB_Keller, *parsedDB_nomorb, ABwidth);
	} else if (endMode == "ModernOnly") {
		E = new DualEndmembers(endScript, *parsedDB_Keller, *parsedDB_nomorb, 99999.9, 0.1);
	} else if (endMode == "Exponential") {
		double ABwidth = StringToData<double>(conf["AgeBinWidth"][0]);
		double Kwidth = StringToData<double>(conf["KernelWidth"][0]);
		E = new ExponentialEndmembers(endScript, *parsedDB_Keller, *parsedDB_nomorb, ABwidth, Kwidth);
	} else if (endMode == "FuturePast") {
		double ABwidth = StringToData<double>(conf["AgeBinWidth"][0]);
		double Kwidth = StringToData<double>(conf["KernelWidth"][0]);
		E = new FuturePastEndmembers(endScript, *parsedDB_Keller, *parsedDB_nomorb, ABwidth, Kwidth);
	} else if (endMode == "Bootstrap") {
		double Kwidth = StringToData<double>(conf["KernelWidth"][0]);
		auto* EPTR = new BoMembers(endScript, *parsedDB_Keller, *parsedDB_nomorb, Kwidth);
		E = EPTR;
	} else {
		throw new std::runtime_error("Unrecognised endmember mode '" + endMode + "'");
	};

	//Select reconstruction program to execute
	execRecon = nullptr;
	std::string reconMode = conf["reconMode"][0];
	if (!conf.Contains("detailedRatioPrinter")) {
		//Standard reporting mode (endmember confidence intervals)
		if (reconMode == "MCMC") {
			switch(E->N_e) {
			case 2:
				execRecon = &MCMCRecon::RunMarkovModel_2M;
				break;
			case 3:
				execRecon = &MCMCRecon::RunMarkovModel_3M;
				break;
			case 4:
				execRecon = &MCMCRecon::RunMarkovModel_4M;
				break;
			case 5:
				execRecon = &MCMCRecon::RunMarkovModel_5M;
				break;
			};
		};
	} else {
		//Ratio reporting mode (ratio confidence intervals)
		if (reconMode == "MCMC") {
			switch (E->N_e) {
			case 2:
				execRecon = &MCMCRecon::RunMarkovModel_2M_Ratios;
				break;
			case 3:
				execRecon = &MCMCRecon::RunMarkovModel_3M_Ratios;
				break;
			case 4:
				execRecon = &MCMCRecon::RunMarkovModel_4M_Ratios;
				break;
			case 5:
				execRecon = &MCMCRecon::RunMarkovModel_5M_Ratios;
				break;
			};
		};
	};

	//Register the ratios which we will use with the endmembers manager, to allow for computation of ratio errors
	for (size_t i = 0; i < CountRatios(); ++i) {
		E->RegisterRatioError(Nmntr[i], Dmntr[i]);
	};
}

double ReconManager::GetNearestValidTime(double start_time, bool scan_forward) const {
	const double step_size = 1.0;
	double step = (scan_forward) ? +step_size : -step_size;

	for (double t = start_time;; t += step_size) {
		for (size_t i = 0; i < CountRatios(); ++i) {
			if (!(std::isfinite(bestF[i](t)) &&
				  std::isfinite(errMF[i](t)))) {
				continue;
			};
		};
		return t;
	};
	return NAN;
};

std::string ReconManager::RunReconstruction() const {
	return execRecon(*this);
};

std::vector<double> ReconManager::ForwardModelCalc(double t, const std::vector<double>& p) const {
	std::vector<double> rVal(CountRatios());

	E->RecalculateForTime(t);

	for (size_t i = 0; i < CountRatios(); ++i) {
		double sumA = 0.0;
		double sumB = 0.0;
		for (size_t j = 0; j < GetEndmemberCount(); ++j) {
			sumA += p[j] * Nmntr[i](E->E[j]);
			sumB += p[j] * Dmntr[i](E->E[j]);
		};
		rVal[i] = sumA / sumB;
	};

	return rVal;
};

double ReconManager::ForwardModelCalc(double t, const std::vector<double>& p, MemberOffset<RockSample, double> el) const {
	E->RecalculateForTime(t);

	double sumA = 0.0;
	double sumB = 0.0;
	for (size_t j = 0; j < GetEndmemberCount(); ++j) {
		sumA += p[j] * el(E->E[j]);
		sumB += p[j] * el(E->E[j]);
	};
	return sumA / sumB;
};

std::string ReconManager::GetEndmemberName(size_t idx) const {
	return E->Ename[idx];
};

size_t ReconManager::GetEndmemberCount() const {
	return E->E.size();
};

#ifdef PYTHON_LIB
std::vector<double> ReconManager::ForwardModelCalc(double t, boost::python::list p) const {
	auto pVect = PyList2Vect<double>(p);
	return ForwardModelCalc(t, pVect);
};
double ReconManager::ForwardModelCalc(double t, boost::python::list p, const std::string& EL) const {
	auto pVect = PyList2Vect<double>(p);
	return ForwardModelCalc(t, pVect, MemberOffset<RockSample, double>(RockSample::allElements[EL]));
};
#endif
