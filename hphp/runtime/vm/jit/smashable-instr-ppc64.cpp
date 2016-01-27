/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015                                   |
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

#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"

#include "hphp/runtime/vm/jit/align-ppc64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////


#define EMIT_BODY(cb, inst, Inst, ...)  \
  ([&] {                                \
    align(cb, Alignment::Smash##Inst,   \
          AlignContext::Live);          \
    auto const start = cb.frontier();   \
    ppc64_asm::Assembler a { cb };      \
    a.inst(__VA_ARGS__);                \
    return start;                       \
  }())

TCA emitSmashableMovq(CodeBlock& cb, uint64_t imm, PhysReg d) {
  return EMIT_BODY(cb, li64, Movq, d, imm);
}

TCA emitSmashableCmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp) {
  align(cb, Alignment::SmashCmpq, AlignContext::Live);

  auto const start = cb.frontier();
  ppc64_asm::Assembler a { cb };

  // don't use cmpqim because of smashableCmpqImm implementation. A "load 32bits
  // immediate" is mandatory
  a.li32(rfuncln(), imm);
  a.lwz   (rAsm, r[disp]); // base + displacement
  a.extsw(rAsm, rAsm);
  a.cmpd(rfuncln(), rAsm);
  return start;
}

TCA emitSmashableCall(CodeBlock& cb, TCA target) {
  align(cb, Alignment::SmashCmpq, AlignContext::Live);

  return EMIT_BODY(cb, call, Call, rsp(), rfuncln(), rvmfp(), target);
}

TCA emitSmashableJmp(CodeBlock& cb, TCA target) {
  return EMIT_BODY(cb, branchAuto, Jmp, target);
}

TCA emitSmashableJcc(CodeBlock& cb, TCA target, ConditionCode cc) {
  assertx(cc != CC_None);
  return EMIT_BODY(cb, branchAuto, Jcc, target, cc);
}

std::pair<TCA,TCA>
emitSmashableJccAndJmp(CodeBlock& cb, TCA target, ConditionCode cc) {
  assertx(cc != CC_None);

  align(cb, Alignment::SmashJccAndJmp, AlignContext::Live);

  ppc64_asm::Assembler a { cb };
  auto const jcc = cb.frontier();
  a.branchAuto(target, cc);
  auto const jmp = cb.frontier();
  a.branchAuto(target);

  return std::make_pair(jcc, jmp);
}

#undef EMIT_BODY

///////////////////////////////////////////////////////////////////////////////

void smashMovq(TCA inst, uint64_t imm) {
  always_assert(is_aligned(inst, Alignment::SmashMovq));

  CodeBlock cb;
  // Initialize code block cb pointing to li64
  cb.init(inst, ppc64_asm::Assembler::kLi64InstrLen, "smashing Movq");
  CodeCursor cursor { cb, inst };
  ppc64_asm::Assembler a { cb };

  Reg64 reg = ppc64_asm::Assembler::getLi64Reg(inst);

  a.li64(reg, imm);
}

void smashCmpq(TCA inst, uint32_t imm) {
  always_assert(is_aligned(inst, Alignment::SmashCmpq));

  auto& cb = mcg->code.blockFor(inst);
  CodeCursor cursor { cb, inst };
  ppc64_asm::Assembler a { cb };

  // the first instruction is a vasm ldimml, which is a li32
  Reg64 reg = ppc64_asm::Assembler::getLi32Reg(inst);

  a.li32(reg, imm);
}

void smashCall(TCA inst, TCA target) {
  auto& cb = mcg->code.blockFor(inst);
  CodeCursor cursor { cb, inst };
  ppc64_asm::Assembler a { cb };

  if (!ppc64_asm::Assembler::isCall(inst)) {
    always_assert(false && "smashCall has unexpected block");
  }

  a.setFrontier(inst + smashableCallSkip());

  a.li64(ppc64_asm::reg::r12, reinterpret_cast<uint64_t>(target));
}

void smashJmp(TCA inst, TCA target) {
  always_assert(is_aligned(inst, Alignment::SmashJmp));

  auto& cb = mcg->code.blockFor(inst);
  CodeCursor cursor { cb, inst };
  ppc64_asm::Assembler a { cb };

  if (target > inst && target - inst <= smashableJmpLen()) {
    a.emitNop(target - inst);
  } else {
    a.branchAuto(target);
  }
}

void smashJcc(TCA inst, TCA target, ConditionCode cc) {
  always_assert(is_aligned(inst, Alignment::SmashJcc));

  if (cc == CC_None) {
    ppc64_asm::Assembler::patchBctr(inst, target);
  } else {
    auto& cb = mcg->code.blockFor(inst);
    CodeCursor cursor { cb, inst };
    ppc64_asm::Assembler a { cb };
    a.branchAuto(target, cc);
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  return static_cast<uint64_t>(ppc64_asm::Assembler::getLi64(inst));
}

uint32_t smashableCmpqImm(TCA inst) {
  return static_cast<uint32_t>(ppc64_asm::Assembler::getLi32(inst));
}

TCA smashableCallTarget(TCA inst) {
  if (!ppc64_asm::Assembler::isCall(inst)) return nullptr;

  return reinterpret_cast<TCA>(
      ppc64_asm::Assembler::getLi64(inst + smashableCallSkip()));
}

TCA smashableJmpTarget(TCA inst) {
  return smashableJccTarget(inst); // for now, it's the same as Jcc
}

TCA smashableJccTarget(TCA inst) {
  // from patchBctr:
  // It has to skip 6 instructions: li64 (5 instructions) and mtctr
  CodeAddress bctr_addr = inst + kStdIns * 6;
  // Opcode located at the 6 most significant bits
  if (((bctr_addr[3] >> 2) & 0x3F) != 19) return nullptr; // from bctr

  return reinterpret_cast<TCA>(ppc64_asm::Assembler::getLi64(inst));
}

ConditionCode smashableJccCond(TCA inst) {
  // skip to the branch instruction in order to get its condition
  ppc64_asm::BranchParams bp(
      *reinterpret_cast<ppc64_asm::PPC64Instr*>(inst + smashableJccSkip()));
  return bp;
}

///////////////////////////////////////////////////////////////////////////////

}}}
