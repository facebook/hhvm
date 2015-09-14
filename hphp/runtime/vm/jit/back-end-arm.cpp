/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/back-end-arm.h"

#include <iostream>

#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/util/text-color.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/func-guard-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/unwind-arm.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

namespace HPHP { namespace jit { namespace arm {

TRACE_SET_MOD(hhir);

namespace {

struct BackEnd final : jit::BackEnd {
  BackEnd() {}
  ~BackEnd() {}

#define CALLEE_SAVED_BARRIER() \
  asm volatile("" : : : "x19", "x20", "x21", "x22", "x23", "x24", "x25", \
               "x26", "x27", "x28")

 private:
  /*
   * A partial equivalent of enterTCHelper, used to set up the ARM simulator.
   */
  uintptr_t setupSimRegsAndStack(vixl::Simulator& sim,
                                 ActRec* saved_rStashedAr) {
    sim.   set_xreg(arm::rGContextReg.code(), g_context.getNoCheck());

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

 public:
  void enterTCHelper(TCA start, ActRec* stashedAR) override {
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

  void streamPhysReg(std::ostream& os, PhysReg reg) override {
    if (reg.isSF()) {
      os << "statusFlags";
      return;
    }
    auto prefix = reg.isGP() ? (vixl::Register(reg).size() == vixl::kXRegSize
                                ? 'x' : 'w')
                  : (vixl::FPRegister(reg).size() == vixl::kSRegSize
                     ? 's' : 'd');
    vixl::CPURegister r = reg;
    os << prefix << r.code();
  }

  void disasmRange(std::ostream& os, int indent, bool dumpIR, TCA begin,
                   TCA end) override {
    using namespace vixl;
    Decoder dec;
    PrintDisassembler disasm(os, indent + 4, dumpIR, color(ANSI_COLOR_BROWN));
    dec.AppendVisitor(&disasm);
    assertx(begin <= end);
    for (; begin < end; begin += kInstructionSize) {
      dec.Decode(Instruction::Cast(begin));
    }
  }
};

}

std::unique_ptr<jit::BackEnd> newBackEnd() {
  return folly::make_unique<BackEnd>();
}

}}}
