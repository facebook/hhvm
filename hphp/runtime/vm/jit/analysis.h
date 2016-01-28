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
#ifndef incl_HPHP_ANALYSIS_H_
#define incl_HPHP_ANALYSIS_H_

#include "hphp/runtime/vm/jit/cfg.h"

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
 * Given a non-const SSATmp `t', return the earliest block B such that `t' is
 * defined on all of B's outgoing edges, and `t' is defined in all blocks
 * dominated by B.
 *
 * Such a block may not exist if the CFG has critical edges, so this function
 * may return nullptr.
 *
 * Details: We have several instructions that conditionally define values on
 * their fallthrough edge.  If that edge goes to a block with other
 * predecessors, this function only returns that block if it also dominates
 * the sources of its other predecessors.  Otherwise, the value is only
 * really defined in that edge -- but in no block in the unit.
 *
 * A normal use for this function is when you have computed that an SSATmp has
 * the same value as another computation, but want to know if it is defined at
 * some program point so you can add a new use to it.  The precondition that
 * `t' is not the result of a DefConst is because this function makes no sense
 * for that instruction, which is used to produce values that are defined
 * everywhere.
 *
 * Pre: !t->inst()->is(DefConst)
 */
Block* findDefiningBlock(const SSATmp* t, const IdomVector& idoms);

/*
 * Returns true if the SSATmp `tmp' is safely usable in the block `where',
 * based only on dominator relationships.  This function will return true even
 * if there's a PHP call between the `tmp's definition and `where'.  The callee
 * must ensure not to add uses of tmps in that situation.
 */
bool is_tmp_usable(const IdomVector&, const SSATmp* tmp, const Block* where);

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
