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
#include <sys/mman.h>
#include <unistd.h>
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

#include "hphp/util/abi-cxx.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/bitops.h"
#include "hphp/util/cycles.h"
#include "hphp/util/debug.h"
#include "hphp/util/disasm.h"
#include "hphp/util/eh-frame.h"
#include "hphp/util/logger.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/meta.h"
#include "hphp/util/process.h"
#include "hphp/util/rank.h"
#include "hphp/util/build-info.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/arch.h"
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
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/source-root-info.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(mcg);

using namespace Trace;

// The global MCGenerator object.
MCGenerator* mcg;

static __thread size_t s_initialTCSize;
static ServiceData::ExportedCounter* s_jitMaturityCounter;

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
      TransKind                   kind,
      RegionDescPtr               region  = RegionDescPtr(),
      std::vector<TransBCMapping> bcmap   = std::vector<TransBCMapping>(),
      Annotations&&               annot   = Annotations(),
      bool                        hasLoop = false) const {
    auto& cold = cache.cold();
    auto& frozen = cache.frozen();
    TCA coldStart = cold.frontier();
    TCA frozenStart = frozen.frontier();
    size_t coldSize = 0;
    size_t frozenSize = 0;

    if (&cache.cold() == &cold) {
      coldStart = loc.coldCodeStart();
      coldSize  = loc.coldCodeSize();
    }
    if (&cache.frozen() == &frozen) {
      frozenStart = loc.frozenCodeStart();
      frozenSize  = loc.frozenCodeSize();
    }

    return TransRec(sk, kind,
                    loc.mainStart(), loc.mainSize(),
                    coldStart, coldSize,
                    frozenStart, frozenSize,
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
  if (!RuntimeOption::EvalJitPGO) return false;

  // JITing pseudo-mains requires extra checks that blow the IR.  PGO
  // can significantly increase the size of the regions, so disable it for
  // pseudo-mains (so regions will be just tracelets).
  if (func.isPseudoMain()) return false;

  if (!RuntimeOption::EvalJitPGOHotOnly) return true;
  return func.attrs() & AttrHot;
}

bool MCGenerator::profileSrcKey(SrcKey sk) const {
  if (!shouldPGOFunc(*sk.func())) return false;
  if (m_tx.profData()->optimized(sk.funcID())) return false;
  if (m_tx.profData()->profiling(sk.funcID())) return true;

  // Don't start profiling new functions if the size of either main or
  // prof is already above Eval.JitAMaxUsage.
  auto tcUsage = std::max(m_code.main().used(), m_code.prof().used());
  if (tcUsage >= CodeCache::AMaxUsage) {
    return false;
  }

  return requestCount() <= RuntimeOption::EvalJitProfileRequests;
}

/*
 * Invalidate the SrcDB entries for func's SrcKeys that have any
 * Profile translation.
 */
void MCGenerator::invalidateFuncProfSrcKeys(const Func* func) {
  assertx(RuntimeOption::EvalJitPGO);
  FuncId funcId = func->getFuncId();
  for (auto tid : m_tx.profData()->funcProfTransIDs(funcId)) {
    invalidateSrcKey(m_tx.profData()->transRec(tid)->srcKey());
  }
}

TCA MCGenerator::retranslate(const TranslArgs& args) {
  auto sr = m_tx.getSrcDB().find(args.sk);
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
  LeaseHolder writer(Translator::WriteLease());
  if (!writer || !shouldTranslate(args.sk.func(), args.kind)) return nullptr;
  if (!locked) {
    // Even though we knew above that we were going to skip
    // doing another translation, we wait until we get the
    // write lease, to avoid spinning through the tracelet
    // guards again and again while another thread is writing
    // to it.
    return sr->getTopTranslation();
  }
  if (sr->translations().size() > RuntimeOption::EvalJitMaxTranslations) {
    always_assert(sr->translations().size() ==
                  RuntimeOption::EvalJitMaxTranslations + 1);
    return sr->getTopTranslation();
  }
  SKTRACE(1, args.sk, "retranslate\n");

  auto newArgs = args;
  newArgs.kind = profileSrcKey(args.sk) ? TransKind::Profile : TransKind::Live;
  auto start = translate(newArgs);

  // In PGO mode, we free all the profiling data once the TC is full.
  if (RuntimeOption::EvalJitPGO &&
      m_code.main().used() >= CodeCache::AMaxUsage) {
    m_tx.profData()->free();
  }

  return start;
}

TCA MCGenerator::retranslateOpt(TransID transId, bool align) {
  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;
  if (isDebuggerAttachedProcess()) return nullptr;

  TRACE(1, "retranslateOpt: transId = %u\n", transId);

  auto rec = m_tx.profData()->transRec(transId);
  if (!rec) return nullptr;

  always_assert(rec->region() != nullptr);

  auto func   = rec->func();
  auto funcId = func->getFuncId();
  auto sk     = rec->srcKey();

  if (m_tx.profData()->optimized(funcId)) return nullptr;
  m_tx.profData()->setOptimized(funcId);

  bool setFuncBody = func->getDVFunclets().size() == 0;

  func->setFuncBody(ustubs().funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  TCA start = regeneratePrologues(func, sk);

  // Regionize func and translate all its regions.
  std::vector<RegionDescPtr> regions;
  regionizeFunc(func, this, regions);

  for (auto region : regions) {
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto translArgs = TranslArgs{regionSk, align};
    translArgs.region = region;
    translArgs.kind = TransKind::Optimize;

    if (setFuncBody && regionSk.offset() == func->base()) {
      translArgs.setFuncBody = true;
      setFuncBody = false;
    }
    auto regionStart = translate(translArgs);
    if (start == nullptr && regionSk == sk) {
      start = regionStart;
    }
  }

  // In PGO mode, we free all the profiling data once the TC is full.
  if (RuntimeOption::EvalJitPGO &&
      m_code.main().used() >= CodeCache::AMaxUsage) {
    m_tx.profData()->free();
  }

  return start;
}

static bool liveFrameIsPseudoMain() {
  auto const ar = vmfp();
  if (!(ar->func()->attrs() & AttrMayUseVV)) return false;
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

/*
 * Find or create a translation for sk. Returns TCA of "best" current
 * translation. May return NULL if it is currently impossible to create
 * a translation.
 */
TCA
MCGenerator::getTranslation(const TranslArgs& args) {
  auto sk = args.sk;
  sk.func()->validate();
  SKTRACE(2, sk,
          "getTranslation: curUnit %s funcId %x offset %d\n",
          sk.unit()->filepath()->data(),
          sk.funcID(),
          sk.offset());
  SKTRACE(2, sk, "   funcId: %x \n", sk.func()->getFuncId());

  if (liveFrameIsPseudoMain() && !RuntimeOption::EvalJitPseudomain) {
    SKTRACE(2, sk, "punting on pseudoMain\n");
    return nullptr;
  }
  if (const SrcRec* sr = m_tx.getSrcDB().find(sk)) {
    TCA tca = sr->getTopTranslation();
    if (tca) {
      SKTRACE(2, sk, "getTranslation: found %p\n", tca);
      return tca;
    }
  }
  return createTranslation(args);
}

int MCGenerator::numTranslations(SrcKey sk) const {
  if (const SrcRec* sr = m_tx.getSrcDB().find(sk)) {
    return sr->translations().size();
  }
  return 0;
}

const StaticString
  s_php_errormsg("php_errormsg"),
  s_http_response_header("http_response_header");

bool MCGenerator::shouldTranslateNoSizeLimit(const Func* func) const {
  // If we've hit Eval.JitGlobalTranslationLimit, then we stop translating.
  if (m_numTrans >= RuntimeOption::EvalJitGlobalTranslationLimit) {
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
  // Otherwise, follow the Eval.JitAMaxUsage limit.  However, we do
  // allow Optimize translations past that limit.
  return m_code.main().used() < CodeCache::AMaxUsage ||
    kind == TransKind::Optimize;
}


static void populateLiveContext(RegionContext& ctx) {
  using L = RegionDesc::Location;

  auto const fp = vmfp();
  auto const sp = vmsp();

  always_assert(ctx.func == fp->m_func);

  // Track local types.
  for (uint32_t i = 0; i < fp->m_func->numLocals(); ++i) {
    ctx.liveTypes.push_back(
      { L::Local{i}, typeFromTV(frame_local(fp, i)) }
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
        { L::Stack{ctx.spOffset - stackOff}, typeFromTV(tv) }
      );
      stackOff++;
      FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
    }
  );
}

TCA
MCGenerator::createTranslation(const TranslArgs& args) {
  if (!shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  /*
   * Try to become the writer. We delay this until we *know* we will have
   * a need to create new translations, instead of just trying to win the
   * lottery at the dawn of time. Hopefully lots of requests won't require
   * any new translation.
   */
  auto sk = args.sk;
  LeaseHolder writer(Translator::WriteLease());
  if (!writer || !shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  if (auto sr = m_tx.getSrcDB().find(sk)) {
    TCA tca = sr->getTopTranslation();
    if (tca) {
      // Handle extremely unlikely race; someone may have just already
      // added the first instance of this SrcRec while we did a
      // non-blocking wait on the write lease.
      return tca;
    }

    // Since we are holding the write lease, we know that sk is properly
    // initialized, except that it has no translations (due to
    // replaceOldTranslations)
    return retranslate(args);
  }

  if (RuntimeOption::EvalFailJitPrologs && sk.op() == Op::FCallAwait) {
    return nullptr;
  }
  auto const srcRecSPOff = [&] () -> folly::Optional<FPInvOffset> {
    if (sk.resumed()) return folly::none;
    return liveSpOff();
  }();

  // We put retranslate requests at the end of our slab to more frequently
  // allow conditional jump fall-throughs
  auto code = m_code.view();
  TCA astart      = code.main().frontier();
  TCA coldStart   = code.cold().frontier();
  TCA frozenStart = code.frozen().frontier();
  TCA req;
  if (!RuntimeOption::EvalEnableReusableTC) {
    req = svcreq::emit_persistent(code.cold(),
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
                                 (TCA)newStart,
                                 srcRecSPOff,
                                 REQ_RETRANSLATE,
                                 sk.offset(),
                                 TransFlags().packed);
  }
  SKTRACE(1, sk, "inserting anchor translation for (%p,%d) at %p\n",
          sk.unit(), sk.offset(), req);
  SrcRec* sr = m_tx.getSrcRec(sk);
  sr->setFuncInfo(sk.func());
  sr->setAnchorTranslation(req);

  if (srcRecSPOff) always_assert(sr->nonResumedSPOff() == *srcRecSPOff);

  size_t asize      = code.main().frontier()   - astart;
  size_t coldSize   = code.cold().frontier()   - coldStart;
  size_t frozenSize = code.frozen().frontier() - frozenStart;
  assertx(asize == 0);
  if (coldSize && RuntimeOption::EvalDumpTCAnchors) {
    TransRec tr(sk,
                TransKind::Anchor,
                astart, asize, coldStart, coldSize,
                frozenStart, frozenSize);
    m_tx.addTranslation(tr);
    if (RuntimeOption::EvalJitUseVtuneAPI) {
      reportTraceletToVtune(sk.unit(), sk.func(), tr);
    }

    assertx(!m_tx.isTransDBEnabled() ||
           m_tx.getTransRec(coldStart)->kind == TransKind::Anchor);
  }

  return retranslate(args);
}

TCA
MCGenerator::lookupTranslation(SrcKey sk) const {
  if (SrcRec* sr = m_tx.getSrcDB().find(sk)) {
    return sr->getTopTranslation();
  }
  return nullptr;
}

TCA
MCGenerator::translate(const TranslArgs& args) {
  INC_TPC(translate);

  assert(args.kind != TransKind::Invalid);
  assertx(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

  if (!shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  auto start = translateWork(args);

  if (args.setFuncBody) {
    const_cast<Func*>(args.sk.func())->setFuncBody(start);
  }
  SKTRACE(1, args.sk, "translate moved head from %p to %p\n",
          getTopTranslation(args.sk), start);
  return start;
}

TCA MCGenerator::getFuncBody(Func* func) {
  TCA tca = func->getFuncBody();
  if (tca != ustubs().funcBodyHelperThunk) return tca;

  DVFuncletsVec dvs = func->getDVFunclets();

  if (dvs.size()) {
    LeaseHolder writer(Translator::WriteLease());
    if (!writer) return nullptr;
    tca = func->getFuncBody();
    if (tca != ustubs().funcBodyHelperThunk) return tca;
    tca = genFuncBodyDispatch(func, dvs, m_code.view());
    func->setFuncBody(tca);
  } else {
    SrcKey sk(func, func->base(), false);
    auto args = TranslArgs{sk, false};
    args.setFuncBody = true;
    tca = mcg->getTranslation(args);
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

TCA MCGenerator::emitFuncPrologue(Func* func, int argc) {
  const int nparams = func->numNonVariadicParams();
  const int paramIndex = argc <= nparams ? argc : nparams + 1;

  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(argc), false};
  auto const kind = profileSrcKey(funcBody) ? TransKind::Proflogue
                                            : TransKind::Prologue;

  profileSetHotFuncAttr();
  auto code = m_code.view(func->attrs() & AttrHot, kind);
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
  auto transID = m_tx.profData()
    ? m_tx.profData()->addTransProflogue(funcBody, paramIndex)
    : kInvalidTransID;

  TCA start = genFuncPrologue(transID, kind, func, argc, code, fixups);

  auto loc = maker.markEnd();
  if (RuntimeOption::EvalEnableReusableTC) {
    TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
               cs = loc.coldStart(), ce = loc.coldEnd(),
               fs = loc.frozenStart(), fe = loc.frozenEnd(),
               oldStart = start;
    bool did_relocate = relocateNewTranslation(loc, code, fixups, &start);

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

  assertx(kind == TransKind::Prologue || kind == TransKind::Proflogue);

  auto tr = maker.rec(funcBody, kind);
  m_tx.addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(func->unit(), func, tr);
  }


  recordGdbTranslation(funcBody, func, code.main(), aStart, false, true);
  recordBCInstr(OpFuncPrologue, loc.mainStart(), loc.mainEnd(), false);

  m_numTrans++;
  assertx(m_numTrans <= RuntimeOption::EvalJitGlobalTranslationLimit);

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

  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;

  // If we're regenerating a prologue, and we want to check shouldTranslate()
  // but ignore the code size limits.  We still want to respect the global
  // translation limit and other restrictions, though.
  if (forRegeneratePrologue) {
    if (!shouldTranslateNoSizeLimit(func)) return nullptr;
  } else {
    if (!shouldTranslate(func, TransKind::Prologue)) return nullptr;
  }

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  try {
    return emitFuncPrologue(func, nPassed);
  } catch (const DataBlockFull& dbFull) {

    // Fail hard if the block isn't code.hot.
    always_assert_flog(dbFull.name == "hot",
                       "data block = {}\nmessage: {}\n",
                       dbFull.name, dbFull.what());

    // Otherwise, fall back to code.main and retry.
    m_code.disableHot();
    try {
      return emitFuncPrologue(func, nPassed);
    } catch (const DataBlockFull& dbFull) {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }
}

/**
 * Given the proflogueTransId for a TransProflogue translation,
 * regenerate the prologue (as a TransPrologue).  Returns the starting
 * address for the translation corresponding to triggerSk, if such
 * translation is generated; otherwise returns nullptr.
 */
TCA MCGenerator::regeneratePrologue(TransID prologueTransId, SrcKey triggerSk) {
  auto rec = m_tx.profData()->transRec(prologueTransId);
  auto func = rec->func();
  auto nArgs = rec->prologueArgs();

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
      auto funcletTransId = m_tx.profData()->dvFuncletTransId(func, nArgs);
      if (funcletTransId != kInvalidTransID) {
        invalidateSrcKey(funcletSK);
        auto args = TranslArgs{funcletSK, false};
        args.transId = funcletTransId;
        args.kind = TransKind::Optimize;
        auto dvStart = translate(args);
        if (dvStart && !triggerSkStart && funcletSK == triggerSk) {
          triggerSkStart = dvStart;
        }
        // Flag that this translation has been retranslated, so that
        // it's not retranslated again along with the function body.
        m_tx.profData()->setOptimized(funcletSK);
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
TCA MCGenerator::regeneratePrologues(Func* func, SrcKey triggerSk) {
  TCA triggerStart = nullptr;
  std::vector<TransID> prologTransIDs;

  for (int nArgs = 0; nArgs < func->numPrologues(); nArgs++) {
    TransID tid = m_tx.profData()->proflogueTransId(func, nArgs);
    if (tid != kInvalidTransID) {
      prologTransIDs.push_back(tid);
    }
  }

  std::sort(prologTransIDs.begin(), prologTransIDs.end(),
          [&](TransID t1, TransID t2) -> bool {
            // This will sort in ascending order.
            return m_tx.profData()->transCounter(t2) >
                   m_tx.profData()->transCounter(t1);
          });

  // Next, we're going to regenerate each prologue along with its DV
  // funclet.  We consider the option of either including the DV
  // funclets in the same region as the function body or not.
  // Including them in the same region enables some type information
  // to flow and thus can eliminate some stores and type checks, but
  // it can also increase the code size by duplicating the whole
  // function body.  Therefore, we keep the DV inits separate if both
  // (a) the function has multiple proflogues, and (b) the size of the
  // function is above a certain threshold.
  //
  // The mechanism used to keep the function body separate from the DV
  // init is to temporarily mark the SrcKey for the function body as
  // already optimized.  (The region selectors break a region whenever
  // they hit a SrcKey that has already been optimized.)
  SrcKey funcBodySk(func, func->base(), false);
  if (prologTransIDs.size() > 1 &&
      func->past() - func->base() > RuntimeOption::EvalJitPGOMaxFuncSizeDupBody)
  {
    m_tx.profData()->setOptimized(funcBodySk);
  }
  SCOPE_EXIT{ m_tx.profData()->clearOptimized(funcBodySk); };

  for (TransID tid : prologTransIDs) {
    TCA start = regeneratePrologue(tid, triggerSk);
    if (triggerStart == nullptr && start != nullptr) {
      triggerStart = start;
    }
  }

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
  auto args = TranslArgs{destSk, false};
  args.flags = trflags;
  auto tDest = getTranslation(args);
  if (!tDest) return nullptr;

  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return tDest;

  SrcRec* sr = m_tx.getSrcRec(destSk);
  // The top translation may have changed while we waited for the
  // write lease, so read it again.  If it was replaced with a new
  // translation, then bind to the new one.  If it was invalidated,
  // then don't bind the jump.
  tDest = sr->getTopTranslation();
  if (tDest == nullptr) return nullptr;

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

  x64::DecodedInstruction di(toSmash);
  if (di.isBranch() && !di.isJmp()) {
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

/*
 * When we end a tracelet with a conditional jump, emitCondJmp first emits:
 *
 *   1:         j<CC> stubJmpccFirst
 *              jmp   stubJmpccFirst
 *
 * Our "taken" argument tells us whether the branch at 1: was taken or
 * not; and therefore which of offTaken and offNotTaken to continue executing.
 * If we did take the branch, we now rewrite the code so that the branch is
 * straightened. This predicts that subsequent executions will go the same way
 * as the first execution.
 *
 *              jn<CC> stubJmpccSecond:offNotTaken
 *              nop5   ; fallthru, or jmp if there's already a translation.
 * offTaken:
 *
 * If we did not take the branch, we leave the sense of the condition
 * intact, while patching it up to go to the unexplored code:
 *
 *              j<CC> stubJmpccSecond:offTaken
 *              nop5
 * offNotTaken:
 */
TCA
MCGenerator::bindJccFirst(TCA jccAddr, SrcKey skTaken, SrcKey skNotTaken,
                          bool taken, bool& smashed) {
  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;

  auto const skWillExplore = taken ? skTaken : skNotTaken;
  auto const skWillDefer = taken ? skNotTaken : skTaken;
  auto const dest = skWillExplore;
  auto cc = smashableJccCond(jccAddr);

  TRACE(3, "bindJccFirst: explored %d, will defer %d; "
           "overwriting cc%02x taken %d\n",
        skWillExplore.offset(), skWillDefer.offset(), cc, taken);
  always_assert(skTaken.resumed() == skNotTaken.resumed());

  // We want the branch to point to whichever side has not been explored yet.
  if (taken) cc = ccNegate(cc);

  auto& cb = m_code.blockFor(jccAddr);

  // It's not clear where the IncomingBranch should go to if cb is frozen.
  assertx(&cb != &m_code.frozen());

  auto const jmpAddr = jccAddr + smashableJccLen();
  auto const afterAddr = jmpAddr + smashableJmpLen();

  // Can we just directly fall through?
  bool const fallThru = afterAddr == cb.frontier() &&
                        !m_tx.getSrcDB().find(dest);

  auto const tDest = getTranslation(TranslArgs{dest, !fallThru});
  if (!tDest) return nullptr;

  auto const jmpTarget = smashableJmpTarget(jmpAddr);
  if (jmpTarget != smashableJccTarget(jccAddr)) {
    // Someone else already smashed this one.  Ideally we would just re-execute
    // from jccAddr---except the status flags will have been trashed.
    return tDest;
  }

  CGMeta fixups;

  auto const stub = svcreq::emit_bindjmp_stub(
    m_code.view().frozen(),
    fixups,
    liveSpOff(),
    jccAddr,
    skWillDefer,
    TransFlags{}
  );

  fixups.process(nullptr);
  smashed = true;
  assertx(Translator::WriteLease().amOwner());

  /*
   * Roll over the jcc and the jmp/fallthru. E.g., from:
   *
   *     toSmash:    jcc   <jmpccFirstStub>
   *     toSmash+6:  jmp   <jmpccFirstStub>
   *     toSmash+11: <probably the new translation == tDest>
   *
   * to:
   *
   *     toSmash:    j[n]z <jmpccSecondStub>
   *     toSmash+6:  nop5
   *     toSmash+11: newHotness
   */
  smashJcc(jccAddr, stub, cc);
  m_tx.getSrcRec(dest)->chainFrom(IncomingBranch::jmpFrom(jmpAddr));

  TRACE(5, "bindJccFirst: overwrote with cc%02x taken %d\n", cc, taken);
  return tDest;
}

namespace {

struct FreeRequestStubTrigger {
  explicit FreeRequestStubTrigger(TCA stub) : m_stub(stub) {
    TRACE(3, "FreeStubTrigger @ %p, stub %p\n", this, m_stub);
  }
  void operator()() {
    TRACE(3, "FreeStubTrigger: Firing @ %p , stub %p\n", this, m_stub);
    if (mcg->freeRequestStub(m_stub) != true) {
      // If we can't free the stub, enqueue again to retry.
      TRACE(3, "FreeStubTrigger: write lease failed, requeueing %p\n", m_stub);
      Treadmill::enqueue(FreeRequestStubTrigger(m_stub));
    }
  }
private:
  TCA m_stub;
};

}

#ifdef DEBUG

struct DepthGuard {
  static __thread int m_depth;
  DepthGuard()  { m_depth++; TRACE(2, "DepthGuard: %d {\n", m_depth); }
  ~DepthGuard() { TRACE(2, "DepthGuard: %d }\n", m_depth); m_depth--; }

  bool depthOne() const { return m_depth == 1; }
};
__thread int DepthGuard::m_depth;

#else

struct DepthGuard { bool depthOne() const { return false; } };

#endif

void
MCGenerator::enterTC(TCA start, ActRec* stashedAR) {
  if (debug) {
    fflush(stdout);
    fflush(stderr);
  }
  DepthGuard d;

  assertx(m_code.isValidCodeAddress(start));
  assertx(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

  Translator::WriteLease().gremlinUnlock();
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

  if (debug) {
    // Debugging code: cede the write lease half the time.
    if (RuntimeOption::EvalJitStressLease) {
      if (d.depthOne() && (rand() % 2) == 0) {
        Translator::WriteLease().gremlinLock();
      }
    }
  }

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

    case REQ_BIND_JCC_FIRST: {
      auto toSmash = info.args[0].tca;
      auto skTaken = SrcKey::fromAtomicInt(info.args[1].sk);
      auto skNotTaken = SrcKey::fromAtomicInt(info.args[2].sk);
      auto taken = info.args[3].boolVal;
      sk = taken ? skTaken : skNotTaken;
      start = bindJccFirst(toSmash, skTaken, skNotTaken, taken, smashed);
      break;
    }

    case REQ_RETRANSLATE: {
      INC_TPC(retranslate);
      sk = SrcKey{liveFunc(), info.args[0].offset, liveResumed()};
      auto trflags = info.args[1].trflags;
      auto args = TranslArgs{sk, true};
      args.flags = trflags;
      start = retranslate(args);
      SKTRACE(2, sk, "retranslated @%p\n", start);
      break;
    }

    case REQ_RETRANSLATE_OPT: {
      sk = SrcKey::fromAtomicInt(info.args[0].sk);
      auto transID = info.args[1].transID;
      start = retranslateOpt(transID, false);
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
      start = getTranslation(TranslArgs{sk, true});
      TRACE(3, "REQ_POST_INTERP_RET: from %s to %s\n",
            ar->m_func->fullName()->data(),
            caller->m_func->fullName()->data());
      break;
    }

    case REQ_POST_DEBUGGER_RET: {
      auto fp = vmfp();
      auto caller = fp->func();
      vmpc() = caller->unit()->at(caller->base() +
                                  g_unwind_rds->debuggerReturnOff);
      FTRACE(3, "REQ_DEBUGGER_RET: pc {} in {}\n",
             vmpc(), fp->func()->fullName()->data());
      sk = SrcKey{fp->func(), vmpc(), fp->resumed()};
      start = getTranslation(TranslArgs{sk, true});
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
    LeaseHolder writer(Translator::WriteLease());
    if (writer) {
      // Someone else may have changed the func prologue while we waited for
      // the write lease, so read it again.
      start = getFuncPrologue(func, nArgs);
      if (start && !isImmutable) start = funcGuardFromPrologue(start, func);

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
        if (m_code.prof().contains(start) && !m_tx.profData()->freed()) {
          auto rec = m_tx.profData()->prologueTransRec(
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
        if (RuntimeOption::EvalEnableReusableTC) {
          if (debug || !isImmutable) {
            recordFuncCaller(func, toSmash, isImmutable,
                             is_profiled, calledPrologNumArgs);
          }
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
    start = getTranslation(TranslArgs(sk, true));
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
    start = getTranslation(TranslArgs{sk, true});
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

bool
MCGenerator::freeRequestStub(TCA stub) {
  LeaseHolder writer(Translator::WriteLease());
  /*
   * If we can't acquire the write lock, the caller
   * (FreeRequestStubTrigger) retries
   */
  if (!writer) return false;
  assertx(m_code.frozen().contains(stub));
  m_debugInfo.recordRelocMap(stub, 0, "FreeStub");
  m_freeStubs.push(stub);
  return true;
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

// Get the address of the literal val in the global data section.
// If it's not there, add it to the map in m_fixups, which will
// be committed to literals when m_fixups.process() is called.
const uint64_t*
MCGenerator::allocLiteral(uint64_t val, CGMeta& fixups) {
  auto it = m_literals.find(val);
  if (it != m_literals.end()) {
    assertx(*it->second == val);
    return it->second;
  }
  auto& pending = fixups.literals;
  it = pending.find(val);
  if (it != pending.end()) {
    assertx(*it->second == val);
    return it->second;
  }
  auto addr = allocData<uint64_t>(sizeof(uint64_t), 1);
  *addr = val;
  return pending[val] = addr;
}

static bool reachedTranslationLimit(SrcKey sk, const SrcRec& srcRec) {
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

namespace {

/*
 * Analyze the given TranslArgs and return the region to translate, or nullptr
 * if one could not be selected.
 */
RegionDescPtr prepareRegion(const TranslArgs& args) {
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

/*
 * Attempt to emit code for the given IRUnit to `code'. Returns true on
 * success, false if codegen failed.
 */
bool mcGenUnit(const IRUnit& unit, CodeCache::View code, CGMeta& fixups) {
  try {
    irlower::genCode(unit, code, fixups);
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      mcg->code().disableHot();
      return false;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  } catch (const FailedTraceGen& e) {
    // Codegen failed for some other reason, most likely because xls wanted too
    // many spill slots.
    FTRACE(1, "codegen failed with {}\n", e.what());
    return false;
  }

  auto const startSk = unit.context().srcKey();
  if (unit.context().kind == TransKind::Profile) {
    mcg->tx().profData()->setProfiling(startSk.func()->getFuncId());
  }

  return true;
}

/*
 * If TC reuse is enabled, attempt to relocate the newly-emitted translation to
 * a hole reclaimed from dead code. Returns true if the translation was
 * relocated and false otherwise.
 */
bool tryRelocateNewTranslation(SrcKey sk, TransLoc& loc,
                               CodeCache::View code, CGMeta& fixups) {
  if (!RuntimeOption::EvalEnableReusableTC) return false;

  TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
             cs = loc.coldStart(), ce = loc.coldEnd(),
             fs = loc.frozenStart(), fe = loc.frozenEnd();
  bool did_relocate = relocateNewTranslation(loc, code, fixups);

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

  assertx(did_relocate == (loc.mainStart() != ms));
  return did_relocate;
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
  int32_t after = code.main().used() * 100 / CodeCache::AMaxUsage;
  if (after > 100) after = 100;
  if (s_jitMaturityCounter) {
    int32_t before = s_jitMaturityCounter->getValue();
    if (after > before) {
      s_jitMaturityCounter->setValue(after);
      constexpr int32_t kThreshold = 15;
      if (StructuredLog::enabled() &&
          before < kThreshold && kThreshold <= after) {
        std::map<std::string, int64_t> cols;
        cols["jit_mature_sec"] = time(nullptr) - HttpServer::StartTime;
        StructuredLog::log("hhvm_warmup", cols);
      }
    }
  }
}
}

TCA MCGenerator::translateWork(const TranslArgs& args) {
  Timer _t(Timer::translate);

  auto const sk = args.sk;
  assertx(m_tx.getSrcDB().find(sk));
  SrcRec& srcRec = *m_tx.getSrcRec(sk);

  auto region = reachedTranslationLimit(sk, srcRec) ? nullptr
                                                    : prepareRegion(args);
  auto const initSpOffset = region ? region->entry()->initialSpOffset()
                                   : liveSpOff();

  std::unique_ptr<IRUnit> unit;
  PostConditions pconds;
  Annotations annotations;

  // First, lower the RegionDesc to an IRUnit.
  if (region) {
    auto const profTransID = RuntimeOption::EvalJitPGO
      ? m_tx.profData()->curTransID()
      : kInvalidTransID;
    auto const transContext =
      TransContext{profTransID, args.kind, args.flags, args.sk, initSpOffset};

    unit = irGenRegion(*region, transContext, pconds, annotations);
  }

  // Next, lower the IRUnit to vasm and emit machine code from that.
  profileSetHotFuncAttr();
  auto code = m_code.view(args.sk.func()->attrs() & AttrHot, args.kind);
  auto const preAlignMain = code.main().frontier();

  if (args.align) {
    // Align without registering fixups. Codegen may fail and cause us to clear
    // the partially-populated fixups, so we wait until after that to manually
    // add the alignment fixup.
    align(code.main(), nullptr, Alignment::CacheLine, AlignContext::Dead);
  }

  CGMeta fixups;
  TransLocMaker maker{code};
  maker.markStart();

  if (unit && !mcGenUnit(*unit, code, fixups)) {
    // mcGenUnit() failed. Roll back, drop the unit and region, and clear
    // fixups.
    maker.rollback();
    maker.markStart();
    unit.reset();
    region.reset();
    fixups.clear();
  }

  auto kind = args.kind;
  if (unit) {
    m_numTrans++;
    assertx(m_numTrans <= RuntimeOption::EvalJitGlobalTranslationLimit);
  } else {
    kind = TransKind::Interp;
    FTRACE(1, "emitting dispatchBB interp request for failed "
           "translation (spOff = {})\n", initSpOffset.offset);
    vwrap(code.main(), fixups,
          [&] (Vout& v) { emitInterpReq(v, sk, initSpOffset); },
          CodeKind::Helper);
  }

  auto loc = maker.markEnd();

  if (args.align) {
    fixups.alignments.emplace(
      loc.mainStart(),
      std::make_pair(Alignment::CacheLine, AlignContext::Dead)
    );
  }

  if (tryRelocateNewTranslation(sk, loc, code, fixups)) {
    code.main().setFrontier(preAlignMain);
  }

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

  recordRelocationMetaData(sk, srcRec, loc, fixups);
  recordGdbTranslation(sk, sk.func(), code.main(), loc.mainStart(),
                       false, false);
  recordGdbTranslation(sk, sk.func(), code.cold(), loc.coldStart(),
                       false, false);
  if (RuntimeOption::EvalJitPGO && kind == TransKind::Profile) {
    always_assert(region);
    m_tx.profData()->addTransProfile(region, pconds);
  }

  auto tr = maker.rec(sk, kind, region, fixups.bcMap,
                      std::move(annotations), unit && cfgHasLoop(*unit));
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
  srcRec.newTranslation(loc, inProgressTailBranches);

  TRACE(1, "mcg: %zd-byte tracelet\n", (ssize_t)loc.mainSize());
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getTCSpace().c_str());
  }

  reportJitMaturity(m_code);

  return loc.mainStart();
}

MCGenerator::MCGenerator()
  : m_numTrans(0)
  , m_catchTraceMap(128)
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

  // Do not initialize JIT stubs for PPC64 - port under development
#if !defined(__powerpc64__)
  m_ustubs.emitAll(m_code, m_debugInfo);
#endif

  // Write an .eh_frame section that covers the whole TC.
  EHFrameWriter ehfw;
  write_tc_cie(ehfw);
  ehfw.begin_fde(m_code.base());
  ehfw.end_fde(m_code.codeSize());
  ehfw.null_fde();

  m_ehFrames.push_back(ehfw.register_and_release());
}

folly::Optional<TCA> MCGenerator::getCatchTrace(CTCA ip) const {
  TCA* found = m_catchTraceMap.find(ip);
  if (found && *found != kInvalidCatchTrace) return *found;
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
  s_initialTCSize = m_code.totalUsed();
}

void MCGenerator::requestExit() {
  always_assert(!Translator::WriteLease().amOwner());
  TRACE_MOD(txlease, 2, "%" PRIx64 " write lease stats: %15" PRId64
            " kept, %15" PRId64 " grabbed\n",
            Process::GetThreadIdForTrace(), Translator::WriteLease().m_hintKept,
            Translator::WriteLease().m_hintGrabbed);
  Stats::dump();
  Stats::clear();
  Timer::RequestExit();

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
    assertx(Translator::WriteLease().amOwner());
    if (!RuntimeOption::EvalJitNoGdb) {
      m_debugInfo.recordTracelet(rangeFrom(cb, start, &cb == &m_code.cold()),
                                 srcFunc,
                                 srcFunc->unit() ?
                                   srcFunc->unit()->at(sk.offset()) : nullptr,
                                 exit, inPrologue);
    }
    if (RuntimeOption::EvalPerfPidMap) {
      m_debugInfo.recordPerfMap(rangeFrom(cb, start, &cb == &m_code.cold()),
                                srcFunc, exit, inPrologue);
    }
    if(RuntimeOption::EvalPerfJitDump) {
      m_debugInfo.recordPerfJitTracelet(rangeFrom(cb, start, &cb == &m_code.cold()),
                                srcFunc, exit, inPrologue);
    }
  }
}

bool MCGenerator::addDbgGuards(const Unit* unit) {
  // TODO refactor
  // It grabs the write lease and iterates through whole SrcDB...
  struct timespec tsBegin, tsEnd;
  {
    BlockingLeaseHolder writer(Translator::WriteLease());
    auto& main = m_code.view().main();
    if (!writer) {
      return false;
    }

    HPHP::Timer::GetMonotonicTime(tsBegin);
    // Doc says even find _could_ invalidate iterator, in pactice it should
    // be very rare, so go with it now.
    CGMeta fixups;
    for (SrcDB::const_iterator it = m_tx.getSrcDB().begin();
         it != m_tx.getSrcDB().end(); ++it) {
      SrcKey const sk = SrcKey::fromAtomicInt(it->first);
      // We may have a SrcKey to a deleted function. NB: this may miss a
      // race with deleting a Func. See task #2826313.
      if (!Func::isFuncIdValid(sk.funcID())) continue;
      SrcRec* sr = it->second;
      if (sr->unitMd5() == unit->md5() &&
          !sr->hasDebuggerGuard() &&
          m_tx.isSrcKeyInBL(sk)) {
        addDbgGuardImpl(sk, sr, main, fixups);
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
    if (SrcRec* sr = m_tx.getSrcDB().find(sk)) {
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
  BlockingLeaseHolder writer(Translator::WriteLease());
  if (!writer) {
    return false;
  }

  CGMeta fixups;
  if (SrcRec* sr = m_tx.getSrcDB().find(sk)) {
    addDbgGuardImpl(sk, sr, m_code.view().main(), fixups);
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
  folly::Optional<BlockingLeaseHolder> writer;
  if (!ignoreLease) {
    writer.emplace(Translator::WriteLease());
    if (!*writer) return false;
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
                m_tx.getCurrentTransID())) {
    return false;
  }

  for (TransID t = 0; t < m_tx.getCurrentTransID(); t++) {
    if (gzputs(tcDataFile,
               m_tx.getTransRec(t)->print(m_tx.getTransCounter(t)).c_str()) ==
        -1) {
      return false;
    }
  }

  gzclose(tcDataFile);
  return true;
}

void MCGenerator::invalidateSrcKey(SrcKey sk) {
  assertx(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  assertx(Translator::WriteLease().amOwner());
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  SrcRec* sr = m_tx.getSrcDB().find(sk);
  assertx(sr);
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
