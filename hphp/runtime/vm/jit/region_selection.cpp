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
#include "hphp/runtime/vm/jit/region_selection.h"

#include <algorithm>
#include <boost/range/adaptors.hpp>

#include "folly/Memory.h"
#include "folly/Conv.h"

#include "hphp/util/assertions.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(region);

//////////////////////////////////////////////////////////////////////

extern RegionDescPtr regionMethod(const RegionContext&);
extern RegionDescPtr regionOneBC(const RegionContext&);

//////////////////////////////////////////////////////////////////////

namespace {

enum class RegionMode {
  None,
  OneBC,
  Method,
  Tracelet,
};

RegionMode regionMode() {
  auto& s = RuntimeOption::EvalJitRegionSelector;
  if (s == "")       return RegionMode::None;
  if (s == "onebc")  return RegionMode::OneBC;
  if (s == "method") return RegionMode::Method;
  if (s == "tracelet") return RegionMode::Tracelet;
  FTRACE(1, "unknown region mode {}: using none\n", s);
  if (debug) abort();
  return RegionMode::None;
}

}

//////////////////////////////////////////////////////////////////////

void RegionDesc::Block::addPredicted(SrcKey sk, TypePred pred) {
  assert(pred.type.subtypeOf(Type::Gen | Type::Cls));
  auto const newElem = std::make_pair(sk, pred);
  auto const it = std::upper_bound(m_predTypes.begin(), m_predTypes.end(),
    newElem, typePredListCmp);
  m_predTypes.insert(it, newElem);
  checkInvariants();
}

/*
 * Check invariants on a RegionDesc::Block.
 *
 * 1. Single entry, single exit (aside from exceptions).  I.e. no
 *    non-fallthrough instructions mid-block and no control flow (not
 *    counting calls as control flow).
 *
 * 2. The type prediction list is ordered by increasing SrcKey.
 *
 * 3. Each prediction in the type prediction list is inside the range
 *    of this block.
 *
 * 4. Each local id referred to in the type prediction list is valid.
 *
 * 5. (Unchecked) each stack offset in the type prediction list is
 *    valid.
 */
void RegionDesc::Block::checkInvariants() const {
  smart::vector<SrcKey> keysInRange;
  keysInRange.reserve(length());
  keysInRange.push_back(start());
  for (int i = 1; i < length(); ++i) {
    if (i != length() - 1) {
      auto const pc = unit()->at(keysInRange.back().offset());
      if (instrFlags(*pc) & TF) {
        FTRACE(1, "Bad block: {}\n", show(*this));
        assert(!"Block may not contain non-fallthrough instruction unless "
                "they are last");
      }
      if (instrIsNonCallControlFlow(*pc)) {
        FTRACE(1, "Bad block: {}\n", show(*this));
        assert(!"Block may not contain control flow instructions unless "
                "they are last");
      }
    }
    keysInRange.push_back(keysInRange.back().advanced(unit()));
  }
  assert(keysInRange.size() == length());

  assert(std::is_sorted(m_predTypes.begin(), m_predTypes.end(),
                        typePredListCmp));
  assert(std::is_sorted(keysInRange.begin(), keysInRange.end()));

  auto rangeIt = keysInRange.begin();
  for (auto& tpred : m_predTypes) {
    while (rangeIt != keysInRange.end() && *rangeIt < tpred.first) ++rangeIt;
    assert(rangeIt != keysInRange.end() && tpred.first == *rangeIt &&
           "RegionDesc::Block contained an out-of-range prediction");

    auto& loc = tpred.second.location;
    switch (loc.tag()) {
    case Location::Tag::Local: assert(loc.localId() < m_func->numLocals());
                               break;
    case Location::Tag::Stack: // Unchecked
                               break;
    }
  }
}

bool RegionDesc::Block::typePredListCmp(TypePredList::const_reference a,
                                        TypePredList::const_reference b) {
  return a.first < b.first;
}

//////////////////////////////////////////////////////////////////////

namespace {
RegionDescPtr createRegion(const Transl::Tracelet& tlet) {
  if (tlet.m_refDeps.size()) {
    // We don't support reffiness guards yet.
    return nullptr;
  }

  auto region = smart::make_unique<RegionDesc>();
  SrcKey sk(tlet.m_sk);
  assert(sk == tlet.m_instrStream.first->source);
  auto unit = tlet.m_instrStream.first->unit();

  // Boundaries for the current RegionDesc::Block, which is created when we
  // finish the Tracelet or start a new basic block.
  SrcKey startSk(sk);
  int numInstrs = 0;

  for (auto ni = tlet.m_instrStream.first; ni; ni = ni->next) {
    assert(sk == ni->source);
    assert(ni->unit() == unit);

    ++numInstrs;
    if (ni->op() == OpJmp && ni->next) {
      // A Jmp that isn't the final instruction in a Tracelet means we traced
      // through a forward jump in analyze. Update sk to point to the next NI
      // in the stream.
      auto dest = ni->offset() + ni->imm[0].u_BA;
      assert(dest > sk.offset()); // We only trace for forward Jmps for now.
      sk.setOffset(dest);

      // The Jmp terminates this block, so append it and start a new one.
      region->blocks.push_back(
        smart::make_unique<RegionDesc::Block>(
          tlet.m_func, startSk.offset(), numInstrs));
      numInstrs = 0;
      startSk = sk;
    } else {
      sk.advance(unit);
    }
  }

  // Append the final block. This will be the only block for Tracelets that
  // didn't include a Jmp.
  region->blocks.push_back(
    smart::make_unique<RegionDesc::Block>(
      tlet.m_func, startSk.offset(), numInstrs));

  // Add tracelet guards as predictions on the first instruction. Predictions
  // and known types from static analysis will be applied by
  // Translator::translateRegion.
  for (auto const& dep : tlet.m_dependencies) {
    if (dep.second->rtt.isVagueValue() ||
        dep.second->location.isThis()) continue;

    typedef RegionDesc R;
    auto addPred = [&](const R::Location& loc) {
      auto type = Type::fromRuntimeType(dep.second->rtt);
      region->blocks.front()->addPredicted(tlet.m_sk, {loc, type});
    };

    switch (dep.first.space) {
      case Transl::Location::Stack:
        addPred(R::Location::Stack{uint32_t(-dep.first.offset - 1)});
        break;

      case Transl::Location::Local:
        addPred(R::Location::Local{uint32_t(dep.first.offset)});
        break;

      default: not_reached();
    }
  }

  FTRACE(2, "Converted Tracelet:\n{}\nInto RegionDesc:\n{}\n",
         tlet.toString(), show(*region));
  return region;
}
}

RegionDescPtr selectRegion(const RegionContext& context,
                           const Transl::Tracelet* t) {
  auto const mode = regionMode();

  FTRACE(1,
    "Select region: {}@{} mode={} context:\n{}{}",
    context.func->fullName()->data(),
    context.offset,
    static_cast<int>(mode),
    [&]{
      std::string ret;
      for (auto& t : context.liveTypes) {
        folly::toAppend(" ", show(t), "\n", &ret);
      }
      return ret;
    }(),
    [&]{
      std::string ret;
      for (auto& ar : context.preLiveARs) {
        folly::toAppend(" ", show(ar), "\n", &ret);
      }
      return ret;
    }()
  );

  auto region = [&]{
    try {
      switch (mode) {
      case RegionMode::None:   return RegionDescPtr{nullptr};
      case RegionMode::OneBC:  return regionOneBC(context);
      case RegionMode::Method: return regionMethod(context);
      case RegionMode::Tracelet: always_assert(t); return createRegion(*t);
      }
      not_reached();
    } catch (const std::exception& e) {
      FTRACE(1, "region selector threw: {}\n", e.what());
      return RegionDescPtr{nullptr};
    }
  }();

  if (region) {
    FTRACE(3, "{}", show(*region));
  } else {
    FTRACE(1, "no region selectable; using tracelet compiler\n");
  }

  return region;
}

//////////////////////////////////////////////////////////////////////

std::string show(RegionDesc::Location l) {
  switch (l.tag()) {
  case RegionDesc::Location::Tag::Local:
    return folly::format("Local{{{}}}", l.localId()).str();
  case RegionDesc::Location::Tag::Stack:
    return folly::format("Stack{{{}}}", l.stackOffset()).str();
  }
  not_reached();
}

std::string show(RegionDesc::TypePred ta) {
  return folly::format(
    "{} :: {}",
    show(ta.location),
    ta.type.toString()
  ).str();
}

std::string show(RegionContext::LiveType ta) {
  return folly::format(
    "{} :: {}",
    show(ta.location),
    ta.type.toString()
  ).str();
}

std::string show(RegionContext::PreLiveAR ar) {
  return folly::format(
    "AR@{}: {} ({})",
    ar.stackOff,
    ar.func->fullName()->data(),
    ar.objOrCls.toString()
  ).str();
}

std::string show(const RegionDesc::Block& b) {
  std::string ret{"Block "};
  folly::toAppend(
    b.func()->fullName()->data(), '@', b.start().offset(),
    " length ", b.length(), '\n',
    &ret
  );

  auto const& tpRange = b.typePreds();
  auto tpIter         = begin(tpRange);

  auto skIter = b.start();
  for (int i = 0; i < b.length(); ++i) {
    while (tpIter != end(tpRange) && tpIter->first < skIter) {
      ++tpIter;
    }
    while (tpIter != end(tpRange) && tpIter->first == skIter) {
      folly::toAppend("  predict: ", show(tpIter->second), "\n", &ret);
      ++tpIter;
    }
    folly::toAppend(
      "    ",
      skIter.offset(),
      "  ",
      instrToString(b.unit()->at(skIter.offset()), b.unit()),
      '\n',
      &ret
    );
    skIter.advance(b.unit());
  }

  return ret;
}

std::string show(const RegionDesc& region) {
  return folly::format(
    "Region ({} blocks):\n{}",
    region.blocks.size(),
    [&]{
      std::string ret;
      for (auto& b : region.blocks) {
        folly::toAppend(show(*b), &ret);
      }
      return ret;
    }()
  ).str();
}

//////////////////////////////////////////////////////////////////////

}}
