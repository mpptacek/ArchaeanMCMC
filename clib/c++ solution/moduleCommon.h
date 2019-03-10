#pragma once
#include "utils.h"
#include "RockSample.h"
#include "module.h"
#include "csvParser.h"
#include "csvWriter.h"
#include "SpecialisedRockDatabase.h"
#include "RockDatabaseFilter.h"
#include "Model.h"
#include "Analysis.h"

typedef MixingModel<RockSample> RockMixingModel;

//#define _USE_CHARAC_FILTER
#define _USE_CHARAC_STRICT
bool isGood_CharacTest(const RockSample& R);

double SumMajors(const RockSample & R);

bool IsGood(const RockSample& R);
bool IsMostlyGood(const RockSample& R, double tolerance);

struct IsGoodish : RockDatabase::Filter {
	double tol;
	virtual bool Test(const RockSample & R) const override;
	IsGoodish(double TOLERANCE) : tol(TOLERANCE) {};
};

class RatioOffset : public FunctorOffset<RockSample, double> {
	MemberOffset<RockSample, double> A;
	MemberOffset<RockSample, double> B;
public:
	double Access(RockSample* P) override;
	double Access(const RockSample* P) const override;

	RatioOffset(MemberOffset<RockSample, double> a, MemberOffset<RockSample, double> b);

	operator MemberOffsetBase() const {
		return ConvertFunctor2Base(this);
	};
	operator MemberOffset<RockSample, double>() const {
		return ConvertFunctor2Off<RockSample, double>(this);
	};
};
