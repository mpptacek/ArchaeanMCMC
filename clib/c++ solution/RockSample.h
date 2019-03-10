#pragma once
typedef double WeightPct;
typedef double Coord;
typedef double Ppm;
typedef double Ratio;
typedef double Myr;

struct RockSample {
	typedef VectorMap< std::string, DataOffset<RockSample, double> > elementalSuite;
	static elementalSuite allNumericValues;
	static elementalSuite allIsotopes;
	static elementalSuite allElements;
	static elementalSuite majorElements;
	static elementalSuite transitionMetals;
	static elementalSuite Alkalis;
	static elementalSuite REEs;
	static elementalSuite Actinides;
	static elementalSuite HFSE;
	static elementalSuite PGE;
	static elementalSuite selectRatios;
	
	Coord			Latitude;
	Coord			Longitude;
	double			Altitude;
	WeightPct		Titanium;
	WeightPct		Silicate;
	WeightPct		Aluminium;
	WeightPct		Magnesium;
	WeightPct		IronTotal;
	WeightPct		Manganese;
	WeightPct		Calcium;
	WeightPct		Sodium;
	WeightPct		Potassium;
	WeightPct		Phosphorous;
	WeightPct		Effective_FeO;
	WeightPct		Iron2;
	WeightPct		Iron3;
	Ppm				Rb;
	Ppm				Sr;
	Ppm				Zr;
	Ppm				Ta;
	Ppm				Hf;
	Ppm				Nb;
	Ppm				Y;
	Ppm				Ho;
	Ppm				La;
	Ppm				Th;
	Ppm				Ce;
	Ppm				Dy;
	Ppm				Er;
	Ppm				Eu;
	Ppm				Gd;
	Ppm				Lu;
	Ppm				Nd;
	Ppm				Pr;
	Ppm				Sm;
	Ppm				Sc;
	Ppm				Tb;
	Ppm				Tm;
	Ppm				Yb;
	Ppm				U;
	Ppm				Ni;
	Ppm				Co;
	Ppm				Cr;
	Ppm				Ba;
	Ppm				V;
	Ppm				Zn;
	Ppm				Cu;
	Ppm				Pb;
	Ppm				Pt;
	Ppm				Pd;
	Ppm				Tl;
	std::string*	RockType;
	std::string*	RockName;
	std::string*	Era;
	bool			Intrusive;
	bool			Extrusive;
	Myr				Age;
	Myr				AgeRange;
	Ratio			d49Ti;
	unsigned int	compositeN;
	
	double			AppliedWeight;

	RockSample();
	void PrintInline() const;

	//Returns (1-f)*A + f*B
	//Only considers elements, not isotopes or other numeric values
	static RockSample InterpolateElements(double f, const RockSample& A, const RockSample& B);
};
