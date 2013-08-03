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
  // same as ar->hasThis()
  if (m_thisOrClass && !(intptr_t(m_thisOrClass) & 1LL)) {
    decRefObj(m_thisOrClass);
  }
}

void c_Closure::t___construct() {
  raise_error("Can't create a Closure directly");
}

const StaticString s_uuinvoke("__invoke");

void c_Closure::init(int numArgs, ActRec* ar, TypedValue* sp) {
  auto const invokeFunc = getVMClass()->lookupMethod(s_uuinvoke.get());

  if (invokeFunc->attrs() & AttrStatic) {
    // Only set the class for static closures
    m_thisOrClass = (ObjectData*)(intptr_t(ar->m_func->cls()) | 1LL);
  } else {
    // I don't care if it is a $this or a late bound class because we will just
    // put it back in the same place on an ActRec.
    m_thisOrClass = ar->m_this;
    if (ar->hasThis()) {
      ar->getThis()->incRefCount();
    }
  }

  // Change my __invoke's m_cls to be the same as my creator's
  Class* scope = ar->m_func->cls();
  m_func = invokeFunc->cloneAndSetClass(scope);

  // copy the props to instance variables
  assert(m_cls->numDeclProperties() == numArgs);
  TypedValue* beforeCurUseVar = sp + numArgs;
  TypedValue* curProperty = propVec();
  for (int i = 0; i < numArgs; i++) {
    // teleport the references in here so we don't incref
    tvCopy(*--beforeCurUseVar, *curProperty++);
  }
}

c_Closure* c_Closure::clone() {
  auto closure = static_cast<c_Closure*>(ObjectData::clone());
  closure->m_VMStatics = m_VMStatics;
  closure->m_thisOrClass = m_thisOrClass;
  closure->m_func = m_func;
  return closure;
}

HphpArray* c_Closure::getStaticLocals() {
  if (m_VMStatics.get() == NULL) {
    m_VMStatics = ArrayData::Make(1);
  }
  return m_VMStatics.get();
}

///////////////////////////////////////////////////////////////////////////////
}
