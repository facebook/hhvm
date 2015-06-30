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

#ifndef incl_HPHP_EXT_ASIO_WAITABLE_WAIT_HANDLE_H_
#error "This should only be included by ext_waitable-wait-handle.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline
c_WaitableWaitHandle::c_WaitableWaitHandle(Class* cb, HeaderKind kind) noexcept
    : c_WaitHandle(cb, kind) {
  m_parentChain.init();
}

inline c_WaitableWaitHandle::~c_WaitableWaitHandle() {
  switch (getState()) {
    case STATE_SUCCEEDED:
      tvRefcountedDecRef(&m_resultOrException);
      break;

    case STATE_FAILED:
      tvDecRefObj(&m_resultOrException);
      break;
  }
}

inline context_idx_t c_WaitableWaitHandle::getContextIdx() const {
  assert(!isFinished());
  return m_contextIdx;
}

inline void c_WaitableWaitHandle::setContextIdx(context_idx_t ctx_idx) {
  assert(!isFinished());
  m_contextIdx = ctx_idx;
}

inline bool c_WaitableWaitHandle::isInContext() const {
  return getContextIdx();
}

inline AsioContext* c_WaitableWaitHandle::getContext() const {
  assert(isInContext());
  return AsioSession::Get()->getContext(getContextIdx());
}

inline AsioBlockableChain& c_WaitableWaitHandle::getParentChain() {
  assert(!isFinished());
  return m_parentChain;
}

// Throws if establishing a dependency from this to child would form a cycle.
inline void
c_WaitableWaitHandle::detectCycle(c_WaitableWaitHandle* child) const {
  if (UNLIKELY(isDescendantOf(child))) {
    throwCycleException(child);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
