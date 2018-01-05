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
#include <condition_variable>
#include <mutex>

#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/runtime/ext/vsdebug/transport.h"
#include "hphp/runtime/ext/vsdebug/break_mode.h"
#include "hphp/runtime/ext/vsdebug/breakpoint.h"
#include "hphp/runtime/ext/vsdebug/session.h"
#include "hphp/runtime/ext/vsdebug/command_queue.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/hook.h"
#include "hphp/runtime/ext/vsdebug/client_preferences.h"
#include "hphp/runtime/ext/vsdebug/server_object.h"
#include "hphp/util/process.h"

namespace HPHP {
namespace VSDEBUG {

struct DebugTransport;
struct DebuggerSession;
struct Breakpoint;
struct Debugger;

enum ProgramState {
  LoaderBreakpoint,
  Paused,
  Running
};

// Structure to represent breakpoint state for a particular request.
struct RequestBreakpointInfo {
  // Breakpoints that have been set in this request thread but not resolved
  // to a loaded compilation unit.
  std::unordered_set<int> m_unresolvedBreakpoints;

  // Breakpoints set by the client that have not been synced to this request
  // thread yet.
  std::unordered_set<int> m_pendingBreakpoints;

  // Map of loaded compilation units for this request by normalized file path.
  std::map<std::string, const HPHP::Unit*> m_loadedUnits;
};

// Structure to represent the state of a single request.
struct RequestInfo {
  struct {
    bool hookAttached;
    bool memoryLimitRemoved;
    bool compilationUnitsMapped;
    bool doNotBreak;
  } m_flags;
  const char* m_stepReason;
  CommandQueue m_commandQueue;
  RequestBreakpointInfo* m_breakpointInfo;
  std::unordered_map<unsigned int, ServerObject*> m_serverObjects;

  struct {
    std::string path;
    int line;
  } m_runToLocationInfo;

  // Number of evaluation frames on this request's stack right now.
  int m_evaluateCommandDepth;

  // Number of recursive calls into processCommandsQueue for this request
  // right now.
  int m_pauseRecurseCount;

  // Number of times this request has entered the command queue since starting.
  unsigned int m_totalPauseCount;
};

// An exception to be thrown when a message from the client cannot be processed.
struct DebuggerCommandException : Exception {

  DebuggerCommandException(const char* message) :
    m_message(message) {
  }

  const char* what() const noexcept override {
    return m_message;
  }

  EXCEPTION_COMMON_IMPL(DebuggerCommandException);

private:
  const char* m_message;
};

// Hooks for intercepting writes to STDOUT and STDERR in attach to server mode.
struct DebuggerStdoutHook final : ExecutionContext::StdoutHook {
  explicit DebuggerStdoutHook(Debugger* debugger)
    : m_debugger(debugger) {
  }

  void operator()(const char* str, int len) override;

private:
  Debugger* m_debugger;
};

struct DebuggerStderrHook final : LoggerHook {
  explicit DebuggerStderrHook(Debugger* debugger)
    : m_debugger(debugger) {
  }

  void operator()(const char*, const char* msg, const char* ending) override;

private:
  Debugger* m_debugger;
};

struct Debugger final {
  Debugger();
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

  // Sends a message to the front-end to be displayed to the user in the
  // debugger console.
  void sendUserMessage(
    const char* message,
    const char* level = DebugTransport::OutputLevelLog
  );

  // Sends a VS Code debug event message to the debugger client.
  void sendEventMessage(
    folly::dynamic& event,
    const char* eventType
  );

  // Handle requests.
  void requestInit();
  void requestShutdown();

  // Returns a pointer to the RequestInfo for the current thread.
  RequestInfo* getRequestInfo(int threadId = -1);

  // Allocates a new request info object.
  static RequestInfo* createRequestInfo();

  // Cleans up and frees the specified request info object and shuts down its
  // command queue, unblocking the waiting request thread (if any).
  static void cleanupRequestInfo(ThreadInfo* ti, RequestInfo* ri);

  // Puts the current thread into the command queue for the specified request
  // info. This routine will block until the debugger is resumed by the client,
  // the client disconnects, or the extension is shut down.
  void processCommandQueue(
    int threadId,
    RequestInfo* requestInfo,
    const char* reason = "execution paused"
  );

  // Called by the debugger transport when a new message is received from
  // a connected debugger client.
  void onClientMessage(folly::dynamic& message);

  // Enters the debugger if the program is paused.
  void enterDebuggerIfPaused(RequestInfo* requestInfo);

  // Executes a command from the debugger client while holding the debugger
  // lock.
  bool executeClientCommand(
    VSCommand* command,
    std::function<bool(DebuggerSession* session,
                       folly::dynamic& responseMsg)> callback
  );

  // Blocks until a client is connected. Returns immediately if a client
  // is already connected.
  void waitForClientConnection();

  // Stores debugger client preferences.
  void setClientPreferences(ClientPreferences& preferences);
  ClientPreferences getClientPreferences();

  // Starts the session's dummy request.
  void startDummyRequest(const std::string& startupDoc);

  // Sets the client initialized flag.
  void setClientInitialized();

  // Returns the synthetic request ID for the current request thread.
  int getCurrentThreadId();

  // Sends a stopped event to the client.
  void sendStoppedEvent(
    const char* reason,
    int64_t threadId
  );

  // Sends a thread continued event to the client.
  void sendContinuedEvent(int64_t threadId);

  // Sets the dummy thread ID.
  void setDummyThreadId(int64_t threadId);

  // Called when a new breakpoint is added to sync it to all requests.
  void onBreakpointAdded(int bpId);

  // Attempts to resolve and install breakpoints for the current request thread.
  // Will either install the breakpoint or add it to the request's unresolved
  // list.
  void tryInstallBreakpoints(RequestInfo* ri);

  // Called when a request loads a new compilation unit.
  void onCompilationUnitLoaded(
    RequestInfo* ri,
    const HPHP::Unit* compilationUnit
  );

  // Called when a request thinks it has hit a breakpoint.
  void onLineBreakpointHit(
    RequestInfo* ri,
    const HPHP::Unit* compilationUnit,
    int line
  );

  // Called when a request hits an exception.
  void onExceptionBreakpointHit(
    RequestInfo* ri,
    const std::string& exceptionName,
    const std::string& exceptionMsg
  );

  void onError(
    RequestInfo* requestInfo,
    const ExtendedException& extendedException,
    int errnum,
    const std::string& message
  );

  // Called when the client requests an async-break of all threads.
  void onAsyncBreak();

  // Checks if we are stepping for a particular request.
  static bool isStepInProgress(RequestInfo* requestInfo) {
    return requestInfo->m_stepReason != nullptr ||
           (!requestInfo->m_runToLocationInfo.path.empty() &&
            requestInfo->m_runToLocationInfo.line > 0) ||
            requestInfo->m_evaluateCommandDepth > 0;
  }

  // Clears the state filters for a step operation on the specified request
  // thread, if any step is currently in progress.
  static void clearStepOperation(RequestInfo* requestInfo) {
    if (isStepInProgress(requestInfo)) {
      phpDebuggerContinue();
      requestInfo->m_stepReason = nullptr;
      requestInfo->m_runToLocationInfo.path.clear();
    }
  }

  // Adjusts a breakpoints source line based on the source mapping table in
  // the specified complilation unit in which the breakpoint is being installed.
  std::pair<int, int> calibrateBreakpointLineInUnit(
    const Unit* unit,
    int bpLine
  );

  // Provides a mechanism for a command to execute code without the debugger
  // lock held. This routine will drop the lock, execute the provided callback
  // and re-acquire the lock before returning.
  void executeWithoutLock(std::function<void()> callback);

  enum ThreadEventType {
    ThreadStarted,
    ThreadExited
  };

  static constexpr int kDummyTheadId = 0;

  // Sends a thread event message to a debugger client.
  void sendThreadEventMessage(int64_t threadId, ThreadEventType eventType);

  // Returns true if the current thread is the dummy request thread, false
  // otherwise.
  bool isDummyRequest() {
    return (int64_t)Process::GetThreadId() == m_dummyThreadId;
  }

private:

  // Cleans up server objects for a request.
  void cleanupServerObjectsForRequest(RequestInfo* ri);

  // Attaches the debugger to the specified request thread and installs the
  // debugger hook.
  RequestInfo* attachToRequest(ThreadInfo* ti);

  // Checks if the specified command requires the target to be broken in before
  // dispatching to a request queue and throws an exception if the condition is
  // violated.
  void enforceRequiresBreak(VSCommand* command) {
    if (command->requiresBreak() && m_state == ProgramState::Running) {
      throw DebuggerCommandException(
        "The debugger issued a command that is only valid while the program is "
        "broken in, but the target is currently running."
      );
    }
  }

  // Reports faiure to process a message from the debugger client to the
  // front-end.
  void reportClientMessageError(
    folly::dynamic& clientMsg,
    const char* errorMessage
  );

  // Sends a response message to a debugger client in response to a debugger
  // client request.
  void sendCommandResponse(VSCommand* command, folly::dynamic& responseMsg);

  // Attempts to send the protocol terminated event if there is still a
  // connected client.
  void trySendTerminatedEvent();

  // Resumes execution of all request threads.
  void resumeTarget();

  // Halts execution of all request threads.
  void pauseTarget(RequestInfo* ri, const char* stopReason);

  // Attempts to resolve and install a breakpoint for the current request.
  // Returns true if the bp was resolved, false if it is unresolved and pending.
  bool tryResolveBreakpoint(
    RequestInfo* ri,
    const int bpId,
    const Breakpoint* bp
  );

  // Attempts to resolve the breakpoint in the specified compilation unit.
  // Returns true on success, false otherwise.
  bool tryResolveBreakpointInUnit(
    const RequestInfo* ri,
    int bpId,
    const Breakpoint* bp,
    const std::string& unitFilePath,
    const HPHP::Unit* compilationUnit
  );

  // Notifies all threads that they need to switch to interpreted mode so we
  // can interrupt them.
  void interruptAllThreads();

  // Executes the specified lambda function for each request the debugger
  // is currently attached to.
  void executeForEachAttachedRequest(
    std::function<void(
      // Supplies the ThreadInfo for the request thread.
      // NOTE: This parameter will be nullptr for the dummy request.
      ThreadInfo* ti,
      // Supplies the request info for the request thread. This will
      // never be nullptr.
      RequestInfo* ri
    )> callback,
    bool includeDummyRequest
  );

  // Blocks until it is safe to pause the target. If any other threads are
  // in the process of pausing or resuming target requests, waits until those
  // operations are complete, and returns with the current thread holding
  // m_lock, the program state == Running and no request threads paused
  // in their command queues.
  //
  // Preparing to pause separately from pauseTarget() allows us to do things
  // like ensure a breakpoint wasn't removed by the client during the previous
  // pause before triggering the breakpoint on a new request thread.
  enum PrepareToPauseResult {
    ReadyToPause,
    ErrorNoClient
  };
  PrepareToPauseResult prepareToPauseTarget(RequestInfo* requestInfo);

  // Normalizes the file path for a compilation unit.
  static std::string getFilePathForUnit(const HPHP::Unit* compilationUnit);

  // Returns a stop reason string for a breakpoint.
  static std::string getStopReasonForBp(
    const int id,
    const std::string& path,
    const int line
  );

  // Returns the request info for the dummy, if one exists or nullptr
  // if there is no client connected.
  RequestInfo* getDummyRequestInfo();

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

  // This flag indicates if there is a connected debugger client, and the
  // client is fully initialized. If true, it is okay to send the client
  // thread events.
  bool m_clientInitialized {false};

  // State of the program.
  ProgramState m_state {ProgramState::LoaderBreakpoint};

  // Information about all the requests that the debugger is aware of.
  std::unordered_map<ThreadInfo*, RequestInfo*> m_requests;

  // Map of synthetic thread ID to ThreadInfo*;
  std::unordered_map<int, ThreadInfo*> m_requestIdMap;

  // Map of ThreadInfo* to synthetic thread ID
  std::unordered_map<ThreadInfo*, int> m_requestInfoMap;

  // Next synthetic request ID to send to client as thread ID.
  int m_nextThreadId {1};

  // Keeps track of the number of requests that are currently blocked inside
  // the debugger extension due to being paused for any reason (breakpoint,
  // exception, loader break, etc...)
  uint64_t m_pausedRequestCount {0};

  // Keeps track of the total number of requests attached to by this extension
  // since the server was started.
  std::atomic<uint64_t> m_totalRequestCount {0};

  // Tracks the thread ID of the dummy request.
  int64_t m_dummyThreadId {-1};

  // Support for waiting for a client connection to arrive.
  std::mutex m_connectionNotifyLock;
  std::condition_variable m_connectionNotifyCondition;

  // Support for waiting for a resume to complete before executing another
  // break of the target and a pause to complete before another thread can
  // also issue a pause
  std::mutex m_resumeMutex;
  std::condition_variable m_resumeCondition;

  // Hooks for stdout and stderr redirection.
  DebuggerStdoutHook m_stdoutHook {DebuggerStdoutHook(this)};
  DebuggerStderrHook m_stderrHook {DebuggerStderrHook(this)};

  static constexpr char* InternalErrorMsg =
    "An internal error occurred while processing a debugger command.";
};

}
}

#endif // incl_HPHP_VSDEBUG_DEBUGGER_H_
