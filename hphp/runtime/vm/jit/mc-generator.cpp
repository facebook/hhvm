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

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include <cinttypes>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <unwind.h>
#endif

#include <algorithm>
#include <exception>
#include <memory>
#include <queue>
#include <string>
#include <strstream>
#include <unordered_set>
#include <vector>

#include <folly/Format.h>
#include <folly/MapUtil.h>
#include <folly/Optional.h>
#include <folly/String.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>

#include "hphp/util/abi-cxx.h"
#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/bitops.h"
#include "hphp/util/cycles.h"
#include "hphp/util/debug.h"
#include "hphp/util/disasm.h"
#include "hphp/util/eh-frame.h"
#include "hphp/util/logger.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/process.h"
#include "hphp/util/rank.h"
#include "hphp/util/build-info.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/vixl/a64/constants-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/php-debug.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/debug-guards.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/recycle-tc.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc-info.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(mcg);

using namespace Trace;

// The global MCGenerator object.
MCGenerator* mcg;

static __thread size_t s_initialTCSize;
static ServiceData::ExportedCounter* s_jitMaturityCounter;
static std::atomic<bool> s_loggedJitMature{false};

///////////////////////////////////////////////////////////////////////////////

namespace {
/*
 * Convenience class for creating TransLocs and TransRecs for new translations.
 *
 * Records the beginning and end of a translation and stores the size of the
 * cold and frozen regions in the first 4 bytes of their respective regions.
 */
struct TransLocMaker {
  explicit TransLocMaker(CodeCache::View c) : cache(c) {}

  /*
   * Record the start of a translation, and reserve space at the top of cold
   * and frozen (if they aren't the same) to record sizes.
   */
  void markStart() {
    loc.setMainStart(cache.main().frontier());
    loc.setColdStart(cache.cold().frontier());
    loc.setFrozenStart(cache.frozen().frontier());
    dataStart = cache.data().frontier();

    cache.cold().dword(0);
    if (&cache.cold() != &cache.frozen()) cache.frozen().dword(0);
  }

  /*
   * If loc contains a valid location, reset the frontiers of all code and data
   * blocks to the positions recorded by the last call to markStart().
   */
  void rollback() {
    if (loc.empty()) return;

    cache.main().setFrontier(loc.mainStart());
    cache.cold().setFrontier(loc.coldStart());
    cache.frozen().setFrontier(loc.frozenStart());
    cache.data().setFrontier(dataStart);
  }

  /*
   * Record the end of a translation, storing the size of cold and frozen,
   * returns a TransLoc representing the translation.
   */
  TransLoc markEnd() {
    uint32_t* coldSize   = (uint32_t*)loc.coldStart();
    uint32_t* frozenSize = (uint32_t*)loc.frozenStart();
    *coldSize   = cache  .cold().frontier() - loc.coldStart();
    *frozenSize = cache.frozen().frontier() - loc.frozenStart();
    loc.setMainSize(cache.main().frontier() - loc.mainStart());

    return loc;
  }

  /*
   * Create a TransRec for the translation, markEnd() should be called prior to
   * calling rec().
   */
  TransRec rec(
      SrcKey                      sk,
      TransID                     transID,
      TransKind                   kind,
      RegionDescPtr               region  = RegionDescPtr(),
      std::vector<TransBCMapping> bcmap   = std::vector<TransBCMapping>(),
      Annotations&&               annot   = Annotations(),
      bool                        hasLoop = false) const {
    return TransRec(sk, transID, kind,
                    loc.mainStart(), loc.mainSize(),
                    loc.coldCodeStart(), loc.coldCodeSize(),
                    loc.frozenCodeStart(), loc.frozenCodeSize(),
                    std::move(region), std::move(bcmap),
                    std::move(annot), hasLoop);
  }

private:
  CodeCache::View cache;
  TransLoc loc;
  Address dataStart;
};
}

///////////////////////////////////////////////////////////////////////////////

bool shouldPGOFunc(const Func& func) {
  if (profData() == nullptr) return false;

  // JITing pseudo-mains requires extra checks that blow the IR.  PGO
  // can significantly increase the size of the regions, so disable it for
  // pseudo-mains (so regions will be just tracelets).
  if (func.isPseudoMain()) return false;

  if (!RuntimeOption::EvalJitPGOHotOnly) return true;
  return func.attrs() & AttrHot;
}

bool dumpTCAnnotation(const Func& func, TransKind transKind) {
  return RuntimeOption::EvalDumpTCAnnotationsForAllTrans ||
    (transKind == TransKind::Optimize && (func.attrs() & AttrHot));
}

///////////////////////////////////////////////////////////////////////////////

bool MCGenerator::profileSrcKey(SrcKey sk) const {
  if (!shouldPGOFunc(*sk.func())) return false;
  if (profData()->optimized(sk.funcID())) return false;
  if (profData()->profiling(sk.funcID())) return true;

  // Don't start profiling new functions if the size of either main or
  // prof is already above Eval.JitAMaxUsage and we already filled hot.
  auto tcUsage = std::max(m_code.main().used(), m_code.prof().used());
  if (tcUsage >= CodeCache::AMaxUsage && !m_code.hotEnabled()) {
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

/*
 * Invalidate the SrcDB entries for func's SrcKeys that have any
 * Profile translation.
 */
void MCGenerator::invalidateFuncProfSrcKeys(const Func* func) {
  assertx(profData());
  FuncId funcId = func->getFuncId();
  auto codeLock = lockCode();
  for (auto tid : profData()->funcProfTransIDs(funcId)) {
    invalidateSrcKey(profData()->transRec(tid)->srcKey());
  }
}

TransResult MCGenerator::retranslate(TransArgs args) {
  auto sr = m_srcDB.find(args.sk);
  always_assert(sr);
  bool locked = sr->tryLock();
  SCOPE_EXIT {
    if (locked) sr->freeLock();
  };
  if (isDebuggerAttachedProcess() && m_tx.isSrcKeyInBL(args.sk)) {
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

  LeaseHolder writer(Translator::WriteLease(), args.sk.func(), args.kind);
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

  auto result = translate(args);

  checkFreeProfData();
  return result;
}

TCA MCGenerator::retranslateOpt(SrcKey sk, TransID transId, bool align) {
  if (isDebuggerAttachedProcess()) return nullptr;

  auto const func = const_cast<Func*>(sk.func());
  auto const funcID = func->getFuncId();
  if (profData() == nullptr || profData()->optimized(funcID)) return nullptr;

  LeaseHolder writer(Translator::WriteLease(), func, TransKind::Optimize);
  if (!writer) return nullptr;

  if (profData()->optimized(funcID)) return nullptr;
  profData()->setOptimized(funcID);

  TRACE(1, "retranslateOpt: transId = %u\n", transId);
  func->setFuncBody(ustubs().funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  bool includedBody{false};
  TCA start = regeneratePrologues(func, sk, includedBody);

  // Regionize func and translate all its regions.
  std::string transCFGAnnot;
  auto const regions = includedBody ? std::vector<RegionDescPtr>{}
                                    : regionizeFunc(func, this, transCFGAnnot);

  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto transArgs = TransArgs{regionSk};
    if (transCFGAnnot.size() > 0) {
      transArgs.annotations.emplace_back("TransCFG", transCFGAnnot);
    }
    transArgs.region = region;
    transArgs.kind = TransKind::Optimize;

    auto const regionStart = translate(transArgs).tca();
    if (regionStart != nullptr &&
        regionSk.offset() == func->base() &&
        func->getDVFunclets().size() == 0 &&
        func->getFuncBody() == ustubs().funcBodyHelperThunk) {
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

void MCGenerator::checkFreeProfData() {
  // In PGO mode, we free all the profiling data once the main area code reaches
  // its maximum usage and either the hot area is also full or all the functions
  // that were profiled have already been optimized.
  //
  // However, we keep the data around indefinitely in a few special modes:
  // * Eval.EnableReusableTC
  // * TC dumping enabled (Eval.DumpTC/DumpIR/etc.)
  if (profData() &&
      !RuntimeOption::EvalEnableReusableTC &&
      m_code.main().used() >= CodeCache::AMaxUsage &&
      (!m_code.hotEnabled() ||
       profData()->profilingFuncs() == profData()->optimizedFuncs()) &&
      !Translator::isTransDBEnabled()) {
    discardProfData();
  }
}

static bool liveFrameIsPseudoMain() {
  auto const ar = vmfp();
  if (!(ar->func()->attrs() & AttrMayUseVV)) return false;
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

/*
 * Return an existing translation for `args', or nullptr if one can't be found.
 */
TCA MCGenerator::findTranslation(const TransArgs& args) const {
  auto sk = args.sk;
  sk.func()->validate();

  if (liveFrameIsPseudoMain() && !RuntimeOption::EvalJitPseudomain) {
    SKTRACE(2, sk, "punting on pseudoMain\n");
    return nullptr;
  }

  if (auto const sr = m_srcDB.find(sk)) {
    if (auto const tca = sr->getTopTranslation()) {
      SKTRACE(2, sk, "getTranslation: found %p\n", tca);
      return tca;
    }
  }

  return nullptr;
}

/*
 * Find or create a translation for `args'. Returns TCA of "best" current
 * translation. May return nullptr if it is currently impossible to create a
 * translation.
 */
TransResult MCGenerator::getTranslation(const TransArgs& args) {
  if (auto const tca = findTranslation(args)) return tca;

  return createTranslation(args);
}

int MCGenerator::numTranslations(SrcKey sk) const {
  if (const SrcRec* sr = m_srcDB.find(sk)) {
    return sr->translations().size();
  }
  return 0;
}

const StaticString
  s_php_errormsg("php_errormsg"),
  s_http_response_header("http_response_header");

bool MCGenerator::shouldTranslateNoSizeLimit(const Func* func) const {
  // If we've hit Eval.JitGlobalTranslationLimit, then we stop translating.
  if (m_numTrans.load(std::memory_order_relaxed) >=
      RuntimeOption::EvalJitGlobalTranslationLimit) {
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

bool MCGenerator::shouldTranslate(const Func* func, TransKind kind) const {
  if (!shouldTranslateNoSizeLimit(func)) return false;

  // Otherwise, follow the Eval.JitAMaxUsage limit.  However, we do allow PGO
  // translations past that limit if there's still space in code.hot.
  if (m_code.main().used() < CodeCache::AMaxUsage) return true;

  switch (kind) {
    case TransKind::ProfPrologue:
    case TransKind::Profile:
    case TransKind::OptPrologue:
    case TransKind::Optimize:
      return m_code.hotEnabled();

    default:
      return false;
  }
}


static void populateLiveContext(RegionContext& ctx) {
  auto const fp = vmfp();
  auto const sp = vmsp();

  always_assert(ctx.func == fp->m_func);

  auto const ctxClass = ctx.func->cls();
  // Track local types.
  for (uint32_t i = 0; i < fp->m_func->numLocals(); ++i) {
    ctx.liveTypes.push_back(
      { Location::Local{i}, typeFromTV(frame_local(fp, i), ctxClass) }
    );
    FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
  }

  // Track stack types and pre-live ActRecs.
  int32_t stackOff = 0;
  visitStackElems(
    fp, sp, ctx.bcOffset,
    [&] (const ActRec* ar, Offset) {
      auto const objOrCls =
        ar->hasThis()  ? Type::SubObj(ar->getThis()->getVMClass()) :
        ar->hasClass() ? Type::SubCls(ar->getClass())
                       : TNullptr;

      ctx.preLiveARs.push_back({ stackOff, ar->m_func, objOrCls });
      FTRACE(2, "added prelive ActRec {}\n", show(ctx.preLiveARs.back()));
      stackOff += kNumActRecCells;
    },
    [&] (const TypedValue* tv) {
      ctx.liveTypes.push_back(
        { Location::Stack{ctx.spOffset - stackOff}, typeFromTV(tv, ctxClass) }
      );
      stackOff++;
      FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
    }
  );
}

TransResult MCGenerator::createTranslation(TransArgs args) {
  args.kind = profileSrcKey(args.sk) ? TransKind::Profile : TransKind::Live;

  if (!shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  auto sk = args.sk;
  LeaseHolder writer(Translator::WriteLease(), sk.func(), args.kind);
  if (!writer || !shouldTranslate(sk.func(), args.kind)) return nullptr;

  if (RuntimeOption::EvalFailJitPrologs && sk.op() == Op::FCallAwait) {
    return nullptr;
  }

  if (!createSrcRec(sk)) return nullptr;

  auto sr = m_srcDB.find(sk);
  always_assert(sr);

  if (auto const tca = sr->getTopTranslation()) {
    // Handle extremely unlikely race; someone may have just added the first
    // translation for this SrcRec while we did a non-blocking wait on the
    // write lease in createSrcRec().
    return tca;
  }

  return retranslate(args);
}

bool MCGenerator::createSrcRec(SrcKey sk) {
  if (m_srcDB.find(sk)) return true;

  auto const srcRecSPOff = sk.resumed() ? folly::none
                                        : folly::make_optional(liveSpOff());

  // We put retranslate requests at the end of our slab to more frequently
  // allow conditional jump fall-throughs
  auto codeLock  = lockCode();
  auto code       = m_code.view();
  TCA astart      = code.main().frontier();
  TCA coldStart   = code.cold().frontier();
  TCA frozenStart = code.frozen().frontier();
  TCA req;
  if (!RuntimeOption::EvalEnableReusableTC) {
    req = svcreq::emit_persistent(code.cold(),
                                  code.data(),
                                  srcRecSPOff,
                                  REQ_RETRANSLATE,
                                  sk.offset(),
                                  TransFlags().packed);
  } else {
    auto const stubsize = svcreq::stub_size();
    auto newStart = code.cold().allocInner(stubsize);
    if (!newStart) {
      newStart = code.cold().frontier();
    }
    // Ensure that the anchor translation is a known size so that it can be
    // reclaimed when the function is freed
    req = svcreq::emit_ephemeral(code.cold(),
                                 code.data(),
                                 (TCA)newStart,
                                 srcRecSPOff,
                                 REQ_RETRANSLATE,
                                 sk.offset(),
                                 TransFlags().packed);
  }
  SKTRACE(1, sk, "inserting anchor translation for (%p,%d) at %p\n",
          sk.unit(), sk.offset(), req);

  auto metaLock = lockMetadata();
  always_assert(m_srcDB.find(sk) == nullptr);
  auto const sr = m_srcDB.insert(sk);
  if (RuntimeOption::EvalEnableReusableTC) {
    recordFuncSrcRec(sk.func(), sr);
  }
  sr->setFuncInfo(sk.func());
  sr->setAnchorTranslation(req);

  if (srcRecSPOff) always_assert(sr->nonResumedSPOff() == *srcRecSPOff);

  size_t asize      = code.main().frontier()   - astart;
  size_t coldSize   = code.cold().frontier()   - coldStart;
  size_t frozenSize = code.frozen().frontier() - frozenStart;
  assertx(asize == 0);
  if (coldSize && RuntimeOption::EvalDumpTCAnchors) {
    auto const transID =
      profData() && Translator::isTransDBEnabled() ? profData()->allocTransID()
                                                   : kInvalidTransID;
    TransRec tr(sk, transID, TransKind::Anchor,
                astart, asize, coldStart, coldSize,
                frozenStart, frozenSize);
    m_tx.addTranslation(tr);
    if (RuntimeOption::EvalJitUseVtuneAPI) {
      reportTraceletToVtune(sk.unit(), sk.func(), tr);
    }

    assertx(!m_tx.isTransDBEnabled() ||
            m_tx.getTransRec(coldStart)->kind == TransKind::Anchor);
  }

  return true;
}

TCA
MCGenerator::lookupTranslation(SrcKey sk) const {
  if (auto const sr = m_srcDB.find(sk)) {
    return sr->getTopTranslation();
  }
  return nullptr;
}

namespace {
/*
 * Returns true iff we already have Eval.JitMaxTranslations translations
 * recorded in srcRec.
 */
bool reachedTranslationLimit(SrcKey sk, const SrcRec& srcRec) {
  if (srcRec.translations().size() != RuntimeOption::EvalJitMaxTranslations) {
    return false;
  }
  INC_TPC(max_trans);

  if (debug && Trace::moduleEnabled(Trace::mcg, 2)) {
    const auto& tns = srcRec.translations();
    TRACE(1, "Too many (%zd) translations: %s, BC offset %d\n",
          tns.size(), sk.unit()->filepath()->data(),
          sk.offset());
    SKTRACE(2, sk, "{\n");
    TCA topTrans = srcRec.getTopTranslation();
    for (size_t i = 0; i < tns.size(); ++i) {
      auto const rec = mcg->tx().getTransRec(tns[i].mainStart());
      assertx(rec);
      SKTRACE(2, sk, "%zd %p\n", i, tns[i].mainStart());
      if (tns[i].mainStart() == topTrans) {
        SKTRACE(2, sk, "%zd: *Top*\n", i);
      }
      if (rec->kind == TransKind::Anchor) {
        SKTRACE(2, sk, "%zd: Anchor\n", i);
      } else {
        SKTRACE(2, sk, "%zd: guards {\n", i);
        for (unsigned j = 0; j < rec->guards.size(); ++j) {
          FTRACE(2, "{}\n", rec->guards[j]);
        }
        SKTRACE(2, sk, "%zd } guards\n", i);
      }
    }
    SKTRACE(2, sk, "} /* Too many translations */\n");
  }
  return true;
}

/*
 * Analyze the given TransArgs and return the region to translate, or nullptr
 * if one could not be selected.
 */
RegionDescPtr prepareRegion(const TransArgs& args) {
  if (args.kind == TransKind::Optimize) {
    assertx(RuntimeOption::EvalJitPGO);
    if (args.region) return args.region;

    assertx(isValidTransID(args.transId));
    return selectHotRegion(args.transId, mcg);
  }

  // Attempt to create a region at this SrcKey
  assertx(args.kind == TransKind::Profile || args.kind == TransKind::Live);
  auto const sk = args.sk;
  RegionContext rContext { sk.func(), sk.offset(), liveSpOff(),
                           sk.resumed() };
  FTRACE(2, "populating live context for region\n");
  populateLiveContext(rContext);
  return selectRegion(rContext, args.kind);
}
}

TransResult MCGenerator::translate(TransArgs args) {
  INC_TPC(translate);

  assert(args.kind != TransKind::Invalid);
  assertx(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

  if (!shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  Timer timer(Timer::mcg_translate);

  auto const srcRec = m_srcDB.find(args.sk);
  always_assert(srcRec);

  args.region = reachedTranslationLimit(args.sk, *srcRec) ? nullptr
                                                          : prepareRegion(args);
  TransEnv env{args};
  env.initSpOffset = args.region ? args.region->entry()->initialSpOffset()
                                 : liveSpOff();
  env.annotations.insert(env.annotations.end(),
                         args.annotations.begin(), args.annotations.end());

  // Lower the RegionDesc to an IRUnit, then lower that to a Vunit.
  if (args.region) {
    if (args.kind == TransKind::Profile ||
        (profData() && Translator::isTransDBEnabled())) {
      env.transID = profData()->allocTransID();
    }
    auto const transContext =
      TransContext{env.transID, args.kind, args.flags, args.sk,
                   env.initSpOffset};

    env.unit = irGenRegion(*args.region, transContext,
                           env.pconds, env.annotations);
    if (env.unit) {
      env.vunit = irlower::lowerUnit(*env.unit);
    }
  }

  // If our caller requested partial translation and we don't already have the
  // write lease, we're done here. Otherwise, finish the translation now.
  if (args.allowPartial && !Translator::WriteLease().amOwner()) {
    return std::move(env);
  }
  timer.stop();
  return finishTranslation(std::move(env));
}

TCA MCGenerator::getFuncBody(Func* func) {
  auto tca = func->getFuncBody();
  if (tca != ustubs().funcBodyHelperThunk) return tca;

  LeaseHolder writer(Translator::WriteLease(), func, TransKind::Profile);
  if (!writer) return nullptr;

  tca = func->getFuncBody();
  if (tca != ustubs().funcBodyHelperThunk) return tca;

  auto const dvs = func->getDVFunclets();
  if (dvs.size()) {
    auto codeLock = mcg->lockCode();
    tca = genFuncBodyDispatch(func, dvs, m_code.view());
    func->setFuncBody(tca);
    if (!RuntimeOption::EvalJitNoGdb) {
      m_debugInfo.recordStub(
        Debug::TCRange(tca, m_code.view().main().frontier(), false),
        Debug::lookupFunction(func, false, false, true));
    }
    if (RuntimeOption::EvalJitUseVtuneAPI) {
      reportHelperToVtune(func->fullName()->data(),
                          tca,
                          m_code.view().main().frontier());
    }
    if (RuntimeOption::EvalPerfPidMap) {
      m_debugInfo.recordPerfMap(
        Debug::TCRange(tca, m_code.view().main().frontier(), false),
        SrcKey{}, func, false, false);
    }
  } else {
    SrcKey sk(func, func->base(), false);
    tca = mcg->getTranslation(TransArgs{sk}).tca();
    if (tca) func->setFuncBody(tca);
  }

  return tca;
}

/*
 * funcPrologue --
 *
 * Given a callee and a number of args, match up to the callee's
 * argument expectations and dispatch.
 *
 * Call/return hand-shaking is a bit funny initially. At translation time,
 * we don't necessarily know what function we're calling. For instance,
 *
 *   f(g());
 *
 * Will lead to a set of basic blocks like:
 *
 * b1: pushfuncd "f"
 *     pushfuncd "g"
 *     fcall
 * b2: fcall
 *
 * The fcall labelled "b2" above may not be statically bindable in our
 * execution model.
 *
 * We decouple the call work into a per-callsite portion, responsible
 * for recording the return address, and a per-(callee, numArgs) portion,
 * responsible for fixing up arguments and dispatching to remaining
 * code. We call the per-callee portion a "prologue."
 *
 * Also, we are called from two distinct environments. From REQ_BIND_CALL,
 * we're running "between" basic blocks, with all VM registers sync'ed.
 * However, we're also called in the middle of basic blocks, when dropping
 * entries into func->m_prologues. So don't go around using the
 * translation-time values of vmfp()/vmsp(), since they have an
 * unpredictable relationship to the source.
 */
bool
MCGenerator::checkCachedPrologue(const Func* func, int paramIdx,
                                 TCA& prologue) const {
  prologue = (TCA)func->getPrologue(paramIdx);
  if (prologue != ustubs().fcallHelperThunk) {
    TRACE(1, "cached prologue %s(%d) -> cached %p\n",
          func->fullName()->data(), paramIdx, prologue);
    assertx(m_code.isValidCodeAddress(prologue));
    return true;
  }
  return false;
}

TCA MCGenerator::emitFuncPrologue(Func* func, int argc, TransKind kind) {
  if (m_numTrans.fetch_add(1, std::memory_order_relaxed) >=
      RuntimeOption::EvalJitGlobalTranslationLimit) {
    return nullptr;
  }

  const int nparams = func->numNonVariadicParams();
  const int paramIndex = argc <= nparams ? argc : nparams + 1;

  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(argc), false};

  profileSetHotFuncAttr();
  auto codeLock = lockCode();
  auto code = m_code.view(kind);
  TCA mainOrig = code.main().frontier();
  CGMeta fixups;

  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  align(code.main(), &fixups, Alignment::CacheLineRoundUp, AlignContext::Dead);

  TransLocMaker maker(code);
  maker.markStart();

  // Careful: this isn't necessarily the real entry point. For funcIsMagic
  // prologues, this is just a possible prologue.
  TCA aStart = code.main().frontier();

  // Give the prologue a TransID if we have profiling data.
  auto const transID = [&]{
    if (kind == TransKind::ProfPrologue) {
      auto const profData = jit::profData();
      auto const id = profData->allocTransID();
      profData->addTransProfPrologue(id, funcBody, paramIndex);
      return id;
    }
    if (profData() && Translator::isTransDBEnabled()) {
      return profData()->allocTransID();
    }
    return kInvalidTransID;
  }();

  TCA start = genFuncPrologue(transID, kind, func, argc, code, fixups);

  auto loc = maker.markEnd();
  auto metaLock = lockMetadata();

  if (RuntimeOption::EvalEnableReusableTC) {
    TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
               cs = loc.coldStart(), ce = loc.coldEnd(),
               fs = loc.frozenStart(), fe = loc.frozenEnd(),
               oldStart = start;

    auto const did_relocate = relocateNewTranslation(loc, code, fixups, &start);

    if (did_relocate) {
      FTRACE_MOD(reusetc, 1,
                 "Relocated prologue for func {} (id = {}) "
                 "from M[{}, {}], C[{}, {}], F[{}, {}] to M[{}, {}] "
                 "C[{}, {}] F[{}, {}] orig start @ {} new start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, loc.mainStart(), loc.mainEnd(),
                 loc.coldStart(), loc.coldEnd(), loc.frozenStart(),
                 loc.frozenEnd(), oldStart, start);
    } else {
      FTRACE_MOD(reusetc, 1,
                 "Created prologue for func {} (id = {}) at "
                 "M[{}, {}], C[{}, {}], F[{}, {}] start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, oldStart);
    }

    recordFuncPrologue(func, loc);
    if (loc.mainStart() != aStart) {
      code.main().setFrontier(mainOrig); // we may have shifted to align
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
  assertx(m_code.isValidCodeAddress(start));

  TRACE(2, "funcPrologue mcg %p %s(%d) setting prologue %p\n",
        this, func->fullName()->data(), argc, start);
  func->setPrologue(paramIndex, start);

  assertx(kind == TransKind::LivePrologue ||
          kind == TransKind::ProfPrologue ||
          kind == TransKind::OptPrologue);

  auto tr = maker.rec(funcBody, transID, kind);
  m_tx.addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(func->unit(), func, tr);
  }


  recordGdbTranslation(funcBody, func, code.main(), loc.mainStart(),
                       false, true);
  recordBCInstr(OpFuncPrologue, loc.mainStart(), loc.mainEnd(), false);

  return start;
}

TCA MCGenerator::getFuncPrologue(Func* func, int nPassed, ActRec* ar,
                                 bool forRegeneratePrologue) {
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
           forRegeneratePrologue   ? TransKind::OptPrologue  :
                                     TransKind::LivePrologue;
  };
  LeaseHolder writer(Translator::WriteLease(), func, computeKind());
  if (!writer) return nullptr;

  auto const kind = computeKind();
  // Check again now that we have the write lease, in case the answer to
  // profileSrcKey() changed.
  if (!writer.checkKind(kind)) return nullptr;

  // If we're regenerating a prologue, and we want to check shouldTranslate()
  // but ignore the code size limits.  We still want to respect the global
  // translation limit and other restrictions, though.
  if (forRegeneratePrologue) {
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
    m_code.disableHot();
    try {
      return emitFuncPrologue(func, nPassed, kind);
    } catch (const DataBlockFull& dbFull) {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }
}

/**
 * Given the proflogueTransId for a TransProflogue translation, regenerate the
 * prologue (as a TransPrologue).  Returns the starting address for the
 * translation corresponding to triggerSk, if such translation is generated;
 * otherwise returns nullptr.
 */
TCA MCGenerator::regeneratePrologue(TransID prologueTransId, SrcKey triggerSk,
                                    bool& emittedDVInit) {
  auto rec = profData()->transRec(prologueTransId);
  auto func = rec->func();
  auto nArgs = rec->prologueArgs();
  emittedDVInit = false;

  // Regenerate the prologue.
  func->resetPrologue(nArgs);
  auto const start = getFuncPrologue(
    func,
    nArgs,
    nullptr /* ActRec */,
    true /* regeneratePrologue */
  );
  if (!start) return nullptr;

  func->setPrologue(nArgs, start);

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

  // If this prologue has a DV funclet, then generate a translation for the DV
  // funclet right after the prologue.
  TCA triggerSkStart = nullptr;
  if (nArgs < func->numNonVariadicParams()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      SrcKey funcletSK(func, paramInfo.funcletOff, false);
      auto funcletTransId = profData()->dvFuncletTransId(func, nArgs);
      if (funcletTransId != kInvalidTransID) {
        invalidateSrcKey(funcletSK);
        // We're done touching the TC on behalf of the prologue so drop the
        // lock.
        codeLock.unlock();

        auto args = TransArgs{funcletSK};
        args.transId = funcletTransId;
        args.kind = TransKind::Optimize;
        auto dvStart = translate(args).tca();
        emittedDVInit |= dvStart != nullptr;
        if (dvStart && !triggerSkStart && funcletSK == triggerSk) {
          triggerSkStart = dvStart;
        }
        // Flag that this translation has been retranslated, so that
        // it's not retranslated again along with the function body.
        profData()->setOptimized(funcletSK);
      }
    }
  }

  return triggerSkStart;
}

/**
 * Regenerate all prologues of func that were previously generated.
 * The prologues are sorted in ascending order of profile counters.
 * For prologues with corresponding DV funclets, their corresponding
 * DV funclet will be regenerated right after them.  The idea is to
 * generate the function body right after calling this function, so
 * that all prologues are placed right before it, and with the hottest
 * prologues closer to it.
 *
 * Returns the starting address for the translation corresponding to
 * triggerSk, if such translation is generated; otherwise returns
 * nullptr.
 */
TCA MCGenerator::regeneratePrologues(Func* func, SrcKey triggerSk,
                                     bool& includedBody) {
  TCA triggerStart = nullptr;
  std::vector<TransID> prologTransIDs;

  for (int nArgs = 0; nArgs < func->numPrologues(); nArgs++) {
    TransID tid = profData()->proflogueTransId(func, nArgs);
    if (tid != kInvalidTransID) {
      prologTransIDs.push_back(tid);
    }
  }

  std::sort(prologTransIDs.begin(), prologTransIDs.end(),
          [&](TransID t1, TransID t2) -> bool {
            // This will sort in ascending order.
            return profData()->transCounter(t2) >
                   profData()->transCounter(t1);
          });

  // Next, we're going to regenerate each prologue along with its DV funclet.
  // We consider the option of either including the DV funclets in the same
  // region as the function body or not.  Including them in the same region
  // enables some type information to flow and thus can eliminate some stores
  // and type checks, but it can also increase the code size by duplicating the
  // whole function body.  Therefore, we only include the function body along
  // with the DV init if both (a) the function has a single proflogue, and (b)
  // the size of the function is within a certain threshold.
  //
  // The mechanism used to keep the function body separate from the DV init is
  // to temporarily mark the SrcKey for the function body as already optimized.
  // (The region selectors break a region whenever they hit a SrcKey that has
  // already been optimized.)
  SrcKey funcBodySk(func, func->base(), false);
  includedBody = prologTransIDs.size() <= 1 &&
    func->past() - func->base() <= RuntimeOption::EvalJitPGOMaxFuncSizeDupBody;

  if (!includedBody) profData()->setOptimized(funcBodySk);
  SCOPE_EXIT{ profData()->clearOptimized(funcBodySk); };

  bool emittedAnyDVInit = false;
  for (TransID tid : prologTransIDs) {
    bool emittedDVInit = false;
    TCA start = regeneratePrologue(tid, triggerSk, emittedDVInit);
    if (triggerStart == nullptr && start != nullptr) {
      triggerStart = start;
    }
    emittedAnyDVInit |= emittedDVInit;
  }

  // If we tried to include the function body along with a DV init, but didn't
  // end up generating any DV init, then flag that the function body was not
  // included.
  if (!emittedAnyDVInit) includedBody = false;

  return triggerStart;
}

/*
 * bindJmp --
 *
 *   Runtime service handler that patches a jmp to the translation of
 *   u:dest from toSmash.
 */
TCA
MCGenerator::bindJmp(TCA toSmash, SrcKey destSk, ServiceRequest req,
                     TransFlags trflags, bool& smashed) {
  auto args = TransArgs{destSk};
  args.flags = trflags;
  auto tDest = getTranslation(args).tca();
  if (!tDest) return nullptr;

  LeaseHolder writer(Translator::WriteLease(), destSk.func(),
                     TransKind::Profile);
  if (!writer) return tDest;

  auto const sr = m_srcDB.find(destSk);
  always_assert(sr);
  // The top translation may have changed while we waited for the write lease,
  // so read it again.  If it was replaced with a new translation, then bind to
  // the new one.  If it was invalidated, then don't bind the jump.
  tDest = sr->getTopTranslation();
  if (tDest == nullptr) return nullptr;

  auto codeLock = lockCode();

  if (req == REQ_BIND_ADDR) {
    auto addr = reinterpret_cast<TCA*>(toSmash);
    if (*addr == tDest) {
      // Already smashed
      return tDest;
    }
    sr->chainFrom(IncomingBranch::addr(addr));
    smashed = true;
    return tDest;
  }

  auto const isJcc = [&] {
    switch (arch()) {
      case Arch::X64: {
        x64::DecodedInstruction di(toSmash);
        return (di.isBranch() && !di.isJmp());
      }

      case Arch::ARM: {
        using namespace vixl;
        struct JccDecoder : public Decoder {
          void VisitConditionalBranch(Instruction* inst) override {
            cc = true;
          }
          bool cc = false;
        };
        JccDecoder decoder;
        decoder.Decode(Instruction::Cast(toSmash));
        return decoder.cc;
      }

      case Arch::PPC64:
        ppc64_asm::DecodedInstruction di(toSmash);
        return (di.isBranch() && !di.isJmp());
    }
    not_reached();
  }();

  if (isJcc) {
    auto const target = smashableJccTarget(toSmash);
    assertx(target);

    // Return if already smashed.
    if (target == tDest) return tDest;
    sr->chainFrom(IncomingBranch::jccFrom(toSmash));
  } else {
    auto const target = smashableJmpTarget(toSmash);
    assertx(target);

    // Return if already smashed.
    if (!target || target == tDest) return tDest;
    sr->chainFrom(IncomingBranch::jmpFrom(toSmash));
  }

  smashed = true;
  return tDest;
}

namespace {

struct FreeRequestStubTrigger {
  explicit FreeRequestStubTrigger(TCA stub) : m_stub(stub) {
    TRACE(3, "FreeStubTrigger @ %p, stub %p\n", this, m_stub);
  }
  void operator()() {
    TRACE(3, "FreeStubTrigger: Firing @ %p , stub %p\n", this, m_stub);
    mcg->freeRequestStub(m_stub);
  }
private:
  TCA m_stub;
};

}

void
MCGenerator::enterTC(TCA start, ActRec* stashedAR) {
  if (debug) {
    fflush(stdout);
    fflush(stderr);
  }

  assertx(m_code.isValidCodeAddress(start));
  assertx(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

  assertx(!Translator::WriteLease().amOwner());

  INC_TPC(enter_tc);
  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = SrcKey{liveFunc(), vmpc(), liveResumed()}.toAtomicInt();
    Trace::ringbufferEntry(RBTypeEnterTC, skData, (uint64_t)start);
  }

  tl_regState = VMRegState::DIRTY;
  enterTCImpl(start, stashedAR);
  tl_regState = VMRegState::CLEAN;
  assertx(isValidVMStackAddress(vmsp()));

  vmfp() = nullptr;
}

TCA MCGenerator::handleServiceRequest(svcreq::ReqInfo& info) noexcept {
  FTRACE(1, "handleServiceRequest {}\n", svcreq::to_name(info.req));

  assert_native_stack_aligned();
  tl_regState = VMRegState::CLEAN; // partially a lie: vmpc() isn't synced

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    Trace::ringbufferEntry(
      RBTypeServiceReq, (uint64_t)info.req, (uint64_t)info.args[0].tca
    );
  }

  TCA start = nullptr;
  SrcKey sk;
  auto smashed = false;

  switch (info.req) {
    case REQ_BIND_JMP:
    case REQ_BIND_ADDR: {
      auto const toSmash = info.args[0].tca;
      sk = SrcKey::fromAtomicInt(info.args[1].sk);
      auto const trflags = info.args[2].trflags;
      start = bindJmp(toSmash, sk, info.req, trflags, smashed);
      break;
    }

    case REQ_RETRANSLATE: {
      INC_TPC(retranslate);
      sk = SrcKey{liveFunc(), info.args[0].offset, liveResumed()};
      auto trflags = info.args[1].trflags;
      auto args = TransArgs{sk};
      args.flags = trflags;
      start = retranslate(args).tca();
      SKTRACE(2, sk, "retranslated @%p\n", start);
      break;
    }

    case REQ_RETRANSLATE_OPT: {
      sk = SrcKey::fromAtomicInt(info.args[0].sk);
      auto transID = info.args[1].transID;
      start = retranslateOpt(sk, transID, false);
      SKTRACE(2, sk, "retranslated-OPT: transId = %d  start: @%p\n", transID,
              start);
      break;
    }

    case REQ_POST_INTERP_RET: {
      // This is only responsible for the control-flow aspect of the Ret:
      // getting to the destination's translation, if any.
      auto ar = info.args[0].ar;
      auto caller = info.args[1].ar;
      assertx(caller == vmfp());
      // If caller is a resumable (aka a generator) then whats likely happened
      // here is that we're resuming a yield from. That expression happens to
      // cause an assumption that we made earlier to be violated (that `ar` is
      // on the stack), so if we detect this situation we need to fix up the
      // value of `ar`.
      if (UNLIKELY(caller->resumed() &&
                   caller->func()->isNonAsyncGenerator())) {
        auto gen = frame_generator(caller);
        if (gen->m_delegate.m_type == KindOfObject) {
          auto delegate = gen->m_delegate.m_data.pobj;
          // We only checked that our delegate is an object, but we can't get
          // into this situation if the object itself isn't a Generator
          assert(delegate->getVMClass() == Generator::getClass());
          // Ok so we're in a `yield from` situation, we know our ar is garbage.
          // The ar that we're looking for is the ar of the delegate generator,
          // so grab that here.
          ar = Generator::fromObject(delegate)->actRec();
        }
      }
      Unit* destUnit = caller->func()->unit();
      // Set PC so logging code in getTranslation doesn't get confused.
      vmpc() = destUnit->at(caller->m_func->base() + ar->m_soff);
      if (ar->isFCallAwait()) {
        // If there was an interped FCallAwait, and we return via the
        // jit, we need to deal with the suspend case here.
        assert(ar->m_r.m_aux.u_fcallAwaitFlag < 2);
        if (ar->m_r.m_aux.u_fcallAwaitFlag) {
          start = ustubs().fcallAwaitSuspendHelper;
          break;
        }
      }
      sk = SrcKey{caller->func(), vmpc(), caller->resumed()};
      start = getTranslation(TransArgs{sk}).tca();
      TRACE(3, "REQ_POST_INTERP_RET: from %s to %s\n",
            ar->m_func->fullName()->data(),
            caller->m_func->fullName()->data());
      break;
    }

    case REQ_POST_DEBUGGER_RET: {
      auto fp = vmfp();
      auto caller = fp->func();
      assert(g_unwind_rds.isInit());
      vmpc() = caller->unit()->at(caller->base() +
                                  g_unwind_rds->debuggerReturnOff);
      FTRACE(3, "REQ_DEBUGGER_RET: pc {} in {}\n",
             vmpc(), fp->func()->fullName()->data());
      sk = SrcKey{fp->func(), vmpc(), fp->resumed()};
      start = getTranslation(TransArgs{sk}).tca();
      break;
    }
  }

  if (smashed && info.stub) {
    Treadmill::enqueue(FreeRequestStubTrigger(info.stub));
  }

  if (start == nullptr) {
    vmpc() = sk.unit()->at(sk.offset());
    start = ustubs().interpHelperSyncedPC;
  }

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = sk.valid() ? sk.toAtomicInt() : uint64_t(-1LL);
    Trace::ringbufferEntry(RBTypeResumeTC, skData, (uint64_t)start);
  }

  tl_regState = VMRegState::DIRTY;
  return start;
}

TCA MCGenerator::handleBindCall(TCA toSmash,
                                ActRec* calleeFrame,
                                bool isImmutable) {
  Func* func = const_cast<Func*>(calleeFrame->m_func);
  int nArgs = calleeFrame->numArgs();
  TRACE(2, "bindCall %s, ActRec %p\n", func->fullName()->data(), calleeFrame);
  TCA start = getFuncPrologue(func, nArgs);
  TRACE(2, "bindCall -> %p\n", start);
  if (start && !isImmutable) {
    // We dont know we're calling the right function, so adjust start to point
    // to the dynamic check of ar->m_func.
    start = funcGuardFromPrologue(start, func);
  } else {
    TRACE(2, "bindCall immutably %s -> %p\n", func->fullName()->data(), start);
  }

  if (start && !RuntimeOption::EvalFailJitPrologs) {
    LeaseHolder writer(Translator::WriteLease(), func, TransKind::Profile);
    if (!writer) return start;

    // Someone else may have changed the func prologue while we waited for
    // the write lease, so read it again.
    start = getFuncPrologue(func, nArgs);
    if (start && !isImmutable) start = funcGuardFromPrologue(start, func);

    auto codeLock = lockCode();

    if (start && smashableCallTarget(toSmash) != start) {
      assertx(smashableCallTarget(toSmash));
      TRACE(2, "bindCall smash %p -> %p\n", toSmash, start);
      smashCall(toSmash, start);

      bool is_profiled = false;
      // For functions to be PGO'ed, if their current prologues are still
      // profiling ones (living in code.prof()), then save toSmash as a
      // caller to the prologue, so that it can later be smashed to call a
      // new prologue when it's generated.
      int calleeNumParams = func->numNonVariadicParams();
      int calledPrologNumArgs = (nArgs <= calleeNumParams ?
                                 nArgs :  calleeNumParams + 1);
      auto const profData = jit::profData();
      if (profData != nullptr && m_code.prof().contains(start)) {
        auto rec = profData->prologueTransRec(
          func,
          calledPrologNumArgs
        );
        if (isImmutable) {
          rec->addMainCaller(toSmash);
        } else {
          rec->addGuardCaller(toSmash);
        }
        is_profiled = true;
      }

      // We need to be able to reclaim the function prologues once the unit
      // associated with this function is treadmilled-- so record all of the
      // callers that will need to be re-smashed
      //
      // Additionally for profiled calls we need to remove them from the main
      // and guard caller maps.
      if (RuntimeOption::EvalEnableReusableTC) {
        if (debug || is_profiled || !isImmutable) {
          auto metaLock = lockMetadata();
          recordFuncCaller(func, toSmash, isImmutable,
                           is_profiled, calledPrologNumArgs);
        }
      }
    }
  } else {
    // We couldn't get a prologue address. Return a stub that will finish
    // entering the callee frame in C++, then call handleResume at the callee's
    // entry point.
    start = ustubs().fcallHelperThunk;
  }

  return start;
}

TCA MCGenerator::handleFCallAwaitSuspend() {
  assert_native_stack_aligned();
  FTRACE(1, "handleFCallAwaitSuspend\n");

  tl_regState = VMRegState::CLEAN;

  vmJitCalledFrame() = vmfp();
  SCOPE_EXIT { vmJitCalledFrame() = nullptr; };

  auto start = suspendStack(vmpc());
  tl_regState = VMRegState::DIRTY;
  return start ? start : ustubs().resumeHelper;
}

TCA MCGenerator::handleResume(bool interpFirst) {
  assert_native_stack_aligned();
  FTRACE(1, "handleResume({})\n", interpFirst);

  if (!vmRegsUnsafe().pc) return ustubs().callToExit;

  tl_regState = VMRegState::CLEAN;

  auto sk = SrcKey{liveFunc(), vmpc(), liveResumed()};
  TCA start;
  if (interpFirst) {
    start = nullptr;
    INC_TPC(interp_bb_force);
  } else {
    start = getTranslation(TransArgs(sk)).tca();
  }

  vmJitCalledFrame() = vmfp();
  SCOPE_EXIT { vmJitCalledFrame() = nullptr; };

  // If we can't get a translation at the current SrcKey, interpret basic
  // blocks until we end up somewhere with a translation (which we may have
  // created, if the lease holder dropped it).
  while (!start) {
    INC_TPC(interp_bb);
    if (auto retAddr = HPHP::dispatchBB()) {
      start = retAddr;
      break;
    }

    assertx(vmpc());
    sk = SrcKey{liveFunc(), vmpc(), liveResumed()};
    start = getTranslation(TransArgs{sk}).tca();
  }

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = sk.valid() ? sk.toAtomicInt() : uint64_t(-1LL);
    Trace::ringbufferEntry(RBTypeResumeTC, skData, (uint64_t)start);
  }

  tl_regState = VMRegState::DIRTY;
  return start;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Support for the stub freelist.
 */
TCA FreeStubList::maybePop() {
  StubNode* ret = m_list;
  if (ret) {
    TRACE(1, "alloc stub %p\n", ret);
    m_list = ret->m_next;
    ret->m_freed = ~kStubFree;
  }
  return (TCA)ret;
}

void FreeStubList::push(TCA stub) {
  /*
   * A freed stub may be released by Treadmill more than once if multiple
   * threads execute the service request before it is freed. We detect
   * duplicates by marking freed stubs
   */
  StubNode* n = reinterpret_cast<StubNode*>(stub);
  if (n->m_freed == kStubFree) {
    TRACE(1, "already freed stub %p\n", stub);
    return;
  }
  n->m_freed = kStubFree;
  n->m_next = m_list;
  TRACE(1, "free stub %p (-> %p)\n", stub, m_list);
  m_list = n;
}

void MCGenerator::freeRequestStub(TCA stub) {
  // We need to lock the code because m_freeStubs.push() writes to the stub and
  // the metadata to protect m_freeStubs itself.
  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  assertx(m_code.frozen().contains(stub));
  m_debugInfo.recordRelocMap(stub, 0, "FreeStub");
  m_freeStubs.push(stub);
}

TCA MCGenerator::getFreeStub(CodeBlock& frozen, CGMeta* fixups,
                             bool* isReused) {
  TCA ret = m_freeStubs.maybePop();
  if (isReused) *isReused = ret;

  if (ret) {
    Stats::inc(Stats::Astub_Reused);
    always_assert(m_freeStubs.peek() == nullptr ||
                  m_code.isValidCodeAddress(m_freeStubs.peek()));
    TRACE(1, "recycle stub %p\n", ret);
  } else {
    ret = frozen.frontier();
    Stats::inc(Stats::Astub_New);
    TRACE(1, "alloc new stub %p\n", ret);
  }

  if (fixups) {
    fixups->reusedStubs.emplace_back(ret);
  }
  return ret;
}

void MCGenerator::syncWork() {
  assertx(tl_regState != VMRegState::CLEAN);
  m_fixupMap.fixup(g_context.getNoCheck());
  tl_regState = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}

namespace {
/*
 * Attempt to emit code for the given IRUnit to `code'. Returns true on
 * success, false if codegen failed.
 */
bool mcGenUnit(TransEnv& env, CodeCache::View code, CGMeta& fixups) {
  auto const& unit = *env.unit;
  try {
    emitVunit(*env.vunit, unit, code, fixups,
              dumpTCAnnotation(*env.args.sk.func(), env.args.kind)
              ? &env.annotations
              : nullptr);
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      mcg->code().disableHot();
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

/*
 * If TC reuse is enabled, attempt to relocate the newly-emitted translation to
 * a hole reclaimed from dead code. Returns true if the translation was
 * relocated and false otherwise.
 */
void tryRelocateNewTranslation(SrcKey sk, TransLoc& loc,
                               CodeCache::View code, CGMeta& fixups) {
  if (!RuntimeOption::EvalEnableReusableTC) return;

  TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
             cs = loc.coldStart(), ce = loc.coldEnd(),
             fs = loc.frozenStart(), fe = loc.frozenEnd();

  auto const did_relocate = relocateNewTranslation(loc, code, fixups);

  if (did_relocate) {
    FTRACE_MOD(reusetc, 1,
               "Relocated translation for func {} (id = {})  @ sk({}) "
               "from M[{}, {}], C[{}, {}], F[{}, {}] to M[{}, {}] "
               "C[{}, {}] F[{}, {}]\n",
               sk.func()->fullName()->data(), sk.func()->getFuncId(),
               sk.offset(), ms, me, cs, ce, fs, fe, loc.mainStart(),
               loc.mainEnd(), loc.coldStart(), loc.coldEnd(),
               loc.frozenStart(), loc.frozenEnd());
  } else {
    FTRACE_MOD(reusetc, 1,
               "Created translation for func {} (id = {}) "
               " @ sk({}) at M[{}, {}], C[{}, {}], F[{}, {}]\n",
               sk.func()->fullName()->data(), sk.func()->getFuncId(),
               sk.offset(), ms, me, cs, ce, fs, fe);
  }
}

/*
 * If live code relocation is enabled, record metadata for the current
 * translation.
 */
void recordRelocationMetaData(SrcKey sk, SrcRec& srcRec,
                              const TransLoc& loc, CGMeta& fixups) {
  if (!RuntimeOption::EvalPerfRelocate) return;

  recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                     loc.coldCodeStart(), loc.coldEnd(),
                     sk, -1,
                     srcRec.tailFallbackJumps(),
                     fixups);
}

/*
 * If the jit maturity counter is enabled, update it with the current amount of
 * emitted code.
 */
void reportJitMaturity(const CodeCache& code) {
  // Optimized translations are faster than profiling translations, which are
  // faster than the interpreter.  But when optimized translations are
  // generated, some profiling translations will become dead.  We assume the
  // incremental value of an optimized translation over the corresponding
  // profiling translations is comparable to the incremental value of a
  // profiling translation of similar size; thus we don't have to apply
  // different weights to code in different regions.
  auto const codeSize =
    code.hot().used() + code.main().used() + code.prof().used();
  if (s_jitMaturityCounter) {
    // EvalJitMatureSize is supposed to to be set to approximately 20% of the
    // code that will give us full performance, so recover the "fully mature"
    // size with some math.
    auto const fullSize = RuntimeOption::EvalJitMatureSize * 5;
    auto const after = codeSize >= fullSize ? 100
                                            : (codeSize * 100 / fullSize);
    auto const before = s_jitMaturityCounter->getValue();
    if (after > before) s_jitMaturityCounter->setValue(after);
  }

  if (!s_loggedJitMature.load(std::memory_order_relaxed) &&
      StructuredLog::enabled() &&
      codeSize >= RuntimeOption::EvalJitMatureSize &&
      !s_loggedJitMature.exchange(true, std::memory_order_relaxed)) {
    StructuredLogEntry cols;
    cols.setInt("jit_mature_sec", time(nullptr) - HttpServer::StartTime);
    StructuredLog::log("hhvm_warmup", cols);
  }
}

// send stats about this translation to StructuredLog
void logTranslation(const TransEnv& env) {
  auto nanos = HPHP::Timer::GetThreadCPUTimeNanos() - env.unit->startNanos();
  StructuredLogEntry cols;
  auto& context = env.unit->context();
  auto kind = show(context.kind);
  cols.setStr("trans_kind", !debug ? kind : kind + "_debug");
  if (context.func) {
    cols.setStr("func", context.func->fullName()->data());
  }
  cols.setInt("jit_sample_rate", RuntimeOption::EvalJitSampleRate);
  // timing info
  cols.setInt("jit_micros", nanos / 1000);
  // hhir stats
  cols.setInt("max_tmps", env.unit->numTmps());
  cols.setInt("max_blocks", env.unit->numBlocks());
  cols.setInt("max_insts", env.unit->numInsts());
  auto hhir_blocks = rpoSortCfg(*env.unit);
  cols.setInt("num_blocks", hhir_blocks.size());
  size_t num_insts = 0;
  for (auto b : hhir_blocks) num_insts += b->instrs().size();
  cols.setInt("num_insts", num_insts);
  // vasm stats
  cols.setInt("max_vreg", env.vunit->next_vr);
  cols.setInt("max_vblocks", env.vunit->blocks.size());
  cols.setInt("max_vcalls", env.vunit->vcallArgs.size());
  size_t max_vinstr = 0;
  for (auto& blk : env.vunit->blocks) max_vinstr += blk.code.size();
  cols.setInt("max_vinstr", max_vinstr);
  cols.setInt("num_vconst", env.vunit->constToReg.size());
  auto vblocks = sortBlocks(*env.vunit);
  size_t num_vinstr[kNumAreas] = {0, 0, 0};
  size_t num_vblocks[kNumAreas] = {0, 0, 0};
  for (auto b : vblocks) {
    const auto& block = env.vunit->blocks[b];
    num_vinstr[(int)block.area_idx] += block.code.size();
    num_vblocks[(int)block.area_idx]++;
  }
  cols.setInt("num_vinstr_main", num_vinstr[(int)AreaIndex::Main]);
  cols.setInt("num_vinstr_cold", num_vinstr[(int)AreaIndex::Cold]);
  cols.setInt("num_vinstr_frozen", num_vinstr[(int)AreaIndex::Frozen]);
  cols.setInt("num_vblocks_main", num_vblocks[(int)AreaIndex::Main]);
  cols.setInt("num_vblocks_cold", num_vblocks[(int)AreaIndex::Cold]);
  cols.setInt("num_vblocks_frozen", num_vblocks[(int)AreaIndex::Frozen]);
  // finish & log
  StructuredLog::log("hhvm_jit", cols);
}

}

TCA MCGenerator::finishTranslation(TransEnv env) {
  Timer timer(Timer::mcg_finishTranslation);

  auto& args = env.args;
  auto const sk = args.sk;

  profileSetHotFuncAttr();

  auto codeLock = lockCode();
  auto code = m_code.view(args.kind);

  CGMeta fixups;
  TransLocMaker maker{code};
  maker.markStart();

  // mcGenUnit emits machine code from vasm
  if (env.vunit && !mcGenUnit(env, code, fixups)) {
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
    if (m_numTrans.fetch_add(1, std::memory_order_relaxed) >=
        RuntimeOption::EvalJitGlobalTranslationLimit) {
      return nullptr;
    }
  } else {
    args.kind = TransKind::Interp;
    FTRACE(1, "emitting dispatchBB interp request for failed "
           "translation (spOff = {})\n", env.initSpOffset.offset);
    vwrap(code.main(), code.data(), fixups,
          [&] (Vout& v) { emitInterpReq(v, sk, env.initSpOffset); },
          CodeKind::Helper);
  }

  Timer metaTimer(Timer::mcg_finishTranslation_metadata);
  auto metaLock = lockMetadata();
  auto loc = maker.markEnd();

  tryRelocateNewTranslation(sk, loc, code, fixups);

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

  auto const srcRec = m_srcDB.find(args.sk);
  always_assert(srcRec);
  recordRelocationMetaData(sk, *srcRec, loc, fixups);
  recordGdbTranslation(sk, sk.func(), code.main(), loc.mainStart(),
                       false, false);
  recordGdbTranslation(sk, sk.func(), code.cold(), loc.coldStart(),
                       false, false);
  if (args.kind == TransKind::Profile) {
    always_assert(args.region);
    profData()->addTransProfile(env.transID, args.region, env.pconds);
  }

  auto tr = maker.rec(sk, env.transID, args.kind, args.region, fixups.bcMap,
                      std::move(env.annotations),
                      env.unit && cfgHasLoop(*env.unit));
  m_tx.addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(sk.unit(), sk.func(), tr);
  }

  GrowableVector<IncomingBranch> inProgressTailBranches;
  fixups.process(&inProgressTailBranches);

  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: (func %d, bcOff %d)\n",
        loc.mainStart(), sk.funcID(), sk.offset());
  srcRec->newTranslation(loc, inProgressTailBranches);

  TRACE(1, "mcg: %zd-byte tracelet\n", (ssize_t)loc.mainSize());
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getTCSpace().c_str());
  }

  reportJitMaturity(m_code);

  if (env.unit) {
    auto func = env.unit->context().func;
    auto enable = func ? func->shouldSampleJit() :
                  StructuredLog::coinflip(RuntimeOption::EvalJitSampleRate);
    if (enable) logTranslation(env);
  }

  return loc.mainStart();
}

MCGenerator::MCGenerator()
  : m_numTrans(0)
  , m_catchTraceMap(128)
  , m_literals(128)
{
  TRACE(1, "MCGenerator@%p startup\n", this);
  mcg = this;

  g_unwind_rds.bind();

  static bool profileUp = false;
  if (!profileUp) {
    profileInit();
    profileUp = true;
  }

  if (Trace::moduleEnabledRelease(Trace::printir) &&
      !RuntimeOption::EvalJit) {
    Trace::traceRelease("TRACE=printir is set but the jit isn't on. "
                        "Did you mean to run with -vEval.Jit=1?\n");
  }

  s_jitMaturityCounter = ServiceData::createCounter("jit.maturity");

  m_ustubs.emitAll(m_code, m_debugInfo);

  // Write an .eh_frame section that covers the whole TC.
  EHFrameWriter ehfw;
  write_tc_cie(ehfw);
  ehfw.begin_fde(m_code.base());
  ehfw.end_fde(m_code.codeSize());
  ehfw.null_fde();

  m_ehFrames.push_back(ehfw.register_and_release());
}

folly::Optional<TCA> MCGenerator::getCatchTrace(CTCA ip) const {
  auto const found = m_catchTraceMap.find(m_code.toOffset(ip));
  if (found && *found != kInvalidCatchTrace) return m_code.toAddr(*found);
  return folly::none;
}

void codeEmittedThisRequest(size_t& requestEntry, size_t& now) {
  requestEntry = s_initialTCSize;
  now = mcg->code().totalUsed();
}

namespace {
__thread std::unordered_map<const ActRec*, TCA>* tl_debuggerCatches{nullptr};
}

void stashDebuggerCatch(const ActRec* fp) {
  if (!tl_debuggerCatches) {
    tl_debuggerCatches = new std::unordered_map<const ActRec*, TCA>();
  }

  auto optCatchBlock = mcg->getCatchTrace(TCA(fp->m_savedRip));
  always_assert(optCatchBlock && *optCatchBlock);
  auto catchBlock = *optCatchBlock;
  FTRACE(1, "Pushing debugger catch {} with fp {}\n", catchBlock, fp);
  tl_debuggerCatches->emplace(fp, catchBlock);
}

TCA unstashDebuggerCatch(const ActRec* fp) {
  always_assert(tl_debuggerCatches);
  auto const it = tl_debuggerCatches->find(fp);
  always_assert(it != tl_debuggerCatches->end());
  auto const catchBlock = it->second;
  tl_debuggerCatches->erase(it);
  FTRACE(1, "Popped debugger catch {} for fp {}\n", catchBlock, fp);
  return catchBlock;
}

void MCGenerator::requestInit() {
  tl_regState = VMRegState::CLEAN;
  Timer::RequestInit();
  memset(&tl_perf_counters, 0, sizeof(tl_perf_counters));
  Stats::init();
  requestInitProfData();
  s_initialTCSize = m_code.totalUsed();
  assert(!g_unwind_rds.isInit());
  memset(g_unwind_rds.get(), 0, sizeof(UnwindRDS));
  g_unwind_rds.markInit();
}

void MCGenerator::requestExit() {
  always_assert(!Translator::WriteLease().amOwner());
  TRACE_MOD(txlease, 2, "%" PRIx64 " write lease stats: %15" PRId64
            " kept, %15" PRId64 " grabbed\n",
            Process::GetThreadIdForTrace(), Translator::WriteLease().hintKept(),
            Translator::WriteLease().hintGrabbed());
  Stats::dump();
  Stats::clear();
  Timer::RequestExit();
  if (profData()) profData()->maybeResetCounters();
  requestExitProfData();

  if (Trace::moduleEnabledRelease(Trace::mcgstats, 1)) {
    Trace::traceRelease("MCGenerator perf counters for %s:\n",
                        g_context->getRequestUrl(50).c_str());
    for (int i = 0; i < tpc_num_counters; i++) {
      Trace::traceRelease("%-20s %10" PRId64 "\n",
                          kPerfCounterNames[i], tl_perf_counters[i]);
    }
    Trace::traceRelease("\n");
  }

  delete tl_debuggerCatches;
  tl_debuggerCatches = nullptr;
}

MCGenerator::~MCGenerator() {
}

static Debug::TCRange rangeFrom(const CodeBlock& cb, const TCA addr,
                                bool isAcold) {
  assertx(cb.contains(addr));
  return Debug::TCRange(addr, cb.frontier(), isAcold);
}

void MCGenerator::recordBCInstr(uint32_t op,
                                const TCA addr,
                                const TCA end,
                                bool cold) {
  if (addr != end) {
    m_debugInfo.recordBCInstr(
      Debug::TCRange(addr, end, cold), op);
  }
}

void MCGenerator::recordGdbTranslation(SrcKey sk,
                                       const Func* srcFunc,
                                       const CodeBlock& cb,
                                       const TCA start,
                                       bool exit,
                                       bool inPrologue) {
  if (start != cb.frontier()) {
    assertOwnsCodeLock();
    if (!RuntimeOption::EvalJitNoGdb) {
      m_debugInfo.recordTracelet(rangeFrom(cb, start, &cb == &m_code.cold()),
                                 srcFunc,
                                 srcFunc->unit() ?
                                   srcFunc->unit()->at(sk.offset()) : nullptr,
                                 exit, inPrologue);
    }
    if (RuntimeOption::EvalPerfPidMap) {
      m_debugInfo.recordPerfMap(rangeFrom(cb, start, &cb == &m_code.cold()),
                                sk, srcFunc, exit, inPrologue);
    }
  }
}

bool MCGenerator::addDbgGuards(const Unit* unit) {
  // TODO refactor
  // It grabs the write lease and iterates through whole SrcDB...
  struct timespec tsBegin, tsEnd;
  {
    auto codeLock = lockCode();
    auto metaLock = lockMetadata();

    auto code = m_code.view();
    auto& main = code.main();
    auto& data = code.data();

    HPHP::Timer::GetMonotonicTime(tsBegin);
    // Doc says even find _could_ invalidate iterator, in pactice it should
    // be very rare, so go with it now.
    CGMeta fixups;
    for (auto& pair : m_srcDB) {
      SrcKey const sk = SrcKey::fromAtomicInt(pair.first);
      // We may have a SrcKey to a deleted function. NB: this may miss a
      // race with deleting a Func. See task #2826313.
      if (!Func::isFuncIdValid(sk.funcID())) continue;
      SrcRec* sr = pair.second;
      if (sr->unitMd5() == unit->md5() &&
          !sr->hasDebuggerGuard() &&
          m_tx.isSrcKeyInBL(sk)) {
        addDbgGuardImpl(sk, sr, main, data, fixups);
      }
    }
    fixups.process(nullptr);
  }

  HPHP::Timer::GetMonotonicTime(tsEnd);
  int64_t elapsed = gettime_diff_us(tsBegin, tsEnd);
  if (Trace::moduleEnabledRelease(Trace::mcg, 5)) {
    Trace::traceRelease("addDbgGuards got lease for %" PRId64 " us\n", elapsed);
  }
  return true;
}

bool MCGenerator::addDbgGuard(const Func* func, Offset offset, bool resumed) {
  SrcKey sk(func, offset, resumed);
  {
    if (SrcRec* sr = m_srcDB.find(sk)) {
      if (sr->hasDebuggerGuard()) {
        return true;
      }
    } else {
      // no translation yet
      return true;
    }
  }
  if (debug) {
    if (!m_tx.isSrcKeyInBL(sk)) {
      TRACE(5, "calling addDbgGuard on PC that is not in blacklist");
      return false;
    }
  }

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  CGMeta fixups;
  if (SrcRec* sr = m_srcDB.find(sk)) {
    auto code = m_code.view();
    addDbgGuardImpl(sk, sr, code.main(), code.data(), fixups);
  }
  fixups.process(nullptr);
  return true;
}

bool MCGenerator::dumpTCCode(const char* filename) {
#define OPEN_FILE(F, SUFFIX)                                    \
  std::string F ## name = std::string(filename).append(SUFFIX); \
  FILE* F = fopen(F ## name .c_str(),"wb");                     \
  if (F == nullptr) return false;                               \
  SCOPE_EXIT{ fclose(F); };

  OPEN_FILE(ahotFile,       "_ahot");
  OPEN_FILE(aFile,          "_a");
  OPEN_FILE(aprofFile,      "_aprof");
  OPEN_FILE(acoldFile,      "_acold");
  OPEN_FILE(afrozenFile,    "_afrozen");
  OPEN_FILE(helperAddrFile, "_helpers_addrs.txt");

#undef OPEN_FILE

  // dump starting from the hot region
  auto result = true;
  auto writeBlock = [&](const CodeBlock& cb, FILE* file) {
    if (result) {
      auto const count = cb.used();
      result = fwrite(cb.base(), 1, count, file) == count;
    }
  };

  writeBlock(m_code.hot(), ahotFile);
  writeBlock(m_code.main(), aFile);
  writeBlock(m_code.prof(), aprofFile);
  writeBlock(m_code.cold(), acoldFile);
  writeBlock(m_code.frozen(), afrozenFile);
  return result;
}

bool MCGenerator::dumpTC(bool ignoreLease /* = false */) {
  std::unique_lock<SimpleMutex> codeLock;
  std::unique_lock<SimpleMutex> metaLock;
  if (!ignoreLease) {
    codeLock = lockCode();
    metaLock = lockMetadata();
  }
  return dumpTCData() && dumpTCCode("/tmp/tc_dump");
}

// Returns true on success
bool MCGenerator::dumpTCData() {
  gzFile tcDataFile = gzopen("/tmp/tc_data.txt.gz", "w");
  if (!tcDataFile) return false;

  if (!gzprintf(tcDataFile,
                "repo_schema      = %s\n"
                "ahot.base        = %p\n"
                "ahot.frontier    = %p\n"
                "a.base           = %p\n"
                "a.frontier       = %p\n"
                "aprof.base       = %p\n"
                "aprof.frontier   = %p\n"
                "acold.base       = %p\n"
                "acold.frontier   = %p\n"
                "afrozen.base     = %p\n"
                "afrozen.frontier = %p\n\n",
                repoSchemaId().begin(),
                m_code.hot().base(), m_code.hot().frontier(),
                m_code.main().base(), m_code.main().frontier(),
                m_code.prof().base(), m_code.prof().frontier(),
                m_code.cold().base(), m_code.cold().frontier(),
                m_code.frozen().base(), m_code.frozen().frontier())) {
    return false;
  }

  if (!gzprintf(tcDataFile, "total_translations = %zu\n\n",
                m_tx.getNumTranslations())) {
    return false;
  }

  // Print all translations, including their execution counters. If global
  // counters are disabled (default), fall back to using ProfData, covering
  // only profiling translations.
  if (!RuntimeOption::EvalJitTransCounters && Translator::isTransDBEnabled()) {
    // Admin requests do not automatically init ProfData, so do it explicitly.
    // No need for matching exit call; data is immortal with trans DB enabled.
    requestInitProfData();
  }
  for (TransID t = 0; t < m_tx.getNumTranslations(); t++) {
    int64_t count = 0;
    if (RuntimeOption::EvalJitTransCounters) {
      count = m_tx.getTransCounter(t);
    } else if (auto prof = profData()) {
      assertx(m_tx.getTransCounter(t) == 0);
      count = prof->transCounter(t);
    }
    if (gzputs(tcDataFile, m_tx.getTransRec(t)->print(count).c_str()) == -1) {
      return false;
    }
  }

  gzclose(tcDataFile);
  return true;
}

void MCGenerator::invalidateSrcKey(SrcKey sk) {
  assertx(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  auto const sr = m_srcDB.find(sk);
  always_assert(sr);
  /*
   * Since previous translations aren't reachable from here, we know we
   * just created some garbage in the TC. We currently have no mechanism
   * to reclaim this.
   */
  FTRACE_MOD(reusetc, 1, "Replacing translations from func {} (id = {}) "
             "@ sk({}) in SrcRec addr={}\n",
             sk.func()->fullName()->data(), sk.func()->getFuncId(), sk.offset(),
             (void*)sr);
  Trace::Indent _i;
  sr->replaceOldTranslations();
}

///////////////////////////////////////////////////////////////////////////////

}}
