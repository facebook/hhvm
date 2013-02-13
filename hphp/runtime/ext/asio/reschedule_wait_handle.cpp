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

namespace {
  StaticString s_reschedule("<reschedule>");
}

const int q_RescheduleWaitHandle$$QUEUE_DEFAULT = AsioContext::QUEUE_DEFAULT;
const int q_RescheduleWaitHandle$$QUEUE_NO_PENDING_IO = AsioContext::QUEUE_NO_PENDING_IO;

c_RescheduleWaitHandle::c_RescheduleWaitHandle(
    const ObjectStaticCallbacks *cb)
    : c_WaitableWaitHandle(cb) {
}

c_RescheduleWaitHandle::~c_RescheduleWaitHandle() {
}

void c_RescheduleWaitHandle::t___construct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(RescheduleWaitHandle, RescheduleWaitHandle::__construct);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use RescheduleWaitHandle::create() instead of constructor"));
  throw e;
}

Object c_RescheduleWaitHandle::ti_create(const char* cls, int queue, int priority) {
  if (UNLIKELY(
      queue != q_RescheduleWaitHandle$$QUEUE_DEFAULT &&
      queue != q_RescheduleWaitHandle$$QUEUE_NO_PENDING_IO)) {
    STATIC_METHOD_INJECTION_BUILTIN(RescheduleWaitHandle, RescheduleWaitHandle::create);
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected queue to be a value defined by one of the QUEUE_ constants"));
    throw e;
  }

  if (UNLIKELY(priority < 0)) {
    STATIC_METHOD_INJECTION_BUILTIN(RescheduleWaitHandle, RescheduleWaitHandle::create);
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected priority to be a non-negative integer"));
    throw e;
  }

  c_RescheduleWaitHandle* wh = NEWOBJ(c_RescheduleWaitHandle);
  wh->initialize(static_cast<uint32_t>(queue), static_cast<uint32_t>(priority));
  return wh;
}

void c_RescheduleWaitHandle::initialize(uint32_t queue, uint32_t priority) {
  AsioContext* ctx = AsioSession::GetCurrentContext();

  setContext(ctx);
  m_queue = queue;
  m_priority = priority;

  setState(STATE_SCHEDULED);
  if (ctx) {
    ctx->schedule(this, m_queue, m_priority);
  }
}

void c_RescheduleWaitHandle::run() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    return;
  }

  setResult(init_null_variant.asTypedValue());
}

String c_RescheduleWaitHandle::getName() {
  return s_reschedule;
}

void c_RescheduleWaitHandle::enterContext(AsioContext* ctx) {
  assert(ctx);

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // already in the more specific context?
  if (LIKELY(ctx->includes(getContext()))) {
    return;
  }

  if (UNLIKELY(getState() != STATE_SCHEDULED)) {
    throw new FatalErrorException(
        "Invariant violation: encountered unexpected state");
  }

  setContext(ctx);
  ctx->schedule(this, m_queue, m_priority);
}

void c_RescheduleWaitHandle::exitContext(AsioContext* ctx) {
  assert(ctx);

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // not in a context being exited
  if (ctx != getContext()) {
    return;
  }

  if (UNLIKELY(getState() != STATE_SCHEDULED)) {
    throw new FatalErrorException(
        "Invariant violation: encountered unexpected state");
  }

  AsioContext* pctx = ctx->getParent();
  setContext(pctx);
  if (pctx) {
    pctx->schedule(this, m_queue, m_priority);
  }

  // recursively move all wait handles blocked by us
  for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
    pwh->exitContextBlocked(ctx);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
