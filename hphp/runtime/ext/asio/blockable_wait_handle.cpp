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

#include "hphp/runtime/ext/asio/blockable_wait_handle.h"

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/waitable_wait_handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void c_BlockableWaitHandle::t___construct() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

c_BlockableWaitHandle* c_BlockableWaitHandle::unblock() {
  c_BlockableWaitHandle* next = m_nextParent;

  // notify subclass that we are no longer blocked
  onUnblocked();

  // decrement ref count, we can't be called by child anymore
  decRefObj(this);

  return next;
}

void c_BlockableWaitHandle::exitContextBlocked(context_idx_t ctx_idx) {
  assert(getState() == STATE_BLOCKED);
  assert(AsioSession::Get()->getContext(ctx_idx));

  // not in a context being exited
  assert(getContextIdx() <= ctx_idx);
  if (getContextIdx() != ctx_idx) {
    return;
  }

  // move us to the parent context
  setContextIdx(getContextIdx() - 1);

  // recursively move all wait handles blocked by us
  for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
    pwh->exitContextBlocked(ctx_idx);
  }
}

// throws if establishing a dependency from this to child would form a cycle
void c_BlockableWaitHandle::detectCycle(c_WaitableWaitHandle* child) const {
  if (UNLIKELY(isDescendantOf(child))) {
    Object e(createCycleException(child));
    throw e;
  }
}

ObjectData*
c_BlockableWaitHandle::createCycleException(c_WaitableWaitHandle* child) const {
  assert(isDescendantOf(child));

  smart::vector<std::string> exception_msg_items;
  exception_msg_items.push_back("Encountered dependency cycle.\n");
  exception_msg_items.push_back("Existing stack:\n");

  exception_msg_items.push_back(folly::stringPrintf(
    "  %s (%" PRId64 ")\n", child->getName().data(), child->t_getid()));

  assert(dynamic_cast<c_BlockableWaitHandle*>(child));
  auto current = static_cast<c_BlockableWaitHandle*>(child);

  while (current != this) {
    assert(current->getState() == STATE_BLOCKED);
    assert(dynamic_cast<c_BlockableWaitHandle*>(current->getChild()));
    current = static_cast<c_BlockableWaitHandle*>(current->getChild());

    exception_msg_items.push_back(folly::stringPrintf(
      "  %s (%" PRId64 ")\n", current->getName().data(), current->t_getid()));
  }

  exception_msg_items.push_back("Trying to introduce dependency on:\n");
  exception_msg_items.push_back(folly::stringPrintf(
    "  %s (%" PRId64 ") (dupe)\n", child->getName().data(), child->t_getid()));
  return SystemLib::AllocInvalidOperationExceptionObject(
      folly::join("", exception_msg_items));
}

///////////////////////////////////////////////////////////////////////////////
}
