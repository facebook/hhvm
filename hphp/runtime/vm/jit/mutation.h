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

#ifndef incl_HPHP_JIT_MUTATION_H_
#define incl_HPHP_JIT_MUTATION_H_

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP { namespace jit {

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
 * Pass that walks over the dominator tree and replaces uses of SSATmps that
 * have more-refined versions available with uses of the more refined versions.
 * This basically fixes patterns like this:
 *
 *    t1 = LdLoc<Cell>
 *    t2 = CheckType<Obj> t1 -> ...
 *    IncRef t1
 *
 * So that we can replace t1 with t2 in the parts of the code dominated by the
 * definition of t2.
 */
void refineTmps(IRUnit&, const BlockList&, const IdomVector&);

/*
 * Insert a phi for inputs in blk. none of blk's input edges may be
 * critical, blk->numPreds() must be equal to inputs.size(), and
 * inputs.size() must be greater than 1.
 * If there's already a suitable phi, it will be returned.
 */
SSATmp* insertPhi(IRUnit&, Block* blk, const jit::vector<SSATmp*>& inputs);

/*
 * Remove the i'th dest from `label' and all incoming Jmps, returning its
 * SSATmp*. May leave the DefLabel with 0 dests, in which case the instruction
 * may be deleted by the caller.
 */
SSATmp* deletePhiDest(IRInstruction* label, unsigned i);

//////////////////////////////////////////////////////////////////////

}}

#endif
