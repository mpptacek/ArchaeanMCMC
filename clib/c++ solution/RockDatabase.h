#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <cmath>

#include "utils.h"
#include "csvParser.h"
#include "csvWriter.h"
#include "RockSample.h"

struct RockDatabase {
	typedef RockSample ETYPE;
	typedef std::vector<ETYPE> STYPE;
	struct Filter {
		virtual bool Test(const ETYPE& R) const = 0;
		virtual ~Filter() {};
	};
	struct WholeDBFilter {
		virtual void Init(const STYPE&) = 0;
		virtual bool Test(const ETYPE& R) = 0;
		virtual ~WholeDBFilter() {};
	};

protected:
	std::string dbName;
	STYPE samples;
	char separator;
	std::string fileExt;

public:

	typedef STYPE::iterator IT;
	typedef STYPE::const_iterator ITconst;

	ITconst begin() const { return Data().cbegin(); };
	ITconst end() const { return Data().cend(); };
	IT begin() { return Data().begin(); };
	IT end() { return Data().end(); };
	size_t size() const { return Data().size(); };
	void Reserve(size_t N) { samples.reserve(N); };

	struct Query {
		template<typename T>
		struct QIterator {
			const Filter* predicate;
			IT dbIT;
			IT dbITend;
			T& operator*()const {
				return dbIT.operator*();
			};
			T* operator->()const {
				return dbIT.operator->();
			};
			QIterator& operator++() {
				++dbIT;
				while ((predicate->Test(this->operator*()) == false)
					   &&(dbIT != dbITend)) {
					++dbIT;
				};
				return *this;
			};
			bool operator==(const QIterator& rhs) const {
				return (rhs.dbIT == this->dbIT);
			};
			bool operator!=(const QIterator& rhs) const {
				return !operator==(rhs);
			};
			QIterator(IT iter, IT iter_end, const Filter* pred) : dbIT(iter), dbITend(iter_end), predicate(pred) {};
		};
	private:
		RockDatabase& parent;
		const Filter* cond;
		Query(RockDatabase& db, const Filter* condition) : parent(db), cond(condition) {};
	public:
		~Query() { delete cond; };
		QIterator<ETYPE> begin() const {
			if (parent.size() == 0) {
				return QIterator<ETYPE>(parent.end(), parent.end(), cond);
			};
			auto startIT = parent.begin();
			while ((startIT != parent.end() && (cond->Test(*startIT) == false))) {
				++startIT;
			};
			return QIterator<ETYPE>(startIT, parent.end(), cond);
		};
		QIterator<ETYPE> end() const {
			return QIterator<ETYPE>(parent.end(), parent.end(), cond);
		};

		template<class F_TYPE>
		static Query Construct(RockDatabase& db, const F_TYPE& fobj) {
			F_TYPE* filter_copy = new F_TYPE(fobj);
			return Query(db, filter_copy);
		};
	};

	struct ConstQuery {
		template<typename T>
		struct QIterator {
			const Filter* predicate;
			ITconst dbIT;
			ITconst dbITend;
			const T& operator*()const {
				return dbIT.operator*();
			};
			const T* operator->()const {
				return dbIT.operator->();
			};
			QIterator& operator++() {
				++dbIT;
				while ((predicate->Test(this->operator*()) == false)
					   && (dbIT != dbITend)) {
					++dbIT;
				};
				return *this;
			};
			bool operator==(const QIterator& rhs) const {
				return (rhs.dbIT == this->dbIT);
			};
			bool operator!=(const QIterator& rhs) const {
				return !operator==(rhs);
			};
			QIterator(ITconst iter, ITconst iter_end, const Filter* pred) : dbIT(iter), dbITend(iter_end), predicate(pred) {};
		};
	private:
		const RockDatabase & parent;
		const Filter* cond;
		ConstQuery(const RockDatabase& db, const Filter* condition) : parent(db), cond(condition) {};
	public:
		~ConstQuery() { delete cond; };
		QIterator<ETYPE> begin() const {
			if (parent.size() == 0) {
				return QIterator<ETYPE>(parent.end(), parent.end(), cond);
			};
			auto startIT = parent.begin();
			while ((startIT != parent.end() && (cond->Test(*startIT) == false))) {
				++startIT;
			};
			return QIterator<ETYPE>(startIT, parent.end(), cond);
		};
		QIterator<ETYPE> end() const {
			return QIterator<ETYPE>(parent.end(), parent.end(), cond);
		};

		template<class F_TYPE>
		static ConstQuery Construct(const RockDatabase& db, const F_TYPE& fobj) {
			F_TYPE* filter_copy = new F_TYPE(fobj);
			return ConstQuery(db, filter_copy);
		};
	};

	template<class F_TYPE>
	Query Select(const F_TYPE& predicate) { return Query::Construct(*this, predicate); };
	template<class F_TYPE>
	ConstQuery Select(const F_TYPE& predicate) const { return ConstQuery::Construct(*this, predicate); };

	STYPE& Data() { return samples; };
	const STYPE& Data() const { return samples; };

	const std::string& Name() const { return dbName; };
	ETYPE& RandomElement();

	template<typename RET_TYPE>
	std::vector<RET_TYPE> ExtractList(MemberOffset<ETYPE, RET_TYPE> ofs) {
		std::vector<RET_TYPE> retVal;
		for (IT it = Data().begin(); it != Data().end(); ++it) {
			retVal.push_back(ofs.Data(&(*it)));
		};
		return retVal;
	};

	template<typename T>
	Vect<1>::List ExtractVectList(MemberOffset<ETYPE, T> ofs0) {
		Vect<1>::List retVal;
		for (IT it = Data().begin(); it != Data().end(); ++it) {
			Vect<1> V;
			V[0] = (double)ofs0.Data(&(*it));
			retVal.push_back(V);
		};
		return retVal;
	};
	template<typename T>
	Vect<2>::List ExtractVectList(MemberOffset<ETYPE, T> ofs0,
								  MemberOffset<ETYPE, T> ofs1) {
		Vect<2>::List retVal;
		for (IT it = Data().begin(); it != Data().end(); ++it) {
			Vect<2> V;
			V[0] = (double)ofs0.Data(&(*it));
			V[1] = (double)ofs1.Data(&(*it));
			retVal.push_back(V);
		};
		return retVal;
	};
	template<typename T>
	Vect<3>::List ExtractVectList(MemberOffset<ETYPE, T> ofs0,
								  MemberOffset<ETYPE, T> ofs1,
								  MemberOffset<ETYPE, T> ofs2) {
		Vect<3>::List retVal;
		for (IT it = Data().begin(); it != Data().end(); ++it) {
			Vect<3> V;
			V[0] = (double)ofs0.Data(&(*it));
			V[1] = (double)ofs1.Data(&(*it));
			V[2] = (double)ofs2.Data(&(*it));
			retVal.push_back(V);
		};
		return retVal;
	};

	RockDatabase& SetSeparator(char);
	RockDatabase& SetFileExtension(const std::string&);
	RockDatabase& SetName(const std::string&);

	virtual RockDatabase& ParseDirectory(const std::string& FOLDERNAME);
	virtual RockDatabase& ParseFile(STYPE& DAT, csvParser& FILE);
	RockDatabase& ParseString(const std::string& CSV);

	template<class FILTER>
	std::string DataAsCSV(const FILTER& f) const {
		csvWriter<ETYPE> r;
		for (auto a : ETYPE::allNumericValues) {
			r.RegisterEntry(a.first, (MemberOffset<ETYPE, double>)a.second);
		};
		return r.Write(Data(), f);
	};

	RockDatabase& ApplyWholeDBFilter(WholeDBFilter&);

	template<class I>
	RockDatabase& Merge(const I& ITERABLE) {
		for (const auto& S : ITERABLE) {
			samples.push_back(S);
		};
		return *this;
	};
	RockDatabase& Refilter(const Filter& F);

	RockDatabase& Add(const ETYPE&);
	RockDatabase& Clear();

	RockDatabase(const Query&);
	RockDatabase(const ConstQuery&);
	RockDatabase();
	virtual ~RockDatabase() {};
};

struct GenericRockDatabase : public RockDatabase {
public:
	//All additional parser functions will recieve the following arguments:
	//Reference to the current stored type
	//Reference to the source CSV reader
	//Integer corresponding to the current row
	//They must return true if the input contained anything worthwhile
	typedef bool(*CUSTOM_PARSER)(ETYPE&, csvParser&, int64);
private:
	typedef void(*TCONVRT)(MemberOffsetBase&, void*, const std::string&);
	std::vector<std::string>		col_name;
	std::vector<MemberOffsetBase>	col_dat;
	std::vector<TCONVRT>			col_converter;
	std::vector<CUSTOM_PARSER>		extra_parsers;

	std::set<std::string>		val_ignore;

protected:
	virtual RockDatabase& ParseFile(std::vector<ETYPE>& DAT, csvParser& FILE);
public:
	template<typename T>
	GenericRockDatabase& RegisterEntry(std::string COL_TITLE, MemberOffset<ETYPE, T> memOff) {
		col_name.push_back(COL_TITLE);
		col_dat.push_back(memOff);
		col_converter.push_back(WriteData<ETYPE, T>);

		return *this;
	};

	GenericRockDatabase& RegisterCustomParser(CUSTOM_PARSER cp);

	GenericRockDatabase& IgnoreValue(const std::string& VAL);

	GenericRockDatabase();
};
