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

#include "hphp/runtime/vm/jit/align-arm.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP::jit {

struct CGMeta;

namespace arm {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of smashable-instr.h.
 */

/*
 * Number of bytes in the instruction sequence.
 */
constexpr size_t smashableMovqLen() { return 4; }
constexpr size_t smashableCmpqLen() { return 0; }
constexpr size_t smashableCallLen() { return 4; } // not including veneer
constexpr size_t smashableJmpLen()  { return 4; } // not including veneer
constexpr size_t smashableJccLen()  { return 4; } // not including veneer
constexpr size_t smashableInterceptLen() { return 4; } // not including veneer

/*
 * Don't align the smashables on arm.  The sensitive part of the instruction is
 * the literal which is stored out of line.
 */
constexpr size_t smashableAlignTo() { return 0; }

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& meta, uint64_t imm,
                      PhysReg d);
TCA emitSmashableCall(CodeBlock& cb, CGMeta& meta, TCA target);
TCA emitSmashableJmp(CodeBlock& cb, CGMeta& meta, TCA target);
TCA emitSmashableJcc(CodeBlock& cb, CGMeta& meta, TCA target,
                     ConditionCode cc);
void smashMovq(TCA inst, uint64_t imm);
void smashCmpq(TCA inst, uint32_t imm);
void smashCall(TCA inst, TCA target);
void smashJmp(TCA inst, TCA target);
void smashJcc(TCA inst, TCA target);
void smashInterceptJcc(TCA inst);
void smashInterceptJmp(TCA inst);

bool possiblySmashableMovq(TCA inst);
bool possiblySmashableJmp(TCA inst);
bool possiblySmashableJcc(TCA inst);

uint64_t smashableMovqImm(TCA inst);
uint32_t smashableCmpqImm(TCA inst);
TCA smashableCallTarget(TCA inst);
TCA smashableJmpTarget(TCA inst);
TCA smashableJccTarget(TCA inst);
ConditionCode smashableJccCond(TCA inst);

bool optimizeSmashedCall(TCA inst);
bool optimizeSmashedJmp(TCA inst);
bool optimizeSmashedJcc(TCA inst);

constexpr size_t kSmashMovqImmOff = 0;
constexpr size_t kSmashCmpqImmOff = 0;
constexpr size_t kSmashCallTargetOff = 0;
constexpr size_t kSmashJmpTargetOff = 0;
constexpr size_t kSmashJccTargetOff = 0;

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
  assertx(is_aligned(inst - kSmashMovqImmOff, Alignment::SmashMovq));
  *reinterpret_cast<uint64_t*>(inst) = reinterpret_cast<uint64_t>(target);
  auto const begin = inst;
  auto const end = begin + 8;
  DataBlock::syncDirect(begin, end);
}

inline void smashInst(TCA addr, uint32_t newInst) {
  assertx(((uint64_t)addr & 3) == 0);
  *reinterpret_cast<uint32_t*>(addr) = newInst;
  auto const begin = addr;
  auto const end = begin + 4;
  DataBlock::syncDirect(begin, end);
}

}}

