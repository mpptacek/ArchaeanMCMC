#include "stdafx.h"
#include "reconEndmembers.h"

//General mafic endmember
bool EndmemberM(const RockSample& R) {
	if (R.Silicate>52)
		return false;
	if (R.Silicate<45)
		return false;
	if (R.Magnesium>18)
		return false;
	return true;
};
//General felsic endmember
bool EndmemberF(const RockSample& R) {
	if (R.Silicate>80)
		return false;
	if (R.Silicate<63)
		return false;
	return true;
};
//General komatiitic endmember
bool EndmemberU(const RockSample& R) {
	if (R.Magnesium < 18) return false;
	if (R.RockName == nullptr) {
		return false;
	};
	if (R.RockName->find("KOMATIITE [") == std::string::npos) {
		if ((*R.RockName) != "KOMATIITE") {
			return false;
		};
	};
	return true;
};

class TholeiiticLine {
	DiscreteFunction line;
public:
	static const DiscreteFunction& Get() {
		static TholeiiticLine tl;
		return tl.line;
	};
	TholeiiticLine() {
		//After Rickwood P.C., Lithos, 1989
		//TAS tholeiite/calc-alkali division
		line.AddNewPoint(39.00,  0.0);
		line.AddNewPoint(41.56,  1.0);
		line.AddNewPoint(43.28,  2.0);
		line.AddNewPoint(45.47,  3.0);
		line.AddNewPoint(48.18,  4.0);
		line.AddNewPoint(51.02,  5.0);
		line.AddNewPoint(53.72,  6.0);
		line.AddNewPoint(56.58,  7.0);
		line.AddNewPoint(60.47,  8.0);
		line.AddNewPoint(66.82,  9.0);
		line.AddNewPoint(77.15, 10.0);
		line.Finalise();
	};
};

//Determine if a mafic rock is on the tholeiitic trend
bool EndmemberMTT(const RockSample& R) {
	if (std::isfinite(R.Silicate) && std::isfinite(R.Potassium) && std::isfinite(R.Sodium)) {
		const DiscreteFunction& line = TholeiiticLine::Get();
		double s = R.Silicate;
		double ta = R.Potassium + R.Sodium;
		if (s < line.FirstX()) {
			return false;
		};
		double pred_max_ta = (s < line.LastX()) ? line(s) : line.LastY();

		return (ta < pred_max_ta);
	};
	return false;
};

//Determine if a mafic rock is non-tholeiitic
bool EndmemberMNT(const RockSample& R) {
	if (std::isfinite(R.Silicate) && std::isfinite(R.Potassium) && std::isfinite(R.Sodium)) {
		return !EndmemberMTT(R);
	};
	return false;
};

Endmembers::Endmembers(size_t N_endmembers) : N_e(N_endmembers), E(N_endmembers), Ename(N_endmembers), ratioErr(N_endmembers) {
};

void Endmembers::RegisterRatioError(MemberOffset<RockSample, double> nominator, MemberOffset<RockSample, double> denominator) {
	ratioErr_Nmntr.push_back(nominator);
	ratioErr_Dnmtr.push_back(denominator);
	for (size_t j = 0; j < N_e; ++j) {
		ratioErr[j].push_back(0.0);
	};
};

void Endmembers::RecalculateForTime(double T_NEW) {
	t = T_NEW;
	RecalcEM();
};

Endmembers::~Endmembers() {
};

const double MORB_CUTOFF_AGE = 200.0;
const double MAX_AGE = 9999999.9;

// Given an input database, normalises the sampling density with respect to 
// age; such that, when the database is randomly sampled via weighted
// average, it is equally probable to draw a rock of any age.
// This is done by updating the 'applied weight' parameter.
RockDatabase Endmembers::CreateAgeUniformDatabase(const RockDatabase& inDB, double width) {
	RockDatabase outDB;

	//Find age limits
	double ageMax = NAN;
	double ageMin = NAN;
	for (const auto& R : inDB) {
		if (std::isfinite(R.Age)) {
			if (std::isfinite(ageMax)) {
				if (R.Age > ageMax) {
					ageMax = R.Age;
				};
				if (R.Age < ageMin) {
					ageMin = R.Age;
				};
			} else {
				ageMax = R.Age;
				ageMin = R.Age;
			};
		};
	};

	//Bin everything
	double ageRange = ageMax - ageMin;
	size_t numBins = (size_t)(ceil(ageRange / width)) + 1;
	std::vector<RockSample>* bins = new std::vector<RockSample>[numBins];
	for (const auto& R : inDB) {
		if (std::isfinite(R.Age)) {
			size_t binID = llround((R.Age - ageMin) / width);
			if (binID >= numBins) {
				std::cout << "BINS: " << numBins << std::endl;
				std::cout << "BIN_ID:" << binID;
				throw std::runtime_error("BIN OVERFLOW");
			};
			bins[binID].push_back(R);
		};
	};

	//Compute total bin weight
	double totalMass = 0.0;
	for (size_t i = 0; i < numBins; ++i) {
		totalMass += bins[i].size();
	};

	//Compute relative weights of each bin
	for (size_t i = 0; i < numBins; ++i) {
		double factor = (double)bins[i].size();
		std::cout << "BIN: " << i << " FACTOR: " << totalMass / factor << std::endl;
		for (auto& R : bins[i]) {
			R.AppliedWeight = totalMass / factor;
		};
	};
	//Write to output database
	for (size_t i = 0; i < numBins; ++i) {
		for (auto& R : bins[i]) {
			outDB.Add(R);
		};
	};

	//Clear temporaries
	delete[] bins;

	return outDB;
};

void Endmembers::CalculateMeanAndRatioErrorsForDB(RockSample& rock, const RockDatabase& db, std::vector<double>& errR) {
	//Compute average rock, taking care of uninitialised values.
	std::vector<double> data;
	data.reserve(db.size());
	//Record all data points which are not NaN
	for (const auto& e : RockSample::allElements) {
		for (const RockSample& R : db) {
			double VAL = e.second.Data(R);
			if (std::isfinite(VAL)) {
				data.push_back(VAL);
			};
		};
		e.second.DataR(rock) = ComputeAverage(data);
		data.clear();
	};
	//Calculate standard deviation of all listed ratios
	errR.clear();
	for (size_t r = 0; r < ratioErr_Nmntr.size(); ++r) {
		auto rA = ratioErr_Nmntr[r];
		auto rB = ratioErr_Dnmtr[r];
		double var = ComputeWeightedErrorSqr(db, rA, rB);
		errR.push_back(sqrt(var));
	};
};

void Endmembers::CalculateWeightedMeanAndRatioErrorsForDB(RockSample& rock, const RockDatabase& db, std::vector<double>& errR) {
	//Compute weighted average rock, taking care of uninitialised values.
	//Record all data points which are not NaN
	for (const auto& e : RockSample::allElements) {
		double sum = 0.0;
		double N = 0.0;
		for (const RockSample& R : db) {
			double VAL = e.second.Data(R);
			if (std::isfinite(VAL)) {
				sum += R.AppliedWeight * VAL;
				N += R.AppliedWeight;
			};
		};
		e.second.DataR(rock) = sum / N;
	};
	//Calculate standard deviation of all listed ratios
	errR.clear();
	for (size_t r = 0; r < ratioErr_Nmntr.size(); ++r) {
		double var = ComputeWeightedWeightedErrorSqr(db, ratioErr_Nmntr[r], ratioErr_Dnmtr[r]);
		errR.push_back(sqrt(var));
	};
};

//------------------------------------------------------------------\\
//------------------------------------------------------------------\\
//---------------------------DUAL ENDMEMBERS------------------------\\
//------------------------------------------------------------------\\
//------------------------------------------------------------------\\

//Return the number of endmembers for a given configuration script
size_t ConfigScriptEndmemberCount(const std::string& configScript) {
	if (configScript == "MF") {
		return 2;
	} else if (configScript == "KMF") {
		return 3;
	} else if (configScript == "QUARTUS") {
		return 4;
	} else if (configScript == "QUINTUS") {
		return 5;
	} else {
		throw new std::runtime_error("Unrecognised endmember configuration script: '" + configScript + "'");
	};
};

void DualEndmembers::GenE() {
	const double ARCHAEAN_CUTOFF_AGE = 2500.0;

	RockDatabase ignKellerModernNoMORB, modernIgn, archaIgn, modernNoMORB;
	ignKellerModernNoMORB.Merge(ignKeller.Select(RangeFilter(OFF(RockSample::Age), MORB_CUTOFF_AGE, ARCHAEAN_CUTOFF_AGE)));

	modernIgn.Merge(ignKeller.Select(RangeFilter(OFF(RockSample::Age), 0.0, ARCHAEAN_CUTOFF_AGE)));
	archaIgn.Merge(ignKeller.Select(RangeFilter(OFF(RockSample::Age), ARCHAEAN_CUTOFF_AGE, 9999.9)));
	modernNoMORB.Merge(ignModernNoMORB).Merge(ignKellerModernNoMORB);

	//Run the correct configuration script
	if (configScript == "MF") {
		DB_Arch[0].Merge(archaIgn.Select(FuncFilter(EndmemberM))); //Archaean mafic
		DB_Arch[1].Merge(archaIgn.Select(FuncFilter(EndmemberF))); //Archaean TTG
		DB_Mdrn[0].Merge(modernNoMORB.Select(FuncFilter(EndmemberM))); //Modern mafic
		DB_Mdrn[1].Merge(modernIgn.Select(FuncFilter(EndmemberF))); //Modern felsic
		Ename[0] = "M";
		Ename[1] = "F";
	} else if (configScript == "KMF") {
		DB_Arch[0].Merge(archaIgn.Select(FuncFilter(EndmemberU))); //Komatiite
		DB_Arch[1].Merge(archaIgn.Select(FuncFilter(EndmemberM))); //Archaean mafic
		DB_Arch[2].Merge(archaIgn.Select(FuncFilter(EndmemberF))); //Archaean TTG
		DB_Mdrn[0]; //Empty database (no modern komatiites!)
		DB_Mdrn[1].Merge(modernNoMORB.Select(FuncFilter(EndmemberM))); //Modern mafic
		DB_Mdrn[2].Merge(modernIgn.Select(FuncFilter(EndmemberF))); //Modern felsic
		Ename[0] = "K";
		Ename[1] = "M";
		Ename[2] = "F";
	} else if (configScript == "QUARTUS") {
		DB_Arch[0].Merge(archaIgn.Select(FuncFilter(EndmemberU))); //Komatiite
		DB_Arch[1].Merge(archaIgn.Select(FuncFilter(EndmemberM))).Refilter(FuncFilter(EndmemberMNT)); //Archaean mafic, non-tholeiitic
		DB_Arch[2].Merge(archaIgn.Select(FuncFilter(EndmemberM))).Refilter(FuncFilter(EndmemberMTT)); //Archaean mafic, tholeiitic trend
		DB_Arch[3].Merge(archaIgn.Select(FuncFilter(EndmemberF))); //Archaean TTG
		DB_Mdrn[0]; //Empty database (no modern komatiites!)
		DB_Mdrn[1].Merge(modernNoMORB.Select(FuncFilter(EndmemberM))).Refilter(FuncFilter(EndmemberMNT)); //Modern mafic, non-tholeiitic
		DB_Mdrn[2].Merge(modernNoMORB.Select(FuncFilter(EndmemberM))).Refilter(FuncFilter(EndmemberMTT)); //Modern mafic, tholeiitic trend
		DB_Mdrn[3].Merge(modernIgn.Select(FuncFilter(EndmemberF))); //Modern felsic
		Ename[0] = "K";
		Ename[1] = "M-nT";
		Ename[2] = "M-T";
		Ename[3] = "F";
	} else {
		throw new std::runtime_error("Unimplemented dual endmember configuration script: '" + configScript + "'");
	};

	//Run all our calculations for both the Archaean and Modern databases
	//Also run for all endmembers
	std::vector<RockSample>* both_E[] = {&E_Arch, &E_Mdrn};
	std::vector<RockDatabase>* both_DB[] = {&DB_Arch, &DB_Mdrn};
	std::vector<std::vector<double>>* both_ratioErr[] = {&ratioErr_Arch, &ratioErr_Mdrn};
	for (size_t j = 0; j < 2; ++j) {
		for (size_t i = 0; i < N_e; ++i) {
			CalculateMeanAndRatioErrorsForDB((*both_E)[j][i], (*both_DB)[j][i], (*both_ratioErr)[j][i]);
		};
	};
};

double DualEndmembers::TransitionParameter(double t) const {
	double SWITCHOVER_START = transition_centre + (transition_width / 2);
	double SWITCHOVER_END = transition_centre - (transition_width / 2);

	if (t < SWITCHOVER_START) {
		if (t < SWITCHOVER_END) {
			return 1.0;
		} else {
			double f = (SWITCHOVER_START - t) / (SWITCHOVER_START - SWITCHOVER_END);
			if (f > 1.0) f = 1.0;
			return f;
		};
	} else {
		return 0.0;
	};
};

std::vector<RockDatabase> DualEndmembers::ExportSamples() {
	std::vector<RockDatabase> rtn(N_e);
	//Komatiitic always stays the same
	size_t jStart = 0;
	if (configScript != "MF") {
		rtn[0] = DB_Arch[0];
		jStart = 1;
	};

	//Others are linearly interpolated over the Archaean transition
	double f = TransitionParameter(t);
	for (size_t j = jStart; j < N_e; ++j) {
		size_t Nrocks = (size_t)((1 - f)*((double)DB_Arch[j].size()) + f * ((double)DB_Mdrn[j].size()));
		rtn[j].Reserve(Nrocks);
		for (size_t i = 0; i < Nrocks; ++i) {
			if (Random::Double() > f) {
				rtn[j].Add(DB_Arch[j].RandomElement());
			} else {
				rtn[j].Add(DB_Arch[j].RandomElement());
			};
		};
	};
	return rtn;
};

void DualEndmembers::RecalcEM() {
	if (!loaded) {
		GenE();
		loaded = true;
	};
	//Komatiitic remains constant w.r.t. time
	size_t jStart = 0;
	if (configScript != "MF") {
		E[0] = E_Arch[0];
		ratioErr[0] = ratioErr_Arch[0];
		jStart = 1;
	};

	//Others are linearly interpolated over the Archaean transition
	double f = TransitionParameter(t);
	//Errors are quadratically interpolated for every registered ratio
	//The weight applied is the product (Bi*fi)
	for (size_t j = jStart; j < N_e; ++j) {
		E[j] = RockSample::InterpolateElements(f, E_Arch[j], E_Mdrn[j]);
		for (size_t i = 0; i < ratioErr_Nmntr.size(); ++i) {
			double AB1, AB2, B1, B2, w1, w2, wSum;
			AB1 = ratioErr_Arch[j][i];    AB2 = ratioErr_Mdrn[j][i];
			B1 = ratioErr_Dnmtr[i](E_Arch[j]); B2 = ratioErr_Dnmtr[i](E_Mdrn[j]);
			w1 = (B1*(1 - f)); w2 = (B2*f); wSum = w1 + w2;
			ratioErr[j][i] = sqrt((w1 / wSum)*(w1 / wSum)*AB1*AB1 + (w2 / wSum)*(w2 / wSum)*AB2*AB2);
		};
	};
};

DualEndmembers::DualEndmembers(const std::string& CONFIG_SCRIPT, 
							   const RockDatabase & IGN_KELLER, const RockDatabase & IGN_NOMORB,
							   double TRANS_CENTRE, double TRANS_WIDTH)
	: Endmembers(ConfigScriptEndmemberCount(CONFIG_SCRIPT)), ignKeller(IGN_KELLER), ignModernNoMORB(IGN_NOMORB),
	transition_centre(TRANS_CENTRE), transition_width(TRANS_WIDTH), loaded(false), configScript(CONFIG_SCRIPT) {
	E_Arch.resize(N_e);
	E_Mdrn.resize(N_e);
	DB_Arch.resize(N_e);
	DB_Mdrn.resize(N_e);
	ratioErr_Arch.resize(N_e);
	ratioErr_Mdrn.resize(N_e);
};

//------------------------------------------------------------------\\
//------------------------------------------------------------------\\
//---------------------CUMULATIVE ENDMEMBERS------------------------\\
//------------------------------------------------------------------\\
//------------------------------------------------------------------\\

void CumulativeEndmembers::RecalcEM() {
	size_t start_recalc_idx = (configScript != "MF") ? 1 : 0; //Do not recalculate K endmember (if it is present)
	for (size_t i = start_recalc_idx; i < N_e; ++i) {
		timeDB[i].Clear().Merge(fullDB[i].Select(RangeFilter(OFF(RockSample::Age), t, t + kernel_length)));
		//Apply additional weight normalization
		for (auto& S : timeDB[i].Select(RangeFilter(OFF(RockSample::Age), t + 1000, MAX_AGE))) {
			S.AppliedWeight *= 0.2;
		};
		CalculateWeightedMeanAndRatioErrorsForDB(E[i], timeDB[i], ratioErr[i]);
	};
	if (needToLoadK) {
		//Calculate K, but only once (since it will not change)
		CalculateWeightedMeanAndRatioErrorsForDB(E[0], fullDB[0], ratioErr[0]);
		needToLoadK = false;
	};
};

CumulativeEndmembers::CumulativeEndmembers(const std::string& cScript, const RockDatabase& kellerDB, const RockDatabase& noOceanDB, double KERNEL_LENGTH, double sampling_width)
	: kernel_length(KERNEL_LENGTH), configScript(cScript), needToLoadK(true), Endmembers(ConfigScriptEndmemberCount(cScript)) {

	if (!std::isfinite(kernel_length)) {
		kernel_length = MAX_AGE;
	};

	fullDB.resize(N_e);
	timeDB.resize(N_e);

	RockDatabase ign_keller = CreateAgeUniformDatabase(kellerDB, sampling_width);
	RockDatabase ign_noOcean;
	ign_noOcean.Merge(noOceanDB).Merge(ign_keller.Select(RangeFilter(OFF(RockSample::Age), MORB_CUTOFF_AGE, MAX_AGE)));
	RockDatabase ign_noOceanW = CreateAgeUniformDatabase(ign_noOcean, sampling_width);
	//RockDatabase ign_keller(kellerDB.Select(NanFilter(OFF(RockSample::Age))));
	//RockDatabase ign_noOcean(noOceanDB.Select(NanFilter(OFF(RockSample::Age))));

	//auto q_keller_nomorb = ign_keller.Select(RangeFilter(OFF(RockSample::Age), MORB_CUTOFF_AGE, MAX_AGE));
	auto q_u = ign_keller.Select(FuncFilter(EndmemberU));
	auto q_m = ign_noOceanW.Select(FuncFilter(EndmemberM));
	auto q_f = ign_keller.Select(FuncFilter(EndmemberF));

	//Run the correct configuration script
	if (configScript == "MF") {
		fullDB[0].Merge(q_m); //Mafic
		fullDB[1].Merge(q_f); //Felsic
		Ename[0] = "M";
		Ename[1] = "F";
		needToLoadK = false;
	} else if (configScript == "KMF") {
		fullDB[0].Merge(q_u); //Komatiite
		fullDB[1].Merge(q_m); //Modern mafic
		fullDB[2].Merge(q_f); //Modern felsic
		Ename[0] = "K";
		Ename[1] = "M";
		Ename[2] = "F";
		timeDB[0] = fullDB[0]; //So sample export works correctly
	} else if (configScript == "QUARTUS") {
		fullDB[0].Merge(q_u); //Komatiite
		fullDB[1].Merge(q_m).Refilter(FuncFilter(EndmemberMNT)); //Mafic, non-tholeiitic
		fullDB[2].Merge(q_m).Refilter(FuncFilter(EndmemberMTT)); //Mafic, tholeiitic trend
		fullDB[3].Merge(q_f); //Felsic
		Ename[0] = "K";
		Ename[1] = "M-nT";
		Ename[2] = "M-T";
		Ename[3] = "F";
		timeDB[0] = fullDB[0]; //So sample export works correctly
	} else {
		throw new std::runtime_error("Unimplemented endmember configuration script: '" + configScript + "'");
	};

};

std::vector<RockDatabase> CumulativeEndmembers::ExportSamples() {
	std::vector<RockDatabase> rtn;
	for (const auto& db : timeDB) {
		rtn.push_back(db);
	};
	return rtn;
};

//------------------------------------------------------------------\\
//------------------------------------------------------------------\\
//---------------------EXPONENTIAL ENDMEMBERS-----------------------\\
//------------------------------------------------------------------\\
//------------------------------------------------------------------\\

inline double expKernel(double deltaX, double s) noexcept {
	return (1.0 / (sqrt(2 * M_PI)*s))*exp(-(deltaX*deltaX) / (2 * s*s));
};

void ExponentialEndmembers::RecalcEM() {
	size_t start_recalc_idx = (configScript != "MF") ? 1 : 0; //Do not recalculate K endmember (if it is present)
	for (size_t i = start_recalc_idx; i < N_e; ++i) {
		timeDB[i].Clear().Merge(fullDB[i]);
		for (auto& S : timeDB[i]) {
			double w = expKernel(S.Age - t, samplingWidth);
			S.AppliedWeight = w;
		};
		CalculateWeightedMeanAndRatioErrorsForDB(E[i], timeDB[i], ratioErr[i]);
	};
	if (needToLoadK) {
		//Calculate K, but only once (since it will not change)
		CalculateWeightedMeanAndRatioErrorsForDB(E[0], fullDB[0], ratioErr[0]);
		needToLoadK = false;
	};
};

//------------------------------------------------------------------\\
//------------------------------------------------------------------\\
//---------------------FUTURE PAST ENDMEMBERS-----------------------\\
//------------------------------------------------------------------\\
//------------------------------------------------------------------\\

void FuturePastEndmembers::RecalcEM() {
	size_t start_recalc_idx = (configScript != "MF") ? 1 : 0; //Do not recalculate K endmember (if it is present)
	for (size_t i = start_recalc_idx; i < N_e; ++i) {
		timeDB[i].Clear().Merge(fullDB[i].Select(RangeFilter(OFF(RockSample::Age),
															 t - samplingWidth,
															 t + samplingWidth)));
		CalculateWeightedMeanAndRatioErrorsForDB(E[i], timeDB[i], ratioErr[i]);
	};
	if (needToLoadK) {
		//Calculate K, but only once (since it will not change)
		CalculateWeightedMeanAndRatioErrorsForDB(E[0], fullDB[0], ratioErr[0]);
		needToLoadK = false;
	};
};

//------------------------------------------------------------------\\
//------------------------------------------------------------------\\
//-------------------------THE BOMEMBERS----------------------------\\
//------------------------------------------------------------------\\
//------------------------------------------------------------------\\

void BoMembers::GenerateBootstraps(size_t start_idx) {
	//Load up the elements we will require into a common list
	bootEl.resize(N_e);
	bootR.resize(N_e);
	//NB: 'Age' is already guaranteed to be non-NaN in the databases, but none of the other elements are!
	//And since the WXB functions are not NaN-resistant, we have to call std::isfinite() here instead.
	for (size_t i = start_idx; i < N_e; ++i) {
		//Bootstrap every element, record in the elemental bootstrap cache
		for (const auto& EL : RockSample::allElements) {
			const auto& O = EL.second;
			std::vector<double> age;
			std::vector<double> el;
			for (const auto& R : fullDB[i]) {
				double val = O.Data(R);
				if (std::isfinite(val)) {
					age.push_back(R.Age);
					el.push_back(val);
				};
			};
			//We don't need errors, only the best fit, so N is set to unity here!
			bootEl[i].push_back(WEB_Bootstrap(age, el, bootKernelWidth, 1));
		};
		//Bootstrap all the required ratios
		for (size_t j = 0; j < ratioErr_Nmntr.size(); ++j) {
			std::vector<double> age;
			std::vector<double> A;
			std::vector<double> B;
			for (const auto& R : fullDB[i]) {
				double a = ratioErr_Nmntr[j].Data(R);
				double b = ratioErr_Dnmtr[j].Data(R);
				if (std::isfinite(a) && std::isfinite(b)) {
					age.push_back(R.Age);
					A.push_back(a);
					B.push_back(b);
				};
			};
			bootR[i].push_back(WRB_Bootstrap(age, A, B, bootKernelWidth));
			ratioErr[i].resize(ratioErr.size());
		};
	};
};

void BoMembers::RecalcEM() {
	size_t start_recalc_idx = (configScript != "MF") ? 1 : 0; //Do not recalculate K endmember (if it is present)
	//Do first-time initialisation (if necessary)
	if (bootEl.empty()) {
		GenerateBootstraps(start_recalc_idx);
		if (needToLoadK) {
			//Use standard estimators to load mean and variance
			CalculateWeightedMeanAndRatioErrorsForDB(E[0], fullDB[0], ratioErr[0]);
			needToLoadK = false;
		};
	};
	for (size_t i = start_recalc_idx; i < N_e; ++i) {
		//Use bootstrapped elements to fix best fit
		for (size_t j = 0; j < RockSample::allElements.size(); ++j) {
			const auto& O = RockSample::allElements.at(j).second;
			O.DataR(E[i]) = bootEl[i][j].bestFit(t);
		};
		//Use bootstrapped ratios to fix variances
		for (size_t j = 0; j < ratioErr_Nmntr.size(); ++j) {
			ratioErr[i][j] = bootR[i][j].stdError(t);
		};
	};
};
