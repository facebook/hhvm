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

#include <iterator>
#include <utility>

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

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
  return BlockMatcher(block).match(ReqBindJmp);
}

/*
 * An "adjusted" normal exit adjusts rVmSp before doing a ReqBindJmp.
 */
int32_t isAdjustedNormalExit(Block* block) {
  return BlockMatcher(block).match(AdjustSP, ReqBindJmp);
}

/*
 * Returns whether `opc' is a within-tracelet conditional jump that
 * can be folded into a ReqBindJmpFoo instruction.
 */
bool jccCanBeDirectExit(Opcode opc) {
  switch (opc) {
  case JmpZero:
  case JmpNZero:
    return true;
  default:
    return false;
  }
}

/*
 * Return true if jccInst is a conditional jump with no side effects
 * on exit, and both successors of the jcc are normal exits.
 */
bool isCondTraceExit(IRInstruction* jccInst, Block* jccExitBlock) {
  auto mainExit = jccInst->next();
  if (!(jccCanBeDirectExit(jccInst->op()) &&
        mainExit &&
        mainExit->isExit() &&
        mainExit->numPreds() == 1)) {
    return false;
  }
  if (isNormalExit(mainExit) && isNormalExit(jccExitBlock)) {
    return true;
  }
  if (isAdjustedNormalExit(mainExit) && isAdjustedNormalExit(jccExitBlock)) {
    // Adjusted exits need to make the same adjustment (to the same stack
    // pointer) or we can't do the optimization.
    auto& mainAdju = *std::prev(mainExit->backIter());
    auto& jccAdju  = *std::prev(jccExitBlock->backIter());
    assert(mainAdju.op() == AdjustSP && jccAdju.op() == AdjustSP);
    return
      mainAdju.extra<AdjustSP>()->offset ==
        jccAdju.extra<AdjustSP>()->offset &&
      mainAdju.src(0) == jccAdju.src(0);
  }
  return false;
}

Opcode jmpToReqBindJmp(Opcode opc) {
  switch (opc) {
  case JmpZero:               return ReqBindJmpZero;
  case JmpNZero:              return ReqBindJmpNZero;
  default:                    always_assert(0);
  }
}

/*
 * Convert a conditional branch that leads to two normal exits to a single
 * conditional ReqBindJmp instruction; then delete the unnecessary branch
 * and exit block.
 *
 * This leads to more efficient code because the service request stubs will
 * patch jumps in the main trace instead of off-trace.
 */
void optimizeCondTraceExit(IRUnit& unit, IRInstruction* jccInst,
                           Block* jccExitBlock) {
  assert(isCondTraceExit(jccInst, jccExitBlock));
  FTRACE(5, "CondExit:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "CondExit:^^^^^^^^^^^^^^^^^^^^^\n"); };

  FTRACE(5, "block ends with jccCanBeDirectExit ({})\n",
         opcodeName(jccInst->op()));
  FTRACE(5, "exit trace is side-effect free\n");

  auto mainExit = jccInst->next();
  auto it = mainExit->backIter();
  auto& reqBindJmp = *(it--);

  auto const newOpcode = jmpToReqBindJmp(jccInst->op());
  ReqBindJccData data;
  data.taken = jccExitBlock->back().extra<ReqBindJmp>()->dest;
  data.notTaken = reqBindJmp.extra<ReqBindJmp>()->dest;

  auto argVec = jit::vector<SSATmp*>(jccInst->srcs().begin(),
                                     jccInst->srcs().end());
  argVec.push_back(reqBindJmp.src(0));

  FTRACE(5, "replacing {} with {}\n", jccInst->id(), opcodeName(newOpcode));
  unit.replace(
    &reqBindJmp,
    newOpcode,
    data,
    std::make_pair(argVec.size(), &argVec[0])
  );

  reqBindJmp.setMarker(jccInst->marker());
  unit.replace(jccInst, Jmp, mainExit);
}

// Return true if this block ends with a trivial Jmp: a Jmp
// whose target's only predecessor is b.
bool isTrivialJmp(IRInstruction* branch, Block* taken) {
  return branch->op() == Jmp && taken->numPreds() == 1;
}

// Coalesce two blocks joined by a trivial jump by moving the second block's
// instructions to the first block and deleting the jump.  If the second block
// starts with BeginCatch or DefLabel, they will also be deleted.
void eliminateJmp(Block* lastBlock, IRInstruction* jmp, Block* target,
                  IRUnit& unit) {
  assert(isTrivialJmp(jmp, target));
  auto lastInst = lastBlock->iteratorTo(jmp); // iterator to last instruction
  if (jmp->numSrcs() != 0) {
    auto& defLabel = target->front();
    assert(defLabel.numDsts() == jmp->numSrcs());
    for (auto i = 0; i < jmp->numSrcs(); i++) {
      lastBlock->insert(lastInst,
                        unit.genWithDst(defLabel.dst(i), Mov,
                                        jmp->marker(), jmp->src(i)));
    }
  }
  lastInst = lastBlock->iteratorTo(jmp); // iterator to last instruction
  lastBlock->splice(lastInst, target, target->skipHeader(), target->end());
  jmp->setTaken(nullptr); // unlink edge
  lastBlock->erase(lastInst); // delete the jmp
}

}

//////////////////////////////////////////////////////////////////////

void optimizeJumps(IRUnit& unit) {
  Timer _t(Timer::optimize_jumpOpts);

  postorderWalk(unit, [&](Block* b) {
    if (RuntimeOption::EvalHHIRDirectExit) {
      auto branch = &b->back();
      auto taken = branch->taken();
      if (isCondTraceExit(branch, taken)) {
        optimizeCondTraceExit(unit, branch, taken);
      }
    }
    auto branch = &b->back();
    auto taken = branch->taken();
    if (isTrivialJmp(branch, taken)) {
      eliminateJmp(b, branch, taken, unit);
    }
  });
}

}}
