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

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP::jit {

struct IRInstruction;
struct IRUnit;

//////////////////////////////////////////////////////////////////////

/*
 * Utility routines for mutating the IR during optimization passes.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Clone a range of IRInstructions into the front of a target block
 * (immediately after its DefLabel).
 *
 * Then, for any block reachable from `target', rewrite the sources of
 * any instructions that referred to the old destinations of the
 * cloned instructions so that they refer to the new destinations this
 * function created.
 *
 * Does not unlink the instructions from the source block.
 *
 * Pre: The range [first,last) may not contain control flow
 *      instructions.
 * Pre: blocks is in reverse postorder
 */
void cloneToBlock(const BlockList& blocks,
                  IRUnit& unit,
                  Block::iterator first,
                  Block::iterator last,
                  Block* target);

/*
 * Move a range of IRInstructions to the front of a target block
 * (immediately after its DefLabel, but before its Marker).
 *
 * The instructions are unlinked from their current source block.
 *
 * Pre: The range [first,last) may not contain control flow
 * instructions.
 */
void moveToBlock(Block::iterator first,
                 Block::iterator last,
                 Block* dst);

/*
 * Walk the cfg of the given unit, recomputing output types of all instructions
 * from their inputs.
 *
 * The new types of any changed SSATmps must be related to their old
 * types.  However, notice that the new types may result in
 * inconsistent operand types for instructions that are unreachable
 * (but not yet removed from the IR unit).
 */
void reflowTypes(IRUnit&);

/*
 * Recomputes the output type of each of inst's dests.
 *
 * Returns: true if any of the instruction's dest types were changed.
 */
bool retypeDests(IRInstruction*, const IRUnit*);

/*
 * Small pass that inserts AssertTypes on the taken edges of CheckType blocks
 * that may add type information about failing checks.  This pass is probably
 * worth running before refineTmps, because it may allow better types when
 * checks fail.
 *
 * (Note: the supplied block list need not be RPO-sorted.)
 */
void insertNegativeAssertTypes(IRUnit&, const BlockList&);

/*
 * Pass that rewrites the unit and replaces any SSATmp that has a more
 * refined version available with that more refined version. A SSATmp
 * is considered more refined if it is the destination of a
 * pass-through instruction.
 *
 * This basically fixes patterns like this:
 *
 *    t1 = LdLoc<Cell>
 *    t2 = CheckType<Obj> t1 -> ...
 *    IncRef t1
 *
 * This pass guarantees that no instruction reachable from a
 * pass-through instruction (on the next edge if a branch) uses the
 * source of the pass-through instruction. Other passes are allowed to
 * rely on this post invariant.
 *
 * Not only does this ensure we always rely on the most refined type,
 * it ensures that only one logical copy of a given value is live at a
 * given time.
 */
void refineTmps(IRUnit&);

/*
 * Insert a phi for inputs in blk. none of blk's input edges may be
 * critical, blk->numPreds() must be equal to inputs.size(), and
 * inputs.size() must be greater than 1.
 *
 * If `forceNew' is false, and if there's already a suitable phi, it
 * will be returned. If true, a new SSATmp will always be created.
 */
SSATmp* insertPhi(IRUnit&, Block* blk,
                  const jit::hash_map<Block*, SSATmp*>& inputs,
                  bool forceNew = false);

/*
 * Remove the i'th dest from `label' and all incoming Jmps, returning its
 * SSATmp*. May leave the DefLabel with 0 dests, in which case the instruction
 * may be deleted by the caller.
 */
SSATmp* deletePhiDest(IRInstruction* label, unsigned i);

//////////////////////////////////////////////////////////////////////

}
