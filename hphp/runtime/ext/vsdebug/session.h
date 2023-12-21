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

#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/runtime/ext/vsdebug/transport.h"
#include "hphp/runtime/ext/vsdebug/command_queue.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/breakpoint.h"
#include "hphp/runtime/ext/vsdebug/client_preferences.h"
#include "hphp/runtime/ext/vsdebug/server_object.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/util/async-func.h"

namespace HPHP {
namespace VSDEBUG {

struct Debugger;
struct BreakpointManager;
struct DebuggerRequestInfo;

// This object represents a connected session with a single debugger client.
// It contains any data specific to the connected client's state.
struct DebuggerSession final {
  DebuggerSession(Debugger* debugger);
  virtual ~DebuggerSession();

  void startDummyRequest(
    const std::string& startupDoc,
    const std::string& sandboxUser,
    const std::string& sandboxName,
    const std::string& debuggerSessionAuth,
    bool displayStartupMsg
  );

  void enqueueDummyCommand(VSCommand* command);
  void setClientPreferences(ClientPreferences& preferences);
  ClientPreferences& getClientPreferences();

  BreakpointManager* getBreakpointManager() { return m_breakpointMgr; }

  unsigned int generateFrameId(request_id_t requestId, int frameDepth);
  FrameObject* getFrameObject(int objectId);

  unsigned int generateScopeId(
    request_id_t requestId,
    int depth,
    ScopeType scopeType
  );
  ScopeObject* getScopeObject(unsigned int objectId);

  unsigned int generateVariableId(request_id_t requestId, Variant& variable);

  unsigned int generateVariableSubScope(
    request_id_t requestId,
    const Variant& variable,
    const Class* cls,
    const std::string& className,
    ClassPropsType type
  );

  ServerObject* getServerObject(unsigned int objectId);

  // Called by the debugger when a server object is removed from a request.
  void onServerObjectDestroyed(unsigned int objectId);

  // DebuggerRequestInfo for the dummy request thread.
  DebuggerRequestInfo* const m_dummyRequestInfo;

  // Indicates if the client wants output about the startup request state.
  bool m_displayStartupMsg;

  folly::dynamic* getCachedVariableObject(const int key);
  void setCachedVariableObject(const int key, const folly::dynamic& value);
  void clearCachedVariable(const int key);

  std::string getDebuggerSessionAuth();

  void setClientId(const std::string& clientId) { m_clientId = clientId; }
  std::string getClientId() const { return m_clientId; }

  uint32_t getSessionId() const { return m_sessionId; }

  int getCurrFrameId() const { return m_currFrameId; }
  void setCurrFrameId(int frameId) { m_currFrameId = frameId; }

  static constexpr int kCachedVariableKeyAll = -1;
  static constexpr int kCachedVariableKeyServerConsts = 1;
  static constexpr int kCachedVariableKeyUserConsts = 2;
  static constexpr int kCachedVariableKeyServerGlobals = 3;

private:

  unsigned int generateOrReuseVariableId(
    const Variant& variable
  );

  void registerRequestObject(
    unsigned int objectId,
    ServerObject* obj
  );

  Debugger* const m_debugger;
  ClientPreferences m_clientPreferences;

  // Support for breakpoints. Breakpoints are tied to a particular client
  // sesson. When the client disconnects, its breakpoints disappear.
  BreakpointManager* m_breakpointMgr;

  // The "dummy" request thread is a hidden request that provides an execution
  // context from which to execute any debugger command that is not directed at
  // a specific real request. For example: servicing evaluate commands while
  // the debugger is not broken in.

  void runDummy();
  void invokeDummyStartupDocument();

  AsyncFunc<DebuggerSession> m_dummyThread;
  std::string m_dummyStartupDoc;

  // Auth token provided by the attached debugger client.
  // Note: it is only safe to read this from the dummy request thread.
  std::string m_debuggerSessionAuth;

  // When a request is paused, the backend must maintain state about scopes,
  // frames and variable IDs sent to the front end. The IDs are globally
  // unique, and the objects to which they refer are valid per-request..
  static unsigned int s_nextObjectId;
  std::unordered_map<unsigned int, ServerObject*> m_serverObjects;

  // A cache for things like server constants that are expensive to compute,
  // are requested for each thread and frame, and unlikely to change while
  // the target is paused. Items in this cache are global, not per-request.
  std::unordered_map<int, folly::dynamic> m_globalVariableCache;

  // Frame id of the last ScopesCommand run in this session.
  // If the execution is paused, the EvaluateCommands will be executed in the
  // context of this frame id.
  int m_currFrameId {-1};

  std::string m_sandboxUser;
  std::string m_sandboxName;

  std::string m_clientId;
  uint32_t m_sessionId;
};

}
}

