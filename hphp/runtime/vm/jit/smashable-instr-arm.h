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

#ifndef incl_HPHP_JIT_SMASHABLE_INSTR_ARM_H_
#define incl_HPHP_JIT_SMASHABLE_INSTR_ARM_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

struct CGMeta;

namespace arm {

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
constexpr size_t smashableCallLen() { return 4 + 8 + 2 * 4; }
constexpr size_t smashableJmpLen()  { return 2 * 4 + 4; }
constexpr size_t smashableJccLen()  { return 4 + smashableJmpLen(); }

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& meta, uint64_t imm,
                      PhysReg d);
TCA emitSmashableCmpq(CodeBlock& cb, CGMeta& meta, int32_t imm,
                      PhysReg r, int8_t disp);
TCA emitSmashableCall(CodeBlock& cb, CGMeta& meta, TCA target);
TCA emitSmashableJmp(CodeBlock& cb, CGMeta& meta, TCA target);
TCA emitSmashableJcc(CodeBlock& cb, CGMeta& meta, TCA target,
                     ConditionCode cc);
void smashMovq(TCA inst, uint64_t imm);
void smashCmpq(TCA inst, uint32_t imm);
void smashCall(TCA inst, TCA target);
void smashJmp(TCA inst, TCA target);
void smashJcc(TCA inst, TCA target);

bool isSmashableMovq(TCA inst);
uint64_t smashableMovqImm(TCA inst);
uint32_t smashableCmpqImm(TCA inst);
TCA smashableCallTarget(TCA inst);
TCA smashableJmpTarget(TCA inst);
TCA smashableJccTarget(TCA inst);
ConditionCode smashableJccCond(TCA inst);

TCA getSmashableFromTargetAddr(TCA addr);

constexpr size_t kSmashMovqImmOff = 8;
constexpr size_t kSmashCmpqImmOff = 0;
constexpr size_t kSmashCallTargetOff = 4;
constexpr size_t kSmashJmpTargetOff = 8;
constexpr size_t kSmashJccTargetOff = 12;

///////////////////////////////////////////////////////////////////////////////

template <class T>
inline uint32_t makeTarget32(T target) {
  assertx(!(reinterpret_cast<intptr_t>(target) >> 32));
  return static_cast<uint32_t>(reinterpret_cast<intptr_t>(target));
}

inline void patchTarget32(TCA inst, TCA target) {
  *reinterpret_cast<uint32_t*>(inst) = makeTarget32(target);
  auto const begin = inst;
  auto const end = begin + 4;
  DataBlock::syncDirect(begin, end);
}

inline void patchTarget64(TCA inst, TCA target) {
  *reinterpret_cast<uint64_t*>(inst) = reinterpret_cast<uint64_t>(target);
  auto const begin = inst;
  auto const end = begin + 8;
  DataBlock::syncDirect(begin, end);
}

}}}

#endif
