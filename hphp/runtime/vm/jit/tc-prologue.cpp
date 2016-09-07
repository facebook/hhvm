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

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/tc-region.h"
#include "hphp/runtime/vm/jit/tc-relocate.h"

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

folly::Optional<std::pair<SrcKey,TransID>>
updateFuncPrologue(TCA start, ProfTransRec* rec) {
  auto func = rec->func();
  auto nArgs = rec->prologueArgs();

  auto codeLock = lockCode();

  // Smash callers of the old prologue with the address of the new one.
  for (auto toSmash : rec->mainCallers()) {
    smashCall(toSmash, start);
  }

  // If the prologue has a matching guard, then smash its guard-callers as
  // well.
  auto const guard = funcGuardFromPrologue(start, func);
  if (funcGuardMatches(guard, func)) {
    for (auto toSmash : rec->guardCallers()) {
      smashCall(toSmash, guard);
    }
  }
  rec->clearAllCallers();

  // If this prologue has a DV funclet, then invalidate it and return its SrcKey
  // and TransID
  if (nArgs < func->numNonVariadicParams()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      SrcKey funcletSK(func, paramInfo.funcletOff, false);
      auto funcletTransId = profData()->dvFuncletTransId(func, nArgs);
      if (funcletTransId != kInvalidTransID) {
        invalidateSrcKey(funcletSK);
        return std::make_pair(funcletSK, funcletTransId);
      }
    }
  }

  return folly::none;
}

static TCA emitFuncPrologueImpl(Func* func, int argc, TransKind kind) {
  if (!newTranslation()) {
    return nullptr;
  }

  const int nparams = func->numNonVariadicParams();
  const int paramIndex = argc <= nparams ? argc : nparams + 1;

  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(argc), false};

  profileSetHotFuncAttr();
  auto codeLock = lockCode();
  auto codeView = code().view(kind);
  TCA mainOrig = codeView.main().frontier();
  CGMeta fixups;

  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  align(codeView.main(), &fixups, Alignment::CacheLineRoundUp,
        AlignContext::Dead);

  TransLocMaker maker(codeView);
  maker.markStart();

  // Careful: this isn't necessarily the real entry point. For funcIsMagic
  // prologues, this is just a possible prologue.
  TCA aStart = codeView.main().frontier();

  // Give the prologue a TransID if we have profiling data.
  auto const transID = [&]{
    if (kind == TransKind::ProfPrologue) {
      auto const profData = jit::profData();
      auto const id = profData->allocTransID();
      profData->addTransProfPrologue(id, funcBody, paramIndex);
      return id;
    }
    if (profData() && transdb::enabled()) {
      return profData()->allocTransID();
    }
    return kInvalidTransID;
  }();

  TCA start = genFuncPrologue(transID, kind, func, argc, codeView, fixups);

  auto loc = maker.markEnd();
  auto metaLock = lockMetadata();

  if (RuntimeOption::EvalEnableReusableTC) {
    TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
               cs = loc.coldStart(), ce = loc.coldEnd(),
               fs = loc.frozenStart(), fe = loc.frozenEnd(),
               oldStart = start;

    auto const did_relocate = relocateNewTranslation(loc, codeView, fixups,
                                                     &start);

    if (did_relocate) {
      FTRACE_MOD(Trace::reusetc, 1,
                 "Relocated prologue for func {} (id = {}) "
                 "from M[{}, {}], C[{}, {}], F[{}, {}] to M[{}, {}] "
                 "C[{}, {}] F[{}, {}] orig start @ {} new start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, loc.mainStart(), loc.mainEnd(),
                 loc.coldStart(), loc.coldEnd(), loc.frozenStart(),
                 loc.frozenEnd(), oldStart, start);
    } else {
      FTRACE_MOD(Trace::reusetc, 1,
                 "Created prologue for func {} (id = {}) at "
                 "M[{}, {}], C[{}, {}], F[{}, {}] start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, oldStart);
    }

    recordFuncPrologue(func, loc);
    if (loc.mainStart() != aStart) {
      codeView.main().setFrontier(mainOrig); // we may have shifted to align
    }
  }
  if (RuntimeOption::EvalPerfRelocate) {
    GrowableVector<IncomingBranch> incomingBranches;
    recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                       loc.coldCodeStart(), loc.coldEnd(),
                       funcBody, paramIndex,
                       incomingBranches,
                       fixups);
  }
  fixups.process(nullptr);

  assertx(funcGuardMatches(funcGuardFromPrologue(start, func), func));
  assertx(code().isValidCodeAddress(start));

  TRACE(2, "funcPrologue %s(%d) setting prologue %p\n",
        func->fullName()->data(), argc, start);
  func->setPrologue(paramIndex, start);

  assertx(kind == TransKind::LivePrologue ||
          kind == TransKind::ProfPrologue ||
          kind == TransKind::OptPrologue);

  auto tr = maker.rec(funcBody, transID, kind);
  transdb::addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(func->unit(), func, tr);
  }


  recordGdbTranslation(funcBody, func, codeView.main(), loc.mainStart(),
                       false, true);
  recordBCInstr(OpFuncPrologue, loc.mainStart(), loc.mainEnd(), false);

  return start;
}

TCA emitFuncPrologue(Func* func, int argc, TransKind kind) {
  try {
    return emitFuncPrologueImpl(func, argc, kind);
  } catch (const DataBlockFull& dbFull) {

    // Fail hard if the block isn't code.hot.
    always_assert_flog(dbFull.name == "hot",
                       "data block = {}\nmessage: {}\n",
                       dbFull.name, dbFull.what());

    // Otherwise, fall back to code.main and retry.
    code().disableHot();
    try {
      return emitFuncPrologueImpl(func, argc, kind);
    } catch (const DataBlockFull& dbFull) {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }
}

TCA emitFuncBodyDispatch(Func* func, const DVFuncletsVec& dvs) {
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  auto const& view = code().view();
  auto const tca = genFuncBodyDispatch(func, dvs, view);

  func->setFuncBody(tca);
  if (!RuntimeOption::EvalJitNoGdb) {
    Debug::DebugInfo::Get()->recordStub(
      Debug::TCRange(tca, view.main().frontier(), false),
      Debug::lookupFunction(func, false, false, true));
  }
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportHelperToVtune(func->fullName()->data(),
                        tca,
                        view.main().frontier());
  }
  if (RuntimeOption::EvalPerfPidMap) {
    Debug::DebugInfo::Get()->recordPerfMap(
      Debug::TCRange(tca, view.main().frontier(), false),
      SrcKey{}, func, false, false);
  }

  return tca;
}

}}}
