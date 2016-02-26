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
  /*
   * First synchronize registers.
   *
   * We're called in two situations: either this is the first frame after a
   * re-entry, in which case calleeAR->m_sfp is enterTCHelper's native stack,
   * or we're called in the middle of one VM entry (from a func prologue).  We
   * want to raise the exception from the caller's FCall instruction in the
   * second case, and in the first case we have to raise in a special way
   * inside this re-entry.
   *
   * Either way the stack depth is below the calleeAR by numArgs, because we
   * haven't run func prologue duties yet.
   */
  auto& unsafeRegs = vmRegsUnsafe();
  auto const isReentry = calleeAR == vmFirstAR();
  auto const arToSync = isReentry ? calleeAR : calleeAR->m_sfp;
  unsafeRegs.fp = arToSync;
  unsafeRegs.stack.top() =
    reinterpret_cast<Cell*>(calleeAR) - calleeAR->numArgs();
  auto const func_base = arToSync->func()->base();
  // calleeAR m_soff is 0 in the re-entry case, so we'll set pc to the func
  // base.  But it also doesn't matter because we're going to throw a special
  // VMReenterStackOverflow in that case so the unwinder won't worry about it.
  unsafeRegs.pc = arToSync->func()->unit()->at(func_base + calleeAR->m_soff);
  tl_regState = VMRegState::CLEAN;

  if (!isReentry) {
    /*
     * The normal case - we were called via FCall, or FCallArray.  We need to
     * construct the pc of the fcall from the return address (which will be
     * after the fcall). Because fcall is a variable length instruction, and
     * because we sometimes delete instructions from the instruction stream, we
     * need to use fpi regions to find the fcall.
     */
    const FPIEnt* fe = liveFunc()->findPrecedingFPI(
      liveUnit()->offsetOf(vmpc()));
    vmpc() = liveUnit()->at(fe->m_fcallOff);
    assertx(isFCallStar(peek_op(vmpc())));
    raise_error("Stack overflow");
  } else {
    /*
     * We were called via re-entry.  Leak the params and the ActRec, and tell
     * the unwinder that there's nothing left to do in this "entry".
     *
     * Also, the caller hasn't set up the m_invName area on the ActRec (unless
     * it was a magic call), since it's the prologue's responsibility if it's a
     * non-magic call.  We can just null it out since we're fatalling.
     */
    vmsp() = reinterpret_cast<Cell*>(calleeAR + 1);
    calleeAR->setVarEnv(nullptr);
    throw VMReenterStackOverflow();
  }
  not_reached();
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
   *       (e.g. if called via FCallArray and then a stack overflow happens).
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
