#include "stdafx.h"
#include "RockSample.h"

double NaN= std::numeric_limits<double>::quiet_NaN();

RockSample::RockSample()
: Latitude(NaN), Longitude(NaN), Altitude(NaN), AppliedWeight(NaN), Aluminium(NaN), Titanium(NaN), Silicate(NaN), Magnesium(NaN),
  IronTotal(NaN), Manganese(NaN), Calcium(NaN), Sodium(NaN), Potassium(NaN), Phosphorous(NaN), Effective_FeO(NaN),
  Rb(NaN), Sr(NaN), Zr(NaN), Ta(NaN), Hf(NaN), Nb(NaN),  Y(NaN), Ho(NaN), La(NaN), Th(NaN), Ce(NaN), Dy(NaN), Er(NaN),
  Eu(NaN), Gd(NaN), Lu(NaN), Nd(NaN), Pr(NaN), Sm(NaN), Sc(NaN), Tb(NaN), Tm(NaN), Yb(NaN),  U(NaN), Ni(NaN), Co(NaN),
  Cr(NaN), Ba(NaN),  V(NaN), Zn(NaN), Cu(NaN), Pb(NaN), Iron2(NaN), Iron3(NaN), Pt(NaN), Pd(NaN), Tl(NaN),
  d49Ti(NaN), compositeN(1), Age(NaN), AgeRange(NaN), Intrusive(false), Extrusive(false), RockType(nullptr), RockName(nullptr), Era(nullptr) {};

RockSample::elementalSuite RockSample::allNumericValues;
RockSample::elementalSuite RockSample::allIsotopes;
RockSample::elementalSuite RockSample::allElements;
RockSample::elementalSuite RockSample::majorElements;
RockSample::elementalSuite RockSample::REEs;
RockSample::elementalSuite RockSample::Actinides;
RockSample::elementalSuite RockSample::HFSE;
RockSample::elementalSuite RockSample::Alkalis;
RockSample::elementalSuite RockSample::transitionMetals;
RockSample::elementalSuite RockSample::PGE;
RockSample::elementalSuite RockSample::selectRatios;
struct InitMemberFields {
	InitMemberFields() {
		RockSample::majorElements["K2O"] = OFF(RockSample::Potassium);
		RockSample::majorElements["SiO2"] = OFF(RockSample::Silicate);
		RockSample::majorElements["Na2O"] = OFF(RockSample::Sodium);
		RockSample::majorElements["Al2O3"] = OFF(RockSample::Aluminium);
		RockSample::majorElements["TiO2"] = OFF(RockSample::Titanium);
		RockSample::majorElements["P2O5"] = OFF(RockSample::Phosphorous);
		RockSample::majorElements["MnO"] = OFF(RockSample::Manganese);
		RockSample::majorElements["FeOt"] = OFF(RockSample::Effective_FeO);
		RockSample::majorElements["CaO"] = OFF(RockSample::Calcium);
		RockSample::majorElements["MgO"] = OFF(RockSample::Magnesium);

		RockSample::REEs["La"] = OFF(RockSample::La);
		RockSample::REEs["Ce"] = OFF(RockSample::Ce);
		RockSample::REEs["Pr"] = OFF(RockSample::Pr);
		RockSample::REEs["Nd"] = OFF(RockSample::Nd);
		RockSample::REEs["Sm"] = OFF(RockSample::Sm);
		RockSample::REEs["Eu"] = OFF(RockSample::Eu);
		RockSample::REEs["Gd"] = OFF(RockSample::Gd);
		RockSample::REEs["Tb"] = OFF(RockSample::Tb);
		RockSample::REEs["Dy"] = OFF(RockSample::Dy);
		RockSample::REEs["Ho"] = OFF(RockSample::Ho);
		RockSample::REEs["Er"] = OFF(RockSample::Er);
		RockSample::REEs["Tm"] = OFF(RockSample::Tm);
		RockSample::REEs["Yb"] = OFF(RockSample::Yb);
		RockSample::REEs["Lu"] = OFF(RockSample::Lu);
		RockSample::REEs["Y"] = OFF(RockSample::Y);

		RockSample::Actinides["Tl"] = OFF(RockSample::Tl);
		RockSample::Actinides["Pb"] = OFF(RockSample::Pb);
		RockSample::Actinides["Th"] = OFF(RockSample::Th);
		RockSample::Actinides["U"] = OFF(RockSample::U);

		RockSample::HFSE["TiO2"] = OFF(RockSample::Titanium);
		RockSample::HFSE["Zr"] = OFF(RockSample::Zr);
		RockSample::HFSE["Hf"] = OFF(RockSample::Hf);
		RockSample::HFSE["Nb"] = OFF(RockSample::Nb);
		RockSample::HFSE["Ta"] = OFF(RockSample::Ta);

		RockSample::Alkalis["Rb"] = OFF(RockSample::Rb);
		RockSample::Alkalis["Sr"] = OFF(RockSample::Sr);
		RockSample::Alkalis["Ba"] = OFF(RockSample::Ba);

		RockSample::transitionMetals["Sc"] = OFF(RockSample::Sc);
		RockSample::transitionMetals["V"] = OFF(RockSample::V);
		RockSample::transitionMetals["Cr"] = OFF(RockSample::Cr);
		RockSample::transitionMetals["Co"] = OFF(RockSample::Co);
		RockSample::transitionMetals["Ni"] = OFF(RockSample::Ni);
		RockSample::transitionMetals["Cu"] = OFF(RockSample::Cu);
		RockSample::transitionMetals["Zn"] = OFF(RockSample::Zn);

		RockSample::PGE["Pt"] = OFF(RockSample::Pt);
		RockSample::PGE["Pd"] = OFF(RockSample::Pd);

		RockSample::allElements.MergeIn(RockSample::majorElements);
		RockSample::allElements.MergeIn(RockSample::REEs);
		RockSample::allElements.MergeIn(RockSample::Actinides);
		RockSample::allElements.MergeIn(RockSample::HFSE);
		RockSample::allElements.MergeIn(RockSample::Alkalis);
		RockSample::allElements.MergeIn(RockSample::transitionMetals);
		RockSample::allElements.MergeIn(RockSample::PGE);

		RockSample::allIsotopes["d49Ti"] = OFF(RockSample::d49Ti);

		RockSample::allNumericValues.MergeIn(RockSample::allElements);
		RockSample::allNumericValues.MergeIn(RockSample::allIsotopes);
		RockSample::allNumericValues["Age"] = OFF(RockSample::Age);
		RockSample::allNumericValues["AgeRange"] = OFF(RockSample::AgeRange);
		RockSample::allNumericValues["Longitude"] = OFF(RockSample::Longitude);
		RockSample::allNumericValues["Latitude"] = OFF(RockSample::Latitude);
	};
};
InitMemberFields _staticInit0;

void RockSample::PrintInline() const {
	for (auto& e : RockSample::allElements) {
		std::cout << "[" << e.first << ": " << (e.second).Data(*this) << "] ";
	};
	std::cout << std::endl;
};
RockSample RockSample::InterpolateElements(double f, const RockSample & A, const RockSample & B) {
	RockSample R;

	for (auto& E : RockSample::allElements) {
		E.second.DataR(R) = (1 - f) * E.second.Data(A) + f * E.second.Data(B);
	};

	return R;
};
