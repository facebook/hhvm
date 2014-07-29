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

#include "hphp/runtime/ext/asio/sleep_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_blockable.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_sleep("<sleep>");
}

void c_SleepWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  AsioSession::Get()->setOnSleepCreateCallback(callback);
}

void c_SleepWaitHandle::ti_setonsuccesscallback(const Variant& callback) {
  AsioSession::Get()->setOnSleepSuccessCallback(callback);
}

Object c_SleepWaitHandle::ti_create(int64_t usecs) {
  if (UNLIKELY(usecs < 0)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected usecs to be a non-negative integer"));
    throw e;
  }

  c_SleepWaitHandle* wh = NEWOBJ(c_SleepWaitHandle);
  wh->initialize(usecs);
  return wh;
}

void c_SleepWaitHandle::initialize(int64_t usecs) {
  setState(STATE_WAITING);
  m_waketime =
    AsioSession::TimePoint::clock::now() +
    std::chrono::microseconds(usecs);

  incRefCount();

  auto session = AsioSession::Get();
  session->getSleepEventQueue().push(this);

  if (isInContext()) {
    registerToContext();
  }

  if (UNLIKELY(session->hasOnSleepCreateCallback())) {
    session->onSleepCreate(this);
  }
}

void c_SleepWaitHandle::process() {
  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    unregisterFromContext();
  }

  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  tvWriteNull(&m_resultOrException);
  parentChain.unblock();

  auto session = AsioSession::Get();
  if (UNLIKELY(session->hasOnSleepSuccessCallback())) {
    session->onSleepSuccess(this);
  }
}

String c_SleepWaitHandle::getName() {
  return s_sleep;
}

void c_SleepWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    unregisterFromContext();
  }

  setContextIdx(ctx_idx);
  registerToContext();
}

void c_SleepWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));
  assert(getContextIdx() == ctx_idx);
  assert(getState() == STATE_WAITING);

  // Move us to the parent context.
  setContextIdx(getContextIdx() - 1);

  // Re-register if still in a context.
  if (isInContext()) {
    registerToContext();
  }

  // Recursively move all wait handles blocked by us.
  getParentChain().exitContext(ctx_idx);
}

void c_SleepWaitHandle::registerToContext() {
  AsioContext *ctx = getContext();
  m_ctxVecIndex = ctx->registerTo(ctx->getSleepEvents(), this);
}

void c_SleepWaitHandle::unregisterFromContext() {
  AsioContext *ctx = getContext();
  ctx->unregisterFrom(ctx->getSleepEvents(), m_ctxVecIndex);
}

///////////////////////////////////////////////////////////////////////////////
}
