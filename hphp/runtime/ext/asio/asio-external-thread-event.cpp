/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_external-thread-event-wait-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

AsioExternalThreadEvent::AsioExternalThreadEvent(ObjectData* priv_data)
    : m_queue(AsioSession::Get()->getExternalThreadEventQueue()),
      m_state(Waiting) {
  m_waitHandle = req::make<c_ExternalThreadEventWaitHandle>().detach();
  m_waitHandle->initialize(this, priv_data);
}

void AsioExternalThreadEvent::abandon() {
  assertx(m_state.load() == Waiting);
  assertx(m_waitHandle->hasExactlyOneRef());
  m_state.store(Abandoned);
  m_waitHandle->abandon(false);
}

bool AsioExternalThreadEvent::cancel() {
  uint32_t/*state_t*/ expected(Waiting);
  if (m_state.compare_exchange_strong(expected, Canceled)) {
    return true;
  }

  assertx(expected == Finished);
  return false;
}

void AsioExternalThreadEvent::markAsFinished() {
  uint32_t/*state_t*/ expected(Waiting);
  if (m_state.compare_exchange_strong(expected, Finished)) {
    m_finishTime = AsioSession::TimePoint::clock::now();
    // transfer ownership
    m_queue->send(m_waitHandle);
  } else {
    // web request died, destroy object
    assertx(expected == Canceled);
    release();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
