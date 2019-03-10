#include "stdafx.h"
#include "reconCommon.h"

struct NormallyDistributed {
	std::set<std::string> set;
	NormallyDistributed() {
		set.insert("Al2O3");
		set.insert("CaO");
		set.insert("Co");
		set.insert("FeOt");
		set.insert("MgO");
		set.insert("MnO");
		set.insert("Na2O");
		set.insert("Sc");
		set.insert("SiO2");
		set.insert("V");
		set.insert("Zn");
	};
};

void ChauvenetFilter::Init(const RockDatabase::STYPE & LIST) {
	std::vector<double> allData;
	for (auto e : LIST) {
		double VAL = _mO(e);
		//Measurements of PRECISELY zero are unrealistic
		if ((std::isfinite(VAL)) && (VAL != 0)) {
			allData.push_back(VAL);
		};
	};
	Avrg = ComputeAverage(allData);
	StDev = ComputeStdDev(allData);
	dataSize = (double)allData.size();
};

bool ChauvenetFilter::Test(const RockDatabase::ETYPE & R) {
	double VAL = (_logNorm ? log(_mO(R)) : _mO(R));
	double x = abs((VAL - Avrg) / StDev);
	double P = dataSize * (1 / sqrt(2 * M_PI))*exp(-x * x / 2);
	if (P > 0.5) {
		return true;
	};
	return false;
};

ChauvenetFilter::ChauvenetFilter(bool LOGNORMAL, MemberOffset<RockSample, double> FIELD) : _logNorm(LOGNORMAL), _mO(FIELD) {};

std::set<std::string>& NormallyDistributedElements() {
	static NormallyDistributed nd;
	return nd.set;
};

const std::vector<std::string>& DenseStringMap::operator[](const std::string& key) const {
	return dat.find(key)->second;
};

const std::string& DenseStringMap::Get(const std::string& key) const {
	return dat.find(key)->second[0];
};

bool DenseStringMap::Contains(const std::string& key) const {
	return (dat.find(key) != dat.end());
};

void DenseStringMap::Insert(const std::string& key, const std::string& val) {
	auto it = dat.find(key);
	if (it == dat.end()) {
		//Insert new vector
		dat[key] = std::vector<std::string>(1, val);
	} else {
		//Append to end of existing vector
		it->second.push_back(val);
	};
};

void DenseStringMap::Insert(const std::string& key, std::initializer_list<std::string> V) {
	for (const auto& v : V) {
		Insert(key, v);
	};
};

DenseStringMap::DenseStringMap() {};

#ifdef PYTHON_LIB
DenseStringMap::DenseStringMap(const boost::python::dict& D) {
	boost::python::list keys = D.keys();
	for (long i = 0; i < boost::python::len(keys); ++i) {
		std::string K = boost::python::extract<std::string>(keys[i]);

		boost::python::extract<boost::python::object> objectExtractor(D[K]);
		boost::python::object o = objectExtractor();
		std::string pyType = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
		//std::cout << K << ": this is an Object: " << pyType << std::endl;

		if (pyType == "list") {
			boost::python::list L = boost::python::extract<boost::python::list>(D[K]);
			for (long j = 0; j < boost::python::len(L); ++j) {
				Insert(K, boost::python::extract<std::string>(D[K][j]));
			};
		} else if (pyType == "str") {
			Insert(K, boost::python::extract<std::string>(D[K]));
		};
	};
};
#endif

double ComputeWeightedErrorSqr(const RockDatabase& db, const MemberOffset<RockSample, double>& rA, const MemberOffset<RockSample, double>& rB) {
	// Implementing the weighted average estimator from Cochran 1977,
	// via Endlich & al. 1988, via Gatz & Smith 1994.
	// Here, B's are the weights, (A/B)'s the values, and A's the products of weights by values
	// Weights are represented by W instead of P, and products by X.
	std::vector<double> WiXi;
	std::vector<double> Wi;
	const double EPSILON = 0.00000001; //This helps us avoid divisions by zero
	double sumWX = 0.0;
	double sumW = 0.0;
	double N = 0.0;
	for (auto& itR : db) {
		//Find all rocks which are non-NaN in both nominator and denominator values
		double A = rA(itR);
		double B = rB(itR);
		if (std::isfinite(A) && std::isfinite(B) && B > EPSILON) {
			sumWX += A;
			sumW += B;
			WiXi.push_back(A);
			Wi.push_back(B);
			N += 1.0;
		};
	};

	double Wbar = sumW / N; //The average weight
	double Xbar = sumWX / sumW; //The weighed average
	double WbarXbar = Wbar * Xbar;
	double sum = 0.0;
	for (size_t k = 0; k < WiXi.size(); ++k) {
		sum += (WiXi[k] - WbarXbar) * (WiXi[k] - WbarXbar);
		sum -= 2 * Xbar * (Wi[k] - Wbar) * (WiXi[k] - WbarXbar);
		sum += Xbar * Xbar * (Wi[k] - Wbar) * (Wi[k] - Wbar);
	};
	//Finalise
	double var = (N * sum) / ((N - 1) * sumW * sumW);
	return var;
};

double ComputeWeightedWeightedErrorSqr(const RockDatabase& db, const MemberOffset<RockSample, double>& rA, const MemberOffset<RockSample, double>& rB) {
	// Implementing the weighted average estimator from Cochran 1977,
	// via Endlich & al. 1988, via Gatz & Smith 1994.
	// Here, B's are the weights, (A/B)'s the values, and A's the products of weights by values
	// Weights are represented by W instead of P, and products by X.
	std::vector<double> WiXi;
	std::vector<double> Wi;
	const double EPSILON = 0.00000001; //This helps us avoid divisions by zero
	double sumWX = 0.0;
	double sumW = 0.0;
	double N = 0.0;
	for (auto& itR : db) {
		//Find all rocks which are non-NaN in both nominator and denominator values
		double A = rA(itR);
		double B = itR.AppliedWeight * rB(itR);
		if (std::isfinite(A) && std::isfinite(B) && B > EPSILON) {
			sumWX += A;
			sumW += B;
			WiXi.push_back(A);
			Wi.push_back(B);
			N += 1.0;
		};
	};

	double Wbar = sumW / N; //The average weight
	double Xbar = sumWX / sumW; //The weighed average
	double WbarXbar = Wbar * Xbar;
	double sum = 0.0;
	for (size_t k = 0; k < WiXi.size(); ++k) {
		sum += (WiXi[k] - WbarXbar) * (WiXi[k] - WbarXbar);
		sum -= 2 * Xbar * (Wi[k] - Wbar) * (WiXi[k] - WbarXbar);
		sum += Xbar * Xbar * (Wi[k] - Wbar) * (Wi[k] - Wbar);
	};
	//Finalise
	double var = (N * sum) / ((N - 1) * sumW * sumW);
	return var;
};
