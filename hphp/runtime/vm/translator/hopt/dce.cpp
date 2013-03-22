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
namespace {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

/* DceFlags tracks the state of one instruction during dead code analysis. */
struct DceFlags {
  DceFlags() : m_state(DEAD), m_decRefNZ(false) {}
  bool isDead()                const { return m_state == DEAD; }
  bool countConsumed()         const { return m_state == REFCOUNT_CONSUMED; }
  bool countConsumedOffTrace() const {
    return m_state == REFCOUNT_CONSUMED_OFF_TRACE;
  }
  bool countConsumedAny()      const {
    return countConsumed() || countConsumedOffTrace();
  }
  bool decRefNZed()            const { return m_decRefNZ; }

  void setDead()                  { m_state = DEAD; }
  void setLive()                  { m_state = LIVE; }
  void setCountConsumed()         { m_state = REFCOUNT_CONSUMED; }
  void setCountConsumedOffTrace() { m_state = REFCOUNT_CONSUMED_OFF_TRACE; }
  void setDecRefNZed()            { m_decRefNZ = true; }

  std::string toString() const {
    static const char* const names[] = {
      "DEAD",
      "LIVE",
      "REFCOUNT_CONSUMED",
      "REFCOUNT_CONSUMED_OFF_TRACE",
    };
    return folly::format(
      "{} nz:{}",
      m_state > array_size(names) ? "<invalid>" : names[m_state],
      m_decRefNZ).str();
  }

private:
  /*
   * An IncRef is marked as consumed if it is a source for an instruction other
   * than DecRefNZ that accounts for the newly created reference, either by
   * decrementing the refcount, or by storing an additional reference to the
   * value to memory.
   * REFCOUNT_CONSUMED: consumed by such an instruction in the main trace.
   * REFCOUNT_CONSUMED_OFF_TRACE: consumed by such an instruction only in exits.
   * DecRefNZed: True iff the IncRef has been consumed by a DecRefNZ
   */
  enum {
    DEAD = 0,
    LIVE,
    REFCOUNT_CONSUMED,
    REFCOUNT_CONSUMED_OFF_TRACE,
  };
  uint8_t m_state:7;
  bool m_decRefNZ:1;
};
static_assert(sizeof(DceFlags) == 1, "sizeof(DceFlags) should be 1 byte");

// DCE state indexed by instr->getIId.
typedef StateVector<IRInstruction, DceFlags> DceState;
typedef std::list<const IRInstruction*> WorkList;

void removeDeadInstructions(Trace* trace, const DceState& state) {
  auto &blocks = trace->getBlocks();
  for (auto it = blocks.begin(), end = blocks.end(); it != end;) {
    auto cur = it; ++it;
    Block* block = *cur;
    block->remove_if([&] (const IRInstruction& inst) {
      return state[inst].isDead();
    });
    if (block->empty()) blocks.erase(cur);
  }
}

bool isUnguardedLoad(IRInstruction* inst) {
  if (!inst->hasDst() || !inst->getDst()) return false;
  Opcode opc = inst->getOpcode();
  SSATmp* dst = inst->getDst();
  Type type = dst->getType();
  return ((opc == LdStack && (type == Type::Gen || type == Type::Cell)) ||
          (opc == LdLoc && type == Type::Gen) ||
          (opc == LdRef && type == Type::Cell) ||
          (opc == LdMem && type == Type::Cell &&
           inst->getSrc(0)->getType() == Type::PtrToCell) ||
          (opc == Unbox && type == Type::Cell));
}

// removeUnreachable erases unreachable blocks from trace, and returns
// a sorted list of the remaining blocks.
BlockList removeUnreachable(Trace* trace, IRFactory* factory) {
  // 1. simplify unguarded loads to remove unnecssary branches, and
  //    perform copy propagation on every instruction. Targets that become
  //    unreachable from this pass will be eliminated in step 2 below.
  forEachTraceInst(trace, [](IRInstruction* inst) {
    Simplifier::copyProp(inst);
    // if this is a load that does not generate a guard, then get rid
    // of its label so that its not an essential control-flow
    // instruction
    if (isUnguardedLoad(inst)) {
      // LdStack and LdLoc instructions that produce generic types
      // and LdStack instruction that produce Cell types will not
      // generate guards, so remove the label from this instruction so
      // that it's no longer an essential control-flow instruction
      inst->setTaken(nullptr);
    }
  });

  // 2. get a list of reachable blocks by sorting them, and erase any
  //    blocks that are unreachable.
  BlockList blocks = sortCfg(trace, *factory);
  StateVector<Block, bool> reachable(factory, false);
  for (Block* b : blocks) reachable[b] = true;
  forEachTrace(trace, [&](Trace* t) {
    t->getBlocks().remove_if([&](Block* b) {
      return !reachable[b];
    });
  });

  return blocks;
}

WorkList
initInstructions(const BlockList& blocks, DceState& state) {
  TRACE(5, "DCE:vvvvvvvvvvvvvvvvvvvv\n");
  // mark reachable, essential, instructions live and enqueue them
  WorkList wl;
  for (Block* block : blocks) {
    for (IRInstruction& inst : *block) {
      for (SSATmp& dst : inst.getDsts()) {
        dst.setUseCount(0);
      }
      if (inst.isControlFlowInstruction()) {
        // mark the destination label so that the destination trace
        // is marked reachable
        state[inst.getTaken()->getLabel()].setLive();
      }
      if (inst.isEssential()) {
        state[inst].setLive();
        wl.push_back(&inst);
      }
      if (inst.getOpcode() == DecRefNZ) {
        auto* srcInst = inst.getSrc(0)->getInstruction();
        Opcode srcOpc = srcInst->getOpcode();
        if (srcOpc != DefConst) {
          assert(srcInst->getOpcode() == IncRef);
          assert(state[srcInst].isDead()); // IncRef isn't essential so it should
                                           // be dead here
          state[srcInst].setDecRefNZed();
        }
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
// 3) Eliminates IncRef-DecRef pairs who value is used only by the DecRef and
//    whose type does not run a destructor with side effects.
void optimizeRefCount(Trace* trace, DceState& state) {
  WorkList decrefs;
  forEachInst(trace, [&](IRInstruction* inst) {
    if (inst->getOpcode() == IncRef && !state[inst].countConsumedAny()) {
      auto& s = state[inst];
      always_assert_log(s.decRefNZed(), [&]{
        Trace* mainTrace = trace->isMain() ? trace : trace->getMain();
        return folly::format("\n{} has state {} in trace:\n{}{}\n",
               inst->toString(), s.toString(), mainTrace->toString(),
               trace == mainTrace ? "" : trace->toString()).str();
      });
      inst->setOpcode(Mov);
      s.setDead();
    }
    if (inst->getOpcode() == DecRefNZ) {
      SSATmp* src = inst->getSrc(0);
      IRInstruction* srcInst = src->getInstruction();
      if (state[srcInst].countConsumedAny()) {
        state[inst].setLive();
        src->incUseCount();
      }
    }
    if (inst->getOpcode() == DecRef) {
      SSATmp* src = inst->getSrc(0);
      if (src->getUseCount() == 1 && !src->getType().canRunDtor()) {
        IRInstruction* srcInst = src->getInstruction();
        if (srcInst->getOpcode() == IncRef) {
          decrefs.push_back(inst);
        }
      }
    }
    // Do copyProp at last. When processing DecRefNZs, we still need to look at
    // its source which should not be trampled over.
    Simplifier::copyProp(inst);
  });
  for (const IRInstruction* decref : decrefs) {
    assert(decref->getOpcode() == DecRef);
    SSATmp* src = decref->getSrc(0);
    assert(src->getInstruction()->getOpcode() == IncRef);
    assert(!src->getType().canRunDtor());
    if (src->getUseCount() == 1) {
      state[decref].setDead();
      state[src->getInstruction()].setDead();
    }
  }
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

  WorkList toSink;

  auto processExit = [&] (Trace* exit) {
    // Sink REFCOUNT_CONSUMED_OFF_TRACE IncRefs before the first non-label
    // instruction, and create a mapping between the original tmps to the sunk
    // tmps so that we can later replace the original ones with the sunk ones.
    std::vector<SSATmp*> sunkTmps(irFactory->numTmps(), nullptr);
    for (auto* inst : boost::adaptors::reverse(toSink)) {
      // prepend inserts an instruction to the beginning of a block, after
      // the label. Therefore, we iterate through toSink in the reversed order.
      IRInstruction* sunkInst = irFactory->gen(IncRef, inst->getSrc(0));
      state[sunkInst].setLive();
      exit->front()->prepend(sunkInst);

      auto dstId = inst->getDst()->getId();
      assert(!sunkTmps[dstId]);
      sunkTmps[dstId] = sunkInst->getDst();
    }
    forEachInst(exit, [&](IRInstruction* inst) {
      // Replace the original tmps with the sunk tmps.
      for (uint32_t i = 0; i < inst->getNumSrcs(); ++i) {
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
      if (state[inst].countConsumedOffTrace()) {
        inst->setOpcode(Mov);
        // Mark them as dead so that they'll be removed later.
        state[inst].setDead();
        // Put all REFCOUNT_CONSUMED_OFF_TRACE IncRefs to the sinking list.
        toSink.push_back(inst);
      } else if (!state[inst].isDead()) {
        assert(state[inst].countConsumed());
      }
    }
    if (inst->getOpcode() == DecRefNZ) {
      IRInstruction* srcInst = inst->getSrc(0)->getInstruction();
      if (state[srcInst].isDead()) {
        state[inst].setDead();
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

} // anonymous namespace

// Publicly exported functions:

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

void eliminateDeadCode(Trace* trace, IRFactory* irFactory) {
  auto removeEmptyExitTraces = [&] {
    trace->getExitTraces().remove_if([](Trace* exit) {
      return exit->getBlocks().empty();
    });
  };

  // kill unreachable code and remove any traces that are now empty
  BlockList blocks = removeUnreachable(trace, irFactory);
  removeEmptyExitTraces();

  // mark the essential instructions and add them to the initial
  // work list; this will also mark reachable exit traces. All
  // other instructions marked dead.
  DceState state(irFactory, DceFlags());
  WorkList wl = initInstructions(blocks, state);

  // process the worklist
  while (!wl.empty()) {
    auto* inst = wl.front();
    wl.pop_front();
    for (uint32_t i = 0; i < inst->getNumSrcs(); i++) {
      SSATmp* src = inst->getSrc(i);
      IRInstruction* srcInst = src->getInstruction();
      if (srcInst->getOpcode() == DefConst) {
        continue;
      }
      src->incUseCount();
      if (state[srcInst].isDead()) {
        state[srcInst].setLive();
        wl.push_back(srcInst);
      }
      // <inst> consumes <srcInst> which is an IncRef, so we mark <srcInst> as
      // REFCOUNT_CONSUMED. If the source instruction is a GuardType and guards
      // to a maybeCounted type, we need to trace through to the source for
      // refcounting purposes.
      while (srcInst->getOpcode() == GuardType &&
             srcInst->getTypeParam().maybeCounted()) {
        srcInst = srcInst->getSrc(0)->getInstruction();
      }
      if (inst->consumesReference(i) && srcInst->getOpcode() == IncRef) {
        if (inst->getTrace()->isMain() || !srcInst->getTrace()->isMain()) {
          // <srcInst> is consumed from its own trace.
          state[srcInst].setCountConsumed();
        } else {
          // <srcInst> is consumed off trace.
          if (!state[srcInst].countConsumed()) {
            // mark <srcInst> as REFCOUNT_CONSUMED_OFF_TRACE unless it is
            // also consumed from its own trace.
            state[srcInst].setCountConsumedOffTrace();
          }
        }
      }
    }
  }

  // Optimize IncRefs and DecRefs.
  forEachTrace(trace, [&](Trace* t) { optimizeRefCount(t, state); });

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
  removeEmptyExitTraces();
}

} } }
