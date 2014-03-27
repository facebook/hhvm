/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/base/tv-conversions.h"

namespace HPHP {

const StaticString s_storage("storage");

class InvalidSetMException : public std::runtime_error {
 public:
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
  const TypedValue m_tv;
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
inline const TypedValue& initScratchKey(const TypedValue& tv) {
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
TypedValue* objOffsetGet(TypedValue& tvRef, ObjectData* base,
                         const Variant& offset, bool validate=true);
bool objOffsetIsset(TypedValue& tvRef, ObjectData* base, const Variant& offset,
                    bool validate=true);
bool objOffsetEmpty(TypedValue& tvRef, ObjectData* base, const Variant& offset,
                    bool validate=true);
void objOffsetSet(ObjectData* base, const Variant& offset, TypedValue* val,
                  bool validate=true);
void objOffsetAppend(ObjectData* base, TypedValue* val, bool validate=true);
void objOffsetUnset(ObjectData* base, const Variant& offset);

void throw_cannot_use_newelem_for_lval_read() ATTRIBUTE_NORETURN;

// Post: base is a Cell*
ALWAYS_INLINE void opPre(TypedValue*& base, DataType& type) {
  // Get inner variant if necessary.
  type = base->m_type;
  if (type == KindOfRef) {
    base = base->m_data.pref->tv();
    type = base->m_type;
  }

  assert(cellIsPlausible(*base));
}

inline TypedValue* ElemArrayPre(ArrayData* base, int64_t key) {
  TypedValue* result = base->nvGet(key);
  return result ? result : const_cast<TypedValue*>(null_variant.asTypedValue());
}

inline TypedValue* ElemArrayPre(ArrayData* base, StringData* key) {
  int64_t n;
  TypedValue* result = !key->isStrictlyInteger(n) ? base->nvGet(key) :
                       base->nvGet(n);
  return result ? result : const_cast<TypedValue*>(null_variant.asTypedValue());
}

inline TypedValue* ElemArrayPre(ArrayData* base, TypedValue key) {
  auto dt = key.m_type;
  if (dt == KindOfInt64)  return ElemArrayPre(base, key.m_data.num);
  if (IS_STRING_TYPE(dt)) return ElemArrayPre(base, key.m_data.pstr);

  return const_cast<TypedValue*>(ArrNR(base).asArray()
                                            .rvalAtRef(cellAsCVarRef(key))
                                            .asTypedValue());
}

/**
 * Elem when base is an Array
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemArray(ArrayData* base, key_type<keyType> key) {
  auto* result = ElemArrayPre(base, key);

  if (UNLIKELY(result->m_type == KindOfUninit)) {
    result = const_cast<TypedValue*>(init_null_variant.asTypedValue());
    if (warn) {
      auto scratch = initScratchKey(key);
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(&scratch).toString().data());
    }
  }

  return result;
}

/**
 * Elem when base is Null
 */
inline TypedValue* ElemEmptyish() {
  return const_cast<TypedValue*>(init_null_variant.asTypedValue());
}

/**
 * Elem when base is an Int64 or Double
 */
inline TypedValue* ElemNumberish() {
  if (RuntimeOption::EnableHipHopSyntax) {
    raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  }
  return ElemEmptyish();
}

/**
 * Elem when base is a Boolean
 */
inline TypedValue* ElemBoolean(TypedValue* base) {
  if (base->m_data.num) {
    return ElemNumberish();
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
  if (LIKELY(IS_INT_TYPE(key.m_type))) {
    return key.m_data.num;
  } else if (LIKELY(IS_STRING_TYPE(key.m_type))) {
    return key.m_data.pstr->toInt64(10);
  } else {
    raise_warning("String offset cast occurred");
    return cellAsCVarRef(key).toInt64();
  }
}

/**
 * Elem when base is a String
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemString(TypedValue& tvScratch, TypedValue* base,
                              key_type<keyType> key) {
  auto offset = ElemStringPre(key);

  if (offset < 0 || offset >= base->m_data.pstr->size()) {
    if (warn && RuntimeOption::EnableHipHopSyntax) {
      raise_warning("Out of bounds");
    }
    static StringData* sd = makeStaticString("");
    tvScratch.m_data.pstr = sd;
    tvScratch.m_type = KindOfStaticString;
  } else {
    tvScratch.m_data.pstr = base->m_data.pstr->getChar(offset);
    assert(tvScratch.m_data.pstr->isStatic());
    tvScratch.m_type = KindOfStaticString;
  }
  return &tvScratch;
}

/**
 * Elem when base is an Object
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemObject(TypedValue& tvRef, TypedValue* base,
                              key_type<keyType> key) {
  TypedValue scratch = initScratchKey(key);

  if (LIKELY(base->m_data.pobj->isCollection())) {
    if (warn) {
      return collectionAt(base->m_data.pobj, &scratch);
    } else {
      auto* res = collectionGet(base->m_data.pobj, &scratch);
      if (!res) {
        res = &tvRef;
        tvWriteNull(res);
      }
      return res;
    }
  }
  return objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(scratch));
}

/**
 * $result = $base[$key];
 */
template <bool warn, KeyType keyType>
NEVER_INLINE TypedValue* ElemSlow(TypedValue& tvScratch, TypedValue& tvRef,
                                  TypedValue* base, key_type<keyType> key) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
    return ElemEmptyish();
  case KindOfInt64:
  case KindOfDouble:
    return ElemNumberish();
  case KindOfBoolean:
    return ElemBoolean(base);
  case KindOfStaticString:
  case KindOfString:
    return ElemString<warn, keyType>(tvScratch, base, key);
  case KindOfArray:
    return ElemArray<warn, keyType>(base->m_data.parr, key);
  case KindOfObject:
    return ElemObject<warn, keyType>(tvRef, base, key);
  default:
    assert(false);
    return nullptr;
  }
}

/**
 * Fast path for Elem assuming base is an Array
 */
template <bool warn, KeyType keyType = KeyType::Any>
inline TypedValue* Elem(TypedValue& tvScratch, TypedValue& tvRef,
                        TypedValue* base, key_type<keyType> key) {
  if (LIKELY(base->m_type == KindOfArray)) {
    return ElemArray<warn, keyType>(base->m_data.parr, key);
  }
  return ElemSlow<warn, keyType>(tvScratch, tvRef, base, key);
}

template<KeyType kt>
inline TypedValue* ElemDArrayPre(Array& base, key_type<kt> key) {
  return const_cast<TypedValue*>(
    base.lvalAt(keyAsValue(key)).asTypedValue());
}

template<>
inline TypedValue* ElemDArrayPre<KeyType::Any>(Array& base, TypedValue key) {
  if (key.m_type == KindOfInt64) {
    return ElemDArrayPre<KeyType::Int>(base, key.m_data.num);
  }

  return const_cast<TypedValue*>(base.lvalAt(tvAsCVarRef(&key)).asTypedValue());
}

/**
 * ElemD when base is an Array
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemDArray(TypedValue* base, key_type<keyType> key) {
  auto& baseArr = tvAsVariant(base).asArrRef();
  bool defined = !warn || baseArr.exists(keyAsValue(key));

  auto* result = ElemDArrayPre<keyType>(baseArr, key);

  if (warn) {
    if (!defined) {
      TypedValue scratchKey = initScratchKey(key);
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(&scratchKey).toString().data());
    }
  }

  return result;
}

/**
 * ElemD when base is Null
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemDEmptyish(TypedValue* base, key_type<keyType> key) {
  TypedValue scratchKey = initScratchKey(key);
  tvAsVariant(base) = Array::Create();
  auto const result = const_cast<TypedValue*>(
    tvAsVariant(base).asArrRef().lvalAt(
      cellAsCVarRef(scratchKey)).asTypedValue()
  );
  if (warn) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&scratchKey).toString().data());
  }
  return result;
}

/**
 * ElemD when base is an Int64 or Double
 */
inline TypedValue* ElemDNumberish(TypedValue& tvScratch) {
  // TODO Task #2757837: Get rid of tvScratch
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteUninit(&tvScratch);
  return &tvScratch;
}

/**
 * ElemD when base is a Boolean
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemDBoolean(TypedValue& tvScratch, TypedValue* base,
                                key_type<keyType> key) {
  if (base->m_data.num) {
    return ElemDNumberish(tvScratch);
  }
  return ElemDEmptyish<warn, keyType>(base, key);
}

/**
 * ElemD when base is a String
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemDString(TypedValue* base, key_type<keyType> key) {
  if (base->m_data.pstr->size() == 0) {
    return ElemDEmptyish<warn, keyType>(base, key);
  }
  raise_error("Operator not supported for strings");
  return nullptr;
}

/**
 * ElemD when base is an Object
 */
template <bool reffy, KeyType keyType>
inline TypedValue* ElemDObject(TypedValue& tvRef, TypedValue* base,
                               key_type<keyType> key) {
  TypedValue scratchKey = initScratchKey(key);
  auto obj = base->m_data.pobj;

  if (LIKELY(obj->isCollection())) {
    if (reffy) {
      raise_error("Collection elements cannot be taken by reference");
      return nullptr;
    }
    return collectionAtLval(obj, &scratchKey);
  } else if (obj->getVMClass()->classof(SystemLib::s_ArrayObjectClass)) {
    auto storage = obj->o_realProp(s_storage, 0,
                                   SystemLib::s_ArrayObjectClass->nameRef());
    // ArrayObject should have the 'storage' property...
    assert(storage != nullptr);
    return ElemDArray<false /* warn */, keyType>(storage->asTypedValue(), key);
  }
  return objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(scratchKey));
}

/**
 * $base[$key] = ...
 * \____ ____/
 *      v
 *   $result
 */
template <bool warn, bool reffy, KeyType keyType = KeyType::Any>
inline TypedValue* ElemD(TypedValue& tvScratch, TypedValue& tvRef,
                         TypedValue* base, key_type<keyType> key) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
    return ElemDEmptyish<warn, keyType>(base, key);
  case KindOfBoolean:
    return ElemDBoolean<warn, keyType>(tvScratch, base, key);
  case KindOfInt64:
  case KindOfDouble:
    return ElemDNumberish(tvScratch);
  case KindOfStaticString:
  case KindOfString:
    return ElemDString<warn, keyType>(base, key);
  case KindOfArray:
    return ElemDArray<warn, keyType>(base, key);
  case KindOfObject:
    return ElemDObject<reffy, keyType>(tvRef, base, key);
  default:
    assert(false);
    return nullptr; // Silence compiler warning.
  }
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
inline TypedValue* ElemUArray(TypedValue& tvScratch, TypedValue* base,
                              key_type<keyType> key) {
  auto& baseArr = tvAsVariant(base).asArrRef();
  bool defined = baseArr.exists(keyAsValue(key));
  if (defined) {
    return ElemUArrayImpl<keyType>(baseArr, key);
  }

  tvWriteUninit(&tvScratch);
  return &tvScratch;
}

/**
 * ElemU when base is an Object
 */
template <KeyType keyType>
inline TypedValue* ElemUObject(TypedValue& tvRef, TypedValue* base,
                               key_type<keyType> key) {
  auto const& scratchKey = initScratchKey(key);
  if (LIKELY(base->m_data.pobj->isCollection())) {
    return collectionAtLval(base->m_data.pobj, &scratchKey);
  }
  return objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(scratchKey));
}

/**
 * $base[$key] = ...
 * \____ ____/
 *      v
 *   $result
 */
template <KeyType keyType = KeyType::Any>
inline TypedValue* ElemU(TypedValue& tvScratch, TypedValue& tvRef,
                         TypedValue* base, key_type<keyType> key) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
    // Unset on a null base never modifies the base, but the
    // const_cast is necessary to placate the type system.
    return const_cast<TypedValue*>(null_variant.asTypedValue());
  case KindOfStaticString:
  case KindOfString:
    raise_error(Strings::OP_NOT_SUPPORTED_STRING);
    return nullptr;
  case KindOfArray:
    return ElemUArray<keyType>(tvScratch, base, key);
  case KindOfObject:
    return ElemUObject<keyType>(tvRef, base, key);
  default:
    not_reached();
    return nullptr;
  }
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
inline TypedValue* NewElemInvalid(TypedValue& tvScratch) {
  raise_warning("Invalid NewElem operand");
  tvWriteUninit(&tvScratch);
  return &tvScratch;
}

/**
 * NewElem when base is a Boolean
 */
inline TypedValue* NewElemBoolean(TypedValue& tvScratch, TypedValue* base) {
  if (base->m_data.num) {
    return NewElemInvalid(tvScratch);
  }
  return NewElemEmptyish(base);
}

/**
 * NewElem when base is a String
 */
inline TypedValue* NewElemString(TypedValue& tvScratch, TypedValue* base) {
  if (base->m_data.pstr->size() == 0) {
    return NewElemEmptyish(base);
  }
  return NewElemInvalid(tvScratch);
}

/**
 * NewElem when base is an Array
 */
inline TypedValue* NewElemArray(TypedValue* base) {
  return const_cast<TypedValue*>(tvAsVariant(base).asArrRef().lvalAt()
                                 .asTypedValue());
}

/**
 * NewElem when base is an Object
 */
inline TypedValue* NewElemObject(TypedValue& tvRef, TypedValue* base) {
  if (base->m_data.pobj->isCollection()) {
    throw_cannot_use_newelem_for_lval_read();
    return nullptr;
  }
  return objOffsetGet(tvRef, instanceFromTv(base), init_null_variant);
}

/**
 * $result = ($base[] = ...);
 */
inline TypedValue* NewElem(TypedValue& tvScratch, TypedValue& tvRef,
                           TypedValue* base) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
    return NewElemEmptyish(base);
  case KindOfBoolean:
    return NewElemBoolean(tvScratch, base);
  case KindOfStaticString:
  case KindOfString:
    return NewElemString(tvScratch, base);
  case KindOfArray:
    return NewElemArray(base);
  case KindOfObject:
    return NewElemObject(tvRef, base);
  default:
    return NewElemInvalid(tvScratch);
  }
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
 * SetElem when base is an Int64 or Double
 */
template <bool setResult>
inline void SetElemNumberish(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (!setResult) {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  tvRefcountedDecRefCell((TypedValue*)value);
  tvWriteNull((TypedValue*)value);
}

/**
 * SetElem when base is a Boolean
 */
template <bool setResult, KeyType keyType>
inline void SetElemBoolean(TypedValue* base, key_type<keyType> key,
                           Cell* value) {
  if (base->m_data.num) {
    SetElemNumberish<setResult>(value);
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
    if (LIKELY(IS_STRING_TYPE(value->m_type))) {
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
  if (x >= 0 && x < baseLen && !base->m_data.pstr->hasMultipleRefs()) {
    // Modify base in place.  This is safe because the LHS owns the
    // only reference.
    auto const oldp = base->m_data.pstr;
    auto const newp = oldp->modifyChar(x, y[0]);
    if (UNLIKELY(newp != oldp)) {
      newp->incRefCount();
      decRefStr(oldp);
      base->m_data.pstr = newp;
      base->m_type = KindOfString;
    }
  } else {
    StringData* sd = StringData::Make(slen);
    char* s = sd->bufferSlice().ptr;
    memcpy(s, base->m_data.pstr->data(), baseLen);
    if (x > baseLen) {
      memset(&s[baseLen], ' ', slen - baseLen - 1);
    }
    s[x] = y[0];
    sd->setSize(slen);
    sd->incRefCount();
    decRefStr(base->m_data.pstr);
    base->m_data.pstr = sd;
    base->m_type = KindOfString;
  }

  StringData* sd = StringData::Make(y, strlen(y), CopyString);
  sd->incRefCount();
  return sd;
}

/**
 * SetElem when base is an Object
 */
template <KeyType keyType>
inline void SetElemObject(TypedValue* base, key_type<keyType> key,
                          Cell* value) {
  auto const& scratchKey = initScratchKey(key);
  if (LIKELY(base->m_data.pobj->isCollection())) {
    collectionSet(base->m_data.pobj, &scratchKey, value);
  } else {
    objOffsetSet(instanceFromTv(base), tvAsCVarRef(&scratchKey), value);
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

template<bool setRef> inline
typename ShuffleReturn<setRef>::return_type
arrayRefShuffle(ArrayData* oldData, ArrayData* newData, TypedValue* base) {
  if (newData == oldData) {
    return ShuffleReturn<setRef>::do_return(oldData);
  }

  newData->incRefCount();
  if (setRef) {
    if (base->m_type == KindOfArray && base->m_data.parr == oldData) {
      base->m_data.parr = newData;
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
  if (key->isStrictlyInteger(n)) {
    return a->set(n, cellAsCVarRef(*value), copy);
  }
  return a->set(StrNR(key), cellAsCVarRef(*value), copy);
}

template<bool setResult>
inline ArrayData* SetElemArrayPre(ArrayData* a,
                                  TypedValue key,
                                  Cell* value,
                                  bool copy) {
  if (IS_NULL_TYPE(key.m_type)) {
    return a->set(empty_string, cellAsCVarRef(*value), copy);
  }
  if (IS_STRING_TYPE(key.m_type)) {
    return SetElemArrayPre<setResult>(a, key.m_data.pstr, value, copy);
  }
  if (key.m_type == KindOfInt64) {
    return SetElemArrayPre<setResult>(a, key.m_data.num, value, copy);
  }
  if (key.m_type != KindOfArray && key.m_type != KindOfObject) {
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
  ArrayData* a = base->m_data.parr;
  bool copy = (a->hasMultipleRefs())
    || (value->m_type == KindOfArray && value->m_data.parr == a);

  auto* newData = SetElemArrayPre<setResult>(a, key, value, copy);

  arrayRefShuffle<true>(a, newData, base);
}

/**
 * SetElem() leaves the result in 'value', rather than returning it as in
 * SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
 * get around.
 */
template <bool setResult, KeyType keyType>
NEVER_INLINE
StringData* SetElemSlow(TypedValue* base, key_type<keyType> key, Cell* value) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
    SetElemEmptyish<keyType>(base, key, value);
    return nullptr;
  case KindOfBoolean:
    SetElemBoolean<setResult, keyType>(base, key, value);
    return nullptr;
  case KindOfInt64:
  case KindOfDouble:
    SetElemNumberish<setResult>(value);
    return nullptr;
  case KindOfStaticString:
  case KindOfString:
    return SetElemString<setResult, keyType>(base, key, value);
  case KindOfArray:
    SetElemArray<setResult, keyType>(base, key, value);
    return nullptr;
  case KindOfObject:
    SetElemObject<keyType>(base, key, value);
    return nullptr;
  default:
    not_reached();
    return nullptr;
  }
}

/**
 * Fast path for SetElem assuming base is an Array
 */
template <bool setResult, KeyType keyType = KeyType::Any>
inline StringData* SetElem(TypedValue* base, key_type<keyType> key,
                           Cell* value) {
  if (LIKELY(base->m_type == KindOfArray)) {
    SetElemArray<setResult, keyType>(base, key, value);
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
inline void SetNewElemNumberish(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (!setResult) {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
  tvRefcountedDecRefCell((TypedValue*)value);
  tvWriteNull((TypedValue*)value);
}

/**
 * SetNewElem when base is a Boolean
 */
template <bool setResult>
inline void SetNewElemBoolean(TypedValue* base, Cell* value) {
  if (base->m_data.num) {
    SetNewElemNumberish<setResult>(value);
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
  ArrayData* a = base->m_data.parr;
  bool copy = (a->hasMultipleRefs())
    || (value->m_type == KindOfArray && value->m_data.parr == a);
  ArrayData* a2 = a->append(cellAsCVarRef(*value), copy);
  if (a2 != a) {
    a2->incRefCount();
    auto old = base->m_data.parr;
    base->m_data.parr = a2;
    old->decRefAndRelease();
  }
}

/**
 * SetNewElem when base is an Object
 */
inline void SetNewElemObject(TypedValue* base, Cell* value) {
  if (LIKELY(base->m_data.pobj->isCollection())) {
    collectionAppend(base->m_data.pobj, (TypedValue*)value);
  } else {
    objOffsetAppend(instanceFromTv(base), (TypedValue*)value);
  }
}

/**
 * $base[] = ...
 */
template <bool setResult>
inline void SetNewElem(TypedValue* base, Cell* value) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
    return SetNewElemEmptyish(base, value);
  case KindOfBoolean:
    return SetNewElemBoolean<setResult>(base,  value);
  case KindOfInt64:
  case KindOfDouble:
    return SetNewElemNumberish<setResult>(value);
  case KindOfStaticString:
  case KindOfString:
    return SetNewElemString(base, value);
  case KindOfArray:
    return SetNewElemArray(base, value);
  case KindOfObject:
    return SetNewElemObject(base, value);
  default: assert(false);
  }
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
  SETOP_BODY_CELL(result, op, rhs);
  return result;
}

/**
 * SetOpElem when base is Int64 or Double
 */
inline TypedValue* SetOpElemNumberish(TypedValue& tvScratch) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvScratch);
  return &tvScratch;
}

/**
 * $result = ($base[$x] <op>= $y)
 */
inline TypedValue* SetOpElem(TypedValue& tvScratch, TypedValue& tvRef,
                             SetOpOp op, TypedValue* base,
                             TypedValue key, Cell* rhs) {
  TypedValue* result;

  DataType type;
  opPre(base, type);

  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    result = SetOpElemEmptyish(op, base, key, rhs);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      result = SetOpElemNumberish(tvScratch);
    } else {
      result = SetOpElemEmptyish(op, base, key, rhs);
    }
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    result = SetOpElemNumberish(tvScratch);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      raise_error("Invalid SetOpElem operand");
    }
    result = SetOpElemEmptyish(op, base, key, rhs);
    break;
  }
  case KindOfArray: {
    result = ElemDArray<MoreWarnings, KeyType::Any>(base, key);
    result = tvToCell(result);
    SETOP_BODY_CELL(result, op, rhs);
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionAtRw(base->m_data.pobj, &key);
      SETOP_BODY(result, op, rhs);
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base),
                            cellAsCVarRef(key));
      SETOP_BODY(result, op, rhs);
      objOffsetSet(instanceFromTv(base), tvAsCVarRef(&key), result, false);
    }
    break;
  }
  default: {
    always_assert(false && "Unexpected DataType");
  }
  }
  return result;
}

inline TypedValue* SetOpNewElemEmptyish(SetOpOp op,
                                        TypedValue* base, Cell* rhs) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt();
  tvAsVariant(base) = a;
  SETOP_BODY(result, op, rhs);
  return result;
}
inline TypedValue* SetOpNewElemNumberish(TypedValue& tvScratch) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvScratch);
  return &tvScratch;
}
inline TypedValue* SetOpNewElem(TypedValue& tvScratch, TypedValue& tvRef,
                                SetOpOp op, TypedValue* base,
                                Cell* rhs) {
  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    result = SetOpNewElemEmptyish(op, base, rhs);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      result = SetOpNewElemNumberish(tvScratch);
    } else {
      result = SetOpNewElemEmptyish(op, base, rhs);
    }
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    result = SetOpNewElemNumberish(tvScratch);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      raise_error("[] operator not supported for strings");
    }
    result = SetOpNewElemEmptyish(op, base, rhs);
    break;
  }
  case KindOfArray: {
    result = (TypedValue*)&tvAsVariant(base).asArrRef().lvalAt();
    SETOP_BODY(result, op, rhs);
    break;
  }
  case KindOfObject: {
    if (base->m_data.pobj->isCollection()) {
      throw_cannot_use_newelem_for_lval_read();
      result = nullptr;
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), init_null_variant);
      SETOP_BODY(result, op, rhs);
      objOffsetAppend(instanceFromTv(base), result, false);
    }
    break;
  }
  default: {
    assert(false);
    result = nullptr; // Silence compiler warning.
  }
  }
  return result;
}

template <bool setResult>
NEVER_INLINE
void incDecBodySlow(IncDecOp op, TypedValue* fr, TypedValue* to) {
  if (fr->m_type == KindOfUninit) {
    ActRec* fp = g_context->m_fp;
    size_t pind = reinterpret_cast<TypedValue*>(fp) - fr - 1;
    if (pind < size_t(fp->m_func->numNamedLocals())) {
      // Only raise a warning if fr points to a local variable
      raise_notice(Strings::UNDEFINED_VARIABLE,
                   fp->m_func->localVarName(pind)->data());
    }
    // Convert uninit null to null so that we don't write out an uninit null
    // to the eval stack for PostInc and PostDec.
    fr->m_type = KindOfNull;
  } else {
    fr = tvToCell(fr);
  }

  assert(cellIsPlausible(*fr));

  auto dup = [&]() { if (setResult) cellDup(*fr, *to); };

  switch (op) {
  case IncDecOp::PreInc:
    cellInc(*fr);
    dup();
    return;
  case IncDecOp::PostInc:
    dup();
    cellInc(*fr);
    return;
  case IncDecOp::PreDec:
    cellDec(*fr);
    dup();
    return;
  case IncDecOp::PostDec:
    dup();
    cellDec(*fr);
    return;
  default: break;
  }

  switch (op) {
  case IncDecOp::PreIncO:
    cellIncO(*fr);
    dup();
    return;
  case IncDecOp::PostIncO:
    dup();
    cellIncO(*fr);
    return;
  case IncDecOp::PreDecO:
    cellDecO(*fr);
    dup();
    return;
  case IncDecOp::PostDecO:
    dup();
    cellDecO(*fr);
    return;
  default: break;
  }
  not_reached();
}

template <bool setResult>
inline void IncDecBody(IncDecOp op, TypedValue* fr, TypedValue* to) {
  if (UNLIKELY(fr->m_type != KindOfInt64)) {
    return incDecBodySlow<setResult>(op, fr, to);
  }

  auto copy = [&]() { if (setResult) cellCopy(*fr, *to); };

  // fast cases, assuming integers overflow to ints
  switch (op) {
  case IncDecOp::PreInc:
    ++fr->m_data.num;
    copy();
    return;
  case IncDecOp::PostInc:
    copy();
    ++fr->m_data.num;
    return;
  case IncDecOp::PreDec:
    --fr->m_data.num;
    copy();
    return;
  case IncDecOp::PostDec:
    copy();
    --fr->m_data.num;
    return;
  default: break;
  }

  // slow case, where integers can overflow to floats
  switch (op) {
  case IncDecOp::PreIncO:
    cellIncO(*fr);
    copy();
    return;
  case IncDecOp::PostIncO:
    copy();
    cellIncO(*fr);
    return;
  case IncDecOp::PreDecO:
    cellDecO(*fr);
    copy();
    return;
  case IncDecOp::PostDecO:
    copy();
    cellDecO(*fr);
    return;
  default: break;
  }
  not_reached();
}

template <bool setResult>
inline void IncDecElemEmptyish(IncDecOp op, TypedValue* base,
                               TypedValue key, TypedValue& dest) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt(tvAsCVarRef(&key));
  tvAsVariant(base) = a;
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(&key).toString().data());
  }
  IncDecBody<setResult>(op, result, &dest);
}
template <bool setResult>
inline void IncDecElemNumberish(TypedValue& dest) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (setResult) {
    tvWriteNull(&dest);
  }
}
template <bool setResult>
inline void IncDecElem(TypedValue& tvScratch, TypedValue& tvRef,
                       IncDecOp op, TypedValue* base,
                       TypedValue key, TypedValue& dest) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    IncDecElemEmptyish<setResult>(op, base, key, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecElemNumberish<setResult>(dest);
    } else {
      IncDecElemEmptyish<setResult>(op, base, key, dest);
    }
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    IncDecElemNumberish<setResult>(dest);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      raise_error("Invalid IncDecElem operand");
    }
    IncDecElemEmptyish<setResult>(op, base, key, dest);
    break;
  }
  case KindOfArray: {
    TypedValue* result = ElemDArray<MoreWarnings, KeyType::Any>(base, key);
    IncDecBody<setResult>(op, result, &dest);
    break;
  }
  case KindOfObject: {
    TypedValue* result;
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionAtRw(base->m_data.pobj, &key);
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(key));
    }
    IncDecBody<setResult>(op, result, &dest);
    break;
  }
  default: assert(false);
  }
}

template <bool setResult>
inline void IncDecNewElemEmptyish(IncDecOp op, TypedValue* base,
                                  TypedValue& dest) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt();
  tvAsVariant(base) = a;
  IncDecBody<setResult>(op, result, &dest);
}
template <bool setResult>
inline void IncDecNewElemNumberish(TypedValue& dest) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (setResult) {
    tvWriteNull(&dest);
  }
}
template <bool setResult>
inline void IncDecNewElem(TypedValue& tvScratch, TypedValue& tvRef,
                          IncDecOp op, TypedValue* base,
                          TypedValue& dest) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    IncDecNewElemEmptyish<setResult>(op, base, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecNewElemNumberish<setResult>(dest);
    } else {
      IncDecNewElemEmptyish<setResult>(op, base, dest);
    }
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    IncDecNewElemNumberish<setResult>(dest);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      raise_error("Invalid IncDecNewElem operand");
    }
    IncDecNewElemEmptyish<setResult>(op, base, dest);
    break;
  }
  case KindOfArray: {
    TypedValue* result = (TypedValue*)&tvAsVariant(base).asArrRef().lvalAt();
    IncDecBody<setResult>(op, result, &dest);
    break;
  }
  case KindOfObject: {
    TypedValue* result;
    if (base->m_data.pobj->isCollection()) {
      throw_cannot_use_newelem_for_lval_read();
      result = nullptr;
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), init_null_variant);
      IncDecBody<setResult>(op, result, &dest);
    }
    break;
  }
  default: assert(false);
  }
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
  if (!key->isStrictlyInteger(n)) {
    return a->remove(StrNR(key), copy);
  } else {
    return a->remove(n, copy);
  }
}

inline ArrayData* UnsetElemArrayPre(ArrayData* a, TypedValue key,
                                    bool copy) {
  if (IS_STRING_TYPE(key.m_type)) {
    return UnsetElemArrayPre(a, key.m_data.pstr, copy);
  } else if (key.m_type == KindOfInt64) {
    return UnsetElemArrayPre(a, key.m_data.num, copy);
  } else {
    VarNR varKey = tvAsCVarRef(&key).toKey();
    if (varKey.isNull()) {
      return a;
    }
    return a->remove(varKey, copy);
  }
}

/**
 * UnsetElem when base is an Array
 */
template <KeyType keyType>
inline void UnsetElemArray(TypedValue* base, key_type<keyType> key) {
  ArrayData* a = base->m_data.parr;
  bool copy = a->hasMultipleRefs();
  ArrayData* a2 = UnsetElemArrayPre(a, key, copy);

  if (a2 != a) {
    a2->incRefCount();
    auto old = base->m_data.parr;
    base->m_data.parr = a2;
    old->decRefAndRelease();
 }
}

/**
 * unset($base[$member])
 */
template <KeyType keyType>
NEVER_INLINE
void UnsetElemSlow(TypedValue* base, key_type<keyType> key) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfStaticString:
  case KindOfString: {
    raise_error(Strings::CANT_UNSET_STRING);
  }
  case KindOfArray: {
    UnsetElemArray<keyType>(base, key);
    break;
  }
  case KindOfObject: {
    auto const& scratchKey = initScratchKey(key);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      collectionUnset(base->m_data.pobj, &scratchKey);
    } else {
      objOffsetUnset(instanceFromTv(base), tvAsCVarRef(&scratchKey));
    }
    break;
  }
  default: break; // Do nothing.
  }
}

/**
 * Fast path for UnsetElem assuming base is an Array
 */
template <KeyType keyType = KeyType::Any>
inline void UnsetElem(TypedValue* base, key_type<keyType> key) {
  if (LIKELY(base->m_type == KindOfArray)) {
    UnsetElemArray<keyType>(base, key);
    return;
  }
  return UnsetElemSlow<keyType>(base, key);
}

/**
 * IssetEmptyElem when base is an Object
 */
template<bool useEmpty, KeyType keyType>
inline bool IssetEmptyElemObj(TypedValue& tvRef, ObjectData* instance,
                              key_type<keyType> key) {
  TypedValue scratchKey = initScratchKey(key);

  if (useEmpty) {
    if (LIKELY(instance->isCollection())) {
      return collectionEmpty(instance, &scratchKey);
    } else {
      return objOffsetEmpty(tvRef, instance, cellAsCVarRef(scratchKey));
    }
  } else {
    if (LIKELY(instance->isCollection())) {
      return collectionIsset(instance, &scratchKey);
    } else {
      return objOffsetIsset(tvRef, instance, cellAsCVarRef(scratchKey));
    }
  }
}

/**
 * IssetEmptyElem when base is a String
 */
template <bool useEmpty, KeyType keyType>
inline bool IssetEmptyElemString(TypedValue& tvScratch, TypedValue* base,
                                 key_type<keyType> key) {
  // TODO Task #2716479: Fix this so that the warnings raised match
  // Zend PHP.
  TypedValue scratchKey = initScratchKey(key);
  int64_t x;
  if (LIKELY(scratchKey.m_type == KindOfInt64)) {
    x = scratchKey.m_data.num;
  } else {
    TypedValue tv;
    cellDup(scratchKey, tv);
    bool badKey = false;
    if (IS_STRING_TYPE(tv.m_type)) {
      const char* str = tv.m_data.pstr->data();
      size_t len = tv.m_data.pstr->size();
      while (len > 0 &&
             (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n')) {
        ++str;
        --len;
      }
      int64_t n;
      badKey = !is_strictly_integer(str, len, n);
    } else if (tv.m_type == KindOfArray || tv.m_type == KindOfObject ||
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
  tvScratch.m_data.pstr = base->m_data.pstr->getChar(x);
  assert(tvScratch.m_data.pstr->isStatic());
  tvScratch.m_type = KindOfStaticString;
  return !cellToBool(*tvToCell(&tvScratch));
}

/**
 * IssetEmptyElem when base is an Array
 */
template <bool useEmpty, KeyType keyType>
inline bool IssetEmptyElemArray(TypedValue* base, key_type<keyType> key) {
  TypedValue* result = ElemArray<false, keyType>(base->m_data.parr, key);
  if (useEmpty) {
    return !cellToBool(*tvToCell(result));
  }
  return !cellIsNull(tvToCell(result));
}

/**
 * isset/empty($base[$key])
 */
template <bool useEmpty, KeyType keyType>
NEVER_INLINE
bool IssetEmptyElemSlow(TypedValue& tvScratch, TypedValue& tvRef,
                        TypedValue* base, key_type<keyType> key) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfStaticString:
  case KindOfString:
    return IssetEmptyElemString<useEmpty, keyType>(tvScratch, base, key);
  case KindOfArray:
    return IssetEmptyElemArray<useEmpty, keyType>(base, key);
  case KindOfObject:
    return IssetEmptyElemObj<useEmpty, keyType>(tvRef, base->m_data.pobj, key);
  default:
    return useEmpty;
  }
}

/**
 * Fast path for IssetEmptyElem assuming base is an Array
 */
template <bool useEmpty, KeyType keyType = KeyType::Any>
inline bool IssetEmptyElem(TypedValue& tvScratch, TypedValue& tvRef,
                           TypedValue* base, key_type<keyType> key) {
  if (LIKELY(base->m_type == KindOfArray)) {
    return IssetEmptyElemArray<useEmpty, keyType>(base, key);
  }
  return IssetEmptyElemSlow<useEmpty, keyType>(tvScratch, tvRef, base, key);
}

template <bool warn>
inline DataType propPreNull(TypedValue& tvScratch, TypedValue*& result) {
  tvWriteNull(&tvScratch);
  result = &tvScratch;
  if (warn) {
    raise_warning("Cannot access property on non-object");
  }
  return KindOfNull;
}

template <bool warn, bool define>
inline DataType propPreStdclass(TypedValue& tvScratch,
                                TypedValue*& result, TypedValue* base) {
  if (!define) {
    return propPreNull<warn>(tvScratch, result);
  }
  // TODO(#1124706): We don't want to do this anymore.
  auto const obj = newInstance(SystemLib::s_stdclassClass);
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  obj->incRefCount();
  result = base;
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);
  return KindOfObject;
}

template <bool warn, bool define, bool issetEmpty>
inline DataType propPre(TypedValue& tvScratch, TypedValue*& result,
                        TypedValue*& base) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    return propPreStdclass<warn, define>(tvScratch, result, base);
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      return propPreNull<warn>(tvScratch, result);
    } else {
      return propPreStdclass<warn, define>(tvScratch, result, base);
    }
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      return propPreNull<warn>(tvScratch, result);
    } else {
      return propPreStdclass<warn, define>(tvScratch, result, base);
    }
  }
  case KindOfArray: {
    return issetEmpty ? KindOfArray : propPreNull<warn>(tvScratch, result);
  }
  case KindOfObject: {
    return KindOfObject;
  }
  default: {
    return propPreNull<warn>(tvScratch, result);
  }
  }
}

// define == false:
//   $result = $base->$key;
//
// define == true:
//   $base->$key = ...
//   \____ ____/
//        v
//     $result
template <bool warn, bool define, bool unset, bool baseIsObj = false,
          KeyType keyType = KeyType::Any>
inline TypedValue* Prop(TypedValue& tvScratch, TypedValue& tvRef,
                        Class* ctx, TypedValue* base, key_type<keyType> key) {
  assert(!warn || !unset);
  TypedValue* result = nullptr;
  ObjectData* instance;
  if (baseIsObj) {
    instance = reinterpret_cast<ObjectData*>(base);
  } else {
    DataType t = propPre<warn, define, false>(tvScratch, result, base);
    if (t == KindOfNull) {
      return result;
    }
    assert(t == KindOfObject);
    instance = instanceFromTv(base);
  }

  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { releaseKey<keyType>(keySD); };

  // Get property.
  result = &tvScratch;
#define ARGS result, tvRef, ctx, keySD
  if (!warn && !(define || unset)) instance->prop  (ARGS);
  if (!warn &&  (define || unset)) instance->propD (ARGS);
  if ( warn && !define           ) instance->propW (ARGS);
  if ( warn &&  define           ) instance->propWD(ARGS);
#undef ARGS
  return result;
}

template <bool useEmpty>
inline bool IssetEmptyPropObj(Class* ctx, ObjectData* instance,
                              TypedValue key) {
  bool issetEmptyResult;
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };

  issetEmptyResult = useEmpty ?
    instance->propEmpty(ctx, keySD) :
    instance->propIsset(ctx, keySD);
  return issetEmptyResult;
}

template <bool useEmpty, bool isObj = false>
inline bool IssetEmptyProp(Class* ctx, TypedValue* base,
                           TypedValue key) {
  if (isObj) {
    return IssetEmptyPropObj<useEmpty>(
      ctx, reinterpret_cast<ObjectData*>(base), key);
  }

  TypedValue tvScratch;
  TypedValue* result = nullptr;
  DataType t = propPre<false, false, true>(tvScratch, result, base);
  if (t == KindOfNull) {
    return useEmpty;
  }
  if (t == KindOfObject) {
    return IssetEmptyPropObj<useEmpty>(ctx, instanceFromTv(base), key);
  } else {
    assert(t == KindOfArray);
    return useEmpty;
  }
}

template <bool setResult>
inline void SetPropNull(Cell* val) {
  raise_warning("Cannot access property on non-object");
  if (setResult) {
    tvRefcountedDecRefCell(val);
    tvWriteNull(val);
  } else {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
}

inline void SetPropStdclass(TypedValue* base, TypedValue key,
                            Cell* val) {
  auto const obj = newInstance(SystemLib::s_stdclassClass);
  obj->incRefCount();
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  obj->setProp(nullptr, keySD, (TypedValue*)val);
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);
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
template <bool setResult, bool isObj = false, KeyType keyType = KeyType::Any>
inline void SetProp(Class* ctx, TypedValue* base, key_type<keyType> key,
                    Cell* val) {
  if (isObj) {
    SetPropObj<keyType>(ctx, reinterpret_cast<ObjectData*>(base), key, val);
    return;
  }

  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    SetPropStdclass(base, initScratchKey(key), val);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      SetPropNull<setResult>(val);
    } else {
      SetPropStdclass(base, initScratchKey(key), val);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      SetPropNull<setResult>(val);
    } else {
      SetPropStdclass(base, initScratchKey(key), val);
    }
    break;
  }
  case KindOfObject: {
    SetPropObj<keyType>(ctx, base->m_data.pobj, key, val);
    break;
  }
  default: {
    SetPropNull<setResult>(val);
    break;
  }
  }
}

inline TypedValue* SetOpPropNull(TypedValue& tvScratch) {
  raise_warning("Attempt to assign property of non-object");
  tvWriteNull(&tvScratch);
  return &tvScratch;
}
inline TypedValue* SetOpPropStdclass(TypedValue& tvRef, SetOpOp op,
                                     TypedValue* base, TypedValue key,
                                     Cell* rhs) {
  auto const obj = newInstance(SystemLib::s_stdclassClass);
  obj->incRefCount();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);

  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  tvWriteNull(&tvRef);
  SETOP_BODY(&tvRef, op, rhs);
  obj->setProp(nullptr, keySD, &tvRef);
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
template<bool isObj = false>
inline TypedValue* SetOpProp(TypedValue& tvScratch, TypedValue& tvRef,
                             Class* ctx, SetOpOp op,
                             TypedValue* base, TypedValue key,
                             Cell* rhs) {
  if (isObj) {
    return SetOpPropObj(tvRef, ctx, op,
                        reinterpret_cast<ObjectData*>(base),
                        key, rhs);
  }

  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    result = SetOpPropStdclass(tvRef, op, base, key, rhs);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      result = SetOpPropNull(tvScratch);
    } else {
      result = SetOpPropStdclass(tvRef, op, base, key, rhs);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      result = SetOpPropNull(tvScratch);
    } else {
      result = SetOpPropStdclass(tvRef, op, base, key, rhs);
    }
    break;
  }
  case KindOfObject: {
    result = SetOpPropObj(tvRef, ctx, op, instanceFromTv(base), key, rhs);
    break;
  }
  default: {
    result = SetOpPropNull(tvScratch);
    break;
  }
  }
  return result;
}

template <bool setResult>
inline void IncDecPropNull(TypedValue& dest) {
  raise_warning("Attempt to increment/decrement property of non-object");
  if (setResult) {
    tvWriteNull(&dest);
  }
}
template <bool setResult>
inline void IncDecPropStdclass(IncDecOp op, TypedValue* base,
                               TypedValue key, TypedValue& dest) {
  auto const obj = newInstance(SystemLib::s_stdclassClass);
  obj->incRefCount();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);

  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  TypedValue tv;
  tvWriteNull(&tv);
  if (setResult) {
    IncDecBody<true>(op, (&tv), &dest);
    obj->setProp(nullptr, keySD, &dest);
  } else {
    // The caller doesn't actually want the result set, but we have to do so in
    // order to call obj->setProp().
    TypedValue tDest;
    tvWriteUninit(&tDest);
    IncDecBody<true>(op, (&tv), &tDest);
    obj->setProp(nullptr, keySD, &tDest);
    assert(!IS_REFCOUNTED_TYPE(tDest.m_type));
  }
  assert(!IS_REFCOUNTED_TYPE(tv.m_type));
}

template <bool setResult>
inline void IncDecPropObj(TypedValue& tvRef, Class* ctx,
                          IncDecOp op, ObjectData* base,
                          TypedValue key, TypedValue& dest) {
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  base->incDecProp<setResult>(tvRef, ctx, op, keySD, dest);
}

template <bool setResult, bool isObj = false>
inline void IncDecProp(TypedValue& tvScratch, TypedValue& tvRef,
                       Class* ctx, IncDecOp op,
                       TypedValue* base, TypedValue key,
                       TypedValue& dest) {
  if (isObj) {
    IncDecPropObj<setResult>(tvRef, ctx, op,
                             reinterpret_cast<ObjectData*>(base),
                             key, dest);
    return;
  }

  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    IncDecPropStdclass<setResult>(op, base, key, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecPropNull<setResult>(dest);
    } else {
      IncDecPropStdclass<setResult>(op, base, key, dest);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      IncDecPropNull<setResult>(dest);
    } else {
      IncDecPropStdclass<setResult>(op, base, key, dest);
    }
    break;
  }
  case KindOfObject: {
    IncDecPropObj<setResult>(tvRef, ctx, op, instanceFromTv(base),
                             key, dest);
    break;
  }
  default: {
    IncDecPropNull<setResult>(dest);
    break;
  }
  }
}

template<bool isObj = false>
inline void UnsetProp(Class* ctx, TypedValue* base, TypedValue key) {
  ObjectData* instance;
  if (!isObj) {
    DataType type;
    opPre(base, type);
    // Validate base.
    if (UNLIKELY(type != KindOfObject)) {
      // Do nothing.
      return;
    }
    instance = instanceFromTv(base);
  } else {
    instance = reinterpret_cast<ObjectData*>(base);
  }
  // Prepare key.
  StringData* keySD = prepareKey(key);
  SCOPE_EXIT { decRefStr(keySD); };
  // Unset property.
  instance->unsetProp(ctx, keySD);
}

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_VM_MEMBER_OPERATIONS_H_
