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
SimplifyResult simplify(IRUnit&, const IRInstruction*);

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
void simplifyInPlace(IRUnit&, IRInstruction*);

/*
 * Perform a simplification pass in the entire unit.
 *
 * After running this pass, the caller must run a manditoryDCE to restore IR
 * invariants.
 */
void simplifyPass(IRUnit&);

//////////////////////////////////////////////////////////////////////

}}
