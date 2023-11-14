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

#include <atomic>
#include <unordered_map>
#include <condition_variable>
#include <mutex>

#include "hphp/runtime/base/request-info.h"
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
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/process.h"

namespace HPHP {

class Watchman;

namespace VSDEBUG {

#define VSDEBUG_NAME "vsdebug"
#define VSDEBUG_VERSION "1.2"


struct DebugTransport;
struct DebuggerSession;
struct Breakpoint;
struct Debugger;
struct DebuggerRequestInfo;

enum ProgramState {
  LoaderBreakpoint,
  Paused,
  AsyncPaused,
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

  bool m_hasExceptionBreakpoint {false};

  // Map of loaded compilation units for this request by normalized file path.
  std::map<std::string, const HPHP::Unit*> m_loadedUnits;

  // Function breakpoints are rare. If there are no function breakpoint set on
  // a request, no breakpoints need to be resolved on class/function loads.
  bool m_hasFuncBp {false};

  hphp_fast_string_set m_filenamesWithBp;
};

struct StepNextFilterInfo {
  const Unit* stepStartUnit {nullptr};
  int skipLine0 {0};
  int skipLine1 {0};
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

// Custom options that clients can pass in launch/attach messages
// to control optional debugger behavior.
struct DebuggerOptions {
  // Show the dummy thread in the threads list when the debugger is
  // responding to a stop command.
  bool showDummyOnAsyncPause;

  // Warn if the client sets a breakpoint in a function that is intercepted.
  bool warnOnInterceptedFunctions;

  // Tell the user if breakpoint calibration moves their bp.
  bool notifyOnBpCalibration;

  // Don't try to unique variable references by address.
  bool disableUniqueVarRef;

  // Don't try to run the vsdebug_post_dummy_eval helper after each evaluate
  // command in the dummy request.
  bool disablePostDummyEvalHelper;

  // The maximum length of a string to return, for perf
  int maxReturnedStringLength;

  // Don't redirect stdout to the debugger console
  bool disableStdoutRedirection;

  // Disable JIT when debugger is attached
  bool disableJit;

  // Warn if any files loaded by the REPL request change
  bool warnOnFileChange;
};

struct ClientInfo {
  std::string clientUser;
  pid_t clientPid;
  uid_t clientUid;
};

struct Debugger final {
  Debugger();
  Debugger(const Debugger&) = delete;
  Debugger& operator=(const Debugger&) = delete;
  Debugger (Debugger && ) = delete;
  ~Debugger() {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Debugger destructor called"
    );
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
  void setClientConnected(
    bool connected,
    bool synchronous = false,
    ClientInfo* clientInfo = nullptr
  );

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
    const char* eventType,
    bool sendImmediately = false
  );

  // Handle requests.
  void requestInit();
  void requestShutdown();

  // Returns a pointer to the DebuggerRequestInfo for the current thread.
  DebuggerRequestInfo* getRequestInfo(request_id_t threadId = (request_id_t)-1);

  // Allocates a new request info object.
  static DebuggerRequestInfo* createRequestInfo();

  // Cleans up and frees the specified request info object and shuts down its
  // command queue, unblocking the waiting request thread (if any).
  static void cleanupRequestInfo(RequestInfo* ti, DebuggerRequestInfo* ri);

  // Puts the current thread into the command queue for the specified request
  // info. This routine will block until the debugger is resumed by the client,
  // the client disconnects, or the extension is shut down.
  void processCommandQueue(
    request_id_t threadId,
    DebuggerRequestInfo* requestInfo,
    const char* reason,
    const char* displayReason,
    bool focusedThread,
    int bpId
  );

  // Called by the debugger transport when a new message is received from
  // a connected debugger client.
  void onClientMessage(folly::dynamic& message);

  void logClientCommand(VSCommand* command);

  // Enters the debugger if the program is paused.
  void enterDebuggerIfPaused(DebuggerRequestInfo* requestInfo);

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
  void startDummyRequest(
    const std::string& startupDoc,
    const std::string& sandboxUser,
    const std::string& sandboxName,
    const std::string& debuggerSessionAuth,
    bool displayStartupMsg
  );

  // Sets the client initialized flag.
  void setClientInitialized();

  // Returns the synthetic request ID for the current request thread.
  request_id_t getCurrentThreadId();

  // Returns the client-provided authentication string to use for
  // the dummy request, if any.  Returns empty string if called from
  // any thread other than the dummy request thread or if there is no
  // debugger client attached.
  std::string getDebuggerSessionAuth();

  // Sends a stopped event to the client.
  void sendStoppedEvent(
    const char* reason,
    const char* displayReason,
    request_id_t threadId,
    bool focusedThread,
    int breakpointId
  );

  // Sends a thread continued event to the client.
  void sendContinuedEvent(request_id_t threadId);

  // Sets the dummy thread ID.
  void setDummyThreadId(int64_t threadId);

  // Called when a new breakpoint is added to sync it to all requests.
  void onBreakpointAdded(int bpId);

  void onExceptionBreakpointChanged(bool isSet);

  // Attempts to resolve and install breakpoints for the current request thread.
  // Will either install the breakpoint or add it to the request's unresolved
  // list.
  void tryInstallBreakpoints(DebuggerRequestInfo* ri);

  // Called when a request loads a new compilation unit.
  void onCompilationUnitLoaded(
    DebuggerRequestInfo* ri,
    const HPHP::Unit* compilationUnit
  );

  // Called when a function is intercepted.
  void onFuncIntercepted(std::string funcName);

  // Called when the request defines a new function.
  void onFunctionDefined(
    DebuggerRequestInfo* ri,
    const Func* func,
    const std::string& funcName
  );

  // Called when a request thinks it has hit a source breakpoint.
  void onLineBreakpointHit(
    DebuggerRequestInfo* ri,
    const HPHP::Unit* compilationUnit,
    int line
  );

  // Called when a request thinks it has hit a function breakpoint.
  void onFuncBreakpointHit(
    DebuggerRequestInfo* ri,
    const HPHP::Func* func
  );

  // Called when a request hits an exception.
  void onExceptionBreakpointHit(
    DebuggerRequestInfo* ri,
    const std::string& exceptionName,
    const std::string& exceptionMsg
  );

  void onError(
    DebuggerRequestInfo* requestInfo,
    const ExtendedException& extendedException,
    int errnum,
    const std::string& message
  );

  // Called when the client requests an async-break of all threads.
  void onAsyncBreak();

  // Called when the user code includes a hard breakpoint, via a call
  // to hphp_debug_break().
  // Returns true if the debugger successfully broke in (and then resumed,
  // since the thread is returning) or false if no debugger was attached.
  bool onHardBreak();

  // Checks if we are stepping for a particular request.
  static bool isStepInProgress(DebuggerRequestInfo* requestInfo);

  // Clears the state filters for a step operation on the specified request
  // thread, if any step is currently in progress.
  static void clearStepOperation(DebuggerRequestInfo* requestInfo);

  // Adjusts a breakpoints source line based on the source mapping table in
  // the specified compilation unit in which the breakpoint is being installed.
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
  void sendThreadEventMessage(
    request_id_t threadId,
    ThreadEventType eventType
  );

  // Returns true if the current thread is the dummy request thread, false
  // otherwise.
  bool isDummyRequest() {
    return (int64_t)Process::GetThreadId() ==
      m_dummyThreadId.load(std::memory_order_acquire);
  }

  // Populates the specified folly::dynamic array with a list of thread IDs.
  void getAllThreadInfo(folly::dynamic& threads);

  bool isPaused() { return m_state != ProgramState::Running; }

  static bool hasSameTty() {
    return !RuntimeOption::ServerExecutionMode() &&
      RuntimeOption::VSDebuggerListenPort <= 0 &&
      RuntimeOption::VSDebuggerDomainSocketPath.empty();
  }

  // Returns the current stdout hook if one is installed, or nullptr otherwise.
  DebuggerStdoutHook* getStdoutHook() {
    return clientConnected() && !hasSameTty() ? &m_stdoutHook : nullptr;
  }

  DebuggerStderrHook* getStderrHook() {
    return clientConnected() ? &m_stderrHook : nullptr;
  }

  // Gets the current VM location, returns a pair of Unit* and current line.
  static std::pair<const Unit*, HPHP::SourceLoc> getVmLocation() {
    VMRegAnchor regAnchor;
    const ActRec* fp = g_context->getStackFrame();
    const Func* func = fp != nullptr ? fp->func() : nullptr;
    const Unit* unit = func != nullptr ? func->unit() : nullptr;
    HPHP::SourceLoc loc;
    if (unit != nullptr) {
      bool success = func->getSourceLoc(pcOff(), loc);
      if (success) {
        return std::pair<const Unit*, HPHP::SourceLoc>(unit, loc);
      }
    }
    return std::pair<const Unit*, HPHP::SourceLoc> (nullptr, loc);
  }

  // Dispatches the specified command to a request.
  void dispatchCommandToRequest(
    request_id_t requestId,
    VSCommand* command
  );

  bool getDebuggerOption(const HPHP::String& option);
  void setDebuggerOption(const HPHP::String& option, bool value);
  void setDebuggerOptions(DebuggerOptions options);

  DebuggerOptions getDebuggerOptions() {
    Lock lock(m_lock);
    return m_debuggerOptions;
  }

  void initWatchmanClient();
  std::shared_ptr<Watchman> getWatchmanClient() const;
  void checkForFileChanges(DebuggerRequestInfo*);

private:

  // Cleans up server objects for a request.
  void cleanupServerObjectsForRequest(DebuggerRequestInfo* ri);

  // Attaches the debugger to the specified request thread and installs the
  // debugger hook.
  DebuggerRequestInfo* attachToRequest(RequestInfo* ti);

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

  void onBreakpointHit(
    DebuggerRequestInfo* ri,
    const HPHP::Unit* compilationUnit,
    const HPHP::Func* func,
    int line
  );

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
  void pauseTarget(DebuggerRequestInfo* ri, bool isAsyncPause);

  // Attempts to resolve and install a breakpoint for the current request.
  // Returns true if the bp was resolved, false if it is unresolved and pending.
  bool tryResolveBreakpoint(
    DebuggerRequestInfo* ri,
    const int bpId,
    const Breakpoint* bp
  );

  // Attempts to resolve the breakpoint in the specified compilation unit.
  // Returns true on success, false otherwise.
  bool tryResolveBreakpointInUnit(
    const DebuggerRequestInfo* ri,
    int bpId,
    const Breakpoint* bp,
    const std::string& unitFilePath,
    const HPHP::Unit* compilationUnit
  );

  // Caches a flag on the request info indicating if there are any pending
  // or unresolved breakpoints, so that the hook can skip calling into the
  // debugger (and acquiring the debugger lock) every time a new func or
  // compilation unit is defined if there are no unresolved breakpoints.
  static inline void updateUnresolvedBpFlag(DebuggerRequestInfo* ri);

  // Notifies all threads that they need to switch to interpreted mode so we
  // can interrupt them.
  void interruptAllThreads();

  // Executes the specified lambda function for each request the debugger
  // is currently attached to.
  void executeForEachAttachedRequest(
    std::function<void(
      // Supplies the RequestInfo for the request thread.
      // NOTE: This parameter will be nullptr for the dummy request.
      RequestInfo* ti,
      // Supplies the request info for the request thread. This will
      // never be nullptr.
      DebuggerRequestInfo* ri
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
  PrepareToPauseResult prepareToPauseTarget(DebuggerRequestInfo* requestInfo);

  // Returns a stop reason string for a breakpoint.
  static std::string getStopReasonForBp(
    const Breakpoint* bp,
    const std::string& path,
    const int line
  );

  void setClientIdFromCommand(VSCommand* command);

  // Returns the request info for the dummy, if one exists or nullptr
  // if there is no client connected.
  DebuggerRequestInfo* getDummyRequestInfo();

  // Returns the next unused synthetic request thread ID.
  request_id_t nextThreadId();

  // Worker routine to clean up old debugger sessions.
  void runSessionCleanupThread();

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
  std::unordered_map<RequestInfo*, DebuggerRequestInfo*> m_requests;

  // Map of synthetic thread ID to RequestInfo*;
  std::unordered_map<request_id_t, RequestInfo*> m_requestIdMap;

  // Map of RequestInfo* to synthetic thread ID
  std::unordered_map<RequestInfo*, request_id_t> m_requestInfoMap;

  // Next synthetic request ID to send to client as thread ID.
  request_id_t m_nextThreadId {1};

  // Keeps track of the number of requests that are currently blocked inside
  // the debugger extension due to being paused for any reason (breakpoint,
  // exception, loader break, etc...)
  uint64_t m_pausedRequestCount {0};

  // Keeps track of the total number of requests attached to by this extension
  // since the server was started.
  std::atomic<uint64_t> m_totalRequestCount {0};

  // Tracks the thread ID of the dummy request.
  std::atomic<int64_t> m_dummyThreadId {-1};

  // Support for waiting for a client connection to arrive.
  std::mutex m_connectionNotifyLock;
  std::condition_variable m_connectionNotifyCondition;

  // Support for waiting for a resume to complete before executing another
  // break of the target and a pause to complete before another thread can
  // also issue a pause
  std::mutex m_resumeMutex;
  std::condition_variable m_resumeCondition;

  // Client options.
  DebuggerOptions m_debuggerOptions {0};

  // Worker thread to clean up old session objects as debugger clients
  // disconnect. Cleaning the session object requires joining with the
  // dummy thread, which can block if it's in native code or a loop
  // or something, so we need to do this in the background.
  std::mutex m_sessionCleanupLock;
  bool m_sessionCleanupTerminating {false};
  std::condition_variable m_sessionCleanupCondition;
  AsyncFunc<Debugger> m_sessionCleanupThread;
  std::unordered_set<DebuggerSession*> m_cleanupSessions;

  // Pending event messages that are queued to be sent after the current
  // client command is processed and responded to.
  struct PendingEventMessage {
    folly::dynamic m_message;
    const char* m_eventType;
  };
  std::vector<PendingEventMessage> m_pendingEventMessages;
  bool m_processingClientCommand {false};

  // Hooks for stdout and stderr redirection.
  DebuggerStdoutHook m_stdoutHook {DebuggerStdoutHook(this)};
  DebuggerStderrHook m_stderrHook {DebuggerStderrHook(this)};

  // Watchman client for detecting file changes in REPL request.
  std::shared_ptr<Watchman> m_watchmanClient {nullptr};

  static constexpr char* InternalErrorMsg =
    "An internal error occurred while processing a debugger command.";
};

// A stdout hook that no-ops all writes to stdout.
struct NoOpStdoutHook final : ExecutionContext::StdoutHook {
  explicit NoOpStdoutHook() {}
  void operator()(const char* /*str*/, int /*len*/) override {}
};

// When performing an evaluation on behalf of part of the debugger engine
// that should not be shown to the user in any way (such as for completions,
// or conditional breakpoints), we want to suppress all output from the VM,
// and prevent breaks of any kind. This class provides those semantics, and
// then disables them when it is destroyed.
struct SilentEvaluationContext {
  SilentEvaluationContext(
    Debugger* debugger,
    DebuggerRequestInfo* ri,
    bool suppressOutput = true
  );

  ~SilentEvaluationContext();

private:
  DebuggerRequestInfo* m_ri;
  bool m_suppressOutput;
  int m_errorLevel;
  StringBuffer* m_savedOutputBuffer;
  NoOpStdoutHook m_noOpHook;
  ExecutionContext::StdoutHook* m_oldHook;
  StringBuffer m_sb;

  PCFilter m_savedFlowFilter;
  PCFilter m_savedBpFilter;
};

struct DebuggerNoBreakContext {
  bool m_prevDoNotBreak;
  bool m_prevDbgNoBreak;
  DebuggerRequestInfo* m_requestInfo;

  DebuggerNoBreakContext(Debugger* debugger);
  ~DebuggerNoBreakContext();
};

}
}
