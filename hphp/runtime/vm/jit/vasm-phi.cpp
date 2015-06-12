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

#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/trace.h"

TRACE_SET_MOD(vasm_phi);

namespace HPHP { namespace jit {

/*
 * optimizePhis() performs limited tail duplication in a few specific
 * situations where the resulting code is strictly better than before. It also
 * performs some branch/setcc fusion as it goes. This is to keep things simpler
 * than full-fledged tail duplication, and because the analysis code already
 * had to do the hard work of detecting that branch fusion was possible (to
 * decide when to perform the duplication). These situations are best explained
 * with examples:
 *
 *   B62          main, B32
 *          setcc      NE, %91 => %92
 *          phijmp     {%92}, B64
 *
 *   B63          cold, B32
 *          setcc      NE, %95 => %94
 *          phijmp     {%94}, B64
 *
 *   B64          main, B62, B63
 *          phidef     => {%69}
 *          testb      %69, %69 => %96
 *          setcc      E, %96 => %70
 *
 * Notice that all preds of B64 are providing the dest of a setcc, then we use
 * the dest of the phidef in B64 to test and set another boolean. We can hoist
 * the setcc into each predecessor and combine the condition codes, ending up
 * with code like this:
 *
 *   B62          main, B32
 *          setcc      E, %91 => %92
 *          phijmp     {%92}, B64
 *
 *   B63          cold, B32
 *          setcc      E, %95 => %94
 *          phijmp     {%94}, B64
 *
 *   B64          main, B62, B63
 *          phidef     => {%70}
 *
 * The previous dest of the merged setcc, %70, is now directly defined by the
 * phidef, and both paths have a single setcc rather than setcc; testb; setcc.
 *
 * Example 2:
 *
 *   B3           main, B5
 *          phijmp     {%69(0b)}, B4
 *
 *   B6           main, B5
 *          setcc      G, %73 => %68
 *          phijmp     {%68}, B4
 *
 *   B4           main, B6, B3
 *          phidef     => {%70}
 *          testb      %70, %70 => %74
 *          jcc        E, %74, B7, else B8
 *
 * B3 provides a constant to B4 and B6 provides the dest of a
 * setcc. Duplicating the testb/jcc from B4 into these allows branch fusion to
 * greatly simplify everything, eliminating the phi and saving a test and jump.
 *
 *   B3           main, B5
 *          jmp        B7
 *
 *   B6           main, B5
 *          jcc        LE, %73, B7, else B8
 */
void optimizePhis(Vunit& unit) {
  auto changed = false;
  VpassTracer t{&unit, TRACEMOD, "optimizePhis", &changed};

  // Make sure constants for true and false exist before doing anything, so we
  // can use them without having to create more Vregs.
  unit.makeConst(true);
  unit.makeConst(false);

  jit::vector<uint32_t> useCounts(unit.next_vr);
  PostorderWalker{unit}.dfs([&](Vlabel label) {
    for (auto const& inst : unit.blocks[label].code) {
      visitUses(unit, inst, [&](Vreg src) { ++useCounts[src]; });
    }
  });

  auto const preds = computePreds(unit);
  PostorderWalker{unit}.dfs([&](Vlabel label) {
    auto& block = unit.blocks[label].code;

    auto it = block.begin();
    if (it->op != Vinstr::phidef) return;
    auto& phidef = it->phidef_;
    auto& defs = unit.tuples[phidef.defs];
    if (defs.size() != 1) {
      // We could in theory support these if they come up but it'd be messy.
      FTRACE(1, "Bailing on multi-def phi in {}: {}\n",
             label, show(unit, phidef));
      return;
    }
    if (useCounts[defs[0]] > 2) return;

    ++it;
    if (it->op != Vinstr::testb) return;
    auto& testb = it->testb_;
    if (testb.s0 != defs[0] || testb.s1 != defs[0]) return;
    if (useCounts[testb.sf] > 1) return;

    ++it;
    ConditionCode phi_cc;
    if (it->op == Vinstr::jcc) {
      auto& jcc = it->jcc_;
      if (jcc.sf != testb.sf) return;
      phi_cc = jcc.cc;
    } else if (it->op == Vinstr::setcc) {
      auto& setcc = it->setcc_;
      if (setcc.sf != testb.sf) return;
      phi_cc = setcc.cc;
    } else {
      return;
    }
    if (phi_cc != CC_Z && phi_cc != CC_NZ) return;
    auto const is_jcc = it->op == Vinstr::jcc;

    FTRACE(2, "Inspecting preds of {}: {}\n", label, show(unit, phidef));
    for (auto pred : preds[label]) {
      auto it = unit.blocks[pred].code.end();
      --it;
      if (it->op != Vinstr::phijmp) return;
      auto const use = unit.tuples[it->phijmp_.uses][0];
      if (!unit.regToConst.count(use)) {
        if (useCounts[use] > 1) return;
        if (it == unit.blocks[pred].code.begin()) return;
        --it;
        if (it->op != Vinstr::setcc || use != it->setcc_.d) return;
      }
    }

    // If we got here, we know that every pred of the current block is either
    // providing a constant or the dest of a setcc.
    FTRACE(1, "Optimizing preds of {}:\n", label);
    for (auto pred : preds[label]) {
      auto& predCode = unit.blocks[pred].code;
      auto& term = predCode.back();
      auto const use = unit.tuples[term.phijmp_.uses][0];
      auto cIt = unit.regToConst.find(use);
      if (cIt != unit.regToConst.end()) {
        if (is_jcc) {
          auto const taken = (phi_cc == CC_NZ) == bool(cIt->second.val);
          auto& jcc = it->jcc_;
          term = jmp{jcc.targets[taken]};
        } else if (phi_cc == CC_Z) {
          auto const newSrc = !cIt->second.val;
          term.phijmp_.uses = unit.makeTuple({unit.makeConst(newSrc)});
        }
      } else {
        auto& setcc = predCode[predCode.size() - 2].setcc_;
        auto const cc = phi_cc == CC_Z ? ccNegate(setcc.cc) : setcc.cc;
        if (is_jcc) {
          auto& jcc = it->jcc_;
          term = jit::jcc{cc, setcc.sf, {jcc.targets[0], jcc.targets[1]}};
          // The setcc should now be dead and will be cleaned up by dce.
        } else {
          setcc.cc = cc;
        }
      }
    }

    if (!is_jcc) {
      // Shuffle things around so the Vreg previously defined by the setcc is
      // now defined by the phidef. We know the setcc was the only use of the
      // phidef's old dest, so this is safe.
      auto const setcc_def = block[2].setcc_.d;
      block.erase(block.begin() + 1 /* testb */ ,
                  block.begin() + 3 /* after setcc */);
      block[0].phidef_.defs = unit.makeTuple({setcc_def});
    }

    changed = true;
  });
}

}}
