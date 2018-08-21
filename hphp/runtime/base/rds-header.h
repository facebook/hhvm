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

#ifndef incl_HPHP_RUNTIME_RDS_HEADER_H_
#define incl_HPHP_RUNTIME_RDS_HEADER_H_

#include <atomic>
#include <cstddef>

#include "hphp/runtime/base/rds.h"
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

  TYPE_SCAN_CUSTOM() {
    // ActRecs are always interior pointers so the type-scanner won't
    // automatically enqueue them.
    scanner.scan(fp);
    scanner.scan(mInstrState);
    scanner.scan(firstAR);
    scanner.scan(jitCalledFrame);
  };
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

#ifndef NDEBUG
  /*
   * In builds with assertions enabled, we write-protect non-persistent RDS
   * while in certain parts of the jit. We still want to allow jit threads to
   * write to the surprise flags, so we don't write-protect the first page and
   * push the rest of Header to the next page.
   *
   * If the Header ends up on a page that is larger than 4096 bytes, vmRegs
   * might not be write-protected when we want it to be, but this is just a
   * debugging aid and isn't necessary for correctness. We use 4096 bytes of
   * padding rather than 4096 - sizeof(size_t) to not disturb the relative
   * alignment of vmRegs.
   */
  const char padding[4096];
#endif

  VMRegs     vmRegs;
  GenNumber  currentGen;
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

} }

#endif
