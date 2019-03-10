#include "stdafx.h"
#include "csvParser.h"

void ProcessString(std::string& s) {
	// Remove all double-quote characters
	s.erase(
			remove( s.begin(), s.end(), '\"' ),
			s.end()
			);
};

csvParser::csvParser(char separator) : sep(separator), totalLns(0) {};

csvParser::csvParser(std::string filename, char separator) : sep(separator), totalLns(0)
{
    csv_file.open(filename.c_str());
    if (!csv_file.is_open())
	{
		perror(("Error while opening file " + filename).c_str());
		exit(1);
	}
	
	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.
	std::istream::sentry se(csv_file, true);
	LoadBuffer(csv_file.rdbuf());
	csv_file.close();
};

csvParser::~csvParser() {};

const std::string& csvParser::GetLn(size_t line_number) {
	return lines[line_number];
};

void csvParser::LoadBuffer(std::streambuf* sb) {
	bool eof = false;
	bool inQuoteMarks = false;
	while (!eof) {
		std::string cLine;
		bool read = true;
		while (read) {
			int c = sb->sbumpc();
			switch (c) {
			case '\"':
				inQuoteMarks = !inQuoteMarks;
				cLine += (char)c;
				break;
			case '\n':
				if (!inQuoteMarks) {
					lines.push_back(cLine);
					read = false;
				};
				break;
			case '\r':
				if (!inQuoteMarks) {
					if (sb->sgetc() == '\n')
						sb->sbumpc();
					lines.push_back(cLine);
					read = false;
				};
				break;
			case EOF:
				eof = true;
				read = false;
				break;
			default:
				cLine += (char)c;
			};
		};
	};
	totalLns = lines.size();
	totalCols = NumberFields(GetLn(0));
};

size_t csvParser::TotalLns() {
	return totalLns;
};

size_t csvParser::NumberFields(std::string line) {
	std::string::iterator it;
	size_t num_of_comma = 0;
	bool inQuoteMarks= false;
	for (it=line.begin();it<line.end();it++) {
		if (*it=='"')
			inQuoteMarks= !inQuoteMarks;
		
		if ((!inQuoteMarks) && (*it==sep)) {
				++num_of_comma;
		};
    };
	return num_of_comma+1;
};

std::string csvParser::InnerScan(const char* p) const {
	bool inQuoteMarks= false;
	const char* first = p;
	while (*p!=NULL) {
		if (*p=='"') {
			inQuoteMarks= !inQuoteMarks;
		} else {
			if ((!inQuoteMarks) && (*p==sep))
				break;
		};
		++p;
	};
	return std::string(first, p - first);
};

std::string csvParser::Get(size_t row, size_t column) {
	const std::string& line = GetLn(row);
	const char* p = line.c_str();
	bool inQuoteMarks= false;
	
	//Case: For first column
	if (column==0) {
		return InnerScan(p);
	};
	//For all other cases
	int total_fields = 0;
	while (*p!=NULL) {
		if (*p=='"')
			inQuoteMarks= !inQuoteMarks;
		
		if ((!inQuoteMarks) && (*p==sep)) {
			++total_fields;
			if (total_fields == column) {
				//Next field will be the one we want!
				++p; //Don't include separator char in final result
				return InnerScan(p);
			};
		};
		++p;
    };
	return "";
};

size_t csvParser::TotalCols() {
	return totalCols;
}

int64 csvParser::GetColumnByTitle(const std::string& TTL) {
	std::map<std::string, size_t>::iterator it;
	it= titledCols.find(TTL);
	if (it!=titledCols.end()) {
		return (*it).second;
	} else {
		for (int i=0; i< TotalCols(); ++i) {
			std::string COL_NAME= Get(0,i);
			titledCols[COL_NAME]= i;
			if (COL_NAME==TTL) {
				return i;
			};
		};
	};
	titledCols[TTL]= -1;
	return -1;
};

int64 csvParser::GetCachedColumnByTitle(const std::string& TITLE) const {
	auto it = titledCols.find(TITLE);
	if (it != titledCols.end()) {
		return (*it).second;
	} else {
		return -1;
	};
};

std::string csvParser::GetTitledValue(const std::string& TTL, size_t row) {
	return Get(row,GetColumnByTitle(TTL));
};

void csvParser::CacheAllColumnTitles() {
	//Because of how these functions are internally specified, calling the following line
	//will cause ALL column titles to become cached
	GetColumnByTitle(Get(0,TotalCols()-1));
};

csvParserSeq::csvParserSeq(std::string filename, char separator) : csvParser(separator), lastLn(-1), outOfSeqOperations(0), cLine(""), pos(CHUNK_SIZE) {
	CountLines(filename);
	csv_file.open(filename.c_str());
    if (!csv_file.is_open()) {
		perror(("Error while opening file " + filename).c_str());
		exit(1);
	};
	_se= new std::istream::sentry(csv_file, true);
	_sb= csv_file.rdbuf();
	
	totalCols= NumberFields(GetLn(0));
	CacheAllColumnTitles();
};

csvParserSeq::~csvParserSeq() {
	delete _se;
	csv_file.close();
};

void csvParserSeq::CountLines(std::string filename) {
	std::ifstream infile;
	infile.open(filename.c_str());
    if (!infile.is_open())
	{
		perror(("Error while opening file " + filename).c_str());
		exit(1);
	}
	totalLns= 0;
	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.
	std::istream::sentry se(infile, true);
	std::streambuf* sb = infile.rdbuf();
	bool eof = false;
	bool inQuoteMarks= false;
	while (!eof){
		std::string cLine;
		bool read = true;
		while (read) {
			int c = sb->sbumpc();
			switch (c) {
				case '\"':
					inQuoteMarks= !inQuoteMarks;
					break;
				case '\n':
					if (!inQuoteMarks) {
						totalLns++;
						read= false;
					};
					break;
				case '\r':
					if (!inQuoteMarks) {
						if(sb->sgetc() == '\n')
							sb->sbumpc();
						totalLns++;
						read= false;
					};
					break;
				case EOF:
					eof= true;
					read= false;
					break;
				default:
					break;
			}
		}
	}
	infile.close();
};

void csvParserSeq::ReadNextLine() {
	cLine.clear();
	bool inQuoteMarks= false;
	bool read = true;
	size_t alloc = CHUNK_SIZE;
	cLine.reserve(alloc);
	size_t startpos = pos;
	while (read) {
		if (pos == CHUNK_SIZE) {
			if (startpos != CHUNK_SIZE) {
				alloc += CHUNK_SIZE;
				cLine.reserve(alloc);
				cLine.append(buf + startpos, CHUNK_SIZE - startpos);
			};
			_sb->sgetn(buf, CHUNK_SIZE);
			pos = 0;
			startpos = 0;
		} else {
			switch (buf[pos]) {
			case '\"':
				inQuoteMarks = !inQuoteMarks;
				//cLine += buf[pos];
				break;
			case '\n':
				if (!inQuoteMarks) {
					read = false;
				};
				break;
			case '\r':
				if (!inQuoteMarks) {
					if (buf[pos + 1] == '\n') {
						++pos;
					};
					read = false;
				};
				break;
			case EOF:
				read = false;
				break;
			default:
				break;
			};
			++pos;
		};
		/*int c = _sb->sbumpc();
		switch (c) {
			case '\"':
				inQuoteMarks= !inQuoteMarks;
				cLine += (char)c;
				break;
			case '\n':
				if (!inQuoteMarks) {
					read= false;
				};
				break;
			case '\r':
				if (!inQuoteMarks) {
					if(_sb->sgetc() == '\n')
						_sb->sbumpc();
					read= false;
				};
				break;
			case EOF:
				read= false;
				break;
			default:
				cLine += (char)c;
		};*/
	};
	cLine.append(buf + startpos, pos - startpos);
	++lastLn;
};

const std::string& csvParserSeq::GetLn(size_t line_number) {
	long lnum = (long)line_number; //Else end up with unsigned/signed problems!
	if (lnum ==lastLn) {
		return cLine;
	} else if (lnum>lastLn) {
		ReadNextLine();
		return cLine;
	} else {
		_sb->pubseekpos(0);//Re-set our state back to character zero!
		lastLn= -1; //Clear internal line counter
		while (lastLn<lnum) {
			ReadNextLine(); //Scan forward from line zero again
		};
		//Mark that we've commited an inefficient operation (unless we've reset back to the early lines, which is A-OK!)
		if (lnum>1) {
			std::cout << "<CVSPOSEQ" << line_number << "> ";
			outOfSeqOperations++;
		};
		//Return value as usual
		return cLine;
	};
};

csvParserString::csvParserString(const std::string & inString, char separator) : csvParser(separator) {
	//Reinterpret input string as a buffer, and load it using csvParser's standard code
	std::stringbuf buf(inString);
	LoadBuffer(&buf);
};
