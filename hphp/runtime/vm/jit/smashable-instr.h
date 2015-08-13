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

#ifndef incl_HPHP_JIT_SMASHABLE_INSTR_H_
#define incl_HPHP_JIT_SMASHABLE_INSTR_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * This header provides the API for interacting with smashable instructions.
 *
 * Smashable instructions are logical instructions that can be safely mutated
 * even in the presence of concurrent execution.  These may map to more than
 * one machine instruction, depending on the target architecture.
 *
 * Supported instructions include:
 *
 *    [insts w/ smashable immediates]
 *      - movq
 *      - cmpq:   A cmpq of a 32-bit immediate and a memory operand with 8-bit
 *                displacement (and no index or scale).  Used for func guards.
 *                TODO(#7831969): Add smash_cmpq and smashable_cmpq_imm().
 *
 *    [insts w/ smashable targets]
 *      - call
 *      - jmp
 *      - jcc
 *      - jcc_jmp:  A jcc followed contiguously by a jmp, both of which have
 *                  independently smashable targets.  Used for bindjcc1st.
 *
 * Smashable instructions must have a statically known length (though they may
 * require nop-gap realignment when they are emitted).
 */

/*
 * Size of the smashable machine code sequence.
 */
size_t sizeof_smashable_movq();
size_t sizeof_smashable_cmpq();
size_t sizeof_smashable_call();
size_t sizeof_smashable_jmp();
size_t sizeof_smashable_jcc();

/*
 * Emit a smashable instruction and return the instruction's address.
 *
 * For jcc_and_jmp, return a pair of (jcc_addr, jmp_addr).
 */
TCA emit_smashable_movq(CodeBlock& cb, uint64_t imm, PhysReg d);
TCA emit_smashable_cmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp);
TCA emit_smashable_call(CodeBlock& cb, TCA target);
TCA emit_smashable_jmp(CodeBlock& cb, TCA target);
TCA emit_smashable_jcc(CodeBlock& cb, TCA target, ConditionCode cc);

std::pair<TCA,TCA>
emit_smashable_jcc_and_jmp(CodeBlock& cb, TCA target, ConditionCode cc);

/*
 * Logically smash the smashable operand of an instruction.
 *
 * These operations are allowed to change the instructions themselves, provided
 * they still behave appropriately (e.g., a fallthrough jmp can be smashed to a
 * nop sequence).
 *
 * The `smash_jcc' routine leaves the condition unchanged if `cc' is CC_None.
 */
void smash_movq(TCA inst, uint64_t imm);
void smash_call(TCA inst, TCA target);
void smash_jmp(TCA inst, TCA target);
void smash_jcc(TCA inst, TCA target, ConditionCode cc = CC_None);

/*
 * Extract instruction operands from assembly.
 *
 * If `inst' does not represent the correct smashable instruction, the
 * _target() routines will return nullptr, and the remaining routines will
 * produce unspecified behavior.
 */
uint64_t smashable_movq_imm(TCA inst);
TCA smashable_call_target(TCA inst);
TCA smashable_jmp_target(TCA inst);
TCA smashable_jcc_target(TCA inst);
ConditionCode smashable_jcc_cond(TCA inst);

/*
 * Obtain the address of a smashable call from its return IP.
 */
TCA smashable_call_from_ret(TCA ret);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
