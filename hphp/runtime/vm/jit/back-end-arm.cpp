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

#include "hphp/runtime/vm/jit/back-end-arm.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/util/text-color.h"
#include "hphp/vixl/a64/disasm-a64.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/code-gen-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/func-prologues-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs-arm.h"
#include "hphp/runtime/vm/jit/reg-alloc-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/unwind-arm.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"

namespace HPHP { namespace jit { namespace arm {

TRACE_SET_MOD(hhir);

struct BackEnd : public jit::BackEnd {
  BackEnd() {}
  ~BackEnd() {}

  Abi abi() override {
    return arm::abi;
  }

  size_t cacheLineSize() override {
    // Not necessarily correct; depends on manufacturer implementation.
    return 64;
  }

  PhysReg rSp() override {
    return PhysReg(vixl::sp);
  }

  PhysReg rVmSp() override {
    return PhysReg(arm::rVmSp);
  }

  PhysReg rVmFp() override {
    return PhysReg(arm::rVmFp);
  }

  Constraint srcConstraint(const IRInstruction& inst, unsigned i) override {
    return arm::srcConstraint(inst, i);
  }

  Constraint dstConstraint(const IRInstruction& inst, unsigned i) override {
    return arm::dstConstraint(inst, i);
  }

  RegPair precolorSrc(const IRInstruction& inst, unsigned i) override;
  RegPair precolorDst(const IRInstruction& inst, unsigned i) override;

#define CALLEE_SAVED_BARRIER() \
  asm volatile("" : : : "x19", "x20", "x21", "x22", "x23", "x24", "x25", \
               "x26", "x27", "x28")

 private:
  /*
   * A partial equivalent of enterTCHelper, used to set up the ARM simulator.
   */
  uintptr_t setupSimRegsAndStack(vixl::Simulator& sim,
                                 uintptr_t saved_rStashedAr) {
    sim.   set_xreg(arm::rGContextReg.code(), g_context.getNoCheck());

    auto& vmRegs = vmRegsUnsafe();
    sim.   set_xreg(arm::rVmFp.code(), vmRegs.fp);
    sim.   set_xreg(arm::rVmSp.code(), vmRegs.stack.top());
    sim.   set_xreg(arm::rVmTl.code(), RDS::tl_base);
    sim.   set_xreg(arm::rStashedAR.code(), saved_rStashedAr);

    // Leave space for register spilling and MInstrState.
    sim.   set_sp(sim.sp() - kReservedRSPTotalSpace);
    assert(sim.is_on_stack(reinterpret_cast<void*>(sim.sp())));

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
  void enterTCHelper(TCA start, TReqInfo& info) override {
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

    DEBUG_ONLY auto spOnEntry =
      setupSimRegsAndStack(sim, info.saved_rStashedAr);

    // The handshake is different in the case of REQ_BIND_CALL. The code we're
    // jumping to expects to find a return address in x30, and a saved return
    // address on the stack.
    if (info.requestNum == REQ_BIND_CALL) {
      // Put the call's return address in the link register.
      auto* ar = reinterpret_cast<ActRec*>(info.saved_rStashedAr);
      sim.set_lr(ar->m_savedRip);
    }

    std::cout.flush();
    sim.RunFrom(vixl::Instruction::Cast(start));
    std::cout.flush();

    assert(sim.sp() == spOnEntry);

    info.requestNum = sim.xreg(0);
    info.args[0] = sim.xreg(1);
    info.args[1] = sim.xreg(2);
    info.args[2] = sim.xreg(3);
    info.args[3] = sim.xreg(4);
    info.args[4] = sim.xreg(5);
    info.saved_rStashedAr = sim.xreg(arm::rStashedAR.code());

    info.stubAddr = reinterpret_cast<TCA>(sim.xreg(arm::rAsm.code()));
  }

  jit::CodeGenerator* newCodeGenerator(const IRUnit& unit,
                                       CodeBlock& mainCode,
                                       CodeBlock& coldCode,
                                       CodeBlock& frozenCode,
                                       CodegenState& state) override {
    return new arm::CodeGenerator(unit, mainCode, coldCode,
                                  frozenCode, state);
  }

  void moveToAlign(CodeBlock& cb,
                   MoveToAlignFlags alignment
                   = MoveToAlignFlags::kJmpTargetAlign) override {
    // TODO(2967396) implement properly
  }

  UniqueStubs emitUniqueStubs() override {
    return arm::emitUniqueStubs();
  }

  TCA emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                         ServiceRequest req,
                         const ServiceReqArgVec& argv) override {
    return arm::emitServiceReqWork(cb, start, flags, req, argv);
  }

  void emitInterpReq(CodeBlock& mainCode, CodeBlock& coldCode,
                     const SrcKey& sk) override {
    if (RuntimeOption::EvalJitTransCounters) {
      vixl::MacroAssembler a { mainCode };
      arm::emitTransCounterInc(a);
    }
    // This jump won't be smashed, but a far jump on ARM requires the same code
    // sequence.
    mcg->backEnd().emitSmashableJump(
      mainCode,
      emitServiceReq(coldCode, REQ_INTERPRET, sk.offset()),
      CC_None
    );
  }

  bool funcPrologueHasGuard(TCA prologue, const Func* func) override {
    return arm::funcPrologueHasGuard(prologue, func);
  }

  TCA funcPrologueToGuard(TCA prologue, const Func* func) override {
    return arm::funcPrologueToGuard(prologue, func);
  }

  SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& coldCode, Func* func,
                          bool funcIsMagic, int nPassed, TCA& start,
                          TCA& aStart) override {
    return arm::emitFuncPrologue(mainCode, coldCode, func, funcIsMagic,
                                 nPassed, start, aStart);
  }

  TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) override {
    return arm::emitCallArrayPrologue(func, dvs);
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

  void emitTraceCall(CodeBlock& cb, Offset pcOff) override {
    // TODO(2967396) implement properly
  }

  void emitFwdJmp(CodeBlock& cb, Block* target, CodegenState& state) override {
    // This function always emits a smashable jump but every jump on ARM is
    // smashable so it's free.
    emitJumpToBlock(cb, target, CC_None, state);
  }

  void patchJumps(CodeBlock& cb, CodegenState& state, Block* block) override {
    auto dest = cb.frontier();
    auto jump = reinterpret_cast<TCA>(state.patches[block]);

    while (jump && jump != kEndOfTargetChain) {
      auto nextIfJmp = mcg->backEnd().jmpTarget(jump);
      auto nextIfJcc = mcg->backEnd().jccTarget(jump);

      // Exactly one of them must be non-nullptr
      assert(!(nextIfJmp && nextIfJcc));
      assert(nextIfJmp || nextIfJcc);

      if (nextIfJmp) {
        mcg->backEnd().smashJmp(jump, dest);
        jump = nextIfJmp;
      } else {
        mcg->backEnd().smashJcc(jump, dest);
        jump = nextIfJcc;
      }
    }
  }

  bool isSmashable(Address frontier, int nBytes, int offset = 0) override {
    // See prepareForSmash().
    return true;
  }

  void prepareForSmash(CodeBlock& cb, int nBytes, int offset = 0) override {
    // Don't do anything. We don't smash code on ARM; we smash non-executable
    // data -- an 8-byte pointer -- that's embedded in the instruction stream.
    // As long as that data is 8-byte aligned, it's safe to smash. All
    // instructions are 4 bytes wide, so we'll just emit a single nop if needed
    // to align the data.  This is done in emitSmashableJump.
  }

  void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
                              TestAndSmashFlags flags) override {
    // Nothing. See prepareForSmash().
  }

 private:
  void smashJmpOrCall(TCA addr, TCA dest, bool isCall) {
    // Assert that this is actually the instruction sequence we expect
    DEBUG_ONLY auto ldr = vixl::Instruction::Cast(addr);
    DEBUG_ONLY auto branch = vixl::Instruction::Cast(addr + 4);
    assert(ldr->Bits(31, 24) == 0x58);
    assert((branch->Bits(31, 10) == 0x3587C0 ||
            branch->Bits(31, 10) == 0x358FC0) &&
           branch->Bits(4, 0) == 0);

    // These offsets are asserted in emitSmashableJump and emitSmashableCall. We
    // wrote two instructions for an unconditional jump, or three for a call,
    // and then the jump/call destination was written at the next 8-byte
    // boundary.
    auto dataPtr = (isCall ? addr + 12 : addr + 8);
    if ((uintptr_t(dataPtr) & 7) != 0) {
      dataPtr += 4;
      assert((uintptr_t(dataPtr) & 7) == 0);
    }
    *reinterpret_cast<TCA*>(dataPtr) = dest;
  }

 public:
  void smashJmp(TCA jmpAddr, TCA newDest) override {
    assert(MCGenerator::canWrite());
    FTRACE(2, "smashJmp: {} -> {}\n", jmpAddr, newDest);
    smashJmpOrCall(jmpAddr, newDest, false);
  }

  void smashCall(TCA callAddr, TCA newDest) override {
    assert(MCGenerator::canWrite());
    FTRACE(2, "smashCall: {} -> {}\n", callAddr, newDest);
    smashJmpOrCall(callAddr, newDest, true);
  }

  void smashJcc(TCA jccAddr, TCA newDest) override {
    // This offset is asserted in emitSmashableJump. We wrote three
    // instructions.  Then the jump destination was written at the next 8-byte
    // boundary.
    auto dataPtr = jccAddr + 12;
    if ((uintptr_t(dataPtr) & 7) != 0) {
      dataPtr += 4;
      assert((uintptr_t(dataPtr) & 7) == 0);
    }
    *reinterpret_cast<TCA*>(dataPtr) = newDest;
  }

  void emitSmashableJump(CodeBlock& cb, TCA dest, ConditionCode cc) override {
    vixl::MacroAssembler a { cb };
    vixl::Label targetData;
    vixl::Label afterData;
    DEBUG_ONLY auto start = cb.frontier();

    // We emit the target address straight into the instruction stream, and then
    // do a pc-relative load to read it. This neatly sidesteps the problem of
    // concurrent modification and execution, as well as the problem of 19- and
    // 26-bit jump offsets (not big enough). It does, however, entail an
    // indirect jump.
    if (cc == CC_None) {
      a.    Ldr  (arm::rAsm, &targetData);
      a.    Br   (arm::rAsm);
      if (!cb.isFrontierAligned(8)) {
        a.  Nop  ();
        assert(cb.isFrontierAligned(8));
      }
      a.    bind (&targetData);
      a.    dc64 (reinterpret_cast<int64_t>(dest));

      // If this assert breaks, you need to change smashJmp
      assert(targetData.target() == start + 8 ||
             targetData.target() == start + 12);
    } else {
      a.    B    (&afterData, InvertCondition(arm::convertCC(cc)));
      a.    Ldr  (arm::rAsm, &targetData);
      a.    Br   (arm::rAsm);
      if (!cb.isFrontierAligned(8)) {
        a.  Nop  ();
        assert(cb.isFrontierAligned(8));
      }
      a.    bind (&targetData);
      a.    dc64 (reinterpret_cast<int64_t>(dest));
      a.    bind (&afterData);

      // If this assert breaks, you need to change smashJcc
      assert(targetData.target() == start + 12 ||
             targetData.target() == start + 16);
    }
  }

  void emitSmashableCall(CodeBlock& cb, TCA dest) override {
    vixl::MacroAssembler a { cb };
    vixl::Label afterData;
    vixl::Label targetData;
    DEBUG_ONLY auto start = cb.frontier();

    a.  Ldr  (arm::rAsm, &targetData);
    a.  Blr  (arm::rAsm);
    // When the call returns, jump over the data.
    a.  B    (&afterData);
    if (!cb.isFrontierAligned(8)) {
      a.Nop  ();
      assert(cb.isFrontierAligned(8));
    }
    a.  bind (&targetData);
    a.  dc64 (reinterpret_cast<int64_t>(dest));
    a.  bind (&afterData);

    // If this assert breaks, you need to change smashCall
    assert(targetData.target() == start + 12 ||
           targetData.target() == start + 16);
  }

  TCA jmpTarget(TCA jmp) override {
    // This doesn't verify that each of the two or three instructions that make
    // up this sequence matches; just the first one and the indirect jump.
    using namespace vixl;
    Instruction* ldr = Instruction::Cast(jmp);
    if (ldr->Bits(31, 24) != 0x58) return nullptr;

    Instruction* br = Instruction::Cast(jmp + 4);
    if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

    uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 8);
    if ((dest & 7) != 0) {
      dest += 4;
      assert((dest & 7) == 0);
    }
    return *reinterpret_cast<TCA*>(dest);
  }

  TCA jccTarget(TCA jmp) override {
    using namespace vixl;
    Instruction* b = Instruction::Cast(jmp);
    if (b->Bits(31, 24) != 0x54 || b->Bit(4) != 0) return nullptr;

    Instruction* br = Instruction::Cast(jmp + 8);
    if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

    uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 12);
    if ((dest & 7) != 0) {
      dest += 4;
      assert((dest & 7) == 0);
    }
    return *reinterpret_cast<TCA*>(dest);
  }

  TCA callTarget(TCA call) override {
    using namespace vixl;
    Instruction* ldr = Instruction::Cast(call);
    if (ldr->Bits(31, 24) != 0x58) return nullptr;

    Instruction* blr = Instruction::Cast(call + 4);
    if (blr->Bits(31, 10) != 0x358FC0 || blr->Bits(4, 0) != 0) return nullptr;

    uintptr_t dest = reinterpret_cast<uintptr_t>(blr + 8);
    if ((dest & 7) != 0) {
      dest += 4;
      assert((dest & 7) == 0);
    }
    return *reinterpret_cast<TCA*>(dest);
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
      assert(a.isFrontierAligned(8));
    }
    a.   bind (&interpReqAddr);
    TCA interpReq =
      emitServiceReq(codeCold, REQ_INTERPRET, sk.offset());
    a.   dc64 (interpReq);
    a.   bind (&after);
  }

  void streamPhysReg(std::ostream& os, PhysReg& reg) override {
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
    assert(begin <= end);
    for (; begin < end; begin += kInstructionSize) {
      dec.Decode(Instruction::Cast(begin));
    }
  }
};

std::unique_ptr<jit::BackEnd> newBackEnd() {
  return std::unique_ptr<jit::BackEnd>{ folly::make_unique<BackEnd>() };
}

RegPair BackEnd::precolorSrc(const IRInstruction& inst, unsigned i) {
  return InvalidRegPair;
}

RegPair BackEnd::precolorDst(const IRInstruction& inst, unsigned i) {
  return InvalidRegPair;
}

}}}
