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

#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

#include <folly/Format.h>

#include "hphp/ppc64-asm/decoder-ppc64.h"
#include "hphp/ppc64-asm/asm-ppc64.h"

#include "hphp/util/data-block.h"

#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#endif

namespace ppc64_asm {

bool DecodedInstruction::couldBeNearBranch() {
  assert(isFarBranch());
  ptrdiff_t diff = farBranchTarget() - m_ip;

  // assert already stated it's a Far branch, but depending if it's conditional
  // or not, an additional range can be used.
  return fitsOnNearBranch(diff, isFarBranch(AllowCond::OnlyUncond));
}

uint8_t* DecodedInstruction::nearBranchTarget() const {
  assert(isNearBranch());
  auto address = reinterpret_cast<uint64_t>(m_ip) + m_dinfo.branchOffset();
  return reinterpret_cast<uint8_t*>(address);
}

bool DecodedInstruction::setNearBranchTarget(uint8_t* target) {
  if (!isNearBranch()) return false;
  ptrdiff_t diff = target - m_ip;
  bool uncond = m_dinfo.isOffsetBranch(AllowCond::OnlyUncond);
  if (fitsOnNearBranch(diff, uncond)) {
    auto pinstr = reinterpret_cast<PPC64Instr*>(m_ip);
    *pinstr = m_dinfo.setBranchOffset(int32_t(diff));
    return true;
  } else {
    return false;
  }
}

bool DecodedInstruction::isImmediate() const {
  // if destination register is r12, then it's preparing to branch
  return isLimmediatePossible() && (reg::r12 != getLimmediateReg());
}

bool DecodedInstruction::setImmediate(int64_t value) {
  if (!isImmediate()) return false;

  // Initialize code block cb pointing to li64
  HPHP::CodeBlock cb;
  cb.init(m_ip, Assembler::kLimmLen, "setImmediate relocation");
  HPHP::CodeCursor cursor { cb, m_ip };
  Assembler a{ cb };

  a.limmediate(getLimmediateReg(), ssize_t(value), ImmType::TocOnly);

  // refresh m_imm and other parameters
  decode();
  return true;
}

bool DecodedInstruction::shrinkBranch() {
  // It should be a Far branch, otherwise don't do anything if it's Near.
  assertx(isBranch() && "Can't shrink instruction that is not a branch.");

  auto uncondBranch = isFarBranch(AllowCond::OnlyUncond);
  auto call = isCall();
  auto condBranch = isFarBranch(AllowCond::OnlyCond);

  if (uncondBranch || call || condBranch) {
    HPHP::CodeBlock cb;
    cb.init(m_ip, instr_size_in_bytes, "shrinkBranch relocation");
    HPHP::CodeCursor cursor { cb, m_ip };
    Assembler a { cb };

    if (uncondBranch || call) {     // unconditional will be set as b
      // offset will be patched later
      if (call) a.bl(0);
      else      a.b(0);
    } else {                        // conditional will be bc
      // grab conditional parameters
      auto branch_instr = m_ip + Assembler::kJccLen - instr_size_in_bytes;
      BranchParams bp(branch_instr);

      // offset will be patched later
      a.bc(bp.bo(), bp.bi(), 0);
    }
    // refresh m_size and other parameters
    decode();
    return true;
  }
  return false;
}

void DecodedInstruction::widenBranch(uint8_t* target) {
  // currently, it should be a Near branch, else don't do anything if it's Far.
  assertx(isBranch() && "Can't widen instruction that is not a branch.");

  if (isNearBranch()) {
    // grab conditional parameters
    BranchParams bp(m_ip);
    bool uncond = (bp.bo() == uint8_t(BranchParams::BO::Always));
    auto block_size = uncond ? Assembler::kCallLen : Assembler::kJccLen;

    HPHP::CodeBlock cb;
    cb.init(m_ip, block_size, "widenBranch relocation");
    HPHP::CodeCursor cursor { cb, m_ip };
    Assembler a { cb };
    a.branchFar(target, bp, ImmType::TocOnly, true);

    // refresh m_size and other parameters
    decode();
  }
}

bool DecodedInstruction::isBranch(AllowCond ac /* = AllowCond::Any */) const {
  return isFarBranch(ac) || isNearBranch(ac);
}

bool DecodedInstruction::isNearBranch(AllowCond ac /* = Any */) const {
  return m_dinfo.isOffsetBranch(ac);
}

bool DecodedInstruction::isFarBranch(AllowCond ac /* = Any */) const {
  return !getFarBranch(ac).isInvalid();
}

DecoderInfo DecodedInstruction::getFarBranch(AllowCond ac /* = Any */) const {
  return getFarBranchLength(ac).m_di;
}

// Returns -1 if not found, otherwise the offset from m_ip that a register
// branch instruction is found
DecInfoOffset DecodedInstruction::getFarBranchLength(AllowCond ac) const {
  DecInfoOffset ret;
  if (!isLimmediatePossible() || (reg::r12 != getLimmediateReg())) return ret;

  // only read bytes up to the smallest of @max_read or @bytes.
  auto canRead = [](uint8_t n, uint8_t max_read, uint8_t bytes) -> bool {
    if (max_read)
      return n < MIN(max_read, bytes);
    else
      return n < bytes;
  };

  auto branch_size = ((ac == AllowCond::OnlyCond)
      ? Assembler::kJccLen
      : ((ac == AllowCond::OnlyUncond)
        ? Assembler::kCallLen
        : MAX(Assembler::kCallLen, Assembler::kJccLen)
        )
      );

  // Search for a register branch instruction like bctr. Return when found.
  for (ret.m_offset = 0;
      canRead(ret.m_offset, m_max_size, branch_size);
      ret.m_offset += instr_size_in_bytes) {
    // skip the preparation instructions that are not actually the branch.
    auto far_branch_instr = m_ip + ret.m_offset;
    ret.m_di = Decoder::GetDecoder().decode(far_branch_instr);
    if (ret.m_di.isRegisterBranch(ac)) return ret;
    // If branch instruction found, stop searching.
    if (ret.m_di.isRegisterBranch(AllowCond::Any)) break;
  }
  return DecInfoOffset();
}

bool DecodedInstruction::setFarBranchTarget(uint8_t* target,
                                            bool smashable) {
  DecoderInfo di = getFarBranch();
  if (di.isInvalid()) return false;

  BranchParams bp(di.ip());
  bool uncond = (bp.bo() == uint8_t(BranchParams::BO::Always));
  auto block_size = uncond ? Assembler::kCallLen : Assembler::kJccLen;

  HPHP::CodeBlock cb;
  cb.init(m_ip, block_size, "setFarBranchTarget");
  Assembler a{ cb };
  a.branchFar(target, bp, ImmType::TocOnly, smashable);

  // Check if something was overwritten
  if ((a.frontier() - m_ip) > m_size) {
    return false;
  }

  // refresh m_imm and other parameters
  decode();
  return true;
}

bool DecodedInstruction::isCall() const {
  // Near branch
  if (m_dinfo.isBranchWithLR()) return true;

  // Far branch: get first register branch instruction in the range.
  // Note: a call is always unconditional
  DecoderInfo branch_di = getFarBranch(AllowCond::OnlyUncond);

  // A Call sets the Link Register when branching
  if (branch_di.isInvalid()) return false;
  return branch_di.isBranchWithLR();
}

Reg64 DecodedInstruction::getLimmediateReg() const {
  auto di = Decoder::GetDecoder().decode(m_ip);
  if (m_dinfo.isLd(true)) {
    DS_form_t ds_instr;
    ds_instr.instruction = m_dinfo.instruction_image();
    return Reg64(ds_instr.RT);
  }
  else if (m_dinfo.isLwz(true)) {
    D_form_t d_instr;
    d_instr.instruction = m_dinfo.instruction_image();
    return Reg64(d_instr.RT);
  }
  else if (m_dinfo.isAddis(true)) {
    auto di = Decoder::GetDecoder().decode(m_ip+4);
    if (di.isLd()) {
      DS_form_t ds_instr;
      ds_instr.instruction = di.instruction_image();
      return Reg64(ds_instr.RT);
    } else if (di.isLwz()) {
      D_form_t d_instr;
      d_instr.instruction = di.instruction_image();
      return Reg64(d_instr.RT);
    }
  }
  return getLi64Reg();
}

Reg64 DecodedInstruction::getLi64Reg() const {
  // First instruction is always either li or lis, both are D-form
  assertx(isLi64Possible());
  D_form_t d_instr;
  d_instr.instruction = m_dinfo.instruction_image();
  return Reg64(d_instr.RT);
}

///////////////////////////////////////////////////////////////////////////////
// Private Interface
///////////////////////////////////////////////////////////////////////////////

void DecodedInstruction::decode() {
  m_dinfo = Decoder::GetDecoder().decode(m_ip);

  if (isLimmediatePossible() && (reg::r12 == getLimmediateReg())) {
    // Compute the whole branch on the m_size. Used on relocation to skip
    // instructions
    DecInfoOffset dio = getFarBranchLength();
    assertx(dio.m_offset > 0 && "Expected to find a Far branch");
    m_size = dio.m_offset + instr_size_in_bytes;
    decodeImm();            // sets m_imm for farBranchTarget()
  } else if (isImmediate()) {
    m_size = decodeImm();
  } else {
    m_size = instr_size_in_bytes;
  }
}

int64_t DecodedInstruction::calcIndex (int16_t indexBigTOC, 
                                       int16_t indexTOC) const {
  return static_cast<int64_t>(static_cast<int64_t>(indexTOC) +
static_cast<int64_t>(indexBigTOC << 16));
}

/*
 * Reads back the immediate when emmited with (without nops) and return the
 * number of bytes read for this immediate decoding.
 */
uint8_t DecodedInstruction::decodeImm() {
  // Functions that detect if @dinfo is exactly the instructions flavor that
  // our limmediate uses. Also the target register to be modified has to be
  // the same.

  if (m_dinfo.isLd(true)) {
    m_imm = VMTOC::getInstance().getValue(calcIndex(0, m_dinfo.offsetDS()),
      true);
    return Assembler::kLimmLen;
  } else if (m_dinfo.isLwz(true)) {
    m_imm = VMTOC::getInstance().getValue(calcIndex(0, m_dinfo.offsetD()));
    return Assembler::kLimmLen;
  } else if (m_dinfo.isAddis(true)) {
    auto bigIndexTOC = m_dinfo.offsetD();
    // Get next instruction
    auto di = Decoder::GetDecoder().decode(m_ip+4);
    if (di.isLd()) {
      m_imm = VMTOC::getInstance().getValue(calcIndex(bigIndexTOC,
          di.offsetDS()), true);
      return Assembler::kLimmLen;
    } else if (di.isLwz()) {
      m_imm = VMTOC::getInstance().getValue(calcIndex(bigIndexTOC,
          di.offsetD()));
      return Assembler::kLimmLen;
    }
  }

  auto isLi = [](DecoderInfo* dinfo, const Reg64& dest, int16_t* imm) {
    D_form_t dform;
    dform.instruction = dinfo->instruction_image();
    *imm = dform.D;
    return (OpcodeNames::op_addi == dinfo->opcode_name()) &&
      (dest == Reg64(dform.RT)) &&
      (!dform.RA);
  };
  auto isLis = [](DecoderInfo* dinfo, const Reg64& dest, int16_t* imm) {
    D_form_t dform;
    dform.instruction = dinfo->instruction_image();
    *imm = dform.D;
    return (OpcodeNames::op_addis == dinfo->opcode_name()) &&
      (dest == Reg64(dform.RT)) &&
      (!dform.RA);
  };
  auto isSldi = [](DecoderInfo* dinfo, const Reg64& dest, uint16_t* bits) {
    MD_form_t mdform;
    mdform.instruction = dinfo->instruction_image();

    // Assembling these crazy encodings
    *bits = (mdform.sh << 5) | mdform.SH;
    auto mask = ((mdform.MB & 0x1) << 5) | (mdform.MB >> 1);

    // It is only interesting when it's shifting 16bits multiples
    return (OpcodeNames::op_rldicr == dinfo->opcode_name()) &&
      (dest == Reg64(mdform.RS)) &&
      (1 == mdform.XO) &&
      ((16 == *bits) || (32 == *bits) || (48 == *bits)) &&
      ((63 - *bits) == mask);
  };
  auto isOri = [](DecoderInfo* dinfo, const Reg64& dest, uint16_t* bits) {
    D_form_t dform;
    dform.instruction = dinfo->instruction_image();
    *bits = dform.D;
    return (OpcodeNames::op_ori == dinfo->opcode_name()) &&
      (dest == Reg64(dform.RT)) &&
      (dest == Reg64(dform.RA));
  };
  auto isOris = [](DecoderInfo* dinfo, const Reg64& dest, uint16_t* bits) {
    D_form_t dform;
    dform.instruction = dinfo->instruction_image();
    *bits = dform.D;
    return (OpcodeNames::op_oris == dinfo->opcode_name()) &&
      (dest == Reg64(dform.RT)) &&
      (dest == Reg64(dform.RA));
  };

  const auto dest_reg = getLi64Reg();
  PPC64Instr* base = reinterpret_cast<PPC64Instr*>(m_ip);
  PPC64Instr* last_instr = base +
    (Assembler::kLi64Len / instr_size_in_bytes);

  // If m_max_size is 0, it can always read more, otherwise it's limited
  auto canReadMore = [&](uint8_t bytes_read) -> bool {
    if (m_max_size) return bytes_read < m_max_size;
    else            return true;
  };

  // Analyze at maximum kLi64Len instructions (and limited by m_max_size, if
  // not 0) but stop when some other instruction appears (or any that doesn't
  // have the dest_reg as a target).
  PPC64Instr* pinstr = base;
  uint8_t bytes_read = 0;
  while ((pinstr < last_instr) && canReadMore(bytes_read)) {

    auto dinfo = Decoder::GetDecoder().decode(pinstr);
    int16_t tmp_imm = 0;
    uint16_t tmp_bits = 0;

    if (isLi(&dinfo, dest_reg, &tmp_imm)) {
      m_imm = tmp_imm;
    } else if (isLis(&dinfo, dest_reg, &tmp_imm)) {
      m_imm = tmp_imm << 16;
    } else if (isSldi(&dinfo, dest_reg, &tmp_bits)) {
      m_imm = m_imm << tmp_bits;
    } else if (isOri(&dinfo, dest_reg, &tmp_bits)) {
      m_imm = m_imm | tmp_bits;
    } else if (isOris(&dinfo, dest_reg, &tmp_bits)) {
      m_imm = m_imm | (tmp_bits << 16);
    } else if (dinfo.isNop()) {
      // do nothing but let it count as a part of it.
    } else { break; }

    pinstr++;
    bytes_read += instr_size_in_bytes;
  }

  // amount of bytes read by this immediate reading
  return bytes_read;
}

uint64_t* DecodedInstruction::decodeTOCAddress() const{
 if (m_dinfo.isLd(true)) {
    return VMTOC::getInstance().getAddr(calcIndex(0, m_dinfo.offsetDS()));
  } else if (m_dinfo.isLwz(true)) {
    return VMTOC::getInstance().getAddr(calcIndex(0, m_dinfo.offsetD()));
  } else if (m_dinfo.isAddis(true)) {
    auto bigIndexTOC = m_dinfo.offsetD();
    // Get next instruction
    auto di = Decoder::GetDecoder().decode(m_ip+4);
    if (di.isLd()) {
      return VMTOC::getInstance().getAddr(calcIndex(bigIndexTOC,
          di.offsetDS()));
    } else if (di.isLwz()) {
      return VMTOC::getInstance().getAddr(calcIndex(bigIndexTOC,
          di.offsetD()));
    }
  }
  return 0;
}

int64_t DecodedInstruction::decodeTOCOffset() const {
 if (m_dinfo.isLd(true)) {
    return calcIndex(0, m_dinfo.offsetDS());
  } else if (m_dinfo.isLwz(true)) {
    return calcIndex(0, m_dinfo.offsetD());
  } else if (m_dinfo.isAddis(true)) {
    auto bigIndexTOC = m_dinfo.offsetD();
    // Get next instruction
    auto di = Decoder::GetDecoder().decode(m_ip+4);
    if (di.isLd()) {
      return calcIndex(bigIndexTOC, di.offsetDS());
    } else if (di.isLwz()) {
      return calcIndex(bigIndexTOC, di.offsetD());
    }
  }
  return 0;

}

bool DecodedInstruction::isSmashable(uint64_t elem) const {
  auto elemOffset = decodeTOCOffset();
  auto offset = VMTOC::getInstance().getIndex(elem);
  return offset!=elemOffset;
}

bool DecodedInstruction::fitsOnNearBranch(ptrdiff_t diff, bool uncond) const {
  // is it b or bc? b can use offsets up to 26bits and bc only 16bits
  auto bitsize = uncond ? 26 : 16;
  return HPHP::jit::deltaFitsBits(diff, bitsize);
}

bool DecodedInstruction::isLimmediatePossible() const {
  if (m_dinfo.isLd(true) || m_dinfo.isLwz(true) || m_dinfo.isAddis(true)) {
    return true;
  }
  return isLi64Possible();
}

bool DecodedInstruction::isLoadingTOC() const {
  if (m_dinfo.isLd(true) || m_dinfo.isLwz(true) || m_dinfo.isAddis(true)) {
    return true;
  }
  return false;
}

bool DecodedInstruction::isLi64Possible() const {
  // The beginning of a li64 starts with either li or lis.
  auto opn = m_dinfo.opcode_name();
  if ((OpcodeNames::op_addi == opn) || (OpcodeNames::op_addis == opn)) {
    D_form_t dform;
    dform.instruction = m_dinfo.instruction_image();
    if (!dform.RA) {
      // li or lis
      return true;
    }
  }
  return false;
}

}
