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
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP {
namespace JIT {
namespace {

TRACE_SET_MOD(hhir);

/* DceFlags tracks the state of one instruction during dead code analysis. */
struct DceFlags {
  DceFlags()
    : m_state(DEAD)
    , m_weakUseCount(0)
  {}

  bool isDead() const { return m_state == DEAD; }
  void setDead()      { m_state = DEAD; }
  void setLive()      { m_state = LIVE; }

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
    };
    return folly::format(
      "{}",
      m_state > array_size(names) ? "<invalid>" : names[m_state]).str();
  }

private:
  enum {
    DEAD = 0,
    LIVE,
  };
  uint8_t m_state:1;
  static constexpr uint8_t kMaxWeakUseCount = 0x7f;
  uint8_t m_weakUseCount:7;
};
static_assert(sizeof(DceFlags) == 1, "sizeof(DceFlags) should be 1 byte");

// DCE state indexed by instr->id().
typedef StateVector<IRInstruction, DceFlags> DceState;
typedef smart::hash_map<SSATmp*, uint32_t> UseCounts;
typedef smart::list<const IRInstruction*> WorkList;

void removeDeadInstructions(IRTrace* trace, const DceState& state) {
  auto& blocks = trace->blocks();
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
    if (block->empty()) trace->unlink(cur);
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

// removeUnreachable erases unreachable blocks from unit, and returns
// a sorted list of the remaining blocks.
BlockList prepareBlocks(IRUnit& unit) {
  FTRACE(5, "RemoveUnreachable:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "RemoveUnreachable:^^^^^^^^^^^^^^^^^^^^\n"); };

  // 1. simplify unguarded loads to remove unnecssary branches, and
  //    perform copy propagation on every instruction. Targets that become
  //    unreachable from this pass will be eliminated in step 2 below.
  forEachTraceInst(unit, [](IRInstruction* inst) {
    copyProp(inst);
    // if this is a load that does not generate a guard, then get rid
    // of its label so that its not an essential control-flow
    // instruction
    if (isUnguardedLoad(inst) && inst->taken()) {
      // LdStack and LdLoc instructions that produce generic types
      // and LdStack instruction that produce Cell types will not
      // generate guards, so remove the label from this instruction so
      // that it's no longer an essential control-flow instruction
      ITRACE(4, "removing taken branch of unguarded load {}\n",
             *inst);
      inst->setTaken(nullptr);
    }
  });

  // 2. erase unreachable blocks and get an rpo sorted list of what remains.
  bool needsReflow = removeUnreachable(unit);

  // 3. if we removed any whole blocks that ended in Jmp instructions, reflow
  //    all types in case they change the incoming types of DefLabel
  //    instructions.
  if (needsReflow) reflowTypes(unit);

  return rpoSortCfg(unit);
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
    }
  }
  TRACE(5, "DCE:^^^^^^^^^^^^^^^^^^^^\n");
  return wl;
}

/*
 * Look for InlineReturn instructions that are the only "non-weak" use
 * of a DefInlineFP.  In this case we can kill both, which may allow
 * removing a SpillFrame as well.
 */
void optimizeActRecs(BlockList& blocks, DceState& state, IRUnit& unit,
                     const UseCounts& uses) {
  FTRACE(3, "AR:vvvvvvvvvvvvvvvvvvvvv\n");
  if (do_assert) {
    for (UNUSED auto const& pair : uses) {
      assert(pair.first->isA(Type::FramePtr));
      assert(pair.first->inst()->is(DefInlineFP));
    }
  }

  SCOPE_EXIT { FTRACE(3, "AR:^^^^^^^^^^^^^^^^^^^^^\n"); };
  using Trace::Indent;
  Indent _i;

  bool killedFrames = false;

  smart::hash_map<SSATmp*, Offset> retFixupMap;
  forEachInst(blocks, [&](IRInstruction* inst) {
    if (state[inst].isDead()) return;

    switch (inst->op()) {
      // We don't need to generate stores to a frame if it can be eliminated.
      case StLoc: {
        auto const frameInst = frameRoot(inst->src(0)->inst());
        if (frameInst->op() == DefInlineFP) {
          ITRACE(4, "{} weak use of {}\n", *inst, *frameInst->dst());
          state[frameInst].incWeakUse();
        }
        break;
      }

      case PassFP: {
        auto frameInst = frameRoot(inst->src(0)->inst());
        if (frameInst->op() == DefInlineFP) {
          ITRACE(4, "{} weak use of {}\n", *inst, *frameInst->dst());
          state[frameInst].incWeakUse();
        }
        break;
      }

      /* DefInlineFP counts as two weak uses of its containing frame, one for
       * the DefInlineFP itself and one for the SpillFrame used here. We wait
       * until this instruction to mark the SpillFrame's use as weak so a
       * normal Call will prevent frames from being elided. */
      case DefInlineFP: {
        auto outerFrame = frameRoot(inst->src(2)->inst());
        if (outerFrame->is(DefInlineFP)) {
          ITRACE(4, "{} weak use of {}\n", *inst, *outerFrame->dst());
          state[outerFrame].incWeakUse();

          auto DEBUG_ONLY spillFrame = findSpillFrame(inst->src(0));
          assert(spillFrame->src(1) == outerFrame->dst());
          ITRACE(4, "{} weak use of {}\n", *spillFrame, *outerFrame->dst());
          state[outerFrame].incWeakUse();
        }
        break;
      }

      case ReDefSP:
      case DefInlineSP: {
        auto const frameInst = frameRoot(inst->src(1)->inst());
        if (frameInst->op() == DefInlineFP) {
          ITRACE(4, "{} weak use of {}\n", *inst, *frameInst->dst());
          state[frameInst].incWeakUse();
        }
        break;
      }

      case InlineReturn: {
        auto* frameInst = frameRoot(inst->src(0)->inst());
        assert(frameInst->is(DefInlineFP));
        auto frameUses = mapGet(uses, frameInst->dst(), 0);
        auto* spillInst = findSpillFrame(frameInst->src(0));

        auto weakUses = state[frameInst].weakUseCount();
        // We haven't counted this InlineReturn as a weak use yet,
        // which is where this '1' comes from.
        if (frameUses - weakUses == 1) {
          ITRACE(4, "killing frame {}\n", *frameInst);
          killedFrames = true;

          Offset retBCOff = frameInst->extra<DefInlineFP>()->retBCOff;
          retFixupMap[frameInst->dst()] = retBCOff;
          ITRACE(4, "replacing {} and {} with PassFP/PassSP, removing {}\n",
                 *frameInst, *spillInst, *inst);
          unit.replace(frameInst, PassFP, frameInst->src(2));
          unit.replace(spillInst, PassSP, spillInst->src(0));
          inst->convertToNop();
        } else {
          ITRACE(3, "not killing frame {}, weak/strong {}/{}\n",
                 *frameInst, weakUses, frameUses);
        }
        break;
      }

      default: {
        break;
      }
    }
  });

  if (!killedFrames) return;

  /*
   * The first time through, we've counted up weak uses of the frame and then
   * finally marked it dead.  The instructions in between that were weak uses
   * may need modifications now that their frame is going away.
   *
   * frameDepths is used to keep track of the current maximum distance the
   * stack could be from the enclosing frame pointer. This is used to update
   * the BCMarkers of instructions that may be sensitive to having their
   * enclosing frame elided.
   */
  smart::hash_map<Block*, uint32_t> frameDepths;
  auto depth = [](const Func* func) {
    return func->numSlotsInFrame() + func->maxStackCells();
  };
  frameDepths[blocks.front()] = 0;

  // We limit the total stack depth during inlining, so this is the deepest
  // we'll ever have to worry about.
  auto* outerFunc = blocks.front()->front().marker().func;
  auto const maxDepth = depth(outerFunc) + kStackCheckLeafPadding;

  ITRACE(3, "Killed some frames. Iterating over blocks for fixups.\n");
  for (auto* block : blocks) {
    ITRACE(5, "Visiting block {}\n", block->id());
    Indent _i;
    assert(frameDepths.count(block));
    auto curDepth = frameDepths[block];
    frameDepths.erase(block);
    ITRACE(5, "loaded depth {}\n", curDepth);

    for (auto& inst : *block) {
      switch (inst.op()) {
        case DefInlineFP: {
          auto* spillInst = findSpillFrame(inst.src(0));
          assert(spillInst);
          curDepth += depth(spillInst->marker().func);
          ITRACE(4, "DefInlineFP ({}): weak/strong uses: {}/{}\n",
                 inst, state[inst].weakUseCount(), mapGet(uses, inst.dst(), 0));
          break;
        }

        case InlineReturn: {
          auto* fpInst = frameRoot(inst.src(0)->inst());
          assert(fpInst->is(DefInlineFP));
          auto* spillInst = findSpillFrame(fpInst->src(0));
          assert(spillInst);
          curDepth -= depth(spillInst->marker().func);
          assert(findPassFP(inst.src(0)->inst()) == nullptr &&
                 "Eliminated DefInlineFP but left its InlineReturn");
          break;
        }

        case DefInlineSP: {
          if (auto const fp = findPassFP(inst.src(1)->inst())) {
            ITRACE(4, "replacing {} with PassSP\n", inst);
            unit.replace(&inst, PassSP, inst.src(0));
            break;
          }
          break;
        }

        case StLoc: {
          if (auto const fp = findPassFP(inst.src(0)->inst())) {
            ITRACE(4, "marking {} as dead\n", inst);
            state[inst].setDead();
          }
          break;
        }

        /*
         * DecRef* are special: they're the only instructions that can reenter
         * but not throw. This means it's safe to elide their inlined frame, as
         * long as we adjust their markers to a depth that is guaranteed to not
         * stomp on the caller's frame if it reenters.
         */
        case DecRef:
        case DecRefLoc:
        case DecRefStack:
        case DecRefMem: {
          auto& spOff = inst.marker().spOff;
          auto newDepth = maxDepth - curDepth;
          assert(spOff <= newDepth);
          ITRACE(4, "adjusting marker spOff for {} from {} to {}\n",
                 inst, spOff, newDepth);
          spOff = newDepth;
          break;
        }

        case ReDefSP: {
          // The first real frame enclosing this ReDefSP may have changed, so
          // update its frameOffset.
          auto* fpInst = frameRoot(inst.src(1)->inst());
          auto* realFp = fpInst->dst();
          int32_t offset = 0;
          auto const& extra = *inst.extra<ReDefSP>();
          ITRACE(4, "calculating new offset for {}\n", inst);
          Indent _i;

          for (unsigned i = 0; i < extra.nFrames; ++i) {
            auto& frame = extra.frames[i];
            ITRACE(4, "adding {} for {}\n", frame.spOff, *frame.fp);
            offset += frame.spOff;
            if (frame.fp == realFp) break;
            assert(i < extra.nFrames - 1);
          }
          ITRACE(4, "final offset: {}\n", offset);
          inst.extra<ReDefSP>()->spOffset = offset;
          break;
        }

        /*
         * When we unroll the stack during an exception the unwinder relies on
         * m_soff whenever it encounters an ActRec to properly restore the PC
         * once the frame has been destroyed.  When we elide a frame from the
         * stack we also update the PC pushed in any ActRec's pushed by a Call
         * so that they reflect the frame that they logically fall inside of.
         */
        case Call: {
          auto const arInst = findSpillFrame(inst.src(0));
          if (arInst && arInst->is(SpillFrame)) {
            if (auto fpInst = findPassFP(arInst->src(1)->inst())) {
              not_reached(); // TODO t3203284
              assert(retFixupMap.count(fpInst->dst()));
              ITRACE(4, "{} repairing\n", inst);
              Offset retBCOff = retFixupMap[fpInst->dst()];
              inst.setSrc(1, unit.cns(retBCOff));
            }
          }
          break;
        }

        default: {
          break;
        }
      }
    }

    ITRACE(5, "finishing block B{} with depth {}\n", block->id(), curDepth);
    if (auto* taken = block->taken()) {
      if (!frameDepths.count(taken)) {
        frameDepths[taken] = curDepth;
      } else {
        assert(frameDepths[taken] == curDepth);
      }
    }
    if (auto* next = block->next()) {
      if (!frameDepths.count(next)) {
        frameDepths[next] = curDepth;
      } else {
        assert(frameDepths[next] == curDepth);
      }
    }
  }
}

} // anonymous namespace

// Publicly exported functions:

void eliminateDeadCode(IRUnit& unit) {
  auto removeEmptyExitTraces = [&] {
    auto& exits = unit.exits();
    auto isEmpty = [](IRTrace* exit) {
      return exit->blocks().empty();
    };
    exits.erase(std::remove_if(exits.begin(), exits.end(), isEmpty),
                exits.end());
  };

  // kill unreachable code and remove any traces that are now empty
  BlockList blocks = prepareBlocks(unit);
  removeEmptyExitTraces();

  // mark the essential instructions and add them to the initial
  // work list; this will also mark reachable exit traces. All
  // other instructions marked dead.
  DceState state(unit, DceFlags());
  UseCounts uses;
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

      if (src->isA(Type::FramePtr) && !srcInst->is(LdRaw, LdContActRec)) {
        auto* root = frameRoot(srcInst);
        if (root->is(DefInlineFP)) {
          FTRACE(4, "adding use to {} from {}\n", *src, *inst);
          ++uses[root->dst()];
        }
      }

      if (state[srcInst].isDead()) {
        state[srcInst].setLive();
        wl.push_back(srcInst);
      }
    }
  }

  if (RuntimeOption::EvalHHIRInlineFrameOpts) {
    // Optimize unused inlined activation records.
    optimizeActRecs(blocks, state, unit, uses);
  }

  // now remove instructions whose id == DEAD
  forEachTrace(unit, [&](IRTrace* t) {
    removeDeadInstructions(t, state);
  });

  // and remove empty exit traces
  removeEmptyExitTraces();
}

} }
