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

#include "hphp/runtime/vm/jit/translator-runtime.h"

#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const StaticString s_staticPrefix("86static_");

// Defined here so it can be inlined below.
RefData* lookupStaticFromClosure(ObjectData* closure,
                                 StringData* name,
                                 bool& inited) {
  assert(closure->instanceof(c_Closure::classof()));
  String str(StringData::Make(s_staticPrefix->slice(), name->slice()));
  auto const cls = closure->getVMClass();
  auto const slot = cls->lookupDeclProp(str.get());
  assert(slot != kInvalidSlot);
  auto const val = static_cast<c_Closure*>(closure)->getStaticVar(slot);

  if (val->m_type == KindOfUninit) {
    inited = false;
    val->m_type = KindOfNull;
    tvBox(val);
    return val->m_data.pref;
  }
  inited = true;
  assert(val->m_type == KindOfRef);
  return val->m_data.pref;
}

namespace Transl {

//////////////////////////////////////////////////////////////////////

ArrayData* addElemIntKeyHelper(ArrayData* ad,
                               int64_t key,
                               TypedValue value) {
  // this does not re-enter
  // set will decRef any old value that may have been overwritten
  // if appropriate
  ArrayData* retval = ad->set(key, tvAsCVarRef(&value),
                              ad->getCount() > 1);
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  tvRefcountedDecRef(&value);
  return arrayRefShuffle<false>(ad, retval, nullptr);
}

ArrayData* addElemStringKeyHelper(ArrayData* ad,
                                  StringData* key,
                                  TypedValue value) {
  // this does not re-enter
  bool copy = ad->getCount() > 1;
  // set will decRef any old value that may have been overwritten
  // if appropriate
  int64_t intkey;
  ArrayData* retval = UNLIKELY(key->isStrictlyInteger(intkey)) ?
                      ad->set(intkey, tvAsCVarRef(&value), copy) :
                      ad->set(key, tvAsCVarRef(&value), copy);
  // TODO Task #1970153: It would be great if there were set()
  // methods that didn't bump up the refcount so that we didn't
  // have to decrement it here
  tvRefcountedDecRef(&value);
  return arrayRefShuffle<false>(ad, retval, nullptr);
}

ArrayData* array_add(ArrayData* a1, ArrayData* a2) {
  if (!a2->empty()) {
    if (a1->empty()) {
      decRefArr(a1);
      return a2;
    }
    if (a1 != a2) {
      ArrayData *escalated = a1->plus(a2, a1->getCount() > 1);
      if (escalated != a1) {
        escalated->incRefCount();
        decRefArr(a2);
        decRefArr(a1);
        return escalated;
      }
    }
  }
  decRefArr(a2);
  return a1;
}

HOT_FUNC_VM void setNewElem(TypedValue* base, Cell val) {
  SetNewElem<false>(base, &val);
}

HOT_FUNC_VM void setNewElemArray(TypedValue* base, Cell val) {
  SetNewElemArray(base, &val);
}

void bindNewElemIR(TypedValue* base, RefData* val, MInstrState* mis) {
  base = NewElem(mis->tvScratch, mis->tvRef, base);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBindRef(val, base);
  }
}

// TODO: Kill this #2031980
HOT_FUNC_VM RefData* box_value(TypedValue tv) {
  return tvBoxHelper(tv.m_type, tv.m_data.num);
}

inline int64_t reinterpretDblAsInt(double d) {
  union {
    int64_t intval;
    double dblval;
  } u;
  u.dblval = d;
  return u.intval;
}

inline double reinterpretIntAsDbl(int64_t i) {
  union {
    int64_t intval;
    double dblval;
  } u;
  u.intval = i;
  return u.dblval;
}

ArrayData* convCellToArrHelper(TypedValue tv) {
  // Note: the call sites of this function all assume that
  // no user code will run and no recoverable exceptions will
  // occur while running this code. This seems trivially true
  // in all cases but converting objects to arrays. It also
  // seems true for that case as well, since the resulting array
  // is essentially metadata for the object. If that is not true,
  // you might end up looking at this code in a debugger and now
  // you know why.
  tvCastToArrayInPlace(&tv); // consumes a ref on counted values
  return tv.m_data.parr;
}

int64_t convArrToBoolHelper(const ArrayData* a) {
  return a->size() != 0;
}

int64_t convObjToBoolHelper(const ObjectData* o) {
  return o->o_toBoolean();
}

int64_t convArrToDblHelper(ArrayData* a) {
  return reinterpretDblAsInt(a->empty() ? 0 : 1);
}

int64_t convStrToDblHelper(const StringData* s) {
  return reinterpretDblAsInt(s->toDouble());
}

int64_t convCellToDblHelper(TypedValue tv) {
  return reinterpretDblAsInt(tvCastToDouble(&tv));
}

int64_t convArrToIntHelper(ArrayData* a) {
  return a->empty() ? 0 : 1;
}

ObjectData* convCellToObjHelper(TypedValue tv) {
  // Note: the call sites of this function all assume that
  // no user code will run and no recoverable exceptions will
  // occur while running this code. This seems trivially true
  // in all cases but converting arrays to objects. It also
  // seems true for that case as well, since the source array
  // is essentially metadata for the object. If that is not true,
  // you might end up looking at this code in a debugger and now
  // you know why.
  tvCastToObjectInPlace(&tv); // consumes a ref on counted values
  return tv.m_data.pobj;
}

StringData* convDblToStrHelper(int64_t i) {
  double d = reinterpretIntAsDbl(i);
  auto r = buildStringData(d);
  r->incRefCount();
  return r;
}

StringData* convIntToStrHelper(int64_t i) {
  auto r = buildStringData(i);
  r->incRefCount();
  return r;
}

StringData* convObjToStrHelper(ObjectData* o) {
  auto s = o->invokeToString();
  auto r = s.get();
  if (!r->isStatic()) r->incRefCount();
  return r;
}

StringData* convResToStrHelper(ResourceData* o) {
  auto s = o->o_toString();
  auto r = s.get();
  if (!r->isStatic()) r->incRefCount();
  return r;
}

const StaticString
  s_empty(""),
  s_1("1"),
  s_Array("Array");

StringData* convCellToStrHelper(TypedValue tv) {
  switch (tv.m_type) {
  case KindOfUninit:
  case KindOfNull:     return s_empty.get();
  case KindOfBoolean:  return tv.m_data.num ? s_1.get() : s_empty.get();
  case KindOfInt64:    return convIntToStrHelper(tv.m_data.num);
  case KindOfDouble:   return convDblToStrHelper(tv.m_data.num);
  case KindOfString:   tv.m_data.pstr->incRefCount();
                       /* fallthrough */
  case KindOfStaticString:
                       return tv.m_data.pstr;
  case KindOfArray:    return s_Array.get();
  case KindOfObject:   return convObjToStrHelper(tv.m_data.pobj);
  case KindOfResource: return convResToStrHelper(tv.m_data.pres);
  default:             not_reached();
  }
}

void raisePropertyOnNonObject() {
  raise_warning("Cannot access property on non-object");
}

void raiseUndefProp(ObjectData* base, const StringData* name) {
  base->raiseUndefProp(name);
}

void raise_error_sd(const StringData *msg) {
  raise_error("%s", msg->data());
}

void VerifyParamTypeFail(int paramNum) {
  VMRegAnchor _;
  const ActRec* ar = liveFrame();
  const Func* func = ar->m_func;
  const TypeConstraint& tc = func->params()[paramNum].typeConstraint();
  TypedValue* tv = frame_local(ar, paramNum);
  assert(!tc.check(tv, func));
  tc.verifyFail(func, paramNum, tv);
}

void VerifyParamTypeCallable(TypedValue value, int param) {
  if (UNLIKELY(!f_is_callable(tvAsCVarRef(&value)))) {
    VerifyParamTypeFail(param);
  }
}

HOT_FUNC_VM
void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         int param,
                         const TypeConstraint* expected) {
  if (LIKELY(constraint && cls->classof(constraint))) {
    return;
  }

  // Check a typedef for a class.  We interp'd if the param wasn't an
  // object, so if it's a typedef for something non-objecty we're
  // failing anyway.
  if (auto namedEntity = expected->namedEntity()) {
    auto def = namedEntity->getCachedTypedef();
    if (UNLIKELY(!def)) {
      VMRegAnchor _;
      String nameStr(const_cast<StringData*>(expected->typeName()));
      if (AutoloadHandler::s_instance->autoloadType(nameStr)) {
        def = namedEntity->getCachedTypedef();
      }
    }
    if (def) {
      // There's no need to handle nullable typedefs specially here:
      // we already know we're checking a non-null object with the
      // class `cls'.  We do however need to check for typedefs to
      // mixed.
      if (def->kind == KindOfObject) {
        constraint = def->klass;
        if (constraint && cls->classof(constraint)) return;
      } else if (def->kind == KindOfAny) {
        return;
      }
    }
  }

  VerifyParamTypeFail(param);
}

RefData* closureStaticLocInit(StringData* name, ActRec* fp, TypedValue val) {
  auto const func = fp->m_func;
  assert(func->isClosureBody() || func->isGeneratorFromClosure());
  auto const closureLoc =
    LIKELY(func->isClosureBody())
      ? frame_local(fp, func->numParams())
      : frame_local(fp, frame_continuation(fp)->m_origFunc->numParams());

  bool inited;
  auto const refData = lookupStaticFromClosure(
    closureLoc->m_data.pobj, name, inited);
  if (!inited) {
    cellCopy(val, *refData->tv());
  }
  refData->incRefCount();
  return refData;
}

HOT_FUNC_VM
bool instanceOfHelper(const Class* objClass,
                      const Class* testClass) {
  return testClass && objClass->classof(testClass);
}

ALWAYS_INLINE
static int64_t ak_exist_string_impl(ArrayData* arr, StringData* key) {
  int64_t n;
  if (key->isStrictlyInteger(n)) {
    return arr->exists(n);
  }
  return arr->exists(key);
}

HOT_FUNC_VM
int64_t ak_exist_string(ArrayData* arr, StringData* key) {
  return ak_exist_string_impl(arr, key);
}

HOT_FUNC_VM
int64_t ak_exist_int(ArrayData* arr, int64_t key) {
  bool res = arr->exists(key);
  return res;
}

HOT_FUNC_VM
int64_t ak_exist_string_obj(ObjectData* obj, StringData* key) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  int64_t res = ak_exist_string_impl(arr.get(), key);
  return res;
}

HOT_FUNC_VM
int64_t ak_exist_int_obj(ObjectData* obj, int64_t key) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  bool res = arr.get()->exists(key);
  return res;
}

ALWAYS_INLINE
TypedValue& getDefaultIfNullCell(TypedValue* tv, TypedValue& def) {
  if (UNLIKELY(nullptr == tv)) {
    // refcount is already correct since def was never decrefed
    return def;
  }
  tvRefcountedDecRef(&def);
  TypedValue* ret = tvToCell(tv);
  tvRefcountedIncRef(ret);
  return *ret;
}

HOT_FUNC_VM
TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue arrayIdxSi(ArrayData* a, StringData* key, TypedValue def) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ?
         getDefaultIfNullCell(a->nvGet(i), def) :
         getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue arrayIdxI(ArrayData* a, int64_t key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue* ldGblAddrHelper(StringData* name) {
  return g_vmContext->m_globalVarEnv->lookup(name);
}

HOT_FUNC_VM
TypedValue* ldGblAddrDefHelper(StringData* name) {
  TypedValue* r = g_vmContext->m_globalVarEnv->lookupAdd(name);
  decRefStr(name);
  return r;
}

TCA sswitchHelperFast(const StringData* val,
                      const SSwitchMap* table,
                      TCA* def) {
  TCA* dest = table->find(val);
  return dest ? *dest : *def;
}

// TODO(#2031980): clear these out
void tv_release_generic(TypedValue* tv) {
  assert(Transl::tx64->stateIsDirty());
  assert(tv->m_type == KindOfString || tv->m_type == KindOfArray ||
         tv->m_type == KindOfObject || tv->m_type == KindOfResource ||
         tv->m_type == KindOfRef);
  g_destructors[typeToDestrIndex(tv->m_type)](tv->m_data.pref);
}

void tv_release_typed(RefData* pv, DataType dt) {
  assert(Transl::tx64->stateIsDirty());
  assert(dt == KindOfString || dt == KindOfArray ||
         dt == KindOfObject || dt == KindOfResource ||
         dt == KindOfRef);
  g_destructors[typeToDestrIndex(dt)](pv);
}

Cell lookupCnsHelper(const TypedValue* tv,
                     StringData* nm,
                     bool error) {
  assert(tv->m_type == KindOfUninit);

  // Deferred constants such as SID
  if (UNLIKELY(tv->m_data.pref != nullptr)) {
    ClassInfo::ConstantInfo* ci =
      (ClassInfo::ConstantInfo*)(void*)tv->m_data.pref;
    Cell *cns = const_cast<Variant&>(ci->getDeferredValue()).asTypedValue();
    if (LIKELY(cns->m_type != KindOfUninit)) {
      Cell c1;
      cellDup(*cns, c1);
      return c1;
    }
  }

  Cell *cns = nullptr;
  if (UNLIKELY(TargetCache::s_constants().get() != nullptr)) {
    cns = TargetCache::s_constants()->nvGet(nm);
  }
  if (!cns) {
    cns = Unit::loadCns(const_cast<StringData*>(nm));
  }
  if (LIKELY(cns != nullptr)) {
    Cell c1;
    c1.m_type = cns->m_type;
    c1.m_data = cns->m_data;
    return c1;
  }

  // Undefined constants
  if (error) {
    raise_error("Undefined constant '%s'", nm->data());
  } else {
    raise_notice(Strings::UNDEFINED_CONSTANT, nm->data(), nm->data());
    Cell c1;
    c1.m_data.pstr = const_cast<StringData*>(nm);
    c1.m_type = KindOfStaticString;
    return c1;
  }
  not_reached();
}

Cell lookupCnsUHelper(const TypedValue* tv,
                      StringData* nm,
                      StringData* fallback) {
  Cell *cns = nullptr;
  Cell c1;

  // lookup qualified name in thread-local constants
  bool cacheConsts = TargetCache::s_constants().get() != nullptr;
  if (UNLIKELY(cacheConsts)) {
    cns = TargetCache::s_constants()->nvGet(nm);
  }
  if (!cns) {
    cns = Unit::loadCns(const_cast<StringData*>(nm));
  }

  // try cache handle for unqualified name
  if (UNLIKELY(!cns && tv->m_type != KindOfUninit)) {
    cns = const_cast<Cell*>(tv);
  }

  // lookup unqualified name in thread-local constants
  if (UNLIKELY(!cns)) {
    if (UNLIKELY(cacheConsts)) {
      cns = TargetCache::s_constants()->nvGet(fallback);
    }
    if (!cns) {
      cns = Unit::loadCns(const_cast<StringData*>(fallback));
    }
    if (UNLIKELY(!cns)) {
      raise_notice(Strings::UNDEFINED_CONSTANT,
                   fallback->data(), fallback->data());
      c1.m_data.pstr = const_cast<StringData*>(fallback);
      c1.m_type = KindOfStaticString;
    }
  } else {
    c1.m_type = cns->m_type;
    c1.m_data = cns->m_data;
  }
  return c1;
}

void iterFreeHelper(Iter* iter) {
  iter->free();
}

void miterFreeHelper(Iter* iter) {
  iter->mfree();
}

void citerFreeHelper(Iter* iter) {
  iter->cfree();
}

//////////////////////////////////////////////////////////////////////

}}
