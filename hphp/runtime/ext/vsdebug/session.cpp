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
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/util/process.h"

namespace HPHP {
namespace VSDEBUG {

DebuggerSession::DebuggerSession(Debugger* debugger) :
  m_debugger(debugger),
  m_breakpointMgr(new BreakpointManager(debugger)),
  m_dummyThread(this, &DebuggerSession::runDummy),
  m_dummyStartupDoc("") {

  assert(m_debugger != nullptr);
}

DebuggerSession::~DebuggerSession() {
  m_dummyCommandQueue.shutdown();
  m_dummyThread.waitForEnd();

  if (m_breakpointMgr != nullptr) {
    delete m_breakpointMgr;
  }
}

void DebuggerSession::startDummyRequest(const std::string& startupDoc) {
  m_dummyStartupDoc = File::TranslatePath(startupDoc).data();
  m_dummyThread.start();
}

void DebuggerSession::invokeDummyStartupDocument() {
  m_debugger->sendUserMessage(
    "Preparing your Hack/PHP console. Please wait...",
    DebugTransport::OutputLevelWarning
  );

  // If a startup document was specified, invoke it now.
  bool error;
  std::string errorMsg;
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
                         true);
  if (!ret || error) {
    std::string displayError =
      std::string("Failed to prepare the Hack/PHP console: ") + errorMsg;

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "%s",
      displayError.c_str()
    );

    m_debugger->sendUserMessage(
      displayError.c_str(),
      DebugTransport::OutputLevelError
    );
  } else {
    m_debugger->sendUserMessage(
      "The Hack/PHP console is now ready to use.",
      DebugTransport::OutputLevelSuccess
    );
  }
}

const StaticString s_memory_limit("memory_limit");

void DebuggerSession::runDummy() {

  // The debugger needs to know which background thread is processing the dummy
  // request. It should not attach to this request as it would a real request:
  // it should not be included in operattions like async-break-all, nor should
  // it be listed in any user-visible thread list.
  m_debugger->setDummyThreadId((int64_t)Process::GetThreadId());

  hphp_session_init();
  SCOPE_EXIT {
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

  // Remove the artificial memory limit for this request since there is a
  // debugger attached to it.
  IniSetting::SetUser(s_memory_limit, std::numeric_limits<int64_t>::max());

  if (!m_dummyStartupDoc.empty()) {
    invokeDummyStartupDocument();
  } else {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelWarning,
      "Dummy request started without a startup document."
    );
  }

  m_dummyCommandQueue.processCommands();
  DebuggerHook::detach();
}

void DebuggerSession::enqueueDummyCommand(VSCommand* command) {
  m_dummyCommandQueue.dispatchCommand(command);
}

void DebuggerSession::setClientPreferences(ClientPreferences& preferences) {
  m_clientPreferences = preferences;
}

ClientPreferences& DebuggerSession::getClientPreferences() {
  return m_clientPreferences;
}

unsigned int DebuggerSession::generateFrameId(int requestId, int frameDepth) {
  const unsigned int objectId = ++s_nextObjectId;
  FrameObject* frame = new FrameObject(objectId, requestId, frameDepth);

  assert(requestId == m_debugger->getCurrentThreadId());
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
  int requestId,
  int depth,
  ScopeType scopeType
) {
  const unsigned int objectId = ++s_nextObjectId;
  ScopeObject* scope = new ScopeObject(objectId, requestId, depth, scopeType);

  assert(requestId == m_debugger->getCurrentThreadId());
  registerRequestObject(objectId, scope);
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

unsigned int DebuggerSession::s_nextObjectId {0};
}
}
