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

#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_StaticWaitHandle::t___construct() {
  // gen-ext-hhvm requires at least one declared method in the class to work
  not_reached();
}

/**
 * Create succeeded StaticWaitHandle object.
 *
 * - consumes reference of the given cell
 * - produces reference for the returned StaticWaitHandle object
 *
 * Both the JIT and bytecode.cpp assume this function gives the nothrow
 * guarantee.
 */
c_StaticWaitHandle* c_StaticWaitHandle::CreateSucceeded(const Cell result) {
  auto waitHandle = req::make<c_StaticWaitHandle>();
  waitHandle->setState(STATE_SUCCEEDED);
  cellCopy(result, waitHandle->m_resultOrException);
  return waitHandle.detach();
}

/**
 * Create failed StaticWaitHandle object.
 *
 * - consumes reference of the given Exception object
 * - produces reference for the returned StaticWaitHandle object
 */
c_StaticWaitHandle* c_StaticWaitHandle::CreateFailed(ObjectData* exception) {
  assert(exception);
  assert(exception->instanceof(SystemLib::s_ExceptionClass));

  auto waitHandle = req::make<c_StaticWaitHandle>();
  waitHandle->setState(STATE_FAILED);
  cellCopy(make_tv<KindOfObject>(exception), waitHandle->m_resultOrException);
  return waitHandle.detach();
}

///////////////////////////////////////////////////////////////////////////////
}
