/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <unwind.h>

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
#include "hphp/util/safe-cast.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/bitops.h"
#include "hphp/util/cycles.h"
#include "hphp/util/debug.h"
#include "hphp/util/disasm.h"
#include "hphp/util/logger.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/meta.h"
#include "hphp/util/process.h"
#include "hphp/util/rank.h"
#include "hphp/util/repo-schema.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/back-end-x64.h" // XXX Layering violation.
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/debug-guards.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/recycle-tc.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translate-region.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/php-debug.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/mc-generator-internal.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(mcg);

using namespace reg;
using namespace Trace;
using std::max;

#define TPC(n) "jit_" #n,
static const char* const kPerfCounterNames[] = {
  TRANS_PERF_COUNTERS
};
#undef TPC

__thread int64_t s_perfCounters[tpc_num_counters];
static __thread size_t s_initialTCSize;

// The global MCGenerator object.
MCGenerator* mcg;

CppCall MCGenerator::getDtorCall(DataType type) {
  switch (type) {
    case KindOfString:
      return CppCall::method(&StringData::release);
    case KindOfArray:
      return CppCall::method(&ArrayData::release);
    case KindOfObject:
      return CppCall::method(
        RuntimeOption::EnableObjDestructCall
          ? &ObjectData::release
          : &ObjectData::releaseNoObjDestructCheck
      );
    case KindOfResource:
      return CppCall::method(&ResourceData::release);
    case KindOfRef:
      return CppCall::method(&RefData::release);
    DT_UNCOUNTED_CASE:
    case KindOfClass:
      break;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

namespace {
/*
 * Convenience class for creating TransLocs and TransRecs for new translations.
 *
 * Records the beginning and end of a translation and stores the size of the
 * cold and frozen regions in the first 4 bytes of their respective regions.
 */
struct TransLocMaker {
  explicit TransLocMaker(CodeCache& c) : cache(c) {}

  /*
   * Record the start of a translation, and reserve space at the top of cold
   * and frozen (if they aren't the same) to record sizes.
   */
  void markStart() {
    loc.setMainStart(cache.main().frontier());
    loc.setColdStart(cache.cold().frontier());
    loc.setFrozenStart(cache.frozen().frontier());

    cache.cold().dword(0);
    if (&cache.cold() != &cache.frozen()) cache.frozen().dword(0);
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
      bool                        llvm    = false,
      bool                        hasLoop = false) const {
    auto& cold = cache.realCold();
    auto& frozen = cache.realFrozen();
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
                    std::move(annot), llvm, hasLoop);
  }

private:
  CodeCache& cache;
  TransLoc loc;
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
  auto tcUsage = std::max(code.mainUsed(), code.profUsed());
  if (tcUsage >= RuntimeOption::EvalJitAMaxUsage) {
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
    invalidateSrcKey(m_tx.profData()->transSrcKey(tid));
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
  if (!writer || !shouldTranslate(args.sk.func())) return nullptr;
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

  m_tx.setMode(profileSrcKey(args.sk) ? TransKind::Profile : TransKind::Live);
  SCOPE_EXIT{ m_tx.setMode(TransKind::Invalid); };

  return translate(args);
}

TCA MCGenerator::retranslateOpt(TransID transId, bool align) {
  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;
  if (isDebuggerAttachedProcess()) return nullptr;

  TRACE(1, "retranslateOpt: transId = %u\n", transId);

  SCOPE_EXIT { m_tx.setMode(TransKind::Invalid); };

  if (!m_tx.profData()->hasTransRec(transId)) return nullptr;

  always_assert(m_tx.profData()->transRegion(transId) != nullptr);

  auto func   = m_tx.profData()->transFunc(transId);
  auto funcId = func->getFuncId();
  auto sk     = m_tx.profData()->transSrcKey(transId);

  if (m_tx.profData()->optimized(funcId)) return nullptr;
  m_tx.profData()->setOptimized(funcId);

  bool setFuncBody = func->getDVFunclets().size() == 0;

  func->setFuncBody(m_tx.uniqueStubs.funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  TCA start = regeneratePrologues(func, sk);

  // Regionize func and translate all its regions.
  m_tx.setMode(TransKind::Optimize);
  std::vector<RegionDescPtr> regions;
  regionizeFunc(func, this, regions);

  for (auto region : regions) {
    m_tx.setMode(TransKind::Optimize);
    always_assert(!region->empty());
    auto regionSk = region->start();
    auto translArgs = TranslArgs{regionSk, align};
    translArgs.region = region;

    if (setFuncBody && regionSk.offset() == func->base()) {
      translArgs.setFuncBody = true;
      setFuncBody = false;
    }
    auto regionStart = translate(translArgs);
    if (start == nullptr && regionSk == sk) {
      start = regionStart;
    }
  }

  m_tx.profData()->freeFuncData(funcId);

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

int
MCGenerator::numTranslations(SrcKey sk) const {
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

bool MCGenerator::shouldTranslate(const Func* func) const {
  if (!shouldTranslateNoSizeLimit(func)) return false;
  // Otherwise, follow the Eval.JitAMaxUsage limit.  However, we do
  // allow Optimize translations past that limit.
  return code.mainUsed() < RuntimeOption::EvalJitAMaxUsage ||
         m_tx.mode() == TransKind::Optimize;
}


static void populateLiveContext(RegionContext& ctx) {
  typedef RegionDesc::Location L;

  const ActRec*     const fp {vmfp()};
  const TypedValue* const sp {vmsp()};

  for (uint32_t i = 0; i < fp->m_func->numLocals(); ++i) {
    ctx.liveTypes.push_back(
      { L::Local{i}, typeFromTV(frame_local(fp, i)) }
    );
  }

  int32_t stackOff = 0;
  visitStackElems(
    fp, sp, ctx.bcOffset,
    [&](const ActRec* ar) {
      // TODO(#2466980): when it's a Cls, we should pass the Class* in
      // the Type.
      auto const objOrCls =
        ar->hasThis()  ? Type::SubObj(ar->getThis()->getVMClass()) :
        ar->hasClass() ? TCls
                       : TNullptr;

      ctx.preLiveARs.push_back({
        stackOff,
        ar->m_func,
        objOrCls
      });
      FTRACE(2, "added prelive ActRec {}\n", show(ctx.preLiveARs.back()));
      stackOff += kNumActRecCells;
    },
    [&](const TypedValue* tv) {
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
  if (!shouldTranslate(args.sk.func())) return nullptr;

  /*
   * Try to become the writer. We delay this until we *know* we will have
   * a need to create new translations, instead of just trying to win the
   * lottery at the dawn of time. Hopefully lots of requests won't require
   * any new translation.
   */
  auto sk = args.sk;
  LeaseHolder writer(Translator::WriteLease());
  if (!writer || !shouldTranslate(args.sk.func())) return nullptr;

  if (auto sr = m_tx.getSrcDB().find(sk)) {
    TCA tca = sr->getTopTranslation();
    if (tca) {
      // Handle extremely unlikely race; someone may have just already
      // added the first instance of this SrcRec while we did a
      // non-blocking wait on the write lease.
      return tca;
    } else {
      // Since we are holding the write lease, we know that sk is properly
      // initialized, except that it has no translations (due to
      // replaceOldTranslations)
      return retranslate(args);
    }
  }

  auto const srcRecSPOff = [&] () -> folly::Optional<FPInvOffset> {
    if (sk.resumed()) return folly::none;
    return liveSpOff();
  }();

  // We put retranslate requests at the end of our slab to more frequently
  //   allow conditional jump fall-throughs
  TCA astart          = code.main().frontier();
  TCA realColdStart   = code.realCold().frontier();
  TCA realFrozenStart = code.realFrozen().frontier();
  TCA req;
  if (!RuntimeOption::EvalEnableReusableTC) {
    req = emitServiceReq(code.cold(),
                         SRFlags::None,
                         srcRecSPOff,
                         REQ_RETRANSLATE,
                         sk.offset(),
                         TransFlags().packed);
  } else {
    auto const stubsize = mcg->backEnd().reusableStubSize();
    auto newStart = code.cold().allocInner(stubsize) ?: code.cold().frontier();
    // Ensure that the anchor translation is a known size so that it can be
    // reclaimed when the function is freed
    req = emitEphemeralServiceReq(code.cold(),
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

  size_t asize = code.main().frontier() - astart;
  size_t realColdSize   = code.realCold().frontier()   - realColdStart;
  size_t realFrozenSize = code.realFrozen().frontier() - realFrozenStart;
  assertx(asize == 0);
  if (realColdSize && RuntimeOption::EvalDumpTCAnchors) {
    TransRec tr(sk,
                TransKind::Anchor,
                astart, asize, realColdStart, realColdSize,
                realFrozenStart, realFrozenSize);
    m_tx.addTranslation(tr);
    if (RuntimeOption::EvalJitUseVtuneAPI) {
      reportTraceletToVtune(sk.unit(), sk.func(), tr);
    }

    if (m_tx.profData()) {
      m_tx.profData()->addTransNonProf(TransKind::Anchor, sk);
    }
    assertx(!m_tx.isTransDBEnabled() ||
           m_tx.getTransRec(realColdStart)->kind == TransKind::Anchor);
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

  assertx(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assertx(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);
  assertx(m_tx.mode() != TransKind::Invalid);
  SCOPE_EXIT{ m_tx.setMode(TransKind::Invalid); };

  if (!shouldTranslate(args.sk.func())) return nullptr;

  auto func = const_cast<Func*>(args.sk.func());
  CodeCache::Selector cbSel(CodeCache::Selector::Args(code)
                            .profile(m_tx.mode() == TransKind::Profile)
                            .hot(RuntimeOption::EvalHotFuncCount &&
                                 (func->attrs() & AttrHot) && m_tx.useAHot()));

  auto start = translateWork(args);

  if (args.setFuncBody) {
    func->setFuncBody(start);
  }
  SKTRACE(1, args.sk, "translate moved head from %p to %p\n",
          getTopTranslation(args.sk), start);

  return start;
}

TCA
MCGenerator::getCallArrayPrologue(Func* func) {
  TCA tca = func->getFuncBody();
  if (tca != m_tx.uniqueStubs.funcBodyHelperThunk) return tca;

  DVFuncletsVec dvs = func->getDVFunclets();

  if (dvs.size()) {
    LeaseHolder writer(Translator::WriteLease());
    if (!writer) return nullptr;
    tca = func->getFuncBody();
    if (tca != m_tx.uniqueStubs.funcBodyHelperThunk) return tca;
    tca = backEnd().emitCallArrayPrologue(func, dvs);
    func->setFuncBody(tca);
  } else {
    SrcKey sk(func, func->base(), false);
    auto args = TranslArgs{sk, false};
    args.setFuncBody = true;
    tca = mcg->getTranslation(args);
  }

  return tca;
}

void
MCGenerator::smashPrologueGuards(TCA* prologues, int numPrologues,
                                 const Func* func) {
  for (int i = 0; i < numPrologues; i++) {
    if (prologues[i] != m_tx.uniqueStubs.fcallHelperThunk
        && backEnd().funcPrologueHasGuard(prologues[i], func)) {
      backEnd().funcPrologueSmashGuard(prologues[i], func);
    }
  }
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
  if (prologue != m_tx.uniqueStubs.fcallHelperThunk) {
    TRACE(1, "cached prologue %s(%d) -> cached %p\n",
          func->fullName()->data(), paramIdx, prologue);
    assertx(isValidCodeAddress(prologue));
    return true;
  }
  return false;
}

TCA MCGenerator::emitFuncPrologue(Func* func, int argc) {
  const int nparams = func->numNonVariadicParams();
  const int paramIndex = argc <= nparams ? argc : nparams + 1;

  auto const funcBody = SrcKey { func, func->getEntryForNumArgs(argc), false };

  CodeCache::Selector cbSel(CodeCache::Selector::Args(code)
                            .profile(m_tx.mode() == TransKind::Proflogue)
                            .hot(RuntimeOption::EvalHotFuncCount &&
                                 (func->attrs() & AttrHot) && m_tx.useAHot()));
  assertx(m_fixups.empty());

  TCA mainOrig = code.main().frontier();
  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  if (((uintptr_t)code.main().frontier() & backEnd().cacheLineMask()) >=
      (backEnd().cacheLineSize() / 2)) {
    backEnd().moveToAlign(code.main(), MoveToAlignFlags::kCacheLineAlign);
  }
  m_fixups.m_alignFixups.emplace(
    code.main().frontier(), std::make_pair(backEnd().cacheLineSize() / 2, 0));

  TransLocMaker maker(code);
  maker.markStart();

  // Careful: this isn't necessarily the real entry point. For funcIsMagic
  // prologues, this is just a possible prologue.
  TCA aStart = code.main().frontier();

  // Give the prologue a TransID if we have profiling data.
  auto transID = m_tx.profData()
    ? m_tx.profData()->addTransPrologue(m_tx.mode(), funcBody, paramIndex)
    : kInvalidTransID;

  TCA start = genFuncPrologue(transID, func, argc);

  auto loc = maker.markEnd();
  if (RuntimeOption::EvalEnableReusableTC) {
    TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
               cs = loc.coldStart(), ce = loc.coldEnd(),
               fs = loc.frozenStart(), fe = loc.frozenEnd(),
               oldStart = start;
    bool did_relocate = relocateNewTranslation(loc, code, &start);

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
                       m_fixups);
  }
  m_fixups.process(nullptr);

  assertx(backEnd().funcPrologueHasGuard(start, func));
  assertx(isValidCodeAddress(start));

  TRACE(2, "funcPrologue mcg %p %s(%d) setting prologue %p\n",
        this, func->fullName()->data(), argc, start);
  func->setPrologue(paramIndex, start);

  assertx(m_tx.mode() == TransKind::Prologue ||
          m_tx.mode() == TransKind::Proflogue);

  auto tr = maker.rec(funcBody, m_tx.mode());
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

  Offset entry = func->getEntryForNumArgs(nPassed);
  SrcKey funcBody(func, entry, false);

  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;

  // If we're regenerating a prologue, and we want to check shouldTranslate()
  // but ignore the code size limits.  We still want to respect the global
  // translation limit and other restrictions, though.
  if (forRegeneratePrologue) {
    if (!shouldTranslateNoSizeLimit(func)) return nullptr;
  } else {
    if (!shouldTranslate(func)) return nullptr;
  }

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  // We're coming from a BIND_CALL service request, so enable
  // profiling if we haven't optimized the function entry yet.
  assertx(m_tx.mode() == TransKind::Invalid ||
         m_tx.mode() == TransKind::Prologue);
  if (m_tx.mode() == TransKind::Invalid && profileSrcKey(funcBody)) {
    m_tx.setMode(TransKind::Proflogue);
  } else {
    m_tx.setMode(TransKind::Prologue);
  }
  SCOPE_EXIT{ m_tx.setMode(TransKind::Invalid); };

  try {
    return emitFuncPrologue(func, nPassed);
  } catch (const DataBlockFull& dbFull) {

    // Fail hard if the block isn't code.hot.
    always_assert_flog(dbFull.name == "hot",
                       "data block = {}\nmessage: {}\n",
                       dbFull.name, dbFull.what());

    // Otherwise, fall back to code.main and retry.
    assertx(m_tx.useAHot());
    m_tx.setUseAHot(false);
    m_fixups.clear();
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
  Func* func = m_tx.profData()->transFunc(prologueTransId);
  int  nArgs = m_tx.profData()->prologueArgs(prologueTransId);

  // Regenerate the prologue.
  func->resetPrologue(nArgs);
  m_tx.setMode(TransKind::Prologue);
  SCOPE_EXIT { m_tx.setMode(TransKind::Invalid); };
  auto const start = getFuncPrologue(
    func,
    nArgs,
    nullptr /* ActRec */,
    true /* regeneratePrologue */
  );
  if (!start) return nullptr;

  func->setPrologue(nArgs, start);

  // Smash callers of the old prologue with the address of the new one.
  PrologueCallersRec* pcr =
    m_tx.profData()->prologueCallers(prologueTransId);
  for (TCA toSmash : pcr->mainCallers()) {
    backEnd().smashCall(toSmash, start);
  }
  // If the prologue has a guard, then smash its guard-callers as well.
  if (backEnd().funcPrologueHasGuard(start, func)) {
    TCA guard = backEnd().funcPrologueToGuard(start, func);
    for (TCA toSmash : pcr->guardCallers()) {
      backEnd().smashCall(toSmash, guard);
    }
  }
  pcr->clearAllCallers();

  // If this prologue has a DV funclet, then generate a translation for the DV
  // funclet right after the prologue.
  TCA triggerSkStart = nullptr;
  if (nArgs < func->numNonVariadicParams()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      m_tx.setMode(TransKind::Optimize);
      SrcKey funcletSK(func, paramInfo.funcletOff, false);
      auto funcletTransId = m_tx.profData()->dvFuncletTransId(func, nArgs);
      if (funcletTransId != kInvalidTransID) {
        invalidateSrcKey(funcletSK);
        auto args = TranslArgs{funcletSK, false};
        args.transId = funcletTransId;
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
    TransID tid = m_tx.profData()->prologueTransId(func, nArgs);
    if (tid != kInvalidTransID) {
      prologTransIDs.push_back(tid);
    }
  }

  std::sort(prologTransIDs.begin(), prologTransIDs.end(),
          [&](TransID t1, TransID t2) -> bool {
            // This will sort in ascending order. Note that transCounters start
            // at JitPGOThreshold and count down.
            return m_tx.profData()->transCounter(t1) >
                   m_tx.profData()->transCounter(t2);
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

  DecodedInstruction di(toSmash);
  if (di.isBranch() && !di.isJmp()) {
    auto jt = backEnd().jccTarget(toSmash);
    assertx(jt);
    if (jt == tDest) {
      // Already smashed
      return tDest;
    }
    sr->chainFrom(IncomingBranch::jccFrom(toSmash));
  } else {
    assertx(!backEnd().jccTarget(toSmash));
    if (!backEnd().jmpTarget(toSmash)
        || backEnd().jmpTarget(toSmash) == tDest) {
      // Already smashed
      return tDest;
    }
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
MCGenerator::bindJmpccFirst(TCA toSmash,
                            SrcKey skTaken, SrcKey skNotTaken,
                            bool taken,
                            bool& smashed) {
  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;
  auto const skWillExplore = taken ? skTaken : skNotTaken;
  auto const skWillDefer = taken ? skNotTaken : skTaken;
  auto const dest = skWillExplore;
  auto cc = backEnd().jccCondCode(toSmash);
  TRACE(3, "bindJmpccFirst: explored %d, will defer %d; overwriting cc%02x "
        "taken %d\n",
        skWillExplore.offset(), skWillDefer.offset(), cc, taken);
  always_assert(skTaken.resumed() == skNotTaken.resumed());
  auto const isResumed = skTaken.resumed();

  // We want the branch to point to whichever side has not been explored yet.
  if (taken) cc = ccNegate(cc);

  auto& cb = code.blockFor(toSmash);
  Asm as { cb };
  // Its not clear where the IncomingBranch should go to if cb is code.frozen()
  assertx(&cb != &code.frozen());

  // XXX Use of kJmp*Len here is a layering violation.
  using namespace x64;

  // can we just directly fall through?
  // a jmp + jz takes 5 + 6 = 11 bytes
  bool const fallThru = toSmash + kJmpccLen + kJmpLen == cb.frontier() &&
    !m_tx.getSrcDB().find(dest);

  auto const tDest = getTranslation(TranslArgs{dest, !fallThru});
  if (!tDest) {
    return 0;
  }

  auto const jmpTarget = backEnd().jmpTarget(toSmash + kJmpccLen);
  if (jmpTarget != backEnd().jccTarget(toSmash)) {
    // someone else already smashed this one. Ideally we would
    // just re-execute from toSmash - except the flags will have
    // been trashed.
    return tDest;
  }

  /*
   * If we're not in a resumed function, we need to fish out the stack offset
   * that the original service request used, so we can use it again on the one
   * we're about to create.
   */
  auto const optSPOff = [&] () -> folly::Optional<FPInvOffset> {
    if (isResumed) return folly::none;
    return serviceReqSPOff(jmpTarget);
  }();

  auto const stub = emitEphemeralServiceReq(
    code.frozen(),
    getFreeStub(code.frozen(), &mcg->cgFixups()),
    optSPOff,
    REQ_BIND_JMP,
    RipRelative(toSmash),
    skWillDefer.toAtomicInt(),
    TransFlags{}.packed
  );

  mcg->cgFixups().process(nullptr);
  smashed = true;
  assertx(Translator::WriteLease().amOwner());
  /*
   * Roll over the jcc and the jmp/fallthru. E.g., from:
   *
   *     toSmash:    jcc   <jmpccFirstStub>
   *     toSmash+6:  jmp   <jmpccFirstStub>
   *     toSmash+11: <probably the new translation == tdest>
   *
   * to:
   *
   *     toSmash:    j[n]z <jmpccSecondStub>
   *     toSmash+6:  nop5
   *     toSmash+11: newHotness
   */
  CodeCursor cg(cb, toSmash);
  as.jcc(cc, stub);
  m_tx.getSrcRec(dest)->chainFrom(IncomingBranch::jmpFrom(cb.frontier()));
  TRACE(5, "bindJmpccFirst: overwrote with cc%02x taken %d\n", cc, taken);
  return tDest;
}

namespace {

class FreeRequestStubTrigger {
  TCA m_stub;
 public:
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

  assertx(isValidCodeAddress(start));
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
  backEnd().enterTCHelper(start, stashedAR);
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

TCA MCGenerator::handleServiceRequest(ServiceReqInfo& info) noexcept {
  FTRACE(1, "handleServiceRequest {}\n", serviceReqName(info.req));

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

    case REQ_BIND_JMPCC_FIRST: {
      auto toSmash = info.args[0].tca;
      auto skTaken = SrcKey::fromAtomicInt(info.args[1].sk);
      auto skNotTaken = SrcKey::fromAtomicInt(info.args[2].sk);
      auto taken = info.args[3].boolVal;
      sk = taken ? skTaken : skNotTaken;
      start = bindJmpccFirst(toSmash, skTaken, skNotTaken, taken, smashed);
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
      Unit* destUnit = caller->func()->unit();
      // Set PC so logging code in getTranslation doesn't get confused.
      vmpc() = destUnit->at(caller->m_func->base() + ar->m_soff);
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
                                  unwindRdsInfo->debuggerReturnOff);
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
    start = m_tx.uniqueStubs.interpHelperSyncedPC;
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
  if (!isImmutable) {
    // We dont know we're calling the right function, so adjust start to point
    // to the dynamic check of ar->m_func.
    start = backEnd().funcPrologueToGuard(start, func);
  } else {
    TRACE(2, "bindCall immutably %s -> %p\n", func->fullName()->data(), start);
  }

  if (start) {
    LeaseHolder writer(Translator::WriteLease());
    if (writer) {
      // Someone else may have changed the func prologue while we waited for
      // the write lease, so read it again.
      start = getFuncPrologue(func, nArgs);
      if (!isImmutable) start = backEnd().funcPrologueToGuard(start, func);

      if (start && backEnd().callTarget(toSmash) != start) {
        assertx(backEnd().callTarget(toSmash));
        TRACE(2, "bindCall smash %p -> %p\n", toSmash, start);
        backEnd().smashCall(toSmash, start);

        bool is_profiled = false;
        // For functions to be PGO'ed, if their current prologues are still
        // profiling ones (living in code.prof()), then save toSmash as a
        // caller to the prologue, so that it can later be smashed to call a
        // new prologue when it's generated.
        int calleeNumParams = func->numNonVariadicParams();
        int calledPrologNumArgs = (nArgs <= calleeNumParams ?
                                   nArgs :  calleeNumParams + 1);
        if (code.prof().contains(start)) {
          if (isImmutable) {
            m_tx.profData()->addPrologueMainCaller(
              func, calledPrologNumArgs, toSmash);
          } else {
            m_tx.profData()->addPrologueGuardCaller(
              func, calledPrologNumArgs, toSmash);
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
    start = m_tx.uniqueStubs.fcallHelperThunk;
  }

  return start;
}

TCA MCGenerator::handleResume(bool interpFirst) {
  FTRACE(1, "handleResume({})\n", interpFirst);

  if (!vmRegsUnsafe().pc) return m_tx.uniqueStubs.callToExit;

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

void handleStackOverflow(ActRec* calleeAR) {
  /*
   * First synchronize registers.
   *
   * We're called in two situations: either this is the first frame after a
   * re-entry, in which case calleeAR->m_sfp is enterTCHelper's native stack,
   * or we're called in the middle of one VM entry (from a func prologue).  We
   * want to raise the exception from the caller's FCall instruction in the
   * second case, and in the first case we have to raise in a special way
   * inside this re-entry.
   *
   * Either way the stack depth is below the calleeAR by numArgs, because we
   * haven't run func prologue duties yet.
   */
  auto& unsafeRegs = vmRegsUnsafe();
  auto const isReentry = calleeAR == vmFirstAR();
  auto const arToSync = isReentry ? calleeAR : calleeAR->m_sfp;
  unsafeRegs.fp = arToSync;
  unsafeRegs.stack.top() =
    reinterpret_cast<Cell*>(calleeAR) - calleeAR->numArgs();
  auto const func_base = arToSync->func()->base();
  // calleeAR m_soff is 0 in the re-entry case, so we'll set pc to the func
  // base.  But it also doesn't matter because we're going to throw a special
  // VMReenterStackOverflow in that case so the unwinder won't worry about it.
  unsafeRegs.pc = arToSync->func()->unit()->at(func_base + calleeAR->m_soff);
  tl_regState = VMRegState::CLEAN;

  if (!isReentry) {
    /*
     * The normal case - we were called via FCall, or FCallArray.  We need to
     * construct the pc of the fcall from the return address (which will be
     * after the fcall). Because fcall is a variable length instruction, and
     * because we sometimes delete instructions from the instruction stream, we
     * need to use fpi regions to find the fcall.
     */
    const FPIEnt* fe = liveFunc()->findPrecedingFPI(
      liveUnit()->offsetOf(vmpc()));
    vmpc() = liveUnit()->at(fe->m_fcallOff);
    assertx(isFCallStar(*reinterpret_cast<const Op*>(vmpc())));
    raise_error("Stack overflow");
  } else {
    /*
     * We were called via re-entry.  Leak the params and the actrec, and tell
     * the unwinder that there's nothing left to do in this "entry".
     *
     * Also, the caller hasn't set up the m_invName area on the ActRec (unless
     * it was a magic call), since it's the prologue's responsibility if it's a
     * non-magic call.  We can just null it out since we're fatalling.
     */
    vmsp() = reinterpret_cast<Cell*>(calleeAR + 1);
    calleeAR->setVarEnv(nullptr);
    throw VMReenterStackOverflow();
  }
  not_reached();
}

void handlePossibleStackOverflow(ActRec* calleeAR) {
  assert_native_stack_aligned();
  auto const func = calleeAR->func();
  auto const limit = func->maxStackCells() + kStackCheckPadding;
  void* const needed_top = reinterpret_cast<TypedValue*>(calleeAR) - limit;
  void* const limit_addr =
    static_cast<char*>(vmRegsUnsafe().stack.getStackLowAddress()) +
    Stack::sSurprisePageSize;
  if (needed_top >= limit_addr) {
    // It was probably a surprise flag trip.  But we can't assert that it is
    // because background threads are allowed to clear surprise bits
    // concurrently, so it could be cleared again by now.
    return;
  }

  /*
   * Stack overflows in this situation are a slightly different case than
   * handleStackOverflow:
   *
   * A function prologue already did all the work to prepare to enter the
   * function, but then it found out it didn't have enough room on the stack.
   * It may even have written uninits deeper than the stack base (but we limit
   * it to sSurprisePageSize, so it's harmless).
   *
   * Most importantly, it might have pulled args /off/ the eval stack and
   * shoved them into an ExtraArgs on the calleeAR, or into an array for a
   * variadic capture param.  We need to get things into an appropriate state
   * for handleStackOverflow to be able to synchronize things to throw from the
   * PC of the caller's FCall.
   *
   * We don't actually need to make sure the stack is the right depth for the
   * FCall: the unwinder will expect to see a pre-live ActRec (and we'll set it
   * up so it will), but it doesn't care how many args (or what types of args)
   * are below it on the stack.
   *
   * It is tempting to try to free the ExtraArgs structure here, but it's ok to
   * not to:
   *
   *     o We're about to raise an uncatchable fatal, which will end the
   *       request.  We leak ExtraArgs in other similar situations for this too
   *       (e.g. if called via FCallArray and then a stack overflow happens).
   *
   *     o If we were going to free the ExtraArgs structure, we'd need to make
   *       sure we can re-enter the VM right now, which means performing a
   *       manual fixup first.  (We aren't in a situation where we can do a
   *       normal VMRegAnchor fixup right now.)  But moreover we shouldn't be
   *       running destructors if a fatal is happening anyway, so we don't want
   *       that either.
   *
   * So, all that boils down to this: we ignore the extra args field (the
   * unwinder will not consult the ExtraArgs field because it believes the
   * ActRec is pre-live).  And set calleeAR->m_numArgs to indicate how many
   * things are actually on the stack (so handleStackOverflow knows what to set
   * the vmsp to)---we just set it to the function's numLocals, which might
   * mean decreffing some uninits unnecessarily, but that's ok.
   */

  if (debug && func->attrs() & AttrMayUseVV && calleeAR->getExtraArgs()) {
    calleeAR->trashVarEnv();
  }
  calleeAR->setNumArgs(calleeAR->m_func->numLocals());
  handleStackOverflow(calleeAR);
}

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
  assertx(code.frozen().contains(stub));
  m_debugInfo.recordRelocMap(stub, 0, "FreeStub");
  m_freeStubs.push(stub);
  return true;
}

TCA MCGenerator::getFreeStub(CodeBlock& frozen, CodeGenFixups* fixups,
                             bool* isReused) {
  TCA ret = m_freeStubs.maybePop();
  if (isReused) *isReused = ret;

  if (ret) {
    Stats::inc(Stats::Astub_Reused);
    always_assert(m_freeStubs.peek() == nullptr ||
                  code.isValidCodeAddress(m_freeStubs.peek()));
    TRACE(1, "recycle stub %p\n", ret);
  } else {
    ret = frozen.frontier();
    Stats::inc(Stats::Astub_New);
    TRACE(1, "alloc new stub %p\n", ret);
  }

  if (fixups) {
    fixups->m_reusedStubs.emplace_back(ret);
  }
  return ret;
}

TCA MCGenerator::getTranslatedCaller() const {
  DECLARE_FRAME_POINTER(fp);
  ActRec* framePtr = fp;  // can't directly mutate the register-mapped one
  for (; framePtr; framePtr = framePtr->m_sfp) {
    TCA rip = (TCA)framePtr->m_savedRip;
    if (isValidCodeAddress(rip)) {
      return rip;
    }
  }
  return nullptr;
}

void
MCGenerator::syncWork() {
  assertx(tl_regState == VMRegState::DIRTY);
  m_fixupMap.fixup(g_context.getNoCheck());
  tl_regState = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}

// Get the address of the literal val in the global data section.
// If it's not there, add it to the map in m_fixups, which will
// be committed to m_literals when m_fixups.process() is called.
const uint64_t*
MCGenerator::allocLiteral(uint64_t val) {
  auto it = m_literals.find(val);
  if (it != m_literals.end()) {
    assertx(*it->second == val);
    return it->second;
  }
  auto& pending = m_fixups.m_literals;
  it = pending.find(val);
  if (it != pending.end()) {
    assertx(*it->second == val);
    return it->second;
  }
  auto addr = allocData<uint64_t>(sizeof(uint64_t), 1);
  *addr = val;
  return pending[val] = addr;
}

bool
MCGenerator::reachedTranslationLimit(SrcKey sk,
                                     const SrcRec& srcRec) const {
  if (srcRec.translations().size() == RuntimeOption::EvalJitMaxTranslations) {
    INC_TPC(max_trans);
    if (debug && Trace::moduleEnabled(Trace::mcg, 2)) {
      const auto& tns = srcRec.translations();
      TRACE(1, "Too many (%zd) translations: %s, BC offset %d\n",
            tns.size(), sk.unit()->filepath()->data(),
            sk.offset());
      SKTRACE(2, sk, "{\n");
      TCA topTrans = srcRec.getTopTranslation();
      for (size_t i = 0; i < tns.size(); ++i) {
        const TransRec* rec = m_tx.getTransRec(tns[i].mainStart());
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

  return false;
}

void
MCGenerator::recordSyncPoint(CodeAddress frontier, Fixup fix) {
  m_fixups.m_pendingFixups.push_back(PendingFixup(frontier, fix));
}

/*
 * Equivalent to container.clear(), but guarantees to free
 * any memory associated with the container (eg clear
 * doesn't affect std::vector's capacity).
 */
template <typename T> void ClearContainer(T& container) {
  T().swap(container);
}

void
CodeGenFixups::process_only(
  GrowableVector<IncomingBranch>* inProgressTailBranches) {
  for (uint32_t i = 0; i < m_pendingFixups.size(); i++) {
    TCA tca = m_pendingFixups[i].m_tca;
    assertx(mcg->isValidCodeAddress(tca));
    mcg->fixupMap().recordFixup(tca, m_pendingFixups[i].m_fixup);
  }
  ClearContainer(m_pendingFixups);

  auto& ctm = mcg->catchTraceMap();
  for (auto const& pair : m_pendingCatchTraces) {
    if (auto pos = ctm.find(pair.first)) {
      *pos = pair.second;
    } else {
      ctm.insert(pair.first, pair.second);
    }
  }
  ClearContainer(m_pendingCatchTraces);

  for (auto const& elm : m_pendingJmpTransIDs) {
    mcg->getJmpToTransIDMap()[elm.first] = elm.second;
  }
  ClearContainer(m_pendingJmpTransIDs);

  mcg->literals().insert(m_literals.begin(), m_literals.end());
  ClearContainer(m_literals);

  if (inProgressTailBranches) {
    m_inProgressTailJumps.swap(*inProgressTailBranches);
  }
  assertx(m_inProgressTailJumps.empty());

  for (auto& stub : m_reusedStubs) {
    mcg->getDebugInfo()->recordRelocMap(stub, 0, "NewStub");
  }
  ClearContainer(m_reusedStubs);
}

void CodeGenFixups::clear() {
  ClearContainer(m_pendingFixups);
  ClearContainer(m_pendingCatchTraces);
  ClearContainer(m_pendingJmpTransIDs);
  ClearContainer(m_reusedStubs);
  ClearContainer(m_addressImmediates);
  ClearContainer(m_codePointers);
  ClearContainer(m_bcMap);
  ClearContainer(m_alignFixups);
  ClearContainer(m_inProgressTailJumps);
  ClearContainer(m_literals);
}

bool CodeGenFixups::empty() const {
  return
    m_pendingFixups.empty() &&
    m_pendingCatchTraces.empty() &&
    m_pendingJmpTransIDs.empty() &&
    m_reusedStubs.empty() &&
    m_addressImmediates.empty() &&
    m_codePointers.empty() &&
    m_bcMap.empty() &&
    m_alignFixups.empty() &&
    m_inProgressTailJumps.empty() &&
    m_literals.empty();
}

TCA
MCGenerator::translateWork(const TranslArgs& args) {
  Timer _t(Timer::translate);
  auto sk = args.sk;

  SKTRACE(1, sk, "translateWork\n");
  assertx(m_tx.getSrcDB().find(sk));

  TCA mainOrig = code.main().frontier();
  if (args.align) {
    mcg->backEnd().moveToAlign(code.main(),
                               MoveToAlignFlags::kNonFallthroughAlign);
  }

  TransLocMaker maker(code);
  TCA        start             = code.main().frontier();
  TCA DEBUG_ONLY frozenStart   = code.frozen().frontier();
  SrcRec&    srcRec            = *m_tx.getSrcRec(sk);
  TransKind  transKindToRecord = TransKind::Interp;
  UndoMarker undo[] = {
    UndoMarker{code.main()},
    UndoMarker{code.cold()},
    UndoMarker{code.frozen()},
    UndoMarker{code.data()},
  };
  m_annotations.clear();

  setUseLLVM(
    RuntimeOption::EvalJitLLVM > 1 ||
    (RuntimeOption::EvalJitLLVM && m_tx.mode() == TransKind::Optimize)
  );
  SCOPE_EXIT { setUseLLVM(false); };

  auto resetState = [&] {
    for (auto& u : undo) u.undo();
    m_fixups.clear();
  };

  auto assertCleanState = [&] {
    assertx(code.main().frontier() == start);
    assertx(code.frozen().frontier() == frozenStart);
    assertx(m_fixups.empty());
  };

  FPInvOffset initSpOffset =
    args.region ? args.region->entry()->initialSpOffset()
                : liveSpOff();

  PostConditions pconds;
  RegionDescPtr region;
  bool hasLoop = false;

  if (!reachedTranslationLimit(sk, srcRec)) {
    // Attempt to create a region at this SrcKey
    if (m_tx.mode() == TransKind::Optimize) {
      assertx(RuntimeOption::EvalJitPGO);
      region = args.region;
      if (region) {
        assertx(!region->empty());
      } else {
        assertx(isValidTransID(args.transId));
        region = selectHotRegion(args.transId, this);
        assertx(region);
        if (region && region->empty()) region = nullptr;
      }
    } else {
      assertx(m_tx.mode() == TransKind::Profile ||
              m_tx.mode() == TransKind::Live);
      RegionContext rContext { sk.func(), sk.offset(), liveSpOff(),
                               sk.resumed() };
      FTRACE(2, "populating live context for region\n");
      populateLiveContext(rContext);
      region = selectRegion(rContext, m_tx.mode());
    }

    auto result = TranslateResult::Retry;
    auto regionInterps = RegionBlacklist{};
    initSpOffset = region ? region->entry()->initialSpOffset()
                          : liveSpOff();
    while (region && result == TranslateResult::Retry) {
      auto const profTransID = RuntimeOption::EvalJitPGO
        ? m_tx.profData()->curTransID()
        : kInvalidTransID;
      auto const transContext = TransContext(profTransID, sk, initSpOffset);

      IRGS irgs { transContext, args.flags };
      FTRACE(1, "{}{:-^40}{}\n",
             color(ANSI_COLOR_BLACK, ANSI_BGCOLOR_GREEN),
             " HHIR during translation ",
             color(ANSI_COLOR_END));

      try {
        assertCleanState();
        maker.markStart();

        result = translateRegion(irgs, *region, regionInterps, args.flags,
                                 pconds);
        hasLoop = RuntimeOption::EvalJitLoops && cfgHasLoop(irgs.unit);
        FTRACE(2, "translateRegion finished with result {}\n", show(result));
      } catch (const std::exception& e) {
        FTRACE(1, "translateRegion failed with '{}'\n", e.what());
        result = TranslateResult::Failure;
      }

      if (result != TranslateResult::Success) {
        // Translation failed or will be retried. Free resources for this
        // trace, rollback the translation cache frontiers, and discard any
        // pending fixups.
        resetState();
      }

      if (result == TranslateResult::Failure) {
        // If the region translator failed, clear `region' to fall
        // back to the interpreter.
        FTRACE(1, "translateRegion: failed on region:\n{}\n", show(*region));
        region.reset();
      }
    }

    if (!region) m_tx.setMode(TransKind::Interp);

    if (result == TranslateResult::Success) {
      assertx(m_tx.mode() == TransKind::Live    ||
             m_tx.mode() == TransKind::Profile ||
             m_tx.mode() == TransKind::Optimize);
      transKindToRecord = m_tx.mode();
    }
  }

  if (transKindToRecord == TransKind::Interp) {
    assertCleanState();
    maker.markStart();

    FTRACE(1, "emitting dispatchBB interp request for failed "
      "translation (spOff = {})\n", initSpOffset.offset);
    backEnd().emitInterpReq(code.main(), sk, initSpOffset);
    // Fall through.
  }

  auto loc = maker.markEnd();

  if (args.align) {
    m_fixups.m_alignFixups.emplace(
      loc.mainStart(), std::make_pair(backEnd().cacheLineSize() - 1, 0));
  }

  if (RuntimeOption::EvalEnableReusableTC) {
    TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
               cs = loc.coldStart(), ce = loc.coldEnd(),
               fs = loc.frozenStart(), fe = loc.frozenEnd(),
               oldStart = start;
    bool did_relocate = relocateNewTranslation(loc, code);

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

    if (loc.mainStart() != start) {
      code.main().setFrontier(mainOrig); // we may have shifted to align
    }
  }

  if (RuntimeOption::EvalProfileBC) {
    auto* unit = sk.unit();
    TransBCMapping prev{};
    for (auto& cur : m_fixups.m_bcMap) {
      if (!cur.aStart) continue;
      if (prev.aStart) {
        if (prev.bcStart < unit->bclen()) {
          recordBCInstr(unit->entry()[prev.bcStart],
                        prev.aStart, cur.aStart, false);
        }
      } else {
        recordBCInstr(OpTraceletGuard, loc.mainStart(), cur.aStart, false);
      }
      prev = cur;
    }
  }

  recordGdbTranslation(sk, sk.func(), code.main(), loc.mainStart(),
                       false, false);
  recordGdbTranslation(sk, sk.func(), code.cold(), loc.coldStart(),
                       false, false);
  if (RuntimeOption::EvalJitPGO) {
    if (transKindToRecord == TransKind::Profile) {
      always_assert(region);
      m_tx.profData()->addTransProfile(region, pconds);
    } else {
      m_tx.profData()->addTransNonProf(transKindToRecord, sk);
    }
  }

  auto tr = maker.rec(sk, transKindToRecord, region, m_fixups.m_bcMap,
                      std::move(m_annotations), useLLVM(), hasLoop);
  m_tx.addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(sk.unit(), sk.func(), tr);
  }

  if (RuntimeOption::EvalPerfRelocate) {
    recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                       loc.coldCodeStart(), loc.coldEnd(),
                       sk, -1,
                       srcRec.tailFallbackJumps(),
                       m_fixups);
  }
  GrowableVector<IncomingBranch> inProgressTailBranches;
  m_fixups.process(&inProgressTailBranches);
  m_annotations.clear();

  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: (func %d, bcOff %d)\n",
        loc.mainStart(), sk.funcID(), sk.offset());
  srcRec.newTranslation(loc, inProgressTailBranches);

  TRACE(1, "mcg: %zd-byte tracelet\n", (ssize_t)loc.mainSize());
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getUsageString().c_str());
  }

  return loc.mainStart();
}

void MCGenerator::traceCodeGen(IRGS& irgs) {
  auto& unit = irgs.unit;

  auto finishPass = [&](const char* msg, int level) {
    printUnit(level, unit, msg, nullptr, irgs.irb->guards());
    assertx(checkCfg(unit));
  };

  finishPass(" after initial translation ", kIRLevel);

  always_assert_flog(
    IMPLIES(cfgHasLoop(unit), RuntimeOption::EvalJitLoops),
    "IRUnit has loop but Eval.JitLoops=0"
  );

  optimize(unit, *irgs.irb, m_tx.mode());
  finishPass(" after optimizing ", kOptLevel);

  always_assert(this == mcg);
  genCode(unit);

  m_numTrans++;
  assertx(m_numTrans <= RuntimeOption::EvalJitGlobalTranslationLimit);
}

MCGenerator::MCGenerator()
  : m_backEnd(newBackEnd())
  , m_numTrans(0)
  , m_catchTraceMap(128)
  , m_useLLVM(false)
{
  TRACE(1, "MCGenerator@%p startup\n", this);
  mcg = this;

  m_unwindRegistrar = register_unwind_region(code.base(), code.codeSize());

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
  if (Trace::moduleEnabledRelease(Trace::llvm_count, 1) ||
      RuntimeOption::EvalJitLLVMCounters) {
    g_bytecodesVasm.bind();
    g_bytecodesLLVM.bind();
  }
}

void MCGenerator::initUniqueStubs() {
  // Put the following stubs into ahot, rather than a.
  CodeCache::Selector cbSel(CodeCache::Selector::Args(code).
                            hot(m_tx.useAHot()));
  m_tx.uniqueStubs = backEnd().emitUniqueStubs();
  m_fixups.process(nullptr); // in case we generated literals
}

void MCGenerator::registerCatchBlock(CTCA ip, TCA block) {
  FTRACE(1, "registerCatchBlock: afterCall: {} block: {}\n", ip, block);
  m_fixups.m_pendingCatchTraces.emplace_back(ip, block);
}

folly::Optional<TCA> MCGenerator::getCatchTrace(CTCA ip) const {
  TCA* found = m_catchTraceMap.find(ip);
  if (found && *found != kInvalidCatchTrace) return *found;
  return folly::none;
}

void MCGenerator::codeEmittedThisRequest(size_t& requestEntry,
                                         size_t& now) const {
  requestEntry = s_initialTCSize;
  now = code.totalUsed();
}

namespace {
__thread std::unordered_map<const ActRec*, TCA>* tl_debuggerCatches{nullptr};
}

void pushDebuggerCatch(const ActRec* fp) {
  if (!tl_debuggerCatches) {
    tl_debuggerCatches = new std::unordered_map<const ActRec*, TCA>();
  }

  auto optCatchBlock = mcg->getCatchTrace(TCA(fp->m_savedRip));
  always_assert(optCatchBlock && *optCatchBlock);
  auto catchBlock = *optCatchBlock;
  FTRACE(1, "Pushing debugger catch {} with fp {}\n", catchBlock, fp);
  tl_debuggerCatches->emplace(fp, catchBlock);
}

TCA popDebuggerCatch(const ActRec* fp) {
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
  memset(&s_perfCounters, 0, sizeof(s_perfCounters));
  Stats::init();
  s_initialTCSize = code.totalUsed();
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
                          kPerfCounterNames[i], s_perfCounters[i]);
    }
    Trace::traceRelease("\n");
  }

  if (Trace::moduleEnabledRelease(Trace::llvm_count, 1)) {
    auto llvm = *g_bytecodesLLVM;
    auto total = llvm + *g_bytecodesVasm;
    Trace::ftraceRelease(
      "{:9} / {:9} bytecodes ({:6.2f}%) handled by LLVM backend for {}\n",
      llvm, total, llvm * 100.0 / total, g_context->getRequestUrl(50)
    );
  }

  delete tl_debuggerCatches;
  tl_debuggerCatches = nullptr;
}

void
MCGenerator::getPerfCounters(Array& ret) {
  for (int i = 0; i < tpc_num_counters; i++) {
    // Until Perflab can automatically scale the values we give it to
    // an appropriate range, we have to fudge these numbers so they
    // look more like reasonable hardware counter values.
    ret.set(String::FromCStr(kPerfCounterNames[i]),
            s_perfCounters[i] * 1000);
  }

  for (auto const& pair : Timer::Counters()) {
    if (pair.second.total == 0 && pair.second.count == 0) continue;

    ret.set(String("jit_time_") + pair.first, pair.second.total);
  }

  if (RuntimeOption::EvalJitLLVMCounters) {
    ret.set(String("jit_instr_vasm"), *g_bytecodesVasm);
    ret.set(String("jit_instr_llvm"), *g_bytecodesLLVM);
  }
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
      m_debugInfo.recordTracelet(rangeFrom(cb, start, &cb == &code.cold()),
                                 srcFunc,
                                 reinterpret_cast<const Op*>(
                                   srcFunc->unit() ?
                                     srcFunc->unit()->at(sk.offset()) : nullptr
                                 ),
                                 exit, inPrologue);
    }
    if (RuntimeOption::EvalPerfPidMap) {
      m_debugInfo.recordPerfMap(rangeFrom(cb, start, &cb == &code.cold()),
                                srcFunc, exit, inPrologue);
    }
  }
}

void MCGenerator::recordGdbStub(const CodeBlock& cb,
                                const TCA start,
                                const std::string& name) {
  if (RuntimeOption::EvalJitNoGdb) return;
  m_debugInfo.recordStub(rangeFrom(cb, start, &cb == &code.cold()), name);
}

std::vector<UsageInfo> MCGenerator::getUsageInfo() {
  std::vector<UsageInfo> tcUsageInfo;
  code.forEachBlock([&](const char* name, const CodeBlock& a) {
    tcUsageInfo.emplace_back(UsageInfo{std::string("code.") + name,
                             a.used(),
                             a.capacity(),
                             true});
  });
  tcUsageInfo.emplace_back(UsageInfo{
      "data",
      code.data().used(),
      code.data().capacity(),
      true});
  tcUsageInfo.emplace_back(UsageInfo{
      "RDS",
      rds::usedBytes(),
      RuntimeOption::EvalJitTargetCacheSize * 3 / 4,
      false});
  tcUsageInfo.emplace_back(UsageInfo{
      "RDSLocal",
      rds::usedLocalBytes(),
      RuntimeOption::EvalJitTargetCacheSize * 3 / 4,
      false});
  tcUsageInfo.emplace_back(UsageInfo{
      "persistentRDS",
      rds::usedPersistentBytes(),
      RuntimeOption::EvalJitTargetCacheSize / 4,
      false});
  return tcUsageInfo;
}

std::string MCGenerator::getUsageString() {
  std::string usage;
  size_t totalBlockSize = 0;
  size_t totalBlockCapacity = 0;
  auto addRow = [&](UsageInfo blockUsageInfo) {
    auto percent = blockUsageInfo.m_capacity ?
      100 * blockUsageInfo.m_used / blockUsageInfo.m_capacity : 0;
    usage += folly::format("mcg: {:9} bytes ({}%) in {}\n",
                           blockUsageInfo.m_used,
                           percent,
                           blockUsageInfo.m_name).str();
    if (blockUsageInfo.m_global) {
      totalBlockSize += blockUsageInfo.m_used;
      totalBlockCapacity += blockUsageInfo.m_capacity;
    }
  };
  auto tcUsageInfo = getUsageInfo();
  for_each(tcUsageInfo.begin(), tcUsageInfo.end(), addRow);
  addRow(UsageInfo{"total", totalBlockSize, totalBlockCapacity, false});
  return usage;
}

std::string MCGenerator::getTCAddrs() {
  std::string addrs;
  code.forEachBlock([&](const char* name, const CodeBlock& a) {
      addrs += folly::format("{}: {}\n", name, a.base()).str();
  });
  return addrs;
}

bool MCGenerator::addDbgGuards(const Unit* unit) {
  // TODO refactor
  // It grabs the write lease and iterates through whole SrcDB...
  struct timespec tsBegin, tsEnd;
  {
    BlockingLeaseHolder writer(Translator::WriteLease());
    if (!writer) {
      return false;
    }
    assertx(mcg->cgFixups().empty());
    HPHP::Timer::GetMonotonicTime(tsBegin);
    // Doc says even find _could_ invalidate iterator, in pactice it should
    // be very rare, so go with it now.
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
        addDbgGuardImpl(sk, sr);
      }
    }
    mcg->cgFixups().process(nullptr);
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
  assertx(mcg->cgFixups().empty());
  {
    if (SrcRec* sr = m_tx.getSrcDB().find(sk)) {
      addDbgGuardImpl(sk, sr);
    }
  }
  mcg->cgFixups().process(nullptr);
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
  size_t count = code.hot().used();
  bool result = (fwrite(code.hot().base(), 1, count, ahotFile) == count);
  if (result) {
    count = code.main().used();
    result = (fwrite(code.main().base(), 1, count, aFile) == count);
  }
  if (result) {
    count = code.prof().used();
    result = (fwrite(code.prof().base(), 1, count, aprofFile) == count);
  }
  if (result) {
    count = code.cold().used();
    result = (fwrite(code.cold().base(), 1, count, acoldFile) == count);
  }
  if (result) {
    count = code.frozen().used();
    result = (fwrite(code.frozen().base(), 1, count, afrozenFile) == count);
  }
  return result;
}

// Returns true on success
bool MCGenerator::dumpTC(bool ignoreLease) {
  folly::Optional<BlockingLeaseHolder> writer;
  if (!ignoreLease) {
    writer.emplace(Translator::WriteLease());
    if (!*writer) return false;
  }
  bool success = dumpTCData();
  if (success) {
    success = dumpTCCode("/tmp/tc_dump");
  }
  return success;
}

// Returns true on success
bool tc_dump(void) {
  return mcg && mcg->dumpTC();
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
                kRepoSchemaId,
                code.hot().base(), code.hot().frontier(),
                code.main().base(), code.main().frontier(),
                code.prof().base(), code.prof().frontier(),
                code.cold().base(), code.cold().frontier(),
                code.frozen().base(), code.frozen().frontier())) {
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

void MCGenerator::setJmpTransID(TCA jmp) {
  if (m_tx.mode() != TransKind::Profile) return;

  TransID transId = m_tx.profData()->curTransID();
  FTRACE(5, "setJmpTransID: adding {} => {}\n", jmp, transId);
  m_fixups.m_pendingJmpTransIDs.emplace_back(jmp, transId);
}

void
emitIncStat(CodeBlock& cb, uint64_t* tl_table, uint32_t index, int n, bool force) {
  if (!force && !Stats::enabled()) return;
  intptr_t disp = uintptr_t(&tl_table[index]) - tlsBase();

  mcg->backEnd().emitIncStat(cb, disp, n);
}

void emitIncStat(Vout& v, Stats::StatCounter stat, int n, bool force) {
  if (!force && !Stats::enabled()) return;
  intptr_t disp = uintptr_t(&Stats::tl_counters[stat]) - tlsBase();
  v << addqim{n, Vptr{baseless(disp), Vptr::FS}, v.makeReg()};
}

// generic vasm service-request generator. target specific details
// are hidden by the svcreq{} instruction.
void emitServiceReq(Vout& v, TCA stub_block,
                    ServiceRequest req, const ServiceReqArgVec& argv) {
  TRACE(3, "Emit Service Req %s(", serviceReqName(req));
  VregList args;
  for (auto& argInfo : argv) {
    switch (argInfo.m_kind) {
      case ServiceReqArgInfo::Immediate: {
        TRACE(3, "%" PRIx64 ", ", argInfo.m_imm);
        args.push_back(v.cns(argInfo.m_imm));
        break;
      }
      default: {
        always_assert(false);
        break;
      }
    }
  }
  auto const regs = x64::kCrossTraceRegs | x64::rVmSp;
  v << svcreq{req, regs, v.makeTuple(args), stub_block};
}

}}
