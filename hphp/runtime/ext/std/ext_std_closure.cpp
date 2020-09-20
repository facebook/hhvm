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
#include "hphp/runtime/vm/native-prop-handler.h"

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
    auto idx = 0;
    closure->props()->foreach(nProps, [&](tv_rval rval){
      useVars.set(StrNR(propsInfos[idx++].name), *rval);
    });

    ret.set(s_static, make_array_like_tv(useVars.toArray().get()));
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

struct ClosurePropHandler: Native::BasePropHandler {
  static bool isPropSupported(const String&, const String&) {
    raise_error("Closure object cannot have properties");
    return false;
  }
};

///////////////////////////////////////////////////////////////////////////////

const StaticString s_uuinvoke("__invoke");

void c_Closure::init(int numArgs, ActRec* ar, TypedValue* sp) {
  auto const cls = getVMClass();
  auto const invokeFunc = getInvokeFunc();

  if (invokeFunc->cls()) {
    if (invokeFunc->isStatic()) {
      setClass(ar->hasClass() ? ar->getClass() : ar->getThis()->getVMClass());
    } else {
      setThis(ar->getThis());
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

  assertx(props()->checkInvariants(numArgs));
  props()->foreach(numArgs, [&](tv_lval lval) {
    assert(beforeCurUseVar != sp);
    tvCopy(*--beforeCurUseVar, lval);
  });
}

int c_Closure::initActRecFromClosure(ActRec* ar, TypedValue* sp) {
  // Request pointer so that we decref once we are done.
  auto closure = req::ptr<c_Closure>::attach(
    c_Closure::fromObject(ar->getThisInPrologue()));

  // Put in the correct context
  ar->setFunc(closure->getInvokeFunc());

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

  // Copy in all the use vars
  int n = closure->getNumUseVars();
  assertx(closure->props()->checkInvariants(n));
  closure->props()->foreach(n, [&](tv_rval rval) {
    tvDup(*rval, *--sp);
  });

  return n;
}

///////////////////////////////////////////////////////////////////////////////

ObjectData* c_Closure::instanceCtor(Class* cls) {
  raise_error("Can't create a Closure directly");
}

void c_Closure::instanceDtor(ObjectData* obj, const Class* cls) {
  if (UNLIKELY(obj->getAttribute(ObjectData::IsWeakRefed))) {
    WeakRefData::invalidateWeakRef((uintptr_t)obj);
  }

  auto closure = c_Closure::fromObject(obj);
  if (auto t = closure->getThis()) decRefObj(t);

  closure->props()->release(cls->countablePropsEnd());

  auto hdr = closure->hdr();
  tl_heap->objFree(hdr, hdr->size());
}

///////////////////////////////////////////////////////////////////////////////

ObjectData* createClosureRepoAuthRawSmall(Class* cls, size_t size,
                                          size_t index) {
  assertx(!(cls->attrs() & (AttrAbstract|AttrInterface|AttrTrait|AttrEnum)));
  assertx(!cls->needInitialization());
  assertx(cls->parent() == c_Closure::classof() && cls != c_Closure::classof());
  auto mem = tl_heap->mallocSmallIndexSize(index, size);
  auto hdr = new (mem) ClosureHdr(size, ClosureHdr::NoThrow{});
  auto obj = new (hdr + 1) c_Closure(cls, ObjectData::InitRaw{});
  assertx(obj->hasExactlyOneRef());
  return obj;
}

ObjectData* createClosureRepoAuth(Class* cls) {
  assertx(!(cls->attrs() & (AttrAbstract|AttrInterface|AttrTrait|AttrEnum)));
  assertx(!cls->needInitialization());
  assertx(cls->parent() == c_Closure::classof() || cls == c_Closure::classof());
  auto const nProps = cls->numDeclProperties();
  auto const size = sizeof(ClosureHdr) + ObjectData::sizeForNProps(nProps);
  auto hdr = new (tl_heap->objMalloc(size)) ClosureHdr(size);
  auto obj = new (hdr + 1) c_Closure(cls);
  obj->props()->init(nProps);
  assertx(obj->props()->checkInvariants(nProps));
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

  auto const nprops = cls->numDeclProperties();
  auto dst = ret->props()->iteratorAt(0);
  for (auto src : props()->range(0, nprops)) {
    tvDup(src, tv_lval{dst});
    ++dst;
  }

  return ret;
}

void StandardExtension::loadClosure() {
  Native::registerNativePropHandler<ClosurePropHandler>(s_Closure);
  HHVM_SYS_ME(Closure, __debugInfo);
}

///////////////////////////////////////////////////////////////////////////////
}
