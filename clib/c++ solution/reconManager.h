#pragma once
#include "reconCommon.h"
#include "reconEndmembers.h"

// Configuration structure to initialise reconstructions
// 
struct ReconManager {
	RockDatabase									shales;
	Endmembers*										E;
	std::vector < MemberOffset<RockSample, double>> Nmntr;
	std::vector < MemberOffset<RockSample, double>> Dmntr;
	std::vector < DiscreteFunction >				bestF;
	std::vector < DiscreteFunction >				errMF;
	std::vector < std::string >						nameR;
	double											kernelWidth;
	~ReconManager() { delete E; };

private:
	DenseStringMap initConfig;
	typedef std::string(*reconFptr)(const ReconManager&);
	reconFptr execRecon;
	RockDatabase* parsedDB_Keller;
	RockDatabase* parsedDB_nomorb;

	MemberOffset<RockSample, double> TranslateOffset(const std::string& sysName);

	WRB_Result GenerateBootstrapIMPL(const MemberOffset<RockSample, double>& A, const MemberOffset<RockSample, double>& B);
public:
	void AddRatio(const std::string& RNAME, MemberOffset<RockSample, double> NOM, MemberOffset<RockSample, double> DNM);
	size_t CountRatios() const;
	void AddBootstrap(const WRB_Result& r);
	size_t DataCountForBootstrap(const std::string& A, const std::string& B);

	WRB_Result GenerateBootstrap(const std::string& A, const std::string& B);

	void GenerateAllBootstraps();
	void ResetAllBootstraps();

	ReconManager(DenseStringMap conf, const std::string& DB = "");

	// Returns the time nearest to start_time for which all ratio data are available
	// (scanning either forward or backwards from start_time).
	double GetNearestValidTime(double start_time, bool scan_forward) const;

	//Returns the configuration object that was used to initialise this ReconManager
	const DenseStringMap& GetInitConfig() const { return initConfig; };

	//Executes the reconstruction, returns output as a CSV string
	std::string RunReconstruction() const;

	//Run the forward mixing calculation, given a time and a proportion of endmembers
	std::vector<double> ForwardModelCalc(double t, const std::vector<double>& p) const;
	double ForwardModelCalc(double t, const std::vector<double>& p, MemberOffset<RockSample, double> el) const;

	std::string GetEndmemberName(size_t idx) const;
	size_t GetEndmemberCount() const;

#ifdef PYTHON_LIB
	ReconManager(boost::python::dict D,
				 const std::string& DB = "") : ReconManager(DenseStringMap(D), DB) {};

	std::vector<double> ForwardModelCalc(double t, boost::python::list p) const;
	double ForwardModelCalc(double t, boost::python::list p, const std::string& EL) const;

#endif
};

//Forward definitions for reconstruction functions
namespace MCMCRecon {
	std::string RunMarkovModel_2M(const ReconManager& RM);
	std::string RunMarkovModel_3M(const ReconManager& RM);
	std::string RunMarkovModel_4M(const ReconManager& RM);
	std::string RunMarkovModel_5M(const ReconManager& RM);
	std::string RunMarkovModel_2M_Ratios(const ReconManager& RM);
	std::string RunMarkovModel_3M_Ratios(const ReconManager& RM);
	std::string RunMarkovModel_4M_Ratios(const ReconManager& RM);
	std::string RunMarkovModel_5M_Ratios(const ReconManager& RM);
};
namespace MatrixRecon {
	std::string RunModel_2M(const ReconManager& RM);
	std::string RunModel_3M(const ReconManager& RM);
	std::string RunModel_4M(const ReconManager& RM);
	std::string RunModel_5M(const ReconManager& RM);
	std::string RunModel_2M_Ratios(const ReconManager& RM);
	std::string RunModel_3M_Ratios(const ReconManager& RM);
	std::string RunModel_4M_Ratios(const ReconManager& RM);
	std::string RunModel_5M_Ratios(const ReconManager& RM);
};
