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

#include "hphp/util/copy-ptr.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/match.h"

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

// Color is a discriminated union representing an unassigned color, a spill
// slot, or a physical register.  Which subset is valid depends on the Vreg's
// RegClass.
struct None {};
struct SpillSlot { size_t slot; };
struct SpillSlotWide { size_t slot; };
using Color = boost::variant<None, PhysReg, SpillSlot, SpillSlotWide>;

// Represents a position in the unit for def/use chains.
struct Position {
  Vlabel block;
  size_t index; // The index of the instruction in the block, multiplied by
                // two. Uses get even numbers, and acrosses and defs get
                // odd. This lets us say a use dominates a def within the same
                // instruction.
  bool operator==(const Position& o) const {
    return std::tie(block, index) == std::tie(o.block, o.index);
  }
};
using PositionVector = jit::vector<Position>;

// State about each Vreg. Instead of separate data-structures, all Vreg
// information is concentrated in this one data-structure.
struct RegInfo {
  RegClass regClass = RegClass::Any;
  // The physical register this was before it was SSA-ized. The allocator will
  // ensure that this Vreg will be assigned the same physical register at all
  // uses and defs (except copies).
  PhysReg precolor = InvalidReg;
  // Color assigned to this Vreg
  Color color;
  // Can this Vreg be potentially rematerialized (instead of reloaded) by this
  // instruction?
  folly::Optional<Vinstr> remat;
  // Where this Vreg is defined (only valid during color optimization).
  Position def;
  // List of uses of this Vreg (only valid during color optimization).
  PositionVector uses;
  // Optional set of interference neighbors. Calculated lazily.
  folly::Optional<VregSet> neighbors;
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

  // The number of non-wide and wide spill slots allocated. This determines how
  // much space to reserve in the stack.
  size_t numSpillSlots = 0;
  size_t numWideSpillSlots = 0;
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

std::string show(Color c) {
  return match<std::string>(
    c,
    [] (None)            { return "-"; },
    [] (PhysReg r)       { return show(r); },
    [] (SpillSlot s)     { return folly::sformat("S{}", s.slot); },
    [] (SpillSlotWide s) { return folly::sformat("SW{}", s.slot); }
  );
}

std::string show(const Position& p) {
  return p.block.isValid()
    ? folly::sformat("({},{})", p.block, p.index)
    : "-";
}

std::string show(const PositionVector& v) {
  using namespace folly::gen;
  return folly::sformat(
    "[{}]",
    from(v)
      | map([] (const Position& p) { return show(p); })
      | unsplit<std::string>(", ")
  );
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
  auto const color = [&]{
    if (info.precolor != InvalidReg) {
      return folly::sformat("{} ({})", show(info.color), show(info.precolor));
    }
    return show(info.color);
  }();
  return folly::sformat(
    "Class: {:10}, Color: {:15}, Def: {:10}, Mat: ({}), Uses: {}",
    show(info.regClass),
    color,
    show(info.def),
    info.remat ? show(unit, *info.remat) : "-",
    show(info.uses)
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
    "Num Spill Slots:      {}\n"
    "Num Wide Spill Slots: {}\n"
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
    state.numSpillSlots,
    state.numWideSpillSlots,
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

bool is_spill(RegClass cls) {
  return cls == RegClass::Spill || cls == RegClass::SpillWide;
}

bool is_colorable(RegClass cls) {
  switch (cls) {
    case RegClass::AnyNarrow:
    case RegClass::GP:
    case RegClass::SIMD:
    case RegClass::SIMDWide:
      return true;
    case RegClass::SF:
    case RegClass::Spill:
    case RegClass::SpillWide:
      return false;
    case RegClass::Any:
      break;
  }
  always_assert(false);
}

// Compatible in the sense they map to the same set of physical registers.
bool compatible_reg_classes(RegClass cls1, RegClass cls2) {
  switch (cls1) {
    case RegClass::AnyNarrow:
    case RegClass::GP:
      return cls2 == RegClass::AnyNarrow || cls2 == RegClass::GP;
    case RegClass::SIMD:
    case RegClass::SIMDWide:
      return cls2 == RegClass::SIMD || cls2 == RegClass::SIMDWide;
    case RegClass::SF:
    case RegClass::Spill:
    case RegClass::SpillWide:
      return cls1 == cls2;
    case RegClass::Any:
      break;
  }
  always_assert(false);
}

// Wrappers around boost::get<>. Will assert if you try to retrieve a value from
// the color that isn't present (so check before calling).
bool is_color_none(Color c) { return boost::get<None>(&c); }

PhysReg color_reg(Color c) {
  auto const r = boost::get<PhysReg>(&c);
  assertx(r);
  return *r;
}

SpillSlot color_spill_slot(Color c) {
  auto const s = boost::get<SpillSlot>(&c);
  assertx(s);
  return *s;
}

SpillSlotWide color_spill_slot_wide(Color c) {
  auto const s = boost::get<SpillSlotWide>(&c);
  assertx(s);
  return *s;
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

// Return true if the given Vreg is live in (that is, before the instruction
// executes) at the instruction given by the block and index. This merely looks
// for a usage of that Vreg in the future, so it may return true if you give it
// a Vreg which isn't yet defined.
bool live_in_at(const State& state, Vreg reg, Vlabel b, size_t i) {
  assertx(b < state.unit.blocks.size());
  assertx(i <= state.unit.blocks[b].code.size());

  // If the position is the beginning of the block, we can just use the
  // pre-calculated liveness information.
  if (i == 0) return state.liveIn[b][reg];

  // Otherwise walk through the block from the specified position and look for a
  // use of that Vreg. If we find one, it must be live (assuming its defined at
  // the specified position).
  auto const& unit = state.unit;
  auto const& block = unit.blocks[b];
  for (; i < block.code.size(); ++i) {
    auto const& inst = block.code[i];
    auto found = false;
    visitUses(
      unit, inst,
      [&](Vreg r) { if (r == reg) found = true; }
    );
    if (found) return true;
  }
  // If we reach the end of the block without finding a usage, it may still be
  // used in a successor, so use the pre-calculated live-out information.
  return state.liveOut[b][reg];
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

  assertx(state.liveIn[state.unit.entry].none());
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

// A block is a loop header if we have loop info for it (since loops are keyed
// by their header).
bool is_loop_header(const State& state, Vlabel b) {
  return state.loopInfo.find(b) != state.loopInfo.end();
}

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

// Populate the def and uses fields of the Vreg info.
void record_defs_and_uses(State& state) {
  auto const& unit = state.unit;

  for (auto const b : state.rpo) {
    auto const& code = unit.blocks[b].code;
    for (size_t i = 0; i < code.size(); ++i) {
      auto const& inst = code[i];

      auto const acrosses = acrossesSet(unit, inst);
      visitUses(
        unit, inst,
        [&] (Vreg r) {
          if (r.isPhys()) return;
          // Uses get an even index
          Position position{b, i*2};
          // Except for acrosses
          if (acrosses[r]) ++position.index;
          auto& uses = reg_info(state, r).uses;
          // Don't record duplicate positions for uses
          if (!uses.empty() && uses.back() == position) return;
          uses.emplace_back(position);
        }
      );

      visitDefs(
        unit, inst,
        [&] (Vreg r) {
          if (r.isPhys()) return;
          auto& def = reg_info(state, r).def;
          assertx(!def.block.isValid());
          // Defs get an odd index
          def = Position{b, i*2 + 1};
        }
      );
    }
  }
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
 * Spilling is by far the most complicated part of the register allocator. It is
 * responsible for inserting Vinstr::spill and Vinstr::reload instructions to
 * move Vregs to/from spill slots in memory. Non-copy instructions have to have
 * their inputs in registers (not in memory) at the point they're executed, but
 * we can never have more Vregs in registers than there are actually physical
 * registers.
 *
 * Once spilling is accomplished then the unit is guaranteed to be trivially
 * colorable. Its guaranteed that at all points in the unit there's never more
 * Vregs in registers than there are physical registers, and therefore there'll
 * always be a free physical register to select (ignoring constrained
 * instructions, see below). This property is completely independent of the
 * colors you choose and thus means the actual color selection is arbitrary.
 *
 * The key behind this algorithm is that in a strict SSA program, the number of
 * needed physical registers at any point is equal to the number of live values
 * (Vregs here). This is just a restatement of the property that the chromatic
 * number of chordal interference graphs is equal to the largest clique. Each
 * clique of size N in the interference graph represents a point where N values
 * are simultaneously alive.
 *
 * Since we have a fixed number of physical registers, it follows that the unit
 * can only be colored if we can reduce (at every point) the number of live
 * Vregs to below that number. Of course we cannot make Vregs be not live, since
 * that affects program semantics, but we can spill them, which means they don't
 * occupy a register. This logic does *not* apply for non-SSA programs.
 *
 * The spilling is accomplished by walking the unit block by block. We keep
 * per-block state of which Vregs are in registers (not-spilled), and which are
 * in memory (spilled). At the entry of each block we first calculate the
 * initial state. That is, we decide which live-in Vregs should be in registers
 * and which in memory initially. For loop headers we apply extra logic to avoid
 * spilling Vregs used within the loop. Every block is free to make its own
 * independent decision in this regard. It can lead to blocks disagreeing about
 * where a Vreg should be, which will be fixed up afterwards.
 *
 * Once the block state is initialized, the block is walked instruction by
 * instruction. We force all the Vregs an instruction uses out of memory (if
 * necessary), generating reload instructions. This might increase the number of
 * Vregs in registers past the limit, so it may trigger spilling other Vregs
 * into memory, restoring the balance (generating spill instructions). We then
 * purge any uses which are now dead out of the state. We then record the defs
 * of the instruction as being in registers (since the instruction has to write
 * to registers). This may generate yet more spills. Finally we purge any defs
 * which are now dead (because they're unused). The decision of which Vregs to
 * actually spill is performed by a heuristic which uses a calculated "spill
 * weight" for each Vreg.
 *
 * This process is repeated for every instruction, generating reload and spill
 * instructions along the way. When we reach the end of the block, we record the
 * out state. If the block ends in a phijmp, we also record the state of the
 * Vregs used by the phi.
 *
 * Copies get special treatment. Copy instructions are special in that they can
 * handle both spilled and non-spilled Vregs (no reload required). This is
 * actually required because we need to be able to copy spilled Vregs around
 * without adding new register pressure. We do, however, require that the source
 * and dest of the copy agree with regards to whether the Vreg is spilled or
 * not. We may have to introduce extra instructions to guarantee this.
 *
 * Once this is all done, each block is self-consistent, but the blocks may not
 * agree amongst themselves about which Vregs are spilled or not. IE, one block
 * may have decided that it would assume that a live-in Vreg is in a register,
 * but its predecessor may have decided to spill this. We resolve this by
 * inserting spills or reloads at the end of the predecessor as necessary. A
 * similar situation may happen with phijmp/phidef pairs.
 *
 * The actual spill and reload instructions are kept until after coloring and
 * will be removed as part of SSA lowering.
 */

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

/*
 * Constrained instructions:
 *
 * Certain instructions require their sources or dests to be assigned particular
 * physical registers (this is why we have pseudo instructions). Such
 * instructions are called "constrained". A constrained instruction is any
 * instruction whose source or dest Vreg(s) has a precolor (IE, it was
 * originally a physical register). The exception is certain copy instructions
 * because (1) there's never a semantic requirement to have a physical register
 * there, and (2) we need to be able to insert those instructions to keep the
 * program colorable even with constraints.
 *
 * Unfortunately constrained instructions add additional structure to the
 * coloring problem which means the program may not be colorable even after the
 * register pressure has been lowered at all points. Once spilling has been
 * performed, the program should be able to be successfully colored with any
 * arbitrary selection of colors (IE, no matter what color you choose, you can
 * always color the remaining program). This is *not* true if we have
 * constrained instructions because you might choose a color for an instruction
 * which will be required by a constrained instruction later on.
 *
 * However (just for this specific problem) the program can be modified to
 * restore the trivial colorability property. Before any constrained instruction
 * we split the live ranges of all Vregs in registers at that point. This is
 * done by inserting a copyargs instruction where all the live Vregs are the
 * sources (with matching dests). We'll restore SSA afterwards which will
 * rewrite the copyargs to have new dests (and fix all downstream uses). This
 * copyargs breaks the live range of all Vregs, which means we always have the
 * option at that point to shuffle the registers as needed during coloring. This
 * ensures that we'll always be able to satisfy the constraint(s), no matter
 * what coloring choices have already been made. If there's no need to shuffle,
 * the copy will be optimized away.
 *
 * Constrained instructions introduce one other problem, this time during
 * spilling and modelling register pressure. It's best illustrated by an
 * example.
 *
 * Consider the instruction "Op R1 -> R2" where R1 and R2 are not precolored,
 * and R1 is not used after Op. This instruction clearly only requires one
 * physical register because R1 is dead after Op, and therefore R2 can always
 * use the same register that R1 had. However, suppose that R1 was precolored to
 * %eax, and R2 was precolored to %ecx. The instruction actually requires two
 * physical registers because we no longer have the freedom to assign R1 and R2
 * the same physical register. Sebastian Hack calls such instructions "register
 * pressure unfaithful".
 *
 * This is problematic for the spiller because it does not attempt to assign
 * colors to Vregs. It only makes sure the register pressure never exceeds the
 * number of physical registers. However, it can be dealt with (conservatively)
 * via a heuristic. If an instruction is constrained, we try to match up
 * precolored sources with same precolored dests. If any precolored sources
 * remain after this, we try to match them with non-precolored dests. If any
 * precolored sources remain after this, and those sources are live-out of the
 * instruction, we treat them as "across" instead of "use". This ensures they
 * are treated as live at the same time as the defs and therefore we model the
 * register pressure appropriately. This is a heuristic, but its safe and this
 * situation almost never happens. Using the above example, we'd fail to match
 * up R1 and R2 because they have different precolors, so R1 would be treated as
 * "across" and the spiller would model the instruction as needing at least two
 * free physical registers.
 */

bool is_constrained_inst(const State& state, const Vinstr& inst) {
  // These are never constrained (we need to be able to use them to resolve
  // constraints).
  switch (inst.op) {
    case Vinstr::copy:
    case Vinstr::copyargs:
    case Vinstr::phijmp:
    case Vinstr::phidef:
    case Vinstr::spill:
    case Vinstr::reload:
      return false;
    default:
      break;
  }

  // Otherwise an instruction is constrained if any of its uses or defs is
  // precolored.
  auto hasPrecolor = false;
  auto const process = [&] (Vreg r) {
    if (hasPrecolor || r.isPhys()) return;
    auto const& info = reg_info(state, r);
    if (info.precolor != InvalidReg) hasPrecolor = true;
  };

  visitUses(state.unit, inst, process);
  if (!hasPrecolor) visitDefs(state.unit, inst, process);
  return hasPrecolor;
}

// Adjust the use and across sets of the instruction at the given instruction to
// account for "register pressure unfaithfulness" as described above. If we
// cannot match up a pre-colored Vreg with a pre-colored Vreg in the defs set,
// move it from the use set to the across set.
void fix_constrained_inst_uses(const State& state,
                               const VregSet& defs,
                               VregSet& uses,
                               VregSet& acrosses,
                               Vlabel block,
                               size_t instIdx) {
  assertx(is_constrained_inst(state, state.unit.blocks[block].code[instIdx]));

  if (defs.none()) return;

  auto const compat_class = [] (RegClass c1, RegClass c2) {
    assertx(c1 != RegClass::Any);
    assertx(c2 != RegClass::Any);
    if (c1 == c2) return true;
    if (c1 == RegClass::AnyNarrow) return (c2 == RegClass::GP);
    if (c2 == RegClass::AnyNarrow) return (c1 == RegClass::GP);
    return false;
  };

  VregSet matchedUse;
  VregSet matchedDef;

  // If the precolored use can be paired up with a same precolored def (which
  // hasn't already been paired), return the def.
  auto const findMatchingPrecoloredDef = [&] (Vreg use) {
    assertx(!matchedUse[use]);
    auto const& useInfo = reg_info(state, use);
    assertx(useInfo.precolor != InvalidReg);

    Vreg match;
    defs.forEach(
      [&] (Vreg def) {
        assertx(!match.isValid());
        auto const& defInfo = reg_info(state, def);
        if (useInfo.precolor == defInfo.precolor) {
          assertx(!matchedDef[def]);
          assertx(compat_class(useInfo.regClass, defInfo.regClass));
          match = def;
          return false;
        }
        return true;
      }
    );
    return match;
  };

  // If the precolored use can be paired up with a precolored def (which isn't
  // already paired and not the same precolor), then return the def.
  auto const findNotMatchingPrecoloredDef = [&] (Vreg use) {
    assertx(!matchedUse[use]);
    auto const& useInfo = reg_info(state, use);
    assertx(useInfo.precolor != InvalidReg);

    Vreg match;
    defs.forEach(
      [&] (Vreg def) {
        assertx(!match.isValid());
        if (matchedDef[def]) return true;
        auto const& defInfo = reg_info(state, def);
        if (defInfo.precolor != InvalidReg &&
            compat_class(useInfo.regClass, defInfo.regClass)) {
          assertx(useInfo.precolor != defInfo.precolor);
          match = def;
          return false;
        }
        return true;
      }
    );
    return match;
  };

  // If the precolored use can be paired up with a non-precolored def (which
  // hasn't already been paired), return the def.
  auto const findMatchingUnconstrainedDef = [&] (Vreg use) {
    assertx(!matchedUse[use]);
    auto const& useInfo = reg_info(state, use);
    assertx(useInfo.precolor != InvalidReg);

    Vreg match;
    defs.forEach(
      [&] (Vreg def) {
        assertx(!match.isValid());
        if (matchedDef[def]) return true;
        auto const& defInfo = reg_info(state, def);
        if (defInfo.precolor == InvalidReg &&
            compat_class(useInfo.regClass, defInfo.regClass)) {
          match = def;
          return false;
        }
        return true;
      }
    );
    return match;
  };

  // Try to pair up uses with defs, preferring use/def pairs which have the same
  // precolor register first.
  auto const matchDef = [&] (auto f) {
    uses.forEach(
      [&] (Vreg r) {
        if (matchedUse[r] ||
            reg_info(state, r).precolor == InvalidReg ||
            live_in_at(state, r, block, instIdx+1)) {
          return;
        }
        auto const match = f(r);
        if (match.isValid()) {
          matchedUse.add(r);
          matchedDef.add(match);
        }
      }
    );
  };
  matchDef(findMatchingPrecoloredDef);
  matchDef(findMatchingUnconstrainedDef);

  // If there's any unpaired uses left, force them to be "across".
  VregSet forcedAcross;
  uses.forEach(
    [&] (Vreg r) {
      if (matchedUse[r] ||
          reg_info(state, r).precolor == InvalidReg ||
          live_in_at(state, r, block, instIdx+1)) {
        return;
      }
      auto const match = findNotMatchingPrecoloredDef(r);
      if (match.isValid()) forcedAcross.add(r);
    }
  );

  uses -= forcedAcross;
  acrosses |= forcedAcross;
}

// The state of which live Vregs are currently spilled or not-spilled. This
// encapsulates that state, as well as the logic to decide which registers
// should be spilled or reloaded.

struct SpillerState {
  // Default state is no Vregs are live
  explicit SpillerState(const State& state)
    : gp{size_t(state.gpUnreserved.size())}
    , simd{size_t(state.simdUnreserved.size())}
    , state{&state}
  {}

  // We track the Vreg state separately for each register class.
  struct PerClass {
    size_t numRegs;
    VregSet inReg;
    VregSet inMem;
  };
  PerClass gp;
  PerClass simd;
  const State* state;

  // Given a Vreg, return the per-class state appropriate for that Vreg (or
  // nullptr if untracked). This lets code manipulate the state generically
  // without having to switch on register class constantly.
  const PerClass* forReg(Vreg r) const {
    if (r.isPhys()) return nullptr;
    switch (reg_info(*state, r).regClass) {
      case RegClass::AnyNarrow:
      case RegClass::GP:
        return &gp;
      case RegClass::SIMD:
      case RegClass::SIMDWide:
        return &simd;
      case RegClass::SF:
        return nullptr;
      case RegClass::Any:
      case RegClass::Spill:
      case RegClass::SpillWide:
        break;
    }
    always_assert(false);
  }
  PerClass* forReg(Vreg r) {
    return const_cast<PerClass*>(
      const_cast<const SpillerState*>(this)->forReg(r)
    );
  }

  /*
   * Using the spill weights at the given position, move any Vregs which need to
   * be moved from "inReg" to "inMem" to bring the number of Vregs in "inReg"
   * back below the size of the register class. If we cannot move enough Vregs,
   * then we'll assert (which means a bug). The set if Vregs thus moved is
   * returned. Spill instructions will need to be generated for these Vregs.
   *
   * The "forbidden" set is the set of Vregs which are not eligible
   * for spilling (typically the set of Vregs which are operands to the
   * instruction and thus cannot be spilled).
   *
   * The "retry" flag changes the semantics a bit. If the retry flag is set,
   * then we first attempt to spill registers as normal. However if we cannot,
   * we'll retry, ignoring the forbidden set. This is used to treat the
   * forbidden set as a hint instead of a hard rule.
   *
   * This function is typically used after some Vregs are moved into the "inReg"
   * set (perhaps because of reloading). You then call this to spill as
   * necessary (using the Vregs just reloaded as the forbidden set).
   */
  VregSet spill(Vlabel b,
                size_t instIdx,
                const VregSet& forbidden = VregSet{},
                bool retry = false) {
    // We'll calculate spill for GP and SIMD separately and then combine the
    // results (since they do not interact).
    auto const impl = [&] (PerClass& per, bool recursed, auto const& self) {
      // If we're already below the size, there's nothing to do (this is the
      // common case).
      if (per.inReg.size() <= per.numRegs) return VregSet{};

      // Otherwise we really have to spill. Gather up the candidates with their
      // spill weights at this point.
      jit::vector<std::pair<Vreg, uint64_t>> candidates;
      candidates.reserve(per.inReg.size());
      per.inReg.forEach(
        [&] (Vreg r) {
          // Are we ignoring the forbidden set?
          if (!recursed && forbidden[r]) return;
          candidates.emplace_back(r, spill_weight_at(*state, r, b, instIdx));
        }
      );

      // If we have less candidates than we need to spill, we're in trouble.
      auto const toRemove = per.inReg.size() - per.numRegs;
      if (toRemove > candidates.size()) {
        // If the forbidden set is "soft" then retry without it.
        if (retry && !recursed) return self(per, true, self);
        always_assert(false);
      }

      // Sort them according to their spill weights. Higher weights (more
      // profitable to spill) come first. Use the Vreg number to break ties.
      std::sort(
        candidates.begin(),
        candidates.end(),
        [](std::pair<Vreg, uint64_t> a, std::pair<Vreg, uint64_t> b) {
          if (a.second > b.second) return true;
          if (a.second < b.second) return false;
          return a.first < b.first;
        }
      );

      // Spill the first N, remove them from the inReg state and put them in
      // inMem to reflect they're spilled.
      VregSet spilled;
      for (size_t i = 0; i < toRemove; ++i) spilled.add(candidates[i].first);
      per.inReg -= spilled;
      per.inMem |= spilled;

      return spilled;
    };
    return impl(gp, false, impl) | impl(simd, false, impl);
  }

  // Remove any Vregs (in the candidates set) from being tracked which are not
  // live after the current position.
  void dropDead(const VregSet& candidates,
                Vlabel b,
                size_t instIdx) {
    assertx(checkInvariants(Vlabel{}, 0));
    candidates.forEach(
      [&] (Vreg r) {
        auto s = forReg(r);
        if (!s) return;
        assertx(s->inReg[r] || s->inMem[r]);
        if (live_in_at(*state, r, b, instIdx+1)) return;
        s->inReg.remove(r);
        s->inMem.remove(r);
      }
    );
  }

  std::string toString() const {
    return folly::sformat(
      "GP: [Reg: {} Mem: {}] SIMD: [Reg: {} Mem: {}]",
      show(gp.inReg), show(gp.inMem),
      show(simd.inReg), show(simd.inMem)
    );
  }

  // Sanity checking
  bool checkInvariants(Vlabel b, size_t instIdx) const {
    auto const impl = [&] (const PerClass& per) {
      // If we can't spill, we should never have a Vreg in memory.
      always_assert(IMPLIES(!state->abi.canSpill, per.inMem.none()));
      // We should never have more un-spilled Vregs than there are physical
      // registers for the class (except before a call to spill()).
      always_assert(per.inReg.size() <= per.numRegs);
      // A Vreg is either spilled or not.
      always_assert((per.inReg & per.inMem).none());
      (per.inReg | per.inMem).forEach(
        [&] (Vreg r) {
          // A Vreg should only be tracked if its actually live at this
          // position, and a Vreg should be in the per-class state appropriate
          // for it.
          always_assert(!r.isPhys());
          always_assert(forReg(r) == &per);
          always_assert(!b.isValid() || live_in_at(*state, r, b, instIdx));
        }
      );
    };
    impl(gp);
    impl(simd);
    return true;
  }
};

// SpillerState at block boundaries, along with created Vregs which need SSA
// restoration.
struct SpillerResults {
  explicit SpillerResults(const State& state)
    : perBlock(state.unit.blocks.size()) {}

  struct PerBlock {
    folly::Optional<SpillerState> in;
    folly::Optional<SpillerState> out;
    // Phi inputs/outputs. If a bit is set at index N, it means the phi
    // input/output at position N of the phi instruction is non-spilled.
    folly::Optional<boost::dynamic_bitset<>> inPhi;
    folly::Optional<boost::dynamic_bitset<>> outPhi;
  };
  jit::vector<PerBlock> perBlock;
  VregSet ssaize;

  std::string toString(const State& state) const {
    std::string ret;
    for (auto const b : state.rpo) {
      auto const& per = perBlock[b];
      std::string inPhi;
      std::string outPhi;
      if (per.inPhi) {
        boost::to_string(*per.inPhi, inPhi);
      } else {
        inPhi = "*";
      }
      if (per.outPhi) {
        boost::to_string(*per.outPhi, outPhi);
      } else {
        outPhi = "*";
      }
      ret += folly::sformat(
        "  {:5}:\n"
        "    In      -> {}\n"
        "    Out     -> {}\n"
        "    In-Phi  -> {}\n"
        "    Out-Phi -> {}\n",
        b,
        per.in ? per.in->toString() : "*",
        per.out ? per.out->toString() : "*",
        inPhi,
        outPhi
      );
    }
    return ret;
  }
};

// Update the phi spiller state for block b to account for which Vregs have been
// spilled or not.
void record_phi_spill_state_helper(Vlabel b,
                                   const VregList& defs,
                                   SpillerState& spillerState,
                                   SpillerResults& results) {
  auto& in = results.perBlock[b].inPhi.emplace();
  in.resize(defs.size());
  for (size_t i = 0; i < defs.size(); ++i) {
    auto const r = defs[i];
    auto const s = spillerState.forReg(r);
    if (!s) continue;
    assertx(s->inReg[r] || s->inMem[r]);
    in[i] = s->inReg[r];
  }

  // Some of the phi outputs might be immediately dead, so drop them now.
  spillerState.dropDead(
    spillerState.gp.inReg | spillerState.gp.inMem |
    spillerState.simd.inReg | spillerState.simd.inMem,
    b,
    0
  );
}

// Initialize the spiller state for entry into a block (populating the block's
// in state) when the block is a loop header. We apply a few heuristics when
// we're in a loop header to avoid spilling things inside the loop. This decides
// which Vregs should be considered spilled or non-spilled upon entry to the
// block. We can make this decision independently per-block because we'll fix up
// any mismatches later. The starting instruction index is returned (to skip
// over any phi).
size_t setup_initial_spiller_state_loop(const State& state,
                                        Vlabel b,
                                        SpillerResults& results) {
  assertx(is_loop_header(state, b));
  auto const& unit = state.unit;

  SpillerState initial{state};

  // Start with all Vregs live-in to the block. Also include any Vregs defined
  // by a phidef.
  auto alive = state.liveIn[b];
  const VregList* defs = nullptr;
  if (unit.blocks[b].code.front().op == Vinstr::phidef) {
    auto const& phidef = unit.blocks[b].code.front().phidef_;
    defs = &unit.tuples[phidef.defs];

    assertx(b != unit.entry);
    assertx(!is_constrained_inst(state, phidef));
    assertx(usesSet(unit, phidef).none());

    for (size_t i = 0; i < defs->size(); ++i) {
      auto const r = (*defs)[i];
      if (!initial.forReg(r)) continue;
      alive.add(r);
    }
  }

  auto const& loopInfo = loop_info(state, b);

  // Split the Vregs into those which are used inside the loop, and those which
  // are just live-through (but unused).
  auto const usedWithin = loopInfo.uses & alive;
  auto const liveThrough = alive - usedWithin;

  // Decide which Vregs should be assumed to be non-spilled on the block entry,
  // and which ones should be assumed to be spilled on block entry. Use the max
  // register pressure within the loop as a guide.
  auto const process = [&] (SpillerState::PerClass& s, size_t pressure) {
    // Assume that all Vregs used within the loop are non-spilled.
    size_t usedCount = 0;
    usedWithin.forEach(
      [&] (Vreg r) {
        if (initial.forReg(r) != &s) return;
        s.inReg.add(r);
        ++usedCount;
      }
    );

    if (usedCount < s.numRegs) {
      // There's still some space left after taking into account the Vregs used
      // within the loop. We might be able to allow some live-through Vregs to
      // be in registers.
      size_t throughCount = 0;
      liveThrough.forEach(
        [&] (Vreg r) {
          if (initial.forReg(r) != &s) return;
          ++throughCount;
        }
      );

      throughCount += s.numRegs;
      if (throughCount > pressure) {
        // There's still some space left. Pick the live-through Vregs which have
        // the lowest spill weights (less profitable to spill) to also be
        // non-spilled.
        throughCount -= pressure;

        jit::vector<std::pair<Vreg, uint64_t>> candidates;
        candidates.reserve(liveThrough.size());
        liveThrough.forEach(
          [&] (Vreg r) {
            if (initial.forReg(r) != &s) return;
            candidates.emplace_back(r, spill_weight_at(state, r, b, 0));
          }
        );

        std::sort(
          candidates.begin(),
          candidates.end(),
          [](std::pair<Vreg, uint64_t> a, std::pair<Vreg, uint64_t> b) {
            return std::tie(a.second, a.first) < std::tie(b.second, b.first);
          }
        );

        // The first N are non-spilled and the rest are spilled.
        auto const numReg = std::min(throughCount, candidates.size());
        for (size_t i = 0; i < numReg; ++i) {
          s.inReg.add(candidates[i].first);
        }
        for (size_t i = numReg; i < candidates.size(); ++i) {
          s.inMem.add(candidates[i].first);
        }

        assertx(s.inReg.size() <= s.numRegs);
        return;
      }
    }

    // We used up all the space for Vregs used within the loop. The live-through
    // Vregs have to be spilled.
    liveThrough.forEach(
      [&] (Vreg r) {
        if (initial.forReg(r) != &s) return;
        s.inMem.add(r);
      }
    );
  };
  process(initial.gp, loopInfo.gpPressure);
  process(initial.simd, loopInfo.simdPressure);

  // If we have more used-within Vregs than physical registers, we still might
  // need to move some to the spilled category. Do so here.
  initial.spill(b, 0);
  assertx(!results.perBlock[b].inPhi);

  // If this block has a phidef, we need to record which outputs of the phi were
  // considered to be spilled or not.
  if (defs) {
    record_phi_spill_state_helper(
      b,
      *defs,
      initial,
      results
    );
  }

  // Record block state
  assertx(!results.perBlock[b].in);
  assertx(initial.checkInvariants(b, !!defs));
  results.perBlock[b].in = std::move(initial);

  return !!defs;
}

// Initialize the spiller state for entry into a block (populating the block's
// in state). This decides which Vregs should be considered spilled or
// non-spilled upon entry to the block. We take into account which Vregs have
// been spilled in predecessors, but we're not required to, since we'll fix up
// any mismatches later. The starting instruction index is returned (to skip
// over any phi).
size_t setup_initial_spiller_state(const State& state,
                                   Vlabel b,
                                   SpillerResults& results) {
  assertx(b != state.unit.entry || state.liveIn[b].none());

  // Loop headers are dealt with specially.
  if (is_loop_header(state, b)) {
    return setup_initial_spiller_state_loop(state, b, results);
  }

  auto const& unit = state.unit;
  SpillerState initial{state};

  // First iterate over all (already processed) predecessors. If a Vreg is
  // spilled in all the predecessors, we'll consider it spilled initially as
  // well. Otherwise we'll optimistically consider it non-spilled. Also track
  // any Vregs which are non-spilled in all predecessors (which will be given
  // priority).
  VregSet allInReg;
  auto first = true;
  for (auto const& pred : state.preds[b]) {
    auto const& out = results.perBlock[pred].out;
    if (!out) continue;
    // There shouldn't be anything live which isn't tracked.
    assertx(
      (state.liveIn[b] -
       (out->gp.inReg | out->simd.inReg | out->gp.inMem | out->simd.inMem)
      ).none()
    );

    if (first) {
      initial = *out;
      allInReg = out->gp.inReg | out->simd.inReg;
      first = false;
    } else {
      initial.gp.inReg |= out->gp.inReg;
      initial.simd.inReg |= out->simd.inReg;
      initial.gp.inMem &= out->gp.inMem;
      initial.simd.inMem &= out->simd.inMem;
      allInReg &= (out->gp.inReg | out->simd.inReg);
    }
  }
  // The predecessors can have live-out Vregs which aren't live-in here, so
  // remove those.
  initial.gp.inReg &= state.liveIn[b];
  initial.gp.inMem &= state.liveIn[b];
  initial.simd.inReg &= state.liveIn[b];
  initial.simd.inMem &= state.liveIn[b];
  allInReg &= state.liveIn[b];

  // Now that we've processed Vregs which were live-in to the block, we need to
  // consider phidef outputs. We can use similar logic to the live-in Vregs,
  // except examining the phi state of the predecessors.
  const VregList* defs = nullptr;
  if (unit.blocks[b].code.front().op == Vinstr::phidef) {
    auto const& phidef = unit.blocks[b].code.front().phidef_;
    defs = &unit.tuples[phidef.defs];

    assertx(b != unit.entry);
    assertx(!is_constrained_inst(state, phidef));
    assertx(usesSet(unit, phidef).none());

    boost::dynamic_bitset<> allInRegPhis;
    boost::dynamic_bitset<> inRegPhis;
    first = true;
    for (auto const pred : state.preds[b]) {
      auto const& out = results.perBlock[pred].outPhi;
      if (!out) continue;

      assertx(out->size() == defs->size());
      if (first) {
        inRegPhis = *out;
        allInRegPhis = *out;
        first = false;
      } else {
        allInRegPhis &= *out;
        inRegPhis |= *out;
      }
    }

    // Similar to the live-in Vregs, only consider a Vreg as spilled if its been
    // spilled in all predecessors. Otherwise treat it as non-spilled.
    for (size_t i = 0; i < defs->size(); ++i) {
      auto const r = (*defs)[i];
      auto const s = initial.forReg(r);
      if (!s) {
        assertx(!allInRegPhis[i] && !inRegPhis[i]);
        continue;
      }
      if (inRegPhis[i]) {
        s->inReg.add(r);
        if (allInRegPhis[i]) allInReg.add(r);
      } else {
        assertx(!allInRegPhis[i]);
        s->inMem.add(r);
      }
    }
  }

  // We might have more non-spilled Vregs than we can fit into registers. Spill
  // as necessary. We hint that the Vregs which were non-spilled in all
  // predecessors should not be spilled.
  initial.spill(b, 0, allInReg, true);
  assertx(!results.perBlock[b].inPhi);

  // If this block has a phidef, we need to record which outputs of the phi were
  // considered to be spilled or not.
  if (defs) {
    record_phi_spill_state_helper(
      b,
      *defs,
      initial,
      results
    );
  }

  // Record block state
  assertx(!results.perBlock[b].in);
  assertx(initial.checkInvariants(b, !!defs));
  results.perBlock[b].in = std::move(initial);

  return !!defs;
}

// Emit instructions to reload Vreg src into Vreg dst, possibly using
// rematerialization. If rematerialization is not possible, then a reload vasm
// instruction is emitted. The number of instructions emitted is returned.
size_t reload_with_remat(Vout& v,
                         State& state,
                         const VregSet& gpInReg,
                         const VregSet& simdInReg,
                         Vreg src,
                         Vreg dst) {
  auto const reload = [&] (Vout& v) { v << jit::reload{src, dst}; return 1; };

  // No rematerialized instruction, just emit a reload instruction.
  auto const& sInfo = reg_info(state, src);
  if (!sInfo.remat) return reload(v);

  // Check if all of the rematerialized instructions are available. This check
  // is actually pointless because we do not allow rematerialization candidates
  // which take inputs right now. This is because the check isn't quite correct
  // because we might spill one of the inputs after this check. TODO
  // (TT37650327): Handle instructions with sources
  auto srcsAvailable = true;
  visitUses(
    state.unit, *sInfo.remat,
    [&] (Vreg r) {
      assertx(!r.isPhys());
      if (!srcsAvailable) return;
      srcsAvailable = gpInReg[r] || simdInReg[r];
    }
  );
  // If the inputs aren't available, just reload
  if (!srcsAvailable) return reload(v);

  // Pick a Vreg to use as the rematerialized instruction output. If the dest
  // Vreg is precolored, make a new one. Otherwise just use the dest. We need to
  // do this because if the dest Vreg is precolored, we'll have to make this
  // rematerialized instruction constrained, which means we need to insert
  // live-range breaks *before it*, but its too late to do that. Instead we'll
  // use a temporary and copy it to the dest Vreg (which doesn't introduce any
  // constraints).
  auto const& dInfo = reg_info(state, dst);
  auto const temp = (dInfo.precolor != InvalidReg)
    ? state.unit.makeReg()
    : dst;

  // Copy the rematerialized instruction, rewrite the output Vreg and insert it.
  auto inst = *sInfo.remat;
  visitRegsMutable(
    state.unit,
    inst,
    [] (Vreg r) { return r; },
    [&] (Vreg)  { return temp; }
  );
  v << inst;

  // If we used a new Vreg as the instruction output, assign it the same info as
  // the dest Vreg (except for the precolor), and insert a copy after the
  // rematerialized instruction. Note that we don't have to insert the temporary
  // into the spiller state because its dead after the copy.
  if (temp != dst) {
    auto tInfo = dInfo;
    tInfo.precolor = InvalidReg;
    reg_info_insert(state, temp, std::move(tInfo));
    v << copy{temp, dst};
    return 2;
  }

  return 1;
}

// Run spiller logic for a phijmp instruction. Phis can handle spilled Vregs, so
// we don't need to reload the phi's inputs.
void process_phijmp_spills(const State& state,
                           Vlabel b,
                           size_t instIdx,
                           const phijmp& phijmp,
                           SpillerState& spiller,
                           SpillerResults& results) {
  auto const& unit = state.unit;

  // This should end a block
  assertx(instIdx == unit.blocks[b].code.size() - 1);
  assertx(!is_constrained_inst(state, phijmp));
  assertx(defsSet(unit, phijmp).none());
  assertx(acrossesSet(unit, phijmp).none());

  // Examine which Vregs are currently non-spilled and record them in the phi
  // state.
  auto const& uses = unit.tuples[phijmp.uses];
  assertx(!results.perBlock[b].outPhi);
  auto& outPhi = results.perBlock[b].outPhi.emplace();
  outPhi.resize(uses.size());
  for (size_t i = 0; i < uses.size(); ++i) {
    auto const r = uses[i];
    auto const s = spiller.forReg(r);
    if (!s) continue;
    assertx(s->inReg[r] || s->inMem[r]);
    outPhi[i] = s->inReg[r];
  }

  // Remove any Vregs which aren't used after the phi so they won't be part of
  // the block's live-out.
  spiller.dropDead(usesSet(unit, phijmp), b, instIdx);
}

// Run spiller logic for a copy-ish instruction. Copies need to be handle
// specially because unlike normal instructions, they can handle both spilled
// and non-spilled Vregs (therefore we don't need to reload the inputs before
// the instruction). We only need to ensure that a copy doesn't attempt to move
// between a spilled or non-spilled Vreg (or vice-versa). Indeed, we need to
// ensure that a copy never causes a reload, as we need to be able to copy
// spilled Vregs around without introducing additional register presure. Return
// the number of instructions inserted.
size_t process_copy_spills(State& state,
                           VregList& uses,
                           const VregList& defs,
                           Vlabel b,
                           size_t instIdx,
                           SpillerState& spiller,
                           SpillerResults& results) {
  auto& unit = state.unit;

  assertx(uses.size() == defs.size());

  // Record which uses of the copy are currently spilled or not.
  VregSet inReg;
  VregSet inMem;
  for (size_t i = 0; i < uses.size(); ++i) {
    auto const u = uses[i];
    auto const d = defs[i];
    auto const s = spiller.forReg(u);
    if (!s) {
      assertx(!spiller.forReg(d));
      continue;
    }
    assertx(s->inReg[u] || s->inMem[u]);
    assertx(spiller.forReg(d));
    if (s->inReg[u]) {
      inReg.add(d);
    } else {
      inMem.add(d);
    }
  }

  // Remove any Vregs which aren't used after the copy from being tracked.
  spiller.dropDead(VregSet{uses}, b, instIdx);

  // Now do the same for the copy outputs.
  for (size_t i = 0; i < defs.size(); ++i) {
    auto const d = defs[i];
    auto const s = spiller.forReg(d);
    if (!s) {
      assertx(!inReg[d] && !inMem[d]);
      assertx(!spiller.forReg(uses[i]));
      continue;
    }
    // Since this is being defined here, we shouldn't already be tracking it.
    assertx(!s->inReg[d] && !s->inMem[d]);
    assertx(spiller.forReg(uses[i]));
    if (inReg[d]) {
      s->inReg.add(d);
    } else {
      s->inMem.add(d);
    }
  }

  // Adding the defs may have exceeded the available registers, so spill as
  // necessary.
  auto const spills = spiller.spill(b, instIdx);
  if (spills.none()) {
    // If no spilling is required, then remove any immediately dead defs and
    // then we're done.
    spiller.dropDead(VregSet{defs}, b, instIdx);
    return 0;
  }

  // We have to spill. Make sure we can.
  always_assert(state.abi.canSpill);

  if (debug) {
    spills.forEach(
      [&] (Vreg r) { always_assert(spiller.forReg(r)); }
    );
  }

  // We know which Vregs we want to spill. Copys can handle copying between
  // spilled Vregs on both sides and non-spilled Vregs on both sides (but not
  // mixing). Moving between spilled and non-spilled Vregs is the job of the
  // spill and reload instructions. So, if we need to spill one side of the copy
  // and not the other (and vice-versa), we need to insert other instructions to
  // keep this invariant.

  jit::vector<std::pair<Vreg, Vreg>> aliasPairs; // Emit ssaalias p1 -> p2
  jit::vector<std::pair<Vreg, Vreg>> spillPairs; // Emit spill p1 -> p2
  auto selfSpills = spills; // Emit spill p -> p
  for (size_t i = 0; i < uses.size(); ++i) {
    auto const u = uses[i];
    auto const d = defs[i];

    if (spills[u]) {
      if (!spills[d]) {
        /*
         * The src has been spilled, but dst has not. We need to make the input
         * and output of the copy agree with regards to the spillness. We can't
         * reload src because we've already decided we don't have the available
         * registers to do that. Instead we'll spill src (because its in
         * selfSpills). We'll emit a ssaalias instruction, aliasing src to dst,
         * rewrite the copy to take dst instead, and mark dst as needing SSA
         * restoration.
         *
         * For example:
         *   copy src -> dst
         *   conjureuse dst
         *
         * Becomes:
         *   ssaalias src -> dst
         *   spill src -> src
         *   copy dst -> dst
         *   conjureuse dst
         *
         * After SSA:
         *  spill src -> src2
         *  copy src -> dst
         *  conjureuse dst
         */
        aliasPairs.emplace_back(u, d);
        uses[i] = d;
        results.ssaize.add(d);
      } else {
        /*
         * Both the src and dst have been spilled. Copies can handle spilled
         * Vregs just fine, so we'll spill src (because its in selfSpills), but
         * don't need to do anything with the dst (so remove it from
         * selfSpills).
         *
         * For example:
         *   copy src -> dst
         *   conjureuse dst
         *
         * Becomes:
         *   spill src -> src
         *   copy src -> dst
         *   conjureuse dst
         *
         * After SSA:
         *   spill src -> src2
         *   copy src2 -> dst
         *   conjureuse dst
         */
        selfSpills.remove(d);
      }
    } else if (spills[d]) {
      /*
       * The src isn't spilled, but dst has been. The copy can handle spilled
       * Vregs just fine, so we'll emit a spill instruction before the copy,
       * spilling the src into the dst. We'll rewrite the copy to take dst as
       * the src. We'll mark dst as needing SSA conversion, so the copy will get
       * a new Vreg as its output and all down stream users will be rewritten.
       *
       * For example:
       *   copy src -> dst
       *   conjureuse dst
       *
       * Becomes:
       *   spill src -> dst
       *   copy dst -> dst
       *   conjureuse dst
       *
       * After SSA:
       *   spill src -> dst
       *   copy dst -> dst2
       *   conjureuse dst2
       */
      spillPairs.emplace_back(u, d);
      uses[i] = d;
      selfSpills.remove(d);
      results.ssaize.add(d);
    }
  }

  // Anything self spilled needs to be SSA restored because we spill to itself.
  results.ssaize |= selfSpills;

  // Now that we've calculated everything, emit the actual instructions. We need
  // to emit them in this particular order for correctness.
  size_t added = 0;
  vmodify(
    unit, b, instIdx,
    [&] (Vout& v) {
      for (auto const& p : aliasPairs) {
        v << ssaalias{p.first, p.second};
        ++added;
      }
      for (auto const& p : spillPairs) {
        v << spill{p.first, p.second};
        ++added;
      }
      selfSpills.forEach(
        [&] (Vreg r) {
          v << spill{r, r};
          ++added;
        }
      );
      return 0;
    }
  );

  // Remove any Vregs which the copy defined and are immediately dead.
  spiller.dropDead(VregSet{defs}, b, instIdx + added);
  return added;
}

// Run spiller logic for normal instructions (not phis or copies). Return the
// number of instructions inserted.
size_t process_inst_spills(State& state,
                           Vlabel b,
                           size_t instIdx,
                           const Vinstr& inst,
                           SpillerState& spiller,
                           SpillerResults& results) {
  // We allow Vinstr::copy if the src/dest is a physical register. In that case,
  // we treat it like a normal instruction (not giving it the special copy
  // logic).
  assertx(inst.op != Vinstr::phijmp &&
          inst.op != Vinstr::phidef &&
          inst.op != Vinstr::copyargs);

  auto& unit = state.unit;

  // NB: acrosses is a subset of uses at this point
  auto uses = usesSet(unit, inst) - state.reservedRegs;
  auto acrosses = acrossesSet(unit, inst) - state.reservedRegs;
  auto const defs = defsSet(unit, inst) - state.reservedRegs;

  auto const isConstrained = is_constrained_inst(state, inst);
  VregSet reloads; // Vregs we'll have to reload
  VregSet spills;  // Vregs we'll have to spill
  VregSet copies;  // Vregs we'll have to emit a copyargs for

  // First process the uses (including acrosses). If any use is currently
  // spilled, make it non-spilled and record that it needs a reload.
  uses.forEach(
    [&] (Vreg r) {
      auto s = spiller.forReg(r);
      if (!s) return;
      assertx(s->inReg[r] || s->inMem[r]);
      if (s->inMem[r]) {
        reloads.add(r);
        s->inMem.remove(r);
      }
      s->inReg.add(r);
    }
  );
  // Moving the uses into registers may require us to spill other Vregs (except
  // the ones we just reloaded).
  spills |= spiller.spill(b, instIdx, uses);
  uses -= acrosses; // Make uses and acrosses disjoint

  if (isConstrained) {
    // If this instruction is constrained, we may need to move some of the uses
    // into the acrosses set to propertly model register pressure. We also need
    // to insert a copyargs for all non-spilled Vregs to split their live
    // ranges.
    fix_constrained_inst_uses(state, defs, uses, acrosses, b, instIdx);
    copies = spiller.gp.inReg | spiller.simd.inReg;
  }

  // If any of the uses aren't used after this, drop them from being tracked to
  // make room for the defs. Since we removed the acrosses from the uses, we'll
  // keep those alive.
  spiller.dropDead(uses, b, instIdx);

  // Process the defs. The defs always start in registers because the
  // instruction has to write them there.
  defs.forEach(
    [&] (Vreg r) {
      auto s = spiller.forReg(r);
      if (!s) return;
      // We shouldn't already be tracking this
      assertx(!s->inReg[r] && !s->inMem[r]);
      s->inReg.add(r);
    }
  );
  // Adding the defs may have caused us to need to spill even more (but not any
  // uses or defs).
  spills |= spiller.spill(b, instIdx, defs | acrosses | uses);
  // Some of the defs or acrosses may not be live (if the defs are never used),
  // so remove them.
  spiller.dropDead(defs | acrosses, b, instIdx);

  // If we ended up spilling a Vreg, we don't need a copyarg for it. If we
  // reloaded one, we need to copy that as well.
  if (isConstrained) {
    copies -= spills;
    copies |= reloads;
  }

  // Make sure we can spill.
  always_assert_flog(
    IMPLIES(!state.abi.canSpill, spills.none() && reloads.none()),
    "Trying to spill/reload for {} in {} when not allowed "
    "(Spills: {}, Reloads: {})",
    show(state.unit, inst),
    b,
    show(spills),
    show(reloads)
  );

  // Bail out if we don't need to emit anything (hopefully the common case).
  if (spills.none() && reloads.none() && copies.none()) return 0;
  // We should either spill or reload a Vreg, not both.
  assertx((spills & reloads).none());

  // We always spill/reload/copy a Vreg to itself, so they'll need to be
  // re-SSAized.
  results.ssaize |= spills;
  results.ssaize |= reloads;
  results.ssaize |= copies;

  size_t added = 0;
  vmodify(
    unit, b, instIdx,
    [&] (Vout& v) {
      // First emit the spills (which decreases register pressure).
      spills.forEach(
        [&] (Vreg r) { v << spill{r, r}; ++added; }
      );

      // Now emit the reloads (which increases register pressure), possibily
      // with rematerialization.
      reloads.forEach(
        [&] (Vreg r) {
          added += reload_with_remat(
            v,
            state,
            spiller.gp.inReg,
            spiller.simd.inReg,
            r,
            r
          );
        }
      );

      // Always emit the copy/copyargs at the end. It needs to immediately
      // precede the constrained instruction. We always emit one instruction for
      // this.
      if (copies.any()) {
        VregList operands;
        copies.forEach([&] (Vreg r) { operands.emplace_back(r); });
        if (operands.size() == 1) {
          v << copy{operands[0], operands[0]};
        } else {
          v << copyargs{v.makeTuple(operands), v.makeTuple(operands)};
        }
        ++added;
      }

      return 0;
    }
  );

  return added;
}

// Run the spiller logic over the entire unit. Returning the spiller state at
// block boundaries (and Vregs which need SSA conversion). Each block is allowed
// to determine which of its in/out Vregs should be spilled or not,
// independently of all others. We'll later pass over all the state and insert
// spills/reloads as needed to make each block compatible.
SpillerResults process_spills(State& state) {
  auto& unit = state.unit;

  SpillerResults results{state};
  SCOPE_ASSERT_DETAIL("Spiller State") { return results.toString(state); };

  for (auto const b : state.rpo) {
    // Initialize the state for this block and get the initial (in) state.
    auto instIdx = setup_initial_spiller_state(state, b, results);
    auto spiller = *results.perBlock[b].in;

    // Iterate over each instruction and run the logic for each one. We need to
    // use indices because we'll modify the unit as part of processing it (which
    // means we need to shift the indices).
    for (; instIdx < unit.blocks[b].code.size(); ++instIdx) {
      assertx(spiller.checkInvariants(b, instIdx));

      auto const& inst = unit.blocks[b].code[instIdx];
      switch (inst.op) {
        case Vinstr::phijmp:
          process_phijmp_spills(
            state, b, instIdx, inst.phijmp_, spiller, results
          );
          break;
        case Vinstr::copyargs:
          assertx(!is_constrained_inst(state, inst));
          assertx(acrossesSet(unit, inst).none());
          instIdx += process_copy_spills(
            state,
            unit.tuples[inst.copyargs_.s],
            unit.tuples[inst.copyargs_.d],
            b,
            instIdx,
            spiller,
            results
          );
          break;
        case Vinstr::copy: {
          assertx(!is_constrained_inst(state, inst));
          assertx(acrossesSet(unit, inst).none());

          // Copies which have physical sources or dests are treated like normal
          // instructions.
          if (inst.copy_.s.isPhys() || inst.copy_.d.isPhys()) {
            instIdx += process_inst_spills(
              state,
              b,
              instIdx,
              inst,
              spiller,
              results
            );
            break;
          }

          // process_copy_spills takes its uses and defs as a VregList, so use a
          // temporary one to satisfy the interface.
          VregList uses{inst.copy_.s};
          instIdx += process_copy_spills(
            state,
            uses,
            VregList{inst.copy_.d},
            b,
            instIdx,
            spiller,
            results
          );
          unit.blocks[b].code[instIdx].copy_.s = uses[0];
          break;
        }
        case Vinstr::phidef:
          // Should be handled as part of setting up the initial block state.
          always_assert(false);
          break;
        default:
          instIdx += process_inst_spills(
            state,
            b,
            instIdx,
            inst,
            spiller,
            results
          );
          break;
      }
    }

    assertx(spiller.checkInvariants(b, unit.blocks[b].code.size()));
    results.perBlock[b].out = std::move(spiller);
  }

  return results;
}

// Mismatches between a block and its successors. This indicates what should be
// inserted in the block to resolve the mismatch.
struct SpillMismatchState {
  VregSet spills;  // Vregs to spill
  VregSet reloads; // Vregs to reload

  // Map a spilled Vreg to the spill destination. If not present, the Vreg is
  // spilled to itself.
  jit::fast_map<Vreg, Vreg> spillDests;
  // Map a reloaded Vreg to the reload destination. If not present, the Vreg is
  // reloaded to itself.
  jit::fast_map<Vreg, Vreg> reloadDests;
  // ssaalias instructions to insert
  jit::fast_map<Vreg, Vreg> aliases;
};

// Calculate the spill/reload mismatches between a block and its successors.
SpillMismatchState find_spill_mismatches(State& state,
                                         Vlabel b,
                                         SpillerResults& results) {
  auto& unit = state.unit;

  auto& pred = results.perBlock[b].out;
  auto& predPhi = results.perBlock[b].outPhi;
  // We should have out state for everything at this point (but only predPhi if
  // this has a phijmp).
  assertx(pred);

  assertx(IMPLIES(!results.perBlock[b].in, b == unit.entry));
  assertx(pred->checkInvariants(b, unit.blocks[b].code.size()));

  SpillMismatchState mismatch;
  boost::dynamic_bitset<> phiSpills;
  boost::dynamic_bitset<> phiReloads;

  // Iterate over the successors and calculate where the states disagree.
  for (auto const succLabel : succs(unit.blocks[b])) {
    auto const& succ = results.perBlock[succLabel].in;
    auto const& succPhi = results.perBlock[succLabel].inPhi;
    assertx(succ);
    assertx((bool)predPhi == (bool)succPhi);

    // If the Vreg is in a register in the predecessor but spilled in the
    // successor, we need to spill it. Likewise, if the Vreg is spilled in the
    // predecessor but in a register in the successor, we need to reload it.
    auto const add = [&] (const SpillerState::PerClass& s1,
                          const SpillerState::PerClass& s2) {
      mismatch.spills |= (s1.inReg & s2.inMem);
      mismatch.reloads |= (s1.inMem & s2.inReg);
    };
    add(pred->gp, succ->gp);
    add(pred->simd, succ->simd);

    if (!predPhi) continue;

    // Do the same thing, but for phi input/outputs.
    assertx(predPhi->size() == succPhi->size());
    if (phiSpills.empty()) {
      assertx(phiReloads.empty());
      phiSpills.resize(predPhi->size());
      phiReloads.resize(predPhi->size());
    } else {
      assertx(phiSpills.size() == predPhi->size());
      assertx(phiReloads.size() == predPhi->size());
    }

    auto const diff = *predPhi ^ *succPhi;
    phiSpills |= (*predPhi & diff);
    phiReloads |= (*succPhi & diff);
  }

  assertx((mismatch.spills & mismatch.reloads).none());
  assertx(phiSpills.size() == phiReloads.size());
  assertx((phiSpills & phiReloads).none());

  // Since we split critical edges, the only way we can get mismatches is if the
  // block only has one successor and that successor has multiple
  // predecessors. This is good, because otherwise resolving the mismatches
  // would be difficult. (IE, some successors might want it spilled and some
  // might want it in physical registers).
  assertx(
    IMPLIES(mismatch.spills.any() || mismatch.reloads.any() ||
            phiSpills.any() || phiReloads.any(),
            succs(unit.blocks[b]).size() == 1 &&
            state.preds[succs(unit.blocks[b])[0]].size() > 1)
  );

  // Update the block's out state to match the changes we need to make.
  auto const update = [&] (SpillerState::PerClass& s) {
    s.inReg |= (mismatch.reloads & s.inMem);
    s.inMem |= (mismatch.spills & s.inReg);
    s.inReg -= mismatch.spills;
    s.inMem -= mismatch.reloads;
  };
  update(pred->gp);
  update(pred->simd);

  // If there's no phi, we're done.
  if (!predPhi) {
    assertx(phiReloads.empty());
    assertx(phiSpills.empty());
    return mismatch;
  }

  // Modify this block's phi state to match the successors.
  assertx(phiSpills.size() == predPhi->size());
  *predPhi |= phiReloads;
  *predPhi -= phiSpills;

  assertx(unit.blocks[b].code.back().op == Vinstr::phijmp);
  auto const& phijmp = unit.blocks[b].code.back().phijmp_;
  auto& uses = unit.tuples[phijmp.uses];

  // If there's a phi, we might need to add additional instructions. The reason
  // is because we might need to spill or reload Vregs to match the successor,
  // but the phijmp might take one of those Vregs as an input. We cannot defer
  // the spill or reloads after the phijmp because that's always the end of the
  // block. So, we might need to insert additional fixup instructions to make
  // sure the inputs of the phijmp matches the spilled/non-spilled state of the
  // phidef in the successor. Just like copies, phijmp/phidefs can handle Vregs
  // with the same spilled/non-spilled state on each side, but not a mix.
  VregSet newSpills;
  VregSet newReloads;
  for (size_t i = 0; i < uses.size(); ++i) {
    auto& r = uses[i];
    if (!pred->forReg(r)) {
      assertx(!phiSpills[i] && !phiReloads[i]);
      continue;
    }

    if (phiSpills[i])  newSpills.add(r);
    if (phiReloads[i]) newReloads.add(r);

    if (mismatch.spills[r] && !phiSpills[i]) {
      /*
       * A Vreg that's an input to the phijmp needs to be spilled, but the
       * successor expects it to be in a register when output by the
       * phidef. We'll make a temporary Vreg and alias it to the original input
       * (before the spill). The phijmp will then take the temporary as input
       * instead.
       *
       * Before:
       *   phijmp src
       *
       * After:
       *   ssaalias src -> src2
       *   spill src -> src
       *   phijmp src2
       *
       * After SSA:
       *   spill src -> src3
       *   phijmp src
       */

      // Re-use an existing alias Vreg if one exists.
      auto const it = mismatch.aliases.find(r);
      if (it != mismatch.aliases.end()) {
        r = it->second;
      } else {
        auto const r2 = unit.makeReg();
        mismatch.aliases.emplace(r, r2);
        r = r2;
      }
    } else if (!mismatch.spills[r] && phiSpills[i]) {
      /*
       * A Vreg that's an input to the phijmp doesn't need to be spilled, but
       * the successor expects it to be spilled when output by the phidef. This
       * case is easy to handle. Just spill to a temporary Vreg, and then have
       * the phijmp use that temporary as an input instead.
       *
       * Before:
       *  phijmp src
       *
       * After:
       *  spill src -> src2
       *  phijmp src2
       *
       * After SSA:
       *  spill src -> src2
       *  phijmp src2
       */
      auto const it = mismatch.spillDests.find(r);
      if (it != mismatch.spillDests.end()) {
        r = it->second;
      } else {
        auto const r2 = unit.makeReg();
        mismatch.spillDests.emplace(r, r2);
        r = r2;
      }
    }

    if (mismatch.reloads[r] && !phiReloads[i]) {
      /*
       * A Vreg that's an input to the phijmp needs to be reloaded, but the
       * successor expects it to be spilled when output by the phidef.  We'll
       * make a temporary Vreg and alias it to the original input (before the
       * reload). The phijmp will then take the temporary as input instead.
       *
       * Before
       *   phijmp src
       *
       * After:
       *  ssaalias src -> src2
       *  reload src -> src
       *  phijmp src2
       *
       * After SSA:
       *   reload src -> src3
       *   phijmp src
       */

      // Re-use an existing alias Vreg if one exists.
      auto const it = mismatch.aliases.find(r);
      if (it != mismatch.aliases.end()) {
        r = it->second;
      } else {
        auto const r2 = unit.makeReg();
        mismatch.aliases.emplace(r, r2);
        r = r2;
      }
    } else if (!mismatch.reloads[r] && phiReloads[i]) {
      /*
       * A Vreg that's an input to the phijmp doesn't need to be reloaded, but
       * the successor expects it to be in a register when output by the
       * phidef. This case is easy to handle. Just reload to a temporary Vreg,
       * and then have the phijmp use that temporary as an input instead.
       *
       * Before:
       *  phijmp src
       *
       * After:
       *  reload src -> src2
       *  phijmp src2
       *
       * After SSA:
       *  reload src -> src2
       *  phijmp src2
       */
      auto const it = mismatch.reloadDests.find(r);
      if (it != mismatch.reloadDests.end()) {
        r = it->second;
      } else {
        auto const r2 = unit.makeReg();
        mismatch.reloadDests.emplace(r, r2);
        r = r2;
      }
    }
  }

  mismatch.spills |= newSpills;
  mismatch.reloads |= newReloads;

  return mismatch;
}

// Find all mismatches between blocks in the unit, and insert spill/reloads as
// necessary to resolve them.
void fixup_spill_mismatches(State& state, SpillerResults& results) {
  auto& unit = state.unit;

  // For each block, find the mismatches and then materialize them.
  for (auto const b : state.rpo) {
    auto const mismatch = find_spill_mismatches(state, b, results);

    always_assert(
      IMPLIES(
        !state.abi.canSpill, mismatch.spills.none() && mismatch.reloads.none()
      )
    );

    if (mismatch.spills.any() || mismatch.reloads.any()) {
      results.ssaize |= mismatch.spills;
      results.ssaize |= mismatch.reloads;

      // Any new Vreg being used as a destination of a spill/reload/ssaalias
      // should have the same RegInfo as its source.
      auto const processMap = [&] (const jit::fast_map<Vreg, Vreg>& m) {
        for (auto const& p : m) {
          results.ssaize.add(p.first);
          results.ssaize.add(p.second);
          reg_info_insert(state, p.second, reg_info(state, p.first));
        }
      };
      processMap(mismatch.spillDests);
      processMap(mismatch.reloadDests);
      processMap(mismatch.aliases);

      // Materialize the actual instructions. The order of these is important.
      vmodify(
        unit, b, unit.blocks[b].code.size() - 1,
        [&] (Vout& v) {
          // First the ssaaliases, which must come before anything else.
          for (auto const& p : mismatch.aliases) {
            v << ssaalias{p.first, p.second};
          }

          // Then the spills, which reduce register pressure.
          mismatch.spills.forEach(
            [&] (Vreg r) {
              auto const it = mismatch.spillDests.find(r);
              auto const r2 =
                (it == mismatch.spillDests.end()) ? r : it->second;
              v << spill{r, r2};
            }
          );

          // Then the reloads, which increases register pressure.
          mismatch.reloads.forEach(
            [&] (Vreg r) {
              auto const it = mismatch.reloadDests.find(r);
              auto const r2 =
                (it == mismatch.reloadDests.end()) ? r : it->second;
              auto const& pred = results.perBlock[b].out;
              reload_with_remat(
                v,
                state,
                pred->gp.inReg,
                pred->simd.inReg,
                r,
                r2
              );
            }
          );

          return 0;
        }
      );
    }

    if (debug) {
      always_assert(
        results.perBlock[b].out->checkInvariants(
          b, unit.blocks[b].code.size()
        )
      );
      for (auto const s : succs(unit.blocks[b])) {
        always_assert(results.perBlock[b].outPhi == results.perBlock[s].inPhi);
      }
    }
  }
}

// Now that all spills and reloads have been inserted, we need to update the
// RegClass of spilled Vregs to indicate they represent spills. A Vreg
// represents a spill if its def is a spill instruction. The "spillness" is also
// propagated across copies and phis (if the source is a spill slot, the dest
// must be as well). Since this runs after SSA restoration, the mappings is used
// to map Vregs back to their original Vregs when necessary.
void set_spill_reg_classes(State& state,
                           const SpillerResults& results,
                           const jit::fast_map<Vreg, Vreg>& mappings) {
  auto const& unit = state.unit;

  // Mark that the given Vreg is a spill of the appropriate width.
  auto const spillize = [&] (Vreg r) {
    assertx(!r.isPhys());
    auto& info = reg_info(state, r);
    auto const cls = info.regClass;
    if (is_spill(cls)) return;
    assertx(cls != RegClass::SF);
    if (cls == RegClass::SIMDWide) {
      info.regClass = RegClass::SpillWide;
    } else {
      info.regClass = RegClass::Spill;
    }
  };

  for (auto const b : state.rpo) {
    auto const& in = results.perBlock[b].in;
    auto const& phi = results.perBlock[b].inPhi;
    assertx(in);

    for (auto const& inst : unit.blocks[b].code) {
      switch (inst.op) {
        case Vinstr::phidef: {
          // Propagate "spillness" across the phi. If the block's phi state
          // indicates that the phi output is spilled, the corresponding dest
          // Vreg must be spillized.

          // This is a bit tricky because the SSA restoration pass that we run
          // before this may have created new phis, or expanded existing
          // ones. Therefore we cannot trust the phi state blindly. If the index
          // exists in the phi state, we can use it. Otherwise we need to map
          // the Vreg back to its pre-SSAize value and use the block state.
          auto const& defs = unit.tuples[inst.phidef_.defs];
          for (size_t i = 0; i < defs.size(); ++i) {
            auto const r = defs[i];

            auto const s = in->forReg(r);
            if (!s) continue;
            if (phi && i < phi->size()) {
              // This phi and index existed before the SSA pass, so we can just
              // use the phi state.
              if (!(*phi)[i]) spillize(r);
              continue;
            }

            // Otherwise this wasn't part of a phi originally but was turned
            // into one. Consult the mapping table to learn what the Vreg was
            // originally and use the block state to see if it was spilled.
            auto const it = mappings.find(r);
            auto const oldR = (it != mappings.end()) ? it->second : r;
            if (s->inMem[oldR]) {
              spillize(r);
            } else {
              assertx(s->inReg[oldR]);
            }
          }
          break;
        }
        case Vinstr::spill:
          // The dest of a spill instruction is a spill.
          spillize(inst.spill_.d);
          break;
        case Vinstr::copy: {
          // Propagate "spillness" across copy
          auto const s = inst.copy_.s;
          auto const d = inst.copy_.d;
          if (s.isPhys()) break;
          auto const cls = reg_info(state, s).regClass;
          if (is_spill(cls)) spillize(d);
          break;
        }
        case Vinstr::copyargs: {
          // Propagate "spillness" across copy
          auto const& uses = unit.tuples[inst.copyargs_.s];
          auto const& defs = unit.tuples[inst.copyargs_.d];
          assertx(uses.size() == defs.size());
          for (size_t i = 0; i < uses.size(); ++i) {
            assertx(uses[i].isPhys() == defs[i].isPhys());
            if (uses[i].isPhys()) continue;
            auto const cls = reg_info(state, uses[i]).regClass;
            if (is_spill(cls)) spillize(defs[i]);
          }
          break;
        }
        default:
          break;
      }
    }
  }

  // Everything after this is sanity checking.
  if (!debug) return;

  // Do a pass over the unit making sure that the RegClasses of everything is
  // sane.
  auto const appropriate = [] (RegClass spill, RegClass reg) {
    switch (reg) {
      case RegClass::GP:
      case RegClass::SIMD:
      case RegClass::AnyNarrow:
        return spill == RegClass::Spill;
      case RegClass::SIMDWide:
        return spill == RegClass::SpillWide;
      case RegClass::Any:
      case RegClass::SF:
      case RegClass::Spill:
      case RegClass::SpillWide:
        break;
    }
    always_assert(false);
  };

  for (auto const b : state.rpo) {
    for (auto const& inst : unit.blocks[b].code) {
      switch (inst.op) {
        case Vinstr::spill: {
          // The dest of a spill should be a spill slot, and the source should
          // not be.
          auto const dCls = reg_info(state, inst.spill_.d).regClass;
          auto const sCls = reg_info(state, inst.spill_.s).regClass;
          always_assert(is_spill(dCls));
          always_assert(!is_spill(sCls));
          always_assert(appropriate(dCls, sCls));
          break;
        }
        case Vinstr::reload: {
          // The source of a reload should be a spill slot, and the dest should
          // not be.
          auto const dCls = reg_info(state, inst.reload_.d).regClass;
          auto const sCls = reg_info(state, inst.reload_.s).regClass;
          always_assert(!is_spill(dCls));
          always_assert(is_spill(sCls));
          always_assert(appropriate(sCls, dCls));
          break;
        }
        case Vinstr::phidef: {
          // The source/dest pairs of a phijmp/phidef should always be the same
          // RegClass. There shouldn't be any copying between physical registers
          // and Vregs.
          auto const& defs = unit.tuples[inst.phidef_.defs];
          for (auto const& pred : state.preds[b]) {
            always_assert(unit.blocks[pred].code.back().op == Vinstr::phijmp);
            auto const& uses = unit.tuples[
              unit.blocks[pred].code.back().phijmp_.uses
            ];
            always_assert(defs.size() == uses.size());
            for (size_t i = 0; i < defs.size(); ++i) {
              always_assert(defs[i].isPhys() == uses[i].isPhys());
              if (defs[i].isPhys()) continue;
              auto const dCls = reg_info(state, defs[i]).regClass;
              auto const uCls = reg_info(state, uses[i]).regClass;
              always_assert(dCls == uCls);
            }
          }
          break;
        }
        case Vinstr::phijmp:
          // Handled as part of phidef
          break;
        case Vinstr::copy: {
          // The source and dest of a copy should both be spill slots or both
          // should not be, never a mix. The widths of the spill slots should be
          // the same.
          auto const s = inst.copy_.s;
          auto const d = inst.copy_.d;
          if (s.isPhys()) {
            always_assert(
              d.isPhys() || !is_spill(reg_info(state, d).regClass)
            );
            break;
          }
          if (d.isPhys()) {
            always_assert(!is_spill(reg_info(state, s).regClass));
            break;
          }
          auto const dCls = reg_info(state, d).regClass;
          auto const uCls = reg_info(state, s).regClass;
          always_assert(is_spill(dCls) == is_spill(uCls));
          always_assert(IMPLIES(is_spill(dCls), dCls == uCls));
          break;
        }
        case Vinstr::copyargs: {
          // Same as Vinstr::copy, but with a tuple
          auto const& uses = unit.tuples[inst.copyargs_.s];
          auto const& defs = unit.tuples[inst.copyargs_.d];
          always_assert(uses.size() == defs.size());
          for (size_t i = 0; i < uses.size(); ++i) {
            always_assert(uses[i].isPhys() == defs[i].isPhys());
            if (uses[i].isPhys()) continue;
            auto const dCls = reg_info(state, defs[i]).regClass;
            auto const uCls = reg_info(state, uses[i]).regClass;
            always_assert(is_spill(dCls) == is_spill(uCls));
            always_assert(IMPLIES(is_spill(dCls), dCls == uCls));
          }
          break;
        }
        default:
          // For most instructions, both the sources and dests should not be
          // spill slots.
          visitUses(
            unit, inst,
            [&] (Vreg r) {
              if (r.isPhys()) return;
              always_assert(!is_spill(reg_info(state, r).regClass));
            }
          );
          visitDefs(
            unit, inst,
            [&] (Vreg r) {
              if (r.isPhys()) return;
              always_assert(!is_spill(reg_info(state, r).regClass));
            }
          );
          break;
      }
    }
  }
}

void insert_spills(State& state) {
  find_materialization_candidates(state);
  calculate_liveness(state);
  calculate_loop_info(state);
  calculate_spill_weights(state);

  auto results = process_spills(state);
  fixup_spill_mismatches(state, results);

  auto const mappings = restoreSSA(state.unit, results.ssaize, state.rpo);
  for (auto const& map : mappings) {
    reg_info_insert(
      state,
      map.first,
      reg_info(state, map.second)
    );
  }

  set_spill_reg_classes(state, results, mappings);

  // Rematerialization may have created dead code, so remove it now.
  removeDeadCode(state.unit);

  assertx(check(state.unit));
}

//////////////////////////////////////////////////////////////////////
// Coloring

/*
 * Compared to spilling, coloring is rather straightforward. As already
 * mentioned, now that the spiller has lowered the register pressure everywhere
 * to below the number of physical registers, the unit should be trivially
 * colorable. To color the unit, all we have to do is visit the blocks in any
 * dominance preserving order (we use RPO) and choose a free color for each Vreg
 * that the instruction defines. Its guaranteed that they'll always be a free
 * color, regardless of previous choices. Thus coloring can be accomplished in a
 * single linear pass. Once a Vreg becomes dead, its color is released.
 *
 * We only color physical registers here. We defer assigning spill slots to the
 * optimization phase because its fairly trivial to use that to assign the slots
 * optimally.
 *
 * Constrained instructions are the only real complexity, since we do not have
 * the freedom to choose arbitrary colors for them. Instead, we rely on the fact
 * that we split all the live ranges of Vregs before each constrained
 * instruction. Therefore we can assume that all colors are free and choose the
 * colors for the uses and defs of the constrained instruction as dictated by
 * their precolors. Once we have selected the colors for the constrained
 * instruction, we then color the preceding copy instruction which broke the
 * live range. The copy isn't constrained, so we can select colors for
 * unassigned Vregs from the set that wasn't used by the constrained
 * instruction.
 *
 * This logic only works if each RegClass can only select from non-overlapping
 * sets of physical registers. If there's overlapping pools of physical
 * registers, it introduces additional constraints which the colorer cannot
 * satisfy. Namely, assume that RegClass::AnyNarrow choose from both GP and SIMD
 * registers. We want to color a particular register with RegClass::Any, so we
 * select a GP register (as opposed to a SIMD). It happens to be the last GP
 * register. Later on, we attempt to color a RegClass::GP register, but we can't
 * because there's none free. If instead we had selected a SIMD earlier, it
 * would have been colorable. In other words, having overlapping physical
 * register pools means you no longer have a free choice for colors. So, we have
 * to restrict RegClass::AnyNarrow to a single register pool, namely GP.
 *
 * We select the free colors arbitrarily. Note that while the algorithm
 * guarantees the unit is colorable regardless of choice, it does not guarantee
 * the choices will result in a good coloring. Indeed, most colorings are pretty
 * bad. We rely on a separate optimization of the colorings afterwards to remove
 * most of the copies.
 */

// FreeRegs is responsible for tracking which physical registers are free and
// assigning them. It only deals with physical registers and not spill slots.

struct FreeRegs {
  explicit FreeRegs(const State& state)
    : state{&state}
    , gp{state.gpUnreserved}
    , simd{state.simdUnreserved} {}

  // Choose (with no particular heuristics) a free physical register appropriate
  // for the given register class and return it as a color (or None if there's
  // none available). This merely chooses the register, and does not mark it as
  // taken.
  Color choose(RegClass cls) const {
    switch (cls) {
      case RegClass::GP:
      case RegClass::AnyNarrow: {
        auto const r = gp.choose();
        return r == InvalidReg ? None{} : Color{r};
      }
      case RegClass::SIMD:
      case RegClass::SIMDWide: {
        auto const r = simd.choose();
        return r == InvalidReg ? None{} : Color{r};
      }
      case RegClass::Any:
      case RegClass::Spill:
      case RegClass::SpillWide:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Mark the given physical register as taken. The register should not already
  // be reserved. The RegClass should be appropriate for that register.
  void reserve(PhysReg r, RegClass cls) {
    switch (cls) {
      case RegClass::GP:
      case RegClass::AnyNarrow: {
        assertx(gp.contains(r));
        gp -= r;
        return;
      }
      case RegClass::SIMD:
      case RegClass::SIMDWide: {
        assertx(simd.contains(r));
        simd -= r;
        return;
      }
      case RegClass::Any:
      case RegClass::Spill:
      case RegClass::SpillWide:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Mark the given physical register as available. The register should be
  // already marked as taken. The RegClass should be appropriate for that
  // register.
  void release(PhysReg r, RegClass cls) {
    switch (cls) {
      case RegClass::GP:
      case RegClass::AnyNarrow: {
        assertx(!gp.contains(r));
        gp |= r;
        return;
      }
      case RegClass::SIMD:
      case RegClass::SIMDWide: {
        assertx(!simd.contains(r));
        simd |= r;
        return;
      }
      case RegClass::Any:
      case RegClass::Spill:
      case RegClass::SpillWide:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Mark all physical registers as available.
  void releaseAll() {
    gp = state->gpUnreserved;
    simd = state->simdUnreserved;
  }

  // Return false if any physical registers are taken.
  bool allAvailable() const {
    return
      gp == state->gpUnreserved &&
      simd == state->simdUnreserved;
  }

  // Check if the given physical register (with appropriate RegClass) is taken.
  bool available(PhysReg r, RegClass cls) const {
    switch (cls) {
      case RegClass::GP:
      case RegClass::AnyNarrow:
        return gp.contains(r);
      case RegClass::SIMD:
      case RegClass::SIMDWide:
        return simd.contains(r);
      case RegClass::Any:
      case RegClass::Spill:
      case RegClass::SpillWide:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Set operations:

  FreeRegs operator-(const FreeRegs& other) const {
    assertx(state == other.state);
    auto temp = *this;
    temp.gp -= other.gp;
    temp.simd -= other.simd;
    return temp;
  }

  FreeRegs operator&(const FreeRegs& other) const {
    assertx(state == other.state);
    auto temp = *this;
    temp.gp &= other.gp;
    temp.simd &= other.simd;
    return temp;
  }

private:
  const State* state;
  RegSet gp;
  RegSet simd;
};

// Helper function to assert that a Color is not None (which means we couldn't
// find a free one, which is a bug).
void assert_found_color(Vreg r, Color c) {
  always_assert_flog(
    !is_color_none(c),
    "Unable to find free color for {}",
    show(r)
  );
}

// Helper function to assert that the precolor for a particular Vreg is
// available (if not, its a bug).
void assert_precolor_avail(const FreeRegs& regs, const RegInfo& info, Vreg r) {
  always_assert_flog(
    regs.available(info.precolor, info.regClass),
    "{} is pre-colored to {}, but it is not available",
    show(r), show(info.precolor)
  );
}

// Release the allocated colors from any Vregs (from the given candidate set)
// which are dead at the given position.
void release_dead_regs(const State& state, FreeRegs& free,
                       const VregSet& candidates,
                       Vlabel block, size_t instIdx) {
  candidates.forEach(
    [&] (Vreg r) {
      if (r.isPhys()) return;
      auto const& info = reg_info(state, r);
      if (!is_colorable(info.regClass)) return;
      // Since we walk the unit in a dominance preserving order, every Vreg
      // defined at this point should have a color assigned.
      assertx(!is_color_none(info.color));
      auto const reg = color_reg(info.color);
      assertx(!free.available(reg, info.regClass));
      if (live_in_at(state, r, block, instIdx + 1)) return;
      free.release(reg, info.regClass);
    }
  );
}

// Color the defs of the given instruction (which is unconstrained) at the given
// position.
void color_unconstrained(State& state, FreeRegs& free,
                         const Vinstr& inst, Vlabel block,
                         size_t instIdx) {
  assertx(!is_constrained_inst(state, inst));

  auto const& unit = state.unit;

  auto const acrosses = acrossesSet(unit, inst) - state.reservedRegs;
  auto const uses = usesSet(unit, inst) - state.reservedRegs - acrosses;
  auto const defs = defsSet(unit, inst) - state.reservedRegs;

  // Make sure the uses of the instruction are all colored already (which should
  // be the case because we walk the unit in dominance preserving order).
  if (debug) {
    auto const checkColored = [&] (Vreg r) {
      auto const& info = reg_info(state, r);
      if (!is_colorable(info.regClass)) return;
      always_assert(!is_color_none(info.color));
    };
    uses.forEach(checkColored);
    acrosses.forEach(checkColored);
  }

  // First release any colors held by now dead uses. This doesn't include
  // acrosses, which we removed from the uses set above.
  release_dead_regs(state, free, uses, block, instIdx);

  defs.forEach(
    [&] (Vreg r) {
      auto& info = reg_info(state, r);
      // This Vreg is being defined here, so it better not have a color already.
      if (!is_colorable(info.regClass)) return;
      assertx(is_color_none(info.color));
      // Pick a color, assert we found something (which we always should) and
      // then reserve it.
      info.color = free.choose(info.regClass);
      assert_found_color(r, info.color);
      free.reserve(color_reg(info.color), info.regClass);
    }
  );

  // Release any now dead defs or acrosses.
  release_dead_regs(state, free, acrosses, block, instIdx);
  release_dead_regs(state, free, defs, block, instIdx);
}

// Color constrained instructions is a bit more complicated because we need to
// ensure that Vregs with precolors are colored to the appropriate physical
// register. We might have inserted a copy/copyargs instruction immediately
// before the constrained instruction to break Vreg live ranges. If so, we need
// to color both the copy and the constrained instruction simultaneously.
void color_constrained(State& state, FreeRegs& finalFree,
                       const Vinstr& inst, const Vinstr* copy,
                       Vlabel block, size_t firstIdx) {
  assertx(is_constrained_inst(state, inst));
  assertx(!copy ||
          copy->op == Vinstr::copy ||
          copy->op == Vinstr::copyargs);
  assertx(!copy || !is_constrained_inst(state, *copy));

  auto const& unit = state.unit;

  // Index of the constrained instruction
  auto const instIdx = firstIdx + (copy ? 1 : 0);

  // finalFree is the state of free registers coming into the copy/constrained
  // instruction pair. If there's no copy, its already empty. If there's a copy,
  // it won't be, but we'll adjust it after coloring the copy instruction.
  auto usesFree = finalFree;
  auto defsFree = finalFree;
  // Start out by assuming all physical registers are available. This is safe to
  // assume because the copy/copyargs (if necessary) broke the live ranges of
  // all Vregs, giving us complete freedom to reassign things.
  usesFree.releaseAll();
  defsFree.releaseAll();

  auto uses = usesSet(unit, inst) - state.reservedRegs;
  auto acrosses = acrossesSet(unit, inst) - state.reservedRegs;
  auto const defs = defsSet(unit, inst) - state.reservedRegs;

  // Just like with spilling, we might need to treat some uses as acrosses to
  // model register pressure properly.
  fix_constrained_inst_uses(state, defs, uses, acrosses, block, instIdx);

  // We color the constrained instruction first, not the copy. Once we color the
  // constrained instruction, we then color the copy with what is left. This
  // ensures the colorability.

  // Does the Vreg live thru the instruction? IE, is it an across, or live-out?
  auto const isLiveThru = [&] (Vreg r) {
    return acrosses[r] || live_in_at(state, r, block, instIdx + 1);
  };

  // Assign colors to the precolored subset of the given Vregs, using "free" to
  // choose available colors. If "other" is provided, and the Vreg is live-thru
  // the instruction, then also mark it as reserved in "other".
  auto const constrained = [&] (const VregSet& regs,
                                FreeRegs& free,
                                FreeRegs* other) {
    regs.forEach(
      [&] (Vreg r) {
        auto& info = reg_info(state, r);
        assertx(!is_spill(info.regClass));
        if (info.precolor == InvalidReg) return;
        if (!is_colorable(info.regClass)) return;

        assertx(is_color_none(info.color));
        assert_precolor_avail(free, info, r);
        info.color = info.precolor;
        free.reserve(info.precolor, info.regClass);

        if (!other) return;
        if (!isLiveThru(r)) return;

        assert_precolor_avail(*other, info, r);
        other->reserve(info.precolor, info.regClass);
      }
    );
  };
  // Color the precolored uses, also reserving them in defsFree if live-thru.
  constrained(uses, usesFree, &defsFree);
  // Color the precolored defs, not using any colors used by live-thru Vregs in
  // the uses.
  constrained(defs, defsFree, nullptr);

  // Assign colors to the non-precolored subset of the given Vregs. Use "free"
  // to select Vregs, preferring Vregs not taken in "avoid"
  auto const unconstrained = [&] (const VregSet& regs,
                                  FreeRegs& free,
                                  FreeRegs& avoid,
                                  bool skipLiveThru) {
    regs.forEach(
      [&] (Vreg r) {
        auto& info = reg_info(state, r);
        assertx(!is_spill(info.regClass));
        if (info.precolor != InvalidReg) return;
        if (info.regClass == RegClass::SF) return;
        assertx(is_color_none(info.color));

        if (skipLiveThru && isLiveThru(r)) return;

        // Choose a color. First pick among the Vregs available in free, but
        // taken in avoid. If we can't find a color there, just choose from free
        // (which should always succeed).
        auto const color = [&]{
          auto const preferred = (free - avoid).choose(info.regClass);
          if (!is_color_none(preferred)) return preferred;
          return free.choose(info.regClass);
        }();
        assert_found_color(r, color);

        info.color = color;
        free.reserve(color_reg(color), info.regClass);
      }
    );
  };
  // Color the non-precolored uses which aren't live-thru. Try to avoid free
  // registers available for defs.
  unconstrained(uses, usesFree, defsFree, true);
  // Color the non-precolored defs. Try to avoid free registers available for
  // uses.
  unconstrained(defs, defsFree, usesFree, false);

  // Finally color the non-precolored uses which are live-thru. Use the
  // registers which are free in both usesFree and defsFree (which should always
  // exist).
  uses.forEach(
    [&] (Vreg r) {
      auto& info = reg_info(state, r);
      assertx(!is_spill(info.regClass));
      if (info.precolor != InvalidReg) return;
      if (info.regClass == RegClass::SF) return;
      if (!isLiveThru(r)) return;

      assertx(is_color_none(info.color));

      auto const color = (usesFree & defsFree).choose(info.regClass);
      assert_found_color(r, color);

      info.color = color;
      usesFree.reserve(color_reg(color), info.regClass);
      defsFree.reserve(color_reg(color), info.regClass);
    }
  );

  // Now that we've colored the constrained instruction, color the copy (if it
  // exists). Since the copy is unconstrained, we can color its defs any way we
  // want.
  if (copy) {
    assertx(acrossesSet(unit, *copy).none());

    auto const copyUses = usesSet(unit, *copy) - state.reservedRegs;
    if (debug) {
      copyUses.forEach(
        [&] (Vreg r) {
          auto const& info = reg_info(state, r);
          if (!is_colorable(info.regClass)) return;
          always_assert(!is_color_none(info.color));
        }
      );
    }

    FreeRegs copyDefsFree{state};

    visitDefs(
      unit, *copy,
      [&] (Vreg r) {
        if (r.isPhys()) return;

        auto& info = reg_info(state, r);
        if (!is_colorable(info.regClass)) return;

        // If this def was used by the constrained instruction, its already been
        // colored. Reserve the color its been assigned.
        if (!is_color_none(info.color)) {
          auto const reg = color_reg(info.color);
          always_assert(copyDefsFree.available(reg, info.regClass));
          copyDefsFree.reserve(reg, info.regClass);
          return;
        }

        // This def isn't assigned a color yet, which means it wasn't used by
        // the constrained instruction. If its live-in to the constrained
        // instruction, we have to assign it a color which isn't used by the
        // constrained instruction at all (and not used by the copy either).
        if (live_in_at(state, r, block, instIdx)) {
          info.color =
            (copyDefsFree & defsFree & usesFree).choose(info.regClass);
          assert_found_color(r, info.color);
          auto const reg = color_reg(info.color);
          copyDefsFree.reserve(reg, info.regClass);
          defsFree.reserve(reg, info.regClass);
          usesFree.reserve(reg, info.regClass);
          return;
        }

        // This def is dead after the copy. It won't interfere with the
        // constrained instruction, so we can give it any color that isn't
        // already used by the copy. However, we'd like to give it a color which
        // isn't used by the constrained instruction to give other Vregs maximum
        // freedom in their choices.
        auto const color = [&]{
          // First try a color which isn't used anywhere
          auto const preferred1 =
            (copyDefsFree - usesFree - defsFree).choose(info.regClass);
          if (!is_color_none(preferred1)) return preferred1;
          // Then try one which isn't used by the copy or the constrained
          // instruction's uses.
          auto const preferred2 =
            (copyDefsFree - usesFree).choose(info.regClass);
          if (!is_color_none(preferred2)) return preferred2;
          // If that fails, just choose one not used by the copy (which is the
          // bare minimum required).
          return copyDefsFree.choose(info.regClass);
        }();
        assert_found_color(r, color);
        info.color = color;
        copyDefsFree.reserve(color_reg(color), info.regClass);
      }
    );

    // Release any uses of the copy which are now dead from the original
    // FreeRegs (reflecting the state before the copy and constrained
    // instruction). Since the point of the copy was to break all the Vreg live
    // ranges, this should release all physical registers.
    release_dead_regs(state, finalFree, copyUses, block, firstIdx);
  }

  // At this point finalFree reflects the register allocation state after the
  // copy (if any) and before the constrained instruction. Since its a
  // pre-requisite for proper coloring that all physical registers are available
  // for selection before a constrained instruction, we can assert the state has
  // everything available. Either there was no copy, and there were no live
  // Vregs, or there was a copy and we just released everything above.
  always_assert(finalFree.allAvailable());

  // The new state is whats free after the defs.
  finalFree = defsFree;
  // Release any now dead acrosses or defs after the constrained instruction.
  release_dead_regs(state, finalFree, acrosses, block, instIdx);
  release_dead_regs(state, finalFree, defs, block, instIdx);
}

void assign_colors(State& state) {
  auto const& unit = state.unit;

  // Walk the unit in RPO order. This is dominance preserving, so we'll always
  // encounter a Vreg's def before any of its usages. This means we can color in
  // a single pass over the unit.
  for (auto const b : state.rpo) {
    // Start with all physical registers being available.
    FreeRegs free{state};

    // Then reserve all the registers already assigned to live-in Vregs.
    state.liveIn[b].forEach(
      [&] (Vreg r) {
        auto const& info = reg_info(state, r);
        if (!is_colorable(info.regClass)) return;
        assertx(!is_color_none(info.color));
        auto const reg = color_reg(info.color);
        always_assert(free.available(reg, info.regClass));
        free.reserve(reg, info.regClass);
      }
    );

    auto const& block = unit.blocks[b];
    for (size_t i = 0; i < block.code.size(); ++i) {
      auto const& inst = block.code[i];

      if (inst.op == Vinstr::copy || inst.op == Vinstr::copyargs) {
        // Constrained instructions need special coloring logic to ensure that
        // the precolors are satisfied. A constrained instruction may have a
        // copy/copyargs in front of it (to break Vreg live ranges) and we want
        // to color both simultaneously as a pair. So, if we have a
        // copy/copyargs, peak ahead and see if the next is a constrained
        // instruction.
        assertx(!is_constrained_inst(state, inst));
        assertx(i + 1 < block.code.size());

        auto const& next = block.code[i+1];
        if (is_constrained_inst(state, next)) {
          // The next is a constrained instruction, color both as a pair.
          color_constrained(state, free, next, &inst, b, i);
          i++; // Skip over the next.
        } else {
          // Its not. Color this copy normally and we'll deal with the next the
          // next trip around.
          color_unconstrained(state, free, inst, b, i);
        }
      } else if (is_constrained_inst(state, inst)) {
        // A constrained instruction is not guaranteed to have a copy/copyargs
        // in front of it (if nothing was live), so handle that case here.
        assertx(inst.op != Vinstr::copy && inst.op != Vinstr::copyargs);
        color_constrained(state, free, inst, nullptr, b, i);
      } else {
        // Normal unconstrained instruction.
        color_unconstrained(state, free, inst, b, i);
      }
    }
  }

  assertx(check(state.unit));
}

//////////////////////////////////////////////////////////////////////
// Color optimization

/*
 * We've colored the unit (except for spills), but the coloring may not be very
 * good. In fact, its usually pretty bad because the color selection is
 * arbitrary. We now attempt to optimize the colors so that we can minimize the
 * number of copies between different registers (and maximize the number of
 * instructions whose hints are satisfied).
 *
 * Note that once the coloring phase is done, we *always* have a valid coloring
 * for the unit. This pass will attempt to optimize the coloring, but at all
 * times the coloring is valid. This means that, in principal, you can stop at
 * any point and have a valid unit.
 *
 * Color optimization is NP-hard, even for SSA programs, so we use a greedy
 * algorithm which works well in practice:
 *
 * - First we construct affinities. An affinity is a pair of Vregs which are
 *   joined by a hint on an instruction (this includes copy instructions), or a
 *   phi. Each affinity has a score. The higher the score is, the more
 *   profitable it is to satisfy the affinity by assigning the Vregs the same
 *   register. Right now the score is just calculated statically from the
 *   block's coldness.
 *
 * - We then group affinities together into affinity chunks. An affinity chunk
 *   is a group of Vregs which all don't interfere with each other and whose has
 *   at least one affinity with another Vreg in the same chunk. The score of the
 *   chunk is the sum of all the contained affinities. An affinity is contained
 *   within the chunk if both Vregs of the affinity are in the chunk. Since the
 *   Vregs in the chunk don't interfere with each other, they can all be
 *   assigned the same color. Since they have affinities with each other, if
 *   they are assigned the same color, you'll profit by eliminating the chunk's
 *   score (all the affinities in the chunk will be automatically satisfied).
 *
 * - Affinity chunks are constructed via a greedy algorithm. First every Vreg is
 *   placed into its own singleton chunk (which therefore has no affinities). We
 *   sort the affinities by their score (higher affinities first). For each
 *   affinity, if its Vregs are in different chunks, and those chunks do not
 *   interfere, we merge them together. This will try to generate chunks with
 *   maximal scores (since affinities with greater scores are considered
 *   first). As the chunks grow, its more and more likely they'll interfere with
 *   another chunk.
 *
 * - The chunks are placed in a priority queue, chunks with higher scores
 *   first. For each chunk, we attempt to find the best register to re-color
 *   that chunk to. That is, if you attempt to re-color every Vreg in the chunk
 *   to that register, what is the sum of the scores of the affinities who are
 *   now satisfied? Calculate that for every register and use the one with the
 *   highest score. That register is the one we re-color the chunk to. It may
 *   not be possible to re-color all Vregs in the chunk to that color (because
 *   of interference with neighbors). The portion that cannot be re-colored is
 *   split into a new chunk and re-inserted into the priority queue. This
 *   process repeats until the queue is empty.
 *
 * - The re-coloring is done recursively. First we change the color of the Vreg
 *   to the desired register. We then iterate over the Vreg's interference
 *   neighbors and attempt to re-color them to a different register (if
 *   necessary). This might require those Vregs' neighbors to be re-colored to a
 *   different color, etc. This is why its called recursive re-coloring. To
 *   avoid infinite recursion, once we set a color for a Vreg, we mark it as
 *   "fixed". This is, it cannot be changed again until the entire recursion
 *   finishes.
 *
 * - Spill slots are handled a bit differently. We didn't bother coloring them
 *   in the coloring phase. Instead we turn them into chunks like above. That
 *   is, every spill Vreg in the same chunk can be assigned the same slot
 *   because they don't interfere. We then merge chunks that don't interfere,
 *   even if there's no affinity between them. This minimize the total number of
 *   chunks. We then assign a unique slot to each chunk and assign all Vregs in
 *   the same chunk that slot.
 */

// Return true if position p1 dominates position p2.
bool dominates(const State& state, const Position& p1, const Position& p2) {
  // If they're in the same block, an earlier index always dominates a later.
  if (p1.block == p2.block) return p1.index <= p2.index;
  // Otherwise use the standard dominance check.
  return dominates(p1.block, p2.block, state.idoms);
}

// Return true if two Vregs interfere. That is, they're both alive at some
// position. In strict SSA form this can be calculated using dominance relations
// and liveness information rather than having to build a full interference
// graph.
bool vregs_interfere(const State& state, Vreg r1, Vreg r2) {
  assertx(!r1.isPhys());
  assertx(!r2.isPhys());

  auto i1 = &reg_info(state, r1);
  auto i2 = &reg_info(state, r2);
  // If they're defined at the same point, they obviously interfere.
  if (i1->def == i2->def) return true;

  if (dominates(state, i2->def, i1->def)) {
    // Ensure the def of r1 dominates the def of r2
    std::swap(r2, r1);
    std::swap(i2, i1);
  } else if (!dominates(state, i1->def, i2->def)) {
    // If neither dominates the other, they cannot be simultaneously alive.
    return false;
  }

  // We know that the def of r1 dominates the def of r2. This implies that r1 is
  // defined in the same block as r2, or in a block leading to the def of r2. If
  // r1 is also live-out of r2's def block, then it means that r1's lifetime
  // crosses r2's, so they interfere.
  if (state.liveOut[i2->def.block][r1]) return true;

  // Otherwise check every use of r1 and see if r2 dominates any of them. The
  // def of r1 dominates the def of r2. If there's a usage of r1 which is
  // dominated by r2, that means any path to that usage must pass through r2's
  // def. That implies that r1 must be alive at the same time as r2.
  for (auto const& use : i1->uses) {
    if (dominates(state, i2->def, use)) return true;
  }

  return false;
}

// Return all the Vregs that a particular Vreg interferes with. This uses
// caching to avoid doing a (potentially expensive) recalculation.
const VregSet& find_interferences(State& state, Vreg r) {
  auto& info = reg_info(state, r);
  assertx(!is_spill(info.regClass));
  assertx(info.regClass != RegClass::SF);

  // We already have the information, so we're done.
  if (info.neighbors) return *info.neighbors;

  boost::dynamic_bitset<> visited(state.unit.blocks.size());
  VregSet interferences;

  // Walk the block backwards, modeling liveness information. Whenever r is
  // alive, record all other live Vregs.
  auto const find = [&] (Vlabel b, auto const& self) {
    assertx(!visited[b]);
    visited[b] = true;

    // Start with the already stored liveness information
    auto live = state.liveOut[b];

    auto const add = [&] (Vreg a) {
      if (a.isPhys()) return;
      if (reg_info(state, a).regClass == RegClass::SF) return;
      live.add(a);
    };
    auto const remove = [&] (Vreg a) { live.remove(a); };
    auto const update = [&] {
      if (live[r]) interferences |= live;
    };

    // And then walk backwards modifying it on the fly.
    auto const& block = state.unit.blocks[b];
    for (auto const& inst : boost::adaptors::reverse(block.code)) {
      visitDefs(state.unit, inst, add);
      visitAcrosses(state.unit, inst, add);
      update();
      visitDefs(state.unit, inst, remove);
      visitUses(state.unit, inst, add);
      update();
    }

    // What we've calculated for liveness should match the already stored
    // per-block liveness information at this point.
    assertx(live == state.liveIn[b]);
    if (!live[r]) {
      // We only processed this block if r had a usage in it. If r is no longer
      // live, this should be where its defined.
      assertx(info.def.block == b);
      return;
    }

    // Recurse to all predecessors of this block which are dominated by the
    // definition (if they're not, r cannot be alive in them).
    for (auto const pred : state.preds[b]) {
      if (dominates(info.def.block, pred, state.idoms) && !visited[pred]) {
        self(pred, self);
      }
    }
  };

  // Visit all blocks where there's a use and record the live Vregs.
  for (auto const& p : info.uses) {
    if (!visited[p.block]) find(p.block, find);
  }
  // The def as well.
  if (!visited[info.def.block]) find(info.def.block, find);

  // Record the calculated information for re-use.
  interferences.remove(r);
  info.neighbors = std::move(interferences);
  return *info.neighbors;
}

// Like an instruction, a Vreg is constrained if it has a precolor and its used
// or defined in a non-copy instruction.
bool is_constrained_vreg(const State& state, Vreg r) {
  auto const& info = reg_info(state, r);
  if (info.precolor == InvalidReg) return false;

  auto const check = [&] (const Position& pos) {
    auto const& block = state.unit.blocks[pos.block];
    auto const& inst = block.code[pos.index / 2];
    switch (inst.op) {
      case Vinstr::copy:
      case Vinstr::copyargs:
      case Vinstr::phidef:
      case Vinstr::phijmp:
      case Vinstr::spill:
      case Vinstr::reload:
        return false;
      default:
        return true;
    }
  };

  if (check(info.def)) return true;
  return std::any_of(info.uses.begin(), info.uses.end(), check);
}

// Return "weight" of a block, an approximation of how many times it will be
// executed. The higher the weight of a block, the more profitable it is to
// remove copies in it.
size_t block_weight(const State& state, Vlabel b) {
  // Right now use hard-coded weights based on the area.  TODO (T37650402):
  // Incorporate profiling information
  auto const base = [&]{
    switch (state.unit.blocks[b].area_idx) {
      case AreaIndex::Main:   return 1000;
      case AreaIndex::Cold:   return 10;
      case AreaIndex::Frozen: return 1;
    }
    always_assert(false);
  }();

  // Cap the max loop depth to avoid numerical issues.
  static constexpr size_t maxLoopDepth = 5;

  // Adjust the base score by the loop nesting depth (we want to eliminate
  // copies in inner loops more than ones outside of them). Real profiling
  // information may make this unnecessary.
  auto const depth = std::min(block_loop_depth(state, b), maxLoopDepth);
  return std::pow(100, depth) * base;
}

// An affinity is a pair of Vregs joined by a copy instruction or a hint. The
// score indicates how profitable it is to eliminate this copy by assigning the
// two Vregs the same color (higher is more profitable). An affinity can have an
// invalid second register, which means it exists just to record the existance
// of that Vreg. This is only needed for spilled Vregs to ensure every such Vreg
// is assigned a slot.
struct Affinity {
  Vreg r1;
  Vreg r2;
  size_t score;
};

// Build a list of all affinities in the unit sorted from biggest score to
// smallest.
jit::vector<Affinity> build_affinities(const State& state) {
  auto const& unit = state.unit;

  jit::vector<Affinity> affinities;

  // A "singleton" is an affinity with a single Vreg in it. We only need these
  // for spilled Vregs to ensure we assign spill slots for all of them (a
  // non-spilled Vreg with no affinity can be ignored since its already assigned
  // color is fine).
  auto const singleton = [&] (Vreg r) {
    if (!r.isValid() || r.isPhys()) return;
    if (!is_spill(reg_info(state, r).regClass)) return;
    affinities.push_back(Affinity{r, Vreg{}, 0});
  };

  auto const add = [&] (Vreg r1, Vreg r2, size_t score) {
    // If the two Vregs aren't compatible (in the sense that its not possible to
    // choose colors to remove the copy) don't record them as a pair.
    if (!r1.isValid() || r1.isPhys()) return singleton(r2);
    if (!r2.isValid() || r2.isPhys()) return singleton(r1);

    auto const& i1 = reg_info(state, r1);
    auto const& i2 = reg_info(state, r2);

    // If the Vregs interfere they can never be assigned the same color, or if
    // they're constrained and have different precolors.
    if (!compatible_reg_classes(i1.regClass, i2.regClass) ||
        i1.regClass == RegClass::SF ||
        i2.regClass == RegClass::SF ||
        vregs_interfere(state, r1, r2) ||
        (is_constrained_vreg(state, r1) &&
         is_constrained_vreg(state, r2) &&
         i1.precolor != i2.precolor)) {
      singleton(r1);
      singleton(r2);
      return;
    }

    affinities.push_back(Affinity{r1, r2, score});
  };

  for (auto const b : state.rpo) {
    auto const blockWeight = block_weight(state, b);

    for (auto const& inst : unit.blocks[b].code) {
      // Phis need to be considered specially since their sources and dests
      // aren't in the same instruction.
      if (inst.op == Vinstr::phidef) {
        auto const& d = unit.tuples[inst.phidef_.defs];
        for (auto const pred : state.preds[b]) {
          assertx(unit.blocks[pred].code.back().op == Vinstr::phijmp);
          auto const& phijmp = unit.blocks[pred].code.back().phijmp_;
          auto const& s = unit.tuples[phijmp.uses];
          assertx(s.size() == d.size());
          // For phis we want to use the weight of the predecessor (so we prefer
          // the hot parts of the phi).
          auto const predWeight = block_weight(state, pred);
          for (size_t i = 0; i < s.size(); ++i) add(s[i], d[i], predWeight);
        }
        continue;
      }

      // The sources and dests of copies are considered hints, so this covers
      // this as well. The score of the affinity is just the weight of the block
      // the instruction is in.
      visitDefsWithHints(
        unit, inst,
        [&] (Vreg r, Vreg hint) { add(hint, r, blockWeight); }
      );
    }
  }

  std::sort(
    affinities.begin(), affinities.end(),
    [](const Affinity& a1, const Affinity& a2) {
      if (a1.score > a2.score) return true;
      if (a1.score < a2.score) return false;
      return std::tie(a1.r1, a1.r2) < std::tie(a2.r1, a2.r2);
    }
  );

  return affinities;
}

// An AffinityChunk is a group of Vregs and affinities between them such that no
// Vreg within interferes with any other. Therefore all the Vregs in a chunk can
// be colored the same color.
struct AffinityChunk {
  AffinityChunk() = default;
  AffinityChunk(size_t index, Vreg r, RegClass cls, PhysReg constraint)
    : cls{cls}
    , constraint{constraint}
    , index{index}
    { regs.emplace_back(r); }

  // We only allow Vregs with compatible RegClass in the same chunk (because
  // they have to be able to be colored the same).
  RegClass cls = RegClass::Any;
  // We don't allow Vregs with different precolors in the same chunk (because
  // they can never be colored the same). We do allow Vregs with the same
  // precolor and non-precolored Vregs however.
  PhysReg constraint;
  size_t index = 0; // Convenient index when building the chunks
  size_t score = 0; // Sum of all the scores of contained affinities
  VregList regs; // All Vregs in chunk, sorted by dominance tree order
  jit::vector<Affinity> affinities;

  // Check if this chunk interferes with another chunk. Two chunks interfere if
  // any Vreg in one interferes with any in another.
  bool interferes(const State& state, const AffinityChunk& o) const {
    // Naive version of the interference check. Useful for debugging, but way
    // too slow even for debug builds by default.
    auto const DEBUG_ONLY naive = [&] (bool expect) {
      if (false) {
        for (auto const r1 : regs) {
          for (auto const r2 : o.regs) {
            if (vregs_interfere(state, r1, r2)) return expect;
          }
        }
        return !expect;
      } else {
        return true;
      }
    };

    /*
     * The naive way do this is to check every Vreg in *this against every Vreg
     * in o, which is O(N^2) comparisons. This is a clever algorithm which takes
     * into account dominance relationships to make it a linear check. See
     * "Revisiting Out-of-SSA Translation for Correctness, Code Quality, and
     * Efficiency" by Benoit Boissinot, Alain Darte, Fabrice Rastello, Benoit
     * Dupont de Dinechin, Christophe Guillon for an explanation of how it
     * works.
     */

    jit::stack<Vreg> domStack;
    size_t i = 0;
    size_t j = 0;
    while (i < regs.size() || j < o.regs.size()) {
      auto const current = [&]{
        if (i == regs.size() ||
            (j < o.regs.size() && compare(state, o.regs[j], regs[i]))) {
          return o.regs[j++];
        } else {
          return regs[i++];
        }
      }();

      auto const parent = [&]{
        while (!domStack.empty()) {
          auto const t = domStack.top();
          if (dominates(state,
                        reg_info(state, t).def,
                        reg_info(state, current).def)) {
            return t;
          }
          domStack.pop();
        }
        return Vreg{};
      }();

      if (parent.isValid() && vregs_interfere(state, current, parent)) {
        assertx(naive(true));
        return true;
      }
      domStack.push(current);
    }

    assertx(naive(false));
    return false;
  }

  // Merge two chunks together, together with an optional affinity. The two
  // chunks should not interfere (nor should the affinity). The optional
  // affinity should "join" each chunk (one Vreg in each).
  void mergeInto(const State& state,
                 const AffinityChunk& o,
                 const Affinity* a) {
    // Since the two chunks cannot have Vregs in common, the result of the merge
    // will be exactly this size.
    regs.resize(regs.size() + o.regs.size());

    auto it1 = regs.rbegin() + o.regs.size();
    auto it2 = o.regs.rbegin();
    auto const end1 = regs.rend();
    auto const end2 = o.regs.rend();
    auto insertIt = regs.rbegin();

    // Standard backwards in-place merge.
    while (true) {
      if (it1 == end1) {
        std::copy(it2, end2, insertIt);
        break;
      }
      if (it2 == end2) {
        std::copy(it1, end1, insertIt);
        break;
      }

      // Maintain dominance tree order when merging.
      if (compare(state, *it1, *it2)) {
        *insertIt = *it2;
        ++it2;
      } else {
        *insertIt = *it1;
        ++it1;
      }
      ++insertIt;
    }

    if (cls == RegClass::Any) cls = o.cls;
    assertx(compatible_reg_classes(cls, o.cls));

    if (constraint != o.constraint) {
      if (constraint == InvalidReg) {
        constraint = o.constraint;
      } else {
        assertx(o.constraint == InvalidReg);
      }
    }

    // Add together the scores and combine the affinities.
    score += o.score;
    affinities.insert(
      affinities.end(), o.affinities.begin(), o.affinities.end()
    );
    if (a) {
      // And add the optional affinity. The affinity's Vregs should already have
      // been in the chunks.
      score += a->score;
      affinities.emplace_back(*a);
    }
    assertx(checkInvariants(state));
  }

  // Split this chunk into a smaller chunk according to the given predicate
  // (called on each Vreg). The original chunk is not changed.
  template <typename F> AffinityChunk split(const State& state, F&& f) const {
    AffinityChunk other;

    for (auto const r : regs) {
      if (!f(r)) continue; // Don't add to new chunk
      other.regs.emplace_back(r); // This maintains the dominance tree order

      auto const& info = reg_info(state, r);
      auto const constraint =
        is_constrained_vreg(state, r) ? info.precolor : InvalidReg;
      assertx(info.regClass != RegClass::SF);

      // Set the RegClass and constraint for the chunk as appropriate.
      if (other.cls == RegClass::Any) other.cls = info.regClass;
      assertx(compatible_reg_classes(other.cls, info.regClass));

      if (other.constraint != constraint) {
        if (other.constraint == InvalidReg) {
          other.constraint = constraint;
        } else {
          assertx(constraint == InvalidReg);
        }
      }
    }

    // Recalculate the score. We only carry over an affinity into the new chunk
    // if both Vregs are in the new chunk.
    other.score = 0;
    for (auto const& affinity : affinities) {
      if (f(affinity.r1) && f(affinity.r2)) {
        other.affinities.emplace_back(affinity);
        other.score += affinity.score;
      }
    }

    assertx(other.checkInvariants(state));
    return other;
  }

  void reset() {
    regs.clear();
    affinities.clear();
    score = 0;
    cls = RegClass::Any;
    constraint = InvalidReg;
  }

  // Sanity checking
  bool checkInvariants(const State& state) const {
    VregSet seen;
    for (size_t i = 0; i < regs.size(); ++i) {
      auto const r = regs[i];
      assertx(r.isValid());
      assertx(!r.isPhys());
      assertx(compatible_reg_classes(reg_info(state, r).regClass, cls));

      // The Vreg list should only have unique Vregs
      assertx(!seen[r]);
      seen.add(r);

      if (false) {
        // Useful for debugging but very expensive, even for debug builds.
        for (size_t j = i+1; j < regs.size(); j++) {
          assertx(!vregs_interfere(state, r, regs[j]));
        }
      }

      if (i+1 >= regs.size()) continue;
      // Verify dominance tree ordering.
      assertx(compare(state, r, regs[i+1]));
    }

    // The chunk's score should be the sum of all of its affinities' scores.
    size_t affinityScore = 0;
    for (auto const& affinity : affinities) {
      assertx(seen[affinity.r1]);
      assertx(seen[affinity.r2]);
      affinityScore += affinity.score;
    }
    assertx(score == affinityScore);

    return true;
  }

private:
  // Helper function to compare according to dominance tree order. Return true
  // if the def of r1 preceeds the def of r2 in the dominator tree.
  static bool compare(const State& state, Vreg r1, Vreg r2) {
    auto const d1 = reg_info(state, r1).def;
    auto const d2 = reg_info(state, r2).def;
    // If they're defined in different blocks, use the pre-calculated dominance
    // tree ordering.
    if (d1.block != d2.block) {
      return state.domOrder[d1.block] < state.domOrder[d2.block];
    }
    // Otherwise whatever one is defined first.
    if (d1.index != d2.index) return d1.index < d2.index;
    // Arbitrary
    return r1 < r2;
  }
};

// Build a set of affinity chunks for the unit. We aim to construct affinity
// chunks with as large a score as possible using a greedy algorithm.
jit::vector<AffinityChunk> build_affinity_chunks(const State& state) {
  // Build the individual affinities
  auto const affinities = build_affinities(state);

  // Map of a Vreg to the chunk it currently occupies, 0 if none.
  jit::vector<size_t> regToChunk(state.regInfo.size());
  jit::vector<AffinityChunk> chunks;
  chunks.emplace_back(); // Since 0 means "no chunk", we reserve a dummy empty
                         // chunk at the first position.

  // First we iterate over all affinities and build a singleton chunk for each
  // unique Vreg we see.
  for (auto const& affinity : affinities) {
    assertx(!affinity.r1.isPhys());

    auto const& i1 = reg_info(state, affinity.r1);
    assertx(i1.regClass != RegClass::SF);

    if (!regToChunk[affinity.r1]) {
      regToChunk[affinity.r1] = chunks.size();
      chunks.emplace_back(
        chunks.size(),
        affinity.r1,
        i1.regClass,
        is_constrained_vreg(state, affinity.r1) ? i1.precolor : InvalidReg
      );
    }

    // If this is a singleton affinity we're done with this one.
    if (!affinity.r2.isValid()) continue;

    assertx(!affinity.r2.isPhys());

    auto const& i2 = reg_info(state, affinity.r2);
    assertx(i2.regClass != RegClass::SF);
    assertx(compatible_reg_classes(i1.regClass, i2.regClass));

    if (!regToChunk[affinity.r2]) {
      regToChunk[affinity.r2] = chunks.size();
      chunks.emplace_back(
        chunks.size(),
        affinity.r2,
        i2.regClass,
        is_constrained_vreg(state, affinity.r2) ? i2.precolor : InvalidReg
      );
    }
  }

  // Retrieve the chunk that a particular Vreg currently resides in. At this
  // point every Vreg should be in a chunk.
  auto const chunk = [&] (Vreg r) -> AffinityChunk& {
    assertx(!r.isPhys());
    auto const idx = regToChunk[r];
    assertx(idx);
    auto& chunk = chunks[idx];
    assertx(chunk.index == idx);
    return chunk;
  };

  jit::vector<jit::fast_set<size_t>> forbid(chunks.size());

  // Check if two chunks are compatible, in the sense that there's nothing
  // stopping us from potentially merging them together. Once two chunks are
  // incompatible, they always will be, so cache that result and avoid duplicate
  // calculation.
  auto const compat = [&] (const AffinityChunk& c1, const AffinityChunk& c2) {
    // Easy checks first
    if (!compatible_reg_classes(c1.cls, c2.cls)) return false;
    if (c1.constraint != c2.constraint) {
      if (c1.constraint != InvalidReg && c2.constraint != InvalidReg) {
        return false;
      }
    }

    // See if we've cached an incompatibility.
    auto const it = forbid[c1.index].find(c2.index);
    if (it != forbid[c1.index].end()) return false;

    // Otherwise do an expensive interference check.
    if (c1.interferes(state, c2)) {
      forbid[c1.index].emplace(c2.index);
      forbid[c2.index].emplace(c1.index);
      return false;
    }
    return true;
  };

  /*
   * Now try to coalesce chunks together maximally. We only coalesce together a
   * chunk if they're compatible and there's an affinity bridging the two. This
   * means the affinity contains a Vreg in one and a Vreg in the other. If
   * there's no affinity between them, its not profitable to merge them because
   * there's no benefit to assigning them the same color.
   *
   * Since the affinities are sorted from greatest score to least, we'll
   * greedily construct chunks with the biggest score.
   */
  for (auto const& affinity : affinities) {
    if (!affinity.r2.isValid()) continue;

    // If they're already in the same chunk or incompatible there's nothing to
    // be done.
    auto& chunk1 = chunk(affinity.r1);
    auto& chunk2 = chunk(affinity.r2);
    if (&chunk1 == &chunk2 || !compat(chunk1, chunk2)) continue;

    // We're going to merge them, update all the bookkeeping to reflect the
    // Vregs now live in the other chunk.
    for (auto const r : chunk2.regs) {
      assertx(regToChunk[r] == chunk2.index);
      regToChunk[r] = chunk1.index;
    }

    // Also keep the forbidden list up to date.
    for (auto const i : forbid[chunk2.index]) {
      forbid[i].emplace(chunk1.index);
      forbid[chunk1.index].emplace(i);
    }

    // Do the actual merge and leave the old chunk empty. Since no Vregs now
    // live in it, we should never look at it again.
    chunk1.mergeInto(state, chunk2, &affinity);
    chunk2.reset();
  }

  // Remove empty chunks, or non-spill singleton chunks. We want to keep spill
  // chunks around because we assign spill slots entirely from the chunks, even
  // if no merging as happened. This invalidates the index information in the
  // chunks, so it cannot be used beyond this point.
  chunks.erase(
    std::remove_if(
      chunks.begin(), chunks.end(),
      [](const AffinityChunk& c) {
        if (is_spill(c.cls)) return c.regs.empty();
        return c.regs.size() <= 1;
      }
    ),
    chunks.end()
  );

  return chunks;
}

// Temporary state used by the re-colorer.
struct RecolorState {
  RegSet available; // Physical registers we have available to chose from

  // Set of Vregs which are "fixed", which means we cannot re-color them. This
  // prevents us from constantly trying to re-color a Vreg we just changed. (We
  // don't use a VregSet because this is more efficient for this particular use
  // case).
  boost::dynamic_bitset<> fixed;

  // The old register for a Vreg before it was last modified.
  jit::vector<PhysReg> old;
  VregSet changed; // Set of Vregs changed during a particular re-color attempt

  // Indicates that a particular Vreg has been set to a register which wasn't
  // its original value. Indicates that the value at the same index in the orig
  // vector is meaningful.
  boost::dynamic_bitset<> notOrig;
  // If notOrig[r] is true, then orig[r] contains the register for r before it
  // was changed. This is similar to old, but orig always contains the original
  // value while old contains the last modified.
  jit::vector<PhysReg> orig;
};

// Re-color the given Vreg to the given register.
void recolor_set(State& state,
                 RecolorState& recolor,
                 Vreg r,
                 PhysReg p) {
  assertx(!recolor.fixed[r]);
  // Whenever we set a register, we fix it so it won't be changed in this
  // attempt again. This prevents infinite recursion.
  recolor.fixed[r] = true;
  auto& color = reg_info(state, r).color;
  // Save the old colors.
  recolor.old[r] = color_reg(color);
  if (!recolor.notOrig[r]) {
    recolor.orig[r] = recolor.old[r];
    recolor.notOrig[r] = true;
  }
  // Update and record that it changed in this attempt.
  color = p;
  recolor.changed.add(r);
}

// Attempt to recolor the given Vreg to some available register other than the
// given register. (It should "avoid" p). This might involve recursively
// recoloring other Vregs. Return true if successful, false otherwise. Vreg
// colorings may have been changed, even in the case of failure, and may need to
// be rolled back,
bool recolor_avoid(State& state,
                   RecolorState& recolor,
                   Vreg r,
                   PhysReg p) {
  auto const& info = reg_info(state, r);
  assertx(!is_constrained_vreg(state, r) ||
          color_reg(info.color) == info.precolor);
  // If we're already not that register, everything is good.
  if (is_spill(info.regClass) || color_reg(info.color) != p) return true;
  // Can't change a Vreg we've fixed or a contrained Vreg.
  if (recolor.fixed[r] || is_constrained_vreg(state, r)) return false;

  auto const& neighbors = find_interferences(state, r);

  // Find the best new register to use for this Vreg. Find the available
  // register which is used least among this Vreg's interference neighbors. This
  // will require the least amount of work to re-color.
  auto const newReg = [&]{
    PhysReg::Map<size_t> uses;
    neighbors.forEach(
      [&] (Vreg neighbor) {
        auto const& info = reg_info(state, neighbor);
        if (is_spill(info.regClass)) return;
        auto const reg = color_reg(info.color);
        if (!recolor.available.contains(reg)) return;
        ++uses[reg];
      }
    );

    PhysReg best;
    size_t bestCount = 0;
    recolor.available.forEach(
      [&] (PhysReg reg) {
        if (reg == p) return;
        if (best != InvalidReg && uses[reg] >= bestCount) return;
        bestCount = uses[reg];
        best = reg;
      }
    );
    assertx(best != InvalidReg);
    return best;
  }();

  // Set this Vreg to the best new register.
  recolor_set(state, recolor, r, newReg);

  // Then attempt to recursively re-color all of the interference neighbors to
  // something else.
  auto success = true;
  neighbors.forEach(
    [&] (Vreg neighbor) {
      assertx(success);
      if (!recolor_avoid(state, recolor, neighbor, newReg)) success = false;
      return success;
    }
  );

  return success;
}

// Attempt to recolor the given Vreg to the given register, recursively
// re-coloring other Vregs as necessary to avoid conflicts.
void recolor_single(State& state,
                    RecolorState& recolor,
                    Vreg r,
                    PhysReg p) {
  // If this Vreg is fixed, we can't change it, nor can we recolor a precolored
  // Vreg to something other than its precolor.
  if (recolor.fixed[r]) return;
  if (is_constrained_vreg(state, r) && reg_info(state, r).precolor != p) return;

  recolor.changed.reset();
  // Update this Vreg's color
  recolor_set(state, recolor, r, p);
  auto const& neighbors = find_interferences(state, r);
  // Attempt to recolor all of its interference neighbors to a different
  // register.
  neighbors.forEach(
    [&] (Vreg neighbor) {
      if (recolor_avoid(state, recolor, neighbor, p)) return true;
      // We failed to recolor this neighbor to a different register. We have to
      // stop with this attempt, so rollback the colors to their old value.
      recolor.changed.forEach(
        [&] (Vreg changed) {
          reg_info(state, changed).color = recolor.old[changed];
        }
      );
      return false;
    }
  );

  // We fixed Vregs as we changed them. Unfix them all now.
  recolor.changed.forEach(
    [&] (Vreg changed) { recolor.fixed[changed] = false; }
  );
}

struct AffinityChunkPriority {
  bool operator()(const AffinityChunk& c1, const AffinityChunk& c2) const {
    if (c1.score < c2.score) return true;
    if (c1.score > c2.score) return false;
    return c1.regs > c2.regs;
  }
};
using AffinityChunkQueue =
  jit::priority_queue<AffinityChunk, AffinityChunkPriority>;

// Attempt to assign all of the Vregs in the chunk at the top of the queue to
// the same color which maximizes the scores of the satisfied affinities. This
// might involve splitting the chunk into smaller pieces, which will be
// re-inserted into the queue.
void recolor_chunk(State& state,
                   RecolorState& recolor,
                   AffinityChunkQueue& chunks) {
  assertx(!chunks.empty());

  auto const& chunk = chunks.top();
  assertx(chunk.regs.size() > 1);

  // The best physical register we've found so far along with the score it
  // generates and the set of Vregs that can be colored to that register.
  PhysReg bestReg;
  VregSet bestSet;
  size_t bestScore = 0;

  // Attempt to re-color the Vregs in the current chunk with the given
  // register. Updating the best score as necessary.
  auto const test = [&] (PhysReg reg) {
    // Unfix all the colors in the chunk since we're going to be changing them.
    for (auto const r : chunk.regs) recolor.fixed[r] = false;

    recolor.notOrig.reset();
    VregSet changed;
    // Attempt to re-color each Vreg in the chunk, fixing each one when we
    // finish (to avoid thrash). Record the Vregs which actually changed during
    // the attempt.
    for (auto const r : chunk.regs) {
      recolor_single(state, recolor, r, reg);
      changed |= recolor.changed;
      recolor.fixed[r] = true;
    }

    // Check which subset of the chunk was actually re-colored to the desired
    // register.
    VregSet matched;
    for (auto const r : chunk.regs) {
      if (color_reg(reg_info(state, r).color) == reg) matched.add(r);
    }

    // Roll-back the colors to what they were originally.
    changed.forEach(
      [&] (Vreg r) {
        if (!recolor.notOrig[r]) return;
        reg_info(state, r).color = recolor.orig[r];
      }
    );

    if (matched.none()) return;

    // Calculate the sum of the affinities in the chunk which are now satisfied
    // by having both Vregs have the same color.
    size_t score = 0;
    for (auto const& affinity : chunk.affinities) {
      if (matched[affinity.r1] && matched[affinity.r2]) {
        score += affinity.score;
      }
    }

    // Update the best score if we've improved it.
    if (bestReg != InvalidReg && score < bestScore) return;
    bestReg = reg;
    bestScore = score;
    bestSet = std::move(matched);
  };

  // If the chunk has a constraint (some of the Vregs inside it have a
  // precolor), try that first. Its very likely that will end up as the best
  // choice anyways.
  if (chunk.constraint != InvalidReg) test(chunk.constraint);
  // If we found a register whose score matches the total of the chunk, we can't
  // do any better. Otherwise try all the other available registers, bailing out
  // if we hit the max score.
  if (bestReg == InvalidReg || bestScore != chunk.score) {
    recolor.available.forEach(
      [&] (PhysReg reg) {
        if (reg == chunk.constraint) return; // Checked outside of the loop
        if (bestReg != InvalidReg && bestScore == chunk.score) return;
        test(reg);
      }
    );
  }

  // We should have found a register and been able to re-color at least one Vreg
  // with it.
  assertx(bestReg != InvalidReg);
  assertx(bestSet.any());

  // After each re-coloring attempt, we rollbacked the colors to what they were
  // originally. Now that we have the best register, we're going to set them for
  // good. Unfix all the colors so we can change them.
  for (auto const r : chunk.regs) recolor.fixed[r] = false;

  // And then recolor the Vregs with the best register.
  for (auto const r : chunk.regs) {
    recolor_single(state, recolor, r, bestReg);
    recolor.fixed[r] = true; // This is its new color, so fix it for good.
    if (!bestSet[r]) continue;
    // If we successfully re-colored this Vreg to this register the first time,
    // we should be able to do it now.
    assertx(color_reg(reg_info(state, r).color) == bestReg);
    assertx(
      !is_constrained_vreg(state, r) ||
      color_reg(reg_info(state, r).color) == reg_info(state, r).precolor
    );
  }

  // The Vregs which were successfully recolored will be left as fixed so they
  // don't get recolored by later chunks. Those that weren't are eligible to be
  // changed, however.
  for (auto const r : chunk.regs) {
    if (!bestSet[r]) recolor.fixed[r] = false;
  }

  // We re-colored everything, so remove the chunk and we're done.
  if (chunk.regs.size() == bestSet.size()) {
    chunks.pop();
    return;
  }

  // We successfully re-colored some subset of the chunk with a color. Split the
  // portion of the chunk which was not re-colored into a new chunk.
  auto rest = chunk.split(state, [&] (Vreg r) { return !bestSet[r]; });
  chunks.pop();
  // Insert the new chunk back into the queue for re-coloring, unless if the
  // chunk only has a single Vreg. A singleton chunk can be left as whatever
  // color it has because by definition it has no copies to optimize away.
  if (rest.regs.size() > 1) chunks.emplace(std::move(rest));
}

void optimize_colors(State& state) {
  // Build def/use information because we'll need that for interference checks.
  record_defs_and_uses(state);
  // Then build the chunks.
  auto chunks = build_affinity_chunks(state);

  // Partition the chunks by the ones which represent spills and the ones which
  // don't.
  auto const spillsBegin = std::stable_partition(
    chunks.begin(), chunks.end(),
    [] (const AffinityChunk& c) { return !is_spill(c.cls); }
  );

  // Process the spills first. Optimization is a bit of misnormer here since we
  // haven't assign colors to spills at all until now. Instead we use the same
  // chunk machinery to assign spill slots. Since there's no upper bound on the
  // number of spill slots we need to allocate this works.

  // The chunks may not be sorted by their score. Sort the spill portion now.
  std::sort(
    spillsBegin, chunks.end(),
    [] (const AffinityChunk& c1, const AffinityChunk& c2) {
      if (c1.score > c2.score) return true;
      if (c1.score < c2.score) return false;
      return c1.regs < c2.regs;
    }
  );

  // We only merged the spill chunks together if there was an affinity between
  // them. Now merge together the spill chunks solely if they don't
  // interfere. We don't want to do this for non-spill chunks because it
  // combines Vregs into a single chunk for no good reason (we'll try to give
  // them the same color even though there's no profit to be had from doing
  // that). However we want to do it for spill chunks because it minimizes the
  // total number of spill slots. This is, of course, a non-issue for non-spill
  // chunks because the number of colors is fixed.
  for (auto it1 = spillsBegin; it1 != chunks.end(); ++it1) {
    auto& chunk1 = *it1;
    if (chunk1.regs.empty()) continue;
    assertx(is_spill(chunk1.cls));
    assertx(chunk1.constraint == InvalidReg);

    for (auto it2 = it1+1; it2 != chunks.end(); ++it2) {
      auto& chunk2 = *it2;
      if (chunk2.regs.empty()) continue;
      assertx(is_spill(chunk2.cls));
      assertx(chunk2.constraint == InvalidReg);
      if (chunk1.cls != chunk2.cls || chunk1.interferes(state, chunk2)) {
        continue;
      }
      chunk1.mergeInto(state, chunk2, nullptr);
      chunk2.reset();
    }
  }

  assertx(state.numSpillSlots == 0);
  assertx(state.numWideSpillSlots == 0);

  // The spill slot assignment is easy now. Just assign them using counters,
  // with every Vreg in each chunk getting the same slot.
  for (auto it = spillsBegin; it != chunks.end(); ++it) {
    auto const& chunk = *it;
    if (chunk.regs.empty()) continue;
    assertx(is_spill(chunk.cls));
    auto const color = chunk.cls == RegClass::Spill
      ? Color{SpillSlot{state.numSpillSlots++}}
      : Color{SpillSlotWide{state.numWideSpillSlots++}};
    for (auto const r : chunk.regs) {
      assertx(is_color_none(reg_info(state, r).color));
      reg_info(state, r).color = color;
    }
  }
  // We're done now with the spill chunks, erase them.
  chunks.erase(spillsBegin, chunks.end());

  // Make sure the spill slot count is round (for alignment).
  if ((state.numSpillSlots % 2) == 1) ++state.numSpillSlots;

  RecolorState recolor;
  recolor.fixed.resize(state.regInfo.size());
  recolor.old.resize(state.regInfo.size());
  recolor.notOrig.resize(state.regInfo.size());
  recolor.orig.resize(state.regInfo.size());

  // Put the remaining chunks in the priority queue and process each one
  // according to its weight.
  AffinityChunkQueue chunksQueue(AffinityChunkPriority{}, std::move(chunks));
  while (!chunksQueue.empty()) {
    recolor.available = [&]{
      switch (chunksQueue.top().cls) {
        case RegClass::AnyNarrow:
        case RegClass::GP:
          return state.gpUnreserved;
        case RegClass::SIMD:
        case RegClass::SIMDWide:
          return state.simdUnreserved;
        case RegClass::Any:
        case RegClass::SF:
        case RegClass::Spill:
        case RegClass::SpillWide:
          break;
      }
      always_assert(false);
    }();
    // Attempt to recolor this chunk. If necessary, it will split the chunk and
    // re-insert it into the queue.
    recolor_chunk(state, recolor, chunksQueue);
  }
}

//////////////////////////////////////////////////////////////////////
// SSA lowering

/*
 * A move plan is a list of sequential moves or swaps needed to lower a set of
 * parallel moves. This is tricky because the original move might take a
 * register as both a source and a dest, and since the move is parallel, it must
 * read from the source before overwriting it.
 *
 * For example:
 *  copyargs r1, r2, r3 -> r2, r1, r4
 *
 * Becomes:
 *  Swap r1, r2
 *  Move r3, r4
 *
 * To build this, we build the transfer graph. This is a directed graph composed
 * of vertices representing registers. If there's a move from register R1 to
 * register R2, then there's an edge from the vertex representing R2 to the
 * vertex representing R1. Suppose we have an edge V1 -> V2 where V1 has an
 * in-degree of 0. V1 represents the register R3 and V2 represents R4. This
 * implies that R3 is not used as a source by any move. We emit a Move R4 -> R3
 * and remove V1 from the graph (and decrease any relevant in-degrees). This
 * process is repeated until there are no more vertices with in-degree of 0.
 *
 * At this point, the graph is now composed solely of loops. We walk each loop,
 * emitting swaps for adjacent vertices (until we get back to the beginning of
 * the loop).
 */

// This is a single component of a move plan. Its templatized to work with both
// physical registers or spill slots.
template <typename T>
struct MoveInfo {
  // Move: Move src -> dst
  // Xchg: Swap src <-> dst
  enum class Kind { Move, Xchg };
  Kind kind;
  T src;
  T dst;
};

// Generate a move plan from a set of copies from src to dest (specified
// backwards).
template <typename T>
jit::vector<MoveInfo<T>> make_move_plan(const jit::fast_map<T,T>& dstToSrc) {
  // Calculate in-degrees
  jit::fast_map<T, int> degree;
  degree.reserve(dstToSrc.size());
  for (auto const& kv : dstToSrc) {
    auto const src = kv.second;
    assertx(kv.first != src);
    ++degree[src];
  }

  // Compute the list of non-swap moves. Keep removing nodes from the graph that
  // have an in-degree of 0 (by setting their degree to -1, which makes it as
  // processed). Emit a move for each one.
  auto moveInfo = [&]{
    jit::vector<MoveInfo<T>> info;

    for (auto const& kv : dstToSrc) {
      auto dst = kv.first;
      auto src = kv.second;

      // Check if its either already processed (-1) or has an incoming edge (>
      // 0).
      if (degree[dst] != 0) continue;
      // It has no incoming edges, so we can remove it. Mark it as processed.
      degree[dst] = -1;

      // Walk the dst -> src chain as long as we keep lowering in-degrees to
      // zero.
      while (true) {
        info.push_back({MoveInfo<T>::Kind::Move, src, dst});
        // Remove the edge from dst to src and decrease the in-degree. If its
        // now zero, follow the chain and repeat the process.
        assertx(degree[src] > 0);
        if (--degree[src] != 0) break;
        degree[src] = -1;

        auto const it = dstToSrc.find(src);
        if (it == dstToSrc.end()) break;
        dst = src;
        src = it->second;
      }
    }

    return info;
  }();

  // At this point the transfer graph is nothing but loops. We can walk each
  // loop, emitting swaps for adjacent nodes.

  for (auto const& kv : dstToSrc) {
    auto const begin = kv.first;

    // Ignore nodes already processed above.
    if (degree[begin] < 0) continue;
    // We shouldn't have any zero nodes at this point.
    assertx(degree[begin] > 0);
    degree[begin] = -1;

    // Walk the loop
    auto prev = begin;
    auto current = kv.second;
    while (current != begin) {
      assertx(degree[current] > 0);
      degree[current] = -1;
      moveInfo.push_back({MoveInfo<T>::Kind::Xchg, prev, current});
      prev = current;
      auto const it = dstToSrc.find(current);
      assertx(it != dstToSrc.end());
      current = it->second;
    }
  }

  return moveInfo;
}

// Lower the instruction at the given position down to a set of copy and copy2
// (swap) instructions. The copies may be optimized away if their source(s) and
// dests(s) are the same register. Remove the input instruction and replace it
// with the new instructions. This assumes that the instructions have already
// been rewritten to take physical registers.
void lower_copies(const State& state,
                  Vlabel block,
                  size_t instIdx) {
  // Map of individual physical register moves (in reverse order, dest to src).
  jit::fast_map<PhysReg, PhysReg> regDstToSrc;
  // Map of individual spill slot moves (in reverse order, dest to src).
  jit::fast_map<size_t, size_t> spillDstToSrc;
  // Map of spill slot to the Vreg representing it in a src.
  jit::fast_map<size_t, Vreg> spillSrcSlotToReg;
  // Map of spill slot to the Vreg representing it in a dest.
  jit::fast_map<size_t, Vreg> spillDstSlotToReg;
  // If this is a phijmp, the target of the jmp.
  Vlabel phiTarget;

  auto const addSrcDstPair = [&] (Vreg src, Vreg dst) {
    // We've already converted Vregs to physical registers everywhere except
    // with Vregs representing spill slots. Therefore the only way the source
    // and dest can vary with regards to physical-ness is if we're copying
    // to/from a spill slot to a non-spill slot, which isn't allowed. Only spill
    // and reload instructions can move values to/from physical registers
    // to/from spill slots.
    assertx(src.isPhys() == dst.isPhys());

    if (src.isPhys()) {
      // Reg to reg copy. If the source and dest is the same, the copy is
      // trivially a no-op, so ignore it.
      if (src == dst) return;
      // Otherwise record the copy (backwards).
      auto const DEBUG_ONLY result =
        regDstToSrc.emplace(dst.physReg(), src.physReg());
      assertx(result.second);
      return;
    }

    // Spill to spill copy.
    auto const sInfo = reg_info(state, src);
    auto const dInfo = reg_info(state, dst);
    assertx(is_spill(sInfo.regClass));
    assertx(is_spill(dInfo.regClass));

    // Lookup the colored slot and record the meta-data.
    auto const sOffset = (sInfo.regClass == RegClass::Spill)
      ? color_spill_slot(sInfo.color).slot
      : color_spill_slot_wide(sInfo.color).slot + state.numSpillSlots;
    auto const dOffset = (dInfo.regClass == RegClass::Spill)
      ? color_spill_slot(dInfo.color).slot
      : color_spill_slot_wide(dInfo.color).slot + state.numSpillSlots;

    auto const DEBUG_ONLY result1 = spillSrcSlotToReg.emplace(sOffset, src);
    assertx(IMPLIES(!result1.second, result1.first->second == src));

    auto const DEBUG_ONLY result2 = spillDstSlotToReg.emplace(dOffset, dst);
    assertx(result2.second);

    // Trivial useless copy
    if (sOffset == dOffset) return;

    auto const DEBUG_ONLY result3 = spillDstToSrc.emplace(dOffset, sOffset);
    assertx(result3.second);
  };

  // Build up the list of moves from the instruction.
  auto const& inst = state.unit.blocks[block].code[instIdx];
  switch (inst.op) {
    // copy2 needs to be treated specially. We don't lower it to a move plan
    // (because it has special semantics). If its a no-op, we can remove it, but
    // otherwise leave it as is.
    case Vinstr::copy2: {
      if (inst.copy2_.s0 == inst.copy2_.d0 &&
          inst.copy2_.s1 == inst.copy2_.d1) {
        vmodify(state.unit, block, instIdx, [] (Vout&) { return 1; });
      }
      return;
    }
    case Vinstr::copy:
      addSrcDstPair(inst.copy_.s, inst.copy_.d);
      break;
    case Vinstr::copyargs: {
      auto const& srcs = state.unit.tuples[inst.copyargs_.s];
      auto const& dsts = state.unit.tuples[inst.copyargs_.d];
      assertx(srcs.size() == dsts.size());
      for (size_t i = 0; i < srcs.size(); ++i) addSrcDstPair(srcs[i], dsts[i]);
      break;
    }
    case Vinstr::phijmp: {
      auto const& srcs = state.unit.tuples[inst.phijmp_.uses];
      auto const succ = succs(state.unit.blocks[block]);
      assertx(succ.size() == 1);
      auto const& succBlock = state.unit.blocks[succ[0]];
      auto const& phidef = succBlock.code.front();
      assertx(phidef.op == Vinstr::phidef);
      auto const& dsts = state.unit.tuples[phidef.phidef_.defs];
      assertx(srcs.size() == dsts.size());
      for (size_t i = 0; i < srcs.size(); ++i) addSrcDstPair(srcs[i], dsts[i]);
      phiTarget = inst.phijmp_.target;
      break;
    }
    default:
      return;
  }

  using RegMoveInfo = MoveInfo<PhysReg>;
  using SpillMoveInfo = MoveInfo<size_t>;
  jit::vector<RegMoveInfo> regMoves;
  jit::vector<SpillMoveInfo> spillMoves;

  // Turn the register moves and spill moves into move plans.
  if (!regDstToSrc.empty()) {
    regMoves = make_move_plan(regDstToSrc);
  }
  if (!spillDstToSrc.empty()) {
    spillMoves = make_move_plan(spillDstToSrc);
  }

  vmodify(
    state.unit, block, instIdx,
    [&] (Vout& v) {
      // Turn the reg move plan into copies.
      for (auto const& move : regMoves) {
        assertx(move.src != move.dst);
        assertx(move.src != state.scratch);
        assertx(move.dst != state.scratch);

        switch (move.kind) {
          case RegMoveInfo::Kind::Move:
            v << copy{move.src, move.dst};
            break;
          case RegMoveInfo::Kind::Xchg:
            // Emit a swap if both operands are GP registers. We can't swap
            // SIMDs, so use the scratch register instead.
            if (move.src.isGP() && move.dst.isGP()) {
              v << copy2{move.src, move.dst, move.dst, move.src};
            } else {
              v << copy{move.src, state.scratch};
              v << copy{move.dst, move.src};
              v << copy{state.scratch, move.dst};
            }
            break;
        }
      }

      auto const srcSlotToReg = [&] (size_t offset) {
        auto const it = spillSrcSlotToReg.find(offset);
        assertx(it != spillSrcSlotToReg.end());
        return it->second;
      };
      auto const dstSlotToReg = [&] (size_t offset) {
        auto const it = spillDstSlotToReg.find(offset);
        assertx(it != spillDstSlotToReg.end());
        return it->second;
      };

      // We cannot lower spill moves into copies (obviously). Instead we emit a
      // reload into the scratch register and then a spill from the scratch
      // register. This accomplishes a mem-to-mem move. The actual spill/reload
      // instructions will be lowered to from/to mem moves later on.
      for (auto const& move : spillMoves) {
        assertx(move.src != move.dst);

        switch (move.kind) {
          case SpillMoveInfo::Kind::Move: {
            auto const s = srcSlotToReg(move.src);
            auto const d = dstSlotToReg(move.dst);
            v << reload{s, state.scratch};
            v << spill{state.scratch, d};
            break;
          }
          case SpillMoveInfo::Kind::Xchg: {
            // Swaps for spills are a pain. We need to use both the scratch
            // register and a temporary spill slot on the stack.
            auto const s1 = srcSlotToReg(move.src);
            auto const s2 = srcSlotToReg(move.dst);
            auto const d1 = dstSlotToReg(move.src);
            auto const d2 = dstSlotToReg(move.dst);
            v << reload{s1, state.scratch};
            if (reg_info(state, s1).regClass == RegClass::SpillWide) {
              v << storeups{state.scratch, rsp()[-16]};
            } else {
              v << store{state.scratch, rsp()[-16]};
            }
            v << reload{s2, state.scratch};
            v << spill{state.scratch, d1};
            if (reg_info(state, d2).regClass == RegClass::SpillWide) {
              v << loadups{rsp()[-16], state.scratch};
            } else {
              v << load{rsp()[-16], state.scratch};
            }
            v << spill{state.scratch, d2};
            break;
          }
        }
      }

      // Keep the jmp part of the phijmp
      if (phiTarget.isValid()) v << jmp{phiTarget};

      // Delete the original instruction
      return 1;
    }
  );
}

// Visitor used during converting pseudo instructions back to their
// originals. This is constructed with the operand data from the pseudo and run
// on the original instruction. It rewrites the instruction to take the (now
// physical) registers in the pseudo operands. In the cases where it cannot
// rewrite the instruction (because it uses a RegSet for example), we verify
// that the original physical registers match the ones the allocator came up
// with.
struct PseudoConvertRestoreVisitor {
  template <typename T> void imm(const T&) const {}

  void use(Vptr& p) {
    if (p.base.isValid())  use(p.base);
    if (p.index.isValid()) use(p.index);
  }
  void use(const RegSet& s) {
    s.forEach(
      [&] (Vreg r) {
        // We shouldn't have changed the RegSet data because it was constrained.
        assertx(usesIdx < uses.size());
        auto const r2 = uses[usesIdx++];
        always_assert(r == r2);
      }
    );
  }
  void use(Vreg& r) {
    assertx(usesIdx < uses.size());
    always_assert(!r.isPhys() || r == uses[usesIdx]);
    r = uses[usesIdx++];
  }
  void use(Vtuple) { always_assert(false); }
  void use(VcallArgsId) { always_assert(false); }

  void use(Vreg64& r) {
    assertx(uses64Idx < uses64.size());
    always_assert(!r.isPhys() || r == uses64[uses64Idx]);
    r = uses64[uses64Idx++];
  }
  void use(VregSF& r) {
    assertx(flags != InvalidReg);
    r = flags;
    flags = InvalidReg;
  }
  template <typename W> void use(Vr<W>& r) { always_assert(false); }

  void useHint(Vreg64& r, Vreg64) {
    always_assert(!r.isPhys() || r == useWithHint);
    assertx(useWithHint != InvalidReg);
    r = useWithHint;
    useWithHint = InvalidReg;
  }

  template <typename U> void useHint(Vtuple, const U&) {
    always_assert(false);
  }
  template <typename U> void useHint(Vreg, const U&) {
    always_assert(false);
  }
  template <typename W, typename U> void useHint(Vr<W>, const U&) {
    always_assert(false);
  }

  void defHint(Vreg64& r, Vreg64) {
    always_assert(!r.isPhys() || r == defWithHint);
    assertx(defWithHint != InvalidReg);
    r = defWithHint;
    defWithHint = InvalidReg;
  }
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
  void def(Vreg& r) {
    assertx(defsIdx < defs.size());
    always_assert(!r.isPhys() || r == defs[defsIdx]);
    r = defs[defsIdx++];
  }

  void def(VregSF& r) {
    assertx(flags != InvalidReg);
    r = flags;
    flags = InvalidReg;
  }
  template <typename W> void def(Vr<W>) { always_assert(false); }

  // Data from the pseudo instruction, the indices below indicate how much
  // data we've consumed from each.
  const jit::vector<Vreg>& defs;
  const jit::vector<Vreg>& uses;
  const jit::vector<Vreg>& uses64;
  const jit::vector<Vreg>& acrosses;

  Vreg defWithHint;
  Vreg useWithHint;

  VregSF flags;

  // As we overwrite each operand of the instruction, using the above
  // pseudo data, we increment the indices.
  size_t defsIdx = 0;
  size_t usesIdx = 0;
  size_t uses64Idx = 0;
  size_t acrossesIdx = 0;
};

void restore_pseudo(State& state, Vinstr& inst) {
  auto const convert = [&] (const jit::vector<Vreg>& defs,
                            const jit::vector<Vreg>& uses,
                            const jit::vector<Vreg>& uses64,
                            const jit::vector<Vreg>& acrosses,
                            VregSF flags = InvalidReg,
                            Vreg defWithHint = InvalidReg,
                            Vreg useWithHint = InvalidReg) {
    assertx(inst.pos < state.pseudos.size());
    // Run the visitor on the original instruction, rewriting its operands.
    PseudoConvertRestoreVisitor v{
      defs, uses, uses64, acrosses, defWithHint, useWithHint, flags
    };
    visitOperands(state.pseudos[inst.pos], v);

    // We can't change implicit register effects, so make sure they weren't
    // changed in the pseudo.
    RegSet implicitUses, implicitAcross, implicitDefs;
    getEffects(
      state.abi,
      state.pseudos[inst.pos],
      implicitUses,
      implicitAcross,
      implicitDefs
    );
    implicitDefs.forEach(
      [&](Vreg r) {
        assertx(v.defsIdx < v.defs.size());
        auto const r2 = v.defs[v.defsIdx++];
        always_assert(r == r2);
      }
    );
    implicitUses.forEach(
      [&](Vreg r) {
        assertx(v.usesIdx < v.uses.size());
        auto const r2 = v.uses[v.usesIdx++];
        always_assert(r == r2);
      }
    );
    implicitAcross.forEach(
      [&](Vreg r) {
        assertx(v.acrossesIdx < v.acrosses.size());
        auto const r2 = v.acrosses[v.acrossesIdx++];
        always_assert(r == r2);
      }
    );

    // The visitor should have consumed all of the operand data.
    assertx(v.defsIdx == v.defs.size());
    assertx(v.usesIdx == v.uses.size());
    assertx(v.acrossesIdx == v.acrosses.size());
    assertx(v.defWithHint == InvalidReg);
    assertx(v.useWithHint == InvalidReg);
    assertx(v.flags == InvalidReg);

    // Change the instruction back to the original (which has been rewritten).
    inst = state.pseudos[inst.pos];
  };

  switch (inst.op) {
    case Vinstr::pseudojmp: {
      auto const& pseudo = inst.pseudojmp_;
      convert(
        state.unit.tuples[pseudo.defs],
        state.unit.tuples[pseudo.uses],
        state.unit.tuples[pseudo.uses64],
        state.unit.tuples[pseudo.across]
      );
      break;
    }
    case Vinstr::pseudocall: {
      auto const& pseudo = inst.pseudocall_;
      convert(
        state.unit.tuples[pseudo.defs],
        state.unit.tuples[pseudo.uses],
        state.unit.tuples[pseudo.uses64],
        state.unit.tuples[pseudo.across]
      );
      break;
    }
    case Vinstr::pseudojcc: {
      auto const& pseudo = inst.pseudojcc_;
      convert(
        state.unit.tuples[pseudo.defs],
        state.unit.tuples[pseudo.uses],
        jit::vector<Vreg>{},
        state.unit.tuples[pseudo.across],
        pseudo.sf
      );
      break;
    }
    case Vinstr::pseudodiv: {
      auto const& pseudo = inst.pseudodiv_;
      convert(
        state.unit.tuples[pseudo.defs],
        state.unit.tuples[pseudo.uses],
        state.unit.tuples[pseudo.uses64],
        state.unit.tuples[pseudo.across],
        pseudo.sf
      );
      break;
    }
    case Vinstr::pseudocallphp: {
      auto const& pseudo = inst.pseudocallphp_;
      convert(
        state.unit.tuples[pseudo.defs],
        state.unit.tuples[pseudo.uses],
        state.unit.tuples[pseudo.uses64],
        state.unit.tuples[pseudo.across]
      );
      break;
    }
    case Vinstr::pseudoshift: {
      auto const& pseudo = inst.pseudoshift_;
      convert(
        state.unit.tuples[pseudo.defs],
        state.unit.tuples[pseudo.uses],
        jit::vector<Vreg>{},
        state.unit.tuples[pseudo.across],
        pseudo.sf,
        pseudo.d,
        pseudo.s
      );
      break;
    }
    default:
      return;
  }
}

/*
 * Spill materialization:
 *
 * The materialization of spill and reload instructions can be complicated. The
 * actual transformation of the instructions is easy, but we need to account for
 * the stack space the spill slots use.
 *
 * Spill slots occupy space on the stack, so the stack pointer needs to be
 * adjusted appropriately. We cannot simply access beyond the stack pointer
 * because it might be clobbered by a call.
 *
 * There are two complications: (1) We want to defer the stack pointer
 * adjustment until its actually needed (the first spill). (2) Other
 * instructions may manipulate the stack pointer. We want to insert the stack
 * pointer adjustments right before a spill slot is used (requiring the space),
 * or when the stack pointer is changed by another instruction.
 *
 * First we split any side exiting blocks. These are blocks which contain
 * instructions which implicitly exit the unit (without being reflected in
 * control flow). The problem is, we might want to insert stack pointer
 * adjustment code along such exits, and we need a place to do so. We replace
 * these instructions with conditional jumps to blocks which use the
 * unconditional version of the side-exit.
 *
 * Next we calculate the live regions for the spill slots. This is just the
 * blocks where there's a live spilled value. Inside these regions, we need to
 * have allocated the spill space on the stack (so the stack pointer should be
 * adjusted).
 *
 * We then calculate the live regions for the stack pointer. This is the blocks
 * where the stack pointer has been adjusted (by other instructions) away from
 * its position when we entered the unit.
 *
 * Normally we insert the stack pointer adjustments when we enter or exit the
 * spill slots live region. However, if we're already in a stack pointer live
 * region when we enter or exit, we need to instead move the adjustment outward
 * to the entry/exit of that stack pointer region. This is because we need to
 * perform our adjustment before/after any stack pointer adjustment done by
 * other instructions.
 *
 * So, if any spill slots live region is adjacent to a stack pointer live
 * region, we expand the spill slots region to include that stack pointer live
 * region. We repeat this process until the spill slots live region won't expand
 * any further.
 *
 * Once this is done, we insert the actual stack pointer adjustments at the
 * frontier of the spill slots live regions. Now that the stack is set up
 * correctly, we lower the spill/reload instructions into stores or loads, using
 * the stack pointer offset at that point.
 *
 * If we split any side-exiting blocks above, we then run some vasm optimization
 * passes to attempt to recollapse them to side-exit instructions. We may not
 * have had to insert any adjustment code there, so they can go back to their
 * original form.
 */

// Return the amount an instruction will move the stack pointer. Negative is
// moving away from the frame pointer, and positive is moving towards it. This
// only supports the type of instructions we expect to see, and not abitrary
// ones.
int sp_change(const State& state, const Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::push:
    case Vinstr::pushf:
    case Vinstr::pushm:
      return -8;
    case Vinstr::pushp:
    case Vinstr::pushpm:
      return -16;
    case Vinstr::pop:
    case Vinstr::popf:
    case Vinstr::popm:
      return 8;
    case Vinstr::popp:
    case Vinstr::poppm:
      return 16;
    case Vinstr::lea: {
      auto const& lea = inst.lea_;
      if (lea.d == rsp()) {
        assertx(lea.s.base == lea.d && !lea.s.index.isValid());
        return lea.s.disp;
      }
      return 0;
    }
    default:
      // No other instruction should be writing to the stack pointer.
      if (debug) {
        visitDefs(
          state.unit, inst,
          [&] (Vreg r) { always_assert(r != rsp()); }
        );
      }
      return 0;
  }
}

// Per-block information about the offset of the stack pointer (relative to its
// position at the start of the unit).
struct SPOffset {
  folly::Optional<int> in;
  folly::Optional<int> out;
};
using SPOffsets = jit::vector<SPOffset>;

// Calculate stack pointer offset information
SPOffsets calculate_sp_offsets(const State& state) {
  auto const& unit = state.unit;

  SPOffsets spOffsets(unit.blocks.size());

  // The offset is relative to the start of unit position.
  spOffsets[unit.entry].in = 0;

  // We don't need dataflow for this because we require that the stack pointer
  // offset is always statically known. This implies that it cannot have
  // different offsets at join points (we'll assert otherwise), so one pass is
  // sufficient.
  for (auto const b : state.rpo) {
    assertx(spOffsets[b].in);
    auto spOffset = *spOffsets[b].in;

    // Calculate the block's instruction effects on the offset.
    auto const& block = unit.blocks[b];
    for (auto const& inst : block.code) {
      spOffset += sp_change(state, inst);
      // Don't support moving the stack pointer before where it started
      // originally.
      assertx(spOffset <= 0);
    }

    // Propagate state to successors.
    auto const successorList = succs(block);
    // If there's no successors, we're exiting the unit, so make sure the stack
    // pointer has been returned to its entry position.
    assertx(IMPLIES(successorList.empty(), spOffset == 0));
    for (auto const succ : successorList) {
      // Join point shouldn't have differing offsets.
      assertx(!spOffsets[succ].in || *spOffsets[succ].in == spOffset);
      spOffsets[succ].in = spOffset;
    }

    spOffsets[b].out = spOffset;
  }

  return spOffsets;
}

// If the given instruction is a spill or reload, turn it into the appropriate
// load/store instruction to/from memory. Use the given stack pointer offset to
// calculate the location of the spill slot. Return true if the transformation
// happened, false otherwise.

/*
 * (Stack grows downward)
 *
 *                 | Prev Frame    |
 * prev %rsp --->  -----------------   <-----------------------
 *                 | Spill Slot #0 |     |                    |
 *                 -----------------     |                    |
 *                 | Spill Slot #1 |     |--- spillSpace (24) |
 *                 -----------------     |                    |
 *                 | Spill Slot #2 |     |                    |-- spOffset (-40)
 *                 -----------------   <--                    |
 *                 |               |     |                    |
 *                 -----------------     |--- skip (16)       |
 *                 |               |     |                    |
 *      %rsp --->  -----------------   <-----------------------
 *
 */
bool materialize_spill(const State& state, Vinstr& inst, int skip) {
  auto const to_offset = [&] (SpillSlot slot) {
    return slot.slot*8 + skip;
  };
  auto const to_offset_wide = [&] (SpillSlotWide slot) {
    return state.numSpillSlots*8 + slot.slot*16 + skip;
  };

  if (inst.op == Vinstr::spill) {
    assertx(skip >= 0);
    assertx(inst.spill_.s.isPhys());
    auto const& info = reg_info(state, inst.spill_.d);
    assertx(is_spill(info.regClass));

    if (info.regClass == RegClass::Spill) {
      auto const color = color_spill_slot(info.color);
      inst.store_ = store{
        inst.spill_.s,
        rsp()[to_offset(color)]
      };
      inst.op = Vinstr::store;
    } else {
      assertx(info.regClass == RegClass::SpillWide);
      assertx(inst.spill_.s.isSIMD());
      auto const color = color_spill_slot_wide(info.color);
      inst.storeups_ = storeups{
        inst.spill_.s,
        rsp()[to_offset_wide(color)]
      };
      inst.op = Vinstr::storeups;
    }

    return true;
  } else if (inst.op == Vinstr::reload) {
    assertx(skip >= 0);
    assertx(inst.reload_.d.isPhys());
    auto const& info = reg_info(state, inst.reload_.s);
    assertx(is_spill(info.regClass));

    if (info.regClass == RegClass::Spill) {
      auto const color = color_spill_slot(info.color);
      inst.load_ = load{
        rsp()[to_offset(color)],
        inst.reload_.d
      };
      inst.op = Vinstr::load;
    } else {
      assertx(info.regClass == RegClass::SpillWide);
      assertx(inst.reload_.d.isSIMD());
      auto const color = color_spill_slot_wide(info.color);
      inst.loadups_ = loadups{
        rsp()[to_offset_wide(color)],
        inst.reload_.d
      };
      inst.op = Vinstr::loadups;
    }

    return true;
  }

  return false;
}

// Turn all spill/reload instructions in the unit into appropriate load/store
// instructions to/from memory. spillSpace is the total amount of space
// allocated for spills.
void materialize_spills(const State& state, size_t spillSpace) {
  // The exact offset to use for a stack slot differs depending on the current
  // stack pointer offset (because spill slots are addressed off the stack
  // pointer). Therefore we need the offsets for every block.
  auto const spOffsets = calculate_sp_offsets(state);
  assertx(spOffsets.size() == state.unit.blocks.size());

  for (auto const b : state.rpo) {
    assertx(spOffsets[b].in);
    auto spOffset = *spOffsets[b].in;
    assertx(IMPLIES(b == state.unit.entry, spOffset == 0));
    assertx(spOffset <= 0);

    auto& block = state.unit.blocks[b];
    for (auto& inst : block.code) {
      if (materialize_spill(state, inst, -spOffset - spillSpace)) {
        assertx(sp_change(state, inst) == 0);
      } else {
        spOffset += sp_change(state, inst);
        assertx(spOffset <= 0);
      }
    }

    // Do a sanity check that our offset calculation for the block matches what
    // was calculated by calculate_sp_offsets().
    if (debug) {
      auto const successorList = succs(block);
      always_assert(IMPLIES(successorList.empty(), spOffset == 0));
      for (auto const succ : successorList) {
        always_assert(spOffsets[succ].in == spOffset);
      }
    }
  }
}

// Per-block liveness information for either spill slots or the stack pointer
// offset. We call the stack pointer offset alive if its different than its
// entry block position (non-zero).
struct SPAdjustLiveness {
  bool in = false; // If true, the spill slots/stack pointer is live-in
  bool out = false; // If true, the spill slots/stack pointer is live-out
  // If set, the index of the first instruction in the block which goes from
  // dead to alive.
  folly::Optional<size_t> begin;
  // If set, the index of the last instruction in the block which goes from
  // alive to dead.
  folly::Optional<size_t> end;
};

// Calculate liveness information for the spill slots.
jit::vector<SPAdjustLiveness> find_spill_liveness(const State& state) {
  auto const& unit = state.unit;

  // First calculate the spill slot liveness in the conventional manner using
  // dataflow.
  struct SlotLiveness {
    boost::dynamic_bitset<> in;
    boost::dynamic_bitset<> out;
    boost::dynamic_bitset<> gen;
    boost::dynamic_bitset<> kill;
  };
  jit::vector<SlotLiveness> slotLiveness(unit.blocks.size());

  auto const offset = [&] (Vreg r) {
    auto const& info = reg_info(state, r);
    if (info.regClass == RegClass::Spill) {
      return color_spill_slot(info.color).slot;
    } else {
      assertx(info.regClass == RegClass::SpillWide);
      return color_spill_slot_wide(info.color).slot + state.numSpillSlots;
    }
  };

  auto const numSlots = state.numSpillSlots + state.numWideSpillSlots;

  dataflow_worklist<size_t, std::less<size_t>> worklist(state.rpo.size());
  for (size_t i = 0; i < state.rpo.size(); ++i) worklist.push(i);

  for (auto const b : state.rpo) {
    auto& live = slotLiveness[b];
    live.in.resize(numSlots);
    live.out.resize(numSlots);
    live.gen.resize(numSlots);
    live.kill.resize(numSlots);

    auto const& block = unit.blocks[b];
    for (auto const& inst : boost::adaptors::reverse(block.code)) {
      if (inst.op == Vinstr::spill) {
        auto const o = offset(inst.spill_.d);
        live.kill[o] = true;
        live.gen[o] = false;
      } else if (inst.op == Vinstr::reload) {
        auto const o = offset(inst.reload_.s);
        live.kill[o] = false;
        live.gen[o] = true;
      }
    }

    live.in = live.gen;
  }

  while (!worklist.empty()) {
    auto const b = state.rpo[worklist.pop()];
    auto& live = slotLiveness[b];

    live.out.reset();
    auto const& block = unit.blocks[b];
    for (auto const succ : succs(block)) {
      live.out |= slotLiveness[succ].in;
    }

    auto in = (live.out - live.kill) | live.gen;
    if (in != live.in) {
      for (auto const pred : state.preds[b]) {
        worklist.push(state.rpoOrder[pred]);
      }
      live.in = std::move(in);
    }
  }

  jit::vector<SPAdjustLiveness> liveness(unit.blocks.size());

  for (auto const b : state.rpo) {
    auto& live = liveness[b];

    // The spill slots are alive if any of the individual ones were calculated
    // to be alive (and vice-versa).
    live.in = slotLiveness[b].in.any();
    live.out = slotLiveness[b].out.any();

    // If there are no spill slots alive going into the block, look for any
    // instruction which make them alive.
    if (!live.in) {
      auto& block = unit.blocks[b];
      for (size_t i = 0; i < block.code.size(); ++i) {
        auto const& inst = block.code[i];
        assertx(inst.op != Vinstr::reload);
        if (inst.op != Vinstr::spill) continue;
        live.begin = i;
        break;
      }
    }

    // If there are no spill slots alive going out of the block and there's any
    // spill slot alive within it, look backwards for any instruction which
    // kills them.
    if (!live.out && (live.in || live.begin)) {
      auto& block = unit.blocks[b];
      for (size_t i = block.code.size(); i > 0; --i) {
        auto const& inst = block.code[i-1];
        // A spill slot could be completely dead, in which case its live-range
        // is just the single spill instruction defining it (without a reload).
        if (inst.op != Vinstr::reload &&
            inst.op != Vinstr::spill) continue;
        live.end = i - 1;
        break;
      }
    }
  }

  return liveness;
}

// Calculate liveness information for the stack pointer offset
jit::vector<SPAdjustLiveness> find_sp_liveness(const State& state,
                                               bool& found) {
  // The stack pointer is alive only if its offset is non-zero, so we'll use the
  // offset information.
  auto const spOffsets = calculate_sp_offsets(state);
  assertx(spOffsets.size() == state.unit.blocks.size());

  auto const& unit = state.unit;
  jit::vector<SPAdjustLiveness> liveness(unit.blocks.size());

  for (auto const b : state.rpo) {
    assertx(spOffsets[b].in);
    assertx(spOffsets[b].out);
    auto const in = *spOffsets[b].in;
    auto const out = *spOffsets[b].out;

    auto& live = liveness[b];

    if (in == 0) {
      // The in-offset of the block is 0, so the stack pointer offset is dead
      // coming into the block. Look for any instruction which changes it to
      // make the stack pointer offset alive.
      auto& block = unit.blocks[b];
      for (size_t i = 0; i < block.code.size(); ++i) {
        auto const& inst = block.code[i];
        if (sp_change(state, inst) == 0) continue;
        live.begin = i;
        found = true;
        break;
      }
    } else {
      // Otherwise its live-in to the block.
      live.in = true;
      found = true;
    }

    if (out == 0) {
      // The out-offset of the block is 0, so the stack pointer offset is dead
      // going out of the block. Look backwards for any instruction which
      // changes it to make the stack pointer offset alive.
      auto& block = unit.blocks[b];
      for (size_t i = block.code.size(); i > 0; --i) {
        auto const& inst = block.code[i-1];
        if (sp_change(state, inst) == 0) continue;
        live.end = i - 1;
        found = true;
        break;
      }
    } else {
      // Otherwise its live-out to the block.
      live.out = true;
      found = true;
    }
  }

  return liveness;
}

// Expand the spill slot liveness information to also include the stack pointer
// offset liveness if they are adjacent anywhere. This will give us the region
// where we have to make stack pointer adjustments.
void expand_spill_liveness(const State& state,
                           jit::vector<SPAdjustLiveness>& spillLiveness,
                           const jit::vector<SPAdjustLiveness>& spLiveness) {
  // First find the blocks where the spill slots and stack pointer offset are
  // both alive and expand the begin or end data appropriately.
  for (auto const b : state.rpo) {
    auto& spill = spillLiveness[b];
    auto const& sp = spLiveness[b];

    if (!spill.in && !spill.out && !spill.begin && !spill.end) continue;

    if (!spill.in && sp.in) {
      spill.in = true;
      spill.begin.reset();
    }

    if (!spill.out && sp.out) {
      spill.out = true;
      spill.end.reset();
    }

    if (spill.begin && sp.begin) {
      spill.begin = std::min(*spill.begin, *sp.begin);
    }

    if (spill.end && sp.end) {
      spill.end = std::max(*spill.end, *sp.end);
    }
  }

  // Then expand the spill slot live region to include any stack pointer offset
  // live regions that are adjacent to it. We repeat this process until we can't
  // expand it any further.
  auto changed = true;
  while (changed) {
    changed = false;

    for (auto const b : state.rpo) {
      auto& spill = spillLiveness[b];
      auto const& sp = spLiveness[b];

      // If the spill slot liveness range already covers this block, no need to
      // extend it.
      if (spill.in || spill.out || spill.begin || spill.end) continue;

      if (sp.out) {
        for (auto const succ : succs(state.unit.blocks[b])) {
          if (!spillLiveness[succ].in) continue;
          spill = sp;
          changed = true;
          break;
        }
      }

      if (spill.out) continue;

      if (sp.in) {
        for (auto const pred : state.preds[b]) {
          if (!spillLiveness[pred].out) continue;
          spill = sp;
          changed = true;
          break;
        }
      }
    }
  }

  // The spill slot live region(s) now incorporates any stack pointer offset
  // live region(s).
}

// Insert stack pointer adjustments at the appropriate places
void insert_sp_adjustments(State& state, size_t spillSpace) {
  // Calculate where spill slots are alive, then where the stack pointer is
  // alive (non zero offset), and expand the spill slot liveness to match the
  // stack pointer liveness.
  auto spillLiveness = find_spill_liveness(state);

  auto found = false;
  auto const spLiveness = find_sp_liveness(state, found);
  if (found) expand_spill_liveness(state, spillLiveness, spLiveness);

  auto& unit = state.unit;
  // Insert a lea to adjust the stack pointer (either forwards or backwards)
  // before the instruction at the given index.
  auto const modify = [&] (Vlabel b, size_t check, size_t i, int adjustment) {
    // If there's already a lea there, we can just modify that.
    if (unit.blocks[b].code[check].op == Vinstr::lea) {
      auto& lea = unit.blocks[b].code[check].lea_;
      if (lea.d == rsp() && lea.s.base == rsp() && !lea.s.index.isValid()) {
        lea.s.disp += adjustment;
        return 0;
      }
    }
    vmodify(
      unit, b, i,
      [&] (Vout& v) {
        v << lea{rsp()[adjustment], rsp()};
        return 0;
      }
    );
    return 1;
  };

  for (auto const b : state.rpo) {
    auto const& spill = spillLiveness[b];

    size_t added = 0;
    if (spill.begin) {
      // The spill slot liveness begins here at an instruction, so insert a
      // stack pointer adjustment before that instruction.
      added += modify(b, *spill.begin, *spill.begin, -spillSpace);
    }
    if (spill.end) {
      // The spill slot liveness ends here at an instruction, so insert a stack
      // pointer adjustment back before that instruction.
      added += modify(
        b, *spill.end + added, *spill.end + added + 1, spillSpace
      );
    }

    if (!spill.out) continue;
    for (auto const succ : succs(unit.blocks[b])) {
      if (spillLiveness[succ].in) continue;
      // The spill slot liveness is live out of the block, but not live into
      // this successor. This can only happen if the block has multiple
      // successors (which implies the successor has only one predecessor, since
      // we've split critical edges). We need to insert a stack pointer
      // adjustment at the beginning of the successor.
      assertx(succs(unit.blocks[b]).size() > 1);
      assertx(state.preds[succ].size() == 1);

      auto& succSpill = spillLiveness[succ];
      if (succSpill.begin) {
        // If we're going to need a stack pointer adjustment later in the
        // successor, we can just elide it by forgoing the adjustment at the
        // entry of the successor.
        succSpill.in = true;
        succSpill.begin.reset();
      } else {
        // Otherwise insert the adjustment at the beginning of the successor,
        // skipping over a landingpad instruction if present. Since this can add
        // instructions, we need to fix up the instruction offsets in the
        // successor's state (we already know that succSpill.begin isn't set).
        auto const idx
          = size_t{unit.blocks[succ].code[0].op == Vinstr::landingpad};
        auto const succAdded = modify(succ, idx, idx, spillSpace);
        if (succSpill.end) *succSpill.end += succAdded;
      }
    }
  }
}

// Look in the given block for any jccs which aren't at the end and have an
// invalid next label. If so, split the block into two at the jcc.
void split_block(State& state, Vlabel block) {
  auto& unit = state.unit;

  jit::vector<Vinstr> orig;
  orig.swap(unit.blocks[block].code);

  for (auto const& inst : orig) {
    unit.blocks[block].code.emplace_back(inst);

    // If the jcc has an invalid next label, that means it marks a place where
    // the block should be split.
    if (inst.op == Vinstr::jcc && !inst.jcc_.targets[0].isValid()) {
      auto const newBlock = unit.makeBlock(
        unit.blocks[block].area_idx,
        unit.blocks[block].weight
      );
      unit.blocks[block].code.back().jcc_.targets[0] = newBlock;
      block = newBlock;
    }
  }
}

// Split side exit instructions into a more explicit form. A side exit
// instruction is something like fallbackcc or bindjcc, which conditionally
// exits the unit (without any explicit control flow). We might have to insert
// stack pointer adjustment code when we exit the unit, so we need a place to
// insert the adjustments. Change these instructions into a conditional jump to
// a block which uses their unconditional version. After inserting adjustment
// code, we may have not inserted anything, and we can re-collapse them. Return
// true if we made any changes.
bool split_side_exits(State& state) {
  auto& unit = state.unit;

  jit::vector<Vlabel> blocksToSplit;
  for (auto const b : state.rpo) {
    for (size_t i = 0; i < unit.blocks[b].code.size(); ++i) {
      auto const makeExitBlock = [&] (const Vinstr& exit) {
        assertx(isBlockEnd(exit));

        // Make a new exit block and populate it with the exit instruction
        auto const target = unit.makeBlock(AreaIndex::Cold, 0);
        auto& inst = unit.blocks[b].code[i];
        auto& code = unit.blocks[target].code;
        code.emplace_back(exit);
        code.back().set_irctx(inst.irctx());

        // Change the side-exit instruction to be a conditional jump to the
        // block (or fall through). Note that this breaks vasm invariants, as we
        // might have a jcc in the middle of a block now. This will be fixed up
        // afterwards. We set the next label as invalid as the marker to
        // split_block() that the block should be split here.
        inst.jcc_ = jcc{
          getConditionCode(inst),
          getSFUseReg(inst),
          {Vlabel{}, target}
        };
        inst.op = Vinstr::jcc;
        // Mark this block as needing splitting at jccs.
        blocksToSplit.emplace_back(b);
      };

      auto const& inst = unit.blocks[b].code[i];
      if (inst.op == Vinstr::fallbackcc) {
        makeExitBlock(
          fallback{
            inst.fallbackcc_.target,
            inst.fallbackcc_.spOff,
            inst.fallbackcc_.trflags,
            inst.fallbackcc_.args
          }
        );
      } else if (inst.op == Vinstr::bindjcc) {
        makeExitBlock(
          bindjmp{
            inst.bindjcc_.target,
            inst.bindjcc_.spOff,
            inst.bindjcc_.trflags,
            inst.bindjcc_.args
          }
        );
      } else if (inst.op == Vinstr::jcci) {
        auto const target = inst.jcci_.target;
        auto const ctx = inst.irctx();
        makeExitBlock(jmpi{inst.jcci_.taken});
        unit.blocks[b].code.emplace_back(jmp{target});
        unit.blocks[b].code.back().set_irctx(ctx);
      }
    }
  }

  // Split any block which now has embedded jccs. This is a rare case where
  // we've changed the CFG, so we need to recalculate RPO and predecessor
  // information.
  if (!blocksToSplit.empty()) {
    for (auto const b : blocksToSplit) split_block(state, b);
    compute_rpo(state);
    state.preds = computePreds(unit);
    return true;
  }

  return false;
}

// Turn spills into load/store instructions, adjusting the stack pointer where
// appropriate.
void lower_spills(State& state) {
  assertx((state.numSpillSlots % 2) == 0);

  // Calculate total space on the stack used by spills.
  auto const spillSpace =
    state.numSpillSlots * 8 + state.numWideSpillSlots * 16;

  assertx(IMPLIES(!state.abi.canSpill, spillSpace == 0));
  assertx((spillSpace % 16) == 0);

  // Common case, no spills.
  if (spillSpace == 0) return;

  // We need to split side exit instructions (which implicitly leave the unit
  // intra-block) into explicit jumps to exit blocks. We might have to insert
  // stack pointer adjustment code in those side-exits.
  auto const split = split_side_exits(state);
  // Insert any stack pointer adjustments
  insert_sp_adjustments(state, spillSpace);
  // Actually transform the spills
  materialize_spills(state, spillSpace);

  // If we split any side exits, try to unsplit any that we can again. If we
  // didn't insert any adjustment code, they can be transformed back. This can
  // change the CFG again, but nothing after this needs anything but RPO
  // information right now.
  if (split) {
    optimizeExits(state.unit);
    optimizeJmps(state.unit);
    compute_rpo(state);
  }

  // Do a second stack pointer offset calculation which will check if the
  // adjustment code kept the stack pointer in a consistent state.
  if (debug) calculate_sp_offsets(state);
}

void lower_ssa(State& state) {
  auto& unit = state.unit;

  // First pass: Remove conjures in entry block, rewrite instructions to take
  // physical registers and rewrite pseudo instructions back to their original.
  for (auto const b : state.rpo) {
    size_t i = 0;
    // While this is true, we're in the region of the entry block which might
    // have inserted conjures.
    auto inEntryConjures = b == unit.entry;

    while (i < unit.blocks[b].code.size()) {
      auto& inst = unit.blocks[b].code[i];
      // While we're in the "conjure region", remove any conjures which look
      // like we inserted them.
      if (inEntryConjures && inst.op == Vinstr::conjure) {
        auto const erase = [&]{
          auto const r = inst.conjure_.c;
          if (r.isPhys()) return false;
          auto const& info = reg_info(state, r);
          return info.precolor != InvalidReg;
        }();
        if (erase) {
          vmodify(unit, b, i, [] (Vout&) { return 1; });
          continue;
        }
      }
      // Leave the "conjure region" if we encounter anything other than a
      // conjure, copy, or copyargs.
      inEntryConjures = inEntryConjures &&
        (inst.op == Vinstr::copy || inst.op == Vinstr::copyargs);

      // Rewrite other instructions to take the physical registers the Vregs
      // have been colored to.
      auto const rewrite = [&] (Vreg r) -> Vreg {
        if (r.isPhys()) return r;
        auto const& info = reg_info(state, r);
        if (is_spill(info.regClass)) return r;
        if (info.regClass == RegClass::SF) return state.abi.sf.choose();
        auto const c = color_reg(info.color);
        assertx(c != state.scratch);
        return c;
      };
      visitRegsMutable(unit, inst, rewrite, rewrite);

      // Change pseudos to their original form.
      restore_pseudo(state, inst);
      ++i;
    }
  }

  // Second pass: lower copy-ish instructions (including phis) to copy and copy2
  // instructions, and optimize them away if they're no-ops.
  for (auto const b : state.rpo) {
    for (auto i = unit.blocks[b].code.size(); i > 0; --i) {
      lower_copies(state, b, i - 1);
    }
  }

  // Third pass: Remove (now useless) phidef instructions
  for (auto const b : state.rpo) {
    auto const& inst = unit.blocks[b].code.front();
    if (inst.op != Vinstr::phidef) continue;
    vmodify(unit, b, 0, [] (Vout&) { return 1; });
  }

  lower_spills(state);

  assertx(check(unit));
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

  // The spiller's SSA restoration may have invalidated liveness information, so
  // recalculate it.
  calculate_liveness(state);
  assign_colors(state);
  optimize_colors(state);
  lower_ssa(state);

  printUnit(kVasmRegAllocLevel, "after vasm-graph-color", unit);
}

//////////////////////////////////////////////////////////////////////

}}
