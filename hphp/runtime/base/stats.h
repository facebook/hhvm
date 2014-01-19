/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
  STAT(Tx64_FusedTypeCheck) \
  STAT(Tx64_UnfusedTypeCheck) \
  STAT(Tx64_VerifyParamTypeSlow) \
  STAT(Tx64_VerifyParamTypeFast) \
  STAT(Tx64_VerifyParamTypeBit) \
  STAT(Tx64_VerifyParamTypeSlowShortcut) \
  STAT(Tx64_VerifyParamTypePass) \
  STAT(Tx64_VerifyParamTypeEqual) \
  STAT(Tx64_InstanceOfDFused) \
  STAT(Tx64_InstanceOfDBypass) \
  STAT(Tx64_InstanceOfDInterface) \
  STAT(Tx64_InstanceOfDSlow) \
  STAT(Tx64_InstanceOfDFast) \
  STAT(Tx64_InstanceOfDBit) \
  STAT(Tx64_InstanceOfDEqual) \
  STAT(Tx64_InstanceOfDFinalTrue) \
  STAT(Tx64_InstanceOfDFinalFalse) \
  STAT(Tx64_CGetMLEE) \
  STAT(Tx64_CGetMGE) \
  STAT(Tx64_CGetMArray) \
  STAT(Tx64_CGetMGeneric) \
  STAT(Tx64_MLitKey) \
  STAT(Tx64_MRegKey) \
  STAT(Tx64_MTVKey) \
  STAT(Tx64_CnsFast) \
  STAT(Tx64_CnsSlow) \
  STAT(Tx64_ContCreateFast) \
  STAT(Tx64_ContCreateSlow) \
  STAT(Tx64_ContUnpackFast) \
  STAT(Tx64_ContUnpackSlow) \
  STAT(Tx64_ContPackFast) \
  STAT(Tx64_ContPackSlow) \
  STAT(Tx64_Spill) \
  STAT(Tx64_SpillHome) \
  STAT(Tx64_ClassExistsFast) \
  STAT(Tx64_ClassExistsSlow) \
  STAT(Tx64_StaticLocFast) \
  STAT(Tx64_StaticLocSlow) \
  STAT(Tx64_OneGuardShort) \
  STAT(Tx64_OneGuardLong) \
  STAT(Tx64_SideExit) \
  STAT(Tx64_SideExitClean) \
  STAT(Tx64_NewInstancePropCheck) \
  STAT(Tx64_NewInstancePropInit) \
  STAT(Tx64_NewInstanceSPropCheck) \
  STAT(Tx64_NewInstanceSPropInit) \
  STAT(Tx64_NewInstanceNoCtorFast) \
  STAT(Tx64_NewInstanceNoCtor) \
  STAT(Tx64_NewInstanceFast) \
  STAT(Tx64_NewInstanceGeneric) \
  STAT(Tx64_StringSwitchSlow) \
  STAT(Tx64_StringSwitchFast) \
  STAT(Tx64_StringSwitchHit) \
  STAT(Tx64_StringSwitchChain) \
  STAT(Tx64_StringSwitchFailFast) \
  STAT(Tx64_StringSwitchFailSlow) \
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

// Both emitIncs use r10.
extern void emitInc(CodeBlock& cb,
                    uint64_t* tl_table,
                    uint index,
                    int n = 1,
                    bool force = false);
inline void emitInc(CodeBlock& cb, StatCounter stat, int n = 1,
                    bool force = false) {
  emitInc(cb, &tl_counters[0], stat, n, force);
}

extern void init();
extern void dump();
extern void clear();

void incStatGrouped(const StringData* cat, const StringData* name, int n = 1);

} }

#endif
