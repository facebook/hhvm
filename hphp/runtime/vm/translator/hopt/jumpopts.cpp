/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/translator/hopt/ir.h"
#include "hphp/runtime/vm/translator/hopt/opt.h"
#include "hphp/runtime/vm/translator/hopt/irfactory.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

// If main trace ends with an unconditional jump, and the target is not
// reached by any other branch, then copy the target of the jump to the
// end of the trace
void elimUnconditionalJump(Trace* trace, IRFactory* irFactory) {
  boost::dynamic_bitset<> isJoin(irFactory->numBlocks());
  boost::dynamic_bitset<> havePred(irFactory->numBlocks());
  for (Block* block : trace->blocks()) {
    if (block->taken()) {
      auto id = block->taken()->id();
      isJoin[id] = havePred[id];
      havePred[id] = 1;
    }
    if (block->next()) {
      auto id = block->next()->id();
      isJoin[id] = havePred[id];
      havePred[id] = 1;
    }
  }
  Block* lastBlock = trace->back();
  auto lastInst = lastBlock->backIter(); // iterator to last instruction
  IRInstruction& jmp = *lastInst;
  if (jmp.op() == Jmp_ && !isJoin[jmp.taken()->id()]) {
    Block* target = jmp.taken();
    lastBlock->splice(lastInst, target, target->skipHeader(), target->end());
    lastBlock->erase(lastInst); // delete the jmp
  }
}

Block* findMainExitBlock(Trace* trace, IRFactory* irFactory) {
  assert(trace->isMain());
  auto const back = trace->back();

  /*
   * We require the invariant that the main trace exit comes last in
   * the main trace block list.  Right now this is always the case,
   * but this assertion is here in case we want to make changes that
   * affect this ordering.  (If we do want to change it, we could use
   * something like the assert below to find the main exit.)
   */
  if (debug) {
    auto const sorted = rpoSortCfg(trace, *irFactory);
    auto it = sorted.rbegin();
    while (it != sorted.rend() && !(*it)->isMain()) {
      ++it;
    }
    assert(it != sorted.rend());
    assert(*it == back && "jumpopts invariant violated");
  }

  return back;
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
    while (m_it != m_block->end() && m_it->op() == Marker) ++m_it;
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
void optimizeCondTraceExit(Trace* trace, IRFactory* irFactory) {
  FTRACE(5, "CondExit:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "CondExit:^^^^^^^^^^^^^^^^^^^^^\n"); };

  auto const mainExit     = findMainExitBlock(trace, irFactory);
  if (!isNormalExit(mainExit)) return;

  auto const& mainPreds = mainExit->preds();
  if (mainPreds.size() != 1) return;

  auto const jccBlock = mainPreds.front().from();
  if (!jccCanBeDirectExit(jccBlock->back()->op())) return;
  FTRACE(5, "previous block ends with jccCanBeDirectExit ({})\n",
         opcodeName(jccBlock->back()->op()));

  auto const jccInst = jccBlock->back();
  auto const jccExitTrace = jccInst->taken();
  if (!isNormalExit(jccExitTrace)) return;
  FTRACE(5, "exit trace is side-effect free\n");

  auto const newOpcode = jmpToReqBindJmp(jccBlock->back()->op());
  ReqBindJccData data;
  data.taken = jccExitTrace->back()->extra<ReqBindJmp>()->offset;
  data.notTaken = mainExit->back()->extra<ReqBindJmp>()->offset;

  FTRACE(5, "replacing {} with {}\n", jccInst->id(), opcodeName(newOpcode));
  irFactory->replace(
    mainExit->back(),
    newOpcode,
    data,
    std::make_pair(jccInst->numSrcs(), jccInst->srcs().begin())
  );

  jccInst->convertToNop();
}

/*
 * Look for CheckStk/CheckLoc instructions in the main trace that
 * branch to "normal exits".  We can optimize these into the
 * SideExitGuard* instructions that can be patched in place.
 */
void optimizeSideExits(Trace* trace, IRFactory* irFactory) {
  FTRACE(5, "SideExit:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "SideExit:^^^^^^^^^^^^^^^^^^^^^\n"); };

  forEachInst(trace, [&] (IRInstruction* inst) {
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
    data.taken = exitBlock->back()->extra<ReqBindJmp>()->offset;

    auto const block = inst->block();
    block->insert(block->iteratorTo(inst),
                  irFactory->cloneInstruction(syncABI));

    irFactory->replace(
      inst,
      isStack ? SideExitGuardStk : SideExitGuardLoc,
      inst->typeParam(),
      data,
      isStack ? sp : fp
    );
  });
}

}

//////////////////////////////////////////////////////////////////////

void optimizeJumps(Trace* trace, IRFactory* irFactory) {
  elimUnconditionalJump(trace, irFactory);

  if (RuntimeOption::EvalHHIRDirectExit) {
    optimizeCondTraceExit(trace, irFactory);
    optimizeSideExits(trace, irFactory);
  }
}

}}
