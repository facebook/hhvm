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

#include "hphp/runtime/ext/asio/ext_wait-handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio-context-enter.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_result("<result>");
const StaticString s_exception("<exception>");

void c_WaitHandle::t___construct() {
  throw_not_supported(
      __func__, "WaitHandles cannot be constructed directly");
}

void c_WaitHandle::ti_setoniowaitentercallback(const Variant& callback) {
  AsioSession::Get()->setOnIOWaitEnter(callback);
}

void c_WaitHandle::ti_setoniowaitexitcallback(const Variant& callback) {
  AsioSession::Get()->setOnIOWaitExit(callback);
}

void c_WaitHandle::ti_setonjoincallback(const Variant& callback) {
  AsioSession::Get()->setOnJoin(callback);
}

Object c_WaitHandle::t_getwaithandle() {
  always_assert(false);
}

// throws if cross-context cycle found
void c_WaitHandle::t_import() {
  if (isFinished()) {
    return;
  }

  assert(instanceof(c_WaitableWaitHandle::classof()));
  auto const ctx_idx = AsioSession::Get()->getCurrentContextIdx();
  asio::enter_context(static_cast<c_WaitableWaitHandle*>(this), ctx_idx);
}

Variant c_WaitHandle::t_join() {
  if (!isFinished()) {
    // run the full blown machinery
    assert(instanceof(c_WaitableWaitHandle::classof()));
    static_cast<c_WaitableWaitHandle*>(this)->join();
  }
  assert(isFinished());

  if (LIKELY(isSucceeded())) {
    // succeeded? return result
    return cellAsCVarRef(getResult());
  } else {
    // failed? throw exception
    throw Object{getException()};
  }
}

Variant c_WaitHandle::t_result() {
  always_assert(false);
}

bool c_WaitHandle::t_isfinished() {
  return isFinished();
}

bool c_WaitHandle::t_issucceeded() {
  return isSucceeded();
}

bool c_WaitHandle::t_isfailed() {
  return isFailed();
}

int64_t c_WaitHandle::t_getid() {
  return ((intptr_t)this) / sizeof(void*);
}

String c_WaitHandle::t_getname() {
  if (isSucceeded()) {
    return s_result;
  } else if (isFailed()) {
    return s_exception;
  }

  assert(instanceof(c_WaitableWaitHandle::classof()));
  return static_cast<c_WaitableWaitHandle*>(this)->getName();
}

///////////////////////////////////////////////////////////////////////////////
}
