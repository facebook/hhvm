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

#include <type_traits>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/system/systemlib.h"

#include <folly/tracing/StaticTracepoint.h>

namespace HPHP {

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

  ~InvalidSetMException() noexcept override {}

  const TypedValue& tv() const { return m_tv; };

 private:
  /* m_tv will contain a TypedValue with a reference destined for the
   * VM eval stack. */
  req::root<TypedValue> m_tv;
};

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

inline void failOnNonCollectionObjArrayAccess(ObjectData* obj) {
  if (UNLIKELY(!obj->isCollection())) {
    raise_error("Cannot use array access on an object");
  }
}

inline ObjectData* instanceFromTv(tv_rval tv) {
  assertx(tvIsObject(tv));
  return val(tv).pobj;
}

[[noreturn]] void throw_cannot_use_newelem_for_lval_read_col();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read(const ArrayData*);
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_clsmeth();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_record();
[[noreturn]] void throw_cannot_unset_for_clsmeth();

[[noreturn]] void unknownBaseType(DataType);

namespace detail {

inline void raiseFalseyPromotion(tv_rval base) {
  if (tvIsNull(base)) {
    throwFalseyPromoteException("null");
  } else if (tvIsBool(base)) {
    throwFalseyPromoteException("false");
  } else if (tvIsString(base)) {
    throwFalseyPromoteException("empty string");
  }
  always_assert(false);
}

inline void raiseEmptyObject() {
  if (RuntimeOption::PHP7_EngineExceptions) {
    SystemLib::throwErrorObject(Strings::SET_PROP_NON_OBJECT);
  } else {
    SystemLib::throwExceptionObject(Strings::SET_PROP_NON_OBJECT);
  }
}

ALWAYS_INLINE void promoteClsMeth(tv_lval base) {
  raiseClsMethToVecWarningHelper();
  val(base).parr = clsMethToVecHelper(val(base).pclsmeth).detach();
  type(base) = val(base).parr->toDataType();
}

}

/**
 * Elem when base is Null
 */
inline TypedValue ElemEmptyish() {
  return make_tv<KindOfNull>();
}

/**
 * Elem when base is a Vec
 */
template<MOpMode mode>
inline TypedValue ElemVecPre(ArrayData* base, int64_t key) {
  return PackedArray::NvGetInt(base, key);
}

template<MOpMode mode>
inline TypedValue ElemVecPre(ArrayData* base, StringData* key) {
  if (mode == MOpMode::Warn || mode == MOpMode::InOut) {
    throwInvalidArrayKeyException(key, base);
  }
  return make_tv<KindOfUninit>();
}

template<MOpMode mode>
inline TypedValue ElemVecPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemVecPre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemVecPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline TypedValue ElemVec(ArrayData* base, key_type<keyType> key) {
  assertx(base->hasVanillaPackedLayout());
  auto const result = ElemVecPre<mode>(base, key);
  if (UNLIKELY(!result.is_init())) {
    if (mode != MOpMode::Warn && mode != MOpMode::InOut) return ElemEmptyish();
    throwOOBArrayKeyException(key, base);
  }
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Dict
 */
inline TypedValue ElemDictPre(ArrayData* base, int64_t key) {
  return MixedArray::NvGetInt(base, key);
}

inline TypedValue ElemDictPre(ArrayData* base, StringData* key) {
  return MixedArray::NvGetStr(base, key);
}

inline TypedValue ElemDictPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemDictPre(base, key.m_data.num);
  if (isStringType(dt)) return ElemDictPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

// This helper may also be used when we know we have a MixedArray in the JIT.
template<MOpMode mode, KeyType keyType>
inline TypedValue ElemDict(ArrayData* base, key_type<keyType> key) {
  assertx(base->hasVanillaMixedLayout());
  auto const result = ElemDictPre(base, key);
  if (UNLIKELY(!result.is_init())) {
    if (mode != MOpMode::Warn && mode != MOpMode::InOut) return ElemEmptyish();
    throwOOBArrayKeyException(key, base);
  }
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Keyset
 */
inline TypedValue ElemKeysetPre(ArrayData* base, int64_t key) {
  return SetArray::NvGetInt(base, key);
}

inline TypedValue ElemKeysetPre(ArrayData* base, StringData* key) {
  return SetArray::NvGetStr(base, key);
}

inline TypedValue ElemKeysetPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemKeysetPre(base, key.m_data.num);
  if (isStringType(dt)) return ElemKeysetPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline TypedValue ElemKeyset(ArrayData* base, key_type<keyType> key) {
  assertx(base->isKeysetKind());
  auto result = ElemKeysetPre(base, key);
  if (UNLIKELY(!result.is_init())) {
    if (mode != MOpMode::Warn && mode != MOpMode::InOut) return ElemEmptyish();
    throwOOBArrayKeyException(key, base);
  }
  assertx(isIntType(result.type()) || isStringType(result.type()));
  return result;
}

/**
 * Elem when base is a bespoke Hack array
 */
template<MOpMode mode>
inline TypedValue ElemBespokePre(ArrayData* base, int64_t key) {
  return BespokeArray::NvGetInt(base, key);
}

template<MOpMode mode>
inline TypedValue ElemBespokePre(ArrayData* base, StringData* key) {
  if ((mode == MOpMode::Warn || mode == MOpMode::InOut) &&
      (base->isVecType() || base->isVArray())) {
    throwInvalidArrayKeyException(key, base);
  }
  return BespokeArray::NvGetStr(base, key);
}

template<MOpMode mode>
inline TypedValue ElemBespokePre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemBespokePre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemBespokePre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline TypedValue ElemBespoke(ArrayData* base, key_type<keyType> key) {
  assertx(!base->isVanilla());
  auto const result = ElemBespokePre<mode>(base, key);
  if (UNLIKELY(!result.is_init())) {
    if (mode != MOpMode::Warn && mode != MOpMode::InOut) return ElemEmptyish();
    throwOOBArrayKeyException(key, base);
  }
  return result;
}

/**
 * Elem when base is a ClsMeth
 */
template<MOpMode mode>
inline TypedValue ElemClsMethPre(ClsMethDataRef base, int64_t key) {
  if (key == 0) {
    return make_tv<KindOfString>(
      const_cast<StringData*>(base->getCls()->name()));
  } else if (key == 1) {
    return make_tv<KindOfString>(
      const_cast<StringData*>(base->getFunc()->name()));
  }
  if (mode == MOpMode::Warn || mode == MOpMode::InOut) {
    SystemLib::throwOutOfBoundsExceptionObject(
      folly::sformat("Out of bounds clsmeth access: invalid index {}", key));
  }
  return make_tv<KindOfNull>();
}

template<MOpMode mode>
inline TypedValue ElemClsMethPre(ClsMethDataRef base, StringData* key) {
  if (mode == MOpMode::Warn || mode == MOpMode::InOut) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Invalid clsmeth key: expected a key of type int, string given");
  }
  return make_tv<KindOfNull>();
}

template<MOpMode mode>
inline TypedValue ElemClsMethPre(ClsMethDataRef base, TypedValue key) {
  if (LIKELY(isIntType(type(key)))) {
    return ElemClsMethPre<mode>(base, val(key).num);
  }
  if (mode == MOpMode::Warn || mode == MOpMode::InOut) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Invalid clsmeth key: expected a key of type int");
  }
  return make_tv<KindOfNull>();
}

template<MOpMode mode, KeyType keyType>
inline TypedValue ElemClsMeth(ClsMethDataRef base, key_type<keyType> key) {
  return ElemClsMethPre<mode>(base, key);
}

/**
 * Elem when base is an Int64, Double, or Resource.
 */
inline TypedValue ElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return ElemEmptyish();
}

/**
 * Elem when base is a Boolean
 */
inline TypedValue ElemBoolean(tv_rval base) {
  return base.val().num ? ElemScalar() : ElemEmptyish();
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
    return tvAsCVarRef(key).toInt64();
  }
}

/**
 * Elem when base is a String
 */
template<MOpMode mode, KeyType keyType>
inline TypedValue ElemString(const StringData* base, key_type<keyType> key) {
  auto const offset = ElemStringPre(key);

  if (size_t(offset) >= base->size()) {
    if (mode == MOpMode::Warn) {
      raise_notice("Uninitialized string offset: %" PRId64, offset);
    }
    return make_tv<KindOfPersistentString>(staticEmptyString());
  } else {
    auto const sd = base->getChar(offset);
    assertx(sd->isStatic());
    return make_tv<KindOfPersistentString>(sd);
  }
}

/**
 * Elem when base is an Object
 */
template<MOpMode mode, KeyType keyType>
inline TypedValue ElemObject(ObjectData* base, key_type<keyType> key) {
  failOnNonCollectionObjArrayAccess(base);

  auto scratch = initScratchKey(key);
  if (mode == MOpMode::Warn) return *collections::at(base, &scratch);
  auto const result = collections::get(base, &scratch);
  return result ? *result : make_tv<KindOfNull>();
}

/**
 * Elem when base is a Record
 */
// TODO (T41029813): Handle different modes
template <KeyType keyType>
inline TypedValue ElemRecord(RecordData* base, key_type<keyType> key) {
  auto const fieldName = tvCastToString(initScratchKey(key));
  auto const idx = base->record()->lookupField(fieldName.get());
  if (idx == kInvalidSlot) {
    raise_record_field_error(base->record()->name(), fieldName.get());
  }
  return *base->rvalAt(idx);
}

/**
 * $result = $base[$key];
 */
template<MOpMode mode, KeyType keyType>
NEVER_INLINE TypedValue ElemSlow(tv_rval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      return ElemEmptyish();
    case KindOfBoolean:
      return ElemBoolean(base);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfRClsMeth:
    case KindOfFunc:
      return ElemScalar();
    case KindOfClass:
      return ElemString<mode, keyType>(
        classToStringHelper(base.val().pclass), key
      );
    case KindOfLazyClass:
      return ElemString<mode, keyType>(
        lazyClassToStringHelper(base.val().plazyclass), key
      );
    case KindOfPersistentString:
    case KindOfString:
      return ElemString<mode, keyType>(base.val().pstr, key);

    // These types are handled in Elem.
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
      always_assert(false);

    case KindOfObject:
      return ElemObject<mode, keyType>(base.val().pobj, key);

    case KindOfClsMeth: {
      raiseClsMethToVecWarningHelper();
      return ElemClsMeth<mode, keyType>(base.val().pclsmeth, key);
    }

    case KindOfRecord:
      return ElemRecord<keyType>(base.val().prec, key);
  }
  unknownBaseType(type(base));
}

template<MOpMode mode, KeyType keyType = KeyType::Any>
inline TypedValue Elem(tv_rval base, key_type<keyType> key) {
  assertx(mode != MOpMode::Define && mode != MOpMode::Unset);
  assertx(tvIsPlausible(base.tv()));

  if (tvIsArrayLike(base)) {
    auto const ad = base.val().parr;
    if (!ad->isVanilla()) {
      return ElemBespoke<mode, keyType>(ad, key);
    } else if (ad->hasVanillaPackedLayout()) {
      return ElemVec<mode, keyType>(ad, key);
    } else if (ad->hasVanillaMixedLayout()) {
      return ElemDict<mode, keyType>(ad, key);
    } else {
      return ElemKeyset<mode, keyType>(ad, key);
    }
  }

  if (mode == MOpMode::InOut) throw_invalid_inout_base();
  return ElemSlow<mode, keyType>(base, key);
}

/**
 * ElemD when base is a bespoke array-like
 */
inline arr_lval ElemDBespokePre(tv_lval base, int64_t key) {
  return BespokeArray::LvalInt(base.val().parr, key);
}

inline arr_lval ElemDBespokePre(tv_lval base, StringData* key) {
  return BespokeArray::LvalStr(base.val().parr, key);
}

inline arr_lval ElemDBespokePre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemDBespokePre(base, key.m_data.num);
  if (isStringType(dt)) return ElemDBespokePre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <KeyType keyType>
inline tv_lval ElemDBespoke(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArrayLike(base));
  assertx(tvIsPlausible(*base));

  auto const oldArr = base.val().parr;
  assertx(!oldArr->isVanilla());
  auto const result = ElemDBespokePre(base, key);
  if (result.arr != oldArr) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = result.arr;
    decRefArr(oldArr);
  }
  assertx(tvIsPlausible(*base));
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Vec/VArray
 */
inline tv_lval ElemDVecPre(tv_lval base, int64_t key) {
  auto const oldArr = base.val().parr;
  auto const lval = PackedArray::LvalInt(oldArr, key);

  if (lval.arr != oldArr) {
    base.type() = dt_with_rc(base.type());
    base.val().parr = lval.arr;
    assertx(tvIsPlausible(base.tv()));
    decRefArr(oldArr);
  }
  return lval;
}

inline tv_lval ElemDVecPre(tv_lval base, StringData* key) {
  throwInvalidArrayKeyException(key, base.val().parr);
}

inline tv_lval ElemDVecPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) {
    return ElemDVecPre(base, key.m_data.num);
  } else if (isStringType(dt)) {
    return ElemDVecPre(base, key.m_data.pstr);
  }
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <KeyType keyType>
inline tv_lval ElemDVec(tv_lval base, key_type<keyType> key) {
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(base.tv()));
  auto const result = ElemDVecPre(base, key);
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(base.tv()));

  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Dict/DArr
 */
inline tv_lval ElemDDictPre(tv_lval base, int64_t key) {
  auto const oldArr = base.val().parr;
  auto const lval = MixedArray::LvalSilentInt(oldArr, key);

  if (UNLIKELY(!lval)) {
    assertx(oldArr == lval.arr);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (lval.arr != oldArr) {
    base.type() = dt_with_rc(base.type());
    base.val().parr = lval.arr;
    assertx(tvIsPlausible(base.tv()));
    decRefArr(oldArr);
  }

  return lval;
}

inline tv_lval ElemDDictPre(tv_lval base, StringData* key) {
  auto const oldArr = base.val().parr;
  auto const lval = MixedArray::LvalSilentStr(oldArr, key);

  if (UNLIKELY(!lval)) {
    assertx(oldArr == lval.arr);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (lval.arr != oldArr) {
    base.type() = dt_with_rc(base.type());
    base.val().parr = lval.arr;
    assertx(tvIsPlausible(base.tv()));
    decRefArr(oldArr);
  }

  return lval;
}

inline tv_lval ElemDDictPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt)) {
    return ElemDDictPre(base, key.m_data.num);
  } else if (isStringType(dt)) {
    return ElemDDictPre(base, key.m_data.pstr);
  }
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <KeyType keyType>
inline tv_lval ElemDDict(tv_lval base, key_type<keyType> key) {
  assertx(tvIsDictOrDArray(base));
  assertx(tvIsPlausible(base.tv()));
  auto result = ElemDDictPre(base, key);
  assertx(tvIsDictOrDArray(base));
  assertx(tvIsPlausible(base.tv()));

  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Keyset
 */
[[noreturn]]
inline tv_lval ElemDKeysetPre(tv_lval /*base*/, int64_t /*key*/) {
  throwInvalidKeysetOperation();
}

[[noreturn]]
inline tv_lval ElemDKeysetPre(tv_lval /*base*/, StringData* /*key*/) {
  throwInvalidKeysetOperation();
}

[[noreturn]]
inline tv_lval ElemDKeysetPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    ElemDKeysetPre(base, key.m_data.num);
  if (isStringType(dt)) ElemDKeysetPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <KeyType keyType>
[[noreturn]]
inline tv_lval ElemDKeyset(tv_lval base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(base.tv()));
  ElemDKeysetPre(base, key);
}

/**
 * ElemD when base is Null
 */
inline tv_lval ElemDEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
  not_reached();
}

/**
 * ElemD when base is an Int64, Double, Resource, Func, or Class.
 * We can use immutable_null_base here because setters on null will throw.
 */
inline tv_lval ElemDScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return const_cast<TypedValue*>(&immutable_null_base);
}

/**
 * ElemD when base is a Boolean
 */
inline tv_lval ElemDBoolean(tv_lval base) {
  return base.val().num ? ElemDScalar() : ElemDEmptyish(base);
}

/**
 * ElemD when base is a String
 */
inline tv_lval ElemDString(tv_lval base) {
  if (!base.val().pstr->size()) return ElemDEmptyish(base);
  raise_error("Operator not supported for strings");
  return tv_lval(nullptr);
}

/**
 * ElemD when base is a Record
 */
template <KeyType keyType>
inline tv_lval ElemDRecord(tv_lval base, key_type<keyType> key) {
  assertx(tvIsRecord(base));
  assertx(tvIsPlausible(base.tv()));
  auto const oldRecData = val(base).prec;
  if (oldRecData->cowCheck()) {
    val(base).prec = oldRecData->copyRecord();
    decRefRec(oldRecData);
  }
  auto const fieldName = tvCastToString(initScratchKey(key));
  auto const rec = val(base).prec->record();
  auto const idx = rec->lookupField(fieldName.get());
  if (idx == kInvalidSlot) {
    raise_record_field_error(rec->name(), fieldName.get());
  }
  return val(base).prec->lvalAt(idx);
}
/**
 * ElemD when base is an Object
 */
template<KeyType keyType>
inline tv_lval ElemDObject(tv_lval base, key_type<keyType> key) {
  auto obj = base.val().pobj;
  failOnNonCollectionObjArrayAccess(obj);
  auto scratchKey = initScratchKey(key);
  return collections::atLval(obj, &scratchKey);
}

/*
 * Intermediate elem operation for defining member instructions.
 */
template<KeyType keyType = KeyType::Any>
tv_lval ElemD(tv_lval base, key_type<keyType> key) {
  assertx(tvIsPlausible(base.tv()));

  // ElemD helpers hand out lvals to immutable_null_base in cases where we know
  // it won't be updated. Confirm that we never do an illegal update on it.
  assertx(type(immutable_null_base) == KindOfNull);

  if (tvIsArrayLike(base) && !base.val().parr->isVanilla()) {
    return ElemDBespoke<keyType>(base, key);
  }

  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      return ElemDEmptyish(base);
    case KindOfBoolean:
      return ElemDBoolean(base);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      return ElemDScalar();
    case KindOfPersistentString:
    case KindOfString:
      return ElemDString(base);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemDKeyset<keyType>(base, key);
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentDArray:
    case KindOfDArray:
      return ElemDDict<keyType>(base, key);
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentVArray:
    case KindOfVArray:
      return ElemDVec<keyType>(base, key);
    case KindOfObject:
      return ElemDObject<keyType>(base, key);
    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      return ElemDVec<keyType>(base, key);
    case KindOfRecord:
      return ElemDRecord<keyType>(base, key);
  }
  unknownBaseType(type(base));
}

/**
 * ElemU when base is Null. We can use immutable_null_base here because
 * unsets on null will succeed with no further updates.
 */
inline tv_lval ElemUEmptyish() {
  return const_cast<TypedValue*>(&immutable_null_base);
}

/**
 * ElemU when base is a bespoke array-like
 */
inline arr_lval ElemUBespokePre(tv_lval base, int64_t key) {
  if (tvIsKeyset(base)) throwInvalidKeysetOperation();
  auto const ad = base.val().parr;
  if (!BespokeArray::ExistsInt(ad, key)) return {ad, ElemUEmptyish()};
  return BespokeArray::LvalInt(ad, key);
}

inline arr_lval ElemUBespokePre(tv_lval base, StringData* key) {
  if (tvIsKeyset(base)) throwInvalidKeysetOperation();
  auto const ad = base.val().parr;
  if (!BespokeArray::ExistsStr(ad, key)) return {ad, ElemUEmptyish()};
  return BespokeArray::LvalStr(ad, key);
}

inline arr_lval ElemUBespokePre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemUBespokePre(base, key.m_data.num);
  if (isStringType(dt)) return ElemUBespokePre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <KeyType keyType>
inline tv_lval ElemUBespoke(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArrayLike(base));
  assertx(tvIsPlausible(*base));

  auto const oldArr = base.val().parr;
  assertx(!oldArr->isVanilla());
  auto const result = ElemUBespokePre(base, key);
  if (result.arr != oldArr) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = result.arr;
    decRefArr(oldArr);
  }
  assertx(tvIsPlausible(*base));
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * ElemU when base is a Vec
 */
inline tv_lval ElemUVecPre(tv_lval base, int64_t key) {
  auto const oldArr = val(base).parr;
  if (UNLIKELY(!PackedArray::ExistsInt(oldArr, key))) {
    return ElemUEmptyish();
  }

  auto const newArr = [&]{
    if (!oldArr->cowCheck()) return oldArr;
    decRefArr(oldArr);
    auto const newArr = PackedArray::Copy(oldArr);
    type(base) = dt_with_rc(type(base));
    val(base).parr = newArr;
    assertx(tvIsPlausible(*base));
    return newArr;
  }();

  return PackedArray::LvalUncheckedInt(newArr, key);
}

inline tv_lval ElemUVecPre(tv_lval /*base*/, StringData* /*key*/) {
  return ElemUEmptyish();
}

inline tv_lval ElemUVecPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return ElemUVecPre(base, key.m_data.num);
  if (isStringType(dt))      return ElemUVecPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, val(base).parr);
}

template <KeyType keyType>
inline tv_lval ElemUVec(tv_lval base, key_type<keyType> key) {
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(*base));
  auto result = ElemUVecPre(base, key);
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(*base));
  assertx(type(result) != KindOfUninit);
  return result;
}

/**
 * ElemU when base is a Dict/DArray
 */
inline tv_lval ElemUDictPre(tv_lval base, int64_t key) {
  ArrayData* oldArr = val(base).parr;
  auto const lval = MixedArray::LvalSilentInt(oldArr, key);

  if (UNLIKELY(!lval)) {
    return ElemUEmptyish();
  }
  if (lval.arr != oldArr) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = lval.arr;
    assertx(tvIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
}

inline tv_lval ElemUDictPre(tv_lval base, StringData* key) {
  ArrayData* oldArr = val(base).parr;
  auto const lval = MixedArray::LvalSilentStr(oldArr, key);

  if (UNLIKELY(!lval)) {
    return ElemUEmptyish();
  }
  if (lval.arr != oldArr) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = lval.arr;
    assertx(tvIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
}

inline tv_lval ElemUDictPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemUDictPre(base, key.m_data.num);
  if (isStringType(dt)) return ElemUDictPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, val(base).parr);
}

template <KeyType keyType>
inline tv_lval ElemUDict(tv_lval base, key_type<keyType> key) {
  assertx(tvIsDictOrDArray(base));
  assertx(tvIsPlausible(*base));
  auto result = ElemUDictPre(base, key);
  assertx(tvIsDictOrDArray(base));
  assertx(tvIsPlausible(*base));
  assertx(type(result) != KindOfUninit);
  return result;
}

/**
 * ElemU when base is a Keyset
 */
[[noreturn]] inline tv_lval
ElemUKeysetPre(tv_lval /*base*/, int64_t /*key*/) {
  throwInvalidKeysetOperation();
}

[[noreturn]] inline tv_lval
ElemUKeysetPre(tv_lval /*base*/, StringData* /*key*/) {
  throwInvalidKeysetOperation();
}

[[noreturn]]
inline tv_lval ElemUKeysetPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    ElemUKeysetPre(base, key.m_data.num);
  if (isStringType(dt)) ElemUKeysetPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, val(base).parr);
}

template <KeyType keyType>
[[noreturn]]
inline tv_lval ElemUKeyset(tv_lval base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  ElemUKeysetPre(base, key);
}

/**
 * ElemU when base is an Object
 */
template <KeyType keyType>
inline tv_lval ElemUObject(tv_lval base, key_type<keyType> key) {
  auto obj = val(base).pobj;
  failOnNonCollectionObjArrayAccess(obj);
  auto const scratchKey = initScratchKey(key);
  return collections::atLval(obj, &scratchKey);
}

/*
 * Intermediate Elem operation for an unsetting member instruction.
 */
template <KeyType keyType = KeyType::Any>
tv_lval ElemU(tv_lval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  // ElemU helpers hand out lvals to immutable_null_base in cases where we know
  // it won't be updated. Confirm that we never do an illegal update on it.
  assertx(type(immutable_null_base) == KindOfNull);

  if (tvIsArrayLike(base) && !base.val().parr->isVanilla()) {
    return ElemUBespoke<keyType>(base, key);
  }

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      // Unset on scalar base never modifies the base, but the const_cast is
      // necessary to placate the type system.
      return const_cast<TypedValue*>(&immutable_uninit_base);
    case KindOfClass:
    case KindOfLazyClass:
      raise_error(Strings::OP_NOT_SUPPORTED_CLASS);
      return nullptr;
    case KindOfRFunc:
      raise_error(Strings::RFUNC_NOT_SUPPORTED);
      return nullptr;
    case KindOfFunc:
      raise_error(Strings::OP_NOT_SUPPORTED_FUNC);
      return nullptr;
    case KindOfPersistentString:
    case KindOfString:
      raise_error(Strings::OP_NOT_SUPPORTED_STRING);
      return nullptr;
    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      return ElemUVec<keyType>(base, key);
    case KindOfRClsMeth:
      raise_error(Strings::RCLS_METH_NOT_SUPPORTED);
      return nullptr;
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemUKeyset<keyType>(base, key);
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentDArray:
    case KindOfDArray:
      return ElemUDict<keyType>(base, key);
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentVArray:
    case KindOfVArray:
      return ElemUVec<keyType>(base, key);
    case KindOfObject:
      return ElemUObject<keyType>(base, key);
    case KindOfRecord:
      raise_error(Strings::OP_NOT_SUPPORTED_RECORD);
  }
  unknownBaseType(type(base));
}

/**
 * NewElem when base is Null
 */
inline tv_lval NewElemEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
  not_reached();
}

/**
 * NewElem when base is an invalid type (number, boolean, string, etc.) and is
 * not falsey. We can use immutable_null_base here because updates will raise.
 */
inline tv_lval NewElemInvalid() {
  raise_warning("Cannot use a scalar value as an array");
  return const_cast<TypedValue*>(&immutable_uninit_base);
}

/**
 * NewElem when base is a Boolean
 */
inline tv_lval NewElemBoolean(tv_lval base) {
  return val(base).num ? NewElemInvalid() : NewElemEmptyish(base);
}

/**
 * NewElem when base is a String
 */
inline tv_lval NewElemString(tv_lval base) {
  return val(base).pstr->size() ? NewElemInvalid() : NewElemEmptyish(base);
}

/**
 * NewElem when base is an Object
 */
inline tv_lval NewElemObject(tv_lval base) {
  failOnNonCollectionObjArrayAccess(val(base).pobj);
  throw_cannot_use_newelem_for_lval_read_col();
  return nullptr;
}

/**
 * $result = ($base[] = ...);
 */
inline tv_lval NewElem(tv_lval base) {
  assertx(tvIsPlausible(base.tv()));

  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      return NewElemEmptyish(base);
    case KindOfBoolean:
      return NewElemBoolean(base);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      return NewElemInvalid();
    case KindOfPersistentString:
    case KindOfString:
      return NewElemString(base);
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
      throw_cannot_use_newelem_for_lval_read(val(base).parr);
    case KindOfObject:
      return NewElemObject(base);
    case KindOfClsMeth:
      throw_cannot_use_newelem_for_lval_read_clsmeth();
    case KindOfRecord:
      throw_cannot_use_newelem_for_lval_read_record();
  }
  unknownBaseType(type(base));
}

/**
 * SetElem when base is Null
 */
inline void SetElemEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
}

/**
 * SetElem when base is an Int64, Double, Resource, Func, or Class.
 */
template <bool setResult>
inline void SetElemScalar(TypedValue* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (!setResult) {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  tvDecRefGen(value);
  tvWriteNull(*value);
}

/**
 * SetElem when base is a Boolean
 */
template <bool setResult>
inline void SetElemBoolean(tv_lval base, TypedValue* value) {
  return val(base).num ? SetElemScalar<setResult>(value)
                       : SetElemEmptyish(base);
}

/**
 * Convert a key to integer for SetElem
 */
template<KeyType keyType>
inline int64_t castKeyToInt(key_type<keyType> key) {
  return tvToInt(initScratchKey(key));
}

template<>
inline int64_t castKeyToInt<KeyType::Int>(int64_t key) {
  return key;
}

/**
 * SetElem when base is a String
 */
template <bool setResult, KeyType keyType>
inline StringData* SetElemString(tv_lval base, key_type<keyType> key,
                                 TypedValue* value) {
  auto const baseLen = val(base).pstr->size();
  if (baseLen == 0) {
    SetElemEmptyish(base);
    return nullptr;
  }

  // Convert key to string offset.
  auto const x = castKeyToInt<keyType>(key);
  if (UNLIKELY(x < 0 || x >= StringData::MaxSize)) {
    // Can't use PRId64 here because of order of inclusion issues
    raise_warning("Illegal string offset: %lld", (long long)x);
    if (!setResult) {
      throw InvalidSetMException(make_tv<KindOfNull>());
    }
    tvDecRefGen(value);
    tvWriteNull(*value);
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
  char y;
  {
    StringData* valStr;
    if (LIKELY(isStringType(value->m_type))) {
      valStr = value->m_data.pstr;
      valStr->incRefCount();
    } else {
      valStr = tvCastToStringData(*value);
    }

    y = valStr->data()[0];
    decRefStr(valStr);
  }

  // Create and save the result.
  assertx(x >= 0); // x < 0 is handled above.
  auto const oldp = val(base).pstr;
  if (x < baseLen && !oldp->cowCheck()) {
    // Modify base in place.  This is safe because the LHS owns the
    // only reference.
    FOLLY_SDT(hhvm, hhvm_mut_modifychar, baseLen, x);
    auto const newp = oldp->modifyChar(x, y);
    if (UNLIKELY(newp != oldp)) {
      // only way we can get here is due to a private (count==1) apc string.
      decRefStr(oldp);
      val(base).pstr = newp;
      type(base) = KindOfString;
    }
    // NB: if x < capacity, we could have appended in-place here.
  } else {
    FOLLY_SDT(hhvm, hhvm_cow_modifychar, baseLen, x);
    StringData* sd = StringData::Make(slen);
    char* s = sd->mutableData();
    memcpy(s, oldp->data(), baseLen);
    if (x > baseLen) {
      memset(&s[baseLen], ' ', slen - baseLen - 1);
    }
    s[x] = y;
    sd->setSize(slen);
    decRefStr(oldp);
    val(base).pstr = sd;
    type(base) = KindOfString;
  }

  return makeStaticString(y);
}

/**
 * SetElem when base is an Object
 */
template <KeyType keyType>
inline void SetElemObject(tv_lval base, key_type<keyType> key,
                          TypedValue* value) {
  auto obj = val(base).pobj;
  failOnNonCollectionObjArrayAccess(obj);
  auto const scratchKey = initScratchKey(key);
  collections::set(obj, &scratchKey, value);
}

/**
 * SetElem where base is a record
 */
template <KeyType keyType>
inline void SetElemRecord(tv_lval base, key_type<keyType> key,
                          TypedValue* value) {
  auto const fieldName = tvCastToString(initScratchKey(key));
  auto const oldRecData = val(base).prec;
  auto const rec = oldRecData->record();
  auto const idx = rec->lookupField(fieldName.get());
  if (idx == kInvalidSlot) {
    raise_record_field_error(rec->name(), fieldName.get());
  }
  auto const& field = rec->field(idx);
  auto const& tc = field.typeConstraint();
  if (tc.isCheckable()) {
    tc.verifyRecField(value, rec->name(), field.name());
  }
  if (oldRecData->cowCheck()) {
    val(base).prec = oldRecData->copyRecord();
    decRefRec(oldRecData);
  }
  auto const& tv = val(base).prec->lvalAt(idx);
  tvSet(*value, tv);
}

/*
 * arraySetUpdateBase is used by SetElem{Array,Vec,Dict} to do the necessary
 * bookkeeping after mutating an array.
 */
ALWAYS_INLINE
void arraySetUpdateBase(ArrayData* oldData, ArrayData* newData, tv_lval base) {
  if (newData == oldData) return;

  assertx(isArrayLikeType(type(base)));
  assertx(val(base).parr == oldData);
  type(base) = dt_with_rc(type(base));
  val(base).parr = newData;
  assertx(type(base) == newData->toDataType());
  assertx(tvIsPlausible(*base));

  decRefArr(oldData);
}

/**
 * SetElem when base is a Vec
 */
inline ArrayData* SetElemVecPre(ArrayData* a, int64_t key, TypedValue* value) {
  return PackedArray::SetInt(a, key, *value);
}

inline ArrayData*
SetElemVecPre(ArrayData* a, StringData* key, TypedValue* /*value*/) {
  throwInvalidArrayKeyException(key, a);
}

inline ArrayData*
SetElemVecPre(ArrayData* a, TypedValue key, TypedValue* value) {
  if (tvIsInt(key))    return SetElemVecPre(a, key.m_data.num, value);
  if (tvIsString(key)) return SetElemVecPre(a, key.m_data.pstr, value);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void SetElemVec(tv_lval base, key_type<keyType> key, TypedValue* value) {
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  auto const newData = SetElemVecPre(a, key, value);
  arraySetUpdateBase(a, newData, base);
}

/**
 * SetElem when base is a Dict
 */
inline ArrayData* SetElemDictPre(ArrayData* a, int64_t key, TypedValue* value) {
  return MixedArray::SetInt(a, key, *value);
}

inline ArrayData*
SetElemDictPre(ArrayData* a, StringData* key, TypedValue* value) {
  return MixedArray::SetStr(a, key, *value);
}

inline ArrayData*
SetElemDictPre(ArrayData* a, TypedValue key, TypedValue* value) {
  if (tvIsInt(key))    return SetElemDictPre(a, key.m_data.num, value);
  if (tvIsString(key)) return SetElemDictPre(a, key.m_data.pstr, value);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void SetElemDict(tv_lval base, key_type<keyType> key,
                        TypedValue* value) {
  assertx(tvIsDictOrDArray(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  auto const newData = SetElemDictPre(a, key, value);
  arraySetUpdateBase(a, newData, base);
}

/**
 * SetElem when base is a bespoke vec or dict
 */
inline ArrayData* SetElemBespokePre(
    ArrayData* a, int64_t key, TypedValue* value) {
  return BespokeArray::SetInt(a, key, *value);
}

inline ArrayData* SetElemBespokePre(
    ArrayData* a, StringData* key, TypedValue* value) {
  return BespokeArray::SetStr(a, key, *value);
}

inline ArrayData* SetElemBespokePre(
    ArrayData* a, TypedValue key, TypedValue* value) {
  if (tvIsInt(key))    return SetElemBespokePre(a, key.m_data.num, value);
  if (tvIsString(key)) return SetElemBespokePre(a, key.m_data.pstr, value);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void SetElemBespoke(
    tv_lval base, key_type<keyType> key, TypedValue* value) {
  assertx(tvIsArrayLike(base));
  assertx(tvIsPlausible(*base));

  auto const oldArr = base.val().parr;
  assertx(!oldArr->isVanilla());
  auto const result = SetElemBespokePre(oldArr, key, value);

  if (result != oldArr) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = result;
    decRefArr(oldArr);
  }
  assertx(tvIsPlausible(*base));
}

/**
 * SetElem() leaves the result in 'value', rather than returning it as in
 * SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
 * get around.
 */
template <bool setResult, KeyType keyType>
NEVER_INLINE
StringData* SetElemSlow(tv_lval base, key_type<keyType> key,
                        TypedValue* value) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      SetElemEmptyish(base);
      return nullptr;
    case KindOfBoolean:
      SetElemBoolean<setResult>(base, value);
      return nullptr;
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      SetElemScalar<setResult>(value);
      return nullptr;
    case KindOfPersistentString:
    case KindOfString:
      return SetElemString<setResult, keyType>(base, key, value);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    // Handled in SetElem
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
      always_assert(false);

    case KindOfObject:
      SetElemObject<keyType>(base, key, value);
      return nullptr;
    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      SetElemVec<keyType>(base, key, value);
      return nullptr;
    case KindOfRecord:
      SetElemRecord<keyType>(base, key, value);
      return nullptr;
  }
  unknownBaseType(type(base));
}

/**
 * Fast path for SetElem assuming base is an Array
 */
template <bool setResult, KeyType keyType = KeyType::Any>
inline StringData* SetElem(tv_lval base, key_type<keyType> key,
                           TypedValue* value) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsVecOrVArray(base))) {
    base.val().parr->isVanilla() ? SetElemVec<keyType>(base, key, value)
                                 : SetElemBespoke<keyType>(base, key, value);
    return nullptr;
  }
  if (LIKELY(tvIsDictOrDArray(base))) {
    base.val().parr->isVanilla() ? SetElemDict<keyType>(base, key, value)
                                 : SetElemBespoke<keyType>(base, key, value);
    return nullptr;
  }
  return SetElemSlow<setResult, keyType>(base, key, value);
}

template<bool reverse>
void SetRange(
  tv_lval base, int64_t offset, TypedValue src, int64_t count, int64_t size
);

/**
 * SetNewElem when base is Null
 */
inline void SetNewElemEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
}

/**
 * SetNewElem when base is Int64, Double, Resource, Func or Class
 */
template <bool setResult>
inline void SetNewElemScalar(TypedValue* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (!setResult) {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  tvDecRefGen(value);
  tvWriteNull(*value);
}

/**
 * SetNewElem when base is a Boolean
 */
template <bool setResult>
inline void SetNewElemBoolean(tv_lval base, TypedValue* value) {
  return val(base).num ? SetNewElemScalar<setResult>(value)
                       : SetNewElemEmptyish(base);
}

/**
 * SetNewElem when base is a String
 */
inline void SetNewElemString(tv_lval base) {
  if (!val(base).pstr->size()) return SetNewElemEmptyish(base);
  raise_error("[] operator not supported for strings");
}

/**
 * SetNewElem when base is a bespoke array-like
 */
inline void SetNewElemBespoke(tv_lval base, TypedValue* value) {
  assertx(tvIsArrayLike(base));
  assertx(tvIsPlausible(*base));
  auto const oldArr = base.val().parr;
  auto const result = BespokeArray::Append(oldArr, *value);
  if (result != oldArr) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = result;
    decRefArr(oldArr);
  }
  assertx(tvIsPlausible(*base));
}

/**
 * SetNewElem when base is a Vec
 */
inline void SetNewElemVec(tv_lval base, TypedValue* value) {
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto a2 = PackedArray::Append(a, *value);
  if (a2 != a) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Vec (moves value)
 */
inline void SetNewElemVecMove(tv_lval base, TypedValue* value) {
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto a2 = PackedArray::AppendMove(a, *value);
  if (a2 != a) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
  }
}


/**
 * SetNewElem when base is a Dict
 */
inline void SetNewElemDict(tv_lval base, TypedValue* value) {
  assertx(tvIsDictOrDArray(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto a2 = MixedArray::Append(a, *value);
  if (a2 != a) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Keyset
 */
inline void SetNewElemKeyset(tv_lval base, TypedValue* value) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto a2 = SetArray::Append(a, *value);
  if (a2 != a) {
    type(base) = KindOfKeyset;
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is an Object
 */
inline void SetNewElemObject(tv_lval base, TypedValue* value) {
  auto obj = val(base).pobj;
  failOnNonCollectionObjArrayAccess(obj);
  collections::append(obj, value);
}

/**
 * $base[] = ...
 */
template <bool setResult>
inline void SetNewElem(tv_lval base, TypedValue* value) {
  assertx(tvIsPlausible(*base));

  if (tvIsArrayLike(base) && !base.val().parr->isVanilla()) {
    return SetNewElemBespoke(base, value);
  }

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetNewElemEmptyish(base);
    case KindOfBoolean:
      return SetNewElemBoolean<setResult>(base, value);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      return SetNewElemScalar<setResult>(value);
    case KindOfPersistentString:
    case KindOfString:
      return SetNewElemString(base);
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
      return SetNewElemVec(base, value);
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentDict:
    case KindOfDict:
      return SetNewElemDict(base, value);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return SetNewElemKeyset(base, value);
    case KindOfObject:
      return SetNewElemObject(base, value);
    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      return SetNewElemVec(base, value);
    case KindOfRecord:
      raise_error(Strings::OP_NOT_SUPPORTED_RECORD);
  }
  unknownBaseType(type(base));
}

/**
 * SetOpElem when base is Null
 */
inline TypedValue SetOpElemEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
  not_reached();
}

/**
 * TypedValue when base is Int64, Double, Resource, Func, or Class
 */
inline TypedValue SetOpElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

/**
 * $result = ($base[$x] <op>= $y)
 */
inline TypedValue SetOpElem(SetOpOp op, tv_lval base,
                            TypedValue key, TypedValue* rhs) {
  assertx(tvIsPlausible(*base));

  if (tvIsArrayLike(base) && !base.val().parr->isVanilla()) {
    if (base.val().parr->isKeysetType()) throwInvalidKeysetOperation();
    auto const result = ElemDBespoke<KeyType::Any>(base, key);
    setopBody(result, op, rhs);
    return *result;
  }

  auto const handleVec = [&] {
    auto const result = ElemDVec<KeyType::Any>(base, key);
    setopBody(tvAssertPlausible(result), op, rhs);
    return *result;
  };

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpElemEmptyish(base);

    case KindOfBoolean:
      return val(base).num ? SetOpElemScalar() : SetOpElemEmptyish(base);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      return SetOpElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("Cannot use assign-op operators with overloaded "
          "objects nor string offsets");
      }
      return SetOpElemEmptyish(base);

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentDArray:
    case KindOfDArray: {
      auto const result = ElemDDict<KeyType::Any>(base, key);
      setopBody(tvAssertPlausible(result), op, rhs);
      return *result;
    }

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentVArray:
    case KindOfVArray:
      return handleVec();

    case KindOfObject: {
      auto obj = val(base).pobj;
      failOnNonCollectionObjArrayAccess(obj);
      auto const result = collections::atRw(obj, &key);
      setopBody(result, op, rhs);
      return *result;
    }

    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      return handleVec();

    case KindOfRecord: {
      auto const result = ElemDRecord<KeyType::Any>(base, key);
      setopBody(tvAssertPlausible(result), op, rhs);
      return *result;
    }
  }
  unknownBaseType(type(base));
}

inline TypedValue SetOpNewElemEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
  not_reached();
}
inline TypedValue SetOpNewElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}
inline TypedValue SetOpNewElem(SetOpOp op, tv_lval base, TypedValue* rhs) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpNewElemEmptyish(base);

    case KindOfBoolean:
      return val(base).num ? SetOpNewElemScalar() : SetOpNewElemEmptyish(base);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      return SetOpNewElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("[] operator not supported for strings");
      }
      return SetOpNewElemEmptyish(base);

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
      throw_cannot_use_newelem_for_lval_read(val(base).parr);

    case KindOfObject: {
      failOnNonCollectionObjArrayAccess(val(base).pobj);
      throw_cannot_use_newelem_for_lval_read_col();
    }

    case KindOfClsMeth:
      throw_cannot_use_newelem_for_lval_read_clsmeth();

    case KindOfRecord:
      throw_cannot_use_newelem_for_lval_read_record();
  }
  unknownBaseType(type(base));
}

NEVER_INLINE TypedValue incDecBodySlow(IncDecOp op, tv_lval fr);

inline TypedValue IncDecBody(IncDecOp op, tv_lval fr) {
  assertx(tvIsPlausible(*fr));

  if (UNLIKELY(!isIntType(type(fr)))) {
    return incDecBodySlow(op, fr);
  }

  // fast cases, assuming integers overflow to ints. Because int64_t overflow is
  // undefined behavior reinterpret_cast<uint64_t&> first
  switch (op) {
  case IncDecOp::PreInc:
    ++reinterpret_cast<uint64_t&>(val(fr).num);
    return *fr;
  case IncDecOp::PostInc: {
    auto const tmp = *fr;
    ++reinterpret_cast<uint64_t&>(val(fr).num);
    return tmp;
  }
  case IncDecOp::PreDec:
    --reinterpret_cast<uint64_t&>(val(fr).num);
    return *fr;
  case IncDecOp::PostDec: {
    auto const tmp = *fr;
    --reinterpret_cast<uint64_t&>(val(fr).num);
    return tmp;
  }
  default:
    return incDecBodySlow(op, fr);
  }
}

inline TypedValue IncDecElemEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
  not_reached();
}

inline TypedValue IncDecElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

inline TypedValue IncDecElem(IncDecOp op, tv_lval base, TypedValue key) {
  assertx(tvIsPlausible(*base));

  if (tvIsArrayLike(base) && !base.val().parr->isVanilla()) {
    auto const result = ElemDBespoke<KeyType::Any>(base, key);
    return IncDecBody(op, tvAssertPlausible(result));
  }

  auto const handleVec = [&] {
    auto const result = ElemDVec<KeyType::Any>(base, key);
    return IncDecBody(op, tvAssertPlausible(result));
  };

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecElemEmptyish(base);

    case KindOfBoolean:
      return val(base).num ? IncDecElemScalar() : IncDecElemEmptyish(base);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      return IncDecElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("Cannot increment/decrement overloaded objects "
          "nor string offsets");
      }
      return IncDecElemEmptyish(base);

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentDArray:
    case KindOfDArray: {
      auto const result = ElemDDict<KeyType::Any>(base, key);
      return IncDecBody(op, tvAssertPlausible(result));
    }

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentVArray:
    case KindOfVArray:
      return handleVec();

    case KindOfObject: {
      tv_lval result;
      auto localTvRef = make_tv<KindOfUninit>();
      auto obj = val(base).pobj;
      failOnNonCollectionObjArrayAccess(obj);
      result = collections::atRw(obj, &key);
      assertx(tvIsPlausible(*result));

      auto const dest = IncDecBody(op, result);
      tvDecRefGen(localTvRef);
      return dest;
    }

    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      return handleVec();

    case KindOfRecord: {
      auto result = ElemDRecord<KeyType::Any>(base, key);
      return IncDecBody(op, tvAssertPlausible(result));
    }
  }
  unknownBaseType(type(base));
}

inline TypedValue IncDecNewElemEmptyish(tv_lval base) {
  detail::raiseFalseyPromotion(base);
  not_reached();
}

inline TypedValue IncDecNewElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

inline TypedValue IncDecNewElem(IncDecOp op, tv_lval base) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecNewElemEmptyish(base);

    case KindOfBoolean:
      return val(base).num ? IncDecNewElemScalar()
                           : IncDecNewElemEmptyish(base);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfRClsMeth:
    case KindOfClass:
    case KindOfLazyClass:
      return IncDecNewElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("[] operator not supported for strings");
      }
      return IncDecNewElemEmptyish(base);

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
      throw_cannot_use_newelem_for_lval_read(val(base).parr);

    case KindOfObject: {
      failOnNonCollectionObjArrayAccess(val(base).pobj);
      throw_cannot_use_newelem_for_lval_read_col();
    }

    case KindOfClsMeth:
      throw_cannot_use_newelem_for_lval_read_clsmeth();

    case KindOfRecord:
      throw_cannot_use_newelem_for_lval_read_record();
  }
  unknownBaseType(type(base));
}

/**
 * UnsetElem when base is a Vec
 */

inline ArrayData* UnsetElemVecPre(ArrayData* a, int64_t key) {
  return PackedArray::RemoveInt(a, key);
}

inline ArrayData*
UnsetElemVecPre(ArrayData* a, StringData* /*key*/) {
  /* Never contains strings, so a no-op. */
  return a;
}

inline ArrayData* UnsetElemVecPre(ArrayData* a, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return UnsetElemVecPre(a, key.m_data.num);
  if (isStringType(dt))      return UnsetElemVecPre(a, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemVec(tv_lval base, key_type<keyType> key) {
  assertx(tvIsVecOrVArray(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemVecPre(a, key);

  if (a2 != a) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * UnsetElem when base is a Dict
 */

inline ArrayData* UnsetElemDictPre(ArrayData* a, int64_t key) {
  return MixedArray::RemoveInt(a, key);
}

inline ArrayData* UnsetElemDictPre(ArrayData* a, StringData* key) {
  return MixedArray::RemoveStr(a, key);
}

inline ArrayData* UnsetElemDictPre(ArrayData* a, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return UnsetElemDictPre(a, key.m_data.num);
  if (isStringType(dt)) return UnsetElemDictPre(a, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemDict(tv_lval base, key_type<keyType> key) {
  assertx(tvIsDictOrDArray(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemDictPre(a, key);

  if (a2 != a) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * UnsetElem when base is a Keyset
 */

inline ArrayData* UnsetElemKeysetPre(ArrayData* a, int64_t key) {
  return SetArray::RemoveInt(a, key);
}

inline ArrayData* UnsetElemKeysetPre(ArrayData* a, StringData* key) {
  return SetArray::RemoveStr(a, key);
}

inline ArrayData* UnsetElemKeysetPre(ArrayData* a, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return UnsetElemKeysetPre(a, key.m_data.num);
  if (isStringType(dt)) return UnsetElemKeysetPre(a, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemKeyset(tv_lval base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemKeysetPre(a, key);

  if (a2 != a) {
    type(base) = KindOfKeyset;
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * UnsetElem when base is a bespoke Hack array
 */
inline ArrayData* UnsetElemBespokePre(ArrayData* a, int64_t key) {
  return BespokeArray::RemoveInt(a, key);
}

inline ArrayData* UnsetElemBespokePre(ArrayData* a, StringData* key) {
  return BespokeArray::RemoveStr(a, key);
}

inline ArrayData* UnsetElemBespokePre(ArrayData* a, TypedValue key) {
  if (tvIsInt(key))    return UnsetElemBespokePre(a, key.m_data.num);
  if (tvIsString(key)) return UnsetElemBespokePre(a, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemBespoke(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArrayLike(base));
  assertx(tvIsPlausible(*base));

  auto const oldArr = base.val().parr;
  assertx(!oldArr->isVanilla());
  auto const result = UnsetElemBespokePre(oldArr, key);

  if (result != oldArr) {
    type(base) = dt_with_rc(type(base));
    val(base).parr = result;
    decRefArr(oldArr);
  }
  assertx(tvIsPlausible(*base));
}

/**
 * unset($base[$member])
 */
template <KeyType keyType>
NEVER_INLINE
void UnsetElemSlow(tv_lval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return; // Do nothing.

    case KindOfRFunc:
      raise_error("Cannot unset a reified function");
      return;
    case KindOfFunc:
      raise_error("Cannot unset a func");
      return;
    case KindOfRClsMeth:
      raise_error("Cannot unset a reified class method pointer");
      return;
    case KindOfClass:
    case KindOfLazyClass:
      raise_error("Cannot unset a class");
      return;

    case KindOfPersistentString:
    case KindOfString:
      raise_error(Strings::CANT_UNSET_STRING);
      return;

    // Handled in UnsetElem
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
      always_assert(false);

    case KindOfObject: {
      auto obj = val(base).pobj;
      failOnNonCollectionObjArrayAccess(obj);
      auto const& scratchKey = initScratchKey(key);
      collections::unset(obj, &scratchKey);
      return;
    }

    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      UnsetElemVec<keyType>(base, key);
      return;

    case KindOfRecord:
      raise_error("Cannot unset a record field");
  }
  unknownBaseType(type(base));
}

/**
 * Fast path for UnsetElem assuming base is an Array
 */
template <KeyType keyType = KeyType::Any>
inline void UnsetElem(tv_lval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  if (tvIsArrayLike(base)) {
    auto const ad = base.val().parr;
    if (!ad->isVanilla())       return UnsetElemBespoke<keyType>(base, key);
    if (tvIsVecOrVArray(base))  return UnsetElemVec<keyType>(base, key);
    if (tvIsDictOrDArray(base)) return UnsetElemDict<keyType>(base, key);
    return UnsetElemKeyset<keyType>(base, key);
  }
  return UnsetElemSlow<keyType>(base, key);
}

/**
 * IssetElem when base is an Object
 */
template<KeyType keyType>
bool IssetElemObj(ObjectData* instance, key_type<keyType> key) {
  failOnNonCollectionObjArrayAccess(instance);
  auto scratchKey = initScratchKey(key);
  return collections::isset(instance, &scratchKey);
}

/**
 * IssetElem when base is a String
 */
template<KeyType keyType>
bool IssetElemString(const StringData* sd, key_type<keyType> key) {
  auto scratchKey = initScratchKey(key);
  int64_t x;
  if (LIKELY(scratchKey.m_type == KindOfInt64)) {
    x = scratchKey.m_data.num;
  } else {
    TypedValue tv;
    tvDup(scratchKey, tv);
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
      return false;
    }
    x = tv.m_data.num;
  }
  return x >= 0 && x < sd->size();
}

/**
 * IssetElem when base is a Vec
 */
template<KeyType keyType>
bool IssetElemVec(ArrayData* a, key_type<keyType> key) {
  assertx(a->hasVanillaPackedLayout());
  auto const result = ElemVec<MOpMode::None, keyType>(a, key);
  return !tvIsNull(tvAssertPlausible(result));
}

/**
 * IssetElem when base is a Dict
 */
template<KeyType keyType>
bool IssetElemDict(ArrayData* a, key_type<keyType> key) {
  assertx(a->hasVanillaMixedLayout());
  auto const result = ElemDict<MOpMode::None, keyType>(a, key);
  return !tvIsNull(tvAssertPlausible(result));
}

/**
 * IssetElem when base is a Keyset
 */
template<KeyType keyType>
bool IssetElemKeyset(ArrayData* a, key_type<keyType> key) {
  assertx(a->isKeysetKind());
  auto const result = ElemKeyset<MOpMode::None, keyType>(a, key);
  return !tvIsNull(tvAssertPlausible(result));
}

/**
 * IssetElem when base is a bespoke Hack array
 */
template<KeyType keyType>
bool IssetElemBespoke(ArrayData* a, key_type<keyType> key) {
  auto const result = ElemBespoke<MOpMode::None, keyType>(a, key);
  return !tvIsNull(tvAssertPlausible(result));
}

/**
 * IssetElem when base is a ClsMeth
 */
template<KeyType keyType>
bool IssetElemClsMeth(ClsMethDataRef base, key_type<keyType> key) {
  const TypedValue result = ElemClsMethPre<MOpMode::None>(base, key);
  return !tvIsNull(tvAssertPlausible(result));
}

/**
 * IssetElem when base is a Record
 */
template<KeyType keyType>
bool IssetElemRecord(RecordData* base, key_type<keyType> key) {
  auto const result = ElemRecord<keyType>(base, key);
  return !tvIsNull(tvAssertPlausible(result));
}

/**
 * isset($base[$key])
 */
template <KeyType keyType>
NEVER_INLINE bool IssetElemSlow(tv_rval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfRClsMeth:
    case KindOfFunc:
      return false;

    case KindOfClass:
      return IssetElemString<keyType>(
        classToStringHelper(val(base).pclass), key
      );

    case KindOfLazyClass:
      return IssetElemString<keyType>(
        lazyClassToStringHelper(val(base).plazyclass), key
      );

    case KindOfPersistentString:
    case KindOfString:
      return IssetElemString<keyType>(val(base).pstr, key);

    // These types are handled in IssetElem.
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
      always_assert(false);

    case KindOfObject:
      return IssetElemObj<keyType>(val(base).pobj, key);

    case KindOfClsMeth: {
      raiseClsMethToVecWarningHelper();
      return IssetElemClsMeth<keyType>(val(base).pclsmeth, key);
    }

    case KindOfRecord:
      return IssetElemRecord<keyType>(val(base).prec, key);
  }
  unknownBaseType(type(base));
}

template <KeyType keyType = KeyType::Any>
bool IssetElem(tv_rval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  if (tvIsArrayLike(base)) {
    auto const ad = val(base).parr;
    if (!ad->isVanilla()) {
      return IssetElemBespoke<keyType>(ad, key);
    } else if (ad->hasVanillaPackedLayout()) {
      return IssetElemVec<keyType>(ad, key);
    } else if (ad->hasVanillaMixedLayout()) {
      return IssetElemDict<keyType>(ad, key);
    } else {
      return IssetElemKeyset<keyType>(ad, key);
    }
  }

  return IssetElemSlow<keyType>(base, key);
}

template<MOpMode mode>
inline tv_lval propPreNull(TypedValue& tvRef) {
  tvWriteNull(tvRef);
  if (mode == MOpMode::Warn) {
    raise_notice("Cannot access property on non-object");
  }
  return tv_lval(&tvRef);
}

template<MOpMode mode>
tv_lval propPreStdclass(TypedValue& tvRef) {
  if (mode != MOpMode::Define) return propPreNull<mode>(tvRef);
  detail::raiseEmptyObject();
  not_reached();
}

template<MOpMode mode>
tv_lval propPre(TypedValue& tvRef, tv_lval base) {
  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      return propPreStdclass<mode>(tvRef);

    case KindOfBoolean:
      return base.val().num ? propPreNull<mode>(tvRef)
                            : propPreStdclass<mode>(tvRef);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
      return propPreNull<mode>(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      return base.val().pstr->size() ? propPreNull<mode>(tvRef)
                                     : propPreStdclass<mode>(tvRef);

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
      return propPreNull<mode>(tvRef);

    case KindOfObject:
      return base;
  }
  unknownBaseType(type(base));
}

inline tv_lval nullSafeProp(TypedValue& tvRef,
                            Class* ctx,
                            tv_rval base,
                            StringData* key) {
  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      tvWriteNull(tvRef);
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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
      tvWriteNull(tvRef);
      raise_notice("Cannot access property on non-object");
      return &tvRef;
    case KindOfObject:
      return val(base).pobj->prop(&tvRef, ctx, key);
  }
  not_reached();
}

/*
 * Generic property access (PropX and PropDX end up here).
 *
 * Returns a pointer to a number of possible places.
 */
template<MOpMode mode, KeyType keyType = KeyType::Any>
inline tv_lval PropObj(TypedValue& tvRef, const Class* ctx,
                       ObjectData* instance, key_type<keyType> key) {
  auto keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Get property.
  if (mode == MOpMode::Define) {
    return instance->propD(&tvRef, ctx, keySD);
  }
  if (mode == MOpMode::None) {
    return instance->prop(&tvRef, ctx, keySD);
  }
  if (mode == MOpMode::Warn) {
    return instance->propW(&tvRef, ctx, keySD);
  }
  assertx(mode == MOpMode::Unset);
  return instance->propU(&tvRef, ctx, keySD);
}

template<MOpMode mode, KeyType keyType = KeyType::Any>
inline tv_lval Prop(TypedValue& tvRef, const Class* ctx,
                    tv_lval base, key_type<keyType> key) {
  auto const result = propPre<mode>(tvRef, base);
  if (result.type() == KindOfNull) return result;
  return PropObj<mode,keyType>(tvRef, ctx, instanceFromTv(result), key);
}

template <KeyType kt>
inline bool IssetPropObj(Class* ctx, ObjectData* instance, key_type<kt> key) {
  auto keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<kt>(keySD); };

  return instance->propIsset(ctx, keySD);
}

template <KeyType kt = KeyType::Any>
bool IssetProp(Class* ctx, tv_lval base, key_type<kt> key) {
  if (LIKELY(type(base) == KindOfObject)) {
    return IssetPropObj<kt>(ctx, instanceFromTv(base), key);
  }
  return false;
}

template <bool setResult>
inline void SetPropNull(TypedValue* val) {
  raise_warning("Cannot access property on non-object");
  if (setResult) {
    tvDecRefGen(val);
    tvWriteNull(*val);
  } else {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
}

template <KeyType keyType>
inline void SetPropObj(Class* ctx, ObjectData* instance,
                       key_type<keyType> key, TypedValue* val) {
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Set property.
  instance->setProp(ctx, keySD, *val);
}

// $base->$key = $val
template <bool setResult, KeyType keyType = KeyType::Any>
inline void SetProp(Class* ctx, tv_lval base, key_type<keyType> key,
                    TypedValue* val) {
  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return detail::raiseEmptyObject();

    case KindOfBoolean:
      return HPHP::val(base).num ? SetPropNull<setResult>(val)
                                 : detail::raiseEmptyObject();

    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
      return SetPropNull<setResult>(val);

    case KindOfPersistentString:
    case KindOfString:
      return HPHP::val(base).pstr->size() ? SetPropNull<setResult>(val)
                                          : detail::raiseEmptyObject();

    case KindOfObject:
      return SetPropObj<keyType>(ctx, HPHP::val(base).pobj, key, val);
  }
  unknownBaseType(type(base));
}

inline tv_lval SetOpPropNull(TypedValue& tvRef) {
  raise_warning("Attempt to assign property of non-object");
  tvWriteNull(tvRef);
  return &tvRef;
}

inline tv_lval SetOpPropObj(TypedValue& tvRef, Class* ctx,
                            SetOpOp op, ObjectData* instance,
                            TypedValue key, TypedValue* rhs) {
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  return instance->setOpProp(tvRef, ctx, op, keySD, rhs);
}

// $base->$key <op>= $rhs
inline tv_lval SetOpProp(TypedValue& tvRef,
                         Class* ctx, SetOpOp op,
                         tv_lval base, TypedValue key,
                         TypedValue* rhs) {
  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      detail::raiseEmptyObject();
      not_reached();

    case KindOfBoolean:
      if (val(base).num) return SetOpPropNull(tvRef);
      detail::raiseEmptyObject();
      not_reached();

    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
      return SetOpPropNull(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size()) return SetOpPropNull(tvRef);
      detail::raiseEmptyObject();
      not_reached();

    case KindOfObject:
      return SetOpPropObj(tvRef, ctx, op, instanceFromTv(base), key, rhs);
  }
  unknownBaseType(type(base));
}

inline TypedValue IncDecPropNull() {
  raise_warning("Attempt to increment/decrement property of non-object");
  return make_tv<KindOfNull>();
}

inline TypedValue IncDecPropObj(Class* ctx,
                          IncDecOp op,
                          ObjectData* base,
                          TypedValue key) {
  auto keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  return base->incDecProp(ctx, op, keySD);
}

inline TypedValue IncDecProp(
  Class* ctx,
  IncDecOp op,
  tv_lval base,
  TypedValue key
) {
  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      detail::raiseEmptyObject();
      not_reached();

    case KindOfBoolean:
      if (val(base).num) return IncDecPropNull();
      detail::raiseEmptyObject();
      not_reached();

    case KindOfInt64:
    case KindOfDouble:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRecord:
      return IncDecPropNull();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size()) return IncDecPropNull();
      detail::raiseEmptyObject();
      not_reached();

    case KindOfObject:
      return IncDecPropObj(ctx, op, instanceFromTv(base), key);
  }
  unknownBaseType(type(base));
}

inline void UnsetPropObj(Class* ctx, ObjectData* instance, TypedValue key) {
  // Prepare key.
  auto keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  // Unset property.
  instance->unsetProp(ctx, keySD);
}

inline void UnsetProp(Class* ctx, tv_lval base, TypedValue key) {
  // Validate base.
  if (LIKELY(type(base) == KindOfObject)) {
    UnsetPropObj(ctx, instanceFromTv(base), key);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
