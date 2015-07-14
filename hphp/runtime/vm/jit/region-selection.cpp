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

#include <folly/Memory.h>
#include <folly/Conv.h>
#include <folly/String.h>

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
  assertx(false);
  return RegionMode::None;
}

template<typename Container>
void truncateMap(Container& c, SrcKey final) {
  c.erase(c.upper_bound(final), c.end());
}
}

//////////////////////////////////////////////////////////////////////

PGORegionMode pgoRegionMode(const Func& func) {
  auto& s = RuntimeOption::EvalJitPGORegionSelector;
  if ((s == "wholecfg" || s == "hotcfg") &&
      RuntimeOption::EvalJitPGOCFGHotFuncOnly && !(func.attrs() & AttrHot)) {
    return PGORegionMode::Hottrace;
  }
  if (s == "hottrace") return PGORegionMode::Hottrace;
  if (s == "hotblock") return PGORegionMode::Hotblock;
  if (s == "hotcfg")   return PGORegionMode::HotCFG;
  if (s == "wholecfg") return PGORegionMode::WholeCFG;
  FTRACE(1, "unknown pgo region mode {}: using hottrace\n", s);
  assertx(false);
  return PGORegionMode::Hottrace;
}

//////////////////////////////////////////////////////////////////////

bool RegionDesc::empty() const {
  return m_blocks.empty();
}

RegionDesc::BlockPtr RegionDesc::entry() const {
  assertx(!empty());
  return m_blocks[0];
}

bool RegionDesc::isExit(BlockId bid) const {
  return succs(bid).empty();
}

SrcKey RegionDesc::start() const {
  assertx(!empty());
  return m_blocks[0]->start();
}

uint32_t RegionDesc::instrSize() const {
  uint32_t size = 0;
  for (auto& b : m_blocks) {
    size += b->length();
  }
  return size;
}

SrcKey RegionDesc::lastSrcKey() const {
  assertx(!empty());
  FuncId startFuncId = start().funcID();
  for (int i = m_blocks.size() - 1; i >= 0; i--) {
    SrcKey sk = m_blocks[i]->last();
    if (sk.funcID() == startFuncId) {
      return sk;
    }
  }
  always_assert(0);
}


RegionDesc::Block* RegionDesc::addBlock(SrcKey      sk,
                                        int         length,
                                        FPInvOffset spOffset,
                                        uint16_t    inlineLevel) {
  m_blocks.push_back(
    std::make_shared<Block>(sk.func(), sk.resumed(), sk.offset(), length,
                            spOffset, inlineLevel));
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
  assertx(it != m_data.end());
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

folly::Optional<RegionDesc::BlockId> RegionDesc::nextRetrans(BlockId id) const {
  return const_cast<RegionDesc*>(this)->data(id).nextRetrans;
}

void RegionDesc::setNextRetrans(BlockId id, BlockId next) {
  assertx(!data(id).nextRetrans);
  data(id).nextRetrans = next;
  data(next).preds.insert(id);
}

const RegionDesc::BlockIdSet& RegionDesc::sideExitingBlocks() const {
  return m_sideExitingBlocks;
}

void RegionDesc::addArc(BlockId srcId, BlockId dstId) {
  data(srcId).succs.insert(dstId);
  data(dstId).preds.insert(srcId);
}

void RegionDesc::removeArc(BlockId srcID, BlockId dstID) {
  data(srcID).succs.erase(dstID);
  data(dstID).preds.erase(srcID);
}

void RegionDesc::renumberBlock(BlockId oldId, BlockId newId) {
  assertx( hasBlock(oldId));
  assertx(!hasBlock(newId));

  block(oldId)->setID(newId);
  m_data[newId] = m_data[oldId];
  m_data.erase(oldId);

  // Fix predecessor sets for the successors.
  for (auto succId : m_data[newId].succs) {
    BlockIdSet& succPreds = m_data[succId].preds;
    assertx(succPreds.count(oldId));
    succPreds.erase(oldId);
    succPreds.insert(newId);
  }

  // Fix successor sets for the predecessors.
  for (auto predId : m_data[newId].preds) {
    BlockIdSet& predSuccs = m_data[predId].succs;
    assertx(predSuccs.count(oldId));
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

/*
 * Perform a DFS starting at block `bid', storing the post-order in
 * `outVec'.
 */
void RegionDesc::postOrderSort(RegionDesc::BlockId     bid,
                               RegionDesc::BlockIdSet& visited,
                               RegionDesc::BlockIdVec& outVec) {
  if (visited.count(bid)) return;
  visited.insert(bid);

  if (auto nextRetr = nextRetrans(bid)) {
    postOrderSort(nextRetr.value(), visited, outVec);
  }
  for (auto succ : succs(bid)) {
    postOrderSort(succ, visited, outVec);
  }
  outVec.push_back(bid);
}

/**
 * Sort the m_blocks vector in reverse post order.  This enforces that
 * m_blocks will be a topological order in case the region is acyclic.
 * All region arcs are taken into account, including retranslation arcs.
 */
void RegionDesc::sortBlocks() {
  RegionDesc::BlockIdSet visited;
  RegionDesc::BlockIdVec reverse;

  postOrderSort(entry()->id(), visited, reverse);
  assertx(m_blocks.size() == reverse.size());

  // Update `m_blocks' vector.
  m_blocks.clear();
  auto size = reverse.size();
  for (size_t i = 0; i < size; i++) {
    m_blocks.push_back(block(reverse[size - i - 1]));
  }
}

namespace {

struct Chain {
  size_t id;
  jit::vector<RegionDesc::BlockId> blocks;
};

using BlockToChainMap = hphp_hash_map<RegionDesc::BlockId, size_t>;

void mergeChains(Chain& dst, Chain& src, BlockToChainMap& b2c) {
  if (dst.id == src.id) return;
  dst.blocks.insert(dst.blocks.end(), src.blocks.begin(), src.blocks.end());
  for (auto bid : src.blocks) {
    b2c[bid] = dst.id;
  }
  src.blocks.clear();
}

RegionDesc::BlockId findFirstInSet(const Chain& c, RegionDesc::BlockIdSet s) {
  for (auto bid : c.blocks) {
    if (s.count(bid)) return bid;
  }
  always_assert(0);
}

}

/**
 * Chain the retranslation blocks.  This method enforces that, for
 * each region block, all its successor have distinct SrcKeys.
 */
void RegionDesc::chainRetransBlocks() {

  jit::vector<Chain> chains;
  BlockToChainMap block2chain;

  // 1. Initially assign each region block to its own chain.
  for (auto b : blocks()) {
    auto bid = b->id();
    auto cid = chains.size();
    chains.push_back({cid, {bid}});
    block2chain[bid] = cid;
  }

  // 2. For each block, if it has 2 successors with the same SrcKey,
  //    then merge the successors' chains into one.
  for (auto b : blocks()) {
    auto bid = b->id();
    const auto& succSet = succs(bid);
    for (auto it1 = succSet.begin(); it1 != succSet.end(); it1++) {
      auto bid1 = *it1;
      auto cid1 = block2chain[bid1];
      for (auto it2 = it1 + 1; it2 != succSet.end(); it2++) {
        auto bid2 = *it2;
        auto cid2 = block2chain[bid2];
        if (data(bid1).block->start() == data(bid2).block->start()) {
          mergeChains(chains[cid1], chains[cid2], block2chain);
        }
      }
    }
  }

  // 3. Sort each chain.  In general, we want to sort each chain in
  //    decreasing order of profile weights.  However, note that this
  //    transformation can turn acyclic graphs into cyclic ones (see
  //    example below).  Therefore, if JitLoops are disabled, we
  //    instead sort each chain following the original block order,
  //    which prevents loops from being generated if the region was
  //    originally acyclic.
  //
  //    Here's an example showing how an acyclic CFG can become cyclic
  //    by chaining its retranslation blocks:
  //
  //      - Region before chaining retranslation blocks, where B2' and B2"
  //        are retranslations starting at the same SrcKey:
  //          B1  -> B2'
  //          B1  -> B2"
  //          B2' -> B3
  //          B3  -> B2"
  //
  //      - Region after sorting the chain as B2" -R-> B2':
  //          B1  ->   B2"
  //          B2" -R-> B2'
  //          B2' ->   B3
  //          B3  ->   B2"
  //        Note the cycle: B2" -R-> B2' -> B3 -> B2".
  //
  auto profData = mcg->tx().profData();

  auto weight = [&](RegionDesc::BlockId bid) {
    return hasTransID(bid) ? profData->absTransCounter(getTransID(bid)) : 0;
  };

  auto sortGeneral = [&](RegionDesc::BlockId bid1, RegionDesc::BlockId bid2) {
    return weight(bid1) > weight(bid2);
  };

  using SortFun = std::function<bool(RegionDesc::BlockId, RegionDesc::BlockId)>;
  SortFun sortFunc = sortGeneral;

  hphp_hash_map<RegionDesc::BlockId, uint32_t> origBlockOrder;
  if (!RuntimeOption::EvalJitLoops) {
    for (uint32_t i = 0; i < m_blocks.size(); i++) {
      origBlockOrder[m_blocks[i]->id()] = i;
    }
    auto sortAcyclic = [&](RegionDesc::BlockId bid1, RegionDesc::BlockId bid2) {
      return origBlockOrder[bid1] < origBlockOrder[bid2];
    };
    sortFunc = sortAcyclic;
  }

  TRACE(1, "chainRetransBlocks: computed chains:\n");
  for (auto& c : chains) {
    std::sort(c.blocks.begin(), c.blocks.end(), sortFunc);

    if (Trace::moduleEnabled(Trace::region, 1) && c.blocks.size() > 0) {
      FTRACE(1, "  -> {} (w={})", c.blocks[0], weight(c.blocks[0]));
      for (size_t i = 1; i < c.blocks.size(); i++) {
        FTRACE(1, ", {} (w={})", c.blocks[i], weight(c.blocks[i]));
      }
      FTRACE(1, "\n");
    }
  }

  // 4. Set the nextRetrans blocks according to the computed chains.
  for (auto& c : chains) {
    if (c.blocks.size() == 0) continue;
    for (size_t i = 0; i < c.blocks.size() - 1; i++) {
      setNextRetrans(c.blocks[i], c.blocks[i + 1]);
    }
  }

  // 5. For each block with multiple successors in the same chain,
  //    only keep the successor that first appears in the chain.
  for (auto b : blocks()) {
    auto& succSet = data(b->id()).succs;
    for (auto s : succSet) {
      auto& c = chains[block2chain[s]];
      auto selectedSucc = findFirstInSet(c, succSet);
      for (auto other : c.blocks) {
        if (other == selectedSucc) continue;
        succSet.erase(other);
      }
    }
  }

  // 6. Reorder the blocks in the region in topological order (if
  //    region is acyclic), since the previous steps may break it.
  sortBlocks();
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

/*
 * We assign unique negative ID's to all new blocks---these correspond to
 * invalid TransIDs.  To maintain this property, we have to start one past
 * the sentinel kInvalidTransID, which is -1.
 */
RegionDesc::BlockId RegionDesc::Block::s_nextId = -2;

TransID getTransID(RegionDesc::BlockId blockId) {
  assertx(hasTransID(blockId));
  return TransID(blockId);
}

bool hasTransID(RegionDesc::BlockId blockId) {
  return blockId >= 0;
}

RegionDesc::Block::Block(const Func* func,
                         bool        resumed,
                         Offset      start,
                         int         length,
                         FPInvOffset initSpOff,
                         uint16_t    inlineLevel)
  : m_id(s_nextId--)
  , m_func(func)
  , m_resumed(resumed)
  , m_start(start)
  , m_last(kInvalidOffset)
  , m_length(length)
  , m_initialSpOffset(initSpOff)
  , m_inlinedCallee(nullptr)
  , m_inlineLevel(inlineLevel)
  , m_profTransID(kInvalidTransID)
{
  assertx(length >= 0);
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
  assertx((m_last == kInvalidOffset) == (m_length == 0));

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
  assertx(newLen != -1);
  m_length = newLen;
  m_last = final.offset();

  truncateMap(m_typePredictions, final);
  truncateMap(m_typePreConditions, final);
  truncateMap(m_byRefs, final);
  truncateMap(m_refPreds, final);
  truncateMap(m_knownFuncs, final);

  checkInstructions();
  checkMetadata();
}

void RegionDesc::Block::addPredicted(SrcKey sk, TypedLocation locType) {
  FTRACE(2, "Block::addPredicted({}, {})\n", showShort(sk), show(locType));
  assertx(locType.type != TBottom);
  assertx(locType.type <= TStkElem);
  assertx(contains(sk));
  m_typePredictions.insert(std::make_pair(sk, locType));
}

void RegionDesc::Block::addPreCondition(SrcKey sk, TypedLocation locType) {
  FTRACE(2, "Block::addPreCondition({}, {})\n", showShort(sk), show(locType));
  assertx(locType.type != TBottom);
  assertx(locType.type <= TStkElem);
  assertx(contains(sk));
  m_typePreConditions.insert(std::make_pair(sk, locType));
}

void RegionDesc::Block::setParamByRef(SrcKey sk, bool byRef) {
  FTRACE(2, "Block::setParamByRef({}, {})\n", showShort(sk),
         byRef ? "by ref" : "by val");
  assertx(m_byRefs.find(sk) == m_byRefs.end());
  assertx(contains(sk));
  m_byRefs.insert(std::make_pair(sk, byRef));
}

void RegionDesc::Block::addReffinessPred(SrcKey sk, const ReffinessPred& pred) {
  FTRACE(2, "Block::addReffinessPred({}, {})\n", showShort(sk), show(pred));
  assertx(contains(sk));
  m_refPreds.insert(std::make_pair(sk, pred));
}

void RegionDesc::Block::setKnownFunc(SrcKey sk, const Func* func) {
  if (func == nullptr && m_knownFuncs.empty()) return;

  FTRACE(2, "Block::setKnownFunc({}, {})\n", showShort(sk),
         func ? func->fullName()->data() : "nullptr");
  assertx(m_knownFuncs.find(sk) == m_knownFuncs.end());
  assertx(contains(sk));
  auto it = m_knownFuncs.lower_bound(sk);
  if (it != m_knownFuncs.begin() && (--it)->second == func) {
    // Adding func at this sk won't add any new information.
    FTRACE(2, "  func exists at {}, not adding\n", showShort(it->first));
    return;
  }

  m_knownFuncs.insert(std::make_pair(sk, func));
}

void RegionDesc::Block::setPostConds(const PostConditions& conds) {
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
  assertx(sk.offset() == m_last);
}

void RegionDesc::Block::checkInstruction(Op op) const {
  if (instrFlags(op) & TF) {
    FTRACE(1, "Bad block: {}\n", show(*this));
    assertx(!"Block may not contain non-fallthrough instruction unless "
           "they are last");
  }
  if (instrIsNonCallControlFlow(op)) {
    FTRACE(1, "Bad block: {}\n", show(*this));
    assertx(!"Block may not contain control flow instructions unless "
           "they are last");
  }
}

/*
 * Check invariants about the metadata for this Block.
 *
 * 1. Each SrcKey in m_typePredictions, m_preConditions, m_byRefs, m_refPreds,
 *    and m_knownFuncs is within the bounds of the block.
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
      assertx(!"Region::Block contained out-of-range metadata");
    }
  };

  auto checkTypedLocations = [&](const char* msg, const TypedLocMap& map) {
    for (auto& typedLoc : map) {
      rangeCheck("type prediction", typedLoc.first.offset());
      auto& loc = typedLoc.second.location;
      switch (loc.tag()) {
      case Location::Tag::Local: assertx(loc.localId() < m_func->numLocals());
                                 break;
      case Location::Tag::Stack: // Unchecked
                                 break;
      }
    }
  };

  checkTypedLocations("type prediction", m_typePredictions);
  checkTypedLocations("type precondition", m_typePreConditions);

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
    always_assert(region->instrSize() <= RuntimeOption::EvalJitMaxRegionInstrs);
  } else {
    FTRACE(1, "no region selectable; using tracelet compiler\n");
  }

  return region;
}

RegionDescPtr selectHotRegion(TransID transId,
                              MCGenerator* mcg) {

  assertx(RuntimeOption::EvalJitPGO);

  const ProfData* profData = mcg->tx().profData();
  auto const& func = *(profData->transFunc(transId));
  FuncId funcId = func.getFuncId();
  TransCFG cfg(funcId, profData, mcg->tx().getSrcDB(),
               mcg->getJmpToTransIDMap());
  TransIDSet selectedTIDs;
  assertx(regionMode() != RegionMode::Method);
  RegionDescPtr region;
  switch (pgoRegionMode(func)) {
    case PGORegionMode::Hottrace:
      region = selectHotTrace(transId, profData, cfg, selectedTIDs);
      break;

    case PGORegionMode::Hotblock:
      region = selectHotBlock(transId, profData, cfg);
      break;

    case PGORegionMode::WholeCFG:
    case PGORegionMode::HotCFG:
      region = selectHotCFG(transId, profData, cfg, selectedTIDs);
      break;
  }
  assertx(region);

  if (Trace::moduleEnabled(HPHP::Trace::pgo, 5)) {
    std::string dotFileName = std::string("/tmp/trans-cfg-") +
                              folly::to<std::string>(transId) + ".dot";

    cfg.print(dotFileName, funcId, profData, &selectedTIDs);
    FTRACE(5, "selectHotRegion: New Translation {} (file: {}) {}\n",
           mcg->tx().profData()->curTransID(), dotFileName,
           region ? show(*region) : std::string("empty region"));
  }

  always_assert(region->instrSize() <= RuntimeOption::EvalJitMaxRegionInstrs);

  return region;
}

//////////////////////////////////////////////////////////////////////

static bool postCondMismatch(const RegionDesc::TypedLocation& postCond,
                             const RegionDesc::TypedLocation& preCond) {
  return postCond.location == preCond.location &&
         !preCond.type.maybe(postCond.type);
}

bool preCondsAreSatisfied(const RegionDesc::BlockPtr& block,
                          const TypedLocations& prevPostConds) {
  const auto& preConds = block->typePreConditions();
  for (const auto& it : preConds) {
    for (const auto& post : prevPostConds) {
      const RegionDesc::TypedLocation& preCond = it.second;
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

bool breaksRegion(SrcKey sk) {
  switch (sk.op()) {
    case Op::MIterNext:
    case Op::MIterNextK:
    case Op::SSwitch:
    case Op::CreateCont:
    case Op::Yield:
    case Op::YieldK:
    case Op::RetC:
    case Op::RetV:
    case Op::Exit:
    case Op::Fatal:
    case Op::MIterInit:
    case Op::MIterInitK:
    case Op::IterBreak:
    case Op::DecodeCufIter:
    case Op::Throw:
    case Op::Unwind:
    case Op::Eval:
    case Op::NativeImpl:
      return true;

    case Op::Await:
      // We break regions at resumed Await instructions, to avoid
      // duplicating the translation of the resumed SrcKey after the
      // Await.
      return sk.resumed();

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
    if (auto nextRetrans = m_region.nextRetrans(id)) {
      if (!check(nextRetrans.value())) return false;
    }
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
 *   6) The region doesn't contain any loops, unless JitLoops is
 *      enabled.
 *
 *   7) All blocks are reachable from the entry block.
 *
 *   8) For each block, there must be a path from the entry to it that
 *      includes only earlier blocks in the region.
 *
 *   9) The region is topologically sorted unless loops are enabled.
 *
 *  10) The block-retranslation chains cannot have cycles.
 *
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
      return bad(folly::sformat("many blocks with id {}", bid));
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
        return bad(folly::sformat("arc with dst not in the region: {} -> {}",
                                  bid, succ));
      }

      // Checks 4) and 5) below don't make sense for arcs corresponding
      // to inlined calls and returns, so skip them in such cases.
      // This won't be possible once task #4076399 is done.
      if (lastSk.func() != succSk.func()) continue;

      // 4) For each arc, the bytecode offset of the dst block must
      //    possibly follow the execution of the src block.
      if (validSuccOffsets.count(succOffset) == 0) {
        return bad(folly::sformat("arc with impossible control flow: {} -> {}",
                                  bid, succ));
      }

      // 5) Each block contains at most one successor corresponding to a
      //    given SrcKey.
      if (succOffsets.count(succOffset) > 0) {
        return bad(folly::sformat("block {} has multiple successors with SK {}",
                                  bid, show(succSk)));
      }
      succOffsets.insert(succOffset);
    }
    for (auto pred : region.preds(bid)) {
      if (blockSet.count(pred) == 0) {
        return bad(folly::sformat("arc with src not in the region: {} -> {}",
                                  pred, bid));
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

  // 8) and 9) are checked below.
  RegionDesc::BlockIdSet visited;
  auto& blocks = region.blocks();
  for (unsigned i = 0; i < blocks.size(); i++) {
    auto bid = blocks[i]->id();
    unsigned nVisited = 0;
    for (auto pred : region.preds(bid)) {
      nVisited += visited.count(pred);
    }
    // 8) For each block, there must be a path from the entry to it that
    //    includes only earlier blocks in the region.
    if (nVisited == 0 && i != 0) {
      return bad(folly::sformat("block {} appears before all its predecessors",
                                bid));
    }
    // 9) The region is topologically sorted unless loops are enabled.
    if (!RuntimeOption::EvalJitLoops && nVisited != region.preds(bid).size()) {
      return bad(folly::sformat("non-topological order (bid: {})", bid));
    }
    visited.insert(bid);
  }

  // 10) The block-retranslation chains cannot have cycles.
  for (auto b : blocks) {
    auto bid = b->id();
    RegionDesc::BlockIdSet chainSet;
    chainSet.insert(bid);
    while (auto next = region.nextRetrans(bid)) {
      auto nextId = next.value();
      if (chainSet.count(nextId)) {
        return bad(folly::sformat("cyclic retranslation chain for block {}",
                                  bid));
      }
      chainSet.insert(nextId);
      bid = nextId;
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

std::string show(RegionDesc::Location l) {
  switch (l.tag()) {
  case RegionDesc::Location::Tag::Local:
    return folly::format("Local{{{}}}", l.localId()).str();
  case RegionDesc::Location::Tag::Stack:
    return folly::format("Stack{{{}}}", l.offsetFromFP().offset).str();
  }
  not_reached();
}

std::string show(RegionDesc::TypedLocation ta) {
  return folly::format(
    "{} :: {}",
    show(ta.location),
    ta.type.toString()
  ).str();
}

std::string show(const PostConditions& pconds) {
  std::string ret;
  for (const auto& postCond : pconds.changed) {
    folly::toAppend("  changed postcondition: ", show(postCond), "\n", &ret);
  }
  for (const auto& postCond : pconds.refined) {
    folly::toAppend("  refined postcondition: ", show(postCond), "\n", &ret);
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
    ar.func->fullName(),
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
                  " initSpOff ", b.initialSpOffset().offset,
                  " inlineLevel ", b.inlineLevel(),
                  " profTransID ", b.profTransID(),
                  '\n',
                  &ret
                 );

  auto predictions = makeMapWalker(b.typePredictions());
  auto preconditions = makeMapWalker(b.typePreConditions());
  auto byRefs    = makeMapWalker(b.paramByRefs());
  auto refPreds  = makeMapWalker(b.reffinessPreds());
  auto knownFuncs= makeMapWalker(b.knownFuncs());
  auto skIter    = b.start();

  const Func* topFunc = nullptr;

  for (int i = 0; i < b.length(); ++i) {
    while (predictions.hasNext(skIter)) {
      folly::toAppend("  predict: ", show(predictions.next()), "\n", &ret);
    }
    while (preconditions.hasNext(skIter)) {
      folly::toAppend("  precondition: ", show(preconditions.next()), "\n",
          &ret);
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
        assertx(topFunc == b.inlinedCallee());
        inlined = " (call is inlined)";
      }
      knownFunc = folly::format(" (top func: {}{})",
                                topFunc->fullName(), inlined).str();
    } else {
      assertx((i < b.length() - 1 || !b.inlinedCallee()) &&
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
        if (auto r = region.nextRetrans(b->id())) {
          folly::toAppend(folly::format("{} -R-> {}\n", b->id(), r.value()),
                          &arcs);
        }
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
