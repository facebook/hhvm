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
#ifndef incl_HPHP_VM_EVENT_HOOK_H_
#define incl_HPHP_VM_EVENT_HOOK_H_

#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/surprise-flags.h"

#include <atomic>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * Event hooks.
 *
 * All hooks can throw because of multiple possible reasons, such as:
 *  - user-defined signal handlers
 *  - pending destructor exceptions
 *  - pending out of memory exceptions
 *  - pending timeout exceptions
 */
class EventHook {
 public:
  enum {
    NormalFunc,
    PseudoMain,
    Eval,
  };

  static void Enable();
  static void Disable();
  static void EnableAsync();
  static void DisableAsync();
  static void EnableDebug();
  static void DisableDebug();
  static void EnableIntercept();
  static void DisableIntercept();

  /**
   * Event hooks -- interpreter entry points.
   */
  static inline bool FunctionCall(const ActRec* ar, int funcType) {
    ringbufferEnter(ar);
    return UNLIKELY(checkSurpriseFlags())
      ? onFunctionCall(ar, funcType) : true;
  }
  static inline void FunctionResumeAwait(const ActRec* ar) {
    ringbufferEnter(ar);
    if (UNLIKELY(checkSurpriseFlags())) { onFunctionResumeAwait(ar); }
  }
  static inline void FunctionResumeYield(const ActRec* ar) {
    ringbufferEnter(ar);
    if (UNLIKELY(checkSurpriseFlags())) { onFunctionResumeYield(ar); }
  }
  static void FunctionSuspendE(ActRec* suspending, const ActRec* resumableAR) {
    ringbufferExit(resumableAR);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionSuspendE(suspending, resumableAR);
    }
  }
  static void FunctionSuspendR(ActRec* suspending, ObjectData* child) {
    ringbufferExit(suspending);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionSuspendR(suspending, child);
    }
  }
  static inline void FunctionReturn(ActRec* ar, TypedValue retval) {
    ringbufferExit(ar);
    if (UNLIKELY(checkSurpriseFlags())) { onFunctionReturn(ar, retval); }
  }
  static inline void FunctionUnwind(ActRec* ar, ObjectData* phpException) {
    ringbufferExit(ar);
    if (UNLIKELY(checkSurpriseFlags())) { onFunctionUnwind(ar, phpException); }
  }

  /**
   * Event hooks -- JIT entry points.
   */
  static bool onFunctionCall(const ActRec* ar, int funcType);
  static void onFunctionSuspendE(ActRec*, const ActRec*);
  static void onFunctionSuspendR(ActRec*, ObjectData*);
  static void onFunctionReturn(ActRec* ar, TypedValue retval);

private:
  enum {
    ProfileEnter,
    ProfileExit,
  };

  static void onFunctionResumeAwait(const ActRec* ar);
  static void onFunctionResumeYield(const ActRec* ar);
  static void onFunctionUnwind(ActRec* ar, ObjectData* phpException);

  static void onFunctionEnter(const ActRec* ar, int funcType, ssize_t flags);
  static void onFunctionExit(const ActRec* ar, const TypedValue* retval,
                             bool unwind, ObjectData* phpException,
                             size_t flags);

  static bool RunInterceptHandler(ActRec* ar);
  static const char* GetFunctionNameForProfiler(const Func* func,
                                                int funcType);

  static inline void ringbufferEnter(const ActRec* ar) {
    if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
      auto name = ar->m_func->fullName();
      Trace::ringbufferMsg(name->data(), name->size(), Trace::RBTypeFuncEntry);
    }
  }
  static inline void ringbufferExit(const ActRec* ar) {
    if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
      auto name = ar->m_func->fullName();
      Trace::ringbufferMsg(name->data(), name->size(), Trace::RBTypeFuncExit);
    }
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif
