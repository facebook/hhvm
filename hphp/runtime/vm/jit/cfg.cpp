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
#include "hphp/runtime/vm/jit/cfg.h"

#include <algorithm>
#include <limits>

#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/mutation.h"

namespace HPHP::jit {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

// If edge is critical, split it by inserting an intermediate block.
// A critical edge is an edge from a block with multiple successors to
// a block with multiple predecessors. Returns the new intermediate block if
// one was inserted, and nullptr otherwise.
Block* splitCriticalEdge(IRUnit& unit, Edge* edge) {
  if (!edge) return nullptr;

  auto* to = edge->to();
  auto* branch = edge->inst();
  auto* from = branch->block();

  // While not necessarily critical, if we had to split any of the edges into
  // the catch we need to split all of them as we will be hoisting the
  // BeginCatch instructions and we cannot hoist them into the preds
  if (to->numPreds() <= 1 || (!to->isCatch() && from->numSuccs() <= 1)) {
    return nullptr;
  }

  return splitEdge(unit, from, to);
}

/*
 * Visit edges that have an unprocessed from() block if we walk the blocks in a
 * RPO.  These are the edges that create loops.
 */
template<class F>
void visit_retreating_edges(const IRUnit& unit, F f) {
  auto const rpo = rpoSortCfg(unit);
  auto seen = boost::dynamic_bitset<>(unit.numBlocks());

  for (auto& b : rpo) {
    for (auto& pred : b->preds()) {
      auto const fr = pred.from()->id();
      if (!seen[fr]) {
        if (!f(&pred)) return;
      }
    }
    seen[b->id()] = true;
  }
}

//////////////////////////////////////////////////////////////////////

}

BlockList poSortCfg(const IRUnit& unit) {
  auto blocks = BlockList{};
  blocks.reserve(unit.numBlocks());
  postorderWalk(unit,
    [&] (Block* block) {
      blocks.push_back(block);
    }
  );
  return blocks;
}

BlockList rpoSortCfg(const IRUnit& unit) {
  auto blocks = poSortCfg(unit);
  std::reverse(blocks.begin(), blocks.end());
  assertx(blocks.size() <= unit.numBlocks());
  return blocks;
}

BlockIDs numberBlocks(const IRUnit& unit, const BlockList& input) {
  auto ret = BlockIDs { unit, std::numeric_limits<uint32_t>::max() };
  auto id = uint32_t{0};
  for (auto block : input) ret[block] = id++;
  return ret;
}

Block* splitEdge(IRUnit& unit, Block* from, Block* to) {
  auto& branch = from->back();
  // Guesstimate the weight of the new block.
  auto profCount = std::min(from->profCount(), to->profCount()) / 2;
  Block* middle = unit.defBlock(profCount);
  FTRACE(3, "splitting edge from B{} -> B{} using B{}\n",
         from->id(), to->id(), middle->id());
  if (branch.taken() == to) {
    branch.setTaken(middle);
  } else {
    assertx(branch.next() == to);
    branch.setNext(middle);
  }

  middle->prepend(unit.gen(Jmp, branch.bcctx(), to));
  // Use the colder of the predecessor and successor to set the new block's
  // hint.
  middle->setHint(std::min(from->hint(), to->hint()));

  // The branch may not be a Jmp, in which case there won't be a label
  if (branch.numSrcs() > 0 && to->front().is(DefLabel)) {
    auto& jmp = middle->back();
    for (auto src : branch.srcs()) {
      unit.expandJmp(&jmp, src);
    }
    branch.setSrcs(0, nullptr);
  }

  return middle;
}

bool splitCriticalEdges(IRUnit& unit) {
  FTRACE(2, "splitting critical edges\n");
  auto modified = removeUnreachable(unit);
  if (modified) reflowTypes(unit);
  auto const startBlocks = unit.numBlocks();

  jit::fast_set<Block*> newCatches;
  jit::fast_set<Block*> oldCatches;

  // Try to split outgoing edges of each reachable block.  This is safe in
  // a postorder walk since we visit blocks after visiting successors.
  postorderWalk(unit, [&](Block* b) {
    auto bnew = splitCriticalEdge(unit, b->takenEdge());
    splitCriticalEdge(unit, b->nextEdge());

    assertx(!b->next() || !b->next()->isCatch());
    if (bnew && b->taken()->isCatch()) {
      newCatches.emplace(bnew);
      oldCatches.emplace(b->taken());
    }
  });

  for (auto b : newCatches) {
    auto const& bc = b->next()->front();
    assertx(bc.is(BeginCatch));
    b->prepend(unit.clone(&bc, bc.dst()));
  }

  for (auto b : oldCatches) {
    auto bc = b->begin();
    assertx(bc->is(BeginCatch));
    b->erase(bc);
  }

  return modified || unit.numBlocks() != startBlocks;
}

bool removeUnreachable(IRUnit& unit) {
  ITRACE(2, "removing unreachable blocks\n");
  Trace::Indent _i;

  boost::dynamic_bitset<> visited(unit.numBlocks());
  jit::vector<Block*> blocks;
  jit::vector<Block*> stack;
  blocks.reserve(unit.numBlocks());
  stack.reserve(unit.numBlocks());

  // Find all blocks reachable from the entry block.
  stack.push_back(unit.entry());
  while (!stack.empty()) {
    auto* b = stack.back();
    stack.pop_back();
    if (visited.test(b->id())) continue;
    visited.set(b->id());
    blocks.push_back(b);
    if (auto* taken = b->taken()) {
      if (!visited.test(taken->id())) stack.push_back(taken);
    }
    if (auto* next = b->next()) {
      if (!visited.test(next->id())) stack.push_back(next);
    }
  }

  // Walk through the reachable blocks and erase any preds that weren't
  // found.
  jit::vector<IRInstruction*> deadInsts;
  for (auto* block : blocks) {
    for (auto &edge : block->preds()) {
      auto* inst = edge.inst();
      always_assert(!inst->isTransient());
      if (!visited.test(inst->block()->id())) {
        deadInsts.push_back(inst);
      }
    }
  }

  for (auto* inst : deadInsts) {
    ITRACE(3, "removing unreachable B{}\n", inst->block()->id());
    inst->setNext(nullptr);
    inst->setTaken(nullptr);
  }

  return !deadInsts.empty();
}

/*
 * Find the immediate dominator of each block using Cooper, Harvey, and
 * Kennedy's "A Simple, Fast Dominance Algorithm", returned as a vector
 * of Block*, indexed by block.  IdomVector[b] == nullptr if b has no
 * dominator.  This is the case for the entry block and any blocks not
 * reachable from the entry block.
 */
IdomVector findDominators(const IRUnit& unit,
                          const BlockList& blocks,
                          const BlockIDs& rpoIDs) {
  // Calculate immediate dominators with the iterative two-finger algorithm.
  // When it terminates, idom[post-id] will contain the post-id of the
  // immediate dominator of each block.  idom[start] will be -1.  This is
  // the general algorithm but it will only loop twice for loop-free graphs.
  IdomVector idom(unit, nullptr);
  auto start = blocks.begin();
  auto entry = *start;
  idom[entry] = entry;
  start++;
  for (bool changed = true; changed; ) {
    changed = false;
    // for each block after start, in reverse postorder
    for (auto it = start; it != blocks.end(); it++) {
      Block* block = *it;
      // p1 = any already-processed predecessor
      auto predIter = block->preds().begin();
      auto predEnd = block->preds().end();
      auto p1 = predIter->from();
      while (!idom[p1]) p1 = (++predIter)->from();
      // for all other already-processed predecessors p2 of block
      for (++predIter; predIter != predEnd; ++predIter) {
        auto p2 = predIter->from();
        if (p2 == p1 || !idom[p2]) continue;
        // find earliest common predecessor of p1 and p2
        // (lower RPO ids are earlier in flow and in dom-tree).
        do {
          while (rpoIDs[p1] < rpoIDs[p2]) p2 = idom[p2];
          while (rpoIDs[p2] < rpoIDs[p1]) p1 = idom[p1];
        } while (p1 != p2);
      }
      if (idom[block] != p1) {
        idom[block] = p1;
        changed = true;
      }
    }
  }
  idom[entry] = nullptr; // entry has no dominator.
  return idom;
}

bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms) {
  assertx(b1 != nullptr && b2 != nullptr);
  for (auto b = b2; b != nullptr; b = idoms[b]) {
    if (b == b1) return true;
  }
  return false;
}

EdgeSet findRetreatingEdges(const IRUnit& unit) {
  auto v = jit::vector<Edge*>{};
  visit_retreating_edges(unit, [&] (Edge* edge) {
    v.push_back(edge);
    return true;
  });
  return EdgeSet(begin(v), end(v));
}

bool cfgHasLoop(const IRUnit& unit) {
  auto ret = false;
  visit_retreating_edges(unit, [&](Edge* /*edge*/) {
    ret = true;
    return false;
  });
  return ret;
}


LoopInfo findBlocksInLoops(const IRUnit& unit, const EdgeSet& backEdges) {
  jit::hash_map<RegionDesc::BlockId, BlockList> loopEntryBlocks;
  jit::hash_set<RegionDesc::BlockId> blocks;
  auto findBlocksInLoop = [&](Edge* backEdge) {
    auto stack = BlockList{};
    jit::hash_set<RegionDesc::BlockId> visited;
    visited.reserve(unit.numBlocks());
    stack.reserve(unit.numBlocks());
    stack.push_back(backEdge->from());
    auto loopHeader = backEdge->to();
    auto loopHeaderId = loopHeader->id();
    visited.insert(loopHeaderId);
    blocks.insert(loopHeaderId);

    // Find all blocks dominated by backEdge->to() that can reach
    // backEdge->from()
    while (!stack.empty()) {
      auto* b = stack.back();
      stack.pop_back();
      if (visited.find(b->id()) != visited.end()) continue;
      blocks.insert(b->id());
      visited.insert(b->id());
      b->forEachPred([&] (Block* pred) {
        stack.push_back(pred);
      });
    }

    BlockList entries{};
    loopHeader->forEachPred([&] (Block* pred) {
      if (visited.find(pred->id()) != visited.end()) return;
      entries.push_back(pred);
    });
    loopEntryBlocks[loopHeaderId] = std::move(entries);
  };
  for (auto edge : backEdges) {
    findBlocksInLoop(edge);
  }
  return {blocks, loopEntryBlocks};
}

//////////////////////////////////////////////////////////////////////

}
