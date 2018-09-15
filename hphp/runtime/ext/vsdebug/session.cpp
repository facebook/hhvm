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

#include "hphp/runtime/ext/vsdebug/session.h"

#include "hphp/runtime/ext/vsdebug/debugger.h"

#include "hphp/runtime/vm/treadmill.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/program-functions.h"

#include "hphp/util/process.h"

namespace HPHP {
namespace VSDEBUG {

DebuggerSession::DebuggerSession(Debugger* debugger) :
  m_dummyRequestInfo(Debugger::createRequestInfo()),
  m_displayStartupMsg(false),
  m_debugger(debugger),
  m_breakpointMgr(new BreakpointManager(debugger)),
  m_dummyThread(this, &DebuggerSession::runDummy),
  m_dummyStartupDoc("") {

  assertx(m_debugger != nullptr);
}

DebuggerSession::~DebuggerSession() {
  m_dummyRequestInfo->m_commandQueue.shutdown();
  m_dummyThread.waitForEnd();

  if (m_breakpointMgr != nullptr) {
    delete m_breakpointMgr;
  }

  if (m_dummyRequestInfo != nullptr) {
    Debugger::cleanupRequestInfo(nullptr, m_dummyRequestInfo);
  }
}

void DebuggerSession::startDummyRequest(
  const std::string& startupDoc,
  const std::string& sandboxUser,
  const std::string& sandboxName,
  bool displayStartupMsg
) {
  m_sandboxUser = sandboxUser;
  m_sandboxName = sandboxName;
  m_dummyStartupDoc = File::TranslatePath(startupDoc).data();
  m_displayStartupMsg = displayStartupMsg;

  // Flush dirty writes to m_sandboxUser and m_dummyStartupDoc.
  std::atomic_thread_fence(std::memory_order_release);

  m_dummyThread.start();
}

void DebuggerSession::invokeDummyStartupDocument() {

  if (m_displayStartupMsg) {
    m_debugger->sendUserMessage(
      "Preparing your Hack/PHP console. Please wait...",
      DebugTransport::OutputLevelWarning
    );
  }

  // If a startup document was specified, invoke it now.
  bool error;
  std::string errorMsg;

  // We must not hit any breakpoints in the dummy while it is initializing,
  // the rest of the debugger is not prepared to deal with a bp yet and the
  // dummy thread would get stuck.
  m_dummyRequestInfo->m_flags.doNotBreak = true;
  std::atomic_thread_fence(std::memory_order_release);

  bool ret = hphp_invoke(g_context.getCheck(),
                         m_dummyStartupDoc,
                         false,
                         null_array,
                         uninit_null(),
                         "",
                         "",
                         error,
                         errorMsg,
                         true,
                         false,
                         true,
                         RuntimeOption::EvalPreludePath);

  if (!ret || error) {
    if (errorMsg == "") {
      errorMsg = "An unknown error was returned from HPHP.";
    }

    std::string displayError =
      std::string("Failed to prepare the Hack/PHP console. ");
    displayError += "Error requiring document ";
    displayError += m_dummyStartupDoc;
    displayError += ": " + errorMsg;
    displayError += ". This may cause Hack/PHP types and symbols to be ";
    displayError += "unresolved in console expressions. You can try running ";
    displayError += "`require('"
        + m_dummyStartupDoc
        + "')` in the console to attempt loading the document again.";

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelWarning,
      "%s",
      displayError.c_str()
    );

    m_debugger->sendUserMessage(
      displayError.c_str(),
      DebugTransport::OutputLevelWarning
    );
  } else {
    if (m_displayStartupMsg) {
      m_debugger->sendUserMessage(
        "The Hack/PHP console is now ready to use.",
        DebugTransport::OutputLevelSuccess
      );
    }
  }

  m_dummyRequestInfo->m_flags.doNotBreak = false;
  std::atomic_thread_fence(std::memory_order_release);
}

const StaticString s_memory_limit("memory_limit");
const StaticString s__SERVER("_SERVER");

void DebuggerSession::runDummy() {
  // The debugger needs to know which background thread is processing the dummy
  // request. It should not attach to this request as it would a real request:
  // it should not be included in operattions like async-break-all, nor should
  // it be listed in any user-visible thread list.
  m_debugger->setDummyThreadId((int64_t)Process::GetThreadId());

  // While the main thread is starting up and preparing the system lib,
  // the code emitter tags all newly compiled functions as "built in".
  // If we pull in the startup document while this is happening, all functions
  // in the startup doc get erroneously tagged as builtin, which breaks
  // our stack traces later if any breakpoint is hit in a routine pulled in
  // by the startup document (especially true of dummy evals with bps in them).
  //
  // Unfortunately there is no signal when this is complete, so we're going to
  // have to poll here. Wait for a maximum time and then proceed anyway.
  int pollCount = 0;
  constexpr int pollTimePerIterUs = 100 * 1000;
  constexpr int pollMaxTimeUs = 3 * 1000 * 1000;
  while (!SystemLib::s_inited &&
         pollCount * pollTimePerIterUs < pollMaxTimeUs) {

    pollCount++;

    // Wait 100ms and try again.
    usleep(pollTimePerIterUs);

    // Ensure this thread sees any writes to SystemLib::s_inited.
    std::atomic_thread_fence(std::memory_order_acquire);
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Polled %d times waiting for SystemLib::s_inited to be true. "
      "(SystemLib::s_inited = %s)",
    pollCount,
    SystemLib::s_inited ? "TRUE" : "FALSE"
  );

  bool hookAttached = false;
  hphp_session_init(Treadmill::SessionKind::Vsdebug);
  init_command_line_globals(0, nullptr, environ, 0,
                            RuntimeOption::ServerVariables,
                            RuntimeOption::EnvVariables);
  SCOPE_EXIT {
    g_context->removeStdoutHook(m_debugger->getStdoutHook());
    Logger::SetThreadHook(nullptr);

    if (hookAttached) {
      DebuggerHook::detach();
    }

    std::atomic_thread_fence(std::memory_order_release);

    hphp_context_exit();
    hphp_session_exit();
  };

  if (!DebuggerHook::attach<VSDebugHook>()) {
    m_debugger->sendUserMessage(
      "Failed to attach the debugger to the Hack/PHP console: another debugger "
      "is already attached!",
      DebugTransport::OutputLevelError
    );
    return;
  }

  hookAttached = true;

  // Remove the artificial memory limit for this request since there is a
  // debugger attached to it.
  m_dummyRequestInfo->m_flags.memoryLimitRemoved = true;

  std::atomic_thread_fence(std::memory_order_release);

  // Redirect the dummy's stdout and stderr and enable implicit flushing
  // so output is sent to the client right away, instead of being buffered.
  g_context->addStdoutHook(m_debugger->getStdoutHook());
  Logger::SetThreadHook(m_debugger->getStderrHook());
  g_context->obSetImplicitFlush(true);

  // Setup sandbox variables for dummy request context.
  if (!m_sandboxUser.empty()) {
    SourceRootInfo sourceRootInfo(m_sandboxUser, m_sandboxName);
    auto server = php_global_exchange(s__SERVER, init_null());
    forceToArray(server);
    Array arr = server.toArrRef();
    server.unset();
    php_global_set(
      s__SERVER,
      sourceRootInfo.setServerVariables(std::move(arr))
    );

    g_context->setSandboxId(sourceRootInfo.getSandboxInfo().id());
  }

  if (!m_dummyStartupDoc.empty()) {
    invokeDummyStartupDocument();
  } else {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelWarning,
      "Dummy request started without a startup document."
    );
    m_debugger->sendUserMessage(
      "No startup document was specified, not loading any Hack/PHP "
        "types for the console.",
      DebugTransport::OutputLevelWarning
    );
  }

  folly::dynamic event = folly::dynamic::object;
  m_debugger->sendEventMessage(event, "readyForEvaluations", true);

  m_dummyRequestInfo->m_commandQueue.processCommands();
}

void DebuggerSession::enqueueDummyCommand(VSCommand* command) {
  m_dummyRequestInfo->m_commandQueue.dispatchCommand(command);
}

void DebuggerSession::setClientPreferences(ClientPreferences& preferences) {
  m_clientPreferences = preferences;
}

ClientPreferences& DebuggerSession::getClientPreferences() {
  return m_clientPreferences;
}

unsigned int DebuggerSession::generateFrameId(
  request_id_t requestId,
  int frameDepth
) {
  const unsigned int objectId = ++s_nextObjectId;
  FrameObject* frame = new FrameObject(objectId, requestId, frameDepth);

  assertx(requestId == m_debugger->getCurrentThreadId());
  registerRequestObject(objectId, frame);
  return objectId;
}

FrameObject* DebuggerSession::getFrameObject(unsigned int objectId) {
  auto object = getServerObject(objectId);
  if (object != nullptr) {
    if (object->objectType() != ServerObjectType::Frame) {
      throw DebuggerCommandException(
        "Object with the specified ID is not a frame!"
      );
    }
  }

  return static_cast<FrameObject*>(object);
}

unsigned int DebuggerSession::generateScopeId(
  request_id_t requestId,
  int depth,
  ScopeType scopeType
) {
  const unsigned int objectId = ++s_nextObjectId;
  ScopeObject* scope = new ScopeObject(objectId, requestId, depth, scopeType);

  assertx(requestId == m_debugger->getCurrentThreadId());
  registerRequestObject(objectId, scope);
  return objectId;
}

unsigned int DebuggerSession::generateVariableId(
  request_id_t requestId,
  Variant& variable
) {
  const unsigned int objectId = ++s_nextObjectId;
  VariableObject* varObj = new VariableObject(objectId, requestId, variable);

  assertx(requestId == m_debugger->getCurrentThreadId());
  registerRequestObject(objectId, varObj);
  return objectId;
}

unsigned int DebuggerSession::generateVariableSubScope(
  request_id_t requestId,
  const Variant& variable,
  const Class* cls,
  const std::string& className,
  ClassPropsType type
) {
  const unsigned int objectId = ++s_nextObjectId;
  VariableSubScope* varObj = new VariableSubScope(
    objectId,
    variable,
    cls,
    className,
    requestId,
    type
  );

  registerRequestObject(objectId, varObj);
  return objectId;
}

ScopeObject* DebuggerSession::getScopeObject(unsigned int objectId) {
  auto object = getServerObject(objectId);
  if (object != nullptr) {
    if (object->objectType() != ServerObjectType::Scope) {
      throw DebuggerCommandException(
        "Object with the specified ID is not a scope!"
      );
    }
  }

  return static_cast<ScopeObject*>(object);
}

void DebuggerSession::registerRequestObject(
  unsigned int objectId,
  ServerObject* obj
) {
  RequestInfo* ri = m_debugger->getRequestInfo();

  // Add this object to the per-request list of objects.
  std::unordered_map<unsigned int, ServerObject*>& objs = ri->m_serverObjects;
  objs.emplace(objectId, obj);

  // Add this object to the per-session global list of objects.
  m_serverObjects.emplace(objectId, obj);
}

ServerObject* DebuggerSession::getServerObject(unsigned int objectId) {
  auto it = m_serverObjects.find(objectId);
  return it == m_serverObjects.end() ? nullptr : it->second;
}

void DebuggerSession::onServerObjectDestroyed(unsigned int objectId) {
  // Remove the object from the global server object map.
  // Note: it is possible for an object in a request's object list to not
  // exist in m_serverObjects because m_serverObjects is cleared if a debugger
  // client disconnects and reconnects - but requests that were paused
  // during that time will clean up their server objects asynchronously.
  auto serverIt = m_serverObjects.find(objectId);
  if (serverIt != m_serverObjects.end()) {
    m_serverObjects.erase(serverIt);
  }
}

folly::dynamic* DebuggerSession::getCachedVariableObject(const int key) {
  auto it = m_globalVariableCache.find(key);
  if (it != m_globalVariableCache.end()) {
    return &it->second;
  }

  return nullptr;
}

void DebuggerSession::setCachedVariableObject(
  const int key,
  const folly::dynamic& value
) {
  m_globalVariableCache.emplace(key, value);
}

void DebuggerSession::clearCachedVariable(const int key) {
  if (key == kCachedVariableKeyAll) {
    // Clear all keys.
    m_globalVariableCache.clear();
  } else {
    auto it = m_globalVariableCache.find(key);
    if (it != m_globalVariableCache.end()) {
      m_globalVariableCache.erase(it);
    }
  }
}

unsigned int DebuggerSession::s_nextObjectId {0};
}
}
