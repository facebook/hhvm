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

#include "hphp/ppc64-asm/decoder-ppc64.h"

#include <cassert>

#include "hphp/ppc64-asm/isa-ppc64.h"

namespace ppc64_asm {

Decoder* Decoder::s_decoder = nullptr;

std::string DecoderInfo::toString() {
  if (m_form == Form::kInvalid) return ".long " + std::to_string(m_image);
  if (isNop())                  return "nop";

  // Output string
  std::string instr = m_mnemonic + " ";

  bool hasParen = false;
  for (auto oper : m_operands) {
    auto op = (m_image & oper.m_mask) >> oper.operandShift();
    if (oper.m_flags & PPC_OPERAND_GPR)   { instr += "r"; }
    if (oper.m_flags & PPC_OPERAND_GPR_0) { if (op != 0) instr += "r"; }
    if (oper.m_flags & PPC_OPERAND_FPR)   { instr += "f"; }
    if (oper.m_flags & PPC_OPERAND_VR)    { instr += "vs"; }
    if (oper.m_flags & PPC_OPERAND_SIGNED) {
      int32_t n = static_cast<int32_t>(op);
      if (n < 0) instr += "-";
      instr += std::to_string(n);
    } else {
      instr += std::to_string(op);
    }

    // Sticky boolean: doesn't clear after set
    hasParen |= (oper.m_flags & PPC_OPERAND_PAREN);
    instr += hasParen ? "(" : ",";
  }
  // remove the "(" or "," at the end. It should not be there.
  instr.pop_back();
  // close that parentheses now.
  if (hasParen) instr += ")";
  return instr;
}

bool DecoderInfo::isNop() const {
  // no-op is a mnemonic of ori 0,0,0
  if ((m_form == Form::kD) && (m_opn == OpcodeNames::op_ori)) {
    D_form_t dform;
    dform.instruction = m_image;
    if ((!dform.D) && (!dform.RA) && (!dform.RT)) {
      // no-op
      return true;
    }
  }
  return false;
}


bool DecoderInfo::isBranch(bool allowCond /* = true */) const {
  // allowCond: true
  //   b, ba, bl - unconditional branches
  //   bc, bca, bcctr, bcctrl, bcl, bcla, bclr, bclrl, bctar, bctarl
  //
  // allowCond: false
  //   b, ba, bl - unconditional branches
  //  And also, if condition is "branch always" (BO field is 1x1xx):
  //   bc, bca, bcctr, bcctrl, bcl, bcla, bclr, bclrl, bctar, bctarl
  //
  // (based on the branch instructions defined on this Decoder)
  constexpr uint32_t uncondition_bo = 0x14;

  switch (m_opn) {
    case OpcodeNames::op_b:
    case OpcodeNames::op_ba:
    case OpcodeNames::op_bl:
      return true;
      break;
    case OpcodeNames::op_bc:
    case OpcodeNames::op_bca:
    case OpcodeNames::op_bcl:
    case OpcodeNames::op_bcla:
    case OpcodeNames::op_bclr:
      if (!allowCond) {
        // checking if the condition is "always branch", then it counts as an
        // unconditional branch
        assert(m_form == Form::kB);
        B_form_t bform;
        bform.instruction = m_image;
        return ((bform.BO & uncondition_bo) == uncondition_bo);
      }
      return true;
      break;
    case OpcodeNames::op_bcctr:
    case OpcodeNames::op_bcctrl:
    case OpcodeNames::op_bclrl:
    case OpcodeNames::op_bctar:
    case OpcodeNames::op_bctarl:
      if (!allowCond) {
        // checking if the condition is "always branch", then it counts as an
        // unconditional branch
        assert(m_form == Form::kXL);
        XL_form_t xlform;
        xlform.instruction = m_image;
        return ((xlform.BT & uncondition_bo) == uncondition_bo);
      }
      return true;
      break;
    default:
      break;
  }
  return false;
}

bool DecoderInfo::isClearSignBit() const {
  // clrldi is a mnemonic to rldicl when
  if (m_opn == OpcodeNames::op_rldicl) {
    MD_form_t instr_md;
    instr_md.instruction = m_image;
    if ((instr_md.SH == 0) && (instr_md.sh == 0)) {
      // it's the clrldi mnemonic!
      switch (instr_md.MB) {
        case 16:
        case 32:
        case 48:
          return true;
        break;
        default:
        break;
      }
    }
  }
  return false;
}

/**
 * Check if instruction is addi or add, which changes the stack pointer. These
 * instructions are created by lea, since on PPC64 there is no Lea instruction.
 */
bool DecoderInfo::isSpOffsetInstr() const {
  if (((m_form == Form::kD) && (m_opn == OpcodeNames::op_addi))
      || ((m_form == Form::kXO) && (m_opn == OpcodeNames::op_add))) {
    return true;
  }
  return false;
}

/**
 * Look for the offset from instructions like addi or add, which was created by
 * the lower of the Lea instruction.
 */
int32_t DecoderInfo::offset() const {
  always_assert(m_form == Form::kD && "Instruction not expected.");
  D_form_t instr_d;
  instr_d.instruction = m_image;
  // As the instruction is known, the immediate is a signed number of
  // 16bits, so to consider the sign, it must be casted to int16_t.
  return static_cast<int16_t>(instr_d.D);
}

///////////////////////////////////////////////////////////////////////////////

DecoderInfo* Decoder::decode(PPC64Instr instr) {
  // To decode a instruction we extract the decoder fields
  // masking the instruction and test if it 'hits' the decoder table.
  for (size_t i = 0; i < sizeof(DecoderList)/sizeof(PPC64Instr); i++) {
    auto decoded_instr = instr & DecoderList[i];
    auto index = (decoded_instr % kDecoderSize);

    while (m_decoder_table[index] != nullptr &&
        m_decoder_table[index]->opcode() != decoded_instr) {
      index = (index + 1) % kDecoderSize;
    }

    if (m_decoder_table[index] != nullptr) {
      m_decoder_table[index]->instruction_image(instr);
      return m_decoder_table[index];
    }
  }

  // invalid instruction! Use fallback.
  return getInvalid();
}

} // namespace ppc64_ams
