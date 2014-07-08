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

#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/layout.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/check.h"

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

void AsmInfo::updateForInstruction(IRInstruction* inst, TCA start, TCA end) {
  auto* block = inst->block();
  instRanges[inst] = TcaRange(start, end);
  asmRanges[block] = TcaRange(asmRanges[block].start(), end);
}

/*
 * Compute and save registers that are live *across* each inst, not including
 * registers whose lifetimes end at inst, nor registers defined by inst.
 */
LiveRegs computeLiveRegs(const IRUnit& unit, const RegAllocInfo& regs) {
  StateVector<Block, RegSet> live_in(unit, RegSet());
  LiveRegs live_regs(unit, RegSet());
  for (bool changed = true; changed;) {
    changed = false;
    postorderWalk(unit,
      [&](Block* block) {
        RegSet live;
        if (Block* taken = block->taken()) live = live_in[taken];
        if (Block* next = block->next()) live |= live_in[next];
        for (auto it = block->end(); it != block->begin(); ) {
          IRInstruction& inst = *--it;
          live -= regs.dstRegs(inst);
          live_regs[inst] = live;
          live |= regs.srcRegs(inst);
        }
        changed |= (live != live_in[block]);
        live_in[block] = live;
      });
  }
  return live_regs;
}

static void genBlock(IRUnit& unit, CodeBlock& cb, CodeBlock& coldCode,
                     CodeBlock& frozenCode,
                     CodegenState& state, Block* block,
                     std::vector<TransBCMapping>* bcMap) {
  FTRACE(6, "genBlock: {}\n", block->id());
  std::unique_ptr<CodeGenerator> cg(mcg->backEnd().newCodeGenerator(unit, cb,
                                                                    coldCode,
                                                                    frozenCode,
                                                                    state));
  for (IRInstruction& instr : *block) {
    IRInstruction* inst = &instr;

    if (instr.is(EndGuards)) state.pastGuards = true;
    if (bcMap && state.pastGuards &&
        (mcg->tx().isTransDBEnabled() || RuntimeOption::EvalJitUseVtuneAPI)) {
      // Don't insert an entry in bcMap if the marker corresponds to last entry
      // in there.
      if (bcMap->empty() ||
          bcMap->back().md5 != inst->marker().func()->unit()->md5() ||
          bcMap->back().bcStart != inst->marker().bcOff()) {
        bcMap->push_back(TransBCMapping{
            inst->marker().func()->unit()->md5(),
            inst->marker().bcOff(),
            mcg->cgFixups().m_tletMain->frontier(),
            mcg->cgFixups().m_tletCold->frontier(),
            mcg->cgFixups().m_tletFrozen->frontier()});
      }
    }
    auto* start = cb.frontier();
    cg->cgInst(inst);
    if (state.asmInfo && start < cb.frontier()) {
      state.asmInfo->updateForInstruction(inst, start, cb.frontier());
    }
  }
}

static void genCodeImpl(IRUnit& unit, AsmInfo* asmInfo) {
  auto regs = allocateRegs(unit);
  assert(checkRegisters(unit, regs)); // calls checkCfg internally.
  Timer _t(Timer::codeGen);
  LiveRegs live_regs = computeLiveRegs(unit, regs);
  CodegenState state(unit, regs, live_regs, asmInfo);

  // Returns: whether a block has already been emitted.
  DEBUG_ONLY auto isEmitted = [&](Block* block) {
    return state.addresses[block];
  };

  CodeBlock& mainCodeIn   = mcg->code.main();
  CodeBlock& coldCodeIn   = mcg->code.cold();
  CodeBlock* frozenCode   = &mcg->code.frozen();

  CodeBlock mainCode;
  CodeBlock coldCode;
  bool relocate = false;
  if (RuntimeOption::EvalJitRelocationSize &&
      mcg->backEnd().supportsRelocation() &&
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
    auto off = rand_r(&seed) & (mcg->backEnd().cacheLineSize() - 1);
    coldCode.init(coldCodeIn.frontier() +
                   RuntimeOption::EvalJitRelocationSize + off,
                   RuntimeOption::EvalJitRelocationSize - off, "cgRelocCold");

    mainCode.init(coldCode.frontier() +
                  RuntimeOption::EvalJitRelocationSize + off,
                  RuntimeOption::EvalJitRelocationSize - off, "cgRelocMain");

    relocate = true;
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
  auto bcMap = &mcg->cgFixups().m_bcMap;

  {
    mcg->code.lock();
    mcg->cgFixups().setBlocks(&mainCode, &coldCode, frozenCode);

    SCOPE_EXIT {
      mcg->cgFixups().setBlocks(nullptr, nullptr, nullptr);
      mcg->code.unlock();
    };

    /*
     * Emit the given block on the supplied assembler.  The `nextLinear'
     * is the next block that will be emitted on this assembler.  If is
     * not the next block in control flow order, then emit a patchable jump
     * to the next flow block.
     */
    auto emitBlock = [&](CodeBlock& cb, Block* block, Block* nextLinear) {
      assert(!isEmitted(block));

      FTRACE(6, "genBlock {} on {}\n", block->id(),
             cb.base() == coldCode.base() ? "acold" : "a");

      auto const aStart       = cb.frontier();
      auto const acoldStart   = coldCode.frontier();
      auto const afrozenStart = frozenCode->frontier();
      mcg->backEnd().patchJumps(cb, state, block);
      state.addresses[block] = aStart;

      // If the block ends with a Jmp and the next block is going to be
      // its target, we don't need to actually emit it.
      IRInstruction* last = &block->back();
      state.noTerminalJmp = last->op() == Jmp && nextLinear == last->taken();

      if (state.asmInfo) {
        state.asmInfo->asmRanges[block] = TcaRange(aStart, cb.frontier());
      }

      genBlock(unit, cb, coldCode, *frozenCode, state, block, bcMap);
      auto nextFlow = block->next();
      if (nextFlow && nextFlow != nextLinear) {
        mcg->backEnd().emitFwdJmp(cb, nextFlow, state);
      }

      if (state.asmInfo) {
        state.asmInfo->asmRanges[block] = TcaRange(aStart, cb.frontier());
        if (cb.base() != coldCode.base() && frozenCode != &coldCode) {
          state.asmInfo->acoldRanges[block] = TcaRange(acoldStart,
                                                       coldCode.frontier());
        }
        if (cb.base() != frozenCode->base()) {
          state.asmInfo->afrozenRanges[block] = TcaRange(afrozenStart,
                                                        frozenCode->frontier());
        }
      }
    };

    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      mcg->backEnd().emitTraceCall(mainCode, unit.bcOff());
    }

    auto const linfo = layoutBlocks(unit);

    for (auto it = linfo.blocks.begin(); it != linfo.acoldIt; ++it) {
      Block* nextLinear = boost::next(it) != linfo.acoldIt
        ? *boost::next(it) : nullptr;
      emitBlock(mainCode, *it, nextLinear);
    }
    for (auto it = linfo.acoldIt; it != linfo.afrozenIt; ++it) {
      Block* nextLinear = boost::next(it) != linfo.afrozenIt
        ? *boost::next(it) : nullptr;
      emitBlock(coldCode, *it, nextLinear);
    }
    for (auto it = linfo.afrozenIt; it != linfo.blocks.end(); ++it) {
      Block* nextLinear = boost::next(it) != linfo.blocks.end()
        ? *boost::next(it) : nullptr;
      emitBlock(*frozenCode, *it, nextLinear);
    }

    if (debug) {
      for (Block* UNUSED block : linfo.blocks) {
        assert(isEmitted(block));
      }
    }
  }

  assert(coldCodeIn.frontier() == coldStart);
  assert(mainCodeIn.frontier() == mainStart);

  if (relocate) {
    if (asmInfo) {
      printUnit(kRelocationLevel, unit, " before relocation ", &regs, asmInfo);
    }

    auto& be = mcg->backEnd();
    RelocationInfo mainRel(mainCode.base(), mainCode.frontier());
    be.relocate(mainRel, mainCodeIn, mcg->cgFixups());

    RelocationInfo coldRel(coldCode.base(), coldCode.frontier());

    be.relocate(coldRel, coldCodeIn, mcg->cgFixups());

    be.adjustForRelocation(mainRel.dest(),
                           mainRel.dest() + mainRel.destSize(),
                           coldRel, mcg->cgFixups());
    be.adjustForRelocation(coldRel.dest(),
                           coldRel.dest() + coldRel.destSize(),
                           mainRel, mcg->cgFixups());
    if (frozenCode != &coldCode) {
      be.adjustForRelocation(frozenStart, frozenCode->frontier(),
                             coldRel, mcg->cgFixups());
      be.adjustForRelocation(frozenStart, frozenCode->frontier(),
                             mainRel, mcg->cgFixups());
    }
    be.adjustForRelocation(asmInfo, coldRel, mcg->cgFixups());
    be.adjustForRelocation(asmInfo, mainRel, mcg->cgFixups());

    if (asmInfo) {
      static int64_t mainDeltaTot = 0, coldDeltaTot = 0;
      int64_t mainDelta =
        mainRel.destSize() -
        (mainCode.frontier() - mainCode.base());
      int64_t coldDelta =
        coldRel.destSize() -
        (coldCode.frontier() - coldCode.base());

      mainDeltaTot += mainDelta;
      HPHP::Trace::traceRelease("main delta after relocation: %" PRId64
                                " (%" PRId64 ")\n",
                                mainDelta, mainDeltaTot);
      coldDeltaTot += coldDelta;
      HPHP::Trace::traceRelease("cold delta after relocation: %" PRId64
                                " (%" PRId64 ")\n",
                                coldDelta, coldDeltaTot);
    }
#ifndef NDEBUG
    auto& ip = mcg->cgFixups().m_inProgressTailJumps;
    for (size_t i = 0; i < ip.size(); ++i) {
      const auto& ib = ip[i];
      assert(!mainCode.contains(ib.toSmash()));
      assert(!coldCode.contains(ib.toSmash()));
    }
    memset(mainCode.base(), 0xcc, mainCode.frontier() - mainCode.base());
    memset(coldCode.base(), 0xcc, coldCode.frontier() - coldCode.base());
#endif
  } else {
    coldCodeIn.skip(coldCode.frontier() - coldCodeIn.frontier());
    mainCodeIn.skip(mainCode.frontier() - mainCodeIn.frontier());
  }
  if (asmInfo) {
    printUnit(kCodeGenLevel, unit, " after code gen ", &regs, asmInfo);
  }
}

void genCode(IRUnit& unit) {
  if (dumpIREnabled()) {
    AsmInfo ai(unit);
    genCodeImpl(unit, &ai);
  } else {
    genCodeImpl(unit, nullptr);
  }
}

}}
