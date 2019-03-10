#include "stdafx.h"
#include "reconResultsProcessors.h"

ResultsProcessor_Generic::ResultsProcessor_Generic(const DenseStringMap & conf)
	: logAcceptanceRatio((conf.Get("reconMode") == "MCMC")) {};
