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
#ifndef incl_HPHP_CFG_INL_H_
#define incl_HPHP_CFG_INL_H_

#include <boost/dynamic_bitset.hpp>

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

namespace detail {

// PostorderSort encapsulates a depth-first postorder walk.
template <class Visitor>
struct PostorderSort {
  PostorderSort(Visitor& visitor, unsigned num_blocks)
    : m_visited(num_blocks), m_visitor(visitor)
  {}

  void walk(Block* block) {
    if (m_visited.test(block->id())) return;
    m_visited.set(block->id());

    // Blocks aren't allowed to be empty but this function is used when
    // printing debug information, so we want to handle invalid Blocks
    // gracefully.
    if (!block->empty()) {
      // If we're not cold but we have two successors and exactly one of them
      // is cold, we visit the cold one last so it appears as early as
      // possible in an RPO sort. This causes better memory usage patterns in
      // traces with lots of exit blocks in certain optimization passes. Note
      // that these are just heuristics; all possible outcomes are valid
      // post-order traversals and should not affect correctness.

      auto next = block->next();
      auto taken = block->taken();
      if (!cold(block) && next && taken && (cold(next) ^ cold(taken))) {
        if (cold(next)) {
          walk(taken);
          taken = nullptr;
        } else {
          walk(next);
          next = nullptr;
        }
      }

      if (taken) walk(taken);
      if (next) walk(next);
    }
    m_visitor(block);
  }
private:
  static bool cold(Block* b) {
    return b->hint() == Block::Hint::Unlikely ||
           b->hint() == Block::Hint::Unused;
  }
private:
  boost::dynamic_bitset<> m_visited;
  Visitor &m_visitor;
};

}

//////////////////////////////////////////////////////////////////////

/**
 * Perform a depth-first postorder walk. If a starting Block is not supplied,
 * unit's entry Block will be used.
 */
template <class Visitor>
void postorderWalk(const IRUnit& unit, Visitor visitor, Block* start) {
  detail::PostorderSort<Visitor> ps(visitor, unit.numBlocks());
  ps.walk(start ? start : unit.entry());
}

template <class BlockList, class Body>
void forEachInst(const BlockList& blocks, Body body) {
  for (Block* block : blocks) {
    for (IRInstruction& inst : *block) {
      body(&inst);
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}


#endif
