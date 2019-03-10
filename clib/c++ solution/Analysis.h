#pragma once
#include "stdafx.h"
#include "Model.h"
#include <map>
#include <vector>

#include <iostream>

template<class FUNCTION>
std::vector<double> MisfitFromModel(FUNCTION& f, const Vect<2>::List& DATA) {
	std::vector<double> retL;
	for (Vect<2> V : DATA) {
		double x = V[0];
		double y = V[1];
		retL.push_back( y - f(x) );
	};
	return retL;
};

template<class FUNCTION>
double SampleVariance(FUNCTION& f, const Vect<2>::List& DATA) {
	if (DATA.size() < 1)
		return 0.0;
	double acc;
	for (Vect<2> V : DATA) {
		double x = V.v[0];
		double y = V.v[1];
		double d = y - f(x);
		acc+= d*d;
	};
	return acc/(DATA.size()-1);
};

template<class FUNCTION>
double MeanSquareError(FUNCTION& f, const Vect<2>::List& DATA) {
	if (DATA.size() < 2)
		return 0.0;
	double acc= 0.0;
	for (Vect<2> V : DATA) {
		double x = V.v[0];
		double y = V.v[1];
		double d = y - f(x);
		acc += d*d;
	};
	return acc / (DATA.size() - 2);
};

template<typename T>
std::vector<T> ApplyUntilStabilisation(const std::vector<T>& DATA, std::vector<T>(*F)(const std::vector<T>&)) {
	const unsigned int MAX_ITER = 999;
	std::vector<T> RESULT = DATA;
	for (unsigned int i = 0; i < MAX_ITER; ++i) {
		unsigned int res_size = RESULT.size();
		RESULT = (*F)(RESULT);
		if (res_size == RESULT.size()) {
			return RESULT;
		};
	};
	std::cout << "Warning: Maximum iteration count exceeded during ApplyUntilStabilisation() run." << std::endl;
	return RESULT;
};


template<typename T>
T MaxValue(const std::vector<T>& V) {
	T val = NAN;
	for (auto v : V) {
		if (!std::isfinite(val)) {
			if (std::isfinite(v)) {
				val = v;
			};
		} else {
			if (std::isfinite(v)) {
				if (v > val) val = v;
			};
		};
	};
	return val;
};

template<typename T>
T MinValue(const std::vector<T>& V) {
	T val = NAN;
	for (auto v : V) {
		if (!std::isfinite(val)) {
			if (std::isfinite(v)) {
				val = v;
			};
		} else {
			if (std::isfinite(v)) {
				if (v < val) val = v;
			};
		};
	};
	return val;
};


template<typename T>
T RangeValue(const std::vector<T>& V) {
	T mx = MaxValue(V);
	T mn = MinValue(V);
	return mx - mn;
};

template<typename T>
std::vector<T> RemoveZeroEntries(const std::vector<T>& V) {
	std::vector<double> retL;
	for (T el : V) { if (el != 0) retL.push_back(el); };
	return retL;
}


template<typename T>
std::vector<T> RemoveNaNEntries(const std::vector<T>& V) {
	std::vector<double> retL;
	for (T el : V) { if (std::isfinite(el)) retL.push_back(el); };
	return retL;
}

template<typename T>
T ComputeAverage(const std::vector<T>& V) {
	if (V.size() == 0)
		return (T)0;
	T acc = 0;
	for (T el : V) {
		acc += el;
	};
	acc /= (double)(V.size());
	return acc;
};

template<typename T>
T ComputeStdDev(const std::vector<T>& V) {
	if (V.size() == 0)
		return (T)0;
	double mean = ComputeAverage(V);
	double acc = 0;
	for (T el : V) {
		acc += (el-mean)*(el - mean);
	};
	acc /= (double)(V.size());
	acc = sqrt(acc);
	return acc;
};

template<typename T>
T ComputeSampleStdDev(const std::vector<T>& V) {
	if (V.size() == 0)
		return (T)0;
	double mean = ComputeAverage(V);
	double acc = 0;
	for (T el : V) {
		acc += (el - mean)*(el - mean);
	};
	acc /= (double)(V.size()-1);
	return sqrt(acc);
};

template<typename T>
T ComputeStdErr(const std::vector<T>& V) {
	double stDev = ComputeSampleStdDev(V);
	double sN = sqrt(((double)V.size()));
	return stDev / sN;
};

template<typename T>
T ComputeMedian(const std::vector<T>& V) {
	if (V.size()==0)
		return (T)0;
	std::vector<T> temp(V);
	std::sort(temp.begin(),temp.end());
	return temp[temp.size()/2];
};

//Returns the percentile of a sorted vector
template<typename T>
inline T SortedVectorPercentile(const std::vector<T>& sortedVector, double percentile) noexcept {
	double IDXd = (percentile / 100.0)*((double)sortedVector.size());
	size_t IDXi = (size_t)IDXd;
	if ((IDXi + 1) <= (sortedVector.size() - 1)) {
		if (IDXi != 0) {
			return (sortedVector[IDXi - 1] + sortedVector[IDXi] + sortedVector[IDXi + 1]) / 3;
		} else {
			return (sortedVector[IDXi] + sortedVector[IDXi + 1]) / 2;
		};
	} else {
		return sortedVector[IDXi];
	};
};

template<typename T>
T ComputePercentile(const std::vector<T>& V, double PER) {
	if (V.size()==0)
		return (T)0;
	std::vector<T> temp(V);
	std::sort(temp.begin(),temp.end());
	return SortedVectorPercentile(temp, PER);
};

template<typename T>
T ComputeCovariance(const std::vector<T>& A, const std::vector<T>& B) {
	T meanA = ComputeAverage(A);
	T meanB = ComputeAverage(B);
	double sum = 0.0;
	size_t N = A.size();
	double Nf = (double)N;
	for (size_t i = 0; i < N; ++i) {
		sum += (A[i] - meanA)*(B[i] - meanB);
	};
	return sum / (Nf - 1);
};


template<typename T>
T ComputeCorrelation(const std::vector<T>& A, const std::vector<T>& B) {
	T meanA = ComputeAverage(A);
	T meanB = ComputeAverage(B);
	T stdA = ComputeSampleStdDev(A);
	T stdB = ComputeSampleStdDev(B);
	double sum = 0.0;
	size_t N = A.size();
	for (size_t i = 0; i < N; ++i) {
		sum += (A[i] - meanA)*(B[i] - meanB);
	};
	return sum / (((double)(N-1)) * stdA * stdB);
};

template<typename T>
std::vector<T> ElementwiseLn(const std::vector<T>& V) {
	std::vector<T> rV;
	for (T el : V) {
		rV.push_back(log(el));
	};
	return rV;
};

template<typename T>
T ComputeSum(const std::vector<T>& V) {
	T acc = 0;
	for (T v : V) { acc += v; };
	return acc;
};

template<typename T>
std::vector<T> ApplyChauvenetCriterion(const std::vector<T>& DATA) {
	std::vector<double> retL;
	double Avrg = ComputeAverage(DATA);
	double StDev = ComputeStdDev(DATA);
	for (double V : DATA) {
		double x = abs((V - Avrg) / StDev);
		double P = DATA.size()*(1 / sqrt(2 * M_PI))*exp(-x * x / 2);
		if (P > 0.5) {
			retL.push_back(V);
		};
	};
	return retL;
};

template<typename T>
std::vector<T> ApplyLognormalChauvenetCriterion(const std::vector<T>& DATA) {
	std::vector<double> zDATA = RemoveZeroEntries(DATA);
	std::vector<double> retL;
	std::vector<double> lnDATA = ElementwiseLn(zDATA);
	double Avrg = ComputeAverage(lnDATA);
	double StDev = ComputeStdDev(lnDATA);
	for (size_t i = 0; i < zDATA.size(); ++i) {
		double x = abs((lnDATA[i] - Avrg) / StDev);
		double P = zDATA.size()*(1 / sqrt(2 * M_PI))*exp(-x * x / 2);
		if (P > 0.5) {
			retL.push_back(zDATA[i]);
		};
	};
	return retL;
};
