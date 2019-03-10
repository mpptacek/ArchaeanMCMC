#pragma once
#include "utils.h"
#include "Analysis.h"

// A "Discrete" function is based on the interpolation of datapoints to produce a semi-smooth curve.
// 
#define EPSILON 0.001
struct DiscreteFunction {
	bool finalised;	
	struct fVal {
		double x;
		double y;
		fVal(double X, double Y) : x(X), y(Y) {};
		fVal() {};
		inline bool operator<(const fVal &rhs) const { return x < rhs.x; };
	};
	std::vector<fVal> vl;
	
	double operator()(double x) const {
		//Bounds check
		size_t lastIDX = vl.size() - 1;
		if (!((x > vl[0].x) && (x < vl[lastIDX].x))) {
			//If x lies within epsilon of either end, use the end value
			if ((x < vl[lastIDX].x) && (x + EPSILON > vl[0].x)) {
				return vl[0].y;
			} else if ((x > vl[0].x) && (x - EPSILON < vl[lastIDX].x)) {
				return vl[lastIDX].y;
			} else {
				//Otherwise, do not extend series outwards based on last datapoint
				return NAN;
			};
		};

		//Binary search algorithm (since we have a sorted array!)
		size_t L = 0;
		size_t R = lastIDX;
		while (L <= R) {
			size_t i = (L + R) / 2;
			if (vl[i].x < x) {
				L = i + 1;
			} else if (vl[i].x > x) {
				R = i - 1;
			} else {
				return vl[i].y;
			};
		};
		//Linear interpolation between L & R (note: L is now larger than R!)
		double rangeLR = (vl[L].x - vl[R].x);
		double f = (x - vl[R].x) / rangeLR;
		return vl[R].y + f*(vl[L].y - vl[R].y);
	};
	
	void AddNewPoint(double x, double y) {
		finalised= false;
		vl.push_back(fVal(x,y));
	};
	
	inline double FirstX() const {
		return vl.front().x;
	};

	inline double LastX() const {
		return vl.back().x;
	};

	inline double FirstY() const {
		return vl.front().y;
	};

	inline double LastY() const {
		return vl.back().y;
	};

	void Finalise() {
		std::sort(vl.begin(),vl.end());
		finalised= true;
	};
	
	std::vector<double> GenerateXList() const {
		std::vector<double> rList;
		for(unsigned int i=0; i<vl.size();++i) {
			rList.push_back(vl[i].x);
		};
		return rList;
	};
	std::vector<double> GenerateYList() const {
		std::vector<double> rList;
		for(unsigned int i=0; i<vl.size();++i) {
			rList.push_back(vl[i].y);
		};
		return rList;
	};
	
	Vect<2>::List GenerateVectList() const {
		Vect<2>::List rList;
		for(unsigned int i=0; i<vl.size();++i) {
			Vect<2> V;
			V[0]= vl[i].x;
			V[1]= vl[i].y;
			rList.push_back(V);
		};
		return rList;
	};
	
	DiscreteFunction(size_t suggestedArraySize = 0) : finalised(true) {
		if (suggestedArraySize > 0) {
			Reserve(suggestedArraySize);
		};
	};

	void PrintToFile(const std::string& FNAME, const std::string& xname, const std::string& yname) const {
		std::stringstream ss;
		ss << xname << "," << yname << std::endl;
		for (const auto& V : vl) {
			ss << V.x << "," << V.y << std::endl;
		};
		std::ofstream oFile((DefaultWritePath() + FNAME + ".csv").c_str());
		oFile << ss.str();
		oFile.close();
	};

	inline void Reserve(size_t SIZE) {
		vl.reserve(SIZE);
	};
};
#undef EPSILON

//A multiple-endmember mixing model
//Can also handle the mixing of isotopes (which depend on the endmember concentration of their species, and not only on the isotopic values)
template<typename T>
class MixingModel {
	std::vector<T> endMember;
	std::vector< MemberOffset<T,double> > mixedElements;
	std::vector< MemberOffset<T,double> > mixedIsotopes;
	std::vector< MemberOffset<T,double> > mixedIsotopes_Element;
	std::vector< MemberOffset<T, double> > mixedRatios;
	std::vector< MemberOffset<T, double> > mixedRatios_aEl;
	std::vector< MemberOffset<T, double> > mixedRatios_bEl;
	MemberOffset<T,double> varY;
	MemberOffset<T,double> varX;

public:
	MixingModel<T>&  Endmember(const T& e) {
		endMember.push_back(e);
		return *this;
	};

	MixingModel<T>&  Endmembers(std::initializer_list<T> il) {
		for (const T& e : il) {
			endMember.push_back(e);
		};
		return *this;
	};

	MixingModel<T>& ResetEndmembers() {
		endMember.clear();
		return *this;
	};

	MixingModel<T>& ResetEndmembers(std::initializer_list<T> il) {
		endMember.clear();
		Endmembers(il);
		return *this;
	};

	MixingModel<T>& Element(MemberOffset<T, double> el) {
		mixedElements.push_back(el);
		return *this;
	};

	MixingModel<T>& Isotope(MemberOffset<T, double> iso, MemberOffset<T, double> carrier) {
		mixedIsotopes.push_back(iso);
		mixedIsotopes_Element.push_back(carrier);
		return *this;
	};

	MixingModel<T>& Ratio(MemberOffset<T, double> top, MemberOffset<T, double> base) {
		mixedRatios_aEl.push_back(top);
		mixedRatios_bEl.push_back(base);
		return *this;
	};

	std::vector<double> ComputeBestFit(const T& tgt) const {
		size_t M = 1 + mixedElements.size() + mixedRatios_aEl.size() + mixedIsotopes.size();
		size_t N = endMember.size();
		std::vector<double> D(M);
		std::vector<std::vector<double>> A(M);
		//Load vector D
		size_t L = 0;
		for (size_t i = 0; i < mixedElements.size(); ++i) {
			D[i + L] = mixedElements[i](tgt);
		};
		L += mixedElements.size();
		for (size_t i = 0; i < mixedRatios_aEl.size(); ++i) {
			D[i + L] = 0.0;
		};
		L += mixedRatios_aEl.size();
		for (size_t i = 0; i < mixedIsotopes.size(); ++i) {
			D[i + L] = 0.0;
		};
		D[M - 1] = 1.0;
		//Load matrix A
		L = 0;
		for (size_t i = 0; i < mixedElements.size(); ++i) {
			for (size_t j = 0; j < N; ++j) {
				A[i].push_back(mixedElements[i](endMember[j]));
			};
		};
		L += mixedElements.size();
		for (size_t i = 0; i < mixedRatios_aEl.size(); ++i) {
			for (size_t j = 0; j < N; ++j) {
				double An = mixedRatios_aEl[i](endMember[j]);
				double Bn = mixedRatios_bEl[i](endMember[j]);
				double At = mixedRatios_aEl[i](tgt);
				double Bt = mixedRatios_bEl[i](tgt);
				double AtBt = At / Bt;
				A[i + L].push_back( An - AtBt*Bn );
			};
		};
		L += mixedRatios_aEl.size();
		for (size_t i = 0; i < mixedIsotopes.size(); ++i) {
			for (size_t j = 0; j < N; ++j) {
				double Cn = mixedIsotopes_Element[i](endMember[j]);
				double It = mixedIsotopes[i](tgt);
				double In = mixedIsotopes[i](endMember[j]);
				double CnIt = Cn * It;
				double CnIn = Cn * In;
				A[i + L].push_back(CnIn - CnIt);
			};
		};
		for (size_t j = 0; j < N; ++j) {
			A[M - 1].push_back(1.0);
		};
		//Solve the system
		std::vector<double> R= MSolve(A, D, N);
		//Normalise (in case of over-determined systems)
		double sum = ComputeSum(R);
		for (double& X : R) { X *= 1 / sum; };
		return R;
	};
	
	MixingModel<T>() {};

private:
	T ComputeMixed(const std::vector<double>& COORD) const {
		T tgt;
		//Mix the individual elements
		for(size_t i=0; i< mixedElements.size(); ++i) {
			MemberOffset<T,double>& mixEl= mixedElements[i];
			mixEl.DataR(tgt)= 0.0;
			for(unsigned int j=0; j< endMember.size(); ++j) {
				mixEl.DataR(tgt)+= COORD[j]*mixEl.Data(endMember[j]);
			};
		};
		//Mix the elemental ratios
		for (size_t i = 0; i < mixedRatios.size(); ++i) {
			MemberOffset<T, double>& mixR_A = mixedRatios_aEl[i];
			MemberOffset<T, double>& mixR_B = mixedRatios_bEl[i];
			mixR_A.DataR(tgt) = 0.0;
			mixR_B.DataR(tgt) = 0.0;
			for (unsigned int j = 0; j < endMember.size(); ++j) {
				mixR_A.DataR(tgt) += COORD[j] * mixR_A.Data(endMember[j]);
				mixR_B.DataR(tgt) += COORD[j] * mixR_B.Data(endMember[j]);
			};
		};

		//Mix the isotopes
		for(size_t i=0; i< mixedIsotopes.size(); ++i) {
			MemberOffset<T,double>& mixIso= mixedIsotopes[i];
			MemberOffset<T,double>& mixIso_Carrier= mixedIsotopes_Element[i];
			mixIso.DataR(tgt)= 0.0;
			double totalCarrierConc= 0.0;
			for(unsigned int j=0; j< endMember.size(); ++j) {
				totalCarrierConc+= COORD[j]*mixIso_Carrier.Data(endMember[j]);
			};
			for(unsigned int j=0; j< endMember.size(); ++j) {
				mixIso.DataR(tgt)+= (COORD[j]*mixIso.Data(endMember[j])*mixIso_Carrier.Data(endMember[j]))/totalCarrierConc;
			};
		};
	};
};
