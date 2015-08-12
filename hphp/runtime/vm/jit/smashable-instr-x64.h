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

#ifndef incl_HPHP_JIT_SMASHABLE_INSTR_X64_H_
#define incl_HPHP_JIT_SMASHABLE_INSTR_X64_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of smashable-instr.h.
 */

constexpr size_t sizeof_smashable_movq() { return 10; }
constexpr size_t sizeof_smashable_cmpq() { return 8; }
constexpr size_t sizeof_smashable_call() { return 5; }
constexpr size_t sizeof_smashable_jmp()  { return 5; }
constexpr size_t sizeof_smashable_jcc()  { return 6; }

TCA emit_smashable_movq(CodeBlock& cb, uint64_t imm, PhysReg d);
TCA emit_smashable_cmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp);
TCA emit_smashable_call(CodeBlock& cb, TCA target);
TCA emit_smashable_jmp(CodeBlock& cb, TCA target);
TCA emit_smashable_jcc(CodeBlock& cb, TCA target, ConditionCode cc);
std::pair<TCA,TCA>
emit_smashable_jcc_and_jmp(CodeBlock& cb, TCA target, ConditionCode cc);

void smash_movq(TCA inst, uint64_t imm);
void smash_call(TCA inst, TCA target);
void smash_jmp(TCA inst, TCA target);
void smash_jcc(TCA inst, TCA target, ConditionCode cc = CC_None);

uint64_t smashable_movq_imm(TCA inst);
TCA smashable_call_target(TCA inst);
TCA smashable_jmp_target(TCA inst);
TCA smashable_jcc_target(TCA inst);
ConditionCode smashable_jcc_cond(TCA inst);

/*
 * Smashable immediate and target offsets.
 *
 * These are also used by align-x64.
 */
constexpr size_t kMovqImmOfff = 2;

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
