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

BlockList rpoSortCfg(const IRUnit& unit) {
  BlockList blocks;
  blocks.reserve(unit.numBlocks());
  unsigned next_id = 0;
  postorderWalk(unit,
    [&](Block* block) {
      block->setPostId(next_id++);
      blocks.push_back(block);
    });
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
 * of Block*, indexed by block.  IdomVector[b] == nullptr if b has no
 * dominator.  This is the case for the entry block and any blocks not
 * reachable from the entry block.
 */
IdomVector findDominators(const IRUnit& unit, const BlockList& blocks) {
  assert(isRPOSorted(blocks));

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
        // (higher postIds are earlier in flow and in dom-tree).
        do {
          while (p1->postId() < p2->postId()) p1 = idom[p1];
          while (p2->postId() < p1->postId()) p2 = idom[p2];
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

DomChildren findDomChildren(const IRUnit& unit, const BlockList& blocks) {
  IdomVector idom = findDominators(unit, blocks);
  DomChildren children(unit, BlockList());
  for (Block* block : blocks) {
    auto idomBlock = idom[block];
    if (idomBlock) children[idomBlock].push_back(block);
  }
  return children;
}

bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms) {
  for (auto b = b2; b != nullptr; b = idoms[b]) {
    if (b == b1) return true;
  }
  return false;
}

}}
