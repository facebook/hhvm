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

  bool isDead()                const { return m_state == DEAD; }
  void setDead()                  { m_state = DEAD; }
  void setLive()                  { m_state = LIVE; }

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
  uint8_t m_state:3;
  static constexpr uint8_t kMaxWeakUseCount = 15;
  uint8_t m_weakUseCount:4;
};
static_assert(sizeof(DceFlags) == 1, "sizeof(DceFlags) should be 1 byte");

// DCE state indexed by instr->id().
typedef StateVector<IRInstruction, DceFlags> DceState;
typedef StateVector<SSATmp, uint32_t> UseCounts;
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
  BlockList blocks = rpoSortCfg(unit);
  StateVector<Block, bool> reachable(unit, false);
  for (Block* b : blocks) reachable[b] = true;
  for (Block* b : blocks) {
    b->forEachPred([&](Block *p) {
      if (!reachable[p]) {
        // remove edges from unreachable block to reachable block.
        if (!p->empty()) p->back().setTaken(nullptr);
        p->setNext(nullptr);
      }
    });
  }
  forEachTrace(unit, [&](IRTrace* t) {
    for (auto bit = t->begin(); bit != t->end();) {
      if (reachable[*bit]) {
        ++bit;
        continue;
      }
      FTRACE(5, "erasing block {}\n", (*bit)->id());
      if ((*bit)->taken() && (*bit)->back().op() == Jmp) {
        needsReflow = true;
      }
      bit = t->erase(bit);
    }
  });

  // 3. if we removed any whole blocks that ended in Jmp
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
void optimizeActRecs(std::list<Block*>& blocks, DceState& state, IRUnit& unit,
                     UseCounts& uses) {
  FTRACE(5, "AR:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "AR:^^^^^^^^^^^^^^^^^^^^^\n"); };

  bool killedFrames = false;

  smart::map<SSATmp*, Offset> retFixupMap;
  forEachInst(blocks, [&](IRInstruction* inst) {
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
        if (frameInst->op() == DefInlineFP && reason->type() <= Type::Func) {
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
  forEachInst(blocks, [&](IRInstruction* inst) {
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

    case StLoc:
    case InlineReturn:
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

} // anonymous namespace

// Publicly exported functions:

void eliminateDeadCode(IRUnit& unit) {
  auto removeEmptyExitTraces = [&] {
    unit.exits().remove_if([](IRTrace* exit) {
      return exit->blocks().empty();
    });
  };

  // kill unreachable code and remove any traces that are now empty
  BlockList blocks = prepareBlocks(unit);
  removeEmptyExitTraces();

  // Ensure that main trace doesn't unconditionally jump to an exit
  // trace.  This invariant is needed by the ref-counting optimization.
  eliminateUnconditionalJump(unit);

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
    }
  }

  // Optimize unused inlined activation records.  It's not necessary
  // to look at non-main traces for this.
  optimizeActRecs(unit.main()->blocks(), state, unit, uses);

  // now remove instructions whose id == DEAD
  forEachTrace(unit, [&](IRTrace* t) {
    removeDeadInstructions(t, state);
  });

  // and remove empty exit traces
  removeEmptyExitTraces();
}

} }
