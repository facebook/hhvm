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

#include "hphp/runtime/vm/jit/unique-stubs-arm.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/unwind-arm.h"

#include "hphp/vixl/a64/decoder-a64.h"
#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/vixl/a64/instructions-a64.h"
#include "hphp/vixl/a64/simulator-a64.h"

#include <folly/ScopeGuard.h>

#include <iostream>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(ustubs);

extern "C" void enterTCHelper(Cell* vm_sp,
                              ActRec* vm_fp,
                              TCA start,
                              ActRec* firstAR,
                              void* targetCacheBase,
                              ActRec* stashedAR);

///////////////////////////////////////////////////////////////////////////////

namespace arm {

///////////////////////////////////////////////////////////////////////////////

/*
 * A partial equivalent of enterTCHelper, used to set up the ARM simulator.
 */
static uintptr_t setupSimRegsAndStack(vixl::Simulator& sim,
                                      ActRec* saved_rStashedAr) {
  auto& vmRegs = vmRegsUnsafe();
  sim.   set_xreg(x2a(rvmfp()).code(), vmRegs.fp);
  sim.   set_xreg(x2a(rvmsp()).code(), vmRegs.stack.top());
  sim.   set_xreg(x2a(rvmtl()).code(), rds::tl_base);

  // Leave space for register spilling and MInstrState.
  assertx(sim.is_on_stack(reinterpret_cast<void*>(sim.sp())));

  auto spOnEntry = sim.sp();

  // Push the link register onto the stack. The link register is technically
  // caller-saved; what this means in practice is that non-leaf functions push
  // it at the very beginning and pop it just before returning (as opposed to
  // just saving it around calls).
  sim.   set_sp(sim.sp() - 16);
  *reinterpret_cast<uint64_t*>(sim.sp()) = sim.lr();

  return spOnEntry;
}

void enterTCImpl(TCA start, ActRec* stashedAR) {
  // This is a pseudo-copy of the logic in enterTCHelper: it sets up the
  // simulator's registers and stack, runs the translation, and gets the
  // necessary information out of the registers when it's done.
  vixl::PrintDisassembler disasm(std::cout);
  vixl::Decoder decoder;
  if (getenv("ARM_DISASM")) {
    decoder.AppendVisitor(&disasm);
  }
  vixl::Simulator sim(&decoder, std::cout);
  SCOPE_EXIT {
    Stats::inc(Stats::vixl_SimulatedInstr, sim.instr_count());
    Stats::inc(Stats::vixl_SimulatedLoad, sim.load_count());
    Stats::inc(Stats::vixl_SimulatedStore, sim.store_count());
  };

  sim.set_exception_hook(arm::simulatorExceptionHook);

  g_context->m_activeSims.push_back(&sim);
  SCOPE_EXIT { g_context->m_activeSims.pop_back(); };

  DEBUG_ONLY auto spOnEntry = setupSimRegsAndStack(sim, stashedAR);

  // The handshake is different when entering at a func prologue. The code
  // we're jumping to expects to find a return address in x30, and a saved
  // return address on the stack.
  if (stashedAR) {
    // Put the call's return address in the link register.
    sim.set_lr(stashedAR->m_savedRip);
  }

  std::cout.flush();
  sim.RunFrom(vixl::Instruction::Cast(start));
  std::cout.flush();

  assertx(sim.sp() == spOnEntry);

  vmRegsUnsafe().fp = (ActRec*)sim.xreg(x2a(rvmfp()).code());
  vmRegsUnsafe().stack.top() = (Cell*)sim.xreg(x2a(rvmsp()).code());
}

///////////////////////////////////////////////////////////////////////////////

}}}
