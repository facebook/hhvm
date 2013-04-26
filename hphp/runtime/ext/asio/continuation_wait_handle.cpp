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
#include <runtime/ext/ext_closure.h>
#include <runtime/ext/ext_continuation.h>
#include <runtime/ext/asio/asio_context.h>
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  // max depth of continuation
  const uint16_t MAX_DEPTH = 512;

  StaticString s_continuationResult("<continuation-result>");
  StaticString s_continuationException("<continuation-exception>");
  StaticString s_continuation("Continuation");
}

c_ContinuationWaitHandle::c_ContinuationWaitHandle(VM::Class* cb)
    : c_BlockableWaitHandle(cb), m_continuation(), m_child(), m_privData(),
      m_depth(0) {
}

c_ContinuationWaitHandle::~c_ContinuationWaitHandle() {
}

void c_ContinuationWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use $continuation->getWaitHandle() instead of constructor"));
  throw e;
}

void c_ContinuationWaitHandle::ti_setoncreatecallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ContinuationWaitHandle::onStart: on_start_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnContinuationCreateCallback(callback.getObjectDataOrNull());
}

void c_ContinuationWaitHandle::ti_setonyieldcallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ContinuationWaitHandle::onYield: on_yield_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnContinuationYieldCallback(callback.getObjectDataOrNull());
}

void c_ContinuationWaitHandle::ti_setonsuccesscallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ContinuationWaitHandle::onSuccess: on_success_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnContinuationSuccessCallback(callback.getObjectDataOrNull());
}

void c_ContinuationWaitHandle::ti_setonfailcallback(CVarRef callback) {
  if (!callback.isNull() && !callback.instanceof(c_Closure::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set ContinuationWaitHandle::onFail: on_fail_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnContinuationFailCallback(callback.getObjectDataOrNull());
}

void c_ContinuationWaitHandle::Create(c_Continuation* continuation) {
  assert(continuation);
  assert(continuation->m_waitHandle.isNull());

  AsioSession* session = AsioSession::Get();
  uint16_t depth = session->getCurrentWaitHandleDepth();
  if (UNLIKELY(depth >= MAX_DEPTH)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Asio stack overflow"));
    throw e;
  }

  if (UNLIKELY(continuation->m_index != -1)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      continuation->m_running
      ? "Encountered an attempt to start currently running continuation"
      : "Encountered an attempt to start tainted continuation"));
    throw e;
  }

  continuation->m_waitHandle = NEWOBJ(c_ContinuationWaitHandle)();
  continuation->m_waitHandle->initialize(continuation, depth + 1);

  // needs to be called after continuation->m_waitHandle is set
  if (UNLIKELY(session->hasOnContinuationCreateCallback())) {
    session->onContinuationCreate(continuation->m_waitHandle.get());
  }
}

Object c_ContinuationWaitHandle::t_getprivdata() {
  return m_privData;
}

void c_ContinuationWaitHandle::t_setprivdata(CObjRef data) {
  m_privData = data;
}

void c_ContinuationWaitHandle::initialize(c_Continuation* continuation, uint16_t depth) {
  m_continuation = continuation;
  m_child = nullptr;
  m_privData = nullptr;
  m_depth = depth;

  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this);
  }
}

void c_ContinuationWaitHandle::run() {
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
      if (m_continuation->m_done) {
        markAsSucceeded(m_continuation->m_value.asTypedValue());
        return;
      }

      // set up dependency
      TypedValue* value = m_continuation->m_value.asTypedValue();
      if (IS_NULL_TYPE(value->m_type)) {
        // null dependency
        m_child = nullptr;
      } else {
        c_WaitHandle* child = c_WaitHandle::fromTypedValue(value);
        if (UNLIKELY(!child)) {
          Object e(SystemLib::AllocInvalidArgumentExceptionObject(
              "Expected yield argument to be an instance of WaitHandle"));
          throw e;
        }

        AsioSession* session = AsioSession::Get();
        if (UNLIKELY(session->hasOnContinuationYieldCallback())) {
          session->onContinuationYield(this, child);
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
  }
}

void c_ContinuationWaitHandle::onUnblocked() {
  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this);
  }
}

void c_ContinuationWaitHandle::markAsSucceeded(const TypedValue* result) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnContinuationSuccessCallback())) {
    session->onContinuationSuccess(this, tvAsCVarRef(result));
  }

  setResult(result);

  // free m_continuation / m_child later, result may be stored there
  m_continuation = nullptr;
  m_child = nullptr;
}

void c_ContinuationWaitHandle::markAsFailed(CObjRef exception) {
  AsioSession* session = AsioSession::Get();
  session->onFailed(exception);
  if (UNLIKELY(session->hasOnContinuationFailCallback())) {
    session->onContinuationFail(this, exception);
  }
  setException(exception.get());

  m_continuation = nullptr;
  m_child = nullptr;
}

String c_ContinuationWaitHandle::getName() {
  switch (getState()) {
    case STATE_SUCCEEDED:
      return s_continuationResult;

    case STATE_FAILED:
      return s_continuationException;

    case STATE_BLOCKED:
    case STATE_SCHEDULED:
    case STATE_RUNNING:
      return m_continuation->t_getorigfuncname();

    default:
      throw FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

c_WaitableWaitHandle* c_ContinuationWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    assert(dynamic_cast<c_WaitableWaitHandle*>(m_child.get()));
    return static_cast<c_WaitableWaitHandle*>(m_child.get());
  } else {
    assert(getState() == STATE_SCHEDULED || getState() == STATE_RUNNING);
    return nullptr;
  }
}

void c_ContinuationWaitHandle::enterContext(context_idx_t ctx_idx) {
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

void c_ContinuationWaitHandle::exitContext(context_idx_t ctx_idx) {
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

///////////////////////////////////////////////////////////////////////////////
}
