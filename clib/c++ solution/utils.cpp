#include "stdafx.h"
#include "utils.h"
#include <sstream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/normal_distribution.hpp>

boost::random::mt19937 gen;
boost::normal_distribution<> stdDev(0.0, 1.0);

const std::string DefaultPath() {
#if defined(_WIN64)
	return "";
#elif defined(_UNIXLIKE)
	return "./";
#else
	static_assert(false, "UNRECOGNIZED ARCHITECTURE!");
	return "<ERROR>";
#endif
};

const std::string DefaultWritePath() {
#ifdef CORE_APP
	return DefaultPath()+"../out/";
#elif PYTHON_LIB
	return DefaultPath() + "cout/";
#endif
};

int Random::Int32(int lowerBound, int upperBound) {
	boost::random::uniform_int_distribution<> dist(lowerBound, upperBound);
    return dist(gen);
};

int64 Random::Int64(int64 lowerBound, int64 upperBound) {
	boost::random::uniform_int_distribution<int64> dist(lowerBound, upperBound);
	return dist(gen);
};

double Random::Double() {
	boost::random::uniform_01<> dist;
    return dist(gen);
};

double Random::StdDstr() {
	return stdDev(gen);
}
double Random::NormDstr(double mean, double sigma) {
	boost::normal_distribution<> distr(mean,sigma);
	return distr(gen);
};

std::string ToFilenameString(const std::string & S) {
	std::string s;
	s.reserve(S.size());
	for (auto C : S) {
		if (isalnum(C)) {
			s.push_back(C);
		} else {
			s.push_back('-');
		};
	};
	return s;
};

void RemoveSubstring(std::string& S, const std::string& BADSTR) {
	if (S.find(BADSTR)!=std::string::npos) {
		std::string tempA, tempB;
		tempA= S.substr(0, S.find(BADSTR));
		tempB= S.substr(S.rfind(BADSTR)+BADSTR.size(), S.size());
		S= tempA+tempB;
	};
};

std::vector<std::string> listfiles(std::string path,std::string REQ_EXT) {
	std::vector<std::string> retval;
	struct dirent *entry;
	DIR *dp;
	
	dp = opendir(path.c_str());
	if (dp == NULL) {
		perror("opendir");
		return retval;
	}
	
	while((entry = readdir(dp))) {
		std::string S(entry->d_name);
		
		if (S[0]=='.')
			continue;
		
		if (S.find_first_of('.')==std::string::npos)
			continue;
		
		if ((REQ_EXT!="") && (S.find("."+REQ_EXT)==std::string::npos))
			continue;
		
		retval.push_back(path+S);
	};
	
	closedir(dp);
	return retval;
};

