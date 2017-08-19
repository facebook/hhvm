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

#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <boost/dynamic_bitset.hpp>

#include <algorithm>
#include <cstdint>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

// Determine the natural width of a register based on its def(s) and uses.
struct WidthAnalysis {
  explicit WidthAnalysis(Vunit& unit);

  Width getAppropriate(Vreg r) const {
    assertx(r < m_def_widths.size() && r < m_use_widths.size());
    return m_def_widths[r] & m_use_widths[r];
  }

private:
  Vunit& m_unit;
  jit::vector<Width> m_def_widths;
  jit::vector<Width> m_use_widths;
};

WidthAnalysis::WidthAnalysis(Vunit& unit)
  : m_unit(unit)
  , m_def_widths{unit.next_vr, Width::Any}
  , m_use_widths{unit.next_vr, Width::Any}
{
  // Initialize physical registers and constants with their appropriate widths.

  for (size_t i = 0; i < m_def_widths.size(); ++i) {
    Vreg r{i};
    if (r.isGP()) m_def_widths[i] &= Width::QuadN;
    if (r.isSIMD()) m_def_widths[i] &= Width::Octa;
    if (r.isSF()) m_def_widths[i] &= Width::Flags;
  }

  for (auto const& pair : unit.constToReg) {
    auto const& cns = pair.first;
    auto const r = pair.second;

    m_def_widths[r] = [&] {
      switch (cns.kind) {
        case Vconst::Quad:
        case Vconst::Long:
        case Vconst::Byte:   return Width::QuadN;
        case Vconst::Double: return Width::Quad;
      }
      not_reached();
    }();
  }

  // Record which registers are just copies from another register. We have to
  // iterate through these copies to find a fixed-point of the widths. A phi is
  // considered a copy for this purpose.
  jit::vector<copy> copies;

  auto const blocks = sortBlocks(unit);
  for (auto const b : blocks) {
    for (auto const& inst : unit.blocks[b].code) {
      if (inst.op == Vinstr::copy) {
        copies.emplace_back(inst.copy_);
      } else if (inst.op == Vinstr::copyargs) {
        auto const& i = inst.copyargs_;

        auto const& srcs = unit.tuples[i.s];
        auto const& dsts = unit.tuples[i.d];
        assertx(srcs.size() == dsts.size());

        for (auto i = 0; i < srcs.size(); ++i) {
          copies.emplace_back(copy{srcs[i], dsts[i]});
        }
      } else if (inst.op == Vinstr::phijmp) {
        auto const& jmp = inst.phijmp_;

        auto const& target = unit.blocks[jmp.target];
        assertx(!target.code.empty() && target.code[0].op == Vinstr::phidef);

        auto const& phi = target.code[0].phidef_;

        auto const& uses = unit.tuples[jmp.uses];
        auto const& defs = unit.tuples[phi.defs];

        assertx(uses.size() == defs.size());
        for (auto i = 0; i < uses.size(); ++i) {
          copies.emplace_back(copy{uses[i], defs[i]});
        }
      } else {
        visitUses(m_unit, inst, [&] (Vreg r, Width w) {
          m_use_widths[r] &= w;
        });
        visitDefs(m_unit, inst, [&] (Vreg r, Width w) {
          m_def_widths[r] &= w;
        });
      }
    }
  }

  // If there were no copies, we'd be done at this point. However, since copies
  // propagate the computed widths through them, we need to find a fixed-point
  // in classical dataflow style.
  bool changed;
  do {
    changed = false;
    for (auto const& p : copies) {
      auto orig = m_def_widths[p.d];
      m_def_widths[p.d] &= m_def_widths[p.s];
      if (m_def_widths[p.d] != orig) changed = true;

      orig = m_use_widths[p.s];
      m_use_widths[p.s] &= m_use_widths[p.d];
      if (m_use_widths[p.s] != orig) changed = true;
    }
  } while (changed);
}

// Is this block composed of nothing but the specified op?
bool isOnly(Vunit& unit, Vlabel b, Vinstr::Opcode op) {
  auto& code = unit.blocks[b].code;
  return code.size() == 1 && op == code[0].op;
}

/*
 * Collapse trivial diamonds into cmovs.
 *
 * Transform this pattern:
 *
 *   jcc %cc $t $f
 *   ...
 *   t: phijmp %reg1 $d
 *   ...
 *   f: phijmp %reg2 $d
 *   ...
 *   d: phidef => %reg3
 *
 * into:
 *
 *   cmov cc %reg1 %reg2 => %reg4
 *   phijmp %reg4 $d
 *   ...
 *   d: phidef => %reg3
 *
 * where the phijmp/phidef pair will be later optimized away and the
 * blocks combined if $d has no other predecessors. The cmov may be
 * further optimized into a setcc in vasm-simplify if the registers
 * are constants of the correct value.
 */
bool diamondIntoCmov(Vunit& unit, jcc& jcc_i,
                     jit::vector<Vinstr>& code,
                     jit::vector<int>& npreds) {
  auto const next = jcc_i.targets[0];
  auto const taken = jcc_i.targets[1];
  if (!isOnly(unit, next, Vinstr::phijmp)) return false;
  if (!isOnly(unit, taken, Vinstr::phijmp)) return false;

  auto const& next_phi = unit.blocks[next].code[0].phijmp_;
  auto const& taken_phi = unit.blocks[taken].code[0].phijmp_;
  if (next_phi.target != taken_phi.target) return false;

  auto const& join_block = unit.blocks[next_phi.target];
  assertx(join_block.code.size() > 0 &&
          join_block.code[0].op == Vinstr::phidef);

  auto const& next_uses = unit.tuples[next_phi.uses];
  auto const& taken_uses = unit.tuples[taken_phi.uses];
  DEBUG_ONLY auto const& join_defs =
    unit.tuples[join_block.code[0].phidef_.defs];

  assertx(next_uses.size() == taken_uses.size());
  assertx(next_uses.size() == join_defs.size());

  // To insert the proper cmov, we need to know what natural width of
  // the registers (based on where they're used and defined).
  WidthAnalysis analysis{unit};

  auto const irctx = code.back().irctx();
  auto const cc = jcc_i.cc;
  auto const sf = jcc_i.sf;

  // Construct all the necessary cmovs, returning (stopping the
  // optimization) if any of the registers have a width that we can't
  // do a conditional move on.
  VregList new_dests;
  std::remove_reference<decltype(code)>::type moves;
  for (size_t i = 0; i < next_uses.size(); ++i) {
    auto const r1 = next_uses[i];
    auto const r2 = taken_uses[i];
    auto const d = unit.makeReg();
    switch (analysis.getAppropriate(r1) &
            analysis.getAppropriate(r2)) {
      case Width::Byte:
        moves.emplace_back(cmovb{cc, sf, r1, r2, d}, irctx);
        break;
      case Width::Word:
      case Width::WordN:
        moves.emplace_back(cmovw{cc, sf, r1, r2, d}, irctx);
        break;
      case Width::Long:
      case Width::LongN:
        moves.emplace_back(cmovl{cc, sf, r1, r2, d}, irctx);
        break;
      case Width::Quad:
      case Width::QuadN:
        moves.emplace_back(cmovq{cc, sf, r1, r2, d}, irctx);
        break;
      case Width::Octa:
      case Width::Flags:
      case Width::AnyNF:
      case Width::Any:
      case Width::None:
        return false;
    }
    new_dests.emplace_back(d);
  }

  // If the branches have no other predecessors, clobber their phijmp
  // instructions, so that this optimization pass will properly
  // realize the join block doesn't have them as predecessors any
  // longer.
  auto const join = next_phi.target;
  ++npreds[join];
  if (!--npreds[next]) {
    unit.blocks[next].code[0] = ud2{};
    --npreds[join];
  }
  if (!--npreds[taken]) {
    unit.blocks[taken].code[0] = ud2{};
    --npreds[join];
  }

  // Insert the cmovs and the phijmp.
  code.pop_back();
  code.insert(code.end(), moves.begin(), moves.end());
  code.emplace_back(phijmp{join, unit.makeTuple(std::move(new_dests))}, irctx);

  return true;
}

}

void optimizeJmps(Vunit& unit) {
  Timer timer(Timer::vasm_jumps);
  bool changed = false;
  bool ever_changed = false;
  // The number of incoming edges from (reachable) predecessors for each block.
  // It is maintained as an upper bound of the actual value during the
  // transformation.
  jit::vector<int> npreds(unit.blocks.size(), 0);
  do {
    if (changed) {
      std::fill(begin(npreds), end(npreds), 0);
    }
    changed = false;
    PostorderWalker{unit}.dfs([&](Vlabel b) {
      for (auto s : succs(unit.blocks[b])) {
        npreds[s]++;
      }
    });
    // give entry an extra predecessor to prevent cloning it.
    npreds[unit.entry]++;

    PostorderWalker{unit}.dfs([&](Vlabel b) {
      auto& block = unit.blocks[b];
      auto& code = block.code;
      assertx(!code.empty());
      if (code.back().op == Vinstr::jcc) {
        auto ss = succs(block);
        if (ss[0] == ss[1]) {
          // both edges have same target, change to jmp
          code.back() = jmp{ss[0]};
          --npreds[ss[0]];
          changed = true;
        } else {
          auto jcc_i = code.back().jcc_;
          if (isOnly(unit, jcc_i.targets[0], Vinstr::fallback)) {
            jcc_i = jcc{ccNegate(jcc_i.cc), jcc_i.sf,
                        {jcc_i.targets[1], jcc_i.targets[0]}};
          }
          if (isOnly(unit, jcc_i.targets[1], Vinstr::fallback)) {
            // replace jcc with fallbackcc and jmp
            const auto& fb_i = unit.blocks[jcc_i.targets[1]].code[0].fallback_;
            const auto t0 = jcc_i.targets[0];
            const auto jcc_irctx = code.back().irctx();
            code.pop_back();
            code.emplace_back(
              fallbackcc{jcc_i.cc, jcc_i.sf, fb_i.target,
                         fb_i.spOff, fb_i.trflags, fb_i.args},
              jcc_irctx
            );
            code.emplace_back(jmp{t0}, jcc_irctx);
            changed = true;
          }

          changed |= diamondIntoCmov(unit, jcc_i, code, npreds);
        }
      }

      for (auto& s : succs(block)) {
        if (isOnly(unit, s, Vinstr::jmp)) {
          // skip over s
          --npreds[s];
          s = unit.blocks[s].code.back().jmp_.target;
          ++npreds[s];
          changed = true;
        }
      }

      // If the block starts with a phijmp, and only has one predecessor, turn
      // the phijmp/phidef pair into a copy and then a jmp. The two blocks
      // should then be combined with the below jmp optimization. Later on, copy
      // propagation should eliminate the copies.
      if (code.back().op == Vinstr::phijmp) {
        auto const target = code.back().phijmp_.target;
        if (npreds[target] == 1) {
          assertx(unit.blocks[target].code.size() > 0 &&
                  unit.blocks[target].code[0].op == Vinstr::phidef);
          auto const& uses = code.back().phijmp_.uses;
          auto const& defs = unit.blocks[target].code[0].phidef_.defs;
          assertx(unit.tuples[uses].size() == unit.tuples[defs].size());

          auto const irctx = code.back().irctx();
          code.pop_back();
          code.emplace_back(copyargs{uses, defs}, irctx);
          code.emplace_back(jmp{target}, irctx);

          unit.blocks[target].code[0] = nop{};

          changed = true;
        }
      }

      if (code.back().op == Vinstr::jmp) {
        auto s = code.back().jmp_.target;
        if (npreds[s] == 1 || isOnly(unit, s, Vinstr::jcc)) {
          // overwrite jmp with copy of s
          auto& code2 = unit.blocks[s].code;
          code.pop_back();
          code.insert(code.end(), code2.begin(), code2.end());
          if (--npreds[s]) {
            for (auto ss : succs(block)) {
              ++npreds[ss];
            }
          }
          changed = true;
        }
      }
    });
    ever_changed |= changed;
  } while (changed);
  if (ever_changed) {
    printUnit(kVasmJumpsLevel, "after vasm-jumps", unit);
  }
}

}}
