/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

enum class KeyType {
  Any = 0,
  Str,
  Int,
};

template<KeyType kt>
struct KeyTypeTraits {};
template<> struct KeyTypeTraits<KeyType::Any> {
  typedef CVarRef valType;
  typedef int64_t rawType; // This is never actually used but it's
                         // needed to keep the compiler happy
};
template<> struct KeyTypeTraits<KeyType::Str> {
  typedef StrNR valType;
  typedef StringData* rawType;
};
template<> struct KeyTypeTraits<KeyType::Int> {
  typedef int64_t valType;
  typedef int64_t rawType;
};

inline DataType keyDataType(KeyType t) {
  if (t == KeyType::Str) {
    return KindOfString;
  } else if (t == KeyType::Int) {
    return KindOfInt64;
  } else {
    not_reached();
  }
}

template<KeyType kt>
inline typename KeyTypeTraits<kt>::valType keyAsValue(TypedValue* key);
template<>
inline int64_t keyAsValue<KeyType::Int>(TypedValue* key) {
  return reinterpret_cast<int64_t>(key);
}
template<>
inline CVarRef keyAsValue<KeyType::Any>(TypedValue* key) {
  return tvAsCVarRef(key);
}
template<>
inline StrNR keyAsValue<KeyType::Str>(TypedValue* key) {
  return StrNR(reinterpret_cast<StringData*>(key));
}

template<KeyType kt>
inline typename KeyTypeTraits<kt>::rawType keyAsRaw(TypedValue* key) {
  if (kt == KeyType::Any) {
    not_reached();
  }
  return reinterpret_cast<typename KeyTypeTraits<kt>::rawType>(key);
}

// This is used for helpers that are not type specialized for the key
// and therefore need the key in a TypedValue
template<KeyType kt>
inline void initScratchKey(TypedValue& tv, TypedValue*& key) {
  if (kt != KeyType::Any) {
    tv.m_type = keyDataType(kt);
    tv.m_data.num = reinterpret_cast<int64_t>(key);
    key = &tv;
  }
}

void objArrayAccess(ObjectData* base);
TypedValue* objOffsetGet(TypedValue& tvRef, ObjectData* base,
                         CVarRef offset, bool validate=true);
bool objOffsetIsset(TypedValue& tvRef, ObjectData* base, CVarRef offset,
                    bool validate=true);
bool objOffsetEmpty(TypedValue& tvRef, ObjectData* base, CVarRef offset,
                    bool validate=true);
void objOffsetSet(ObjectData* base, CVarRef offset, TypedValue* val,
                  bool validate=true);
void objOffsetAppend(ObjectData* base, TypedValue* val, bool validate=true);
void objOffsetUnset(ObjectData* base, CVarRef offset);

StringData* prepareAnyKey(TypedValue* tv);

template <KeyType keyType = KeyType::Any>
inline StringData* prepareKey(TypedValue* tv) {
  if (keyType == KeyType::Str) {
    return reinterpret_cast<StringData*>(tv);
  } else if (keyType == KeyType::Any) {
    return prepareAnyKey(tv);
  } else {
    not_reached();
  }
}

template <KeyType keyType>
inline void releaseKey(StringData* keySD) {
  if (keyType == KeyType::Any) {
    decRefStr(keySD);
  } else {
    assert(keyType == KeyType::Str);
  }
}

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

inline TypedValue* ElemArrayRawKey(ArrayData* base, int64_t key) {
  TypedValue* result = base->nvGet(key);
  return result ? result : const_cast<TypedValue*>(null_variant.asTypedValue());
}

inline TypedValue* ElemArrayRawKey(ArrayData* base, StringData* key) {
  int64_t n;
  TypedValue* result = !key->isStrictlyInteger(n) ? base->nvGet(key) :
                       base->nvGet(n);
  return result ? result : const_cast<TypedValue*>(null_variant.asTypedValue());
}

/**
 * Elem when base is an Array
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemArray(ArrayData* base, TypedValue* key) {
  TypedValue* result;
  if (keyType == KeyType::Any) {
    DataType rtt = key->m_type;
    if (rtt == KindOfInt64) {
      result = ElemArrayRawKey(base, key->m_data.num);
    } else if (IS_STRING_TYPE(rtt)) {
      result = ElemArrayRawKey(base, key->m_data.pstr);
    } else {
      result = const_cast<TypedValue*>(ArrNR(base).asArray()
                                       .rvalAtRef(cellAsCVarRef(*key))
                                       .asTypedValue());
    }
  } else {
    result = ElemArrayRawKey(base, keyAsRaw<keyType>(key));
  }

  if (UNLIKELY(result->m_type == KindOfUninit)) {
    result = const_cast<TypedValue*>(init_null_variant.asTypedValue());
    if (warn) {
      TypedValue scratch;
      initScratchKey<keyType>(scratch, key);
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(key).toString().data());
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

/**
 * Elem when base is a String
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemString(TypedValue& tvScratch, TypedValue* base,
                              TypedValue* key) {
  int64_t x;
  if (keyType == KeyType::Int) {
    x = reinterpret_cast<int64_t>(key);
  } else if (keyType == KeyType::Str) {
    x = reinterpret_cast<StringData*>(key)->toInt64(10);
  } else if (LIKELY(IS_INT_TYPE(key->m_type))) {
    x = key->m_data.num;
  } else if (LIKELY(IS_STRING_TYPE(key->m_type))) {
    x = key->m_data.pstr->toInt64(10);
  } else {
    raise_warning("String offset cast occurred");
    x = cellAsCVarRef(*key).toInt64();
  }
  if (x < 0 || x >= base->m_data.pstr->size()) {
    if (warn) {
      raise_warning("Out of bounds");
    }
    static StringData* sd = makeStaticString("");
    tvScratch.m_data.pstr = sd;
    tvScratch.m_type = KindOfString;
  } else {
    tvScratch.m_data.pstr = base->m_data.pstr->getChar(x);
    assert(tvScratch.m_data.pstr->isStatic());
    tvScratch.m_type = KindOfStaticString;
  }
  return &tvScratch;
}

/**
 * Elem when base is an Object
 */
template <KeyType keyType>
inline TypedValue* ElemObject(TypedValue& tvRef, TypedValue* base,
                              TypedValue* key) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  if (LIKELY(base->m_data.pobj->isCollection())) {
    return collectionGet(base->m_data.pobj, key);
  }
  return objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
}

/**
 * $result = $base[$key];
 */
template <bool warn, KeyType keyType = KeyType::Any>
NEVER_INLINE TypedValue* ElemSlow(TypedValue& tvScratch, TypedValue& tvRef,
                                  TypedValue* base, TypedValue* key) {
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
    return ElemObject<keyType>(tvRef, base, key);
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
                        TypedValue* base, TypedValue* key) {
  if (LIKELY(base->m_type == KindOfArray)) {
    return ElemArray<warn, keyType>(base->m_data.parr, key);
  }
  return ElemSlow<warn, keyType>(tvScratch, tvRef, base, key);
}

/**
 * ElemD when base is an Array
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemDArray(TypedValue* base, TypedValue* key) {
  TypedValue* result;
  bool defined = !warn ||
    tvAsCVarRef(base).asCArrRef().exists(keyAsValue<keyType>(key));

  if (keyType == KeyType::Any && key->m_type == KindOfInt64) {
    result = const_cast<TypedValue*>(tvAsVariant(base).asArrRef()
                                     .lvalAt(key->m_data.num)
                                     .asTypedValue());
  } else {
    result = const_cast<TypedValue*>(tvAsVariant(base).asArrRef()
                                     .lvalAt(keyAsValue<keyType>(key))
                                     .asTypedValue());
  }

  if (warn) {
    if (!defined) {
      TypedValue scratch;
      initScratchKey<keyType>(scratch, key);
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(key).toString().data());
    }
  }

  return result;
}

/**
 * ElemD when base is Null
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemDEmptyish(TypedValue* base, TypedValue* key) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  Array a = Array::Create();
  TypedValue* result = const_cast<TypedValue*>(a.lvalAt(cellAsCVarRef(*key))
                                               .asTypedValue());
  if (warn) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(key).toString().data());
  }
  tvAsVariant(base) = a;
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
                                TypedValue* key) {
  if (base->m_data.num) {
    return ElemDNumberish(tvScratch);
  }
  return ElemDEmptyish<warn, keyType>(base, key);
}

/**
 * ElemD when base is a String
 */
template <bool warn, KeyType keyType>
inline TypedValue* ElemDString(TypedValue* base, TypedValue* key) {
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
                               TypedValue* key) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  auto obj = base->m_data.pobj;

  if (LIKELY(obj->isCollection())) {
    if (reffy) {
      raise_error("Collection elements cannot be taken by reference");
      return nullptr;
    }
    return collectionGet(obj, key);
  } else if (obj->getVMClass() == SystemLib::s_ArrayObjectClass) {
    auto storage = obj->o_realProp(s_storage, 0,
                                   SystemLib::s_ArrayObjectClass->nameRef());
    // ArrayObject should have the 'storage' property...
    assert(storage != nullptr);
    return ElemDArray<false /* warn */, keyType>(storage->asTypedValue(), key);
  }
  return objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
}

/**
 * $base[$key] = ...
 * \____ ____/
 *      v
 *   $result
 */
template <bool warn, bool reffy, KeyType keyType = KeyType::Any>
inline TypedValue* ElemD(TypedValue& tvScratch, TypedValue& tvRef,
                         TypedValue* base, TypedValue* key) {
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

/**
 * ElemU when base is an Array
 */
template <KeyType keyType>
inline TypedValue* ElemUArray(TypedValue& tvScratch, TypedValue* base,
                              TypedValue* key) {
  bool defined =
    tvAsCVarRef(base).asCArrRef().exists(keyAsValue<keyType>(key));
  if (defined) {
    if (keyType == KeyType::Any && key->m_type == KindOfInt64) {
      return const_cast<TypedValue*>(tvAsVariant(base).asArrRef()
                                     .lvalAt(key->m_data.num)
                                     .asTypedValue());
    }
    return const_cast<TypedValue*>(tvAsVariant(base).asArrRef()
                                   .lvalAt(keyAsValue<keyType>(key))
                                   .asTypedValue());
  }
  tvWriteUninit(&tvScratch);
  return &tvScratch;
}

/**
 * ElemU when base is an Object
 */
template <KeyType keyType>
inline TypedValue* ElemUObject(TypedValue& tvRef, TypedValue* base,
                               TypedValue* key) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  if (LIKELY(base->m_data.pobj->isCollection())) {
    return collectionGet(base->m_data.pobj, key);
  }
  return objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
}

/**
 * $base[$key] = ...
 * \____ ____/
 *      v
 *   $result
 */
template <KeyType keyType = KeyType::Any>
inline TypedValue* ElemU(TypedValue& tvScratch, TypedValue& tvRef,
                         TypedValue* base, TypedValue* key) {
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
  if (LIKELY(base->m_data.pobj->isCollection())) {
    raise_error("Cannot use [] for reading");
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
inline void SetElemEmptyish(TypedValue* base, TypedValue* key,
                            Cell* value) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  Array a = Array::Create();
  a.set(tvAsCVarRef(key), tvAsCVarRef(value));
  tvAsVariant(base) = a;
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
inline void SetElemBoolean(TypedValue* base, TypedValue* key,
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
inline int64_t castKeyToInt(TypedValue* key) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  return cellToInt(*tvToCell(key));
}

template<>
inline int64_t castKeyToInt<KeyType::Int>(TypedValue* key) {
  return keyAsRaw<KeyType::Int>(key);
}

/**
 * SetElem when base is a String
 */
template <bool setResult, KeyType keyType>
inline StringData* SetElemString(TypedValue* base, TypedValue* key,
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
inline void SetElemObject(TypedValue* base, TypedValue* key, Cell* value) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  if (LIKELY(base->m_data.pobj->isCollection())) {
    collectionSet(base->m_data.pobj, key, (TypedValue*)value);
  } else {
    objOffsetSet(instanceFromTv(base), tvAsCVarRef(key), (TypedValue*)value);
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
inline ArrayData* SetElemArrayRawKey(ArrayData* a,
                                     int64_t key,
                                     Cell* value,
                                     bool copy) {
  return a->set(key, cellAsCVarRef(*value), copy);
}

/**
 * SetElem helper with Array base and String key
 */
inline ArrayData* SetElemArrayRawKey(ArrayData* a,
                                     StringData* key,
                                     Cell* value,
                                     bool copy) {
  int64_t n;
  if (key->isStrictlyInteger(n)) {
    return a->set(n, cellAsCVarRef(*value), copy);
  }
  return a->set(StrNR(key), cellAsCVarRef(*value), copy);
}

/**
 * SetElem when base is an Array
 */
template <bool setResult, KeyType keyType>
inline void SetElemArray(TypedValue* base, TypedValue* key,
                         Cell* value) {
  ArrayData* a = base->m_data.parr;
  ArrayData* newData = a;
  bool copy = (a->hasMultipleRefs())
    || (value->m_type == KindOfArray && value->m_data.parr == a);

  if (keyType != KeyType::Any) {
    newData = SetElemArrayRawKey(a, keyAsRaw<keyType>(key), value, copy);
  } else if (key->m_type <= KindOfNull) {
    newData = a->set(empty_string, cellAsCVarRef(*value), copy);
  } else if (IS_STRING_TYPE(key->m_type)) {
    newData = SetElemArrayRawKey(a, key->m_data.pstr, value, copy);
  } else if (key->m_type != KindOfArray && key->m_type != KindOfObject) {
    newData = SetElemArrayRawKey(a, tvAsCVarRef(key).toInt64(), value, copy);
  } else {
    raise_warning("Illegal offset type");
    // Assignment failed, so the result is null rather than the RHS.
    // XXX This does not match bytecode.specification, but it does
    // roughly match Zend behavior.
    if (setResult) {
      tvRefcountedDecRef(value);
      tvWriteNull(value);
    } else {
      throw InvalidSetMException(make_tv<KindOfNull>());
    }
  }

  arrayRefShuffle<true>(a, newData, base);
}

/**
 * SetElem() leaves the result in 'value', rather than returning it as in
 * SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
 * get around.
 */
template <bool setResult, KeyType keyType = KeyType::Any>
NEVER_INLINE
StringData* SetElemSlow(TypedValue* base, TypedValue* key, Cell* value) {
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
inline StringData* SetElem(TypedValue* base, TypedValue* key, Cell* value) {
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
                                     TypedValue* key, Cell* rhs) {
  assert(cellIsPlausible(*base));

  Array a = Array::Create();
  TypedValue* result = const_cast<TypedValue*>(a.lvalAt(tvAsCVarRef(key))
                                               .asTypedValue());
  tvAsVariant(base) = a;
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(key).toString().data());
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
template <KeyType keyType = KeyType::Any>
inline TypedValue* SetOpElem(TypedValue& tvScratch, TypedValue& tvRef,
                             SetOpOp op, TypedValue* base,
                             TypedValue* key, Cell* rhs) {
  TypedValue scratch;
  TypedValue* result;

  DataType type;
  opPre(base, type);

  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    initScratchKey<keyType>(scratch, key);
    result = SetOpElemEmptyish(op, base, key, rhs);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      result = SetOpElemNumberish(tvScratch);
    } else {
      initScratchKey<keyType>(scratch, key);
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
    initScratchKey<keyType>(scratch, key);
    result = SetOpElemEmptyish(op, base, key, rhs);
    break;
  }
  case KindOfArray: {
    result = ElemDArray<MoreWarnings, keyType>(base, key);
    result = tvToCell(result);
    SETOP_BODY_CELL(result, op, rhs);
    break;
  }
  case KindOfObject: {
    initScratchKey<keyType>(scratch, key);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
      SETOP_BODY(result, op, rhs);
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
      SETOP_BODY(result, op, rhs);
      objOffsetSet(instanceFromTv(base), tvAsCVarRef(key), result, false);
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
    if (LIKELY(base->m_data.pobj->isCollection())) {
      raise_error("Cannot use [] for reading");
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
    ActRec* fp = g_vmContext->m_fp;
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

  switch (op) {
  case IncDecOp::PreInc:
    cellInc(*fr);
    if (setResult) {
      cellDup(*fr, *to);
    }
    return;
  case IncDecOp::PostInc:
    if (setResult) {
      cellDup(*fr, *to);
    }
    cellInc(*fr);
    return;
  case IncDecOp::PreDec:
    cellDec(*fr);
    if (setResult) {
      cellDup(*fr, *to);
    }
    return;
  case IncDecOp::PostDec:
    if (setResult) {
      cellDup(*fr, *to);
    }
    cellDec(*fr);
    return;
  }
  not_reached();
}

template <bool setResult>
inline void IncDecBody(IncDecOp op, TypedValue* fr, TypedValue* to) {
  if (UNLIKELY(fr->m_type != KindOfInt64)) {
    return incDecBodySlow<setResult>(op, fr, to);
  }

  switch (op) {
  case IncDecOp::PreInc:
    ++fr->m_data.num;
    if (setResult) {
      cellCopy(*fr, *to);
    }
    return;
  case IncDecOp::PostInc:
    if (setResult) {
      cellCopy(*fr, *to);
    }
    ++fr->m_data.num;
    return;
  case IncDecOp::PreDec:
    --fr->m_data.num;
    if (setResult) {
      cellCopy(*fr, *to);
    }
    return;
  case IncDecOp::PostDec:
    if (setResult) {
      cellCopy(*fr, *to);
    }
    --fr->m_data.num;
    return;
  }
  not_reached();
}

template <bool setResult>
inline void IncDecElemEmptyish(IncDecOp op, TypedValue* base,
                               TypedValue* key, TypedValue& dest) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt(tvAsCVarRef(key));
  tvAsVariant(base) = a;
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(key).toString().data());
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
template <bool setResult, KeyType keyType = KeyType::Any>
inline void IncDecElem(TypedValue& tvScratch, TypedValue& tvRef,
                       IncDecOp op, TypedValue* base,
                       TypedValue* key, TypedValue& dest) {
  TypedValue scratch;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    initScratchKey<keyType>(scratch, key);
    IncDecElemEmptyish<setResult>(op, base, key, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecElemNumberish<setResult>(dest);
    } else {
      initScratchKey<keyType>(scratch, key);
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
    initScratchKey<keyType>(scratch, key);
    IncDecElemEmptyish<setResult>(op, base, key, dest);
    break;
  }
  case KindOfArray: {
    TypedValue* result = ElemDArray<MoreWarnings, keyType>(base, key);
    IncDecBody<setResult>(op, result, &dest);
    break;
  }
  case KindOfObject: {
    TypedValue* result;
    initScratchKey<keyType>(scratch, key);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
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
    if (LIKELY(base->m_data.pobj->isCollection())) {
      raise_error("Cannot use [] for reading");
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
inline ArrayData* UnsetElemArrayRawKey(ArrayData* a, int64_t key,
                                       bool copy) {
  return a->remove(key, copy);
}

/**
 * UnsetElemArray when key is a String
 */
inline ArrayData* UnsetElemArrayRawKey(ArrayData* a, StringData* key,
                                       bool copy) {
  int64_t n;
  if (!key->isStrictlyInteger(n)) {
    return a->remove(StrNR(key), copy);
  } else {
    return a->remove(n, copy);
  }
}

/**
 * UnsetElem when base is an Array
 */
template <KeyType keyType>
inline void UnsetElemArray(TypedValue* base, TypedValue* key) {
  ArrayData* a = base->m_data.parr;
  ArrayData* a2;
  bool copy = a->hasMultipleRefs();
  if (keyType == KeyType::Any) {
    if (IS_STRING_TYPE(key->m_type)) {
      a2 = UnsetElemArrayRawKey(a, key->m_data.pstr, copy);
    } else if (key->m_type == KindOfInt64) {
      a2 = UnsetElemArrayRawKey(a, key->m_data.num, copy);
    } else {
      VarNR varKey = tvAsCVarRef(key).toKey();
      if (varKey.isNull()) {
        return;
      }
      a2 = a->remove(varKey, copy);
    }
  } else {
    a2 = UnsetElemArrayRawKey(a, keyAsRaw<keyType>(key), copy);
  }
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
template <KeyType keyType = KeyType::Any>
NEVER_INLINE
void UnsetElemSlow(TypedValue* base, TypedValue* member) {
  TypedValue scratch;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfStaticString:
  case KindOfString: {
    raise_error(Strings::CANT_UNSET_STRING);
  }
  case KindOfArray: {
    UnsetElemArray<keyType>(base, member);
    break;
  }
  case KindOfObject: {
    initScratchKey<keyType>(scratch, member);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      collectionUnset(base->m_data.pobj, member);
    } else {
      objOffsetUnset(instanceFromTv(base), tvAsCVarRef(member));
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
inline void UnsetElem(TypedValue* base, TypedValue* member) {
  if (LIKELY(base->m_type == KindOfArray)) {
    UnsetElemArray<keyType>(base, member);
    return;
  }
  return UnsetElemSlow<keyType>(base, member);
}

/**
 * IssetEmptyElem when base is an Object
 */
template<bool useEmpty, KeyType keyType>
inline bool IssetEmptyElemObj(TypedValue& tvRef, ObjectData* instance,
                              TypedValue* key) {
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  if (useEmpty) {
    if (LIKELY(instance->isCollection())) {
      return collectionEmpty(instance, key);
    } else {
      return objOffsetEmpty(tvRef, instance, cellAsCVarRef(*key));
    }
  } else {
    if (LIKELY(instance->isCollection())) {
      return collectionIsset(instance, key);
    } else {
      return objOffsetIsset(tvRef, instance, cellAsCVarRef(*key));
    }
  }
}

/**
 * IssetEmptyElem when base is a String
 */
template <bool useEmpty, KeyType keyType>
inline bool IssetEmptyElemString(TypedValue& tvScratch, TypedValue* base,
                                 TypedValue* key) {
  // TODO Task #2716479: Fix this so that the warnings raised match
  // Zend PHP.
  TypedValue scratch;
  initScratchKey<keyType>(scratch, key);
  int64_t x;
  if (LIKELY(key->m_type == KindOfInt64)) {
    x = key->m_data.num;
  } else {
    TypedValue tv;
    cellDup(*key, tv);
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
inline bool IssetEmptyElemArray(TypedValue* base, TypedValue* key) {
  TypedValue* result = ElemArray<false, keyType>(base->m_data.parr, key);
  if (useEmpty) {
    return !cellToBool(*tvToCell(result));
  }
  return !cellIsNull(tvToCell(result));
}

/**
 * isset/empty($base[$key])
 */
template <bool useEmpty, KeyType keyType = KeyType::Any>
NEVER_INLINE
bool IssetEmptyElemSlow(TypedValue& tvScratch, TypedValue& tvRef,
                        TypedValue* base, TypedValue* key) {
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
                           TypedValue* base, TypedValue* key) {
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
inline ObjectData* createDefaultObject() {
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);
  ObjectData* obj = newInstance(SystemLib::s_stdclassClass);
  return obj;
}
template <bool warn, bool define>
inline DataType propPreStdclass(TypedValue& tvScratch,
                                TypedValue*& result, TypedValue* base) {
  if (!define) {
    return propPreNull<warn>(tvScratch, result);
  }
  // TODO(#1124706): We don't want to do this anymore.
  ObjectData* obj = createDefaultObject();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  obj->incRefCount();
  result = base;
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
                        Class* ctx, TypedValue* base, TypedValue* key) {
  static_assert(keyType != KeyType::Int,
                "Integer property keys are not supported");
  assert(!warn || !unset);
  TypedValue* result = nullptr;
  StringData* keySD = nullptr;
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

  keySD = prepareKey<keyType>(key);
  // Get property.
  result = &tvScratch;
#define ARGS result, tvRef, ctx, keySD
  if (!warn && !(define || unset)) instance->prop  (ARGS);
  if (!warn &&  (define || unset)) instance->propD (ARGS);
  if ( warn && !define           ) instance->propW (ARGS);
  if ( warn &&  define           ) instance->propWD(ARGS);
#undef ARGS
  releaseKey<keyType>(keySD);
  return result;
}

template <bool useEmpty, KeyType keyType>
inline bool IssetEmptyPropObj(Class* ctx, ObjectData* instance,
                              TypedValue* key) {
  StringData* keySD;
  bool issetEmptyResult;
  keySD = prepareKey<keyType>(key);
  issetEmptyResult = useEmpty ?
    instance->propEmpty(ctx, keySD) :
    instance->propIsset(ctx, keySD);
  releaseKey<keyType>(keySD);
  return issetEmptyResult;
}

template <bool useEmpty, bool isObj = false, KeyType keyType = KeyType::Any>
inline bool IssetEmptyProp(Class* ctx, TypedValue* base, TypedValue* key) {
  if (isObj) {
    return IssetEmptyPropObj<useEmpty, keyType>(
      ctx, reinterpret_cast<ObjectData*>(base), key);
  }

  TypedValue tvScratch;
  TypedValue* result = nullptr;
  DataType t = propPre<false, false, true>(tvScratch, result, base);
  if (t == KindOfNull) {
    return useEmpty;
  }
  if (t == KindOfObject) {
    return IssetEmptyPropObj<useEmpty, keyType>(ctx, instanceFromTv(base), key);
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

inline void SetPropStdclass(TypedValue* base, TypedValue* key,
                            Cell* val) {
  // createDefaultObject could re-enter and clobber base.
  auto const baseCopy = *base;

  ObjectData* obj = createDefaultObject();
  obj->incRefCount();
  StringData* keySD = prepareKey(key);
  obj->setProp(nullptr, keySD, (TypedValue*)val);
  decRefStr(keySD);
  tvRefcountedDecRef(baseCopy);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
}

template <KeyType keyType>
inline void SetPropObj(Class* ctx, ObjectData* instance,
                       TypedValue* key, Cell* val) {
  StringData* keySD = prepareKey<keyType>(key);
  // Set property.
  instance->setProp(ctx, keySD, val);
  releaseKey<keyType>(keySD);
}

// $base->$key = $val
template <bool setResult, bool isObj = false, KeyType keyType = KeyType::Any>
inline void SetProp(Class* ctx, TypedValue* base, TypedValue* key, Cell* val) {
  if (isObj) {
    SetPropObj<keyType>(ctx, reinterpret_cast<ObjectData*>(base), key, val);
    return;
  }

  TypedValue scratch;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    initScratchKey<keyType>(scratch, key);
    SetPropStdclass(base, key, val);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      SetPropNull<setResult>(val);
    } else {
      initScratchKey<keyType>(scratch, key);
      SetPropStdclass(base, key, val);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      SetPropNull<setResult>(val);
    } else {
      initScratchKey<keyType>(scratch, key);
      SetPropStdclass(base, key, val);
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
                                     TypedValue* base, TypedValue* key,
                                     Cell* rhs) {
  ObjectData* obj = createDefaultObject();
  obj->incRefCount();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;

  StringData* keySD = prepareKey(key);
  tvWriteNull(&tvRef);
  SETOP_BODY(&tvRef, op, rhs);
  obj->setProp(nullptr, keySD, &tvRef);
  decRefStr(keySD);
  return &tvRef;
}

template <KeyType keyType>
inline TypedValue* SetOpPropObj(TypedValue& tvRef, Class* ctx,
                                SetOpOp op, ObjectData* instance,
                                TypedValue* key, Cell* rhs) {
  StringData* keySD = prepareKey<keyType>(key);
  TypedValue* result = instance->setOpProp(tvRef, ctx, op, keySD, rhs);
  releaseKey<keyType>(keySD);
  return result;
}

// $base->$key <op>= $rhs
template<bool isObj = false, KeyType keyType = KeyType::Any>
inline TypedValue* SetOpProp(TypedValue& tvScratch, TypedValue& tvRef,
                             Class* ctx, SetOpOp op,
                             TypedValue* base, TypedValue* key,
                             Cell* rhs) {
  TypedValue scratch;
  if (isObj) {
    return SetOpPropObj<keyType>(tvRef, ctx, op,
                                 reinterpret_cast<ObjectData*>(base),
                                 key, rhs);
  }

  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    initScratchKey<keyType>(scratch, key);
    result = SetOpPropStdclass(tvRef, op, base, key, rhs);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      result = SetOpPropNull(tvScratch);
    } else {
      initScratchKey<keyType>(scratch, key);
      result = SetOpPropStdclass(tvRef, op, base, key, rhs);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      result = SetOpPropNull(tvScratch);
    } else {
      initScratchKey<keyType>(scratch, key);
      result = SetOpPropStdclass(tvRef, op, base, key, rhs);
    }
    break;
  }
  case KindOfObject: {
    result = SetOpPropObj<keyType>(tvRef, ctx, op, instanceFromTv(base), key,
                                   rhs);
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
                               TypedValue* key, TypedValue& dest) {
  ObjectData* obj = createDefaultObject();
  obj->incRefCount();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;

  StringData* keySD = prepareKey(key);
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
  decRefStr(keySD);
}

template <bool setResult, KeyType keyType>
inline void IncDecPropObj(TypedValue& tvRef, Class* ctx,
                          IncDecOp op, ObjectData* base,
                          TypedValue* key, TypedValue& dest) {
  StringData* keySD = prepareKey<keyType>(key);
  base->incDecProp<setResult>(tvRef, ctx, op, keySD, dest);
  releaseKey<keyType>(keySD);
}

template <bool setResult, bool isObj = false, KeyType keyType = KeyType::Any>
inline void IncDecProp(TypedValue& tvScratch, TypedValue& tvRef,
                       Class* ctx, IncDecOp op,
                       TypedValue* base, TypedValue* key,
                       TypedValue& dest) {
  TypedValue scratch;
  if (isObj) {
    IncDecPropObj<setResult, keyType>(tvRef, ctx, op,
                                      reinterpret_cast<ObjectData*>(base),
                                      key, dest);
    return;
  }

  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    initScratchKey<keyType>(scratch, key);
    IncDecPropStdclass<setResult>(op, base, key, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecPropNull<setResult>(dest);
    } else {
      initScratchKey<keyType>(scratch, key);
      IncDecPropStdclass<setResult>(op, base, key, dest);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      IncDecPropNull<setResult>(dest);
    } else {
      initScratchKey<keyType>(scratch, key);
      IncDecPropStdclass<setResult>(op, base, key, dest);
    }
    break;
  }
  case KindOfObject: {
    IncDecPropObj<setResult, keyType>(tvRef, ctx, op, instanceFromTv(base),
                                      key, dest);
    break;
  }
  default: {
    IncDecPropNull<setResult>(dest);
    break;
  }
  }
}

template<bool isObj = false, KeyType keyType = KeyType::Any>
inline void UnsetProp(Class* ctx, TypedValue* base, TypedValue* key) {
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
  StringData* keySD = prepareKey<keyType>(key);
  // Unset property.
  instance->unsetProp(ctx, keySD);
  releaseKey<keyType>(keySD);
}

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_VM_MEMBER_OPERATIONS_H_
