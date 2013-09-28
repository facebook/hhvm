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

#include <boost/range/adaptors.hpp>

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/mutation.h"

namespace HPHP {
namespace JIT {
namespace {

TRACE_SET_MOD(hhir);

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
      // Too many weak uses for us to know we can optimize it away.
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

// DCE state indexed by instr->id().
typedef StateVector<IRInstruction, DceFlags> DceState;
typedef StateVector<SSATmp, uint32_t> UseCounts;
typedef smart::list<const IRInstruction*> WorkList;

void removeDeadInstructions(IRTrace* trace, const DceState& state) {
  auto &blocks = trace->blocks();
  for (auto it = blocks.begin(), end = blocks.end(); it != end;) {
    auto cur = it; ++it;
    Block* block = *cur;
    block->remove_if([&] (const IRInstruction& inst) {
      ONTRACE(7,
              if (state[inst].isDead()) {
                FTRACE(3, "Removing dead instruction {}\n", inst.toString());
              });
      return state[inst].isDead();
    });
    if (block->empty()) {
      // Update any predecessors to point to the empty block's next block.
      auto next = block->next();
      for (auto it = block->preds().begin(); it != block->preds().end(); ) {
        auto cur = it;
        ++it;
        cur->setTo(next);
      }
      trace->erase(cur);
    }
  }
}

bool isUnguardedLoad(IRInstruction* inst) {
  if (!inst->hasDst() || !inst->dst()) return false;
  Opcode opc = inst->op();
  SSATmp* dst = inst->dst();
  Type type = dst->type();
  return ((opc == LdStack && (type == Type::Gen || type == Type::Cell)) ||
          (opc == LdLoc && type == Type::Gen) ||
          (opc == LdRef && type == Type::Cell) ||
          (opc == LdMem && type == Type::Cell &&
           inst->src(0)->type() == Type::PtrToCell) ||
          (opc == Unbox && type == Type::Cell));
}

// removeUnreachable erases unreachable blocks from trace, and returns
// a sorted list of the remaining blocks.
BlockList removeUnreachable(IRTrace* trace, IRUnit& unit) {
  FTRACE(5, "RemoveUnreachable:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "RemoveUnreachable:^^^^^^^^^^^^^^^^^^^^\n"); };

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
  bool needsReflow = false;
  BlockList blocks = rpoSortCfg(trace, unit);
  StateVector<Block, bool> reachable(unit, false);
  for (Block* b : blocks) reachable[b] = true;
  for (Block* b : blocks) {
    b->forEachPred([&](Block *p) {
      if (!reachable[p]) {
        // remove edges from unreachable block to reachable block.
        if (!p->empty()) p->back()->setTaken(nullptr);
        p->setNext(nullptr);
      }
    });
  }
  forEachTrace(trace, [&](IRTrace* t) {
    for (auto bit = t->begin(); bit != t->end();) {
      if (reachable[*bit]) {
        ++bit;
        continue;
      }
      FTRACE(5, "erasing block {}\n", (*bit)->id());
      if ((*bit)->taken() && (*bit)->back()->op() == Jmp_) {
        needsReflow = true;
      }
      bit = t->erase(bit);
    }
  });

  // 3. if we removed any whole blocks that ended in Jmp_
  //    instructions, reflow all types in case they change the
  //    incoming types of DefLabel instructions.
  if (needsReflow) reflowTypes(blocks.front(), blocks);

  return blocks;
}

WorkList
initInstructions(const BlockList& blocks, DceState& state) {
  TRACE(5, "DCE(initInstructions):vvvvvvvvvvvvvvvvvvvv\n");
  // mark reachable, essential, instructions live and enqueue them
  WorkList wl;
  for (Block* block : blocks) {
    for (IRInstruction& inst : *block) {
      if (inst.isEssential()) {
        state[inst].setLive();
        wl.push_back(&inst);
      }
      if (inst.op() == DecRefNZ) {
        auto* srcInst = inst.src(0)->inst();
        Opcode srcOpc = srcInst->op();
        if (srcOpc != DefConst) {
          assert(srcInst->op() == IncRef);
          assert(state[srcInst].isDead()); // IncRef isn't essential so it
                                           // should be dead here
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
void optimizeRefCount(IRTrace* trace, IRTrace* main, DceState& state,
                      UseCounts& uses) {
  assert(trace && main->isMain());
  WorkList decrefs;
  forEachInst(trace, [&](IRInstruction* inst) {
    if (inst->op() == IncRef && !state[inst].countConsumedAny()) {
      // This assert is often hit when an instruction should have a
      // consumesReferences flag but doesn't.
      auto& s = state[inst];
      always_assert_log(s.decRefNZed(), [&]{
        return folly::format("\n{} has state {} in trace:\n{}{}\n",
               inst->toString(), s.toString(), main->toString(),
               trace == main ? "" : trace->toString()).str();
      });
      inst->setOpcode(Mov);
      s.setDead();
    }
    if (inst->op() == DecRefNZ) {
      SSATmp* src = inst->src(0);
      IRInstruction* srcInst = src->inst();
      if (state[srcInst].countConsumedAny()) {
        state[inst].setLive();
        uses[src]++;
      }
    }
    if (inst->op() == DecRef) {
      SSATmp* src = inst->src(0);
      if (uses[src] == 1 && !src->type().canRunDtor()) {
        IRInstruction* srcInst = src->inst();
        if (srcInst->op() == IncRef) {
          decrefs.push_back(inst);
        }
      }
    }
    // Do copyProp at last. When processing DecRefNZs, we still need to look at
    // its source which should not be trampled over.
    copyProp(inst);
  });
  for (const IRInstruction* decref : decrefs) {
    assert(decref->op() == DecRef);
    SSATmp* src = decref->src(0);
    assert(src->inst()->op() == IncRef);
    assert(!src->type().canRunDtor());
    if (uses[src] == 1) {
      state[decref].setDead();
      state[src->inst()].setDead();
    }
  }
}

/*
 * Sink IncRefs consumed off trace.
 * Assumptions:
 *   a) Flow graph must not have critical edges.
 *   b) The main trace doesn't unconditionally jump to an exit trace.
 *   c) The instructions have been annotated already by the DCE algorithm.
 *      This pass uses the REFCOUNT_CONSUMED* flags to copy IncRefs from the
 *      main trace to each exit trace that consumes the incremented pointer.
 *
 * Algorithm:
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
void sinkIncRefs(IRTrace* trace, IRUnit& unit, DceState& state) {
  assert(trace->isMain());

  assert(trace->back()->back()->op() != Jmp_);

  auto copyPropTrace = [] (IRTrace* trace) {
    forEachInst(trace, copyProp);
  };

  WorkList toSink;

  // Hoisted outside the loop to reduce allocations.
  smart::flat_map<SSATmp*,SSATmp*> sunkTmps;

  auto processExit = [&] (IRTrace* exit) {
    // Sink REFCOUNT_CONSUMED_OFF_TRACE IncRefs before the first non-label
    // instruction, and create a mapping between the original tmps to the sunk
    // tmps so that we can later replace the original ones with the sunk ones.
    sunkTmps.clear();
    for (auto* inst : boost::adaptors::reverse(toSink)) {
      // prepend inserts an instruction to the beginning of a block, after
      // the label. Therefore, we iterate through toSink in the reversed order.
      auto* sunkInst = unit.gen(IncRef, inst->marker(), inst->src(0));
      state[sunkInst].setLive();
      exit->front()->prepend(sunkInst);

      assert(!sunkTmps[inst->dst()]);
      sunkTmps[inst->dst()] = sunkInst->dst();
    }
    forEachInst(exit, [&](IRInstruction* inst) {
      // Replace the original tmps with the sunk tmps.
      for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
        SSATmp* src = inst->src(i);
        auto it = sunkTmps.find(src);
        if (it != sunkTmps.end()) {
          inst->setSrc(i, it->second);
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
  boost::dynamic_bitset<> pushedTo(unit.numBlocks());
  forEachInst(trace, [&](IRInstruction* inst) {
    if (inst->op() == IncRef) {
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
    if (inst->op() == DecRefNZ) {
      IRInstruction* srcInst = inst->src(0)->inst();
      if (state[srcInst].isDead()) {
        state[inst].setDead();
        // This may take O(I) time where I is the number of IncRefs
        // in the main trace.
        toSink.remove(srcInst);
      }
    }
    if (Block* target = inst->taken()) {
      if (!pushedTo[target->id()]) {
        pushedTo[target->id()] = 1;
        IRTrace* exit = target->trace();
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
void optimizeActRecs(IRTrace* trace, DceState& state, IRUnit& unit,
                     UseCounts& uses) {
  FTRACE(5, "AR:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "AR:^^^^^^^^^^^^^^^^^^^^^\n"); };

  bool killedFrames = false;

  smart::map<SSATmp*, Offset> retFixupMap;
  forEachInst(trace, [&](IRInstruction* inst) {
    if (!state[inst].isDead()) return;
    switch (inst->op()) {
    // We don't need to generate stores to a frame if it can be
    // eliminated.
    case StLoc:
      {
        auto const frameInst = inst->src(0)->inst();
        if (frameInst->op() == DefInlineFP) {
          FTRACE(5, "Marking StLoc for {}\n", frameInst->id());
          state[frameInst].incWeakUse();
        }
      }
      break;

    case SpillFrame:
      {
        auto const frameInst = inst->src(1)->inst();
        auto const reason = inst->src(2);
        if (frameInst->op() == DefInlineFP &&
            reason->type().subtypeOf(Type::Func)) {
            FTRACE(5, "Marking SpillFrame for {}\n", frameInst->id());
          state[frameInst].incWeakUse();
        }
      }
      break;

    case ReDefSP:
    case DefInlineSP:
      {
        auto const frameInst = inst->src(0)->inst();
        if (frameInst->op() == DefInlineFP) {
          FTRACE(5, "Marking DefSP for {}\n", frameInst->id());
          state[frameInst].incWeakUse();
        }
      }
      break;

    case InlineReturn:
      {
        auto frameUses = uses[inst->src(0)];
        auto srcInst = inst->src(0)->inst();
        if (srcInst->op() == DefInlineFP) {
          auto weakUses = state[srcInst].weakUseCount();
          // We haven't counted this InlineReturn as a weak use yet,
          // which is where this '1' comes from.
          if (frameUses - weakUses == 1) {
            FTRACE(5, "killing frame {}\n", srcInst->id());
            killedFrames = true;
            auto const stkInst = srcInst->src(0)->inst();

            Offset retBCOff = srcInst->extra<DefInlineFP>()->retBCOff;
            retFixupMap[srcInst->dst()] = retBCOff;
            unit.replace(srcInst, PassFP, srcInst->src(2));
            if (stkInst->op() == SpillFrame) {
              unit.replace(stkInst, PassSP, stkInst->src(0));
            }
          }
        }
      }
      break;

    case PassFP:
      {
        auto frameInst = inst->src(0)->inst();
        if (frameInst->op() == DefInlineFP) {
          FTRACE(5, "Marking PassFP for {}\n", frameInst->id());
          state[frameInst].incWeakUse();
        }
      }
      break;

    case DefInlineFP:
      {
        auto outerInst = inst->src(2)->inst();
        if (outerInst->op() == DefInlineFP) {
          FTRACE(5, "Marking DefInlineFP for {}\n", outerInst->id());
          state[outerInst].incWeakUse();
        }
      }
      break;

    default:
      {
        for (uint32_t i = 0; i < inst->numSrcs(); i++) {
          auto src = inst->src(i);
          if (src->inst()->op() == DefInlineFP) {
            FTRACE(5, "not killing frame {} b/c: {}\n", src->inst()->id(),
                   inst->toString());
          }
        }
      }
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
    switch (inst->op()) {
    case DefInlineSP:
      {
        auto const fp = inst->src(0);
        if (fp->inst()->op() == PassFP) {
          FTRACE(5, "{} ({}) masking\n",
                 opcodeName(inst->op()),
                 inst->id());
          unit.replace(inst, PassSP, inst->src(1));
          break;
        }
      }
      break;

    case StLoc: case InlineReturn:
      {
        auto const fp = inst->src(0);
        if (fp->inst()->op() == PassFP) {
          FTRACE(5, "{} ({}) setDead\n",
                 opcodeName(inst->op()),
                 inst->id());
          state[inst].setDead();
        }
      }
      break;

      /*
       * When we unroll the stack during an exception the unwinder relies on
       * m_soff whenever it encounters an ActRec to properly restore the
       * PC once the frame has been destroyed.  When we elide a frame from the
       * stack we also update the PC pushed in any ActRec's pushed by a Call
       * so that they reflect the frame that they logically fall inside of.
       */
      case Call:
        {
          auto const arInst = inst->src(0)->inst();
          if (arInst->op() == SpillFrame) {
            auto const fp = arInst->src(1);
            if (fp->inst()->op() == PassFP) {
              auto fpInst = fp->inst();
              while (fpInst->src(0)->inst()->op() == PassFP) {
                fpInst = fpInst->src(0)->inst();
              }
              assert(retFixupMap.find(fpInst->dst()) != retFixupMap.end());
              FTRACE(5, "{} ({}) repairing\n",
                     opcodeName(inst->op()),
                     inst->id());
              Offset retBCOff = retFixupMap[fpInst->dst()];
              inst->setSrc(1, unit.cns(retBCOff));
            }
          }
        }
        break;

    case DefInlineFP:
      FTRACE(5, "DefInlineFP ({}): weak/strong uses: {}/{}\n",
             inst->id(),
             state[inst].weakUseCount(),
             uses[inst->dst()]);
      break;

    default:
      break;
    }
  });
}

// Assuming that the 'consumer' instruction consumes 'src', trace back through
// src's instruction to the real origin of the value. Currently this traces
// through CheckType, AssertType and DefLabel.
void consumeIncRef(const IRInstruction* consumer, const SSATmp* src,
                   DceState& state) {
  const IRInstruction* srcInst = src->inst();
  if ((srcInst->op() == CheckType || srcInst->op() == AssertType) &&
      srcInst->typeParam().maybeCounted()) {
    // srcInst is a CheckType/AsserType that guards to a refcounted type. We
    // need to trace through to its source. If the instruction guards to a
    // non-refcounted type then the reference is consumed by CheckType itself.
    consumeIncRef(consumer, srcInst->src(0), state);
    return;
  }

  if (srcInst->op() == DefLabel) {
    // srcInst is a DefLabel that may be a join node. We need to find
    // the dst index of src in srcInst and trace through to each jump
    // providing a value for it.
    for (unsigned i = 0, n = srcInst->numDsts(); i < n; ++i) {
      if (srcInst->dst(i) == src) {
        srcInst->block()->forEachSrc(i,
          [&](IRInstruction* jmp, SSATmp* val) {
            consumeIncRef(consumer, val, state);
          }
        );
        break;
      }
    }
    return;
  }

  if (srcInst->op() != IncRef) return;

  // <inst> consumes <srcInst> which is an IncRef, so we mark <srcInst> as
  // REFCOUNT_CONSUMED.
  if (consumer->trace()->isMain() || !srcInst->trace()->isMain()) {
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

} // anonymous namespace

// Publicly exported functions:

void eliminateDeadCode(IRTrace* trace, IRUnit& unit) {
  auto removeEmptyExitTraces = [&] {
    trace->exitTraces().remove_if([](IRTrace* exit) {
      return exit->blocks().empty();
    });
  };

  // kill unreachable code and remove any traces that are now empty
  BlockList blocks = removeUnreachable(trace, unit);
  removeEmptyExitTraces();

  // Ensure that main trace doesn't unconditionally jump to an exit
  // trace.  This invariant is needed by the ref-counting optimization.
  eliminateUnconditionalJump(trace);

  // mark the essential instructions and add them to the initial
  // work list; this will also mark reachable exit traces. All
  // other instructions marked dead.
  DceState state(unit, DceFlags());
  UseCounts uses(unit, 0);
  WorkList wl = initInstructions(blocks, state);

  // process the worklist
  while (!wl.empty()) {
    auto* inst = wl.front();
    wl.pop_front();
    for (uint32_t i = 0; i < inst->numSrcs(); i++) {
      SSATmp* src = inst->src(i);
      IRInstruction* srcInst = src->inst();
      if (srcInst->op() == DefConst) {
        continue;
      }

      if (srcInst->op() == DefInlineFP) {
        FTRACE(5, "adding use to frame {} b/c: {}\n", srcInst->id(),
               inst->toString());
      }

      uses[src]++;
      if (state[srcInst].isDead()) {
        state[srcInst].setLive();
        wl.push_back(srcInst);
      }

      // If inst consumes this source, find the true source instruction and
      // mark it as consumed if it's an IncRef.
      if (inst->consumesReference(i)) {
        consumeIncRef(inst, src, state);
      }
    }
  }

  // Optimize IncRefs and DecRefs.
  forEachTrace(trace, [&](IRTrace* t) {
    optimizeRefCount(t, unit.main(), state, uses);
  });

  if (RuntimeOption::EvalHHIREnableSinking) {
    // Sink IncRefs consumed off trace.
    sinkIncRefs(trace, unit, state);
  }

  // Optimize unused inlined activation records.  It's not necessary
  // to look at non-main traces for this.
  optimizeActRecs(trace, state, unit, uses);

  // now remove instructions whose id == DEAD
  removeDeadInstructions(trace, state);
  for (IRTrace* exit : trace->exitTraces()) {
    removeDeadInstructions(exit, state);
  }

  // and remove empty exit traces
  removeEmptyExitTraces();
}

} }
