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

#include "util/trace.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/opt.h"
#include "runtime/vm/translator/hopt/irfactory.h"
#include "runtime/vm/translator/hopt/simplifier.h"
#include <boost/range/adaptors.hpp>

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

/*
 * An IncRef is marked as REFCOUNT_CONSUMED[_OFF_TRACE], if it is consumed by
 * an instruction other than DecRefNZ that decrements the ref count.
 * REFCOUNT_CONSUMED: consumed by such an instruction in the main trace.
 * REFCOUNT_CONSUMED_OFF_TRACE: consumed by such an instruction only in exits.
 */
enum DceFlags : uint8_t {
  DEAD,
  LIVE,
  REFCOUNT_CONSUMED,
  REFCOUNT_CONSUMED_OFF_TRACE
};

// DCE state indexed by instr->getIId.
typedef StateVector<IRInstruction, DceFlags> DceState;

void removeDeadInstructions(Trace* trace, const boost::dynamic_bitset<>& live) {
  auto &blocks = trace->getBlocks();
  for (auto it = blocks.begin(), end = blocks.end(); it != end;) {
    auto cur = it; ++it;
    Block* block = *cur;
    block->remove_if([&] (const IRInstruction& inst) {
      assert(inst.getIId() < live.size());
      return !live.test(inst.getIId());
    });
    if (block->empty()) blocks.erase(cur);
  }
}

void removeDeadInstructions(Trace* trace, const DceState& state) {
  auto &blocks = trace->getBlocks();
  for (auto it = blocks.begin(), end = blocks.end(); it != end;) {
    auto cur = it; ++it;
    Block* block = *cur;
    block->remove_if([&] (const IRInstruction& inst) {
      return state[inst] == DEAD;
    });
    if (block->empty()) blocks.erase(cur);
  }
}

bool isUnguardedLoad(IRInstruction* inst) {
  if (!inst->hasDst() || !inst->getDst()) return false;
  Opcode opc = inst->getOpcode();
  SSATmp* dst = inst->getDst();
  Type type = dst->getType();
  return (opc == LdStack && (type == Type::Gen || type == Type::Cell))
          || (opc == LdLoc && type == Type::Gen)
          || (opc == LdRef && type == Type::Cell)
          || (opc == LdMem && type == Type::Cell &&
              inst->getSrc(0)->getType() == Type::PtrToCell);
}

std::list<IRInstruction*> initInstructions(Trace* trace, DceState& state,
                                           IRFactory* factory) {
  TRACE(5, "DCE:vvvvvvvvvvvvvvvvvvvv\n");
  // 1. simplify unguarded loads to remove unnecssary branches, and
  //    perform copy propagation on every instruction.  Targets that become
  //    unreachable from this pass won't be visited in step 2 below.
  forEachTraceInst(trace, [](IRInstruction* inst) {
    Simplifier::copyProp(inst);
    // if this is a load that does not generate a guard, then get rid
    // of its label so that its not an essential control-flow
    // instruction
    if (isUnguardedLoad(inst)) {
      // LdStack and LdLoc instructions that produce generic types
      // and LdStack instruction that produce Cell types will not
      // generate guards, so remove the label from this instruction so
      // that its no longer an essential control-flow instruction
      inst->setTaken(nullptr);
    }
  });

  // 2. mark reachable, essential, instructions live and enqueue them
  std::list<IRInstruction*> wl;
  BlockList blocks = sortCfg(trace, *factory);
  for (Block* block : blocks) {
    for (IRInstruction& inst : *block) {
      if (inst.isControlFlowInstruction()) {
        // mark the destination label so that the destination trace
        // is marked reachable
        state[inst.getTaken()->getLabel()] = LIVE;
      }
      if (inst.isEssential()) {
        state[inst] = LIVE;
        wl.push_back(&inst);
      }
    }
  }
  TRACE(5, "DCE:^^^^^^^^^^^^^^^^^^^^\n");
  return wl;
}

// Perform the following transformations:
// 1) Change all unconsumed IncRefs to Mov.
// 2) Mark a conditionally dead DecRefNZ as live if its corresponding IncRef
//    cannot be eliminated.
void optimizeRefCount(Trace* trace, DceState& state) {
  forEachInst(trace, [&](IRInstruction* inst) {
    if (inst->getOpcode() == IncRef &&
        state[inst] != REFCOUNT_CONSUMED &&
        state[inst] != REFCOUNT_CONSUMED_OFF_TRACE) {
      inst->setOpcode(Mov);
      state[inst] = DEAD;
    }
    if (inst->getOpcode() == DecRefNZ) {
      IRInstruction* srcInst = inst->getSrc(0)->getInstruction();
      if (state[srcInst] == REFCOUNT_CONSUMED ||
          state[srcInst] == REFCOUNT_CONSUMED_OFF_TRACE) {
        state[inst] = LIVE;
      }
    }
    // Do copyProp at last. When processing DecRefNZs, we still need to look at
    // its source which should not be trampled over.
    Simplifier::copyProp(inst);
  });
}

/*
 * Sink IncRefs consumed off trace.
 * Assumptions: Flow graph must not have critical edges, and the instructions
 * have been annotated already by the DCE algorithm.  This pass uses
 * the REFCOUNT_CONSUMED* flags to copy IncRefs from the main trace to each
 * exit trace that consumes the incremented pointer.
 * 1. toSink = {}
 * 2. iterate forwards over the main trace:
 *    * when a movable IncRef is found, insert into toSink list and mark
 *      it as DEAD.
 *    * If a decref of a dead incref is found, remove the corresponding
 *      incref from toSink, and mark the decref DEAD because too.
 *    * the first time we see a branch to an exit trace, process the
 *      exit tace.
 * 3. to process an exit trace:
 *    * clone each IncRef found in toSink then prepend to the exit trace.
 *    * replace each use of the original incref's result with the new
 *      incref's result.
 */
void sinkIncRefs(Trace* trace, IRFactory* irFactory, DceState& state) {
  assert(trace->isMain());

  auto copyProp = [] (Trace* trace) {
    forEachInst(trace, Simplifier::copyProp);
  };

  std::list<IRInstruction*> toSink;

  auto processExit = [&] (Trace* exit) {
    // Sink REFCOUNT_CONSUMED_OFF_TRACE IncRefs before the first non-label
    // instruction, and create a mapping between the original tmps to the sunk
    // tmps so that we can later replace the original ones with the sunk ones.
    std::vector<SSATmp*> sunkTmps(irFactory->numTmps(), nullptr);
    for (auto* inst : boost::adaptors::reverse(toSink)) {
      // prepend inserts an instruction to the beginning of a block, after
      // the label. Therefore, we iterate through toSink in the reversed order.
      IRInstruction* sunkInst = irFactory->gen(IncRef, inst->getSrc(0));
      state[sunkInst] = LIVE;
      exit->front()->prepend(sunkInst);

      auto dstId = inst->getDst()->getId();
      assert(!sunkTmps[dstId]);
      sunkTmps[dstId] = sunkInst->getDst();
    }
    forEachInst(exit, [&](IRInstruction* inst) {
      // Replace the original tmps with the sunk tmps.
      for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
        SSATmp* src = inst->getSrc(i);
        if (SSATmp* sunkTmp = sunkTmps[src->getId()]) {
          inst->setSrc(i, sunkTmp);
        }
      }
    });
    // Do copyProp at last, because we need to keep REFCOUNT_CONSUMED_OFF_TRACE
    // Movs as the prototypes for sunk instructions.
    copyProp(exit);
  };

  // An exit trace may be entered from multiple exit points. We keep track of
  // which exit traces we already pushed sunk IncRefs to, so that we won't push
  // them multiple times.
  boost::dynamic_bitset<> pushedTo(irFactory->numBlocks());
  forEachInst(trace, [&](IRInstruction* inst) {
    if (inst->getOpcode() == IncRef) {
      // Must be REFCOUNT_CONSUMED or REFCOUNT_CONSUMED_OFF_TRACE;
      // otherwise, it should be already removed in optimizeRefCount.
      if (state[inst] == REFCOUNT_CONSUMED_OFF_TRACE) {
        inst->setOpcode(Mov);
        // Mark them as dead so that they'll be removed later.
        state[inst] = DEAD;
        // Put all REFCOUNT_CONSUMED_OFF_TRACE IncRefs to the sinking list.
        toSink.push_back(inst);
      } else {
        assert(state[inst] == REFCOUNT_CONSUMED);
      }
    }
    if (inst->getOpcode() == DecRefNZ) {
      IRInstruction* srcInst = inst->getSrc(0)->getInstruction();
      if (state[srcInst] == DEAD) {
        state[inst] = DEAD;
        // This may take O(I) time where I is the number of IncRefs
        // in the main trace.
        toSink.remove(srcInst);
      }
    }
    if (Block* target = inst->getTaken()) {
      if (!pushedTo[target->getId()]) {
        pushedTo[target->getId()] = 1;
        Trace* exit = target->getTrace();
        if (exit != trace) processExit(exit);
      }
    }
  });

  // Do copyProp at last, because we need to keep REFCOUNT_CONSUMED_OFF_TRACE
  // Movs as the prototypes for sunk instructions.
  copyProp(trace);
}

void eliminateDeadCode(Trace* trace, IRFactory* irFactory) {
  // mark the essential instructions and add them to the initial
  // work list; this will also mark reachable exit traces. All
  // other instructions marked dead.
  DceState state(irFactory, DEAD);
  std::list<IRInstruction*> wl = initInstructions(trace, state, irFactory);

  // process the worklist
  while (!wl.empty()) {
    IRInstruction* inst = wl.front();
    wl.pop_front();
    for (uint32 i = 0; i < inst->getNumSrcs(); i++) {
      SSATmp* src = inst->getSrc(i);
      if (src->getInstruction()->getOpcode() == DefConst) {
        continue;
      }
      IRInstruction* srcInst = src->getInstruction();
      if (state[srcInst] == DEAD) {
        state[srcInst] = LIVE;
        wl.push_back(srcInst);
      }
      // <inst> consumes <srcInst> which is an IncRef,
      // so we mark <srcInst> as REFCOUNT_CONSUMED.
      if (inst->consumesReference(i) && srcInst->getOpcode() == IncRef) {
        if (inst->getTrace()->isMain() || !srcInst->getTrace()->isMain()) {
          // <srcInst> is consumed from its own trace.
          state[srcInst] = REFCOUNT_CONSUMED;
        } else {
          // <srcInst> is consumed off trace.
          if (state[srcInst] != REFCOUNT_CONSUMED) {
            // mark <srcInst> as REFCOUNT_CONSUMED_OFF_TRACE unless it is
            // also consumed from its own trace.
            state[srcInst] = REFCOUNT_CONSUMED_OFF_TRACE;
          }
        }
      }
    }
  }

  // Optimize IncRefs and DecRefs.
  optimizeRefCount(trace, state);
  for (Trace* exit : trace->getExitTraces()) {
    optimizeRefCount(exit, state);
  }

  if (RuntimeOption::EvalHHIREnableSinking) {
    // Sink IncRefs consumed off trace.
    sinkIncRefs(trace, irFactory, state);
  }

  // now remove instructions whose id == DEAD
  removeDeadInstructions(trace, state);
  for (Trace* exit : trace->getExitTraces()) {
    removeDeadInstructions(exit, state);
  }

  // and remove empty exit traces
  trace->getExitTraces().remove_if([](Trace* exit) {
    return exit->getBlocks().empty();
  });
}

} } }
