#pragma once
#include "moduleCommon.h"
#include "WRB.h"
#include "pyLib.h"

struct ChauvenetFilter : public RockDatabase::WholeDBFilter {
	MemberOffset<RockSample, double> _mO;
	bool _logNorm;
	double Avrg;
	double StDev;
	double dataSize;

	ChauvenetFilter(bool LOGNORMAL, MemberOffset<RockSample, double> FIELD);

	void Init(const RockDatabase::STYPE&) override;
	bool Test(const RockDatabase::ETYPE& R) override;
};

//Get the set of elements which are normally distributed in igneous rocks
//All other elements are assumed to be log-normally distributed
std::set<std::string>& NormallyDistributedElements();

// Data structures to deliver reconstruction results in
// 
template<int N>
struct MixState {
private:
	double E[N];
public:
	const static int COMPONENT_COUNT = N;
	inline double& operator[](size_t idx) {
		return E[idx];
	};
	inline double operator[](size_t idx) const {
		return E[idx];
	};
	MixState() {};
	MixState(const std::vector<double>& v) {
		assert(v.size() >= N);
		for (int i = 0; i < N; ++i) {
			E[i] = v[i];
		};
	};
	static MixState Default() {
		double defaultValue = 1.0 / ((double)N);
		MixState def;
		for (int i = 0; i < N; ++i) {
			def[i] = defaultValue;
		};
		return def;
	};
#ifdef PYTHON_LIB
	static void BoundsCheck(int idx) {
		if (idx < 0 || idx >= N) {
			PyErr_SetString(PyExc_IndexError, "MixState index out of range");
			throw boost::python::error_already_set();
		};
	};
	static double get_endmember_value(const MixState<N>& M, int idx) {
		BoundsCheck(idx);
		return M[idx];
	};
	static void set_endmember_value(MixState<N>& M, int idx, double value) {
		BoundsCheck(idx);
		M[idx] = value;
	};
#endif
};

// Use the weighted variance estimator from Cochran '77 to estimate the squared standard
// error of a geological ratio within a database.
double ComputeWeightedErrorSqr(const RockDatabase& db, const MemberOffset<RockSample, double>& A, const MemberOffset<RockSample, double>& B);

double ComputeWeightedWeightedErrorSqr(const RockDatabase& db, const MemberOffset<RockSample, double>& A, const MemberOffset<RockSample, double>& B);


// Structure to store detailed results from the MCMC run of a single timestep.
//
class SingleTimeState {
public:
	struct RatioSystem {
		//Concentrations of nominator (a) and denominator (b) elements in all endmembers
		std::vector<double>		cA;
		std::vector<double>		cB;
		//Ratio value given by the bootstrap prediction
		double bootR;
		//Ratio value given by the best-fit state
		double bestR;
		//Ratio values from a random subset of the MCMC states
		std::vector<double>		mcmcR;
	};
private:
	std::vector<RatioSystem> ratio;
public:
	inline RatioSystem& operator[](size_t idx) {
		return ratio[idx];
	};
	inline RatioSystem operator[](size_t idx) const {
		return ratio[idx];
	};

	inline RatioSystem& Add() {
		ratio.push_back(RatioSystem());
		return ratio.back();
	};

	//Endmember proportions
	std::vector<double> endmemberP025;
	std::vector<double> best;
	std::vector<double> endmemberP975;

#ifdef PYTHON_LIB
	void BoundsCheck(int idx) const {
		if (idx < 0 || idx >= ratio.size()) {
			PyErr_SetString(PyExc_IndexError, "MixState index out of range");
			throw boost::python::error_already_set();
		};
	};
	static RatioSystem get_endmember_value(const SingleTimeState& STS, int idx) {
		STS.BoundsCheck(idx);
		return STS[idx];
	};
	static void set_endmember_value(SingleTimeState& STS, int idx, const RatioSystem& value) {
		STS.BoundsCheck(idx);
		STS[idx] = value;
	};
#endif
};

// Dense map for configuring the ReconManager and the ResultsProcessors
// 
class DenseStringMap {
	std::map<std::string, std::vector<std::string>> dat;
public:
	const std::vector<std::string>& operator[](const std::string& key) const;
	const std::string& Get(const std::string& key) const;
	bool Contains(const std::string& key) const;
	void Insert(const std::string& key, const std::string& val);
	void Insert(const std::string& key, std::initializer_list<std::string> V);

#ifdef PYTHON_LIB
	DenseStringMap(const boost::python::dict& D);
#endif
	DenseStringMap();
};

