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

c_WaitableWaitHandle::c_WaitableWaitHandle(const ObjectStaticCallbacks *cb)
    : c_WaitHandle(cb), m_firstParent(nullptr) {
  setState(STATE_NEW);
  setContextIdx(AsioSession::Get()->getCurrentContextIdx());
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
}

void c_WaitableWaitHandle::t___construct() {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
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

  // unblock parents
  while (m_firstParent) {
    m_firstParent = m_firstParent->unblock();
  }
}

void c_WaitableWaitHandle::setException(ObjectData* exception) {
  assert(exception);
  assert(exception->o_instanceof("Exception"));

  setState(STATE_FAILED);
  tvWriteObject(exception, &m_resultOrException);

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

const TypedValue* c_WaitableWaitHandle::join() {
  INSTANCE_METHOD_INJECTION_BUILTIN(WaitHandle, WaitHandle::join);

  switch (getState()) {
    case STATE_NEW:
      throw FatalErrorException(
          "Invariant violation: uninitialized wait handle");
    case STATE_SUCCEEDED:
      return &m_resultOrException;
    case STATE_FAILED:
      Object e(m_resultOrException.m_data.pobj);
      throw e;
  }

  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(!session->isInContext())) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Unable to join wait handle: not in an asio context"));
    throw e;
  }
  if (UNLIKELY(session->getCurrentContext()->isRunning())) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Unable to join wait handle: context busy, another join in progress"));
    throw e;
  }

  // not finished, run queue
  enterContext(session->getCurrentContextIdx());
  session->getCurrentContext()->runUntil(this);

  switch (getState()) {
    case STATE_NEW:
      throw FatalErrorException(
          "Invariant violation: uninitialized wait handle");
    case STATE_SUCCEEDED:
      return &m_resultOrException;
    case STATE_FAILED:
      Object e(m_resultOrException.m_data.pobj);
      throw e;
  }

  throw FatalErrorException(
      "Invariant violation: join succeeded, but wait handle not ready");
}

void c_WaitableWaitHandle::enterContext(context_idx_t ctx_idx) {
  throw NotSupportedException(__func__, "WTF? This is an abstract class");
}

///////////////////////////////////////////////////////////////////////////////
}
