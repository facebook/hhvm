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
#include "hphp/runtime/vm/jit/tc-prologue.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/tc-relocate.h"

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

namespace {

/*
 * Attempt to emit code for the given IRUnit to `code'. Returns true on
 * success, false if codegen failed.
 */
bool mcGenUnit(TransEnv& env, CodeCache::View codeView, CGMeta& fixups) {
  auto const& unit = *env.unit;
  try {
    emitVunit(*env.vunit, unit, codeView, fixups,
              mcgen::dumpTCAnnotation(*env.args.sk.func(), env.args.kind)
              ? &env.annotations
              : nullptr);
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      code().disableHot();
      return false;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }

  auto const startSk = unit.context().srcKey();
  if (unit.context().kind == TransKind::Profile) {
    profData()->setProfiling(startSk.func()->getFuncId());
  }

  return true;
}

TransLocMaker relocateLocalTranslation(TransRange range, TransKind kind,
                                       CodeCache::View srcView,
                                       CGMeta& fixups) {
  auto reloc = [&] () -> folly::Optional<TransLocMaker> {
    auto view = code().view(kind);
    TransLocMaker tlm(view);
    tlm.markStart();

    try {
      RelocationInfo rel;

      auto origin = range.data;
      if (!origin.empty()) {
        view.data().bytes(origin.size(),
                          srcView.data().toDestAddress(origin.begin()));

        auto dest = tlm.dataRange();
        auto oAddr = origin.begin();
        auto dAddr = dest.begin();
        while (oAddr != origin.end()) {
          assertx(dAddr != dest.end());
          rel.recordAddress(oAddr++, dAddr++, 0);
        }
      }

      relocate(rel, view.main(), range.main.begin(), range.main.end(),
               srcView.main(), fixups, nullptr, AreaIndex::Main);
      relocate(rel, view.cold(), range.cold.begin(), range.cold.end(),
               srcView.cold(), fixups, nullptr, AreaIndex::Cold);
      if (&srcView.cold() != &srcView.frozen()) {
        relocate(rel, view.frozen(), range.frozen.begin(),
                 range.frozen.end(), srcView.frozen(), fixups, nullptr,
                 AreaIndex::Frozen);
      }

      adjustForRelocation(rel);
      adjustMetaDataForRelocation(rel, nullptr, fixups);
      adjustCodeForRelocation(rel, fixups);
    } catch (const DataBlockFull& dbFull) {
      tlm.rollback();
      if (dbFull.name == "hot") {
        code().disableHot();
        return folly::none;
      }
      throw;
    }
    return tlm;
  };

  if (auto tlm = reloc()) return *tlm;

  auto tlm = reloc();
  always_assert(tlm);
  return *tlm;
}

bool checkLimit(const TransMetaInfo& info, const SrcRec* srcRec) {
  auto const limit = info.viewKind == TransKind::Profile
    ? RuntimeOption::EvalJitMaxProfileTranslations
    : RuntimeOption::EvalJitMaxTranslations;

  auto const numTrans = srcRec->numTrans();

  // Once numTrans has reached limit + 1 we know that an interp translation
  // has already been emitted. Prior to that if numTrans == limit only allow
  // interp translations to avoid a race where multiple threads believe there
  // are still available translations.
  if (numTrans == limit + 1) return false;
  if (numTrans == limit && info.transKind != TransKind::Interp) return false;
  return true;
}

folly::Optional<TransLoc>
publishTranslationInternal(TransMetaInfo info, OptView optSrcView) {
  auto const sk = info.sk;
  auto range = info.range;
  auto& fixups = info.meta;
  auto& tr = info.transRec;
  bool needsRelocate = optSrcView.hasValue();

  always_assert(
    needsRelocate ||
    code().isValidCodeAddress(info.range.main.begin())
  );

  auto const srcRec = srcDB().find(sk);
  always_assert(srcRec);

  // Check again now that we have the lock, the first check is racy but may
  // prevent unnecessarily acquiring the lock.
  if (!checkLimit(info, srcRec)) return folly::none;

  // 1) If we are currently in a thread local TC we need to relocate into the
  //    end of the real TC.
  // 2) If reusable TC is enabled we need to relocate to an inner allocation if
  //    sufficiently sized free blocks can be found
  //
  // Ideally we can combine (1) and (2), however, currently thread-local TC is
  // only useful in production environments where we may want to emit optimize
  // translations in parallel, while reusable TC is only meaningful in sandboxes
  // where translations may be invalidates by source modifications.

  auto optDstView = [&] () -> folly::Optional<CodeCache::View> {
    if (!needsRelocate) return info.emitView;
    try {
      auto tlm = relocateLocalTranslation(range, info.viewKind, *optSrcView,
                                          fixups);
      range = tlm.markEnd();
      return tlm.view();
    } catch (const DataBlockFull& dbFull) {
      return folly::none;
    }
  }();

  if (!optDstView) return folly::none;
  auto view = *optDstView;

  auto loc = range.loc();
  tryRelocateNewTranslation(sk, loc, view, fixups);

  // Finally, record various metadata about the translation and add it to the
  // SrcRec.
  if (RuntimeOption::EvalProfileBC) {
    auto const vmUnit = sk.unit();
    TransBCMapping prev{};
    for (auto& cur : fixups.bcMap) {
      if (!cur.aStart) continue;
      if (prev.aStart) {
        if (prev.bcStart < vmUnit->bclen()) {
          recordBCInstr(uint32_t(vmUnit->getOp(prev.bcStart)),
                        prev.aStart, cur.aStart, false);
        }
      } else {
        recordBCInstr(OpTraceletGuard, loc.mainStart(), cur.aStart, false);
      }
      prev = cur;
    }
  }

  recordRelocationMetaData(sk, *srcRec, loc, fixups);
  recordGdbTranslation(sk, sk.func(), view.main(), loc.mainStart(),
                       loc.mainEnd(), false, false);
  recordGdbTranslation(sk, sk.func(), view.cold(), loc.coldCodeStart(),
                       loc.coldEnd(), false, false);

  // If we relocated the code, the machine-code addresses in the `tr' TransRec
  // need to be updated.
  if (tr.isValid() && needsRelocate) {
    tr.aStart = loc.mainStart();
    tr.acoldStart = loc.coldCodeStart();
    tr.afrozenStart = loc.frozenCodeStart();
    tr.aLen = loc.mainSize();
    tr.acoldLen = loc.coldCodeSize();
    tr.afrozenLen = loc.frozenCodeSize();
    tr.bcMapping = fixups.bcMap;
  }

  transdb::addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(sk.unit(), sk.func(), tr);
  }

  GrowableVector<IncomingBranch> inProgressTailBranches;
  fixups.process(&inProgressTailBranches);

  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: %s\n",
        loc.mainStart(), showShort(sk).c_str());
  srcRec->newTranslation(loc, inProgressTailBranches);

  TRACE(1, "mcg: %zd-byte tracelet\n", (ssize_t)loc.mainSize());
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getTCSpace().c_str());
  }

  reportJitMaturity(code());

  return loc;
}

void invalidateSrcKey(SrcKey sk) {
  assertx(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  auto const sr = srcDB().find(sk);
  always_assert(sr);
  /*
   * Since previous translations aren't reachable from here, we know we
   * just created some garbage in the TC. We currently have no mechanism
   * to reclaim this.
   */
  FTRACE_MOD(Trace::reusetc, 1,
             "Replacing translations from sk: {} " "to SrcRec addr={}\n",
             showShort(sk), (void*)sr);
  Trace::Indent _i;
  sr->replaceOldTranslations();
}

void invalidateFuncProfSrcKeys(const Func* func) {
  assertx(profData());
  auto const funcId = func->getFuncId();
  for (auto tid : profData()->funcProfTransIDs(funcId)) {
    invalidateSrcKey(profData()->transRec(tid)->srcKey());
  }
}

size_t infoSize(const FuncMetaInfo& info) {
  size_t sz = 0;
  for (auto& trans : info.translations) {
    auto& range = trans.range;
    sz += range.main.size() + range.cold.size() + range.frozen.size();
  }
  return sz;
}

bool checkTCLimits() {
  auto const main_under = code().main().used() < CodeCache::AMaxUsage;
  auto const cold_under = code().cold().used() < CodeCache::AColdMaxUsage;
  auto const froz_under = code().frozen().used() < CodeCache::AFrozenMaxUsage;

  if (main_under && cold_under && froz_under) return true;
  return cold_under && froz_under && code().hotEnabled();
}

void publishOptFunctionInternal(FuncMetaInfo info,
                                size_t* failedBytes = nullptr) {
  auto const func = info.func;

  // If the function had a body dispatch emitted during profiling, then emit it
  // again right before the optimized prologues.
  if (func->getFuncBody() != tc::ustubs().funcBodyHelperThunk &&
      (func->getDVFunclets().size() > 0 || func->hasThisVaries())) {
    // NB: this already calls func->setFuncBody() with the new TCA
    emitFuncBodyDispatchInternal(func, func->getDVFunclets(),
                                 TransKind::OptPrologue);
  }

  for (auto const rec : info.prologues) {
    emitFuncPrologueOptInternal(rec);
  }

  invalidateFuncProfSrcKeys(func);

  for (auto& trans : info.translations) {
    auto const regionSk = trans.sk;
    auto& range = trans.range;
    auto bytes = range.main.size() + range.cold.size() + range.frozen.size();
    auto const loc = publishTranslationInternal(
      std::move(trans), info.tcBuf.view()
    );
    if (loc &&
        regionSk.offset() == func->base() &&
        !func->hasThisVaries() &&
        func->getDVFunclets().size() == 0) {
      func->setFuncBody(loc->mainStart());
    } else if (!loc && failedBytes) {
      *failedBytes += bytes;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
}

SrcRec* findSrcRec(SrcKey sk) {
  return srcDB().find(sk);
}

void createSrcRec(SrcKey sk, FPInvOffset spOff) {
  if (srcDB().find(sk)) return;

  auto const srcRecSPOff = sk.resumeMode() != ResumeMode::None
    ? folly::none : folly::make_optional(spOff);

  // We put retranslate requests at the end of our slab to more frequently
  // allow conditional jump fall-throughs
  auto codeLock = lockCode();
  auto codeView = code().view();
  TCA astart = codeView.main().frontier();
  TCA coldStart = codeView.cold().frontier();
  TCA frozenStart = codeView.frozen().frontier();
  TCA req;
  if (!RuntimeOption::EvalEnableReusableTC) {
    req = svcreq::emit_persistent(codeView.cold(),
                                  codeView.data(),
                                  srcRecSPOff,
                                  REQ_RETRANSLATE,
                                  sk.offset(),
                                  TransFlags().packed);
  } else {
    auto const stubsize = svcreq::stub_size();
    auto newStart = codeView.cold().allocInner(stubsize);
    if (!newStart) {
      newStart = codeView.cold().frontier();
    }
    // Ensure that the anchor translation is a known size so that it can be
    // reclaimed when the function is freed
    req = svcreq::emit_ephemeral(codeView.cold(),
                                 codeView.data(),
                                 (TCA)newStart,
                                 srcRecSPOff,
                                 REQ_RETRANSLATE,
                                 sk.offset(),
                                 TransFlags().packed);
  }
  SKTRACE(1, sk, "inserting anchor translation for (%p,%d) at %p\n",
          sk.unit(), sk.offset(), req);

  auto metaLock = lockMetadata();
  always_assert(srcDB().find(sk) == nullptr);
  auto const sr = srcDB().insert(sk, req);
  if (RuntimeOption::EvalEnableReusableTC) {
    recordFuncSrcRec(sk.func(), sr);
  }

  if (srcRecSPOff) always_assert(sr->nonResumedSPOff() == *srcRecSPOff);

  size_t asize      = codeView.main().frontier()   - astart;
  size_t coldSize   = codeView.cold().frontier()   - coldStart;
  size_t frozenSize = codeView.frozen().frontier() - frozenStart;
  assertx(asize == 0);
  if (coldSize && RuntimeOption::EvalDumpTCAnchors) {
    auto const transID =
      profData() && transdb::enabled() ? profData()->allocTransID()
                                       : kInvalidTransID;
    TransRec tr(sk, transID, TransKind::Anchor,
                astart, asize, coldStart, coldSize,
                frozenStart, frozenSize);
    transdb::addTranslation(tr);
    if (RuntimeOption::EvalJitUseVtuneAPI) {
      reportTraceletToVtune(sk.unit(), sk.func(), tr);
    }

    assertx(!transdb::enabled() ||
            transdb::getTransRec(coldStart)->kind == TransKind::Anchor);
  }
}


////////////////////////////////////////////////////////////////////////////////

folly::Optional<TransMetaInfo> emitTranslation(TransEnv env, OptView optDst) {
  Timer timer(Timer::mcg_finishTranslation);

  VMProtect _;

  auto& args = env.args;
  auto const sk = args.sk;

  profileSetHotFuncAttr();

  std::unique_lock<SimpleMutex> codeLock;
  if (!optDst) {
    codeLock = lockCode();
  }

  auto codeView = optDst ? *optDst : code().view(args.kind);
  auto viewKind = args.kind;

  CGMeta fixups;
  TransLocMaker maker{codeView};
  maker.markStart();

  // mcGenUnit emits machine code from vasm
  if (env.vunit && !mcGenUnit(env, codeView, fixups)) {
    // mcGenUnit() failed. Roll back, drop the unit and region, and clear
    // fixups.
    maker.rollback();
    maker.markStart();
    env.unit.reset();
    env.vunit.reset();
    args.region.reset();
    fixups.clear();
  }

  if (env.vunit) {
    if (!newTranslation()) {
      return folly::none;
    }
  } else {
    args.kind = TransKind::Interp;
    FTRACE(1, "emitting dispatchBB interp request for failed "
           "translation (spOff = {})\n", env.initSpOffset.offset);
    vwrap(codeView.main(), codeView.data(), fixups,
          [&] (Vout& v) { emitInterpReq(v, sk, env.initSpOffset); },
          CodeKind::Helper);
  }

  Timer metaTimer(Timer::mcg_finishTranslation_metadata);
  auto range = maker.markEnd();

  if (args.kind == TransKind::Profile) {
    always_assert(args.region);
    auto metaLock = lockMetadata();
    profData()->addTransProfile(env.transID, args.region, env.pconds);
  }

  TransRec tr;
  if (RuntimeOption::EvalJitUseVtuneAPI ||
      Trace::moduleEnabledRelease(Trace::trans, 1) ||
      transdb::enabled()) {
    tr = maker.rec(sk, env.transID, args.kind, args.region, fixups.bcMap,
                   std::move(env.annotations),
                   env.unit && cfgHasLoop(*env.unit));
  }

  if (env.unit && env.unit->logEntry()) {
    auto metaLock = lockMetadata();
    logTranslation(env, range);
  }

  if (!RuntimeOption::EvalJitLogAllInlineRegions.empty() && env.vunit) {
    logFrames(*env.vunit);
  }

  return TransMetaInfo{sk, codeView, viewKind, args.kind, range,
                       std::move(fixups), std::move(tr)};
}

folly::Optional<TransLoc>
publishTranslation(TransMetaInfo info, OptView optSrcView) {
  auto const srcRec = srcDB().find(info.sk);
  always_assert(srcRec);

  if (!checkLimit(info, srcRec)) return folly::none;

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  return publishTranslationInternal(std::move(info), optSrcView);
}

void publishOptFunction(FuncMetaInfo info) {
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  publishOptFunctionInternal(std::move(info));
}

void publishSortedOptFunctions(std::vector<FuncMetaInfo> infos) {
  // Do this first to ensure that the code and metadata locks have been dropped
  // before running the treadmill
  ProfData::Session pds;

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  size_t failedBytes = 0;
  bool hasSpace = checkTCLimits();

  for (auto& finfo : infos) {
    if (!Func::isFuncIdValid(finfo.fid)) {
      continue;
    }

    if (!hasSpace) {
      failedBytes += infoSize(finfo);
      continue;
    }

    publishOptFunctionInternal(std::move(finfo), &failedBytes);
    hasSpace = checkTCLimits();
  }

  if (failedBytes) {
    logPerfWarning("opt_translation_overflow", 1, [&] (StructuredLogEntry& e) {
      e.setInt("bytes_dropped", failedBytes);
    });
  }
}

}}}
