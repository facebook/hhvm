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

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

void AsmInfo::updateForInstruction(IRInstruction* inst, TCA start, TCA end) {
  auto* block = inst->block();
  instRanges[inst] = TcaRange(start, end);
  asmRanges[block] = TcaRange(asmRanges[block].start(), end);
}

const Func* loadClassCtor(Class* cls) {
  const Func* f = cls->getCtor();
  if (UNLIKELY(!(f->attrs() & AttrPublic))) {
    VMRegAnchor _;
    UNUSED LookupResult res =
      g_context->lookupCtorMethod(f, cls, true /*raise*/);
    assert(res == LookupResult::MethodFoundWithThis);
  }
  return f;
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

static void genBlock(IRUnit& unit, CodeBlock& cb, CodeBlock& stubsCode,
                     CodeBlock& unusedCode, MCGenerator* mcg,
                     CodegenState& state, Block* block,
                     std::vector<TransBCMapping>* bcMap) {
  FTRACE(6, "genBlock: {}\n", block->id());
  std::unique_ptr<CodeGenerator> cg(mcg->backEnd().newCodeGenerator(unit, cb,
                                                                    stubsCode,
                                                                    unusedCode,
                                                                    mcg,
                                                                    state));

  BCMarker prevMarker;
  for (IRInstruction& instr : *block) {
    IRInstruction* inst = &instr;
    // If we're on the first instruction of the block or we have a new
    // marker since the last instruction, update the bc mapping.
    if ((!prevMarker.valid() || inst->marker() != prevMarker) &&
        (mcg->tx().isTransDBEnabled() ||
        RuntimeOption::EvalJitUseVtuneAPI) && bcMap) {
      bcMap->push_back(TransBCMapping{inst->marker().func()->unit()->md5(),
                                      inst->marker().bcOff(),
                                      cb.frontier(),
                                      stubsCode.frontier()});
      prevMarker = inst->marker();
    }
    auto* addr = cg->cgInst(inst);
    if (state.asmInfo && addr) {
      state.asmInfo->updateForInstruction(inst, addr, cb.frontier());
    }
  }
}

void genCodeImpl(IRUnit& unit,
                 std::vector<TransBCMapping>* bcMap,
                 JIT::MCGenerator* mcg,
                 const RegAllocInfo& regs,
                 AsmInfo* asmInfo) {
  LiveRegs live_regs = computeLiveRegs(unit, regs);
  CodegenState state(unit, regs, live_regs, asmInfo);

  // Returns: whether a block has already been emitted.
  DEBUG_ONLY auto isEmitted = [&](Block* block) {
    return state.addresses[block];
  };

  CodeBlock& mainCode   = mcg->code.main();
  CodeBlock& stubsCode  = mcg->code.stubs();
  CodeBlock& unusedCode = mcg->code.unused();
  mcg->code.lock();
  SCOPE_EXIT { mcg->code.unlock(); };

  /*
   * Emit the given block on the supplied assembler.  The `nextLinear'
   * is the next block that will be emitted on this assembler.  If is
   * not the next block in control flow order, then emit a patchable jump
   * to the next flow block.
   */
  auto emitBlock = [&](CodeBlock& cb, Block* block, Block* nextLinear) {
    assert(!isEmitted(block));

    FTRACE(6, "genBlock {} on {}\n", block->id(),
           cb.base() == stubsCode.base() ? "astubs" : "a");

    auto const aStart      = cb.frontier();
    auto const astubsStart = stubsCode.frontier();
    mcg->backEnd().patchJumps(cb, state, block);
    state.addresses[block] = aStart;

    // If the block ends with a Jmp and the next block is going to be
    // its target, we don't need to actually emit it.
    IRInstruction* last = &block->back();
    state.noTerminalJmp = last->op() == Jmp && nextLinear == last->taken();

    if (state.asmInfo) {
      state.asmInfo->asmRanges[block] = TcaRange(aStart, cb.frontier());
    }

    genBlock(unit, cb, stubsCode, unusedCode, mcg, state, block, bcMap);
    auto nextFlow = block->next();
    if (nextFlow && nextFlow != nextLinear) {
      mcg->backEnd().emitFwdJmp(cb, nextFlow, state);
    }

    if (state.asmInfo) {
      state.asmInfo->asmRanges[block] = TcaRange(aStart, cb.frontier());
      if (cb.base() != stubsCode.base()) {
        state.asmInfo->astubRanges[block] = TcaRange(astubsStart,
                                                     stubsCode.frontier());
      }
    }
  };

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    mcg->backEnd().emitTraceCall(mainCode, unit.bcOff());
  }

  auto const linfo = layoutBlocks(unit);

  for (auto it = linfo.blocks.begin(); it != linfo.astubsIt; ++it) {
    Block* nextLinear = boost::next(it) != linfo.astubsIt
      ? *boost::next(it) : nullptr;
    emitBlock(mainCode, *it, nextLinear);
  }
  for (auto it = linfo.astubsIt; it != linfo.aunusedIt; ++it) {
    Block* nextLinear = boost::next(it) != linfo.aunusedIt
      ? *boost::next(it) : nullptr;
    emitBlock(stubsCode, *it, nextLinear);
  }
  for (auto it = linfo.aunusedIt; it != linfo.blocks.end(); ++it) {
    Block* nextLinear = boost::next(it) != linfo.blocks.end()
      ? *boost::next(it) : nullptr;
    emitBlock(unusedCode, *it, nextLinear);
  }

  if (debug) {
    for (Block* UNUSED block : linfo.blocks) {
      assert(isEmitted(block));
    }
  }
}

void genCode(IRUnit& unit, std::vector<TransBCMapping>* bcMap,
             JIT::MCGenerator* mcg,
             const RegAllocInfo& regs) {
  Timer _t(Timer::codeGen);

  if (dumpIREnabled()) {
    AsmInfo ai(unit);
    genCodeImpl(unit, bcMap, mcg, regs, &ai);
    printUnit(kCodeGenLevel, unit, " after code gen ", &regs, &ai);
  } else {
    genCodeImpl(unit, bcMap, mcg, regs, nullptr);
  }
}

}}
