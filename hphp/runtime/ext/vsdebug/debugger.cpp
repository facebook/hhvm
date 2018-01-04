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

    // Attach the debugger to any request threads that were already
    // running before the client connected. We did not attach to them if they
    // were initialized when the client was disconnected to avoid taking a perf
    // hit for a debugger hook that wasn't going to be used.
    //
    // Only do this after seeing at least one request via the requestInit path:
    // on initial startup in script mode, the script request thread will have
    // been created already in HHVM main, but is not ready for us to attach
    // yet because extensions are still being initialized.
    if (m_totalRequestCount.load() > 0) {
      ThreadInfo::ExecutePerThread([this] (ThreadInfo* ti) {
        this->attachToRequest(ti);
      });
    }

    m_session = new DebuggerSession(this);
    if (m_session == nullptr) {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelError,
        "Failed to allocate debugger session!"
      );
      m_clientConnected.store(false, std::memory_order_release);
    }

    // If the script startup thread is waiting for a client connection, wake it
    // up now.
    {
      std::unique_lock<std::mutex> lock(m_connectionNotifyLock);
      m_connectionNotifyCondition.notify_all();
    }

    // TODO: (Ericblue) Set the program state to LoaderBreakpoint and wrangle
    // all threads for the initial pause + breakpoint sync here.
  } else {

    // The client has detached. Walk through any requests we are currently
    // attached to and release them if they are blocked in the debugger.
    for (auto it = m_requests.begin(); it != m_requests.end();) {
      // Shut down the request's command queue to unblock it if it's broken
      // in to the debugger and remove it from the map.
      // NOTE: The request's RequestInfo will be cleaned up and freed when the
      // request completes in requestShutdown. It is not safe to do that from
      // this thread.
      it->second->m_commandQueue.shutdown();
      it = m_requests.erase(it);
    }

    m_state = ProgramState::Running;
    assert(m_requests.empty());
  }
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

  m_transport->shutdown();
  setClientConnected(false);

  // m_session is deleted and set to nullptr by setClientConnected(false).
  assert(m_session == nullptr);

  delete m_transport;
  m_transport = nullptr;
}

void Debugger::sendUserMessage(const char* message, const char* level) {
  Lock lock(m_lock);
  if (m_transport != nullptr) {
    m_transport->enqueueOutgoingUserMessage(message, level);
  }
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
    requestInfo = attachToRequest(threadInfo);
    pauseRequest = m_state != ProgramState::Running;
  }

  // If the debugger was already paused when this request started, drop the lock
  // and block the request in its command queue until the debugger resumes.
  // Note: if the debugger client issues a resume between the time the lock
  // is dropped above and entering the command queue, there will be a pending
  // Resume command in this queue, which will cause this thread to unblock.
  if (pauseRequest && requestInfo != nullptr) {
    processCommandQueue(requestInfo);
  }
}

void Debugger::processCommandQueue(RequestInfo* requestInfo) {
  {
    Lock lock(m_lock);
    m_pausedRequestCount++;
  }

  requestInfo->m_commandQueue.processCommands();

  {
    Lock lock(m_lock);
    m_pausedRequestCount--;
  }
}

RequestInfo* Debugger::attachToRequest(ThreadInfo* ti) {
  // Note: the caller of this routine must hold m_lock.
  RequestInfo* requestInfo = nullptr;

  auto it = m_requests.find(ti);
  if (it == m_requests.end()) {
    // New request. Insert a request info object into our map.
    requestInfo = new RequestInfo();
    if (requestInfo == nullptr) {
      // Failed to allocate request info.
      return nullptr;
    }

    m_requests.emplace(std::make_pair(ti, requestInfo));

    auto const threadId = (int64_t)Process::GetThreadId();
    m_requestIds.emplace(std::make_pair(threadId, ti));
  } else {
    requestInfo = it->second;
  }

  assert(requestInfo != nullptr);

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

  {
    Lock lock(m_lock);
    auto it = m_requests.find(threadInfo);
    if (it != m_requests.end()) {
      requestInfo = it->second;
      m_requests.erase(it);
    }

    auto const threadId = (int64_t)Process::GetThreadId();
    auto idItr = m_requestIds.find(threadId);
    if (idItr != m_requestIds.end()) {
      m_requestIds.erase(idItr);
    }
  }

  if (requestInfo != nullptr) {
    cleanupRequestInfo(threadInfo, requestInfo);
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
    sendCommandResponse(command, responseMsg);

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

void Debugger::onClientMessage(folly::dynamic& message) {
  Lock lock(m_lock);

  // It's possible the client disconnected between the time the message was
  // received and when the lock was acquired in this routine. If the client
  // has gone, do not process the message.
  if (!clientConnected()) {
    return;
  }

  VSCommand* command = nullptr;

  try {
    if (!VSCommand::parseCommand(this, message, &command)) {
      assert(command == nullptr);
      throw DebuggerCommandException(
        "The command was invalid or is not implemented in the debugger."
      );
    }

    assert(command != nullptr);
    enforceRequiresBreak(command);

    switch(command->commandTarget()) {
      case CommandTarget::None:
        command->execute();
        break;
      case CommandTarget::Request:
        // Dispatch this command to the correct request.
        {
          const auto threadId = command->targetThreadId();
          auto it = m_requestIds.find(threadId);
          if (it != m_requestIds.end()) {
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

  if (command != nullptr) {
    delete command;
  }
}

void Debugger::waitForClientConnection() {
  std::unique_lock<std::mutex> lock(m_connectionNotifyLock);
  if (clientConnected()) {
    return;
  }

  m_connectionNotifyCondition.wait(lock);
}

}
}
