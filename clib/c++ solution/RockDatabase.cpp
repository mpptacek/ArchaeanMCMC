#include "stdafx.h"
#include "RockDatabase.h"

//#define FILTER_REPORT

RockDatabase::RockDatabase() : separator(','), fileExt("csv") {};

RockDatabase::RockDatabase(const Query & Q) {
	this->Merge(Q);
};

RockDatabase::RockDatabase(const ConstQuery & Q) {
	this->Merge(Q);
};

RockDatabase& RockDatabase::SetSeparator(char SEP) {
	separator = SEP;
	return *this;
};
RockDatabase& RockDatabase::SetFileExtension(const std::string& EXT) {
	fileExt = EXT;
	return *this;
};
RockDatabase& RockDatabase::SetName(const std::string& NM) {
	dbName = NM;
	return *this;
};

RockDatabase& RockDatabase::ParseDirectory(const std::string& FOLDERNOM) {
	std::vector<std::string> datafiles = listfiles(DefaultPath() + FOLDERNOM + "/", fileExt);
	for (int i = 0; i<datafiles.size(); ++i) {
		std::cout << "PARSING " << datafiles[i] << std::endl;
		//csvParserSeq is big, so let's not put it on the stack!
		csvParserSeq* inFile = new csvParserSeq(datafiles[i], separator);
		ParseFile(samples, *inFile);
		delete inFile;
	};
	return *this;
};

RockDatabase & RockDatabase::ParseFile(STYPE & DAT, csvParser & FILE) {
	throw std::runtime_error("RockDatabase::ParseFile() - COULD NOT FIND SPECIALISATION.");
	return *this;
};

RockDatabase & RockDatabase::ApplyWholeDBFilter(WholeDBFilter & F) {
	size_t samples_start0 = samples.size();
	F.Init(samples);
	STYPE tempList;
	std::swap(tempList, samples);
	samples.reserve(tempList.size());
	for (const auto& S : tempList) {
		if (F.Test(S)) {
			samples.push_back(S);
		};
	};
#ifdef FILTER_REPORT
	std::cout << "Whole database filter rejected " << samples_start0 - samples.size() << " samples." << std::endl;
#endif
	return *this;
};

RockSample& RockDatabase::RandomElement() {
	return Data()[Random::Int64(0, Data().size() - 1)];
};

RockDatabase& RockDatabase::Refilter(const Filter& F) {
	STYPE tempList;
	std::swap(samples, tempList);
	samples.reserve(tempList.size());
	for (const auto& S : tempList) {
		if (F.Test(S)) {
			samples.push_back(S);
		};
	};
	return *this;
};

RockDatabase& RockDatabase::Add(const ETYPE& R) {
	samples.push_back(R);
	return *this;
};

RockDatabase& RockDatabase::Clear() {
	samples.clear();
	return *this;
};

RockDatabase& GenericRockDatabase::ParseFile(std::vector<ETYPE>& DAT, csvParser& FILE) {
	std::vector<size_t> col_number;
	for (size_t i = 0; i< col_name.size(); ++i) {
		col_number.push_back(FILE.GetColumnByTitle(col_name[i]));
	};

	for (size_t i = 1; i<FILE.TotalLns(); ++i) {
		ETYPE rok;
		bool allEntriesIgnored = true;
		//Run main parser
		for (unsigned int j = 0; j<col_number.size(); ++j) {
			if (col_number[j] >= 0) {
				std::string str_input = FILE.Get(i, col_number[j]);
				//Only parse this cell if it's not on the ignore list!
				if (val_ignore.count(str_input) == 0) {
					allEntriesIgnored = false;
					(*col_converter[j])(col_dat[j], (void*)(&rok), str_input);
				};
			};
		};
		//Run all additional custom parsers
		for (size_t k = 0; k<extra_parsers.size(); ++k) {
			bool SUCCESS = (*extra_parsers[k])(rok, FILE, i);
			if (SUCCESS) {
				allEntriesIgnored = false;
			};
		};
		if (!allEntriesIgnored) {
			DAT.push_back(rok);
		};
	};
	return *this;
};

RockDatabase & RockDatabase::ParseString(const std::string & CSV_string) {
	csvParserString csv(CSV_string);
	ParseFile(samples, csv);
	return *this;
};

GenericRockDatabase& GenericRockDatabase::RegisterCustomParser(CUSTOM_PARSER cp) {
	extra_parsers.push_back(cp);
	return *this;
};

GenericRockDatabase& GenericRockDatabase::IgnoreValue(const std::string& VAL) {
	val_ignore.insert(VAL);
	return *this;
};

GenericRockDatabase::GenericRockDatabase() {
	IgnoreValue("");
};

