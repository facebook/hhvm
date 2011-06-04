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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_Closure::c_Closure() : m_vars(Array::Create()), 
  m_callInfo(NULL), m_extraData(NULL) {}
c_Closure::~c_Closure() {}

void c_Closure::t___construct(int64 func, int64 extra, CArrRef vars) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Closure, Closure::__construct);
  m_callInfo = (const CallInfo*) func;
  ASSERT(m_callInfo);
  m_extraData = (void*) extra;
  m_vars = vars;
}

Variant c_Closure::t___invoke(int _argc, CArrRef _argv) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Closure, Closure::__invoke);
  return (m_callInfo->getFunc())((void*)this, _argv);
}

Variant c_Closure::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Closure, Closure::__destruct);
  return null;
}

const CallInfo *c_Closure::t___invokeCallInfoHelper(void *&extra) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Closure, Closure::__invokeCallInfoHelper);
  extra = (void*) this;
  return m_callInfo;
}

///////////////////////////////////////////////////////////////////////////////
}
