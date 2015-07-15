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

#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event-queue.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_externalThreadEvent("<external-thread-event>");
}

void c_ExternalThreadEventWaitHandle::ti_setoncreatecallback(
  const Variant& callback
) {
  AsioSession::Get()->setOnExternalThreadEventCreate(callback);
}

void c_ExternalThreadEventWaitHandle::ti_setonsuccesscallback(
  const Variant& callback
) {
  AsioSession::Get()->setOnExternalThreadEventSuccess(callback);
}

void c_ExternalThreadEventWaitHandle::ti_setonfailcallback(
  const Variant& callback
) {
  AsioSession::Get()->setOnExternalThreadEventFail(callback);
}

void c_ExternalThreadEventWaitHandle::sweep() {
  assert(getState() == STATE_WAITING);

  if (m_event->cancel()) {
    // canceled; the processing thread will take care of cleanup
    return;
  }

  // event has finished, but process() was not called yet
  auto queue = AsioSession::Get()->getExternalThreadEventQueue();
  bool done = queue->hasReceived() && queue->abandonAllReceived(this);
  while (!done) {
    queue->receiveSome();
    done = queue->abandonAllReceived(this);
  }
}

req::ptr<c_ExternalThreadEventWaitHandle>
c_ExternalThreadEventWaitHandle::Create(
  AsioExternalThreadEvent* event,
  ObjectData* priv_data
) {
  auto wh = req::make<c_ExternalThreadEventWaitHandle>();
  wh->initialize(event, priv_data);
  return wh;
}

void c_ExternalThreadEventWaitHandle::initialize(
  AsioExternalThreadEvent* event,
  ObjectData* priv_data
) {
  auto const session = AsioSession::Get();
  setState(STATE_WAITING);
  setContextIdx(session->getCurrentContextIdx());
  m_event = event;
  m_privData = priv_data;

  if (isInContext()) {
    registerToContext();
  }

  if (UNLIKELY(session->hasOnExternalThreadEventCreate())) {
    session->onExternalThreadEventCreate(this);
  }
}

void c_ExternalThreadEventWaitHandle::destroyEvent(bool sweeping /*= false */) {
  // destroy event and its private data
  m_event->release();
  m_event = nullptr;

  // unregister Sweepable
  m_sweepable.unregister();

  if (LIKELY(!sweeping)) {
    m_privData = nullptr;
    // drop ownership by pending event (see initialize())
    decRefObj(this);
  }
}

void c_ExternalThreadEventWaitHandle::abandon(bool sweeping) {
  assert(getState() == STATE_WAITING);
  assert(hasExactlyOneRef() || sweeping);

  if (isInContext()) {
    unregisterFromContext();
  }

  // clean up
  destroyEvent(sweeping);
}

void c_ExternalThreadEventWaitHandle::process() {
  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    unregisterFromContext();
  }

  // clean up once event is processed
  auto exit_guard = folly::makeGuard([&] { destroyEvent(); });

  Cell result;
  try {
    m_event->unserialize(result);
  } catch (const Object& exception) {
    assert(exception->instanceof(SystemLib::s_ExceptionClass));
    auto parentChain = getParentChain();
    setState(STATE_FAILED);
    tvWriteObject(exception.get(), &m_resultOrException);
    parentChain.unblock();

    auto session = AsioSession::Get();
    if (UNLIKELY(session->hasOnExternalThreadEventFail())) {
      session->onExternalThreadEventFail(this, exception);
    }
    return;
  } catch (...) {
    auto parentChain = getParentChain();
    setState(STATE_FAILED);
    tvWriteObject(AsioSession::Get()->getAbruptInterruptException(),
                  &m_resultOrException);
    parentChain.unblock();
    throw;
  }

  assert(cellIsPlausible(result));
  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  cellCopy(result, m_resultOrException);
  parentChain.unblock();

  auto session = AsioSession::Get();
  if (UNLIKELY(session->hasOnExternalThreadEventSuccess())) {
    session->onExternalThreadEventSuccess(this, tvAsCVarRef(&result));
  }
}

String c_ExternalThreadEventWaitHandle::getName() {
  return s_externalThreadEvent;
}

void c_ExternalThreadEventWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));
  assert(getState() == STATE_WAITING);
  assert(getContextIdx() == ctx_idx);

  // Move us to the parent context.
  setContextIdx(getContextIdx() - 1);

  // Re-register if still in a context.
  if (isInContext()) {
    registerToContext();
  }

  // Recursively move all wait handles blocked by us.
  getParentChain().exitContext(ctx_idx);
}

void c_ExternalThreadEventWaitHandle::registerToContext() {
  AsioContext *ctx = getContext();
  m_ctxVecIndex = ctx->registerTo(ctx->getExternalThreadEvents(), this);
}

void c_ExternalThreadEventWaitHandle::unregisterFromContext() {
  AsioContext *ctx = getContext();
  ctx->unregisterFrom(ctx->getExternalThreadEvents(), m_ctxVecIndex);
}

///////////////////////////////////////////////////////////////////////////////
}
