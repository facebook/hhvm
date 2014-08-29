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
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit {

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

bool RegionDesc::empty() const {
  return m_blocks.empty();
}

RegionDesc::BlockPtr RegionDesc::entry() const {
  assert(!empty());
  return m_blocks[0];
}

bool RegionDesc::isExit(BlockId bid) const {
  return succs(bid).empty();
}

SrcKey RegionDesc::start() const {
  assert(!empty());
  return m_blocks[0]->start();
}

RegionDesc::Block* RegionDesc::addBlock(SrcKey sk,
                                        int    length,
                                        Offset spOffset) {
  m_blocks.push_back(
    std::make_shared<Block>(sk.func(), sk.resumed(), sk.offset(), length,
                            spOffset));
  BlockPtr block = m_blocks.back();
  m_data[block->id()] = BlockData(block);
  return block.get();
}

void RegionDesc::deleteBlock(BlockId bid) {
  auto it = std::find_if(m_blocks.begin(), m_blocks.end(),
                         [&](const BlockPtr b) { return b->id() == bid; });
  if (it == m_blocks.end()) return;
  m_blocks.erase(it);
  auto d = data(bid);
  always_assert(d.succs.empty() && d.preds.empty() &&
                "RegionDesc::deleteBlock needs support for blocks with arcs");
  m_data.erase(bid);
}

const RegionDesc::BlockVec& RegionDesc::blocks() const {
  return m_blocks;
}

RegionDesc::BlockData& RegionDesc::data(BlockId id) {
  auto it = m_data.find(id);
  assert(it != m_data.end());
  return it->second;
}

bool RegionDesc::hasBlock(BlockId id) const {
  return m_data.count(id);
}

RegionDesc::BlockPtr RegionDesc::block(BlockId id) const {
  return const_cast<RegionDesc*>(this)->data(id).block;
}

const RegionDesc::BlockIdSet& RegionDesc::succs(BlockId id) const {
  return const_cast<RegionDesc*>(this)->data(id).succs;
}

const RegionDesc::BlockIdSet& RegionDesc::preds(BlockId id) const {
  return const_cast<RegionDesc*>(this)->data(id).preds;
}

const RegionDesc::BlockIdSet& RegionDesc::sideExitingBlocks() const {
  return m_sideExitingBlocks;
}

void RegionDesc::addArc(BlockId srcId, BlockId dstId) {
  data(srcId).succs.insert(dstId);
  data(dstId).preds.insert(srcId);
}

void RegionDesc::renumberBlock(BlockId oldId, BlockId newId) {
  assert( hasBlock(oldId));
  assert(!hasBlock(newId));

  block(oldId)->setId(newId);
  m_data[newId] = m_data[oldId];
  m_data.erase(oldId);

  // Fix predecessor sets for the successors.
  for (auto succId : m_data[newId].succs) {
    BlockIdSet& succPreds = m_data[succId].preds;
    assert(succPreds.count(oldId));
    succPreds.erase(oldId);
    succPreds.insert(newId);
  }

  // Fix successor sets for the predecessors.
  for (auto predId : m_data[newId].preds) {
    BlockIdSet& predSuccs = m_data[predId].succs;
    assert(predSuccs.count(oldId));
    predSuccs.erase(oldId);
    predSuccs.insert(newId);
  }
}

void RegionDesc::setSideExitingBlock(BlockId bid) {
  m_sideExitingBlocks.insert(bid);
}

bool RegionDesc::isSideExitingBlock(BlockId bid) const {
  return m_sideExitingBlocks.count(bid);
}

void RegionDesc::copyArcsFrom(const RegionDesc& srcRegion) {
  for (auto const b : srcRegion.m_blocks) {
    auto bid = b->id();
    for (auto succId : srcRegion.succs(bid)) {
      addArc(bid, succId);
    }
  }
}

void RegionDesc::copyBlocksFrom(const RegionDesc&  other,
                                BlockVec::iterator where) {
  auto otherBlocks = other.blocks();
  m_blocks.insert(where, otherBlocks.begin(), otherBlocks.end());
  for (auto b : otherBlocks) {
    m_data[b->id()] = BlockData(b);
  }
}

void RegionDesc::append(const RegionDesc& other) {
  copyBlocksFrom(other, m_blocks.end());
  copyArcsFrom(other);
  m_sideExitingBlocks.insert(other.m_sideExitingBlocks.begin(),
                             other.m_sideExitingBlocks.end());
}

void RegionDesc::prepend(const RegionDesc& other) {
  copyBlocksFrom(other, m_blocks.begin());
  copyArcsFrom(other);
  m_sideExitingBlocks.insert(other.m_sideExitingBlocks.begin(),
                             other.m_sideExitingBlocks.end());
}

std::string RegionDesc::toString() const {
  auto ret = show(*this);
  ret += "data:\n";
  for (auto d : m_data) {
    ret += folly::format("  block id: {}\n", d.first).str();
  }
  return ret;
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
          return selectTracelet(context, kind == TransKind::Profile);
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

bool breaksRegion(Op opc) {
  switch (opc) {
    case OpMIterNext:
    case OpMIterNextK:
    case OpSwitch:
    case OpSSwitch:
    case OpCreateCont:
    case OpYield:
    case OpYieldK:
    case OpRetC:
    case OpRetV:
    case OpExit:
    case OpFatal:
    case OpMIterInit:
    case OpMIterInitK:
    case OpIterBreak:
    case OpDecodeCufIter:
    case OpThrow:
    case OpUnwind:
    case OpEval:
    case OpNativeImpl:
      return true;

    default:
      return false;
  }
}

//////////////////////////////////////////////////////////////////////

namespace {

struct DFSChecker {

  explicit DFSChecker(const RegionDesc& region)
    : m_region(region) { }

  bool check(RegionDesc::BlockId id) {
    if (m_visiting.count(id) > 0) {
      // Found a loop. This is only valid if EvalJitLoops is enabled.
      return RuntimeOption::EvalJitLoops;
    }
    if (m_visited.count(id) > 0) return true;
    m_visited.insert(id);
    m_visiting.insert(id);
    for (auto succ : m_region.succs(id)) {
      if (!check(succ)) return false;
    }
    m_visiting.erase(id);
    return true;
  }

  size_t numVisited() const { return m_visited.size(); }

 private:
  const RegionDesc&      m_region;
  RegionDesc::BlockIdSet m_visited;
  RegionDesc::BlockIdSet m_visiting;
};

}

/*
 * Checks if the given region is well-formed, which entails the
 * following properties:
 *
 *   1) The region has at least one block.
 *
 *   2) Each block in the region has a different id.
 *
 *   3) All arcs involve blocks within the region.
 *
 *   4) For each arc, the bytecode offset of the dst block must
 *      possibly follow the execution of the src block.
 *
 *   5) Each block contains at most one successor corresponding to a
 *      given SrcKey.
 *
 *   6) The region doesn't contains any loops, unless JitLoops is
 *      enabled.
 *
 *   7) All blocks are reachable from the entry block.
 */
bool check(const RegionDesc& region, std::string& error) {

  auto bad = [&](const std::string& errorMsg) {
    error = errorMsg;
    return false;
  };

  // 1) The region has at least one block.
  if (region.empty()) return bad("empty region");

  RegionDesc::BlockIdSet blockSet;
  for (auto b : region.blocks()) {
    auto bid = b->id();
    // 2) Each block in the region has a different id.
    if (blockSet.count(bid)) {
      return bad(folly::format("many blocks with id {}", bid).str());
    }
    blockSet.insert(bid);
  }

  for (auto b : region.blocks()) {
    auto bid = b->id();
    SrcKey    lastSk = region.block(bid)->last();
    OffsetSet validSuccOffsets = lastSk.succOffsets();
    OffsetSet succOffsets;

    for (auto succ : region.succs(bid)) {
      SrcKey succSk = region.block(succ)->start();
      Offset succOffset = succSk.offset();

      // 3) All arcs involve blocks within the region.
      if (blockSet.count(succ) == 0) {
        return bad(folly::format("arc with dst not in the region: {} -> {}",
                                 bid, succ).str());
      }

      // Checks 4) and 5) below don't make sense for arcs corresponding
      // to inlined calls and returns, so skip them in such cases.
      // This won't be possible once task #4076399 is done.
      if (lastSk.func() != succSk.func()) continue;

      // 4) For each arc, the bytecode offset of the dst block must
      //    possibly follow the execution of the src block.
      if (validSuccOffsets.count(succOffset) == 0) {
        return bad(folly::format("arc with impossible control flow: {} -> {}",
                                 bid, succ).str());
      }

      // 5) Each block contains at most one successor corresponding to a
      //    given SrcKey.
      if (succOffsets.count(succOffset) > 0) {
        return bad(folly::format("block {} has multiple successors with SK {}",
                                 bid, show(succSk)).str());
      }
      succOffsets.insert(succOffset);
    }
    for (auto pred : region.preds(bid)) {
      if (blockSet.count(pred) == 0) {
        return bad(folly::format("arc with src not in the region: {} -> {}",
                                 pred, bid).str());
      }
    }
  }

  // 6) is checked by dfsCheck.
  DFSChecker dfsCheck(region);
  if (!dfsCheck.check(region.entry()->id())) {
    return bad("region is cyclic");
  }

  // 7) All blocks are reachable from the entry (first) block.
  if (dfsCheck.numVisited() != blockSet.size()) {
    return bad("region has unreachable blocks");
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

std::string show(const RegionDesc& region) {
  return folly::format(
    "Region ({} blocks):\n{}",
    region.blocks().size(),
    [&]{
      std::string ret;
      std::string arcs;
      for (auto& b : region.blocks()) {
        folly::toAppend(show(*b), &ret);
        for (auto s : region.succs(b->id())) {
          folly::toAppend(folly::format("{} -> {}\n", b->id(), s), &arcs);
        }
      }
      folly::toAppend("Arcs:\n" + arcs, &ret);
      folly::toAppend("Side-exiting Blocks:\n",
                      folly::join(", ", region.sideExitingBlocks()),
                      "\n",
                      &ret);
      return ret;
    }()
  ).str();
}

//////////////////////////////////////////////////////////////////////

}}
