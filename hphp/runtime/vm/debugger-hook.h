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
struct Unit;
struct ObjectData;

// Is this thread being debugged?
inline bool isDebuggerAttached(ThreadInfo* ti = nullptr) {
  ti = (ti != nullptr) ? ti : ThreadInfo::s_threadInfo.getNoCheck();
  return ti->m_reqInjectionData.getDebuggerAttached();
}

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
  // TODO(#4489053) This should create the pc filters
  template<class HandlerClass>
  static bool attach(ThreadInfo* ti = nullptr) {
    ti = (ti != nullptr) ? ti : ThreadInfo::s_threadInfo.getNoCheck();
    if (isDebuggerAttached(ti)) {
      return false;
    }

    s_numAttached++;
    ti->m_reqInjectionData.setDebuggerAttached(true);
    ti->m_debugHookHandler = new HandlerClass();

    // Event hooks need to be enabled to receive function entry and exit events.
    // This comes at the cost of a small bit of performance, however, it makes
    // the code dealing with line breakpoints much easier. Essentially the
    // problem is ensuring we only break on a line breakpoint once. We can't
    // just disable the breakpoint until we leave the site because some opcode
    // in the site could recurse to the site. So a disable must be attached to
    // a stack depth. This will be disabled on call to detach().
    EventHook::EnableDebug();

    return true;
  }

  // If a handler is attached to the thread, detaches it
  // TODO(#4489053) This should clear the pc filters
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

  // Called when we have hit a registered function entry breakpoint
  virtual void onFuncEntryBreak(const Func* f) {}

  // Called when we have hit a registered function exit breakpoint
  virtual void onFuncExitBreak(const Func* f) {}

  // Called when we have hit a registered line breakpoint. Even though a line
  // spans multiple opcodes, this will only be called once per hit.
  virtual void onLineBreak(const Unit* unit, int line) {}

  // The number of DebugHookHandlers that are currently attached to the process.
  static std::atomic_int s_numAttached;
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

#define DEBUGGER_ATTACHED_ONLY(code) do {                             \
  if (isDebuggerAttached()) {                                         \
    code;                                                             \
  }                                                                   \
} while(0)                                                            \

// Flag that can be set by the client to force interrupts
#define DEBUGGER_INTR \
  (ThreadInfo::s_threadInfo->m_reqInjectionData.getDebuggerIntr())

// Whether or not there we are over an active line break
#define DEBUGGER_ACTIVE_LINE_BREAK \
  (ThreadInfo::s_threadInfo->m_reqInjectionData.getActiveLineBreak() != -1)

// This flag ensures two things: first, that we stay in the interpreter and
// out of JIT code. Second, that phpDebuggerOpcodeHook will continue to allow
// debugger interrupts for every opcode executed (modulo filters.)
#define DEBUGGER_FORCE_INTR \
  (DEBUGGER_INTR || DEBUGGER_ACTIVE_LINE_BREAK)

// "Hooks" called by the VM at various points during program execution while
// debugging to give the debugger a chance to act. The debugger may block
// execution indefinitely within one of these hooks.
void phpDebuggerOpcodeHook(const unsigned char* pc);
void phpDebuggerFuncEntryHook(const ActRec* ar);
void phpDebuggerFuncExitHook(const ActRec* ar);
void phpDebuggerExceptionThrownHook(ObjectData* exception);
void phpDebuggerExceptionHandlerHook();
void phpDebuggerErrorHook(const ExtendedException &ee,
                          int errnum,
                          const std::string& message);
void phpDebuggerEvalHook(const Func* f);
void phpDebuggerFileLoadHook(Unit* efile);
void phpDebuggerDefClassHook(const Class* cls);
void phpDebuggerDefFuncHook(const Func* func);

// Add breakpoints of various types
void phpAddBreakPoint(const Unit* unit, Offset offset);
void phpAddBreakPointRange(const Unit* unit, OffsetRangeVec& offsets);
void phpAddBreakPointFuncEntry(const Func* f);
void phpAddBreakPointFuncExit(const Func* f);
// Returns false if the line is invalid
bool phpAddBreakPointLine(const Unit* unit, int line);

// FIXME: Internally, there is no distinction between the types of breakpoints,
// so there is no good way to remove added breakpoints. Furthermore, there is
// no exposed undoing of the JIT opcode blacklisting, so there is little benefit
// to removal (as opposed to debug hook handlers ignoring deleted breakpoints).
void phpRemoveBreakPoint(const Unit* unit, Offset offset);

bool phpHasBreakpoint(const Unit* unit, Offset offset);

}

#endif
