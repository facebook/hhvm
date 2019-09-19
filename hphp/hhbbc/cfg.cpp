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
#include "hphp/hhbbc/cfg.h"

#include <boost/dynamic_bitset.hpp>
#include <algorithm>
#include <vector>

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Note: Our style is generally to use lambdas, rather than helper
 * classes. postOrderWalk was originally written using a lambda,
 * rather than a helper class, but that ended up taking nearly 1k per
 * recursive call, which means it only takes about 10000 blocks to
 * overflow the stack.
 *
 * By putting everything in a helper class, we get that down to ~128
 * bytes per recursive call, which is a lot less likely to hit issues.
 *
 */
struct PostOrderWalker {
  const php::Func& func;
  std::vector<BlockId>& out;
  boost::dynamic_bitset<>& visited;

  void walk(BlockId blk) {
    if (visited[blk]) return;
    visited[blk] = true;
    forEachSuccessor(
      *func.blocks[blk],
      [this] (BlockId next) {
        walk(next);
      }
    );
    out.push_back(blk);
  }
};

void postorderWalk(
    const php::Func& func,
    std::vector<BlockId>& out,
    boost::dynamic_bitset<>& visited,
    BlockId blk) {
  auto walker = PostOrderWalker { func, out, visited };
  walker.walk(blk);
}

}

//////////////////////////////////////////////////////////////////////

std::vector<BlockId> rpoSortFromBlock(const php::Func& func, BlockId start) {
  boost::dynamic_bitset<> visited(func.blocks.size());
  std::vector<BlockId> ret;
  ret.reserve(func.blocks.size());
  postorderWalk(func, ret, visited, start);
  std::reverse(begin(ret), end(ret));
  return ret;
}

std::vector<BlockId> rpoSortFromMain(const php::Func& func) {
  return rpoSortFromBlock(func, func.mainEntry);
}

std::vector<BlockId> rpoSortAddDVs(const php::Func& func) {
  boost::dynamic_bitset<> visited(func.blocks.size());
  std::vector<BlockId> ret;
  ret.reserve(func.blocks.size());
  postorderWalk(func, ret, visited, func.mainEntry);

  /*
   * We've already marked the blocks reachable from the main entry
   * point.  Do post order walks from each DV entry with the same
   * visited set (so we'll stop if they chain to the main entry, which
   * is the normal case).
   */
  for (auto it = func.params.end(); it != func.params.begin(); ) {
    --it;
    if (it->dvEntryPoint == NoBlockId) continue;
    postorderWalk(func, ret, visited, it->dvEntryPoint);
  }
  std::reverse(begin(ret), end(ret));
  return ret;
}

BlockToBlocks
computeNonThrowPreds(const php::Func& func,
                     const std::vector<BlockId>& rpoBlocks) {
  auto preds = BlockToBlocks{};
  preds.reserve(rpoBlocks.size());
  for (auto const bid : rpoBlocks) {
    if (preds.size() < bid + 1) {
      preds.resize(bid + 1);
    }
    auto const b = func.blocks[bid].get();
    forEachNonThrowSuccessor(*b, [&] (BlockId blkId) {
      if (preds.size() < blkId + 1) {
        preds.resize(blkId + 1);
      }
      preds[blkId].insert(bid);
    });
  }
  return preds;
}

BlockToBlocks
computeThrowPreds(const php::Func& func,
                  const std::vector<BlockId>& rpoBlocks) {
  auto preds = BlockToBlocks{};
  preds.reserve(rpoBlocks.size());
  for (auto const bid : rpoBlocks) {
    if (preds.size() < bid + 1) {
      preds.resize(bid + 1);
    }
    auto const b = func.blocks[bid].get();
    if (b->throwExit != NoBlockId) {
      if (preds.size() < b->throwExit + 1) {
        preds.resize(b->throwExit + 1);
      }
      preds[b->throwExit].insert(bid);
    }
  }
  return preds;
}

/*
 * Walk forward through no-op blocks. To avoid cycles we don't take
 * "backward" branches unless its to the lowest numbered block seen to
 * date (this is just a heuristic to avoid having to keep a seen set,
 * because we don't expect long cyclic chains of no-op blocks).
 */
BlockId next_real_block(const php::Func& func, BlockId id) {
  auto blk = func.blocks[id].get();
  auto min = id;
  while (is_single_nop(*blk)) {
    if (blk->fallthrough == id || blk->fallthrough == NoBlockId) break;
    if (blk->fallthrough < id) {
      if (blk->fallthrough >= min) {
        // we may be in a cycle, but take one more hop anyway,
        // in case we're not.
        id = blk->fallthrough;
        break;
      }
      min = blk->fallthrough;
    }
    id = blk->fallthrough;
    blk = func.blocks[id].get();
  }
  return id;
}

//////////////////////////////////////////////////////////////////////

}}
