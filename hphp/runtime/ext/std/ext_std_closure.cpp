/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Class* c_Closure::cls_Closure;

const StaticString
  s_Closure("Closure"),
  s_this("this"),
  s_varprefix("$"),
  s_parameter("parameter"),
  s_required("<required>"),
  s_optional("<optional>");

static Array HHVM_METHOD(Closure, __debugInfo) {
  auto closure = c_Closure::fromObject(this_);

  Array ret = Array::CreateDArray();

  // Serialize 'use' parameters.
  auto cls = this_->getVMClass();
  if (auto nProps = cls->numDeclProperties()) {
    DArrayInit useVars(nProps);

    auto propsInfos = cls->declProperties();
    auto props = closure->propVec();
    for (size_t i = 0; i < nProps; ++i) {
      useVars.set(StrNR(propsInfos[i].name), props[i]);
    }

    ret.set(s_static, make_tv<KindOfArray>(useVars.toArray().get()));
  }

  auto const func = closure->getInvokeFunc();

  // Serialize function parameters.
  if (auto nParams = func->numParams()) {
   Array params = Array::CreateDArray();

   auto lNames = func->localNames();
   for (int i = 0; i < nParams; ++i) {
      auto str = String::attach(
        StringData::Make(s_varprefix.get(), lNames[i])
      );

      bool optional = func->params()[i].phpCode;
      params.set(str, optional ? s_optional : s_required);
    }

    ret.set(s_parameter, params);
  }

  // Serialize 'this' object.
  if (closure->hasThis()) {
    ret.set(s_this, Object(closure->getThis()));
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_uuinvoke("__invoke");

void c_Closure::init(int numArgs, ActRec* ar, TypedValue* sp) {
  auto const cls = getVMClass();
  auto const invokeFunc = getInvokeFunc();

  if (invokeFunc->cls()) {
    setThisOrClass(ar->getThisOrClass());
    if (invokeFunc->isStatic()) {
      if (!hasClass()) {
        setClass(getThisUnchecked()->getVMClass());
      }
    } else if (!hasClass()) {
      getThisUnchecked()->incRefCount();
    }
  } else {
    hdr()->ctx = nullptr;
  }

  /*
   * Copy the use vars to instance variables.
   */
  assertx(cls->numDeclProperties() == numArgs);

  if (debug) {
    // Closure properties shouldn't have type-hints nor should they be LateInit.
    for (auto const& prop : cls->declProperties()) {
      always_assert(!prop.typeConstraint.isCheckable());
      always_assert(!(prop.attrs & AttrLateInit));
    }
  }

  auto beforeCurUseVar = sp + numArgs;
  auto curProperty = propVecForConstruct();
  while (beforeCurUseVar != sp) cellCopy(*--beforeCurUseVar, *curProperty++);
}

int c_Closure::initActRecFromClosure(ActRec* ar, TypedValue* sp) {
  auto closure = c_Closure::fromObject(ar->getThis());

  // Put in the correct context
  ar->m_func = closure->getInvokeFunc();

  if (ar->func()->cls()) {
    // Swap in the $this or late bound class or null if it is from a plain
    // function or pseudomain
    ar->setThisOrClass(closure->getThisOrClass());

    if (ar->hasThis()) {
      ar->getThis()->incRefCount();
    }
  } else {
    ar->trashThis();
  }

  // The closure is the first local.
  // Similar to tvWriteObject() but we don't incref because it used to be $this
  // and now it is a local, so they cancel out
  TypedValue* firstLocal = --sp;
  firstLocal->m_type = KindOfObject;
  firstLocal->m_data.pobj = closure;

  // Copy in all the use vars
  auto prop = closure->propVec();
  int n = closure->getNumUseVars();
  for (int i = 0; i < n; i++) {
    cellDup(*prop++, *--sp);
  }

  return n + 1;
}

///////////////////////////////////////////////////////////////////////////////

ObjectData* createClosureRepoAuth(Class* cls) {
  assertx(!(cls->attrs() & (AttrAbstract|AttrInterface|AttrTrait|AttrEnum)));
  assertx(!cls->needInitialization());
  assertx(cls->parent() == c_Closure::classof() || cls == c_Closure::classof());
  auto const nProps = cls->numDeclProperties();
  auto const size = sizeof(ClosureHdr) + ObjectData::sizeForNProps(nProps);
  auto hdr = new (tl_heap->objMalloc(size)) ClosureHdr(size);
  auto obj = new (hdr + 1) c_Closure(cls);
  assertx(obj->hasExactlyOneRef());
  return obj;
}

ObjectData* createClosure(Class* cls) {
  /*
   * We call Unit::defClosure while jitting, so its not allowed to
   * mark the class as cached unless its persistent. Do it here
   * instead, but only in non-repo-auth mode.
   */
  if (!rds::isHandleInit(cls->classHandle())) {
    cls->preClass()->namedEntity()->clsList()->setCached();
  }
  return createClosureRepoAuth(cls);
}

static ObjectData* closureInstanceCtor(Class* cls) {
  raise_error("Can't create a Closure directly");
}

// should never be called
ATTRIBUTE_USED ATTRIBUTE_UNUSED EXTERNALLY_VISIBLE
static void closuseInstanceReference(void) {
  // ensure c_Closure and ClosureHdr ptrs are scanned inside other types
  (void)type_scan::getIndexForMalloc<c_Closure>();
  (void)type_scan::getIndexForMalloc<ClosureHdr>();
}

ObjectData* c_Closure::clone() {
  auto const cls = getVMClass();
  auto ret = c_Closure::fromObject(createClosureRepoAuth(cls));

  ret->hdr()->ctx = hdr()->ctx;
  if (auto t = getThis()) {
    t->incRefCount();
  }

  auto src  = propVec();
  auto dest = ret->propVecForConstruct();
  auto const stop = src + cls->numDeclProperties();
  while (src != stop) cellDup(*src++, *dest++);

  return ret;
}

static void closureInstanceDtor(ObjectData* obj, const Class* cls) {
  if (UNLIKELY(obj->getAttribute(ObjectData::IsWeakRefed))) {
    WeakRefData::invalidateWeakRef((uintptr_t)obj);
  }

  auto closure = c_Closure::fromObject(obj);
  if (auto t = closure->getThis()) decRefObj(t);

  // We're destructing, not constructing, but we're unconditionally allowed to
  // write just the same.
  auto prop = closure->propVecForConstruct();
  auto const stop = prop + cls->numDeclProperties();
  while (prop != stop) tvDecRefGen(prop++);

  auto hdr = closure->hdr();
  tl_heap->objFree(hdr, hdr->size());
}

void StandardExtension::loadClosure() {
  HHVM_SYS_ME(Closure, __debugInfo);
}

void StandardExtension::initClosure() {
  c_Closure::cls_Closure = Unit::lookupClass(s_Closure.get());
  assertx(c_Closure::cls_Closure);
  assertx(!c_Closure::cls_Closure->hasMemoSlots());
  c_Closure::cls_Closure->allocExtraData();
  auto& extraData = *c_Closure::cls_Closure->m_extra.raw();
  extraData.m_instanceCtor = extraData.m_instanceCtorUnlocked =
    closureInstanceCtor;
  extraData.m_instanceDtor = closureInstanceDtor;
  c_Closure::cls_Closure->m_releaseFunc = closureInstanceDtor;
}

///////////////////////////////////////////////////////////////////////////////
}
