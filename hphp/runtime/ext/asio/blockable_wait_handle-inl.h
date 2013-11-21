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

#ifndef incl_HPHP_EXT_ASIO_BLOCKABLE_WAIT_HANDLE_H_
#error "This should only be included by blockable_wait_handle.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void
c_BlockableWaitHandle::blockOn(c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);

  assert(!child->isFinished());
  assert(getChild() == child);
  assert(getContextIdx() <= child->getContextIdx());
  assert(!isDescendantOf(child));

  // Extend the linked list of parents.
  m_nextParent = child->addParent(this);

  // Increment ref count so that we won't disappear before child calls back.
  incRefCount();
}

///////////////////////////////////////////////////////////////////////////////
}
