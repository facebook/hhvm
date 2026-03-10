// Copyright 2020, VIXL authors
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

// Initialisation data for a std::map, from instruction form to the visitor
// function that handles it. This allows reuse of existing visitor functions
// that support groups of instructions, though they may do extra decoding
// no longer needed.
// In the long term, it's expected that each component that uses the decoder
// will want to group instruction handling in the way most appropriate to
// the component's function, so this map initialisation will no longer be
// shared.

#define DEFAULT_FORM_TO_VISITOR_MAP(VISITORCLASS)                              \
  {"abs_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},        \
      {"addpl_r_ri"_h, &VISITORCLASS::VisitSVEStackFrameAdjustment},           \
      {"addvl_r_ri"_h, &VISITORCLASS::VisitSVEStackFrameAdjustment},           \
      {"add_z_p_zz"_h,                                                         \
       &VISITORCLASS::VisitSVEIntAddSubtractVectors_Predicated},               \
      {"add_z_zi"_h, &VISITORCLASS::VisitSVEIntAddSubtractImm_Unpredicated},   \
      {"add_z_zz"_h, &VISITORCLASS::VisitSVEIntArithmeticUnpredicated},        \
      {"adr_z_az_d_s32_scaled"_h, &VISITORCLASS::VisitSVEAddressGeneration},   \
      {"adr_z_az_d_u32_scaled"_h, &VISITORCLASS::VisitSVEAddressGeneration},   \
      {"adr_z_az_sd_same_scaled"_h, &VISITORCLASS::VisitSVEAddressGeneration}, \
      {"ands_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},            \
      {"andv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                   \
      {"and_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},             \
      {"and_z_p_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogical_Predicated},      \
      {"and_z_zi"_h,                                                           \
       &VISITORCLASS::VisitSVEBitwiseLogicalWithImm_Unpredicated},             \
      {"and_z_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogicalUnpredicated},       \
      {"asrd_z_p_zi"_h, &VISITORCLASS::VisitSVEBitwiseShiftByImm_Predicated},  \
      {"asrr_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEBitwiseShiftByVector_Predicated},                \
      {"asr_z_p_zi"_h, &VISITORCLASS::VisitSVEBitwiseShiftByImm_Predicated},   \
      {"asr_z_p_zw"_h,                                                         \
       &VISITORCLASS::VisitSVEBitwiseShiftByWideElements_Predicated},          \
      {"asr_z_p_zz"_h,                                                         \
       &VISITORCLASS::VisitSVEBitwiseShiftByVector_Predicated},                \
      {"asr_z_zi"_h, &VISITORCLASS::VisitSVEBitwiseShiftUnpredicated},         \
      {"asr_z_zw"_h, &VISITORCLASS::VisitSVEBitwiseShiftUnpredicated},         \
      {"bics_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},            \
      {"bic_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},             \
      {"bic_z_p_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogical_Predicated},      \
      {"bic_z_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogicalUnpredicated},       \
      {"brkas_p_p_p_z"_h, &VISITORCLASS::VisitSVEPartitionBreakCondition},     \
      {"brka_p_p_p"_h, &VISITORCLASS::VisitSVEPartitionBreakCondition},        \
      {"brkbs_p_p_p_z"_h, &VISITORCLASS::VisitSVEPartitionBreakCondition},     \
      {"brkb_p_p_p"_h, &VISITORCLASS::VisitSVEPartitionBreakCondition},        \
      {"brkns_p_p_pp"_h,                                                       \
       &VISITORCLASS::VisitSVEPropagateBreakToNextPartition},                  \
      {"brkn_p_p_pp"_h, &VISITORCLASS::VisitSVEPropagateBreakToNextPartition}, \
      {"brkpas_p_p_pp"_h, &VISITORCLASS::VisitSVEPropagateBreak},              \
      {"brkpa_p_p_pp"_h, &VISITORCLASS::VisitSVEPropagateBreak},               \
      {"brkpbs_p_p_pp"_h, &VISITORCLASS::VisitSVEPropagateBreak},              \
      {"brkpb_p_p_pp"_h, &VISITORCLASS::VisitSVEPropagateBreak},               \
      {"clasta_r_p_z"_h,                                                       \
       &VISITORCLASS::VisitSVEConditionallyExtractElementToGeneralRegister},   \
      {"clasta_v_p_z"_h,                                                       \
       &VISITORCLASS::VisitSVEConditionallyExtractElementToSIMDFPScalar},      \
      {"clasta_z_p_zz"_h,                                                      \
       &VISITORCLASS::VisitSVEConditionallyBroadcastElementToVector},          \
      {"clastb_r_p_z"_h,                                                       \
       &VISITORCLASS::VisitSVEConditionallyExtractElementToGeneralRegister},   \
      {"clastb_v_p_z"_h,                                                       \
       &VISITORCLASS::VisitSVEConditionallyExtractElementToSIMDFPScalar},      \
      {"clastb_z_p_zz"_h,                                                      \
       &VISITORCLASS::VisitSVEConditionallyBroadcastElementToVector},          \
      {"cls_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},    \
      {"clz_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},    \
      {"cmpeq_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareSignedImm},          \
      {"cmpeq_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpeq_p_p_zz"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpge_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareSignedImm},          \
      {"cmpge_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpge_p_p_zz"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpgt_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareSignedImm},          \
      {"cmpgt_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpgt_p_p_zz"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmphi_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareUnsignedImm},        \
      {"cmphi_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmphi_p_p_zz"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmphs_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareUnsignedImm},        \
      {"cmphs_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmphs_p_p_zz"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmple_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareSignedImm},          \
      {"cmple_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmplo_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareUnsignedImm},        \
      {"cmplo_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpls_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareUnsignedImm},        \
      {"cmpls_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmplt_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareSignedImm},          \
      {"cmplt_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpne_p_p_zi"_h, &VISITORCLASS::VisitSVEIntCompareSignedImm},          \
      {"cmpne_p_p_zw"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cmpne_p_p_zz"_h, &VISITORCLASS::VisitSVEIntCompareVectors},            \
      {"cnot_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"cntb_r_s"_h, &VISITORCLASS::VisitSVEElementCount},                     \
      {"cntd_r_s"_h, &VISITORCLASS::VisitSVEElementCount},                     \
      {"cnth_r_s"_h, &VISITORCLASS::VisitSVEElementCount},                     \
      {"cntp_r_p_p"_h, &VISITORCLASS::VisitSVEPredicateCount},                 \
      {"cntw_r_s"_h, &VISITORCLASS::VisitSVEElementCount},                     \
      {"cnt_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},    \
      {"compact_z_p_z"_h, &VISITORCLASS::VisitSVECompressActiveElements},      \
      {"cpy_z_o_i"_h, &VISITORCLASS::VisitSVECopyIntImm_Predicated},           \
      {"cpy_z_p_i"_h, &VISITORCLASS::VisitSVECopyIntImm_Predicated},           \
      {"cpy_z_p_r"_h,                                                          \
       &VISITORCLASS::VisitSVECopyGeneralRegisterToVector_Predicated},         \
      {"cpy_z_p_v"_h,                                                          \
       &VISITORCLASS::VisitSVECopySIMDFPScalarRegisterToVector_Predicated},    \
      {"ctermeq_rr"_h, &VISITORCLASS::VisitSVEConditionallyTerminateScalars},  \
      {"ctermne_rr"_h, &VISITORCLASS::VisitSVEConditionallyTerminateScalars},  \
      {"decb_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"decd_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"decd_z_zs"_h, &VISITORCLASS::VisitSVEIncDecVectorByElementCount},      \
      {"dech_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"dech_z_zs"_h, &VISITORCLASS::VisitSVEIncDecVectorByElementCount},      \
      {"decp_r_p_r"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},         \
      {"decp_z_p_z"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},         \
      {"decw_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"decw_z_zs"_h, &VISITORCLASS::VisitSVEIncDecVectorByElementCount},      \
      {"dupm_z_i"_h, &VISITORCLASS::VisitSVEBroadcastBitmaskImm},              \
      {"dup_z_i"_h, &VISITORCLASS::VisitSVEBroadcastIntImm_Unpredicated},      \
      {"dup_z_r"_h, &VISITORCLASS::VisitSVEBroadcastGeneralRegister},          \
      {"dup_z_zi"_h, &VISITORCLASS::VisitSVEBroadcastIndexElement},            \
      {"eors_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},            \
      {"eorv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                   \
      {"eor_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},             \
      {"eor_z_p_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogical_Predicated},      \
      {"eor_z_zi"_h,                                                           \
       &VISITORCLASS::VisitSVEBitwiseLogicalWithImm_Unpredicated},             \
      {"eor_z_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogicalUnpredicated},       \
      {"ext_z_zi_des"_h, &VISITORCLASS::VisitSVEPermuteVectorExtract},         \
      {"fabd_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},       \
      {"fabs_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"facge_p_p_zz"_h, &VISITORCLASS::VisitSVEFPCompareVectors},             \
      {"facgt_p_p_zz"_h, &VISITORCLASS::VisitSVEFPCompareVectors},             \
      {"fadda_v_p_z"_h, &VISITORCLASS::VisitSVEFPAccumulatingReduction},       \
      {"faddv_v_p_z"_h, &VISITORCLASS::VisitSVEFPFastReduction},               \
      {"fadd_z_p_zs"_h,                                                        \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fadd_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},       \
      {"fadd_z_zz"_h, &VISITORCLASS::VisitSVEFPArithmeticUnpredicated},        \
      {"fcadd_z_p_zz"_h, &VISITORCLASS::VisitSVEFPComplexAddition},            \
      {"fcmeq_p_p_z0"_h, &VISITORCLASS::VisitSVEFPCompareWithZero},            \
      {"fcmeq_p_p_zz"_h, &VISITORCLASS::VisitSVEFPCompareVectors},             \
      {"fcmge_p_p_z0"_h, &VISITORCLASS::VisitSVEFPCompareWithZero},            \
      {"fcmge_p_p_zz"_h, &VISITORCLASS::VisitSVEFPCompareVectors},             \
      {"fcmgt_p_p_z0"_h, &VISITORCLASS::VisitSVEFPCompareWithZero},            \
      {"fcmgt_p_p_zz"_h, &VISITORCLASS::VisitSVEFPCompareVectors},             \
      {"fcmla_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPComplexMulAdd},             \
      {"fcmla_z_zzzi_h"_h, &VISITORCLASS::VisitSVEFPComplexMulAddIndex},       \
      {"fcmla_z_zzzi_s"_h, &VISITORCLASS::VisitSVEFPComplexMulAddIndex},       \
      {"fcmle_p_p_z0"_h, &VISITORCLASS::VisitSVEFPCompareWithZero},            \
      {"fcmlt_p_p_z0"_h, &VISITORCLASS::VisitSVEFPCompareWithZero},            \
      {"fcmne_p_p_z0"_h, &VISITORCLASS::VisitSVEFPCompareWithZero},            \
      {"fcmne_p_p_zz"_h, &VISITORCLASS::VisitSVEFPCompareVectors},             \
      {"fcmuo_p_p_zz"_h, &VISITORCLASS::VisitSVEFPCompareVectors},             \
      {"fcpy_z_p_i"_h, &VISITORCLASS::VisitSVECopyFPImm_Predicated},           \
      {"fcvtzs_z_p_z_d2w"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvtzs_z_p_z_d2x"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvtzs_z_p_z_fp162h"_h, &VISITORCLASS::VisitSVEFPConvertToInt},        \
      {"fcvtzs_z_p_z_fp162w"_h, &VISITORCLASS::VisitSVEFPConvertToInt},        \
      {"fcvtzs_z_p_z_fp162x"_h, &VISITORCLASS::VisitSVEFPConvertToInt},        \
      {"fcvtzs_z_p_z_s2w"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvtzs_z_p_z_s2x"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvtzu_z_p_z_d2w"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvtzu_z_p_z_d2x"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvtzu_z_p_z_fp162h"_h, &VISITORCLASS::VisitSVEFPConvertToInt},        \
      {"fcvtzu_z_p_z_fp162w"_h, &VISITORCLASS::VisitSVEFPConvertToInt},        \
      {"fcvtzu_z_p_z_fp162x"_h, &VISITORCLASS::VisitSVEFPConvertToInt},        \
      {"fcvtzu_z_p_z_s2w"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvtzu_z_p_z_s2x"_h, &VISITORCLASS::VisitSVEFPConvertToInt},           \
      {"fcvt_z_p_z_d2h"_h, &VISITORCLASS::VisitSVEFPConvertPrecision},         \
      {"fcvt_z_p_z_d2s"_h, &VISITORCLASS::VisitSVEFPConvertPrecision},         \
      {"fcvt_z_p_z_h2d"_h, &VISITORCLASS::VisitSVEFPConvertPrecision},         \
      {"fcvt_z_p_z_h2s"_h, &VISITORCLASS::VisitSVEFPConvertPrecision},         \
      {"fcvt_z_p_z_s2d"_h, &VISITORCLASS::VisitSVEFPConvertPrecision},         \
      {"fcvt_z_p_z_s2h"_h, &VISITORCLASS::VisitSVEFPConvertPrecision},         \
      {"fdivr_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},      \
      {"fdiv_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},       \
      {"fdup_z_i"_h, &VISITORCLASS::VisitSVEBroadcastFPImm_Unpredicated},      \
      {"fexpa_z_z"_h, &VISITORCLASS::VisitSVEFPExponentialAccelerator},        \
      {"fmad_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                     \
      {"fmaxnmv_v_p_z"_h, &VISITORCLASS::VisitSVEFPFastReduction},             \
      {"fmaxnm_z_p_zs"_h,                                                      \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fmaxnm_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},     \
      {"fmaxv_v_p_z"_h, &VISITORCLASS::VisitSVEFPFastReduction},               \
      {"fmax_z_p_zs"_h,                                                        \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fmax_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},       \
      {"fminnmv_v_p_z"_h, &VISITORCLASS::VisitSVEFPFastReduction},             \
      {"fminnm_z_p_zs"_h,                                                      \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fminnm_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},     \
      {"fminv_v_p_z"_h, &VISITORCLASS::VisitSVEFPFastReduction},               \
      {"fmin_z_p_zs"_h,                                                        \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fmin_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},       \
      {"fmla_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                     \
      {"fmla_z_zzzi_d"_h, &VISITORCLASS::VisitSVEFPMulAddIndex},               \
      {"fmla_z_zzzi_h"_h, &VISITORCLASS::VisitSVEFPMulAddIndex},               \
      {"fmla_z_zzzi_s"_h, &VISITORCLASS::VisitSVEFPMulAddIndex},               \
      {"fmls_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                     \
      {"fmls_z_zzzi_d"_h, &VISITORCLASS::VisitSVEFPMulAddIndex},               \
      {"fmls_z_zzzi_h"_h, &VISITORCLASS::VisitSVEFPMulAddIndex},               \
      {"fmls_z_zzzi_s"_h, &VISITORCLASS::VisitSVEFPMulAddIndex},               \
      {"fmsb_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                     \
      {"fmulx_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},      \
      {"fmul_z_p_zs"_h,                                                        \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fmul_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},       \
      {"fmul_z_zz"_h, &VISITORCLASS::VisitSVEFPArithmeticUnpredicated},        \
      {"fmul_z_zzi_d"_h, &VISITORCLASS::VisitSVEFPMulIndex},                   \
      {"fmul_z_zzi_h"_h, &VISITORCLASS::VisitSVEFPMulIndex},                   \
      {"fmul_z_zzi_s"_h, &VISITORCLASS::VisitSVEFPMulIndex},                   \
      {"fneg_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"fnmad_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                    \
      {"fnmla_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                    \
      {"fnmls_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                    \
      {"fnmsb_z_p_zzz"_h, &VISITORCLASS::VisitSVEFPMulAdd},                    \
      {"frecpe_z_z"_h, &VISITORCLASS::VisitSVEFPUnaryOpUnpredicated},          \
      {"frecps_z_zz"_h, &VISITORCLASS::VisitSVEFPArithmeticUnpredicated},      \
      {"frecpx_z_p_z"_h, &VISITORCLASS::VisitSVEFPUnaryOp},                    \
      {"frinta_z_p_z"_h, &VISITORCLASS::VisitSVEFPRoundToIntegralValue},       \
      {"frinti_z_p_z"_h, &VISITORCLASS::VisitSVEFPRoundToIntegralValue},       \
      {"frintm_z_p_z"_h, &VISITORCLASS::VisitSVEFPRoundToIntegralValue},       \
      {"frintn_z_p_z"_h, &VISITORCLASS::VisitSVEFPRoundToIntegralValue},       \
      {"frintp_z_p_z"_h, &VISITORCLASS::VisitSVEFPRoundToIntegralValue},       \
      {"frintx_z_p_z"_h, &VISITORCLASS::VisitSVEFPRoundToIntegralValue},       \
      {"frintz_z_p_z"_h, &VISITORCLASS::VisitSVEFPRoundToIntegralValue},       \
      {"frsqrte_z_z"_h, &VISITORCLASS::VisitSVEFPUnaryOpUnpredicated},         \
      {"frsqrts_z_zz"_h, &VISITORCLASS::VisitSVEFPArithmeticUnpredicated},     \
      {"fscale_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},     \
      {"fsqrt_z_p_z"_h, &VISITORCLASS::VisitSVEFPUnaryOp},                     \
      {"fsubr_z_p_zs"_h,                                                       \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fsubr_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},      \
      {"fsub_z_p_zs"_h,                                                        \
       &VISITORCLASS::VisitSVEFPArithmeticWithImm_Predicated},                 \
      {"fsub_z_p_zz"_h, &VISITORCLASS::VisitSVEFPArithmetic_Predicated},       \
      {"fsub_z_zz"_h, &VISITORCLASS::VisitSVEFPArithmeticUnpredicated},        \
      {"ftmad_z_zzi"_h, &VISITORCLASS::VisitSVEFPTrigMulAddCoefficient},       \
      {"ftsmul_z_zz"_h, &VISITORCLASS::VisitSVEFPArithmeticUnpredicated},      \
      {"ftssel_z_zz"_h, &VISITORCLASS::VisitSVEFPTrigSelectCoefficient},       \
      {"incb_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"incd_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"incd_z_zs"_h, &VISITORCLASS::VisitSVEIncDecVectorByElementCount},      \
      {"inch_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"inch_z_zs"_h, &VISITORCLASS::VisitSVEIncDecVectorByElementCount},      \
      {"incp_r_p_r"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},         \
      {"incp_z_p_z"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},         \
      {"incw_r_rs"_h, &VISITORCLASS::VisitSVEIncDecRegisterByElementCount},    \
      {"incw_z_zs"_h, &VISITORCLASS::VisitSVEIncDecVectorByElementCount},      \
      {"index_z_ii"_h, &VISITORCLASS::VisitSVEIndexGeneration},                \
      {"index_z_ir"_h, &VISITORCLASS::VisitSVEIndexGeneration},                \
      {"index_z_ri"_h, &VISITORCLASS::VisitSVEIndexGeneration},                \
      {"index_z_rr"_h, &VISITORCLASS::VisitSVEIndexGeneration},                \
      {"insr_z_r"_h, &VISITORCLASS::VisitSVEInsertGeneralRegister},            \
      {"insr_z_v"_h, &VISITORCLASS::VisitSVEInsertSIMDFPScalarRegister},       \
      {"lasta_r_p_z"_h,                                                        \
       &VISITORCLASS::VisitSVEExtractElementToGeneralRegister},                \
      {"lasta_v_p_z"_h,                                                        \
       &VISITORCLASS::VisitSVEExtractElementToSIMDFPScalarRegister},           \
      {"lastb_r_p_z"_h,                                                        \
       &VISITORCLASS::VisitSVEExtractElementToGeneralRegister},                \
      {"lastb_v_p_z"_h,                                                        \
       &VISITORCLASS::VisitSVEExtractElementToSIMDFPScalarRegister},           \
      {"ld1b_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ld1b_z_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ld1b_z_p_bi_u16"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1b_z_p_bi_u32"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1b_z_p_bi_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1b_z_p_bi_u8"_h,                                                     \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1b_z_p_br_u16"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1b_z_p_br_u32"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1b_z_p_br_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1b_z_p_br_u8"_h,                                                     \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1b_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ld1b_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ld1b_z_p_bz_s_x32_unscaled"_h,                                         \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ld1d_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ld1d_z_p_bi_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1d_z_p_br_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1d_z_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ld1d_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ld1d_z_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ld1d_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ld1h_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ld1h_z_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ld1h_z_p_bi_u16"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1h_z_p_bi_u32"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1h_z_p_bi_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1h_z_p_br_u16"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1h_z_p_br_u32"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1h_z_p_br_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1h_z_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ld1h_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ld1h_z_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ld1h_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ld1h_z_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsets},     \
      {"ld1h_z_p_bz_s_x32_unscaled"_h,                                         \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ld1rb_z_p_bi_u16"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rb_z_p_bi_u32"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rb_z_p_bi_u64"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rb_z_p_bi_u8"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},   \
      {"ld1rd_z_p_bi_u64"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rh_z_p_bi_u16"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rh_z_p_bi_u32"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rh_z_p_bi_u64"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rqb_z_p_bi_u8"_h,                                                   \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},           \
      {"ld1rqb_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},        \
      {"ld1rqd_z_p_bi_u64"_h,                                                  \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},           \
      {"ld1rqd_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},        \
      {"ld1rqh_z_p_bi_u16"_h,                                                  \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},           \
      {"ld1rqh_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},        \
      {"ld1rqw_z_p_bi_u32"_h,                                                  \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusImm},           \
      {"ld1rqw_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVELoadAndBroadcastQOWord_ScalarPlusScalar},        \
      {"ld1rsb_z_p_bi_s16"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement}, \
      {"ld1rsb_z_p_bi_s32"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement}, \
      {"ld1rsb_z_p_bi_s64"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement}, \
      {"ld1rsh_z_p_bi_s32"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement}, \
      {"ld1rsh_z_p_bi_s64"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement}, \
      {"ld1rsw_z_p_bi_s64"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement}, \
      {"ld1rw_z_p_bi_u32"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1rw_z_p_bi_u64"_h, &VISITORCLASS::VisitSVELoadAndBroadcastElement},  \
      {"ld1sb_z_p_ai_d"_h,                                                     \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ld1sb_z_p_ai_s"_h,                                                     \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ld1sb_z_p_bi_s16"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1sb_z_p_bi_s32"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1sb_z_p_bi_s64"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1sb_z_p_br_s16"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1sb_z_p_br_s32"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1sb_z_p_br_s64"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1sb_z_p_bz_d_64_unscaled"_h,                                         \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ld1sb_z_p_bz_d_x32_unscaled"_h,                                        \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ld1sb_z_p_bz_s_x32_unscaled"_h,                                        \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ld1sh_z_p_ai_d"_h,                                                     \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ld1sh_z_p_ai_s"_h,                                                     \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ld1sh_z_p_bi_s32"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1sh_z_p_bi_s64"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1sh_z_p_br_s32"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1sh_z_p_br_s64"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1sh_z_p_bz_d_64_scaled"_h,                                           \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ld1sh_z_p_bz_d_64_unscaled"_h,                                         \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ld1sh_z_p_bz_d_x32_scaled"_h,                                          \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ld1sh_z_p_bz_d_x32_unscaled"_h,                                        \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ld1sh_z_p_bz_s_x32_scaled"_h,                                          \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsets},     \
      {"ld1sh_z_p_bz_s_x32_unscaled"_h,                                        \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ld1sw_z_p_ai_d"_h,                                                     \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ld1sw_z_p_bi_s64"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1sw_z_p_br_s64"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1sw_z_p_bz_d_64_scaled"_h,                                           \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ld1sw_z_p_bz_d_64_unscaled"_h,                                         \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ld1sw_z_p_bz_d_x32_scaled"_h,                                          \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ld1sw_z_p_bz_d_x32_unscaled"_h,                                        \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ld1w_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ld1w_z_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ld1w_z_p_bi_u32"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1w_z_p_bi_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusImm},                   \
      {"ld1w_z_p_br_u32"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1w_z_p_br_u64"_h,                                                    \
       &VISITORCLASS::VisitSVEContiguousLoad_ScalarPlusScalar},                \
      {"ld1w_z_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ld1w_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ld1w_z_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ld1w_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ld1w_z_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsets},         \
      {"ld1w_z_p_bz_s_x32_unscaled"_h,                                         \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ld2b_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld2b_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld2d_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld2d_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld2h_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld2h_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld2w_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld2w_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld3b_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld3b_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld3d_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld3d_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld3h_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld3h_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld3w_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld3w_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld4b_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld4b_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld4d_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld4d_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld4h_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld4h_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ld4w_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusImm},           \
      {"ld4w_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVELoadMultipleStructures_ScalarPlusScalar},        \
      {"ldff1b_z_p_ai_d"_h,                                                    \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ldff1b_z_p_ai_s"_h,                                                    \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ldff1b_z_p_br_u16"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1b_z_p_br_u32"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1b_z_p_br_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1b_z_p_br_u8"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1b_z_p_bz_d_64_unscaled"_h,                                        \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ldff1b_z_p_bz_d_x32_unscaled"_h,                                       \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ldff1b_z_p_bz_s_x32_unscaled"_h,                                       \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ldff1d_z_p_ai_d"_h,                                                    \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ldff1d_z_p_br_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1d_z_p_bz_d_64_scaled"_h,                                          \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ldff1d_z_p_bz_d_64_unscaled"_h,                                        \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ldff1d_z_p_bz_d_x32_scaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ldff1d_z_p_bz_d_x32_unscaled"_h,                                       \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ldff1h_z_p_ai_d"_h,                                                    \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ldff1h_z_p_ai_s"_h,                                                    \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ldff1h_z_p_br_u16"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1h_z_p_br_u32"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1h_z_p_br_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1h_z_p_bz_d_64_scaled"_h,                                          \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ldff1h_z_p_bz_d_64_unscaled"_h,                                        \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ldff1h_z_p_bz_d_x32_scaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ldff1h_z_p_bz_d_x32_unscaled"_h,                                       \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ldff1h_z_p_bz_s_x32_scaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsets},     \
      {"ldff1h_z_p_bz_s_x32_unscaled"_h,                                       \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ldff1sb_z_p_ai_d"_h,                                                   \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ldff1sb_z_p_ai_s"_h,                                                   \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ldff1sb_z_p_br_s16"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1sb_z_p_br_s32"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1sb_z_p_br_s64"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1sb_z_p_bz_d_64_unscaled"_h,                                       \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ldff1sb_z_p_bz_d_x32_unscaled"_h,                                      \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ldff1sb_z_p_bz_s_x32_unscaled"_h,                                      \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ldff1sh_z_p_ai_d"_h,                                                   \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ldff1sh_z_p_ai_s"_h,                                                   \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ldff1sh_z_p_br_s32"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1sh_z_p_br_s64"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1sh_z_p_bz_d_64_scaled"_h,                                         \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ldff1sh_z_p_bz_d_64_unscaled"_h,                                       \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ldff1sh_z_p_bz_d_x32_scaled"_h,                                        \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ldff1sh_z_p_bz_d_x32_unscaled"_h,                                      \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ldff1sh_z_p_bz_s_x32_scaled"_h,                                        \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherLoadHalfwords_ScalarPlus32BitScaledOffsets},     \
      {"ldff1sh_z_p_bz_s_x32_unscaled"_h,                                      \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ldff1sw_z_p_ai_d"_h,                                                   \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ldff1sw_z_p_br_s64"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1sw_z_p_bz_d_64_scaled"_h,                                         \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ldff1sw_z_p_bz_d_64_unscaled"_h,                                       \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ldff1sw_z_p_bz_d_x32_scaled"_h,                                        \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ldff1sw_z_p_bz_d_x32_unscaled"_h,                                      \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ldff1w_z_p_ai_d"_h,                                                    \
       &VISITORCLASS::VisitSVE64BitGatherLoad_VectorPlusImm},                  \
      {"ldff1w_z_p_ai_s"_h,                                                    \
       &VISITORCLASS::VisitSVE32BitGatherLoad_VectorPlusImm},                  \
      {"ldff1w_z_p_br_u32"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1w_z_p_br_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousFirstFaultLoad_ScalarPlusScalar},      \
      {"ldff1w_z_p_bz_d_64_scaled"_h,                                          \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitScaledOffsets},   \
      {"ldff1w_z_p_bz_d_64_unscaled"_h,                                        \
       &VISITORCLASS::VisitSVE64BitGatherLoad_ScalarPlus64BitUnscaledOffsets}, \
      {"ldff1w_z_p_bz_d_x32_scaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlus32BitUnpackedScaledOffsets},      \
      {"ldff1w_z_p_bz_d_x32_unscaled"_h,                                       \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherLoad_ScalarPlusUnpacked32BitUnscaledOffsets},    \
      {"ldff1w_z_p_bz_s_x32_scaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherLoadWords_ScalarPlus32BitScaledOffsets},         \
      {"ldff1w_z_p_bz_s_x32_unscaled"_h,                                       \
       &VISITORCLASS::VisitSVE32BitGatherLoad_ScalarPlus32BitUnscaledOffsets}, \
      {"ldnf1b_z_p_bi_u16"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1b_z_p_bi_u32"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1b_z_p_bi_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1b_z_p_bi_u8"_h,                                                   \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1d_z_p_bi_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1h_z_p_bi_u16"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1h_z_p_bi_u32"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1h_z_p_bi_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1sb_z_p_bi_s16"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1sb_z_p_bi_s32"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1sb_z_p_bi_s64"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1sh_z_p_bi_s32"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1sh_z_p_bi_s64"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1sw_z_p_bi_s64"_h,                                                 \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1w_z_p_bi_u32"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnf1w_z_p_bi_u64"_h,                                                  \
       &VISITORCLASS::VisitSVEContiguousNonFaultLoad_ScalarPlusImm},           \
      {"ldnt1b_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusImm},        \
      {"ldnt1b_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusScalar},     \
      {"ldnt1d_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusImm},        \
      {"ldnt1d_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusScalar},     \
      {"ldnt1h_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusImm},        \
      {"ldnt1h_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusScalar},     \
      {"ldnt1w_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusImm},        \
      {"ldnt1w_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalLoad_ScalarPlusScalar},     \
      {"ldr_p_bi"_h, &VISITORCLASS::VisitSVELoadPredicateRegister},            \
      {"ldr_z_bi"_h, &VISITORCLASS::VisitSVELoadVectorRegister},               \
      {"lslr_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEBitwiseShiftByVector_Predicated},                \
      {"lsl_z_p_zi"_h, &VISITORCLASS::VisitSVEBitwiseShiftByImm_Predicated},   \
      {"lsl_z_p_zw"_h,                                                         \
       &VISITORCLASS::VisitSVEBitwiseShiftByWideElements_Predicated},          \
      {"lsl_z_p_zz"_h,                                                         \
       &VISITORCLASS::VisitSVEBitwiseShiftByVector_Predicated},                \
      {"lsl_z_zi"_h, &VISITORCLASS::VisitSVEBitwiseShiftUnpredicated},         \
      {"lsl_z_zw"_h, &VISITORCLASS::VisitSVEBitwiseShiftUnpredicated},         \
      {"lsrr_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEBitwiseShiftByVector_Predicated},                \
      {"lsr_z_p_zi"_h, &VISITORCLASS::VisitSVEBitwiseShiftByImm_Predicated},   \
      {"lsr_z_p_zw"_h,                                                         \
       &VISITORCLASS::VisitSVEBitwiseShiftByWideElements_Predicated},          \
      {"lsr_z_p_zz"_h,                                                         \
       &VISITORCLASS::VisitSVEBitwiseShiftByVector_Predicated},                \
      {"lsr_z_zi"_h, &VISITORCLASS::VisitSVEBitwiseShiftUnpredicated},         \
      {"lsr_z_zw"_h, &VISITORCLASS::VisitSVEBitwiseShiftUnpredicated},         \
      {"mad_z_p_zzz"_h, &VISITORCLASS::VisitSVEIntMulAddPredicated},           \
      {"mla_z_p_zzz"_h, &VISITORCLASS::VisitSVEIntMulAddPredicated},           \
      {"mls_z_p_zzz"_h, &VISITORCLASS::VisitSVEIntMulAddPredicated},           \
      {"movprfx_z_p_z"_h, &VISITORCLASS::VisitSVEMovprfx},                     \
      {"movprfx_z_z"_h,                                                        \
       &VISITORCLASS::VisitSVEConstructivePrefix_Unpredicated},                \
      {"msb_z_p_zzz"_h, &VISITORCLASS::VisitSVEIntMulAddPredicated},           \
      {"mul_z_p_zz"_h, &VISITORCLASS::VisitSVEIntMulVectors_Predicated},       \
      {"mul_z_zi"_h, &VISITORCLASS::VisitSVEIntMulImm_Unpredicated},           \
      {"nands_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},           \
      {"nand_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},            \
      {"neg_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},    \
      {"nors_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},            \
      {"nor_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},             \
      {"not_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},    \
      {"orns_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},            \
      {"orn_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},             \
      {"orrs_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},            \
      {"orr_p_p_pp_z"_h, &VISITORCLASS::VisitSVEPredicateLogical},             \
      {"orr_z_p_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogical_Predicated},      \
      {"orr_z_zi"_h,                                                           \
       &VISITORCLASS::VisitSVEBitwiseLogicalWithImm_Unpredicated},             \
      {"orr_z_zz"_h, &VISITORCLASS::VisitSVEBitwiseLogicalUnpredicated},       \
      {"orv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                    \
      {"pfalse_p"_h, &VISITORCLASS::VisitSVEPredicateZero},                    \
      {"pfirst_p_p_p"_h, &VISITORCLASS::VisitSVEPredicateFirstActive},         \
      {"pnext_p_p_p"_h, &VISITORCLASS::VisitSVEPredicateNextActive},           \
      {"prfb_i_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherPrefetch_VectorPlusImm},              \
      {"prfb_i_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitGatherPrefetch_VectorPlusImm},              \
      {"prfb_i_p_bi_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusImm},               \
      {"prfb_i_p_br_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusScalar},            \
      {"prfb_i_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsets},          \
      {"prfb_i_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsets},  \
      {"prfb_i_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsets},          \
      {"prfd_i_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherPrefetch_VectorPlusImm},              \
      {"prfd_i_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitGatherPrefetch_VectorPlusImm},              \
      {"prfd_i_p_bi_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusImm},               \
      {"prfd_i_p_br_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusScalar},            \
      {"prfd_i_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsets},          \
      {"prfd_i_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsets},  \
      {"prfd_i_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsets},          \
      {"prfh_i_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherPrefetch_VectorPlusImm},              \
      {"prfh_i_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitGatherPrefetch_VectorPlusImm},              \
      {"prfh_i_p_bi_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusImm},               \
      {"prfh_i_p_br_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusScalar},            \
      {"prfh_i_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsets},          \
      {"prfh_i_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsets},  \
      {"prfh_i_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsets},          \
      {"prfw_i_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitGatherPrefetch_VectorPlusImm},              \
      {"prfw_i_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitGatherPrefetch_VectorPlusImm},              \
      {"prfw_i_p_bi_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusImm},               \
      {"prfw_i_p_br_s"_h,                                                      \
       &VISITORCLASS::VisitSVEContiguousPrefetch_ScalarPlusScalar},            \
      {"prfw_i_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlus64BitScaledOffsets},          \
      {"prfw_i_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitGatherPrefetch_ScalarPlusUnpacked32BitScaledOffsets},  \
      {"prfw_i_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE32BitGatherPrefetch_ScalarPlus32BitScaledOffsets},          \
      {"ptest_p_p"_h, &VISITORCLASS::VisitSVEPredicateTest},                   \
      {"ptrues_p_s"_h, &VISITORCLASS::VisitSVEPredicateInitialize},            \
      {"ptrue_p_s"_h, &VISITORCLASS::VisitSVEPredicateInitialize},             \
      {"punpkhi_p_p"_h, &VISITORCLASS::VisitSVEUnpackPredicateElements},       \
      {"punpklo_p_p"_h, &VISITORCLASS::VisitSVEUnpackPredicateElements},       \
      {"rbit_z_p_z"_h, &VISITORCLASS::VisitSVEReverseWithinElements},          \
      {"rdffrs_p_p_f"_h,                                                       \
       &VISITORCLASS::VisitSVEPredicateReadFromFFR_Predicated},                \
      {"rdffr_p_f"_h,                                                          \
       &VISITORCLASS::VisitSVEPredicateReadFromFFR_Unpredicated},              \
      {"rdffr_p_p_f"_h,                                                        \
       &VISITORCLASS::VisitSVEPredicateReadFromFFR_Predicated},                \
      {"rdvl_r_i"_h, &VISITORCLASS::VisitSVEStackFrameSize},                   \
      {"revb_z_z"_h, &VISITORCLASS::VisitSVEReverseWithinElements},            \
      {"revh_z_z"_h, &VISITORCLASS::VisitSVEReverseWithinElements},            \
      {"revw_z_z"_h, &VISITORCLASS::VisitSVEReverseWithinElements},            \
      {"rev_p_p"_h, &VISITORCLASS::VisitSVEReversePredicateElements},          \
      {"rev_z_z"_h, &VISITORCLASS::VisitSVEReverseVectorElements},             \
      {"sabd_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEIntMinMaxDifference_Predicated},                 \
      {"saddv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                  \
      {"scvtf_z_p_z_h2fp16"_h, &VISITORCLASS::VisitSVEIntConvertToFP},         \
      {"scvtf_z_p_z_w2d"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"scvtf_z_p_z_w2fp16"_h, &VISITORCLASS::VisitSVEIntConvertToFP},         \
      {"scvtf_z_p_z_w2s"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"scvtf_z_p_z_x2d"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"scvtf_z_p_z_x2fp16"_h, &VISITORCLASS::VisitSVEIntConvertToFP},         \
      {"scvtf_z_p_z_x2s"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"sdivr_z_p_zz"_h, &VISITORCLASS::VisitSVEIntDivideVectors_Predicated},  \
      {"sdiv_z_p_zz"_h, &VISITORCLASS::VisitSVEIntDivideVectors_Predicated},   \
      {"sdot_z_zzz"_h, &VISITORCLASS::VisitSVEIntMulAddUnpredicated},          \
      {"sdot_z_zzzi_d"_h, &VISITORCLASS::VisitSVEMulIndex},                    \
      {"sdot_z_zzzi_s"_h, &VISITORCLASS::VisitSVEMulIndex},                    \
      {"sel_p_p_pp"_h, &VISITORCLASS::VisitSVEPredicateLogical},               \
      {"sel_z_p_zz"_h, &VISITORCLASS::VisitSVEVectorSelect},                   \
      {"setffr_f"_h, &VISITORCLASS::VisitSVEFFRInitialise},                    \
      {"smaxv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                  \
      {"smax_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEIntMinMaxDifference_Predicated},                 \
      {"smax_z_zi"_h, &VISITORCLASS::VisitSVEIntMinMaxImm_Unpredicated},       \
      {"sminv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                  \
      {"smin_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEIntMinMaxDifference_Predicated},                 \
      {"smin_z_zi"_h, &VISITORCLASS::VisitSVEIntMinMaxImm_Unpredicated},       \
      {"smulh_z_p_zz"_h, &VISITORCLASS::VisitSVEIntMulVectors_Predicated},     \
      {"splice_z_p_zz_des"_h, &VISITORCLASS::VisitSVEVectorSplice},            \
      {"sqadd_z_zi"_h, &VISITORCLASS::VisitSVEIntAddSubtractImm_Unpredicated}, \
      {"sqadd_z_zz"_h, &VISITORCLASS::VisitSVEIntArithmeticUnpredicated},      \
      {"sqdecb_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdecb_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdecd_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdecd_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdecd_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"sqdech_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdech_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdech_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"sqdecp_r_p_r_sx"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},    \
      {"sqdecp_r_p_r_x"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},     \
      {"sqdecp_z_p_z"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},       \
      {"sqdecw_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdecw_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqdecw_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"sqincb_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqincb_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqincd_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqincd_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqincd_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"sqinch_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqinch_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqinch_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"sqincp_r_p_r_sx"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},    \
      {"sqincp_r_p_r_x"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},     \
      {"sqincp_z_p_z"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},       \
      {"sqincw_r_rs_sx"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqincw_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"sqincw_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"sqsub_z_zi"_h, &VISITORCLASS::VisitSVEIntAddSubtractImm_Unpredicated}, \
      {"sqsub_z_zz"_h, &VISITORCLASS::VisitSVEIntArithmeticUnpredicated},      \
      {"st1b_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitScatterStore_VectorPlusImm},                \
      {"st1b_z_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitScatterStore_VectorPlusImm},                \
      {"st1b_z_p_bi"_h, &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusImm}, \
      {"st1b_z_p_br"_h,                                                        \
       &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusScalar},               \
      {"st1b_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlus64BitUnscaledOffsets},          \
      {"st1b_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsets},  \
      {"st1b_z_p_bz_s_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE32BitScatterStore_ScalarPlus32BitUnscaledOffsets},          \
      {"st1d_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitScatterStore_VectorPlusImm},                \
      {"st1d_z_p_bi"_h, &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusImm}, \
      {"st1d_z_p_br"_h,                                                        \
       &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusScalar},               \
      {"st1d_z_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::VisitSVE64BitScatterStore_ScalarPlus64BitScaledOffsets}, \
      {"st1d_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlus64BitUnscaledOffsets},          \
      {"st1d_z_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsets},    \
      {"st1d_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsets},  \
      {"st1h_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitScatterStore_VectorPlusImm},                \
      {"st1h_z_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitScatterStore_VectorPlusImm},                \
      {"st1h_z_p_bi"_h, &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusImm}, \
      {"st1h_z_p_br"_h,                                                        \
       &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusScalar},               \
      {"st1h_z_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::VisitSVE64BitScatterStore_ScalarPlus64BitScaledOffsets}, \
      {"st1h_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlus64BitUnscaledOffsets},          \
      {"st1h_z_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsets},    \
      {"st1h_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsets},  \
      {"st1h_z_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::VisitSVE32BitScatterStore_ScalarPlus32BitScaledOffsets}, \
      {"st1h_z_p_bz_s_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE32BitScatterStore_ScalarPlus32BitUnscaledOffsets},          \
      {"st1w_z_p_ai_d"_h,                                                      \
       &VISITORCLASS::VisitSVE64BitScatterStore_VectorPlusImm},                \
      {"st1w_z_p_ai_s"_h,                                                      \
       &VISITORCLASS::VisitSVE32BitScatterStore_VectorPlusImm},                \
      {"st1w_z_p_bi"_h, &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusImm}, \
      {"st1w_z_p_br"_h,                                                        \
       &VISITORCLASS::VisitSVEContiguousStore_ScalarPlusScalar},               \
      {"st1w_z_p_bz_d_64_scaled"_h,                                            \
       &VISITORCLASS::VisitSVE64BitScatterStore_ScalarPlus64BitScaledOffsets}, \
      {"st1w_z_p_bz_d_64_unscaled"_h,                                          \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlus64BitUnscaledOffsets},          \
      {"st1w_z_p_bz_d_x32_scaled"_h,                                           \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitScaledOffsets},    \
      {"st1w_z_p_bz_d_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE64BitScatterStore_ScalarPlusUnpacked32BitUnscaledOffsets},  \
      {"st1w_z_p_bz_s_x32_scaled"_h,                                           \
       &VISITORCLASS::VisitSVE32BitScatterStore_ScalarPlus32BitScaledOffsets}, \
      {"st1w_z_p_bz_s_x32_unscaled"_h,                                         \
       &VISITORCLASS::                                                         \
           VisitSVE32BitScatterStore_ScalarPlus32BitUnscaledOffsets},          \
      {"st2b_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st2b_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st2d_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st2d_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st2h_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st2h_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st2w_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st2w_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st3b_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st3b_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st3d_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st3d_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st3h_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st3h_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st3w_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st3w_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st4b_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st4b_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st4d_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st4d_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st4h_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st4h_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"st4w_z_p_bi_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusImm},          \
      {"st4w_z_p_br_contiguous"_h,                                             \
       &VISITORCLASS::VisitSVEStoreMultipleStructures_ScalarPlusScalar},       \
      {"stnt1b_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusImm},       \
      {"stnt1b_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusScalar},    \
      {"stnt1d_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusImm},       \
      {"stnt1d_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusScalar},    \
      {"stnt1h_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusImm},       \
      {"stnt1h_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusScalar},    \
      {"stnt1w_z_p_bi_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusImm},       \
      {"stnt1w_z_p_br_contiguous"_h,                                           \
       &VISITORCLASS::VisitSVEContiguousNonTemporalStore_ScalarPlusScalar},    \
      {"str_p_bi"_h, &VISITORCLASS::VisitSVEStorePredicateRegister},           \
      {"str_z_bi"_h, &VISITORCLASS::VisitSVEStoreVectorRegister},              \
      {"subr_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEIntAddSubtractVectors_Predicated},               \
      {"subr_z_zi"_h, &VISITORCLASS::VisitSVEIntAddSubtractImm_Unpredicated},  \
      {"sub_z_p_zz"_h,                                                         \
       &VISITORCLASS::VisitSVEIntAddSubtractVectors_Predicated},               \
      {"sub_z_zi"_h, &VISITORCLASS::VisitSVEIntAddSubtractImm_Unpredicated},   \
      {"sub_z_zz"_h, &VISITORCLASS::VisitSVEIntArithmeticUnpredicated},        \
      {"sunpkhi_z_z"_h, &VISITORCLASS::VisitSVEUnpackVectorElements},          \
      {"sunpklo_z_z"_h, &VISITORCLASS::VisitSVEUnpackVectorElements},          \
      {"sxtb_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"sxth_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"sxtw_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"tbl_z_zz_1"_h, &VISITORCLASS::VisitSVETableLookup},                    \
      {"trn1_p_pp"_h, &VISITORCLASS::VisitSVEPermutePredicateElements},        \
      {"trn1_z_zz"_h, &VISITORCLASS::VisitSVEPermuteVectorInterleaving},       \
      {"trn2_p_pp"_h, &VISITORCLASS::VisitSVEPermutePredicateElements},        \
      {"trn2_z_zz"_h, &VISITORCLASS::VisitSVEPermuteVectorInterleaving},       \
      {"uabd_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEIntMinMaxDifference_Predicated},                 \
      {"uaddv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                  \
      {"ucvtf_z_p_z_h2fp16"_h, &VISITORCLASS::VisitSVEIntConvertToFP},         \
      {"ucvtf_z_p_z_w2d"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"ucvtf_z_p_z_w2fp16"_h, &VISITORCLASS::VisitSVEIntConvertToFP},         \
      {"ucvtf_z_p_z_w2s"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"ucvtf_z_p_z_x2d"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"ucvtf_z_p_z_x2fp16"_h, &VISITORCLASS::VisitSVEIntConvertToFP},         \
      {"ucvtf_z_p_z_x2s"_h, &VISITORCLASS::VisitSVEIntConvertToFP},            \
      {"udf_only_perm_undef"_h, &VISITORCLASS::VisitReserved},                 \
      {"udivr_z_p_zz"_h, &VISITORCLASS::VisitSVEIntDivideVectors_Predicated},  \
      {"udiv_z_p_zz"_h, &VISITORCLASS::VisitSVEIntDivideVectors_Predicated},   \
      {"udot_z_zzz"_h, &VISITORCLASS::VisitSVEIntMulAddUnpredicated},          \
      {"udot_z_zzzi_d"_h, &VISITORCLASS::VisitSVEMulIndex},                    \
      {"udot_z_zzzi_s"_h, &VISITORCLASS::VisitSVEMulIndex},                    \
      {"umaxv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                  \
      {"umax_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEIntMinMaxDifference_Predicated},                 \
      {"umax_z_zi"_h, &VISITORCLASS::VisitSVEIntMinMaxImm_Unpredicated},       \
      {"uminv_r_p_z"_h, &VISITORCLASS::VisitSVEIntReduction},                  \
      {"umin_z_p_zz"_h,                                                        \
       &VISITORCLASS::VisitSVEIntMinMaxDifference_Predicated},                 \
      {"umin_z_zi"_h, &VISITORCLASS::VisitSVEIntMinMaxImm_Unpredicated},       \
      {"umulh_z_p_zz"_h, &VISITORCLASS::VisitSVEIntMulVectors_Predicated},     \
      {"uqadd_z_zi"_h, &VISITORCLASS::VisitSVEIntAddSubtractImm_Unpredicated}, \
      {"uqadd_z_zz"_h, &VISITORCLASS::VisitSVEIntArithmeticUnpredicated},      \
      {"uqdecb_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdecb_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdecd_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdecd_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdecd_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"uqdech_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdech_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdech_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"uqdecp_r_p_r_uw"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},    \
      {"uqdecp_r_p_r_x"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},     \
      {"uqdecp_z_p_z"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},       \
      {"uqdecw_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdecw_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqdecw_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"uqincb_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqincb_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqincd_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqincd_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqincd_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"uqinch_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqinch_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqinch_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"uqincp_r_p_r_uw"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},    \
      {"uqincp_r_p_r_x"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},     \
      {"uqincp_z_p_z"_h, &VISITORCLASS::VisitSVEIncDecByPredicateCount},       \
      {"uqincw_r_rs_uw"_h,                                                     \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqincw_r_rs_x"_h,                                                      \
       &VISITORCLASS::VisitSVESaturatingIncDecRegisterByElementCount},         \
      {"uqincw_z_zs"_h,                                                        \
       &VISITORCLASS::VisitSVESaturatingIncDecVectorByElementCount},           \
      {"uqsub_z_zi"_h, &VISITORCLASS::VisitSVEIntAddSubtractImm_Unpredicated}, \
      {"uqsub_z_zz"_h, &VISITORCLASS::VisitSVEIntArithmeticUnpredicated},      \
      {"uunpkhi_z_z"_h, &VISITORCLASS::VisitSVEUnpackVectorElements},          \
      {"uunpklo_z_z"_h, &VISITORCLASS::VisitSVEUnpackVectorElements},          \
      {"uxtb_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"uxth_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"uxtw_z_p_z"_h, &VISITORCLASS::VisitSVEIntUnaryArithmeticPredicated},   \
      {"uzp1_p_pp"_h, &VISITORCLASS::VisitSVEPermutePredicateElements},        \
      {"uzp1_z_zz"_h, &VISITORCLASS::VisitSVEPermuteVectorInterleaving},       \
      {"uzp2_p_pp"_h, &VISITORCLASS::VisitSVEPermutePredicateElements},        \
      {"uzp2_z_zz"_h, &VISITORCLASS::VisitSVEPermuteVectorInterleaving},       \
      {"whilele_p_p_rr"_h,                                                     \
       &VISITORCLASS::VisitSVEIntCompareScalarCountAndLimit},                  \
      {"whilelo_p_p_rr"_h,                                                     \
       &VISITORCLASS::VisitSVEIntCompareScalarCountAndLimit},                  \
      {"whilels_p_p_rr"_h,                                                     \
       &VISITORCLASS::VisitSVEIntCompareScalarCountAndLimit},                  \
      {"whilelt_p_p_rr"_h,                                                     \
       &VISITORCLASS::VisitSVEIntCompareScalarCountAndLimit},                  \
      {"wrffr_f_p"_h, &VISITORCLASS::VisitSVEFFRWriteFromPredicate},           \
      {"zip1_p_pp"_h, &VISITORCLASS::VisitSVEPermutePredicateElements},        \
      {"zip1_z_zz"_h, &VISITORCLASS::VisitSVEPermuteVectorInterleaving},       \
      {"zip2_p_pp"_h, &VISITORCLASS::VisitSVEPermutePredicateElements},        \
      {"zip2_z_zz"_h, &VISITORCLASS::VisitSVEPermuteVectorInterleaving},       \
      {"adds_32s_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},           \
      {"adds_64s_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},           \
      {"add_32_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},             \
      {"add_64_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},             \
      {"subs_32s_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},           \
      {"subs_64s_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},           \
      {"sub_32_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},             \
      {"sub_64_addsub_ext"_h, &VISITORCLASS::VisitAddSubExtended},             \
      {"adds_32s_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},          \
      {"adds_64s_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},          \
      {"add_32_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},            \
      {"add_64_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},            \
      {"subs_32s_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},          \
      {"subs_64s_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},          \
      {"sub_32_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},            \
      {"sub_64_addsub_imm"_h, &VISITORCLASS::VisitAddSubImmediate},            \
      {"adds_32_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},           \
      {"adds_64_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},           \
      {"add_32_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},            \
      {"add_64_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},            \
      {"subs_32_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},           \
      {"subs_64_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},           \
      {"sub_32_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},            \
      {"sub_64_addsub_shift"_h, &VISITORCLASS::VisitAddSubShifted},            \
      {"adcs_32_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},         \
      {"adcs_64_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},         \
      {"adc_32_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},          \
      {"adc_64_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},          \
      {"sbcs_32_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},         \
      {"sbcs_64_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},         \
      {"sbc_32_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},          \
      {"sbc_64_addsub_carry"_h, &VISITORCLASS::VisitAddSubWithCarry},          \
      {"ldaddab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldaddah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldaddalb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldaddalh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldaddal_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldaddal_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldadda_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldadda_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldaddb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldaddh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldaddlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldaddlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldaddl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldaddl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldadd_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldadd_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldaprb_32l_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldaprh_32l_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldapr_32l_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldapr_64l_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldclrab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldclrah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldclralb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldclralh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldclral_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldclral_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldclra_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldclra_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldclrb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldclrh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldclrlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldclrlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldclrl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldclrl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldclr_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldclr_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldeorab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldeorah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldeoralb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldeoralh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldeoral_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldeoral_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldeora_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldeora_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldeorb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldeorh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldeorlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldeorlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldeorl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldeorl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldeor_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldeor_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldsetab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsetah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsetalb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsetalh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsetal_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsetal_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldseta_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldseta_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldsetb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldseth_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldsetlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsetlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsetl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldsetl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldset_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldset_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"ldsmaxab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsmaxah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsmaxalb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"ldsmaxalh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"ldsmaxal_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsmaxal_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsmaxa_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmaxa_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmaxb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmaxh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmaxlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsmaxlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsmaxl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmaxl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmax_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldsmax_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldsminab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsminah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsminalb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"ldsminalh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"ldsminal_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsminal_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsmina_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmina_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsminb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsminh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsminlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsminlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldsminl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsminl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldsmin_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldsmin_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldumaxab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldumaxah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldumaxalb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"ldumaxalh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"ldumaxal_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldumaxal_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldumaxa_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumaxa_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumaxb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumaxh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumaxlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldumaxlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldumaxl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumaxl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumax_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldumax_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"lduminab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"lduminah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"lduminalb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"lduminalh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},              \
      {"lduminal_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"lduminal_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"ldumina_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumina_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"lduminb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"lduminh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"lduminlb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"lduminlh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},               \
      {"lduminl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"lduminl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                \
      {"ldumin_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"ldumin_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"swpab_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"swpah_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"swpalb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"swpalh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                 \
      {"swpal_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"swpal_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"swpa_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                   \
      {"swpa_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                   \
      {"swpb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                   \
      {"swph_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                   \
      {"swplb_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"swplh_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                  \
      {"swpl_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                   \
      {"swpl_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                   \
      {"swp_32_memop"_h, &VISITORCLASS::VisitAtomicMemory},                    \
      {"swp_64_memop"_h, &VISITORCLASS::VisitAtomicMemory},                    \
      {"bfm_32m_bitfield"_h, &VISITORCLASS::VisitBitfield},                    \
      {"bfm_64m_bitfield"_h, &VISITORCLASS::VisitBitfield},                    \
      {"sbfm_32m_bitfield"_h, &VISITORCLASS::VisitBitfield},                   \
      {"sbfm_64m_bitfield"_h, &VISITORCLASS::VisitBitfield},                   \
      {"ubfm_32m_bitfield"_h, &VISITORCLASS::VisitBitfield},                   \
      {"ubfm_64m_bitfield"_h, &VISITORCLASS::VisitBitfield},                   \
      {"cbnz_32_compbranch"_h, &VISITORCLASS::VisitCompareBranch},             \
      {"cbnz_64_compbranch"_h, &VISITORCLASS::VisitCompareBranch},             \
      {"cbz_32_compbranch"_h, &VISITORCLASS::VisitCompareBranch},              \
      {"cbz_64_compbranch"_h, &VISITORCLASS::VisitCompareBranch},              \
      {"b_only_condbranch"_h, &VISITORCLASS::VisitConditionalBranch},          \
      {"ccmn_32_condcmp_imm"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareImmediate},                       \
      {"ccmn_64_condcmp_imm"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareImmediate},                       \
      {"ccmp_32_condcmp_imm"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareImmediate},                       \
      {"ccmp_64_condcmp_imm"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareImmediate},                       \
      {"ccmn_32_condcmp_reg"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareRegister},                        \
      {"ccmn_64_condcmp_reg"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareRegister},                        \
      {"ccmp_32_condcmp_reg"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareRegister},                        \
      {"ccmp_64_condcmp_reg"_h,                                                \
       &VISITORCLASS::VisitConditionalCompareRegister},                        \
      {"csel_32_condsel"_h, &VISITORCLASS::VisitConditionalSelect},            \
      {"csel_64_condsel"_h, &VISITORCLASS::VisitConditionalSelect},            \
      {"csinc_32_condsel"_h, &VISITORCLASS::VisitConditionalSelect},           \
      {"csinc_64_condsel"_h, &VISITORCLASS::VisitConditionalSelect},           \
      {"csinv_32_condsel"_h, &VISITORCLASS::VisitConditionalSelect},           \
      {"csinv_64_condsel"_h, &VISITORCLASS::VisitConditionalSelect},           \
      {"csneg_32_condsel"_h, &VISITORCLASS::VisitConditionalSelect},           \
      {"csneg_64_condsel"_h, &VISITORCLASS::VisitConditionalSelect},           \
      {"sha1h_ss_cryptosha2"_h, &VISITORCLASS::VisitCrypto2RegSHA},            \
      {"sha1su1_vv_cryptosha2"_h, &VISITORCLASS::VisitCrypto2RegSHA},          \
      {"sha256su0_vv_cryptosha2"_h, &VISITORCLASS::VisitCrypto2RegSHA},        \
      {"sha1c_qsv_cryptosha3"_h, &VISITORCLASS::VisitCrypto3RegSHA},           \
      {"sha1m_qsv_cryptosha3"_h, &VISITORCLASS::VisitCrypto3RegSHA},           \
      {"sha1p_qsv_cryptosha3"_h, &VISITORCLASS::VisitCrypto3RegSHA},           \
      {"sha1su0_vvv_cryptosha3"_h, &VISITORCLASS::VisitCrypto3RegSHA},         \
      {"sha256h2_qqv_cryptosha3"_h, &VISITORCLASS::VisitCrypto3RegSHA},        \
      {"sha256h_qqv_cryptosha3"_h, &VISITORCLASS::VisitCrypto3RegSHA},         \
      {"sha256su1_vvv_cryptosha3"_h, &VISITORCLASS::VisitCrypto3RegSHA},       \
      {"aesd_b_cryptoaes"_h, &VISITORCLASS::VisitCryptoAES},                   \
      {"aese_b_cryptoaes"_h, &VISITORCLASS::VisitCryptoAES},                   \
      {"aesimc_b_cryptoaes"_h, &VISITORCLASS::VisitCryptoAES},                 \
      {"aesmc_b_cryptoaes"_h, &VISITORCLASS::VisitCryptoAES},                  \
      {"autda_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"autdb_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"autdza_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"autdzb_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"autia_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"autib_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"autiza_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"autizb_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"cls_32_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},         \
      {"cls_64_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},         \
      {"clz_32_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},         \
      {"clz_64_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},         \
      {"pacda_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"pacdb_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"pacdza_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"pacdzb_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"pacia_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"pacib_64p_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"paciza_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"pacizb_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},     \
      {"rbit_32_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},        \
      {"rbit_64_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},        \
      {"rev16_32_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},       \
      {"rev16_64_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},       \
      {"rev32_64_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},       \
      {"rev_32_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},         \
      {"rev_64_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},         \
      {"xpacd_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"xpaci_64z_dp_1src"_h, &VISITORCLASS::VisitDataProcessing1Source},      \
      {"asrv_32_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"asrv_64_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"crc32b_32c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},     \
      {"crc32cb_32c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},    \
      {"crc32ch_32c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},    \
      {"crc32cw_32c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},    \
      {"crc32cx_64c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},    \
      {"crc32h_32c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},     \
      {"crc32w_32c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},     \
      {"crc32x_64c_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},     \
      {"lslv_32_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"lslv_64_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"lsrv_32_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"lsrv_64_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"pacga_64p_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},      \
      {"rorv_32_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"rorv_64_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"sdiv_32_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"sdiv_64_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"udiv_32_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"udiv_64_dp_2src"_h, &VISITORCLASS::VisitDataProcessing2Source},        \
      {"madd_32a_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},       \
      {"madd_64a_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},       \
      {"msub_32a_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},       \
      {"msub_64a_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},       \
      {"smaddl_64wa_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},    \
      {"smsubl_64wa_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},    \
      {"smulh_64_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},       \
      {"umaddl_64wa_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},    \
      {"umsubl_64wa_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},    \
      {"umulh_64_dp_3src"_h, &VISITORCLASS::VisitDataProcessing3Source},       \
      {"setf16_only_setf"_h, &VISITORCLASS::VisitEvaluateIntoFlags},           \
      {"setf8_only_setf"_h, &VISITORCLASS::VisitEvaluateIntoFlags},            \
      {"brk_ex_exception"_h, &VISITORCLASS::VisitException},                   \
      {"dcps1_dc_exception"_h, &VISITORCLASS::VisitException},                 \
      {"dcps2_dc_exception"_h, &VISITORCLASS::VisitException},                 \
      {"dcps3_dc_exception"_h, &VISITORCLASS::VisitException},                 \
      {"hlt_ex_exception"_h, &VISITORCLASS::VisitException},                   \
      {"hvc_ex_exception"_h, &VISITORCLASS::VisitException},                   \
      {"smc_ex_exception"_h, &VISITORCLASS::VisitException},                   \
      {"svc_ex_exception"_h, &VISITORCLASS::VisitException},                   \
      {"extr_32_extract"_h, &VISITORCLASS::VisitExtract},                      \
      {"extr_64_extract"_h, &VISITORCLASS::VisitExtract},                      \
      {"fcmpe_dz_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                  \
      {"fcmpe_d_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                   \
      {"fcmpe_hz_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                  \
      {"fcmpe_h_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                   \
      {"fcmpe_sz_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                  \
      {"fcmpe_s_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                   \
      {"fcmp_dz_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                   \
      {"fcmp_d_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                    \
      {"fcmp_hz_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                   \
      {"fcmp_h_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                    \
      {"fcmp_sz_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                   \
      {"fcmp_s_floatcmp"_h, &VISITORCLASS::VisitFPCompare},                    \
      {"fccmpe_d_floatccmp"_h, &VISITORCLASS::VisitFPConditionalCompare},      \
      {"fccmpe_h_floatccmp"_h, &VISITORCLASS::VisitFPConditionalCompare},      \
      {"fccmpe_s_floatccmp"_h, &VISITORCLASS::VisitFPConditionalCompare},      \
      {"fccmp_d_floatccmp"_h, &VISITORCLASS::VisitFPConditionalCompare},       \
      {"fccmp_h_floatccmp"_h, &VISITORCLASS::VisitFPConditionalCompare},       \
      {"fccmp_s_floatccmp"_h, &VISITORCLASS::VisitFPConditionalCompare},       \
      {"fcsel_d_floatsel"_h, &VISITORCLASS::VisitFPConditionalSelect},         \
      {"fcsel_h_floatsel"_h, &VISITORCLASS::VisitFPConditionalSelect},         \
      {"fcsel_s_floatsel"_h, &VISITORCLASS::VisitFPConditionalSelect},         \
      {"bfcvt_bs_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"fabs_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fabs_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fabs_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fcvt_dh_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fcvt_ds_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fcvt_hd_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fcvt_hs_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fcvt_sd_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fcvt_sh_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fmov_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fmov_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fmov_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fneg_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fneg_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"fneg_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},      \
      {"frint32x_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frint32x_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frint32z_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frint32z_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frint64x_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frint64x_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frint64z_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frint64z_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},  \
      {"frinta_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frinta_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frinta_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frinti_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frinti_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frinti_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintm_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintm_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintm_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintn_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintn_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintn_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintp_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintp_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintp_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintx_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintx_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintx_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintz_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintz_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"frintz_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},    \
      {"fsqrt_d_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fsqrt_h_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fsqrt_s_floatdp1"_h, &VISITORCLASS::VisitFPDataProcessing1Source},     \
      {"fadd_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fadd_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fadd_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fdiv_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fdiv_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fdiv_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmaxnm_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},    \
      {"fmaxnm_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},    \
      {"fmaxnm_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},    \
      {"fmax_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmax_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmax_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fminnm_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},    \
      {"fminnm_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},    \
      {"fminnm_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},    \
      {"fmin_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmin_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmin_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmul_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmul_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmul_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fnmul_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},     \
      {"fnmul_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},     \
      {"fnmul_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},     \
      {"fsub_d_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fsub_h_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fsub_s_floatdp2"_h, &VISITORCLASS::VisitFPDataProcessing2Source},      \
      {"fmadd_d_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},     \
      {"fmadd_h_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},     \
      {"fmadd_s_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},     \
      {"fmsub_d_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},     \
      {"fmsub_h_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},     \
      {"fmsub_s_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},     \
      {"fnmadd_d_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},    \
      {"fnmadd_h_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},    \
      {"fnmadd_s_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},    \
      {"fnmsub_d_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},    \
      {"fnmsub_h_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},    \
      {"fnmsub_s_floatdp3"_h, &VISITORCLASS::VisitFPDataProcessing3Source},    \
      {"fcvtzs_32d_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzs_32h_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzs_32s_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzs_64d_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzs_64h_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzs_64s_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzu_32d_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzu_32h_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzu_32s_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzu_64d_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzu_64h_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"fcvtzu_64s_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},     \
      {"scvtf_d32_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"scvtf_d64_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"scvtf_h32_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"scvtf_h64_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"scvtf_s32_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"scvtf_s64_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"ucvtf_d32_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"ucvtf_d64_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"ucvtf_h32_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"ucvtf_h64_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"ucvtf_s32_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"ucvtf_s64_float2fix"_h, &VISITORCLASS::VisitFPFixedPointConvert},      \
      {"fmov_d_floatimm"_h, &VISITORCLASS::VisitFPImmediate},                  \
      {"fmov_h_floatimm"_h, &VISITORCLASS::VisitFPImmediate},                  \
      {"fmov_s_floatimm"_h, &VISITORCLASS::VisitFPImmediate},                  \
      {"fcvtas_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtas_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtas_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtas_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtas_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtas_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtau_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtau_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtau_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtau_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtau_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtau_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtms_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtms_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtms_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtms_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtms_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtms_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtmu_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtmu_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtmu_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtmu_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtmu_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtmu_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtns_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtns_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtns_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtns_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtns_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtns_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtnu_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtnu_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtnu_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtnu_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtnu_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtnu_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtps_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtps_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtps_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtps_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtps_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtps_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtpu_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtpu_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtpu_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtpu_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtpu_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtpu_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzs_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzs_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzs_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzs_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzs_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzs_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzu_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzu_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzu_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzu_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzu_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fcvtzu_64s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},        \
      {"fjcvtzs_32d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},       \
      {"fmov_32h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_32s_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_64d_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_64h_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_64vx_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"fmov_d64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_h32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_h64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_s32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},          \
      {"fmov_v64i_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"scvtf_d32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"scvtf_d64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"scvtf_h32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"scvtf_h64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"scvtf_s32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"scvtf_s64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"ucvtf_d32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"ucvtf_d64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"ucvtf_h32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"ucvtf_h64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"ucvtf_s32_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"ucvtf_s64_float2int"_h, &VISITORCLASS::VisitFPIntegerConvert},         \
      {"ldrsw_64_loadlit"_h, &VISITORCLASS::VisitLoadLiteral},                 \
      {"ldr_32_loadlit"_h, &VISITORCLASS::VisitLoadLiteral},                   \
      {"ldr_64_loadlit"_h, &VISITORCLASS::VisitLoadLiteral},                   \
      {"ldr_d_loadlit"_h, &VISITORCLASS::VisitLoadLiteral},                    \
      {"ldr_q_loadlit"_h, &VISITORCLASS::VisitLoadLiteral},                    \
      {"ldr_s_loadlit"_h, &VISITORCLASS::VisitLoadLiteral},                    \
      {"prfm_p_loadlit"_h, &VISITORCLASS::VisitLoadLiteral},                   \
      {"casab_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"casah_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"casalb_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"casalh_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"casal_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"casal_c64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"casa_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},         \
      {"casa_c64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},         \
      {"casb_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},         \
      {"cash_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},         \
      {"caslb_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"caslh_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"casl_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},         \
      {"casl_c64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},         \
      {"caspal_cp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"caspal_cp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"caspa_cp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"caspa_cp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"caspl_cp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"caspl_cp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"casp_cp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"casp_cp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"cas_c32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},          \
      {"cas_c64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},          \
      {"ldarb_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldarh_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldar_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"ldar_lr64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"ldaxp_lp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldaxp_lp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldaxrb_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"ldaxrh_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"ldaxr_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldaxr_lr64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldlarb_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"ldlarh_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"ldlar_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldlar_lr64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldxp_lp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"ldxp_lp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"ldxrb_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldxrh_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"ldxr_lr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"ldxr_lr64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"stllrb_sl32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"stllrh_sl32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"stllr_sl32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stllr_sl64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stlrb_sl32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stlrh_sl32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stlr_sl32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"stlr_sl64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"stlxp_sp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stlxp_sp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stlxrb_sr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"stlxrh_sr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},      \
      {"stlxr_sr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stlxr_sr64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stxp_sp32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"stxp_sp64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"stxrb_sr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stxrh_sr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},       \
      {"stxr_sr32_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"stxr_sr64_ldstexcl"_h, &VISITORCLASS::VisitLoadStoreExclusive},        \
      {"ldraa_64w_ldst_pac"_h, &VISITORCLASS::VisitLoadStorePAC},              \
      {"ldraa_64_ldst_pac"_h, &VISITORCLASS::VisitLoadStorePAC},               \
      {"ldrab_64w_ldst_pac"_h, &VISITORCLASS::VisitLoadStorePAC},              \
      {"ldrab_64_ldst_pac"_h, &VISITORCLASS::VisitLoadStorePAC},               \
      {"ldnp_32_ldstnapair_offs"_h,                                            \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"ldnp_64_ldstnapair_offs"_h,                                            \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"ldnp_d_ldstnapair_offs"_h,                                             \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"ldnp_q_ldstnapair_offs"_h,                                             \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"ldnp_s_ldstnapair_offs"_h,                                             \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"stnp_32_ldstnapair_offs"_h,                                            \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"stnp_64_ldstnapair_offs"_h,                                            \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"stnp_d_ldstnapair_offs"_h,                                             \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"stnp_q_ldstnapair_offs"_h,                                             \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"stnp_s_ldstnapair_offs"_h,                                             \
       &VISITORCLASS::VisitLoadStorePairNonTemporal},                          \
      {"ldpsw_64_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},    \
      {"ldp_32_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},      \
      {"ldp_64_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},      \
      {"ldp_d_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},       \
      {"ldp_q_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},       \
      {"ldp_s_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},       \
      {"stp_32_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},      \
      {"stp_64_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},      \
      {"stp_d_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},       \
      {"stp_q_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},       \
      {"stp_s_ldstpair_off"_h, &VISITORCLASS::VisitLoadStorePairOffset},       \
      {"ldpsw_64_ldstpair_post"_h,                                             \
       &VISITORCLASS::VisitLoadStorePairPostIndex},                            \
      {"ldp_32_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},  \
      {"ldp_64_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},  \
      {"ldp_d_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},   \
      {"ldp_q_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},   \
      {"ldp_s_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},   \
      {"stp_32_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},  \
      {"stp_64_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},  \
      {"stp_d_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},   \
      {"stp_q_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},   \
      {"stp_s_ldstpair_post"_h, &VISITORCLASS::VisitLoadStorePairPostIndex},   \
      {"ldpsw_64_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},  \
      {"ldp_32_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},    \
      {"ldp_64_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},    \
      {"ldp_d_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},     \
      {"ldp_q_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},     \
      {"ldp_s_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},     \
      {"stp_32_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},    \
      {"stp_64_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},    \
      {"stp_d_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},     \
      {"stp_q_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},     \
      {"stp_s_ldstpair_pre"_h, &VISITORCLASS::VisitLoadStorePairPreIndex},     \
      {"ldrb_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},      \
      {"ldrh_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},      \
      {"ldrsb_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},     \
      {"ldrsb_64_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},     \
      {"ldrsh_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},     \
      {"ldrsh_64_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},     \
      {"ldrsw_64_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},     \
      {"ldr_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},       \
      {"ldr_64_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},       \
      {"ldr_b_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"ldr_d_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"ldr_h_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"ldr_q_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"ldr_s_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"strb_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},      \
      {"strh_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},      \
      {"str_32_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},       \
      {"str_64_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},       \
      {"str_b_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"str_d_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"str_h_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"str_q_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"str_s_ldst_immpost"_h, &VISITORCLASS::VisitLoadStorePostIndex},        \
      {"ldrb_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},        \
      {"ldrh_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},        \
      {"ldrsb_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},       \
      {"ldrsb_64_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},       \
      {"ldrsh_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},       \
      {"ldrsh_64_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},       \
      {"ldrsw_64_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},       \
      {"ldr_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},         \
      {"ldr_64_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},         \
      {"ldr_b_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"ldr_d_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"ldr_h_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"ldr_q_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"ldr_s_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"strb_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},        \
      {"strh_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},        \
      {"str_32_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},         \
      {"str_64_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},         \
      {"str_b_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"str_d_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"str_h_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"str_q_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"str_s_ldst_immpre"_h, &VISITORCLASS::VisitLoadStorePreIndex},          \
      {"ldapurb_32_ldapstl_unscaled"_h,                                        \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapurh_32_ldapstl_unscaled"_h,                                        \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapursb_32_ldapstl_unscaled"_h,                                       \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapursb_64_ldapstl_unscaled"_h,                                       \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapursh_32_ldapstl_unscaled"_h,                                       \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapursh_64_ldapstl_unscaled"_h,                                       \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapursw_64_ldapstl_unscaled"_h,                                       \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapur_32_ldapstl_unscaled"_h,                                         \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldapur_64_ldapstl_unscaled"_h,                                         \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"stlurb_32_ldapstl_unscaled"_h,                                         \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"stlurh_32_ldapstl_unscaled"_h,                                         \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"stlur_32_ldapstl_unscaled"_h,                                          \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"stlur_64_ldapstl_unscaled"_h,                                          \
       &VISITORCLASS::VisitLoadStoreRCpcUnscaledOffset},                       \
      {"ldrb_32bl_ldst_regoff"_h,                                              \
       &VISITORCLASS::VisitLoadStoreRegisterOffset},                           \
      {"ldrb_32b_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset}, \
      {"ldrh_32_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},  \
      {"ldrsb_32bl_ldst_regoff"_h,                                             \
       &VISITORCLASS::VisitLoadStoreRegisterOffset},                           \
      {"ldrsb_32b_ldst_regoff"_h,                                              \
       &VISITORCLASS::VisitLoadStoreRegisterOffset},                           \
      {"ldrsb_64bl_ldst_regoff"_h,                                             \
       &VISITORCLASS::VisitLoadStoreRegisterOffset},                           \
      {"ldrsb_64b_ldst_regoff"_h,                                              \
       &VISITORCLASS::VisitLoadStoreRegisterOffset},                           \
      {"ldrsh_32_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset}, \
      {"ldrsh_64_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset}, \
      {"ldrsw_64_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset}, \
      {"ldr_32_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},   \
      {"ldr_64_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},   \
      {"ldr_bl_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},   \
      {"ldr_b_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"ldr_d_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"ldr_h_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"ldr_q_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"ldr_s_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"prfm_p_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},   \
      {"strb_32bl_ldst_regoff"_h,                                              \
       &VISITORCLASS::VisitLoadStoreRegisterOffset},                           \
      {"strb_32b_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset}, \
      {"strh_32_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},  \
      {"str_32_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},   \
      {"str_64_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},   \
      {"str_bl_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},   \
      {"str_b_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"str_d_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"str_h_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"str_q_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"str_s_ldst_regoff"_h, &VISITORCLASS::VisitLoadStoreRegisterOffset},    \
      {"ldurb_32_ldst_unscaled"_h,                                             \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldurh_32_ldst_unscaled"_h,                                             \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldursb_32_ldst_unscaled"_h,                                            \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldursb_64_ldst_unscaled"_h,                                            \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldursh_32_ldst_unscaled"_h,                                            \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldursh_64_ldst_unscaled"_h,                                            \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldursw_64_ldst_unscaled"_h,                                            \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldur_32_ldst_unscaled"_h,                                              \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldur_64_ldst_unscaled"_h,                                              \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"ldur_b_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"ldur_d_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"ldur_h_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"ldur_q_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"ldur_s_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"prfum_p_ldst_unscaled"_h,                                              \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"sturb_32_ldst_unscaled"_h,                                             \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"sturh_32_ldst_unscaled"_h,                                             \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"stur_32_ldst_unscaled"_h,                                              \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"stur_64_ldst_unscaled"_h,                                              \
       &VISITORCLASS::VisitLoadStoreUnscaledOffset},                           \
      {"stur_b_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"stur_d_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"stur_h_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"stur_q_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"stur_s_ldst_unscaled"_h, &VISITORCLASS::VisitLoadStoreUnscaledOffset}, \
      {"ldrb_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},     \
      {"ldrh_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},     \
      {"ldrsb_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},    \
      {"ldrsb_64_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},    \
      {"ldrsh_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},    \
      {"ldrsh_64_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},    \
      {"ldrsw_64_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},    \
      {"ldr_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},      \
      {"ldr_64_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},      \
      {"ldr_b_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"ldr_d_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"ldr_h_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"ldr_q_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"ldr_s_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"prfm_p_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},      \
      {"strb_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},     \
      {"strh_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},     \
      {"str_32_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},      \
      {"str_64_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},      \
      {"str_b_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"str_d_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"str_h_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"str_q_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"str_s_ldst_pos"_h, &VISITORCLASS::VisitLoadStoreUnsignedOffset},       \
      {"ands_32s_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},            \
      {"ands_64s_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},            \
      {"and_32_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},              \
      {"and_64_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},              \
      {"eor_32_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},              \
      {"eor_64_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},              \
      {"orr_32_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},              \
      {"orr_64_log_imm"_h, &VISITORCLASS::VisitLogicalImmediate},              \
      {"ands_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},             \
      {"ands_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},             \
      {"and_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"and_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"bics_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},             \
      {"bics_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},             \
      {"bic_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"bic_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"eon_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"eon_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"eor_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"eor_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"orn_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"orn_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"orr_32_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"orr_64_log_shift"_h, &VISITORCLASS::VisitLogicalShifted},              \
      {"movk_32_movewide"_h, &VISITORCLASS::VisitMoveWideImmediate},           \
      {"movk_64_movewide"_h, &VISITORCLASS::VisitMoveWideImmediate},           \
      {"movn_32_movewide"_h, &VISITORCLASS::VisitMoveWideImmediate},           \
      {"movn_64_movewide"_h, &VISITORCLASS::VisitMoveWideImmediate},           \
      {"movz_32_movewide"_h, &VISITORCLASS::VisitMoveWideImmediate},           \
      {"movz_64_movewide"_h, &VISITORCLASS::VisitMoveWideImmediate},           \
      {"fabs_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},        \
      {"fcmeq_asimdmiscfp16_fz"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcmge_asimdmiscfp16_fz"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcmgt_asimdmiscfp16_fz"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcmle_asimdmiscfp16_fz"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcmlt_asimdmiscfp16_fz"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtas_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtau_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtms_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtmu_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtns_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtnu_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtps_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtpu_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtzs_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fcvtzu_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"fneg_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},        \
      {"frecpe_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frinta_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frinti_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frintm_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frintn_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frintp_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frintx_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frintz_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},      \
      {"frsqrte_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},     \
      {"fsqrt_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},       \
      {"scvtf_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},       \
      {"ucvtf_asimdmiscfp16_r"_h, &VISITORCLASS::VisitNEON2RegMiscFP16},       \
      {"addhn_asimddiff_n"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"raddhn_asimddiff_n"_h, &VISITORCLASS::VisitNEON3Different},            \
      {"rsubhn_asimddiff_n"_h, &VISITORCLASS::VisitNEON3Different},            \
      {"sabal_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"sabdl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"saddl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"saddw_asimddiff_w"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"smlal_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"smlsl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"smull_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"sqdmlal_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},           \
      {"sqdmlsl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},           \
      {"sqdmull_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},           \
      {"ssubl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"ssubw_asimddiff_w"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"subhn_asimddiff_n"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"uabal_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"uabdl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"uaddl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"uaddw_asimddiff_w"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"umlal_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"umlsl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"umull_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"usubl_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"usubw_asimddiff_w"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"addp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"add_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"cmeq_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"cmge_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"cmgt_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"cmhi_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"cmhs_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"cmtst_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fabd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"facge_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"facgt_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"faddp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fadd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fcmeq_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fcmge_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fcmgt_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fdiv_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fmaxnmp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},             \
      {"fmaxnm_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},              \
      {"fmaxp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fmax_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fminnmp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},             \
      {"fminnm_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},              \
      {"fminp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fmin_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fmla_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fmls_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fmulx_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"fmul_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"frecps_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},              \
      {"frsqrts_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},             \
      {"fsub_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"sqadd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"sqdmulh_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},             \
      {"sqrdmulh_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},            \
      {"sqrshl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},              \
      {"sqshl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"sqsub_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"srshl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"sshl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"sub_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"uqadd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"uqrshl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},              \
      {"uqshl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"uqsub_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"urshl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"ushl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fcadd_asimdsame2_c"_h, &VISITORCLASS::VisitNEON3SameExtra},            \
      {"fcmla_asimdsame2_c"_h, &VISITORCLASS::VisitNEON3SameExtra},            \
      {"sdot_asimdsame2_d"_h, &VISITORCLASS::VisitNEON3SameExtra},             \
      {"sqrdmlah_asimdsame2_only"_h, &VISITORCLASS::VisitNEON3SameExtra},      \
      {"sqrdmlsh_asimdsame2_only"_h, &VISITORCLASS::VisitNEON3SameExtra},      \
      {"udot_asimdsame2_d"_h, &VISITORCLASS::VisitNEON3SameExtra},             \
      {"fabd_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"facge_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"facgt_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"faddp_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"fadd_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"fcmeq_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"fcmge_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"fcmgt_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"fdiv_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"fmaxnmp_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},     \
      {"fmaxnm_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},      \
      {"fmaxp_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"fmax_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"fminnmp_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},     \
      {"fminnm_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},      \
      {"fminp_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"fmin_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"fmla_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"fmls_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"fmulx_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},       \
      {"fmul_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"frecps_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},      \
      {"frsqrts_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},     \
      {"fsub_asimdsamefp16_only"_h, &VISITORCLASS::VisitNEON3SameFP16},        \
      {"addv_asimdall_only"_h, &VISITORCLASS::VisitNEONAcrossLanes},           \
      {"saddlv_asimdall_only"_h, &VISITORCLASS::VisitNEONAcrossLanes},         \
      {"smaxv_asimdall_only"_h, &VISITORCLASS::VisitNEONAcrossLanes},          \
      {"sminv_asimdall_only"_h, &VISITORCLASS::VisitNEONAcrossLanes},          \
      {"uaddlv_asimdall_only"_h, &VISITORCLASS::VisitNEONAcrossLanes},         \
      {"umaxv_asimdall_only"_h, &VISITORCLASS::VisitNEONAcrossLanes},          \
      {"uminv_asimdall_only"_h, &VISITORCLASS::VisitNEONAcrossLanes},          \
      {"mla_asimdelem_r"_h, &VISITORCLASS::VisitNEONByIndexedElement},         \
      {"mls_asimdelem_r"_h, &VISITORCLASS::VisitNEONByIndexedElement},         \
      {"mul_asimdelem_r"_h, &VISITORCLASS::VisitNEONByIndexedElement},         \
      {"sqdmulh_asimdelem_r"_h, &VISITORCLASS::VisitNEONByIndexedElement},     \
      {"sqrdmlah_asimdelem_r"_h, &VISITORCLASS::VisitNEONByIndexedElement},    \
      {"sqrdmlsh_asimdelem_r"_h, &VISITORCLASS::VisitNEONByIndexedElement},    \
      {"sqrdmulh_asimdelem_r"_h, &VISITORCLASS::VisitNEONByIndexedElement},    \
      {"dup_asimdins_dr_r"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"dup_asimdins_dv_v"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"ins_asimdins_ir_r"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"ins_asimdins_iv_v"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"smov_asimdins_w_w"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"smov_asimdins_x_x"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"umov_asimdins_w_w"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"umov_asimdins_x_x"_h, &VISITORCLASS::VisitNEONCopy},                   \
      {"ext_asimdext_only"_h, &VISITORCLASS::VisitNEONExtract},                \
      {"ld1_asisdlse_r1_1v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"ld1_asisdlse_r2_2v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"ld1_asisdlse_r3_3v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"ld1_asisdlse_r4_4v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"ld2_asisdlse_r2"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},     \
      {"ld3_asisdlse_r3"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},     \
      {"ld4_asisdlse_r4"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},     \
      {"st1_asisdlse_r1_1v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"st1_asisdlse_r2_2v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"st1_asisdlse_r3_3v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"st1_asisdlse_r4_4v"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},  \
      {"st2_asisdlse_r2"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},     \
      {"st3_asisdlse_r3"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},     \
      {"st4_asisdlse_r4"_h, &VISITORCLASS::VisitNEONLoadStoreMultiStruct},     \
      {"ld1_asisdlsep_i1_i1"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1_asisdlsep_i2_i2"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1_asisdlsep_i3_i3"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1_asisdlsep_i4_i4"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1_asisdlsep_r1_r1"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1_asisdlsep_r2_r2"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1_asisdlsep_r3_r3"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1_asisdlsep_r4_r4"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld2_asisdlsep_i2_i"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld2_asisdlsep_r2_r"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld3_asisdlsep_i3_i"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld3_asisdlsep_r3_r"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld4_asisdlsep_i4_i"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld4_asisdlsep_r4_r"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_i1_i1"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_i2_i2"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_i3_i3"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_i4_i4"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_r1_r1"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_r2_r2"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_r3_r3"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st1_asisdlsep_r4_r4"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st2_asisdlsep_i2_i"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st2_asisdlsep_r2_r"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st3_asisdlsep_i3_i"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st3_asisdlsep_r3_r"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st4_asisdlsep_i4_i"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"st4_asisdlsep_r4_r"_h,                                                 \
       &VISITORCLASS::VisitNEONLoadStoreMultiStructPostIndex},                 \
      {"ld1r_asisdlso_r1"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct},   \
      {"ld1_asisdlso_b1_1b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld1_asisdlso_d1_1d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld1_asisdlso_h1_1h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld1_asisdlso_s1_1s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld2r_asisdlso_r2"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct},   \
      {"ld2_asisdlso_b2_2b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld2_asisdlso_d2_2d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld2_asisdlso_h2_2h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld2_asisdlso_s2_2s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld3r_asisdlso_r3"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct},   \
      {"ld3_asisdlso_b3_3b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld3_asisdlso_d3_3d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld3_asisdlso_h3_3h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld3_asisdlso_s3_3s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld4r_asisdlso_r4"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct},   \
      {"ld4_asisdlso_b4_4b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld4_asisdlso_d4_4d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld4_asisdlso_h4_4h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld4_asisdlso_s4_4s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st1_asisdlso_b1_1b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st1_asisdlso_d1_1d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st1_asisdlso_h1_1h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st1_asisdlso_s1_1s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st2_asisdlso_b2_2b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st2_asisdlso_d2_2d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st2_asisdlso_h2_2h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st2_asisdlso_s2_2s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st3_asisdlso_b3_3b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st3_asisdlso_d3_3d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st3_asisdlso_h3_3h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st3_asisdlso_s3_3s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st4_asisdlso_b4_4b"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st4_asisdlso_d4_4d"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st4_asisdlso_h4_4h"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"st4_asisdlso_s4_4s"_h, &VISITORCLASS::VisitNEONLoadStoreSingleStruct}, \
      {"ld1r_asisdlsop_r1_i"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1r_asisdlsop_rx1_r"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_b1_i1b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_bx1_r1b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_d1_i1d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_dx1_r1d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_h1_i1h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_hx1_r1h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_s1_i1s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld1_asisdlsop_sx1_r1s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2r_asisdlsop_r2_i"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2r_asisdlsop_rx2_r"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_b2_i2b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_bx2_r2b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_d2_i2d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_dx2_r2d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_h2_i2h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_hx2_r2h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_s2_i2s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld2_asisdlsop_sx2_r2s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3r_asisdlsop_r3_i"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3r_asisdlsop_rx3_r"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_b3_i3b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_bx3_r3b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_d3_i3d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_dx3_r3d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_h3_i3h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_hx3_r3h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_s3_i3s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld3_asisdlsop_sx3_r3s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4r_asisdlsop_r4_i"_h,                                                \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4r_asisdlsop_rx4_r"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_b4_i4b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_bx4_r4b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_d4_i4d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_dx4_r4d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_h4_i4h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_hx4_r4h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_s4_i4s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"ld4_asisdlsop_sx4_r4s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_b1_i1b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_bx1_r1b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_d1_i1d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_dx1_r1d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_h1_i1h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_hx1_r1h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_s1_i1s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st1_asisdlsop_sx1_r1s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_b2_i2b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_bx2_r2b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_d2_i2d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_dx2_r2d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_h2_i2h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_hx2_r2h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_s2_i2s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st2_asisdlsop_sx2_r2s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_b3_i3b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_bx3_r3b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_d3_i3d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_dx3_r3d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_h3_i3h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_hx3_r3h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_s3_i3s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st3_asisdlsop_sx3_r3s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_b4_i4b"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_bx4_r4b"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_d4_i4d"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_dx4_r4d"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_h4_i4h"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_hx4_r4h"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_s4_i4s"_h,                                               \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"st4_asisdlsop_sx4_r4s"_h,                                              \
       &VISITORCLASS::VisitNEONLoadStoreSingleStructPostIndex},                \
      {"bic_asimdimm_l_hl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},      \
      {"bic_asimdimm_l_sl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},      \
      {"fmov_asimdimm_d2_d"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"fmov_asimdimm_h_h"_h, &VISITORCLASS::VisitNEONModifiedImmediate},      \
      {"fmov_asimdimm_s_s"_h, &VISITORCLASS::VisitNEONModifiedImmediate},      \
      {"movi_asimdimm_d2_d"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"movi_asimdimm_d_ds"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"movi_asimdimm_l_hl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"movi_asimdimm_l_sl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"movi_asimdimm_m_sm"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"movi_asimdimm_n_b"_h, &VISITORCLASS::VisitNEONModifiedImmediate},      \
      {"mvni_asimdimm_l_hl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"mvni_asimdimm_l_sl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"mvni_asimdimm_m_sm"_h, &VISITORCLASS::VisitNEONModifiedImmediate},     \
      {"orr_asimdimm_l_hl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},      \
      {"orr_asimdimm_l_sl"_h, &VISITORCLASS::VisitNEONModifiedImmediate},      \
      {"trn1_asimdperm_only"_h, &VISITORCLASS::VisitNEONPerm},                 \
      {"trn2_asimdperm_only"_h, &VISITORCLASS::VisitNEONPerm},                 \
      {"uzp1_asimdperm_only"_h, &VISITORCLASS::VisitNEONPerm},                 \
      {"uzp2_asimdperm_only"_h, &VISITORCLASS::VisitNEONPerm},                 \
      {"zip1_asimdperm_only"_h, &VISITORCLASS::VisitNEONPerm},                 \
      {"zip2_asimdperm_only"_h, &VISITORCLASS::VisitNEONPerm},                 \
      {"sqabs_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},         \
      {"sqneg_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},         \
      {"sqxtn_asisdmisc_n"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},         \
      {"sqxtun_asisdmisc_n"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"suqadd_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"uqxtn_asisdmisc_n"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},         \
      {"usqadd_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcmeq_asisdmiscfp16_fz"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcmge_asisdmiscfp16_fz"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcmgt_asisdmiscfp16_fz"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcmle_asisdmiscfp16_fz"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcmlt_asisdmiscfp16_fz"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtas_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtau_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtms_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtmu_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtns_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtnu_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtps_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtpu_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtzs_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"fcvtzu_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"frecpe_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"frecpx_asisdmiscfp16_r"_h,                                             \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"frsqrte_asisdmiscfp16_r"_h,                                            \
       &VISITORCLASS::VisitNEONScalar2RegMiscFP16},                            \
      {"scvtf_asisdmiscfp16_r"_h, &VISITORCLASS::VisitNEONScalar2RegMiscFP16}, \
      {"ucvtf_asisdmiscfp16_r"_h, &VISITORCLASS::VisitNEONScalar2RegMiscFP16}, \
      {"sqdmlal_asisddiff_only"_h, &VISITORCLASS::VisitNEONScalar3Diff},       \
      {"sqdmlsl_asisddiff_only"_h, &VISITORCLASS::VisitNEONScalar3Diff},       \
      {"sqdmull_asisddiff_only"_h, &VISITORCLASS::VisitNEONScalar3Diff},       \
      {"sqadd_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"sqdmulh_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},       \
      {"sqrdmulh_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},      \
      {"sqrshl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},        \
      {"sqshl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"sqsub_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"srshl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"sshl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"uqadd_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"uqrshl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},        \
      {"uqshl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"uqsub_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"urshl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"ushl_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"fabd_asisdsamefp16_only"_h, &VISITORCLASS::VisitNEONScalar3SameFP16},  \
      {"facge_asisdsamefp16_only"_h, &VISITORCLASS::VisitNEONScalar3SameFP16}, \
      {"facgt_asisdsamefp16_only"_h, &VISITORCLASS::VisitNEONScalar3SameFP16}, \
      {"fcmeq_asisdsamefp16_only"_h, &VISITORCLASS::VisitNEONScalar3SameFP16}, \
      {"fcmge_asisdsamefp16_only"_h, &VISITORCLASS::VisitNEONScalar3SameFP16}, \
      {"fcmgt_asisdsamefp16_only"_h, &VISITORCLASS::VisitNEONScalar3SameFP16}, \
      {"fmulx_asisdsamefp16_only"_h, &VISITORCLASS::VisitNEONScalar3SameFP16}, \
      {"frecps_asisdsamefp16_only"_h,                                          \
       &VISITORCLASS::VisitNEONScalar3SameFP16},                               \
      {"frsqrts_asisdsamefp16_only"_h,                                         \
       &VISITORCLASS::VisitNEONScalar3SameFP16},                               \
      {"sqdmulh_asisdelem_r"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"sqrdmlah_asisdelem_r"_h,                                               \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"sqrdmlsh_asisdelem_r"_h,                                               \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"sqrdmulh_asisdelem_r"_h,                                               \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"dup_asisdone_only"_h, &VISITORCLASS::VisitNEONScalarCopy},             \
      {"addp_asisdpair_only"_h, &VISITORCLASS::VisitNEONScalarPairwise},       \
      {"faddp_asisdpair_only_h"_h, &VISITORCLASS::VisitNEONScalarPairwise},    \
      {"faddp_asisdpair_only_sd"_h, &VISITORCLASS::VisitNEONScalarPairwise},   \
      {"fmaxnmp_asisdpair_only_h"_h, &VISITORCLASS::VisitNEONScalarPairwise},  \
      {"fmaxnmp_asisdpair_only_sd"_h, &VISITORCLASS::VisitNEONScalarPairwise}, \
      {"fmaxp_asisdpair_only_h"_h, &VISITORCLASS::VisitNEONScalarPairwise},    \
      {"fmaxp_asisdpair_only_sd"_h, &VISITORCLASS::VisitNEONScalarPairwise},   \
      {"fminnmp_asisdpair_only_h"_h, &VISITORCLASS::VisitNEONScalarPairwise},  \
      {"fminnmp_asisdpair_only_sd"_h, &VISITORCLASS::VisitNEONScalarPairwise}, \
      {"fminp_asisdpair_only_h"_h, &VISITORCLASS::VisitNEONScalarPairwise},    \
      {"fminp_asisdpair_only_sd"_h, &VISITORCLASS::VisitNEONScalarPairwise},   \
      {"fcvtzs_asisdshf_c"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},   \
      {"fcvtzu_asisdshf_c"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},   \
      {"scvtf_asisdshf_c"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"sqshlu_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},   \
      {"sqshl_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"ucvtf_asisdshf_c"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"uqshl_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"sqshlu_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},         \
      {"sqshl_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"uqshl_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"shl_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},            \
      {"sli_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},            \
      {"tbl_asimdtbl_l1_1"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"tbl_asimdtbl_l2_2"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"tbl_asimdtbl_l3_3"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"tbl_asimdtbl_l4_4"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"tbx_asimdtbl_l1_1"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"tbx_asimdtbl_l2_2"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"tbx_asimdtbl_l3_3"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"tbx_asimdtbl_l4_4"_h, &VISITORCLASS::VisitNEONTable},                  \
      {"adrp_only_pcreladdr"_h, &VISITORCLASS::VisitPCRelAddressing},          \
      {"adr_only_pcreladdr"_h, &VISITORCLASS::VisitPCRelAddressing},           \
      {"rmif_only_rmif"_h, &VISITORCLASS::VisitRotateRightIntoFlags},          \
      {"bti_hb_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"clrex_bn_barriers"_h, &VISITORCLASS::VisitSystem},                     \
      {"dmb_bo_barriers"_h, &VISITORCLASS::VisitSystem},                       \
      {"dsb_bo_barriers"_h, &VISITORCLASS::VisitSystem},                       \
      {"hint_hm_hints"_h, &VISITORCLASS::VisitSystem},                         \
      {"chkfeat_hf_hints"_h, &VISITORCLASS::VisitSystem},                      \
      {"mrs_rs_systemmove"_h, &VISITORCLASS::VisitSystem},                     \
      {"msr_sr_systemmove"_h, &VISITORCLASS::VisitSystem},                     \
      {"psb_hc_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"sb_only_barriers"_h, &VISITORCLASS::VisitSystem},                      \
      {"sysl_rc_systeminstrs"_h, &VISITORCLASS::VisitSystem},                  \
      {"sys_cr_systeminstrs"_h, &VISITORCLASS::VisitSystem},                   \
      {"tcommit_only_barriers"_h, &VISITORCLASS::VisitSystem},                 \
      {"tsb_hc_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"tbnz_only_testbranch"_h, &VISITORCLASS::VisitTestBranch},              \
      {"tbz_only_testbranch"_h, &VISITORCLASS::VisitTestBranch},               \
      {"bl_only_branch_imm"_h, &VISITORCLASS::VisitUnconditionalBranch},       \
      {"b_only_branch_imm"_h, &VISITORCLASS::VisitUnconditionalBranch},        \
      {"blraaz_64_branch_reg"_h,                                               \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"blraa_64p_branch_reg"_h,                                               \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"blrabz_64_branch_reg"_h,                                               \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"blrab_64p_branch_reg"_h,                                               \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"blr_64_branch_reg"_h,                                                  \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"braaz_64_branch_reg"_h,                                                \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"braa_64p_branch_reg"_h,                                                \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"brabz_64_branch_reg"_h,                                                \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"brab_64p_branch_reg"_h,                                                \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"br_64_branch_reg"_h,                                                   \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"drps_64e_branch_reg"_h,                                                \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"eretaa_64e_branch_reg"_h,                                              \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"eretab_64e_branch_reg"_h,                                              \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"eret_64e_branch_reg"_h,                                                \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"retaa_64e_branch_reg"_h,                                               \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"retab_64e_branch_reg"_h,                                               \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"ret_64r_branch_reg"_h,                                                 \
       &VISITORCLASS::VisitUnconditionalBranchToRegister},                     \
      {"bfcvtn_asimdmisc_4s"_h, &VISITORCLASS::VisitUnimplemented},            \
      {"bfdot_asimdelem_e"_h, &VISITORCLASS::VisitUnimplemented},              \
      {"bfdot_asimdsame2_d"_h, &VISITORCLASS::VisitUnimplemented},             \
      {"bfmlal_asimdelem_f"_h, &VISITORCLASS::VisitUnimplemented},             \
      {"bfmlal_asimdsame2_f"_h, &VISITORCLASS::VisitUnimplemented},            \
      {"bfmmla_asimdsame2_e"_h, &VISITORCLASS::VisitUnimplemented},            \
      {"dsb_bon_barriers"_h, &VISITORCLASS::VisitUnimplemented},               \
      {"ld64b_64l_memop"_h, &VISITORCLASS::VisitUnimplemented},                \
      {"ldgm_64bulk_ldsttags"_h, &VISITORCLASS::VisitUnimplemented},           \
      {"ldtrb_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},           \
      {"ldtrh_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},           \
      {"ldtrsb_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},          \
      {"ldtrsb_64_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},          \
      {"ldtrsh_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},          \
      {"ldtrsh_64_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},          \
      {"ldtrsw_64_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},          \
      {"ldtr_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},            \
      {"ldtr_64_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},            \
      {"sm3partw1_vvv4_cryptosha512_3"_h, &VISITORCLASS::VisitCryptoSM3},      \
      {"sm3partw2_vvv4_cryptosha512_3"_h, &VISITORCLASS::VisitCryptoSM3},      \
      {"sm3ss1_vvv4_crypto4"_h, &VISITORCLASS::VisitCryptoSM3},                \
      {"sm3tt1a_vvv4_crypto3_imm2"_h, &VISITORCLASS::VisitCryptoSM3},          \
      {"sm3tt1b_vvv4_crypto3_imm2"_h, &VISITORCLASS::VisitCryptoSM3},          \
      {"sm3tt2a_vvv4_crypto3_imm2"_h, &VISITORCLASS::VisitCryptoSM3},          \
      {"sm3tt2b_vvv_crypto3_imm2"_h, &VISITORCLASS::VisitCryptoSM3},           \
      {"sm4ekey_vvv4_cryptosha512_3"_h, &VISITORCLASS::VisitCryptoSM4},        \
      {"sm4e_vv4_cryptosha512_2"_h, &VISITORCLASS::VisitCryptoSM4},            \
      {"st64b_64l_memop"_h, &VISITORCLASS::VisitUnimplemented},                \
      {"st64bv_64_memop"_h, &VISITORCLASS::VisitUnimplemented},                \
      {"st64bv0_64_memop"_h, &VISITORCLASS::VisitUnimplemented},               \
      {"stgm_64bulk_ldsttags"_h, &VISITORCLASS::VisitUnimplemented},           \
      {"sttrb_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},           \
      {"sttrh_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},           \
      {"sttr_32_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},            \
      {"sttr_64_ldst_unpriv"_h, &VISITORCLASS::VisitUnimplemented},            \
      {"stzgm_64bulk_ldsttags"_h, &VISITORCLASS::VisitUnimplemented},          \
      {"tcancel_ex_exception"_h, &VISITORCLASS::VisitUnimplemented},           \
      {"tstart_br_systemresult"_h, &VISITORCLASS::VisitUnimplemented},         \
      {"ttest_br_systemresult"_h, &VISITORCLASS::VisitUnimplemented},          \
      {"wfet_only_systeminstrswithreg"_h, &VISITORCLASS::VisitUnimplemented},  \
      {"wfit_only_systeminstrswithreg"_h, &VISITORCLASS::VisitUnimplemented},  \
      {"bfcvt_z_p_z_s2bf"_h, &VISITORCLASS::VisitUnimplemented},               \
      {"bfcvtnt_z_p_z_s2bf"_h, &VISITORCLASS::VisitUnimplemented},             \
      {"bfdot_z_zzz"_h, &VISITORCLASS::VisitUnimplemented},                    \
      {"bfdot_z_zzzi"_h, &VISITORCLASS::VisitUnimplemented},                   \
      {"bfmlalb_z_zzz"_h, &VISITORCLASS::VisitUnimplemented},                  \
      {"bfmlalb_z_zzzi"_h, &VISITORCLASS::VisitUnimplemented},                 \
      {"bfmlalt_z_zzz"_h, &VISITORCLASS::VisitUnimplemented},                  \
      {"bfmlalt_z_zzzi"_h, &VISITORCLASS::VisitUnimplemented},                 \
      {"bfmmla_z_zzz"_h, &VISITORCLASS::VisitUnimplemented}, {                 \
    "unallocated"_h, &VISITORCLASS::VisitUnallocated                           \
  }

#define SIM_AUD_VISITOR_MAP(VISITORCLASS)                                      \
  {"autia1716_hi_hints"_h, &VISITORCLASS::VisitSystem},                        \
      {"autiasp_hi_hints"_h, &VISITORCLASS::VisitSystem},                      \
      {"autiaz_hi_hints"_h, &VISITORCLASS::VisitSystem},                       \
      {"autib1716_hi_hints"_h, &VISITORCLASS::VisitSystem},                    \
      {"autibsp_hi_hints"_h, &VISITORCLASS::VisitSystem},                      \
      {"autibz_hi_hints"_h, &VISITORCLASS::VisitSystem},                       \
      {"axflag_m_pstate"_h, &VISITORCLASS::VisitSystem},                       \
      {"cfinv_m_pstate"_h, &VISITORCLASS::VisitSystem},                        \
      {"csdb_hi_hints"_h, &VISITORCLASS::VisitSystem},                         \
      {"dgh_hi_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"esb_hi_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"isb_bi_barriers"_h, &VISITORCLASS::VisitSystem},                       \
      {"nop_hi_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"pacia1716_hi_hints"_h, &VISITORCLASS::VisitSystem},                    \
      {"paciasp_hi_hints"_h, &VISITORCLASS::VisitSystem},                      \
      {"paciaz_hi_hints"_h, &VISITORCLASS::VisitSystem},                       \
      {"pacib1716_hi_hints"_h, &VISITORCLASS::VisitSystem},                    \
      {"pacibsp_hi_hints"_h, &VISITORCLASS::VisitSystem},                      \
      {"pacibz_hi_hints"_h, &VISITORCLASS::VisitSystem},                       \
      {"sev_hi_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"sevl_hi_hints"_h, &VISITORCLASS::VisitSystem},                         \
      {"ssbb_only_barriers"_h, &VISITORCLASS::VisitSystem},                    \
      {"wfe_hi_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"wfi_hi_hints"_h, &VISITORCLASS::VisitSystem},                          \
      {"xaflag_m_pstate"_h, &VISITORCLASS::VisitSystem},                       \
      {"xpaclri_hi_hints"_h, &VISITORCLASS::VisitSystem},                      \
      {"yield_hi_hints"_h, &VISITORCLASS::VisitSystem},                        \
      {"abs_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                 \
      {"cls_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                 \
      {"clz_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                 \
      {"cmeq_asimdmisc_z"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"cmge_asimdmisc_z"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"cmgt_asimdmisc_z"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"cmle_asimdmisc_z"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"cmlt_asimdmisc_z"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"cnt_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                 \
      {"fabs_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"fcmeq_asimdmisc_fz"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcmge_asimdmisc_fz"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcmgt_asimdmisc_fz"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcmle_asimdmisc_fz"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcmlt_asimdmisc_fz"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtas_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtau_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtl_asimdmisc_l"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"fcvtms_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtmu_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtns_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtnu_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtn_asimdmisc_n"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"fcvtps_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtpu_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtxn_asimdmisc_n"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtzs_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fcvtzu_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"fneg_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"frecpe_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frint32x_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},            \
      {"frint32z_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},            \
      {"frint64x_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},            \
      {"frint64z_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},            \
      {"frinta_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frinti_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frintm_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frintn_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frintp_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frintx_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frintz_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"frsqrte_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},             \
      {"fsqrt_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"neg_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                 \
      {"not_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                 \
      {"rbit_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"rev16_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"rev32_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"rev64_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"sadalp_asimdmisc_p"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"saddlp_asimdmisc_p"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"scvtf_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"shll_asimdmisc_s"_h, &VISITORCLASS::VisitNEON2RegMisc},                \
      {"sqabs_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"sqneg_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"sqxtn_asimdmisc_n"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"sqxtun_asimdmisc_n"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"suqadd_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"uadalp_asimdmisc_p"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"uaddlp_asimdmisc_p"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"ucvtf_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"uqxtn_asimdmisc_n"_h, &VISITORCLASS::VisitNEON2RegMisc},               \
      {"urecpe_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"ursqrte_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},             \
      {"usqadd_asimdmisc_r"_h, &VISITORCLASS::VisitNEON2RegMisc},              \
      {"xtn_asimdmisc_n"_h, &VISITORCLASS::VisitNEON2RegMisc},                 \
      {"mla_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"mls_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"mul_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"saba_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"sabd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"shadd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"shsub_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"smaxp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"smax_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"sminp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"smin_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"srhadd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},              \
      {"uaba_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"uabd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"uhadd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"uhsub_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"umaxp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"umax_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"uminp_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},               \
      {"umin_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"urhadd_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},              \
      {"and_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"bic_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"bif_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"bit_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"bsl_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"eor_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"orr_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"orn_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"pmul_asimdsame_only"_h, &VISITORCLASS::VisitNEON3Same},                \
      {"fmlal2_asimdsame_f"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"fmlal_asimdsame_f"_h, &VISITORCLASS::VisitNEON3Same},                  \
      {"fmlsl2_asimdsame_f"_h, &VISITORCLASS::VisitNEON3Same},                 \
      {"fmlsl_asimdsame_f"_h, &VISITORCLASS::VisitNEON3Same},                  \
      {"pmull_asimddiff_l"_h, &VISITORCLASS::VisitNEON3Different},             \
      {"ushll_asimdshf_l"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"sshll_asimdshf_l"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"shrn_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},           \
      {"rshrn_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"sqshrn_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},         \
      {"sqrshrn_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},        \
      {"sqshrun_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},        \
      {"sqrshrun_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},       \
      {"uqshrn_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},         \
      {"uqrshrn_asimdshf_n"_h, &VISITORCLASS::VisitNEONShiftImmediate},        \
      {"sri_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},            \
      {"srshr_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"srsra_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"sshr_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},           \
      {"ssra_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},           \
      {"urshr_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"ursra_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"ushr_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},           \
      {"usra_asimdshf_r"_h, &VISITORCLASS::VisitNEONShiftImmediate},           \
      {"scvtf_asimdshf_c"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"ucvtf_asimdshf_c"_h, &VISITORCLASS::VisitNEONShiftImmediate},          \
      {"fcvtzs_asimdshf_c"_h, &VISITORCLASS::VisitNEONShiftImmediate},         \
      {"fcvtzu_asimdshf_c"_h, &VISITORCLASS::VisitNEONShiftImmediate},         \
      {"sqdmlal_asisdelem_l"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"sqdmlsl_asisdelem_l"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"sqdmull_asisdelem_l"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"fabd_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"facge_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"facgt_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"fcmeq_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"fcmge_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"fcmgt_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"fmulx_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"frecps_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},        \
      {"frsqrts_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},       \
      {"cmeq_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"cmge_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"cmgt_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"cmhi_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"cmhs_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},          \
      {"cmtst_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},         \
      {"add_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},           \
      {"sub_asisdsame_only"_h, &VISITORCLASS::VisitNEONScalar3Same},           \
      {"sqrdmlah_asisdsame2_only"_h,                                           \
       &VISITORCLASS::VisitNEONScalar3SameExtra},                              \
      {"sqrdmlsh_asisdsame2_only"_h,                                           \
       &VISITORCLASS::VisitNEONScalar3SameExtra},                              \
      {"fmaxnmv_asimdall_only_h"_h, &VISITORCLASS::VisitNEONAcrossLanes},      \
      {"fmaxv_asimdall_only_h"_h, &VISITORCLASS::VisitNEONAcrossLanes},        \
      {"fminnmv_asimdall_only_h"_h, &VISITORCLASS::VisitNEONAcrossLanes},      \
      {"fminv_asimdall_only_h"_h, &VISITORCLASS::VisitNEONAcrossLanes},        \
      {"fmaxnmv_asimdall_only_sd"_h, &VISITORCLASS::VisitNEONAcrossLanes},     \
      {"fminnmv_asimdall_only_sd"_h, &VISITORCLASS::VisitNEONAcrossLanes},     \
      {"fmaxv_asimdall_only_sd"_h, &VISITORCLASS::VisitNEONAcrossLanes},       \
      {"fminv_asimdall_only_sd"_h, &VISITORCLASS::VisitNEONAcrossLanes},       \
      {"shl_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},      \
      {"sli_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},      \
      {"sri_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},      \
      {"srshr_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"srsra_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"sshr_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},     \
      {"ssra_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},     \
      {"urshr_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"ursra_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},    \
      {"ushr_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},     \
      {"usra_asisdshf_r"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},     \
      {"sqrshrn_asisdshf_n"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},  \
      {"sqrshrun_asisdshf_n"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate}, \
      {"sqshrn_asisdshf_n"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},   \
      {"sqshrun_asisdshf_n"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},  \
      {"uqrshrn_asisdshf_n"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},  \
      {"uqshrn_asisdshf_n"_h, &VISITORCLASS::VisitNEONScalarShiftImmediate},   \
      {"cmeq_asisdmisc_z"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},          \
      {"cmge_asisdmisc_z"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},          \
      {"cmgt_asisdmisc_z"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},          \
      {"cmle_asisdmisc_z"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},          \
      {"cmlt_asisdmisc_z"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},          \
      {"abs_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},           \
      {"neg_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},           \
      {"fcmeq_asisdmisc_fz"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcmge_asisdmisc_fz"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcmgt_asisdmisc_fz"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcmle_asisdmisc_fz"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcmlt_asisdmisc_fz"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtas_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtau_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtms_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtmu_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtns_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtnu_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtps_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtpu_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtxn_asisdmisc_n"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtzs_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"fcvtzu_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"frecpe_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"frecpx_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},        \
      {"frsqrte_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},       \
      {"scvtf_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},         \
      {"ucvtf_asisdmisc_r"_h, &VISITORCLASS::VisitNEONScalar2RegMisc},         \
      {"fmla_asisdelem_rh_h"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"fmla_asisdelem_r_sd"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"fmls_asisdelem_rh_h"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"fmls_asisdelem_r_sd"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"fmulx_asisdelem_rh_h"_h,                                               \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"fmulx_asisdelem_r_sd"_h,                                               \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
      {"fmul_asisdelem_rh_h"_h,                                                \
       &VISITORCLASS::VisitNEONScalarByIndexedElement},                        \
  {                                                                            \
    "fmul_asisdelem_r_sd"_h, &VISITORCLASS::VisitNEONScalarByIndexedElement    \
  }
