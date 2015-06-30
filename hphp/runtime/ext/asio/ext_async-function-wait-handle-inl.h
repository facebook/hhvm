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

#ifndef incl_HPHP_EXT_ASIO_ASYNC_FUNCTION_WAIT_HANDLE_H_
#error "This should only be included by ext_async-function-wait-handle.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void
c_AsyncFunctionWaitHandle::Node::setChild(c_WaitableWaitHandle* child) {
  assert(child);
  assert(!child->isFinished());

  m_child = child;
  m_child->getParentChain()
    .addParent(m_blockable, AsioBlockable::Kind::AsyncFunctionWaitHandleNode);
}

inline c_WaitableWaitHandle*
c_AsyncFunctionWaitHandle::Node::getChild() const {
  return m_child;
}

inline bool
c_AsyncFunctionWaitHandle::Node::isFirstUnfinishedChild() const {
  return true;
}

inline c_AsyncFunctionWaitHandle*
c_AsyncFunctionWaitHandle::Node::getWaitHandle() const {
  return reinterpret_cast<c_AsyncFunctionWaitHandle*>(
    const_cast<char*>(
      reinterpret_cast<const char*>(this) -
      c_AsyncFunctionWaitHandle::childrenOff()));
}

inline void
c_AsyncFunctionWaitHandle::Node::onUnblocked() {
  getWaitHandle()->onUnblocked();
}

///////////////////////////////////////////////////////////////////////////////
}
