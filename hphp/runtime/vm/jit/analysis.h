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
#ifndef incl_HPHP_ANALYSIS_H_
#define incl_HPHP_ANALYSIS_H_

namespace HPHP { namespace jit {

struct SSATmp;
struct IRInstruction;
struct Block;

//////////////////////////////////////////////////////////////////////

/*
 * Utility routines for performing very simple queries on the IR, that may be
 * needed from multiple optimization passes, or even during IR generation.
 *
 * The functions in this module never mutate the IR, even if they take
 * pointers-to-non-const.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Returns the canonical version of the given value by tracing through any
 * passthrough instructions (Mov, CheckType, etc...).
 */
const SSATmp* canonical(const SSATmp*);
SSATmp* canonical(SSATmp*);

/*
 * Assuming sp is the VM stack pointer either from inside an FPI region or an
 * inlined call, find the SpillFrame instruction that defined the current
 * frame. Returns nullptr if the frame can't be found.
 */
IRInstruction* findSpillFrame(SSATmp* sp);

/*
 * Given a non-const SSATmp `t', return the earliest block B such that `t' is
 * defined on all of B's outgoing edges, and `t' is defined in all blocks
 * dominated by B.
 *
 * Such a block may not exist if the CFG has critical edges, so this function
 * may return nullptr.
 *
 * Details: we have several instructions that conditionally define values on
 * their fallthrough edge---if this fallthrough edge is a critical edge, the
 * value is actually only defined on that edge, and there is no block with the
 * desired properties.
 *
 * A normal use for this function is when you have computed that an SSATmp has
 * the same value as another computation, but want to know if it is defined at
 * some program point so you can add a new use to it.  The precondition that
 * `t' is not const is because this function makes no sense for that use case
 * for constants, which are defined everywhere.
 *
 * Pre: !t->isConst()
 */
Block* findDefiningBlock(const SSATmp* t);

/*
 * Finds the least common ancestor of two SSATmps.  A temp has a `parent' for
 * purposes of finding this LCA if it is the result of a passthrough
 * instruction.  Either argument to this function may be nullptr.
 *
 * Returns nullptr when there is no LCA.
 */
SSATmp* least_common_ancestor(SSATmp*, SSATmp*);

//////////////////////////////////////////////////////////////////////

}}


#endif
