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
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
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

  ~InvalidSetMException() noexcept override {}

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
  assertx(!isRefType(tv.m_type));
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

inline ObjectData* instanceFromTv(tv_rval tv) {
  assertx(tvIsObject(tv));
  return val(tv).pobj;
}

[[noreturn]] void throw_cannot_use_newelem_for_lval_read_col();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_vec();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_dict();
[[noreturn]] void throw_cannot_use_newelem_for_lval_read_keyset();

[[noreturn]] void unknownBaseType(DataType);

void raise_inout_undefined_index(TypedValue);
void raise_inout_undefined_index(int64_t i);
void raise_inout_undefined_index(const StringData* sd);

namespace detail {

ALWAYS_INLINE void checkPromotion(tv_rval base, const MInstrPropState* pState) {
  if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    assertx(pState != nullptr);
    auto const cls = pState->getClass();
    if (UNLIKELY(cls != nullptr)) {
      auto const slot = pState->getSlot();
      auto const tv = make_tv<KindOfArray>(staticEmptyArray());
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
    }
  }

  if (LIKELY(!checkHACFalseyPromote())) return;

  if (tvIsNull(base)) {
    raise_hackarr_compat_notice("Promoting null to array");
  } else if (tvIsBool(base)) {
    raise_hackarr_compat_notice("Promoting false to array");
  }
}

}

/**
 * Elem when base is Null
 */
inline tv_rval ElemEmptyish() {
  return tv_rval { &immutable_null_base };
}

template<MOpMode mode, bool intishWarn>
inline tv_rval ElemArrayPre(ArrayData* base, int64_t key) {
  return mode == MOpMode::Warn ? base->rvalStrict(key) : base->rval(key);
}

template<MOpMode mode, bool intishWarn>
inline tv_rval ElemArrayPre(ArrayData* base, StringData* key) {
  auto constexpr warn = mode == MOpMode::Warn;
  int64_t n;
  assertx(base->isPHPArray());
  if (key->isStrictlyInteger(n)) {
    if (intishWarn) raise_intish_index_cast();
    return warn ? base->rvalStrict(n) : base->rval(n);
  } else {
    return warn ? base->rvalStrict(key) : base->rval(key);
  }
}

template<MOpMode mode, bool intishWarn>
inline tv_rval ElemArrayPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    {
    return ElemArrayPre<mode, false>(base, key.m_data.num);
  }
  if (isStringType(dt)) {
    return ElemArrayPre<mode, intishWarn>(base, key.m_data.pstr);
  }
  if (isFuncType(dt)) {
    return ElemArrayPre<mode, intishWarn>(
      base, const_cast<StringData*>(funcToStringHelper(key.m_data.pfunc))
    );
  }

  // TODO(#3888164): Array elements can never be KindOfUninit.  This API should
  // be changed.
  auto const rval = ArrNR{base}.asArray().rvalAt(cellAsCVarRef(key));
  return rval.type() != KindOfUninit ? rval : tv_rval { nullptr };
}

/**
 * Fast path for Elem assuming base is an Array.  Does not unbox the returned
 * pointer.
 */
template<MOpMode mode, KeyType keyType, bool intishWarn>
inline tv_rval ElemArray(ArrayData* base, key_type<keyType> key) {
  assertx(base->isPHPArray());

  auto result = ElemArrayPre<mode, intishWarn>(base, key);

  if (UNLIKELY(!result)) {
    if (mode == MOpMode::Warn) {
      auto const scratch = initScratchKey(key);
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(&scratch).toString().data());
    }
    if (mode == MOpMode::InOut) {
      raise_inout_undefined_index(initScratchKey(key));
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
  return mode == MOpMode::Warn || mode == MOpMode::InOut
    ? PackedArray::RvalIntStrictVec(base, key)
    : PackedArray::RvalIntVec(base, key);
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
  if (LIKELY(isIntType(dt))) return ElemVecPre<mode>(base, key.m_data.num);
  if (isStringType(dt))      return ElemVecPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline tv_rval ElemVec(ArrayData* base, key_type<keyType> key) {
  assertx(base->isVecArray());
  auto result = ElemVecPre<mode>(base, key);
  if (mode != MOpMode::Warn && mode != MOpMode::InOut) {
    if (UNLIKELY(!result)) return ElemEmptyish();
  }
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Dict
 */
template<MOpMode mode>
inline tv_rval ElemDictPre(ArrayData* base, int64_t key) {
  return mode == MOpMode::Warn || mode == MOpMode::InOut
    ? MixedArray::RvalIntStrictDict(base, key)
    : MixedArray::RvalIntDict(base, key);
}

template<MOpMode mode>
inline tv_rval ElemDictPre(ArrayData* base, StringData* key) {
  return mode == MOpMode::Warn || mode == MOpMode::InOut
    ? MixedArray::RvalStrStrictDict(base, key)
    : MixedArray::RvalStrDict(base, key);
}

template<MOpMode mode>
inline tv_rval ElemDictPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemDictPre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemDictPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline tv_rval ElemDict(ArrayData* base, key_type<keyType> key) {
  assertx(base->isDict());
  auto result = ElemDictPre<mode>(base, key);
  if (mode != MOpMode::Warn && mode != MOpMode::InOut) {
    if (UNLIKELY(!result)) return ElemEmptyish();
  }
  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * Elem when base is a Keyset
 */
template<MOpMode mode>
inline tv_rval ElemKeysetPre(ArrayData* base, int64_t key) {
  return mode == MOpMode::Warn || mode == MOpMode::InOut
    ? SetArray::RvalIntStrict(base, key)
    : SetArray::RvalInt(base, key);
}

template<MOpMode mode>
inline tv_rval ElemKeysetPre(ArrayData* base, StringData* key) {
  return mode == MOpMode::Warn || mode == MOpMode::InOut
    ? SetArray::RvalStrStrict(base, key)
    : SetArray::RvalStr(base, key);
}

template<MOpMode mode>
inline tv_rval ElemKeysetPre(ArrayData* base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemKeysetPre<mode>(base, key.m_data.num);
  if (isStringType(dt)) return ElemKeysetPre<mode>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base);
}

template<MOpMode mode, KeyType keyType>
inline tv_rval ElemKeyset(ArrayData* base, key_type<keyType> key) {
  assertx(base->isKeyset());
  auto result = ElemKeysetPre<mode>(base, key);
  if (mode != MOpMode::Warn && mode != MOpMode::InOut) {
    if (UNLIKELY(!result)) return ElemEmptyish();
  }
  assertx(isIntType(result.type()) || isStringType(result.type()));
  return result;
}

/**
 * Elem when base is an Int64, Double, or Resource.
 */
inline tv_rval ElemScalar() {
  if (RuntimeOption::EnableHipHopSyntax) {
    raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  }
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
    return cellAsCVarRef(key).toInt64();
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
  auto scratch = initScratchKey(key);

  if (LIKELY(base->isCollection())) {
    if (mode == MOpMode::Warn) {
      return collections::at(base, &scratch);
    }
    auto res = collections::get(base, &scratch);
    if (!res) {
      res = &tvRef;
      tvWriteNull(*res);
    }
    return res;
  }

  tvRef = objOffsetGet(base, scratch);
  return &tvRef;
}

/**
 * $result = $base[$key];
 */
template<MOpMode mode, KeyType keyType, bool intishWarn>
NEVER_INLINE tv_rval ElemSlow(TypedValue& tvRef,
                              tv_rval base,
                              key_type<keyType> key) {
  base = base.unboxed();
  assertx(cellIsPlausible(*base));

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
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemArray<mode, keyType, intishWarn>(base.val().parr, key);
    case KindOfObject:
      return ElemObject<mode, keyType>(tvRef, base.val().pobj, key);
    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

template<MOpMode mode, bool intishWarn, KeyType keyType = KeyType::Any>
inline tv_rval Elem(TypedValue& tvRef,
                    tv_rval base,
                    key_type<keyType> key) {
  assertx(mode != MOpMode::Define && mode != MOpMode::Unset);
  assertx(tvIsPlausible(base.tv()));

  if (mode == MOpMode::InOut) {
    if (UNLIKELY(tvIsRef(base) && !base.val().pref->isReferenced())) {
      base = base.unboxed();
    }
  }

  if (LIKELY(tvIsArray(base))) {
    return ElemArray<mode, keyType, intishWarn>(base.val().parr, key);
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

  return ElemSlow<mode, keyType, intishWarn>(tvRef, base, key);
}

template<MOpMode mode, bool reffy, bool intishWarn>
inline tv_lval ElemDArrayPre(tv_lval base, int64_t key, bool& defined) {
  auto oldArr = val(base).parr;

  defined = (mode != MOpMode::Warn) || oldArr->exists(key);
  auto const lval = reffy
    ? oldArr->lvalRef(key, oldArr->cowCheck())
    : oldArr->lval(key, oldArr->cowCheck());

  if (lval.arr != oldArr) {
    type(base) = KindOfArray;
    val(base).parr = lval.arr;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }

  return lval;
}

template<MOpMode mode, bool reffy, bool intishWarn>
inline tv_lval ElemDArrayPre(tv_lval base, StringData* key,
                             bool& defined) {
  auto oldArr = val(base).parr;

  auto const lval = [&]{
    auto const cow = oldArr->cowCheck();
    int64_t n;
    if (oldArr->convertKey(key, n, intishWarn)) {
      defined = (mode != MOpMode::Warn) || oldArr->exists(n);
      return reffy ? oldArr->lvalRef(n, cow) : oldArr->lval(n, cow);
    } else {
      defined = (mode != MOpMode::Warn) || oldArr->exists(key);
      return reffy ? oldArr->lvalRef(key, cow) : oldArr->lval(key, cow);
    }
  }();

  if (lval.arr != oldArr) {
    type(base) = KindOfArray;
    val(base).parr = lval.arr;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
}

template<MOpMode mode, bool reffy, bool intishWarn>
inline tv_lval ElemDArrayPre(tv_lval base, TypedValue key,
                             bool& defined) {
  auto const dt = key.m_type;
  if (isIntType(dt)) {
    return ElemDArrayPre<mode, reffy, false>(base, key.m_data.num, defined);
  }
  if (isStringType(dt)) {
    return ElemDArrayPre<mode, reffy, intishWarn>(
      base, key.m_data.pstr, defined
    );
  }
  auto& arr = asArrRef(base);
  defined = (mode != MOpMode::Warn) || arr.exists(tvAsCVarRef(&key));
  return reffy ? arr.lvalAtRef(key) : arr.lvalAt(key);
}

/**
 * ElemD when base is an Array
 */
template<MOpMode mode, bool reffy, bool intishWarn, KeyType keyType>
inline tv_lval ElemDArray(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));

  bool defined;
  auto lval = ElemDArrayPre<mode, reffy, intishWarn>(base, key, defined);

  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  assertx(lval.type() != KindOfUninit);

  if (!defined) {
    auto scratchKey = initScratchKey(key);
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&scratchKey).toString().data());
  }

  return lval;
}

/**
 * ElemD when base is a Vec
 */
template <bool reffy>
inline tv_lval ElemDVecPre(tv_lval base, int64_t key) {
  ArrayData* oldArr = base.val().parr;

  if (reffy) throwRefInvalidArrayValueException(oldArr);

  auto const lval = PackedArray::LvalIntVec(oldArr, key, oldArr->cowCheck());
  if (lval.arr != oldArr) {
    base.type() = KindOfVec;
    base.val().parr = lval.arr;
    assertx(cellIsPlausible(base.tv()));
    decRefArr(oldArr);
  }
  return lval;
}

template <bool reffy>
inline tv_lval ElemDVecPre(tv_lval base, StringData* key) {
  throwInvalidArrayKeyException(key, base.val().parr);
}

template <bool reffy>
inline tv_lval ElemDVecPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (LIKELY(isIntType(dt))) return ElemDVecPre<reffy>(base, key.m_data.num);
  if (isStringType(dt))      return ElemDVecPre<reffy>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <bool reffy, KeyType keyType>
inline tv_lval ElemDVec(tv_lval base, key_type<keyType> key) {
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(base.tv()));
  auto const result = ElemDVecPre<reffy>(base, key);
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(base.tv()));

  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Dict
 */
template <bool reffy>
inline tv_lval ElemDDictPre(tv_lval base, int64_t key) {
  ArrayData* oldArr = base.val().parr;

  if (reffy) throwRefInvalidArrayValueException(oldArr);

  auto const lval =
    MixedArray::LvalSilentIntDict(oldArr, key, oldArr->cowCheck());

  if (UNLIKELY(!lval)) {
    assertx(oldArr == lval.arr);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (lval.arr != oldArr) {
    base.type() = KindOfDict;
    base.val().parr = lval.arr;
    assertx(cellIsPlausible(base.tv()));
    decRefArr(oldArr);
  }

  return lval;
}

template <bool reffy>
inline tv_lval ElemDDictPre(tv_lval base, StringData* key) {
  ArrayData* oldArr = base.val().parr;

  if (reffy) throwRefInvalidArrayValueException(oldArr);

  auto const lval =
    MixedArray::LvalSilentStrDict(oldArr, key, oldArr->cowCheck());

  if (UNLIKELY(!lval)) {
    assertx(oldArr == lval.arr);
    throwOOBArrayKeyException(key, oldArr);
  }

  if (lval.arr != oldArr) {
    base.type() = KindOfDict;
    base.val().parr = lval.arr;
    assertx(cellIsPlausible(base.tv()));
    decRefArr(oldArr);
  }

  return lval;
}

template <bool reffy>
inline tv_lval ElemDDictPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    return ElemDDictPre<reffy>(base, key.m_data.num);
  if (isStringType(dt)) return ElemDDictPre<reffy>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <bool reffy, KeyType keyType>
inline tv_lval ElemDDict(tv_lval base, key_type<keyType> key) {
  assertx(isDictType(base.type()));
  assertx(tvIsPlausible(base.tv()));
  auto result = ElemDDictPre<reffy>(base, key);
  assertx(isDictType(base.type()));
  assertx(tvIsPlausible(base.tv()));

  assertx(result.type() != KindOfUninit);
  return result;
}

/**
 * ElemD when base is a Keyset
 */
template <bool reffy>
[[noreturn]]
inline tv_lval ElemDKeysetPre(tv_lval base, int64_t /*key*/) {
  if (reffy) throwRefInvalidArrayValueException(base.val().parr);
  throwInvalidKeysetOperation();
}

template <bool reffy>
[[noreturn]]
inline tv_lval ElemDKeysetPre(tv_lval base, StringData* /*key*/) {
  if (reffy) throwRefInvalidArrayValueException(base.val().parr);
  throwInvalidKeysetOperation();
}

template <bool reffy>
[[noreturn]]
inline tv_lval ElemDKeysetPre(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt))    ElemDKeysetPre<reffy>(base, key.m_data.num);
  if (isStringType(dt)) ElemDKeysetPre<reffy>(base, key.m_data.pstr);
  throwInvalidArrayKeyException(&key, base.val().parr);
}

template <bool reffy, KeyType keyType>
[[noreturn]]
inline tv_lval ElemDKeyset(tv_lval base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(base.tv()));
  ElemDKeysetPre<reffy>(base, key);
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

  tvMove(make_tv<KindOfArray>(ArrayData::Create()), base);

  auto const result = asArrRef(base).lvalAt(cellAsCVarRef(scratchKey));
  if (mode == MOpMode::Warn) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&scratchKey).toString().data());
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
 * ElemD when base is an Object
 */
template<MOpMode mode, bool reffy, KeyType keyType>
inline tv_lval ElemDObject(TypedValue& tvRef, tv_lval base,
                           key_type<keyType> key) {
  auto scratchKey = initScratchKey(key);
  auto obj = base.val().pobj;

  if (LIKELY(obj->isCollection())) {
    if (reffy) {
      raise_error("Collection elements cannot be taken by reference");
      return tv_lval(nullptr);
    }
    return collections::atLval(obj, &scratchKey);
  } else if (obj->getVMClass()->classof(SystemLib::s_ArrayObjectClass)) {
    auto storage = obj->getPropLval(SystemLib::s_ArrayObjectClass,
                                    s_storage.get());
    // ArrayObject should always have the 'storage' property...
    assertx(storage);
    // We shouldn't have a type-hint on the storage property.
    assertx(
      !obj->getVMClass()->declPropTypeConstraint(
        obj->getVMClass()->getDeclPropIndex(SystemLib::s_ArrayObjectClass,
                                            s_storage.get()).slot
      ).isCheckable()
    );
    return UNLIKELY(checkHACIntishCast())
      ? ElemDArray<mode, reffy, true, keyType>(storage, key)
      : ElemDArray<mode, reffy, false, keyType>(storage, key);
  }


  tvRef = objOffsetGet(instanceFromTv(base), scratchKey);
  return tv_lval(&tvRef);
}

/*
 * Intermediate elem operation for defining member instructions.
 *
 * Returned pointer is not yet unboxed.  (I.e. it cannot point into a RefData.)
 */
template<MOpMode mode, bool reffy, bool intishWarn,
         KeyType keyType = KeyType::Any>
tv_lval ElemD(TypedValue& tvRef, tv_lval base,
              key_type<keyType> key, const MInstrPropState* pState) {
  assertx(mode == MOpMode::Define);

  base = base.unboxed();
  assertx(cellIsPlausible(base.tv()));

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
      return ElemDVec<reffy, keyType>(base, key);
    case KindOfPersistentDict:
    case KindOfDict:
      return ElemDDict<reffy, keyType>(base, key);
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return ElemDKeyset<reffy, keyType>(base, key);
    case KindOfPersistentArray:
    case KindOfArray:
      return ElemDArray<mode, reffy, intishWarn, keyType>(base, key);
    case KindOfObject:
      return ElemDObject<mode, reffy, keyType>(tvRef, base, key);
    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

/*
 * Implementation for SetWithRef*ML bytecodes.
 */
template<MOpMode mode, bool reffy, bool intishWarn,
         KeyType keyType = KeyType::Any>
void SetWithRefMLElem(TypedValue& tvRef, tv_lval base,
                      key_type<keyType> key, TypedValue val,
                      const MInstrPropState* pState) {
  assertx(mode == MOpMode::Define);

  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  auto const elem = [&] {
    switch (type(base)) {
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
        return ElemDVec<reffy, keyType>(base, key);
      case KindOfPersistentDict:
      case KindOfDict:
        return ElemDDict<reffy, keyType>(base, key);
      case KindOfPersistentKeyset:
      case KindOfKeyset:
        return ElemDKeyset<reffy, keyType>(base, key);
      case KindOfPersistentArray:
      case KindOfArray: {
        // We want to notice for binding assignments here, but not for missing
        // index, since we're about to initialize the value in that case.
        // Rather than fork the Lval API to have warn and no-warn flavors, we
        // instead lift the binding assignment warning here, and then disable
        // Hack array notices.
        if (reffy && checkHACRefBind() &&
            !HPHP::val(base).parr->isGlobalsArray()) {
          raiseHackArrCompatRefBind(key);
        }
        SuppressHackArrCompatNotices shacn;
        return ElemDArray<mode, reffy, intishWarn, keyType>(base, key);
      }
      case KindOfObject:
        return ElemDObject<mode, reffy, keyType>(tvRef, base, key);
    case KindOfRef:
        break;
    }
    unknownBaseType(type(base));
  }();

  // Intentionally leak the old value pointed to by elem, including from magic
  // methods.
  tvDup(val, elem);
}

/**
 * ElemU when base is Null
 */
inline tv_lval ElemUEmptyish() {
  return const_cast<TypedValue*>(&immutable_null_base);
}

template <bool intishWarn>
inline tv_lval ElemUArrayImpl(tv_lval base, int64_t key) {
  auto oldArr = val(base).parr;
  if (!oldArr->exists(key)) return ElemUEmptyish();
  auto const lval = oldArr->lval(key, oldArr->cowCheck());
  if (lval.arr != oldArr) {
    type(base) = KindOfArray;
    val(base).parr = lval.arr;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
}

template <bool intishWarn>
inline tv_lval ElemUArrayImpl(tv_lval base, StringData* key) {
  auto oldArr = val(base).parr;
  int64_t n;
  if (oldArr->convertKey(key, n, intishWarn)) {
    if (!oldArr->exists(n)) return ElemUEmptyish();
    auto const lval = oldArr->lval(n, oldArr->cowCheck());
    if (lval.arr != oldArr) {
      type(base) = KindOfArray;
      val(base).parr = lval.arr;
      assertx(cellIsPlausible(*base));
      decRefArr(oldArr);
    }
    return lval;
  } else {
    if (!oldArr->exists(key)) return ElemUEmptyish();
    auto const lval = oldArr->lval(key, oldArr->cowCheck());
    if (lval.arr != oldArr) {
      type(base) = KindOfArray;
      val(base).parr = lval.arr;
      assertx(cellIsPlausible(*base));
      decRefArr(oldArr);
    }
    return lval;
  }
}

template <bool intishWarn>
inline tv_lval ElemUArrayImpl(tv_lval base, TypedValue key) {
  auto const dt = key.m_type;
  if (isIntType(dt)) {
    return ElemUArrayImpl<false>(base, key.m_data.num);
  }
  if (isStringType(dt)) {
    return ElemUArrayImpl<intishWarn>(base, key.m_data.pstr);
  }
  auto& arr = asArrRef(base);
  if (!arr.exists(keyAsValue(key))) {
    return ElemUEmptyish();
  }
  return arr.lvalAt(tvAsCVarRef(&key));
}

/**
 * ElemU when base is an Array
 */
template <bool intishWarn, KeyType keyType>
inline tv_lval ElemUArray(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  auto lval = ElemUArrayImpl<intishWarn>(base, key);
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  assertx(lval.type() != KindOfUninit);
  return lval;
}

/**
 * ElemU when base is a Vec
 */
inline tv_lval ElemUVecPre(tv_lval base, int64_t key) {
  ArrayData* oldArr = val(base).parr;
  auto const lval =
    PackedArray::LvalSilentIntVec(oldArr, key, oldArr->cowCheck());

  if (UNLIKELY(!lval)) {
    return ElemUEmptyish();
  }
  if (lval.arr != oldArr) {
    type(base) = KindOfVec;
    val(base).parr = lval.arr;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
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
  auto const lval =
    MixedArray::LvalSilentIntDict(oldArr, key, oldArr->cowCheck());

  if (UNLIKELY(!lval)) {
    return ElemUEmptyish();
  }
  if (lval.arr != oldArr) {
    type(base) = KindOfDict;
    val(base).parr = lval.arr;
    assertx(cellIsPlausible(*base));
    decRefArr(oldArr);
  }
  return lval;
}

inline tv_lval ElemUDictPre(tv_lval base, StringData* key) {
  ArrayData* oldArr = val(base).parr;
  auto const lval =
    MixedArray::LvalSilentStrDict(oldArr, key, oldArr->cowCheck());

  if (UNLIKELY(!lval)) {
    return ElemUEmptyish();
  }
  if (lval.arr != oldArr) {
    type(base) = KindOfDict;
    val(base).parr = lval.arr;
    assertx(cellIsPlausible(*base));
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
  auto const scratchKey = initScratchKey(key);
  if (LIKELY(val(base).pobj->isCollection())) {
    return collections::atLval(val(base).pobj, &scratchKey);
  }
  tvRef = objOffsetGet(instanceFromTv(base), scratchKey);
  return &tvRef;
}

/*
 * Intermediate Elem operation for an unsetting member instruction.
 *
 * Returned pointer is not yet unboxed.  (I.e. it cannot point into a RefData.)
 */
template <bool intishWarn, KeyType keyType = KeyType::Any>
tv_lval ElemU(TypedValue& tvRef, tv_lval base, key_type<keyType> key) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      // Unset on scalar base never modifies the base, but the const_cast is
      // necessary to placate the type system.
      return const_cast<TypedValue*>(&immutable_uninit_base);
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
      return ElemUArray<intishWarn, keyType>(base, key);
    case KindOfObject:
      return ElemUObject<keyType>(tvRef, base, key);
    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

/**
 * NewElem when base is Null
 */
inline tv_lval NewElemEmptyish(tv_lval base, const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  tvMove(make_tv<KindOfArray>(ArrayData::Create()), base);
  return asArrRef(base).lvalAt();
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
template <bool reffy>
inline tv_lval NewElemArray(tv_lval base) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  return reffy ? asArrRef(base).lvalAtRef() : asArrRef(base).lvalAt();
}

/**
 * NewElem when base is an Object
 */
inline tv_lval NewElemObject(TypedValue& tvRef, tv_lval base) {
  if (val(base).pobj->isCollection()) {
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
inline tv_lval NewElem(TypedValue& tvRef,
                       tv_lval base,
                       const MInstrPropState* pState) {
  base = base.unboxed();
  assertx(cellIsPlausible(base.tv()));

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
    case KindOfPersistentArray:
    case KindOfArray:
      return NewElemArray<reffy>(base);
    case KindOfObject:
      return NewElemObject(tvRef, base);
    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

/**
 * SetElem when base is Null
 */
template <KeyType keyType>
inline void SetElemEmptyish(tv_lval base, key_type<keyType> key,
                            Cell* value, const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  auto const& scratchKey = initScratchKey(key);
  cellMove(make_tv<KindOfArray>(staticEmptyArray()), base);
  asArrRef(base).set(tvAsCVarRef(&scratchKey), tvAsCVarRef(value));
}

/**
 * SetElem when base is an Int64, Double, Resource, Func, or Class.
 */
template <bool setResult>
inline void SetElemScalar(Cell* value) {
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
                           Cell* value, const MInstrPropState* pState) {
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
inline StringData* SetElemString(tv_lval base, key_type<keyType> key,
                                 Cell* value, const MInstrPropState* pState) {
  int baseLen = val(base).pstr->size();
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
  assert(x >= 0); // x < 0 is handled above.
  if (x < baseLen && !val(base).pstr->cowCheck()) {
    // Modify base in place.  This is safe because the LHS owns the
    // only reference.
    auto const oldp = val(base).pstr;
    auto const newp = oldp->modifyChar(x, y);
    if (UNLIKELY(newp != oldp)) {
      decRefStr(oldp);
      val(base).pstr = newp;
      type(base) = KindOfString;
    }
  } else {
    StringData* sd = StringData::Make(slen);
    char* s = sd->mutableData();
    memcpy(s, val(base).pstr->data(), baseLen);
    if (x > baseLen) {
      memset(&s[baseLen], ' ', slen - baseLen - 1);
    }
    s[x] = y;
    sd->setSize(slen);
    decRefStr(val(base).pstr);
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
                          Cell* value) {
  auto const scratchKey = initScratchKey(key);
  if (LIKELY(val(base).pobj->isCollection())) {
    collections::set(val(base).pobj, &scratchKey, value);
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
  static void do_return(ArrayData* /*a*/) {}
};

template<> struct ShuffleReturn<false> {
  static ArrayData* do_return(ArrayData* a) { return a; }
};

template<bool setRef, DataType dt> inline
auto arrayRefShuffle(ArrayData* oldData, ArrayData* newData, tv_lval base) {
  if (newData == oldData) {
    return ShuffleReturn<setRef>::do_return(oldData);
  }

  if (setRef) {
    if (isArrayLikeType(type(base)) && val(base).parr == oldData) {
      type(base) = dt;
      val(base).parr = newData;
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
template<bool setResult, bool intishWarn>
inline ArrayData* SetElemArrayPre(ArrayData* a,
                                  int64_t key,
                                  Cell* value,
                                  bool copy) {
  return a->set(key, *value, copy);
}

/**
 * SetElem helper with Array base and String key
 */
template<bool setResult, bool intishWarn>
inline ArrayData* SetElemArrayPre(ArrayData* a,
                                  StringData* key,
                                  Cell* value,
                                  bool copy) {
  int64_t n;
  assertx(a->isPHPArray());
  if (key->isStrictlyInteger(n)) {
    if (intishWarn) raise_intish_index_cast();
    return a->set(n, *value, copy);
  } else {
    return a->set(key, *value, copy);
  }
}

template<bool setResult, bool intishWarn>
inline ArrayData* SetElemArrayPre(ArrayData* a,
                                  TypedValue key,
                                  Cell* value,
                                  bool copy) {
  if (isStringType(key.m_type)) {
    return SetElemArrayPre<setResult, intishWarn>(
      a, key.m_data.pstr, value, copy
    );
  }
  if (key.m_type == KindOfInt64) {
    return SetElemArrayPre<setResult, false>(
      a, key.m_data.num, value, copy
    );
  }
  if (isFuncType(key.m_type)) {
    return SetElemArrayPre<setResult, intishWarn>(
      a, const_cast<StringData*>(funcToStringHelper(key.m_data.pfunc)), value,
      copy
    );
  }
  if (checkHACMisc()) {
    raiseHackArrCompatImplicitArrayKey(&key);
  }
  if (isNullType(key.m_type)) {
    return a->set(staticEmptyString(), *value, copy);
  }
  if (!isArrayLikeType(key.m_type) && key.m_type != KindOfObject) {
    return SetElemArrayPre<setResult, false>(a, tvAsCVarRef(&key).toInt64(),
                                             value, copy);
  }

  raise_warning("Illegal offset type");
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
template <bool setResult, KeyType keyType, bool intishWarn>
inline void SetElemArray(tv_lval base, key_type<keyType> key,
                         Cell* value) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  bool copy = a->cowCheck() ||
    (tvIsArray(value) && value->m_data.parr == a);

  auto* newData = SetElemArrayPre<setResult, intishWarn>(a, key, value, copy);
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

template <bool setResult>
inline ArrayData*
SetElemVecPre(ArrayData* a, StringData* key, Cell* /*value*/, bool /*copy*/) {
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
inline void SetElemVec(tv_lval base, key_type<keyType> key,
                       Cell* value) {
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  bool copy = a->cowCheck() || (tvIsVec(value) && value->m_data.parr == a);

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
inline void SetElemDict(tv_lval base, key_type<keyType> key,
                        Cell* value) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));

  ArrayData* a = val(base).parr;
  bool copy = a->cowCheck() ||
    (tvIsDict(value) && value->m_data.parr == a);

  auto newData = SetElemDictPre<setResult>(a, key, value, copy);
  assertx(newData->isDict());

  arrayRefShuffle<true, KindOfDict>(a, newData, base);
}

/**
 * SetElem() leaves the result in 'value', rather than returning it as in
 * SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
 * get around.
 */
template <bool setResult, KeyType keyType, bool intishWarn>
NEVER_INLINE
StringData* SetElemSlow(tv_lval base,
                        key_type<keyType> key,
                        Cell* value,
                        const MInstrPropState* pState) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

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
      SetElemArray<setResult, keyType, intishWarn>(base, key, value);
      return nullptr;
    case KindOfObject:
      SetElemObject<keyType>(base, key, value);
      return nullptr;
    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

/**
 * Fast path for SetElem assuming base is an Array
 */
template <bool setResult, bool intishWarn, KeyType keyType = KeyType::Any>
inline StringData* SetElem(tv_lval base, key_type<keyType> key,
                           Cell* value, const MInstrPropState* pState) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    SetElemArray<setResult, keyType, intishWarn>(base, key, value);
    return nullptr;
  }
  if (LIKELY(tvIsVec(base))) {
    SetElemVec<setResult, keyType>(base, key, value);
    return nullptr;
  }
  if (LIKELY(tvIsDict(base))) {
    SetElemDict<setResult, keyType>(base, key, value);
    return nullptr;
  }
  return SetElemSlow<setResult, keyType, intishWarn>(base, key, value, pState);
}

/**
 * SetNewElem when base is Null
 */
inline void SetNewElemEmptyish(tv_lval base,
                               Cell* value,
                               const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  Array a = Array::Create();
  a.append(cellAsCVarRef(*value));
  cellMove(make_tv<KindOfArray>(a.detach()), base);
}

/**
 * SetNewElem when base is Int64, Double, Resource, Func or Class
 */
template <bool setResult>
inline void SetNewElemScalar(Cell* value) {
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
                              Cell* value,
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
                             Cell* value,
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
inline void SetNewElemArray(tv_lval base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto const copy = a->cowCheck() ||
    (tvIsArray(value) && value->m_data.parr == a);
  auto a2 = a->append(*value, copy);
  if (a2 != a) {
    type(base) = KindOfArray;
    val(base).parr = a2;
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Vec
 */
inline void SetNewElemVec(tv_lval base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto const copy = a->cowCheck() ||
    (tvIsVec(value) && value->m_data.parr == a);
  auto a2 = PackedArray::AppendVec(a, *value, copy);
  if (a2 != a) {
    type(base) = KindOfVec;
    val(base).parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Dict
 */
inline void SetNewElemDict(tv_lval base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto const copy = a->cowCheck() ||
    (tvIsDict(value) && value->m_data.parr == a);
  auto a2 = MixedArray::AppendDict(a, *value, copy);
  if (a2 != a) {
    type(base) = KindOfDict;
    val(base).parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is a Keyset
 */
inline void SetNewElemKeyset(tv_lval base, Cell* value) {
  base = tvToCell(base);
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  auto a = val(base).parr;
  auto const copy = a->cowCheck() ||
    (tvIsKeyset(value) && value->m_data.parr == a);
  auto a2 = SetArray::Append(a, *value, copy);
  if (a2 != a) {
    type(base) = KindOfKeyset;
    val(base).parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is an Object
 */
inline void SetNewElemObject(tv_lval base, Cell* value) {
  if (LIKELY(val(base).pobj->isCollection())) {
    collections::append(val(base).pobj, value);
  } else {
    objOffsetAppend(instanceFromTv(base), value);
  }
}

/**
 * $base[] = ...
 */
template <bool setResult>
inline void SetNewElem(tv_lval base,
                       Cell* value,
                       const MInstrPropState* pState) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

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
      break;
  }
  unknownBaseType(type(base));
}

/**
 * SetOpElem when base is Null
 */
inline tv_lval SetOpElemEmptyish(SetOpOp op, tv_lval base,
                                 TypedValue key, Cell* rhs,
                                 const MInstrPropState* pState) {
  assertx(cellIsPlausible(*base));

  detail::checkPromotion(base, pState);

  cellMove(make_tv<KindOfArray>(staticEmptyArray()), base);
  auto const lval = asArrRef(base).lvalAt(tvAsCVarRef(&key));
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&key).toString().data());
  }
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
template <bool intishWarn>
inline tv_lval SetOpElem(TypedValue& tvRef,
                         SetOpOp op, tv_lval base,
                         TypedValue key, Cell* rhs,
                         const MInstrPropState* pState) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

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
    case KindOfVec: {
      auto result = ElemDVec<false, KeyType::Any>(base, key);
      result = tvAssertCell(result);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      auto result = ElemDDict<false, KeyType::Any>(base, key);
      result = tvAssertCell(result);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throwInvalidKeysetOperation();

    case KindOfPersistentArray:
    case KindOfArray: {
      if (UNLIKELY(
            checkHACFalseyPromote() &&
            !asCArrRef(base).exists(tvAsCVarRef(&key))
          )) {
        raiseHackArrCompatMissingSetOp();
      }
      auto constexpr mode = MoreWarnings ? MOpMode::Warn : MOpMode::None;
      auto result =
        ElemDArray<mode, false, intishWarn, KeyType::Any>(base, key);
      result = tvToCell(result);
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfObject: {
      TypedValue* result;
      if (LIKELY(val(base).pobj->isCollection())) {
        result = collections::atRw(val(base).pobj, &key);
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
      break;
  }
  unknownBaseType(type(base));
}

inline tv_lval SetOpNewElemEmptyish(SetOpOp op, tv_lval base, Cell* rhs,
                                    const MInstrPropState* pState) {
  detail::checkPromotion(base, pState);
  cellMove(make_tv<KindOfArray>(staticEmptyArray()), base);
  auto result = asArrRef(base).lvalAt();
  setopBody(tvToCell(result), op, rhs);
  return result;
}
inline tv_lval SetOpNewElemScalar(TypedValue& tvRef) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(tvRef);
  return &tvRef;
}
inline tv_lval SetOpNewElem(TypedValue& tvRef,
                            SetOpOp op, tv_lval base,
                            Cell* rhs, const MInstrPropState* pState) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

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

    case KindOfPersistentArray:
    case KindOfArray: {
      auto result = asArrRef(base).lvalAt();
      setopBody(result, op, rhs);
      return result;
    }

    case KindOfObject: {
      TypedValue* result;
      if (val(base).pobj->isCollection()) {
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
      break;
  }
  unknownBaseType(type(base));
}

NEVER_INLINE Cell incDecBodySlow(IncDecOp op, tv_lval fr);

inline Cell IncDecBody(IncDecOp op, tv_lval fr) {
  assertx(cellIsPlausible(*fr));

  if (UNLIKELY(!isIntType(type(fr)))) {
    return incDecBodySlow(op, fr);
  }

  // fast cases, assuming integers overflow to ints
  switch (op) {
  case IncDecOp::PreInc:
    ++val(fr).num;
    return *fr;
  case IncDecOp::PostInc: {
    auto const tmp = *fr;
    ++val(fr).num;
    return tmp;
  }
  case IncDecOp::PreDec:
    --val(fr).num;
    return *fr;
  case IncDecOp::PostDec: {
    auto const tmp = *fr;
    --val(fr).num;
    return tmp;
  }
  default:
    return incDecBodySlow(op, fr);
  }
}

inline Cell IncDecElemEmptyish(
  IncDecOp op,
  tv_lval base,
  TypedValue key,
  const MInstrPropState* pState
) {
  detail::checkPromotion(base, pState);

  cellMove(make_tv<KindOfArray>(staticEmptyArray()), base);
  auto const lval = asArrRef(base).lvalAt(tvAsCVarRef(&key));
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&key).toString().data());
  }
  assertx(type(lval) == KindOfNull);
  return IncDecBody(op, lval);
}

inline Cell IncDecElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

template <bool intishWarn>
inline Cell IncDecElem(
  IncDecOp op,
  tv_lval base,
  TypedValue key,
  const MInstrPropState* pState
) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

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
      if (UNLIKELY(
            checkHACFalseyPromote() &&
            !asCArrRef(base).exists(tvAsCVarRef(&key))
          )) {
        raiseHackArrCompatMissingIncDec();
      }
      auto constexpr mode = MoreWarnings ? MOpMode::Warn : MOpMode::None;
      auto result =
        ElemDArray<mode, false, intishWarn, KeyType::Any>(base, key);
      return IncDecBody(op, tvToCell(result));
    }

    case KindOfObject: {
      tv_lval result;
      auto localTvRef = make_tv<KindOfUninit>();

      if (LIKELY(val(base).pobj->isCollection())) {
        result = collections::atRw(val(base).pobj, &key);
        assertx(cellIsPlausible(*result));
      } else {
        localTvRef = objOffsetGet(instanceFromTv(base), key);
        result = tvToCell(&localTvRef);
      }

      auto const dest = IncDecBody(op, result);
      tvDecRefGen(localTvRef);
      return dest;
    }

    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

inline Cell IncDecNewElemEmptyish(
  IncDecOp op,
  tv_lval base,
  const MInstrPropState* pState
) {
  detail::checkPromotion(base, pState);
  cellMove(make_tv<KindOfArray>(staticEmptyArray()), base);
  auto result = asArrRef(base).lvalAt();
  assertx(type(result) == KindOfNull);
  return IncDecBody(op, result);
}

inline Cell IncDecNewElemScalar() {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  return make_tv<KindOfNull>();
}

inline Cell IncDecNewElem(
  TypedValue& tvRef,
  IncDecOp op,
  tv_lval base,
  const MInstrPropState* pState
) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

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

    case KindOfPersistentArray:
    case KindOfArray: {
      auto result = asArrRef(base).lvalAt();
      assertx(type(result) == KindOfNull);
      return IncDecBody(op, result);
    }

    case KindOfObject: {
      if (val(base).pobj->isCollection()) {
        throw_cannot_use_newelem_for_lval_read_col();
      }
      tvRef = objOffsetGet(instanceFromTv(base), make_tv<KindOfNull>());
      auto result = tvToCell(&tvRef);
      return IncDecBody(op, result);
    }

    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

/**
 * UnsetElemArray when key is an Int64
 */
template <bool intishWarn>
inline ArrayData* UnsetElemArrayPre(ArrayData* a, int64_t key,
                                    bool copy) {
  return a->remove(key, copy);
}

/**
 * UnsetElemArray when key is a String
 */
template <bool intishWarn>
inline ArrayData* UnsetElemArrayPre(ArrayData* a, StringData* key,
                                    bool copy) {
  int64_t n;
  assertx(a->isPHPArray());
  if (key->isStrictlyInteger(n)) {
    if (intishWarn) raise_intish_index_cast();
    return a->remove(n, copy);
  } else {
    return a->remove(key, copy);
  }
}

template <bool intishWarn>
inline ArrayData* UnsetElemArrayPre(ArrayData* a, TypedValue key,
                                    bool copy) {
  if (isStringType(key.m_type)) {
    return UnsetElemArrayPre<intishWarn>(a, key.m_data.pstr, copy);
  }
  if (key.m_type == KindOfInt64) {
    return UnsetElemArrayPre<false>(a, key.m_data.num, copy);
  }
  if (isFuncType(key.m_type)) {
    return UnsetElemArrayPre<intishWarn>(
      a, const_cast<StringData*>(funcToStringHelper(key.m_data.pfunc)), copy
    );
  }
  auto const k = tvToKey(key, a);
  if (isNullType(k.m_type)) return a;
  return a->remove(k, copy);
}

/**
 * UnsetElem when base is an Array
 */
template <KeyType keyType, bool intishWarn>
inline void UnsetElemArray(tv_lval base, key_type<keyType> key) {
  assertx(tvIsArray(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemArrayPre<intishWarn>(a, key, a->cowCheck());

  if (a2 != a) {
    type(base) = KindOfArray;
    val(base).parr = a2;
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

inline ArrayData*
UnsetElemVecPre(ArrayData* a, StringData* /*key*/, bool /*copy*/) {
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
inline void UnsetElemVec(tv_lval base, key_type<keyType> key) {
  assertx(tvIsVec(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemVecPre(a, key, a->cowCheck());
  assertx(a2->isVecArray() || a2->isDict());

  if (a2 != a) {
    type(base) = a2->toDataType();
    val(base).parr = a2;
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
inline void UnsetElemDict(tv_lval base, key_type<keyType> key) {
  assertx(tvIsDict(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemDictPre(a, key, a->cowCheck());

  if (a2 != a) {
    type(base) = KindOfDict;
    val(base).parr = a2;
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
inline void UnsetElemKeyset(tv_lval base, key_type<keyType> key) {
  assertx(tvIsKeyset(base));
  assertx(tvIsPlausible(*base));
  ArrayData* a = val(base).parr;
  ArrayData* a2 = UnsetElemKeysetPre(a, key, a->cowCheck());

  if (a2 != a) {
    type(base) = KindOfKeyset;
    val(base).parr = a2;
    assertx(cellIsPlausible(*base));
    a->decRefAndRelease();
 }
}

/**
 * unset($base[$member])
 */
template <KeyType keyType, bool intishWarn>
NEVER_INLINE
void UnsetElemSlow(tv_lval base, key_type<keyType> key) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

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

    case KindOfPersistentArray:
    case KindOfArray:
      UnsetElemArray<keyType, intishWarn>(base, key);
      return;

    case KindOfObject: {
      auto const& scratchKey = initScratchKey(key);
      if (LIKELY(val(base).pobj->isCollection())) {
        collections::unset(val(base).pobj, &scratchKey);
      } else {
        objOffsetUnset(instanceFromTv(base), scratchKey);
      }
      return;
    }

    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

/**
 * Fast path for UnsetElem assuming base is an Array
 */
template <bool intishWarn, KeyType keyType = KeyType::Any>
inline void UnsetElem(tv_lval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    return UnsetElemArray<keyType, intishWarn>(base, key);
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
  return UnsetElemSlow<keyType, intishWarn>(base, key);
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
bool IssetEmptyElemString(const StringData* sd, key_type<keyType> key) {
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
  if (x < 0 || x >= sd->size()) {
    return useEmpty;
  }
  if (!useEmpty) {
    return true;
  }

  auto str = sd->getChar(x);
  assertx(str->isStatic());
  return !str->toBoolean();
}

/**
 * IssetEmptyElem when base is an Array
 */
template <bool useEmpty, KeyType keyType, bool intishWarn>
bool IssetEmptyElemArray(ArrayData* a, key_type<keyType> key) {
  assertx(a->isPHPArray());
  auto const result = ElemArray<MOpMode::None, keyType, intishWarn>(a, key);
  if (useEmpty) {
    return !cellToBool(tvToCell(result.tv()));
  }
  return !cellIsNull(tvToCell(result.tv()));
}

/**
 * IssetEmptyElem when base is a Vec
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemVec(ArrayData* a, key_type<keyType> key) {
  assertx(a->isVecArray());
  auto const result = ElemVec<MOpMode::None, keyType>(a, key);
  if (useEmpty) {
    return !cellToBool(tvAssertCell(result.tv()));
  }
  return !cellIsNull(tvAssertCell(result.tv()));
}

/**
 * IssetEmptyElem when base is a Dict
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemDict(ArrayData* a, key_type<keyType> key) {
  assertx(a->isDict());
  auto const result = ElemDict<MOpMode::None, keyType>(a, key);
  if (useEmpty) {
    return !cellToBool(tvAssertCell(result.tv()));
  }
  return !cellIsNull(tvAssertCell(result.tv()));
}

/**
 * IssetEmptyElem when base is a Keyset
 */
template <bool useEmpty, KeyType keyType>
bool IssetEmptyElemKeyset(ArrayData* a, key_type<keyType> key) {
  assertx(a->isKeyset());
  auto const result = ElemKeyset<MOpMode::None, keyType>(a, key);
  if (useEmpty) {
    return !cellToBool(tvAssertCell(result.tv()));
  }
  return !cellIsNull(tvAssertCell(result.tv()));
}

/**
 * isset/empty($base[$key])
 */
template <bool useEmpty, KeyType keyType, bool intishWarn>
NEVER_INLINE bool IssetEmptyElemSlow(tv_rval base, key_type<keyType> key) {
  base = tvToCell(base);
  assertx(cellIsPlausible(*base));

  switch (type(base)) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfResource:
      return useEmpty;

    case KindOfFunc:
      return IssetEmptyElemString<useEmpty, keyType>(
        funcToStringHelper(val(base).pfunc), key
      );

    case KindOfClass:
      return IssetEmptyElemString<useEmpty, keyType>(
        classToStringHelper(val(base).pclass), key
      );

    case KindOfPersistentString:
    case KindOfString:
      return IssetEmptyElemString<useEmpty, keyType>(val(base).pstr, key);

    case KindOfPersistentVec:
    case KindOfVec:
      return IssetEmptyElemVec<useEmpty, keyType>(val(base).parr, key);

    case KindOfPersistentDict:
    case KindOfDict:
      return IssetEmptyElemDict<useEmpty, keyType>(val(base).parr, key);

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return IssetEmptyElemKeyset<useEmpty, keyType>(val(base).parr, key);

    case KindOfPersistentArray:
    case KindOfArray:
      return IssetEmptyElemArray<useEmpty, keyType, intishWarn>(
        val(base).parr, key
      );

    case KindOfObject:
      return IssetEmptyElemObj<useEmpty, keyType>(val(base).pobj, key);

    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

template <bool useEmpty, bool intishWarn, KeyType keyType = KeyType::Any>
bool IssetEmptyElem(tv_rval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));

  if (LIKELY(tvIsArray(base))) {
    return IssetEmptyElemArray<useEmpty, keyType, intishWarn>(
      val(base).parr, key
    );
  }
  if (LIKELY(tvIsVec(base))) {
    return IssetEmptyElemVec<useEmpty, keyType>(val(base).parr, key);
  }
  if (LIKELY(tvIsDict(base))) {
    return IssetEmptyElemDict<useEmpty, keyType>(val(base).parr, key);
  }
  if (LIKELY(tvIsKeyset(base))) {
    return IssetEmptyElemKeyset<useEmpty, keyType>(val(base).parr, key);
  }
  return IssetEmptyElemSlow<useEmpty, keyType, intishWarn>(base, key);
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
                              bool warn,
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
      auto const tv = make_tv<KindOfObject>(obj.get());
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
    }
  }

  if (base.type() == KindOfString) {
    decRefStr(base.val().pstr);
  } else {
    assertx(!isRefcountedType(base.type()));
  }
  base.type() = KindOfObject;
  base.val().pobj = obj.get();

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
tv_lval propPreStdclass(TypedValue& tvRef,
                        tv_lval base,
                        MInstrPropState* pState) {
  if (mode != MOpMode::Define) {
    return propPreNull<mode>(tvRef, pState);
  }

  promoteToStdClass(base, RuntimeOption::EnableHipHopSyntax,
                    [] (ObjectData*) {}, pState);
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
  base = base.unboxed();
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

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      return propPreNull<mode>(tvRef, pState);

    case KindOfObject:
      return base;

    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

inline tv_lval nullSafeProp(TypedValue& tvRef,
                            Class* ctx,
                            tv_rval base,
                            StringData* key) {
  base = base.unboxed();
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfFunc:
    case KindOfClass:
      tvWriteNull(tvRef);
      raise_notice("Cannot access property on non-object");
      return &tvRef;
    case KindOfObject:
      return val(base).pobj->prop(&tvRef, ctx, key);
    case KindOfRef:
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
template<MOpMode mode, KeyType keyType = KeyType::Any, bool reffy = false>
inline tv_lval PropObj(TypedValue& tvRef, const Class* ctx,
                       ObjectData* instance, key_type<keyType> key,
                       MInstrPropState* pState) {
  auto keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Get property.
  if (mode == MOpMode::Define) {
    if (reffy) {
      return instance->propB(&tvRef, ctx, keySD, pState);
    } else {
      return instance->propD(&tvRef, ctx, keySD, pState);
    }
  }
  assertx(!reffy);
  if (mode == MOpMode::None) {
    return instance->prop(&tvRef, ctx, keySD);
  }
  if (mode == MOpMode::Warn) {
    return instance->propW(&tvRef, ctx, keySD);
  }
  assertx(mode == MOpMode::Unset);
  return instance->propU(&tvRef, ctx, keySD);
}

template<MOpMode mode, KeyType keyType = KeyType::Any, bool reffy = false>
inline tv_lval Prop(TypedValue& tvRef,
                    const Class* ctx,
                    tv_lval base,
                    key_type<keyType> key,
                    MInstrPropState* pState) {
  auto result = propPre<mode>(tvRef, base, pState);
  if (result.type() == KindOfNull) return result;

  return PropObj<mode,keyType,reffy>(
    tvRef, ctx, instanceFromTv(result), key, pState
  );
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
bool IssetEmptyProp(Class* ctx, tv_lval base, key_type<kt> key) {
  base = tvToCell(base);
  if (LIKELY(type(base) == KindOfObject)) {
    return IssetEmptyPropObj<useEmpty, kt>(ctx, instanceFromTv(base), key);
  }
  return useEmpty;
}

template <bool setResult>
inline void SetPropNull(Cell* val) {
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
                            Cell* val,
                            const MInstrPropState* pState) {
  promoteToStdClass(
    base,
    true,
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
                       key_type<keyType> key, Cell* val) {
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Set property.
  instance->setProp(ctx, keySD, *val);
}

// $base->$key = $val
template <bool setResult, KeyType keyType = KeyType::Any>
inline void SetProp(Class* ctx, tv_lval base, key_type<keyType> key,
                    Cell* val, const MInstrPropState* pState) {
  base = tvToCell(base);
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return SetPropNull<setResult>(val);

    case KindOfPersistentString:
    case KindOfString:
      if (HPHP::val(base).pstr->size() != 0) {
        return SetPropNull<setResult>(val);
      }
      return SetPropStdclass(base, initScratchKey(key), val, pState);

    case KindOfObject:
      return SetPropObj<keyType>(ctx, HPHP::val(base).pobj, key, val);

    case KindOfRef:
      break;
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
                                 Cell* rhs, const MInstrPropState* pState) {
  promoteToStdClass(
    base,
    true,
    [&] (ObjectData* obj) {
      StringData* keySD = prepareKey(key);
      SCOPE_EXIT { decRefStr(keySD); };
      tvWriteNull(tvRef);
      setopBody(tvAssertCell(&tvRef), op, rhs);
      obj->setProp(nullptr, keySD, tvAssertCell(tvRef));
    },
    pState
  );

  return &tvRef;
}

inline tv_lval SetOpPropObj(TypedValue& tvRef, Class* ctx,
                            SetOpOp op, ObjectData* instance,
                            TypedValue key, Cell* rhs) {
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  return instance->setOpProp(tvRef, ctx, op, keySD, rhs);
}

// $base->$key <op>= $rhs
inline tv_lval SetOpProp(TypedValue& tvRef,
                         Class* ctx, SetOpOp op,
                         tv_lval base, TypedValue key,
                         Cell* rhs, const MInstrPropState* pState) {
  base = tvToCell(base);
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return SetOpPropNull(tvRef);

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        return SetOpPropNull(tvRef);
      }
      return SetOpPropStdclass(tvRef, op, base, key, rhs, pState);

    case KindOfObject:
      return SetOpPropObj(tvRef, ctx, op, instanceFromTv(base), key, rhs);

    case KindOfRef:
      break;
  }
  unknownBaseType(type(base));
}

inline Cell IncDecPropNull() {
  raise_warning("Attempt to increment/decrement property of non-object");
  return make_tv<KindOfNull>();
}

inline Cell IncDecPropStdclass(IncDecOp op, tv_lval base,
                               TypedValue key, const MInstrPropState* pState) {
  Cell dest;
  promoteToStdClass(
    base,
    true,
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
  tv_lval base,
  TypedValue key,
  const MInstrPropState* pState
) {
  base = tvToCell(base);
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
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return IncDecPropNull();

    case KindOfPersistentString:
    case KindOfString:
      if (val(base).pstr->size() != 0) {
        return IncDecPropNull();
      }
      return IncDecPropStdclass(op, base, key, pState);

    case KindOfObject:
      return IncDecPropObj(ctx, op, instanceFromTv(base), key);

    case KindOfRef:
      break;
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
  base = tvToCell(base);
  if (LIKELY(type(base) == KindOfObject)) {
    UnsetPropObj(ctx, instanceFromTv(base), key);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_VM_MEMBER_OPERATIONS_H_
