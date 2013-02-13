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

c_ContinuationWaitHandle::c_ContinuationWaitHandle(
    const ObjectStaticCallbacks *cb)
    : c_BlockableWaitHandle(cb), m_continuation(), m_child(), m_privData(),
      m_depth(0), m_tailCall(false) {
}

c_ContinuationWaitHandle::~c_ContinuationWaitHandle() {
}

void c_ContinuationWaitHandle::t___construct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(ContinuationWaitHandle, ContinuationWaitHandle::__construct);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use ContinuationWaitHandle::start() instead of constructor"));
  throw e;
}

Object c_ContinuationWaitHandle::ti_start(const char* cls, CObjRef continuation, int prio) {
  AsioContext* ctx = AsioSession::GetCurrentContext();
  if (UNLIKELY(!continuation.instanceof(s_continuation))) {
    STATIC_METHOD_INJECTION_BUILTIN(ContinuationWaitHandle, ContinuationWaitHandle::start);
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected continuation to be an instance of Continuation"));
    throw e;
  }

  if (UNLIKELY(prio < 0)) {
    STATIC_METHOD_INJECTION_BUILTIN(ContinuationWaitHandle, ContinuationWaitHandle::start);
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected prio to be a non-negative integer"));
    throw e;
  }

  uint16_t depth = ctx ? ctx->getCurrentWaitHandleDepth() : 0;
  if (UNLIKELY(depth >= MAX_DEPTH)) {
    STATIC_METHOD_INJECTION_BUILTIN(ContinuationWaitHandle, ContinuationWaitHandle::start);
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Asio stack overflow"));
    throw e;
  }

  c_Continuation* cont = static_cast<c_Continuation*>(continuation.get());
  if (!cont->m_waitHandle.isNull()) {
    if (ctx) {
      cont->m_waitHandle->enterContext(ctx);
    }
    return cont->m_waitHandle;
  }

  c_ContinuationWaitHandle* wh = NEWOBJ(c_ContinuationWaitHandle)();
  wh->start(cont, static_cast<uint32_t>(prio), depth + 1);
  return wh;
}

void c_ContinuationWaitHandle::ti_markcurrentassucceeded(const char* cls, CVarRef result) {
  AsioContext* ctx = AsioSession::GetCurrentContext();
  c_ContinuationWaitHandle* wh = ctx ? ctx->getCurrent() : nullptr;
  if (!wh) {
    STATIC_METHOD_INJECTION_BUILTIN(ContinuationWaitHandle, ContinuationWaitHandle::markcurrentassucceeded);
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Unable to set result: no continuation running"));
    throw e;
  }

  wh->markAsSucceeded(tvToCell(result.asTypedValue()));
}

void c_ContinuationWaitHandle::ti_markcurrentastailcall(const char* cls) {
  AsioContext* ctx = AsioSession::GetCurrentContext();
  c_ContinuationWaitHandle* wh = ctx ? ctx->getCurrent() : nullptr;
  if (!wh) {
    STATIC_METHOD_INJECTION_BUILTIN(ContinuationWaitHandle, ContinuationWaitHandle::markcurrentastailcall);
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Unable to setup tail call: no continuation running"));
    throw e;
  }

  wh->m_tailCall = true;
}

Object c_ContinuationWaitHandle::t_getprivdata() {
  return m_privData;
}

void c_ContinuationWaitHandle::t_setprivdata(CObjRef data) {
  m_privData = data;
}

void c_ContinuationWaitHandle::start(c_Continuation* continuation, uint32_t prio, uint16_t depth) {
  AsioContext* ctx = AsioSession::GetCurrentContext();

  setContext(ctx);
  m_continuation = continuation;
  m_child = nullptr;
  m_privData = nullptr;
  m_depth = depth;
  m_tailCall = false;
  continuation->m_waitHandle = this;

  if (prio == 0) {
    setState(STATE_SCHEDULED);
    if (ctx) {
      ctx->schedule(this);
    }
  } else {
    // TODO: deprecate directly passed non-zero priorities
    m_child = c_RescheduleWaitHandle::t_create(AsioContext::QUEUE_DEFAULT, prio);
    blockOn(static_cast<c_WaitableWaitHandle*>(m_child.get()));
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
      if (m_tailCall) {
        if (m_child.isNull()) {
          markAsSucceeded(init_null_variant.asTypedValue());
          return;
        } else if (m_child->isSucceeded()) {
          markAsSucceeded(m_child->getResult());
          return;
        } else {
          m_tailCall = false;
        }
      }

      // iterate continuation
      if (m_child.isNull()) {
        // first iteration or null dependency
        m_continuation->call_next();
      } else if (m_child->isSucceeded()) {
        // child succeeded, pass the result to the continuation
        if (IS_NULL_TYPE(m_child->getResult()->m_type)) {
          // FIXME: may happen due to RescheduleWaitHandle
          m_continuation->call_next();
        } else {
          m_continuation->call_send(m_child->getResult());
        }
      } else if (m_child->isFailed()) {
        // child failed, raise the exception inside continuation
        m_continuation->call_raise(m_child->getException());
      } else {
        throw FatalErrorException(
            "Invariant violation: child neither succeeded nor failed");
      }

      // continuation was marked as finished via markCurrentAsFinished()
      if (m_continuation.isNull()) {
        return;
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
      } else if (value->m_type == KindOfArray) {
        // array of dependencies; TODO: deprecate this
        m_child = c_GenArrayWaitHandle::t_create(value->m_data.parr);
      } else {
        c_WaitHandle* child = c_WaitHandle::fromTypedValue(value);
        if (UNLIKELY(!child)) {
          Object e(SystemLib::AllocInvalidArgumentExceptionObject(
              "Expected yield argument to be an instance of WaitHandle"));
          throw e;
        }
        m_child = child;
      }
    } while (m_child.isNull() || m_child->isFinished());

    // we are blocked on m_child so it must be WaitableWaitHandle
    assert(dynamic_cast<c_WaitableWaitHandle*>(m_child.get()));
    blockOn(static_cast<c_WaitableWaitHandle*>(m_child.get()));
  } catch (Object exception) {
    markAsFailed(exception);
  }
}

c_WaitableWaitHandle* c_ContinuationWaitHandle::getBlockedOn() {
  assert(getState() == STATE_BLOCKED);
  assert(dynamic_cast<c_WaitableWaitHandle*>(m_child.get()));
  return static_cast<c_WaitableWaitHandle*>(m_child.get());
}

void c_ContinuationWaitHandle::onUnblocked() {
  setState(STATE_SCHEDULED);
  if (getContext()) {
    getContext()->schedule(this);
  }
}

void c_ContinuationWaitHandle::failBlock(CObjRef exception) {
  m_child = c_StaticExceptionWaitHandle::t_create(exception);
  onUnblocked();
}

void c_ContinuationWaitHandle::markAsSucceeded(const TypedValue* result) {
  setResult(result);

  // free m_continuation / m_child later, result may be stored there
  m_continuation = nullptr;
  m_child = nullptr;
}

void c_ContinuationWaitHandle::markAsFailed(CObjRef exception) {
  AsioSession::OnFailed(exception);
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
      throw new FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

void c_ContinuationWaitHandle::enterContext(AsioContext* ctx) {
  assert(ctx);

  // stop before corrupting unioned data
  if (getState() == STATE_SUCCEEDED || getState() == STATE_FAILED) {
    return;
  }

  // already in the more specific context?
  if (LIKELY(ctx->includes(getContext()))) {
    return;
  }

  switch (getState()) {
    case STATE_BLOCKED:
      // enter child into new context recursively
      setContext(ctx);
      m_child->enterContext(ctx);
      break;

    case STATE_SCHEDULED:
      // reschedule so that we get run
      setContext(ctx);
      ctx->schedule(this);
      break;

    case STATE_RUNNING: {
      INSTANCE_METHOD_INJECTION_BUILTIN(WaitHandle, WaitHandle::import);
      Object e(SystemLib::AllocInvalidOperationExceptionObject(
          "Detected cross-context dependency cycle. You are trying to depend "
          "on something that is running you serially."));
      throw e;
    }

    default:
      throw new FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

void c_ContinuationWaitHandle::exitContext(AsioContext* ctx) {
  assert(ctx);

  // stop before corrupting unioned data
  if (getState() == STATE_SUCCEEDED || getState() == STATE_FAILED) {
    return;
  }

  // not in a context being exited
  if (ctx != getContext()) {
    return;
  }

  switch (getState()) {
    case STATE_BLOCKED:
      // we were already ran due to duplicit scheduling; the context will be
      // updated thru exitContext() call on the non-blocked wait handle we
      // recursively depend on
      break;

    case STATE_SCHEDULED: {
      // move us to the parent context
      AsioContext* pctx = ctx->getParent();
      setContext(pctx);
      if (pctx) {
        pctx->schedule(this);
      }

      // recursively move all wait handles blocked by us
      for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
        pwh->exitContextBlocked(ctx);
      }

      break;
    }

    default:
      throw new FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
