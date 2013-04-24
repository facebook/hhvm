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
#include <util/base.h>
#include <runtime/vm/translator/x64-util.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/base/stats.h>

namespace HPHP {
namespace Stats {

using namespace HPHP::VM::Transl;

TRACE_SET_MOD(stats);

const char* g_counterNames[] = {
#include "runtime/vm/stats-opcodeDef.h"
#define STAT(s) #s ,
  STATS
#undef STAT
#undef O
};
__thread uint64_t tl_counters[kNumStatCounters];
__thread uint64_t tl_helper_counters[kMaxNumTrampolines];

typedef hphp_const_char_map<hphp_const_char_map<uint64_t>> StatGroupMap;
__thread StatGroupMap* tl_stat_groups = nullptr;

// Only the thread holding the write lease will set the entries in the
// helperNames array but other threads may concurrently read these
// entries, so each entry is volatile (or an atomic type per the new
// C++11 standard).
const char* volatile helperNames[kMaxNumTrampolines];

void
emitInc(X64Assembler& a, uint64_t* tl_table, uint index, int n,
        ConditionCode cc, bool force) {
  if (!force && !enabled()) return;
  bool havecc = cc != CC_None;
  uintptr_t virtualAddress = uintptr_t(&tl_table[index]) - tlsBase();

  TCA jcc = nullptr;
  if (havecc) {
    jcc = a.code.frontier;
    a.  jcc8  (ccNegate(cc), jcc);
  }
  a.    pushf ();
  a.    push  (reg::rScratch);
  a.    movq  (virtualAddress, reg::rScratch);
  a.    fs();
  a.    addq  (n, *reg::rScratch);
  a.    pop   (reg::rScratch);
  a.    popf  ();
  if (havecc) {
    a.  patchJcc8(jcc, a.code.frontier);
  }
}

void emitIncTranslOp(X64Assembler& a, Opcode opc, bool force) {
  if (!force && !enableInstrCount()) return;
  emitInc(a, &tl_counters[0], opcodeToTranslStatCounter(opc), 1,
          CC_None, force);
}

void init() {
  if (!enabledAny()) return;
  assert(tl_stat_groups == nullptr);
  tl_stat_groups = new StatGroupMap();
}

static __thread int64_t epoch;
void dump() {
  if (!enabledAny()) return;
  auto url = g_context->getRequestUrl(50);
  TRACE(0, "STATS %ld %s\n", epoch, url.c_str());
#include "runtime/vm/stats-opcodeDef.h"
#define STAT(s) \
  if (!tl_counters[s]) {} else                                  \
    TRACE(0, "STAT %-50s %15" PRId64 "\n", #s, tl_counters[s]);
  STATS
#undef STAT
#undef O
  for (int i=0; helperNames[i]; i++) {
    if (tl_helper_counters[i]) {
      TRACE(0, "STAT %-50s %15ld\n",
            helperNames[i],
            tl_helper_counters[i]);
    }
  }

  typedef std::pair<const char*, uint64_t> StatPair;
  for (auto const& group : *tl_stat_groups) {
    std::ostringstream stats;
    auto const& map = group.second;
    uint64_t total = 0, accum = 0;;

    std::vector<StatPair> rows(map.begin(), map.end());
    std::for_each(rows.begin(), rows.end(),
                  [&](const StatPair& p) { total += p.second; });
    auto gt = [](const StatPair& a, const StatPair& b) {
      return a.second > b.second;
    };
    std::sort(rows.begin(), rows.end(), gt);

    stats << folly::format("{:-^80}\n",
                           folly::format(" group {} ",
                                         group.first, url))
          << folly::format("{:>45}   {:>9} {:>8} {:>8}\n",
                           "name", "count", "% total", "accum %");
    for (auto const& row : rows) {
      accum += row.second;
      stats << folly::format("{:>45} {} {:9} {:8.2%} {:8.2%}\n",
                             row.first, ':', row.second,
                             (double)row.second / total, (double)accum / total);
    }
    FTRACE(0, "{}\n", stats.str());
  }
}

void clear() {
  if (!RuntimeOption::EnableInstructionCounts && !enabledAny()) return;
  ++epoch;
  memset(&tl_counters[0], 0, sizeof(tl_counters));
  memset(&tl_helper_counters[0], 0, sizeof(tl_helper_counters));

  assert(tl_stat_groups);
  delete tl_stat_groups;
  tl_stat_groups = nullptr;
}

void incStatGrouped(const StringData* category, const StringData* name, int n) {
  assert(tl_stat_groups);
  (*tl_stat_groups)[category->data()][name->data()] += n;
}

} }
