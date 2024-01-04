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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

#include <boost/dynamic_bitset.hpp>

#include <algorithm>

namespace HPHP::jit {
///////////////////////////////////////////////////////////////////////////////

folly::Range<Vlabel*> succs(Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::contenter:    return {inst.contenter_.targets, 2};
    case Vinstr::jcc:          return {inst.jcc_.targets, 2};
    case Vinstr::interceptjcc: return {inst.interceptjcc_.targets, 2};
    case Vinstr::jmp:          return {&inst.jmp_.target, 1};
    case Vinstr::phijmp:       return {&inst.phijmp_.target, 1};
    case Vinstr::unwind:       return {inst.unwind_.targets, 2};
    case Vinstr::vinvoke:      return {inst.vinvoke_.targets, 2};
    default:                   return {nullptr, nullptr};
  }
}

folly::Range<Vlabel*> succs(Vblock& block) {
  if (block.code.empty()) return {nullptr, nullptr};
  return succs(block.code.back());
}

folly::Range<const Vlabel*> succs(const Vinstr& inst) {
  return succs(const_cast<Vinstr&>(inst)).castToConst();
}

folly::Range<const Vlabel*> succs(const Vblock& block) {
  return succs(const_cast<Vblock&>(block)).castToConst();
}

boost::dynamic_bitset<> backedgeTargets(const Vunit& unit,
                                        const jit::vector<Vlabel>& rpoBlocks) {
  boost::dynamic_bitset<> ret(unit.blocks.size());
  boost::dynamic_bitset<> seen(unit.blocks.size());

  for (auto label : rpoBlocks) {
    seen.set(label);
    for (auto target : succs(unit.blocks[label])) {
      if (seen.test(target)) ret.set(target);
    }
  }

  return ret;
}

/*
 * Rewrite any references to livefp to rvmfp(), and any Vloc references to
 * livefp or its descendants (per regchain) to be relative to rvmfp() instead.
 *
 * livepf is the SSA register currently in rmvfp() and regchain contains the
 * SSA parent FP registers of each SSA FP register.
 */
struct FpVisit {
  Vunit& unit;
  Vreg livefp;
  const jit::fast_map<size_t, std::pair<Vreg,int32_t>>& regchain;

  template<class T> void imm(T&) {}
  template<class T> void across(T& t) { use(t); }
  template<class T, class H> void useHint(T& t, H&) { use(t); }
  template<class T, class H> void defHint(T& t, H&) { def(t); }
  template<class T> void def(T&) {}

  void use(RegSet) {}
  void use(VregSF) {}
  void use(Vreg128) {}

  void use(Vtuple t) { for (auto& reg : unit.tuples[t]) use(reg); }
  void use(VcallArgsId id) {
    for (auto& reg : unit.vcallArgs[id].args) use(reg);
  }

  void use(Vptr& ptr) {
    // Rewrite memory operands that are based on registers we've copied or
    // lea'd off of other registers.
    if (ptr.seg != Segment::DS) return;

    int32_t off = 0;
    for (auto v = livefp; v != ptr.base;) {
      auto const it = regchain.find(v);
      if (it == regchain.end() || !it->second.second) return;
      off += it->second.second;
      v = it->second.first;
    }

    ptr.base = rvmfp();
    ptr.disp += off;
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
    if (reg == livefp) reg = rvmfp();
  }
};

void fixupVmfpUses(Vunit& unit) {
  struct BlockState {
    Vreg liveFp = InvalidReg;
    jit::fast_map<size_t, std::pair<Vreg,int32_t>> regchain;
  };

  auto const rpo = sortBlocks(unit);
  jit::vector<BlockState> states{unit.blocks.size()};

  states[rpo[0]].liveFp = rvmfp();

  for (auto const b : rpo) {
    auto& block = unit.blocks[b];
    auto fp = states[b].liveFp;
    auto regchain = states[b].regchain;
    always_assert_flog(fp.isValid(), "{} cannot have unknown vmfp", b);

    for (auto& inst : block.code) {
      auto const curFp = fp;
      switch (inst.op) {
      case Vinstr::defvmfp:
        always_assert(fp == rvmfp());
        fp = inst.defvmfp_.d;
        break;
      case Vinstr::pushvmfp: {
        auto const& p = inst.pushvmfp_;
        auto const& r = regchain.emplace(
          p.s, std::make_pair(fp, p.offset)
        ).first->second;
        always_assert(r.first == fp);
        always_assert(r.second == p.offset);
        fp = p.s;
        break;
      }
      case Vinstr::contenter: {
        auto const it = regchain.find(fp);
        always_assert(it != regchain.end());
        fp = it->second.first;
        regchain.erase(it);
        // fallthru
      }
      default:
        FpVisit visit{unit, curFp, regchain};
        visitOperands(inst, visit);
      }
    }

    for (auto const s : succs(block)) {
      auto& succState = states[s];
      if (succState.liveFp.isValid()) {
        always_assert_flog(
          succState.liveFp == fp,
          "{} has multiple known vmfp values ({} and {} via {})",
          s, show(succState.liveFp), show(fp), b
        );
        always_assert_flog(
          succState.regchain == regchain,
          "{} has mismatched reg-chain state with {}",
          s, b
        );
      } else {
        succState.liveFp = fp;
        succState.regchain = regchain;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Sort blocks in reverse postorder, and try to arrange fall-through blocks in
 * the same area to be close together.
 */
struct BlockSorter {
  explicit BlockSorter(const Vunit& unit)
    : unit(unit)
    , visited(unit.blocks.size())
  {
    blocks.reserve(unit.blocks.size());
  }

  unsigned area(Vlabel b) {
    return (unsigned)unit.blocks[b].area_idx;
  }

  void dfs(Vlabel b) {
    assertx(size_t(b) < unit.blocks.size());
    if (visited.test(b)) return;
    visited.set(b);

    if (area(b) == 0) {
      for (auto s : succs(unit.blocks[b])) {
        // visit colder
        if (area(s) > area(b)) dfs(s);
      }
      for (auto s : succs(unit.blocks[b])) {
        if (area(s) <= area(b)) dfs(s);
      }
    } else {
      for (auto s : succs(unit.blocks[b])) dfs(s);
    }
    blocks.push_back(b);
  }

  /*
   * Data members.
   */
  const Vunit& unit;
  jit::vector<Vlabel> blocks;
  boost::dynamic_bitset<> visited;
};

jit::vector<Vlabel> sortBlocks(const Vunit& unit) {
  BlockSorter s(unit);
  s.dfs(unit.entry);
  std::reverse(s.blocks.begin(), s.blocks.end());

  // Put the blocks containing "fallthru" last; expect at most one per Vunit.
  std::stable_partition(s.blocks.begin(), s.blocks.end(), [&] (Vlabel b) {
    auto& block = unit.blocks[b];
    auto& code = block.code;
    return !code.empty() && code.back().op != Vinstr::fallthru;
  });

  return s.blocks;
}

///////////////////////////////////////////////////////////////////////////////

}
