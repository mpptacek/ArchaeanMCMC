#include "stdafx.h"
#include "reconResultsProcessors.h"
#include "reconManager.h"
#include "pyLib.h"

#include "WRB.h"

//#define LOG_MCMC_STATE 1 

namespace MCMCRecon {
	//MCMC parameters
	const double JUMP_SZ = 0.03;
	const size_t MC_ITER = 1500000;
	const size_t MC_BURN = MC_ITER / 5;

	// Define various behaviours for different stages of the MCMC reconstruction pipeline
	namespace Behaviour {
		//MCMC new state generator: Arbitrary number of endmembers
		template<int Ne>
		inline MixState<Ne> StateGenerator_N(const MixState<Ne>& curFit) {
			MixState<Ne> newFit;
			double sum = 0.0;
			for (int i = 1; i < Ne; ++i) { //Start at one, not zero!
				newFit[i] = Random::NormDstr(curFit[i], JUMP_SZ);
				sum += newFit[i];
			};
			newFit[0] = 1.0 - sum;
			return newFit;
		};

		//MCMC constraints verifier: Arbitrary number of endmembers
		template<int Ne>
		inline bool ConstraintsVerifier_N(const MixState<Ne>& newFit) {
			for (int i = 0; i < Ne; ++i) {
				if (newFit[i] < 0.0 || newFit[i] > 1.0) {
					return false;
				};
			}
			return true;
		};
	};

	// Initialises the shale arrays for a given timestep
	// Returns false if this timestep should be skipped (due to insufficient data)
	inline bool InitShaleData(const ReconManager& RM, double t, double* gShale, double* gShaleErr) {
		for (size_t i = 0; i < RM.CountRatios(); ++i) {
			gShale[i] = RM.bestF[i](t);
			gShaleErr[i] = RM.errMF[i](t);
			if (!(std::isfinite(RM.bestF[i](t)) &&
				  std::isfinite(RM.errMF[i](t)))) {
				return false;
			};
		};
		return true;
	};

	// Initialises endmember arrays for a given timestep
	// 
	template<int Ne>
	inline void InitEndmemberData(const ReconManager& RM, double t, Endmembers* e, double* endNmntr, double* endDmntr, double* endErr) {
		const size_t Nsys = RM.CountRatios();
		e->RecalculateForTime(t);
		for (size_t i = 0; i < Nsys; ++i) {
			for (size_t j = 0; j < Ne; ++j) {
				endNmntr[i*Ne + j] = RM.Nmntr[i](e->E[j]);
				endDmntr[i*Ne + j] = RM.Dmntr[i](e->E[j]);
			};
		};
		//Load standard errors of endmember ratios
		for (size_t i = 0; i < Nsys; ++i) {
			for (size_t j = 0; j < Ne; ++j) {
				endErr[i*Ne + j] = e->ratioErr[j][i];
			};
		};
	};

	//Determine, via the Metropolis algorithm, whether the Markov Chain transitions into a new state or not
	inline bool MetropolisAccept(double curChi2, double newChi2) {
		double r = exp(-newChi2 + curChi2);
		double n = Random::Double();
		return (r > n);
	};

	// For a given endmember mix, compute the chi-square statistic
	// Chi square with effective variance
	template<int Ne>
	inline double Chi2(const MixState<Ne>& fitE, const double* obs, const double* sErr, const double* eNmntr, const double* eDmntr, const double* eErr, const size_t Nsys) {
		double acc = 0.0;
		//To compute chi2, consider one ratio at a time
		for (size_t i = 0; i < Nsys; ++i) {
			//Compute ratio value predicted by the mixing model
			double wE[Ne];
			double wSum = 0.0;
			double model = 0.0;
			for (int j = 0; j < Ne; ++j) {
				model += fitE[j] * eNmntr[i*Ne + j];
				wE[j] =  fitE[j] * eDmntr[i*Ne + j];
				wSum += wE[j];
			};
			model /= wSum;
			//Compute misfit & variance (note: all 'errors' are std.devs, hence we need to square)
			double misfit = model - obs[i];
			misfit *= misfit;
			double var = (sErr[i] * sErr[i]);
			for (int j = 0; j < Ne; ++j) {
				var += (wE[j] / wSum)*(wE[j] / wSum)*(eErr[i*Ne + j] * eErr[i*Ne + j]);
			};
			//Add current ratio's contribution to the overall accumulator
			acc += misfit / var;
		};
		return acc;
	};

	// The inner loop of the MCMC procedure
	// The constraints and new state functions can be fully customized via templating
	template<int Ne,
		MixState<Ne>(*GEN_NEW_STATE)(const MixState<Ne>&),
		bool(*CONSTRAINT_PASS)(const MixState<Ne>&)>
	size_t inline MCMC_INNER_LOOP(MixState<Ne>& bestFit, std::vector<MixState<Ne>>& mixStates, double* gShale, double* gShaleErr, double* endNmntr, double* endDmntr, double* endErr, size_t Nsys) {
		MixState<Ne> initialFit = MixState<Ne>::Default();
		bestFit = initialFit;
		double bestChi2 = Chi2<Ne>(initialFit, gShale, gShaleErr, endNmntr, endDmntr, endErr, Nsys);
		MixState<Ne> curFit, newFit;
		double curChi2, newChi2;
		curFit = bestFit;
		curChi2 = bestChi2;
		size_t acceptances = 0;
		for (size_t mc = 0; mc < MC_ITER; ++mc) {
			//Generate a new proposal for the data which fits the hard constraints
			do { newFit = GEN_NEW_STATE(curFit); }
			while (!CONSTRAINT_PASS(newFit));
			//Compute Chi2 of new proposal
			newChi2 = Chi2<Ne>(newFit, gShale, gShaleErr, endNmntr, endDmntr, endErr, Nsys);
			//Use the Metropolis criterion to determine if the Markov Chain transitions or not
			if (MetropolisAccept(curChi2, newChi2)) {
				curFit = newFit;
				curChi2 = newChi2;
				++acceptances;
				//Keep track of the lowest chi2 value, update bestFit parameter accordingly
				if (curChi2 < bestChi2) {
					bestChi2 = curChi2;
					bestFit = curFit;
				};
			};
			//Record state of the Markov Chain
			mixStates[mc] = curFit;
		};
		return acceptances;
	};

	//Full MCMC timeline reconstruction
	template<int Ne,
			typename RESULTS_PROCESSOR>
	RESULTS_PROCESSOR inline RunMarkovModel_Impl(const ReconManager& RM) {
		std::cout << "CRUSTAL MCMC REE-CONSTRUCTION INITIATED." << std::endl;

		RESULTS_PROCESSOR results(RM);
		size_t Nsys =        RM.CountRatios();
		Endmembers* e =      RM.E;
		double TIME_START =  4000.0;
		double TIME_END =    0.0;
		double RES =         10.0;
		double REPORT_FREQ = 100.0;
		double last_report = INFINITY;
		double* gShale =     new double[Nsys];
		double* gShaleErr =  new double[Nsys];
		double* endNmntr =   new double[Ne * Nsys];
		double* endDmntr =   new double[Ne * Nsys];
		double* endErr =     new double[Ne * Nsys];
		std::vector<MixState<Ne>> mixStates(MC_ITER);
		std::vector<double>      chi2States(MC_ITER);
		MixState<Ne>              bestFit;

		for (double t = TIME_START; t > TIME_END; t -= RES) {
			//Generate global representative shale at time t, and get its standard error (acquired from the bootstraps)
			if (!InitShaleData(RM, t, gShale, gShaleErr)) {
				continue; //If we encountered a NaN value, it means we have no data - so skip this timestep altogether!
			};
			//Report progress
			if (last_report - t > REPORT_FREQ - 0.001) {
				std::cout << "t: " << t << "Ma" << std::endl;
				last_report = t;
			};

			//Generate endmembers, and their standard errors
			InitEndmemberData<Ne>(RM, t, e, endNmntr, endDmntr, endErr);

			//Run MCMC 
			size_t acceptances = MCMC_INNER_LOOP<Ne,
									&Behaviour::StateGenerator_N<Ne>,
									&Behaviour::ConstraintsVerifier_N<Ne>>(bestFit, mixStates,
																		   gShale, gShaleErr,
																		   endNmntr, endDmntr,
																		   endErr, Nsys);

#ifdef LOG_MCMC_STATE
			//DEBUG: Output run of MC!
			csvWriter<MixState> w;
			w.RegisterEntry("K", OFF(MixState::K)).RegisterEntry("M", OFF(MixState::M)).RegisterEntry("F", OFF(MixState::F));
			w.Write("MCMC_OUT_t"+std::to_string(t),mixStates);
#endif
			//Register the Earth's state at this time
			double acceptance_ratio = ((double)acceptances) / ((double)MC_ITER);
			results.Record(t, bestFit, mixStates, *e, MC_BURN, acceptance_ratio);
		};

		delete[] gShale;
		delete[] gShaleErr;
		delete[] endNmntr;
		delete[] endDmntr;
		delete[] endErr;
		return results;
	};

	//Interface functions
	std::string RunMarkovModel_2M(const ReconManager& RM) {
		return RunMarkovModel_Impl<2, ResultsProcessor_Endmembers<2>>(RM).Results2CSV();
	};
	std::string RunMarkovModel_3M(const ReconManager& RM) {
		return RunMarkovModel_Impl<3, ResultsProcessor_Endmembers<3>>(RM).Results2CSV();
	};
	std::string RunMarkovModel_4M(const ReconManager& RM) {
		return RunMarkovModel_Impl<4, ResultsProcessor_Endmembers<4>>(RM).Results2CSV();
	};
	std::string RunMarkovModel_5M(const ReconManager& RM) {
		return RunMarkovModel_Impl<5, ResultsProcessor_Endmembers<5>>(RM).Results2CSV();
	};
	std::string RunMarkovModel_2M_Ratios(const ReconManager& RM) {
		return RunMarkovModel_Impl<2, ResultsProcessor_Ratios<2>>(RM).Results2CSV();
	};
	std::string RunMarkovModel_3M_Ratios(const ReconManager& RM) {
		return RunMarkovModel_Impl<3, ResultsProcessor_Ratios<3>>(RM).Results2CSV();
	};
	std::string RunMarkovModel_4M_Ratios(const ReconManager& RM) {
		return RunMarkovModel_Impl<4, ResultsProcessor_Ratios<4>>(RM).Results2CSV();
	};
	std::string RunMarkovModel_5M_Ratios(const ReconManager& RM) {
		return RunMarkovModel_Impl<5, ResultsProcessor_Ratios<5>>(RM).Results2CSV();
	};

	//Analyse single time only, output ratio cloud
	template<int Ne>
	SingleTimeState SingleTimestepMCMCR(const ReconManager& RM, double t) {
		size_t Nsys = RM.CountRatios();
		Endmembers* e = RM.E;
		double* gShale = new double[Nsys];
		double* gShaleErr = new double[Nsys];
		double* endNmntr = new double[Ne * Nsys];
		double* endDmntr = new double[Ne * Nsys];
		double* endErr = new double[Ne * Nsys];
		std::vector<MixState<Ne>> mixStates(MC_ITER);
		MixState<Ne> bestFit;

		//Initialise shale & endmember data
		InitShaleData(RM, t, gShale, gShaleErr);
		InitEndmemberData<Ne>(RM, t, e, endNmntr, endDmntr, endErr);

		//Run MCMC 
		MCMC_INNER_LOOP<Ne,
			&Behaviour::StateGenerator_N<Ne>,
			&Behaviour::ConstraintsVerifier_N<Ne>>(bestFit, mixStates,
												   gShale, gShaleErr,
												   endNmntr, endDmntr,
												   endErr, Nsys);

		//Calculate endmember confidence intervals
		std::vector<double> bestFitState;
		std::vector<double> p025;
		std::vector<double> p975;
		std::vector<double> tempVec;
		tempVec.reserve(mixStates.size());
		for (size_t idx = 0; idx < Ne; ++idx) {
			tempVec.clear();
			for (size_t mc = MC_BURN; mc < mixStates.size(); ++mc) {
				tempVec.push_back(mixStates[mc][idx]);
			};
			std::sort(tempVec.begin(), tempVec.end());
			p025.push_back(SortedVectorPercentile(tempVec, 2.5));
			p975.push_back(SortedVectorPercentile(tempVec, 97.5));
			bestFitState.push_back(bestFit[idx]);
		};
		
		//Select MCMC states to subsample
		size_t SAMPLE_SZ = 10000;
		std::vector<size_t> sampleIdx(SAMPLE_SZ);
		for (size_t i = 0; i < SAMPLE_SZ; ++i) {
			sampleIdx[i] = Random::Int64(MC_BURN, MC_ITER - 1);
		};

		//Record each ratio system in turn
		SingleTimeState mS;
		mS.best = bestFitState;
		mS.endmemberP025 = p025;
		mS.endmemberP975 = p975;
		
		for (size_t rIdx = 0; rIdx < Nsys; ++rIdx) {
			SingleTimeState::RatioSystem& r = mS.Add();

			//Lambda to compute the value of this ratio for a given MixState
			auto MIXTURE = [&](const MixState<Ne>& ST)->double {
				double sumA = 0.0;
				double sumB = 0.0;
				for (size_t j = 0; j < Ne; ++j) {
					sumA += ST[j] * endNmntr[j + Ne * rIdx];
					sumB += ST[j] * endDmntr[j + Ne * rIdx];
				};
				return sumA / sumB;
			};

			//Record the shale predictions & the best-fitting mixture
			r.bootR = gShale[rIdx];
			r.bestR = MIXTURE(bestFit);

			//Record the endmember concentrations
			for (size_t j = 0; j < Ne; ++j) {
				r.cA.push_back(endNmntr[j + Ne * rIdx]);
				r.cB.push_back(endDmntr[j + Ne * rIdx]);
			};

			//Record the MCMC subsample
			r.mcmcR.reserve(SAMPLE_SZ);
			for (size_t sample : sampleIdx) {
				r.mcmcR.push_back(MIXTURE(mixStates[sample]));
			};
		};

		delete[] gShale;
		delete[] gShaleErr;
		delete[] endNmntr;
		delete[] endDmntr;
		delete[] endErr;

		return mS;
	};
#ifdef PYTHON_LIB
	SingleTimeState MCMC_SingleTimeTest_WRB(const ReconManager& RM, double t) {
		switch (RM.GetEndmemberCount()) {
		case 2:
			return SingleTimestepMCMCR<2>(RM, t);
		case 3:
			return SingleTimestepMCMCR<3>(RM, t);
		case 4:
			return SingleTimestepMCMCR<4>(RM, t);
		case 5:
			return SingleTimestepMCMCR<5>(RM, t);
		default:
			throw std::runtime_error("Incompatible endmember count for single-step MCMCR!");
		};
	};
	PYTHON_LINK_FUNCTION(MCMC_SingleTimeTest_WRB);
#endif
};

namespace {
	REGISTER_MODULE(MCMCReecon);
	MODULE_DEPENDENCY(MCMCReecon, CommonDBs);
	MODULE_DEPENDENCY(MCMCReecon, ShaleDB);

	void MCMCReecon::Exec() {
		DenseStringMap conf;
		conf.Insert("rA", {"Th", "Sc"});
		conf.Insert("rB", {"Ni", "Co"});
		conf.Insert("r", {"Th/Sc", "Ni/Co"});
		conf.Insert("AgeBinWidth", "500");
		conf.Insert("BootstrapKernelWidth", "400");
		conf.Insert("endmemberMode", "Continuous");
		//conf.Insert("endmemberMode", "Dual");
		conf.Insert("reconMode", "MCMC");
		conf.Insert("endmemberScript", "KMF");
		conf.Insert("ContinuousBinWidth", "500");
		//conf.Insert("detailedRatioPrinter", "Tm/Cu");
		
		using namespace MCMCRecon;
		ReconManager RM(conf);
		RM.GenerateAllBootstraps();
		auto eStates = RM.RunReconstruction();
		std::cout << eStates;
	};
};
