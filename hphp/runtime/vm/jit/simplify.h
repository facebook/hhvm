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

#ifndef incl_HPHP_HHVM_HHIR_SIMPLIFIER_H_
#define incl_HPHP_HHVM_HHIR_SIMPLIFIER_H_

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"

namespace HPHP { namespace jit {

struct IRInstruction;
struct IRUnit;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * Module that handles state-independent optimizations, or queries IR for
 * state-independent properties.
 *
 * The optimizations in this module should be those that we can do based only
 * on chasing the use-def chain.  Functions like getStackValue are in here
 * because they are similarly restricted.
 *
 * The line of separation between this and other modules is essentially about
 * who needs to know about position-specific state in the IR.  If an
 * optimization is completely stateless (e.g. strength reduction, constant
 * folding, etc) it goes in here, otherwise it goes in IRBuilder or some other
 * pass.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Simplify performs a number of state-independent optimizations, primarily
 * forms of copy propagation, constant folding, or removing opcodes that have
 * no effects.
 *
 * The Simplifier recursively invokes itself, so that all instructions returned
 * from simplify() have been fully simplified themselves.
 *
 * In general, the simplifier transforms one instruction into zero or more
 * instructions.  The pair represents the zero or more instructions to replace
 * the input with, plus the SSATmp* to use instead of the input instruction's
 * dst (if any).
 */
struct SimplifyResult {
  jit::vector<IRInstruction*> instrs;
  SSATmp* dst;
};
SimplifyResult simplify(IRUnit&, const IRInstruction*, bool typesMightRelax);

//////////////////////////////////////////////////////////////////////

/*
 * Propagate very simple copies on the given instruction.
 * Specifically, Movs.
 *
 * More complicated copy-propagation is performed in the Simplifier.
 */
void copyProp(IRInstruction*);

/*
 * Checks whether a packed array bounds check is unnecessary.  We share this
 * logic with gen time so that cases that are visible immediately don't require
 * generating IR with control flow that we have to clean up later.
 */
bool packedArrayBoundsCheckUnnecessary(Type arrayType, int64_t key);

//////////////////////////////////////////////////////////////////////

}}

#endif
