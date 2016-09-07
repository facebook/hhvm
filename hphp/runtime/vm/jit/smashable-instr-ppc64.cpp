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

#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"

#include "hphp/runtime/vm/jit/abi-ppc64.h"
#include "hphp/runtime/vm/jit/align-ppc64.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/decoder-ppc64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace ppc64 {

using ppc64_asm::PPC64Instr;
using ppc64_asm::Assembler;

///////////////////////////////////////////////////////////////////////////////


#define EMIT_BODY(cb, inst, Inst, ...)  \
  ([&] {                                \
    align(cb, &fixups,                  \
          Alignment::Smash##Inst,       \
          AlignContext::Live);          \
    auto const start = cb.frontier();   \
    Assembler a { cb };                 \
    a.inst(__VA_ARGS__);                \
    return start;                       \
  }())

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& fixups, uint64_t imm,
                      PhysReg d) {
  return EMIT_BODY(cb, li64, Movq, d, imm);
}

TCA emitSmashableCmpq(CodeBlock& cb, CGMeta& fixups, int32_t imm,
                      PhysReg r, int8_t disp) {
  align(cb, &fixups, Alignment::SmashCmpq, AlignContext::Live);

  auto const start = cb.frontier();
  Assembler a { cb };

  // don't use cmpqim because of smashableCmpqImm implementation. A "load 32bits
  // immediate" is mandatory
  a.li32 (rfuncln(), imm);
  a.lwz  (rAsm, r[disp]); // base + displacement
  a.extsw(rAsm, rAsm);
  a.cmpd (rfuncln(), rAsm);
  return start;
}

TCA emitSmashableCall(CodeBlock& cb, CGMeta& fixups, TCA target) {
  align(cb, &fixups, Alignment::SmashCmpq, AlignContext::Live);
  return EMIT_BODY(cb, call, Call, target, Assembler::CallArg::Smashable);
}

TCA emitSmashableJmp(CodeBlock& cb, CGMeta& fixups, TCA target) {
  return EMIT_BODY(cb, branchFar, Jmp, target);
}

TCA emitSmashableJcc(CodeBlock& cb, CGMeta& fixups, TCA target,
                     ConditionCode cc) {
  assertx(cc != CC_None);
  return EMIT_BODY(cb, branchFar, Jcc, target, cc);
}

#undef EMIT_BODY

///////////////////////////////////////////////////////////////////////////////

void smashMovq(TCA inst, uint64_t imm) {
  always_assert(is_aligned(inst, Alignment::SmashMovq));

  CodeBlock cb;
  // Initialize code block cb pointing to li64
  cb.init(inst, Assembler::kLi64InstrLen, "smashing Movq");
  CodeCursor cursor { cb, inst };
  Assembler a { cb };

  Reg64 reg = Assembler::getLi64Reg(inst);

  a.li64(reg, imm);
}

void smashCmpq(TCA inst, uint32_t imm) {
  always_assert(is_aligned(inst, Alignment::SmashCmpq));

  auto& cb = tc::code().blockFor(inst);
  CodeCursor cursor { cb, inst };
  Assembler a { cb };

  // the first instruction is a vasm ldimml, which is a li32
  Reg64 reg = Assembler::getLi32Reg(inst);

  a.li32(reg, imm);
}

void smashCall(TCA inst, TCA target) {
  auto& cb = tc::code().blockFor(inst);
  CodeCursor cursor { cb, inst };
  Assembler a { cb };

  if (!Assembler::isCall(inst)) {
    always_assert(false && "smashCall has unexpected block");
  }

  a.setFrontier(inst);

  a.li64(rfuncentry(), reinterpret_cast<uint64_t>(target));
}

void smashJmp(TCA inst, TCA target) {
  always_assert(is_aligned(inst, Alignment::SmashJmp));

  auto& cb = tc::code().blockFor(inst);
  CodeCursor cursor { cb, inst };
  Assembler a { cb };

  if (target > inst && target - inst <= smashableJmpLen()) {
    a.emitNop(target - inst);
  } else {
    a.branchAuto(target);
  }
}

void smashJcc(TCA inst, TCA target, ConditionCode cc) {
  always_assert(is_aligned(inst, Alignment::SmashJcc));

  if (cc == CC_None) {
    Assembler::patchBranch(inst, target);
  } else {
    auto& cb = tc::code().blockFor(inst);
    CodeCursor cursor { cb, inst };
    Assembler a { cb };
    a.branchAuto(target, cc);
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  return static_cast<uint64_t>(Assembler::getLi64(inst));
}

uint32_t smashableCmpqImm(TCA inst) {
  return static_cast<uint32_t>(Assembler::getLi32(inst));
}

TCA smashableCallTarget(TCA inst) {
  if (!Assembler::isCall(inst)) return nullptr;

  return reinterpret_cast<TCA>(
      Assembler::getLi64(inst));
}

TCA smashableJmpTarget(TCA inst) {
  return smashableJccTarget(inst); // for now, it's the same as Jcc
}

TCA smashableJccTarget(TCA inst) {
  // To analyse the cc, it has to be on the bcctr so we need go backward one
  // instruction.
  uint8_t jccLen = smashableJccLen() - kStdIns;
  auto branch_instr = *reinterpret_cast<PPC64Instr*>(inst + jccLen);
  if (!ppc64_asm::Decoder::GetDecoder().decode(branch_instr)->isBranch(true)) {
    return nullptr;
  }

  return reinterpret_cast<TCA>(Assembler::getLi64(inst));
}

ConditionCode smashableJccCond(TCA inst) {
  // To analyse the cc, it has to be on the bcctr so we need go backward one
  // instruction.
  uint8_t jccLen = smashableJccLen() - kStdIns;
  // skip to the branch instruction in order to get its condition
  ppc64_asm::BranchParams bp(*reinterpret_cast<PPC64Instr*>(inst + jccLen));
  return bp;
}

///////////////////////////////////////////////////////////////////////////////

}}}
