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

#include "hphp/runtime/vm/jit/smashable-instr-x64.h"

#include "hphp/runtime/vm/jit/back-end-x64.h"
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
 */

bool is_smashable(TCA frontier, int nbytes, int offset = 0) {
  assertx(nbytes <= kCacheLineSize);
  uintptr_t ifrontier = uintptr_t(frontier) + offset;
  uintptr_t last_byte = uintptr_t(frontier) + nbytes - 1;
  return (ifrontier & ~kCacheLineMask) == (last_byte & ~kCacheLineMask);
}

void make_smashable(CodeBlock& cb, int nbytes, int offset) {
  if (is_smashable(cb.frontier(), nbytes, offset)) return;

  auto const gap_sz =
    (~(uintptr_t(cb.frontier()) + offset) & kCacheLineMask) + 1;

  X64Assembler a { cb };
  a.emitNop(gap_sz);

  assertx(is_smashable(cb.frontier(), nbytes, offset));
}

void register_align_fixup(CodeBlock& cb, int nbytes, int offset = 0) {
  mcg->cgFixups().m_alignFixups.emplace(
    cb.frontier(),
    std::make_pair(nbytes, offset)
  );
}

///////////////////////////////////////////////////////////////////////////////

#define EMIT_BODY(cb, inst, size, ...)  \
  ([&] {                                \
    make_smashable(cb, size);           \
    register_align_fixup(cb, size);     \
    auto const start = cb.frontier();   \
    X64Assembler a { cb };              \
    a.inst(__VA_ARGS__);                \
    return start;                       \
  }())

TCA emit_smashable_movq(CodeBlock& cb, uint64_t imm, PhysReg d) {
  auto const start = EMIT_BODY(cb, movq, kMovLen, 0xdeadbeeffeedface, d);

  auto immp = reinterpret_cast<uint64_t*>(
    cb.frontier() - kMovLen + kMovImmOff
  );
  *immp = imm;

  return start;
}

TCA emit_smashable_cmpq(CodeBlock& cb, int32_t imm, PhysReg r, int8_t disp) {
  // TODO(#7831969): Get rid of this magic number when the sizeof_* API is made.
  return EMIT_BODY(cb, cmpq, 8, imm, r[disp]);
}

TCA emit_smashable_call(CodeBlock& cb, TCA target) {
  return EMIT_BODY(cb, call, kCallLen, target);
}

TCA emit_smashable_jmp(CodeBlock& cb, TCA target) {
  return EMIT_BODY(cb, jmp, kJmpLen, target);
}

TCA emit_smashable_jcc(CodeBlock& cb, TCA target, ConditionCode cc) {
  assertx(cc != CC_None);
  return EMIT_BODY(cb, jcc, kJccLen, cc, target);
}

std::pair<TCA,TCA>
emit_smashable_jcc_and_jmp(CodeBlock& cb, TCA target, ConditionCode cc) {
  assertx(cc != CC_None);

  // Make the instructions individually smashable.
  make_smashable(cb, kJccLen);
  make_smashable(cb, kJccLen + kJmpLen, kJccLen);

  assertx(is_smashable(cb.frontier(), kJccLen));
  assertx(is_smashable(cb.frontier(), kJccLen + kJmpLen, kJccLen));

  register_align_fixup(cb, kJccLen);
  register_align_fixup(cb, kJccLen + kJmpLen, kJccLen);

  X64Assembler a { cb };
  auto const jcc = cb.frontier();
  a.jcc(cc, target);
  auto const jmp = cb.frontier();
  a.jmp(target);

  return std::make_pair(jcc, jmp);
}

#undef EMIT_BODY

///////////////////////////////////////////////////////////////////////////////

void smash_movq(TCA inst, uint64_t imm) {
  always_assert(is_smashable(inst, kMovLen));
  *reinterpret_cast<uint64_t*>(inst + kMovImmOff) = imm;
}

void smash_call(TCA inst, TCA target) {
  always_assert(is_smashable(inst, kCallLen));
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

void smash_jmp(TCA inst, TCA target) {
  always_assert(is_smashable(inst, kJmpLen));

  auto& cb = mcg->code.blockFor(inst);
  CodeCursor cursor { cb, inst };
  X64Assembler a { cb };

  if (target > inst && target - inst <= kJmpLen) {
    a.emitNop(target - inst);
  } else {
    a.jmp(target);
  }
}

void smash_jcc(TCA inst, TCA target) {
  always_assert(is_smashable(inst, kJccLen));
  X64Assembler::patchJcc(inst, target);
}

///////////////////////////////////////////////////////////////////////////////

uint64_t smashable_movq_imm(TCA inst) {
  return *reinterpret_cast<uint64_t*>(inst + kMovImmOff);
}

TCA smashable_call_target(TCA inst) {
  if (inst[0] != 0xE8) return nullptr;
  return inst + 5 + ((int32_t*)(inst + 5))[-1];
}

TCA smashable_jmp_target(TCA inst) {
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

TCA smashable_jcc_target(TCA inst) {
  if (inst[0] != 0x0F || (inst[1] & 0xF0) != 0x80) return nullptr;
  return inst + 6 + ((int32_t*)(inst + 6))[-1];
}

ConditionCode smashable_jcc_cond(TCA inst) {
  return DecodedInstruction(inst).jccCondCode();
}

///////////////////////////////////////////////////////////////////////////////

}}}
