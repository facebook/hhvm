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

#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/irlower.h"

#include "hphp/runtime/base/runtime-option.h"

namespace HPHP { namespace jit {
namespace {

Vcost instrSize(const Vunit& u, AreaIndex area, Vinstr inst) {
  auto const tuple_size = [&] (Vtuple t) -> int {
    return u.tuples[t].size();
  };

  switch (inst.op) {
  case Vinstr::phidef:
    return {tuple_size(inst.get<phidef>().defs), false};
  case Vinstr::phijmp:
    return {tuple_size(inst.get<phijmp>().uses) + 1, false};
  case Vinstr::copyargs:
    return {tuple_size(inst.get<copyargs>().s), false};
  case Vinstr::copy2:
    return {2, false};
  case Vinstr::bindjmp:
  case Vinstr::fallback:
    return {1, area == AreaIndex::Main};

  default:
    return {1, false};
  }
}
}

Vcost computeVunitCost(const Vunit& unit) {
  int cost = 0;
  bool incomplete = false;

  for (auto const b : sortBlocks(unit)) {
    auto const& block = unit.blocks[b];

    auto const factor = [&] {
      switch (block.area_idx) {
        case AreaIndex::Main:
          return RuntimeOption::EvalHHIRInliningCostFactorMain;
        case AreaIndex::Cold:
          return RuntimeOption::EvalHHIRInliningCostFactorCold;
        case AreaIndex::Frozen:
          return RuntimeOption::EvalHHIRInliningCostFactorFrozen;
      }
      not_reached();
    }();

    for (auto const& instr : block.code) {
      auto info = instrSize(unit, block.area_idx, instr);
      cost += info.cost * factor;
      incomplete |= info.incomplete;
    }
  }

  return {cost, incomplete};
}

}}
