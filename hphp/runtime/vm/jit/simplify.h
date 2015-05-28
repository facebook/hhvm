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

#include <folly/Optional.h>

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
 * state-independent properties.  The optimizations in this module should be
 * those that we can do based only on chasing the use-def chain.
 *
 * The line of separation between this and other modules is essentially about
 * who needs to know about position-specific state in the IR.  If an
 * optimization is completely stateless (e.g. strength reduction, constant
 * folding, etc) it goes in here; otherwise it goes in IRBuilder or some other
 * pass.
 */

//////////////////////////////////////////////////////////////////////

/*
 * simplify() performs a number of state-independent optimizations, primarily
 * forms of copy propagation, constant folding, or removing opcodes that have
 * no effects.
 *
 * simplify() recursively invokes itself, so that all instructions returned
 * from simplify() have been fully simplified themselves.
 *
 * In general, simplify() transforms one instruction into zero or more
 * instructions.  The pair represents the zero or more instructions to replace
 * the input with, plus the SSATmp* to use instead of the input instruction's
 * dst (if any).
 */
struct SimplifyResult {
  jit::vector<IRInstruction*> instrs;
  SSATmp* dst;
};
SimplifyResult simplify(IRUnit&, const IRInstruction*, bool typesMightRelax);

/*
 * Instruction stream modifying simplification routine.
 *
 * This version of simplify() expects an instruction in the IRUnit's CFG as
 * input, and replaces it with the simplified instruction stream, preserving
 * the integrity of the containing Block's forward iterators.
 *
 * This may cause other blocks in the unit to become unreachable, and also note
 * that it is not legal to run simplify() on instructions in blocks that were
 * made unreachable by other calls to simplify().  This generally means you
 * need to track which blocks are still reachable if you are making simplify()
 * calls.
 */
void simplify(IRUnit&, IRInstruction*);

/*
 * Perform a simplification pass in the entire unit.
 *
 * After running this pass, the caller must run a manditoryDCE to restore IR
 * invariants.
 */
void simplifyPass(IRUnit&);

//////////////////////////////////////////////////////////////////////

/*
 * Return true if the given AssertType-like instruction can be nop'd.
 *
 * This is exposed so that the preOptimizeAssertX() methods can share this
 * logic.
 *
 * WARNING: Under certain (very uncommon) conditions, we may find that external
 * information (e.g., from static analysis) conflicts with the instruction
 * stream we have built.  This function will detect this scenario and will punt
 * the entire trace in this case.
 */
bool canSimplifyAssertType(const IRInstruction* inst,
                           Type srcType,
                           bool srcMightRelax);

/*
 * Propagate very simple copies through Mov instructions.
 *
 * More complicated copy-propagation is performed in the Simplifier.
 */
void copyProp(IRInstruction*);

//////////////////////////////////////////////////////////////////////

/*
 * Statically check whether a packed array access is within bounds, based on
 * the type of the array.
 */
enum class PackedBounds { In, Out, Unknown };
PackedBounds packedArrayBoundsStaticCheck(Type, int64_t key);

/*
 * Get the type of `arr[idx]` for a packed array, considering constness,
 * staticness, and RAT types.
 *
 * Note that this function does not require the existence of `arr[idx]`.  If we
 * can statically determine that the access is out of bounds, InitNull is
 * returned.  Otherwise we return a type `t`, such that when the access is
 * within bounds, `arr[idx].isA(t)` holds.  (Thus, if this function is used in
 * contexts where the bounds are not statically known, TInitNull must be
 * unioned in for correctness.)
 */
Type packedArrayElemType(SSATmp* arr, SSATmp* idx);

//////////////////////////////////////////////////////////////////////

}}

#endif
