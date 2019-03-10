#include "stdafx.h"
#include "SpecialisedRockDatabase.h"

//Helper functions for parsing the giant mess that is iron concentration data
bool IsValid_IronInput(double Fe2O3t, double FeOt, double Fe2O3, double FeO) {
	if (std::isfinite(Fe2O3t) && std::isfinite(FeOt)) {
		return true;
	} else if (std::isfinite(Fe2O3t)) {
		return true;
	} else if (std::isfinite(FeOt)) {
		return true;
	} else if (std::isfinite(Fe2O3) && std::isfinite(FeO)) {
		return true;
	} else if (std::isfinite(Fe2O3)) {
		return true;
	} else if (std::isfinite(FeO)) {
		return true;
	};
	return false;
};

double Compute_IronTotal(double Fe2O3t, double FeOt, double Fe2O3, double FeO) {
	if (std::isfinite(Fe2O3t) && std::isfinite(FeOt)) {
		return Fe2O3t;
	} else if (std::isfinite(Fe2O3t)) {
		return Fe2O3t;
	} else if (std::isfinite(FeOt)) {
		return FeOt;
	} else if (std::isfinite(Fe2O3) && std::isfinite(FeO)) {
		return 1.111*FeO + Fe2O3;
	} else if (std::isfinite(Fe2O3)) {
		return Fe2O3;
	} else if (std::isfinite(FeO)) {
		return FeO;
	};
	return std::numeric_limits<double>::infinity();
};

double Compute_EffectiveFeO(double Fe2O3t, double FeOt, double Fe2O3, double FeO) {
	if (std::isfinite(Fe2O3t) && std::isfinite(FeOt)) {
		return Fe2O3t / 1.1113;
	} else if (std::isfinite(Fe2O3t)) {
		return Fe2O3t / 1.1113;
	} else if (std::isfinite(FeOt)) {
		return FeOt;
	} else if (std::isfinite(Fe2O3) && std::isfinite(FeO)) {
		return FeO + Fe2O3 / 1.1113;
	} else if (std::isfinite(Fe2O3)) {
		return Fe2O3 / 1.1113;
	} else if (std::isfinite(FeO)) {
		return FeO;
	};
	return std::numeric_limits<double>::infinity();
};

bool KellerIronParser(RockSample& R, csvParser& P, int64 ROW) {
	auto cFe2O3t = P.GetCachedColumnByTitle("Fe2O3T");
	auto cFeOt = P.GetCachedColumnByTitle("FeOT");
	auto cFe2O3 = P.GetCachedColumnByTitle("Fe2O3");
	auto cFeO = P.GetCachedColumnByTitle("FeO");
	std::string sFe2O3t = (cFe2O3t >= 0) ? P.Get(ROW, cFe2O3t) : "";
	std::string sFe2O3 = (cFe2O3 >= 0) ? P.Get(ROW, cFe2O3) : "";
	std::string sFeOt = (cFeOt >= 0) ? P.Get(ROW, cFeOt) : "";
	std::string sFeO = (cFeO >= 0) ? P.Get(ROW, cFeO) : "";
	if (sFe2O3t == "NaN") sFe2O3t = "";
	if (sFe2O3 == "NaN") sFe2O3t = "";
	if (sFeOt == "NaN") sFe2O3t = "";
	if (sFeO == "NaN") sFe2O3t = "";
	double Fe2O3t = (sFe2O3t.size()>0) ? StringToData<double>(P.Get(ROW, cFe2O3t)) : std::numeric_limits<double>::infinity();
	double FeOt = (sFeOt.size() > 0) ? StringToData<double>(P.Get(ROW, cFeOt)) : std::numeric_limits<double>::infinity();
	double Fe2O3 = (sFe2O3.size() > 0) ? StringToData<double>(P.Get(ROW, cFe2O3)) : std::numeric_limits<double>::infinity();
	double FeO = (sFeO.size() > 0) ? StringToData<double>(P.Get(ROW, cFeO)) : std::numeric_limits<double>::infinity();

	R.IronTotal = Compute_IronTotal(Fe2O3t,FeOt, Fe2O3, FeO);
	R.Effective_FeO = Compute_EffectiveFeO(Fe2O3t, FeOt, Fe2O3, FeO);
	R.Iron2 = FeO;
	R.Iron3 = Fe2O3;
	return IsValid_IronInput(Fe2O3t, FeOt, Fe2O3, FeO);
};

bool KellerIntExtParser(RockSample& R, csvParser& P, int64 ROW) {
	auto col = P.GetCachedColumnByTitle("TYPE");
	std::string val = P.Get(ROW, col);
	if (val == "PLUTONIC") {
		R.Intrusive = true;
		return true;
	};
	if (val == "VOLCANIC") {
		R.Extrusive = true;
		return true;
	};
	return false;
};

KellerDatabase::KellerDatabase() {
	RegisterEntry("SiO2", OFF(RockSample::Silicate)).RegisterEntry("Age", OFF(RockSample::Age)).RegisterEntry("Rock_Name", OFF(RockSample::RockName));
	RegisterEntry("TiO2", OFF(RockSample::Titanium)).RegisterCustomParser(KellerIronParser).RegisterCustomParser(KellerIntExtParser);
	RegisterEntry("Al2O3", OFF(RockSample::Aluminium)).RegisterEntry("MgO", OFF(RockSample::Magnesium));
	RegisterEntry("MnO", OFF(RockSample::Manganese)).RegisterEntry("CaO", OFF(RockSample::Calcium));
	RegisterEntry("Na2O", OFF(RockSample::Sodium)).RegisterEntry("K2O", OFF(RockSample::Potassium));
	RegisterEntry("P2O5", OFF(RockSample::Phosphorous)).RegisterEntry("Co", OFF(RockSample::Co));
	RegisterEntry("Ni", OFF(RockSample::Ni)).RegisterEntry("Zr", OFF(RockSample::Zr)).RegisterEntry("La", OFF(RockSample::La));
	RegisterEntry("Th", OFF(RockSample::Th)).RegisterEntry("Rb", OFF(RockSample::Rb)).RegisterEntry("Sr", OFF(RockSample::Sr));
	RegisterEntry("Y", OFF(RockSample::Y)).RegisterEntry("Ho", OFF(RockSample::Ho)).RegisterEntry("Ce", OFF(RockSample::Ce)).RegisterEntry("Dy", OFF(RockSample::Dy));
	RegisterEntry("Er", OFF(RockSample::Er)).RegisterEntry("Eu", OFF(RockSample::Eu)).RegisterEntry("Gd", OFF(RockSample::Gd)).RegisterEntry("Lu", OFF(RockSample::Lu));
	RegisterEntry("Nd", OFF(RockSample::Nd)).RegisterEntry("Pr", OFF(RockSample::Pr)).RegisterEntry("Sm", OFF(RockSample::Sm)).RegisterEntry("Cu", OFF(RockSample::Cu));
	RegisterEntry("Sc", OFF(RockSample::Sc)).RegisterEntry("Tb", OFF(RockSample::Tb)).RegisterEntry("Tm", OFF(RockSample::Tm)).RegisterEntry("Yb", OFF(RockSample::Yb));
	RegisterEntry("U", OFF(RockSample::U)).RegisterEntry("Ta", OFF(RockSample::Ta)).RegisterEntry("Nb", OFF(RockSample::Nb)).RegisterEntry("Hf", OFF(RockSample::Hf));
	RegisterEntry("Cr", OFF(RockSample::Cr)).RegisterEntry("Ba", OFF(RockSample::Ba)).RegisterEntry("V", OFF(RockSample::V)).RegisterEntry("Pb", OFF(RockSample::Pb));
	RegisterEntry("Zn", OFF(RockSample::Zn));
	RegisterEntry("Pt", OFF(RockSample::Pt)).RegisterEntry("Pd", OFF(RockSample::Pd)).RegisterEntry("Tl",OFF(RockSample::Tl));
	IgnoreValue("NaN");
};

StandardGeochemDatabase::StandardGeochemDatabase() {
	RegisterEntry("SiO2", OFF(RockSample::Silicate)).RegisterEntry("Age", OFF(RockSample::Age)).IgnoreValue("x");//.RegisterEntry("compositeN", OFF(RockSample::compositeN));
	RegisterEntry("TiO2", OFF(RockSample::Titanium)).RegisterEntry("CaO", OFF(RockSample::Calcium)).RegisterEntry("Zr", OFF(RockSample::Zr)).RegisterEntry("Sc", OFF(RockSample::Sc));
	RegisterEntry("Al2O3", OFF(RockSample::Aluminium)).RegisterEntry("V", OFF(RockSample::V)).RegisterEntry("Cr", OFF(RockSample::Cr)).RegisterEntry("Y", OFF(RockSample::Y));
	RegisterEntry("Th", OFF(RockSample::Th)).RegisterEntry("Co", OFF(RockSample::Co)).RegisterEntry("Ni", OFF(RockSample::Ni)).RegisterEntry("Cu", OFF(RockSample::Cu)).RegisterEntry("Pb", OFF(RockSample::Pb));
	RegisterEntry("Ta", OFF(RockSample::Ta)).RegisterEntry("Nb", OFF(RockSample::Nb)).RegisterEntry("Hf", OFF(RockSample::Hf)).RegisterEntry("Zn", OFF(RockSample::Zn)).RegisterEntry("MnO", OFF(RockSample::Manganese));
	RegisterEntry("La", OFF(RockSample::La)).RegisterEntry("Ce", OFF(RockSample::Ce)).RegisterEntry("Pr", OFF(RockSample::Pr)).RegisterEntry("Nd", OFF(RockSample::Nd)).RegisterEntry("Sm", OFF(RockSample::Sm));
	RegisterEntry("Eu", OFF(RockSample::Eu)).RegisterEntry("Gd", OFF(RockSample::Gd)).RegisterEntry("Tb", OFF(RockSample::Tb)).RegisterEntry("Dy", OFF(RockSample::Dy)).RegisterEntry("Ho", OFF(RockSample::Ho));
	RegisterEntry("Er", OFF(RockSample::Er)).RegisterEntry("Tm", OFF(RockSample::Tm)).RegisterEntry("Yb", OFF(RockSample::Yb)).RegisterEntry("Lu", OFF(RockSample::Lu));
};
