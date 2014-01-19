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

#include "hphp/runtime/ext/asio/reschedule_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/blockable_wait_handle.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_reschedule("<reschedule>");
}

const int64_t q_RescheduleWaitHandle$$QUEUE_DEFAULT =
  AsioContext::QUEUE_DEFAULT;
const int64_t q_RescheduleWaitHandle$$QUEUE_NO_PENDING_IO =
  AsioContext::QUEUE_NO_PENDING_IO;

void c_RescheduleWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use RescheduleWaitHandle::create() instead of constructor"));
  throw e;
}

Object c_RescheduleWaitHandle::ti_create(int64_t queue, int priority) {
  if (UNLIKELY(
      queue != q_RescheduleWaitHandle$$QUEUE_DEFAULT &&
      queue != q_RescheduleWaitHandle$$QUEUE_NO_PENDING_IO)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected queue to be a value defined by one of the QUEUE_ constants"));
    throw e;
  }

  if (UNLIKELY(priority < 0)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected priority to be a non-negative integer"));
    throw e;
  }

  c_RescheduleWaitHandle* wh = NEWOBJ(c_RescheduleWaitHandle);
  wh->initialize(static_cast<uint32_t>(queue), static_cast<uint32_t>(priority));
  return wh;
}

void c_RescheduleWaitHandle::initialize(uint32_t queue, uint32_t priority) {
  m_queue = queue;
  m_priority = priority;

  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this, m_queue, m_priority);
  }
}

void c_RescheduleWaitHandle::run() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    return;
  }

  setResult(make_tv<KindOfNull>());
}

String c_RescheduleWaitHandle::getName() {
  return s_reschedule;
}

void c_RescheduleWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  assert(getState() == STATE_SCHEDULED);

  setContextIdx(ctx_idx);
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
    getContext()->schedule(this, m_queue, m_priority);
  }

  // recursively move all wait handles blocked by us
  for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
    pwh->exitContextBlocked(ctx_idx);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
