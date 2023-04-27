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

#pragma once

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/lazy-class.h"
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
struct StaticCoeffects;
struct StringData;
struct MemoCacheBase;
struct Func;
struct RFuncData;
struct Class;
struct RClsMethData;

struct BlobEncoder;
struct BlobDecoder;

///////////////////////////////////////////////////////////////////////////////

/*
 * The payload of a PHP value.
 *
 * This union may only be used in contexts that have a discriminator, e.g. in
 * TypedValue (below), or when the type is known beforehand.
 */
#if defined(__x86_64__) || defined(__aarch64__)
#pragma pack(push, 1)
#endif
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
  RClsMethData* prclsmeth; // KindOfRClsMeth
  LazyClassData plazyclass;   // KindOfLazyClass
};
#if defined(__x86_64__) || defined(__aarch64__)
#pragma pack(pop)
#endif

#if defined(__x86_64__) || defined(__aarch64__)
static_assert(alignof(Value) == 1);
#else
static_assert(alignof(Value) == 8);
#endif
static_assert(sizeof(Value) == 8);

struct ConstModifiers {
  // Note that cgCheckSubClsCns relies on Value being 0.
  enum class Kind : uint8_t { Value = 0, Type = 1, Context = 2 };

  uint32_t rawData;

  static uint32_t constexpr kDataShift = 3;
  static uint32_t constexpr kMask = (uint32_t)-1UL << kDataShift;
  static uint32_t constexpr kKindMask = (1 << (kDataShift - 1)) - 1;
  static uint32_t constexpr kAbstractMask = 1 << (kDataShift - 1);

  StringData* getPointedClsName() const {
    assertx(use_lowptr);
    return (StringData*)(uintptr_t)(rawData & kMask);
  }
  StaticCoeffects getCoeffects() const;
  bool isAbstract() const { return rawData & kAbstractMask; }
  Kind kind() const { return static_cast<Kind>(rawData & kKindMask); }

  void setPointedClsName(StringData* clsName) {
    assertx(use_lowptr);
    rawData = (uintptr_t)clsName | (rawData & ~kMask);
  }
  void setCoeffects(StaticCoeffects coeffects);
  void setIsAbstract(bool isAbstract) {
    if (isAbstract) {
      rawData |= kAbstractMask;
    } else {
      rawData &= ~kAbstractMask;
    }
  }
  void setKind(Kind kind) {
    rawData |= (uint32_t(kind) & kKindMask);
  }

  static const char* show(Kind t) {
    switch (t) {
      case Kind::Value:   return "constant";
      case Kind::Type:    return "type constant";
      case Kind::Context: return "context constant";
    }
    not_reached();
  }
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
  // Key type and hash for VanillaDict.
  int32_t u_hash;
  // Used by Class::Const.
  ConstModifiers u_constModifiers;
  // Used by system constants
  bool u_dynamic;
};

/*
 * A TypedValue is a type-discriminated PHP Value.
 */
struct alignas(8) TypedValue {
  Value m_data;
  DataType m_type;
  AuxUnion m_aux;

  std::string pretty() const; // debug formatting. see trace.h

  // Accessors to simplify the migration from tv_rval to TypedValue.
  bool is_init() const { return m_type != KindOfUninit; }
  DataType type() const { return m_type; }
  Value val() const { return m_data; }

  void serde(BlobEncoder&) const;
  void serde(BlobDecoder&, bool makeStatic = true);

  TYPE_SCAN_CUSTOM() {
    if (isRefcountedType(m_type)) scanner.scan(m_data.pcnt);
  }
};

static_assert(alignof(TypedValue) == 8);
static_assert(sizeof(TypedValue) == 16);

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
 * Used to set m_data when a PreClassEmitter::Const's valOption() is
 * none to distinguish from constants that have an 86cinit. See
 * tvWriteConstValMissing.
 *
 * NB: We want the least significant bit to be unset to avoid
 * confusion with a resolved type-structure (see Class::clsCnsGet).
 */
constexpr int64_t kConstValMissing = -2;
static_assert(!(kConstValMissing & 0x1));

/*
 * Subclass of TypedValue which exposes m_aux accessors.
 */
struct TypedValueAux : TypedValue {
  static constexpr size_t auxOffset = offsetof(TypedValue, m_aux);
  static constexpr size_t auxSize = sizeof(decltype(m_aux));

  const int32_t& hash() const { return m_aux.u_hash; }
        int32_t& hash()       { return m_aux.u_hash; }

  bool is_const_val_missing() const { return m_data.num == kConstValMissing; }

  const ConstModifiers& constModifiers() const {
    return m_aux.u_constModifiers;
  }
  ConstModifiers& constModifiers() {
    return m_aux.u_constModifiers;
  }

  const bool& dynamic() const { return m_aux.u_dynamic; }
        bool& dynamic()       { return m_aux.u_dynamic; }

  void serde(BlobEncoder&) const = delete;
  void serde(BlobDecoder&) = delete;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Sometimes TypedValues need to be allocated with alignment that allows use of
 * 128-bit SIMD stores/loads.  This constant just helps self-document that
 * case.
 */
constexpr size_t kTVSimdAlign = 0x10;

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
X(KindOfRFunc,        RFuncData*);
X(KindOfClass,        Class*);
X(KindOfClsMeth,      ClsMethDataRef);
X(KindOfRClsMeth,     RClsMethData*);
X(KindOfLazyClass,    LazyClassData);

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

inline Value make_value(LazyClassData lclass) {
  Value v;
  v.plazyclass = lclass;
  return v;
}

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

ALWAYS_INLINE TypedValue make_tv_of_type(Value value, DataType dt) {
  // GCC does all sorts of unnecessary spills if we attempt to construct a
  // TypedValue here with an unknown DataType. Explicitly initializing the
  // AuxUnion doesn't help, either: https://godbolt.org/z/MbG48c
  //
  // This C++ code is the only code we've found that results in reasonable
  // codegen on GCC and that avoids undefined behavior. It simply moves the
  // value and movzbl's the type.
  //
  // GCC issue tracker: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98335
  static_assert(sizeof(TypedValue) == 16);
  static_assert(offsetof(TypedValue, m_data) == 0);
  static_assert(offsetof(TypedValue, m_type) == 8);
  __attribute__((__may_alias__)) int64_t raw[2];
  raw[0] = value.num;
  raw[1] = int64_t(uint8_t(dt));
  return *reinterpret_cast<const TypedValue*>(raw);
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
