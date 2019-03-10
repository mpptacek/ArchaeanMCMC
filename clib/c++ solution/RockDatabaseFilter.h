#pragma once
#include "RockDatabase.h"

template<typename DATA, class DB>
std::vector<DATA> VectorFromDB(DB db, MemberOffset<DATA, double> dat) {
	std::vector<DATA> rV(db.Data().size());
	for (typename DB::ITconst it = db.Data().begin(); it != db.Data().end(); ++it) {
		rV.push_back(dat.Data(&(*it)));
	};
	return rV;
};

template<typename T>
struct ValueRangeFilter : public RockDatabase::Filter {
	T MinVal;
	T MaxVal;
	MemberOffset<RockSample, T> off;

	virtual bool Test(const RockSample& R) const {
		if (!std::isfinite(off.Data(R)))
			return false;
		if (off.Data(R)>MaxVal)
			return false;
		if (off.Data(R)<MinVal)
			return false;
		return true;
	};

	ValueRangeFilter(MemberOffset<RockSample, T> MO, T MIN_VAL, T MAX_VAL) : off(MO), MinVal(MIN_VAL), MaxVal(MAX_VAL) {};;
};
typedef ValueRangeFilter<double> RangeFilter;

template<typename T>
struct IdentityFilter : public RockDatabase::Filter {
	T val;
	MemberOffset<RockSample, T> off;

	virtual bool Test(const RockSample& R) const {
		return (off.Data(R) == val);
	};

	IdentityFilter(MemberOffset<RockSample, T> MO, T VAL) : val(VAL), off(MO) {};
};

template<typename T>
struct NOTFilter : public RockDatabase::Filter {
	T val;
	MemberOffset<RockSample, T> off;

	virtual bool Test(const RockSample& R) const {
		return (off.Data(R) != val);
	};

	NOTFilter(MemberOffset<RockSample, T> MO, T VAL) : val(VAL), off(MO) {};
};

template<typename T>
struct ORFilter : public RockDatabase::Filter {
	std::vector<T> vals;
	MemberOffset<RockSample, T> off;

	virtual bool Test(const RockSample& R) const {
		for (unsigned int i = 0; i<vals.size(); ++i) {
			if (off.Data(R) == vals[i])
				return true;
		};
		return false;
	};

	ORFilter(MemberOffset<RockSample, T> MO, T V0) : off(MO) {
		vals.push_back(V0);
	};
	ORFilter(MemberOffset<RockSample, T> MO, T V0, T V1) : off(MO) {
		vals.push_back(V0); vals.push_back(V1);
	};
	ORFilter(MemberOffset<RockSample, T> MO, T V0, T V1, T V2) : off(MO) {
		vals.push_back(V0); vals.push_back(V1); vals.push_back(V2);
	};
	ORFilter(MemberOffset<RockSample, T> MO, T V0, T V1, T V2, T V3) : off(MO) {
		vals.push_back(V0); vals.push_back(V1); vals.push_back(V2); vals.push_back(V3);
	};
	ORFilter(MemberOffset<RockSample, T> MO, T V0, T V1, T V2, T V3, T V4) : off(MO) {
		vals.push_back(V0); vals.push_back(V1); vals.push_back(V2); vals.push_back(V3);
		vals.push_back(V4);
	};
	ORFilter(MemberOffset<RockSample, T> MO, T V0, T V1, T V2, T V3, T V4, T V5) : off(MO) {
		vals.push_back(V0); vals.push_back(V1); vals.push_back(V2); vals.push_back(V3);
		vals.push_back(V4); vals.push_back(V5);
	};
	ORFilter(MemberOffset<RockSample, T> MO, T V0, T V1, T V2, T V3, T V4, T V5, T V6) : off(MO) {
		vals.push_back(V0); vals.push_back(V1); vals.push_back(V2); vals.push_back(V3);
		vals.push_back(V4); vals.push_back(V5); vals.push_back(V6);
	};
	ORFilter(MemberOffset<RockSample, T> MO, T V0, T V1, T V2, T V3, T V4, T V5, T V6, T V7) : off(MO) {
		vals.push_back(V0); vals.push_back(V1); vals.push_back(V2); vals.push_back(V3);
		vals.push_back(V4); vals.push_back(V5); vals.push_back(V6); vals.push_back(V7);
	};
};

struct FuncFilter : public RockDatabase::Filter {
	typedef bool(*IS_GOOD)(const RockSample& R);
	IS_GOOD pred;

	virtual bool Test(const RockSample& R) const {
		return pred(R);
	};
	FuncFilter(IS_GOOD func) : pred(func) {};
};

struct NanFilter : public RockDatabase::Filter {
	MemberOffset<RockSample, double> off;

	virtual bool Test(const RockSample& R) const {
		return std::isfinite(off(R));
	};
	NanFilter(MemberOffset<RockSample, double> MO) : off(MO) {};
};

struct NullFilter : public RockDatabase::Filter {
	virtual bool Test(const RockSample& R) const {
		return true;
	};
	NullFilter() {};
};

struct RandomFilter : public RockDatabase::Filter {
	double probSuccess;

	virtual bool Test(const RockSample& R) const {
		if (probSuccess>Random::Double())
			return true;
		return false;
	};

	RandomFilter(double PROB_SUCCESS) : probSuccess(PROB_SUCCESS) {};
};
