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
  DceFlags()
    : m_state(DEAD)
    , m_weakUseCount(0)
    , m_decRefNZ(false)
  {}

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

  /*
   * "Weak" uses are used in optimizeActRecs.
   *
   * If a frame pointer is used for something that can be modified to
   * not be a use as long as the whole frame can go away, we'll track
   * that here.
   */
  void incWeakUse() {
    if (m_weakUseCount + 1 > kMaxWeakUseCount) {
      always_assert(!"currently there's only one instruction "
                    "using this machinery, so this shouldn't "
                    "happen ... ");
      return;
    }
    ++m_weakUseCount;
  }

  int32_t weakUseCount() const {
    return m_weakUseCount;
  }

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
  uint8_t m_state:3;
  static constexpr uint8_t kMaxWeakUseCount = 15;
  uint8_t m_weakUseCount:4;
  bool m_decRefNZ:1;
};
static_assert(sizeof(DceFlags) == 1, "sizeof(DceFlags) should be 1 byte");

// DCE state indexed by instr->getIId.
typedef StateVector<IRInstruction, DceFlags> DceState;
typedef hphp_hash_set<const SSATmp*, pointer_hash<SSATmp>> SSASet;
typedef StateVector<SSATmp, SSASet> SSACache;
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
    copyProp(inst);
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
      // This assert is often hit when an instruction should have a
      // consumesReferences flag but doesn't.
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
    copyProp(inst);
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

  auto copyPropTrace = [] (Trace* trace) {
    forEachInst(trace, copyProp);
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
    copyPropTrace(exit);
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
  copyPropTrace(trace);
}

/*
 * Look for InlineReturn instructions that are the only "non-weak" use
 * of a DefInlineFP.  In this case we can kill both, which may allow
 * removing a SpillFrame as well.
 */
void optimizeActRecs(Trace* trace, DceState& state) {
  FTRACE(5, "AR:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "AR:^^^^^^^^^^^^^^^^^^^^^\n"); };

  bool killedFrames = false;

  forEachInst(trace, [&](IRInstruction* inst) {
    switch (inst->getOpcode()) {
    case DecRefKillThis:
      {
        auto frame = inst->getSrc(1);
        auto frameInst = frame->getInstruction();
        if (frameInst->getOpcode() == DefInlineFP) {
          FTRACE(5, "DecRefKillThis ({}): weak use of frame {}\n",
                 inst->getIId(),
                 frameInst->getIId());
          state[frameInst].incWeakUse();
        }
      }
      break;

    case InlineReturn:
      {
        auto frameUses = inst->getSrc(0)->getUseCount();
        auto srcInst = inst->getSrc(0)->getInstruction();
        if (srcInst->getOpcode() == DefInlineFP) {
          auto weakUses = state[srcInst].weakUseCount();
          // We haven't counted this InlineReturn as a weak use yet,
          // which is where this '1' comes from.
          if (frameUses - weakUses == 1) {
            FTRACE(5, "killing frame {}\n", srcInst->getIId());
            killedFrames = true;
            state[srcInst].setDead();
          }
        }
      }
      break;

    default:
      break;
    }
  });

  if (!killedFrames) return;

  /*
   * The first time through, we've counted up weak uses of the frame
   * and then finally marked it dead.  The instructions in between
   * that were weak uses may need modifications now that their frame
   * is going away.
   */
  forEachInst(trace, [&](IRInstruction* inst) {
    switch (inst->getOpcode()) {
    case DecRefKillThis:
      {
        auto fp = inst->getSrc(1);
        if (state[fp->getInstruction()].isDead()) {
          FTRACE(5, "DecRefKillThis ({}) -> DecRef\n", inst->getIId());
          inst->setOpcode(DecRef);
          inst->setSrc(1, nullptr);
          inst->setNumSrcs(1);
        }
      }
      break;

    case InlineReturn:
      {
        auto fp = inst->getSrc(0);
        if (state[fp->getInstruction()].isDead()) {
          FTRACE(5, "InlineReturn ({}) setDead\n", inst->getIId());
          state[inst].setDead();
        }
      }
      break;

    case DefInlineFP:
      FTRACE(5, "DefInlineFP ({}): weak/strong uses: {}/{}\n",
             inst->getIId(),
             state[inst].weakUseCount(),
             inst->getDst()->getUseCount());
      break;

    default:
      break;
    }
  });
}

// Assuming that the 'consumer' instruction consumes 'src', trace back through
// src's instruction to the real origin of the value. Currently this traces
// through GuardType and DefLabel.
void consumeIncRef(const IRInstruction* consumer, const SSATmp* src,
                   DceState& state, SSACache& ssas, SSASet visitedSrcs) {
  assert(!visitedSrcs.count(src) && "Cycle detected in dataflow graph");
  auto const& cache = ssas[src];
  if (!cache.empty()) {
    // We've already traced this path. Use the cache.
    for (const SSATmp* cached : cache) {
      consumeIncRef(consumer, cached, state, ssas, SSASet());
    }
    return;
  }

  const IRInstruction* srcInst = src->getInstruction();
  visitedSrcs.insert(src);
  if (srcInst->getOpcode() == GuardType &&
      srcInst->getTypeParam().maybeCounted()) {
    // srcInst is a GuardType that guards to a refcounted type. We need to
    // trace through to its source. If the GuardType guards to a non-refcounted
    // type then the reference is consumed by GuardType itself.
    consumeIncRef(consumer, srcInst->getSrc(0), state, ssas, visitedSrcs);
  } else if (srcInst->getOpcode() == DefLabel) {
    // srcInst is a DefLabel that may be a join node. We need to find
    // the dst index of src in srcInst and trace through to each jump
    // providing a value for it.
    for (unsigned i = 0, n = srcInst->getNumDsts(); i < n; ++i) {
      if (srcInst->getDst(i) == src) {
        srcInst->getBlock()->forEachSrc(i,
          [&](IRInstruction* jmp, SSATmp* val) {
            consumeIncRef(consumer, val, state, ssas, visitedSrcs);
          }
        );
        break;
      }
    }
  } else {
    // src is the canonical representation of everything in visitedSrcs. Put
    // that knowledge in the cache.
    for (const SSATmp* visited : visitedSrcs) {
      // We don't need to store the fact that src is its own canonical
      // representation.
      if (visited != src) {
        ssas[visited].insert(src);
      }
    }

    if (srcInst->getOpcode() == IncRef) {
      // <inst> consumes <srcInst> which is an IncRef, so we mark <srcInst> as
      // REFCOUNT_CONSUMED.
      if (consumer->getTrace()->isMain() || !srcInst->getTrace()->isMain()) {
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
  SSACache ssaOriginals(irFactory, SSASet());
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

      // If inst consumes this source, find the true source instruction and
      // mark it as consumed if it's an IncRef.
      if (inst->consumesReference(i)) {
        consumeIncRef(inst, src, state, ssaOriginals, SSASet());
      }
    }
  }

  // Optimize IncRefs and DecRefs.
  forEachTrace(trace, [&](Trace* t) { optimizeRefCount(t, state); });

  if (RuntimeOption::EvalHHIREnableSinking) {
    // Sink IncRefs consumed off trace.
    sinkIncRefs(trace, irFactory, state);
  }

  // Optimize unused inlined activation records.  It's not necessary
  // to look at non-main traces for this.
  optimizeActRecs(trace, state);

  // now remove instructions whose id == DEAD
  removeDeadInstructions(trace, state);
  for (Trace* exit : trace->getExitTraces()) {
    removeDeadInstructions(exit, state);
  }

  // and remove empty exit traces
  removeEmptyExitTraces();
}

} } }
