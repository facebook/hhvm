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
