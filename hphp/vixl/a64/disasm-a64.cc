// Copyright 2013, ARM Limited
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/util/text-color.h"

#include <folly/Format.h>

namespace vixl {

Disassembler::Disassembler() {
  buffer_size_ = 256;
  buffer_ = reinterpret_cast<char*>(malloc(buffer_size_));
  buffer_pos_ = 0;
  own_buffer_ = true;
  code_address_offset_ = 0;
}


Disassembler::Disassembler(char* text_buffer, int buffer_size) {
  buffer_size_ = buffer_size;
  buffer_ = text_buffer;
  buffer_pos_ = 0;
  own_buffer_ = false;
  code_address_offset_ = 0;
}


Disassembler::~Disassembler() {
  if (own_buffer_) {
    free(buffer_);
  }
}


char* Disassembler::GetOutput() {
  return buffer_;
}


void Disassembler::VisitAddSubImmediate(Instruction* instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool stack_op = (rd_is_zr || RnIsZROrSP(instr)) &&
                  (instr->ImmAddSub() == 0) ? true : false;
  const char *mnemonic = "";
  const char *form = "'Rds, 'Rns, 'IAddSub";
  const char *form_cmp = "'Rns, 'IAddSub";
  const char *form_mov = "'Rds, 'Rns";

  switch (instr->Mask(AddSubImmediateMask)) {
    case ADD_w_imm:
    case ADD_x_imm: {
      mnemonic = "add";
      if (stack_op) {
        mnemonic = "mov";
        form = form_mov;
      }
      break;
    }
    case ADDS_w_imm:
    case ADDS_x_imm: {
      mnemonic = "adds";
      if (rd_is_zr) {
        mnemonic = "cmn";
        form = form_cmp;
      }
      break;
    }
    case SUB_w_imm:
    case SUB_x_imm: mnemonic = "sub"; break;
    case SUBS_w_imm:
    case SUBS_x_imm: {
      mnemonic = "subs";
      if (rd_is_zr) {
        mnemonic = "cmp";
        form = form_cmp;
      }
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitAddSubShifted(Instruction* instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm'HDP";
  const char *form_cmp = "'Rn, 'Rm'HDP";
  const char *form_neg = "'Rd, 'Rm'HDP";

  switch (instr->Mask(AddSubShiftedMask)) {
    case ADD_w_shift:
    case ADD_x_shift: mnemonic = "add"; break;
    case ADDS_w_shift:
    case ADDS_x_shift: {
      mnemonic = "adds";
      if (rd_is_zr) {
        mnemonic = "cmn";
        form = form_cmp;
      }
      break;
    }
    case SUB_w_shift:
    case SUB_x_shift: {
      mnemonic = "sub";
      if (rn_is_zr) {
        mnemonic = "neg";
        form = form_neg;
      }
      break;
    }
    case SUBS_w_shift:
    case SUBS_x_shift: {
      mnemonic = "subs";
      if (rd_is_zr) {
        mnemonic = "cmp";
        form = form_cmp;
      } else if (rn_is_zr) {
        mnemonic = "negs";
        form = form_neg;
      }
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitAddSubExtended(Instruction* instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  const char *mnemonic = "";
  Extend mode = static_cast<Extend>(instr->ExtendMode());
  const char *form = ((mode == UXTX) || (mode == SXTX)) ?
                     "'Rds, 'Rns, 'Xm'Ext" : "'Rds, 'Rns, 'Wm'Ext";
  const char *form_cmp = ((mode == UXTX) || (mode == SXTX)) ?
                         "'Rns, 'Xm'Ext" : "'Rns, 'Wm'Ext";

  switch (instr->Mask(AddSubExtendedMask)) {
    case ADD_w_ext:
    case ADD_x_ext: mnemonic = "add"; break;
    case ADDS_w_ext:
    case ADDS_x_ext: {
      mnemonic = "adds";
      if (rd_is_zr) {
        mnemonic = "cmn";
        form = form_cmp;
      }
      break;
    }
    case SUB_w_ext:
    case SUB_x_ext: mnemonic = "sub"; break;
    case SUBS_w_ext:
    case SUBS_x_ext: {
      mnemonic = "subs";
      if (rd_is_zr) {
        mnemonic = "cmp";
        form = form_cmp;
      }
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitAddSubWithCarry(Instruction* instr) {
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm";
  const char *form_neg = "'Rd, 'Rm";

  switch (instr->Mask(AddSubWithCarryMask)) {
    case ADC_w:
    case ADC_x: mnemonic = "adc"; break;
    case ADCS_w:
    case ADCS_x: mnemonic = "adcs"; break;
    case SBC_w:
    case SBC_x: {
      mnemonic = "sbc";
      if (rn_is_zr) {
        mnemonic = "ngc";
        form = form_neg;
      }
      break;
    }
    case SBCS_w:
    case SBCS_x: {
      mnemonic = "sbcs";
      if (rn_is_zr) {
        mnemonic = "ngcs";
        form = form_neg;
      }
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLogicalImmediate(Instruction* instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = "";
  const char *form = "'Rds, 'Rn, 'ITri";

  if (instr->ImmLogical() == 0) {
    // The immediate encoded in the instruction is not in the expected format.
    Format(instr, "unallocated", "(LogicalImmediate)");
    return;
  }

  switch (instr->Mask(LogicalImmediateMask)) {
    case AND_w_imm:
    case AND_x_imm: mnemonic = "and"; break;
    case ORR_w_imm:
    case ORR_x_imm: {
      mnemonic = "orr";
      unsigned reg_size = (instr->SixtyFourBits() == 1) ? kXRegSize
                                                        : kWRegSize;
      if (rn_is_zr && !IsMovzMovnImm(reg_size, instr->ImmLogical())) {
        mnemonic = "mov";
        form = "'Rds, 'ITri";
      }
      break;
    }
    case EOR_w_imm:
    case EOR_x_imm: mnemonic = "eor"; break;
    case ANDS_w_imm:
    case ANDS_x_imm: {
      mnemonic = "ands";
      if (rd_is_zr) {
        mnemonic = "tst";
        form = "'Rn, 'ITri";
      }
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


bool Disassembler::IsMovzMovnImm(unsigned reg_size, uint64_t value) {
  assert((reg_size == kXRegSize) ||
         ((reg_size == kWRegSize) && (value <= 0xffffffff)));

  // Test for movz: 16 bits set at positions 0, 16, 32 or 48.
  if (((value & 0xffffffffffff0000UL) == 0UL) ||
      ((value & 0xffffffff0000ffffUL) == 0UL) ||
      ((value & 0xffff0000ffffffffUL) == 0UL) ||
      ((value & 0x0000ffffffffffffUL) == 0UL)) {
    return true;
  }

  // Test for movn: NOT(16 bits set at positions 0, 16, 32 or 48).
  if ((reg_size == kXRegSize) &&
      (((value & 0xffffffffffff0000UL) == 0xffffffffffff0000UL) ||
       ((value & 0xffffffff0000ffffUL) == 0xffffffff0000ffffUL) ||
       ((value & 0xffff0000ffffffffUL) == 0xffff0000ffffffffUL) ||
       ((value & 0x0000ffffffffffffUL) == 0x0000ffffffffffffUL))) {
    return true;
  }
  if ((reg_size == kWRegSize) &&
      (((value & 0xffff0000) == 0xffff0000) ||
       ((value & 0x0000ffff) == 0x0000ffff))) {
    return true;
  }
  return false;
}


void Disassembler::VisitLogicalShifted(Instruction* instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm'HLo";

  switch (instr->Mask(LogicalShiftedMask)) {
    case AND_w:
    case AND_x: mnemonic = "and"; break;
    case BIC_w:
    case BIC_x: mnemonic = "bic"; break;
    case EOR_w:
    case EOR_x: mnemonic = "eor"; break;
    case EON_w:
    case EON_x: mnemonic = "eon"; break;
    case BICS_w:
    case BICS_x: mnemonic = "bics"; break;
    case ANDS_w:
    case ANDS_x: {
      mnemonic = "ands";
      if (rd_is_zr) {
        mnemonic = "tst";
        form = "'Rn, 'Rm'HLo";
      }
      break;
    }
    case ORR_w:
    case ORR_x: {
      mnemonic = "orr";
      if (rn_is_zr && (instr->ImmDPShift() == 0) && (instr->ShiftDP() == LSL)) {
        mnemonic = "mov";
        form = "'Rd, 'Rm";
      }
      break;
    }
    case ORN_w:
    case ORN_x: {
      mnemonic = "orn";
      if (rn_is_zr) {
        mnemonic = "mvn";
        form = "'Rd, 'Rm'HLo";
      }
      break;
    }
    default: not_reached();
  }

  Format(instr, mnemonic, form);
}


void Disassembler::VisitConditionalCompareRegister(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Rn, 'Rm, 'INzcv, 'Cond";

  switch (instr->Mask(ConditionalCompareRegisterMask)) {
    case CCMN_w:
    case CCMN_x: mnemonic = "ccmn"; break;
    case CCMP_w:
    case CCMP_x: mnemonic = "ccmp"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitConditionalCompareImmediate(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Rn, 'IP, 'INzcv, 'Cond";

  switch (instr->Mask(ConditionalCompareImmediateMask)) {
    case CCMN_w_imm:
    case CCMN_x_imm: mnemonic = "ccmn"; break;
    case CCMP_w_imm:
    case CCMP_x_imm: mnemonic = "ccmp"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitConditionalSelect(Instruction* instr) {
  bool rnm_is_zr = (RnIsZROrSP(instr) && RmIsZROrSP(instr));
  bool rn_is_rm = (instr->Rn() == instr->Rm());
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm, 'Cond";
  const char *form_test = "'Rd, 'CInv";
  const char *form_update = "'Rd, 'Rn, 'CInv";

  Condition cond = static_cast<Condition>(instr->Condition());
  bool invertible_cond = (cond != al) && (cond != nv);

  switch (instr->Mask(ConditionalSelectMask)) {
    case CSEL_w:
    case CSEL_x: mnemonic = "csel"; break;
    case CSINC_w:
    case CSINC_x: {
      mnemonic = "csinc";
      if (rnm_is_zr && invertible_cond) {
        mnemonic = "cset";
        form = form_test;
      } else if (rn_is_rm && invertible_cond) {
        mnemonic = "cinc";
        form = form_update;
      }
      break;
    }
    case CSINV_w:
    case CSINV_x: {
      mnemonic = "csinv";
      if (rnm_is_zr && invertible_cond) {
        mnemonic = "csetm";
        form = form_test;
      } else if (rn_is_rm && invertible_cond) {
        mnemonic = "cinv";
        form = form_update;
      }
      break;
    }
    case CSNEG_w:
    case CSNEG_x: {
      mnemonic = "csneg";
      if (rn_is_rm && invertible_cond) {
        mnemonic = "cneg";
        form = form_update;
      }
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitBitfield(Instruction* instr) {
  unsigned s = instr->ImmS();
  unsigned r = instr->ImmR();
  unsigned rd_size_minus_1 =
    ((instr->SixtyFourBits() == 1) ? kXRegSize : kWRegSize) - 1;
  const char *mnemonic = "";
  const char *form = "";
  const char *form_shift_right = "'Rd, 'Rn, 'IBr";
  const char *form_extend = "'Rd, 'Wn";
  const char *form_bfiz = "'Rd, 'Rn, 'IBZ-r, 'IBs+1";
  const char *form_bfx = "'Rd, 'Rn, 'IBr, 'IBs-r+1";
  const char *form_lsl = "'Rd, 'Rn, 'IBZ-r";

  switch (instr->Mask(BitfieldMask)) {
    case SBFM_w:
    case SBFM_x: {
      mnemonic = "sbfx";
      form = form_bfx;
      if (r == 0) {
        form = form_extend;
        if (s == 7) {
          mnemonic = "sxtb";
        } else if (s == 15) {
          mnemonic = "sxth";
        } else if ((s == 31) && (instr->SixtyFourBits() == 1)) {
          mnemonic = "sxtw";
        } else {
          form = form_bfx;
        }
      } else if (s == rd_size_minus_1) {
        mnemonic = "asr";
        form = form_shift_right;
      } else if (s < r) {
        mnemonic = "sbfiz";
        form = form_bfiz;
      }
      break;
    }
    case UBFM_w:
    case UBFM_x: {
      mnemonic = "ubfx";
      form = form_bfx;
      if (r == 0) {
        form = form_extend;
        if (s == 7) {
          mnemonic = "uxtb";
        } else if (s == 15) {
          mnemonic = "uxth";
        } else {
          form = form_bfx;
        }
      }
      if (s == rd_size_minus_1) {
        mnemonic = "lsr";
        form = form_shift_right;
      } else if (r == s + 1) {
        mnemonic = "lsl";
        form = form_lsl;
      } else if (s < r) {
        mnemonic = "ubfiz";
        form = form_bfiz;
      }
      break;
    }
    case BFM_w:
    case BFM_x: {
      mnemonic = "bfxil";
      form = form_bfx;
      if (s < r) {
        mnemonic = "bfi";
        form = form_bfiz;
      }
    }
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitExtract(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm, 'IExtract";

  switch (instr->Mask(ExtractMask)) {
    case EXTR_w:
    case EXTR_x: {
      if (instr->Rn() == instr->Rm()) {
        mnemonic = "ror";
        form = "'Rd, 'Rn, 'IExtract";
      } else {
        mnemonic = "extr";
      }
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitPCRelAddressing(Instruction* instr) {
  switch (instr->Mask(PCRelAddressingMask)) {
    case ADR: Format(instr, "adr", "'Xd, 'AddrPCRelByte"); break;
    // ADRP is not implemented.
    default: Format(instr, "unimplemented", "(PCRelAddressing)");
  }
}


void Disassembler::VisitConditionalBranch(Instruction* instr) {
  switch (instr->Mask(ConditionalBranchMask)) {
    case B_cond: Format(instr, "b.'CBrn", "'BImmCond"); break;
    default: not_reached();
  }
}


void Disassembler::VisitUnconditionalBranchToRegister(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'Xn";

  switch (instr->Mask(UnconditionalBranchToRegisterMask)) {
    case BR: mnemonic = "br"; break;
    case BLR: mnemonic = "blr"; break;
    case RET: {
      mnemonic = "ret";
      if (instr->Rn() == kLinkRegCode) {
        form = nullptr;
      }
      break;
    }
    default: form = "(UnconditionalBranchToRegister)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitUnconditionalBranch(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'BImmUncn";

  switch (instr->Mask(UnconditionalBranchMask)) {
    case B: mnemonic = "b"; break;
    case BL: mnemonic = "bl"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitDataProcessing1Source(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn";

  switch (instr->Mask(DataProcessing1SourceMask)) {
    #define FORMAT(A, B)  \
    case A##_w:           \
    case A##_x: mnemonic = B; break;
    FORMAT(RBIT, "rbit");
    FORMAT(REV16, "rev16");
    FORMAT(REV, "rev");
    FORMAT(CLZ, "clz");
    FORMAT(CLS, "cls");
    #undef FORMAT
    case REV32_x: mnemonic = "rev32"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitDataProcessing2Source(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'Rd, 'Rn, 'Rm";

  switch (instr->Mask(DataProcessing2SourceMask)) {
    #define FORMAT(A, B)  \
    case A##_w:           \
    case A##_x: mnemonic = B; break;
    FORMAT(UDIV, "udiv");
    FORMAT(SDIV, "sdiv");
    FORMAT(LSLV, "lsl");
    FORMAT(LSRV, "lsr");
    FORMAT(ASRV, "asr");
    FORMAT(RORV, "ror");
    #undef FORMAT
    default: form = "(DataProcessing2Source)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitDataProcessing3Source(Instruction* instr) {
  bool ra_is_zr = RaIsZROrSP(instr);
  const char *mnemonic = "";
  const char *form = "'Xd, 'Wn, 'Wm, 'Xa";
  const char *form_rrr = "'Rd, 'Rn, 'Rm";
  const char *form_rrrr = "'Rd, 'Rn, 'Rm, 'Ra";
  const char *form_xww = "'Xd, 'Wn, 'Wm";
  const char *form_xxx = "'Xd, 'Xn, 'Xm";

  switch (instr->Mask(DataProcessing3SourceMask)) {
    case MADD_w:
    case MADD_x: {
      mnemonic = "madd";
      form = form_rrrr;
      if (ra_is_zr) {
        mnemonic = "mul";
        form = form_rrr;
      }
      break;
    }
    case MSUB_w:
    case MSUB_x: {
      mnemonic = "msub";
      form = form_rrrr;
      if (ra_is_zr) {
        mnemonic = "mneg";
        form = form_rrr;
      }
      break;
    }
    case SMADDL_x: {
      mnemonic = "smaddl";
      if (ra_is_zr) {
        mnemonic = "smull";
        form = form_xww;
      }
      break;
    }
    case SMSUBL_x: {
      mnemonic = "smsubl";
      if (ra_is_zr) {
        mnemonic = "smnegl";
        form = form_xww;
      }
      break;
    }
    case UMADDL_x: {
      mnemonic = "umaddl";
      if (ra_is_zr) {
        mnemonic = "umull";
        form = form_xww;
      }
      break;
    }
    case UMSUBL_x: {
      mnemonic = "umsubl";
      if (ra_is_zr) {
        mnemonic = "umnegl";
        form = form_xww;
      }
      break;
    }
    case SMULH_x: {
      mnemonic = "smulh";
      form = form_xxx;
      break;
    }
    case UMULH_x: {
      mnemonic = "umulh";
      form = form_xxx;
      break;
    }
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitCompareBranch(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Rt, 'BImmCmpa";

  switch (instr->Mask(CompareBranchMask)) {
    case CBZ_w:
    case CBZ_x: mnemonic = "cbz"; break;
    case CBNZ_w:
    case CBNZ_x: mnemonic = "cbnz"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitTestBranch(Instruction* instr) {
  const char *mnemonic = "";
  // If the top bit of the immediate is clear, the tested register is
  // disassembled as Wt, otherwise Xt. As the top bit of the immediate is
  // encoded in bit 31 of the instruction, we can reuse the Rt form, which
  // uses bit 31 (normally "sf") to choose the register size.
  const char *form = "'Rt, 'IS, 'BImmTest";

  switch (instr->Mask(TestBranchMask)) {
    case TBZ: mnemonic = "tbz"; break;
    case TBNZ: mnemonic = "tbnz"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitMoveWideImmediate(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Rd, 'IMoveImm";

  // Print the shift separately for movk, to make it clear which half word will
  // be overwritten. Movn and movz print the computed immediate, which includes
  // shift calculation.
  switch (instr->Mask(MoveWideImmediateMask)) {
    case MOVN_w:
    case MOVN_x: mnemonic = "movn"; break;
    case MOVZ_w:
    case MOVZ_x: mnemonic = "movz"; break;
    case MOVK_w:
    case MOVK_x: mnemonic = "movk"; form = "'Rd, 'IMoveLSL"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLseLdOp(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "";

  if (instr->Rt() == 31) {
    // alias when Rt is wzr/xzr
    mnemonic = "st'lo'la'ls";
    form = "'Rs, ['Xns]";
  } else {
    mnemonic = "ld'lo'la'ls";
    form = "'Rs, 'Rt, ['Xns]";
  }

  Format(instr, mnemonic, form);
}


#define LOAD_STORE_LIST(V)    \
  V(STRB_w, "strb", "'Wt")    \
  V(STRH_w, "strh", "'Wt")    \
  V(STR_w, "str", "'Wt")      \
  V(STR_x, "str", "'Xt")      \
  V(LDRB_w, "ldrb", "'Wt")    \
  V(LDRH_w, "ldrh", "'Wt")    \
  V(LDR_w, "ldr", "'Wt")      \
  V(LDR_x, "ldr", "'Xt")      \
  V(LDRSB_x, "ldrsb", "'Xt")  \
  V(LDRSH_x, "ldrsh", "'Xt")  \
  V(LDRSW_x, "ldrsw", "'Xt")  \
  V(LDRSB_w, "ldrsb", "'Wt")  \
  V(LDRSH_w, "ldrsh", "'Wt")  \
  V(STR_s, "str", "'St")      \
  V(STR_d, "str", "'Dt")      \
  V(LDR_s, "ldr", "'St")      \
  V(LDR_d, "ldr", "'Dt")

void Disassembler::VisitLoadStorePreIndex(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(LoadStorePreIndex)";

  switch (instr->Mask(LoadStorePreIndexMask)) {
    #define LS_PREINDEX(A, B, C) \
    case A##_pre: mnemonic = B; form = C ", ['Xns'ILS]!"; break;
    LOAD_STORE_LIST(LS_PREINDEX)
    #undef LS_PREINDEX
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStorePostIndex(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(LoadStorePostIndex)";

  switch (instr->Mask(LoadStorePostIndexMask)) {
    #define LS_POSTINDEX(A, B, C) \
    case A##_post: mnemonic = B; form = C ", ['Xns]'ILS"; break;
    LOAD_STORE_LIST(LS_POSTINDEX)
    #undef LS_POSTINDEX
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStoreUnsignedOffset(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(LoadStoreUnsignedOffset)";

  switch (instr->Mask(LoadStoreUnsignedOffsetMask)) {
    #define LS_UNSIGNEDOFFSET(A, B, C) \
    case A##_unsigned: mnemonic = B; form = C ", ['Xns'ILU]"; break;
    LOAD_STORE_LIST(LS_UNSIGNEDOFFSET)
    #undef LS_UNSIGNEDOFFSET
    case PRFM_unsigned: mnemonic = "prfm"; form = "'PrefOp, ['Xn'ILU]";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStoreRegisterOffset(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(LoadStoreRegisterOffset)";

  switch (instr->Mask(LoadStoreRegisterOffsetMask)) {
    #define LS_REGISTEROFFSET(A, B, C) \
    case A##_reg: mnemonic = B; form = C ", ['Xns, 'Offsetreg]"; break;
    LOAD_STORE_LIST(LS_REGISTEROFFSET)
    #undef LS_REGISTEROFFSET
    case PRFM_reg: mnemonic = "prfm"; form = "'PrefOp, ['Xns, 'Offsetreg]";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStoreUnscaledOffset(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'Wt, ['Xns'ILS]";
  const char *form_x = "'Xt, ['Xns'ILS]";
  const char *form_s = "'St, ['Xns'ILS]";
  const char *form_d = "'Dt, ['Xns'ILS]";

  switch (instr->Mask(LoadStoreUnscaledOffsetMask)) {
    case STURB_w:  mnemonic = "sturb"; break;
    case STURH_w:  mnemonic = "sturh"; break;
    case STUR_w:   mnemonic = "stur"; break;
    case STUR_x:   mnemonic = "stur"; form = form_x; break;
    case STUR_s:   mnemonic = "stur"; form = form_s; break;
    case STUR_d:   mnemonic = "stur"; form = form_d; break;
    case LDURB_w:  mnemonic = "ldurb"; break;
    case LDURH_w:  mnemonic = "ldurh"; break;
    case LDUR_w:   mnemonic = "ldur"; break;
    case LDUR_x:   mnemonic = "ldur"; form = form_x; break;
    case LDUR_s:   mnemonic = "ldur"; form = form_s; break;
    case LDUR_d:   mnemonic = "ldur"; form = form_d; break;
    case LDURSB_x: form = form_x;  // Fall through.
    case LDURSB_w: mnemonic = "ldursb"; break;
    case LDURSH_x: form = form_x;  // Fall through.
    case LDURSH_w: mnemonic = "ldursh"; break;
    case LDURSW_x: mnemonic = "ldursw"; form = form_x; break;
    default: form = "(LoadStoreUnscaledOffset)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadLiteral(Instruction* instr) {
  const char *mnemonic = "ldr";
  const char *form = "(LoadLiteral)";

  switch (instr->Mask(LoadLiteralMask)) {
    case LDR_w_lit: form = "'Wt, 'ILLiteral 'LValue"; break;
    case LDR_x_lit: form = "'Xt, 'ILLiteral 'LValue"; break;
    case LDR_s_lit: form = "'St, 'ILLiteral 'LValue"; break;
    case LDR_d_lit: form = "'Dt, 'ILLiteral 'LValue"; break;
    default: mnemonic = "unimplemented";
  }
  Format(instr, mnemonic, form);
}


#define LOAD_STORE_PAIR_LIST(V)         \
  V(STP_w, "stp", "'Wt, 'Wt2", "4")     \
  V(LDP_w, "ldp", "'Wt, 'Wt2", "4")     \
  V(LDPSW_x, "ldpsw", "'Xt, 'Xt2", "4") \
  V(STP_x, "stp", "'Xt, 'Xt2", "8")     \
  V(LDP_x, "ldp", "'Xt, 'Xt2", "8")     \
  V(STP_s, "stp", "'St, 'St2", "4")     \
  V(LDP_s, "ldp", "'St, 'St2", "4")     \
  V(STP_d, "stp", "'Dt, 'Dt2", "8")     \
  V(LDP_d, "ldp", "'Dt, 'Dt2", "8")

void Disassembler::VisitLoadStorePairPostIndex(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(LoadStorePairPostIndex)";

  switch (instr->Mask(LoadStorePairPostIndexMask)) {
    #define LSP_POSTINDEX(A, B, C, D) \
    case A##_post: mnemonic = B; form = C ", ['Xns]'ILP" D; break;
    LOAD_STORE_PAIR_LIST(LSP_POSTINDEX)
    #undef LSP_POSTINDEX
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStorePairPreIndex(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(LoadStorePairPreIndex)";

  switch (instr->Mask(LoadStorePairPreIndexMask)) {
    #define LSP_PREINDEX(A, B, C, D) \
    case A##_pre: mnemonic = B; form = C ", ['Xns'ILP" D "]!"; break;
    LOAD_STORE_PAIR_LIST(LSP_PREINDEX)
    #undef LSP_PREINDEX
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStorePairOffset(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(LoadStorePairOffset)";

  switch (instr->Mask(LoadStorePairOffsetMask)) {
    #define LSP_OFFSET(A, B, C, D) \
    case A##_off: mnemonic = B; form = C ", ['Xns'ILP" D "]"; break;
    LOAD_STORE_PAIR_LIST(LSP_OFFSET)
    #undef LSP_OFFSET
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStorePairNonTemporal(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form;

  switch (instr->Mask(LoadStorePairNonTemporalMask)) {
    case STNP_w: mnemonic = "stnp"; form = "'Wt, 'Wt2, ['Xns'ILP4]"; break;
    case LDNP_w: mnemonic = "ldnp"; form = "'Wt, 'Wt2, ['Xns'ILP4]"; break;
    case STNP_x: mnemonic = "stnp"; form = "'Xt, 'Xt2, ['Xns'ILP8]"; break;
    case LDNP_x: mnemonic = "ldnp"; form = "'Xt, 'Xt2, ['Xns'ILP8]"; break;
    case STNP_s: mnemonic = "stnp"; form = "'St, 'St2, ['Xns'ILP4]"; break;
    case LDNP_s: mnemonic = "ldnp"; form = "'St, 'St2, ['Xns'ILP4]"; break;
    case STNP_d: mnemonic = "stnp"; form = "'Dt, 'Dt2, ['Xns'ILP8]"; break;
    case LDNP_d: mnemonic = "ldnp"; form = "'Dt, 'Dt2, ['Xns'ILP8]"; break;
    default: form = "(LoadStorePairNonTemporal)";
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitLoadStoreExclusive(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form;
  switch(instr->Mask(LoadStoreExclusiveMask)) {
    case STXRB_w:  mnemonic = "stxrb";  form = "'Ws, 'Wt, ['Xns]"; break;
    case STXRH_w:  mnemonic = "stxrh";  form = "'Ws, 'Wt, ['Xns]"; break;
    case STXR_w:   mnemonic = "stxr";   form = "'Ws, 'Wt, ['Xns]"; break;
    case STXR_x:   mnemonic = "stxr";   form = "'Ws, 'Xt, ['Xns]"; break;
    case LDXRB_w:  mnemonic = "ldxrb";  form = "'Wt, ['Xns]"; break;
    case LDXRH_w:  mnemonic = "ldxrh";  form = "'Wt, ['Xns]"; break;
    case LDXR_w:   mnemonic = "ldxr";   form = "'Wt, ['Xns]"; break;
    case LDXR_x:   mnemonic = "ldxr";   form = "'Xt, ['Xns]"; break;
    case STXP_w:   mnemonic = "stxp";   form = "'Ws, 'Wt, 'Wt2, ['Xns]"; break;
    case STXP_x:   mnemonic = "stxp";   form = "'Ws, 'Xt, 'Xt2, ['Xns]"; break;
    case LDXP_w:   mnemonic = "ldxp";   form = "'Wt, 'Wt2, ['Xns]"; break;
    case LDXP_x:   mnemonic = "ldxp";   form = "'Xt, 'Xt2, ['Xns]"; break;
    case STLXRB_w: mnemonic = "stlxrb"; form = "'Ws, 'Wt, ['Xns]"; break;
    case STLXRH_w: mnemonic = "stlxrh"; form = "'Ws, 'Wt, ['Xns]"; break;
    case STLXR_w:  mnemonic = "stlxr";  form = "'Ws, 'Wt, ['Xns]"; break;
    case STLXR_x:  mnemonic = "stlxr";  form = "'Ws, 'Xt, ['Xns]"; break;
    case LDAXRB_w: mnemonic = "ldaxrb"; form = "'Wt, ['Xns]"; break;
    case LDAXRH_w: mnemonic = "ldaxrh"; form = "'Wt, ['Xns]"; break;
    case LDAXR_w:  mnemonic = "ldaxr";  form = "'Wt, ['Xns]"; break;
    case LDAXR_x:  mnemonic = "ldaxr";  form = "'Xt, ['Xns]"; break;
    case STLXP_w:  mnemonic = "stlxp";  form = "'Ws, 'Wt, 'Wt2, ['Xns]"; break;
    case STLXP_x:  mnemonic = "stlxp";  form = "'Ws, 'Xt, 'Xt2, ['Xns]"; break;
    case LDAXP_w:  mnemonic = "ldaxp";  form = "'Wt, 'Wt2, ['Xns]"; break;
    case LDAXP_x:  mnemonic = "ldaxp";  form = "'Xt, 'Xt2, ['Xns]"; break;
    case STLRB_w:  mnemonic = "stlrb";  form = "'Wt, ['Xns]"; break;
    case STLRH_w:  mnemonic = "stlrh";  form = "'Wt, ['Xns]"; break;
    case STLR_w:   mnemonic = "stlr";   form = "'Wt, ['Xns]"; break;
    case STLR_x:   mnemonic = "stlr";   form = "'Xt, ['Xns]"; break;
    case LDARB_w:  mnemonic = "ldarb";  form = "'Wt, ['Xns]"; break;
    case LDARH_w:  mnemonic = "ldarh";  form = "'Wt, ['Xns]"; break;
    case LDAR_w:   mnemonic = "ldar";   form = "'Wt, ['Xns]"; break;
    case LDAR_x:   mnemonic = "ldar";   form = "'Xt, ['Xns]"; break;
    default: form = "(LoadStoreExclusive)";
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitFPCompare(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'Fn, 'Fm";
  const char *form_zero = "'Fn, #0.0";

  switch (instr->Mask(FPCompareMask)) {
    case FCMP_s_zero:
    case FCMP_d_zero: form = form_zero;  // Fall through.
    case FCMP_s:
    case FCMP_d: mnemonic = "fcmp"; break;
    default: form = "(FPCompare)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPConditionalCompare(Instruction* instr) {
  const char *mnemonic = "unmplemented";
  const char *form = "'Fn, 'Fm, 'INzcv, 'Cond";

  switch (instr->Mask(FPConditionalCompareMask)) {
    case FCCMP_s:
    case FCCMP_d: mnemonic = "fccmp"; break;
    case FCCMPE_s:
    case FCCMPE_d: mnemonic = "fccmpe"; break;
    default: form = "(FPConditionalCompare)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPConditionalSelect(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Fd, 'Fn, 'Fm, 'Cond";

  switch (instr->Mask(FPConditionalSelectMask)) {
    case FCSEL_s:
    case FCSEL_d: mnemonic = "fcsel"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPDataProcessing1Source(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'Fd, 'Fn";

  switch (instr->Mask(FPDataProcessing1SourceMask)) {
    #define FORMAT(A, B)  \
    case A##_s:           \
    case A##_d: mnemonic = B; break;
    FORMAT(FMOV, "fmov");
    FORMAT(FABS, "fabs");
    FORMAT(FNEG, "fneg");
    FORMAT(FSQRT, "fsqrt");
    FORMAT(FRINTN, "frintn");
    FORMAT(FRINTP, "frintp");
    FORMAT(FRINTM, "frintm");
    FORMAT(FRINTZ, "frintz");
    FORMAT(FRINTA, "frinta");
    FORMAT(FRINTX, "frintx");
    FORMAT(FRINTI, "frinti");
    #undef FORMAT
    case FCVT_ds: mnemonic = "fcvt"; form = "'Dd, 'Sn"; break;
    case FCVT_sd: mnemonic = "fcvt"; form = "'Sd, 'Dn"; break;
    default: form = "(FPDataProcessing1Source)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPDataProcessing2Source(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Fd, 'Fn, 'Fm";

  switch (instr->Mask(FPDataProcessing2SourceMask)) {
    #define FORMAT(A, B)  \
    case A##_s:           \
    case A##_d: mnemonic = B; break;
    FORMAT(FMUL, "fmul");
    FORMAT(FDIV, "fdiv");
    FORMAT(FADD, "fadd");
    FORMAT(FSUB, "fsub");
    FORMAT(FMAX, "fmax");
    FORMAT(FMIN, "fmin");
    FORMAT(FMAXNM, "fmaxnm");
    FORMAT(FMINNM, "fminnm");
    FORMAT(FNMUL, "fnmul");
    #undef FORMAT
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPDataProcessing3Source(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Fd, 'Fn, 'Fm, 'Fa";

  switch (instr->Mask(FPDataProcessing3SourceMask)) {
    #define FORMAT(A, B)  \
    case A##_s:           \
    case A##_d: mnemonic = B; break;
    FORMAT(FMADD, "fmadd");
    FORMAT(FMSUB, "fmsub");
    FORMAT(FNMADD, "fnmadd");
    FORMAT(FNMSUB, "fnmsub");
    #undef FORMAT
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPImmediate(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "(FPImmediate)";

  switch (instr->Mask(FPImmediateMask)) {
    case FMOV_s_imm: mnemonic = "fmov"; form = "'Sd, 'IFPSingle"; break;
    case FMOV_d_imm: mnemonic = "fmov"; form = "'Dd, 'IFPDouble"; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPIntegerConvert(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(FPIntegerConvert)";
  const char *form_rf = "'Rd, 'Fn";
  const char *form_fr = "'Fd, 'Rn";

  switch (instr->Mask(FPIntegerConvertMask)) {
    case FMOV_ws:
    case FMOV_xd: mnemonic = "fmov"; form = form_rf; break;
    case FMOV_sw:
    case FMOV_dx: mnemonic = "fmov"; form = form_fr; break;
    case FCVTMS_ws:
    case FCVTMS_xs:
    case FCVTMS_wd:
    case FCVTMS_xd: mnemonic = "fcvtms"; form = form_rf; break;
    case FCVTMU_ws:
    case FCVTMU_xs:
    case FCVTMU_wd:
    case FCVTMU_xd: mnemonic = "fcvtmu"; form = form_rf; break;
    case FCVTNS_ws:
    case FCVTNS_xs:
    case FCVTNS_wd:
    case FCVTNS_xd: mnemonic = "fcvtns"; form = form_rf; break;
    case FCVTNU_ws:
    case FCVTNU_xs:
    case FCVTNU_wd:
    case FCVTNU_xd: mnemonic = "fcvtnu"; form = form_rf; break;
    case FCVTZU_xd:
    case FCVTZU_ws:
    case FCVTZU_wd:
    case FCVTZU_xs: mnemonic = "fcvtzu"; form = form_rf; break;
    case FCVTZS_xd:
    case FCVTZS_wd:
    case FCVTZS_xs:
    case FCVTZS_ws: mnemonic = "fcvtzs"; form = form_rf; break;
    case SCVTF_sw:
    case SCVTF_sx:
    case SCVTF_dw:
    case SCVTF_dx: mnemonic = "scvtf"; form = form_fr; break;
    case UCVTF_sw:
    case UCVTF_sx:
    case UCVTF_dw:
    case UCVTF_dx: mnemonic = "ucvtf"; form = form_fr; break;
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitFPFixedPointConvert(Instruction* instr) {
  const char *mnemonic = "";
  const char *form = "'Rd, 'Fn, 'IFPFBits";
  const char *form_fr = "'Fd, 'Rn, 'IFPFBits";

  switch (instr->Mask(FPFixedPointConvertMask)) {
    case FCVTZS_ws_fixed:
    case FCVTZS_xs_fixed:
    case FCVTZS_wd_fixed:
    case FCVTZS_xd_fixed: mnemonic = "fcvtzs"; break;
    case FCVTZU_ws_fixed:
    case FCVTZU_xs_fixed:
    case FCVTZU_wd_fixed:
    case FCVTZU_xd_fixed: mnemonic = "fcvtzu"; break;
    case SCVTF_sw_fixed:
    case SCVTF_sx_fixed:
    case SCVTF_dw_fixed:
    case SCVTF_dx_fixed: mnemonic = "scvtf"; form = form_fr; break;
    case UCVTF_sw_fixed:
    case UCVTF_sx_fixed:
    case UCVTF_dw_fixed:
    case UCVTF_dx_fixed: mnemonic = "ucvtf"; form = form_fr; break;
    default: not_reached();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitSystem(Instruction* instr) {
  // Some system instructions hijack their Op and Cp fields to represent a
  // range of immediates instead of indicating a different instruction. This
  // makes the decoding tricky.
  const char *mnemonic = "unimplemented";
  const char *form = "(System)";

  if (instr->Mask(SystemSysRegFMask) == SystemSysRegFixed) {
    switch (instr->Mask(SystemSysRegMask)) {
      case MRS: {
        mnemonic = "mrs";
        switch (instr->ImmSystemRegister()) {
          case NZCV: form = "'Xt, nzcv"; break;
          case FPCR: form = "'Xt, fpcr"; break;
          case TPIDR_EL0: form = "'Xt, tpidr_el0"; break;
          default: form = "'Xt, (unknown)"; break;
        }
        break;
      }
      case MSR: {
        mnemonic = "msr";
        switch (instr->ImmSystemRegister()) {
          case NZCV: form = "nzcv, 'Xt"; break;
          case FPCR: form = "fpcr, 'Xt"; break;
          default: form = "(unknown), 'Xt"; break;
        }
        break;
      }
    }
  } else if (instr->Mask(SystemHintFMask) == SystemHintFixed) {
    assert(instr->Mask(SystemHintMask) == HINT);
    switch (instr->ImmHint()) {
      case NOP: {
        mnemonic = "nop";
        form = nullptr;
        break;
      }
    }
  }

  Format(instr, mnemonic, form);
}


void Disassembler::VisitException(Instruction* instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'IDebug";

  switch (instr->Mask(ExceptionMask)) {
    case HLT: mnemonic = "hlt"; break;
    case BRK: mnemonic = "brk"; break;
    case SVC: mnemonic = "svc"; break;
    case HVC: mnemonic = "hvc"; break;
    case SMC: mnemonic = "smc"; break;
    case DCPS1: mnemonic = "dcps1"; form = "{'IDebug}"; break;
    case DCPS2: mnemonic = "dcps2"; form = "{'IDebug}"; break;
    case DCPS3: mnemonic = "dcps3"; form = "{'IDebug}"; break;
    default: form = "(Exception)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitUnimplemented(Instruction* instr) {
  Format(instr, "unimplemented", "(Unimplemented)");
}


void Disassembler::VisitUnallocated(Instruction* instr) {
  Format(instr, "unallocated", "(Unallocated)");
}


void Disassembler::ProcessOutput(Instruction* /*instr*/) {
  // The base disasm does nothing more than disassembling into a buffer.
}

void Disassembler::MapCodeAddress(int64_t base_address,
                                  const Instruction *instr_address) {
  SetCodeAddressOffset(base_address -
                          reinterpret_cast<intptr_t>(instr_address));
}
int64_t Disassembler::CodeRelativeAddress(const void *addr) {
  return reinterpret_cast<intptr_t>(addr) + CodeAddressOffset();
}

void Disassembler::AppendCodeRelativeAddressToOutput(const Instruction *instr,
                                                     const void *addr) {
  USE(instr);
  int64_t rel_addr = CodeRelativeAddress(addr);
  if (rel_addr >= 0) {
    AppendToOutput("(addr 0x%" PRIx64 ")", rel_addr);
  } else {
    AppendToOutput("(addr -0x%" PRIx64 ")", -rel_addr);
  }
}

void Disassembler::Format(Instruction* instr, const char* mnemonic,
                          const char* format) {
  assert(mnemonic != nullptr);
  ResetOutput();
  Substitute(instr, mnemonic);
  if (format != nullptr) {
    buffer_[buffer_pos_++] = ' ';
    Substitute(instr, format);
  }
  buffer_[buffer_pos_] = 0;
  ProcessOutput(instr);
}


void Disassembler::Substitute(Instruction* instr, const char* string) {
  char chr = *string++;
  while (chr != '\0') {
    if (chr == '\'') {
      string += SubstituteField(instr, string);
    } else {
      buffer_[buffer_pos_++] = chr;
    }
    chr = *string++;
  }
}


int Disassembler::SubstituteField(Instruction* instr, const char* format) {
  switch (format[0]) {
    case 'R':  // Register. X or W, selected by sf bit.
    case 'F':  // FP Register. S or D, selected by type field.
    case 'W':
    case 'X':
    case 'S':
    case 'D': return SubstituteRegisterField(instr, format);
    case 'I': return SubstituteImmediateField(instr, format);
    case 'L': return SubstituteLiteralField(instr, format);
    case 'H': return SubstituteShiftField(instr, format);
    case 'P': return SubstitutePrefetchField(instr, format);
    case 'C': return SubstituteConditionField(instr, format);
    case 'E': return SubstituteExtendField(instr, format);
    case 'A': return SubstitutePCRelAddressField(instr, format);
    case 'B': return SubstituteBranchTargetField(instr, format);
    case 'O': return SubstituteLSRegOffsetField(instr, format);
    case 'l': return SubstituteInstructionAttributes(instr, format);
    default: {
      not_reached();
      return 1;
    }
  }
}


int Disassembler::SubstituteRegisterField(Instruction* instr,
                                          const char* format) {
  unsigned reg_num = 0;
  unsigned field_len = 2;
  switch (format[1]) {
    case 'd': reg_num = instr->Rd(); break;
    case 'n': reg_num = instr->Rn(); break;
    case 'm': reg_num = instr->Rm(); break;
    case 'a': reg_num = instr->Ra(); break;
    case 't': {
      if (format[2] == '2') {
        reg_num = instr->Rt2();
        field_len = 3;
      } else {
        reg_num = instr->Rt();
      }
      break;
    }
    case 's': reg_num = instr->Rs(); break;
    default: not_reached();
  }

  // Increase field length for registers tagged as stack.
  if (format[2] == 's') {
    field_len = 3;
  }

  char reg_type;
  if (format[0] == 'R') {
    // Register type is R: use sf bit to choose X and W.
    reg_type = instr->SixtyFourBits() ? 'x' : 'w';
  } else if (format[0] == 'F') {
    // Floating-point register: use type field to choose S or D.
    reg_type = ((instr->FPType() & 1) == 0) ? 's' : 'd';
  } else {
    // Register type is specified. Make it lower case.
    reg_type = format[0] + 0x20;
  }

  if ((reg_num != kZeroRegCode) || (reg_type == 's') || (reg_type == 'd')) {
    // A normal register: w0 - w30, x0 - x30, s0 - s31, d0 - d31.
    AppendToOutput("%c%d", reg_type, reg_num);
  } else if (format[2] == 's') {
    // Disassemble w31/x31 as stack pointer wsp/sp.
    AppendToOutput("%s", (reg_type == 'w') ? "wsp" : "sp");
  } else {
    // Disassemble w31/x31 as zero register wzr/xzr.
    AppendToOutput("%czr", reg_type);
  }

  return field_len;
}


int Disassembler::SubstituteImmediateField(Instruction* instr,
                                           const char* format) {
  assert(format[0] == 'I');

  switch (format[1]) {
    case 'M': {  // IMoveImm or IMoveLSL.
      if (format[5] == 'I') {
        uint64_t imm = instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
        AppendToOutput("#0x%" PRIx64, imm);
      } else {
        assert(format[5] == 'L');
        AppendToOutput("#0x%" PRIx64, instr->ImmMoveWide());
        if (instr->ShiftMoveWide() > 0) {
          AppendToOutput(", lsl #%d", 16 * instr->ShiftMoveWide());
        }
      }
      return 8;
    }
    case 'L': {
      switch (format[2]) {
        case 'L': {  // ILLiteral - Immediate Load Literal.
          AppendToOutput("pc%+" PRId64,
                         instr->ImmLLiteral() << kLiteralEntrySizeLog2);
          return 9;
        }
        case 'S': {  // ILS - Immediate Load/Store.
          if (instr->ImmLS() != 0) {
            AppendToOutput(", #%" PRId64, instr->ImmLS());
          }
          return 3;
        }
        case 'P': {  // ILPx - Immediate Load/Store Pair, x = access size.
          if (instr->ImmLSPair() != 0) {
            // format[3] is the scale value. Convert to a number.
            int scale = format[3] - 0x30;
            AppendToOutput(", #%" PRId64, instr->ImmLSPair() * scale);
          }
          return 4;
        }
        case 'U': {  // ILU - Immediate Load/Store Unsigned.
          if (instr->ImmLSUnsigned() != 0) {
            AppendToOutput(", #%" PRIu64,
                           instr->ImmLSUnsigned() << instr->SizeLS());
          }
          return 3;
        }
      }
    }
    case 'C': {  // ICondB - Immediate Conditional Branch.
      int64_t offset = instr->ImmCondBranch() << 2;
      char sign = (offset >= 0) ? '+' : '-';
      AppendToOutput("#%c0x%" PRIx64, sign, offset);
      return 6;
    }
    case 'A': {  // IAddSub.
      assert(instr->ShiftAddSub() <= 1);
      int64_t imm = instr->ImmAddSub() << (12 * instr->ShiftAddSub());
      AppendToOutput("#0x%" PRIx64 " (%" PRId64 ")", imm, imm);
      return 7;
    }
    case 'F': {  // IFPSingle, IFPDouble or IFPFBits.
      if (format[3] == 'F') {  // IFPFbits.
        AppendToOutput("#%d", 64 - instr->FPScale());
        return 8;
      } else {
        AppendToOutput("#0x%" PRIx64 " (%.4f)", instr->ImmFP(),
                       format[3] == 'S' ? instr->ImmFP32() : instr->ImmFP64());
        return 9;
      }
    }
    case 'T': {  // ITri - Immediate Triangular Encoded.
      AppendToOutput("#0x%" PRIx64, instr->ImmLogical());
      return 4;
    }
    case 'N': {  // INzcv.
      int nzcv = (instr->Nzcv() << Flags_offset);
      AppendToOutput("#%c%c%c%c", ((nzcv & NFlag) == 0) ? 'n' : 'N',
                                  ((nzcv & ZFlag) == 0) ? 'z' : 'Z',
                                  ((nzcv & CFlag) == 0) ? 'c' : 'C',
                                  ((nzcv & VFlag) == 0) ? 'v' : 'V');
      return 5;
    }
    case 'P': {  // IP - Conditional compare.
      AppendToOutput("#%d", instr->ImmCondCmp());
      return 2;
    }
    case 'B': {  // Bitfields.
      return SubstituteBitfieldImmediateField(instr, format);
    }
    case 'E': {  // IExtract.
      AppendToOutput("#%d", instr->ImmS());
      return 8;
    }
    case 'S': {  // IS - Test and branch bit.
      AppendToOutput("#%d", (instr->ImmTestBranchBit5() << 5) |
                            instr->ImmTestBranchBit40());
      return 2;
    }
    case 'D': {  // IDebug - HLT and BRK instructions.
      AppendToOutput("#0x%x", instr->ImmException());
      return 6;
    }
    default: {
      not_implemented();
      return 0;
    }
  }
}


int Disassembler::SubstituteBitfieldImmediateField(Instruction* instr,
                                                   const char* format) {
  assert((format[0] == 'I') && (format[1] == 'B'));
  unsigned r = instr->ImmR();
  unsigned s = instr->ImmS();

  switch (format[2]) {
    case 'r': {  // IBr.
      AppendToOutput("#%d", r);
      return 3;
    }
    case 's': {  // IBs+1 or IBs-r+1.
      if (format[3] == '+') {
        AppendToOutput("#%d", s + 1);
        return 5;
      } else {
        assert(format[3] == '-');
        AppendToOutput("#%d", s - r + 1);
        return 7;
      }
    }
    case 'Z': {  // IBZ-r.
      assert((format[3] == '-') && (format[4] == 'r'));
      unsigned reg_size = (instr->SixtyFourBits() == 1) ? kXRegSize : kWRegSize;
      AppendToOutput("#%d", reg_size - r);
      return 5;
    }
    default: {
      not_reached();
      return 0;
    }
  }
}


int Disassembler::SubstituteLiteralField(Instruction* instr,
                                         const char* format) {
  assert(strncmp(format, "LValue", 6) == 0);
  USE(format);
  const void* address = instr->LiteralAddress();
  switch (instr->Mask(LoadLiteralMask)) {
    case LDR_w_lit:
    case LDR_x_lit:
    case LDR_s_lit:
    case LDR_d_lit: AppendCodeRelativeAddressToOutput(instr, address); break;
    default: not_reached();
  }

  return 6;
}


int Disassembler::SubstituteShiftField(Instruction* instr, const char* format) {
  assert(format[0] == 'H');
  assert(instr->ShiftDP() <= 0x3);

  switch (format[1]) {
    case 'D': {  // HDP.
      assert(instr->ShiftDP() != ROR);
    }  // Fall through.
    case 'L': {  // HLo.
      if (instr->ImmDPShift() != 0) {
        const char* shift_type[] = {"lsl", "lsr", "asr", "ror"};
        AppendToOutput(", %s #%" PRId64, shift_type[instr->ShiftDP()],
                       instr->ImmDPShift());
      }
      return 3;
    }
    default:
      not_implemented();
      return 0;
  }
}


int Disassembler::SubstituteConditionField(Instruction* instr,
                                           const char* format) {
  assert(format[0] == 'C');
  const char* condition_code[] = { "eq", "ne", "hs", "lo",
                                   "mi", "pl", "vs", "vc",
                                   "hi", "ls", "ge", "lt",
                                   "gt", "le", "al", "nv" };
  int cond;
  switch (format[1]) {
    case 'B': cond = instr->ConditionBranch(); break;
    case 'I': {
      cond = InvertCondition(static_cast<Condition>(instr->Condition()));
      break;
    }
    default: cond = instr->Condition();
  }
  AppendToOutput("%s", condition_code[cond]);
  return 4;
}


int Disassembler::SubstitutePCRelAddressField(Instruction* instr,
                                              const char* format) {
  USE(format);
  assert(strncmp(format, "AddrPCRel", 9) == 0);

  int offset = instr->ImmPCRel();

  // Only ADR (AddrPCRelByte) is supported.
  assert(strcmp(format, "AddrPCRelByte") == 0);

  char sign = '+';
  if (offset < 0) {
    offset = -offset;
    sign = '-';
  }
  // TODO: Extend this to support printing the target address.
  AppendToOutput("#%c0x%x", sign, offset);
  return 13;
}


int Disassembler::SubstituteBranchTargetField(Instruction* instr,
                                              const char* format) {
  assert(strncmp(format, "BImm", 4) == 0);

  int64_t offset = 0;
  switch (format[5]) {
    // BImmUncn - unconditional branch immediate.
    case 'n': offset = instr->ImmUncondBranch(); break;
    // BImmCond - conditional branch immediate.
    case 'o': offset = instr->ImmCondBranch(); break;
    // BImmCmpa - compare and branch immediate.
    case 'm': offset = instr->ImmCmpBranch(); break;
    // BImmTest - test and branch immediate.
    case 'e': offset = instr->ImmTestBranch(); break;
    default: not_implemented();
  }
  offset <<= kInstructionSizeLog2;
  char sign = '+';
  if (offset < 0) {
    offset = -offset;
    sign = '-';
  }
  AppendToOutput("#%c0x%" PRIx64, sign, offset);
  return 8;
}


int Disassembler::SubstituteExtendField(Instruction* instr,
                                        const char* format) {
  assert(strncmp(format, "Ext", 3) == 0);
  assert(instr->ExtendMode() <= 7);
  USE(format);

  const char* extend_mode[] = { "uxtb", "uxth", "uxtw", "uxtx",
                                "sxtb", "sxth", "sxtw", "sxtx" };

  // If rd or rn is SP, uxtw on 32-bit registers and uxtx on 64-bit
  // registers becomes lsl.
  if (((instr->Rd() == kZeroRegCode) || (instr->Rn() == kZeroRegCode)) &&
      (((instr->ExtendMode() == UXTW) && (instr->SixtyFourBits() == 0)) ||
       (instr->ExtendMode() == UXTX))) {
    if (instr->ImmExtendShift() > 0) {
      AppendToOutput(", lsl #%d", instr->ImmExtendShift());
    }
  } else {
    AppendToOutput(", %s", extend_mode[instr->ExtendMode()]);
    if (instr->ImmExtendShift() > 0) {
      AppendToOutput(" #%d", instr->ImmExtendShift());
    }
  }
  return 3;
}


int Disassembler::SubstituteLSRegOffsetField(Instruction* instr,
                                             const char* format) {
  assert(strncmp(format, "Offsetreg", 9) == 0);
  const char* extend_mode[] = { "undefined", "undefined", "uxtw", "lsl",
                                "undefined", "undefined", "sxtw", "sxtx" };
  USE(format);

  unsigned shift = instr->ImmShiftLS();
  Extend ext = static_cast<Extend>(instr->ExtendMode());
  char reg_type = ((ext == UXTW) || (ext == SXTW)) ? 'w' : 'x';

  unsigned rm = instr->Rm();
  if (rm == kZeroRegCode) {
    AppendToOutput("%czr", reg_type);
  } else {
    AppendToOutput("%c%d", reg_type, rm);
  }

  // Extend mode UXTX is an alias for shift mode LSL here.
  if (!((ext == UXTX) && (shift == 0))) {
    AppendToOutput(", %s", extend_mode[ext]);
    if (shift != 0) {
      AppendToOutput(" #%d", instr->SizeLS());
    }
  }
  return 9;
}


int Disassembler::SubstitutePrefetchField(Instruction* instr,
                                          const char* format) {
  assert(format[0] == 'P');
  USE(format);

  int prefetch_mode = instr->PrefetchMode();

  const char* ls = (prefetch_mode & 0x10) ? "st" : "ld";
  int level = (prefetch_mode >> 1) + 1;
  const char* ks = (prefetch_mode & 1) ? "strm" : "keep";

  AppendToOutput("p%sl%d%s", ls, level, ks);
  return 6;
}


int Disassembler::SubstituteInstructionAttributes(Instruction* instr,
                                          const char* format) {
  assert(format[0] == 'l');
  const char* lse_op[] = { "add", "clr", "eor", "set", 
                           "smax", "smin", "umax", "umin" };
  const char* lse_size[] = { "b", "h", "", "" };
  const char* lse_semantic[] = { "", "l", "a", "al" };

  int idx;
  switch (format[1]) {
    case 'a':
      idx = instr->Ar();
      AppendToOutput("%s", lse_semantic[idx]);
      break;
    case 'o':
      idx = instr->Opc();
      AppendToOutput("%s", lse_op[idx]);
      break;
    case 's':
      idx = instr->SizeLS();
      AppendToOutput("%s", lse_size[idx]);
      break;
  }
  return 2;
}

void Disassembler::ResetOutput() {
  buffer_pos_ = 0;
  buffer_[buffer_pos_] = 0;
}


void Disassembler::AppendToOutput(const char* format, ...) {
  va_list args;
  va_start(args, format);
  buffer_pos_ += vsnprintf(&buffer_[buffer_pos_], buffer_size_, format, args);
  va_end(args);
}


void PrintDisassembler::ProcessOutput(Instruction* instr) {
  for (int i = 0; i < indent_; i++) {
    stream_ << ' ';
  }
  if (*color_) {
    stream_ << color_;
  }

  if (showEncoding_) {
    stream_ << folly::format(
      "{:#16x}  {:08x}\t\t{}\n",
      reinterpret_cast<uint64_t>(instr),
      instr->InstructionBits(),
      GetOutput()
    );
  } else {
    stream_ << folly::format(
      "{:#16x}  \t\t{}\n",
      reinterpret_cast<uint64_t>(instr),
      GetOutput()
    );
  }

  if (*color_) {
    stream_ << HPHP::ANSI_COLOR_END;
  }
}
}  // namespace vixl
