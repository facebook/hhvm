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
#include "hphp/runtime/ext/vsdebug/php_executor.h"

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
namespace VSDEBUG {

PHPExecutor::~PHPExecutor()
{
}

PHPExecutor::PHPExecutor(
  Debugger *debugger,
  DebuggerSession *session,
  const std::string &breakpointFireMessage,
  request_id_t threadId,
  bool evalSilent
) : m_debugger{debugger},
    m_session{session},
    m_breakpointFireMessage{breakpointFireMessage},
    m_threadId{threadId},
    m_ri{nullptr},
    m_evalSilent{evalSilent}
{
}

void PHPExecutor::execute()
{
  // Enable bypassCheck, which allows eval statements from the debugger to
  // violate visibility checks on object properties.
  g_context->debuggerSettings.bypassCheck = true;

  // Set the error reporting level to 0 so non-fatal errors
  // are swallowed.
  RequestInjectionData& rid = RID();
  const int previousErrorLevel = rid.getErrorReportingLevel();
  if (m_evalSilent) {
    rid.setErrorReportingLevel(0);
  }

  m_ri = m_debugger->getRequestInfo();
  if (!m_ri) {
    // A thread acquires the debugger lock before executing client commands,
    // preventing the client from disconnecting in the middle of executing the
    // command. However, a client command may execute multiple php codes and it
    // drops the lock before executing any php code (and requires it afterwards)
    // allowing clients to be disconnected in the middle. When that happens,
    // request info may be null.
    return;
  }
  assertx(m_ri->m_evaluateCommandDepth >= 0);
  m_ri->m_evaluateCommandDepth++;

  // Track if the call caused any opcode stepping to occur so we know
  // if we need to re-send a stop event after the evaluation.
  int previousPauseCount = m_ri->m_totalPauseCount;
  bool isDummy = m_debugger->isDummyRequest();
  bool exitDummyContext = false;

  // Put everything back on scope exit.
  SCOPE_EXIT {
    g_context->debuggerSettings.bypassCheck = false;
    rid.setErrorReportingLevel(previousErrorLevel);

    m_ri->m_evaluateCommandDepth--;
    assertx(m_ri->m_evaluateCommandDepth >= 0);

    if (m_ri->m_evaluateCommandDepth == 0 && isDummy) {
      // The dummy request only appears in the client UX while it is
      // stopped at a breakpoint during an evaluation (because the user
      // needs to see a call stack and scopes at that point). Otherwise,
      // existence of the dummy is hidden from the user. If the dummy is
      // no longer executing any evaluation, send a thread exited event
      // to remove it from the front-end UX.
      m_debugger->sendThreadEventMessage(
        0,
        Debugger::ThreadEventType::ThreadExited
      );

      if (exitDummyContext) {
        g_context->exitDebuggerDummyEnv();
      }
    }

    // It is difficult to prove if this call wrote to any
    // server constant or server global, so we must err on the side of caution
    // and invalidate cached copies.
    m_session->clearCachedVariable(DebuggerSession::kCachedVariableKeyAll);

    m_ri = nullptr;
  };

  if (m_ri->m_evaluateCommandDepth == 1 && isDummy) {
    // Set up the dummy evaluation environment unless we have recursively
    // re-entered eval on the dummy thread, in which case it's already set.
    if (vmfp() == nullptr && vmStack().count() == 0) {
      g_context->enterDebuggerDummyEnv();
      exitDummyContext = true;
    }

    // Show the dummy thread while it is doing an evaluation so it can
    // present a call stack if it hits a breakpoint during the eval.
    m_debugger->sendThreadEventMessage(
      0,
      Debugger::ThreadEventType::ThreadStarted
    );
  }

  // We must drop the lock before calling PHP code because the code
  // is permitted to hit breakpoints, which can call back into Debugger and
  // enter a command queue. Threads must never enter the command queue while
  // holding the debugger lock, because we would be unable to processes more
  // commands from the client: there'd be no way to resume the blocked request.

  Variant debugDisplay;
  m_debugger->executeWithoutLock(
    [&]() {
      callPHPCode();
    });

  if (previousPauseCount != m_ri->m_totalPauseCount &&
      m_ri->m_pauseRecurseCount > 0) {

    m_debugger->sendStoppedEvent(
      "breakpoint",
      m_breakpointFireMessage.c_str(),
      m_threadId,
      true,
      -1
    );
  }
}

}
}
