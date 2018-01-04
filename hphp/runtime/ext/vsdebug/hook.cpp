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
  if (!shouldEnterDebugger()) {
    return;
  }

  // TODO: (Ericblue) Any breakpoints that are already set need to be synced
  // to the new request thread here.

}

void VSDebugHook::onRequestShutdown() {
}

void VSDebugHook::onOpcode(PC pc) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onFuncEntryBreak(const Func* func) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onFuncExitBreak(const Func* func) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onLineBreak(const Unit* unit, int line) {
  // TODO: (Ericblue) NOT IMPLEMENTED
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
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onDefClass(const Class* cls) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onDefFunc(const Func* func) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

bool VSDebugHook::shouldEnterDebugger() {
  Debugger* debugger = VSDebugExtension::getDebugger();
  if (!debugger->clientConnected()) {
    return false;
  }

  RequestInfo* requestInfo = debugger->getRequestInfo();

  // The first time this request enters the debugger, remove the artificial
  // memory limit since a debugger is attached.
  if (!requestInfo->m_flags.memoryLimitRemoved) {
    IniSetting::SetUser(s_memoryLimit, std::numeric_limits<int64_t>::max());
    requestInfo->m_flags.memoryLimitRemoved = true;
  }

  return true;
}

const StaticString VSDebugHook::s_memoryLimit {"memory_limit"};

}
}
