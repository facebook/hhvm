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

#include "hphp/runtime/base/configs/debugger.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/watchman-connection.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"
#include "hphp/runtime/ext/vsdebug/debugger-request-info.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/util/process.h"
#include "hphp/util/timer.h"

#include <folly/json/dynamic.h>

namespace HPHP {
namespace VSDEBUG {

Debugger::Debugger() :
  m_sessionCleanupThread(this, &Debugger::runSessionCleanupThread) {

  m_sessionCleanupThread.start();
}

void Debugger::setTransport(DebugTransport* transport) {
  assertx(m_transport == nullptr);
  m_transport = transport;
  setClientConnected(m_transport->clientConnected());
}

bool Debugger::getDebuggerOption(const HPHP::String& option) {
  std::string optionStr = option.toCppString();

  // It's easier for the user if this routine is not case-sensitive.
  std::transform(optionStr.begin(), optionStr.end(), optionStr.begin(),
    [](unsigned char c){ return std::tolower(c); });

  DebuggerOptions options = getDebuggerOptions();

  if (optionStr == "showdummyonasyncpause") {
    return options.showDummyOnAsyncPause;
  } else if (optionStr == "warnoninterceptedfunctions") {
    return options.warnOnInterceptedFunctions;
  } else if (optionStr == "notifyonbpcalibration") {
    return options.notifyOnBpCalibration;
  } else if (optionStr == "disableuniquevarref") {
    return options.disableUniqueVarRef;
  } else if (optionStr == "disablepostdummyevalhelper") {
    return options.disablePostDummyEvalHelper;
  } else if (optionStr == "disableStdoutRedirection") {
    return options.disableStdoutRedirection;
  } else if (optionStr == "disablejit") {
    return options.disableJit;
  } else if (optionStr == "warnonfilechange") {
    return options.warnOnFileChange;
  } else {
    raise_error("getDebuggerOption: Unknown option specified");
  }
}

void Debugger::setDebuggerOption(const HPHP::String& option, bool value) {
  std::string optionStr = option.toCppString();

  // It's easier for the user if this routine is not case-sensitive.
  std::transform(optionStr.begin(), optionStr.end(), optionStr.begin(),
    [](unsigned char c){ return std::tolower(c); });

  DebuggerOptions options = getDebuggerOptions();

  if (optionStr == "showdummyonasyncpause") {
    options.showDummyOnAsyncPause = value;
  } else if (optionStr == "warnoninterceptedfunctions") {
    options.warnOnInterceptedFunctions = value;
  } else if (optionStr == "notifyonbpcalibration") {
    options.notifyOnBpCalibration = value;
  } else if (optionStr == "disableuniquevarref") {
    options.disableUniqueVarRef = value;
  } else if (optionStr == "disablepostdummyevalhelper") {
    options.disablePostDummyEvalHelper = value;
  } else if (optionStr == "disablestdoutredirection") {
    options.disableStdoutRedirection = value;
  } else if (optionStr == "disablejit") {
    options.disableJit = value;
  } else if (optionStr == "warnonfilechange") {
    options.warnOnFileChange = value;
  } else {
    raise_error("setDebuggerOption: Unknown option specified");
  }

  setDebuggerOptions(options);
}

void Debugger::setDebuggerOptions(DebuggerOptions options) {
  Lock lock(m_lock);
  m_debuggerOptions = options;

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Client options set:\n"
      "showDummyOnAsyncPause: %s\n"
      "warnOnInterceptedFunctions: %s\n"
      "notifyOnBpCalibration: %s\n"
      "disableUniqueVarRef: %s\n"
      "disablePostDummyEvalHelper: %s\n"
      "maxReturnedStringLength: %d\n"
      "disableStdoutRedirection: %s\n"
      "disableJit: %s\n"
      "warnOnFileChange: %s\n",
    options.showDummyOnAsyncPause ? "YES" : "NO",
    options.warnOnInterceptedFunctions ? "YES" : "NO",
    options.notifyOnBpCalibration ? "YES" : "NO",
    options.disableUniqueVarRef ? "YES" : "NO",
    options.disablePostDummyEvalHelper ? "YES" : "NO",
    options.maxReturnedStringLength,
    options.disableStdoutRedirection ? "YES" : "NO",
    options.disableJit ? "YES" : "NO",
    options.warnOnFileChange ? "YES" : "NO"
  );
}

void Debugger::initWatchmanClient() {
  assertx(getDebuggerOptions().warnOnFileChange);
  m_watchmanClient =
    get_watchman_client(SourceRootInfo::GetCurrentSourceRoot());
}

std::shared_ptr<Watchman> Debugger::getWatchmanClient() const {
  return m_watchmanClient;
}

void Debugger::checkForFileChanges(DebuggerRequestInfo* ri) {
  if (!m_watchmanClient || !getDebuggerOptions().warnOnFileChange) return;
  const auto& loadedFiles = g_context->m_loadedUnits;
  if (!ri->m_lastClock || loadedFiles.empty()) {
    auto clock = m_watchmanClient->getClock().getTry();
    if (clock.hasException()) return;
    ri->m_lastClock = *clock;
  } else {
    folly::dynamic queryObj =
      folly::dynamic::object("fields",  folly::dynamic::array("name"));
    queryObj["since"] = *ri->m_lastClock;
    auto const& res = m_watchmanClient->query(queryObj).getTry();
    if (res.hasException()) return;
    folly::dynamic validRes = *res;
    ri->m_lastClock = validRes["clock"].asString();
    std::vector<std::string> changedFiles;
    for (auto const& file : validRes["files"]) {
      auto fullPath =
        SourceRootInfo::GetCurrentSourceRoot().native() + file.getString();
      auto staticPath = makeStaticString(fullPath);
      if (loadedFiles.find(staticPath) != loadedFiles.end()) {
        changedFiles.push_back(staticPath->data());
      }
    }
    if (!changedFiles.empty()) {
      std::string msg =
        "Following files have changed since last loaded by the REPL:\n";
      for (auto s : changedFiles) {
        msg += s + '\n';
      }
      msg += "Reload the debugger to reload the modified files.\n";
      msg += "Run hphp_debugger_set_option('warnOnFileChange', false) "
             "to disable this message.\n";

      sendUserMessage(msg.c_str(), DebugTransport::OutputLevelInfo);
    }
  }
}

void Debugger::runSessionCleanupThread() {
  bool terminating = false;
  std::unordered_set<DebuggerSession*> sessionsToDelete;

  while (true) {
    {
      std::unique_lock<std::mutex> lock(m_sessionCleanupLock);

      // Read m_sessionCleanupTerminating while the lock is held.
      terminating = m_sessionCleanupTerminating;

      if (!terminating) {
        // Wait for signal
        m_sessionCleanupCondition.wait(lock);

        // Re-check terminating flag while the lock is still re-acquired.
        terminating = m_sessionCleanupTerminating;
      }

      // Make a local copy of the session pointers to delete and drop
      // the lock.
      sessionsToDelete = m_cleanupSessions;
      m_cleanupSessions.clear();
    }

    // Free the sessions.
    for (DebuggerSession* sessionToDelete : sessionsToDelete) {
      delete sessionToDelete;
    }

    if (terminating) {
      break;
    }
  }

}

void Debugger::setClientConnected(
  bool connected,
  bool synchronous /* = false */,
  ClientInfo* clientInfo /* = nullptr */
) {
  always_assert_flog(
    !connected || !m_clientConnected.load(std::memory_order_acquire),
    "Trying to connect a client when another client is alreay connected");
  DebuggerSession* sessionToDelete = nullptr;
  SCOPE_EXIT {
    if (sessionToDelete != nullptr) {
      // Unless the debugger is shutting down (in which case we need to
      // join with the worker threads), delete the session asynchronously.
      // The session destructor needs to wait to join with the dummy thread,
      // which could take a while if it's running native code.
      if (synchronous) {
        delete sessionToDelete;
      } else {
        std::unique_lock<std::mutex> lock(m_sessionCleanupLock);
        m_cleanupSessions.insert(sessionToDelete);
        m_sessionCleanupCondition.notify_all();
      }

      VSDebugLogger::LogFlush();
    }
  };

  {
    Lock lock(m_lock);

    VSDebugLogger::SetLogRotationEnabled(connected);

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
      // Ensure usage logging is initialized if configured. Init is a no-op if
      // the logger is already initialized.
      auto logger = Eval::Debugger::GetUsageLogger();
      if (logger != nullptr) {
        logger->init();

        if (clientInfo == nullptr) {
          VSDebugLogger::Log(
            VSDebugLogger::LogLevelInfo,
            "Clearing client info. Connected client has no ID."
          );
          logger->clearClientInfo();
        } else {
          VSDebugLogger::Log(
            VSDebugLogger::LogLevelInfo,
            "Setting connected client info: user=%s,uid=%d,pid=%d",
            clientInfo->clientUser.c_str(),
            clientInfo->clientUid,
            clientInfo->clientPid
          );
          logger->setClientInfo(
            clientInfo->clientUser,
            clientInfo->clientUid,
            clientInfo->clientPid
          );
        }
      } else {
        VSDebugLogger::Log(
          VSDebugLogger::LogLevelWarning,
          "Not logging user: no usage logger configured."
        );
      }

      VSDebugLogger::LogFlush();

      // Create a new debugger session.
      assertx(m_session == nullptr);
      m_session = new DebuggerSession(this);
      if (m_session == nullptr) {
        VSDebugLogger::Log(
          VSDebugLogger::LogLevelError,
          "Failed to allocate debugger session!"
        );
        m_clientConnected.store(false, std::memory_order_release);
      }
      if (clientInfo) {
        m_session->setClientUser(clientInfo->clientUser);
      }

      // When the client connects, break the entire program to get it into a
      // known state, set initial breakpoints and then wait for
      // the client to send a configurationDone command, which will resume
      // the target.
      m_state = ProgramState::LoaderBreakpoint;

      // Attach the debugger to any request threads that were already
      // running before the client connected. We did not attach to them if they
      // were initialized when the client was disconnected to avoid taking a
      // perf hit for a debugger hook that wasn't going to be used.
      //
      // Only do this after seeing at least 1 request via the requestInit path
      // in script mode: on initial startup the script request thread will have
      // been created already in HHVM main, but is not ready for us to attach
      // yet because extensions are still being initialized.
      if (RuntimeOption::ServerExecutionMode() ||
          m_totalRequestCount.load() > 0) {

        RequestInfo::ExecutePerRequest([this] (RequestInfo* ti) {
          this->attachToRequest(ti);
        });
      }

      executeForEachAttachedRequest(
        [&](RequestInfo* ti, DebuggerRequestInfo* ri) {
          if (ri->m_breakpointInfo == nullptr) {
            ri->m_breakpointInfo = new RequestBreakpointInfo();
          }

          if (ri->m_breakpointInfo == nullptr) {
            VSDebugLogger::Log(
              VSDebugLogger::LogLevelError,
              "Failed to allocate request breakpoint info!"
            );
          }
        },
        true /* includeDummyRequest */
      );

      // If the script startup thread is waiting for a client connection, wake
      // it up now.
      {
        std::unique_lock<std::mutex> lock(m_connectionNotifyLock);
        m_connectionNotifyCondition.notify_all();
      }
    } else {
      // disconnected case
      auto logger = Eval::Debugger::GetUsageLogger();
      if (logger != nullptr) {
        VSDebugLogger::Log(
          VSDebugLogger::LogLevelInfo,
          "Clearing client info - user disconnected."
        );
        logger->clearClientInfo();
      }

      // The client has detached. Walk through any requests we are currently
      // attached to and release them if they are blocked in the debugger.
      executeForEachAttachedRequest(
        [&](RequestInfo* ti, DebuggerRequestInfo* ri) {
          // Clear any undelivered client messages in the request's command
          // queue, since they apply to the debugger session that just ended.
          // NOTE: The request's DebuggerRequestInfo will be cleaned up and freed when
          // the request completes in requestShutdown. It is not safe to do that
          // from this thread.
          ri->m_commandQueue.clearPendingMessages();

          // Have the debugger hook update the output hook on next interrupt.
          ri->m_flags.outputHooked = false;
          ti->m_reqInjectionData.setDebuggerIntr(true);

          // Clear the breakpoint info.
          if (ri->m_breakpointInfo != nullptr) {
            delete ri->m_breakpointInfo;
            ri->m_breakpointInfo = nullptr;
          }

          ri->m_breakpointInfo = new RequestBreakpointInfo();
          if (ri->m_breakpointInfo != nullptr) {
            assertx(ri->m_breakpointInfo->m_pendingBreakpoints.empty());
            assertx(ri->m_breakpointInfo->m_unresolvedBreakpoints.empty());
          }
          updateUnresolvedBpFlag(ri);
        },
        true /* includeDummyRequest */
      );

      m_clientInitialized = false;

      // If we launched this request in debugger launch mode, detach of
      // the client should terminate the request.
      if (VSDebugExtension::s_launchMode) {
        interruptAllThreads();
      }

      DebuggerHook::setActiveDebuggerInstance(
        VSDebugHook::GetInstance(),
        false
      );

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
    if (it->second->m_executing == RequestInfo::Executing::UserFunctions ||
        it->second->m_executing == RequestInfo::Executing::RuntimeFunctions) {
      sendThreadEventMessage(it->first, ThreadEventType::ThreadStarted);
    }
  }
}

std::string Debugger::getDebuggerSessionAuth() {
  Lock lock(m_lock);

  if (!clientConnected() || getCurrentThreadId() != kDummyTheadId) {
    return "";
  }

  return m_session->getDebuggerSessionAuth();
}

request_id_t Debugger::getCurrentThreadId() {
  Lock lock(m_lock);

  if (isDummyRequest()) {
    return kDummyTheadId;
  }

  RequestInfo* const threadInfo = &RI();
  const auto it = m_requestInfoMap.find(threadInfo);
  if (it == m_requestInfoMap.end()) {
    return -1;
  }

  return it->second;
}

void Debugger::cleanupRequestInfo(RequestInfo* ti, DebuggerRequestInfo* ri) {
  std::atomic_thread_fence(std::memory_order_acquire);
  if (ti != nullptr && isDebuggerAttached(ti)) {
    DebuggerHook::detach(ti);
  }

  // Shut down the request's command queue. This releases the thread if
  // it is waiting inside the queue.
  ri->m_commandQueue.shutdown();

  if (ri->m_breakpointInfo != nullptr) {
    delete ri->m_breakpointInfo;
  }

  assertx(ri->m_serverObjects.size() == 0);
  delete ri;
}

void Debugger::cleanupServerObjectsForRequest(DebuggerRequestInfo* ri) {
  m_lock.assertOwnedBySelf();

  std::unordered_map<unsigned int, ServerObject*>& objs = ri->m_serverObjects;

  for (auto it = objs.begin(); it != objs.end();) {
    unsigned int objectId = it->first;

    if (m_session != nullptr) {
      m_session->onServerObjectDestroyed(objectId);
    }

    // Free the object. Note if the object is a variable in request memory,
    // the destruction of ServerObject releases the GC root we're holding.
    ServerObject* object = it->second;
    delete object;

    it = objs.erase(it);
  }

  assertx(objs.size() == 0);
}

void Debugger::executeForEachAttachedRequest(
  std::function<void(RequestInfo* ti, DebuggerRequestInfo* ri)> callback,
  bool includeDummyRequest
) {
  m_lock.assertOwnedBySelf();

  DebuggerRequestInfo* dummyRequestInfo = getDummyRequestInfo();
  std::for_each(m_requests.begin(), m_requests.end(), [](auto &it) {
    it.second->m_flags.alive = false;
  });


  RequestInfo::ExecutePerRequest([&] (RequestInfo* ti) {
    auto ri = ti->m_reqInjectionData.getDebuggerRequestInfo();
    if (ri && ri != dummyRequestInfo) {
      ri->m_flags.alive = true;
      callback(ti, ri);
    }
  });

  // This is to catch requests that have fallen out of the list without a
  // requestShutdown notification.
  auto it = m_requests.begin();
  while (it != m_requests.end()) {
    if (it->second->m_flags.alive) {
      it++;
      continue;
    }
    it->first->m_reqInjectionData.setDebuggerRequestInfo(nullptr);
    it = m_requests.erase(it);
  }

  if (includeDummyRequest && dummyRequestInfo != nullptr) {
    callback(nullptr, dummyRequestInfo);
  }
}

void Debugger::getAllThreadInfo(folly::dynamic& threads) {
  assertx(threads.isArray());
  executeForEachAttachedRequest(
    [&](RequestInfo* ti, DebuggerRequestInfo* ri) {
      threads.push_back(folly::dynamic::object);
      folly::dynamic& threadInfo = threads[threads.size() - 1];

      auto it = m_requestInfoMap.find(ti);
      if (it != m_requestInfoMap.end()) {
        request_id_t requestId = it->second;
        threadInfo["id"] = requestId;
        threadInfo["name"] = std::string("Request ") +
                             std::to_string(requestId);

        if (!ri->m_requestUrl.empty()) {
          threadInfo["name"] += ": " + ri->m_requestUrl;
        }
      }
    },
    false /* includeDummyRequest */
  );

  // the dummy request isn't in the request map, so add info for it manually
  threads.push_back(folly::dynamic::object);
  folly::dynamic& threadInfo = threads[threads.size() - 1];
  threadInfo["id"] = 0;
  threadInfo["name"] = std::string("Console/REPL request");
}

void Debugger::shutdown() {
  if (m_transport == nullptr) {
    return;
  }

  trySendTerminatedEvent();

  m_transport->shutdown();
  setClientConnected(false, true);

  // m_session is deleted and set to nullptr by setClientConnected(false).
  assertx(m_session == nullptr);

  delete m_transport;
  m_transport = nullptr;

  {
    std::unique_lock<std::mutex> lock(m_sessionCleanupLock);
    m_sessionCleanupTerminating = true;
    m_sessionCleanupCondition.notify_all();
  }

  m_sessionCleanupThread.waitForEnd();
}

void Debugger::trySendTerminatedEvent() {
  Lock lock(m_lock);

  folly::dynamic event = folly::dynamic::object;
  sendEventMessage(event, "terminated", true);
}

void Debugger::sendStoppedEvent(
  const char* reason,
  const char* displayReason,
  request_id_t threadId,
  bool focusedThread,
  int breakpointId
) {
  Lock lock(m_lock);

  folly::dynamic event = folly::dynamic::object;
  const bool allThreadsStopped = m_pausedRequestCount == m_requests.size();

  if (!allThreadsStopped && threadId < 0) {
    // Don't send a stop message for a specific thread if this stop
    // event doesn't have a valid thread id.
    return;
  }

  event["allThreadsStopped"] = allThreadsStopped;

  if (threadId >= 0) {
    event["threadId"] = threadId;
  }

  if (reason != nullptr) {
    event["reason"] = reason;
  }

  if (breakpointId >= 0) {
    event["breakpointId"] = breakpointId;
  }

  if (displayReason == nullptr) {
    event["description"] = "execution paused";
  } else {
    event["description"] = displayReason;
  }

  event["preserveFocusHint"] = !focusedThread;
  sendEventMessage(event, "stopped");
}

void Debugger::sendContinuedEvent(request_id_t threadId) {
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
    m_transport->enqueueOutgoingUserMessage(
      getCurrentThreadId(), message, level
    );
  }
}

void Debugger::sendEventMessage(
  folly::dynamic& event,
  const char* eventType,
  bool sendImmediately /* = false */
) {
  Lock lock(m_lock);

  // Deferred events are sent after the response to the current debugger client
  // request is sent, except in the case where we're hitting a nested breakpoint
  // during an evaluation: that we must send immediately because the evaluation
  // response won't happen until the client resumes from the inner breakpoint.
  DebuggerRequestInfo* ri = getRequestInfo();
  if (ri != nullptr &&
      (ri->m_evaluateCommandDepth > 0 || ri-> m_pauseRecurseCount > 0)) {

    sendImmediately = true;
  }

  if (!sendImmediately && m_processingClientCommand) {
    // If an outgoing client event is generated while the debugger is processing
    // a command for the client, keep it in a queue and send it after the
    // response is sent for the current client command.
    PendingEventMessage pendingEvent;
    pendingEvent.m_message = event;
    pendingEvent.m_eventType = eventType;
    m_pendingEventMessages.push_back(pendingEvent);
  } else {
    // Otherwise, go ahead and send it immediately.
    if (m_transport != nullptr) {
      m_transport->enqueueOutgoingEventMessage(event, eventType);
    }
  }
}

void Debugger::sendThreadEventMessage(
  request_id_t threadId,
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

  event["threadId"] = threadId;

  sendEventMessage(event, "thread", true);
}

void Debugger::requestInit() {
  Lock lock(m_lock);

  m_totalRequestCount++;

  if (!clientConnected()) {
    // Don't pay for attaching to the thread if no debugger client is connected.
    return;
  }

  RequestInfo* const threadInfo = &RI();
  bool pauseRequest;
  DebuggerRequestInfo* requestInfo;

  bool dummy = isDummyRequest();

  // Don't attach to the dummy request thread. DebuggerSession manages the
  // dummy requests debugger hook state.
  if (!dummy) {
    requestInfo = attachToRequest(threadInfo);
    pauseRequest = m_state != ProgramState::Running;
  } else {
    pauseRequest = false;
  }

  // If the debugger was already paused when this request started, block the
  // request in its command queue until the debugger resumes.
  if (pauseRequest && requestInfo != nullptr) {
    processCommandQueue(
      getCurrentThreadId(),
      requestInfo,
      "entry",
      nullptr,
      false,
      -1
    );
  }
}

void Debugger::enterDebuggerIfPaused(DebuggerRequestInfo* requestInfo) {
  Lock lock(m_lock);

  if (!clientConnected() && VSDebugExtension::s_launchMode) {
    // If the debugger client launched this script in launch mode, and
    // has detached while the request is still running, terminate the
    // request by throwing a fatal PHP exception.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Debugger client detached and we launched this script. "
        "Killing request with fatal error."
    );

    raise_fatal_error(
      "Request terminated due to debugger client detaching.",
      null_array,
      false,
      true,
      true
    );
  }

  if (m_state != ProgramState::Running) {
    if (requestInfo->m_stepReason != nullptr) {
      processCommandQueue(
        getCurrentThreadId(),
        requestInfo,
        "step",
        requestInfo->m_stepReason,
        true,
        -1
      );
    } else if (m_state == ProgramState::AsyncPaused) {
      processCommandQueue(
        getCurrentThreadId(),
        requestInfo,
        "async-pause",
        "async-pause",
        true,
        -1
      );
    } else {
      processCommandQueue(
        getCurrentThreadId(),
        requestInfo,
        "pause",
        nullptr,
        false,
        -1
      );
    }
  }
}

void Debugger::processCommandQueue(
  request_id_t threadId,
  DebuggerRequestInfo* requestInfo,
  const char* reason,
  const char* displayReason,
  bool focusedThread,
  int bpId
) {
  m_lock.assertOwnedBySelf();

  if (requestInfo->m_pauseRecurseCount == 0) {
    m_pausedRequestCount++;
  }

  requestInfo->m_pauseRecurseCount++;
  requestInfo->m_totalPauseCount++;

  // Don't actually tell the client about the stop if it's due to the
  // loader breakpoint, this is an internal implementation detail that
  // should not be visible to the client.
  bool sendEvent = m_state != ProgramState::LoaderBreakpoint;
  if (sendEvent) {
    sendStoppedEvent(reason, displayReason, threadId, focusedThread, bpId);
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Thread %d pausing",
    threadId
  );

  // Drop the lock before entering the command queue and re-acquire it
  // when leaving the command queue.
  m_lock.unlock();
  requestInfo->m_commandQueue.processCommands();
  m_lock.lock();

  requestInfo->m_pauseRecurseCount--;
  if (requestInfo->m_pauseRecurseCount == 0) {
    m_pausedRequestCount--;
  }

  if (sendEvent) {
    sendContinuedEvent(threadId);
  }

  // Any server objects stored for the client for this request are invalid
  // as soon as the thread is allowed to step.
  cleanupServerObjectsForRequest(requestInfo);

  // Once any request steps, we must invalidate cached globals and constants,
  // since the user code could have modified them.
  if (m_session != nullptr) {
    m_session->clearCachedVariable(DebuggerSession::kCachedVariableKeyAll);
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Thread %d resumed",
    threadId
  );

  m_resumeCondition.notify_all();
}

DebuggerRequestInfo* Debugger::createRequestInfo() {
  DebuggerRequestInfo* requestInfo = new DebuggerRequestInfo();
  if (requestInfo == nullptr) {
    // Failed to allocate request info.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to allocate request info!"
    );
    return nullptr;
  }

  assertx(requestInfo->m_allFlags == 0);

  requestInfo->m_breakpointInfo = new RequestBreakpointInfo();
  if (requestInfo->m_breakpointInfo == nullptr) {
    // Failed to allocate breakpoint info.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to allocate request breakpoint info!"
    );
    delete requestInfo;
    return nullptr;
  }

  return requestInfo;
}

request_id_t Debugger::nextThreadId() {
  request_id_t threadId = m_nextThreadId++;

  // Unlikely: handle rollver. Id 0 is reserved for the dummy, and then
  // in the very unlikley event that there's a very long running request
  // we need to ensure we don't reuse its id.
  if (threadId == 0) {
    threadId++;
  }

  while (m_requestIdMap.find(threadId) != m_requestIdMap.end()) {
    threadId++;
  }

  return threadId;
}

DebuggerRequestInfo* Debugger::attachToRequest(RequestInfo* ti) {
  m_lock.assertOwnedBySelf();

  DebuggerRequestInfo* requestInfo = nullptr;

  request_id_t threadId;
  auto it = m_requests.find(ti);
  if (it == m_requests.end()) {
    // New request. Insert a request info object into our map.
    threadId = nextThreadId();
    assertx(threadId > 0);

    requestInfo = createRequestInfo();
    if (requestInfo == nullptr) {
      // Failed to allocate request info.
      return nullptr;
    }

    m_requests.emplace(std::make_pair(ti, requestInfo));
    m_requestIdMap.emplace(std::make_pair(threadId, ti));
    m_requestInfoMap.emplace(std::make_pair(ti, threadId));

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Created new request info for request %d (current thread=%d), flags=%u",
      threadId,
      (int64_t)Process::GetThreadId(),
      static_cast<unsigned int>(requestInfo->m_allFlags)
    );

  } else {
    requestInfo = it->second;
    always_assert(
      requestInfo == ti->m_reqInjectionData.getDebuggerRequestInfo()
    );
    auto idIt = m_requestInfoMap.find(ti);
    assertx(idIt != m_requestInfoMap.end());
    threadId = idIt->second;

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Found existing request info for thread %d, flags=%u",
      threadId,
      static_cast<unsigned int>(requestInfo->m_allFlags)
    );
  }

  assertx(requestInfo != nullptr && requestInfo->m_breakpointInfo != nullptr);

  // Have the debugger hook update the output hook when we enter the debugger.
  requestInfo->m_flags.outputHooked = false;

  ti->m_reqInjectionData.setDebuggerIntr(true);

  if (ti->m_executing == RequestInfo::Executing::UserFunctions ||
      ti->m_executing == RequestInfo::Executing::RuntimeFunctions) {
    sendThreadEventMessage(threadId, ThreadEventType::ThreadStarted);
  }

  // Try to attach our debugger hook to the request.
  if (!isDebuggerAttached(ti)) {
    if (DebuggerHook::attach<VSDebugHook>(ti)) {
      // Install all breakpoints as pending for this request.
      const std::unordered_set<int> breakpoints =
        m_session->getBreakpointManager()->getAllBreakpointIds();
      for (auto it = breakpoints.begin(); it != breakpoints.end(); it++) {
        requestInfo->m_breakpointInfo->m_pendingBreakpoints.emplace(*it);
      }
      auto exceptionBpMode =
        m_session->getBreakpointManager()->getExceptionBreakMode();
      requestInfo->m_breakpointInfo->m_hasExceptionBreakpoint =
        (exceptionBpMode != ExceptionBreakMode::BreakNone);
    } else {
      m_transport->enqueueOutgoingUserMessage(
        kDummyTheadId,
        "Failed to attach to new HHVM request: another debugger is already "
          "attached.",
        DebugTransport::OutputLevelError
      );

      VSDebugLogger::Log(
        VSDebugLogger::LogLevelError,
        "Failed to attach to new HHVM request: another debugger is already "
          "attached."
      );
    }
  } else {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Not attaching to request %d, a debug hook is already attached.",
      threadId
    );
  }

  updateUnresolvedBpFlag(requestInfo);
  // This must be set last because request threads can access their own
  // DebuggerRequestInfo without grabbing a lock.
  ti->m_reqInjectionData.setDebuggerRequestInfo(requestInfo);
  return requestInfo;
}

void Debugger::requestShutdown() {
  auto const threadInfo = &RI();
  DebuggerRequestInfo* requestInfo = nullptr;
  request_id_t threadId = -1;

  SCOPE_EXIT {
    if (clientConnected() && threadId >= 0) {
      sendThreadEventMessage(threadId, ThreadEventType::ThreadExited);
      m_session->getBreakpointManager()->onRequestShutdown(threadId);
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
    threadInfo->m_reqInjectionData.setDebuggerRequestInfo(nullptr);

    auto infoItr = m_requestInfoMap.find(threadInfo);
    assertx(infoItr != m_requestInfoMap.end());

    threadId = infoItr->second;
    auto idItr = m_requestIdMap.find(threadId);
    assertx(idItr != m_requestIdMap.end());

    m_requestIdMap.erase(idItr);
    m_requestInfoMap.erase(infoItr);

    if (!g_context.isNull()) {
      g_context->removeStdoutHook(getStdoutHook());
    }
    Logger::SetThreadHook(nullptr);

    // Cleanup any server objects for this request before dropping the lock.
    cleanupServerObjectsForRequest(requestInfo);
  }
}

DebuggerRequestInfo* Debugger::getDummyRequestInfo() {
  m_lock.assertOwnedBySelf();
  if (!clientConnected()) {
    return nullptr;
  }

  return m_session->m_dummyRequestInfo;
}

DebuggerRequestInfo* Debugger::getRequestInfo(request_id_t threadId /* = -1 */) {

  if (threadId != -1 || isDummyRequest()) {
    Lock lock(m_lock);
    // Find the info for the requested thread ID.
    if (threadId == kDummyTheadId || isDummyRequest()) {
      return getDummyRequestInfo();
    }

    auto it = m_requestIdMap.find(threadId);
    if (it != m_requestIdMap.end()) {
      auto requestIt = m_requests.find(it->second);
      if (requestIt != m_requests.end()) {
        return requestIt->second;
      }
    }
  } else {
    // Find the request info for the current request thread.
    return RID().getDebuggerRequestInfo();
  }

  return nullptr;
}

bool Debugger::executeClientCommand(
  VSCommand* command,
  std::function<bool(DebuggerSession* session,
                     folly::dynamic& responseMsg)> callback
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

    m_processingClientCommand = true;
    SCOPE_EXIT {
      m_processingClientCommand = false;

      for (PendingEventMessage& message : m_pendingEventMessages) {
        sendEventMessage(message.m_message, message.m_eventType);
      }

      m_pendingEventMessages.clear();
    };

    std::string cmd = command->commandName();
    if (cmd == "InitializeCommand") {
      setClientIdFromCommand(command);
    }

    // Log command if logging is enabled.
    logClientCommand(command);

    bool resumeThread = callback(m_session, responseMsg);
    if (command->commandTarget() != CommandTarget::WorkItem) {
      sendCommandResponse(command, responseMsg);
    }
    return resumeThread;
  } catch (DebuggerCommandException &e) {
    reportClientMessageError(command->getMessage(), e.what());
  } catch (...) {
    reportClientMessageError(command->getMessage(), InternalErrorMsg);
  }

  // On error, do not resume the request thread.
  return false;
}

void Debugger::executeWithoutLock(std::function<void()> callback) {
  m_lock.assertOwnedBySelf();
  m_lock.unlock();

  callback();

  m_lock.lock();
  m_lock.assertOwnedBySelf();
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
  m_lock.assertOwnedBySelf();

  m_state = ProgramState::Running;

  // Resume every paused request. Each request will send a thread continued
  // event when it exits its command loop.
  executeForEachAttachedRequest(
    [&](RequestInfo* ti, DebuggerRequestInfo* ri) {
      ri->m_stepReason = nullptr;

      if (ri->m_pauseRecurseCount > 0) {
        VSCommand* resumeCommand = ContinueCommand::createInstance(this);
        ri->m_commandQueue.dispatchCommand(resumeCommand);
      }
    },
    true /* includeDummyRequest */
  );

  sendContinuedEvent(-1);
}

Debugger::PrepareToPauseResult
Debugger::prepareToPauseTarget(DebuggerRequestInfo* requestInfo) {
  m_lock.assertOwnedBySelf();

  if ((m_state == ProgramState::Paused ||
       m_state == ProgramState::AsyncPaused) &&
      isStepInProgress(requestInfo)) {
    // A step operation for a single request is still in the middle of
    // handling whatever caused us to break execution of the other threads
    // in the first place. We don't need to wait for resume here.
    return clientConnected() ? ReadyToPause : ErrorNoClient;
  }

  // Wait here until the program is in a consistent state, and this thread
  // is ready to pause the target: this is the case when the current thread
  // is holding m_lock, the program is running (m_state == Running), and no
  // threads are still in the process of resuming from the previous break
  // (m_pausedRequestCount == 0).
  while (m_state != ProgramState::Running || m_pausedRequestCount > 0) {
    m_lock.assertOwnedBySelf();

    if (m_state != ProgramState::Running) {
      // The target is paused by another thread. Process this thread's
      // command queue to service the current break.
      // NOTE: processCommandQueue drops and re-acquires m_lock.
      if (requestInfo != nullptr) {
        processCommandQueue(
          getCurrentThreadId(),
          requestInfo,
          "pause",
          nullptr,
          false,
          -1
        );
      } else {
        // This is true only in the case of async-break, which is not
        // specific to any request. Ok to proceed here, async-break just
        // wants to break the target, and it is broken.
        break;
      }
    } else {
      assertx(m_state == ProgramState::Running && m_pausedRequestCount > 0);

      // The target is running, but at least one thread is still paused.
      // This means a resume is in progress, drop the lock and wait for
      // all threads to finish resuming.
      //
      // If a resume is currently in progress, we must wait for all threads
      // to resume execution before breaking again. Otherwise, the client will
      // see interleaved continue and stop events and the the state of the UX
      // becomes undefined.
      std::unique_lock<std::mutex> conditionLock(m_resumeMutex);

      // Need to drop m_lock before waiting.
      m_lock.unlock();

      m_resumeCondition.wait(conditionLock);
      conditionLock.unlock();

      // And re-acquire it before continuing.
      m_lock.lock();
      conditionLock.lock();
    }
  }

  m_lock.assertOwnedBySelf();
  assertx(requestInfo == nullptr || m_state == ProgramState::Running);

  return clientConnected() ? ReadyToPause : ErrorNoClient;
}

void Debugger::pauseTarget(DebuggerRequestInfo* ri, bool isAsyncPause) {
  m_lock.assertOwnedBySelf();

  m_state = isAsyncPause ? ProgramState::AsyncPaused : ProgramState::Paused;

  if (ri != nullptr) {
    clearStepOperation(ri);
  }

  interruptAllThreads();
}

void Debugger::dispatchCommandToRequest(
  request_id_t requestId,
  VSCommand* command
) {
  m_lock.assertOwnedBySelf();

  DebuggerRequestInfo* ri = nullptr;
  if (requestId == kDummyTheadId) {
    ri = getDummyRequestInfo();
  } else {
    auto it = m_requestIdMap.find(requestId);
    if (it != m_requestIdMap.end()) {
      const auto request = m_requests.find(it->second);
      assertx(request != m_requests.end());
      ri = request->second;
    }
  }

  if (ri == nullptr) {
    // Delete the command because the caller expects the command queue
    // to have taken ownership of it.
    delete command;
  } else {
    ri->m_commandQueue.dispatchCommand(command);
  }
}

void Debugger::setClientIdFromCommand(VSCommand* command) {
  always_assert(m_session);
  const folly::dynamic& message = command->getMessage();
  const folly::dynamic& args =
    VSCommand::tryGetObject(message, "arguments", VSCommand::s_emptyArgs);
  auto clientId = VSCommand::tryGetString(args, "clientID", "unknown");
  m_session->setClientId(clientId);
}

void Debugger::logClientCommand(
  VSCommand* command
) {
  auto logger = Eval::Debugger::GetUsageLogger();
  if (logger == nullptr) {
    return;
  }

  const std::string cmd(command->commandName());

  // Don't bother logging certain very chatty commands. These can
  // be sent frequently by the client UX and their arguments aren't
  // interesting from a security perspective.
  if (cmd == "CompletionsCommand" ||
      cmd == "ContinueCommand" ||
      cmd == "ResolveBreakpointsCommand" ||
      cmd == "StackTraceCommand" ||
      cmd == "ThreadsCommand") {
    return;
  }

  try {
    folly::json::serialization_opts jsonOptions;
    jsonOptions.sort_keys = true;
    jsonOptions.pretty_formatting = true;

    const std::string sandboxId = g_context.isNull()
      ? ""
      : g_context->getSandboxId().toCppString();
    const std::string data =
      folly::json::serialize(command->getMessage(), jsonOptions);
    const std::string mode = RuntimeOption::ServerExecutionMode()
      ? "vsdebug-webserver"
      : "vsdebug-script";
    logger->log(m_session->getClientId(), mode, sandboxId, cmd, data);
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelVerbose,
      "Logging client command: %s: %s\n",
      cmd.c_str(), data.c_str()
    );
  } catch (...) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Error logging client command"
    );
  }
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
    } catch (std::out_of_range &) {
      throw DebuggerCommandException(
        "Message is missing a required attribute."
      );
    }

    if (!VSCommand::parseCommand(this, message, &command)) {
      assertx(command == nullptr);

      try {
        auto cmdName = message["command"];
        if (cmdName.isString()) {
          std::string commandName = cmdName.asString();
          std::string errorMsg("The command \"");
          errorMsg += commandName;
          errorMsg += "\" was invalid or is not implemented in the debugger.";
          throw DebuggerCommandException(errorMsg.c_str());
        }
      } catch (std::out_of_range &) {
      }

      throw DebuggerCommandException(
        "The command was invalid or is not implemented in the debugger."
      );
    }

    assertx(command != nullptr);
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
          DebuggerRequestInfo* ri = nullptr;
          const auto threadId = command->targetThreadId(m_session);
          if (threadId == kDummyTheadId) {
            ri = getDummyRequestInfo();
          } else {
            auto it = m_requestIdMap.find(threadId);
            if (it != m_requestIdMap.end()) {
              const auto request = m_requests.find(it->second);
              assertx(request != m_requests.end());
              ri = request->second;
            }
          }

          if (ri == nullptr) {
            constexpr char* errorMsg =
              "The requested thread ID does not exist in the target.";
            reportClientMessageError(message, errorMsg);
          } else {
            ri->m_commandQueue.dispatchCommand(command);

            // Lifetime of command is now owned by the request thread's queue.
            command = nullptr;
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
        assertx(false);
    }
  } catch (DebuggerCommandException &e) {
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

  assertx(m_session != nullptr);
  m_session->setClientPreferences(preferences);
}

ClientPreferences Debugger::getClientPreferences() {
  Lock lock(m_lock);
  if (!clientConnected()) {
    ClientPreferences empty = {};
    return empty;
  }

  assertx(m_session != nullptr);
  return m_session->getClientPreferences();
}

void Debugger::startDummyRequest(
  const std::string& startupDoc,
  const std::string& sandboxUser,
  const std::string& sandboxName,
  const std::string& debuggerSessionAuthToken,
  bool displayStartupMsg
) {
  Lock lock(m_lock);
  if (!clientConnected()) {
    return;
  }

  assertx(m_session != nullptr);
  m_session->startDummyRequest(
    startupDoc,
    sandboxUser,
    sandboxName,
    debuggerSessionAuthToken,
    displayStartupMsg
  );
}

void Debugger::setDummyThreadId(int64_t threadId) {
  m_dummyThreadId.store(threadId, std::memory_order_release);
}

void Debugger::onExceptionBreakpointChanged(bool isSet) {
  Lock lock(m_lock);

  assertx(m_session != nullptr);
  executeForEachAttachedRequest(
    [&](RequestInfo* ti, DebuggerRequestInfo* ri) {
      ri->m_breakpointInfo->m_hasExceptionBreakpoint = isSet;
      updateUnresolvedBpFlag(ri);
      if (m_state != ProgramState::Running || ti == nullptr) {
        const auto cmd = ResolveBreakpointsCommand::createInstance(this);
        ri->m_commandQueue.dispatchCommand(cmd);
      }
    },
    true /* includeDummyRequest */
  );
}

void Debugger::onBreakpointAdded(int bpId) {
  Lock lock(m_lock);

  assertx(m_session != nullptr);

  // Now to actually install the breakpoints, each request thread needs to
  // process the bp and set it in some TLS data structures. If the program
  // is already paused, then every request thread is blocking in its command
  // loop: we'll put a work item to resolve the bp in each command queue.
  //
  // Otherwise, if the program is running, we need to gain control of each
  // request thread by interrupting it. It will install the bp when it
  // calls into the command hook on the next Hack opcode.
  executeForEachAttachedRequest(
    [&](RequestInfo* ti, DebuggerRequestInfo* ri) {
      if (ti != nullptr) {
        ti->m_reqInjectionData.setDebuggerIntr(true);
      }

      ri->m_breakpointInfo->m_pendingBreakpoints.emplace(bpId);
      updateUnresolvedBpFlag(ri);

      // If the program is running, the request thread will pick up and install
      // the breakpoint the next time it calls into the opcode hook, except for
      // the dummy thread, it's not running anything, so it won't ever call the
      // opcode hook. Ask it to resolve the breakpoint.
      //
      // Per contract with executeForEachAttachedRequest, ti == nullptr if and
      // only if DebuggerRequestInfo points to the dummy's request info.
      if (m_state != ProgramState::Running || ti == nullptr) {
        const auto cmd = ResolveBreakpointsCommand::createInstance(this);
        ri->m_commandQueue.dispatchCommand(cmd);
      }
    },
    true /* includeDummyRequest */
  );
}

void Debugger::tryInstallBreakpoints(DebuggerRequestInfo* ri) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  BreakpointManager* bpMgr = m_session->getBreakpointManager();
  phpSetExceptionBreakpoint(bpMgr->getExceptionBreakMode());
  auto bpInfo = ri->m_breakpointInfo;
  auto& pendingBps = bpInfo->m_pendingBreakpoints;
  if (pendingBps.empty()) return;

  // Create a map of the normalized file paths of all compilation units that
  // have already been loaded by this request before the debugger attached to
  // it to allow for quick lookup when resolving breakpoints. Any units loaded
  // after this will be added to the map by onCompilationUnitLoaded().
  for (auto p : g_context->m_loadedUnits) {
    bpInfo->m_loadedUnits.emplace(p.first->data(), p.second);
  }

  // For any breakpoints that are pending for this request, try to resolve
  // and install them, or mark them as unresolved.

  for (auto it = pendingBps.begin(); it != pendingBps.end();) {
    const int breakpointId = *it;
    const Breakpoint* bp = bpMgr->getBreakpointById(breakpointId);

    // Remove the breakpoint from "pending". After this point, it will
    // either be resolved, or unresolved, and no longer pending install.
    it = pendingBps.erase(it);

    // It's ok if bp was not found. The client could have removed the
    // breakpoint before this request got a chance to install it.
    if (bp == nullptr) {
      continue;
    }

    bool resolved = tryResolveBreakpoint(ri, breakpointId, bp);
    // This breakpoint could not be resolved yet. As new compilation units
    // are loaded, we'll try again.
    // In debugger clients that support multiple languages like Nuclide,
    // users tend to leave breakpoints set in files that are for other
    // debuggers. It's annoying to see warnings in those cases. Assume
    // any file path that doesn't end in PHP is ok not to tell the user
    // that the breakpoint failed to set.
    const bool phpFile =
      bp->m_path.size() >= 4 &&
      std::equal(
        bp->m_path.rbegin(),
        bp->m_path.rend(),
        std::string(".php").rbegin()
      );
    if (phpFile && !resolved) {
      std::string resolveMsg = "Warning: request ";
      resolveMsg += std::to_string(getCurrentThreadId());
      resolveMsg += " could not resolve breakpoint #";
      resolveMsg += std::to_string(breakpointId);
      resolveMsg += ". The Hack/PHP file at ";
      resolveMsg += bp->m_path;
      resolveMsg += " is not yet loaded, or failed to compile.";

      sendUserMessage(
        resolveMsg.c_str(),
        DebugTransport::OutputLevelWarning
      );
    }
    if (!resolved || bp->isRelativeBp()) {
      bpInfo->m_unresolvedBreakpoints.emplace(breakpointId);
      if (bp->m_type == BreakpointType::Function) {
        bpInfo->m_hasFuncBp = true;
      } else {
        bpInfo->m_filenamesWithBp.emplace(bp->m_filePath.filename().string());
      }
    }
  }

  updateUnresolvedBpFlag(ri);
  assertx(bpInfo->m_pendingBreakpoints.empty());
}

bool Debugger::tryResolveBreakpoint(
  DebuggerRequestInfo* ri,
  const int bpId,
  const Breakpoint* bp
) {
  if (bp->m_type == BreakpointType::Source) {
    // Search all compilation units loaded by this request for a matching
    // location for this breakpoint.
    const auto& loadedUnits = ri->m_breakpointInfo->m_loadedUnits;
    for (auto it = loadedUnits.begin(); it != loadedUnits.end(); it++) {
      if (tryResolveBreakpointInUnit(ri, bpId, bp, it->first, it->second)) {
        // Found a match, and installed the breakpoint!
        return true;
      }
    }
  } else {
    assertx(bp->m_type == BreakpointType::Function);

    const HPHP::String functionName(bp->m_function);
    Func* func = Func::lookup(functionName.get());

    if (func != nullptr) {
      BreakpointManager* bpMgr = m_session->getBreakpointManager();

      if ((func->fullName() != nullptr &&
            bp->m_function == func->fullName()->toCppString()) ||
           (func->name() != nullptr &&
              bp->m_function == func->name()->toCppString())) {

        // Found a matching function!
        phpAddBreakPointFuncEntry(func);
        bpMgr->onFuncBreakpointResolved(*const_cast<Breakpoint*>(bp), func);

        return true;
      }
    }
  }

  return false;
}

bool Debugger::tryResolveBreakpointInUnit(const DebuggerRequestInfo* /*ri*/, int bpId,
                                          const Breakpoint* bp,
                                          const std::string& unitFilePath,
                                          const HPHP::Unit* compilationUnit) {

  if (bp->m_type != BreakpointType::Source ||
      !BreakpointManager::bpMatchesPath(
        bp,
        std::filesystem::path(unitFilePath))) {

    return false;
  }

  std::pair<int,int> lines =
    calibrateBreakpointLineInUnit(compilationUnit, bp->m_line);

  if (lines.first > 0 && lines.second != lines.first) {
    lines = calibrateBreakpointLineInUnit(compilationUnit, lines.first);
  }

  if (lines.first < 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "NOT installing bp ID %d in file %s. No source locations matching "
        " line %d were found.",
      bpId,
      unitFilePath.c_str(),
      bp->m_line
    );
    return false;
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Installing bp ID %d at line %d (original line was %d) of file %s.",
    bpId,
    lines.first,
    bp->m_line,
    unitFilePath.c_str()
  );

  if (!phpAddBreakPointLine(compilationUnit, lines.first)) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Installing %d at line %d of file %s FAILED in phpAddBreakPointLine!",
      bpId,
      lines.first,
      unitFilePath.c_str()
    );
    return false;
  }

  // Warn the user if the breakpoint is going into a unit that has intercepted
  // functions or a memoized function. We can't be certain this breakpoint is
  // reachable in code anymore.
  BreakpointManager* bpMgr = m_session->getBreakpointManager();

  std::string functionName = "";
  const HPHP::Func* function = nullptr;
  if (m_debuggerOptions.notifyOnBpCalibration &&
      !bpMgr->warningSentForBp(bpId)) {

    compilationUnit->forEachFunc([&](const Func* func) {
      if (func != nullptr &&
          func->name() != nullptr &&
          func->line1() <= lines.first &&
          func->line2() >= lines.second) {
        std::string clsName =
          func->preClass() != nullptr && func->preClass()->name() != nullptr
            ? std::string(func->preClass()->name()->data()) + "::"
            : "";
        functionName = clsName;
        functionName += func->name()->data();
        function = func;
        return true;
      }
      return false;
    });

    if (function != nullptr &&
          (function->isMemoizeWrapper() || function->isMemoizeImpl())) {
        // This breakpoint looks like it's going into either a memoized
        // function, or the wrapper for a memoized function. That means after
        // the first time this routine executes, it might not execute again,
        // even if the code is invoked again. Warn the user because this can
        // cause confusion.
        bpMgr->sendMemoizeWarning(bpId);
    }
  }

  bpMgr->onBreakpointResolved(
    bpId,
    lines.first,
    lines.second,
    0,
    0,
    unitFilePath,
    functionName
  );

  return true;
}

std::pair<int, int> Debugger::calibrateBreakpointLineInUnit(
  const Unit* unit,
  int bpLine
) {
  // Attempt to find a matching source location entry in the compilation unit
  // that corresponds to the breakpoint's requested line number. Note that the
  // line provided by the client could be in the middle of a multi-line
  // statement, or could be on a line that contains multiple statements. It
  // could also be in whitespace, or past the end of the file.
  std::pair<int, int> bestLocation = {-1, -1};
  struct sourceLocCompare {
    bool operator()(const SourceLoc& a, const SourceLoc& b) const {
      if (a.line0 == b.line0) {
        return a.line1 < b.line1;
      }

      return a.line0 < b.line0;
    }
  };

  std::set<SourceLoc, sourceLocCompare> candidateLocations;
  unit->forEachFunc([&](const Func* func) {
    const auto& table = func->getLocTable();
    for (auto const& tableEntry : table) {
      const SourceLoc& sourceLocation = tableEntry.val();

      // If this source location is invalid, ends before the line we are
      // looking for, or starts before the line we are looking for there
      // is no match. Exception: if it is a multi-line statement that begins
      // before the target line and ends ON the target line, that is a match
      // to allow, for example, setting a breakpoint on the line containing
      // a closing paren for a multi-line function call.
      if (!sourceLocation.valid() ||
          sourceLocation.line1 < bpLine ||
          (sourceLocation.line0 < bpLine && sourceLocation.line1 != bpLine)) {

        continue;
      }

      candidateLocations.insert(sourceLocation);
    }
    return false;
  });

  if (candidateLocations.size() > 0) {
    const auto it = candidateLocations.begin();
    const SourceLoc& location = *it;
    bestLocation.first = location.line0;
    bestLocation.second = location.line1;
  }

  return bestLocation;
}

void Debugger::onFunctionDefined(
  DebuggerRequestInfo* ri,
  const Func* func,
  const std::string& funcName
) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  BreakpointManager* bpMgr = m_session->getBreakpointManager();
  auto& unresolvedBps = ri->m_breakpointInfo->m_unresolvedBreakpoints;

  for (auto it = unresolvedBps.begin(); it != unresolvedBps.end(); ) {
    const int bpId = *it;
    const Breakpoint* bp = bpMgr->getBreakpointById(bpId);
    if (bp == nullptr) {
        // Breakpoint has been removed by the client.
        it = unresolvedBps.erase(it);
        continue;
    }

    if (bp->m_type == BreakpointType::Function && funcName == bp->m_function) {
        // Found a matching function!
        phpAddBreakPointFuncEntry(func);
        bpMgr->onFuncBreakpointResolved(*const_cast<Breakpoint*>(bp), func);

        // Breakpoint is no longer unresolved!
        it = unresolvedBps.erase(it);
    } else {
      // Still no match, move on to the next unresolved function breakpoint.
      it++;
    }
  }

  updateUnresolvedBpFlag(ri);
}

void Debugger::onCompilationUnitLoaded(
  DebuggerRequestInfo* ri,
  const HPHP::Unit* compilationUnit
) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  BreakpointManager* bpMgr = m_session->getBreakpointManager();
  const auto filePath = getFilePathForUnit(compilationUnit);

  if (ri->m_breakpointInfo->m_loadedUnits[filePath] != nullptr &&
      ri->m_breakpointInfo->m_loadedUnits[filePath] != compilationUnit) {

    const auto& bps = bpMgr->getBreakpointIdsForPath(filePath);

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Compilation unit for %s changed/reloaded in request %d. ",
      filePath.c_str(),
      getCurrentThreadId()
    );

    // The unit has been re-loaded from disk since the last time we saw it.
    // We must re-place any breakpoints in this file into the new unit.
    for (const auto bpId : bps) {
      auto it = ri->m_breakpointInfo->m_unresolvedBreakpoints.find(bpId);
      if (it == ri->m_breakpointInfo->m_unresolvedBreakpoints.end()) {
        ri->m_breakpointInfo->m_unresolvedBreakpoints.emplace(bpId);
      }
    }
  }

  ri->m_breakpointInfo->m_loadedUnits[filePath] = compilationUnit;

  // See if any unresolved breakpoints for this request can be placed in the
  // compilation unit that just loaded.
  auto& unresolvedBps = ri->m_breakpointInfo->m_unresolvedBreakpoints;

  for (auto it = unresolvedBps.begin(); it != unresolvedBps.end();) {
    const int bpId = *it;
    const Breakpoint* bp = bpMgr->getBreakpointById(bpId);
    if (bp == nullptr ||
        tryResolveBreakpointInUnit(ri, bpId, bp, filePath, compilationUnit)) {

      if (bp == nullptr || !bp->isRelativeBp()) {
        // If this breakpoint no longer exists (it was removed by the client),
        // or it was successfully installed, then it is no longer unresolved.
        it = unresolvedBps.erase(it);
        continue;
      }
    }

    it++;
  }

  updateUnresolvedBpFlag(ri);
}

void Debugger::onFuncIntercepted(std::string funcName) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  BreakpointManager* bpMgr = m_session->getBreakpointManager();
  bpMgr->onFuncIntercepted(getCurrentThreadId(), funcName);
}

void Debugger::onFuncBreakpointHit(
  DebuggerRequestInfo* ri,
  const HPHP::Func* func
) {
  Lock lock(m_lock);

  if (func != nullptr) {
    onBreakpointHit(ri, func->unit(), func, func->line1());
  }
}

void Debugger::onLineBreakpointHit(
  DebuggerRequestInfo* ri,
  const HPHP::Unit* compilationUnit,
  int line
) {
  Lock lock(m_lock);
  onBreakpointHit(ri, compilationUnit, nullptr, line);
}

void Debugger::onBreakpointHit(
  DebuggerRequestInfo* ri,
  const HPHP::Unit* compilationUnit,
  const HPHP::Func* func,
  int line
) {
  std::string stopReason;
  const std::string filePath = getFilePathForUnit(compilationUnit);

  if (prepareToPauseTarget(ri) != PrepareToPauseResult::ReadyToPause) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "onBreakpointHit: Not pausing target, the client disconnected."
    );
    return;
  }

  BreakpointManager* bpMgr = m_session->getBreakpointManager();

  const auto removeBreakpoint =
    [&](BreakpointType type) {
      if (type == BreakpointType::Source) {
        phpRemoveBreakPointLine(compilationUnit, line);
      } else {
        assertx(type == BreakpointType::Function);
        assertx(func != nullptr);
        phpRemoveBreakPointFuncEntry(func);
      }
    };

  const auto bps = bpMgr->getAllBreakpointIds();
  for (auto it = bps.begin(); it != bps.end(); it++) {
    const int bpId = *it;
    Breakpoint* bp = bpMgr->getBreakpointById(bpId);
    if (bp == nullptr) {
      continue;
    }

    const auto resolvedLocation = bpMgr->bpResolvedInfoForFile(bp, filePath);
    bool lineInRange = line >= resolvedLocation.m_startLine &&
        line <= resolvedLocation.m_endLine;

    if (resolvedLocation.m_path == filePath && lineInRange) {
      if (bpMgr->isBreakConditionSatisified(ri, bp)) {
        stopReason = getStopReasonForBp(
          bp,
          filePath,
          line
        );

        // Breakpoint hit!
        assertx(bp);
        auto firstBpHit = ri->m_firstBpHit;
        ri->m_firstBpHit = true;
        if (Cfg::Debugger::LogBreakpointHitTime && StructuredLog::enabled() &&
            !firstBpHit && bp->m_type == BreakpointType::Source &&
            !isDummyRequest() && g_context->getTransport()) {
          StructuredLogEntry ent;
          ent.setStr("filename", bp->m_filePath.filename().c_str());
          ent.setInt("line_number", bp->m_line);
		    	struct timespec now;
          Timer::GetMonotonicTime(now);
          auto t =
            gettime_diff_us(g_context->getTransport()->getWallTime(), now);
          ent.setInt("time_to_hit", t);
          StructuredLog::log("hhvm_time_till_bp", ent);
        }

        pauseTarget(ri, false);
        bpMgr->onBreakpointHit(bpId);

        processCommandQueue(
          getCurrentThreadId(),
          ri,
          "breakpoint",
          stopReason.c_str(),
          true,
          bpId
        );

        return;
      } else {
        VSDebugLogger::Log(
          VSDebugLogger::LogLevelInfo,
          "onBreakpointHit: Not pausing target, breakpoint found but the bp "
            " condition (%s) is not satisfied.",
          bp->getCondition().c_str()
        );
      }
    }
  }

  if (ri->m_runToLocationInfo.path == filePath &&
      line == ri->m_runToLocationInfo.line) {

    // Hit our run to location destination!
    stopReason = "Run to location";
    ri->m_runToLocationInfo.path = "";
    ri->m_runToLocationInfo.line = -1;

    // phpRemoveBreakpointLine doesn't refcount or anything, so it's only
    // safe to remove this if there is no real bp at the line.
    bool realBp = false;
    BreakpointManager* bpMgr = m_session->getBreakpointManager();
    const auto bpIds = bpMgr->getBreakpointIdsForPath(filePath);
    for (auto it = bpIds.begin(); it != bpIds.end(); it++) {
      Breakpoint* bp = bpMgr->getBreakpointById(*it);
      if (bp != nullptr && bp->m_line == line) {
        realBp = true;
        break;
      }
    }

    if (!realBp) {
      removeBreakpoint(BreakpointType::Source);
    }

    pauseTarget(ri, false);
    processCommandQueue(
      getCurrentThreadId(),
      ri,
      "step",
      stopReason.c_str(),
      true,
      -1
    );
  }
}

void Debugger::onExceptionBreakpointHit(
  DebuggerRequestInfo* ri,
  const std::string& exceptionName,
  const std::string& exceptionMsg
) {
  std::string stopReason("Exception (");
  stopReason += exceptionName;
  stopReason += ") thrown";

  std::string userMsg = "Request ";
  userMsg += std::to_string(getCurrentThreadId());
  userMsg += ": ";
  userMsg += stopReason + ": ";
  userMsg += exceptionMsg;

  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  BreakpointManager* bpMgr = m_session->getBreakpointManager();
  ExceptionBreakMode breakMode = bpMgr->getExceptionBreakMode();

  if (breakMode == BreakNone) {
    return;
  }

  sendUserMessage(userMsg.c_str(), DebugTransport::OutputLevelWarning);

  if (breakMode == BreakUnhandled || breakMode == BreakUserUnhandled) {
    // The PHP VM doesn't give us any way to distinguish between handled
    // and unhandled exceptions. A message was already printed indicating
    // the exception, but we won't actually break in.
    return;
  }

  assertx(breakMode == BreakAll);

  if (prepareToPauseTarget(ri) != PrepareToPauseResult::ReadyToPause) {
    return;
  }

  pauseTarget(ri, false);
  processCommandQueue(
    getCurrentThreadId(),
    ri,
    "exception",
    stopReason.c_str(),
    true,
    -1
  );
}

bool Debugger::onHardBreak() {
  static constexpr char* stopReason = "hphp_debug_break()";

  Lock lock(m_lock);
  VMRegAnchor regAnchor;
  DebuggerRequestInfo* ri = getRequestInfo();

  if (ri == nullptr) {
    return false;
  }

  if (!Debugger::isStepInProgress(ri)) {
    enterDebuggerIfPaused(ri);
  }

  if (g_context->m_dbgNoBreak || ri->m_flags.doNotBreak) {
    return false;
  }

  if (!clientConnected() ||
      prepareToPauseTarget(ri) != PrepareToPauseResult::ReadyToPause) {

    return false;
  }

  pauseTarget(ri, false);
  processCommandQueue(
    getCurrentThreadId(),
    ri,
    "breakpoint",
    stopReason,
    true,
    -1
  );

  // We actually need to step out here, because as far as the PC filter is
  // concerned, hphp_debug_break() was a function call that increased the
  // stack depth.
  phpDebuggerStepOut();

  return true;
}

bool Debugger::isStepInProgress(DebuggerRequestInfo* requestInfo) {
  return requestInfo->m_stepReason != nullptr ||
         (!requestInfo->m_runToLocationInfo.path.empty() &&
          requestInfo->m_runToLocationInfo.line > 0) ||
          requestInfo->m_evaluateCommandDepth > 0;
}

void Debugger::clearStepOperation(DebuggerRequestInfo* requestInfo) {
  if (isStepInProgress(requestInfo)) {
    phpDebuggerContinue();
    requestInfo->m_stepReason = nullptr;
    requestInfo->m_runToLocationInfo.path.clear();
    requestInfo->m_nextFilterInfo.clear();
  }
}

void Debugger::updateUnresolvedBpFlag(DebuggerRequestInfo* ri) {
  ri->m_flags.unresolvedBps =
    !ri->m_breakpointInfo->m_unresolvedBreakpoints.empty() ||
    !ri->m_breakpointInfo->m_pendingBreakpoints.empty() ||
    ri->m_breakpointInfo->m_hasExceptionBreakpoint;
  std::atomic_thread_fence(std::memory_order_release);
}

void Debugger::onAsyncBreak() {
  Lock lock(m_lock);

  if (m_state == ProgramState::Paused || m_state == ProgramState::AsyncPaused) {
    // Already paused.
    return;
  }

  if (prepareToPauseTarget(nullptr) != PrepareToPauseResult::ReadyToPause) {
    return;
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Debugger paused due to async-break request from client."
  );

  pauseTarget(nullptr, true);

  if (m_debuggerOptions.showDummyOnAsyncPause) {
    // Show the dummy request as stopped.
    constexpr char* reason = "Async-break";
    sendStoppedEvent(reason, reason, 0, false, -1);
  }
}

void Debugger::onError(DebuggerRequestInfo* requestInfo,
                       const ExtendedException& /*extendedException*/,
                       int errnum, const std::string& message) {
  const char* phpError;
  switch (static_cast<ErrorMode>(errnum)) {
    case ErrorMode::ERROR:
    case ErrorMode::CORE_ERROR:
    case ErrorMode::COMPILE_ERROR:
    case ErrorMode::USER_ERROR:
      phpError = "Fatal error";
      break;
    case ErrorMode::RECOVERABLE_ERROR:
      phpError = "Catchable fatal error";
      break;
    case ErrorMode::WARNING:
    case ErrorMode::CORE_WARNING:
    case ErrorMode::COMPILE_WARNING:
    case ErrorMode::USER_WARNING:
      phpError = "Warning";
      break;
    case ErrorMode::PARSE:
      phpError = "Parse error";
      break;
    case ErrorMode::NOTICE:
    case ErrorMode::USER_NOTICE:
      phpError = "Notice";
      break;
    case ErrorMode::STRICT:
      phpError = "Strict standards";
      break;
    case ErrorMode::PHP_DEPRECATED:
    case ErrorMode::USER_DEPRECATED:
      phpError = "Deprecated";
      break;
    default:
      phpError = "Unknown error";
      break;
  }

  onExceptionBreakpointHit(requestInfo, phpError, message);
}

std::string Debugger::getFilePathForUnit(const HPHP::Unit* compilationUnit) {
  const auto path =
    HPHP::String(const_cast<StringData*>(compilationUnit->filepath()));
  const auto translatedPath = File::TranslatePath(path).toCppString();
  return realpathLibc(translatedPath.c_str());
}

std::string Debugger::getStopReasonForBp(
  const Breakpoint* bp,
  const std::string& path,
  const int line
) {
  std::string description("Breakpoint " + std::to_string(bp->m_id));
  if (!path.empty()) {
    auto const name = std::filesystem::path(path).filename().string();
    description += " (";
    description += name;
    description += ":";
    description += std::to_string(line);
    description += ")";
  }

  if (bp->m_type == BreakpointType::Function) {
    description += " - " + bp->m_functionFullName + "()";
  }

  return description;
}

void Debugger::interruptAllThreads() {
  executeForEachAttachedRequest(
    [&](RequestInfo* ti, DebuggerRequestInfo* ri) {
      assertx(ti != nullptr);
      ti->m_reqInjectionData.setDebuggerIntr(true);
      ti->m_reqInjectionData.setFlag(DebuggerSignalFlag);
    },
    false /* includeDummyRequest */
  );
}

void DebuggerStdoutHook::operator()(const char* str, int len) {
  fflush(stdout);
  write(fileno(stdout), str, len);

  // Quickly no-op if there's no client.
  if (!m_debugger->clientConnected()) {
    return;
  }

  std::string output = std::string(str, len);
  m_debugger->sendUserMessage(
    output.c_str(),
    DebugTransport::OutputLevelStdout);
}

void DebuggerStderrHook::
operator()(const char*, const char* msg, const char* /*ending*/
) {
  // Quickly no-op if there's no client.
  if (!m_debugger->clientConnected()) {
    return;
  }

  m_debugger->sendUserMessage(msg, DebugTransport::OutputLevelStderr);
}

SilentEvaluationContext::SilentEvaluationContext(
  Debugger* debugger,
  DebuggerRequestInfo* ri,
  bool suppressOutput /* = true */
) : m_ri(ri), m_suppressOutput(suppressOutput) {
  // Disable hitting breaks of any kind due to this eval.
  m_ri->m_flags.doNotBreak = true;
  std::atomic_thread_fence(std::memory_order_release);

  g_context->m_dbgNoBreak = true;

  RequestInjectionData& rid = RID();

  if (m_suppressOutput) {
    // Disable raising of PHP errors during this eval.
    m_errorLevel = rid.getErrorReportingLevel();
    rid.setErrorReportingLevel(0);

    // Disable all sorts of output during this eval.
    m_oldHook = debugger->getStdoutHook();
    m_savedOutputBuffer = g_context->swapOutputBuffer(&m_sb);
    g_context->removeStdoutHook(m_oldHook);
    g_context->addStdoutHook(&m_noOpHook);
  }

  // Set aside the flow filters to disable all stepping and bp filtering.
  m_savedFlowFilter.swap(rid.m_flowFilter);
  m_savedBpFilter.swap(rid.m_breakPointFilter);
}

SilentEvaluationContext::~SilentEvaluationContext() {
  m_ri->m_flags.doNotBreak = false;
  std::atomic_thread_fence(std::memory_order_release);

  g_context->m_dbgNoBreak = false;

  RequestInjectionData& rid = RID();

  if (m_suppressOutput) {
    rid.setErrorReportingLevel(m_errorLevel);
    g_context->swapOutputBuffer(m_savedOutputBuffer);
    g_context->removeStdoutHook(&m_noOpHook);
    g_context->addStdoutHook(m_oldHook);
  }

  m_savedFlowFilter.swap(rid.m_flowFilter);
  m_savedBpFilter.swap(rid.m_breakPointFilter);
  m_sb.clear();
}

DebuggerNoBreakContext::DebuggerNoBreakContext(Debugger* debugger) {
  m_requestInfo = debugger->getRequestInfo();
  m_prevDoNotBreak = m_requestInfo->m_flags.doNotBreak;
  m_requestInfo->m_flags.doNotBreak = true;
  m_prevDbgNoBreak = g_context->m_dbgNoBreak;
  g_context->m_dbgNoBreak = true;
}

DebuggerNoBreakContext::~DebuggerNoBreakContext() {
  m_requestInfo->m_flags.doNotBreak = m_prevDoNotBreak;
  g_context->m_dbgNoBreak = m_prevDbgNoBreak;
}
}
}
