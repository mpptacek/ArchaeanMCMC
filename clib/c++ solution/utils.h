#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <dirent.h>

/// ...................................................................................................................
/// 64-bit integer (signed & unsiged)
/// 
/// ...................................................................................................................
typedef long long int64;
typedef unsigned long long uint64;

/// ...................................................................................................................
/// The vector class
/// 
/// ...................................................................................................................
template<int D>
struct Vect {
private:
	double v[D];
public:
	double& operator[](size_t idx) {
		return v[idx];
	};

	double operator[](size_t idx) const {
		return v[idx];
	};
	
	typedef std::vector< Vect<D> > List;
	
	static std::vector<double> Sublist(const List& L, unsigned int IDX) {
		std::vector<double> retVal;
		for(unsigned int i= 0; i<L.size();++i) {
			retVal.push_back(L[i].v[IDX]);
		};
		return retVal;
	};

	Vect(std::initializer_list<double> iL) {
		auto it = iL.begin();
		for (unsigned int i = 0; i < D; ++i) {
			v[i] = (*it);
			++it;
		};
	};
	Vect() {};
};

/// ...................................................................................................................
/// The VectorMap class
/// 
/// ...................................................................................................................
//This class can be accessed like a map (albeit with o(n) lookup time), or iterated through like a vector.
//Crucially, the order of the values is always the same as the order of insertion.
template<typename KEY, typename DAT>
class VectorMap {
	typedef std::vector<std::pair<KEY, DAT>> STORE;
	typedef typename STORE::iterator ITER;
	typedef typename STORE::const_iterator CITER;
	STORE d;
	static const size_t BAD_IDX = (size_t)(-1);

public:
	DAT& operator[](const KEY& k) {
		for (size_t i = 0; i < d.size(); ++i) {
			if (k == d[i].first) {
				return d[i].second;
			};
		};
		d.push_back(std::pair<KEY, DAT>(k,DAT()));
		return d.back().second;
	};
	DAT& operator[](KEY&& k) {
		for (size_t i = 0; i < d.size(); ++i) {
			if (k == d[i].first) {
				return d[i].second;
			};
		};
		d.push_back(std::pair<KEY, DAT>(k, DAT()));
		return d.back().second;
	};

	const std::pair<KEY, DAT>& at(size_t idx) {
		return d[idx];
	};

	void resize(size_t n) {
		d.resize(n);
	};
	void reserve(size_t n) {
		d.reserve(n);
	};
	size_t size() noexcept {
		return d.size();
	};
	ITER begin() noexcept {
		return d.begin();
	};
	ITER end() noexcept {
		return d.end();
	};
	CITER cbegin() const noexcept {
		return d.cbegin();
	};
	CITER cend() const noexcept {
		return d.cend();
	};
	ITER rbegin() noexcept {
		return d.rbegin();
	};
	ITER rend() noexcept {
		return d.rend();
	};
	CITER crbegin() const noexcept {
		return d.crbegin();
	};
	CITER crend() const noexcept {
		return d.crend();
	};

	size_t indexof(const std::string& k) const noexcept {
		for (size_t i = 0; i < d.size(); ++i) {
			if (k == d[i].first) {
				return i;
			};
		};
		return BAD_IDX;
	};

	bool contains(const std::string& k) const noexcept {
		return (indexof(k) != BAD_IDX);
	};

	void MergeIn(const VectorMap<KEY,DAT>& o) {
		for (auto IT = o.cbegin(); IT != o.cend(); ++IT ) {
			//d.push_back(std::pair<KEY, DAT>((*IT).first, (*IT).second));
			(*this)[(*IT).first] = (*IT).second; //Less efficient than the above alternative, but guarantees zero duplicate entries
		};
	};
};

/// ...................................................................................................................
/// Initialiser list manipulation
/// 
/// ...................................................................................................................
template<typename T>
std::vector<T> VectorFromIList(std::initializer_list<T> il) {
	std::vector<T> rVec;
	for (auto e : il) {
		rVec.push_back(e);
	};
	return rVec;
};

template<unsigned int I>
typename Vect<I>::List VectListFromIList(std::initializer_list<std::initializer_list<double>> il) {
	typename Vect<I>::List rVec;
	for (auto v : il) {
		Vect<I> V;
		unsigned int i = 0;
		for (auto e : v) {
			V.v[i] = e;
			++i;
		};
		rVec.push_back(V);
	};
	return rVec;
};

/// ...................................................................................................................
/// String & filepath manipulation
/// 
/// ...................................................................................................................
std::vector<std::string> listfiles(std::string path,std::string REQ_EXT="");

std::string ToFilenameString(const std::string& S);

const std::string DefaultPath();

const std::string DefaultWritePath();

void RemoveSubstring(std::string&, const std::string& );

/// ...................................................................................................................
/// Random number generation
/// 
/// ...................................................................................................................
namespace Random {
	//Produces a random (32 bit) integer; CAN produce both lowerBound & upperBound!
	int Int32(int lowerBound, int upperBound);

	//Produces a random (64 bit) integer; CAN produce both lowerBound & upperBound!
	int64 Int64(int64 lowerBound, int64 upperBound);

	//Produces a random double between 0.0 and 1.0
	double Double();

	//Produces a random double drawn from the standard distribution (mean=0, var= 1)
	double StdDstr();

	//Produces a random double drawn from an arbitrary normal distribution
	double NormDstr(double mean, double sigma);
};
/// ...................................................................................................................
/// Container print functions
/// 
/// ...................................................................................................................
template <typename T0>
void Print(const std::vector<T0>& ML) {
	for(typename std::vector<T0>::const_iterator it= ML.begin(); it!= ML.end(); ++it) {
		std::cout << "*" << (*it) << std::endl;
	};
};

template <typename T0>
void PrintInline(const std::vector<T0>& ML) {
	for(typename std::vector<T0>::const_iterator it= ML.begin(); it!= ML.end(); ++it) {
		std::cout << "[" << (*it) << "] ";
	};
	std::cout << std::endl;
};

template <typename T1, typename T2>
void Print(const std::map<T1,T2>& MP) {
	for(typename std::map<T1,T2>::const_iterator it= MP.begin(); it!= MP.end(); ++it) {
		std::cout << "*" << (*it).first << ", " << (*it).second << std::endl;
	};
};

template <typename T1, typename T2>
void PrintInline(const std::map<T1,T2>& MP) {
	for(typename std::map<T1,T2>::const_iterator it= MP.begin(); it!= MP.end(); ++it) {
		std::cout << (*it).first << " [" << (*it).second << "] ";
	};
	std::cout << std::endl;
};

/// ...................................................................................................................
/// Member offset classes
/// 
/// ...................................................................................................................
#include "MemberOffset.h"

/// ...................................................................................................................
/// String deserialisation functions
/// 
/// ...................................................................................................................
//Without the inline statements, gcc throws a fit
template<typename T>
inline T StringToData(const std::string& S) {
	throw std::runtime_error("Could not find a suitable StringToData() overload.");
	return *(T*)(nullptr);
};
template<>
inline double StringToData<double>(const std::string& S) {
	return atof(S.c_str());
};
template<>
inline int StringToData<int>(const std::string& S) {
	return atoi(S.c_str());
};
template<>
inline unsigned int StringToData<unsigned int>(const std::string& S) {
	return atoi(S.c_str());
};
template<>
inline size_t StringToData<size_t>(const std::string& S) {
	return atol(S.c_str());
};
template<>
inline std::string StringToData<std::string>(const std::string& S) {
	return S;
};
template<>
inline std::string* StringToData<std::string*>(const std::string& S) {
	return new std::string(S);
};
//Given a void* to an instance of class MBR, and a MemoryOffset corresponding to a member of MBR, this
//function converts the input string into type DTYPE (using the specialised template set StringToData<>)
//and then writes it into the appropriate member of the instance provided.
template<class MBR, typename DTYPE>
void WriteData(MemberOffsetBase& mOff, void* target, const std::string& STR) {
	MemberOffset<MBR, DTYPE> mo(mOff);
	MBR* obj = (MBR*)target;
	mo.DataR(obj) = StringToData<DTYPE>(STR);
};

/// ...................................................................................................................
/// Useful ancillary macros
/// 
/// ...................................................................................................................
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
