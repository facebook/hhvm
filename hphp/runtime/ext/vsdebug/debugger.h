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

#ifndef incl_HPHP_VSDEBUG_DEBUGGER_H_
#define incl_HPHP_VSDEBUG_DEBUGGER_H_

#include <atomic>
#include <unordered_map>

#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/runtime/ext/vsdebug/transport.h"
#include "hphp/runtime/ext/vsdebug/session.h"
#include "hphp/runtime/ext/vsdebug/command_queue.h"

namespace HPHP {
namespace VSDEBUG {

// Forward declaration for transport.
struct DebugTransport;

// Forward declaration of debugger session.
struct DebuggerSession;

struct Debugger final {
  Debugger() {}
  virtual ~Debugger() {
    shutdown();
  }

  // Sets the transport mechanism to be used to communicate with a debug client.
  // This call transfers ownership of transport from the caller to this object,
  // which will be responsbile for cleaning it up and deleting it before the
  // debugger is destroyed.
  void setTransport(DebugTransport* transport);

  // Invoked by the transport to indicate if a debugger client is currently
  // connected or not. Many debugger events will be skipped if no client is
  // connected to avoid impacting perf when there is no debugger client
  // attached.
  void setClientConnected(bool connected);

  // Shuts down the debugger session and cleans up any resources. This will also
  // unblock any requests that are broken in to the debugger.
  void shutdown();

  // Returns true if there is a client connected.
  inline bool clientConnected() const {
    return m_clientConnected.load(std::memory_order_acquire);
  }

  bool sendUserMessage(
    const char* message,
    const char* level = DebugTransport::OutputLevelLog
  );

private:

  Mutex m_lock;
  DebugTransport* m_transport {nullptr};

  // The DebuggerSession represents the connected debugger client. This object
  // is nullptr if no client is connected, and never nullptr if a client is
  // connected.
  DebuggerSession* m_session {nullptr};

  // The following flag will indicate if there is any debugger client
  // connected to the extension. This allows us to return quickly from
  // debugger hook operations when the debugger is "enabled" but not
  // actually in used due to no connected debugger clients.
  std::atomic<bool> m_clientConnected {false};
};

}
}

#endif // incl_HPHP_VSDEBUG_DEBUGGER_H_
