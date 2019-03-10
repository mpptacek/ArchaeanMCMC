#pragma once

#ifdef PYTHON_LIB
#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include "boost/python.hpp"
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"

//Translate vector to pylist
template<typename T>
boost::python::list Vect2PyList(const std::vector<T>& v) {
	boost::python::list R;
	for (const T& e : v) {
		R.append(e);
	};
	return R;
};

//Translate pylist to vector
template<typename T>
std::vector<T> PyList2Vect(const boost::python::list& l) {
	size_t N = boost::python::len(l);
	std::vector<T> V(N);
	for (size_t i = 0; i < N; ++i) {
		V[i] = boost::python::extract<T>(l[i]);
	};
	return V;
};

struct PythonRegistry {
public:
	typedef void(*PY_INIT_FPTR)(void);
	static PythonRegistry& Get();
	void Register(const std::string&, PY_INIT_FPTR);
	void ExecAll();
private:
	std::vector<PY_INIT_FPTR> init;
	std::vector<std::string> sname;
	PythonRegistry() {};
};

template<void(*T)(void)>
struct PyRegistrator {
	PyRegistrator(const std::string& S) {
		PythonRegistry::Get().Register(S, T);
	};
};
#else
template<void(*T)(void)>
struct PyRegistrator {
	PyRegistrator(const std::string& S) {
		//Python bindings are disabled!
	};
};
#endif

#define PYTHON_LINK(f) namespace { static PyRegistrator<&f> zpmi_##f(#f); };

#define PYTHON_LINK_FUNCTION(fname) void PyFuncLink##fname() { boost::python::def(#fname, &fname); }; PYTHON_LINK(PyFuncLink##fname);

#define PYTHON_LINK_EXEC(fname) void PyExecLink##fname(); PYTHON_LINK(PyExecLink##fname); void PyExecLink##fname()
