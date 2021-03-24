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

#include "hphp/runtime/vm/jit/service-request-handlers.h"

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-record.h"

#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/vixl/a64/decoder-a64.h"

#include "hphp/util/arch.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

TCA bindJmp(TCA toSmash, SrcKey destSk, bool& smashed) {
  auto const sr = srcDB().find(destSk);
  always_assert(sr);
  if (sr->getTopTranslation() == nullptr) return nullptr;

  auto srLock = sr->writelock();

  // Check again now that we have the lock
  auto const tDest = sr->getTopTranslation();
  if (tDest == nullptr) return nullptr;

  auto const isJcc = [&] {
    switch (arch()) {
      case Arch::X64: {
        x64::DecodedInstruction di(toSmash);
        return (di.isBranch() && !di.isJmp());
      }

      case Arch::ARM: {
        auto instr = reinterpret_cast<vixl::Instruction*>(toSmash);
        return instr->IsCondBranchImm();
      }

    }
    not_reached();
  }();

  // Return if already smashed.  Note that smashableXxTarget returns nullptr
  // when the target was smashed if the XX was able to be optimized in place
  // (so that it doesn't look like a smashable XX anymore).
  if (isJcc) {
    auto const target = smashableJccTarget(toSmash);
    if (target == nullptr || target == tDest) return tDest;
    sr->chainFrom(IncomingBranch::jccFrom(toSmash));
  } else {
    auto const target = smashableJmpTarget(toSmash);
    if (target == nullptr || target == tDest) return tDest;
    sr->chainFrom(IncomingBranch::jmpFrom(toSmash));
  }

  smashed = true;
  return tDest;
}

TCA bindJmpToStub(TCA toSmash, TCA oldTarget, TCA stub,
                  SrcKey destSk, bool& smashed) {
  auto const sr = srcDB().find(destSk);
  always_assert(sr);
  auto srLock = sr->writelock();

  auto const isJcc = [&] {
    switch (arch()) {
      case Arch::X64: {
        x64::DecodedInstruction di(toSmash);
        return (di.isBranch() && !di.isJmp());
      }

      case Arch::ARM: {
        auto instr = reinterpret_cast<vixl::Instruction*>(toSmash);
        return instr->IsCondBranchImm();
      }

    }
    not_reached();
  }();

  // Return if already smashed.  Note that smashableXxTarget returns nullptr
  // when the target was smashed if the XX was able to be optimized in place
  // (so that it doesn't look like a smashable XX anymore).
  if (isJcc) {
    auto const target = smashableJccTarget(toSmash);
    if (!target || target != oldTarget) return nullptr;
    smashJcc(toSmash, stub);
  } else {
    auto const target = smashableJmpTarget(toSmash);
    if (!target || target != oldTarget) return nullptr;
    smashJmp(toSmash, stub);
  }

  smashed = true;
  return stub;
}

TCA bindAddr(TCA toSmash, SrcKey destSk, bool& smashed) {
  auto const sr = srcDB().find(destSk);
  always_assert(sr);
  if (sr->getTopTranslation() == nullptr) return nullptr;

  auto srLock = sr->writelock();

  // Check again now that we have the lock
  auto const tDest = sr->getTopTranslation();
  if (tDest == nullptr) return nullptr;

  auto addr = reinterpret_cast<TCA*>(toSmash);
  if (*addr == tDest) {
    // Already smashed
    return tDest;
  }
  sr->chainFrom(IncomingBranch::addr(addr));
  smashed = true;
  return tDest;
}

TCA bindAddrToStub(TCA toSmash, TCA oldTarget, TCA stub,
                   SrcKey destSk, bool& smashed) {
  auto const sr = srcDB().find(destSk);
  always_assert(sr);
  auto srLock = sr->writelock();

  auto addr = reinterpret_cast<TCA*>(toSmash);
  assert_address_is_atomically_accessible(addr);
  if (*addr != oldTarget) return nullptr;
  *addr = stub;
  smashed = true;
  return stub;
}

void bindCall(TCA toSmash, TCA start, Func* callee, int nArgs) {
  // Return if already smashed.  Note that smashableCallTarget returns nullptr
  // when the target was smashed if the call was able to be optimized in place
  // (so that it doesn't look like a smashable call anymore).
  if (start == nullptr || !smashableCallTarget(toSmash) ||
      smashableCallTarget(toSmash) == start) {
    return;
  }

  // For functions to be PGO'ed, if their current prologues are still
  // profiling ones (living in code.prof()), then save toSmash as a
  // caller to the prologue, so that it can later be smashed to call a
  // new prologue when it's generated.
  int calleeNumParams = callee->numNonVariadicParams();
  int calledPrologNumArgs = (nArgs <= calleeNumParams ?
                             nArgs :  calleeNumParams + 1);
  auto const profData = jit::profData();

  // Ensure that the smash is performed while holding the lock, this way if
  // the list of prologue callers is resmashed/cleared there won't be a race.
  // Right now we never reset non-profiled prologues so we only need the lock
  // if the prologue being smashed is in Aprof.
  ProfTransRec* rec{nullptr};
  std::unique_lock<Mutex> recLock;
  if (profData && code().prof().contains(start)) {
    rec = profData->prologueTransRec(callee, calledPrologNumArgs);
    recLock = rec->lockCallerList();

    // It's possible that the callee prologue was reset before we acquired the
    // lock. Make sure we have the right one.
    auto const trans = mcgen::getFuncPrologue(callee, nArgs);
    start = trans.isProcessPersistentFailure()
      ? tc::ustubs().fcallHelperNoTranslateThunk
      : trans.addr();

    // Do these checks again with the lock.
    if (start == nullptr || !smashableCallTarget(toSmash) ||
        smashableCallTarget(toSmash) == start) {
      return;
    }

    if (code().prof().contains(start)) {
      rec->addMainCaller(toSmash);
    } else {
      rec = nullptr;
      recLock.unlock();
    }
  }

  assertx(smashableCallTarget(toSmash));
  TRACE(2, "bindCall smash %p -> %p\n", toSmash, start);
  smashCall(toSmash, start);

  // We need to be able to reclaim the function prologues once the unit
  // associated with this function is treadmilled-- so record all of the
  // callers that will need to be re-smashed
  //
  // Additionally for profiled calls we need to remove them from the main
  // caller map.
  if (RuntimeOption::EvalEnableReusableTC) {
    if (debug || rec) {
      recordFuncCaller(callee, toSmash, rec);
    }
  }
}

}}}
