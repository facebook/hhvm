/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <string>
#include <algorithm>
#include <array>

#include <folly/Format.h>

#include "hphp/util/trace.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

namespace HPHP { namespace jit {

namespace {

TRACE_SET_MOD(vasm_copy);

//////////////////////////////////////////////////////////////////////

using RpoID = uint32_t;

constexpr auto kNumPhysRegs = 2;

folly::Optional<int32_t> phys_reg_index(Vreg reg) {
  assert(reg.isPhys());
  if (reg == rvmfp()) return 0;
  if (reg == rvmtl()) return 1;
  return folly::none;
}

//////////////////////////////////////////////////////////////////////

/*
 * Information about the definition of a virtual register, if it was defined by
 * either copying or lea'ing off another register.  The `base' register is not
 * necessarily a virtual register.
 */
struct RegInfo { Vreg base; int32_t disp; };

DEBUG_ONLY std::string show(RegInfo x) {
  return folly::sformat("{} + {}", show(x.base), x.disp);
}

// Dataflow state.  There is one of these for each block entry.
struct State {
  bool initialized{false};
  /*
   * We have a bit of state for each physical register this pass tracks.  If
   * the physical register has never been redefined, we will be safe rewriting
   * uses of other Vregs that we know were defined in terms of it.  But if it
   * may have been altered, we can't rewrite.
   */
  std::array<bool,kNumPhysRegs> phys_altered;
};

struct BlockInfo {
  RpoID rpoID;
  State stateIn;
};

struct Env {
  explicit Env(Vunit& unit, const Abi& abi)
    : unit(unit)
    , abi(abi)
    , rpoBlocks(sortBlocks(unit))
    , blockInfos(unit.blocks.size())
    , regs(unit.next_vr)
  {
    auto rpoID = uint32_t{0};
    for (auto& b : rpoBlocks) blockInfos[b].rpoID = rpoID++;
  }

  Vunit& unit;
  const Abi& abi;
  jit::vector<Vlabel> rpoBlocks;

  // Keyed by Vlabel.  This is populated by the flow-sensitive
  // analyze_physical pass.
  jit::vector<BlockInfo> blockInfos;

  // Keyed by Vreg.  This is populated as we go through the optimize pass, and
  // doesn't need to be flow-sensitive since it only contains a RegInfo for
  // virtual Vregs.  The information in the RegInfo is "usable" if it is also
  // only virtual (and therefore SSA), or if it makes references to physical
  // registers that may not have been altered (which is flow-sensitive
  // information in blockInfos).
  jit::vector<RegInfo> regs;
};

//////////////////////////////////////////////////////////////////////

State entry_state(Env& env) {
  auto ret = State{};
  ret.initialized = true;
  ret.phys_altered.fill(0);
  return ret;
}

bool merge_into(State& dst, const State& src) {
  if (!dst.initialized) {
    dst = src;
    return true;
  }

  auto changed = false;

  for (auto i = uint32_t{0}; i < kNumPhysRegs; ++i) {
    auto const new_alt = dst.phys_altered[i] || src.phys_altered[i];
    if (new_alt != dst.phys_altered[i]) {
      dst.phys_altered[i] = new_alt;
      changed = true;
    }
  }

  return changed;
}

//////////////////////////////////////////////////////////////////////

template<class F>
void for_all_defs(Env& env, const Vinstr& inst, F f) {
  visitDefs(env.unit, inst, f);
  auto uses   = RegSet{};
  auto across = RegSet{};
  auto defs   = RegSet{};
  getEffects(env.abi, inst, uses, across, defs);
  defs.forEach(f);
}

void analyze_inst_physical(Env& env,
                           State& state,
                           Vinstr& inst,
                           Vinstr* next) {
  for_all_defs(env, inst, [&] (Vreg dst) {
    if (!dst.isPhys()) return;
    if (auto const idx = phys_reg_index(dst)) {
      /*
       * A common pattern for us is to load an address into the frame pointer
       * right before a PHP call.  In this case, if the frame pointer was not
       * altered before this redefinition, it will effectively still be
       * not-altered after the call, because callphp{} restores it to the
       * previous value.
       *
       * We don't need to worry about not setting the altered flag in between
       * this instruction and the callphp{}, because callphp{}'s uses are only
       * of a RegSet---we cannot mis-optimize any of its args based on the
       * state we're tracking for the frame pointer.
       *
       * We also skip over callphp{}'s definition of rvmfp() for this reason.
       * Really callphp{} only preserves rvmfp() if we properly set up the
       * rvmfp() arg to it, but the program is ill-formed if it's not doing
       * that so it's ok to just ignore that definition here.
       */
      if (next && next->op == Vinstr::callphp && dst == rvmfp()) {
        FTRACE(3, "      post-dominated by callphp---preserving frame ptr\n");
        return;
      }
      if (inst.op == Vinstr::callphp && dst == rvmfp()) return;

      FTRACE(3, "      kill {}\n", show(dst));
      state.phys_altered[*idx] = true;
    }
  });
}

//////////////////////////////////////////////////////////////////////

void analyze_physical(Env& env) {
  FTRACE(1, "analyze_physical ---------------------------------\n");

  env.blockInfos[env.unit.entry].stateIn = entry_state(env);
  auto workQ = dataflow_worklist<RpoID>(env.unit.blocks.size());
  workQ.push(RpoID{0});
  do {
    auto const label = env.rpoBlocks[workQ.pop()];
    FTRACE(1, "{}:\n", label);
    auto& blk = env.unit.blocks[label];
    auto& binfo = env.blockInfos[label];

    auto state = binfo.stateIn;
    for (auto it = begin(blk.code); it != end(blk.code); ++it) {
      auto& inst = *it;
      FTRACE(2, "    {}\n", show(env.unit, inst));
      auto const next_it = std::next(it);
      auto const next_inst = next_it != end(blk.code) ? &*next_it : nullptr;
      analyze_inst_physical(env, state, inst, next_inst);
    }

    for (auto s : succs(blk.code.back())) {
      FTRACE(4, "  -> {}\n", s);
      auto& sinfo = env.blockInfos[s];
      if (merge_into(sinfo.stateIn, state)) workQ.push(sinfo.rpoID);
    }
  } while (!workQ.empty());
}

DEBUG_ONLY std::string show_fixed_point(Env& env) {
  auto ret = std::string{};

  for (auto& b : env.rpoBlocks) {
    auto const& state = env.blockInfos[b].stateIn;
    folly::format(&ret, "{: <4}:  ", b);
    for (auto& v : state.phys_altered) folly::format(&ret, " {}", v);
    ret += "\n";
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

void analyze_copy(Env& env, const copy& copy) {
  if (!copy.d.isVirt()) return;
  auto& dst = env.regs[copy.d];
  dst = RegInfo { copy.s, 0 };
  FTRACE(3, "      {} = {}\n", show(copy.d), show(dst));
}

void analyze_lea(Env& env, const lea& lea) {
  if (!(lea.s.seg == Vptr::DS &&
        lea.s.index == InvalidReg &&
        lea.d.isVirt())) {
    return;
  }
  auto& dst = env.regs[lea.d];
  dst = RegInfo { lea.s.base, lea.s.disp };
  FTRACE(3, "      {} = {}\n", show(lea.d), show(dst));
}

void analyze_inst_virtual(Env& env, const Vinstr& inst) {
  switch (inst.op) {
  case Vinstr::copy: return analyze_copy(env, inst.copy_);
  case Vinstr::lea:  return analyze_lea(env, inst.lea_);
  default: break;
  }
}

//////////////////////////////////////////////////////////////////////

template<class F>
void if_tracking_reg(const Env& env, const State& state, Vreg reg, F f) {
  if (!reg.isValid()) return;
  auto& info = env.regs[reg];
  if (!info.base.isValid()) return;
  if (info.base.isPhys()) {
    auto const index = phys_reg_index(info.base);
    if (!index) return;
    if (state.phys_altered[*index]) {
      FTRACE(4, "        base may be altered\n");
      return;
    }
  }
  f(info);
}

struct OptVisit {
  const Env& env;
  const State& state;

  template<class T> void imm(T&) {}
  template<class T> void across(T& t) { use(t); }
  template<class T, class H> void useHint(T& t, H&) { use(t); }
  template<class T, class H> void defHint(T& t, H&) { def(t); }
  template<class T> void def(T&) {}

  void use(RegSet) {}
  void use(VregSF) {}
  void use(VcallArgsId) {}
  void use(Vreg128) {}

  void use(Vtuple t) { for (auto& reg : env.unit.tuples[t]) use(reg); }

  void use(Vptr& ptr) {
    // Rewrite memory operands that are based on registers we've copied or
    // lea'd off of other registers.
    if (ptr.seg != Vptr::DS) return;
    if_tracking_reg(env, state, ptr.base, [&] (const RegInfo& info) {
      FTRACE(2, "      rewrite: {} => {}\n", show(ptr.base), show(info));
      ptr.base = info.base;
      ptr.disp += info.disp;
    });
  }

  template<class T>
  typename std::enable_if<
    std::is_same<Vreg,T>::value ||
      std::is_same<Vreg8,T>::value ||
      std::is_same<Vreg16,T>::value ||
      std::is_same<Vreg32,T>::value ||
      std::is_same<Vreg64,T>::value ||
      std::is_same<VregDbl,T>::value
  >::type use(T& reg) {
    // Rewrite to another register if it's just a copy.
    if_tracking_reg(env, state, reg, [&] (const RegInfo& info) {
      if (info.disp != 0) return;
      FTRACE(2, "      rewrite: {} => {}\n", show(reg), show(info.base));
      reg = info.base;
    });
  }
};

void optimize_copy(const Env& env, const State& state, Vinstr& inst) {
  auto& copy = inst.copy_;
  if_tracking_reg(env, state, copy.s, [&] (const RegInfo& info) {
    if (info.disp != 0) {
      FTRACE(2, "      copy => lea {}\n", show(info));
      inst = lea{info.base[info.disp], copy.d};
    }
  });
}

void optimize_inst(const Env& env, const State& state, Vinstr& inst) {
  auto visit = OptVisit { env, state };
  visitOperands(inst, visit);
  switch (inst.op) {
  case Vinstr::copy:  optimize_copy(env, state, inst); break;
  default:            break;
  }
}

void optimize(Env& env) {
  FTRACE(1, "optimize ---------------------------------\n");

  for (auto const& label : env.rpoBlocks) {
    FTRACE(1, "{}:\n", label);
    auto& blk = env.unit.blocks[label];
    auto& binfo = env.blockInfos[label];

    auto state = binfo.stateIn;
    for (auto it = begin(blk.code); it != end(blk.code); ++it) {
      auto& inst = *it;
      FTRACE(2, "    {}\n", show(env.unit, inst));
      optimize_inst(env, state, inst);
      analyze_inst_virtual(env, inst);
      auto const next_it = std::next(it);
      auto const next_inst = next_it != end(blk.code) ? &*next_it : nullptr;
      analyze_inst_physical(env, state, inst, next_inst);
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

/*
 * This pass performs straight-forward copy propagation, along with stateful
 * copy propagation of values through physical registers.  (Tracking the values
 * of physical registers requires dataflow analysis, because they do not have
 * single definitions.)
 *
 * The pass also tracks registers defined via lea instructions, and it knows
 * when a register holds a value that is the same as another register plus some
 * offset.  It then folds offsets in memory operands to try to require fewer
 * registers.  The main motivation for this is to generally eliminate the need
 * for a separate stack pointer (the result of HHIR's DefSP instruction, which
 * will just be an lea off of the rvmfp() physical register).
 */
void optimizeCopies(Vunit& unit, const Abi& abi) {
  VpassTracer tracer{&unit, Trace::vasm_copy, "vasm-copy"};
  Env env { unit, abi };
  analyze_physical(env);
  FTRACE(5, "\nfixed point:\n{}\n", show_fixed_point(env));
  optimize(env);
}

}}
