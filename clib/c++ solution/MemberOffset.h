#pragma once

enum MemberAccessMode {
	STATIC_FUNCTION,
	MEMBER_FUNCTION,
	DATA_OFFSET,
	VIRTUAL_FUNCTOR
};
/// ...................................................................................................................
/// Generalised untyped base class, containing all necessary information for specialised casting
/// 
/// ...................................................................................................................
struct MemberOffsetBase {
	void* MO; //This is usually a typecast to something that allows access to the underlying type (e.g. a function pointer)
	MemberAccessMode MODE; //Determines the mode by which this MemberOffset accesses the underlying data
	MemberOffsetBase(void* V, MemberAccessMode M) : MO(V), MODE(M) {};
};

/// ...................................................................................................................
/// Storage for implementation data of interface-only members.
/// 
/// ...................................................................................................................
template<typename T>
void* AllocStorageForClass() {
	return malloc(sizeof(T));
};

/// ...................................................................................................................
/// Interface to a type-safe functor
/// 
/// ...................................................................................................................
template<class OBJ_TYPE, typename MEMBER_TYPE>
struct FunctorOffset {
	static MEMBER_TYPE DelegatedAccess(void* _c, OBJ_TYPE* P) {
		return static_cast<FunctorOffset*>(_c)->Access(P);
	};
	static MEMBER_TYPE DelegatedAccess(void* _c, const OBJ_TYPE* P) {
		return static_cast<const FunctorOffset*>(_c)->Access(P);
	};

	virtual MEMBER_TYPE Access(OBJ_TYPE* P) { return MEMBER_TYPE(); };
	virtual MEMBER_TYPE Access(const OBJ_TYPE* P) const { return MEMBER_TYPE(); };
	virtual ~FunctorOffset() {};
};

/// ...................................................................................................................
/// Interface to a type-safe data member
/// 
/// ...................................................................................................................
template<class OBJ_TYPE, typename MEMBER_TYPE>
struct DataOffset {
private:
	typedef MEMBER_TYPE OBJ_TYPE::*MPTR;
	union { void* DAT; MPTR PTR; } c; //Need a union here because the compiler refuses to reinterpret_cast this abomination
public:
	DataOffset() { c.DAT = nullptr; };
	DataOffset(void* P) { c.DAT = P; };

	static MemberOffsetBase Make(MPTR PTR) {
		static_assert(sizeof(decltype(c.DAT)) >= sizeof(decltype(c.PTR)), "SIZE MISMATCH");
		static_assert(alignof(decltype(c.DAT)) >= alignof(decltype(c.PTR)), "ALIGN MISMATCH");
		decltype(c) U; U.PTR = PTR;
		return MemberOffsetBase(U.DAT, MemberAccessMode::DATA_OFFSET);
	};

	static MEMBER_TYPE Data(void* _c, OBJ_TYPE* OBJ) {
		decltype(c) U; U.DAT = _c;
		return OBJ->*(U.PTR);
	};
	static const MEMBER_TYPE Data(void* _c, const OBJ_TYPE* OBJ) {
		decltype(c) U; U.DAT = _c;
		return OBJ->*(U.PTR);
	};

	static MEMBER_TYPE& DataR(void* _c, OBJ_TYPE* OBJ) {
		decltype(c) U; U.DAT = _c;
		return OBJ->*(U.PTR);
	};
	static const MEMBER_TYPE& DataR(void* _c, const OBJ_TYPE* OBJ) {
		decltype(c) U; U.DAT = _c;
		return OBJ->*(U.PTR);
	};

	MEMBER_TYPE Data(OBJ_TYPE* OBJ) const {
		return OBJ->*(reinterpret_cast<MPTR>(c.PTR));
	};
	const MEMBER_TYPE Data(const OBJ_TYPE* OBJ) const {
		return OBJ->*(reinterpret_cast<MPTR>(c.PTR));
	};
	MEMBER_TYPE& DataR(OBJ_TYPE* OBJ) const {
		return OBJ->*(reinterpret_cast<MPTR>(c.PTR));
	};
	const MEMBER_TYPE& DataR(const OBJ_TYPE* OBJ) const {
		return OBJ->*(reinterpret_cast<MPTR>(c.PTR));
	};

	MEMBER_TYPE operator()(OBJ_TYPE& OBJ) const { return Data(OBJ); };
	const MEMBER_TYPE operator()(const OBJ_TYPE& OBJ) const { return Data(OBJ); };
	MEMBER_TYPE operator()(OBJ_TYPE* OBJ) const { return Data(OBJ); };
	const MEMBER_TYPE operator()(const OBJ_TYPE* OBJ) const { return Data(OBJ); };
	MEMBER_TYPE Data(OBJ_TYPE& OBJ) const { return Data(&OBJ); };
	const MEMBER_TYPE Data(const OBJ_TYPE& OBJ) const { return Data(&OBJ); };
	MEMBER_TYPE& DataR(OBJ_TYPE& OBJ) const { return DataR(&OBJ); };
	const MEMBER_TYPE& DataR(const OBJ_TYPE& OBJ) const { return DataR(&OBJ); };

	operator MemberOffsetBase() const {
		return MemberOffsetBase(c.DAT, MemberAccessMode::DATA_OFFSET);
	};
};

/// ...................................................................................................................
/// Interface to a type-safe member function
/// 
/// ...................................................................................................................
template<class OBJ_TYPE, typename MEMBER_TYPE>
class FunctionOffsetM {
public:
	typedef MEMBER_TYPE(OBJ_TYPE::*FPTR) ();
	typedef MEMBER_TYPE(OBJ_TYPE::*FPTRC)() const;
	union {
		FPTR	p;
		FPTRC	cp;
	} f;

	FunctionOffsetM() {};
	FunctionOffsetM(FPTR   fp) { this->f.p = fp; };
	FunctionOffsetM(FPTRC cfp) { this->f.cp = cfp; };
	FunctionOffsetM(FPTR*  fp) { this->f.p = *fp; };
	FunctionOffsetM(FPTRC*cfp) { this->f.cp = *cfp; };
	FunctionOffsetM(const FunctionOffsetM& gM) : f(gM.f) {};

	MEMBER_TYPE Data(OBJ_TYPE* PTR) const {
		return ((*PTR).*(this->f.p))();
	};
	MEMBER_TYPE Data(const OBJ_TYPE* PTR) const {
		return ((*PTR).*(this->f.cp))();
	};
};

/// ...................................................................................................................
/// Generalised type-safe function interface
/// 
/// ...................................................................................................................
template<class OBJ_TYPE, typename MEMBER_TYPE>
struct FunctionOffset {
private:
	void* c; //pointer to the function pointer (if member function), or the function pointer itself (if static function)
	bool IS_FOM;
	typedef MEMBER_TYPE(*FPTR)(OBJ_TYPE&);
	typedef MEMBER_TYPE(*FPTRC)(const OBJ_TYPE&);
public:
	static FunctionOffsetM<OBJ_TYPE, MEMBER_TYPE>* c2fom(void* _c) {
		return (FunctionOffsetM<OBJ_TYPE, MEMBER_TYPE>*)_c;
	};

	MEMBER_TYPE Data(OBJ_TYPE* PTR) const {
		if (IS_FOM) {
			return c2fom(c)->Data(PTR);
		} else {
			return (*(FPTR)c)(*PTR);
		};
	};
	const MEMBER_TYPE Data(const OBJ_TYPE* PTR) const {
		if (IS_FOM) {
			return c2fom(c)->Data(PTR);
		} else {
			return (*(FPTRC)c)(*PTR);
		};
	};

	static MEMBER_TYPE Data(void* _c, bool IS_F, OBJ_TYPE* PTR) {
		if (IS_F) {
			return c2fom(_c)->Data(PTR);
		} else {
			return (*(FPTR)_c)(*PTR);
		};
	};
	static const MEMBER_TYPE Data(void* _c, bool IS_F, const OBJ_TYPE* PTR) {
		if (IS_F) {
			return c2fom(_c)->Data(PTR);
		} else {
			return (*(FPTRC)_c)(*PTR);
		};
	};

	MEMBER_TYPE operator()(OBJ_TYPE& REF) const { return Data(REF); };
	const MEMBER_TYPE operator()(const OBJ_TYPE& REF) const { return Data(REF); };
	MEMBER_TYPE operator()(OBJ_TYPE* PTR) const { return Data(PTR); };
	const MEMBER_TYPE operator()(const OBJ_TYPE* PTR) const { return Data(PTR); };

	operator MemberOffsetBase() const {
		return MemberOffsetBase(c, IS_FOM ? MemberAccessMode::MEMBER_FUNCTION : MemberAccessMode::STATIC_FUNCTION);
	};

	FunctionOffset() : c(nullptr) {};
	FunctionOffset(void* P, bool member_func) : c(P), IS_FOM(member_func) {};

	static MemberOffsetBase Make(MEMBER_TYPE(*FuncPtr)(OBJ_TYPE &)) {
		void* P = (void*)FuncPtr;
		return MemberOffsetBase(P, MemberAccessMode::STATIC_FUNCTION);
	};
	static MemberOffsetBase Make(MEMBER_TYPE(*FuncPtr)(const OBJ_TYPE &)) {
		void* P = (void*)FuncPtr;
		return MemberOffsetBase(P, MemberAccessMode::STATIC_FUNCTION);
	};
	static MemberOffsetBase Make(MEMBER_TYPE(OBJ_TYPE::*FPTR)()) {
		void* P = (void*)AllocStorageForClass< FunctionOffsetM<OBJ_TYPE, MEMBER_TYPE> >();
		return MemberOffsetBase(P, MemberAccessMode::MEMBER_FUNCTION);
	};
	static MemberOffsetBase Make(MEMBER_TYPE(OBJ_TYPE::*FPTR)() const) {
		void* P = (void*)AllocStorageForClass< FunctionOffsetM<OBJ_TYPE, MEMBER_TYPE> >();
		return MemberOffsetBase(P, MemberAccessMode::MEMBER_FUNCTION);
	};
};

/// ...................................................................................................................
/// Temporary storage, which is accessed when the user attempts to access a function offset by reference.
/// Instead of throwing an exception or returning an invalid pointer, we construct a copy of the function's
/// return in this memory 'safe zone,' then pass a reference to here.
/// Since this safe zone is in static memory, furthermore, we do not have to worry about memory leaks.
/// Due to thread-local storage and the sanitised interface, this even manages to be thread-safe!
/// 
/// ...................................................................................................................
struct TempStrLoc {
private:
	static void* Ptr();
public:
	static const size_t BUFFER_SZ = 1024;
	typedef size_t BUFFER_TYPE;
	template<typename T>
	static T& Access() {
		static_assert((sizeof(T) <= BUFFER_SZ), "Attempting to allocate too large a type into temporary storage; increase BUFFER_SZ constant!");
		static_assert(alignof(T) <= alignof(BUFFER_TYPE), "Buffer has insufficient alignment");
		return *(T*)(Ptr());
	};
};

/// ...................................................................................................................
/// Generalised type-safe interface to member
/// 
/// ...................................................................................................................
template<class OBJ_TYPE, typename MEMBER_TYPE>
class MemberOffset : public MemberOffsetBase {
public:
	MEMBER_TYPE Data(OBJ_TYPE* PTR) const {
		switch (MODE) {
		case MemberAccessMode::STATIC_FUNCTION:
			return FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, false, PTR);
		case MemberAccessMode::MEMBER_FUNCTION:
			return FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, true, PTR);
		case MemberAccessMode::DATA_OFFSET:
			return DataOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, PTR);
		case MemberAccessMode::VIRTUAL_FUNCTOR:
			return FunctorOffset<OBJ_TYPE, MEMBER_TYPE>::DelegatedAccess(MO, PTR);
		default:
			return MEMBER_TYPE();
		};
	};

	MEMBER_TYPE Data(OBJ_TYPE& REF) const { return Data(&REF); };

	const MEMBER_TYPE Data(const OBJ_TYPE* PTR) const {
		switch (MODE) {
		case MemberAccessMode::STATIC_FUNCTION:
			return FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, false, PTR);
		case MemberAccessMode::MEMBER_FUNCTION:
			return FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, true, PTR);
		case MemberAccessMode::DATA_OFFSET:
			return DataOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, PTR);
		case MemberAccessMode::VIRTUAL_FUNCTOR:
			return FunctorOffset<OBJ_TYPE, MEMBER_TYPE>::DelegatedAccess(MO, PTR);
		default:
			return MEMBER_TYPE();
		};
	};

	const MEMBER_TYPE Data(const OBJ_TYPE& REF) const { return Data(&REF); };

	MEMBER_TYPE& DataR(OBJ_TYPE* PTR) const {
		switch (MODE) {
		case MemberAccessMode::STATIC_FUNCTION:
			TempStrLoc::Access<MEMBER_TYPE>() = FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, false, PTR);
			return TempStrLoc::Access<MEMBER_TYPE>();
		case MemberAccessMode::MEMBER_FUNCTION:
			TempStrLoc::Access<MEMBER_TYPE>() = FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, true, PTR);
			return TempStrLoc::Access<MEMBER_TYPE>();
		case MemberAccessMode::DATA_OFFSET:
			return DataOffset<OBJ_TYPE, MEMBER_TYPE>::DataR(MO, PTR);
		case MemberAccessMode::VIRTUAL_FUNCTOR:
			TempStrLoc::Access<MEMBER_TYPE>() = FunctorOffset<OBJ_TYPE, MEMBER_TYPE>::DelegatedAccess(MO, PTR);
			return TempStrLoc::Access<MEMBER_TYPE>();
		default:
			return TempStrLoc::Access<MEMBER_TYPE>();
		};
	};
	const MEMBER_TYPE& DataR(const OBJ_TYPE* PTR) const {
		switch (MODE) {
		case MemberAccessMode::STATIC_FUNCTION:
			TempStrLoc::Access<MEMBER_TYPE>() = FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, false, PTR);
			return TempStrLoc::Access<MEMBER_TYPE>();
		case MemberAccessMode::MEMBER_FUNCTION:
			TempStrLoc::Access<MEMBER_TYPE>() = FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Data(MO, true, PTR);
			return TempStrLoc::Access<MEMBER_TYPE>();
		case MemberAccessMode::DATA_OFFSET:
			return DataOffset<OBJ_TYPE, MEMBER_TYPE>::DataR(MO, PTR);
		case MemberAccessMode::VIRTUAL_FUNCTOR:
			TempStrLoc::Access<MEMBER_TYPE>() = FunctorOffset<OBJ_TYPE, MEMBER_TYPE>::DelegatedAccess(MO, PTR);
			return TempStrLoc::Access<MEMBER_TYPE>();
		default:
			return TempStrLoc::Access<MEMBER_TYPE>();
		};
	};
	MEMBER_TYPE& DataR(OBJ_TYPE& REF) const { return DataR(&REF); };
	const MEMBER_TYPE& DataR(const OBJ_TYPE& REF) const { return DataR(&REF); };

	MemberOffset() : MemberOffsetBase(nullptr, MemberAccessMode::VIRTUAL_FUNCTOR) {}; //Will segfault (nullptr dereference) if not further adjusted
	MemberOffset(MemberOffsetBase MOB) : MemberOffsetBase(MOB.MO, MOB.MODE) {};
	MemberOffset(void* MO, MemberAccessMode MODE) : MemberOffsetBase(MO, MODE) {};

	MEMBER_TYPE operator()(OBJ_TYPE& O) const {
		return Data(O);
	};

	const MEMBER_TYPE operator()(const OBJ_TYPE& O) const {
		return Data(O);
	};

	operator DataOffset<OBJ_TYPE, MEMBER_TYPE>() const {
		switch (MODE) {
		case MemberAccessMode::DATA_OFFSET:
			return DataOffset<OBJ_TYPE, MEMBER_TYPE>(MO);
		default:
			return *(DataOffset<OBJ_TYPE, MEMBER_TYPE>*)(nullptr);
		};
	};

	operator FunctionOffset<OBJ_TYPE, MEMBER_TYPE>() const {
		switch (MODE) {
		case MemberAccessMode::STATIC_FUNCTION:
			return  FunctionOffset<OBJ_TYPE, MEMBER_TYPE>(MO, false);
		case MemberAccessMode::MEMBER_FUNCTION:
			return  FunctionOffset<OBJ_TYPE, MEMBER_TYPE>(MO, true);
		default:
			return *(FunctionOffset<OBJ_TYPE, MEMBER_TYPE>*)(nullptr);
		};
	};

	operator FunctorOffset<OBJ_TYPE, MEMBER_TYPE>() const {
		switch (MODE) {
		case MemberAccessMode::VIRTUAL_FUNCTOR:
			return *(FunctorOffset<OBJ_TYPE, MEMBER_TYPE>*)((void*)MO);
		default:
			return *(FunctorOffset<OBJ_TYPE, MEMBER_TYPE>*)(nullptr);
		};
	};
};


/// ...................................................................................................................
/// Functor conversion operator helpers
/// 
/// ...................................................................................................................
#include <cstring>

template<class DERIVED_TYPE>
static MemberOffsetBase ConvertFunctor2Base(const DERIVED_TYPE*const derived_this) {
	//Create a copy of the derived object in a safe location that won't get deleted until the program terminates
	//Store pointer to said copy
	void* p = AllocStorageForClass<DERIVED_TYPE>();
	std::memcpy(p, derived_this, sizeof(DERIVED_TYPE));
	return MemberOffsetBase(p, MemberAccessMode::VIRTUAL_FUNCTOR);
};

template<class OBJ_TYPE, typename MEMBER_TYPE, class DERIVED_TYPE>
MemberOffset<OBJ_TYPE, MEMBER_TYPE> ConvertFunctor2Off(const DERIVED_TYPE*const derived_this) {
	//Create a copy of the derived object in a safe location that won't get deleted until the program terminates
	//Store pointer to said copy
	void* p = AllocStorageForClass<DERIVED_TYPE>();
	std::memcpy(p, derived_this, sizeof(DERIVED_TYPE));
	return MemberOffset<OBJ_TYPE, MEMBER_TYPE>(p, MemberAccessMode::VIRTUAL_FUNCTOR);
};

/// ...................................................................................................................
/// Factory classes
/// 
/// ...................................................................................................................
template<class OBJ_TYPE, typename MEMBER_TYPE>
MemberOffset<OBJ_TYPE, MEMBER_TYPE> gmoff(MEMBER_TYPE OBJ_TYPE::*MPTR) {
	return static_cast<MemberOffset<OBJ_TYPE, MEMBER_TYPE>>(DataOffset<OBJ_TYPE, MEMBER_TYPE>::Make(MPTR));
};
template<class OBJ_TYPE, typename MEMBER_TYPE>
MemberOffset<OBJ_TYPE, MEMBER_TYPE> gmoff(MEMBER_TYPE(OBJ_TYPE::*FPTR)()) {
	return static_cast<MemberOffset<OBJ_TYPE, MEMBER_TYPE>>(FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Make(FPTR));
};
template<class OBJ_TYPE, typename MEMBER_TYPE>
MemberOffset<OBJ_TYPE, MEMBER_TYPE> gmoff(MEMBER_TYPE(OBJ_TYPE::*FPTR)() const) {
	return static_cast<MemberOffset<OBJ_TYPE, MEMBER_TYPE>>(FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Make(FPTR));
};
template<class OBJ_TYPE, typename MEMBER_TYPE>
MemberOffset<OBJ_TYPE, MEMBER_TYPE> gmoff(MEMBER_TYPE(*FPTR)(OBJ_TYPE &)) {
	return static_cast<MemberOffset<OBJ_TYPE, MEMBER_TYPE>>(FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Make(FPTR));
};
template<class OBJ_TYPE, typename MEMBER_TYPE>
MemberOffset<OBJ_TYPE, MEMBER_TYPE> gmoff(MEMBER_TYPE(*FPTR)(const OBJ_TYPE &)) {
	return static_cast<MemberOffset<OBJ_TYPE, MEMBER_TYPE>>(FunctionOffset<OBJ_TYPE, MEMBER_TYPE>::Make(FPTR));
};

/// ...................................................................................................................
/// Ease of use macro
/// 
/// ...................................................................................................................
#define OFF(MBR_NAME) gmoff(&MBR_NAME)

