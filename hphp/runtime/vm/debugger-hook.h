/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_DEBUGGER_HOOK_H_
#define incl_HPHP_DEBUGGER_HOOK_H_

#include <functional>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/unit.h"  // OffsetRangeVec
#include "hphp/runtime/base/thread-info.h"

///////////////////////////////////////////////////////////////////////////////
// This is a set of functions which are primarily called from the VM to notify
// the debugger about various events. Some of the implementations also interact
// with the VM to setup further notifications, though this is not the only place
// the debugger interacts directly with the VM.

namespace HPHP {

struct Class;
struct Func;
struct ObjectData;

// Is this thread being debugged?
inline bool isDebuggerAttached(ThreadInfo* ti = nullptr) {
  ti = (ti != nullptr) ? ti : ThreadInfo::s_threadInfo.getNoCheck();
  return ti->m_reqInjectionData.getDebuggerAttached();
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
#define DEBUGGER_FORCE_INTR \
  (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebuggerForceIntr())

////////////////////////////////////////////////////////////////////////////////
// Hook Handler
// A handler for debugger events. Any extension can subclass this class and
// attach it to the thread in order to receive debugging events.
class DebugHookHandler {
public:
  DebugHookHandler() {}
  virtual ~DebugHookHandler() {}

  // Attempts to attach an instance of the given debug hook handler to the
  // passed thread. If no thread info is passed, the current one is used.
  // The template parameter should be an instance of DebugHookHandler.
  // Returns true on success, false on failure (for instance, if another debug
  // hook handler is already attached).
  template<class HandlerClass>
  static bool attach(ThreadInfo* ti = nullptr) {
    ti = (ti != nullptr) ? ti : ThreadInfo::s_threadInfo.getNoCheck();
    if (isDebuggerAttached(ti)) {
      return false;
    }

    // Attach to the thread
    ti->m_debugHookHandler = HandlerClass::GetInstance();

    // Increment the number of attached handlers
    {
      Lock lock(s_lock);
      s_numAttached++;
      ti->m_reqInjectionData.setDebuggerAttached(true);
    }

    // Event hooks need to be enabled to receive function entry and exit events.
    // This comes at the cost of a small bit of performance, however, it makes
    // the code dealing with line breakpoints much easier. Essentially the
    // problem is ensuring we only break on a line breakpoint once. We can't
    // just disable the breakpoint until we leave the site because some opcode
    // in the site could recurse to the site. So a disable must be attached to
    // a stack depth. This will be disabled on call to detach().
    ti->m_reqInjectionData.setDebuggerHookFlag();

    return true;
  }

  // If a handler is attached to the thread, detaches it
  static void detach(ThreadInfo* ti = nullptr);

  // Debugger events. Subclasses can override these methods to receive
  // events.
  virtual void onExceptionThrown(ObjectData* exception) {}
  virtual void onExceptionHandle() {}
  virtual void onError(const ExtendedException &ee,
                       int errnum,
                       const std::string& message) {}
  virtual void onEval(const Func* f) {}
  virtual void onFileLoad(Unit* efile) {}
  virtual void onDefClass(const Class* cls) {}
  virtual void onDefFunc(const Func* func) {}

  // Called whenever the program counter is at a location that could be
  // interesting to a debugger. Such as when have hit a registered breakpoint
  // (regardless of type), when interrupt forcing is enabled, or when the pc is
  // over an active line breakpoint
  virtual void onOpcode(const unsigned char* pc) {}

  // Called right before top-level pseudo-main enters and right after it
  // exits. This is useful for debuggers to initialize and shutdown separate
  // from an extension as they can assume other extensions are initialized.
  virtual void onRequestInit() {}
  virtual void onRequestShutdown() {}

  // Called whenever we are breaking due to completion of a step in or step out
  virtual void onStepInBreak(const Unit* unit, int line) {}
  virtual void onStepOutBreak(const Unit* unit, int line) {}
  virtual void onNextBreak(const Unit* unit, int line) {}

  // Called when we have hit a registered function entry breakpoint
  virtual void onFuncEntryBreak(const Func* f) {}

  // Called when we have hit a registered function exit breakpoint
  virtual void onFuncExitBreak(const Func* f) {}

  // Called when we have hit a registered line breakpoint. Even though a line
  // spans multiple opcodes, this will only be called once per hit.
  virtual void onLineBreak(const Unit* unit, int line) {}

  // The number of DebugHookHandlers that are currently attached to the process.
  // The mutex is needed because we need to perform work when we are sure there
  // are no handlers attached
  static Mutex s_lock;
  static int s_numAttached;
};

// Returns the current hook handler
inline DebugHookHandler* getHookHandler() {
  return ThreadInfo::s_threadInfo.getNoCheck()->m_debugHookHandler;
}

// Is this process being debugged? Since this is across all threads, this cannot
// be counted on to be accurate.
inline bool isDebuggerAttachedProcess() {
  return DebugHookHandler::s_numAttached > 0;
}

////////////////////////////////////////////////////////////////////////////////
// Hooks
// Called by the VM at various points during program execution while
// debugging to give the debugger a chance to act. The debugger may block
// execution indefinitely within one of these hooks.
void phpDebuggerOpcodeHook(const unsigned char* pc);
void phpDebuggerRequestInitHook();
void phpDebuggerRequestShutdownHook();
void phpDebuggerFuncEntryHook(const ActRec* ar);
void phpDebuggerFuncExitHook(const ActRec* ar);
void phpDebuggerExceptionThrownHook(ObjectData* exception);
void phpDebuggerExceptionHandlerHook() noexcept;
void phpDebuggerErrorHook(const ExtendedException &ee,
                          int errnum,
                          const std::string& message);
void phpDebuggerEvalHook(const Func* f);
void phpDebuggerFileLoadHook(Unit* efile);
void phpDebuggerDefClassHook(const Class* cls);
void phpDebuggerDefFuncHook(const Func* func);

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
void phpAddBreakPoint(const Unit* unit, Offset offset);
void phpAddBreakPointRange(const Unit* unit, OffsetRangeVec& offsets);
void phpAddBreakPointFuncEntry(const Func* f);
void phpAddBreakPointFuncExit(const Func* f);
// Returns false if the line is invalid
bool phpAddBreakPointLine(const Unit* unit, int line);

// Breakpoint removal functions.
// FIXME Note that internally there is a global PCFilter for all breakpoints.
// This is checked against every opcode to determine if we should break. While
// good for performance, this allows no distinction between the types of
// breakpoints. That is, we can never remove breakpoints from the
// global filter because there could be overlap.
//
// If the new style of breakpoints is used (function breakpoints and lines),
// we can at least prevent the appropriate breakpoint hooks from being called.
// This means once added, we will always interrupt on that breakpoint until
// the hook handler is detached. This isn't a huge deal as it just means we
// can't jit those opcodes, but it should be fixed since it's just a design
// issue.
void phpRemoveBreakPoint(const Unit* unit, Offset offset);
void phpRemoveBreakPointFuncEntry(const Func* f);
void phpRemoveBreakPointFuncExit(const Func* f);
void phpRemoveBreakPointLine(const Unit* unit, int line);

bool phpHasBreakpoint(const Unit* unit, Offset offset);

}

#endif
