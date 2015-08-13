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
#include "hphp/runtime/vm/jit/code-gen-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/func-guard-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs-arm.h"
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

  Abi abi() override {
    return arm::abi;
  }

  PhysReg rSp() override {
    return PhysReg(vixl::sp);
  }

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
    sim.   set_xreg(arm::rVmFp.code(), vmRegs.fp);
    sim.   set_xreg(arm::rVmSp.code(), vmRegs.stack.top());
    sim.   set_xreg(arm::rVmTl.code(), rds::tl_base);

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

    vmRegsUnsafe().fp = (ActRec*)sim.xreg(arm::rVmFp.code());
    vmRegsUnsafe().stack.top() = (Cell*)sim.xreg(arm::rVmSp.code());
  }

  UniqueStubs emitUniqueStubs() override {
    return arm::emitUniqueStubs();
  }

  void emitInterpReq(CodeBlock& mainCode,
                     SrcKey sk,
                     FPInvOffset spOff) override {
    if (RuntimeOption::EvalJitTransCounters) {
      vixl::MacroAssembler a { mainCode };
      arm::emitTransCounterInc(a);
    }
    not_implemented();
  }

  bool funcPrologueHasGuard(TCA prologue, const Func* func) override {
    return arm::funcPrologueHasGuard(prologue, func);
  }

  TCA funcPrologueToGuard(TCA prologue, const Func* func) override {
    return arm::funcPrologueToGuard(prologue, func);
  }

  void funcPrologueSmashGuard(TCA prologue, const Func* func) override {
    arm::funcPrologueSmashGuard(prologue, func);
  }

  void emitIncStat(CodeBlock& cb, intptr_t disp, int n) override {
    using arm::rAsm;
    using arm::rAsm2;
    vixl::MacroAssembler a { cb };

    a.    Mrs   (rAsm2, vixl::TPIDR_EL0);
    a.    Ldr   (rAsm, rAsm2[disp]);
    a.    Add   (rAsm, rAsm, n);
    a.    Str   (rAsm, rAsm2[disp]);
  }

  void addDbgGuard(CodeBlock& codeMain, CodeBlock& codeCold,
                   SrcKey sk, size_t dbgOff) override {
    vixl::MacroAssembler a { codeMain };

    vixl::Label after;
    vixl::Label interpReqAddr;

    // Get the debugger-attached flag from thread-local storage. Don't bother
    // saving caller-saved regs around the host call; this is between blocks.
    emitTLSLoad<ThreadInfo>(a, ThreadInfo::s_threadInfo, rAsm);

    // Is the debugger attached?
    a.   Ldr  (rAsm.W(), rAsm[dbgOff]);
    a.   Tst  (rAsm, 0xff);
    // skip jump to cold if no debugger attached
    a.   B    (&after, vixl::eq);
    a.   Ldr  (rAsm, &interpReqAddr);
    a.   Br   (rAsm);
    if (!a.isFrontierAligned(8)) {
      a. Nop  ();
      assertx(a.isFrontierAligned(8));
    }
    a.   bind (&interpReqAddr);
    not_implemented();
    a.   bind (&after);
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

  void genCodeImpl(IRUnit& unit, CodeKind, AsmInfo*) override;
};

}

std::unique_ptr<jit::BackEnd> newBackEnd() {
  return folly::make_unique<BackEnd>();
}

static size_t genBlock(CodegenState& state, Vout& v, Vout& vc, Block* block) {
  FTRACE(6, "genBlock: {}\n", block->id());
  CodeGenerator cg(state, v, vc);
  size_t hhir_count{0};
  for (IRInstruction& inst : *block) {
    hhir_count++;
    if (inst.is(EndGuards)) state.pastGuards = true;
    v.setOrigin(&inst);
    vc.setOrigin(&inst);
    cg.cgInst(&inst);
  }
  return hhir_count;
}

void BackEnd::genCodeImpl(IRUnit& unit, CodeKind kind, AsmInfo* asmInfo) {
  Timer _t(Timer::codeGen);
  CodeBlock& mainCodeIn   = mcg->code.main();
  CodeBlock& coldCodeIn   = mcg->code.cold();
  CodeBlock* frozenCode   = &mcg->code.frozen();

  CodeBlock mainCode;
  CodeBlock coldCode;
  /*
   * Use separate code blocks, so that attempts to use the mcg's
   * code blocks directly will fail (eg by overwriting the same
   * memory being written through these locals).
   */
  coldCode.init(coldCodeIn.frontier(), coldCodeIn.available(),
                coldCodeIn.name().c_str());
  mainCode.init(mainCodeIn.frontier(), mainCodeIn.available(),
                mainCodeIn.name().c_str());

  if (frozenCode == &coldCodeIn) {
    frozenCode = &coldCode;
  }
  auto coldStart DEBUG_ONLY = coldCodeIn.frontier();
  auto mainStart DEBUG_ONLY = mainCodeIn.frontier();
  size_t hhir_count{0};

  {
    mcg->code.lock();
    mcg->cgFixups().setBlocks(&mainCode, &coldCode, frozenCode);

    SCOPE_EXIT {
      mcg->cgFixups().setBlocks(nullptr, nullptr, nullptr);
      mcg->code.unlock();
    };

    CodegenState state(unit, asmInfo, *frozenCode);
    auto const blocks = rpoSortCfg(unit);
    Vasm vasm;
    auto& vunit = vasm.unit();
    // create the initial set of vasm numbered the same as hhir blocks.
    for (uint32_t i = 0, n = unit.numBlocks(); i < n; ++i) {
      state.labels[i] = vunit.makeBlock(AreaIndex::Main);
    }
    // create vregs for all relevant SSATmps
    assignRegs(unit, vunit, state, blocks);
    vunit.entry = state.labels[unit.entry()];

    Vtext vtext { mainCode, coldCode, *frozenCode };

    for (auto block : blocks) {
      auto& v = block->hint() == Block::Hint::Unlikely ? vasm.cold() :
               block->hint() == Block::Hint::Unused ? vasm.frozen() :
               vasm.main();
      FTRACE(6, "genBlock {} on {}\n", block->id(),
             area_names[(unsigned)v.area()]);
      auto b = state.labels[block];
      vunit.blocks[b].area = v.area();
      v.use(b);
      hhir_count += genBlock(state, v, vasm.cold(), block);
      assertx(v.closed());
      assertx(vasm.main().empty() || vasm.main().closed());
      assertx(vasm.cold().empty() || vasm.cold().closed());
      assertx(vasm.frozen().empty() || vasm.frozen().closed());
    }
    printUnit(kInitialVasmLevel, "after initial vasm generation", vunit);
    assertx(check(vunit));
    finishARM(vasm.unit(), vtext, arm::abi, state.asmInfo);
  }

  assertx(coldCodeIn.frontier() == coldStart);
  assertx(mainCodeIn.frontier() == mainStart);
  coldCodeIn.skip(coldCode.frontier() - coldCodeIn.frontier());
  mainCodeIn.skip(mainCode.frontier() - mainCodeIn.frontier());
  if (asmInfo) {
    printUnit(kCodeGenLevel, unit, " after code gen ", asmInfo);
  }
}

}}}
