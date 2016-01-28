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

#ifndef incl_HPHP_JIT_SMASHABLE_INSTR_ARM_H_
#define incl_HPHP_JIT_SMASHABLE_INSTR_ARM_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of smashable-instr.h.
 */

/*
 * Number of instructions (each of which is four bytes) in the sequence, plus
 * the size of the smashable immediate.
 */
constexpr size_t smashableMovqLen() { return 2 * 4 + 8; }
constexpr size_t smashableCmpqLen() { return 0; }
constexpr size_t smashableCallLen() { return 3 * 4 + 8; }
constexpr size_t smashableJmpLen()  { return 2 * 4 + 8; }
constexpr size_t smashableJccLen()  { return 3 * 4 + 8; }

TCA emitSmashableMovq(CodeBlock& cb, uint64_t imm, PhysReg d);
TCA emitSmashableCmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp);
TCA emitSmashableCall(CodeBlock& cb, TCA target);
TCA emitSmashableJmp(CodeBlock& cb, TCA target);
TCA emitSmashableJcc(CodeBlock& cb, TCA target, ConditionCode cc);
std::pair<TCA,TCA>
emitSmashableJccAndJmp(CodeBlock& cb, TCA target, ConditionCode cc);

void smashMovq(TCA inst, uint64_t imm);
void smashCmpq(TCA inst, uint32_t imm);
void smashCall(TCA inst, TCA target);
void smashJmp(TCA inst, TCA target);
void smashJcc(TCA inst, TCA target, ConditionCode cc = CC_None);

uint64_t smashableMovqImm(TCA inst);
uint32_t smashableCmpqImm(TCA inst);
TCA smashableCallTarget(TCA inst);
TCA smashableJmpTarget(TCA inst);
TCA smashableJccTarget(TCA inst);
ConditionCode smashableJccCond(TCA inst);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
