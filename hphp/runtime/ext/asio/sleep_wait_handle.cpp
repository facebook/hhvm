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

#include "hphp/runtime/ext/asio/sleep_wait_handle.h"

#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  StaticString s_sleep("<sleep>");
}

void c_SleepWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Use SleepWaitHandle::create() instead of constructor"));
  throw e;
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
  m_waketime =
    AsioSession::TimePoint::clock::now() +
    std::chrono::microseconds(usecs);

  incRefCount();
  AsioSession::Get()->getSleepEventQueue().push(this);

  setState(STATE_WAITING);
  if (isInContext()) {
    registerToContext();
  }
}

void c_SleepWaitHandle::process() {
  assert(getState() == STATE_WAITING);

  if (isInContext()) {
    unregisterFromContext();
  }

  setResult(make_tv<KindOfNull>());
}

String c_SleepWaitHandle::getName() {
  return s_sleep;
}

void c_SleepWaitHandle::registerToContext() {
  AsioContext *ctx = getContext();
  m_index = ctx->registerTo<c_SleepWaitHandle>(ctx->getSleepEvents(), this);
}

void c_SleepWaitHandle::unregisterFromContext() {
  AsioContext *ctx = getContext();
  ctx->unregisterFrom<c_SleepWaitHandle>(ctx->getSleepEvents(), m_index);
}

///////////////////////////////////////////////////////////////////////////////
}
