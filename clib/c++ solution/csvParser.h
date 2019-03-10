#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>

class csvParser {
protected:
	std::ifstream csv_file;
	std::vector<std::string> lines;
	std::map<std::string, size_t> titledCols;
	char sep;
	size_t totalLns;
	size_t totalCols;
	
	std::string InnerScan(const char*) const;
	csvParser(char separator); //Protected constructor; initialises state, but does not load file
	size_t NumberFields(std::string line); //Returns Number of fields in the line
	virtual const std::string& GetLn(size_t line_number);//Returns entire line as a string based on line number.
	void LoadBuffer(std::streambuf*);
public:
	csvParser(std::string filename, char separator=','); //Public constructor; loads file into memory
	virtual ~csvParser();
	
	std::string Get(size_t row, size_t column); //Returns the field in the specified row and column.

	template<typename T>
	T Get(size_t row, size_t column) {
		std::stringstream ss;
		ss << Get(row, column);
		T retVal;
		ss >> retVal;
		return retVal;
	};
	
	size_t TotalCols();
	size_t TotalLns();
	
	int64 GetColumnByTitle(const std::string& TITLE);
	int64 GetCachedColumnByTitle(const std::string& TITLE) const;
	void CacheAllColumnTitles();
	std::string GetTitledValue(const std::string& TITLE, size_t row);
};

class csvParserString : public csvParser {
public:
	csvParserString(const std::string& inString, char separator = ',');
};

class csvParserSeq : public csvParser {
	std::istream::sentry* _se;
	std::streambuf* _sb;
	size_t pos;
	const static size_t CHUNK_SIZE = (2 << 18) - 1;
	char buf[CHUNK_SIZE + 1];
	unsigned int outOfSeqOperations;
	long lastLn; //Value of -1 if no line has been read yet
	std::string cLine;
	void CountLines(std::string);
	
	virtual const std::string& GetLn(size_t line_number);
	void ReadNextLine();
public:
	csvParserSeq(std::string filename, char separator=',');
	~csvParserSeq();
};

