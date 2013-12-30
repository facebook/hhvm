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

#ifndef incl_HPHP_VM_CFG_H_
#define incl_HPHP_VM_CFG_H_

#include <boost/dynamic_bitset.hpp>

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ir-trace.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP { namespace JIT {

/**
 * perform a depth-first postorder walk
 */
template <class Visitor>
void postorderWalk(const IRUnit&, Visitor visitor);

/*
 * Return an iterator into an rpo-sorted BlockList for a given Block.
 * Uses the postId() assigned by rpoSortCfg.
 *
 * Pre: isRPOSorted(cfg)
 */
BlockList::const_iterator rpoIteratorTo(const BlockList& cfg, Block* b);

/*
 * Compute a reverse postorder list of the basic blocks reachable from
 * the IR's entry block. Updates the postId field of each reachable Block.
 *
 * Post: isRPOSorted(return value)
 */
BlockList rpoSortCfg(const IRUnit&);

/*
 * Returns: true if the supplied block list is sorted in reverse post
 * order.
 */
bool isRPOSorted(const BlockList&);

/*
 * Removes unreachable blocks from the unit and then splits any critical edges.
 *
 * Returns: true iff any modifications were made to the unit.
 */
bool splitCriticalEdges(IRUnit&);

/*
 * Remove unreachable blocks from the given unit.
 *
 * Returns: true iff one or more blocks were deleted.
 */
bool removeUnreachable(IRUnit& unit);

/*
 * Compute the postorder number of each immediate dominator of each
 * block, using a list produced by rpoSortCfg().
 *
 * Pre: isRPOSorted(blocks)
 */
typedef StateVector<Block,Block*> IdomVector;
IdomVector findDominators(const IRUnit&, const BlockList& blocks);

/*
 * A vector of children lists, indexed by block
 */
typedef StateVector<Block,BlockList> DomChildren;

/*
 * Compute the dominator tree, then populate a list of dominator children
 * for each block.
 */
DomChildren findDomChildren(const IRUnit&, const BlockList& blocks);

/*
 * return true if b1 == b2 or if b1 dominates b2.
 */
bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms);

/*
 * Visit basic blocks in a preorder traversal over the dominator tree.
 * The state argument is passed by value (copied) as we move down the tree,
 * so each child in the tree gets the state after the parent was processed.
 * The body lambda should take State& (by reference) so it can modify it
 * as each block is processed.
 */
template <class State, class Body>
void forPreorderDoms(Block* block, const DomChildren& children,
                     State state, Body body);

/*
 * Visit the main trace followed by exit traces.
 */
template <class Body> void forEachTrace(const IRUnit&, Body body);

/*
 * Visit the instructions in this blocklist, in block order.
 */
template <class BlockList, class Body>
void forEachInst(const BlockList& blocks, Body body);

/*
 * Visit each instruction in the main trace, then the exit traces
 */
template <class Body> void forEachTraceInst(const IRUnit&, Body body);

namespace detail {
   // PostorderSort encapsulates a depth-first postorder walk
  template <class Visitor>
  struct PostorderSort {
    PostorderSort(Visitor& visitor, unsigned num_blocks)
      : m_visited(num_blocks), m_visitor(visitor)
    {}

    void walk(Block* block) {
      assert(!block->empty());
      if (m_visited.test(block->id())) return;
      m_visited.set(block->id());
      Block* taken = block->taken();
      if (taken && !cold(block) && cold(taken)) {
        walk(taken);
        taken = nullptr;
      }
      if (Block* next = block->next()) walk(next);
      if (taken) walk(taken);
      m_visitor(block);
    }
  private:
    static bool cold(Block* b) { return b->hint() == Block::Hint::Unlikely; }
  private:
    boost::dynamic_bitset<> m_visited;
    Visitor &m_visitor;
  };
}

/**
 * perform a depth-first postorder walk
 */
template <class Visitor>
void postorderWalk(const IRUnit& unit, Visitor visitor) {
  detail::PostorderSort<Visitor> ps(visitor, unit.numBlocks());
  ps.walk(unit.entry());
}

inline
BlockList::const_iterator rpoIteratorTo(const BlockList& cfg, Block* b) {
  return cfg.end() - (b->postId() + 1);
}

template <class State, class Body>
void forPreorderDoms(Block* block, const DomChildren& children,
                     State state, Body body) {
  body(block, state);
  for (Block* child : children[block]) {
    forPreorderDoms(child, children, state, body);
  }
}

template <class Body>
void forEachTrace(IRUnit& unit, Body body) {
  body(unit.main());
  for (auto* exit : unit.exits()) {
    body(exit);
  }
}

template <class Body>
void forEachTrace(const IRUnit& unit, Body body) {
  body(unit.main());
  for (auto* exit : unit.exits()) {
    body(exit);
  }
}

template <class BlockList, class Body>
void forEachInst(const BlockList& blocks, Body body) {
  for (Block* block : blocks) {
    for (IRInstruction& inst : *block) {
      body(&inst);
    }
  }
}

template <class Body>
void forEachTraceInst(IRUnit& unit, Body body) {
  forEachTrace(unit, [=](IRTrace* t) {
    forEachInst(t->blocks(), body);
  });
}

}}

#endif
