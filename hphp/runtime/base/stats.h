/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_STATS_H_
#define incl_HPHP_STATS_H_

#include <atomic>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/util/data-block.h"
#include "hphp/util/trace.h"

namespace HPHP {
namespace Stats {

#include "hphp/runtime/vm/stats-opcodeDef.h"

#define STATS \
  STAT(Instr_TC) \
  OPCODES \
  STAT(TgtCache_SPropHit) \
  STAT(TgtCache_SPropMiss) \
  STAT(TgtCache_StaticHit) \
  STAT(TgtCache_StaticMiss) \
  STAT(TgtCache_ClsCnsHit) \
  STAT(TgtCache_ClsCnsMiss) \
  STAT(TgtCache_FuncDHit) \
  STAT(TgtCache_FuncDMiss) \
  STAT(TgtCache_CtorDHit) \
  STAT(TgtCache_CtorDMiss) \
  STAT(TgtCache_CallHit) \
  STAT(TgtCache_CallMiss) \
  STAT(TgtCache_FixedCallHit) \
  STAT(TgtCache_FixedCallMiss) \
  STAT(TgtCache_MethodHit) \
  STAT(TgtCache_MethodMiss) \
  STAT(TgtCache_MethodFast) \
  STAT(TgtCache_MethodBypass) \
  STAT(TgtCache_GlobalHit) \
  STAT(TgtCache_GlobalMiss) \
  STAT(TgtCache_StaticMethodHit) \
  STAT(TgtCache_StaticMethodMiss) \
  STAT(TgtCache_StaticMethodBypass) \
  STAT(TgtCache_StaticMethodFHit) \
  STAT(TgtCache_StaticMethodFMiss) \
  STAT(TgtCache_StaticMethodFBypass) \
  STAT(TgtCache_StaticMethodFFill) \
  STAT(TgtCache_ClassExistsHit) \
  STAT(TgtCache_ClassExistsMiss) \
  STAT(MCG_FusedTypeCheck) \
  STAT(MCG_UnfusedTypeCheck) \
  STAT(MCG_VerifyParamTypeSlow) \
  STAT(MCG_VerifyParamTypeFast) \
  STAT(MCG_VerifyParamTypeBit) \
  STAT(MCG_VerifyParamTypeSlowShortcut) \
  STAT(MCG_VerifyParamTypePass) \
  STAT(MCG_VerifyParamTypeEqual) \
  STAT(MCG_InstanceOfDFused) \
  STAT(MCG_InstanceOfDBypass) \
  STAT(MCG_InstanceOfDInterface) \
  STAT(MCG_InstanceOfDSlow) \
  STAT(MCG_InstanceOfDFast) \
  STAT(MCG_InstanceOfDBit) \
  STAT(MCG_InstanceOfDEqual) \
  STAT(MCG_InstanceOfDFinalTrue) \
  STAT(MCG_InstanceOfDFinalFalse) \
  STAT(MCG_CGetMLEE) \
  STAT(MCG_CGetMGE) \
  STAT(MCG_CGetMArray) \
  STAT(MCG_CGetMGeneric) \
  STAT(MCG_MLitKey) \
  STAT(MCG_MRegKey) \
  STAT(MCG_MTVKey) \
  STAT(MCG_CnsFast) \
  STAT(MCG_CnsSlow) \
  STAT(MCG_ContCreateFast) \
  STAT(MCG_ContCreateSlow) \
  STAT(MCG_ContUnpackFast) \
  STAT(MCG_ContUnpackSlow) \
  STAT(MCG_ContPackFast) \
  STAT(MCG_ContPackSlow) \
  STAT(MCG_Spill) \
  STAT(MCG_SpillHome) \
  STAT(MCG_ClassExistsFast) \
  STAT(MCG_ClassExistsSlow) \
  STAT(MCG_StaticLocFast) \
  STAT(MCG_StaticLocSlow) \
  STAT(MCG_OneGuardShort) \
  STAT(MCG_OneGuardLong) \
  STAT(MCG_SideExit) \
  STAT(MCG_SideExitClean) \
  STAT(MCG_NewInstancePropCheck) \
  STAT(MCG_NewInstancePropInit) \
  STAT(MCG_NewInstanceSPropCheck) \
  STAT(MCG_NewInstanceSPropInit) \
  STAT(MCG_NewInstanceNoCtorFast) \
  STAT(MCG_NewInstanceNoCtor) \
  STAT(MCG_NewInstanceFast) \
  STAT(MCG_NewInstanceGeneric) \
  STAT(MCG_StringSwitchSlow) \
  STAT(MCG_StringSwitchFast) \
  STAT(MCG_StringSwitchHit) \
  STAT(MCG_StringSwitchChain) \
  STAT(MCG_StringSwitchFailFast) \
  STAT(MCG_StringSwitchFailSlow) \
  /* Type prediction stats */ \
  STAT(TypePred_Insert) \
  STAT(TypePred_Evict) \
  STAT(TypePred_Hit) \
  STAT(TypePred_Miss) \
  STAT(TypePred_MissTooFew) \
  STAT(TypePred_MissTooWeak) \
  /* Translation cache statistics */ \
  STAT(TC_MissPMain) \
  STAT(TC_MissWriteLease) \
  STAT(TC_Hit) \
  STAT(TC_Sync) \
  STAT(TC_SyncUnwind) \
  STAT(TC_TypePredHit) \
  STAT(TC_TypePredMiss) \
  STAT(TC_TypePredUnneeded) \
  STAT(TC_TypePredOverridden) \
  STAT(TC_CatchTrace) \
  STAT(TC_CatchSideExit) \
  STAT(TC_SetMStrGuess_Hit) \
  STAT(TC_SetMStrGuess_Miss) \
  /* Fixup */ \
  STAT(Fixup_Find) \
  STAT(Fixup_Probe) \
  /* Execute pseudomain */ \
  STAT(PseudoMain_Reentered) \
  STAT(PseudoMain_Executed) \
  STAT(PseudoMain_Skipped) \
  STAT(PseudoMain_SkipDeep) \
  STAT(PseudoMain_Guarded) \
  STAT(Cont_CreateVerySlow) \
  STAT(Cont_UnpackVerySlow) \
  STAT(Cont_PackVerySlow) \
  STAT(VMEnter) \
  STAT(TraceletGuard_enter) \
  STAT(TraceletGuard_branch) \
  STAT(TraceletGuard_execute) \
  /* Unit merging stats */ \
  STAT(UnitMerge_hoistable) \
  STAT(UnitMerge_hoistable_persistent) \
  STAT(UnitMerge_hoistable_persistent_cache) \
  STAT(UnitMerge_hoistable_persistent_parent) \
  STAT(UnitMerge_hoistable_persistent_parent_cache) \
  STAT(UnitMerge_mergeable) \
  STAT(UnitMerge_mergeable_unique) \
  STAT(UnitMerge_mergeable_unique_persistent) \
  STAT(UnitMerge_mergeable_unique_persistent_cache) \
  STAT(UnitMerge_mergeable_define) \
  STAT(UnitMerge_mergeable_global) \
  STAT(UnitMerge_mergeable_class) \
  STAT(UnitMerge_mergeable_require) \
  STAT(UnitMerge_redo_hoistable) \
  /* property getter stats */ \
  STAT(PropAsm_Generic) \
  STAT(PropAsm_Specialized) \
  STAT(PropAsm_GenFinal) \
  /* astubs stats */ \
  STAT(Astubs_New) \
  STAT(Astubs_Reused) \
  /* Switches */ \
  STAT(Switch_Generic) \
  STAT(Switch_Integer) \
  STAT(Switch_String) \
  /* ARM simulator */ \
  STAT(vixl_SimulatedInstr) \
  STAT(vixl_SimulatedLoad) \
  STAT(vixl_SimulatedStore) \

enum StatCounter {
#define STAT(name) \
  name,
  STATS
#undef STAT
  kNumStatCounters
};
#undef O

extern const char* g_counterNames[kNumStatCounters];
extern __thread uint64_t tl_counters[kNumStatCounters];

extern __thread uint64_t tl_helper_counters[];
extern std::atomic<const char*> helperNames[];

inline bool enabled() {
  return Trace::moduleEnabled(Trace::stats, 1);
}

inline bool enabledAny() {
  return enabled() || Trace::moduleEnabled(Trace::statgroups);
}

inline bool enableInstrCount() {
  return Trace::moduleEnabled(Trace::stats, 2);
}

inline void inc(StatCounter stat, int n = 1) {
  if (enabled()) {
    tl_counters[stat] += n;
  }
}

static_assert(static_cast<uint64_t>(OpLowInvalid) == 0,
              "stats.h assumes OpLowInvalid == 0");

inline StatCounter opcodeToStatCounter(Op opc) {
  return StatCounter(Instr_InterpBBLowInvalid +
                     STATS_PER_OPCODE * uint8_t(opc));
}

inline void incOp(Op opc) {
  inc(opcodeToStatCounter(opc));
}

inline StatCounter opcodeToTranslStatCounter(Op opc) {
  return StatCounter(Instr_TranslLowInvalid +
                     STATS_PER_OPCODE * uint8_t(opc));
}

inline StatCounter opcodeToIRPreStatCounter(Op opc) {
  return StatCounter(Instr_TranslIRPreLowInvalid +
                     STATS_PER_OPCODE * uint8_t(opc));
}

inline StatCounter opcodeToIRPostStatCounter(Op opc) {
  return StatCounter(Instr_TranslIRPostLowInvalid +
                     STATS_PER_OPCODE * uint8_t(opc));
}

extern void init();
extern void dump();
extern void clear();

void incStatGrouped(const StringData* cat, const StringData* name, int n = 1);

} }

#endif
