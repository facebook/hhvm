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
   * Surprise flags.  May be written by other threads.  At various
   * points, the runtime will check whether this word is non-zero, and
   * if so go to a slow path to handle unusual conditions (e.g. OOM).
   */
  std::atomic<ssize_t> conditionFlags;

  VMRegs vmRegs;
};

/*
 * Access to the statically layed out header.
 */
inline Header* header() {
  return static_cast<Header*>(tl_base);
}

constexpr ptrdiff_t kConditionFlagsOff = offsetof(Header, conditionFlags);
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
