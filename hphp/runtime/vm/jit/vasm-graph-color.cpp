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
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/ppc64-asm/asm-ppc64.h"

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

// State about each Vreg. Instead of separate data-structures, all Vreg
// information is concentrated in this one data-structure.
struct RegInfo {
  // The physical register this was before it was SSA-ized. The allocator will
  // ensure that this Vreg will be assigned the same physical register at all
  // uses and defs (except copies).
  PhysReg precolor = InvalidReg;
};

using BlockVector = jit::vector<Vlabel>;

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
  const VregSet reservedRegs;
  // SIMD scratch register for resolving shuffles.
  const PhysReg scratch;

  // All the instructions which have been converted to
  // pseudo-instructions. Those pseudo-instructions have the index into this
  // vector stored in their "pos" field to map back.
  jit::vector<Vinstr> pseudos;

  // Vreg state
  jit::vector<folly::Optional<RegInfo>> regInfo;
};

//////////////////////////////////////////////////////////////////////
// Pretty-printers

std::string show(const BlockVector& v) {
  using namespace folly::gen;
  return folly::sformat(
    "[{}]",
    from(v)
      | map([] (Vlabel b) { return folly::sformat("{}", b); })
      | unsplit<std::string>(", ")
  );
}

std::string show(const Vunit& unit, const RegInfo& info) {
  if (info.precolor != InvalidReg) {
    return folly::sformat("Pre-Color: {}", show(info.precolor));
  } else {
    return std::string{"Pre-Color: -"};
  }
}

std::string show(const State& state) {
  return folly::sformat(
    "GP Unreserved:        {}\n"
    "SIMD Unreserved:      {}\n"
    "Reserved Regs:        {}\n"
    "Scratch Reg:          {}\n"
    "RPO:                  {}\n"
    "Reg Info:\n{}",
    show(state.gpUnreserved),
    show(state.simdUnreserved),
    show(state.reservedRegs),
    show(state.scratch),
    show(state.rpo),
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

//////////////////////////////////////////////////////////////////////
// Initial state creation

void compute_rpo(State& state) {
  auto rpo = sortBlocks(state.unit);
  jit::vector<size_t> rpoOrder(state.unit.blocks.size());
  for (size_t i = 0; i < rpo.size(); ++i) rpoOrder[rpo[i]] = i;
  state.rpo = std::move(rpo);
  state.rpoOrder = std::move(rpoOrder);
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
    VregSet{(abi.all() - (abi.gpUnreserved | abi.simdUnreserved)) | scratch},
    scratch
  };

  compute_rpo(state);
  return state;
}

//////////////////////////////////////////////////////////////////////
// Pre-allocation preparation

/*
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

  auto const mappings = restoreSSA(state.unit, unreserved, state.rpo);
  for (auto const& map : mappings) {
    if (!map.second.isPhys()) continue;
    // Remember what physical register these new Vregs were
    reg_info_create(state, map.first).precolor = map.second.physReg();
  }
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

  printUnit(kVasmRegAllocLevel, "after vasm-graph-color", unit);

  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

}}
