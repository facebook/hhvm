/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_VM_MEMBER_OPERATIONS_H_
#define incl_VM_MEMBER_OPERATIONS_H_

#include "runtime/base/types.h"
#include "runtime/base/strings.h"
#include "runtime/base/tv_macros.h"
#include "system/lib/systemlib.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/runtime.h"
#include "runtime/ext/ext_collection.h"

namespace HPHP {
namespace VM {

// When MoreWarnings is set to true, the VM will raise more warnings
// on SetOpM, IncDecM and CGetG, intended to match Zend.
const bool MoreWarnings =
#ifdef HHVM_MORE_WARNINGS
  true
#else
  false
#endif
  ;

enum KeyType {
  AnyKey = 0,
  StrKey,
  IntKey,
};

template<KeyType kt>
struct KeyTypeTraits {};
template<> struct KeyTypeTraits<AnyKey> {
  typedef CVarRef valType;
  typedef int64 rawType; // This is never actually used but it's
                         // needed to keep the compiler happy
};
template<> struct KeyTypeTraits<StrKey> {
  typedef StrNR valType;
  typedef StringData* rawType;
};
template<> struct KeyTypeTraits<IntKey> {
  typedef int64 valType;
  typedef int64 rawType;
};

static inline DataType keyDataType(KeyType t) {
  if (t == StrKey) {
    return KindOfString;
  } else if (t == IntKey) {
    return KindOfInt64;
  } else {
    not_reached();
  }
}

template<KeyType kt>
inline typename KeyTypeTraits<kt>::valType keyAsValue(TypedValue* key);
template<>
inline int64 keyAsValue<IntKey>(TypedValue* key) {
  return reinterpret_cast<int64>(key);
}
template<>
inline CVarRef keyAsValue<AnyKey>(TypedValue* key) {
  return tvAsCVarRef(key);
}
template<>
inline StrNR keyAsValue<StrKey>(TypedValue* key) {
  return StrNR(reinterpret_cast<StringData*>(key));
}

template<KeyType kt>
inline typename KeyTypeTraits<kt>::rawType keyAsRaw(TypedValue* key) {
  if (kt == AnyKey) {
    not_reached();
  }
  return reinterpret_cast<typename KeyTypeTraits<kt>::rawType>(key);
}

// This is used for helpers that are not type specialized for the key
// and therefore need the key in a TypedValue
template<KeyType kt>
inline void initScratchKey(TypedValue& tv, TypedValue*& key) {
  if (kt != AnyKey) {
    tv.m_type = keyDataType(kt);
    tv.m_data.num = reinterpret_cast<int64>(key);
    key = &tv;
  }
}

void objArrayAccess(Instance* base);
TypedValue* objOffsetGet(TypedValue& tvRef, Instance* base,
                         CVarRef offset, bool validate=true);
bool objOffsetIsset(TypedValue& tvRef, Instance* base, CVarRef offset,
                    bool validate=true);
bool objOffsetEmpty(TypedValue& tvRef, Instance* base, CVarRef offset,
                    bool validate=true);
void objOffsetSet(Instance* base, CVarRef offset, TypedValue* val,
                  bool validate=true);
void objOffsetAppend(Instance* base, TypedValue* val, bool validate=true);
void objOffsetUnset(Instance* base, CVarRef offset);

StringData* prepareAnyKey(TypedValue* tv);

template <KeyType keyType = AnyKey>
static inline StringData* prepareKey(TypedValue* tv) {
  if (keyType == StrKey) {
    return reinterpret_cast<StringData*>(tv);
  } else if (keyType == AnyKey) {
    return prepareAnyKey(tv);
  } else {
    not_reached();
  }
}

template <KeyType keyType>
static inline void releaseKey(StringData* keySD) {
  if (keyType == AnyKey) {
    LITSTR_DECREF(keySD);
  } else {
    ASSERT(keyType == StrKey);
  }
}

static inline void opPre(TypedValue*& base, DataType& type) {
  // Get inner variant if necessary.
  type = base->m_type;
  if (type == KindOfRef) {
    base = base->m_data.pref->tv();
    type = base->m_type;
  }
}

static inline TypedValue* ElemArrayRawKey(ArrayData* base,
                                          int64 key) {
  TypedValue* result;
  if (LIKELY(IsHphpArray(base))) {
    result = static_cast<HphpArray*>(base)->nvGet(key);
    if (result == NULL) {
      result = (TypedValue*)&null_variant;
    }
  } else {
    result = (TypedValue*)&base->get(key);
  }
  return result;
}

static inline TypedValue* ElemArrayRawKey(ArrayData* base,
                                          StringData* key) {
  TypedValue* result;
  if (LIKELY(IsHphpArray(base))) {
    int64 n;
    if (!key->isStrictlyInteger(n)) {
      result = static_cast<HphpArray*>(base)->nvGet(key);
    } else {
      result = static_cast<HphpArray*>(base)->nvGet(n);
    }
    if (result == NULL) {
      result = (TypedValue*)&null_variant;
    }
  } else {
    result = (TypedValue*)&ArrNR(base).asArray().rvalAtRef(StrNR(key));
  }
  return result;
}

template <bool warn, KeyType keyType>
static inline TypedValue* ElemArray(ArrayData* base,
                                    TypedValue* key) {
  TypedValue* result;
  if (keyType == AnyKey) {
    DataType rtt = key->m_type;
    if (rtt == KindOfInt64) {
      result = ElemArrayRawKey(base, key->m_data.num);
    } else if (IS_STRING_TYPE(rtt)) {
      result = ElemArrayRawKey(base, key->m_data.pstr);
    } else {
      result = (TypedValue*)&ArrNR(base).asArray()
        .rvalAtRef(tvCellAsCVarRef(key));
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
template <bool warn, KeyType keyType = AnyKey>
static inline TypedValue* Elem(TypedValue& tvScratch, TypedValue& tvRef,
                               TypedValue* base, bool& baseStrOff,
                               TypedValue* key) {
  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble: {
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (baseStrOff) {
      raise_error("Cannot use string offset as an array");
    }
    int64 x;
    if (keyType == IntKey) {
      x = reinterpret_cast<int64>(key);
    } else if (keyType == StrKey) {
      x = reinterpret_cast<StringData*>(key)->toInt64(10);
    } else {
      x = IS_INT_TYPE(key->m_type) ? key->m_data.num
        : int64(tvCellAsCVarRef(key));
    }
    if (x < 0 || x >= base->m_data.pstr->size()) {
      if (warn) {
        raise_warning("Out of bounds");
      }
      static StringData* sd = StringData::GetStaticString("");
      tvScratch.m_data.pstr = sd;
      tvScratch._count = 0;
      tvScratch.m_type = KindOfString;
    } else {
      TV_WRITE_UNINIT(&tvScratch);
      tvAsVariant(&tvScratch) = base->m_data.pstr->getChar(x);
    }
    result = &tvScratch;
    baseStrOff = true;
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
      result = objOffsetGet(tvRef, instanceFromTv(base), tvCellAsCVarRef(key));
    }
    break;
  }
  default: {
    ASSERT(false);
    result = NULL;
  }
  }
  return result;
}

template <bool warn, KeyType keyType>
static inline TypedValue* ElemDArray(TypedValue* base, TypedValue* key) {
  TypedValue* result;
  bool defined = !warn ||
    tvAsCVarRef(base).asCArrRef().exists(keyAsValue<keyType>(key));

  if (keyType == AnyKey && key->m_type == KindOfInt64) {
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
template <bool warn, bool reffy, KeyType keyType = AnyKey>
static inline TypedValue* ElemD(TypedValue& tvScratch, TypedValue& tvRef,
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
    result = (TypedValue*)&a.lvalAt(tvCellAsCVarRef(key));
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
      result = (TypedValue*)&a.lvalAt(tvCellAsCVarRef(key));
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
      result = (TypedValue*)&a.lvalAt(tvCellAsCVarRef(key));
      if (warn) {
        raise_notice(Strings::UNDEFINED_INDEX,
                     tvAsCVarRef(key).toString().data());
      }
      tvAsVariant(base) = a;
    } else {
      raise_error("Operator not supported for strings");
      result = NULL; // Silence compiler warning.
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
        result = NULL;
      } else {
        result = collectionGet(base->m_data.pobj, key);
      }
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), tvCellAsCVarRef(key));
    }
    break;
  }
  default: {
    ASSERT(false);
    result = NULL; // Silence compiler warning.
  }
  }
  return result;
}

// $base[$key] = ...
// \____ ____/
//      v
//   $result
template <KeyType keyType = AnyKey>
static inline TypedValue* ElemU(TypedValue& tvScratch, TypedValue& tvRef,
                                TypedValue* base, TypedValue* key) {
  TypedValue* result = NULL;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble: {
    tvWriteUninit(&tvScratch);
    result = &tvScratch;
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    raise_error("Operator not supported for strings");
    break;
  }
  case KindOfArray: {
    bool defined =
      tvAsCVarRef(base).asCArrRef().exists(keyAsValue<keyType>(key));
    if (defined) {
      if (keyType == AnyKey && key->m_type == KindOfInt64) {
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
      result = objOffsetGet(tvRef, instanceFromTv(base), tvCellAsCVarRef(key));
    }
    break;
  }
  default: {
    ASSERT(false);
    result = NULL;
  }
  }
  return result;
}

// $result = ($base[] = ...);
static inline TypedValue* NewElem(TypedValue& tvScratch, TypedValue& tvRef,
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
      result = NULL;
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

static inline void SetElemEmptyish(TypedValue* base, TypedValue* key,
                                   Cell* value) {
  Array a = Array::Create();
  a.set(tvAsCVarRef(key), tvAsCVarRef(value));
  tvAsVariant(base) = a;
}
template <bool setResult>
static inline void SetElemNumberish(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (setResult) {
    tvRefcountedDecRefCell((TypedValue*)value);
    tvWriteNull((TypedValue*)value);
  }
}

static inline ArrayData* SetElemArrayRawKey(ArrayData* a,
                                            int64 key,
                                            Cell* value,
                                            bool copy) {
  return a->set(key, tvCellAsCVarRef(value), copy);
}

static inline ArrayData* SetElemArrayRawKey(ArrayData* a,
                                            StringData* key,
                                            Cell* value,
                                            bool copy) {
  int64 n;
  if (key->isStrictlyInteger(n)) {
    return a->set(n, tvCellAsCVarRef(value), copy);
  } else {
    return a->set(StrNR(key), tvCellAsCVarRef(value), copy);
  }
}

template <bool setResult, KeyType keyType>
static inline void SetElemArray(TypedValue* base, TypedValue* key,
                                Cell* value) {
  ArrayData* a = base->m_data.parr;
  ArrayData* newData = NULL;
  bool copy = (a->getCount() > 1)
    || (value->m_type == KindOfArray && value->m_data.parr == a);

  if (keyType != AnyKey) {
    newData = SetElemArrayRawKey(a, keyAsRaw<keyType>(key), value, copy);
  } else if (key->m_type <= KindOfNull) {
    newData = a->set(empty_string, tvCellAsCVarRef(value), copy);
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
      if (IS_REFCOUNTED_TYPE(value->m_type)) {
        tvDecRef(value);
      }
      tvWriteNull(value);
    }
  }

  if (newData != NULL && newData != a) {
    newData->incRefCount();
    if (a->decRefCount() == 0) {
      a->release();
    }
    base->m_data.parr = newData;
  }
}
// SetElem() leaves the result in 'value', rather than returning it as in
// SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
// get around.
template <bool setResult, KeyType keyType = AnyKey>
static inline void SetElem(TypedValue* base, TypedValue* key, Cell* value) {
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
    initScratchKey<keyType>(scratch, key);
    int baseLen = base->m_data.pstr->size();
    if (baseLen == 0) {
      SetElemEmptyish(base, key, value);
    } else {
      // Convert key to string offset.
      int64 x;
      {
        TypedValue tv;
        tvDup(key, &tv);
        tvCastToInt64InPlace(&tv);
        x = tv.m_data.num;
      }
      if (x < 0 || x >= StringData::MaxSize) {
        raise_warning("Illegal string offset: %lld", x);
        break;
      }
      // Compute how long the resulting string will be. Type needs
      // to agree with x.
      int64 slen;
      if (x >= baseLen) {
        slen = x + 1;
      } else {
        slen = baseLen;
      }
      // Extract the first character of (string)value.
      char y[2];
      {
        TypedValue tv;
        tvDup(value, &tv);
        tvCastToStringInPlace(&tv);
        if (tv.m_data.pstr->size() > 0) {
          y[0] = tv.m_data.pstr->data()[0];
          y[1] = '\0';
        } else {
          y[0] = '\0';
        }
        tvRefcountedDecRef(&tv);
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
        if (base->m_data.pstr->decRefCount() == 0) base->m_data.pstr->release();
        base->m_data.pstr = sd;
        base->m_type = KindOfString;
      }
      if (setResult) {
        // Push y onto the stack.
        tvRefcountedDecRef(value);
        StringData* sd = NEW(StringData)(y, strlen(y), CopyString);
        sd->incRefCount();
        value->m_data.pstr = sd;
        value->_count = 0;
        value->m_type = KindOfString;
      }
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
}

static inline void SetNewElemEmptyish(TypedValue* base, Cell* value) {
  Array a = Array::Create();
  a.append(tvCellAsCVarRef(value));
  tvAsVariant(base) = a;
}
template <bool setResult>
static inline void SetNewElemNumberish(Cell* value) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (setResult) {
    tvRefcountedDecRefCell((TypedValue*)value);
    tvWriteNull((TypedValue*)value);
  }
}
template <bool setResult>
static inline void SetNewElem(TypedValue* base, Cell* value) {
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
    a = a->append(tvCellAsCVarRef(value), copy);
    if (a) {
      a->incRefCount();
      base->m_data.parr->decRefCount();
      base->m_data.parr = a;
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
  default: ASSERT(false);
  }
}

static inline TypedValue* SetOpElemEmptyish(unsigned char op, TypedValue* base,
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
static inline TypedValue* SetOpElemNumberish(TypedValue& tvScratch) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvScratch);
  return &tvScratch;
}

template <KeyType keyType = AnyKey>
static inline TypedValue* SetOpElem(TypedValue& tvScratch, TypedValue& tvRef,
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
      result = objOffsetGet(tvRef, instanceFromTv(base), tvCellAsCVarRef(key));
      SETOP_BODY(result, op, rhs);
      objOffsetSet(instanceFromTv(base), tvAsCVarRef(key), result, false);
    }
    break;
  }
  default: {
    ASSERT(false);
    result = NULL; // Silence compiler warning.
  }
  }
  return result;
}

static inline TypedValue* SetOpNewElemEmptyish(unsigned char op,
                                               TypedValue* base, Cell* rhs) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt();
  tvAsVariant(base) = a;
  SETOP_BODY(result, op, rhs);
  return result;
}
static inline TypedValue* SetOpNewElemNumberish(TypedValue& tvScratch) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  tvWriteNull(&tvScratch);
  return &tvScratch;
}
static inline TypedValue* SetOpNewElem(TypedValue& tvScratch, TypedValue& tvRef,
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
      result = NULL;
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), init_null_variant);
      SETOP_BODY(result, op, rhs);
      objOffsetAppend(instanceFromTv(base), result, false);
    }
    break;
  }
  default: {
    ASSERT(false);
    result = NULL; // Silence compiler warning.
  }
  }
  return result;
}

template <bool setResult>
static inline void IncDecBody(unsigned char op, TypedValue* fr,
                              TypedValue* to) {
  if (fr->m_type == KindOfInt64) {
    switch ((IncDecOp)op) {
    case PreInc: {
      ++(fr->m_data.num);
      if (setResult) {
        tvDupCell(fr, to);
      }
      break;
    }
    case PostInc: {
      if (setResult) {
        tvDupCell(fr, to);
      }
      ++(fr->m_data.num);
      break;
    }
    case PreDec: {
      --(fr->m_data.num);
      if (setResult) {
        tvDupCell(fr, to);
      }
      break;
    }
    case PostDec: {
      if (setResult) {
        tvDupCell(fr, to);
      }
      --(fr->m_data.num);
      break;
    }
    default: ASSERT(false);
    }
    return;
  }
  if (fr->m_type == KindOfUninit) {
    ActRec* fp = g_vmContext->m_fp;
    size_t pind = ((uintptr_t(fp) - uintptr_t(fr)) / sizeof(TypedValue)) - 1;
    if (pind < size_t(fp->m_func->numNamedLocals())) {
      // Only raise a warning if fr points to a local variable
      raise_notice(Strings::UNDEFINED_VARIABLE,
                   fp->m_func->localVarName(pind)->data());
    }
    // Convert uninit null to null so that we don't write out an uninit null
    // to the eval stack for PostInc and PostDec.
    fr->m_type = KindOfNull;
  }
  switch ((IncDecOp)op) {
  case PreInc: {
    ++(tvAsVariant(fr));
    if (setResult) {
      tvReadCell(fr, to);
    }
    break;
  }
  case PostInc: {
    if (setResult) {
      tvReadCell(fr, to);
    }
    ++(tvAsVariant(fr));
    break;
  }
  case PreDec: {
    --(tvAsVariant(fr));
    if (setResult) {
      tvReadCell(fr, to);
    }
    break;
  }
  case PostDec: {
    if (setResult) {
      tvReadCell(fr, to);
    }
    --(tvAsVariant(fr));
    break;
  }
  default: ASSERT(false);
  }
}

template <bool setResult>
static inline void IncDecElemEmptyish(unsigned char op, TypedValue* base,
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
static inline void IncDecElemNumberish(TypedValue& dest) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (setResult) {
    tvWriteNull(&dest);
  }
}
template <bool setResult, KeyType keyType = AnyKey>
static inline void IncDecElem(TypedValue& tvScratch, TypedValue& tvRef,
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
      result = objOffsetGet(tvRef, instanceFromTv(base), tvCellAsCVarRef(key));
    }
    IncDecBody<setResult>(op, result, &dest);
    break;
  }
  default: ASSERT(false);
  }
}

template <bool setResult>
static inline void IncDecNewElemEmptyish(unsigned char op, TypedValue* base,
                                         TypedValue& dest) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt();
  tvAsVariant(base) = a;
  IncDecBody<setResult>(op, result, &dest);
}
template <bool setResult>
static inline void IncDecNewElemNumberish(TypedValue& dest) {
  raise_warning(Strings::CANNOT_USE_SCALAR_AS_ARRAY);
  if (setResult) {
    tvWriteNull(&dest);
  }
}
template <bool setResult>
static inline void IncDecNewElem(TypedValue& tvScratch, TypedValue& tvRef,
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
      result = NULL;
    } else {
      result = objOffsetGet(tvRef, instanceFromTv(base), init_null_variant);
      IncDecBody<setResult>(op, result, &dest);
    }
    break;
  }
  default: ASSERT(false);
  }
}

static inline ArrayData* UnsetElemArrayRawKey(ArrayData* a, int64 key,
                                              bool copy) {
  return a->remove(key, copy);
}

static inline ArrayData* UnsetElemArrayRawKey(ArrayData* a, StringData* key,
                                              bool copy) {
  int64 n;
  if (!key->isStrictlyInteger(n)) {
    return a->remove(StrNR(key), copy);
  } else {
    return a->remove(n, copy);
  }
}

template <KeyType keyType>
static inline void UnsetElemArray(TypedValue* base, TypedValue* key) {
  ArrayData* a = base->m_data.parr;
  bool copy = a->getCount() > 1;
  if (keyType == AnyKey) {
    if (IS_STRING_TYPE(key->m_type)) {
      a = UnsetElemArrayRawKey(a, key->m_data.pstr, copy);
    } else if (key->m_type == KindOfInt64) {
      a = UnsetElemArrayRawKey(a, key->m_data.num, copy);
    } else {
      VarNR varKey = tvAsCVarRef(key).toKey();
      if (varKey.isNull()) {
        return;
      }
      a = a->remove(varKey, copy);
    }
  } else {
    a = UnsetElemArrayRawKey(a, keyAsRaw<keyType>(key), copy);
  }
  if (a) {
    a->incRefCount();
    base->m_data.parr->decRefCount();
    base->m_data.parr = a;
  }
}

template <KeyType keyType = AnyKey>
static inline void UnsetElem(TypedValue* base, TypedValue* member) {
  TypedValue scratch;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfStaticString:
  case KindOfString: {
    raise_error("Cannot unset string offsets");
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
static inline DataType propPreNull(TypedValue& tvScratch, TypedValue*& result) {
  TV_WRITE_NULL(&tvScratch);
  result = &tvScratch;
  if (warn) {
    raise_warning("Cannot access property on non-object");
  }
  return KindOfNull;
}
static inline Instance* createDefaultObject() {
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);
  Instance* obj = newInstance(SystemLib::s_stdclassClass);
  return obj;
}
template <bool warn, bool define>
static inline DataType propPreStdclass(TypedValue& tvScratch,
                                       TypedValue*& result, TypedValue* base) {
  if (!define) {
    return propPreNull<warn>(tvScratch, result);
  }
  // TODO(#1124706): We don't want to do this anymore.
  Instance* obj = createDefaultObject();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  obj->incRefCount();
  result = base;
  return KindOfObject;
}

template <bool warn, bool define, bool issetEmpty>
static inline DataType propPre(TypedValue& tvScratch, TypedValue*& result,
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
          KeyType keyType = AnyKey>
static inline TypedValue* Prop(TypedValue& tvScratch, TypedValue& tvRef,
                               Class* ctx, TypedValue* base, TypedValue* key) {
  static_assert(keyType != IntKey, "Integer property keys are not supported");
  ASSERT(!warn || !unset);
  TypedValue* result = NULL;
  StringData* keySD = NULL;
  Instance* instance;
  if (baseIsObj) {
    instance = reinterpret_cast<Instance*>(base);
  } else {
    DataType t = propPre<warn, define, false>(tvScratch, result, base);
    if (t == KindOfNull) {
      return result;
    }
    ASSERT(t == KindOfObject);
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
static inline bool IssetEmptyElemObj(TypedValue& tvRef, Instance* instance,
                                     bool baseStrOff, TypedValue* key) {
  if (useEmpty) {
    if (LIKELY(instance->isCollection())) {
      return collectionEmpty(instance, key);
    } else {
      return objOffsetEmpty(tvRef, instance, tvCellAsCVarRef(key));
    }
  } else {
    if (LIKELY(instance->isCollection())) {
      return collectionIsset(instance, key);
    } else {
      return objOffsetIsset(tvRef, instance, tvCellAsCVarRef(key));
    }
  }
}

template <bool useEmpty, bool isObj = false, KeyType keyType = AnyKey>
static inline bool IssetEmptyElem(TypedValue& tvScratch, TypedValue& tvRef,
                                  TypedValue* base, bool baseStrOff,
                                  TypedValue* key) {
  TypedValue scratch;
  if (isObj) {
    initScratchKey<keyType>(scratch, key);
    return IssetEmptyElemObj<useEmpty>(
      tvRef, reinterpret_cast<Instance*>(base), baseStrOff, key);
  }

  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfStaticString:
  case KindOfString: {
    if (baseStrOff) {
      return useEmpty;
    }
    TypedValue tv;
    initScratchKey<keyType>(scratch, key);
    tvDup(key, &tv);
    tvCastToInt64InPlace(&tv);
    int64 x = tv.m_data.num;
    if (x < 0 || x >= base->m_data.pstr->size()) {
      return useEmpty;
    }
    if (!useEmpty) {
      return true;
    }
    tvAsVariant(&tvScratch) = base->m_data.pstr->getChar(x);
    result = &tvScratch;
    break;
  }
  case KindOfArray: {
    result = ElemArray<false, keyType>(base->m_data.parr, key);
    break;
  }
  case KindOfObject: {
    initScratchKey<keyType>(scratch, key);
    return IssetEmptyElemObj<useEmpty>(
      tvRef, static_cast<Instance*>(base->m_data.pobj), baseStrOff, key);
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
static inline bool IssetEmptyPropObj(Class* ctx, Instance* instance,
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

template <bool useEmpty, bool isObj = false, KeyType keyType = AnyKey>
static inline bool IssetEmptyProp(Class* ctx, TypedValue* base,
                                  TypedValue* key) {
  if (isObj) {
    return IssetEmptyPropObj<useEmpty, keyType>(
      ctx, reinterpret_cast<Instance*>(base), key);
  }

  TypedValue tvScratch;
  TypedValue* result = NULL;
  DataType t = propPre<false, false, true>(tvScratch, result, base);
  if (t == KindOfNull) {
    return useEmpty;
  }
  if (t == KindOfObject) {
    return IssetEmptyPropObj<useEmpty, keyType>(ctx, instanceFromTv(base), key);
  } else {
    ASSERT(t == KindOfArray);
    return useEmpty;
  }
}

template <bool setResult>
static inline void SetPropNull(Cell* val) {
  if (setResult) {
    tvRefcountedDecRefCell(val);
    tvWriteNull(val);
  }
  raise_warning("Cannot access property on non-object");
}
static inline void SetPropStdclass(TypedValue* base, TypedValue* key,
                                   Cell* val) {
  Instance* obj = createDefaultObject();
  obj->incRefCount();
  StringData* keySD = prepareKey(key);
  obj->setProp(NULL, keySD, (TypedValue*)val);
  if (keySD->decRefCount() == 0) {
    keySD->release();
  }
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  /* dont set _count; base could be an inner variant */
  base->m_data.pobj = obj;
}

template <KeyType keyType>
static inline void SetPropObj(Class* ctx, Instance* instance,
                              TypedValue* key, Cell* val) {
  StringData* keySD = prepareKey<keyType>(key);
  // Set property.
  instance->setProp(ctx, keySD, val);
  releaseKey<keyType>(keySD);
}

// $base->$key = $val
template <bool setResult, bool isObj = false, KeyType keyType = AnyKey>
static inline void SetProp(Class* ctx, TypedValue* base, TypedValue* key,
                           Cell* val) {
  if (isObj) {
    SetPropObj<keyType>(ctx, reinterpret_cast<Instance*>(base),
                        key, val);
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
    SetPropObj<keyType>(ctx, static_cast<Instance*>(base->m_data.pobj),
                        key, val);
    break;
  }
  default: {
    SetPropNull<setResult>(val);
    break;
  }
  }
}

static inline TypedValue* SetOpPropNull(TypedValue& tvScratch) {
  raise_warning("Attempt to assign property of non-object");
  tvWriteNull(&tvScratch);
  return &tvScratch;
}
static inline TypedValue* SetOpPropStdclass(TypedValue& tvRef, unsigned char op,
                                            TypedValue* base, TypedValue* key,
                                            Cell* rhs) {
  Instance* obj = createDefaultObject();
  obj->incRefCount();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->_count = 0;
  base->m_data.pobj = obj;

  StringData* keySD = prepareKey(key);
  tvWriteNull(&tvRef);
  SETOP_BODY(&tvRef, op, rhs);
  obj->setProp(NULL, keySD, &tvRef);
  LITSTR_DECREF(keySD);
  return &tvRef;
}

template <KeyType keyType>
static inline TypedValue* SetOpPropObj(TypedValue& tvRef, Class* ctx,
                                       unsigned char op, Instance* instance,
                                       TypedValue* key, Cell* rhs) {
  StringData* keySD = prepareKey<keyType>(key);
  TypedValue* result = instance->setOpProp(tvRef, ctx, op, keySD, rhs);
  releaseKey<keyType>(keySD);
  return result;
}

// $base->$key <op>= $rhs
template<bool isObj = false, KeyType keyType = AnyKey>
static inline TypedValue* SetOpProp(TypedValue& tvScratch, TypedValue& tvRef,
                                    Class* ctx, unsigned char op,
                                    TypedValue* base, TypedValue* key,
                                    Cell* rhs) {
  TypedValue scratch;
  if (isObj) {
    return SetOpPropObj<keyType>(tvRef, ctx, op,
                                 reinterpret_cast<Instance*>(base), key, rhs);
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
static inline void IncDecPropNull(TypedValue& dest) {
  raise_warning("Attempt to increment/decrement property of non-object");
  if (setResult) {
    tvWriteNull(&dest);
  }
}
template <bool setResult>
static inline void IncDecPropStdclass(unsigned char op, TypedValue* base,
                                      TypedValue* key, TypedValue& dest) {
  Instance* obj = createDefaultObject();
  obj->incRefCount();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->_count = 0;
  base->m_data.pobj = obj;

  StringData* keySD = prepareKey(key);
  TypedValue tv;
  tvWriteNull(&tv);
  if (setResult) {
    IncDecBody<true>(op, (&tv), &dest);
    obj->setProp(NULL, keySD, &dest);
  } else {
    // The caller doesn't actually want the result set, but we have to do so in
    // order to call obj->setProp().
    TypedValue tDest;
    tvWriteUninit(&tDest);
    IncDecBody<true>(op, (&tv), &tDest);
    obj->setProp(NULL, keySD, &tDest);
    ASSERT(!IS_REFCOUNTED_TYPE(tDest.m_type));
  }
  ASSERT(!IS_REFCOUNTED_TYPE(tv.m_type));
  LITSTR_DECREF(keySD);
}

template <bool setResult, KeyType keyType>
static inline void IncDecPropObj(TypedValue& tvRef, Class* ctx,
                                 unsigned char op, Instance* base,
                                 TypedValue* key, TypedValue& dest) {
  StringData* keySD = prepareKey<keyType>(key);
  base->incDecProp<setResult>(tvRef, ctx, op, keySD, dest);
  releaseKey<keyType>(keySD);
}

template <bool setResult, bool isObj = false, KeyType keyType = AnyKey>
static inline void IncDecProp(TypedValue& tvScratch, TypedValue& tvRef,
                              Class* ctx, unsigned char op,
                              TypedValue* base, TypedValue* key,
                              TypedValue& dest) {
  TypedValue scratch;
  if (isObj) {
    IncDecPropObj<setResult, keyType>(tvRef, ctx, op,
                                      reinterpret_cast<Instance*>(base),
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

template<bool isObj = false, KeyType keyType = AnyKey>
static inline void UnsetProp(Class* ctx, TypedValue* base,
                             TypedValue* key) {
  Instance* instance;
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
    instance = reinterpret_cast<Instance*>(base);
  }
  // Prepare key.
  StringData* keySD = prepareKey<keyType>(key);
  // Unset property.
  instance->unsetProp(ctx, keySD);
  releaseKey<keyType>(keySD);
}

///////////////////////////////////////////////////////////////////////////////
}
}
#endif // incl_VM_MEMBER_OPERATIONS_H_
