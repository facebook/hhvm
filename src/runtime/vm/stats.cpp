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
#include <runtime/vm/stats.h>

namespace HPHP {
namespace VM {
namespace Stats {

TRACE_SET_MOD(stats);

__thread uint64_t tl_counters[kNumStatCounters];
__thread uint64_t tl_helper_counters[kMaxNumTrampolines];

// Only the thread holding the write lease will set the entries in the
// helperNames array but other threads may concurrently read these
// entries, so each entry is volatile (or an atomic type per the new
// C++11 standard).
const char* volatile helperNames[kMaxNumTrampolines];

void
emitInc(Transl::X64Assembler& a,uint64_t* tl_table,uint index,int n) {
  using namespace HPHP::VM::Transl;
  uintptr_t virtualAddress = uintptr_t(&tl_table[index]) - tlsBase();
  emitImmReg(a, virtualAddress, reg::rScratch);
  a.   fs();
  a.   add_imm64_disp_reg64(n, 0, reg::rScratch);
}



void emitInc(Transl::X64Assembler& a, StatCounter stat, int n /* = 1*/) {
  if (!enabled()) return;
  emitInc(a,&tl_counters[0],stat,n);
}

void emitIncTranslOp(Transl::X64Assembler& a, Opcode opc) {
  if (!enableInstrCount()) return;
  emitInc(a, &tl_counters[0], opcodeToTranslStatCounter(opc), 1);
}

static __thread int64 epoch;
void dump() {
  if (!enabled()) return;
  TRACE(1, "STATS %lld %s\n", epoch, g_context->getRequestUrl(50).c_str());
#include "runtime/vm/stats-opcodeDef.h"
#define STAT(s) \
  if (tl_counters[s]) TRACE(1, "STAT %-50s %15ld\n", \
                            #s, tl_counters[s]);
  STATS
#undef STAT
#undef O
  for (int i=0; helperNames[i]; i++) {
    if (tl_helper_counters[i]) {
      TRACE(1, "STAT %-50s %15ld\n",
            helperNames[i],
            tl_helper_counters[i]);
    }
  }
}

void clear() {
  if (!enabled()) return;
  ++epoch;
  memset(&tl_counters[0], 0, sizeof(tl_counters));
  memset(&tl_helper_counters[0], 0, sizeof(tl_helper_counters));
}

} } }
