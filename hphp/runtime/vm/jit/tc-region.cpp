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
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/util/boot-stats.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

namespace {

using PrologueTCAMap = jit::hash_map<PrologueID,TCA,PrologueID::Hasher>;

using SrcKeyTransMap = jit::hash_map<SrcKey,jit::vector<TransMetaInfo*>,
                                     SrcKey::Hasher>;

/*
 * Attempt to emit code for the given IRUnit to `code'. Returns true on
 * success, false if codegen failed.
 */
bool mcGenUnit(TransEnv& env, CodeCache::View codeView, CGMeta& fixups) {
  auto const& unit = *env.unit;
  try {
    emitVunit(*env.vunit, unit, codeView, fixups,
              mcgen::dumpTCAnnotation(env.args.kind) ? &env.annotations
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

  auto const startSk = unit.context().initSrcKey;
  if (unit.context().kind == TransKind::Profile) {
    profData()->setProfiling(startSk.func()->getFuncId());
  }

  return true;
}

TransLocMaker relocateLocalTranslation(TransRange range, TransKind kind,
                                       CodeCache::View srcView,
                                       CGMeta& fixups,
                                       CodeMetaLock* locker) {
  auto reloc = [&] () -> folly::Optional<TransLocMaker> {
    if (locker) locker->lock();

    auto view = code().view(kind);
    TransLocMaker tlm(view);
    tlm.markStart();

    RelocationInfo rel;
    {
      SCOPE_EXIT { if (locker) locker->unlock(); };
      try {
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

        tlm.markEnd();
      } catch (const DataBlockFull& dbFull) {
        tlm.rollback();
        if (dbFull.name == "hot") {
          code().disableHot();
          return folly::none;
        }
        throw;
      }
    }

    adjustForRelocation(rel);
    adjustMetaDataForRelocation(rel, nullptr, fixups);
    adjustCodeForRelocation(rel, fixups);
    return tlm;
  };

  if (auto tlm = reloc()) return *tlm;

  auto tlm = reloc();
  always_assert(tlm);
  return *tlm;
}

bool checkLimit(const TransMetaInfo& info, const size_t numTrans) {
  auto const limit = info.viewKind == TransKind::Profile
    ? RuntimeOption::EvalJitMaxProfileTranslations
    : RuntimeOption::EvalJitMaxTranslations;

  // Once numTrans has reached limit + 1 we know that an interp translation
  // has already been emitted. Prior to that if numTrans == limit only allow
  // interp translations to avoid a race where multiple threads believe there
  // are still available translations.
  if (numTrans == limit + 1) return false;
  if (numTrans == limit && info.transKind != TransKind::Interp) return false;
  return true;
}

folly::Optional<TransLoc>
relocateTranslation(TransMetaInfo& info, OptView optSrcView, CodeMetaLock* locker) {
  auto const sk = info.sk;
  auto range = info.range;
  auto& fixups = info.meta;
  auto& tr = info.transRec;
  bool needsRelocate = optSrcView.has_value();

  always_assert(
    needsRelocate ||
    code().isValidCodeAddress(info.range.main.begin())
  );

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
                                          fixups, locker);
      range = tlm.range();
      return tlm.view();
    } catch (const DataBlockFull& dbFull) {
      return folly::none;
    }
  }();

  if (!optDstView) return folly::none;
  info.finalView = std::make_unique<CodeCache::View>(*optDstView);

  auto loc = range.loc();
  if (!locker) tryRelocateNewTranslation(sk, loc, *optDstView, fixups);
  info.loc = loc;

  // Update the machine-code addresses in the `tr' TransRec, which may have
  // changed due to relocation.
  if (tr.isValid() && needsRelocate) {
    tr.aStart = loc.mainStart();
    tr.acoldStart = loc.coldCodeStart();
    tr.afrozenStart = loc.frozenCodeStart();
    tr.aLen = loc.mainSize();
    tr.acoldLen = loc.coldCodeSize();
    tr.afrozenLen = loc.frozenCodeSize();
    tr.bcMapping = fixups.bcMap;
  }

  return loc;
}

/*
 * Record various metadata about the translation into the global data
 * structures.
 */
void publishTranslationMeta(TransMetaInfo& info) {
  const auto sk = info.sk;
  auto& fixups = info.meta;
  auto& tr = info.transRec;
  assertx(info.finalView);
  const auto& view = *info.finalView;
  const auto& loc = info.loc;
  assertx(!loc.empty());

  auto const srcRec = srcDB().find(sk);
  always_assert(srcRec);
  assertx(checkLimit(info, srcRec->numTrans()));

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

  transdb::addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(sk.unit(), sk.func(), tr);
  }

  fixups.process(&info.tailBranches);
}

/*
 * Add a translation to the corresponding SrcRec, effectively making it
 * reachable.  This should be done after the metadata for the translation has
 * been published.
 */
void publishTranslationCode(TransMetaInfo info) {
  const auto loc = info.loc;
  const auto srcRec = srcDB().find(info.sk);
  always_assert(srcRec);

  TRACE(1, "newTranslation: %p  sk: %s\n",
        loc.mainStart(), showShort(info.sk).c_str());

  srcRec->newTranslation(loc, info.tailBranches);

  TRACE(1, "mcg: %zd-byte translation\n", (ssize_t)loc.mainSize());
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getTCSpace().c_str());
  }
}

void invalidateSrcKey(SrcKey sk) {
  assertx(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  auto const sr = srcDB().find(sk);
  if (!sr) {
    always_assert(profData()->wasDeserialized());
    return;
  }

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

void relocateOptFunc(FuncMetaInfo& info, SrcKeyTransMap& srcKeyTrans,
                     PrologueTCAMap* prologueTCAs = nullptr,
                     size_t* failedBytes = nullptr, CodeMetaLock* locker = nullptr) {
  auto const func = info.func;

  // If the function had a body dispatch emitted during profiling, then emit it
  // again right before the optimized prologues.
  if (func->getFuncBody() != tc::ustubs().funcBodyHelperThunk &&
      func->getDVFunclets().size() > 0) {
    auto const loc = emitFuncBodyDispatchInternal(
      func, func->getDVFunclets(), TransKind::OptPrologue, locker
    );
    info.bodyDispatch = std::make_unique<BodyDispatchMetaInfo>(
      loc.mainStart(), loc.mainEnd()
    );
  }

  // Relocate/emit all prologues and translations for func in order.
  size_t prologueIdx = 0;
  size_t translationIdx = 0;

  for (auto kind : info.order) {
    switch (kind) {
      case FuncMetaInfo::Kind::Prologue: {
        assertx(prologueIdx < info.prologues.size());
        auto& prologueInfo = info.prologues[prologueIdx];
        assertx(func == prologueInfo.transRec->func());
        emitFuncPrologueOptInternal(prologueInfo, locker);
        if (prologueTCAs != nullptr && prologueInfo.start != nullptr) {
          const auto nargs = prologueInfo.transRec->prologueArgs();
          const auto pid = PrologueID(func, nargs);
          always_assert(code().inHotOrMain(prologueInfo.start));
          (*prologueTCAs)[pid] = prologueInfo.start;
        }
        prologueIdx++;
        break;
      }
      case FuncMetaInfo::Kind::Translation: {
        assertx(translationIdx < info.translations.size());
        auto& transInfo = info.translations[translationIdx];
        translationIdx++;
        FTRACE(3, "relocateOptFunc: trying to relocate translation at {}\n",
               transInfo.range.main.begin());
        auto it = srcKeyTrans.find(transInfo.sk);
        if (it != srcKeyTrans.end()) {
          auto& vec = it->second;
          if (!checkLimit(transInfo, vec.size())) {
            FTRACE(1, " - skipping translation that wouldn't be published!"
                   "transInfo = {} sk = {} vec.size = {}\n",
                   &transInfo, showShort(transInfo.sk), vec.size());
            transInfo.loc = TransLoc{};
            continue;
          }
        }
        auto& range = transInfo.range;
        auto bytes = range.main.size() + range.cold.size() +
                     range.frozen.size();
        auto loc = relocateTranslation(transInfo, info.tcBuf.view(), locker);
        FTRACE(3, "relocateOptFunc: relocated to start loc {}\n",
               loc ? loc->mainStart() : 0x0);
        if (loc) {
          auto& vec = srcKeyTrans[transInfo.sk];
          assertx(vec.size() < RuntimeOption::EvalJitMaxTranslations);
          always_assert(code().inHotOrMain(transInfo.loc.mainStart()));
          FTRACE(3, "appending transInfo {} (mainStart @ {}) to sk {} "
                 "at index {}\n",
                 &transInfo, transInfo.loc.mainStart(), showShort(transInfo.sk),
                 vec.size());
          vec.emplace_back(&transInfo);
        }
        if (!loc && failedBytes) {
          *failedBytes += bytes;
        }
        break;
      }
    }
  }
  assertx(prologueIdx == info.prologues.size());
  assertx(translationIdx == info.translations.size());
}

void publishOptFuncMeta(FuncMetaInfo& info) {
  auto const func = info.func;

  for (auto& prologueInfo : info.prologues) {
    const auto tca = prologueInfo.start;
    if (tca != nullptr) {
      const auto nArgs = prologueInfo.transRec->prologueArgs();
      publishFuncPrologueMeta(func, nArgs, TransKind::OptPrologue,
                              prologueInfo);
    }
  }

  for (auto& transInfo : info.translations) {
    const auto loc = transInfo.loc;
    if (!loc.empty()) {
      publishTranslationMeta(transInfo);
    }
  }
}

void publishOptFuncCode(FuncMetaInfo& info,
                        jit::hash_set<TCA>* publishedSet = nullptr) {
  auto const func = info.func;

  if (info.bodyDispatch) {
    const auto start = info.bodyDispatch->start;
    const auto   end = info.bodyDispatch->end;
    always_assert(start);
    // NB: this already calls func->setFuncBody() with the new start address
    publishFuncBodyDispatch(func, start, end);
    if (publishedSet) publishedSet->insert(start);
  }

  // Publish all prologues and translations for func in order.
  size_t prologueIdx = 0;
  size_t translationIdx = 0;

  for (auto kind : info.order) {
    switch (kind) {
      case FuncMetaInfo::Kind::Prologue: {
        assertx(prologueIdx < info.prologues.size());
        auto& prologueInfo = info.prologues[prologueIdx];
        const auto rec = prologueInfo.transRec;
        assertx(func == rec->func());
        const auto tca = prologueInfo.start;
        if (tca != nullptr) {
          const auto nArgs = rec->prologueArgs();
          bool succeeded = publishFuncPrologueCode(func, nArgs, prologueInfo);
          assertx(succeeded);
          smashFuncCallers(tca, rec);
          if (succeeded && publishedSet) publishedSet->insert(tca);
        }
        prologueIdx++;
        break;
      }
      case FuncMetaInfo::Kind::Translation: {
        assertx(translationIdx < info.translations.size());
        auto& transInfo = info.translations[translationIdx];
        const auto regionSk = transInfo.sk;
        const auto      loc = transInfo.loc;
        if (!loc.empty()) {
          publishTranslationCode(std::move(transInfo));
          if (publishedSet) publishedSet->insert(loc.mainStart());
          if (regionSk.offset() == func->base() &&
              func->getDVFunclets().size() == 0) {
            func->setFuncBody(loc.mainStart());
          }
        }
        translationIdx++;
        break;
      }
    }
  }
  assertx(prologueIdx == info.prologues.size());
  assertx(translationIdx == info.translations.size());
}

void relocateSortedOptFuncs(std::vector<FuncMetaInfo>& infos,
                            PrologueTCAMap& prologueTCAs,
                            SrcKeyTransMap& srcKeyTrans) {
  size_t failedBytes = 0;

  CodeMetaLock locker{false};

  bool shouldLog = RuntimeOption::ServerExecutionMode();
  for (auto& finfo : infos) {
    if (!Func::isFuncIdValid(finfo.fid)) {
      continue;
    }

    // make sure we don't get ahead of the translation threads
    mcgen::waitForTranslate(finfo);

    if (!checkTCLimits()) {
      FTRACE(1, "relocateSortedOptFuncs: ran out of space in the TC. "
             "Skipping function {} {}\n", finfo.func->getFuncId(),
             finfo.func->fullName());
      failedBytes += infoSize(finfo);
      continue;
    }
    if (shouldLog) {
      shouldLog = false;
      Logger::Info("retranslateAll: starting to relocate functions");
    }
    relocateOptFunc(finfo, srcKeyTrans, &prologueTCAs, &failedBytes, &locker);
  }

  if (failedBytes) {
    FTRACE(1, "relocateSortedOptFuncs: failedBytes = {}\n", failedBytes);
    logPerfWarning("opt_translation_overflow", 1, [&] (StructuredLogEntry& e) {
      e.setInt("bytes_dropped", failedBytes);
    });
  }
}

/*
 * Smash and optimize the calls in `transInfo' to prologues in `prologueTCAs'.
 */
void smashOptCalls(TransMetaInfo& transInfo,
                   const PrologueTCAMap& prologueTCAs) {
  assertx(!transInfo.loc.empty());

  auto const oldSmashableCallData = std::move(transInfo.meta.smashableCallData);
  for (auto& pair : oldSmashableCallData) {
    TCA call = pair.first;
    const PrologueID& pid = pair.second;
    auto it = prologueTCAs.find(pid);
    if (it == prologueTCAs.end()) {
      // insert non-smashed call back into transInfo.meta.smashableCallData
      transInfo.meta.smashableCallData.emplace(pair);
      continue;
    }

    const TCA target = it->second;
    FTRACE(1, "smashedOptCalls: found candidate call @ {}, "
           "target prologue @ {} (funcId={}, nArgs={})\n",
           call, target, pid.funcId(), pid.nargs());
    assertx(code().inHotOrMainOrColdOrFrozen(call));
    assertx(code().inHotOrMain(target));

    smashCall(call, target);
    optimizeSmashedCall(call);

    transInfo.meta.smashableLocations.erase(call);
  }
}

/*
 * Find the jump target for jumps of kind `jumpKind' within the translation
 * corresponding to `info' going to the SrcRec corresponding to `vec'.  `info'
 * is nullptr when the source translation is a prologue.
 */
TCA findJumpTarget(const TransMetaInfo* info,
                   const jit::vector<TransMetaInfo*>& vec,
                   CGMeta::JumpKind jumpKind) {
  always_assert(vec.size() > 0);

  using Kind = CGMeta::JumpKind;
  const bool isRetrans = jumpKind == Kind::Fallback ||
                         jumpKind == Kind::Fallbackcc;
  always_assert(IMPLIES(info == nullptr, !isRetrans));

  // Case 1: when the jump is in a prologue (info == nullptr) or it jumps to a
  //         different SrcKey, we jump to the first translation in the list.
  if (!isRetrans) {
    return vec.front()->loc.mainStart();
  }

  // Case 2: when jumping to the same SrcKey, we jump to the translation
  //         following this one (`info').
  always_assert(info->sk == vec.front()->sk);
  for (size_t i = 0; i < vec.size() - 1; i++) {
    if (vec[i] == info) {
      return vec[i + 1]->loc.mainStart();
    }
  }

  // We should only get here if `info' is the last translation.
  always_assert(info == vec.back());
  return nullptr;
}

/*
 * Smash and optimize the jumps in `transInfo' going to the optimized
 * translations in `srcKeyTrans'.
 */
void smashOptJumps(CGMeta& meta,
                   const TransMetaInfo* transInfo, // nullptr for prologues
                   const SrcKeyTransMap& srcKeyTrans) {
  using Kind = CGMeta::JumpKind;

  jit::hash_set<TCA> smashed;
  jit::hash_set<TCA> smashedOldTargets;
  decltype(meta.smashableJumpData) newSmashableJumpData;

  for (auto& pair : meta.smashableJumpData) {
    auto jump = pair.first;
    auto   sk = pair.second.sk;
    auto kind = pair.second.kind;
    auto it = srcKeyTrans.find(sk);
    if (it == srcKeyTrans.end()) {
      newSmashableJumpData.emplace(pair);
      continue;
    }
    const auto& transVec = it->second;
    TCA succTCA = findJumpTarget(transInfo, transVec, kind);
    if (succTCA == nullptr) {
      newSmashableJumpData.emplace(pair);
      continue;
    }
    assertx(code().inHotOrMain(succTCA));

    DEBUG_ONLY auto kindStr = [&] {
      switch (kind) {
        case Kind::Bindjmp:    return "bindjmp";
        case Kind::Bindjcc:    return "bindjcc";
        case Kind::Fallback:   return "fallback";
        case Kind::Fallbackcc: return "fallbackcc";
      }
      not_reached();
    }();

    FTRACE(3, "smashOptJumps: found candidate jump @ {}, kind = {}, "
           "target SrcKey {}\n",
           jump, kindStr, showShort(sk));
    assertx(code().inHotOrMainOrColdOrFrozen(jump));
    TCA prevTarget = nullptr;
    switch (kind) {
      case Kind::Bindjmp:
      case Kind::Fallback: {
        prevTarget = smashableJmpTarget(jump);
        smashJmp(jump, succTCA);
        optimizeSmashedJmp(jump);
        break;
      }
      case Kind::Bindjcc:
      case Kind::Fallbackcc: {
        prevTarget = smashableJccTarget(jump);
        smashJcc(jump, succTCA);
        optimizeSmashedJcc(jump);
        break;
      }
    }
    smashed.insert(jump);
    smashedOldTargets.insert(prevTarget);
    meta.smashableLocations.erase(jump);
  }

  // Remove jumps that were smashed from meta.smashableJumpData.
  meta.smashableJumpData.swap(newSmashableJumpData);

  // If any of the jumps we smashed was a tail jump (i.e. going to a
  // retranslation of the entry SrcKey), then remove it from the set of
  // inProgressTailJumps as it has already been smashed.
  GrowableVector<IncomingBranch> newTailJumps;
  for (auto& jump : meta.inProgressTailJumps) {
    if (smashed.count(jump.toSmash()) == 0) {
      newTailJumps.push_back(jump);
    } else {
      FTRACE(3, "smashOptJumps: removing {} from inProgressTailJumps\n",
             jump.toSmash());
    }
  }
  meta.inProgressTailJumps.swap(newTailJumps);

  // If any smashed jumps had corresponding stubs, then remove those stubs from
  // meta and free their memory.
  std::vector<TCA> newReusedStubs;
  for (auto tca : meta.reusedStubs) {
    if (smashedOldTargets.count(tca) == 0) {
      newReusedStubs.push_back(tca);
    } else {
      FTRACE(3, "smashOptJumps: freeing dead reused stub @ {}\n", tca);
      markStubFreed(tca);
    }
  }
  meta.reusedStubs.swap(newReusedStubs);
}

/*
 * Smash and optimize the calls and jumps between the translations/prologues in
 * `infos'.
 */
void smashOptSortedOptFuncs(std::vector<FuncMetaInfo>& infos,
                            const PrologueTCAMap& prologueTCAs,
                            const SrcKeyTransMap& srcKeyTrans) {
  BootStats::Block timer("RTA_smash_opt_funcs",
                         RuntimeOption::ServerExecutionMode());
  for (auto& finfo : infos) {
    if (!Func::isFuncIdValid(finfo.fid)) continue;

    for (auto& transInfo : finfo.translations) {
      // Skip if the translation wasn't relocated (e.g. ran out of TC space).
      if (transInfo.loc.empty()) continue;
      assertx(code().inHotOrMain(transInfo.loc.mainStart()));

      smashOptCalls(transInfo, prologueTCAs);
      smashOptJumps(transInfo.meta, &transInfo, srcKeyTrans);
    }

    for (auto& prologueInfo : finfo.prologues) {
      // Skip if the prologue wasn't relocated (e.g. ran out of TC space).
      if (prologueInfo.start == nullptr) continue;
      assertx(code().inHotOrMain(prologueInfo.start));

      smashOptJumps(prologueInfo.meta, nullptr, srcKeyTrans);
    }
  }
}

void invalidateFuncsProfSrcKeys() {
  BootStats::Block timer("RTA_invalidate_prof_srckeys",
                         RuntimeOption::ServerExecutionMode());
  auto const pd = profData();
  assertx(pd);
  auto const maxFuncId = pd->maxProfilingFuncId();

  for (FuncId funcId = 0; funcId <= maxFuncId; funcId++) {
    if (!Func::isFuncIdValid(funcId) || !pd->profiling(funcId)) continue;

    auto func = const_cast<Func*>(Func::fromFuncId(funcId));
    invalidateFuncProfSrcKeys(func);

    // clear the func body and prologues
    func->resetFuncBody();
    auto const numPrologues = func->numPrologues();
    for (int p = 0; p < numPrologues; p++) {
      func->resetPrologue(p);
    }
  }
}

void publishSortedOptFuncsMeta(std::vector<FuncMetaInfo>& infos) {
  BootStats::Block timer("RTA_publish_meta",
                         RuntimeOption::ServerExecutionMode());
  for (auto& finfo : infos) {
    if (Func::isFuncIdValid(finfo.fid)) {
      publishOptFuncMeta(finfo);
    }
  }
}

void publishSortedOptFuncsCode(std::vector<FuncMetaInfo>& infos,
                               jit::hash_set<TCA>* publishedSet) {
  BootStats::Block timer("RTA_publish_code",
                         RuntimeOption::ServerExecutionMode());
  for (auto& finfo : infos) {
    if (Func::isFuncIdValid(finfo.fid)) {
      publishOptFuncCode(finfo, publishedSet);
    }
  }
}

std::string show(const SrcKeyTransMap& map) {
  std::string ret;
  for (auto& skt : map) {
    folly::format(&ret, "  - [{}]:", showShort(skt.first));
    for (auto tinfo : skt.second) {
      folly::format(&ret, " {},", tinfo->loc.mainStart());
    }
    ret += "\n";
  }
  return ret;
}

void checkPublishedAddr(TCA tca, const jit::hash_set<TCA>& publishedSet) {
  always_assert_flog(code().inHotOrMain(tca),
                     "srcKeyTrans has address not in hot/main: {}", tca);
  always_assert_flog(publishedSet.count(tca),
                     "srcKeyTrans has unpublished translation @ {}", tca);
}

/*
 * Make sure that every address that we may have smashed to was published in
 * either hot or main.
 */
void checkPublishedAddresses(const PrologueTCAMap&     prologueTCAs,
                             const SrcKeyTransMap&     srcKeyTrans,
                             const jit::hash_set<TCA>& publishedSet) {
  for (auto& prologueTCA : prologueTCAs) {
    checkPublishedAddr(prologueTCA.second, publishedSet);
  }

  for (auto& skt : srcKeyTrans) {
    auto& vec = skt.second;
    for (auto tinfo : vec) {
      auto loc = tinfo->loc;
      if (!loc.empty()) {
        checkPublishedAddr(loc.mainStart(), publishedSet);
      }
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
  if (srcDB().find(sk)) return;
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

  tracing::Block _b{
    "emit-translation",
    [&] {
      if (env.vunit) return traceProps(*env.vunit);
      if (env.unit) return traceProps(*env.unit);
      return tracing::Props{};
    }
  };

  VMProtect _;

  auto& args = env.args;
  auto const sk = args.sk;

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
    // If we were trying to create profile translation, we just bail to the
    // interpreter.  This prevents generating Interp translations in code.prof.
    if (isProfiling(viewKind)) return folly::none;

    args.kind = TransKind::Interp;
    FTRACE(1, "emitting dispatchBB interp request for failed "
           "translation (spOff = {})\n", env.initSpOffset.offset);
    vwrap(codeView.main(), codeView.data(), fixups,
          [&] (Vout& v) { emitInterpReq(v, sk, env.initSpOffset); },
          CodeKind::Helper, false);
  }

  Timer metaTimer(Timer::mcg_finishTranslation_metadata);
  auto range = maker.markEnd();

  if (args.kind == TransKind::Profile) {
    always_assert(args.region);
    auto metaLock = lockMetadata();
    profData()->addTransProfile(env.transID, args.region, env.pconds,
                                range.main.size());
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

  return TransMetaInfo{sk, codeView, viewKind, args.kind, range, nullptr,
                       TransLoc{}, std::move(fixups), std::move(tr),
                       GrowableVector<IncomingBranch>{}};
}

folly::Optional<TransLoc>
publishTranslation(TransMetaInfo info, OptView optSrcView) {
  auto const srcRec = srcDB().find(info.sk);
  always_assert(srcRec);

  if (!checkLimit(info, srcRec->numTrans())) return folly::none;

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  // Recheck after acquiring the lock.
  if (!checkLimit(info, srcRec->numTrans())) return folly::none;

  auto loc = relocateTranslation(info, optSrcView, nullptr);
  if (loc) {
    publishTranslationMeta(info);
    publishTranslationCode(std::move(info));
  }
  return loc;
}

void publishOptFunc(FuncMetaInfo info) {
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  SrcKeyTransMap srcKeyTrans;
  relocateOptFunc(info, srcKeyTrans);
  invalidateFuncProfSrcKeys(Func::fromFuncId(info.fid));
  publishOptFuncMeta(info);
  publishOptFuncCode(info);
}

void relocatePublishSortedOptFuncs(std::vector<FuncMetaInfo> infos) {
  // Grab the session now (which includes a Treadmill::Session) so
  // that no Func's get destroyed during this step
  ProfData::Session pds;
  const bool serverMode = RuntimeOption::ServerExecutionMode();

  PrologueTCAMap prologueTCAs;
  SrcKeyTransMap srcKeyTrans;

  relocateSortedOptFuncs(infos, prologueTCAs, srcKeyTrans);

  if (serverMode) {
    Logger::Info(
      "retranslateAll: finished optimizing and relocating functions"
    );
  }

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  FTRACE(3,
         "relocatePublishSortedOptFuncs: after relocateSortedOptFuncs:\n{}\n",
         show(srcKeyTrans));

  smashOptSortedOptFuncs(infos, prologueTCAs, srcKeyTrans);

  FTRACE(3,
         "relocatePublishSortedOptFuncs: after smashOptSortedOptFuncs:\n{}\n",
         show(srcKeyTrans));

  if (serverMode) {
    Logger::Info("retranslateAll: starting to publish functions");
  }

  invalidateFuncsProfSrcKeys();

  // Publish the metadata for all the translations/prologues before actually
  // publishing any of their code.  This is necessary because we've smashed
  // calls and jumps across translations so, once any of them is made reachable,
  // we have to assume that all of them are already reachable, even before
  // they're all published.
  publishSortedOptFuncsMeta(infos);

  jit::hash_set<TCA> publishedSet;
  publishSortedOptFuncsCode(infos, debug ? &publishedSet : nullptr);

  FTRACE(3,
         "relocatePublishSortedOptFuncs: after publishSortedOptFuncsCode:\n"
         "{}\n", show(srcKeyTrans));

  if (debug) {
    checkPublishedAddresses(prologueTCAs, srcKeyTrans, publishedSet);
  }
}

}}}
