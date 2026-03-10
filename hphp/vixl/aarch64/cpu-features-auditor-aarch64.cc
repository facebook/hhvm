// Copyright 2018, VIXL authors
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
//   * Neither the name of Arm Limited nor the names of its contributors may be
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

#include "cpu-features-auditor-aarch64.h"

#include "cpu-features.h"
#include "globals-vixl.h"
#include "utils-vixl.h"

#include "decoder-aarch64.h"

namespace vixl {
namespace aarch64 {


const CPUFeaturesAuditor::FormToVisitorFnMap*
CPUFeaturesAuditor::GetFormToVisitorFnMap() {
  static const FormToVisitorFnMap form_to_visitor = {
      DEFAULT_FORM_TO_VISITOR_MAP(CPUFeaturesAuditor),
      SIM_AUD_VISITOR_MAP(CPUFeaturesAuditor),
      {"fcmla_asimdelem_c_h"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fcmla_asimdelem_c_s"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmlal2_asimdelem_lh"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmlal_asimdelem_lh"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmla_asimdelem_rh_h"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmla_asimdelem_r_sd"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmlsl2_asimdelem_lh"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmlsl_asimdelem_lh"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmls_asimdelem_rh_h"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmls_asimdelem_r_sd"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmulx_asimdelem_rh_h"_h,
       &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmulx_asimdelem_r_sd"_h,
       &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmul_asimdelem_rh_h"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"fmul_asimdelem_r_sd"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"sdot_asimdelem_d"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"smlal_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"smlsl_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"smull_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"sqdmlal_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"sqdmlsl_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"sqdmull_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"udot_asimdelem_d"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"umlal_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"umlsl_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
      {"umull_asimdelem_l"_h, &CPUFeaturesAuditor::VisitNEONByIndexedElement},
  };
  return &form_to_visitor;
}

// Every instruction must update last_instruction_, even if only to clear it,
// and every instruction must also update seen_ once it has been fully handled.
// This scope makes that simple, and allows early returns in the decode logic.
class CPUFeaturesAuditor::RecordInstructionFeaturesScope {
 public:
  explicit RecordInstructionFeaturesScope(CPUFeaturesAuditor* auditor)
      : auditor_(auditor) {
    auditor_->last_instruction_ = CPUFeatures::None();
  }
  ~RecordInstructionFeaturesScope() {
    auditor_->seen_.Combine(auditor_->last_instruction_);
  }

  void Record(const CPUFeatures& features) {
    auditor_->last_instruction_.Combine(features);
  }

  void Record(CPUFeatures::Feature feature0,
              CPUFeatures::Feature feature1 = CPUFeatures::kNone,
              CPUFeatures::Feature feature2 = CPUFeatures::kNone,
              CPUFeatures::Feature feature3 = CPUFeatures::kNone) {
    auditor_->last_instruction_.Combine(feature0, feature1, feature2, feature3);
  }

  // If exactly one of a or b is known to be available, record it. Otherwise,
  // record both. This is intended for encodings that can be provided by two
  // different features.
  void RecordOneOrBothOf(CPUFeatures::Feature a, CPUFeatures::Feature b) {
    bool hint_a = auditor_->available_.Has(a);
    bool hint_b = auditor_->available_.Has(b);
    if (hint_a && !hint_b) {
      Record(a);
    } else if (hint_b && !hint_a) {
      Record(b);
    } else {
      Record(a, b);
    }
  }

 private:
  CPUFeaturesAuditor* auditor_;
};

void CPUFeaturesAuditor::LoadStoreHelper(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(LoadStoreMask)) {
    case LDR_b:
    case LDR_q:
    case STR_b:
    case STR_q:
      scope.Record(CPUFeatures::kNEON);
      return;
    case LDR_h:
    case LDR_s:
    case LDR_d:
    case STR_h:
    case STR_s:
    case STR_d:
      scope.RecordOneOrBothOf(CPUFeatures::kFP, CPUFeatures::kNEON);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::LoadStorePairHelper(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(LoadStorePairMask)) {
    case LDP_q:
    case STP_q:
      scope.Record(CPUFeatures::kNEON);
      return;
    case LDP_s:
    case LDP_d:
    case STP_s:
    case STP_d: {
      scope.RecordOneOrBothOf(CPUFeatures::kFP, CPUFeatures::kNEON);
      return;
    }
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitAddSubExtended(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitAddSubImmediate(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitAddSubShifted(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitAddSubWithCarry(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitRotateRightIntoFlags(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(RotateRightIntoFlagsMask)) {
    case RMIF:
      scope.Record(CPUFeatures::kFlagM);
      return;
  }
}

void CPUFeaturesAuditor::VisitEvaluateIntoFlags(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(EvaluateIntoFlagsMask)) {
    case SETF8:
    case SETF16:
      scope.Record(CPUFeatures::kFlagM);
      return;
  }
}

void CPUFeaturesAuditor::VisitAtomicMemory(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(AtomicMemoryMask)) {
    case LDAPRB:
    case LDAPRH:
    case LDAPR_w:
    case LDAPR_x:
      scope.Record(CPUFeatures::kRCpc);
      return;
    default:
      // Everything else belongs to the Atomics extension.
      scope.Record(CPUFeatures::kAtomics);
      return;
  }
}

void CPUFeaturesAuditor::VisitBitfield(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitCompareBranch(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitConditionalBranch(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitConditionalCompareImmediate(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitConditionalCompareRegister(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitConditionalSelect(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitCrypto2RegSHA(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  if (form_hash_ == "sha256su0_vv_cryptosha2"_h) {
    scope.Record(CPUFeatures::kNEON, CPUFeatures::kSHA2);
  } else {
    scope.Record(CPUFeatures::kNEON, CPUFeatures::kSHA1);
  }
  USE(instr);
}

void CPUFeaturesAuditor::VisitCrypto3RegSHA(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (form_hash_) {
    case "sha1c_qsv_cryptosha3"_h:
    case "sha1m_qsv_cryptosha3"_h:
    case "sha1p_qsv_cryptosha3"_h:
    case "sha1su0_vvv_cryptosha3"_h:
      scope.Record(CPUFeatures::kNEON, CPUFeatures::kSHA1);
      break;
    case "sha256h_qqv_cryptosha3"_h:
    case "sha256h2_qqv_cryptosha3"_h:
    case "sha256su1_vvv_cryptosha3"_h:
      scope.Record(CPUFeatures::kNEON, CPUFeatures::kSHA2);
      break;
  }
  USE(instr);
}

void CPUFeaturesAuditor::VisitCryptoAES(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  scope.Record(CPUFeatures::kNEON, CPUFeatures::kAES);
  USE(instr);
}

void CPUFeaturesAuditor::VisitCryptoSM3(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  scope.Record(CPUFeatures::kNEON, CPUFeatures::kSM3);
  USE(instr);
}

void CPUFeaturesAuditor::VisitCryptoSM4(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  scope.Record(CPUFeatures::kNEON, CPUFeatures::kSM4);
  USE(instr);
}

void CPUFeaturesAuditor::VisitDataProcessing1Source(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(DataProcessing1SourceMask)) {
    case PACIA:
    case PACIB:
    case PACDA:
    case PACDB:
    case AUTIA:
    case AUTIB:
    case AUTDA:
    case AUTDB:
    case PACIZA:
    case PACIZB:
    case PACDZA:
    case PACDZB:
    case AUTIZA:
    case AUTIZB:
    case AUTDZA:
    case AUTDZB:
    case XPACI:
    case XPACD:
      scope.Record(CPUFeatures::kPAuth);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitDataProcessing2Source(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(DataProcessing2SourceMask)) {
    case CRC32B:
    case CRC32H:
    case CRC32W:
    case CRC32X:
    case CRC32CB:
    case CRC32CH:
    case CRC32CW:
    case CRC32CX:
      scope.Record(CPUFeatures::kCRC32);
      return;
    case PACGA:
      scope.Record(CPUFeatures::kPAuth, CPUFeatures::kPAuthGeneric);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitLoadStoreRCpcUnscaledOffset(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(LoadStoreRCpcUnscaledOffsetMask)) {
    case LDAPURB:
    case LDAPURSB_w:
    case LDAPURSB_x:
    case LDAPURH:
    case LDAPURSH_w:
    case LDAPURSH_x:
    case LDAPUR_w:
    case LDAPURSW:
    case LDAPUR_x:

    // These stores don't actually have RCpc semantics but they're included with
    // the RCpc extensions.
    case STLURB:
    case STLURH:
    case STLUR_w:
    case STLUR_x:
      scope.Record(CPUFeatures::kRCpc, CPUFeatures::kRCpcImm);
      return;
  }
}

void CPUFeaturesAuditor::VisitLoadStorePAC(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
  scope.Record(CPUFeatures::kPAuth);
}

void CPUFeaturesAuditor::VisitDataProcessing3Source(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitException(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitExtract(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitFPCompare(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  switch (instr->Mask(FPCompareMask)) {
    case FCMP_h:
    case FCMP_h_zero:
    case FCMPE_h:
    case FCMPE_h_zero:
      scope.Record(CPUFeatures::kFPHalf);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitFPConditionalCompare(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  switch (instr->Mask(FPConditionalCompareMask)) {
    case FCCMP_h:
    case FCCMPE_h:
      scope.Record(CPUFeatures::kFPHalf);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitFPConditionalSelect(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  if (instr->Mask(FPConditionalSelectMask) == FCSEL_h) {
    scope.Record(CPUFeatures::kFPHalf);
  }
}

void CPUFeaturesAuditor::VisitFPDataProcessing1Source(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  switch (instr->Mask(FPDataProcessing1SourceMask)) {
    case FMOV_h:
    case FABS_h:
    case FNEG_h:
    case FSQRT_h:
    case FRINTN_h:
    case FRINTP_h:
    case FRINTM_h:
    case FRINTZ_h:
    case FRINTA_h:
    case FRINTX_h:
    case FRINTI_h:
      scope.Record(CPUFeatures::kFPHalf);
      return;
    case FRINT32X_s:
    case FRINT32X_d:
    case FRINT32Z_s:
    case FRINT32Z_d:
    case FRINT64X_s:
    case FRINT64X_d:
    case FRINT64Z_s:
    case FRINT64Z_d:
      scope.Record(CPUFeatures::kFrintToFixedSizedInt);
      return;
    default:
      // No special CPU features.
      // This category includes some half-precision FCVT instructions that do
      // not require FPHalf.
      return;
  }
}

void CPUFeaturesAuditor::VisitFPDataProcessing2Source(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  switch (instr->Mask(FPDataProcessing2SourceMask)) {
    case FMUL_h:
    case FDIV_h:
    case FADD_h:
    case FSUB_h:
    case FMAX_h:
    case FMIN_h:
    case FMAXNM_h:
    case FMINNM_h:
    case FNMUL_h:
      scope.Record(CPUFeatures::kFPHalf);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitFPDataProcessing3Source(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  switch (instr->Mask(FPDataProcessing3SourceMask)) {
    case FMADD_h:
    case FMSUB_h:
    case FNMADD_h:
    case FNMSUB_h:
      scope.Record(CPUFeatures::kFPHalf);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitFPFixedPointConvert(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  switch (instr->Mask(FPFixedPointConvertMask)) {
    case FCVTZS_wh_fixed:
    case FCVTZS_xh_fixed:
    case FCVTZU_wh_fixed:
    case FCVTZU_xh_fixed:
    case SCVTF_hw_fixed:
    case SCVTF_hx_fixed:
    case UCVTF_hw_fixed:
    case UCVTF_hx_fixed:
      scope.Record(CPUFeatures::kFPHalf);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitFPImmediate(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require FP.
  scope.Record(CPUFeatures::kFP);
  if (instr->Mask(FPImmediateMask) == FMOV_h_imm) {
    scope.Record(CPUFeatures::kFPHalf);
  }
}

void CPUFeaturesAuditor::VisitFPIntegerConvert(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(FPIntegerConvertMask)) {
    case FCVTAS_wh:
    case FCVTAS_xh:
    case FCVTAU_wh:
    case FCVTAU_xh:
    case FCVTMS_wh:
    case FCVTMS_xh:
    case FCVTMU_wh:
    case FCVTMU_xh:
    case FCVTNS_wh:
    case FCVTNS_xh:
    case FCVTNU_wh:
    case FCVTNU_xh:
    case FCVTPS_wh:
    case FCVTPS_xh:
    case FCVTPU_wh:
    case FCVTPU_xh:
    case FCVTZS_wh:
    case FCVTZS_xh:
    case FCVTZU_wh:
    case FCVTZU_xh:
    case FMOV_hw:
    case FMOV_hx:
    case FMOV_wh:
    case FMOV_xh:
    case SCVTF_hw:
    case SCVTF_hx:
    case UCVTF_hw:
    case UCVTF_hx:
      scope.Record(CPUFeatures::kFP);
      scope.Record(CPUFeatures::kFPHalf);
      return;
    case FMOV_dx:
      scope.RecordOneOrBothOf(CPUFeatures::kFP, CPUFeatures::kNEON);
      return;
    case FMOV_d1_x:
    case FMOV_x_d1:
      scope.Record(CPUFeatures::kFP);
      scope.Record(CPUFeatures::kNEON);
      return;
    case FJCVTZS:
      scope.Record(CPUFeatures::kFP);
      scope.Record(CPUFeatures::kJSCVT);
      return;
    default:
      scope.Record(CPUFeatures::kFP);
      return;
  }
}

void CPUFeaturesAuditor::VisitLoadLiteral(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(LoadLiteralMask)) {
    case LDR_s_lit:
    case LDR_d_lit:
      scope.RecordOneOrBothOf(CPUFeatures::kFP, CPUFeatures::kNEON);
      return;
    case LDR_q_lit:
      scope.Record(CPUFeatures::kNEON);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitLoadStoreExclusive(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(LoadStoreExclusiveMask)) {
    case CAS_w:
    case CASA_w:
    case CASL_w:
    case CASAL_w:
    case CAS_x:
    case CASA_x:
    case CASL_x:
    case CASAL_x:
    case CASB:
    case CASAB:
    case CASLB:
    case CASALB:
    case CASH:
    case CASAH:
    case CASLH:
    case CASALH:
    case CASP_w:
    case CASPA_w:
    case CASPL_w:
    case CASPAL_w:
    case CASP_x:
    case CASPA_x:
    case CASPL_x:
    case CASPAL_x:
      scope.Record(CPUFeatures::kAtomics);
      return;
    case STLLRB:
    case LDLARB:
    case STLLRH:
    case LDLARH:
    case STLLR_w:
    case LDLAR_w:
    case STLLR_x:
    case LDLAR_x:
      scope.Record(CPUFeatures::kLORegions);
      return;
    default:
      // No special CPU features.
      return;
  }
}

void CPUFeaturesAuditor::VisitLoadStorePairNonTemporal(
    const Instruction* instr) {
  LoadStorePairHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStorePairOffset(const Instruction* instr) {
  LoadStorePairHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStorePairPostIndex(const Instruction* instr) {
  LoadStorePairHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStorePairPreIndex(const Instruction* instr) {
  LoadStorePairHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStorePostIndex(const Instruction* instr) {
  LoadStoreHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStorePreIndex(const Instruction* instr) {
  LoadStoreHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStoreRegisterOffset(
    const Instruction* instr) {
  LoadStoreHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStoreUnscaledOffset(
    const Instruction* instr) {
  LoadStoreHelper(instr);
}

void CPUFeaturesAuditor::VisitLoadStoreUnsignedOffset(
    const Instruction* instr) {
  LoadStoreHelper(instr);
}

void CPUFeaturesAuditor::VisitLogicalImmediate(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitLogicalShifted(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitMoveWideImmediate(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEON2RegMisc(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  switch (instr->Mask(NEON2RegMiscFPMask)) {
    case NEON_FABS:
    case NEON_FNEG:
    case NEON_FSQRT:
    case NEON_FCVTL:
    case NEON_FCVTN:
    case NEON_FCVTXN:
    case NEON_FRINTI:
    case NEON_FRINTX:
    case NEON_FRINTA:
    case NEON_FRINTM:
    case NEON_FRINTN:
    case NEON_FRINTP:
    case NEON_FRINTZ:
    case NEON_FCVTNS:
    case NEON_FCVTNU:
    case NEON_FCVTPS:
    case NEON_FCVTPU:
    case NEON_FCVTMS:
    case NEON_FCVTMU:
    case NEON_FCVTZS:
    case NEON_FCVTZU:
    case NEON_FCVTAS:
    case NEON_FCVTAU:
    case NEON_SCVTF:
    case NEON_UCVTF:
    case NEON_FRSQRTE:
    case NEON_FRECPE:
    case NEON_FCMGT_zero:
    case NEON_FCMGE_zero:
    case NEON_FCMEQ_zero:
    case NEON_FCMLE_zero:
    case NEON_FCMLT_zero:
      scope.Record(CPUFeatures::kFP);
      return;
    case NEON_FRINT32X:
    case NEON_FRINT32Z:
    case NEON_FRINT64X:
    case NEON_FRINT64Z:
      scope.Record(CPUFeatures::kFP, CPUFeatures::kFrintToFixedSizedInt);
      return;
    default:
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEON2RegMiscFP16(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEONHalf.
  scope.Record(CPUFeatures::kFP, CPUFeatures::kNEON, CPUFeatures::kNEONHalf);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEON3Different(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  if (form_hash_ == "pmull_asimddiff_l"_h) {
    if (instr->GetNEONSize() == 3) {
      // Source is 1D or 2D, destination is 1Q.
      scope.Record(CPUFeatures::kPmull1Q);
    }
  }
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEON3Same(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  if (instr->Mask(NEON3SameFPFMask) == NEON3SameFPFixed) {
    scope.Record(CPUFeatures::kFP);
  }
  switch (instr->Mask(NEON3SameFHMMask)) {
    case NEON_FMLAL:
    case NEON_FMLAL2:
    case NEON_FMLSL:
    case NEON_FMLSL2:
      scope.Record(CPUFeatures::kFP, CPUFeatures::kNEONHalf, CPUFeatures::kFHM);
      return;
    default:
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEON3SameExtra(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  if ((instr->Mask(NEON3SameExtraFCMLAMask) == NEON_FCMLA) ||
      (instr->Mask(NEON3SameExtraFCADDMask) == NEON_FCADD)) {
    scope.Record(CPUFeatures::kFP, CPUFeatures::kFcma);
    if (instr->GetNEONSize() == 1) scope.Record(CPUFeatures::kNEONHalf);
  } else {
    switch (instr->Mask(NEON3SameExtraMask)) {
      case NEON_SDOT:
      case NEON_UDOT:
        scope.Record(CPUFeatures::kDotProduct);
        return;
      case NEON_SQRDMLAH:
      case NEON_SQRDMLSH:
        scope.Record(CPUFeatures::kRDM);
        return;
      default:
        // No additional features.
        return;
    }
  }
}

void CPUFeaturesAuditor::VisitNEON3SameFP16(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON FP16 support.
  scope.Record(CPUFeatures::kFP, CPUFeatures::kNEON, CPUFeatures::kNEONHalf);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONAcrossLanes(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  if (instr->Mask(NEONAcrossLanesFP16FMask) == NEONAcrossLanesFP16Fixed) {
    // FMAXV_H, FMINV_H, FMAXNMV_H, FMINNMV_H
    scope.Record(CPUFeatures::kFP, CPUFeatures::kNEONHalf);
  } else if (instr->Mask(NEONAcrossLanesFPFMask) == NEONAcrossLanesFPFixed) {
    // FMAXV, FMINV, FMAXNMV, FMINNMV
    scope.Record(CPUFeatures::kFP);
  }
}

void CPUFeaturesAuditor::VisitNEONByIndexedElement(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  switch (instr->Mask(NEONByIndexedElementMask)) {
    case NEON_SDOT_byelement:
    case NEON_UDOT_byelement:
      scope.Record(CPUFeatures::kDotProduct);
      return;
    case NEON_SQRDMLAH_byelement:
    case NEON_SQRDMLSH_byelement:
      scope.Record(CPUFeatures::kRDM);
      return;
    default:
      // Fall through to check other instructions.
      break;
  }
  switch (instr->Mask(NEONByIndexedElementFPLongMask)) {
    case NEON_FMLAL_H_byelement:
    case NEON_FMLAL2_H_byelement:
    case NEON_FMLSL_H_byelement:
    case NEON_FMLSL2_H_byelement:
      scope.Record(CPUFeatures::kFP, CPUFeatures::kNEONHalf, CPUFeatures::kFHM);
      return;
    default:
      // Fall through to check other instructions.
      break;
  }
  switch (instr->Mask(NEONByIndexedElementFPMask)) {
    case NEON_FMLA_H_byelement:
    case NEON_FMLS_H_byelement:
    case NEON_FMUL_H_byelement:
    case NEON_FMULX_H_byelement:
      scope.Record(CPUFeatures::kNEONHalf);
      VIXL_FALLTHROUGH();
    case NEON_FMLA_byelement:
    case NEON_FMLS_byelement:
    case NEON_FMUL_byelement:
    case NEON_FMULX_byelement:
      scope.Record(CPUFeatures::kFP);
      return;
    default:
      switch (instr->Mask(NEONByIndexedElementFPComplexMask)) {
        case NEON_FCMLA_byelement:
          scope.Record(CPUFeatures::kFP, CPUFeatures::kFcma);
          if (instr->GetNEONSize() == 1) scope.Record(CPUFeatures::kNEONHalf);
          return;
      }
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEONCopy(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONExtract(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONLoadStoreMultiStruct(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONLoadStoreMultiStructPostIndex(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONLoadStoreSingleStruct(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONLoadStoreSingleStructPostIndex(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONModifiedImmediate(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  if (instr->GetNEONCmode() == 0xf) {
    // FMOV (vector, immediate), double-, single- or half-precision.
    scope.Record(CPUFeatures::kFP);
    if (instr->ExtractBit(11)) scope.Record(CPUFeatures::kNEONHalf);
  }
}

void CPUFeaturesAuditor::VisitNEONPerm(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONScalar2RegMisc(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  switch (instr->Mask(NEONScalar2RegMiscFPMask)) {
    case NEON_FRECPE_scalar:
    case NEON_FRECPX_scalar:
    case NEON_FRSQRTE_scalar:
    case NEON_FCMGT_zero_scalar:
    case NEON_FCMGE_zero_scalar:
    case NEON_FCMEQ_zero_scalar:
    case NEON_FCMLE_zero_scalar:
    case NEON_FCMLT_zero_scalar:
    case NEON_SCVTF_scalar:
    case NEON_UCVTF_scalar:
    case NEON_FCVTNS_scalar:
    case NEON_FCVTNU_scalar:
    case NEON_FCVTPS_scalar:
    case NEON_FCVTPU_scalar:
    case NEON_FCVTMS_scalar:
    case NEON_FCVTMU_scalar:
    case NEON_FCVTZS_scalar:
    case NEON_FCVTZU_scalar:
    case NEON_FCVTAS_scalar:
    case NEON_FCVTAU_scalar:
    case NEON_FCVTXN_scalar:
      scope.Record(CPUFeatures::kFP);
      return;
    default:
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEONScalar2RegMiscFP16(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEONHalf.
  scope.Record(CPUFeatures::kFP, CPUFeatures::kNEON, CPUFeatures::kNEONHalf);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONScalar3Diff(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONScalar3Same(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  if (instr->Mask(NEONScalar3SameFPFMask) == NEONScalar3SameFPFixed) {
    scope.Record(CPUFeatures::kFP);
  }
}

void CPUFeaturesAuditor::VisitNEONScalar3SameExtra(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON and RDM.
  scope.Record(CPUFeatures::kNEON, CPUFeatures::kRDM);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONScalar3SameFP16(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEONHalf.
  scope.Record(CPUFeatures::kFP, CPUFeatures::kNEON, CPUFeatures::kNEONHalf);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONScalarByIndexedElement(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  switch (instr->Mask(NEONScalarByIndexedElementMask)) {
    case NEON_SQRDMLAH_byelement_scalar:
    case NEON_SQRDMLSH_byelement_scalar:
      scope.Record(CPUFeatures::kRDM);
      return;
    default:
      switch (instr->Mask(NEONScalarByIndexedElementFPMask)) {
        case NEON_FMLA_H_byelement_scalar:
        case NEON_FMLS_H_byelement_scalar:
        case NEON_FMUL_H_byelement_scalar:
        case NEON_FMULX_H_byelement_scalar:
          scope.Record(CPUFeatures::kNEONHalf);
          VIXL_FALLTHROUGH();
        case NEON_FMLA_byelement_scalar:
        case NEON_FMLS_byelement_scalar:
        case NEON_FMUL_byelement_scalar:
        case NEON_FMULX_byelement_scalar:
          scope.Record(CPUFeatures::kFP);
          return;
      }
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEONScalarCopy(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitNEONScalarPairwise(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  switch (instr->Mask(NEONScalarPairwiseMask)) {
    case NEON_FMAXNMP_h_scalar:
    case NEON_FADDP_h_scalar:
    case NEON_FMAXP_h_scalar:
    case NEON_FMINNMP_h_scalar:
    case NEON_FMINP_h_scalar:
      scope.Record(CPUFeatures::kNEONHalf);
      VIXL_FALLTHROUGH();
    case NEON_FADDP_scalar:
    case NEON_FMAXP_scalar:
    case NEON_FMAXNMP_scalar:
    case NEON_FMINP_scalar:
    case NEON_FMINNMP_scalar:
      scope.Record(CPUFeatures::kFP);
      return;
    default:
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEONScalarShiftImmediate(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  switch (instr->Mask(NEONScalarShiftImmediateMask)) {
    case NEON_FCVTZS_imm_scalar:
    case NEON_FCVTZU_imm_scalar:
    case NEON_SCVTF_imm_scalar:
    case NEON_UCVTF_imm_scalar:
      scope.Record(CPUFeatures::kFP);
      // If immh is 0b001x then the data type is FP16, and requires kNEONHalf.
      if ((instr->GetImmNEONImmh() & 0xe) == 0x2) {
        scope.Record(CPUFeatures::kNEONHalf);
      }
      return;
    default:
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEONShiftImmediate(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  switch (instr->Mask(NEONShiftImmediateMask)) {
    case NEON_SCVTF_imm:
    case NEON_UCVTF_imm:
    case NEON_FCVTZS_imm:
    case NEON_FCVTZU_imm:
      scope.Record(CPUFeatures::kFP);
      // If immh is 0b001x then the data type is FP16, and requires kNEONHalf.
      if ((instr->GetImmNEONImmh() & 0xe) == 0x2) {
        scope.Record(CPUFeatures::kNEONHalf);
      }
      return;
    default:
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitNEONTable(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  // All of these instructions require NEON.
  scope.Record(CPUFeatures::kNEON);
  USE(instr);
}

void CPUFeaturesAuditor::VisitPCRelAddressing(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

// Most SVE visitors require only SVE.
#define VIXL_SIMPLE_SVE_VISITOR_LIST(V)                          \
  V(SVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets)           \
  V(SVE32BitGatherLoad_VectorPlusImm)                            \
  V(SVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsets)    \
  V(SVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsets)        \
  V(SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsets)         \
  V(SVE32BitGatherPrefetch_VectorPlusImm)                        \
  V(SVE32BitScatterStore_ScalarPlus32BitScaledOffsets)           \
  V(SVE32BitScatterStore_ScalarPlus32BitUnscaledOffsets)         \
  V(SVE32BitScatterStore_VectorPlusImm)                          \
  V(SVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets)     \
  V(SVE64BitGatherLoad_ScalarPlus64BitScaledOffsets)             \
  V(SVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets)           \
  V(SVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets)   \
  V(SVE64BitGatherLoad_VectorPlusImm)                            \
  V(SVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsets)         \
  V(SVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsets) \
  V(SVE64BitGatherPrefetch_VectorPlusImm)                        \
  V(SVE64BitScatterStore_ScalarPlus64BitScaledOffsets)           \
  V(SVE64BitScatterStore_ScalarPlus64BitUnscaledOffsets)         \
  V(SVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsets)   \
  V(SVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsets) \
  V(SVE64BitScatterStore_VectorPlusImm)                          \
  V(SVEAddressGeneration)                                        \
  V(SVEBitwiseLogicalUnpredicated)                               \
  V(SVEBitwiseShiftUnpredicated)                                 \
  V(SVEFFRInitialise)                                            \
  V(SVEFFRWriteFromPredicate)                                    \
  V(SVEFPAccumulatingReduction)                                  \
  V(SVEFPArithmeticUnpredicated)                                 \
  V(SVEFPCompareVectors)                                         \
  V(SVEFPCompareWithZero)                                        \
  V(SVEFPComplexAddition)                                        \
  V(SVEFPComplexMulAdd)                                          \
  V(SVEFPComplexMulAddIndex)                                     \
  V(SVEFPFastReduction)                                          \
  V(SVEFPMulIndex)                                               \
  V(SVEFPMulAdd)                                                 \
  V(SVEFPMulAddIndex)                                            \
  V(SVEFPUnaryOpUnpredicated)                                    \
  V(SVEIncDecByPredicateCount)                                   \
  V(SVEIndexGeneration)                                          \
  V(SVEIntArithmeticUnpredicated)                                \
  V(SVEIntCompareSignedImm)                                      \
  V(SVEIntCompareUnsignedImm)                                    \
  V(SVEIntCompareVectors)                                        \
  V(SVEIntMulAddPredicated)                                      \
  V(SVEIntMulAddUnpredicated)                                    \
  V(SVEIntReduction)                                             \
  V(SVEIntUnaryArithmeticPredicated)                             \
  V(SVEMovprfx)                                                  \
  V(SVEMulIndex)                                                 \
  V(SVEPermuteVectorExtract)                                     \
  V(SVEPermuteVectorInterleaving)                                \
  V(SVEPredicateCount)                                           \
  V(SVEPredicateLogical)                                         \
  V(SVEPropagateBreak)                                           \
  V(SVEStackFrameAdjustment)                                     \
  V(SVEStackFrameSize)                                           \
  V(SVEVectorSelect)                                             \
  V(SVEBitwiseLogical_Predicated)                                \
  V(SVEBitwiseLogicalWithImm_Unpredicated)                       \
  V(SVEBitwiseShiftByImm_Predicated)                             \
  V(SVEBitwiseShiftByVector_Predicated)                          \
  V(SVEBitwiseShiftByWideElements_Predicated)                    \
  V(SVEBroadcastBitmaskImm)                                      \
  V(SVEBroadcastFPImm_Unpredicated)                              \
  V(SVEBroadcastGeneralRegister)                                 \
  V(SVEBroadcastIndexElement)                                    \
  V(SVEBroadcastIntImm_Unpredicated)                             \
  V(SVECompressActiveElements)                                   \
  V(SVEConditionallyBroadcastElementToVector)                    \
  V(SVEConditionallyExtractElementToSIMDFPScalar)                \
  V(SVEConditionallyExtractElementToGeneralRegister)             \
  V(SVEConditionallyTerminateScalars)                            \
  V(SVEConstructivePrefix_Unpredicated)                          \
  V(SVEContiguousFirstFaultLoad_ScalarPlusScalar)                \
  V(SVEContiguousLoad_ScalarPlusImm)                             \
  V(SVEContiguousLoad_ScalarPlusScalar)                          \
  V(SVEContiguousNonFaultLoad_ScalarPlusImm)                     \
  V(SVEContiguousNonTemporalLoad_ScalarPlusImm)                  \
  V(SVEContiguousNonTemporalLoad_ScalarPlusScalar)               \
  V(SVEContiguousNonTemporalStore_ScalarPlusImm)                 \
  V(SVEContiguousNonTemporalStore_ScalarPlusScalar)              \
  V(SVEContiguousPrefetch_ScalarPlusImm)                         \
  V(SVEContiguousPrefetch_ScalarPlusScalar)                      \
  V(SVEContiguousStore_ScalarPlusImm)                            \
  V(SVEContiguousStore_ScalarPlusScalar)                         \
  V(SVECopySIMDFPScalarRegisterToVector_Predicated)              \
  V(SVECopyFPImm_Predicated)                                     \
  V(SVECopyGeneralRegisterToVector_Predicated)                   \
  V(SVECopyIntImm_Predicated)                                    \
  V(SVEElementCount)                                             \
  V(SVEExtractElementToSIMDFPScalarRegister)                     \
  V(SVEExtractElementToGeneralRegister)                          \
  V(SVEFPArithmetic_Predicated)                                  \
  V(SVEFPArithmeticWithImm_Predicated)                           \
  V(SVEFPConvertPrecision)                                       \
  V(SVEFPConvertToInt)                                           \
  V(SVEFPExponentialAccelerator)                                 \
  V(SVEFPRoundToIntegralValue)                                   \
  V(SVEFPTrigMulAddCoefficient)                                  \
  V(SVEFPTrigSelectCoefficient)                                  \
  V(SVEFPUnaryOp)                                                \
  V(SVEIncDecRegisterByElementCount)                             \
  V(SVEIncDecVectorByElementCount)                               \
  V(SVEInsertSIMDFPScalarRegister)                               \
  V(SVEInsertGeneralRegister)                                    \
  V(SVEIntAddSubtractImm_Unpredicated)                           \
  V(SVEIntAddSubtractVectors_Predicated)                         \
  V(SVEIntCompareScalarCountAndLimit)                            \
  V(SVEIntConvertToFP)                                           \
  V(SVEIntDivideVectors_Predicated)                              \
  V(SVEIntMinMaxImm_Unpredicated)                                \
  V(SVEIntMinMaxDifference_Predicated)                           \
  V(SVEIntMulImm_Unpredicated)                                   \
  V(SVEIntMulVectors_Predicated)                                 \
  V(SVELoadAndBroadcastElement)                                  \
  V(SVELoadAndBroadcastQOWord_ScalarPlusImm)                     \
  V(SVELoadAndBroadcastQOWord_ScalarPlusScalar)                  \
  V(SVELoadMultipleStructures_ScalarPlusImm)                     \
  V(SVELoadMultipleStructures_ScalarPlusScalar)                  \
  V(SVELoadPredicateRegister)                                    \
  V(SVELoadVectorRegister)                                       \
  V(SVEPartitionBreakCondition)                                  \
  V(SVEPermutePredicateElements)                                 \
  V(SVEPredicateFirstActive)                                     \
  V(SVEPredicateInitialize)                                      \
  V(SVEPredicateNextActive)                                      \
  V(SVEPredicateReadFromFFR_Predicated)                          \
  V(SVEPredicateReadFromFFR_Unpredicated)                        \
  V(SVEPredicateTest)                                            \
  V(SVEPredicateZero)                                            \
  V(SVEPropagateBreakToNextPartition)                            \
  V(SVEReversePredicateElements)                                 \
  V(SVEReverseVectorElements)                                    \
  V(SVEReverseWithinElements)                                    \
  V(SVESaturatingIncDecRegisterByElementCount)                   \
  V(SVESaturatingIncDecVectorByElementCount)                     \
  V(SVEStoreMultipleStructures_ScalarPlusImm)                    \
  V(SVEStoreMultipleStructures_ScalarPlusScalar)                 \
  V(SVEStorePredicateRegister)                                   \
  V(SVEStoreVectorRegister)                                      \
  V(SVETableLookup)                                              \
  V(SVEUnpackPredicateElements)                                  \
  V(SVEUnpackVectorElements)                                     \
  V(SVEVectorSplice)

#define VIXL_DEFINE_SIMPLE_SVE_VISITOR(NAME)                       \
  void CPUFeaturesAuditor::Visit##NAME(const Instruction* instr) { \
    RecordInstructionFeaturesScope scope(this);                    \
    scope.Record(CPUFeatures::kSVE);                               \
    USE(instr);                                                    \
  }
VIXL_SIMPLE_SVE_VISITOR_LIST(VIXL_DEFINE_SIMPLE_SVE_VISITOR)
#undef VIXL_DEFINE_SIMPLE_SVE_VISITOR
#undef VIXL_SIMPLE_SVE_VISITOR_LIST

void CPUFeaturesAuditor::VisitSystem(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);

  CPUFeatures required;
  switch (form_hash_) {
    case "pacib1716_hi_hints"_h:
    case "pacia1716_hi_hints"_h:
    case "pacibsp_hi_hints"_h:
    case "paciasp_hi_hints"_h:
    case "pacibz_hi_hints"_h:
    case "paciaz_hi_hints"_h:
    case "autib1716_hi_hints"_h:
    case "autia1716_hi_hints"_h:
    case "autibsp_hi_hints"_h:
    case "autiasp_hi_hints"_h:
    case "autibz_hi_hints"_h:
    case "autiaz_hi_hints"_h:
    case "xpaclri_hi_hints"_h:
      required.Combine(CPUFeatures::kPAuth);
      break;
    case "esb_hi_hints"_h:
      required.Combine(CPUFeatures::kRAS);
      break;
    case "bti_hb_hints"_h:
      required.Combine(CPUFeatures::kBTI);
      break;
  }

  // The instructions above are all HINTs and behave as NOPs if the
  // corresponding features are not implemented, so we record the corresponding
  // features only if they are available.
  if (available_.Has(required)) scope.Record(required);

  switch (form_hash_) {
    case "cfinv_m_pstate"_h:
      scope.Record(CPUFeatures::kFlagM);
      break;
    case "axflag_m_pstate"_h:
    case "xaflag_m_pstate"_h:
      scope.Record(CPUFeatures::kAXFlag);
      break;
    case "mrs_rs_systemmove"_h:
      switch (instr->GetImmSystemRegister()) {
        case RNDR:
        case RNDRRS:
          scope.Record(CPUFeatures::kRNG);
          break;
      }
      break;
    case "sys_cr_systeminstrs"_h:
      switch (instr->GetSysOp()) {
        // DC instruction variants.
        case CGVAC:
        case CGDVAC:
        case CGVAP:
        case CGDVAP:
        case CIGVAC:
        case CIGDVAC:
        case GVA:
        case GZVA:
          scope.Record(CPUFeatures::kMTE);
          break;
        case CVAP:
          scope.Record(CPUFeatures::kDCPoP);
          break;
        case CVADP:
          scope.Record(CPUFeatures::kDCCVADP);
          break;
        case IVAU:
        case CVAC:
        case CVAU:
        case CIVAC:
        case ZVA:
          // No special CPU features.
          break;
        case GCSPUSHM:
        case GCSSS1:
          scope.Record(CPUFeatures::kGCS);
          break;
      }
      break;
    case "sysl_rc_systeminstrs"_h:
      switch (instr->GetSysOp()) {
        case GCSPOPM:
        case GCSSS2:
          scope.Record(CPUFeatures::kGCS);
          break;
      }
      break;
  }
}

void CPUFeaturesAuditor::VisitTestBranch(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitUnallocated(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitUnconditionalBranch(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitUnconditionalBranchToRegister(
    const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  switch (instr->Mask(UnconditionalBranchToRegisterMask)) {
    case BRAAZ:
    case BRABZ:
    case BLRAAZ:
    case BLRABZ:
    case RETAA:
    case RETAB:
    case BRAA:
    case BRAB:
    case BLRAA:
    case BLRAB:
      scope.Record(CPUFeatures::kPAuth);
      return;
    default:
      // No additional features.
      return;
  }
}

void CPUFeaturesAuditor::VisitReserved(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::VisitUnimplemented(const Instruction* instr) {
  RecordInstructionFeaturesScope scope(this);
  USE(instr);
}

void CPUFeaturesAuditor::Visit(Metadata* metadata, const Instruction* instr) {
  VIXL_ASSERT(metadata->count("form") > 0);
  const std::string& form = (*metadata)["form"];
  form_hash_ = Hash(form.c_str());
  const FormToVisitorFnMap* fv = CPUFeaturesAuditor::GetFormToVisitorFnMap();
  FormToVisitorFnMap::const_iterator it = fv->find(form_hash_);
  if (it == fv->end()) {
    RecordInstructionFeaturesScope scope(this);
    std::map<uint32_t, const CPUFeatures> features = {
        {"adclb_z_zzz"_h, CPUFeatures::kSVE2},
        {"adclt_z_zzz"_h, CPUFeatures::kSVE2},
        {"addhnb_z_zz"_h, CPUFeatures::kSVE2},
        {"addhnt_z_zz"_h, CPUFeatures::kSVE2},
        {"addp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"bcax_z_zzz"_h, CPUFeatures::kSVE2},
        {"bdep_z_zz"_h,
         CPUFeatures(CPUFeatures::kSVE2, CPUFeatures::kSVEBitPerm)},
        {"bext_z_zz"_h,
         CPUFeatures(CPUFeatures::kSVE2, CPUFeatures::kSVEBitPerm)},
        {"bgrp_z_zz"_h,
         CPUFeatures(CPUFeatures::kSVE2, CPUFeatures::kSVEBitPerm)},
        {"bsl1n_z_zzz"_h, CPUFeatures::kSVE2},
        {"bsl2n_z_zzz"_h, CPUFeatures::kSVE2},
        {"bsl_z_zzz"_h, CPUFeatures::kSVE2},
        {"cadd_z_zz"_h, CPUFeatures::kSVE2},
        {"cdot_z_zzz"_h, CPUFeatures::kSVE2},
        {"cdot_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"cdot_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"cmla_z_zzz"_h, CPUFeatures::kSVE2},
        {"cmla_z_zzzi_h"_h, CPUFeatures::kSVE2},
        {"cmla_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"eor3_z_zzz"_h, CPUFeatures::kSVE2},
        {"eorbt_z_zz"_h, CPUFeatures::kSVE2},
        {"eortb_z_zz"_h, CPUFeatures::kSVE2},
        {"ext_z_zi_con"_h, CPUFeatures::kSVE2},
        {"faddp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"fcvtlt_z_p_z_h2s"_h, CPUFeatures::kSVE2},
        {"fcvtlt_z_p_z_s2d"_h, CPUFeatures::kSVE2},
        {"fcvtnt_z_p_z_d2s"_h, CPUFeatures::kSVE2},
        {"fcvtnt_z_p_z_s2h"_h, CPUFeatures::kSVE2},
        {"fcvtx_z_p_z_d2s"_h, CPUFeatures::kSVE2},
        {"fcvtxnt_z_p_z_d2s"_h, CPUFeatures::kSVE2},
        {"flogb_z_p_z"_h, CPUFeatures::kSVE2},
        {"fmaxnmp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"fmaxp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"fminnmp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"fminp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"fmlalb_z_zzz"_h, CPUFeatures::kSVE2},
        {"fmlalb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"fmlalt_z_zzz"_h, CPUFeatures::kSVE2},
        {"fmlalt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"fmlslb_z_zzz"_h, CPUFeatures::kSVE2},
        {"fmlslb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"fmlslt_z_zzz"_h, CPUFeatures::kSVE2},
        {"fmlslt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"histcnt_z_p_zz"_h, CPUFeatures::kSVE2},
        {"histseg_z_zz"_h, CPUFeatures::kSVE2},
        {"ldnt1b_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1b_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1d_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1h_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1h_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1sb_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1sb_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1sh_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1sh_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1sw_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1w_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"ldnt1w_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"match_p_p_zz"_h, CPUFeatures::kSVE2},
        {"mla_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"mla_z_zzzi_h"_h, CPUFeatures::kSVE2},
        {"mla_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"mls_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"mls_z_zzzi_h"_h, CPUFeatures::kSVE2},
        {"mls_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"mul_z_zz"_h, CPUFeatures::kSVE2},
        {"mul_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"mul_z_zzi_h"_h, CPUFeatures::kSVE2},
        {"mul_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"nbsl_z_zzz"_h, CPUFeatures::kSVE2},
        {"nmatch_p_p_zz"_h, CPUFeatures::kSVE2},
        {"pmul_z_zz"_h, CPUFeatures::kSVE2},
        {"pmullb_z_zz"_h, CPUFeatures::kSVE2},
        {"pmullt_z_zz"_h, CPUFeatures::kSVE2},
        {"raddhnb_z_zz"_h, CPUFeatures::kSVE2},
        {"raddhnt_z_zz"_h, CPUFeatures::kSVE2},
        {"rshrnb_z_zi"_h, CPUFeatures::kSVE2},
        {"rshrnt_z_zi"_h, CPUFeatures::kSVE2},
        {"rsubhnb_z_zz"_h, CPUFeatures::kSVE2},
        {"rsubhnt_z_zz"_h, CPUFeatures::kSVE2},
        {"saba_z_zzz"_h, CPUFeatures::kSVE2},
        {"sabalb_z_zzz"_h, CPUFeatures::kSVE2},
        {"sabalt_z_zzz"_h, CPUFeatures::kSVE2},
        {"sabdlb_z_zz"_h, CPUFeatures::kSVE2},
        {"sabdlt_z_zz"_h, CPUFeatures::kSVE2},
        {"sadalp_z_p_z"_h, CPUFeatures::kSVE2},
        {"saddlb_z_zz"_h, CPUFeatures::kSVE2},
        {"saddlbt_z_zz"_h, CPUFeatures::kSVE2},
        {"saddlt_z_zz"_h, CPUFeatures::kSVE2},
        {"saddwb_z_zz"_h, CPUFeatures::kSVE2},
        {"saddwt_z_zz"_h, CPUFeatures::kSVE2},
        {"sbclb_z_zzz"_h, CPUFeatures::kSVE2},
        {"sbclt_z_zzz"_h, CPUFeatures::kSVE2},
        {"shadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"shrnb_z_zi"_h, CPUFeatures::kSVE2},
        {"shrnt_z_zi"_h, CPUFeatures::kSVE2},
        {"shsub_z_p_zz"_h, CPUFeatures::kSVE2},
        {"shsubr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sli_z_zzi"_h, CPUFeatures::kSVE2},
        {"smaxp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sminp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"smlalb_z_zzz"_h, CPUFeatures::kSVE2},
        {"smlalb_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"smlalb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"smlalt_z_zzz"_h, CPUFeatures::kSVE2},
        {"smlalt_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"smlalt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"smlslb_z_zzz"_h, CPUFeatures::kSVE2},
        {"smlslb_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"smlslb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"smlslt_z_zzz"_h, CPUFeatures::kSVE2},
        {"smlslt_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"smlslt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"smulh_z_zz"_h, CPUFeatures::kSVE2},
        {"smullb_z_zz"_h, CPUFeatures::kSVE2},
        {"smullb_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"smullb_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"smullt_z_zz"_h, CPUFeatures::kSVE2},
        {"smullt_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"smullt_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"splice_z_p_zz_con"_h, CPUFeatures::kSVE2},
        {"sqabs_z_p_z"_h, CPUFeatures::kSVE2},
        {"sqadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sqcadd_z_zz"_h, CPUFeatures::kSVE2},
        {"sqdmlalb_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqdmlalb_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"sqdmlalb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"sqdmlalbt_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqdmlalt_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqdmlalt_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"sqdmlalt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"sqdmlslb_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqdmlslb_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"sqdmlslb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"sqdmlslbt_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqdmlslt_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqdmlslt_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"sqdmlslt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"sqdmulh_z_zz"_h, CPUFeatures::kSVE2},
        {"sqdmulh_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"sqdmulh_z_zzi_h"_h, CPUFeatures::kSVE2},
        {"sqdmulh_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"sqdmullb_z_zz"_h, CPUFeatures::kSVE2},
        {"sqdmullb_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"sqdmullb_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"sqdmullt_z_zz"_h, CPUFeatures::kSVE2},
        {"sqdmullt_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"sqdmullt_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"sqneg_z_p_z"_h, CPUFeatures::kSVE2},
        {"sqrdcmlah_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqrdcmlah_z_zzzi_h"_h, CPUFeatures::kSVE2},
        {"sqrdcmlah_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"sqrdmlah_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqrdmlah_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"sqrdmlah_z_zzzi_h"_h, CPUFeatures::kSVE2},
        {"sqrdmlah_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"sqrdmlsh_z_zzz"_h, CPUFeatures::kSVE2},
        {"sqrdmlsh_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"sqrdmlsh_z_zzzi_h"_h, CPUFeatures::kSVE2},
        {"sqrdmlsh_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"sqrdmulh_z_zz"_h, CPUFeatures::kSVE2},
        {"sqrdmulh_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"sqrdmulh_z_zzi_h"_h, CPUFeatures::kSVE2},
        {"sqrdmulh_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"sqrshl_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sqrshlr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sqrshrnb_z_zi"_h, CPUFeatures::kSVE2},
        {"sqrshrnt_z_zi"_h, CPUFeatures::kSVE2},
        {"sqrshrunb_z_zi"_h, CPUFeatures::kSVE2},
        {"sqrshrunt_z_zi"_h, CPUFeatures::kSVE2},
        {"sqshl_z_p_zi"_h, CPUFeatures::kSVE2},
        {"sqshl_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sqshlr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sqshlu_z_p_zi"_h, CPUFeatures::kSVE2},
        {"sqshrnb_z_zi"_h, CPUFeatures::kSVE2},
        {"sqshrnt_z_zi"_h, CPUFeatures::kSVE2},
        {"sqshrunb_z_zi"_h, CPUFeatures::kSVE2},
        {"sqshrunt_z_zi"_h, CPUFeatures::kSVE2},
        {"sqsub_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sqsubr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sqxtnb_z_zz"_h, CPUFeatures::kSVE2},
        {"sqxtnt_z_zz"_h, CPUFeatures::kSVE2},
        {"sqxtunb_z_zz"_h, CPUFeatures::kSVE2},
        {"sqxtunt_z_zz"_h, CPUFeatures::kSVE2},
        {"srhadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"sri_z_zzi"_h, CPUFeatures::kSVE2},
        {"srshl_z_p_zz"_h, CPUFeatures::kSVE2},
        {"srshlr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"srshr_z_p_zi"_h, CPUFeatures::kSVE2},
        {"srsra_z_zi"_h, CPUFeatures::kSVE2},
        {"sshllb_z_zi"_h, CPUFeatures::kSVE2},
        {"sshllt_z_zi"_h, CPUFeatures::kSVE2},
        {"ssra_z_zi"_h, CPUFeatures::kSVE2},
        {"ssublb_z_zz"_h, CPUFeatures::kSVE2},
        {"ssublbt_z_zz"_h, CPUFeatures::kSVE2},
        {"ssublt_z_zz"_h, CPUFeatures::kSVE2},
        {"ssubltb_z_zz"_h, CPUFeatures::kSVE2},
        {"ssubwb_z_zz"_h, CPUFeatures::kSVE2},
        {"ssubwt_z_zz"_h, CPUFeatures::kSVE2},
        {"stnt1b_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"stnt1b_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"stnt1d_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"stnt1h_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"stnt1h_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"stnt1w_z_p_ar_d_64_unscaled"_h, CPUFeatures::kSVE2},
        {"stnt1w_z_p_ar_s_x32_unscaled"_h, CPUFeatures::kSVE2},
        {"subhnb_z_zz"_h, CPUFeatures::kSVE2},
        {"subhnt_z_zz"_h, CPUFeatures::kSVE2},
        {"suqadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"tbl_z_zz_2"_h, CPUFeatures::kSVE2},
        {"tbx_z_zz"_h, CPUFeatures::kSVE2},
        {"uaba_z_zzz"_h, CPUFeatures::kSVE2},
        {"uabalb_z_zzz"_h, CPUFeatures::kSVE2},
        {"uabalt_z_zzz"_h, CPUFeatures::kSVE2},
        {"uabdlb_z_zz"_h, CPUFeatures::kSVE2},
        {"uabdlt_z_zz"_h, CPUFeatures::kSVE2},
        {"uadalp_z_p_z"_h, CPUFeatures::kSVE2},
        {"uaddlb_z_zz"_h, CPUFeatures::kSVE2},
        {"uaddlt_z_zz"_h, CPUFeatures::kSVE2},
        {"uaddwb_z_zz"_h, CPUFeatures::kSVE2},
        {"uaddwt_z_zz"_h, CPUFeatures::kSVE2},
        {"uhadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uhsub_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uhsubr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"umaxp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uminp_z_p_zz"_h, CPUFeatures::kSVE2},
        {"umlalb_z_zzz"_h, CPUFeatures::kSVE2},
        {"umlalb_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"umlalb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"umlalt_z_zzz"_h, CPUFeatures::kSVE2},
        {"umlalt_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"umlalt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"umlslb_z_zzz"_h, CPUFeatures::kSVE2},
        {"umlslb_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"umlslb_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"umlslt_z_zzz"_h, CPUFeatures::kSVE2},
        {"umlslt_z_zzzi_d"_h, CPUFeatures::kSVE2},
        {"umlslt_z_zzzi_s"_h, CPUFeatures::kSVE2},
        {"umulh_z_zz"_h, CPUFeatures::kSVE2},
        {"umullb_z_zz"_h, CPUFeatures::kSVE2},
        {"umullb_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"umullb_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"umullt_z_zz"_h, CPUFeatures::kSVE2},
        {"umullt_z_zzi_d"_h, CPUFeatures::kSVE2},
        {"umullt_z_zzi_s"_h, CPUFeatures::kSVE2},
        {"uqadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uqrshl_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uqrshlr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uqrshrnb_z_zi"_h, CPUFeatures::kSVE2},
        {"uqrshrnt_z_zi"_h, CPUFeatures::kSVE2},
        {"uqshl_z_p_zi"_h, CPUFeatures::kSVE2},
        {"uqshl_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uqshlr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uqshrnb_z_zi"_h, CPUFeatures::kSVE2},
        {"uqshrnt_z_zi"_h, CPUFeatures::kSVE2},
        {"uqsub_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uqsubr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"uqxtnb_z_zz"_h, CPUFeatures::kSVE2},
        {"uqxtnt_z_zz"_h, CPUFeatures::kSVE2},
        {"urecpe_z_p_z"_h, CPUFeatures::kSVE2},
        {"urhadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"urshl_z_p_zz"_h, CPUFeatures::kSVE2},
        {"urshlr_z_p_zz"_h, CPUFeatures::kSVE2},
        {"urshr_z_p_zi"_h, CPUFeatures::kSVE2},
        {"ursqrte_z_p_z"_h, CPUFeatures::kSVE2},
        {"ursra_z_zi"_h, CPUFeatures::kSVE2},
        {"ushllb_z_zi"_h, CPUFeatures::kSVE2},
        {"ushllt_z_zi"_h, CPUFeatures::kSVE2},
        {"usqadd_z_p_zz"_h, CPUFeatures::kSVE2},
        {"usra_z_zi"_h, CPUFeatures::kSVE2},
        {"usublb_z_zz"_h, CPUFeatures::kSVE2},
        {"usublt_z_zz"_h, CPUFeatures::kSVE2},
        {"usubwb_z_zz"_h, CPUFeatures::kSVE2},
        {"usubwt_z_zz"_h, CPUFeatures::kSVE2},
        {"whilege_p_p_rr"_h, CPUFeatures::kSVE2},
        {"whilegt_p_p_rr"_h, CPUFeatures::kSVE2},
        {"whilehi_p_p_rr"_h, CPUFeatures::kSVE2},
        {"whilehs_p_p_rr"_h, CPUFeatures::kSVE2},
        {"whilerw_p_rr"_h, CPUFeatures::kSVE2},
        {"whilewr_p_rr"_h, CPUFeatures::kSVE2},
        {"xar_z_zzi"_h, CPUFeatures::kSVE2},
        {"smmla_z_zzz"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEI8MM)},
        {"ummla_z_zzz"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEI8MM)},
        {"usmmla_z_zzz"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEI8MM)},
        {"fmmla_z_zzz_s"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF32MM)},
        {"fmmla_z_zzz_d"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"smmla_asimdsame2_g"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kI8MM)},
        {"ummla_asimdsame2_g"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kI8MM)},
        {"usmmla_asimdsame2_g"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kI8MM)},
        {"ld1row_z_p_bi_u32"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"ld1row_z_p_br_contiguous"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"ld1rod_z_p_bi_u64"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"ld1rod_z_p_br_contiguous"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"ld1rob_z_p_bi_u8"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"ld1rob_z_p_br_contiguous"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"ld1roh_z_p_bi_u16"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"ld1roh_z_p_br_contiguous"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEF64MM)},
        {"usdot_asimdsame2_d"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kI8MM)},
        {"sudot_asimdelem_d"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kI8MM)},
        {"usdot_asimdelem_d"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kI8MM)},
        {"usdot_z_zzz_s"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEI8MM)},
        {"usdot_z_zzzi_s"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEI8MM)},
        {"sudot_z_zzzi_s"_h,
         CPUFeatures(CPUFeatures::kSVE, CPUFeatures::kSVEI8MM)},
        {"addg_64_addsub_immtags"_h, CPUFeatures::kMTE},
        {"gmi_64g_dp_2src"_h, CPUFeatures::kMTE},
        {"irg_64i_dp_2src"_h, CPUFeatures::kMTE},
        {"ldg_64loffset_ldsttags"_h, CPUFeatures::kMTE},
        {"st2g_64soffset_ldsttags"_h, CPUFeatures::kMTE},
        {"st2g_64spost_ldsttags"_h, CPUFeatures::kMTE},
        {"st2g_64spre_ldsttags"_h, CPUFeatures::kMTE},
        {"stgp_64_ldstpair_off"_h, CPUFeatures::kMTE},
        {"stgp_64_ldstpair_post"_h, CPUFeatures::kMTE},
        {"stgp_64_ldstpair_pre"_h, CPUFeatures::kMTE},
        {"stg_64soffset_ldsttags"_h, CPUFeatures::kMTE},
        {"stg_64spost_ldsttags"_h, CPUFeatures::kMTE},
        {"stg_64spre_ldsttags"_h, CPUFeatures::kMTE},
        {"stz2g_64soffset_ldsttags"_h, CPUFeatures::kMTE},
        {"stz2g_64spost_ldsttags"_h, CPUFeatures::kMTE},
        {"stz2g_64spre_ldsttags"_h, CPUFeatures::kMTE},
        {"stzg_64soffset_ldsttags"_h, CPUFeatures::kMTE},
        {"stzg_64spost_ldsttags"_h, CPUFeatures::kMTE},
        {"stzg_64spre_ldsttags"_h, CPUFeatures::kMTE},
        {"subg_64_addsub_immtags"_h, CPUFeatures::kMTE},
        {"subps_64s_dp_2src"_h, CPUFeatures::kMTE},
        {"subp_64s_dp_2src"_h, CPUFeatures::kMTE},
        {"cpyen_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyern_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyewn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpye_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfen_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfern_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfewn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfe_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfmn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfmrn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfmwn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfm_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfpn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfprn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfpwn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyfp_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpymn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpymrn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpymwn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpym_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpypn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyprn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpypwn_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"cpyp_cpy_memcms"_h, CPUFeatures::kMOPS},
        {"seten_set_memcms"_h, CPUFeatures::kMOPS},
        {"sete_set_memcms"_h, CPUFeatures::kMOPS},
        {"setgen_set_memcms"_h,
         CPUFeatures(CPUFeatures::kMOPS, CPUFeatures::kMTE)},
        {"setge_set_memcms"_h,
         CPUFeatures(CPUFeatures::kMOPS, CPUFeatures::kMTE)},
        {"setgmn_set_memcms"_h,
         CPUFeatures(CPUFeatures::kMOPS, CPUFeatures::kMTE)},
        {"setgm_set_memcms"_h,
         CPUFeatures(CPUFeatures::kMOPS, CPUFeatures::kMTE)},
        {"setgpn_set_memcms"_h,
         CPUFeatures(CPUFeatures::kMOPS, CPUFeatures::kMTE)},
        {"setgp_set_memcms"_h,
         CPUFeatures(CPUFeatures::kMOPS, CPUFeatures::kMTE)},
        {"setmn_set_memcms"_h, CPUFeatures::kMOPS},
        {"setm_set_memcms"_h, CPUFeatures::kMOPS},
        {"setpn_set_memcms"_h, CPUFeatures::kMOPS},
        {"setp_set_memcms"_h, CPUFeatures::kMOPS},
        {"abs_32_dp_1src"_h, CPUFeatures::kCSSC},
        {"abs_64_dp_1src"_h, CPUFeatures::kCSSC},
        {"cnt_32_dp_1src"_h, CPUFeatures::kCSSC},
        {"cnt_64_dp_1src"_h, CPUFeatures::kCSSC},
        {"ctz_32_dp_1src"_h, CPUFeatures::kCSSC},
        {"ctz_64_dp_1src"_h, CPUFeatures::kCSSC},
        {"smax_32_dp_2src"_h, CPUFeatures::kCSSC},
        {"smax_64_dp_2src"_h, CPUFeatures::kCSSC},
        {"smin_32_dp_2src"_h, CPUFeatures::kCSSC},
        {"smin_64_dp_2src"_h, CPUFeatures::kCSSC},
        {"umax_32_dp_2src"_h, CPUFeatures::kCSSC},
        {"umax_64_dp_2src"_h, CPUFeatures::kCSSC},
        {"umin_32_dp_2src"_h, CPUFeatures::kCSSC},
        {"umin_64_dp_2src"_h, CPUFeatures::kCSSC},
        {"smax_32_minmax_imm"_h, CPUFeatures::kCSSC},
        {"smax_64_minmax_imm"_h, CPUFeatures::kCSSC},
        {"smin_32_minmax_imm"_h, CPUFeatures::kCSSC},
        {"smin_64_minmax_imm"_h, CPUFeatures::kCSSC},
        {"umax_32u_minmax_imm"_h, CPUFeatures::kCSSC},
        {"umax_64u_minmax_imm"_h, CPUFeatures::kCSSC},
        {"umin_32u_minmax_imm"_h, CPUFeatures::kCSSC},
        {"umin_64u_minmax_imm"_h, CPUFeatures::kCSSC},
        {"bcax_vvv16_crypto4"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA3)},
        {"eor3_vvv16_crypto4"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA3)},
        {"rax1_vvv2_cryptosha512_3"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA3)},
        {"xar_vvv2_crypto3_imm6"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA3)},
        {"sha512h_qqv_cryptosha512_3"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA512)},
        {"sha512h2_qqv_cryptosha512_3"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA512)},
        {"sha512su0_vv2_cryptosha512_2"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA512)},
        {"sha512su1_vvv2_cryptosha512_3"_h,
         CPUFeatures(CPUFeatures::kNEON, CPUFeatures::kSHA512)},
        {"pmullb_z_zz_q"_h,
         CPUFeatures(CPUFeatures::kSVE2, CPUFeatures::kSVEPmull128)},
        {"pmullt_z_zz_q"_h,
         CPUFeatures(CPUFeatures::kSVE2, CPUFeatures::kSVEPmull128)},
    };

    if (features.count(form_hash_) > 0) {
      scope.Record(features[form_hash_]);
    }
  } else {
    (it->second)(this, instr);
  }
}

}  // namespace aarch64
}  // namespace vixl
