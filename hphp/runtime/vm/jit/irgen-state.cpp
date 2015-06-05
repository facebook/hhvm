/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

namespace {

BCMarker initial_marker(TransContext ctx) {
  return BCMarker { ctx.srcKey(), ctx.initSpOffset, ctx.transID, nullptr };
}

}

//////////////////////////////////////////////////////////////////////

IRGS::IRGS(TransContext context, TransFlags flags)
  : context(context)
  , transFlags(flags)
  , unit(context)
  , irb(new IRBuilder(unit, initial_marker(context)))
  , bcStateStack { context.srcKey() }
{
  irgen::updateMarker(*this);
  auto const frame = irgen::gen(*this, DefFP);
  irgen::gen(*this, DefSP, FPInvOffsetData { context.initSpOffset }, frame);
}

//////////////////////////////////////////////////////////////////////

std::string show(const IRGS& irgs) {
  std::ostringstream out;
  auto header = [&](const std::string& str) {
    out << folly::format("+{:-^82}+\n", str);
  };

  const int32_t frameCells = irgen::resumed(irgs)
    ? 0
    : irgen::curFunc(irgs)->numSlotsInFrame();
  auto const stackDepth = irgs.irb->syncedSpLevel().offset +
      safe_cast<int32_t>(irgs.irb->evalStack().size()) -
      safe_cast<int32_t>(irgs.irb->stackDeficit()) - frameCells;
  assertx(stackDepth >= 0);
  auto spOffset = stackDepth;
  auto elem = [&](const std::string& str) {
    out << folly::format("| {:<80} |\n",
                         folly::format("{:>2}: {}",
                                       stackDepth - spOffset, str));
    assertx(spOffset > 0);
    --spOffset;
  };

  auto fpi = irgen::curFunc(irgs)->findFPI(irgen::bcOff(irgs));
  auto checkFpi = [&]() {
    if (fpi && spOffset + frameCells == fpi->m_fpOff) {
      auto fpushOff = fpi->m_fpushOff;
      auto after = fpushOff + instrLen((Op*)irgen::curUnit(irgs)->at(fpushOff));
      std::ostringstream msg;
      msg << "ActRec from ";
      irgen::curUnit(irgs)->prettyPrint(
        msg,
        Unit::PrintOpts().range(fpushOff, after)
                         .noLineNumbers()
                         .indent(0)
                         .noFuncs()
      );
      auto msgStr = msg.str();
      assertx(msgStr.back() == '\n');
      msgStr.erase(msgStr.size() - 1);
      for (unsigned i = 0; i < kNumActRecCells; ++i) elem(msgStr);
      fpi = fpi->m_parentIndex != -1
        ? &irgen::curFunc(irgs)->fpitab()[fpi->m_parentIndex]
        : nullptr;
      return true;
    }
    return false;
  };

  header(folly::format(" {} stack element(s); m_evalStack: ",
                       stackDepth).str());
  for (auto i = 0; i < irgs.irb->evalStack().size(); ++i) {
    while (checkFpi());
    auto const value = irgen::top(const_cast<IRGS&>(irgs),
                                  BCSPOffset{i}, DataTypeGeneric);
    elem(value->inst()->toString());
  }

  header(" in-memory ");
  for (auto i = irgs.irb->evalStack().size(); spOffset > 0; ) {
    assertx(i < irgen::curFunc(irgs)->maxStackCells());
    if (checkFpi()) {
      i += kNumActRecCells;
      continue;
    }

    auto const stkTy = irgs.irb->stackType(
      irgen::offsetFromIRSP(irgs, BCSPOffset{i}),
      DataTypeGeneric
    );
    auto const stkVal = irgs.irb->stackValue(
      irgen::offsetFromIRSP(irgs, BCSPOffset{i}),
      DataTypeGeneric
    );

    std::ostringstream elemStr;
    if (stkTy == TStkElem) {
      elem("unknown");
    } else if (stkVal) {
      elem(stkVal->inst()->toString());
    } else {
      elem(stkTy.toString());
    }

    ++i;
  }
  header("");
  out << "\n";

  header(folly::format(" {} local(s) ",
                       irgen::curFunc(irgs)->numLocals()).str());
  for (unsigned i = 0; i < irgen::curFunc(irgs)->numLocals(); ++i) {
    auto const localValue = irgs.irb->localValue(i, DataTypeGeneric);
    auto const localTy = localValue ? localValue->type()
                                    : irgs.irb->localType(i, DataTypeGeneric);
    auto str = localValue ? localValue->inst()->toString()
                          : localTy.toString();
    if (localTy <= TBoxedCell) {
      auto const pred = irgs.irb->predictedInnerType(i);
      if (pred != TBottom) {
        str += folly::sformat(" (predict inner: {})", pred.toString());
      }
    }
    out << folly::format("| {:<80} |\n",
                         folly::format("{:>2}: {}", i, str));
  }
  header("");
  return out.str();
}

//////////////////////////////////////////////////////////////////////

}}
