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
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace ppc64 {

using ppc64_asm::PPC64Instr;
using ppc64_asm::Assembler;
using ppc64_asm::ImmType;
using ppc64_asm::DecodedInstruction;

///////////////////////////////////////////////////////////////////////////////


#define EMIT_BODY(cb, meta, inst, ...)      \
  ([&] {                                    \
    auto const start = cb.frontier();       \
    meta.smashableLocations.insert(start);  \
    Assembler a { cb };                     \
    a.inst(__VA_ARGS__);                    \
    return start;                           \
  }())

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& fixups, uint64_t imm,
                      PhysReg d) {
  return EMIT_BODY(cb, fixups, limmediate, d, imm, ImmType::TocOnly,
                   true);
}

TCA emitSmashableCall(CodeBlock& cb, CGMeta& fixups, TCA target,
    Assembler::CallArg ca) {
  return EMIT_BODY(cb, fixups, call, target, ca);
}

TCA emitSmashableJmp(CodeBlock& cb, CGMeta& fixups, TCA target) {
  return EMIT_BODY(cb, fixups, branchFar, target,
                   ppc64_asm::BranchConditions::Always,
                   ppc64_asm::LinkReg::DoNotTouch,
                   ppc64_asm::ImmType::TocOnly, true);
}

TCA emitSmashableJcc(CodeBlock& cb, CGMeta& fixups, TCA target,
                     ConditionCode cc) {
  assertx(cc != CC_None);
  return EMIT_BODY(cb, fixups, branchFar, target, cc,
                   ppc64_asm::LinkReg::DoNotTouch,
                   ppc64_asm::ImmType::TocOnly, true);
}

#undef EMIT_BODY

///////////////////////////////////////////////////////////////////////////////

void smashMovq(TCA inst, uint64_t imm) {
  // Smash TOC value
  const DecodedInstruction di(inst);
  assertx(di.isLoadingTOC() && di.isSmashable(imm));
  uint64_t* imm_address = di.decodeTOCAddress();
  *imm_address = imm;
}

void smashCmpq(TCA inst, uint32_t imm) {
  // Smash TOC value
  const DecodedInstruction di(inst);
  assertx(di.isLoadingTOC() && di.isSmashable(imm));
  uint64_t* imm_address = di.decodeTOCAddress();
  *imm_address = imm;
}

void smashCall(TCA inst, TCA target) {
  // Smash TOC value
  const DecodedInstruction di(inst);
  assertx(di.isLoadingTOC() &&
          di.isSmashable(reinterpret_cast<uint64_t>(target)));
  uint64_t* imm_address = di.decodeTOCAddress();
  *imm_address = reinterpret_cast<uint64_t>(target);
}

void smashJmp(TCA inst, TCA target) {
  // Smash TOC value
  const DecodedInstruction di(inst);
  assertx(di.isLoadingTOC() &&
          di.isSmashable(reinterpret_cast<uint64_t>(target)));
  uint64_t* imm_address = di.decodeTOCAddress();
  if (imm_address) *imm_address = reinterpret_cast<uint64_t>(target);
}

void smashJcc(TCA inst, TCA target) {
  // Smash TOC value
  const DecodedInstruction di(inst);
  assertx(di.isLoadingTOC() &&
          di.isSmashable(reinterpret_cast<uint64_t>(target)));
  uint64_t* imm_address = di.decodeTOCAddress();
  *imm_address = reinterpret_cast<uint64_t>(target);
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  const DecodedInstruction di(inst);
  return di.immediate();
}

uint32_t smashableCmpqImm(TCA inst) {
  const DecodedInstruction di(inst);
  return static_cast<uint32_t>(di.immediate());
}

TCA smashableCallTarget(TCA inst) {
  const DecodedInstruction di(inst);
  if (!di.isCall()) return nullptr;

  return di.farBranchTarget();
}

static TCA smashableBranchTarget(TCA inst, bool allowCond) {
  const DecodedInstruction di(inst);
  auto ac = allowCond
    ? ppc64_asm::AllowCond::Any
    : ppc64_asm::AllowCond::OnlyUncond;
  if (!di.isBranch(ac)) return nullptr;

  return di.farBranchTarget();
}

TCA smashableJmpTarget(TCA inst) {
  return smashableBranchTarget(inst, false);
}

TCA smashableJccTarget(TCA inst) {
  return smashableBranchTarget(inst, true);
}

ConditionCode smashableJccCond(TCA inst) {
  // To analyse the cc, it has to be on the bcctr so we need go backward one
  // instruction.
  uint8_t jccLen = smashableJccLen() - kStdIns;
  // skip to the branch instruction in order to get its condition
  ppc64_asm::BranchParams bp(reinterpret_cast<PPC64Instr*>(inst + jccLen));
  return bp;
}

///////////////////////////////////////////////////////////////////////////////

bool optimizeSmashedCall(TCA inst) {
  return false;
}

bool optimizeSmashedJmp(TCA inst) {
  return false;
}

bool optimizeSmashedJcc(TCA inst) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////

}}}
