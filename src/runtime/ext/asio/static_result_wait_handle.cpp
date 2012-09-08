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

#include <runtime/ext/ext_asio.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_staticResult("<static-result>");
}

c_StaticResultWaitHandle::c_StaticResultWaitHandle(
    const ObjectStaticCallbacks *cb)
    : c_StaticWaitHandle(cb) {
}

c_StaticResultWaitHandle::~c_StaticResultWaitHandle() {
  tvRefcountedDecRefCell(&m_result);
}

void c_StaticResultWaitHandle::t___construct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(StaticResultWaitHandle, StaticResultWaitHandle::__construct);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use StaticResultWaitHandle::create() instead of constructor"));
  throw e;
}

Object c_StaticResultWaitHandle::ti_create(const char* cls, CVarRef result) {
  p_StaticResultWaitHandle wh = NEWOBJ(c_StaticResultWaitHandle)();
  tvReadCell(result.asTypedValue(), &wh->m_result);
  return wh;
}

const TypedValue* c_StaticResultWaitHandle::join() {
  return &m_result;
}

String c_StaticResultWaitHandle::getName() {
  return s_staticResult;
}

///////////////////////////////////////////////////////////////////////////////
}
