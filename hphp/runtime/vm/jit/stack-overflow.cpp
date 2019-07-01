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

#include "hphp/runtime/base/runtime-error.h"
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
#include "hphp/util/compilation-flags.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

bool checkCalleeStackOverflow(const ActRec* calleeAR) {
  auto const func = calleeAR->func();
  auto const limit = func->maxStackCells() + kStackCheckPadding;

  const void* const needed_top =
    reinterpret_cast<const TypedValue*>(calleeAR) - limit;

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
    reinterpret_cast<Cell*>(calleeAR) - calleeAR->func()->numSlotsInFrame();
  tl_regState = VMRegState::CLEAN;

  throw FatalErrorException("Stack overflow");
}

void handlePossibleStackOverflow(ActRec* calleeAR) {
  assert_native_stack_aligned();

  // If it's not an overflow, it was probably a surprise flag trip.  But we
  // can't assert that it is because background threads are allowed to clear
  // surprise bits concurrently, so it could be cleared again by now.
  if (!checkCalleeStackOverflow(calleeAR)) return;
  auto const func = calleeAR->func();

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
   * shoved them into an ExtraArgs on the calleeAR, or into an array for a
   * variadic capture param.  We need to get things into an appropriate state
   * for handleStackOverflow to be able to synchronize things to throw from the
   * PC of the caller's FCall.
   *
   * We don't actually need to make sure the stack is the right depth for the
   * FCall: the unwinder will expect to see a pre-live ActRec (and we'll set it
   * up so it will), but it doesn't care how many args (or what types of args)
   * are below it on the stack.
   *
   * It is tempting to try to free the ExtraArgs structure here, but it's ok to
   * not to:
   *
   *     o We're about to raise an uncatchable fatal, which will end the
   *       request.  We leak ExtraArgs in other similar situations for this too
   *       (e.g. if called via FCall with unpack and then a stack overflow
   *       happens).
   *
   *     o If we were going to free the ExtraArgs structure, we'd need to make
   *       sure we can re-enter the VM right now, which means performing a
   *       manual fixup first.  (We aren't in a situation where we can do a
   *       normal VMRegAnchor fixup right now.)  But moreover we shouldn't be
   *       running destructors if a fatal is happening anyway, so we don't want
   *       that either.
   *
   * So, all that boils down to this: we ignore the extra args field (the
   * unwinder will not consult the ExtraArgs field because it believes the
   * ActRec is pre-live).  And set calleeAR->m_numArgs to indicate how many
   * things are actually on the stack (so handleStackOverflow knows what to set
   * the vmsp to)---we just set it to the function's numLocals, which might
   * mean decreffing some uninits unnecessarily, but that's ok.
   */

  if (debug && func->attrs() & AttrMayUseVV && calleeAR->getExtraArgs()) {
    calleeAR->trashVarEnv();
  }
  calleeAR->setNumArgs(calleeAR->m_func->numLocals());
  handleStackOverflow(calleeAR);
}

///////////////////////////////////////////////////////////////////////////////

}}
