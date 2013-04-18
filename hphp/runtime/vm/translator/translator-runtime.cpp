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

#include "runtime/vm/translator/translator-runtime.h"

#include "runtime/ext/ext_function.h"
#include "runtime/vm/member_operations.h"
#include "runtime/vm/type_constraint.h"

namespace HPHP { namespace VM { namespace Transl {

ArrayData* addElemIntKeyHelper(ArrayData* ad,
                               int64_t key,
                               TypedValue value) {
  // this does not re-enter
  return array_setm_ik1_v0(0, ad, key, &value);
}

ArrayData* addElemStringKeyHelper(ArrayData* ad,
                                  StringData* key,
                                  TypedValue value) {
  // this does not re-enter
  return array_setm_s0k1_v0(0, ad, key, &value);
}

HOT_FUNC_VM TypedValue setNewElem(TypedValue* base, Cell val) {
  SetNewElem<true>(base, &val);
  return val;
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

int64_t convStrToBoolHelper(const StringData* s) {
  return s->toBoolean();
}

int64_t convCellToBoolHelper(TypedValue tv) {
  // Cannot call tvCastToBooleanInPlace here because some of the
  // call sites will not be increasing the ref count on tv before
  // calling, the ref count must be left alone.

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:    return false;
    case KindOfBoolean: return tv.m_data.num;
    case KindOfInt64:   return tv.m_data.num != 0;
    case KindOfDouble:  return tv.m_data.dbl != 0;
    case KindOfStaticString:
    case KindOfString:  return tv.m_data.pstr->toBoolean();
    case KindOfArray:   return !tv.m_data.parr->empty();
    case KindOfObject:  return tv.m_data.pobj != nullptr;
    default:            not_reached();
  }
}

int64_t convArrToDblHelper(ArrayData* a) {
  return reinterpretDblAsInt(a->empty() ? 0 : 1);
}

int64_t convStrToDblHelper(const StringData* s) {
  return reinterpretDblAsInt(s->toDouble());
}

int64_t convCellToDblHelper(TypedValue tv) {
  tvCastToDoubleInPlace(&tv); // consumes a ref on counted values
                              // but not if an exception happens. (REVIEW)
  return tv.m_data.num;
}

int64_t convArrToIntHelper(ArrayData* a) {
  return a->empty() ? 0 : 1;
}

int64_t convDblToIntHelper(int64_t i) {
  double d = reinterpretIntAsDbl(i);
  return (d >= 0 ? d > std::numeric_limits<uint64_t>::max() ? 0u :
          (uint64_t)d : (int64_t)d);
}

int64_t convStrToIntHelper(const StringData* s) {
  return s->toInt64(10);
}

int64_t convCellToIntHelper(TypedValue tv) {
  tvCastToInt64InPlace(&tv); // consumes a ref on counted values
                             // but not if an exception happens. (REVIEW)
  return tv.m_data.num;
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
  try {
    auto s = o->t___tostring();
    auto r = s.get();
    o->decRefCount();
    if (!r->isStatic()) r->incRefCount();
    return r;
  } catch (...) {
    o->decRefCount();
    throw;
  }
}

StringData* convCellToStrHelper(TypedValue tv) {
  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:    return buildStringData("");
    case KindOfBoolean: return buildStringData(tv.m_data.num ? "1" : "");
    case KindOfInt64:   return convIntToStrHelper(tv.m_data.num);
    case KindOfDouble:  return convDblToStrHelper(tv.m_data.num);
    case KindOfStaticString:
    case KindOfString:  return tv.m_data.pstr;
    case KindOfArray:   tvDecRefArr(&tv); return buildStringData("Array");
    case KindOfObject:  return convObjToStrHelper(tv.m_data.pobj);
    default:            not_reached();
  }
}

void raisePropertyOnNonObject() {
  raise_warning("Cannot access property on non-object");
}

void raiseUndefProp(ObjectData* base, const StringData* name) {
  static_cast<Instance*>(base)->raiseUndefProp(name);
}

void raise_error_sd(const StringData *msg) {
  raise_error("%s", msg->data());
}

void VerifyParamTypeFail(int paramNum) {
  VMRegAnchor _;
  const ActRec* ar = curFrame();
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
    NameDef def = namedEntity->getCachedNameDef();
    if (UNLIKELY(!def)) {
      VMRegAnchor _;
      String nameStr(const_cast<StringData*>(expected->typeName()));
      if (AutoloadHandler::s_instance->autoloadType(nameStr)) {
        def = namedEntity->getCachedNameDef();
      }
    }
    if (def && (constraint = def.asClass()) && cls->classof(constraint)) {
      return;
    }
  }

  VerifyParamTypeFail(param);
}

template<bool useTargetCache>
RefData* staticLocInitImpl(StringData* name, ActRec* fp, TypedValue val,
                           TargetCache::CacheHandle ch) {
  assert(useTargetCache == (bool)ch);
  HphpArray* map;
  if (useTargetCache) {
    // If we have a cache handle, we know the current func isn't a
    // closure or generator closure so we can directly grab its static
    // locals map.
    const Func* func = fp->m_func;
    assert(!(func->isClosureBody() || func->isGeneratorFromClosure()));
    map = func->getStaticLocals();
  } else {
    map = get_static_locals(fp);
  }

  TypedValue *mapVal = map->nvGet(name);
  if (!mapVal) {
    map->nvSet(name, &val, false);
    mapVal = map->nvGet(name);
  }
  if (mapVal->m_type != KindOfRef) {
    tvBox(mapVal);
  }
  assert(mapVal->m_type == KindOfRef);
  RefData* ret = mapVal->m_data.pref;
  if (useTargetCache) {
    *TargetCache::handleToPtr<RefData*>(ch) = ret;
  }
  ret->incRefCount();
  return ret;
}

RefData* staticLocInit(StringData* name, ActRec* fp, TypedValue val) {
  return staticLocInitImpl<false>(name, fp, val, 0);
}

RefData* staticLocInitCached(StringData* name, ActRec* fp, TypedValue val,
                             TargetCache::CacheHandle ch) {
  return staticLocInitImpl<true>(name, fp, val, ch);
}

} } }
