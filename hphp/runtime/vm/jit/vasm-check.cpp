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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/assertions.h"
#include "hphp/util/dataflow-worklist.h"

#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
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
    if (lastOp == Vinstr::phijmp || lastOp == Vinstr::phijcc) {
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
        case Vinstr::callstub:
        case Vinstr::callarray:
        case Vinstr::contenter:
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
    assert(!r.isValid() || r.isSF() || r.isVirt());
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
      assert(livein[s].isSF() || livein[s].isVirt());
      cur_sf = livein[s];
    }
    for (auto i = block.code.end(); i != block.code.begin();) {
      auto& inst = *--i;
      RegSet implicit_uses, implicit_across, implicit_defs;
      if (inst.op == Vinstr::vcall || inst.op == Vinstr::vinvoke ||
          inst.op == Vinstr::vcallarray) {
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
      } else if (inst.op == Vinstr::copy2) {
        auto const& i = inst.copy2_;

        if (!i.s0.isPhys() && !i.d0.isPhys()) {
          widths[i.d0] = widths[i.s0];
        }
        if (!i.s1.isPhys() && !i.d1.isPhys()) {
          widths[i.d1] = widths[i.s1];
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
}}
