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

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/block.h"

namespace HPHP {  namespace JIT {

BlockList rpoSortCfg(IRTrace* trace, const IRUnit& unit) {
  assert(trace->isMain());
  BlockList blocks;
  blocks.reserve(unit.numBlocks());
  unsigned next_id = 0;
  postorderWalk(
    [&](Block* block) {
      block->setPostId(next_id++);
      blocks.push_back(block);
    },
    unit.numBlocks(),
    trace->front()
  );
  std::reverse(blocks.begin(), blocks.end());
  assert(blocks.size() <= unit.numBlocks());
  assert(next_id <= unit.numBlocks());
  return blocks;
}

bool isRPOSorted(const BlockList& blocks) {
  int id = 0;
  for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
    if ((*it)->postId() != id++) return false;
  }
  return true;
}

/*
 * Find the immediate dominator of each block using Cooper, Harvey, and
 * Kennedy's "A Simple, Fast Dominance Algorithm", returned as a vector
 * of postorder ids, indexed by postorder id.
 */
IdomVector findDominators(const BlockList& blocks) {
  assert(isRPOSorted(blocks));

  // Calculate immediate dominators with the iterative two-finger algorithm.
  // When it terminates, idom[post-id] will contain the post-id of the
  // immediate dominator of each block.  idom[start] will be -1.  This is
  // the general algorithm but it will only loop twice for loop-free graphs.
  auto const num_blocks = blocks.size();
  IdomVector idom(num_blocks, -1);
  auto start = blocks.begin();
  int start_id = (*start)->postId();
  idom[start_id] = start_id;
  start++;
  for (bool changed = true; changed; ) {
    changed = false;
    // for each block after start, in reverse postorder
    for (auto it = start; it != blocks.end(); it++) {
      Block* block = *it;
      int b = block->postId();
      // new_idom = any already-processed predecessor
      auto edge_it = block->preds().begin();
      int new_idom = edge_it->from()->postId();
      while (idom[new_idom] == -1) new_idom = (++edge_it)->from()->postId();
      // for all other already-processed predecessors p of b
      for (auto& edge : block->preds()) {
        auto p = edge.from()->postId();
        if (p != new_idom && idom[p] != -1) {
          // find earliest common predecessor of p and new_idom
          // (higher postIds are earlier in flow and in dom-tree).
          int b1 = p, b2 = new_idom;
          do {
            while (b1 < b2) b1 = idom[b1];
            while (b2 < b1) b2 = idom[b2];
          } while (b1 != b2);
          new_idom = b1;
        }
      }
      if (idom[b] != new_idom) {
        idom[b] = new_idom;
        changed = true;
      }
    }
  }
  idom[start_id] = -1; // start has no idom.
  return idom;
}

DomChildren findDomChildren(const BlockList& blocks) {
  IdomVector idom = findDominators(blocks);
  DomChildren children(blocks.size(), BlockList());
  for (Block* block : blocks) {
    int idom_id = idom[block->postId()];
    if (idom_id != -1) children[idom_id].push_back(block);
  }
  return children;
}

bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms) {
  int p1 = b1->postId();
  int p2 = b2->postId();
  for (int i = p2; i != -1; i = idoms[i]) {
    if (i == p1) return true;
  }
  return false;
}

}}
