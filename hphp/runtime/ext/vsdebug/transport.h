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

#ifndef incl_HPHP_VSDEBUG_TRANSPORT_H_
#define incl_HPHP_VSDEBUG_TRANSPORT_H_

#include <folly/dynamic.h>
#include <folly/json.h>

#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/util/lock.h"

namespace HPHP {
namespace VSDEBUG {

// Forward declaration for Debugger
struct Debugger;

// Abstract base class for debug transports, which are responsible
// for communication with the debugger client.
struct DebugTransport {
  DebugTransport(Debugger* debugger);
  virtual ~DebugTransport() {
    if (m_buffer != nullptr) {
      free(m_buffer);
    }
  }

  virtual int sendToClient(
    const folly::dynamic& message,
    const char* messageType
  );

  virtual int readMessage(folly::dynamic& message);
  virtual int connectedClientCount() const = 0;

  static constexpr char* MessageTypeRequest = "request";
  static constexpr char* MessageTypeResponse = "response";
  static constexpr char* MessageTypeEvent = "event";

protected:

  const std::string wrapOutgoingMessage(
    const folly::dynamic& message,
    const char* messageType
  ) const;

  FILE* m_transportFd {nullptr};

  // Pointer to the debugger object that owns this transport. The debugger
  // owns the lifetime of this object.
  Debugger* const m_debugger;

private:

  static constexpr int ReadBufferDefaultSize = 1024;

  // Input buffer for reading messages from the debugger client.
  int resizeBuffer();
  bool processMessage(folly::dynamic& message);

  Mutex m_mutex;
  char* m_buffer {nullptr};
  size_t m_bufferSize {0};
  size_t m_bufferPosition {0};
};

}
}

#endif // incl_HPHP_VSDEBUG_TRANSPORT_H_
