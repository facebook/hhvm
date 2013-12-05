/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_VM_EVENT_HOOK_H_
#define incl_HPHP_VM_EVENT_HOOK_H_

#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/base/rds.h"

#include <atomic>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline bool checkConditionFlags() {
  return RDS::header()->conditionFlags.load(std::memory_order_acquire);
}

//////////////////////////////////////////////////////////////////////

class EventHook {
 public:
  enum {
    NormalFunc,
    PseudoMain,
    Eval,
  };

  static void Enable();
  static void Disable();
  static void EnableIntercept();
  static void DisableIntercept();
  static ssize_t CheckSurprise();

  /*
   * Can throw from user-defined signal handlers, or OOM or timeout
   * exceptions.
   */
  static bool onFunctionEnter(const ActRec* ar, int funcType);
  static inline bool FunctionEnter(const ActRec* ar, int funcType) {
    if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
      auto name = ar->m_func->fullName();
      Trace::ringbufferMsg(name->data(), name->size(), Trace::RBTypeFuncEntry);
    }
    if (UNLIKELY(checkConditionFlags())) {
      return onFunctionEnter(ar, funcType);
    }
    return true;
  }

  /*
   * FunctionExit may throw.
   *
   * This means we have to be extra careful when tearing down frames
   * (which might be because an exception is propagating).  The
   * unwinder itself will call the function exit hooks and swallow
   * exceptions.
   */
  static void onFunctionExit(const ActRec* ar);
  static inline void FunctionExit(const ActRec* ar) {
    if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
      auto name = ar->m_func->fullName();
      Trace::ringbufferMsg(name->data(), name->size(), Trace::RBTypeFuncExit);
    }
    if (UNLIKELY(checkConditionFlags())) {
      onFunctionExit(ar);
    }
  }

private:
  enum {
    ProfileEnter,
    ProfileExit,
  };

  static void RunUserProfiler(const ActRec* ar, int mode);
  static bool RunInterceptHandler(ActRec* ar);
  static const char* GetFunctionNameForProfiler(const ActRec* ar,
                                                int funcType);
};

//////////////////////////////////////////////////////////////////////

}

#endif
