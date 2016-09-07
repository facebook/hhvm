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

#include "hphp/runtime/vm/jit/mcgen-emit.h"
#include "hphp/runtime/vm/jit/mcgen-prologue.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"

#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace {

bool shouldPGOFunc(const Func& func) {
  if (profData() == nullptr) return false;

  // JITing pseudo-mains requires extra checks that blow the IR.  PGO
  // can significantly increase the size of the regions, so disable it for
  // pseudo-mains (so regions will be just tracelets).
  if (func.isPseudoMain()) return false;

  if (!RuntimeOption::EvalJitPGOHotOnly) return true;
  return func.attrs() & AttrHot;
}

const StaticString
  s_php_errormsg("php_errormsg"),
  s_http_response_header("http_response_header");

bool shouldTranslateNoSizeLimit(const Func* func) {
  // If we've hit Eval.JitGlobalTranslationLimit, then we stop translating.
  if (!mcgen::canTranslate()) {
    return false;
  }

  // Do not translate functions from units marked as interpret-only.
  if (func->unit()->isInterpretOnly()) {
    return false;
  }

  /*
   * We don't support JIT compiling functions that use some super-dynamic php
   * variables.
   */
  if (func->lookupVarId(s_php_errormsg.get()) != -1 ||
      func->lookupVarId(s_http_response_header.get()) != -1) {
    return false;
  }

  return true;
}

bool checkCachedPrologue(const Func* func, int paramIdx, TCA& prologue) {
  prologue = (TCA)func->getPrologue(paramIdx);
  if (prologue != mcg->ustubs().fcallHelperThunk) {
    TRACE(1, "cached prologue %s(%d) -> cached %p\n",
          func->fullName()->data(), paramIdx, prologue);
    assertx(mcg->code().isValidCodeAddress(prologue));
    return true;
  }
  return false;
}

void checkFreeProfData() {
  // In PGO mode, we free all the profiling data once the main code area reaches
  // its maximum usage and either the hot area is also full or all the functions
  // that were profiled have already been optimized.
  //
  // However, we keep the data around indefinitely in a few special modes:
  // * Eval.EnableReusableTC
  // * TC dumping enabled (Eval.DumpTC/DumpIR/etc.)
  if (profData() &&
      !RuntimeOption::EvalEnableReusableTC &&
      mcg->code().main().used() >= CodeCache::AMaxUsage &&
      (!mcg->code().hotEnabled() ||
       profData()->profilingFuncs() == profData()->optimizedFuncs()) &&
      !transdb::enabled()) {
    discardProfData();
  }
}

////////////////////////////////////////////////////////////////////////////////
}

bool profileSrcKey(SrcKey sk) {
  if (!shouldPGOFunc(*sk.func())) return false;
  if (profData()->optimized(sk.funcID())) return false;
  if (profData()->profiling(sk.funcID())) return true;

  // Don't start profiling new functions if the size of either main or
  // prof is already above Eval.JitAMaxUsage and we already filled hot.
  auto tcUsage = std::max(mcg->code().main().used(), mcg->code().prof().used());
  if (tcUsage >= CodeCache::AMaxUsage && !mcg->code().hotEnabled()) {
    return false;
  }

  // We have two knobs to control the number of functions we're allowed to
  // profile: Eval.JitProfileRequests and Eval.JitProfileBCSize. We profile new
  // functions until either of these limits is exceeded. In practice we expect
  // to hit the bytecode size limit first but we keep the request limit around
  // as a safety net.
  if (RuntimeOption::EvalJitProfileBCSize > 0 &&
      profData()->profilingBCSize() >= RuntimeOption::EvalJitProfileBCSize) {
    return false;
  }

  return requestCount() <= RuntimeOption::EvalJitProfileRequests;
}

bool shouldTranslate(const Func* func, TransKind kind) {
  if (!shouldTranslateNoSizeLimit(func)) return false;

  // Otherwise, follow the Eval.JitAMaxUsage limit.  However, we do allow PGO
  // translations past that limit if there's still space in code.hot.
  if (mcg->code().main().used() < CodeCache::AMaxUsage) return true;

  switch (kind) {
    case TransKind::ProfPrologue:
    case TransKind::Profile:
    case TransKind::OptPrologue:
    case TransKind::Optimize:
      return mcg->code().hotEnabled();

    default:
      return false;
  }
}

TCA getFuncPrologue(Func* func, int nPassed, ActRec* ar, bool forRegenerate) {
  using namespace mcgen;

  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);
  int const numParams = func->numNonVariadicParams();
  int paramIndex = nPassed <= numParams ? nPassed : numParams + 1;

  // Do a quick test before grabbing the write lease
  TCA prologue;
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(nPassed), false};
  auto computeKind = [&] {
    return profileSrcKey(funcBody) ? TransKind::ProfPrologue :
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
    if (!shouldTranslateNoSizeLimit(func)) return nullptr;
  } else {
    if (!shouldTranslate(func, TransKind::LivePrologue)) return nullptr;
  }

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  try {
    return emitFuncPrologue(func, nPassed, kind);
  } catch (const DataBlockFull& dbFull) {

    // Fail hard if the block isn't code.hot.
    always_assert_flog(dbFull.name == "hot",
                       "data block = {}\nmessage: {}\n",
                       dbFull.name, dbFull.what());

    // Otherwise, fall back to code.main and retry.
    mcg->code().disableHot();
    try {
      return emitFuncPrologue(func, nPassed, kind);
    } catch (const DataBlockFull& dbFull) {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }
}

TCA getFuncBody(Func* func) {
  auto tca = func->getFuncBody();
  if (tca != mcg->ustubs().funcBodyHelperThunk) return tca;

  LeaseHolder writer(GetWriteLease(), func, TransKind::Profile);
  if (!writer) return nullptr;

  tca = func->getFuncBody();
  if (tca != mcg->ustubs().funcBodyHelperThunk) return tca;

  auto const dvs = func->getDVFunclets();
  if (dvs.size()) {
    auto codeLock = mcg->lockCode();
    tca = genFuncBodyDispatch(func, dvs, mcg->code().view());
    func->setFuncBody(tca);
    if (!RuntimeOption::EvalJitNoGdb) {
      mcg->debugInfo()->recordStub(
        Debug::TCRange(tca, mcg->code().view().main().frontier(), false),
        Debug::lookupFunction(func, false, false, true));
    }
    if (RuntimeOption::EvalJitUseVtuneAPI) {
      reportHelperToVtune(func->fullName()->data(),
                          tca,
                          mcg->code().view().main().frontier());
    }
    if (RuntimeOption::EvalPerfPidMap) {
      mcg->debugInfo()->recordPerfMap(
        Debug::TCRange(tca, mcg->code().view().main().frontier(), false),
        SrcKey{}, func, false, false);
    }
  } else {
    SrcKey sk(func, func->base(), false);
    tca = getTranslation(TransArgs{sk});
    if (tca) func->setFuncBody(tca);
  }

  return tca;
}

TCA getTranslation(const TransArgs& args) {
  if (auto const tca = mcgen::findTranslation(args)) return tca;

  return mcgen::createTranslation(args);
}

TCA retranslate(TransArgs args) {
  auto sr = mcg->srcDB().find(args.sk);
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
    return profileSrcKey(args.sk) ? TransKind::Profile : TransKind::Live;
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
  if (!writer || !shouldTranslate(args.sk.func(), kind())) {
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

  auto result = mcgen::translate(args);

  checkFreeProfData();
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
  func->setFuncBody(mcg->ustubs().funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  mcgen::invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  bool includedBody{false};
  TCA start = mcgen::regeneratePrologues(func, sk, includedBody);

  // Regionize func and translate all its regions.
  std::string transCFGAnnot;
  auto const regions = includedBody ? std::vector<RegionDescPtr>{}
                                    : regionizeFunc(func, mcg, transCFGAnnot);

  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto transArgs = TransArgs{regionSk};
    if (transCFGAnnot.size() > 0) {
      transArgs.annotations.emplace_back("TransCFG", transCFGAnnot);
    }
    transArgs.region = region;
    transArgs.kind = TransKind::Optimize;

    auto const regionStart = mcgen::translate(transArgs);
    if (regionStart != nullptr &&
        regionSk.offset() == func->base() &&
        func->getDVFunclets().size() == 0 &&
        func->getFuncBody() == mcg->ustubs().funcBodyHelperThunk) {
      func->setFuncBody(regionStart);
    }
    if (start == nullptr && regionSk == sk) {
      start = regionStart;
    }
    transCFGAnnot = ""; // so we don't annotate it again
  }

  checkFreeProfData();
  return start;
}

////////////////////////////////////////////////////////////////////////////////
namespace { int64_t s_startTime; }
namespace mcgen {
void processInit() {
  s_startTime = HPHP::Timer::GetCurrentTimeMicros();
  initInstrInfo();
}
}

int64_t jitInitTime() { return s_startTime; }

}}
