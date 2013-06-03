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

#ifndef incl_HPHP_JIT_MUTATION_H_
#define incl_HPHP_JIT_MUTATION_H_

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * Utility routines for mutating the IR during optimization passes.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Clone a range of IRInstructions into the front of a target block
 * (immediately after its DefLabel, but before its Marker).
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
 * Pre: isRPOSorted(blocks)
 */
void cloneToBlock(const BlockList& blocks,
                  IRFactory* irFactory,
                  Block::iterator first,
                  Block::iterator last,
                  Block* dst);


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
 * Given a block where some SSATmps have changed type, walk the
 * sub-cfg reachable from that block and recompute the types of any
 * tmps that depend on it.
 *
 * The new types of any changed SSATmps must be related to their old
 * types.
 *
 * Pre: isRPOSorted(blocks)
 */
void reflowTypes(Block*, const BlockList& blocks);

//////////////////////////////////////////////////////////////////////

}}

#endif
