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
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

namespace HPHP {

inline void callerDynamicCallChecks(const Func* func) {
  if (RuntimeOption::EvalForbidDynamicCalls <= 0) return;
  if (func->isDynamicallyCallable()) return;

  if (RuntimeOption::EvalForbidDynamicCalls >= 2) {
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
  }
}

inline void calleeDynamicCallChecks(const ActRec* ar) {
  if (!ar->isDynamicCall()) return;
  auto const func = ar->func();

  if (func->accessesCallerFrame()) {
    raise_disallowed_dynamic_call(func);
  }

  if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && func->isBuiltin()) {
    raise_notice(
      Strings::FUNCTION_CALLED_DYNAMICALLY,
      func->fullDisplayName()->data()
    );
  }
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

}
#endif
