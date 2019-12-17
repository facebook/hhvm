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

#include "hphp/runtime/vm/jit/stack-overflow.h"

#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/util/assertions.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

bool checkCalleeStackOverflow(const TypedValue* calleeFP, const Func* callee) {
  auto const limit = callee->maxStackCells() + kStackCheckPadding;
  const void* const needed_top = calleeFP - limit;
  const void* const lower_limit =
    static_cast<char*>(vmRegsUnsafe().stack.getStackLowAddress()) +
    Stack::sSurprisePageSize;

  return needed_top < lower_limit;
}

void handleStackOverflow(ActRec* calleeAR) {
  // We didn't finish setting up the prologue, so let the unwinder know that
  // locals are already freed so it doesn't try to decref the garbage. Do not
  // bother decrefing numArgs, as we are going to fail the request anyway.
  calleeAR->setLocalsDecRefd();

  // sync_regstate in unwind-itanium.cpp doesn't have enough context to properly
  // sync registers, so do it here. Sync them to correspond to the state on
  // function entry. This is not really true, but it's good enough for unwinder.
  auto& unsafeRegs = vmRegsUnsafe();
  unsafeRegs.fp = calleeAR;
  unsafeRegs.pc = calleeAR->func()->getEntry();
  unsafeRegs.stack.top() =
    reinterpret_cast<TypedValue*>(calleeAR) - calleeAR->func()->numSlotsInFrame();
  unsafeRegs.jitReturnAddr = nullptr;
  tl_regState = VMRegState::CLEAN;

  throw_stack_overflow();
}

void handlePossibleStackOverflow(ActRec* calleeAR) {
  assert_native_stack_aligned();

  // If it's not an overflow, it was probably a surprise flag trip.  But we
  // can't assert that it is because background threads are allowed to clear
  // surprise bits concurrently, so it could be cleared again by now.
  auto const calleeFP = reinterpret_cast<const TypedValue*>(calleeAR);
  if (!checkCalleeStackOverflow(calleeFP, calleeAR->func())) return;

  /*
   * Stack overflows in this situation are a slightly different case than
   * handleStackOverflow:
   *
   * A function prologue already did all the work to prepare to enter the
   * function, but then it found out it didn't have enough room on the stack.
   * It may even have written uninits deeper than the stack base (but we limit
   * it to sSurprisePageSize, so it's harmless).
   *
   * Most importantly, it might have pulled args /off/ the eval stack and
   * shoved them into an array for a variadic capture param.  We need to get
   * things into an appropriate state for handleStackOverflow to be able to
   * synchronize things to throw from the PC of the caller's FCall.
   *
   * We don't actually need to make sure the stack is the right depth for the
   * FCall: the unwinder will expect to see a pre-live ActRec (and we'll set it
   * up so it will), but it doesn't care how many args (or what types of args)
   * are below it on the stack.
   *
   * So, all that boils down to this: we set calleeAR->m_numArgs to indicate
   * how many things are actually on the stack (so handleStackOverflow knows
   * what to set the vmsp to)---we just set it to the function's numLocals,
   * which might mean decreffing some uninits unnecessarily, but that's ok.
   */

  calleeAR->setNumArgs(calleeAR->m_func->numLocals());
  handleStackOverflow(calleeAR);
}

///////////////////////////////////////////////////////////////////////////////

}}
