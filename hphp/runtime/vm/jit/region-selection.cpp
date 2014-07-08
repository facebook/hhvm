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
#include "hphp/runtime/vm/jit/region-selection.h"

#include <algorithm>
#include <functional>
#include <exception>
#include <utility>
#include <iostream>

#include "folly/Memory.h"
#include "folly/Conv.h"
#include "folly/String.h"

#include "hphp/util/assertions.h"
#include "hphp/util/map-walker.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(region);


//////////////////////////////////////////////////////////////////////

extern RegionDescPtr selectMethod(const RegionContext&);
extern RegionDescPtr selectOneBC(const RegionContext&);
extern RegionDescPtr selectHotBlock(TransID transId,
                                    const ProfData* profData,
                                    const TransCFG& cfg);

//////////////////////////////////////////////////////////////////////

namespace {

enum class RegionMode {
  None,      // empty region

  // Modes that create a region by inspecting live VM state
  Method,    // region with a whole method
  Tracelet,  // single-entry, multiple-exits region that ends on conditional
             // branches or when an instruction consumes a value of unknown type
};

RegionMode regionMode() {
  auto& s = RuntimeOption::EvalJitRegionSelector;
  if (s == ""        ) return RegionMode::None;
  if (s == "method"  ) return RegionMode::Method;
  if (s == "tracelet") return RegionMode::Tracelet;
  FTRACE(1, "unknown region mode {}: using none\n", s);
  assert(false);
  return RegionMode::None;
}

enum class PGORegionMode {
  Hottrace, // Select a long region, using profile counters to guide the trace
  Hotblock, // Select a single block
  WholeCFG, // Select the entire CFG that has been profiled
};

PGORegionMode pgoRegionMode() {
  auto& s = RuntimeOption::EvalJitPGORegionSelector;
  if (s == "hottrace") return PGORegionMode::Hottrace;
  if (s == "hotblock") return PGORegionMode::Hotblock;
  if (s == "wholecfg") return PGORegionMode::WholeCFG;
  FTRACE(1, "unknown pgo region mode {}: using hottrace\n", s);
  assert(false);
  return PGORegionMode::Hottrace;
}

template<typename Container>
void truncateMap(Container& c, SrcKey final) {
  c.erase(c.upper_bound(final), c.end());
}
}

//////////////////////////////////////////////////////////////////////

void RegionDesc::addArc(BlockId src, BlockId dst) {
  arcs.push_back({src, dst});
}

void RegionDesc::renumberBlock(BlockId oldId, BlockId newId) {
  for (auto& block : blocks) {
    if (block->id() == oldId) {
      block->setId(newId);
      break;
    }
  }
  for (auto& arc : arcs) {
    if (arc.src == oldId) arc.src = newId;
    if (arc.dst == oldId) arc.dst = newId;
  }
}

void RegionDesc::setSideExitingBlock(BlockId bid) {
  sideExitingBlocks.insert(bid);
}

bool RegionDesc::isSideExitingBlock(BlockId bid) const {
  return sideExitingBlocks.count(bid);
}

//////////////////////////////////////////////////////////////////////

RegionDesc::BlockId RegionDesc::Block::s_nextId = -1;

TransID getTransId(RegionDesc::BlockId blockId) {
  return blockId >= 0 ? blockId : kInvalidTransID;
}

bool hasTransId(RegionDesc::BlockId blockId) {
  return blockId >= 0;
}

RegionDesc::Block::Block(const Func* func, bool resumed, Offset start,
                         int length, Offset initSpOff)
  : m_id(s_nextId--)
  , m_func(func)
  , m_resumed(resumed)
  , m_start(start)
  , m_last(kInvalidOffset)
  , m_length(length)
  , m_initialSpOffset(initSpOff)
  , m_inlinedCallee(nullptr)
{
  assert(length >= 0);
  if (length > 0) {
    SrcKey sk(func, start, resumed);
    for (unsigned i = 1; i < length; ++i) sk.advance();
    m_last = sk.offset();
  }
  checkInstructions();
  checkMetadata();
}

bool RegionDesc::Block::contains(SrcKey sk) const {
  return sk >= start() && sk <= last();
}

void RegionDesc::Block::addInstruction() {
  if (m_length > 0) checkInstruction(last().op());
  assert((m_last == kInvalidOffset) == (m_length == 0));

  ++m_length;
  if (m_length == 1) {
    m_last = m_start;
  } else {
    m_last = last().advanced().offset();
  }
}

void RegionDesc::Block::truncateAfter(SrcKey final) {
  assert_not_implemented(!m_inlinedCallee);

  auto skIter = start();
  int newLen = -1;
  for (int i = 0; i < m_length; ++i, skIter.advance(unit())) {
    if (skIter == final) {
      newLen = i + 1;
      break;
    }
  }
  assert(newLen != -1);
  m_length = newLen;
  m_last = final.offset();

  truncateMap(m_typePreds, final);
  truncateMap(m_byRefs, final);
  truncateMap(m_refPreds, final);
  truncateMap(m_knownFuncs, final);

  checkInstructions();
  checkMetadata();
}

void RegionDesc::Block::addPredicted(SrcKey sk, TypePred pred) {
  FTRACE(2, "Block::addPredicted({}, {})\n", showShort(sk), show(pred));
  assert(pred.type <= Type::StackElem);
  assert(contains(sk));
  m_typePreds.insert(std::make_pair(sk, pred));
}

void RegionDesc::Block::setParamByRef(SrcKey sk, bool byRef) {
  FTRACE(2, "Block::setParamByRef({}, {})\n", showShort(sk),
         byRef ? "by ref" : "by val");
  assert(m_byRefs.find(sk) == m_byRefs.end());
  assert(contains(sk));
  m_byRefs.insert(std::make_pair(sk, byRef));
}

void RegionDesc::Block::addReffinessPred(SrcKey sk, const ReffinessPred& pred) {
  FTRACE(2, "Block::addReffinessPred({}, {})\n", showShort(sk), show(pred));
  assert(contains(sk));
  m_refPreds.insert(std::make_pair(sk, pred));
}

void RegionDesc::Block::setKnownFunc(SrcKey sk, const Func* func) {
  if (func == nullptr && m_knownFuncs.empty()) return;

  FTRACE(2, "Block::setKnownFunc({}, {})\n", showShort(sk),
         func ? func->fullName()->data() : "nullptr");
  assert(m_knownFuncs.find(sk) == m_knownFuncs.end());
  assert(contains(sk));
  auto it = m_knownFuncs.lower_bound(sk);
  if (it != m_knownFuncs.begin() && (--it)->second == func) {
    // Adding func at this sk won't add any new information.
    FTRACE(2, "  func exists at {}, not adding\n", showShort(it->first));
    return;
  }

  m_knownFuncs.insert(std::make_pair(sk, func));
}

void RegionDesc::Block::setPostConditions(const PostConditions& conds) {
  m_postConds = conds;
}

/*
 * Check invariants about the bytecode instructions in this Block.
 *
 * 1. Single entry, single exit (aside from exceptions).  I.e. no
 *    non-fallthrough instructions mid-block and no control flow (not
 *    counting calls as control flow).
 *
 */
void RegionDesc::Block::checkInstructions() const {
  if (!debug || length() == 0) return;

  auto u = unit();
  auto sk = start();

  for (int i = 1; i < length(); ++i) {
    if (i != length() - 1) checkInstruction(sk.op());
    sk.advance(u);
  }
  assert(sk.offset() == m_last);
}

void RegionDesc::Block::checkInstruction(Op op) const {
  if (instrFlags(op) & TF) {
    FTRACE(1, "Bad block: {}\n", show(*this));
    assert(!"Block may not contain non-fallthrough instruction unless "
           "they are last");
  }
  if (instrIsNonCallControlFlow(op)) {
    FTRACE(1, "Bad block: {}\n", show(*this));
    assert(!"Block may not contain control flow instructions unless "
           "they are last");
  }
}

/*
 * Check invariants about the metadata for this Block.
 *
 * 1. Each SrcKey in m_typePreds, m_byRefs, m_refPreds, and m_knownFuncs is
 *    within the bounds of the block.
 *
 * 2. Each local id referred to in the type prediction list is valid.
 *
 * 3. (Unchecked) each stack offset in the type prediction list is
 *    valid.
*/
void RegionDesc::Block::checkMetadata() const {
  auto rangeCheck = [&](const char* type, Offset o) {
    if (o < m_start || o > m_last) {
      std::cerr << folly::format("{} at {} outside range [{}, {}]\n",
                                 type, o, m_start, m_last);
      assert(!"Region::Block contained out-of-range metadata");
    }
  };
  for (auto& tpred : m_typePreds) {
    rangeCheck("type prediction", tpred.first.offset());
    auto& loc = tpred.second.location;
    switch (loc.tag()) {
    case Location::Tag::Local: assert(loc.localId() < m_func->numLocals());
                               break;
    case Location::Tag::Stack: // Unchecked
                               break;
    }
  }

  for (auto& byRef : m_byRefs) {
    rangeCheck("parameter reference flag", byRef.first.offset());
  }
  for (auto& refPred : m_refPreds) {
    rangeCheck("reffiness prediction", refPred.first.offset());
  }
  for (auto& func : m_knownFuncs) {
    rangeCheck("known Func*", func.first.offset());
  }
}

RegionDescPtr selectRegion(const RegionContext& context,
                           TransKind kind) {
  auto const mode = regionMode();

  FTRACE(1,
    "Select region: mode={} context:\n{}",
    static_cast<int>(mode), show(context)
  );

  auto region = [&]{
    try {
      switch (mode) {
        case RegionMode::None:
          return RegionDescPtr{nullptr};
        case RegionMode::Method:
          return selectMethod(context);
        case RegionMode::Tracelet:
          return selectTracelet(context, 0, kind == TransKind::Profile);
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
                              MCGenerator* mcg) {

  assert(RuntimeOption::EvalJitPGO);

  const ProfData* profData = mcg->tx().profData();
  FuncId funcId = profData->transFuncId(transId);
  TransCFG cfg(funcId, profData, mcg->tx().getSrcDB(),
               mcg->getJmpToTransIDMap());
  TransIDSet selectedTIDs;
  assert(regionMode() != RegionMode::Method);
  RegionDescPtr region;
  switch (pgoRegionMode()) {
    case PGORegionMode::Hottrace:
      region = selectHotTrace(transId, profData, cfg, selectedTIDs);
      break;

    case PGORegionMode::Hotblock:
      region = selectHotBlock(transId, profData, cfg);
      break;

    case PGORegionMode::WholeCFG:
      region = selectWholeCFG(transId, profData, cfg, selectedTIDs);
      break;
  }
  assert(region);

  if (Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    std::string dotFileName = std::string("/tmp/trans-cfg-") +
                              folly::to<std::string>(transId) + ".dot";

    cfg.print(dotFileName, funcId, profData, &selectedTIDs);
    FTRACE(5, "selectHotRegion: New Translation {} (file: {}) {}\n",
           mcg->tx().profData()->curTransID(), dotFileName,
           region ? show(*region) : std::string("empty region"));
  }

  return region;
}

//////////////////////////////////////////////////////////////////////

static bool postCondMismatch(const RegionDesc::TypePred& postCond,
                             const RegionDesc::TypePred& preCond) {
  return postCond.location == preCond.location &&
         preCond.type.not(postCond.type);
}

bool preCondsAreSatisfied(const RegionDesc::BlockPtr& block,
                          const PostConditions& prevPostConds) {
  const auto& preConds = block->typePreds();
  for (const auto& it : preConds) {
    for (const auto& post : prevPostConds) {
      const RegionDesc::TypePred& preCond = it.second;
      if (postCondMismatch(post, preCond)) {
        FTRACE(6, "preCondsAreSatisfied: postcondition check failed!\n"
               "  postcondition was {}, precondition was {}\n",
               show(post), show(preCond));
        return false;
      }
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////


namespace {
template<typename T>
struct Ignore {
  bool a(const T&) const { return false; }
  bool b(const T&) const { return false; }
};

struct IgnoreTypePred {
  explicit IgnoreTypePred(bool aLonger = false)
    : m_aLonger(aLonger)
  {}

  // It's ok for a to have more TypePreds if it's longer.
  bool a(const RegionDesc::TypePred& tp) const {
    return m_aLonger;
  }

  // It's ok for b to have more TypePreds if it's for a type we can probably
  // get from statically-known stack flavors.
  bool b(const RegionDesc::TypePred& tp) const {
    return tp.location.tag() == RegionDesc::Location::Tag::Stack &&
      tp.location.stackOffset() == 0 &&
      tp.type.isBoxed();
  }

 private:
  const bool m_aLonger;
};

struct IgnoreKnownFunc {
  // It's ok for a to have known funcs that b doesn't but not the other way
  // around.
  bool a(const Func*) const { return true; }
  bool b(const Func*) const { return false; }
};

template<typename M, typename Cmp = std::equal_to<typename M::mapped_type>,
         typename IgnorePred = Ignore<typename M::mapped_type>>
bool mapsEqual(const M& a, const M& b, SrcKey endSk, Cmp equal = Cmp(),
               IgnorePred ignore = IgnorePred()) {
  // Return true iff every value in aRange also exists in bRange. Leaves 'it'
  // pointing to aRange.second either way.
  using IterPair = std::pair<typename M::const_iterator,
                             typename M::const_iterator>;
  auto checkRange = [&](typename M::const_iterator& it, IterPair aRange,
                        IterPair bRange, bool aFirst) {
    for (it = aRange.first; it != aRange.second; ++it) {
      if (aFirst ? ignore.a(it->second) : ignore.b(it->second)) continue;

      auto bIt = bRange.first;
      for (; bIt != bRange.second; ++bIt) {
        if (equal(it->second, bIt->second)) break;
      }
      if (bIt == bRange.second) {
        it = aRange.second;
        return false;
      }
    }
    return true;
  };

  // Check if b has anything a doesn't
  for (auto it = b.begin(), end = b.end(); it != end; ) {
    if (!checkRange(it, b.equal_range(it->first), a.equal_range(it->first),
                    false)) {
      return false;
    }
  }

  // Check if a has anything b doesn't, up to b's end.
  for (auto it = a.begin(), end = a.end(); it != end; ) {
    if (it->first > endSk) break;

    if (!checkRange(it, a.equal_range(it->first), b.equal_range(it->first),
                    true)) {
      return false;
    }
  }

  return true;
}
}

std::string show(RegionDesc::Location l) {
  switch (l.tag()) {
  case RegionDesc::Location::Tag::Local:
    return folly::format("Local{{{}}}", l.localId()).str();
  case RegionDesc::Location::Tag::Stack:
    return folly::format("Stack{{{},{}}}",
                         l.stackOffset(), l.stackOffsetFromFp()).str();
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

std::string show(const PostConditions& pconds) {
  std::string ret;
  for (const auto& postCond : pconds) {
    folly::toAppend("  postcondition: ", show(postCond), "\n", &ret);
  }
  return ret;
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

std::string show(const RegionContext& ctx) {
  std::string ret;
  folly::toAppend(ctx.func->fullName()->data(), "@", ctx.bcOffset,
                  ctx.resumed ? "r" : "", "\n", &ret);
  for (auto& t : ctx.liveTypes) folly::toAppend(" ", show(t), "\n", &ret);
  for (auto& ar : ctx.preLiveARs) folly::toAppend(" ", show(ar), "\n", &ret);

  return ret;
}

std::string show(const RegionDesc::Block& b) {
  std::string ret{"Block "};
  folly::toAppend(b.id(), ' ',
                  b.func()->fullName()->data(), '@', b.start().offset(),
                  b.start().resumed() ? "r" : "",
                  " length ", b.length(),
                  " initSpOff ", b.initialSpOffset(), '\n',
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

  folly::toAppend(show(b.postConds()), &ret);

  return ret;
}

std::string show(const RegionDesc::Arc& arc) {
  return folly::format("{} -> {}\n", arc.src, arc.dst).str();
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
      folly::toAppend("Arcs:\n", &ret);
      for (auto& arc : region.arcs) {
        folly::toAppend(show(arc), &ret);
      }
      folly::toAppend("Side-exiting Blocks:\n",
                      folly::join(", ", region.sideExitingBlocks),
                      "\n",
                      &ret);
      return ret;
    }()
  ).str();
}

//////////////////////////////////////////////////////////////////////

}}
