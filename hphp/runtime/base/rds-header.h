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

#ifndef incl_HPHP_RUNTIME_RDS_HEADER_H_
#define incl_HPHP_RUNTIME_RDS_HEADER_H_

#include <atomic>
#include <cstddef>

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/minstr-state.h"

namespace HPHP {

/*
 * Do not access this struct directly from rds::header(). Use the accessors in
 * runtime/vm/vm-regs.h.
 */
struct VMRegs {
  /* VM evaluation stack. Grows down in memory in multiples of 16 bytes - the
   * size of a TypedValue. */
  Stack stack;

  /* VM frame pointer. Contains information about the current executing
   * function, class context, caller, and a base pointer for accessing local
   * variables. */
  ActRec* fp;

  /* VM program counter. Points to the beginning of the currently executing
   * bytecode instruction. */
  PC pc;

  /* Scratch space for use by member instructions. */
  MInstrState mInstrState;

  /* First ActRec of this VM instance. */
  ActRec* firstAR;

  /* If the current VM nesting level is dispatchBB() as called by
   * MCGenerator::handleResume(), this is set to what vmfp() was on the first
   * entry to dispatchBB(). Otherwise, it's nullptr. See jitReturnPre() and
   * jitReturnPost() in bytecode.cpp for usage. Note that we will have at most
   * one active call to handleResume() in each VM nesting level, which is why
   * this is just a single pointer. */
  ActRec* jitCalledFrame;
};

namespace rds {

/*
 * Statically layed-out header that goes at the front of RDS.
 */
struct Header {
  /*
   * Combination of surprise flags and the limit (lowest address) of the
   * evaluation stack.  May be written to by other threads.
   *
   * At various points, the runtime will check whether this word contains a
   * higher number than what it believes the evaluation stack needs to be
   * (remember the eval stack grows down), which combines a stack overflow
   * check and a check for unusual conditions.  If this check triggers, the
   * runtime will do more detailed checks to see if it's actually dealing with
   * a stack overflow, or a surprise condition.
   *
   * All the surprise flag bits are in the upper 16 bits of this value, which
   * must be zero if it is actually a pointer to the lowest address of the
   * evaluation stack (the normal, "unsurprised" situation)---if one of the
   * surprise flags is set, the pointer will be higher than any legal eval
   * stack pointer and we'll go to a slow path to handle possible unusual
   * conditions (e.g. OOM).  (This is making use of the x64 property that
   * "canonical form" addresses have all their upper bits the same as bit 47,
   * and that this is zero for linux userland pointers.)
   */
  std::atomic<size_t> stackLimitAndSurprise;

  VMRegs vmRegs;
};

/*
 * Access to the statically layed out header.
 */
inline Header* header() {
  return static_cast<Header*>(tl_base);
}

constexpr ptrdiff_t kSurpriseFlagsOff  = offsetof(Header,
                                                  stackLimitAndSurprise);
constexpr ptrdiff_t kVmRegsOff         = offsetof(Header, vmRegs);
constexpr ptrdiff_t kVmspOff           = kVmRegsOff + offsetof(VMRegs, stack) +
                                           Stack::topOfStackOffset();
constexpr ptrdiff_t kVmfpOff           = kVmRegsOff + offsetof(VMRegs, fp);
constexpr ptrdiff_t kVmpcOff           = kVmRegsOff + offsetof(VMRegs, pc);
constexpr ptrdiff_t kVmFirstAROff      = kVmRegsOff + offsetof(VMRegs, firstAR);
constexpr ptrdiff_t kVmMInstrStateOff  = kVmRegsOff +
                                           offsetof(VMRegs, mInstrState);

static_assert((kVmMInstrStateOff % 16) == 0,
              "MInstrState should be 16-byte aligned in rds::Header");
static_assert(kVmspOff == 16, "Eager vm-reg save in translator-asm-helpers.S");
static_assert(kVmfpOff == 32, "Eager vm-reg save in translator-asm-helpers.S");

} }

#endif
