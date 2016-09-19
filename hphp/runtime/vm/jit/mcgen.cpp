/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/debug/debug.h"

#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit {

TransEnv::~TransEnv() {}

namespace mcgen {

namespace {

bool checkCachedPrologue(const Func* func, int paramIdx, TCA& prologue) {
  prologue = (TCA)func->getPrologue(paramIdx);
  if (prologue != tc::ustubs().fcallHelperThunk) {
    TRACE(1, "cached prologue %s(%d) -> cached %p\n",
          func->fullName()->data(), paramIdx, prologue);
    assertx(tc::isValidCodeAddress(prologue));
    return true;
  }
  return false;
}

int64_t s_startTime;
bool s_inited{false};

////////////////////////////////////////////////////////////////////////////////
}

TCA getFuncPrologue(Func* func, int nPassed, ActRec* ar, bool forRegenerate) {
  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);
  int const numParams = func->numNonVariadicParams();
  int paramIndex = nPassed <= numParams ? nPassed : numParams + 1;

  // Do a quick test before grabbing the write lease
  TCA prologue;
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(nPassed), false};
  auto computeKind = [&] {
    return tc::profileSrcKey(funcBody) ? TransKind::ProfPrologue :
               forRegenerate           ? TransKind::OptPrologue  :
                                         TransKind::LivePrologue;
  };
  LeaseHolder writer(GetWriteLease(), func, computeKind());
  if (!writer) return nullptr;

  auto const kind = computeKind();
  // Check again now that we have the write lease, in case the answer to
  // profileSrcKey() changed.
  if (!writer.checkKind(kind)) return nullptr;

  // If we're regenerating a prologue, and we want to check shouldTranslate()
  // but ignore the code size limits.  We still want to respect the global
  // translation limit and other restrictions, though.
  if (forRegenerate) {
    if (!tc::shouldTranslateNoSizeLimit(func)) return nullptr;
  } else {
    if (!tc::shouldTranslate(func, TransKind::LivePrologue)) return nullptr;
  }

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  return tc::emitFuncPrologue(func, nPassed, kind);
}

TCA getFuncBody(Func* func) {
  auto tca = func->getFuncBody();
  if (tca != tc::ustubs().funcBodyHelperThunk) return tca;

  LeaseHolder writer(GetWriteLease(), func, TransKind::Profile);
  if (!writer) return nullptr;

  tca = func->getFuncBody();
  if (tca != tc::ustubs().funcBodyHelperThunk) return tca;

  auto const dvs = func->getDVFunclets();
  if (dvs.size()) {
    tca = tc::emitFuncBodyDispatch(func, dvs);
  } else {
    SrcKey sk(func, func->base(), false);
    tca = getTranslation(TransArgs{sk});
    if (tca) func->setFuncBody(tca);
  }

  return tca;
}

TCA getTranslation(const TransArgs& args) {
  if (auto const tca = findTranslation(args)) return tca;

  return createTranslation(args);
}

TCA retranslate(TransArgs args) {
  auto sr = tc::findSrcRec(args.sk);
  always_assert(sr);
  bool locked = sr->tryLock();
  SCOPE_EXIT {
    if (locked) sr->freeLock();
  };
  if (isDebuggerAttachedProcess() && isSrcKeyInDbgBL(args.sk)) {
    // We are about to translate something known to be blacklisted by
    // debugger, exit early
    SKTRACE(1, args.sk, "retranslate abort due to debugger\n");
    return nullptr;
  }

  // We need to recompute the kind after acquiring the write lease in case the
  // answer to profileSrcKey() changes, so use a lambda rather than just
  // storing the result.
  auto kind = [&] {
    return tc::profileSrcKey(args.sk) ? TransKind::Profile : TransKind::Live;
  };
  args.kind = kind();

  // Only start profiling new functions at their entry point. This reduces the
  // chances of profiling the body of a function but not its entry (where we
  // trigger retranslation) and helps remove bias towards larger functions that
  // can cause variations in the size of code.prof.
  if (args.kind == TransKind::Profile &&
      !profData()->profiling(args.sk.funcID()) &&
      !args.sk.func()->isEntry(args.sk.offset())) {
    return nullptr;
  }

  LeaseHolder writer(GetWriteLease(), args.sk.func(), args.kind);
  if (!writer || !tc::shouldTranslate(args.sk.func(), kind())) {
    return nullptr;
  }

  if (!locked) {
    // Even though we knew above that we were going to skip doing another
    // translation, we wait until we get the write lease, to avoid spinning
    // through the tracelet guards again and again while another thread is
    // writing to it.
    return sr->getTopTranslation();
  }
  if (sr->translations().size() > RuntimeOption::EvalJitMaxTranslations) {
    always_assert(sr->translations().size() ==
                  RuntimeOption::EvalJitMaxTranslations + 1);
    return sr->getTopTranslation();
  }
  SKTRACE(1, args.sk, "retranslate\n");

  args.kind = kind();
  if (!writer.checkKind(args.kind)) return nullptr;

  auto result = translate(args);

  tc::checkFreeProfData();
  return result;
}

TCA retranslateOpt(SrcKey sk, TransID transId) {
  if (isDebuggerAttachedProcess()) return nullptr;

  auto const func = const_cast<Func*>(sk.func());
  auto const funcID = func->getFuncId();
  if (profData() == nullptr || profData()->optimized(funcID)) return nullptr;

  LeaseHolder writer(GetWriteLease(), func, TransKind::Optimize);
  if (!writer) return nullptr;

  if (profData()->optimized(funcID)) return nullptr;
  profData()->setOptimized(funcID);

  TRACE(1, "retranslateOpt: transId = %u\n", transId);
  func->setFuncBody(tc::ustubs().funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  tc::invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  bool includedBody{false};
  TCA start = regeneratePrologues(func, sk, includedBody);

  // Regionize func and translate all its regions.
  std::string transCFGAnnot;
  auto const regions = includedBody ? std::vector<RegionDescPtr>{}
                                    : regionizeFunc(func, transCFGAnnot);

  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto transArgs = TransArgs{regionSk};
    if (transCFGAnnot.size() > 0) {
      transArgs.annotations.emplace_back("TransCFG", transCFGAnnot);
    }
    transArgs.region = region;
    transArgs.kind = TransKind::Optimize;

    auto const regionStart = translate(transArgs);
    if (regionStart != nullptr &&
        regionSk.offset() == func->base() &&
        func->getDVFunclets().size() == 0 &&
        func->getFuncBody() == tc::ustubs().funcBodyHelperThunk) {
      func->setFuncBody(regionStart);
    }
    if (start == nullptr && regionSk == sk) {
      start = regionStart;
    }
    transCFGAnnot = ""; // so we don't annotate it again
  }

  tc::checkFreeProfData();
  return start;
}

////////////////////////////////////////////////////////////////////////////////

void processInit() {
  TRACE(1, "mcgen startup\n");

  g_unwind_rds.bind();

  Debug::initDebugInfo();
  tc::processInit();

  if (Trace::moduleEnabledRelease(Trace::printir) &&
      !RuntimeOption::EvalJit) {
    Trace::traceRelease("TRACE=printir is set but the jit isn't on. "
                        "Did you mean to run with -vEval.Jit=1?\n");
  }

  s_startTime = HPHP::Timer::GetCurrentTimeMicros();
  initInstrInfo();

  s_inited = true;
}

bool initialized() { return s_inited; }

int64_t jitInitTime() { return s_startTime; }

bool dumpTCAnnotation(const Func& func, TransKind transKind) {
  return RuntimeOption::EvalDumpTCAnnotationsForAllTrans ||
    (transKind == TransKind::Optimize && (func.attrs() & AttrHot));
}

}}}
