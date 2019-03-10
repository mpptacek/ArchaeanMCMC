#pragma once
#include "reconCommon.h"
#include "reconManager.h"

class ResultsProcessor_Generic {
protected:
	bool logAcceptanceRatio;
	ResultsProcessor_Generic(const DenseStringMap& conf);
};

//Reconstruction results processor: Only prints confidence intervals for endmembers
template<int N>
class ResultsProcessor_Endmembers : public ResultsProcessor_Generic {
	struct EarthState {
		double time;
		double mcmc_acceptance;
		MixState<N> mean;
		MixState<N> p975;
		MixState<N> p025;
		RockSample bestFit;
	};

	std::vector<EarthState> V;
	const ReconManager* rm;
public:
	void Record(double t, const MixState<N>& bestFit, const std::vector<MixState<N>>& states, const Endmembers& e, size_t skip_records = 0, double accept_ratio = 0.0) {
		if ((!logAcceptanceRatio) || (accept_ratio > 0)) {
			EarthState es;
			es.time = t;
			es.mean = bestFit;

			//Compute the 2.5th, and 97.5th percentiles for each endmember's contribution.
			std::vector<double> tempVec;
			tempVec.reserve(states.size());
			for (size_t idx = 0; idx < N; ++idx) {
				tempVec.clear();
				for (size_t mc = skip_records; mc < states.size(); ++mc) {
					tempVec.push_back(states[mc][idx]);
				};
				std::sort(tempVec.begin(), tempVec.end());
				es.p025[idx] = SortedVectorPercentile(tempVec, 2.5);
				es.p975[idx] = SortedVectorPercentile(tempVec, 97.5);
			};

			es.bestFit.Age = t;
			es.mcmc_acceptance = accept_ratio;
			for (auto& E : RockSample::allElements) {
				E.second.DataR(es.bestFit) = 0.0;
				for (size_t idx = 0; idx < N; ++idx) {
					E.second.DataR(es.bestFit) += es.mean[idx] * E.second.Data(e.E[idx]);
				};
			};
			V.push_back(es);
		};
	};

	std::string Results2CSV() {
		std::stringstream ss;
		ss << "TIME(/MYR),";
		//Generate column names for endmembers
		for (size_t idx = 0; idx < N; ++idx) {
			ss << rm->GetEndmemberName(idx) << ",";
		};
		const char* suffix[] = {"025", "975"};
		for (size_t idx = 0; idx < N; ++idx) {
			for (size_t i = 0; i < ARRAY_SIZE(suffix); ++i) {
				ss << "ERR_" << rm->GetEndmemberName(idx) << suffix[i] << ",";
			};
		};
		if (logAcceptanceRatio) {
			ss << "MCMC_ACCEPT%,";
		};
		for (auto& E : RockSample::allElements) {
			ss << E.first << ",";
		};
		ss << std::endl;
		for (const auto& es : V) {
			ss << es.time << ",";
			for (size_t idx = 0; idx < N; ++idx) {
				ss << 100 * es.mean[idx] << ",";
			};
			for (size_t idx = 0; idx < N; ++idx) {
				ss << 100 * es.p025[idx] << "," << 100 * es.p975[idx] << ",";
			};
			if (logAcceptanceRatio) {
				ss << 100 * es.mcmc_acceptance << ",";
			};
			for (auto& E : RockSample::allElements) {
				ss << std::to_string(E.second.Data(es.bestFit)) << ",";
			};
			ss << std::endl;
		};
		return ss.str();
	};

	ResultsProcessor_Endmembers(const ReconManager& RM) : ResultsProcessor_Generic(RM.GetInitConfig()), rm(&RM) {};
};

//Reconstruction results processor: Prints confidence intervals for all ratios in analysis
template<int N>
class ResultsProcessor_Ratios : public ResultsProcessor_Generic {
	struct EarthState {
		double time;
		double mcmc_acceptance;
		std::vector<double> bestFit;
		std::vector<double> p975;
		std::vector<double> p025;
	};

	std::vector<EarthState> V;
	std::vector<std::string> logRatioNames;
	std::vector<DataOffset<RockSample, double>> logRatioA;
	std::vector<DataOffset<RockSample, double>> logRatioB;
public:
	void Record(double t, const MixState<N>& bestFit, const std::vector<MixState<N>>& states, const Endmembers& e, size_t skip_records = 0, double accept_ratio = 0.0) {
		if ((!logAcceptanceRatio) || (accept_ratio > 0)) {
			EarthState es;
			es.time = t;
			es.mcmc_acceptance = accept_ratio;
			std::vector<double> tempVec;
			tempVec.reserve(states.size());

			//Given a mixing state and a member pointer, computes the concentration of that element in the final mixture
			auto ELEMENT_CONCENTRATION = [&](const MixState<N>& MIXSTATE, const DataOffset<RockSample, double> DAT) {
				double conc = 0.0;
				for (size_t idx = 0; idx < N; ++idx) {
					conc += MIXSTATE[idx] * DAT(e.E[idx]);
				};
				return conc;
			};

			auto RATIO_VALUE = [&](const MixState<N>& MIXSTATE,
								   const DataOffset<RockSample, double> A,
								   const DataOffset<RockSample, double> B) {
				return ELEMENT_CONCENTRATION(MIXSTATE, A) / ELEMENT_CONCENTRATION(MIXSTATE, B); };

			//Record percentiles & best fit of every ratio
			for (size_t i = 0; i < logRatioNames.size(); ++i) {
				auto A = logRatioA[i];
				auto B = logRatioB[i];
				tempVec.clear();
				for (size_t mc = skip_records; mc < states.size(); ++mc) {
					tempVec.push_back(RATIO_VALUE(states[mc], A, B));
				};
				std::sort(tempVec.begin(), tempVec.end());
				es.p025.push_back(SortedVectorPercentile(tempVec, 2.5));
				es.p975.push_back(SortedVectorPercentile(tempVec, 97.5));
				es.bestFit.push_back(RATIO_VALUE(bestFit, A, B));
			};
			V.push_back(es);
		};
	};

	std::string Results2CSV() {
		std::stringstream ss;
		ss << "TIME(/MYR),";
		if (logAcceptanceRatio) {
			ss << "MCMC_ACCEPT%,";
		};
		for (const auto& E : logRatioNames) {
			ss << E + "_025," << E << "," << E + "_975,";
		};
		ss << std::endl;
		for (const auto& es : V) {
			ss << es.time << ",";
			if (logAcceptanceRatio) {
				ss << 100 * es.mcmc_acceptance << ",";
			};
			for (size_t i = 0; i < logRatioNames.size(); ++i) {
				ss << std::to_string(es.p025[i]) << ",";
				ss << std::to_string(es.bestFit[i]) << ",";
				ss << std::to_string(es.p975[i]) << ",";
			};
			ss << std::endl;
		};
		return ss.str();
	};

	ResultsProcessor_Ratios(const ReconManager& RM) : ResultsProcessor_Generic(RM.GetInitConfig()) {
		for (const auto& Rstr : RM.GetInitConfig()["detailedRatioPrinter"]) {
			size_t sep = Rstr.find_first_of('/');
			std::string Astr = Rstr.substr(0, sep);
			std::string Bstr = Rstr.substr(sep + 1);
			auto A = (MemberOffset<RockSample, double>)RockSample::allElements[Astr];
			auto B = (MemberOffset<RockSample, double>)RockSample::allElements[Bstr];

			logRatioNames.push_back(Rstr);
			logRatioA.push_back(A);
			logRatioB.push_back(B);
		};
	};
};
