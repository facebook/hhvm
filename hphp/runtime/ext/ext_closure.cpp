/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_closure.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/vm/translator/translator-inline.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_Closure::c_Closure(VM::Class* cb) : ExtObjectData(cb),
  m_thisOrClass(nullptr), m_func(nullptr) {}
c_Closure::~c_Closure() {
  // same as ar->hasThis()
  if (m_thisOrClass && !(intptr_t(m_thisOrClass) & 3LL)) {
    m_thisOrClass->decRefCount();
  }
}

void c_Closure::t___construct() {
  VM::ActRec* ar;
  {
    // like calling CallerFrame twice
    VM::Transl::VMRegAnchor _;
    VMExecutionContext* context = g_vmContext;
    VM::ActRec* me = context->getFP();
    VM::ActRec* childClosure = context->getPrevVMState(me);
    ar = context->getPrevVMState(childClosure);
  }

  // I don't care if it is a $this or a late bound class because we will just
  // put it back in the same place on an ActRec.
  m_thisOrClass = ar->m_this;
  if (ar->hasThis()) {
    ar->getThis()->incRefCount();
  }

  // Change my __invoke's m_cls to be the same as my creator's
  static StringData* invokeName = StringData::GetStaticString("__invoke");
  VM::Class* scope = ar->m_func->cls();
  m_func = getVMClass()->lookupMethod(invokeName)->cloneAndSetClass(scope);
}

ObjectData* c_Closure::clone() {
  ObjectData* obj = ObjectData::clone();
  auto closure = static_cast<c_Closure*>(obj);
  closure->m_VMStatics = m_VMStatics;
  closure->m_thisOrClass = m_thisOrClass;
  closure->m_func = m_func;
  return obj;
}


bool c_Closure::php_sleep(Variant &ret) {
  ret = false;
  return true;
}

HphpArray* c_Closure::getStaticLocals() {
  if (m_VMStatics.get() == NULL) {
    m_VMStatics = NEW(HphpArray)(1);
  }
  return m_VMStatics.get();
}

///////////////////////////////////////////////////////////////////////////////

c_DummyClosure::c_DummyClosure(VM::Class* cb) :
  ExtObjectData(cb) {
}

c_DummyClosure::~c_DummyClosure() {}

void c_DummyClosure::t___construct() {
}

///////////////////////////////////////////////////////////////////////////////
}
