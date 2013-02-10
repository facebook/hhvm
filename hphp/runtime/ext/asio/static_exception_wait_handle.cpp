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
  StaticString s_staticException("<static-exception>");
}

c_StaticExceptionWaitHandle::c_StaticExceptionWaitHandle(
    const ObjectStaticCallbacks *cb)
    : c_StaticWaitHandle(cb) {
  setState(STATE_FAILED);
}

c_StaticExceptionWaitHandle::~c_StaticExceptionWaitHandle() {
  tvDecRefObj(&m_resultOrException);
}

void c_StaticExceptionWaitHandle::t___construct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(StaticExceptionWaitHandle, StaticExceptionWaitHandle::__construct);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use StaticExceptionWaitHandle::create() instead of constructor"));
  throw e;
}

Object c_StaticExceptionWaitHandle::ti_create(const char* cls, CObjRef exception) {
  if (!exception.instanceof("Exception")) {
    STATIC_METHOD_INJECTION_BUILTIN(StaticExceptionWaitHandle, StaticExceptionWaitHandle::create);
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected exception to be an instance of Exception"));
    throw e;
  }

  p_StaticExceptionWaitHandle wh = NEWOBJ(c_StaticExceptionWaitHandle)();
  tvWriteObject(exception.get(), &wh->m_resultOrException);
  return wh;
}

const TypedValue* c_StaticExceptionWaitHandle::join() {
  Object e(m_resultOrException.m_data.pobj);
  throw e;
}

String c_StaticExceptionWaitHandle::getName() {
  return s_staticException;
}

///////////////////////////////////////////////////////////////////////////////
}
