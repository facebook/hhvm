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

#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/util/process.h"

namespace HPHP {
namespace VSDEBUG {

Debugger::Debugger() {
}

void Debugger::setTransport(DebugTransport* transport) {
  assert(m_transport == nullptr);
  m_transport = transport;
  setClientConnected(m_transport->clientConnected());
}

void Debugger::setClientConnected(bool connected) {
  DebuggerSession* sessionToDelete = nullptr;
  SCOPE_EXIT {
    if (sessionToDelete != nullptr) {
      delete sessionToDelete;
    }
  };

  {
    Lock lock(m_lock);

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

    // Defer cleaning up the session until after we've dropped the lock.
    // Shutting down the dummy request thread will cause the worker to call
    // back into this extension since the HHVM context will call requestShutdown
    // on the dummy request.
    if (m_session != nullptr) {
      sessionToDelete = m_session;
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
        m_clientConnected.store(false, std::memory_order_release);
      }

      // When the client connects, break the entire program to get it into a
      // known state that matches the thread list being presented in the
      // debugger. Once all threads are wrangled and the front-end is updated,
      // the program can resume execution.
      m_state = ProgramState::LoaderBreakpoint;

      // Attach the debugger to any request threads that were already
      // running before the client connected. We did not attach to them if they
      // were initialized when the client was disconnected to avoid taking a
      // perf hit for a debugger hook that wasn't going to be used.
      //
      // Only do this after seeing at least 1 request via the requestInit path:
      // on initial startup in script mode, the script request thread will have
      // been created already in HHVM main, but is not ready for us to attach
      // yet because extensions are still being initialized.
      if (m_totalRequestCount.load() > 0) {
        ThreadInfo::ExecutePerThread([this] (ThreadInfo* ti) {
          this->attachToRequest(ti);
        });
      }

      // If the script startup thread is waiting for a client connection, wake
      // it up now.
      {
        std::unique_lock<std::mutex> lock(m_connectionNotifyLock);
        m_connectionNotifyCondition.notify_all();
      }
    } else {

      // The client has detached. Walk through any requests we are currently
      // attached to and release them if they are blocked in the debugger.
      for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
        // Clear any undelivered client messages in the request's command queue,
        // since they apply to the debugger session that just ended.
        // NOTE: The request's RequestInfo will be cleaned up and freed when the
        // request completes in requestShutdown. It is not safe to do that from
        // this thread.
        it->second->m_commandQueue.clearPendingMessages();
      }

      m_clientInitialized = false;
      resumeTarget();
    }
  }
}

void Debugger::setClientInitialized() {
  Lock lock(m_lock);
  if (m_clientInitialized) {
    return;
  }

  m_clientInitialized = true;

  // Send a thread start event for any thread that exists already at the point
  // the debugger client initializes communication so that they appear in the
  // client side thread list.
  for (auto it = m_requestIdMap.begin(); it != m_requestIdMap.end(); it++) {
    if (it->second->m_executing == ThreadInfo::Executing::UserFunctions) {
      sendThreadEventMessage(it->first, ThreadEventType::ThreadStarted);
    }

    auto requestIt = m_requests.find(it->second);
    assert(requestIt != m_requests.end());
    RequestInfo* ri = requestIt->second;
    if (ri->m_flags.requestPaused) {
      sendStoppedEvent("pause", "Initial Break", it->first);
    }
  }

  sendStoppedEvent("pause", "Initial Break", -1);
}

int Debugger::getCurrentThreadId() {
  Lock lock(m_lock);

  auto const threadInfo = &TI();
  const auto it = m_requestInfoMap.find(threadInfo);
  assert(it != m_requestInfoMap.end());

  return it->second;
}

void Debugger::cleanupRequestInfo(ThreadInfo* ti, RequestInfo* ri) {
  if (ri->m_flags.hookAttached) {
    DebuggerHook::detach(ti);
  }

  // Shut down the request's command queue. This releases the thread if
  // it is waiting inside the queue.
  ri->m_commandQueue.shutdown();
  delete ri;
}

void Debugger::shutdown() {
  if (m_transport == nullptr) {
    return;
  }

  trySendTerminatedEvent();

  m_transport->shutdown();
  setClientConnected(false);

  // m_session is deleted and set to nullptr by setClientConnected(false).
  assert(m_session == nullptr);

  delete m_transport;
  m_transport = nullptr;
}

void Debugger::trySendTerminatedEvent() {
  Lock lock(m_lock);

  folly::dynamic event = folly::dynamic::object;
  sendEventMessage(event, "terminated");
}

void Debugger::sendStoppedEvent(
  const char* reason,
  const char* displayDetails,
  int64_t threadId
) {
  Lock lock(m_lock);

  folly::dynamic event = folly::dynamic::object;
  event["allThreadsStopped"] = m_pausedRequestCount == m_requests.size();

  if (reason != nullptr) {
    event["reason"] = reason;
  }

  if (displayDetails != nullptr) {
    event["description"] = displayDetails;
  }

  if (threadId > 0) {
    event["threadId"] = threadId;
  }

  sendEventMessage(event, "stopped");
}

void Debugger::sendContinuedEvent(int64_t threadId) {
  Lock lock(m_lock);
  folly::dynamic event = folly::dynamic::object;
  event["allThreadsContinued"] = m_pausedRequestCount == 0;
  if (threadId >= 0) {
    event["threadId"] = threadId;
  }

  sendEventMessage(event, "continued");
}

void Debugger::sendUserMessage(const char* message, const char* level) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  if (m_transport != nullptr) {
    m_transport->enqueueOutgoingUserMessage(message, level);
  }
}

void Debugger::sendEventMessage(folly::dynamic& event, const char* eventType) {
  Lock lock(m_lock);
  if (m_transport != nullptr) {
    m_transport->enqueueOutgoingEventMessage(event, eventType);
  }
}

void Debugger::sendThreadEventMessage(
  int64_t threadId,
  ThreadEventType eventType
) {
  Lock lock(m_lock);

  if (!m_clientInitialized) {
    // Don't start sending the client "thread started" and "thread exited"
    // events before its finished its initialization flow.
    // In this case, we'll send a thread started event for any threads that are
    // currently running when initialization completes (and any threads that
    // exit before that point, the debugger never needs to know about).
    return;
  }

  folly::dynamic event = folly::dynamic::object;

  event["reason"] = eventType == ThreadEventType::ThreadStarted
    ? "started"
    : "exited";

  // TODO: Thread IDs can be 64 bit here, but the VS protocol is going to
  // interpret this as a JavaScript number which has a smaller max value...
  event["threadId"] = threadId;

  sendEventMessage(event, "thread");
}

void Debugger::requestInit() {
  m_totalRequestCount++;

  if (!clientConnected()) {
    // Don't pay for attaching to the thread if no debugger client is connected.
    return;
  }

  auto const threadInfo = &TI();
  bool pauseRequest;
  RequestInfo* requestInfo;

  {
    Lock lock(m_lock);

    // Don't attach to the dummy request thread. DebuggerSession manages the
    // dummy requests debugger hook state.
    if ((int64_t)Process::GetThreadId() != m_dummyThreadId) {
      requestInfo = attachToRequest(threadInfo);
      pauseRequest = m_clientInitialized && m_state != ProgramState::Running;
    } else {
      pauseRequest = false;
    }
  }

  // If the debugger was already paused when this request started, drop the lock
  // and block the request in its command queue until the debugger resumes.
  // Note: if the debugger client issues a resume between the time the lock
  // is dropped above and entering the command queue, there will be a pending
  // Resume command in this queue, which will cause this thread to unblock.
  if (pauseRequest && requestInfo != nullptr) {
    processCommandQueue(getCurrentThreadId(), requestInfo);
  }
}

void Debugger::processCommandQueue(int threadId, RequestInfo* requestInfo) {
  {
    Lock lock(m_lock);
    m_pausedRequestCount++;
    requestInfo->m_flags.requestPaused = true;

    // TODO: Put proper stop reason here - if this is the thread that hit a
    // bp/step/exn, say that. Otherwise indicate its paused due to stop one
    // stop all semantics + an event on another thread.
    sendStoppedEvent("pause", "pause", threadId);
  }

  requestInfo->m_commandQueue.processCommands();

  {
    Lock lock(m_lock);
    requestInfo->m_flags.requestPaused = false;
    m_pausedRequestCount--;
    sendContinuedEvent(threadId);
  }
}

RequestInfo* Debugger::attachToRequest(ThreadInfo* ti) {
  // Note: the caller of this routine must hold m_lock.
  RequestInfo* requestInfo = nullptr;

  int threadId;
  auto it = m_requests.find(ti);
  if (it == m_requests.end()) {
    // New request. Insert a request info object into our map.
    threadId = ++m_nextThreadId;
    requestInfo = new RequestInfo();
    if (requestInfo == nullptr) {
      // Failed to allocate request info.
      return nullptr;
    }

    m_requests.emplace(std::make_pair(ti, requestInfo));
    m_requestIdMap.emplace(std::make_pair(threadId, ti));
    m_requestInfoMap.emplace(std::make_pair(ti, threadId));
  } else {
    requestInfo = it->second;
    auto idIt = m_requestInfoMap.find(ti);
    assert(idIt != m_requestInfoMap.end());
    threadId = idIt->second;
  }

  assert(requestInfo != nullptr);

  if (ti->m_executing == ThreadInfo::Executing::UserFunctions) {
    sendThreadEventMessage(threadId, ThreadEventType::ThreadStarted);
  }


  // Try to attach our debugger hook to the request.
  if (!requestInfo->m_flags.hookAttached) {
    if (DebuggerHook::attach<VSDebugHook>(ti)) {
      requestInfo->m_flags.hookAttached = true;
    } else {
      m_transport->enqueueOutgoingUserMessage(
        "Failed to attach to new HHVM request: another debugger is already "
          "attached.",
        DebugTransport::OutputLevelError
      );
    }
  }

  return requestInfo;
}

void Debugger::requestShutdown() {
  auto const threadInfo = &TI();
  RequestInfo* requestInfo = nullptr;
  int threadId = -1;

  SCOPE_EXIT {
    if (clientConnected() && threadId >= 0) {
      sendThreadEventMessage(threadId, ThreadEventType::ThreadExited);
    }

    if (requestInfo != nullptr) {
      cleanupRequestInfo(threadInfo, requestInfo);
    }
  };

  {
    Lock lock(m_lock);
    auto it = m_requests.find(threadInfo);
    if (it == m_requests.end()) {
      return;
    }

    requestInfo = it->second;
    m_requests.erase(it);

    auto infoItr = m_requestInfoMap.find(threadInfo);
    assert(infoItr != m_requestInfoMap.end());

    threadId = infoItr->second;
    auto idItr = m_requestIdMap.find(threadId);
    assert(idItr != m_requestIdMap.end());

    m_requestIdMap.erase(idItr);
    m_requestInfoMap.erase(infoItr);
  }
}

RequestInfo* Debugger::getRequestInfo() {
  Lock lock(m_lock);
  auto it = m_requests.find(&TI());
  if (it != m_requests.end()) {
    return it->second;
  }

  return nullptr;
}

bool Debugger::executeClientCommand(
  VSCommand* command,
  std::function<bool(folly::dynamic& responseMsg)> callback
) {
  Lock lock(m_lock);

  // If there is no debugger client connected anymore, the client command
  // should not be processed, and the target request thread should resume.
  if (!clientConnected()) {
    return true;
  }

  try {
    enforceRequiresBreak(command);

    // Invoke the command execute callback. It will return true if this thread
    // should be resumed, or false if it should continue to block in its
    // command queue.
    folly::dynamic responseMsg = folly::dynamic::object;
    bool resumeThread = callback(responseMsg);

    if (command->commandTarget() != CommandTarget::WorkItem) {
      sendCommandResponse(command, responseMsg);
    }

    return resumeThread;
  } catch (DebuggerCommandException e) {
    reportClientMessageError(command->getMessage(), e.what());
  } catch (...) {
    reportClientMessageError(command->getMessage(), InternalErrorMsg);
  }

  // On error, do not resume the request thread.
  return false;
}

void Debugger::reportClientMessageError(
  folly::dynamic& clientMsg,
  const char* errorMessage
) {
  try {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to process client message (%s): %s",
      folly::toJson(clientMsg).c_str(),
      errorMessage
    );

    folly::dynamic responseMsg = folly::dynamic::object;
    responseMsg["success"] = false;
    responseMsg["request_seq"] = clientMsg["seq"];
    responseMsg["command"] = clientMsg["command"];
    responseMsg["message"] = errorMessage;

    m_transport->enqueueOutgoingMessageForClient(
      responseMsg,
      DebugTransport::MessageTypeResponse
    );

    // Print an error to the debugger console to inform the user as well.
    sendUserMessage(errorMessage, DebugTransport::OutputLevelError);
  } catch (...) {
    // We tried.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Unexpected failure while trying to send response to client."
    );
  }
}

void Debugger::sendCommandResponse(
  VSCommand* command,
  folly::dynamic& responseMsg
) {
  folly::dynamic& clientMsg = command->getMessage();
  responseMsg["success"] = true;
  responseMsg["request_seq"] = clientMsg["seq"];
  responseMsg["command"] = clientMsg["command"];

  m_transport->enqueueOutgoingMessageForClient(
    responseMsg,
    DebugTransport::MessageTypeResponse
  );
}

void Debugger::resumeTarget() {
  // Note: caller must hold m_lock here.

  m_state = ProgramState::Running;

  // Resume every paused request. Each request will send a thread continued
  // event when it exits its command loop.
  for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
    RequestInfo* ri = it->second;

    if (ri->m_flags.requestPaused) {
      VSCommand* resumeCommand = ContinueCommand::createInstance(this);
      ri->m_commandQueue.dispatchCommand(resumeCommand);
    }
  }

  sendContinuedEvent(-1);
}

void Debugger::onClientMessage(folly::dynamic& message) {
  Lock lock(m_lock);

  // It's possible the client disconnected between the time the message was
  // received and when the lock was acquired in this routine. If the client
  // has gone, do not process the message.
  if (!clientConnected()) {
    return;
  }

  VSCommand* command = nullptr;
  SCOPE_EXIT {
    if (command != nullptr) {
      delete command;
    }
  };

  try {

    // All valid client messages should have a sequence number and type.
    try {
      const auto& seq = message["seq"];
      if (!seq.isInt()) {
        throw DebuggerCommandException("Invalid message sequence number.");
      }

      const auto& type = message["type"];
      if (!type.isString() || type.getString().empty()) {
        throw DebuggerCommandException("Invalid command type.");
      }
    } catch (std::out_of_range e) {
      throw DebuggerCommandException(
        "Message is missing a required attribute."
      );
    }

    if (!VSCommand::parseCommand(this, message, &command)) {
      assert(command == nullptr);
      throw DebuggerCommandException(
        "The command was invalid or is not implemented in the debugger."
      );
    }

    assert(command != nullptr);
    enforceRequiresBreak(command);

    // Otherwise this is a normal command. Dispatch it to its target.
    switch(command->commandTarget()) {
      case CommandTarget::None:
      case CommandTarget::WorkItem:
        if (command->execute()) {
          // The command requested that the target be resumed. A command with
          // CommandTarget == None that does this resumes the entire program.
          resumeTarget();
        }
        break;
      case CommandTarget::Request:
        // Dispatch this command to the correct request.
        {
          const auto threadId = command->targetThreadId();
          auto it = m_requestIdMap.find(threadId);
          if (it != m_requestIdMap.end()) {
            const auto request = m_requests.find(it->second);
            assert(request != m_requests.end());
            request->second->m_commandQueue.dispatchCommand(command);

            // Lifetime of command is now owned by the request thread's queue.
            command = nullptr;
          } else {
            constexpr char* errorMsg =
              "The requested thread ID does not exist in the target.";
            reportClientMessageError(message, errorMsg);
          }
        }
        break;
      case CommandTarget::Dummy:
        // Dispatch this command to the dummy thread.
        m_session->enqueueDummyCommand(command);

        // Lifetime of command is now owned by the dummy thread's queue.
        command = nullptr;
        break;
      default:
        assert(false);
    }
  } catch (DebuggerCommandException e) {
    reportClientMessageError(message, e.what());
  } catch (...) {
    reportClientMessageError(message, InternalErrorMsg);
  }
}

void Debugger::waitForClientConnection() {
  std::unique_lock<std::mutex> lock(m_connectionNotifyLock);
  if (clientConnected()) {
    return;
  }

  while (!clientConnected()) {
    m_connectionNotifyCondition.wait(lock);
  }
}

void Debugger::setClientPreferences(ClientPreferences& preferences) {
  Lock lock(m_lock);
  if (!clientConnected()) {
    return;
  }

  assert(m_session != nullptr);
  m_session->setClientPreferences(preferences);
}

void Debugger::startDummyRequest(const std::string& startupDoc) {
  Lock lock(m_lock);
  if (!clientConnected()) {
    return;
  }

  assert(m_session != nullptr);
  m_session->startDummyRequest(startupDoc);
}

void Debugger::setDummyThreadId(int64_t threadId) {
  Lock lock(m_lock);
  m_dummyThreadId = threadId;
}

}
}
