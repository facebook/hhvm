/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_MEMBER_OPERATIONS_H_
#define incl_HPHP_VM_MEMBER_OPERATIONS_H_

#include <type_traits>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

const StaticString s_storage("storage");

struct InvalidSetMException : std::runtime_error {
  InvalidSetMException()
    : std::runtime_error("Empty InvalidSetMException")
    , m_tv(make_tv<KindOfNull>())
  {}

  explicit InvalidSetMException(const TypedValue& value)
    : std::runtime_error(folly::format("InvalidSetMException containing {}",
                                       value.pretty()).str())
    , m_tv(value)
  {}

  ~InvalidSetMException() noexcept {}

  const TypedValue& tv() const { return m_tv; };

 private:
  /* m_tv will contain a TypedValue with a reference destined for the
   * VM eval stack. */
  req::root<TypedValue> m_tv;
};

// When MoreWarnings is set to true, the VM will raise more warnings
// on SetOpM, IncDecM and CGetG, intended to match Zend.
const bool MoreWarnings =
#ifdef HHVM_MORE_WARNINGS
  true
#else
  false
#endif
  ;

/*
 * KeyType and the associated functions below are used to generate member
 * operation functions specialized for certain key types. Many functions take a
 * KeyType template parameter, then use key_type<keyType> as the type of their
 * key parameter. Depending on which KeyType is used, the parameter will be a
 * TypedValue, int64_t, or StringData*.
 */
enum class KeyType {
  Any, // Key is passed as a TypedValue and could be any type
  Int, // Key is passed as an int64_t
  Str, // Key is passed as a StringData*
};

/* KeyTypeTraits maps from KeyType to the C++ type holding they key. */
template<KeyType> struct KeyTypeTraits;
template<> struct KeyTypeTraits<KeyType::Any> { typedef TypedValue type; };
template<> struct KeyTypeTraits<KeyType::Int> { typedef int64_t type; };
template<> struct KeyTypeTraits<KeyType::Str> { typedef StringData* type; };

/* key_type is the type used in the signatures of functions taking a member
 * key. */
template<KeyType kt> using key_type = typename KeyTypeTraits<kt>::type;

/* initScratchKey is used in scenarios where we want a TypedValue key
 * regardless of what the current function was given. */
inline TypedValue initScratchKey(TypedValue tv) {
  assertx(tv.m_type != KindOfRef);
  return tv;
}

inline TypedValue initScratchKey(int64_t key) {
  return make_tv<KindOfInt64>(key);
}

inline TypedValue initScratchKey(StringData* key) {
  return make_tv<KindOfString>(key);
}

/* keyAsValue transforms a key into a value suitable for indexing into an
 * Array. */
inline const Variant& keyAsValue(TypedValue& key) {
  return tvAsCVarRef(&key);
}
inline int64_t keyAsValue(int64_t key)           { return key; }
inline StrNR keyAsValue(StringData* key)         { return StrNR(key); }

/* prepareKey is used by operations that need to cast their key to a
 * string. For generic keys, the returned value must be decreffed after use. */
StringData* prepareAnyKey(TypedValue* tv);
inline StringData* prepareKey(TypedValue tv)  { return prepareAnyKey(&tv); }
inline StringData* prepareKey(StringData* sd) { return sd; }

/* releaseKey helps with decreffing a StringData* returned from
 * prepareKey. When used with KeyType::Any, corresponding to
 * prepareKey(TypedValue), it will consume the reference produced by
 * prepareKey. When used with KeyType::Str, corresponding to
 * prepareKey(StringData*), it is a nop. */
template<KeyType keyType>
inline void releaseKey(StringData* sd) {
  static_assert(keyType == KeyType::Any, "bad KeyType");
  decRefStr(sd);
}

template<>
inline void releaseKey<KeyType::Str>(StringData*) {
  // do nothing. we don't own a reference to this string.
}

void objArrayAccess(ObjectData* base);

TypedValue objOffsetGet(
  ObjectData* base,
  TypedValue offset,
  bool validate = true
);

bool objOffsetIsset(ObjectData* base, TypedValue offset, bool validate = true);
bool objOffsetEmpty(ObjectData* base, TypedValue offset, bool validate = true);

void objOffsetSet(
  ObjectData* base,
  TypedValue offset,
  TypedValue* val,
  bool validate = true
);

void objOffsetAppend(ObjectData* base, TypedValue* val, bool validate = true);
void objOffsetUnset(ObjectData* base, TypedValue offset);

[[noreturn]] void throw_cannot_use_newelem_for_lval_read_col();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_vec();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_dict();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_keyset();

[[noreturn]] void unknownBaseType(const TypedValue*);

/**
 * Elem when base is Null
 */
inline const TypedValue* ElemEmptyish() {
  return init_null_variant.asTypedValue();
}

template<MOpMode mode>
inline const TypedValue* ElemArrayPre(ArrayData* base, int64_t key) {
  return mode == MOpMode::Warn ? base->nvTryGet(key) : base->nvGet(key);
}

template<MOpMode mode>
inline const TypedValue* ElemArrayPre(ArrayData* base, StringData* key) {
  auto constexpr warn = mode == MOpMode::Warn;
  int64_t n;
  return base->convertKey(key, n)
    ? (warn ? base->nvTryGet(n) : base->nvGet(n))
    : (warn ? base->nvTryGet(key) : base->nvGet(key));
}

template<MOpMode mode>
inline const TypedValue* ElemArrayPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemArrayPre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemArrayPre<mode>(base, key.m_data.pstr);

  // TODO(#3888164): Array elements can never be KindOfUninit.  This API should
  // be changed.
  auto tv = ArrNR(base).asArray().rvalAtRef(cellAsCVarRef(key)).asTypedValue();
  return tv->m_type != KindOfUninit ? tv : nullptr;
}

/**
 * Elem when base is an Array
 */
template<MOpMode mode, KeyType keyType>
inline const TypedValue* ElemArray(ArrayData* base, key_type<keyType> key) {
  assert(base->isPHPArray());

  auto result = ElemArrayPre<mode>(base, key);

  if (UNLIKELY(result == nullptr)) {
    if (mode == MOpMode::Warn) {
      auto const scratch = initScratchKey(key);
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(&scratch).toString().data());
    }
    return ElemEmptyish();
  }

  assertx(result->m_type != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Vec
 */
template<MOpMode mode>
inline const TypedValue* ElemVecPre(ArrayData* base, int64_t key) {
  return mode == MOpMode::Warn
    ? PackedArray::NvTryGetIntVec(base, key)
    : PackedArray::NvGetIntVec(base, key);
}

template<MOpMode mode>
inline const TypedValue* ElemVecPre(ArrayData* base, StringData* key) {
  if (mode == MOpMode::Warn) throwInvalidArrayKeyException(key, base);
  return nullptr;
}

template<MOpMode mode>
inline const TypedValue* ElemVecPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return ElemVecPre<mode>(base, key.m_data.num);
  if (isStringType(dt))      return ElemVecPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline const TypedValue* ElemVec(ArrayData* base, key_type<keyType> key) {
  assertx(base->isVecArray());
  auto result = ElemVecPre<mode>(base, key);
  if (mode != MOpMode::Warn) {
    if (UNLIKELY(!result)) return ElemEmptyish();
  }
  assertx(result->m_type != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Dict
 */
template<MOpMode mode>
inline const TypedValue* ElemDictPre(ArrayData* base, int64_t key) {
  return mode == MOpMode::Warn
    ? MixedArray::NvTryGetIntDict(base, key)
    : MixedArray::NvGetIntDict(base, key);
}

template<MOpMode mode>
inline const TypedValue* ElemDictPre(ArrayData* base, StringData* key) {
  return mode == MOpMode::Warn
    ? MixedArray::NvTryGetStrDict(base, key)
    : MixedArray::NvGetStrDict(base, key);
}

template<MOpMode mode>
inline const TypedValue* ElemDictPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemDictPre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemDictPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline const TypedValue* ElemDict(ArrayData* base, key_type<keyType> key) {
  assertx(base->isDict());
  auto result = ElemDictPre<mode>(base, key);
  if (mode != MOpMode::Warn) {
    if (UNLIKELY(!result)) return ElemEmptyish();
  }
  assertx(result->m_type != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Keyset
 */
template<MOpMode mode>
inline const TypedValue* ElemKeysetPre(ArrayData* base, int64_t key) {
  return mode == MOpMode::Warn
    ? SetArray::NvTryGetInt(base, key)
    : SetArray::NvGetInt(base, key);
}

template<MOpMode mode>
inline const TypedValue* ElemKeysetPre(ArrayData* base, StringData* key) {
  return mode == MOpMode::Warn
    ? SetArray::NvTryGetStr(base, key)
    : SetArray::NvGetStr(base, key);
}

template<MOpMode mode>
inline const TypedValue* ElemKeysetPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemKeysetPre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemKeysetPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline const TypedValue* ElemKeyset(ArrayData* base, key_type<keyType> key) {
  assertx(base->isKeyset());
  auto result = ElemKeysetPre<mode>(base, key);
  if (mode != MOpMode::Warn) {
    if (UNLIKELY(!result)) return ElemEmptyish();
  }
  assertx(isIntType(result->m_type) || isStringType(result->m_type));
  return result;
}

/**
 * Elem when base is an Int64, Double, or Resource.
 */
inline const TypedValue* ElemScalar() {
  if (RuntimeOption::EnableHipHopSyntax) {
    raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  }
  return ElemEmptyish();
}

/**
 * Elem when base is a Boolean
 */
inline const TypedValue* ElemBoolean(TypedValue* base) {
  if (base->m_data.num) {
    return ElemScalar();
  }
  return ElemEmptyish();
}

inline int64_t ElemStringPre(int64_t key) {
  return key;
}

inline int64_t ElemStringPre(StringData* key) {
  return key->toInt64(10);
}

inline int64_t ElemStringPre(TypedValue key) {
  if (LIKELY(isIntType(key.m_type))) {
    return key.m_data.num;
  } else if (LIKELY(isStringType(key.m_type))) {
    return key.m_data.pstr->toInt64(10);
  } else {
    raise_notice("String offset cast occurred");
    return cellAsCVarRef(key).toInt64();
  }
}

/**
 * Elem when base is a String
 */
template<MOpMode mode, KeyType keyType>
inline const TypedValue* ElemString(TypedValue& tvRef,
                                    TypedValue* base,
                                    key_type<keyType> key) {
  auto offset = ElemStringPre(key);

  if (offset < 0 || offset >= base->m_data.pstr->size()) {
    if (mode == MOpMode::Warn) {
      raise_notice("Uninitialized string offset: %" PRId64, offset);
    }
    tvRef = make_tv<KindOfPersistentString>(staticEmptyString());
  } else {
    tvRef = make_tv<KindOfPersistentString>(base->m_data.pstr->getChar(offset));
    assert(tvRef.m_data.pstr->isStatic());
  }
  return &tvRef;
}

/**
 * Elem when base is an Object
 */
template<MOpMode mode, KeyType keyType>
inline const TypedValue* ElemObject(TypedValue& tvRef,
                                    TypedValue* base,
                                    key_type<keyType> key) {
  auto scratch = initScratchKey(key);

  if (LIKELY(base->m_data.pobj->isCollection())) {
    if (mode == MOpMode::Warn) {
      return collections::at(base->m_data.pobj, &scratch);
    }
    auto res = collections::get(base->m_data.pobj, &scratch);
    if (!res) {
      res = &tvRef;
      tvWriteNull(res);
    }
    return res;
  }

  tvRef = objOffsetGet(instanceFromTv(base), scratch);
  return &tvRef;
}

/**
 * $result = $base[$key];
 */
template<MOpMode mode, KeyType keyType>
NEVER_INLINE const TypedValue* ElemSlow(TypedValue& tvRef,
                                        TypedValue* base,
                                        key_type<keyType> key) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return ElemEmptyish();
    case KindOfBoolean:
      return ElemBoolean(base);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return ElemScalar();
    case KindOfPersistentString:
    case KindOfString:
      return ElemString<mode, keyType>(tvRef, base, key);
    case KindOfPersistentVec:
    case KindOfVec:
      return ElemVec<mode, keyType>(base->m_data.parr, key);
    case KindOfPersistentDict:
    case KindOfDict:
      return ElemDict<mode, keyType>(base->m_data.parr, key);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemKeyset<mode, keyType>(base->m_data.parr, key);
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemArray<mode, keyType>(base->m_data.parr, key);
    case KindOfObject:
      return ElemObject<mode, keyType>(tvRef, base, key);
    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/*
 * Fast path for Elem assuming base is an Array.  Does not unbox the returned
 * pointer.
 */
template<MOpMode mode, KeyType keyType = KeyType::Any>
inline const TypedValue* Elem(TypedValue& tvRef,
                              TypedValue* base,
                              key_type<keyType> key) {
  assertx(mode != MOpMode::Define && mode != MOpMode::Unset);
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    return ElemArray<mode, keyType>(base->m_data.parr, key);
  }
  if (LIKELY(tvIsVecArray(base))) {
    return ElemVec<mode, keyType>(base->m_data.parr, key);
  }
  if (LIKELY(tvIsDict(base))) {
    return ElemDict<mode, keyType>(base->m_data.parr, key);
  }
  if (LIKELY(tvIsKeyset(base))) {
    return ElemKeyset<mode, keyType>(base->m_data.parr, key);
  }

  return ElemSlow<mode, keyType>(tvRef, base, key);
}

template<bool reffy, KeyType kt>
inline TypedValue* ElemDArrayPre(Array& base, key_type<kt> key) {
  return reffy ?
    const_cast<TypedValue*>(base.lvalAtRef(keyAsValue(key)).asTypedValue()) :
    const_cast<TypedValue*>(base.lvalAt(keyAsValue(key)).asTypedValue());
}

template<>
inline TypedValue*
ElemDArrayPre<true, KeyType::Any>(Array& base, TypedValue key) {
  if (key.m_type == KindOfInt64) {
    return ElemDArrayPre<true, KeyType::Int>(base, key.m_data.num);
  }
  return const_cast<TypedValue*>(
    base.lvalAtRef(tvAsCVarRef(&key)).asTypedValue()
  );
}

template<>
inline TypedValue*
ElemDArrayPre<false, KeyType::Any>(Array& base, TypedValue key) {
  if (key.m_type == KindOfInt64) {
    return ElemDArrayPre<false, KeyType::Int>(base, key.m_data.num);
  }
  return const_cast<TypedValue*>(
    base.lvalAt(tvAsCVarRef(&key)).asTypedValue()
  );
}

/**
 * ElemD when base is an Array
 */
template<MOpMode mode, bool reffy, KeyType keyType>
inline TypedValue* ElemDArray(TypedValue* base, key_type<keyType> key) {
  auto& baseArr = tvAsVariant(base).asArrRef();
  assertx(baseArr.isPHPArray());
  auto constexpr warn = mode == MOpMode::Warn;
  auto const defined = !warn || baseArr.exists(keyAsValue(key));

  auto* result = ElemDArrayPre<reffy, keyType>(baseArr, key);
  if (!defined) {
    auto scratchKey = initScratchKey(key);
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&scratchKey).toString().data());
  }

  return result;
}

/**
 * ElemD when base is a Vec
 */
template <bool reffy>
inline TypedValue* ElemDVecPre(TypedValue* base, int64_t key) {
  ArrayData* oldArr = base->m_data.parr;

  if (reffy) throwRefInvalidArrayValueException(oldArr);

  auto const r = PackedArray::LvalIntVec(oldArr, key, oldArr->cowCheck());
  if (r.array != oldArr) {
    base->m_type = KindOfVec;
    base->m_data.parr = r.array;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return r.val->asTypedValue();
}

template <bool reffy>
inline TypedValue* ElemDVecPre(TypedValue* base, StringData* key) {
  throwInvalidArrayKeyException(key, base->m_data.parr);
}

template <bool reffy>
inline TypedValue* ElemDVecPre(TypedValue* base, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return ElemDVecPre<reffy>(base, key.m_data.num);
  if (isStringType(dt))      return ElemDVecPre<reffy>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base->m_data.parr);
}

template <bool reffy, KeyType keyType>
inline TypedValue* ElemDVec(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsVecArray(base));
  assertx(tvIsPlausible(*base));
  auto* result = ElemDVecPre<reffy>(base, key);
  assertx(tvIsVecArray(base));
  assertx(tvIsPlausible(*base));
  assertx(result->m_type != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Dict
 */
template <bool reffy>
inline TypedValue* ElemDDictPre(TypedValue* base, int64_t key) {
  ArrayData* oldArr = base->m_data.parr;

  if (reffy) throwRefInvalidArrayValueException(oldArr);

  auto const r = MixedArray::LvalSilentIntDict(oldArr, key, oldArr->cowCheck());
  if (UNLIKELY(!r.val)) {
    assertx(oldArr == r.array);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (r.array != oldArr) {
    base->m_type = KindOfDict;
    base->m_data.parr = r.array;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }

  return r.val->asTypedValue();
}

template <bool reffy>
inline TypedValue* ElemDDictPre(TypedValue* base, StringData* key) {
  ArrayData* oldArr = base->m_data.parr;

  if (reffy) throwRefInvalidArrayValueException(oldArr);

  auto const r = MixedArray::LvalSilentStrDict(oldArr, key, oldArr->cowCheck());
  if (UNLIKELY(!r.val)) {
    assertx(oldArr == r.array);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (r.array != oldArr) {
    base->m_type = KindOfDict;
    base->m_data.parr = r.array;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }

  return r.val->asTypedValue();
}

template <bool reffy>
inline TypedValue* ElemDDictPre(TypedValue* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemDDictPre<reffy>(base, key.m_data.num);
  if (isStringType(dt)) return ElemDDictPre<reffy>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base->m_data.parr);
}

template <bool reffy, KeyType keyType>
inline TypedValue* ElemDDict(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  auto* result = ElemDDictPre<reffy>(base, key);
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  assertx(result->m_type != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Keyset
 */
template <bool reffy>
[[noreturn]]
inline TypedValue* ElemDKeysetPre(TypedValue* base, int64_t key) {
  if (reffy) throwRefInvalidArrayValueException(base->m_data.parr);
  throwInvalidKeysetOperation();
}

template <bool reffy>
[[noreturn]]
inline TypedValue* ElemDKeysetPre(TypedValue* base, StringData* key) {
  if (reffy) throwRefInvalidArrayValueException(base->m_data.parr);
  throwInvalidKeysetOperation();
}

template <bool reffy>
[[noreturn]]
inline TypedValue* ElemDKeysetPre(TypedValue* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    ElemDKeysetPre<reffy>(base, key.m_data.num);
  if (isStringType(dt)) ElemDKeysetPre<reffy>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base->m_data.parr);
}

template <bool reffy, KeyType keyType>
[[noreturn]]
inline TypedValue* ElemDKeyset(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  ElemDKeysetPre<reffy>(base, key);
}

/**
 * ElemD when base is Null
 */
template<MOpMode mode, KeyType keyType>
inline TypedValue* ElemDEmptyish(TypedValue* base, key_type<keyType> key) {
  auto scratchKey = initScratchKey(key);
  tvAsVariant(base) = Array::Create();
  auto const result = const_cast<TypedValue*>(
    tvAsVariant(base).asArrRef().lvalAt(
      cellAsCVarRef(scratchKey)).asTypedValue()
  );
  if (mode == MOpMode::Warn) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&scratchKey).toString().data());
  }
  return result;
}

/**
 * ElemD when base is an Int64, Double, or Resource.
 */
inline TypedValue* ElemDScalar(TypedValue& tvRef) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvRef);
  return &tvRef;
}

/**
 * ElemD when base is a Boolean
 */
template<MOpMode mode, KeyType keyType>
inline TypedValue* ElemDBoolean(TypedValue& tvRef,
                                TypedValue* base,
                                key_type<keyType> key) {
  if (base->m_data.num) {
    return ElemDScalar(tvRef);
  }
  return ElemDEmptyish<mode, keyType>(base, key);
}

/**
 * ElemD when base is a String
 */
template<MOpMode mode, KeyType keyType>
inline TypedValue* ElemDString(TypedValue* base, key_type<keyType> key) {
  if (base->m_data.pstr->size() == 0) {
    return ElemDEmptyish<mode, keyType>(base, key);
  }
  raise_error("Operator not supported for strings");
  return nullptr;
}

/**
 * ElemD when base is an Object
 */
template<MOpMode mode, bool reffy, KeyType keyType>
inline TypedValue* ElemDObject(TypedValue& tvRef, TypedValue* base,
                               key_type<keyType> key) {
  auto scratchKey = initScratchKey(key);
  auto obj = base->m_data.pobj;

  if (LIKELY(obj->isCollection())) {
    if (reffy) {
      raise_error("Collection elements cannot be taken by reference");
      return nullptr;
    }
    return collections::atLval(obj, &scratchKey);
  } else if (obj->getVMClass()->classof(SystemLib::s_ArrayObjectClass)) {
    auto storage = obj->o_realProp(s_storage, 0,
                                   SystemLib::s_ArrayObjectClass->nameStr());
    // ArrayObject should have the 'storage' property...
    assert(storage != nullptr);
    return ElemDArray<mode, reffy, keyType>(storage->asTypedValue(), key);
  }


  tvRef = objOffsetGet(instanceFromTv(base), scratchKey);
  return &tvRef;
}

/*
 * Intermediate elem operation for defining member instructions.
 *
 * Returned pointer is not yet unboxed.  (I.e. it cannot point into a RefData.)
 */
template<MOpMode mode, bool reffy, KeyType keyType = KeyType::Any>
TypedValue* ElemD(TypedValue& tvRef, TypedValue* base, key_type<keyType> key) {
  assertx(mode == MOpMode::Define);

  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return ElemDEmptyish<mode, keyType>(base, key);
    case KindOfBoolean:
      return ElemDBoolean<mode, keyType>(tvRef, base, key);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return ElemDScalar(tvRef);
    case KindOfPersistentString:
    case KindOfString:
      return ElemDString<mode, keyType>(base, key);
    case KindOfPersistentVec:
    case KindOfVec:
      return ElemDVec<reffy, keyType>(base, key);
    case KindOfPersistentDict:
    case KindOfDict:
      return ElemDDict<reffy, keyType>(base, key);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemDKeyset<reffy, keyType>(base, key);
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemDArray<mode, reffy, keyType>(base, key);
    case KindOfObject:
      return ElemDObject<mode, reffy, keyType>(tvRef, base, key);
    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

template<KeyType kt>
inline TypedValue* ElemUArrayImpl(Array& base, key_type<kt> key) {
  return base.lvalAt(keyAsValue(key)).asTypedValue();
}

template<>
inline TypedValue* ElemUArrayImpl<KeyType::Any>(Array& base, TypedValue key) {
  if (key.m_type == KindOfInt64) {
    return ElemUArrayImpl<KeyType::Int>(base, key.m_data.num);
  }
  return base.lvalAt(tvAsCVarRef(&key)).asTypedValue();
}

/**
 * ElemU when base is an Array
 */
template <KeyType keyType>
inline TypedValue* ElemUArray(TypedValue* base, key_type<keyType> key) {
  auto& baseArr = tvAsVariant(base).asArrRef();
  assertx(baseArr.isPHPArray());
  if (baseArr.exists(keyAsValue(key))) {
    return ElemUArrayImpl<keyType>(baseArr, key);
  }

  // Unset{Elem,Prop} do nothing when the base is InitNull, so this sketchy but
  // should be okay.
  return const_cast<TypedValue*>(ElemEmptyish());
}

/**
 * ElemU when base is a Vec
 */
inline TypedValue* ElemUVecPre(TypedValue* base, int64_t key) {
  ArrayData* oldArr = base->m_data.parr;
  auto const r = PackedArray::LvalSilentIntVec(oldArr, key, oldArr->cowCheck());
  if (UNLIKELY(!r.val)) {
    return const_cast<TypedValue*>(ElemEmptyish());
  }
  if (r.array != oldArr) {
    base->m_type = KindOfVec;
    base->m_data.parr = r.array;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return r.val->asTypedValue();
}

inline TypedValue* ElemUVecPre(TypedValue* base, StringData* key) {
  return const_cast<TypedValue*>(ElemEmptyish());
}

inline TypedValue* ElemUVecPre(TypedValue* base, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return ElemUVecPre(base, key.m_data.num);
  if (isStringType(dt))      return ElemUVecPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base->m_data.parr);
}

template <KeyType keyType>
inline TypedValue* ElemUVec(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsVecArray(base));
  assertx(tvIsPlausible(*base));
  auto* result = ElemUVecPre(base, key);
  assertx(tvIsVecArray(base));
  assertx(tvIsPlausible(*base));
  assertx(result->m_type != KindOfUninit);
  return result;
}

/**
 * ElemU when base is a Dict
 */
inline TypedValue* ElemUDictPre(TypedValue* base, int64_t key) {
  ArrayData* oldArr = base->m_data.parr;
  auto const r = MixedArray::LvalSilentIntDict(oldArr, key, oldArr->cowCheck());
  if (UNLIKELY(!r.val)) {
    return const_cast<TypedValue*>(ElemEmptyish());
  }
  if (r.array != oldArr) {
    base->m_type = KindOfDict;
    base->m_data.parr = r.array;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return r.val->asTypedValue();
}

inline TypedValue* ElemUDictPre(TypedValue* base, StringData* key) {
  ArrayData* oldArr = base->m_data.parr;
  auto const r = MixedArray::LvalSilentStrDict(oldArr, key, oldArr->cowCheck());
  if (UNLIKELY(!r.val)) {
    return const_cast<TypedValue*>(ElemEmptyish());
  }
  if (r.array != oldArr) {
    base->m_type = KindOfDict;
    base->m_data.parr = r.array;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return r.val->asTypedValue();
}

inline TypedValue* ElemUDictPre(TypedValue* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemUDictPre(base, key.m_data.num);
  if (isStringType(dt)) return ElemUDictPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base->m_data.parr);
}

template <KeyType keyType>
inline TypedValue* ElemUDict(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  auto* result = ElemUDictPre(base, key);
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  assertx(result->m_type != KindOfUninit);
  return result;
}

/**
 * ElemU when base is a Keyset
 */
[[noreturn]]
inline TypedValue* ElemUKeysetPre(TypedValue* base, int64_t key) {
  throwInvalidKeysetOperation();
}

[[noreturn]]
inline TypedValue* ElemUKeysetPre(TypedValue* base, StringData* key) {
  throwInvalidKeysetOperation();
}

[[noreturn]]
inline TypedValue* ElemUKeysetPre(TypedValue* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    ElemUKeysetPre(base, key.m_data.num);
  if (isStringType(dt)) ElemUKeysetPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base->m_data.parr);
}

template <KeyType keyType>
[[noreturn]]
inline TypedValue* ElemUKeyset(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  ElemUKeysetPre(base, key);
}

/**
 * ElemU when base is an Object
 */
template <KeyType keyType>
inline TypedValue* ElemUObject(TypedValue& tvRef, TypedValue* base,
                               key_type<keyType> key) {
  auto const scratchKey = initScratchKey(key);
  if (LIKELY(base->m_data.pobj->isCollection())) {
    return collections::atLval(base->m_data.pobj, &scratchKey);
  }
  tvRef = objOffsetGet(instanceFromTv(base), scratchKey);
  return &tvRef;
}

/*
 * Intermediate Elem operation for an unsetting member instruction.
 *
 * Returned pointer is not yet unboxed.  (I.e. it cannot point into a RefData.)
 */
template <KeyType keyType = KeyType::Any>
TypedValue* ElemU(TypedValue& tvRef, TypedValue* base, key_type<keyType> key) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      // Unset on scalar base never modifies the base, but the const_cast is
      // necessary to placate the type system.
      return const_cast<TypedValue*>(uninit_variant.asTypedValue());
    case KindOfPersistentString:
    case KindOfString:
      raise_error(Strings::OP_NOT_SUPPORTED_STRING);
      return nullptr;
    case KindOfPersistentVec:
    case KindOfVec:
      return ElemUVec<keyType>(base, key);
    case KindOfPersistentDict:
    case KindOfDict:
      return ElemUDict<keyType>(base, key);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemUKeyset<keyType>(base, key);
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemUArray<keyType>(base, key);
    case KindOfObject:
      return ElemUObject<keyType>(tvRef, base, key);
    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/**
 * NewElem when base is Null
 */
inline TypedValue* NewElemEmptyish(TypedValue* base) {
  Array a = Array::Create();
  TypedValue* result = const_cast<TypedValue*>(a.lvalAt().asTypedValue());
  tvAsVariant(base) = a;
  return result;
}

/**
 * NewElem when base is not a valid type (a number, true boolean,
 * non-empty string, etc.)
 */
inline TypedValue* NewElemInvalid(TypedValue& tvRef) {
  raise_warning("Cannot use a scalar value as an array");
  tvWriteNull(&tvRef);
  return &tvRef;
}

/**
 * NewElem when base is a Boolean
 */
inline TypedValue* NewElemBoolean(TypedValue& tvRef, TypedValue* base) {
  if (base->m_data.num) {
    return NewElemInvalid(tvRef);
  }
  return NewElemEmptyish(base);
}

/**
 * NewElem when base is a String
 */
inline TypedValue* NewElemString(TypedValue& tvRef, TypedValue* base) {
  if (base->m_data.pstr->size() == 0) {
    return NewElemEmptyish(base);
  }
  return NewElemInvalid(tvRef);
}

/**
 * NewElem when base is an Array
 */
template <bool reffy>
inline TypedValue* NewElemArray(TypedValue* base) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  return reffy ?
    tvAsVariant(base).asArrRef().lvalAtRef().asTypedValue() :
    tvAsVariant(base).asArrRef().lvalAt().asTypedValue();
}

/**
 * NewElem when base is an Object
 */
inline TypedValue* NewElemObject(TypedValue& tvRef, TypedValue* base) {
  if (base->m_data.pobj->isCollection()) {
    throw_cannot_use_newelem_for_lval_read_col();
    return nullptr;
  }
  tvRef = objOffsetGet(instanceFromTv(base), make_tv<KindOfNull>());
  return &tvRef;
}

/**
 * $result = ($base[] = ...);
 */
template <bool reffy>
inline TypedValue* NewElem(TypedValue& tvRef,
                           TypedValue* base) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return NewElemEmptyish(base);
    case KindOfBoolean:
      return NewElemBoolean(tvRef, base);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return NewElemInvalid(tvRef);
    case KindOfPersistentString:
    case KindOfString:
      return NewElemString(tvRef, base);
    case KindOfPersistentVec:
    case KindOfVec:
      throw_cannot_use_newelem_for_lval_read_vec();
    case KindOfPersistentDict:
    case KindOfDict:
      throw_cannot_use_newelem_for_lval_read_dict();
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throw_cannot_use_newelem_for_lval_read_keyset();
    case KindOfPersistentArray:
    case KindOfArray:
      return NewElemArray<reffy>(base);
    case KindOfObject:
      return NewElemObject(tvRef, base);
    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/**
 * SetElem when base is Null
 */
template <KeyType keyType>
inline void SetElemEmptyish(TypedValue* base, key_type<keyType> key,
                            Cell* value) {
  auto const& scratchKey = initScratchKey(key);
  tvAsVariant(base) = Array::Create();
  tvAsVariant(base).asArrRef().set(tvAsCVarRef(&scratchKey),
                                   tvAsCVarRef(value));
}

/**
 * SetElem when base is an Int64, Double, or Resource.
 */
template <bool setResult>
inline void SetElemScalar(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (!setResult) {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  tvRefcountedDecRef((TypedValue*)value);
  tvWriteNull((TypedValue*)value);
}

/**
 * SetElem when base is a Boolean
 */
template <bool setResult, KeyType keyType>
inline void SetElemBoolean(TypedValue* base, key_type<keyType> key,
                           Cell* value) {
  if (base->m_data.num) {
    SetElemScalar<setResult>(value);
  } else {
    SetElemEmptyish<keyType>(base, key, value);
  }
}

/**
 * Convert a key to integer for SetElem
 */
template<KeyType keyType>
inline int64_t castKeyToInt(key_type<keyType> key) {
  return cellToInt(initScratchKey(key));
}

template<>
inline int64_t castKeyToInt<KeyType::Int>(int64_t key) {
  return key;
}

/**
 * SetElem when base is a String
 */
template <bool setResult, KeyType keyType>
inline StringData* SetElemString(TypedValue* base, key_type<keyType> key,
                                 Cell* value) {
  int baseLen = base->m_data.pstr->size();
  if (baseLen == 0) {
    SetElemEmptyish<keyType>(base, key, value);
    if (!setResult) {
      tvRefcountedIncRef(value);
      throw InvalidSetMException(*value);
    }
    return nullptr;
  }

  // Convert key to string offset.
  int64_t x = castKeyToInt<keyType>(key);
  if (UNLIKELY(x < 0 || x >= StringData::MaxSize)) {
    // Can't use PRId64 here because of order of inclusion issues
    raise_warning("Illegal string offset: %lld", (long long)x);
    if (!setResult) {
      throw InvalidSetMException(make_tv<KindOfNull>());
    }
    tvRefcountedDecRef(value);
    tvWriteNull(value);
    return nullptr;
  }

  // Compute how long the resulting string will be. Type needs
  // to agree with x.
  int64_t slen;
  if (x >= baseLen) {
    slen = x + 1;
  } else {
    slen = baseLen;
  }

  // Extract the first character of (string)value.
  char y[2];
  {
    StringData* valStr;
    if (LIKELY(isStringType(value->m_type))) {
      valStr = value->m_data.pstr;
      valStr->incRefCount();
    } else {
      valStr = tvCastToString(value);
    }

    if (valStr->size() > 0) {
      y[0] = valStr->data()[0];
      y[1] = '\0';
    } else {
      y[0] = '\0';
    }
    decRefStr(valStr);
  }

  // Create and save the result.
  if (x >= 0 && x < baseLen && !base->m_data.pstr->cowCheck()) {
    // Modify base in place.  This is safe because the LHS owns the
    // only reference.
    auto const oldp = base->m_data.pstr;
    auto const newp = oldp->modifyChar(x, y[0]);
    if (UNLIKELY(newp != oldp)) {
      decRefStr(oldp);
      base->m_data.pstr = newp;
      base->m_type = KindOfString;
    }
  } else {
    StringData* sd = StringData::Make(slen);
    char* s = sd->mutableData();
    memcpy(s, base->m_data.pstr->data(), baseLen);
    if (x > baseLen) {
      memset(&s[baseLen], ' ', slen - baseLen - 1);
    }
    s[x] = y[0];
    sd->setSize(slen);
    decRefStr(base->m_data.pstr);
    base->m_data.pstr = sd;
    base->m_type = KindOfString;
  }

  return StringData::Make(y, strlen(y), CopyString);
}

/**
 * SetElem when base is an Object
 */
template <KeyType keyType>
inline void SetElemObject(TypedValue* base, key_type<keyType> key,
                          Cell* value) {
  auto const scratchKey = initScratchKey(key);
  if (LIKELY(base->m_data.pobj->isCollection())) {
    collections::set(base->m_data.pobj, &scratchKey, value);
  } else {
    objOffsetSet(instanceFromTv(base), scratchKey, value);
  }
}

/*
 * arrayRefShuffle is used by SetElemArray and by helpers for translated code
 * to do the necessary bookkeeping after mutating an array. The helpers return
 * an ArrayData* if and only if the base array was not in a php reference. If
 * the base array was in a reference, that reference may no longer refer to an
 * array after the set operation, so the helpers don't return anything.
 */
template<bool setRef> struct ShuffleReturn {};

template<> struct ShuffleReturn<true> {
  typedef void return_type;
  static void do_return(ArrayData* a) {}
};

template<> struct ShuffleReturn<false> {
  typedef ArrayData* return_type;
  static ArrayData* do_return(ArrayData* a) { return a; }
};

template<bool setRef, DataType dt> inline
typename ShuffleReturn<setRef>::return_type
arrayRefShuffle(ArrayData* oldData, ArrayData* newData, TypedValue* base) {
  if (newData == oldData) {
    return ShuffleReturn<setRef>::do_return(oldData);
  }

  if (setRef) {
    if (isArrayLikeType(base->m_type) && base->m_data.parr == oldData) {
      base->m_type = dt;
      base->m_data.parr = newData;
      assertx(cellIsPlausible(*base));
    } else {
      // The base was in a reference that was overwritten by the set operation,
      // so we don't want to store the new ArrayData to it. oldData has already
      // been decrefed and there's nobody left to care about newData, so decref
      // newData instead of oldData.
      oldData = newData;
    }
  }
  decRefArr(oldData);
  return ShuffleReturn<setRef>::do_return(newData);
}


/**
 * SetElem helper with Array base and Int64 key
 */
template<bool setResult>
inline ArrayData* SetElemArrayPre(ArrayData* a,
                                  int64_t key,
                                  Cell* value,
                                  bool copy) {
  return a->set(key, cellAsCVarRef(*value), copy);
}

/**
 * SetElem helper with Array base and String key
 */
template<bool setResult>
inline ArrayData* SetElemArrayPre(ArrayData* a,
                                  StringData* key,
                                  Cell* value,
                                  bool copy) {
  int64_t n;
  return a->convertKey(key, n)
    ? a->set(n, cellAsCVarRef(*value), copy)
    : a->set(StrNR(key), cellAsCVarRef(*value), copy);
}

template<bool setResult>
inline ArrayData* SetElemArrayPre(ArrayData* a,
                                  TypedValue key,
                                  Cell* value,
                                  bool copy) {
  if (isStringType(key.m_type)) {
    return SetElemArrayPre<setResult>(a, key.m_data.pstr, value, copy);
  }
  if (key.m_type == KindOfInt64) {
    return SetElemArrayPre<setResult>(a, key.m_data.num, value, copy);
  }
  if (isNullType(key.m_type)) {
    return a->set(staticEmptyString(), cellAsCVarRef(*value), copy);
  }
  if (!isArrayLikeType(key.m_type) && key.m_type != KindOfObject) {
    return SetElemArrayPre<setResult>(a, tvAsCVarRef(&key).toInt64(),
                                      value, copy);
  }

  raise_warning("Illegal offset type");
  // Assignment failed, so the result is null rather than the RHS.
  if (setResult) {
    tvRefcountedDecRef(value);
    tvWriteNull(value);
  } else {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  return a;
}

/**
 * SetElem when base is an Array
 */
template <bool setResult, KeyType keyType>
inline void SetElemArray(TypedValue* base, key_type<keyType> key,
                         Cell* value) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = base->m_data.parr;
  bool copy = a->cowCheck() ||
    (tvIsArray(value) && value->m_data.parr == a);

  auto* newData = SetElemArrayPre<setResult>(a, key, value, copy);
  assertx(newData->isPHPArray());

  arrayRefShuffle<true, KindOfArray>(a, newData, base);
}

/**
 * SetElem when base is a Vec
 */
template<bool setResult>
inline ArrayData* SetElemVecPre(ArrayData* a,
                                int64_t key,
                                Cell* value,
                                bool copy) {
  return PackedArray::SetIntVec(a, key, *value, copy);
}

template<bool setResult>
inline ArrayData* SetElemVecPre(ArrayData* a,
                                StringData* key,
                                Cell* value,
                                bool copy) {
  throwInvalidArrayKeyException(key, a);
}

template<bool setResult>
inline ArrayData* SetElemVecPre(ArrayData* a,
                                TypedValue key,
                                Cell* value,
                                bool copy) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return SetElemVecPre<setResult>(a, key.m_data.num,
                                                             value, copy);
  if (isStringType(dt))      return SetElemVecPre<setResult>(a, key.m_data.pstr,
                                                             value, copy);
  throwInvalidArrayKeyException(&key, a);
}

template <bool setResult, KeyType keyType>
inline void SetElemVec(TypedValue* base, key_type<keyType> key,
                       Cell* value) {
  assertx(tvIsVecArray(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = base->m_data.parr;
  bool copy = a->cowCheck() ||
    (tvIsVecArray(value) && value->m_data.parr == a);

  auto* newData = SetElemVecPre<setResult>(a, key, value, copy);
  assertx(newData->isVecArray());

  arrayRefShuffle<true, KindOfVec>(a, newData, base);
}

/**
 * SetElem when base is a Dict
 */
template<bool setResult>
inline ArrayData* SetElemDictPre(ArrayData* a,
                                 int64_t key,
                                 Cell* value,
                                 bool copy) {
  return MixedArray::SetIntDict(a, key, *value, copy);
}

template<bool setResult>
inline ArrayData* SetElemDictPre(ArrayData* a,
                                 StringData* key,
                                 Cell* value,
                                 bool copy) {
  return MixedArray::SetStrDict(a, key, *value, copy);
}

template<bool setResult>
inline ArrayData* SetElemDictPre(ArrayData* a,
                                 TypedValue key,
                                 Cell* value,
                                 bool copy) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return SetElemDictPre<setResult>(a, key.m_data.num,
                                                         value, copy);
  if (isStringType(dt)) return SetElemDictPre<setResult>(a, key.m_data.pstr,
                                                         value, copy);
  throwInvalidArrayKeyException(&key, a);
}

template <bool setResult, KeyType keyType>
inline void SetElemDict(TypedValue* base, key_type<keyType> key,
                        Cell* value) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = base->m_data.parr;
  bool copy = a->cowCheck() ||
    (tvIsDict(value) && value->m_data.parr == a);

  auto* newData = SetElemDictPre<setResult>(a, key, value, copy);
  assertx(newData->isDict());

  arrayRefShuffle<true, KindOfDict>(a, newData, base);
}

/**
 * SetElem() leaves the result in 'value', rather than returning it as in
 * SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
 * get around.
 */
template <bool setResult, KeyType keyType>
NEVER_INLINE
StringData* SetElemSlow(TypedValue* base, key_type<keyType> key, Cell* value) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      SetElemEmptyish<keyType>(base, key, value);
      return nullptr;
    case KindOfBoolean:
      SetElemBoolean<setResult, keyType>(base, key, value);
      return nullptr;
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      SetElemScalar<setResult>(value);
      return nullptr;
    case KindOfPersistentString:
    case KindOfString:
      return SetElemString<setResult, keyType>(base, key, value);
    case KindOfPersistentVec:
    case KindOfVec:
      SetElemVec<setResult, keyType>(base, key, value);
      return nullptr;
    case KindOfPersistentDict:
    case KindOfDict:
      SetElemDict<setResult, keyType>(base, key, value);
      return nullptr;
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();
    case KindOfPersistentArray:
    case KindOfArray:
      SetElemArray<setResult, keyType>(base, key, value);
      return nullptr;
    case KindOfObject:
      SetElemObject<keyType>(base, key, value);
      return nullptr;
    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/**
 * Fast path for SetElem assuming base is an Array
 */
template <bool setResult, KeyType keyType = KeyType::Any>
inline StringData* SetElem(TypedValue* base, key_type<keyType> key,
                           Cell* value) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    SetElemArray<setResult, keyType>(base, key, value);
    return nullptr;
  }
  if (LIKELY(tvIsVecArray(base))) {
    SetElemVec<setResult, keyType>(base, key, value);
    return nullptr;
  }
  if (LIKELY(tvIsDict(base))) {
    SetElemDict<setResult, keyType>(base, key, value);
    return nullptr;
  }
  return SetElemSlow<setResult, keyType>(base, key, value);
}

/**
 * SetNewElem when base is Null
 */
inline void SetNewElemEmptyish(TypedValue* base, Cell* value) {
  Array a = Array::Create();
  a.append(cellAsCVarRef(*value));
  tvAsVariant(base) = a;
}

/**
 * SetNewElem when base is Int64 or Double
 */
template <bool setResult>
inline void SetNewElemScalar(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (!setResult) {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  tvRefcountedDecRef((TypedValue*)value);
  tvWriteNull((TypedValue*)value);
}

/**
 * SetNewElem when base is a Boolean
 */
template <bool setResult>
inline void SetNewElemBoolean(TypedValue* base, Cell* value) {
  if (base->m_data.num) {
    SetNewElemScalar<setResult>(value);
  } else {
    SetNewElemEmptyish(base, value);
  }
}

/**
 * SetNewElem when base is a String
 */
inline void SetNewElemString(TypedValue* base, Cell* value) {
  int baseLen = base->m_data.pstr->size();
  if (baseLen == 0) {
    SetNewElemEmptyish(base, value);
  } else {
    raise_error("[] operator not supported for strings");
  }
}

/**
 * SetNewElem when base is an Array
 */
inline void SetNewElemArray(TypedValue* base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  auto a = base->m_data.parr;
  auto const copy = a->cowCheck() ||
    (tvIsArray(value) && value->m_data.parr == a);
  auto a2 = a->append(*value, copy);
  if (a2 != a) {
    base->m_type = KindOfArray;
    base->m_data.parr = a2;
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Vec
 */
inline void SetNewElemVec(TypedValue* base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsVecArray(base));
  assertx(tvIsPlausible(*base));
  auto a = base->m_data.parr;
  auto const copy = a->cowCheck() ||
    (tvIsVecArray(value) && value->m_data.parr == a);
  auto a2 = PackedArray::AppendVec(a, *value, copy);
  if (a2 != a) {
    base->m_type = KindOfVec;
    base->m_data.parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Dict
 */
inline void SetNewElemDict(TypedValue* base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  auto a = base->m_data.parr;
  auto const copy = a->cowCheck() ||
    (tvIsDict(value) && value->m_data.parr == a);
  auto a2 = MixedArray::AppendDict(a, *value, copy);
  if (a2 != a) {
    base->m_type = KindOfDict;
    base->m_data.parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Keyset
 */
inline void SetNewElemKeyset(TypedValue* base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  auto a = base->m_data.parr;
  auto const copy = a->cowCheck() ||
    (tvIsKeyset(value) && value->m_data.parr == a);
  auto a2 = SetArray::Append(a, *value, copy);
  if (a2 != a) {
    base->m_type = KindOfKeyset;
    base->m_data.parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is an Object
 */
inline void SetNewElemObject(TypedValue* base, Cell* value) {
  if (LIKELY(base->m_data.pobj->isCollection())) {
    collections::append(base->m_data.pobj, (TypedValue*)value);
  } else {
    objOffsetAppend(instanceFromTv(base), (TypedValue*)value);
  }
}

/**
 * $base[] = ...
 */
template <bool setResult>
inline void SetNewElem(TypedValue* base, Cell* value) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return SetNewElemEmptyish(base, value);
    case KindOfBoolean:
      return SetNewElemBoolean<setResult>(base,  value);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return SetNewElemScalar<setResult>(value);
    case KindOfPersistentString:
    case KindOfString:
      return SetNewElemString(base, value);
    case KindOfPersistentVec:
    case KindOfVec:
      return SetNewElemVec(base, value);
    case KindOfPersistentDict:
    case KindOfDict:
      return SetNewElemDict(base, value);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return SetNewElemKeyset(base, value);
    case KindOfPersistentArray:
    case KindOfArray:
      return SetNewElemArray(base, value);
    case KindOfObject:
      return SetNewElemObject(base, value);
    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/**
 * SetOpElem when base is Null
 */
inline TypedValue* SetOpElemEmptyish(SetOpOp op, Cell* base,
                                     TypedValue key, Cell* rhs) {
  assert(cellIsPlausible(*base));

  Array a = Array::Create();
  TypedValue* result = const_cast<TypedValue*>(a.lvalAt(tvAsCVarRef(&key))
                                               .asTypedValue());
  tvAsVariant(base) = a;
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&key).toString().data());
  }
  setopBody(result, op, rhs);
  return result;
}

/**
 * SetOpElem when base is Int64 or Double
 */
inline TypedValue* SetOpElemScalar(TypedValue& tvRef) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvRef);
  return &tvRef;
}

/**
 * $result = ($base[$x] <op>= $y)
 */
inline TypedValue* SetOpElem(TypedValue& tvRef,
                             SetOpOp op, TypedValue* base,
                             TypedValue key, Cell* rhs) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpElemEmptyish(op, base, key, rhs);

    case KindOfBoolean:
      if (base->m_data.num) {
        return SetOpElemScalar(tvRef);
      }
      return SetOpElemEmptyish(op, base, key, rhs);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return SetOpElemScalar(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        raise_error("Cannot use assign-op operators with overloaded "
          "objects nor string offsets");
      }
      return SetOpElemEmptyish(op, base, key, rhs);

    case KindOfPersistentVec:
    case KindOfVec: {
      TypedValue* result;
      result = ElemDVec<false, KeyType::Any>(base, key);
      result = tvAssertCell(result);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      TypedValue* result;
      result = ElemDDict<false, KeyType::Any>(base, key);
      result = tvAssertCell(result);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    case KindOfPersistentArray:
    case KindOfArray: {
      TypedValue* result;
      auto constexpr mode = MoreWarnings ? MOpMode::Warn : MOpMode::None;
      result = ElemDArray<mode, false, KeyType::Any>(base, key);
      result = tvToCell(result);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfObject: {
      TypedValue* result;
      if (LIKELY(base->m_data.pobj->isCollection())) {
        result = collections::atRw(base->m_data.pobj, &key);
        setopBody(tvToCell(result), op, rhs);
      } else {
        tvRef = objOffsetGet(instanceFromTv(base), key);
        result = &tvRef;
        setopBody(tvToCell(result), op, rhs);
        objOffsetSet(instanceFromTv(base), key, result, false);
      }
      return result;
    }

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

inline TypedValue* SetOpNewElemEmptyish(SetOpOp op,
                                        TypedValue* base, Cell* rhs) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt();
  tvAsVariant(base) = a;
  setopBody(tvToCell(result), op, rhs);
  return result;
}
inline TypedValue* SetOpNewElemScalar(TypedValue& tvRef) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvRef);
  return &tvRef;
}
inline TypedValue* SetOpNewElem(TypedValue& tvRef,
                                SetOpOp op, TypedValue* base,
                                Cell* rhs) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpNewElemEmptyish(op, base, rhs);

    case KindOfBoolean:
      if (base->m_data.num) {
        return SetOpNewElemScalar(tvRef);
      }
      return SetOpNewElemEmptyish(op, base, rhs);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return SetOpNewElemScalar(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        raise_error("[] operator not supported for strings");
      }
      return SetOpNewElemEmptyish(op, base, rhs);

    case KindOfPersistentVec:
    case KindOfVec:
      throw_cannot_use_newelem_for_lval_read_vec();
    case KindOfPersistentDict:
    case KindOfDict:
      throw_cannot_use_newelem_for_lval_read_dict();
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throw_cannot_use_newelem_for_lval_read_keyset();

    case KindOfPersistentArray:
    case KindOfArray: {
      TypedValue* result;
      result = (TypedValue*)&tvAsVariant(base).asArrRef().lvalAt();
      setopBody(tvToCell(result), op, rhs);
      return result;
    }

    case KindOfObject: {
      TypedValue* result;
      if (base->m_data.pobj->isCollection()) {
        throw_cannot_use_newelem_for_lval_read_col();
        result = nullptr;
      } else {
        tvRef = objOffsetGet(instanceFromTv(base), make_tv<KindOfNull>());
        result = &tvRef;
        setopBody(tvToCell(result), op, rhs);
        objOffsetAppend(instanceFromTv(base), result, false);
      }
      return result;
    }

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

Cell incDecBodySlow(IncDecOp op, Cell* fr);

inline Cell IncDecBody(IncDecOp op, Cell* fr) {
  assert(cellIsPlausible(*fr));

  if (UNLIKELY(fr->m_type != KindOfInt64)) {
    return incDecBodySlow(op, fr);
  }

  auto copy = [&]() {
    assert(cellIsPlausible(*fr));
    return *fr;
  };

  // fast cases, assuming integers overflow to ints
  switch (op) {
  case IncDecOp::PreInc:
    ++fr->m_data.num;
    return copy();
  case IncDecOp::PostInc: {
    auto const tmp = copy();
    ++fr->m_data.num;
    return tmp;
  }
  case IncDecOp::PreDec:
    --fr->m_data.num;
    return copy();
  case IncDecOp::PostDec: {
    auto const tmp = copy();
    --fr->m_data.num;
    return tmp;
  }
  default: break;
  }

  // slow case, where integers can overflow to floats
  switch (op) {
  case IncDecOp::PreIncO:
    cellIncO(*fr);
    return copy();
  case IncDecOp::PostIncO: {
    auto const tmp = copy();
    cellIncO(*fr);
    return tmp;
  }
  case IncDecOp::PreDecO:
    cellDecO(*fr);
    return copy();
  case IncDecOp::PostDecO: {
    auto const tmp = copy();
    cellDecO(*fr);
    return tmp;
  }
  default: break;
  }
  not_reached();
}

inline Cell IncDecElemEmptyish(
  IncDecOp op,
  TypedValue* base,
  TypedValue key
) {
  auto a = Array::Create();
  auto result = (TypedValue*)&a.lvalAt(tvAsCVarRef(&key));
  tvAsVariant(base) = a;
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&key).toString().data());
  }
  assert(result->m_type == KindOfNull);
  return IncDecBody(op, result);
}

inline Cell IncDecElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

inline Cell IncDecElem(
  IncDecOp op,
  TypedValue* base,
  TypedValue key
) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecElemEmptyish(op, base, key);

    case KindOfBoolean:
      if (base->m_data.num) {
        return IncDecElemScalar();
      }
      return IncDecElemEmptyish(op, base, key);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return IncDecElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        raise_error("Cannot increment/decrement overloaded objects "
          "nor string offsets");
      }
      return IncDecElemEmptyish(op, base, key);

    case KindOfPersistentVec:
    case KindOfVec: {
      auto result = ElemDVec<false, KeyType::Any>(base, key);
      return IncDecBody(op, tvAssertCell(result));
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      auto result = ElemDDict<false, KeyType::Any>(base, key);
      return IncDecBody(op, tvAssertCell(result));
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    case KindOfPersistentArray:
    case KindOfArray: {
      auto constexpr mode = MoreWarnings ? MOpMode::Warn : MOpMode::None;
      auto result = ElemDArray<mode, false, KeyType::Any>(base, key);
      return IncDecBody(op, tvToCell(result));
    }

    case KindOfObject: {
      TypedValue* result;
      auto localTvRef = make_tv<KindOfUninit>();

      if (LIKELY(base->m_data.pobj->isCollection())) {
        result = collections::atRw(base->m_data.pobj, &key);
        assert(cellIsPlausible(*result));
      } else {
        localTvRef = objOffsetGet(instanceFromTv(base), key);
        result = tvToCell(&localTvRef);
      }

      auto const dest = IncDecBody(op, result);
      tvRefcountedDecRef(localTvRef);
      return dest;
    }

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

inline Cell IncDecNewElemEmptyish(
  IncDecOp op,
  TypedValue* base
) {
  auto a = Array::Create();
  auto result = (TypedValue*)&a.lvalAt();
  tvAsVariant(base) = a;
  assert(result->m_type == KindOfNull);
  return IncDecBody(op, result);
}

inline Cell IncDecNewElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

inline Cell IncDecNewElem(
  TypedValue& tvRef,
  IncDecOp op,
  TypedValue* base
) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecNewElemEmptyish(op, base);

    case KindOfBoolean:
      if (base->m_data.num) {
        return IncDecNewElemScalar();
      }
      return IncDecNewElemEmptyish(op, base);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return IncDecNewElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        raise_error("[] operator not supported for strings");
      }
      return IncDecNewElemEmptyish(op, base);

    case KindOfPersistentVec:
    case KindOfVec:
      throw_cannot_use_newelem_for_lval_read_vec();
    case KindOfPersistentDict:
    case KindOfDict:
      throw_cannot_use_newelem_for_lval_read_dict();
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throw_cannot_use_newelem_for_lval_read_keyset();

    case KindOfPersistentArray:
    case KindOfArray: {
      TypedValue* result = (TypedValue*)&tvAsVariant(base).asArrRef().lvalAt();
      assert(result->m_type == KindOfNull);
      return IncDecBody(op, tvToCell(result));
    }

    case KindOfObject: {
      TypedValue* result;
      if (base->m_data.pobj->isCollection()) {
        throw_cannot_use_newelem_for_lval_read_col();
      }
      tvRef = objOffsetGet(instanceFromTv(base), make_tv<KindOfNull>());
      result = tvToCell(&tvRef);
      return IncDecBody(op, result);
    }

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/**
 * UnsetElemArray when key is an Int64
 */
inline ArrayData* UnsetElemArrayPre(ArrayData* a, int64_t key,
                                    bool copy) {
  return a->remove(key, copy);
}

/**
 * UnsetElemArray when key is a String
 */
inline ArrayData* UnsetElemArrayPre(ArrayData* a, StringData* key,
                                    bool copy) {
  int64_t n;
  return !a->convertKey(key, n)
    ? a->remove(StrNR(key), copy)
    : a->remove(n, copy);
}

inline ArrayData* UnsetElemArrayPre(ArrayData* a, TypedValue key,
                                    bool copy) {
  if (isStringType(key.m_type)) {
    return UnsetElemArrayPre(a, key.m_data.pstr, copy);
  }
  if (key.m_type == KindOfInt64) {
    return UnsetElemArrayPre(a, key.m_data.num, copy);
  }
  VarNR varKey = tvAsCVarRef(&key).toKey(a);
  if (varKey.isNull()) {
    return a;
  }
  return a->remove(varKey, copy);
}

/**
 * UnsetElem when base is an Array
 */
template <KeyType keyType>
inline void UnsetElemArray(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = base->m_data.parr;
  ArrayData* a2 = UnsetElemArrayPre(a, key, a->cowCheck());

  if (a2 != a) {
    base->m_type = KindOfArray;
    base->m_data.parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * UnsetElem when base is a Vec
 */

inline ArrayData* UnsetElemVecPre(ArrayData* a, int64_t key,
                                  bool copy) {
  return PackedArray::RemoveIntVec(a, key, copy);
}

inline ArrayData* UnsetElemVecPre(ArrayData* a, StringData* key,
                                  bool copy) {
  /* Never contains strings, so a no-op. */
  return a;
}

inline ArrayData* UnsetElemVecPre(ArrayData* a, TypedValue key,
                                  bool copy) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return UnsetElemVecPre(a, key.m_data.num, copy);
  if (isStringType(dt))      return UnsetElemVecPre(a, key.m_data.pstr, copy);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemVec(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsVecArray(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = base->m_data.parr;
  ArrayData* a2 = UnsetElemVecPre(a, key, a->cowCheck());
  assertx(a2->isVecArray() || a2->isDict());

  if (a2 != a) {
    base->m_type = a2->toDataType();
    base->m_data.parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * UnsetElem when base is a Dict
 */

inline ArrayData* UnsetElemDictPre(ArrayData* a, int64_t key,
                                   bool copy) {
  return MixedArray::RemoveIntDict(a, key, copy);
}

inline ArrayData* UnsetElemDictPre(ArrayData* a, StringData* key,
                                   bool copy) {
  return MixedArray::RemoveStrDict(a, key, copy);
}

inline ArrayData* UnsetElemDictPre(ArrayData* a, TypedValue key,
                                   bool copy) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return UnsetElemDictPre(a, key.m_data.num, copy);
  if (isStringType(dt)) return UnsetElemDictPre(a, key.m_data.pstr, copy);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemDict(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = base->m_data.parr;
  ArrayData* a2 = UnsetElemDictPre(a, key, a->cowCheck());

  if (a2 != a) {
    base->m_type = KindOfDict;
    base->m_data.parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * UnsetElem when base is a Keyset
 */

inline ArrayData* UnsetElemKeysetPre(ArrayData* a, int64_t key,
                                     bool copy) {
  return SetArray::RemoveInt(a, key, copy);
}

inline ArrayData* UnsetElemKeysetPre(ArrayData* a, StringData* key,
                                     bool copy) {
  return SetArray::RemoveStr(a, key, copy);
}

inline ArrayData* UnsetElemKeysetPre(ArrayData* a, TypedValue key,
                                     bool copy) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return UnsetElemKeysetPre(a, key.m_data.num, copy);
  if (isStringType(dt)) return UnsetElemKeysetPre(a, key.m_data.pstr, copy);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemKeyset(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = base->m_data.parr;
  ArrayData* a2 = UnsetElemKeysetPre(a, key, a->cowCheck());

  if (a2 != a) {
    base->m_type = KindOfKeyset;
    base->m_data.parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * unset($base[$member])
 */
template <KeyType keyType>
NEVER_INLINE
void UnsetElemSlow(TypedValue* base, key_type<keyType> key) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return; // Do nothing.

    case KindOfPersistentString:
    case KindOfString:
      raise_error(Strings::CANT_UNSET_STRING);
      return;

    case KindOfPersistentVec:
    case KindOfVec:
      UnsetElemVec<keyType>(base, key);
      return;

    case KindOfPersistentDict:
    case KindOfDict:
      UnsetElemDict<keyType>(base, key);
      return;

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      UnsetElemKeyset<keyType>(base, key);
      return;

    case KindOfPersistentArray:
    case KindOfArray:
      UnsetElemArray<keyType>(base, key);
      return;

    case KindOfObject: {
      auto const& scratchKey = initScratchKey(key);
      if (LIKELY(base->m_data.pobj->isCollection())) {
        collections::unset(base->m_data.pobj, &scratchKey);
      } else {
        objOffsetUnset(instanceFromTv(base), scratchKey);
      }
      return;
    }

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/**
 * Fast path for UnsetElem assuming base is an Array
 */
template <KeyType keyType = KeyType::Any>
inline void UnsetElem(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    return UnsetElemArray<keyType>(base, key);
  }
  if (LIKELY(tvIsVecArray(base))) {
    return UnsetElemVec<keyType>(base, key);
  }
  if (LIKELY(tvIsDict(base))) {
    return UnsetElemDict<keyType>(base, key);
  }
  if (LIKELY(tvIsKeyset(base))) {
    return UnsetElemKeyset<keyType>(base, key);
  }
  return UnsetElemSlow<keyType>(base, key);
}

/**
 * IssetEmptyElem when base is an Object
 */
template<bool useEmpty, KeyType keyType>
bool IssetEmptyElemObj(ObjectData* instance, key_type<keyType> key) {
  auto scratchKey = initScratchKey(key);
  if (LIKELY(instance->isCollection())) {
    return useEmpty
      ? collections::empty(instance, &scratchKey)
      : collections::isset(instance, &scratchKey);
  }

  return useEmpty
    ? objOffsetEmpty(instance, scratchKey)
    : objOffsetIsset(instance, scratchKey);
}

/**
 * IssetEmptyElem when base is a String
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemString(TypedValue* base, key_type<keyType> key) {
  // TODO Task #2716479: Fix this so that the warnings raised match
  // PHP5.
  auto scratchKey = initScratchKey(key);
  int64_t x;
  if (LIKELY(scratchKey.m_type == KindOfInt64)) {
    x = scratchKey.m_data.num;
  } else {
    TypedValue tv;
    cellDup(scratchKey, tv);
    bool badKey = false;
    if (isStringType(tv.m_type)) {
      const char* str = tv.m_data.pstr->data();
      size_t len = tv.m_data.pstr->size();
      while (len > 0 &&
             (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n')) {
        ++str;
        --len;
      }
      int64_t n;
      badKey = !is_strictly_integer(str, len, n);
    } else if (isArrayLikeType(tv.m_type) || tv.m_type == KindOfObject ||
               tv.m_type == KindOfResource) {
      badKey = true;
    }
    // Even if badKey == true, we still perform the cast so that we
    // raise the appropriate warnings.
    tvCastToInt64InPlace(&tv);
    if (badKey) {
      return useEmpty;
    }
    x = tv.m_data.num;
  }
  if (x < 0 || x >= base->m_data.pstr->size()) {
    return useEmpty;
  }
  if (!useEmpty) {
    return true;
  }

  auto str = base->m_data.pstr->getChar(x);
  assert(str->isStatic());
  return !str->toBoolean();
}

/**
 * IssetEmptyElem when base is an Array
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemArray(ArrayData* a, key_type<keyType> key) {
  assertx(a->isPHPArray());
  auto const result = ElemArray<MOpMode::None, keyType>(a, key);
  if (useEmpty) {
    return !cellToBool(*tvToCell(result));
  }
  return !cellIsNull(tvToCell(result));
}

/**
 * IssetEmptyElem when base is a Vec
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemVec(ArrayData* a, key_type<keyType> key) {
  assertx(a->isVecArray());
  auto const result = ElemVec<MOpMode::None, keyType>(a, key);
  if (useEmpty) {
    return !cellToBool(*tvAssertCell(result));
  }
  return !cellIsNull(tvAssertCell(result));
}

/**
 * IssetEmptyElem when base is a Dict
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemDict(ArrayData* a, key_type<keyType> key) {
  assertx(a->isDict());
  auto const result = ElemDict<MOpMode::None, keyType>(a, key);
  if (useEmpty) {
    return !cellToBool(*tvAssertCell(result));
  }
  return !cellIsNull(tvAssertCell(result));
}

/**
 * IssetEmptyElem when base is a Keyset
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemKeyset(ArrayData* a, key_type<keyType> key) {
  assertx(a->isKeyset());
  auto const result = ElemKeyset<MOpMode::None, keyType>(a, key);
  if (useEmpty) {
    return !cellToBool(*tvAssertCell(result));
  }
  return !cellIsNull(tvAssertCell(result));
}

/**
 * isset/empty($base[$key])
 */
template <bool useEmpty, KeyType keyType>
NEVER_INLINE bool IssetEmptyElemSlow(TypedValue* base, key_type<keyType> key) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return useEmpty;

    case KindOfPersistentString:
    case KindOfString:
      return IssetEmptyElemString<useEmpty, keyType>(base, key);

    case KindOfPersistentVec:
    case KindOfVec:
      return IssetEmptyElemVec<useEmpty, keyType>(base->m_data.parr, key);

    case KindOfPersistentDict:
    case KindOfDict:
      return IssetEmptyElemDict<useEmpty, keyType>(base->m_data.parr, key);

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return IssetEmptyElemKeyset<useEmpty, keyType>(base->m_data.parr, key);

    case KindOfPersistentArray:
    case KindOfArray:
      return IssetEmptyElemArray<useEmpty, keyType>(base->m_data.parr, key);

    case KindOfObject:
      return IssetEmptyElemObj<useEmpty, keyType>(base->m_data.pobj, key);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

/**
 * Fast path for IssetEmptyElem assuming base is an Array
 */
template <bool useEmpty, KeyType keyType = KeyType::Any>
bool IssetEmptyElem(TypedValue* base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    return IssetEmptyElemArray<useEmpty, keyType>(base->m_data.parr, key);
  }
  if (LIKELY(tvIsVecArray(base))) {
    return IssetEmptyElemVec<useEmpty, keyType>(base->m_data.parr, key);
  }
  if (LIKELY(tvIsDict(base))) {
    return IssetEmptyElemDict<useEmpty, keyType>(base->m_data.parr, key);
  }
  if (LIKELY(tvIsKeyset(base))) {
    return IssetEmptyElemKeyset<useEmpty, keyType>(base->m_data.parr, key);
  }
  return IssetEmptyElemSlow<useEmpty, keyType>(base, key);
}

template<MOpMode mode>
inline TypedValue* propPreNull(TypedValue& tvRef) {
  tvWriteNull(&tvRef);
  if (mode == MOpMode::Warn) {
    raise_notice("Cannot access property on non-object");
  }
  return &tvRef;
}

template <class F>
inline void promoteToStdClass(TypedValue* base, bool warn, F fun) {
  if (!RuntimeOption::EvalPromoteEmptyObject) {
    // note that the whole point here is to guarantee that the property
    // never auto updates to a stdclass - so we must do this before
    // calling promote, and we don't want the try catch below around
    // this call.
    if (RuntimeOption::PHP7_EngineExceptions) {
      SystemLib::throwErrorObject(Strings::SET_PROP_NON_OBJECT);
    } else {
      SystemLib::throwExceptionObject(Strings::SET_PROP_NON_OBJECT);
    }
    not_reached();
  }

  Object obj { ObjectData::newInstance(SystemLib::s_stdclassClass) };
  if (base->m_type == KindOfString) {
    decRefStr(base->m_data.pstr);
  } else {
    assert(!isRefcountedType(base->m_type));
  }
  base->m_type = KindOfObject;
  base->m_data.pobj = obj.get();

  if (warn) {
    // Behavior here is observable.
    // In PHP 5.6, raise_warning is called before updating base, so
    // the error_handler sees the original base; but if an exception
    // is thrown from the error handler, any catch block will see the
    // updated base.
    // In PHP 7+, raise_warning is called after updating base, but before
    // doing the work of fun, and again, if an exception is thrown, fun
    // still gets called before reaching the catch block.
    // We'll match PHP7, because we have no way of ensuring that base survives
    // across a call to the error_handler: eg $a[0][0][0]->foo = 0; if $a
    // started out null, and the error handler resets it to null, base is
    // left dangling.
    // Note that this means that the error handler can overwrite the object
    // so there is no guarantee that we have an object on return from
    // promoteToStdClass.
    try {
      raise_warning(Strings::CREATING_DEFAULT_OBJECT);
    } catch (const Object&) {
      fun(obj.get());
      throw;
    }
  }

  fun(obj.get());
}

template<MOpMode mode>
TypedValue* propPreStdclass(TypedValue& tvRef, TypedValue* base) {
  if (mode != MOpMode::Define) {
    return propPreNull<mode>(tvRef);
  }

  promoteToStdClass(base, RuntimeOption::EnableHipHopSyntax,
                    [] (ObjectData*) {});
  if (UNLIKELY(base->m_type != KindOfObject)) {
    // See the comments above. Although promoteToStdClass will have
    // either thrown an exception, or promoted base to an object, an
    // installed error handler might have caused it to be overwritten
    tvWriteNull(&tvRef);
    return &tvRef;
  }

  return base;
}

template<MOpMode mode>
TypedValue* propPre(TypedValue& tvRef, TypedValue* base) {
  base = tvToCell(base);
  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return propPreStdclass<mode>(tvRef, base);

    case KindOfBoolean:
      if (base->m_data.num) {
        return propPreNull<mode>(tvRef);
      }
      return propPreStdclass<mode>(tvRef, base);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return propPreNull<mode>(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        return propPreNull<mode>(tvRef);
      }
      return propPreStdclass<mode>(tvRef, base);

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      return propPreNull<mode>(tvRef);

    case KindOfObject:
      return base;

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

inline TypedValue* nullSafeProp(TypedValue& tvRef,
                                Class* ctx,
                                TypedValue* base,
                                StringData* key) {
  base = tvToCell(base);
  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      tvWriteNull(&tvRef);
      return &tvRef;
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      tvWriteNull(&tvRef);
      raise_notice("Cannot access property on non-object");
      return &tvRef;
    case KindOfObject:
      return base->m_data.pobj->prop(&tvRef, ctx, key);
    case KindOfRef:
    case KindOfClass:
      always_assert(false);
  }
  not_reached();
}

/*
 * Generic property access (PropX and PropDX end up here).
 *
 * Returns a pointer to a number of possible places, but does not unbox it.
 * (The returned pointer is never pointing into a RefData.)
 */
template<MOpMode mode, KeyType keyType = KeyType::Any>
inline TypedValue* PropObj(TypedValue& tvRef, const Class* ctx,
                           ObjectData* instance, key_type<keyType> key) {
  auto constexpr warn   = mode == MOpMode::Warn;
  auto constexpr define = mode == MOpMode::Define;
  auto constexpr unset  = mode == MOpMode::Unset;

  auto keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Get property.
  if (warn) {
    return instance->propW(&tvRef, ctx, keySD);
  }

  if (define || unset) return instance->propD(&tvRef, ctx, keySD);
  return instance->prop(&tvRef, ctx, keySD);
}

template<MOpMode mode, KeyType keyType = KeyType::Any>
inline TypedValue* Prop(TypedValue& tvRef,
                        const Class* ctx,
                        TypedValue* base,
                        key_type<keyType> key) {
  auto result = propPre<mode>(tvRef, base);
  if (result->m_type == KindOfNull) {
    return result;
  }
  assertx(result->m_type == KindOfObject);
  auto instance = instanceFromTv(result);
  return PropObj<mode,keyType>(tvRef, ctx, instance, key);
}

template <bool useEmpty, KeyType kt>
inline bool IssetEmptyPropObj(Class* ctx, ObjectData* instance,
                              key_type<kt> key) {
  auto keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<kt>(keySD); };

  return useEmpty ?
    instance->propEmpty(ctx, keySD) :
    instance->propIsset(ctx, keySD);
}

template <bool useEmpty, KeyType kt = KeyType::Any>
bool IssetEmptyProp(Class* ctx, TypedValue* base, key_type<kt> key) {
  base = tvToCell(base);
  if (LIKELY(base->m_type == KindOfObject)) {
    return IssetEmptyPropObj<useEmpty, kt>(ctx, instanceFromTv(base), key);
  }
  return useEmpty;
}

template <bool setResult>
inline void SetPropNull(Cell* val) {
  raise_warning("Cannot access property on non-object");
  if (setResult) {
    tvRefcountedDecRef(val);
    tvWriteNull(val);
  } else {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
}

inline void SetPropStdclass(TypedValue* base, TypedValue key, Cell* val) {
  promoteToStdClass(
    base,
    true,
    [&] (ObjectData* obj) {
      auto const keySD = prepareKey(key);
      SCOPE_EXIT { decRefStr(keySD); };
      obj->setProp(nullptr, keySD, (TypedValue*)val);
    });
}

template <KeyType keyType>
inline void SetPropObj(Class* ctx, ObjectData* instance,
                       key_type<keyType> key, Cell* val) {
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Set property.
  instance->setProp(ctx, keySD, val);
}

// $base->$key = $val
template <bool setResult, KeyType keyType = KeyType::Any>
inline void SetProp(Class* ctx, TypedValue* base, key_type<keyType> key,
                    Cell* val) {
  base = tvToCell(base);
  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return SetPropStdclass(base, initScratchKey(key), val);

    case KindOfBoolean:
      if (base->m_data.num) {
        return SetPropNull<setResult>(val);
      }
      return SetPropStdclass(base, initScratchKey(key), val);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
      return SetPropNull<setResult>(val);

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        return SetPropNull<setResult>(val);
      }
      return SetPropStdclass(base, initScratchKey(key), val);

    case KindOfObject:
      return SetPropObj<keyType>(ctx, base->m_data.pobj, key, val);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

inline TypedValue* SetOpPropNull(TypedValue& tvRef) {
  raise_warning("Attempt to assign property of non-object");
  tvWriteNull(&tvRef);
  return &tvRef;
}

inline TypedValue* SetOpPropStdclass(TypedValue& tvRef, SetOpOp op,
                                     TypedValue* base, TypedValue key,
                                     Cell* rhs) {
  promoteToStdClass(
    base,
    true,
    [&] (ObjectData* obj) {
      StringData* keySD = prepareKey(key);
      SCOPE_EXIT { decRefStr(keySD); };
      tvWriteNull(&tvRef);
      setopBody(tvToCell(&tvRef), op, rhs);
      obj->setProp(nullptr, keySD, &tvRef);
    });

  return &tvRef;
}

inline TypedValue* SetOpPropObj(TypedValue& tvRef, Class* ctx,
                                SetOpOp op, ObjectData* instance,
                                TypedValue key, Cell* rhs) {
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  TypedValue* result = instance->setOpProp(tvRef, ctx, op, keySD, rhs);
  return result;
}

// $base->$key <op>= $rhs
inline TypedValue* SetOpProp(TypedValue& tvRef,
                             Class* ctx, SetOpOp op,
                             TypedValue* base, TypedValue key,
                             Cell* rhs) {
  base = tvToCell(base);
  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpPropStdclass(tvRef, op, base, key, rhs);

    case KindOfBoolean:
      if (base->m_data.num) {
        return SetOpPropNull(tvRef);
      }
      return SetOpPropStdclass(tvRef, op, base, key, rhs);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
      return SetOpPropNull(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        return SetOpPropNull(tvRef);
      }
      return SetOpPropStdclass(tvRef, op, base, key, rhs);

    case KindOfObject:
      return SetOpPropObj(tvRef, ctx, op, instanceFromTv(base), key, rhs);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

inline Cell IncDecPropNull() {
  raise_warning("Attempt to increment/decrement property of non-object");
  return make_tv<KindOfNull>();
}

inline Cell IncDecPropStdclass(IncDecOp op, TypedValue* base,
                               TypedValue key) {
  Cell dest;
  promoteToStdClass(
    base,
    true,
    [&] (ObjectData* obj) {
      StringData* keySD = prepareKey(key);
      SCOPE_EXIT { decRefStr(keySD); };
      TypedValue tv;
      tvWriteNull(&tv);
      dest = IncDecBody(op, &tv);
      obj->setProp(nullptr, keySD, &dest);
      assert(!isRefcountedType(tv.m_type));
    });

  return dest;
}

inline Cell IncDecPropObj(Class* ctx,
                          IncDecOp op,
                          ObjectData* base,
                          TypedValue key) {
  auto keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  return base->incDecProp(ctx, op, keySD);
}

inline Cell IncDecProp(
  Class* ctx,
  IncDecOp op,
  TypedValue* base,
  TypedValue key
) {
  base = tvToCell(base);
  switch (base->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecPropStdclass(op, base, key);

    case KindOfBoolean:
      if (base->m_data.num) {
        return IncDecPropNull();
      }
      return IncDecPropStdclass(op, base, key);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
      return IncDecPropNull();

    case KindOfPersistentString:
    case KindOfString:
      if (base->m_data.pstr->size() != 0) {
        return IncDecPropNull();
      }
      return IncDecPropStdclass(op, base, key);

    case KindOfObject:
      return IncDecPropObj(ctx, op, instanceFromTv(base), key);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  unknownBaseType(base);
}

inline void UnsetPropObj(Class* ctx, ObjectData* instance, TypedValue key) {
  // Prepare key.
  auto keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  // Unset property.
  instance->unsetProp(ctx, keySD);
}

inline void UnsetProp(Class* ctx, TypedValue* base, TypedValue key) {
  // Validate base.
  base = tvToCell(base);
  if (LIKELY(base->m_type == KindOfObject)) {
    UnsetPropObj(ctx, instanceFromTv(base), key);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_VM_MEMBER_OPERATIONS_H_
