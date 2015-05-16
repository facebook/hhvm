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

#ifndef incl_HPHP_VM_CFG_H_
#define incl_HPHP_VM_CFG_H_

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP { namespace jit {

struct IRUnit;
struct Block;

//////////////////////////////////////////////////////////////////////

/*
 * Perform a depth-first postorder walk.
 */
template <class Visitor>
void postorderWalk(const IRUnit&, Visitor visitor, Block* start = nullptr);

/*
 * Compute a postorder list of the basic blocks reachable from the IR's entry
 * block.
 */
BlockList poSortCfg(const IRUnit&);

/*
 * Compute a reverse postorder list of the basic blocks reachable from
 * the IR's entry block.
 */
BlockList rpoSortCfg(const IRUnit&);

/*
 * Take a BlockList, and compute a reverse map from blocks to its index in the
 * list.  Blocks that don't appear in the list will all get the number
 * std::numeric_limits<uint32_t>::max().
 */
using BlockIDs = StateVector<Block,uint32_t>;
BlockIDs numberBlocks(const IRUnit&, const BlockList&);

/*
 * Split the edge between "from" and "to", returning the new middle block.
 */
Block* splitEdge(IRUnit& unit, Block* from, Block* to);

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
 * Pre: `blocks' is in a reverse postorder, and `ids' are the rpoIDs for that
 *      order.
 */
using IdomVector = StateVector<Block,Block*>;
IdomVector findDominators(const IRUnit&,
                          const BlockList& blocks,
                          const BlockIDs& ids);

/*
 * return true if b1 == b2 or if b1 dominates b2.
 */
bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms);

/*
 * Return true iff the CFG has any loops, regardless of whether they are
 * natural loops or not.
 */
bool cfgHasLoop(const IRUnit&);

/*
 * Return a set of the retreating edges in the unit.  These are edges that
 * create (possibly-irreducible) loops in the CFG.
 */
EdgeSet findRetreatingEdges(const IRUnit&);

/*
 * Visit the instructions in this blocklist, in block order.
 */
template <class BlockList, class Body>
void forEachInst(const BlockList& blocks, Body body);

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/cfg-inl.h"

#endif
