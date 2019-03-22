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

#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Class* c_Closure::cls_Closure;

const StaticString
  s_Closure("Closure"),
  s_this("this"),
  s_varprefix("$"),
  s_parameter("parameter"),
  s_required("<required>"),
  s_optional("<optional>"),
  s_staticPrefix("86static_");

Slot lookupStaticSlotFromClosure(const Class* cls, const StringData* name) {
  auto str = String::attach(
    StringData::Make(s_staticPrefix.slice(), name->slice())
  );
  auto const slot = cls->lookupDeclProp(str.get());
  assertx(slot != kInvalidSlot);
  return slot;
}

TypedValue* lookupStaticTvFromClosure(ObjectData* closure,
                                      const StringData* name) {
  assertx(closure->instanceof(c_Closure::classof()));
  auto const slot = lookupStaticSlotFromClosure(closure->getVMClass(), name);
  return c_Closure::fromObject(closure)->getStaticVar(slot);
}

static Array HHVM_METHOD(Closure, __debugInfo) {
  auto closure = c_Closure::fromObject(this_);

  Array ret = Array::CreateDArray();

  // Serialize 'use' parameters.
  if (auto useVars = closure->getUseVars()) {
    Array use = Array::CreateDArray();

    auto cls = this_->getVMClass();
    auto propsInfo = cls->declProperties();
    auto nProps = cls->numDeclProperties();
    for (size_t i = 0; i < nProps; ++i) {
      auto value = &useVars[i];
      use.setWithRef(Variant(StrNR(propsInfo[i].name)), tvAsCVarRef(value));
    }

    if (!use.empty()) {
      ret.set(s_static, use);
    }
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
   * Copy the use vars to instance variables, and initialize any
   * instance properties that are for static locals to KindOfUninit.
   */
  auto const numDeclProperties = cls->numDeclProperties();
  assertx(numDeclProperties - numArgs == getInvokeFunc()->numStaticLocals());

  if (debug) {
    // Closure properties shouldn't have type-hints nor should they be LateInit.
    for (auto const& prop : cls->declProperties()) {
      always_assert(!prop.typeConstraint.isCheckable());
      always_assert(!(prop.attrs & AttrLateInit));
    }
  }

  auto beforeCurUseVar = sp + numArgs;
  auto curProperty = getUseVars();
  int i = 0;
  assertx(numArgs <= numDeclProperties);
  for (; i < numArgs; i++) {
    // teleport the references in here so we don't incref
    tvCopy(*--beforeCurUseVar, *curProperty++);
  }
  for (; i < numDeclProperties; ++i) {
    tvWriteUninit(*curProperty++);
  }
}

///////////////////////////////////////////////////////////////////////////////

// Minified versions of nativeDataInstanceCtor/Dtor

static ObjectData* closureInstanceCtorRepoAuth(Class* cls) {
  assertx(!(cls->attrs() & (AttrAbstract|AttrInterface|AttrTrait|AttrEnum)));
  assertx(!cls->needInitialization());
  assertx(cls->parent() == c_Closure::classof() || cls == c_Closure::classof());
  // ensure c_Closure and ClosureHdr ptrs are scanned inside other types
  (void)type_scan::getIndexForMalloc<c_Closure>();
  (void)type_scan::getIndexForMalloc<ClosureHdr>();
  auto const nProps = cls->numDeclProperties();
  auto const size = sizeof(ClosureHdr) + ObjectData::sizeForNProps(nProps);
  auto hdr = new (tl_heap->objMalloc(size)) ClosureHdr(size);
  auto obj = new (hdr + 1) c_Closure(cls);
  assertx(obj->hasExactlyOneRef());
  return obj;
}

static ObjectData* closureInstanceCtor(Class* cls) {
  /*
   * We call Unit::defClosure while jitting, so its not allowed to
   * mark the class as cached unless its persistent. Do it here
   * instead, but only in non-repo-auth mode.
   */
  if (!rds::isHandleInit(cls->classHandle())) {
    cls->preClass()->namedEntity()->clsList()->setCached();
  }
  return closureInstanceCtorRepoAuth(cls);
}

ObjectData* c_Closure::clone() {
  auto const cls = getVMClass();
  auto ret = c_Closure::fromObject(closureInstanceCtorRepoAuth(cls));

  ret->hdr()->ctx = hdr()->ctx;
  if (auto t = getThis()) {
    t->incRefCount();
  }

  auto src  = getUseVars();
  auto dest = ret->getUseVars();
  auto const nProps = cls->numDeclProperties();
  auto const stop = src + nProps;
  for (; src != stop; ++src, ++dest) {
    tvDup(*src, *dest);
  }

  return ret;
}

static void closureInstanceDtor(ObjectData* obj, const Class* cls) {
  if (UNLIKELY(obj->getAttribute(ObjectData::IsWeakRefed))) {
    WeakRefData::invalidateWeakRef((uintptr_t)obj);
  }
  auto const nProps = size_t{cls->numDeclProperties()};
  auto prop = c_Closure::fromObject(obj)->getUseVars();
  auto const stop = prop + nProps;
  auto closure = c_Closure::fromObject(obj);
  if (auto t = closure->getThis()) {
    decRefObj(t);
  }
  for (; prop != stop; ++prop) {
    tvDecRefGen(prop);
  }
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
  c_Closure::cls_Closure->m_extra.raw()->m_instanceCtor =
    RuntimeOption::RepoAuthoritative
      ? closureInstanceCtorRepoAuth : closureInstanceCtor;
  c_Closure::cls_Closure->m_extra.raw()->m_instanceDtor = closureInstanceDtor;
}

///////////////////////////////////////////////////////////////////////////////
}
