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
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/base/tv_conversions.h"

namespace HPHP {

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

inline ALWAYS_INLINE void opPre(TypedValue*& base, DataType& type) {
  // Get inner variant if necessary.
  type = base->m_type;
  if (type == KindOfRef) {
    base = base->m_data.pref->tv();
    type = base->m_type;
  }
}

inline TypedValue* ElemArrayRawKey(ArrayData* base, int64_t key) {
  TypedValue* result = base->nvGet(key);
  return result ? result : (TypedValue*)&null_variant;
}

inline TypedValue* ElemArrayRawKey(ArrayData* base, StringData* key) {
  int64_t n;
  TypedValue* result = !key->isStrictlyInteger(n) ? base->nvGet(key) :
                       base->nvGet(n);
  return result ? result : (TypedValue*)&null_variant;
}

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
      result = (TypedValue*)&ArrNR(base).asArray()
        .rvalAtRef(cellAsCVarRef(*key));
    }
  } else {
    result = ElemArrayRawKey(base, keyAsRaw<keyType>(key));
  }

  if (UNLIKELY(result->m_type == KindOfUninit)) {
    result = (TypedValue*)&init_null_variant;
    if (warn) {
      TypedValue scratch;
      initScratchKey<keyType>(scratch, key);
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(key).toString().data());
    }
  }
  return result;
}

// $result = $base[$key];
template <bool warn, KeyType keyType = KeyType::Any>
inline TypedValue* Elem(TypedValue& tvScratch, TypedValue& tvRef,
                        TypedValue* base, TypedValue* key) {
  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
    }
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
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
      static StringData* sd = StringData::GetStaticString("");
      tvScratch.m_data.pstr = sd;
      tvScratch.m_type = KindOfString;
    } else {
      tvScratch.m_data.pstr = base->m_data.pstr->getChar(x);
      assert(tvScratch.m_data.pstr->isStatic());
      tvScratch.m_type = KindOfStaticString;
    }
    result = &tvScratch;
    break;
  }
  case KindOfArray: {
    result = ElemArray<warn, keyType>(base->m_data.parr, key);
    break;
  }
  case KindOfObject: {
    TypedValue scratch;
    initScratchKey<keyType>(scratch, key);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
    }
    break;
  }
  default: {
    assert(false);
    result = nullptr;
  }
  }
  return result;
}

template <bool warn, KeyType keyType>
inline TypedValue* ElemDArray(TypedValue* base, TypedValue* key) {
  TypedValue* result;
  bool defined = !warn ||
    tvAsCVarRef(base).asCArrRef().exists(keyAsValue<keyType>(key));

  if (keyType == KeyType::Any && key->m_type == KindOfInt64) {
    result = (TypedValue*)&tvAsVariant(base).asArrRef()
                                            .lvalAt(key->m_data.num);
  } else {
    result = (TypedValue*)&tvAsVariant(base).asArrRef()
                                            .lvalAt(keyAsValue<keyType>(key));
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

// $base[$key] = ...
// \____ ____/
//      v
//   $result
template <bool warn, bool reffy, KeyType keyType = KeyType::Any>
inline TypedValue* ElemD(TypedValue& tvScratch, TypedValue& tvRef,
                         TypedValue* base, TypedValue* key) {
  TypedValue scratch;
  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    initScratchKey<keyType>(scratch, key);
    Array a = Array::Create();
    result = (TypedValue*)&a.lvalAt(cellAsCVarRef(*key));
    if (warn) {
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(key).toString().data());
    }
    tvAsVariant(base) = a;
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
      tvWriteUninit(&tvScratch);
      result = &tvScratch;
    } else {
      initScratchKey<keyType>(scratch, key);
      Array a = Array::Create();
      result = (TypedValue*)&a.lvalAt(cellAsCVarRef(*key));
      if (warn) {
        raise_notice(Strings::UNDEFINED_INDEX,
                     tvAsCVarRef(key).toString().data());
      }
      tvAsVariant(base) = a;
    }
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
    tvWriteUninit(&tvScratch);
    result = &tvScratch;
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() == 0) {
      initScratchKey<keyType>(scratch, key);
      Array a = Array::Create();
      result = (TypedValue*)&a.lvalAt(cellAsCVarRef(*key));
      if (warn) {
        raise_notice(Strings::UNDEFINED_INDEX,
                     tvAsCVarRef(key).toString().data());
      }
      tvAsVariant(base) = a;
    } else {
      raise_error("Operator not supported for strings");
      result = nullptr; // Silence compiler warning.
    }
    break;
  }
  case KindOfArray: {
    result = ElemDArray<warn, keyType>(base, key);
    break;
  }
  case KindOfObject: {
    initScratchKey<keyType>(scratch, key);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      if (reffy) {
        raise_error("Collection elements cannot be taken by reference");
        result = nullptr;
      } else {
        result = collectionGet(base->m_data.pobj, key);
      }
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
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

// $base[$key] = ...
// \____ ____/
//      v
//   $result
template <KeyType keyType = KeyType::Any>
inline TypedValue* ElemU(TypedValue& tvScratch, TypedValue& tvRef,
                         TypedValue* base, TypedValue* key) {
  TypedValue* result = nullptr;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble: {
    // Unset on a null base never modifies the base, but the
    // const_cast is necessary to placate the type system.
    result = const_cast<TypedValue*>(null_variant.asTypedValue());
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    raise_error(Strings::OP_NOT_SUPPORTED_STRING);
    break;
  }
  case KindOfArray: {
    bool defined =
      tvAsCVarRef(base).asCArrRef().exists(keyAsValue<keyType>(key));
    if (defined) {
      if (keyType == KeyType::Any && key->m_type == KindOfInt64) {
        result = (TypedValue*)&tvAsVariant(base).asArrRef()
                                                .lvalAt(key->m_data.num);
      } else {
        result = (TypedValue*)&tvAsVariant(base).asArrRef().lvalAt(
          keyAsValue<keyType>(key));
      }
    } else {
      tvWriteUninit(&tvScratch);
      result = &tvScratch;
    }
    break;
  }
  case KindOfObject: {
    TypedValue scratch;
    initScratchKey<keyType>(scratch, key);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), cellAsCVarRef(*key));
    }
    break;
  }
  default: {
    not_reached();
  }
  }
  return result;
}

// $result = ($base[] = ...);
inline TypedValue* NewElem(TypedValue& tvScratch, TypedValue& tvRef,
                           TypedValue* base) {
  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    Array a = Array::Create();
    result = (TypedValue*)&a.lvalAt();
    tvAsVariant(base) = a;
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      raise_warning("Invalid NewElem operand");
      tvWriteUninit(&tvScratch);
      result = &tvScratch;
    } else {
      Array a = Array::Create();
      result = (TypedValue*)&a.lvalAt();
      tvAsVariant(base) = a;
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() == 0) {
      Array a = Array::Create();
      result = (TypedValue*)&a.lvalAt();
      tvAsVariant(base) = a;
    } else {
      raise_warning("Invalid NewElem operand");
      tvWriteUninit(&tvScratch);
      result = &tvScratch;
    }
    break;
  }
  case KindOfArray: {
    result = (TypedValue*)&tvAsVariant(base).asArrRef().lvalAt();
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      raise_error("Cannot use [] for reading");
      result = nullptr;
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), init_null_variant);
    }
    break;
  }
  default: {
    raise_warning("Invalid NewElem operand");
    tvWriteUninit(&tvScratch);
    result = &tvScratch;
    break;
  }
  }
  return result;
}

inline void SetElemEmptyish(TypedValue* base, TypedValue* key,
                            Cell* value) {
  Array a = Array::Create();
  a.set(tvAsCVarRef(key), tvAsCVarRef(value));
  tvAsVariant(base) = a;
}
template <bool setResult>
inline void SetElemNumberish(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  // XXX Flip?
  if (setResult) {
    tvRefcountedDecRefCell((TypedValue*)value);
    tvWriteNull((TypedValue*)value);
  } else {
    throw InvalidSetMException(make_tv<KindOfNull>());
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

inline ArrayData* SetElemArrayRawKey(ArrayData* a,
                                     int64_t key,
                                     Cell* value,
                                     bool copy) {
  return a->set(key, cellAsCVarRef(*value), copy);
}

inline ArrayData* SetElemArrayRawKey(ArrayData* a,
                                     StringData* key,
                                     Cell* value,
                                     bool copy) {
  int64_t n;
  if (key->isStrictlyInteger(n)) {
    return a->set(n, cellAsCVarRef(*value), copy);
  } else {
    return a->set(StrNR(key), cellAsCVarRef(*value), copy);
  }
}

template <bool setResult, KeyType keyType>
inline void SetElemArray(TypedValue* base, TypedValue* key,
                         Cell* value) {
  ArrayData* a = base->m_data.parr;
  ArrayData* newData = a;
  bool copy = (a->getCount() > 1)
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

// SetElem() leaves the result in 'value', rather than returning it as in
// SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
// get around.
template <bool setResult, KeyType keyType = KeyType::Any>
inline StringData* SetElem(TypedValue* base, TypedValue* key, Cell* value) {
  TypedValue scratch;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    initScratchKey<keyType>(scratch, key);
    SetElemEmptyish(base, key, value);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      SetElemNumberish<setResult>(value);
    } else {
      initScratchKey<keyType>(scratch, key);
      SetElemEmptyish(base, key, value);
    }
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    SetElemNumberish<setResult>(value);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    int baseLen = base->m_data.pstr->size();
    if (baseLen == 0) {
      initScratchKey<keyType>(scratch, key);
      SetElemEmptyish(base, key, value);
      if (!setResult) {
        tvRefcountedIncRef(value);
        throw InvalidSetMException(*value);
      }
    } else {
      // Convert key to string offset.
      int64_t x = castKeyToInt<keyType>(key);
      if (UNLIKELY(x < 0 || x >= StringData::MaxSize)) {
        // Can't use PRId64 here because of order of inclusion issues
        raise_warning("Illegal string offset: %lld", (long long)x);
        if (!setResult) {
          throw InvalidSetMException(make_tv<KindOfNull>());
        } else {
          tvRefcountedDecRef(value);
          tvWriteNull(value);
          break;
        }
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
      if (x >= 0 && x < baseLen && base->m_data.pstr->getCount() <= 1) {
        // Modify base in place.  This is safe because the LHS owns the only
        // reference.
        base->m_data.pstr->setChar(x, y[0]);
      } else {
        StringData* sd = NEW(StringData)(slen);
        char* s = sd->mutableSlice().ptr;
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

      StringData* sd = NEW(StringData)(y, strlen(y), CopyString);
      sd->incRefCount();
      return sd;
    }
    break;
  }
  case KindOfArray: {
    SetElemArray<setResult, keyType>(base, key, value);
    break;
  }
  case KindOfObject: {
    initScratchKey<keyType>(scratch, key);
    if (LIKELY(base->m_data.pobj->isCollection())) {
      collectionSet(base->m_data.pobj, key, (TypedValue*)value);
    } else {
      objOffsetSet(instanceFromTv(base), tvAsCVarRef(key), (TypedValue*)value);
    }
    break;
  }
  default: not_reached();
  }

  return nullptr;
}

inline void SetNewElemEmptyish(TypedValue* base, Cell* value) {
  Array a = Array::Create();
  a.append(cellAsCVarRef(*value));
  tvAsVariant(base) = a;
}
template <bool setResult>
inline void SetNewElemNumberish(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (setResult) {
    tvRefcountedDecRefCell((TypedValue*)value);
    tvWriteNull((TypedValue*)value);
  } else {
    throw InvalidSetMException(make_tv<KindOfNull>());
  }
}
template <bool setResult>
inline void SetNewElem(TypedValue* base, Cell* value) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    SetNewElemEmptyish(base, value);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      SetNewElemNumberish<setResult>(value);
    } else {
      SetNewElemEmptyish(base, value);
    }
    break;
  }
  case KindOfInt64:
  case KindOfDouble: {
    SetNewElemNumberish<setResult>(value);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    int baseLen = base->m_data.pstr->size();
    if (baseLen == 0) {
      SetNewElemEmptyish(base, value);
    } else {
      raise_error("[] operator not supported for strings");
    }
    break;
  }
  case KindOfArray: {
    ArrayData* a = base->m_data.parr;
    bool copy = (a->getCount() > 1)
                || (value->m_type == KindOfArray && value->m_data.parr == a);
    ArrayData* a2 = a->append(cellAsCVarRef(*value), copy);
    if (a2 != a) {
      a2->incRefCount();
      base->m_data.parr->decRefCount();
      base->m_data.parr = a2;
    }
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      collectionAppend(base->m_data.pobj, (TypedValue*)value);
    } else {
      objOffsetAppend(instanceFromTv(base), (TypedValue*)value);
    }
    break;
  }
  default: assert(false);
  }
}

inline TypedValue* SetOpElemEmptyish(unsigned char op, TypedValue* base,
                                     TypedValue* key, Cell* rhs) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt(tvAsCVarRef(key));
  tvAsVariant(base) = a;
  if (MoreWarnings) {
    raise_notice(Strings::UNDEFINED_INDEX,
                 tvAsCVarRef(key).toString().data());
  }
  SETOP_BODY(result, op, rhs);
  return result;
}
inline TypedValue* SetOpElemNumberish(TypedValue& tvScratch) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvScratch);
  return &tvScratch;
}

template <KeyType keyType = KeyType::Any>
inline TypedValue* SetOpElem(TypedValue& tvScratch, TypedValue& tvRef,
                             unsigned char op, TypedValue* base,
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
    SETOP_BODY(result, op, rhs);
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

inline TypedValue* SetOpNewElemEmptyish(unsigned char op,
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
                                unsigned char op, TypedValue* base,
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
void incDecBodySlow(unsigned char op, TypedValue* fr, TypedValue* to) {
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

  switch (static_cast<IncDecOp>(op)) {
  case PreInc:
    cellInc(*fr);
    if (setResult) {
      cellDup(*fr, *to);
    }
    break;
  case PostInc:
    if (setResult) {
      cellDup(*fr, *to);
    }
    cellInc(*fr);
    break;
  case PreDec:
    cellDec(*fr);
    if (setResult) {
      cellDup(*fr, *to);
    }
    break;
  case PostDec:
    if (setResult) {
      cellDup(*fr, *to);
    }
    cellDec(*fr);
    break;
  case IncDec_invalid:
    not_reached();
  }
}

template <bool setResult>
inline void IncDecBody(unsigned char op, TypedValue* fr, TypedValue* to) {
  if (UNLIKELY(fr->m_type != KindOfInt64)) {
    return incDecBodySlow<setResult>(op, fr, to);
  }

  switch (static_cast<IncDecOp>(op)) {
  case PreInc:
    ++fr->m_data.num;
    if (setResult) {
      cellCopy(*fr, *to);
    }
    return;
  case PostInc:
    if (setResult) {
      cellCopy(*fr, *to);
    }
    ++fr->m_data.num;
    return;
  case PreDec:
    --fr->m_data.num;
    if (setResult) {
      cellCopy(*fr, *to);
    }
    return;
  case PostDec:
    if (setResult) {
      cellCopy(*fr, *to);
    }
    --fr->m_data.num;
    return;
  case IncDec_invalid:
    break;
  }
  not_reached();
}

template <bool setResult>
inline void IncDecElemEmptyish(unsigned char op, TypedValue* base,
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
                       unsigned char op, TypedValue* base,
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
inline void IncDecNewElemEmptyish(unsigned char op, TypedValue* base,
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
                          unsigned char op, TypedValue* base,
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

inline ArrayData* UnsetElemArrayRawKey(ArrayData* a, int64_t key,
                                       bool copy) {
  return a->remove(key, copy);
}

inline ArrayData* UnsetElemArrayRawKey(ArrayData* a, StringData* key,
                                       bool copy) {
  int64_t n;
  if (!key->isStrictlyInteger(n)) {
    return a->remove(StrNR(key), copy);
  } else {
    return a->remove(n, copy);
  }
}

template <KeyType keyType>
inline void UnsetElemArray(TypedValue* base, TypedValue* key) {
  ArrayData* a = base->m_data.parr;
  ArrayData* a2;
  bool copy = a->getCount() > 1;
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
    base->m_data.parr->decRefCount();
    base->m_data.parr = a2;
  }
}

template <KeyType keyType = KeyType::Any>
inline void UnsetElem(TypedValue* base, TypedValue* member) {
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

template<bool useEmpty>
inline bool IssetEmptyElemObj(TypedValue& tvRef, ObjectData* instance,
                              TypedValue* key) {
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

template <bool useEmpty, bool isObj = false, KeyType keyType = KeyType::Any>
inline bool IssetEmptyElem(TypedValue& tvScratch, TypedValue& tvRef,
                           TypedValue* base, TypedValue* key) {
  TypedValue scratch;
  if (isObj) {
    initScratchKey<keyType>(scratch, key);
    return IssetEmptyElemObj<useEmpty>(
      tvRef, reinterpret_cast<ObjectData*>(base), key);
  }

  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfStaticString:
  case KindOfString: {
    // TODO Task #XXXXXXX: Fix this so that the warnings raised match
    // Zend PHP.
    TypedValue tv;
    initScratchKey<keyType>(scratch, key);
    tvDup(*key, tv);
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
    int64_t x = tv.m_data.num;
    if (x < 0 || x >= base->m_data.pstr->size()) {
      return useEmpty;
    }
    if (!useEmpty) {
      return true;
    }
    tvScratch.m_data.pstr = base->m_data.pstr->getChar(x);
    assert(tvScratch.m_data.pstr->isStatic());
    tvScratch.m_type = KindOfStaticString;
    result = &tvScratch;
    break;
  }
  case KindOfArray: {
    result = ElemArray<false, keyType>(base->m_data.parr, key);
    break;
  }
  case KindOfObject: {
    initScratchKey<keyType>(scratch, key);
    return IssetEmptyElemObj<useEmpty>(tvRef, base->m_data.pobj, key);
    break;
  }
  default: {
    return useEmpty;
  }
  }

  if (useEmpty) {
    return empty(tvAsCVarRef(result));
  } else {
    return isset(tvAsCVarRef(result));
  }
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
  ObjectData* obj = createDefaultObject();
  obj->incRefCount();
  StringData* keySD = prepareKey(key);
  obj->setProp(nullptr, keySD, (TypedValue*)val);
  decRefStr(keySD);
  tvRefcountedDecRef(base);
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
inline TypedValue* SetOpPropStdclass(TypedValue& tvRef, unsigned char op,
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
                                unsigned char op, ObjectData* instance,
                                TypedValue* key, Cell* rhs) {
  StringData* keySD = prepareKey<keyType>(key);
  TypedValue* result = instance->setOpProp(tvRef, ctx, op, keySD, rhs);
  releaseKey<keyType>(keySD);
  return result;
}

// $base->$key <op>= $rhs
template<bool isObj = false, KeyType keyType = KeyType::Any>
inline TypedValue* SetOpProp(TypedValue& tvScratch, TypedValue& tvRef,
                             Class* ctx, unsigned char op,
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
inline void IncDecPropStdclass(unsigned char op, TypedValue* base,
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
                          unsigned char op, ObjectData* base,
                          TypedValue* key, TypedValue& dest) {
  StringData* keySD = prepareKey<keyType>(key);
  base->incDecProp<setResult>(tvRef, ctx, op, keySD, dest);
  releaseKey<keyType>(keySD);
}

template <bool setResult, bool isObj = false, KeyType keyType = KeyType::Any>
inline void IncDecProp(TypedValue& tvScratch, TypedValue& tvRef,
                       Class* ctx, unsigned char op,
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
