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

#include <array>

#include "folly/MapUtil.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace JIT {
namespace {

TRACE_SET_MOD(hhir_dce);

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
    std::array<const char*,2> const names = {{
      "DEAD",
      "LIVE",
    }};
    return folly::format(
      "{}",
      m_state < names.size() ? names[m_state] : "<invalid>"
    ).str();
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

void removeDeadInstructions(IRUnit& unit, const DceState& state) {
  postorderWalk(unit, [&](Block* block) {
    block->remove_if([&] (const IRInstruction& inst) {
      ONTRACE(4,
              if (state[inst].isDead()) {
                FTRACE(1, "Removing dead instruction {}\n", inst.toString());
              });

      // For now, all control flow instructions are essential. If we ever
      // change this, we'll need to be careful about unlinking dead CF
      // instructions here.
      assert(IMPLIES(inst.isControlFlow(), !state[inst].isDead()));
      return state[inst].isDead();
    });
  });
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
           inst->src(0)->type() == Type::PtrToCell));
}

// removeUnreachable erases unreachable blocks from unit, and returns
// a sorted list of the remaining blocks.
BlockList prepareBlocks(IRUnit& unit) {
  FTRACE(1, "RemoveUnreachable:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "RemoveUnreachable:^^^^^^^^^^^^^^^^^^^^\n"); };

  BlockList blocks = rpoSortCfg(unit);
  bool needsResort = false;

  // 1. simplify unguarded loads to remove unnecssary branches, and
  //    perform copy propagation on every instruction. Targets that become
  //    unreachable from this pass will be eliminated in step 2 below.
  for (auto block : blocks) {
    for (auto& inst : *block) {
      copyProp(&inst);
    }
    auto inst = &block->back();
    // if this is a load that does not generate a guard, then get rid
    // of its label so that its not an essential control-flow
    // instruction
    if (isUnguardedLoad(inst) && inst->taken()) {
      // LdStack and LdLoc instructions that produce generic types
      // and LdStack instruction that produce Cell types will not
      // generate guards, so remove the label from this instruction so
      // that it's no longer an essential control-flow instruction
      ITRACE(2, "removing taken branch of unguarded load {}\n",
             *inst);
      inst->setTaken(nullptr);
      needsResort = true;
      if (inst->next()) {
        block->push_back(unit.gen(Jmp, inst->marker(), inst->next()));
        inst->setNext(nullptr);
      }
    }
  }

  // 2. erase unreachable blocks and get an rpo sorted list of what remains.
  bool needsReflow = removeUnreachable(unit);

  // 3. if we removed any whole blocks that ended in Jmp instructions, reflow
  //    all types in case they change the incoming types of DefLabel
  //    instructions.
  if (needsReflow) reflowTypes(unit);

  if (needsResort) blocks = rpoSortCfg(unit);
  return blocks;
}

WorkList initInstructions(const BlockList& blocks, DceState& state) {
  TRACE(1, "DCE(initInstructions):vvvvvvvvvvvvvvvvvvvv\n");
  // Mark reachable, essential, instructions live and enqueue them.
  WorkList wl;
  forEachInst(blocks, [&] (IRInstruction* inst) {
    if (inst->isEssential()) {
      state[inst].setLive();
      wl.push_back(inst);
    }
  });
  TRACE(1, "DCE:^^^^^^^^^^^^^^^^^^^^\n");
  return wl;
}

//////////////////////////////////////////////////////////////////////

/*
 * A use of an inlined frame that can be modified to work without the
 * frame is called a "weak use" here.  For example, storing to a local
 * on a frame is weak because if no other uses of the frame are
 * keeping it alive (for example a load of that same local), we can
 * just remove the store.
 *
 * This routine counts the weak uses of inlined frames and marks them
 * dead if they have no non-weak uses.  Returns true if any inlined
 * frames were marked dead.
 */
bool findWeakActRecUses(const BlockList& blocks,
                        DceState& state,
                        IRUnit& unit,
                        const UseCounts& uses) {
  bool killedFrames = false;

  auto const incWeak = [&] (const IRInstruction* inst, const SSATmp* src) {
    auto const frameInst = frameRoot(src->inst());
    if (frameInst->op() == DefInlineFP) {
      ITRACE(3, "weak use of {} from {}\n", *frameInst, *inst);
      state[frameInst].incWeakUse();
    }
  };

  /*
   * Maintain a list of the Calls depending on each DefInlineFP.
   * Calls can count as weak uses as long as there is only one right
   * now.  The limit to 1 is just because we have tested or
   * investigated the more-than-one case.
   */
  smart::flat_map<const IRInstruction*,uint32_t> callCounts;

  forEachInst(blocks, [&] (IRInstruction* inst) {
    if (state[inst].isDead()) return;

    switch (inst->op()) {
    // We don't need to generate stores to a frame if it can be eliminated.
    case StLocNT:
    case StLoc:
      incWeak(inst, inst->src(0));
      break;

    /*
     * You can use the stack inside an inlined callee without using
     * the frame, and we can adjust the initial ReDefSP that comes at
     * the start of the callee in this situation, but only if we're
     * not in a resumable.  In a resumable, there is no relation
     * between the main frame and the stack, so we can't modify this
     * ReDefSP to work on the outer frame.
     */
    case ReDefSP:
      {
        auto const fp = inst->src(1)->inst();
        if (fp->is(DefInlineFP) &&
            !fp->src(2)->inst()->marker().resumed()) {
          ITRACE(3, "weak use of {} from {}\n", fp->dst(), *inst);
          state[fp].incWeakUse();
        }
      }
      break;

    /*
     * Calls can count as weak frame uses with some restrictions:
     *
     *   o We must statically know what we're calling.
     *
     *   o The call itself is not protected by an EH region.
     *
     *   o We're not calling a native function.
     *
     *   o The callee must already have a translation for the prologue
     *     we need (has knownPrologue).
     *
     *   o There's only one Call instruction depending on this frame.
     *
     * The reason it is limited to a known prologue is that otherwise
     * the REQ_BIND_CALL service request could need to enter the
     * interpreter at the FCall instruction, which means it needs the
     * ActRec for the function containing that instruction to be on
     * the stack.  To know this also implies the first requirement.
     *
     * Similarly, we can't eliminate the outer frame if we may need it
     * to enter a catch block that is in the calling function.
     *
     * The other limits are just conservative while this was being
     * developed.
     *
     * Important: Right now all of this is disabled in
     * hhbc-translator, because the knownPrologue mechanism is buggy.
     * So we'll never have a knownPrologue here.  TODO(#4357498).
     */
    case Call:
      {
        auto const extra  = inst->extra<Call>();
        if (!extra->callee ||
            isNativeImplCall(extra->callee, extra->numParams) ||
            !inst->extra<Call>()->knownPrologue) {
          break;
        }
        if (inst->marker().func()->findEH(inst->marker().bcOff())) {
          FTRACE(2, "strong due to EH: {}\n", inst->toString());
          break;
        }
        auto const frameInst = frameRoot(inst->src(1)->inst());
        if (frameInst->is(DefInlineFP)) {
          // See above about the limit to 1.
          if (callCounts[inst->src(1)->inst()]++ < 1) {
            ITRACE(3, "weak use of {} from {}\n", *frameInst->dst(), *inst);
            state[frameInst].incWeakUse();
          }
        }
      }
      break;

    case InlineReturn:
      {
        auto const frameInst = frameRoot(inst->src(0)->inst());
        assert(frameInst->is(DefInlineFP));
        auto const frameUses = folly::get_default(uses, frameInst->dst(), 0);
        auto const weakUses  = state[frameInst].weakUseCount();
        /*
         * We can kill the frame if all uses of the frame are counted
         * as weak uses.  Note that this InlineReturn counts as a weak
         * use, but we haven't incremented for it yet, which is where
         * the "+ 1" comes from below.
         */
        ITRACE(2, "frame {}: weak/strong {}/{}\n",
          *frameInst, weakUses, frameUses);
        if (frameUses - (weakUses + 1) == 0) {
          ITRACE(1, "killing frame {}\n", *frameInst);
          killedFrames = true;
          state[inst].setDead();
          state[frameInst].setDead();
        }
      }
      break;

    default:
      // Default is conservative: we don't increment a weak use if it
      // uses the frame (or stack), so they can't be eliminated.
      break;
    }
  });

  return killedFrames;
}

/*
 * The first time through, we've counted up weak uses of the frame and
 * then finally marked it dead.  The instructions in between that were
 * weak uses may need modifications now that their frame is going
 * away.
 *
 * Also, if we eliminated some frames, DecRef instructions (which can
 * re-enter the VM without requiring a materialized frame) need to
 * have stack depths in their markers adjusted so they can't stomp on
 * parts of the outer function.  We handle this conservatively by just
 * pushing all DecRef markers where the DecRef is from a function
 * other than the outer function down to a safe re-entry depth.
 */
void performActRecFixups(const BlockList& blocks,
                         DceState& state,
                         IRUnit& unit,
                         const UseCounts& uses) {
  // We limit the total stack depth during inlining, so this is the deepest
  // we'll ever have to worry about.
  auto const outerFunc = blocks.front()->front().marker().func();
  auto const safeDepth = outerFunc->maxStackCells() + kStackCheckLeafPadding;
  ITRACE(3, "safeDepth: {}, outerFunc depth: {}\n",
         safeDepth,
         outerFunc->maxStackCells());

  for (auto block : blocks) {
    ITRACE(2, "Visiting block {}\n", block->id());
    Trace::Indent indenter;

    for (auto& inst : *block) {
      ITRACE(5, "{}\n", inst.toString());

      switch (inst.op()) {
      case DefInlineFP:
        ITRACE(3, "DefInlineFP ({}): weak/strong uses: {}/{}\n",
             inst, state[inst].weakUseCount(),
             folly::get_default(uses, inst.dst(), 0));
        break;

      /*
       * A ReDefSP that depends on a removable DefInlineFP needs to be
       * adjusted.  Its current frame is going away, so we have to
       * adjust it to depend on the outer frame, and have an offset
       * relative to that frame.
       *
       * In the common cases this ReDefSP is also going to be dce'd,
       * but we need to adjust it in case it isn't.
       *
       * The offset from the current frame (DefInlineFP) is a
       * parameter to the ReDefSP.  To turn that into an effective
       * offset from the outer frame we take the following: the
       * returnSpOffset that we recorded in this DefInlineFP, plus
       * whatever offset the ReDefSP had from the frame, plus the
       * cells for the ActRec, minus the space for the frame had for
       * locals or iterators.
       *
       * If this is the first ReDefSP in a callee, the spOffset here
       * will be numSlotsInFrame, so it cancels out and we just use
       * the return sp offset (plus the ActRec cells).  However, if
       * it's the ReDefSP that comes /after/ an InlineReturn, the fact
       * that it depends on a DefInlineFP (instead of a DefFP) means
       * we're in a nested inlining situation, and it is depending on
       * a stack defined inside the outer inlined callee.  In this
       * case, its offset will potentially not just be
       * numSlotsInFrame, because we may have more spills going on in
       * the outer callee.
       *
       * It's easiest to understand this by ignoring the above
       * particulars and thinking about what it should mean to adjust
       * a ReDefSP whose frame is being eliminated in isolation.
       */
      case ReDefSP:
        {
          auto const fp = inst.src(1)->inst();
          if (fp->is(DefInlineFP) && state[fp].isDead()) {
            inst.setSrc(1, fp->src(2));
            inst.extra<ReDefSP>()->spOffset +=
              fp->extra<DefInlineFP>()->retSPOff + kNumActRecCells -
              fp->extra<DefInlineFP>()->target->numSlotsInFrame();
          }
        }
        break;

      case Call:
        {
          auto const fp = inst.src(1)->inst();
          if (state[fp].isDead()) {
            assert(fp->is(DefInlineFP));
            inst.setSrc(1, fp->src(2));
            inst.extra<Call>()->after = fp->extra<DefInlineFP>()->retBCOff;
          }
        }
        break;

      case StLocNT:
      case StLoc:
        if (state[inst.src(0)->inst()].isDead()) {
          ITRACE(3, "marking {} as dead\n", inst);
          state[inst].setDead();
        }
        break;

      /*
       * DecRef* are special: they're the only instructions that can reenter
       * but not throw. This means it's safe to elide their inlined frame, as
       * long as we adjust their markers to a depth that is guaranteed to not
       * stomp on the caller's frame if it reenters.
       */
      case DecRef:
      case DecRefLoc:
      case DecRefStack:
      case DecRefMem:
        if (inst.marker().func() != outerFunc) {
          ITRACE(3, "pushing stack depth of {} to {}\n", safeDepth, inst);
          inst.marker().setSpOff(safeDepth);
        }
        break;

      default:
        break;
      }
    }
  }
}

/*
 * Look for InlineReturn instructions that are the only "non-weak" use
 * of a DefInlineFP.  In this case we can kill both, which may allow
 * removing a SpillFrame as well.
 *
 * Prior to calling this routine, `uses' should contain the direct
 * (non-transitive) use counts of each DefInlineFP instruction.  If
 * the weak references are equal to the normal references, the
 * instruction is not necessary and can be removed (if we make the
 * required changes to each instruction that used it weakly).
 */
void optimizeActRecs(const BlockList& blocks,
                     DceState& state,
                     IRUnit& unit,
                     const UseCounts& uses) {
  FTRACE(1, "AR:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "AR:^^^^^^^^^^^^^^^^^^^^^\n"); };
  if (do_assert) {
    for (UNUSED auto const& pair : uses) {
      assert(pair.first->isA(Type::FramePtr) &&
             pair.first->inst()->is(DefInlineFP));
    }
  }

  // Make a pass to find if we can kill any of the frames.  If so, we
  // have to do some fixups.  These two routines are coupled---most
  // cases in findWeakActRecUses should have a corresponding case in
  // performActRecFixups to deal with the frame being removed.
  auto const killedFrames = findWeakActRecUses(blocks, state, unit, uses);
  if (killedFrames) {
    ITRACE(1, "Killed some frames. Iterating over blocks for fixups.\n");
    performActRecFixups(blocks, state, unit, uses);
  }
}

//////////////////////////////////////////////////////////////////////

} // anonymous namespace

// Publicly exported functions:

void eliminateDeadCode(IRUnit& unit) {
  Timer dceTimer(Timer::optimize_dce);

  // kill unreachable code and remove any traces that are now empty
  auto const blocks = prepareBlocks(unit);

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

      if (RuntimeOption::EvalHHIRInlineFrameOpts) {
        if (src->isA(Type::FramePtr) && !srcInst->is(LdRaw, LdContActRec)) {
          if (srcInst->is(DefInlineFP)) {
            FTRACE(3, "adding use to {} from {}\n", *src, *inst);
            ++uses[src];
          }
        }
      }

      if (state[srcInst].isDead()) {
        state[srcInst].setLive();
        wl.push_back(srcInst);
      }
    }
  }

  if (RuntimeOption::EvalHHIRInlineFrameOpts) {
    optimizeActRecs(blocks, state, unit, uses);
  }

  // Now remove instructions whose state is DEAD.
  removeDeadInstructions(unit, state);
}

}}
