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

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-info.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/ppc64-asm/asm-ppc64.h"

#include "hphp/util/dataflow-worklist.h"

#include <boost/range/adaptor/reversed.hpp>

TRACE_SET_MOD(vasm_graph_color);

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Register allocation using SSA graph-coloring:
 *
 * This register allocator is based on the theory of graph coloring on SSA
 * programs. Programs in strict SSA form have the unique property that their
 * interference graphs are chordal. This implies that the chromatic number of
 * the graph is equal to the size of its largest clique. This means that the
 * points at which spilling is required can be calculated easily (separate from
 * the coloring decisions), and once those spills are inserted, the program can
 * be colored trivially.
 *
 * This is more efficient than traditional graph-coloring. The interference
 * graph never needs to be materialized and we never need to "restart" from the
 * beginning. Moreover, spilling and coloring are divorced and handled in their
 * own separate passes. This makes for a more modular and easier to understand
 * algorithm. It does, however, require the unit to be completely in SSA form,
 * which adds overhead.
 *
 * The allocation is done in the following (broad) steps:
 *
 * - The unit is "prepared". This involves turning any physical registers into
 *   Vregs in strict SSA form. This, in turn, requires the transformation of
 *   certain vasm instructions into "pseudo" instructions. Constants are also
 *   materialized into actual Vregs here.
 *
 * - Register classes are inferred. This involves analysing the uses and defs of
 *   each Vreg to determine which register class they should be assigned to (GP
 *   or SIMD). The algorithm requires every register to be in a single register
 *   class and there can be no mixing of the two).
 *
 * - Spills and reloads are inserted. The unit is walked, keeping track of all
 *   live Vregs. At any point where the number of live Vregs exceeds the number
 *   of physical registers, spills are inserted to bring the number of live
 *   Vregs down. A reload is inserted where-ever an instruction uses a Vreg
 *   which is currently spilled.
 *
 * - Colors are assigned. Now that spills and reloads have been inserted, its
 *   guaranteed that every Vreg will always have at least one color available
 *   for it. Walk the unit and assign a free color for every Vreg.
 *
 * - Colors are optimized. The colors just assigned may not be optimal. Attempt
 *   to modify the assignments to reduce the number of unneeded copies. Spill
 *   slots are assigned.
 *
 * - Lower out of SSA. Modify the instructions to take the assigned physical
 *   registers. Convert the pseudo instructions back to their original
 *   instructions. Lower phis into register and spill slot moves. Optimize away
 *   no-op copies. Assign stack pointer adjustments as necessary for spill
 *   slots. At this point the unit no longer has any Vregs and is fully register
 *   allocated.
 *
 * - SF peephole optimizations. Not really register allocator related, but take
 *   advantage of liveness information to do a few peephole optimizations which
 *   are only valid when the flags register is dead.
 *
 * See the comments for each pass for more detailed information.
 *
 * This allocator is largely built on the ideas in the following:
 *
 * - "Register Allocation for Programs in SSA Form" by Sebastian Hack
 *
 * - "Register Spilling and Live-Range Splitting for SSA-Form Programs" by
 *    Matthias Braun and Sebastian Hack
 *
 * - "Preference-Guided Register Assignment" by Matthias Braun, Christoph
 *    Mallon, and Sebastian Hack (Compiler Construction 2010)
 *
 * - "Revisiting Out-of-SSA Translation for Correctness, Code Quality, and
 *    Efficiency" by Benoit Boissinot, Alain Darte, Fabrice Rastello, Benoit
 *    Dupont de Dinechin, Christophe Guillon
 */

namespace {

//////////////////////////////////////////////////////////////////////
// State

// RegClass determine what set of things a Vreg can be colored to. This includes
// whether the Vreg represents a spill slot, and whether its a wide value or
// not. It is inferred by looking at how a Vreg is used and defined.
enum RegClass {
  Any,      // Completely unconstrained. This is the default, but is not valid
            // for a used register.
  AnyNarrow,// This means the Vreg can be safely given a GP or SIMD register and
            // is non-wide. However, the graph coloring algorithm cannot handle
            // this kind of constraint (union of two other constraints), so
            // right now its effectively a synonym for GP. TODO (T37587676) to
            // try to deal with this better.

  GP,       // Generic purpose register
  SIMD,     // SIMD register
  SIMDWide, // 128-bit SIMD register
  SF,       // Singleton flags register
  Spill,    // Spill slot
  SpillWide // 128-bit spill slot
};

// State about each Vreg. Instead of separate data-structures, all Vreg
// information is concentrated in this one data-structure.
struct RegInfo {
  RegClass regClass = RegClass::Any;
  // The physical register this was before it was SSA-ized. The allocator will
  // ensure that this Vreg will be assigned the same physical register at all
  // uses and defs (except copies).
  PhysReg precolor = InvalidReg;
  // Can this Vreg be potentially rematerialized (instead of reloaded) by this
  // instruction?
  folly::Optional<Vinstr> remat;
};

using BlockVector = jit::vector<Vlabel>;
using WeightMap = jit::fast_map<Vreg, uint64_t>;
using PhiWeightVector = jit::vector<folly::Optional<uint64_t>>;

// Information about each inferred loop. A loop is represented by its header
// block.
struct LoopInfo {
  VregSet uses;            // Vregs used inside
  size_t gpPressure = 0;   // Max GP and SIMD pressure
  size_t simdPressure = 0;
  size_t depth = 0;        // Nesting depth (this will always be at least one
                           // once the information is initialized).
};

// Global allocator state.
struct State {
  Vunit& unit;
  const Abi& abi;

  // CFG information
  BlockVector rpo;
  jit::vector<size_t> rpoOrder;
  PredVector preds;
  VIdomVector idoms;
  // Total ordering of the blocks according to the dominator tree
  jit::vector<size_t> domOrder;

  // Registers available for selection by the allocator.
  const RegSet gpUnreserved;
  const RegSet simdUnreserved;
  // Registers not available for selection. These will be left untouched if
  // present.
  const VregSet reservedRegs;
  // SIMD scratch register for resolving shuffles.
  const PhysReg scratch;

  // All the instructions which have been converted to
  // pseudo-instructions. Those pseudo-instructions have the index into this
  // vector stored in their "pos" field to map back.
  jit::vector<Vinstr> pseudos;

  // Liveness information
  jit::vector<VregSet> liveIn;
  jit::vector<VregSet> liveOut;

  // Loop information. A loop is represented by its header block.
  jit::fast_map<Vlabel, LoopInfo> loopInfo;
  // Map of block to the inner-most loop it belongs to. Blocks not contained
  // within a loop will not be present.
  jit::fast_map<Vlabel, Vlabel> blockToLoop;

  // For each block, a map of Vregs to their spill weight at that block. Vregs
  // with higher spill weights are preferable to spill.
  jit::vector<WeightMap> spillWeightsIn;
  jit::vector<WeightMap> spillWeightsOut;
  // For each block that has a phidef, a map of phi indices to the spill weight
  // for that phi output.
  jit::vector<PhiWeightVector> spillPhiWeights;

  // Vreg state
  jit::vector<folly::Optional<RegInfo>> regInfo;
};

//////////////////////////////////////////////////////////////////////
// Pretty-printers

std::string show(RegClass r) {
  switch (r) {
    case RegClass::Any:       return "any";
    case RegClass::AnyNarrow: return "any-narrow";
    case RegClass::GP:        return "gp";
    case RegClass::SIMD:      return "simd";
    case RegClass::SIMDWide:  return "simd-wide";
    case RegClass::SF:        return "sf";
    case RegClass::Spill:     return "spill";
    case RegClass::SpillWide: return "spill-wide";
  }
  always_assert(false);
}

std::string show(const BlockVector& v) {
  using namespace folly::gen;
  return folly::sformat(
    "[{}]",
    from(v)
      | map([] (Vlabel b) { return folly::sformat("{}", b); })
      | unsplit<std::string>(", ")
  );
}

std::string show(const WeightMap& w) {
  std::vector<std::pair<Vreg, uint64_t>> sorted{w.begin(), w.end()};
  std::sort(sorted.begin(), sorted.end());

  using namespace folly::gen;
  return folly::sformat(
    "{{{}}}",
    from(sorted)
      | map([] (const std::pair<Vreg, uint64_t>& p) {
          return folly::sformat("{} -> {}", show(p.first), p.second);
        })
      | unsplit<std::string>(", ")
  );
}

std::string show(const PhiWeightVector& v) {
  using namespace folly::gen;
  return folly::sformat(
    "[{}]",
    from(v)
      | map([] (const folly::Optional<uint64_t>& w) -> std::string {
          if (!w) return "*";
          return std::to_string(*w);
        })
      | unsplit<std::string>(", ")
  );
}

std::string show(const Vunit& unit, const RegInfo& info) {
  return folly::sformat(
    "Class: {:10}, Pre-Color: {}, Mat: ({})",
    show(info.regClass),
    info.precolor != InvalidReg ? show(info.precolor) : "-",
    info.remat ? show(unit, *info.remat) : "-"
  );
}

std::string show(const LoopInfo& info) {
  return folly::sformat(
    "Depth: {:2}, GP Pressure: {:3}, SIMD Pressure: {:3}, Uses: {}",
    info.depth,
    info.gpPressure,
    info.simdPressure,
    show(info.uses)
  );
}

std::string show(const State& state) {
  auto const dumpBlockInfo = [&] (auto const& info) -> std::string {
    if (info.empty()) return "";
    std::string str;
    for (auto const b : state.rpo) {
      if (info[b].empty()) continue;
      str += folly::sformat(
        "  {:5} -> {}\n",
        b, show(info[b])
      );
    }
    return str;
  };

  return folly::sformat(
    "GP Unreserved:        {}\n"
    "SIMD Unreserved:      {}\n"
    "Reserved Regs:        {}\n"
    "Scratch Reg:          {}\n"
    "RPO:                  {}\n"
    "Idoms:                {}\n"
    "Reg Info:\n{}"
    "Live In:\n{}"
    "Live Out:\n{}"
    "Loop Info:\n{}"
    "Block To Loop:\n{}"
    "Spill Weights In:\n{}"
    "Spill Weights Out:\n{}"
    "Phi Weights:\n{}",
    show(state.gpUnreserved),
    show(state.simdUnreserved),
    show(state.reservedRegs),
    show(state.scratch),
    show(state.rpo),
    [&]{
      std::string str = "[";
      auto first = true;
      for (auto const b : state.rpo) {
        if (!first) str += ", ";
        first = false;
        str += folly::sformat("{} -> {}", b, state.idoms[b]);
      }
      str += "]";
      return str;
    }(),
    [&]{
      std::string str;
      for (size_t i = 0; i < state.regInfo.size(); ++i) {
        auto const& info = state.regInfo[i];
        if (!info) continue;
        str += folly::sformat(
          "  {:6} -> {}\n",
          show(Vreg{i}),
          show(state.unit, *info)
        );
      }
      return str;
    }(),
    dumpBlockInfo(state.liveIn),
    dumpBlockInfo(state.liveOut),
    [&]{
      std::string str;
      for (auto const& kv : state.loopInfo) {
        str += folly::sformat(
          "  {:5} -> {}\n",
          kv.first,
          show(kv.second)
        );
      }
      return str;
    }(),
    [&]{
      std::string str;
      for (auto const& kv : state.blockToLoop) {
        str += folly::sformat(
          "  {:5} -> {}\n",
          kv.first,
          kv.second
        );
      }
      return str;
    }(),
    dumpBlockInfo(state.spillWeightsIn),
    dumpBlockInfo(state.spillWeightsOut),
    dumpBlockInfo(state.spillPhiWeights)
  );
}

//////////////////////////////////////////////////////////////////////
// RegInfo accessors

// Access the RegInfo for the given Vreg (the info should exist).
RegInfo& reg_info(State& state, Vreg r) {
  assertx(r.isValid());
  assertx(!r.isPhys());
  assertx(r < state.regInfo.size());
  auto& info = state.regInfo[r];
  assertx(info);
  return *info;
}

// Access the const RegInfo for the given Vreg (the info should exist).
const RegInfo& reg_info(const State& state, Vreg r) {
  assertx(r.isValid());
  assertx(!r.isPhys());
  assertx(r < state.regInfo.size());
  auto const& info = state.regInfo[r];
  assertx(info);
  return *info;
}

// Access the RegInfo for the given Vreg, create new state if not present.
RegInfo& reg_info_create(State& state, Vreg r) {
  assertx(r.isValid());
  assertx(!r.isPhys());
  if (state.regInfo.size() <= r) state.regInfo.resize(size_t{r}+1);
  auto& info = state.regInfo[r];
  if (!info) info.emplace();
  return *info;
}

// Add RegInfo for the given Vreg (the Vreg should not already have info).
RegInfo& reg_info_insert(State& state, Vreg r, RegInfo info) {
  assertx(r.isValid());
  assertx(!r.isPhys());
  if (state.regInfo.size() <= r) state.regInfo.resize(size_t{r}+1);
  auto& newInfo = state.regInfo[r];
  assertx(!newInfo);
  newInfo.emplace(std::move(info));
  return *newInfo;
}

//////////////////////////////////////////////////////////////////////
// State utilities

void compute_rpo(State& state) {
  auto rpo = sortBlocks(state.unit);
  jit::vector<size_t> rpoOrder(state.unit.blocks.size());
  for (size_t i = 0; i < rpo.size(); ++i) rpoOrder[rpo[i]] = i;
  state.rpo = std::move(rpo);
  state.rpoOrder = std::move(rpoOrder);
}

// Calculate immediate dominators and dominator tree ordering
void compute_dominator_info(State& state) {
  state.idoms = findDominators(state.unit, state.rpo);

  // Reverse the idoms to build the explicit dominator tree
  jit::vector<BlockVector> dominatorTree(state.unit.blocks.size());
  for (auto const b : state.rpo) {
    auto const parent = state.idoms[b];
    if (!parent.isValid()) {
      assertx(b == state.unit.entry);
      continue;
    }
    dominatorTree[parent].emplace_back(b);
  }

  // Then walk it in pre-order to build a total ordering of blocks inside it.
  state.domOrder.resize(state.unit.blocks.size());

  size_t order = 0;
  jit::stack<Vlabel> dfs;
  dfs.push(state.unit.entry);
  while (!dfs.empty()) {
    auto const b = dfs.top();
    dfs.pop();
    state.domOrder[b] = order++;
    for (auto const child : dominatorTree[b]) dfs.push(child);
  }
}

// Calculate liveness in the traditional dataflow way. There's more efficient
// algorithms leveraging SSA but they require tracking use and def positions and
// it doesn't seem worth it (this is fast enough in practice).
void calculate_liveness(State& state) {
  auto const& unit = state.unit;

  state.liveIn.resize(unit.blocks.size());
  state.liveOut.resize(unit.blocks.size());

  jit::vector<VregSet> gen(unit.blocks.size());
  jit::vector<VregSet> kill(unit.blocks.size());

  dataflow_worklist<size_t, std::less<size_t>> worklist(state.rpo.size());
  for (size_t i = 0; i < state.rpo.size(); ++i) {
    auto const b = state.rpo[i];
    auto const& block = unit.blocks[b];
    auto& g = gen[b];
    auto& k = kill[b];
    for (auto const& inst : boost::adaptors::reverse(block.code)) {
      visitDefs(
        unit, inst,
        [&](Vreg r) {
          if (r.isPhys()) return;
          if (reg_info(state, r).regClass == RegClass::SF) return;
          k.add(r);
          g.remove(r);
        }
      );
      visitUses(
        unit, inst,
        [&](Vreg r) {
          if (r.isPhys()) return;
          if (reg_info(state, r).regClass == RegClass::SF) return;
          g.add(r);
        }
      );
    }
    state.liveIn[b].reset();
    state.liveOut[b].reset();
    worklist.push(i);
  }

  while (!worklist.empty()) {
    auto const b = state.rpo[worklist.pop()];
    auto const& block = unit.blocks[b];

    auto& out = state.liveOut[b];
    out.reset();
    for (auto const s : succs(block)) out |= state.liveIn[s];
    auto transfer = out;
    transfer -= kill[b];
    transfer |= gen[b];

    if (transfer != state.liveIn[b]) {
      for (auto const pred : state.preds[b]) {
        worklist.push(state.rpoOrder[pred]);
      }
      state.liveIn[b] = std::move(transfer);
    }
  }
}

// Helper function to retrieve loop info (and asserting if it doesn't
// exist). Avoid using [] because it will create new entries.
const LoopInfo& loop_info(const State& state, Vlabel loop) {
  auto const it = state.loopInfo.find(loop);
  assertx(it != state.loopInfo.end());
  return it->second;
};
LoopInfo& loop_info(State& state, Vlabel loop) {
  auto const it = state.loopInfo.find(loop);
  assertx(it != state.loopInfo.end());
  return it->second;
};

// Calculate various loop metadata
void calculate_loop_info(State& state) {
  auto const& unit = state.unit;

  // Find the loops
  auto const backEdges = findBackEdges(unit, state.rpo, state.idoms);
  if (backEdges.empty()) return;

  auto const loopBlocks = findLoopBlocks(unit, state.preds, backEdges);

  // First create entries in loopInfo for each block. After this, we'll never
  // create new entries, just modify the existing ones.
  state.loopInfo.reserve(loopBlocks.size());
  for (auto const& p : loopBlocks) {
    auto const DEBUG_ONLY result = state.loopInfo.emplace(p.first, LoopInfo{});
    assertx(result.second);
  }

  // Now calculate the loop nesting depths by bumping it for every loop
  // contained within another. Also track which loops are members of other loops
  // for the register pressure calculation.
  std::unordered_multimap<Vlabel, Vlabel> membership;
  for (auto const& p : loopBlocks) {
    for (auto const b : p.second) {
      auto const it = state.loopInfo.find(b);
      if (it == state.loopInfo.end()) continue; // Not a loop header
      ++it->second.depth;
      membership.emplace(b, p.first);
    }
  }

  // Create the block to loop mapping. A block will be assigned to the loop with
  // the highest depth (the inner-most nested one).
  for (auto const& p : loopBlocks) {
    auto const newLoopInfo = state.loopInfo.find(p.first);
    assertx(newLoopInfo != state.loopInfo.end());

    for (auto const b : p.second) {
      auto& loop = state.blockToLoop[b];
      if (!loop.isValid()) { // First assignment
        loop = p.first;
        continue;
      }
      // Its already assigned a loop. Use the new one if it has a higher depth.
      auto const oldLoopInfo = state.loopInfo.find(loop);
      assertx(oldLoopInfo != state.loopInfo.end());
      if (newLoopInfo->second.depth > oldLoopInfo->second.depth) {
        loop = p.first;
      }
    }
  }

  // Now calculate the maximum register pressure inside each loop alongside
  // which registers are used in each loop. The heuristic which wants to know
  // which registers are used in each loop performs better if you ignore copies
  // whose dest is not used in the loop. Because of this the analysis is flow
  // sensitive, so we loop until we hit a fixed point.

  // Record that a register is used in a particular loop
  auto const markUse = [&] (Vreg r, Vlabel loop) {
    if (r.isPhys()) return false;
    if (reg_info(state, r).regClass == RegClass::SF) return false;
    auto& info = loop_info(state, loop);
    auto const oldSize = info.uses.size();
    info.uses.add(r);
    return info.uses.size() != oldSize;
  };

  auto changed = true;
  while (changed) {
    changed = false;

    // Do the analysis for each block
    for (auto const b : state.rpo) {
      // Find the loops that this block belongs to (as an iterator range).
      auto const loops = [&] {
        auto const it = state.blockToLoop.find(b);
        if (it == state.blockToLoop.end()) {
          return std::make_pair(membership.end(), membership.end());
        }
        auto const range = membership.equal_range(it->second);
        assertx(range.first != range.second);
        return range;
      }();
      // Not in a loop, skip
      if (loops.first == loops.second) continue;

      size_t maxGPPressure = 0;
      size_t maxSIMDPressure = 0;
      // The live registers
      VregSet gpLive;
      VregSet simdLive;

      auto const liveUse = [&] (Vreg r) {
        if (r.isPhys()) return;
        switch (reg_info(state, r).regClass) {
          case RegClass::GP:
          case RegClass::AnyNarrow:
            return gpLive.add(r);
          case RegClass::SIMD:
          case RegClass::SIMDWide:
            return simdLive.add(r);
          case RegClass::SF:
            return;
          case RegClass::Any:
          case RegClass::Spill:
          case RegClass::SpillWide:
            break;
        }
        always_assert(false);
      };
      auto const liveDef = [&] (Vreg r) {
        if (r.isPhys()) return;
        switch (reg_info(state, r).regClass) {
          case RegClass::GP:
          case RegClass::AnyNarrow:
            return gpLive.remove(r);
          case RegClass::SIMD:
          case RegClass::SIMDWide:
            return simdLive.remove(r);
          case RegClass::SF:
            return;
          case RegClass::Any:
          case RegClass::Spill:
          case RegClass::SpillWide:
            break;
        }
        always_assert(false);
      };

      // First take into account registers which are live-out of this
      // block. These might include registers which aren't used by any
      // instructions in the block, but still increase the register pressure
      // (because they're live-thru).
      state.liveOut[b].forEach(liveUse);

      // The register pressure can increase and decrease within a particular
      // instruction (as we account for the uses and defs), so update the
      // maximum at certain points according to the currently live registers.
      auto const update = [&]{
        maxGPPressure   = std::max(maxGPPressure, gpLive.size());
        maxSIMDPressure = std::max(maxSIMDPressure, simdLive.size());
      };
      update();

      /*
       * We want to model register liveness here (to account for which registers
       * are alive at which points), which is easier to do backwards.
       *
       * The operation for all instructions is the same:
       *
       * - Mark defs as live. Even if a def isn't live (because its unused), it
       *   still increases register pressure because the instruction has to
       *   write the result somewhere.
       *
       * - Mark acrosses as live. Acrosses are like used, except they're alive
       *   at the same time as the defs. Therefore they have to be marked alive
       *   here.
       *
       * - Snapshot currently live registers to record pressure.
       *
       * - Mark defs as dead (reducing register pressure).
       *
       * - Mark uses as alive.
       *
       * - Snapshot currently live registers to record pressure.
       *
       * For the non-copy instructions, we'll also mark any uses as being used
       * in all the loops the current block belongs to. For copy instructions,
       * we only do that if the destination of the copy is also used in the
       * block (this is why we have to repeat all this until we reach a fixed
       * point).
       */
      for (auto const& inst : boost::adaptors::reverse(unit.blocks[b].code)) {
        switch (inst.op) {
          case Vinstr::copy:
            assertx(acrossesSet(unit, inst).none());
            for (auto loop = loops.first; loop != loops.second; ++loop) {
              if (loop_info(state, loop->second).uses[inst.copy_.d]) {
                changed |= markUse(inst.copy_.s, loop->second);
              }
            }
            liveUse(inst.copy_.d);
            update();
            liveDef(inst.copy_.d);
            liveUse(inst.copy_.s);
            update();
            break;
          case Vinstr::copyargs: {
            assertx(acrossesSet(unit, inst).none());
            auto const& s = unit.tuples[inst.copyargs_.s];
            auto const& d = unit.tuples[inst.copyargs_.d];
            assertx(s.size() == d.size());
            std::for_each(d.begin(), d.end(), liveUse);
            update();
            std::for_each(d.begin(), d.end(), liveDef);
            for (size_t i = 0; i < s.size(); ++i) {
              for (auto loop = loops.first; loop != loops.second; ++loop) {
                if (loop_info(state, loop->second).uses[d[i]]) {
                  changed |= markUse(s[i], loop->second);
                }
              }
              liveUse(s[i]);
            }
            update();
            break;
          }
          case Vinstr::phijmp: {
            assertx(acrossesSet(unit, inst).none());
            assertx(defsSet(unit, inst).none());
            auto const& s = unit.tuples[inst.phijmp_.uses];
            auto const& target = unit.blocks[inst.phijmp_.target].code.front();
            assertx(target.op == Vinstr::phidef);
            auto const& d = unit.tuples[target.phidef_.defs];
            assertx(s.size() == d.size());
            for (size_t i = 0; i < s.size(); ++i) {
              for (auto loop = loops.first; loop != loops.second; ++loop) {
                if (loop_info(state, loop->second).uses[d[i]]) {
                  changed |= markUse(s[i], loop->second);
                }
              }
              liveUse(s[i]);
            }
            update();
            break;
          }
          default:
            visitDefs(unit, inst, liveUse);
            visitAcrosses(
              unit, inst,
              [&] (Vreg r) {
                for (auto loop = loops.first; loop != loops.second; ++loop) {
                  changed |= markUse(r, loop->second);
                }
                liveUse(r);
              }
            );
            update();
            visitDefs(unit, inst, liveDef);
            visitUses(
              unit, inst,
              [&] (Vreg r) {
                for (auto loop = loops.first; loop != loops.second; ++loop) {
                  changed |= markUse(r, loop->second);
                }
                liveUse(r);
              }
            );
            update();
            break;
        }
      }

      // Mark sure our liveness analysis matches which calculate_liveness()
      // found.
      assertx((gpLive | simdLive) == state.liveIn[b]);

      // We have the maximum register pressure for this block, so use it to
      // potentially increase the register pressure for the loops the block
      // belongs to.
      for (auto loop = loops.first; loop != loops.second; ++loop) {
        auto& info = loop_info(state, loop->second);
        info.gpPressure = std::max(info.gpPressure, maxGPPressure);
        info.simdPressure = std::max(info.simdPressure, maxSIMDPressure);
      }
    }
  }
}

// Map an arbitrary block to the depth of the loop it belongs to (if any).
size_t block_loop_depth(const State& state, Vlabel b) {
  auto const it = state.blockToLoop.find(b);
  if (it == state.blockToLoop.end()) return 0; // Not in a loop
  auto const& info = loop_info(state, it->second);
  // The depth includes itself, so must always be at least 1.
  assertx(info.depth > 0);
  return info.depth;
}

State make_state(Vunit& unit, const Abi& abi) {
  assertx(!abi.unreserved().contains(rsp()));

  // Reserve a SIMD scratch register to resolve register shuffles.
  auto const scratch = [&] () -> PhysReg {
    switch (arch()) {
      case Arch::X64:   return reg::xmm15;
      case Arch::ARM:   return vixl::d31;
      case Arch::PPC64: return ppc64_asm::reg::v29;
    }
    always_assert(false);
  }();
  assertx(scratch.isSIMD());

  State state{
    unit,
    abi,
    {},
    {},
    computePreds(unit),
    {},
    {},
    abi.gpUnreserved,
    abi.simdUnreserved - scratch,
    VregSet{(abi.all() - (abi.gpUnreserved | abi.simdUnreserved)) | scratch},
    scratch
  };

  compute_rpo(state);
  compute_dominator_info(state);
  return state;
}

//////////////////////////////////////////////////////////////////////
// Pre-allocation preparation

/*
 * Constant placement
 *
 * Vregs representing constants need to be materialized and placed in the unit
 * before register allocation. Each constant needs to be materialized before its
 * first usage. The easiest thing would be to just place constants in the entry
 * block, but that's not optimal.
 *
 * Ideally a constant should be (1) placed as close to possible to its uses (to
 * minimize register pressure), (2) as few times as possible (to minimize code
 * size), and should not be (3) placed on a path where it may not be used (to
 * minimize useless work).
 *
 * (1) can be dealt with by starting at the entry block and then sinking the
 * definitions as far as possible using standard dataflow. This will prevent us
 * from sinking constant definitions inside loops, which is desirable, and
 * satisfies (3), but tends to violate (2). With no other constraints, the
 * definitions will tend to be sunk to immediately before all their usages,
 * leading to code duplication.
 *
 * To prevent this (and satisfy (2)), we first calculate "sink stops". These
 * indicate that at a particular block, a particular Vreg (representing the
 * constant) should not be sunk further.
 *
 * The sink stops for a constant are the post-dominator frontier of all usages
 * of that constant. That is, the blocks where the constant is used on all
 * successors from that block (or in the block itself), but where the
 * predecessor is not a sink stop for that constant. This captures (2) and (3)
 * as we will not place the constant where there's a path where it might be
 * used, but if the constant is used on all paths, we won't duplicate it along
 * those paths.
 *
 * Once sink stops are calculated, we start at the entry block and attempt to
 * sink each constant as far as possible. We use typical partial redundancy
 * dataflow for this and stop sinking at a sink stop, or if we encounter a use
 * of that constant. In addition, we use liveness of the constants to avoid
 * sinking down paths where the constant isn't used at all.
 *
 * Once we stop sinking, we insert the appropriate "ldimm" vasm instruction
 * (writing to its associated Vreg) at that point to materialize the constant. A
 * given constant (which has a single Vreg) may be placed at multiple points,
 * which means that Vreg now has multiple definitions. This is fine, as the unit
 * will be rewritten into SSA form anyways, which will fix this.
 */

struct PlaceConstantsBlockInfo {
  VregSet stopSink;
  VregSet liveIn;
};

// Calculate sink stop and liveness information for each constant for every
// block, returning an empty vector if there's no constants.
jit::vector<PlaceConstantsBlockInfo>
compute_place_constants_block_info(const State& state) {
  auto const& unit = state.unit;
  auto const& rpo = state.rpo;
  auto const& rpoOrder = state.rpoOrder;
  auto const& preds = state.preds;

  if (unit.regToConst.empty()) return {};

  /* Per-block dataflow state:
   *
   * We keep track of two things for dataflow:
   *
   * definitelyUsed - True if a register is used in the block, or if
   *                  definitelyUsed for all successors.
   *
   * nearestUse - This block if the register is used by this block, or if
   *              there's different blocks in the uses state among the
   *              successors. Otherwise its the same as all the successors' uses
   *              block.
   */
  struct BlockState {
    // Mapping of a constant to its nearest usage (or union of multiple usages)
    // at this block
    jit::fast_map<Vreg, Vlabel> nearestUse;
    // Set of constants which are used along all paths from this block
    VregSet definitelyUsed;
    bool operator==(const BlockState& o) const {
      return std::tie(nearestUse, definitelyUsed) ==
        std::tie(o.nearestUse, o.definitelyUsed);
    }
    bool operator!=(const BlockState& o) const { return !(*this == o); }
  };

  // Dataflow bock state. As an optimization, we store the state in a copy_ptr,
  // which lets us share it between blocks cheaply. We use an empty copy_ptr to
  // represent the empty state.
  jit::vector<copy_ptr<BlockState>> blockIn(unit.blocks.size());

  // Mapping of blocks to the constants used in that block
  jit::vector<VregSet> blockUses(unit.blocks.size());

  dataflow_worklist<size_t, std::less<size_t>> worklist(rpo.size());

  boost::dynamic_bitset<> visited(unit.blocks.size());
  visited.flip();

  // Populate the constant usages for each block. Since this is a backwards
  // dataflow, start the worklist with successor-less blocks (the exits).
  auto foundUse = false;
  for (size_t i = 0; i < rpo.size(); ++i) {
    auto const b = rpo[i];
    auto const& block = unit.blocks[b];
    auto& uses = blockUses[b];

    for (auto const& inst : block.code) {
      visitUses(
        unit,
        inst,
        [&] (Vreg r) { if (unit.regToConst.count(r)) uses.add(r); }
      );

      // A constant should never have a definition (before we place them).
      if (debug) {
        visitDefs(
          unit,
          inst,
          [&] (Vreg r) { always_assert(!unit.regToConst.count(r)); }
        );
      }
    }

    visited[b] = false;
    if (!foundUse && uses.any()) foundUse = true;
    if (succs(block).size() == 0) worklist.push(i);
  }

  // The unit has constants but they're never actually used. Bail out.
  if (!foundUse) return {};

  while (!worklist.empty()) {
    auto const b = rpo[worklist.pop()];
    auto const& block = unit.blocks[b];

    auto const successors = succs(block);

    // All successors are identical if there's no adjacent successors which are
    // different.
    auto const allSuccsIdentical =
      std::adjacent_find(
        successors.begin(),
        successors.end(),
        [&] (Vlabel s1, Vlabel s2) {
          // NB: We're checking if the successors are different, so the return
          // values are inverted.
          if (!visited[s1] || !visited[s2]) return true;
          auto const& succState1 = blockIn[s1];
          auto const& succState2 = blockIn[s2];
          // Short-cut if they're the same pointer
          if (succState1.get() == succState2.get()) return false;
          if (!succState1 || !succState2) return true;
          return *succState1 != *succState2;
        }
      ) == successors.end();

    // Schedule predecessors if the state has changed. Return true if the state
    // changed, false otherwise.
    auto const schedulePreds = [&] (const BlockState* newState) {
      auto const& oldState = blockIn[b];
      assertx(!oldState || newState);
      if (!visited[b] || (!oldState && newState) ||
          (oldState.get() != newState && *oldState != *newState)) {
        for (auto const pred : preds[b]) worklist.push(rpoOrder[pred]);
        return true;
      }
      return false;
    };

    auto const& uses = blockUses[b];
    if (allSuccsIdentical) {
      // All the successors have identical state. We can just cheaply copy one
      // of the successors state for our own (because they're in copy_ptrs).
      assertx(successors.empty() || visited[successors[0]]);

      // Copy arbitrary successor
      auto newState = decltype(blockIn)::value_type{};
      if (!successors.empty()) newState = blockIn[successors[0]];

      if (uses.any()) {
        // If this block has any usages, we have to mutate the state (and make a
        // deep copy). Since this block uses the constant, that constant is
        // definitely used at this point.
        if (!newState) newState.emplace();
        auto mutated = newState.mutate();
        mutated->definitelyUsed |= uses;
        uses.forEach(
          [&] (Vreg r) { mutated->nearestUse.insert_or_assign(r, b); }
        );
      }
      // Update this block's state and schedule any predecessors
      if (schedulePreds(newState.get())) blockIn[b] = newState;
    } else {
      // Successors have differing states. We can't copy it cheaply but need to
      // compute the union among them.
      assertx(!successors.empty());

      BlockState newState;
      auto first = true;
      for (auto const& succ : successors) {
        if (!visited[succ]) continue;

        auto const& succState = blockIn[succ];
        if (!succState) {
          // Successor doesn't use anything, so nothing from the successors is
          // definitely used.
          newState.definitelyUsed.reset();
          first = false;
          continue;
        }
        // Merge together usages from this successor. If we already have a usage
        // for this constant and that usage isn't the same block, we're at a
        // join point between distinct usages. Treat the current block as the
        // usage point.
        for (auto const& p : succState->nearestUse) {
          auto const result = newState.nearestUse.emplace(p.first, p.second);
          if (!result.second && result.first->second != p.second) {
            result.first->second = b;
          }
        }
        // Constants are definitely used only if they're definitely used by all
        // of their successors.
        if (first) {
          newState.definitelyUsed = succState->definitelyUsed;
          first = false;
        } else {
          newState.definitelyUsed &= succState->definitelyUsed;
        }
      }
      assertx(!first);

      // If this block uses a constant, that constant is definitely used at this
      // point.
      newState.definitelyUsed |= uses;
      uses.forEach(
        [&] (Vreg r) { newState.nearestUse.insert_or_assign(r, b); }
      );

      // Update this block's state and schedule any predecessors
      if (schedulePreds(&newState)) blockIn[b].emplace(std::move(newState));
    }

    visited[b] = true;
  };

  always_assert(visited.all());

  jit::vector<PlaceConstantsBlockInfo> outInfo(unit.blocks.size());
  for (auto const b : rpo) {
    auto const& state = blockIn[b];
    if (!state) continue;

    auto& out = outInfo[b];

    for (auto const& p : state->nearestUse) {
      // A constant is live if there's any usage from this block.
      out.liveIn.add(p.first);
      // We want to stop sinking at this block only if its definitely used from
      // this point, and the nearest usage is this block. The latter condition
      // implies that either the constant is used in this block, or there's at
      // least two distinct use sites from this block. Since the constant is
      // definitely used, its not profitable to sink any farther.
      if (state->definitelyUsed[p.first] && p.second == b) {
        out.stopSink.add(p.first);
      }
    }
  }

  return outInfo;
}

// Materialize constants by placing instructions to create them at the
// appropriate points. The set of registers used is returned.
VregSet place_constants(State& state) {
  auto const blockInfo = compute_place_constants_block_info(state);
  if (blockInfo.empty()) return {};

  auto& unit = state.unit;
  assertx(blockInfo.size() == unit.blocks.size());

  auto const& rpo = state.rpo;
  auto const& rpoOrder = state.rpoOrder;
  auto const& preds = state.preds;

  // The set of constants in each block which can be sunk to the beginning of
  // that block.
  jit::vector<VregSet> inStates(unit.blocks.size());
  // The set of constants in each block which can be sunk to the end of that
  // block.
  jit::vector<VregSet> outStates(unit.blocks.size());
  boost::dynamic_bitset<> visited(unit.blocks.size());

  dataflow_worklist<size_t> worklist(rpo.size());

  assertx(!rpo.empty() && rpo[0] == unit.entry);
  worklist.push(0);

  /*
   * Dataflow:
   *
   * IN(B) = intersection of OUT(P) for each P in PREDS(B)
   *
   * (It is safe to sink a constant to the beginning of B if it can be sunk to
   * the end of every predecessor).
   *
   * OUT(B) = (IN(B) - STOP(B)) & LIVE(B)
   *
   * (It is safe to sink a constant to the end of B if it can be sunk to the
   * beginning of B, does not have a stop sink, and is live from B.
   *
   * PLACE(B) = (IN(B) & LIVE(B) & STOP(B)) | (OUT(B) - (intersection of IN(S)
   *             for each S in SUCCS(B)))
   *
   * (A constant should be placed in B if it can be sunk to the beginning of B,
   * is live in B, and is stopped in B. It should also be placed in B if it can
   * be sunk to the end of B, and can not be sunk to the beginning of all of B's
   * successors).
   */
  do {
    auto const b = rpo[worklist.pop()];
    auto const& info = blockInfo[b];
    auto const& predList = preds[b];
    auto& in = inStates[b];
    auto& out = outStates[b];

    if (b == unit.entry) {
      assertx(predList.empty());
      assertx(!visited[b]);
      assertx(in.none());
      assertx(out.none());
      // A constant can be trivially "sunk" to the beginning of the entry block
      // if its live.
      in = info.liveIn;
    } else {
      assertx(!predList.empty());

      // Find all the constants which can be sunk to the beginning of this
      // block. A constant can be sunk to the beginning of this block if it can
      // be sunk to the end of all of its predecessors.

      in.reset();
      // Find the first visited predecessor. There has to be at least one
      // because we're going in RPO order.
      auto predIt = std::find_if(
        predList.begin(),
        predList.end(),
        [&] (Vlabel p) { return visited[p]; }
      );
      assertx(predIt != predList.end());

      in = outStates[*predIt];
      for (++predIt; predIt != predList.end(); ++predIt) {
        if (visited[*predIt]) in &= outStates[*predIt];
      }
    }

    // From the constants which can be sunk to the beginning of the block, find
    // the subset which can be sunk to the end. A constant can be sunk through
    // the block if there's not a sink stop and if its still live in the block.
    auto newOut = in;
    newOut -= info.stopSink;
    newOut &= info.liveIn;

    // Schedule successors if the out state has changed.
    if (!visited[b] || newOut != out) {
      out = std::move(newOut);
      for (auto const succ : succs(unit.blocks[b])) {
        worklist.push(rpoOrder[succ]);
      }
    }

    visited[b] = true;
  } while (!worklist.empty());

  VregSet allPlaced;
  VregSet placeThisInst;
  for (auto const b : rpo) {
    auto place = [&] {
      // Find the constants which should be materialized in this block. The
      // constant should be materialized if it can be sunk to the beginning of
      // the block, is live in that block, and we have a sink stop.
      auto const& info = blockInfo[b];
      auto place = inStates[b];
      place &= info.liveIn;
      place &= info.stopSink;

      auto const& succList = succs(unit.blocks[b]);
      assertx(!succList.empty() || outStates[b].none());
      if (!succList.empty()) {
        // If a constant could be sunk to the end of the block, but not sunk to
        // the beginning of all of its predecessors, we also need to place it
        // here.
        auto sunk = inStates[succList[0]];
        for (size_t i = 1; i < succList.size(); ++i) {
          sunk &= inStates[succList[i]];
        }
        auto didntSink = outStates[b];
        didntSink -= sunk;
        place |= didntSink;
      }
      return place;
    }();
    allPlaced |= place;

    auto block = &unit.blocks[b];
    assertx(!block->code.empty());

    // Find the last index in the block where its not legal to insert the
    // ldimm. IE, avoid the special block ending instructions which we cannot
    // insert anything after.
    auto stopIdx = [&]{
      auto const valid = [] (Vinstr::Opcode op) {
        return op != Vinstr::unwind &&
               op != Vinstr::syncpoint &&
               op != Vinstr::nothrow &&
               op != Vinstr::fallthru;
      };
      auto i = block->code.size();
      while (i > 0 && !valid(block->code[i-1].op)) --i;
      return i;
    }();

    // Walk through this block, inserting ldimms immediately before a constant's
    // use for the constants we decided to materialize here.
    for (size_t i = 0; i < stopIdx; ++i) {
      if (place.none()) break;

      placeThisInst.reset();
      if (i == stopIdx - 1) {
        // We reached the end of the block. Any constants remaining are ones we
        // couldn't sink to all predecessors. Materialize them here at the end.
        placeThisInst |= place;
      } else {
        visitUses(
          unit,
          block->code[i],
          [&] (Vreg r) { if (place[r]) placeThisInst.add(r); }
        );
      }

      if (placeThisInst.none()) continue;

      auto const materialize = [&] (Vout& v, Vreg r) {
        auto const it = unit.regToConst.find(r);
        assertx(it != unit.regToConst.end());
        auto const& vconst = it->second;
        // TODO (T37584483): Undef constants can be dealt with better here.
        switch (vconst.kind) {
          case Vconst::Quad:
          case Vconst::Double:
            v << ldimmq{uint64_t(vconst.val), r};
            break;
          case Vconst::Long:
            v << ldimml{int32_t(vconst.val), r};
            break;
          case Vconst::Byte:
            v << ldimmb{uint8_t(vconst.val), r};
            break;
        }
        ++i;
        ++stopIdx;
      };

      vmodify(
        unit, b, i,
        [&] (Vout& v) {
          placeThisInst.forEach([&] (Vreg r) { materialize(v, r); });
          return 0;
        }
      );
      block = &unit.blocks[b];
      place -= placeThisInst;
    }

    assertx(place.none());
  }

  // Constant registers no longer exist!
  unit.regToConst.clear();
  unit.constToReg.clear();

  return allPlaced;
}

//////////////////////////////////////////////////////////////////////

/*
 * Pseudo conversion
 *
 * Vasm instructions may have RegSets or implicit register effects (which aren't
 * reflected in the instruction's operands). Its not possible to store arbitrary
 * Vregs in these, but we need to put the program into SSA form (which will
 * involve rewriting physical registers into virtual ones). So, convert all such
 * instructions into "pseudo" instructions, which serve solely to make their
 * uses/defs explicit with Vregs. These instructions can then be manipulated
 * just like any other instruction. We use the "pos" field as an index to their
 * original instruction, which lets us convert these pseudo instructions back
 * when we're done. We have a variety of pseudo instructions to represent the
 * distinct categories of instructions that we care about.
 */

// Gather the operands into lists to make for easy conversion
struct PseudoConvertVisitor {
  template <typename T> void imm(const T&) const {}

  void use(const Vptr& p) {
    if (p.base.isValid())  use(p.base);
    if (p.index.isValid()) use(p.index);
  }
  void use(const RegSet& s) { s.forEach([this](Vreg r) { use(r); }); }
  void use(Vreg r) { uses.emplace_back(r); }
  void use(Vtuple) { always_assert(false); }
  void use(VcallArgsId) { always_assert(false); }

  void use(Vreg64 r) { uses64.push_back(r); }
  void use(VregSF r) { assertx(!flagUse); flagUse = r; }
  template <typename W> void use(Vr<W> r) { always_assert(false); }

  void useHint(Vreg64 r1, Vreg64 r2) { uses64WithHints.emplace_back(r1, r2); }
  template <typename U> void useHint(Vtuple, const U&) {
    always_assert(false);
  }
  template <typename U> void useHint(Vreg, const U&) {
    always_assert(false);
  }
  template <typename W, typename U> void useHint(Vr<W>, const U&) {
    always_assert(false);
  }

  void defHint(Vreg64 r1, Vreg64 r2) { defs64WithHints.emplace_back(r1, r2); }
  template <typename U> void defHint(Vtuple, const U&) {
    always_assert(false);
  }
  template <typename U> void defHint(Vreg, const U&) {
    always_assert(false);
  }
  template <typename W, typename U> void defHint(Vr<W>, const U&) {
    always_assert(false);
  }

  template <typename T> void across(const T&) { always_assert(false); }

  void def(Vtuple) { always_assert(false); }
  void def(Vreg r) { defs.emplace_back(r); }

  void def(VregSF r) { assertx(!flagDef); flagDef = r; }
  template <typename W> void def(Vr<W>) { always_assert(false); }

  VregList uses;
  VregList uses64;
  VregList defs;
  VregList acrosses;
  jit::vector<std::pair<Vreg64, Vreg64>> uses64WithHints;
  jit::vector<std::pair<Vreg64, Vreg64>> defs64WithHints;
  folly::Optional<VregSF> flagUse;
  folly::Optional<VregSF> flagDef;
};

// Check if any operand is a RegSet
struct HasRegSetVisitor {
  template <typename T> void imm(const T&) const {}
  template <typename T> void use(const T& t) { check(t); }
  template <typename T> void def(const T& t) { check(t); }
  template <typename T> void across(const T& t) { check(t); }
  template <typename T, typename U> void useHint(const T& t, const U& u) {
    check(t); check(u);
  }
  template <typename T, typename U> void defHint(const T& t, const U& u) {
    check(t); check(u);
  }

  template <typename T> void check(const T&) const {}
  void check(const RegSet&) { hasRegSet = true; }

  bool hasRegSet = false;
};

void pseudo_convert(State& state) {
  auto& unit = state.unit;
  auto const& abi = state.abi;

  for (auto const label : state.rpo) {
    for (auto& inst : unit.blocks[label].code) {

      auto const visit = [&] {
        PseudoConvertVisitor v;
        visitOperands(inst, v);

        RegSet implicitUses, implicitAcross, implicitDefs;
        getEffects(abi, inst, implicitUses, implicitAcross, implicitDefs);
        implicitDefs.forEach([&](Vreg r) { v.defs.emplace_back(r); });
        implicitUses.forEach([&](Vreg r) { v.uses.emplace_back(r); });
        implicitAcross.forEach([&](Vreg r) { v.acrosses.emplace_back(r); });
        return v;
      };

      switch (inst.op) {
        case Vinstr::bindjmp:
        case Vinstr::callfaststub:
        case Vinstr::calltc:
        case Vinstr::cqo:
        case Vinstr::fallback:
        case Vinstr::fallthru:
        case Vinstr::jmpi:
        case Vinstr::jmpm:
        case Vinstr::jmpr:
        case Vinstr::leavetc:
        case Vinstr::phpret:
        case Vinstr::resumetc:
        case Vinstr::ret:
        case Vinstr::retransopt:
        case Vinstr::stubret:
        case Vinstr::tailcallphp:
        case Vinstr::tailcallstub: {
          assertx(succs(inst).size() == 0);

          auto v = visit();
          assertx(v.uses64WithHints.empty());
          assertx(v.defs64WithHints.empty());
          assertx(!v.flagUse);
          assertx(!v.flagDef);

          state.pseudos.emplace_back(inst);
          inst.pos = state.pseudos.size() - 1;

          inst.op = Vinstr::pseudojmp;
          inst.pseudojmp_ = pseudojmp{
            unit.makeTuple(std::move(v.defs)),
            unit.makeTuple(std::move(v.uses)),
            unit.makeTuple(std::move(v.uses64)),
            unit.makeTuple(std::move(v.acrosses))
          };
          continue;
        }
        case Vinstr::call:
        case Vinstr::callm:
        case Vinstr::callr:
        case Vinstr::calls:
        case Vinstr::callstub:
        case Vinstr::callunpack: {
          assertx(succs(inst).size() == 0);

          auto v = visit();
          assertx(v.uses64WithHints.empty());
          assertx(v.defs64WithHints.empty());
          assertx(!v.flagUse);

          state.pseudos.emplace_back(inst);
          inst.pos = state.pseudos.size() - 1;

          inst.op = Vinstr::pseudocall;
          inst.pseudocall_ = pseudocall{
            unit.makeTuple(std::move(v.defs)),
            unit.makeTuple(std::move(v.uses)),
            unit.makeTuple(std::move(v.uses64)),
            unit.makeTuple(std::move(v.acrosses))
          };
          continue;
        }
        case Vinstr::bindjcc:
        case Vinstr::fallbackcc: {
          assertx(succs(inst).size() == 0);

          auto v = visit();
          assertx(v.uses64.empty());
          assertx(v.uses64WithHints.empty());
          assertx(v.defs64WithHints.empty());
          assertx(v.flagUse);
          assertx(!v.flagDef);

          state.pseudos.emplace_back(inst);
          inst.pos = state.pseudos.size() - 1;

          inst.op = Vinstr::pseudojcc;
          inst.pseudojcc_ = pseudojcc{
            unit.makeTuple(std::move(v.defs)),
            unit.makeTuple(std::move(v.uses)),
            unit.makeTuple(std::move(v.acrosses)),
            *v.flagUse
          };
          continue;
        }
        case Vinstr::idiv: {
          assertx(succs(inst).size() == 0);

          auto v = visit();
          assertx(v.uses64WithHints.empty());
          assertx(v.defs64WithHints.empty());
          assertx(!v.flagUse);
          assertx(v.flagDef);

          state.pseudos.emplace_back(inst);
          inst.pos = state.pseudos.size() - 1;

          inst.op = Vinstr::pseudodiv;
          inst.pseudodiv_ = pseudodiv{
            unit.makeTuple(std::move(v.defs)),
            unit.makeTuple(std::move(v.uses)),
            unit.makeTuple(std::move(v.uses64)),
            unit.makeTuple(std::move(v.acrosses)),
            *v.flagDef
          };
          continue;
        }
        case Vinstr::callphp:
        case Vinstr::contenter: {
          auto v = visit();
          assertx(v.uses64WithHints.empty());
          assertx(v.defs64WithHints.empty());
          assertx(!v.flagUse);
          assertx(!v.flagDef);

          auto const succList = succs(inst);
          assertx(succList.size() == 2);
          Vlabel targets[2] = { succList[0], succList[1] };

          state.pseudos.emplace_back(inst);
          inst.pos = state.pseudos.size() - 1;

          inst.op = Vinstr::pseudocallphp;
          inst.pseudocallphp_ = pseudocallphp{
            unit.makeTuple(std::move(v.defs)),
            unit.makeTuple(std::move(v.uses)),
            unit.makeTuple(std::move(v.uses64)),
            unit.makeTuple(std::move(v.acrosses)),
            { targets[0], targets[1] }
          };
          continue;
        }
        case Vinstr::sarq:
        case Vinstr::shlq: {
          assertx(succs(inst).size() == 0);

          auto v = visit();
          assertx(v.uses64WithHints.size() == 1);
          assertx(v.defs64WithHints.size() == 1);
          assertx(v.uses64WithHints[0].second == v.defs64WithHints[0].first);
          assertx(v.defs64WithHints[0].second == v.uses64WithHints[0].first);
          assertx(!v.flagUse);
          assertx(v.flagDef);

          state.pseudos.emplace_back(inst);
          inst.pos = state.pseudos.size() - 1;

          inst.op = Vinstr::pseudoshift;
          inst.pseudoshift_ = pseudoshift{
            v.defs64WithHints[0].first,
            v.uses64WithHints[0].first,
            unit.makeTuple(std::move(v.defs)),
            unit.makeTuple(std::move(v.uses)),
            unit.makeTuple(std::move(v.acrosses)),
            *v.flagDef
          };
          continue;
        }
        default:
          break;
      }

      // We should have covered any instruction which requires conversion
      // already. Do a sanity check to make sure the remaining instructions
      // don't have implicit register effects or RegSets.
      if (debug) {
        RegSet uses, across, defs;
        getEffects(abi, inst, uses, across, defs);
        always_assert_flog(
          (defs | uses | across).empty(),
          "Instruction '{}' in {} with non-trivial effects "
          "not converted to pseudo",
          show(unit, inst),
          label
        );

        HasRegSetVisitor v;
        visitOperands(inst, v);
        always_assert_flog(
          !v.hasRegSet,
          "Instruction '{}' in {} with RegSet operand "
          "not converted to pseudo",
          show(unit, inst),
          label
        );
      }
    }
  }

  assertx(check(unit));
}

//////////////////////////////////////////////////////////////////////

void prepare_unit(State& state) {
  /*
   * The algorithm requires all registers to be in strict SSA form. Existing
   * Vregs should already be in SSA form, so only physical registers need to be
   * dealt with. First convert any instructions which have RegSets or implicit
   * effects into pseudo forms, turning them into Vregs. Then use the SSA
   * restoration pass to turn them into SSA form.
   */
  pseudo_convert(state);

  // Materialize constants (this has to be done after pseudo_convert because it
  // breaks SSA form).
  auto toSSA = place_constants(state);

  VregSet unreserved{state.gpUnreserved | state.simdUnreserved};
  // Ensure that the physical registers have a definition (required for the SSA
  // restoration algorithm).
  vmodify(
    state.unit, state.unit.entry, 0,
    [&] (Vout& v) {
      unreserved.forEach([&] (Vreg r) { v << conjure{r}; });
      return 0;
    }
  );
  toSSA |= unreserved;

  auto const mappings = restoreSSA(state.unit, toSSA, state.rpo);
  for (auto const& map : mappings) {
    if (!map.second.isPhys()) continue;
    // Remember what physical register these new Vregs were
    reg_info_create(state, map.first).precolor = map.second.physReg();
  }
}

//////////////////////////////////////////////////////////////////////
// Register class inference

/*
 * Vregs themselves don't inherently have a class. That is, they're not
 * inherently GP or SIMD. Instead the set of physical registers they can be
 * allocated depends solely on their usages and defs. The instructions put
 * constraints on their operands which is used to infer the proper register
 * class.
 *
 * We need to infer such information here. We start by assuming that all Vregs
 * are unconstrained (RegClass::Any). We then examine all uses and defs of that
 * Vreg, constraining the RegClass as appropriate. If the constraints are
 * incompatible (for example, RegClass::GP and RegClass::SIMD), we insert a copy
 * between the incompatible points. Copies never constrain their sources or
 * dests, and its assumed that a copy can deal with moving values between
 * register classes, so this is always safe.
 *
 * A register is wide if its source or dest is a Vreg128. Unlike other
 * constraints, wideness is propagated across copies (because otherwise the
 * source would be RegClass::SIMDWide, and the dest would be
 * RegClass::AnyNarrow, which is 64-bits, so we'd drop data). So a copy which
 * has a source or dest of RegClass::SIMDWide has the other operand as
 * RegClass::SIMDWide as well. Other than this wrinkle, the inference is flow
 * insensitive.
 *
 * RegClass::SF is meant to mark registers as being the flags register. We don't
 * attempt to allocate the flags register (there's only one), so it mainly
 * serves as a marker to ignore that register (its compatible with nothing).
 *
 * RegClass::AnyNarrow means the Vreg is unconstrained (but is not wide). In
 * theory this means we're completely free to assign it either a GP or SIMD
 * register. Unfortunately the graph coloring algorithm cannot deal with a
 * constraint like this. Every Vreg has to either be selected from GPs or SIMDs,
 * not both. Therefore RegClass::AnyNarrow is a synonym for RegClass::GP right
 * now. We can do better than that. TODO (T37587676)
 *
 * Finally, we insert copies to resolve a few tricky cases. We ensure that
 * there's no physical to virtual, or virtual to physical copies in copyargs by
 * copying the physical register to a virtual one either before or after. This
 * simplifies having to deal with such copies when lowering the copyargs. We
 * also enforce that phis have the same register class on the src/dst side (by
 * inserting copies) for similar reasons. We don't do this for copy2 because
 * unlike other copies it imposed a constraint.
 */

namespace detail {

// Turn Vreg constraint into RegClass generically
RegClass reg_class(Vreg)      { return RegClass::AnyNarrow; }
RegClass reg_class(Vreg64)    { return RegClass::GP; }
RegClass reg_class(Vreg32)    { return RegClass::GP; }
RegClass reg_class(Vreg16)    { return RegClass::GP; }
RegClass reg_class(Vreg8)     { return RegClass::GP; }
RegClass reg_class(VregDbl)   { return RegClass::SIMD; }
RegClass reg_class(Vreg128)   { return RegClass::SIMDWide; }
RegClass reg_class(VregSF)    { return RegClass::SF; }
RegClass reg_class(PhysReg r) {
  switch (r.type()) {
    case PhysReg::GP:   return RegClass::GP;
    case PhysReg::SIMD: return RegClass::SIMD;
    case PhysReg::SF:   return RegClass::SF;
  }
  always_assert(false);
}

}

void infer_register_classes(State& state) {
  auto& unit = state.unit;

  // Vregs which have incompatible uses/defs. Copies must be inserted to resolve
  // this.
  VregSet incompatible;

  // Returns the common RegClass between c1 and c2, or folly::none if none
  // exists.
  auto const merge = [] (RegClass c1,
                         RegClass c2) -> folly::Optional<RegClass> {
    // We shouldn't be mixing SF with anything
    assertx((c1 == RegClass::SF) == (c2 == RegClass::SF));
    if (c1 == RegClass::Any) return c2;
    if (c2 == RegClass::Any) return c1;
    if (c1 == c2) return c1;
    if (c1 == RegClass::AnyNarrow &&
        (c2 == RegClass::GP || c2 == RegClass::SIMD)) {
      return c2;
    }
    if (c2 == RegClass::AnyNarrow &&
        (c1 == RegClass::GP || c1 == RegClass::SIMD)) {
      return c1;
    }
    return folly::none;
  };

  auto haveWide = false;

  auto const updateDefs = [&] (auto r) {
    if (r.isPhys()) return;
    auto& info = reg_info_create(state, r);
    assertx(info.regClass == RegClass::Any);
    info.regClass = (info.precolor != InvalidReg)
      ? detail::reg_class(info.precolor)
      : detail::reg_class(r);
    haveWide |= (info.regClass == RegClass::SIMDWide);
  };

  auto const updateUses = [&] (auto r) {
    if (r.isPhys()) return;
    auto& info = reg_info_create(state, r);
    auto const newClass = (info.precolor != InvalidReg)
      ? detail::reg_class(info.precolor)
      : detail::reg_class(r);
    if (auto const c = merge(info.regClass, newClass)) {
      info.regClass = *c;
      haveWide |= (info.regClass == RegClass::SIMDWide);
    } else {
      incompatible.add(r);
    }
  };

  auto const processInst = [&] (const Vinstr& inst, bool skipDefs = false) {
    // Some of the pseudo-instructions use a VregList to store Vreg64
    // operands. Since this loses the type information, we need to special case
    // these.
    auto const uses64 = [&]{
      switch (inst.op) {
        case Vinstr::pseudojmp: return inst.pseudojmp_.uses64;
        case Vinstr::pseudocall: return inst.pseudocall_.uses64;
        case Vinstr::pseudodiv: return inst.pseudodiv_.uses64;
        case Vinstr::pseudocallphp: return inst.pseudocallphp_.uses64;
        default: return Vtuple{};
      }
    }();
    if (uses64.isValid()) {
      for (auto const r : unit.tuples[uses64]) updateUses(Vreg64{r});
    }
    visitUses(unit, inst, updateUses);
    if (!skipDefs) visitDefs(unit, inst, updateDefs);
  };

  // Insert copies as necessary to ensure that a copyargs instruction does not
  // copy between a virtual register and a physical one (which makes the spiller
  // easier).
  auto const canonicalizeCopyArgs = [&] (Vlabel b, size_t instIdx) {
    auto const& inst = unit.blocks[b].code[instIdx];
    if (inst.op != Vinstr::copyargs) return;

    // If the source is a physical register, we insert a copy before.
    jit::fast_map<Vreg, Vreg> precopy;
    // If the dest is a physical register, we insert a copy after.
    jit::fast_map<Vreg, Vreg> postcopy;

    auto& srcs = unit.tuples[inst.copyargs_.s];
    auto& dsts = unit.tuples[inst.copyargs_.d];
    assertx(srcs.size() == dsts.size());

    if (debug) {
      // Sanity check that the copyargs doesn't have any duplicate defs (even
      // physical registers, whose semantics would be unclear).
      VregSet defs;
      for (size_t i = 0; i < dsts.size(); ++i) {
        auto const d = dsts[i];
        always_assert(!defs[d]);
        defs.add(d);
      }
    }

    for (size_t i = 0; i < srcs.size(); ++i) {
      auto const s = srcs[i];
      auto const d = dsts[i];
      if (s.isPhys() == d.isPhys()) continue;
      if (s.isPhys()) {
        auto it = precopy.find(s);
        // The copyargs can take the same register multiple times as a source,
        // so re-use the same copied register for that.
        if (it == precopy.end()) it = precopy.emplace(s, unit.makeReg()).first;
        srcs[i] = it->second;
      }
      if (d.isPhys()) {
        // Dests should be unique, however.
        auto const result = postcopy.emplace(d, unit.makeReg());
        assertx(result.second);
        dsts[i] = result.first->second;
      }
    }

    // Insert the copies at the appropriate places.
    if (!precopy.empty()) {
      vmodify(
        unit, b, instIdx,
        [&] (Vout& v) {
          for (auto const& p : precopy) v << copy{p.first, p.second};
          return 0;
        }
      );
    }

    if (!postcopy.empty()) {
      vmodify(
        unit, b, instIdx + precopy.size() + 1,
        [&] (Vout& v) {
          for (auto const& p : postcopy) v << copy{p.second, p.first};
          return 0;
        }
      );
    }
  };

  // Emit a set of copies as either a single copy or a copyargs instruction. Its
  // guaranteed to only ever emit a single instruction.
  auto const addCopies = [&] (const jit::vector<std::pair<Vreg, Vreg>>& copies,
                              Vlabel b,
                              size_t i) {
    vmodify(
      unit, b, i,
      [&] (Vout& v) {
        if (copies.size() == 1) {
          v << copy{copies[0].first, copies[0].second};
        } else {
          VregList src;
          VregList dst;
          for (auto const& p : copies) {
            src.emplace_back(p.first);
            dst.emplace_back(p.second);
          }
          v << copyargs{
            unit.makeTuple(std::move(src)),
            unit.makeTuple(std::move(dst))
          };
        }
        return 0;
      }
    );
  };

  // First the flow insensitive part. Visit every instruction, using the uses
  // and defs to determine the register class.
  for (auto const b : state.rpo) {
    for (size_t i = 0; i < unit.blocks[b].code.size(); ++i) {
      // Deal with strange copyargs
      canonicalizeCopyArgs(b, i);
      // Analyze the use and defs. If there's no incompatibilities we're done.
      processInst(unit.blocks[b].code[i]);
      if (incompatible.none()) continue;

      // Otherwise we need to insert a copy before the use.
      jit::vector<std::pair<Vreg, Vreg>> copies;
      incompatible.forEach(
        [&] (Vreg r) {
          assertx(!r.isPhys());
          auto const newReg = unit.makeReg();
          // Clone the register, except for the RegClass, which is automatically
          // Any because its coming from a copy. We'll reprocess this
          // instruction to set the RegClass appropriately.
          auto& newInfo = reg_info_create(state, newReg);
          newInfo = reg_info(state, r);
          newInfo.regClass = RegClass::Any;
          copies.emplace_back(r, newReg);
        }
      );
      assertx(!copies.empty());
      addCopies(copies, b, i);
      // We've inserted a copy, so adjust the instruction index.
      ++i;

      // Rewrite the instruction to take the new (from copy) register.
      visitRegsMutable(
        unit, unit.blocks[b].code[i],
        [&] (Vreg r) {
          if (!incompatible[r]) return r;
          for (auto const& p : copies) {
            if (p.first == r) return p.second;
          }
          always_assert(false);
        },
        [&] (Vreg r) { return r; }
      );

      // Now process the copy and reprocess the instruction. This will set the
      // RegClass of the new (from copy) registers properly.
      incompatible.reset();
      processInst(unit.blocks[b].code[i-1]);
      processInst(unit.blocks[b].code[i], true);
      // Reprocessing shouldn't create new incompatibilities.
      assertx(incompatible.none());
    }
  }

  // Now for the flow sensitive part. If either side of a copy is a wide
  // register, the other side must be forced to be wide as well. Only wideness
  // is propagated across copies in this way. We only propagate wideness if the
  // other register is RegClass::AnyNarrow (unconstrained non-wide). If there's
  // an actual constraint, we want to honor that (it means the code is
  // attempting to move a 128-bit register to a 64-bit one or vice-versa). If we
  // didn't see any wide RegClass already, we can skip this. Since this is rare,
  // we simply loop to a fixed point rather than try a worklist.
  auto changed = haveWide;
  while (changed) {
    changed = false;

    auto const propagateWide = [&] (Vreg s, Vreg d) {
      if (s.isPhys() || d.isPhys()) return false;
      auto& sInfo = reg_info(state, s);
      auto& dInfo = reg_info(state, d);
      if (sInfo.regClass == RegClass::SIMDWide &&
          dInfo.regClass == RegClass::AnyNarrow) {
        dInfo.regClass = RegClass::SIMDWide;
        return true;
      } else if (sInfo.regClass == RegClass::AnyNarrow &&
                 dInfo.regClass == RegClass::SIMDWide) {
        sInfo.regClass = RegClass::SIMDWide;
        return true;
      }
      return false;
    };

    for (auto const b : state.rpo) {
      auto const& block = unit.blocks[b];
      for (auto const& inst : block.code) {
        if (inst.op == Vinstr::copy) {
          changed |= propagateWide(inst.copy_.s, inst.copy_.d);
        } else if (inst.op == Vinstr::copyargs) {
          auto const& s = unit.tuples[inst.copyargs_.s];
          auto const& d = unit.tuples[inst.copyargs_.d];
          assertx(s.size() == d.size());
          for (size_t i = 0; i < s.size(); ++i) {
            changed |= propagateWide(s[i], d[i]);
          }
        } else if (inst.op == Vinstr::phidef) {
          // Phis are treated as copies in this context
          auto const& d = unit.tuples[inst.phidef_.defs];
          for (auto const pred : state.preds[b]) {
            auto const& phijmp = unit.blocks[pred].code.back();
            assertx(phijmp.op == Vinstr::phijmp);
            auto const& s = unit.tuples[phijmp.phijmp_.uses];
            assertx(s.size() == d.size());
            for (size_t i = 0; i < s.size(); ++i) {
              changed |= propagateWide(s[i], d[i]);
            }
          }
        }
      }
    }
  }

  // Finally we have to ensure that the src/dest pairs of a phi have the same
  // register class. If not, we'll ensure they do by (yet again) inserting a
  // copy before the phi of the old src to a new Vreg which has the same
  // register class of the dest. The new Vreg will then be the src in the
  // Phi. We need to do all of this because having to deal with moves between
  // register classes during a parallel phi copy greatly complicates the
  // spilling logic and the phi lowering logic.
  for (auto const b : state.rpo) {
    auto const& block = unit.blocks[b];
    auto const& inst = block.code.back();
    if (inst.op != Vinstr::phijmp) continue;

    auto& s = unit.tuples[inst.phijmp_.uses];
    auto const successorList = succs(block);
    assertx(successorList.size() == 1);
    auto const& phidef = unit.blocks[successorList[0]].code.front();
    assertx(phidef.op == Vinstr::phidef);
    auto const& d = unit.tuples[phidef.phidef_.defs];
    assertx(s.size() == d.size());

    if (debug) {
      // Sanity check that the phi doesn't have duplicate (non-physical) defs.
      VregSet defs;
      for (size_t i = 0; i < d.size(); ++i) {
        if (d[i].isPhys()) continue;
        always_assert(!defs[d[i]]);
        defs.add(d[i]);
      }
    }

    jit::vector<std::pair<Vreg, Vreg>> copies;
    for (size_t i = 0; i < s.size(); ++i) {
      assertx(s[i].isPhys() == d[i].isPhys());
      if (s[i].isPhys()) continue;

      auto const& sInfo = reg_info(state, s[i]);
      auto const& dInfo = reg_info(state, d[i]);
      if (sInfo.regClass == dInfo.regClass) continue;

      // Create a new Vreg in the same RegClass as the dest and use that in the
      // phi instead. Add a copy between the old src and the new Vreg. This lets
      // the move between register classes be done outside of the phi, instead
      // of during it.
      auto const newReg = unit.makeReg();
      reg_info_insert(state, newReg, dInfo);

      copies.emplace_back(s[i], newReg);
      s[i] = newReg;
    }

    if (copies.empty()) continue;
    addCopies(copies, b, block.code.size() - 1);
  }
}

//////////////////////////////////////////////////////////////////////
// Spilling

/*
 * Materialization:
 *
 * When attempting to reload a spilled Vreg, we can instead rematerialize the
 * value in some cases. Right now we only attempt to materialize instructions
 * which have no source operands and are pure (this is basically the ldimm*
 * instructions). If we successfully rematerialize a Vreg, we should (hopefully)
 * have a spill with no associated reloads, which will be cleaned up by dead
 * code elimination.
 *
 * We'd like to expand this list, but we have to deal with issues about
 * detecting which sources are available at the point of the reload.
 */

// Check if the given instruction can possibly be rematerialized, returning the
// Vreg the instruction defines if successful (invalid Vreg otherwise).
Vreg is_materialization_candidate(const State& state, const Vinstr& inst) {
  if (inst.op == Vinstr::copy ||
      inst.op == Vinstr::copy2 ||
      inst.op == Vinstr::copyargs ||
      inst.op == Vinstr::phijmp ||
      inst.op == Vinstr::phidef) return Vreg{};

  // Not safe to rematerialize non-pure instructions
  if (!isPure(inst)) return Vreg{};

  // Can't rematerialize instructions which define more than one Vreg, or define
  // flags or physical registers.
  auto valid = true;
  Vreg def;
  visitDefs(
    state.unit, inst,
    [&] (Vreg r) {
      if (!valid) return;
      if (r.isPhys() ||
          def.isValid() ||
          reg_info(state, r).regClass == RegClass::SF) {
        valid = false;
        return;
      }
      def = r;
    }
  );

  if (!valid || !def.isValid()) return Vreg{};

  // Right now we don't allow any source Vregs.
  visitUses(
    state.unit, inst,
    [&] (Vreg r) {
      // TODO (TT37650327): Handle instructions with sources
      valid = false;
    }
  );

  return valid ? def : Vreg{};
}

// Comparators for instruction source types
namespace detail {

// Make these all types explicit (no default case) so that we get compile errors
// if we have a new source type.
bool src_cmp(const Vunit&, Vreg r1, Vreg r2)     { return r1 == r2; }
bool src_cmp(const Vunit&, Vptr r1, Vptr r2)     { return r1 == r2; }
bool src_cmp(const Vunit&, RegSet r1, RegSet r2) { return r1 == r2; }
bool src_cmp(const Vunit& unit, Vtuple t1, Vtuple t2) {
  return unit.tuples[t1] == unit.tuples[t2];
}
bool src_cmp(const Vunit& unit, VcallArgsId a1, VcallArgsId a2) {
  return unit.vcallArgs[a1] == unit.vcallArgs[a2];
}

}

// Check if two instructions are equal (modulo the def Vreg). Used to possibly
// merge together different rematerialization candidates at join points.
bool compare_insts_without_defs(const Vunit& unit,
                                const Vinstr& inst1,
                                const Vinstr& inst2) {
  if (inst1.op != inst2.op) return false;

  switch (inst1.op) {
#define O(name, imms, uses, ...)                       \
    case Vinstr::name: {                               \
      auto const& i1 = inst1.name##_; (void)i1;        \
      auto const& i2 = inst2.name##_; (void)i2;        \
      imms                                             \
      uses                                             \
      break;                                           \
    }
#define I(f)    if (i1.f != i2.f) return false;
#define U(s)    if (!detail::src_cmp(unit, i1.s, i2.s)) return false;
#define UA(s)   if (!detail::src_cmp(unit, i1.s, i2.s)) return false;
#define UH(s,h) if (!detail::src_cmp(unit, i1.s, i2.s)) return false;
#define UM(s)   if (!detail::src_cmp(unit, i1.s, i2.s)) return false;
#define UW(s)   if (!detail::src_cmp(unit, i1.s, i2.s)) return false;
#define Inone
#define Un
    VASM_OPCODES
#undef Un
#undef Inone
#undef UW
#undef UM
#undef UH
#undef UA
#undef U
#undef I
#undef O
  }

  return true;
}

// Find all Vregs which can potentially be rematerialized rather than reloaded
// if spilled.
void find_materialization_candidates(State& state) {
  auto const& unit = state.unit;

  // First iterate over every instruction and record if its a candidate or not.
  for (auto const b : state.rpo) {
    auto const& block = unit.blocks[b];
    for (auto const& inst : block.code) {
      auto const r = is_materialization_candidate(state, inst);
      if (r.isValid()) reg_info(state, r).remat = inst;
    }
  }

  // Now use dataflow to propagate this candidate information across copy
  // instructions. At a merge point we'll keep the information if the two
  // instructions are the same (same sources and immediates), but drop it if
  // they are not.

  auto const compare = [&] (const folly::Optional<Vinstr>& remat1,
                            const folly::Optional<Vinstr>& remat2) {
    if (!remat1) return !remat2;
    if (!remat2) return false;
    return compare_insts_without_defs(unit, *remat1, *remat2);
  };

  auto const updateDst = [&] (const folly::Optional<Vinstr>& remat, Vreg d) {
    assertx(!d.isPhys());
    auto& dInfo = reg_info(state, d);
    if (!compare(remat, dInfo.remat)) {
      assertx(!!remat != !!dInfo.remat);
      dInfo.remat = remat;
      return true;
    } else {
      return false;
    }
  };

  // Propagate information across the copy
  auto const srcToDst = [&] (Vreg s, Vreg d) {
    if (s.isPhys() || d.isPhys()) return false;
    auto const& sInfo = reg_info(state, s);
    return updateDst(sInfo.remat, d);
  };

  boost::dynamic_bitset<> visited(unit.blocks.size());

  auto changed = true;
  while (changed) {
    changed = false;

    for (auto const b : state.rpo) {
      auto const& block = unit.blocks[b];
      for (auto const& inst : block.code) {
        switch (inst.op) {
          case Vinstr::copy:
            changed |= srcToDst(inst.copy_.s, inst.copy_.d);
            break;
          case Vinstr::copy2:
            changed |= srcToDst(inst.copy2_.s0, inst.copy2_.d0);
            changed |= srcToDst(inst.copy2_.s1, inst.copy2_.d1);
            break;
          case Vinstr::copyargs: {
            auto const& s = unit.tuples[inst.copyargs_.s];
            auto const& d = unit.tuples[inst.copyargs_.d];
            assertx(s.size() == d.size());
            for (size_t i = 0; i < s.size(); ++i) {
              changed |= srcToDst(s[i], d[i]);
            }
            break;
          }
          case Vinstr::phidef: {
            // Phi is like a copy, except we might have to merge together
            // instructions.
            auto const& defs = unit.tuples[inst.phidef_.defs];
            for (size_t i = 0; i < defs.size(); ++i) {
              folly::Optional<Vinstr> remat;
              auto first = true;
              for (auto const& pred : state.preds[b]) {
                if (!visited[pred]) continue;

                auto const& phijmp = unit.blocks[pred].code.back();
                assertx(phijmp.op == Vinstr::phijmp);
                auto const& uses = unit.tuples[phijmp.phijmp_.uses];
                assertx(defs.size() == uses.size());

                if (defs[i].isPhys() || uses[i].isPhys()) {
                  remat.clear();
                  break;
                }

                auto const& info = reg_info(state, uses[i]);
                if (first) {
                  remat = info.remat;
                  first = false;
                } else if (!compare(remat, info.remat)) {
                  remat.clear();
                  break;
                }
              }

              if (!defs[i].isPhys()) changed |= updateDst(remat, defs[i]);
            }

            break;
          }
          default:
            break;
        }
      }

      visited[b] = true;
    }
  }
}

/*
 * Spill Weight:
 *
 * The spill weight is what drives the spilling heuristic. Every Vreg at every
 * point has a spill weight. When we need to spill a Vreg, the spill weights of
 * all (eligible) Vregs are sorted and the one with the largest spill weight is
 * chosen. Therefore we want higher spill weights to reflect a Vreg that is
 * more profitable to spill.
 *
 * The heuristic we use is based on farthest usage from the current
 * location. That is, we want to spill the Vreg which is used the furthest away
 * (in terms of instructions). If a Vreg has different use distances at a spill,
 * we take the minimum (reflecting the conservative possibility). In addition,
 * we take into account edges which are exiting loops to add a higher spill
 * weight. This makes the heuristic more likely to select Vregs which are live
 * across the loop (but not used within them).
 *
 * For the purposes of distance calculation, we ignore copy instructions. We
 * insert copy instructions quite liberally, and we do not want that to affect
 * the spilling heuristic (since most of them are inserted by the register
 * allocator and don't reflect the original program). Instead copy instructions
 * just propagate the weight of their destination(s) to their source(s).
 *
 * We calculate the spill weights at block entry/exit, and weights within a
 * block are calculate on-demand.
 *
 * We should use profiling information to refine this heuristic: TODO
 * (T37650402).
 */

// Heuristic constants:

// The base penalty assigned to spill weight for a Vreg live-out of a loop.
static constexpr size_t kSpillWeightLoopBasePenalty = 1000000;
// For each depth the loop is nested, this "relief" will be removed from the
// base penalty (inner loops will be penalized less than outer loops).
static constexpr size_t kSpillWeightLoopRelief = 10000;
// Cap the max loop depth to avoid numerical issues. The max loop depth
// multiplied by the relief should not cause the penalty to become 0.
static constexpr size_t kMaxSpillWeightLoopDepth = 10;

// Impossibly large weight for a Vreg that is dead. This only has to be larger
// than any realistic unit. We don't use the maximum possible integer because
// then we can do arithmetic on it like any other number without worrying about
// overflow.
static constexpr size_t kSpillWeightInfinity = 5000000;

// Return the spill weight of a Vreg at a position within a block.
uint64_t spill_weight_at(const State& state,
                         Vreg reg,
                         Vlabel b,
                         size_t instIdx) {
  assertx(b < state.unit.blocks.size());
  assertx(instIdx <= state.unit.blocks[b].code.size());

  assertx(!reg.isPhys());
  assertx(reg_info(state, reg).regClass != RegClass::SF);

  // Keep an equivalence set of Vregs we care about. If any Vreg in this set is
  // involved in a copy, it is added to the set.
  VregSet regs{reg};
  uint64_t weight = kSpillWeightInfinity; // TODO (T37650402): Be smarter about
                                          // these numbers
  size_t instCount = 0;

  // Walk forward from the specified starting point, looking for any usages of a
  // Vreg in the equivalence set. If we find one, the weight is just the
  // distance from the starting point to the current instruction (not counting
  // copies). If the Vreg is involved in a copy, add it to the equivalence set.
  auto const& unit = state.unit;
  auto const& block = unit.blocks[b];
  for (size_t i = instIdx; i < block.code.size(); ++i) {
    auto const& inst = block.code[i];
    if (inst.op == Vinstr::copy) {
      if (regs[inst.copy_.s] && !inst.copy_.d.isPhys()) regs.add(inst.copy_.d);
    } else if (inst.op == Vinstr::copyargs) {
      auto const& s = unit.tuples[inst.copyargs_.s];
      auto const& d = unit.tuples[inst.copyargs_.d];
      assertx(s.size() == d.size());
      for (size_t j = 0; j < s.size(); ++j) {
        if (regs[s[j]] && !d[j].isPhys()) regs.add(d[j]);
      }
    } else if (inst.op == Vinstr::phijmp) {
      auto const& s = unit.tuples[inst.phijmp_.uses];
      auto const& weights = state.spillPhiWeights[inst.phijmp_.target];
      assertx(s.size() == weights.size());
      for (size_t j = 0; j < s.size(); ++j) {
        // Multiple Vregs in the phi can be in the equivalence set, so take the
        // minimum.
        if (!regs[s[j]] || !weights[j]) continue;
        weight = std::min(weight, *weights[j]);
      }
    } else {
      auto found = false;
      visitUses(
        unit, inst,
        [&] (Vreg r) { if (regs[r]) found = true; }
      );
      if (found) return instCount;
      ++instCount;
    }
  }

  // If we get here, the Vreg isn't used from the start point to the end of the
  // block. We can just use the weight at the block exit. Check all registers in
  // the equivalence set and pick the minimum one.
  regs.forEach(
    [&] (Vreg r) {
      auto const it = state.spillWeightsOut[b].find(r);
      if (it != state.spillWeightsOut[b].end()) {
        weight = std::min(weight, it->second);
      }
    }
  );
  return weight + instCount;
}

// Calculate the spill weights at block entry/exit.
void calculate_spill_weights(State& state) {
  auto const& unit = state.unit;

  state.spillWeightsIn.resize(unit.blocks.size());
  state.spillWeightsOut.resize(unit.blocks.size());
  state.spillPhiWeights.resize(unit.blocks.size());

  dataflow_worklist<size_t, std::less<size_t>> worklist(state.rpo.size());
  for (size_t i = 0; i < state.rpo.size(); ++i) worklist.push(i);

  while (!worklist.empty()) {
    auto const b = state.rpo[worklist.pop()];
    auto const& block = unit.blocks[b];

    // What is the loop depth of this block?
    auto const predDepth =
      std::max(block_loop_depth(state, b), kMaxSpillWeightLoopDepth);

    auto& out = state.spillWeightsOut[b];
    out.clear();
    for (auto const s : succs(block)) {
      // What is the loop depth of this successor?
      auto const succDepth =
        std::max(block_loop_depth(state, s), kMaxSpillWeightLoopDepth);

      // If the successor has a lower loop depth than the current block (which
      // means that we're exiting one or more loops), assess a penalty. Note
      // that is considered a "penalty" because higher weights are more likely
      // to be spilled. If a Vreg is live across this edge, it means that its
      // live-out of the loop, and we want to make those Vregs more likely to be
      // spilled (as opposed to Vregs which are only live within the loop). The
      // penalty is higher the less we're nesting loops. If the Vreg is actually
      // used in the block, its weight will be adjusted as part of the transfer
      // function below.
      //
      // TODO (T37650402): Incorporate profiling information
      auto const penalty = (succDepth < predDepth)
        ? (kSpillWeightLoopBasePenalty - succDepth * kSpillWeightLoopRelief)
        : 0;

      // The out spill weight for a Vreg is the minimum of the in spill weights
      // of all successors (once the penalty is assessed).
      for (auto const& p : state.spillWeightsIn[s]) {
        auto it = out.find(p.first);
        if (it == out.end()) {
          out.emplace(p.first, p.second + penalty);
        } else {
          it->second = std::min(it->second, p.second + penalty);
        }
      }
    }

    // Now that we have the out spill weights, analyze the instructions of the
    // block to find the in spill weights. Since the spill weights are the
    // minimum distance to the next use, if the Vreg is used within the block,
    // it will have a lesser spill weight.
    auto transfer = out;
    size_t instCount = 0;

    // As a trick, when we record a weight, we record the number of instructions
    // from the end of the block, negated. When we're done, we increment all the
    // weights in the map by the total number of instructions in the block,
    // which converts everything to the number of instructions from the
    // beginning of the block.

    // Assign the weight for a given Vreg. If there's already a weight for it,
    // the minimum of the new and existing value is used.
    auto const propagateWeight = [&] (uint64_t w, Vreg d) {
      if (d.isPhys()) return;
      assertx(reg_info(state, d).regClass != RegClass::SF);
      auto const it = transfer.find(d);
      if (it == transfer.end()) {
        transfer.emplace(d, w);
      } else {
        it->second = std::min(
          it->second + instCount,
          w + instCount
        ) - instCount;
      }
    };

    // Propagate weights from a copy source to destination. Copies do not count
    // as uses or defs, but just transfer the weight. NB: dest and src are
    // reversed here because we're flowing backwards.
    auto const propagateAcrossCopy = [&] (Vreg d, Vreg s) {
      if (d.isPhys()) return propagateWeight(-instCount, s);
      assertx(reg_info(state, d).regClass != RegClass::SF);
      auto const it = transfer.find(d);
      if (it == transfer.end()) return;
      propagateWeight(it->second, s);
    };

    // Iterate over the instructions (backwards since we flow from uses to defs)
    // and for every use, update the weight. Note that we only update the
    // instruction count for non-copies. We don't want copies to affect the
    // spill weight because we can insert an arbitrary amount of them for
    // various reasons and they should not affect the spill heuristics.
    for (auto const& inst : boost::adaptors::reverse(block.code)) {
      if (inst.op == Vinstr::copy) {
        // Simple copy, just propagate the weights
        propagateAcrossCopy(inst.copy_.d, inst.copy_.s);
        transfer.erase(inst.copy_.d);
      } else if (inst.op == Vinstr::copyargs) {
        // Same as copy above, but done for multiple pairs
        auto const& s = unit.tuples[inst.copyargs_.s];
        auto const& d = unit.tuples[inst.copyargs_.d];
        assertx(s.size() == d.size());
        for (size_t i = 0; i < s.size(); ++i) {
          propagateAcrossCopy(d[i], s[i]);
          transfer.erase(d[i]);
        }
      } else if (inst.op == Vinstr::phidef) {
        // Phis have to be treated differently. We want to propagate the weight
        // of the dsts to all of the srcs in each predecessor. We use a spearate
        // phi weight vector (which is keyed by the phi index) to propagate the
        // weights.
        PhiWeightVector p;

        for (auto const d : unit.tuples[inst.phidef_.defs]) {
          if (d.isPhys()) {
            // We can't spill physical registers, so they always have a weight
            // of zero.
            p.emplace_back(0);
            continue;
          }
          assertx(reg_info(state, d).regClass != RegClass::SF);

          // If the dst has a weight, store it in the weight vector
          auto const it = transfer.find(d);
          p.emplace_back(
            it == transfer.end()
              ? folly::none
              : folly::make_optional(it->second + instCount)
          );
          transfer.erase(d);
        }

        // Reschedule all the predecessors and update the phi weights if it
        // changed.
        if (p != state.spillPhiWeights[b]) {
          for (auto const pred : state.preds[b]) {
            worklist.push(state.rpoOrder[pred]);
          }
          state.spillPhiWeights[b] = std::move(p);
        }
      } else if (inst.op == Vinstr::phijmp) {
        // For the jmp side of the phi, loads the values out of the stored phi
        // weights and propagate them to the srcs.
        auto const& s = unit.tuples[inst.phijmp_.uses];
        auto const& d = state.spillPhiWeights[inst.phijmp_.target];
        if (d.empty()) continue;
        assertx(s.size() == d.size());
        for (size_t i = 0; i < s.size(); ++i) {
          if (!d[i]) continue;
          propagateWeight(*d[i], s[i]);
        }
      } else {
        ++instCount;
        visitDefs(
          unit, inst,
          [&](Vreg r) {
            if (r.isPhys()) return;
            if (reg_info(state, r).regClass == RegClass::SF) return;
            // This Vreg is defined here so it has no weight before it.
            transfer.erase(r);
          }
        );
        visitUses(
          unit, inst,
          [&](Vreg r) {
            if (r.isPhys()) return;
            if (reg_info(state, r).regClass == RegClass::SF) return;
            // This Vreg is used here, so its weight at this block is its
            // distance from the front of the block.
            transfer.insert_or_assign(r, -instCount);
          }
        );
      }
    }
    // Update all the weights so that everything is in term of distance from the
    // beginning of the block. Note that we use instCount instead of the size of
    // the block to avoid counting copy instructions.
    for (auto& p : transfer) p.second += instCount;

    // Reschedule the predecessors and update if the weights changed.
    if (transfer != state.spillWeightsIn[b]) {
      for (auto const pred : state.preds[b]) {
        worklist.push(state.rpoOrder[pred]);
      }
      state.spillWeightsIn[b] = std::move(transfer);
    }
  }
}

void insert_spills(State& state) {
  find_materialization_candidates(state);
  calculate_liveness(state);
  calculate_loop_info(state);
  calculate_spill_weights(state);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void allocateRegistersWithGraphColor(Vunit& unit, const Abi& abi) {
  Timer timer(Timer::vasm_reg_alloc, unit.log_entry);
  splitCriticalEdges(unit);
  assertx(check(unit));

  auto state = make_state(unit, abi);
  SCOPE_ASSERT_DETAIL("Graph color state") { return show(state); };

  prepare_unit(state);
  infer_register_classes(state);
  insert_spills(state);

  printUnit(kVasmRegAllocLevel, "after vasm-graph-color", unit);

  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

}}
