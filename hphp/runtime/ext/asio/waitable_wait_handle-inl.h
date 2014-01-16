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

#ifndef incl_HPHP_EXT_ASIO_WAITABLE_WAIT_HANDLE_H_
#error "This should only be included by waitable_wait_handle.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void
c_WaitableWaitHandle::enterContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // Already in a more specific context?
  if (LIKELY(getContextIdx() >= ctx_idx)) {
    return;
  }

  // If this wait handle is being finished and there is a parent A that is being
  // unblocked and a parent B that was not unblocked yet, it is possible that
  // the parent A triggered an enterContext() that reaches us back thru the
  // parent B. Fortunately, parent's context is always equal or smaller, so
  // the condition above handles !isFinished() case.
  assert(!isFinished());

  enterContextImpl(ctx_idx);
}

inline c_BlockableWaitHandle*
c_WaitableWaitHandle::addParent(c_BlockableWaitHandle* parent) {
  assert(!isFinished());

  auto prev = m_firstParent;
  m_firstParent = parent;
  return prev;
}

///////////////////////////////////////////////////////////////////////////////
}
