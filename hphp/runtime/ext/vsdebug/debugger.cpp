/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

void Debugger::setTransport(DebugTransport* transport) {
  assert(m_transport == nullptr);
  m_transport = transport;
  setClientConnected(m_transport->clientConnected());
}

void Debugger::setClientConnected(bool connected) {
  Lock lock(m_lock);

  if (connected == m_clientConnected.load()) {
    // If the connected state didn't change, just return.
    return;
  }

  // Store connected first. New request threads will first check this value
  // to quickly determine if a debugger client is connected to avoid having
  // to grab a lock on the request init path in the case where this extension
  // is enabled, but not in use by any client.
  m_clientConnected.store(connected, std::memory_order_release);

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Debugger client connected: %s",
    connected ? "YES" : "NO"
  );

  // Clean up and free the previous session, if any.
  if (m_session != nullptr) {
    delete m_session;
    m_session = nullptr;
  }

  if (connected) {
    // Create a new debugger session.
    assert(m_session == nullptr);
    m_session = new DebuggerSession(this);
    if (m_session == nullptr) {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelError,
        "Failed to allocate debugger session!"
      );
    }
  }
}

void Debugger::shutdown() {
  if (m_transport == nullptr) {
    return;
  }

  setClientConnected(false);

  // m_session is deleted and set to nullptr by setClientConnected(false).
  assert(m_session == nullptr);

  delete m_transport;
  m_transport = nullptr;
}

bool Debugger::sendUserMessage(const char* message, const char* level) {
  Lock lock(m_lock);
  if (m_transport == nullptr) {
    return false;
  }

  return m_transport->sendUserMessage(message, level);
}

}
}
