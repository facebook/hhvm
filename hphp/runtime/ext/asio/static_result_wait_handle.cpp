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

#include "hphp/runtime/ext/asio/static_result_wait_handle.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_StaticResultWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use StaticResultWaitHandle::create() instead of constructor"));
  throw e;
}

c_StaticResultWaitHandle* c_StaticResultWaitHandle::Create(const Cell& result) {
  c_StaticResultWaitHandle* wh = NEWOBJ(c_StaticResultWaitHandle)();
  cellDup(result, wh->m_resultOrException);
  return wh;
}

ObjectData* c_StaticResultWaitHandle::CreateFromVM(const Cell result) {
  c_StaticResultWaitHandle* wh = NEWOBJ(c_StaticResultWaitHandle)();
  cellCopy(result, wh->m_resultOrException);
  wh->incRefCount();
  return wh;
}

///////////////////////////////////////////////////////////////////////////////
}
