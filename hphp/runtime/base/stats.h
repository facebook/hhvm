/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/rds-local.h"

namespace HPHP {
namespace Stats {

#include "hphp/runtime/vm/stats-opcodeDef.h"

#define STATS \
  STAT(Instr_TC) \
  OPCODES \
  STAT(TgtCache_StaticMethodHit) \
  STAT(TgtCache_StaticMethodMiss) \
  STAT(TgtCache_StaticMethodFHit) \
  STAT(TgtCache_StaticMethodFMiss) \
  STAT(TgtCache_StaticMethodFFill) \
  /* Translation cache statistics */ \
  STAT(TC_Sync) \
  STAT(TC_SyncUnwind) \
  STAT(TC_CatchTrace) \
  STAT(TC_CatchSideExit) \
  STAT(TC_DecRef_NZ) \
  STAT(TC_DecRef_Profiled_100) \
  STAT(TC_DecRef_Profiled_0) \
  /* Execute pseudomain */ \
  STAT(PseudoMain_Reentered) \
  STAT(PseudoMain_Executed) \
  STAT(PseudoMain_Skipped) \
  STAT(PseudoMain_SkipDeep) \
  STAT(PseudoMain_Guarded) \
  STAT(VMEnter) \
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
  STAT(UnitMerge_mergeable_persistent_define) \
  STAT(UnitMerge_mergeable_global) \
  STAT(UnitMerge_mergeable_class) \
  STAT(UnitMerge_mergeable_require) \
  STAT(UnitMerge_mergeable_typealias) \
  STAT(UnitMerge_redo_hoistable) \
  /* stub reuse stats */ \
  STAT(Astub_New) \
  STAT(Astub_Reused) \
  /* ObjectData construction */ \
  STAT(ObjectData_new_dtor_yes) \
  STAT(ObjectData_new_dtor_no) \
  STAT(ObjMethod_total) \
  STAT(ObjMethod_known) \
  STAT(ObjMethod_methodslot) \
  STAT(ObjMethod_ifaceslot) \
  STAT(ObjMethod_cached) \

enum StatCounter {
#define STAT(name) \
  name,
  STATS
#undef STAT
  kNumStatCounters
};
#undef O

extern const char* g_counterNames[kNumStatCounters];

struct StatCounters {
  uint64_t counters[kNumStatCounters];
};

extern RDS_LOCAL(StatCounters, rl_counters);

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
    rl_counters->counters[stat] += n;
  }
}

inline StatCounter opToTranslStat(Op opc) {
  return StatCounter(STATS_PER_OPCODE * size_t(opc));
}

extern void init();
extern void dump();
extern void clear();

void incStatGrouped(const StringData* cat, const StringData* name, int n = 1);

} }

#endif
