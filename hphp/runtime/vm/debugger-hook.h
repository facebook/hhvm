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
  template<class HandlerClass>
  static bool attach(ThreadInfo* ti = nullptr) {
    ti = (ti != nullptr) ? ti : ThreadInfo::s_threadInfo.getNoCheck();
    if (isDebuggerAttached(ti)) {
      return false;
    }

    s_numAttached++;
    ti->m_reqInjectionData.setDebuggerAttached(true);
    ti->m_debugHookHandler = new HandlerClass();
    return true;
  }

  // If a handler is attached to the thread, detaches it
  static void detach(ThreadInfo* ti = nullptr);

  // Debugger events. Subclasses can override these methods to receive
  // events.
  virtual void onOpcode(const unsigned char* pc) {}
  virtual void onExceptionThrown(ObjectData* exception) {}
  virtual void onExceptionHandle() {}
  virtual void onError(const ExtendedException &ee,
                       int errnum,
                       const std::string& message) {}
  virtual void onEval(const Func* f) {}
  virtual void onFileLoad(Unit* efile) {}
  virtual void onDefClass(const Class* cls) {}
  virtual void onDefFunc(const Func* func) {}

  // The number of DebugHookHandlers that are currently attached to the process.
  static std::atomic_int s_numAttached;
};

// Returns the current hook handler
inline DebugHookHandler* getHookHandler() {
  return ThreadInfo::s_threadInfo.getNoCheck()->m_debugHookHandler;
}

// Is this process being debugged?
inline bool isDebuggerAttachedProcess() {
  return DebugHookHandler::s_numAttached > 0;
}

#define DEBUGGER_ATTACHED_ONLY(code) do {                             \
  if (isDebuggerAttached()) {                                         \
    code;                                                             \
  }                                                                   \
} while(0)                                                            \

// This flag ensures two things: first, that we stay in the interpreter and
// out of JIT code. Second, that phpDebuggerOpcodeHook will continue to allow
// debugger interrupts for every opcode executed (modulo filters.)
#define DEBUGGER_FORCE_INTR  \
  (ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData.getDebuggerIntr())

// "Hooks" called by the VM at various points during program execution while
// debugging to give the debugger a chance to act. The debugger may block
// execution indefinitely within one of these hooks.
void phpDebuggerOpcodeHook(const unsigned char* pc);
void phpDebuggerExceptionThrownHook(ObjectData* exception);
void phpDebuggerExceptionHandlerHook();
void phpDebuggerErrorHook(const ExtendedException &ee,
                          int errnum,
                          const std::string& message);
void phpDebuggerEvalHook(const Func* f);
void phpDebuggerFileLoadHook(Unit* efile);
void phpDebuggerDefClassHook(const Class* cls);
void phpDebuggerDefFuncHook(const Func* func);

// Add/remove breakpoints
void phpAddBreakPoint(const Unit* unit, Offset offset);
void phpAddBreakPointRange(const Unit* unit, OffsetRangeVec& offsets);
void phpAddBreakPointFuncEntry(const Func* f);
void phpRemoveBreakPoint(const Unit* unit, Offset offset);
bool phpHasBreakpoint(const Unit* unit, Offset offset);

// Map which holds a set of PCs and supports reasonably fast addition and
// lookup. Used by the debugger to decide if a given PC falls within an
// interesting area, e.g., for breakpoints and stepping.
class PCFilter {
private:
  // Radix-tree implementation of pointer map
  struct PtrMapNode;
  class PtrMap {
#define PTRMAP_PTR_SIZE       (sizeof(void*) * 8)
#define PTRMAP_LEVEL_BITS     8LL
#define PTRMAP_LEVEL_ENTRIES  (1LL << PTRMAP_LEVEL_BITS)
#define PTRMAP_LEVEL_MASK     (PTRMAP_LEVEL_ENTRIES - 1LL)

  public:
    PtrMap() {
      static_assert(PTRMAP_PTR_SIZE % PTRMAP_LEVEL_BITS == 0,
                    "PTRMAP_PTR_SIZE must be a multiple of PTRMAP_LEVEL_BITS");
      m_root = MakeNode();
    }
    ~PtrMap();
    void setPointer(void* ptr, void* val);
    void* getPointer(void* ptr);
    void clear();

  private:
    PtrMapNode* m_root;
    static PtrMapNode* MakeNode();
  };

  PtrMap m_map;

public:
  PCFilter() {}

  // Filter function to exclude opcodes when adding ranges.
  typedef std::function<bool(Op)> OpcodeFilter;

  // Add/remove offsets, either individually or by range. By default allow all
  // opcodes.
  void addRanges(const Unit* unit, const OffsetRangeVec& offsets,
                 OpcodeFilter isOpcodeAllowed = [] (Op) { return true; });
  void removeOffset(const Unit* unit, Offset offset);

  // Add/remove/check explicit PCs.
  void addPC(const unsigned char* pc) {
    m_map.setPointer((void*)pc, (void*)pc);
  }
  void removePC(const unsigned char* pc) {
    m_map.setPointer((void*)pc, nullptr);
  }
  bool checkPC(const unsigned char* pc) {
    return m_map.getPointer((void*)pc) == (void*)pc;
  }

  void clear() {
    m_map.clear();
  }
};

}

#endif
