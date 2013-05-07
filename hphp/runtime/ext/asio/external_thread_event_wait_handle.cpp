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
#include <runtime/ext/asio/asio_external_thread_event.h>
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_externalThreadEvent("<external-thread-event>");
}

c_ExternalThreadEventWaitHandle::c_ExternalThreadEventWaitHandle(VM::Class *cb)
    : c_WaitableWaitHandle(cb) {
}

c_ExternalThreadEventWaitHandle::~c_ExternalThreadEventWaitHandle() {
}

void c_ExternalThreadEventWaitHandle::sweep() {
  assert(getState() == STATE_WAITING);

  if (m_event->cancel()) {
    // canceled; the processing thread will take care of cleanup
    return;
  }

  // event has finished, but process() was not called yet
  auto session = AsioSession::Get();
  bool done = false;
  do {
    auto ete_wh = session->waitForExternalThreadEvents();
    while (ete_wh) {
      done |= ete_wh == this;
      auto next_wh = ete_wh->getNextToProcess();
      ete_wh->abandon(true);
      ete_wh = next_wh;
    }
  } while (!done);
}

void c_ExternalThreadEventWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "ExternalThreadEventWaitHandle can be constructed only from extension"));
  throw e;
}

c_ExternalThreadEventWaitHandle* c_ExternalThreadEventWaitHandle::Create(AsioExternalThreadEvent* event, ObjectData* priv_data) {
  c_ExternalThreadEventWaitHandle* wh = NEWOBJ(c_ExternalThreadEventWaitHandle);
  wh->initialize(event, priv_data);
  return wh;
}

void c_ExternalThreadEventWaitHandle::initialize(AsioExternalThreadEvent* event, ObjectData* priv_data) {
  // this wait handle is owned by existence of unprocessed event
  incRefCount();
  m_event = event;
  m_privData = priv_data;

  setState(STATE_WAITING);
  if (isInContext()) {
    m_index = getContext()->registerExternalThreadEvent(this);
  }
}

void c_ExternalThreadEventWaitHandle::abandon(bool sweeping) {
  assert(getState() == STATE_WAITING);
  assert(getCount() == 1 || sweeping);

  if (isInContext()) {
    getContext()->unregisterExternalThreadEvent(m_index);
  }

  // event is abandoned, destroy it, unregister sweepable and decref ownership
  m_event->release();
  m_event = nullptr;
  unregister();
  decRefObj(this);
}

void c_ExternalThreadEventWaitHandle::process() {
  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    getContext()->unregisterExternalThreadEvent(m_index);
  }

  try {
    TypedValue result;
    m_event->unserialize(&result);
    assert(tvIsPlausible(&result));
    setResult(&result);
    tvRefcountedDecRefCell(&result);
  } catch (const Object& exception) {
    setException(exception.get());
  }

  // event is processed, destroy it, unregister sweepable and decref ownership
  m_event->release();
  m_event = nullptr;
  m_privData = nullptr;
  unregister();
  decRefObj(this);
}

String c_ExternalThreadEventWaitHandle::getName() {
  return s_externalThreadEvent;
}

void c_ExternalThreadEventWaitHandle::enterContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // already in the more specific context?
  if (LIKELY(getContextIdx() >= ctx_idx)) {
    return;
  }

  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    getContext()->unregisterExternalThreadEvent(m_index);
  }

  setContextIdx(ctx_idx);
  m_index = getContext()->registerExternalThreadEvent(this);
}

void c_ExternalThreadEventWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));
  assert(getContextIdx() == ctx_idx);
  assert(getState() == STATE_WAITING);

  // move us to the parent context
  setContextIdx(getContextIdx() - 1);

  // re-register if still in a context
  if (isInContext()) {
    getContext()->registerExternalThreadEvent(this);
  }

  // recursively move all wait handles blocked by us
  for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
    pwh->exitContextBlocked(ctx_idx);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
