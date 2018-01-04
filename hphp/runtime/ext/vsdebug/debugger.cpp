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
  setClientConnected(m_transport->connectedClientCount() > 0);
}

void Debugger::setClientConnected(bool connected) {
  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Debugger client connected: %s",
    connected ? "YES" : "NO"
  );
  m_clientConnected.store(connected, std::memory_order_release);
}

void Debugger::shutdown() {
  if (m_transport == nullptr) {
    return;
  }

  setClientConnected(false);
  delete m_transport;
  m_transport = nullptr;
}

}
}
