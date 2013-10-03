/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/jump-smash.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

bool isSmashable(Address frontier, int nBytes, int offset /* = 0 */) {
  if (arch() == Arch::X64) {
    assert(nBytes <= int(kX64CacheLineSize));
    uintptr_t iFrontier = uintptr_t(frontier) + offset;
    uintptr_t lastByte = uintptr_t(frontier) + nBytes - 1;
    return (iFrontier & ~kX64CacheLineMask) == (lastByte & ~kX64CacheLineMask);
  } else if (arch() == Arch::ARM) {
    // See prepareForSmash().
    return true;
  } else {
    not_implemented();
  }
}

void prepareForSmash(CodeBlock& cb, int nBytes, int offset /* = 0 */) {
  if (arch() == Arch::X64) {
    if (!isSmashable(cb.frontier(), nBytes, offset)) {
      Transl::X64Assembler a { cb };
      int gapSize = (~(uintptr_t(a.frontier()) + offset) &
                     kX64CacheLineMask) + 1;
      a.emitNop(gapSize);
      assert(isSmashable(a.frontier(), nBytes, offset));
    }
  } else if (arch() == Arch::ARM) {
    // Don't do anything. We don't smash code on ARM; we smash non-executable
    // data -- an 8-byte pointer -- that's embedded in the instruction stream.
    // As long as that data is 8-byte aligned, it's safe to smash. All
    // instructions are 4 bytes wide, so we'll just emit a single nop if needed
    // to align the data. This is done in emitSmashableJump.
  } else {
    not_implemented();
  }
}

void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
                            TestAndSmashFlags flags) {
  if (arch() == Arch::X64) {
    using namespace X64;
    switch (flags) {
    case TestAndSmashFlags::kAlignJcc:
      prepareForSmash(cb, testBytes + kJmpccLen, testBytes);
      assert(isSmashable(cb.frontier() + testBytes, kJmpccLen));
      break;
    case TestAndSmashFlags::kAlignJccImmediate:
      prepareForSmash(cb,
                      testBytes + kJmpccLen,
                      testBytes + kJmpccLen - kJmpImmBytes);
      assert(isSmashable(cb.frontier() + testBytes, kJmpccLen,
                         kJmpccLen - kJmpImmBytes));
      break;
    case TestAndSmashFlags::kAlignJccAndJmp:
      // Ensure that the entire jcc, and the entire jmp are smashable
      // (but we dont need them both to be in the same cache line)
      prepareForSmash(cb, testBytes + kJmpccLen, testBytes);
      prepareForSmash(cb, testBytes + kJmpccLen + kJmpLen,
                      testBytes + kJmpccLen);
      assert(isSmashable(cb.frontier() + testBytes, kJmpccLen));
      assert(isSmashable(cb.frontier() + testBytes + kJmpccLen, kJmpLen));
      break;
    }
  } else if (arch() == Arch::ARM) {
    // Nothing. See prepareForSmash().
  } else {
    not_implemented();
  }
}

//////////////////////////////////////////////////////////////////////

static void smashX64JmpOrCall(TCA addr, TCA dest, bool isCall) {
  // Unconditional rip-relative jmps can also be encoded with an EB as the first
  // byte, but that means the delta is 1 byte, and we shouldn't be encoding
  // smashable jumps that way.
  assert(isSmashable(addr, X64::kJmpLen));

  auto& cb = tx64->codeBlockFor(addr);
  CodeCursor cursor { cb, addr };
  Transl::X64Assembler a { cb };
  if (dest > addr && dest - addr <= X64::kJmpLen) {
    assert(!isCall);
    a.  emitNop(dest - addr);
  } else if (isCall) {
    a.  call   (dest);
  } else {
    a.  jmp    (dest);
  }
}

//////////////////////////////////////////////////////////////////////

void smashJmp(TCA jmpAddr, TCA newDest) {
  assert(TranslatorX64::canWrite());
  FTRACE(2, "smashJmp: {} -> {}\n", jmpAddr, newDest);
  if (arch() == Arch::X64) {
    smashX64JmpOrCall(jmpAddr, newDest, false);
  } else {
    not_implemented();
  }
}

void smashCall(TCA callAddr, TCA newDest) {
  assert(TranslatorX64::canWrite());
  FTRACE(2, "smashCall: {} -> {}\n", callAddr, newDest);
  if (arch() == Arch::X64) {
    smashX64JmpOrCall(callAddr, newDest, true);
  } else {
    not_implemented();
  }
}

void smashJcc(TCA jccAddr, TCA newDest) {
  assert(TranslatorX64::canWrite());
  FTRACE(2, "smashJcc: {} -> {}\n", jccAddr, newDest);
  if (arch() == Arch::X64) {
    // Make sure the encoding is what we expect. It has to be a rip-relative jcc
    // with a 4-byte delta.
    assert(*jccAddr == 0x0F && (*(jccAddr + 1) & 0xF0) == 0x80);
    assert(isSmashable(jccAddr, X64::kJmpccLen));

    // Can't use the assembler to write out a new instruction, because we have
    // to preserve the condition code.
    auto newDelta = safe_cast<int32_t>(newDest - jccAddr - X64::kJmpccLen);
    auto deltaAddr = reinterpret_cast<int32_t*>(jccAddr
                                                + X64::kJmpccLen
                                                - X64::kJmpImmBytes);
    *deltaAddr = newDelta;
  } else {
    // This offset is asserted in emitSmashableJump. We wrote four instructions.
    // Then the jump destination was written at the next 8-byte boundary.
    auto dataPtr = jccAddr + 16;
    if ((uintptr_t(dataPtr) & 7) != 0) {
      dataPtr += 4;
      assert((uintptr_t(dataPtr) & 7) == 0);
    }
    *reinterpret_cast<TCA*>(dataPtr) = newDest;
  }
}

//////////////////////////////////////////////////////////////////////

void emitSmashableJump(CodeBlock& cb, Transl::TCA dest,
                       Transl::ConditionCode cc) {
  if (arch() == Arch::X64) {
    Transl::X64Assembler a { cb };
    if (cc == CC_None) {
      assert(isSmashable(cb.frontier(), X64::kJmpLen));
      a.  jmp(dest);
    } else {
      assert(isSmashable(cb.frontier(), X64::kJmpccLen));
      a.  jcc(cc, dest);
    }
  } else {
    vixl::MacroAssembler a { cb };
    vixl::Label targetData;
    vixl::Label afterData;
    DEBUG_ONLY auto start = cb.frontier();

    // We emit the target address straight into the instruction stream, and then
    // do a pc-relative load to read it. This neatly sidesteps the problem of
    // concurrent modification and execution, as well as the problem of 19- and
    // 26-bit jump offsets (not big enough). It does, however, entail an
    // indirect jump.
    if (cc == CC_None) {
      a.    Adr  (ARM::rAsm, &targetData);
      a.    Ldr  (ARM::rAsm, ARM::rAsm[0]);
      a.    Br   (ARM::rAsm);
      if (!cb.isFrontierAligned(8)) {
        a.  Nop  ();
        assert(cb.isFrontierAligned(8));
      }
      a.    bind (&targetData);
      a.    dc64 (reinterpret_cast<int64_t>(dest));

      // If this assert breaks, you need to change smashJmp
      assert(targetData.target() == start + 12 ||
             targetData.target() == start + 16);
    } else {
      a.    B    (&afterData, InvertCondition(ARM::convertCC(cc)));
      a.    Adr  (ARM::rAsm, &targetData);
      a.    Ldr  (ARM::rAsm, ARM::rAsm[0]);
      a.    Br   (ARM::rAsm);
      if (!cb.isFrontierAligned(8)) {
        a.  Nop  ();
        assert(cb.isFrontierAligned(8));
      }
      a.    bind (&targetData);
      a.    dc64 (reinterpret_cast<int64_t>(dest));
      a.    bind (&afterData);

      // If this assert breaks, you need to change smashJcc
      assert(targetData.target() == start + 16 ||
             targetData.target() == start + 20);
    }
  }
}

}}
