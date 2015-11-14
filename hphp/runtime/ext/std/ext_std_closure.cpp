/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Closure::~Closure() {
  if (auto t = getThis()) {
    decRefObj(t);
  }
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_Closure("Closure"),
  s_this("this"),
  s_varprefix("$"),
  s_parameter("parameter"),
  s_required("<required>"),
  s_optional("<optional>");

static Array HHVM_METHOD(Closure, __debugInfo) {
  auto closure = Native::data<Closure>(this_);

  Array ret = Array::Create();

  // Serialize 'use' parameters.
  if (auto propValues = this_->propVec()) {
    Array use;

    auto propsInfo = this_->getVMClass()->declProperties();
    for (size_t i = 0; i < this_->getVMClass()->numDeclProperties(); ++i) {
      auto value = &propValues[i];
      use.setWithRef(Variant(StrNR(propsInfo[i].name)), tvAsCVarRef(value));
    }

    if (!use.empty()) {
      ret.set(s_static, use);
    }
  }

  auto const func = closure->getInvokeFunc();

  // Serialize function parameters.
  if (func->numParams()) {
   Array params;

   for (int i = 0; i < func->numParams(); ++i) {
      auto str = String::attach(
        StringData::Make(s_varprefix.get(), func->localNames()[i])
      );

      bool optional = func->params()[i].phpCode;
      if (auto mi = func->methInfo()) {
        optional = optional || mi->parameters[i]->valueText;
      }

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

void Closure::init(int numArgs, ActRec* ar, TypedValue* sp) {
  auto obj = Native::object<Closure>(this);
  auto const invokeFunc = obj->getVMClass()->lookupMethod(s_uuinvoke.get());

  m_ctx = ar->m_this;
  if (ar->hasThis()) {
    if (invokeFunc->isStatic()) {
      // Only set the class for static closures.
      setClass(ar->getThis()->getVMClass());
    } else {
      ar->getThis()->incRefCount();
    }
  }

  /*
   * Copy the use vars to instance variables, and initialize any
   * instance properties that are for static locals to KindOfUninit.
   */
  auto const numDeclProperties = obj->getVMClass()->numDeclProperties();
  assert(numDeclProperties - numArgs == getInvokeFunc()->numStaticLocals());
  TypedValue* beforeCurUseVar = sp + numArgs;
  TypedValue* curProperty = obj->propVec();
  int i = 0;
  assert(numArgs <= numDeclProperties);
  for (; i < numArgs; i++) {
    // teleport the references in here so we don't incref
    tvCopy(*--beforeCurUseVar, *curProperty++);
  }
  for (; i < numDeclProperties; ++i) {
    tvWriteUninit(curProperty++);
  }
}

Closure& Closure::operator=(const Closure &src) {
  m_ctx = src.m_ctx;
  return *this;
}

static Variant HHVM_METHOD(Closure, bindto,
                           const Variant& newthis, const Variant& scope) {
  if (RuntimeOption::RepoAuthoritative &&
      RuntimeOption::EvalAllowScopeBinding) {
    raise_warning("Closure binding is not supported in RepoAuthoritative mode");
    return null_variant;
  }

  auto const cls = this_->getVMClass();
  auto const invoke = cls->getCachedInvoke();

  ObjectData* od = nullptr;
  if (newthis.isObject()) {
    if (invoke->isStatic()) {
      raise_warning("Cannot bind an instance to a static closure");
    } else {
      od = newthis.getObjectData();
    }
  } else if (!newthis.isNull()) {
    raise_warning("Closure::bindto() expects parameter 1 to be object");
    return null_variant;
  }

  auto const curscope = invoke->cls();
  auto newscope = curscope;

  if (scope.isObject()) {
    newscope = scope.getObjectData()->getVMClass();
  } else if (scope.isString()) {
    auto const className = scope.getStringData();

    if (!className->equal(s_static.get())) {
      newscope = Unit::loadClass(className);
      if (!newscope) {
        raise_warning("Class '%s' not found", className->data());
        return null_variant;
      }
    }
  } else if (scope.isNull()) {
    newscope = nullptr;
  } else {
    raise_warning("Closure::bindto() expects parameter 2 "
                  "to be string or object");
    return null_variant;
  }

  if (od && !newscope) {
    // Bound closures should be scoped.  If no scope is specified, scope it to
    // the Closure class.
    newscope = static_cast<Class*>(Closure::classof());
  }

  bool thisNotOfCtx = od && !od->getVMClass()->classof(newscope);

  if (!RuntimeOption::EvalAllowScopeBinding) {
    if (newscope != curscope) {
      raise_warning("Re-binding closure scopes is disabled");
      return null_variant;
    }

    if (thisNotOfCtx) {
      raise_warning("Binding to objects not subclassed from closure "
                    "context is disabled");
      return null_variant;
    }
  }

  auto cloneObj = this_->clone();
  auto clone = Native::data<Closure>(cloneObj);
  clone->setClass(nullptr);

  Attr curattrs = invoke->attrs();
  Attr newattrs = static_cast<Attr>(curattrs & ~AttrHasForeignThis);

  if (od) {
    od->incRefCount();
    clone->setThis(od);

    if (thisNotOfCtx) {
      // If the bound $this is not a subclass of the context class, then we
      // have to pessimize translation.
      newattrs |= AttrHasForeignThis;
    }
  } else if (newscope) {
    // If we attach a scope to a function with no bound $this we need to make
    // the function static.
    newattrs |= AttrStatic;
    clone->setClass(newscope);
  }

  // If we are changing either the scope or the attributes of the closure, we
  // need to re-scope its Closure subclass.
  if (newscope != curscope || newattrs != curattrs) {
    assert(newattrs != AttrNone);

    auto newcls = cls->rescope(newscope, newattrs);
    cloneObj->setVMClass(newcls);
  }

  return Object{cloneObj};
}

static Variant HHVM_METHOD(Closure, call,
                           const Variant& newthis,
                           const Array& params) {
  if (newthis.isNull() || !newthis.isObject()) {
    raise_warning(
      "Closure::call() expects parameter 1 to be object, %s given",
      getDataTypeString(newthis.getType()).c_str()
    );
    return null_variant;
  }

  // So, with bind/bindTo, if we are trying to bind an instance to a static
  // closure, we just raise a warning and continue on. However, with call
  // we are supposed to just return null (according to the PHP 7 implementation)
  // Here is that speciality check, then. Do it here so we don't have to go
  // through the rigormorale of binding if this is the case.
  if (this_->getVMClass()->getCachedInvoke()->isStatic()) {
    raise_warning("Cannot bind an instance to a static closure");
    return null_variant;
  }

  auto bound = HHVM_MN(Closure, bindto)(this_, newthis, newthis);
  // If something went wrong in the binding (warning, for example), then
  // we can get an empty object back. And an empty object is null by
  // default. Return null if that is the case.
  if (bound.isNull()) {
    return null_variant;
  }

  Variant ret;
  // Could call vm_user_func(bound, params) here which goes through a
  // whole decode function process to get a Func*. But we know this
  // is a closure, and we can get a Func* via getInvokeFunc(), so just
  // bypass all that decode process to save time.
  g_context->invokeFunc((TypedValue*)&ret,
                        Native::data<Closure>(this_)->getInvokeFunc(),
                        params, bound.toObject().get(),
                        nullptr, nullptr, nullptr,
                        ExecutionContext::InvokeCuf);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::initClosure() {
  HHVM_ME(Closure, __debugInfo);
  HHVM_ME(Closure, bindto);
  HHVM_ME(Closure, call);

  Native::registerNativeDataInfo<Closure>(s_Closure.get());
}

///////////////////////////////////////////////////////////////////////////////
}
