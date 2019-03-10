#pragma once
#include "Model.h"

//The Exponential-kernel weighed moving average ratio bootstrap
struct WRB_Result {
	DiscreteFunction bestFit;
	DiscreteFunction stdError;

	double Percentile975(double x) const;
	double Percentile025(double x) const;
};

//Generate a ratio bootstrap
WRB_Result WRB_Bootstrap(const std::vector<double>& age, const std::vector<double>& A, const std::vector<double>& B, double kernelWidth, size_t ITER = 10000);

//Generate an elemental bootstrap
WRB_Result WEB_Bootstrap(const std::vector<double>& age, const std::vector<double>& A, double kernelWidth, size_t ITER = 10000);
