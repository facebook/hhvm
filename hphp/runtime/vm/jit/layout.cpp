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
#include "hphp/runtime/vm/jit/layout.h"

#include <chrono>
#include <random>

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

void postorderWalk(BlockList& out,
                   StateVector<Block,bool>& visited,
                   Block* block) {
  if (visited[block]) return;
  visited[block] = true;
  if (auto t = block->taken()) postorderWalk(out, visited, t);
  if (auto n = block->next())  postorderWalk(out, visited, n);
  out.push_back(block);
}

BlockList rpoForCodegen(const IRUnit& unit) {
  StateVector<Block,bool> visited(unit, false);
  BlockList ret;
  ret.reserve(unit.numBlocks());
  postorderWalk(ret, visited, unit.entry());
  std::reverse(ret.begin(), ret.end());
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Currently we have very limited control flow in any given tracelet,
 * so this just selects an appropriate reverse post order on the
 * blocks, and partitions the unlikely ones to astubs.
 */
LayoutInfo layoutBlocks(const IRUnit& unit) {
  LayoutInfo ret;
  ret.blocks = rpoForCodegen(unit);

  // Optionally stress test by randomizing the positions.
  if (RuntimeOption::EvalHHIRStressCodegenBlocks) {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::random_shuffle(ret.blocks.begin() + 1, ret.blocks.end(),
                        [&](int i) { return gen() % i; });
  }

  // Partition into a and astubs, without changing relative order.
  ret.astubsIt = std::stable_partition(
    ret.blocks.begin(), ret.blocks.end(),
    [&] (Block* b) { return b->hint() != Block::Hint::Unlikely; }
  );

  if (HPHP::Trace::moduleEnabled(HPHP::Trace::hhir, 5)) {
    std::string str = "Layout:";

    auto printRegion = [&] (const char* what,
                            BlockList::iterator& it,
                            BlockList::iterator stop) {
      folly::toAppend(what, &str);
      for (; it != stop; ++it) {
        folly::toAppend((*it)->id(), &str);
        folly::toAppend(" ", &str);
      }
    };

    auto it = ret.blocks.begin();
    printRegion("\n       a: ", it, ret.astubsIt);
    printRegion("\n  astubs: ", it, ret.blocks.end());

    HPHP::Trace::traceRelease("%s\n", str.c_str());
  }

  /*
   * No matter what happens above, it's going to be very broken if the
   * entry block isn't first, and it's going to perform poorly if the
   * main exit isn't the last block in a.  Assert these.
   *
   * Note: this isn't the case if the main exit contains a return, but
   * we can revisit that later.
   */
  if (!RuntimeOption::EvalHHIRStressCodegenBlocks) {
    always_assert(ret.blocks.front()->isEntry());
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
