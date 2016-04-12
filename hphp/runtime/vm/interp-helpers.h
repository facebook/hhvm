/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/util/trace.h"

namespace HPHP {

// In PHP7 the caller specifies if parameter type-checking is strict in the
// callee. NB: HH files ignore this preference and always use strict checking,
// except in systemlib. Calls originating in systemlib are always strict.
inline bool callUsesStrictTypes(ActRec* caller) {
  if (RuntimeOption::EnableHipHopSyntax || !RuntimeOption::PHP7_ScalarTypes) {
    return true;
  }
  if (!caller) {
    return true;
  }
  auto func = caller->func();
  return func->isBuiltin() || func->unit()->useStrictTypes();
}

inline bool builtinCallUsesStrictTypes(const Unit* caller) {
  if (!RuntimeOption::PHP7_ScalarTypes || RuntimeOption::EnableHipHopSyntax) {
    return false;
  }
  return caller->useStrictTypes() && !caller->isHHFile();
}

inline void setTypesFlag(ActRec* fp, ActRec* ar) {
  if (!callUsesStrictTypes(fp)) ar->setUseWeakTypes();
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
