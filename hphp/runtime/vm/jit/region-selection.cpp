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
#include "hphp/runtime/vm/jit/region-selection.h"

#include <algorithm>
#include <boost/range/adaptors.hpp>

#include "folly/Memory.h"
#include "folly/Conv.h"

#include "hphp/util/assertions.h"
#include "hphp/util/map-walker.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(region);

using Transl::TransID;
using Transl::TranslatorX64;

//////////////////////////////////////////////////////////////////////

extern RegionDescPtr selectMethod(const RegionContext&);
extern RegionDescPtr selectOneBC(const RegionContext&);
extern RegionDescPtr selectTracelet(const RegionContext&);
extern RegionDescPtr selectHotBlock(TransID transId,
                                    const ProfData* profData,
                                    const TransCFG& cfg);
extern RegionDescPtr selectHotTrace(TransID triggerId,
                                    const ProfData* profData,
                                    TransCFG& cfg,
                                    TransIDSet& selectedSet);

//////////////////////////////////////////////////////////////////////

namespace {

enum class RegionMode {
  None,      // empty region

  // Modes that create a region by inspecting live VM state
  OneBC,     // region with a single bytecode instruction
  Method,    // region with a whole method
  Tracelet,  // single-entry, multiple-exits region that ends on conditional
             // branches or when an instruction consumes a value of unknown type
  Legacy,    // same as Tracelet, but using the legacy analyze() code

  // Modes that create a region by leveraging profiling data
  HotBlock,  // single-entry, single-exit region
  HotTrace,  // single-entry, multiple-exits region
};

RegionMode regionMode() {
  auto& s = RuntimeOption::EvalJitRegionSelector;
  if (s == ""        ) return RegionMode::None;
  if (s == "onebc"   ) return RegionMode::OneBC;
  if (s == "method"  ) return RegionMode::Method;
  if (s == "tracelet") return RegionMode::Tracelet;
  if (s == "legacy"  ) return RegionMode::Legacy;
  if (s == "hotblock") return RegionMode::HotBlock;
  if (s == "hottrace") return RegionMode::HotTrace;
  FTRACE(1, "unknown region mode {}: using none\n", s);
  if (debug) abort();
  return RegionMode::None;
}

}

//////////////////////////////////////////////////////////////////////

void RegionDesc::Block::addPredicted(SrcKey sk, TypePred pred) {
  assert(pred.type.subtypeOf(Type::Gen | Type::Cls));
  m_typePreds.insert(std::make_pair(sk, pred));
  checkInvariants();
}

void RegionDesc::Block::setParamByRef(SrcKey sk, bool byRef) {
  assert(m_byRefs.find(sk) == m_byRefs.end());
  m_byRefs.insert(std::make_pair(sk, byRef));
  checkInvariants();
}

void RegionDesc::Block::addReffinessPred(SrcKey sk, const ReffinessPred& pred) {
  m_refPreds.insert(std::make_pair(sk, pred));
  checkInvariants();
}

void RegionDesc::Block::setKnownFunc(SrcKey sk, const Func* func) {
  assert(m_knownFuncs.find(sk) == m_knownFuncs.end());
  m_knownFuncs.insert(std::make_pair(sk, func));
  checkInvariants();
}

void RegionDesc::Block::setPostConditions(const PostConditions& conds) {
  m_postConds = conds;
}

/*
 * Check invariants on a RegionDesc::Block.
 *
 * 1. Single entry, single exit (aside from exceptions).  I.e. no
 *    non-fallthrough instructions mid-block and no control flow (not
 *    counting calls as control flow).
 *
 * 2. Each SrcKey in m_typePreds, m_byRefs, m_refPreds, and m_knownFuncs is
 *    within the bounds of the block.
 *
 * 3. Each local id referred to in the type prediction list is valid.
 *
 * 4. (Unchecked) each stack offset in the type prediction list is
 *    valid.
 */
void RegionDesc::Block::checkInvariants() const {
  if (!debug || length() == 0) return;

  smart::set<SrcKey> keysInRange;
  auto firstKey = [&] { return *keysInRange.begin(); };
  auto lastKey = [&] {
    assert(!keysInRange.empty());
    return *--keysInRange.end();
  };
  keysInRange.insert(start());
  for (int i = 1; i < length(); ++i) {
    if (i != length() - 1) {
      auto const pc = unit()->at(lastKey().offset());
      if (instrFlags(toOp(*pc)) & TF) {
        FTRACE(1, "Bad block: {}\n", show(*this));
        assert(!"Block may not contain non-fallthrough instruction unless "
                "they are last");
      }
      if (instrIsNonCallControlFlow(toOp(*pc))) {
        FTRACE(1, "Bad block: {}\n", show(*this));
        assert(!"Block may not contain control flow instructions unless "
                "they are last");
      }
    }
    keysInRange.insert(lastKey().advanced(unit()));
  }
  assert(keysInRange.size() == length());

  auto rangeCheck = [&](const char* type, SrcKey sk) {
    if (!keysInRange.count(sk)) {
      std::cerr << folly::format("{} at {} outside range [{}, {}]\n",
                                type, show(sk),
                                 show(firstKey()), show(lastKey()));
      assert(!"Region::Block contained out-of-range metadata");
    }
  };
  for (auto& tpred : m_typePreds) {
    rangeCheck("type prediction", tpred.first);
    auto& loc = tpred.second.location;
    switch (loc.tag()) {
    case Location::Tag::Local: assert(loc.localId() < m_func->numLocals());
                               break;
    case Location::Tag::Stack: // Unchecked
                               break;
    }
  }

  for (auto& byRef : m_byRefs) {
    rangeCheck("parameter reference flag", byRef.first);
  }
  for (auto& refPred : m_refPreds) {
    rangeCheck("reffiness prediction", refPred.first);
  }
  for (auto& func : m_knownFuncs) {
    rangeCheck("known Func*", func.first);
  }
}

//////////////////////////////////////////////////////////////////////

namespace {
RegionDescPtr selectTraceletLegacy(const Transl::Tracelet& tlet) {
  typedef RegionDesc::Block Block;

  auto region = smart::make_unique<RegionDesc>();
  SrcKey sk(tlet.m_sk);
  auto unit = tlet.func()->unit();

  const Func* topFunc = nullptr;
  Block* curBlock = nullptr;
  auto newBlock = [&](const Func* func, SrcKey start) {
    assert(curBlock == nullptr || curBlock->length() > 0);
    region->blocks.push_back(
      std::make_shared<Block>(func, start.offset(), 0));
    curBlock = region->blocks.back().get();
  };
  newBlock(tlet.func(), sk);

  for (auto ni = tlet.m_instrStream.first; ni; ni = ni->next) {
    assert(sk == ni->source);
    assert(ni->unit() == unit);

    curBlock->addInstruction();
    if ((curBlock->length() == 1 && ni->funcd != nullptr) ||
        ni->funcd != topFunc) {
      topFunc = ni->funcd;
      curBlock->setKnownFunc(sk, topFunc);
    }

    if (ni->calleeTrace && !ni->calleeTrace->m_inliningFailed) {
      assert(ni->op() == OpFCall);
      assert(ni->funcd == ni->calleeTrace->func());
      // This should be translated as an inlined call. Insert the blocks of the
      // callee in the region.
      auto const& callee = *ni->calleeTrace;
      curBlock->setInlinedCallee(ni->funcd);
      SrcKey cSk = callee.m_sk;
      Unit* cUnit = callee.func()->unit();

      newBlock(callee.func(), cSk);

      for (auto cni = callee.m_instrStream.first; cni; cni = cni->next) {
        assert(cSk == cni->source);
        assert(cni->op() == OpRetC ||
               cni->op() == OpContRetC ||
               cni->op() == OpNativeImpl ||
               !instrIsControlFlow(cni->op()));

        curBlock->addInstruction();
        cSk.advance(cUnit);
      }

      if (ni->next) {
        sk.advance(unit);
        newBlock(tlet.func(), sk);
      }
      continue;
    }

    if (!ni->noOp && isFPassStar(ni->op())) {
      curBlock->setParamByRef(sk, ni->preppedByRef);
    }

    if (ni->next && ni->op() == OpJmp) {
      // A Jmp that isn't the final instruction in a Tracelet means we traced
      // through a forward jump in analyze. Update sk to point to the next NI
      // in the stream.
      auto dest = ni->offset() + ni->imm[0].u_BA;
      assert(dest > sk.offset()); // We only trace for forward Jmps for now.
      sk.setOffset(dest);

      // The Jmp terminates this block.
      newBlock(tlet.func(), sk);
    } else {
      sk.advance(unit);
    }
  }

  auto& frontBlock = *region->blocks.front();

  // Add tracelet guards as predictions on the first instruction. Predictions
  // and known types from static analysis will be applied by
  // Translator::translateRegion.
  for (auto const& dep : tlet.m_dependencies) {
    if (dep.second->rtt.isVagueValue() ||
        dep.second->location.isThis()) continue;

    typedef RegionDesc R;
    auto addPred = [&](const R::Location& loc) {
      auto type = Type::fromRuntimeType(dep.second->rtt);
      frontBlock.addPredicted(tlet.m_sk, {loc, type});
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

  // Add reffiness dependencies as predictions on the first instruction.
  for (auto const& dep : tlet.m_refDeps.m_arMap) {
    RegionDesc::ReffinessPred pred{dep.second.m_mask,
                                   dep.second.m_vals,
                                   dep.first};
    frontBlock.addReffinessPred(tlet.m_sk, pred);
  }

  FTRACE(2, "Converted Tracelet:\n{}\nInto RegionDesc:\n{}\n",
         tlet.toString(), show(*region));
  return region;
}
}

RegionDesc::BlockPtr createBlock(const Transl::Tracelet& tlet) {
  RegionDescPtr region = selectTraceletLegacy(tlet);

  if (region == nullptr) return nullptr;

  always_assert(region->blocks.size() == 1);
  return region->blocks.front();
}

RegionDescPtr selectRegion(const RegionContext& context,
                           const Transl::Tracelet* t) {
  auto const mode = regionMode();

  FTRACE(1,
    "Select region: {}@{} mode={} context:\n{}{}",
    context.func->fullName()->data(),
    context.bcOffset,
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
        case RegionMode::None:     return RegionDescPtr{nullptr};
        case RegionMode::OneBC:    return selectOneBC(context);
        case RegionMode::Method:   return selectMethod(context);
        case RegionMode::Tracelet: return selectTracelet(context);
        case RegionMode::Legacy:
                 always_assert(t); return selectTraceletLegacy(*t);
        case RegionMode::HotBlock:
        case RegionMode::HotTrace: always_assert(0 &&
                                                 "unsupported region mode");
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

RegionDescPtr selectHotRegion(TransID transId,
                              TranslatorX64* tx64) {

  assert(RuntimeOption::EvalJitPGO);

  const ProfData* profData = tx64->profData();
  FuncId funcId = profData->transFuncId(transId);
  TransCFG cfg(funcId, profData, tx64->getSrcDB(), tx64->getJmpToTransIDMap());
  TransIDSet selectedTIDs;
  RegionDescPtr region = nullptr;
  RegionMode mode = regionMode();

  switch (mode) {
    case RegionMode::None:
      region = RegionDescPtr{nullptr};
      break;
    case RegionMode::HotBlock:
      region = selectHotBlock(transId, profData, cfg);
      break;
    case RegionMode::HotTrace:
      region = selectHotTrace(transId, profData, cfg, selectedTIDs);
      break;
    case RegionMode::OneBC:
    case RegionMode::Method:
    case RegionMode::Tracelet:
    case RegionMode::Legacy:
      always_assert(0 && "unsupported region mode");
  }

  if (Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    std::string dotFileName = string("/tmp/trans-cfg-") +
                              lexical_cast<std::string>(transId) + ".dot";

    cfg.print(dotFileName, profData, &selectedTIDs);
    FTRACE(5, "selectHotRegion: New Translation {} (file: {}) {}\n",
           tx64->profData()->curTransID(), dotFileName,
           region ? show(*region) : std::string("empty region"));
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

std::string show(const RegionDesc::ReffinessPred& pred) {
  std::ostringstream out;
  out << "offset: " << pred.arSpOffset << " mask: ";
  for (auto const bit : pred.mask) out << (bit ? '1' : '0');
  out << " vals: ";
  for (auto const bit : pred.vals) out << (bit ? '1' : '0');
  return out.str();
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

  auto typePreds = makeMapWalker(b.typePreds());
  auto byRefs    = makeMapWalker(b.paramByRefs());
  auto refPreds  = makeMapWalker(b.reffinessPreds());
  auto knownFuncs= makeMapWalker(b.knownFuncs());
  auto skIter    = b.start();

  const Func* topFunc = nullptr;

  for (int i = 0; i < b.length(); ++i) {
    while (typePreds.hasNext(skIter)) {
      folly::toAppend("  predict: ", show(typePreds.next()), "\n", &ret);
    }
    while (refPreds.hasNext(skIter)) {
      folly::toAppend("  predict reffiness: ", show(refPreds.next()), "\n",
                      &ret);
    }

    std::string knownFunc;
    if (knownFuncs.hasNext(skIter)) {
      topFunc = knownFuncs.next();
    }
    if (topFunc) {
      const char* inlined = "";
      if (i == b.length() - 1 && b.inlinedCallee()) {
        assert(topFunc == b.inlinedCallee());
        inlined = " (call is inlined)";
      }
      knownFunc = folly::format(" (top func: {}{})",
                                topFunc->fullName()->data(), inlined).str();
    } else {
      assert((i < b.length() - 1 || !b.inlinedCallee()) &&
             "inlined FCall without a known funcd");
    }

    std::string byRef;
    if (byRefs.hasNext(skIter)) {
      byRef = folly::format(" (passed by {})", byRefs.next() ? "reference"
                                                             : "value").str();
    }

    std::string instrString;
    folly::toAppend(instrToString((Op*)b.unit()->at(skIter.offset()), b.unit()),
                    byRef,
                    &instrString);

    folly::toAppend(
      "    ",
      skIter.offset(),
      "  ",
      knownFunc.empty() ? instrString
                        : folly::format("{:<40}", instrString).str(),
      knownFunc,
      "\n",
      &ret
    );
    skIter.advance(b.unit());
  }

  for (const auto& postCond : b.postConds()) {
    folly::toAppend("  postcondition: ", show(postCond), "\n", &ret);
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
