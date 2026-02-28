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

#pragma once

#include <folly/json/dynamic.h>
#include <folly/json/json.h>
#include <list>
#include <condition_variable>
#include <mutex>

#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/util/async-func.h"
#include "hphp/util/lock.h"

namespace HPHP {
namespace VSDEBUG {

struct Debugger;

// Abstract base class for debug transports, which are responsible
// for communication with the debugger client.
struct DebugTransport {
  DebugTransport(Debugger* debugger);
  virtual ~DebugTransport() {
    shutdown();
  }

  virtual void shutdown();
  virtual void cleanupFd(int fd);

  // Enqueues an outgoing message to be sent to the client. This routine will
  // put the message into the outgoing message queue and then return. A worker
  // thread will process outgoing messages in order and send them to the client.
  virtual void enqueueOutgoingMessageForClient(
    folly::dynamic& message,
    const char* messageType
  );

  // Enqueues an outgoing user-facing message to be sent to the client.
  // This routine will add the necessary protocol attributes for a user message,
  // put the message into the outgoing message queue and then return. A worker
  // thread will process outgoing messages in order and send them to the client.
  virtual void enqueueOutgoingUserMessage(
    request_id_t threadId,
    const char* message,
    const char* level = OutputLevelLog
  );

  virtual bool clientConnected() const = 0;

  // Enqueues an outgoing event message to send to the debugger client.
  void enqueueOutgoingEventMessage(
    folly::dynamic& message,
    const char* eventType
  );

  // VS Code protocol message types.
  static constexpr char* MessageTypeRequest = "request";
  static constexpr char* MessageTypeResponse = "response";
  static constexpr char* MessageTypeEvent = "event";

  // VS Code protocol event types
  static constexpr char* EventTypeOutput = "output";

  // Custom event types.
  static constexpr char* EventTypeConnectionRefused = "hhvmConnectionRefused";

  // Message output levels to be displayed in the debugger console.
  // NOTE: the protocol explicitly defines:
  //    "console", "stdout", "stderr", "telemetry", with "console" being the
  //    default.
  //
  //    Nuclide supports additional log levels which can be specified here.
  //    Other VS Code Debug Protocol consumers should default to "console" if
  //    they do not understand.
  static constexpr char* OutputLevelSuccess = "success";
  static constexpr char* OutputLevelInfo = "info";
  static constexpr char* OutputLevelWarning = "console";
  static constexpr char* OutputLevelError = "error";
  static constexpr char* OutputLevelLog = "console";
  static constexpr char* OutputLevelStdout = "stdout";
  static constexpr char* OutputLevelStderr = "stderr";

  #ifdef POLLRDHUP // Linux-only
    static constexpr int g_platformPollFlags = POLLRDHUP;
  #else
    static constexpr int g_platformPollFlags = 0;
  #endif

protected:

  const std::string wrapOutgoingMessage(
    folly::dynamic& message,
    const char* messageType
  ) const;

  // Pointer to the debugger object that owns this transport. The debugger
  // owns the lifetime of this object.
  Debugger* const m_debugger;

  void setTransportFd(int fd);

  inline int getTransportFd() const {
    Lock lock(m_mutex);
    return m_transportFd;
  }

  virtual void onClientDisconnected();

private:

  // Internal counter of the message seq for responses and events
  std::atomic<unsigned long> m_responseSeqId {1};

  static constexpr int ReadBufferDefaultSize = 1024;

  static bool tryProcessMessage(
    char* buffer,
    size_t bufferSize,
    size_t* bufferPosition,
    folly::dynamic* message
  );

  void processOutgoingMessages();
  void processIncomingMessages();

  void shutdownInputThread();
  void shutdownOutputThread();

  // File descriptor to use for communication with the debugger client.
  mutable Mutex m_mutex;
  int m_transportFd {-1};

  // The abort pipe pair of fds is a pipe that allows the main thread to
  // signal the polling thread that its time to exit. We can use this to
  // break it out of a blocking call to recv.
  int m_abortPipeFd[2] {-1, -1};

  // Queue of messages waiting to be sent to the debugger client.
  std::mutex m_outgoingMsgLock;
  std::list<std::string> m_outgoingMessages;
  std::condition_variable m_outgoingMsgCondition;
  std::atomic<bool> m_terminating {false};

  // A worker thread to process outgoing messages and send them to the client.
  AsyncFunc<DebugTransport> m_outputThread;

  // A worker thread to process incoming messages from the client and sends them
  // to the debugger engine.
  AsyncFunc<DebugTransport> m_inputThread;
};

}
}

