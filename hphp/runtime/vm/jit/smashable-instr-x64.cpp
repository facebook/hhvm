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

#include "hphp/runtime/vm/jit/smashable-instr-x64.h"

#include "hphp/runtime/vm/jit/align-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * On x64, concurrent modification and execution of instructions is safe if all
 * of the following hold:
 *
 *  1/  The modification is done with a single processor store.
 *  2/  Only one instruction in the original stream is modified.
 *  3/  The modified instruction does not cross a cacheline boundary.
 *
 * Cache alignment is required for mutable instructions to make sure mutations
 * don't "tear" on remote CPUs.
 */

#define EMIT_BODY(cb, inst, Inst, ...)  \
  ([&] {                                \
    align(cb, Alignment::Smash##Inst,   \
          AlignContext::Live);          \
    auto const start = cb.frontier();   \
    X64Assembler a { cb };              \
    a.inst(__VA_ARGS__);                \
    return start;                       \
  }())

TCA emitSmashableMovq(CodeBlock& cb, uint64_t imm, PhysReg d) {
  auto const start = EMIT_BODY(cb, movq, Movq, 0xdeadbeeffeedface, d);

  auto immp = reinterpret_cast<uint64_t*>(
    cb.frontier() - smashableMovqLen() + kSmashMovqImmOff
  );
  *immp = imm;

  return start;
}

TCA emitSmashableCmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp) {
  return EMIT_BODY(cb, cmpq, Cmpq, imm, r[disp]);
}

TCA emitSmashableCall(CodeBlock& cb, TCA target) {
  return EMIT_BODY(cb, call, Call, target);
}

TCA emitSmashableJmp(CodeBlock& cb, TCA target) {
  return EMIT_BODY(cb, jmp, Jmp, target);
}

TCA emitSmashableJcc(CodeBlock& cb, TCA target, ConditionCode cc) {
  assertx(cc != CC_None);
  return EMIT_BODY(cb, jcc, Jcc, cc, target);
}

std::pair<TCA,TCA>
emitSmashableJccAndJmp(CodeBlock& cb, TCA target, ConditionCode cc) {
  assertx(cc != CC_None);

  align(cb, Alignment::SmashJccAndJmp, AlignContext::Live);

  X64Assembler a { cb };
  auto const jcc = cb.frontier();
  a.jcc(cc, target);
  auto const jmp = cb.frontier();
  a.jmp(target);

  return std::make_pair(jcc, jmp);
}

#undef EMIT_BODY

///////////////////////////////////////////////////////////////////////////////

void smashMovq(TCA inst, uint64_t imm) {
  always_assert(is_aligned(inst, Alignment::SmashMovq));
  *reinterpret_cast<uint64_t*>(inst + kSmashMovqImmOff) = imm;
}

void smashCmpq(TCA inst, uint32_t imm) {
  always_assert(is_aligned(inst, Alignment::SmashCmpq));
  *reinterpret_cast<uint32_t*>(inst + kSmashCmpqImmOff) = imm;
}

void smashCall(TCA inst, TCA target) {
  always_assert(is_aligned(inst, Alignment::SmashCall));
  /*
   * TODO(#7889486): We'd like this just to be:
   *
   *    X64Assembler::patchCall(inst, target);
   *
   * but presently this causes asserts to fire in MCGenerator because of a bug
   * with PGO and relocation.
   */
  auto& cb = mcg->code.blockFor(inst);
  CodeCursor cursor { cb, inst };
  X64Assembler a { cb };
  a.call(target);
}

void smashJmp(TCA inst, TCA target) {
  always_assert(is_aligned(inst, Alignment::SmashJmp));

  auto& cb = mcg->code.blockFor(inst);
  CodeCursor cursor { cb, inst };
  X64Assembler a { cb };

  if (target > inst && target - inst <= smashableJmpLen()) {
    a.emitNop(target - inst);
  } else {
    a.jmp(target);
  }
}

void smashJcc(TCA inst, TCA target, ConditionCode cc) {
  always_assert(is_aligned(inst, Alignment::SmashJcc));

  if (cc == CC_None) {
    X64Assembler::patchJcc(inst, target);
  } else {
    auto& cb = mcg->code.blockFor(inst);
    CodeCursor cursor { cb, inst };
    X64Assembler a { cb };
    a.jcc(cc, target);
  }
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashableMovqImm(TCA inst) {
  return *reinterpret_cast<uint64_t*>(inst + kSmashMovqImmOff);
}

uint32_t smashableCmpqImm(TCA inst) {
  return *reinterpret_cast<uint32_t*>(inst + kSmashCmpqImmOff);
}

TCA smashableCallTarget(TCA inst) {
  if (inst[0] != 0xE8) return nullptr;
  return inst + 5 + ((int32_t*)(inst + 5))[-1];
}

TCA smashableJmpTarget(TCA inst) {
  if (inst[0] != 0xe9) {
    if (inst[0] == 0x0f &&
        inst[1] == 0x1f &&
        inst[2] == 0x44) {
      // 5 byte nop
      return inst + 5;
    }
    return nullptr;
  }
  return inst + 5 + ((int32_t*)(inst + 5))[-1];
}

TCA smashableJccTarget(TCA inst) {
  if (inst[0] != 0x0F || (inst[1] & 0xF0) != 0x80) return nullptr;
  return inst + 6 + ((int32_t*)(inst + 6))[-1];
}

ConditionCode smashableJccCond(TCA inst) {
  return DecodedInstruction(inst).jccCondCode();
}

///////////////////////////////////////////////////////////////////////////////

}}}
