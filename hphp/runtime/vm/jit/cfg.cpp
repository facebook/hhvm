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

#include "hphp/runtime/vm/jit/cfg.h"
#include <algorithm>
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/mutation.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

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
  assert(blocks.size() <= unit.numBlocks());
  return blocks;
}

BlocksWithIds rpoSortCfgWithIds(const IRUnit& unit) {
  auto ret = BlocksWithIds{rpoSortCfg(unit), {unit, 0xffffffff}};

  uint32_t id = 0;
  for (auto block : ret.blocks) {
    ret.ids[block] = id++;
  }
  assert(id == ret.blocks.size());

  return ret;
}

Block* splitEdge(IRUnit& unit, Block* from, Block* to) {
  auto& branch = from->back();
  Block* middle = unit.defBlock();
  FTRACE(3, "splitting edge from B{} -> B{} using B{}\n",
         from->id(), to->id(), middle->id());
  if (branch.taken() == to) {
    branch.setTaken(middle);
  } else {
    assert(branch.next() == to);
    branch.setNext(middle);
  }

  middle->prepend(unit.gen(Jmp, branch.marker(), to));
  auto const unlikely = Block::Hint::Unlikely;
  if (from->hint() == unlikely || to->hint() == unlikely) {
    middle->setHint(unlikely);
  }
  return middle;
}

namespace {

// If edge is critical, split it by inserting an intermediate block.
// A critical edge is an edge from a block with multiple successors to
// a block with multiple predecessors.
void splitCriticalEdge(IRUnit& unit, Edge* edge) {
  if (!edge) return;

  auto* to = edge->to();
  auto* branch = edge->inst();
  auto* from = branch->block();
  if (to->numPreds() <= 1 || from->numSuccs() <= 1) return;

  splitEdge(unit, from, to);
}
}

bool splitCriticalEdges(IRUnit& unit) {
  FTRACE(2, "splitting critical edges\n");
  auto modified = removeUnreachable(unit);
  if (modified) reflowTypes(unit);
  auto const startBlocks = unit.numBlocks();

  // Try to split outgoing edges of each reachable block.  This is safe in
  // a postorder walk since we visit blocks after visiting successors.
  postorderWalk(unit, [&](Block* b) {
    splitCriticalEdge(unit, b->takenEdge());
    splitCriticalEdge(unit, b->nextEdge());
  });

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
  bool modified = false;
  for (auto* block: blocks) {
    auto& preds = block->preds();
    for (auto it = preds.begin(); it != preds.end(); ) {
      auto* inst = it->inst();
      ++it;

      if (!visited.test(inst->block()->id())) {
        ITRACE(3, "removing unreachable B{}\n", inst->block()->id());
        inst->setNext(nullptr);
        inst->setTaken(nullptr);
        modified = true;
      }
    }
  }

  return modified;
}

/*
 * Find the immediate dominator of each block using Cooper, Harvey, and
 * Kennedy's "A Simple, Fast Dominance Algorithm", returned as a vector
 * of Block*, indexed by block.  IdomVector[b] == nullptr if b has no
 * dominator.  This is the case for the entry block and any blocks not
 * reachable from the entry block.
 */
IdomVector findDominators(const IRUnit& unit, const BlocksWithIds& blockIds) {
  auto& blocks = blockIds.blocks;
  auto& rpoIds = blockIds.ids;

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
          while (rpoIds[p1] < rpoIds[p2]) p2 = idom[p2];
          while (rpoIds[p2] < rpoIds[p1]) p1 = idom[p1];
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
  assert(b1 != nullptr && b2 != nullptr);
  for (auto b = b2; b != nullptr; b = idoms[b]) {
    if (b == b1) return true;
  }
  return false;
}

namespace {

// Visits all back-edges in a CFG.
template <class Visitor>
struct BackEdgeVisitor {
  BackEdgeVisitor(const IRUnit& unit, Visitor& visitor)
    : m_path(unit.numBlocks())
    , m_visited(unit.numBlocks())
    , m_visitor(visitor)
  {}

  using BitSet = boost::dynamic_bitset<>;

  void walk(Edge* e) {
    if (e == nullptr) return;

    auto const block = e->to();
    auto const id = block->id();

    // If we're revisiting a block in our current search, then we've
    // found a backedge.
    if (m_path.test(id)) {
      // The entry block can't be a loop header.
      assert(!block->isEntry());

      m_visitor(e);
    }

    // Otherwise if we're getting back to a block that's already been
    // visited, but it hasn't been visited in this path, then we can
    // prune this search.
    if (m_visited.test(id)) return;

    m_visited.set(id);
    m_path.set(id);

    walk(block->takenEdge());
    walk(block->nextEdge());

    m_path.set(id, false);
  }

private:
  BitSet m_path;
  BitSet m_visited;
  Visitor& m_visitor;
};

template <class Visitor>
void backEdgeWalk(const IRUnit& unit, Visitor visitor) {
  BackEdgeVisitor<Visitor> bev(unit, visitor);

  auto const entry = unit.entry();
  bev.walk(entry->takenEdge());
  bev.walk(entry->nextEdge());
}

}

bool insertLoopPreHeaders(IRUnit& unit) {
  ITRACE(2, "making preheaders\n");
  Trace::Indent _i;

  bool changed = false;

  auto const backEdges = findBackEdges(unit);

  for (auto header : findLoopHeaders(unit)) {
    // Compute the set of forward predecessors for the loop header.
    EdgeSet fwdPreds;
    for (auto& pred : header->preds()) {
      if (backEdges.find(&pred) == backEdges.end()) fwdPreds.insert(&pred);
    }

    // Header can't be the entry block.
    assert(fwdPreds.size() != 0);

    // Already have a pre-header, so do nothing.
    if (fwdPreds.size() == 1) continue;

    auto const preheader = unit.defBlock();
    preheader->push_back(unit.gen(Jmp, header->front().marker(), header));

    ITRACE(4, "making pre-header B{} for header B{}\n",
           preheader->id(), header->id());

    auto constexpr unlikely = Block::Hint::Unlikely;
    if (header->hint() == unlikely) preheader->setHint(unlikely);

    // Point all forward preds at pre-header.
    for (auto const pred : fwdPreds) {
      auto& branch = pred->from()->back();

      assert(branch.taken() == header || branch.next() == header);

      if (branch.taken() == header) branch.setTaken(preheader);
      if (branch.next() == header) branch.setNext(preheader);

      changed = true;
    }
  }

  return changed;
}

EdgeSet findBackEdges(const IRUnit& unit) {
  EdgeSet edges;
  backEdgeWalk(unit, [&] (Edge* e) { edges.insert(e); });
  return edges;
}

BlockSet findLoopHeaders(const IRUnit& unit) {
  BlockSet headers;
  backEdgeWalk(unit, [&] (Edge* e) { headers.insert(e->to()); });
  return headers;
}

bool cfgHasLoop(const IRUnit& unit) {
  bool hasLoop = false;
  backEdgeWalk(unit, [&] (Edge*) { hasLoop = true; });
  return hasLoop;
}

}}
