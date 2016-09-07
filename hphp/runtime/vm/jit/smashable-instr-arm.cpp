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

#include "hphp/runtime/vm/jit/smashable-instr-arm.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/alignment.h"
#include "hphp/runtime/vm/jit/align-arm.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

#include "hphp/vixl/a64/constants-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

/*
 * For smashable jmps and calls in ARM, we emit the target address straight
 * into the instruction stream, and then do a pc-relative load to read it.
 *
 * This neatly sidesteps the problem of concurrent modification and execution,
 * as well as the problem of 19- and 26-bit jump offsets (not big enough).  It
 * does, however, entail an indirect jump.
 */

TCA emitSmashableMovq(CodeBlock& cb, CGMeta& fixups, uint64_t imm,
                      PhysReg d) {
  align(cb, &fixups, Alignment::SmashMovq, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label imm_data;
  vixl::Label after_data;

  auto const start = cb.frontier();

  a.    Ldr  (x2a(d), &imm_data);
  a.    B    (&after_data);

  // Emit the immediate into the instruction stream.
  a.    bind (&imm_data);
  a.    dc64 (imm);
  a.    bind (&after_data);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

TCA emitSmashableCmpq(CodeBlock& cb, CGMeta& fixups, int32_t imm,
                      PhysReg r, int8_t disp) {
  // FIXME: This is used in func-guard*
  not_implemented();
}

TCA emitSmashableCall(CodeBlock& cb, CGMeta& fixups, TCA target) {
  align(cb, &fixups, Alignment::SmashCall, AlignContext::Live);

  vixl::MacroAssembler a { cb };
  vixl::Label target_data;
  vixl::Label after_data;

  auto const start = cb.frontier();

  // Jump over the data
  a.    B    (&after_data);

  // Emit the call target into the instruction stream.
  a.    bind (&target_data);
  a.    dc64 (target);
  a.    bind (&after_data);

  // Load the target address and call it
  a.    Ldr  (rAsm, &target_data);
  a.    Blr  (rAsm);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

TCA emitSmashableJmpImpl(CodeBlock& cb, TCA target) {
  vixl::MacroAssembler a { cb };
  vixl::Label target_data;

  auto const start = cb.frontier();

  a.    Ldr  (rAsm, &target_data);
  a.    Br   (rAsm);

  // Emit the jmp target into the instruction stream.
  a.    bind (&target_data);
  a.    dc64 (target);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

TCA emitSmashableJmp(CodeBlock& cb, CGMeta& fixups, TCA target) {
  align(cb, &fixups, Alignment::SmashJmp, AlignContext::Live);
  return emitSmashableJmpImpl(cb, target);
}

TCA emitSmashableJccImpl(CodeBlock& cb, TCA target, ConditionCode cc) {
  // During smashing, 'cc' is modified first followed by the target address.
  // This can cause 'new cc' jump to 'old target', but never the 'old cc'
  // jump to 'new target'. The former is safe because the 'old target'
  // is a stub.

  vixl::MacroAssembler a { cb };
  vixl::Label after_data;

  auto const start = cb.frontier();

  // Emit the conditional branch
  a.    B    (&after_data, InvertCondition(arm::convertCC(cc)));

  // Emit the smashable jump
  emitSmashableJmpImpl(cb, target);
  a.    bind (&after_data);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

TCA emitSmashableJcc(CodeBlock& cb, CGMeta& fixups, TCA target,
                     ConditionCode cc) {
  align(cb, &fixups, Alignment::SmashJcc, AlignContext::Live);
  return emitSmashableJccImpl(cb, target, cc);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static void smashInstr(TCA inst, T target, size_t sz) {
  *reinterpret_cast<T*>(inst + sz - 8) = target;
  auto const end = reinterpret_cast<char*>(inst + sz);
  auto const begin = end - 8;
  __builtin___clear_cache(begin, end);
}

void smashMovq(TCA inst, uint64_t target) {
  smashInstr(inst, target, smashableMovqLen());
}

void smashCmpq(TCA inst, uint32_t target) {
  not_implemented();
}

void smashCall(TCA inst, TCA target) {
  // Skip over the initial branch instruction
  *reinterpret_cast<TCA*>(inst + 4) = target;
  auto const begin = reinterpret_cast<char*>(inst + 4);
  auto const end = begin + 8;
  __builtin___clear_cache(begin, end);
}

void smashJmp(TCA inst, TCA target) {
  smashInstr(inst, target, smashableJmpLen());
}

void smashJcc(TCA inst, TCA target, ConditionCode cc) {
  if (cc != CC_None) {
    // Condition has changed, emit the 'jccs' sequence again
    CodeBlock cb;
    cb.init(inst, smashableJccLen(), "smashJcc");
    emitSmashableJccImpl(cb, target, cc);
    auto const begin = reinterpret_cast<char*>(cb.frontier());
    auto const end = begin + smashableJccLen();
    __builtin___clear_cache(begin, end);
  } else {
    // Update the target address
    smashInstr(inst, target, smashableJccLen());
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  return *reinterpret_cast<uint64_t*>(inst + smashableMovqLen() - 8);
}

uint32_t smashableCmpqImm(TCA inst) {
  not_implemented();
}

TCA smashableCallTarget(TCA call) {
  // Skip over the initial branch instruction
  return *reinterpret_cast<TCA*>(call + 4);
}

TCA smashableJmpTarget(TCA jmp) {
  return *reinterpret_cast<TCA*>(jmp + smashableJmpLen() - 8);
}

TCA smashableJccTarget(TCA jcc) {
  return *reinterpret_cast<TCA*>(jcc + smashableJccLen() - 8);
}

ConditionCode smashableJccCond(TCA inst) {
  using namespace vixl;
  struct JccDecoder : public Decoder {
    void VisitConditionalBranch(Instruction* inst) override {
      cc = (Condition)inst->ConditionBranch();
    }
    folly::Optional<Condition> cc;
  };
  JccDecoder decoder;
  decoder.Decode(Instruction::Cast(inst));
  always_assert(decoder.cc);
  return convertCC(InvertCondition(*decoder.cc));
}

///////////////////////////////////////////////////////////////////////////////

}}}
