/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

void c_Closure::t___construct() {
  raise_error("Can't create a Closure directly");
}

const StaticString s_uuinvoke("__invoke");

void c_Closure::init(int numArgs, ActRec* ar, TypedValue* sp) {
  auto const invokeFunc = getVMClass()->lookupMethod(s_uuinvoke.get());

  m_thisOrClass = ar->m_this;
  if (ar->hasThis()) {
    if (invokeFunc->attrs() & AttrStatic) {
      // Only set the class for static closures.
      setClass(ar->getThis()->getVMClass());
    } else {
      ar->getThis()->incRefCount();
    }
  }

  // Change my __invoke's m_cls to be the same as my creator's
  Class* scope = ar->m_func->cls();
  m_func = invokeFunc->cloneAndSetClass(scope);

  /*
   * Copy the use vars to instance variables, and initialize any
   * instance properties that are for static locals to KindOfUninit.
   */
  auto const numDeclProperties = getVMClass()->numDeclProperties();
  assert(numDeclProperties - numArgs == m_func->numStaticLocals());
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
  closure->m_thisOrClass = thiz->m_thisOrClass;
  closure->m_func = thiz->m_func;
  return closure;
}

///////////////////////////////////////////////////////////////////////////////
}
