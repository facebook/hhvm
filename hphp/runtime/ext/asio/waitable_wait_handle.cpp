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

#include "hphp/runtime/ext/asio/waitable_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_WaitableWaitHandle::c_WaitableWaitHandle(Class* cb)
    : c_WaitHandle(cb)
    , m_creator(AsioSession::Get()->getCurrentWaitHandle())
    , m_firstParent(nullptr) {
  setState(STATE_NEW);
  setContextIdx(AsioSession::Get()->getCurrentContextIdx());

  // ref creator
  if (m_creator) {
    m_creator->incRefCount();
  }
}

c_WaitableWaitHandle::~c_WaitableWaitHandle() {
  switch (getState()) {
    case STATE_SUCCEEDED:
      tvRefcountedDecRefCell(&m_resultOrException);
      break;

    case STATE_FAILED:
      tvDecRefObj(&m_resultOrException);
      break;
  }

  // unref creator
  if (m_creator) {
    decRefObj(m_creator);
    m_creator = nullptr;
  }
}

void c_WaitableWaitHandle::t___construct() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

int c_WaitableWaitHandle::t_getcontextidx() {
  return getContextIdx();
}

Object c_WaitableWaitHandle::t_getcreator() {
  return m_creator;
}

Array c_WaitableWaitHandle::t_getparents() {
  // no parent data available if finished
  if (isFinished()) {
    return Array::Create();
  }

  Array result = Array::Create();
  c_BlockableWaitHandle* curr = m_firstParent;

  while (curr) {
    result.append(curr);
    curr = curr->getNextParent();
  }

  return result;
}

void c_WaitableWaitHandle::setResult(const Cell& result) {
  assert(cellIsPlausible(result));

  setState(STATE_SUCCEEDED);
  cellDup(result, m_resultOrException);

  // unref creator
  if (m_creator) {
    decRefObj(m_creator);
    m_creator = nullptr;
  }

  // unblock parents
  while (m_firstParent) {
    m_firstParent = m_firstParent->unblock();
  }
}

void c_WaitableWaitHandle::setException(ObjectData* exception) {
  assert(exception);
  assert(exception->instanceof(SystemLib::s_ExceptionClass));

  setState(STATE_FAILED);
  tvWriteObject(exception, &m_resultOrException);

  // unref creator
  if (m_creator) {
    decRefObj(m_creator);
    m_creator = nullptr;
  }

  // unblock parents
  while (m_firstParent) {
    m_firstParent = m_firstParent->unblock();
  }
}

// throws on context depth level overflows and cross-context cycles
void c_WaitableWaitHandle::join() {
  JIT::EagerVMRegAnchor _;

  AsioSession* session = AsioSession::Get();

  assert(!isFinished());
  assert(!session->isInContext() || session->getCurrentContext()->isRunning());

  if (UNLIKELY(session->hasOnJoinCallback())) {
    session->onJoin(this);
  }

  // enter new asio context and set up guard that will exit once we are done
  session->enterContext();
  auto exit_guard = folly::makeGuard([&] { session->exitContext(); });

  assert(session->isInContext());
  assert(!session->getCurrentContext()->isRunning());

  // import this wait handle to the newly created context
  // throws if cross-context cycle found
  enterContext(session->getCurrentContextIdx());

  // run queues until we are finished
  session->getCurrentContext()->runUntil(this);
  assert(isFinished());
}

c_WaitableWaitHandle* c_WaitableWaitHandle::getChild() {
  assert(!isFinished());

  // waitable wait handle does not have any child
  return nullptr;
}

bool
c_WaitableWaitHandle::isDescendantOf(c_WaitableWaitHandle* wait_handle) const {
  assert(wait_handle);

  while (wait_handle != this && wait_handle && !wait_handle->isFinished()) {
    wait_handle = wait_handle->getChild();
  }

  return wait_handle == this;
}

Array c_WaitableWaitHandle::t_getdependencystack() {
  Array result = Array::Create();
  if (isFinished()) return result;
  hphp_hash_set<int64_t> visited;
  auto wait_handle = this;
  while (wait_handle != nullptr) {
    result.append(wait_handle);
    visited.insert(wait_handle->t_getid());
    auto context_idx = wait_handle->getContextIdx();

    // 1. find parent in the same context
    auto p = wait_handle->getFirstParent();
    while (p) {
      if ((p->getContextIdx() == context_idx) &&
          visited.find(p->t_getid()) == visited.end()) {
        wait_handle = p;
        break;
      }
      p = p->getNextParent();
    }
    if (p) continue;

    // 2. follow creator
    if (m_creator && !m_creator->isFinished() &&
        (m_creator->getContextIdx() == context_idx) &&
        visited.find(m_creator->t_getid()) == visited.end()) {
      wait_handle = m_creator;
      continue;
    }

    // 3. cross the context boundary
    result.append(null_object);
    wait_handle = (context_idx > 1)
      ? AsioSession::Get()->getContext(context_idx - 1)->getCurrent()
      : nullptr;
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}
