#include "stdafx.h"
#include "WRB.h"
#include "utils.h"

//Resolution of all discretised functions
const size_t RES = 250;

//Get maximum and minimum values of an array
inline double ArrMax(size_t N, const double* ARR) noexcept {
	double X = ARR[0];
	for (size_t i = 0; i < N; ++i) {
		X = std::max(X, ARR[i]);
	};
	return X;
};
inline double ArrMin(size_t N, const double* ARR) noexcept {
	double X = ARR[0];
	for (size_t i = 0; i < N; ++i) {
		X = std::min(X, ARR[i]);
	};
	return X;
};

//Exponential kernel helper object
struct Kernel {
	double s; //Wavelength of kernel
	double operator()(double t0, double t1) const noexcept {
		double deltaX = t0 - t1;
		return (1 / (sqrt(2 * M_PI)*s))*exp(-(deltaX*deltaX) / (2 * s*s));
	};
	Kernel(double WIDTH) : s(WIDTH) {};
};

//Apply weight kernel to all samples, to generate an array of weights
inline void WeighAll(size_t N, double ageNow, const double* ageArr, double* weights, const Kernel& k) noexcept {
	for (size_t i = 0; i < N; ++i) {
		weights[i] = k(ageNow, ageArr[i]);
	};
};

// Inner loop for the elemental bootstrap
// 
inline double INNER_WEB_POINTCALC(size_t N, const double* W, const double* A, const double* B) {
	double sumA = 0.0;
	double sumW = 0.0;
	for (size_t i = 0; i < N; ++i) {
		sumA += W[i] * A[i];
		sumW += W[i];
	};
	return sumA / sumW;
};

// Inner loop for the ratio bootstrap
//
inline double INNER_WRB_POINTCALC(size_t N, const double* W, const double* A, const double* B) {
	double sumA = 0.0;
	double sumB = 0.0;
	for (size_t i = 0; i < N; ++i) {
		sumA += W[i] * A[i];
		sumB += W[i] * B[i];
	};
	return sumA / sumB;
};

//Create best fit for a single draw of samples
//Implements both the ratio- and elemental- bootstraps
template<double(*INNER_LOOP)(size_t,const double*,const double*,const double*)>
inline void WXB_BestFit(DiscreteFunction& f, size_t N, const double* age, double* weights, const double* ValA, const double* ValB, const Kernel& k) noexcept {
	double ageMax = ArrMax(N, age);
	double ageMin = ArrMin(N, age);
	double ageRng = ageMax - ageMin;
	double stepSize = ageRng / ((double)RES);

	//Apply kernel function to all samples to create weight
	for (double t = ageMin; t < ageMax; t += stepSize) {
		//Use kernel to assign weights
		WeighAll(N, t, age, weights, k);
		//Compute and store result
		double Y = INNER_LOOP(N, weights, ValA, ValB);
		if (std::isfinite(Y)) {
			f.AddNewPoint(t, Y);
		};
	};
	f.Finalise();
};

//Bootstrap sampler for the elemental bootstrap
void SAMPLER_WEB(size_t N, const double* Ag, const double* Ar, const double* Br,
				 double* AgB, double* VAB, double* VBB) {
	//Draw a sample from A values only
	//(inefficient, lots of unsteady memcpy, but eyh it's probs going to work!)
	for (size_t s = 0; s < N; ++s) {
		size_t IDX = Random::Int64(0, N - 1);
		AgB[s] = Ag[IDX];
		VAB[s] = Ar[IDX];
	};
};

//Bootstrap sampler for the ratio bootstrap
void SAMPLER_WRB(size_t N, const double* Ag, const double* Ar, const double* Br,
				 double* AgB, double* VAB, double* VBB) {
	//Draw a sample from both A-values and B-values
	//(inefficient, lots of unsteady memcpy, but eyh it's probs going to work!)
	for (size_t s = 0; s < N; ++s) {
		size_t IDX = Random::Int64(0, N - 1);
		AgB[s] = Ag[IDX];
		VAB[s] = Ar[IDX];
		VBB[s] = Br[IDX];
	};
};

//Handle the bootstrapping and results-reporting logic for either type of bootstrap
//Cannot handle NaNs!
template<double(*INNER_LOOP)(size_t, const double*, const double*,const double*),
void(*SAMPLER)(size_t,const double*,const double*,const double*,double*,double*,double*)>
WRB_Result WXB_Bootstrap(size_t N, const double* Ag, const double* Ar, const double* Br, double kernelWidth, size_t ITER) {
	std::cout << "WRB BOOTSTRAP INIT" << std::endl;
	const Kernel k(kernelWidth);
	WRB_Result res;
	res.bestFit.Reserve(RES + 1);
	res.stdError.Reserve(RES + 1);
	
	//Set up the correct memory arrays and initialise them appropriately,
	//based on whether we are in ratio mode or not.
	double* WgB = new double[N];
	double* AgB = new double[N];
	double* VAB = new double[N];
	double* VBB = new double[N];
	memcpy(AgB, Ag, sizeof(double)*N);
	memcpy(VAB, Ar, sizeof(double)*N);
	if (Br != nullptr) {
		memcpy(VBB, Br, sizeof(double)*N);
	};
	std::vector<double> y;
	y.reserve(ITER);

	//Best fit & bounds computation	
	double ageMax = ArrMax(N, AgB);
	double ageMin = ArrMin(N, AgB);
	WXB_BestFit<INNER_LOOP>(res.bestFit, N, AgB, WgB, VAB, VBB, k);
	std::cout << "..BEST FIT COMPUTED" << std::endl;

	//Confidence interval computation
	std::vector<DiscreteFunction> fs(ITER, DiscreteFunction(RES + 1));
	//Draw samples from data, and generate a best fit for each draw
	size_t lastPrint = 0;
	size_t printFreq = 250;
	if (ITER > 1) {
		for (size_t i = 0; i < ITER; ++i) {
			SAMPLER(N, Ag, Ar, Br, AgB, VAB, VBB);
			//Compute best fit of bootstrapped sample
			WXB_BestFit<INNER_LOOP>(fs[i], N, AgB, WgB, VAB, VBB, k);
			//Update us on status
			if (lastPrint + printFreq <= i) {
				lastPrint = i;
				std::cout << "   ITER " << i << std::endl;
			};
		};

		//Generate best fit and 95% confidence intervals for all points in the age range
		double ageRng = ageMax - ageMin;
		double stepSize = ageRng / ((double)RES);
		std::cout << "..CALC STDEV" << std::endl;
		for (double t = ageMin; t < ageMax; t += stepSize) {
			y.clear();
			for (size_t i = 0; i < ITER; ++i) {
				double v = fs[i](t);
				if (std::isfinite(v)) {
					y.push_back(v);
				};
			};
			res.stdError.AddNewPoint(t, ComputeSampleStdDev(y));
		};
		std::cout << "..FINALISE" << std::endl;
		res.stdError.Finalise();
	};

	//Clean up temporaries, and return
	delete[] WgB;
	delete[] AgB;
	delete[] VAB;
	delete[] VBB;
 	return res;
};

WRB_Result WRB_Bootstrap(const std::vector<double>& age, const std::vector<double>& A, const std::vector<double>& B, double kernelWidth, size_t ITER) {
	return WXB_Bootstrap<&INNER_WRB_POINTCALC, &SAMPLER_WRB>(age.size(), age.data(), A.data(), B.data(), kernelWidth, ITER);
};

WRB_Result WEB_Bootstrap(const std::vector<double>& age, const std::vector<double>& A, double kernelWidth, size_t ITER) {
	return WXB_Bootstrap<&INNER_WEB_POINTCALC, &SAMPLER_WEB>(age.size(), age.data(), A.data(), nullptr, kernelWidth, ITER);
};

double WRB_Result::Percentile975(double x) const {
	return bestFit(x) + 2 * stdError(x);
};

double WRB_Result::Percentile025(double x) const {
	return bestFit(x) - 2 * stdError(x);
};
