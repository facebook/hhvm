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

#include "hphp/runtime/ext/ext_asio.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  // max depth of continuation
  const uint16_t MAX_DEPTH = 512;

  StaticString s_continuationResult("<continuation-result>");
  StaticString s_continuationException("<continuation-exception>");
  StaticString s_continuation("Continuation");
}

c_AsyncFunctionWaitHandle::c_AsyncFunctionWaitHandle(Class* cb)
    : c_BlockableWaitHandle(cb), m_continuation(), m_child(), m_privData(),
      m_depth(0) {
}

c_AsyncFunctionWaitHandle::~c_AsyncFunctionWaitHandle() {
}

void c_AsyncFunctionWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use $continuation->getWaitHandle() instead of constructor"));
  throw e;
}

void c_AsyncFunctionWaitHandle::ti_setoncreatecallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::classof())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onStart: on_start_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionCreateCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonawaitcallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::classof())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onAwait: on_await_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionAwaitCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonsuccesscallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::classof())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onSuccess: on_success_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionSuccessCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonfailcallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::classof())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onFail: on_fail_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionFailCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::Create(c_Continuation* continuation) {
  assert(continuation);
  assert(continuation->m_waitHandle.isNull());

  AsioSession* session = AsioSession::Get();
  uint16_t depth = session->getCurrentWaitHandleDepth();
  if (UNLIKELY(depth >= MAX_DEPTH)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Asio stack overflow"));
    throw e;
  }

  if (UNLIKELY(continuation->started())) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      continuation->running()
      ? "Encountered an attempt to start currently running continuation"
      : "Encountered an attempt to start tainted continuation"));
    throw e;
  }

  continuation->m_waitHandle = NEWOBJ(c_AsyncFunctionWaitHandle)();
  continuation->m_waitHandle->initialize(continuation, depth + 1);

  // needs to be called after continuation->m_waitHandle is set
  if (UNLIKELY(session->hasOnAsyncFunctionCreateCallback())) {
    session->onAsyncFunctionCreate(continuation->m_waitHandle.get());
  }
}

Object c_AsyncFunctionWaitHandle::t_getprivdata() {
  return m_privData;
}

void c_AsyncFunctionWaitHandle::t_setprivdata(CObjRef data) {
  m_privData = data;
}

void c_AsyncFunctionWaitHandle::initialize(c_Continuation* continuation, uint16_t depth) {
  m_continuation = continuation;
  m_child = nullptr;
  m_privData = nullptr;
  m_depth = depth;

  setState(STATE_SCHEDULED);

  // In async functions, continutions are created only after the execution is
  // blocked (i.e. with non-zero return label). If this is the case,
  // update the state and child accordingly.
  if (continuation->m_label > 0) {
    m_continuation->start();
    Cell* value = m_continuation->m_value.asCell();
    assert(c_WaitHandle::fromCell(value));
    auto child = static_cast<c_WaitableWaitHandle*>(value->m_data.pobj);
    assert(!child->isFinished());
    m_child = child;
    blockOn(child);
  }
  if (isInContext()) {
    getContext()->schedule(this);
  }
}

void c_AsyncFunctionWaitHandle::run() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    return;
  }

  try {
    setState(STATE_RUNNING);

    do {
      // iterate continuation
      if (m_child.isNull()) {
        // first iteration or null dependency
        m_continuation->call_next();
      } else if (m_child->isSucceeded()) {
        // child succeeded, pass the result to the continuation
        m_continuation->call_send(m_child->getResult());
      } else if (m_child->isFailed()) {
        // child failed, raise the exception inside continuation
        m_continuation->call_raise(m_child->getException());
      } else {
        throw FatalErrorException(
            "Invariant violation: child neither succeeded nor failed");
      }

      // continuation finished, retrieve result from its m_value
      if (m_continuation->done()) {
        markAsSucceeded(*m_continuation->m_value.asCell());
        return;
      }

      // set up dependency
      Cell* value = m_continuation->m_value.asCell();
      if (IS_NULL_TYPE(value->m_type)) {
        // null dependency
        m_child = nullptr;
      } else {
        c_WaitHandle* child = c_WaitHandle::fromCell(value);
        if (UNLIKELY(!child)) {
          Object e(SystemLib::AllocInvalidArgumentExceptionObject(
              "Expected await argument to be an instance of Awaitable"));
          throw e;
        }

        AsioSession* session = AsioSession::Get();
        if (UNLIKELY(session->hasOnAsyncFunctionAwaitCallback())) {
          session->onAsyncFunctionAwait(this, child);
        }

        m_child = child;
      }
    } while (m_child.isNull() || m_child->isFinished());

    // we are blocked on m_child so it must be WaitableWaitHandle
    assert(dynamic_cast<c_WaitableWaitHandle*>(m_child.get()));
    blockOn(static_cast<c_WaitableWaitHandle*>(m_child.get()));
  } catch (const Object& exception) {
    // process exception thrown by generator or blockOn cycle detection
    markAsFailed(exception);
  } catch (...) {
    // process C++ exception
    markAsFailed(AsioSession::Get()->getAbruptInterruptException());
    throw;
  }
}

void c_AsyncFunctionWaitHandle::onUnblocked() {
  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this);
  }
}

void c_AsyncFunctionWaitHandle::markAsSucceeded(const Cell& result) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnAsyncFunctionSuccessCallback())) {
    session->onAsyncFunctionSuccess(this, cellAsCVarRef(result));
  }

  setResult(result);

  // free m_continuation / m_child later, result may be stored there
  m_continuation = nullptr;
  m_child = nullptr;
}

void c_AsyncFunctionWaitHandle::markAsFailed(CObjRef exception) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnAsyncFunctionFailCallback())) {
    session->onAsyncFunctionFail(this, exception);
  }

  setException(exception.get());

  m_continuation = nullptr;
  m_child = nullptr;
}

String c_AsyncFunctionWaitHandle::getName() {
  switch (getState()) {
    case STATE_SUCCEEDED:
      return s_continuationResult;

    case STATE_FAILED:
      return s_continuationException;

    case STATE_BLOCKED:
    case STATE_SCHEDULED:
    case STATE_RUNNING:
      if (m_continuation->t_getcalledclass().empty()) {
        return m_continuation->t_getorigfuncname();
      } else {
        return concat3(m_continuation->t_getcalledclass(),
                       "::",
                       m_continuation->t_getorigfuncname());
      }

    default:
      throw FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

c_WaitableWaitHandle* c_AsyncFunctionWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    assert(dynamic_cast<c_WaitableWaitHandle*>(m_child.get()));
    return static_cast<c_WaitableWaitHandle*>(m_child.get());
  } else {
    assert(getState() == STATE_SCHEDULED || getState() == STATE_RUNNING);
    return nullptr;
  }
}

void c_AsyncFunctionWaitHandle::enterContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // already in the more specific context?
  if (LIKELY(getContextIdx() >= ctx_idx)) {
    return;
  }

  switch (getState()) {
    case STATE_BLOCKED:
      // enter child into new context recursively
      assert(dynamic_cast<c_WaitableWaitHandle*>(m_child.get()));
      static_cast<c_WaitableWaitHandle*>(m_child.get())->enterContext(ctx_idx);
      setContextIdx(ctx_idx);
      break;

    case STATE_SCHEDULED:
      // reschedule so that we get run
      setContextIdx(ctx_idx);
      getContext()->schedule(this);
      break;

    case STATE_RUNNING: {
      Object e(SystemLib::AllocInvalidOperationExceptionObject(
          "Detected cross-context dependency cycle. You are trying to depend "
          "on something that is running you serially."));
      throw e;
    }

    default:
      assert(false);
  }
}

void c_AsyncFunctionWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // not in a context being exited
  assert(getContextIdx() <= ctx_idx);
  if (getContextIdx() != ctx_idx) {
    return;
  }

  switch (getState()) {
    case STATE_BLOCKED:
      // we were already ran due to duplicit scheduling; the context will be
      // updated thru exitContext() call on the non-blocked wait handle we
      // recursively depend on
      break;

    case STATE_SCHEDULED:
      // move us to the parent context
      setContextIdx(getContextIdx() - 1);

      // reschedule if still in a context
      if (isInContext()) {
        getContext()->schedule(this);
      }

      // recursively move all wait handles blocked by us
      for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
        pwh->exitContextBlocked(ctx_idx);
      }

      break;

    default:
      assert(false);
  }
}

// Get the filename in which execution will proceed when execution resumes.
String c_AsyncFunctionWaitHandle::getFileName() {
  if (m_continuation.isNull()) return empty_string;
  auto ar = m_continuation->actRec();
  auto file = ar->m_func->unit()->filepath()->data();
  if (ar->m_func->originalFilename()) {
    file = ar->m_func->originalFilename()->data();
  }
  return file;
}

// Get the line number on which execution will proceed when execution resumes.
int c_AsyncFunctionWaitHandle::getLineNumber() {
  if (m_continuation.isNull()) return -1;
  auto const unit = m_continuation->actRec()->m_func->unit();
  return unit->getLineNumber(m_continuation->getNextExecutionOffset());
}

///////////////////////////////////////////////////////////////////////////////
}
