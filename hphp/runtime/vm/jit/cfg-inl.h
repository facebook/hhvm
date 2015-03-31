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
template<class Visitor>
struct PostorderSort {
  PostorderSort(Visitor& visitor, unsigned num_blocks)
    : m_visited(num_blocks), m_visitor(visitor)
  {}

  void walk(Block* block) {
    if (m_visited.test(block->id())) return;
    m_visited.set(block->id());
    // Valid blocks can't be empty, but we can see empty blocks here
    // when trying to print a unit before it's finished for debugging.
    if (block->empty()) return;
    if (auto const t = block->taken()) walk(t);
    if (auto const n = block->next()) walk(n);
    m_visitor(block);
  }

private:
  boost::dynamic_bitset<> m_visited;
  Visitor& m_visitor;
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
