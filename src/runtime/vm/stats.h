/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef _STATS_H_
#define _STATS_H_

#include <runtime/vm/hhbc.h>
#include <util/trace.h>

namespace HPHP {
namespace x64 { class X64Assembler; }

namespace VM {
namespace Stats {

extern __thread uint64_t tl_interpInstrs;
extern __thread uint64_t tl_tcInstrs;

#include "stats-opcodeDef.h"

#define STATS \
  STAT(Instr_TC) \
  OPCODES \
  STAT(TgtCache_SPropHit) \
  STAT(TgtCache_SPropMiss) \
  STAT(TgtCache_PropGetFill) \
  STAT(TgtCache_PropGetHit) \
  STAT(TgtCache_PropGetFail) \
  STAT(TgtCache_PropGetMiss) \
  STAT(TgtCache_PropSetFill) \
  STAT(TgtCache_PropSetHit) \
  STAT(TgtCache_PropSetFail) \
  STAT(TgtCache_PropSetMiss) \
  STAT(TgtCache_StaticHit) \
  STAT(TgtCache_StaticMiss) \
  STAT(TgtCache_ClsCnsHit) \
  STAT(TgtCache_ClsCnsMiss) \
  STAT(TgtCache_KnownClsHit) \
  STAT(TgtCache_KnownClsMiss) \
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
  STAT(TgtCache_MethodBypass) \
  STAT(TgtCache_GlobalHit) \
  STAT(TgtCache_GlobalMiss) \
  STAT(TgtCache_StaticMethodHit) \
  STAT(TgtCache_StaticMethodMiss) \
  STAT(TgtCache_StaticMethodBypass) \
  STAT(TgtCache_StaticMethodFHit) \
  STAT(TgtCache_StaticMethodFMiss) \
  STAT(TgtCache_StaticMethodFBypass) \
  STAT(TgtCache_ClassExistsHit) \
  STAT(TgtCache_ClassExistsMiss) \
  STAT(Tx64_VerifyParamTypeSlow) \
  STAT(Tx64_VerifyParamTypeFast) \
  STAT(Tx64_InstanceOfDSlow) \
  STAT(Tx64_InstanceOfDFast) \
  STAT(Tx64_PropCache) \
  STAT(Tx64_PropNameCache) \
  STAT(Tx64_PropCtxCache) \
  STAT(Tx64_PropCtxNameCache) \
  STAT(Tx64_PropGetFast) \
  STAT(Tx64_PropGetSlow) \
  STAT(Tx64_PropSetFast) \
  STAT(Tx64_PropSetSlow) \
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
  /* Translation cache statistics */ \
  STAT(TC_MissPMain) \
  STAT(TC_MissWriteLease) \
  STAT(TC_Hit) \
  STAT(TC_Sync) \
  /* Fixup */ \
  STAT(Fixup_Find) \
  STAT(Fixup_Probe) \
  /* Execute pseudomain */ \
  STAT(ExecPS_Always) \
  STAT(ExecPS_MergeFailed) \
  STAT(ExecPS_Skipped) \
  STAT(Cont_CreateVerySlow) \
  STAT(Cont_UnpackVerySlow) \
  STAT(Cont_PackVerySlow) \
  STAT(VMEnter) \
  STAT(TraceletGuard_enter) \
  STAT(TraceletGuard_branch) \
  STAT(TraceletGuard_execute) \

enum StatCounter {
#define STAT(name) \
  name,
  STATS
#undef STAT
  kNumStatCounters
};
#undef O

extern __thread uint64_t tl_counters[kNumStatCounters];

extern __thread uint64_t tl_helper_counters[];
extern const char* volatile helperNames[];

static inline bool enabled() {
  return Trace::moduleEnabled(Trace::stats, 1);
}

static inline bool enableInstrCount() {
  return Trace::moduleEnabled(Trace::stats, 2);
}

static inline void inc(StatCounter stat, int n = 1) {
  if (Trace::moduleEnabled(Trace::stats, 1)) {
    tl_counters[stat] += n;
  }
}

static inline StatCounter opcodeToStatCounter(Opcode opc) {
  ASSERT(OpLowInvalid == 0);
  return StatCounter(Instr_InterpBBLowInvalid + STATS_PER_OPCODE * opc);
}

static inline void incOp(Opcode opc) {
  inc(opcodeToStatCounter(opc));
}

static inline StatCounter opcodeToTranslStatCounter(Opcode opc) {
  ASSERT(OpLowInvalid == 0);
  return StatCounter(Instr_TranslLowInvalid + STATS_PER_OPCODE * opc);
}

// Both emitIncs use r10.
extern void emitInc(x64::X64Assembler &a, StatCounter stat, int n = 1);
extern void emitInc(x64::X64Assembler& a,
                    uint64_t* tl_table,
                    uint index,
                    int n = 1);
extern void emitIncTranslOp(x64::X64Assembler& a, Opcode opc);
extern void dump();
extern void clear();

} } }

#endif
