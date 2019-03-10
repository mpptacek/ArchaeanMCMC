#include "stdafx.h"
#include "moduleCommon.h"

bool isGood_CharacTest(const RockSample& R) {
#ifdef _USE_CHARAC_FILTER
#ifdef _USE_CHARAC_STRICT
	if (!std::isfinite(R.ZrHf())) return false;
	if (!std::isfinite(R.NbTa())) return false;
#endif
	if (R.ZrHf()>(32 * 1.2)) return false;
	if (R.ZrHf()<(32 * 0.8)) return false;
	if (R.NbTa()>(17.5*1.5)) return false;
	if (R.NbTa()<(17.5*0.5)) return false;
#endif
	return true;
};

double SumMajors(const RockSample& R) {
	double mElSum = 0.0;
	if (std::isfinite(R.Silicate))
		mElSum += R.Silicate;
	if (std::isfinite(R.Magnesium))
		mElSum += R.Magnesium;
	if (std::isfinite(R.Aluminium))
		mElSum += R.Aluminium;
	if (std::isfinite(R.Titanium))
		mElSum += R.Titanium;
	if (std::isfinite(R.IronTotal))
		mElSum += R.IronTotal;
	if (std::isfinite(R.Manganese))
		mElSum += R.Manganese;
	if (std::isfinite(R.Calcium))
		mElSum += R.Calcium;
	if (std::isfinite(R.Sodium))
		mElSum += R.Sodium;
	if (std::isfinite(R.Potassium))
		mElSum += R.Potassium;
	if (std::isfinite(R.Phosphorous))
		mElSum += R.Phosphorous;
	return mElSum;
};

bool IsMostlyGood(const RockSample& R, double tolerance) {
	double mElSum = SumMajors(R);
	//mElSum = R.Silicate + R.Magnesium + R.Aluminium+ R.Titanium + R.IronTotal + R.Manganese + R.Calcium + R.Sodium + R.Potassium + R.Phosphorous;
	//if (!std::isfinite(mElSum) return false;
	if (mElSum < (100 - tolerance)) return false;
	if (mElSum > (100 + tolerance)) return false;
	if (!isGood_CharacTest(R)) return false;
	return true;
};

bool IsGood(const RockSample& R) {
	return IsMostlyGood(R, 1.0);
};

bool IsGoodish::Test(const RockSample & R) const {
	return IsMostlyGood(R, tol);
};

inline double RatioOffset::Access(RockSample * P) { return A.Data(P) / B.Data(P); };

inline double RatioOffset::Access(const RockSample * P) const { return A.Data(P) / B.Data(P); };

RatioOffset::RatioOffset(MemberOffset<RockSample, double> a, MemberOffset<RockSample, double> b) : A(a), B(b) {};
