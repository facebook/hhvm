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
#include "hphp/runtime/vm/jit/vasm-info.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <boost/dynamic_bitset.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <algorithm>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

// Remove dead instructions by doing a traditional liveness analysis.
// instructions that mutate memory, physical registers, or status flags are
// considered useful. All branches are considered useful. We give copyargs and
// phijmp/defs special treatment to allow for liveness to propagate for each
// individual src/dst pair.
//
// Given SSA, there's a faster sparse version of this algorithm that marks
// useful instructions in one pass, then transitively marks pure instructions
// that define inputs to useful instructions. However it requires a mapping
// from vreg numbers to the instruction that defines them, and a way to address
// individual instructions.
//
// We could remove useless branches by computing the post-dominator tree and
// RDF(b) for each block; then a branch is only useful if it controls whether
// or not a useful block executes, and useless branches can be forwarded to
// the nearest useful post-dominator.
void removeDeadCode(Vunit& unit, MaybeVinstrId clobber) {
  Timer timer(Timer::vasm_dce);

  assertx(check(unit));

  auto const blocks = sortBlocks(unit);

  boost::dynamic_bitset<> live{unit.next_vr};
  auto const isLive = [&] (Vreg r) { return r.isPhys() || live[r]; };

  auto const isLiveInst = [&] (const Vinstr& inst) {
    if (effectful(inst)) return true;
    auto anyLive = false;
    visitDefs(
      unit, inst,
      [&] (Vreg r) { anyLive |= isLive(r); }
    );
    return anyLive;
  };

  // Lazily load preds as needed
  Optional<PredVector> preds;
  auto const getPreds = [&] () -> const PredVector& {
    if (preds) return *preds;
    preds = computePreds(unit);
    return *preds;
  };

  auto const analyze = [&] {
    auto changed = false;
    for (auto const b : boost::adaptors::reverse(blocks)) {
      auto const& block = unit.blocks[b];
      for (auto const& inst : boost::adaptors::reverse(block.code)) {
        switch (inst.op) {
          case Vinstr::copyargs: {
            // A copyarg's source is live if only its corresponding dest is.
            auto const& s = unit.tuples[inst.copyargs_.s];
            auto const& d = unit.tuples[inst.copyargs_.d];
            assertx(s.size() == d.size());
            for (size_t i = 0; i < s.size(); ++i) {
              if (!isLive(d[i]) || isLive(s[i])) continue;
              live[s[i]] = true;
              changed = true;
            }
            break;
          }
          case Vinstr::copy2:
            if (isLive(inst.copy2_.d0) && !isLive(inst.copy2_.s0)) {
              live[inst.copy2_.s0] = true;
              changed = true;
            }
            if (isLive(inst.copy2_.d1) && !isLive(inst.copy2_.s1)) {
              live[inst.copy2_.s1] = true;
              changed = true;
            }
            break;
          case Vinstr::phijmp: {
            // A phijmp's source is live if the corresponding dest in the phidef
            // is.
            auto const& s = unit.tuples[inst.phijmp_.uses];
            auto const& phidef = unit.blocks[inst.phijmp_.target].code.front();
            assertx(phidef.op == Vinstr::phidef);
            auto const& d = unit.tuples[phidef.phidef_.defs];
            assertx(d.size() == s.size());
            for (size_t i = 0; i < d.size(); ++i) {
              if (!isLive(d[i]) || isLive(s[i])) continue;
              live[s[i]] = true;
              changed = true;
            }
            break;
          }
          case Vinstr::phidef: // Processed as part of phijmp above
            break;
          default:
            // Otherwise the sources are only live if any of the dests are.
            if (!isLiveInst(inst)) break;
            visitUses(
              unit, inst,
              [&] (Vreg r) {
                if (isLive(r)) return;
                live[r] = true;
                changed = true;
              }
            );
            break;
        }
      }
    }

    return changed;
  };

  auto const remove = [&] {
    auto changed = false;
    for (auto const b : blocks) {
      auto& block = unit.blocks[b];
      for (auto& inst : block.code) {
        switch (inst.op) {
          case Vinstr::copyargs: {
            // Remove dead source/dest pairs from copyargs.
            auto const& s = unit.tuples[inst.copyargs_.s];
            auto const& d = unit.tuples[inst.copyargs_.d];
            assertx(s.size() == d.size());
            if (!d.empty() && std::all_of(d.begin(), d.end(), isLive)) break;

            VregList newSrcs;
            VregList newDsts;
            for (size_t i = 0; i < s.size(); ++i) {
              if (!isLive(d[i])) continue;
              assertx(isLive(s[i]));
              newSrcs.emplace_back(s[i]);
              newDsts.emplace_back(d[i]);
            }

            // If no pairs remain, turn it into a nop. If there's a single pair
            // left, a copy. Otherwise just shrink the argument tuples.
            assertx(newSrcs.size() == newDsts.size());
            if (newDsts.empty()) {
              inst = nop{};
            } else if (newDsts.size() == 1) {
              inst.op = Vinstr::copy;
              inst.copy_ = copy{newSrcs[0], newDsts[0]};
            } else {
              inst.copyargs_.s = unit.makeTuple(std::move(newSrcs));
              inst.copyargs_.d = unit.makeTuple(std::move(newDsts));
            }
            if (clobber) inst.id = *clobber;
            changed = true;
            break;
          }
          case Vinstr::copy2:
            if (!isLive(inst.copy2_.d0)) {
              if (!isLive(inst.copy2_.d1)) {
                inst = nop{};
              } else {
                auto const s = inst.copy2_.s1;
                auto const d = inst.copy2_.d1;
                inst.op = Vinstr::copy;
                inst.copy_ = copy{s, d};
              }
              if (clobber) inst.id = *clobber;
              changed = true;
            } else if (!isLive(inst.copy2_.d1)) {
              auto const s = inst.copy2_.s0;
              auto const d = inst.copy2_.d0;
              inst.op = Vinstr::copy;
              inst.copy_ = copy{s, d};
              if (clobber) inst.id = *clobber;
              changed = true;
            }
            break;
          case Vinstr::phidef: {
            // Remove dead source/dest pairs from phidef/phijmp.
            auto d = &unit.tuples[inst.phidef_.defs];
            if (!d->empty() && std::all_of(d->begin(), d->end(), isLive)) break;

            // First remove dead sources from the phijmps in all the
            // predecessors.
            for (auto const pred : getPreds()[b]) {
              auto& phijmp = unit.blocks[pred].code.back();
              assertx(phijmp.op == Vinstr::phijmp);
              auto const& s = unit.tuples[phijmp.phijmp_.uses];
              assertx(s.size() == d->size());

              VregList newSrcs;
              for (size_t i = 0; i < d->size(); ++i) {
                if (!isLive((*d)[i])) continue;
                assertx(isLive(s[i]));
                newSrcs.emplace_back(s[i]);
              }

              // If there's no sources left, just leave a jmp. Otherwise shrink
              // the argument tuple.
              if (newSrcs.empty()) {
                assertx(phijmp.phijmp_.target == b);
                phijmp.op = Vinstr::jmp;
                phijmp.jmp_.target = b;
              } else {
                phijmp.phijmp_.uses = unit.makeTuple(std::move(newSrcs));
                // makeTuple may have invalidated d
                d = &unit.tuples[inst.phidef_.defs];
              }
              if (clobber) phijmp.id = *clobber;
            }

            // Now likewise shrink the dests in the phidef
            VregList newDsts;
            for (size_t i = 0; i < d->size(); ++i) {
              if (!isLive((*d)[i])) continue;
              newDsts.emplace_back((*d)[i]);
            }

            if (newDsts.empty()) {
              inst = nop{};
            } else {
              inst.phidef_.defs = unit.makeTuple(std::move(newDsts));
            }

            if (clobber) inst.id = *clobber;
            changed = true;
            break;
          }
          case Vinstr::phijmp: // Processed as part of the phidef above.
            break;
          default:
            // Otherwise change the instruction into a nop if all of its dests
            // are dead.
            if (isLiveInst(inst)) continue;
            changed = true;
            inst = nop{};
            if (clobber) inst.id = *clobber;
            break;
        }
      }
    }
    return changed;
  };

  while (analyze()) {}
  auto const changed = remove();
  removeTrivialNops(unit);
  if (changed) {
    printUnit(kVasmDCELevel, "after vasm-dead", unit);
    assertx(check(unit));
  }
}

/*
 * A very simple dead code elimination pass that just removes trivial nop
 * instructions.  We run this before any other passes because it allows
 * code-gen to create things like self-copies or self-lea's without affect on
 * optimizations downstream.  (In particular early passes like optimizeExits
 * that are looking for specific vasm sequences inside of a block.)
 */
void removeTrivialNops(Vunit& unit) {
  for (auto& b : unit.blocks) {
    b.code.erase(
      std::remove_if(
        begin(b.code), end(b.code),
        is_trivial_nop
      ),
      end(b.code)
    );
  }
}

}}
