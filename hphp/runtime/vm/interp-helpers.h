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

#ifndef incl_HPHP_VM_INTERP_HELPERS_H_
#define incl_HPHP_VM_INTERP_HELPERS_H_

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/rx.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

namespace HPHP {

inline void callerReffinessChecks(const Func* func, const FCallArgs& fca) {
  for (auto i = 0; i < fca.numArgs; ++i) {
    auto const byRef = func->byRef(i);
    if (byRef != fca.byRef(i)) {
      SystemLib::throwInvalidArgumentExceptionObject(
        formatParamRefMismatch(func->fullDisplayName()->data(), i, byRef));
    }
  }
}

/*
 * Check if a dynamic call to `func` is allowed. Return true if it is, otherwise
 * raise a notice and return false or raise an exception.
 */
inline bool callerDynamicCallChecks(const Func* func) {
  int dynCallErrorLevel = func->isMethod() ?
    (
      func->isStatic() ?
        RuntimeOption::EvalForbidDynamicCallsToClsMeth :
        RuntimeOption::EvalForbidDynamicCallsToInstMeth
    ) :
    RuntimeOption::EvalForbidDynamicCallsToFunc;
  if (dynCallErrorLevel <= 0) return true;
  if (func->isDynamicallyCallable()) return true;

  if (dynCallErrorLevel >= 2) {
    std::string msg;
    string_printf(
      msg,
      Strings::FUNCTION_CALLED_DYNAMICALLY,
      func->fullDisplayName()->data()
    );
    throw_invalid_operation_exception(makeStaticString(msg));
  } else {
    raise_notice(
      Strings::FUNCTION_CALLED_DYNAMICALLY,
      func->fullDisplayName()->data()
    );
    return false;
  }
}

inline void callerDynamicConstructChecks(const Class* cls) {
  if (RuntimeOption::EvalForbidDynamicConstructs <= 0) return;
  if (cls->isDynamicallyConstructible()) return;

  if (RuntimeOption::EvalForbidDynamicConstructs >= 2) {
    std::string msg;
    string_printf(
      msg,
      Strings::CLASS_CONSTRUCTED_DYNAMICALLY,
      cls->name()->data()
    );
    throw_invalid_operation_exception(makeStaticString(msg));
  } else {
    raise_notice(
      Strings::CLASS_CONSTRUCTED_DYNAMICALLY,
      cls->name()->data()
    );
  }
}

inline void calleeDynamicCallChecks(const ActRec* ar) {
  if (!ar->isDynamicCall()) return;
  auto const func = ar->func();

  if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && func->isBuiltin()) {
    raise_notice(
      Strings::FUNCTION_CALLED_DYNAMICALLY,
      func->fullDisplayName()->data()
    );
  }
}

/*
 * Check if a call from `caller` to `callee` satisfies reactivity constraints.
 * Returns true if yes, otherwise raise a warning and return false or raise
 * an exception.
 */
inline bool callerRxChecks(const ActRec* caller, const Func* callee) {
  if (RuntimeOption::EvalRxEnforceCalls <= 0) return true;
  // Conditional reactivity is not tracked yet, so assume the caller has minimum
  // and the callee has maximum possible level of reactivity.
  auto const callerLevel = caller->rxMinLevel();
  if (!rxEnforceCallsInLevel(callerLevel)) return true;

  auto const minReqCalleeLevel = rxRequiredCalleeLevel(callerLevel);
  if (LIKELY(callee->rxLevel() >= minReqCalleeLevel)) return true;
  raiseRxCallViolation(caller, callee);
  return false;
}

inline void checkForRequiredCallM(const ActRec* ar) {
  if (!ar->func()->takesInOutParams()) return;

  if (!ar->isFCallM()) {
    raise_error("In/out function called dynamically without inout annotations");
  }
}

/*
 * This helper only does a stack overflow check for the native stack.
 * Both native and VM stack overflows are independently possible.
 */
inline void checkNativeStack() {
  // Check whether we're going out of bounds of our native stack.
  if (LIKELY(stack_in_bounds())) return;
  TRACE_MOD(Trace::gc, 1, "Maximum stack depth exceeded.\n");
  raise_error("Stack overflow");
}

/*
 * This helper does a stack overflow check on *both* the native stack
 * and the VM stack.
 *
 * In some cases for re-entry, we're checking for space other than
 * just the callee, and `extraCells' may need to be passed with a
 * non-zero value.  (We over-check in these situations, but it's fine.)
 */
ALWAYS_INLINE
void checkStack(Stack& stk, const Func* f, int32_t extraCells) {
  /*
   * Check whether func's maximum stack usage would overflow the stack.
   * Both native and VM stack overflows are independently possible.
   *
   * All stack checks are inflated by kStackCheckPadding to ensure
   * there is space both for calling leaf functions /and/ for
   * re-entry.  (See kStackCheckReenterPadding and
   * kStackCheckLeafPadding.)
   */
  auto limit = f->maxStackCells() + kStackCheckPadding + extraCells;
  if (LIKELY(stack_in_bounds() && !stk.wouldOverflow(limit))) return;
  TRACE_MOD(Trace::gc, 1, "Maximum stack depth exceeded.\n");
  raise_error("Stack overflow");
}

/*
 * Only use in case of extreme shadiness.
 *
 * FCall* opcodes that do not use FPI regions write a pre-live ActRec
 * on the stack and then call a doFCall() helper that performs various
 * checks and may throw. In the absence of FPI regions the unwinder does
 * not expect an ActRec on the stack, so we overwrite it with TypedValues.
 *
 * There are also a few cases in the JIT where we allocate a pre-live
 * ActRec on the stack, and then call a helper that may re-enter the
 * VM (e.g. for autoload) to do the rest of the work filling it out.
 * Examples are FuncCache::lookup() or loadArrayFunctionContext().
 *
 * In these situations, we set up a "strange marker" by calling
 * updateMarker() before the instruction is done (but after the
 * pre-live ActRec is pushed).  This marker will have a lower SP than
 * the start of the instruction, but the PC will still point at the
 * instruction.  This is done so that if we need to re-enter from the
 * C++ helper we don't clobber the pre-live ActRec.
 *
 * However, if we throw, the unwinder won't think we're in the FPI
 * region yet.  So in the case that the helper throws an exception,
 * the unwinder will believe it has to decref three normal stack slots
 * (where the pre-live ActRec is).  We need the unwinder to ignore the
 * half-built ActRec allocated on the stack and certainly to avoid
 * attempting to decref its contents.  We achieve this by overwriting
 * the ActRec cells with nulls.
 *
 * A TypedValue* is also returned here to allow the CPP helper to
 * write whatever it needs to be decref'd into one of the eval cells,
 * to ensure that the unwinder leaves state the same as it was before
 * the call into FPush bytecode that threw.
 */
inline TypedValue* arPreliveOverwriteCells(ActRec *preLiveAR) {
  auto actRecCell = reinterpret_cast<TypedValue*>(preLiveAR);
  for (size_t ar_cell = 0; ar_cell < HPHP::kNumActRecCells; ++ar_cell) {
    tvWriteNull(*(actRecCell + ar_cell));
  }
  return actRecCell + HPHP::kNumActRecCells - 1;
}

}
#endif
