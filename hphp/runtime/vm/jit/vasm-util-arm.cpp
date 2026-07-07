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

#include "hphp/runtime/vm/jit/vasm-util-arm.h"

#include "hphp/vixl/hphp-compat.h"

namespace HPHP::jit::arm {

using vixl::is_int21;
using vixl::aarch64::Assembler;
using vixl::aarch64::CBNZ_w;
using vixl::aarch64::CBNZ_x;
using vixl::aarch64::CompareBranchMask;
using vixl::aarch64::Instruction;
using vixl::aarch64::LDR_w_lit;
using vixl::aarch64::LDR_x_lit;
using vixl::aarch64::LDR_w_unsigned;
using vixl::aarch64::LDR_x_unsigned;
using vixl::aarch64::LoadLiteralMask;
using vixl::aarch64::LoadStoreUnsignedOffsetMask;
using vixl::aarch64::Register;
using vixl::aarch64::Rt_mask;
using vixl::aarch64::kInstructionSize;
using vixl::aarch64::kPageSize;
using vixl::aarch64::kWRegSize;
using vixl::aarch64::kXRegSize;

static_assert(kPageSize == (1u << 12),
              "ADRP/LDR page-offset encoding assumes 4KB pages");

namespace {

bool isUnsignedOffsetLdr(const Instruction* ldr) {
  auto const op = ldr->Mask(LoadStoreUnsignedOffsetMask);
  return op == LDR_w_unsigned || op == LDR_x_unsigned;
}

bool isPairAdrpLdr(const Instruction* adrp, const Instruction* ldr) {
  if (!adrp || !ldr) return false;
  if (!adrp->IsAdrp()) return false;
  if (!isUnsignedOffsetLdr(ldr)) return false;
  return adrp->GetRd() == ldr->GetRn() && adrp->GetRd() == ldr->GetRt();
}

bool isNearLdr(const Instruction* ldr) {
  if (!ldr->IsLoadLiteral()) return false;
  auto const op = ldr->Mask(LoadLiteralMask);
  return op == LDR_w_lit || op == LDR_x_lit;
}

uint8_t ldrWidth(const Instruction* ldr) {
  if (isNearLdr(ldr)) {
    return ldr->Mask(LoadLiteralMask) == LDR_w_lit ? 32 : 64;
  }
  assertx(isUnsignedOffsetLdr(ldr));
  return ldr->Mask(LoadStoreUnsignedOffsetMask) == LDR_w_unsigned ? 32 : 64;
}

uint32_t ldrReg(const Instruction* ldr) {
  if (isNearLdr(ldr)) return ldr->Rd();
  assertx(isUnsignedOffsetLdr(ldr));
  return ldr->Rt();
}

}

CompareAndBranchDetails getCompareAndBranchDetails(const Instruction* cb) {
  assertx(cb->IsCompareBranch());
  auto const reg =
    Register(cb->Rt(), cb->GetSixtyFourBits() ? kXRegSize : kWRegSize);
  auto const op = cb->Mask(CompareBranchMask);
  return {reg, op == CBNZ_w || op == CBNZ_x};
}

LoadLiteral LoadLiteral::at(Instruction* start, const Instruction* end) {
  if (start == nullptr) return {};
  if (isNearLdr(start)) {
    return LoadLiteral{start, nullptr, false};
  }
  auto const next = start->GetNextInstruction();
  if (end != nullptr && next >= end) return {};
  if (!isPairAdrpLdr(start, next)) return {};
  return LoadLiteral{start, next, true};
}

uint8_t LoadLiteral::width() const {
  assertx(valid());
  return ldrWidth(ldr());
}

uint32_t LoadLiteral::destReg() const {
  assertx(valid());
  return ldrReg(ldr());
}

uint8_t LoadLiteral::offset() const {
  assertx(valid());
  return far_ ? kInstructionSize : 0;
}

uint8_t* LoadLiteral::literalAddress(const Instruction* logicalStart) const {
  assertx(valid());

  if (!far_) {
    auto const base = reinterpret_cast<uint64_t>(logicalStart);
    auto const off = ldr()->GetImmLLiteral() * static_cast<int>(kInstructionSize);
    return reinterpret_cast<uint8_t*>(base + off);
  }

  auto const page = reinterpret_cast<uintptr_t>(
    adrp()->GetImmPCOffsetTarget(logicalStart)
  );
  auto const off = ldr()->GetImmLSUnsigned() << ldr()->GetSizeLS();
  return reinterpret_cast<uint8_t*>(page + off);
}

bool LoadLiteral::reachableByNearLiteral(const Instruction* target,
                                         const Instruction* from) {
  auto const imm = reinterpret_cast<const uint8_t*>(target) -
                   reinterpret_cast<const uint8_t*>(from);
  if ((imm & (kInstructionSize - 1)) != 0) return false;
  return is_int21(imm);
}

bool LoadLiteral::reachableByFarLiteral(const Instruction* target,
                                        const Instruction* from) {
  auto const fromPage = reinterpret_cast<uintptr_t>(from) / kPageSize;
  auto const targetPage = reinterpret_cast<uintptr_t>(target) / kPageSize;
  auto const pageDiff =
    static_cast<int64_t>(targetPage) - static_cast<int64_t>(fromPage);
  return is_int21(pageDiff);
}

bool LoadLiteral::setTarget(const Instruction* target,
                            const Instruction* from) const {
  assertx(valid());

  if (!far_) {
    if (!reachableByNearLiteral(target, from)) return false;

    auto* l = ldr();
    auto const op = ldrWidth(l) == 32 ? LDR_w_lit : LDR_x_lit;
    l->SetInstructionBits(op | (l->GetInstructionBits() & Rt_mask));
    l->SetImmPCOffsetTarget(target, from);
    return true;
  }

  auto const targetAddr = reinterpret_cast<uintptr_t>(target);
  auto const pageOffset = targetAddr & (kPageSize - 1);
  auto* l = ldr();
  assertx(isUnsignedOffsetLdr(l));
  auto const shift = l->GetSizeLS();
  auto const scale = 1u << shift;

  if ((pageOffset & (scale - 1)) != 0) return false;
  if (!reachableByFarLiteral(target, from)) return false;

  auto const imm12 = pageOffset >> shift;
  adrp()->SetImmPCOffsetTarget(target, from);

  auto const kImmMask = Assembler::ImmLSUnsigned(0xfff);
  l->SetInstructionBits(
    (l->GetInstructionBits() & ~kImmMask) | Assembler::ImmLSUnsigned(imm12)
  );
  return true;
}

}
