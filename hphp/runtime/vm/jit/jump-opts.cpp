/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <utility>

#include <boost/next_prior.hpp>

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ir-trace.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

BlockList findMainExitBlocks(IRUnit& unit) {
  BlockList blocks;

  for (Block* b: unit.main()->blocks()) {
    if (b->isExit()) {
      blocks.push_back(b);
    }
  }
  return blocks;
}

/*
 * Utility class for pattern matching the instructions in a Block,
 * ignoring markers and the label.
 *
 * To use, create a BlockMatcher and call match with a variable-length
 * list of opcode ids.
 */
struct BlockMatcher {
  explicit BlockMatcher(Block* block)
    : m_block(block)
    , m_it(block->skipHeader())
  {}

  bool match() { return true; }

  template<class... Opcodes>
  bool match(Opcode op, Opcodes... opcs) {
    if (m_it == m_block->end()) return false;
    auto const cur = m_it->op();
    ++m_it;
    return cur == op && match(opcs...);
  }

private:
  Block* m_block;
  Block::const_iterator m_it;
};

/*
 * Returns whether the supplied block is a "normal" trace exit.
 *
 * That is, it does nothing other than sync ABI registers and bind to
 * the next tracelet.
 */
bool isNormalExit(Block* block) {
  return BlockMatcher(block).match(SyncABIRegs, ReqBindJmp);
}

// Returns whether `opc' is a within-tracelet conditional jump that
// can be folded into a ReqBindJmpFoo instruction.
bool jccCanBeDirectExit(Opcode opc) {
  return isQueryJmpOp(opc) && (opc != JmpIsType) && (opc != JmpIsNType);
    // TODO(#2404341)
}

/*
 * If main trace ends with a conditional jump with no side-effects on
 * exit, followed by the normal ReqBindJmp sequence, convert the whole
 * thing into a conditional ReqBindJmp.
 *
 * This leads to more efficient code because the service request stubs
 * will patch jumps in the main trace instead of off-trace.
 */
void optimizeCondTraceExit(IRUnit& unit) {
  FTRACE(5, "CondExit:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "CondExit:^^^^^^^^^^^^^^^^^^^^^\n"); };

  auto const mainExitBlocks = findMainExitBlocks(unit);
  for (Block* mainExit : mainExitBlocks) {
    if (!isNormalExit(mainExit)) continue;

    auto const& mainPreds = mainExit->preds();
    if (mainPreds.size() != 1) continue;

    auto const jccInst = mainPreds.front().inst();
    if (!jccCanBeDirectExit(jccInst->op())) return;
    FTRACE(5, "previous block ends with jccCanBeDirectExit ({})\n",
           opcodeName(jccInst->op()));

    auto jccExitBlock = jccInst->taken();
    if (jccExitBlock == mainExit) jccExitBlock = jccInst->next();
    if (!isNormalExit(jccExitBlock)) continue;
    FTRACE(5, "exit trace is side-effect free\n");

    auto it = mainExit->backIter();
    auto& reqBindJmp = *(it--);
    auto& syncAbi = *it;
    assert(syncAbi.op() == SyncABIRegs);

    auto const newOpcode = jmpToReqBindJmp(jccInst->op());
    ReqBindJccData data;
    data.taken = jccExitBlock->back().extra<ReqBindJmp>()->offset;
    data.notTaken = reqBindJmp.extra<ReqBindJmp>()->offset;

    FTRACE(5, "replacing {} with {}\n", jccInst->id(), opcodeName(newOpcode));
    unit.replace(
      &reqBindJmp,
      newOpcode,
      data,
      std::make_pair(jccInst->numSrcs(), jccInst->srcs().begin())
    );

    syncAbi.setMarker(jccInst->marker());
    reqBindJmp.setMarker(jccInst->marker());
    jccInst->convertToJmp(mainExit);
  }
}

/*
 * Look for CheckStk/CheckLoc instructions in the main trace that
 * branch to "normal exits".  We can optimize these into the
 * SideExitGuard* instructions that can be patched in place.
 */
void optimizeSideExitChecks(IRUnit& unit) {
  auto trace = unit.main();
  FTRACE(5, "SideExit:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "SideExit:^^^^^^^^^^^^^^^^^^^^^\n"); };

  forEachInst(trace->blocks(), [&] (IRInstruction* inst) {
    if (inst->op() != CheckStk && inst->op() != CheckLoc) return;
    auto const exitBlock = inst->taken();
    if (!isNormalExit(exitBlock)) return;

    auto const syncABI = &*boost::prior(exitBlock->backIter());
    assert(syncABI->op() == SyncABIRegs);

    FTRACE(5, "converting jump ({}) to side exit\n",
           inst->id());

    auto const isStack = inst->op() == CheckStk;
    auto const fp      = syncABI->src(0);
    auto const sp      = syncABI->src(1);

    SideExitGuardData data;
    data.checkedSlot = isStack
      ? inst->extra<CheckStk>()->offset
      : inst->extra<CheckLoc>()->locId;
    data.taken = exitBlock->back().extra<ReqBindJmp>()->offset;

    auto const block = inst->block();
    block->insert(block->iteratorTo(inst),
                  unit.cloneInstruction(syncABI));

    auto next = inst->next();
    unit.replace(
      inst,
      isStack ? SideExitGuardStk : SideExitGuardLoc,
      inst->typeParam(),
      data,
      isStack ? sp : fp
    );
    block->push_back(unit.gen(Jmp, inst->marker(), next));
  });
}

/*
 * Look for Jcc instructions in the main trace that
 * branch to "normal exits".  We can optimize these into the
 * SideExitJcc* instructions that can be patched in place.
 */
void optimizeSideExitJccs(IRUnit& unit) {
  auto trace = unit.main();
  FTRACE(5, "SideExitJcc:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "SideExitJcc:^^^^^^^^^^^^^^^^^^^^^\n"); };

  forEachInst(trace->blocks(), [&] (IRInstruction* inst) {
    if (!jccCanBeDirectExit(inst->op())) return;
    auto const exitBlock = inst->taken();
    if (!isNormalExit(exitBlock)) return;

    auto it = exitBlock->backIter();
    auto& reqBindJmp = *(it--);
    auto& syncABI = *it;
    assert(syncABI.op() == SyncABIRegs);

    FTRACE(5, "converting jcc ({}) to side exit\n",
           inst->id());

    auto const newOpcode = jmpToSideExitJmp(inst->op());
    SideExitJccData data;
    data.taken = reqBindJmp.extra<ReqBindJmp>()->offset;

    auto const block = inst->block();
    block->insert(block->iteratorTo(inst),
                  unit.cloneInstruction(&syncABI));

    auto next = inst->next();
    unit.replace(
      inst,
      newOpcode,
      data,
      std::make_pair(inst->numSrcs(), inst->srcs().begin())
    );
    block->push_back(unit.gen(Jmp, inst->marker(), next));
  });
}

// Return true if this block ends with a trivial Jmp (a Jmp that does
// not pass arguments, and whose target's only predecessor is b.
bool isTrivialJmp(IRInstruction* branch, Block* taken) {
  return branch->op() == Jmp && branch->numSrcs() == 0 &&
         taken->numPreds() == 1;
}

// If main trace ends with an unconditional jump, and the target is not
// reached by any other branch, then copy the target of the jump to the
// end of the trace
void eliminateJmp(Block* lastBlock, IRInstruction* jmp, Block* target) {
  assert(isTrivialJmp(jmp, target));
  auto lastInst = lastBlock->iteratorTo(jmp); // iterator to last instruction
  lastBlock->splice(lastInst, target, target->skipHeader(), target->end());
  jmp->setTaken(nullptr); // unlink edge
  lastBlock->erase(lastInst); // delete the jmp
}

}

//////////////////////////////////////////////////////////////////////

void optimizeJumps(IRUnit& unit) {
  if (RuntimeOption::EvalHHIRDirectExit) {
    optimizeCondTraceExit(unit);
    optimizeSideExitChecks(unit);
    optimizeSideExitJccs(unit);
  }

  postorderWalk(unit, [&](Block* b) {
    auto branch = &b->back();
    auto taken = branch->taken();
    if (isTrivialJmp(branch, taken)) {
      eliminateJmp(b, branch, taken);
    }
  });
}

}}
