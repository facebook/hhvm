/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_LOOP_ANALYSIS_H_
#define incl_HPHP_LOOP_ANALYSIS_H_

#include <string>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/edge.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

struct IRUnit;

//////////////////////////////////////////////////////////////////////

/*
 * Each natural loop has a unique id, which is its index in the `naturals'
 * vector in a LoopAnalysis.  We use these ids to link related loops together
 * (to represent the loop nesting structure in the cfg).
 */
using LoopID = uint32_t;
constexpr auto kInvalidLoopID = -LoopID{1};

//////////////////////////////////////////////////////////////////////

struct NaturalLoopInfo {
  LoopID id;

  /*
   * The next loop that shares the same header as this loop.  The first loop in
   * any `header_next' list is called the canonical natural loop for that
   * header.  See LoopAnalysis::headers.
   *
   * These loops may have overlapping blocks in their members list, and may or
   * may not have a nesting structure, but we don't represent that.
   *
   * You can find the first loop with a particular header by looking in
   * LoopAnalysis's headers list.
   */
  LoopID header_next{kInvalidLoopID};

  /*
   * Pointer to the canonical outer loop.
   *
   * This is the canonical natural loop for a header block that has at least
   * one natural loop which contains this loop.  Note that this loop may be
   * contained in more than one of the natural loops from that header.
   */
  LoopID canonical_outer{kInvalidLoopID};

  /*
   * The header dominates all nodes in the loop (members), and is the target of
   * the loop's back edge.
   */
  Block* header;

  /*
   * The `pre_header' is not part of the loop, and does not necessarily exist,
   * so it may be nullptr.
   *
   * If it exists, it is a block which has the loop header as its only
   * successor, and that is the only predecessor of `header' other than
   * predecessors via back-edges.
   *
   * Note that this definition of `pre_header' still allows the actual loop
   * header to have some predecessors that are not part of this loop, even when
   * `pre_header' is not nullptr.  Specifically, there may be back-edge
   * predecessors of the header that are not part of this natural loop, in
   * situations with multiple loops that share the same header.  This also
   * means different natural loops may have the same pre_header block, if (and
   * only if) they have the same header.
   */
  Block* pre_header{nullptr};

  /*
   * The members of this natural loop, excluding the header block.  The header
   * dominates each loop member, and each loop member has a path to the back
   * edge.
   */
  jit::flat_set<Block*> members;
};

/*
 * Information about loops in the CFG.
 */
struct LoopAnalysis {
  explicit LoopAnalysis(uint32_t numBlocks)
    : headers(numBlocks)
  {}

  /*
   * The set of back edges.
   *
   * The target of a back edge dominates its source, and each back edge is
   * associated with exactly one of the natural loop entries in `naturals'.
   */
  jit::flat_set<Edge*> back_edges;

  /*
   * The natural loops in the CFG.  Each natural loop is defined by the set of
   * CFG nodes that can reach a particular back edge without going through the
   * block the back edge targets (the loop header).
   *
   * We do not combine non-nested loops with the same header into the same
   * NaturalLoopInfo, so there is exactly one entry in `naturals' for each back
   * edge.
   */
  jit::vector<NaturalLoopInfo> naturals;

  /*
   * The first natural loop in the list for each loop header block.  This is
   * called the "canonical" natural loop for that header.
   *
   * Note that the size of this sparse map is based on the number of blocks
   * when this LoopAnalysis structure was created.  If more blocks are added
   * after that, you may have blocks outside of its universe.
   */
  sparse_idptr_map<Block,LoopID> headers;

  /*
   * List of natural loops that have no loops nested inside them.  If
   * inner_loops.size() == naturals.size(), there are no nested loops in the
   * program.
   */
  jit::vector<LoopID> inner_loops;
};

//////////////////////////////////////////////////////////////////////

/*
 * Produce a LoopAnalysis structure that contains information about loops in
 * the CFG.
 */
LoopAnalysis identify_loops(const IRUnit&, const BlockList& rpoBlocks);

/*
 * Given a LoopID for the canonical natural loop with a given header block,
 * return a set of all the Blocks that are members of any natural loop with
 * that header.  The returned set does not include the header block.
 */
jit::flat_set<Block*> expanded_loop_blocks(const LoopAnalysis&, LoopID);

//////////////////////////////////////////////////////////////////////

/*
 * Modify the CFG to create a pre-header for a natural loop, and update an
 * existing LoopAnalysis structure to reflect the changes.
 *
 * Guaranteed not to change the number of loops or otherwise invalidate
 * references or iterators to the vectors in the LoopAnalysis.  It will change
 * the number of blocks and invalidate any IdomVectors, though.
 */
void insert_loop_pre_header(IRUnit&, LoopAnalysis&, LoopID);

/*
 * Update containing loop member lists to reflect a newly inserted pre-header
 * in a possibly nested loop.
 *
 * If you are inserting blocks in CFG, generally speaking you should consider a
 * LoopAnalysis invalidated.  In the special case that a loop pre header is
 * being inserted, if the pre_header fields in NaturalLoopInfo are maintained,
 * calling this function will keep the rest of the LoopAnalysis valid as well.
 */
void update_pre_header(LoopAnalysis&,
                       LoopID canonical_loop_id,
                       Block* new_pre_header);

//////////////////////////////////////////////////////////////////////

/*
 * Summary information for debugging.
 */
std::string show(const LoopAnalysis&);

//////////////////////////////////////////////////////////////////////

}}

#endif
