#include "stdafx.h"
#include "moduleCommon.h"

REGISTER_DB_MODULE(CommonDBs);


void CommonDBs::Exec() {
	KellerDatabase db_keller_unfiltered;
	db_keller_unfiltered.SetName("Keller 2015").ParseDirectory("db/keller_web");

	StandardGeochemDatabase db_modernIgn_noOcean;
	db_modernIgn_noOcean.SetName("Igneous continental").ParseDirectory("db/PetDB_NoOcean");
	db_modernIgn_noOcean.Refilter(RangeFilter(OFF(RockSample::Age), 0, 1000));

	RockDatabase db_keller(db_keller_unfiltered.Select(FuncFilter(IsGood)));

	db("ign_nomorb") = db_modernIgn_noOcean;
	db("keller") = db_keller;
	db("keller_unfiltered") = db_keller_unfiltered;
};

REGISTER_DB_MODULE(ShaleDB);

bool IsGood_ArchaShale(const RockSample& R) {
	if (!std::isfinite(R.Age)) return false;
	if (!isGood_CharacTest(R)) return false;
	return true;
};

void ShaleDB::Exec() {
	GenericRockDatabase db_shales;
	db_shales.RegisterEntry("SiO2", OFF(RockSample::Silicate)).RegisterEntry("Age", OFF(RockSample::Age)).IgnoreValue("x");//.RegisterEntry("compositeN", OFF(RockSample::compositeN));
	db_shales.RegisterEntry("TiO2", OFF(RockSample::Titanium)).RegisterEntry("CaO", OFF(RockSample::Calcium)).RegisterEntry("Zr", OFF(RockSample::Zr)).RegisterEntry("Sc", OFF(RockSample::Sc));
	db_shales.RegisterEntry("Al2O3", OFF(RockSample::Aluminium)).RegisterEntry("V", OFF(RockSample::V)).RegisterEntry("Cr", OFF(RockSample::Cr)).RegisterEntry("Y", OFF(RockSample::Y));
	db_shales.RegisterEntry("Th", OFF(RockSample::Th)).RegisterEntry("Co", OFF(RockSample::Co)).RegisterEntry("Ni", OFF(RockSample::Ni)).RegisterEntry("Cu",OFF(RockSample::Cu)).RegisterEntry("Pb", OFF(RockSample::Pb));
	db_shales.RegisterEntry("Ta", OFF(RockSample::Ta)).RegisterEntry("Nb", OFF(RockSample::Nb)).RegisterEntry("Hf", OFF(RockSample::Hf)).RegisterEntry("Zn", OFF(RockSample::Zn)).RegisterEntry("Mn", OFF(RockSample::Manganese));
	db_shales.RegisterEntry("La", OFF(RockSample::La)).RegisterEntry("Ce", OFF(RockSample::Ce)).RegisterEntry("Pr", OFF(RockSample::Pr)).RegisterEntry("Nd", OFF(RockSample::Nd)).RegisterEntry("Sm", OFF(RockSample::Sm));
	db_shales.RegisterEntry("Eu", OFF(RockSample::Eu)).RegisterEntry("Gd", OFF(RockSample::Gd)).RegisterEntry("Tb", OFF(RockSample::Tb)).RegisterEntry("Dy", OFF(RockSample::Dy)).RegisterEntry("Ho", OFF(RockSample::Ho));
	db_shales.RegisterEntry("Er", OFF(RockSample::Er)).RegisterEntry("Tm", OFF(RockSample::Tm)).RegisterEntry("Yb", OFF(RockSample::Yb)).RegisterEntry("Lu", OFF(RockSample::Lu));
	db_shales.SetName("Global shales").ParseDirectory("AllShales");
	db_shales.Refilter(FuncFilter(IsGood_ArchaShale));
	db("shales") = db_shales;
};
