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

#include "disasm-aarch64.h"

#include <bitset>
#include <cstdlib>
#include <sstream>

namespace vixl {
namespace aarch64 {

const Disassembler::FormToVisitorFnMap *Disassembler::GetFormToVisitorFnMap() {
  static const FormToVisitorFnMap form_to_visitor = {
      DEFAULT_FORM_TO_VISITOR_MAP(Disassembler),
      {"autia1716_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"autiasp_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"autiaz_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"autib1716_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"autibsp_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"autibz_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"axflag_m_pstate"_h, &Disassembler::DisassembleNoArgs},
      {"cfinv_m_pstate"_h, &Disassembler::DisassembleNoArgs},
      {"csdb_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"dgh_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"ssbb_only_barriers"_h, &Disassembler::DisassembleNoArgs},
      {"esb_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"isb_bi_barriers"_h, &Disassembler::DisassembleNoArgs},
      {"nop_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"pacia1716_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"paciasp_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"paciaz_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"pacib1716_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"pacibsp_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"pacibz_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"sev_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"sevl_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"wfe_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"wfi_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"xaflag_m_pstate"_h, &Disassembler::DisassembleNoArgs},
      {"xpaclri_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"yield_hi_hints"_h, &Disassembler::DisassembleNoArgs},
      {"abs_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"cls_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"clz_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"cnt_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"neg_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"rev16_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"rev32_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"rev64_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"sqabs_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"sqneg_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"suqadd_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"urecpe_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"ursqrte_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"usqadd_asimdmisc_r"_h, &Disassembler::VisitNEON2RegMisc},
      {"not_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegLogical},
      {"rbit_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegLogical},
      {"xtn_asimdmisc_n"_h, &Disassembler::DisassembleNEON2RegExtract},
      {"sqxtn_asimdmisc_n"_h, &Disassembler::DisassembleNEON2RegExtract},
      {"uqxtn_asimdmisc_n"_h, &Disassembler::DisassembleNEON2RegExtract},
      {"sqxtun_asimdmisc_n"_h, &Disassembler::DisassembleNEON2RegExtract},
      {"shll_asimdmisc_s"_h, &Disassembler::DisassembleNEON2RegExtract},
      {"sadalp_asimdmisc_p"_h, &Disassembler::DisassembleNEON2RegAddlp},
      {"saddlp_asimdmisc_p"_h, &Disassembler::DisassembleNEON2RegAddlp},
      {"uadalp_asimdmisc_p"_h, &Disassembler::DisassembleNEON2RegAddlp},
      {"uaddlp_asimdmisc_p"_h, &Disassembler::DisassembleNEON2RegAddlp},
      {"cmeq_asimdmisc_z"_h, &Disassembler::DisassembleNEON2RegCompare},
      {"cmge_asimdmisc_z"_h, &Disassembler::DisassembleNEON2RegCompare},
      {"cmgt_asimdmisc_z"_h, &Disassembler::DisassembleNEON2RegCompare},
      {"cmle_asimdmisc_z"_h, &Disassembler::DisassembleNEON2RegCompare},
      {"cmlt_asimdmisc_z"_h, &Disassembler::DisassembleNEON2RegCompare},
      {"fcmeq_asimdmisc_fz"_h, &Disassembler::DisassembleNEON2RegFPCompare},
      {"fcmge_asimdmisc_fz"_h, &Disassembler::DisassembleNEON2RegFPCompare},
      {"fcmgt_asimdmisc_fz"_h, &Disassembler::DisassembleNEON2RegFPCompare},
      {"fcmle_asimdmisc_fz"_h, &Disassembler::DisassembleNEON2RegFPCompare},
      {"fcmlt_asimdmisc_fz"_h, &Disassembler::DisassembleNEON2RegFPCompare},
      {"fcvtl_asimdmisc_l"_h, &Disassembler::DisassembleNEON2RegFPConvert},
      {"fcvtn_asimdmisc_n"_h, &Disassembler::DisassembleNEON2RegFPConvert},
      {"fcvtxn_asimdmisc_n"_h, &Disassembler::DisassembleNEON2RegFPConvert},
      {"fabs_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtas_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtau_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtms_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtmu_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtns_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtnu_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtps_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtpu_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtzs_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fcvtzu_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fneg_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frecpe_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frint32x_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frint32z_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frint64x_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frint64z_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frinta_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frinti_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frintm_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frintn_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frintp_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frintx_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frintz_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"frsqrte_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"fsqrt_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"scvtf_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"ucvtf_asimdmisc_r"_h, &Disassembler::DisassembleNEON2RegFP},
      {"smlal_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"smlsl_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"smull_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"umlal_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"umlsl_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"umull_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"sqdmull_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"sqdmlal_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"sqdmlsl_asimdelem_l"_h, &Disassembler::DisassembleNEONMulByElementLong},
      {"sdot_asimdelem_d"_h, &Disassembler::DisassembleNEONDotProdByElement},
      {"udot_asimdelem_d"_h, &Disassembler::DisassembleNEONDotProdByElement},
      {"usdot_asimdelem_d"_h, &Disassembler::DisassembleNEONDotProdByElement},
      {"sudot_asimdelem_d"_h, &Disassembler::DisassembleNEONDotProdByElement},
      {"fmlal2_asimdelem_lh"_h,
       &Disassembler::DisassembleNEONFPMulByElementLong},
      {"fmlal_asimdelem_lh"_h,
       &Disassembler::DisassembleNEONFPMulByElementLong},
      {"fmlsl2_asimdelem_lh"_h,
       &Disassembler::DisassembleNEONFPMulByElementLong},
      {"fmlsl_asimdelem_lh"_h,
       &Disassembler::DisassembleNEONFPMulByElementLong},
      {"fcmla_asimdelem_c_h"_h,
       &Disassembler::DisassembleNEONComplexMulByElement},
      {"fcmla_asimdelem_c_s"_h,
       &Disassembler::DisassembleNEONComplexMulByElement},
      {"fmla_asimdelem_rh_h"_h,
       &Disassembler::DisassembleNEONHalfFPMulByElement},
      {"fmls_asimdelem_rh_h"_h,
       &Disassembler::DisassembleNEONHalfFPMulByElement},
      {"fmulx_asimdelem_rh_h"_h,
       &Disassembler::DisassembleNEONHalfFPMulByElement},
      {"fmul_asimdelem_rh_h"_h,
       &Disassembler::DisassembleNEONHalfFPMulByElement},
      {"fmla_asimdelem_r_sd"_h, &Disassembler::DisassembleNEONFPMulByElement},
      {"fmls_asimdelem_r_sd"_h, &Disassembler::DisassembleNEONFPMulByElement},
      {"fmulx_asimdelem_r_sd"_h, &Disassembler::DisassembleNEONFPMulByElement},
      {"fmul_asimdelem_r_sd"_h, &Disassembler::DisassembleNEONFPMulByElement},
      {"mla_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"mls_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"mul_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"saba_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"sabd_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"shadd_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"shsub_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"smaxp_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"smax_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"sminp_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"smin_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"srhadd_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"uaba_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"uabd_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"uhadd_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"uhsub_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"umaxp_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"umax_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"uminp_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"umin_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"urhadd_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameNoD},
      {"and_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"bic_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"bif_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"bit_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"bsl_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"eor_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"orr_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"orn_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"pmul_asimdsame_only"_h, &Disassembler::DisassembleNEON3SameLogical},
      {"fmlal2_asimdsame_f"_h, &Disassembler::DisassembleNEON3SameFHM},
      {"fmlal_asimdsame_f"_h, &Disassembler::DisassembleNEON3SameFHM},
      {"fmlsl2_asimdsame_f"_h, &Disassembler::DisassembleNEON3SameFHM},
      {"fmlsl_asimdsame_f"_h, &Disassembler::DisassembleNEON3SameFHM},
      {"sri_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"srshr_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"srsra_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"sshr_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"ssra_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"urshr_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"ursra_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"ushr_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"usra_asimdshf_r"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"scvtf_asimdshf_c"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"ucvtf_asimdshf_c"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"fcvtzs_asimdshf_c"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"fcvtzu_asimdshf_c"_h, &Disassembler::DisassembleNEONShiftRightImm},
      {"ushll_asimdshf_l"_h, &Disassembler::DisassembleNEONShiftLeftLongImm},
      {"sshll_asimdshf_l"_h, &Disassembler::DisassembleNEONShiftLeftLongImm},
      {"shrn_asimdshf_n"_h, &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"rshrn_asimdshf_n"_h, &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"sqshrn_asimdshf_n"_h,
       &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"sqrshrn_asimdshf_n"_h,
       &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"sqshrun_asimdshf_n"_h,
       &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"sqrshrun_asimdshf_n"_h,
       &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"uqshrn_asimdshf_n"_h,
       &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"uqrshrn_asimdshf_n"_h,
       &Disassembler::DisassembleNEONShiftRightNarrowImm},
      {"sqdmlal_asisdelem_l"_h,
       &Disassembler::DisassembleNEONScalarSatMulLongIndex},
      {"sqdmlsl_asisdelem_l"_h,
       &Disassembler::DisassembleNEONScalarSatMulLongIndex},
      {"sqdmull_asisdelem_l"_h,
       &Disassembler::DisassembleNEONScalarSatMulLongIndex},
      {"fmla_asisdelem_rh_h"_h, &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fmla_asisdelem_r_sd"_h, &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fmls_asisdelem_rh_h"_h, &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fmls_asisdelem_r_sd"_h, &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fmulx_asisdelem_rh_h"_h,
       &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fmulx_asisdelem_r_sd"_h,
       &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fmul_asisdelem_rh_h"_h, &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fmul_asisdelem_r_sd"_h, &Disassembler::DisassembleNEONFPScalarMulIndex},
      {"fabd_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"facge_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"facgt_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"fcmeq_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"fcmge_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"fcmgt_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"fmulx_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"frecps_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"frsqrts_asisdsame_only"_h, &Disassembler::DisassembleNEONFPScalar3Same},
      {"sqrdmlah_asisdsame2_only"_h, &Disassembler::VisitNEONScalar3Same},
      {"sqrdmlsh_asisdsame2_only"_h, &Disassembler::VisitNEONScalar3Same},
      {"cmeq_asisdsame_only"_h, &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"cmge_asisdsame_only"_h, &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"cmgt_asisdsame_only"_h, &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"cmhi_asisdsame_only"_h, &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"cmhs_asisdsame_only"_h, &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"cmtst_asisdsame_only"_h,
       &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"add_asisdsame_only"_h, &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"sub_asisdsame_only"_h, &Disassembler::DisassembleNEONScalar3SameOnlyD},
      {"fmaxnmv_asimdall_only_h"_h,
       &Disassembler::DisassembleNEONFP16AcrossLanes},
      {"fmaxv_asimdall_only_h"_h,
       &Disassembler::DisassembleNEONFP16AcrossLanes},
      {"fminnmv_asimdall_only_h"_h,
       &Disassembler::DisassembleNEONFP16AcrossLanes},
      {"fminv_asimdall_only_h"_h,
       &Disassembler::DisassembleNEONFP16AcrossLanes},
      {"fmaxnmv_asimdall_only_sd"_h,
       &Disassembler::DisassembleNEONFPAcrossLanes},
      {"fminnmv_asimdall_only_sd"_h,
       &Disassembler::DisassembleNEONFPAcrossLanes},
      {"fmaxv_asimdall_only_sd"_h, &Disassembler::DisassembleNEONFPAcrossLanes},
      {"fminv_asimdall_only_sd"_h, &Disassembler::DisassembleNEONFPAcrossLanes},
      {"shl_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"sli_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"sri_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"srshr_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"srsra_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"sshr_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"ssra_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"urshr_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"ursra_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"ushr_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"usra_asisdshf_r"_h, &Disassembler::DisassembleNEONScalarShiftImmOnlyD},
      {"sqrshrn_asisdshf_n"_h,
       &Disassembler::DisassembleNEONScalarShiftRightNarrowImm},
      {"sqrshrun_asisdshf_n"_h,
       &Disassembler::DisassembleNEONScalarShiftRightNarrowImm},
      {"sqshrn_asisdshf_n"_h,
       &Disassembler::DisassembleNEONScalarShiftRightNarrowImm},
      {"sqshrun_asisdshf_n"_h,
       &Disassembler::DisassembleNEONScalarShiftRightNarrowImm},
      {"uqrshrn_asisdshf_n"_h,
       &Disassembler::DisassembleNEONScalarShiftRightNarrowImm},
      {"uqshrn_asisdshf_n"_h,
       &Disassembler::DisassembleNEONScalarShiftRightNarrowImm},
      {"cmeq_asisdmisc_z"_h, &Disassembler::DisassembleNEONScalar2RegMiscOnlyD},
      {"cmge_asisdmisc_z"_h, &Disassembler::DisassembleNEONScalar2RegMiscOnlyD},
      {"cmgt_asisdmisc_z"_h, &Disassembler::DisassembleNEONScalar2RegMiscOnlyD},
      {"cmle_asisdmisc_z"_h, &Disassembler::DisassembleNEONScalar2RegMiscOnlyD},
      {"cmlt_asisdmisc_z"_h, &Disassembler::DisassembleNEONScalar2RegMiscOnlyD},
      {"abs_asisdmisc_r"_h, &Disassembler::DisassembleNEONScalar2RegMiscOnlyD},
      {"neg_asisdmisc_r"_h, &Disassembler::DisassembleNEONScalar2RegMiscOnlyD},
      {"fcmeq_asisdmisc_fz"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcmge_asisdmisc_fz"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcmgt_asisdmisc_fz"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcmle_asisdmisc_fz"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcmlt_asisdmisc_fz"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtas_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtau_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtms_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtmu_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtns_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtnu_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtps_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtpu_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtxn_asisdmisc_n"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtzs_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"fcvtzu_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"frecpe_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"frecpx_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"frsqrte_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"scvtf_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"ucvtf_asisdmisc_r"_h, &Disassembler::DisassembleNEONFPScalar2RegMisc},
      {"pmull_asimddiff_l"_h, &Disassembler::DisassembleNEONPolynomialMul},
      {"adclb_z_zzz"_h, &Disassembler::DisassembleSVEAddSubCarry},
      {"adclt_z_zzz"_h, &Disassembler::DisassembleSVEAddSubCarry},
      {"addhnb_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"addhnt_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"addp_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"aesd_z_zz"_h, &Disassembler::Disassemble_ZdnB_ZdnB_ZmB},
      {"aese_z_zz"_h, &Disassembler::Disassemble_ZdnB_ZdnB_ZmB},
      {"aesimc_z_z"_h, &Disassembler::Disassemble_ZdnB_ZdnB},
      {"aesmc_z_z"_h, &Disassembler::Disassemble_ZdnB_ZdnB},
      {"bcax_z_zzz"_h, &Disassembler::DisassembleSVEBitwiseTernary},
      {"bdep_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"bext_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"bgrp_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"bsl1n_z_zzz"_h, &Disassembler::DisassembleSVEBitwiseTernary},
      {"bsl2n_z_zzz"_h, &Disassembler::DisassembleSVEBitwiseTernary},
      {"bsl_z_zzz"_h, &Disassembler::DisassembleSVEBitwiseTernary},
      {"cadd_z_zz"_h, &Disassembler::DisassembleSVEComplexIntAddition},
      {"cdot_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb_const},
      {"cdot_z_zzzi_d"_h, &Disassembler::Disassemble_ZdaD_ZnH_ZmH_imm_const},
      {"cdot_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnB_ZmB_imm_const},
      {"cmla_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT_const},
      {"cmla_z_zzzi_h"_h, &Disassembler::Disassemble_ZdaH_ZnH_ZmH_imm_const},
      {"cmla_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnS_ZmS_imm_const},
      {"eor3_z_zzz"_h, &Disassembler::DisassembleSVEBitwiseTernary},
      {"eorbt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"eortb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"ext_z_zi_con"_h, &Disassembler::Disassemble_ZdB_Zn1B_Zn2B_imm},
      {"faddp_z_p_zz"_h, &Disassembler::DisassembleSVEFPPair},
      {"fcvtlt_z_p_z_h2s"_h, &Disassembler::Disassemble_ZdS_PgM_ZnH},
      {"fcvtlt_z_p_z_s2d"_h, &Disassembler::Disassemble_ZdD_PgM_ZnS},
      {"fcvtnt_z_p_z_d2s"_h, &Disassembler::Disassemble_ZdS_PgM_ZnD},
      {"fcvtnt_z_p_z_s2h"_h, &Disassembler::Disassemble_ZdH_PgM_ZnS},
      {"fcvtx_z_p_z_d2s"_h, &Disassembler::Disassemble_ZdS_PgM_ZnD},
      {"fcvtxnt_z_p_z_d2s"_h, &Disassembler::Disassemble_ZdS_PgM_ZnD},
      {"flogb_z_p_z"_h, &Disassembler::DisassembleSVEFlogb},
      {"fmaxnmp_z_p_zz"_h, &Disassembler::DisassembleSVEFPPair},
      {"fmaxp_z_p_zz"_h, &Disassembler::DisassembleSVEFPPair},
      {"fminnmp_z_p_zz"_h, &Disassembler::DisassembleSVEFPPair},
      {"fminp_z_p_zz"_h, &Disassembler::DisassembleSVEFPPair},
      {"fmlalb_z_zzz"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH},
      {"fmlalb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"fmlalt_z_zzz"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH},
      {"fmlalt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"fmlslb_z_zzz"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH},
      {"fmlslb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"fmlslt_z_zzz"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH},
      {"fmlslt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"histcnt_z_p_zz"_h, &Disassembler::Disassemble_ZdT_PgZ_ZnT_ZmT},
      {"histseg_z_zz"_h, &Disassembler::Disassemble_ZdB_ZnB_ZmB},
      {"ldnt1b_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm},
      {"ldnt1b_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_PgZ_ZnS_Xm},
      {"ldnt1d_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm},
      {"ldnt1h_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm},
      {"ldnt1h_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_PgZ_ZnS_Xm},
      {"ldnt1sb_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm},
      {"ldnt1sb_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_PgZ_ZnS_Xm},
      {"ldnt1sh_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm},
      {"ldnt1sh_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_PgZ_ZnS_Xm},
      {"ldnt1sw_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm},
      {"ldnt1w_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm},
      {"ldnt1w_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_PgZ_ZnS_Xm},
      {"match_p_p_zz"_h, &Disassembler::Disassemble_PdT_PgZ_ZnT_ZmT},
      {"mla_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnD_ZmD_imm},
      {"mla_z_zzzi_h"_h, &Disassembler::Disassemble_ZdH_ZnH_ZmH_imm},
      {"mla_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnS_ZmS_imm},
      {"mls_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnD_ZmD_imm},
      {"mls_z_zzzi_h"_h, &Disassembler::Disassemble_ZdH_ZnH_ZmH_imm},
      {"mls_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnS_ZmS_imm},
      {"mul_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"mul_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnD_ZmD_imm},
      {"mul_z_zzi_h"_h, &Disassembler::Disassemble_ZdH_ZnH_ZmH_imm},
      {"mul_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnS_ZmS_imm},
      {"nbsl_z_zzz"_h, &Disassembler::DisassembleSVEBitwiseTernary},
      {"nmatch_p_p_zz"_h, &Disassembler::Disassemble_PdT_PgZ_ZnT_ZmT},
      {"pmul_z_zz"_h, &Disassembler::Disassemble_ZdB_ZnB_ZmB},
      {"pmullb_z_zz"_h, &Disassembler::DisassembleSVEPmull},
      {"pmullt_z_zz"_h, &Disassembler::DisassembleSVEPmull},
      {"raddhnb_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"raddhnt_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"rax1_z_zz"_h, &Disassembler::Disassemble_ZdD_ZnD_ZmD},
      {"rshrnb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"rshrnt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"rsubhnb_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"rsubhnt_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"saba_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT},
      {"sabalb_z_zzz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"sabalt_z_zzz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"sabdlb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"sabdlt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"sadalp_z_p_z"_h, &Disassembler::Disassemble_ZdaT_PgM_ZnTb},
      {"saddlb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"saddlbt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"saddlt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"saddwb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"saddwt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"sbclb_z_zzz"_h, &Disassembler::DisassembleSVEAddSubCarry},
      {"sbclt_z_zzz"_h, &Disassembler::DisassembleSVEAddSubCarry},
      {"shadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"shrnb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"shrnt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"shsub_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"shsubr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sli_z_zzi"_h, &Disassembler::VisitSVEBitwiseShiftUnpredicated},
      {"sm4e_z_zz"_h, &Disassembler::Disassemble_ZdnS_ZdnS_ZmS},
      {"sm4ekey_z_zz"_h, &Disassembler::Disassemble_ZdS_ZnS_ZmS},
      {"smaxp_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sminp_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"smlalb_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"smlalb_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"smlalb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"smlalt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"smlalt_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"smlalt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"smlslb_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"smlslb_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"smlslb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"smlslt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"smlslt_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"smlslt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"smulh_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"smullb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"smullb_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"smullb_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"smullt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"smullt_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"smullt_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"splice_z_p_zz_con"_h, &Disassembler::Disassemble_ZdT_Pg_Zn1T_Zn2T},
      {"sqabs_z_p_z"_h, &Disassembler::Disassemble_ZdT_PgM_ZnT},
      {"sqadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sqcadd_z_zz"_h, &Disassembler::DisassembleSVEComplexIntAddition},
      {"sqdmlalb_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"sqdmlalb_z_zzzi_d"_h, &Disassembler::Disassemble_ZdaD_ZnS_ZmS_imm},
      {"sqdmlalb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"sqdmlalbt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"sqdmlalt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"sqdmlalt_z_zzzi_d"_h, &Disassembler::Disassemble_ZdaD_ZnS_ZmS_imm},
      {"sqdmlalt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"sqdmlslb_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"sqdmlslb_z_zzzi_d"_h, &Disassembler::Disassemble_ZdaD_ZnS_ZmS_imm},
      {"sqdmlslb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"sqdmlslbt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"sqdmlslt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"sqdmlslt_z_zzzi_d"_h, &Disassembler::Disassemble_ZdaD_ZnS_ZmS_imm},
      {"sqdmlslt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm},
      {"sqdmulh_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"sqdmulh_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnD_ZmD_imm},
      {"sqdmulh_z_zzi_h"_h, &Disassembler::Disassemble_ZdH_ZnH_ZmH_imm},
      {"sqdmulh_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnS_ZmS_imm},
      {"sqdmullb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"sqdmullb_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"sqdmullb_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"sqdmullt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"sqdmullt_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"sqdmullt_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"sqneg_z_p_z"_h, &Disassembler::Disassemble_ZdT_PgM_ZnT},
      {"sqrdcmlah_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT_const},
      {"sqrdcmlah_z_zzzi_h"_h,
       &Disassembler::Disassemble_ZdaH_ZnH_ZmH_imm_const},
      {"sqrdcmlah_z_zzzi_s"_h,
       &Disassembler::Disassemble_ZdaS_ZnS_ZmS_imm_const},
      {"sqrdmlah_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT},
      {"sqrdmlah_z_zzzi_d"_h, &Disassembler::Disassemble_ZdaD_ZnD_ZmD_imm},
      {"sqrdmlah_z_zzzi_h"_h, &Disassembler::Disassemble_ZdaH_ZnH_ZmH_imm},
      {"sqrdmlah_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnS_ZmS_imm},
      {"sqrdmlsh_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT},
      {"sqrdmlsh_z_zzzi_d"_h, &Disassembler::Disassemble_ZdaD_ZnD_ZmD_imm},
      {"sqrdmlsh_z_zzzi_h"_h, &Disassembler::Disassemble_ZdaH_ZnH_ZmH_imm},
      {"sqrdmlsh_z_zzzi_s"_h, &Disassembler::Disassemble_ZdaS_ZnS_ZmS_imm},
      {"sqrdmulh_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"sqrdmulh_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnD_ZmD_imm},
      {"sqrdmulh_z_zzi_h"_h, &Disassembler::Disassemble_ZdH_ZnH_ZmH_imm},
      {"sqrdmulh_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnS_ZmS_imm},
      {"sqrshl_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sqrshlr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sqrshrnb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqrshrnt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqrshrunb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqrshrunt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqshl_z_p_zi"_h, &Disassembler::VisitSVEBitwiseShiftByImm_Predicated},
      {"sqshl_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sqshlr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sqshlu_z_p_zi"_h, &Disassembler::VisitSVEBitwiseShiftByImm_Predicated},
      {"sqshrnb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqshrnt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqshrunb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqshrunt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"sqsub_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sqsubr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sqxtnb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb},
      {"sqxtnt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb},
      {"sqxtunb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb},
      {"sqxtunt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb},
      {"srhadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"sri_z_zzi"_h, &Disassembler::VisitSVEBitwiseShiftUnpredicated},
      {"srshl_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"srshlr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"srshr_z_p_zi"_h, &Disassembler::VisitSVEBitwiseShiftByImm_Predicated},
      {"srsra_z_zi"_h, &Disassembler::VisitSVEBitwiseShiftUnpredicated},
      {"sshllb_z_zi"_h, &Disassembler::DisassembleSVEShiftLeftImm},
      {"sshllt_z_zi"_h, &Disassembler::DisassembleSVEShiftLeftImm},
      {"ssra_z_zi"_h, &Disassembler::VisitSVEBitwiseShiftUnpredicated},
      {"ssublb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"ssublbt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"ssublt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"ssubltb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"ssubwb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"ssubwt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"stnt1b_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_Pg_ZnD_Xm},
      {"stnt1b_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_Pg_ZnS_Xm},
      {"stnt1d_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_Pg_ZnD_Xm},
      {"stnt1h_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_Pg_ZnD_Xm},
      {"stnt1h_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_Pg_ZnS_Xm},
      {"stnt1w_z_p_ar_d_64_unscaled"_h,
       &Disassembler::Disassemble_ZtD_Pg_ZnD_Xm},
      {"stnt1w_z_p_ar_s_x32_unscaled"_h,
       &Disassembler::Disassemble_ZtS_Pg_ZnS_Xm},
      {"subhnb_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"subhnt_z_zz"_h, &Disassembler::DisassembleSVEAddSubHigh},
      {"suqadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"tbl_z_zz_2"_h, &Disassembler::Disassemble_ZdT_Zn1T_Zn2T_ZmT},
      {"tbx_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"uaba_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT},
      {"uabalb_z_zzz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"uabalt_z_zzz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"uabdlb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"uabdlt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"uadalp_z_p_z"_h, &Disassembler::Disassemble_ZdaT_PgM_ZnTb},
      {"uaddlb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"uaddlt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"uaddwb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"uaddwt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"uhadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uhsub_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uhsubr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"umaxp_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uminp_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"umlalb_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"umlalb_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"umlalb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"umlalt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"umlalt_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"umlalt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"umlslb_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"umlslb_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"umlslb_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"umlslt_z_zzz"_h, &Disassembler::Disassemble_ZdaT_ZnTb_ZmTb},
      {"umlslt_z_zzzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"umlslt_z_zzzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"umulh_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmT},
      {"umullb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"umullb_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"umullb_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"umullt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"umullt_z_zzi_d"_h, &Disassembler::Disassemble_ZdD_ZnS_ZmS_imm},
      {"umullt_z_zzi_s"_h, &Disassembler::Disassemble_ZdS_ZnH_ZmH_imm},
      {"uqadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uqrshl_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uqrshlr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uqrshrnb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"uqrshrnt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"uqshl_z_p_zi"_h, &Disassembler::VisitSVEBitwiseShiftByImm_Predicated},
      {"uqshl_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uqshlr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uqshrnb_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"uqshrnt_z_zi"_h, &Disassembler::DisassembleSVEShiftRightImm},
      {"uqsub_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uqsubr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"uqxtnb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb},
      {"uqxtnt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb},
      {"urecpe_z_p_z"_h, &Disassembler::Disassemble_ZdS_PgM_ZnS},
      {"urhadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"urshl_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"urshlr_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"urshr_z_p_zi"_h, &Disassembler::VisitSVEBitwiseShiftByImm_Predicated},
      {"ursqrte_z_p_z"_h, &Disassembler::Disassemble_ZdS_PgM_ZnS},
      {"ursra_z_zi"_h, &Disassembler::VisitSVEBitwiseShiftUnpredicated},
      {"ushllb_z_zi"_h, &Disassembler::DisassembleSVEShiftLeftImm},
      {"ushllt_z_zi"_h, &Disassembler::DisassembleSVEShiftLeftImm},
      {"usqadd_z_p_zz"_h, &Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT},
      {"usra_z_zi"_h, &Disassembler::VisitSVEBitwiseShiftUnpredicated},
      {"usublb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"usublt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnTb_ZmTb},
      {"usubwb_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"usubwt_z_zz"_h, &Disassembler::Disassemble_ZdT_ZnT_ZmTb},
      {"whilege_p_p_rr"_h,
       &Disassembler::VisitSVEIntCompareScalarCountAndLimit},
      {"whilegt_p_p_rr"_h,
       &Disassembler::VisitSVEIntCompareScalarCountAndLimit},
      {"whilehi_p_p_rr"_h,
       &Disassembler::VisitSVEIntCompareScalarCountAndLimit},
      {"whilehs_p_p_rr"_h,
       &Disassembler::VisitSVEIntCompareScalarCountAndLimit},
      {"whilerw_p_rr"_h, &Disassembler::VisitSVEIntCompareScalarCountAndLimit},
      {"whilewr_p_rr"_h, &Disassembler::VisitSVEIntCompareScalarCountAndLimit},
      {"xar_z_zzi"_h, &Disassembler::Disassemble_ZdnT_ZdnT_ZmT_const},
      {"fmmla_z_zzz_s"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT},
      {"fmmla_z_zzz_d"_h, &Disassembler::Disassemble_ZdaT_ZnT_ZmT},
      {"smmla_z_zzz"_h, &Disassembler::Disassemble_ZdaS_ZnB_ZmB},
      {"ummla_z_zzz"_h, &Disassembler::Disassemble_ZdaS_ZnB_ZmB},
      {"usmmla_z_zzz"_h, &Disassembler::Disassemble_ZdaS_ZnB_ZmB},
      {"usdot_z_zzz_s"_h, &Disassembler::Disassemble_ZdaS_ZnB_ZmB},
      {"smmla_asimdsame2_g"_h, &Disassembler::Disassemble_Vd4S_Vn16B_Vm16B},
      {"ummla_asimdsame2_g"_h, &Disassembler::Disassemble_Vd4S_Vn16B_Vm16B},
      {"usmmla_asimdsame2_g"_h, &Disassembler::Disassemble_Vd4S_Vn16B_Vm16B},
      {"ld1row_z_p_bi_u32"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},
      {"ld1row_z_p_br_contiguous"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},
      {"ld1rod_z_p_bi_u64"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},
      {"ld1rod_z_p_br_contiguous"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},
      {"ld1rob_z_p_bi_u8"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},
      {"ld1rob_z_p_br_contiguous"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},
      {"ld1roh_z_p_bi_u16"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},
      {"ld1roh_z_p_br_contiguous"_h,
       &Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},
      {"usdot_z_zzzi_s"_h, &Disassembler::VisitSVEMulIndex},
      {"sudot_z_zzzi_s"_h, &Disassembler::VisitSVEMulIndex},
      {"usdot_asimdsame2_d"_h, &Disassembler::VisitNEON3SameExtra},
      {"addg_64_addsub_immtags"_h,
       &Disassembler::Disassemble_XdSP_XnSP_uimm6_uimm4},
      {"gmi_64g_dp_2src"_h, &Disassembler::Disassemble_Xd_XnSP_Xm},
      {"irg_64i_dp_2src"_h, &Disassembler::Disassemble_XdSP_XnSP_Xm},
      {"ldg_64loffset_ldsttags"_h, &Disassembler::DisassembleMTELoadTag},
      {"st2g_64soffset_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"st2g_64spost_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"st2g_64spre_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stgp_64_ldstpair_off"_h, &Disassembler::DisassembleMTEStoreTagPair},
      {"stgp_64_ldstpair_post"_h, &Disassembler::DisassembleMTEStoreTagPair},
      {"stgp_64_ldstpair_pre"_h, &Disassembler::DisassembleMTEStoreTagPair},
      {"stg_64soffset_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stg_64spost_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stg_64spre_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stz2g_64soffset_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stz2g_64spost_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stz2g_64spre_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stzg_64soffset_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stzg_64spost_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"stzg_64spre_ldsttags"_h, &Disassembler::DisassembleMTEStoreTag},
      {"subg_64_addsub_immtags"_h,
       &Disassembler::Disassemble_XdSP_XnSP_uimm6_uimm4},
      {"subps_64s_dp_2src"_h, &Disassembler::Disassemble_Xd_XnSP_XmSP},
      {"subp_64s_dp_2src"_h, &Disassembler::Disassemble_Xd_XnSP_XmSP},
      {"cpyen_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyern_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyewn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpye_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfen_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfern_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfewn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfe_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfmn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfmrn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfmwn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfm_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfpn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfprn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfpwn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyfp_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpymn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpymrn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpymwn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpym_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpypn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyprn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpypwn_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"cpyp_cpy_memcms"_h, &Disassembler::DisassembleCpy},
      {"seten_set_memcms"_h, &Disassembler::DisassembleSet},
      {"sete_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setgen_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setge_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setgmn_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setgm_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setgpn_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setgp_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setmn_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setm_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setpn_set_memcms"_h, &Disassembler::DisassembleSet},
      {"setp_set_memcms"_h, &Disassembler::DisassembleSet},
      {"abs_32_dp_1src"_h, &Disassembler::VisitDataProcessing1Source},
      {"abs_64_dp_1src"_h, &Disassembler::VisitDataProcessing1Source},
      {"cnt_32_dp_1src"_h, &Disassembler::VisitDataProcessing1Source},
      {"cnt_64_dp_1src"_h, &Disassembler::VisitDataProcessing1Source},
      {"ctz_32_dp_1src"_h, &Disassembler::VisitDataProcessing1Source},
      {"ctz_64_dp_1src"_h, &Disassembler::VisitDataProcessing1Source},
      {"smax_32_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"smax_64_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"smin_32_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"smin_64_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"umax_32_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"umax_64_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"umin_32_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"umin_64_dp_2src"_h, &Disassembler::VisitDataProcessing2Source},
      {"smax_32_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"smax_64_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"smin_32_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"smin_64_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"umax_32u_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"umax_64u_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"umin_32u_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"umin_64u_minmax_imm"_h, &Disassembler::DisassembleMinMaxImm},
      {"bcax_vvv16_crypto4"_h, &Disassembler::DisassembleNEON4Same},
      {"eor3_vvv16_crypto4"_h, &Disassembler::DisassembleNEON4Same},
      {"xar_vvv2_crypto3_imm6"_h, &Disassembler::DisassembleNEONXar},
      {"rax1_vvv2_cryptosha512_3"_h, &Disassembler::DisassembleNEONRax1},
      {"sha512h2_qqv_cryptosha512_3"_h, &Disassembler::DisassembleSHA512},
      {"sha512h_qqv_cryptosha512_3"_h, &Disassembler::DisassembleSHA512},
      {"sha512su0_vv2_cryptosha512_2"_h, &Disassembler::DisassembleSHA512},
      {"sha512su1_vvv2_cryptosha512_3"_h, &Disassembler::DisassembleSHA512},
      {"pmullb_z_zz_q"_h, &Disassembler::DisassembleSVEPmull128},
      {"pmullt_z_zz_q"_h, &Disassembler::DisassembleSVEPmull128},
  };
  return &form_to_visitor;
}  // NOLINT(readability/fn_size)

Disassembler::Disassembler() {
  buffer_size_ = 256;
  buffer_ = reinterpret_cast<char *>(malloc(buffer_size_));
  buffer_pos_ = 0;
  own_buffer_ = true;
  code_address_offset_ = 0;
}

Disassembler::Disassembler(char *text_buffer, int buffer_size) {
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

char *Disassembler::GetOutput() { return buffer_; }

void Disassembler::VisitAddSubImmediate(const Instruction *instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool stack_op =
      (rd_is_zr || RnIsZROrSP(instr)) && (instr->GetImmAddSub() == 0) ? true
                                                                      : false;
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Rds, 'Rns, 'IAddSub";
  const char *form_cmp = "'Rns, 'IAddSub";
  const char *form_mov = "'Rds, 'Rns";

  switch (form_hash_) {
    case "add_32_addsub_imm"_h:
    case "add_64_addsub_imm"_h:
      if (stack_op) {
        mnemonic = "mov";
        form = form_mov;
      }
      break;
    case "adds_32s_addsub_imm"_h:
    case "adds_64s_addsub_imm"_h:
      if (rd_is_zr) {
        mnemonic = "cmn";
        form = form_cmp;
      }
      break;
    case "subs_32s_addsub_imm"_h:
    case "subs_64s_addsub_imm"_h:
      if (rd_is_zr) {
        mnemonic = "cmp";
        form = form_cmp;
      }
      break;
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitAddSubShifted(const Instruction *instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Rd, 'Rn, 'Rm'NDP";
  const char *form_cmp = "'Rn, 'Rm'NDP";
  const char *form_neg = "'Rd, 'Rm'NDP";

  if (instr->GetShiftDP() == ROR) {
    // Add/sub/adds/subs don't allow ROR as a shift mode.
    VisitUnallocated(instr);
    return;
  }

  switch (form_hash_) {
    case "adds_32_addsub_shift"_h:
    case "adds_64_addsub_shift"_h:
      if (rd_is_zr) {
        mnemonic = "cmn";
        form = form_cmp;
      }
      break;
    case "sub_32_addsub_shift"_h:
    case "sub_64_addsub_shift"_h:
      if (rn_is_zr) {
        mnemonic = "neg";
        form = form_neg;
      }
      break;
    case "subs_32_addsub_shift"_h:
    case "subs_64_addsub_shift"_h:
      if (rd_is_zr) {
        mnemonic = "cmp";
        form = form_cmp;
      } else if (rn_is_zr) {
        mnemonic = "negs";
        form = form_neg;
      }
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitAddSubExtended(const Instruction *instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  const char *mnemonic = "";
  Extend mode = static_cast<Extend>(instr->GetExtendMode());
  const char *form = ((mode == UXTX) || (mode == SXTX)) ? "'Rds, 'Rns, 'Xm'Ext"
                                                        : "'Rds, 'Rns, 'Wm'Ext";
  const char *form_cmp =
      ((mode == UXTX) || (mode == SXTX)) ? "'Rns, 'Xm'Ext" : "'Rns, 'Wm'Ext";

  switch (instr->Mask(AddSubExtendedMask)) {
    case ADD_w_ext:
    case ADD_x_ext:
      mnemonic = "add";
      break;
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
    case SUB_x_ext:
      mnemonic = "sub";
      break;
    case SUBS_w_ext:
    case SUBS_x_ext: {
      mnemonic = "subs";
      if (rd_is_zr) {
        mnemonic = "cmp";
        form = form_cmp;
      }
      break;
    }
    default:
      VIXL_UNREACHABLE();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitAddSubWithCarry(const Instruction *instr) {
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm";
  const char *form_neg = "'Rd, 'Rm";

  switch (instr->Mask(AddSubWithCarryMask)) {
    case ADC_w:
    case ADC_x:
      mnemonic = "adc";
      break;
    case ADCS_w:
    case ADCS_x:
      mnemonic = "adcs";
      break;
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
    default:
      VIXL_UNREACHABLE();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitRotateRightIntoFlags(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Xn, 'IRr, 'INzcv");
}


void Disassembler::VisitEvaluateIntoFlags(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Wn");
}


void Disassembler::VisitLogicalImmediate(const Instruction *instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = "";
  const char *form = "'Rds, 'Rn, 'ITri";

  if (instr->GetImmLogical() == 0) {
    // The immediate encoded in the instruction is not in the expected format.
    Format(instr, "unallocated", "(LogicalImmediate)");
    return;
  }

  switch (instr->Mask(LogicalImmediateMask)) {
    case AND_w_imm:
    case AND_x_imm:
      mnemonic = "and";
      break;
    case ORR_w_imm:
    case ORR_x_imm: {
      mnemonic = "orr";
      unsigned reg_size =
          (instr->GetSixtyFourBits() == 1) ? kXRegSize : kWRegSize;
      if (rn_is_zr && !IsMovzMovnImm(reg_size, instr->GetImmLogical())) {
        mnemonic = "mov";
        form = "'Rds, 'ITri";
      }
      break;
    }
    case EOR_w_imm:
    case EOR_x_imm:
      mnemonic = "eor";
      break;
    case ANDS_w_imm:
    case ANDS_x_imm: {
      mnemonic = "ands";
      if (rd_is_zr) {
        mnemonic = "tst";
        form = "'Rn, 'ITri";
      }
      break;
    }
    default:
      VIXL_UNREACHABLE();
  }
  Format(instr, mnemonic, form);
}


bool Disassembler::IsMovzMovnImm(unsigned reg_size, uint64_t value) {
  VIXL_ASSERT((reg_size == kXRegSize) ||
              ((reg_size == kWRegSize) && (value <= 0xffffffff)));

  // Test for movz: 16 bits set at positions 0, 16, 32 or 48.
  if (((value & UINT64_C(0xffffffffffff0000)) == 0) ||
      ((value & UINT64_C(0xffffffff0000ffff)) == 0) ||
      ((value & UINT64_C(0xffff0000ffffffff)) == 0) ||
      ((value & UINT64_C(0x0000ffffffffffff)) == 0)) {
    return true;
  }

  // Test for movn: NOT(16 bits set at positions 0, 16, 32 or 48).
  if ((reg_size == kXRegSize) &&
      (((~value & UINT64_C(0xffffffffffff0000)) == 0) ||
       ((~value & UINT64_C(0xffffffff0000ffff)) == 0) ||
       ((~value & UINT64_C(0xffff0000ffffffff)) == 0) ||
       ((~value & UINT64_C(0x0000ffffffffffff)) == 0))) {
    return true;
  }
  if ((reg_size == kWRegSize) && (((value & 0xffff0000) == 0xffff0000) ||
                                  ((value & 0x0000ffff) == 0x0000ffff))) {
    return true;
  }
  return false;
}


void Disassembler::VisitLogicalShifted(const Instruction *instr) {
  bool rd_is_zr = RdIsZROrSP(instr);
  bool rn_is_zr = RnIsZROrSP(instr);
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Rd, 'Rn, 'Rm'NLo";

  switch (form_hash_) {
    case "ands_32_log_shift"_h:
    case "ands_64_log_shift"_h:
      if (rd_is_zr) {
        mnemonic = "tst";
        form = "'Rn, 'Rm'NLo";
      }
      break;
    case "orr_32_log_shift"_h:
    case "orr_64_log_shift"_h:
      if (rn_is_zr && (instr->GetImmDPShift() == 0) &&
          (instr->GetShiftDP() == LSL)) {
        mnemonic = "mov";
        form = "'Rd, 'Rm";
      }
      break;
    case "orn_32_log_shift"_h:
    case "orn_64_log_shift"_h:
      if (rn_is_zr) {
        mnemonic = "mvn";
        form = "'Rd, 'Rm'NLo";
      }
      break;
  }

  Format(instr, mnemonic, form);
}


void Disassembler::VisitConditionalCompareRegister(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Rn, 'Rm, 'INzcv, 'Cond");
}


void Disassembler::VisitConditionalCompareImmediate(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Rn, 'IP, 'INzcv, 'Cond");
}


void Disassembler::VisitConditionalSelect(const Instruction *instr) {
  bool rnm_is_zr = (RnIsZROrSP(instr) && RmIsZROrSP(instr));
  bool rn_is_rm = (instr->GetRn() == instr->GetRm());
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm, 'Cond";
  const char *form_test = "'Rd, 'CInv";
  const char *form_update = "'Rd, 'Rn, 'CInv";

  Condition cond = static_cast<Condition>(instr->GetCondition());
  bool invertible_cond = (cond != al) && (cond != nv);

  switch (instr->Mask(ConditionalSelectMask)) {
    case CSEL_w:
    case CSEL_x:
      mnemonic = "csel";
      break;
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
    default:
      VIXL_UNREACHABLE();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitBitfield(const Instruction *instr) {
  unsigned s = instr->GetImmS();
  unsigned r = instr->GetImmR();
  unsigned rd_size_minus_1 =
      ((instr->GetSixtyFourBits() == 1) ? kXRegSize : kWRegSize) - 1;
  const char *mnemonic = "";
  const char *form = "";
  const char *form_shift_right = "'Rd, 'Rn, 'IBr";
  const char *form_extend = "'Rd, 'Wn";
  const char *form_bfiz = "'Rd, 'Rn, 'IBZ-r, 'IBs+1";
  const char *form_bfc = "'Rd, 'IBZ-r, 'IBs+1";
  const char *form_bfx = "'Rd, 'Rn, 'IBr, 'IBs-r+1";
  const char *form_lsl = "'Rd, 'Rn, 'IBZ-r";

  if (instr->GetSixtyFourBits() != instr->GetBitN()) {
    VisitUnallocated(instr);
    return;
  }

  if ((instr->GetSixtyFourBits() == 0) && ((s > 31) || (r > 31))) {
    VisitUnallocated(instr);
    return;
  }

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
        } else if ((s == 31) && (instr->GetSixtyFourBits() == 1)) {
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
        if (instr->GetRn() == kZeroRegCode) {
          mnemonic = "bfc";
          form = form_bfc;
        } else {
          mnemonic = "bfi";
          form = form_bfiz;
        }
      }
    }
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitExtract(const Instruction *instr) {
  const char *mnemonic = "";
  const char *form = "'Rd, 'Rn, 'Rm, 'IExtract";

  switch (instr->Mask(ExtractMask)) {
    case EXTR_w:
    case EXTR_x: {
      if (instr->GetRn() == instr->GetRm()) {
        mnemonic = "ror";
        form = "'Rd, 'Rn, 'IExtract";
      } else {
        mnemonic = "extr";
      }
      break;
    }
    default:
      VIXL_UNREACHABLE();
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitPCRelAddressing(const Instruction *instr) {
  switch (instr->Mask(PCRelAddressingMask)) {
    case ADR:
      Format(instr, "adr", "'Xd, 'AddrPCRelByte");
      break;
    case ADRP:
      Format(instr, "adrp", "'Xd, 'AddrPCRelPage");
      break;
    default:
      Format(instr, "unimplemented", "(PCRelAddressing)");
  }
}


void Disassembler::VisitConditionalBranch(const Instruction *instr) {
  // We can't use the mnemonic directly here, as there's no space between it and
  // the condition. Assert that we have the correct mnemonic, then use "b"
  // explicitly for formatting the output.
  VIXL_ASSERT(form_hash_ == "b_only_condbranch"_h);
  Format(instr, "b.'CBrn", "'TImmCond");
}


void Disassembler::VisitUnconditionalBranchToRegister(
    const Instruction *instr) {
  const char *form = "'Xn";

  switch (form_hash_) {
    case "ret_64r_branch_reg"_h:
      if (instr->GetRn() == kLinkRegCode) {
        form = "";
      }
      break;
    case "retaa_64e_branch_reg"_h:
    case "retab_64e_branch_reg"_h:
      form = "";
      break;
    case "braa_64p_branch_reg"_h:
    case "brab_64p_branch_reg"_h:
    case "blraa_64p_branch_reg"_h:
    case "blrab_64p_branch_reg"_h:
      form = "'Xn, 'Xds";
      break;
  }

  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitUnconditionalBranch(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'TImmUncn");
}


void Disassembler::VisitDataProcessing1Source(const Instruction *instr) {
  const char *form = "'Rd, 'Rn";

  switch (form_hash_) {
    case "pacia_64p_dp_1src"_h:
    case "pacda_64p_dp_1src"_h:
    case "autia_64p_dp_1src"_h:
    case "autda_64p_dp_1src"_h:
    case "pacib_64p_dp_1src"_h:
    case "pacdb_64p_dp_1src"_h:
    case "autib_64p_dp_1src"_h:
    case "autdb_64p_dp_1src"_h:
      form = "'Xd, 'Xns";
      break;
    case "paciza_64z_dp_1src"_h:
    case "pacdza_64z_dp_1src"_h:
    case "autiza_64z_dp_1src"_h:
    case "autdza_64z_dp_1src"_h:
    case "pacizb_64z_dp_1src"_h:
    case "pacdzb_64z_dp_1src"_h:
    case "autizb_64z_dp_1src"_h:
    case "autdzb_64z_dp_1src"_h:
    case "xpacd_64z_dp_1src"_h:
    case "xpaci_64z_dp_1src"_h:
      form = "'Xd";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitDataProcessing2Source(const Instruction *instr) {
  std::string mnemonic = mnemonic_;
  const char *form = "'Rd, 'Rn, 'Rm";

  switch (form_hash_) {
    case "asrv_32_dp_2src"_h:
    case "asrv_64_dp_2src"_h:
    case "lslv_32_dp_2src"_h:
    case "lslv_64_dp_2src"_h:
    case "lsrv_32_dp_2src"_h:
    case "lsrv_64_dp_2src"_h:
    case "rorv_32_dp_2src"_h:
    case "rorv_64_dp_2src"_h:
      // Drop the last 'v' character.
      VIXL_ASSERT(mnemonic[3] == 'v');
      mnemonic.pop_back();
      break;
    case "pacga_64p_dp_2src"_h:
      form = "'Xd, 'Xn, 'Xms";
      break;
    case "crc32x_64c_dp_2src"_h:
    case "crc32cx_64c_dp_2src"_h:
      form = "'Wd, 'Wn, 'Xm";
      break;
  }
  Format(instr, mnemonic.c_str(), form);
}


void Disassembler::VisitDataProcessing3Source(const Instruction *instr) {
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
    default:
      VIXL_UNREACHABLE();
  }
  Format(instr, mnemonic, form);
}

void Disassembler::DisassembleMinMaxImm(const Instruction *instr) {
  const char *suffix = (instr->ExtractBit(18) == 0) ? "'s1710" : "'u1710";
  FormatWithDecodedMnemonic(instr, "'Rd, 'Rn, #", suffix);
}

void Disassembler::VisitCompareBranch(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Rt, 'TImmCmpa");
}


void Disassembler::VisitTestBranch(const Instruction *instr) {
  // If the top bit of the immediate is clear, the tested register is
  // disassembled as Wt, otherwise Xt. As the top bit of the immediate is
  // encoded in bit 31 of the instruction, we can reuse the Rt form, which
  // uses bit 31 (normally "sf") to choose the register size.
  FormatWithDecodedMnemonic(instr, "'Rt, 'It, 'TImmTest");
}


void Disassembler::VisitMoveWideImmediate(const Instruction *instr) {
  const char *mnemonic = "";
  const char *form = "'Rd, 'IMoveImm";

  // Print the shift separately for movk, to make it clear which half word will
  // be overwritten. Movn and movz print the computed immediate, which includes
  // shift calculation.
  switch (instr->Mask(MoveWideImmediateMask)) {
    case MOVN_w:
    case MOVN_x:
      if ((instr->GetImmMoveWide()) || (instr->GetShiftMoveWide() == 0)) {
        if ((instr->GetSixtyFourBits() == 0) &&
            (instr->GetImmMoveWide() == 0xffff)) {
          mnemonic = "movn";
        } else {
          mnemonic = "mov";
          form = "'Rd, 'IMoveNeg";
        }
      } else {
        mnemonic = "movn";
      }
      break;
    case MOVZ_w:
    case MOVZ_x:
      if ((instr->GetImmMoveWide()) || (instr->GetShiftMoveWide() == 0))
        mnemonic = "mov";
      else
        mnemonic = "movz";
      break;
    case MOVK_w:
    case MOVK_x:
      mnemonic = "movk";
      form = "'Rd, 'IMoveLSL";
      break;
    default:
      VIXL_UNREACHABLE();
  }
  Format(instr, mnemonic, form);
}


#define LOAD_STORE_LIST(V) \
  V(STRB_w, "'Wt")         \
  V(STRH_w, "'Wt")         \
  V(STR_w, "'Wt")          \
  V(STR_x, "'Xt")          \
  V(LDRB_w, "'Wt")         \
  V(LDRH_w, "'Wt")         \
  V(LDR_w, "'Wt")          \
  V(LDR_x, "'Xt")          \
  V(LDRSB_x, "'Xt")        \
  V(LDRSH_x, "'Xt")        \
  V(LDRSW_x, "'Xt")        \
  V(LDRSB_w, "'Wt")        \
  V(LDRSH_w, "'Wt")        \
  V(STR_b, "'Bt")          \
  V(STR_h, "'Ht")          \
  V(STR_s, "'St")          \
  V(STR_d, "'Dt")          \
  V(LDR_b, "'Bt")          \
  V(LDR_h, "'Ht")          \
  V(LDR_s, "'St")          \
  V(LDR_d, "'Dt")          \
  V(STR_q, "'Qt")          \
  V(LDR_q, "'Qt")

void Disassembler::VisitLoadStorePreIndex(const Instruction *instr) {
  const char *form = "(LoadStorePreIndex)";
  const char *suffix = ", ['Xns'ILSi]!";

  switch (instr->Mask(LoadStorePreIndexMask)) {
#define LS_PREINDEX(A, B) \
  case A##_pre:           \
    form = B;             \
    break;
    LOAD_STORE_LIST(LS_PREINDEX)
#undef LS_PREINDEX
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}


void Disassembler::VisitLoadStorePostIndex(const Instruction *instr) {
  const char *form = "(LoadStorePostIndex)";
  const char *suffix = ", ['Xns]'ILSi";

  switch (instr->Mask(LoadStorePostIndexMask)) {
#define LS_POSTINDEX(A, B) \
  case A##_post:           \
    form = B;              \
    break;
    LOAD_STORE_LIST(LS_POSTINDEX)
#undef LS_POSTINDEX
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}


void Disassembler::VisitLoadStoreUnsignedOffset(const Instruction *instr) {
  const char *form = "(LoadStoreUnsignedOffset)";
  const char *suffix = ", ['Xns'ILU]";

  switch (instr->Mask(LoadStoreUnsignedOffsetMask)) {
#define LS_UNSIGNEDOFFSET(A, B) \
  case A##_unsigned:            \
    form = B;                   \
    break;
    LOAD_STORE_LIST(LS_UNSIGNEDOFFSET)
#undef LS_UNSIGNEDOFFSET
    case PRFM_unsigned:
      form = "'prefOp";
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}


void Disassembler::VisitLoadStoreRCpcUnscaledOffset(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Wt, ['Xns'ILS]";
  const char *form_x = "'Xt, ['Xns'ILS]";

  switch (form_hash_) {
    case "ldapursb_64_ldapstl_unscaled"_h:
    case "ldapursh_64_ldapstl_unscaled"_h:
    case "ldapursw_64_ldapstl_unscaled"_h:
    case "ldapur_64_ldapstl_unscaled"_h:
    case "stlur_64_ldapstl_unscaled"_h:
      form = form_x;
      break;
  }

  Format(instr, mnemonic, form);
}


void Disassembler::VisitLoadStoreRegisterOffset(const Instruction *instr) {
  const char *form = "(LoadStoreRegisterOffset)";
  const char *suffix = ", ['Xns, 'Offsetreg]";

  switch (instr->Mask(LoadStoreRegisterOffsetMask)) {
#define LS_REGISTEROFFSET(A, B) \
  case A##_reg:                 \
    form = B;                   \
    break;
    LOAD_STORE_LIST(LS_REGISTEROFFSET)
#undef LS_REGISTEROFFSET
    case PRFM_reg:
      form = "'prefOp";
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}


void Disassembler::VisitLoadStoreUnscaledOffset(const Instruction *instr) {
  const char *form = "'Wt";
  const char *suffix = ", ['Xns'ILS]";

  switch (form_hash_) {
    case "ldur_64_ldst_unscaled"_h:
    case "ldursb_64_ldst_unscaled"_h:
    case "ldursh_64_ldst_unscaled"_h:
    case "ldursw_64_ldst_unscaled"_h:
    case "stur_64_ldst_unscaled"_h:
      form = "'Xt";
      break;
    case "ldur_b_ldst_unscaled"_h:
    case "stur_b_ldst_unscaled"_h:
      form = "'Bt";
      break;
    case "ldur_h_ldst_unscaled"_h:
    case "stur_h_ldst_unscaled"_h:
      form = "'Ht";
      break;
    case "ldur_s_ldst_unscaled"_h:
    case "stur_s_ldst_unscaled"_h:
      form = "'St";
      break;
    case "ldur_d_ldst_unscaled"_h:
    case "stur_d_ldst_unscaled"_h:
      form = "'Dt";
      break;
    case "ldur_q_ldst_unscaled"_h:
    case "stur_q_ldst_unscaled"_h:
      form = "'Qt";
      break;
    case "prfum_p_ldst_unscaled"_h:
      form = "'prefOp";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}


void Disassembler::VisitLoadLiteral(const Instruction *instr) {
  const char *form = "'Wt";
  const char *suffix = ", 'ILLiteral 'LValue";

  switch (form_hash_) {
    case "ldr_64_loadlit"_h:
    case "ldrsw_64_loadlit"_h:
      form = "'Xt";
      break;
    case "ldr_s_loadlit"_h:
      form = "'St";
      break;
    case "ldr_d_loadlit"_h:
      form = "'Dt";
      break;
    case "ldr_q_loadlit"_h:
      form = "'Qt";
      break;
    case "prfm_p_loadlit"_h:
      form = "'prefOp";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}


#define LOAD_STORE_PAIR_LIST(V) \
  V(STP_w, "'Wt, 'Wt2", "2")    \
  V(LDP_w, "'Wt, 'Wt2", "2")    \
  V(LDPSW_x, "'Xt, 'Xt2", "2")  \
  V(STP_x, "'Xt, 'Xt2", "3")    \
  V(LDP_x, "'Xt, 'Xt2", "3")    \
  V(STP_s, "'St, 'St2", "2")    \
  V(LDP_s, "'St, 'St2", "2")    \
  V(STP_d, "'Dt, 'Dt2", "3")    \
  V(LDP_d, "'Dt, 'Dt2", "3")    \
  V(LDP_q, "'Qt, 'Qt2", "4")    \
  V(STP_q, "'Qt, 'Qt2", "4")

void Disassembler::VisitLoadStorePairPostIndex(const Instruction *instr) {
  const char *form = "(LoadStorePairPostIndex)";

  switch (instr->Mask(LoadStorePairPostIndexMask)) {
#define LSP_POSTINDEX(A, B, C)     \
  case A##_post:                   \
    form = B ", ['Xns]'ILP" C "i"; \
    break;
    LOAD_STORE_PAIR_LIST(LSP_POSTINDEX)
#undef LSP_POSTINDEX
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitLoadStorePairPreIndex(const Instruction *instr) {
  const char *form = "(LoadStorePairPreIndex)";

  switch (instr->Mask(LoadStorePairPreIndexMask)) {
#define LSP_PREINDEX(A, B, C)       \
  case A##_pre:                     \
    form = B ", ['Xns'ILP" C "i]!"; \
    break;
    LOAD_STORE_PAIR_LIST(LSP_PREINDEX)
#undef LSP_PREINDEX
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitLoadStorePairOffset(const Instruction *instr) {
  const char *form = "(LoadStorePairOffset)";

  switch (instr->Mask(LoadStorePairOffsetMask)) {
#define LSP_OFFSET(A, B, C)       \
  case A##_off:                   \
    form = B ", ['Xns'ILP" C "]"; \
    break;
    LOAD_STORE_PAIR_LIST(LSP_OFFSET)
#undef LSP_OFFSET
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitLoadStorePairNonTemporal(const Instruction *instr) {
  const char *form = "'Wt, 'Wt2, ['Xns'ILP2]";

  switch (form_hash_) {
    case "ldnp_64_ldstnapair_offs"_h:
    case "stnp_64_ldstnapair_offs"_h:
      form = "'Xt, 'Xt2, ['Xns'ILP3]";
      break;
    case "ldnp_s_ldstnapair_offs"_h:
    case "stnp_s_ldstnapair_offs"_h:
      form = "'St, 'St2, ['Xns'ILP2]";
      break;
    case "ldnp_d_ldstnapair_offs"_h:
    case "stnp_d_ldstnapair_offs"_h:
      form = "'Dt, 'Dt2, ['Xns'ILP3]";
      break;
    case "ldnp_q_ldstnapair_offs"_h:
    case "stnp_q_ldstnapair_offs"_h:
      form = "'Qt, 'Qt2, ['Xns'ILP4]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

// clang-format off
#define LOAD_STORE_EXCLUSIVE_LIST(V)   \
  V(STXRB_w,  "'Ws, 'Wt")              \
  V(STXRH_w,  "'Ws, 'Wt")              \
  V(STXR_w,   "'Ws, 'Wt")              \
  V(STXR_x,   "'Ws, 'Xt")              \
  V(LDXR_x,   "'Xt")                   \
  V(STXP_w,   "'Ws, 'Wt, 'Wt2")        \
  V(STXP_x,   "'Ws, 'Xt, 'Xt2")        \
  V(LDXP_w,   "'Wt, 'Wt2")             \
  V(LDXP_x,   "'Xt, 'Xt2")             \
  V(STLXRB_w, "'Ws, 'Wt")              \
  V(STLXRH_w, "'Ws, 'Wt")              \
  V(STLXR_w,  "'Ws, 'Wt")              \
  V(STLXR_x,  "'Ws, 'Xt")              \
  V(LDAXR_x,  "'Xt")                   \
  V(STLXP_w,  "'Ws, 'Wt, 'Wt2")        \
  V(STLXP_x,  "'Ws, 'Xt, 'Xt2")        \
  V(LDAXP_w,  "'Wt, 'Wt2")             \
  V(LDAXP_x,  "'Xt, 'Xt2")             \
  V(STLR_x,   "'Xt")                   \
  V(LDAR_x,   "'Xt")                   \
  V(STLLR_x,  "'Xt")                   \
  V(LDLAR_x,  "'Xt")                   \
  V(CAS_w,    "'Ws, 'Wt")              \
  V(CAS_x,    "'Xs, 'Xt")              \
  V(CASA_w,   "'Ws, 'Wt")              \
  V(CASA_x,   "'Xs, 'Xt")              \
  V(CASL_w,   "'Ws, 'Wt")              \
  V(CASL_x,   "'Xs, 'Xt")              \
  V(CASAL_w,  "'Ws, 'Wt")              \
  V(CASAL_x,  "'Xs, 'Xt")              \
  V(CASB,     "'Ws, 'Wt")              \
  V(CASAB,    "'Ws, 'Wt")              \
  V(CASLB,    "'Ws, 'Wt")              \
  V(CASALB,   "'Ws, 'Wt")              \
  V(CASH,     "'Ws, 'Wt")              \
  V(CASAH,    "'Ws, 'Wt")              \
  V(CASLH,    "'Ws, 'Wt")              \
  V(CASALH,   "'Ws, 'Wt")              \
  V(CASP_w,   "'Ws, 'Ws+, 'Wt, 'Wt+")  \
  V(CASP_x,   "'Xs, 'Xs+, 'Xt, 'Xt+")  \
  V(CASPA_w,  "'Ws, 'Ws+, 'Wt, 'Wt+")  \
  V(CASPA_x,  "'Xs, 'Xs+, 'Xt, 'Xt+")  \
  V(CASPL_w,  "'Ws, 'Ws+, 'Wt, 'Wt+")  \
  V(CASPL_x,  "'Xs, 'Xs+, 'Xt, 'Xt+")  \
  V(CASPAL_w, "'Ws, 'Ws+, 'Wt, 'Wt+")  \
  V(CASPAL_x, "'Xs, 'Xs+, 'Xt, 'Xt+")
// clang-format on


void Disassembler::VisitLoadStoreExclusive(const Instruction *instr) {
  const char *form = "'Wt";
  const char *suffix = ", ['Xns]";

  switch (instr->Mask(LoadStoreExclusiveMask)) {
#define LSX(A, B) \
  case A:         \
    form = B;     \
    break;
    LOAD_STORE_EXCLUSIVE_LIST(LSX)
#undef LSX
  }

  switch (instr->Mask(LoadStoreExclusiveMask)) {
    case CASP_w:
    case CASP_x:
    case CASPA_w:
    case CASPA_x:
    case CASPL_w:
    case CASPL_x:
    case CASPAL_w:
    case CASPAL_x:
      if ((instr->GetRs() % 2 == 1) || (instr->GetRt() % 2 == 1)) {
        VisitUnallocated(instr);
        return;
      }
      break;
  }

  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitLoadStorePAC(const Instruction *instr) {
  const char *form = "'Xt, ['Xns'ILA]";
  const char *suffix = "";
  switch (form_hash_) {
    case "ldraa_64w_ldst_pac"_h:
    case "ldrab_64w_ldst_pac"_h:
      suffix = "!";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitAtomicMemory(const Instruction *instr) {
  bool is_x = (instr->ExtractBits(31, 30) == 3);
  const char *form = is_x ? "'Xs, 'Xt" : "'Ws, 'Wt";
  const char *suffix = ", ['Xns]";

  std::string mnemonic = mnemonic_;

  switch (form_hash_) {
    case "ldaprb_32l_memop"_h:
    case "ldaprh_32l_memop"_h:
    case "ldapr_32l_memop"_h:
      form = "'Wt";
      break;
    case "ldapr_64l_memop"_h:
      form = "'Xt";
      break;
    default:
      // Zero register implies a store instruction.
      if (instr->GetRt() == kZeroRegCode) {
        mnemonic.replace(0, 2, "st");
        form = is_x ? "'Xs" : "'Ws";
      }
  }
  Format(instr, mnemonic.c_str(), form, suffix);
}


void Disassembler::VisitFPCompare(const Instruction *instr) {
  const char *form = "'Fn, 'Fm";
  switch (form_hash_) {
    case "fcmpe_dz_floatcmp"_h:
    case "fcmpe_hz_floatcmp"_h:
    case "fcmpe_sz_floatcmp"_h:
    case "fcmp_dz_floatcmp"_h:
    case "fcmp_hz_floatcmp"_h:
    case "fcmp_sz_floatcmp"_h:
      form = "'Fn, #0.0";
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitFPConditionalCompare(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Fn, 'Fm, 'INzcv, 'Cond");
}


void Disassembler::VisitFPConditionalSelect(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Fd, 'Fn, 'Fm, 'Cond");
}


void Disassembler::VisitFPDataProcessing1Source(const Instruction *instr) {
  const char *form = "'Fd, 'Fn";
  switch (form_hash_) {
    case "fcvt_ds_floatdp1"_h:
      form = "'Dd, 'Sn";
      break;
    case "fcvt_sd_floatdp1"_h:
      form = "'Sd, 'Dn";
      break;
    case "fcvt_hs_floatdp1"_h:
      form = "'Hd, 'Sn";
      break;
    case "fcvt_sh_floatdp1"_h:
      form = "'Sd, 'Hn";
      break;
    case "fcvt_dh_floatdp1"_h:
      form = "'Dd, 'Hn";
      break;
    case "fcvt_hd_floatdp1"_h:
      form = "'Hd, 'Dn";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitFPDataProcessing2Source(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Fd, 'Fn, 'Fm");
}


void Disassembler::VisitFPDataProcessing3Source(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Fd, 'Fn, 'Fm, 'Fa");
}


void Disassembler::VisitFPImmediate(const Instruction *instr) {
  const char *form = "'Hd";
  const char *suffix = ", 'IFP";
  switch (form_hash_) {
    case "fmov_s_floatimm"_h:
      form = "'Sd";
      break;
    case "fmov_d_floatimm"_h:
      form = "'Dd";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}


void Disassembler::VisitFPIntegerConvert(const Instruction *instr) {
  const char *form = "'Rd, 'Fn";
  switch (form_hash_) {
    case "fmov_h32_float2int"_h:
    case "fmov_h64_float2int"_h:
    case "fmov_s32_float2int"_h:
    case "fmov_d64_float2int"_h:
    case "scvtf_d32_float2int"_h:
    case "scvtf_d64_float2int"_h:
    case "scvtf_h32_float2int"_h:
    case "scvtf_h64_float2int"_h:
    case "scvtf_s32_float2int"_h:
    case "scvtf_s64_float2int"_h:
    case "ucvtf_d32_float2int"_h:
    case "ucvtf_d64_float2int"_h:
    case "ucvtf_h32_float2int"_h:
    case "ucvtf_h64_float2int"_h:
    case "ucvtf_s32_float2int"_h:
    case "ucvtf_s64_float2int"_h:
      form = "'Fd, 'Rn";
      break;
    case "fmov_v64i_float2int"_h:
      form = "'Vd.D[1], 'Rn";
      break;
    case "fmov_64vx_float2int"_h:
      form = "'Rd, 'Vn.D[1]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitFPFixedPointConvert(const Instruction *instr) {
  const char *form = "'Rd, 'Fn";
  const char *suffix = ", 'IFPFBits";

  switch (form_hash_) {
    case "scvtf_d32_float2fix"_h:
    case "scvtf_d64_float2fix"_h:
    case "scvtf_h32_float2fix"_h:
    case "scvtf_h64_float2fix"_h:
    case "scvtf_s32_float2fix"_h:
    case "scvtf_s64_float2fix"_h:
    case "ucvtf_d32_float2fix"_h:
    case "ucvtf_d64_float2fix"_h:
    case "ucvtf_h32_float2fix"_h:
    case "ucvtf_h64_float2fix"_h:
    case "ucvtf_s32_float2fix"_h:
    case "ucvtf_s64_float2fix"_h:
      form = "'Fd, 'Rn";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::DisassembleNoArgs(const Instruction *instr) {
  Format(instr, mnemonic_.c_str(), "");
}

void Disassembler::VisitSystem(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "";
  const char *suffix = NULL;

  switch (form_hash_) {
    case "clrex_bn_barriers"_h:
      form = (instr->GetCRm() == 0xf) ? "" : "'IX";
      break;
    case "mrs_rs_systemmove"_h:
      form = "'Xt, 'IY";
      break;
    case "msr_sr_systemmove"_h:
      form = "'IY, 'Xt";
      break;
    case "bti_hb_hints"_h:
      switch (instr->ExtractBits(7, 6)) {
        case 0:
          form = "";
          break;
        case 1:
          form = "c";
          break;
        case 2:
          form = "j";
          break;
        case 3:
          form = "jc";
          break;
      }
      break;
    case "chkfeat_hf_hints"_h:
      mnemonic = "chkfeat";
      form = "x16";
      break;
    case "hint_hm_hints"_h:
      form = "'IH";
      break;
    case Hash("dmb_bo_barriers"):
      form = "'M";
      break;
    case Hash("dsb_bo_barriers"): {
      int crm = instr->GetCRm();
      if (crm == 0) {
        mnemonic = "ssbb";
        form = "";
      } else if (crm == 4) {
        mnemonic = "pssbb";
        form = "";
      } else {
        form = "'M";
      }
      break;
    }
    case Hash("sys_cr_systeminstrs"): {
      const std::map<uint32_t, const char *> dcop = {
          {IVAU, "ivau"},
          {CVAC, "cvac"},
          {CVAU, "cvau"},
          {CVAP, "cvap"},
          {CVADP, "cvadp"},
          {CIVAC, "civac"},
          {ZVA, "zva"},
          {GVA, "gva"},
          {GZVA, "gzva"},
          {CGVAC, "cgvac"},
          {CGDVAC, "cgdvac"},
          {CGVAP, "cgvap"},
          {CGDVAP, "cgdvap"},
          {CIGVAC, "cigvac"},
          {CIGDVAC, "cigdvac"},
      };

      uint32_t sysop = instr->GetSysOp();
      if (dcop.count(sysop)) {
        if (sysop == IVAU) {
          mnemonic = "ic";
        } else {
          mnemonic = "dc";
        }
        form = dcop.at(sysop);
        suffix = ", 'Xt";
      } else if (sysop == GCSSS1) {
        mnemonic = "gcsss1";
        form = "'Xt";
      } else if (sysop == GCSPUSHM) {
        mnemonic = "gcspushm";
        form = "'Xt";
      } else {
        mnemonic = "sys";
        form = "'G1, 'Kn, 'Km, 'G2";
        if (instr->GetRt() < 31) {
          suffix = ", 'Xt";
        }
      }
      break;
    }
    case "sysl_rc_systeminstrs"_h:
      uint32_t sysop = instr->GetSysOp();
      if (sysop == GCSPOPM) {
        mnemonic = "gcspopm";
        form = (instr->GetRt() == 31) ? "" : "'Xt";
      } else if (sysop == GCSSS2) {
        mnemonic = "gcsss2";
        form = "'Xt";
      }
      break;
  }
  Format(instr, mnemonic, form, suffix);
}


void Disassembler::VisitException(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'IDebug";

  switch (instr->Mask(ExceptionMask)) {
    case HLT:
      mnemonic = "hlt";
      break;
    case BRK:
      mnemonic = "brk";
      break;
    case SVC:
      mnemonic = "svc";
      break;
    case HVC:
      mnemonic = "hvc";
      break;
    case SMC:
      mnemonic = "smc";
      break;
    case DCPS1:
      mnemonic = "dcps1";
      form = "{'IDebug}";
      break;
    case DCPS2:
      mnemonic = "dcps2";
      form = "{'IDebug}";
      break;
    case DCPS3:
      mnemonic = "dcps3";
      form = "{'IDebug}";
      break;
    default:
      form = "(Exception)";
  }
  Format(instr, mnemonic, form);
}


void Disassembler::VisitCrypto2RegSHA(const Instruction *instr) {
  const char *form = "'Vd.4s, 'Vn.4s";
  if (form_hash_ == "sha1h_ss_cryptosha2"_h) {
    form = "'Sd, 'Sn";
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitCrypto3RegSHA(const Instruction *instr) {
  const char *form = "'Qd, 'Sn, 'Vm.4s";
  switch (form_hash_) {
    case "sha1su0_vvv_cryptosha3"_h:
    case "sha256su1_vvv_cryptosha3"_h:
      form = "'Vd.4s, 'Vn.4s, 'Vm.4s";
      break;
    case "sha256h_qqv_cryptosha3"_h:
    case "sha256h2_qqv_cryptosha3"_h:
      form = "'Qd, 'Qn, 'Vm.4s";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}


void Disassembler::VisitCryptoAES(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Vd.16b, 'Vn.16b");
}

void Disassembler::VisitCryptoSM3(const Instruction *instr) {
  const char *form = "'Vd.4s, 'Vn.4s, 'Vm.";
  const char *suffix = "4s";

  switch (form_hash_) {
    case "sm3ss1_vvv4_crypto4"_h:
      suffix = "4s, 'Va.4s";
      break;
    case "sm3tt1a_vvv4_crypto3_imm2"_h:
    case "sm3tt1b_vvv4_crypto3_imm2"_h:
    case "sm3tt2a_vvv4_crypto3_imm2"_h:
    case "sm3tt2b_vvv_crypto3_imm2"_h:
      suffix = "s['u1312]";
      break;
  }

  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitCryptoSM4(const Instruction *instr) {
  VIXL_ASSERT((form_hash_ == "sm4ekey_vvv4_cryptosha512_3"_h) ||
              (form_hash_ == "sm4e_vv4_cryptosha512_2"_h));
  const char *form = "'Vd.4s, 'Vn.4s";
  const char *suffix =
      (form_hash_ == "sm4e_vv4_cryptosha512_2"_h) ? NULL : ", 'Vm.4s";

  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::DisassembleSHA512(const Instruction *instr) {
  const char *form = "'Qd, 'Qn, 'Vm.2d";
  const char *suffix = NULL;
  switch (form_hash_) {
    case "sha512su1_vvv2_cryptosha512_3"_h:
      suffix = ", 'Vm.2d";
      VIXL_FALLTHROUGH();
    case "sha512su0_vv2_cryptosha512_2"_h:
      form = "'Vd.2d, 'Vn.2d";
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::DisassembleNEON2RegAddlp(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s";

  static const NEONFormatMap map_lp_ta =
      {{23, 22, 30}, {NF_4H, NF_8H, NF_2S, NF_4S, NF_1D, NF_2D}};
  NEONFormatDecoder nfd(instr);
  nfd.SetFormatMap(0, &map_lp_ta);
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEON2RegCompare(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, #0";
  NEONFormatDecoder nfd(instr);
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEON2RegFPCompare(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, #0.0";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::FPFormatMap());
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEON2RegFPConvert(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s";
  static const NEONFormatMap map_cvt_ta = {{22}, {NF_4S, NF_2D}};

  static const NEONFormatMap map_cvt_tb = {{22, 30},
                                           {NF_4H, NF_8H, NF_2S, NF_4S}};
  NEONFormatDecoder nfd(instr, &map_cvt_tb, &map_cvt_ta);

  VectorFormat vform_dst = nfd.GetVectorFormat(0);
  switch (form_hash_) {
    case "fcvtl_asimdmisc_l"_h:
      nfd.SetFormatMaps(&map_cvt_ta, &map_cvt_tb);
      break;
    case "fcvtxn_asimdmisc_n"_h:
      if ((vform_dst != kFormat2S) && (vform_dst != kFormat4S)) {
        mnemonic = NULL;
      }
      break;
  }
  Format(instr, nfd.Mnemonic(mnemonic), nfd.Substitute(form));
}

void Disassembler::DisassembleNEON2RegFP(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::FPFormatMap());
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEON2RegLogical(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LogicalFormatMap());
  if (form_hash_ == "not_asimdmisc_r"_h) {
    mnemonic = "mvn";
  }
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEON2RegExtract(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s";
  const char *suffix = NULL;
  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::IntegerFormatMap(),
                        NEONFormatDecoder::LongIntegerFormatMap());

  if (form_hash_ == "shll_asimdmisc_s"_h) {
    nfd.SetFormatMaps(nfd.LongIntegerFormatMap(), nfd.IntegerFormatMap());
    switch (instr->GetNEONSize()) {
      case 0:
        suffix = ", #8";
        break;
      case 1:
        suffix = ", #16";
        break;
      case 2:
        suffix = ", #32";
        break;
    }
  }
  Format(instr, nfd.Mnemonic(mnemonic), nfd.Substitute(form), suffix);
}

void Disassembler::VisitNEON2RegMisc(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s";
  NEONFormatDecoder nfd(instr);

  VectorFormat vform_dst = nfd.GetVectorFormat(0);
  if (vform_dst != kFormatUndefined) {
    uint32_t ls_dst = LaneSizeInBitsFromFormat(vform_dst);
    switch (form_hash_) {
      case "cnt_asimdmisc_r"_h:
      case "rev16_asimdmisc_r"_h:
        if (ls_dst != kBRegSize) {
          mnemonic = NULL;
        }
        break;
      case "rev32_asimdmisc_r"_h:
        if ((ls_dst == kDRegSize) || (ls_dst == kSRegSize)) {
          mnemonic = NULL;
        }
        break;
      case "urecpe_asimdmisc_r"_h:
      case "ursqrte_asimdmisc_r"_h:
        // For urecpe and ursqrte, only S-sized elements are supported. The MSB
        // of the size field is always set by the instruction (0b1x) so we need
        // only check and discard D-sized elements here.
        VIXL_ASSERT((ls_dst == kSRegSize) || (ls_dst == kDRegSize));
        VIXL_FALLTHROUGH();
      case "clz_asimdmisc_r"_h:
      case "cls_asimdmisc_r"_h:
      case "rev64_asimdmisc_r"_h:
        if (ls_dst == kDRegSize) {
          mnemonic = NULL;
        }
        break;
    }
  }

  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::VisitNEON2RegMiscFP16(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.'?30:84h, 'Vn.'?30:84h";
  const char *suffix = NULL;

  switch (form_hash_) {
    case "fcmeq_asimdmiscfp16_fz"_h:
    case "fcmge_asimdmiscfp16_fz"_h:
    case "fcmgt_asimdmiscfp16_fz"_h:
    case "fcmle_asimdmiscfp16_fz"_h:
    case "fcmlt_asimdmiscfp16_fz"_h:
      suffix = ", #0.0";
  }
  Format(instr, mnemonic, form, suffix);
}

void Disassembler::DisassembleNEON3SameLogical(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LogicalFormatMap());

  switch (form_hash_) {
    case "orr_asimdsame_only"_h:
      if (instr->GetRm() == instr->GetRn()) {
        mnemonic = "mov";
        form = "'Vd.%s, 'Vn.%s";
      }
      break;
    case "pmul_asimdsame_only"_h:
      if (instr->GetNEONSize() != 0) {
        mnemonic = NULL;
      }
  }
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEON3SameFHM(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Vd.'?30:42s, 'Vn.'?30:42h, 'Vm.'?30:42h");
}

void Disassembler::DisassembleNEON3SameNoD(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s";
  static const NEONFormatMap map =
      {{23, 22, 30},
       {NF_8B, NF_16B, NF_4H, NF_8H, NF_2S, NF_4S, NF_UNDEF, NF_UNDEF}};
  NEONFormatDecoder nfd(instr, &map);
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::VisitNEON3Same(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s";
  NEONFormatDecoder nfd(instr);

  if (instr->Mask(NEON3SameFPFMask) == NEON3SameFPFixed) {
    nfd.SetFormatMaps(nfd.FPFormatMap());
  }

  VectorFormat vform_dst = nfd.GetVectorFormat(0);
  if (vform_dst != kFormatUndefined) {
    uint32_t ls_dst = LaneSizeInBitsFromFormat(vform_dst);
    switch (form_hash_) {
      case "sqdmulh_asimdsame_only"_h:
      case "sqrdmulh_asimdsame_only"_h:
        if ((ls_dst == kBRegSize) || (ls_dst == kDRegSize)) {
          mnemonic = NULL;
        }
        break;
    }
  }
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::VisitNEON3SameFP16(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s";
  NEONFormatDecoder nfd(instr);
  nfd.SetFormatMaps(nfd.FP16FormatMap());
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::VisitNEON3SameExtra(const Instruction *instr) {
  static const NEONFormatMap map_dot =
      {{23, 22, 30}, {NF_UNDEF, NF_UNDEF, NF_UNDEF, NF_UNDEF, NF_2S, NF_4S}};
  static const NEONFormatMap map_fc =
      {{23, 22, 30},
       {NF_UNDEF, NF_UNDEF, NF_4H, NF_8H, NF_2S, NF_4S, NF_UNDEF, NF_2D}};
  static const NEONFormatMap map_rdm =
      {{23, 22, 30}, {NF_UNDEF, NF_UNDEF, NF_4H, NF_8H, NF_2S, NF_4S}};

  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s";
  const char *suffix = NULL;

  NEONFormatDecoder nfd(instr, &map_fc);

  switch (form_hash_) {
    case "fcmla_asimdsame2_c"_h:
      suffix = ", #'u1211*90";
      break;
    case "fcadd_asimdsame2_c"_h:
      // Bit 10 is always set, so this gives 90 * 1 or 3.
      suffix = ", #'u1212:1010*90";
      break;
    case "sdot_asimdsame2_d"_h:
    case "udot_asimdsame2_d"_h:
    case "usdot_asimdsame2_d"_h:
      nfd.SetFormatMaps(nfd.LogicalFormatMap());
      nfd.SetFormatMap(0, &map_dot);
      break;
    default:
      nfd.SetFormatMaps(&map_rdm);
      break;
  }

  Format(instr, mnemonic, nfd.Substitute(form), suffix);
}

void Disassembler::DisassembleNEON4Same(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Vd.16b, 'Vn.16b, 'Vm.16b, 'Va.16b");
}

void Disassembler::DisassembleNEONXar(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Vd.2d, 'Vn.2d, 'Vm.2d, #'u1510");
}

void Disassembler::DisassembleNEONRax1(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Vd.2d, 'Vn.2d, 'Vm.2d");
}

void Disassembler::VisitNEON3Different(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s";

  NEONFormatDecoder nfd(instr);
  nfd.SetFormatMap(0, nfd.LongIntegerFormatMap());

  switch (form_hash_) {
    case "saddw_asimddiff_w"_h:
    case "ssubw_asimddiff_w"_h:
    case "uaddw_asimddiff_w"_h:
    case "usubw_asimddiff_w"_h:
      nfd.SetFormatMap(1, nfd.LongIntegerFormatMap());
      break;
    case "addhn_asimddiff_n"_h:
    case "raddhn_asimddiff_n"_h:
    case "rsubhn_asimddiff_n"_h:
    case "subhn_asimddiff_n"_h:
      nfd.SetFormatMaps(nfd.LongIntegerFormatMap());
      nfd.SetFormatMap(0, nfd.IntegerFormatMap());
      break;
    case "sqdmlal_asimddiff_l"_h:
    case "sqdmlsl_asimddiff_l"_h:
    case "sqdmull_asimddiff_l"_h:
      if (nfd.GetVectorFormat(0) == kFormat8H) {
        mnemonic = NULL;
      }
      break;
  }
  Format(instr, nfd.Mnemonic(mnemonic), nfd.Substitute(form));
}

void Disassembler::DisassembleNEONPolynomialMul(const Instruction *instr) {
  const char *mnemonic = instr->ExtractBit(30) ? "pmull2" : "pmull";
  const char *form = NULL;
  int size = instr->ExtractBits(23, 22);
  if (size == 0) {
    // Bits 30:27 of the instruction are x001, where x is the Q bit. Map
    // this to "8" and "16" by adding 7.
    form = "'Vd.8h, 'Vn.'u3127+7b, 'Vm.'u3127+7b";
  } else if (size == 3) {
    form = "'Vd.1q, 'Vn.'?30:21d, 'Vm.'?30:21d";
  } else {
    mnemonic = NULL;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::DisassembleNEONFPAcrossLanes(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Sd, 'Vn.4s";
  if ((instr->GetNEONQ() == 0) || (instr->ExtractBit(22) == 1)) {
    mnemonic = NULL;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::DisassembleNEONFP16AcrossLanes(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Hd, 'Vn.'?30:84h");
}

void Disassembler::VisitNEONAcrossLanes(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, 'Vn.%s";

  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::ScalarFormatMap(),
                        NEONFormatDecoder::IntegerFormatMap());

  switch (form_hash_) {
    case "saddlv_asimdall_only"_h:
    case "uaddlv_asimdall_only"_h:
      nfd.SetFormatMap(0, nfd.LongScalarFormatMap());
  }

  VectorFormat vform_src = nfd.GetVectorFormat(1);
  if ((vform_src == kFormat2S) || (vform_src == kFormat2D)) {
    mnemonic = NULL;
  }

  Format(instr,
         mnemonic,
         nfd.Substitute(form,
                        NEONFormatDecoder::kPlaceholder,
                        NEONFormatDecoder::kFormat));
}

void Disassembler::VisitNEONByIndexedElement(const Instruction *instr) {
  const char *form = "'Vd.%s, 'Vn.%s, 'Vf.%s['IVByElemIndex]";
  static const NEONFormatMap map_v =
      {{23, 22, 30},
       {NF_UNDEF, NF_UNDEF, NF_4H, NF_8H, NF_2S, NF_4S, NF_UNDEF, NF_UNDEF}};
  static const NEONFormatMap map_s = {{23, 22},
                                      {NF_UNDEF, NF_H, NF_S, NF_UNDEF}};
  NEONFormatDecoder nfd(instr, &map_v, &map_v, &map_s);
  Format(instr, mnemonic_.c_str(), nfd.Substitute(form));
}

void Disassembler::DisassembleNEONMulByElementLong(const Instruction *instr) {
  const char *form = "'Vd.%s, 'Vn.%s, 'Vf.%s['IVByElemIndex]";
  // TODO: Disallow undefined element types for this instruction.
  static const NEONFormatMap map_ta = {{23, 22}, {NF_UNDEF, NF_4S, NF_2D}};
  NEONFormatDecoder nfd(instr,
                        &map_ta,
                        NEONFormatDecoder::IntegerFormatMap(),
                        NEONFormatDecoder::ScalarFormatMap());
  Format(instr, nfd.Mnemonic(mnemonic_.c_str()), nfd.Substitute(form));
}

void Disassembler::DisassembleNEONDotProdByElement(const Instruction *instr) {
  const char *form = instr->ExtractBit(30) ? "'Vd.4s, 'Vn.16" : "'Vd.2s, 'Vn.8";
  const char *suffix = "b, 'Vm.4b['u1111:2121]";
  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::DisassembleNEONFPMulByElement(const Instruction *instr) {
  const char *form = "'Vd.%s, 'Vn.%s, 'Vf.%s['IVByElemIndex]";
  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::FPFormatMap(),
                        NEONFormatDecoder::FPFormatMap(),
                        NEONFormatDecoder::FPScalarFormatMap());
  Format(instr, mnemonic_.c_str(), nfd.Substitute(form));
}

void Disassembler::DisassembleNEONHalfFPMulByElement(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "'Vd.'?30:84h, 'Vn.'?30:84h, "
                            "'Ve.h['IVByElemIndex]");
}

void Disassembler::DisassembleNEONFPMulByElementLong(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "'Vd.'?30:42s, 'Vn.'?30:42h, "
                            "'Ve.h['IVByElemIndexFHM]");
}

void Disassembler::DisassembleNEONComplexMulByElement(
    const Instruction *instr) {
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s['IVByElemIndexRot], #'u1413*90";
  // TODO: Disallow undefined element types for this instruction.
  static const NEONFormatMap map_cn =
      {{23, 22, 30},
       {NF_UNDEF, NF_UNDEF, NF_4H, NF_8H, NF_UNDEF, NF_4S, NF_UNDEF, NF_UNDEF}};
  NEONFormatDecoder nfd(instr,
                        &map_cn,
                        &map_cn,
                        NEONFormatDecoder::ScalarFormatMap());
  Format(instr, mnemonic_.c_str(), nfd.Substitute(form));
}

void Disassembler::VisitNEONCopy(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "(NEONCopy)";

  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::TriangularFormatMap(),
                        NEONFormatDecoder::TriangularScalarFormatMap());

  switch (form_hash_) {
    case "ins_asimdins_iv_v"_h:
      mnemonic = "mov";
      nfd.SetFormatMap(0, nfd.TriangularScalarFormatMap());
      form = "'Vd.%s['IVInsIndex1], 'Vn.%s['IVInsIndex2]";
      break;
    case "ins_asimdins_ir_r"_h:
      mnemonic = "mov";
      nfd.SetFormatMap(0, nfd.TriangularScalarFormatMap());
      if (nfd.GetVectorFormat() == kFormatD) {
        form = "'Vd.%s['IVInsIndex1], 'Xn";
      } else {
        form = "'Vd.%s['IVInsIndex1], 'Wn";
      }
      break;
    case "umov_asimdins_w_w"_h:
    case "umov_asimdins_x_x"_h:
      if (instr->Mask(NEON_Q) || ((instr->GetImmNEON5() & 7) == 4)) {
        mnemonic = "mov";
      }
      nfd.SetFormatMap(0, nfd.TriangularScalarFormatMap());
      if (nfd.GetVectorFormat() == kFormatD) {
        form = "'Xd, 'Vn.%s['IVInsIndex1]";
      } else {
        form = "'Wd, 'Vn.%s['IVInsIndex1]";
      }
      break;
    case "smov_asimdins_w_w"_h:
    case "smov_asimdins_x_x"_h: {
      nfd.SetFormatMap(0, nfd.TriangularScalarFormatMap());
      VectorFormat vform = nfd.GetVectorFormat();
      if ((vform == kFormatD) ||
          ((vform == kFormatS) && (instr->ExtractBit(30) == 0))) {
        mnemonic = NULL;
      }
      form = "'R30d, 'Vn.%s['IVInsIndex1]";
      break;
    }
    case "dup_asimdins_dv_v"_h:
      form = "'Vd.%s, 'Vn.%s['IVInsIndex1]";
      break;
    case "dup_asimdins_dr_r"_h:
      if (nfd.GetVectorFormat() == kFormat2D) {
        form = "'Vd.%s, 'Xn";
      } else {
        form = "'Vd.%s, 'Wn";
      }
  }
  Format(instr, mnemonic, nfd.Substitute(form));
}


void Disassembler::VisitNEONExtract(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'Vm.%s, 'IVExtract";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LogicalFormatMap());
  if ((instr->GetImmNEONExt() > 7) && (instr->GetNEONQ() == 0)) {
    mnemonic = NULL;
  }
  Format(instr, mnemonic, nfd.Substitute(form));
}


void Disassembler::VisitNEONLoadStoreMultiStruct(const Instruction *instr) {
  const char *mnemonic = NULL;
  const char *form = NULL;
  const char *form_1v = "{'Vt.%s}, ['Xns]";
  const char *form_2v = "{'Vt.%s, 'Vt2.%s}, ['Xns]";
  const char *form_3v = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s}, ['Xns]";
  const char *form_4v = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s, 'Vt4.%s}, ['Xns]";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LoadStoreFormatMap());

  switch (instr->Mask(NEONLoadStoreMultiStructMask)) {
    case NEON_LD1_1v:
      mnemonic = "ld1";
      form = form_1v;
      break;
    case NEON_LD1_2v:
      mnemonic = "ld1";
      form = form_2v;
      break;
    case NEON_LD1_3v:
      mnemonic = "ld1";
      form = form_3v;
      break;
    case NEON_LD1_4v:
      mnemonic = "ld1";
      form = form_4v;
      break;
    case NEON_LD2:
      mnemonic = "ld2";
      form = form_2v;
      break;
    case NEON_LD3:
      mnemonic = "ld3";
      form = form_3v;
      break;
    case NEON_LD4:
      mnemonic = "ld4";
      form = form_4v;
      break;
    case NEON_ST1_1v:
      mnemonic = "st1";
      form = form_1v;
      break;
    case NEON_ST1_2v:
      mnemonic = "st1";
      form = form_2v;
      break;
    case NEON_ST1_3v:
      mnemonic = "st1";
      form = form_3v;
      break;
    case NEON_ST1_4v:
      mnemonic = "st1";
      form = form_4v;
      break;
    case NEON_ST2:
      mnemonic = "st2";
      form = form_2v;
      break;
    case NEON_ST3:
      mnemonic = "st3";
      form = form_3v;
      break;
    case NEON_ST4:
      mnemonic = "st4";
      form = form_4v;
      break;
    default:
      break;
  }

  // Work out unallocated encodings.
  bool allocated = (mnemonic != NULL);
  switch (instr->Mask(NEONLoadStoreMultiStructMask)) {
    case NEON_LD2:
    case NEON_LD3:
    case NEON_LD4:
    case NEON_ST2:
    case NEON_ST3:
    case NEON_ST4:
      // LD[2-4] and ST[2-4] cannot use .1d format.
      allocated = (instr->GetNEONQ() != 0) || (instr->GetNEONLSSize() != 3);
      break;
    default:
      break;
  }
  if (allocated) {
    VIXL_ASSERT(mnemonic != NULL);
    VIXL_ASSERT(form != NULL);
  } else {
    mnemonic = "unallocated";
    form = "(NEONLoadStoreMultiStruct)";
  }

  Format(instr, mnemonic, nfd.Substitute(form));
}


void Disassembler::VisitNEONLoadStoreMultiStructPostIndex(
    const Instruction *instr) {
  const char *mnemonic = NULL;
  const char *form = NULL;
  const char *form_1v = "{'Vt.%s}, ['Xns], 'Xmr1";
  const char *form_2v = "{'Vt.%s, 'Vt2.%s}, ['Xns], 'Xmr2";
  const char *form_3v = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s}, ['Xns], 'Xmr3";
  const char *form_4v = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s, 'Vt4.%s}, ['Xns], 'Xmr4";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LoadStoreFormatMap());

  switch (instr->Mask(NEONLoadStoreMultiStructPostIndexMask)) {
    case NEON_LD1_1v_post:
      mnemonic = "ld1";
      form = form_1v;
      break;
    case NEON_LD1_2v_post:
      mnemonic = "ld1";
      form = form_2v;
      break;
    case NEON_LD1_3v_post:
      mnemonic = "ld1";
      form = form_3v;
      break;
    case NEON_LD1_4v_post:
      mnemonic = "ld1";
      form = form_4v;
      break;
    case NEON_LD2_post:
      mnemonic = "ld2";
      form = form_2v;
      break;
    case NEON_LD3_post:
      mnemonic = "ld3";
      form = form_3v;
      break;
    case NEON_LD4_post:
      mnemonic = "ld4";
      form = form_4v;
      break;
    case NEON_ST1_1v_post:
      mnemonic = "st1";
      form = form_1v;
      break;
    case NEON_ST1_2v_post:
      mnemonic = "st1";
      form = form_2v;
      break;
    case NEON_ST1_3v_post:
      mnemonic = "st1";
      form = form_3v;
      break;
    case NEON_ST1_4v_post:
      mnemonic = "st1";
      form = form_4v;
      break;
    case NEON_ST2_post:
      mnemonic = "st2";
      form = form_2v;
      break;
    case NEON_ST3_post:
      mnemonic = "st3";
      form = form_3v;
      break;
    case NEON_ST4_post:
      mnemonic = "st4";
      form = form_4v;
      break;
    default:
      break;
  }

  // Work out unallocated encodings.
  bool allocated = (mnemonic != NULL);
  switch (instr->Mask(NEONLoadStoreMultiStructPostIndexMask)) {
    case NEON_LD2_post:
    case NEON_LD3_post:
    case NEON_LD4_post:
    case NEON_ST2_post:
    case NEON_ST3_post:
    case NEON_ST4_post:
      // LD[2-4] and ST[2-4] cannot use .1d format.
      allocated = (instr->GetNEONQ() != 0) || (instr->GetNEONLSSize() != 3);
      break;
    default:
      break;
  }
  if (allocated) {
    VIXL_ASSERT(mnemonic != NULL);
    VIXL_ASSERT(form != NULL);
  } else {
    mnemonic = "unallocated";
    form = "(NEONLoadStoreMultiStructPostIndex)";
  }

  Format(instr, mnemonic, nfd.Substitute(form));
}


void Disassembler::VisitNEONLoadStoreSingleStruct(const Instruction *instr) {
  const char *mnemonic = NULL;
  const char *form = NULL;

  const char *form_1b = "{'Vt.b}['IVLSLane0], ['Xns]";
  const char *form_1h = "{'Vt.h}['IVLSLane1], ['Xns]";
  const char *form_1s = "{'Vt.s}['IVLSLane2], ['Xns]";
  const char *form_1d = "{'Vt.d}['IVLSLane3], ['Xns]";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LoadStoreFormatMap());

  switch (instr->Mask(NEONLoadStoreSingleStructMask)) {
    case NEON_LD1_b:
      mnemonic = "ld1";
      form = form_1b;
      break;
    case NEON_LD1_h:
      mnemonic = "ld1";
      form = form_1h;
      break;
    case NEON_LD1_s:
      mnemonic = "ld1";
      VIXL_STATIC_ASSERT((NEON_LD1_s | (1 << NEONLSSize_offset)) == NEON_LD1_d);
      form = ((instr->GetNEONLSSize() & 1) == 0) ? form_1s : form_1d;
      break;
    case NEON_ST1_b:
      mnemonic = "st1";
      form = form_1b;
      break;
    case NEON_ST1_h:
      mnemonic = "st1";
      form = form_1h;
      break;
    case NEON_ST1_s:
      mnemonic = "st1";
      VIXL_STATIC_ASSERT((NEON_ST1_s | (1 << NEONLSSize_offset)) == NEON_ST1_d);
      form = ((instr->GetNEONLSSize() & 1) == 0) ? form_1s : form_1d;
      break;
    case NEON_LD1R:
      mnemonic = "ld1r";
      form = "{'Vt.%s}, ['Xns]";
      break;
    case NEON_LD2_b:
    case NEON_ST2_b:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld2" : "st2";
      form = "{'Vt.b, 'Vt2.b}['IVLSLane0], ['Xns]";
      break;
    case NEON_LD2_h:
    case NEON_ST2_h:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld2" : "st2";
      form = "{'Vt.h, 'Vt2.h}['IVLSLane1], ['Xns]";
      break;
    case NEON_LD2_s:
    case NEON_ST2_s:
      VIXL_STATIC_ASSERT((NEON_ST2_s | (1 << NEONLSSize_offset)) == NEON_ST2_d);
      VIXL_STATIC_ASSERT((NEON_LD2_s | (1 << NEONLSSize_offset)) == NEON_LD2_d);
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld2" : "st2";
      if ((instr->GetNEONLSSize() & 1) == 0) {
        form = "{'Vt.s, 'Vt2.s}['IVLSLane2], ['Xns]";
      } else {
        form = "{'Vt.d, 'Vt2.d}['IVLSLane3], ['Xns]";
      }
      break;
    case NEON_LD2R:
      mnemonic = "ld2r";
      form = "{'Vt.%s, 'Vt2.%s}, ['Xns]";
      break;
    case NEON_LD3_b:
    case NEON_ST3_b:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld3" : "st3";
      form = "{'Vt.b, 'Vt2.b, 'Vt3.b}['IVLSLane0], ['Xns]";
      break;
    case NEON_LD3_h:
    case NEON_ST3_h:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld3" : "st3";
      form = "{'Vt.h, 'Vt2.h, 'Vt3.h}['IVLSLane1], ['Xns]";
      break;
    case NEON_LD3_s:
    case NEON_ST3_s:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld3" : "st3";
      if ((instr->GetNEONLSSize() & 1) == 0) {
        form = "{'Vt.s, 'Vt2.s, 'Vt3.s}['IVLSLane2], ['Xns]";
      } else {
        form = "{'Vt.d, 'Vt2.d, 'Vt3.d}['IVLSLane3], ['Xns]";
      }
      break;
    case NEON_LD3R:
      mnemonic = "ld3r";
      form = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s}, ['Xns]";
      break;
    case NEON_LD4_b:
    case NEON_ST4_b:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld4" : "st4";
      form = "{'Vt.b, 'Vt2.b, 'Vt3.b, 'Vt4.b}['IVLSLane0], ['Xns]";
      break;
    case NEON_LD4_h:
    case NEON_ST4_h:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld4" : "st4";
      form = "{'Vt.h, 'Vt2.h, 'Vt3.h, 'Vt4.h}['IVLSLane1], ['Xns]";
      break;
    case NEON_LD4_s:
    case NEON_ST4_s:
      VIXL_STATIC_ASSERT((NEON_LD4_s | (1 << NEONLSSize_offset)) == NEON_LD4_d);
      VIXL_STATIC_ASSERT((NEON_ST4_s | (1 << NEONLSSize_offset)) == NEON_ST4_d);
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld4" : "st4";
      if ((instr->GetNEONLSSize() & 1) == 0) {
        form = "{'Vt.s, 'Vt2.s, 'Vt3.s, 'Vt4.s}['IVLSLane2], ['Xns]";
      } else {
        form = "{'Vt.d, 'Vt2.d, 'Vt3.d, 'Vt4.d}['IVLSLane3], ['Xns]";
      }
      break;
    case NEON_LD4R:
      mnemonic = "ld4r";
      form = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s, 'Vt4.%s}, ['Xns]";
      break;
    default:
      break;
  }

  // Work out unallocated encodings.
  bool allocated = (mnemonic != NULL);
  switch (instr->Mask(NEONLoadStoreSingleStructMask)) {
    case NEON_LD1_h:
    case NEON_LD2_h:
    case NEON_LD3_h:
    case NEON_LD4_h:
    case NEON_ST1_h:
    case NEON_ST2_h:
    case NEON_ST3_h:
    case NEON_ST4_h:
      VIXL_ASSERT(allocated);
      allocated = ((instr->GetNEONLSSize() & 1) == 0);
      break;
    case NEON_LD1_s:
    case NEON_LD2_s:
    case NEON_LD3_s:
    case NEON_LD4_s:
    case NEON_ST1_s:
    case NEON_ST2_s:
    case NEON_ST3_s:
    case NEON_ST4_s:
      VIXL_ASSERT(allocated);
      allocated = (instr->GetNEONLSSize() <= 1) &&
                  ((instr->GetNEONLSSize() == 0) || (instr->GetNEONS() == 0));
      break;
    case NEON_LD1R:
    case NEON_LD2R:
    case NEON_LD3R:
    case NEON_LD4R:
      VIXL_ASSERT(allocated);
      allocated = (instr->GetNEONS() == 0);
      break;
    default:
      break;
  }
  if (allocated) {
    VIXL_ASSERT(mnemonic != NULL);
    VIXL_ASSERT(form != NULL);
  } else {
    mnemonic = "unallocated";
    form = "(NEONLoadStoreSingleStruct)";
  }

  Format(instr, mnemonic, nfd.Substitute(form));
}


void Disassembler::VisitNEONLoadStoreSingleStructPostIndex(
    const Instruction *instr) {
  const char *mnemonic = NULL;
  const char *form = NULL;

  const char *form_1b = "{'Vt.b}['IVLSLane0], ['Xns], 'Xmb1";
  const char *form_1h = "{'Vt.h}['IVLSLane1], ['Xns], 'Xmb2";
  const char *form_1s = "{'Vt.s}['IVLSLane2], ['Xns], 'Xmb4";
  const char *form_1d = "{'Vt.d}['IVLSLane3], ['Xns], 'Xmb8";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LoadStoreFormatMap());

  switch (instr->Mask(NEONLoadStoreSingleStructPostIndexMask)) {
    case NEON_LD1_b_post:
      mnemonic = "ld1";
      form = form_1b;
      break;
    case NEON_LD1_h_post:
      mnemonic = "ld1";
      form = form_1h;
      break;
    case NEON_LD1_s_post:
      mnemonic = "ld1";
      VIXL_STATIC_ASSERT((NEON_LD1_s | (1 << NEONLSSize_offset)) == NEON_LD1_d);
      form = ((instr->GetNEONLSSize() & 1) == 0) ? form_1s : form_1d;
      break;
    case NEON_ST1_b_post:
      mnemonic = "st1";
      form = form_1b;
      break;
    case NEON_ST1_h_post:
      mnemonic = "st1";
      form = form_1h;
      break;
    case NEON_ST1_s_post:
      mnemonic = "st1";
      VIXL_STATIC_ASSERT((NEON_ST1_s | (1 << NEONLSSize_offset)) == NEON_ST1_d);
      form = ((instr->GetNEONLSSize() & 1) == 0) ? form_1s : form_1d;
      break;
    case NEON_LD1R_post:
      mnemonic = "ld1r";
      form = "{'Vt.%s}, ['Xns], 'Xmz1";
      break;
    case NEON_LD2_b_post:
    case NEON_ST2_b_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld2" : "st2";
      form = "{'Vt.b, 'Vt2.b}['IVLSLane0], ['Xns], 'Xmb2";
      break;
    case NEON_ST2_h_post:
    case NEON_LD2_h_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld2" : "st2";
      form = "{'Vt.h, 'Vt2.h}['IVLSLane1], ['Xns], 'Xmb4";
      break;
    case NEON_LD2_s_post:
    case NEON_ST2_s_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld2" : "st2";
      if ((instr->GetNEONLSSize() & 1) == 0)
        form = "{'Vt.s, 'Vt2.s}['IVLSLane2], ['Xns], 'Xmb8";
      else
        form = "{'Vt.d, 'Vt2.d}['IVLSLane3], ['Xns], 'Xmb16";
      break;
    case NEON_LD2R_post:
      mnemonic = "ld2r";
      form = "{'Vt.%s, 'Vt2.%s}, ['Xns], 'Xmz2";
      break;
    case NEON_LD3_b_post:
    case NEON_ST3_b_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld3" : "st3";
      form = "{'Vt.b, 'Vt2.b, 'Vt3.b}['IVLSLane0], ['Xns], 'Xmb3";
      break;
    case NEON_LD3_h_post:
    case NEON_ST3_h_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld3" : "st3";
      form = "{'Vt.h, 'Vt2.h, 'Vt3.h}['IVLSLane1], ['Xns], 'Xmb6";
      break;
    case NEON_LD3_s_post:
    case NEON_ST3_s_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld3" : "st3";
      if ((instr->GetNEONLSSize() & 1) == 0)
        form = "{'Vt.s, 'Vt2.s, 'Vt3.s}['IVLSLane2], ['Xns], 'Xmb12";
      else
        form = "{'Vt.d, 'Vt2.d, 'Vt3.d}['IVLSLane3], ['Xns], 'Xmb24";
      break;
    case NEON_LD3R_post:
      mnemonic = "ld3r";
      form = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s}, ['Xns], 'Xmz3";
      break;
    case NEON_LD4_b_post:
    case NEON_ST4_b_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld4" : "st4";
      form = "{'Vt.b, 'Vt2.b, 'Vt3.b, 'Vt4.b}['IVLSLane0], ['Xns], 'Xmb4";
      break;
    case NEON_LD4_h_post:
    case NEON_ST4_h_post:
      mnemonic = (instr->GetLdStXLoad()) == 1 ? "ld4" : "st4";
      form = "{'Vt.h, 'Vt2.h, 'Vt3.h, 'Vt4.h}['IVLSLane1], ['Xns], 'Xmb8";
      break;
    case NEON_LD4_s_post:
    case NEON_ST4_s_post:
      mnemonic = (instr->GetLdStXLoad() == 1) ? "ld4" : "st4";
      if ((instr->GetNEONLSSize() & 1) == 0)
        form = "{'Vt.s, 'Vt2.s, 'Vt3.s, 'Vt4.s}['IVLSLane2], ['Xns], 'Xmb16";
      else
        form = "{'Vt.d, 'Vt2.d, 'Vt3.d, 'Vt4.d}['IVLSLane3], ['Xns], 'Xmb32";
      break;
    case NEON_LD4R_post:
      mnemonic = "ld4r";
      form = "{'Vt.%s, 'Vt2.%s, 'Vt3.%s, 'Vt4.%s}, ['Xns], 'Xmz4";
      break;
    default:
      break;
  }

  // Work out unallocated encodings.
  bool allocated = (mnemonic != NULL);
  switch (instr->Mask(NEONLoadStoreSingleStructPostIndexMask)) {
    case NEON_LD1_h_post:
    case NEON_LD2_h_post:
    case NEON_LD3_h_post:
    case NEON_LD4_h_post:
    case NEON_ST1_h_post:
    case NEON_ST2_h_post:
    case NEON_ST3_h_post:
    case NEON_ST4_h_post:
      VIXL_ASSERT(allocated);
      allocated = ((instr->GetNEONLSSize() & 1) == 0);
      break;
    case NEON_LD1_s_post:
    case NEON_LD2_s_post:
    case NEON_LD3_s_post:
    case NEON_LD4_s_post:
    case NEON_ST1_s_post:
    case NEON_ST2_s_post:
    case NEON_ST3_s_post:
    case NEON_ST4_s_post:
      VIXL_ASSERT(allocated);
      allocated = (instr->GetNEONLSSize() <= 1) &&
                  ((instr->GetNEONLSSize() == 0) || (instr->GetNEONS() == 0));
      break;
    case NEON_LD1R_post:
    case NEON_LD2R_post:
    case NEON_LD3R_post:
    case NEON_LD4R_post:
      VIXL_ASSERT(allocated);
      allocated = (instr->GetNEONS() == 0);
      break;
    default:
      break;
  }
  if (allocated) {
    VIXL_ASSERT(mnemonic != NULL);
    VIXL_ASSERT(form != NULL);
  } else {
    mnemonic = "unallocated";
    form = "(NEONLoadStoreSingleStructPostIndex)";
  }

  Format(instr, mnemonic, nfd.Substitute(form));
}


void Disassembler::VisitNEONModifiedImmediate(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vt.%s, 'IVMIImm8, lsl 'IVMIShiftAmt1";

  static const NEONFormatMap map_h = {{30}, {NF_4H, NF_8H}};
  static const NEONFormatMap map_s = {{30}, {NF_2S, NF_4S}};
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LogicalFormatMap());

  switch (form_hash_) {
    case "movi_asimdimm_n_b"_h:
      form = "'Vt.%s, 'IVMIImm8";
      break;
    case "bic_asimdimm_l_hl"_h:
    case "movi_asimdimm_l_hl"_h:
    case "mvni_asimdimm_l_hl"_h:
    case "orr_asimdimm_l_hl"_h:
      nfd.SetFormatMap(0, &map_h);
      break;
    case "movi_asimdimm_m_sm"_h:
    case "mvni_asimdimm_m_sm"_h:
      form = "'Vt.%s, 'IVMIImm8, msl 'IVMIShiftAmt2";
      VIXL_FALLTHROUGH();
    case "bic_asimdimm_l_sl"_h:
    case "movi_asimdimm_l_sl"_h:
    case "mvni_asimdimm_l_sl"_h:
    case "orr_asimdimm_l_sl"_h:
      nfd.SetFormatMap(0, &map_s);
      break;
    case "movi_asimdimm_d_ds"_h:
      form = "'Dd, 'IVMIImm";
      break;
    case "movi_asimdimm_d2_d"_h:
      form = "'Vt.2d, 'IVMIImm";
      break;
    case "fmov_asimdimm_h_h"_h:
      form = "'Vt.%s, 'IFPNeon";
      nfd.SetFormatMap(0, &map_h);
      break;
    case "fmov_asimdimm_s_s"_h:
      form = "'Vt.%s, 'IFPNeon";
      nfd.SetFormatMap(0, &map_s);
      break;
    case "fmov_asimdimm_d2_d"_h:
      form = "'Vt.2d, 'IFPNeon";
      break;
  }

  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEONScalar2RegMiscOnlyD(
    const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Dd, 'Dn";
  const char *suffix = ", #0";
  if (instr->GetNEONSize() != 3) {
    mnemonic = NULL;
  }
  switch (form_hash_) {
    case "abs_asisdmisc_r"_h:
    case "neg_asisdmisc_r"_h:
      suffix = NULL;
  }
  Format(instr, mnemonic, form, suffix);
}

void Disassembler::DisassembleNEONFPScalar2RegMisc(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn";
  const char *suffix = NULL;
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::FPScalarFormatMap());
  switch (form_hash_) {
    case "fcmeq_asisdmisc_fz"_h:
    case "fcmge_asisdmisc_fz"_h:
    case "fcmgt_asisdmisc_fz"_h:
    case "fcmle_asisdmisc_fz"_h:
    case "fcmlt_asisdmisc_fz"_h:
      suffix = ", #0.0";
      break;
    case "fcvtxn_asisdmisc_n"_h:
      if (nfd.GetVectorFormat(0) == kFormatS) {  // Source format.
        mnemonic = NULL;
      }
      form = "'Sd, 'Dn";
  }
  Format(instr, mnemonic, nfd.SubstitutePlaceholders(form), suffix);
}

void Disassembler::VisitNEONScalar2RegMisc(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::ScalarFormatMap());
  switch (form_hash_) {
    case "sqxtn_asisdmisc_n"_h:
    case "sqxtun_asisdmisc_n"_h:
    case "uqxtn_asisdmisc_n"_h:
      nfd.SetFormatMap(1, nfd.LongScalarFormatMap());
  }
  Format(instr, mnemonic, nfd.SubstitutePlaceholders(form));
}

void Disassembler::VisitNEONScalar2RegMiscFP16(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Hd, 'Hn";
  const char *suffix = NULL;

  switch (form_hash_) {
    case "fcmeq_asisdmiscfp16_fz"_h:
    case "fcmge_asisdmiscfp16_fz"_h:
    case "fcmgt_asisdmiscfp16_fz"_h:
    case "fcmle_asisdmiscfp16_fz"_h:
    case "fcmlt_asisdmiscfp16_fz"_h:
      suffix = ", #0.0";
  }
  Format(instr, mnemonic, form, suffix);
}


void Disassembler::VisitNEONScalar3Diff(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, %sm";
  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::LongScalarFormatMap(),
                        NEONFormatDecoder::ScalarFormatMap());
  if (nfd.GetVectorFormat(0) == kFormatH) {
    mnemonic = NULL;
  }
  Format(instr, mnemonic, nfd.SubstitutePlaceholders(form));
}

void Disassembler::DisassembleNEONFPScalar3Same(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, %sm";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::FPScalarFormatMap());
  Format(instr, mnemonic, nfd.SubstitutePlaceholders(form));
}

void Disassembler::DisassembleNEONScalar3SameOnlyD(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Dd, 'Dn, 'Dm";
  if (instr->GetNEONSize() != 3) {
    mnemonic = NULL;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitNEONScalar3Same(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, %sm";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::ScalarFormatMap());
  VectorFormat vform = nfd.GetVectorFormat(0);
  switch (form_hash_) {
    case "srshl_asisdsame_only"_h:
    case "urshl_asisdsame_only"_h:
    case "sshl_asisdsame_only"_h:
    case "ushl_asisdsame_only"_h:
      if (vform != kFormatD) {
        mnemonic = NULL;
      }
      break;
    case "sqdmulh_asisdsame_only"_h:
    case "sqrdmulh_asisdsame_only"_h:
    case "sqrdmlah_asisdsame2_only"_h:
    case "sqrdmlsh_asisdsame2_only"_h:
      if ((vform == kFormatB) || (vform == kFormatD)) {
        mnemonic = NULL;
      }
  }
  Format(instr, mnemonic, nfd.SubstitutePlaceholders(form));
}

void Disassembler::VisitNEONScalar3SameFP16(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Hd, 'Hn, 'Hm");
}

void Disassembler::VisitNEONScalar3SameExtra(const Instruction *instr) {
  USE(instr);
  // Nothing to do - handled by VisitNEONScalar3Same.
  VIXL_UNREACHABLE();
}

void Disassembler::DisassembleNEONScalarSatMulLongIndex(
    const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, 'Vf.%s['IVByElemIndex]";
  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::LongScalarFormatMap(),
                        NEONFormatDecoder::ScalarFormatMap());
  if (nfd.GetVectorFormat(0) == kFormatH) {
    mnemonic = NULL;
  }
  Format(instr,
         mnemonic,
         nfd.Substitute(form, nfd.kPlaceholder, nfd.kPlaceholder, nfd.kFormat));
}

void Disassembler::DisassembleNEONFPScalarMulIndex(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, 'Vf.%s['IVByElemIndex]";
  static const NEONFormatMap map = {{23, 22}, {NF_H, NF_UNDEF, NF_S, NF_D}};
  NEONFormatDecoder nfd(instr, &map);
  Format(instr,
         mnemonic,
         nfd.Substitute(form, nfd.kPlaceholder, nfd.kPlaceholder, nfd.kFormat));
}

void Disassembler::VisitNEONScalarByIndexedElement(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, 'Vf.%s['IVByElemIndex]";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::ScalarFormatMap());
  VectorFormat vform_dst = nfd.GetVectorFormat(0);
  if ((vform_dst == kFormatB) || (vform_dst == kFormatD)) {
    mnemonic = NULL;
  }
  Format(instr,
         mnemonic,
         nfd.Substitute(form, nfd.kPlaceholder, nfd.kPlaceholder, nfd.kFormat));
}


void Disassembler::VisitNEONScalarCopy(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(NEONScalarCopy)";

  NEONFormatDecoder nfd(instr, NEONFormatDecoder::TriangularScalarFormatMap());

  if (instr->Mask(NEONScalarCopyMask) == NEON_DUP_ELEMENT_scalar) {
    mnemonic = "mov";
    form = "%sd, 'Vn.%s['IVInsIndex1]";
  }

  Format(instr, mnemonic, nfd.Substitute(form, nfd.kPlaceholder, nfd.kFormat));
}


void Disassembler::VisitNEONScalarPairwise(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  if (form_hash_ == "addp_asisdpair_only"_h) {
    // All pairwise operations except ADDP use bit U to differentiate FP16
    // from FP32/FP64 variations.
    if (instr->GetNEONSize() != 3) {
      mnemonic = NULL;
    }
    Format(instr, mnemonic, "'Dd, 'Vn.2d");
  } else {
    const char *form = "%sd, 'Vn.2%s";
    NEONFormatDecoder nfd(instr,
                          NEONFormatDecoder::FPScalarPairwiseFormatMap());

    Format(instr,
           mnemonic,
           nfd.Substitute(form,
                          NEONFormatDecoder::kPlaceholder,
                          NEONFormatDecoder::kFormat));
  }
}

void Disassembler::DisassembleNEONScalarShiftImmOnlyD(
    const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Dd, 'Dn, ";
  const char *suffix = "'IsR";

  if (instr->ExtractBit(22) == 0) {
    // Only D registers are supported.
    mnemonic = NULL;
  }

  switch (form_hash_) {
    case "shl_asisdshf_r"_h:
    case "sli_asisdshf_r"_h:
      suffix = "'IsL";
  }

  Format(instr, mnemonic, form, suffix);
}

void Disassembler::DisassembleNEONScalarShiftRightNarrowImm(
    const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, 'IsR";
  static const NEONFormatMap map_dst =
      {{22, 21, 20, 19}, {NF_UNDEF, NF_B, NF_H, NF_H, NF_S, NF_S, NF_S, NF_S}};
  static const NEONFormatMap map_src =
      {{22, 21, 20, 19}, {NF_UNDEF, NF_H, NF_S, NF_S, NF_D, NF_D, NF_D, NF_D}};
  NEONFormatDecoder nfd(instr, &map_dst, &map_src);
  Format(instr, mnemonic, nfd.SubstitutePlaceholders(form));
}

void Disassembler::VisitNEONScalarShiftImmediate(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "%sd, %sn, ";
  const char *suffix = "'IsR";

  // clang-format off
  static const NEONFormatMap map = {{22, 21, 20, 19},
                                    {NF_UNDEF, NF_B, NF_H, NF_H,
                                     NF_S,     NF_S, NF_S, NF_S,
                                     NF_D,     NF_D, NF_D, NF_D,
                                     NF_D,     NF_D, NF_D, NF_D}};
  // clang-format on
  NEONFormatDecoder nfd(instr, &map);
  switch (form_hash_) {
    case "sqshlu_asisdshf_r"_h:
    case "sqshl_asisdshf_r"_h:
    case "uqshl_asisdshf_r"_h:
      suffix = "'IsL";
      break;
    default:
      if (nfd.GetVectorFormat(0) == kFormatB) {
        mnemonic = NULL;
      }
  }
  Format(instr, mnemonic, nfd.SubstitutePlaceholders(form), suffix);
}

void Disassembler::DisassembleNEONShiftLeftLongImm(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s";
  const char *suffix = ", 'IsL";

  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::ShiftLongNarrowImmFormatMap(),
                        NEONFormatDecoder::ShiftImmFormatMap());

  if (instr->GetImmNEONImmb() == 0 &&
      CountSetBits(instr->GetImmNEONImmh(), 32) == 1) {  // xtl variant.
    VIXL_ASSERT((form_hash_ == "sshll_asimdshf_l"_h) ||
                (form_hash_ == "ushll_asimdshf_l"_h));
    mnemonic = (form_hash_ == "sshll_asimdshf_l"_h) ? "sxtl" : "uxtl";
    suffix = NULL;
  }
  Format(instr, nfd.Mnemonic(mnemonic), nfd.Substitute(form), suffix);
}

void Disassembler::DisassembleNEONShiftRightImm(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'IsR";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::ShiftImmFormatMap());

  VectorFormat vform_dst = nfd.GetVectorFormat(0);
  if (vform_dst != kFormatUndefined) {
    uint32_t ls_dst = LaneSizeInBitsFromFormat(vform_dst);
    switch (form_hash_) {
      case "scvtf_asimdshf_c"_h:
      case "ucvtf_asimdshf_c"_h:
      case "fcvtzs_asimdshf_c"_h:
      case "fcvtzu_asimdshf_c"_h:
        if (ls_dst == kBRegSize) {
          mnemonic = NULL;
        }
        break;
    }
  }
  Format(instr, mnemonic, nfd.Substitute(form));
}

void Disassembler::DisassembleNEONShiftRightNarrowImm(
    const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'IsR";

  NEONFormatDecoder nfd(instr,
                        NEONFormatDecoder::ShiftImmFormatMap(),
                        NEONFormatDecoder::ShiftLongNarrowImmFormatMap());
  Format(instr, nfd.Mnemonic(mnemonic), nfd.Substitute(form));
}

void Disassembler::VisitNEONShiftImmediate(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Vd.%s, 'Vn.%s, 'IsL";
  NEONFormatDecoder nfd(instr, NEONFormatDecoder::ShiftImmFormatMap());
  Format(instr, mnemonic, nfd.Substitute(form));
}


void Disassembler::VisitNEONTable(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char form_1v[] = "'Vd.%%s, {'Vn.16b}, 'Vm.%%s";
  const char form_2v[] = "'Vd.%%s, {'Vn.16b, v%d.16b}, 'Vm.%%s";
  const char form_3v[] = "'Vd.%%s, {'Vn.16b, v%d.16b, v%d.16b}, 'Vm.%%s";
  const char form_4v[] =
      "'Vd.%%s, {'Vn.16b, v%d.16b, v%d.16b, v%d.16b}, 'Vm.%%s";
  const char *form = form_1v;

  NEONFormatDecoder nfd(instr, NEONFormatDecoder::LogicalFormatMap());

  switch (form_hash_) {
    case "tbl_asimdtbl_l2_2"_h:
    case "tbx_asimdtbl_l2_2"_h:
      form = form_2v;
      break;
    case "tbl_asimdtbl_l3_3"_h:
    case "tbx_asimdtbl_l3_3"_h:
      form = form_3v;
      break;
    case "tbl_asimdtbl_l4_4"_h:
    case "tbx_asimdtbl_l4_4"_h:
      form = form_4v;
      break;
  }
  VIXL_ASSERT(form != NULL);

  char re_form[sizeof(form_4v) + 6];  // 3 * two-digit substitutions => 6
  int reg_num = instr->GetRn();
  snprintf(re_form,
           sizeof(re_form),
           form,
           (reg_num + 1) % kNumberOfVRegisters,
           (reg_num + 2) % kNumberOfVRegisters,
           (reg_num + 3) % kNumberOfVRegisters);

  Format(instr, mnemonic, nfd.Substitute(re_form));
}


void Disassembler::VisitNEONPerm(const Instruction *instr) {
  NEONFormatDecoder nfd(instr);
  FormatWithDecodedMnemonic(instr, nfd.Substitute("'Vd.%s, 'Vn.%s, 'Vm.%s"));
}

void Disassembler::Disassemble_Vd4S_Vn16B_Vm16B(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Vd.4s, 'Vn.16b, 'Vm.16b");
}

void Disassembler::
    VisitSVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsets(
        const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.s}, 'Pgl/z, ['Xns, 'Zm.s, '?22:suxtw #1]");
}

void Disassembler::VisitSVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.s}, 'Pgl/z, ['Xns, 'Zm.s, '?22:suxtw #2]");
}

void Disassembler::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.s}, 'Pgl/z, ['Xns, 'Zm.s, '?22:suxtw]");
}

void Disassembler::VisitSVE32BitGatherLoad_VectorPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.s}, 'Pgl/z, ['Zn.s]";
  const char *form_imm = "{'Zt.s}, 'Pgl/z, ['Zn.s, #'u2016]";
  const char *form_imm_h = "{'Zt.s}, 'Pgl/z, ['Zn.s, #'u2016*2]";
  const char *form_imm_w = "{'Zt.s}, 'Pgl/z, ['Zn.s, #'u2016*4]";

  const char *mnemonic = mnemonic_.c_str();
  switch (form_hash_) {
    case "ld1h_z_p_ai_s"_h:
    case "ld1sh_z_p_ai_s"_h:
    case "ldff1h_z_p_ai_s"_h:
    case "ldff1sh_z_p_ai_s"_h:
      form_imm = form_imm_h;
      break;
    case "ld1w_z_p_ai_s"_h:
    case "ldff1w_z_p_ai_s"_h:
      form_imm = form_imm_w;
      break;
  }
  if (instr->ExtractBits(20, 16) != 0) form = form_imm;

  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsets(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'prefSVEOp, 'Pgl, ['Xns, 'Zm.s, '?22:suxtw";
  const char *suffix = NULL;

  switch (
      instr->Mask(SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsetsMask)) {
    case PRFB_i_p_bz_s_x32_scaled:
      mnemonic = "prfb";
      suffix = "]";
      break;
    case PRFD_i_p_bz_s_x32_scaled:
      mnemonic = "prfd";
      suffix = " #3]";
      break;
    case PRFH_i_p_bz_s_x32_scaled:
      mnemonic = "prfh";
      suffix = " #1]";
      break;
    case PRFW_i_p_bz_s_x32_scaled:
      mnemonic = "prfw";
      suffix = " #2]";
      break;
    default:
      form = "(SVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsets)";
      break;
  }
  Format(instr, mnemonic, form, suffix);
}

void Disassembler::VisitSVE32BitGatherPrefetch_VectorPlusImm(
    const Instruction *instr) {
  const char *form = (instr->ExtractBits(20, 16) != 0)
                         ? "'prefSVEOp, 'Pgl, ['Zn.s, #'u2016]"
                         : "'prefSVEOp, 'Pgl, ['Zn.s]";
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVE32BitScatterStore_ScalarPlus32BitScaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.s}, 'Pgl, ['Xns, 'Zm.s, '?14:suxtw #'u2423]");
}

void Disassembler::VisitSVE32BitScatterStore_ScalarPlus32BitUnscaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "{'Zt.s}, 'Pgl, ['Xns, 'Zm.s, '?14:suxtw]");
}

void Disassembler::VisitSVE32BitScatterStore_VectorPlusImm(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "{'Zt.s}, 'Pgl, ['Zn.s";
  const char *suffix = NULL;

  bool is_zero = instr->ExtractBits(20, 16) == 0;

  switch (instr->Mask(SVE32BitScatterStore_VectorPlusImmMask)) {
    case ST1B_z_p_ai_s:
      mnemonic = "st1b";
      suffix = is_zero ? "]" : ", #'u2016]";
      break;
    case ST1H_z_p_ai_s:
      mnemonic = "st1h";
      suffix = is_zero ? "]" : ", #'u2016*2]";
      break;
    case ST1W_z_p_ai_s:
      mnemonic = "st1w";
      suffix = is_zero ? "]" : ", #'u2016*4]";
      break;
    default:
      form = "(SVE32BitScatterStore_VectorPlusImm)";
      break;
  }
  Format(instr, mnemonic, form, suffix);
}

void Disassembler::VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.d}, 'Pgl/z, ['Xns, 'Zm.d, '?22:suxtw "
                            "#'u2423]");
}

void Disassembler::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.d}, 'Pgl/z, ['Xns, 'Zm.d, lsl #'u2423]");
}

void Disassembler::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "{'Zt.d}, 'Pgl/z, ['Xns, 'Zm.d]");
}

void Disassembler::
    VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets(
        const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.d}, 'Pgl/z, ['Xns, 'Zm.d, '?22:suxtw]");
}

void Disassembler::VisitSVE64BitGatherLoad_VectorPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.d}, 'Pgl/z, ['Zn.d]";
  const char *form_imm[4] = {"{'Zt.d}, 'Pgl/z, ['Zn.d, #'u2016]",
                             "{'Zt.d}, 'Pgl/z, ['Zn.d, #'u2016*2]",
                             "{'Zt.d}, 'Pgl/z, ['Zn.d, #'u2016*4]",
                             "{'Zt.d}, 'Pgl/z, ['Zn.d, #'u2016*8]"};

  if (instr->ExtractBits(20, 16) != 0) {
    unsigned msz = instr->ExtractBits(24, 23);
    bool sign_extend = instr->ExtractBit(14) == 0;
    if ((msz == kDRegSizeInBytesLog2) && sign_extend) {
      form = "(SVE64BitGatherLoad_VectorPlusImm)";
    } else {
      VIXL_ASSERT(msz < ArrayLength(form_imm));
      form = form_imm[msz];
    }
  }

  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsets(
    const Instruction *instr) {
  const char *form = "'prefSVEOp, 'Pgl, ['Xns, 'Zm.d";
  const char *suffix = "]";

  switch (form_hash_) {
    case "prfh_i_p_bz_d_64_scaled"_h:
      suffix = ", lsl #1]";
      break;
    case "prfs_i_p_bz_d_64_scaled"_h:
      suffix = ", lsl #2]";
      break;
    case "prfd_i_p_bz_d_64_scaled"_h:
      suffix = ", lsl #3]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::
    VisitSVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsets(
        const Instruction *instr) {
  const char *form = "'prefSVEOp, 'Pgl, ['Xns, 'Zm.d, '?22:suxtw ";
  const char *suffix = "]";

  switch (form_hash_) {
    case "prfh_i_p_bz_d_x32_scaled"_h:
      suffix = "#1]";
      break;
    case "prfs_i_p_bz_d_x32_scaled"_h:
      suffix = "#2]";
      break;
    case "prfd_i_p_bz_d_x32_scaled"_h:
      suffix = "#3]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVE64BitGatherPrefetch_VectorPlusImm(
    const Instruction *instr) {
  const char *form = (instr->ExtractBits(20, 16) != 0)
                         ? "'prefSVEOp, 'Pgl, ['Zn.d, #'u2016]"
                         : "'prefSVEOp, 'Pgl, ['Zn.d]";

  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVE64BitScatterStore_ScalarPlus64BitScaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "{'Zt.d}, 'Pgl, ['Xns, 'Zm.d, lsl #'u2423]");
}

void Disassembler::VisitSVE64BitScatterStore_ScalarPlus64BitUnscaledOffsets(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "{'Zt.d}, 'Pgl, ['Xns, 'Zm.d]");
}

void Disassembler::
    VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsets(
        const Instruction *instr) {
  FormatWithDecodedMnemonic(instr,
                            "{'Zt.d}, 'Pgl, ['Xns, 'Zm.d, '?14:suxtw #'u2423]");
}

void Disassembler::
    VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsets(
        const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "{'Zt.d}, 'Pgl, ['Xns, 'Zm.d, '?14:suxtw]");
}

void Disassembler::VisitSVE64BitScatterStore_VectorPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.d}, 'Pgl, ['Zn.d";
  const char *suffix = "]";

  if (instr->ExtractBits(20, 16) != 0) {
    switch (form_hash_) {
      case "st1b_z_p_ai_d"_h:
        suffix = ", #'u2016]";
        break;
      case "st1h_z_p_ai_d"_h:
        suffix = ", #'u2016*2]";
        break;
      case "st1w_z_p_ai_d"_h:
        suffix = ", #'u2016*4]";
        break;
      case "st1d_z_p_ai_d"_h:
        suffix = ", #'u2016*8]";
        break;
    }
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEBitwiseLogicalWithImm_Unpredicated(
    const Instruction *instr) {
  if (instr->GetSVEImmLogical() == 0) {
    // The immediate encoded in the instruction is not in the expected format.
    Format(instr, "unallocated", "(SVEBitwiseImm)");
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'tl, 'Zd.'tl, 'ITriSvel");
  }
}

void Disassembler::VisitSVEBitwiseLogical_Predicated(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVEBitwiseShiftByImm_Predicated(
    const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Zd.'tszp, 'Pgl/m, 'Zd.'tszp, ";
  const char *suffix = NULL;
  unsigned tsize = (instr->ExtractBits(23, 22) << 2) | instr->ExtractBits(9, 8);

  if (tsize == 0) {
    mnemonic = "unimplemented";
    form = "(SVEBitwiseShiftByImm_Predicated)";
  } else {
    switch (form_hash_) {
      case "lsl_z_p_zi"_h:
      case "sqshl_z_p_zi"_h:
      case "sqshlu_z_p_zi"_h:
      case "uqshl_z_p_zi"_h:
        suffix = "'ITriSvep";
        break;
      case "asrd_z_p_zi"_h:
      case "asr_z_p_zi"_h:
      case "lsr_z_p_zi"_h:
      case "srshr_z_p_zi"_h:
      case "urshr_z_p_zi"_h:
        suffix = "'ITriSveq";
        break;
      default:
        mnemonic = "unimplemented";
        form = "(SVEBitwiseShiftByImm_Predicated)";
        break;
    }
  }
  Format(instr, mnemonic, form, suffix);
}

void Disassembler::VisitSVEBitwiseShiftByVector_Predicated(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVEBitwiseShiftByWideElements_Predicated(
    const Instruction *instr) {
  if (instr->GetSVESize() == kDRegSizeInBytesLog2) {
    Format(instr, "unallocated", "(SVEBitwiseShiftByWideElements_Predicated)");
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.d");
  }
}

static bool SVEMoveMaskPreferred(uint64_t value, int lane_bytes_log2) {
  VIXL_ASSERT(IsUintN(8 << lane_bytes_log2, value));

  // Duplicate lane-sized value across double word.
  switch (lane_bytes_log2) {
    case 0:
      value *= 0x0101010101010101;
      break;
    case 1:
      value *= 0x0001000100010001;
      break;
    case 2:
      value *= 0x0000000100000001;
      break;
    case 3:  // Nothing to do
      break;
    default:
      VIXL_UNREACHABLE();
  }

  if ((value & 0xff) == 0) {
    // Check for 16-bit patterns. Set least-significant 16 bits, to make tests
    // easier; we already checked least-significant byte is zero above.
    uint64_t generic_value = value | 0xffff;

    // Check 0x00000000_0000pq00 or 0xffffffff_ffffpq00.
    if ((generic_value == 0xffff) || (generic_value == UINT64_MAX)) {
      return false;
    }

    // Check 0x0000pq00_0000pq00 or 0xffffpq00_ffffpq00.
    if (AllWordsMatch(value)) {
      generic_value &= 0xffffffff;
      if ((generic_value == 0xffff) || (generic_value == UINT32_MAX)) {
        return false;
      }
    }

    // Check 0xpq00pq00_pq00pq00.
    if (AllHalfwordsMatch(value)) {
      return false;
    }
  } else {
    // Check for 8-bit patterns. Set least-significant byte, to make tests
    // easier.
    uint64_t generic_value = value | 0xff;

    // Check 0x00000000_000000pq or 0xffffffff_ffffffpq.
    if ((generic_value == 0xff) || (generic_value == UINT64_MAX)) {
      return false;
    }

    // Check 0x000000pq_000000pq or 0xffffffpq_ffffffpq.
    if (AllWordsMatch(value)) {
      generic_value &= 0xffffffff;
      if ((generic_value == 0xff) || (generic_value == UINT32_MAX)) {
        return false;
      }
    }

    // Check 0x00pq00pq_00pq00pq or 0xffpqffpq_ffpqffpq.
    if (AllHalfwordsMatch(value)) {
      generic_value &= 0xffff;
      if ((generic_value == 0xff) || (generic_value == UINT16_MAX)) {
        return false;
      }
    }

    // Check 0xpqpqpqpq_pqpqpqpq.
    if (AllBytesMatch(value)) {
      return false;
    }
  }
  return true;
}

void Disassembler::VisitSVEBroadcastBitmaskImm(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEBroadcastBitmaskImm)";

  switch (instr->Mask(SVEBroadcastBitmaskImmMask)) {
    case DUPM_z_i: {
      uint64_t imm = instr->GetSVEImmLogical();
      if (imm != 0) {
        int lane_size = instr->GetSVEBitwiseImmLaneSizeInBytesLog2();
        mnemonic = SVEMoveMaskPreferred(imm, lane_size) ? "mov" : "dupm";
        form = "'Zd.'tl, 'ITriSvel";
      }
      break;
    }
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEBroadcastFPImm_Unpredicated(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEBroadcastFPImm_Unpredicated)";

  if (instr->GetSVEVectorFormat() != kFormatVnB) {
    switch (instr->Mask(SVEBroadcastFPImm_UnpredicatedMask)) {
      case FDUP_z_i:
        // The preferred disassembly for fdup is "fmov".
        mnemonic = "fmov";
        form = "'Zd.'t, 'IFPSve";
        break;
      default:
        break;
    }
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEBroadcastGeneralRegister(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEBroadcastGeneralRegister)";

  switch (instr->Mask(SVEBroadcastGeneralRegisterMask)) {
    case DUP_z_r:
      // The preferred disassembly for dup is "mov".
      mnemonic = "mov";
      if (instr->GetSVESize() == kDRegSizeInBytesLog2) {
        form = "'Zd.'t, 'Xns";
      } else {
        form = "'Zd.'t, 'Wns";
      }
      break;
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEBroadcastIndexElement(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEBroadcastIndexElement)";

  switch (instr->Mask(SVEBroadcastIndexElementMask)) {
    case DUP_z_zi: {
      // The tsz field must not be zero.
      int tsz = instr->ExtractBits(20, 16);
      if (tsz != 0) {
        // The preferred disassembly for dup is "mov".
        mnemonic = "mov";
        int imm2 = instr->ExtractBits(23, 22);
        if ((CountSetBits(imm2) + CountSetBits(tsz)) == 1) {
          // If imm2:tsz has one set bit, the index is zero. This is
          // disassembled as a mov from a b/h/s/d/q scalar register.
          form = "'Zd.'ti, 'ti'u0905";
        } else {
          form = "'Zd.'ti, 'Zn.'ti['IVInsSVEIndex]";
        }
      }
      break;
    }
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEBroadcastIntImm_Unpredicated(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEBroadcastIntImm_Unpredicated)";

  switch (instr->Mask(SVEBroadcastIntImm_UnpredicatedMask)) {
    case DUP_z_i:
      // The encoding of byte-sized lanes with lsl #8 is undefined.
      if ((instr->GetSVEVectorFormat() == kFormatVnB) &&
          (instr->ExtractBit(13) == 1))
        break;

      // The preferred disassembly for dup is "mov".
      mnemonic = "mov";
      form = (instr->ExtractBit(13) == 0) ? "'Zd.'t, #'s1205"
                                          : "'Zd.'t, #'s1205, lsl #8";
      break;
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVECompressActiveElements(const Instruction *instr) {
  // The top bit of size is always set for compact, so 't can only be
  // substituted with types S and D.
  if (instr->ExtractBit(23) == 1) {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl, 'Zn.'t");
  } else {
    VisitUnallocated(instr);
  }
}

void Disassembler::VisitSVEConditionallyBroadcastElementToVector(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl, 'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVEConditionallyExtractElementToGeneralRegister(
    const Instruction *instr) {
  const char *form = "'Wd, 'Pgl, 'Wd, 'Zn.'t";

  if (instr->GetSVESize() == kDRegSizeInBytesLog2) {
    form = "'Xd, p'u1210, 'Xd, 'Zn.'t";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEConditionallyExtractElementToSIMDFPScalar(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'t'u0400, 'Pgl, 't'u0400, 'Zn.'t");
}

void Disassembler::VisitSVEConditionallyTerminateScalars(
    const Instruction *instr) {
  const char *form = (instr->ExtractBit(22) == 0) ? "'Wn, 'Wm" : "'Xn, 'Xm";
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEConstructivePrefix_Unpredicated(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd, 'Zn");
}

void Disassembler::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar(
    const Instruction *instr) {
  const char *form = "{'Zt.'tlss}, 'Pgl/z, ['Xns";
  const char *suffix = "]";

  if (instr->GetRm() != kZeroRegCode) {
    switch (form_hash_) {
      case "ldff1b_z_p_br_u8"_h:
      case "ldff1b_z_p_br_u16"_h:
      case "ldff1b_z_p_br_u32"_h:
      case "ldff1b_z_p_br_u64"_h:
      case "ldff1sb_z_p_br_s16"_h:
      case "ldff1sb_z_p_br_s32"_h:
      case "ldff1sb_z_p_br_s64"_h:
        suffix = ", 'Xm]";
        break;
      case "ldff1h_z_p_br_u16"_h:
      case "ldff1h_z_p_br_u32"_h:
      case "ldff1h_z_p_br_u64"_h:
      case "ldff1sh_z_p_br_s32"_h:
      case "ldff1sh_z_p_br_s64"_h:
        suffix = ", 'Xm, lsl #1]";
        break;
      case "ldff1w_z_p_br_u32"_h:
      case "ldff1w_z_p_br_u64"_h:
      case "ldff1sw_z_p_br_s64"_h:
        suffix = ", 'Xm, lsl #2]";
        break;
      case "ldff1d_z_p_br_u64"_h:
        suffix = ", 'Xm, lsl #3]";
        break;
    }
  }

  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEContiguousNonFaultLoad_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.'tlss}, 'Pgl/z, ['Xns";
  const char *suffix =
      (instr->ExtractBits(19, 16) == 0) ? "]" : ", #'s1916, mul vl]";
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEContiguousNonTemporalLoad_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.b}, 'Pgl/z, ['Xns";
  const char *suffix =
      (instr->ExtractBits(19, 16) == 0) ? "]" : ", #'s1916, mul vl]";
  switch (form_hash_) {
    case "ldnt1d_z_p_bi_contiguous"_h:
      form = "{'Zt.d}, 'Pgl/z, ['Xns";
      break;
    case "ldnt1h_z_p_bi_contiguous"_h:
      form = "{'Zt.h}, 'Pgl/z, ['Xns";
      break;
    case "ldnt1w_z_p_bi_contiguous"_h:
      form = "{'Zt.s}, 'Pgl/z, ['Xns";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEContiguousNonTemporalLoad_ScalarPlusScalar(
    const Instruction *instr) {
  const char *form = "{'Zt.b}, 'Pgl/z, ['Xns, 'Rm]";
  switch (form_hash_) {
    case "ldnt1d_z_p_br_contiguous"_h:
      form = "{'Zt.d}, 'Pgl/z, ['Xns, 'Rm, lsl #3]";
      break;
    case "ldnt1h_z_p_br_contiguous"_h:
      form = "{'Zt.h}, 'Pgl/z, ['Xns, 'Rm, lsl #1]";
      break;
    case "ldnt1w_z_p_br_contiguous"_h:
      form = "{'Zt.s}, 'Pgl/z, ['Xns, 'Rm, lsl #2]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEContiguousNonTemporalStore_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.b}, 'Pgl, ['Xns";
  const char *suffix =
      (instr->ExtractBits(19, 16) == 0) ? "]" : ", #'s1916, mul vl]";

  switch (form_hash_) {
    case "stnt1d_z_p_bi_contiguous"_h:
      form = "{'Zt.d}, 'Pgl, ['Xns";
      break;
    case "stnt1h_z_p_bi_contiguous"_h:
      form = "{'Zt.h}, 'Pgl, ['Xns";
      break;
    case "stnt1w_z_p_bi_contiguous"_h:
      form = "{'Zt.s}, 'Pgl, ['Xns";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEContiguousNonTemporalStore_ScalarPlusScalar(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEContiguousNonTemporalStore_ScalarPlusScalar)";

  switch (instr->Mask(SVEContiguousNonTemporalStore_ScalarPlusScalarMask)) {
    case STNT1B_z_p_br_contiguous:
      mnemonic = "stnt1b";
      form = "{'Zt.b}, 'Pgl, ['Xns, 'Rm]";
      break;
    case STNT1D_z_p_br_contiguous:
      mnemonic = "stnt1d";
      form = "{'Zt.d}, 'Pgl, ['Xns, 'Rm, lsl #3]";
      break;
    case STNT1H_z_p_br_contiguous:
      mnemonic = "stnt1h";
      form = "{'Zt.h}, 'Pgl, ['Xns, 'Rm, lsl #1]";
      break;
    case STNT1W_z_p_br_contiguous:
      mnemonic = "stnt1w";
      form = "{'Zt.s}, 'Pgl, ['Xns, 'Rm, lsl #2]";
      break;
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEContiguousPrefetch_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = (instr->ExtractBits(21, 16) != 0)
                         ? "'prefSVEOp, 'Pgl, ['Xns, #'s2116, mul vl]"
                         : "'prefSVEOp, 'Pgl, ['Xns]";
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEContiguousPrefetch_ScalarPlusScalar(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEContiguousPrefetch_ScalarPlusScalar)";

  if (instr->GetRm() != kZeroRegCode) {
    switch (instr->Mask(SVEContiguousPrefetch_ScalarPlusScalarMask)) {
      case PRFB_i_p_br_s:
        mnemonic = "prfb";
        form = "'prefSVEOp, 'Pgl, ['Xns, 'Rm]";
        break;
      case PRFD_i_p_br_s:
        mnemonic = "prfd";
        form = "'prefSVEOp, 'Pgl, ['Xns, 'Rm, lsl #3]";
        break;
      case PRFH_i_p_br_s:
        mnemonic = "prfh";
        form = "'prefSVEOp, 'Pgl, ['Xns, 'Rm, lsl #1]";
        break;
      case PRFW_i_p_br_s:
        mnemonic = "prfw";
        form = "'prefSVEOp, 'Pgl, ['Xns, 'Rm, lsl #2]";
        break;
      default:
        break;
    }
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEContiguousStore_ScalarPlusImm(
    const Instruction *instr) {
  // The 'size' field isn't in the usual place here.
  const char *form = "{'Zt.'tls}, 'Pgl, ['Xns, #'s1916, mul vl]";
  if (instr->ExtractBits(19, 16) == 0) {
    form = "{'Zt.'tls}, 'Pgl, ['Xns]";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEContiguousStore_ScalarPlusScalar(
    const Instruction *instr) {
  // The 'size' field isn't in the usual place here.
  FormatWithDecodedMnemonic(instr, "{'Zt.'tls}, 'Pgl, ['Xns, 'Xm'NSveS]");
}

void Disassembler::VisitSVECopyFPImm_Predicated(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVECopyFPImm_Predicated)";

  if (instr->GetSVEVectorFormat() != kFormatVnB) {
    switch (instr->Mask(SVECopyFPImm_PredicatedMask)) {
      case FCPY_z_p_i:
        // The preferred disassembly for fcpy is "fmov".
        mnemonic = "fmov";
        form = "'Zd.'t, 'Pm/m, 'IFPSve";
        break;
      default:
        break;
    }
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVECopyGeneralRegisterToVector_Predicated(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVECopyGeneralRegisterToVector_Predicated)";

  switch (instr->Mask(SVECopyGeneralRegisterToVector_PredicatedMask)) {
    case CPY_z_p_r:
      // The preferred disassembly for cpy is "mov".
      mnemonic = "mov";
      form = "'Zd.'t, 'Pgl/m, 'Wns";
      if (instr->GetSVESize() == kXRegSizeInBytesLog2) {
        form = "'Zd.'t, 'Pgl/m, 'Xns";
      }
      break;
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVECopyIntImm_Predicated(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVECopyIntImm_Predicated)";
  const char *suffix = NULL;

  switch (instr->Mask(SVECopyIntImm_PredicatedMask)) {
    case CPY_z_p_i: {
      // The preferred disassembly for cpy is "mov".
      mnemonic = "mov";
      form = "'Zd.'t, 'Pm/'?14:mz, #'s1205";
      if (instr->ExtractBit(13) != 0) suffix = ", lsl #8";
      break;
    }
    default:
      break;
  }
  Format(instr, mnemonic, form, suffix);
}

void Disassembler::VisitSVECopySIMDFPScalarRegisterToVector_Predicated(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVECopySIMDFPScalarRegisterToVector_Predicated)";

  switch (instr->Mask(SVECopySIMDFPScalarRegisterToVector_PredicatedMask)) {
    case CPY_z_p_v:
      // The preferred disassembly for cpy is "mov".
      mnemonic = "mov";
      form = "'Zd.'t, 'Pgl/m, 'Vnv";
      break;
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEExtractElementToGeneralRegister(
    const Instruction *instr) {
  const char *form = "'Wd, 'Pgl, 'Zn.'t";
  if (instr->GetSVESize() == kDRegSizeInBytesLog2) {
    form = "'Xd, p'u1210, 'Zn.'t";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEExtractElementToSIMDFPScalarRegister(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'t'u0400, 'Pgl, 'Zn.'t");
}

void Disassembler::VisitSVEFFRInitialise(const Instruction *instr) {
  DisassembleNoArgs(instr);
}

void Disassembler::VisitSVEFFRWriteFromPredicate(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pn.b");
}

void Disassembler::VisitSVEFPArithmeticWithImm_Predicated(
    const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/m, 'Zd.'t, #";
  const char *suffix00 = "0.0";
  const char *suffix05 = "0.5";
  const char *suffix10 = "1.0";
  const char *suffix20 = "2.0";
  int i1 = instr->ExtractBit(5);
  const char *suffix = i1 ? suffix10 : suffix00;

  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
    return;
  }

  switch (form_hash_) {
    case "fadd_z_p_zs"_h:
    case "fsubr_z_p_zs"_h:
    case "fsub_z_p_zs"_h:
      suffix = i1 ? suffix10 : suffix05;
      break;
    case "fmul_z_p_zs"_h:
      suffix = i1 ? suffix20 : suffix05;
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEFPArithmetic_Predicated(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t");
  }
}

void Disassembler::VisitSVEFPConvertPrecision(const Instruction *instr) {
  const char *form = NULL;

  switch (form_hash_) {
    case "fcvt_z_p_z_d2h"_h:
      form = "'Zd.h, 'Pgl/m, 'Zn.d";
      break;
    case "fcvt_z_p_z_d2s"_h:
      form = "'Zd.s, 'Pgl/m, 'Zn.d";
      break;
    case "fcvt_z_p_z_h2d"_h:
      form = "'Zd.d, 'Pgl/m, 'Zn.h";
      break;
    case "fcvt_z_p_z_h2s"_h:
      form = "'Zd.s, 'Pgl/m, 'Zn.h";
      break;
    case "fcvt_z_p_z_s2d"_h:
      form = "'Zd.d, 'Pgl/m, 'Zn.s";
      break;
    case "fcvt_z_p_z_s2h"_h:
      form = "'Zd.h, 'Pgl/m, 'Zn.s";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEFPConvertToInt(const Instruction *instr) {
  const char *form = NULL;

  switch (form_hash_) {
    case "fcvtzs_z_p_z_d2w"_h:
    case "fcvtzu_z_p_z_d2w"_h:
      form = "'Zd.s, 'Pgl/m, 'Zn.d";
      break;
    case "fcvtzs_z_p_z_d2x"_h:
    case "fcvtzu_z_p_z_d2x"_h:
      form = "'Zd.d, 'Pgl/m, 'Zn.d";
      break;
    case "fcvtzs_z_p_z_fp162h"_h:
    case "fcvtzu_z_p_z_fp162h"_h:
      form = "'Zd.h, 'Pgl/m, 'Zn.h";
      break;
    case "fcvtzs_z_p_z_fp162w"_h:
    case "fcvtzu_z_p_z_fp162w"_h:
      form = "'Zd.s, 'Pgl/m, 'Zn.h";
      break;
    case "fcvtzs_z_p_z_fp162x"_h:
    case "fcvtzu_z_p_z_fp162x"_h:
      form = "'Zd.d, 'Pgl/m, 'Zn.h";
      break;
    case "fcvtzs_z_p_z_s2w"_h:
    case "fcvtzu_z_p_z_s2w"_h:
      form = "'Zd.s, 'Pgl/m, 'Zn.s";
      break;
    case "fcvtzs_z_p_z_s2x"_h:
    case "fcvtzu_z_p_z_s2x"_h:
      form = "'Zd.d, 'Pgl/m, 'Zn.s";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEFPExponentialAccelerator(const Instruction *instr) {
  unsigned size = instr->GetSVESize();
  if ((size == kHRegSizeInBytesLog2) || (size == kSRegSizeInBytesLog2) ||
      (size == kDRegSizeInBytesLog2)) {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'t");
  } else {
    VisitUnallocated(instr);
  }
}

void Disassembler::VisitSVEFPRoundToIntegralValue(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zn.'t");
  }
}

void Disassembler::VisitSVEFPTrigMulAddCoefficient(const Instruction *instr) {
  unsigned size = instr->GetSVESize();
  if ((size == kHRegSizeInBytesLog2) || (size == kSRegSizeInBytesLog2) ||
      (size == kDRegSizeInBytesLog2)) {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zd.'t, 'Zn.'t, #'u1816");
  } else {
    VisitUnallocated(instr);
  }
}

void Disassembler::VisitSVEFPTrigSelectCoefficient(const Instruction *instr) {
  unsigned size = instr->GetSVESize();
  if ((size == kHRegSizeInBytesLog2) || (size == kSRegSizeInBytesLog2) ||
      (size == kDRegSizeInBytesLog2)) {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'t, 'Zm.'t");
  } else {
    VisitUnallocated(instr);
  }
}

void Disassembler::VisitSVEFPUnaryOp(const Instruction *instr) {
  if (instr->GetSVESize() == kBRegSizeInBytesLog2) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zn.'t");
  }
}

static const char *IncDecFormHelper(const Instruction *instr,
                                    const char *reg_pat_mul_form,
                                    const char *reg_pat_form,
                                    const char *reg_form) {
  if (instr->ExtractBits(19, 16) == 0) {
    if (instr->ExtractBits(9, 5) == SVE_ALL) {
      // Use the register only form if the multiplier is one (encoded as zero)
      // and the pattern is SVE_ALL.
      return reg_form;
    }
    // Use the register and pattern form if the multiplier is one.
    return reg_pat_form;
  }
  return reg_pat_mul_form;
}

void Disassembler::VisitSVEIncDecRegisterByElementCount(
    const Instruction *instr) {
  const char *form =
      IncDecFormHelper(instr, "'Xd, 'Ipc, mul #'u1916+1", "'Xd, 'Ipc", "'Xd");
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEIncDecVectorByElementCount(
    const Instruction *instr) {
  const char *form = IncDecFormHelper(instr,
                                      "'Zd.'t, 'Ipc, mul #'u1916+1",
                                      "'Zd.'t, 'Ipc",
                                      "'Zd.'t");
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEInsertGeneralRegister(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Wn";
  if (instr->GetSVESize() == kDRegSizeInBytesLog2) {
    form = "'Zd.'t, 'Xn";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEInsertSIMDFPScalarRegister(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Vnv");
}

void Disassembler::VisitSVEIntAddSubtractImm_Unpredicated(
    const Instruction *instr) {
  const char *form = (instr->ExtractBit(13) == 0)
                         ? "'Zd.'t, 'Zd.'t, #'u1205"
                         : "'Zd.'t, 'Zd.'t, #'u1205, lsl #8";
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEIntAddSubtractVectors_Predicated(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVEIntCompareScalarCountAndLimit(
    const Instruction *instr) {
  const char *form =
      (instr->ExtractBit(12) == 0) ? "'Pd.'t, 'Wn, 'Wm" : "'Pd.'t, 'Xn, 'Xm";
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEIntConvertToFP(const Instruction *instr) {
  const char *form = NULL;
  switch (form_hash_) {
    case "scvtf_z_p_z_h2fp16"_h:
    case "ucvtf_z_p_z_h2fp16"_h:
      form = "'Zd.h, 'Pgl/m, 'Zn.h";
      break;
    case "scvtf_z_p_z_w2d"_h:
    case "ucvtf_z_p_z_w2d"_h:
      form = "'Zd.d, 'Pgl/m, 'Zn.s";
      break;
    case "scvtf_z_p_z_w2fp16"_h:
    case "ucvtf_z_p_z_w2fp16"_h:
      form = "'Zd.h, 'Pgl/m, 'Zn.s";
      break;
    case "scvtf_z_p_z_w2s"_h:
    case "ucvtf_z_p_z_w2s"_h:
      form = "'Zd.s, 'Pgl/m, 'Zn.s";
      break;
    case "scvtf_z_p_z_x2d"_h:
    case "ucvtf_z_p_z_x2d"_h:
      form = "'Zd.d, 'Pgl/m, 'Zn.d";
      break;
    case "scvtf_z_p_z_x2fp16"_h:
    case "ucvtf_z_p_z_x2fp16"_h:
      form = "'Zd.h, 'Pgl/m, 'Zn.d";
      break;
    case "scvtf_z_p_z_x2s"_h:
    case "ucvtf_z_p_z_x2s"_h:
      form = "'Zd.s, 'Pgl/m, 'Zn.d";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEIntDivideVectors_Predicated(
    const Instruction *instr) {
  unsigned size = instr->GetSVESize();
  if ((size == kSRegSizeInBytesLog2) || (size == kDRegSizeInBytesLog2)) {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t");
  } else {
    VisitUnallocated(instr);
  }
}

void Disassembler::VisitSVEIntMinMaxDifference_Predicated(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVEIntMinMaxImm_Unpredicated(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zd.'t, #";
  const char *suffix = "'u1205";

  switch (form_hash_) {
    case "smax_z_zi"_h:
    case "smin_z_zi"_h:
      suffix = "'s1205";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEIntMulImm_Unpredicated(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zd.'t, #'s1205");
}

void Disassembler::VisitSVEIntMulVectors_Predicated(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVELoadAndBroadcastElement(const Instruction *instr) {
  const char *form = "(SVELoadAndBroadcastElement)";
  const char *suffix_b = ", #'u2116]";
  const char *suffix_h = ", #'u2116*2]";
  const char *suffix_w = ", #'u2116*4]";
  const char *suffix_d = ", #'u2116*8]";
  const char *suffix = NULL;

  switch (form_hash_) {
    case "ld1rb_z_p_bi_u8"_h:
      form = "{'Zt.b}, 'Pgl/z, ['Xns";
      suffix = suffix_b;
      break;
    case "ld1rb_z_p_bi_u16"_h:
    case "ld1rsb_z_p_bi_s16"_h:
      form = "{'Zt.h}, 'Pgl/z, ['Xns";
      suffix = suffix_b;
      break;
    case "ld1rb_z_p_bi_u32"_h:
    case "ld1rsb_z_p_bi_s32"_h:
      form = "{'Zt.s}, 'Pgl/z, ['Xns";
      suffix = suffix_b;
      break;
    case "ld1rb_z_p_bi_u64"_h:
    case "ld1rsb_z_p_bi_s64"_h:
      form = "{'Zt.d}, 'Pgl/z, ['Xns";
      suffix = suffix_b;
      break;
    case "ld1rh_z_p_bi_u16"_h:
      form = "{'Zt.h}, 'Pgl/z, ['Xns";
      suffix = suffix_h;
      break;
    case "ld1rh_z_p_bi_u32"_h:
    case "ld1rsh_z_p_bi_s32"_h:
      form = "{'Zt.s}, 'Pgl/z, ['Xns";
      suffix = suffix_h;
      break;
    case "ld1rh_z_p_bi_u64"_h:
    case "ld1rsh_z_p_bi_s64"_h:
      form = "{'Zt.d}, 'Pgl/z, ['Xns";
      suffix = suffix_h;
      break;
    case "ld1rw_z_p_bi_u32"_h:
      form = "{'Zt.s}, 'Pgl/z, ['Xns";
      suffix = suffix_w;
      break;
    case "ld1rsw_z_p_bi_s64"_h:
    case "ld1rw_z_p_bi_u64"_h:
      form = "{'Zt.d}, 'Pgl/z, ['Xns";
      suffix = suffix_w;
      break;
    case "ld1rd_z_p_bi_u64"_h:
      form = "{'Zt.d}, 'Pgl/z, ['Xns";
      suffix = suffix_d;
      break;
  }

  // Hide curly brackets if immediate is zero.
  if (instr->ExtractBits(21, 16) == 0) {
    suffix = "]";
  }

  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.'tmsz}, 'Pgl/z, ['Xns";
  const char *suffix = ", #'s1916*16]";

  switch (form_hash_) {
    case "ld1rob_z_p_bi_u8"_h:
    case "ld1rod_z_p_bi_u64"_h:
    case "ld1roh_z_p_bi_u16"_h:
    case "ld1row_z_p_bi_u32"_h:
      suffix = ", #'s1916*32]";
      break;
  }
  if (instr->ExtractBits(19, 16) == 0) suffix = "]";

  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar(
    const Instruction *instr) {
  const char *form = "{'Zt.'tmsz}, 'Pgl/z, ['Xns, ";
  const char *suffix = "'Rm, lsl #'u2423]";

  switch (form_hash_) {
    case "ld1rqb_z_p_br_contiguous"_h:
    case "ld1rob_z_p_br_contiguous"_h:
      suffix = "'Rm]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVELoadMultipleStructures_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.'tmsz, 'Zt2.'tmsz}";
  const char *form_3 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz}";
  const char *form_4 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz, 'Zt4.'tmsz}";
  const char *suffix = ", 'Pgl/z, ['Xns'ISveSvl]";

  switch (form_hash_) {
    case "ld3b_z_p_bi_contiguous"_h:
    case "ld3d_z_p_bi_contiguous"_h:
    case "ld3h_z_p_bi_contiguous"_h:
    case "ld3w_z_p_bi_contiguous"_h:
      form = form_3;
      break;
    case "ld4b_z_p_bi_contiguous"_h:
    case "ld4d_z_p_bi_contiguous"_h:
    case "ld4h_z_p_bi_contiguous"_h:
    case "ld4w_z_p_bi_contiguous"_h:
      form = form_4;
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVELoadMultipleStructures_ScalarPlusScalar(
    const Instruction *instr) {
  const char *form = "{'Zt.'tmsz, 'Zt2.'tmsz}";
  const char *form_3 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz}";
  const char *form_4 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz, 'Zt4.'tmsz}";
  const char *suffix = ", 'Pgl/z, ['Xns, 'Xm'NSveS]";

  switch (form_hash_) {
    case "ld3b_z_p_br_contiguous"_h:
    case "ld3d_z_p_br_contiguous"_h:
    case "ld3h_z_p_br_contiguous"_h:
    case "ld3w_z_p_br_contiguous"_h:
      form = form_3;
      break;
    case "ld4b_z_p_br_contiguous"_h:
    case "ld4d_z_p_br_contiguous"_h:
    case "ld4h_z_p_br_contiguous"_h:
    case "ld4w_z_p_br_contiguous"_h:
      form = form_4;
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVELoadPredicateRegister(const Instruction *instr) {
  const char *form = "'Pd, ['Xns, #'s2116:1210, mul vl]";
  if (instr->Mask(0x003f1c00) == 0) {
    form = "'Pd, ['Xns]";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVELoadVectorRegister(const Instruction *instr) {
  const char *form = "'Zt, ['Xns, #'s2116:1210, mul vl]";
  if (instr->Mask(0x003f1c00) == 0) {
    form = "'Zd, ['Xns]";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEPartitionBreakCondition(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.b, p'u1310/'?04:mz, 'Pn.b");
}

void Disassembler::VisitSVEPermutePredicateElements(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.'t, 'Pn.'t, 'Pm.'t");
}

void Disassembler::VisitSVEPredicateFirstActive(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.b, 'Pn, 'Pd.b");
}

void Disassembler::VisitSVEPredicateReadFromFFR_Unpredicated(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.b");
}

void Disassembler::VisitSVEPredicateTest(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "p'u1310, 'Pn.b");
}

void Disassembler::VisitSVEPredicateZero(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.b");
}

void Disassembler::VisitSVEPropagateBreakToNextPartition(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.b, p'u1310/z, 'Pn.b, 'Pd.b");
}

void Disassembler::VisitSVEReversePredicateElements(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.'t, 'Pn.'t");
}

void Disassembler::VisitSVEReverseVectorElements(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVEReverseWithinElements(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'Zd.'t, 'Pgl/m, 'Zn.'t";

  unsigned size = instr->GetSVESize();
  switch (instr->Mask(SVEReverseWithinElementsMask)) {
    case RBIT_z_p_z:
      mnemonic = "rbit";
      break;
    case REVB_z_z:
      if ((size == kHRegSizeInBytesLog2) || (size == kSRegSizeInBytesLog2) ||
          (size == kDRegSizeInBytesLog2)) {
        mnemonic = "revb";
      } else {
        form = "(SVEReverseWithinElements)";
      }
      break;
    case REVH_z_z:
      if ((size == kSRegSizeInBytesLog2) || (size == kDRegSizeInBytesLog2)) {
        mnemonic = "revh";
      } else {
        form = "(SVEReverseWithinElements)";
      }
      break;
    case REVW_z_z:
      if (size == kDRegSizeInBytesLog2) {
        mnemonic = "revw";
      } else {
        form = "(SVEReverseWithinElements)";
      }
      break;
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVESaturatingIncDecRegisterByElementCount(
    const Instruction *instr) {
  const char *form = IncDecFormHelper(instr,
                                      "'R20d, 'Ipc, mul #'u1916+1",
                                      "'R20d, 'Ipc",
                                      "'R20d");
  const char *form_sx = IncDecFormHelper(instr,
                                         "'Xd, 'Wd, 'Ipc, mul #'u1916+1",
                                         "'Xd, 'Wd, 'Ipc",
                                         "'Xd, 'Wd");

  switch (form_hash_) {
    case "sqdecb_r_rs_sx"_h:
    case "sqdecd_r_rs_sx"_h:
    case "sqdech_r_rs_sx"_h:
    case "sqdecw_r_rs_sx"_h:
    case "sqincb_r_rs_sx"_h:
    case "sqincd_r_rs_sx"_h:
    case "sqinch_r_rs_sx"_h:
    case "sqincw_r_rs_sx"_h:
      form = form_sx;
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVESaturatingIncDecVectorByElementCount(
    const Instruction *instr) {
  const char *form = IncDecFormHelper(instr,
                                      "'Zd.'t, 'Ipc, mul #'u1916+1",
                                      "'Zd.'t, 'Ipc",
                                      "'Zd.'t");
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEStoreMultipleStructures_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.'tmsz, 'Zt2.'tmsz}";
  const char *form_3 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz}";
  const char *form_4 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz, 'Zt4.'tmsz}";
  const char *suffix = ", 'Pgl, ['Xns'ISveSvl]";

  switch (form_hash_) {
    case "st3b_z_p_bi_contiguous"_h:
    case "st3h_z_p_bi_contiguous"_h:
    case "st3w_z_p_bi_contiguous"_h:
    case "st3d_z_p_bi_contiguous"_h:
      form = form_3;
      break;
    case "st4b_z_p_bi_contiguous"_h:
    case "st4h_z_p_bi_contiguous"_h:
    case "st4w_z_p_bi_contiguous"_h:
    case "st4d_z_p_bi_contiguous"_h:
      form = form_4;
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEStoreMultipleStructures_ScalarPlusScalar(
    const Instruction *instr) {
  const char *form = "{'Zt.'tmsz, 'Zt2.'tmsz}";
  const char *form_3 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz}";
  const char *form_4 = "{'Zt.'tmsz, 'Zt2.'tmsz, 'Zt3.'tmsz, 'Zt4.'tmsz}";
  const char *suffix = ", 'Pgl, ['Xns, 'Xm'NSveS]";

  switch (form_hash_) {
    case "st3b_z_p_br_contiguous"_h:
    case "st3d_z_p_br_contiguous"_h:
    case "st3h_z_p_br_contiguous"_h:
    case "st3w_z_p_br_contiguous"_h:
      form = form_3;
      break;
    case "st4b_z_p_br_contiguous"_h:
    case "st4d_z_p_br_contiguous"_h:
    case "st4h_z_p_br_contiguous"_h:
    case "st4w_z_p_br_contiguous"_h:
      form = form_4;
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEStorePredicateRegister(const Instruction *instr) {
  const char *form = "'Pd, ['Xns, #'s2116:1210, mul vl]";
  if (instr->Mask(0x003f1c00) == 0) {
    form = "'Pd, ['Xns]";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEStoreVectorRegister(const Instruction *instr) {
  const char *form = "'Zt, ['Xns, #'s2116:1210, mul vl]";
  if (instr->Mask(0x003f1c00) == 0) {
    form = "'Zd, ['Xns]";
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVETableLookup(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, {'Zn.'t}, 'Zm.'t");
}

void Disassembler::VisitSVEUnpackPredicateElements(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.h, 'Pn.b");
}

void Disassembler::VisitSVEUnpackVectorElements(const Instruction *instr) {
  if (instr->GetSVESize() == 0) {
    // The lowest lane size of the destination vector is H-sized lane.
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'th");
  }
}

void Disassembler::VisitSVEVectorSplice(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl, 'Zd.'t, 'Zn.'t");
}

void Disassembler::VisitSVEAddressGeneration(const Instruction *instr) {
  const char *mnemonic = "adr";
  const char *form = "'Zd.d, ['Zn.d, 'Zm.d";
  const char *suffix = NULL;

  bool msz_is_zero = (instr->ExtractBits(11, 10) == 0);

  switch (instr->Mask(SVEAddressGenerationMask)) {
    case ADR_z_az_d_s32_scaled:
      suffix = msz_is_zero ? ", sxtw]" : ", sxtw #'u1110]";
      break;
    case ADR_z_az_d_u32_scaled:
      suffix = msz_is_zero ? ", uxtw]" : ", uxtw #'u1110]";
      break;
    case ADR_z_az_s_same_scaled:
    case ADR_z_az_d_same_scaled:
      form = "'Zd.'t, ['Zn.'t, 'Zm.'t";
      suffix = msz_is_zero ? "]" : ", lsl #'u1110]";
      break;
    default:
      mnemonic = "unimplemented";
      form = "(SVEAddressGeneration)";
      break;
  }
  Format(instr, mnemonic, form, suffix);
}

void Disassembler::VisitSVEBitwiseLogicalUnpredicated(
    const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "'Zd.d, 'Zn.d, 'Zm.d";

  switch (instr->Mask(SVEBitwiseLogicalUnpredicatedMask)) {
    case AND_z_zz:
      mnemonic = "and";
      break;
    case BIC_z_zz:
      mnemonic = "bic";
      break;
    case EOR_z_zz:
      mnemonic = "eor";
      break;
    case ORR_z_zz:
      mnemonic = "orr";
      if (instr->GetRn() == instr->GetRm()) {
        mnemonic = "mov";
        form = "'Zd.d, 'Zn.d";
      }
      break;
    default:
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEBitwiseShiftUnpredicated(const Instruction *instr) {
  const char *mnemonic = "unimplemented";
  const char *form = "(SVEBitwiseShiftUnpredicated)";
  unsigned tsize =
      (instr->ExtractBits(23, 22) << 2) | instr->ExtractBits(20, 19);
  unsigned lane_size = instr->GetSVESize();

  const char *suffix = NULL;
  const char *form_i = "'Zd.'tszs, 'Zn.'tszs, ";

  switch (form_hash_) {
    case "asr_z_zi"_h:
    case "lsr_z_zi"_h:
    case "sri_z_zzi"_h:
    case "srsra_z_zi"_h:
    case "ssra_z_zi"_h:
    case "ursra_z_zi"_h:
    case "usra_z_zi"_h:
      if (tsize != 0) {
        // The tsz field must not be zero.
        mnemonic = mnemonic_.c_str();
        form = form_i;
        suffix = "'ITriSves";
      }
      break;
    case "lsl_z_zi"_h:
    case "sli_z_zzi"_h:
      if (tsize != 0) {
        // The tsz field must not be zero.
        mnemonic = mnemonic_.c_str();
        form = form_i;
        suffix = "'ITriSver";
      }
      break;
    case "asr_z_zw"_h:
    case "lsl_z_zw"_h:
    case "lsr_z_zw"_h:
      if (lane_size <= kSRegSizeInBytesLog2) {
        mnemonic = mnemonic_.c_str();
        form = "'Zd.'t, 'Zn.'t, 'Zm.d";
      }
      break;
    default:
      break;
  }

  Format(instr, mnemonic, form, suffix);
}

void Disassembler::VisitSVEElementCount(const Instruction *instr) {
  const char *form =
      IncDecFormHelper(instr, "'Xd, 'Ipc, mul #'u1916+1", "'Xd, 'Ipc", "'Xd");
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEFPAccumulatingReduction(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'t'u0400, 'Pgl, 't'u0400, 'Zn.'t");
  }
}

void Disassembler::VisitSVEFPArithmeticUnpredicated(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'t, 'Zm.'t");
  }
}

void Disassembler::VisitSVEFPCompareVectors(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Pd.'t, 'Pgl/z, 'Zn.'t, 'Zm.'t");
  }
}

void Disassembler::VisitSVEFPCompareWithZero(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Pd.'t, 'Pgl/z, 'Zn.'t, #0.0");
  }
}

void Disassembler::VisitSVEFPComplexAddition(const Instruction *instr) {
  // Bit 15 is always set, so this gives 90 * 1 or 3.
  const char *form = "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t, #'u1615*90";
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, form);
  }
}

void Disassembler::VisitSVEFPComplexMulAdd(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/m, 'Zn.'t, 'Zm.'t, #'u1413*90";
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, form);
  }
}

void Disassembler::VisitSVEFPComplexMulAddIndex(const Instruction *instr) {
  const char *form = "'Zd.h, 'Zn.h, z'u1816.h['u2019]";
  const char *suffix = ", #'u1110*90";
  switch (form_hash_) {
    case "fcmla_z_zzzi_s"_h:
      form = "'Zd.s, 'Zn.s, z'u1916.s['u2020]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEFPFastReduction(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'t'u0400, 'Pgl, 'Zn.'t");
  }
}

void Disassembler::VisitSVEFPMulIndex(const Instruction *instr) {
  const char *form = "'Zd.h, 'Zn.h, z'u1816.h['u2222:2019]";
  switch (form_hash_) {
    case "fmul_z_zzi_d"_h:
      form = "'Zd.d, 'Zn.d, z'u1916.d['u2020]";
      break;
    case "fmul_z_zzi_s"_h:
      form = "'Zd.s, 'Zn.s, z'u1816.s['u2019]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEFPMulAdd(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zn.'t, 'Zm.'t");
  }
}

void Disassembler::VisitSVEFPMulAddIndex(const Instruction *instr) {
  const char *form = "'Zd.h, 'Zn.h, z'u1816.h['u2222:2019]";
  switch (form_hash_) {
    case "fmla_z_zzzi_s"_h:
    case "fmls_z_zzzi_s"_h:
      form = "'Zd.s, 'Zn.s, z'u1816.s['u2019]";
      break;
    case "fmla_z_zzzi_d"_h:
    case "fmls_z_zzzi_d"_h:
      form = "'Zd.d, 'Zn.d, z'u1916.d['u2020]";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEFPUnaryOpUnpredicated(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    VisitUnallocated(instr);
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'t");
  }
}

void Disassembler::VisitSVEIncDecByPredicateCount(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pn";
  switch (form_hash_) {
    // <Xdn>, <Pg>.<T>
    case "decp_r_p_r"_h:
    case "incp_r_p_r"_h:
      form = "'Xd, 'Pn.'t";
      break;
    // <Xdn>, <Pg>.<T>, <Wdn>
    case "sqdecp_r_p_r_sx"_h:
    case "sqincp_r_p_r_sx"_h:
      form = "'Xd, 'Pn.'t, 'Wd";
      break;
    // <Xdn>, <Pg>.<T>
    case "sqdecp_r_p_r_x"_h:
    case "sqincp_r_p_r_x"_h:
    case "uqdecp_r_p_r_x"_h:
    case "uqincp_r_p_r_x"_h:
      form = "'Xd, 'Pn.'t";
      break;
    // <Wdn>, <Pg>.<T>
    case "uqdecp_r_p_r_uw"_h:
    case "uqincp_r_p_r_uw"_h:
      form = "'Wd, 'Pn.'t";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEIndexGeneration(const Instruction *instr) {
  const char *form = "'Zd.'t, #'s0905, #'s2016";
  bool w_inputs =
      static_cast<unsigned>(instr->GetSVESize()) <= kWRegSizeInBytesLog2;

  switch (form_hash_) {
    case "index_z_ir"_h:
      form = w_inputs ? "'Zd.'t, #'s0905, 'Wm" : "'Zd.'t, #'s0905, 'Xm";
      break;
    case "index_z_ri"_h:
      form = w_inputs ? "'Zd.'t, 'Wn, #'s2016" : "'Zd.'t, 'Xn, #'s2016";
      break;
    case "index_z_rr"_h:
      form = w_inputs ? "'Zd.'t, 'Wn, 'Wm" : "'Zd.'t, 'Xn, 'Xm";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEIntArithmeticUnpredicated(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'t, 'Zm.'t");
}

void Disassembler::VisitSVEIntCompareSignedImm(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.'t, 'Pgl/z, 'Zn.'t, #'s2016");
}

void Disassembler::VisitSVEIntCompareUnsignedImm(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.'t, 'Pgl/z, 'Zn.'t, #'u2014");
}

void Disassembler::VisitSVEIntCompareVectors(const Instruction *instr) {
  const char *form = "'Pd.'t, 'Pgl/z, 'Zn.'t, 'Zm.";
  const char *suffix = "d";
  switch (form_hash_) {
    case "cmpeq_p_p_zz"_h:
    case "cmpge_p_p_zz"_h:
    case "cmpgt_p_p_zz"_h:
    case "cmphi_p_p_zz"_h:
    case "cmphs_p_p_zz"_h:
    case "cmpne_p_p_zz"_h:
      suffix = "'t";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEIntMulAddPredicated(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/m, ";
  const char *suffix = "'Zn.'t, 'Zm.'t";
  switch (form_hash_) {
    case "mad_z_p_zzz"_h:
    case "msb_z_p_zzz"_h:
      suffix = "'Zm.'t, 'Zn.'t";
      break;
  }
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEIntMulAddUnpredicated(const Instruction *instr) {
  if (static_cast<unsigned>(instr->GetSVESize()) >= kSRegSizeInBytesLog2) {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'tq, 'Zm.'tq");
  } else {
    VisitUnallocated(instr);
  }
}

void Disassembler::VisitSVEMovprfx(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/'?16:mz, 'Zn.'t");
}

void Disassembler::VisitSVEIntReduction(const Instruction *instr) {
  const char *form = "'Vdv, 'Pgl, 'Zn.'t";
  switch (form_hash_) {
    case "saddv_r_p_z"_h:
    case "uaddv_r_p_z"_h:
      form = "'Dd, 'Pgl, 'Zn.'t";
      break;
  }
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEIntUnaryArithmeticPredicated(
    const Instruction *instr) {
  VectorFormat vform = instr->GetSVEVectorFormat();

  switch (form_hash_) {
    case "sxtw_z_p_z"_h:
    case "uxtw_z_p_z"_h:
      if (vform == kFormatVnS) {
        VisitUnallocated(instr);
        return;
      }
      VIXL_FALLTHROUGH();
    case "sxth_z_p_z"_h:
    case "uxth_z_p_z"_h:
      if (vform == kFormatVnH) {
        VisitUnallocated(instr);
        return;
      }
      VIXL_FALLTHROUGH();
    case "sxtb_z_p_z"_h:
    case "uxtb_z_p_z"_h:
    case "fabs_z_p_z"_h:
    case "fneg_z_p_z"_h:
      if (vform == kFormatVnB) {
        VisitUnallocated(instr);
        return;
      }
      break;
  }

  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Pgl/m, 'Zn.'t");
}

void Disassembler::VisitSVEMulIndex(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.b, z'u1816.b['u2019]";

  switch (form_hash_) {
    case "sdot_z_zzzi_d"_h:
    case "udot_z_zzzi_d"_h:
      form = "'Zd.d, 'Zn.h, z'u1916.h['u2020]";
      break;
  }

  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEPermuteVectorExtract(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.b, 'Zd.b, 'Zn.b, #'u2016:1210");
}

void Disassembler::VisitSVEPermuteVectorInterleaving(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'t, 'Zm.'t");
}

void Disassembler::VisitSVEPredicateCount(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Xd, p'u1310, 'Pn.'t");
}

void Disassembler::VisitSVEPredicateLogical(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Pd.b, p'u1310/z, 'Pn.b, 'Pm.b";

  int pd = instr->GetPd();
  int pn = instr->GetPn();
  int pm = instr->GetPm();
  int pg = instr->ExtractBits(13, 10);

  switch (form_hash_) {
    case "ands_p_p_pp_z"_h:
      if (pn == pm) {
        mnemonic = "movs";
        form = "'Pd.b, p'u1310/z, 'Pn.b";
      }
      break;
    case "and_p_p_pp_z"_h:
      if (pn == pm) {
        mnemonic = "mov";
        form = "'Pd.b, p'u1310/z, 'Pn.b";
      }
      break;
    case "eors_p_p_pp_z"_h:
      if (pm == pg) {
        mnemonic = "nots";
        form = "'Pd.b, 'Pm/z, 'Pn.b";
      }
      break;
    case "eor_p_p_pp_z"_h:
      if (pm == pg) {
        mnemonic = "not";
        form = "'Pd.b, 'Pm/z, 'Pn.b";
      }
      break;
    case "orrs_p_p_pp_z"_h:
      if ((pn == pm) && (pn == pg)) {
        mnemonic = "movs";
        form = "'Pd.b, 'Pn.b";
      }
      break;
    case "orr_p_p_pp_z"_h:
      if ((pn == pm) && (pn == pg)) {
        mnemonic = "mov";
        form = "'Pd.b, 'Pn.b";
      }
      break;
    case "sel_p_p_pp"_h:
      if (pd == pm) {
        mnemonic = "mov";
        form = "'Pd.b, p'u1310/m, 'Pn.b";
      } else {
        form = "'Pd.b, p'u1310, 'Pn.b, 'Pm.b";
      }
      break;
  }
  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEPredicateInitialize(const Instruction *instr) {
  const char *form = "'Pd.'t, 'Ipc";
  // Omit the pattern if it is the default ('ALL').
  if (instr->ExtractBits(9, 5) == SVE_ALL) form = "'Pd.'t";
  FormatWithDecodedMnemonic(instr, form);
}

void Disassembler::VisitSVEPredicateNextActive(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.'t, 'Pn, 'Pd.'t");
}

void Disassembler::VisitSVEPredicateReadFromFFR_Predicated(
    const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.b, 'Pn/z");
}

void Disassembler::VisitSVEPropagateBreak(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Pd.b, p'u1310/z, 'Pn.b, 'Pm.b");
}

void Disassembler::VisitSVEStackFrameAdjustment(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Xds, 'Xms, #'s1005");
}

void Disassembler::VisitSVEStackFrameSize(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Xd, #'s1005");
}

void Disassembler::VisitSVEVectorSelect(const Instruction *instr) {
  const char *mnemonic = mnemonic_.c_str();
  const char *form = "'Zd.'t, p'u1310, 'Zn.'t, 'Zm.'t";

  if (instr->GetRd() == instr->GetRm()) {
    mnemonic = "mov";
    form = "'Zd.'t, p'u1310/m, 'Zn.'t";
  }

  Format(instr, mnemonic, form);
}

void Disassembler::VisitSVEContiguousLoad_ScalarPlusImm(
    const Instruction *instr) {
  const char *form = "{'Zt.'tlss}, 'Pgl/z, ['Xns";
  const char *suffix =
      (instr->ExtractBits(19, 16) == 0) ? "]" : ", #'s1916, mul vl]";
  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitSVEContiguousLoad_ScalarPlusScalar(
    const Instruction *instr) {
  const char *form = "{'Zt.'tlss}, 'Pgl/z, ['Xns, 'Xm";
  const char *suffix = "]";

  switch (form_hash_) {
    case "ld1h_z_p_br_u16"_h:
    case "ld1h_z_p_br_u32"_h:
    case "ld1h_z_p_br_u64"_h:
    case "ld1w_z_p_br_u32"_h:
    case "ld1w_z_p_br_u64"_h:
    case "ld1d_z_p_br_u64"_h:
      suffix = ", lsl #'u2423]";
      break;
    case "ld1sh_z_p_br_s32"_h:
    case "ld1sh_z_p_br_s64"_h:
      suffix = ", lsl #1]";
      break;
    case "ld1sw_z_p_br_s64"_h:
      suffix = ", lsl #2]";
      break;
  }

  FormatWithDecodedMnemonic(instr, form, suffix);
}

void Disassembler::VisitReserved(const Instruction *instr) {
  // UDF is the only instruction in this group, and the Decoder is precise.
  VIXL_ASSERT(instr->Mask(ReservedMask) == UDF);
  Format(instr, "udf", "'IUdf");
}

void Disassembler::VisitUnimplemented(const Instruction *instr) {
  Format(instr, "unimplemented", "(Unimplemented)");
}


void Disassembler::VisitUnallocated(const Instruction *instr) {
  Format(instr, "unallocated", "(Unallocated)");
}

void Disassembler::Visit(Metadata *metadata, const Instruction *instr) {
  VIXL_ASSERT(metadata->count("form") > 0);
  const std::string &form = (*metadata)["form"];
  form_hash_ = Hash(form.c_str());
  const FormToVisitorFnMap *fv = Disassembler::GetFormToVisitorFnMap();
  FormToVisitorFnMap::const_iterator it = fv->find(form_hash_);
  if (it == fv->end()) {
    VisitUnimplemented(instr);
  } else {
    SetMnemonicFromForm(form);
    (it->second)(this, instr);
  }
}

void Disassembler::Disassemble_PdT_PgZ_ZnT_ZmT(const Instruction *instr) {
  const char *form = "'Pd.'t, 'Pgl/z, 'Zn.'t, 'Zm.'t";
  VectorFormat vform = instr->GetSVEVectorFormat();

  if ((vform == kFormatVnS) || (vform == kFormatVnD)) {
    Format(instr, "unimplemented", "(PdT_PgZ_ZnT_ZmT)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::Disassemble_ZdB_Zn1B_Zn2B_imm(const Instruction *instr) {
  const char *form = "'Zd.b, {'Zn.b, 'Zn2.b}, #'u2016:1210";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdB_ZnB_ZmB(const Instruction *instr) {
  const char *form = "'Zd.b, 'Zn.b, 'Zm.b";
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    Format(instr, mnemonic_.c_str(), form);
  } else {
    Format(instr, "unimplemented", "(ZdB_ZnB_ZmB)");
  }
}

void Disassembler::Disassemble_ZdD_PgM_ZnS(const Instruction *instr) {
  const char *form = "'Zd.d, 'Pgl/m, 'Zn.s";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdD_ZnD_ZmD(const Instruction *instr) {
  const char *form = "'Zd.d, 'Zn.d, 'Zm.d";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdD_ZnD_ZmD_imm(const Instruction *instr) {
  const char *form = "'Zd.d, 'Zn.d, z'u1916.d['u2020]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdD_ZnS_ZmS_imm(const Instruction *instr) {
  const char *form = "'Zd.d, 'Zn.s, z'u1916.s['u2020:1111]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdH_PgM_ZnS(const Instruction *instr) {
  const char *form = "'Zd.h, 'Pgl/m, 'Zn.s";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdH_ZnH_ZmH_imm(const Instruction *instr) {
  const char *form = "'Zd.h, 'Zn.h, z'u1816.h['u2222:2019]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdS_PgM_ZnD(const Instruction *instr) {
  const char *form = "'Zd.s, 'Pgl/m, 'Zn.d";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdS_PgM_ZnH(const Instruction *instr) {
  const char *form = "'Zd.s, 'Pgl/m, 'Zn.h";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdS_PgM_ZnS(const Instruction *instr) {
  const char *form = "'Zd.s, 'Pgl/m, 'Zn.s";
  if (instr->GetSVEVectorFormat() == kFormatVnS) {
    Format(instr, mnemonic_.c_str(), form);
  } else {
    Format(instr, "unimplemented", "(ZdS_PgM_ZnS)");
  }
}

void Disassembler::Disassemble_ZdS_ZnH_ZmH_imm(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.h, z'u1816.h['u2019:1111]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdS_ZnS_ZmS(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.s, 'Zm.s";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdS_ZnS_ZmS_imm(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.s, z'u1816.s['u2019]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::DisassembleSVEFlogb(const Instruction *instr) {
  const char *form = "'Zd.'tf, 'Pgl/m, 'Zn.'tf";
  if (instr->GetSVEVectorFormat(17) == kFormatVnB) {
    Format(instr, "unimplemented", "(SVEFlogb)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::Disassemble_ZdT_PgM_ZnT(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/m, 'Zn.'t";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdT_PgZ_ZnT_ZmT(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/z, 'Zn.'t, 'Zm.'t";
  VectorFormat vform = instr->GetSVEVectorFormat();
  if ((vform == kFormatVnS) || (vform == kFormatVnD)) {
    Format(instr, mnemonic_.c_str(), form);
  } else {
    Format(instr, "unimplemented", "(ZdT_PgZ_ZnT_ZmT)");
  }
}

void Disassembler::Disassemble_ZdT_Pg_Zn1T_Zn2T(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl, {'Zn.'t, 'Zn2.'t}";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdT_Zn1T_Zn2T_ZmT(const Instruction *instr) {
  const char *form = "'Zd.'t, {'Zn.'t, 'Zn2.'t}, 'Zm.'t";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdT_ZnT_ZmT(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zn.'t, 'Zm.'t";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdT_ZnT_ZmTb(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zn.'t, 'Zm.'th";
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    Format(instr, "unimplemented", "(ZdT_ZnT_ZmTb)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::Disassemble_ZdT_ZnTb(const Instruction *instr) {
  const char *form = "'Zd.'tszs, 'Zn.'tszd";
  std::pair<int, int> shift_and_lane_size =
      instr->GetSVEImmShiftAndLaneSizeLog2(/* is_predicated = */ false);
  int shift_dist = shift_and_lane_size.first;
  int lane_size = shift_and_lane_size.second;
  // Convert shift_dist from a right to left shift. Valid xtn instructions
  // must have a left shift_dist equivalent of zero.
  shift_dist = (8 << lane_size) - shift_dist;
  if ((lane_size >= static_cast<int>(kBRegSizeInBytesLog2)) &&
      (lane_size <= static_cast<int>(kSRegSizeInBytesLog2)) &&
      (shift_dist == 0)) {
    Format(instr, mnemonic_.c_str(), form);
  } else {
    Format(instr, "unimplemented", "(ZdT_ZnTb)");
  }
}

void Disassembler::DisassembleSVEPmull(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnS) {
    VisitUnallocated(instr);
  } else {
    Disassemble_ZdT_ZnTb_ZmTb(instr);
  }
}

void Disassembler::DisassembleSVEPmull128(const Instruction *instr) {
  FormatWithDecodedMnemonic(instr, "'Zd.q, 'Zn.d, 'Zm.d");
}

void Disassembler::Disassemble_ZdT_ZnTb_ZmTb(const Instruction *instr) {
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    // TODO: This is correct for saddlbt, ssublbt, subltb, which don't have
    // b-lane sized form, but may need changes for other instructions reaching
    // here.
    Format(instr, "unimplemented", "(ZdT_ZnTb_ZmTb)");
  } else {
    FormatWithDecodedMnemonic(instr, "'Zd.'t, 'Zn.'th, 'Zm.'th");
  }
}

void Disassembler::DisassembleSVEAddSubHigh(const Instruction *instr) {
  const char *form = "'Zd.'th, 'Zn.'t, 'Zm.'t";
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    Format(instr, "unimplemented", "(SVEAddSubHigh)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::DisassembleSVEShiftLeftImm(const Instruction *instr) {
  const char *form = "'Zd.'tszd, 'Zn.'tszs, 'ITriSver";
  std::pair<int, int> shift_and_lane_size =
      instr->GetSVEImmShiftAndLaneSizeLog2(/* is_predicated = */ false);
  int lane_size = shift_and_lane_size.second;
  if ((lane_size >= static_cast<int>(kBRegSizeInBytesLog2)) &&
      (lane_size <= static_cast<int>(kSRegSizeInBytesLog2))) {
    Format(instr, mnemonic_.c_str(), form);
  } else {
    Format(instr, "unimplemented", "(SVEShiftLeftImm)");
  }
}

void Disassembler::DisassembleSVEShiftRightImm(const Instruction *instr) {
  const char *form = "'Zd.'tszs, 'Zn.'tszd, 'ITriSves";
  std::pair<int, int> shift_and_lane_size =
      instr->GetSVEImmShiftAndLaneSizeLog2(/* is_predicated = */ false);
  int lane_size = shift_and_lane_size.second;
  if ((lane_size >= static_cast<int>(kBRegSizeInBytesLog2)) &&
      (lane_size <= static_cast<int>(kSRegSizeInBytesLog2))) {
    Format(instr, mnemonic_.c_str(), form);
  } else {
    Format(instr, "unimplemented", "(SVEShiftRightImm)");
  }
}

void Disassembler::Disassemble_ZdaD_ZnD_ZmD_imm(const Instruction *instr) {
  const char *form = "'Zd.d, 'Zn.d, z'u1916.d['u2020]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaD_ZnH_ZmH_imm_const(
    const Instruction *instr) {
  const char *form = "'Zd.d, 'Zn.h, z'u1916.h['u2020], #'u1110*90";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaD_ZnS_ZmS_imm(const Instruction *instr) {
  const char *form = "'Zd.d, 'Zn.s, z'u1916.s['u2020:1111]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaH_ZnH_ZmH_imm(const Instruction *instr) {
  const char *form = "'Zd.h, 'Zn.h, z'u1816.h['u2222:2019]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaH_ZnH_ZmH_imm_const(
    const Instruction *instr) {
  const char *form = "'Zd.h, 'Zn.h, z'u1816.h['u2019], #'u1110*90";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaS_ZnB_ZmB(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.b, 'Zm.b";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaS_ZnB_ZmB_imm_const(
    const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.b, z'u1816.b['u2019], #'u1110*90";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaS_ZnH_ZmH(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.h, 'Zm.h";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaS_ZnH_ZmH_imm(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.h, z'u1816.h['u2019:1111]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaS_ZnS_ZmS_imm(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.s, z'u1816.s['u2019]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaS_ZnS_ZmS_imm_const(
    const Instruction *instr) {
  const char *form = "'Zd.s, 'Zn.s, z'u1916.s['u2020], #'u1110*90";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaT_PgM_ZnTb(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/m, 'Zn.'th";

  if (instr->GetSVESize() == 0) {
    // The lowest lane size of the destination vector is H-sized lane.
    Format(instr, "unimplemented", "(Disassemble_ZdaT_PgM_ZnTb)");
    return;
  }

  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::DisassembleSVEAddSubCarry(const Instruction *instr) {
  const char *form = "'Zd.'?22:ds, 'Zn.'?22:ds, 'Zm.'?22:ds";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaT_ZnT_ZmT(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zn.'t, 'Zm.'t";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaT_ZnT_ZmT_const(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zn.'t, 'Zm.'t, #'u1110*90";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdaT_ZnTb_ZmTb(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zn.'th, 'Zm.'th";
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    Format(instr, "unimplemented", "(ZdaT_ZnTb_ZmTb)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::Disassemble_ZdaT_ZnTb_ZmTb_const(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zn.'tq, 'Zm.'tq, #'u1110*90";
  VectorFormat vform = instr->GetSVEVectorFormat();

  if ((vform == kFormatVnB) || (vform == kFormatVnH)) {
    Format(instr, "unimplemented", "(ZdaT_ZnTb_ZmTb_const)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::Disassemble_ZdnB_ZdnB(const Instruction *instr) {
  const char *form = "'Zd.b, 'Zd.b";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdnB_ZdnB_ZmB(const Instruction *instr) {
  const char *form = "'Zd.b, 'Zd.b, 'Zn.b";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::DisassembleSVEBitwiseTernary(const Instruction *instr) {
  const char *form = "'Zd.d, 'Zd.d, 'Zm.d, 'Zn.d";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_ZdnS_ZdnS_ZmS(const Instruction *instr) {
  const char *form = "'Zd.s, 'Zd.s, 'Zn.s";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::DisassembleSVEFPPair(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t";
  if (instr->GetSVEVectorFormat() == kFormatVnB) {
    Format(instr, "unimplemented", "(SVEFPPair)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::Disassemble_ZdnT_PgM_ZdnT_ZmT(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Pgl/m, 'Zd.'t, 'Zn.'t";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::DisassembleSVEComplexIntAddition(const Instruction *instr) {
  const char *form = "'Zd.'t, 'Zd.'t, 'Zn.'t, #";
  const char *suffix = (instr->ExtractBit(10) == 0) ? "90" : "270";
  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::Disassemble_ZdnT_ZdnT_ZmT_const(const Instruction *instr) {
  const char *form = "'Zd.'tszs, 'Zd.'tszs, 'Zn.'tszs, 'ITriSves";
  unsigned tsize =
      (instr->ExtractBits(23, 22) << 2) | instr->ExtractBits(20, 19);

  if (tsize == 0) {
    Format(instr, "unimplemented", "(ZdnT_ZdnT_ZmT_const)");
  } else {
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::Disassemble_ZtD_PgZ_ZnD_Xm(const Instruction *instr) {
  const char *form = "{'Zt.d}, 'Pgl/z, ['Zn.d";
  const char *suffix = instr->GetRm() == 31 ? "]" : ", 'Xm]";
  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::Disassemble_ZtD_Pg_ZnD_Xm(const Instruction *instr) {
  const char *form = "{'Zt.d}, 'Pgl, ['Zn.d";
  const char *suffix = instr->GetRm() == 31 ? "]" : ", 'Xm]";
  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::Disassemble_ZtS_PgZ_ZnS_Xm(const Instruction *instr) {
  const char *form = "{'Zt.s}, 'Pgl/z, ['Zn.s";
  const char *suffix = instr->GetRm() == 31 ? "]" : ", 'Xm]";
  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::Disassemble_ZtS_Pg_ZnS_Xm(const Instruction *instr) {
  const char *form = "{'Zt.s}, 'Pgl, ['Zn.s";
  const char *suffix = instr->GetRm() == 31 ? "]" : ", 'Xm]";
  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::Disassemble_XdSP_XnSP_Xm(const Instruction *instr) {
  const char *form = "'Xds, 'Xns";
  const char *suffix = instr->GetRm() == 31 ? "" : ", 'Xm";
  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::Disassemble_XdSP_XnSP_uimm6_uimm4(const Instruction *instr) {
  VIXL_STATIC_ASSERT(kMTETagGranuleInBytes == 16);
  const char *form = "'Xds, 'Xns, #'u2116*16, #'u1310";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_Xd_XnSP_Xm(const Instruction *instr) {
  const char *form = "'Rd, 'Xns, 'Rm";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::Disassemble_Xd_XnSP_XmSP(const Instruction *instr) {
  if ((form_hash_ == Hash("subps_64s_dp_2src")) && (instr->GetRd() == 31)) {
    Format(instr, "cmpp", "'Xns, 'Xms");
  } else {
    const char *form = "'Xd, 'Xns, 'Xms";
    Format(instr, mnemonic_.c_str(), form);
  }
}

void Disassembler::DisassembleMTEStoreTagPair(const Instruction *instr) {
  const char *form = "'Xt, 'Xt2, ['Xns";
  const char *suffix = NULL;
  switch (form_hash_) {
    case Hash("stgp_64_ldstpair_off"):
      suffix = ", #'s2115*16]";
      break;
    case Hash("stgp_64_ldstpair_post"):
      suffix = "], #'s2115*16";
      break;
    case Hash("stgp_64_ldstpair_pre"):
      suffix = ", #'s2115*16]!";
      break;
    default:
      mnemonic_ = "unimplemented";
      break;
  }

  if (instr->GetImmLSPair() == 0) {
    suffix = "]";
  }

  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::DisassembleMTEStoreTag(const Instruction *instr) {
  const char *form = "'Xds, ['Xns";
  const char *suffix = NULL;
  switch (form_hash_) {
    case Hash("st2g_64soffset_ldsttags"):
    case Hash("stg_64soffset_ldsttags"):
    case Hash("stz2g_64soffset_ldsttags"):
    case Hash("stzg_64soffset_ldsttags"):
      suffix = ", #'s2012*16]";
      break;
    case Hash("st2g_64spost_ldsttags"):
    case Hash("stg_64spost_ldsttags"):
    case Hash("stz2g_64spost_ldsttags"):
    case Hash("stzg_64spost_ldsttags"):
      suffix = "], #'s2012*16";
      break;
    case Hash("st2g_64spre_ldsttags"):
    case Hash("stg_64spre_ldsttags"):
    case Hash("stz2g_64spre_ldsttags"):
    case Hash("stzg_64spre_ldsttags"):
      suffix = ", #'s2012*16]!";
      break;
    default:
      mnemonic_ = "unimplemented";
      break;
  }

  if (instr->GetImmLS() == 0) {
    suffix = "]";
  }

  Format(instr, mnemonic_.c_str(), form, suffix);
}

void Disassembler::DisassembleMTELoadTag(const Instruction *instr) {
  const char *form =
      (instr->GetImmLS() == 0) ? "'Xt, ['Xns]" : "'Xt, ['Xns, #'s2012*16]";
  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::DisassembleCpy(const Instruction *instr) {
  const char *form = "['Xd]!, ['Xs]!, 'Xn!";

  int d = instr->GetRd();
  int n = instr->GetRn();
  int s = instr->GetRs();

  // Aliased registers and sp/zr are disallowed.
  if ((d == n) || (d == s) || (n == s) || (d == 31) || (n == 31) || (s == 31)) {
    form = NULL;
  }

  // Bits 31 and 30 must be zero.
  if (instr->ExtractBits(31, 30)) {
    form = NULL;
  }

  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::DisassembleSet(const Instruction *instr) {
  const char *form = "['Xd]!, 'Xn!, 'Xs";

  int d = instr->GetRd();
  int n = instr->GetRn();
  int s = instr->GetRs();

  // Aliased registers are disallowed. Only Xs may be xzr.
  if ((d == n) || (d == s) || (n == s) || (d == 31) || (n == 31)) {
    form = NULL;
  }

  // Bits 31 and 30 must be zero.
  if (instr->ExtractBits(31, 30)) {
    form = NULL;
  }

  Format(instr, mnemonic_.c_str(), form);
}

void Disassembler::ProcessOutput(const Instruction * /*instr*/) {
  // The base disasm does nothing more than disassembling into a buffer.
}


void Disassembler::AppendRegisterNameToOutput(const Instruction *instr,
                                              const CPURegister &reg) {
  USE(instr);
  VIXL_ASSERT(reg.IsValid());
  char reg_char;

  if (reg.IsRegister()) {
    reg_char = reg.Is64Bits() ? 'x' : 'w';
  } else {
    VIXL_ASSERT(reg.IsVRegister());
    switch (reg.GetSizeInBits()) {
      case kBRegSize:
        reg_char = 'b';
        break;
      case kHRegSize:
        reg_char = 'h';
        break;
      case kSRegSize:
        reg_char = 's';
        break;
      case kDRegSize:
        reg_char = 'd';
        break;
      default:
        VIXL_ASSERT(reg.Is128Bits());
        reg_char = 'q';
    }
  }

  if (reg.IsVRegister() || !(reg.Aliases(sp) || reg.Aliases(xzr))) {
    // A core or scalar/vector register: [wx]0 - 30, [bhsdq]0 - 31.
    AppendToOutput("%c%d", reg_char, reg.GetCode());
  } else if (reg.Aliases(sp)) {
    // Disassemble w31/x31 as stack pointer wsp/sp.
    AppendToOutput("%s", reg.Is64Bits() ? "sp" : "wsp");
  } else {
    // Disassemble w31/x31 as zero register wzr/xzr.
    AppendToOutput("%czr", reg_char);
  }
}


void Disassembler::AppendPCRelativeOffsetToOutput(const Instruction *instr,
                                                  int64_t offset) {
  USE(instr);
  if (offset < 0) {
    // Cast to uint64_t so that INT64_MIN is handled in a well-defined way.
    uint64_t abs_offset = UnsignedNegate(static_cast<uint64_t>(offset));
    AppendToOutput("#-0x%" PRIx64, abs_offset);
  } else {
    AppendToOutput("#+0x%" PRIx64, offset);
  }
}


void Disassembler::AppendAddressToOutput(const Instruction *instr,
                                         const void *addr) {
  USE(instr);
  AppendToOutput("(addr 0x%" PRIxPTR ")", reinterpret_cast<uintptr_t>(addr));
}


void Disassembler::AppendCodeAddressToOutput(const Instruction *instr,
                                             const void *addr) {
  AppendAddressToOutput(instr, addr);
}


void Disassembler::AppendDataAddressToOutput(const Instruction *instr,
                                             const void *addr) {
  AppendAddressToOutput(instr, addr);
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


void Disassembler::AppendCodeRelativeCodeAddressToOutput(
    const Instruction *instr, const void *addr) {
  AppendCodeRelativeAddressToOutput(instr, addr);
}


void Disassembler::AppendCodeRelativeDataAddressToOutput(
    const Instruction *instr, const void *addr) {
  AppendCodeRelativeAddressToOutput(instr, addr);
}


void Disassembler::MapCodeAddress(int64_t base_address,
                                  const Instruction *instr_address) {
  set_code_address_offset(base_address -
                          reinterpret_cast<intptr_t>(instr_address));
}
int64_t Disassembler::CodeRelativeAddress(const void *addr) {
  return reinterpret_cast<intptr_t>(addr) + code_address_offset();
}


void Disassembler::Format(const Instruction *instr,
                          const char *mnemonic,
                          const char *format0,
                          const char *format1) {
  if ((mnemonic == NULL) || (format0 == NULL)) {
    VisitUnallocated(instr);
  } else {
    ResetOutput();
    Substitute(instr, mnemonic);
    if (format0[0] != 0) {  // Not a zero-length string.
      VIXL_ASSERT(buffer_pos_ < buffer_size_);
      buffer_[buffer_pos_++] = ' ';
      Substitute(instr, format0);
      // TODO: consider using a zero-length string here, too.
      if (format1 != NULL) {
        Substitute(instr, format1);
      }
    }
    VIXL_ASSERT(buffer_pos_ < buffer_size_);
    buffer_[buffer_pos_] = 0;
    ProcessOutput(instr);
  }
}

void Disassembler::FormatWithDecodedMnemonic(const Instruction *instr,
                                             const char *format0,
                                             const char *format1) {
  Format(instr, mnemonic_.c_str(), format0, format1);
}

void Disassembler::Substitute(const Instruction *instr, const char *string) {
  char chr = *string++;
  while (chr != '\0') {
    if (chr == '\'') {
      string += SubstituteField(instr, string);
    } else {
      VIXL_ASSERT(buffer_pos_ < buffer_size_);
      buffer_[buffer_pos_++] = chr;
    }
    chr = *string++;
  }
}


int Disassembler::SubstituteField(const Instruction *instr,
                                  const char *format) {
  switch (format[0]) {
    // NB. The remaining substitution prefix upper-case characters are: JU.
    case 'R':  // Register. X or W, selected by sf (or alternative) bit.
    case 'F':  // FP register. S or D, selected by type field.
    case 'V':  // Vector register, V, vector format.
    case 'Z':  // Scalable vector register.
    case 'W':
    case 'X':
    case 'B':
    case 'H':
    case 'S':
    case 'D':
    case 'Q':
      return SubstituteRegisterField(instr, format);
    case 'P':
      return SubstitutePredicateRegisterField(instr, format);
    case 'I':
      return SubstituteImmediateField(instr, format);
    case 'L':
      return SubstituteLiteralField(instr, format);
    case 'N':
      return SubstituteShiftField(instr, format);
    case 'C':
      return SubstituteConditionField(instr, format);
    case 'E':
      return SubstituteExtendField(instr, format);
    case 'A':
      return SubstitutePCRelAddressField(instr, format);
    case 'T':
      return SubstituteBranchTargetField(instr, format);
    case 'O':
      return SubstituteLSRegOffsetField(instr, format);
    case 'M':
      return SubstituteBarrierField(instr, format);
    case 'K':
      return SubstituteCrField(instr, format);
    case 'G':
      return SubstituteSysOpField(instr, format);
    case 'p':
      return SubstitutePrefetchField(instr, format);
    case 'u':
    case 's':
      return SubstituteIntField(instr, format);
    case 't':
      return SubstituteSVESize(instr, format);
    case '?':
      return SubstituteTernary(instr, format);
    default: {
      VIXL_UNREACHABLE();
      return 1;
    }
  }
}

std::pair<unsigned, unsigned> Disassembler::GetRegNumForField(
    const Instruction *instr, char reg_prefix, const char *field) {
  unsigned reg_num = UINT_MAX;
  unsigned field_len = 1;

  switch (field[0]) {
    case 'd':
      reg_num = instr->GetRd();
      break;
    case 'n':
      reg_num = instr->GetRn();
      break;
    case 'm':
      reg_num = instr->GetRm();
      break;
    case 'e':
      // This is register Rm, but using a 4-bit specifier. Used in NEON
      // by-element instructions.
      reg_num = instr->GetRmLow16();
      break;
    case 'f':
      // This is register Rm, but using an element size dependent number of bits
      // in the register specifier.
      reg_num =
          (instr->GetNEONSize() < 2) ? instr->GetRmLow16() : instr->GetRm();
      break;
    case 'a':
      reg_num = instr->GetRa();
      break;
    case 's':
      reg_num = instr->GetRs();
      break;
    case 't':
      reg_num = instr->GetRt();
      break;
    default:
      VIXL_UNREACHABLE();
  }

  switch (field[1]) {
    case '2':
    case '3':
    case '4':
      if ((reg_prefix == 'V') || (reg_prefix == 'Z')) {  // t2/3/4, n2/3/4
        VIXL_ASSERT((field[0] == 't') || (field[0] == 'n'));
        reg_num = (reg_num + field[1] - '1') % 32;
        field_len++;
      } else {
        VIXL_ASSERT((field[0] == 't') && (field[1] == '2'));
        reg_num = instr->GetRt2();
        field_len++;
      }
      break;
    case '+':  // Rt+, Rs+ (ie. Rt + 1, Rs + 1)
      VIXL_ASSERT((reg_prefix == 'W') || (reg_prefix == 'X'));
      VIXL_ASSERT((field[0] == 's') || (field[0] == 't'));
      reg_num++;
      field_len++;
      break;
    case 's':  // Core registers that are (w)sp rather than zr.
      VIXL_ASSERT((reg_prefix == 'W') || (reg_prefix == 'X'));
      reg_num = (reg_num == kZeroRegCode) ? kSPRegInternalCode : reg_num;
      field_len++;
      break;
  }

  VIXL_ASSERT(reg_num != UINT_MAX);
  return std::make_pair(reg_num, field_len);
}

int Disassembler::SubstituteRegisterField(const Instruction *instr,
                                          const char *format) {
  unsigned field_len = 1;  // Initially, count only the first character.

  // The first character of the register format field, eg R, X, S, etc.
  char reg_prefix = format[0];

  // Pointer to the character after the prefix. This may be one of the standard
  // symbols representing a register encoding, or a two digit bit position,
  // handled by the following code.
  const char *reg_field = &format[1];

  if (reg_prefix == 'R') {
    bool is_x = instr->GetSixtyFourBits() == 1;
    if (strspn(reg_field, "0123456789") == 2) {  // r20d, r31n, etc.
      // Core W or X registers where the type is determined by a specified bit
      // position, eg. 'R20d, 'R05n. This is like the 'Rd syntax, where bit 31
      // is implicitly used to select between W and X.
      int bitpos = ((reg_field[0] - '0') * 10) + (reg_field[1] - '0');
      VIXL_ASSERT(bitpos <= 31);
      is_x = (instr->ExtractBit(bitpos) == 1);
      reg_field = &format[3];
      field_len += 2;
    }
    reg_prefix = is_x ? 'X' : 'W';
  }

  std::pair<unsigned, unsigned> rn =
      GetRegNumForField(instr, reg_prefix, reg_field);
  unsigned reg_num = rn.first;
  field_len += rn.second;

  if (reg_field[0] == 'm') {
    switch (reg_field[1]) {
      // Handle registers tagged with b (bytes), z (instruction), or
      // r (registers), used for address updates in NEON load/store
      // instructions.
      case 'r':
      case 'b':
      case 'z': {
        VIXL_ASSERT(reg_prefix == 'X');
        field_len = 3;
        char *eimm;
        int imm = static_cast<int>(strtol(&reg_field[2], &eimm, 10));
        field_len += static_cast<unsigned>(eimm - &reg_field[2]);
        if (reg_num == 31) {
          switch (reg_field[1]) {
            case 'z':
              imm *= (1 << instr->GetNEONLSSize());
              break;
            case 'r':
              imm *= (instr->GetNEONQ() == 0) ? kDRegSizeInBytes
                                              : kQRegSizeInBytes;
              break;
            case 'b':
              break;
          }
          AppendToOutput("#%d", imm);
          return field_len;
        }
        break;
      }
    }
  }

  CPURegister::RegisterType reg_type = CPURegister::kRegister;
  unsigned reg_size = kXRegSize;

  if (reg_prefix == 'F') {
    switch (instr->GetFPType()) {
      case 3:
        reg_prefix = 'H';
        break;
      case 0:
        reg_prefix = 'S';
        break;
      default:
        reg_prefix = 'D';
    }
  }

  switch (reg_prefix) {
    case 'W':
      reg_type = CPURegister::kRegister;
      reg_size = kWRegSize;
      break;
    case 'X':
      reg_type = CPURegister::kRegister;
      reg_size = kXRegSize;
      break;
    case 'B':
      reg_type = CPURegister::kVRegister;
      reg_size = kBRegSize;
      break;
    case 'H':
      reg_type = CPURegister::kVRegister;
      reg_size = kHRegSize;
      break;
    case 'S':
      reg_type = CPURegister::kVRegister;
      reg_size = kSRegSize;
      break;
    case 'D':
      reg_type = CPURegister::kVRegister;
      reg_size = kDRegSize;
      break;
    case 'Q':
      reg_type = CPURegister::kVRegister;
      reg_size = kQRegSize;
      break;
    case 'V':
      if (reg_field[1] == 'v') {
        reg_type = CPURegister::kVRegister;
        reg_size = 1 << (instr->GetSVESize() + 3);
        field_len++;
        break;
      }
      AppendToOutput("v%d", reg_num);
      return field_len;
    case 'Z':
      AppendToOutput("z%d", reg_num);
      return field_len;
    default:
      VIXL_UNREACHABLE();
  }

  AppendRegisterNameToOutput(instr, CPURegister(reg_num, reg_size, reg_type));

  return field_len;
}

int Disassembler::SubstitutePredicateRegisterField(const Instruction *instr,
                                                   const char *format) {
  VIXL_ASSERT(format[0] == 'P');
  switch (format[1]) {
    // This field only supports P register that are always encoded in the same
    // position.
    case 'd':
    case 't':
      AppendToOutput("p%u", instr->GetPt());
      break;
    case 'n':
      AppendToOutput("p%u", instr->GetPn());
      break;
    case 'm':
      AppendToOutput("p%u", instr->GetPm());
      break;
    case 'g':
      VIXL_ASSERT(format[2] == 'l');
      AppendToOutput("p%u", instr->GetPgLow8());
      return 3;
    default:
      VIXL_UNREACHABLE();
  }
  return 2;
}

int Disassembler::SubstituteImmediateField(const Instruction *instr,
                                           const char *format) {
  VIXL_ASSERT(format[0] == 'I');

  switch (format[1]) {
    case 'M': {  // IMoveImm, IMoveNeg or IMoveLSL.
      if (format[5] == 'L') {
        AppendToOutput("#0x%" PRIx32, instr->GetImmMoveWide());
        if (instr->GetShiftMoveWide() > 0) {
          AppendToOutput(", lsl #%" PRId32, 16 * instr->GetShiftMoveWide());
        }
      } else {
        VIXL_ASSERT((format[5] == 'I') || (format[5] == 'N'));
        uint64_t imm = static_cast<uint64_t>(instr->GetImmMoveWide())
                       << (16 * instr->GetShiftMoveWide());
        if (format[5] == 'N') imm = ~imm;
        if (!instr->GetSixtyFourBits()) imm &= UINT64_C(0xffffffff);
        AppendToOutput("#0x%" PRIx64, imm);
      }
      return 8;
    }
    case 'L': {
      switch (format[2]) {
        case 'L': {  // ILLiteral - Immediate Load Literal.
          AppendToOutput("pc%+" PRId32,
                         instr->GetImmLLiteral() *
                             static_cast<int>(kLiteralEntrySize));
          return 9;
        }
        case 'S': {  // ILS - Immediate Load/Store.
                     // ILSi - As above, but an index field which must not be
                     // omitted even if it is zero.
          bool is_index = format[3] == 'i';
          if (is_index || (instr->GetImmLS() != 0)) {
            AppendToOutput(", #%" PRId32, instr->GetImmLS());
          }
          return is_index ? 4 : 3;
        }
        case 'P': {  // ILPx - Immediate Load/Store Pair, x = access size.
                     // ILPxi - As above, but an index field which must not be
                     // omitted even if it is zero.
          VIXL_ASSERT((format[3] >= '0') && (format[3] <= '9'));
          bool is_index = format[4] == 'i';
          if (is_index || (instr->GetImmLSPair() != 0)) {
            // format[3] is the scale value. Convert to a number.
            int scale = 1 << (format[3] - '0');
            AppendToOutput(", #%" PRId32, instr->GetImmLSPair() * scale);
          }
          return is_index ? 5 : 4;
        }
        case 'U': {  // ILU - Immediate Load/Store Unsigned.
          if (instr->GetImmLSUnsigned() != 0) {
            int shift = instr->GetSizeLS();
            AppendToOutput(", #%" PRId32, instr->GetImmLSUnsigned() << shift);
          }
          return 3;
        }
        case 'A': {  // ILA - Immediate Load with pointer authentication.
          if (instr->GetImmLSPAC() != 0) {
            AppendToOutput(", #%" PRId32, instr->GetImmLSPAC());
          }
          return 3;
        }
        default: {
          VIXL_UNIMPLEMENTED();
          return 0;
        }
      }
    }
    case 'C': {  // ICondB - Immediate Conditional Branch.
      int64_t offset = instr->GetImmCondBranch() << 2;
      AppendPCRelativeOffsetToOutput(instr, offset);
      return 6;
    }
    case 'A': {  // IAddSub.
      int64_t imm = instr->GetImmAddSub() << (12 * instr->GetImmAddSubShift());
      AppendToOutput("#0x%" PRIx64 " (%" PRId64 ")", imm, imm);
      return 7;
    }
    case 'F': {  // IFP, IFPNeon, IFPSve or IFPFBits.
      int imm8 = 0;
      size_t len = strlen("IFP");
      switch (format[3]) {
        case 'F':
          VIXL_ASSERT(strncmp(format, "IFPFBits", strlen("IFPFBits")) == 0);
          AppendToOutput("#%" PRId32, 64 - instr->GetFPScale());
          return static_cast<int>(strlen("IFPFBits"));
        case 'N':
          VIXL_ASSERT(strncmp(format, "IFPNeon", strlen("IFPNeon")) == 0);
          imm8 = instr->GetImmNEONabcdefgh();
          len += strlen("Neon");
          break;
        case 'S':
          VIXL_ASSERT(strncmp(format, "IFPSve", strlen("IFPSve")) == 0);
          imm8 = instr->ExtractBits(12, 5);
          len += strlen("Sve");
          break;
        default:
          VIXL_ASSERT(strncmp(format, "IFP", strlen("IFP")) == 0);
          imm8 = instr->GetImmFP();
          break;
      }
      AppendToOutput("#0x%" PRIx32 " (%.4f)",
                     imm8,
                     Instruction::Imm8ToFP32(imm8));
      return static_cast<int>(len);
    }
    case 'H': {  // IH - ImmHint
      AppendToOutput("#%" PRId32, instr->GetImmHint());
      return 2;
    }
    case 'T': {  // ITri - Immediate Triangular Encoded.
      if (format[4] == 'S') {
        VIXL_ASSERT((format[5] == 'v') && (format[6] == 'e'));
        switch (format[7]) {
          case 'l':
            // SVE logical immediate encoding.
            AppendToOutput("#0x%" PRIx64, instr->GetSVEImmLogical());
            return 8;
          case 'p': {
            // SVE predicated shift immediate encoding, lsl.
            std::pair<int, int> shift_and_lane_size =
                instr->GetSVEImmShiftAndLaneSizeLog2(
                    /* is_predicated = */ true);
            int lane_bits = 8 << shift_and_lane_size.second;
            AppendToOutput("#%" PRId32, lane_bits - shift_and_lane_size.first);
            return 8;
          }
          case 'q': {
            // SVE predicated shift immediate encoding, asr and lsr.
            std::pair<int, int> shift_and_lane_size =
                instr->GetSVEImmShiftAndLaneSizeLog2(
                    /* is_predicated = */ true);
            AppendToOutput("#%" PRId32, shift_and_lane_size.first);
            return 8;
          }
          case 'r': {
            // SVE unpredicated shift immediate encoding, left shifts.
            std::pair<int, int> shift_and_lane_size =
                instr->GetSVEImmShiftAndLaneSizeLog2(
                    /* is_predicated = */ false);
            int lane_bits = 8 << shift_and_lane_size.second;
            AppendToOutput("#%" PRId32, lane_bits - shift_and_lane_size.first);
            return 8;
          }
          case 's': {
            // SVE unpredicated shift immediate encoding, right shifts.
            std::pair<int, int> shift_and_lane_size =
                instr->GetSVEImmShiftAndLaneSizeLog2(
                    /* is_predicated = */ false);
            AppendToOutput("#%" PRId32, shift_and_lane_size.first);
            return 8;
          }
          default:
            VIXL_UNREACHABLE();
            return 0;
        }
      } else {
        AppendToOutput("#0x%" PRIx64, instr->GetImmLogical());
        return 4;
      }
    }
    case 'N': {  // INzcv.
      int nzcv = (instr->GetNzcv() << Flags_offset);
      AppendToOutput("#%c%c%c%c",
                     ((nzcv & NFlag) == 0) ? 'n' : 'N',
                     ((nzcv & ZFlag) == 0) ? 'z' : 'Z',
                     ((nzcv & CFlag) == 0) ? 'c' : 'C',
                     ((nzcv & VFlag) == 0) ? 'v' : 'V');
      return 5;
    }
    case 'P': {  // IP - Conditional compare.
      AppendToOutput("#%" PRId32, instr->GetImmCondCmp());
      return 2;
    }
    case 'B': {  // Bitfields.
      return SubstituteBitfieldImmediateField(instr, format);
    }
    case 'E': {  // IExtract.
      AppendToOutput("#%" PRId32, instr->GetImmS());
      return 8;
    }
    case 't': {  // It - Test and branch bit.
      AppendToOutput("#%" PRId32,
                     (instr->GetImmTestBranchBit5() << 5) |
                         instr->GetImmTestBranchBit40());
      return 2;
    }
    case 'S': {  // ISveSvl - SVE 'mul vl' immediate for structured ld/st.
      VIXL_ASSERT(strncmp(format, "ISveSvl", 7) == 0);
      int imm = instr->ExtractSignedBits(19, 16);
      if (imm != 0) {
        int reg_count = instr->ExtractBits(22, 21) + 1;
        AppendToOutput(", #%d, mul vl", imm * reg_count);
      }
      return 7;
    }
    case 's': {  // Is - Shift (immediate).
      switch (format[2]) {
        case 'R': {  // IsR - right shifts.
          int shift = 16 << HighestSetBitPosition(instr->GetImmNEONImmh());
          shift -= instr->GetImmNEONImmhImmb();
          AppendToOutput("#%d", shift);
          return 3;
        }
        case 'L': {  // IsL - left shifts.
          int shift = instr->GetImmNEONImmhImmb();
          shift -= 8 << HighestSetBitPosition(instr->GetImmNEONImmh());
          AppendToOutput("#%d", shift);
          return 3;
        }
        default: {
          VIXL_UNIMPLEMENTED();
          return 0;
        }
      }
    }
    case 'D': {  // IDebug - HLT and BRK instructions.
      AppendToOutput("#0x%" PRIx32, instr->GetImmException());
      return 6;
    }
    case 'U': {  // IUdf - UDF immediate.
      AppendToOutput("#0x%" PRIx32, instr->GetImmUdf());
      return 4;
    }
    case 'V': {  // Immediate Vector.
      switch (format[2]) {
        case 'E': {  // IVExtract.
          AppendToOutput("#%" PRId32, instr->GetImmNEONExt());
          return 9;
        }
        case 'B': {  // IVByElemIndex.
          int ret = static_cast<int>(strlen("IVByElemIndex"));
          uint32_t vm_index = instr->GetNEONH() << 2;
          vm_index |= instr->GetNEONL() << 1;
          vm_index |= instr->GetNEONM();

          static const char *format_rot = "IVByElemIndexRot";
          static const char *format_fhm = "IVByElemIndexFHM";
          if (strncmp(format, format_rot, strlen(format_rot)) == 0) {
            // FCMLA uses 'H' bit index when SIZE is 2, else H:L
            VIXL_ASSERT((instr->GetNEONSize() == 1) ||
                        (instr->GetNEONSize() == 2));
            vm_index >>= instr->GetNEONSize();
            ret = static_cast<int>(strlen(format_rot));
          } else if (strncmp(format, format_fhm, strlen(format_fhm)) == 0) {
            // Nothing to do - FMLAL and FMLSL use H:L:M.
            ret = static_cast<int>(strlen(format_fhm));
          } else {
            if (instr->GetNEONSize() == 2) {
              // S-sized elements use H:L.
              vm_index >>= 1;
            } else if (instr->GetNEONSize() == 3) {
              // D-sized elements use H.
              vm_index >>= 2;
            }
          }
          AppendToOutput("%d", vm_index);
          return ret;
        }
        case 'I': {  // INS element.
          if (strncmp(format, "IVInsIndex", strlen("IVInsIndex")) == 0) {
            unsigned rd_index, rn_index;
            unsigned imm5 = instr->GetImmNEON5();
            unsigned imm4 = instr->GetImmNEON4();
            int tz = CountTrailingZeros(imm5, 32);
            if (tz <= 3) {  // Defined for tz = 0 to 3 only.
              rd_index = imm5 >> (tz + 1);
              rn_index = imm4 >> tz;
              if (strncmp(format, "IVInsIndex1", strlen("IVInsIndex1")) == 0) {
                AppendToOutput("%d", rd_index);
                return static_cast<int>(strlen("IVInsIndex1"));
              } else if (strncmp(format,
                                 "IVInsIndex2",
                                 strlen("IVInsIndex2")) == 0) {
                AppendToOutput("%d", rn_index);
                return static_cast<int>(strlen("IVInsIndex2"));
              }
            }
            return 0;
          } else if (strncmp(format,
                             "IVInsSVEIndex",
                             strlen("IVInsSVEIndex")) == 0) {
            std::pair<int, int> index_and_lane_size =
                instr->GetSVEPermuteIndexAndLaneSizeLog2();
            AppendToOutput("%d", index_and_lane_size.first);
            return static_cast<int>(strlen("IVInsSVEIndex"));
          }
          VIXL_FALLTHROUGH();
        }
        case 'L': {  // IVLSLane[0123] - suffix indicates access size shift.
          AppendToOutput("%d", instr->GetNEONLSIndex(format[8] - '0'));
          return 9;
        }
        case 'M': {  // Modified Immediate cases.
          if (strncmp(format, "IVMIImm8", strlen("IVMIImm8")) == 0) {
            uint64_t imm8 = instr->GetImmNEONabcdefgh();
            AppendToOutput("#0x%" PRIx64, imm8);
            return static_cast<int>(strlen("IVMIImm8"));
          } else if (strncmp(format, "IVMIImm", strlen("IVMIImm")) == 0) {
            uint64_t imm8 = instr->GetImmNEONabcdefgh();
            uint64_t imm = 0;
            for (int i = 0; i < 8; ++i) {
              if (imm8 & (UINT64_C(1) << i)) {
                imm |= (UINT64_C(0xff) << (8 * i));
              }
            }
            AppendToOutput("#0x%" PRIx64, imm);
            return static_cast<int>(strlen("IVMIImm"));
          } else if (strncmp(format,
                             "IVMIShiftAmt1",
                             strlen("IVMIShiftAmt1")) == 0) {
            int cmode = instr->GetNEONCmode();
            int shift_amount = 8 * ((cmode >> 1) & 3);
            AppendToOutput("#%d", shift_amount);
            return static_cast<int>(strlen("IVMIShiftAmt1"));
          } else if (strncmp(format,
                             "IVMIShiftAmt2",
                             strlen("IVMIShiftAmt2")) == 0) {
            int cmode = instr->GetNEONCmode();
            int shift_amount = 8 << (cmode & 1);
            AppendToOutput("#%d", shift_amount);
            return static_cast<int>(strlen("IVMIShiftAmt2"));
          } else {
            VIXL_UNIMPLEMENTED();
            return 0;
          }
        }
        default: {
          VIXL_UNIMPLEMENTED();
          return 0;
        }
      }
    }
    case 'X': {  // IX - CLREX instruction.
      AppendToOutput("#0x%" PRIx32, instr->GetCRm());
      return 2;
    }
    case 'Y': {  // IY - system register immediate.
      switch (instr->GetImmSystemRegister()) {
        case NZCV:
          AppendToOutput("nzcv");
          break;
        case FPCR:
          AppendToOutput("fpcr");
          break;
        case RNDR:
          AppendToOutput("rndr");
          break;
        case RNDRRS:
          AppendToOutput("rndrrs");
          break;
        case DCZID_EL0:
          AppendToOutput("dczid_el0");
          break;
        default:
          AppendToOutput("S%d_%d_c%d_c%d_%d",
                         instr->GetSysOp0(),
                         instr->GetSysOp1(),
                         instr->GetCRn(),
                         instr->GetCRm(),
                         instr->GetSysOp2());
          break;
      }
      return 2;
    }
    case 'R': {  // IR - Rotate right into flags.
      switch (format[2]) {
        case 'r': {  // IRr - Rotate amount.
          AppendToOutput("#%d", instr->GetImmRMIFRotation());
          return 3;
        }
        default: {
          VIXL_UNIMPLEMENTED();
          return 0;
        }
      }
    }
    case 'p': {  // Ipc - SVE predicate constraint specifier.
      VIXL_ASSERT(format[2] == 'c');
      unsigned pattern = instr->GetImmSVEPredicateConstraint();
      switch (pattern) {
        // VL1-VL8 are encoded directly.
        case SVE_VL1:
        case SVE_VL2:
        case SVE_VL3:
        case SVE_VL4:
        case SVE_VL5:
        case SVE_VL6:
        case SVE_VL7:
        case SVE_VL8:
          AppendToOutput("vl%u", pattern);
          break;
        // VL16-VL256 are encoded as log2(N) + c.
        case SVE_VL16:
        case SVE_VL32:
        case SVE_VL64:
        case SVE_VL128:
        case SVE_VL256:
          AppendToOutput("vl%u", 16 << (pattern - SVE_VL16));
          break;
        // Special cases.
        case SVE_POW2:
          AppendToOutput("pow2");
          break;
        case SVE_MUL4:
          AppendToOutput("mul4");
          break;
        case SVE_MUL3:
          AppendToOutput("mul3");
          break;
        case SVE_ALL:
          AppendToOutput("all");
          break;
        default:
          AppendToOutput("#0x%x", pattern);
          break;
      }
      return 3;
    }
    default: {
      VIXL_UNIMPLEMENTED();
      return 0;
    }
  }
}


int Disassembler::SubstituteBitfieldImmediateField(const Instruction *instr,
                                                   const char *format) {
  VIXL_ASSERT((format[0] == 'I') && (format[1] == 'B'));
  unsigned r = instr->GetImmR();
  unsigned s = instr->GetImmS();

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
        VIXL_ASSERT(format[3] == '-');
        AppendToOutput("#%d", s - r + 1);
        return 7;
      }
    }
    case 'Z': {  // IBZ-r.
      VIXL_ASSERT((format[3] == '-') && (format[4] == 'r'));
      unsigned reg_size =
          (instr->GetSixtyFourBits() == 1) ? kXRegSize : kWRegSize;
      AppendToOutput("#%d", reg_size - r);
      return 5;
    }
    default: {
      VIXL_UNREACHABLE();
      return 0;
    }
  }
}


int Disassembler::SubstituteLiteralField(const Instruction *instr,
                                         const char *format) {
  VIXL_ASSERT(strncmp(format, "LValue", 6) == 0);
  USE(format);

  const void *address = instr->GetLiteralAddress<const void *>();
  switch (instr->Mask(LoadLiteralMask)) {
    case LDR_w_lit:
    case LDR_x_lit:
    case LDRSW_x_lit:
    case LDR_s_lit:
    case LDR_d_lit:
    case LDR_q_lit:
      AppendCodeRelativeDataAddressToOutput(instr, address);
      break;
    case PRFM_lit: {
      // Use the prefetch hint to decide how to print the address.
      switch (instr->GetPrefetchHint()) {
        case 0x0:  // PLD: prefetch for load.
        case 0x2:  // PST: prepare for store.
          AppendCodeRelativeDataAddressToOutput(instr, address);
          break;
        case 0x1:  // PLI: preload instructions.
          AppendCodeRelativeCodeAddressToOutput(instr, address);
          break;
        case 0x3:  // Unallocated hint.
          AppendCodeRelativeAddressToOutput(instr, address);
          break;
      }
      break;
    }
    default:
      VIXL_UNREACHABLE();
  }

  return 6;
}


int Disassembler::SubstituteShiftField(const Instruction *instr,
                                       const char *format) {
  VIXL_ASSERT(format[0] == 'N');
  VIXL_ASSERT(instr->GetShiftDP() <= 0x3);

  switch (format[1]) {
    case 'D': {  // NDP.
      VIXL_ASSERT(instr->GetShiftDP() != ROR);
      VIXL_FALLTHROUGH();
    }
    case 'L': {  // NLo.
      if (instr->GetImmDPShift() != 0) {
        const char *shift_type[] = {"lsl", "lsr", "asr", "ror"};
        AppendToOutput(", %s #%" PRId32,
                       shift_type[instr->GetShiftDP()],
                       instr->GetImmDPShift());
      }
      return 3;
    }
    case 'S': {  // NSveS (SVE structured load/store indexing shift).
      VIXL_ASSERT(strncmp(format, "NSveS", 5) == 0);
      int msz = instr->ExtractBits(24, 23);
      if (msz > 0) {
        AppendToOutput(", lsl #%d", msz);
      }
      return 5;
    }
    default:
      VIXL_UNIMPLEMENTED();
      return 0;
  }
}


int Disassembler::SubstituteConditionField(const Instruction *instr,
                                           const char *format) {
  VIXL_ASSERT(format[0] == 'C');
  const char *condition_code[] = {"eq",
                                  "ne",
                                  "hs",
                                  "lo",
                                  "mi",
                                  "pl",
                                  "vs",
                                  "vc",
                                  "hi",
                                  "ls",
                                  "ge",
                                  "lt",
                                  "gt",
                                  "le",
                                  "al",
                                  "nv"};
  int cond;
  switch (format[1]) {
    case 'B':
      cond = instr->GetConditionBranch();
      break;
    case 'I': {
      cond = InvertCondition(static_cast<Condition>(instr->GetCondition()));
      break;
    }
    default:
      cond = instr->GetCondition();
  }
  AppendToOutput("%s", condition_code[cond]);
  return 4;
}


int Disassembler::SubstitutePCRelAddressField(const Instruction *instr,
                                              const char *format) {
  VIXL_ASSERT((strcmp(format, "AddrPCRelByte") == 0) ||  // Used by `adr`.
              (strcmp(format, "AddrPCRelPage") == 0));   // Used by `adrp`.

  int64_t offset = instr->GetImmPCRel();

  // Compute the target address based on the effective address (after applying
  // code_address_offset). This is required for correct behaviour of adrp.
  const Instruction *base = instr + code_address_offset();
  if (format[9] == 'P') {
    offset *= kPageSize;
    base = AlignDown(base, kPageSize);
  }
  // Strip code_address_offset before printing, so we can use the
  // semantically-correct AppendCodeRelativeAddressToOutput.
  const void *target =
      reinterpret_cast<const void *>(base + offset - code_address_offset());

  AppendPCRelativeOffsetToOutput(instr, offset);
  AppendToOutput(" ");
  AppendCodeRelativeAddressToOutput(instr, target);
  return 13;
}


int Disassembler::SubstituteBranchTargetField(const Instruction *instr,
                                              const char *format) {
  VIXL_ASSERT(strncmp(format, "TImm", 4) == 0);

  int64_t offset = 0;
  switch (format[5]) {
    // BImmUncn - unconditional branch immediate.
    case 'n':
      offset = instr->GetImmUncondBranch();
      break;
    // BImmCond - conditional branch immediate.
    case 'o':
      offset = instr->GetImmCondBranch();
      break;
    // BImmCmpa - compare and branch immediate.
    case 'm':
      offset = instr->GetImmCmpBranch();
      break;
    // BImmTest - test and branch immediate.
    case 'e':
      offset = instr->GetImmTestBranch();
      break;
    default:
      VIXL_UNIMPLEMENTED();
  }
  offset *= static_cast<int>(kInstructionSize);
  const void *target_address = reinterpret_cast<const void *>(instr + offset);
  VIXL_STATIC_ASSERT(sizeof(*instr) == 1);

  AppendPCRelativeOffsetToOutput(instr, offset);
  AppendToOutput(" ");
  AppendCodeRelativeCodeAddressToOutput(instr, target_address);

  return 8;
}


int Disassembler::SubstituteExtendField(const Instruction *instr,
                                        const char *format) {
  VIXL_ASSERT(strncmp(format, "Ext", 3) == 0);
  VIXL_ASSERT(instr->GetExtendMode() <= 7);
  USE(format);

  const char *extend_mode[] =
      {"uxtb", "uxth", "uxtw", "uxtx", "sxtb", "sxth", "sxtw", "sxtx"};

  // If rd or rn is SP, uxtw on 32-bit registers and uxtx on 64-bit
  // registers becomes lsl.
  if (((instr->GetRd() == kZeroRegCode) || (instr->GetRn() == kZeroRegCode)) &&
      (((instr->GetExtendMode() == UXTW) && (instr->GetSixtyFourBits() == 0)) ||
       (instr->GetExtendMode() == UXTX))) {
    if (instr->GetImmExtendShift() > 0) {
      AppendToOutput(", lsl #%" PRId32, instr->GetImmExtendShift());
    }
  } else {
    AppendToOutput(", %s", extend_mode[instr->GetExtendMode()]);
    if (instr->GetImmExtendShift() > 0) {
      AppendToOutput(" #%" PRId32, instr->GetImmExtendShift());
    }
  }
  return 3;
}


int Disassembler::SubstituteLSRegOffsetField(const Instruction *instr,
                                             const char *format) {
  VIXL_ASSERT(strncmp(format, "Offsetreg", 9) == 0);
  const char *extend_mode[] = {"undefined",
                               "undefined",
                               "uxtw",
                               "lsl",
                               "undefined",
                               "undefined",
                               "sxtw",
                               "sxtx"};
  USE(format);

  unsigned shift = instr->GetImmShiftLS();
  Extend ext = static_cast<Extend>(instr->GetExtendMode());
  char reg_type = ((ext == UXTW) || (ext == SXTW)) ? 'w' : 'x';

  unsigned rm = instr->GetRm();
  if (rm == kZeroRegCode) {
    AppendToOutput("%czr", reg_type);
  } else {
    AppendToOutput("%c%d", reg_type, rm);
  }

  // Extend mode UXTX is an alias for shift mode LSL here.
  if (!((ext == UXTX) && (shift == 0))) {
    AppendToOutput(", %s", extend_mode[ext]);
    if (shift != 0) {
      AppendToOutput(" #%d", instr->GetSizeLS());
    }
  }
  return 9;
}


int Disassembler::SubstitutePrefetchField(const Instruction *instr,
                                          const char *format) {
  VIXL_ASSERT(format[0] == 'p');
  USE(format);

  bool is_sve =
      (strncmp(format, "prefSVEOp", strlen("prefSVEOp")) == 0) ? true : false;
  int placeholder_length = is_sve ? 9 : 6;
  static const char *stream_options[] = {"keep", "strm"};

  auto get_hints = [](bool want_sve_hint) -> std::vector<std::string> {
    static const std::vector<std::string> sve_hints = {"ld", "st"};
    static const std::vector<std::string> core_hints = {"ld", "li", "st"};
    return (want_sve_hint) ? sve_hints : core_hints;
  };

  std::vector<std::string> hints = get_hints(is_sve);
  unsigned hint =
      is_sve ? instr->GetSVEPrefetchHint() : instr->GetPrefetchHint();
  unsigned target = instr->GetPrefetchTarget() + 1;
  unsigned stream = instr->GetPrefetchStream();

  if ((hint >= hints.size()) || (target > 3)) {
    // Unallocated prefetch operations.
    if (is_sve) {
      std::bitset<4> prefetch_mode(instr->GetSVEImmPrefetchOperation());
      AppendToOutput("#0b%s", prefetch_mode.to_string().c_str());
    } else {
      std::bitset<5> prefetch_mode(instr->GetImmPrefetchOperation());
      AppendToOutput("#0b%s", prefetch_mode.to_string().c_str());
    }
  } else {
    VIXL_ASSERT(stream < ArrayLength(stream_options));
    AppendToOutput("p%sl%d%s",
                   hints[hint].c_str(),
                   target,
                   stream_options[stream]);
  }
  return placeholder_length;
}

int Disassembler::SubstituteBarrierField(const Instruction *instr,
                                         const char *format) {
  VIXL_ASSERT(format[0] == 'M');
  USE(format);

  static const char *options[4][4] = {{"sy (0b0000)", "oshld", "oshst", "osh"},
                                      {"sy (0b0100)", "nshld", "nshst", "nsh"},
                                      {"sy (0b1000)", "ishld", "ishst", "ish"},
                                      {"sy (0b1100)", "ld", "st", "sy"}};
  int domain = instr->GetImmBarrierDomain();
  int type = instr->GetImmBarrierType();

  AppendToOutput("%s", options[domain][type]);
  return 1;
}

int Disassembler::SubstituteSysOpField(const Instruction *instr,
                                       const char *format) {
  VIXL_ASSERT(format[0] == 'G');
  int op = -1;
  switch (format[1]) {
    case '1':
      op = instr->GetSysOp1();
      break;
    case '2':
      op = instr->GetSysOp2();
      break;
    default:
      VIXL_UNREACHABLE();
  }
  AppendToOutput("#%d", op);
  return 2;
}

int Disassembler::SubstituteCrField(const Instruction *instr,
                                    const char *format) {
  VIXL_ASSERT(format[0] == 'K');
  int cr = -1;
  switch (format[1]) {
    case 'n':
      cr = instr->GetCRn();
      break;
    case 'm':
      cr = instr->GetCRm();
      break;
    default:
      VIXL_UNREACHABLE();
  }
  AppendToOutput("C%d", cr);
  return 2;
}

int Disassembler::SubstituteIntField(const Instruction *instr,
                                     const char *format) {
  VIXL_ASSERT((format[0] == 'u') || (format[0] == 's'));

  // A generic signed or unsigned int field uses a placeholder of the form
  // 'sAABB and 'uAABB respectively where AA and BB are two digit bit positions
  // between 00 and 31, and AA >= BB. The placeholder is substituted with the
  // decimal integer represented by the bits in the instruction between
  // positions AA and BB inclusive.
  //
  // In addition, split fields can be represented using 'sAABB:CCDD, where CCDD
  // become the least-significant bits of the result, and bit AA is the sign bit
  // (if 's is used).
  int32_t bits = 0;
  int width = 0;
  const char *c = format;
  do {
    c++;  // Skip the 'u', 's' or ':'.
    VIXL_ASSERT(strspn(c, "0123456789") == 4);
    int msb = ((c[0] - '0') * 10) + (c[1] - '0');
    int lsb = ((c[2] - '0') * 10) + (c[3] - '0');
    c += 4;  // Skip the characters we just read.
    int chunk_width = msb - lsb + 1;
    VIXL_ASSERT((chunk_width > 0) && (chunk_width < 32));
    bits = (bits << chunk_width) | (instr->ExtractBits(msb, lsb));
    width += chunk_width;
  } while (*c == ':');
  VIXL_ASSERT(IsUintN(width, bits));

  if (format[0] == 's') {
    bits = ExtractSignedBitfield32(width - 1, 0, bits);
  }

  if (*c == '+') {
    // A "+n" trailing the format specifier indicates the extracted value should
    // be incremented by n. This is for cases where the encoding is zero-based,
    // but range of values is not, eg. values [1, 16] encoded as [0, 15]
    char *new_c;
    uint64_t value = strtoul(c + 1, &new_c, 10);
    c = new_c;
    VIXL_ASSERT(IsInt32(value));
    bits = static_cast<int32_t>(bits + value);
  } else if (*c == '*') {
    // Similarly, a "*n" trailing the format specifier indicates the extracted
    // value should be multiplied by n. This is for cases where the encoded
    // immediate is scaled, for example by access size.
    char *new_c;
    uint64_t value = strtoul(c + 1, &new_c, 10);
    c = new_c;
    VIXL_ASSERT(IsInt32(value));
    bits = static_cast<int32_t>(bits * value);
  }

  AppendToOutput("%d", bits);

  return static_cast<int>(c - format);
}

int Disassembler::SubstituteSVESize(const Instruction *instr,
                                    const char *format) {
  USE(format);
  VIXL_ASSERT(format[0] == 't');

  static const char sizes[] = {'b', 'h', 's', 'd', 'q'};
  unsigned size_in_bytes_log2 = instr->GetSVESize();
  int placeholder_length = 1;
  switch (format[1]) {
    case 'f':  // 'tf - FP size encoded in <18:17>
      placeholder_length++;
      size_in_bytes_log2 = instr->ExtractBits(18, 17);
      break;
    case 'l':
      placeholder_length++;
      if (format[2] == 's') {
        // 'tls: Loads and stores
        size_in_bytes_log2 = instr->ExtractBits(22, 21);
        placeholder_length++;
        if (format[3] == 's') {
          // Sign extension load.
          unsigned msize = instr->ExtractBits(24, 23);
          if (msize > size_in_bytes_log2) size_in_bytes_log2 ^= 0x3;
          placeholder_length++;
        }
      } else {
        // 'tl: Logical operations
        size_in_bytes_log2 = instr->GetSVEBitwiseImmLaneSizeInBytesLog2();
      }
      break;
    case 'm':  // 'tmsz
      VIXL_ASSERT(strncmp(format, "tmsz", 4) == 0);
      placeholder_length += 3;
      size_in_bytes_log2 = instr->ExtractBits(24, 23);
      break;
    case 'i': {  // 'ti: indices.
      std::pair<int, int> index_and_lane_size =
          instr->GetSVEPermuteIndexAndLaneSizeLog2();
      placeholder_length++;
      size_in_bytes_log2 = index_and_lane_size.second;
      break;
    }
    case 's':
      if (format[2] == 'z') {
        VIXL_ASSERT((format[3] == 'p') || (format[3] == 's') ||
                    (format[3] == 'd'));
        bool is_predicated = (format[3] == 'p');
        std::pair<int, int> shift_and_lane_size =
            instr->GetSVEImmShiftAndLaneSizeLog2(is_predicated);
        size_in_bytes_log2 = shift_and_lane_size.second;
        if (format[3] == 'd') {  // Double size lanes.
          size_in_bytes_log2++;
        }
        placeholder_length += 3;  // skip "sz(p|s|d)"
      }
      break;
    case 'h':
      // Half size of the lane size field.
      size_in_bytes_log2 -= 1;
      placeholder_length++;
      break;
    case 'q':
      // Quarter size of the lane size field.
      size_in_bytes_log2 -= 2;
      placeholder_length++;
      break;
    default:
      break;
  }

  VIXL_ASSERT(size_in_bytes_log2 < ArrayLength(sizes));
  AppendToOutput("%c", sizes[size_in_bytes_log2]);

  return placeholder_length;
}

int Disassembler::SubstituteTernary(const Instruction *instr,
                                    const char *format) {
  VIXL_ASSERT((format[0] == '?') && (format[3] == ':'));

  // The ternary substitution of the format "'?bb:TF" is replaced by a single
  // character, either T or F, depending on the value of the bit at position
  // bb in the instruction. For example, "'?31:xw" is substituted with "x" if
  // bit 31 is true, and "w" otherwise.
  VIXL_ASSERT(strspn(&format[1], "0123456789") == 2);
  char *c;
  uint64_t value = strtoul(&format[1], &c, 10);
  VIXL_ASSERT(value < (kInstructionSize * kBitsPerByte));
  VIXL_ASSERT((*c == ':') && (strlen(c) >= 3));  // Minimum of ":TF"
  c++;
  AppendToOutput("%c", c[1 - instr->ExtractBit(static_cast<int>(value))]);
  return 6;
}

void Disassembler::ResetOutput() {
  buffer_pos_ = 0;
  buffer_[buffer_pos_] = 0;
}


void Disassembler::AppendToOutput(const char *format, ...) {
  va_list args;
  va_start(args, format);
  buffer_pos_ += vsnprintf(&buffer_[buffer_pos_],
                           buffer_size_ - buffer_pos_,
                           format,
                           args);
  va_end(args);
}


void PrintDisassembler::Disassemble(const Instruction *instr) {
  Decoder decoder;
  if (cpu_features_auditor_ != NULL) {
    decoder.AppendVisitor(cpu_features_auditor_);
  }
  decoder.AppendVisitor(this);
  decoder.Decode(instr);
}

void PrintDisassembler::DisassembleBuffer(const Instruction *start,
                                          const Instruction *end) {
  Decoder decoder;
  if (cpu_features_auditor_ != NULL) {
    decoder.AppendVisitor(cpu_features_auditor_);
  }
  decoder.AppendVisitor(this);
  decoder.Decode(start, end);
}

void PrintDisassembler::DisassembleBuffer(const Instruction *start,
                                          uint64_t size) {
  DisassembleBuffer(start, start + size);
}


void PrintDisassembler::ProcessOutput(const Instruction *instr) {
  int64_t address = CodeRelativeAddress(instr);

  uint64_t abs_address;
  const char *sign;
  if (signed_addresses_) {
    if (address < 0) {
      sign = "-";
      abs_address = UnsignedNegate(static_cast<uint64_t>(address));
    } else {
      // Leave a leading space, to maintain alignment.
      sign = " ";
      abs_address = address;
    }
  } else {
    sign = "";
    abs_address = address;
  }

  int bytes_printed = fprintf(stream_,
                              "%s0x%016" PRIx64 "  %08" PRIx32 "\t\t%s",
                              sign,
                              abs_address,
                              instr->GetInstructionBits(),
                              GetOutput());
  if (cpu_features_auditor_ != NULL) {
    CPUFeatures needs = cpu_features_auditor_->GetInstructionFeatures();
    needs.Remove(cpu_features_auditor_->GetAvailableFeatures());
    if (needs != CPUFeatures::None()) {
      // Try to align annotations. This value is arbitrary, but based on looking
      // good with most instructions. Note that, for historical reasons, the
      // disassembly itself is printed with tab characters, so bytes_printed is
      // _not_ equivalent to the number of occupied screen columns. However, the
      // prefix before the tabs is always the same length, so the annotation
      // indentation does not change from one line to the next.
      const int indent_to = 70;
      // Always allow some space between the instruction and the annotation.
      const int min_pad = 2;

      int pad = std::max(min_pad, (indent_to - bytes_printed));
      fprintf(stream_, "%*s", pad, "");

      std::stringstream features;
      features << needs;
      fprintf(stream_,
              "%s%s%s",
              cpu_features_prefix_,
              features.str().c_str(),
              cpu_features_suffix_);
    }
  }
  fprintf(stream_, "\n");
}

}  // namespace aarch64
}  // namespace vixl
