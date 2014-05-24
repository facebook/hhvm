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

#include "hphp/runtime/ext/asio/external_thread_event_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_external_thread_event.h"
#include "hphp/runtime/ext/asio/asio_external_thread_event_queue.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_externalThreadEvent("<external-thread-event>");
}

void c_ExternalThreadEventWaitHandle::t___construct() {
  // gen-ext-hhvm requires at least one declared method in the class to work
  not_reached();
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

c_ExternalThreadEventWaitHandle* c_ExternalThreadEventWaitHandle::Create(AsioExternalThreadEvent* event, ObjectData* priv_data) {
  c_ExternalThreadEventWaitHandle* wh = NEWOBJ(c_ExternalThreadEventWaitHandle);
  wh->initialize(event, priv_data);
  return wh;
}

void c_ExternalThreadEventWaitHandle::initialize(AsioExternalThreadEvent* event, ObjectData* priv_data) {
  setState(STATE_WAITING);
  m_event = event;
  m_privData = priv_data;

  // this wait handle is owned by existence of unprocessed event
  incRefCount();

  if (isInContext()) {
    registerToContext();
  }
}

void c_ExternalThreadEventWaitHandle::destroyEvent(bool sweeping /*= false */) {
  // destroy event and its private data
  m_event->release();
  m_event = nullptr;

  // unregister from sweep()
  unregister();

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
    setState(STATE_FAILED);
    tvWriteObject(exception.get(), &m_resultOrException);
    done();
    return;
  } catch (...) {
    setState(STATE_FAILED);
    tvWriteObject(AsioSession::Get()->getAbruptInterruptException().get(),
                  &m_resultOrException);
    done();
    throw;
  }

  assert(cellIsPlausible(result));
  setState(STATE_SUCCEEDED);
  cellCopy(result, m_resultOrException);
  done();
}

String c_ExternalThreadEventWaitHandle::getName() {
  return s_externalThreadEvent;
}

void c_ExternalThreadEventWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    unregisterFromContext();
  }

  setContextIdx(ctx_idx);
  registerToContext();
}

void c_ExternalThreadEventWaitHandle::exitContext(context_idx_t ctx_idx) {
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
  for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
    pwh->exitContextBlocked(ctx_idx);
  }
}

void c_ExternalThreadEventWaitHandle::registerToContext() {
  AsioContext *ctx = getContext();
  m_index = ctx->registerTo(ctx->getExternalThreadEvents(), this);
}

void c_ExternalThreadEventWaitHandle::unregisterFromContext() {
  AsioContext *ctx = getContext();
  ctx->unregisterFrom(ctx->getExternalThreadEvents(), m_index);
}

///////////////////////////////////////////////////////////////////////////////
}
