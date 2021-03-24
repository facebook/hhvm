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

#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/dataflow-worklist.h"

#include <boost/range/adaptor/reversed.hpp>

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

bool is_nop(const copy& i) { return i.s == i.d; }
bool is_nop(const copy2& i) { return i.s0 == i.d0 && i.s1 == i.d1; }

// movb r,r is a nop, however movl is not since it zeros upper bits.
bool is_nop(const movb& i) { return i.s == i.d; }

bool is_nop(const lea& i) {
  if (i.s.disp != 0) return false;
  return
    (i.s.base == i.d && !i.s.index.isValid()) ||
    (!i.s.base.isValid() && i.s.index == i.d && i.s.scale == 1 &&
      i.s.seg == Segment::DS);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

bool is_trivial_nop(const Vinstr& inst) {
  return
    (inst.op == Vinstr::copy && is_nop(inst.copy_)) ||
    (inst.op == Vinstr::copy2 && is_nop(inst.copy2_)) ||
    (inst.op == Vinstr::lea && is_nop(inst.lea_)) ||
    (inst.op == Vinstr::movb && is_nop(inst.movb_)) ||
    inst.op == Vinstr::nop;
}

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Add a jump from middleLabel to destLabel, taking into account any special
 * instructions at the beginning of destLabel.
 */
void forwardJmp(Vunit& unit, jit::flat_set<size_t>& catch_blocks,
                Vlabel middleLabel, Vlabel destLabel) {
  auto& middle = unit.blocks[middleLabel];
  auto& dest = unit.blocks[destLabel];

  auto& headInst = dest.code.front();
  auto irctx = headInst.irctx();
  if (headInst.op == Vinstr::phidef) {
    // We need to preserve any phidefs in the forwarding block if they're
    // present in the original destination block.
    auto tuple = headInst.phidef_.defs;
    auto forwardedRegs = unit.tuples[tuple];
    VregList regs(forwardedRegs.size());
    for (unsigned i = 0; i < forwardedRegs.size(); ++i) {
      regs[i] = unit.makeReg();
    }
    middle.code.emplace_back(phidef{unit.makeTuple(regs)}, irctx);
    middle.code.emplace_back(phijmp{destLabel, unit.makeTuple(regs)}, irctx);
    return;
  } else if (headInst.op == Vinstr::landingpad) {
    // If the dest started with a landingpad, copy it to middle. The dest's
    // will be erased at the end of the pass.
    catch_blocks.insert(destLabel);
    assertx(middle.code.empty());
    middle.code.emplace_back(headInst);
  }

  middle.code.emplace_back(jmp{destLabel}, irctx);
}

}

/*
 * Splits the critical edges in `unit', if any.
 * Returns true iff the unit was modified.
 */
bool splitCriticalEdges(Vunit& unit) {
  jit::vector<unsigned> preds(unit.blocks.size());
  jit::flat_set<size_t> catch_blocks;

  auto const rpo = sortBlocks(unit);
  for (auto const b : rpo) {
    auto succlist = succs(unit.blocks[b]);
    for (auto succ : succlist) {
      preds[succ]++;
    }
  }

  auto changed = false;
  for (auto const pred : rpo) {
    auto succlist = succs(unit.blocks[pred]);
    if (succlist.size() <= 1) continue;
    for (auto& succ : succlist) {
      if (preds[succ] <= 1) continue;
      // Split the critical edge. Use the colder of the predecessor and
      // successor to select the area of the new block.
      auto middle = unit.makeBlock(
        std::max(unit.blocks[pred].area_idx, unit.blocks[succ].area_idx),
        std::min(unit.blocks[pred].weight, unit.blocks[succ].weight)
      );
      forwardJmp(unit, catch_blocks, middle, succ);
      succ = middle;
      changed = true;
    }
  }

  // Remove any landingpad{} instructions that were hoisted to split edges.
  for (auto block : catch_blocks) {
    auto& code = unit.blocks[block].code;
    assertx(code.front().op == Vinstr::landingpad);
    code.front() = nop{};
  }

  return changed;
}

Vloc make_const(Vunit& unit, Type type) {
  if (type.subtypeOfAny(TUninit, TInitNull)) {
    // Return undefined value.
    return Vloc{unit.makeConst(Vconst::Quad)};
  }
  if (type <= TNullptr) return Vloc{unit.makeConst(0)};

  assertx(type.hasConstVal());
  if (type <= TBool) return Vloc{unit.makeConst(type.boolVal())};
  if (type <= TDbl) return Vloc{unit.makeConst(type.dblVal())};
  if (type <= TLvalToCell) {
    auto const rval = tv_rval{type.ptrVal()};
    auto const typeReg = unit.makeConst(&rval.type());
    auto const valReg = unit.makeConst(&rval.val());
    return Vloc{
      tv_lval::type_idx == 0 ? typeReg : valReg,
      tv_lval::val_idx == 1 ? valReg : typeReg,
    };
  }
  return Vloc{unit.makeConst(type.rawVal())};
}

//////////////////////////////////////////////////////////////////////

bool dominates(Vlabel b1, Vlabel b2, const VIdomVector& idoms) {
  assertx(b1.isValid() && b2.isValid());
  for (auto b = b2; b.isValid(); b = idoms[b]) {
    if (b == b1) return true;
  }
  return false;
}

VIdomVector findDominators(const Vunit& unit,
                           const jit::vector<Vlabel>& rpo) {
  assertx(!rpo.empty() && rpo[0] == unit.entry);

  auto const preds = computePreds(unit);
  VIdomVector idom(unit.blocks.size());

  jit::vector<size_t> rpoOrder(unit.blocks.size());
  for (size_t i = 0; i < rpo.size(); ++i) rpoOrder[rpo[i]] = i;

  dataflow_worklist<size_t> worklist(rpo.size());

  idom[unit.entry] = unit.entry;
  for (auto const succ : succs(unit.blocks[0])) {
    worklist.push(rpoOrder[succ]);
  }

  while (!worklist.empty()) {
    auto const block = rpo[worklist.pop()];
    auto const& blockPreds = preds[block];

    // Find the first already processed predecessor (there must be at least one
    // because we shouldn't be on the worklist otherwise).
    auto predIt = std::find_if(
      blockPreds.begin(),
      blockPreds.end(),
      [&] (Vlabel p) { return idom[p].isValid(); }
    );
    assertx(predIt != blockPreds.end());
    auto p1 = *predIt;

    // For all other already processed predecessors
    for (++predIt; predIt != blockPreds.end(); ++predIt) {
      auto p2 = *predIt;
      if (p2 == p1 || !idom[p2].isValid()) continue;
      do {
        // Find earliest common predecessor of p1 and p2
        while (rpoOrder[p1] < rpoOrder[p2]) p2 = idom[p2];
        while (rpoOrder[p2] < rpoOrder[p1]) p1 = idom[p1];
      } while (p1 != p2);
    }

    if (!idom[block].isValid() || idom[block] != p1) {
      idom[block] = p1;
      for (auto const succ : succs(unit.blocks[block])) {
        worklist.push(rpoOrder[succ]);
      }
    }
  }

  idom[unit.entry] = Vlabel{}; // entry has no dominator
  return idom;
}

//////////////////////////////////////////////////////////////////////

BackEdgeVector findBackEdges(const Vunit& unit,
                             const jit::vector<Vlabel>& rpo,
                             const VIdomVector& idoms) {
  BackEdgeVector backEdges;

  boost::dynamic_bitset<> seen(unit.blocks.size());
  for (auto const b : rpo) {
    seen[b] = true;
    for (auto const succ : succs(unit.blocks[b])) {
      if (!seen[succ]) continue; // If we haven't seen it, it can't dominate b,
                                 // so skip the dominator check.
      if (!dominates(succ, b, idoms)) continue;
      backEdges.emplace_back(b, succ);
    }
  }

  return backEdges;
}

LoopBlocks findLoopBlocks(const Vunit& unit,
                          const PredVector& preds,
                          const BackEdgeVector& backEdges) {

  jit::fast_map<Vlabel, jit::vector<Vlabel>> headers;
  for (auto const& edge : backEdges) {
    headers[edge.second].emplace_back(edge.first);
  }

  /*
   * Use a flood-fill algorithm, starting at the first node of the back-edge
   * (which is a predecessor of the loop header inside the loop). Any node which
   * is reachable via predecessors from that node (not passing through the
   * header block) is considered part of the loop. This may not be true for
   * irreducible loops.
   */
  auto const fillBlocks = [&] (Vlabel header,
                               const jit::vector<Vlabel>& edgePreds) {
    boost::dynamic_bitset<> visited(unit.blocks.size());
    visited[header] = true;

    jit::vector<Vlabel> blocks;
    blocks.emplace_back(header);

    jit::stack<Vlabel> worklist;
    for (auto const pred : edgePreds) worklist.push(pred);

    while (!worklist.empty()) {
      auto const block = worklist.top();
      worklist.pop();

      if (visited[block]) continue;
      visited[block] = true;

      for (auto const pred : preds[block]) worklist.push(pred);
      blocks.emplace_back(block);
    }

    return blocks;
  };

  jit::fast_map<Vlabel, jit::vector<Vlabel>> loopBlocks;
  for (auto const& p : headers) {
    loopBlocks[p.first] = fillBlocks(p.first, p.second);
  }

  return loopBlocks;
}

//////////////////////////////////////////////////////////////////////

/*
 * This SSA restoration algorithm is based on "Simple and Efficent Construction
 * of Static Single Assignment Form" by Matthias Braun, Sebastian Buchwald,
 * Sebastian Hack, et all (with a few tweaks).
 *
 * The idea is to walk the blocks of the unit in RPO order. For each eligible
 * Vreg which is being defined, we assign a fresh Vreg and keep a mapping of the
 * old Vreg to the new one at that particular label. For each eligible Vreg
 * which is used, we walk backwards through predecessors to find the closest
 * definition of that Vreg (where a mapping is defined). We then use the mapping
 * to rewrite that Vreg use to the new one. If there's more than one definition
 * available at that point, we create a Phi where the two definitions merge (and
 * use the dest of the Phi as the new Vreg).
 *
 * This, on its own, would be fairly expensive (because we'd be walking
 * backwards a lot). So, instead we memoize the lookup results heavily so that
 * most lookups don't need to do any walking at all.
 *
 * We also defer construction of Phis until after every Vreg has been rewritten
 * (we record that they should exist, but don't actually insert them). This lets
 * us find Phis where all the inputs are the same (or are self-defined), which
 * can be removed.
 */

namespace {

struct SSAConverter {
  SSAConverter(Vunit& unit,
               const VregSet& targets,
               boost::dynamic_bitset<>& blocksWithTargets,
               const jit::vector<Vlabel>& rpo,
               const PredVector& preds,
               MaybeVinstrId clobber)
    : doneBlocks{unit.blocks.size()}
    , blocksWithTargets{blocksWithTargets}
    , predecessors{preds}
    , targets{targets}
    , rpo{rpo}
    , unit{unit}
    , clobber{clobber}
  {
    assertx(blocksWithTargets.size() == unit.blocks.size());
  }

  // Create a new Vreg, recording that it replaced the original specified Vreg.
  Vreg makeVreg(Vreg pre) {
    auto const post = unit.makeReg();
    auto const DEBUG_ONLY result = newVregs.emplace(post, pre);
    assertx(result.second);
    return post;
  }

  // Record that the Vreg 'post' now represents the original Vreg 'pre' at
  // 'label'.
  void write(Vlabel label, Vreg pre, Vreg post) {
    assertx(post.isValid());
    defAtBlock.insert_or_assign({label, pre}, post);
  }

  // Record that we're recursing through 'label' for the purposes of
  // processing 'pre'. We use an invalid Vreg for this purpose.
  void writeRecursionMarker(Vlabel label, Vreg pre) {
    defAtBlock.insert_or_assign({label, pre}, Vreg{});
  }

  // Lookup what Vreg represents 'pre' at 'label'. If there's no information for
  // it at this label, chase backwards until we find a definition.
  Vreg read(Vlabel label, Vreg pre) {
    while (true) {
      // Look for a defintiion at this point
      auto const it = defAtBlock.find({label, pre});
      if (it != defAtBlock.end()) {
        // We found a definition. If it's valid, we can use it,
        // otherwise it's a recursion marker and we need to chase the
        // definition.
        if (it->second.isValid()) return it->second;
        return chaseDefinition(label, pre, true);
      }

      // No definition. If there's multiple predecessors we need to
      // chase the definition. Otherwise just move onto the (single)
      // predecessor.
      auto const& preds = predecessors[label];
      assertx(!preds.empty());
      if (preds.size() > 1) return chaseDefinition(label, pre, false);
      label = preds[0];
    }
  }

  // Follow a rewrite chain, returning the Vreg that 'r' should be substituted
  // with (if any).
  Vreg rewrite(Vreg r) {
    auto const it = rewrites.find(r);
    // If there's no entry for 'r', then there's no substitution.
    if (it == rewrites.end()) return r;
    auto const r2 = it->second;
    // Otherwise there's a rewrite. Resolve that rewrite recursively to get the
    // final Vreg.
    auto const r3 = rewrite(r2);
    // As an optimization, overwrite this rewrite to point directly at the final
    // value, so we don't need to search again.
    if (r3 != r2) rewrites.insert_or_assign(r, r3);
    return r3;
  }

  void addRewrite(Vreg src, Vreg dst) {
    assertx(dst.isValid());
    auto const DEBUG_ONLY result = rewrites.emplace(src, dst);
    assertx(result.second);
    // Resolving rewrites requires the second pass
    needsSecondPass = true;
  }

  VregList getInputsForPhi(const PredVector::value_type& preds, Vreg pre) {
    VregList phiInputs;
    phiInputs.reserve(preds.size());
    // Lookup the proper Vreg for 'pre' at each predecessor. These will be the
    // phi inputs.
    for (auto const& pred : preds) phiInputs.emplace_back(read(pred, pre));
    return phiInputs;
  }

  Vreg phiIsTrivial(const VregList& phiInputs, Vreg phi = Vreg{}) {
    // If all the inputs of the phi are the same (or the phi Vreg
    // itself if present), then it is trivial and does not have to exist.
    Vreg unique;
    for (auto input : phiInputs) {
      input = rewrite(input);
      assertx(input.isValid());
      if (input == phi || input == unique) continue;
      if (unique.isValid()) return Vreg{};
      unique = input;
    }
    assertx(unique.isValid());
    return unique;
  }

  // Turn an incomplete phi into a real phi.
  void fillPhi(Vlabel block,
               Vreg phi,
               Vreg pre,
               const VregList& phiInputs) {
    assertx(block != unit.entry);
    // Try to optimize away the phi. If successful, we're done.
    if (optimizePhi(phi, phiInputs)) return;
    // Otherwise we need a real one.
    auto const DEBUG_ONLY result =
      phis.emplace(phi, Phi{ block, std::move(phiInputs) });
    assertx(result.second);
  }

  // Return true if the Vreg representing a phi (along with the list of inputs
  // to that phi) can be optimized away to just one of the inputs.
  bool optimizePhi(Vreg phi, const VregList& inputs) {
    // If the phi is trivial, then it can be eliminated.
    auto const unique = phiIsTrivial(inputs, phi);
    if (!unique.isValid()) return false;
    // Erase this phi (if it already exists) and record that any existing users
    // of this phi Vreg should instead use the unique Vreg.
    phis.erase(phi);
    addRewrite(phi, unique);
    return true;
  }

  // Check if we've visited all of the predecessors of the given block and is
  // ready to be processed.
  bool blockIsReady(Vlabel label) const {
    auto const& preds = predecessors[label];
    return std::all_of(
      preds.begin(),
      preds.end(),
      [&](Vlabel b) { return doneBlocks[b]; }
    );
  }

  // Starting at 'label', recursively look backwards through
  // predecessors to find a definition of 'pre' to a new Vreg. This
  // should only be used if 'label' has multiple predecessors
  // (otherwise just walk up to the predecessor).
  Vreg chaseDefinition(Vlabel label, Vreg pre, bool recursion) {
    always_assert_flog(
      label != unit.entry,
      "Unable to find def for {}",
      show(pre)
    );

    auto const& preds = predecessors[label];
    assertx(preds.size() > 1);

    // If this block isn't ready to be processed (there's an unvisited
    // predecessor), then we can't go any further. Create an provisional phi
    // and use its Vreg (the phi might get optimized away later).
    if (!blockIsReady(label)) {
      assertx(!recursion);
      assertx(preds.size() > 1);
      auto const phi = makeVreg(pre);
      incompletePhis[label].emplace_back(
        IncompletePhi{ phi, pre }
      );
      write(label, pre, phi);
      return phi;
    }

    // There's multiple preds and they've all been processed.

    if (!recursion) {
      // We haven't seen this label already. Record that we're
      // visiting it (in case we loop back to it), then try to resolve
      // all of the predecessors (which will serve as inputs to the
      // phi).
      writeRecursionMarker(label, pre);
      auto const phiInputs = getInputsForPhi(preds, pre);

      // Check if there's a definition at this block. Normally there
      // shouldn't be (we wouldn't have called chaseDefinition in that
      // case), but we might have resolved it as part of getting the
      // phi inputs above. If there's a valid definition, just use
      // that.
      auto const it = defAtBlock.find({label, pre});
      assertx(it != defAtBlock.end());
      if (it->second.isValid()) return it->second;

      // Attempt to optimize away the phi
      auto const unique = phiIsTrivial(phiInputs);
      if (unique.isValid()) {
        write(label, pre, unique);
        return unique;
      }

      // Otherwise create the phi
      auto const phi = makeVreg(pre);
      fillPhi(label, phi, pre, phiInputs);
      write(label, pre, phi);
      return phi;
    } else {
      // We've recursed back into a label we've already
      // visited. There's no point in continuing any further (since
      // we'll just loop infinitely). Instead just create a phi.
      auto const phi = makeVreg(pre);
      write(label, pre, phi);
      auto const phiInputs = getInputsForPhi(preds, pre);
      fillPhi(label, phi, pre, phiInputs);
      return phi;
    }
  }

  // Actually generate phijmp/phidef instructions for the phis.
  void materializePhis() {
    // All phis should be completed by now.
    assertx(incompletePhis.empty());

    for (auto const& phi : phis) {
      auto const& preds = predecessors[phi.second.block];
      assertx(preds.size() == phi.second.inputs.size());
      assertx(phi.second.inputs.size() > 1);

      // First insert a phidef
      auto& block = unit.blocks[phi.second.block];
      auto foundExistingPhi = false;
      if (block.code.empty() || block.code.front().op != Vinstr::phidef) {
        // Insert a phidef instruction because one doesn't exist.
        assertx(phi.second.block != unit.entry);
        vmodify(
          unit, phi.second.block, 0,
          [&] (Vout& v) { v << phidef{unit.makeTuple({phi.first})}; return 0; }
        );
      } else {
        // Otherwise expand an existing one.
        auto const& phidef = block.code.front().phidef_;
        unit.tuples[phidef.defs].push_back(phi.first);
        if (clobber) block.code.front().id = *clobber;
        foundExistingPhi = true;
      }

      // We've modified this block
      blocksWithTargets[phi.second.block] = true;

      // Now insert phijmps at all the predecessors
      for (size_t i = 0; i < phi.second.inputs.size(); ++i) {
        // Also modified this block
        blocksWithTargets[preds[i]] = true;

        auto const input = rewrite(phi.second.inputs[i]);
        auto& pred = unit.blocks[preds[i]];
        assertx(!pred.code.empty());
        if (foundExistingPhi) {
          // If we found an existing phidef, there had better be an existing
          // phijmp too. Expand it with the new Vreg.
          assertx(pred.code.back().op == Vinstr::phijmp);
          auto const& phijmp = pred.code.back().phijmp_;
          unit.tuples[phijmp.uses].push_back(input);
          if (clobber) pred.code.back().id = *clobber;
        } else {
          // Otherwise there should be a jmp instruction here. We should have
          // split critical edges, so there can only be an unconditional jmp at
          // the predecessor. Turn the jmp into a phijmp.
          assertx(pred.code.back().op == Vinstr::jmp);
          auto const jmp = pred.code.back().jmp_;
          assertx(jmp.target == phi.second.block);
          pred.code.back().op = Vinstr::phijmp;
          pred.code.back().phijmp_ =
            phijmp{jmp.target, unit.makeTuple({input})};
          if (clobber) pred.code.back().id = *clobber;
        }
      }
    }
  }

  // First pass. Rewrite instructions with new Vregs (which may not be final)
  // and then finish any incomplete phis we created in the process.
  void firstPass() {
    for (auto const label : rpo) {
      auto& block = unit.blocks[label];

      // Only process blocks which we've been told can contain target
      // Vregs. In debug builds we'll examine every block, but assert
      // if we find a target Vreg in a non-marked block.
      if (debug || blocksWithTargets[label]) {
        for (auto& inst : block.code) {
          if (inst.op == Vinstr::ssaalias) {
            // ssaalias override. Any usage of the dest after this
            // point will use the source instead (which may have been
            // rewritten).
            assertx(targets[inst.ssaalias_.s]);
            assertx(targets[inst.ssaalias_.d]);
            assertx(blocksWithTargets[label]);
            inst.ssaalias_.s = read(label, inst.ssaalias_.s);
            if (clobber) inst.id = *clobber;
            write(label, inst.ssaalias_.d, inst.ssaalias_.s);
            // ssaalias instructions need the second pass
            needsSecondPass = true;
            continue;
          }

          auto changed = false;
          visitRegsMutable(
            unit, inst,
            [&](Vreg r) {
              if (!targets[r]) return r;
              assertx(blocksWithTargets[label]);
              // Lookup what this Vreg should be rewritten to
              auto const r2 = read(label, r);
              if (r != r2) changed = true;
              return r2;
            },
            [&](Vreg r) {
              if (!targets[r]) return r;
              assertx(blocksWithTargets[label]);
              // Create a new Vreg and record that any usage of 'r'
              // after this point should instead use 'r2'. If 'r'
              // hasn't already been used, re-use it (keeps total
              // number of Vregs down).
              if (!reused[r]) {
                reused.add(r);
                write(label, r, r);
                return r;
              } else {
                auto const r2 = makeVreg(r);
                changed = true;
                write(label, r, r2);
                return r2;
              }
            },
            // We can't rewrite RegSets, so you'd better not have asked us to.
            [&](RegSet s) {
              if (debug) s.forEach([&](Vreg r) { always_assert(!targets[r]); });
              return s;
            },
            [&](RegSet s) {
              if (debug) s.forEach([&](Vreg r) { always_assert(!targets[r]); });
              return s;
            }
          );
          if (changed && clobber) inst.id = *clobber;
        }
      }

      // Mark this block as processed and check if this has made any successor
      // blocks now ready. If so, convert any incomplete phis for the successor
      // block into a real phi.
      doneBlocks[label] = true;
      [&] {
        auto const& successors = succs(block);
        if (successors.size() != 1) return;
        auto const succ = successors[0];
        auto const& preds = predecessors[succ];
        assertx(!preds.empty());
        if (preds.size() < 2) return;
        if (!blockIsReady(succ)) return;
        auto const it = incompletePhis.find(succ);
        if (it == incompletePhis.end()) return;
        auto const incompletes = std::move(it->second);
        incompletePhis.erase(it);
        for (auto const& incomplete : incompletes) {
          auto const phiInputs = getInputsForPhi(preds, incomplete.variable);
          fillPhi(succ, incomplete.phi, incomplete.variable, phiInputs);
        }
      }();
    }
  }

  // Second pass. Remove any ssaalias instructions if present, and do a final
  // rewrite of the instructions to their final Vregs (which may have changed
  // due to phi optimizations).
  void secondPass() {
    assertx(incompletePhis.empty());

    // We only need to run the second pass if we have ssaalias
    // instructions (which must be removed), or if we had any rewrites
    // which now need to materialized.
    if (!needsSecondPass) return;

    for (auto const label : rpo) {
      // Any block with ssaalias instructions or rewrites should have
      // been set in blocksWithTargets, so only process those. Similar
      // to firstPass(), process every block in debug builds and
      // assert the block was marked.
      if (debug || blocksWithTargets[label]) {
        size_t i = 0;
        while (i < unit.blocks[label].code.size()) {
          if (unit.blocks[label].code[i].op == Vinstr::ssaalias) {
            assertx(blocksWithTargets[label]);
            vmodify(unit, label, i, [] (Vout&) { return 1; });
            continue;
          }

          auto& inst = unit.blocks[label].code[i];
          auto changed = false;
          visitRegsMutable(
            unit, inst,
            [&](Vreg r) {
              auto const r2 = rewrite(r);
              if (r != r2) changed = true;
              return r2;
            },
            [&](Vreg r) { return r; },
            [&](RegSet s) {
              if (debug) s.forEach([&](Vreg r) { always_assert(!targets[r]); });
              return s;
            },
            [&](RegSet s) {
              if (debug) s.forEach([&](Vreg r) { always_assert(!targets[r]); });
              return s;
            }
          );
          assertx(!changed || blocksWithTargets[label]);
          if (changed && clobber) inst.id = *clobber;

          ++i;
        }
      }
    }
  }

  void operator()() {
    if (targets.none()) return;
    assertx(!hasCriticalEdge());
    firstPass();
    secondPass();
    materializePhis();
    assertx(check(unit));
  }

  // Sanity check that no critical edges exist in the unit.
  bool hasCriticalEdge() const {
    for (auto const label : rpo) {
      auto const& block = unit.blocks[label];
      auto const successors = succs(block);
      if (successors.size() < 2) continue;
      for (auto const succ : successors) {
        if (predecessors[succ].size() > 1) return true;
      }
    }
    return false;
  }

  // We only need to run the second pass under certain
  // circumstances. This is set to true if we've detected we need to.
  bool needsSecondPass = false;
  // Record which target Vregs have already been used when rewriting
  // instructions. In many cases, there's only one rewrite for that
  // target, and re-using the original Vreg keeps the total number of
  // Vregs down.
  VregSet reused;

  // Represents a phi being built (we haven't visited all of the predecessors of
  // a node). Once we've visited all the blocks, any incomplete phis will be
  // converted into real phis.
  struct IncompletePhi {
    Vreg phi;
    Vreg variable;
  };
  jit::fast_map<Vlabel, jit::vector<IncompletePhi>> incompletePhis;

  // Represents all of the phis that need to be emitted. We only actually create
  // phijmp/phidef instructions at the end.
  struct Phi {
    Vlabel block;
    VregList inputs;
  };
  // Need stable iteration order.
  jit::flat_map<Vreg, Phi> phis;

  /*
   * Indicates that any usage of the first Vreg should be (eventually) rewritten
   * to the second Vreg. This lets us replace phis with another Vreg lazily
   * without having to immediately rewrite all of the phis usages. This map
   * forms a chain (IE, you keep resolving rewrites until you find a Vreg which
   * has no rewrite).
   */
  jit::fast_map<Vreg, Vreg> rewrites;

  // Indicates that a Vreg at a particular label should be replaced with another
  // Vreg (used when walking backwards for definitions).
  jit::fast_map<std::pair<Vlabel, Vreg>, Vreg> defAtBlock;
  boost::dynamic_bitset<> doneBlocks;

  // Map of new Vregs to the ones they replaced
  jit::fast_map<Vreg, Vreg> newVregs;

  boost::dynamic_bitset<>& blocksWithTargets;

  const PredVector& predecessors;
  const VregSet& targets;
  const jit::vector<Vlabel>& rpo;
  Vunit& unit;
  MaybeVinstrId clobber;
};

}

jit::fast_map<Vreg, Vreg> restoreSSA(
    Vunit& unit,
    const VregSet& targets,
    boost::dynamic_bitset<>& blocksWithTargets,
    const jit::vector<Vlabel>& rpo,
    const PredVector& preds,
    MaybeVinstrId clobber) {
  SSAConverter converter{unit, targets, blocksWithTargets, rpo, preds, clobber};
  converter();
  return std::move(converter.newVregs);
}

//////////////////////////////////////////////////////////////////////

static const Abi sf_abi {
  RegSet{}, RegSet{}, RegSet{}, RegSet{}, RegSet{},
  RegSet{RegSF{0}}
};


std::vector<Vreg> compute_sf_livein(const Vunit& unit,
                                    const jit::vector<Vlabel>& rpo,
                                    const PredVector& preds) {
  auto livein = std::vector<Vreg>(unit.blocks.size());

  auto workQ = dataflow_worklist<uint32_t>(unit.blocks.size());

  auto const po_to_block = [&] {
    auto blocks = rpo;
    std::reverse(blocks.begin(), blocks.end());
    return blocks;
  }();
  auto const block_to_po = [&] {
    auto order = std::vector<uint32_t>(unit.blocks.size());

    for (size_t po = 0; po < po_to_block.size(); ++po) {
      workQ.push(po);
      order[po_to_block[po]] = po;
    }
    return order;
  }();

  while (!workQ.empty()) {
    auto const b = po_to_block[workQ.pop()];
    auto const& block = unit.blocks[b];

    auto live = Vreg{};
    for (auto const s : succs(block)) {
      auto const other = livein[s];
      if (!other.isValid()) continue;

      assertx(!live.isValid() || live == other);
      live = other;
    }

    for (auto const& inst : boost::adaptors::reverse(block.code)) {
      if (inst.op == Vinstr::phidef) {
        // Skip phidef{}---if the def-tuple includes a VregSF, then it's
        // actually live on the incoming edge.
        continue;
      }

      RegSet implicit_uses, implicit_across, implicit_defs;
      if (inst.op == Vinstr::vcall || inst.op == Vinstr::vinvoke) {
        // getEffects() would assert since these haven't been lowered yet.
        implicit_defs |= RegSF{0};
      } else {
        getEffects(sf_abi, inst, implicit_uses, implicit_across, implicit_defs);
      }

      auto const visit_def = [&] (Vreg, Width w) {
        if (w == Width::Flags) live = Vreg{};
      };
      auto const visit_use = [&] (Vreg r, Width w) {
        if (w == Width::Flags) live = r;
      };

      // Determine liveness at `inst'.  We rely on the assumption that VregSF
      // lifetimes can never overlap, which the checkSF() pass provides.
      visitDefs(unit, inst, visit_def);
      visit(unit, implicit_defs, visit_def);
      visitUses(unit, inst, visit_use);
      visit(unit, implicit_uses, visit_use);
    }

    if (live.isValid()) {
      if (livein[b].isValid()) {
        assertx(live == livein[b]);
      } else {
        livein[b] = live;
        for (auto p : preds[b]) workQ.push(block_to_po[p]);
      }
    }
  }

  return livein;
}

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * VregSF-renaming visitor.
 */
struct FlagsVisitor {
  explicit FlagsVisitor(const jit::fast_set<unsigned>& sf_renames)
    : m_sf_renames(sf_renames) {}

  template<class F> void imm(const F&) {}
  template<class R> void across(R&) {}
  void across(VregSF&) = delete;

  template<class R> void def(R&) {}
  template<class R, class H> void defHint(R& r, H) { def(r); }
  template<class R> void use(R&) {}
  template<class R, class H> void useHint(R& r, H) { use(r); }

  void use(VregSF& r) { if (m_sf_renames.count(r)) r = RegSF{0}; }
  void def(VregSF& r) { use(r); }

 private:
  jit::fast_set<unsigned> m_sf_renames;
};

}

void rename_sf_regs(Vunit& unit, const jit::fast_set<unsigned>& sf_renames) {
  auto visitor = FlagsVisitor(sf_renames);

  for (auto& blk : unit.blocks) {
    for (auto& inst : blk.code) {
      visitOperands(inst, visitor);
    }
  }
}

//////////////////////////////////////////////////////////////////////

jit::vector<LiveSet> computeLiveness(const Vunit& unit,
                                     const Abi& abi,
                                     const jit::vector<Vlabel>& blocks) {
  auto livein = jit::vector<LiveSet>{unit.blocks.size()};
  auto const preds = computePreds(unit);

  auto blockPO = jit::vector<uint32_t>(unit.blocks.size());
  auto revBlocks = blocks;
  std::reverse(begin(revBlocks), end(revBlocks));

  auto wl = dataflow_worklist<uint32_t>(revBlocks.size());

  for (unsigned po = 0; po < revBlocks.size(); po++) {
    wl.push(po);
    blockPO[revBlocks[po]] = po;
  }

  while (!wl.empty()) {
    auto b = revBlocks[wl.pop()];
    auto& block = unit.blocks[b];

    // start with the union of the successor blocks
    LiveSet live(unit.next_vr);
    for (auto s : succs(block)) {
      if (!livein[s].empty()) live |= livein[s];
    }

    // and now go through the instructions in the block in reverse order
    for (auto const& inst : boost::adaptors::reverse(block.code)) {
      RegSet implicit_uses, implicit_across, implicit_defs;
      getEffects(abi, inst, implicit_uses, implicit_across, implicit_defs);

      auto const vsf = Vreg{RegSF{0}};

      auto const dvisit = [&] (Vreg r, Width w) {
        live.reset(w == Width::Flags ? vsf : r);
      };
      auto const uvisit = [&] (Vreg r, Width w) {
        live.set(w == Width::Flags ? vsf : r);
      };

      visitDefs(unit, inst, dvisit);
      visit(unit, implicit_defs, dvisit);

      visitUses(unit, inst, uvisit);
      visit(unit, implicit_uses, uvisit);
      visit(unit, implicit_across, uvisit);
    }

    if (live != livein[b]) {
      livein[b] = live;
      for (auto p : preds[b]) {
        wl.push(blockPO[p]);
      }
    }
  }

  return livein;
}

//////////////////////////////////////////////////////////////////////

}}
