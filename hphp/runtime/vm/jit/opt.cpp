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

#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/dce.h"

#include "hphp/util/trace.h"

#include <boost/dynamic_bitset.hpp>

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

enum class DCE { None, Minimal, Full };

template<class PassFN>
bool doPass(IRUnit& unit, PassFN fn, DCE dce) {
  auto result = ArrayData::call_helper(fn, unit);
  switch (dce) {
  case DCE::Minimal:  mandatoryDCE(unit); break;
  case DCE::Full:     fullDCE(unit); // fallthrough
  case DCE::None:     assertx(checkEverything(unit)); break;
  }
  return result;
}

void removeExitPlaceholders(IRUnit& unit) {
  for (auto& block : rpoSortCfg(unit)) {
    if (block->back().is(ExitPlaceholder)) {
      unit.replace(&block->back(), Jmp, block->next());
    }
  }
}

void simplifyOrdStrIdx(IRUnit& unit) {
  auto blocks = poSortCfg(unit);

  // map from the dests of StringGet instructions, to the list
  // of uses.
  jit::hash_map<SSATmp*, std::vector<IRInstruction*>> strGets;
  for (auto& block : blocks) {
    if (block->back().is(StringGet)) {
      strGets[block->back().dst()] = std::vector<IRInstruction*>();
    }
  }

  if (strGets.empty()) return;

  for (auto& block : blocks) {
    for (auto& inst : *block) {
      for (auto& src : inst.srcs()) {
        auto it = strGets.find(src);
        if (it == strGets.end()) continue;
        if (!inst.is(OrdStr)) {
          // If we find a non-OrdStr use, we're not going to do the
          // optimization, so clear the list, and make sure this one
          // is first, so we can easily skip without searching the
          // whole vector.
          it->second.clear();
        }
        it->second.push_back(&inst);
      }
    }
  }

  for (auto& strGet : strGets) {
    if (strGet.second.empty()) continue;
    if (!strGet.second[0]->is(OrdStr)) {
      // There was at least one non-OrdStr use
      continue;
    }

    // Change all the OrdStr uses into Movs
    for (auto inst : strGet.second) {
      unit.replace(inst, Mov, strGet.first);
    }

    // Change the StringGet to an OrdStrIdx
    // so
    //   t3 = StringGet(t1, t2)
    //   t4 = OrdStr(t3)
    // becomes
    //   t3 = OrdStrIdx(t1, t2)
    //   t4 = Mov(t3)
    auto inst = strGet.first->inst();
    auto next = inst->next();
    unit.replace(inst, OrdStrIdx, inst->taken(), inst->src(0), inst->src(1));
    inst->setNext(next);
  }

  reflowTypes(unit);
  printUnit(6, unit, " after simplifyOrdStrIdx ");
}

//////////////////////////////////////////////////////////////////////

/*
 * This pass fixes the hints in the blocks to make sure that a block is not
 * marked as hotter (i.e. more likely to execute) than any of its predecessors.
 */
void fixBlockHints(IRUnit& unit) {
  TRACE_SET_MOD(hhir_fixhint);
  bool changed = false;
  auto blocks = rpoSortCfg(unit);
  do {
    changed = false;
    for (auto& block : blocks) {
      // The unit entry has no predecessor but we want to keep it in the main
      // area code.
      if (block == unit.entry()) continue;

      uint8_t maxPredHint = static_cast<uint8_t>(Block::Hint::Unused);
      for (auto& inEdge : block->preds()) {
        const auto& pred = inEdge.from();
        auto predHint = static_cast<uint8_t>(pred->hint());
        maxPredHint = std::max(maxPredHint, predHint);
      }

      if (static_cast<uint8_t>(block->hint()) > maxPredHint) {
        const auto newHint = static_cast<Block::Hint>(maxPredHint);
        FTRACE(3, "fixBlockHints: changing B{} from {} to {}\n", block->id(),
               blockHintName(block->hint()), blockHintName(newHint));
        block->setHint(newHint);
        changed = true;
      }
    }
  } while (changed);
}

//////////////////////////////////////////////////////////////////////

}

void optimize(IRUnit& unit, TransKind kind) {
  Timer timer(Timer::optimize, unit.logEntry().get_pointer());

  assertx(checkEverything(unit));

  fullDCE(unit);
  printUnit(6, unit, " after initial DCE ");
  assertx(checkEverything(unit));

  if (RuntimeOption::EvalHHIRPredictionOpts) {
    doPass(unit, optimizePredictions, DCE::None);
  }

  if (RuntimeOption::EvalHHIRSimplification) {
    doPass(unit, simplifyPass, DCE::Full);
    doPass(unit, cleanCfg, DCE::None);
  }

  if (RuntimeOption::EvalHHIRGlobalValueNumbering) {
    doPass(unit, gvn, DCE::Full);
  }

  while (true) {
    if (kind != TransKind::Profile && RuntimeOption::EvalHHIRMemoryOpts) {
      doPass(unit, optimizeLoads, DCE::Full);
      printUnit(6, unit, " after optimizeLoads ");
    }

    if (kind != TransKind::Profile && RuntimeOption::EvalHHIRMemoryOpts) {
      doPass(unit, optimizeStores, DCE::Full);
      printUnit(6, unit, " after optimizeStores ");
    }

    if (RuntimeOption::EvalHHIRPartialInlineFrameOpts) {
      doPass(unit, optimizeInlineReturns, DCE::Full);
      printUnit(6, unit, " after optimizeInlineReturns ");
    }

    if (!doPass(unit, optimizePhis, DCE::Full)) break;
    doPass(unit, cleanCfg, DCE::None);
    printUnit(6, unit, " after optimizePhis ");
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRRefcountOpts) {
    doPass(unit, optimizeRefcounts, DCE::Full);
    printUnit(6, unit, " after optimizeRefCounts ");
  }

  if (RuntimeOption::EvalHHIRLICM && cfgHasLoop(unit) &&
      kind != TransKind::Profile) {
    doPass(unit, optimizeLoopInvariantCode, DCE::Minimal);
    printUnit(6, unit, " after optimizeLoopInvariantCode ");
  }

  doPass(unit, simplifyOrdStrIdx, DCE::Minimal);

  doPass(unit, removeExitPlaceholders, DCE::Full);

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    doPass(unit, insertAsserts, DCE::None);
  }

  // Perform final cleanup passes to collapse any critical edges that were
  // split, and simplify our instructions before shipping off to codegen.
  doPass(unit, cleanCfg, DCE::None);

  if (kind != TransKind::Profile &&
      RuntimeOption::EvalHHIRGlobalValueNumbering) {
    doPass(unit, gvn, DCE::Full);
  }

  if (kind != TransKind::Profile && RuntimeOption::EvalHHIRSimplification) {
    doPass(unit, simplifyPass, DCE::Full);
  }
  doPass(unit, fixBlockHints, DCE::None);

  printUnit(6, unit, " after optimize ");
}

//////////////////////////////////////////////////////////////////////

}}
