/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <functional>

#include "hphp/runtime/base/configs/jit.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/unit.h"  // OffsetRangeVec

#include "hphp/runtime/ext/vsdebug/break_mode.h"

///////////////////////////////////////////////////////////////////////////////
// This is a set of functions which are primarily called from the VM to notify
// the debugger about various events. Some of the implementations also interact
// with the VM to setup further notifications, though this is not the only place
// the debugger interacts directly with the VM.

namespace HPHP {

namespace Eval { struct HphpdHook; }

struct Class;
struct DebuggerHook;
struct Func;
struct ObjectData;

// Is this thread being debugged?
inline bool isDebuggerAttached(RequestInfo* ti = nullptr) {
  ti = (ti != nullptr) ? ti : &RI();
  return ti->m_reqInjectionData.getDebuggerAttached();
}

inline bool disableJitAtAttach(RequestInjectionData& rid) {
  return Cfg::Jit::DisabledByVSDebug &&
    !rid.m_breakPointFilter.isNull();
}

inline bool disableJitForDebuggerCli() {
  return Cfg::Jit::DisabledByVSDebug && is_any_cli_mode();
}

// Executes the passed code only if there is a debugger attached to the current
// thread.
#define DEBUGGER_ATTACHED_ONLY(code) do {                             \
  if (isDebuggerAttached()) {                                         \
    code;                                                             \
  }                                                                   \
} while(0)                                                            \

// This flag ensures two things: first, that we stay in the interpreter and
// out of JIT code. Second, that phpDebuggerOpcodeHook will continue to allow
// debugger interrupts for every opcode executed (modulo filters.)
#define DEBUGGER_FORCE_INTR (RID().getDebuggerForceIntr())

// Initialize a RDS handle indicating function f needs to run in the interpreter
// so that the debugger hooks are run.
void markFunctionWithDebuggerIntr(const Func* f);

enum class StackDepthDisposition {
  Equal,        // Same.
  Shallower,    // Less than baseline.
  Deeper,       // Greater than baseline.
};

////////////////////////////////////////////////////////////////////////////////
// DebuggerHook
// A hook for debugger events.  Any extension can subclass this class and
// attach it to the thread in order to receive debugging events.
struct DebuggerHook {
  DebuggerHook() {}
  virtual ~DebuggerHook() {}

  // Attempts to attach an instance of the given debugger hook to the passed
  // thread.  If no thread info is passed, the current one is used.  The
  // template parameter should be an instance of DebuggerHook.  Returns true on
  // success, false on failure (for instance, if another debugger hook is
  // already attached).
  template<class HookClass>
  static bool attach(RequestInfo* ti = nullptr) {
    ti = (ti != nullptr) ? ti : &RI();

    if (isDebuggerAttached(ti)) {
      // Check if this debugger hook is already attached.
      // TODO: Ideally this wouldn't be necessary here, debuggers should
      // be well behaved and only attach once per thread. There is at least
      // one instance where hphpd still needs this. Remove once that's
      // cleaned up.
      return dynamic_cast<HookClass*>(ti->m_debuggerHook) != nullptr;
    }

    // Increment the number of attached hooks.
    {
      Lock lock(s_lock);
      auto instance = HookClass::GetInstance();

      // Once a debugger has attached to any request, it is the only debugger
      // allowed to attach to subsequent requests until it has detached from
      // all requests.
      if (s_activeHook == nullptr) {
        s_activeHook = instance;
      } else if (s_activeHook != instance) {
        return false;
      }

      // Attach to the thread
      ti->m_debuggerHook = instance;

      s_numAttached++;
      ti->m_reqInjectionData.setDebuggerAttached(true);
      if (disableJitAtAttach(ti->m_reqInjectionData) ||
          disableJitForDebuggerCli()) {
        ti->m_reqInjectionData.setJittingDisabled(true);
        rl_typeProfileLocals->forceInterpret = true;
        ti->m_reqInjectionData.updateJit();
      }

    }

    // Event hooks need to be enabled to receive function entry and exit events.
    // This comes at the cost of a small bit of performance, however, it makes
    // the code dealing with line breakpoints much easier. Essentially the
    // problem is ensuring we only break on a line breakpoint once. We can't
    // just disable the breakpoint until we leave the site because some opcode
    // in the site could recurse to the site. So a disable must be attached to
    // a stack depth. This will be disabled on call to detach().
    ti->m_reqInjectionData.setFlag(DebuggerHookFlag);

    return true;
  }

  // Attempts to set or remove the specified debugger hook as the "active" hook.
  // The active hook is the only hook that is permitted to attach to any request
  static bool setActiveDebuggerInstance(DebuggerHook* hook, bool attach) {
    Lock lock(s_lock);
    if (attach) {
      if (s_activeHook != nullptr) {
        return s_activeHook == hook;
      }

      s_activeHook = hook;
      return true;
    } else {
      if (s_activeHook != hook) {
        return false;
      }

      s_activeHook = nullptr;
      return true;
    }
  }

  // If a hook is attached to the thread, detaches it.
  static void detach(RequestInfo* ti = nullptr);

  // Debugger events. Subclasses can override these methods to receive
  // events.
  virtual void onExceptionThrown(ObjectData* /*exception*/) {}
  virtual void onExceptionHandle() {}
  virtual void onError(const ExtendedException& /*ee*/, int /*errnum*/,
                       const std::string& /*message*/) {}
  virtual void onEval(const Func* /*f*/) {}
  virtual void onFileLoad(Unit* /*efile*/) {}
  virtual void onDefClass(const Class* /*cls*/) {}
  virtual void onDefFunc(const Func* /*func*/) {}
  virtual void onRegisterFuncIntercept(const String& /*name*/) {}

  // Called whenever the program counter is at a location that could be
  // interesting to a debugger. Such as when have hit a registered breakpoint
  // (regardless of type), when interrupt forcing is enabled, or when the pc is
  // over an active line breakpoint
  virtual void onOpcode(const unsigned char* /*pc*/) {}

  // Called right before top-level pseudo-main enters. This may be useful for
  // debuggers to initialize separate from an extension as they can assume
  // other extensions are initialized.
  virtual void onRequestInit() {}

  // Called whenever we are breaking due to completion of a step in or step out
  virtual void onStepInBreak(const Unit* /*unit*/, int /*line*/) {}
  virtual void onStepOutBreak(const Unit* /*unit*/, int /*line*/) {}
  virtual void onNextBreak(const Unit* /*unit*/, int /*line*/) {}

  // Called when we have hit a registered function entry breakpoint
  virtual void onFuncEntryBreak(const Func* /*f*/) {}

  // Called when we have hit a registered function exit breakpoint
  virtual void onFuncExitBreak(const Func* /*f*/) {}

  // Called when we have hit a registered line breakpoint. Even though a line
  // spans multiple opcodes, this will only be called once per hit.
  virtual void onLineBreak(const Unit* /*unit*/, int /*line*/) {}

  // The number of DebuggerHooks that are currently attached to the process.
  // The mutex is needed because we need to perform work when we are sure there
  // are no hooks attached.
  static Mutex s_lock;
  static int s_numAttached;
  static DebuggerHook* s_activeHook;
  static rds::Link<bool, rds::Mode::Normal> s_exceptionBreakpointIntr;
};

// Returns the current hook.
inline DebuggerHook* getDebuggerHook() {
  return RI().m_debuggerHook;
}

// Is this process being debugged? Since this is across all threads, this cannot
// be counted on to be accurate.
inline bool isDebuggerAttachedProcess() {
  return DebuggerHook::s_numAttached > 0;
}

////////////////////////////////////////////////////////////////////////////////
// Hooks
// Called by the VM at various points during program execution while
// debugging to give the debugger a chance to act. The debugger may block
// execution indefinitely within one of these hooks.
void phpDebuggerOpcodeHook(const unsigned char* pc);
void phpDebuggerRequestInitHook();
void phpDebuggerFuncEntryHook(const ActRec* ar, bool isResume);
void phpDebuggerFuncExitHook(const ActRec* ar, bool isSuspend);
void phpDebuggerExceptionThrownHook(ObjectData* exception);
void phpDebuggerExceptionHandlerHook() noexcept;
void phpDebuggerErrorHook(const ExtendedException& ee,
                          int errnum,
                          const std::string& message);
void phpDebuggerEvalHook(const Func* f);
void phpDebuggerFileLoadHook(Unit* efile);
void phpDebuggerDefClassHook(const Class* cls);
void phpDebuggerDefFuncHook(const Func* func);
void phpDebuggerInterceptRegisterHook(const String& name);

////////////////////////////////////////////////////////////////////////////////
// Flow commands
// Commands manipulating control flow. Calling any one of these short-cicruits
// the others.

// Continues execution until the next breakpoint or program exit
void phpDebuggerContinue();

// Steps a single line, stepping into functions if necessary. If the current
// site is invalid, the break will occur on the next valid opcode.
void phpDebuggerStepIn();

// Steps until the current function returns. Breaks on the opcode following the
// return site.
void phpDebuggerStepOut();

// Steps a single line, stepping over functions if necessary. If the current
// site is invalid, the break will occur on the next valid opcode.
void phpDebuggerNext();

////////////////////////////////////////////////////////////////////////////////
// Breakpoint manipulation

// Add breakpoints of various types
void phpAddBreakPoint(const Func* f, Offset offset);
void phpAddBreakPointFuncEntry(const Func* f);
void phpAddBreakPointFuncExit(const Func* f);
// Returns false if the line is invalid
bool phpAddBreakPointLine(const Unit* u, int line);

void phpSetExceptionBreakpoint(VSDEBUG::ExceptionBreakMode mode);

// Breakpoint removal functions.
// FIXME Note that internally there is a global PCFilter for all breakpoints.
// This is checked against every opcode to determine if we should break. While
// good for performance, this allows no distinction between the types of
// breakpoints. That is, we can never remove breakpoints from the
// global filter because there could be overlap.
//
// If the new style of breakpoints is used (function breakpoints and lines), we
// can at least prevent the appropriate breakpoint hooks from being called.
// This means once added, we will always interrupt on that breakpoint until the
// hook is detached.  This isn't a huge deal as it just means we can't jit those
// opcodes, but it should be fixed since it's just a design issue.
void phpRemoveBreakPoint(const Func* f, Offset offset);
void phpRemoveBreakPointFuncEntry(const Func* f);
void phpRemoveBreakPointFuncExit(const Func* f);
void phpRemoveBreakPointLine(const Unit* u, int line);

bool phpHasBreakpoint(const Func* f, Offset offset);

StackDepthDisposition getStackDisposition(int baseline);

PCFilter* getBreakPointFilter();
PCFilter* getFlowFilter();

String getCurrentFilePath(int* pLine);

}
