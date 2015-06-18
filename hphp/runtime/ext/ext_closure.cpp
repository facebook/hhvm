/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_closure.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_Closure::~c_Closure() {
  if (auto t = getThis()) {
    decRefObj(t);
  }
}

///////////////////////////////////////////////////////////////////////////////

void c_Closure::t___construct() {
  raise_error("Can't create a Closure directly");
}

Variant c_Closure::t___get(Variant member) {
  raise_recoverable_error("Closure object cannot have properties");
  return init_null();
}

Variant c_Closure::t___set(Variant member, Variant value) {
  raise_recoverable_error("Closure object cannot have properties");
  return init_null();
}

bool c_Closure::t___isset(Variant name) {
  raise_recoverable_error("Closure object cannot have properties");
  return false;
}

Variant c_Closure::t___unset(Variant name) {
  raise_recoverable_error("Closure object cannot have properties");
  return init_null();
}

static StaticString
  s_this("this"),
  s_varprefix("$"),
  s_parameter("parameter"),
  s_required("<required>"),
  s_optional("<optional>");

Array c_Closure::t___debuginfo() {
  Array ret = Array::Create();

  // Serialize 'use' parameters.
  if (auto propValues = propVec()) {
    Array use;

    auto propsInfo = getVMClass()->declProperties();
    for (int i = 0; i < getVMClass()->numDeclProperties(); ++i) {
      TypedValue* value = &propValues[i];
      use.setWithRef(Variant(StrNR(propsInfo[i].m_name)), tvAsCVarRef(value));
    }

    if (!use.empty()) {
      ret.set(s_static, use);
    }
  }

  auto const func = getInvokeFunc();

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
  if (hasThis()) {
    ret.set(s_this, Object(getThis()));
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_uuinvoke("__invoke");

void c_Closure::init(int numArgs, ActRec* ar, TypedValue* sp) {
  auto const invokeFunc = getVMClass()->lookupMethod(s_uuinvoke.get());

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
  auto const numDeclProperties = getVMClass()->numDeclProperties();
  assert(numDeclProperties - numArgs == getInvokeFunc()->numStaticLocals());
  TypedValue* beforeCurUseVar = sp + numArgs;
  TypedValue* curProperty = propVec();
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

c_Closure* c_Closure::Clone(ObjectData* obj) {
  auto thiz = static_cast<c_Closure*>(obj);
  auto closure = static_cast<c_Closure*>(obj->cloneImpl());
  closure->m_ctx = thiz->m_ctx;
  return closure;
}

Object c_Closure::ti_bind(const Variant& closure, const Variant& newthis,
                           const Variant& scope) {
  if (!closure.isObject()) {
    raise_warning("Closure::bind() expects parameter 1 to be an object");
    return nullptr;
  }

  Object closureObject = closure.toObject();
  if (!closureObject.is<c_Closure>()) {
    raise_warning("Closure::bind() expects parameter 1 to be closure");
    return nullptr;
  }

  if (!newthis.isObject() && !newthis.isNull()) {
    raise_warning("Closure::bind() expects parameter 2 to be object or NULL");
    return nullptr;
  }

  if (!scope.isObject() && !scope.isString() && !scope.isNull()) {
    raise_warning("Closure::bindto expects parameter 3 to be string or object");
    return nullptr;
  }

  return unsafe_cast<c_Closure>(closureObject)->t_bindto(newthis, scope);
}

Object c_Closure::t_bindto(const Variant& newthis, const Variant& scope) {
  if (RuntimeOption::RepoAuthoritative &&
      RuntimeOption::EvalAllowScopeBinding) {
    raise_warning("Closure binding is not supported in RepoAuthoritative mode");
    return nullptr;
  }

  auto const cls = getVMClass();
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
    return nullptr;
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
        return nullptr;
      }
    }
  } else if (scope.isNull()) {
    newscope = nullptr;
  } else {
    raise_warning("Closure::bindto() expects parameter 2 "
                  "to be string or object");
    return nullptr;
  }

  if (od && !newscope) {
    // Bound closures should be scoped.  If no scope is specified, scope it to
    // the Closure class.
    newscope = static_cast<Class*>(c_Closure::classof());
  }

  bool thisNotOfCtx = od && !od->getVMClass()->classof(newscope);

  if (!RuntimeOption::EvalAllowScopeBinding) {
    if (newscope != curscope) {
      raise_warning("Re-binding closure scopes is disabled");
      return nullptr;
    }

    if (thisNotOfCtx) {
      raise_warning("Binding to objects not subclassed from closure "
                    "context is disabled");
      return nullptr;
    }
  }

  c_Closure* clone = Clone(this);
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
    clone->setVMClass(newcls);
  }

  return Object(clone);
}

///////////////////////////////////////////////////////////////////////////////
}
