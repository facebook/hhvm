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
#include "hphp/runtime/vm/jit/vasm-info.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/assertions.h"
#include "hphp/util/dataflow-worklist.h"

#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(vasm);

namespace HPHP::jit {
///////////////////////////////////////////////////////////////////////////////

namespace {
///////////////////////////////////////////////////////////////////////////////

DEBUG_ONLY bool
checkSSA(const Vunit& unit, const jit::vector<Vlabel>& blocks) {
  using Bits = boost::dynamic_bitset<>;

  jit::vector<Bits> block_defs(unit.blocks.size()); // index by [Vlabel]
  Bits global_defs(unit.next_vr);
  Bits consts(unit.next_vr);

  for (auto const& c : unit.constToReg) {
    global_defs.set(c.second);
    consts.set(c.second);
  }
  for (auto const b : blocks) {
    Bits local_defs;
    if (block_defs[b].empty()) {
      local_defs.resize(unit.next_vr);
      for (auto const& c : unit.constToReg) {
        local_defs.set(c.second);
      }
    } else {
      local_defs = block_defs[b];
    }
    for (auto const& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&](Vreg v) {
        assert_flog(v.isValid(), "invalid vreg used in B{}\n{}",
                    size_t(b), show(unit));
        assert_flog(!v.isVirt() || local_defs[v],
                    "%{} used before def in B{}\n{}",
                    size_t(v), size_t(b), show(unit));
      });
      visitDefs(unit, inst, [&](Vreg v) {
        assert_flog(v.isValid(), "invalid vreg defined in B{}\n{}",
                    size_t(b), show(unit));
        assert_flog(!v.isVirt() || !consts.test(v),
                    "%{} const defined in B{}\n{}",
                    size_t(v), size_t(b), show(unit));
        assert_flog(!v.isVirt() || !local_defs[v],
                    "%{} locally redefined in B{}\n{}",
                    size_t(v), size_t(b), show(unit));
        assert_flog(!v.isVirt() || !global_defs[v],
                    "%{} redefined in B{}\n{}",
                    size_t(v), size_t(b), show(unit));
        local_defs.set(v);
        global_defs.set(v);
      });
    }

    auto const& block = unit.blocks[b];
    auto const lastOp = block.code.back().op;
    if (lastOp == Vinstr::phijmp) {
      for (DEBUG_ONLY auto s : succs(block)) {
        assert_flog(
          !unit.blocks[s].code.empty() &&
            unit.blocks[s].code.front().op == Vinstr::phidef,
          "B{} ends in {} but successor B{} doesn't begin with phidef\n",
          size_t(b), vinst_names[lastOp], size_t(s)
        );
      }
    }

    for (auto s : succs(block)) {
      if (block_defs[s].empty()) {
        block_defs[s] = local_defs;
      } else {
        block_defs[s] &= local_defs;
      }
    }
  }
  return true;
}

/*
 * Make sure syncpoint{}, nothrow{}, or unwind{} only appear immediately after
 * a call.
 */
DEBUG_ONLY bool
checkCalls(const Vunit& unit, const jit::vector<Vlabel>& blocks) {
  for (auto const b : blocks) {
    bool unwind_valid = false;
    bool nothrow_valid = false;
    bool sync_valid = false;
    for (auto const& inst : unit.blocks[b].code) {
      switch (inst.op) {
        case Vinstr::call:
        case Vinstr::callm:
        case Vinstr::callr:
        case Vinstr::calls:
        case Vinstr::callphp:
        case Vinstr::callphpfe:
        case Vinstr::callphpr:
        case Vinstr::callphps:
        case Vinstr::callstub:
        case Vinstr::contenter:
        case Vinstr::inlinesideexit:
          sync_valid = unwind_valid = nothrow_valid = true;
          break;
        case Vinstr::syncpoint:
          assertx(sync_valid);
          sync_valid = false;
          break;
        case Vinstr::unwind:
          assertx(unwind_valid);
          unwind_valid = nothrow_valid = false;
          break;
        case Vinstr::nothrow:
          assertx(nothrow_valid);
          unwind_valid = nothrow_valid = false;
          break;
        default:
          unwind_valid = nothrow_valid = sync_valid = false;
          break;
      }
    }
  }
  return true;
}

/* Check for any Vtuples are used by more than one Vinstr */

struct VtupleVisitor {
  explicit VtupleVisitor(const Vunit& unit)
    : unit{unit} { uses.resize(unit.tuples.size()); }

  const Vunit& unit;

  struct Pos { Vlabel block; size_t instr; };
  Pos pos;
  jit::vector<Pos> uses;

  template <typename T> void imm(const T&) const {}
  template <typename T> void use(const T&) const {}
  template <typename T> void def(const T&) const {}
  template <typename T> void across(const T& t) { use(t); }
  template <typename T, typename U>
  void useHint(const T& t, const U&) { use(t); }
  template <typename T, typename U>
  void defHint(const T& t, const U&) { def(t); }
  void use(Vtuple t) { check(t); }
  void def(Vtuple t) { check(t); }
  void check(Vtuple t) {
    auto const usePos = uses[t];
    always_assert_flog(
      !usePos.block.isValid(),
      "Instruction '{}' in {} uses a Vtuple already used by '{}' in {}\n{}\n",
      show(unit, unit.blocks[pos.block].code[pos.instr]),
      pos.block,
      show(unit, unit.blocks[usePos.block].code[usePos.instr]),
      usePos.block,
      show(unit)
    );
    uses[t] = pos;
  }
};

DEBUG_ONLY bool
checkVtuples(const Vunit& unit, const jit::vector<Vlabel>& blocks) {
  VtupleVisitor visitor{unit};
  for (auto const b : blocks) {
    auto const& code = unit.blocks[b].code;
    for (size_t i = 0; i < code.size(); ++i) {
      visitor.pos = VtupleVisitor::Pos{b, i};
      visitOperands(code[i], visitor);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

struct FlagUseChecker {
  explicit FlagUseChecker(VregSF& sf) : cur_sf(sf) {}
  template<class T> void imm(const T&) {}
  template<class T> void def(T) {}
  template<class T, class H> void defHint(T, H) {}
  template<class T> void across(T r) { use(r); }
  template<class T> void use(T) {}
  template<class T, class H> void useHint(T r, H) { use(r); }
  void use(RegSet regs) {
    regs.forEach([&](Vreg r) {
      if (r.isSF()) use(VregSF(r));
    });
  }
  void use(VregSF r) {
    assertx(!cur_sf.isValid() || cur_sf == r);
    assertx(!r.isValid() || r.isSF() || r.isVirt());
    cur_sf = r;
  }
  VregSF& cur_sf;
};

struct FlagDefChecker {
  explicit FlagDefChecker(VregSF& sf) : cur_sf(sf) {}
  template<class T> void imm(const T&) {}
  template<class T> void def(T) {}
  template<class T, class H> void defHint(T r, H) { def(r); }
  template <class T>
  void across(T /*r*/) {}
  template<class T> void use(T) {}
  template <class T, class H>
  void useHint(T /*r*/, H) {}
  void def(RegSet regs) {
    regs.forEach([&](Vreg r) {
      if (r.isSF()) def(VregSF(r));
    });
  }
  void def(VregSF r) {
    assertx(!cur_sf.isValid() || cur_sf == r);
    cur_sf = InvalidReg;
  }
  VregSF& cur_sf;
};

const Abi sf_abi {
  RegSet{}, RegSet{}, RegSet{}, RegSet{}, RegSet{},
  RegSet{RegSF{0}}
};

/*
 * Check that no two status-flag register lifetimes overlap.
 *
 * Traverses the code bottom up, keeping track of the currently live (single)
 * status-flag register, or InvalidReg if there isn't one live, then iterates
 * to a fixed point.
 */
DEBUG_ONLY bool
checkSF(const Vunit& unit, const jit::vector<Vlabel>& blocks) {
  auto const preds = computePreds(unit);
  jit::vector<uint32_t> blockPO(unit.blocks.size());
  auto revBlocks = blocks;
  auto wl = dataflow_worklist<uint32_t>(revBlocks.size());
  for (unsigned po = 0; po < revBlocks.size(); po++) {
    wl.push(po);
    blockPO[revBlocks[po]] = po;
  }
  jit::vector<VregSF> livein(unit.blocks.size(), VregSF{InvalidReg});
  while (!wl.empty()) {
    auto b = revBlocks[wl.pop()];
    auto& block = unit.blocks[b];
    VregSF cur_sf = InvalidReg;
    for (auto s : succs(block)) {
      if (!livein[s].isValid()) continue;
      assertx(!cur_sf.isValid() || cur_sf == livein[s]);
      assertx(livein[s].isSF() || livein[s].isVirt());
      cur_sf = livein[s];
    }
    for (auto i = block.code.end(); i != block.code.begin();) {
      auto& inst = *--i;
      RegSet implicit_uses, implicit_across, implicit_defs;
      if (inst.op == Vinstr::vcall || inst.op == Vinstr::vinvoke) {
        // getEffects would assert since these haven't been lowered yet.
        implicit_defs |= RegSF{0};
      } else {
        getEffects(sf_abi, inst, implicit_uses, implicit_across, implicit_defs);
      }
      FlagDefChecker d(cur_sf);
      visitOperands(inst, d);
      d.def(implicit_defs);
      FlagUseChecker u(cur_sf);
      visitOperands(inst, u);
      u.use(implicit_uses);
      u.across(implicit_across);
    }
    if (cur_sf != livein[b]) {
      livein[b] = cur_sf;
      for (auto p : preds[b]) wl.push(blockPO[p]);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Check if two width constraints have any overlap.
 */
DEBUG_ONLY bool compatible(Width w1, Width w2) {
  return (w1 & w2) != Width::None;
}

struct VisitOp {
  template<class T> void imm(T) {}
  template<class T> void across(T) {}
  template<class T, class H> void useHint(T,H) {}
  template<class T, class H> void defHint(T,H) {}
  void def(Vptr8 /*m*/) { memWidth = Width::Byte; }
  void def(Vptr m) { memWidth = m.width; }
  template <Width w>
  void def(Vp<w> /*m*/) {
    memWidth = w;
  }
  template <Width w>
  void use(Vp<w> /*m*/) {
    memWidth = w;
  }
  template<class T> void use(T) {}
  template<class T> void def(T) {}
  void use(Vreg r) { useWidth = width(r); }
  void def(Vreg r) { defWidth = width(r); }

  Width memWidth = Width::None;
  Width useWidth = Width::None;
  Width defWidth = Width::None;
};

DEBUG_ONLY bool
checkWidths(const Vunit& unit, const jit::vector<Vlabel>& blocks) {
  auto widths = jit::vector<Width>(unit.next_vr, Width::Any);

  for (auto const& pair : unit.constToReg) {
    auto const& cns = pair.first;
    auto const r = pair.second;

    widths[r] = [&] {
      switch (cns.kind) {
        case Vconst::Quad:
        case Vconst::Long:
        case Vconst::Byte:   return Width::QuadN;
        case Vconst::Double: return Width::Quad;
      }
      not_reached();
    }();
  }

  for (auto const b : blocks) {
    for (auto const& inst : unit.blocks[b].code) {
      if (inst.op == Vinstr::copy) {
        auto const& i = inst.copy_;

        if (!i.s.isPhys() && !i.d.isPhys()) {
          widths[i.d] = widths[i.s];
        }
      } else if (inst.op == Vinstr::copyargs) {
        auto const& i = inst.copyargs_;

        auto const& srcs = unit.tuples[i.s];
        auto const& dsts = unit.tuples[i.d];
        assertx(srcs.size() == dsts.size());

        for (auto i = 0; i < srcs.size(); ++i) {
          if (!srcs[i].isPhys() && !dsts[i].isPhys()) {
            widths[dsts[i]] = widths[srcs[i]];
          }
        }
      } else {
        visitDefs(unit, inst, [&] (Vreg r, Width w) {
          if (r.isPhys()) return;
          widths[r] = w;
        });
        visitUses(unit, inst, [&] (Vreg r, Width uw) {
          if (r.isPhys()) return;

          DEBUG_ONLY auto const dw = widths[r];
          assert_flog(compatible(dw, uw),
                      "width mismatch for %{}: def {}, use {}\n{}",
                      size_t(r), show(dw), show(uw), show(unit));
        });
        if (touchesMemory(inst.op) && !isCall(inst)) {
          switch (inst.op) {
            case Vinstr::loadzbl:
              static_assert(
                  std::is_same<decltype(inst.loadzbl_.s),Vptr8>::value,
                  "loadzbl should load a byte\n");
              static_assert(
                  std::is_same<decltype(inst.loadzbl_.d),Vreg32>::value,
                  "loadzbl should write a long\n");
                  break;
            case Vinstr::loadzbq:
              static_assert(
                  std::is_same<decltype(inst.loadzbq_.s),Vptr8>::value,
                  "loadzbq should load a byte\n");
              static_assert(
                  std::is_same<decltype(inst.loadzbq_.d),Vreg64>::value,
                  "loadzbq should write a quad\n");
                  break;
            case Vinstr::loadzwq:
              static_assert(
                  std::is_same<decltype(inst.loadzwq_.s),Vptr16>::value,
                  "loadzwq should load a word\n");
              static_assert(
                  std::is_same<decltype(inst.loadzwq_.d),Vreg64>::value,
                  "loadzwq should write a quad\n");
                  break;
            case Vinstr::loadzlq:
              static_assert(
                  std::is_same<decltype(inst.loadzlq_.s),Vptr32>::value,
                  "loadzlq should load a long\n");
              static_assert(
                  std::is_same<decltype(inst.loadzlq_.d),Vreg64>::value,
                  "loadzlq should write a quad\n");
            case Vinstr::loadtqb:
              static_assert(
                  std::is_same<decltype(inst.loadtqb_.s),Vptr64>::value,
                  "loadtqb should load a quad\n");
              static_assert(
                  std::is_same<decltype(inst.loadtqb_.d),Vreg8>::value,
                  "loadtqb should write a byte\n");
                  break;
            case Vinstr::loadtql:
              static_assert(
                  std::is_same<decltype(inst.loadtql_.s),Vptr64>::value,
                  "loadtql should load a quad\n");
              static_assert(
                  std::is_same<decltype(inst.loadtql_.d),Vreg32>::value,
                  "loadtql should write a long\n");
                  break;
            default:
              VisitOp vo;
              visitOperands(inst, vo);
              DEBUG_ONLY Width mw = vo.memWidth & Width::AnyNF;
              DEBUG_ONLY Width rw = vo.useWidth & Width::AnyNF;
              DEBUG_ONLY Width dw = vo.defWidth & Width::AnyNF;
              assert_flog(dw == Width::None || compatible(dw, mw),
                "width mismatch : def {}, mem {} \n {} \n{}",
                show(dw), show(mw), show(unit,inst), show(unit));
              assert_flog(rw == Width::None || compatible(rw, mw),
                "width mismatch : use {}, mem {} \n {}\n{}",
                show(rw), show(mw), show(unit,inst), show(unit));
          }
        }
      }
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}

bool check(Vunit& unit) {
  auto const blocks = sortBlocks(unit);
  assertx(checkSSA(unit, blocks));
  assertx(checkCalls(unit, blocks));
  assertx(checkSF(unit, blocks));
  assertx(checkVtuples(unit, blocks));
  return true;
}

bool checkWidths(Vunit& unit) {
  auto const blocks = sortBlocks(unit);
  assertx(checkWidths(unit, blocks));
  return true;
}

bool checkBlockEnd(const Vunit& unit, Vlabel b) {
  assertx(!unit.blocks[b].code.empty());
  auto& block = unit.blocks[b];
  auto n = block.code.size();
  for (size_t i = 0; i < n - 1; ++i) {
    assertx(!isBlockEnd(block.code[i]));
  }
  assertx(isBlockEnd(block.code[n - 1]));
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool checkNoSideExits(const Vunit& unit) {
  auto const DEBUG_ONLY isSideExit = [] (const Vinstr& inst) {
    return
      inst.op == Vinstr::jcci ||
      inst.op == Vinstr::fallbackcc ||
      inst.op == Vinstr::bindjcc;
  };

  auto const blocks = sortBlocks(unit);
  for (auto const b : blocks) {
    for (DEBUG_ONLY auto const& inst : unit.blocks[b].code) {
      assert_flog(
        !isSideExit(inst),
        "Disallowed side-exiting instruction {} in {}\n{}",
        show(unit, inst),
        b,
        show(unit)
      );
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool checkNoCriticalEdges(const Vunit& unit) {
  auto const blocks = sortBlocks(unit);
  auto const DEBUG_ONLY preds = computePreds(unit);
  for (auto const b : blocks) {
    auto const& block = unit.blocks[b];
    auto const successors = succs(block);
    if (successors.size() < 2) continue;
    for (auto const DEBUG_ONLY succ : successors) {
      assert_flog(
        preds[succ].size() <= 1,
        "Disallowed critical edge from {} to {}\n{}",
        b, succ,
        show(unit)
      );
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

}
