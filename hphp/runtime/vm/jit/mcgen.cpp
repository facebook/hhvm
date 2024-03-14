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

#include "hphp/runtime/vm/jit/mcgen.h"

#include "hphp/runtime/vm/jit/mcgen-prologue.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"

#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/debug/debug.h"

#include "hphp/util/configs/jit.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP::jit {

namespace mcgen {

namespace {

int64_t s_startTime;
bool s_inited{false};

////////////////////////////////////////////////////////////////////////////////
}

void processInit() {
  TRACE(1, "mcgen startup\n");

  g_unwind_rds.bind(rds::Mode::Normal, rds::LinkID{"Unwind"});

  Debug::initDebugInfo();
  tc::processInit();

  if (Trace::moduleEnabledRelease(Trace::printir) &&
      !Cfg::Jit::Enabled) {
    Trace::traceRelease("TRACE=printir is set but the jit isn't on. "
                        "Did you mean to run with -vEval.Jit=1?\n");
  }

  s_startTime = HPHP::Timer::GetCurrentTimeMicros();
  initInstrInfo();

  s_inited = true;
}

bool initialized() { return s_inited; }

int64_t jitInitTime() { return s_startTime; }

bool dumpTCAnnotation(TransKind transKind) {
  if (RuntimeOption::EvalDumpTCAnnotationsForAllTrans) return true;
  if (transKind != TransKind::Optimize && transKind != TransKind::OptPrologue) {
    return false;
  }
  return !isJitSerializing();
}

}}
