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
#pragma once

#include <vector>

#include <boost/variant/static_visitor.hpp>
#include <boost/container/flat_set.hpp>

#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

namespace detail {

template<class Fun>
void visitExnLeaves(const php::Func& func, const php::ExnNode& n, Fun f) {
  if (n.idx == NoExnNodeId) return;
  for (auto& c : n.children) visitExnLeaves(func, func.exnNodes[c], f);
  f(n);
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Returns whether a block consists of a single Nop instruction.
 */
inline bool is_single_nop(const php::Block& b) {
  return b.hhbcs.size() == 1 && b.hhbcs.back().op == Op::Nop;
}

/*
 * Returns whether a block consists of a single Throw instruction.
 */
inline bool is_single_throw(const php::Block& b) {
  return b.hhbcs.size() == 1 && b.hhbcs.back().op == Op::Throw;
}

inline bool is_unsetl_throw(const php::Block& b) {
  if (b.hhbcs.empty() || b.hhbcs.back().op != Op::Throw) return false;
  for (auto const& bc : b.hhbcs) {
    switch (bc.op) {
      case Op::Nop:
      case Op::UnsetL:
      case Op::Throw:
        break;
      default:
        return false;
    }
  }
  return true;
}

/*
 * Walk through single_nop blocks to the next block that actually does
 * something.
 */
BlockId next_real_block(const php::WideFunc& func, BlockId id);

/*
 * Walk through catch blocks, stopping at the first block which does
 * something other than just immediately throw.
 */
std::pair<BlockId, ExnNodeId>
next_catch_block(const php::WideFunc& func, BlockId id, ExnNodeId exnId);

/*
 * Call a function for every jump target of a given bytecode, or Op.
 * If the bytecode has no targets, the function is not called.
 */
template<typename Bc, class Fun>
void forEachTakenEdge(Bc& b, Fun f) {
  b.forEachTarget(f);
}

/*
 * Call a function for every successor of `block'.
 *
 * Order unspecified, and the types of successor edges are not
 * distinguished.
 *
 * Exit edges are traversed only if the block consists of
 * more than a single Nop instruction.
 */
template<class Fun>
void forEachSuccessor(const php::Block& block, Fun f) {
  if (!is_single_nop(block)) {
    forEachTakenEdge(block.hhbcs.back(), f);
    if (block.throwExit != NoBlockId) f(block.throwExit);
  }
  if (block.fallthrough != NoBlockId) f(block.fallthrough);
}

/*
 * Call a function for every successor of `block' that is reachable
 * through a fallthrough or taken edge.
 */
template<class Block, class Fun>
void forEachNormalSuccessor(Block& block, Fun f) {
  forEachTakenEdge(block.hhbcs.back(), f);
  if (block.fallthrough != NoBlockId) f(block.fallthrough);
}

/*
 * Call a function for every successor of `block' that is reachable
 * through a non-throw edge.
 */
template<class Fun>
void forEachNonThrowSuccessor(const php::Block& block, Fun f) {
  forEachTakenEdge(block.hhbcs.back(), f);
  if (block.fallthrough != NoBlockId) f(block.fallthrough);
}

/*
 * Obtain the blocks for a function in a reverse post order, starting
 * with the specified block.  The exact order is not specified.
 */
std::vector<BlockId> rpoSortFromBlock(const php::WideFunc&, BlockId);

/*
 * Obtain the blocks for a function in a reverse post order, starting
 * with the main entry point.  The exact order is not specified.
 *
 * DV initializer blocks will not appear in this list.
 */
std::vector<BlockId> rpoSortFromMain(const php::WideFunc&);

/*
 * Obtain the blocks for a function in a reverse post order, taking
 * into account all entry points.
 *
 * This can be thought of as an RPO on the CFG of Func starting from a
 * virtual empty "entry" block, with edges to each DV entry point and
 * an edge to the main entry point.
 */
std::vector<BlockId> rpoSortAddDVs(const php::WideFunc&);

/*
 * Mappings from blocks to sets of blocks.
 *
 * The first level is indexed by block id.  The second is a set of
 * block pointers.
 */
using BlockToBlocks = std::vector<
  boost::container::flat_set<BlockId>
>;

/*
 * Find the immediate non-throw predecessors for each block in
 * an RPO-sorted list of blocks.
 *
 * The BlockToBlocks map returned will have any entry for each block
 * in the input array, but may not have entries for blocks that aren't
 * in the list.
 */
BlockToBlocks
computeNonThrowPreds(const php::WideFunc&, const std::vector<BlockId>&);

/*
 * Find the immediate throw predecessors for each block in an
 * RPO-sorted list of blocks.
 *
 * The BlockToBlocks map returned will have any entry for each block
 * in the input array, but may not have entries for blocks that aren't
 * in the list.
 */
BlockToBlocks
computeThrowPreds(const php::WideFunc&, const std::vector<BlockId>&);

/*
 * Visit each leaf in the ExnNode tree.
 */
template<class Fun>
void visitExnLeaves(const php::Func& func, Fun f) {
  for (auto& n : func.exnNodes) {
    detail::visitExnLeaves(func, n, f);
  }
}

//////////////////////////////////////////////////////////////////////

}
