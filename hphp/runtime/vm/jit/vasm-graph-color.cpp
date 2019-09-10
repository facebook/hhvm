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
 * - The unit is prepared by materializing any constants into actual
     Vregs.
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
 *   for it. Walk the unit and assign registers for every Vreg using a
 *   heuristic to try to minimize copies. A Vreg can be in different physical
 *   registers at different points in the unit. Spill slots are assigned
 *   using a separate algorithm.
 *
 * - Lower out of SSA. Lower phis into register and spill slot moves. Optimize
 *   away no-op copies. Assign stack pointer adjustments as necessary for spill
 *   slots. At this point the unit no longer has any virtual registers and is
 *   fully register allocated.
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
// slot, or a physical register. Which subset is valid depends on the Vreg's
// RegClass.
struct None {};
struct SpillSlot { size_t slot; };
struct SpillSlotWide { size_t slot; };
using Color = boost::variant<None, PhysReg, SpillSlot, SpillSlotWide>;

// State about each Vreg. Instead of separate data-structures, all Vreg
// information is concentrated in this one data-structure.
struct RegInfo {
  RegClass regClass = RegClass::Any;
  // Color assigned to this Vreg at this particular moment (during
  // coloring, Vregs can be assigned different colors at different
  // points in the unit). Spills will always keep the same color.
  Color color;
  // Index into the penalty vector table
  size_t penaltyIdx = 0;
  // Can this Vreg be potentially rematerialized (instead of reloaded)
  // by this instruction? This field is calculated lazily and then
  // cached. Even if there's an instruction here, we may still need to
  // do context sensitive checks to see if its usuable.
  folly::Optional<Vinstr> cachedRemat;
};

using BlockVector = jit::vector<Vlabel>;
using WeightMap = jit::fast_map<Vreg, uint64_t>;
using PhiWeightVector = jit::vector<folly::Optional<uint64_t>>;
using PenaltyVector = PhysReg::Map<int64_t>;

// Information about each inferred loop. A loop is represented by its header
// block.
struct LoopInfo {
  VregSet uses;            // Vregs used inside
  size_t gpPressure = 0;   // Max GP and SIMD pressure
  size_t simdPressure = 0;
  size_t depth = 0;        // Nesting depth (this will always be at least one
                           // once the information is initialized).
};

// Cached def/use/across operands for an instruction.
struct CachedOperands {
  VregSet defs;
  VregSet uses;
  VregSet acrosses;
};

// Global allocator state.
struct State {
  Vunit& unit;
  const Abi& abi;

  // CFG information
  BlockVector rpo;
  jit::vector<size_t> rpoOrder;
  PredVector preds;

  // Registers available for selection by the allocator.
  const RegSet gpUnreserved;
  const RegSet simdUnreserved;
  // Registers not available for selection. These will be left untouched if
  // present.
  const RegSet reservedRegs;
  // SIMD scratch register for resolving shuffles.
  const PhysReg scratch;

  // Liveness information
  jit::vector<VregSet> liveIn;
  jit::vector<VregSet> liveOut;

  // Loop information. A loop is represented by its header block.
  jit::fast_map<Vlabel, LoopInfo> loopInfo;
  // Map of block to the inner-most loop it belongs to. Blocks not contained
  // within a loop will not be present.
  jit::fast_map<Vlabel, Vlabel> blockToLoop;
  // Map of a loop to all of the loops its contained within.
  std::unordered_multimap<Vlabel, Vlabel> loopMembership;

  // Cached operands. The "id" field in a Vinstr is an index into this
  // table. Since 0 means no cached information, the first entry is
  // never used. We use a deque to guarantee that references will
  // never be invalidated.
  jit::deque<CachedOperands> cachedOperands;

  // Calculate penalty vectors. Different Vregs may share the same
  // penalty vector.
  jit::vector<PenaltyVector> penalties;

  // Vreg state
  jit::vector<folly::Optional<RegInfo>> regInfo;

  // Pre-calculated mapping of spill Vregs to spill slots
  jit::fast_map<Vreg, Color> spillColors;

  // The number of non-wide and wide spill slots allocated. This determines how
  // much space to reserve in the stack.
  size_t numSpillSlots = 0;
  size_t numWideSpillSlots = 0;

  // If we spilled anything in this unit
  bool spilled = false;
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

std::string show(const PenaltyVector& v) {
  std::string out;
  auto first = true;
  for (auto const r : v) {
    if (!first) out += ", ";
    first = false;
    out += folly::sformat("{}: {}", show(r), v[r]);
  }
  return folly::sformat("[{}]", out);
}

std::string show(const Vunit& unit, const RegInfo& info) {
  return folly::sformat(
    "Class: {:10}, Color: {:6}, Penalty: {:3}, Mat: ({})",
    show(info.regClass),
    show(info.color),
    info.penaltyIdx,
    info.cachedRemat ? show(unit, *info.cachedRemat) : "-"
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
      if (b >= info.size() || info[b].empty()) continue;
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
    "Spill Colors:         {}\n"
    "Reg Info:\n{}"
    "Live In:\n{}"
    "Live Out:\n{}"
    "Loop Info:\n{}"
    "Block To Loop:\n{}"
    "Penalties:\n{}",
    show(state.gpUnreserved),
    show(state.simdUnreserved),
    show(state.reservedRegs),
    show(state.scratch),
    state.numSpillSlots,
    state.numWideSpillSlots,
    show(state.rpo),
    [&]{
      using namespace folly::gen;
      return folly::sformat(
        "{{{}}}",
        from(state.spillColors)
        | map([] (std::pair<Vreg, Color> p) {
            return folly::sformat("{}: {}", show(p.first), show(p.second));
          })
        | unsplit<std::string>(", ")
      );
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
    [&]{
      std::string str;
      for (size_t i = 1; i < state.penalties.size(); ++i) {
        auto const& v = state.penalties[i];
        str += folly::sformat("  {:3} -> {}\n", i, show(v));
      }
      return str;
    }()
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

// Returns true if this Vreg is a physical register which is
// reserved. Such a register is generally ignored by the allocator.
bool is_ignored(const State& state, Vreg r) {
  return r.isPhys() && state.reservedRegs.contains(r.physReg());
}

// Retrieve the reg-class for this Vreg. This works for physical
// registers as well.
RegClass reg_class(const State& state, Vreg r) {
  assertx(!is_ignored(state, r));
  if (!r.isPhys()) return reg_info(state, r).regClass;
  switch (r.physReg().type()) {
    case PhysReg::GP:   return RegClass::GP;
    case PhysReg::SIMD: return RegClass::SIMD;
    case PhysReg::SF:   return RegClass::SF;
  }
  always_assert(false);
}

// Retrieve the assigned color and reg-class for this Vreg. This works
// for physical registers as well (their color will always be
// themselves).
struct RegColorInfo {
  Color color;
  RegClass regClass;
};

RegColorInfo reg_color_info(const State& state, Vreg r) {
  if (r.isPhys()) return { r.physReg(), reg_class(state, r) };
  auto const& info = reg_info(state, r);
  return { info.color, info.regClass };
}

// Is this reg-class a spill?
bool is_spill(RegClass cls) {
  return cls == RegClass::Spill || cls == RegClass::SpillWide;
}

// Does this reg-class require coloring? Both registers and spills
// need to be colored, but status flags do not.
bool is_colorable(RegClass cls) {
  switch (cls) {
    case RegClass::AnyNarrow:
    case RegClass::GP:
    case RegClass::SIMD:
    case RegClass::SIMDWide:
    case RegClass::Spill:
    case RegClass::SpillWide:
      return true;
    case RegClass::SF:
      return false;
    case RegClass::Any:
      break;
  }
  always_assert(false);
}

// Does this reg-class require coloring, and is not a spill?
bool is_colorable_reg(RegClass cls) {
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
bool is_color_reg(Color c) { return boost::get<PhysReg>(&c); }
bool is_color_spill_slot(Color c) { return boost::get<SpillSlot>(&c); }
bool is_color_spill_slot_wide(Color c) { return boost::get<SpillSlotWide>(&c); }

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
// Operand visitors

/*
 * We need to retrieve the operands for instructions many times. Since
 * the operands are often the same, we can cache them. We store the
 * cached operands in State, and use the "id" field in Vinstr as an
 * index into the cached information. If "id" is 0, it means nothing
 * is cached and we need to visit the operands.
 *
 * If the operands are changed, we need to call
 * invalidate_cached_operands() to reset "id" back to 0, which will
 * force re-caching of the information.
 *
 * The VregSets already have ignored registers removed from them.
 *
 * Note: we never remove information from the cached operands table,
 * so its always safe to retain references to the VregSets, even after
 * an invalidation.
 */

struct CacheOperandsVisitor {
  CacheOperandsVisitor(VregSet& defs,
                       VregSet& uses,
                       VregSet& acrosses,
                       const State& state)
    : defs{defs}
    , uses{uses}
    , acrosses{acrosses}
    , state{state} {}

  template<class T> void imm(const T&) {}

  template <typename T> void def(T t) {
    visit(state.unit, t, [this] (Vreg r) { addDef(r); });
  }
  template <typename T, typename U> void defHint(T t, U) { def(t); }

  template <typename T> void across(T t) {
    visit(state.unit, t, [this] (Vreg r) { addAcross(r); });
  }

  template <typename T> void use(T t) {
    visit(state.unit, t, [this] (Vreg r) { addUse(r); });
  }
  template <typename T, typename U> void useHint(T t, U) { use(t); }

  void addDef(Vreg r) {
    if (is_ignored(state, r)) return;
    defs.add(r);
  }
  void addUse(Vreg r) {
    if (is_ignored(state, r)) return;
    uses.add(r);
  }
  void addAcross(Vreg r) {
    if (is_ignored(state, r)) return;
    acrosses.add(r);
    uses.add(r);
  }

  VregSet& defs;
  VregSet& uses;
  VregSet& acrosses;
  const State& state;
};

// Do the work of actually building the cached information.
NEVER_INLINE
void cache_operands(State& state, Vinstr& inst) {
  VregSet defs, uses, acrosses;

  // First get the normal operands
  CacheOperandsVisitor v{defs, uses, acrosses, state};
  visitOperands(inst, v);

  // Then add in any implicit ones
  RegSet implicitDefs, implicitUses, implicitAcrosses;
  getEffects(state.abi, inst, implicitUses, implicitAcrosses, implicitDefs);

  implicitDefs.forEach(
    [&] (PhysReg r) {
      if (state.reservedRegs.contains(r)) return;
      defs.add(r);
    }
  );
  implicitUses.forEach(
    [&] (PhysReg r) {
      if (state.reservedRegs.contains(r)) return;
      uses.add(r);
    }
  );
  implicitAcrosses.forEach(
    [&] (PhysReg r) {
      if (state.reservedRegs.contains(r)) return;
      acrosses.add(r);
      uses.add(r);
    }
  );

  // We never remove any information from the table, but the index is
  // 32-bits so we should never run out of space.
  always_assert(
    state.cachedOperands.size() <
    std::numeric_limits<decltype(inst.id)>::max()
  );

  inst.id = state.cachedOperands.size();
  assertx(inst.id > 0);
  state.cachedOperands.emplace_back(
    CachedOperands{
      std::move(defs),
      std::move(uses),
      std::move(acrosses)
    }
  );
}

// Invalidate any cached operands for an instruction.
void invalidate_cached_operands(Vinstr& inst) {
  inst.id = 0;
}

// Cached operand getters. Check if we need to cache the operands and
// then return a reference to the appropriate set. These references
// are safe to hold onto indefinitely, as they'll never be removed (we
// only add information to the table).

const VregSet& defs_set_cached(State& state, Vinstr& inst) {
  if (!inst.id) cache_operands(state, inst);
  return state.cachedOperands[inst.id].defs;
}

const VregSet& uses_set_cached(State& state, Vinstr& inst) {
  if (!inst.id) cache_operands(state, inst);
  return state.cachedOperands[inst.id].uses;
}

const VregSet& acrosses_set_cached(State& state, Vinstr& inst) {
  if (!inst.id) cache_operands(state, inst);
  return state.cachedOperands[inst.id].acrosses;
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

// Return true if the given Vreg is live in (that is, before the instruction
// executes) at the instruction given by the block and index. This merely looks
// for a usage of that Vreg in the future, so it may return true if you give it
// a Vreg which isn't yet defined.
bool live_in_at(State& state, Vreg reg, Vlabel b, size_t i) {
  assertx(b < state.unit.blocks.size());
  assertx(i <= state.unit.blocks[b].code.size());
  assertx(!is_ignored(state, reg));

  // If the position is the beginning of the block, we can just use the
  // pre-calculated liveness information.
  if (i == 0) return state.liveIn[b][reg];

  // Otherwise walk through the block from the specified position and look for a
  // use of that Vreg. If we find one, it must be live (assuming its defined at
  // the specified position).
  auto& unit = state.unit;
  auto& block = unit.blocks[b];
  for (; i < block.code.size(); ++i) {
    auto& inst = block.code[i];
    if (uses_set_cached(state, inst)[reg]) return true;
    if (reg.isPhys()) {
      if (defs_set_cached(state, inst)[reg]) return false;
    }
  }
  // If we reach the end of the block without finding a usage, it may still be
  // used in a successor, so use the pre-calculated live-out information.
  return state.liveOut[b][reg];
}

// Calculate liveness in the traditional dataflow way. There's more efficient
// algorithms leveraging SSA but they require tracking use and def positions and
// it doesn't seem worth it (this is fast enough in practice).
void calculate_liveness(State& state) {
  auto& unit = state.unit;

  state.liveIn.resize(unit.blocks.size());
  state.liveOut.resize(unit.blocks.size());

  jit::vector<VregSet> gen(unit.blocks.size());
  jit::vector<VregSet> kill(unit.blocks.size());

  dataflow_worklist<size_t, std::less<size_t>> worklist(state.rpo.size());
  for (size_t i = 0; i < state.rpo.size(); ++i) {
    auto const b = state.rpo[i];
    auto& block = unit.blocks[b];
    auto& g = gen[b];
    auto& k = kill[b];
    for (auto& inst : boost::adaptors::reverse(block.code)) {
      for (auto const r : defs_set_cached(state, inst)) {
        if (reg_class(state, r) == RegClass::SF) continue;
        k.add(r);
        g.remove(r);
      }
      for (auto const r : uses_set_cached(state, inst)) {
        if (reg_class(state, r) == RegClass::SF) continue;
        g.add(r);
      }
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

  // Only physical registers should be live-in to the entry block.
  assertx(
    (state.liveIn[state.unit.entry] -
     VregSet{state.gpUnreserved | state.simdUnreserved}).none()
  );
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

// Identify the loops within an unit (only their relationship to each
// other).
void find_loops(State& state) {
  auto& unit = state.unit;

  // Find the loops
  auto const backEdges =
    findBackEdges(unit, state.rpo, findDominators(unit, state.rpo));
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
  for (auto const& p : loopBlocks) {
    for (auto const b : p.second) {
      auto const it = state.loopInfo.find(b);
      if (it == state.loopInfo.end()) continue; // Not a loop header
      ++it->second.depth;
      state.loopMembership.emplace(b, p.first);
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
}

// Calculate register uses and pressure information for any loops
// within the unit.
void calculate_loop_info(State& state) {
  // If there's no loops, nothing to do.
  if (state.loopInfo.empty()) return;

  auto& unit = state.unit;

  // Calculate the maximum register pressure inside each loop
  // alongside which registers are used in each loop. The heuristic
  // which wants to know which registers are used in each loop
  // performs better if you ignore copies whose dest is not used in
  // the loop. Because of this the analysis is flow sensitive, so we
  // loop until we hit a fixed point.

  // Record that a register is used in a particular loop
  auto const markUse = [&] (Vreg r, Vlabel loop) {
    if (is_ignored(state, r)) return false;
    if (reg_class(state, r) == RegClass::SF) return false;
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
          return std::make_pair(
            state.loopMembership.end(),
            state.loopMembership.end()
          );
        }
        auto const range = state.loopMembership.equal_range(it->second);
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
        if (is_ignored(state, r)) return;
        switch (reg_class(state, r)) {
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
        if (is_ignored(state, r)) return;
        switch (reg_class(state, r)) {
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
      for (auto const r : state.liveOut[b]) liveUse(r);

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
      for (auto& inst : boost::adaptors::reverse(unit.blocks[b].code)) {
        switch (inst.op) {
          case Vinstr::copy:
            assertx(acrosses_set_cached(state, inst).none());
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
            assertx(acrosses_set_cached(state, inst).none());
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
            assertx(acrosses_set_cached(state, inst).none());
            assertx(defs_set_cached(state, inst).none());
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
            for (auto const r : defs_set_cached(state, inst)) liveUse(r);
            for (auto const r : acrosses_set_cached(state, inst)) {
              for (auto loop = loops.first; loop != loops.second; ++loop) {
                changed |= markUse(r, loop->second);
              }
              liveUse(r);
            }
            update();
            for (auto const r : defs_set_cached(state, inst)) liveDef(r);
            for (auto const r : uses_set_cached(state, inst)) {
              for (auto loop = loops.first; loop != loops.second; ++loop) {
                changed |= markUse(r, loop->second);
              }
              liveUse(r);
            }
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

// Return "weight" of a block, an approximation of how many times it will be
// executed.
int64_t block_weight(const State& state, Vlabel b) {
  // Force the weight to always be greater than 0. To minimize
  // distortions in the proportionality between blocks, multiply the
  // actual value by 100.
  return std::max<int64_t>(state.unit.blocks[b].weight * 100, 1);
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
    abi.gpUnreserved,
    abi.simdUnreserved - scratch,
    (abi.all() - (abi.gpUnreserved | abi.simdUnreserved)) | scratch,
    scratch
  };

  // Pre-size the table to avoid excessive resizing.
  state.regInfo.reserve(unit.next_vr * 2);
  // Insert dummy cached operand entry since 0 is not a valid index.
  state.cachedOperands.emplace_back();

  compute_rpo(state);
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
 *
 * As an optimization, we look for "trivial" constants. These are
 * constants used exactly one place (not within a loop). Such
 * constants do not require the above mentioned dataflow and can be
 * placed immediately before their use.
 */

struct PlaceConstantsBlockInfo {
  VregSet stopSink;
  VregSet liveIn;
};

// Calculate sink stop and liveness information for each constant for every
// block, returning an empty vector if there's no constants.
std::pair<
  jit::vector<PlaceConstantsBlockInfo>,
  jit::vector<VregSet>
>
compute_place_constants_block_info(State& state) {
  auto& unit = state.unit;
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

  // All constants with a use
  VregSet allUsed;
  // All constants with more than one use, or a use within a loop
  VregSet nonTrivial;
  // Per-block trivial uses
  jit::vector<VregSet> trivialUses(unit.blocks.size());

  // Populate the constant usages for each block. Since this is a backwards
  // dataflow, start the worklist with successor-less blocks (the exits).
  for (size_t i = 0; i < rpo.size(); ++i) {
    auto const b = rpo[i];
    auto& block = unit.blocks[b];
    auto& uses = blockUses[b];

    for (auto& inst : block.code) {
      // Special case: If we have a copy moving a constant into an
      // ignored register, turn it into the appropriate ldimm
      // instruction with that register as the destination. Otherwise
      // we'll materialize the constant into an available register and
      // then immediately copy it into the ignored register, which is
      // inefficient. This happens a lot in stubs.
      if (inst.op == Vinstr::copy &&
          is_ignored(state, inst.copy_.d) &&
          unit.regToConst.count(inst.copy_.s)) {
        auto const d = inst.copy_.d;
        auto const it = unit.regToConst.find(inst.copy_.s);
        assertx(it != unit.regToConst.end());
        auto const& vconst = it->second;
        // TODO (T37584483): Undef constants can be dealt with better here.
        switch (vconst.kind) {
          case Vconst::Quad:
          case Vconst::Double:
            inst.op = Vinstr::ldimmq;
            inst.ldimmq_.s = uint64_t(vconst.val);
            inst.ldimmq_.d = d;
            break;
          case Vconst::Long:
            inst.op = Vinstr::ldimml;
            inst.ldimml_.s = int32_t(vconst.val);
            inst.ldimml_.d = d;
            break;
          case Vconst::Byte:
            inst.op = Vinstr::ldimmb;
            inst.ldimmb_.s = uint8_t(vconst.val);
            inst.ldimmb_.d = d;
            break;
        }
        invalidate_cached_operands(inst);
      } else {
        for (auto const r : uses_set_cached(state, inst)) {
          if (unit.regToConst.count(r)) uses.add(r);
        }
      }

      // A constant should never have a definition (before we place them).
      if (debug) {
        for (auto const r : defs_set_cached(state, inst)) {
          always_assert(!unit.regToConst.count(r));
        }
      }
    }

    // Update triviality information. Mark a constant as trivial if we
    // haven't seen any previous uses. If we detect a second use, mark
    // it as non-trivial. Since we don't update previous block
    // information, it may be incorrect after this. We'll fix it up
    // later by subtracting "nonTrivial" from it.
    auto const inLoop = block_loop_depth(state, b) > 0;
    for (auto const r : uses) {
      if (inLoop || allUsed[r]) {
        allUsed.add(r);
        nonTrivial.add(r);
        trivialUses[b].remove(r);
        continue;
      }
      allUsed.add(r);
      trivialUses[b].add(r);
    }

    visited[b] = false;
    if (succs(block).size() == 0) worklist.push(i);
  }

  // We only need the dataflow if there's any non-trivial usages.
  if (nonTrivial.none()) {
    // Either the unit has no usages at all, or all usages are
    // trivial.
    if (allUsed.none()) return {};
    return {{}, std::move(trivialUses)};
  }

  while (!worklist.empty()) {
    auto const b = rpo[worklist.pop()];
    auto const& block = unit.blocks[b];

    // Fixup the per-block trivial information by removing constants
    // known to be non-trivial.
    trivialUses[b] -= nonTrivial;

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

    auto const uses = blockUses[b] & nonTrivial;
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
        for (auto const r : uses) mutated->nearestUse.insert_or_assign(r, b);
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
      for (auto const r : uses) newState.nearestUse.insert_or_assign(r, b);

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

  return {std::move(outInfo), std::move(trivialUses)};
}

// Find the last index in the block where it's not legal to insert the
// ldimm. IE, avoid the special instructions which end blocks which we
// cannot insert anything after.
size_t find_first_invalid_block_index(const Vblock& block) {
  auto const valid = [] (Vinstr::Opcode op) {
    return op != Vinstr::unwind &&
      op != Vinstr::syncpoint &&
      op != Vinstr::nothrow &&
      op != Vinstr::fallthru;
  };
  auto i = block.code.size();
  while (i > 0 && !valid(block.code[i-1].op)) --i;
  return i;
};

// Look for any blocks where all the successors of the block define
// the same constant at the front. Hoist those definitions up into the
// end of the block. Constant materialization can sometimes cause such
// definitions, and its difficult to modify the dataflow to not do
// so. Instead just fix it up after the fact.
void hoist_constants(State& state) {
  auto& unit = state.unit;

  // NB: Be careful with references to things inside the unit here,
  // since they can be invalidated whenever we call vmodify.
  for (auto const b : state.rpo) {
    auto successors = succs(unit.blocks[b]);
    // Can only happen if we have more than one successor
    if (successors.size() < 2) continue;

    // Avoid self-loops
    if (std::any_of(successors.begin(),
                    successors.end(),
                    [&] (Vlabel s) { return s == b; })) {
      continue;
    }

    // Search a block for a constant defining the given Vreg. We stop
    // looking if we encounter a non-constant materialization
    // instruction.
    auto const findVreg = [&] (Vreg r, Vlabel succ) -> folly::Optional<size_t> {
      size_t idx = 0;
      auto const& code = unit.blocks[succ].code;
      auto const limit = code.size();
      while (idx < limit) {
        auto const& inst = code[idx];
        switch (inst.op) {
          case Vinstr::ldimmq:
            if (inst.ldimmq_.d == r) return idx;
            break;
          case Vinstr::ldimml:
            if (inst.ldimml_.d == r) return idx;
            break;
          case Vinstr::ldimmb:
            if (inst.ldimmb_.d == r) return idx;
            break;
          default:
            return folly::none;
        }
        ++idx;
      }
      return folly::none;
    };

    VregSet hoisted;

    // Attempt to hoist a Vreg (defined by the given instruction from
    // one of the successors) into the parent block.
    auto const hoist = [&] (Vreg r, const Vinstr& inst) {
      // Check all the other successors and make sure its also defined there.
      for (size_t succIndex = 1; succIndex < successors.size(); ++succIndex) {
        if (!findVreg(r, successors[succIndex])) return;
      }

      // It is, so copy the instruction into the end of the parent
      // block.
      vmodify(
        unit, b, find_first_invalid_block_index(unit.blocks[b]) - 1,
        [&] (Vout& v) { v << inst; return 0; }
      );
      // vmodify could have shift data, so re-calculate successors
      // (which is a pointer).
      successors = succs(unit.blocks[b]);
      hoisted.add(r);
    };

    // For every constant defined in the first successor (an arbitrary
    // choise), attempt to hoist into the parent. We only examine
    // constants which are defined before any other instruction.
    //
    // hoist() can call vmodify, which can invalidate references in
    // the unit, so use indices.
    for (size_t idx = 0; idx < unit.blocks[successors[0]].code.size(); ++idx) {
      auto const& inst = unit.blocks[successors[0]].code[idx];
      switch (inst.op) {
        case Vinstr::ldimmq:
          hoist(inst.ldimmq_.d, inst);
          continue;
        case Vinstr::ldimml:
          hoist(inst.ldimml_.d, inst);
          continue;
        case Vinstr::ldimmb:
          hoist(inst.ldimmb_.d, inst);
          continue;
        default:
          break;
      }
      break;
    }

    // Didn't hoist anything, so we're done
    if (hoisted.none()) continue;

    // Otherwise we need to remove the now unneeded definitions from
    // the successors.
    for (size_t idx = 0; idx < successors.size(); ++idx) {
      auto const succ = successors[idx];
      for (auto const r : hoisted) {
        auto const constIdx = findVreg(r, succ);
        assertx(constIdx); // Should be there
        vmodify(unit, succ, *constIdx, [&] (Vout&) { return 1; });
      }
      // vmodify() might have invalidated successors (which is a
      // pointer), so re-acquire it.
      successors = succs(unit.blocks[b]);
    }
  }
}

// Materialize constants by placing instructions to create them at the
// appropriate points. The set of constants placed multiple times
// (thus needing SSA restoration) is returned.
VregSet place_constants(State& state) {
  auto& unit = state.unit;

  auto info = compute_place_constants_block_info(state);
  auto const& blockInfo = info.first;
  auto const& trivial = info.second;
  if (blockInfo.empty() && trivial.empty()) return {};

  assertx(trivial.size() == unit.blocks.size());

  auto const& rpo = state.rpo;
  auto const& rpoOrder = state.rpoOrder;
  auto const& preds = state.preds;

  // The set of constants in each block which can be sunk to the beginning of
  // that block.
  jit::vector<VregSet> inStates;
  // The set of constants in each block which can be sunk to the end of that
  // block.
  jit::vector<VregSet> outStates;

  // If we don't have block information, all uses are trivial so we
  // don't need dataflow.
  if (!blockInfo.empty()) {
    assertx(blockInfo.size() == unit.blocks.size());

    inStates.resize(unit.blocks.size());
    outStates.resize(unit.blocks.size());
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
     * (A constant should be placed in B if it can be sunk to the beginning of
     * B, is live in B, and is stopped in B. It should also be placed in B if it
     * can be sunk to the end of B, and can not be sunk to the beginning of all
     * of B's successors).
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
        // A constant can be trivially "sunk" to the beginning of the entry
        // block if its live.
        in = info.liveIn;
      } else {
        assertx(!predList.empty());

        // Find all the constants which can be sunk to the beginning of this
        // block. A constant can be sunk to the beginning of this block if it
        // can be sunk to the end of all of its predecessors.

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

      // From the constants which can be sunk to the beginning of the block,
      // find the subset which can be sunk to the end. A constant can be sunk
      // through the block if there's not a sink stop and if its still live in
      // the block.
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
  }

  VregSet allPlaced;
  VregSet multiplePlaced;
  VregSet placeThisInst;
  for (auto const b : rpo) {
    auto place = [&] {
      // If we have no in states, its because we didn't perform
      // dataflow above, which only happens if we only have trivial
      // uses. Therefore the trivial set is everything we might place.
      if (inStates.empty()) return trivial[b];
      assertx(inStates.size() == unit.blocks.size());
      assertx(outStates.size() == unit.blocks.size());

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
      return place | trivial[b];
    }();
    multiplePlaced |= (allPlaced & place);
    allPlaced |= place;

    auto block = &unit.blocks[b];
    assertx(!block->code.empty());

    auto stopIdx = find_first_invalid_block_index(*block);

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
        placeThisInst |= (place & uses_set_cached(state, block->code[i]));
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
          for (auto const r : placeThisInst) materialize(v, r);
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

  // Attempt to hoist constants
  hoist_constants(state);

  return multiplePlaced;
}

//////////////////////////////////////////////////////////////////////

void prepare_unit(State& state) {
  // Constant materialization requires knowing where any loops are.
  find_loops(state);

  // Materialize constants, which might result in a non-SSA unit.
  auto const toSSA = place_constants(state);
  if (toSSA.none()) return;

  // Restore SSA if necessary and create RegInfo for the new Vregs,
  // mirroring the info of the original Vregs (which have been
  // rewritten).
  auto const mappings = restoreSSA(state.unit, toSSA, state.rpo, 0);
  for (auto const& map : mappings) {
    if (!map.second.isPhys()) continue;
    reg_info_create(state, map.first);
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
 * Finally, we insert copies to resolve a few tricky cases. We ensure
 * that there's no copyargs between ignored and non-ignored
 * registers. We ensure that a copyargs only has all physical or all
 * virtual dests, not a mix of both. We ensure that a phidef/phijmp
 * pair doesn't copy between register classes, nor does it write to a
 * physical register. We resolve these cases by adding new virtual
 * registers as the dests, and then copying from those new registers
 * to the old dests. These restrictions make the spilling and phi
 * lowering logic simpler. We don't have to do this for copy2 because
 * unlike other copies it imposes a constraint and does not get
 * special treatment from the spiller.
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
    info.regClass = detail::reg_class(r);
    haveWide |= (info.regClass == RegClass::SIMDWide);
  };

  auto const updateUses = [&] (auto r) {
    if (r.isPhys()) return;
    auto& info = reg_info_create(state, r);
    auto const newClass = detail::reg_class(r);
    if (auto const c = merge(info.regClass, newClass)) {
      info.regClass = *c;
      haveWide |= (info.regClass == RegClass::SIMDWide);
    } else {
      incompatible.add(r);
    }
  };

  auto const processInst = [&] (const Vinstr& inst, bool skipDefs = false) {
    // NB: We can't use the cached operands here because we need to
    // know the specific kind of Vreg.
    visitUses(unit, inst, updateUses);
    if (!skipDefs) visitDefs(unit, inst, updateDefs);
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

  // Insert copies as necessary to ensure that a copyargs instruction
  // does not copy between a non-ignored register and an ignored
  // one. Also ensure that a copyargs only has all physical registers
  // as dests, or only all virtual ones. This makes the spilling logic
  // easier. Return true if anything was changed.
  auto const canonicalizeCopyArgs = [&] (Vlabel b, size_t instIdx) {
    auto& inst = unit.blocks[b].code[instIdx];
    if (inst.op != Vinstr::copyargs) return false;

    // If the source is an ignored register, we insert a copy before.
    jit::fast_map<Vreg, Vreg> precopy;
    // If the dest is an ignored register, we insert a copy after.
    jit::fast_map<Vreg, Vreg> postcopy;

    auto& srcs = unit.tuples[inst.copyargs_.s];
    auto& dsts = unit.tuples[inst.copyargs_.d];
    assertx(srcs.size() == dsts.size());

    if (debug) {
      // Sanity check that the copyargs doesn't have any duplicate
      // defs (even physical registers, whose semantics would be
      // unclear).
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
      if (is_ignored(state, s) == is_ignored(state, d)) continue;
      if (is_ignored(state, s)) {
        auto it = precopy.find(s);
        // The copyargs can take the same register multiple times as a source,
        // so re-use the same copied register for that.
        if (it == precopy.end()) it = precopy.emplace(s, unit.makeReg()).first;
        srcs[i] = it->second;
        invalidate_cached_operands(inst);
      }
      if (is_ignored(state, d)) {
        // Dests should be unique, however.
        auto const result = postcopy.emplace(d, unit.makeReg());
        assertx(result.second);
        dsts[i] = result.first->second;
        invalidate_cached_operands(inst);
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

    // If we added copies, return true. This will get called again and
    // we might handle the below case if necessary.
    if (!precopy.empty() || !postcopy.empty()) return true;

    // Ensure the dests are all physical registers or all virtual
    // ones. If not, add new virtual registers as dests, then do a
    // copy from those to the physical registers.
    auto foundPhys = false;
    auto foundNonPhys = false;
    for (size_t i = 0; i < srcs.size(); ++i) {
      auto const s = srcs[i];
      auto const d = dsts[i];
      assertx(is_ignored(state, s) == is_ignored(state, d));
      if (is_ignored(state, s)) continue;
      if (d.isPhys()) {
        foundPhys = true;
      } else {
        foundNonPhys = true;
      }
    }
    if (!foundPhys || !foundNonPhys) return false;

    jit::vector<std::pair<Vreg, Vreg>> physCopies;
    for (size_t i = 0; i < srcs.size(); ++i) {
      auto const d = dsts[i];
      if (!d.isPhys()) continue;
      auto const newReg = unit.makeReg();
      physCopies.emplace_back(newReg, d);
      dsts[i] = newReg;
      invalidate_cached_operands(inst);
    }
    assertx(!physCopies.empty());
    addCopies(physCopies, b, instIdx + 1);
    return true;
  };

  // First the flow insensitive part. Visit every instruction, using the uses
  // and defs to determine the register class.
  for (auto const b : state.rpo) {
    for (size_t i = 0; i < unit.blocks[b].code.size();) {
      // Deal with strange copyargs. If we changed anything, loop
      // without advancing i, so we'll process this copyargs again
      // (until we stop needing to change it).
      if (canonicalizeCopyArgs(b, i)) continue;
      // Analyze the use and defs. If there's no incompatibilities we're done.
      processInst(unit.blocks[b].code[i]);
      if (incompatible.none()) {
        ++i;
        continue;
      }

      // Otherwise we need to insert a copy before the use.
      jit::vector<std::pair<Vreg, Vreg>> copies;
      for (auto const r : incompatible) {
        assertx(!is_ignored(state, r));
        auto const newReg = unit.makeReg();
        // Clone the register, except for the RegClass, which is automatically
        // Any because its coming from a copy. We'll reprocess this
        // instruction to set the RegClass appropriately.
        auto& newInfo = reg_info_create(state, newReg);
        newInfo = reg_info(state, r);
        newInfo.regClass = RegClass::Any;
        copies.emplace_back(r, newReg);
      }
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
      invalidate_cached_operands(unit.blocks[b].code[i]);

      // Now process the copy and reprocess the instruction. This will set the
      // RegClass of the new (from copy) registers properly.
      incompatible.reset();
      processInst(unit.blocks[b].code[i-1]);
      processInst(unit.blocks[b].code[i], true);
      // Reprocessing shouldn't create new incompatibilities.
      assertx(incompatible.none());
      ++i;
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

  // Finally we have to ensure that the src/dest pairs of a phi have
  // the same register class. If not, we'll ensure they do by (yet
  // again) inserting a copy before the phi of the old src to a new
  // Vreg which has the same register class of the dest. The new Vreg
  // will then be the src in the phi. We also disallow a phi from
  // having a physical register as dest. We need to do all of this
  // because having to deal with moves between register classes during
  // a parallel phi copy greatly complicates the spilling logic and
  // the phi lowering logic.
  for (auto const b : state.rpo) {
    auto& firstInst = unit.blocks[b].code.front();
    if (firstInst.op == Vinstr::phidef) {
      auto& d = unit.tuples[firstInst.phidef_.defs];

      if (debug) {
        // Sanity check that the phi doesn't have duplicate defs. We
        // allow rvmfp() as a special case because sometimes we phi
        // FramePtrs and those are always assigned rvmfp().
        VregSet defs;
        for (size_t i = 0; i < d.size(); ++i) {
          if (d[i] == rvmfp()) continue;
          always_assert(!defs[d[i]]);
          defs.add(d[i]);
        }
      }

      // Check for any physical registers as dests. If any, set a new
      // virtual register as the dest, and emit a copy from it to the
      // physical register.
      jit::vector<std::pair<Vreg, Vreg>> copies;
      for (size_t i = 0; i < d.size(); ++i) {
        if (is_ignored(state, d[i])) continue;
        if (!d[i].isPhys()) continue;
        auto const newReg = unit.makeReg();
        reg_info_insert(state, newReg, RegInfo{reg_class(state, d[i])});
        copies.emplace_back(newReg, d[i]);
        d[i] = newReg;
        invalidate_cached_operands(firstInst);
      }

      if (!copies.empty()) addCopies(copies, b, 1);
    }

    auto& lastInst = unit.blocks[b].code.back();
    if (lastInst.op == Vinstr::phijmp) {
      auto& s = unit.tuples[lastInst.phijmp_.uses];
      auto const successorList = succs(unit.blocks[b]);
      assertx(successorList.size() == 1);
      auto const& phidef = unit.blocks[successorList[0]].code.front();
      assertx(phidef.op == Vinstr::phidef);
      auto const& d = unit.tuples[phidef.phidef_.defs];
      assertx(s.size() == d.size());

      jit::vector<std::pair<Vreg, Vreg>> copies;
      for (size_t i = 0; i < s.size(); ++i) {
        // Since we allow rvmfp() to appear multiple times in a
        // phidef, its not really clear how to properly emit code for
        // them. This isn't an issue if the source and dests are
        // always the same, which always happens now, so assert that
        // this remains the case.
        assertx((s[i] == rvmfp()) == (d[i] == rvmfp()));
        // We don't handle a phi between an ignored and non-ignored
        // register right now. We could, but it complicates things and
        // we don't generate it right now, so assert that this remains
        // the case.
        assertx(is_ignored(state, s[i]) == is_ignored(state, d[i]));
        if (is_ignored(state, s[i])) continue;

        auto const sCls = reg_class(state, s[i]);
        auto const dCls = reg_class(state, d[i]);
        if (sCls == dCls && !s[i].isPhys()) continue;

        // Create a new Vreg in the same RegClass as the dest and use that in
        // the phi instead. Add a copy between the old src and the new
        // Vreg. This lets the move between register classes be done outside of
        // the phi, instead of during it.
        auto const newReg = unit.makeReg();
        reg_info_insert(state, newReg, RegInfo{dCls});

        copies.emplace_back(s[i], newReg);
        s[i] = newReg;
        invalidate_cached_operands(lastInst);
      }

      if (!copies.empty()) {
        addCopies(copies, b, unit.blocks[b].code.size() - 1);
      }
    }
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
 * always be a free physical register to select. This property is completely
 * independent of the colors you choose and thus means the actual color
 * selection is arbitrary (ignoring physical register constraints).
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
 *
 * We can use register pressure calculations to determine which blocks can
 * actually trigger a spill. If the register pressure within a block never
 * exceeds the number of available registers, we know a spill will never be
 * generated. For these blocks, we can update the spilling state particularly
 * efficiently (its just the live-out Vregs all in registers). In the (common)
 * case where no blocks will cause a spill, we can skip the spiller entirely.
 */

/*
 * Materialization:
 *
 * When attempting to reload a spilled Vreg, we can instead
 * rematerialize the value in some cases. We look backwards from the
 * current Vreg use to its definition(s). We chain through copyish
 * instructions (including phis, which is how we can have multiple
 * definitions). If the definition is pure, and any sources in the
 * definition are still available, we can re-emit the definition
 * instead of a reload. If there are multiple definitions, they must
 * be all equivalent (same op and sources). If we successfully
 * rematerialize a Vreg, we should (hopefully) have a spill with no
 * associated reloads, which will be cleaned up by dead code
 * elimination.
 */

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

// Given a Vreg, search backwards in the unit to find its defining
// instruction. The search is started at the specified block and
// instruction index. Copyish instructions (including phis) are
// "transparent" to this search. If the Vreg has multiple definitions,
// and they are all equivalent, return an arbitrary one. If they are
// not equivalent, or if the definition cannot be found, return a
// nullptr, or a pointer to a static nop Vinstr (the difference is an
// internal implementation detail). The visited set is used to prevent
// unbounded recursion and should be initially empty.
const Vinstr* find_defining_inst_for_remat(
    State& state,
    Vlabel b,
    size_t instIdx,
    Vreg r,
    jit::vector<std::pair<Vlabel, Vreg>>& visited
) {
  auto& unit = state.unit;

  auto const compatible = [&] (Vreg r1, Vreg r2) {
    return compatible_reg_classes(
      reg_info(state, r1).regClass,
      reg_info(state, r2).regClass
    );
  };

  // Record whether we passed through a copyish instruction where the
  // source and dest were not compatible (different register
  // classes). If so, we'll only report the defining instruction if
  // its an immediate load. The copyish instruction may actually
  // change the value if they're incompatible (it could truncate or
  // zero-extend). The reason we still allow immediate loads is
  // because we often materialize constants into a different register
  // class than how its ultimately used.
  auto incompatibility = false;

  while (true) {
    assertx(!r.isPhys());

    // First find the correct block:
    //
    // From the current block, walk backwards through the predecessors
    // until we encounter a block where the desired Vreg is *not*
    // live-in. Since it's live-in for its successors, this means it
    // must be defined in that block. This lets us skip over most
    // blocks without examining their instructions.
    while (state.liveIn[b][r]) {
      auto const& preds = state.preds[b];
      assertx(!preds.empty());

      // If a block has multiple predecessors, choose the one with the
      // lowest RPO order. Not only does this prevent us from getting
      // loops, but it helps us find the def faster.
      auto bestOrder = state.rpoOrder[preds[0]];
      auto bestPred = preds[0];
      assertx(state.liveOut[preds[0]][r]);
      for (size_t i = 1; i < preds.size(); ++i) {
        auto const pred = preds[i];
        assertx(state.liveOut[pred][r]);
        if (state.rpoOrder[pred] < bestOrder) {
          bestOrder = state.rpoOrder[pred];
          bestPred = pred;
        }
      }
      // Should always decrease
      assertx(state.rpoOrder[bestPred] < state.rpoOrder[b]);
      b = bestPred;
      assertx(!unit.blocks[b].code.empty());
      instIdx = unit.blocks[b].code.size();
    }

    // Now that we have the correct block, find the correct
    // instruction within it.
    auto& block = unit.blocks[b];
    while (true) {
      always_assert(instIdx > 0);
      --instIdx;
      auto& inst = block.code[instIdx];

      // Quick check if this instruction defs the Vreg
      if (!defs_set_cached(state, inst)[r]) continue;

      // Process each instruction. If it's copyish, change the tracked
      // Vreg to be the source. We treat reload/spill/ssaalias as
      // copyish here because they don't change the actual value in
      // the Vreg.

      switch (inst.op) {
        case Vinstr::copy:
          assertx(inst.copy_.d == r);
          // We don't track physical registers (they're not
          // necessarily in SSA), so we cannot proceed
          // further. Otherwise start tracking the copy's source.
          if (inst.copy_.s.isPhys()) return nullptr;
          if (!compatible(inst.copy_.s, r)) incompatibility = true;
          r = inst.copy_.s;
          break;
        case Vinstr::reload:
          assertx(inst.reload_.d == r);
          r = inst.reload_.s;
          break;
        case Vinstr::spill:
          assertx(inst.spill_.d == r);
          r = inst.spill_.s;
          break;
        case Vinstr::ssaalias:
          assertx(inst.ssaalias_.d == r);
          r = inst.ssaalias_.s;
          break;
        case Vinstr::copyargs: {
          auto const& dsts = unit.tuples[inst.copyargs_.d];
          auto const& srcs = unit.tuples[inst.copyargs_.s];
          assertx(srcs.size() == dsts.size());
          auto DEBUG_ONLY found = false;
          for (size_t i = 0; i < dsts.size(); ++i) {
            if (dsts[i] != r) continue;
            if (srcs[i].isPhys()) return nullptr;
            if (!compatible(srcs[i], r)) incompatibility = true;
            r = srcs[i];
            if (debug) found = true;
            break;
          }
          // We should always find something because of the defs set
          // check above.
          assertx(found);
          break;
        }
        case Vinstr::phidef: {
          // A phidef is copyish, but we may have to examine more than
          // one source, hence the need for recursion.

          // We need a special return value to indicate that this
          // source should be ignored (because it's recursively
          // defined). Use a pointer to a static nop Vinstr to mean
          // this (nop cannot legitimately be returned because it
          // doesn't define anything).
          auto const ignore = [] () -> const Vinstr* {
            static const Vinstr nopInstr{nop{}};
            return &nopInstr;
          };

          // Phidef can only appear at the beginning of the block.
          assertx(instIdx == 0);

          // Self-recursion check. If we encounter the same phidef
          // with the same Vreg, we've looped and should ignore this
          // one.
          if (std::find(
                visited.begin(), visited.end(), std::make_pair(b, r)
              ) != visited.end()) {
            return ignore();
          }
          // Otherwise remember this phidef and Vreg pair for later
          // recursion checks.
          visited.emplace_back(b, r);
          SCOPE_EXIT { visited.pop_back(); };

          auto const& dsts = unit.tuples[inst.phidef_.defs];
          auto const& preds = state.preds[b];

          for (size_t i = 0; i < dsts.size(); ++i) {
            if (dsts[i] != r) continue;

            // Found a def. Recursively find the defining instruction
            // for each predecessor Vreg.
            const Vinstr* common = nullptr;
            for (auto const p : preds) {
              auto const& pred = unit.blocks[p];
              auto const& phijmp = pred.code.back();
              assertx(phijmp.op == Vinstr::phijmp);
              auto const s = unit.tuples[phijmp.phijmp_.uses][i];
              // Can't track phi-ing with a physical register
              if (s.isPhys()) return nullptr;
              auto const defining = find_defining_inst_for_remat(
                state,
                p,
                pred.code.size(),
                s,
                visited
              );
              if (!defining) return nullptr;
              // Skip over recursive definitions
              if (defining->op == ignore()->op) continue;
              if (!compatible(s, r) &&
                  defining->op != Vinstr::ldimmq &&
                  defining->op != Vinstr::ldimml &&
                  defining->op != Vinstr::ldimmb) {
                return nullptr;
              }
              // All defining instructions should be compatible.
              if (!common) {
                common = defining;
              } else if (
                !compare_insts_without_defs(unit, *common, *defining)
              ) {
                return nullptr;
              }
            }
            // This can only happen if all of the sources were
            // recursively defined.
            if (!common) return ignore();
            return common;
          }

          // We should always find a matching def because of the defs
          // set check above.
          always_assert(false);
        }
        case Vinstr::ldimmq:
        case Vinstr::ldimml:
        case Vinstr::ldimmb:
          // These can be used regardless of the incompatibility flag.
          return &inst;
        default:
          // The rest can only be used if we didn't encounter any
          // incompatible copies.
          return !incompatibility ? &inst : nullptr;
      }

      // If we reach here, we encountered a copyish instruction which
      // changed what Vreg we're looking for. We need to find the
      // block which defines the new Vreg, so break out of the
      // instruction loop and restart the block search above.
      break;
    }
  }
}

// Return an instruction to reload a spilled Vreg src into Vreg dst,
// possibly using rematerialization. If rematerialization is not
// possible, then a normal reload instruction is returned. If
// rematerialization is accomplished, set the rematerialized parameter
// to true (this avoids an annoying cyclic dependency on
// SpillerState). The search for the rematerialized instruction is
// started at the specified block and instruction offset. The "inReg"
// VregSet determines which Vregs are known to be in registers (and
// thus available) at the current program point. An instruction can
// only be rematerialized if all of its sources are available in
// physical registers.
Vinstr reload_with_remat(State& state,
                         bool& rematerialized,
                         Vlabel b,
                         size_t instIdx,
                         const VregSet& inReg,
                         Vreg src,
                         Vreg dst) {
  assertx(!src.isPhys());

  // We cache rematerialization resolutions to avoid redundant
  // lookups. If there's no cached information for this Vreg,
  // calculate it by searching for a defining instruction. If the
  // instruction isn't found, or if its always unusable (if its not
  // pure, for example), we'll just store a reload instead.
  auto& info = reg_info(state, src);
  if (!info.cachedRemat) {
    info.cachedRemat = [&] () -> Vinstr {
      jit::vector<std::pair<Vlabel, Vreg>> visited;
      auto const inst =
        find_defining_inst_for_remat(state, b, instIdx, src, visited);
      // No single defining instruction found
      if (!inst || inst->op == Vinstr::nop) return reload{src, dst};

      // Not safe to rematerialize non-pure instructions
      if (!isPure(*inst)) return reload{src, dst};

      // Can't rematerialize instructions which define more than one Vreg,
      // or define flags or physical registers. We can't use cached
      // operands here because we need to take into account ignored
      // registers.
      size_t defCount = 0;
      visitDefs(state.unit, *inst, [&] (Vreg r) { ++defCount; });
      if (defCount != 1) return reload{src, dst};

      return *inst;
    }();
  }
  // At this point, we know cachedRemat is populated.

  // If the cached rematerialization instruction is a reload, we'll
  // just return it, so no further checks are needed. Otherwise we can
  // cache an instruction which may be situationally usable, so we
  // need to do these checks everytime (if the instruction is never
  // useful, we should have a reload stored here).
  if (info.cachedRemat->op != Vinstr::reload) {
    // All sources need to be available. We don't support physical
    // register sources right now because we don't know if they contain
    // the same value at this point as they did originally. This needs
    // to take into account ignored registers, so we can't use cached
    // operands.
    auto unavailableSrc = false;
    auto physicalSrc = false;
    visitUses(
      state.unit, *info.cachedRemat,
      [&] (Vreg r) {
        if (r.isPhys()) physicalSrc = true;
        if (!inReg[r]) unavailableSrc = true;
      }
    );
    if (physicalSrc) {
      // The instruction uses a physical register. This instruction
      // will never be suitable, so turn it into a reload so we'll
      // never check again.
      info.cachedRemat = reload{src, dst};
    } else if (unavailableSrc) {
      // Otherwise one of the sources is unavailable. It could be
      // available at a different program point, so keep the cached
      // instruction, but return a reload.
      return reload{src, dst};
    }
  }

  // Copy the rematerialized instruction, rewrite the output Vreg and
  // insert it.
  auto copy = *info.cachedRemat;
  visitRegsMutable(
    state.unit,
    copy,
    [] (Vreg r) { return r; },
    [&] (Vreg)  { return dst; }
  );
  invalidate_cached_operands(copy);
  if (copy.op != Vinstr::reload) rematerialized = true;
  return copy;
}

/*
 * Spill Weight:
 *
 * The spill weight is what drives the spilling heuristic. When we
 * need to spill a Vreg, the spill weights of all (eligible) Vregs are
 * calculated and sorted. The Vregs with the largest spill weights are
 * chosen. Therefore we want higher spill weights to reflect that a
 * Vreg is more profitable to spill.
 *
 * The spill weight is composed of two values, the usage frequency and
 * the distance. We use profiling information to drive the usage
 * frequency calculation. From the current location, we search forward
 * and find all the next uses of the Vreg. That is, all the uses
 * reachable from the current location which do not already have a use
 * on the path. The usage frequency is the sum of the block weights of
 * each of those uses. Since we will reload the Vreg before each of
 * these uses, it reflects an estimation of how many reloads we'll
 * have to execute at runtime. Since copyish instructions do not
 * trigger a reload, they are not considered a use. Instead their
 * dests are added to the set of Vregs we are looking for.
 *
 * The second component of the spill weight is the distance. This uses
 * the same Vreg uses we found for the usage frequency. It is the
 * minimum of the distances (in terms of instructions) between the
 * current location and those uses. All other things being equal, we
 * want to choose the Vreg which is used farthest away. The distance
 * is only used to break ties when the frequency is the same (which
 * can happen if we don't have profiling information).
 *
 * We calculate the spill weights on demand rather than
 * pre-calculating them, as many Vregs will never need to be spilled.
 */

struct SpillWeight {
  int64_t usage;
  int64_t distance;

  bool operator==(const SpillWeight& o) const {
    return usage == o.usage && distance == o.distance;
  }
  bool operator!=(const SpillWeight& o) const {
    return usage != o.usage || distance != o.distance;
  }
  // Higher usage is less (since a higher weight means more profitable
  // to spill, which means less usage).
  bool operator<(const SpillWeight& o) const {
    if (usage != o.usage) return usage > o.usage;
    return distance < o.distance;
  }
  bool operator>(const SpillWeight& o) const {
    if (usage != o.usage) return usage < o.usage;
    return distance > o.distance;
  }
};

struct SpillerResults;
SpillWeight spill_weight_at(State&,
                            const SpillerResults&,
                            Vreg,
                            Vlabel,
                            size_t);

// The state of which live Vregs are currently spilled or not-spilled. This
// encapsulates that state, as well as the logic to decide which registers
// should be spilled or reloaded.

struct SpillerState {
  // Default state is no Vregs are live
  explicit SpillerState(State& state)
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
  State* state;

  // Given a Vreg, return the per-class state appropriate for that Vreg (or
  // nullptr if untracked). This lets code manipulate the state generically
  // without having to switch on register class constantly.
  const PerClass* forReg(Vreg r) const {
    if (is_ignored(*state, r)) return nullptr;
    switch (reg_class(*state, r)) {
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

  using SpillPenalties = jit::vector<std::pair<Vreg, int64_t>>;

  /*
   * Using the spill weights at the given position, move any Vregs which need to
   * be moved from "inReg" to "inMem" to bring the number of Vregs in "inReg"
   * back below the size of the register class. If we cannot move enough Vregs,
   * then we'll assert (which means a bug). The set of Vregs thus moved is
   * returned. Spill instructions will need to be generated for these Vregs.
   *
   * The "forbidden" set is the set of Vregs which are not eligible
   * for spilling (typically the set of Vregs which are operands to the
   * instruction and thus cannot be spilled).
   *
   * The "penalties" set is a set of manual adjustments which will be
   * made to the calculated spill weights before they are sorted. This
   * can be used to bias decisions based on some special knowledge.
   *
   * This function is typically used after some Vregs are moved into the "inReg"
   * set (perhaps because of reloading). You then call this to spill as
   * necessary (using the Vregs just reloaded as the forbidden set).
   */
  VregSet spill(Vlabel b,
                size_t instIdx,
                const SpillerResults& results,
                const VregSet& forbidden = {},
                const SpillPenalties& penalties = {}) {
    // We'll calculate spill for GP and SIMD separately and then combine the
    // results (since they do not interact).
    auto const impl = [&] (PerClass& per) {
      // If we're already below the size, there's nothing to do (this is the
      // common case).
      if (per.inReg.size() <= per.numRegs) return VregSet{};

      // Otherwise we really have to spill. Gather up the candidates with their
      // spill weights at this point.
      jit::vector<std::pair<Vreg, SpillWeight>> candidates;
      candidates.reserve(per.inReg.size());
      for (auto const r : per.inReg) {
        // We never can spill physical registers or ones which have
        // been forbidden.
        if (r.isPhys() || forbidden[r]) continue;
        // Calculate the spill weights
        auto weight = spill_weight_at(*state, results, r, b, instIdx);
        // Apply any manual adjustments
        auto const it = std::find_if(
          penalties.begin(), penalties.end(),
          [&] (std::pair<Vreg, int64_t> p) { return p.first == r; }
        );
        if (it != penalties.end()) weight.usage += it->second;
        candidates.emplace_back(r, weight);
      }

      // If we have less candidates than we need to spill, we're in trouble.
      auto const toRemove = per.inReg.size() - per.numRegs;
      always_assert(toRemove <= candidates.size());

      // Sort them according to their spill weights. Higher weights (more
      // profitable to spill) come first. Use the Vreg number to break ties.
      std::sort(
        candidates.begin(),
        candidates.end(),
        [](const std::pair<Vreg, SpillWeight>& a,
           const std::pair<Vreg, SpillWeight>& b) {
          if (a.second != b.second) return a.second > b.second;
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
    return impl(gp) | impl(simd);
  }

  // Remove any Vregs (in the candidates set) from being tracked which
  // are not live after the current position, or that belong to the
  // optional kills set.
  void dropDead(const VregSet& candidates,
                const VregSet* kills,
                Vlabel b,
                size_t instIdx) {
    assertx(checkInvariants(Vlabel{}, 0));
    for (auto const r : candidates) {
      auto s = forReg(r);
      if (!s) continue;
      assertx(s->inReg[r] || s->inMem[r]);
      if (live_in_at(*state, r, b, instIdx + 1) &&
          (!kills || !r.isPhys() || !(*kills)[r])) {
        continue;
      }
      s->inReg.remove(r);
      s->inMem.remove(r);
    }
  }

  // Go back to initial state
  void reset() {
    gp.inReg.reset();
    gp.inMem.reset();
    simd.inReg.reset();
    simd.inMem.reset();
  }

  // Whether nothing is currently spilled
  bool allRegs() const {
    return
      gp.inMem.none() &&
      simd.inMem.none();
  }

  // Whether everything live is a physical register
  bool allPhys() const {
    return allRegs() && gp.inReg.allPhys() && simd.inReg.allPhys();
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
      for (auto const r : (per.inReg | per.inMem)) {
        // A Vreg should only be tracked if its actually live at this
        // position, and a Vreg should be in the per-class state appropriate
        // for it.
        always_assert(!is_ignored(*state, r));
        always_assert(forReg(r) == &per);
        always_assert(!b.isValid() || live_in_at(*state, r, b, instIdx));
      }
      // A physical register can never be in memory
      for (auto const r : per.inMem) always_assert(!r.isPhys());
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
  // Did we rematerialize anything?
  bool rematerialized = false;

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

// Recursive implementation for spill_weight_at (see below).
SpillWeight spill_weight_at_impl(State& state,
                                 const SpillerResults& spillerResults,
                                 boost::dynamic_bitset<>& processed,
                                 VregSet& regs,
                                 Vlabel startBlock,
                                 size_t startRpo,
                                 Vlabel b,
                                 size_t instIdx) {
  // The total block weights of all the usages seen so far.
  int64_t totalUsage = 0;
  // The instruction count of the first usage.
  int64_t firstDistance = -1;
  // The number of instructions seen so far on this linear chain of
  // blocks (not counting copyish instructions).
  int64_t instructionCount = 0;

  auto& unit = state.unit;

  while (true) {
    // We should never get here with a block that is before the
    // start block (such blocks have already been processed by the
    // spiller, thus they don't matter to future decisions).
    assertx(state.rpoOrder[b] >= startRpo);

    if (processed[b]) break;

    // Remove any interesting Vregs which are no longer live. If
    // there's none left, we're done and can report our
    // results. Don't do this for the start block because the
    // Vreg(s) may not be live in to that block (they may be defined
    // in it).
    if (b != startBlock) regs &= state.liveIn[b];
    if (regs.none()) break;

    processed[b] = true;

    auto const recordUse = [&] (Vreg r) {
      totalUsage += block_weight(state, b);
      if (firstDistance < 0) firstDistance = instructionCount;
      regs.remove(r);
    };

    // Examine each instruction in the current block, looking for
    // uses. For copyish instructions, add the destination Vreg to
    // the set of interesting Vregs. However, if the destination is
    // a physical register, treat that as a use. Once used, remove
    // it from the interesting set.
    auto& block = unit.blocks[b];
    for (size_t i = instIdx; i < block.code.size(); ++i) {
      auto& inst = block.code[i];

      if (inst.op == Vinstr::copy) {
        if (!regs[inst.copy_.s]) continue;
        if (inst.copy_.d.isPhys()) {
          recordUse(inst.copy_.s);
        } else {
          regs.add(inst.copy_.d);
        }
      } else if (inst.op == Vinstr::copyargs) {
        auto const& srcs = unit.tuples[inst.copyargs_.s];
        auto const& dsts = unit.tuples[inst.copyargs_.d];
        assertx(srcs.size() == dsts.size());
        for (size_t j = 0; j < srcs.size(); ++j) {
          if (!regs[srcs[j]]) continue;
          if (dsts[j].isPhys()) {
            recordUse(srcs[j]);
          } else {
            regs.add(dsts[j]);
          }
        }
      } else if (inst.op == Vinstr::phijmp) {
        auto const successors = succs(block);
        assertx(successors.size() == 1);
        auto const succ = successors[0];
        auto const& phidef = unit.blocks[succ].code.front();
        assertx(phidef.op == Vinstr::phidef);
        auto const& srcs = unit.tuples[inst.phijmp_.uses];
        auto const& dsts = unit.tuples[phidef.phidef_.defs];
        assertx(srcs.size() == dsts.size());

        if (state.rpoOrder[succ] < startRpo) {
          // Special case: if this jump is a back edge to a block
          // which the spiller has already processed. We don't want
          // to examine this block like the others, but we can use
          // the already calculated spiller state. If the spiller
          // has determined that the phi def is non-spilled, we
          // treat that as a use. If the Vreg is spilled, we'll have
          // to insert a reload on the back edge.
          auto const& spillerState = spillerResults.perBlock[succ].inPhi;
          assertx(spillerState.hasValue());
          assertx(spillerState->size() == srcs.size());

          for (size_t j = 0; j < srcs.size(); ++j) {
            if (!regs[srcs[j]]) continue;
            if ((*spillerState)[j] || dsts[j].isPhys()) recordUse(srcs[j]);
          }
        } else {
          // Otherwise treat it like a copyargs
          for (size_t j = 0; j < srcs.size(); ++j) {
            if (!regs[srcs[j]]) continue;
            if (dsts[j].isPhys()) {
              recordUse(srcs[j]);
            } else {
              regs.add(dsts[j]);
            }
          }
        }
      } else {
        // A normal instruction. Shouldn't be a spill or reload
        // because we should only examine blocks that the spiller
        // hasn't visited yet.
        assertx(inst.op != Vinstr::spill && inst.op != Vinstr::reload);
        // Remove any Vregs which have been used.
        auto const uses = uses_set_cached(state, inst) & regs;
        if (uses.any()) {
          totalUsage += block_weight(state, b) * uses.size();
          if (firstDistance < 0) firstDistance = instructionCount;
          regs -= uses;
        }
        ++instructionCount;
      }
    }

    // We didn't find any actual uses in this block. We need to now
    // process the successors.
    auto const successors = succs(block);
    if (successors.empty()) break;

    // If there's more than one successor we can't do this
    // iteratively. Instead recurse and combine the results among
    // the successors.
    if (successors.size() > 1) {
      int64_t usage = 0;
      int64_t distance = 0;

      for (auto const succ : successors) {
        // Since critical edges are split, we cannot have any
        // back-edges here.
        assertx(state.rpoOrder[succ] >= state.rpoOrder[b]);
        auto const result = spill_weight_at_impl(
          state,
          spillerResults,
          processed,
          regs,
          startBlock,
          startRpo,
          succ,
          0
        );
        if (result.usage <= 0) continue;
        if (usage <= 0) {
          distance = result.distance;
        } else {
          distance = std::min(distance, result.distance);
        }
        usage += result.usage;
      }
      if (usage > 0) {
        totalUsage += usage;
        if (firstDistance < 0) firstDistance = distance + instructionCount;
      }
      break;
    }

    // Otherwise just advance to the next block (and start from the
    // beginning of that block).

    auto const succ = successors[0];
    if (state.rpoOrder[succ] < startRpo) {
      // Special case: if the successor is a jump backwards to a
      // block which the spiller has already processed, we can
      // examine the spiller state to see what it has already
      // decided there. If it has decided the Vreg is not-spilled,
      // that counts as a use (we'll have to reload it on the
      // back-edge). This can only happen if there's a single
      // successor because we've split critical edges.
      auto const& spillerState = spillerResults.perBlock[succ].in;
      assertx(spillerState.hasValue());

      for (auto const r : regs) {
        auto const s = spillerState->forReg(r);
        assertx(s);
        if (s->inReg[r]) {
          totalUsage += block_weight(state, b);
          if (firstDistance < 0) firstDistance = instructionCount;
        }
      }

      // We don't actually examine the successor in this case so
      // we're done.
      break;
    }

    // Otherwise proceed normally
    b = succ;
    instIdx = 0;
  }

  if (firstDistance < 0) {
    // No usages at all
    assertx(totalUsage == 0);
    return SpillWeight{0, 0};
  }
  return SpillWeight{totalUsage, firstDistance};
}

// Calculate the spill weight for the given Vreg at the given block
// and index into that block.
SpillWeight spill_weight_at(State& state,
                            const SpillerResults& spillerResults,
                            Vreg reg,
                            Vlabel startBlock,
                            size_t startInstIdx) {
  assertx(startBlock < state.unit.blocks.size());
  assertx(startInstIdx <= state.unit.blocks[startBlock].code.size());

  assertx(!reg.isPhys());
  assertx(reg_info(state, reg).regClass != RegClass::SF);

  /*
   * From the specified starting block, we walk the CFG. We keep a set
   * of "interesting" Vregs (initially just the provided Vreg). For
   * each instruction in the block we check if any of the interesting
   * Vregs are used. If they are, we increment the usage counter by
   * the block's weight (and set the distance if this is the first
   * use). We then remove the Vreg from the interesting set. If the
   * interesting set is ever empty, we are done and return the usage
   * counter and distance as the block weight.
   *
   * If the block has a single successor, we simply loop and then
   * repeat on that successor. Thus runs of blocks with single
   * successors are treated like one super block. If the block has
   * multiple successors, we recursively calculate the spill weight
   * for the successors and then combine the results (along with the
   * already gathered results).
   *
   * Copyish instructions have some special treatment. Since they
   * generally preserve spill-ness, they are not usually considered as
   * uses. Instead their dest Vregs are added to the interesting set
   * (and the uses are untouched). This is how the interesting set can
   * grow beyond its initial single Vreg. The exception is when the
   * copy writes to a physical register. That is treated like a normal
   * use (since we'll be forced to emit a reload for that case).
   *
   * We only examine blocks which the spiller hasn't processed
   * yet. This is because the choice of whether a Vreg is spilled or
   * not has already been fixed for those blocks, so it shouldn't
   * change the heuristic. Instead we check the spiller state to see
   * what has been decided for that Vreg. If its been decided to not
   * be spilled in that block, we'll treat that as a use (because
   * we'll have to eventually emit a reload on the back edge to it).
   * Since the spiller workers in RPO order, we can determine whether
   * a block has been processed by just looking at the RPO index of
   * the blocks (is it before our first block?).
   *
   * Each block is only processed once (we keep a set of processed
   * blocks). This isn't *quite* correct for two reasons. One, the
   * distance calculation is path sensitive and thus you might get a
   * different result depending how you got there (and thus processing
   * a block multiple times will give different results). Second, the
   * interesting Vreg set might be different depending on the path you
   * took to the block (because of phis). However this doesn't seem to
   * matter in practice and simplifies things a lot.
   */

  auto const startRpo = state.rpoOrder[startBlock];

  // The current set of "interesting" Vregs. Since copyish
  // instructions do not trigger a reload (unless one side is a
  // physical register), we may be looking for multiple Vregs at
  // once. Once a Vreg has a use, we'll remove it from the set.
  VregSet regs{reg};

  // Avoid infinite loops
  boost::dynamic_bitset<> processed(state.unit.blocks.size());
  return spill_weight_at_impl(
    state,
    spillerResults,
    processed,
    regs,
    startBlock,
    startRpo,
    startBlock,
    startInstIdx
  );
}

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
    assertx(!r.isPhys() || s->inReg[r]);
    in[i] = s->inReg[r];
  }

  // Some of the phi outputs might be immediately dead, so drop them now.
  spillerState.dropDead(
    spillerState.gp.inReg | spillerState.gp.inMem |
    spillerState.simd.inReg | spillerState.simd.inMem,
    nullptr,
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
size_t setup_initial_spiller_state_loop(State& state,
                                        Vlabel b,
                                        SpillerResults& results) {
  assertx(is_loop_header(state, b));
  auto& unit = state.unit;

  SpillerState initial{state};

  // Start with all Vregs live-in to the block. Also include any Vregs defined
  // by a phidef.
  auto alive = state.liveIn[b];
  const VregList* defs = nullptr;
  if (unit.blocks[b].code.front().op == Vinstr::phidef) {
    auto const& phidef = unit.blocks[b].code.front().phidef_;
    defs = &unit.tuples[phidef.defs];

    assertx(b != unit.entry);
    assertx(
      !defs_set_cached(state, unit.blocks[b].code.front()).containsPhys()
    );
    assertx(uses_set_cached(state, unit.blocks[b].code.front()).none());

    for (size_t i = 0; i < defs->size(); ++i) {
      auto const r = (*defs)[i];
      if (!initial.forReg(r)) continue;
      assertx(!r.isPhys());
      alive.add(r);
    }
  }

  auto const& loopInfo = loop_info(state, b);

  // Split the Vregs into those which are used inside the loop, and those which
  // are just live-through (but unused).
  auto const usedWithin = loopInfo.uses & alive;
  auto const liveThrough = alive - usedWithin;

  // Force the physical registers to be non-spilled.
  for (auto const r : alive) {
    auto const s = initial.forReg(r);
    if (!s) continue;
    if (!r.isPhys()) continue;
    s->inReg.add(r);
  }

  // Decide which Vregs should be assumed to be non-spilled on the block entry,
  // and which ones should be assumed to be spilled on block entry. Use the max
  // register pressure within the loop as a guide.
  auto const process = [&] (SpillerState::PerClass& s, size_t pressure) {
    // Assume that all Vregs used within the loop are non-spilled.
    for (auto const r : usedWithin) {
      if (initial.forReg(r) != &s) continue;
      if (r.isPhys()) continue;
      s.inReg.add(r);
    }

    if (s.inReg.size() < s.numRegs) {
      // There's still some space left after taking into account the Vregs used
      // within the loop. We might be able to allow some live-through Vregs to
      // be in registers.
      size_t throughCount = 0;
      for (auto const r : liveThrough) {
        if (initial.forReg(r) != &s) continue;
        if (r.isPhys()) continue;
        ++throughCount;
      }

      throughCount += s.numRegs;
      if (throughCount > pressure) {
        // There's still some space left. Pick the live-through Vregs
        // which have the lowest spill weights (less profitable to
        // spill) to also be non-spilled.
        throughCount -= pressure;

        jit::vector<std::pair<Vreg, SpillWeight>> candidates;
        candidates.reserve(liveThrough.size());
        for (auto const r : liveThrough) {
          if (initial.forReg(r) != &s) continue;
          if (r.isPhys()) continue;
          candidates.emplace_back(r, spill_weight_at(state, results, r, b, 0));
        }

        std::sort(
          candidates.begin(),
          candidates.end(),
          [](const std::pair<Vreg, SpillWeight>& a,
             const std::pair<Vreg, SpillWeight>& b) {
            if (a.second != b.second) return a.second < b.second;
            return a.first < b.first;
          }
        );

        // The first N are non-spilled and the rest are spilled.
        auto const numReg = std::min(throughCount, candidates.size());
        for (size_t i = 0; i < numReg; ++i) {
          s.inReg.add(candidates[i].first);
        }
        for (size_t i = numReg; i < candidates.size(); ++i) {
          assertx(!candidates[i].first.isPhys());
          s.inMem.add(candidates[i].first);
        }

        assertx(s.inReg.size() <= s.numRegs);
        return;
      }
    }

    // We used up all the space for Vregs used within the loop. The live-through
    // Vregs have to be spilled.
    for (auto const r : liveThrough) {
      if (initial.forReg(r) != &s) continue;
      if (r.isPhys()) continue;
      s.inMem.add(r);
    }
  };
  process(initial.gp, loopInfo.gpPressure);
  process(initial.simd, loopInfo.simdPressure);

  // If we have more used-within Vregs than physical registers, we still might
  // need to move some to the spilled category. Do so here.
  initial.spill(b, 0, results);
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
size_t setup_initial_spiller_state(State& state,
                                   Vlabel b,
                                   SpillerResults& results) {
  // Loop headers are dealt with specially.
  if (is_loop_header(state, b)) {
    return setup_initial_spiller_state_loop(state, b, results);
  }

  auto& unit = state.unit;
  SpillerState initial{state};

  // First iterate over all (already processed) predecessors. For each
  // Vreg which is live-in to this block, we'll examine its state in
  // the predecessors, and sum the total block weights of the
  // predecessors where its spilled and where its not spilled. We'll
  // initially consider the Vreg to be whatever has the higher total
  // weight. We'll record the weight of the non-spilled preds so we
  // can bias the spilling decision below.
  SpillerState::SpillPenalties spillPenalties;
  if (b == state.unit.entry) {
    for (auto const r : state.liveIn[b]) {
      // Only physical registers should be live-in for the entry
      // block. They'll always be non-spilled.
      assertx(r.isPhys() && !is_ignored(state, r));
      auto const s = initial.forReg(r);
      if (!s) continue;
      s->inReg.add(r);
    }
  } else {
    for (auto const r : state.liveIn[b]) {
      auto initialState = initial.forReg(r);
      if (!initialState) continue;

      int64_t inRegWeight = 0;
      int64_t inMemWeight = 0;
      for (auto const pred : state.preds[b]) {
        auto const& out = results.perBlock[pred].out;
        if (!out) continue;

        // There shouldn't be anything live which isn't tracked.
        assertx(
          (state.liveIn[b] -
           (out->gp.inReg | out->simd.inReg | out->gp.inMem | out->simd.inMem)
          ).none()
        );

        // Sum up the block weights for each case
        auto const s = out->forReg(r);
        assertx(s);

        if (s->inReg[r]) {
          inRegWeight += block_weight(state, pred);
        } else {
          assertx(!r.isPhys());
          assertx(s->inMem[r]);
          inMemWeight += block_weight(state, pred);
        }
      }

      // Pick the majority case
      if (inRegWeight > inMemWeight) {
        initialState->inReg.add(r);
        // We might have to spill this anyways below, so record the
        // penalty of doing so (the weight of all the predecessors
        // where its not spilled).
        if (!r.isPhys()) spillPenalties.emplace_back(r, inRegWeight);
      } else {
        assertx(!r.isPhys());
        initialState->inMem.add(r);
      }
    }
  }

  // Now that we've processed Vregs which were live-in to the block, we need to
  // consider phidef outputs. We can use similar logic to the live-in Vregs,
  // except examining the phi state of the predecessors.
  const VregList* defs = nullptr;
  if (unit.blocks[b].code.front().op == Vinstr::phidef) {
    auto const& phidef = unit.blocks[b].code.front().phidef_;
    defs = &unit.tuples[phidef.defs];

    assertx(b != unit.entry);
    assertx(
      !defs_set_cached(state, unit.blocks[b].code.front()).containsPhys()
    );
    assertx(uses_set_cached(state, unit.blocks[b].code.front()).none());

    for (size_t i = 0; i < defs->size(); ++i) {
      auto const def = (*defs)[i];

      auto initialState = initial.forReg(def);
      if (!initialState) continue;

      int64_t inRegWeight = 0;
      int64_t inMemWeight = 0;
      for (auto const pred : state.preds[b]) {
        auto const& out = results.perBlock[pred].outPhi;
        if (!out) continue;

        assertx(out->size() == defs->size());
        if ((*out)[i]) {
          inRegWeight += block_weight(state, pred);
        } else {
          assertx(!def.isPhys());
          inMemWeight += block_weight(state, pred);
        }
      }

      if (inRegWeight > inMemWeight) {
        initialState->inReg.add(def);
        if (!def.isPhys()) spillPenalties.emplace_back(def, inRegWeight);
      } else {
        assertx(!def.isPhys());
        initialState->inMem.add(def);
      }
    }
  }

  // We might have more non-spilled Vregs than we can fit into
  // registers. Spill as necessary. Use the recorded weights above to
  // bias the decision appropriately.
  initial.spill(b, 0, results, VregSet{}, spillPenalties);
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

// Run spiller logic for a phijmp instruction. Phis can handle spilled Vregs, so
// we don't need to reload the phi's inputs.
void process_phijmp_spills(State& state,
                           Vlabel b,
                           size_t instIdx,
                           Vinstr& phijmp,
                           SpillerState& spiller,
                           SpillerResults& results) {
  assertx(phijmp.op == Vinstr::phijmp);
  auto const& unit = state.unit;

  // This should end a block
  assertx(instIdx == unit.blocks[b].code.size() - 1);
  assertx(defs_set_cached(state, phijmp).none());
  assertx(acrosses_set_cached(state, phijmp).none());

  // Examine which Vregs are currently non-spilled and record them in the phi
  // state.
  auto const& uses = unit.tuples[phijmp.phijmp_.uses];
  assertx(!results.perBlock[b].outPhi);
  auto& outPhi = results.perBlock[b].outPhi.emplace();
  outPhi.resize(uses.size());
  for (size_t i = 0; i < uses.size(); ++i) {
    auto const r = uses[i];
    auto const s = spiller.forReg(r);
    if (!s) continue;
    assertx(s->inReg[r] || s->inMem[r]);
    assertx(!r.isPhys() || s->inReg[r]);
    outPhi[i] = s->inReg[r];
  }

  // Remove any Vregs which aren't used after the phi so they won't be part of
  // the block's live-out.
  spiller.dropDead(uses_set_cached(state, phijmp), nullptr, b, instIdx);
}

// Run spiller logic for a copy-ish instruction with no physical
// dests. Copies need to be handle specially because unlike normal
// instructions, they can handle both spilled and non-spilled Vregs
// (therefore we don't need to reload the inputs before the
// instruction). We only need to ensure that a copy doesn't attempt to
// move between a spilled or non-spilled Vreg (or vice-versa). Indeed,
// we need to ensure that a copy never causes a reload, as we need to
// be able to copy spilled Vregs around without introducing additional
// register presure. Return the number of instructions inserted.
size_t process_copy_spills(State& state,
                           VregList& uses,
                           const VregList& defs,
                           Vlabel b,
                           size_t instIdx,
                           SpillerState& spiller,
                           SpillerResults& results) {
  assertx(uses.size() == defs.size());

  auto& unit = state.unit;

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
    assertx(!u.isPhys() || s->inReg[u]);
    assertx(!d.isPhys());
    assertx(!inReg[d] && !inMem[d]);
    if (s->inReg[u]) {
      inReg.add(d);
    } else {
      inMem.add(d);
    }
  }

  // Remove any Vregs which aren't used after the copy from being tracked.
  VregSet defsSet{defs};
  spiller.dropDead(VregSet{uses}, &defsSet, b, instIdx);

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
    assertx(!d.isPhys());
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
  auto const spills = spiller.spill(b, instIdx, results);
  if (spills.none()) {
    // If no spilling is required, then remove any immediately dead defs and
    // then we're done.
    spiller.dropDead(defsSet, nullptr, b, instIdx);
    return 0;
  }

  // We have to spill. Make sure we can.
  always_assert(state.abi.canSpill);

  if (debug) {
    for (auto const r : spills) always_assert(spiller.forReg(r));
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
      assertx(!u.isPhys());

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
         *   spill src -> src2
         *   copy src -> dst
         *   conjureuse dst
         */
        aliasPairs.emplace_back(u, d);
        uses[i] = d;
        results.ssaize.add(d);
        invalidate_cached_operands(unit.blocks[b].code[instIdx]);
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
      assertx(!d.isPhys());
      spillPairs.emplace_back(u, d);
      uses[i] = d;
      selfSpills.remove(d);
      results.ssaize.add(d);
      invalidate_cached_operands(unit.blocks[b].code[instIdx]);
    }
  }

  // Anything self spilled needs to be SSA restored because we use the same Vreg
  // as the source and dest.
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
        state.spilled = true;
      }
      for (auto const r : selfSpills) {
        v << spill{r, r};
        ++added;
        state.spilled = true;
      }
      return 0;
    }
  );

  // Remove any Vregs which the copy defined and are immediately dead.
  spiller.dropDead(defsSet, nullptr, b, instIdx + added);
  return added;
}

// Run spiller logic for normal instructions (not phis or copies). Return the
// number of instructions inserted.
size_t process_inst_spills(State& state,
                           Vlabel b,
                           size_t instIdx,
                           Vinstr& inst,
                           SpillerState& spiller,
                           SpillerResults& results) {
  // We allow Vinstr::copy if the src/dest is a physical register. In that case,
  // we treat it like a normal instruction (not giving it the special copy
  // logic).
  assertx(inst.op != Vinstr::phijmp && inst.op != Vinstr::phidef);
  assertx(
    inst.op != Vinstr::copyargs || defs_set_cached(state, inst).allPhys()
  );

  auto& unit = state.unit;

  // NB: acrosses is a subset of uses at this point
  auto uses = uses_set_cached(state, inst);
  auto acrosses = acrosses_set_cached(state, inst);
  auto const& defs = defs_set_cached(state, inst);

  VregSet reloads; // Vregs we'll have to reload
  VregSet spills;  // Vregs we'll have to spill

  // First process the uses (including acrosses). If any use is currently
  // spilled, make it non-spilled and record that it needs a reload.
  for (auto const r : uses) {
    auto s = spiller.forReg(r);
    if (!s) continue;
    assertx(s->inReg[r] || s->inMem[r]);
    if (s->inMem[r]) {
      assertx(!r.isPhys());
      reloads.add(r);
      s->inMem.remove(r);
    }
    s->inReg.add(r);
  }
  // Moving the uses into registers may require us to spill other Vregs (except
  // the ones we just reloaded).
  spills |= spiller.spill(b, instIdx, results, uses);

  /*
   * Consider the instruction "Op R1 -> R2" where R1 and R2 are
   * virtual, and R1 is not used after Op. This instruction clearly
   * only requires one physical register because R1 is dead after Op,
   * and therefore R2 can always use the same register that R1
   * had. However, suppose that R1 is %eax, and R2 is %ecx. The
   * instruction actually requires two physical registers because we
   * no longer have the freedom to assign R1 and R2 the same physical
   * register. Sebastian Hack calls such instructions "register
   * pressure unfaithful".
   *
   * This is problematic for the spiller because it only considers the
   * number of live Vregs when determining whether spilling is
   * needed. It only sees the first situation. Fix this conservatively
   * by forcing any physical register uses which don't have a matching
   * physical register def to be across.
   */
  if (defs.containsPhys() && uses.containsPhys()) {
    acrosses.add(uses.physRegs() - defs.physRegs());
  }
  uses -= acrosses; // Make uses and acrosses disjoint

  // If any of the uses aren't used after this, drop them from being tracked to
  // make room for the defs. Since we removed the acrosses from the uses, we'll
  // keep those alive.
  spiller.dropDead(uses, &defs, b, instIdx);

  // Process the defs. The defs always start in registers because the
  // instruction has to write them there.
  for (auto const r : defs) {
    auto s = spiller.forReg(r);
    if (!s) continue;
    // We shouldn't already be tracking this
    assertx(!s->inReg[r] && !s->inMem[r]);
    s->inReg.add(r);
  }
  // Adding the defs may have caused us to need to spill even more (but not any
  // uses or defs).
  spills |= spiller.spill(b, instIdx, results, defs | acrosses | uses);

  // Some of the defs or acrosses may not be live (if the defs are never used),
  // so remove them.
  spiller.dropDead(defs | acrosses, nullptr, b, instIdx);

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
  if (spills.none() && reloads.none()) return 0;
  // We should either spill or reload a Vreg, not both.
  assertx((spills & reloads).none());

  // We always spill/reload a Vreg to itself, so they'll need to be
  // re-SSAized.
  results.ssaize |= spills;
  results.ssaize |= reloads;

  size_t added = 0;
  vmodify(
    unit, b, instIdx,
    [&] (Vout& v) {
      // First emit the spills (which decreases register pressure).
      for (auto const r : spills) {
        assertx(!r.isPhys());
        v << spill{r, r};
        ++added;
        state.spilled = true;
      }

      // Now emit the reloads (which increases register pressure),
      // possibly with rematerialization. Keep track of the Vregs
      // which are currently in registers so we know which ones are
      // available as rematerialization sources.
      auto inReg = (spiller.gp.inReg | spiller.simd.inReg) - reloads;
      for (auto const r : reloads) {
        assertx(!r.isPhys());
        v << reload_with_remat(
          state,
          results.rematerialized,
          b,
          instIdx,
          inReg,
          r,
          r
        );
        ++added;
        inReg.add(r);
      }

      return 0;
    }
  );

  return added;
}

// Run the spiller logic for an entire block which we've
// pre-determined will not contain a spill. There must not be any
// spilled registers in the spiller state.
void process_spills_skip(State& state,
                         Vlabel b,
                         SpillerState& spiller,
                         SpillerResults& results) {
  assertx(spiller.allRegs());

  // There's no spilled registers alive and we know no instruction
  // within the block can cause a spill. The state after processing
  // this block should just be all the live-out Vregs in registers.
  auto const& unit = state.unit;
  auto const& block = unit.blocks[b];
  auto const& lastInst = block.code.back();
  if (lastInst.op == Vinstr::phijmp) {
    // Remember to record phi spill state (no spills).
    auto const& uses = unit.tuples[lastInst.phijmp_.uses];
    assertx(!results.perBlock[b].outPhi);
    auto& outPhi = results.perBlock[b].outPhi.emplace();
    outPhi.resize(uses.size());
    for (size_t i = 0; i < uses.size(); ++i) {
      auto const r = uses[i];
      if (spiller.forReg(r)) outPhi[i] = true;
    }
  }

  spiller.reset();
  for (auto const r : state.liveOut[b]) {
    if (auto s = spiller.forReg(r)) s->inReg.add(r);
  }
}

// Run the spiller logic over the entire unit. Returning the spiller
// state at block boundaries (and Vregs which need SSA
// conversion). Each block is allowed to determine which of its in/out
// Vregs should be spilled or not, independently of all others. We'll
// later pass over all the state and insert spills/reloads as needed
// to make each block compatible. The bitset determines which blocks
// actually need spilling. If register pressure calculation has
// determined a block cannot cause a spill, we can process it more
// efficiently.
SpillerResults process_spills(State& state,
                              const boost::dynamic_bitset<>& needsSpilling) {
  auto& unit = state.unit;

  SpillerResults results{state};
  SCOPE_ASSERT_DETAIL("Spiller State") { return results.toString(state); };

  for (auto const b : state.rpo) {
    // Initialize the state for this block and get the initial (in) state.
    auto instIdx = setup_initial_spiller_state(state, b, results);
    auto spiller = *results.perBlock[b].in;

    if (!needsSpilling[b] && spiller.allRegs()) {
      // Be efficient if there's no potential spills to worry about.
      process_spills_skip(state, b, spiller, results);
    } else {
      // Iterate over each instruction and run the logic for each one. We need
      // to use indices because we'll modify the unit as part of processing it
      // (which means we need to shift the indices).
      for (; instIdx < unit.blocks[b].code.size(); ++instIdx) {
        assertx(spiller.checkInvariants(b, instIdx));

        auto& inst = unit.blocks[b].code[instIdx];
        switch (inst.op) {
          case Vinstr::phijmp:
            process_phijmp_spills(state, b, instIdx, inst, spiller, results);
            break;
          case Vinstr::copyargs:
            assertx(acrosses_set_cached(state, inst).none());
            if (defs_set_cached(state, inst).allPhys()) {
              instIdx += process_inst_spills(
                state,
                b,
                instIdx,
                inst,
                spiller,
                results
              );
            } else {
              assertx(!defs_set_cached(state, inst).containsPhys());
              instIdx += process_copy_spills(
                state,
                unit.tuples[inst.copyargs_.s],
                unit.tuples[inst.copyargs_.d],
                b,
                instIdx,
                spiller,
                results
              );
            }
            break;
          case Vinstr::copy: {
            assertx(acrosses_set_cached(state, inst).none());

            // Copies which have physical sources or dests are treated like
            // normal instructions.
            if (is_ignored(state, inst.copy_.s) || inst.copy_.d.isPhys()) {
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

            // process_copy_spills takes its uses and defs as a VregList, so
            // use a temporary one to satisfy the interface.
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
  assertx(pred->checkInvariants(b, unit.blocks[b].code.size()));

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
  auto& phijmp = unit.blocks[b].code.back();
  auto& uses = unit.tuples[phijmp.phijmp_.uses];

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
      invalidate_cached_operands(phijmp);
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
      invalidate_cached_operands(phijmp);
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
      invalidate_cached_operands(phijmp);
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
      invalidate_cached_operands(phijmp);
    }
  }

  mismatch.spills |= newSpills;
  mismatch.reloads |= newReloads;
  assertx((mismatch.spills & mismatch.reloads).none());

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

      size_t reloadCount = 0;

      // Materialize the actual instructions. The order of these is important.
      vmodify(
        unit, b, unit.blocks[b].code.size() - 1,
        [&] (Vout& v) {
          // First the ssaaliases, which must come before anything else.
          for (auto const& p : mismatch.aliases) {
            v << ssaalias{p.first, p.second};
          }

          // Then the spills, which reduce register pressure.
          for (auto const r : mismatch.spills) {
            assertx(!r.isPhys());
            auto const it = mismatch.spillDests.find(r);
            auto const r2 =
              (it == mismatch.spillDests.end()) ? r : it->second;
            v << spill{r, r2};
            state.spilled = true;
          }

          // Then the reloads, which increases register pressure.
          for (auto const r : mismatch.reloads) {
            assertx(!r.isPhys());
            auto const it = mismatch.reloadDests.find(r);
            auto const r2 =
              (it == mismatch.reloadDests.end()) ? r : it->second;
            // This may be replaced by rematerialized instruction below
            v << reload{r, r2};
            ++reloadCount;
          }

          return 0;
        }
      );

      // We have to defer rematerialization until after we emitted all
      // the reloads. Otherwise the rematerialization logic could
      // become confused because it will try to walk backwards through
      // a block which we're still emitting instructions into.
      if (reloadCount > 0) {
        auto const& pred = results.perBlock[b].out;
        // We can only rematerialize instructions which have available
        // sources. These are the Vregs which are currently in
        // registers and haven't been reloaded (if they were reloaded
        // they may not be in registers when we need them).
        auto inReg = (pred->gp.inReg | pred->simd.inReg) - mismatch.reloads;

        auto instIdx = unit.blocks[b].code.size() - reloadCount - 1;
        for (auto const r : mismatch.reloads) {
          assertx(instIdx < unit.blocks[b].code.size());
          assertx(!r.isPhys());
          auto const it = mismatch.reloadDests.find(r);
          auto const r2 = (it == mismatch.reloadDests.end()) ? r : it->second;

          // Try to replace the instruction with a rematerialized one.
          auto& inst = unit.blocks[b].code[instIdx];
          auto const irctx = inst.irctx();
          assertx(inst.op == Vinstr::reload &&
                  inst.reload_.s == r &&
                  inst.reload_.d == r2);
          inst = reload_with_remat(
            state,
            results.rematerialized,
            b,
            instIdx,
            inReg,
            r,
            r2
          );
          inst.set_irctx(irctx);
          // The def of the instruction is now available for more
          // rematerialized instructions.
          inReg.add(r2);
          ++instIdx;
        }
      }
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
  auto& unit = state.unit;

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
          if (is_ignored(state, s)) break;
          auto const cls = reg_class(state, s);
          if (is_spill(cls)) spillize(d);
          break;
        }
        case Vinstr::copyargs: {
          // Propagate "spillness" across copy
          auto const& uses = unit.tuples[inst.copyargs_.s];
          auto const& defs = unit.tuples[inst.copyargs_.d];
          assertx(uses.size() == defs.size());
          for (size_t i = 0; i < uses.size(); ++i) {
            assertx(is_ignored(state, uses[i]) == is_ignored(state, defs[i]));
            if (is_ignored(state, uses[i])) continue;
            auto const cls = reg_class(state, uses[i]);
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
    for (auto& inst : unit.blocks[b].code) {
      switch (inst.op) {
        case Vinstr::spill: {
          // The dest of a spill should be a spill slot, and the source should
          // not be.
          always_assert(!inst.reload_.d.isPhys());
          auto const dCls = reg_class(state, inst.spill_.d);
          auto const sCls = reg_class(state, inst.spill_.s);
          always_assert(is_spill(dCls));
          always_assert(!is_spill(sCls));
          always_assert(appropriate(dCls, sCls));
          break;
        }
        case Vinstr::reload: {
          // The source of a reload should be a spill slot, and the dest should
          // not be.
          always_assert(!inst.reload_.s.isPhys());
          auto const dCls = reg_class(state, inst.reload_.d);
          auto const sCls = reg_class(state, inst.reload_.s);
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
              always_assert(
                is_ignored(state, defs[i]) == is_ignored(state, uses[i])
              );
              if (is_ignored(state, defs[i])) continue;
              auto const dCls = reg_class(state, defs[i]);
              auto const uCls = reg_class(state, uses[i]);
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
          if (is_ignored(state, s)) {
            always_assert(
              is_ignored(state, d) || !is_spill(reg_class(state, d))
            );
            break;
          }
          if (is_ignored(state, d)) {
            always_assert(!is_spill(reg_class(state, s)));
            break;
          }
          auto const dCls = reg_class(state, d);
          auto const uCls = reg_class(state, s);
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
            always_assert(
              is_ignored(state, uses[i]) == is_ignored(state, defs[i])
            );
            auto const dCls = reg_class(state, defs[i]);
            auto const uCls = reg_class(state, uses[i]);
            always_assert(is_spill(dCls) == is_spill(uCls));
            always_assert(IMPLIES(is_spill(dCls), dCls == uCls));
          }
          break;
        }
        default:
          // For most instructions, both the sources and dests should not be
          // spill slots.
          for (auto const r : uses_set_cached(state, inst)) {
            always_assert(!is_spill(reg_class(state, r)));
          }
          for (auto const r : defs_set_cached(state, inst)) {
            always_assert(!is_spill(reg_class(state, r)));
          }
          break;
      }
    }
  }
}

// Calculate a bitset indicating which blocks may actually generate
// spills. Such blocks can be processed more efficiently. If the
// bitset has no bits set, spilling can be skipped entirely.
boost::dynamic_bitset<> determine_spilling_needed(State& state) {
  auto& unit = state.unit;
  auto const gpLimit = state.gpUnreserved.size();
  auto const simdLimit = state.simdUnreserved.size();

  // We can determine if a block might trigger a spill by calculating
  // the maximum register pressure within the block. If it exceeds the
  // number of physical registers, we might need a spill. This
  // calculation is conservative because we assume all live Vregs will
  // be in registers (in reality, some might already be
  // spilled). Thus, at worst, we overestimate the register pressure,
  // thinking we might have a spill when we won't.

  boost::dynamic_bitset<> needsSpilling;
  needsSpilling.resize(unit.blocks.size());

  for (auto const b : state.rpo) {
    VregSet gpLive;
    VregSet simdLive;

    auto const liveSet = [&] (Vreg r) -> VregSet* {
      switch (reg_class(state, r)) {
        case RegClass::AnyNarrow:
        case RegClass::GP:
          return &gpLive;
        case RegClass::SIMD:
        case RegClass::SIMDWide:
          return &simdLive;
        case RegClass::SF:
          return nullptr;
        case RegClass::Any:
        case RegClass::Spill:
        case RegClass::SpillWide:
          break;
      }
      always_assert(false);
    };

    auto const add = [&] (Vreg r) {
      if (auto l = liveSet(r)) l->add(r);
    };
    auto const remove = [&] (Vreg r) {
      if (auto l = liveSet(r)) l->remove(r);
    };
    auto const update = [&] {
      if (gpLive.size() > gpLimit || simdLive.size() > simdLimit) {
        needsSpilling[b] = true;
      }
    };

    // Start with the already stored liveness information
    for (auto const r : state.liveOut[b]) add(r);
    update();

    // And then walk backwards modifying it on the fly.
    for (auto instIdx = unit.blocks[b].code.size(); instIdx > 0; --instIdx) {
      auto& inst = unit.blocks[b].code[instIdx-1];

      auto const& defs = defs_set_cached(state, inst);
      auto const& uses = uses_set_cached(state, inst);
      auto const& acrosses = acrosses_set_cached(state, inst);

      for (auto const r : defs)     add(r);
      for (auto const r : acrosses) add(r);
      // Deal with register pressure unfaithful instructions like the
      // spiller.
      if (defs.containsPhys() && uses.containsPhys()) {
        for (auto const r : uses) if (r.isPhys()) add(r);
      }
      update();
      for (auto const r : defs) remove(r);
      for (auto const r : uses) add(r);
      update();
    }

    // What we've calculated for liveness should match the already stored
    // per-block liveness information at this point.
    assertx((gpLive | simdLive) == state.liveIn[b]);
  }

  return needsSpilling;
}

void insert_spills(State& state) {
  // Calculate liveness. This is needed to determine if spilling is
  // necessary.
  calculate_liveness(state);

  auto const needsSpilling = determine_spilling_needed(state);
  // If no block can possibly spill, the spiller can be skipped
  // entirely.
  if (needsSpilling.none()) return;

  // Otherwise calculate metadata the spiller will need.
  calculate_loop_info(state);

  // Generate spills and fixup mismatches between blocks.
  auto results = process_spills(state, needsSpilling);
  fixup_spill_mismatches(state, results);

  // Spilling may break SSA form because we use the same Vreg when
  // both spilled and reloaded, so fix that here.
  auto const mappings = [&] {
    auto const mappings = restoreSSA(state.unit, results.ssaize, state.rpo, 0);
    for (auto const& map : mappings) {
      reg_info_insert(
        state,
        map.first,
        reg_info(state, map.second)
      );
    }
    return mappings;
  }();

  // Update the reg-classes of spilled Vregs to mark that they are
  // spills.
  set_spill_reg_classes(state, results, mappings);

  // If we rematerialized anything, we may have created dead spills,
  // so remove them.
  if (results.rematerialized) removeDeadCode(state.unit, 0);

  // Re-calculate liveness since we changed Vregs
  calculate_liveness(state);

  assertx(check(state.unit));
}

//////////////////////////////////////////////////////////////////////
// Coloring

/*
 * As already mentioned, now that the spiller has lowered the register
 * pressure everywhere to below the number of physical registers, the
 * unit is guaranteed to be colorable.
 *
 * For instructions which do not write to physical registers, the
 * coloring is easy. Simply assign the defs the next available
 * physical registers. Once a Vreg becomes dead, the register is
 * released.
 *
 * For instructions which do write to physical registers, it is more
 * complex, however. An instruction could write to a physical register
 * which has already been assigned to a Vreg which is live across the
 * instruction. This means we need to migrate that Vreg to a new
 * (free) physical register (inserting a copy). For this reason, a
 * Vreg can potentially be assigned different physical registers at
 * different points in the unit.
 *
 * Spill slots are assigned to spill in a different manner using
 * global analysis. The "coloring" just looks up the already assigned
 * slot. Spills never change their color once assigned.
 *
 * While technically the colorer has the freedom to choose arbitrary
 * physical registers for Vregs, in practice most colorings are
 * terrible. They leave a lot of copies, and because of the
 * aforementioned physical def constraints, causes a lot of
 * shuffling. We use a heuristic using "penalty vectors" to try to
 * find a good coloring on the fly.
 *
 * Once a physical register has been assigned to a Vreg at a
 * particular instruction, the instruction will be rewritten to
 * operate on that physical register directly. This is easier because,
 * as mentioned above, a Vreg can be in different physical registers
 * at different points, and therefore we avoid having a complicated
 * per-instruction mapping. It is also more efficient to do the
 * rewriting as we color, instead of a separate pass. Spills are
 * rewritten separately.
 */

/*
 * Spill Coloring:
 *
 * We use a global analysis to assign spills to slots. This generates
 * very good results, but is potentially expensive. However it is not
 * an issue in practice because units tend to have a very small number
 * of spills.
 *
 * First we generate spill affinities. Two spills have an affinity
 * with each other if they do not interfere and have a copy between
 * them. The weight of the affinity is the block weight of the copy.
 *
 * We build up chunks from the affinities. Initially each spill is
 * assigned its own chunk. We then walk the affinities (sorted by
 * descending weight), and try to merge together the two spills'
 * chunks. We merge the chunks if all the spills in the merged chunk
 * don't interfere with each other (and have a compatible reg-class).
 *
 * Once we've built up the chunks this way, each chunk corresponds to
 * a single slot. Since every spill in the chunk doesn't interfere with
 * any other, they can be assigned the same slot. Indeed, since
 * they'll (tend to) share affinities, its profitable for them to
 * share the same slot.
 *
 * These assignments are stored away, where the colorer will look them
 * up while walking the unit.
 */

struct SpillAffinity {
  Vreg r1;
  Vreg r2;
  size_t weight;
};

void assign_spill_colors(State& state,
                         const VregSet& spills,
                         const jit::fast_map<Vreg, VregSet>& interferences,
                         jit::vector<SpillAffinity> affinities) {
  assertx(spills.any());

  // Sort affinities by weight. Spills with higher weighted affinities
  // are more profitable to put into the same chunk (where they'll be
  // assigned the same slot).
  std::sort(
    affinities.begin(),
    affinities.end(),
    [&] (const SpillAffinity& a1, const SpillAffinity& a2) {
      if (a1.weight != a2.weight) return a1.weight > a2.weight;
      if (a1.r1 != a2.r1) return a1.r1 < a2.r1;
      return a1.r2 < a2.r2;
    }
  );

  // Set of active chunks.
  jit::vector<VregSet> chunks;
  // Map a chunk to its canonical representation.
  jit::vector<size_t> chunkMappings;
  // Mapping of a spill to the chunk its currently assigned to.
  jit::fast_map<Vreg, size_t> spillToChunk;

  // Assign each spill its own chunk. Every chunk maps to itself.
  for (auto const r : spills) {
    spillToChunk[r] = chunks.size();
    chunks.emplace_back(r);
    chunkMappings.emplace_back(chunkMappings.size());
  }

  // Canonicalize a chunk index to the index of the chunk its been
  // merged with. This can update the mappings as it goes (to shorten
  // lookup chains).
  auto const canonicalize = [&] (size_t chunk) {
    auto const impl = [&] (size_t chunk, auto const& self) {
      auto& chunk2 = chunkMappings[chunk];
      if (chunk == chunk2) return chunk;
      assertx(chunk2 < chunk);
      chunk2 = self(chunk2, self);
      return chunk2;
    };
    return impl(chunk, impl);
  };

  // Map a spill to the chunk it currently occupies.
  auto const lookup = [&] (Vreg r) {
    auto const it = spillToChunk.find(r);
    assertx(it != spillToChunk.end());
    auto const chunk = canonicalize(it->second);
    it->second = chunk;
    return chunk;
  };

  // Merge two chunks together
  auto const combine = [&] (size_t chunk1, size_t chunk2) {
    // Find their canonical forms
    chunk1 = canonicalize(chunk1);
    chunk2 = canonicalize(chunk2);
    assertx(chunk1 != chunk2);
    // Merge the one with the higher index into the lesser. Move the
    // contents of the source into the dest, empty the source, and
    // update the mappings so that the source now canonicalizes to the
    // dest.
    if (chunk1 > chunk2) std::swap(chunk1, chunk2);
    chunks[chunk1] |= chunks[chunk2];
    chunks[chunk2].reset();
    chunkMappings[chunk2] = chunk1;
    return chunk1;
  };

  // Check if any Vreg in c1 interferes with any Vreg in c2. This is a
  // naive N^2 check, but its fine for the small number of spills we
  // encounter.
  auto const interferes = [&] (const VregSet& c1, const VregSet& c2) {
    // We only need to check the first reg as all registers in the
    // chunk must have the same reg class.
    assertx(!c1.empty());
    assertx(!c2.empty());
    if (reg_info(state, *c1.begin()).regClass !=
        reg_info(state, *c2.begin()).regClass) {
      return true;
    }

    for (auto const r1 : c1) {
      auto const it = interferences.find(r1);
      if (it == interferences.end()) continue;
      for (auto const r2 : c2) {
        if (it->second[r2]) return true;
      }
    }

    return false;
  };

  // Loop over the affinities (which have been sorted by
  // weight). Attempt to merge non-interfering chunks.
  for (auto const& affinity : affinities) {
    auto const r1 = affinity.r1;
    auto const r2 = affinity.r2;
    auto const chunk1 = lookup(r1);
    auto const chunk2 = lookup(r2);
    if (chunk1 == chunk2) continue;
    if (interferes(chunks[chunk1], chunks[chunk2])) continue;
    combine(chunk1, chunk2);
  }

  // We can still have non-interfering chunks which do not share any
  // affinities. Attempt to merge these which just minimizes the total
  // number of slots needed.
  for (size_t i = 0; i < chunks.size(); ++i) {
    if (chunks[i].none()) continue;
    for (size_t j = i+1; j < chunks.size(); ++j) {
      if (chunks[j].none()) continue;
      if (interferes(chunks[i], chunks[j])) continue;
      combine(i, j);
    }
  }

  assertx(state.numSpillSlots == 0);
  assertx(state.numWideSpillSlots == 0);

  // Now assign an unique slot for each remaining non-empty chunk.
  for (auto const& chunk : chunks) {
    if (chunk.none()) continue;

    folly::Optional<bool> isWide;
    for (auto const r : chunk) {
      auto const& info = reg_info(state, r);
      assertx(is_spill(info.regClass));
      assertx(!isWide || *isWide == (info.regClass == RegClass::SpillWide));
      isWide = info.regClass == RegClass::SpillWide;

      assertx(state.spillColors.find(r) == state.spillColors.end());
      auto const color = *isWide
        ? Color { SpillSlotWide { state.numWideSpillSlots } }
        : Color { SpillSlot { state.numSpillSlots } };
      auto const DEBUG_ONLY result = state.spillColors.emplace(r, color);
      assertx(result.second);
    }

    if (*isWide) {
      ++state.numWideSpillSlots;
    } else {
      ++state.numSpillSlots;
    }
  }

  // Make sure the spill slot count is round (for alignment).
  if ((state.numSpillSlots % 2) == 1) ++state.numSpillSlots;
}

/*
 * Penalty Vectors:
 *
 * When assigning physical registers for Vregs, we want to choose a
 * register which will minimize total copies in the unit. However this
 * requires forward looking information. The colorer works an
 * instruction at a time and doesn't necessarily know how the Vreg
 * will be used. Penalty vectors are used to represent this
 * information.
 *
 * The vectors are a map of physical registers to penalty weights,
 * where the weight is the number of (dynamic) copies we'll have to
 * pay if you assign the Vreg that physical register. When assigning a
 * Vreg a physical register, we want to assign it the physical
 * register with the lowest weight.
 *
 * Two things affect the penalty vector for a Vreg:
 *
 * - If any physical register is used as a def in an instruction while
 *   the Vreg is alive. This induces a penalty on that physical
 *   register, since if the Vreg is assigned that register, we'll be
 *   forced to emit a copy to a different register. The penalty is the
 *   block weight where the instruction is.
 *
 * - If the Vreg is a hinted use of an instruction with a physical
 *   register def. This induces a penalty on every register *except*
 *   that physical register. This is because if we assign the Vreg any
 *   register except the def, we won't be able to satisfy the hint
 *   (this includes copy instructions).
 *
 * In addition, we want to satisfy as many hints as possible, even if
 * a physical register is not involved. If two Vregs are connected
 * with a hint, and do not interfere (if they interfere the hint can
 * never be satisfied so we ignore it), we merge their penalty vectors
 * together and have them share the same vector. Therefore both Vregs
 * will tend to be assigned the same register and we'll likely satisfy
 * the hint.
 *
 * So, we build up initial penalty vectors for all Vregs using the
 * above criteria. We iterate over the unit backwards, assessing
 * penalties, and merging hinted Vregs.
 *
 * While coloring, we remove penalties as we process
 * instructions. That is, once we process an instruction with one of
 * the above cases, we undo its effect on the penalty vector. The idea
 * being, we've already colored that instruction and therefore it
 * shouldn't influence any future decisions. This means that once
 * we're done coloring, all penalty vectors should contain zeros.
 *
 * Note that the penalty vector doesn't reflect a hint from a physical
 * register use to a non-physical register def (just the
 * opposite). That is dealt with by the colorer directly.
 *
 * For phis, its possible to color the destination of the phi before
 * one of the sources. In that case, we modify the penalty vector on
 * the fly like the first case above. This helps ensure the phi
 * sources will be colored like the def.
 */

// Calculate penalty vectors for all (non-spill) Vregs (and calculate
// spill colors as well).
void calculate_penalties(State& state) {
  auto& unit = state.unit;

  // A penalty index of zero means not set, so have a dummy initial
  // entry.
  state.penalties.emplace_back();

  // While merging penalty vectors, every Vreg will have a "canonical"
  // Vreg. The canonical Vreg will point at the penalty vector that
  // the original Vreg should use. This allows for fast merging.
  jit::vector<Vreg> mappings;
  mappings.resize(unit.next_vr);

  auto const canonicalize = [&] (Vreg r) {
    auto const impl = [&] (Vreg r, auto const& self) {
      auto& r2 = mappings[r];
      if (!r2.isValid() || r == r2) return r;
      assertx(r2 < r);
      r2 = self(r2, self);
      return r2;
    };
    return impl(r, impl);
  };

  jit::vector<SpillAffinity> spillAffinities;

  // Mark that two Vregs are connected by a hint (with the given
  // weight). If the Vregs are spills, this records a spill
  // affinity. Otherwise it attempts to merge their penalty vectors
  // together and marks them as shared.
  auto const unify = [&] (Vreg use, Vreg def, size_t weight) {
    if (use == def) return;
    if (use.isPhys() || is_ignored(state, def)) return;

    // Hints with incompatible reg-classes can never be satisfies, so
    // ignore them.
    auto const cls1 = reg_class(state, use);
    auto const cls2 = reg_class(state, def);
    if (!compatible_reg_classes(cls1, cls2)) return;

    // Specialize case, the hint def is a physical register. We don't
    // unify with physical registers (they don't have penalty
    // vectors), but we can modify the use's penalty vector to bias
    // toward this physical register.
    if (def.isPhys()) {
      // Get the canonical penalty vector
      auto const canon = canonicalize(use);
      auto& info = reg_info(state, canon);
      if (!info.penaltyIdx) {
        info.penaltyIdx = state.penalties.size();
        state.penalties.emplace_back();
      }
      // Assess a penalty for any register, except for the def.
      auto& v = state.penalties[info.penaltyIdx];
      auto const phys = def.physReg();
      for (auto const r : v) {
        if (r != phys) v[r] += weight;
      }
      return;
    }

    // Spills. Just record the affinity.
    if (is_spill(cls1)) {
      assertx(cls1 == cls2);
      spillAffinities.emplace_back(SpillAffinity{use, def, weight});
      return;
    }

    // Attempt to merge their penalty vectors
    use = canonicalize(use);
    def = canonicalize(def);
    // Already merged
    if (use == def) return;

    // We merge the penalty vector for the higher Vreg into the lesser
    // Vreg.
    auto const src = (use < def) ? def : use;
    auto const dst = (use < def) ? use : def;
    mappings[src] = dst;

    // The source penalty vector hasn't been allocated yet, so nothing
    // to merge.
    auto const srcIdx = reg_info(state, src).penaltyIdx;
    if (!srcIdx) return;

    // The dest penalty vector hasn't been allocated yet, so do so
    // now.
    auto& dstInfo = reg_info(state, dst);
    if (!dstInfo.penaltyIdx) {
      dstInfo.penaltyIdx = state.penalties.size();
      state.penalties.emplace_back();
    }

    // Merge the penalty vectors, zeroing out the source vector.
    auto& srcPenalties = state.penalties[srcIdx];
    auto& dstPenalties = state.penalties[dstInfo.penaltyIdx];
    for (auto const r : srcPenalties) {
      dstPenalties[r] += srcPenalties[r];
      srcPenalties[r] = 0;
    }
  };

  jit::fast_map<Vreg, VregSet> spillInterferences;
  VregSet spills;
  VregSet nonSpills;

  // Build up penalty vectors and spill information
  for (auto const b : boost::adaptors::reverse(state.rpo)) {
    auto& block = unit.blocks[b];
    auto const weight = block_weight(state, b);

    // Deal with phijmps specially, since they do not have hinted
    // operands, but we want to treat them as such with the phidef.
    if (block.code.back().op == Vinstr::phijmp) {
      auto const& phijmp = block.code.back().phijmp_;
      for (auto const succ : succs(block)) {
        auto const& succBlock = unit.blocks[succ];
        assertx(succBlock.code.front().op == Vinstr::phidef);
        auto const& phidef = succBlock.code.front().phidef_;
        auto const& liveIn = state.liveIn[succ];
        auto const& uses = unit.tuples[phijmp.uses];
        auto const& defs = unit.tuples[phidef.defs];
        assertx(uses.size() == defs.size());
        for (size_t i = 0; i < uses.size(); ++i) {
          if (liveIn[uses[i]]) continue;
          unify(uses[i], defs[i], weight);
        }
      }
    }

    // We need liveness information to know which hints are
    // unsatisfiable, so walk backwards modifying it on the fly.
    auto live = state.liveOut[b];

    for (auto& inst : boost::adaptors::reverse(block.code)) {
      // Attempt to unify hinted src/dest pairs. Each hinted src can
      // be used only once (more than one is not satisfiable), so
      // ignore multiples.
      VregSet hints;
      auto const processDef = [&] (Vreg r, Vreg hint) {
        if (!r.isPhys() && is_colorable_reg(reg_class(state, r))) {
          nonSpills.add(r);
        }
        if (!hint.isValid() || live[hint] || hints[hint]) return;
        hints.add(hint);
        unify(hint, r, weight);
      };
      visitDefsWithHints(unit, inst, processDef);

      auto const& defs = defs_set_cached(state, inst);

      // If we don't have spills and there's no physical registers in
      // the defs, we can skip the below logic since the penalty
      // vectors can't change. We only have to update the liveness
      // information.
      if (state.spilled || defs.containsPhys()) {
        // Add defs and acrosses to the live set, since we need to
        // know which Vregs interfer.
        live |= defs;
        live |= acrosses_set_cached(state, inst);

        for (auto const defReg : defs) {
          // If a physical register is defined, assess a penalty for
          // it to any live Vreg penalty vectors.
          if (defReg.isPhys()) {
            auto const phys = defReg.physReg();
            for (auto const liveReg : live) {
              if (liveReg.isPhys() || defs[liveReg]) continue;
              if (!is_colorable_reg(reg_class(state, liveReg))) continue;

              // Assess a penalty for the def physical register since
              // any Vreg with that register at this point will need
              // to be moved.
              auto const canon = canonicalize(liveReg);
              auto& info = reg_info(state, canon);
              if (!info.penaltyIdx) {
                info.penaltyIdx = state.penalties.size();
                state.penalties.emplace_back();
              }
              state.penalties[info.penaltyIdx][phys] += weight;
            }
            continue;
          }

          // If the def is a spill, remember it, and mark any other
          // live spills as interfering with it.
          auto const& info = reg_info(state, defReg);
          if (!is_spill(info.regClass)) continue;

          spills.add(defReg);
          for (auto const liveReg : live) {
            if (defReg == liveReg) continue;
            auto const cls = reg_class(state, liveReg);
            if (info.regClass != cls) continue;
            spillInterferences[defReg].add(liveReg);
            spillInterferences[liveReg].add(defReg);
          }
        }
      }

      // This is sufficient for liveness if we don't have physical
      // register defs.
      live -= defs;
      live |= uses_set_cached(state, inst);
    }

    // Assert that our liveness information matches what liveness
    // analysis calculated originally.
    assertx(
      [&] {
        for (auto const r : state.liveIn[b]) always_assert(live[r]);
        for (auto const r : live) {
          if (state.liveIn[b][r]) continue;
          always_assert(!r.isPhys());
          always_assert(reg_info(state, r).regClass == RegClass::SF);
        }
        return true;
      }()
    );
  }

  // Update the penalty vector index for each Vreg to point directly
  // at its penalty vector, rather than going through its canonical
  // mapping. Make sure every non-spill Vreg has a penalty vector
  // assigned. If they don't have one at this point, it means there's
  // no penalties, so give them a default one. Keep track of the
  // highest penalty vector index seen, so we can discard unused ones.
  size_t maxPenaltyIdx = 0;
  for (auto const r : nonSpills) {
    auto& info = reg_info(state, r);
    auto const canon = canonicalize(r);
    info.penaltyIdx = reg_info(state, canon).penaltyIdx;
    if (!info.penaltyIdx) {
      info.penaltyIdx = state.penalties.size();
      state.penalties.emplace_back();
    }
    maxPenaltyIdx = std::max(maxPenaltyIdx, info.penaltyIdx);
  }
  state.penalties.resize(maxPenaltyIdx + 1);

  // If we found any spills, used the interference and affinity
  // information to calculate slots for them.
  if (spills.any()) {
    assertx(state.spilled);
    assign_spill_colors(
      state,
      spills,
      spillInterferences,
      std::move(spillAffinities)
    );
  }
}

// The unit can be colored in any order thats dominance preserving
// (this includes RPO). However, we can get better coloring by walking
// the CFG in an order that prioritizes higher weight blocks. This
// function calculates such a block order.
BlockVector calculate_block_color_order(const State& state) {
  // With 1 or 2 block units, the visit order is fixed, so the result
  // will always be the same as RPO.
  if (state.rpo.size() <= 2) return state.rpo;

  // Build up traces for each block. The trace for a block is its own
  // weight plus the maximum of the traces of its predecessors. It
  // reflects the maximum possible execution count to get to this
  // block.
  jit::vector<size_t> trace;
  trace.resize(state.unit.blocks.size());

  for (auto const b : state.rpo) {
    size_t t = 0;
    for (auto const pred : state.preds[b]) t = std::max(t, trace[pred]);
    trace[b] = t + block_weight(state, b);
    assertx(trace[b] > 0);
  }

  // Sort the blocks by trace. Bigger traces first. Use RPO order to
  // break ties.
  auto blocks = state.rpo;
  std::sort(
    blocks.begin(),
    blocks.end(),
    [&] (Vlabel a, Vlabel b) {
      if (trace[a] != trace[b]) return trace[a] > trace[b];
      return state.rpoOrder[a] < state.rpoOrder[b];
    }
  );

  /*
   * Now use the traces to find a good block ordering. We want to
   * color the blocks with the higher traces first (they'll be
   * executed most often). However, since we need to preserve
   * dominance order, we cannot color a block until we've colored at
   * least one of its predecessors.
   *
   * So, select the unprocessed block with the highest trace. Select
   * the block's predecessor with the highest weight (if any) and
   * recurse. Once the recursion returns, mark the current block as
   * processed and add it to the order. Repeat this until no
   * unprocessed blocks remain.
   *
   * NB: In RPO order if the unit has no loops, all predecessors for a
   * block will be ordered before that block. This is *not* the case
   * for this ordering.
   */
  BlockVector order;
  boost::dynamic_bitset<> processed;
  order.reserve(blocks.size());
  processed.resize(state.unit.blocks.size());

  auto const addTrace = [&] (Vlabel b, auto const& self) {
    if (processed[b]) return;
    size_t bestTrace = 0;
    Vlabel bestPred;
    for (auto const pred : state.preds[b]) {
      if (state.rpoOrder[pred] >= state.rpoOrder[b]) continue;
      if (bestTrace >= trace[pred]) continue;
      bestTrace = trace[pred];
      bestPred = pred;
    }
    if (bestPred.isValid()) self(bestPred, self);
    order.emplace_back(b);
    processed[b] = true;
  };
  for (auto const b : blocks) addTrace(b, addTrace);

  return order;
}

// FreeRegs is responsible for tracking which physical registers are free and
// assigning them.

struct FreeRegs {
  explicit FreeRegs(State& state)
    : state{&state}
    , regs{state.gpUnreserved | state.simdUnreserved} {}

  // Choose an appropriate coloring for the given Vreg. This merely
  // chooses the color, and does not mark it as taken or modify any
  // other internal state. If the Vreg represents a spill, it will be
  // chosen from the pre-computed spill colors. Otherwise, a physical
  // register is chosen to try to maximize satisfied hints.
  //
  // For the register case, the heuristic may decide to migrate a
  // (already taken) physical register to a different (free) physical
  // register. In that case, the "moveDest" field of the coloring will
  // be set to the new physical register. One can use assignedTo() on
  // the returned color to get the source physical register.
  //
  // "moveWeight" is a lambda, which given two physical registers,
  // returns the estimated weight of moving between them.
  //
  // If the given Vreg is a physical register, that physical register
  // will always be chosen, if available. Otherwise the function will
  // consider moving the old Vreg from that physical register.

  struct Coloring {
    Coloring(PhysReg r) : color{r} {}
    Coloring(Color c) : color{c} {}
    Coloring(PhysReg r, PhysReg m) : color{r}, moveDest{m} {}
    Color color;
    PhysReg moveDest = InvalidReg;
  };

  template <typename W>
  Coloring choose(Vreg r, W&& moveWeight) const {
    // Find best physical register for a Vreg, choosing from the set
    // of registers in "mask".
    auto const findBest = [&] (RegSet mask) -> Coloring {
      struct PhysWeight {
        PhysReg reg;
        int64_t weight;
      };
      struct PhysWeightArray {
        std::array<PhysWeight, PhysReg::kMaxRegs> pairs;
        size_t size = 0;
      };

      // Build a list of candidate physical registers from
      // 'candidates', sorted by their weight ascending (calculated by
      // "calc").
      auto const buildPhysWeights = [this] (Vreg seed,
                                            RegSet candidates,
                                            auto&& calc) {
        PhysWeightArray weights;
        // Build the weights
        candidates.forEach(
          [&] (PhysReg r) {
            weights.pairs[weights.size++] = PhysWeight{r, calc(r)};
          }
        );
        // And sort them. Physical registers with lower weights are
        // always preferred. With equal weight, we prefer available
        // registers over unavailable ones. Finally, to break any
        // remaining ties, we hash the associated Vreg and the
        // physical register to come up with a pseudo-random
        // order. This helps avoid pathologies caused by lots of
        // physical registers with the same weight.
        std::sort(
          weights.pairs.begin(), weights.pairs.begin() + weights.size,
          [&] (const PhysWeight& p1, const PhysWeight& p2) {
            if (p1.weight != p2.weight) return p1.weight < p2.weight;
            auto const avail1 = regs.contains(p1.reg);
            auto const avail2 = regs.contains(p2.reg);
            if (avail1 != avail2) return avail1 > avail2;

            auto const perturb1 = hash_int64_pair(seed, Vreg{p1.reg});
            auto const perturb2 = hash_int64_pair(seed, Vreg{p2.reg});
            if (perturb1 != perturb2) return perturb1 < perturb2;

            return Vreg{p1.reg} < Vreg{p2.reg};
          }
        );
        return weights;
      };

      auto const weights = [&] {
        // If the Vreg is a physical register, there's only one
        // candidate (itself) the weight is irrelevant here.
        if (r.isPhys()) {
          PhysWeightArray weights;
          weights.pairs[0] = PhysWeight{r.physReg(), 0};
          weights.size = 1;
          return weights;
        }

        // Otherwise build up the full list, using the penalty vectors
        // to order them.
        auto const& info = reg_info(*state, r);
        assertx(info.penaltyIdx > 0 &&
                info.penaltyIdx < state->penalties.size());
        auto const& penalties = state->penalties[info.penaltyIdx];
        return buildPhysWeights(
          r, mask, [&] (PhysReg r) { return penalties[r]; }
        );
      }();

      // Examine each candidate starting from the best.
      for (size_t i = 0; i < weights.size; ++i) {
        auto const current = weights.pairs[i].reg;
        auto const weight = weights.pairs[i].weight;

        // This candidate is available, just use it.
        if (regs.contains(current)) return current;

        // Otherwise, if we don't want moves here, try the next
        // one. If this is the last candidate, we're done since the
        // heuristic looks at the next possible candidate.
        if (i+1 == weights.size && !r.isPhys()) continue;

        // We support moves, so see if its profitable to generate one.
        auto const assignedTo = occupied[current];
        assertx(assignedTo.isValid());
        // Can never move physical registers
        if (assignedTo.isPhys()) continue;

        // Get the penalty vector of the Vreg which might be moved.
        auto const& movePenalties = [&] {
          auto const idx = reg_info(*state, assignedTo).penaltyIdx;
          assertx(idx > 0 && idx < state->penalties.size());
          return state->penalties[idx];
        }();

        // Build a candidate list to move the Vreg to. We only support
        // moving to a free physical register.
        auto const moveWeights = buildPhysWeights(
          assignedTo, mask & regs,
          [&] (PhysReg r) { return movePenalties[r]; }
        );

        // No free physical register to move to, this candidate is
        // done.
        if (!moveWeights.size) continue;

        // If we're trying to select for a physical register, we
        // *have* to move. So just go with the best one, regardless of
        // its weights.
        if (r.isPhys()) {
          assertx(current == r.physReg());
          return {current, moveWeights.pairs[0].reg};
        }

        // Otherwise check if the penalty for the move is less than
        // the win from moving it. Get the delta between the weight
        // for the current candidate and the next best one. The
        // penalty for the move is the difference between the moved
        // Vreg's current register to its new one, plus the weight of
        // the move itself. If the penalty for the move is less than
        // the candidate's penalty delta, this is an overall win and
        // we should do it.
        auto const nextWeight = weights.pairs[i+1].weight;
        auto const moveWin = nextWeight - weight;
        auto const movePenalty =
          (moveWeights.pairs[0].weight - movePenalties[current]) +
          moveWeight(current, moveWeights.pairs[0].reg);
        if (moveWin > movePenalty) {
          return {current, moveWeights.pairs[0].reg};
        }
      }

      return InvalidReg;
    };

    // Lookup the pre-computed spill slot for a spill. This never
    // changes.
    auto const findSlot = [&] {
      auto const it = state->spillColors.find(r);
      assertx(it != state->spillColors.end());
      return it->second;
    };

    switch (reg_class(*state, r)) {
      case RegClass::GP:
      case RegClass::AnyNarrow: return findBest(state->gpUnreserved);
      case RegClass::SIMD:
      case RegClass::SIMDWide: return findBest(state->simdUnreserved);
      case RegClass::Spill: {
        auto const c = findSlot();
        assertx(is_color_spill_slot(c));
        return c;
      }
      case RegClass::SpillWide: {
        auto const c = findSlot();
        assertx(is_color_spill_slot_wide(c));
        return c;
      }
      case RegClass::Any:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Mark the given physical register as taken. The register should not already
  // be reserved. The RegClass should be appropriate for that register.
  void reserve(Vreg r, Color c, RegClass cls) {
    switch (cls) {
      case RegClass::GP:
      case RegClass::AnyNarrow:
      case RegClass::SIMD:
      case RegClass::SIMDWide: {
        auto const reg = color_reg(c);
        assertx(regs.contains(reg));
        assertx(!occupied[reg].isValid());
        regs -= reg;
        occupied[reg] = r;
        return;
      }
      case RegClass::Spill:
        assertx(is_color_spill_slot(c));
        return;
      case RegClass::SpillWide:
        assertx(is_color_spill_slot_wide(c));
        return;
      case RegClass::Any:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Mark the given physical register as available. The register should be
  // already marked as taken. The RegClass should be appropriate for that
  // register.
  void release(Color c, RegClass cls) {
    switch (cls) {
      case RegClass::GP:
      case RegClass::AnyNarrow:
      case RegClass::SIMD:
      case RegClass::SIMDWide: {
        auto const r = color_reg(c);
        assertx(!regs.contains(r));
        assertx(occupied[r].isValid());
        regs |= r;
        occupied[r] = Vreg{};
        return;
      }
      case RegClass::Spill:
        assertx(is_color_spill_slot(c));
        return;
      case RegClass::SpillWide:
        assertx(is_color_spill_slot_wide(c));
        return;
      case RegClass::Any:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Check if the given physical register (with appropriate RegClass) is taken.
  bool available(Color c, RegClass cls) const {
    switch (cls) {
      case RegClass::GP:
      case RegClass::AnyNarrow:
      case RegClass::SIMD:
      case RegClass::SIMDWide:
        return regs.contains(color_reg(c));
      case RegClass::Spill:
        assertx(is_color_spill_slot(c));
        return true;
      case RegClass::SpillWide:
        assertx(is_color_spill_slot_wide(c));
        return true;
      case RegClass::Any:
      case RegClass::SF:
        break;
    }
    always_assert(false);
  }

  // Express a move between two physical registers. The destination
  // should not be assigned a Vreg, while the source should be. The
  // Vreg will now be assigned the destination register.
  void move(PhysReg src, PhysReg dst) {
    assertx(src != dst);
    assertx(!regs.contains(src));
    assertx(regs.contains(dst));
    auto const v = occupied[src];
    assertx(v.isValid());
    assertx(!occupied[dst].isValid());
    occupied[dst] = v;
    occupied[src] = Vreg{};
    regs -= dst;
    regs |= src;
  }

  // Return all Vregs which are currently assigned a physical register
  // (so ignoring spills).
  VregSet live() const {
    VregSet out;
    for (auto const r : occupied) {
      if (!occupied[r].isValid()) continue;
      assertx(!regs.contains(r));
      out.add(occupied[r]);
    }
    return out;
  }

  // Return the Vreg that a physical register is currently assigned
  // to. The physical register should be currently used.
  Vreg assignedTo(PhysReg r) const {
     assertx(!regs.contains(r));
     assertx(occupied[r].isValid());
     return occupied[r];
  }

  // Return physical registers not currently allocated.
  const RegSet& freeRegs() const { return regs; }
private:
  State* state;
  RegSet regs;
  PhysReg::Map<Vreg> occupied;
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

// Release the allocated colors from any Vregs (from the given
// candidate set) which are dead at the given position, or in the
// optional kills set. If "released" is provided, then fill it with
// the physical registers freed.
void release_dead_regs(State& state,
                       FreeRegs& free,
                       const VregSet& candidates,
                       const VregSet* kills,
                       Vlabel block,
                       size_t instIdx,
                       RegSet* released = nullptr) {
  for (auto const r : candidates) {
    auto const info = reg_color_info(state, r);
    if (!is_colorable(info.regClass)) continue;
    // Since we walk the unit in a dominance preserving order, every Vreg
    // defined at this point should have a color assigned.
    assertx(!is_color_none(info.color));
    assertx(is_spill(info.regClass) ||
            !free.available(info.color, info.regClass));
    if (live_in_at(state, r, block, instIdx + 1) &&
        (!kills || !r.isPhys() || !(*kills)[r])) {
      continue;
    }
    if (released && is_color_reg(info.color)) {
      *released |= color_reg(info.color);
    }
    free.release(info.color, info.regClass);
    if (debug && !is_spill(info.regClass) && !r.isPhys()) {
      // In debug builds set the color back to None so we'll catch
      // errors more quickly.
      reg_info(state, r).color = Color{};
    }
  }
}

// Whether two physical registers can be exchanged with a single
// instruction.
bool can_single_swap(PhysReg r1, PhysReg r2) {
  switch (arch()) {
    case Arch::X64:   return r1.isGP() && r2.isGP();
    case Arch::ARM:   return false;
    case Arch::PPC64: return false;
  }
  always_assert(false);
}

// Run the coloring logic for a single instruction. Returns the number
// of instructions added.
size_t color_inst(State& state,
                  FreeRegs& free,
                  Vinstr& inst,
                  Vlabel block,
                  size_t instIdx,
                  const boost::dynamic_bitset<>& processed,
                  const jit::vector<boost::dynamic_bitset<>>& phiAdjustments) {
  auto const& acrosses = acrosses_set_cached(state, inst);
  auto const& uses = uses_set_cached(state, inst) - acrosses;
  auto const& defs = defs_set_cached(state, inst);

  // Phidefs should be done as part of block initialization.
  assertx(inst.op != Vinstr::phidef);

  auto& unit = state.unit;

  auto const weight = block_weight(state, block);

  // If this is a phijmp, we're at the end of the block. If our
  // successor (there should only be one) is already processed, it
  // means we've biased the penalty vectors for the phijmp sources (to
  // try to make their assignments match the ones in the
  // successor). Since we're done with this block, we need to undo
  // those penalty vector adjustments now. We use the phi adjustment
  // metadata to know which ones to undo (its hard to determine
  // otherwise).
  if (inst.op == Vinstr::phijmp) {
    assertx(defs.none());
    assertx(instIdx == unit.blocks[block].code.size() - 1);
    auto const successors = succs(unit.blocks[block]);
    assertx(successors.size() == 1);
    auto const successor = successors[0];
    if (processed[successor]) {
      auto const& phidef = unit.blocks[successor].code.front();
      assertx(phidef.op == Vinstr::phidef);
      auto const& uses = unit.tuples[inst.phijmp_.uses];
      auto const& defs = unit.tuples[phidef.phidef_.defs];
      assertx(uses.size() == defs.size());

      auto const& adjustments = phiAdjustments[block];
      assertx(adjustments.empty() || adjustments.size() == uses.size());

      // If adjustments[i] is true, we it means we adjusted the
      // penalty vector for the Vreg at uses[i] by the weight of this
      // block towards defs[i]. Undo this.
      for (size_t i = 0; i < adjustments.size(); ++i) {
        if (!adjustments[i]) continue;
        assertx(!is_ignored(state, defs[i]));
        assertx(!is_ignored(state, uses[i]));
        assertx(defs[i].isPhys());
        assertx(!uses[i].isPhys());

        auto& info = reg_info(state, uses[i]);
        assertx(is_colorable_reg(info.regClass));
        auto const idx = info.penaltyIdx;
        assertx(idx > 0 && idx < state.penalties.size());
        auto& v = state.penalties[idx];

        auto const phys = defs[i].physReg();
        for (auto const r : v) {
          if (r != phys) {
            assertx(v[r] >= weight);
            v[r] -= weight;
          }
        }
      }
    }
  }
  // For a phijmp, most of the below logic will be skipped (because it
  // has no defs).

  // If this instruction has a hint from Vreg sources to physical
  // registers, we'll have incorporated this fact into the penalty
  // vectors of those Vregs (bias it towards that physical
  // register). We want to undo these adjustments, but we have to do
  // it after selection below. Record the adjustments to be undone
  // afterwards.
  struct Adjustment {
    Adjustment(PenaltyVector* p, PhysReg s) : penalties{p}, skip{s} {}
    PenaltyVector* penalties;
    PhysReg skip;
  };
  jit::vector<Adjustment> penaltyAdjustments;
  if (defs.containsPhys()) {
    VregSet hints;
    visitDefsWithHints(
      unit, inst,
      [&] (Vreg def, Vreg hint) {
        // This matches the logic for biasing or not when we processed
        // the phidef.
        if (!hint.isValid() || !def.isPhys() || hint.isPhys() || hints[hint]) {
          return;
        }
        hints.add(hint);

        auto const& info = reg_info(state, hint);
        if (!is_colorable_reg(info.regClass)) return;

        auto const cls1 = reg_class(state, hint);
        auto const cls2 = reg_class(state, def);
        if (!compatible_reg_classes(cls1, cls2)) return;

        if (acrosses[hint] || live_in_at(state, hint, block, instIdx + 1)) {
          return;
        }

        auto const idx = info.penaltyIdx;
        assertx(idx > 0 && idx < state.penalties.size());
        penaltyAdjustments.emplace_back(&state.penalties[idx], def.physReg());
      }
    );
  }

  // Rewrite the sources of this instruction to become the physical
  // registers the Vregs are already assigned to. Record the physical
  // registers corresponding to "across" Vregs.
  RegSet acrossPhys;
  visitRegsMutable(
    unit,
    inst,
    [&] (Vreg r) -> Vreg {
      if (r.isPhys()) return r;
      auto const& info = reg_info(state, r);
      if (is_spill(info.regClass)) return r;
      if (info.regClass == RegClass::SF) return state.abi.sf.choose();
      auto const c = color_reg(info.color);
      assertx(c != state.scratch);
      if (acrosses[r]) acrossPhys |= c;
      return c;
    },
    [] (Vreg r) { return r; }
  );

  // Release any colors held by now dead uses. This doesn't include
  // acrosses, which we removed from the uses set. Record those
  // physical registers which were released.
  RegSet usesNowDead;
  release_dead_regs(state, free, uses, &defs, block, instIdx, &usesNowDead);

  // Snapshot the free physical registers before we begin coloring.
  auto const freeRegsBeforeDefs = free.freeRegs();

  // The sources (and thus all the hints) of this instruction have
  // been rewritten to be physical registers. If the corresponding def
  // is not a physical register, we want to bias that def's penalty
  // vector towards the hinted source (to satisfy the hint). Do that
  // here, and record the adjustment to be undone afterwards.
  visitDefsWithHints(
    unit, inst,
    [&] (Vreg def, Vreg hint) {
      if (!hint.isValid() || !hint.isPhys() || def.isPhys()) return;

      // If the hint isn't free at the def, its not satisfiable.
      auto const phys = hint.physReg();
      if (!freeRegsBeforeDefs.contains(phys)) return;

      auto const& info = reg_info(state, def);
      if (!is_colorable_reg(info.regClass)) return;

      assertx(info.penaltyIdx > 0 && info.penaltyIdx < state.penalties.size());
      auto& v = state.penalties[info.penaltyIdx];
      for (auto const r : v) {
        if (r != phys) v[r] += weight;
      }
      penaltyAdjustments.emplace_back(&v, phys);
    }
  );

  // Perform the actual physical register selection. For Vregs
  // representing non-physical registers, we try to find the best
  // physical register for it, possibly moving another Vreg. For Vregs
  // representing physical registers, we don't have any choice in the
  // matter, but we still might have to migrate a Vreg out of the
  // desired physical register.
  VregList copySrcs;
  VregList copyDsts;
  for (auto const r : defs) {
    auto const cls = reg_class(state, r);
    if (!is_colorable(cls)) continue;

    auto info = r.isPhys() ? nullptr : &reg_info(state, r);

    // This Vreg is being defined here, so it better not have a
    // color already.
    assertx(!info || is_color_none(info->color));

    // Pick a color, assert we found something (which we always
    // should), handle any possible moves, and then reserve it. If the
    // Vreg is a physical register, it will always return itself, but
    // will move a Vreg if it resides in the destination.
    auto const coloring = free.choose(
      r,
      [&] (Vreg from, Vreg to) -> int64_t {
        // Move weight calculation. This matches the logic below for
        // determining whether a copy is actually needed.
        if (freeRegsBeforeDefs.contains(from)) return 0;
        auto const it = std::find(copySrcs.begin(), copySrcs.end(), from);
        if (it != copySrcs.end()) return 0;
        if (!usesNowDead.contains(to)) return weight;
        // If we can't emit a swap, assume it will take three
        // instructions.
        return can_single_swap(from, to) ? weight : (weight * 3);
      }
    );
    assert_found_color(r, coloring.color);

    // Some other Vreg has to be moved to make way for this physical
    // register.
    if (coloring.moveDest != InvalidReg) {
      // Update metadata to track the move.
      auto const movedFrom = color_reg(coloring.color);
      auto const vreg = free.assignedTo(movedFrom);
      free.move(movedFrom, coloring.moveDest);
      reg_info(state, vreg).color = coloring.moveDest;

      // Add to the copy list so we'll generate a copy before this
      // instruction. If the copy source is a destination of another
      // copy, just update the original copy. If the copy source is
      // dead when the instruction executes, no copy is actually
      // needed. Finally, if the copy source is already being moved to
      // another register, no copy is needed (its already being
      // saved).
      std::replace(
        copyDsts.begin(), copyDsts.end(), movedFrom, coloring.moveDest
      );
      if (!freeRegsBeforeDefs.contains(movedFrom)) {
        if (std::find(copySrcs.begin(), copySrcs.end(), movedFrom) ==
            copySrcs.end()) {
          copySrcs.emplace_back(movedFrom);
          copyDsts.emplace_back(coloring.moveDest);
        }
      }
    }

    if (info) info->color = coloring.color;
    free.reserve(r, coloring.color, cls);
  }

  // Undo all the penalty adjustments we did before coloring.
  for (auto const p : penaltyAdjustments) {
    auto& v = *p.penalties;
    for (auto const r : v) {
      if (r == p.skip) continue;
      assertx(v[r] >= weight);
      v[r] -= weight;
    }
  }

  // Rewrite the sources and the defs of this instruction to take into
  // account the physical register selections and any moves we need to
  // generate.
  visitRegsMutable(
    unit,
    inst,
    [&] (Vreg r) -> Vreg {
      assertx(r.isPhys() || is_spill(reg_class(state, r)));
      if (inst.op == Vinstr::copy || inst.op == Vinstr::copyargs) {
        // If the current instruction is a copy, we'll fold the new
        // copies into it below, so nothing needs to be changed.
        return r;
      }
      if (r.isPhys()) {
        // Rewrite already colored source to take into account moves
        // we need to generate.
        auto const phys = r.physReg();
        assertx(copySrcs.size() == copyDsts.size());
        for (size_t i = 0; i < copyDsts.size(); ++i) {
          if (!usesNowDead.contains(copyDsts[i]) &&
              !acrossPhys.contains(copySrcs[i])) {
            // If the copy destination isn't used by the instruction,
            // and the copy source isn't across, then the source and
            // destination will contain the same value when the
            // instruction executes, so we don't actually have to
            // change anything.
            continue;
          }
          // Otherwise rewrite as appropriate. We need to check both
          // copySrcs and copyDsts because we might be swapping them
          // and both can appear as uses in the current instruction.
          if (copyDsts[i] == phys) return copySrcs[i];
          if (copySrcs[i] == phys) return copyDsts[i];
        }
      }
      return r;
    },
    [&] (Vreg r) -> Vreg {
      if (r.isPhys()) return r;
      auto const& info = reg_info(state, r);
      if (is_spill(info.regClass)) return r;
      if (info.regClass == RegClass::SF) return state.abi.sf.choose();
      auto const c = color_reg(info.color);
      assertx(c != state.scratch);
      return c;
    }
  );
  invalidate_cached_operands(inst);

  // If this instruction defined any physical registers, we'll have
  // incorporated this information into the initial penalty vectors
  // for any live Vreg. Since this instruction is now processed, we
  // want to undo this adjustment.
  if (defs.containsPhys()) {
    for (auto const r : free.live()) {
      // VregSet always provides the physical registers first, so if
      // we see a non-physical Vreg, we've seen all the physical
      // registers.
      if (r.isPhys()) continue;
      if (!is_colorable_reg(reg_class(state, r))) continue;
      auto const idx = reg_info(state, r).penaltyIdx;
      assertx(idx > 0 && idx < state.penalties.size());
      auto& v = state.penalties[idx];
      for (auto const d : defs) {
        if (!d.isPhys()) break;
        auto const phys = d.physReg();
        assertx(v[phys] >= weight);
        v[phys] -= weight;
      }
    }
  }

  // Release any now dead defs or acrosses.
  release_dead_regs(state, free, acrosses, nullptr, block, instIdx);
  release_dead_regs(state, free, defs, nullptr, block, instIdx);

  assertx(copySrcs.size() == copyDsts.size());
  if (copySrcs.empty()) return 0;

  // Emit any needed copies:

  // Special case. If the previous instruction to this one defines the
  // copy source, we possibly just modify it to write to the copy
  // destination instead.
  if (instIdx > 0) {
    auto& last = unit.blocks[block].code[instIdx - 1];
    // Rewriting phidefs is tricky because of the penalty vector
    // biasing, so skip for now.
    if (last.op != Vinstr::phidef) {
      auto const& lastDefs = defs_set_cached(state, last);
      auto const& lastAcrosses = acrosses_set_cached(state, last);
      size_t copyIdx = 0;
      for (size_t i = 0; i < copySrcs.size(); ++i) {
        // We can safely perform this optimization if the destination
        // is not used by the current instruction (can't clobber
        // otherwise), if the source is actually defined by the
        // previous instruction, if the destination isn't also defined
        // by the previous instruction, and if the destination isn't
        // an across operand for the previous instruction.
        if (!usesNowDead.contains(copyDsts[i]) &&
            lastDefs[copySrcs[i]] &&
            !lastDefs[copyDsts[i]] &&
            !lastAcrosses[copyDsts[i]]) {
          // Rewrite the previous instruction to write to the
          // destination.
          visitRegsMutable(
            unit, last,
            [&] (Vreg r) { return r; },
            [&] (Vreg r) -> Vreg {
              return (copySrcs[i] == r) ? copyDsts[i] : r;
            }
          );
          // And rewrite the current instruction to take the
          // destination as the source.
          visitRegsMutable(
            unit, inst,
            [&] (Vreg r) { return (copySrcs[i] == r) ? copyDsts[i] : r; },
            [&] (Vreg r) { return r; }
          );
        } else {
          // Otherwise, no rewrite, keep this pair in the list.
          copySrcs[copyIdx] = copySrcs[i];
          copyDsts[copyIdx] = copyDsts[i];
          ++copyIdx;
        }
      }
      // Did we shrink the copy list (and hence changed something)?
      if (copyIdx < copySrcs.size()) {
        invalidate_cached_operands(last);
        invalidate_cached_operands(inst);
      }
      // If we eliminated all copies, we're done
      if (copyIdx == 0) return 0;
      copySrcs.resize(copyIdx);
      copyDsts.resize(copyIdx);
    }
  }

  // More special cases: If this instruction is a copy, or copyargs,
  // we can fold the copies into the current instruction (this works
  // because copyargs is a parallel copy).
  if (inst.op == Vinstr::copy) {
    copySrcs.emplace_back(inst.copy_.s);
    copyDsts.emplace_back(inst.copy_.d);
    inst.op = Vinstr::copyargs;
    inst.copyargs_.s = unit.makeTuple(std::move(copySrcs));
    inst.copyargs_.d = unit.makeTuple(std::move(copyDsts));
    invalidate_cached_operands(inst);
    return 0;
  } else if (inst.op == Vinstr::copyargs) {
    auto& srcs = unit.tuples[inst.copyargs_.s];
    auto& dsts = unit.tuples[inst.copyargs_.d];
    for (size_t i = 0; i < copySrcs.size(); ++i) {
      srcs.emplace_back(copySrcs[i]);
      dsts.emplace_back(copyDsts[i]);
    }
    invalidate_cached_operands(inst);
    return 0;
  }

  // If the destination of the copy is used as a source of this
  // instruction, we can't just emit a copy overwriting it before the
  // instruction. Instead we swap the source and destination. This is
  // fine because the destination is guaranteed to be dead before the
  // instruction writes to its defs. We do this by inserting an
  // opposite order of the copy in the list, which the phi lowering
  // logic will turn into a swap.
  auto const origSize = copySrcs.size();
  for (size_t i = 0; i < origSize; ++i) {
    if (!usesNowDead.contains(copyDsts[i])) continue;
    auto const src = copySrcs[i];
    auto const dst = copyDsts[i];
    copySrcs.emplace_back(dst);
    copyDsts.emplace_back(src);
  }

  // Emit the copies
  vmodify(
    unit, block, instIdx,
    [&] (Vout& v) {
      if (copySrcs.size() == 1) {
        v << copy{copySrcs[0], copyDsts[0]};
      } else {
        v << copyargs{
          unit.makeTuple(std::move(copySrcs)),
          unit.makeTuple(std::move(copyDsts))
        };
      }
      return 0;
    }
  );

  return 1;
}

// Flat mapping of Vreg to a physical register its been assigned
// to. This is used to record Vreg to physical register assignments at
// block entry/exit. The block will have an entry for each
// live-in/live-out Vreg (in that order), except for Vregs
// representing physical registers. Registers will always be present
// in ascending order.
using AssignmentVector = jit::vector<std::pair<Vreg, PhysReg>>;

// Initialize the initial set of Vreg to physical register assignments
// for a block, taking into account predecessors. If the block has one
// predecessor, we just copy over the assignments from that. If it has
// multiple predecessors, we invoke the register selection logic, but
// biasing (via the penalty vectors) towards assignments which match
// the predecessors. In extreme cases, we may choose a register that
// none of the predecessors selected. This all means the block can
// have register assignments which don't match their predecessors or
// successors, but that will be resolved afterwards.
size_t color_block_initialize(
    State& state,
    FreeRegs& free,
    Vlabel b,
    const jit::vector<AssignmentVector>& assignments,
    const boost::dynamic_bitset<>& processed,
    jit::vector<boost::dynamic_bitset<>>& phiAdjustments
) {
  auto const reserve = [&] (Vreg r, PhysReg phys) {
    auto const cls = reg_class(state, phys);
    always_assert(free.available(phys, cls));
    free.reserve(r, phys, cls);
    if (r.isPhys()) return;
    auto& info = reg_info(state, r);
    assertx(is_color_none(info.color));
    info.color = phys;
  };

  auto const& preds = state.preds[b];
  if (preds.empty()) {
    assertx(b == state.unit.entry);
    assertx(state.liveIn[b].allPhys());
    for (auto const r : state.liveIn[b]) reserve(r, r.physReg());
    return 0;
  }

  if (preds.size() == 1) {
    // If this block has exactly one predecessor, it must already be
    // processed (dominance preserving order guarantees this). We can
    // simply use the same assignments the predecessor used for Vregs.
    assertx(processed[preds[0]]);

    auto const& predOut = assignments[preds[0]];
    size_t predOutIdx = 0;
    for (auto const r : state.liveIn[b]) {
      auto const cls = reg_class(state, r);
      if (!is_colorable_reg(cls)) continue;

      if (r.isPhys()) {
        reserve(r, r.physReg());
        continue;
      }

      // The predecessor can have a Vreg which is live-out (and thus
      // in the assignment vector), but isn't live-in to this
      // block. Skip over such Vregs. This is only possible if the
      // predecessor has multiple successors.
      assertx(predOutIdx < predOut.size());
      while (r != predOut[predOutIdx].first) {
        ++predOutIdx;
        assertx(predOutIdx < predOut.size());
      }

      reserve(r, predOut[predOutIdx].second);
    }

    // If the block doesn't begin with a phidef, we're done.
    auto& inst = state.unit.blocks[b].code.front();
    if (inst.op != Vinstr::phidef) return 0;

    auto const phijmp = state.unit.blocks[preds[0]].code.back();
    assertx(phijmp.op == Vinstr::phijmp);
    auto const& uses = state.unit.tuples[phijmp.phijmp_.uses];
    auto& defs = state.unit.tuples[inst.phidef_.defs];
    assertx(uses.size() == defs.size());

    // The uses of the associated phijmp should already be colored, so
    // just use that to initialize the phidef defs.
    auto const& defsSet = defs_set_cached(state, inst);
    for (size_t i = 0; i < uses.size(); ++i) {
      if (is_ignored(state, defs[i])) {
        assertx(is_ignored(state, uses[i]));
        continue;
      }
      assertx(!defs[i].isPhys());
      auto const& info = reg_info(state, defs[i]);
      if (is_spill(info.regClass)) continue;
      if (info.regClass == RegClass::SF) {
        defs[i] = state.abi.sf.choose();
        continue;
      }
      // Rewrite the instruction to take the physical registers.
      assertx(uses[i].isPhys());
      auto const phys = uses[i].physReg();
      assertx(phys != state.scratch);
      reserve(defs[i], phys);
      defs[i] = phys;
    }
    invalidate_cached_operands(inst);
    // The phidef can define registers which are immediately dead, so
    // release those here.
    release_dead_regs(state, free, defsSet, nullptr, b, 0);
  }

  // Since we've split critical edges, if we have multiple
  // predecessors, all those predecessors must have one successor
  // (this block), and therefore the live-out information should
  // always match the live-in.
  if (debug) {
    for (auto const pred : preds) {
      always_assert(state.liveOut[pred] == state.liveIn[b]);
    }
  }

  // We have multiple predecessors. The different predecessors might
  // have different Vreg to physical register assignments. So, instead
  // of just taking it from one predecessor or another, instead we
  // invoke the full "choose" logic for each Vreg, which will attempt
  // to chose the physical register which is optimal (according to
  // penalty vectors). However, we bias the penalty vectors by what
  // the predecessors have already chosen, to reflect that certain
  // choices will avoid copies on the edge. Since these biases are
  // only for this one instruction, we need to undo the adjustments
  // after selection.

  struct Adjustment {
    Adjustment(PenaltyVector* p, PhysReg s, int64_t a)
      : penalties{p}, skip{s}, amount{a} {}
    PenaltyVector* penalties;
    PhysReg skip;
    int64_t amount;
  };
  jit::vector<Adjustment> penaltyAdjustments;

  size_t assignmentIdx = 0;
  for (auto const r : state.liveIn[b]) {
    if (r.isPhys()) continue;

    auto& info = reg_info(state, r);
    if (!is_colorable_reg(info.regClass)) continue;

    assertx(info.penaltyIdx > 0 && info.penaltyIdx < state.penalties.size());
    auto& penalties = state.penalties[info.penaltyIdx];

    // Bias the penalty vector for this Vreg by what processed
    // predecessors have already selected for it. Record the
    // adjustment so we can undo it.
    for (auto const pred : preds) {
      if (!processed[pred]) continue;

      auto const& predAssignment = assignments[pred];
      assertx(assignmentIdx < predAssignment.size());
      assertx(predAssignment[assignmentIdx].first == r);

      // The copy will be done on the predecessor, so that's the
      // weight to use.
      auto const weight = block_weight(state, pred);

      for (auto const phys : penalties) {
        if (phys == predAssignment[assignmentIdx].second) continue;
        penalties[phys] += weight;
      }
      penaltyAdjustments.emplace_back(
        &penalties,
        predAssignment[assignmentIdx].second,
        weight
      );
    }

    ++assignmentIdx;
  }

  // Same as above, but for a phidef/phijmp pair, if any.
  auto& phidef = state.unit.blocks[b].code.front();
  if (phidef.op == Vinstr::phidef) {
    auto const& defs = state.unit.tuples[phidef.phidef_.defs];
    for (size_t i = 0; i < defs.size(); ++i) {
      if (is_ignored(state, defs[i])) continue;
      assertx(!defs[i].isPhys());
      auto& info = reg_info(state, defs[i]);
      if (!is_colorable_reg(info.regClass)) continue;

      assertx(info.penaltyIdx > 0 && info.penaltyIdx < state.penalties.size());
      auto& penalties = state.penalties[info.penaltyIdx];

      for (auto const pred : preds) {
        auto const phijmp = state.unit.blocks[pred].code.back();
        assertx(phijmp.op == Vinstr::phijmp);
        auto const& uses = state.unit.tuples[phijmp.phijmp_.uses];
        assertx(uses.size() == defs.size());

        if (!processed[pred]) continue;

        assertx(uses[i].isPhys());
        assertx(!is_ignored(state, uses[i]));
        auto const physUse = uses[i].physReg();
        auto const weight = block_weight(state, pred);
        for (auto const phys : penalties) {
          if (phys == physUse) continue;
          penalties[phys] += weight;
        }
        penaltyAdjustments.emplace_back(&penalties, physUse, weight);
      }
    }
  }

  // Now that we've biased things appropriately, do the actual
  // selection.
  for (auto const r : state.liveIn[b]) {
    // NB: Physical register always come before virtual ones in a
    // VregSet, so we'll reserve all the physical registers first.
    if (r.isPhys()) {
      reserve(r, r.physReg());
      continue;
    }

    auto& info = reg_info(state, r);
    if (!is_colorable_reg(info.regClass)) continue;

    // Choose a physical register for this Vreg. "Moving" here is
    // free, because it just results in a change to a previous
    // assignment.
    auto const coloring = free.choose(r, [] (Vreg, Vreg) { return 0; });
    assert_found_color(r, coloring.color);

    auto const selected = color_reg(coloring.color);
    if (coloring.moveDest != InvalidReg) {
      auto const vreg = free.assignedTo(selected);
      free.move(selected, coloring.moveDest);
      reg_info(state, vreg).color = coloring.moveDest;
    }

    reserve(r, selected);
  }

  // Do the same, but for phidef/phijmp pair if any.
  if (phidef.op == Vinstr::phidef) {
    auto const& defs = state.unit.tuples[phidef.phidef_.defs];
    for (size_t i = 0; i < defs.size(); ++i) {
      if (is_ignored(state, defs[i])) continue;
      assertx(!defs[i].isPhys());
      auto& info = reg_info(state, defs[i]);
      if (!is_colorable(info.regClass)) continue;
      assertx(is_color_none(info.color));

      auto const coloring = free.choose(defs[i], [] (Vreg, Vreg) { return 0; });
      assert_found_color(defs[i], coloring.color);
      info.color = coloring.color;

      if (coloring.moveDest != InvalidReg) {
        auto const selected = color_reg(info.color);
        auto const vreg = free.assignedTo(selected);
        free.move(selected, coloring.moveDest);
        reg_info(state, vreg).color = coloring.moveDest;
      }

      // We don't use reserve() here because defs[i] might be a spill.
      free.reserve(defs[i], info.color, info.regClass);
    }
  }

  // Now that we've selected registers for all the Vregs, we need to
  // bias any unprocessed predecessors. That is, we want to make it
  // more likely that when coloring the predecessor, we select
  // registers for the same Vregs there, that match our selections
  // here. This will eliminate any necessary copies between the
  // blocks. Assess a penalty for each Vreg to every physical register
  // which isn't the one we selected. The weight of the penalty is the
  // weight of the predecessor. These adjustments will be undone when
  // the predecessor is colored.
  for (auto const r : state.liveIn[b]) {
    if (r.isPhys()) continue;

    auto& info = reg_info(state, r);
    if (!is_colorable_reg(info.regClass)) continue;

    auto const selection = color_reg(info.color);
    for (auto const pred : preds) {
      if (processed[pred]) continue;

      assertx(info.penaltyIdx > 0 && info.penaltyIdx < state.penalties.size());
      auto& penalties = state.penalties[info.penaltyIdx];

      auto const weight = block_weight(state, pred);
      for (auto const phys : penalties) {
        if (phys == selection) continue;
        penalties[phys] += weight;
      }
    }
  }

  // Same idea as above, but for phijmp/phidef pairs. We need to
  // record which operands were adjusted, as its difficult to
  // determine after the fact.
  if (phidef.op == Vinstr::phidef) {
    auto const& defs = state.unit.tuples[phidef.phidef_.defs];
    for (size_t i = 0; i < defs.size(); ++i) {
      if (is_ignored(state, defs[i])) continue;
      assertx(!defs[i].isPhys());
      auto& info = reg_info(state, defs[i]);
      if (!is_colorable_reg(info.regClass)) continue;

      auto const physDef = color_reg(info.color);
      for (auto const pred : preds) {
        if (processed[pred]) continue;
        auto const& phijmp = state.unit.blocks[pred].code.back();
        auto const& uses = state.unit.tuples[phijmp.phijmp_.uses];
        assertx(uses.size() == defs.size());
        assertx(!is_ignored(state, uses[i]));

        auto& adjustments = phiAdjustments[pred];
        assertx(adjustments.empty() || adjustments.size() == uses.size());
        if (adjustments.empty()) adjustments.resize(uses.size());

        auto const penaltyIdx = reg_info(state, uses[i]).penaltyIdx;
        assertx(penaltyIdx > 0 && penaltyIdx < state.penalties.size());
        // If the source and dest don't have the same penalty vector,
        // there's no benefit to assigning them the same physical
        // register (they probably interfere anyways), so skip the
        // adjustment.
        if (penaltyIdx != info.penaltyIdx) continue;

        adjustments[i] = true;

        auto& v = state.penalties[penaltyIdx];
        auto const weight = block_weight(state, pred);
        for (auto const r : v) {
          if (r != physDef) v[r] += weight;
        }
      }
    }
  }

  // Undo the penalty vector adjustments we did early before the
  // selections were made.
  for (auto const p : penaltyAdjustments) {
    auto& penalties = *p.penalties;
    for (auto const r : penalties) {
      if (r == p.skip) continue;
      assertx(penalties[r] >= p.amount);
      penalties[r] -= p.amount;
    }
  }

  // Rewrite the phidef (if present) to take the physical registers
  // selected for it.
  if (phidef.op == Vinstr::phidef) {
    // Capture this before rewriting the operands.
    auto const& defsSet = defs_set_cached(state, phidef);
    auto& defs = state.unit.tuples[phidef.phidef_.defs];
    for (size_t i = 0; i < defs.size(); ++i) {
      if (is_ignored(state, defs[i])) continue;
      assertx(!defs[i].isPhys());
      auto const& info = reg_info(state, defs[i]);
      if (is_spill(info.regClass)) continue;
      if (info.regClass == RegClass::SF) {
        defs[i] = state.abi.sf.choose();
        continue;
      }
      defs[i] = color_reg(info.color);
      assertx(defs[i] != state.scratch);
    }
    invalidate_cached_operands(phidef);
    // The phidef can define registers which are immediately dead, so
    // release those here.
    release_dead_regs(state, free, defsSet, nullptr, b, 0);
    return 1;
  }

  return 0;
}

void assign_colors(State& state) {
  // Calculate (initial) penalty vectors and find a good coloring
  // order.
  calculate_penalties(state);
  auto const order = calculate_block_color_order(state);

  auto& unit = state.unit;

  // Vreg to physical register assignments to the entry and exit of
  // blocks. Used to initialize assignments when starting a block, and
  // for resolving mismatches between blocks.
  jit::vector<AssignmentVector> assignmentsIn;
  jit::vector<AssignmentVector> assignmentsOut;
  assignmentsIn.resize(unit.blocks.size());
  assignmentsOut.resize(unit.blocks.size());

  // When we color a phidef, not all of the phijmp blocks may have
  // been colored already. We adjust the penalty vectors for those
  // uncolored phijmp sources to express a preference for the phidef
  // def assigned registers. However, not all Vregs may be adjusted
  // (because interference may mean we don't try to satisfy the
  // hint). Its difficult to back out what got adjusted and what
  // didn't, so explicitly encode it in this data-structure. The
  // vector is keyed by block, and each bit in the bitset represents
  // an index into the phijmp source.
  jit::vector<boost::dynamic_bitset<>> phiAdjustments;
  phiAdjustments.resize(unit.blocks.size());

  // Populate register assignments from a set of live Vregs.
  auto const liveToAssignment = [&] (const VregSet& live,
                                     AssignmentVector& assignment) {
    assignment.reserve(live.size());
    for (auto const r : live) {
      if (r.isPhys()) continue;
      auto const& info = reg_info(state, r);
      if (!is_colorable_reg(info.regClass)) continue;
      assertx(!is_color_none(info.color));
      assignment.emplace_back(r, color_reg(info.color));
    }
  };

  boost::dynamic_bitset<> processed(state.unit.blocks.size());

  // Since the block order is dominance preserving, we'll always encouter a
  // Vreg's def before any of its usages. This means we can color in a single
  // pass over the unit.
  for (auto const b : order) {
    assertx(!processed[b]);

    // Initialize the initial assignments according to the information
    // from predecessors and phidefs.
    FreeRegs free{state};
    auto const startIdx = color_block_initialize(
      state, free, b, assignmentsOut,
      processed, phiAdjustments
    );

    // Record the initial assignment for mismatch resolution. This is
    // only necessary if the block has multiple predecessors (if
    // there's only one predecessor, it must be already processed and
    // there can be no mismatches).
    if (state.preds[b].size() > 1) {
      liveToAssignment(state.liveIn[b], assignmentsIn[b]);
    } else if (state.preds[b].size() > 0) {
      assertx(processed[state.preds[b][0]]);
    }

    // Walk the instructions, running the coloring logic on them.
    for (size_t i = startIdx; i < unit.blocks[b].code.size(); ++i) {
      auto& inst = unit.blocks[b].code[i];
      // Make sure the uses of the instruction are all colored already (which
      // should be the case because we walk the unit in dominance preserving
      // order).
      if (debug) {
        for (auto const r : uses_set_cached(state, inst)) {
          if (r.isPhys()) continue;
          auto const& info = reg_info(state, r);
          if (!is_colorable(info.regClass)) continue;
          always_assert(!is_color_none(info.color));
        };
      }

      i += color_inst(state, free, inst, b, i, processed, phiAdjustments);
    }

    auto const successors = succs(unit.blocks[b]);
    if (!successors.empty()) {
      // If this block has one successor, but that successor is
      // already processed, it means that successor has multiple
      // predecessors (because we split critical edges), and one of
      // them has a higher weight than us.  When the predecessor was
      // colored, we modified the penalty vectors for live-in Vregs to
      // reflect a preference for whatever they were colored to. Since
      // we've now processed this block, we must now undo these
      // preferences.
      if (successors.size() == 1 && processed[successors[0]]) {
        auto const weight = block_weight(state, b);

        for (auto const& assignment : assignmentsIn[successors[0]]) {
          auto& info = reg_info(state, assignment.first);
          assertx(is_colorable_reg(info.regClass));

          auto const penaltyIdx = info.penaltyIdx;
          assertx(penaltyIdx > 0 && penaltyIdx < state.penalties.size());
          auto& v = state.penalties[penaltyIdx];

          // Undo the preference (really a penalty for everything else
          // except one Vreg).
          for (auto const r : v) {
            if (r != assignment.second) {
              assertx(v[r] >= weight);
              v[r] -= weight;
            }
          }
        }
      }

      // Record assignments at the end of the block for mismatch
      // resolution.
      liveToAssignment(state.liveOut[b], assignmentsOut[b]);
    }

    processed[b] = true;

    // Clear color information from the Vregs in debug builds, to
    // catch uses before assignment.
    if (debug) {
      for (auto const r : state.liveOut[b]) {
        if (r.isPhys()) continue;
        auto& info = reg_info(state, r);
        if (!is_spill(info.regClass)) info.color = Color{};
      }
    }
  }

  // At this point all penalties should be zero, as we should have
  // undone every assessment as we colored the unit.
  if (debug) {
    for (auto const& v : state.penalties) {
      for (auto const r : v) always_assert(v[r] == 0);
    }
  }

  // Now compare the assignment information between predecessors and
  // successors and look for mismatches. This means the blocks made
  // different coloring decisions. If so, we need to insert phis
  // between the blocks to make everything consistent.
  for (auto const b : state.rpo) {
    // If a block only has one predecessor, it should always be
    // consistent, so we can skip.
    auto const& preds = state.preds[b];
    if (preds.size() <= 1) continue;

    // Since we've split critical edges, a block with multiple
    // predecessors should have the same live-in Vregs as all the
    // predecessors' live-out.
    if (debug) {
      for (auto const pred : preds) {
        always_assert(state.liveOut[pred] == state.liveIn[b]);
      }
    }

    auto const& in = assignmentsIn[b];

    VregSet conflicts;
    for (size_t i = 0; i < in.size(); ++i) {
      assertx(!in[i].first.isPhys());
      auto const virt = in[i].first;
      auto const phys = in[i].second;
      for (auto const pred : preds) {
        assertx(assignmentsOut[pred].size() == in.size());
        assertx(assignmentsOut[pred][i].first == virt);
        if (assignmentsOut[pred][i].second != phys) conflicts.add(virt);
      }
    }
    // Hopefully the common case
    if (conflicts.none()) continue;

    // Either create a new phidef/phijmp or expand an existing one.
    auto const addToPhiDef = [&] (PhysReg reg) {
      auto& first = unit.blocks[b].code.front();
      if (first.op == Vinstr::phidef) {
        unit.tuples[first.phidef_.defs].push_back(reg);
        invalidate_cached_operands(first);
      } else {
        vmodify(
          unit, b, 0,
          [&] (Vout& v) { v << phidef{unit.makeTuple({reg})}; return 0; }
        );
      }
    };

    auto const addToPhiJmp = [&] (Vlabel pred, PhysReg reg) {
      auto& last = unit.blocks[pred].code.back();
      if (last.op == Vinstr::phijmp) {
        unit.tuples[last.phijmp_.uses].push_back(reg);
      } else {
        assertx(last.op == Vinstr::jmp);
        auto const jmp = last.jmp_;
        assertx(jmp.target == b);
        last.op = Vinstr::phijmp;
        last.phijmp_ = phijmp{jmp.target, unit.makeTuple({reg})};
      }
      invalidate_cached_operands(last);
    };

    // Resolve the mismatches with phis
    for (size_t i = 0; i < in.size(); ++i) {
      if (!conflicts[in[i].first]) continue;
      addToPhiDef(in[i].second);
      for (auto const pred : preds) {
        addToPhiJmp(pred, assignmentsOut[pred][i].second);
      }
    }
  }

  assertx(check(state.unit));
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
      if (inst.copy_.s == inst.copy_.d) {
        vmodify(state.unit, block, instIdx, [] (Vout&) { return 1; });
        return;
      } else if (inst.copy_.s.isPhys() && inst.copy_.d.isPhys()) {
        // Single argument copies of physical registers don't need
        // lowering.
        return;
      } else {
        addSrcDstPair(inst.copy_.s, inst.copy_.d);
        break;
      }
    case Vinstr::copyargs: {
      auto const& srcs = state.unit.tuples[inst.copyargs_.s];
      auto const& dsts = state.unit.tuples[inst.copyargs_.d];
      assertx(srcs.size() == dsts.size());
      auto noop = true;
      for (size_t i = 0; i < srcs.size(); ++i) {
        if (srcs[i] == dsts[i]) continue;
        noop = false;
        addSrcDstPair(srcs[i], dsts[i]);
      }
      if (noop) {
        // Trivial copy, just remove it
        vmodify(state.unit, block, instIdx, [] (Vout&) { return 1; });
        return;
      }
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
      auto noop = true;
      for (size_t i = 0; i < srcs.size(); ++i) {
        if (srcs[i] == dsts[i]) continue;
        noop = false;
        addSrcDstPair(srcs[i], dsts[i]);
      }
      phiTarget = inst.phijmp_.target;
      if (noop) {
        // Trivial phi, just turn the phijmp into a normal jmp.
        vmodify(
          state.unit, block, instIdx,
          [&] (Vout& v) {
            v << jmp{phiTarget};
            return 1;
          }
        );
        return;
      }
      break;
    }
    default:
      always_assert(false);
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
    invalidate_cached_operands(inst);

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
    invalidate_cached_operands(inst);

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
        invalidate_cached_operands(inst);
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
    optimizeExits(state.unit, 0);
    optimizeJmps(state.unit, 0);
    compute_rpo(state);
  }

  // Do a second stack pointer offset calculation which will check if the
  // adjustment code kept the stack pointer in a consistent state.
  if (debug) calculate_sp_offsets(state);
}

void lower_ssa(State& state) {
  auto& unit = state.unit;

  /*
   * First pass:
   *
   * Suppose we have:
   *
   * B1: phijmp R1 -> B4
   * B2: phijmp R1 -> B4
   * B3: phijmp R1 -> B4
   * B4: phidef R2
   *
   * Rewrite this to:
   *
   * .....
   * B4: phidef R1
   *     copy R1 -> R2
   *
   * That is, if a phi has all the same register at the phijmps, but a
   * different register at the phidef, then change the phidef to the
   * register at the phijmps, and then copy to the actual dest. This
   * lets us make the phi trivial, and emit a single copy at the
   * def. Otherwise we'll emit a copy at every jmp, which increases
   * code size.
   */
  for (auto const b : state.rpo) {
    auto const& preds = state.preds[b];
    // Not an issue if we only have a single pred
    if (preds.size() < 2) continue;

    auto& phidef = unit.blocks[b].code.front();
    if (phidef.op != Vinstr::phidef) continue;

    auto& defs = unit.tuples[phidef.phidef_.defs];

    VregList copySrcs;
    VregList copyDsts;
    for (size_t i = 0; i < defs.size(); ++i) {
      // Any Vreg which isn't a physical register at this point should
      // be a spill.
      if (!defs[i].isPhys()) continue;

      // Iterate over the phijmps, looking for the same register.
      PhysReg same;
      for (auto const pred : preds) {
        auto const& phijmp = unit.blocks[pred].code.back();
        assertx(phijmp.op == Vinstr::phijmp);
        auto const& uses = unit.tuples[phijmp.phijmp_.uses];
        assertx(uses.size() == defs.size());

        assertx(uses[i].isPhys());
        if (uses[i] == defs[i]) {
          same = InvalidReg;
          break;
        }

        auto const phys = uses[i].physReg();
        if (same == InvalidReg) {
          same = phys;
        } else if (same != phys) {
          same = InvalidReg;
          break;
        }
      }

      if (same == InvalidReg) continue;
      // This optimization is invalid if the def writes to the same
      // register the jmps use.
      if (defs_set_cached(state, phidef)[same]) continue;

      copySrcs.emplace_back(same);
      copyDsts.emplace_back(defs[i]);
      defs[i] = same;
      invalidate_cached_operands(phidef);
    }

    assertx(copySrcs.size() == copyDsts.size());
    if (copySrcs.size() == 0) continue;

    vmodify(
      unit, b, 1,
      [&] (Vout& v) {
        if (copySrcs.size() == 1) {
          v << copy{copySrcs[0], copyDsts[0]};
        } else {
          v << copyargs{
            unit.makeTuple(std::move(copySrcs)),
            unit.makeTuple(std::move(copyDsts))
          };
        }
        return 0;
      }
    );
  }

  // Second pass: lower copy-ish instructions (including phis) to copy and copy2
  // instructions, and optimize them away if they're no-ops.
  for (auto const b : state.rpo) {
    for (auto i = unit.blocks[b].code.size(); i > 0; --i) {
      auto const& inst = unit.blocks[b].code[i - 1];
      switch (inst.op) {
        case Vinstr::copy:
        case Vinstr::copy2:
        case Vinstr::copyargs:
        case Vinstr::phijmp:
          lower_copies(state, b, i - 1);
          break;
        default:
          break;
      }
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
  assign_colors(state);
  lower_ssa(state);

  printUnit(kVasmRegAllocLevel, "after vasm-graph-color", unit);
}

//////////////////////////////////////////////////////////////////////

}}
