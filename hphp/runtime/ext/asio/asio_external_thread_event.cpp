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

#include "hphp/runtime/ext/asio/asio_external_thread_event.h"
#include <thread>
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/asio/external_thread_event_wait_handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

AsioExternalThreadEvent::AsioExternalThreadEvent(ObjectData* priv_data)
    : m_queue(AsioSession::Get()->getExternalThreadEventQueue()),
      m_state(Waiting) {
  m_waitHandle = c_ExternalThreadEventWaitHandle::Create(this, priv_data);
}

void AsioExternalThreadEvent::abandon() {
  assert(m_state.load() == Waiting);
  assert(m_waitHandle->getCount() == 1);
  m_state.store(Abandoned);
  m_waitHandle->abandon(false);
}

bool AsioExternalThreadEvent::cancel() {
  uint32_t/*state_t*/ expected(Waiting);
  if (m_state.compare_exchange_strong(expected, Canceled)) {
    return true;
  }

  assert(expected == Finished);
  return false;
}

void AsioExternalThreadEvent::markAsFinished() {
  uint32_t/*state_t*/ expected(Waiting);
  if (m_state.compare_exchange_strong(expected, Finished)) {
    // transfer ownership
    m_queue->send(m_waitHandle);
  } else {
    // web request died, destroy object
    assert(expected == Canceled);
    release();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
