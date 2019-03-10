#pragma once
#include "utils.h"
#include <fstream>
#include <sstream>

namespace csvWriterImpl {
	//If GCC wasn't dumb, this would be a member method of csvWriter
	template<typename MBR, typename INTYPE>
	struct Functor {
		static std::string ToString(MemberOffsetBase mOff, void* oPtr) {
			MemberOffset<MBR, INTYPE> mo(mOff);
			MBR* obj = (MBR*)oPtr;
			std::stringstream ss;
			ss << mo.Data(obj);
			return ss.str();
		};
	};
	template <typename MBR>
	struct Functor<MBR, double> {
		static std::string ToString(MemberOffsetBase mOff, void* oPtr) {
			MemberOffset<MBR, double> mo(mOff);
			MBR* obj = (MBR*)oPtr;
			double val = mo.Data(obj);
			if (std::isfinite(val)) {
				return std::to_string(val);
			} else {
				return "x";
			};
		};
	};
};

template <class MBR>
class csvWriter {
	typedef std::string(*TCONVRT)(MemberOffsetBase, void*);

	std::vector<std::string>		col_name;
	std::vector<MemberOffsetBase>	col_dat;
	std::vector<TCONVRT>			col_datConvertor;

	std::string						sep;
	std::string						nLn;

	template<typename INTYPE>
	static std::string ToString(MemberOffsetBase mOff, void* oPtr) {
		return csvWriterImpl::Functor<MBR, INTYPE>::ToString(mOff, oPtr);
	};

public:
	template<typename T>
	csvWriter<MBR>& RegisterEntry(std::string COL_TITLE, MemberOffset<MBR, T> memOff) {
		col_name.push_back(COL_TITLE);
		col_dat.push_back(memOff);
		col_datConvertor.push_back(&ToString<T>);
		return *this;
	};

	template<class ITERABLE, class FILTER>
	std::string Write(const ITERABLE& data, const FILTER& filter) {
		std::string out;

		//Write column titles
		for (size_t c = 0; c<col_name.size(); ++c) {
			out += col_name[c];
			if (c + 1 != col_name.size()) {
				out += sep;
			};
		};
		out += nLn;

		//Write all subsequent lines
		for (const auto& S : data) {
			if (filter.Test(S)) {
				for (unsigned int c = 0; c < col_name.size(); ++c) {
					TCONVRT convertor = col_datConvertor[c];
					out += (*convertor)(col_dat[c], (void*)(&S));

					if (c + 1 != col_name.size()) {
						out += sep;
					};
				};
			};
			out += nLn;
		};

		//Final newline
		out += nLn;

		return out;
	};
	
	template<class ITERABLE, class FILTER>
	void Write(const std::string& FNAME, const ITERABLE& data, const FILTER& flt) {
		//Write the actual file
		std::ofstream oFile((DefaultWritePath() + FNAME + ".csv").c_str());
		oFile << Write(data, flt);
		oFile.close();
	};
	
	csvWriter(const std::string& SEP= ",",
			  const std::string& NEWLINE= "\n") : sep(SEP), nLn(NEWLINE) {
	};
};
