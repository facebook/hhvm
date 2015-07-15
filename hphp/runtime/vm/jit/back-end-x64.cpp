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

#include "hphp/runtime/vm/jit/back-end-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/disasm.h"
#include "hphp/util/text-color.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/func-prologues-x64.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-llvm.h"
#include "hphp/runtime/vm/jit/relocation.h"

namespace HPHP { namespace jit {

using namespace reg;

extern "C" void enterTCHelper(Cell* vm_sp,
                              ActRec* vm_fp,
                              TCA start,
                              ActRec* firstAR,
                              void* targetCacheBase,
                              ActRec* stashedAR);

namespace x64 {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

struct BackEnd final : jit::BackEnd {
  Abi abi() override {
    return x64::abi;
  }

  size_t cacheLineSize() override {
    return 64;
  }

  PhysReg rSp() override {
    return PhysReg(reg::rsp);
  }

  PhysReg rVmSp() override {
    return x64::rVmSp;
  }

  PhysReg rVmFp() override {
    return x64::rVmFp;
  }

  PhysReg rVmTl() override {
    return x64::rVmTl;
  }

  /*
   * enterTCHelper does not save callee-saved registers except %rbp. This means
   * when we call it from C++, we have to tell gcc to clobber all the other
   * callee-saved registers.
   */
  #define CALLEE_SAVED_BARRIER()                                    \
      asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15");

  /*
   * enterTCHelper is a handwritten assembly function that transfers control in
   * and out of the TC.
   */
  static_assert(x64::rVmSp == rbx &&
                x64::rVmFp == rbp &&
                x64::rVmTl == r12,
                "__enterTCHelper needs to be modified to use the correct ABI");

  void enterTCHelper(TCA start, ActRec* stashedAR) override {
    // We have to force C++ to spill anything that might be in a callee-saved
    // register (aside from rbp). enterTCHelper does not save them.
    CALLEE_SAVED_BARRIER();
    auto& regs = vmRegsUnsafe();
    jit::enterTCHelper(regs.stack.top(), regs.fp, start,
                       vmFirstAR(), rds::tl_base, stashedAR);
    CALLEE_SAVED_BARRIER();
  }

  UniqueStubs emitUniqueStubs() override {
    return x64::emitUniqueStubs();
  }

  TCA emitServiceReqWork(CodeBlock& cb,
                         TCA start,
                         SRFlags flags,
                         folly::Optional<FPInvOffset> spOff,
                         ServiceRequest req,
                         const ServiceReqArgVec& argv) override {
    return x64::emitServiceReqWork(cb, start, flags, spOff, req, argv);
  }

  size_t reusableStubSize() const override {
    return x64::reusableStubSize();
  }

  void emitInterpReq(CodeBlock& mainCode,
                     SrcKey sk,
                     FPInvOffset spOff) override {
    Asm a { mainCode };
    // Add a counter for the translation if requested
    if (RuntimeOption::EvalJitTransCounters) {
      x64::emitTransCounterInc(a);
    }
    a.    emitImmReg(uint64_t(sk.pc()), argNumToRegName[0]);
    if (!sk.resumed()) {
      a.  lea(x64::rVmFp[-cellsToBytes(spOff.offset)], x64::rVmSp);
    }
    a.    jmp(mcg->tx().uniqueStubs.interpHelper);
  }

  bool funcPrologueHasGuard(TCA prologue, const Func* func) override {
    return x64::funcPrologueHasGuard(prologue, func);
  }

  TCA funcPrologueToGuard(TCA prologue, const Func* func) override {
    return x64::funcPrologueToGuard(prologue, func);
  }

  TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) override {
    return x64::emitCallArrayPrologue(func, dvs);
  }

  void funcPrologueSmashGuard(TCA prologue, const Func* func) override {
    x64::funcPrologueSmashGuard(prologue, func);
  }

  void emitIncStat(CodeBlock& cb, intptr_t disp, int n) override {
    X64Assembler a { cb };

    a.    pushf ();
    //    addq $n, [%fs:disp]
    a.    fs().addq(n, baseless(disp));
    a.    popf  ();
  }

  void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
                              TestAndSmashFlags flags) override {
    using namespace x64;
    switch (flags) {
      case TestAndSmashFlags::kAlignJcc:
        prepareForSmash(cb, testBytes + kJmpccLen, testBytes);
        assertx(isSmashable(cb.frontier() + testBytes, kJmpccLen));
        break;
      case TestAndSmashFlags::kAlignJccImmediate:
        prepareForSmash(cb,
                        testBytes + kJmpccLen,
                        testBytes + kJmpccLen - kJmpImmBytes);
        assertx(isSmashable(cb.frontier() + testBytes, kJmpccLen,
                           kJmpccLen - kJmpImmBytes));
        break;
      case TestAndSmashFlags::kAlignJccAndJmp:
        // Ensure that the entire jcc, and the entire jmp are smashable
        // (but we dont need them both to be in the same cache line)
        prepareForSmashImpl(cb, testBytes + kJmpccLen, testBytes);
        prepareForSmashImpl(cb, testBytes + kJmpccLen + kJmpLen,
                            testBytes + kJmpccLen);
        mcg->cgFixups().m_alignFixups.emplace(
          cb.frontier(), std::make_pair(testBytes + kJmpccLen, testBytes));
        mcg->cgFixups().m_alignFixups.emplace(
          cb.frontier(), std::make_pair(testBytes + kJmpccLen + kJmpLen,
                                        testBytes + kJmpccLen));
        assertx(isSmashable(cb.frontier() + testBytes, kJmpccLen));
        assertx(isSmashable(cb.frontier() + testBytes + kJmpccLen, kJmpLen));
        break;
    }
  }

  void smashJmp(TCA jmpAddr, TCA newDest) override {
    return x64::smashJmp(jmpAddr, newDest);
  }

  void smashCall(TCA callAddr, TCA newDest) override {
    x64::smashCall(callAddr, newDest);
  }

  void smashJcc(TCA jccAddr, TCA newDest) override {
    assertx(MCGenerator::canWrite());
    FTRACE(2, "smashJcc: {} -> {}\n", jccAddr, newDest);
    // Make sure the encoding is what we expect. It has to be a rip-relative jcc
    // with a 4-byte delta.
    assertx(*jccAddr == 0x0F && (*(jccAddr + 1) & 0xF0) == 0x80);
    assertx(isSmashable(jccAddr, x64::kJmpccLen));

    // Can't use the assembler to write out a new instruction, because we have
    // to preserve the condition code.
    auto newDelta = safe_cast<int32_t>(newDest - jccAddr - x64::kJmpccLen);
    auto deltaAddr = reinterpret_cast<int32_t*>(jccAddr
                                                + x64::kJmpccLen
                                                - x64::kJmpImmBytes);
    *deltaAddr = newDelta;
  }

  void emitSmashableJump(CodeBlock& cb, TCA dest, ConditionCode cc) override {
    X64Assembler a { cb };
    if (cc == CC_None) {
      assertx(isSmashable(cb.frontier(), x64::kJmpLen));
      a.  jmp(dest);
    } else {
      assertx(isSmashable(cb.frontier(), x64::kJmpccLen));
      a.  jcc(cc, dest);
    }
  }

  TCA smashableCallFromReturn(TCA retAddr) override {
    auto addr = retAddr - x64::kCallLen;
    assertx(isSmashable(addr, x64::kCallLen));
    return addr;
  }

  void emitSmashableCall(CodeBlock& cb, TCA dest) override {
    X64Assembler a { cb };
    assertx(isSmashable(cb.frontier(), x64::kCallLen));
    a.  call(dest);
  }

  TCA jmpTarget(TCA jmp) override {
    if (jmp[0] != 0xe9) {
      if (jmp[0] == 0x0f &&
          jmp[1] == 0x1f &&
          jmp[2] == 0x44) {
        // 5 byte nop
        return jmp + 5;
      }
      return nullptr;
    }
    return jmp + 5 + ((int32_t*)(jmp + 5))[-1];
  }

  TCA jccTarget(TCA jmp) override {
    if (jmp[0] != 0x0F || (jmp[1] & 0xF0) != 0x80) return nullptr;
    return jmp + 6 + ((int32_t*)(jmp + 6))[-1];
  }

  ConditionCode jccCondCode(TCA jmp) override {
    return DecodedInstruction(jmp).jccCondCode();
  }

  TCA callTarget(TCA call) override {
    if (call[0] != 0xE8) return nullptr;
    return call + 5 + ((int32_t*)(call + 5))[-1];
  }

  void addDbgGuard(CodeBlock& codeMain,
                   CodeBlock& codeCold,
                   SrcKey sk,
                   size_t dbgOff) override {
    Asm a { codeMain };

    // Emit the checks for debugger attach
    auto rtmp = rAsm;
    emitTLSLoad<ThreadInfo>(a, ThreadInfo::s_threadInfo, rtmp);
    a.   loadb  (rtmp[dbgOff], rbyte(rtmp));
    a.   testb  ((int8_t)0xff, rbyte(rtmp));

    if (!sk.resumed()) {
      auto const off = mcg->tx().getSrcRec(sk)->nonResumedSPOff();
      a. lea    (x64::rVmFp[-cellsToBytes(off.offset)], x64::rVmSp);
    }

    // Branch to interpHelper if attached
    a.   emitImmReg(uint64_t(sk.pc()), argNumToRegName[0]);
    a.   jnz    (mcg->tx().uniqueStubs.interpHelper);
  }

  void streamPhysReg(std::ostream& os, PhysReg reg) override {
    auto name = (reg.type() == PhysReg::GP) ? reg::regname(Reg64(reg)) :
      (reg.type() == PhysReg::SIMD) ? reg::regname(RegXMM(reg)) :
      /* (reg.type() == PhysReg::SF) ? */ reg::regname(RegSF(reg));
    os << name;
  }

  void disasmRange(std::ostream& os, int indent, bool dumpIR, TCA begin,
                   TCA end) override {
    Disasm disasm(Disasm::Options().indent(indent + 4)
                  .printEncoding(dumpIR)
                  .color(color(ANSI_COLOR_BROWN)));
    disasm.disasm(os, begin, end);
  }

  void genCodeImpl(IRUnit& unit, CodeKind, AsmInfo*) override;

private:
  void do_moveToAlign(CodeBlock& cb, MoveToAlignFlags alignment) override {
    size_t x64Alignment;

    switch (alignment) {
    case MoveToAlignFlags::kJmpTargetAlign:
      x64Alignment = kJmpTargetAlign;
      break;
    case MoveToAlignFlags::kNonFallthroughAlign:
      x64Alignment = jit::kNonFallthroughAlign;
      break;
    case MoveToAlignFlags::kCacheLineAlign:
      x64Alignment = kCacheLineSize;
      break;
    }
    x64::moveToAlign(cb, x64Alignment);
  }

  bool do_isSmashable(Address frontier, int nBytes, int offset) override {
    return x64::isSmashable(frontier, nBytes, offset);
  }

  void do_prepareForSmash(CodeBlock& cb, int nBytes, int offset) override {
    prepareForSmashImpl(cb, nBytes, offset);
    mcg->cgFixups().m_alignFixups.emplace(cb.frontier(),
                                          std::make_pair(nBytes, offset));
  }
};

//////////////////////////////////////////////////////////////////////

void smashJmpOrCall(TCA addr, TCA dest, bool isCall) {
  // Unconditional rip-relative jmps can also be encoded with an EB as the
  // first byte, but that means the delta is 1 byte, and we shouldn't be
  // encoding smashable jumps that way.
  assertx(kJmpLen == kCallLen);
  always_assert(isSmashable(addr, kJmpLen));

  auto& cb = mcg->code.blockFor(addr);
  CodeCursor cursor { cb, addr };
  X64Assembler a { cb };
  if (dest > addr && dest - addr <= kJmpLen) {
    assertx(!isCall);
    a.  emitNop(dest - addr);
  } else if (isCall) {
    a.  call   (dest);
  } else {
    a.  jmp    (dest);
  }
}

//////////////////////////////////////////////////////////////////////

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

/*
 * Print side-by-side code dumps comparing vasm output with LLVM.
 */
static void printLLVMComparison(const IRUnit& ir_unit,
                                const Vunit& vasm_unit,
                                const Vasm::AreaList& areas,
                                const CompareLLVMCodeGen* compare) {
  auto const vasm_size = areas[0].code.frontier() - areas[0].start;
  auto const percentage = compare->main_size * 100 / vasm_size;

  // We accept a few different formats for the runtime option:
  // - "all": print all tracelets
  // - "<x": print when llvm code is < x% the size of vasm
  // - ">x" or "x": print when llvm code is > x% the size of vasm
  // - "=x": print when llvm code is = x% the size of vasm
  folly::StringPiece mode(RuntimeOption::EvalJitLLVMCompare);
  if (mode.empty()) return;
  if (mode != "all") {
    auto pred = '>';
    switch (mode[0]) {
      case '<':
      case '=':
      case '>':
        pred = mode[0];
        mode.pop_front();
        break;

      default:
        break;
    }
    auto const threshold = folly::to<int>(mode);
    if ((pred == '<' && percentage >= threshold) ||
        (pred == '=' && percentage != threshold) ||
        (pred == '>' && percentage <= threshold)) {
      return;
    }
  }

  Trace::ftraceRelease(
    "{:-^121}\n{}\n{:-^121}\n{}\n{:-^121}\n{}\n",
    folly::sformat(
      " vasm: {} bytes | llvm: {} bytes | llvm is {}% of vasm",
      vasm_size, compare->main_size, percentage
    ),
    show(ir_unit),
    " vasm unit ",
    show(vasm_unit),
    " llvm IR ",
    compare->llvm
  );

  auto const& llvmAreas = compare->disasm;
  assert(llvmAreas.size() == areas.size());
  Disasm disasm;

  for (auto i = 0; i < kNumAreas; ++i) {
    std::ostringstream vasmOut;
    auto& area = areas[i];
    disasm.disasm(vasmOut, area.start, area.code.frontier());
    auto const vasmCode = vasmOut.str();

    std::vector<folly::StringPiece> llvmLines, vasmLines;
    folly::split('\n', llvmAreas[i], llvmLines);
    folly::split('\n', vasmCode, vasmLines);

    Trace::ftraceRelease("{:-^121}\n", folly::sformat(" area {} ", i));
    for (auto llvmIt = llvmLines.begin(), vasmIt = vasmLines.begin();
         llvmIt != llvmLines.end() || vasmIt != vasmLines.end(); ) {
      folly::StringPiece llvmLine, vasmLine;
      if (llvmIt != llvmLines.end()) {
        llvmLine = *llvmIt;
        ++llvmIt;
      }
      if (vasmIt != vasmLines.end()) {
        vasmLine = *vasmIt;
        ++vasmIt;
      }
      if (vasmLine.empty() && llvmLine.empty()) continue;

      Trace::ftraceRelease("{:60.60} {:.60}\n", vasmLine, llvmLine);
    }
    Trace::ftraceRelease("\n");
  }
}

void BackEnd::genCodeImpl(IRUnit& unit, CodeKind kind, AsmInfo* asmInfo) {
  Timer _t(Timer::codeGen);
  CodeBlock& mainCodeIn   = mcg->code.main();
  CodeBlock& coldCodeIn   = mcg->code.cold();
  CodeBlock* frozenCode   = &mcg->code.frozen();

  CodeBlock mainCode;
  CodeBlock coldCode;
  bool do_relocate = false;
  if (!mcg->useLLVM() &&
      !RuntimeOption::EvalEnableReusableTC &&
      RuntimeOption::EvalJitRelocationSize &&
      coldCodeIn.canEmit(RuntimeOption::EvalJitRelocationSize * 3)) {
    /*
     * This is mainly to exercise the relocator, and ensure that its
     * not broken by new non-relocatable code. Later, it will be
     * used to do some peephole optimizations, such as reducing branch
     * sizes.
     * Allocate enough space that the relocated cold code doesn't
     * overlap the emitted cold code.
     */

    static unsigned seed = 42;
    auto off = rand_r(&seed) & (cacheLineSize() - 1);
    coldCode.init(coldCodeIn.frontier() +
                   RuntimeOption::EvalJitRelocationSize + off,
                   RuntimeOption::EvalJitRelocationSize - off, "cgRelocCold");

    mainCode.init(coldCode.frontier() +
                  RuntimeOption::EvalJitRelocationSize + off,
                  RuntimeOption::EvalJitRelocationSize - off, "cgRelocMain");

    do_relocate = true;
  } else {
    /*
     * Use separate code blocks, so that attempts to use the mcg's
     * code blocks directly will fail (eg by overwriting the same
     * memory being written through these locals).
     */
    coldCode.init(coldCodeIn.frontier(), coldCodeIn.available(),
                  coldCodeIn.name().c_str());
    mainCode.init(mainCodeIn.frontier(), mainCodeIn.available(),
                  mainCodeIn.name().c_str());
  }

  if (frozenCode == &coldCodeIn) {
    frozenCode = &coldCode;
  }

  auto frozenStart = frozenCode->frontier();
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
    SCOPE_ASSERT_DETAIL("vasm unit") { return show(vunit); };
    // create the initial set of vasm numbered the same as hhir blocks.
    for (uint32_t i = 0, n = unit.numBlocks(); i < n; ++i) {
      state.labels[i] = vunit.makeBlock(AreaIndex::Main);
    }
    // create vregs for all relevant SSATmps
    assignRegs(unit, vunit, state, blocks);
    vunit.entry = state.labels[unit.entry()];
    vasm.main(mainCode);
    vasm.cold(coldCode);
    vasm.frozen(*frozenCode);

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

    auto const& abi = kind == CodeKind::Trace ? x64::abi
                                              : x64::cross_trace_abi;

    if (mcg->useLLVM()) {
      auto& areas = vasm.areas();
      auto x64_unit = vunit;
      auto vasm_size = std::numeric_limits<size_t>::max();

      jit::vector<UndoMarker> undoAll = {UndoMarker(mcg->globalData())};
      for (auto const& area : areas) undoAll.emplace_back(area.code);
      auto resetCode = [&] {
        for (auto& marker : undoAll) marker.undo();
        mcg->cgFixups().clear();
      };
      auto optimized = false;

      // When EvalJitLLVMKeepSize is non-zero, we'll throw away the LLVM code
      // and use vasm's output instead if the LLVM code is more than x% the
      // size of the vasm code. First we generate and throw away code with
      // vasm, just to see how big it is. The cost of this is trivial compared
      // to the LLVM code generation.
      if (RuntimeOption::EvalJitLLVMKeepSize) {
        optimizeX64(x64_unit, abi);
        optimized = true;
        emitX64(x64_unit, areas, nullptr);
        vasm_size = areas[0].code.frontier() - areas[0].start;
        resetCode();
      }

      try {
        genCodeLLVM(vunit, areas);

        auto const llvm_size = areas[0].code.frontier() - areas[0].start;
        if (llvm_size * 100 / vasm_size > RuntimeOption::EvalJitLLVMKeepSize) {
          throw FailedLLVMCodeGen("LLVM size {}, vasm size {}\n",
                                  llvm_size, vasm_size);
        }
      } catch (const FailedLLVMCodeGen& e) {
        FTRACE_MOD(Trace::llvm,
                   1, "LLVM codegen failed ({}); falling back to x64 backend\n",
                   e.what());
        always_assert_flog(
          RuntimeOption::EvalJitLLVM < 3,
          "Mandatory LLVM codegen failed with reason `{}' on unit:\n{}",
          e.what(), show(vunit)
        );

        mcg->setUseLLVM(false);
        resetCode();
        if (!optimized) optimizeX64(x64_unit, abi);
        emitX64(x64_unit, areas, state.asmInfo);

        if (auto compare = dynamic_cast<const CompareLLVMCodeGen*>(&e)) {
          printLLVMComparison(unit, vasm.unit(), areas, compare);
        }
      }
    } else {
      optimizeX64(vunit, abi);
      emitX64(vunit, vasm.areas(), state.asmInfo);
    }
  }

  auto bcMap = &mcg->cgFixups().m_bcMap;
  if (do_relocate && !bcMap->empty()) {
    TRACE(1, "BCMAPS before relocation\n");
    for (UNUSED auto& map : *bcMap) {
      TRACE(1, "%s %-6d %p %p %p\n", map.md5.toString().c_str(),
             map.bcStart, map.aStart, map.acoldStart, map.afrozenStart);
    }
  }

  assertx(coldCodeIn.frontier() == coldStart);
  assertx(mainCodeIn.frontier() == mainStart);

  if (do_relocate) {
    if (asmInfo) {
      printUnit(kRelocationLevel, unit, " before relocation ", asmInfo);
    }

    RelocationInfo rel;
    size_t asm_count{0};
    asm_count += relocate(rel, mainCodeIn,
                          mainCode.base(), mainCode.frontier(),
                          mcg->cgFixups(), nullptr);

    asm_count += relocate(rel, coldCodeIn,
                          coldCode.base(), coldCode.frontier(),
                          mcg->cgFixups(), nullptr);
    TRACE(1, "hhir-inst-count %ld asm %ld\n", hhir_count, asm_count);

    if (frozenCode != &coldCode) {
      rel.recordRange(frozenStart, frozenCode->frontier(),
                      frozenStart, frozenCode->frontier());
    }
    adjustForRelocation(rel);
    adjustMetaDataForRelocation(rel, asmInfo, mcg->cgFixups());
    adjustCodeForRelocation(rel, mcg->cgFixups());

    if (asmInfo) {
      static int64_t mainDeltaTot = 0, coldDeltaTot = 0;
      int64_t mainDelta =
        (mainCodeIn.frontier() - mainStart) -
        (mainCode.frontier() - mainCode.base());
      int64_t coldDelta =
        (coldCodeIn.frontier() - coldStart) -
        (coldCode.frontier() - coldCode.base());

      mainDeltaTot += mainDelta;
      coldDeltaTot += coldDelta;
      if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, 1)) {
        HPHP::Trace::traceRelease("main delta after relocation: "
                                  "%" PRId64 " (%" PRId64 ")\n",
                                  mainDelta, mainDeltaTot);
        HPHP::Trace::traceRelease("cold delta after relocation: "
                                  "%" PRId64 " (%" PRId64 ")\n",
                                  coldDelta, coldDeltaTot);
      }
    }
#ifndef NDEBUG
    auto& ip = mcg->cgFixups().m_inProgressTailJumps;
    for (size_t i = 0; i < ip.size(); ++i) {
      const auto& ib = ip[i];
      assertx(!mainCode.contains(ib.toSmash()));
      assertx(!coldCode.contains(ib.toSmash()));
    }
    memset(mainCode.base(), 0xcc, mainCode.frontier() - mainCode.base());
    memset(coldCode.base(), 0xcc, coldCode.frontier() - coldCode.base());
#endif
  } else {
    coldCodeIn.skip(coldCode.frontier() - coldCodeIn.frontier());
    mainCodeIn.skip(mainCode.frontier() - mainCodeIn.frontier());
  }

  if (asmInfo) {
    printUnit(kCodeGenLevel, unit, " after code gen ", asmInfo);
  }
}

//////////////////////////////////////////////////////////////////////

bool isSmashable(Address frontier, int nBytes, int offset /* = 0 */) {
  assertx(nBytes <= int(kCacheLineSize));
  uintptr_t iFrontier = uintptr_t(frontier) + offset;
  uintptr_t lastByte = uintptr_t(frontier) + nBytes - 1;
  return (iFrontier & ~kCacheLineMask) == (lastByte & ~kCacheLineMask);
}

void prepareForSmashImpl(CodeBlock& cb, int nBytes, int offset) {
  if (isSmashable(cb.frontier(), nBytes, offset)) return;
  X64Assembler a { cb };
  int gapSize = (~(uintptr_t(a.frontier()) + offset) & kCacheLineMask) + 1;
  a.emitNop(gapSize);
  assertx(isSmashable(a.frontier(), nBytes, offset));
}

void smashJmp(TCA jmpAddr, TCA newDest) {
  assertx(MCGenerator::canWrite());
  FTRACE(2, "smashJmp: {} -> {}\n", jmpAddr, newDest);
  smashJmpOrCall(jmpAddr, newDest, false);
}

void smashCall(TCA callAddr, TCA newDest) {
  assertx(MCGenerator::canWrite());
  FTRACE(2, "smashCall: {} -> {}\n", callAddr, newDest);
  smashJmpOrCall(callAddr, newDest, true);
}

//////////////////////////////////////////////////////////////////////

}}}
