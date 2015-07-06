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

#include "hphp/runtime/ext/asio/ext_reschedule-wait-handle.h"

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_reschedule("<reschedule>");
}

Object c_RescheduleWaitHandle::ti_create(int64_t queue, int64_t priority) {
  if (UNLIKELY(
      queue != q_RescheduleWaitHandle$$QUEUE_DEFAULT &&
      queue != q_RescheduleWaitHandle$$QUEUE_NO_PENDING_IO)) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected queue to be a value defined by one of the QUEUE_ constants");
  }

  if (UNLIKELY(priority < 0)) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Expected priority to be a non-negative integer");
  }

  auto wh = req::make<c_RescheduleWaitHandle>();
  wh->initialize(static_cast<uint32_t>(queue), priority);
  return Object(std::move(wh));
}

void c_RescheduleWaitHandle::initialize(uint32_t queue, int64_t priority) {
  setState(STATE_SCHEDULED);
  setContextIdx(AsioSession::Get()->getCurrentContextIdx());
  m_queue = queue;
  m_priority = priority;

  if (isInContext()) {
    scheduleInContext();
  }
}

void c_RescheduleWaitHandle::run() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    return;
  }

  auto parentChain = getParentChain();
  setState(STATE_SUCCEEDED);
  tvWriteNull(&m_resultOrException);
  parentChain.unblock();
}

String c_RescheduleWaitHandle::getName() {
  return s_reschedule;
}

void c_RescheduleWaitHandle::scheduleInContext() {
  getContext()->schedule(this, m_queue, m_priority);
}

void c_RescheduleWaitHandle::exitContext(context_idx_t ctx_idx) {
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

  if (UNLIKELY(getState() != STATE_SCHEDULED)) {
    throw FatalErrorException(
      "Invariant violation: encountered unexpected state");
  }

  // move us to the parent context
  setContextIdx(getContextIdx() - 1);

  // reschedule if still in a context
  if (isInContext()) {
    scheduleInContext();
  }

  // recursively move all wait handles blocked by us
  getParentChain().exitContext(ctx_idx);
}

///////////////////////////////////////////////////////////////////////////////
}
