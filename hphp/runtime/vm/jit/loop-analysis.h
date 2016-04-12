/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
 * Each loop has a unique id, which is its index in the `loops' vector
 * in a LoopAnalysis.  We use these ids to link related loops together
 * (to represent the loop nesting structure in the CFG).
 */
using LoopID = uint32_t;
constexpr auto kInvalidLoopID = -LoopID{1};

//////////////////////////////////////////////////////////////////////

struct LoopInfo {
  LoopID id;

  /*
   * Parent loop in the loop forest, if any.
   */
  LoopID parent{kInvalidLoopID};

  /*
   * The header dominates all blocks in the loop, and is the target of
   * the loop's back-edges.
   */
  Block* header;

  /*
   * The `preHeader' is not part of the loop, and does not necessarily exist,
   * so it may be nullptr.
   *
   * If it exists, it is a block which has the loop header as its only
   * successor, and that is the only predecessor of `header' other than
   * predecessors via back-edges.
   */
  Block* preHeader{nullptr};

  /*
   * The `preExit' is an exit block right before the loop that is
   * suitable to be the target of check instructions that are hoisted
   * out of the loop.  It may not exist, and we may not be able to
   * create one, which is only guaranteed if the loop starts with an
   * ExitPlaceholder instruction (optionally preceded by a DefLabel).
   * Therefore, any optimization that wants to rely on the existence
   * of a `preExit' must also make sure to insert an ExitPlaceholder
   * in the loop headers (see call to irgen::makeExitPlaceholder() in
   * irGenRegion(), in translate-region.cpp).
   */
  Block* preExit{nullptr};

  /*
   * The blocks within this loop.
   */
  jit::flat_set<Block*> blocks;

  /*
   * The loop's back-edges.
   */
  jit::flat_set<Edge*> backEdges;

  /*
   * Number of times that the loop was invoked (i.e. entered) during
   * profiling.
   */
  uint64_t numInvocations{0};
};

/*
 * Information about loops in the CFG.
 */
struct LoopAnalysis {
  explicit LoopAnalysis(uint32_t numBlocks)
    : headers(numBlocks)
  {}

  /*
   * The set of back-edges.
   */
  jit::flat_set<Edge*> backEdges;

  /*
   * The loops in the CFG.  Each loop contains a header block and a
   * set of blocks that are dominated by the header and that can reach
   * the header in the reverse CFG starting at any of the loop's
   * back-edges.  Notice that multiple back-edges sharing the same
   * target (header) block are thus part of the same loop.
   */
  jit::vector<LoopInfo> loops;

  /*
   * This maps blocks that are loop headers to their corresponding LoopID.
   *
   * Note that the size of this sparse map is based on the number of blocks
   * when this LoopAnalysis structure was created.  If more blocks are added
   * after that, you may have blocks outside of its universe.
   */
  sparse_idptr_map<Block, LoopID> headers;

  /*
   * List of inner-most loops in the CFG.
   */
  jit::vector<LoopID> innerLoops;
};

//////////////////////////////////////////////////////////////////////

/*
 * Produce a LoopAnalysis structure that contains information about loops in
 * the CFG.
 */
LoopAnalysis identifyLoops(const IRUnit&, const BlockList& rpoBlocks);

//////////////////////////////////////////////////////////////////////

/*
 * Modify the CFG to create a pre-header for a natural loop, and update an
 * existing LoopAnalysis structure to reflect the changes.
 *
 * Guaranteed not to change the number of loops or otherwise invalidate
 * references or iterators to the vectors in the LoopAnalysis.  It will change
 * the number of blocks and invalidate any IdomVectors, though.
 */
void insertLoopPreHeader(IRUnit&, LoopAnalysis&, LoopID);

/*
 * Update containing loop member lists to reflect a newly inserted pre-header
 * in a possibly nested loop.
 *
 * If you are inserting blocks in the CFG, generally speaking you should
 * consider a LoopAnalysis invalidated.  In the special case that a loop
 * pre-header is being inserted, if the pre_header fields in LoopInfo are
 * maintained, calling this function will keep the rest of the LoopAnalysis
 * valid as well.
 */
void updatePreHeader(LoopAnalysis&, LoopID loopId, Block* preHeader);

Block* insertLoopPreExit(IRUnit& unit, LoopAnalysis& la, LoopID loopId);

//////////////////////////////////////////////////////////////////////

/*
 * Summary information for debugging.
 */
std::string show(const LoopAnalysis&);

//////////////////////////////////////////////////////////////////////

}}

#endif
