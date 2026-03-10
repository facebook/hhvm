// Copyright 2015, VIXL authors
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

#include "instructions-aarch64.h"

#include "assembler-aarch64.h"

namespace vixl {
namespace aarch64 {

static uint64_t RepeatBitsAcrossReg(unsigned reg_size,
                                    uint64_t value,
                                    unsigned width) {
  VIXL_ASSERT((width == 2) || (width == 4) || (width == 8) || (width == 16) ||
              (width == 32));
  VIXL_ASSERT((reg_size == kBRegSize) || (reg_size == kHRegSize) ||
              (reg_size == kSRegSize) || (reg_size == kDRegSize));
  uint64_t result = value & ((UINT64_C(1) << width) - 1);
  for (unsigned i = width; i < reg_size; i *= 2) {
    result |= (result << i);
  }
  return result;
}

bool Instruction::CanTakeSVEMovprfx(const char* form,
                                    const Instruction* movprfx) const {
  return CanTakeSVEMovprfx(Hash(form), movprfx);
}

bool Instruction::CanTakeSVEMovprfx(uint32_t form_hash,
                                    const Instruction* movprfx) const {
  bool movprfx_is_predicated = movprfx->Mask(SVEMovprfxMask) == MOVPRFX_z_p_z;
  bool movprfx_is_unpredicated =
      movprfx->Mask(SVEConstructivePrefix_UnpredicatedMask) == MOVPRFX_z_z;
  VIXL_ASSERT(movprfx_is_predicated != movprfx_is_unpredicated);

  int movprfx_zd = movprfx->GetRd();
  int movprfx_pg = movprfx_is_predicated ? movprfx->GetPgLow8() : -1;
  VectorFormat movprfx_vform =
      movprfx_is_predicated ? movprfx->GetSVEVectorFormat() : kFormatUndefined;

  bool pg_matches_low8 = movprfx_pg == GetPgLow8();
  bool vform_matches = movprfx_vform == GetSVEVectorFormat();
  bool zd_matches = movprfx_zd == GetRd();
  bool zd_isnt_zn = movprfx_zd != GetRn();
  bool zd_isnt_zm = movprfx_zd != GetRm();

  switch (form_hash) {
    case "cdot_z_zzzi_s"_h:
    case "sdot_z_zzzi_s"_h:
    case "sudot_z_zzzi_s"_h:
    case "udot_z_zzzi_s"_h:
    case "usdot_z_zzzi_s"_h:
      return (GetRd() != static_cast<int>(ExtractBits(18, 16))) &&
             movprfx_is_unpredicated && zd_isnt_zn && zd_matches;

    case "cdot_z_zzzi_d"_h:
    case "sdot_z_zzzi_d"_h:
    case "udot_z_zzzi_d"_h:
      return (GetRd() != static_cast<int>(ExtractBits(19, 16))) &&
             movprfx_is_unpredicated && zd_isnt_zn && zd_matches;

    case "fmlalb_z_zzzi_s"_h:
    case "fmlalt_z_zzzi_s"_h:
    case "fmlslb_z_zzzi_s"_h:
    case "fmlslt_z_zzzi_s"_h:
    case "smlalb_z_zzzi_d"_h:
    case "smlalb_z_zzzi_s"_h:
    case "smlalt_z_zzzi_d"_h:
    case "smlalt_z_zzzi_s"_h:
    case "smlslb_z_zzzi_d"_h:
    case "smlslb_z_zzzi_s"_h:
    case "smlslt_z_zzzi_d"_h:
    case "smlslt_z_zzzi_s"_h:
    case "sqdmlalb_z_zzzi_d"_h:
    case "sqdmlalb_z_zzzi_s"_h:
    case "sqdmlalt_z_zzzi_d"_h:
    case "sqdmlalt_z_zzzi_s"_h:
    case "sqdmlslb_z_zzzi_d"_h:
    case "sqdmlslb_z_zzzi_s"_h:
    case "sqdmlslt_z_zzzi_d"_h:
    case "sqdmlslt_z_zzzi_s"_h:
    case "umlalb_z_zzzi_d"_h:
    case "umlalb_z_zzzi_s"_h:
    case "umlalt_z_zzzi_d"_h:
    case "umlalt_z_zzzi_s"_h:
    case "umlslb_z_zzzi_d"_h:
    case "umlslb_z_zzzi_s"_h:
    case "umlslt_z_zzzi_d"_h:
    case "umlslt_z_zzzi_s"_h:
      return (GetRd() != GetSVEMulLongZmAndIndex().first) &&
             movprfx_is_unpredicated && zd_isnt_zn && zd_matches;

    case "cmla_z_zzzi_h"_h:
    case "cmla_z_zzzi_s"_h:
    case "fcmla_z_zzzi_h"_h:
    case "fcmla_z_zzzi_s"_h:
    case "fmla_z_zzzi_d"_h:
    case "fmla_z_zzzi_h"_h:
    case "fmla_z_zzzi_s"_h:
    case "fmls_z_zzzi_d"_h:
    case "fmls_z_zzzi_h"_h:
    case "fmls_z_zzzi_s"_h:
    case "mla_z_zzzi_d"_h:
    case "mla_z_zzzi_h"_h:
    case "mla_z_zzzi_s"_h:
    case "mls_z_zzzi_d"_h:
    case "mls_z_zzzi_h"_h:
    case "mls_z_zzzi_s"_h:
    case "sqrdcmlah_z_zzzi_h"_h:
    case "sqrdcmlah_z_zzzi_s"_h:
    case "sqrdmlah_z_zzzi_d"_h:
    case "sqrdmlah_z_zzzi_h"_h:
    case "sqrdmlah_z_zzzi_s"_h:
    case "sqrdmlsh_z_zzzi_d"_h:
    case "sqrdmlsh_z_zzzi_h"_h:
    case "sqrdmlsh_z_zzzi_s"_h:
      return (GetRd() != GetSVEMulZmAndIndex().first) &&
             movprfx_is_unpredicated && zd_isnt_zn && zd_matches;

    case "adclb_z_zzz"_h:
    case "adclt_z_zzz"_h:
    case "bcax_z_zzz"_h:
    case "bsl1n_z_zzz"_h:
    case "bsl2n_z_zzz"_h:
    case "bsl_z_zzz"_h:
    case "cdot_z_zzz"_h:
    case "cmla_z_zzz"_h:
    case "eor3_z_zzz"_h:
    case "eorbt_z_zz"_h:
    case "eortb_z_zz"_h:
    case "fmlalb_z_zzz"_h:
    case "fmlalt_z_zzz"_h:
    case "fmlslb_z_zzz"_h:
    case "fmlslt_z_zzz"_h:
    case "nbsl_z_zzz"_h:
    case "saba_z_zzz"_h:
    case "sabalb_z_zzz"_h:
    case "sabalt_z_zzz"_h:
    case "sbclb_z_zzz"_h:
    case "sbclt_z_zzz"_h:
    case "sdot_z_zzz"_h:
    case "smlalb_z_zzz"_h:
    case "smlalt_z_zzz"_h:
    case "smlslb_z_zzz"_h:
    case "smlslt_z_zzz"_h:
    case "sqdmlalb_z_zzz"_h:
    case "sqdmlalbt_z_zzz"_h:
    case "sqdmlalt_z_zzz"_h:
    case "sqdmlslb_z_zzz"_h:
    case "sqdmlslbt_z_zzz"_h:
    case "sqdmlslt_z_zzz"_h:
    case "sqrdcmlah_z_zzz"_h:
    case "sqrdmlah_z_zzz"_h:
    case "sqrdmlsh_z_zzz"_h:
    case "uaba_z_zzz"_h:
    case "uabalb_z_zzz"_h:
    case "uabalt_z_zzz"_h:
    case "udot_z_zzz"_h:
    case "umlalb_z_zzz"_h:
    case "umlalt_z_zzz"_h:
    case "umlslb_z_zzz"_h:
    case "umlslt_z_zzz"_h:
    case "usdot_z_zzz_s"_h:
    case "fmmla_z_zzz_s"_h:
    case "fmmla_z_zzz_d"_h:
    case "smmla_z_zzz"_h:
    case "ummla_z_zzz"_h:
    case "usmmla_z_zzz"_h:
      return movprfx_is_unpredicated && zd_isnt_zm && zd_isnt_zn && zd_matches;

    case "addp_z_p_zz"_h:
    case "cadd_z_zz"_h:
    case "clasta_z_p_zz"_h:
    case "clastb_z_p_zz"_h:
    case "decd_z_zs"_h:
    case "dech_z_zs"_h:
    case "decw_z_zs"_h:
    case "ext_z_zi_des"_h:
    case "faddp_z_p_zz"_h:
    case "fmaxnmp_z_p_zz"_h:
    case "fmaxp_z_p_zz"_h:
    case "fminnmp_z_p_zz"_h:
    case "fminp_z_p_zz"_h:
    case "ftmad_z_zzi"_h:
    case "incd_z_zs"_h:
    case "inch_z_zs"_h:
    case "incw_z_zs"_h:
    case "insr_z_v"_h:
    case "smaxp_z_p_zz"_h:
    case "sminp_z_p_zz"_h:
    case "splice_z_p_zz_des"_h:
    case "sqcadd_z_zz"_h:
    case "sqdecd_z_zs"_h:
    case "sqdech_z_zs"_h:
    case "sqdecw_z_zs"_h:
    case "sqincd_z_zs"_h:
    case "sqinch_z_zs"_h:
    case "sqincw_z_zs"_h:
    case "srsra_z_zi"_h:
    case "ssra_z_zi"_h:
    case "umaxp_z_p_zz"_h:
    case "uminp_z_p_zz"_h:
    case "uqdecd_z_zs"_h:
    case "uqdech_z_zs"_h:
    case "uqdecw_z_zs"_h:
    case "uqincd_z_zs"_h:
    case "uqinch_z_zs"_h:
    case "uqincw_z_zs"_h:
    case "ursra_z_zi"_h:
    case "usra_z_zi"_h:
    case "xar_z_zzi"_h:
      return movprfx_is_unpredicated && zd_isnt_zn && zd_matches;

    case "add_z_zi"_h:
    case "and_z_zi"_h:
    case "decp_z_p_z"_h:
    case "eor_z_zi"_h:
    case "incp_z_p_z"_h:
    case "insr_z_r"_h:
    case "mul_z_zi"_h:
    case "orr_z_zi"_h:
    case "smax_z_zi"_h:
    case "smin_z_zi"_h:
    case "sqadd_z_zi"_h:
    case "sqdecp_z_p_z"_h:
    case "sqincp_z_p_z"_h:
    case "sqsub_z_zi"_h:
    case "sub_z_zi"_h:
    case "subr_z_zi"_h:
    case "umax_z_zi"_h:
    case "umin_z_zi"_h:
    case "uqadd_z_zi"_h:
    case "uqdecp_z_p_z"_h:
    case "uqincp_z_p_z"_h:
    case "uqsub_z_zi"_h:
      return movprfx_is_unpredicated && zd_matches;

    case "cpy_z_p_i"_h:
      if (movprfx_is_predicated) {
        if (!vform_matches) return false;
        if (movprfx_pg != GetRx<19, 16>()) return false;
      }
      // Only the merging form can take movprfx.
      if (ExtractBit(14) == 0) return false;
      return zd_matches;

    case "fcpy_z_p_i"_h:
      return (movprfx_is_unpredicated ||
              ((movprfx_pg == GetRx<19, 16>()) && vform_matches)) &&
             zd_matches;

    case "flogb_z_p_z"_h:
      return (movprfx_is_unpredicated ||
              ((movprfx_vform == GetSVEVectorFormat(17)) && pg_matches_low8)) &&
             zd_isnt_zn && zd_matches;

    case "asr_z_p_zi"_h:
    case "asrd_z_p_zi"_h:
    case "lsl_z_p_zi"_h:
    case "lsr_z_p_zi"_h:
    case "sqshl_z_p_zi"_h:
    case "sqshlu_z_p_zi"_h:
    case "srshr_z_p_zi"_h:
    case "uqshl_z_p_zi"_h:
    case "urshr_z_p_zi"_h:
      return (movprfx_is_unpredicated ||
              ((movprfx_vform ==
                SVEFormatFromLaneSizeInBytesLog2(
                    GetSVEImmShiftAndLaneSizeLog2(true).second)) &&
               pg_matches_low8)) &&
             zd_matches;

    case "fcvt_z_p_z_d2h"_h:
    case "fcvt_z_p_z_d2s"_h:
    case "fcvt_z_p_z_h2d"_h:
    case "fcvt_z_p_z_s2d"_h:
    case "fcvtx_z_p_z_d2s"_h:
    case "fcvtzs_z_p_z_d2w"_h:
    case "fcvtzs_z_p_z_d2x"_h:
    case "fcvtzs_z_p_z_fp162x"_h:
    case "fcvtzs_z_p_z_s2x"_h:
    case "fcvtzu_z_p_z_d2w"_h:
    case "fcvtzu_z_p_z_d2x"_h:
    case "fcvtzu_z_p_z_fp162x"_h:
    case "fcvtzu_z_p_z_s2x"_h:
    case "scvtf_z_p_z_w2d"_h:
    case "scvtf_z_p_z_x2d"_h:
    case "scvtf_z_p_z_x2fp16"_h:
    case "scvtf_z_p_z_x2s"_h:
    case "ucvtf_z_p_z_w2d"_h:
    case "ucvtf_z_p_z_x2d"_h:
    case "ucvtf_z_p_z_x2fp16"_h:
    case "ucvtf_z_p_z_x2s"_h:
      return (movprfx_is_unpredicated ||
              ((movprfx_vform == kFormatVnD) && pg_matches_low8)) &&
             zd_isnt_zn && zd_matches;

    case "fcvtzs_z_p_z_fp162h"_h:
    case "fcvtzu_z_p_z_fp162h"_h:
    case "scvtf_z_p_z_h2fp16"_h:
    case "ucvtf_z_p_z_h2fp16"_h:
      return (movprfx_is_unpredicated ||
              ((movprfx_vform == kFormatVnH) && pg_matches_low8)) &&
             zd_isnt_zn && zd_matches;

    case "fcvt_z_p_z_h2s"_h:
    case "fcvt_z_p_z_s2h"_h:
    case "fcvtzs_z_p_z_fp162w"_h:
    case "fcvtzs_z_p_z_s2w"_h:
    case "fcvtzu_z_p_z_fp162w"_h:
    case "fcvtzu_z_p_z_s2w"_h:
    case "scvtf_z_p_z_w2fp16"_h:
    case "scvtf_z_p_z_w2s"_h:
    case "ucvtf_z_p_z_w2fp16"_h:
    case "ucvtf_z_p_z_w2s"_h:
      return (movprfx_is_unpredicated ||
              ((movprfx_vform == kFormatVnS) && pg_matches_low8)) &&
             zd_isnt_zn && zd_matches;

    case "fcmla_z_p_zzz"_h:
    case "fmad_z_p_zzz"_h:
    case "fmla_z_p_zzz"_h:
    case "fmls_z_p_zzz"_h:
    case "fmsb_z_p_zzz"_h:
    case "fnmad_z_p_zzz"_h:
    case "fnmla_z_p_zzz"_h:
    case "fnmls_z_p_zzz"_h:
    case "fnmsb_z_p_zzz"_h:
    case "mad_z_p_zzz"_h:
    case "mla_z_p_zzz"_h:
    case "mls_z_p_zzz"_h:
    case "msb_z_p_zzz"_h:
      return (movprfx_is_unpredicated || (pg_matches_low8 && vform_matches)) &&
             zd_isnt_zm && zd_isnt_zn && zd_matches;

    case "abs_z_p_z"_h:
    case "add_z_p_zz"_h:
    case "and_z_p_zz"_h:
    case "asr_z_p_zw"_h:
    case "asr_z_p_zz"_h:
    case "asrr_z_p_zz"_h:
    case "bic_z_p_zz"_h:
    case "cls_z_p_z"_h:
    case "clz_z_p_z"_h:
    case "cnot_z_p_z"_h:
    case "cnt_z_p_z"_h:
    case "cpy_z_p_v"_h:
    case "eor_z_p_zz"_h:
    case "fabd_z_p_zz"_h:
    case "fabs_z_p_z"_h:
    case "fadd_z_p_zz"_h:
    case "fcadd_z_p_zz"_h:
    case "fdiv_z_p_zz"_h:
    case "fdivr_z_p_zz"_h:
    case "fmax_z_p_zz"_h:
    case "fmaxnm_z_p_zz"_h:
    case "fmin_z_p_zz"_h:
    case "fminnm_z_p_zz"_h:
    case "fmul_z_p_zz"_h:
    case "fmulx_z_p_zz"_h:
    case "fneg_z_p_z"_h:
    case "frecpx_z_p_z"_h:
    case "frinta_z_p_z"_h:
    case "frinti_z_p_z"_h:
    case "frintm_z_p_z"_h:
    case "frintn_z_p_z"_h:
    case "frintp_z_p_z"_h:
    case "frintx_z_p_z"_h:
    case "frintz_z_p_z"_h:
    case "fscale_z_p_zz"_h:
    case "fsqrt_z_p_z"_h:
    case "fsub_z_p_zz"_h:
    case "fsubr_z_p_zz"_h:
    case "lsl_z_p_zw"_h:
    case "lsl_z_p_zz"_h:
    case "lslr_z_p_zz"_h:
    case "lsr_z_p_zw"_h:
    case "lsr_z_p_zz"_h:
    case "lsrr_z_p_zz"_h:
    case "mul_z_p_zz"_h:
    case "neg_z_p_z"_h:
    case "not_z_p_z"_h:
    case "orr_z_p_zz"_h:
    case "rbit_z_p_z"_h:
    case "revb_z_z"_h:
    case "revh_z_z"_h:
    case "revw_z_z"_h:
    case "sabd_z_p_zz"_h:
    case "sadalp_z_p_z"_h:
    case "sdiv_z_p_zz"_h:
    case "sdivr_z_p_zz"_h:
    case "shadd_z_p_zz"_h:
    case "shsub_z_p_zz"_h:
    case "shsubr_z_p_zz"_h:
    case "smax_z_p_zz"_h:
    case "smin_z_p_zz"_h:
    case "smulh_z_p_zz"_h:
    case "sqabs_z_p_z"_h:
    case "sqadd_z_p_zz"_h:
    case "sqneg_z_p_z"_h:
    case "sqrshl_z_p_zz"_h:
    case "sqrshlr_z_p_zz"_h:
    case "sqshl_z_p_zz"_h:
    case "sqshlr_z_p_zz"_h:
    case "sqsub_z_p_zz"_h:
    case "sqsubr_z_p_zz"_h:
    case "srhadd_z_p_zz"_h:
    case "srshl_z_p_zz"_h:
    case "srshlr_z_p_zz"_h:
    case "sub_z_p_zz"_h:
    case "subr_z_p_zz"_h:
    case "suqadd_z_p_zz"_h:
    case "sxtb_z_p_z"_h:
    case "sxth_z_p_z"_h:
    case "sxtw_z_p_z"_h:
    case "uabd_z_p_zz"_h:
    case "uadalp_z_p_z"_h:
    case "udiv_z_p_zz"_h:
    case "udivr_z_p_zz"_h:
    case "uhadd_z_p_zz"_h:
    case "uhsub_z_p_zz"_h:
    case "uhsubr_z_p_zz"_h:
    case "umax_z_p_zz"_h:
    case "umin_z_p_zz"_h:
    case "umulh_z_p_zz"_h:
    case "uqadd_z_p_zz"_h:
    case "uqrshl_z_p_zz"_h:
    case "uqrshlr_z_p_zz"_h:
    case "uqshl_z_p_zz"_h:
    case "uqshlr_z_p_zz"_h:
    case "uqsub_z_p_zz"_h:
    case "uqsubr_z_p_zz"_h:
    case "urecpe_z_p_z"_h:
    case "urhadd_z_p_zz"_h:
    case "urshl_z_p_zz"_h:
    case "urshlr_z_p_zz"_h:
    case "ursqrte_z_p_z"_h:
    case "usqadd_z_p_zz"_h:
    case "uxtb_z_p_z"_h:
    case "uxth_z_p_z"_h:
    case "uxtw_z_p_z"_h:
      return (movprfx_is_unpredicated || (pg_matches_low8 && vform_matches)) &&
             zd_isnt_zn && zd_matches;

    case "cpy_z_p_r"_h:
    case "fadd_z_p_zs"_h:
    case "fmax_z_p_zs"_h:
    case "fmaxnm_z_p_zs"_h:
    case "fmin_z_p_zs"_h:
    case "fminnm_z_p_zs"_h:
    case "fmul_z_p_zs"_h:
    case "fsub_z_p_zs"_h:
    case "fsubr_z_p_zs"_h:
      return (movprfx_is_unpredicated || (pg_matches_low8 && vform_matches)) &&
             zd_matches;
    default:
      return false;
  }
}  // NOLINT(readability/fn_size)

bool Instruction::IsLoad() const {
  if (Mask(LoadStoreAnyFMask) != LoadStoreAnyFixed) {
    return false;
  }

  if (Mask(LoadStorePairAnyFMask) == LoadStorePairAnyFixed) {
    return Mask(LoadStorePairLBit) != 0;
  } else {
    LoadStoreOp op = static_cast<LoadStoreOp>(Mask(LoadStoreMask));
    switch (op) {
      case LDRB_w:
      case LDRH_w:
      case LDR_w:
      case LDR_x:
      case LDRSB_w:
      case LDRSB_x:
      case LDRSH_w:
      case LDRSH_x:
      case LDRSW_x:
      case LDR_b:
      case LDR_h:
      case LDR_s:
      case LDR_d:
      case LDR_q:
        return true;
      default:
        return false;
    }
  }
}


bool Instruction::IsStore() const {
  if (Mask(LoadStoreAnyFMask) != LoadStoreAnyFixed) {
    return false;
  }

  if (Mask(LoadStorePairAnyFMask) == LoadStorePairAnyFixed) {
    return Mask(LoadStorePairLBit) == 0;
  } else {
    LoadStoreOp op = static_cast<LoadStoreOp>(Mask(LoadStoreMask));
    switch (op) {
      case STRB_w:
      case STRH_w:
      case STR_w:
      case STR_x:
      case STR_b:
      case STR_h:
      case STR_s:
      case STR_d:
      case STR_q:
        return true;
      default:
        return false;
    }
  }
}


std::pair<int, int> Instruction::GetSVEPermuteIndexAndLaneSizeLog2() const {
  uint32_t imm_2 = ExtractBits<0x00C00000>();
  uint32_t tsz_5 = ExtractBits<0x001F0000>();
  uint32_t imm_7 = (imm_2 << 5) | tsz_5;
  int lane_size_in_byte_log_2 = std::min(CountTrailingZeros(tsz_5), 5);
  int index = ExtractUnsignedBitfield32(6, lane_size_in_byte_log_2 + 1, imm_7);
  return std::make_pair(index, lane_size_in_byte_log_2);
}

// Get the register and index for SVE indexed multiplies encoded in the forms:
//  .h : Zm = <18:16>, index = <22><20:19>
//  .s : Zm = <18:16>, index = <20:19>
//  .d : Zm = <19:16>, index = <20>
std::pair<int, int> Instruction::GetSVEMulZmAndIndex() const {
  int reg_code = GetRmLow16();
  int index = ExtractBits(20, 19);

  // For .h, index uses bit zero of the size field, so kFormatVnB below implies
  // half-word lane, with most-significant bit of the index zero.
  switch (GetSVEVectorFormat()) {
    case kFormatVnD:
      index >>= 1;  // Only bit 20 in the index for D lanes.
      break;
    case kFormatVnH:
      index += 4;  // Bit 22 is the top bit of index.
      VIXL_FALLTHROUGH();
    case kFormatVnB:
    case kFormatVnS:
      reg_code &= 7;  // Three bits used for the register.
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }
  return std::make_pair(reg_code, index);
}

// Get the register and index for SVE indexed long multiplies encoded in the
// forms:
//  .h : Zm = <18:16>, index = <20:19><11>
//  .s : Zm = <19:16>, index = <20><11>
std::pair<int, int> Instruction::GetSVEMulLongZmAndIndex() const {
  int reg_code = GetRmLow16();
  int index = ExtractBit(11);

  // For long multiplies, the SVE size field <23:22> encodes the destination
  // element size. The source element size is half the width.
  switch (GetSVEVectorFormat()) {
    case kFormatVnS:
      reg_code &= 7;
      index |= ExtractBits(20, 19) << 1;
      break;
    case kFormatVnD:
      index |= ExtractBit(20) << 1;
      break;
    default:
      VIXL_UNIMPLEMENTED();
      break;
  }
  return std::make_pair(reg_code, index);
}

// Get the register and index for NEON indexed multiplies.
std::pair<int, int> Instruction::GetNEONMulRmAndIndex() const {
  int reg_code = GetRm();
  int index = (GetNEONH() << 2) | (GetNEONL() << 1) | GetNEONM();
  switch (GetNEONSize()) {
    case 0:  // FP H-sized elements.
    case 1:  // Integer H-sized elements.
      // 4-bit Rm, 3-bit index.
      reg_code &= 0xf;
      break;
    case 2:  // S-sized elements.
      // 5-bit Rm, 2-bit index.
      index >>= 1;
      break;
    case 3:  // FP D-sized elements.
      // 5-bit Rm, 1-bit index.
      index >>= 2;
      break;
  }
  return std::make_pair(reg_code, index);
}

// Logical immediates can't encode zero, so a return value of zero is used to
// indicate a failure case. Specifically, where the constraints on imm_s are
// not met.
uint64_t Instruction::GetImmLogical() const {
  unsigned reg_size = GetSixtyFourBits() ? kXRegSize : kWRegSize;
  int32_t n = GetBitN();
  int32_t imm_s = GetImmSetBits();
  int32_t imm_r = GetImmRotate();
  return DecodeImmBitMask(n, imm_s, imm_r, reg_size);
}

// Logical immediates can't encode zero, so a return value of zero is used to
// indicate a failure case. Specifically, where the constraints on imm_s are
// not met.
uint64_t Instruction::GetSVEImmLogical() const {
  int n = GetSVEBitN();
  int imm_s = GetSVEImmSetBits();
  int imm_r = GetSVEImmRotate();
  int lane_size_in_bytes_log2 = GetSVEBitwiseImmLaneSizeInBytesLog2();
  switch (lane_size_in_bytes_log2) {
    case kDRegSizeInBytesLog2:
    case kSRegSizeInBytesLog2:
    case kHRegSizeInBytesLog2:
    case kBRegSizeInBytesLog2: {
      int lane_size_in_bits = 1 << (lane_size_in_bytes_log2 + 3);
      return DecodeImmBitMask(n, imm_s, imm_r, lane_size_in_bits);
    }
    default:
      return 0;
  }
}

std::pair<int, int> Instruction::GetSVEImmShiftAndLaneSizeLog2(
    bool is_predicated) const {
  Instr tsize =
      is_predicated ? ExtractBits<0x00C00300>() : ExtractBits<0x00D80000>();
  Instr imm_3 =
      is_predicated ? ExtractBits<0x000000E0>() : ExtractBits<0x00070000>();
  if (tsize == 0) {
    // The bit field `tsize` means undefined if it is zero, so return a
    // convenience value kWMinInt to indicate a failure case.
    return std::make_pair(kWMinInt, kWMinInt);
  }

  int lane_size_in_bytes_log_2 = 32 - CountLeadingZeros(tsize, 32) - 1;
  int esize = (1 << lane_size_in_bytes_log_2) * kBitsPerByte;
  int shift = (2 * esize) - ((tsize << 3) | imm_3);
  return std::make_pair(shift, lane_size_in_bytes_log_2);
}

int Instruction::GetSVEMsizeFromDtype(bool is_signed, int dtype_h_lsb) const {
  Instr dtype_h = ExtractBits(dtype_h_lsb + 1, dtype_h_lsb);
  if (is_signed) {
    dtype_h = dtype_h ^ 0x3;
  }
  return dtype_h;
}

int Instruction::GetSVEEsizeFromDtype(bool is_signed, int dtype_l_lsb) const {
  Instr dtype_l = ExtractBits(dtype_l_lsb + 1, dtype_l_lsb);
  if (is_signed) {
    dtype_l = dtype_l ^ 0x3;
  }
  return dtype_l;
}

int Instruction::GetSVEBitwiseImmLaneSizeInBytesLog2() const {
  int n = GetSVEBitN();
  int imm_s = GetSVEImmSetBits();
  unsigned type_bitset =
      (n << SVEImmSetBits_width) | (~imm_s & GetUintMask(SVEImmSetBits_width));

  // An lane size is constructed from the n and imm_s bits according to
  // the following table:
  //
  // N   imms   size
  // 0  0xxxxx   32
  // 0  10xxxx   16
  // 0  110xxx    8
  // 0  1110xx    8
  // 0  11110x    8
  // 1  xxxxxx   64

  if (type_bitset == 0) {
    // Bail out early since `HighestSetBitPosition` doesn't accept zero
    // value input.
    return -1;
  }

  switch (HighestSetBitPosition(type_bitset)) {
    case 6:
      return kDRegSizeInBytesLog2;
    case 5:
      return kSRegSizeInBytesLog2;
    case 4:
      return kHRegSizeInBytesLog2;
    case 3:
    case 2:
    case 1:
      return kBRegSizeInBytesLog2;
    default:
      // RESERVED encoding.
      return -1;
  }
}

int Instruction::GetSVEExtractImmediate() const {
  const int imm8h_mask = 0x001F0000;
  const int imm8l_mask = 0x00001C00;
  return ExtractBits<imm8h_mask | imm8l_mask>();
}

uint64_t Instruction::DecodeImmBitMask(int32_t n,
                                       int32_t imm_s,
                                       int32_t imm_r,
                                       int32_t size) const {
  // An integer is constructed from the n, imm_s and imm_r bits according to
  // the following table:
  //
  //  N   imms    immr    size        S             R
  //  1  ssssss  rrrrrr    64    UInt(ssssss)  UInt(rrrrrr)
  //  0  0sssss  xrrrrr    32    UInt(sssss)   UInt(rrrrr)
  //  0  10ssss  xxrrrr    16    UInt(ssss)    UInt(rrrr)
  //  0  110sss  xxxrrr     8    UInt(sss)     UInt(rrr)
  //  0  1110ss  xxxxrr     4    UInt(ss)      UInt(rr)
  //  0  11110s  xxxxxr     2    UInt(s)       UInt(r)
  // (s bits must not be all set)
  //
  // A pattern is constructed of size bits, where the least significant S+1
  // bits are set. The pattern is rotated right by R, and repeated across a
  // 32 or 64-bit value, depending on destination register width.
  //

  if (n == 1) {
    if (imm_s == 0x3f) {
      return 0;
    }
    uint64_t bits = (UINT64_C(1) << (imm_s + 1)) - 1;
    return RotateRight(bits, imm_r, 64);
  } else {
    if ((imm_s >> 1) == 0x1f) {
      return 0;
    }
    for (int width = 0x20; width >= 0x2; width >>= 1) {
      if ((imm_s & width) == 0) {
        int mask = width - 1;
        if ((imm_s & mask) == mask) {
          return 0;
        }
        uint64_t bits = (UINT64_C(1) << ((imm_s & mask) + 1)) - 1;
        return RepeatBitsAcrossReg(size,
                                   RotateRight(bits, imm_r & mask, width),
                                   width);
      }
    }
  }
  VIXL_UNREACHABLE();
  return 0;
}


uint32_t Instruction::GetImmNEONabcdefgh() const {
  return GetImmNEONabc() << 5 | GetImmNEONdefgh();
}


Float16 Instruction::Imm8ToFloat16(uint32_t imm8) {
  // Imm8: abcdefgh (8 bits)
  // Half: aBbb.cdef.gh00.0000 (16 bits)
  // where B is b ^ 1
  uint32_t bits = imm8;
  uint16_t bit7 = (bits >> 7) & 0x1;
  uint16_t bit6 = (bits >> 6) & 0x1;
  uint16_t bit5_to_0 = bits & 0x3f;
  uint16_t result = (bit7 << 15) | ((4 - bit6) << 12) | (bit5_to_0 << 6);
  return RawbitsToFloat16(result);
}


float Instruction::Imm8ToFP32(uint32_t imm8) {
  // Imm8: abcdefgh (8 bits)
  // Single: aBbb.bbbc.defg.h000.0000.0000.0000.0000 (32 bits)
  // where B is b ^ 1
  uint32_t bits = imm8;
  uint32_t bit7 = (bits >> 7) & 0x1;
  uint32_t bit6 = (bits >> 6) & 0x1;
  uint32_t bit5_to_0 = bits & 0x3f;
  uint32_t result = (bit7 << 31) | ((32 - bit6) << 25) | (bit5_to_0 << 19);

  return RawbitsToFloat(result);
}


Float16 Instruction::GetImmFP16() const { return Imm8ToFloat16(GetImmFP()); }


float Instruction::GetImmFP32() const { return Imm8ToFP32(GetImmFP()); }


double Instruction::Imm8ToFP64(uint32_t imm8) {
  // Imm8: abcdefgh (8 bits)
  // Double: aBbb.bbbb.bbcd.efgh.0000.0000.0000.0000
  //         0000.0000.0000.0000.0000.0000.0000.0000 (64 bits)
  // where B is b ^ 1
  uint32_t bits = imm8;
  uint64_t bit7 = (bits >> 7) & 0x1;
  uint64_t bit6 = (bits >> 6) & 0x1;
  uint64_t bit5_to_0 = bits & 0x3f;
  uint64_t result = (bit7 << 63) | ((256 - bit6) << 54) | (bit5_to_0 << 48);

  return RawbitsToDouble(result);
}


double Instruction::GetImmFP64() const { return Imm8ToFP64(GetImmFP()); }


Float16 Instruction::GetImmNEONFP16() const {
  return Imm8ToFloat16(GetImmNEONabcdefgh());
}


float Instruction::GetImmNEONFP32() const {
  return Imm8ToFP32(GetImmNEONabcdefgh());
}


double Instruction::GetImmNEONFP64() const {
  return Imm8ToFP64(GetImmNEONabcdefgh());
}


unsigned CalcLSDataSize(LoadStoreOp op) {
  VIXL_ASSERT((LSSize_offset + LSSize_width) == (kInstructionSize * 8));
  unsigned size = static_cast<Instr>(op) >> LSSize_offset;
  if ((op & LSVector_mask) != 0) {
    // Vector register memory operations encode the access size in the "size"
    // and "opc" fields.
    if ((size == 0) && ((op & LSOpc_mask) >> LSOpc_offset) >= 2) {
      size = kQRegSizeInBytesLog2;
    }
  }
  return size;
}


unsigned CalcLSPairDataSize(LoadStorePairOp op) {
  VIXL_STATIC_ASSERT(kXRegSizeInBytes == kDRegSizeInBytes);
  VIXL_STATIC_ASSERT(kWRegSizeInBytes == kSRegSizeInBytes);
  switch (op) {
    case STP_q:
    case LDP_q:
      return kQRegSizeInBytesLog2;
    case STP_x:
    case LDP_x:
    case STP_d:
    case LDP_d:
      return kXRegSizeInBytesLog2;
    default:
      return kWRegSizeInBytesLog2;
  }
}


int Instruction::GetImmBranchRangeBitwidth(ImmBranchType branch_type) {
  switch (branch_type) {
    case UncondBranchType:
      return ImmUncondBranch_width;
    case CondBranchType:
      return ImmCondBranch_width;
    case CompareBranchType:
      return ImmCmpBranch_width;
    case TestBranchType:
      return ImmTestBranch_width;
    default:
      VIXL_UNREACHABLE();
      return 0;
  }
}


int32_t Instruction::GetImmBranchForwardRange(ImmBranchType branch_type) {
  int32_t encoded_max = 1 << (GetImmBranchRangeBitwidth(branch_type) - 1);
  return encoded_max * kInstructionSize;
}


bool Instruction::IsValidImmPCOffset(ImmBranchType branch_type,
                                     int64_t offset) {
  return IsIntN(GetImmBranchRangeBitwidth(branch_type), offset);
}


const Instruction* Instruction::GetImmPCOffsetTarget() const {
  const Instruction* base = this;
  ptrdiff_t offset;
  if (IsPCRelAddressing()) {
    // ADR and ADRP.
    offset = GetImmPCRel();
    if (Mask(PCRelAddressingMask) == ADRP) {
      base = AlignDown(base, kPageSize);
      offset *= kPageSize;
    } else {
      VIXL_ASSERT(Mask(PCRelAddressingMask) == ADR);
    }
  } else {
    // All PC-relative branches.
    VIXL_ASSERT(GetBranchType() != UnknownBranchType);
    // Relative branch offsets are instruction-size-aligned.
    offset = GetImmBranch() * static_cast<int>(kInstructionSize);
  }
  return base + offset;
}


int Instruction::GetImmBranch() const {
  switch (GetBranchType()) {
    case CondBranchType:
      return GetImmCondBranch();
    case UncondBranchType:
      return GetImmUncondBranch();
    case CompareBranchType:
      return GetImmCmpBranch();
    case TestBranchType:
      return GetImmTestBranch();
    default:
      VIXL_UNREACHABLE();
  }
  return 0;
}


void Instruction::SetImmPCOffsetTarget(const Instruction* target) {
  if (IsPCRelAddressing()) {
    SetPCRelImmTarget(target);
  } else {
    SetBranchImmTarget(target);
  }
}


void Instruction::SetPCRelImmTarget(const Instruction* target) {
  ptrdiff_t imm21;
  if ((Mask(PCRelAddressingMask) == ADR)) {
    imm21 = target - this;
  } else {
    VIXL_ASSERT(Mask(PCRelAddressingMask) == ADRP);
    uintptr_t this_page = reinterpret_cast<uintptr_t>(this) / kPageSize;
    uintptr_t target_page = reinterpret_cast<uintptr_t>(target) / kPageSize;
    imm21 = target_page - this_page;
  }
  Instr imm = Assembler::ImmPCRelAddress(static_cast<int32_t>(imm21));

  SetInstructionBits(Mask(~ImmPCRel_mask) | imm);
}


void Instruction::SetBranchImmTarget(const Instruction* target) {
  VIXL_ASSERT(((target - this) & 3) == 0);
  Instr branch_imm = 0;
  uint32_t imm_mask = 0;
  int offset = static_cast<int>((target - this) >> kInstructionSizeLog2);
  switch (GetBranchType()) {
    case CondBranchType: {
      branch_imm = Assembler::ImmCondBranch(offset);
      imm_mask = ImmCondBranch_mask;
      break;
    }
    case UncondBranchType: {
      branch_imm = Assembler::ImmUncondBranch(offset);
      imm_mask = ImmUncondBranch_mask;
      break;
    }
    case CompareBranchType: {
      branch_imm = Assembler::ImmCmpBranch(offset);
      imm_mask = ImmCmpBranch_mask;
      break;
    }
    case TestBranchType: {
      branch_imm = Assembler::ImmTestBranch(offset);
      imm_mask = ImmTestBranch_mask;
      break;
    }
    default:
      VIXL_UNREACHABLE();
  }
  SetInstructionBits(Mask(~imm_mask) | branch_imm);
}


void Instruction::SetImmLLiteral(const Instruction* source) {
  VIXL_ASSERT(IsWordAligned(source));
  ptrdiff_t offset = (source - this) >> kLiteralEntrySizeLog2;
  Instr imm = Assembler::ImmLLiteral(static_cast<int>(offset));
  Instr mask = ImmLLiteral_mask;

  SetInstructionBits(Mask(~mask) | imm);
}


VectorFormat VectorFormatHalfWidth(VectorFormat vform) {
  switch (vform) {
    case kFormat8H:
      return kFormat8B;
    case kFormat4S:
      return kFormat4H;
    case kFormat2D:
      return kFormat2S;
    case kFormat1Q:
      return kFormat1D;
    case kFormatH:
      return kFormatB;
    case kFormatS:
      return kFormatH;
    case kFormatD:
      return kFormatS;
    case kFormatVnH:
      return kFormatVnB;
    case kFormatVnS:
      return kFormatVnH;
    case kFormatVnD:
      return kFormatVnS;
    case kFormatVnQ:
      return kFormatVnD;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


VectorFormat VectorFormatDoubleWidth(VectorFormat vform) {
  switch (vform) {
    case kFormat8B:
      return kFormat8H;
    case kFormat4H:
      return kFormat4S;
    case kFormat2S:
      return kFormat2D;
    case kFormatB:
      return kFormatH;
    case kFormatH:
      return kFormatS;
    case kFormatS:
      return kFormatD;
    case kFormatVnB:
      return kFormatVnH;
    case kFormatVnH:
      return kFormatVnS;
    case kFormatVnS:
      return kFormatVnD;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


VectorFormat VectorFormatFillQ(VectorFormat vform) {
  switch (vform) {
    case kFormatB:
    case kFormat8B:
    case kFormat16B:
      return kFormat16B;
    case kFormatH:
    case kFormat4H:
    case kFormat8H:
      return kFormat8H;
    case kFormatS:
    case kFormat2S:
    case kFormat4S:
      return kFormat4S;
    case kFormatD:
    case kFormat1D:
    case kFormat2D:
      return kFormat2D;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}

VectorFormat VectorFormatHalfWidthDoubleLanes(VectorFormat vform) {
  switch (vform) {
    case kFormat4H:
      return kFormat8B;
    case kFormat8H:
      return kFormat16B;
    case kFormat2S:
      return kFormat4H;
    case kFormat4S:
      return kFormat8H;
    case kFormat1D:
      return kFormat2S;
    case kFormat2D:
      return kFormat4S;
    case kFormat1Q:
      return kFormat2D;
    case kFormatVnH:
      return kFormatVnB;
    case kFormatVnS:
      return kFormatVnH;
    case kFormatVnD:
      return kFormatVnS;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}

VectorFormat VectorFormatDoubleLanes(VectorFormat vform) {
  VIXL_ASSERT(vform == kFormat8B || vform == kFormat4H || vform == kFormat2S);
  switch (vform) {
    case kFormat8B:
      return kFormat16B;
    case kFormat4H:
      return kFormat8H;
    case kFormat2S:
      return kFormat4S;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


VectorFormat VectorFormatHalfLanes(VectorFormat vform) {
  VIXL_ASSERT(vform == kFormat16B || vform == kFormat8H || vform == kFormat4S);
  switch (vform) {
    case kFormat16B:
      return kFormat8B;
    case kFormat8H:
      return kFormat4H;
    case kFormat4S:
      return kFormat2S;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


VectorFormat ScalarFormatFromLaneSize(int lane_size_in_bits) {
  switch (lane_size_in_bits) {
    case 8:
      return kFormatB;
    case 16:
      return kFormatH;
    case 32:
      return kFormatS;
    case 64:
      return kFormatD;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


bool IsSVEFormat(VectorFormat vform) {
  switch (vform) {
    case kFormatVnB:
    case kFormatVnH:
    case kFormatVnS:
    case kFormatVnD:
    case kFormatVnQ:
    case kFormatVnO:
      return true;
    default:
      return false;
  }
}


VectorFormat SVEFormatFromLaneSizeInBytes(int lane_size_in_bytes) {
  switch (lane_size_in_bytes) {
    case 1:
      return kFormatVnB;
    case 2:
      return kFormatVnH;
    case 4:
      return kFormatVnS;
    case 8:
      return kFormatVnD;
    case 16:
      return kFormatVnQ;
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


VectorFormat SVEFormatFromLaneSizeInBits(int lane_size_in_bits) {
  switch (lane_size_in_bits) {
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
      return SVEFormatFromLaneSizeInBytes(lane_size_in_bits / kBitsPerByte);
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


VectorFormat SVEFormatFromLaneSizeInBytesLog2(int lane_size_in_bytes_log2) {
  switch (lane_size_in_bytes_log2) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      return SVEFormatFromLaneSizeInBytes(1 << lane_size_in_bytes_log2);
    default:
      VIXL_UNREACHABLE();
      return kFormatUndefined;
  }
}


VectorFormat ScalarFormatFromFormat(VectorFormat vform) {
  return ScalarFormatFromLaneSize(LaneSizeInBitsFromFormat(vform));
}


unsigned RegisterSizeInBitsFromFormat(VectorFormat vform) {
  VIXL_ASSERT(vform != kFormatUndefined);
  VIXL_ASSERT(!IsSVEFormat(vform));
  switch (vform) {
    case kFormatB:
      return kBRegSize;
    case kFormatH:
      return kHRegSize;
    case kFormatS:
    case kFormat2H:
      return kSRegSize;
    case kFormatD:
    case kFormat8B:
    case kFormat4H:
    case kFormat2S:
    case kFormat1D:
      return kDRegSize;
    case kFormat16B:
    case kFormat8H:
    case kFormat4S:
    case kFormat2D:
    case kFormat1Q:
      return kQRegSize;
    default:
      VIXL_UNREACHABLE();
      return 0;
  }
}


unsigned RegisterSizeInBytesFromFormat(VectorFormat vform) {
  return RegisterSizeInBitsFromFormat(vform) / 8;
}


unsigned LaneSizeInBitsFromFormat(VectorFormat vform) {
  VIXL_ASSERT(vform != kFormatUndefined);
  switch (vform) {
    case kFormatB:
    case kFormat8B:
    case kFormat16B:
    case kFormatVnB:
      return 8;
    case kFormatH:
    case kFormat2H:
    case kFormat4H:
    case kFormat8H:
    case kFormatVnH:
      return 16;
    case kFormatS:
    case kFormat2S:
    case kFormat4S:
    case kFormatVnS:
      return 32;
    case kFormatD:
    case kFormat1D:
    case kFormat2D:
    case kFormatVnD:
      return 64;
    case kFormat1Q:
    case kFormatVnQ:
      return 128;
    case kFormatVnO:
      return 256;
    default:
      VIXL_UNREACHABLE();
      return 0;
  }
}


int LaneSizeInBytesFromFormat(VectorFormat vform) {
  return LaneSizeInBitsFromFormat(vform) / 8;
}


int LaneSizeInBytesLog2FromFormat(VectorFormat vform) {
  VIXL_ASSERT(vform != kFormatUndefined);
  switch (vform) {
    case kFormatB:
    case kFormat8B:
    case kFormat16B:
    case kFormatVnB:
      return 0;
    case kFormatH:
    case kFormat2H:
    case kFormat4H:
    case kFormat8H:
    case kFormatVnH:
      return 1;
    case kFormatS:
    case kFormat2S:
    case kFormat4S:
    case kFormatVnS:
      return 2;
    case kFormatD:
    case kFormat1D:
    case kFormat2D:
    case kFormatVnD:
      return 3;
    case kFormatVnQ:
      return 4;
    default:
      VIXL_UNREACHABLE();
      return 0;
  }
}


int LaneCountFromFormat(VectorFormat vform) {
  VIXL_ASSERT(vform != kFormatUndefined);
  switch (vform) {
    case kFormat16B:
      return 16;
    case kFormat8B:
    case kFormat8H:
      return 8;
    case kFormat4H:
    case kFormat4S:
      return 4;
    case kFormat2H:
    case kFormat2S:
    case kFormat2D:
      return 2;
    case kFormat1D:
    case kFormat1Q:
    case kFormatB:
    case kFormatH:
    case kFormatS:
    case kFormatD:
      return 1;
    default:
      VIXL_UNREACHABLE();
      return 0;
  }
}


int MaxLaneCountFromFormat(VectorFormat vform) {
  VIXL_ASSERT(vform != kFormatUndefined);
  switch (vform) {
    case kFormatB:
    case kFormat8B:
    case kFormat16B:
      return 16;
    case kFormatH:
    case kFormat4H:
    case kFormat8H:
      return 8;
    case kFormatS:
    case kFormat2S:
    case kFormat4S:
      return 4;
    case kFormatD:
    case kFormat1D:
    case kFormat2D:
      return 2;
    default:
      VIXL_UNREACHABLE();
      return 0;
  }
}


// Does 'vform' indicate a vector format or a scalar format?
bool IsVectorFormat(VectorFormat vform) {
  VIXL_ASSERT(vform != kFormatUndefined);
  switch (vform) {
    case kFormatB:
    case kFormatH:
    case kFormatS:
    case kFormatD:
      return false;
    default:
      return true;
  }
}


int64_t MaxIntFromFormat(VectorFormat vform) {
  int lane_size = LaneSizeInBitsFromFormat(vform);
  return static_cast<int64_t>(GetUintMask(lane_size) >> 1);
}


int64_t MinIntFromFormat(VectorFormat vform) {
  return -MaxIntFromFormat(vform) - 1;
}


uint64_t MaxUintFromFormat(VectorFormat vform) {
  return GetUintMask(LaneSizeInBitsFromFormat(vform));
}

}  // namespace aarch64
}  // namespace vixl
