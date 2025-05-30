/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline
c_WaitableWaitHandle::c_WaitableWaitHandle(Class* cb, HeaderKind kind,
                                           type_scan::Index tyindex) noexcept
    : c_Awaitable(cb, kind, tyindex) {
  m_parentChain.init();
}

inline c_WaitableWaitHandle::~c_WaitableWaitHandle() {
  switch (getState()) {
    case STATE_SUCCEEDED:
      tvDecRefGen(&m_resultOrException);
      break;

    case STATE_FAILED:
      tvDecRefObj(&m_resultOrException);
      break;
  }
}

inline ContextIndex c_WaitableWaitHandle::getContextIndex() const {
  assertx(!isFinished());
  return m_ctxStateIdx.contextIndex();
}

inline ContextStateIndex c_WaitableWaitHandle::getContextStateIndex() const {
  assertx(!isFinished());
  return m_ctxStateIdx;
}

inline void
c_WaitableWaitHandle::setContextStateIndex(ContextStateIndex ctxStateIdx) {
  assertx(!isFinished());
  m_ctxStateIdx = ctxStateIdx;
}

inline bool c_WaitableWaitHandle::isInContext() const {
  return getContextIndex().value;
}

inline AsioContext* c_WaitableWaitHandle::getContext() const {
  assertx(isInContext());
  return AsioSession::Get()->getContext(getContextIndex());
}

inline AsioBlockableChain& c_WaitableWaitHandle::getParentChain() {
  assertx(!isFinished());
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
