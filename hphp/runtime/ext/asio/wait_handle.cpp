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

#include "hphp/runtime/ext/asio/wait_handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_result("<result>");
const StaticString s_exception("<exception>");

void c_WaitHandle::t___construct() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

void c_WaitHandle::ti_setonjoincallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set WaitHandle::onJoin: on_join_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnJoinCallback(callback.getObjectDataOrNull());
}

Object c_WaitHandle::t_getwaithandle() {
  return this;
}

// throws if cross-context cycle found
void c_WaitHandle::t_import() {
  if (isFinished()) {
    return;
  }

  context_idx_t ctx_idx = AsioSession::Get()->getCurrentContextIdx();
  if (ctx_idx) {
    assert(instanceof(c_WaitableWaitHandle::classof()));
    static_cast<c_WaitableWaitHandle*>(this)->enterContext(ctx_idx);
  }
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
    Object e(getException());
    throw e;
  }
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
  return ((long) this) / sizeof(void*);
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

Object c_WaitHandle::t_getexceptioniffailed() {
  return isFailed() ? getException() : nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
