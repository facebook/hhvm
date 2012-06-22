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
#ifndef incl_VM_EVENT_HOOK_H_
#define incl_VM_EVENT_HOOK_H_

#include "runtime/base/execution_context.h"
#include "runtime/vm/bytecode.h"

namespace HPHP {
namespace VM {

#define DECLARE_HOOK(name, declargs, useargs)                              \
  static inline void name declargs {                                       \
    if (UNLIKELY(                                                          \
           ThreadInfo::s_threadInfo->m_reqInjectionData.conditionFlags)) { \
      g_vmContext->m_eventHook->on ## name useargs;                        \
    }                                                                      \
  }                                                                        \
  void on ## name declargs;

class EventHook {
 public:
  enum {
    NormalFunc,
    PseudoMain,
    Eval,
  };
  enum {
    ProfileEnter,
    ProfileExit,
  };

  static void Enable();
  static void Disable();
  static void CheckSurprise();

  DECLARE_HOOK(FunctionEnter, (const ActRec* ar, int funcType),
               (ar, funcType));
  DECLARE_HOOK(FunctionExit, (const ActRec* ar), (ar));

private:
  static void RunUserProfiler(const ActRec* ar, int mode);
};

#undef DECLARE_HOOK

} // namespace VM
} // namespace HPHP

#endif
