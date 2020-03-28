/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_TYPED_VALUE_H_
#define incl_HPHP_TYPED_VALUE_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/util/type-scan.h"
#include "hphp/util/type-traits.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <type_traits>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct MaybeCountable;
struct ObjectData;
struct ResourceHdr;
struct StringData;
struct MemoCacheBase;
struct Func;
struct RFuncData;
struct Class;
struct RecordData;

///////////////////////////////////////////////////////////////////////////////

/*
 * The payload of a PHP value.
 *
 * This union may only be used in contexts that have a discriminator, e.g. in
 * TypedValue (below), or when the type is known beforehand.
 */
union Value {
  int64_t       num;    // KindOfInt64, KindOfBool (must be zero-extended)
  double        dbl;    // KindOfDouble
  StringData*   pstr;   // KindOfString, KindOfPersistentString
  ArrayData*    parr;   // KindOf{Persistent,}{Vec,Dict,Keyset,{,D,V}Array}
  ObjectData*   pobj;   // KindOfObject
  ResourceHdr*  pres;   // KindOfResource
  MaybeCountable* pcnt; // for alias-safe generic refcounting operations
  MemoCacheBase* pcache; // Not valid except when in a MemoSlot
  const Func*   pfunc;  // KindOfFunc
  RFuncData*    prfunc; // KindOfRFunc
  Class*        pclass; // KindOfClass
  ClsMethDataRef pclsmeth; // KindOfClsMeth
  RecordData*   prec;   // KindOfRecord
};

enum VarNrFlag { NR_FLAG = 1 << 29 };

struct ConstModifiers {
  uint32_t rawData;

  static uint32_t constexpr kMask = (uint32_t)-1UL << 2;

  StringData* getPointedClsName() const {
    assertx(use_lowptr);
    return (StringData*)(uintptr_t)(rawData & kMask);
  }
  bool isAbstract()      const { return rawData & 2; }
  bool isType()          const { return rawData & 1; }

  void setPointedClsName (StringData* clsName) {
    assertx(use_lowptr);
    rawData = (uintptr_t)clsName | (rawData & ~kMask);
  }
  void setIsAbstract(bool isAbstract) { rawData |= (isAbstract ? 2 : 0); }
  void setIsType    (bool isType)     { rawData |= (isType ? 1 : 0); }
};

/*
 * Auxiliary data in a TypedValue.
 *
 * Must only be read or written to in specialized contexts.
 */
union AuxUnion {
  // Undiscriminated raw value.
  uint32_t u_raw;
  // True if the function was supposed to return an Awaitable, but instead
  // returned the value that would be packed inside that Awaitable. If this
  // flag is false, it doesn't mean that the TV is a non-finished Awaitable,
  // or an Awaitable at all.
  uint32_t u_asyncEagerReturnFlag;
  // Key type and hash for MixedArray.
  int32_t u_hash;
  // Magic number for asserts in VarNR.
  VarNrFlag u_varNrFlag;
  // Used by Class::initPropsImpl() for deep init.
  bool u_deepInit;
  // Used by unit.cpp to squirrel away RDS handles.
  int32_t u_rdsHandle;
  // Used by Class::Const.
  ConstModifiers u_constModifiers;
  // Used by InvokeResult.
  bool u_ok;
  // Used by system constants
  bool u_dynamic;
};

/*
 * A TypedValue is a type-discriminated PHP Value.
 */
struct TypedValue {
  Value m_data;
  DataType m_type;
  AuxUnion m_aux;

  std::string pretty() const; // debug formatting. see trace.h

  TYPE_SCAN_CUSTOM() {
    if (isRefcountedType(m_type)) scanner.scan(m_data.pcnt);
  }
};

constexpr size_t kTypedValueAlignMask = sizeof(TypedValue) - 1;

constexpr size_t alignTypedValue(size_t sz) {
  return (sz + kTypedValueAlignMask) & ~kTypedValueAlignMask;
}

/*
 * sizeof(TypedValue) should be a power of 2 no greater than 16 bytes.
 */
static_assert((sizeof(TypedValue) & (kTypedValueAlignMask)) == 0,
              "TypedValue's size is expected to be a power of 2");
static_assert(sizeof(TypedValue) <= 16, "Don't add big things to AuxUnion");

/*
 * Subclass of TypedValue which exposes m_aux accessors.
 */
struct TypedValueAux : TypedValue {
  static constexpr size_t auxOffset = offsetof(TypedValue, m_aux);
  static constexpr size_t auxSize = sizeof(decltype(m_aux));

  const int32_t& hash() const { return m_aux.u_hash; }
        int32_t& hash()       { return m_aux.u_hash; }

  const VarNrFlag& varNrFlag() const { return m_aux.u_varNrFlag; }
        VarNrFlag& varNrFlag()       { return m_aux.u_varNrFlag; }

  const bool& deepInit() const { return m_aux.u_deepInit; }
        bool& deepInit()       { return m_aux.u_deepInit; }

  const int32_t& rdsHandle() const { return m_aux.u_rdsHandle; }
        int32_t& rdsHandle()       { return m_aux.u_rdsHandle; }

  const ConstModifiers& constModifiers() const {
    return m_aux.u_constModifiers;
  }
  ConstModifiers& constModifiers() {
    return m_aux.u_constModifiers;
  }

  const bool& dynamic() const { return m_aux.u_dynamic; }
        bool& dynamic()       { return m_aux.u_dynamic; }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Sometimes TypedValues need to be allocated with alignment that allows use of
 * 128-bit SIMD stores/loads.  This constant just helps self-document that
 * case.
 */
constexpr size_t kTVSimdAlign = 0x10;

/*
 * A TypedNum is a TypedValue that is either KindOfDouble or KindOfInt64.
 */
using TypedNum = TypedValue;

///////////////////////////////////////////////////////////////////////////////

/*
 * Assertions on Cells and TypedValues.  Should usually only happen inside an
 * assertx().
 */
bool tvIsPlausible(TypedValue);

///////////////////////////////////////////////////////////////////////////////

/*
 * TV-lval "concept"-like trait.
 *
 * This enables us to take functions that logically operate on a TypedValue&
 * and parametrize them over any opaque mutable reference to a DataType tag and
 * a Value data element.  This decouples the representation of a TypedValue
 * from its actual memory layout.
 *
 * See tv-mutate.h for usage examples.
 */
template<typename T, typename Ret = void>
using enable_if_lval_t = typename std::enable_if<
  conjunction<
    std::is_same<
      ident_t<decltype((type(std::declval<T>())))>,
      DataType&
    >,
    std::is_same<
      ident_t<decltype((val(std::declval<T>())))>,
      Value&
    >,
    std::is_same<
      ident_t<decltype((as_tv(std::declval<T>())))>,
      TypedValue
    >
  >::value,
  Ret
>::type;

template<typename T, typename Ret = void>
using enable_if_tv_val_t = typename std::enable_if<
  conjunction<
    std::is_convertible<
      ident_t<decltype((type(std::declval<T>())))>,
      DataType
    >,
    std::is_convertible<
      ident_t<decltype((val(std::declval<T>())))>,
      Value
    >,
    std::is_convertible<
      ident_t<decltype((as_tv(std::declval<T>())))>,
      TypedValue
    >
  >::value,
  Ret
>::type;

// TV-lval / -rval API for TypedValue& / const TypedValue&, respectively.
ALWAYS_INLINE DataType& type(TypedValue& tv) { return tv.m_type; }
ALWAYS_INLINE Value& val(TypedValue& tv) { return tv.m_data; }
ALWAYS_INLINE const DataType& type(const TypedValue& tv) { return tv.m_type; }
ALWAYS_INLINE const Value& val(const TypedValue& tv) { return tv.m_data; }
ALWAYS_INLINE TypedValue as_tv(const TypedValue& tv) { return tv; }

// TV-lval / -rval API for TypedValue* / const TypedValue*, respectively.
ALWAYS_INLINE DataType& type(TypedValue* tv) { return tv->m_type; }
ALWAYS_INLINE Value& val(TypedValue* tv) { return tv->m_data; }
ALWAYS_INLINE const DataType& type(const TypedValue* tv) { return tv->m_type; }
ALWAYS_INLINE const Value& val(const TypedValue* tv) { return tv->m_data; }
ALWAYS_INLINE TypedValue as_tv(const TypedValue* tv) { return *tv; }

///////////////////////////////////////////////////////////////////////////////

template<DataType> struct DataTypeCPPType;

#define X(dt, cpp) \
template<> struct DataTypeCPPType<dt> { using type = cpp; }

X(KindOfUninit,       void);
X(KindOfNull,         void);
X(KindOfBoolean,      bool);
X(KindOfInt64,        int64_t);
X(KindOfDouble,       double);
X(KindOfDArray,       ArrayData*);
X(KindOfPersistentDArray,  const ArrayData*);
X(KindOfVArray,       ArrayData*);
X(KindOfPersistentVArray,  const ArrayData*);
X(KindOfArray,        ArrayData*);
X(KindOfPersistentArray,  const ArrayData*);
X(KindOfVec,          ArrayData*);
X(KindOfPersistentVec, const ArrayData*);
X(KindOfDict,         ArrayData*);
X(KindOfPersistentDict, const ArrayData*);
X(KindOfKeyset,       ArrayData*);
X(KindOfPersistentKeyset, const ArrayData*);
X(KindOfObject,       ObjectData*);
X(KindOfResource,     ResourceHdr*);
X(KindOfString,       StringData*);
X(KindOfPersistentString, const StringData*);
X(KindOfFunc,         Func*);
X(KindOfClass,        Class*);
X(KindOfClsMeth,      ClsMethDataRef);
X(KindOfRecord,       RecordData*);

#undef X

/*
 * Pack a base data element into a Value.
 */
template<class T>
typename std::enable_if<
  std::is_integral<T>::value,
  Value
>::type make_value(T t) { Value v; v.num = t; return v; }

template<class T>
typename std::enable_if<
  std::is_pointer<T>::value,
  Value
>::type make_value(T t) {
  Value v;
  v.num = reinterpret_cast<int64_t>(t);
  return v;
}

inline Value make_value(double d) { Value v; v.dbl = d; return v; }

inline Value make_value(ClsMethDataRef clsMeth) {
  Value v;
  v.pclsmeth = clsMeth;
  return v;
}

/*
 * Pack a base data element into a TypedValue for use elsewhere in the runtime.
 *
 * TypedValue tv = make_tv<KindOfInt64>(123);
 */
template<DataType DType>
typename std::enable_if<
  !std::is_same<typename DataTypeCPPType<DType>::type,void>::value &&
  DType != KindOfDArray && DType != KindOfPersistentDArray &&
  DType != KindOfVArray && DType != KindOfPersistentVArray &&
  DType != KindOfArray && DType != KindOfPersistentArray,
  TypedValue
>::type make_tv(typename DataTypeCPPType<DType>::type val) {
  TypedValue ret;
  ret.m_data = make_value(val);
  ret.m_type = DType;
  assertx(tvIsPlausible(ret));
  return ret;
}

template<DataType DType>
typename std::enable_if<
  std::is_same<typename DataTypeCPPType<DType>::type,void>::value,
  TypedValue
>::type make_tv() {
  TypedValue ret;
  ret.m_type = DType;
  assertx(tvIsPlausible(ret));
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Unlike init_null_variant and uninit_variant, these should be placed in
 * .rodata and cause a segfault if written to.
 */
extern const TypedValue immutable_null_base;
extern const TypedValue immutable_uninit_base;

///////////////////////////////////////////////////////////////////////////////

}

#endif
