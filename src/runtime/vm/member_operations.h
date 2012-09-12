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

void objArrayAccess(TypedValue* base);
TypedValue* objOffsetGet(TypedValue& tvRef, TypedValue* base,
                         CVarRef offset, bool validate=true);
bool objOffsetIsset(TypedValue& tvRef, TypedValue* base, CVarRef offset,
                    bool validate=true);
bool objOffsetEmpty(TypedValue& tvRef, TypedValue* base, CVarRef offset,
                    bool validate=true);
void objOffsetSet(TypedValue* base, CVarRef offset, TypedValue* val,
                  bool validate=true);
void objOffsetAppend(TypedValue* base, TypedValue* val, bool validate=true);
void objOffsetUnset(TypedValue* base, CVarRef offset);

static inline String prepareKey(TypedValue* tv) {
  String ret;
  if (IS_STRING_TYPE(tv->m_type)) {
    ret = tv->m_data.pstr;
  } else {
    ret = tvAsCVarRef(tv).toString();
  }
  return ret;
}

static inline void opPre(TypedValue*& base, DataType& type) {
  // Get inner variant if necessary.
  type = base->m_type;
  if (type == KindOfRef) {
    base = base->m_data.pref->tv();
    type = base->m_type;
  }
}

template <bool warn>
static inline TypedValue* ElemArray(TypedValue* base,
                                    TypedValue* key) {
  TypedValue* result;
  bool isHphpArray = IsHphpArray(base->m_data.parr);
  if (key->m_type == KindOfInt64) {
    if (LIKELY(isHphpArray)) {
      result = (static_cast<HphpArray*>(base->m_data.parr))
               ->nvGet(key->m_data.num);
      if (result == NULL) {
        result = (TypedValue*)&null_variant;
      }
    } else {
      result = (TypedValue*)&tvCellAsVariant(base).asArrRef()
        .rvalAtRef(key->m_data.num);
    }
  } else if (IS_STRING_TYPE(key->m_type)) {
    if (LIKELY(isHphpArray)) {
      int64 n;
      if (!key->m_data.pstr->isStrictlyInteger(n)) {
        result = (static_cast<HphpArray*>(base->m_data.parr))
                 ->nvGet(key->m_data.pstr);
      } else {
        result = (static_cast<HphpArray*>(base->m_data.parr))->nvGet(n);
      }

      if (result == NULL) {
        result = (TypedValue*)&null_variant;
      }
    } else {
      result = (TypedValue*)&tvCellAsVariant(base).asArrRef()
        .rvalAtRef(tvCellAsVariant(key).asCStrRef());
    }
  } else {
    result = (TypedValue*)&tvCellAsVariant(base).asArrRef()
      .rvalAtRef(tvCellAsCVarRef(key));
  }

  if (UNLIKELY(result->m_type == KindOfUninit)) {
    result = (TypedValue*)&init_null_variant;
    if (warn) {
      raise_notice(Strings::UNDEFINED_INDEX,
                   tvAsCVarRef(key).toString().data());
    }
  }
  return result;
}

// $result = $base[$key];
template <bool warn>
static inline TypedValue* Elem(TypedValue& tvScratch, TypedValue& tvRef,
                               TypedValue* base, bool& baseStrOff,
                               TypedValue* key) {
  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfBoolean: {
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfInt64: {
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfDouble: {
    result = (TypedValue*)&init_null_variant;
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (baseStrOff) {
      raise_error("Cannot use string offset as an array");
    }
    int64 x = IS_INT_TYPE(key->m_type)
              ? key->m_data.num
              : int64(tvCellAsCVarRef(key));
    if (x < 0 || x >= base->m_data.pstr->size()) {
      if (warn) {
        raise_warning("Out of bounds");
      }
      static StringData* sd = StringData::GetStaticString("");
      tvScratch.m_data.pstr = sd;
      tvScratch._count = 0;
      tvScratch.m_type = KindOfString;
    } else {
      tvAsVariant(&tvScratch) = base->m_data.pstr->getChar(x);
    }
    result = &tvScratch;
    baseStrOff = true;
    break;
  }
  case KindOfArray: {
    result = ElemArray<warn>(base, key);
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
    } else {
      result = objOffsetGet(tvRef, base, tvCellAsCVarRef(key));
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

template <bool warn>
static inline TypedValue* ElemDArray(TypedValue* base, TypedValue* key) {
  TypedValue* result;
  bool defined = !warn || tvAsVariant(base).asArrRef()
                          .exists(tvAsCVarRef(key));
  if (key->m_type == KindOfInt64) {
    result = (TypedValue*)&tvAsVariant(base).asArrRef()
                                            .lvalAt(key->m_data.num);
  } else {
    result = (TypedValue*)&tvAsVariant(base).asArrRef()
                                            .lvalAt(tvCellAsCVarRef(key));
  }

  if (warn) {
    if (!defined) {
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
template <bool warn, bool reffy>
static inline TypedValue* ElemD(TypedValue& tvScratch, TypedValue& tvRef,
                                TypedValue* base, TypedValue* key) {
  TypedValue* result;
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
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
    result = ElemDArray<warn>(base, key);
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      if (reffy) {
        raise_error("Collection elements cannot be taken by reference");
        result = NULL;
      } else {
        result = collectionGet(base->m_data.pobj, key);
      }
    } else {
      result = objOffsetGet(tvRef, base, tvCellAsCVarRef(key));
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
    bool defined = tvAsVariant(base).asArrRef()
                      .exists(tvAsCVarRef(key));
    if (defined) {
      if (key->m_type == KindOfInt64) {
        result = (TypedValue*)&tvAsVariant(base).asArrRef()
                                                .lvalAt(key->m_data.num);
      } else {
        result = (TypedValue*)&tvAsVariant(base).asArrRef()
                                                .lvalAt(tvCellAsCVarRef(key));
      }
    } else {
      tvWriteUninit(&tvScratch);
      result = &tvScratch;
    }
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
    } else {
      result = objOffsetGet(tvRef, base, tvCellAsCVarRef(key));
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
      result = objOffsetGet(tvRef, base, init_null_variant);
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
// SetElem() leaves the result in 'value', rather than returning it as in
// SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
// get around.
template <bool setResult>
static inline void SetElem(TypedValue* base, TypedValue* key, Cell* value) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    SetElemEmptyish(base, key, value);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      SetElemNumberish<setResult>(value);
    } else {
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
        char* s = (char*)malloc(slen + 1);
        if (s == NULL) {
          raise_error("Out of memory");
        }
        memcpy(s, base->m_data.pstr->data(), baseLen);
        if (x > baseLen) {
          memset(&s[baseLen], ' ', slen - baseLen - 1);
        }
        s[x] = y[0];
        s[slen] = '\0';
        StringData* sd = NEW(StringData)(s, slen, AttachString);
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
    ArrayData* a = base->m_data.parr;
    ArrayData* newData = NULL;
    bool copy = (a->getCount() > 1)
                || (value->m_type == KindOfArray && value->m_data.parr == a);
    ASSERT(key->m_type != KindOfRef);
    if (key->m_type <= KindOfNull) {
      newData = a->set(empty_string, tvCellAsCVarRef(value), copy);
    } else if (IS_STRING_TYPE(key->m_type)) {
      int64 n;
      if (key->m_data.pstr->isStrictlyInteger(n)) {
        newData = a->set(n, tvCellAsCVarRef(value), copy);
      } else {
        newData = a->set(tvAsCVarRef(key).asCStrRef(), tvCellAsCVarRef(value),
                         copy);
      }
    } else if (key->m_type != KindOfArray && key->m_type != KindOfObject) {
      newData = a->set(tvAsCVarRef(key).toInt64(), tvCellAsCVarRef(value),
                       copy);
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
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      collectionSet(base->m_data.pobj, key, (TypedValue*)value);
    } else {
      objOffsetSet(base, tvAsCVarRef(key), (TypedValue*)value);
    }
    break;
  }
  default: ASSERT(false);
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
      objOffsetAppend(base, (TypedValue*)value);
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
static inline TypedValue* SetOpElem(TypedValue& tvScratch, TypedValue& tvRef,
                                    unsigned char op, TypedValue* base,
                                    TypedValue* key, Cell* rhs) {
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
    result = ElemDArray<MoreWarnings>(base, key);
    SETOP_BODY(result, op, rhs);
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
      SETOP_BODY(result, op, rhs);
    } else {
      result = objOffsetGet(tvRef, base, tvCellAsCVarRef(key));
      SETOP_BODY(result, op, rhs);
      objOffsetSet(base, tvAsCVarRef(key), result, false);
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
      result = objOffsetGet(tvRef, base, init_null_variant);
      SETOP_BODY(result, op, rhs);
      objOffsetAppend(base, result, false);
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
template <bool setResult>
static inline void IncDecElem(TypedValue& tvScratch, TypedValue& tvRef,
                              unsigned char op, TypedValue* base,
                              TypedValue* key, TypedValue& dest) {
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
    TypedValue* result = ElemDArray<MoreWarnings>(base, key);
    IncDecBody<setResult>(op, result, &dest);
    break;
  }
  case KindOfObject: {
    TypedValue* result;
    if (LIKELY(base->m_data.pobj->isCollection())) {
      result = collectionGet(base->m_data.pobj, key);
    } else {
      result = objOffsetGet(tvRef, base, tvCellAsCVarRef(key));
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
      result = objOffsetGet(tvRef, base, init_null_variant);
      IncDecBody<setResult>(op, result, &dest);
    }
    break;
  }
  default: ASSERT(false);
  }
}

static inline void UnsetElem(TypedValue* base, TypedValue* member) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfStaticString:
  case KindOfString: {
    raise_error("Cannot unset string offsets");
  }
  case KindOfArray: {
    ArrayData* a = base->m_data.parr;
    bool copy = (a->getCount() > 1);
    int64 n;
    if (IS_STRING_TYPE(member->m_type)) {
      if (member->m_data.pstr->isStrictlyInteger(n)) {
        a = a->remove(n, copy);
      } else {
        a = a->remove(tvAsCVarRef(member).asCStrRef(), copy);
      }
    } else if (member->m_type == KindOfInt64) {
      a = a->remove(member->m_data.num, copy);
    } else {
      CVarRef memberCVR = tvAsCVarRef(member);
      const VarNR &key = memberCVR.toKey();
      if (key.isNull()) {
        return;
      }
      a = a->remove(key, copy);
    }
    if (a) {
      a->incRefCount();
      base->m_data.parr->decRefCount();
      base->m_data.parr = a;
    }
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isCollection())) {
      collectionUnset(base->m_data.pobj, member);
    } else {
      objOffsetUnset(base, tvAsCVarRef(member));
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
                                       TypedValue*& result, TypedValue* base,
                                       TypedValue* key, StringData*& keySD) {
  if (!define) {
    return propPreNull<warn>(tvScratch, result);
  }
  Instance* obj = createDefaultObject();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  obj->incRefCount();
  result = base;
  keySD = prepareKey(key).detach();
  return KindOfObject;
}

template <bool warn, bool define, bool issetEmpty>
static inline DataType propPre(TypedValue& tvScratch, TypedValue*& result,
                               TypedValue*& base, TypedValue* key,
                               StringData*& keySD) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    return propPreStdclass<warn, define>(tvScratch, result, base, key, keySD);
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      return propPreNull<warn>(tvScratch, result);
    } else {
      return propPreStdclass<warn, define>(tvScratch, result, base, key, keySD);
    }
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      return propPreNull<warn>(tvScratch, result);
    } else {
      return propPreStdclass<warn, define>(tvScratch, result, base, key, keySD);
    }
  }
  case KindOfArray: {
    return issetEmpty ? KindOfArray : propPreNull<warn>(tvScratch, result);
  }
  case KindOfObject: {
    keySD = prepareKey(key).detach();
    return KindOfObject;
  }
  default: {
    return propPreNull<warn>(tvScratch, result);
  }
  }
}

static inline void propPost(StringData* keySD) {
  LITSTR_DECREF(keySD);
}

// define == false:
//   $result = $base->$key;
//
// define == true:
//   $base->$key = ...
//   \____ ____/
//        v
//     $result
template <bool warn, bool define, bool unset>
static inline TypedValue* Prop(TypedValue& tvScratch, TypedValue& tvRef,
                               Class* ctx, TypedValue* base, TypedValue* key) {
  ASSERT(!warn || !unset);
  TypedValue* result = NULL;
  StringData* keySD = 0;
  DataType t = propPre<warn, define, false>(tvScratch, result, base, key,
                                            keySD);
  if (t == KindOfNull) {
    return result;
  }
  ASSERT(t == KindOfObject);
  // Get property.
  Instance* instance = static_cast<Instance*>(base->m_data.pobj);
  result = &tvScratch;
#define ARGS result, tvRef, ctx, keySD
  if (!warn && !(define || unset)) instance->prop  (ARGS);
  if (!warn &&  (define || unset)) instance->propD (ARGS);
  if ( warn && !define           ) instance->propW (ARGS);
  if ( warn &&  define           ) instance->propWD(ARGS);
#undef ARGS
  propPost(keySD);
  return result;
}

template <bool useEmpty>
static inline bool IssetEmptyElem(TypedValue& tvScratch, TypedValue& tvRef,
                                  TypedValue* base, bool baseStrOff,
                                  TypedValue* key) {
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
    result = ElemArray<false>(base, key);
    break;
  }
  case KindOfObject: {
    if (useEmpty) {
      if (LIKELY(base->m_data.pobj->isCollection())) {
        return collectionEmpty(base->m_data.pobj, key);
      } else {
        return objOffsetEmpty(tvRef, base, tvCellAsCVarRef(key));
      }
    } else {
      if (LIKELY(base->m_data.pobj->isCollection())) {
        return collectionIsset(base->m_data.pobj, key);
      } else {
        return objOffsetIsset(tvRef, base, tvCellAsCVarRef(key));
      }
    }
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

template <bool useEmpty>
static inline bool IssetEmptyProp(Class* ctx, TypedValue* base,
                                  TypedValue* key) {
  StringData* keySD;
  TypedValue tvScratch;
  TypedValue* result = NULL;
  DataType t = propPre<false, false, true>(tvScratch, result, base, key,
                                           keySD);
  if (t == KindOfNull) {
    return useEmpty;
  }
  if (t == KindOfObject) {
    bool issetEmptyResult;
    Instance* instance = static_cast<Instance*>(base->m_data.pobj);
    issetEmptyResult = useEmpty ?
                  instance->propEmpty(ctx, keySD) :
                  instance->propIsset(ctx, keySD);
    propPost(keySD);
    return issetEmptyResult;
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
  StringData* keySD = prepareKey(key).detach();
  obj->setProp(NULL, keySD, (TypedValue*)val);
  if (keySD->decRefCount() == 0) {
    keySD->release();
  }
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  /* dont set _count; base could be an inner variant */
  base->m_data.pobj = obj;
}
// $base->$key = $val
template <bool setResult>
static inline void SetProp(Class* ctx, TypedValue* base, TypedValue* key,
                           Cell* val) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    SetPropStdclass(base, key, val);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      SetPropNull<setResult>(val);
    } else {
      SetPropStdclass(base, key, val);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      SetPropNull<setResult>(val);
    } else {
      SetPropStdclass(base, key, val);
    }
    break;
  }
  case KindOfObject: {
    StringData* keySD = prepareKey(key).detach();
    // Set property.
    Instance* instance = static_cast<Instance*>(base->m_data.pobj);
    instance->setProp(ctx, keySD, val);
    LITSTR_DECREF(keySD);
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

  StringData* keySD = prepareKey(key).detach();
  tvWriteNull(&tvRef);
  SETOP_BODY(&tvRef, op, rhs);
  obj->setProp(NULL, keySD, &tvRef);
  LITSTR_DECREF(keySD);
  return &tvRef;
}
// $base->$key <op>= $rhs
static inline TypedValue* SetOpProp(TypedValue& tvScratch, TypedValue& tvRef,
                                    Class* ctx, unsigned char op,
                                    TypedValue* base, TypedValue* key,
                                    Cell* rhs) {
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
    StringData* keySD = prepareKey(key).detach();
    Instance* instance = static_cast<Instance*>(base->m_data.pobj);
    result = instance->setOpProp(tvRef, ctx, op, keySD, rhs);
    LITSTR_DECREF(keySD);
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

  StringData* keySD = prepareKey(key).detach();
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
template <bool setResult>
static inline void IncDecProp(TypedValue& tvScratch, TypedValue& tvRef,
                              Class* ctx, unsigned char op,
                              TypedValue* base, TypedValue* key,
                              TypedValue& dest) {
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
    StringData* keySD = prepareKey(key).detach();
    Instance* instance = static_cast<Instance*>(base->m_data.pobj);
    instance->incDecProp<setResult>(tvRef, ctx, op, keySD, dest);
    LITSTR_DECREF(keySD);
    break;
  }
  default: {
    IncDecPropNull<setResult>(dest);
    break;
  }
  }
}

static inline void UnsetProp(Class* ctx, TypedValue* base,
                             TypedValue* key) {
  DataType type;
  opPre(base, type);
  // Validate base.
  if (UNLIKELY(type != KindOfObject)) {
    // Do nothing.
    return;
  }
  // Prepare key.
  StringData* keySD = prepareKey(key).detach();
  // Unset property.
  Instance* instance = static_cast<Instance*>(base->m_data.pobj);
  instance->unsetProp(ctx, keySD);

  LITSTR_DECREF(keySD);
}

///////////////////////////////////////////////////////////////////////////////
}
}
#endif // incl_VM_MEMBER_OPERATIONS_H_
