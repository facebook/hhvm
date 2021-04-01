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
#include "hphp/runtime/vm/jit/irgen-state.h"

#include <sstream>

#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/resumable.h"

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

IRGS::IRGS(IRUnit& unit, const RegionDesc* region, int32_t budgetBCInstrs,
           TranslateRetryContext* retryContext)
  : context(unit.context())
  , region(region)
  , unit(unit)
  , irb(new IRBuilder(unit, context.initSrcKey.func()))
  , bcState(context.initSrcKey)
  , budgetBCInstrs(budgetBCInstrs)
  , retryContext(retryContext)
{
  updateMarker(*this);
}

//////////////////////////////////////////////////////////////////////

std::string show(const IRGS& irgs) {
  std::ostringstream out;
  auto header = [&](const std::string& str) {
    out << folly::format("+{:-^102}+\n", str);
  };

  const int32_t frameCells = resumeMode(irgs) != ResumeMode::None
    ? 0
    : curFunc(irgs)->numSlotsInFrame();
  auto const stackDepth = irgs.irb->fs().bcSPOff().offset - frameCells;
  assertx(stackDepth >= 0);
  auto spOffset = stackDepth;
  auto elem = [&](const std::string& str) {
    out << folly::format("| {:<100} |\n",
                         folly::format("{:>2}: {}",
                                       stackDepth - spOffset, str));
    assertx(spOffset > 0);
    --spOffset;
  };

  header(folly::format(" {} stack element(s): ",
                       stackDepth).str());
  assertx(spOffset <= curFunc(irgs)->maxStackCells());

  for (auto i = 0; i < stackDepth; ) {
    auto const spRel = offsetFromIRSP(irgs, BCSPRelOffset{i});
    auto const stkTy  = irgs.irb->stack(spRel, DataTypeGeneric).type;
    auto const stkVal = irgs.irb->stack(spRel, DataTypeGeneric).value;

    std::string elemStr;
    if (stkTy == TCell) {
      elemStr = "unknown";
    } else if (stkVal) {
      elemStr = stkVal->inst()->toString();
    } else {
      elemStr = stkTy.toString();
    }

    auto const irSPRel = BCSPRelOffset{i}
      .to<FPInvOffset>(irgs.irb->fs().bcSPOff());
    auto const predicted = predictedType(irgs, Location::Stack { irSPRel });

    if (predicted < stkTy) {
      elemStr += folly::sformat(" (predict: {})", predicted);
    }
    elem(elemStr);
    ++i;
  }
  header("");
  out << "\n";

  header(folly::format(" {} local(s) ",
                       curFunc(irgs)->numLocals()).str());
  for (unsigned i = 0; i < curFunc(irgs)->numLocals(); ++i) {
    auto const localValue = irgs.irb->local(i, DataTypeGeneric).value;
    auto const localTy = localValue ? localValue->type()
                                    : irgs.irb->local(i, DataTypeGeneric).type;
    auto str = localValue ? localValue->inst()->toString()
                          : localTy.toString();
    auto const predicted = irgs.irb->fs().local(i).predictedType;
    if (predicted < localTy) str += folly::sformat(" (predict: {})", predicted);

    out << folly::format("| {:<100} |\n",
                         folly::format("{:>2}: {}", i, str));
  }
  header("");
  return out.str();
}

//////////////////////////////////////////////////////////////////////

}}}
