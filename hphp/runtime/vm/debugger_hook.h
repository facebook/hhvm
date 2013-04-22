/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <util/base.h>

namespace HPHP {
namespace Eval{
class DebuggerProxy;
class PhpFile;
}}

///////////////////////////////////////////////////////////////////////////////
// This is a set of functions which are primarily called from the VM to notify
// the debugger about various events. Some of the implemenatitons also interact
// with the VM to setup further notifications, though this is not the only place
// the debugger interacts directly with the VM.

namespace HPHP {
namespace VM {

// "Hooks" called by the VM at various points during program execution while
// debugging to give the debugger a chance to act. The debugger may block
// execution indefinetly within one of these hooks.
void phpDebuggerOpcodeHook(const uchar* pc);
void phpDebuggerExceptionHook(ObjectData* e);
void phpDebuggerEvalHook(const Func* f);
void phpDebuggerFileLoadHook(Eval::PhpFile* efile);
class Class;
class Func;
void phpDebuggerDefClassHook(const Class* cls);
void phpDebuggerDefFuncHook(const Func* func);

// Helper to apply pending breakpoints to all files.
void phpSetBreakPointsInAllFiles(Eval::DebuggerProxy* proxy);

// Add/remove breakpoints at a specific offset.
void phpAddBreakPoint(const Unit* unit, Offset offset);
void phpRemoveBreakPoint(const Unit* unit, Offset offset);

// Is this thread being debugged?
static inline bool isDebuggerAttached() {
  return ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData.debugger;
}

#define DEBUGGER_ATTACHED_ONLY(code) do {                             \
  if (isDebuggerAttached()) {                                         \
    code;                                                             \
  }                                                                   \
} while(0)                                                            \

// Is this process being debugged?
bool isDebuggerAttachedProcess();

// This flag ensures two things: first, that we stay in the interpreter and
// out of JIT code. Second, that phpDebuggerOpcodeHook will continue to allow
// debugger interrupts for every opcode executed (modulo filters.)
#define DEBUGGER_FORCE_INTR  \
  (ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData.debuggerIntr)

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

  // Add/remove offsets, either individually or by range.
  int addRanges(const Unit* unit, const OffsetRangeVec& offsets);
  void removeOffset(const Unit* unit, Offset offset);

  // Add/remove/check explicit PCs.
  void addPC(const uchar* pc) {
    m_map.setPointer((void*)pc, (void*)pc);
  }
  void removePC(const uchar* pc) {
    m_map.setPointer((void*)pc, nullptr);
  }
  bool checkPC(const uchar* pc) {
    return m_map.getPointer((void*)pc) == (void*)pc;
  }

  void clear() {
    m_map.clear();
  }
};

}}      // namespace HPHP::VM

#endif /* incl_HPHP_DEBUGGER_HOOK_H_ */
