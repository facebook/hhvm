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

#include "hphp/runtime/ext/vsdebug/hook.h"

namespace HPHP {
namespace VSDEBUG {

void VSDebugHook::onRequestInit() {
  tryEnterDebugger();
}

void VSDebugHook::onRequestShutdown() {
}

void VSDebugHook::onOpcode(PC pc) {
  RID().setDebuggerIntr(false);
  tryEnterDebugger();
}

void VSDebugHook::onFuncEntryBreak(const Func* func) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onFuncExitBreak(const Func* func) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onLineBreak(const Unit* unit, int line) {
  // If the VM thinks this request has hit a line break, first try to enter
  // the debugger if it is already paused due to another event (another
  // request's bp, an exception, an async-break, etc) and handle that.
  tryEnterDebugger();

  // After resuming from that, service this breakpoint if it is still active.
  Debugger* debugger = VSDebugExtension::getDebugger();
  RequestInfo* requestInfo = debugger->getRequestInfo();
  if (requestInfo == nullptr) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Could not find request info for request!"
    );
    return;
  }

  debugger->onLineBreakpointHit(requestInfo, unit, line);
}

void VSDebugHook::onExceptionThrown(ObjectData* exception) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onError(
  const ExtendedException& extendedException,
  int errnum,
  const std::string& message
) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onStepInBreak(const Unit* unit, int line) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onStepOutBreak(const Unit* unit, int line) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onNextBreak(const Unit* unit, int line) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onFileLoad(Unit* efile) {
  Debugger* debugger = VSDebugExtension::getDebugger();
  RequestInfo* requestInfo = debugger->getRequestInfo();
  if (requestInfo == nullptr) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Could not find request info for request!"
    );
    return;
  }

  // Resolve any unresolved breakpoints that may be in this compilation unit.
  debugger->onCompilationUnitLoaded(requestInfo, efile);
  tryEnterDebugger();
}

void VSDebugHook::onDefClass(const Class* cls) {
}

void VSDebugHook::onDefFunc(const Func* func) {
  // TODO: (Ericblue) This routine is not needed unless we add support for
  // function entry breakpoints.
}

void VSDebugHook::tryEnterDebugger() {
  Debugger* debugger = VSDebugExtension::getDebugger();
  if (!debugger->clientConnected()) {
    return;
  }

  RequestInfo* requestInfo = debugger->getRequestInfo();
  if (requestInfo == nullptr) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Could not find request info for request!"
    );
    return;
  }

  // The first time this request enters the debugger, remove the artificial
  // memory limit since a debugger is attached.
  if (!requestInfo->m_flags.memoryLimitRemoved) {
    IniSetting::SetUser(s_memoryLimit, std::numeric_limits<int64_t>::max());
    requestInfo->m_flags.memoryLimitRemoved = true;
  }

  debugger->enterDebuggerIfPaused(requestInfo);

  // Install any breakpoints that have not yet been set on this request.
  debugger->tryInstallBreakpoints(requestInfo);
}

const StaticString VSDebugHook::s_memoryLimit {"memory_limit"};

}
}
