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
class DebuggerProxyVM;
class PhpFile;
}}

namespace HPHP {
namespace VM {

void phpDebuggerHook(const uchar* pc);
void phpExceptionHook(ObjectData* e);

void phpDebuggerEvalHook(const Func* f);
void phpBreakPointHook(Eval::DebuggerProxyVM* proxy);
void phpFileLoadHook(Eval::PhpFile* efile);

class Class;
class Func;
void phpDefClassHook(const Class* cls);
void phpDefFuncHook(const Func* func);

static inline bool isDebuggerAttached() {
  return ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData.debugger;
}

#define DEBUGGER_ATTACHED_ONLY(code) do {                             \
  if (isDebuggerAttached()) {                                         \
    code;                                                             \
  }                                                                   \
} while(0)                                                            \

#define DEBUGGER_FORCE_INTR  \
  (ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData.debuggerIntr)

#define PTRMAP_PTR_SIZE       (sizeof(void*) * 8)
#define PTRMAP_LEVEL_BITS     8LL
#define PTRMAP_LEVEL_ENTRIES  (1LL << PTRMAP_LEVEL_BITS)
#define PTRMAP_LEVEL_MASK     (PTRMAP_LEVEL_ENTRIES - 1LL)

bool isDebuggerAttachedProcess();

class PtrMapNode;
class PtrMap {
  // Radix-tree implementation of pointer map
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

class PCFilter {
private:
  PtrMap m_map;

public:
  PCFilter() {}
  int addRanges(const Unit* unit, const OffsetRangeVec& offsets);
  void addPC(const uchar* pc) {
    m_map.setPointer((void*)pc, (void*)pc);
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
