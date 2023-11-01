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

#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/debugger-request-info.h"

namespace HPHP {
namespace VSDEBUG {

struct BreakContext {
  BreakContext(bool breakNoStepOnly)
    : m_debugger(VSDebugExtension::getDebugger()),
      m_requestInfo(m_debugger->getRequestInfo()),
      m_breakNoStepOnly(breakNoStepOnly) {

    if (m_debugger == nullptr || m_requestInfo == nullptr) {
      return;
    }

    VSDebugHook::tryEnterDebugger(m_debugger, m_requestInfo, m_breakNoStepOnly);
  }

  Debugger* m_debugger;
  DebuggerRequestInfo* m_requestInfo;

  // If true, tryEnterDebugger should try to break only when a step operation
  // is not in progress, otherwise it should always ask the debugger if we
  // should break.
  const bool m_breakNoStepOnly;
};

void VSDebugHook::onRequestInit() {
  BreakContext breakContext(false);
  auto const disableJit = breakContext.m_debugger ?
    breakContext.m_debugger->getDebuggerOptions().disableJit :
    RuntimeOption::EvalJitDisabledByVSDebug;
  RID().setVSDebugDisablesJit(disableJit);
}

void VSDebugHook::onOpcode(PC /*pc*/) {
  RID().setDebuggerIntr(false);
  RID().clearFlag(DebuggerSignalFlag);
  BreakContext breakContext(true);
}

void VSDebugHook::onFuncEntryBreak(const Func* func) {
  BreakContext breakContext(true);

  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    if (breakContext.m_requestInfo->m_flags.doNotBreak) {
      return;
    }

    breakContext.m_debugger->onFuncBreakpointHit(
      breakContext.m_requestInfo,
      func
    );
  }
}

void VSDebugHook::onFuncExitBreak(const Func* /*func*/) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onLineBreak(const Unit* unit, int line) {
  BreakContext breakContext(true);

  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    if (breakContext.m_requestInfo->m_flags.doNotBreak) {
      return;
    }

    breakContext.m_debugger->onLineBreakpointHit(
      breakContext.m_requestInfo,
      unit,
      line
    );
  }
}

void VSDebugHook::onExceptionThrown(ObjectData* exception) {
  BreakContext breakContext(true);

  const StringData* name = exception->getVMClass()->name();
  const Variant msg = exception->o_invoke_few_args(
    s_getMsg, RuntimeCoeffects::fixme(), 0);
  const HPHP::String msg_str = msg.isNull() ? empty_string() : msg.toString();

  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    if (breakContext.m_requestInfo->m_flags.doNotBreak) {
      return;
    }

    breakContext.m_debugger->onExceptionBreakpointHit(
      breakContext.m_requestInfo,
      name == nullptr
        ? std::string("Unknown Exception")
        : name->toCppString(),
      msg_str.toCppString()
    );
  }
}

void VSDebugHook::onError(
  const ExtendedException& extendedException,
  int errnum,
  const std::string& message
) {
  BreakContext breakContext(true);

  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    if (breakContext.m_requestInfo->m_flags.doNotBreak) {
      return;
    }

    breakContext.m_debugger->onError(
      breakContext.m_requestInfo,
      extendedException,
      errnum,
      message
    );
  }
}

void VSDebugHook::onStepInBreak(const Unit* /*unit*/, int /*line*/) {
  BreakContext breakContext(false);
}

void VSDebugHook::onStepOutBreak(const Unit* /*unit*/, int /*line*/) {
  BreakContext breakContext(false);
}

void VSDebugHook::onNextBreak(const Unit* /*unit*/, int /*line*/) {
  BreakContext breakContext(true);

  if (breakContext.m_debugger == nullptr ||
      breakContext.m_requestInfo == nullptr) {
    return;
  }

  // When stepping over a multiline statement like a function call,
  // we might be filtering a particular line to avoid hitting it multiple times.
  auto ri = breakContext.m_requestInfo;
  while (!ri->m_nextFilterInfo.empty()) {
    auto currentLocation = Debugger::getVmLocation();
    const Unit* unit = currentLocation.first;
    const HPHP::SourceLoc& loc = currentLocation.second;

    const StepNextFilterInfo& filter = ri->m_nextFilterInfo.back();
    int skipLine0 = filter.skipLine0;
    int skipLine1 = filter.skipLine1;

    if (unit != filter.stepStartUnit ||
        loc.line0 < skipLine0 ||
        loc.line0 > skipLine1) {

      // We stepped into another unit or out of the source range that the
      // statement we're avoiding covers. Pop off the filter.
      ri->m_nextFilterInfo.pop_back();

      // Check the next filter in case we're inside a multi-line statement
      // that is inside a multi-line statement...
      continue;
    } else if (loc.line0 == skipLine0 || loc.line1 == skipLine1) {
      // If this location is filtered out, skip stopping at this PC.
      // Re-enable interrupts so we get invoked again on the next opcode.
      RID().setDebuggerIntr(true);
      phpDebuggerNext();
      return;
    } else {
      // Still in the range of the current filter, but we didn't decide
      // to skip this line. Enter the debugger.
      break;
    }
  }

  VSDebugHook::tryEnterDebugger(
    breakContext.m_debugger,
    breakContext.m_requestInfo,
    false
  );
}

void VSDebugHook::onFileLoad(Unit* efile) {
  Debugger* debugger = VSDebugExtension::getDebugger();
  if (debugger == nullptr) {
    return;
  }

  DebuggerRequestInfo* requestInfo = debugger->getRequestInfo();
  if (requestInfo == nullptr) {
    return;
  }

  std::filesystem::path p{efile->filepath()->toCppString()};
  auto const& unitFilename = p.filename().string();
  auto const& filenamesWithBp =
    requestInfo->m_breakpointInfo->m_filenamesWithBp;
  if (filenamesWithBp.find(unitFilename) == filenamesWithBp.end()) {
    return;
  }

  // Resolve any unresolved breakpoints that may be in this compilation unit.
  debugger->onCompilationUnitLoaded(
    requestInfo,
    efile
  );
}

void VSDebugHook::onDefClass(const Class* cls) {
  Debugger* debugger = VSDebugExtension::getDebugger();
  if (debugger == nullptr) {
    return;
  }

  DebuggerRequestInfo* requestInfo = debugger->getRequestInfo();
  if (requestInfo == nullptr) {
    return;
  }

  if (!requestInfo->m_breakpointInfo->m_hasFuncBp) {
    return;
  }

  // Resolve any breakpoints that are set on functions in this class.
  // Acquire semantics around reading requestInfo->m_flags lock-free.
  std::atomic_thread_fence(std::memory_order_acquire);

  if (requestInfo->m_flags.unresolvedBps) {
    size_t methodCount = cls->numMethods();
    for (unsigned int i = 0; i < methodCount; i++) {
      auto func = cls->getMethod(i);
      const std::string functionName(func->fullName()->data());
      debugger->onFunctionDefined(
        requestInfo,
        func,
        functionName
      );
    }
  }
}

void VSDebugHook::onRegisterFuncIntercept(const String& name) {
  BreakContext breakContext(true);

  // This callback is invoked when a function is intercepted in HHVM.
  // Intercepts are used by things like mocking and testing frameworks
  // and some autoload infrastructure.  The debugger is interested in
  // this so that we can warn the user if a breakpoint is in a function that's
  // been detoured.

  if (breakContext.m_debugger != nullptr) {
    breakContext.m_debugger->onFuncIntercepted(
      name.toCppString()
    );
  }
}

void VSDebugHook::onDefFunc(const Func* func) {
  Debugger* debugger = VSDebugExtension::getDebugger();
  if (debugger == nullptr) {
    return;
  }

  DebuggerRequestInfo* requestInfo = debugger->getRequestInfo();
  if (requestInfo == nullptr) {
    return;
  }

  if (!requestInfo->m_breakpointInfo->m_hasFuncBp) {
    return;
  }

  // Resolve any breakpoints that are set on entry of this function.

  // Acquire semantics around reading requestInfo->m_flags lock-free.
  std::atomic_thread_fence(std::memory_order_acquire);

  if (requestInfo->m_flags.unresolvedBps) {
    std::string funcName(func->fullName()->data());
    debugger->onFunctionDefined(
      requestInfo,
      func,
      funcName
    );
  }
}

void VSDebugHook::tryEnterDebugger(
  Debugger* debugger,
  DebuggerRequestInfo* requestInfo,
  bool breakNoStepOnly
) {
  // Acquire semantics around reading requestInfo->m_flags lock-free.
  std::atomic_thread_fence(std::memory_order_acquire);

  if (requestInfo->m_flags.terminateRequest) {
    std::string message = "Request " +
      std::to_string(debugger->getCurrentThreadId()) +
      " terminating at debugger client's request.";
    debugger->sendUserMessage(
      message.c_str(),
      DebugTransport::OutputLevelLog
    );

    raise_fatal_error(
      "Request terminated by debugger client.",
      null_array,
      false,
      true,
      true
    );
  }

  if (requestInfo->m_flags.doNotBreak) {
    return;
  }

  // The first time this request enters the debugger, remove the artificial
  // memory limit since a debugger is attached.
  if (!requestInfo->m_flags.memoryLimitRemoved) {
    IniSetting::SetUser(s_memoryLimit, std::numeric_limits<int64_t>::max());
    requestInfo->m_flags.memoryLimitRemoved = true;
  }

  if (!requestInfo->m_flags.requestUrlInitialized) {
    requestInfo->m_requestUrl = g_context->getRequestUrl();
    requestInfo->m_flags.requestUrlInitialized = true;
  }

  // Check if we need to update the hook stdout and stderr for the request
  // thread.
  if (!requestInfo->m_flags.outputHooked) {
    requestInfo->m_flags.outputHooked = true;

    // In server mode, attach logging hooks to redirect stdout and stderr
    // to the debugger client. This is not needed in launch mode, because
    // the wrapper has the actual stdout and stderr pipes to use directly,
    // except for the case where we attached to an already-running script,
    // which behaves like server mode.
    bool scriptAttachMode = RuntimeOption::VSDebuggerListenPort > 0 ||
      (!RuntimeOption::ServerExecutionMode() &&
       !RuntimeOption::VSDebuggerDomainSocketPath.empty());
    if (!Debugger::hasSameTty()) {
      if (debugger->getDebuggerOptions().disableStdoutRedirection == false) {
        if (!g_context.isNull()) {
          g_context->addStdoutHook(debugger->getStdoutHook());
        }

        if (scriptAttachMode || debugger->isDummyRequest()) {
          // Attach to stderr in server mode only for the dummy thread (to show
          // any error spew from evals, etc) or in script attach mode.
          // Attaching to all requests in server mode produces way too much error
          // spew for the client. Users see stderr output for the webserver via
          // web server logs.
          Logger::SetThreadHook(debugger->getStderrHook());
        }
      }
    }
  }

  if (!breakNoStepOnly || !Debugger::isStepInProgress(requestInfo)) {
    debugger->enterDebuggerIfPaused(requestInfo);
  }

  // Install any breakpoints that have not yet been set on this request.
  if (requestInfo->m_flags.unresolvedBps) {
    debugger->tryInstallBreakpoints(requestInfo);
  }
}

const StaticString VSDebugHook::s_memoryLimit {"memory_limit"};
const StaticString VSDebugHook::s_getMsg {"getMessage"};

}
}
