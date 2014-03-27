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

#include "hphp/runtime/ext/asio/static_exception_wait_handle.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_StaticExceptionWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use StaticExceptionWaitHandle::create() instead of constructor"));
  throw e;
}

c_StaticExceptionWaitHandle*
c_StaticExceptionWaitHandle::Create(ObjectData* exception) {
  assert(exception->instanceof(SystemLib::s_ExceptionClass));

  auto wait_handle = NEWOBJ(c_StaticExceptionWaitHandle)();
  tvWriteObject(exception, &wait_handle->m_resultOrException);
  return wait_handle;
}

///////////////////////////////////////////////////////////////////////////////
}
