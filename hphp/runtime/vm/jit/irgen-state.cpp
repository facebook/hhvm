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
  return BCMarker { ctx.srcKey(), ctx.initSpOffset, ctx.transID };
}

}

//////////////////////////////////////////////////////////////////////

HTS::HTS(TransContext context)
  : context(context)
  , unit(context)
  , irb(new IRBuilder(unit, initial_marker(context)))
  , bcStateStack { context.srcKey() }
{
  irgen::updateMarker(*this);
  auto const frame = irgen::gen(*this, DefFP);
  irgen::gen(*this, DefSP, StackOffset { context.initSpOffset }, frame);
}

//////////////////////////////////////////////////////////////////////

std::string show(const HTS& hts) {
  std::ostringstream out;
  auto header = [&](const std::string& str) {
    out << folly::format("+{:-^82}+\n", str);
  };

  const int32_t frameCells = irgen::resumed(hts)
    ? 0
    : irgen::curFunc(hts)->numSlotsInFrame();
  const int32_t stackDepth =
    hts.irb->spOffset() + hts.irb->evalStack().size()
    - hts.irb->stackDeficit() - frameCells;
  auto spOffset = stackDepth;
  auto elem = [&](const std::string& str) {
    out << folly::format("| {:<80} |\n",
                         folly::format("{:>2}: {}",
                                       stackDepth - spOffset, str));
    assert(spOffset > 0);
    --spOffset;
  };

  auto fpi = irgen::curFunc(hts)->findFPI(irgen::bcOff(hts));
  auto checkFpi = [&]() {
    if (fpi && spOffset + frameCells == fpi->m_fpOff) {
      auto fpushOff = fpi->m_fpushOff;
      auto after = fpushOff + instrLen((Op*)irgen::curUnit(hts)->at(fpushOff));
      std::ostringstream msg;
      msg << "ActRec from ";
      irgen::curUnit(hts)->prettyPrint(
        msg,
        Unit::PrintOpts().range(fpushOff, after)
                         .noLineNumbers()
                         .indent(0)
                         .noFuncs()
      );
      auto msgStr = msg.str();
      assert(msgStr.back() == '\n');
      msgStr.erase(msgStr.size() - 1);
      for (unsigned i = 0; i < kNumActRecCells; ++i) elem(msgStr);
      fpi = fpi->m_parentIndex != -1
        ? &irgen::curFunc(hts)->fpitab()[fpi->m_parentIndex]
        : nullptr;
      return true;
    }
    return false;
  };

  header(folly::format(" {} stack element(s); m_evalStack: ",
                       stackDepth).str());
  for (unsigned i = 0; i < hts.irb->evalStack().size(); ++i) {
    while (checkFpi());
    auto const value = irgen::top(const_cast<HTS&>(hts), i, DataTypeGeneric);
    elem(value->inst()->toString());
  }

  header(" in-memory ");
  for (unsigned i = hts.irb->stackDeficit(); spOffset > 0; ) {
    assert(i < irgen::curFunc(hts)->maxStackCells());
    if (checkFpi()) {
      i += kNumActRecCells;
      continue;
    }

    auto stkVal = getStackValue(irgen::sp(hts), i);
    std::ostringstream elemStr;
    if (stkVal.knownType == Type::StackElem) elem("unknown");
    else if (stkVal.value) elem(stkVal.value->inst()->toString());
    else elem(stkVal.knownType.toString());

    ++i;
  }
  header("");
  out << "\n";

  header(folly::format(" {} local(s) ", irgen::curFunc(hts)->numLocals()).str());
  for (unsigned i = 0; i < irgen::curFunc(hts)->numLocals(); ++i) {
    auto const localValue = hts.irb->localValue(i, DataTypeGeneric);
    auto const localTy = localValue ? localValue->type()
                                    : hts.irb->localType(i, DataTypeGeneric);
    auto str = localValue ? localValue->inst()->toString()
                          : localTy.toString();
    if (localTy.isBoxed()) {
      auto const pred = hts.irb->predictedInnerType(i);
      if (!pred.subtypeOf(Type::Bottom)) {
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

