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

#include <iostream>
#include <iomanip>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <libgen.h>

#include <compiler/builtin_symbols.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/dyn_tracer.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/base/base_includes.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/array/hphp_array.h>
#include <util/util.h>
#include <util/trace.h>
#include <util/debug.h>

#include <runtime/base/tv_macros.h>
#include <runtime/vm/instrumentation_hook.h>
#include <runtime/vm/php_debug.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/type_constraint.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/ext/profile/extprofile_string.h>
#include <runtime/ext/ext_continuation.h>

#include <system/lib/systemlib.h>

using std::string;

namespace HPHP {
namespace VM {

static const Trace::Module TRACEMOD = Trace::bcinterp;

PreClass* arGetContextPreClass(const ActRec* ar) {
  if (ar->m_func->m_preClass == NULL) {
    return NULL;
  }
  if (!(ar->m_func->m_preClass->m_attrs & AttrTrait)) {
    return ar->m_func->m_preClass;
  }
  Class* klass;
  if (ar->hasThis()) {
    klass = ar->getThis()->getVMClass();
  } else  {
    klass = ar->getClass();
  }
  ASSERT(klass);
  klass = klass->findMethodBaseClass(ar->m_func->m_name);
  ASSERT(klass);
  return klass->m_preClass.get();
}

// When MoreWarnings is set to true, the VM will match Zend on
// warnings for SetOpM and CGetG
static const bool MoreWarnings = false;

static StaticString s_call_user_func(LITSTR_INIT("call_user_func"));
static StaticString s_call_user_func_array(LITSTR_INIT("call_user_func_array"));
static StaticString s_file(LITSTR_INIT("file"));
static StaticString s_line(LITSTR_INIT("line"));
static StaticString s_stdclass(LITSTR_INIT("stdclass"));
static StaticString s___call(LITSTR_INIT("__call"));
static StaticString s___callStatic(LITSTR_INIT("__callStatic"));

///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Miscellaneous macros.

#define NEXT() pc++
#define DECODE_JMP(type, var)                                                 \
  type var __attribute__((unused)) = *(type*)pc;                              \
  ONTRACE(1,                                                                  \
          Trace::trace("decode:     Immediate %s %"PRIi64"\n", #type,         \
                       (int64_t)var));
#define ITER_SKIP(offset)  pc = origPc + (offset);

#define DECODE(type, var)                                                     \
  DECODE_JMP(type, var);                                                      \
  pc += sizeof(type)
#define DECODE_IVA(var)                                                       \
  int32 var UNUSED = decodeVariableSizeImm((unsigned char**)&pc);             \
  ONTRACE(1,                                                                  \
          Trace::trace("decode:     Immediate int32 %"PRIi64"\n",             \
                       (int64_t)var));

#define SYNC() m_pc = pc

//=============================================================================
// Miscellaneous helpers.

static void
objArrayAccess(TypedValue* base) {
  ASSERT(base->m_type == KindOfObject);
  if (!instanceOf(tvAsCVarRef(base), "ArrayAccess")) {
    raise_error("Object does not implement ArrayAccess");
  }
}

static TypedValue*
objOffsetGet(TypedValue& tvScratch, TypedValue& tvRef, TypedValue* base,
             CVarRef offset, bool validate=true) {
  if (validate) {
    objArrayAccess(base);
  }
  TypedValue* result;
  ObjectData* obj = base->m_data.pobj;
  if (LIKELY(obj->isInstance())) {
    Instance* instance = static_cast<Instance*>(obj);
    static StringData* sd__offsetGet = StringData::GetStaticString("offsetGet");
    const Func* method = instance->methodNamed(sd__offsetGet);
    ASSERT(method != NULL);
    instance->invokeUserMethod(&tvRef, method, CREATE_VECTOR1(offset));
    result = &tvRef;
  } else {
    tvAsVariant(&tvScratch)
      = tvAsVariant(base).getArrayAccess()->___offsetget_lval(offset);
    result = &tvScratch;
  }
  return result;
}

static bool
objOffsetExists(TypedValue* base, CVarRef offset) {
  objArrayAccess(base);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  static StringData* sd__offsetExists
    = StringData::GetStaticString("offsetExists");
  ObjectData* obj = base->m_data.pobj;
  if (LIKELY(obj->isInstance())) {
    Instance* instance = static_cast<Instance*>(obj);
    const Func* method = instance->methodNamed(sd__offsetExists);
    ASSERT(method != NULL);
    instance->invokeUserMethod(&tvResult, method, CREATE_VECTOR1(offset));
  } else {
    tvAsVariant(&tvResult) = tvAsVariant(base).getArrayAccess()
      ->o_invoke(sd__offsetExists, CREATE_VECTOR1(offset));
  }
  tvCastToBooleanInPlace(&tvResult);
  return bool(tvResult.m_data.num);
}

static void
objOffsetSet(TypedValue* base, CVarRef offset, TypedValue* val,
             bool validate=true) {
  if (validate) {
    objArrayAccess(base);
  }
  static StringData* sd__offsetSet = StringData::GetStaticString("offsetSet");
  ObjectData* obj = base->m_data.pobj;
  if (LIKELY(obj->isInstance())) {
    Instance* instance = static_cast<Instance*>(obj);
    const Func* method = instance->methodNamed(sd__offsetSet);
    ASSERT(method != NULL);
    TypedValue tvResult;
    tvWriteUninit(&tvResult);
    instance->invokeUserMethod(&tvResult, method,
                               CREATE_VECTOR2(offset, tvAsCVarRef(val)));
    tvRefcountedDecRef(&tvResult);
  } else {
    tvAsVariant(base).getArrayAccess()
      ->o_invoke(sd__offsetSet, CREATE_VECTOR2(offset, tvAsCVarRef(val)));
  }
}

static void
objOffsetUnset(TypedValue* base, CVarRef offset) {
  objArrayAccess(base);
  static StringData* sd__offsetUnset
    = StringData::GetStaticString("offsetUnset");
  ObjectData* obj = base->m_data.pobj;
  if (LIKELY(obj->isInstance())) {
    Instance* instance = static_cast<Instance*>(obj);
    const Func* method = instance->methodNamed(sd__offsetUnset);
    ASSERT(method != NULL);
    TypedValue tv;
    tvWriteUninit(&tv);
    instance->invokeUserMethod(&tv, method, CREATE_VECTOR1(offset));
    tvRefcountedDecRef(&tv);
  } else {
    tvAsVariant(base).getArrayAccess()
      ->o_invoke(sd__offsetUnset, CREATE_VECTOR1(offset));
  }
}

static inline Instance*
newInstance(Class* cls) {
  Instance *inst = Instance::newInstance(cls);
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall) &&
      inst->ObjectData::isInstance()) {
    g_context->m_liveBCObjs.insert(inst);
  }
  return inst;
}

// The vector instructions' P and E addressing modes might find homes and
// homed variants on the stack. Consolidate this logic here.
static inline TypedValue*
tvToKeyValue(TypedValue* outer) {
  if (outer->m_type == KindOfHome) {
    outer = outer->m_data.ptv;
    if (outer->m_type == KindOfVariant) {
      outer = outer->m_data.ptv;
    }
  }
  return outer;
}

inline StringData* prepareKey(TypedValue* tv) {
  StringData* ret;
  if (IS_STRING_TYPE(tv->m_type)) {
    ret = tv->m_data.pstr;
    ret->incRefCount();
  } else {
    String str(tvAsCVarRef(tv).toString());
    ret = str.get();
    ret->incRefCount();
  }
  return ret;
}

static inline void opPre(TypedValue*& base, DataType& type) {
  // Get inner variant if necessary.
  type = base->m_type;
  if (type == KindOfVariant) {
    base = base->m_data.ptv;
    type = base->m_type;
  }
}

template <bool warn>
static inline TypedValue* ElemArray(TypedValue& tvScratch,
                                    TypedValue* base,
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
      tvAsVariant(&tvScratch) = tvCellAsVariant(base).asArrRef()
                                .rvalAt(key->m_data.num);
      result = &tvScratch;
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
      tvAsVariant(&tvScratch) = tvCellAsVariant(base).asArrRef()
                                .rvalAt(tvCellAsVariant(key).asCStrRef());
      result = &tvScratch;
    }
  } else {
    tvAsVariant(&tvScratch) = tvCellAsVariant(base).asArrRef()
                              .rvalAt(tvCellAsCVarRef(key));
    result = &tvScratch;
  }

  if (UNLIKELY(result->m_type == KindOfUninit)) {
    result = (TypedValue*)&init_null_variant;
    if (warn) {
      raise_notice("Undefined index: %s",
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
  case KindOfInt32:
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
    result = ElemArray<warn>(tvScratch, base, key);
    break;
  }
  case KindOfObject: {
    result = objOffsetGet(tvScratch, tvRef, base, tvCellAsCVarRef(key));
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
      raise_notice("Undefined index: %s",
                   tvAsCVarRef(key).toString().data());
    }
  }

  return result;
}

// $base[$key] = ...
// \____ ____/
//      v
//   $result
template <bool warn>
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
      raise_warning("Cannot use a scalar value as an array");
    }
    tvAsVariant(base) = a;
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      raise_warning("Cannot use a scalar value as an array");
      tvWriteUninit(&tvScratch);
      result = &tvScratch;
    } else {
      Array a = Array::Create();
      result = (TypedValue*)&a.lvalAt(tvCellAsCVarRef(key));
      if (warn) {
        raise_warning("Cannot use a scalar value as an array");
      }
      tvAsVariant(base) = a;
    }
    break;
  }
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble: {
    raise_warning("Cannot use a scalar value as an array");
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
        raise_warning("Cannot use a scalar value as an array");
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
    result = objOffsetGet(tvScratch, tvRef, base, tvCellAsCVarRef(key));
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
  case KindOfInt32:
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
    result = objOffsetGet(tvScratch, tvRef, base, tvCellAsCVarRef(key));
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
static TypedValue* NewElem(TypedValue& tvScratch, TypedValue& tvRef,
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
    result = objOffsetGet(tvScratch, tvRef, base, null_variant);
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
  TypedValue* result = (TypedValue*)&a.lvalAt(tvAsCVarRef(key));
  tvAsVariant(base) = a;
  tvDupCell((TypedValue*)value, result);
}
static inline void SetElemNumberish(Cell* value) {
  raise_warning("Cannot use a scalar value as an array");
  tvRefcountedDecRefCell((TypedValue*)value);
  tvWriteNull((TypedValue*)value);
}
// SetElem() leaves the result in 'value', rather than returning it as in
// SetOpElem(), because doing so avoids a dup operation that SetOpElem() can't
// get around.
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
      SetElemNumberish(value);
    } else {
      SetElemEmptyish(base, key, value);
    }
    break;
  }
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble: {
    SetElemNumberish(value);
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
      if (x < 0) {
        raise_warning("Illegal string offset: %"PRId64"", x);
        break;
      }
      // Compute how long the resulting string will be.
      int slen;
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
        tvDecRef(&tv);
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
        base->m_data.pstr = sd;
      }
      // Push y onto the stack.
      tvRefcountedDecRef(value);
      StringData* sd = NEW(StringData)(y, strlen(y), CopyString);
      sd->incRefCount();
      value->m_data.pstr = sd;
      value->_count = 0;
      value->m_type = KindOfString;
    }
    break;
  }
  case KindOfArray: {
    ArrayData* a = base->m_data.parr;
    ArrayData* newData = NULL;
    bool copy = (a->getCount() > 1)
                || (value->m_type == KindOfArray && value->m_data.parr == a);
    ASSERT(key->m_type != KindOfVariant);
    if (key->m_type <= KindOfNull) {
      newData = a->set(empty_string, tvCellAsCVarRef(value), copy);
    } else if (IS_STRING_TYPE(key->m_type)) {
      int64 n;
      if (key->m_data.pstr->isStrictlyInteger(n)) {
        newData = a->set(n, tvCellAsCVarRef(value), copy);
      } else {
        newData = a->set(tvAsCVarRef(key), tvCellAsCVarRef(value), copy);
      }
    } else if (key->m_type != KindOfArray && key->m_type != KindOfObject) {
      newData = a->set(tvAsCVarRef(key).toInt64(), tvCellAsCVarRef(value),
                       copy);
    } else {
      raise_warning("Illegal offset type");
      // Assignment failed, so the result is null rather than the RHS.
      // XXX This does not match bytecode.specification, but it does roughly
      // match Zend and hphpi behavior.
      if (IS_REFCOUNTED_TYPE(value->m_type)) {
        tvDecRef(value);
      }
      tvWriteNull(value);
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
    objOffsetSet(base, tvAsCVarRef(key), (TypedValue*)value);
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
static inline void SetNewElemNumberish(Cell* value) {
  raise_warning("Cannot use a scalar value as an array");
  tvRefcountedDecRefCell((TypedValue*)value);
  tvWriteNull((TypedValue*)value);
}
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
      SetNewElemNumberish(value);
    } else {
      SetNewElemEmptyish(base, value);
    }
    break;
  }
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble: {
    SetNewElemNumberish(value);
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
    objOffsetSet(base, init_null_variant, (TypedValue*)value);
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
  SETOP_BODY(result, op, rhs);
  return result;
}
static inline TypedValue* SetOpElemNumberish(TypedValue& tvScratch) {
  raise_warning("Cannot use a scalar value as an array");
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
  case KindOfInt32:
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
    result = objOffsetGet(tvScratch, tvRef, base, tvAsCVarRef(key));
    SETOP_BODY(result, op, rhs);
    objOffsetSet(base, tvAsCVarRef(key), result, false);
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
  raise_warning("Cannot use a scalar value as an array");
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
  case KindOfInt32:
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
    result = objOffsetGet(tvScratch, tvRef, base, init_null_variant);
    SETOP_BODY(result, op, rhs);
    objOffsetSet(base, init_null_variant, result, false);
    break;
  }
  default: {
    ASSERT(false);
    result = NULL; // Silence compiler warning.
  }
  }
  return result;
}

static inline void IncDecElemEmptyish(unsigned char op, TypedValue* base,
                                      TypedValue* key, TypedValue& dest) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt(tvAsCVarRef(key));
  tvAsVariant(base) = a;
  IncDecBody(op, result, &dest);
}
static inline void IncDecElemNumberish(TypedValue& dest) {
  raise_warning("Cannot use a scalar value as an array");
  tvWriteNull(&dest);
}
static inline void IncDecElem(TypedValue& tvScratch, TypedValue& tvRef,
                              unsigned char op, TypedValue* base,
                              TypedValue* key, TypedValue& dest) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    IncDecElemEmptyish(op, base, key, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecElemNumberish(dest);
    } else {
      IncDecElemEmptyish(op, base, key, dest);
    }
    break;
  }
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble: {
    IncDecElemNumberish(dest);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      raise_error("Invalid IncDecElem operand");
    }
    IncDecElemEmptyish(op, base, key, dest);
    break;
  }
  case KindOfArray: {
    TypedValue* result = ElemDArray<MoreWarnings>(base, key);
    IncDecBody(op, result, &dest);
    break;
  }
  case KindOfObject: {
    TypedValue* result = objOffsetGet(tvScratch, tvRef, base, tvAsCVarRef(key));
    IncDecBody(op, result, &dest);
    break;
  }
  default: ASSERT(false);
  }
}

static inline void IncDecNewElemEmptyish(unsigned char op, TypedValue* base,
                                         TypedValue& dest) {
  Array a = Array::Create();
  TypedValue* result = (TypedValue*)&a.lvalAt();
  tvAsVariant(base) = a;
  IncDecBody(op, result, &dest);
}
static inline void IncDecNewElemNumberish(TypedValue& dest) {
  raise_warning("Cannot use a scalar value as an array");
  tvWriteNull(&dest);
}
static inline void IncDecNewElem(TypedValue& tvScratch, TypedValue& tvRef,
                                 unsigned char op, TypedValue* base,
                                 TypedValue& dest) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    IncDecNewElemEmptyish(op, base, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecNewElemNumberish(dest);
    } else {
      IncDecNewElemEmptyish(op, base, dest);
    }
    break;
  }
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble: {
    IncDecNewElemNumberish(dest);
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      raise_error("Invalid IncDecNewElem operand");
    }
    IncDecNewElemEmptyish(op, base, dest);
    break;
  }
  case KindOfArray: {
    TypedValue* result = (TypedValue*)&tvAsVariant(base).asArrRef().lvalAt();
    IncDecBody(op, result, &dest);
    break;
  }
  case KindOfObject: {
    TypedValue* result = objOffsetGet(tvScratch, tvRef, base,
                                      init_null_variant);
    IncDecBody(op, result, &dest);
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
        a = a->remove(tvAsCVarRef(member), copy);
      }
    } else if (member->m_type == KindOfInt32 || member->m_type == KindOfInt64) {
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
    objOffsetUnset(base, tvAsCVarRef(member));
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
template <bool warn, bool define>
static inline DataType propPreStdclass(TypedValue& tvScratch,
                                       TypedValue*& result, TypedValue* base,
                                       TypedValue* key, StringData*& keySD) {
  if (!define) {
    return propPreNull<warn>(tvScratch, result);
  }
  Instance* obj = newInstance(SystemLib::s_stdclassClass);
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->m_data.pobj = obj;
  obj->incRefCount();
  result = base;
  if (warn) {
    raise_warning("Cannot access property on non-object");
  }
  keySD = prepareKey(key);
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
    keySD = prepareKey(key);
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
static inline TypedValue* prop(TypedValue& tvScratch, PreClass* ctx,
                               TypedValue* base, TypedValue* key) {
  ASSERT(!warn || !unset);
  TypedValue* result = NULL;
  StringData* keySD;
  DataType t = propPre<warn, define, false>(tvScratch, result, base, key,
                                            keySD);
  if (t == KindOfNull) {
    return result;
  }
  ASSERT(t == KindOfObject);
  // Get property.
  if (LIKELY(base->m_data.pobj->isInstance())) {
    Instance* instance = static_cast<Instance*>(base->m_data.pobj);
    result = &tvScratch;
    if (!warn && !(define || unset)) instance->prop  (result, ctx, keySD);
    if (!warn &&  (define || unset)) instance->propD (result, ctx, keySD);
    if ( warn && !define           ) instance->propW (result, ctx, keySD);
    if ( warn &&  define           ) instance->propWD(result, ctx, keySD);
  } else {
    // Extension class instance.
    TV_WRITE_NULL(&tvScratch);
    CStrRef ctxName = ctx ? CStrRef(ctx->m_name) : CStrRef(null_string);
    if (define || unset) {
      result = (TypedValue*)&base->m_data.pobj->o_lval(CStrRef(keySD),
                                                       tvAsCVarRef(&tvScratch),
                                                       ctxName);
    } else {
      tvAsVariant(&tvScratch) = base->m_data.pobj->o_get(CStrRef(keySD),
                                                         warn, /* error */
                                                         ctxName);
      result = &tvScratch;
    }
  }
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
    result = ElemArray<false>(tvScratch, base, key);
    break;
  }
  case KindOfObject: {
    if (!useEmpty) {
      return objOffsetExists(base, tvAsCVarRef(key));
    }
    if (!objOffsetExists(base, tvAsCVarRef(key))) {
      return true;
    }
    result = objOffsetGet(tvScratch, tvRef, base, tvAsCVarRef(key), false);
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
static inline bool IssetEmptyProp(PreClass* ctx, TypedValue* base,
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
    if (LIKELY(base->m_data.pobj->isInstance())) {
      Instance* instance = static_cast<Instance*>(base->m_data.pobj);
      issetEmptyResult = useEmpty ?
                    instance->propEmpty(ctx, keySD) :
                    instance->propIsset(ctx, keySD);
    } else {
      // Extension class instance.
      CStrRef ctxName = ctx ? CStrRef(ctx->m_name) : CStrRef(null_string);
      issetEmptyResult = useEmpty ?
                    base->m_data.pobj->o_empty(CStrRef(keySD), ctxName) :
                    base->m_data.pobj->o_isset(CStrRef(keySD), ctxName);
    }
    propPost(keySD);
    return issetEmptyResult;
  } else {
    ASSERT(t == KindOfArray);
    ASSERT(base->m_type == KindOfArray);
    TV_WRITE_NULL(&tvScratch);
    result = ElemArray<false>(tvScratch, base, key);
    return useEmpty ? empty(tvAsCVarRef(result)) : isset(tvAsCVarRef(result));
  }
}

static inline void SetPropNull(Cell* val) {
  tvRefcountedDecRefCell(val);
  tvWriteNull(val);
  raise_warning("Cannot access property on non-object");
}
static inline void SetPropStdclass(TypedValue* base, TypedValue* key,
                                   Cell* val) {
  Instance* obj = newInstance(SystemLib::s_stdclassClass);
  obj->incRefCount();
  StringData* keySD = prepareKey(key);
  obj->setProp(NULL, keySD, (TypedValue*)val);
  if (keySD->decRefCount() == 0) {
    keySD->release();
  }
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->_count = 0;
  base->m_data.pobj = obj;
}
// $base->$key = $val
static inline void SetProp(PreClass* ctx, TypedValue* base, TypedValue* key,
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
      SetPropNull(val);
    } else {
      SetPropStdclass(base, key, val);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      SetPropNull(val);
    } else {
      SetPropStdclass(base, key, val);
    }
    break;
  }
  case KindOfObject: {
    StringData* keySD = prepareKey(key);
    // Set property.
    if (LIKELY(base->m_data.pobj->isInstance())) {
      Instance* instance = static_cast<Instance*>(base->m_data.pobj);
      instance->setProp(ctx, keySD, val);
    } else {
      // Extension class instance.
      base->m_data.pobj->o_set(keySD, tvCellAsCVarRef(val));
    }
    LITSTR_DECREF(keySD);
    break;
  }
  default: {
    SetPropNull(val);
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
  Instance* obj = newInstance(SystemLib::s_stdclassClass);
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
// $base->$key <op>= $rhs
static inline TypedValue* SetOpProp(TypedValue& tvScratch, TypedValue& tvRef,
                                    PreClass* ctx, unsigned char op,
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
    if (LIKELY(base->m_data.pobj->isInstance())) {
      StringData* keySD = prepareKey(key);
      Instance* instance = static_cast<Instance*>(base->m_data.pobj);
      result = instance->setOpProp(tvRef, ctx, op, keySD, rhs);
      LITSTR_DECREF(keySD);
    } else {
      // Extension class instance.
      // XXX Not entirely spec-compliant.
      result = prop<true, false, false>(tvScratch, ctx, base, key);
      SETOP_BODY(result, op, rhs);
    }
    break;
  }
  default: {
    result = SetOpPropNull(tvScratch);
    break;
  }
  }
  return result;
}

static inline void IncDecPropNull(TypedValue& dest) {
  raise_warning("Attempt to increment/decrement property of non-object");
  tvWriteNull(&dest);
}
static inline void IncDecPropStdclass(unsigned char op, TypedValue* base,
                                      TypedValue* key, TypedValue& dest) {
  Instance* obj = newInstance(SystemLib::s_stdclassClass);
  obj->incRefCount();
  tvRefcountedDecRef(base);
  base->m_type = KindOfObject;
  base->_count = 0;
  base->m_data.pobj = obj;

  StringData* keySD = prepareKey(key);
  TypedValue tv;
  tvWriteNull(&tv);
  IncDecBody(op, (&tv), &dest);
  obj->setProp(NULL, keySD, &dest);
  ASSERT(!IS_REFCOUNTED_TYPE(tv.m_type));
  LITSTR_DECREF(keySD);
}
static inline void IncDecProp(TypedValue& tvScratch, TypedValue& tvRef,
                              PreClass* ctx, unsigned char op, TypedValue* base,
                              TypedValue* key, TypedValue& dest) {
  DataType type;
  opPre(base, type);
  switch (type) {
  case KindOfUninit:
  case KindOfNull: {
    IncDecPropStdclass(op, base, key, dest);
    break;
  }
  case KindOfBoolean: {
    if (base->m_data.num) {
      IncDecPropNull(dest);
    } else {
      IncDecPropStdclass(op, base, key, dest);
    }
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    if (base->m_data.pstr->size() != 0) {
      IncDecPropNull(dest);
    } else {
      IncDecPropStdclass(op, base, key, dest);
    }
    break;
  }
  case KindOfObject: {
    if (LIKELY(base->m_data.pobj->isInstance())) {
      StringData* keySD = prepareKey(key);
      Instance* instance = static_cast<Instance*>(base->m_data.pobj);
      instance->incDecProp(tvRef, ctx, op, keySD, dest);
      LITSTR_DECREF(keySD);
    } else {
      // Extension class instance.
      // XXX Not entirely spec-compliant.
      TypedValue* result = prop<true, true, false>(tvScratch, ctx, base, key);
      IncDecBody(op, result, &dest);
    }
    break;
  }
  default: {
    IncDecPropNull(dest);
    break;
  }
  }
}

static inline void UnsetProp(PreClass* ctx, TypedValue* base, TypedValue* key) {
  DataType type;
  opPre(base, type);
  // Validate base.
  if (UNLIKELY(type != KindOfObject)) {
    // Do nothing.
    return;
  }
  // Prepare key.
  StringData* keySD = prepareKey(key);
  // Unset property.
  if (LIKELY(base->m_data.pobj->isInstance())) {
    Instance* instance = static_cast<Instance*>(base->m_data.pobj);
    instance->unsetProp(ctx, keySD);
  } else {
    // Extension class instance.
    base->m_data.pobj->o_unset(keySD);
  }

  LITSTR_DECREF(keySD);
}

//=============================================================================
// VarEnv.

VarEnv::VarEnv(bool isGlobalScope /* = false */, bool skipInsert /* = false */)
  : m_cfp(NULL), m_depth(0), m_name2info(NULL), m_extraArgs(NULL),
    m_numExtraArgs(0), m_isGlobalScope(isGlobalScope) {
  if (!skipInsert) {
    g_context->m_varEnvs.push_back(this);
  }
  TRACE(3, "Creating VarEnv %p [%s]\n",
           this,
           m_isGlobalScope ? "global scope" : "local scope");
}

VarEnv::~VarEnv() {
  TRACE(3, "Destroying VarEnv %p [%s]\n",
           this,
           m_isGlobalScope ? "global scope" : "local scope");
  ASSERT(g_context->m_varEnvs.back() == this);
  g_context->m_varEnvs.pop_back();

  ASSERT(m_cfp == NULL);
  if (m_extraArgs != NULL) {
    for (unsigned i = 0; i < m_numExtraArgs; i++) {
      tvRefcountedDecRef(&m_extraArgs[i]);
    }
    free(m_extraArgs);
  }
  if (m_name2info != NULL) {
    if (!m_isGlobalScope) {
      if (m_name2info->decRefCount() == 0) {
        m_name2info->release();
      }
    }
  }
}

void VarEnv::attach(ActRec* fp) {
  TRACE(3, "Attaching VarEnv %p [%s] %d pnames\n",
           this,
           m_isGlobalScope ? "global scope" : "local scope",
           int(fp->m_func->m_pnames.size()));
  ASSERT(m_depth == 0 || g_context->arGetSfp(fp) == m_cfp ||
         (g_context->arGetSfp(fp) == fp && g_context->isNested()));
  m_cfp = fp;
  m_depth++;
  if (m_name2info == NULL) {
    if (!m_isGlobalScope) {
      m_name2info = NEW(HphpArray)(0);
      m_name2info->incRefCount();
    } else {
      SystemGlobals *g = (SystemGlobals*)get_global_variables();
      m_name2info = dynamic_cast<HphpArray*>(
          g->hg_global_storage.getArrayData());
      ASSERT(m_name2info != NULL);
    }
  }
  // Overlay fp's locals.
  const Func* func = fp->m_func;
  TypedValue* loc = frame_local(fp, 0);
  TypedValue** origLocs =
      (TypedValue**)malloc(func->m_pnames.size() * sizeof(TypedValue*));
  for (unsigned i = 0; i < func->m_pnames.size(); i++, loc--) {
    origLocs[i] = m_name2info->migrate(
      const_cast<StringData*>(func->m_pnames[i]), loc);
  }
  m_restoreLocations.push_back(origLocs);
}

void VarEnv::lazyAttach(ActRec* fp) {
  TRACE(3, "Lazily attaching VarEnv %p [%s]\n",
           this,
           m_isGlobalScope ? "global scope" : "local scope");
  ASSERT(m_depth == 0 || g_context->arGetSfp(fp) == m_cfp ||
         (g_context->arGetSfp(fp) == fp && g_context->isNested()));
  m_cfp = fp;
  m_depth++;
  if (m_name2info == NULL) {
    m_name2info = NEW(HphpArray)(0);
    m_name2info->incRefCount();
  }
  // Overlay fp's locals.
  const Func* func = fp->m_func;
  TypedValue* loc = frame_local(fp, 0);
  TypedValue** origLocs =
    (TypedValue**)malloc(func->m_pnames.size() * sizeof(TypedValue*));
  for (unsigned i = 0; i < func->m_pnames.size(); i++, loc--) {
    ASSERT(func->lookupVarId(func->m_pnames[i]) == (int)i);
    origLocs[i] = m_name2info->migrateAndSet(
      const_cast<StringData*>(func->m_pnames[i]), loc);
  }
  m_restoreLocations.push_back(origLocs);
}

void VarEnv::detach(ActRec* fp) {
  TRACE(3, "Detaching VarEnv %p [%s]\n",
           this,
           m_isGlobalScope ? "global scope" : "local scope");
  ASSERT(fp == m_cfp);
  ASSERT(m_depth > 0);
  ASSERT(m_name2info != NULL);
  ASSERT(!m_restoreLocations.empty());

  // Merge/remove fp's overlaid locals.
  const Func* func = fp->m_func;
  TypedValue** origLocs = m_restoreLocations.back();
  for (unsigned i = 0; i < func->m_pnames.size(); i++) {
    m_name2info->migrate(
      const_cast<StringData*>(func->m_pnames[i]), origLocs[i]);
  }
  m_restoreLocations.pop_back();
  free(origLocs);
  ExecutionContext* context = g_context.getNoCheck();
  m_cfp = context->getPrevVMState(fp);
  m_depth--;
  // don't free global varEnv
  if (m_depth == 0) {
    m_cfp = NULL;
    if (context->m_varEnvs.front() != this) {
      ASSERT(!m_isGlobalScope);
      delete this;
    }
  }
}

void VarEnv::set(const StringData* name, TypedValue* tv) {
  ASSERT(m_name2info != NULL);
  m_name2info->nvSet(const_cast<StringData*>(name), tv, false);
}

void VarEnv::bind(const StringData* name, TypedValue* tv) {
  ASSERT(m_name2info != NULL);
  m_name2info->nvBind(const_cast<StringData*>(name), tv, false);
}

TypedValue* VarEnv::lookup(const StringData* name) {
  ASSERT(m_name2info != NULL);
  return m_name2info->nvGet(name);
}

bool VarEnv::unset(const StringData* name) {
  ASSERT(m_name2info != NULL);
  m_name2info->nvRemove(const_cast<StringData*>(name), false);
  return true;
}

void VarEnv::setExtraArgs(TypedValue* args, unsigned nargs) {
  // The VarEnv takes over ownership of the args; the original copies are
  // discarded from the stack without adjusting reference counts, thus allowing
  // VarEnv to avoid reference count manipulation here.
  m_extraArgs = (TypedValue*)malloc(nargs * sizeof(TypedValue));
  m_numExtraArgs = nargs;

  // The stack grows downward, so the args in memory are "backward"; i.e. the
  // leftmost (in PHP) extra arg is highest in memory.  We just copy them in a
  // blob here, and compensate in getExtraArg().
  memcpy(m_extraArgs, args, nargs * sizeof(TypedValue));
}

unsigned VarEnv::numExtraArgs() const {
  return m_numExtraArgs;
}

TypedValue* VarEnv::getExtraArg(unsigned argInd) const {
  ASSERT(argInd < m_numExtraArgs);
  return &m_extraArgs[m_numExtraArgs - argInd - 1];
}

Array VarEnv::getDefinedVariables() const {
  ASSERT(m_name2info != NULL);
  Array ret = Array::Create();
  for (ArrayIter it(m_name2info); !it.end(); it.next()) {
    CVarRef val = it.secondRef();
    if (!val.isInitialized()) {
      continue;
    }
    Variant name(it.first());
    if (val.isReferenced()) {
      ret.setRef(name, val);
    } else {
      ret.add(name, val);
    }
  }
  return ret;
}

//=============================================================================
// Stack.

Stack::Stack(size_t maxelms) {
  // Must be a power of two
  ASSERT((maxelms & (maxelms - 1)) == 0);
  // maxelms-sized and -aligned.
  size_t algnSz = maxelms * sizeof(TypedValue);
  if (posix_memalign((void**)&m_elms, algnSz, algnSz) != 0) {
    fprintf(stderr, "VM stack initialization failed: %s\n", strerror(errno));
    throw std::exception();
  }
  // Burn one element of the stack, to satisfy the constraint that valid
  // m_top values always have the same high-order (> log(maxelms)) bits.
  m_top = m_base = m_elms + maxelms - 1;
  m_maxElms = maxelms;
  ASSERT(!wouldOverflow(maxelms - 1));
  ASSERT(wouldOverflow(maxelms));
}

Stack::~Stack() {
  clear();
  if (m_elms != NULL) {
    free(m_elms);
  }
}

void Stack::toStringElm(std::ostream& os, TypedValue* tv) const {
  switch (tv->m_type) {
  case KindOfVariant:
    os << "V:";
    tv = tv->m_data.ptv;  // Unbox so contents get printed below
    break;
  case KindOfHome:
    os << "H:";
    break;
  case KindOfClass:
    os << "A:";
    break;
  default:
    os << "C:";
    break;
  }
  switch (tv->m_type) {
  case KindOfUninit: {
    os << "Undefined";
    break;
  }
  case KindOfNull: {
    os << "Null";
    break;
  }
  case KindOfBoolean: {
    os << (tv->m_data.num ? "True" : "False");
    break;
  }
  case KindOfInt32:
  case KindOfInt64: {
    os << "0x" << std::hex << tv->m_data.num << std::dec;
    break;
  }
  case KindOfDouble: {
    os << tv->m_data.dbl;
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    os << tv->m_data.pstr << ":\""
       << Util::escapeStringForCPP(tvAsCVarRef(tv).toString().data(),
                                   tvAsCVarRef(tv).toString().size())
       << "\"";
    break;
  }
  case KindOfArray: {
    os << tv->m_data.parr << ":Array";
    break;
  }
  case KindOfObject: {
    os << tv->m_data.pobj << ":Object("
       << tvAsVariant(tv).asObjRef().get()->o_getClassName().get()->data()
       << ")";
    break;
  }
  case KindOfVariant: {
    not_reached();
  }
  case KindOfHome: {
    os << tv->m_data.ptv;
    break;
  }
  case KindOfClass: {
    os << tv->m_data.pcls
       << ":" << tv->m_data.pcls->m_preClass.get()->m_name->data();
    break;
  }
  default: {
    os << "?";
    break;
  }
  }
}

void Stack::toStringIter(std::ostream& os, Iter* it) const {
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    os << "I:Undefined";
    break;
  }
  case Iter::TypeArray: {
    os << "I:Array";
    break;
  }
  case Iter::TypeMutableArray: {
    os << "I:MutableArray";
    break;
  }
  case Iter::TypeIterator: {
    os << "I:Iterator";
    break;
  }
  default: {
    os << "I:?";
    break;
  }
  }
}

void Stack::toStringFrag(std::ostream& os, const ActRec* fp,
                         const TypedValue* top) const {
  TypedValue* tv;

  // The only way to figure out which stack elements are activation records is
  // to follow the m_prevOff chain.  However, the goal for each stack frame is
  // to print stack fragments from deepest to shallowest -- a then b in the
  // following example:
  //
  //   {func:foo,soff:51}<C:8> {func:bar} C:8 C:1 {func:biz} C:0
  //                           aaaaaaaaaaaaaaaaaa bbbbbbbbbbbbbb
  //
  // Use depth-first recursion to get the output order correct.

  tv = (TypedValue*)((uintptr_t)fp
                - (uintptr_t)(fp->m_func->m_numLocals * sizeof(TypedValue))
                - (uintptr_t)(fp->m_func->m_numIterators * sizeof(Iter)));

  for (tv--; (uintptr_t)tv >= (uintptr_t)top; tv--) {
    os << " ";
    toStringElm(os, tv);
  }
}

void Stack::toStringAR(std::ostream& os, const ActRec* fp,
                       const FPIEnt *fe, const TypedValue* top) const {
  ActRec *ar = arAtOffset(fp, -fe->m_fpOff);

  if (fe->m_parentIndex != -1) {
    toStringAR(os, fp, &fp->m_func->m_fpitab[fe->m_parentIndex],
      (TypedValue*)&ar[1]);
  } else {
    toStringFrag(os, fp, (TypedValue*)&ar[1]);
  }

  os << " {func:" << ar->m_func->m_name->data() << "}";
  TypedValue* tv = (TypedValue*)ar;
  for (tv--; (uintptr_t)tv >= (uintptr_t)top; tv--) {
    os << " ";
    toStringElm(os, tv);
  }
}

void Stack::toStringFragAR(std::ostream& os, const ActRec* fp,
                           int offset, const TypedValue* top) const {
  const FPIEnt *fe = fp->m_func->findFPI(offset);
  if (fe != NULL) {
    toStringAR(os, fp, fe, top);
  } else {
    toStringFrag(os, fp, top);
  }
}

void Stack::toStringFrame(std::ostream& os, const ActRec* fp,
                          int offset, const TypedValue* ftop,
                          const string& prefix) const {
  ASSERT(fp);

  // Use depth-first recursion to output the most deeply nested stack frame
  // first.
  {
    Offset prevPc;
    TypedValue* prevStackTop;
    ActRec* prevFp = g_context->getPrevVMState(fp, &prevPc, &prevStackTop);
    if (prevFp != NULL) {
      toStringFrame(os, prevFp, prevPc, prevStackTop, prefix);
    }
  }

  os << prefix;
  os << "{func:" << fp->m_func->m_name->data()
     << ",soff:" << fp->m_soff
     << ",this:0x" << std::hex << (fp->hasThis() ? fp->getThis() : NULL)
     << std::dec << "}";
  TypedValue* tv = (TypedValue*)fp;
  tv--;

  if (fp->m_func->m_numLocals > 0) {
    os << "<";
    for (int i = 0; i < fp->m_func->m_numLocals; i++, tv--) {
      if (i > 0) {
        os << " ";
      }
      toStringElm(os, tv);
    }
    os << ">";
  }

  if (fp->m_func->m_numIterators > 0) {
    os << "|";
    Iter* it = &((Iter*)&tv[1])[-1];
    for (int i = 0; i < fp->m_func->m_numIterators; i++, it--) {
      if (i > 0) {
        os << " ";
      }
      if (fp->m_func->checkIterScope(offset, i)) {
        toStringIter(os, it);
      } else {
        os << "I:Undefined";
      }
    }
    os << "|";
  }

  toStringFragAR(os, fp, offset, ftop);

  os << std::endl;
}

string Stack::toString(const ActRec* fp, int offset,
                       const string prefix/* = "" */) const {
  std::ostringstream os;
  toStringFrame(os, fp, offset, m_top, prefix);

  return os.str();
}

void Stack::clearEvalStack(ActRec *fp, int32 numArgs) {
  TypedValue *evalTop = (TypedValue*)((uintptr_t)fp
                        - (uintptr_t)(numArgs * sizeof(TypedValue)));
  while (m_top < evalTop) {
    popTVImpl<false>();
  }
}

UnwindStatus Stack::unwindFrag(ActRec* fp, int offset,
                               PC& pc, Fault& f) {
  const Func* func = fp->m_func;
  TypedValue* evalTop = (TypedValue*)((uintptr_t)fp
                  - (uintptr_t)(func->m_numLocals) * sizeof(TypedValue)
                  - (uintptr_t)(func->m_numIterators * sizeof(Iter)));
  while (m_top < evalTop) {
    popTVImpl<false>();
  }

  const EHEnt *eh = fp->m_func->findEH(offset);
  while (eh != NULL) {
    if (eh->m_ehtype == EHEnt::EHType_Fault) {
      pc = (uchar*)(fp->m_func->m_unit->entry() + eh->m_fault);
      return UnwindResumeVM;
    } else if (f.m_faultType == Fault::KindOfThrow) {
      TypedValue *tv = &f.m_userException;
      ASSERT(tv->m_type == KindOfObject);
      ObjectData* obj = tv->m_data.pobj;
      std::vector<std::pair<Id, Offset> >::const_iterator it;
      for (it = eh->m_catches.begin(); it != eh->m_catches.end(); it++) {
        StringData *sd = fp->m_func->m_unit->lookupLitstrId(it->first);
        Class* cls = g_context->lookupClass(sd);
        if (cls && obj->instanceof(cls)) {
          pc = fp->m_func->m_unit->at(it->second);
          return UnwindResumeVM;
        }
      }
    }
    if (eh->m_parentIndex != -1) {
      eh = &fp->m_func->m_ehtab[eh->m_parentIndex];
    } else {
      break;
    }
  }

  frame_free_locals_inl_impl<false>(fp);
  ndiscard(fp->m_func->numSlotsInFrame());
  return UnwindNextFrame;
}

void Stack::unwindARFrag(ActRec* ar) {
  while (m_top < (TypedValue*)ar) {
    popTVImpl<false>();
  }
}

void Stack::unwindAR(ActRec* fp, int offset, const FPIEnt* fe) {
  while (true) {
    ActRec* ar = arAtOffset(fp, -fe->m_fpOff);
    ASSERT((TypedValue*)ar >= m_top);
    unwindARFrag(ar);
    popARImpl<false>();
    if (fe->m_parentIndex != -1) {
      fe = &fp->m_func->m_fpitab[fe->m_parentIndex];
    } else {
      return;
    }
  }
}

UnwindStatus Stack::unwindFrame(ActRec*& fp, int offset, PC& pc, Fault& f) {
  TRACE(3, "unwindFrame: func %s, offset %d\n", fp->m_func->m_name->data(),
        offset);
  const FPIEnt *fe = fp->m_func->findFPI(offset);
  if (fe != NULL) {
    unwindAR(fp, offset, fe);
  }
  if (unwindFrag(fp, offset, pc, f) == UnwindResumeVM) {
    return UnwindResumeVM;
  }
  ExecutionContext* context = g_context.getNoCheck();
  ActRec *prevFp = context->arGetSfp(fp);
  Offset prevOff = fp->m_soff + prevFp->m_func->m_base;
  // We don't need to refcount the AR's refcounted members; that was taken care
  // of in frame_free_locals, called from unwindFrag()
  discardAR();
  if (prevFp != fp) {
    // Keep the pc up to date while unwinding.
    const Func *prevF = prevFp->m_func;
    pc = prevF->m_unit->at(prevF->m_base + fp->m_soff);
    fp = prevFp;
    return unwindFrame(fp, prevOff, pc, f);
  }

  // if this is a reentrant VM frame, propagate exception
  if (context->isNested()) {
    if (fp->m_func->isDestructor()) {
      return UnwindIgnore;
    }
    return UnwindPropagate;
  }

  return UnwindExit;
}

void Stack::clear() {
  // Clean up stack if it isn't already empty.
  // XXX
}

bool Stack::wouldOverflow(int numCells) const {
  // The funny approach here is to validate the translator's assembly
  // technique. We've aligned and sized the stack so that the high order
  // bits of valid cells are all the same. In the translator, numCells
  // and m_maxElms can be hardcoded, and m_top is wired into a register,
  // so the expression requires no loads.
  intptr_t truncatedTop = intptr_t(m_top) / sizeof(TypedValue);
  truncatedTop &= m_maxElms - 1;
  intptr_t diff = truncatedTop - numCells;
  return diff < 0;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace VM

//=============================================================================
// ExecutionContext.

using namespace HPHP::VM;
using namespace HPHP::MethodLookup;

ActRec* ExecutionContext::arGetSfp(const ActRec* ar) {
  ActRec* prevFrame = (ActRec*)ar->m_savedRbp;
  if (prevFrame >= m_stack.getStackLowAddress() &&
      prevFrame < m_stack.getStackHighAddress()) {
    return prevFrame;
  }
  return const_cast<ActRec*>(ar);
}

Class* ExecutionContext::defClass(PreClass* preClass,
                                  bool failIsFatal /* = true */) {
  Class* cls;
  if (mapGet(m_definedClasses, preClass->m_name, &cls)) {
    // Raise a fatal unless the existing class definition is identical to the
    // one this invocation would create.
    if (cls->equiv(preClass) == Class::EquivFalse) {
      if (failIsFatal) {
        raise_error("Class already declared: %s", preClass->m_name->data());
      }
      return NULL;
    }
    return cls;
  }
  // Get a compatible Class from the Unit, and add it to the list of defined
  // classes.
  cls = preClass->m_unit->defClass(preClass, failIsFatal);
  if (cls != NULL) {
    m_definedClasses[preClass->m_name] = cls;
  }
  return cls;
}

Class* ExecutionContext::getClass(const StringData* className,
                                  bool tryAutoload) {
  if (className->empty()) return NULL;
  Class* cls = NULL;
  if (!mapGet(m_definedClasses, className, &cls) && tryAutoload) {
    // Try autoloading the class
    AutoloadHandler::s_instance->invokeHandler(className->data());
    (void) mapGet(m_definedClasses, className, &cls); // may be NULL.
  }
  return cls;
}

// Look up the method specified by methodName from the class specified by cls
// and enforce accessibility. Accessibility checks depend on the relationship
// between the class that first declared the method (baseClass) and the context
// class (ctx).
//
// If there are multiple accessible methods with the specified name declared in
// cls and ancestors of cls, the method from the most derived class will be
// returned, except if we are doing an ObjMethod call ("$obj->foo()") and there
// is an accessible private method, in which case the accessible private method
// will be returned.
//
// Accessibility rules:
//
//   | baseClass/ctx relationship | public | protected | private |
//   +----------------------------+--------+-----------+---------+
//   | anon/unrelated             | yes    | no        | no      |
//   | baseClass == ctx           | yes    | yes       | yes     |
//   | baseClass derived from ctx | yes    | yes       | no      |
//   | ctx derived from baseClass | yes    | yes       | no      |
//   +----------------------------+--------+-----------+---------+

const Func* ExecutionContext::lookupMethodCtx(const Class* cls,
                                              const StringData* methodName,
                                              PreClass* pctx,
                                              CallType callType,
                                              bool raise /* = false */) {
  const Class::MethodEntry* e = cls->lookupMethodEntry(methodName);
  if (!e) {
    static StringData* sd__construct
      = StringData::GetStaticString("__construct");
    if (UNLIKELY(methodName == sd__construct)) {
      // We were looking up __construct and failed to find it. Fall back to
      // old-style constructor: same as class name.
      e = &cls->m_ctor;
    }
    if (!e) {
      if (raise) {
        raise_error("Call to undefined method %s::%s from %s%s",
                    cls->m_preClass->m_name->data(),
                    methodName->data(),
                    pctx ? "context " : "anonymous context",
                    pctx ? pctx->m_name->data() : "");
      }
      return NULL;
    }
  }
  Func* method = e->func;
  Class* ctx = NULL;
  // If we found a protected or private method, we need to do some
  // accessibility checks.
  if (method && (e->attrs & (AttrProtected|AttrPrivate)) &&
      !g_context->getDebuggerBypassCheck()) {
    Class* baseClass = e->baseClass;
    ASSERT(baseClass);
    // If the context class is the same as the class that first
    // declared this method, then we know we have the right method
    // and we can stop here.
    if (pctx == baseClass->m_preClass.get()) {
      return method;
    }
    // The anonymous context cannot access protected or private methods,
    // so we can fail fast here.
    if (pctx == NULL) {
      if (raise) {
        raise_error("Call to %s method %s::%s from anonymous context",
                (e->attrs & AttrPrivate) ? "private" : "protected",
                cls->m_preClass->m_name->data(), methodName->data());
      }
      return NULL;
    }
    ASSERT(pctx);
    if (e->attrs & AttrPrivate) {
      // The context class is not the same as the class that declared
      // this private method, so this private method is not accessible.
      // We need to keep going because the context class may define a
      // private method with this name.
      method = NULL;
    } else {
      ctx = lookupClass(pctx->m_name);
      ASSERT(ctx);
      // If the context class is derived from the class that first
      // declared this protected method, then we know this method is
      // accessible and we know the context class cannot have a private
      // method with the same name, so we're done.
      if (ctx->classof(baseClass->m_preClass.get()) != NULL) {
        return method;
      }
      if (baseClass->classof(pctx) == NULL) {
        // The context class is not the same, an ancestor, or a descendent
        // of the class that first declared this protected method, so
        // this method is not accessible. Because the context class is
        // not the same or an ancestor of the class the first declared
        // the method, we know that the context class is not the same
        // or an ancestor of cls, and therefore we don't need to check
        // if the context class declares a private method with this name,
        // so we can fail fast here.
        if (raise) {
          raise_error("Call to protected method %s::%s from context %s",
                      cls->m_preClass->m_name->data(),
                      methodName->data(), pctx->m_name->data());
        }
        return NULL;
      }
      // We now know this protected method is accessible, but we need to
      // keep going because the context class may define a private method
      // with this name.
      ASSERT(method && baseClass->classof(pctx));
    }
  }
  // If this is an ObjMethod call ("$obj->foo()") AND there is an ancestor
  // of cls that declares a private method with this name AND the context
  // class is an ancestor of cls, check if the context class declares a
  // private method with this name.
  if (e->ancestorPrivate && callType == ObjMethod &&
      pctx && cls->classof(pctx)) {
    if (!ctx) ctx = lookupClass(pctx->m_name);
    ASSERT(ctx);
    const Func* ctxMethod = ctx->lookupMethod(methodName);
    // For ObjMethod class a private method from the context class
    // trumps any other method we may have found.
    if (ctxMethod) {
      return ctxMethod;
    }
  }
  if (method) {
    return method;
  }
  if (raise) {
    raise_error("Call to private method %s::%s from %s%s",
                e->baseClass->m_preClass->m_name->data(),
                methodName->data(),
                pctx ? "context " : "anonymous context",
                pctx ? pctx->m_name->data() : "");
  }
  return NULL;
}

LookupResult ExecutionContext::lookupObjMethod(const Func*& f,
                                               const Class* cls,
                                               const StringData* methodName,
                                               bool raise /* = false */) {
  PreClass* pctx = arGetContextPreClass(m_fp);
  f = lookupMethodCtx(cls, methodName, pctx, ObjMethod, false);
  if (!f) {
    f = cls->lookupMethod(s___call.get());
    if (!f) {
      if (raise) {
        // Throw a fatal error
        lookupMethodCtx(cls, methodName, pctx, ObjMethod, true);
      }
      return MethodNotFound;
    }
    return MagicCallFound;
  }
  if (f->m_attrs & AttrStatic) {
    return MethodFoundNoThis;
  }
  return MethodFoundWithThis;
}

LookupResult
ExecutionContext::lookupClsMethod(const Func*& f,
                                  const Class* cls,
                                  const StringData* methodName,
                                  ObjectData* obj,
                                  bool raise /* = false */) {
  PreClass* pctx = arGetContextPreClass(m_fp);
  f = lookupMethodCtx(cls, methodName, pctx, ClsMethod, false);
  if (!f) {
    if (obj && obj->instanceof(cls)) {
      f = obj->getVMClass()->lookupMethod(s___call.get());
    }
    if (!f) {
      f = cls->lookupMethod(s___callStatic.get());
      if (!f) {
        if (raise) {
          // Throw a fatal errpr
          lookupMethodCtx(cls, methodName, pctx, ClsMethod, true);
        }
        return MethodNotFound;
      }
      ASSERT(f);
      ASSERT(f->m_attrs & AttrStatic);
      return MagicCallStaticFound;
    }
    ASSERT(f);
    ASSERT(obj);
    // __call cannot be static, this should be enforced by semantic
    // checks defClass time or earlier
    ASSERT(!(f->m_attrs & AttrStatic));
    return MagicCallFound;
  }
  if (obj && !(f->m_attrs & AttrStatic) && obj->instanceof(cls)) {
    return MethodFoundWithThis;
  }
  return MethodFoundNoThis;
}

ObjectData* ExecutionContext::createObject(StringData* clsName,
                                           CArrRef params,
                                           bool init /* = true */) {
  Class* class_ = loadClass(clsName);
  if (class_ == NULL) {
    throw_missing_class(clsName->data());
  }
  Object o;
  o = newInstance(class_);
  if (init) {
    // call constructor
    TypedValue ret;
    invokeFunc(&ret, class_->m_ctor.func, params, o.get());
    tvRefcountedDecRef(&ret);
  }
  // the caller needs to own a reference to the created object.
  // incref the object reference so that the object destructor
  // does not free it on exit from this function.
  o->incRefCount();
  return o.get();
}

ObjectData* ExecutionContext::createObjectOnly(StringData* clsName) {
  return createObject(clsName, null_array, false);
}

ActRec* ExecutionContext::getStackFrame(int level) {
  ActRec* ar = m_fp;
  for (int i = 0; i < level && ar != NULL; ++i) {
    while (ar != NULL && ar->m_func->isPseudoMain()) {
      ar = arGetSfp(ar);
    }
    if (ar != NULL) {
      ar = arGetSfp(ar);
    }
  }
  return ar;
}

ObjectData* ExecutionContext::getThis(bool skipFrame /* = false */) {
  ActRec* fp = m_fp;
  if (skipFrame) {
    fp = arGetSfp(fp);
    ASSERT(fp != NULL);
  }
  if (fp->hasThis()) {
    return fp->getThis();
  }
  return NULL;
}

CStrRef ExecutionContext::getContextClassName(bool skipFrame /* = false */) {
  ActRec* ar = m_fp;
  ASSERT(ar != NULL);
  if (skipFrame) {
    ar = arGetSfp(ar);
    ASSERT(ar != NULL);
  }
  if (ar->hasThis()) {
    return ar->getThis()->o_getClassName();
  } else if (ar->hasClass()) {
    return CStrRef(ar->getClass()->m_preClass->m_name);
  } else {
    return empty_string;
  }
}

CStrRef ExecutionContext::getParentContextClassName(bool skip /* = false */) {
  ActRec* ar = m_fp;
  ASSERT(ar != NULL);
  if (skip) {
    ar = arGetSfp(ar);
    ASSERT(ar != NULL);
  }
  if (ar->hasThis()) {
    const Class* cls = ar->getThis()->getVMClass();
    if (cls->m_parent.get() == NULL) {
      return empty_string;
    }
    return CStrRef(cls->m_parent->m_preClass->m_name);
  } else if (ar->hasClass()) {
    const Class* cls = ar->getClass();
    if (cls->m_parent.get() == NULL) {
      return empty_string;
    }
    return CStrRef(cls->m_parent->m_preClass->m_name);
  } else {
    return empty_string;
  }
}

CStrRef ExecutionContext::getContainingFileName(bool skipFrame /* = false */) {
  ActRec* ar = m_fp;
  if (ar == NULL) return empty_string;
  if (skipFrame) {
    ar = getPrevVMState(ar);
    if (ar == NULL) return empty_string;
  }
  Unit* unit = ar->m_func->m_unit;
  if (unit == NULL) return empty_string;
  return *(String*)(&unit->m_filepath);
}

int ExecutionContext::getLine(bool skipFrame /* = false */) {
  ActRec* ar = m_fp;
  Unit* unit = ar ? ar->m_func->m_unit : NULL;
  Offset pc = unit ? pcOff() : 0;
  if (ar == NULL) return -1;
  if (skipFrame) {
    ar = getPrevVMState(ar, &pc);
  }
  if (ar == NULL || (unit = ar->m_func->m_unit) == NULL) return -1;
  return unit->getLineNumber(pc);
}

Array ExecutionContext::getCallerInfo(bool skipFrame /* = false */) {
  Array result = Array::Create();
  ActRec* ar = m_fp;
  if (skipFrame) {
    ar = getPrevVMState(ar);
  }
  while (ar->m_func->m_name->isame(s_call_user_func.get())
         || ar->m_func->m_name->isame(s_call_user_func_array.get())) {
    ar = getPrevVMState(ar);
    if (ar == NULL) {
      return result;
    }
  }

  Offset pc;
  ar = getPrevVMState(ar, &pc);
  while (ar != NULL) {
    if (!ar->m_func->m_name->isame(s_call_user_func.get())
        && !ar->m_func->m_name->isame(s_call_user_func_array.get())) {
      Unit* unit = ar->m_func->m_unit;
      if (unit != NULL && unit->getLineNumber(pc) != -1) {
        result.set(s_file, unit->m_filepath->data(), true);
        result.set(s_line, unit->getLineNumber(pc));
        return result;
      }
    }
    ar = getPrevVMState(ar, &pc);
  }
  return result;
}

bool ExecutionContext::defined(CStrRef name) {
  return m_constants.nvGet(name.get()) != NULL;
}

TypedValue* ExecutionContext::getCns(StringData* cns, bool system /* = true */,
                                     bool dynamic /* = true */) {
  if (dynamic) {
    TypedValue* tv = m_constants.nvGet(cns);
    if (tv != NULL) {
      return tv;
    }
  }
  if (system) {
    const ClassInfo::ConstantInfo* ci = ClassInfo::FindConstant(cns->data());
    if (ci != NULL) {
      if (!dynamic) {
        ConstInfoMap::const_iterator it = m_constInfo.find(cns);
        if (it != m_constInfo.end()) {
          // This is a dynamic constant, so don't report it.
          ASSERT(ci == it->second);
          return NULL;
        }
      }
      TypedValue tv;
      TV_WRITE_UNINIT(&tv);
      tvAsVariant(&tv) = ci->getValue();
      m_constants.nvSet(cns, &tv, false);
      tvRefcountedDecRef(&tv);
      return m_constants.nvGet(cns);
    }
  }
  return NULL;
}

bool ExecutionContext::setCns(StringData* cns, CVarRef val) {
  if (m_constants.nvGet(cns) != NULL ||
      ClassInfo::FindConstant(cns->data()) != NULL) {
    raise_warning("Constant %s already defined", cns->data());
    return false;
  }
  if (val.isArray() || val.isObject()) {
    raise_warning("Constants may only evaluate to scalar values");
    return false;
  }
  m_constants.nvSet(cns, (TypedValue*)(&val), false);
  ASSERT(m_constants.nvGet(cns) != NULL);
  m_transl->defineCns(cns);
  return true;
}

Func* ExecutionContext::lookupFunc(const StringData* funcName) {
  return m_funcDict.get(funcName);
}

bool ExecutionContext::renameFunction(const StringData* oldName,
                                      const StringData* newName) {
  return m_funcDict.rename(oldName, newName);
}

bool ExecutionContext::isFunctionRenameable(const StringData* name) {
  return m_funcDict.isFunctionRenameable(name);
}

void ExecutionContext::addRenameableFunctions(ArrayData* arr) {
  m_funcDict.addRenameableFunctions(arr);
}

VarEnv* ExecutionContext::getVarEnv() {
  // getVarEnv is supposed to be called only from compact/extract.
  // so, m_fp points to frame for builtin call
  ActRec *fp = arGetSfp(m_fp);
  ASSERT(fp != m_fp);
  ASSERT(!fp->hasInvName());
  if (fp->m_varEnv == NULL) {
    fp->m_varEnv = new VarEnv();
    fp->m_varEnv->lazyAttach(fp);
  }
  return fp->m_varEnv;
}

void ExecutionContext::setVar(StringData* name, TypedValue* v, bool ref) {
  // setVar() should only be called after getVarEnv() has been called
  // to create a varEnv
  ActRec *fp = arGetSfp(m_fp);
  ASSERT(fp != m_fp);
  ASSERT(!fp->hasInvName());
  ASSERT(fp->m_varEnv != NULL);
  if (ref) {
    fp->m_varEnv->bind(name, v);
  } else {
    fp->m_varEnv->set(name, v);
  }
}

Array ExecutionContext::getLocalDefinedVariables(int frame) {
  ActRec *fp = m_fp;
  for (; frame > 0; --frame) {
    if (!fp) break;
    fp = getPrevVMState(fp);
  }
  if (!fp) {
    return Array();
  }
  ASSERT(!fp->hasInvName());
  if (fp->m_varEnv) {
    return fp->m_varEnv->getDefinedVariables();
  }
  Array ret;
  const Func *func = fp->m_func;
  hphp_hash_map<const StringData*, Id,
    string_data_hash, string_data_same>::const_iterator it =
      func->m_name2pind.begin();
  for (; it != func->m_name2pind.end(); ++it) {
    TypedValue* ptv = frame_local(fp, it->second);
    if (ptv->m_type == KindOfUninit) {
      continue;
    }
    Variant name(it->first->data());
    ret.add(name, tvAsVariant(ptv));
  }
  return ret;
}

void ExecutionContext::shuffleMagicArgs(ActRec* ar, int nargs) {
    // We need to put this where the first argument is
    StringData* invName = ar->getInvName();
    ar->setVarEnv(NULL);
    ASSERT(!ar->hasVarEnv() && !ar->hasInvName());
    // We need to make an array containing all the arguments passed by the
    // caller and put it where the second argument is
    HphpArray* argArray = pack_args_into_array(ar, nargs);
    argArray->incRefCount();
    // Remove the arguments from the stack
    for (int i = 0; i < nargs; ++i) {
      m_stack.popC();
    }
    // Move invName  to where the first argument belongs, no need
    // to incRef/decRef since we are transferring ownership
    m_stack.pushStringNoRc(invName);
    // Move argArray to where the second argument belongs. We've already
    // incReffed the array above so we don't need to do it here.
    m_stack.pushArrayNoRc(argArray);
}

static inline void checkStack(Stack& stk, const Func* f) {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  // Check whether func's maximum stack usage would overflow the stack.
  // Both native and VM stack overflows are independently possible.
  if (!stack_in_bounds(info) ||
      stk.wouldOverflow(f->maxStackCells())) {
    TRACE(1, "Maximum VM stack depth exceeded.\n");
    raise_error("Stack overflow");
  }
}

void ExecutionContext::callBuiltin(ActRec* ar) {
  INST_HOOK_FENTRY(ar->m_func->m_fullName);
  // XXX We need to save numArgs and fp separately because the builtin trashes
  // m_numArgs and m_savedRbp with the return value.
  int32 savedNumArgs = ar->m_numArgs;
  ActRec* savedFp = m_fp;
  m_fp = ar;
  if (m_fp->hasInvName()) {
    shuffleMagicArgs(m_fp, savedNumArgs);
    savedNumArgs = ar->m_numArgs = 2;
  }

  DynTracer::FunctionEnter(ar);
  if (ar->m_func->m_preClass != NULL) {
    // Builtin Class Function
    BuiltinClassFunction funcPtr = ar->m_func->m_builtinClassFuncPtr;
    ASSERT(funcPtr);
    funcPtr(ar);
  } else {
    // Builtin.
    TypedValue*(*funcPtr)(ActRec* ar) =
      (TypedValue*(*)(ActRec* ar))ar->m_func->m_builtinFuncPtr;
    ASSERT(funcPtr);
    funcPtr(ar);
  }
  DynTracer::FunctionExit(ar);

  m_fp = savedFp;
  if (ar->m_r.m_type == KindOfInt32) {
    ar->m_r.m_type = KindOfInt64;
  }
  // It's the builtin's responsibility to downref any args; we can simply
  // discard.
  m_stack.ndiscard(savedNumArgs);
  ASSERT(!ar->hasVarEnv() && !ar->hasInvName());
  // Leave return value on stack.
  m_stack.ret();
  // For debugger, normally bytecode inside the function will put down
  // the flag but not for builtin.
  g_context->m_debuggerFuncEntry = false;
}

template <bool checkOverflow>
bool ExecutionContext::prepareNonBuiltinEntry(ActRec *ar, PC& pc) {
  int nargs = ar->m_numArgs;
  const Func* func = ar->m_func;
  if (checkOverflow) {
    checkStack(m_stack, func);
  }

  if (UNLIKELY(ar->hasInvName())) {
    shuffleMagicArgs(ar, nargs);
    // Fix up nargs and the ActRec's m_numArgs field
    nargs = ar->m_numArgs = 2;
  }

  // It is now safe to access m_varEnv directly
  ASSERT(!ar->hasInvName());

  FuncDict::InterceptData* data = intercept_data(ar);
  if (UNLIKELY(data != NULL)) {
    bool enterFunction = run_intercept_handler(ar, data);
    if (!enterFunction) {
      return false;
    }
  }

  // Set pc below, once we know that DV dispatch is unnecessary.
  m_fp = ar;

  int nparams = func->m_numParams;
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  if (nargs != nparams) {
    if (nargs < nparams) {
      // Push uninitialized nulls for missing arguments. Some of them may end up
      // getting default-initialized, but regardless, we need to make space for
      // them on the stack.
      for (int i = nargs; i < nparams; ++i) {
        Offset dvInitializer = func->m_params[i].m_funcletOff;
        if (dvInitializer == InvalidAbsoluteOffset) {
          // m_pc is not set to callee. if the callee is in a
          // different unit, debugBacktrace() can barf in unit->offsetOf(m_pc)
          // where it asserts that m_pc >= m_bc && m_pc < m_bc + m_bclen.
          // Sync m_fp to function entry point in called unit.
          pc = func->getEntry();
          SYNC();
          raise_warning("Missing argument %d to %s()",
                        i + 1, func->m_name->data());
        } else {
          if (firstDVInitializer == InvalidAbsoluteOffset) {
            // This is the first unpassed arg with a default value, so this is
            // where we'll need to jump to.
            firstDVInitializer = dvInitializer;
          }
        }
        m_stack.pushUninit();
      }
      ASSERT(m_fp->m_func == func);
    } else {
      // Extra parameters must be moved off the stack.
      if (m_fp->m_varEnv == NULL) {
        m_fp->m_varEnv = new VarEnv();
        m_fp->m_varEnv->lazyAttach(m_fp);
      }
      int numExtras = nargs - nparams;
      m_fp->m_varEnv->setExtraArgs(
        (TypedValue*)(uintptr_t(m_fp) - nargs * sizeof(TypedValue)),
        numExtras);
      for (int i = 0; i < numExtras; i++) {
        m_stack.discard();
      }
    }
  }
  pushLocalsAndIterators(func, nparams);
  if (firstDVInitializer != InvalidAbsoluteOffset) {
    pc = func->m_unit->entry() + firstDVInitializer;
  } else {
    pc = func->getEntry();
  }
  SYNC();
  DynTracer::FunctionEnter(ar);
  INST_HOOK_FENTRY(func->m_fullName);

  return true;
}

void ExecutionContext::handleExit() {
  ASSERT(m_halted);
  // this is the final VM frame, do some cleanups before exiting
  m_fp = NULL;
  m_pc = NULL;
  ASSERT(m_faults.size() == 1);
  Fault fault = m_faults.back();
  m_faults.pop_back();
  switch(fault.m_faultType) {
  case Fault::KindOfExit: {
    m_stack.pushNull();
    break;
  }
  case Fault::KindOfFatal: {
    CStrRef obj = tvAsCVarRef(&fault.m_userException).asCStrRef();
    raise_error(obj.data());
    break;
  }
  case Fault::KindOfThrow: {
    // unhandled exception: free the exception object
    m_halted = false;
    Instance *i =
      reinterpret_cast<Instance*>(fault.m_userException.m_data.pobj);
    Object exn(i);
    tvDecRef(&fault.m_userException);
    onUnhandledException(exn);
    m_halted = true;
    m_stack.pushNull();
    break;
  }
  case Fault::KindOfCPPException: {
    if (fault.m_cppException != NULL) {
      fault.m_cppException->throwException();
    }
    //XXX: placeholder to pretty print exception stack
    fprintf(stderr, "Unhandled CPP exception\n");
    m_stack.pushNull();
    break;
  }
  default: ASSERT(false);
  }
}

void ExecutionContext::syncGdbState() {
  if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitNoGdb) {
    ((VM::Transl::TranslatorX64*)m_transl)->m_debugInfo.debugSync();
  }
}

#define DO_ENTER_VM(pc, isbuiltin, ar) do {                      \
  bool handleErr = false;                                        \
  Fault fault;                                                   \
  try {                                                          \
    if (isbuiltin) {                                             \
      callBuiltin(ar);                                           \
    } else {                                                     \
      if (RuntimeOption::EvalJit) {                              \
        Transl::SrcKey sk(Transl::curUnit(), pc);                \
        (void) curUnit()->offsetOf(pc); /* assert */             \
        m_transl->resume(sk);                                    \
      } else {                                                   \
        dispatch();                                              \
      }                                                          \
    }                                                            \
  } catch (Exception &e) {                                       \
    if (!g_context->m_propagateException) {                      \
      fault.m_faultType = Fault::KindOfCPPException;             \
      fault.m_cppException = e.clone();                          \
      m_faults.push_back(fault);                                 \
    }                                                            \
    handleErr = true;                                            \
  } catch (...) {                                                \
    if (!g_context->m_propagateException) {                      \
      fault.m_faultType = Fault::KindOfCPPException;             \
      fault.m_cppException = new Exception("unknown exception"); \
      m_faults.push_back(fault);                                 \
    }                                                            \
    handleErr = true;                                            \
  }                                                              \
  if (handleErr) {                                               \
    hhvmThrow();                                                 \
  }                                                              \
} while(0)

void ExecutionContext::enterVM(TypedValue* retval, const Func* f,
                               VarEnv* varEnv, ActRec* ar) {
  bool enter = true;
  if (!f->isBuiltin()) {
    // We have already checked for stack overflow in invokeFunc().
    // Don't check again while we are in the middle of re-entering
    // the VM.
    enter = prepareNonBuiltinEntry<false>(ar, m_pc);
  }
  if (varEnv) {
    ar->m_varEnv->attach(ar);
  }
  ar->m_savedRip = (uintptr_t)m_transl->getCallToExit();
  m_fp = ar;
  m_firstAR = ar;
  m_nestedVMMap[ar] = m_nestedVMs.size() - 1;
  m_halted = false;

  if (enter) {
    jmp_buf buf;
    m_jmpBufs.push_back(&buf);
    __asm__ __volatile__("":::"memory");
    if (setjmp(buf) == 0) {
      __asm__ __volatile__("":::"memory");
      DO_ENTER_VM(m_pc, f->isBuiltin(), ar);
    } else if (!m_ignoreException) {
      __asm__ __volatile__("":::"memory");
      if (m_propagateException) {
        m_jmpBufs.pop_back();
        Fault *f = &m_faults.back();
        switch(f->m_faultType) {
          case Fault::KindOfThrow:
            throw Object(f->m_userException.m_data.pobj);
          case Fault::KindOfCPPException:
            f->m_cppException->throwException();
          default:
            throw std::exception();
        }
      } else if (!m_halted) {
        DO_ENTER_VM(m_pc, false, ar);
      } else {
        handleExit();
      }
    }
    m_jmpBufs.pop_back();
  }

  m_nestedVMMap.erase(ar);
  m_halted = false;

  memcpy(retval, m_stack.topTV(), sizeof(TypedValue));
  m_stack.discard();
}

void ExecutionContext::invokeFunc(TypedValue* retval, const Func* f,
                                  CArrRef params,
                                  ObjectData* this_ /* = NULL */,
                                  Class* cls /* = NULL */,
                                  VarEnv* varEnv /* = NULL */,
                                  StringData* invName /* = NULL */,
                                  Unit* toMerge /* = NULL */) {
  ASSERT(retval);
  ASSERT(f);
  // If this is a regular function, this_ and cls must be NULL
  ASSERT(f->m_preClass || f->isPseudoMain() || (!this_ && !cls));
  // If this is a method, either this_ or cls must be non-NULL
  ASSERT(!f->m_preClass || (this_ || cls));
  // If this is a static method, this_ must be NULL
  ASSERT(!(f->m_attrs & HPHP::VM::AttrStatic) || (!this_));
  // invName should only be non-NULL if we are calling __call or
  // __callStatic
  ASSERT(!invName || f->m_name->isame(s___call.get()) ||
         f->m_name->isame(s___callStatic.get()));
  // If a variable environment is being inherited then params must be empty
  ASSERT(!varEnv || params.empty());

  bool isMagicCall = (invName != NULL);
  if (this_ != NULL) {
    this_->incRefCount();
  }
  Cell* savedSP = m_stack.top();
  if (RuntimeOption::EvalJit && m_fp != NULL) {
    // We only need to do this in jit mode, because it may reenter the VM
    // without having m_stack.top in sync with the in-memory stack. Just having
    // it in sync with %rbx is not enough, because memory at negative offsets
    // from %rbx can be live.
    // XXX: This is bad, because we are filling up the eval stack with garbage
    // by setting m_stack.top() to the max possible value. If an exception is
    // throw in this context, the stack unwinder will try to free the garbage
    // values on the eval stack. This also throws off the stack space
    // availability check elsewhere in the code.
    m_stack.top() -= m_fp->m_func->maxStackCells();
  }

  checkStack(m_stack, f);

  ActRec* ar = m_stack.allocA();
  ar->m_func = f;
  if (this_) {
    ar->setThis(this_);
    ar->m_staticLocalCtx = this_->getVMClass()->getStaticLocals();
  } else if (cls) {
    ar->setClass(cls);
    ar->m_staticLocalCtx = cls->getStaticLocals();
  } else if (f->m_isGenerator && f->m_needsStaticLocalCtx) {
    // If this is a generator function body we should have one
    // argument: the Continuation object
    ASSERT(params.size() == 1);
    c_GenericContinuation* cont =
      dynamic_cast<c_GenericContinuation*>(params.rvalAt(0).toObject().get());
    ASSERT(cont != NULL);
    ar->setThis(NULL);
    ar->m_staticLocalCtx = cont->getStaticLocals();
  } else {
    ar->setThis(NULL);
    ASSERT(!f->m_needsStaticLocalCtx);
  }
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  if (isMagicCall) {
    ar->m_numArgs = 2;
  } else {
    ar->m_numArgs = params.size();
  }
  ar->setVarEnv(varEnv);

#ifdef HPHP_TRACE
  if (m_fp == NULL) {
    TRACE(1, "Reentry: enter %s(%p) from top-level\n",
          f->m_name->data(), ar);
  } else {
    TRACE(1, "Reentry: enter %s(pc %p ar %p) from %s(%p)\n",
          f->m_name->data(), m_pc, ar,
          m_fp->m_func->m_name->data(), m_fp);
  }
#endif
  ASSERT(!RuntimeOption::EvalJit || m_isValid);

  int paramId = 0;
  HphpArray *arr = dynamic_cast<HphpArray*>(params.get());
  if (isMagicCall) {
    // Put the method name into the location of the first parameter. We
    // are transferring ownership, so no need to incRef/decRef here.
    m_stack.pushStringNoRc(invName);
    // Put array of arguments into the location of the second parameter
    m_stack.pushArray(arr);
  } else {
    if (arr) {
      for (ssize_t i = arr->iter_begin(); i != ArrayData::invalid_index;
           i = arr->iter_advance(i), paramId++) {
        TypedValue *to = m_stack.allocTV();
        TypedValue *from = arr->nvGetValueRef(i);
        if (paramId > f->m_numParams || !f->byRef(paramId)) {
          tvDup(from, to);
          if (to->m_type == KindOfVariant) {
            tvUnbox(to);
          }
        } else {
          if (from->m_type != KindOfVariant) {
            tvBox(from);
          }
          tvDup(from, to);
        }
      }
    }
  }

  if (toMerge != NULL) {
    mergeUnit(toMerge);
  }

  if (m_fp) {
    VMState savedVM = { m_pc, m_fp, m_firstAR, savedSP };
    reenterVM(savedVM);
    ASSERT(m_nestedVMs.size() >= 1);
    enterVM(retval, f, varEnv, ar);
    m_pc = savedVM.pc;
    m_fp = savedVM.fp;
    m_firstAR = savedVM.firstAR;
    exitVM();
    if (!RuntimeOption::EvalJit) {
      ASSERT(m_stack.top() == savedVM.sp);
    }
    m_stack.top() = savedVM.sp;
    TRACE(1, "Reentry: exit\n");
  } else {
    ASSERT(m_nestedVMs.size() == 0);
    enterVM(retval, f, varEnv, ar);
  }
}

void ExecutionContext::invokeUnit(Unit* unit) {
  TypedValue rv;
  Func* func = unit->getMain();
  VarEnv* varEnv;
  if (g_context->m_varEnvs.empty()) {
    // The global variable environment hasn't been created yet, so
    // create it
    varEnv = new VarEnv(true);
  } else {
    // Get the global variable environment
    varEnv = g_context->m_varEnvs.front();
  }
  invokeFunc(&rv, func, Array::Create(), NULL, NULL, varEnv, NULL, unit);
}


Variant i_callUserFunc(void *extra, CArrRef params) {
  Variant v;
  g_context->invokeFunc((TypedValue*)&v, (Func*)extra, params);
  return v;
}

// XXX hackity hack hack
// This is solely here to support generator methods. The C++ implementation of
// the Continuation class is the only thing that uses this function. It also
// does not get static-method generators right when inheritance is involved: it
// will be using the class context where the generator is defined, rather than
// where the generator is called.
Variant i_callUserFunc1ArgMCP(MethodCallPackage& mcp, int num, CVarRef arg0) {
  ASSERT(num == 1);
  Func* func = (Func*)mcp.extra;
  Class* cls = NULL;
  if (mcp.isObj && mcp.obj == NULL) {
    // If mcp.obj is NULL, this must be a static generator method and we don't
    // have a useful class context. Grab the proper one from the c_Continuation
    // object.
    ASSERT(arg0.isObject());
    c_Continuation* cont =
      dynamic_cast<c_Continuation*>(arg0.toObject().get());
    ASSERT(cont != NULL);
    cls = g_context->lookupClass(cont->m_called_class.get());
  }
  Variant v;
  g_context->invokeFunc((TypedValue*)&v, func, CREATE_VECTOR1(arg0),
                        mcp.obj, cls);
  return v;
}

CallInfo ci_callUserFunc((void *)&i_callUserFunc,
                         (void *)&i_callUserFunc1ArgMCP,
                         0, 0, 0);

bool ExecutionContext::getCallInfo(const CallInfo *&outCi,
                                   void *&outExtra, const char *s) {
  StringData funcName(s);
  Func* func = m_funcDict.get(&funcName);
  if (!func) {
    return false;
  }
  outExtra = (void*)func;
  outCi = &ci_callUserFunc;
  return true;
}

bool ExecutionContext::getCallInfoStatic(const CallInfo*& outCi,
                                         void*& outExtra,
                                         const StringData* clsName,
                                         const StringData* funcName) {
  Class* cls = lookupClass(clsName);
  if (cls == NULL) {
    return false;
  }
  const Func* method = cls->lookupMethod(funcName);
  if (method == NULL) {
    return false;
  }
  outExtra = (void *)method;
  outCi = &ci_callUserFunc;
  return true;
}

void ExecutionContext::hhvmThrow() {
  Fault fault = m_faults.back();
  m_propagateException = false;
  m_ignoreException = false;

  /* discard the parameters and ActRec for the call to the extension */
  bool propagate = false;
  if (m_fp->m_func->isBuiltin()) {
    int32 numArgs = m_fp->m_numArgs;
    ActRec* sfp = arGetSfp(m_fp);
    m_stack.clearEvalStack(m_fp, numArgs);
    if (sfp != m_fp) {
      m_fp = sfp;
    } else {
      // This is the last frame for this entry into the VM, so propagate the
      // exception rather than trying to unwind this VM entry further.
      propagate = true;
    }
    m_stack.ndiscard(numArgs + kNumActRecCells);
  }
  UnwindStatus unwindType;
  if (propagate) {
    unwindType = UnwindPropagate;
  } else {
    unwindType = m_stack.unwindFrame(m_fp, pcOff(),
                                     m_pc, fault);
  }
  handleUnwind(unwindType);
}

/*
 * Given a pointer to a VM frame, returns the previous VM frame in the call
 * stack. This function will also pass back by reference the previous PC (if
 * prevFP is non-null) and the previous SP (if prevSp is non-null).
 *
 * If there is no previous VM frame, this function returns NULL and does not
 * set prevPc and prevSp.
 */
ActRec* ExecutionContext::getPrevVMState(const ActRec* fp,
                                         Offset*       prevPc /* = NULL */,
                                         TypedValue**  prevSp /* = NULL */) {
  ActRec* prevFp = arGetSfp(fp);
  if (prevFp != fp) {
    if (prevSp) *prevSp = (TypedValue*)&fp[1];
    if (prevPc) *prevPc = prevFp->m_func->m_base + fp->m_soff;
    return prevFp;
  }
  ASSERT(m_nestedVMMap.find(fp) != m_nestedVMMap.end());
  int k = m_nestedVMMap[fp];
  if (k < 0) {
    return NULL;
  }
  ASSERT(k < (int)m_nestedVMs.size());
  prevFp = m_nestedVMs[k].fp;
  if (prevSp) *prevSp = m_nestedVMs[k].sp;
  if (prevPc) {
    *prevPc = (prevFp->m_func->m_unit != NULL) ?
      prevFp->m_func->m_unit->offsetOf(m_nestedVMs[k].pc) : 0;
  }
  return prevFp;
}

Array ExecutionContext::debugBacktrace(bool skip /* = false */,
                                       bool withSelf /* = false */,
                                       bool withThis /* = false */,
                                       VMParserFrame*
                                         parserFrame /* = NULL */) {
  static StringData* s_file = StringData::GetStaticString("file");
  static StringData* s_line = StringData::GetStaticString("line");
  static StringData* s_function = StringData::GetStaticString("function");
  static StringData* s_args = StringData::GetStaticString("args");
  static StringData* s_class = StringData::GetStaticString("class");
  static StringData* s_object = StringData::GetStaticString("object");
  static StringData* s_type = StringData::GetStaticString("type");

  Array bt = Array::Create();

  // If there is a parser frame, put it at the beginning of
  // the backtrace
  if (parserFrame) {
    Array frame = Array::Create();
    frame.set(String(s_file), parserFrame->filename, true);
    frame.set(String(s_line), parserFrame->lineNumber, true);
    bt.append(frame);
  }
  if (!m_fp) {
    // If there are no VM frames, we're done
    return bt;
  }
  // Get the fp and pc of the top frame (possibly skipping one frame)
  ActRec* fp;
  Offset pc;
  if (skip) {
    fp = getPrevVMState(m_fp, &pc);
    if (!fp) {
      // We skipped over the only VM frame, we're done
      return bt;
    }
  } else {
    fp = m_fp;
    Unit *unit = m_fp->m_func->m_unit;
    pc = unit ? unit->offsetOf(m_pc) : 0;
  }
  // Handle the top frame
  if (withSelf) {
    Unit *unit = fp->m_func->m_unit;
    if (unit != NULL) {
      const char* filename = unit->m_filepath->data();
      ASSERT(filename);
      Offset off = pc;
      Array frame = Array::Create();
      frame.set(String(s_file), filename, true);
      frame.set(String(s_line), unit->getLineNumber(off), true);
      if (parserFrame) {
        frame.set(String(s_function), "include", true);
        frame.set(String(s_args), Array::Create(parserFrame->filename), true);
      }
      bt.append(frame);
    }
  }
  // Handle the subsequent VM frames
  for (ActRec* prevFp = getPrevVMState(fp, &pc); prevFp != NULL;
       fp = prevFp, prevFp = getPrevVMState(fp, &pc)) {
    Array frame = Array::Create();
    // builtins don't have a unit
    Unit* unit = prevFp->m_func->m_unit;
    if (unit != NULL) {
      const char *filename = unit->m_filepath->data();
      ASSERT(filename);
      frame.set(String(s_file), filename, true);
      frame.set(String(s_line),
                prevFp->m_func->m_unit->getLineNumber(pc), true);
    }
    // check for include
    StringData *funcname = const_cast<StringData*>(fp->m_func->m_name);
    if (fp->m_func->m_isClosureBody) {
      funcname = StringData::GetStaticString("{closure}");
    }
    frame.set(String(s_function), String(funcname), true);
    // Closures have an m_this but they aren't in object context
    PreClass* ctx = arGetContextPreClass(fp);
    if (ctx != NULL && !fp->m_func->m_isClosureBody) {
      frame.set(String(s_class), ctx->m_name->data(), true);
      if (fp->hasThis()) {
        if (withThis) {
          frame.set(String(s_object), Object(fp->getThis()), true);
        }
        frame.set(String(s_type), "->", true);
      } else {
        frame.set(String(s_type), "::", true);
      }
    }
    Array args = Array::Create();
    int nparams = fp->m_func->m_numParams;
    int nargs = fp->m_numArgs;
    /* builtin extra args are not stored in varenv */
    if (nargs <= nparams || fp->m_func->isBuiltin()) {
      for (int i = 0; i < nargs; i++) {
        TypedValue *arg = frame_local(fp, i);
        args.append(tvAsVariant(arg));
      }
    } else {
      int i;
      for (i = 0; i < nparams; i++) {
        TypedValue *arg = frame_local(fp, i);
        args.append(tvAsVariant(arg));
      }
      for (; i < nargs; i++) {
        ASSERT(fp->hasVarEnv());
        TypedValue *arg = fp->getVarEnv()->getExtraArg(i - nparams);
        args.append(tvAsVariant(arg));
      }
    }
    frame.set(String(s_args), args, true);
    bt.append(frame);
  }
  return bt;
}

void ExecutionContext::mergeUnit(Unit* unit) {
  // Classes.
  for (std::vector<PreClass*>::const_iterator
       it = unit->m_hoistablePreClassVec.begin();
       it != unit->m_hoistablePreClassVec.end(); ++it) {
    defClass(*it, false);
  }
  mergeUnitFuncs(unit);
}

void ExecutionContext::mergeUnitFuncs(Unit* unit) {
  for (Unit::FuncMap::const_iterator it = unit->m_funcMap.begin();
       it != unit->m_funcMap.end(); ++it) {
    if (m_funcDict.get(it->first)) {
      raise_error("Function already defined: %s", it->first->data());
    }
    m_funcDict.insert(it->first, it->second);
  }
}

MethodInfoVM::~MethodInfoVM() {
  for (std::vector<const ClassInfo::ParameterInfo*>::iterator it =
       parameters.begin(); it != parameters.end(); ++it) {
    if ((*it)->value != NULL) {
      free((void*)(*it)->value);
    }
  }
}

ClassInfoVM::~ClassInfoVM() {
  destroyMembers(m_methodsVec);
  destroyMapValues(m_properties);
  destroyMapValues(m_constants);
}

Array ExecutionContext::getUserFunctionsInfo() {
  // Return an array of all user-defined function names.  This method is used to
  // support get_defined_functions().
  return m_funcDict.getUserFunctions();
}

Array ExecutionContext::getClassesInfo() {
  // Return an array of all defined class names.  This method is used to
  // support get_declared_classes().
  Array a = Array::Create();
  for (DefinedClassMap::const_iterator it = m_definedClasses.begin();
       it != m_definedClasses.end(); ++it) {
    Class* class_ = it->second;
    if (!(class_->m_preClass->m_attrs & (AttrInterface | AttrTrait))) {
      a.append(*(String*)(&class_->m_preClass->m_name));
    }
  }
  return a;
}

Array ExecutionContext::getInterfacesInfo() {
  // Return an array of all defined interface names.  This method is used to
  // support get_declared_interfaces().
  Array a = Array::Create();
  for (DefinedClassMap::const_iterator it = m_definedClasses.begin();
       it != m_definedClasses.end(); ++it) {
    Class* class_ = it->second;
    if (class_->m_preClass->m_attrs & AttrInterface) {
      a.append(*(String*)(&class_->m_preClass->m_name));
    }
  }
  return a;
}

// Returns an array with all defined trait names.  This method is used to
// support get_declared_traits().
Array ExecutionContext::getTraitsInfo() {
  Array array = Array::Create();
  for (DefinedClassMap::const_iterator it = m_definedClasses.begin();
       it != m_definedClasses.end(); it++) {
    Class* cls = it->second;
    if (cls->m_preClass->m_attrs & AttrTrait) {
      array.append(*(String*)(&cls->m_preClass->m_name));
    }
  }
  return array;
}

Array ExecutionContext::getConstantsInfo() {
  // Return an array of all defined constant:value pairs.  This method is used
  // to support get_defined_constants().
  return Array(m_constants.copy());
}

const ClassInfo::MethodInfo* ExecutionContext::findFunctionInfo(CStrRef name) {
  StringIMap<SmartPtr<MethodInfoVM> >::iterator it =
    m_functionInfos.find(name);
  if (it == m_functionInfos.end()) {
    Func* func = lookupFunc(name.get());
    if (func == NULL || func->m_builtinFuncPtr) {
      // Fall back to the logic in ClassInfo::FindFunction() logic to deal
      // with builtin functions
      return NULL;
    }
    SmartPtr<MethodInfoVM> &m = m_functionInfos[name];
    m = new MethodInfoVM();
    func->getFuncInfo(m.get());
    return m.get();
  } else {
    return it->second.get();
  }
}

const ClassInfo* ExecutionContext::findClassInfo(CStrRef name) {
  if (name->empty()) return NULL;
  StringIMap<SmartPtr<ClassInfoVM> >::iterator it =
    m_classInfos.find(name);
  if (it == m_classInfos.end()) {
    Class* cls = lookupClass(name.get());
    if (cls == NULL || cls->m_isCppExtClass) {
      // Fall back to the logic in ClassInfo::FindClass() logic to deal
      // with builtin classes
      return NULL;
    }
    if (cls->m_preClass->m_attrs & (AttrInterface | AttrTrait)) {
      // If the specified name matches with something that is not formally
      // a class, return NULL
      return NULL;
    }
    SmartPtr<ClassInfoVM> &c = m_classInfos[name];
    c = new ClassInfoVM();
    cls->getClassInfo(c.get());
    return c.get();
  } else {
    return it->second.get();
  }
}

const ClassInfo* ExecutionContext::findInterfaceInfo(CStrRef name) {
  StringIMap<SmartPtr<ClassInfoVM> >::iterator it =
    m_interfaceInfos.find(name);
  if (it == m_interfaceInfos.end()) {
    Class* cls = lookupClass(name.get());
    if (cls == NULL || cls->m_isCppExtClass) {
      // Fall back to the logic in ClassInfo::FindInterface() logic to deal
      // with builtin interfaces
      return NULL;
    }
    if (!(cls->m_preClass->m_attrs & AttrInterface)) {
      // If the specified name matches with something that is not formally
      // an interface, return NULL
      return NULL;
    }
    SmartPtr<ClassInfoVM> &c = m_interfaceInfos[name];
    c = new ClassInfoVM();
    cls->getClassInfo(c.get());
    return c.get();
  } else {
    return it->second.get();
  }
}

const ClassInfo* ExecutionContext::findTraitInfo(CStrRef name) {
  StringIMap<SmartPtr<ClassInfoVM> >::iterator it = m_traitInfos.find(name);
  if (it != m_traitInfos.end()) {
    return it->second.get();
  }
  Class* cls = lookupClass(name.get());
  if (cls == NULL || cls->m_isCppExtClass) {
    return NULL;
  }
  if (!(cls->m_preClass->m_attrs & AttrTrait)) {
    return NULL;
  }
  SmartPtr<ClassInfoVM> &classInfo = m_traitInfos[name];
  classInfo = new ClassInfoVM();
  cls->getClassInfo(classInfo.get());
  return classInfo.get();
}

const ClassInfo::ConstantInfo* ExecutionContext::findConstantInfo(
    CStrRef name) {
  TypedValue* tv = m_constants.nvGet(name.get());
  if (tv == NULL) {
    return NULL;
  }
  ConstInfoMap::const_iterator it = m_constInfo.find(name.get());
  if (it != m_constInfo.end()) {
    return it->second;
  }
  StringData* key = StringData::GetStaticString(name.get());
  ClassInfo::ConstantInfo* ci = new ClassInfo::ConstantInfo();
  ci->name = *(const String*)&key;
  ci->valueLen = 0;
  ci->valueText = "";
  ci->setValue(tvAsCVarRef(tv));
  m_constInfo[key] = ci;
  return ci;
}

struct ResolveIncludeContext {
  String path; // translated path of the file
  struct stat* s; // stat for the file
};

static bool findFileWrapper(CStrRef file, void* ctx) {
  ResolveIncludeContext* context = (ResolveIncludeContext*)ctx;
  ASSERT(context->path.isNull());
  // TranslatePath() will canonicalize the path and also check
  // whether the file is in an allowed directory.
  String translatedPath = File::TranslatePath(file, false, true);
  if (file[0] != '/') {
    if (HPHP::Eval::FileRepository::findFile(translatedPath.data(),
                                             context->s)) {
      context->path = translatedPath;
      return true;
    }
    return false;
  }
  if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
    if (HPHP::Eval::FileRepository::findFile(translatedPath.data(),
                                             context->s)) {
      context->path = translatedPath;
      return true;
    }
  }
  string server_root = RuntimeOption::SourceRoot;
  if (server_root.empty()) {
    server_root = string(g_context->getCwd()->data());
    if (server_root.empty() || server_root[server_root.size() - 1] != '/') {
      server_root += "/";
    }
  }
  String rel_path(Util::relativePath(server_root, file.data()));
  translatedPath = File::TranslatePath(rel_path, false, true);
  if (HPHP::Eval::FileRepository::findFile(translatedPath.data(),
                                           context->s)) {
    context->path = translatedPath;
    return true;
  }
  return false;
}

HPHP::Eval::PhpFile* ExecutionContext::lookupPhpFile(StringData* path,
                                                     const char* currentDir,
                                                     bool& initial) {
  initial = true;
  struct stat s;
  String spath;
  // Resolve the include path
  {
    ResolveIncludeContext ctx;
    ctx.s = &s;
    resolve_include(path, currentDir, findFileWrapper,
                    (void*)&ctx);
    // If resolve_include() could not find the file, return NULL
    if (ctx.path.isNull()) {
      return NULL;
    }
    spath = ctx.path;
  }
  // Check if this file has already been included.
  EvaledFilesMap::const_iterator it = m_evaledFiles.find(spath.get());
  HPHP::Eval::PhpFile* efile = NULL;
  if (it != m_evaledFiles.end()) {
    // We found it! Return the unit.
    efile = it->second;
    initial = false;
    return efile;
  }
  // We didn't find it, so try the realpath.
  bool alreadyResolved = !RuntimeOption::CheckSymLink && (spath[0] == '/');
  bool hasRealpath = false;
  String rpath;
  if (!alreadyResolved) {
    char* rp = realpath(spath.data(), NULL);
    if (rp) {
      rpath = NEW(StringData)(rp, AttachString);
      if (!rpath.same(spath)) {
        hasRealpath = true;
        it = m_evaledFiles.find(rpath.get());
        if (it != m_evaledFiles.end()) {
          // We found it! Update the mapping for spath and
          // return the unit.
          efile = it->second;
          m_evaledFiles[spath.get()] = efile;
          spath.get()->incRefCount();
          efile->incRef();
          initial = false;
          return efile;
        }
      }
    }
  }
  // This file hasn't been included yet, so we need to parse the file
  efile = HPHP::Eval::FileRepository::checkoutFile(
    hasRealpath ? rpath.data() : spath.data(), s);
  if (efile) {
    // If parsing was successful, update the mappings for spath and
    // rpath (if it exists) and return the unit.
    m_evaledFiles[spath.get()] = efile;
    spath.get()->incRefCount();
    efile->incRef();
    if (hasRealpath) {
      m_evaledFiles[rpath.get()] = efile;
      rpath.get()->incRefCount();
      efile->incRef();
    }
  }
  return efile;
}

Unit* ExecutionContext::evalInclude(StringData* path,
                                    const StringData* curUnitFilePath,
                                    bool& initial) {
  using namespace boost;
  filesystem::path currentUnit(curUnitFilePath->data());
  filesystem::path currentDir(currentUnit.branch_path());
  HPHP::Eval::PhpFile* efile =
    lookupPhpFile(path, currentDir.string().c_str(), initial);
  if (efile) {
    return efile->unit();
  }
  return NULL;
}

void ExecutionContext::evalUnit(Unit* unit, PC& pc) {
  ActRec* ar = m_stack.allocA();
  ASSERT((uintptr_t)&ar->m_func < (uintptr_t)&ar->m_r);
  Func* func = unit->getMain();
  ASSERT(!func->isBuiltin());
  ar->m_func = func;
  ar->m_numArgs = 0;
  ASSERT(m_fp);
  ASSERT(!m_fp->hasInvName());
  if (m_fp->hasThis()) {
    ObjectData *this_ = m_fp->getThis();
    this_->incRefCount();
    ar->setThis(this_);
  } else if (m_fp->hasClass()) {
    ar->setClass(m_fp->getClass());
  } else {
    ar->setThis(NULL);
  }
  arSetSfp(ar, m_fp);
  if (m_fp->m_varEnv == NULL) {
    m_fp->m_varEnv = new VarEnv();
    m_fp->m_varEnv->lazyAttach(m_fp);
  }
  ar->m_varEnv = m_fp->m_varEnv;
  ar->m_soff = uintptr_t(m_fp->m_func->m_unit->offsetOf(pc) -
                         m_fp->m_func->m_base);
  ar->m_savedRip = (uintptr_t)m_transl->getRetFromInterpretedFrame();
  pc = func->getEntry();
  m_fp = ar;
  pushLocalsAndIterators(func);
  ar->m_varEnv->attach(ar);
  SYNC(); // mergeUnit can reenter, e.g., via autoload.
  mergeUnit(unit);
}

/*
 * Helper for function entry, including pseudo-main entry.
 */
inline void
ExecutionContext::pushLocalsAndIterators(const Func* func,
                                         int nparams /*= 0*/) {
  // Push locals.
  for (int i = nparams; i < func->m_numLocals; i++) {
    m_stack.pushUninit();
  }
  // Push iterators.
  for (int i = 0; i < func->m_numIterators; i++) {
    m_stack.allocI();
  }
}

void ExecutionContext::destructObjects() {
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
    while (!m_liveBCObjs.empty()) {
      ObjectData* o = *m_liveBCObjs.begin();
      Instance* instance = static_cast<Instance*>(o);
      ASSERT(o->isInstance());
      instance->destruct(); // Let the instance remove the node.
    }
    m_liveBCObjs.clear();
  }
}

void ExecutionContext::evalPHPDebugger(TypedValue* retval, StringData *code,
                                       int frame) {
  ASSERT(retval);
  // The code has "<?php" prepended already
  Unit* unit = compile_string(code->data(), code->size(), "");
  if (unit == NULL) {
    raise_error("Syntax error");
    tvWriteNull(retval);
    return;
  }

  VarEnv *varEnv = NULL;
  ActRec *fp = m_fp;
  ActRec *cfpSave = NULL;
  if (fp) {
    ASSERT(!g_context->m_varEnvs.empty());
    std::list<HPHP::VM::VarEnv*>::iterator it = g_context->m_varEnvs.end();
    for (; frame > 0; --frame) {
      if (fp->m_varEnv != NULL) {
        if (it == g_context->m_varEnvs.end() || *it != fp->m_varEnv) {
          --it;
        }
        ASSERT(*it == fp->m_varEnv);
      }
      ActRec *prevFp = getPrevVMState(fp);
      fp = prevFp;
    }
    if (fp->m_varEnv == NULL) {
      fp->m_varEnv = new VarEnv(false, true);
      fp->m_varEnv->lazyAttach(fp);
      g_context->m_varEnvs.insert(it, fp->m_varEnv);
    }
    varEnv = fp->m_varEnv;
    cfpSave = varEnv->getCfp();
  }
  ObjectData *this_ = NULL;
  Class *cls = NULL;
  if (fp) {
    if (fp->hasThis()) {
      this_ = fp->getThis();
    } else if (fp->hasClass()) {
      cls = fp->getClass();
    }
  }

  const static StaticString s_cppException("Hit an exception");
  const static StaticString s_phpException("Hit a php exception");
  const static StaticString s_exit("Hit exit");
  const static StaticString s_fatal("Hit fatal");
  try {
    invokeFunc(retval, unit->getMain(), Array::Create(), this_, cls,
               varEnv, NULL, unit);
  } catch (FatalErrorException &e) {
    g_context->write(s_fatal);
    g_context->write(" : ");
    g_context->write(e.getMessage().c_str());
  } catch (Exception &e) {
    g_context->write(s_cppException.data());
    g_context->write(" : ");
    g_context->write(e.getMessage().c_str());
  } catch (...) {
    Fault fault = m_faults.back();
    switch (fault.m_faultType) {
      case Fault::KindOfThrow:
        g_context->write(s_phpException.data());
        break;
      case Fault::KindOfExit:
        g_context->write(s_exit.data());
        break;
      case Fault::KindOfFatal:
        g_context->write(s_fatal.data());
        break;
      default:
        g_context->write(s_cppException.data());
    }
  }

  if (varEnv) {
    // Set the Cfp back
    varEnv->setCfp(cfpSave);
  }

  m_evaledUnits.push_back(unit);
}

void ExecutionContext::enterDebuggerDummyEnv() {
  static Unit* s_debuggerDummy = NULL;
  if (!s_debuggerDummy) {
    s_debuggerDummy = compile_string("<?php?>", 7, "");
  }
  ASSERT(g_context->m_varEnvs.empty());
  VarEnv* varEnv = new VarEnv(true);
  ActRec* ar = m_stack.allocA();
  ar->m_func = s_debuggerDummy->getMain();
  ar->setThis(NULL);
  ar->m_soff = 0;
  ar->m_savedRbp = 0;
  ar->setVarEnv(varEnv);
  varEnv->attach(ar);
  ar->m_savedRip = (uintptr_t)m_transl->getCallToExit();
  m_fp = ar;
  m_pc = s_debuggerDummy->m_bc;
  m_firstAR = ar;
  m_nestedVMMap[ar] = m_nestedVMs.size() - 1;
  m_halted = false;
}

void ExecutionContext::exitDebuggerDummyEnv() {
  ASSERT(!g_context->m_varEnvs.empty());
  ASSERT(g_context->m_varEnvs.front() == g_context->m_varEnvs.back());
  VarEnv* varEnv = g_context->m_varEnvs.front();
  varEnv->detach(m_fp);
  delete varEnv;
}

#define LOOKUP_NAME(name, key) (name) = prepareKey(key)
#define LOOKUP_VAR(name, key, val) do {                                       \
  LOOKUP_NAME(name, key);                                                     \
  const Func* func = m_fp->m_func;                                            \
  Id id;                                                                      \
  if (mapGet(func->m_name2pind, name, &id)) {                                 \
    (val) = frame_local(m_fp, id);                                            \
  } else {                                                                    \
    ASSERT(!m_fp->hasInvName());                                              \
    if (m_fp->m_varEnv != NULL) {                                             \
      (val) = m_fp->m_varEnv->lookup(name);                                   \
    } else {                                                                  \
      (val) = NULL;                                                           \
    }                                                                         \
  }                                                                           \
} while (0)
#define LOOKUPD_VAR(name, key, val) do {                                      \
  LOOKUP_NAME(name, key);                                                     \
  const Func* func = m_fp->m_func;                                            \
  Id id;                                                                      \
  if (mapGet(func->m_name2pind, name, &id)) {                                 \
    (val) = frame_local(m_fp, id);                                            \
  } else {                                                                    \
    ASSERT(!m_fp->hasInvName());                                              \
    if (m_fp->m_varEnv == NULL) {                                             \
      m_fp->m_varEnv = new VarEnv();                                          \
      m_fp->m_varEnv->lazyAttach(m_fp);                                       \
    }                                                                         \
    (val) = m_fp->m_varEnv->lookup(name);                                     \
    if ((val) == NULL) {                                                      \
      TypedValue tv;                                                          \
      TV_WRITE_NULL(&tv);                                                     \
      m_fp->m_varEnv->set(name, &tv);                                         \
      (val) = m_fp->m_varEnv->lookup(name);                                   \
    }                                                                         \
  }                                                                           \
} while (0)
#define LOOKUP_GBL(name, key, val) do {                                       \
  LOOKUP_NAME(name, key);                                                     \
  ASSERT(!g_context->m_varEnvs.empty());                                      \
  VarEnv* varEnv = g_context->m_varEnvs.front();                              \
  ASSERT(varEnv != NULL);                                                     \
  (val) = varEnv->lookup(name);                                               \
} while (0)
#define LOOKUPD_GBL(name, key, val) do {                                      \
  LOOKUP_NAME(name, key);                                                     \
  ASSERT(!g_context->m_varEnvs.empty());                                      \
  VarEnv* varEnv = g_context->m_varEnvs.front();                              \
  ASSERT(varEnv != NULL);                                                     \
  (val) = varEnv->lookup(name);                                               \
  if ((val) == NULL) {                                                        \
    TypedValue tv;                                                            \
    TV_WRITE_NULL(&tv);                                                       \
    varEnv->set(name, &tv);                                                   \
    (val) = varEnv->lookup(name);                                             \
  }                                                                           \
} while (0)

#define LOOKUPD_STATIC(name, val, inited) do {                                \
  HphpArray* map = m_fp->m_func->m_needsStaticLocalCtx ?                      \
    m_fp->m_staticLocalCtx : g_context->m_staticVars;                         \
  ASSERT(map != NULL);                                                        \
  (val) = map->nvGet(name);                                                   \
  if ((val) == NULL) {                                                        \
    TypedValue tv;                                                            \
    TV_WRITE_UNINIT(&tv);                                                     \
    map->nvSet(name, &tv, false);                                             \
    (val) = map->nvGet(name);                                                 \
    (inited) = false;                                                         \
  } else {                                                                    \
    (inited) = true;                                                          \
  }                                                                           \
} while (0)

#define LOOKUP_SPROP(clsRef, name, key, val, visible, accessible) do {        \
  ASSERT(clsRef->m_type == KindOfClass);                                      \
  LOOKUP_NAME(name, key);                                                     \
  PreClass* ctx = arGetContextPreClass(m_fp);                                 \
  val = clsRef->m_data.pcls->getSProp(ctx, name, visible, accessible);        \
} while (0)

static inline void ratchetRefs(TypedValue*& result, TypedValue& tvRef,
                               TypedValue& tvRef2) {

  // Due to complications associated with ArrayAccess, it is possible to acquire
  // a reference as a side effect of vector operation processing. Such a
  // reference must be retained until after the next iteration is complete.
  // Therefore, move the reference from tvRef to tvRef2, so that the reference
  // will be released one iteration later. But only do this if tvRef was used in
  // this iteration, otherwise we may wipe out the last reference to something
  // that we need to stay alive until the next iteration.
  if (tvRef.m_type != KindOfUninit) {
    if (IS_REFCOUNTED_TYPE(tvRef2.m_type)) {
      tvDecRef(&tvRef2);
      tvWriteUninit(&tvRef2);
    }

    memcpy(&tvRef2, &tvRef, sizeof(TypedValue));
    tvWriteUninit(&tvRef);
    // Update result to point to relocated reference. This can be done
    // unconditionally here because we maintain the invariant throughout that
    // either tvRef is KindOfUninit, or tvRef contains a valid object that
    // result points to.
    ASSERT(result == &tvRef);
    result = &tvRef2;
  }
}

#define DECLARE_GETHELPER_ARGS                                                \
  unsigned ndiscard;                                                          \
  TypedValue* tvL;                                                            \
  TypedValue* base;                                                           \
  bool baseStrOff = false;                                                    \
  TypedValue tvScratch;                                                       \
  TypedValue tvRef;                                                           \
  TypedValue tvRef2;
#define GETHELPERPRE_ARGS                                                     \
  pc, ndiscard, tvL, base, baseStrOff, tvScratch, tvRef, tvRef2
// The following arguments are outputs:
// pc:         bytecode instruction after the vector instruction
// ndiscard:   number of stack elements to discard
// tvL:        stack cell corresponding to the first element of the vector
// base:       ultimate result of the vector-get
// baseStrOff: StrOff flag associated with base
// tvScratch:  temporary result storage
// tvRef:      temporary result storage
// tvRef2:     temporary result storage
//
// Process all but mleave of the members in the vector.  If saveResult is true,
// then upon completion of getHelperPre(), tvScratch contains a reference to
// the result (a duplicate of what base refers to).  getHelperPost<true>(...)
// then saves the result to its final location.
template <bool warn, bool saveResult, unsigned mleave>
inline void ALWAYS_INLINE ExecutionContext::getHelperPre(PC& pc,
                                                         unsigned& ndiscard,
                                                         TypedValue*& tvL,
                                                         TypedValue*& base,
                                                         bool& baseStrOff,
                                                         TypedValue& tvScratch,
                                                         TypedValue& tvRef,
                                                         TypedValue& tvRef2) {
  // The caller is responsible for moving pc to point to the vector immediate
  // before calling getHelperPre().
  DECODE(int32_t, veclen);
  ASSERT(veclen > 0);
  // Members can only be ME or MP, so the depth of the location is directly
  // computable from the vector length.
  int32_t depth = veclen - 1;
  ndiscard = unsigned(depth);
  DECODE(unsigned char, lcode);
  // Get the location.
  tvL = m_stack.indTV(depth);
  TypedValue* loc = NULL;
  TypedValue dummy;
  PreClass* ctx = arGetContextPreClass(m_fp);
  switch (LocationCode(lcode)) {
  case LH: {
    loc = tvL->m_data.ptv;
    break;
  }
  case LN: {
    StringData* name;
    TypedValue* fr = NULL;
    LOOKUP_VAR(name, tvL, fr);
    if (fr == NULL) {
      if (warn) {
        raise_notice("Undefined variable: %s", name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
    break;
  }
  case LG: {
    StringData* name;
    TypedValue* fr = NULL;
    LOOKUP_GBL(name, tvL, fr);
    if (fr == NULL) {
      if (warn) {
        raise_notice("Undefined variable: %s", name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
    break;
  }
  case LS: {
    ++ndiscard; // Account for location being two stack elements.
    bool visible, accessible;
    TypedValue* tvL2 = tvL;
    tvL = m_stack.indTV(depth+1);
    ASSERT(tvL->m_type == KindOfClass);
    const Class* class_ = tvL->m_data.pcls;
    StringData* name;
    tvL2 = tvToKeyValue(tvL2);
    LOOKUP_NAME(name, tvL2);
    loc = class_->getSProp(ctx, name, visible, accessible);
    if (!(visible && accessible)) {
      raise_error("Invalid static property access: %s::%s",
                  class_->m_preClass->m_name->data(),
                  name->data());
    }
    LITSTR_DECREF(name);
    break;
  }
  case LC:
  case LR: {
    loc = tvL;
    break;
  }
  default: ASSERT(false);
  }
  base = loc;
  tvWriteUninit(&tvScratch);
  tvWriteUninit(&tvRef);
  tvWriteUninit(&tvRef2);
  // Iterate through the members.
  for (--depth; depth >= int(mleave); --depth) {
    DECODE(unsigned char, mcode);
    TypedValue* result;
    TypedValue* tvV = tvToKeyValue(m_stack.indTV(depth));
    switch (MemberCode(mcode)) {
    case ME: {
      result = Elem<warn>(tvScratch, tvRef, base, baseStrOff, tvV);
      break;
    }
    case MP: {
      result = prop<warn, false, false>(tvScratch, ctx, base, tvV);
      break;
    }
    default: {
      ASSERT(false);
      result = NULL; // Silence compiler warning.
    }
    }
    ASSERT(result != NULL);
    ratchetRefs(result, tvRef, tvRef2);
    base = result;
  }
  ASSERT(depth == int(mleave) - 1);
  if (saveResult) {
    // If requested, save a copy of the result.  If base already points to
    // tvScratch, no reference counting is necessary, because (with the
    // exception of the following block), tvScratch is never populated such
    // that it owns a reference that must be accounted for.
    if (base != &tvScratch) {
      // Acquire a reference to the result via tvDup(); base points to the
      // result but does not own a reference.
      tvDup(base, &tvScratch);
    }
  }
}

#define GETHELPERPOST_ARGS ndiscard, tvL, tvScratch, tvRef, tvRef2
template <bool saveResult>
inline void ALWAYS_INLINE ExecutionContext::getHelperPost(unsigned ndiscard,
                                                          TypedValue* tvL,
                                                          TypedValue& tvScratch,
                                                          TypedValue& tvRef,
                                                          TypedValue& tvRef2) {
  // Clean up the first ndiscard+1 elements on the stack (tvL plus members),
  // but don't discard tvL, since it will be overwritten with the final result
  // of the current instruction.
  for (unsigned depth = 0; depth <= ndiscard; ++depth) {
    TypedValue* tv = m_stack.indTV(depth);
    tvRefcountedDecRef(tv);
  }
  m_stack.ndiscard(ndiscard);
  tvRefcountedDecRef(&tvRef);
  tvRefcountedDecRef(&tvRef2);

  if (saveResult) {
    memcpy((void*)tvL, (void*)&tvScratch, sizeof(TypedValue));
  }
}

#define GETHELPER_ARGS GETHELPERPRE_ARGS
inline void ALWAYS_INLINE ExecutionContext::getHelper(PC& pc,
                                                      unsigned& ndiscard,
                                                      TypedValue*& tvL,
                                                      TypedValue*& base,
                                                      bool& baseStrOff,
                                                      TypedValue& tvScratch,
                                                      TypedValue& tvRef,
                                                      TypedValue& tvRef2) {
  getHelperPre<true, true, 0>(GETHELPERPRE_ARGS);
  getHelperPost<true>(GETHELPERPOST_ARGS);
}

#define DECLARE_SETHELPER_ARGS                                                \
  unsigned ndiscard;                                                          \
  TypedValue* tvL;                                                            \
  PC vecp;                                                                    \
  TypedValue* base;                                                           \
  bool baseStrOff = false;                                                    \
  TypedValue tvScratch;                                                       \
  TypedValue tvRef;                                                           \
  TypedValue tvRef2;
#define SETHELPERPRE_ARGS                                                     \
  pc, ndiscard, tvL, vecp, base, baseStrOff, tvScratch, tvRef, tvRef2
// The following arguments are outputs:
// pc:         bytecode instruction after the vector instruction
// ndiscard:   number of stack elements to discard
// tvL:        stack cell corresponding to the first element of the vector
// base:       ultimate result of the vector-get
// baseStrOff: StrOff flag associated with base
// tvScratch:  temporary result storage
// tvRef:      temporary result storage
// tvRef2:     temporary result storage
//
// Process all but mleave of the members in the vector.
template <bool warn, bool define, bool unset, unsigned mdepth, unsigned mleave>
inline bool ALWAYS_INLINE ExecutionContext::setHelperPre(PC& pc,
                                                         unsigned& ndiscard,
                                                         TypedValue*& tvL,
                                                         PC& vecp,
                                                         TypedValue*& base,
                                                         bool& baseStrOff,
                                                         TypedValue& tvScratch,
                                                         TypedValue& tvRef,
                                                         TypedValue& tvRef2) {
  DECODE(int32_t, veclen);
  ASSERT(veclen > 0);
  // Compute the depth of the location to be stored to.
  unsigned depth = mdepth;
  DECODE(unsigned char, lcode);
  for (int32_t vecind = 1; vecind < veclen; ++vecind) {
    DECODE(unsigned char, mcode);
    switch (MemberCode(mcode)) {
    case ME:
    case MP: ++depth; break;
    case MW: ASSERT(define); break;
    default: ASSERT(false);
    }
  }
  ndiscard = depth + 1 - mdepth;
  // Get the location.
  tvL = m_stack.indTV(depth);
  TypedValue* loc = NULL;
  TypedValue dummy;
  PreClass* ctx = arGetContextPreClass(m_fp);
  switch (LocationCode(lcode)) {
  case LH: loc = tvL->m_data.ptv; break;
  case LN: {
    StringData* name;
    TypedValue* fr = NULL;
    if (define) {
      LOOKUPD_VAR(name, tvL, fr);
    } else {
      LOOKUP_VAR(name, tvL, fr);
    }
    if (fr == NULL) {
      if (warn) {
        raise_notice("Undefined variable: %s", name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
    break;
  }
  case LG: {
    StringData* name;
    TypedValue* fr = NULL;
    if (define) {
      LOOKUPD_GBL(name, tvL, fr);
    } else {
      LOOKUP_GBL(name, tvL, fr);
    }
    if (fr == NULL) {
      if (warn) {
        raise_notice("Undefined variable: %s", name->data());
      }
      TV_WRITE_NULL(&dummy);
      loc = &dummy;
    } else {
      loc = fr;
    }
    LITSTR_DECREF(name);
    break;
  }
  case LS: {
    ++ndiscard; // Account for location being two stack elements.
    bool visible, accessible;
    TypedValue* tvL2 = tvL;
    tvL = m_stack.indTV(depth+1);
    ASSERT(tvL->m_type == KindOfClass);
    const Class* class_ = tvL->m_data.pcls;
    tvL2 = m_stack.indTV(depth);
    StringData* name;
    LOOKUP_NAME(name, tvL2);
    loc = class_->getSProp(ctx, name, visible, accessible);
    if (!(visible && accessible)) {
      raise_error("Invalid static property access: %s::%s",
                  class_->m_preClass->m_name->data(),
                  name->data());
    }
    LITSTR_DECREF(name);
    break;
  }
  case LC:
  case LR: {
    loc = tvL;
    break;
  }
  default: ASSERT(false);
  }
  base = loc;
  tvWriteUninit(&tvScratch);
  tvWriteUninit(&tvRef);
  tvWriteUninit(&tvRef2);
  // Iterate through all but mleave of the members.
  for (vecp = pc - (veclen-1); vecp < pc-mleave; ++vecp) {
    TypedValue* result;
    switch (MemberCode(*vecp)) {
    case ME: {
      --depth;
      TypedValue* tvV = tvToKeyValue(m_stack.indTV(depth));
      if (unset) {
        ASSERT(vecp < pc-mleave);
        result = ElemU(tvScratch, tvRef, base, tvV);
      } else if (define) {
        ASSERT(vecp < pc-mleave);
        result = ElemD<warn>(tvScratch, tvRef, base, tvV);
      } else {
        result = Elem<warn>(tvScratch, tvRef, base, baseStrOff, tvV);
      }
      break;
    }
    case MW: {
      ASSERT(define);
      ASSERT(vecp < pc-mleave);
      result = NewElem(tvScratch, tvRef, base);
      break;
    }
    case MP: {
      --depth;
      TypedValue* tvV = tvToKeyValue(m_stack.indTV(depth));
      result = prop<warn, define, unset>(tvScratch, ctx, base, tvV);
      break;
    }
    default: {
      ASSERT(false);
      result = NULL; // Silence compiler warning.
    }
    }
    ASSERT(result != NULL);
    ratchetRefs(result, tvRef, tvRef2);
    // Check whether an error occurred (i.e. no result was set).
    if (result == &tvScratch && result->m_type == KindOfUninit) {
      return true;
    }
    base = result;
  }
  ASSERT(depth == mdepth + mleave || MemberCode(*vecp) == MW);
  return false;
}

#define SETHELPERPOST_ARGS ndiscard, tvL, tvRef, tvRef2
template <unsigned mdepth>
inline void ALWAYS_INLINE ExecutionContext::setHelperPost(unsigned ndiscard,
                                                          TypedValue* tvL,
                                                          TypedValue& tvRef,
                                                          TypedValue& tvRef2) {
  // Clean up the stack.
  for (unsigned depth = mdepth; depth-mdepth < ndiscard; ++depth) {
    TypedValue* tv = m_stack.indTV(depth);
    tvRefcountedDecRef(tv);
  }
  if (mdepth > 0) {
    TypedValue* tv1 = m_stack.topTV();
    memcpy((void*)tvL, (void*)tv1, sizeof(TypedValue));
  }
  m_stack.ndiscard(ndiscard);
  tvRefcountedDecRef(&tvRef);
  tvRefcountedDecRef(&tvRef2);
}

inline void ALWAYS_INLINE ExecutionContext::iopLowInvalid(PC& pc) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

inline void ALWAYS_INLINE ExecutionContext::iopNop(PC& pc) {
  NEXT();
}

inline void ALWAYS_INLINE ExecutionContext::iopPopC(PC& pc) {
  NEXT();
  m_stack.popC();
}

inline void ALWAYS_INLINE ExecutionContext::iopPopV(PC& pc) {
  NEXT();
  m_stack.popV();
}

inline void ALWAYS_INLINE ExecutionContext::iopPopR(PC& pc) {
  NEXT();
  if (m_stack.topTV()->m_type != KindOfVariant) {
    m_stack.popC();
  } else {
    m_stack.popV();
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopDup(PC& pc) {
  NEXT();
  m_stack.dup();
}

inline void ALWAYS_INLINE ExecutionContext::iopBox(PC& pc) {
  NEXT();
  m_stack.box();
}

inline void ALWAYS_INLINE ExecutionContext::iopUnbox(PC& pc) {
  NEXT();
  m_stack.unbox();
}

inline void ALWAYS_INLINE ExecutionContext::iopBoxR(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  if (tv->m_type != KindOfVariant) {
    tvBox(tv);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopUnboxR(PC& pc) {
  NEXT();
  if (m_stack.topTV()->m_type == KindOfVariant) {
    m_stack.unbox();
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopNull(PC& pc) {
  NEXT();
  m_stack.pushNull();
}

inline void ALWAYS_INLINE ExecutionContext::iopTrue(PC& pc) {
  NEXT();
  m_stack.pushTrue();
}

inline void ALWAYS_INLINE ExecutionContext::iopFalse(PC& pc) {
  NEXT();
  m_stack.pushFalse();
}

inline void ALWAYS_INLINE ExecutionContext::iopInt(PC& pc) {
  NEXT();
  DECODE(int64, i);
  m_stack.pushInt(i);
}

inline void ALWAYS_INLINE ExecutionContext::iopDouble(PC& pc) {
  NEXT();
  DECODE(double, d);
  m_stack.pushDouble(d);
}

inline void ALWAYS_INLINE ExecutionContext::iopString(PC& pc) {
  NEXT();
  DECODE(Id, id);
  StringData* s = m_fp->m_func->m_unit->lookupLitstrId(id);
  m_stack.pushStaticString(s);
}

inline void ALWAYS_INLINE ExecutionContext::iopArray(PC& pc) {
  NEXT();
  DECODE(Id, id);
  ArrayData* a = m_fp->m_func->m_unit->lookupArrayId(id);
  m_stack.pushStaticArray(a);
}

inline void ALWAYS_INLINE ExecutionContext::iopNewArray(PC& pc) {
  NEXT();
  // Clever sizing avoids extra work in HphpArray construction.
  ArrayData* arr = NEW(HphpArray)(size_t(3U) << (HphpArray::MinLgTableSize-2));
  m_stack.pushArray(arr);
}

inline void ALWAYS_INLINE ExecutionContext::iopAddElemC(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type != KindOfArray) {
    raise_error("AddElemC: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    tvCellAsVariant(c3).asArrRef().set(c2->m_data.num, tvAsCVarRef(c1));
  } else {
    tvCellAsVariant(c3).asArrRef().set(tvAsCVarRef(c2), tvAsCVarRef(c1));
  }
  m_stack.popC();
  m_stack.popC();
}

inline void ALWAYS_INLINE ExecutionContext::iopAddElemV(PC& pc) {
  NEXT();
  Var* v1 = m_stack.topV();
  Cell* c2 = m_stack.indC(1);
  Cell* c3 = m_stack.indC(2);
  if (c3->m_type != KindOfArray) {
    raise_error("AddElemC: $3 must be an array");
  }
  if (c2->m_type == KindOfInt64) {
    tvCellAsVariant(c3).asArrRef().set(c2->m_data.num, ref(tvAsCVarRef(v1)));
  } else {
    tvCellAsVariant(c3).asArrRef().set(tvAsCVarRef(c2), ref(tvAsCVarRef(v1)));
  }
  m_stack.popV();
  m_stack.popC();
}

inline void ALWAYS_INLINE ExecutionContext::iopAddNewElemC(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  tvCellAsVariant(c2).asArrRef().append(tvAsCVarRef(c1));
  m_stack.popC();
}

inline void ALWAYS_INLINE ExecutionContext::iopAddNewElemV(PC& pc) {
  NEXT();
  Var* v1 = m_stack.topV();
  Cell* c2 = m_stack.indC(1);
  if (c2->m_type != KindOfArray) {
    raise_error("AddNewElemC: $2 must be an array");
  }
  tvCellAsVariant(c2).asArrRef().append(ref(tvAsCVarRef(v1)));
  m_stack.popV();
}

inline void ALWAYS_INLINE ExecutionContext::iopCns(PC& pc) {
  NEXT();
  DECODE(Id, id);
  StringData* s = m_fp->m_func->m_unit->lookupLitstrId(id);
  TypedValue* cns = getCns(s);
  if (cns != NULL) {
    Cell* c1 = m_stack.allocC();
    TV_READ_CELL(cns, c1);
  } else {
    raise_notice("Undefined constant: %s", s->data());
    m_stack.pushStaticString(s);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopClsCns(PC& pc) {
  NEXT();
  DECODE(Id, id);
  TypedValue* tv = m_stack.topTV();
  ASSERT(tv->m_type == KindOfClass);
  Class* class_ = tv->m_data.pcls;
  ASSERT(class_ != NULL);
  const StringData* clsCnsName = m_fp->m_func->m_unit->lookupLitstrId(id);
  TypedValue* clsCns = class_->clsCnsGet(clsCnsName);
  if (clsCns == NULL) {
    raise_error("Undefined class constant: %s::%s",
                class_->m_preClass->m_name->data(), clsCnsName->data());
  }
  TV_READ_CELL(clsCns, tv);
}

inline void ALWAYS_INLINE ExecutionContext::iopClsCnsD(PC& pc) {
  NEXT();
  DECODE(Id, cnsId);
  DECODE(Id, classId);
  const StringData* className = m_fp->m_func->m_unit->lookupLitstrId(classId);
  Class* class_ = loadClass(className);
  if (class_ == NULL) {
    raise_error("Class undefined: %s", className->data());
  }
  const StringData* clsCnsName = m_fp->m_func->m_unit->lookupLitstrId(cnsId);
  TypedValue* clsCns = class_->clsCnsGet(clsCnsName);
  if (clsCns == NULL) {
    raise_error("Undefined class constant: %s::%s",
                className->data(), clsCnsName->data());
  }
  Cell* c1 = m_stack.allocC();
  TV_READ_CELL(clsCns, c1);
}

inline void ALWAYS_INLINE ExecutionContext::iopConcat(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  Cell* c2 = m_stack.indC(1);
  if (IS_STRING_TYPE(c1->m_type) && IS_STRING_TYPE(c2->m_type)) {
    tvCellAsVariant(c2) = concat(tvCellAsVariant(c2), tvCellAsCVarRef(c1));
  } else {
    tvCellAsVariant(c2) = concat(tvCellAsVariant(c2).toString(),
                                 tvCellAsCVarRef(c1).toString());
  }
  ASSERT(c2->m_data.ptv->_count > 0);
  m_stack.popC();
}

#define MATHOP(OP, VOP) do {                                                  \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64) {               \
    int64 a = c2->m_data.num;                                                 \
    int64 b = c1->m_data.num;                                                 \
    MATHOP_DIVCHECK(0)                                                        \
    c2->m_data.num = a OP b;                                                  \
    m_stack.popX();                                                           \
  }                                                                           \
  MATHOP_DOUBLE(OP)                                                           \
  else {                                                                      \
    tvCellAsVariant(c2) = VOP(tvCellAsVariant(c2), tvCellAsCVarRef(c1));      \
    m_stack.popC();                                                           \
  }                                                                           \
} while (0)
#define MATHOP_DOUBLE(OP)                                                     \
  else if (c2->m_type == KindOfDouble                                         \
             && c1->m_type == KindOfDouble) {                                 \
    double a = c2->m_data.dbl;                                                \
    double b = c1->m_data.dbl;                                                \
    MATHOP_DIVCHECK(0.0)                                                      \
    c2->m_data.dbl = a OP b;                                                  \
    m_stack.popX();                                                           \
  }
#define MATHOP_DIVCHECK(x)
inline void ALWAYS_INLINE ExecutionContext::iopAdd(PC& pc) {
  MATHOP(+, plus);
}

inline void ALWAYS_INLINE ExecutionContext::iopSub(PC& pc) {
  MATHOP(-, minus);
}

inline void ALWAYS_INLINE ExecutionContext::iopMul(PC& pc) {
  MATHOP(*, multiply);
}
#undef MATHOP_DIVCHECK

#define MATHOP_DIVCHECK(x)                                                    \
    if (b == x) {                                                             \
      raise_warning("Division by zero");                                      \
      c2->m_data.num = 0;                                                     \
      c2->m_type = KindOfBoolean;                                             \
    } else
inline void ALWAYS_INLINE ExecutionContext::iopDiv(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();  // denominator
  Cell* c2 = m_stack.indC(1); // numerator
  // Special handling for evenly divisible ints
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64
      && c1->m_data.num != 0 && c2->m_data.num % c1->m_data.num == 0) {
    int64 b = c1->m_data.num;
    MATHOP_DIVCHECK(0)
    c2->m_data.num /= b;
    m_stack.popX();
  }
  MATHOP_DOUBLE(/)
  else {
    tvCellAsVariant(c2) = divide(tvCellAsVariant(c2), tvCellAsCVarRef(c1));
    m_stack.popC();
  }
}
#undef MATHOP_DOUBLE

#define MATHOP_DOUBLE(OP)
inline void ALWAYS_INLINE ExecutionContext::iopMod(PC& pc) {
  MATHOP(%, modulo);
}
#undef MATHOP_DOUBLE
#undef MATHOP_DIVCHECK

#define LOGICOP(OP) do {                                                      \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  {                                                                           \
    tvCellAsVariant(c2) =                                                     \
      (bool)(bool(tvCellAsVariant(c2)) OP bool(tvCellAsVariant(c1)));         \
  }                                                                           \
  m_stack.popC();                                                             \
} while (0)
inline void ALWAYS_INLINE ExecutionContext::iopAnd(PC& pc) {
  LOGICOP(&&);
}

inline void ALWAYS_INLINE ExecutionContext::iopOr(PC& pc) {
  LOGICOP(||);
}

inline void ALWAYS_INLINE ExecutionContext::iopXor(PC& pc) {
  LOGICOP(^);
}
#undef LOGICOP

inline void ALWAYS_INLINE ExecutionContext::iopNot(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCellAsVariant(c1) = !bool(tvCellAsVariant(c1));
}

#define CMPOP(OP, VOP) do {                                                   \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64) {               \
    int64 a = c2->m_data.num;                                                 \
    int64 b = c1->m_data.num;                                                 \
    c2->m_data.num = (a OP b);                                                \
    c2->m_type = KindOfBoolean;                                               \
    m_stack.popX();                                                           \
  } else {                                                                    \
    int64 result = VOP(tvCellAsVariant(c2), tvCellAsCVarRef(c1));             \
    tvRefcountedDecRefCell(c2);                                               \
    c2->m_data.num = result;                                                  \
    c2->_count = 0;                                                           \
    c2->m_type = KindOfBoolean;                                               \
    m_stack.popC();                                                           \
  }                                                                           \
} while (0)
inline void ALWAYS_INLINE ExecutionContext::iopSame(PC& pc) {
  CMPOP(==, same);
}

inline void ALWAYS_INLINE ExecutionContext::iopNSame(PC& pc) {
  CMPOP(!=, !same);
}

inline void ALWAYS_INLINE ExecutionContext::iopEq(PC& pc) {
  CMPOP(==, equal);
}

inline void ALWAYS_INLINE ExecutionContext::iopNeq(PC& pc) {
  CMPOP(!=, !equal);
}

inline void ALWAYS_INLINE ExecutionContext::iopLt(PC& pc) {
  CMPOP(<, less);
}

inline void ALWAYS_INLINE ExecutionContext::iopLte(PC& pc) {
  CMPOP(<=, not_more);
}

inline void ALWAYS_INLINE ExecutionContext::iopGt(PC& pc) {
  CMPOP(>, more);
}

inline void ALWAYS_INLINE ExecutionContext::iopGte(PC& pc) {
  CMPOP(>=, not_less);
}
#undef CMPOP

#define MATHOP_DOUBLE(OP)
#define MATHOP_DIVCHECK(x)
inline void ALWAYS_INLINE ExecutionContext::iopBitAnd(PC& pc) {
  MATHOP(&, bitwise_and);
}

inline void ALWAYS_INLINE ExecutionContext::iopBitOr(PC& pc) {
  MATHOP(|, bitwise_or);
}

inline void ALWAYS_INLINE ExecutionContext::iopBitXor(PC& pc) {
  MATHOP(^, bitwise_xor);
}
#undef MATHOP
#undef MATHOP_DOUBLE
#undef MATHOP_DIVCHECK

inline void ALWAYS_INLINE ExecutionContext::iopBitNot(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  if (LIKELY(c1->m_type == KindOfInt64)) {
    c1->m_data.num = ~c1->m_data.num;
  } else if (c1->m_type == KindOfDouble) {
    c1->m_type = KindOfInt64;
    c1->m_data.num = ~int64(c1->m_data.dbl);
  } else if (IS_STRING_TYPE(c1->m_type)) {
    tvCellAsVariant(c1) = ~tvCellAsVariant(c1);
  } else if (c1->m_type == KindOfInt32) {
    // Separate from KindOfInt64 due to the infrequency of KindOfInt32.
    c1->m_data.num = ~c1->m_data.num;
  } else {
    raise_error("Unsupported operand type for ~");
  }
}

#define SHIFTOP(OP) do {                                                      \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  Cell* c2 = m_stack.indC(1);                                                 \
  if (c2->m_type == KindOfInt64 && c1->m_type == KindOfInt64) {               \
    int64 a = c2->m_data.num;                                                 \
    int64 b = c1->m_data.num;                                                 \
    c2->m_data.num = a OP b;                                                  \
    m_stack.popX();                                                           \
  } else {                                                                    \
    tvCellAsVariant(c2) = tvCellAsVariant(c2).toInt64() OP                    \
                          tvCellAsCVarRef(c1).toInt64();                      \
    m_stack.popC();                                                           \
  }                                                                           \
} while (0)
inline void ALWAYS_INLINE ExecutionContext::iopShl(PC& pc) {
  SHIFTOP(<<);
}

inline void ALWAYS_INLINE ExecutionContext::iopShr(PC& pc) {
  SHIFTOP(>>);
}
#undef SHIFTOP

inline void ALWAYS_INLINE ExecutionContext::iopCastBool(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToBooleanInPlace(c1);
}

inline void ALWAYS_INLINE ExecutionContext::iopCastInt(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToInt64InPlace(c1);
}

inline void ALWAYS_INLINE ExecutionContext::iopCastDouble(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToDoubleInPlace(c1);
}

inline void ALWAYS_INLINE ExecutionContext::iopCastString(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToStringInPlace(c1);
}

inline void ALWAYS_INLINE ExecutionContext::iopCastArray(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToArrayInPlace(c1);
}

inline void ALWAYS_INLINE ExecutionContext::iopCastObject(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  tvCastToObjectInPlace(c1);
}

inline bool ALWAYS_INLINE ExecutionContext::cellInstanceOfStr(TypedValue* tv,
                                                              StringData* s) {
  ASSERT(tv->m_type != KindOfVariant);
  if (tv->m_type == KindOfObject) {
    Class* cls = lookupClass(s);
    if (cls) return tv->m_data.pobj->instanceof(cls);
  }
  return false;
}

inline void ALWAYS_INLINE ExecutionContext::iopInstanceOf(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();   // c2 instanceof c1
  Cell* c2 = m_stack.indC(1);
  bool r = false;
  if (IS_STRING_TYPE(c1->m_type)) {
    StringData* rhs = c1->m_data.pstr;
    r = cellInstanceOfStr(c2, rhs);
  } else if (c1->m_type == KindOfObject) {
    if (c2->m_type == KindOfObject) {
      ObjectData* lhs = c2->m_data.pobj;
      ObjectData* rhs = c1->m_data.pobj;
      r = lhs->instanceof(rhs->getVMClass());
    }
  } else {
    raise_error("Class name must be a valid object or a string");
  }
  m_stack.popC();
  tvRefcountedDecRefCell(c2);
  c2->m_data.num = r;
  c2->_count = 0;
  c2->m_type = KindOfBoolean;
}

inline void ALWAYS_INLINE ExecutionContext::iopInstanceOfD(PC& pc) {
  NEXT();
  DECODE(Id, id);
  StringData* s = m_fp->m_func->m_unit->lookupLitstrId(id);
  Cell* c1 = m_stack.topC();
  bool r = cellInstanceOfStr(c1, s);
  tvRefcountedDecRefCell(c1);
  c1->m_data.num = r;
  c1->_count = 0;
  c1->m_type = KindOfBoolean;
}

inline void ALWAYS_INLINE ExecutionContext::iopPrint(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  print(tvCellAsVariant(c1).toString());
  tvRefcountedDecRefCell(c1);
  c1->_count = 0;
  c1->m_type = KindOfInt64;
  c1->m_data.num = 1;
}

inline void ALWAYS_INLINE ExecutionContext::iopClone(PC& pc) {
  NEXT();
  TypedValue* tv = m_stack.topTV();
  if (tv->m_type != KindOfObject) {
    raise_error("clone called on non-object");
  }
  ObjectData* obj = tv->m_data.pobj;
  const Class* class_ UNUSED = obj->getVMClass();
  ObjectData* newobj;

  /* XXX: This only works for pure user classes or pure
   * builtins. For both, we call obj->clone(), which does
   * the correct thing.
   * For user classes that extend a builtin, we need to call
   * the builtin's ->clone() method, which is currently not
   * available.
   */
  ASSERT(class_->m_derivesFromBuiltin == false);
  newobj = obj->clone();
  m_stack.popTV();
  m_stack.pushNull();
  tv->m_type = KindOfObject;
  tv->_count = 0;
  tv->m_data.pobj = newobj;
}

inline void ALWAYS_INLINE
ExecutionContext::handleUnwind(UnwindStatus unwindType) {
  m_propagateException = false;
  m_ignoreException = false;
  switch (unwindType) {
  case UnwindExit:
    m_halted = true;
    break;
  case UnwindPropagate:
    VMState savedVM;
    m_nestedVMMap.erase(m_firstAR);
    memcpy(&savedVM, &m_nestedVMs.back(), sizeof(savedVM));
    m_pc = savedVM.pc;
    m_fp = savedVM.fp;
    m_firstAR = savedVM.firstAR;
    if (!RuntimeOption::EvalJit) {
      ASSERT(m_stack.top() == savedVM.sp);
    }
    m_stack.top() = savedVM.sp;
    m_propagateException = true;
    exitVM();
    break;
  case UnwindIgnore:
    m_faults.pop_back();
    m_stack.pushNull();
    m_ignoreException = true;
    break;
  case UnwindResumeVM:
    break;
  default:
    not_implemented();
  }
  jmp_buf *buf = m_jmpBufs.back();
  longjmp(*buf, 1);
}

inline void ALWAYS_INLINE ExecutionContext::iopExit(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  if (c1->m_type == KindOfInt64) {
    // XXX Set exit status to c1->m_data.num
  } else {
    // XXX Set exit status to 0
    print(tvCellAsVariant(c1).toString());
  }
  m_stack.popC();
  Fault fault;
  fault.m_faultType = Fault::KindOfExit;
  m_faults.push_back(fault);
  UnwindStatus unwindType = m_stack.unwindFrame(m_fp, pcOff(),
                                                m_pc, fault);
  if (unwindType == UnwindExit) {
    ONTRACE(1,
            Trace::trace("dispatch: Exit ExecutionContext::dispatch(%p)\n",
                         m_fp->m_func));
  }
  handleUnwind(unwindType);
}

inline void ALWAYS_INLINE ExecutionContext::iopRaise(PC& pc) {
  not_implemented();
}

inline void ALWAYS_INLINE ExecutionContext::iopFatal(PC& pc) {
  Fault fault;
  memcpy(&fault.m_userException, m_stack.top(), sizeof(TypedValue));
  m_stack.discard();
  if (!IS_STRING_TYPE(fault.m_userException.m_type)) {
    //XXX:
    not_implemented();
  }
  fault.m_faultType = Fault::KindOfFatal;
  m_faults.push_back(fault);
  UnwindStatus unwindType = m_stack.unwindFrame(m_fp, pcOff(),
                                                m_pc, fault);
  if (unwindType == UnwindExit) {
    ONTRACE(1,
            Trace::trace("Exit from Fatal ExecutionContext::dispatch(%p)\n",
                         m_fp->m_func));
  }
  handleUnwind(unwindType);
}

inline void ALWAYS_INLINE ExecutionContext::iopJmp(PC& pc) {
  NEXT();
  DECODE_JMP(Offset, offset);
  pc += offset - 1;
}

#define JMPOP(OP, VOP) do {                                                   \
  Cell* c1 = m_stack.topC();                                                  \
  if (c1->m_type == KindOfInt64 || c1->m_type == KindOfBoolean) {             \
    int64 n = c1->m_data.num;                                                 \
    if (n OP 0) {                                                             \
      NEXT();                                                                 \
      DECODE_JMP(Offset, offset);                                             \
      pc += offset - 1;                                                       \
      m_stack.popX();                                                         \
    } else {                                                                  \
        pc += 1 + sizeof(Offset);                                             \
      m_stack.popX();                                                         \
    }                                                                         \
  } else {                                                                    \
    if (VOP(tvCellAsCVarRef(c1))) {                                           \
      NEXT();                                                                 \
      DECODE_JMP(Offset, offset);                                             \
      pc += offset - 1;                                                       \
      m_stack.popC();                                                         \
    } else {                                                                  \
      pc += 1 + sizeof(Offset);                                               \
      m_stack.popC();                                                         \
    }                                                                         \
  }                                                                           \
} while (0)
inline void ALWAYS_INLINE ExecutionContext::iopJmpZ(PC& pc) {
  JMPOP(==, !bool);
}

inline void ALWAYS_INLINE ExecutionContext::iopJmpNZ(PC& pc) {
  JMPOP(!=, bool);
}
#undef JMPOP

inline void ALWAYS_INLINE ExecutionContext::iopRetC(PC& pc) {
  NEXT();
  // Call the runtime helpers to free the local variables and iterators
  frame_free_locals_inl(m_fp);
  ActRec* sfp = arGetSfp(m_fp);

  // Memcpy the the return value on top of the activation record. This works
  // the same regardless of whether the return value is boxed or not.
  memcpy(&(m_fp->m_r), m_stack.topTV(), sizeof(TypedValue));
  // Adjust the stack
  m_stack.ndiscard(m_fp->m_func->numSlotsInFrame() + 1);

  if (LIKELY(sfp != m_fp)) {
    // Restore caller's execution state.
    uint soff = m_fp->m_soff;
    m_fp = sfp;
    pc = m_fp->m_func->m_unit->entry() + m_fp->m_func->m_base + soff;
    m_stack.ret();
  } else {
    // No caller; terminate.
    m_stack.ret();
#ifdef HPHP_TRACE
    {
      std::ostringstream os;
      m_stack.toStringElm(os, m_stack.topTV());
      ONTRACE(1,
              Trace::trace("Return %s from ExecutionContext::dispatch("
                           "%p)\n", os.str().c_str(), m_fp));
    }
#endif
    pc = NULL;
    m_fp = NULL;
    g_context->m_halted = true;
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopRetV(PC& pc) {
  iopRetC(pc);
}

inline void ALWAYS_INLINE ExecutionContext::iopUnwind(PC& pc) {
  Offset faultPC = m_fp->m_func->findFaultPCFromEH(pcOff());
  Fault fault = m_faults.back();
  UnwindStatus unwindType = m_stack.unwindFrame(m_fp, faultPC, m_pc, fault);
  if (unwindType == UnwindExit) {
    ONTRACE(1,
            Trace::trace("Exit from Unwind ExecutionContext::dispatch(%p)\n",
                         m_fp->m_func));
  }
  handleUnwind(unwindType);
}

inline void ALWAYS_INLINE ExecutionContext::iopThrow(PC& pc) {
  Fault fault;
  memcpy(&fault.m_userException, m_stack.top(), sizeof(TypedValue));
  m_stack.discard();
  ASSERT(fault.m_userException.m_type == KindOfObject);
  Instance *e = reinterpret_cast<Instance*>(fault.m_userException.m_data.pobj);
  if (!e->instanceof(SystemLib::s_ExceptionClass)) {
    raise_error("Thrown object is not of type Exception: %s",
                   e->o_getClassName().get()->data());
  }
  // call the __init__ method on thrown exception. The method
  // internally takes care that it captures the stack trace only once
  static StringData* sd_init = StringData::GetStaticString("__init__");
  const Func* exceptionInit = e->getVMClass()->lookupMethod(sd_init);
  if (exceptionInit) {
    TypedValue tv;
    invokeFunc(&tv, exceptionInit, Array::Create(), e);
    ASSERT(!IS_REFCOUNTED_TYPE(tv.m_type));
  }
  DEBUGGER_ATTACHED_ONLY(phpExceptionHook(&fault.m_userException));
  fault.m_faultType = Fault::KindOfThrow;
  m_faults.push_back(fault);
  UnwindStatus unwindType = m_stack.unwindFrame(m_fp, pcOff(),
                                                m_pc, fault);
  if (unwindType == UnwindExit) {
    ONTRACE(1,
            Trace::trace("Exit from Throw ExecutionContext::dispatch(%p)\n",
                         m_fp->m_func));
  }
  handleUnwind(unwindType);
}

inline void ALWAYS_INLINE ExecutionContext::iopLoc(PC& pc) {
  NEXT();
  DECODE_IVA(local);
  ASSERT(local < m_fp->m_func->m_numLocals);
  Home* h1 = m_stack.allocH();
  h1->m_data.ptv = frame_local(m_fp, local);
  h1->_count = 0;
  h1->m_type = KindOfHome;
}

#define CLS() do {                                                            \
  NEXT();                                                                     \
  DECODE_IVA(ind);                                                            \
  TypedValue* tv = m_stack.indTV(ind);                                        \
  TypedValue* input = CLS_INPUT;                                              \
  const Class* class_ = NULL;                                                 \
  if (IS_STRING_TYPE(input->m_type)) {                                        \
    class_ = g_context->loadClass(input->m_data.pstr);                        \
    if (class_ == NULL) {                                                     \
      raise_error("Class undefined: %s", input->m_data.pstr->data());         \
    }                                                                         \
  } else if (input->m_type == KindOfObject) {                                 \
    class_ = input->m_data.pobj->getVMClass();                                \
  } else {                                                                    \
    raise_error("Cls: Expected string or object");                            \
  }                                                                           \
  CLS_CLOBBER                                                                 \
  tv->m_data.pcls = const_cast<Class*>(class_);                               \
  tv->_count = 0;                                                             \
  tv->m_type = KindOfClass;                                                   \
} while (0)

inline void ALWAYS_INLINE ExecutionContext::iopCls(PC& pc) {
#define CLS_INPUT tv
#define CLS_CLOBBER do {                                                      \
  if (IS_REFCOUNTED_TYPE(tv->m_type)) tvDecRefCell(tv);                       \
} while(0);
  CLS();
#undef CLS_INPUT
#undef CLS_CLOBBER
}

inline void ALWAYS_INLINE ExecutionContext::iopClsH(PC& pc) {
#define CLS_INPUT tv->m_data.ptv
#define CLS_CLOBBER
  CLS();
#undef CLS_INPUT
#undef CLS_CLOBBER
}
#undef CLS

#define CGETH_BODY()                                                          \
  if (fr->m_type == KindOfUninit) {                                           \
    size_t pind = ((uintptr_t(m_fp) - uintptr_t(fr)) / sizeof(TypedValue))    \
                  - 1;                                                        \
    raise_notice("Undefined variable: %s",                                    \
                 (pind < m_fp->m_func->m_pnames.size())                       \
                 ? m_fp->m_func->m_pnames[pind]->data() : "<?>");             \
  }                                                                           \
  tvDup(fr, to);                                                              \
  if (to->m_type == KindOfVariant) {                                          \
    tvUnbox(to);                                                              \
  }
inline void ALWAYS_INLINE ExecutionContext::iopCGetH(PC& pc) {
  NEXT();
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = to->m_data.ptv;
  CGETH_BODY()
}

inline void ALWAYS_INLINE ExecutionContext::iopCGetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  LOOKUP_VAR(name, to, fr);
  if (fr == NULL) {
    raise_notice("Undefined variable: %s", name->data());
    tvRefcountedDecRefCell(to);
    TV_WRITE_NULL(to);
  } else {
    tvRefcountedDecRefCell(to);
    CGETH_BODY()
  }
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopCGetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  LOOKUP_GBL(name, to, fr);
  if (fr == NULL) {
    if (MoreWarnings) {
      raise_notice("Undefined variable: %s", name->data());
    }
    tvRefcountedDecRefCell(to);
    TV_WRITE_NULL(to);
  } else {
    tvRefcountedDecRefCell(to);
    CGETH_BODY()
  }
  LITSTR_DECREF(name);
}

#define SPROP_OP_PRELUDE                                                      \
  NEXT();                                                                     \
  TypedValue* tv1 = m_stack.topTV();                                          \
  TypedValue* tv2 = m_stack.indTV(1);                                         \
  StringData* name;                                                           \
  TypedValue* val;                                                            \
  bool visible, accessible;                                                   \
  LOOKUP_SPROP(tv2, name, tv1, val, visible, accessible);
#define SPROP_OP_POSTLUDE                                                     \
  LITSTR_DECREF(name);

#define GETS(box) do {                                                        \
  SPROP_OP_PRELUDE                                                            \
  if (!(visible && accessible)) {                                             \
    raise_error("Invalid static property access: %s::%s",                     \
                tv2->m_data.pcls->m_preClass->m_name->data(),                 \
                name->data());                                                \
  }                                                                           \
  if (box) {                                                                  \
    if (val->m_type != KindOfVariant) {                                       \
      tvBox(val);                                                             \
    }                                                                         \
    tvDupVar(val, tv2);                                                       \
  } else {                                                                    \
    tvReadCell(val, tv2);                                                     \
  }                                                                           \
  m_stack.popC();                                                             \
  SPROP_OP_POSTLUDE                                                           \
} while (0)
inline void ALWAYS_INLINE ExecutionContext::iopCGetS(PC& pc) {
  GETS(false);
}

inline void ALWAYS_INLINE ExecutionContext::iopCGetM(PC& pc) {
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelper(GETHELPER_ARGS);
  if (tvL->m_type == KindOfVariant) {
    tvUnbox(tvL);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopCGetH2(PC& pc) {
  NEXT();
  TypedValue* to = m_stack.indTV(1);
  TypedValue* fr = to->m_data.ptv;
  CGETH_BODY()
}
#undef CGETH_BODY

#define VGETH_BODY()                                                          \
  if (fr->m_type != KindOfVariant) {                                          \
    tvBox(fr);                                                                \
  }                                                                           \
  tvDup(fr, to);

inline void ALWAYS_INLINE ExecutionContext::iopVGetH(PC& pc) {
  NEXT();
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = to->m_data.ptv;
  VGETH_BODY()
}

inline void ALWAYS_INLINE ExecutionContext::iopVGetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  LOOKUPD_VAR(name, to, fr);
  ASSERT(fr != NULL);
  tvRefcountedDecRefCell(to);
  VGETH_BODY()
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopVGetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = NULL;
  LOOKUPD_GBL(name, to, fr);
  ASSERT(fr != NULL);
  tvRefcountedDecRefCell(to);
  VGETH_BODY()
  LITSTR_DECREF(name);
}
#undef VGETH_BODY

inline void ALWAYS_INLINE ExecutionContext::iopVGetS(PC& pc) {
  GETS(true);
}
#undef GETS

inline void ALWAYS_INLINE ExecutionContext::iopVGetM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  TypedValue* tv1 = m_stack.allocTV();
  tvWriteUninit(tv1);
  if (!setHelperPre<false, true, false, 1, 0>(SETHELPERPRE_ARGS)) {
    if (base->m_type != KindOfVariant) {
      tvBox(base);
    }
    tvDupVar(base, tv1);
  } else {
    tvWriteNull(tv1);
    tvBox(tv1);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void ALWAYS_INLINE ExecutionContext::iopIssetH(PC& pc) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  bool e = isset(tvAsCVarRef(tv1->m_data.ptv));
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->_count = 0;
  tv1->m_type = KindOfBoolean;
}

inline void ALWAYS_INLINE ExecutionContext::iopIssetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  LOOKUP_VAR(name, tv1, tv);
  if (tv == NULL) {
    e = false;
  } else {
    e = isset(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->_count = 0;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopIssetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  LOOKUP_GBL(name, tv1, tv);
  if (tv == NULL) {
    e = false;
  } else {
    e = isset(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->_count = 0;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopIssetS(PC& pc) {
  SPROP_OP_PRELUDE
  bool e;
  if (!(visible && accessible)) {
    e = false;
  } else {
    e = isset(tvAsCVarRef(val));
  }
  m_stack.popC();
  tv2->m_data.num = e;
  tv2->_count = 0;
  tv2->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

inline void ALWAYS_INLINE ExecutionContext::iopIssetM(PC& pc) {
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelperPre<false, false, 1>(GETHELPERPRE_ARGS);
  // Process last member specially, in order to employ the IssetElem/IssetProp
  // operations.
  DECODE(unsigned char, mcode);
  TypedValue* tvV = tvToKeyValue(m_stack.topTV());
  bool issetResult = false;
  switch (MemberCode(mcode)) {
  case ME: {
    issetResult = IssetEmptyElem<false>(tvScratch, tvRef, base, baseStrOff,
                                        tvV);
    break;
  }
  case MP: {
    PreClass* ctx = arGetContextPreClass(m_fp);
    issetResult = IssetEmptyProp<false>(ctx, base, tvV);
    break;
  }
  default: ASSERT(false);
  }
  getHelperPost<false>(GETHELPERPOST_ARGS);
  tvL->m_data.num = issetResult;
  tvL->_count = 0;
  tvL->m_type = KindOfBoolean;
}

inline void ALWAYS_INLINE ExecutionContext::iopEmptyH(PC& pc) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  bool e = empty(tvAsCVarRef(tv1->m_data.ptv));
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->_count = 0;
  tv1->m_type = KindOfBoolean;
}

inline void ALWAYS_INLINE ExecutionContext::iopEmptyN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  LOOKUP_VAR(name, tv1, tv);
  if (tv == NULL) {
    e = true;
  } else {
    e = empty(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->_count = 0;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopEmptyG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  bool e;
  LOOKUP_GBL(name, tv1, tv);
  if (tv == NULL) {
    e = true;
  } else {
    e = empty(tvAsCVarRef(tv));
  }
  tvRefcountedDecRefCell(tv1);
  tv1->m_data.num = e;
  tv1->_count = 0;
  tv1->m_type = KindOfBoolean;
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopEmptyS(PC& pc) {
  SPROP_OP_PRELUDE
  bool e;
  if (!(visible && accessible)) {
    e = true;
  } else {
    e = empty(tvAsCVarRef(val));
  }
  m_stack.popC();
  tv2->m_data.num = e;
  tv2->_count = 0;
  tv2->m_type = KindOfBoolean;
  SPROP_OP_POSTLUDE
}

inline void ALWAYS_INLINE ExecutionContext::iopEmptyM(PC& pc) {
  NEXT();
  DECLARE_GETHELPER_ARGS
  getHelperPre<false, false, 1>(GETHELPERPRE_ARGS);
  // Process last member specially, in order to employ the EmptyElem/EmptyProp
  // operations.
  DECODE(unsigned char, mcode);
  TypedValue* tvV = tvToKeyValue(m_stack.topTV());
  bool emptyResult = false;
  switch (MemberCode(mcode)) {
  case ME: {
    emptyResult = IssetEmptyElem<true>(tvScratch, tvRef, base, baseStrOff, tvV);
    break;
  }
  case MP: {
    PreClass* ctx = arGetContextPreClass(m_fp);
    emptyResult = IssetEmptyProp<true>(ctx, base, tvV);
    break;
  }
  default: ASSERT(false);
  }
  getHelperPost<false>(GETHELPERPOST_ARGS);
  tvL->m_data.num = emptyResult;
  tvL->_count = 0;
  tvL->m_type = KindOfBoolean;
}

inline void ALWAYS_INLINE ExecutionContext::iopSetH(PC& pc) {
  NEXT();
  Cell* fr = m_stack.topC();
  Home* h2 = m_stack.indH(1);
  TypedValue* to = h2->m_data.ptv;
  tvSet(fr, to);
  memcpy((void*)h2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
}

inline void ALWAYS_INLINE ExecutionContext::iopSetN(PC& pc) {
  NEXT();
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  LOOKUPD_VAR(name, tv2, to);
  ASSERT(to != NULL);
  tvSet(fr, to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopSetG(PC& pc) {
  NEXT();
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  LOOKUPD_GBL(name, tv2, to);
  ASSERT(to != NULL);
  tvSet(fr, to);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopSetS(PC& pc) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* tv3 = m_stack.indTV(2);
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  LOOKUP_SPROP(tv3, name, tv2, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                tv3->m_data.pcls->m_preClass->m_name->data(),
                name->data());
  }
  tvSet(tv1, val);
  memcpy(tv3, tv1, sizeof(TypedValue));
  m_stack.discard();
  m_stack.popC();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopSetM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, true, false, 1, 1>(SETHELPERPRE_ARGS)) {
    // Set.
    Cell* c1 = m_stack.topC();
    switch (MemberCode(*vecp)) {
    case ME: {
      TypedValue* key = tvToKeyValue(m_stack.indTV(1));
      SetElem(base, key, c1);
      break;
    }
    case MW: {
      SetNewElem(base, c1);
      break;
    }
    case MP: {
      TypedValue* key = tvToKeyValue(m_stack.indTV(1));
      PreClass* ctx = arGetContextPreClass(m_fp);
      SetProp(ctx, base, key, c1);
      break;
    }
    default: ASSERT(false);
    }
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void ALWAYS_INLINE ExecutionContext::iopSetOpH(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  Cell* fr = m_stack.topC();
  Home* h2 = m_stack.indH(1);
  TypedValue* to = h2->m_data.ptv;
  SETOP_BODY(to, op, fr);
  tvReadCell(to, (TypedValue*)h2);
  m_stack.popTV();
}

inline void ALWAYS_INLINE ExecutionContext::iopSetOpN(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  // XXX We're probably not getting warnings totally correct here
  LOOKUPD_VAR(name, tv2, to);
  ASSERT(to != NULL);
  SETOP_BODY(to, op, fr);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopSetOpG(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* to = NULL;
  // XXX We're probably not getting warnings totally correct here
  LOOKUPD_GBL(name, tv2, to);
  ASSERT(to != NULL);
  SETOP_BODY(to, op, fr);
  memcpy((void*)tv2, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopSetOpS(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  Cell* fr = m_stack.topC();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* tv3 = m_stack.indTV(2);
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  LOOKUP_SPROP(tv3, name, tv2, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                tv3->m_data.pcls->m_preClass->m_name->data(),
                name->data());
  }
  SETOP_BODY(val, op, fr);
  memcpy(tv3, fr, sizeof(TypedValue));
  m_stack.discard();
  m_stack.popC();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopSetOpM(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<MoreWarnings, true, false, 1, 1>(SETHELPERPRE_ARGS)) {
    TypedValue* result;
    Cell* rhs = m_stack.topC();
    switch (MemberCode(*vecp)) {
    case ME: {
      TypedValue* key = tvToKeyValue(m_stack.indTV(1));
      result = SetOpElem(tvScratch, tvRef, op, base, key, rhs);
      break;
    }
    case MW: {
      result = SetOpNewElem(tvScratch, tvRef, op, base, rhs);
      break;
    }
    case MP: {
      TypedValue* key = tvToKeyValue(m_stack.indTV(1));
      PreClass *ctx = arGetContextPreClass(m_fp);
      result = SetOpProp(tvScratch, tvRef, ctx, op, base, key, rhs);
      break;
    }
    default: {
      ASSERT(false);
      result = NULL; // Silence compiler warning.
    }
    }

    if (result->m_type == KindOfVariant) {
      tvUnbox(result);
    }
    tvRefcountedDecRef(rhs);
    tvDup(result, rhs);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void ALWAYS_INLINE ExecutionContext::iopIncDecH(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  TypedValue* to = m_stack.topTV();
  TypedValue* fr = to->m_data.ptv;
  IncDecBody(op, fr, to);
}

inline void ALWAYS_INLINE ExecutionContext::iopIncDecN(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* local = NULL;
  // XXX We're probably not getting warnings totally correct here
  LOOKUPD_VAR(name, nameCell, local);
  ASSERT(local != NULL);
  IncDecBody(op, local, nameCell);
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopIncDecG(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  StringData* name;
  TypedValue* nameCell = m_stack.topTV();
  TypedValue* gbl = NULL;
  // XXX We're probably not getting warnings totally correct here
  LOOKUPD_GBL(name, nameCell, gbl);
  ASSERT(gbl != NULL);
  IncDecBody(op, gbl, nameCell);
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopIncDecS(PC& pc) {
  SPROP_OP_PRELUDE
  DECODE(unsigned char, op);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                tv2->m_data.pcls->m_preClass->m_name->data(),
                name->data());
  }
  IncDecBody(op, val, tv2);
  m_stack.popC();
  SPROP_OP_POSTLUDE
}

inline void ALWAYS_INLINE ExecutionContext::iopIncDecM(PC& pc) {
  NEXT();
  DECODE(unsigned char, op);
  DECLARE_SETHELPER_ARGS
  TypedValue to;
  tvWriteUninit(&to);
  if (!setHelperPre<true, true, false, 0, 1>(SETHELPERPRE_ARGS)) {
    switch (MemberCode(*vecp)) {
    case ME: {
      TypedValue* key = tvToKeyValue(m_stack.topTV());
      IncDecElem(tvScratch, tvRef, op, base, key, to);
      break;
    }
    case MW: {
      IncDecNewElem(tvScratch, tvRef, op, base, to);
      break;
    }
    case MP: {
      PreClass* ctx = arGetContextPreClass(m_fp);
      TypedValue* key = tvToKeyValue(m_stack.topTV());
      IncDecProp(tvScratch, tvRef, ctx, op, base, key, to);
      break;
    }
    default: ASSERT(false);
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
  Cell* c1 = m_stack.allocC();
  memcpy(c1, &to, sizeof(TypedValue));
}

inline void ALWAYS_INLINE ExecutionContext::iopBindH(PC& pc) {
  NEXT();
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* fr = m_stack.indTV(1);
  tvBind(tv1, fr->m_data.ptv);
  memcpy((void*)fr, (void*)tv1, sizeof(TypedValue));
  m_stack.discard();
}

inline void ALWAYS_INLINE ExecutionContext::iopBindN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* fr = m_stack.topTV();
  TypedValue* nameTV = m_stack.indTV(1);
  TypedValue* to = NULL;
  LOOKUPD_VAR(name, nameTV, to);
  ASSERT(to != NULL);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopBindG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* fr = m_stack.topTV();
  TypedValue* nameTV = m_stack.indTV(1);
  TypedValue* to = NULL;
  LOOKUPD_GBL(name, nameTV, to);
  ASSERT(to != NULL);
  tvBind(fr, to);
  memcpy((void*)nameTV, (void*)fr, sizeof(TypedValue));
  m_stack.discard();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopBindS(PC& pc) {
  NEXT();
  TypedValue* fr = m_stack.topTV();
  TypedValue* tv2 = m_stack.indTV(1);
  TypedValue* tv3 = m_stack.indTV(2);
  StringData* name;
  TypedValue* val;
  bool visible, accessible;
  LOOKUP_SPROP(tv3, name, tv2, val, visible, accessible);
  if (!(visible && accessible)) {
    raise_error("Invalid static property access: %s::%s",
                tv3->m_data.pcls->m_preClass->m_name->data(),
                name->data());
  }
  tvBind(fr, val);
  memcpy(tv3, fr, sizeof(TypedValue));
  m_stack.discard();
  m_stack.popC();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopBindM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  TypedValue* tv1 = m_stack.topTV();
  if (!setHelperPre<false, true, false, 1, 0>(SETHELPERPRE_ARGS)) {
    // Bind the element/property with the var on the top of the stack
    tvBind(tv1, base);
  }
  setHelperPost<1>(SETHELPERPOST_ARGS);
}

inline void ALWAYS_INLINE ExecutionContext::iopUnsetH(PC& pc) {
  NEXT();
  Home* h1 = m_stack.topH();
  TypedValue* tv = h1->m_data.ptv;
  tvRefcountedDecRef(tv);
  TV_WRITE_UNINIT(tv);
  m_stack.popH();
}

inline void ALWAYS_INLINE ExecutionContext::iopUnsetN(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  TypedValue* tv = NULL;
  LOOKUP_VAR(name, tv1, tv);
  ASSERT(!m_fp->hasInvName());
  if (tv != NULL) {
    tvRefcountedDecRef(tv);
    TV_WRITE_UNINIT(tv);
  } else if (m_fp->m_varEnv != NULL) {
    m_fp->m_varEnv->unset(name);
  }
  m_stack.popC();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopUnsetG(PC& pc) {
  NEXT();
  StringData* name;
  TypedValue* tv1 = m_stack.topTV();
  LOOKUP_NAME(name, tv1);
  VarEnv* varEnv = g_context->m_varEnvs.front();
  ASSERT(varEnv != NULL);
  varEnv->unset(name);
  m_stack.popC();
  LITSTR_DECREF(name);
}

inline void ALWAYS_INLINE ExecutionContext::iopUnsetM(PC& pc) {
  NEXT();
  DECLARE_SETHELPER_ARGS
  if (!setHelperPre<false, false, true, 0, 1>(SETHELPERPRE_ARGS)) {
    // Unset.
    TypedValue* tv1 = m_stack.topTV();
    if (tv1->m_type == KindOfHome) {
      tv1 = tv1->m_data.ptv;
    }
    TypedValue* key = tvToKeyValue(tv1);
    switch (MemberCode(*vecp)) {
    case ME: {
      UnsetElem(base, key);
      break;
    }
    case MP: {
      PreClass* ctx = arGetContextPreClass(m_fp);
      UnsetProp(ctx, base, key);
      break;
    }
    default: ASSERT(false);
    }
  }
  setHelperPost<0>(SETHELPERPOST_ARGS);
}

inline void ALWAYS_INLINE ExecutionContext::iopFPushFunc(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC();
  const Func* func = NULL;
  ObjectData* origObj = NULL;
  StringData* origSd = NULL;
  if (IS_STRING_TYPE(c1->m_type)) {
    origSd = c1->m_data.pstr;
    func = lookupFunc(origSd);
  } else if (c1->m_type == KindOfObject) {
    static StringData* invokeName = StringData::GetStaticString("__invoke");
    origObj = c1->m_data.pobj;
    const Class* cls = origObj->getVMClass();
    func = cls->lookupMethod(invokeName);
    if (func == NULL) {
      raise_error("Function name must be a string");
    }
  } else {
    raise_error("Function name must be a string");
  }
  if (func == NULL) {
    raise_error("Undefined function: %s", c1->m_data.pstr->data());
  }
  ASSERT(!origObj || !origSd);
  ASSERT(origObj || origSd);
  ASSERT(origObj || !func->m_needsStaticLocalCtx);
  // We've already saved origObj or origSd; we'll use them after
  // overwriting the pointer on the stack.  Don't refcount it now; defer
  // till after we're done with it.
  m_stack.discard();
  ActRec* ar = m_stack.allocA();
  ar->m_func = func;
  if (origObj) {
    if (func->m_needsStaticLocalCtx) {
      if (func->m_isClosureBody) {
        // Closures are special -- each closure instantiation, even from the
        // same closure body, gets its own static local context.
        TypedValue* prop;
        ASSERT(origObj->isInstance());
        static_cast<Instance*>(origObj)->prop(
          prop,
          origObj->getVMClass()->m_preClass.get(),
          StringData::GetStaticString("__static_locals"));
        if (prop->m_type == KindOfNull) {
          prop->m_data.parr = NEW(HphpArray)(1);
          prop->m_data.parr->incRefCount();
          prop->m_type = KindOfArray;
        }
        ASSERT(prop->m_type == KindOfArray);
        ASSERT(HphpArray::isHphpArray(prop->m_data.parr));

        ar->m_staticLocalCtx = static_cast<HphpArray*>(prop->m_data.parr);
      } else {
        ar->m_staticLocalCtx = origObj->getVMClass()->getStaticLocals();
      }
    }
    if (func->m_attrs & AttrStatic) {
      ar->setClass(origObj->getVMClass());
      if (origObj->decRefCount() == 0) {
        origObj->release();
      }
    } else {
      ar->setThis(origObj);
      // Teleport the reference from the destroyed stack cell to the
      // ActRec. Don't try this at home.
    }
  } else {
    ar->setThis(NULL);
    if (origSd->decRefCount() == 0) {
      origSd->release();
    }
  }
  ar->m_numArgs = numArgs;
  ar->setVarEnv(NULL);
}

inline void ALWAYS_INLINE ExecutionContext::iopFPushFuncD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  Func* func = lookupFunc(m_fp->m_func->m_unit->lookupLitstrId(id));
  if (func == NULL) {
    raise_error("Undefined function: %s",
                m_fp->m_func->m_unit->lookupLitstrId(id)->data());
  }
  DEBUGGER_IF(phpBreakpointEnabled(func->m_name->data()));
  ASSERT(!func->m_needsStaticLocalCtx);
  ActRec* ar = m_stack.allocA();
  ar->m_func = func;
  ar->setThis(NULL);
  ar->m_numArgs = numArgs;
  ar->setVarEnv(NULL);
}

#define OBJMETHOD_BODY(cls, name, obj) do { \
  const Func* f; \
  LookupResult res = lookupObjMethod(f, cls, name, true); \
  ASSERT(f); \
  ActRec* ar = m_stack.allocA(); \
  ar->m_func = f; \
  if (res == MethodFoundNoThis) { \
    if (obj->decRefCount() == 0) obj->release(); \
    ar->setClass(cls); \
  } else { \
    ASSERT(res == MethodFoundWithThis || res == MagicCallFound); \
    /* Transfer ownership of obj to the ActRec*/ \
    ar->setThis(obj); \
  } \
  ar->m_staticLocalCtx = cls->getStaticLocals(); \
  ar->m_numArgs = numArgs; \
  if (res == MagicCallFound) { \
    ar->setInvName(name); \
  } else { \
    ar->setVarEnv(NULL); \
    LITSTR_DECREF(name); \
  } \
} while (0)

inline void ALWAYS_INLINE ExecutionContext::iopFPushObjMethod(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC(); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error("FPushObjMethod method argument must be a string");
  }
  Cell* c2 = m_stack.indC(1); // Object.
  if (c2->m_type != KindOfObject) {
    raise_error("Method call on non-object");
  }
  ObjectData* obj = c2->m_data.pobj;
  Class* cls = obj->getVMClass();
  StringData* name = c1->m_data.pstr;
  // We handle decReffing obj and name below
  m_stack.ndiscard(2);
  OBJMETHOD_BODY(cls, name, obj);
}

inline void ALWAYS_INLINE ExecutionContext::iopFPushObjMethodD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  Cell* c1 = m_stack.topC();
  if (c1->m_type != KindOfObject) {
    raise_error("Method call on non-object");
  }
  ObjectData* obj = c1->m_data.pobj;
  Class* cls = obj->getVMClass();
  StringData* name =
    const_cast<StringData*>(m_fp->m_func->m_unit->lookupLitstrId(id));
  // We handle decReffing obj below
  m_stack.discard();
  OBJMETHOD_BODY(cls, name, obj);
}

#define CLSMETHOD_BODY(cls, name, obj, forwarding) do { \
  const Func* f; \
  LookupResult res = lookupClsMethod(f, cls, name, obj, true); \
  if (res == MethodFoundNoThis || res == MagicCallStaticFound) { \
    obj = NULL; \
  } else { \
    ASSERT(obj); \
    ASSERT(res == MethodFoundWithThis || res == MagicCallFound); \
    obj->incRefCount(); \
  } \
  ASSERT(f); \
  ActRec* ar = m_stack.allocA(); \
  ar->m_func = f; \
  ar->m_staticLocalCtx = cls->getStaticLocals(); \
  if (obj) { \
    ar->setThis(obj); \
  } else { \
    if (!forwarding) { \
      ar->setClass(cls); \
    } else { \
      /* Propogate the current late bound class if there is one, */ \
      /* otherwise use the class given by this instruction's input */ \
      if (m_fp->hasThis()) { \
        cls = m_fp->getThis()->getVMClass(); \
      } else if (m_fp->hasClass()) { \
        cls = m_fp->getClass(); \
      } \
      ar->setClass(cls); \
    } \
  } \
  ar->m_numArgs = numArgs; \
  if (res == MagicCallFound || res == MagicCallStaticFound) { \
    ar->setInvName(name); \
  } else { \
    ar->setVarEnv(NULL); \
    LITSTR_DECREF(const_cast<StringData*>(name)); \
  } \
} while (0)

inline void ALWAYS_INLINE ExecutionContext::iopFPushClsMethod(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC(); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error("FPushClsMethod method argument must be a string");
  }
  TypedValue* tv = m_stack.indTV(1);
  ASSERT(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  StringData* name = c1->m_data.pstr;
  // CLSMETHOD_BODY will take care of decReffing name
  m_stack.ndiscard(2);
  ASSERT(cls && name);
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : NULL;
  CLSMETHOD_BODY(cls, name, obj, false);
}

inline void ALWAYS_INLINE ExecutionContext::iopFPushClsMethodD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, methodId);
  DECODE(Id, classId);
  const StringData* className = m_fp->m_func->m_unit->lookupLitstrId(classId);
  Class* cls = loadClass(className);
  if (cls == NULL) {
    raise_error("Class undefined: %s", className->data());
  }
  StringData* name =
    const_cast<StringData*>(m_fp->m_func->m_unit->lookupLitstrId(methodId));
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : NULL;
  CLSMETHOD_BODY(cls, name, obj, false);
}

inline void ALWAYS_INLINE ExecutionContext::iopFPushClsMethodF(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  Cell* c1 = m_stack.topC(); // Method name.
  if (!IS_STRING_TYPE(c1->m_type)) {
    raise_error("FPushClsMethodF method argument must be a string");
  }
  TypedValue* tv = m_stack.indTV(1);
  ASSERT(tv->m_type == KindOfClass);
  Class* cls = tv->m_data.pcls;
  ASSERT(cls);
  StringData* name = c1->m_data.pstr;
  // CLSMETHOD_BODY will take care of decReffing name
  m_stack.ndiscard(2);
  ObjectData* obj = m_fp->hasThis() ? m_fp->getThis() : NULL;
  CLSMETHOD_BODY(cls, name, obj, true);
}

#undef CLSMETHOD_BODY

inline void ALWAYS_INLINE ExecutionContext::iopFPushCtor(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  TypedValue* tv = m_stack.topTV();
  ASSERT(tv->m_type == KindOfClass);
  Class* class_ = tv->m_data.pcls;
  ASSERT(class_ != NULL);
  // Replace input with uninitialized instance.
  ObjectData* this_ = newInstance(class_);
  TRACE(2, "FPushCtor: just new'ed an instance of class %s: %p\n",
        class_->m_preClass->m_name->data(), this_);
  this_->incRefCount();
  this_->incRefCount();
  tv->m_type = KindOfObject;
  tv->_count = 0;
  tv->m_data.pobj = this_;
  // Push new activation record.
  ActRec* ar = m_stack.allocA();
  ar->m_func = class_->m_ctor.func;
  ar->setThis(this_);
  ar->m_staticLocalCtx = class_->getStaticLocals();
  ar->m_numArgs = numArgs;
  ar->setVarEnv(NULL);
}

inline void ALWAYS_INLINE ExecutionContext::iopFPushCtorD(PC& pc) {
  NEXT();
  DECODE_IVA(numArgs);
  DECODE(Id, id);
  Class* class_ = loadClass(m_fp->m_func->m_unit->lookupLitstrId(id));
  if (class_ == NULL) {
    raise_error("Undefined class: %s",
                m_fp->m_func->m_unit->lookupLitstrId(id)->data());
  }
  // Push uninitialized instance.
  ObjectData* this_ = newInstance(class_);
  TRACE(2, "FPushCtorD: new'ed an instance of class %s: %p\n",
        class_->m_preClass->m_name->data(), this_);
  this_->incRefCount();
  m_stack.pushObject(this_);
  // Push new activation record.
  ActRec* ar = m_stack.allocA();
  ar->m_func = class_->m_ctor.func;
  ar->setThis(this_);
  ar->m_staticLocalCtx = class_->getStaticLocals();
  ar->m_numArgs = numArgs;
  ar->setVarEnv(NULL);
}

static inline ActRec* arFromInstr(TypedValue* sp, const Opcode* pc) {
  return arFromSpOffset((ActRec*)sp, instrSpToArDelta(pc));
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassC(PC& pc) {
#ifdef DEBUG
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
#endif
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
}

#define FPASSC_CHECKED_PRELUDE                                                \
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);                       \
  NEXT();                                                                     \
  DECODE_IVA(paramId);                                                        \
  ASSERT(paramId < ar->m_numArgs);                                            \
  const Func* func = ar->m_func;

inline void ALWAYS_INLINE ExecutionContext::iopFPassCW(PC& pc) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCW: function %s(%d) param %d is by reference, "
          "raising a strict warning (attr:0x%x)\n",
          func->name(), func->m_numParams, paramId,
          func->isBuiltin() ? func->m_info->attribute : 0);
    raise_strict_warning("Only variables should be passed by reference");
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassCE(PC& pc) {
  FPASSC_CHECKED_PRELUDE
  if (func->mustBeRef(paramId)) {
    TRACE(1, "FPassCE: function %s(%d) param %d is by reference, "
          "throwing a fatal error (attr:0x%x)\n",
          func->name(), func->m_numParams, paramId,
          func->isBuiltin() ? func->m_info->attribute : 0);
    raise_error("Cannot pass parameter %d by reference", paramId+1);
  }
}

#undef FPASSC_CHECKED_PRELUDE

inline void ALWAYS_INLINE ExecutionContext::iopFPassV(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
  const Func* func = ar->m_func;
  if (!func->byRef(paramId)) {
    m_stack.unbox();
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassR(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
  const Func* func = ar->m_func;
  if (func->byRef(paramId)) {
    TypedValue* tv = m_stack.topTV();
    if (tv->m_type != KindOfVariant) {
      tvBox(tv);
    }
  } else {
    if (m_stack.topTV()->m_type == KindOfVariant) {
      m_stack.unbox();
    }
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassH(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
  if (!ar->m_func->byRef(paramId)) {
    iopCGetH(origPc);
  } else {
    iopVGetH(origPc);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassN(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
  if (!ar->m_func->byRef(paramId)) {
    iopCGetN(origPc);
  } else {
    iopVGetN(origPc);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassG(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
  if (!ar->m_func->byRef(paramId)) {
    iopCGetG(origPc);
  } else {
    iopVGetG(origPc);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassS(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  PC origPc = pc;
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
  if (!ar->m_func->byRef(paramId)) {
    iopCGetS(origPc);
  } else {
    iopVGetS(origPc);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFPassM(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(paramId);
  ASSERT(paramId < ar->m_numArgs);
  if (!ar->m_func->byRef(paramId)) {
    DECLARE_GETHELPER_ARGS
    getHelper(GETHELPER_ARGS);
    if (tvL->m_type == KindOfVariant) {
      tvUnbox(tvL);
    }
  } else {
    DECLARE_SETHELPER_ARGS
    TypedValue* tv1 = m_stack.allocTV();
    tvWriteUninit(tv1);
    if (!setHelperPre<false, true, false, 1, 0>(SETHELPERPRE_ARGS)) {
      if (base->m_type != KindOfVariant) {
        tvBox(base);
      }
      tvDupVar(base, tv1);
    } else {
      tvWriteNull(tv1);
      tvBox(tv1);
    }
    setHelperPost<1>(SETHELPERPOST_ARGS);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopFCall(PC& pc) {
  ActRec* ar = arFromInstr(m_stack.top(), (Opcode*)pc);
  NEXT();
  DECODE_IVA(numArgs);
  ASSERT(numArgs == ar->m_numArgs);
  arSetSfp(ar, m_fp);
  ar->m_savedRip = (uintptr_t)m_transl->getRetFromInterpretedFrame();
  // Strictly greater than start address of calling function; first
  // instruction can't be fcall.
  TRACE(3, "FCall: pc %p func %p base %d\n", m_pc,
        m_fp->m_func->m_unit->entry(),
        int(m_fp->m_func->m_base));
  ar->m_soff = m_fp->m_func->m_unit->offsetOf(pc)
    - (uintptr_t)m_fp->m_func->m_base;
  if (ar->m_func->isBuiltin()) {
    SYNC();
    callBuiltin(ar);
  } else {
    ASSERT(pcOff() > m_fp->m_func->m_base);
    prepareNonBuiltinEntry<true>(ar, pc);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopIterInit(PC& pc) {
  PC origPc = pc;
  NEXT();
  DECODE_IVA(itId);
  DECODE(Offset, offset);
  Cell* c1 = m_stack.topC();
  if (c1->m_type == KindOfArray) {
    if (!c1->m_data.parr->empty()) {
      Iter* it = FP2ITER(m_fp, itId);
      (void) new (&it->arr()) ArrayIter(c1->m_data.parr); // call CTor
      it->m_itype = Iter::TypeArray;
    } else {
      ITER_SKIP(offset);
    }
  } else if (c1->m_type == KindOfObject) {
    PreClass* ctx = arGetContextPreClass(m_fp);
    Iter* it = FP2ITER(m_fp, itId);
    CStrRef ctxStr = ctx ? CStrRef(ctx->m_name) : null_string;
    bool isIterator;
    Object obj = c1->m_data.pobj->iterableObject(isIterator);
    if (isIterator) {
      (void) new (&it->arr()) ArrayIter(obj.get());
    } else {
      Array iterArray(obj->o_toIterArray(ctxStr));
      ArrayData* ad = iterArray.getArrayData();
      (void) new (&it->arr()) ArrayIter(ad);
    }
    if (it->arr().end()) {
      // Iterator was empty; branch to done case
      ITER_SKIP(offset);
    } else {
      it->m_itype = (isIterator ? Iter::TypeIterator : Iter::TypeArray);
    }
  } else {
    raise_warning("Invalid argument supplied for foreach()");
    ITER_SKIP(offset);
  }
  m_stack.popC();
}

inline void ALWAYS_INLINE ExecutionContext::iopIterInitM(PC& pc) {
  PC origPc = pc;
  NEXT();
  DECODE_IVA(itId);
  DECODE(Offset, offset);
  Var* v1 = m_stack.topV();
  tvAsVariant(v1).escalate(true);
  if (v1->m_data.ptv->m_type == KindOfArray) {
    ArrayData* ad = v1->m_data.ptv->m_data.parr;
    if (!ad->empty()) {
      Iter* it = FP2ITER(m_fp, itId);
      MIterCtx& mi = it->marr();
      if (ad->getCount() > 1) {
        ArrayData* copy = ad->copy();
        copy->incRefCount();
        ad->decRefCount();  // count > 1 to begin with; don't need release
        ad = v1->m_data.ptv->m_data.parr = copy;
      }
      (void) new (&mi) MIterCtx((const Variant*)v1->m_data.ptv);
      it->m_itype = Iter::TypeMutableArray;
      mi.m_mArray->advance();
    } else {
      ITER_SKIP(offset);
    }
  } else if (v1->m_data.ptv->m_type == KindOfObject)  {
    PreClass* ctx = arGetContextPreClass(m_fp);
    CStrRef ctxStr = ctx ? CStrRef(ctx->m_name) : null_string;

    bool isIterator;
    Object obj = v1->m_data.ptv->m_data.pobj->iterableObject(isIterator);
    if (isIterator) {
      raise_error("An iterator cannot be used with foreach by reference");
    }
    Array iterArray = obj->o_toIterArray(ctxStr, true);
    ArrayData* ad = iterArray.getArrayData();
    if (ad->empty()) {
      ITER_SKIP(offset);
    } else {
      if (ad->getCount() > 1) {
        ArrayData* copy = ad->copy();
        copy->incRefCount();
        ad->decRefCount();  // count > 1 to begin with; don't need release
        ad = v1->m_data.ptv->m_data.parr = copy;
      }
      Iter* it = FP2ITER(m_fp, itId);
      MIterCtx& mi = it->marr();
      (void) new (&mi) MIterCtx(ad);
      mi.m_mArray->advance();
      it->m_itype = Iter::TypeMutableArray;
    }
  } else {
    raise_warning("Invalid argument supplied for foreach()");
    ITER_SKIP(offset);
  }
  m_stack.popV();
}

inline void ALWAYS_INLINE ExecutionContext::iopIterValueC(PC& pc) {
  NEXT();
  DECODE_IVA(itId);
  Iter* it = FP2ITER(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The emitter should never generate bytecode where the iterator
    // is at the end before IterValueC is executed. However, even if
    // the iterator is at the end, it is safe to call second().
    Cell* c1 = m_stack.allocC();
    TV_WRITE_NULL(c1);
    tvCellAsVariant(c1) = it->arr().second();
    break;
  }
  case Iter::TypeMutableArray: {
    // Dup value.
    TypedValue* tv1 = m_stack.allocTV();
    tvDup(&it->marr().m_val, tv1);
    ASSERT(tv1->m_type == KindOfVariant);
    tvUnbox(tv1);
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopIterValueV(PC& pc) {
  NEXT();
  DECODE_IVA(itId);
  Iter* it = FP2ITER(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The emitter should never generate bytecode where the iterator
    // is at the end before IterValueV is executed. However, even if
    // the iterator is at the end, it is safe to call secondRef().
    TypedValue* tv = m_stack.allocTV();
    TV_WRITE_NULL(tv);
    tvAsVariant(tv) = ref(it->arr().secondRef());
    break;
  }
  case Iter::TypeMutableArray: {
    // Dup value.
    TypedValue* tv1 = m_stack.allocTV();
    tvDup(&it->marr().m_val, tv1);
    ASSERT(tv1->m_type == KindOfVariant);
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopIterKey(PC& pc) {
  NEXT();
  DECODE_IVA(itId);
  Iter* it = FP2ITER(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The iterator should never be at the end here. We can't check for it,
    // because that may call into user code, which has PHP-visible effects and
    // is incorrect.
    Cell* c1 = m_stack.allocC();
    TV_WRITE_NULL(c1);
    tvCellAsVariant(c1) = it->arr().first();
    break;
  }
  case Iter::TypeMutableArray: {
    // Dup key.
    TypedValue* tv1 = m_stack.allocTV();
    tvDup(&it->marr().m_key, tv1);
    if (tv1->m_type == KindOfVariant) {
      tvUnbox(tv1);
    }
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopIterNext(PC& pc) {
  PC origPc = pc;
  NEXT();
  DECODE_IVA(itId);
  DECODE(Offset, offset);
  Iter* it = FP2ITER(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    // The emitter should never generate bytecode where the iterator
    // is at the end before IterNext is executed. However, even if
    // the iterator is at the end, it is safe to call next().
    if (iter_next_array(it)) {
      // If after advancing the iterator we have not reached the end,
      // jump to the location specified by the second immediate argument.
      ITER_SKIP(offset);
    } else {
      // If after advancing the iterator we have reached the end, free
      // the iterator and fall through to the next instruction.
      ASSERT(it->m_itype == Iter::TypeUndefined);
    }
    break;
  }
  case Iter::TypeMutableArray: {
    MIterCtx &mi = it->marr();
    if (!mi.m_mArray->advance()) {
      // If after advancing the iterator we have reached the end, free
      // the iterator and fall through to the next instruction.
      tvRefcountedDecRef(&mi.m_key);
      tvRefcountedDecRef(&mi.m_val);
      mi.~MIterCtx();
      it->m_itype = Iter::TypeUndefined;
    } else {
      // If after advancing the iterator we have not reached the end,
      // jump to the location specified by the second immediate argument.
      ITER_SKIP(offset);
    }
    break;
  }
  default: {
    not_reached();
  }
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopIterFree(PC& pc) {
  NEXT();
  DECODE_IVA(itId);
  Iter* it = FP2ITER(m_fp, itId);
  switch (it->m_itype) {
  case Iter::TypeUndefined: {
    ASSERT(false);
    break;
  }
  case Iter::TypeArray:
  case Iter::TypeIterator: {
    it->arr().~ArrayIter();
    break;
  }
  case Iter::TypeMutableArray: {
    MIterCtx &mi = it->marr();
    tvRefcountedDecRef(&mi.m_key);
    tvRefcountedDecRef(&mi.m_val);
    mi.~MIterCtx();
    break;
  }
  default: {
    not_reached();
  }
  }
  it->m_itype = Iter::TypeUndefined;
}

#define INCLOP(raise_type, once_check) do {                                   \
  NEXT();                                                                     \
  Cell* c1 = m_stack.topC();                                                  \
  StringData* path = prepareKey(c1);                                          \
  bool initial;                                                               \
  Unit* u = evalInclude(path, m_fp->m_func->m_unit->m_filepath, initial);     \
  m_stack.popC();                                                             \
  if (u == NULL) {                                                            \
    raise_##raise_type("File not found: %s", path->data());                   \
    m_stack.pushFalse();                                                      \
  } else {                                                                    \
    if (once_check) {                                                         \
      evalUnit(u, pc);                                                        \
    } else {                                                                  \
      m_stack.pushTrue();                                                     \
    }                                                                         \
  }                                                                           \
  LITSTR_DECREF(path);                                                        \
} while (0)

inline void ALWAYS_INLINE ExecutionContext::iopIncl(PC& pc) {
  INCLOP(warning, true);
}

inline void ALWAYS_INLINE ExecutionContext::iopInclOnce(PC& pc) {
  INCLOP(warning, initial);
}

inline void ALWAYS_INLINE ExecutionContext::iopReq(PC& pc) {
  INCLOP(error, true);
}

inline void ALWAYS_INLINE ExecutionContext::iopReqOnce(PC& pc) {
  INCLOP(error, initial);
}
#undef INCLOP

inline void ALWAYS_INLINE ExecutionContext::iopEval(PC& pc) {
  NEXT();
  Cell* c1 = m_stack.topC();
  StringData* code = prepareKey(c1);
  String prefixedCode = concat("<?php ", code);
  Unit* unit = compile_string(prefixedCode->data(), prefixedCode->size(), NULL);
  if (unit == NULL) {
    raise_error("Syntax error in eval()");
  }
  m_evaledUnits.push_back(unit);
  m_stack.popC();
  evalUnit(unit, pc);
  LITSTR_DECREF(code);
}

inline void ALWAYS_INLINE ExecutionContext::iopDefFunc(PC& pc) {
  NEXT();
  DECODE_IVA(fid);
  Func* f = m_fp->m_func->m_unit->m_funcs[fid];
  Func* oldFunc = m_funcDict.get(f->m_name);
  if (oldFunc) {
    if (oldFunc != f) {
      raise_error("Function already defined: %s", f->m_name->data());
    }
  } else {
    m_funcDict.insert(f->m_name, f);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopDefCls(PC& pc) {
  NEXT();
  DECODE_IVA(cid);
  PreClass* c = m_fp->m_func->m_unit->m_preClasses[cid].get();
  defClass(c);
}

inline void ALWAYS_INLINE ExecutionContext::iopThis(PC& pc) {
  NEXT();
  if (!m_fp->hasThis()) {
    raise_error("$this is null");
  }
  ObjectData* this_ = m_fp->getThis();
  m_stack.pushObject(this_);
}

inline void ALWAYS_INLINE ExecutionContext::iopInitThisLoc(PC& pc) {
  NEXT();
  DECODE_IVA(id);
  TypedValue* thisLoc = frame_local(m_fp, id);
  tvRefcountedDecRef(thisLoc);
  if (m_fp->hasThis()) {
    thisLoc->m_data.pobj = m_fp->getThis();
    thisLoc->_count = 0;
    thisLoc->m_type = KindOfObject;
    tvIncRef(thisLoc);
  } else {
    TV_WRITE_UNINIT(thisLoc);
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopStaticLoc(PC& pc) {
  NEXT();
  DECODE_IVA(localId);
  DECODE(Id, litstrId);
  StringData* var = m_fp->m_func->m_unit->lookupLitstrId(litstrId);
  TypedValue* fr = NULL;
  bool inited;
  LOOKUPD_STATIC(var, fr, inited);
  ASSERT(fr != NULL);
  if (fr->m_type != KindOfVariant) {
    ASSERT(!inited);
    tvBox(fr);
  }
  TypedValue* tvLocal = frame_local(m_fp, localId);
  tvBind(fr, tvLocal);
  if (inited) {
    m_stack.pushTrue();
  } else {
    m_stack.pushFalse();
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopStaticLocInit(PC& pc) {
  NEXT();
  DECODE_IVA(localId);
  DECODE(Id, litstrId);
  StringData* var = m_fp->m_func->m_unit->lookupLitstrId(litstrId);
  TypedValue* fr = NULL;
  bool inited;
  LOOKUPD_STATIC(var, fr, inited);
  ASSERT(fr != NULL);
  if (!inited) {
    Cell* initVal = m_stack.topC();
    tvDup(initVal, fr);
  }
  if (fr->m_type != KindOfVariant) {
    ASSERT(!inited);
    tvBox(fr);
  }
  TypedValue* tvLocal = frame_local(m_fp, localId);
  tvBind(fr, tvLocal);
  m_stack.discard();
}

inline void ALWAYS_INLINE ExecutionContext::iopCatch(PC& pc) {
  NEXT();
  m_stack.pushNull();
  Fault fault = m_faults.back();
  m_faults.pop_back();
  ASSERT(fault.m_userException.m_type == KindOfObject);
  memcpy(m_stack.top(), &fault.m_userException, sizeof(TypedValue));
}

inline void ALWAYS_INLINE ExecutionContext::iopLateBoundCls(PC& pc) {
  NEXT();
  m_stack.pushNull();
  TypedValue* tv = m_stack.top();
  const Class* class_ = NULL;
  if (m_fp->hasThis()) {
    class_ = m_fp->getThis()->getVMClass();
  } else if (m_fp->hasClass()) {
    class_ = m_fp->getClass();
  } else {
    ASSERT(false);
  }
  tv->m_data.pcls = const_cast<Class*>(class_);
  tv->_count = 0;
  tv->m_type = KindOfClass;
}

inline void ALWAYS_INLINE ExecutionContext::iopVerifyParamType(PC& pc) {
  SYNC(); // We might need m_pc to be updated to throw.
  NEXT();
  DECODE_IVA(param);
  const Func *func = m_fp->m_func;
  ASSERT(param < func->m_numParams);
  ASSERT(func->m_numParams == int(func->m_params.size()));
  const TypeConstraint& tc = func->m_params[param].m_typeConstraint;
  ASSERT(tc.exists());
  const TypedValue *tv = frame_local(m_fp, param);
  ExecutionContext* ec = g_context.getNoCheck();
  using namespace std::tr1;
  if (UNLIKELY(!tc.check(tv,
                         bind(&ExecutionContext::loadClass,
                              ec, placeholders::_1)))) {
    std::ostringstream fname;
    if (func->m_preClass != NULL) {
      fname << func->m_preClass->m_name->data() << "::"
            << func->m_name->data() << "()";
    } else {
      fname << func->m_name->data() << "()";
    }
    throw_unexpected_argument_type(param + 1, fname.str().c_str(),
                                   tc.typeName(), tvAsCVarRef(tv));
  }
}

inline void ALWAYS_INLINE ExecutionContext::iopHighInvalid(PC& pc) {
  fprintf(stderr, "invalid bytecode executed\n");
  abort();
}

string
ExecutionContext::prettyStack(const string& prefix) const {
  if (m_halted) {
    string s("__Halted");
    return s;
  }
  int offset = (m_fp->m_func->m_unit != NULL)
               ? pcOff()
               : 0;
  string begPrefix = prefix + "__";
  string midPrefix = prefix + "|| ";
  string endPrefix = prefix + "\\/";
  string stack = m_stack.toString(m_fp, offset, midPrefix);
  return begPrefix + "\n" + stack + endPrefix;
}

void ExecutionContext::DumpStack() {
  string s = g_context->prettyStack("");
  fprintf(stderr, "%s\n", s.c_str());
}

#define COND_STACKTRACE_SEP(pfx)                                              \
  ONTRACE(3,                                                                  \
          Trace::trace("%s"                                                   \
                       "========================================"             \
                       "========================================\n",          \
                       pfx);)
#define COND_STACKTRACE(pfx)                                                  \
  ONTRACE(3,                                                                  \
          string stack = prettyStack(pfx);                                    \
          Trace::trace("%s\n", stack.c_str());)

#define O(name, imm, pusph, pop, flags)                                       \
void ExecutionContext::op##name() {                                           \
  COND_STACKTRACE_SEP("op"#name" ");                                          \
  COND_STACKTRACE("op"#name" pre:  ");                                        \
  PC pc = m_pc;                                                               \
  ASSERT(*pc == Op##name);                                                    \
  ONTRACE(1,                                                                  \
          int offset = m_fp->m_func->m_unit->offsetOf(pc);                    \
          Trace::trace("op"#name" offset: %d\n", offset));                    \
  iop##name(pc);                                                              \
  SYNC();                                                                     \
  COND_STACKTRACE("op"#name" post: ");                                        \
  COND_STACKTRACE_SEP("op"#name" ");                                          \
}
OPCODES
#undef O
#undef NEXT
#undef DECODE_JMP
#undef DECODE

template <bool limInstrs, bool breakOnCtlFlow>
inline void ExecutionContext::dispatchImpl(int numInstrs) {
  static const void *optabDirect[] = {
#define O(name, imm, push, pop, flags) \
    &&Label##name,
    OPCODES
#undef O
  };
  static const void *optabDbg[] = {
#define O(name, imm, push, pop, flags) \
    &&LabelDbg##name,
    OPCODES
#undef O
  };
  static const void *optabInst[] __attribute__((unused)) = {
#define O(name, imm, push, pop, flags) \
    &&LabelInst##name,
    OPCODES
#undef O
  };
  ASSERT(sizeof(optabDirect) / sizeof(const void *) == Op_count);
  ASSERT(sizeof(optabDbg) / sizeof(const void *) == Op_count);
  const void **optab = optabDirect;
  InjectionTableInt64* injTable = g_context->m_injTables ?
    g_context->m_injTables->getInt64Table(InstHookTypeBCPC) : NULL;
  if (injTable) {
    optab = optabInst;
  }
  DEBUGGER_ATTACHED_ONLY(optab = optabDbg);
  /*
   * Trace-only mapping of opcodes to names.
   */
#ifdef HPHP_TRACE
  static const char *nametab[] = {
#define O(name, imm, push, pop, flags) \
    #name,
    OPCODES
#undef O
  };
#endif /* HPHP_TRACE */
  bool isCtlFlow = false;

#define DISPATCH() do {                                                       \
    if ((breakOnCtlFlow && isCtlFlow) ||                                      \
        (limInstrs && UNLIKELY(numInstrs-- == 0))) {                          \
      ONTRACE(1,                                                              \
              Trace::trace("dispatch: Halt ExecutionContext::dispatch(%p)\n", \
                           m_fp));                                            \
      return;                                                                 \
    }                                                                         \
    Op op = (Op)*pc;                                                          \
    COND_STACKTRACE("dispatch:                    ");                         \
    ONTRACE(1,                                                                \
            Trace::trace("dispatch: %d: %s\n", pcOff(), nametab[op]));        \
    ASSERT(op < Op_count);                                                    \
    goto *optab[op];                                                          \
} while (0)

  ONTRACE(1, Trace::trace("dispatch: Enter ExecutionContext::dispatch(%p)\n",
          m_fp));
  PC pc = m_pc;
  DISPATCH();

#define O(name, imm, pusph, pop, flags)                                       \
  LabelDbg##name:                                                             \
    phpDebuggerHook();                                                        \
  LabelInst##name:                                                            \
    INST_HOOK_PC(injTable, pc);                                               \
  Label##name: {                                                              \
    iop##name(pc);                                                            \
    SYNC();                                                                   \
    if (g_context->m_halted) {                                                \
      return;                                                                 \
    }                                                                         \
    if (breakOnCtlFlow) isCtlFlow = instrIsControlFlow(Op##name);             \
    DISPATCH();                                                               \
  }
  OPCODES
#undef O
#undef DISPATCH
}

void ExecutionContext::dispatch() {
  dispatchImpl<false, false>(0);
}

void ExecutionContext::dispatchN(int numInstrs) {
  dispatchImpl<true, false>(numInstrs);
}

void ExecutionContext::dispatchBB() {
  dispatchImpl<false, true>(0);
}

void ExecutionContext::reenterVM(VMState &savedVM) {
  if (debug && savedVM.fp && savedVM.fp->m_func->m_unit) {
    // Some asserts and tracing.
    const Func* func = savedVM.fp->m_func;
    (void) /* bound-check asserts in offsetOf */
      func->m_unit->offsetOf(savedVM.pc);
    TRACE(3, "reenterVM: saving frame %s pc %p off %d fp %p\n",
          func->m_name->data(),
          savedVM.pc,
          func->m_unit->offsetOf(savedVM.pc),
          savedVM.fp);
  }
  m_nestedVMs.push_back(savedVM);
  m_nesting++;
}

void ExecutionContext::exitVM() {
  ASSERT(m_nestedVMs.size() >= 1);
  if (debug) {
    const VMState& savedVM = m_nestedVMs.back();
    if (savedVM.fp && savedVM.fp->m_func->m_unit) {
      const Func* func = savedVM.fp->m_func;
      (void) /* bound-check asserts in offsetOf */
        func->m_unit->offsetOf(savedVM.pc);
      TRACE(3, "exitVM: restoring frame %s pc %p off %d fp %p\n",
            func->m_name->data(),
            savedVM.pc,
            func->m_unit->offsetOf(savedVM.pc),
            savedVM.fp);
    }
  }
  m_nestedVMs.pop_back();
  m_nesting--;
}

void ExecutionContext::requestInit() {
  const_assert(hhvm);

  // Merge the systemlib unit into the ExecutionContext
  ASSERT(SystemLib::s_unit);
  mergeUnit(SystemLib::s_unit);

#ifdef DEBUG
  ExecutionContext::DefinedClassMap::const_iterator it =
    m_definedClasses.find(s_stdclass.get());
  ASSERT(it != m_definedClasses.end());
  ASSERT(it->second == SystemLib::s_stdclassClass);
#endif

  ASSERT(m_staticVars == NULL);
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  m_staticVars = dynamic_cast<HphpArray*>(g->hg_static_storage.get());
  ASSERT(m_staticVars != NULL);

  m_transl->requestInit();
}

void ExecutionContext::requestExit() {
  const_assert(hhvm);
  destructObjects();
  m_transl->requestExit();
  DynTracer::Disable();
}

///////////////////////////////////////////////////////////////////////////////
}
