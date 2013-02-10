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
#include <runtime/ext/asio/asio_context.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_BlockableWaitHandle::c_BlockableWaitHandle(const ObjectStaticCallbacks *cb)
    : c_WaitableWaitHandle(cb), m_nextParent(nullptr) {
}

c_BlockableWaitHandle::~c_BlockableWaitHandle() {
}

void c_BlockableWaitHandle::t___construct() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

c_WaitableWaitHandle* c_BlockableWaitHandle::getBlockedOn() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

void c_BlockableWaitHandle::blockOn(c_WaitableWaitHandle* child) {
  setState(STATE_BLOCKED);

  // make sure the child is going to do some work
  if (getContext()) {
    child->enterContext(getContext());
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

void c_BlockableWaitHandle::onUnblocked() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

void c_BlockableWaitHandle::failBlock(CObjRef exception) {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

void c_BlockableWaitHandle::exitContextBlocked(AsioContext* ctx) {
  assert(ctx);
  assert(getState() == STATE_BLOCKED);

  // not in a context being exited
  if (ctx != getContext()) {
    return;
  }

  // move us to the parent context
  setContext(ctx->getParent());

  // recursively move all wait handles blocked by us
  for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
    pwh->exitContextBlocked(ctx);
  }
}

void c_BlockableWaitHandle::killCycle() {
  if (getState() != STATE_BLOCKED) {
    throw FatalErrorException(
        "Invariant violation: trying to kill non-existing cycle starting with "
        "wait handle that is not blocked");
  }

  std::vector<std::string> exception_msg_items;
  exception_msg_items.push_back("Encountered dependency cycle:\n");

  c_BlockableWaitHandle* current = this;
  c_WaitableWaitHandle* next = current->getBlockedOn();
  hphp_hash_set<void*> visited;

  visited.insert(current);
  exception_msg_items.push_back(folly::stringPrintf(
      "  %s (%" PRId64 ")\n", current->getName()->data(), current->t_getid()));

  while (visited.find(next) == visited.end()) {
    visited.insert(next);
    exception_msg_items.push_back(folly::stringPrintf(
        "  %s (%" PRId64 ")\n", next->getName()->data(), next->t_getid()));

    current = dynamic_cast<c_BlockableWaitHandle*>(next);

    if (!current || current->getState() != STATE_BLOCKED) {
      throw FatalErrorException(
          "Invariant violation: trying to kill non-existing cycle containing "
          "wait handle that is not blocked");
    }

    next = current->getBlockedOn();
  }

  // cycle found; let's kill current -> next edge

  // generate the exception
  exception_msg_items.push_back(folly::stringPrintf(
      "  %s (%" PRId64 ") (dupe)\n", next->getName()->data(), next->t_getid()));
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
      folly::join("", exception_msg_items)));

  // find us in our child's list of parents
  c_BlockableWaitHandle** next_parent_ptr = next->getFirstParentPtr();
  while (*next_parent_ptr != current) {
    assert(*next_parent_ptr);
    next_parent_ptr = &((*next_parent_ptr)->m_nextParent);
  }

  // and remove us from that list
  *next_parent_ptr = current->m_nextParent;

  // replace block with an exception
  current->failBlock(e.get());

  // decrement ref count held by our child
  decRefObj(current);
}

///////////////////////////////////////////////////////////////////////////////
}
