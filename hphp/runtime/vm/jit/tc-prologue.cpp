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
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

namespace {

void emitFuncPrologueImpl(Func* func, int argc, TransKind kind,
                          PrologueMetaInfo& info) {
  assertx(isPrologue(kind));

  if (!newTranslation()) {
    info.start = nullptr;
    return;
  }

  auto& transID = info.transID;
  auto&     loc = info.loc;
  auto&  fixups = info.meta;
  const int nparams = func->numNonVariadicParams();
  const int paramIndex = argc <= nparams ? argc : nparams + 1;

  auto const funcBody =
    SrcKey{func, func->getEntryForNumArgs(argc), SrcKey::PrologueTag{}};

  profileSetHotFunc();
  auto codeView = code().view(kind);
  TCA mainOrig = codeView.main().frontier();

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
  transID = [&]{
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

  info.start = genFuncPrologue(transID, kind, func, argc, codeView, fixups);

  loc = maker.markEnd().loc();

  if (RuntimeOption::EvalEnableReusableTC) {
    TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
               cs = loc.coldStart(), ce = loc.coldEnd(),
               fs = loc.frozenStart(), fe = loc.frozenEnd(),
               oldStart = info.start;

    auto const did_relocate = relocateNewTranslation(loc, codeView, fixups,
                                                     &info.start);

    if (did_relocate) {
      FTRACE_MOD(Trace::reusetc, 1,
                 "Relocated prologue for func {} (id = {}) "
                 "from M[{}, {}], C[{}, {}], F[{}, {}] to M[{}, {}] "
                 "C[{}, {}] F[{}, {}] orig start @ {} new start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, loc.mainStart(), loc.mainEnd(),
                 loc.coldStart(), loc.coldEnd(), loc.frozenStart(),
                 loc.frozenEnd(), oldStart, info.start);
    } else {
      FTRACE_MOD(Trace::reusetc, 1,
                 "Created prologue for func {} (id = {}) at "
                 "M[{}, {}], C[{}, {}], F[{}, {}] start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, oldStart);
    }

    if (loc.mainStart() != aStart) {
      codeView.main().setFrontier(mainOrig); // we may have shifted to align
    }
  }

  info.finalView = std::make_unique<CodeCache::View>(codeView);

  assertx(funcGuardMatches(funcGuardFromPrologue(info.start, func), func));
  assertx(code().isValidCodeAddress(info.start));
}

void emitFuncPrologueInternal(Func* func, int argc, TransKind kind,
                              PrologueMetaInfo& info) {
  try {
    emitFuncPrologueImpl(func, argc, kind, info);
  } catch (const DataBlockFull& dbFull) {

    // Fail hard if the block isn't code.hot.
    always_assert_flog(dbFull.name == "hot",
                       "data block = {}\nmessage: {}\n",
                       dbFull.name, dbFull.what());

    // Otherwise, fall back to code.main and retry.
    code().disableHot();
    info.meta.clear();
    try {
      emitFuncPrologueImpl(func, argc, kind, info);
    } catch (const DataBlockFull& dbStillFull) {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbStillFull.name, dbStillFull.what());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
}

bool publishFuncPrologueMeta(Func* func, int nArgs, TransKind kind,
                             PrologueMetaInfo& info) {
  assertOwnsMetadataLock();

  const auto start = info.start;
  if (start == nullptr) return false;

  const auto transID = info.transID;
  const auto&    loc = info.loc;
  auto&         meta = info.meta;

  const int nparams = func->numNonVariadicParams();
  const int paramIndex = nArgs <= nparams ? nArgs : nparams + 1;
  auto codeView = *info.finalView;
  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(nArgs),
                               SrcKey::PrologueTag{}};

  if (RuntimeOption::EvalEnableReusableTC) {
    recordFuncPrologue(func, loc);
  }

  if (RuntimeOption::EvalPerfRelocate) {
    GrowableVector<IncomingBranch> incomingBranches;
    recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                       loc.coldCodeStart(), loc.coldEnd(),
                       funcBody, paramIndex,
                       incomingBranches,
                       meta);
  }
  meta.process(nullptr);

  assertx(funcGuardMatches(funcGuardFromPrologue(start, func), func));
  assertx(code().isValidCodeAddress(start));
  assertx(isPrologue(kind));

  TransRec tr{funcBody, transID, kind, loc.mainStart(), loc.mainSize(),
      loc.coldStart(), loc.coldSize(), loc.frozenStart(), loc.frozenSize()};
  transdb::addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(func->unit(), func, tr);
  }

  recordGdbTranslation(funcBody, func, codeView.main(), loc.mainStart(),
                       loc.mainEnd(), false, true);
  recordBCInstr(OpFuncPrologue, loc.mainStart(), loc.mainEnd(), false);
  return true;
}

bool publishFuncPrologueCode(Func* func, int nArgs, PrologueMetaInfo& info) {
  assertOwnsMetadataLock();

  const auto start = info.start;
  if (start == nullptr) return false;

  const int nparams = func->numNonVariadicParams();
  const int paramIndex = nArgs <= nparams ? nArgs : nparams + 1;

  TRACE(2, "funcPrologue %s(%d) setting prologue %p\n",
        func->fullName()->data(), nArgs, start);
  func->setPrologue(paramIndex, start);
  return true;
}

void smashFuncCallers(TCA start, ProfTransRec* rec) {
  assertOwnsMetadataLock();
  assertx(rec->isProflogue());

  const auto func = rec->func();
  auto lock = rec->lockCallerList();

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
}

/*
 * Emit an OptPrologue for the given `info', updating it accordingly.
 */
void emitFuncPrologueOptInternal(PrologueMetaInfo& info) {
  assertOwnsCodeLock();
  assertOwnsMetadataLock();

  emitFuncPrologueInternal(info.transRec->func(), info.transRec->prologueArgs(),
                           TransKind::OptPrologue, info);
}

TCA emitFuncPrologue(Func* func, int argc, TransKind kind) {
  VMProtect _;

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  PrologueMetaInfo info{nullptr};
  emitFuncPrologueInternal(func, argc, kind, info);
  publishFuncPrologueMeta(func, argc, kind, info);
  publishFuncPrologueCode(func, argc, info);
  return info.start;
}

/*
 * Emit and publish an OptPrologue for `rec'.
 */
void emitFuncPrologueOpt(ProfTransRec* rec) {
  VMProtect _;

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  PrologueMetaInfo info(rec);
  emitFuncPrologueOptInternal(info);
  if (info.start) {
    publishFuncPrologueMeta(rec->func(), rec->prologueArgs(),
                            TransKind::OptPrologue, info);
    publishFuncPrologueCode(rec->func(), rec->prologueArgs(), info);
    smashFuncCallers(info.start, rec);
  }
}

TCA emitFuncBodyDispatchInternal(Func* func, const DVFuncletsVec& dvs,
                                 CodeCache::View view) {
  return genFuncBodyDispatch(func, dvs, view);
}

void publishFuncBodyDispatch(Func* func, TCA start, TCA end) {
  TRACE(2, "emitFuncBodyDispatch: emitted code for %s at %p\n",
        func->fullName()->data(), start);

  func->setFuncBody(start);

  if (!RuntimeOption::EvalJitNoGdb) {
    Debug::DebugInfo::Get()->recordStub(
      Debug::TCRange(start, end, false),
      Debug::lookupFunction(func, false, false, true));
  }
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportHelperToVtune(func->fullName()->data(), start, end);
  }
  if (RuntimeOption::EvalPerfPidMap) {
    Debug::DebugInfo::Get()->recordPerfMap(
      Debug::TCRange(start, end, false),
      SrcKey{}, func, false, false);
  }
}

TCA emitFuncBodyDispatch(Func* func, const DVFuncletsVec& dvs, TransKind kind) {
  VMProtect _;

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  const auto& view = code().view(kind);
  const auto tca = emitFuncBodyDispatchInternal(func, dvs, view);
  publishFuncBodyDispatch(func, tca, view.main().frontier());
  return tca;
}

}}}
