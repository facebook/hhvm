/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015-2016                              |
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

#ifndef incl_HPHP_JIT_SMASHABLE_INSTR_PPC64_H_
#define incl_HPHP_JIT_SMASHABLE_INSTR_PPC64_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

struct CGMeta;

namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of smashable-instr.h.
 */

constexpr uint8_t kStdIns = ppc64_asm::instr_size_in_bytes;

// li64
constexpr size_t smashableMovqLen() { return kStdIns * 5; }

// li64 + cmpd
constexpr size_t smashableCmpqLen() { return kStdIns * 6; }

// The following instruction size is from the beginning of the smashableCall
// to the address the LR saves upon branching with bctrl (so the following)
// Currently this calculation considers:
// prologue, li64 (5 instr), mtctr, nop, nop, bctrl
constexpr size_t smashableCallLen() {
  return ppc64_asm::Assembler::kCallLen;
}

// li64 + mtctr + nop + nop + bcctr
constexpr size_t smashableJccLen()  { return ppc64_asm::Assembler::kJccLen; }

// Same length as Jcc.
constexpr size_t smashableJmpLen()  { return smashableJccLen(); }

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& fixups, uint64_t imm,
                      PhysReg d);
TCA emitSmashableCmpq(CodeBlock& cb, CGMeta& fixups, int32_t imm,
                      PhysReg r, int8_t disp);
TCA emitSmashableCall(CodeBlock& cb, CGMeta& fixups, TCA target);
TCA emitSmashableJmp(CodeBlock& cb, CGMeta& fixups, TCA target);
TCA emitSmashableJcc(CodeBlock& cb, CGMeta& fixups, TCA target,
                     ConditionCode cc);
std::pair<TCA,TCA>
emitSmashableJccAndJmp(CodeBlock& cb, CGMeta& fixups, TCA target,
                       ConditionCode cc);

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

constexpr size_t kSmashMovqImmOff = 0;
constexpr size_t kSmashCmpqImmOff = 0;

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
