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

void postorderWalk(const php::Func& func,
                   std::vector<borrowed_ptr<php::Block>>& out,
                   boost::dynamic_bitset<>& visited,
                   php::Block& blk) {
  if (visited[blk.id]) return;
  visited[blk.id] = true;
  forEachSuccessor(blk, [&] (BlockId next) {
      postorderWalk(func, out, visited, *func.blocks[next]);
  });
  out.push_back(&blk);
}

}

//////////////////////////////////////////////////////////////////////

std::vector<borrowed_ptr<php::Block>> rpoSortFromMain(const php::Func& func) {
  boost::dynamic_bitset<> visited(func.blocks.size());
  std::vector<borrowed_ptr<php::Block>> ret;
  ret.reserve(func.blocks.size());
  postorderWalk(func, ret, visited, *func.blocks[func.mainEntry]);
  std::reverse(begin(ret), end(ret));
  return ret;
}

std::vector<borrowed_ptr<php::Block>> rpoSortAddDVs(const php::Func& func) {
  boost::dynamic_bitset<> visited(func.blocks.size());
  std::vector<borrowed_ptr<php::Block>> ret;
  ret.reserve(func.blocks.size());
  postorderWalk(func, ret, visited, *func.blocks[func.mainEntry]);

  /*
   * We've already marked the blocks reachable from the main entry
   * point.  Do post order walks from each DV entry with the same
   * visited set (so we'll stop if they chain to the main entry, which
   * is the normal case).
   */
  for (auto it = func.params.end(); it != func.params.begin(); ) {
    --it;
    if (it->dvEntryPoint == NoBlockId) continue;
    postorderWalk(func, ret, visited, *func.blocks[it->dvEntryPoint]);
  }
  std::reverse(begin(ret), end(ret));
  return ret;
}

BlockToBlocks
computeNormalPreds(const std::vector<borrowed_ptr<php::Block>>& rpoBlocks) {
  auto preds = BlockToBlocks{};
  preds.reserve(rpoBlocks.size());
  for (auto& b : rpoBlocks) {
    if (preds.size() < b->id + 1) {
      preds.resize(b->id + 1);
    }
    forEachNormalSuccessor(*b, [&] (BlockId blkId) {
      if (preds.size() < blkId + 1) {
        preds.resize(blkId + 1);
      }
      preds[blkId].insert(b);
    });
  }
  return preds;
}

BlockToBlocks
computeFactoredPreds(const std::vector<borrowed_ptr<php::Block>>& rpoBlocks) {
  auto preds = BlockToBlocks{};
  preds.reserve(rpoBlocks.size());
  for (auto& b : rpoBlocks) {
    if (preds.size() < b->id + 1) {
      preds.resize(b->id + 1);
    }
    for (auto& ex : b->factoredExits) {
      if (preds.size() < ex + 1) {
        preds.resize(ex + 1);
      }
      preds[ex].insert(b);
    }
  }
  return preds;
}

//////////////////////////////////////////////////////////////////////

}}
