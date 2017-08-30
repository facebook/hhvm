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

#include "hphp/util/type-scan.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <type_traits>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct MaybeCountable;
struct ObjectData;
struct RefData;
struct ResourceHdr;
struct StringData;

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
  ArrayData*    parr;   // KindOfArray, KindOfVec, KindOfDict, KindOfKeyset
  ObjectData*   pobj;   // KindOfObject
  ResourceHdr*  pres;   // KindOfResource
  RefData*      pref;   // KindOfRef
  MaybeCountable* pcnt; // for alias-safe generic refcounting operations
};

enum VarNrFlag { NR_FLAG = 1 << 29 };

struct ConstModifiers {
  bool isAbstract;
  bool isType;
};

/*
 * Auxiliary data in a TypedValue.
 *
 * Must only be read or written to in specialized contexts.
 */
union AuxUnion {
  // Undiscriminated raw value.
  uint32_t u_raw;
  // True if we're suspending an FCallAwait.
  uint32_t u_fcallAwaitFlag;
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
 * These may be used to provide a little more self-documentation about whether
 * typed values must be cells (not KindOfRef) or ref (must be KindOfRef).
 *
 * See bytecode.specification for details.  Note that in
 * bytecode.specification, refs are abbreviated as "V".
 */
using Cell = TypedValue;
using Ref = TypedValue;

/*
 * A TypedNum is a TypedValue that is either KindOfDouble or KindOfInt64.
 */
using TypedNum = TypedValue;

///////////////////////////////////////////////////////////////////////////////

/*
 * Assertions on Cells and TypedValues.  Should usually only happen inside an
 * assert().
 */
bool tvIsPlausible(TypedValue);
bool cellIsPlausible(Cell);
bool refIsPlausible(Ref);

///////////////////////////////////////////////////////////////////////////////

template<DataType> struct DataTypeCPPType;

#define X(dt, cpp) \
template<> struct DataTypeCPPType<dt> { using type = cpp; }

X(KindOfUninit,       void);
X(KindOfNull,         void);
X(KindOfBoolean,      bool);
X(KindOfInt64,        int64_t);
X(KindOfDouble,       double);
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
X(KindOfRef,          RefData*);
X(KindOfString,       StringData*);
X(KindOfPersistentString, const StringData*);

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

/*
 * Pack a base data element into a TypedValue for use elsewhere in the runtime.
 *
 * TypedValue tv = make_tv<KindOfInt64>(123);
 */
template<DataType DType>
typename std::enable_if<
  !std::is_same<typename DataTypeCPPType<DType>::type,void>::value,
  TypedValue
>::type make_tv(typename DataTypeCPPType<DType>::type val) {
  TypedValue ret;
  ret.m_data = make_value(val);
  ret.m_type = DType;
  assert(tvIsPlausible(ret));
  return ret;
}

template<DataType DType>
typename std::enable_if<
  std::is_same<typename DataTypeCPPType<DType>::type,void>::value,
  TypedValue
>::type make_tv() {
  TypedValue ret;
  ret.m_type = DType;
  assert(tvIsPlausible(ret));
  return ret;
}

/*
 * Extract the underlying data element for the TypedValue
 *
 * int64_t val = unpack_tv<KindOfInt64>(tv);
 */
template <DataType DType>
typename std::enable_if<
  std::is_same<typename DataTypeCPPType<DType>::type,double>::value,
  double
>::type unpack_tv(TypedValue *tv) {
  assert(DType == tv->m_type);
  assert(tvIsPlausible(*tv));
  return tv->m_data.dbl;
}

template <DataType DType>
typename std::enable_if<
  std::is_integral<typename DataTypeCPPType<DType>::type>::value,
  typename DataTypeCPPType<DType>::type
>::type unpack_tv(TypedValue *tv) {
  assert(DType == tv->m_type);
  assert(tvIsPlausible(*tv));
  return tv->m_data.num;
}

template <DataType DType>
typename std::enable_if<
  std::is_pointer<typename DataTypeCPPType<DType>::type>::value,
  typename DataTypeCPPType<DType>::type
>::type unpack_tv(TypedValue *tv) {
  assert((DType == tv->m_type) ||
         (isStringType(DType) && isStringType(tv->m_type)) ||
         (isArrayType(DType) && isArrayType(tv->m_type)) ||
         (isVecType(DType) && isVecType(tv->m_type)) ||
         (isDictType(DType) && isDictType(tv->m_type)) ||
         (isKeysetType(DType) && isKeysetType(tv->m_type)));
  assert(tvIsPlausible(*tv));
  return reinterpret_cast<typename DataTypeCPPType<DType>::type>
           (tv->m_data.pstr);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Unlike init_null_variant and uninit_variant, these should be placed in
 * .rodata and cause a segfault if written to.
 */
extern const Cell immutable_null_base;
extern const Cell immutable_uninit_base;

///////////////////////////////////////////////////////////////////////////////

}

#endif
