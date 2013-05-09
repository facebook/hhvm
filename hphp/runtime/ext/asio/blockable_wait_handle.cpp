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

#include <runtime/ext/ext_asio.h>
#include <runtime/ext/asio/asio_context.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_BlockableWaitHandle::c_BlockableWaitHandle(VM::Class* cb)
    : c_WaitableWaitHandle(cb), m_nextParent(nullptr) {
}

c_BlockableWaitHandle::~c_BlockableWaitHandle() {
}

void c_BlockableWaitHandle::t___construct() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

// throws on cycle
void c_BlockableWaitHandle::blockOn(c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);
  assert(getChild() == child);

  // detect complete cycles
  if (UNLIKELY(hasCycle(child))) {
    reportCycle(child);
    assert(false);
  }

  // make sure the child is going to do some work
  // throws if cross-context cycle found
  if (isInContext()) {
    child->enterContext(getContextIdx());
  }

  // extend the linked list of parents
  m_nextParent = child->addParent(this);

  // increment ref count so that we won't deallocated before child calls back
  incRefCount();
}

c_BlockableWaitHandle* c_BlockableWaitHandle::getNextParent() {
  return m_nextParent;
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

// always throws
void c_BlockableWaitHandle::reportCycle(c_WaitableWaitHandle* start) {
  assert(getState() == STATE_BLOCKED);
  assert(getChild() == start);

  smart::vector<std::string> exception_msg_items;
  exception_msg_items.push_back("Encountered dependency cycle.\n");
  exception_msg_items.push_back("Existing stack:\n");

  assert(dynamic_cast<c_BlockableWaitHandle*>(start));
  auto current = static_cast<c_BlockableWaitHandle*>(start);
  assert(current->getState() == STATE_BLOCKED);

  do {
    exception_msg_items.push_back(folly::stringPrintf(
        "  %s (%" PRId64 ")\n", current->getName()->data(), current->t_getid()));

    auto next = current->getChild();
    assert(dynamic_cast<c_BlockableWaitHandle*>(next));
    current = static_cast<c_BlockableWaitHandle*>(next);
    assert(current->getState() == STATE_BLOCKED);
  } while (current != start);

  exception_msg_items.push_back("Trying to introduce dependency on:\n");
  exception_msg_items.push_back(folly::stringPrintf(
      "  %s (%" PRId64 ") (dupe)\n", start->getName()->data(), start->t_getid()));
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
      folly::join("", exception_msg_items)));
  throw e;
}

///////////////////////////////////////////////////////////////////////////////
}
