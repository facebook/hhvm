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
   * We can't finish setting up the prolog here, so there is no
   * "correct" vm state. However, we need to mark the registers clean
   * because sync_regstate in unwind-itanium.cpp doesn't know how to
   * clean them.
   *
   * So the best we can do is to set vmfp() to the ActRec of the
   * function that was just called, vmpc() to the start of the
   * function, and vmsp() to the last parameter pushed(), and then
   * throw a special exception so the unwinder knows what to do.
   *
   * Since we want to raise the exception as if it were raised in the
   * caller, its tempting to set things up to calleeAR->sfp(), but
   * there are two problems: this could be the first frame in a VM
   * re-entry, and our caller might be inlined.
   *
   * In the first case, setting vmfp() to a frame in a prior vm
   * invocation will confuse things, and in the latter, our caller's
   * frame might not be linked into the call chain until we run the
   * catch trace corresponding to this call.
   *
   * We can solve both problems by throwing a special exception here,
   * and dealing with it after unwinding the whole vm entry.
   */
  auto& unsafeRegs = vmRegsUnsafe();
  unsafeRegs.fp = calleeAR;
  unsafeRegs.stack.top() =
    reinterpret_cast<Cell*>(calleeAR) - calleeAR->numArgs();
  auto const func_base = calleeAR->func()->base();
  // this isn't really true, but VMStackOverflow will fix it for us.
  unsafeRegs.pc = calleeAR->func()->unit()->at(func_base);
  tl_regState = VMRegState::CLEAN;

  calleeAR->setVarEnv(nullptr);
  throw VMStackOverflow{};
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
