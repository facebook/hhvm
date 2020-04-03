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

#ifndef incl_HPHP_VM_MEMBER_OPERATIONS_H_
#define incl_HPHP_VM_MEMBER_OPERATIONS_H_

#include <type_traits>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
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
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_vec();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_dict();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_keyset();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_clsmeth();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_record();
[[noreturn]] void throw_cannot_unset_for_clsmeth();

[[noreturn]] void unknownBaseType(DataType);

[[noreturn]] void throw_inout_undefined_index(TypedValue);
[[noreturn]] void throw_inout_undefined_index(int64_t i);
[[noreturn]] void throw_inout_undefined_index(const StringData* sd);

namespace detail {

ALWAYS_INLINE void checkPromotion(tv_rval base, const MInstrPropState* pState) {
  if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    assertx(pState != nullptr);
    auto const cls = pState->getClass();
    if (UNLIKELY(cls != nullptr)) {
      auto const slot = pState->getSlot();
      auto tv = make_array_like_tv(staticEmptyArray());
      if (pState->isStatic()) {
        assertx(slot < cls->numStaticProperties());
        auto const& sprop = cls->staticProperties()[slot];
        auto const& tc = sprop.typeConstraint;
        if (tc.isCheckable()) {
          tc.verifyStaticProperty(&tv, cls, sprop.cls, sprop.name);
        }
      } else {
        assertx(slot < cls->numDeclProperties());
        auto const& prop = cls->declProperties()[slot];
        auto const& tc = prop.typeConstraint;
        if (tc.isCheckable()) tc.verifyProperty(&tv, cls, prop.cls, prop.name);
      }
      // No coercion for array types.
      assertx(isArrayLikeType(tv.m_type) &&
              tv.m_data.parr == staticEmptyArray());
    }
  }

  if (tvIsNull(base)) {
    throwFalseyPromoteException("null");
  } else if (tvIsBool(base)) {
    throwFalseyPromoteException("false");
  } else if (tvIsString(base)) {
    throwFalseyPromoteException("empty string");
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
inline tv_rval ElemEmptyish() {
  return tv_rval { &immutable_null_base };
}

inline tv_rval ElemArrayPre(ArrayData* base, int64_t key) {
  return base->rval(key);
}

inline tv_rval ElemArrayPre(ArrayData* base, StringData* key) {
  return base->rval(key);
}

inline tv_rval ElemArrayPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemArrayPre(base, key.m_data.num);
  if (isStringType(dt)) return ElemArrayPre(base, key.m_data.pstr);

  // tvToKey will raise a warning for most remaining inputs, which the error
  // elevator may convert into an error (and thus halt the lookup).
  auto const tv = tvToKey(key, base);
  if (isIntType(type(tv)))    return ElemArrayPre(base, tv.m_data.num);
  if (isStringType(type(tv))) return ElemArrayPre(base, tv.m_data.pstr);
  return nullptr;
}

/**
 * Fast path for Elem assuming base is an Array.
 */
template<MOpMode mode, KeyType keyType>
inline tv_rval ElemArray(ArrayData* base, key_type<keyType> key) {
  assertx(base->isPHPArrayType());
  auto const result = ElemArrayPre(base, key);

  if (UNLIKELY(!result)) {
    if (mode == MOpMode::Warn) {
      auto const scratch = initScratchKey(key);
      throwArrayKeyException(tvAsCVarRef(&scratch).toString().get(), false);
    } else if (mode == MOpMode::InOut) {
      throw_inout_undefined_index(initScratchKey(key));
    }
    return ElemEmptyish();
  }

  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Vec
 */
template<MOpMode mode>
inline tv_rval ElemVecPre(ArrayData* base, int64_t key) {
  return PackedArray::NvGetIntVec(base, key);
}

template<MOpMode mode>
inline tv_rval ElemVecPre(ArrayData* base, StringData* key) {
  if (mode == MOpMode::Warn || mode == MOpMode::InOut) {
    throwInvalidArrayKeyException(key, base);
  }
  return tv_rval{};
}

template<MOpMode mode>
inline tv_rval ElemVecPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemVecPre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemVecPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline tv_rval ElemVec(ArrayData* base, key_type<keyType> key) {
  assertx(base->isVecArrayKind());
  auto const result = ElemVecPre<mode>(base, key);
  if (UNLIKELY(!result)) {
    if (mode != MOpMode::Warn && mode != MOpMode::InOut) return ElemEmptyish();
    throwOOBArrayKeyException(key, base);
  }
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Dict
 */
inline tv_rval ElemDictPre(ArrayData* base, int64_t key) {
  return MixedArray::NvGetIntDict(base, key);
}

inline tv_rval ElemDictPre(ArrayData* base, StringData* key) {
  return MixedArray::NvGetStrDict(base, key);
}

inline tv_rval ElemDictPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemDictPre(base, key.m_data.num);
  if (isStringType(dt)) return ElemDictPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline tv_rval ElemDict(ArrayData* base, key_type<keyType> key) {
  assertx(base->hasVanillaMixedLayout());
  static_assert(MixedArray::NvGetInt == MixedArray::NvGetIntDict, "");
  static_assert(MixedArray::NvGetStr == MixedArray::NvGetStrDict, "");
  auto const result = ElemDictPre(base, key);
  if (UNLIKELY(!result)) {
    if (mode != MOpMode::Warn && mode != MOpMode::InOut) return ElemEmptyish();
    assertx(IMPLIES(base->isDictType(), base->isDictKind()));
    if (base->isDictKind()) {
      throwOOBArrayKeyException(key, base);
    } else {
      throwArrayKeyException(tvCastToStringData(initScratchKey(key)), false);
    }
  }
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Keyset
 */
inline tv_rval ElemKeysetPre(ArrayData* base, int64_t key) {
  return SetArray::NvGetInt(base, key);
}

inline tv_rval ElemKeysetPre(ArrayData* base, StringData* key) {
  return SetArray::NvGetStr(base, key);
}

inline tv_rval ElemKeysetPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemKeysetPre(base, key.m_data.num);
  if (isStringType(dt)) return ElemKeysetPre(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline tv_rval ElemKeyset(ArrayData* base, key_type<keyType> key) {
  assertx(base->isKeysetKind());
  auto result = ElemKeysetPre(base, key);
  if (UNLIKELY(!result)) {
    if (mode != MOpMode::Warn && mode != MOpMode::InOut) return ElemEmptyish();
    throwOOBArrayKeyException(key, base);
  }
  assertx(isIntType(result.type()) || isStringType(result.type()));
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
inline tv_rval ElemClsMeth(
  TypedValue& tvRef, ClsMethDataRef base, key_type<keyType> key) {
  tvRef = ElemClsMethPre<mode>(base, key);
  return tv_rval{ &tvRef };
}

/**
 * Elem when base is an Int64, Double, or Resource.
 */
inline tv_rval ElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return ElemEmptyish();
}

/**
 * Elem when base is a Boolean
 */
inline tv_rval ElemBoolean(tv_rval base) {
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
inline tv_rval ElemString(TypedValue& tvRef,
                              const StringData* base,
                              key_type<keyType> key) {
  auto offset = ElemStringPre(key);

  if (offset < 0 || offset >= base->size()) {
    if (mode == MOpMode::Warn) {
      raise_notice("Uninitialized string offset: %" PRId64, offset);
    }
    tvRef = make_tv<KindOfPersistentString>(staticEmptyString());
  } else {
    tvRef = make_tv<KindOfPersistentString>(base->getChar(offset));
    assertx(tvRef.m_data.pstr->isStatic());
  }
  return tv_rval { &tvRef };
}

/**
 * Elem when base is an Object
 */
template<MOpMode mode, KeyType keyType>
inline tv_rval ElemObject(TypedValue& tvRef,
                          ObjectData* base,
                          key_type<keyType> key) {
  failOnNonCollectionObjArrayAccess(base);

  auto scratch = initScratchKey(key);
  if (mode == MOpMode::Warn) {
    return collections::at(base, &scratch);
  }
  auto res = collections::get(base, &scratch);
  if (!res) {
    res = &tvRef;
    tvWriteNull(res);
  }
  return res;
}

/**
 * Elem when base is a Record
 */
// TODO (T41029813): Handle different modes
template <KeyType keyType>
inline tv_rval ElemRecord(RecordData* base, key_type<keyType> key) {
  auto const fieldName = tvCastToString(initScratchKey(key));
  auto const idx = base->record()->lookupField(fieldName.get());
  if (idx == kInvalidSlot) {
    raise_record_field_error(base->record()->name(), fieldName.get());
  }
  return base->rvalAt(idx);
}

/**
 * $result = $base[$key];
 */
template<MOpMode mode, KeyType keyType>
NEVER_INLINE tv_rval ElemSlow(TypedValue& tvRef,
                              tv_rval base,
                              key_type<keyType> key) {
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
      return ElemScalar();
    case KindOfFunc:
      return ElemString<mode, keyType>(
        tvRef, funcToStringHelper(base.val().pfunc), key
      );
    case KindOfClass:
      return ElemString<mode, keyType>(
        tvRef, classToStringHelper(base.val().pclass), key
      );
    case KindOfPersistentString:
    case KindOfString:
      return ElemString<mode, keyType>(tvRef, base.val().pstr, key);
    case KindOfPersistentVec:
    case KindOfVec:
      return ElemVec<mode, keyType>(base.val().parr, key);
    case KindOfPersistentDict:
    case KindOfDict:
      return ElemDict<mode, keyType>(base.val().parr, key);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemKeyset<mode, keyType>(base.val().parr, key);
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemArray<mode, keyType>(base.val().parr, key);
    case KindOfObject:
      return ElemObject<mode, keyType>(tvRef, base.val().pobj, key);

    case KindOfClsMeth: {
      raiseClsMethToVecWarningHelper();
      return ElemClsMeth<mode, keyType>(tvRef, base.val().pclsmeth, key);
    }

    case KindOfRecord:
      return ElemRecord<keyType>(base.val().prec, key);
  }
  unknownBaseType(type(base));
}

template<MOpMode mode, KeyType keyType = KeyType::Any>
inline tv_rval Elem(TypedValue& tvRef,
                    tv_rval base,
                    key_type<keyType> key) {
  assertx(mode != MOpMode::Define && mode != MOpMode::Unset);
  assertx(tvIsPlausible(base.tv()));

  if (LIKELY(tvIsArray(base))) {
    return ElemArray<mode, keyType>(base.val().parr, key);
  }
  if (LIKELY(tvIsVec(base))) {
    return ElemVec<mode, keyType>(base.val().parr, key);
  }
  if (LIKELY(tvIsDict(base))) {
    return ElemDict<mode, keyType>(base.val().parr, key);
  }
  if (LIKELY(tvIsKeyset(base))) {
    return ElemKeyset<mode, keyType>(base.val().parr, key);
  }

  if (mode == MOpMode::InOut) throw_invalid_inout_base();

  return ElemSlow<mode, keyType>(tvRef, base, key);
}

template<MOpMode mode>
inline tv_lval ElemDArrayPre(tv_lval base, int64_t key, bool& defined) {
  auto oldArr = val(base).parr;

  defined = (mode != MOpMode::Warn) || oldArr->exists(key);
  auto const lval = oldArr->lval(key, oldArr->cowCheck());

  if (lval.arr != oldArr) {
    assertx(lval.arr->isPHPArrayType());
    type(base) = lval.arr->toDataType();
    val(base).parr = lval.arr;
    assertx(tvIsPlausible(*base));
    decRefArr(oldArr);
  }

  return lval;
}

template<MOpMode mode>
inline tv_lval ElemDArrayPre(tv_lval base, StringData* key,
                             bool& defined) {
  auto oldArr = val(base).parr;

  auto const lval = [&]{
    auto const cow = oldArr->cowCheck();
    defined = (mode != MOpMode::Warn) || oldArr->exists(key);
    return oldArr->lval(key, cow);
  }();

  if (lval.arr != oldArr) {
    assertx(lval.arr->isPHPArrayType());
    type(base) = lval.arr->toDataType();
    val(base).parr = lval.arr;
    assertx(tvIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
}

template<MOpMode mode>
inline tv_lval ElemDArrayPre(tv_lval base, TypedValue key,
                             bool& defined) {
  auto const dt = key.m_type;
  if (isIntType(dt)) {
    return ElemDArrayPre<mode>(
      base, key.m_data.num, defined
    );
  }
  if (isStringType(dt)) {
    return ElemDArrayPre<mode>(base, key.m_data.pstr, defined);
  }
  auto& arr = asArrRef(base);
  defined = (mode != MOpMode::Warn) || arr.exists(tvAsCVarRef(&key));
  return arr.lval(key);
}

/**
 * ElemD when base is an Array
 */
template<MOpMode mode, KeyType keyType>
inline tv_lval ElemDArray(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));

  bool defined;
  auto lval = ElemDArrayPre<mode>(base, key, defined);

  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  assertx(lval.type() != KindOfUninit);

  if (!defined) {
    auto scratchKey = initScratchKey(key);
    throwArrayKeyException(tvAsCVarRef(&scratchKey).toString().get(), false);
  }

  return lval;
}

/**
 * ElemD when base is a Vec
 */
template <bool copyProv>
inline tv_lval ElemDVecPre(tv_lval base, int64_t key) {
  ArrayData* oldArr = base.val().parr;

  auto const lval = PackedArray::LvalIntVec(oldArr, key, oldArr->cowCheck());
  if (lval.arr != oldArr) {
    base.type() = KindOfVec;
    base.val().parr = lval.arr;
    assertx(tvIsPlausible(base.tv()));
    decRefArr(oldArr);
  }
  return lval;
}

template <bool copyProv>
inline tv_lval ElemDVecPre(tv_lval base, StringData* key) {
  throwInvalidArrayKeyException(key, base.val().parr);
}

template <bool copyProv>
inline tv_lval ElemDVecPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) {
    return ElemDVecPre<copyProv>(base, key.m_data.num);
  } else if (isStringType(dt)) {
    return ElemDVecPre<copyProv>(base, key.m_data.pstr);
  }
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <KeyType keyType, bool copyProv>
inline tv_lval ElemDVec(tv_lval base, key_type<keyType> key) {
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(base.tv()));
  auto const result = ElemDVecPre<copyProv>(base, key);
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(base.tv()));

  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Dict
 */
template <bool copyProv>
inline tv_lval ElemDDictPre(tv_lval base, int64_t key) {
  ArrayData* oldArr = base.val().parr;
  auto const lval = MixedArray::LvalSilentInt(oldArr, key);

  if (UNLIKELY(!lval)) {
    assertx(oldArr == lval.arr);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (lval.arr != oldArr) {
    base.type() = KindOfDict;
    base.val().parr = lval.arr;
    assertx(tvIsPlausible(base.tv()));
    decRefArr(oldArr);
  }

  return lval;
}

template <bool copyProv>
inline tv_lval ElemDDictPre(tv_lval base, StringData* key) {
  ArrayData* oldArr = base.val().parr;
  auto const lval = MixedArray::LvalSilentStr(oldArr, key);

  if (UNLIKELY(!lval)) {
    assertx(oldArr == lval.arr);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (lval.arr != oldArr) {
    base.type() = KindOfDict;
    base.val().parr = lval.arr;
    assertx(tvIsPlausible(base.tv()));
    decRefArr(oldArr);
  }

  return lval;
}

template <bool copyProv>
inline tv_lval ElemDDictPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt)) {
    return ElemDDictPre<copyProv>(base, key.m_data.num);
  } else if (isStringType(dt)) {
    return ElemDDictPre<copyProv>(base, key.m_data.pstr);
  }
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <KeyType keyType, bool copyProv>
inline tv_lval ElemDDict(tv_lval base, key_type<keyType> key) {
  assertx(isDictType(base.type()));
  assertx(tvIsPlausible(base.tv()));
  auto result = ElemDDictPre<copyProv>(base, key);
  assertx(isDictType(base.type()));
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
template<MOpMode mode, KeyType keyType>
inline tv_lval ElemDEmptyish(tv_lval base,
                             key_type<keyType> key,
                             const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  auto scratchKey = initScratchKey(key);

  tvMove(make_array_like_tv(ArrayData::CreateDArray()), base);

  auto const result = asArrRef(base).lval(tvAsCVarRef(scratchKey));
  if (mode == MOpMode::Warn) {
    throwArrayKeyException(tvAsCVarRef(&scratchKey).toString().get(), false);
  }
  return result;
}

/**
 * ElemD when base is an Int64, Double, Resource, Func, or Class.
 */
inline tv_lval ElemDScalar(TypedValue& tvRef) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(tvRef);
  return tv_lval(&tvRef);
}

/**
 * ElemD when base is a Boolean
 */
template<MOpMode mode, KeyType keyType>
inline tv_lval ElemDBoolean(TypedValue& tvRef,
                            tv_lval base,
                            key_type<keyType> key,
                            const MInstrPropState* pState) {
  return base.val().num
    ? ElemDScalar(tvRef)
    : ElemDEmptyish<mode, keyType>(base, key, pState);
}

/**
 * ElemD when base is a String
 */
template<MOpMode mode, KeyType keyType>
inline tv_lval ElemDString(tv_lval base,
                           key_type<keyType> key,
                           const MInstrPropState* pState) {
  if (base.val().pstr->size() == 0) {
    return ElemDEmptyish<mode, keyType>(base, key, pState);
  }
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
template<MOpMode mode, KeyType keyType>
inline tv_lval ElemDObject(TypedValue& tvRef, tv_lval base,
                           key_type<keyType> key) {
  auto obj = base.val().pobj;
  failOnNonCollectionObjArrayAccess(obj);
  auto scratchKey = initScratchKey(key);
  return collections::atLval(obj, &scratchKey);
}

/*
 * Intermediate elem operation for defining member instructions.
 */
template<MOpMode mode, KeyType keyType = KeyType::Any, bool copyProv>
tv_lval ElemD(TypedValue& tvRef, tv_lval base,
              key_type<keyType> key, const MInstrPropState* pState) {
  assertx(mode == MOpMode::Define);
  assertx(tvIsPlausible(base.tv()));

  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      return ElemDEmptyish<mode, keyType>(base, key, pState);
    case KindOfBoolean:
      return ElemDBoolean<mode, keyType>(tvRef, base, key, pState);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return ElemDScalar(tvRef);
    case KindOfPersistentString:
    case KindOfString:
      return ElemDString<mode, keyType>(base, key, pState);
    case KindOfPersistentVec:
    case KindOfVec:
      return ElemDVec<keyType, copyProv>(base, key);
    case KindOfPersistentDict:
    case KindOfDict:
      return ElemDDict<keyType, copyProv>(base, key);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemDKeyset<keyType>(base, key);
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemDArray<mode, keyType>(base, key);
    case KindOfObject:
      return ElemDObject<mode, keyType>(tvRef, base, key);
    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      if (RO::EvalHackArrDVArrs) {
        return ElemDVec<keyType, copyProv>(base, key);
      } else {
        return ElemDArray<mode, keyType>(base, key);
      }
    case KindOfRecord:
      return ElemDRecord<keyType>(base, key);
  }
  unknownBaseType(type(base));
}

/**
 * ElemU when base is Null
 */
inline tv_lval ElemUEmptyish() {
  return const_cast<TypedValue*>(&immutable_null_base);
}

inline tv_lval ElemUArrayImpl(tv_lval base, int64_t key) {
  auto oldArr = val(base).parr;
  if (!oldArr->exists(key)) return ElemUEmptyish();
  auto const lval = oldArr->lval(key, oldArr->cowCheck());
  if (lval.arr != oldArr) {
    assertx(lval.arr->isPHPArrayType());
    type(base) = lval.arr->toDataType();
    val(base).parr = lval.arr;
    assertx(tvIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
}

inline tv_lval ElemUArrayImpl(tv_lval base, StringData* key) {
  auto arr = val(base).parr;
  if (!arr->exists(key)) return ElemUEmptyish();

  auto const lval = arr->lval(key, arr->cowCheck());
  if (lval.arr != arr) {
    assertx(lval.arr->isPHPArrayType());
    type(base) = lval.arr->toDataType();
    val(base).parr = lval.arr;
    assertx(tvIsPlausible(*base));
    decRefArr(arr);
  }
  return lval;
}

inline tv_lval ElemUArrayImpl(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt)) {
    return ElemUArrayImpl(base, key.m_data.num);
  }
  if (isStringType(dt)) {
    return ElemUArrayImpl(base, key.m_data.pstr);
  }
  auto& arr = asArrRef(base);
  if (!arr.exists(keyAsValue(key))) {
    return ElemUEmptyish();
  }
  return arr.lval(tvAsCVarRef(&key));
}

/**
 * ElemU when base is an Array
 */
template <KeyType keyType>
inline tv_lval ElemUArray(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  auto lval = ElemUArrayImpl(base, key);
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  assertx(lval.type() != KindOfUninit);
  return lval;
}

/**
 * ElemU when base is a Vec
 */
inline tv_lval ElemUVecPre(tv_lval base, int64_t key) {
  auto const oldArr = val(base).parr;
  if (UNLIKELY(!PackedArray::ExistsIntVec(oldArr, key))) {
    return ElemUEmptyish();
  }

  auto const newArr = [&]{
    if (!oldArr->cowCheck()) return oldArr;
    decRefArr(oldArr);
    auto const newArr = PackedArray::CopyVec(oldArr);
    tvCopy(make_tv<KindOfVec>(newArr), base);
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
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));
  auto result = ElemUVecPre(base, key);
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));
  assertx(type(result) != KindOfUninit);
  return result;
}

/**
 * ElemU when base is a Dict
 */
inline tv_lval ElemUDictPre(tv_lval base, int64_t key) {
  ArrayData* oldArr = val(base).parr;
  auto const lval = MixedArray::LvalSilentInt(oldArr, key);

  if (UNLIKELY(!lval)) {
    return ElemUEmptyish();
  }
  if (lval.arr != oldArr) {
    type(base) = KindOfDict;
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
    type(base) = KindOfDict;
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
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  auto result = ElemUDictPre(base, key);
  assertx(tvIsDict(base));
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
inline tv_lval ElemUObject(TypedValue& tvRef, tv_lval base,
                           key_type<keyType> key) {
  auto obj = val(base).pobj;
  failOnNonCollectionObjArrayAccess(obj);
  auto const scratchKey = initScratchKey(key);
  return collections::atLval(obj, &scratchKey);
}

/*
 * Intermediate Elem operation for an unsetting member instruction.
 */
template <KeyType keyType = KeyType::Any>
tv_lval ElemU(TypedValue& tvRef, tv_lval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

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
      raise_error(Strings::OP_NOT_SUPPORTED_CLASS);
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
      if (RO::EvalHackArrDVArrs) {
        return ElemUVec<keyType>(base, key);
      } else {
        return ElemUArray<keyType>(base, key);
      }
    case KindOfPersistentVec:
    case KindOfVec:
      return ElemUVec<keyType>(base, key);
    case KindOfPersistentDict:
    case KindOfDict:
      return ElemUDict<keyType>(base, key);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemUKeyset<keyType>(base, key);
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemUArray<keyType>(base, key);
    case KindOfObject:
      return ElemUObject<keyType>(tvRef, base, key);
    case KindOfRecord:
      raise_error(Strings::OP_NOT_SUPPORTED_RECORD);
  }
  unknownBaseType(type(base));
}

/**
 * NewElem when base is Null
 */
inline tv_lval NewElemEmptyish(tv_lval base, const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  tvMove(make_array_like_tv(ArrayData::CreateVArray()), base);
  throwMissingElementException("Lval");
  return nullptr;
}

/**
 * NewElem when base is not a valid type (a number, true boolean,
 * non-empty string, etc.)
 */
inline tv_lval NewElemInvalid(TypedValue& tvRef) {
  raise_warning("Cannot use a scalar value as an array");
  tvWriteNull(tvRef);
  return tv_lval(&tvRef);
}

/**
 * NewElem when base is a Boolean
 */
inline tv_lval NewElemBoolean(TypedValue& tvRef,
                              tv_lval base,
                              const MInstrPropState* pState) {
  return val(base).num
    ? NewElemInvalid(tvRef)
    : NewElemEmptyish(base, pState);
}

/**
 * NewElem when base is a String
 */
inline tv_lval NewElemString(TypedValue& tvRef,
                             tv_lval base,
                             const MInstrPropState* pState) {
  if (val(base).pstr->size() == 0) {
    return NewElemEmptyish(base, pState);
  }
  return NewElemInvalid(tvRef);
}

/**
 * NewElem when base is an Array
 */
inline tv_lval NewElemArray(tv_lval base) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  throwMissingElementException("Lval");
  return nullptr;
}

/**
 * NewElem when base is an Object
 */
inline tv_lval NewElemObject(TypedValue& tvRef, tv_lval base) {
  failOnNonCollectionObjArrayAccess(val(base).pobj);
  throw_cannot_use_newelem_for_lval_read_col();
  return nullptr;
}

/**
 * $result = ($base[] = ...);
 */
inline tv_lval NewElem(TypedValue& tvRef,
                       tv_lval base,
                       const MInstrPropState* pState) {
  assertx(tvIsPlausible(base.tv()));

  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      return NewElemEmptyish(base, pState);
    case KindOfBoolean:
      return NewElemBoolean(tvRef, base, pState);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return NewElemInvalid(tvRef);
    case KindOfPersistentString:
    case KindOfString:
      return NewElemString(tvRef, base, pState);
    case KindOfPersistentVec:
    case KindOfVec:
      throw_cannot_use_newelem_for_lval_read_vec();
    case KindOfPersistentDict:
    case KindOfDict:
      throw_cannot_use_newelem_for_lval_read_dict();
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throw_cannot_use_newelem_for_lval_read_keyset();
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return NewElemArray(base);
    case KindOfObject:
      return NewElemObject(tvRef, base);
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
template <KeyType keyType>
inline void SetElemEmptyish(tv_lval base, key_type<keyType> key,
                            TypedValue* value, const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  auto const& scratchKey = initScratchKey(key);
  tvMove(make_array_like_tv(ArrayData::CreateDArray()), base);
  asArrRef(base).set(tvAsCVarRef(&scratchKey), tvAsCVarRef(value));
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
template <bool setResult, KeyType keyType>
inline void SetElemBoolean(tv_lval base, key_type<keyType> key,
                           TypedValue* value, const MInstrPropState* pState) {
  if (val(base).num) {
    SetElemScalar<setResult>(value);
  } else {
    SetElemEmptyish<keyType>(base, key, value, pState);
  }
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
                                 TypedValue* value, const MInstrPropState* pState) {
  auto const baseLen = val(base).pstr->size();
  if (baseLen == 0) {
    SetElemEmptyish<keyType>(base, key, value, pState);
    if (!setResult) {
      tvIncRefGen(*value);
      throw InvalidSetMException(*value);
    }
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
template<DataType dt>
ALWAYS_INLINE
void arraySetUpdateBase(ArrayData* oldData, ArrayData* newData, tv_lval base) {
  if (newData == oldData) return;

  assertx(isArrayLikeType(type(base)));
  assertx(val(base).parr == oldData);
  type(base) = dt;
  val(base).parr = newData;
  assertx(dt == newData->toDataType());
  assertx(tvIsPlausible(*base));

  decRefArr(oldData);
}

/**
 * SetElem helper with Array base and Int64 key
 */
template<bool setResult>
inline ArrayData* SetElemArrayPre(ArrayData* a, int64_t key, TypedValue* value) {
  return a->set(key, *value);
}

/**
 * SetElem helper with Array base and String key
 */
template<bool setResult>
inline ArrayData* SetElemArrayPre(ArrayData* a, StringData* key, TypedValue* value) {
  assertx(a->isPHPArrayType());
  return a->set(key, *value);
}

template<bool setResult>
inline ArrayData* SetElemArrayPre(ArrayData* a, TypedValue key, TypedValue* value) {
  if (isStringType(key.m_type)) {
    return SetElemArrayPre<setResult>(a, key.m_data.pstr, value);
  }
  if (key.m_type == KindOfInt64) {
    return SetElemArrayPre<setResult>(
      a, key.m_data.num, value
    );
  }
  if (isFuncType(key.m_type)) {
    return SetElemArrayPre<setResult>(
      a, const_cast<StringData*>(funcToStringHelper(key.m_data.pfunc)), value
    );
  }
  if (isClassType(key.m_type)) {
    return SetElemArrayPre<setResult>(
      a, const_cast<StringData*>(classToStringHelper(key.m_data.pclass)), value
    );
  }
  if (checkHACArrayKeyCast()) {
    raiseHackArrCompatImplicitArrayKey(&key);
  }
  if (isNullType(key.m_type)) {
    return a->set(staticEmptyString(), *value);
  }
  if (!isArrayLikeType(key.m_type) && key.m_type != KindOfObject) {
    return SetElemArrayPre<setResult>(
      a, tvAsCVarRef(&key).toInt64(), value
    );
  }

  raise_warning("Invalid operand type was used: Invalid type used as key");

  // Assignment failed, so the result is null rather than the RHS.
  if (setResult) {
    tvDecRefGen(value);
    tvWriteNull(*value);
  } else {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  return a;
}

/**
 * SetElem when base is an Array
 */
template <bool setResult, KeyType keyType>
inline void SetElemArray(tv_lval base, key_type<keyType> key, TypedValue* value) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  auto* newData = SetElemArrayPre<setResult>(a, key, value);
  // NB: If 'a' was sitting inside a reference, it may have been released during
  // the set (and 'newData' will equal 'a'). We can only safely dereference
  // 'newData' if its not equal to 'a'.
  assertx(a == newData || newData->isPHPArrayType());

  if (UNLIKELY(RuntimeOption::EvalEmitDVArray)) {
    if (newData->toDataType() == KindOfDArray) {
      arraySetUpdateBase<KindOfDArray>(a, newData, base);
      return;
    } else if (newData->toDataType() == KindOfVArray) {
      arraySetUpdateBase<KindOfVArray>(a, newData, base);
      return;
    }
  }
  arraySetUpdateBase<KindOfArray>(a, newData, base);
}

/**
 * SetElem when base is a Vec
 */
template<bool setResult>
inline ArrayData* SetElemVecPre(ArrayData* a,
                                int64_t key,
                                TypedValue* value) {
  return PackedArray::SetIntVec(a, key, *value);
}

template <bool setResult>
inline ArrayData*
SetElemVecPre(ArrayData* a, StringData* key, TypedValue* /*value*/) {
  throwInvalidArrayKeyException(key, a);
}

template<bool setResult>
inline ArrayData* SetElemVecPre(ArrayData* a,
                                TypedValue key,
                                TypedValue* value) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) {
    return SetElemVecPre<setResult>(a, key.m_data.num, value);
  }
  if (isStringType(dt)) {
    return SetElemVecPre<setResult>(a, key.m_data.pstr, value);
  }
  throwInvalidArrayKeyException(&key, a);
}

template <bool setResult, KeyType keyType, bool copyProv>
inline void SetElemVec(tv_lval base, key_type<keyType> key, TypedValue* value) {
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  auto* newData = SetElemVecPre<setResult>(a, key, value);
  assertx(newData->isVecArrayType());

  arraySetUpdateBase<KindOfVec>(a, newData, base);
}

/**
 * SetElem when base is a Dict
 */
template<bool setResult>
inline ArrayData* SetElemDictPre(ArrayData* a,
                                 int64_t key,
                                 TypedValue* value) {
  return MixedArray::SetIntDict(a, key, *value);
}

template<bool setResult>
inline ArrayData* SetElemDictPre(ArrayData* a,
                                 StringData* key,
                                 TypedValue* value) {
  return MixedArray::SetStrDict(a, key, *value);
}

template<bool setResult>
inline ArrayData* SetElemDictPre(ArrayData* a,
                                 TypedValue key,
                                 TypedValue* value) {
  auto const dt = key.m_type;
  if (isIntType(dt)) {
    return SetElemDictPre<setResult>(a, key.m_data.num, value);
  }
  if (isStringType(dt)) {
    return SetElemDictPre<setResult>(a, key.m_data.pstr, value);
  }
  throwInvalidArrayKeyException(&key, a);
}

template <bool setResult, KeyType keyType, bool copyProv>
inline void SetElemDict(tv_lval base, key_type<keyType> key,
                        TypedValue* value) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  auto newData = SetElemDictPre<setResult>(a, key, value);
  assertx(newData->isDictKind());

  arraySetUpdateBase<KindOfDict>(a, newData, base);
}

/**
 * SetElem() leaves the result in 'value', rather than returning it as in
 * SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
 * get around.
 */
template <bool setResult, KeyType keyType, bool copyProv>
NEVER_INLINE
StringData* SetElemSlow(tv_lval base,
                        key_type<keyType> key,
                        TypedValue* value,
                        const MInstrPropState* pState) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      SetElemEmptyish<keyType>(base, key, value, pState);
      return nullptr;
    case KindOfBoolean:
      SetElemBoolean<setResult, keyType>(base, key, value, pState);
      return nullptr;
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      SetElemScalar<setResult>(value);
      return nullptr;
    case KindOfPersistentString:
    case KindOfString:
      return SetElemString<setResult, keyType>(base, key, value, pState);
    case KindOfPersistentVec:
    case KindOfVec:
      SetElemVec<setResult, keyType, copyProv>(base, key, value);
      return nullptr;
    case KindOfPersistentDict:
    case KindOfDict:
      SetElemDict<setResult, keyType, copyProv>(base, key, value);
      return nullptr;
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      SetElemArray<setResult, keyType>(base, key, value);
      return nullptr;
    case KindOfObject:
      SetElemObject<keyType>(base, key, value);
      return nullptr;
    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      if (RO::EvalHackArrDVArrs) {
        SetElemVec<setResult, keyType, copyProv>(base, key, value);
      } else {
        SetElemArray<setResult, keyType>(base, key, value);
      }
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
template <bool setResult, bool copyProv, KeyType keyType = KeyType::Any>
inline StringData* SetElem(tv_lval base, key_type<keyType> key,
                           TypedValue* value, const MInstrPropState* pState) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    SetElemArray<setResult, keyType>(base, key, value);
    return nullptr;
  }
  if (LIKELY(tvIsVec(base))) {
    SetElemVec<setResult, keyType, copyProv>(base, key, value);
    return nullptr;
  }
  if (LIKELY(tvIsDict(base))) {
    SetElemDict<setResult, keyType, copyProv>(base, key, value);
    return nullptr;
  }
  return SetElemSlow<setResult, keyType, copyProv>(base, key, value, pState);
}

template<bool reverse>
void SetRange(
  tv_lval base, int64_t offset, TypedValue src, int64_t count, int64_t size
);

/**
 * SetNewElem when base is Null
 */
inline void SetNewElemEmptyish(tv_lval base,
                               TypedValue* value,
                               const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  Array a = Array::CreateVArray();
  a.append(tvAsCVarRef(*value));
  tvMove(make_array_like_tv(a.detach()), base);
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
inline void SetNewElemBoolean(tv_lval base,
                              TypedValue* value,
                              const MInstrPropState* pState) {
  if (val(base).num) {
    SetNewElemScalar<setResult>(value);
  } else {
    SetNewElemEmptyish(base, value, pState);
  }
}

/**
 * SetNewElem when base is a String
 */
inline void SetNewElemString(tv_lval base,
                             TypedValue* value,
                             const MInstrPropState* pState) {
  int baseLen = val(base).pstr->size();
  if (baseLen == 0) {
    SetNewElemEmptyish(base, value, pState);
  } else {
    raise_error("[] operator not supported for strings");
  }
}

/**
 * SetNewElem when base is an Array
 */
inline void SetNewElemArray(tv_lval base, TypedValue* value) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto a2 = a->append(*value);
  if (a2 != a) {
    assertx(a2->isPHPArrayType());
    type(base) = a2->toDataType();
    val(base).parr = a2;
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Vec
 */
template <bool copyProv>
inline void SetNewElemVec(tv_lval base, TypedValue* value) {
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto a2 = PackedArray::AppendVec(a, *value);
  if (a2 != a) {
    type(base) = KindOfVec;
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Dict
 */
template <bool copyProv>
inline void SetNewElemDict(tv_lval base, TypedValue* value) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto a2 = MixedArray::AppendDict(a, *value);
  if (a2 != a) {
    type(base) = KindOfDict;
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
template <bool setResult, bool copyProv>
inline void SetNewElem(tv_lval base,
                       TypedValue* value,
                       const MInstrPropState* pState) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetNewElemEmptyish(base, value, pState);
    case KindOfBoolean:
      return SetNewElemBoolean<setResult>(base, value, pState);
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return SetNewElemScalar<setResult>(value);
    case KindOfPersistentString:
    case KindOfString:
      return SetNewElemString(base, value, pState);
    case KindOfPersistentVec:
    case KindOfVec:
      return SetNewElemVec<copyProv>(base, value);
    case KindOfPersistentDict:
    case KindOfDict:
      return SetNewElemDict<copyProv>(base, value);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return SetNewElemKeyset(base, value);
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return SetNewElemArray(base, value);
    case KindOfObject:
      return SetNewElemObject(base, value);
    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      if (RO::EvalHackArrDVArrs) {
        return SetNewElemVec<copyProv>(base, value);
      } else {
        return SetNewElemArray(base, value);
      }
    case KindOfRecord:
      raise_error(Strings::OP_NOT_SUPPORTED_RECORD);
  }
  unknownBaseType(type(base));
}

/**
 * SetOpElem when base is Null
 */
inline tv_lval SetOpElemEmptyish(SetOpOp op, tv_lval base,
                                 TypedValue key, TypedValue* rhs,
                                 const MInstrPropState* pState) {
  assertx(tvIsPlausible(*base));

  detail::checkPromotion(base, pState);

  tvMove(make_array_like_tv(ArrayData::CreateDArray()), base);
  auto const lval = asArrRef(base).lval(tvAsCVarRef(&key));
  setopBody(lval, op, rhs);
  return lval;
}

/**
 * SetOpElem when base is Int64, Double, Resource, Func, or Class
 */
inline tv_lval SetOpElemScalar(TypedValue& tvRef) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(tvRef);
  return &tvRef;
}

/**
 * $result = ($base[$x] <op>= $y)
 */
inline tv_lval SetOpElem(TypedValue& tvRef,
                         SetOpOp op, tv_lval base,
                         TypedValue key, TypedValue* rhs,
                         const MInstrPropState* pState) {
  assertx(tvIsPlausible(*base));

  auto const handleArray = [&] {
    if (UNLIKELY(!asCArrRef(base).exists(tvAsCVarRef(&key)))) {
      throwMissingElementException("Set-op");
    }
    auto result =
      ElemDArray<MOpMode::None, KeyType::Any>(base, key);
    setopBody(result, op, rhs);
    return result;
  };

  auto const handleVec = [&] {
    auto result = [&]{
      if (RuntimeOption::EvalArrayProvenance) {
        return ElemDVec<KeyType::Any, true>(base, key);
      } else {
        return ElemDVec<KeyType::Any, false>(base, key);
      }
    }();
    result = tvAssertPlausible(result);
    setopBody(result, op, rhs);
    return result;
  };

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpElemEmptyish(op, base, key, rhs, pState);

    case KindOfBoolean:
      if (val(base).num) {
        return SetOpElemScalar(tvRef);
      }
      return SetOpElemEmptyish(op, base, key, rhs, pState);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return SetOpElemScalar(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("Cannot use assign-op operators with overloaded "
          "objects nor string offsets");
      }
      return SetOpElemEmptyish(op, base, key, rhs, pState);

    case KindOfPersistentVec:
    case KindOfVec:
      return handleVec();

    case KindOfPersistentDict:
    case KindOfDict: {
      auto result = [&]{
        if (RuntimeOption::EvalArrayProvenance) {
          return ElemDDict<KeyType::Any, true>(base, key);
        } else {
          return ElemDDict<KeyType::Any, false>(base, key);
        }
      }();
      result = tvAssertPlausible(result);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return handleArray();

    case KindOfObject: {
      auto obj = val(base).pobj;
      failOnNonCollectionObjArrayAccess(obj);
      auto result = collections::atRw(obj, &key);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      if (RO::EvalHackArrDVArrs) {
        return handleVec();
      } else {
        return handleArray();
      }

    case KindOfRecord: {
      auto result = ElemDRecord<KeyType::Any>(base, key);
      result = tvAssertPlausible(result);
      setopBody(result, op, rhs);
      return result;
    }
  }
  unknownBaseType(type(base));
}

inline tv_lval SetOpNewElemEmptyish(SetOpOp op, tv_lval base, TypedValue* rhs,
                                    const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  tvMove(make_array_like_tv(ArrayData::CreateVArray()), base);
  throwMissingElementException("Lval");
  return nullptr;
}
inline tv_lval SetOpNewElemScalar(TypedValue& tvRef) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(tvRef);
  return &tvRef;
}
inline tv_lval SetOpNewElem(TypedValue& tvRef,
                            SetOpOp op, tv_lval base,
                            TypedValue* rhs, const MInstrPropState* pState) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpNewElemEmptyish(op, base, rhs, pState);

    case KindOfBoolean:
      if (val(base).num) {
        return SetOpNewElemScalar(tvRef);
      }
      return SetOpNewElemEmptyish(op, base, rhs, pState);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return SetOpNewElemScalar(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("[] operator not supported for strings");
      }
      return SetOpNewElemEmptyish(op, base, rhs, pState);

    case KindOfPersistentVec:
    case KindOfVec:
      throw_cannot_use_newelem_for_lval_read_vec();
    case KindOfPersistentDict:
    case KindOfDict:
      throw_cannot_use_newelem_for_lval_read_dict();
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throw_cannot_use_newelem_for_lval_read_keyset();

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      throwMissingElementException("Set-op");

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

inline TypedValue IncDecElemEmptyish(
  IncDecOp op,
  tv_lval base,
  TypedValue key,
  const MInstrPropState* pState
) {
  detail::checkPromotion(base, pState);

  tvMove(make_array_like_tv(ArrayData::CreateDArray()), base);
  auto const lval = asArrRef(base).lval(tvAsCVarRef(&key));
  assertx(type(lval) == KindOfNull);
  return IncDecBody(op, lval);
}

inline TypedValue IncDecElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

inline TypedValue IncDecElem(
  IncDecOp op,
  tv_lval base,
  TypedValue key,
  const MInstrPropState* pState
) {
  assertx(tvIsPlausible(*base));

  auto const handleArray = [&] {
    if (UNLIKELY(!asCArrRef(base).exists(tvAsCVarRef(&key)))) {
      throwMissingElementException("Inc/dec");
    }
    auto result =
      ElemDArray<MOpMode::None, KeyType::Any>(base, key);
    return IncDecBody(op, result);
  };

  auto const handleVec = [&] {
    auto result = [&]{
      if (RuntimeOption::EvalArrayProvenance) {
        return ElemDVec<KeyType::Any, true>(base, key);
      } else {
        return ElemDVec<KeyType::Any, false>(base, key);
      }
    }();
    return IncDecBody(op, tvAssertPlausible(result));
  };

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecElemEmptyish(op, base, key, pState);

    case KindOfBoolean:
      if (val(base).num) {
        return IncDecElemScalar();
      }
      return IncDecElemEmptyish(op, base, key, pState);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return IncDecElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("Cannot increment/decrement overloaded objects "
          "nor string offsets");
      }
      return IncDecElemEmptyish(op, base, key, pState);

    case KindOfPersistentVec:
    case KindOfVec:
      return handleVec();

    case KindOfPersistentDict:
    case KindOfDict: {
      auto result = [&]{
        if (RuntimeOption::EvalArrayProvenance) {
          return ElemDDict<KeyType::Any, true>(base, key);
        } else {
          return ElemDDict<KeyType::Any, false>(base, key);
        }
      }();
      return IncDecBody(op, tvAssertPlausible(result));
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return handleArray();

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
      if (RO::EvalHackArrDVArrs) {
        return handleVec();
      } else {
        return handleArray();
      }

    case KindOfRecord: {
      auto result = ElemDRecord<KeyType::Any>(base, key);
      return IncDecBody(op, tvAssertPlausible(result));
    }
  }
  unknownBaseType(type(base));
}

inline TypedValue IncDecNewElemEmptyish(
  IncDecOp op,
  tv_lval base,
  const MInstrPropState* pState
) {
  detail::checkPromotion(base, pState);
  tvMove(make_array_like_tv(ArrayData::CreateVArray()), base);
  throwMissingElementException("Lval");
  return make_tv<KindOfNull>();
}

inline TypedValue IncDecNewElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

inline TypedValue IncDecNewElem(
  TypedValue& tvRef,
  IncDecOp op,
  tv_lval base,
  const MInstrPropState* pState
) {
  assertx(tvIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecNewElemEmptyish(op, base, pState);

    case KindOfBoolean:
      if (val(base).num) {
        return IncDecNewElemScalar();
      }
      return IncDecNewElemEmptyish(op, base, pState);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return IncDecNewElemScalar();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        raise_error("[] operator not supported for strings");
      }
      return IncDecNewElemEmptyish(op, base, pState);

    case KindOfPersistentVec:
    case KindOfVec:
      throw_cannot_use_newelem_for_lval_read_vec();
    case KindOfPersistentDict:
    case KindOfDict:
      throw_cannot_use_newelem_for_lval_read_dict();
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throw_cannot_use_newelem_for_lval_read_keyset();

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      throwMissingElementException("Inc/dec");

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
 * UnsetElemArray when key is an Int64
 */
inline ArrayData* UnsetElemArrayPre(ArrayData* a, int64_t key) {
  return a->remove(key);
}

/**
 * UnsetElemArray when key is a String
 */
inline ArrayData* UnsetElemArrayPre(ArrayData* a, StringData* key) {
  assertx(a->isPHPArrayType());
  return a->remove(key);
}

inline ArrayData* UnsetElemArrayPre(ArrayData* a, TypedValue key) {
  if (isStringType(key.m_type)) {
    return UnsetElemArrayPre(a, key.m_data.pstr);
  }
  if (key.m_type == KindOfInt64) {
    return UnsetElemArrayPre(a, key.m_data.num);
  }
  if (isFuncType(key.m_type)) {
    return UnsetElemArrayPre(
      a, const_cast<StringData*>(funcToStringHelper(key.m_data.pfunc))
    );
  }
  if (isClassType(key.m_type)) {
    return UnsetElemArrayPre(
      a, const_cast<StringData*>(classToStringHelper(key.m_data.pclass))
    );
  }
  auto const k = tvToKey(key, a);
  if (isNullType(k.m_type)) return a;
  return a->remove(k);
}

/**
 * UnsetElem when base is an Array
 */
template <KeyType keyType>
inline void UnsetElemArray(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemArrayPre(a, key);

  if (a2 != a) {
    assertx(a2->isPHPArrayType());
    type(base) = a2->toDataType();
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * UnsetElem when base is a Vec
 */

inline ArrayData* UnsetElemVecPre(ArrayData* a, int64_t key) {
  return PackedArray::RemoveIntVec(a, key);
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
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemVecPre(a, key);
  assertx(a2->isVecArrayType() || a2->isDictType());

  if (a2 != a) {
    type(base) = a2->toDataType();
    val(base).parr = a2;
    assertx(tvIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * UnsetElem when base is a Dict
 */

inline ArrayData* UnsetElemDictPre(ArrayData* a, int64_t key) {
  return MixedArray::RemoveIntDict(a, key);
}

inline ArrayData* UnsetElemDictPre(ArrayData* a, StringData* key) {
  return MixedArray::RemoveStrDict(a, key);
}

inline ArrayData* UnsetElemDictPre(ArrayData* a, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return UnsetElemDictPre(a, key.m_data.num);
  if (isStringType(dt)) return UnsetElemDictPre(a, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, a);
}

template <KeyType keyType>
inline void UnsetElemDict(tv_lval base, key_type<keyType> key) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemDictPre(a, key);

  if (a2 != a) {
    type(base) = KindOfDict;
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

    case KindOfFunc:
      raise_error("Cannot unset a func");
      return;
    case KindOfClass:
      raise_error("Cannot unset a class");
      return;

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

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      UnsetElemArray<keyType>(base, key);
      return;

    case KindOfObject: {
      auto obj = val(base).pobj;
      failOnNonCollectionObjArrayAccess(obj);
      auto const& scratchKey = initScratchKey(key);
      collections::unset(obj, &scratchKey);
      return;
    }

    case KindOfClsMeth:
      detail::promoteClsMeth(base);
      if (RO::EvalHackArrDVArrs) {
        UnsetElemVec<keyType>(base, key);
      } else {
        UnsetElemArray<keyType>(base, key);
      }
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

  if (LIKELY(tvIsArray(base))) {
    return UnsetElemArray<keyType>(base, key);
  }
  if (LIKELY(tvIsVec(base))) {
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
 * IssetElem when base is an Array
 */
template<KeyType keyType>
bool IssetElemArray(ArrayData* a, key_type<keyType> key) {
  assertx(a->isPHPArrayType());
  auto const result = ElemArray<MOpMode::None, keyType>(a, key);
  return !tvIsNull(result.tv());
}

/**
 * IssetElem when base is a Vec
 */
template<KeyType keyType>
bool IssetElemVec(ArrayData* a, key_type<keyType> key) {
  assertx(a->isVecArrayKind());
  auto const result = ElemVec<MOpMode::None, keyType>(a, key);
  return !tvIsNull(tvAssertPlausible(result.tv()));
}

/**
 * IssetElem when base is a Dict
 */
template<KeyType keyType>
bool IssetElemDict(ArrayData* a, key_type<keyType> key) {
  assertx(a->isDictKind());
  auto const result = ElemDict<MOpMode::None, keyType>(a, key);
  return !tvIsNull(tvAssertPlausible(result.tv()));
}

/**
 * IssetElem when base is a Keyset
 */
template<KeyType keyType>
bool IssetElemKeyset(ArrayData* a, key_type<keyType> key) {
  assertx(a->isKeysetKind());
  auto const result = ElemKeyset<MOpMode::None, keyType>(a, key);
  return !tvIsNull(tvAssertPlausible(result.tv()));
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
  auto const  result = ElemRecord<keyType>(base, key);
  return !tvIsNull(tvAssertPlausible(result.tv()));
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
      return false;

    case KindOfFunc:
      return IssetElemString<keyType>(
        funcToStringHelper(val(base).pfunc), key
      );

    case KindOfClass:
      return IssetElemString<keyType>(
        classToStringHelper(val(base).pclass), key
      );

    case KindOfPersistentString:
    case KindOfString:
      return IssetElemString<keyType>(val(base).pstr, key);

    case KindOfPersistentVec:
    case KindOfVec:
      return IssetElemVec<keyType>(val(base).parr, key);

    case KindOfPersistentDict:
    case KindOfDict:
      return IssetElemDict<keyType>(val(base).parr, key);

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return IssetElemKeyset<keyType>(val(base).parr, key);

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
      return IssetElemArray<keyType>(val(base).parr, key);

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

  if (LIKELY(tvIsArray(base))) {
    return IssetElemArray<keyType>(val(base).parr, key);
  }
  if (LIKELY(tvIsVec(base))) {
    return IssetElemVec<keyType>(val(base).parr, key);
  }
  if (LIKELY(tvIsDict(base))) {
    return IssetElemDict<keyType>(val(base).parr, key);
  }
  if (LIKELY(tvIsKeyset(base))) {
    return IssetElemKeyset<keyType>(val(base).parr, key);
  }
  return IssetElemSlow<keyType>(base, key);
}

template<MOpMode mode>
inline tv_lval propPreNull(TypedValue& tvRef, MInstrPropState* pState) {
  tvWriteNull(tvRef);
  if (mode == MOpMode::Warn) {
    raise_notice("Cannot access property on non-object");
  }
  if (mode == MOpMode::Define && pState) *pState = MInstrPropState{};
  return tv_lval(&tvRef);
}

template <class F>
inline void promoteToStdClass(tv_lval base,
                              F fun,
                              const MInstrPropState* pState) {
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

  if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    assertx(pState != nullptr);
    auto const cls = pState->getClass();
    if (UNLIKELY(cls != nullptr)) {
      auto const slot = pState->getSlot();
      auto tv = make_tv<KindOfObject>(obj.get());
      if (pState->isStatic()) {
        assertx(slot < cls->numStaticProperties());
        auto const& sprop = cls->staticProperties()[slot];
        auto const& tc = sprop.typeConstraint;
        if (tc.isCheckable()) {
          tc.verifyStaticProperty(&tv, cls, sprop.cls, sprop.name);
        }
      } else {
        assertx(slot < cls->numDeclProperties());
        auto const& prop = cls->declProperties()[slot];
        auto const& tc = prop.typeConstraint;
        if (tc.isCheckable()) tc.verifyProperty(&tv, cls, prop.cls, prop.name);
      }

      // No coercion for should occur for object types.
      assertx(isObjectType(tv.m_type) && tv.m_data.pobj == obj.get());
    }
  }

  if (base.type() == KindOfString) {
    decRefStr(base.val().pstr);
  } else {
    assertx(!isRefcountedType(base.type()));
  }
  base.type() = KindOfObject;
  base.val().pobj = obj.get();

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

  fun(obj.get());
}

template<MOpMode mode>
tv_lval propPreStdclass(TypedValue& tvRef,
                        tv_lval base,
                        MInstrPropState* pState) {
  if (mode != MOpMode::Define) {
    return propPreNull<mode>(tvRef, pState);
  }

  promoteToStdClass(base, [] (ObjectData*) {}, pState);
  if (UNLIKELY(base.type() != KindOfObject)) {
    // See the comments above. Although promoteToStdClass will have
    // either thrown an exception, or promoted base to an object, an
    // installed error handler might have caused it to be overwritten
    tvWriteNull(tvRef);
    if (pState) *pState = MInstrPropState{};
    return tv_lval(&tvRef);
  }

  return base;
}

template<MOpMode mode>
tv_lval propPre(TypedValue& tvRef, tv_lval base, MInstrPropState* pState) {
  switch (base.type()) {
    case KindOfUninit:
    case KindOfNull:
      return propPreStdclass<mode>(tvRef, base, pState);

    case KindOfBoolean:
      if (base.val().num) {
        return propPreNull<mode>(tvRef, pState);
      }
      return propPreStdclass<mode>(tvRef, base, pState);

    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return propPreNull<mode>(tvRef, pState);

    case KindOfPersistentString:
    case KindOfString:
      if (base.val().pstr->size() != 0) {
        return propPreNull<mode>(tvRef, pState);
      }
      return propPreStdclass<mode>(tvRef, base, pState);

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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfClsMeth:
    case KindOfRecord:
      return propPreNull<mode>(tvRef, pState);

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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
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
                       ObjectData* instance, key_type<keyType> key,
                       MInstrPropState* pState) {
  auto keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Get property.
  if (mode == MOpMode::Define) {
    return instance->propD(&tvRef, ctx, keySD, pState);
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
inline tv_lval Prop(TypedValue& tvRef,
                    const Class* ctx,
                    tv_lval base,
                    key_type<keyType> key,
                    MInstrPropState* pState) {
  auto result = propPre<mode>(tvRef, base, pState);
  if (result.type() == KindOfNull) return result;

  return PropObj<mode,keyType>(
    tvRef, ctx, instanceFromTv(result), key, pState
  );
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

inline void SetPropStdclass(tv_lval base,
                            TypedValue key,
                            TypedValue* val,
                            const MInstrPropState* pState) {
  promoteToStdClass(
    base,
    [&] (ObjectData* obj) {
      auto const keySD = prepareKey(key);
      SCOPE_EXIT { decRefStr(keySD); };
      obj->setProp(nullptr, keySD, *val);
    },
    pState
  );
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
                    TypedValue* val, const MInstrPropState* pState) {
  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetPropStdclass(base, initScratchKey(key), val, pState);

    case KindOfBoolean:
      if (HPHP::val(base).num) {
        return SetPropNull<setResult>(val);
      }
      return SetPropStdclass(base, initScratchKey(key), val, pState);

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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRecord:
      return SetPropNull<setResult>(val);

    case KindOfPersistentString:
    case KindOfString:
      if (HPHP::val(base).pstr->size() != 0) {
        return SetPropNull<setResult>(val);
      }
      return SetPropStdclass(base, initScratchKey(key), val, pState);

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

inline tv_lval SetOpPropStdclass(TypedValue& tvRef, SetOpOp op,
                                 tv_lval base, TypedValue key,
                                 TypedValue* rhs, const MInstrPropState* pState) {
  promoteToStdClass(
    base,
    [&] (ObjectData* obj) {
      StringData* keySD = prepareKey(key);
      SCOPE_EXIT { decRefStr(keySD); };
      tvWriteNull(tvRef);
      setopBody(tvAssertPlausible(&tvRef), op, rhs);
      obj->setProp(nullptr, keySD, tvAssertPlausible(tvRef));
    },
    pState
  );

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
                         TypedValue* rhs, const MInstrPropState* pState) {
  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return SetOpPropStdclass(tvRef, op, base, key, rhs, pState);

    case KindOfBoolean:
      if (val(base).num) {
        return SetOpPropNull(tvRef);
      }
      return SetOpPropStdclass(tvRef, op, base, key, rhs, pState);

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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRecord:
      return SetOpPropNull(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        return SetOpPropNull(tvRef);
      }
      return SetOpPropStdclass(tvRef, op, base, key, rhs, pState);

    case KindOfObject:
      return SetOpPropObj(tvRef, ctx, op, instanceFromTv(base), key, rhs);
  }
  unknownBaseType(type(base));
}

inline TypedValue IncDecPropNull() {
  raise_warning("Attempt to increment/decrement property of non-object");
  return make_tv<KindOfNull>();
}

inline TypedValue IncDecPropStdclass(IncDecOp op, tv_lval base,
                               TypedValue key, const MInstrPropState* pState) {
  TypedValue dest;
  promoteToStdClass(
    base,
    [&] (ObjectData* obj) {
      StringData* keySD = prepareKey(key);
      SCOPE_EXIT { decRefStr(keySD); };
      TypedValue tv;
      tvWriteNull(tv);
      dest = IncDecBody(op, &tv);
      obj->setProp(nullptr, keySD, dest);
      assertx(!isRefcountedType(tv.m_type));
    },
    pState
  );

  return dest;
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
  TypedValue key,
  const MInstrPropState* pState
) {
  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
      return IncDecPropStdclass(op, base, key, pState);

    case KindOfBoolean:
      if (val(base).num) {
        return IncDecPropNull();
      }
      return IncDecPropStdclass(op, base, key, pState);

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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
    case KindOfClsMeth:
    case KindOfRecord:
      return IncDecPropNull();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        return IncDecPropNull();
      }
      return IncDecPropStdclass(op, base, key, pState);

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
#endif // incl_HPHP_VM_MEMBER_OPERATIONS_H_
