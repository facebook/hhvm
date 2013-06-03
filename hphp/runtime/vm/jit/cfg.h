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

#include "hphp/runtime/base/memory/memory_manager.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/trace.h"

namespace HPHP { namespace JIT {

/*
 * This header contains classes and functions for iterating over and
 * inspecting the control flow graph of a Trace's Blocks.
 */

/**
 * PostorderSort encapsulates a depth-first postorder walk
 */
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
    if (taken && taken->trace()->isMain() != block->trace()->isMain()) {
      walk(taken);
      taken = nullptr;
    }
    if (Block* next = block->next()) walk(next);
    if (taken) walk(taken);
    m_visitor(block);
  }

private:
  boost::dynamic_bitset<> m_visited;
  Visitor &m_visitor;
};

/**
 * perform a depth-first postorder walk
 */
template <class Visitor>
void postorderWalk(Visitor visitor, unsigned num_blocks, Block* head) {
  PostorderSort<Visitor> ps(visitor, num_blocks);
  ps.walk(head);
}

/*
 * Compute a reverse postorder list of the basic blocks reachable from
 * the first block in trace.
 *
 * Post: isRPOSorted(return value)
 */
BlockList rpoSortCfg(Trace*, const IRFactory&);

/*
 * Return an iterator into an rpo-sorted BlockList for a given Block.
 * Uses the postId() assigned by rpoSortCfg.
 *
 * Pre: isRPOSorted(cfg)
 */
inline BlockList::const_iterator
rpoIteratorTo(const BlockList& cfg, Block* b) {
  return cfg.end() - (b->postId() + 1);
}

/*
 * Returns: true if the supplied block list is sorted in reverse post
 * order.
 */
bool isRPOSorted(const BlockList&);

/*
 * Compute the postorder number of each immediate dominator of each
 * block, using a list produced by rpoSortCfg().
 *
 * Pre: isRPOSorted(blocks)
 */
typedef smart::vector<int> IdomVector;
IdomVector findDominators(const BlockList& blocks);

/*
 * A vector of children lists, indexed by block->postId()
 */
typedef smart::vector<BlockList> DomChildren;

/*
 * Compute the dominator tree, then populate a list of dominator children
 * for each block.  Note that DomChildren is indexed by block->postId(),
 * not block->id(); that's why we don't use StateVector here.
 */
DomChildren findDomChildren(const BlockList& blocks);

/*
 * return true if b1 == b2 or if b1 dominates b2.
 */
bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms);

/*
 * Return true if trace has internal control flow (IE it has a branch
 * to itself somewhere.
 */
bool hasInternalFlow(Trace*);

/*
 * Visit basic blocks in a preorder traversal over the dominator tree.
 * The state argument is passed by value (copied) as we move down the tree,
 * so each child in the tree gets the state after the parent was processed.
 * The body lambda should take State& (by reference) so it can modify it
 * as each block is processed.
 */
template <class State, class Body>
void forPreorderDoms(Block* block, const DomChildren& children,
                     State state, Body body) {
  body(block, state);
  for (Block* child : children[block->postId()]) {
    forPreorderDoms(child, children, state, body);
  }
}

/*
 * Visit the main trace followed by exit traces.
 */
template <class Body>
void forEachTrace(Trace* main, Body body) {
  body(main);
  for (Trace* exit : main->exitTraces()) {
    body(exit);
  }
}

/*
 * Visit the blocks in the main trace followed by exit trace blocks.
 */
template <class Body>
void forEachTraceBlock(Trace* main, Body body) {
  for (Block* block : main->blocks()) {
    body(block);
  }
  for (Trace* exit : main->exitTraces()) {
    for (Block* block : exit->blocks()) {
      body(block);
    }
  }
}

/*
 * Visit the instructions in this trace, in block order.
 */
template <class BlockList, class Body>
void forEachInst(const BlockList& blocks, Body body) {
  for (Block* block : blocks) {
    for (IRInstruction& inst : *block) {
      body(&inst);
    }
  }
}

template <class Body>
void forEachInst(Trace* trace, Body body) {
  forEachInst(trace->blocks(), body);
}

/*
 * Visit each instruction in the main trace, then the exit traces
 */
template <class Body>
void forEachTraceInst(Trace* main, Body body) {
  forEachTrace(main, [=](Trace* t) {
    forEachInst(t, body);
  });
}

}}

#endif
