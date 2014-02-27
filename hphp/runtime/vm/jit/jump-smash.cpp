/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/arch.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

bool isSmashable(Address frontier, int nBytes, int offset /* = 0 */) {
  switch (arch()) {
    case Arch::X64: {
      assert(nBytes <= int(kX64CacheLineSize));
      uintptr_t iFrontier = uintptr_t(frontier) + offset;
      uintptr_t lastByte = uintptr_t(frontier) + nBytes - 1;
      return (iFrontier & ~kX64CacheLineMask) ==
              (lastByte & ~kX64CacheLineMask);
    }
    case Arch::ARM:
      // See prepareForSmash().
      return true;
  }
  not_reached();
}

void prepareForSmash(CodeBlock& cb, int nBytes, int offset /* = 0 */) {
  switch (arch()) {
    case Arch::X64:
      if (!isSmashable(cb.frontier(), nBytes, offset)) {
        X64Assembler a { cb };
        int gapSize = (~(uintptr_t(a.frontier()) + offset) &
                       kX64CacheLineMask) + 1;
        a.emitNop(gapSize);
        assert(isSmashable(a.frontier(), nBytes, offset));
      }
      break;
    case Arch::ARM:
      // Don't do anything. We don't smash code on ARM; we smash non-executable
      // data -- an 8-byte pointer -- that's embedded in the instruction stream.
      // As long as that data is 8-byte aligned, it's safe to smash. All
      // instructions are 4 bytes wide, so we'll just emit a single nop if
      // needed to align the data. This is done in emitSmashableJump.
      break;
  }
}

void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
                            TestAndSmashFlags flags) {
  switch (arch()) {
    case Arch::X64: {
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
      break;
    }
    case Arch::ARM: {
      // Nothing. See prepareForSmash().
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////

static void smashX64JmpOrCall(TCA addr, TCA dest, bool isCall) {
  // Unconditional rip-relative jmps can also be encoded with an EB as the first
  // byte, but that means the delta is 1 byte, and we shouldn't be encoding
  // smashable jumps that way.
  assert(isSmashable(addr, X64::kJmpLen));

  auto& cb = mcg->code.blockFor(addr);
  CodeCursor cursor { cb, addr };
  X64Assembler a { cb };
  if (dest > addr && dest - addr <= X64::kJmpLen) {
    assert(!isCall);
    a.  emitNop(dest - addr);
  } else if (isCall) {
    a.  call   (dest);
  } else {
    a.  jmp    (dest);
  }
}

static void smashARMJmpOrCall(TCA addr, TCA dest, bool isCall) {
  // Assert that this is actually the instruction sequence we expect
  DEBUG_ONLY auto ldr = vixl::Instruction::Cast(addr);
  DEBUG_ONLY auto branch = vixl::Instruction::Cast(addr + 4);
  assert(ldr->Bits(31, 24) == 0x58);
  assert((branch->Bits(31, 10) == 0x3587C0 ||
          branch->Bits(31, 10) == 0x358FC0) &&
         branch->Bits(4, 0) == 0);

  // These offsets are asserted in emitSmashableJump and emitSmashableCall. We
  // wrote two instructions for an unconditional jump, or three for a call, and
  // then the jump/call destination was written at the next 8-byte boundary.
  auto dataPtr = (isCall ? addr + 12 : addr + 8);
  if ((uintptr_t(dataPtr) & 7) != 0) {
    dataPtr += 4;
    assert((uintptr_t(dataPtr) & 7) == 0);
  }
  *reinterpret_cast<TCA*>(dataPtr) = dest;
}

//////////////////////////////////////////////////////////////////////

void smashJmp(TCA jmpAddr, TCA newDest) {
  assert(MCGenerator::canWrite());
  FTRACE(2, "smashJmp: {} -> {}\n", jmpAddr, newDest);
  switch (arch()) {
    case Arch::X64:
      smashX64JmpOrCall(jmpAddr, newDest, false);
      break;
    case Arch::ARM:
      smashARMJmpOrCall(jmpAddr, newDest, false);
      break;
  }
}

void smashCall(TCA callAddr, TCA newDest) {
  assert(MCGenerator::canWrite());
  FTRACE(2, "smashCall: {} -> {}\n", callAddr, newDest);
  switch (arch()) {
    case Arch::X64:
      smashX64JmpOrCall(callAddr, newDest, true);
      break;
    case Arch::ARM:
      smashARMJmpOrCall(callAddr, newDest, true);
      break;
  }
}

void smashJcc(TCA jccAddr, TCA newDest) {
  assert(MCGenerator::canWrite());
  FTRACE(2, "smashJcc: {} -> {}\n", jccAddr, newDest);
  switch (arch()) {
    case Arch::X64: {
      // Make sure the encoding is what we expect. It has to be a rip-relative
      // jcc with a 4-byte delta.
      assert(*jccAddr == 0x0F && (*(jccAddr + 1) & 0xF0) == 0x80);
      assert(isSmashable(jccAddr, X64::kJmpccLen));

      // Can't use the assembler to write out a new instruction, because we have
      // to preserve the condition code.
      auto newDelta = safe_cast<int32_t>(newDest - jccAddr - X64::kJmpccLen);
      auto deltaAddr = reinterpret_cast<int32_t*>(jccAddr
                                                  + X64::kJmpccLen
                                                  - X64::kJmpImmBytes);
      *deltaAddr = newDelta;
      break;
    }
    case Arch::ARM: {
      // This offset is asserted in emitSmashableJump. We wrote three
      // instructions. Then the jump destination was written at the next 8-byte
      // boundary.
      auto dataPtr = jccAddr + 12;
      if ((uintptr_t(dataPtr) & 7) != 0) {
        dataPtr += 4;
        assert((uintptr_t(dataPtr) & 7) == 0);
      }
      *reinterpret_cast<TCA*>(dataPtr) = newDest;
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////

void emitSmashableJump(CodeBlock& cb, TCA dest,
                       ConditionCode cc) {
  switch (arch()) {
    case Arch::X64: {
      X64Assembler a { cb };
      if (cc == CC_None) {
        assert(isSmashable(cb.frontier(), X64::kJmpLen));
        a.  jmp(dest);
      } else {
        assert(isSmashable(cb.frontier(), X64::kJmpccLen));
        a.  jcc(cc, dest);
      }
      break;
    }
    case Arch::ARM: {
      vixl::MacroAssembler a { cb };
      vixl::Label targetData;
      vixl::Label afterData;
      DEBUG_ONLY auto start = cb.frontier();

      // We emit the target address straight into the instruction stream, and
      // then do a pc-relative load to read it. This neatly sidesteps the
      // problem of concurrent modification and execution, as well as the
      // problem of 19- and 26-bit jump offsets (not big enough). It does,
      // however, entail an indirect jump.
      if (cc == CC_None) {
        a.    Ldr  (ARM::rAsm, &targetData);
        a.    Br   (ARM::rAsm);
        if (!cb.isFrontierAligned(8)) {
          a.  Nop  ();
          assert(cb.isFrontierAligned(8));
        }
        a.    bind (&targetData);
        a.    dc64 (reinterpret_cast<int64_t>(dest));

        // If this assert breaks, you need to change smashJmp
        assert(targetData.target() == start + 8 ||
               targetData.target() == start + 12);
      } else {
        a.    B    (&afterData, InvertCondition(ARM::convertCC(cc)));
        a.    Ldr  (ARM::rAsm, &targetData);
        a.    Br   (ARM::rAsm);
        if (!cb.isFrontierAligned(8)) {
          a.  Nop  ();
          assert(cb.isFrontierAligned(8));
        }
        a.    bind (&targetData);
        a.    dc64 (reinterpret_cast<int64_t>(dest));
        a.    bind (&afterData);

        // If this assert breaks, you need to change smashJcc
        assert(targetData.target() == start + 12 ||
               targetData.target() == start + 16);
      }
      break;
    }
  }
}

void emitSmashableCall(CodeBlock& cb, TCA dest) {
  switch (arch()) {
    case Arch::X64: {
      X64Assembler a { cb };
      assert(isSmashable(cb.frontier(), X64::kCallLen));
      a.  call(dest);
      break;
    }
    case Arch::ARM: {
      vixl::MacroAssembler a { cb };
      vixl::Label afterData;
      vixl::Label targetData;
      DEBUG_ONLY auto start = cb.frontier();

      a.  Ldr  (ARM::rAsm, &targetData);
      a.  Blr  (ARM::rAsm);
      // When the call returns, jump over the data.
      a.  B    (&afterData);
      if (!cb.isFrontierAligned(8)) {
        a.Nop  ();
        assert(cb.isFrontierAligned(8));
      }
      a.  bind (&targetData);
      a.  dc64 (reinterpret_cast<int64_t>(dest));
      a.  bind (&afterData);

      // If this assert breaks, you need to change smashCall
      assert(targetData.target() == start + 12 ||
             targetData.target() == start + 16);
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////

TCA jmpTarget(TCA jmp) {
  switch (arch()) {
    case Arch::X64:
      if (jmp[0] != 0xe9) return nullptr;
      return jmp + 5 + ((int32_t*)(jmp + 5))[-1];
    case Arch::ARM: {
      // This doesn't verify that each of the two or three instructions that
      // make up this sequence matches; just the first one and the indirect
      // jump.
      using namespace vixl;
      Instruction* ldr = Instruction::Cast(jmp);
      if (ldr->Bits(31, 24) != 0x58) return nullptr;

      Instruction* br = Instruction::Cast(jmp + 4);
      if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

      uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 8);
      if ((dest & 7) != 0) {
        dest += 4;
        assert((dest & 7) == 0);
      }
      return *reinterpret_cast<TCA*>(dest);
    }
  }
  not_reached();
}

TCA jccTarget(TCA jmp) {
  switch (arch()) {
    case Arch::X64:
      if (jmp[0] != 0x0F || (jmp[1] & 0xF0) != 0x80) return nullptr;
      return jmp + 6 + ((int32_t*)(jmp + 6))[-1];
    case Arch::ARM: {
      using namespace vixl;
      Instruction* b = Instruction::Cast(jmp);
      if (b->Bits(31, 24) != 0x54 || b->Bit(4) != 0) return nullptr;

      Instruction* br = Instruction::Cast(jmp + 8);
      if (br->Bits(31, 10) != 0x3587C0 || br->Bits(4, 0) != 0) return nullptr;

      uintptr_t dest = reinterpret_cast<uintptr_t>(jmp + 12);
      if ((dest & 7) != 0) {
        dest += 4;
        assert((dest & 7) == 0);
      }
      return *reinterpret_cast<TCA*>(dest);
    }
  }
  not_reached();
}

TCA callTarget(TCA call) {
  switch (arch()) {
    case Arch::X64:
      if (call[0] != 0xE8) return nullptr;
      return call + 5 + ((int32_t*)(call + 5))[-1];
    case Arch::ARM: {
      using namespace vixl;
      Instruction* ldr = Instruction::Cast(call);
      if (ldr->Bits(31, 24) != 0x58) return nullptr;

      Instruction* blr = Instruction::Cast(call + 4);
      if (blr->Bits(31, 10) != 0x358FC0 || blr->Bits(4, 0) != 0) return nullptr;

      uintptr_t dest = reinterpret_cast<uintptr_t>(blr + 8);
      if ((dest & 7) != 0) {
        dest += 4;
        assert((dest & 7) == 0);
      }
      return *reinterpret_cast<TCA*>(dest);
    }
  }
  not_reached();
}

}}
