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

#include "hphp/runtime/ext/asio/session_scoped_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_SessionScopedWaitHandle::t___construct() {
  throw NotSupportedException(__func__, "Cannot construct abstract class");
}

void c_SessionScopedWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    unregisterFromContext();
  }

  setContextIdx(ctx_idx);
  registerToContext();
}

void c_SessionScopedWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));
  assert(getContextIdx() == ctx_idx);
  assert(getState() == STATE_WAITING);

  // Move us to the parent context.
  setContextIdx(getContextIdx() - 1);

  // Re-register if still in a context.
  if (isInContext()) {
    registerToContext();
  }

  // Recursively move all wait handles blocked by us.
  for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
    pwh->exitContextBlocked(ctx_idx);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
