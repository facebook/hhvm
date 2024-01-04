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

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/vm/act-rec.h"

#include "hphp/util/ringbuffer.h"

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
struct EventHook {
  enum {
    NormalFunc,
    Eval,
  };
  enum {
    ProfileEnters = 1,
    ProfileExits = 2,
    ProfileDefault = 3,
    ProfileFramePointers = 4,
    ProfileConstructors = 8,
    ProfileResumeAware = 16,
    /* This flag enables access to $this when profiling instance methods. It
     * is used for internal profiling tools. It *may break* in the future. */
    ProfileThisObject = 32,
    ProfileFileLine = 64,
  };
  enum class Source {
    Asio,
    Interpreter,
    Jit,
    Native,
    Unwinder
  };

  static void Enable();
  static void Disable();
  static void EnableInternal(ExecutionContext::InternalEventHookCallbackType);
  static void DisableInternal();
  static void EnableAsync();
  static void DisableAsync();
  static void EnableDebug();
  static void DisableDebug();

  static void DoMemoryThresholdCallback();

  static inline bool checkSurpriseFlagsAndIntercept(const Func* func) {
    return checkSurpriseFlags() ||
      (func->maybeIntercepted() && is_intercepted(func));
  }

  /**
   * Event hooks -- interpreter entry points.
   */
  static inline bool FunctionCall(const ActRec* ar,
                                  int funcType,
                                  EventHook::Source sourceType) {
    ringbufferEnter(ar);
    return UNLIKELY(checkSurpriseFlagsAndIntercept(ar->func()))
      ? onFunctionCall(ar, funcType, sourceType) : true;
  }
  static inline void FunctionResumeAwait(const ActRec* ar,
                                         EventHook::Source sourceType) {
    ringbufferEnter(ar);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionResumeAwait(ar, sourceType);
    }
  }
  static inline void FunctionResumeYield(const ActRec* ar,
                                         EventHook::Source sourceType) {
    ringbufferEnter(ar);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionResumeYield(ar, sourceType);
    }
  }
  static void FunctionSuspendAwaitEF(ActRec* suspending,
                                     const ActRec* resumableAR,
                                     EventHook::Source sourceType) {
    ringbufferExit(resumableAR);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionSuspendAwaitEF(suspending, resumableAR, sourceType);
    }
  }
  static void FunctionSuspendAwaitEG(ActRec* suspending,
                                     EventHook::Source sourceType) {
    ringbufferExit(suspending);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionSuspendAwaitEG(suspending, sourceType);
    }
  }
  static void FunctionSuspendAwaitR(ActRec* suspending,
                                    ObjectData* child,
                                    EventHook::Source sourceType) {
    ringbufferExit(suspending);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionSuspendAwaitR(suspending, child, sourceType);
    }
  }
  static void FunctionSuspendCreateCont(ActRec* suspending,
                                        const ActRec* resumableAR,
                                        EventHook::Source sourceType) {
    ringbufferExit(resumableAR);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionSuspendCreateCont(suspending, resumableAR, sourceType);
    }
  }
  static void FunctionSuspendYield(ActRec* suspending,
                                   EventHook::Source sourceType) {
    ringbufferExit(suspending);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionSuspendYield(suspending, sourceType);
    }
  }
  static inline void FunctionReturn(ActRec* ar,
                                    TypedValue retval,
                                    EventHook::Source sourceType) {
    ringbufferExit(ar);
    if (UNLIKELY(checkSurpriseFlags())) {
      onFunctionReturn(ar, retval, sourceType);
    }
  }
  static inline void FunctionUnwind(ActRec* ar, ObjectData* phpException) {
    ringbufferExit(ar);
    if (UNLIKELY(checkSurpriseFlags())) { onFunctionUnwind(ar, phpException); }
  }

  /**
   * Event hooks -- JIT entry points.
   */
  static uint64_t onFunctionCallJit(const ActRec* ar, int funcType);
  static void onFunctionSuspendAwaitEFJit(ActRec*, const ActRec*);
  static void onFunctionSuspendAwaitEGJit(ActRec*);
  static void onFunctionSuspendAwaitRJit(ActRec*, ObjectData*);
  static void onFunctionSuspendCreateContJit(ActRec*, const ActRec*);
  static void onFunctionSuspendYieldJit(ActRec*);
  static void onFunctionReturnJit(ActRec* ar, TypedValue retval);

private:
  enum {
    ProfileEnter,
    ProfileExit,
  };

  static bool onFunctionCall(const ActRec* ar, int funcType, EventHook::Source);
  static void onFunctionSuspendAwaitEF(ActRec*, const ActRec*, EventHook::Source);
  static void onFunctionSuspendAwaitEG(ActRec*, EventHook::Source);
  static void onFunctionSuspendAwaitR(ActRec*, ObjectData*, EventHook::Source);
  static void onFunctionSuspendCreateCont(ActRec*, const ActRec*, EventHook::Source);
  static void onFunctionSuspendYield(ActRec*, EventHook::Source);
  static void onFunctionReturn(ActRec* ar, TypedValue retval, EventHook::Source);

  static void onFunctionResumeAwait(const ActRec* ar, EventHook::Source);
  static void onFunctionResumeYield(const ActRec* ar, EventHook::Source);
  static void onFunctionUnwind(ActRec* ar, ObjectData* phpException);

  static void onFunctionEnter(const ActRec* ar, int funcType,
                              ssize_t flags, bool isResume);
  static void onFunctionExit(const ActRec* ar, const TypedValue* retval,
                             bool unwind, ObjectData* phpException,
                             size_t flags, bool isSuspend,
                             EventHook::Source);

  static bool RunInterceptHandler(ActRec* ar);

  static inline void ringbufferEnter(const ActRec* ar) {
    if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
      auto name = ar->func()->fullName();
      Trace::ringbufferMsg(name->data(), name->size(), Trace::RBTypeFuncEntry);
    }
  }
  static inline void ringbufferExit(const ActRec* ar) {
    if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
      auto name = ar->func()->fullName();
      Trace::ringbufferMsg(name->data(), name->size(), Trace::RBTypeFuncExit);
    }
  }
};

//////////////////////////////////////////////////////////////////////

}
