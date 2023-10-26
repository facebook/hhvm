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

#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/code-coverage-util.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include <atomic>
#include <cstdint>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Warmup/profiling.
 *
 * In cli mode, we only record samples if we're in recording to replay later.
 *
 * In server mode, we exclude warmup document requests from profiling.
 */

RDS_LOCAL_NO_CHECK(TypeProfileLocals, rl_typeProfileLocals)
  {TypeProfileLocals{}};

namespace {

bool warmingUp;
std::atomic<uint64_t> numRequests;

}

ProfileNonVMThread::ProfileNonVMThread() {
  m_saved = rl_typeProfileLocals->nonVMThread;
  rl_typeProfileLocals->nonVMThread = true;
}

ProfileNonVMThread::~ProfileNonVMThread() {
  rl_typeProfileLocals->nonVMThread =  m_saved;
}

void profileWarmupStart() {
  warmingUp = true;
}

void profileWarmupEnd() {
  warmingUp = false;
}

uint64_t requestCount() {
  return numRequests.load(std::memory_order_relaxed);
}

static inline RequestKind getRequestKind() {
  if (rl_typeProfileLocals->nonVMThread) return RequestKind::NonVM;
  if (warmingUp) return RequestKind::Warmup;
  return RequestKind::Standard;
}

void profileRequestStart() {
  rl_typeProfileLocals->requestKind = getRequestKind();

  auto const codeCoverageForceInterp = []{
    if (RuntimeOption::EvalEnableCodeCoverage > 1) return true;
    if (RuntimeOption::EvalEnableCodeCoverage == 1) {
      if (RuntimeOption::RepoAuthoritative) return false;
      return isEnableCodeCoverageReqParamTrue();
    }
    return false;
  }();

  // Force the request to use interpreter (not even running jitted code) during
  // retranslateAll when we need to dump out precise profile data.
  auto const forceInterp = (jit::mcgen::pendingRetranslateAllScheduled() &&
                            RuntimeOption::DumpPreciseProfData) ||
                           codeCoverageForceInterp;
  bool okToJit = !forceInterp && isStandardRequest();
  if (!RequestInfo::s_requestInfo.isNull()) {
    if (RID().isJittingDisabled()) {
      okToJit = false;
    } else if (!okToJit) {
      RID().setJittingDisabled(true);
    }
  }
  jit::setMayAcquireLease(okToJit);

  // Force interpretation if needed.
  if (rl_typeProfileLocals->forceInterpret != forceInterp) {
    rl_typeProfileLocals->forceInterpret = forceInterp;
    if (!RequestInfo::s_requestInfo.isNull()) {
      RID().updateJit();
    }
  }
}

void profileRequestEnd() {
  if (!isStandardRequest()) return;
  numRequests.fetch_add(1, std::memory_order_relaxed);
  static auto const requestSeries = ServiceData::createTimeSeries(
    "vm.requests",
    {ServiceData::StatsType::RATE, ServiceData::StatsType::SUM},
    {std::chrono::seconds(60), std::chrono::seconds(0)}
  );
  requestSeries->addValue(1);
}

}
