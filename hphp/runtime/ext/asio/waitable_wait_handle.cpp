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
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_WaitableWaitHandle::c_WaitableWaitHandle(VM::Class* cb)
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

Array c_WaitableWaitHandle::t_getstacktrace() {
  // no parent data available if finished
  if (isFinished()) {
    return Array::Create();
  }

  /**
   * At this point, we have an unfinished wait handle and by definition, all
   * recursive parents are unfinished as well. Let's walk thru them up the
   * stack and populate the output array. One particularly tricky case is to
   * correctly follow cross-context boundaries. Sometimes, a wait handle
   * originally created in the parent context gets imported and following
   * its original parents may skip one or more context boundaries. To avoid
   * this, only parents from the same context are considered. A condition
   * where no such parent exists indicates a wait handle that was join()-ed
   * from the parent context.
   */
  AsioSession* session = AsioSession::Get();
  Array result = Array::Create(this);
  c_WaitableWaitHandle* curr = this;

  while (true) {
    // find parent in the same context (context may be null)
    context_idx_t ctx_idx = curr->getContextIdx();
    curr = curr->getParentInContext(ctx_idx);

    // continue up the stack within the same context
    if (curr) {
      result.append(curr);
      continue;
    }

    // the whole dependency chain is outside of context
    if (!ctx_idx) {
      return result;
    }

    // cross the boundary; keep crossing until non-empty context is found
    do {
      --ctx_idx;
      result.append(null_variant);
    } while (ctx_idx && !(curr = session->getContext(ctx_idx)->getCurrent()));

    // all contexts processed
    if (!ctx_idx) {
      return result;
    }

    result.append(curr);
  }
}

c_BlockableWaitHandle* c_WaitableWaitHandle::addParent(c_BlockableWaitHandle* parent) {
  c_BlockableWaitHandle* prev = m_firstParent;
  m_firstParent = parent;
  return prev;
}

void c_WaitableWaitHandle::setResult(const TypedValue* result) {
  assert(result);
  assert(result->m_type != KindOfRef);

  setState(STATE_SUCCEEDED);
  tvDupCell(result, &m_resultOrException);

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

c_BlockableWaitHandle* c_WaitableWaitHandle::getParentInContext(context_idx_t ctx_idx) {
  c_BlockableWaitHandle* parent = m_firstParent;

  while (parent && parent->getContextIdx() != ctx_idx) {
    parent = parent->getNextParent();
  }

  return parent;
}

// throws on context depth level overflows and cross-context cycles
void c_WaitableWaitHandle::join() {
  AsioSession* session = AsioSession::Get();

  assert(!isFinished());
  assert(!session->isInContext() || session->getCurrentContext()->isRunning());

  // enter new asio context and set up guard that will exit once we are done
  session->enterContext();

  assert(session->isInContext());
  assert(!session->getCurrentContext()->isRunning());

  try {
    // import this wait handle to the newly created context
    // throws if cross-context cycle found
    enterContext(session->getCurrentContextIdx());

    // run queues until we are finished
    session->getCurrentContext()->runUntil(this);
  } catch (const Object& exception) {
    // recover from PHP exceptions; HPHP internal exceptions are deliberately
    // ignored as there is no easy way to recover from them
    session->exitContext();
    throw;
  }
  session->exitContext();

  assert(isFinished());
}

c_WaitableWaitHandle* c_WaitableWaitHandle::getChild() {
  assert(!isFinished());

  // waitable wait handle does not have any child
  return nullptr;
}

bool c_WaitableWaitHandle::hasCycle(c_WaitableWaitHandle* start) {
  assert(start);

  while (start != this && start && !start->isFinished()) {
    start = start->getChild();
  }

  return start == this;
}

///////////////////////////////////////////////////////////////////////////////
}
