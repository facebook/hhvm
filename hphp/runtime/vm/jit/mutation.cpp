/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/mutation.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/timer.h"

#include "hphp/util/dataflow-worklist.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

bool retypeDst(IRInstruction* inst, int num) {
  auto const ssa = inst->dst(num);
  auto const oldType = ssa->type();

  /*
   * The type of a tmp defined by DefLabel is the union of the types of the
   * tmps at each incoming Jmp.
   */
  if (inst->op() == DefLabel) {
    auto type = TBottom;
    inst->block()->forEachSrc(num, [&] (IRInstruction*, SSATmp* tmp) {
      type = type | tmp->type();
    });
    ssa->setType(type);
    return type != oldType;
  }

  auto const newType = outputType(inst, num);
  ssa->setType(newType);

  return newType != oldType;
}

//////////////////////////////////////////////////////////////////////

struct RefineTmps {
  TRACE_SET_MOD(hhir_refineTmps);

  explicit RefineTmps(IRUnit& unit)
    : unit{unit}
    , rpoBlocks{rpoSortCfg(unit)}
    , blockState{unit, State{}} {}

  /*
   * Overview of algorithm:
   *
   * For each block we maintain a map of "canonical" SSATmps to what
   * they should be rewritten to.
   *
   * A SSATmp is canonical if it's produced by a non-passthrough
   * instruction. SSATmps are canonicalized by chasing passthrough
   * instructions which define them, until we hit a
   * non-passthrough. DefLabels have special handling. If all sources
   * of the DefLabel canonicalize to the same SSATmp, we treat it as a
   * passthrough. If not, they're considered "normal" defining
   * instructions.
   *
   * A SSATmp can be rewritten to another SSATmp, or to a phi. For
   * each block, union together the rewrite maps in each
   * predecessor. If two predecessors disagree, then the rewrite
   * becomes a phi at the current block. A SSATmp not present in the
   * map implicitly means it rewrites to itself.
   *
   * Once the rewrites for the block start have been set up, we walk
   * through each instruction in the block. If the instruction uses a
   * (canonicalized) SSATmp in the rewrite map, and the rewrite target
   * is a SSATmp, rewrite the use to the target SSATmp. If the rewrite
   * target is a phi (whether this block or another), record the usage
   * in a side map, but don't modify the use.
   *
   * If the instruction is a passthrough, store the instruction's
   * (canonicalized) source in the rewrite map, with the dest of the
   * instruction as the rewrite target. Again, DefLabel's are treated
   * as passthrough or not depending on how its sources canonicalize.
   *
   * Store the rewrite map (updated from processing the instructions)
   * in the block's out state (next and/or taken). Do the block
   * processing in the typical dataflow manner, scheduling
   * predecessors and rescheduling a block every time one of its
   * successor's changes state. This is guaranteed to terminate
   * because the rewrite map (for every canonical SSATmp) can either
   * stay the same, or turn into a phi. Once it becomes a phi, it
   * stays that way.
   *
   * Once the dataflow is done, we have the rewrite maps for every
   * block, and all SSATmp uses (non-phi) have been rewritten. We then
   * materialize all *used* phis. A phi might have been created in the
   * rewrite maps, but never used. During dataflow we recorded which
   * phis were actually used.
   *
   * Phis are materialized by creating a DefLabel at their associated
   * block. The inputs for the phi are done by looking at the rewrite
   * maps in each predecessor. If the rewrite is to a SSATmp, or to an
   * already materialized phi, we simply use the SSATmp. If not, we
   * materialize the non-materialized phi from the pred
   * recursively. This requires a bit of trickery because DefLabels
   * might be mutually recursive and we can end up in an infinite
   * loop. We store "dummy" SSATmps in the DefLabel, then recursively
   * materialize the dependent phis, then fixup the dummy SSATmps.
   *
   * Once all (used) phis are materialized, we have SSATmps for all of
   * them. Iterate over the instructions which use those phis
   * (gathered during dataflow), and rewrite the remaining uses.
   */
  bool operator()() {
    ITRACE(1, "refine-tmps:vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
    SCOPE_EXIT {
      ITRACE(1, "refine-tmps:^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    };
    Trace::Indent indenter;

    // First calculate the dataflow to see which rewrites are present
    // at each block. Non-phi SSATmps are rewritten as we go. If no
    // phis are created, this is sufficient.
    auto const changed = dataflow();
    if (usedPhis.empty()) {
      ITRACE(2, "no phis to insert\n");
      return changed;
    }

    // Otherwise we have phis we need to create. Sort them according
    // to RPO order. This lets us tend to create phis before the
    // usages of the phis, which makes for less fixups needed.
    jit::vector<Phi> toMaterialize;
    for (auto const& phi : usedPhis) {
      assertx(!phi.second);
      toMaterialize.emplace_back(phi.first);
    }
    std::sort(
      toMaterialize.begin(), toMaterialize.end(),
      [&] (auto const& p1, auto const& p2) {
        auto const order1 = blockState[p1.block].order;
        auto const order2 = blockState[p2.block].order;
        if (order1 != order2) return order1 < order2;
        return p1.canon->id() < p2.canon->id();
      }
    );

    ONTRACE(
      2,
      ITRACE(2, "To Materialize:\n");
      for (auto const& phi : toMaterialize) {
        ITRACE(2, "  B{} for {}\n", phi.block->id(), *phi.canon);
      }
    );

    // Materialize phis:
    for (auto const& phi : toMaterialize) materializePhi(phi);

    // At this point everything is created. We only need to rewrite
    // any instructions which use one of the materialized phis. We
    // kept an explicit list of these, so we don't need to walk the
    // entire unit.
    for (auto const& use : phiUses) {
      auto inst = use.first.inst;
      auto const idx = use.first.srcIdx;
      auto const block = use.second.block;
      auto const canon = use.second.canon;
      assertx(idx < inst->numSrcs());
      auto const it = usedPhis.find(Phi{block, canon});
      assertx(it != usedPhis.end());
      assertx(it->second);
      ITRACE(2, "Rewriting {} -> {} in {}\n",
             *inst->src(idx), *it->second, inst->toString());
      inst->setSrc(idx, it->second);
      retypeDests(inst, &unit);
    }

    return true;
  }

  // Use dataflow to calculate the appropriate rewrites for each
  // block. If any phis need to be created, record them in
  // `usedPhis'. Record any instructions which will use a phi SSATmp,
  // record it in `phiUses'.
  bool dataflow() {
    dataflow_worklist<size_t> worklist{rpoBlocks.size()};
    for (uint32_t i = 0; i < rpoBlocks.size(); ++i) {
      worklist.push(i);
      blockState[rpoBlocks[i]].order = i;
    }

    auto changed = false;

    do {
      auto const block = rpoBlocks[worklist.pop()];
      auto& state = blockState[block];

      ITRACE(2, "Processing B{}:\n", block->id());
      Trace::Indent indenter;

      // Build up the in state from predecessors:
      auto remaps = [&] {
        if (block == unit.entry()) return Remaps{};

        Optional<Remaps> ret;
        block->forEachPred(
          [&] (Block* pred) {
            assertx(block == pred->next() || block == pred->taken());
            auto const& predState = block == pred->next()
              ? blockState[pred].outNext
              : blockState[pred].outTaken;
            // If the predecessor wasn't visited yet, ignore it. If we
            // visit it later, we'll reschedule this block.
            if (!predState) return;
            if (!ret) {
              ret = *predState;
              return;
            }

            // Merge together the remaps between the current state and
            // this pred. If there's any mismatch, we produce a phi at
            // this block.
            for (auto& remap : *ret) {
              assertx(RemapTo{remap.first} != remap.second);
              auto const o = [&] {
                auto const it = predState->find(remap.first);
                if (it == predState->end()) return RemapTo{remap.first};
                return it->second;
              }();
              remap.second.merge(o, block);
            }

            for (auto const& remap : *predState) {
              assertx(RemapTo{remap.first} != remap.second);
              auto const it = ret->find(remap.first);
              if (it != ret->end()) continue;
              ret->emplace(remap.first, RemapTo{block});
            }
          }
        );

        assertx(ret.has_value());
        return *ret;
      }();

      ONTRACE(
        4,
        ITRACE(4, "Remaps (initial):\n");
        for (auto const& remap : remaps) {
          ITRACE(4, "  {} -> {}\n", *remap.first, remap.second.toString());
        }
      );

      // Now visit each instruction in the block, rewriting sources,
      // and updating rewrites if we see a passthrough instruction.
      for (auto& inst : *block) {
        // If this instruction has a taken edge (which means it's a
        // block end), we need to push the state before processing
        // this instruction.
        if (auto const taken = inst.taken()) {
          if (state.outTaken != remaps) {
            ITRACE(4, "Scheduling taken block B{}\n", taken->id());
            worklist.push(blockState[taken].order);
            if (!inst.next() && inst.numSrcs() == 0) {
              // Optimization. If this instruction doesn't have a next
              // edge (so just taken), and has no sources, we can move
              // the state and not process the instruction.
              state.outTaken = std::move(remaps);
              continue;
            } else {
              // Otherwise we need to copy it, since we might need to
              // use it.
              state.outTaken = remaps;
            }
          } else if (!inst.next() && inst.numSrcs() == 0) {
            continue;
          }
        }

        // Perform rewrites. If we have none, we don't have to visit
        // the sources.
        if (!remaps.empty()) {
          auto retype = false;
          for (uint32_t i = 0; i < inst.numSrcs(); ++i) {
            auto const from = canonicalize(inst.src(i));
            auto const it = remaps.find(from);
            if (it == remaps.end()) continue;
            if (auto const to = it->second.ssatmp()) {
              // Rewrites to a SSATmp. Change the source.
              assertx(from != to);
              if (inst.src(i) != to) {
                ITRACE(2, "Rewriting {} -> {} in {}\n",
                       *inst.src(i), *to, inst.toString());
                inst.setSrc(i, to);
                retype = true;
              }
            } else {
              // Rewrites to a phi. Record the phi as used and record
              // the instruction so it can be rewritten later.
              auto const phi = it->second.block();
              assertx(phi);
              usedPhis.emplace(Phi{phi, from}, nullptr);
              phiUses.emplace(PhiUse{&inst, i}, Phi{phi, from});
              ITRACE(2, "Would rewrite {} to phi B{} in {}\n",
                     *inst.src(i), phi->id(), inst.toString());
            }
          }

          // If we changed a source, we have to retype the dest.
          if (retype) {
            retypeDests(&inst, &unit);
            changed = true;
          }
        }

        if (inst.isPassthrough()) {
          // Record a rewrite from the canonical source of this
          // passthrough to its dest.
          auto const from = canonicalize(inst.src(0));
          // Ignore constants. They're perfect as they are.
          if (!from->inst()->is(DefConst)) {
            auto const to = inst.dst();
            ITRACE(
              2, "{} now maps to {} (via {})\n",
              *from, *to, inst.toString()
            );
            remaps.insert_or_assign(from, RemapTo{to});
          }
        } else if (inst.is(DefLabel)) {
          // Check if the DefLabel's sources all canonicalize the
          // same. If it does, treat this like a passthrough. If not,
          // treat it as a non-passthrough.
          for (uint32_t i = 0; i < inst.numDsts(); ++i) {
            auto const to = inst.dst(i);
            auto const from = canonicalize(to);
            if (from == to) {
              remaps.erase(to);
            } else if (!from->inst()->is(DefConst)) {
              ITRACE(
                2, "{} now maps to {} (via DefLabel {})\n",
                *from, *to, inst.toString()
              );
              remaps.insert_or_assign(from, RemapTo{to});
            }
          }
        } else {
          // Normal instruction. If this instruction produces a SSATmp
          // we have a rewrite to, erase that rewrite. This might seem
          // odd, but it can happen in a loop.
          for (uint32_t i = 0; i < inst.numDsts(); ++i) {
            remaps.erase(inst.dst(i));
          }
        }

        if (auto const next = inst.next()) {
          if (state.outNext != remaps) {
            ITRACE(4, "Scheduling next block B{}\n", next->id());
            worklist.push(blockState[next].order);
            state.outNext = std::move(remaps);
          }
        }
      }

      ONTRACE(
        4,
        ITRACE(4, "Remaps (next):\n");
        if (state.outNext) {
          for (auto const& remap : *state.outNext) {
            ITRACE(4, "  {} -> {}\n", *remap.first, remap.second.toString());
          }
        }
      );
      ONTRACE(
        4,
        ITRACE(4, "Remaps (taken):\n");
        if (state.outTaken) {
          for (auto const& remap : *state.outTaken) {
            ITRACE(4, "  {} -> {}\n", *remap.first, remap.second.toString());
          }
        }
      );
    } while (!worklist.empty());

    return changed;
  }

  struct Phi {
    Block* block;
    SSATmp* canon;
    bool operator==(const Phi& o) const {
      return block == o.block && canon == o.canon;
    }
  };

  // Create a DefLabel at the given block for the given canonical
  // SSATmp. This includes recursively materializing any phis this phi
  // might depend on.
  SSATmp* materializePhi(const Phi& phiToMaterialize) {
    {
      auto const it = usedPhis.find(phiToMaterialize);
      assertx(it != usedPhis.end());
      if (it->second) return it->second;
    }

    auto const block = phiToMaterialize.block;
    auto const canon = phiToMaterialize.canon;

    ITRACE(2, "Materializing phi at B{} for {}\n", block->id(), *canon);
    Trace::Indent indenter;

    jit::hash_map<Block*, SSATmp*> inputs;
    jit::fast_map<Block*, Block*> fixups;

    // Visit each pred to determine what the input for the DefLabel
    // should be
    block->forEachPred(
      [&] (Block* pred) {
        assertx(block == pred->next() || block == pred->taken());
        auto const& predState = block == pred->next()
          ? blockState[pred].outNext
          : blockState[pred].outTaken;
        assertx(predState);

        auto const it = predState->find(canon);
        if (it == predState->end()) {
          ITRACE(4, "Pred B{} has no mapping for {}\n", pred->id(), *canon);
          inputs.emplace(pred, canon);
        } else if (auto const tmp = it->second.ssatmp()) {
          assertx(tmp != canon);
          ITRACE(4, "Pred B{} maps {} to {}\n", pred->id(), *canon, *tmp);
          inputs.emplace(pred, tmp);
        } else if (auto const predPhi = it->second.block()) {
          // The input is another DefLabel. If the phi has a SSATmp,
          // then it's already resolved and we can treat it as
          // normal. If not, we don't have a SSATmp yet for that
          // phi. We use a Bottom constant as a placeholder
          // SSATmp. Afterwards we'll resolve the phi and then fixup
          // the input.
          auto const phiIt =
            usedPhis.emplace(Phi{predPhi, canon}, nullptr).first;
          if (phiIt->second) {
            ITRACE(
              4, "Pred B{} maps {} to resolved phi {}\n",
              pred->id(), *canon, *phiIt->second
            );
            inputs.emplace(pred, phiIt->second);
          } else {
            ITRACE(
              4, "Pred B{} maps {} to unresolved phi at B{}\n",
              pred->id(), *canon, predPhi->id()
            );
            inputs.emplace(pred, unit.cns(TBottom));
            fixups.emplace(pred, predPhi);
          }
        } else {
          always_assert(false);
        }
      }
    );

    // Create the DefLabel
    auto const phi = insertPhi(unit, block, inputs, !fixups.empty());
    ITRACE(
      4, "Phi at B{} for {} materialized as {}\n",
      block->id(), *canon, *phi
    );

    // Store the SSATmp we created for the DefLabel. Note: we do this
    // before resolving the placeholders below. This means if the
    // DefLabel has mutual recursion, we won't hit an infinite
    // loop. We'll just return this SSATmp immediately.
    auto const it = usedPhis.find(Phi{block, canon});
    assertx(it != usedPhis.end());
    assertx(!it->second);
    it->second = phi;

    if (!fixups.empty()) {
      // If we made use of any Bottom placeholders above, materialize
      // the phi for it, and then update the placeholder.
      ITRACE(
        4, "Resolving fixups for phi at B{} for {}\n",
        block->id(), *canon
      );
      assertx(phi->inst()->findDst(phi) == phi->inst()->numDsts() - 1);
      block->forEachPred(
        [&] (Block* pred) {
          auto& jmp = pred->back();
          assertx(jmp.is(Jmp));
          assertx(jmp.numSrcs() == phi->inst()->numDsts());
          auto const fixupIt = fixups.find(pred);
          if (fixupIt == fixups.end()) return;
          assertx(jmp.src(phi->inst()->numDsts() - 1)->isA(TBottom));
          auto const fixup = materializePhi(Phi{fixupIt->second, canon});
          ITRACE(4, "Fixup for phi B{} for {} resolves to {}\n",
                 fixupIt->second->id(), *canon, *fixup);
          jmp.setSrc(phi->inst()->numDsts() - 1, fixup);
        }
      );
    }

    // Set up the output type for the DefLabel
    retypeDests(phi->inst(), &unit);
    return phi;
  }

  using VisitedSet = jit::fast_set<std::pair<Block*, uint32_t>>;

  // "Canonicalize" a SSATmp by walking backwards through passthrough
  // instructions, stopping at the SSATmp defined by a
  // non-passthrough.
  //
  // We don't use canonical for this, because it includes
  // ConvPtrToLval (which isn't a passthrough for our purposes), and
  // we want special handling for DefLabel.
  static SSATmp* canonicalize(SSATmp* v, VisitedSet* visited = nullptr) {
    while (v->inst()->isPassthrough()) v = v->inst()->getPassthroughValue();
    auto const inst = v->inst();

    // Not a passthrough and not a DefLabel. This is canonical.
    if (!inst->is(DefLabel)) return v;
    // We deal with DefLabels specially. A DefLabel is considered
    // "passthrough" if all of its associated sources canonicalize
    // to the same SSATmp. If not, it's considered a normal
    // defining instruction.

    auto const dstIdx = inst->findDst(v);
    assertx(dstIdx != inst->numDsts());

    // We need to canonicalize each of the DefLabel's sources
    // recursively. Since this can visit more DefLabels (and
    // DefLabels can be mutually recursive), we need to keep a
    // visited set.
    Optional<VisitedSet> optVisited;
    if (visited) {
      // If we visited this DefLabel already, return nullptr. This
      // is a special value (only allowed for DefLabels) which
      // means "ignore this branch".
      if (!visited->emplace(inst->block(), dstIdx).second) return nullptr;
    } else {
      optVisited.emplace();
      visited = optVisited.get_pointer();
    }

    // Visit each source and see if they all canonicalize the same
    Optional<SSATmp*> commonCanon;
    inst->block()->forEachSrc(
      dstIdx,
      [&] (IRInstruction*, SSATmp* src) {
        if (commonCanon && !*commonCanon) return;
        auto const canon = canonicalize(src, visited);
        // If nullptr, ignore this source
        if (!canon) return;

        if (!commonCanon) {
          commonCanon = canon;
        } else if (*commonCanon != canon) {
          commonCanon = nullptr;
        }
      }
    );
    if (!commonCanon) {
      return optVisited.has_value() ? v : nullptr;
    }
    if (*commonCanon) return *commonCanon;
    return v;
  }

  struct RemapTo {
    explicit RemapTo(SSATmp* t) : to{t} {}
    explicit RemapTo(Block* b) : to{b} {}

    SSATmp* ssatmp() const { return to.left(); }
    Block* block() const { return to.right(); }

    void merge(const RemapTo& o, Block* current) {
      assertx(!to.isNull());
      assertx(!o.to.isNull());
      if (to != o.to) to = current;
    }

    std::string toString() const {
      return to.match(
        [] (SSATmp* s) { return s->toString(); },
        [] (Block* b) { return folly::sformat("Phi B{}", b->id()); }
      );
    }

    bool operator==(const RemapTo& o) const { return to == o.to; }
    bool operator!=(const RemapTo& o) const { return to != o.to; }

    Either<SSATmp*, Block*> to;
  };

  IRUnit& unit;
  BlockList rpoBlocks;

  using Remaps = jit::fast_map<SSATmp*, RemapTo>;
  struct State {
    size_t order;
    Optional<Remaps> outNext;
    Optional<Remaps> outTaken;
  };
  StateVector<Block, State> blockState{unit, State{}};

  struct PhiUse {
    IRInstruction* inst;
    uint32_t srcIdx;
    bool operator==(const PhiUse& o) const {
      return inst == o.inst && srcIdx == o.srcIdx;
    }
  };

  struct Hasher {
    size_t operator()(const Phi& p) const {
      return folly::hash::hash_combine(
        pointer_hash<Block>{}(p.block),
        pointer_hash<SSATmp>{}(p.canon)
      );
    }
    size_t operator()(const PhiUse& p) const {
      return folly::hash::hash_combine(
        pointer_hash<IRInstruction>{}(p.inst),
        p.srcIdx
      );
    }
  };

  jit::fast_map<Phi, SSATmp*, Hasher> usedPhis;
  jit::fast_map<PhiUse, Phi, Hasher> phiUses;
};

//////////////////////////////////////////////////////////////////////

}

void cloneToBlock(const BlockList& /*rpoBlocks*/, IRUnit& unit,
                  Block::iterator const first, Block::iterator const last,
                  Block* const target) {
  StateVector<SSATmp,SSATmp*> rewriteMap(unit, nullptr);

  auto rewriteSources = [&] (IRInstruction* inst) {
    for (int i = 0; i < inst->numSrcs(); ++i) {
      if (auto newTmp = rewriteMap[inst->src(i)]) {
        FTRACE(5, "  rewrite: {} -> {}\n",
               inst->src(i)->toString(),
               newTmp->toString());
        inst->setSrc(i, newTmp);
      }
    }
  };

  auto targetIt = target->skipHeader();
  for (auto it = first; it != last; ++it) {
    assertx(!it->isControlFlow());

    FTRACE(5, "cloneToBlock({}): {}\n", target->id(), it->toString());
    auto const newInst = unit.clone(&*it);

    if (auto const numDests = newInst->numDsts()) {
      for (int i = 0; i < numDests; ++i) {
        FTRACE(5, "  add rewrite: {} -> {}\n",
               it->dst(i)->toString(),
               newInst->dst(i)->toString());
        rewriteMap[it->dst(i)] = newInst->dst(i);
      }
    }

    target->insert(targetIt, newInst);
    targetIt = ++target->iteratorTo(newInst);
  }

  postorderWalk(
    unit,
    [&](Block* block) {
      FTRACE(5, "cloneToBlock: rewriting block {}\n", block->id());
      for (auto& inst : *block) {
        FTRACE(5, " rewriting {}\n", inst.toString());
        rewriteSources(&inst);
      }
    },
    target
  );
}

void moveToBlock(Block::iterator const first,
                 Block::iterator const last,
                 Block* const target) {
  if (first == last) return;

  auto const srcBlock = first->block();

  auto targetIt = target->skipHeader();
  for (auto it = first; it != last;) {
    auto const inst = &*it;
    assertx(!inst->isControlFlow());

    FTRACE(5, "moveToBlock({}): {}\n",
           target->id(),
           inst->toString());

    it = srcBlock->erase(it);
    target->insert(targetIt, inst);
    targetIt = ++target->iteratorTo(inst);
  }
}

bool retypeDests(IRInstruction* inst, const IRUnit* /*unit*/) {
  auto changed = false;
  for (auto i = uint32_t{0}; i < inst->numDsts(); ++i) {
    DEBUG_ONLY auto const oldType = inst->dst(i)->type();
    if (retypeDst(inst, i)) {
      changed = true;
      ITRACE(5, "reflowTypes: retyped {} in {}\n", oldType.toString(),
             inst->toString());
    }
  }
  return changed;
}

/*
 * Algorithm for reflow:
 * 1. for each block in reverse postorder:
 * 2.   compute dest types of each instruction in forwards order
 * 3.   if the block ends with a jmp that passes types to a label,
 *      and the jmp is a loop edge,
 *      and any passed types cause the target label's type to widen,
 *      then set again=true
 * 4. if again==true, goto step 1
 */
void reflowTypes(IRUnit& unit) {
  auto const blocks = rpoSortCfg(unit);
  auto const ids    = numberBlocks(unit, blocks);

  auto isBackEdge = [&] (Block* from, Block* to) {
    return ids[from] > ids[to];
  };

  for (bool again = true; again;) {
    again = false;
    for (auto* block : blocks) {
      FTRACE(5, "reflowTypes: visiting block {}\n", block->id());
      for (auto& inst : *block) retypeDests(&inst, &unit);
      auto& jmp = block->back();
      auto n = jmp.numSrcs();
      if (!again && jmp.is(Jmp) && n > 0 && isBackEdge(block, jmp.taken())) {
        // if we pass a widening type to a label, loop again.
        auto srcs = jmp.srcs();
        auto dsts = jmp.taken()->front().dsts();
        for (unsigned i = 0; i < n; ++i) {
          if (srcs[i]->type() <= dsts[i]->type()) continue;
          again = true;
          break;
        }
      }
    }
  }
}

void insertNegativeAssertTypes(IRUnit& unit, const BlockList& blocks) {
  for (auto& blk : blocks) {
    auto const& inst = blk->back();
    if (!inst.is(CheckType)) continue;

    // Note that we can't assert the type if this isn't the only predecessor,
    // but also, it's currently not the case that it would ever be useful to do
    // so.  If a taken edge coming out of a CheckType is critical, even if we
    // split it so we could insert the AssertType, there would be no use of the
    // tmp in the edge-splitting-block to rewrite to the dst of the AssertType.
    if (inst.taken()->numPreds() != 1) continue;

    auto const checkTy = inst.typeParam();
    auto const srcTy   = inst.src(0)->type();
    auto const takenTy = negativeCheckType(srcTy, checkTy);
    if (takenTy < srcTy) {
      inst.taken()->prepend(
        unit.gen(AssertType, inst.bcctx(), takenTy, inst.src(0))
      );
    }
  }
}

void refineTmps(IRUnit& unit) {
  TRACE_SET_MOD(hhir_refineTmps);
  PassTracer tracer{&unit, Trace::hhir_refineTmps, "refineTmps"};
  Timer timer{Timer::optimize_refineTmps, unit.logEntry().get_pointer()};

  RefineTmps refiner{unit};
  auto const changed = refiner();
  if (changed) reflowTypes(unit);
  assertx(checkEverything(unit));
}

SSATmp* insertPhi(IRUnit& unit, Block* blk,
                  const jit::hash_map<Block*, SSATmp*>& inputs,
                  bool forceNew) {
  assertx(blk->numPreds() > 1);
  auto label = &blk->front();
  if (!label->is(DefLabel)) {
    label = unit.defLabel(1, blk, label->bcctx());
  } else if (forceNew) {
    unit.expandLabel(label, 1);
  } else {
    for (auto d = label->numDsts(); d--; ) {
      auto result = label->dst(d);
      blk->forEachPred([&](Block* pred) {
          if (result) {
            auto& jmp = pred->back();
            auto it = inputs.find(pred);
            assertx(it != inputs.end());
            if (jmp.src(d) != it->second) {
              result = nullptr;
            }
          }
        });
      if (result) return result;
    }
    unit.expandLabel(label, 1);
  }

  blk->forEachPred([&](Block* pred) {
      auto it = inputs.find(pred);
      assertx(it != inputs.end());
      unit.expandJmp(&pred->back(), it->second);
    });
  retypeDests(label, &unit);
  return label->dst(label->numDsts() - 1);
}

SSATmp* deletePhiDest(IRInstruction* label, unsigned i) {
  assertx(label->is(DefLabel));
  auto dest = label->dst(i);
  label->block()->forEachSrc(
    i, [&](IRInstruction* jmp, SSATmp* /*src*/) { jmp->deleteSrc(i); });
  label->deleteDst(i);
  return dest;
}

//////////////////////////////////////////////////////////////////////

}}
