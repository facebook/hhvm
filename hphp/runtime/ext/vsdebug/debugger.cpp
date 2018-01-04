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
    }

    // TODO: (Ericblue) Set the program state to LoaderBreakpoint and wrangle
    // all threads for the initial pause + breakpoint sync here.
  } else {

    // The client has detached. Walk through any requests we are currently
    // attached to and release them if they are blocked in the debugger.
    for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
      // Shut down the request's command queue to unblock it if it's broken
      // in to the debugger and remove it from the map.
      // NOTE: The request's RequestInfo will be cleaned up and freed when the
      // request completes in requestShutdown. It is not safe to do that from
      // this thread.
      it->second->m_commandQueue.shutdown();
      m_requests.erase(it);
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
    if (pauseRequest) {
      m_pausedRequestCount++;
    }
  }

  // If the debugger was already paused when this request started, drop the lock
  // and block the request in its command queue until the debugger resumes.
  // Note: if the debugger client issues a resume between the time the lock
  // is dropped above and entering the command queue, there will be a pending
  // Resume command in this queue, which will cause this thread to unblock.
  if (pauseRequest && requestInfo != nullptr) {
    requestInfo->m_commandQueue.processCommands();

    {
      Lock lock(m_lock);
      m_pausedRequestCount--;
    }
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

void Debugger::onClientMessage(folly::dynamic& message) {
  // TODO: not implemented.
}

}
}
