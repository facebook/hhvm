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
#include <runtime/ext/asio/asio_session.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_WaitHandle::c_WaitHandle(VM::Class* cb)
    : ExtObjectData(cb) {
}

c_WaitHandle::~c_WaitHandle() {
}

void c_WaitHandle::t___construct() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

// throws if cross-context cycle found
void c_WaitHandle::t_import() {
  if (isFinished()) {
    return;
  }

  context_idx_t ctx_idx = AsioSession::Get()->getCurrentContextIdx();
  if (ctx_idx) {
    assert(dynamic_cast<c_WaitableWaitHandle*>(this));
    static_cast<c_WaitableWaitHandle*>(this)->enterContext(ctx_idx);
  }
}

Variant c_WaitHandle::t_join() {
  if (!isFinished()) {
    // run the full blown machinery
    assert(dynamic_cast<c_WaitableWaitHandle*>(this));
    static_cast<c_WaitableWaitHandle*>(this)->join();
  }

  assert(isFinished());

  if (LIKELY(isSucceeded())) {
    // succeeded? return result
    return tvAsCVarRef(getResult());
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
  return getName();
}

Object c_WaitHandle::t_getexceptioniffailed() {
  return isFailed() ? getException() : nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
