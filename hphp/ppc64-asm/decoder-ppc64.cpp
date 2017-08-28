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

#include "hphp/ppc64-asm/branch-ppc64.h"
#include "hphp/ppc64-asm/isa-ppc64.h"
#include "hphp/ppc64-asm/asm-ppc64.h"

#include "hphp/util/assertions.h"

namespace ppc64_asm {

Decoder* Decoder::s_decoder = nullptr;

// Operand Masks
#define PPC_OPERAND_GPR      0x1
#define PPC_OPERAND_GPR_0    0x2
#define PPC_OPERAND_SIGNED   0x4
#define PPC_OPERAND_UNSIGNED 0x8
#define PPC_OPERAND_PAREN    0x10
#define PPC_OPERAND_RELATIVE 0x20
#define PPC_OPERAND_ABSOLUTE 0x40
#define PPC_OPERAND_OPTIONAL 0x80
#define PPC_OPERAND_FPR      0x100
#define PPC_OPERAND_VR       0x200
#define PPC_OPERAND_VSX      0x400
#define PPC_OPERAND_SIGNOPT  0x800
#define PPC_OPERAND_CR       0x1000
#define PPC_OPERAND_NEXT     0x2000
#define PPC_OPERAND_PLUS1    0x4000
#define PPC_OPERAND_NOSHIFT  0x8000

#define A         { 0x2000000, 0x0 }
#define A_L       { 0x20, 0x0 }
#define BA        { 0x001f0000, 0x0 }
#define BB        { 0x0000f800, 0x0 }
#define BD        { 0x0000fffc, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED | \
                                PPC_OPERAND_NOSHIFT }
#define BDA       { 0xfffc, PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED }
#define BF        BDA
#define BFA       { 0x1c0000, PPC_OPERAND_CR }
#define BFF       { 0x3800000, 0x0 }
#define BH        { 0x00001800, 0x0 }
#define BHRBE     { 0x1ff800, 0x0 }
#define BI        { 0x001f0000, 0x0 }
#define BO        { 0x03e00000, 0x0 }
#define BT        { 0x03e00000, 0x0 }
#define CRFD      { 0x3800000, PPC_OPERAND_CR }
#define CRB       { 0x1c0000, PPC_OPERAND_CR | PPC_OPERAND_OPTIONAL }
#define CT        { 0x7, PPC_OPERAND_CR }
#define D         { 0xffff, PPC_OPERAND_PAREN | PPC_OPERAND_SIGNED }
#define DCM       { 0xfc00, 0x0 }
#define DGM       DCM
#define DM        { 0xc0, 0x0 }
#define DQ        { 0xfff0, 0x0 }
#define DS        { 0xfffc, PPC_OPERAND_PAREN | PPC_OPERAND_SIGNED | \
                            PPC_OPERAND_NOSHIFT}
#define DUI       { 0x3e00000, 0x0 }
#define DUIS      { 0x1ff800, PPC_OPERAND_SIGNED }
#define E         { 0x8000, 0x0 }
#define EH        { 0x1, PPC_OPERAND_OPTIONAL }
#define EVUIMM    { 0xf800, 0x0 }
#define EVUIMM_2  { 0xf800, PPC_OPERAND_PAREN }
#define EVUIMM_4  EVUIMM_2
#define EVUIMM_8  EVUIMM_4
#define FLM       { 0x1fe0000, 0x0 }
#define FRA       { 0x1f0000, PPC_OPERAND_FPR }
#define FRB       { 0xf800, PPC_OPERAND_FPR }
#define FRC       { 0x7c0, PPC_OPERAND_FPR }
#define FRS       { 0x3e00000, PPC_OPERAND_FPR }
#define FRT       FRS
#define FXM       { 0xff000, 0x0 }
#define L         { 0x2000000, PPC_OPERAND_OPTIONAL }
#define LEV       { 0xfe0, PPC_OPERAND_OPTIONAL }
#define LI        { 0x3fffffc, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED | \
                               PPC_OPERAND_NOSHIFT }
#define LIA       { 0x3fffffc, PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED | \
                               PPC_OPERAND_NOSHIFT }
#define LS        { 0x600000, PPC_OPERAND_OPTIONAL }
#define MB6       { 0x7e0, 0x0 }
#define ME6       MB6
#define MBE       { 0x7c0, PPC_OPERAND_OPTIONAL | PPC_OPERAND_NEXT }
#define ME        { 0x3e, 0x0 }
#define MO        { 0x3e00000, PPC_OPERAND_OPTIONAL }
#define NB        { 0xf800, PPC_OPERAND_PLUS1 }
#define OC        { 0x3fff800, 0x0 }
#define PMR       { 0x1ff800, PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT }
#define PS        { 0x200, 0x0 }
#define R         { 0x10000, 0x0 }
#define RA        { 0x1f0000, PPC_OPERAND_GPR }
// 0 in the RA field means zero, not r0.
#define RA0       { 0x1f0000, PPC_OPERAND_GPR_0 }
#define RAL       RA0
#define RAM       RA0
#define RAOPT     { 0x1f0000, PPC_OPERAND_GPR | PPC_OPERAND_OPTIONAL}
#define RAQ       RA0
#define RAS       RA0
#define RB        { 0xf800, PPC_OPERAND_GPR }
#define RMC       { 0x600, 0x0 }
#define RS        { 0x3e00000, PPC_OPERAND_GPR }
#define RSO       { 0x3e00000, PPC_OPERAND_GPR | PPC_OPERAND_OPTIONAL }
#define RSQ       { 0x3c00000, PPC_OPERAND_GPR_0 }
#define RT        RS
#define RTO       RSO
#define RTQ       RSQ
#define S         { 0x100000, 0x0 }
#define SH        { 0xf000, 0x0 }
#define SH6       { (0xf000 | 0x2), 0x0 }
#define SH16      { 0xf800, 0x0 }
#define SHB       { 0x3c0, 0x0 }
#define SHO       { 0xf800, PPC_OPERAND_OPTIONAL }
#define SHW       { 0xc0, 0x0 }
#define SI        { 0xffff, PPC_OPERAND_SIGNED }
#define SIMM      { 0x1f0000, 0x0 }
#define SISIGNOPT { 0xffff, PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT }
#define SIX       { 0x3c00000, 0x0 }
#define SP        { 0x180000, 0x0 }
#define SPR       { 0x1ff800, 0x0 }
#define SR        { 0xf0000, 0x0 }
#define ST        { 0x600000, 0x0 }
#define TBR       { 0x1ff800, PPC_OPERAND_OPTIONAL }
#define TE        { 0x1f0000, 0x0 }
#define TH        { 0x3e00000, 0x0 }
#define TO        TH
#define U         { 0xf000, 0x0 }
#define UI        { 0xffff, PPC_OPERAND_UNSIGNED }
#define UIMM      { 0x1f0000, PPC_OPERAND_SIGNED }
#define UN        { 0x0, 0x0 }
#define VA        { 0x1f0000, PPC_OPERAND_VR }
#define VB        { 0xf800, PPC_OPERAND_VR }
#define VC        { 0x7c0, PPC_OPERAND_VR }
#define VD        { 0x3e00000, PPC_OPERAND_VR }
#define VS        VD
#define W         { 0x10000, PPC_OPERAND_OPTIONAL }
#define WC        { 0x600000, 0x0 }
#define XA        { 0x1f0000, PPC_OPERAND_VSX }
#define XB        { 0xf800, PPC_OPERAND_VSX }
#define XC        { 0xf800, PPC_OPERAND_VSX }
#define XFL_L     { 0x2000000, PPC_OPERAND_OPTIONAL }
#define XS        { 0x3e00000, PPC_OPERAND_VSX }
#define XT        XS

Decoder::Decoder() {
  m_decoder_table = new DecoderInfo*[kDecoderSize];
  m_decoder_table_size = 0;
  for (int i = 0; i < kDecoderSize; i++) {
    m_decoder_table[i] = nullptr;
  }

  m_opcode_index_table = new DecoderInfo*[kTotalOpcodes];

#define DE(name, op, type, mnemonic, ... )      \
  DecoderInfo instr_##name {                    \
    OpcodeNames::op_##name,                     \
      op,                                       \
      type,                                     \
      #mnemonic,                                \
      __VA_ARGS__ };                            \
  setInstruction(instr_##name);

  PPC64_OPCODES

  // Create index based on instruction opcode
  createOpcodeIndex();

#undef DE
}

#undef A
#undef A_L
#undef BA
#undef BB
#undef BD
#undef BDA
#undef BF
#undef BFA
#undef BFF
#undef BH
#undef BHRBE
#undef BI
#undef BO
#undef BT
#undef CRFD
#undef CRB
#undef CT
#undef D
#undef DCM
#undef DGM
#undef DM
#undef DQ
#undef DS
#undef DUI
#undef DUIS
#undef E
#undef EH
#undef EVUIMM
#undef EVUIMM_2
#undef EVUIMM_4
#undef EVUIMM_8
#undef FLM
#undef FRA
#undef FRB
#undef FRC
#undef FRS
#undef FRT
#undef FXM
#undef L
#undef LEV
#undef LI
#undef LIA
#undef LS
#undef MB6
#undef ME6
#undef MBE
#undef ME
#undef MO
#undef NB
#undef OC
#undef PMR
#undef PS
#undef R
#undef RA
#undef RA0
#undef RAL
#undef RAM
#undef RAOPT
#undef RAQ
#undef RAS
#undef RB
#undef RMC
#undef RS
#undef RSO
#undef RSQ
#undef RT
#undef RTO
#undef RTQ
#undef S
#undef SH
#undef SH16
#undef SH6
#undef SHB
#undef SHO
#undef SHW
#undef SI
#undef SIMM
#undef SISIGNOPT
#undef SIX
#undef SP
#undef SPR
#undef SR
#undef ST
#undef TBR
#undef TE
#undef TH
#undef TO
#undef U
#undef UI
#undef UIMM
#undef UN
#undef VA
#undef VB
#undef VC
#undef VD
#undef VS
#undef W
#undef WC
#undef XA
#undef XB
#undef XC
#undef XFL_L
#undef XFL_L
#undef XS
#undef XT

std::string DecoderInfo::toString() const {
  if (m_form == Form::kInvalid) return ".long " + std::to_string(m_image);
  if (isNop())                  return "nop";

  // Output string
  std::string instr = m_mnemonic + " ";

  bool hasParen = false;
  for (auto oper : m_operands) {
    auto op = m_image & oper.m_mask;
    if (!(oper.m_flags & PPC_OPERAND_NOSHIFT)) op >>= oper.operandShift();
    auto toHex = [] (std::string& instruction, intptr_t n) {
      std::stringstream stringStream;
      if (n < 0) {
          stringStream << "-0x";
          n = -n;
      } else stringStream << "0x";
      stringStream << std::hex << n;
      instruction += stringStream.str();
    };
    if (oper.m_flags & PPC_OPERAND_GPR)   { instr += "r"; }
    if (oper.m_flags & PPC_OPERAND_GPR_0) { if (op != 0) instr += "r"; }
    if (oper.m_flags & PPC_OPERAND_FPR)   { instr += "f"; }
    if (oper.m_flags & PPC_OPERAND_VR)    { instr += "vs"; }
    if (oper.m_flags & PPC_OPERAND_RELATIVE) {
      // print branch target instead of the relative offset
      int32_t n = static_cast<int32_t>(op << 6) >> 6; // extend sign
      toHex(instr, reinterpret_cast<intptr_t>(m_ip) + n);
    } else if (oper.m_flags & PPC_OPERAND_SIGNED) {
      int16_t n = static_cast<int16_t>(op);
      toHex(instr, n);
    } else if (oper.m_flags & PPC_OPERAND_UNSIGNED) {
      toHex(instr, op);
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

bool DecoderInfo::isException() const {
  // trap is a mnemonic of tw 31,0,0
  if ((m_form == Form::kX) && (m_opn == OpcodeNames::op_tw)) {
    X_form_t xform;
    xform.instruction = m_image;
    if ((31 == xform.RT) && (!xform.RA) && (!xform.RB) && (4 == xform.XO)) {
      // trap
      return true;
    }
  }
  return false;
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

bool DecoderInfo::isLd(bool toc) const {
  if ((m_form == Form::kDS) && (m_opn == OpcodeNames::op_ld)) {
    if (!toc) return true;

    DS_form_t dsform;
    dsform.instruction = m_image;
    if (Reg64(dsform.RA) == reg::r2) {
      return true;
    }
  }
  return false;
}

/*
 * Auxiliary for isLwz and isAddis
 */
bool DecoderInfo::isDformOp(OpcodeNames opn, bool toc) const {
  if ((m_form == Form::kD) && (m_opn == opn)) {
    if (!toc) return true;

    D_form_t dform;
    dform.instruction = m_image;
    if (Reg64(dform.RA) == reg::r2) {
      return true;
    }
  }
  return false;
}

bool DecoderInfo::isLwz(bool toc) const {
  return isDformOp(OpcodeNames::op_lwz, toc);
}

bool DecoderInfo::isAddis(bool toc) const {
  return isDformOp(OpcodeNames::op_addis, toc);
}

bool DecoderInfo::isOffsetBranch(AllowCond ac /* = AllowCond::Any */) const {
  // ac: OnlyUncond
  //   b, bl   - unconditional branches
  //  And also, if condition is "branch always" (BO field is 1x1xx):
  //   bc, bcl
  //
  // ac: Any
  //   b, bl   - unconditional branches
  //   bc, bcl - conditional branches
  //
  // ac: OnlyCond
  //   bc, bcl - conditional branches and it can't be "branch always"
  //
  // (based on the branch instructions defined on this Decoder)
  switch (m_opn) {
    case OpcodeNames::op_b:
    case OpcodeNames::op_bl:
      return (AllowCond::OnlyCond != ac);
      break;
    case OpcodeNames::op_bc:
    case OpcodeNames::op_bcl:
      {
        // checking if the condition is "always branch", then it counts as an
        // unconditional branch
        assert(m_form == Form::kB);
        B_form_t bform;
        bform.instruction = m_image;
        BranchParams uncondition_bp(BranchConditions::Always);

        if (uncondition_bp.bo() == bform.BO) {
          // not acceptable if it was required to have a condition
          return (AllowCond::OnlyCond != ac);
        } else {
          // it's a conditional branch
          return (AllowCond::OnlyUncond != ac);
        }
        break;
      }
    default:
      break;
  }
  return false;
}

/*
 * True if bcctrl (as "branch always" condition) or bl
 * False otherwise
 */
bool DecoderInfo::isBranchWithLR() const {
  switch (m_opn) {
    case OpcodeNames::op_bcctrl:
      {
        // checking if the condition is "always branch", then it counts as an
        // unconditional branch
        assert(m_form == Form::kXL);
        XL_form_t xlform;
        xlform.instruction = m_image;
        BranchParams uncondition_bp(BranchConditions::Always);
        return xlform.BT == uncondition_bp.bo();
      }
      break;
    case OpcodeNames::op_bl:
      return true;
      break;
    default:
      break;
  }
  return false;
}

bool DecoderInfo::isRegisterBranch(AllowCond ac /* = AllowCond::Any */) const {
  // ac: Any
  //   bcctr, bcctrl
  //
  // ac: OnlyUncond
  //   bcctr, bcctrl - only if condition is "branch always" (BO field: 1x1xx)
  //
  // ac: OnlyCond
  //   bcctr, bcctrl - only if condition is NOT "branch always" (BO field)
  //
  // NOTE: not considering bclr as it's more like a return than a branch
  switch (m_opn) {
    case OpcodeNames::op_bcctr:
    case OpcodeNames::op_bcctrl:
      {
        // checking if the condition is "always branch", then it counts as an
        // unconditional branch
        assert(m_form == Form::kXL);
        XL_form_t xlform;
        xlform.instruction = m_image;
        BranchParams uncondition_bp(BranchConditions::Always);
        if (uncondition_bp.bo() == xlform.BT) {
          // unconditional is only allowed if it's not OnlyCond
          return (AllowCond::OnlyCond != ac);
        } else {
          // it's a conditional branch
          return (AllowCond::OnlyUncond != ac);
        }
      }
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

/*
 * Return offset of a branch by offset like b or bc.
 */
int32_t DecoderInfo::branchOffset() const {
  int32_t offset = 0;
  switch (m_opn) {
    case OpcodeNames::op_b:
    case OpcodeNames::op_bl:
      {
        I_form_t iform;
        iform.instruction = m_image;
        // sign bit of LI
        auto signBit = 1 << 25;
        auto li = iform.LI << 2;
        // sign-extend 32-bits from a 26-bit value, if negative
        offset = (signBit & li) ? ((int32_t(-1) - ((1<<26)-1)) | li) : li;
        break;
      }
    case OpcodeNames::op_bc:
    case OpcodeNames::op_bcl:
      {
        B_form_t bform;
        bform.instruction = m_image;
        offset = int16_t(bform.BD << 2);
        break;
      }
    default:
      always_assert(false && "Instruction not expected.");
      break;
  }
  return offset;
}

/*
 * Set offset of a branch by offset like b or bc. Return that instruction
 */
PPC64Instr DecoderInfo::setBranchOffset(int32_t offset) const {
  switch (m_opn) {
    case OpcodeNames::op_b:
    case OpcodeNames::op_bl:
      {
        I_form_t iform;
        iform.instruction = m_image;
        iform.LI = offset >> 2;
        return iform.instruction;
        break;
      }
    case OpcodeNames::op_bc:
    case OpcodeNames::op_bcl:
      {
        B_form_t bform;
        bform.instruction = m_image;
        bform.BD = offset >> 2;
        return bform.instruction;
        break;
      }
    default:
      always_assert(false && "Instruction not expected.");
      return 0;
      break;
  }
}
/*
 * Find the offset from instructions like ld, which was created by
 * limmediate.
 */
int16_t DecoderInfo::offsetDS() const {
  always_assert(m_form == Form::kDS && "Instruction not expected.");
  DS_form_t instr_d;
  instr_d.instruction = m_image;

  return static_cast<int16_t>(instr_d.DS << 2);
}

/*
 * Find the offset from instructions like lwz.
 */
int16_t DecoderInfo::offsetD() const {
  always_assert(m_form == Form::kD && "Instruction not expected.");
  D_form_t instr_d;
  instr_d.instruction = m_image;

  return static_cast<int16_t>(instr_d.D);
}

///////////////////////////////////////////////////////////////////////////////

const DecoderInfo Decoder::decode(const PPC64Instr* const ip) {
  int32_t position;
  PPC64Instr decoded_instr, operand, opcode_index, opcode_size;
  PPC64Instr opcode = *ip & kOpcodeMask;

  // Get index and size for the opcode.
  opcode_index = (opcode_index_map.find(opcode))->second;
  opcode_size = (opcode_size_map.find(opcode))->second;

  // To decode a instruction we extract the decoder fields
  // masking the instruction and test if it 'hits' the decoder table.
  for (size_t i = 0; i < sizeof(DecoderList)/sizeof(PPC64Instr); i++) {
    decoded_instr = *ip & DecoderList[i];
    operand = decoded_instr & kOperandMask;

    // Search the instruction for the current mask
    position = searchInstr(opcode_index, opcode_size, operand);

    // If instruction found, return it.
    if (position != -1) {
      DecoderInfo pdi = *m_decoder_table[position];
      assert(pdi.opcode() == decoded_instr);
      pdi.instruction_image(*ip);
      pdi.setIp(ip);
      return pdi;
    }
  }

  // invalid instruction! Use fallback.
  DecoderInfo ret = getInvalid();
  ret.setIp(ip);
  return ret;
}

/*
 * Binary search when looking for the operand
 */
int32_t Decoder::searchInstr(int32_t opd_index,
                             int32_t opc_size,
                             PPC64Instr search) const {
  PPC64Instr operand;
  auto first = opd_index;
  auto last = opd_index + opc_size -1;
  auto middle = (first + last)/2;

  while (first <= last) {
    operand = m_decoder_table[middle]->opcode() & kOperandMask;
    if (operand < search) {
      first = middle + 1;
    }
    else if (operand == search) {
      return middle;
    }
    else {
      last = middle -1;
    }
    middle = (first + last)/2;
  }

  return -1;
}

} // namespace ppc64_ams
