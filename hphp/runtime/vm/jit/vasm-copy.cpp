/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/dataflow-worklist.h"

#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <folly/Format.h>

namespace HPHP { namespace jit {

TRACE_SET_MOD(vasm);

namespace {

/*
 * Convert lea to copy when possible, exposing more opportunities for copy
 * propagation. Returns true iff the unit was modified.
 */
bool reduceLeas(Vunit& unit, jit::vector<Vlabel>& blocks) {
  bool changed = false;
  for (auto label : blocks) {
    for (auto& inst : unit.blocks[label].code) {
      if (inst.op != Vinstr::lea) continue;

      auto& lea = inst.lea_;
      if (lea.s.disp == 0 && lea.s.base.isValid() &&
          !lea.s.index.isValid() && lea.s.seg == Vptr::DS) {
        changed = true;
        inst = copy{lea.s.base, lea.d};
      }
    }
  }
  return changed;
}

/*
 * RegInfo represents what we know about the contents of a PhysReg, and can be
 * viewed as a lattice. folly::none is equivalent to Bottom (no value), a valid
 * Vreg represents a single known value, and an invalid Vreg represents Top
 * (unknown value). Values are merged with the union operator, as usual.
 *
 * All blocks start out with their input states set to Bottom, with states
 * merged in from predecessors as they become available. Note that the entry
 * block has an implicit predecessor with a Top out state, which is handled
 * while setting up the initial state for the optimization.
 */
using RegInfo = folly::Optional<Vreg>;

/*
 * Merge src into dst, returning true iff dst was modified. src must be
 * initialized.
 */
bool mergeInfo(RegInfo& dst, const RegInfo src) {
  assert(src);

  // Uninitialized dst: always take src's value.
  if (!dst) {
    dst = src;
    return true;
  }

  // Unknown dst: no change no matter what src is.
  if (!dst->isValid()) return false;

  // Known dst but different src: degrade to unknown.
  if (*dst != *src) {
    dst = Vreg{};
    return true;
  }

  return false;
}

/*
 * This optimization was written and tested to work with all physical
 * registers. However, there is only one that really matters in practice:
 * rVmSp. State only cares about this register, drastically reducing both the
 * time and memory used by the optimization.
 */
struct State {
  static bool isSupported(Vreg reg) {
    return reg == kAllowedReg;
  }

  void setUnknown() {
    m_info = Vreg{};
  }

  void set(PhysReg r, Vreg info) {
    assert(r == kAllowedReg);
    assert(!info.isPhys());
    m_info = info;
  }

  Vreg get(PhysReg r) const {
    assert(r == kAllowedReg);
    assert(m_info);
    return *m_info;
  }

  bool merge(State other) {
    return mergeInfo(m_info, other.m_info);
  }

  template<typename L>
  void forEachKnown(L body) const {
    assert(m_info);
    if (m_info->isValid()) {
      body(kAllowedReg, *m_info);
    }
  }

 private:
  static constexpr PhysReg kAllowedReg = x64::rVmSp;

  RegInfo m_info;
};
constexpr PhysReg State::kAllowedReg;

std::string show(const State& state) {
  const char* sep = "";
  std::string ret = "[";

  state.forEachKnown([&](PhysReg phys, Vreg virt) {
    if (!virt.isValid()) return;
    folly::format(&ret, "{}{}: {}", sep, show(phys), show(virt));
    sep = ", ";
  });

  return ret + "]";
}

/*
 * Env holds global state shared between different parts of the optimization.
 */
struct Env {
  Env(Vunit& unit, const Abi& abi)
    : unit(unit)
    , abi(abi)
    , rpoBlocks(sortBlocks(unit))
    , rpoIds(unit.blocks.size())
    , inStates(unit.blocks.size())
    , vregVals(unit.next_vr)
  {
    for (uint32_t i = 0; i < rpoBlocks.size(); ++i) {
      rpoIds[rpoBlocks[i]] = i;
    }
  }

  Vunit& unit;
  const Abi& abi;

  // Reachable blocks in RPO.
  jit::vector<Vlabel> rpoBlocks;

  // Keyed by Vlabel.
  jit::vector<uint32_t> rpoIds;
  jit::vector<State> inStates;

  // Results of Vreg => Vreg copies, keyed by Vreg.
  jit::vector<Vreg> vregVals;

  // Did we mutate the Vunit?
  bool changed{false};
};

/*
 * VregSets maps from Vreg id to a RegSet of PhysRegs that currently contain
 * that Vreg's value. Used by UseVisitor to efficiently replace uses of a Vreg
 * with a PhysReg holding the same value.
 */
using VregSets = jit::hash_map<size_t, RegSet>;

/*
 * UseVisitor visits all srcs of an instruction, replacing operands in two
 * cases:
 *   - If virtual %x was defined as a copy of virtual %y, replace uses of %x
 *     with uses of %y. This unblocks some copy hinting in vxls.
 *   - If physical %p contains virtual %x, replace uses of %x with uses of
 *     %p. This eliminates uses of %x that would conflict with %p, allowing %x
 *     to be assigned to %p in many situations.
 */
struct UseVisitor {
  UseVisitor(const Vinstr& inst,
             Env& env,
             const VregSets& vregSets)
    : m_env(env)
    , m_changed(env.changed)
    , m_vregSets(vregSets)
    , m_inst(inst)
  {}

  /* Ignore imm and defs. */
  template<typename T> void imm(T) {}
  template<typename T> void def(T) {}
  template<typename T, typename U> void defHint(T, U) {}

  template<typename R> void use(R& r) { replace(r); }
  void use(Vptr& ptr) {
    if (ptr.base.isValid()) replace(ptr.base);
    if (ptr.index.isValid()) replace(ptr.index);
  }
  void use(Vtuple t) {
    for (auto& reg : m_env.unit.tuples[t]) replace(reg);
  }
  void use(VcallArgsId) { always_assert(false); }
  void use(RegSet reg) {
    // RegSets can't have Vregs in them. Nothing to do.
  }
  template<typename R, typename H> void useHint(R& reg, H&) { use(reg); }

  template<typename R> void across(R& reg) {
    // Only try to rename uses if the dest isn't physical, to avoid adding
    // conflicts we can't resolve.
    bool physDest = false;
    visitDefs(m_env.unit, m_inst, [&](Vreg r) {
      if (r.isPhys()) physDest = true;
    });

    if (!physDest) replace(reg);
  }

 private:
  template<typename R> void replace(R& reg) {
    // If we have an SSA substitute for reg, use that first.
    if (m_env.vregVals[reg].isValid()) {
      ITRACE(2, "Replacing use of {} with {} in `{}'\n",
             show(reg), show(m_env.vregVals[reg]), show(m_env.unit, m_inst));
      reg = m_env.vregVals[reg];
      m_changed = true;
    }

    // Next, check if reg is currently in any PhysRegs, and replace it with a
    // use of that PhysReg if so.
    auto const it = m_vregSets.find(reg);
    if (it == m_vregSets.end()) return;
    auto const newReg = it->second.findFirst();
    if (newReg != InvalidReg) {
      ITRACE(2, "Replacing use of {} with {} in `{}'\n",
             show(reg), show(newReg), show(m_env.unit, m_inst));
      reg = newReg;
      m_changed = true;
    }
  }

  const Env& m_env;
  bool& m_changed;
  const VregSets& m_vregSets;
  const Vinstr& m_inst;
};

/*
 * Visit each dest of the given instruction, updating tracked state as
 * appropriate. vregSets is only useful during the mutation phase, so it will
 * be nullptr during analysis. In order to process all sources of copy2 and
 * copyargs before any definitions become visible, inState is never
 * modified. Instead, changes are made to a copy in outState, which is returned
 * to the caller.
 */
State processDests(Env& env, const State inState,
                   VregSets* vregSets, const Vinstr& inst) {
  auto outState = inState;

  auto handleCopy = [&](Vreg dst, Vreg src, bool allowSwap) {
    assert(dst.isValid());

    if (dst.isVirt() && src.isVirt()) {
      // First, the easy case: defining a Vreg from another Vreg. Remember the
      // assignment in the flow-insensitive vregVals map.
      if (env.vregVals[src].isValid()) src = env.vregVals[src];
      assert(!env.vregVals[src].isValid());
      env.vregVals[dst] = src;
      return;
    }

    // Next, see if we're copying a Vreg to a PhysReg, and remember that the
    // PhysReg contains the Vreg if so.
    if (allowSwap && State::isSupported(src) && dst.isVirt()) {
      // Defining a Vreg from a PhysReg gives us the same information: the
      // PhysReg contains the Vreg's value. It's only ok to take this path if
      // the src we're copying from doesn't also appear as a dest of this
      // instruction.
      std::swap(src, dst);
    }
    if (State::isSupported(dst)) {
      if (src.isPhys()) {
        // If the src is another PhysReg, use its tracked value instead, even
        // if it's unknown.
        src = inState.get(src);
      } else if (src.isVirt() && env.vregVals[src].isValid()) {
        // Make sure we use canonical Vregs.
        src = env.vregVals[src];
      }

      auto const prevReg = inState.get(dst);
      // Forget that the previous value lives in dst.
      if (vregSets && prevReg.isValid()) (*vregSets)[prevReg].remove(dst);

      // Register that src lives in dst. src may still be invalid here.
      outState.set(dst, src);
      if (vregSets && src.isValid()) (*vregSets)[src].add(dst);
    }
  };

  if (inst.op == Vinstr::copy) {
    handleCopy(inst.copy_.d, inst.copy_.s, true);
  } else if (inst.op == Vinstr::copy2) {
    auto const& copy2 = inst.copy2_;
    handleCopy(copy2.d0, copy2.s0, copy2.s0 != copy2.d1);
    handleCopy(copy2.d1, copy2.s1, copy2.s1 != copy2.d0);
  } else if (inst.op == Vinstr::copyargs) {
    auto const& srcs = env.unit.tuples[inst.copyargs_.s];
    auto const& dsts = env.unit.tuples[inst.copyargs_.d];
    RegSet dstSet;
    for (auto dst : dsts) if (dst.isPhys()) dstSet.add(dst);

    for (auto i = 0; i < srcs.size(); ++i) {
      handleCopy(dsts[i], srcs[i],
                 srcs[i].isPhys() && !dstSet.contains(srcs[i]));
    }
  } else {
    auto killReg = [&](Vreg dst) { handleCopy(dst, Vreg{}, false); };
    visitDefs(env.unit, inst, killReg);

    RegSet uses, across, defs;
    getEffects(env.abi, inst, uses, across, defs);
    defs.forEach(killReg);
  }

  return outState;
}

} // namespace

void optimizeCopies(Vunit& unit, const Abi& abi) {
  FTRACE(2, "\n{:-^80}\n", " vasm-copy ");
  Env env{unit, abi};
  env.changed = reduceLeas(unit, env.rpoBlocks);

  ITRACE(2, "\n{:-^60}\n", " analysis ");

  // First, iterate over the CFG until we reach a fixed point. Give the entry
  // block an unknown in state, push it on the worklist, and go.
  env.inStates[unit.entry].setUnknown();
  dataflow_worklist<uint32_t> worklist(unit.blocks.size());
  worklist.push(0);

  while (!worklist.empty()) {
    auto const label = env.rpoBlocks[worklist.pop()];
    auto& block      = env.unit.blocks[label];
    auto state       = env.inStates[label];
    ITRACE(2, "Entering {} with state: {}\n", label, show(state));
    Trace::Indent indent;

    for (auto& inst : block.code) {
      state = processDests(env, state, nullptr, inst);
    }

    auto const& block_succs = succs(block.code.back());
    if (!block_succs.empty()) {
      ITRACE(2, "Leaving {} with state: {}\n", label, show(state));

      // Merge our out state into all successors. If this changes any of their
      // in states, enqueue them for processing.
      for (auto succ : block_succs) {
        if (env.inStates[succ].merge(state)) {
          if (worklist.push(env.rpoIds[succ])) {
            ITRACE(3, "Enqueued successor {}\n", succ);
          } else {
            ITRACE(3, "Successor {} already in worklist\n", succ);
          }
        }
      }
    } else {
      ITRACE(2, "Leaving terminal {}\n", label);
    }
    FTRACE(2, "\n");
  }

  ITRACE(2, "\n{:-^60}\n", " mutation ");
  // Now do a single pass over the blocks, mutating uses this time.
  VregSets vregSets;
  for (auto b : env.rpoBlocks) {
    auto state = env.inStates[b];
    ITRACE(2, "Entering {} with state {}\n", b, show(state));
    Trace::Indent indent;

    // Build up a map to efficiently check if any Vreg is in a Physreg.
    vregSets.clear();
    state.forEachKnown([&](PhysReg phys, Vreg virt) {
      vregSets[virt].add(phys);
    });

    for (auto& inst : unit.blocks[b].code) {
      UseVisitor use{inst, env, vregSets};
      visitOperands(inst, use);
      state = processDests(env, state, &vregSets, inst);
    }
  }

  if (env.changed) {
    printUnit(kVasmCopyPropLevel, "after vasm-copy", unit);
  }
}

}}
