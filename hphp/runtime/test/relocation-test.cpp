/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/util/arch.h"
#include "hphp/util/data-block.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

#include <gtest/gtest.h>

using namespace vixl;

namespace HPHP { namespace jit {

namespace arm {

uint8_t* code_;
size_t blockSize_ = 4096;

void initBlocks(size_t size, CodeBlock& main, DataBlock& data) {
  blockSize_ = size;
  code_ = static_cast<uint8_t*>(mmap(nullptr, blockSize_,
                                         PROT_READ | PROT_WRITE | PROT_EXEC,
                                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  auto dataSize = 100;
  auto codeSize = blockSize_ - dataSize;
  // None of these tests should use much data.
  auto data_buffer = code_ + codeSize;

  main.init(code_, codeSize, "test");
  data.init(data_buffer, dataSize, "data");

}

void freeBlocks() {
  munmap(code_, blockSize_);
}

/*
 * Tests relocating PC relative conditional branches
 * to a sequence of a conditional branch and an absolute
 * branch to register. See arm::relocatePCRelativeHelper()
 *
 *     b.cc <target>
 * to
 *     b.!cc <past>
 *     movz/movk $tmp, <target>
 *     br $tmp
 */
TEST(Relocation, RelocateBccImm2MovzMovkBccReg) {
  if (arch() != Arch::ARM) {
    SUCCEED();
    return;
  }

  // 1. Init
  CodeBlock main;
  DataBlock data;
  initBlocks(4096, main, data);
  SCOPE_EXIT { freeBlocks(); };

  CGMeta meta;

  // 2. Emit b.cc <imm> of -0x40000 (19-bit int min)
  auto start = main.frontier();

  MacroAssembler a { main };
  meta.addressImmediates.insert(main.frontier());
  a.b(-0x40000, eq);
  auto bccOrig = Instruction::Cast(start);
  auto const cond = static_cast<Condition>(bccOrig->ConditionBranch());

  auto end = main.frontier();

  // 3. Call relocate()
  RelocationInfo rel;
  AreaIndex ai;
  auto instr = Instruction::Cast(end);
  relocate(rel, main, start, end, main, meta, nullptr, ai);

  // 4. Expect !b.cc, movz/movk X and br X where with imm in X
  auto bcc = instr;
  EXPECT_TRUE(bcc->IsCondBranchImm());
  EXPECT_EQ(bcc->ConditionBranch(), InvertCondition(cond));

  auto movz = bcc->NextInstruction();
  EXPECT_TRUE(movz->IsMovz());
  const auto rd = movz->Rd();
  uint64_t target = movz->ImmMoveWide() << (16 * movz->ShiftMoveWide());
  instr = movz->NextInstruction();
  while (instr->IsMovk()) {
    EXPECT_EQ(instr->Rd(), rd);
    target |= instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    instr = instr->NextInstruction();
  }
  EXPECT_EQ(Instruction::Cast(target), bccOrig->ImmPCOffsetTarget());

  auto br = instr;
  EXPECT_TRUE(br->IsUncondBranchReg());
  EXPECT_EQ(br->Rn(), rd);
  EXPECT_EQ(bcc->ImmPCOffsetTarget(), br->NextInstruction());
}

/*
 * Tests relocating PC relative compare and branch
 * to a sequence of a compare and branch and an absolute
 * branch to register. See arm::relocatePCRelativeHelper()
 *
 *     cbz $rt, <target>
 * to
 *     cbnz $rt, <past>
 *     movz/movk $tmp, <target>
 *     br $tmp
 */
TEST(Relocation, RelocateCbz2MovzMovkCbnzReg) {
  if (arch() != Arch::ARM) {
    SUCCEED();
    return;
  }

  // 1. Init
  CodeBlock main;
  DataBlock data;
  initBlocks(4096, main, data);
  SCOPE_EXIT { freeBlocks(); };

  CGMeta meta;

  // 2. Emit cbz <imm> of -0x40000 (19-bit int min)
  auto start = main.frontier();

  MacroAssembler a { main };
  meta.addressImmediates.insert(main.frontier());
  a.cbz(x0, -0x40000);
  auto cbzOrig = Instruction::Cast(start);

  auto end = main.frontier();

  // 3. Call relocate()
  RelocationInfo rel;
  AreaIndex ai;
  auto instr = Instruction::Cast(end);
  relocate(rel, main, start, end, main, meta, nullptr, ai);

  // 4. Expect cbnz, movz/movk X and br X where with imm in X
  auto cbnz = instr;
  EXPECT_TRUE(cbnz->Mask(LoadStoreUnsignedOffsetMask) == CBNZ_x);
  const auto rt = cbnz->Rt();
  EXPECT_EQ(cbzOrig->Rt(), rt);

  auto movz = cbnz->NextInstruction();
  EXPECT_TRUE(movz->IsMovz());
  const auto rd = movz->Rd();
  uint64_t target = movz->ImmMoveWide() << (16 * movz->ShiftMoveWide());
  instr = movz->NextInstruction();
  while (instr->IsMovk()) {
    EXPECT_EQ(instr->Rd(), rd);
    target |= instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    instr = instr->NextInstruction();
  }
  EXPECT_EQ(Instruction::Cast(target), cbzOrig->ImmPCOffsetTarget());

  auto br = instr;
  EXPECT_TRUE(br->IsUncondBranchReg());
  EXPECT_EQ(br->Rn(), rd);
  EXPECT_EQ(cbnz->ImmPCOffsetTarget(), br->NextInstruction());
}

/*
 * Tests relocating PC relative test bit and branch
 * to a sequence of a test bit and branch and an absolute
 * branch to register. See arm::relocatePCRelativeHelper()
 *
 *     tbz $rt, <bit_pos>, <target>
 * to
 *     tbnz $rt, <it_pos>, <past>
 *     movz/movk $tmp, <target>
 *     br $tmp
 */
TEST(Relocation, RelocateTbz2MovzMovkTbnzReg) {
  if (arch() != Arch::ARM) {
    SUCCEED();
    return;
  }

  // 1. Init
  CodeBlock main;
  DataBlock data;
  initBlocks(4096, main, data);
  SCOPE_EXIT { freeBlocks(); };

  CGMeta meta;

  // 2. Emit tbz <imm> of -0x2000 (14-bit int min)
  auto start = main.frontier();

  MacroAssembler a { main };
  meta.addressImmediates.insert(main.frontier());
  a.tbz(x0, 3, -0x2000);
  auto tbzOrig = Instruction::Cast(start);

  auto end = main.frontier();

  // 3. Call relocate()
  RelocationInfo rel;
  AreaIndex ai;
  auto instr = Instruction::Cast(end);
  relocate(rel, main, start, end, main, meta, nullptr, ai);

  // 4. Expect tbnz, movz/movk X and br X where with imm in X
  auto tbnz = instr;
  EXPECT_TRUE(tbnz->Mask(LoadStoreUnsignedOffsetMask) == TBNZ);
  const auto rt = tbnz->Rt();
  EXPECT_EQ(tbzOrig->Rt(), rt);
  const auto bit_pos = tbnz->ImmTestBranchBit40();
  EXPECT_EQ(bit_pos, 3);

  auto movz = tbnz->NextInstruction();
  EXPECT_TRUE(movz->IsMovz());
  const auto rd = movz->Rd();
  uint64_t target = movz->ImmMoveWide() << (16 * movz->ShiftMoveWide());
  instr = movz->NextInstruction();
  while (instr->IsMovk()) {
    EXPECT_EQ(instr->Rd(), rd);
    target |= instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    instr = instr->NextInstruction();
  }
  EXPECT_EQ(Instruction::Cast(target), tbzOrig->ImmPCOffsetTarget());

  auto br = instr;
  EXPECT_TRUE(br->IsUncondBranchReg());
  EXPECT_EQ(br->Rn(), rd);
  EXPECT_EQ(tbnz->ImmPCOffsetTarget(), br->NextInstruction());
}

/*
 * See arm::relocateImmediateHelper().
 */
TEST(Relocation, RelocateMovzMovkLdr2LdrLiteral) {

  if (arch() != Arch::ARM) {
    SUCCEED();
    return;
  }

  // 1. Init
  CodeBlock main;
  DataBlock data;
  initBlocks(4096, main, data);
  SCOPE_EXIT { freeBlocks(); };

  CGMeta meta;

  // 2. Emit movz $x17 addr; movk $x17 addr; ldr $0, $x17
  auto start = main.frontier();

  MacroAssembler a { main };
  // We will kill this nop during relocation.
  a.nop();
  // The relocator won't know this is a litteral.  This seems like a problem
  // with this test.
  a.dc64(0xdeadbeef);
  meta.addressImmediates.insert(main.frontier());
  a.Mov(x17, start + 4);

  a.Ldr(x0, MemOperand(x17, 0));

  auto end = main.frontier();

  // 3. Call relocate()
  RelocationInfo rel;
  AreaIndex ai;
  auto instr = Instruction::Cast(end + 8);
  relocate(rel, main, start, end, main, meta, nullptr, ai);

  // 4. Expect a Ldr Literal to $x0 from addr
  EXPECT_TRUE(instr->IsLoadLiteral());
  EXPECT_EQ(instr->Rd(), 0);
  EXPECT_EQ(reinterpret_cast<TCA>(instr->LiteralAddress()), end);
}

/*
 * Tests the adjustment of a movz/movk target after that
 * movz/movk is relocated. See arm::relocateImmediateHelper().
 */
TEST(Relocation, RelocateAdjustedMovzMovk) {
  if (arch() != Arch::ARM) {
    SUCCEED();
    return;
  }

  // 1. Init
  CodeBlock main;
  DataBlock data;
  initBlocks(5 << 20, main, data);
  SCOPE_EXIT { freeBlocks(); };

  CGMeta meta;

  // 2. Emit Mov of start into x17 after padding out 2MB+.
  auto start = main.frontier();

  main.setFrontier(start + (2 << 20) + 16);
  MacroAssembler a { main };
  meta.addressImmediates.insert(main.frontier());
  a.Mov(x17, start + 8);

  auto end = main.frontier();

  // 3. Call relocate()
  RelocationInfo rel;
  AreaIndex ai;
  auto instr = Instruction::Cast(end);
  relocate(rel, main, start, end, main, meta, nullptr, ai);

  // 4. Expect movz/movk of new target
  auto movz = instr + (2 << 20) + 16;
  EXPECT_TRUE(movz->IsMovz());
  EXPECT_EQ(movz->Rd(), 17);
  uint64_t target = movz->ImmMoveWide() << (16 * movz->ShiftMoveWide());
  instr = movz->NextInstruction();
  while (instr->IsMovk()) {
    EXPECT_EQ(instr->Rd(), 17);
    target |= instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    instr = instr->NextInstruction();
  }
  EXPECT_EQ(Instruction::Cast(target), Instruction::Cast(end + 8));
}

/*
 * Tests the adjustment of a movz/movk target after that
 * movz/movk is relocated. This is a second adjustment of
 * the internal reference triggered because the translation
 * changed in size during relocation. See second portion of
 * relocateImpl().
 */
TEST(Relocation, RelocateInternalAdjustedMovzMovk) {
  if (arch() != Arch::ARM) {
    SUCCEED();
    return;
  }

  // 1. Init
  CodeBlock main;
  DataBlock data;
  initBlocks(9 << 20, main, data);
  SCOPE_EXIT { freeBlocks(); };

  CGMeta meta;

  // 2. Emit Mov of start + 2MB into x17 and then pad out 4MB+.
  auto start = main.frontier();

  MacroAssembler a { main };
  meta.addressImmediates.insert(main.frontier());
  a.Mov(x17, start + (2 << 20) + 16);
  main.setFrontier(start + (4 << 20) + 32);

  auto end = main.frontier();

  // 3. Call relocate()
  RelocationInfo rel;
  AreaIndex ai;
  auto instr = Instruction::Cast(end);
  relocate(rel, main, start, end, main, meta, nullptr, ai);

  // 4. Expect movz/movk of new target
  auto movz = instr;
  EXPECT_TRUE(movz->IsMovz());
  EXPECT_EQ(movz->Rd(), 17);
  uint64_t target = movz->ImmMoveWide() << (16 * movz->ShiftMoveWide());
  instr = movz->NextInstruction();
  while (instr->IsMovk()) {
    EXPECT_EQ(instr->Rd(), 17);
    target |= instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    instr = instr->NextInstruction();
  }
  EXPECT_EQ(Instruction::Cast(target), Instruction::Cast(end + (2 << 20) + 16));
}

/*
 * Tests the adjustment of a movz/movk target when its target
 * is relocated. See arm::adjustInstruction().
 */
TEST(Relocation, AdjustMovzMovk) {
  if (arch() != Arch::ARM) {
    SUCCEED();
    return;
  }

  // 1. Init
  CodeBlock main;
  DataBlock data;
  initBlocks(3 << 20, main, data);
  SCOPE_EXIT { freeBlocks(); };

  CGMeta meta;

  // 2. Emit Mov of start + 2MB+ into x17. This is not relocated, but
  //    its target at 2MB+ will be.
  auto orig = main.frontier();

  MacroAssembler a { main };
  meta.addressImmediates.insert(main.frontier());
  a.Mov(x17, orig + (2 << 20) + 16);
  main.setFrontier(orig + (2 << 20) + 16);

  auto start = main.frontier();
  a.nop();
  auto end = main.frontier();

  // 3. Call relocate() and then adjust the mov/movk
  RelocationInfo rel;
  AreaIndex ai;
  relocate(rel, main, start, end, main, meta, nullptr, ai);
  adjustForRelocation(rel, orig, start);

  // 4. Expect movz/movk of new target
  auto movz = Instruction::Cast(orig);
  EXPECT_TRUE(movz->IsMovz());
  EXPECT_EQ(movz->Rd(), 17);
  uint64_t target = movz->ImmMoveWide() << (16 * movz->ShiftMoveWide());
  auto instr = movz->NextInstruction();
  while (instr->IsMovk()) {
    EXPECT_EQ(instr->Rd(), 17);
    target |= instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    instr = instr->NextInstruction();
  }
  EXPECT_EQ(Instruction::Cast(target), Instruction::Cast(end));
}

}}}
