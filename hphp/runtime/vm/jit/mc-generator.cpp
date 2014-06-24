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

#include "folly/MapUtil.h"

#include <cinttypes>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <strstream>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <queue>
#include <unwind.h>
#include <unordered_set>

#include <algorithm>
#include <exception>
#include <memory>
#include <vector>

#include "folly/Format.h"
#include "folly/String.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/disasm.h"
#include "hphp/util/bitops.h"
#include "hphp/util/debug.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/rank.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"
#include "hphp/util/meta.h"
#include "hphp/util/process.h"
#include "hphp/util/repo-schema.h"
#include "hphp/util/cycles.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/php-debug.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-option-guard.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/back-end-x64.h" // XXX Layering violation.
#include "hphp/runtime/vm/jit/debug-guards.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/mc-generator-internal.h"

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(mcg);

using namespace reg;
using namespace Trace;
using std::max;

#define TRANS_PERF_COUNTERS \
  TPC(translate) \
  TPC(retranslate) \
  TPC(interp_bb) \
  TPC(interp_instr) \
  TPC(interp_one) \
  TPC(max_trans) \
  TPC(enter_tc) \
  TPC(service_req)

#define TPC(n) "jit_" #n,
static const char* const kPerfCounterNames[] = {
  TRANS_PERF_COUNTERS
};
#undef TPC

#define TPC(n) tpc_ ## n,
enum TransPerfCounter {
  TRANS_PERF_COUNTERS
  tpc_num_counters
};
#undef TPC
static __thread int64_t s_perfCounters[tpc_num_counters];
#define INC_TPC(n) ++s_perfCounters[tpc_ ## n];

static __thread size_t s_initialTCSize;

// The global MCGenerator object.
MCGenerator* mcg;

CppCall MCGenerator::getDtorCall(DataType type) {
  switch (type) {
  case BitwiseKindOfString:
    return CppCall::method(&StringData::release);
  case KindOfArray:
    return CppCall::method(&ArrayData::release);
  case KindOfObject:
    return CppCall::method(&ObjectData::release);
  case KindOfResource:
    return CppCall::method(&ResourceData::release);
  case KindOfRef:
    return CppCall::method(&RefData::release);
  default:
    assert(false);
    NOT_REACHED();
  }
}

bool MCGenerator::profileSrcKey(const SrcKey& sk) const {
  if (!sk.func()->shouldPGO()) return false;
  if (m_tx.profData()->optimized(sk.getFuncId())) return false;
  if (m_tx.profData()->profiling(sk.getFuncId())) return true;
  return requestCount() <= RuntimeOption::EvalJitProfileRequests;
}

/*
 * Invalidate the SrcDB entries for func's SrcKeys that have any
 * Profile translation.
 */
void MCGenerator::invalidateFuncProfSrcKeys(const Func* func) {
  assert(RuntimeOption::EvalJitPGO);
  FuncId funcId = func->getFuncId();
  for (auto tid : m_tx.profData()->funcProfTransIDs(funcId)) {
    invalidateSrcKey(m_tx.profData()->transSrcKey(tid));
  }
}

TCA MCGenerator::retranslate(const TranslArgs& args) {
  SrcRec* sr = m_tx.getSrcDB().find(args.m_sk);
  always_assert(sr);
  bool locked = sr->tryLock();
  SCOPE_EXIT {
    if (locked) sr->freeLock();
  };
  if (isDebuggerAttachedProcess() && m_tx.isSrcKeyInBL(args.m_sk)) {
    // We are about to translate something known to be blacklisted by
    // debugger, exit early
    SKTRACE(1, args.m_sk, "retranslate abort due to debugger\n");
    return nullptr;
  }
  LeaseHolder writer(Translator::WriteLease());
  if (!writer || !shouldTranslate()) return nullptr;
  if (!locked) {
    // Even though we knew above that we were going to skip
    // doing another translation, we wait until we get the
    // write lease, to avoid spinning through the tracelet
    // guards again and again while another thread is writing
    // to it.
    return sr->getTopTranslation();
  }
  SKTRACE(1, args.m_sk, "retranslate\n");

  m_tx.setMode(profileSrcKey(args.m_sk) ? TransKind::Profile
                                        : TransKind::Live);
  SCOPE_EXIT{ m_tx.setMode(TransKind::Invalid); };

  return translate(args);
}

TCA MCGenerator::retranslateOpt(TransID transId, bool align) {
  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;
  if (isDebuggerAttachedProcess()) return nullptr;

  TRACE(1, "retranslateOpt: transId = %u\n", transId);

  SCOPE_EXIT { m_tx.setMode(TransKind::Invalid); };

  always_assert(m_tx.profData()->transRegion(transId) != nullptr);

  Func*       func = m_tx.profData()->transFunc(transId);
  FuncId    funcId = func->getFuncId();
  const SrcKey& sk = m_tx.profData()->transSrcKey(transId);

  if (m_tx.profData()->optimized(funcId)) return nullptr;
  m_tx.profData()->setOptimized(funcId);

  bool setFuncBody = func->getDVFunclets().size() == 0;

  func->setFuncBody(m_tx.uniqueStubs.funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  TCA start = regeneratePrologues(func, sk);

  // Regionize func and translate all its regions.
  std::vector<RegionDescPtr> regions;
  regionizeFunc(func, this, regions);

  for (auto region : regions) {
    m_tx.setMode(TransKind::Optimize);
    always_assert(region->blocks.size() > 0);
    SrcKey regionSk = region->blocks[0]->start();
    auto translArgs = TranslArgs(regionSk, align).region(region);
    if (setFuncBody && regionSk.offset() == func->base()) {
      translArgs.setFuncBody();
      setFuncBody = false;
    }
    TCA regionStart = translate(translArgs);
    if (start == nullptr && regionSk == sk) {
      assert(regionStart);
      start = regionStart;
    }
    // Cloned closures' prologue tables point to the corresponding
    // main/DV entry point.  So update the prologue table when
    // retranslating their entries.
    if (func->isClonedClosure() && func->isEntry(regionSk.offset())) {
      int entryNumParams = func->getEntryNumParams(regionSk.offset());
      func->setPrologue(entryNumParams, regionStart);
    }
  }
  assert(start);
  return start;
}

/*
 * Find or create a translation for sk. Returns TCA of "best" current
 * translation. May return NULL if it is currently impossible to create
 * a translation.
 */
TCA
MCGenerator::getTranslation(const TranslArgs& args) {
  auto sk = args.m_sk;
  sk.func()->validate();
  SKTRACE(2, sk,
          "getTranslation: curUnit %s funcId %x offset %d\n",
          sk.unit()->filepath()->data(),
          sk.getFuncId(),
          sk.offset());
  SKTRACE(2, sk, "   funcId: %x \n", sk.func()->getFuncId());

  if (Translator::liveFrameIsPseudoMain() &&
      !RuntimeOption::EvalJitPseudomain) {
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

static void populateLiveContext(RegionContext& ctx) {
  typedef RegionDesc::Location L;

  const ActRec*     const fp {vmfp()};
  const TypedValue* const sp {vmsp()};

  for (uint32_t i = 0; i < fp->m_func->numLocals(); ++i) {
    ctx.liveTypes.push_back(
      { L::Local{i}, liveTVType(frame_local(fp, i)) }
    );
  }

  uint32_t stackOff = 0;
  visitStackElems(
    fp, sp, ctx.bcOffset,
    [&](const ActRec* ar) {
      // TODO(#2466980): when it's a Cls, we should pass the Class* in
      // the Type.
      auto const objOrCls =
        ar->hasThis()  ? Type::Obj.specialize(ar->getThis()->getVMClass()) :
        ar->hasClass() ? Type::Cls
                       : Type::Nullptr;

      ctx.preLiveARs.push_back(
        { stackOff,
          ar->m_func,
          objOrCls
        }
      );
      FTRACE(2, "added prelive ActRec {}\n", show(ctx.preLiveARs.back()));

      stackOff += kNumActRecCells;
    },
    [&](const TypedValue* tv) {
      ctx.liveTypes.push_back(
        { L::Stack{stackOff, ctx.spOffset - stackOff}, liveTVType(tv) }
      );
      stackOff++;
      FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
    }
  );
}

TCA
MCGenerator::createTranslation(const TranslArgs& args) {
  if (!shouldTranslate()) return nullptr;

  /*
   * Try to become the writer. We delay this until we *know* we will have
   * a need to create new translations, instead of just trying to win the
   * lottery at the dawn of time. Hopefully lots of requests won't require
   * any new translation.
   */
  auto sk = args.m_sk;
  LeaseHolder writer(Translator::WriteLease());
  if (!writer || !shouldTranslate()) return nullptr;

  if (SrcRec* sr = m_tx.getSrcDB().find(sk)) {
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

  // We put retranslate requests at the end of our slab to more frequently
  //   allow conditional jump fall-throughs
  TCA astart          = code.main().frontier();
  TCA realColdStart   = code.realCold().frontier();
  TCA realFrozenStart = code.realFrozen().frontier();
  TCA req = emitServiceReq(code.cold(), REQ_RETRANSLATE,
                           sk.offset(), TransFlags().packed);
  SKTRACE(1, sk, "inserting anchor translation for (%p,%d) at %p\n",
          sk.unit(), sk.offset(), req);
  SrcRec* sr = m_tx.getSrcRec(sk);
  sr->setFuncInfo(sk.func());
  sr->setAnchorTranslation(req);

  size_t asize = code.main().frontier() - astart;
  size_t realColdSize   = code.realCold().frontier()   - realColdStart;
  size_t realFrozenSize = code.realFrozen().frontier() - realFrozenStart;
  assert(asize == 0);
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
    assert(!m_tx.isTransDBEnabled() ||
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

  assert(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assert(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);
  assert(m_tx.mode() != TransKind::Invalid);
  SCOPE_EXIT{ m_tx.setMode(TransKind::Invalid); };

  if (!args.m_interp) {
    if (m_numHHIRTrans == RuntimeOption::EvalJitGlobalTranslationLimit) {
      RuntimeOption::EvalJit = false;
      ThreadInfo::s_threadInfo->m_reqInjectionData.updateJit();
      return nullptr;
    }
  }

  Func* func = const_cast<Func*>(args.m_sk.func());
  CodeCache::Selector cbSel(CodeCache::Selector::Args(code)
                            .profile(m_tx.mode() == TransKind::Profile)
                            .hot((func->attrs() & AttrHot) && m_tx.useAHot()));

  if (args.m_align) {
    mcg->backEnd().moveToAlign(code.main(),
                               MoveToAlignFlags::kNonFallthroughAlign);
  }

  TCA start = code.main().frontier();
  translateWork(args);

  if (args.m_setFuncBody) {
    func->setFuncBody(start);
  }
  SKTRACE(1, args.m_sk, "translate moved head from %p to %p\n",
          getTopTranslation(args.m_sk), start);

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
    tca = mcg->backEnd().emitCallArrayPrologue(func, dvs);
    func->setFuncBody(tca);
  } else {
    SrcKey sk(func, func->base(), false);
    tca = mcg->getTranslation(TranslArgs(sk, false).setFuncBody());
  }

  return tca;
}

void
MCGenerator::smashPrologueGuards(TCA* prologues, int numPrologues,
                                 const Func* func) {
  DEBUG_ONLY std::unique_ptr<LeaseHolder> writer;
  for (int i = 0; i < numPrologues; i++) {
    if (prologues[i] != m_tx.uniqueStubs.fcallHelperThunk
        && backEnd().funcPrologueHasGuard(prologues[i], func)) {
      if (debug) {
        /*
         * Unit's are sometimes created racily, in which case all
         * but the first are destroyed immediately. In that case,
         * the Funcs of the destroyed Units never need their
         * prologues smashing, and it would be a lock rank violation
         * to take the write lease here.
         * In all other cases, Funcs are destroyed via a delayed path
         * (treadmill) and the rank violation isn't an issue.
         */
        if (!writer) {
          writer.reset(new LeaseHolder(Translator::WriteLease(),
                       LeaseAcquire::BLOCKING));
        }
      }
      mcg->backEnd().funcPrologueSmashGuard(prologues[i], func);
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
 * The fcallc labelled "b2" above is not statically bindable in our
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
    assert(isValidCodeAddress(prologue));
    return true;
  }
  return false;
}

static void interp_set_regs(ActRec* ar, Cell* sp, Offset pcOff) {
  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  vmfp() = ar;
  vmsp() = sp;
  vmpc() = ar->unit()->at(pcOff);
}

TCA
MCGenerator::getFuncPrologue(Func* func, int nPassed, ActRec* ar,
                             bool ignoreTCLimit) {
  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);
  int const numParams = func->numNonVariadicParams();
  int paramIndex = nPassed <= numParams ? nPassed : numParams + 1;

  bool const funcIsMagic = func->isMagic();

  // Do a quick test before grabbing the write lease
  TCA prologue;
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  Offset entry = func->getEntryForNumArgs(nPassed);
  SrcKey funcBody(func, entry, false);

  if (func->isClonedClosure()) {
    assert(ar);
    interp_set_regs(ar, (Cell*)ar - func->numSlotsInFrame(), entry);
    TCA tca = getTranslation(TranslArgs(funcBody, false));
    tl_regState = VMRegState::DIRTY;
    if (tca) {
      // racy, but ok...
      func->setPrologue(paramIndex, tca);
    }
    return tca;
  }

  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;
  if (!ignoreTCLimit && !shouldTranslate()) return nullptr;

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  // We're comming from a BIND_CALL service request, so enable
  // profiling if we haven't optimized the function entry yet.
  assert(m_tx.mode() == TransKind::Invalid ||
         m_tx.mode() == TransKind::Prologue);
  if (m_tx.mode() == TransKind::Invalid && profileSrcKey(funcBody)) {
    m_tx.setMode(TransKind::Proflogue);
  } else {
    m_tx.setMode(TransKind::Prologue);
  }
  SCOPE_EXIT{ m_tx.setMode(TransKind::Invalid); };

  CodeCache::Selector cbSel(CodeCache::Selector::Args(code)
                            .profile(m_tx.mode() == TransKind::Proflogue)
                            .hot((func->attrs() & AttrHot) && m_tx.useAHot()));

  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  if (((uintptr_t)code.main().frontier() & backEnd().cacheLineMask()) >=
      (backEnd().cacheLineSize() / 2)) {
    backEnd().moveToAlign(code.main(), MoveToAlignFlags::kCacheLineAlign);
  }

  // Careful: this isn't necessarily the real entry point. For funcIsMagic
  // prologues, this is just a possible prologue.
  TCA aStart    = code.main().frontier();
  TCA start     = aStart;
  TCA realColdStart   = mcg->code.realCold().frontier();
  TCA realFrozenStart = mcg->code.realFrozen().frontier();

  auto const skFuncBody = [&] {
    assert(m_fixups.empty());
    auto ret = backEnd().emitFuncPrologue(code.main(), code.cold(), func,
                                          funcIsMagic, nPassed,
                                          start, aStart);
    m_fixups.process();
    return ret;
  }();

  assert(backEnd().funcPrologueHasGuard(start, func));
  TRACE(2, "funcPrologue mcg %p %s(%d) setting prologue %p\n",
        this, func->fullName()->data(), nPassed, start);
  assert(isValidCodeAddress(start));
  func->setPrologue(paramIndex, start);

  assert(m_tx.mode() == TransKind::Prologue ||
         m_tx.mode() == TransKind::Proflogue);
  TransRec tr(skFuncBody,
              m_tx.mode(),
              aStart,          code.main().frontier()       - aStart,
              realColdStart,   code.realCold().frontier()   - realColdStart,
              realFrozenStart, code.realFrozen().frontier() - realFrozenStart);
  m_tx.addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(func->unit(), func, tr);
  }

  if (m_tx.profData()) {
    m_tx.profData()->addTransPrologue(m_tx.mode(), skFuncBody, paramIndex);
  }

  recordGdbTranslation(skFuncBody, func,
                       code.main(), aStart,
                       false, true);
  recordBCInstr(OpFuncPrologue, code.main(), start, false);

  return start;
}

/**
 * Given the proflogueTransId for a TransProflogue translation,
 * regenerate the prologue (as a TransPrologue).  Returns the starting
 * address for the translation corresponding to triggerSk, if such
 * translation is generated; otherwise returns nullptr.
 */
TCA MCGenerator::regeneratePrologue(TransID prologueTransId,
                                    SrcKey triggerSk) {
  Func* func = m_tx.profData()->transFunc(prologueTransId);
  int  nArgs = m_tx.profData()->prologueArgs(prologueTransId);

  // Regenerate the prologue.
  func->resetPrologue(nArgs);
  m_tx.setMode(TransKind::Prologue);
  SCOPE_EXIT { m_tx.setMode(TransKind::Invalid); };
  TCA start = getFuncPrologue(func, nArgs, nullptr /* ActRec */,
                              true /* ignoreTCLimit */);
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

  // If this prologue has a DV funclet, then generate a translation
  // for the DV funclet right after the prologue.  However, skip
  // cloned closures because their prologues are actually the DV
  // funclets already.
  TCA triggerSkStart = nullptr;
  if (nArgs < func->numNonVariadicParams() && !func->isClonedClosure()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      m_tx.setMode(TransKind::Optimize);
      SrcKey  funcletSK(func, paramInfo.funcletOff, false);
      TransID funcletTransId = m_tx.profData()->dvFuncletTransId(func, nArgs);
      if (funcletTransId != kInvalidTransID) {
        invalidateSrcKey(funcletSK);
        TCA dvStart = translate(TranslArgs(funcletSK, false).
                                transId(funcletTransId));
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
  TCA tDest = getTranslation(TranslArgs(destSk, false).flags(trflags));
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
  } else if (req == REQ_BIND_JCC || req == REQ_BIND_SIDE_EXIT) {
    auto jt = backEnd().jccTarget(toSmash);
    assert(jt);
    if (jt == tDest) {
      // Already smashed
      return tDest;
    }
    sr->chainFrom(IncomingBranch::jccFrom(toSmash));
  } else {
    assert(!backEnd().jccTarget(toSmash));
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
                            Offset offTaken, Offset offNotTaken,
                            bool taken,
                            ConditionCode cc,
                            bool& smashed) {
  const Func* f = liveFunc();
  LeaseHolder writer(Translator::WriteLease());
  if (!writer) return nullptr;
  Offset offWillExplore = taken ? offTaken : offNotTaken;
  Offset offWillDefer = taken ? offNotTaken : offTaken;
  SrcKey dest(f, offWillExplore, liveResumed());
  TRACE(3, "bindJmpccFirst: explored %d, will defer %d; overwriting cc%02x "
        "taken %d\n",
        offWillExplore, offWillDefer, cc, taken);

  // We want the branch to point to whichever side has not been explored
  // yet.
  if (taken) {
    cc = ccNegate(cc);
  }

  auto& cb = code.blockFor(toSmash);
  Asm as { cb };
  // Its not clear where the IncomingBranch should go to if cb is code.frozen()
  assert(&cb != &code.frozen());

  // XXX Use of kJmp*Len here is a layering violation.
  using namespace X64;

  // can we just directly fall through?
  // a jmp + jz takes 5 + 6 = 11 bytes
  bool fallThru = toSmash + kJmpccLen + kJmpLen == cb.frontier() &&
    !m_tx.getSrcDB().find(dest);

  TCA tDest;
  tDest = getTranslation(TranslArgs(dest, !fallThru));
  if (!tDest) {
    return 0;
  }

  if (backEnd().jmpTarget(toSmash + kJmpccLen)
      != backEnd().jccTarget(toSmash)) {
    // someone else already smashed this one. Ideally we would
    // just re-execute from toSmash - except the flags will have
    // been trashed.
    return tDest;
  }

  TCA stub = emitEphemeralServiceReq(code.frozen(),
                                     getFreeStub(code.frozen(), nullptr),
                                     REQ_BIND_JMPCC_SECOND,
                                     RipRelative(toSmash),
                                     offWillDefer, cc);

  smashed = true;
  assert(Translator::WriteLease().amOwner());
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

// smashes a jcc to point to a new destination
TCA
MCGenerator::bindJmpccSecond(TCA toSmash, const Offset off,
                             ConditionCode cc, bool& smashed) {
  const Func* f = liveFunc();
  SrcKey dest(f, off, liveResumed());
  TCA branch = getTranslation(TranslArgs(dest, true));
  if (branch) {
    LeaseHolder writer(Translator::WriteLease());
    if (writer) {
      if (branch == backEnd().jccTarget(toSmash)) {
        // already smashed
        return branch;
      } else {
        smashed = true;
        SrcRec* destRec = m_tx.getSrcRec(dest);
        destRec->chainFrom(IncomingBranch::jccFrom(toSmash));
      }
    }
  }
  return branch;
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
MCGenerator::enterTC(TCA start, void* data) {
  if (debug) {
    fflush(stdout);
    fflush(stderr);
  }
  DepthGuard d;
  TReqInfo info;
  SrcKey sk;

  if (LIKELY(start != nullptr)) {
    info.requestNum = data ? REQ_BIND_CALL : -1;
    info.saved_rStashedAr = (uintptr_t)data;
  } else {
    info.requestNum = -1;
    info.saved_rStashedAr = 0;
    sk = *(SrcKey*)data;
    start = getTranslation(TranslArgs(sk, true));
  }
  for (;;) {
    assert(sizeof(Cell) == 16);
    assert(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
    assert(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);

    Translator::WriteLease().gremlinUnlock();
    // Keep dispatching until we end up somewhere the translator
    // recognizes, or we luck out and the leaseholder exits.
    while (!start) {
      TRACE(2, "enterTC forwarding BB to interpreter\n");
      vmpc() = sk.unit()->at(sk.offset());
      INC_TPC(interp_bb);
      g_context->dispatchBB();
      PC newPc = vmpc();
      if (!newPc) { vmfp() = 0; return; }
      sk = SrcKey(liveFunc(), newPc, liveResumed());
      start = getTranslation(TranslArgs(sk, true));
    }
    assert(start == m_tx.uniqueStubs.funcBodyHelperThunk ||
           isValidCodeAddress(start) ||
           (start == m_tx.uniqueStubs.fcallHelperThunk &&
            info.saved_rStashedAr == (uintptr_t)data));
    assert(!Translator::WriteLease().amOwner());
    const Func* func = (vmfp() ? (ActRec*)vmfp() : (ActRec*)data)->m_func;
    func->validate();
    INC_TPC(enter_tc);

    TRACE(1, "enterTC: %p fp%p(%s) sp%p enter {\n", start,
          vmfp(), func->name()->data(), vmsp());
    tl_regState = VMRegState::DIRTY;

    if (Trace::moduleEnabledRelease(Trace::ringbuffer, 1)) {
      auto skData = sk.valid() ? sk.toAtomicInt() : uint64_t(-1LL);
      Trace::ringbufferEntry(RBTypeEnterTC, skData, (uint64_t)start);
    }

    mcg->backEnd().enterTCHelper(start, info);
    assert(isValidVMStackAddress(vmRegsUnsafe().stack.top()));

    tl_regState = VMRegState::CLEAN; // Careful: pc isn't sync'ed yet.
    TRACE(1, "enterTC: %p fp%p sp%p } return\n", start,
          vmfp(), vmsp());

    if (debug) {
      // Debugging code: cede the write lease half the time.
      if (RuntimeOption::EvalJitStressLease) {
        if (d.depthOne() && (rand() % 2) == 0) {
          Translator::WriteLease().gremlinLock();
        }
      }
      // Ensure that each case either returns, or drives start to a valid
      // value.
      start = TCA(0xbee5face);
    }

    TRACE(2, "enterTC: request(%s) args: %" PRIxPTR " %" PRIxPTR " %"
             PRIxPTR " %" PRIxPTR " %" PRIxPTR "\n",
          serviceReqName(info.requestNum),
          info.args[0], info.args[1], info.args[2], info.args[3],
          info.args[4]);

    if (LIKELY(info.requestNum == REQ_EXIT)) {
      vmfp() = nullptr;
      return;
    }
    if (!handleServiceRequest(info, start, sk)) return;
  }
}

/*
 * The contract is that each case will set sk to the place where
 * execution should resume, and optionally set start to the hardware
 * translation of the resumption point (or otherwise set it to null).
 * Returns false if we need to halt this nesting of the VM.
 *
 * start and sk might be subtly different; i.e., there are cases where
 * start != NULL && start != getTranslation(sk). For instance,
 * REQ_BIND_CALL has not finished executing the OpCall when it gets
 * here, and has even done some work on its behalf. sk == OpFCall,
 * while start == the point in the TC that's "half-way through" the
 * Call instruction. If we punt to the interpreter, the interpreter
 * will redo some of the work that the translator has already done.
 */
bool MCGenerator::handleServiceRequest(TReqInfo& info,
                                       TCA& start,
                                       SrcKey& sk) {
  const ServiceRequest requestNum =
    static_cast<ServiceRequest>(info.requestNum);
  auto* const args = info.args;
  assert(requestNum != REQ_EXIT);
  INC_TPC(service_req);

  bool smashed = false;
  switch (requestNum) {
  case REQ_BIND_CALL: {
    ReqBindCall* req = reinterpret_cast<ReqBindCall*>(args[0]);
    ActRec* calleeFrame = reinterpret_cast<ActRec*>(args[1]);
    TCA toSmash = req->m_toSmash;
    Func *func = const_cast<Func*>(calleeFrame->m_func);
    int nArgs = req->m_nArgs;
    bool isImmutable = req->m_isImmutable;
    TRACE(2, "enterTC: bindCall %s, ActRec %p\n",
          func->fullName()->data(), calleeFrame);
    TCA dest = getFuncPrologue(func, nArgs);
    TRACE(2, "enterTC: bindCall -> %p\n", dest);
    if (!isImmutable) {
      // We dont know we're calling the right function, so adjust
      // dest to point to the dynamic check of ar->m_func.
      dest = backEnd().funcPrologueToGuard(dest, func);
    } else {
      TRACE(2, "enterTC: bindCall immutably %s -> %p\n",
            func->fullName()->data(), dest);
    }
    if (dest) {
      LeaseHolder writer(Translator::WriteLease());
      if (writer) {
        // Someone else may have changed the func prologue while we
        // waited for the write lease, so read it again.
        dest = getFuncPrologue(func, nArgs);
        assert(dest);
        if (!isImmutable) dest = backEnd().funcPrologueToGuard(dest, func);

        if (backEnd().callTarget(toSmash) != dest) {
          assert(backEnd().callTarget(toSmash));
          TRACE(2, "enterTC: bindCall smash %p -> %p\n", toSmash, dest);
          backEnd().smashCall(toSmash, dest);
          smashed = true;
          // For functions to be PGO'ed, if their current prologues
          // are still profiling ones (living in code.prof()), then
          // save toSmash as a caller to the prologue, so that it can
          // later be smashed to call a new prologue when it's generated.
          int calleeNumParams = func->numNonVariadicParams();
          int calledPrologNumArgs = (nArgs <= calleeNumParams ?
                                     nArgs :  calleeNumParams + 1);
          if (code.prof().contains(dest)) {
            if (isImmutable) {
              m_tx.profData()->addPrologueMainCaller(func, calledPrologNumArgs,
                                                     toSmash);
            } else {
              m_tx.profData()->addPrologueGuardCaller(func, calledPrologNumArgs,
                                                      toSmash);
            }
          }
        }
      }
      // sk: stale, but doesn't matter since we have a valid dest TCA.
    } else {
      // We need translator help; we're not at the callee yet, so
      // roll back. The prelude has done some work already, but it
      // should be safe to redo.
      TRACE(2, "enterTC: bindCall rollback smash %p -> %p\n",
            toSmash, dest);
      sk = req->m_sourceInstr;

      // EnterTCHelper pushes the return ip onto the stack when the
      // requestNum is REQ_BIND_CALL, but if start is NULL, it will
      // interpret in doFCall, so we clear out the requestNum in this
      // case to prevent enterTCHelper from pushing the return ip
      // onto the stack.
      info.requestNum = ~REQ_BIND_CALL;
    }
    start = dest;
  } break;

  case REQ_BIND_SIDE_EXIT:
  case REQ_BIND_JMP:
  case REQ_BIND_JCC:
  case REQ_BIND_ADDR:
  {
    TCA toSmash = (TCA)args[0];
    auto ai = static_cast<SrcKey::AtomicInt>(args[1]);
    sk = SrcKey::fromAtomicInt(ai);
    TransFlags trflags{args[2]};

    if (requestNum == REQ_BIND_SIDE_EXIT) {
      SKTRACE(3, sk, "side exit taken!\n");
    }
    start = bindJmp(toSmash, sk, requestNum, trflags, smashed);
  } break;

  case REQ_BIND_JMPCC_FIRST: {
    TCA toSmash = (TCA)args[0];
    Offset offTaken = (Offset)args[1];
    Offset offNotTaken = (Offset)args[2];
    ConditionCode cc = ConditionCode(args[3]);
    bool taken = int64_t(args[4]) & 1;
    start = bindJmpccFirst(toSmash, offTaken, offNotTaken,
                           taken, cc, smashed);
    // SrcKey: we basically need to emulate the fail
    sk = SrcKey(liveFunc(), taken ? offTaken : offNotTaken, liveResumed());
  } break;

  case REQ_BIND_JMPCC_SECOND: {
    TCA toSmash = (TCA)args[0];
    Offset off = (Offset)args[1];
    ConditionCode cc = ConditionCode(args[2]);
    start = bindJmpccSecond(toSmash, off, cc, smashed);
    sk = SrcKey(liveFunc(), off, liveResumed());
  } break;

  case REQ_RETRANSLATE_OPT: {
    auto ai = static_cast<SrcKey::AtomicInt>(args[0]);
    TransID transId = (TransID)args[1];
    sk = SrcKey::fromAtomicInt(ai);
    start = retranslateOpt(transId, false);
    SKTRACE(2, sk, "retranslated-OPT: transId = %d  start: @%p\n", transId,
            start);
    break;
  }

  case REQ_RETRANSLATE: {
    INC_TPC(retranslate);
    sk = SrcKey(liveFunc(), (Offset)args[0], liveResumed());
    auto trflags = TransFlags(args[1]);
    start = retranslate(TranslArgs(sk, true).flags(trflags));
    SKTRACE(2, sk, "retranslated @%p\n", start);
  } break;

  case REQ_INTERPRET: {
    Offset off = args[0];
    vmpc() = liveUnit()->at(off);
    /*
     * We know the compilation unit has not changed; basic blocks do
     * not span files. I claim even exceptions do not violate this
     * axiom.
     */
    SKTRACE(5, SrcKey(liveFunc(), off, liveResumed()), "interp: enter\n");
    // dispatch until BB ends
    INC_TPC(interp_bb);
    g_context->dispatchBB();
    PC newPc = vmpc();
    if (!newPc) { vmfp() = 0; return false; }
    SrcKey newSk(liveFunc(), newPc, liveResumed());
    SKTRACE(5, newSk, "interp: exit\n");
    sk = newSk;
    start = getTranslation(TranslArgs(newSk, true));
  } break;

  case REQ_POST_INTERP_RET: {
    // This is only responsible for the control-flow aspect of the Ret:
    // getting to the destination's translation, if any.
    ActRec* ar = (ActRec*)args[0];
    ActRec* caller = (ActRec*)args[1];
    assert(caller == vmfp());
    Unit* destUnit = caller->m_func->unit();
    // Set PC so logging code in getTranslation doesn't get confused.
    vmpc() = destUnit->at(caller->m_func->base() + ar->m_soff);
    SrcKey dest(caller->func(), vmpc(), caller->resumed());
    sk = dest;
    start = getTranslation(TranslArgs(dest, true));
    TRACE(3, "REQ_POST_INTERP_RET: from %s to %s\n",
          ar->m_func->fullName()->data(),
          caller->m_func->fullName()->data());
  } break;

  case REQ_RESUME: {
    if (UNLIKELY(vmpc() == 0)) {
      vmfp() = 0;
      return false;
    }
    SrcKey dest(liveFunc(), vmpc(), liveResumed());
    sk = dest;
    start = getTranslation(TranslArgs(dest, true));
  } break;

  case REQ_STACK_OVERFLOW:
    if (((ActRec*)info.saved_rStashedAr)->m_sfp == (ActRec*)vmfp()) {
      /*
       * The normal case - we were called via FCall, or FCallArray.
       * We need to construct the pc of the fcall from the return
       * address (which will be after the fcall). Because fcall is
       * a variable length instruction, and because we sometimes
       * delete instructions from the instruction stream, we
       * need to use fpi regions to find the fcall.
       */
      const FPIEnt* fe = liveFunc()->findPrecedingFPI(
        liveUnit()->offsetOf(vmpc()));
      vmpc() = liveUnit()->at(fe->m_fcallOff);
      assert(isFCallStar(*reinterpret_cast<const Op*>(vmpc())));
      raise_error("Stack overflow");
      NOT_REACHED();
    } else {
      /*
       * We were called via re-entry
       * Leak the params and the actrec, and tell the unwinder
       * that there's nothing left to do in this "entry".
       */
      vmsp() = (Cell*)((ActRec*)info.saved_rStashedAr + 1);
      throw VMReenterStackOverflow();
    }

  case REQ_EXIT:
    not_reached();
  }

  assert(start != TCA(0xbee5face));
  if (smashed && info.stubAddr) {
    Treadmill::enqueue(FreeRequestStubTrigger(info.stubAddr));
  }

  return true;
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
  StubNode* n = (StubNode *)stub;
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
  assert(code.frozen().contains(stub));
  m_freeStubs.push(stub);
  return true;
}

TCA MCGenerator::getFreeStub(CodeBlock& frozen, CodeGenFixups* fixups) {
  TCA ret = m_freeStubs.maybePop();
  if (ret) {
    Stats::inc(Stats::Astub_Reused);
    always_assert(m_freeStubs.m_list == nullptr ||
                  code.isValidCodeAddress(TCA(m_freeStubs.m_list)));
    TRACE(1, "recycle stub %p\n", ret);
    if (fixups) {
      fixups->m_reusedStubs.emplace_back(ret);
    }
  } else {
    ret = frozen.frontier();
    Stats::inc(Stats::Astub_New);
    TRACE(1, "alloc new stub %p\n", ret);
  }
  return ret;
}

#define O(opcode, imm, pusph, pop, flags) \
/**
 * The interpOne methods saves m_pc, m_fp, and m_sp ExecutionContext,
 * calls into the interpreter, and then return a pointer to the
 * current ExecutionContext.
 */  \
ExecutionContext*                                                       \
interpOne##opcode(ActRec* ar, Cell* sp, Offset pcOff) {                 \
  interp_set_regs(ar, sp, pcOff);                                       \
  SKTRACE(5, SrcKey(liveFunc(), vmpc(), liveResumed()), "%40s %p %p\n", \
          "interpOne" #opcode " before (fp,sp)",                        \
          vmfp(), vmsp());                                              \
  assert(*reinterpret_cast<const Op*>(vmpc()) == Op::opcode);           \
  auto const ec = g_context.getNoCheck();                               \
  Stats::inc(Stats::Instr_InterpOne ## opcode);                         \
  if (Trace::moduleEnabled(Trace::interpOne, 1)) {                      \
    static const StringData* cat = makeStaticString("interpOne");       \
    static const StringData* name = makeStaticString(#opcode);          \
    Stats::incStatGrouped(cat, name, 1);                                \
  }                                                                     \
  INC_TPC(interp_one)                                                   \
  /* Correct for over-counting in TC-stats. */                          \
  Stats::inc(Stats::Instr_TC, -1);                                      \
  ec->op##opcode();                                                     \
  /*
   * Only set regstate back to dirty if an exception is not
   * propagating.  If an exception is throwing, regstate for this call
   * is actually still correct, and we don't have information in the
   * fixup map for interpOne calls anyway.
   */ \
  tl_regState = VMRegState::DIRTY;                                      \
  return ec;                                                            \
}

OPCODES
#undef O

void* interpOneEntryPoints[] = {
#define O(opcode, imm, pusph, pop, flags) \
  (void*)(interpOne ## opcode),
OPCODES
#undef O
};

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
  assert(tl_regState == VMRegState::DIRTY);
  m_fixupMap.fixup(g_context.getNoCheck());
  tl_regState = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}

TCA
MCGenerator::emitNativeTrampoline(TCA helperAddr) {
  auto& trampolines = code.trampolines();
  if (!trampolines.canEmit(kExpectedPerTrampolineSize)) {
    // not enough space to emit a trampoline, so just return the
    // helper address and emitCall will the emit the right sequence
    // to call it indirectly
    TRACE(1, "Ran out of space to emit a trampoline for %p\n", helperAddr);
    return helperAddr;
  }

  uint32_t index = m_numNativeTrampolines++;
  TCA trampAddr = trampolines.frontier();
  if (Stats::enabled()) {
    emitIncStat(trampolines, &Stats::tl_helper_counters[0], index);
    auto name = getNativeFunctionName(helperAddr);
    const size_t limit = 50;
    if (name.size() > limit) {
      name[limit] = '\0';
    }

    // The duped string lives until process death intentionally.
    Stats::helperNames[index].store(strdup(name.c_str()),
                                    std::memory_order_release);
  }

  Asm a { trampolines };
  // Move the 64-bit immediate address to rax, then jmp. If clobbering
  // rax is a problem, we could do an rip-relative call with the address
  // stored in the data section with no extra registers; but it has
  // worse memory locality.
  a.    emitImmReg(helperAddr, rax);
  a.    jmp    (rax);
  a.    ud2(); // hint that the jump doesn't go here.

  m_trampolineMap[helperAddr] = trampAddr;
  recordBCInstr(OpNativeTrampoline, trampolines, trampAddr, false);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTrampolineToVtune(trampAddr, trampolines.frontier() - trampAddr);
  }

  return trampAddr;
}

TCA
MCGenerator::getNativeTrampoline(TCA helperAddr) {
  if (!RuntimeOption::EvalJitTrampolines && !Stats::enabled()) {
    return helperAddr;
  }
  auto const trampAddr = (TCA)folly::get_default(m_trampolineMap, helperAddr);
  if (trampAddr) {
    return trampAddr;
  }
  return emitNativeTrampoline(helperAddr);
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
        const TransRec* rec = m_tx.getTransRec(tns[i]);
        assert(rec);
        SKTRACE(2, sk, "%zd %p\n", i, tns[i]);
        if (tns[i] == topTrans) {
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
MCGenerator::recordSyncPoint(CodeAddress frontier, Offset pcOff, Offset spOff) {
  m_fixups.m_pendingFixups.push_back(
    PendingFixup(frontier, Fixup(pcOff, spOff)));
}

void
CodeGenFixups::process() {
  for (uint i = 0; i < m_pendingFixups.size(); i++) {
    TCA tca = m_pendingFixups[i].m_tca;
    assert(mcg->isValidCodeAddress(tca));
    mcg->fixupMap().recordFixup(tca, m_pendingFixups[i].m_fixup);
  }
  m_pendingFixups.clear();

  for (auto const& pair : m_pendingCatchTraces) {
    mcg->catchTraceMap().insert(pair.first, pair.second);
  }
  m_pendingCatchTraces.clear();

  for (auto const& elm : m_pendingJmpTransIDs) {
    mcg->getJmpToTransIDMap().insert(elm);
  }
  m_pendingJmpTransIDs.clear();
  /*
   * Currently these are only used by the relocator,
   * so there's nothing left to do here.
   *
   * Once we try to relocate live code, we'll need to
   * store compact forms of these for later.
   */
  m_reusedStubs.clear();
  m_addressImmediates.clear();
  m_codePointers.clear();
  m_bcMap.clear();
  m_alignFixups.clear();
}

void CodeGenFixups::clear() {
  m_pendingFixups.clear();
  m_pendingCatchTraces.clear();
  m_pendingJmpTransIDs.clear();
  m_reusedStubs.clear();
  m_addressImmediates.clear();
  m_codePointers.clear();
  m_bcMap.clear();
  m_alignFixups.clear();
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
    m_alignFixups.empty();
}

void
MCGenerator::translateWork(const TranslArgs& args) {
  Timer _t(Timer::translate);
  auto sk = args.m_sk;

  SKTRACE(1, sk, "translateWork\n");
  assert(m_tx.getSrcDB().find(sk));

  TCA        start             = code.main().frontier();
  TCA        coldStart         = code.cold().frontier();
  TCA        realColdStart     = code.realCold().frontier();
  TCA DEBUG_ONLY frozenStart   = code.frozen().frontier();
  TCA        realFrozenStart   = code.realFrozen().frontier();
  SrcRec&    srcRec            = *m_tx.getSrcRec(sk);
  TransKind  transKindToRecord = TransKind::Interp;
  UndoMarker undoA(code.main());
  UndoMarker undoAcold(code.cold());
  UndoMarker undoAfrozen(code.frozen());
  UndoMarker undoGlobalData(code.data());

  auto resetState = [&] {
    undoA.undo();
    undoAcold.undo();
    undoAfrozen.undo();
    undoGlobalData.undo();
    m_fixups.clear();
    srcRec.clearInProgressTailJumps();
  };

  auto assertCleanState = [&] {
    assert(code.main().frontier() == start);
    assert(code.frozen().frontier() == frozenStart);
    assert(m_fixups.empty());
    assert(srcRec.inProgressTailJumps().empty());
  };

  PostConditions pconds;
  RegionDescPtr region;
  if (!args.m_interp && !reachedTranslationLimit(sk, srcRec)) {
    // Attempt to create a region at this SrcKey
    if (m_tx.mode() == TransKind::Optimize) {
      assert(RuntimeOption::EvalJitPGO);
      region = args.m_region;
      if (region) {
        assert(region->blocks.size() > 0);
      } else {
        TransID transId = args.m_transId;
        assert(transId != kInvalidTransID);
        region = selectHotRegion(transId, this);
        assert(region);
        if (region && region->blocks.size() == 0) region = nullptr;
      }
    } else {
      assert(m_tx.mode() == TransKind::Profile ||
             m_tx.mode() == TransKind::Live);
      RegionContext rContext { sk.func(), sk.offset(), liveSpOff(),
                               sk.resumed() };
      FTRACE(2, "populating live context for region\n");
      populateLiveContext(rContext);
      region = selectRegion(rContext, m_tx.mode());
    }

    Translator::TranslateResult result = Translator::Retry;
    Translator::RegionBlacklist regionInterps;
    Offset const initSpOffset = region ? region->blocks[0]->initialSpOffset()
                                       : liveSpOff();
    bool bcControlFlow = RuntimeOption::EvalHHIRBytecodeControlFlow;

    auto const transContext = TransContext {
      RuntimeOption::EvalJitPGO
        ? m_tx.profData()->curTransID()
        : kInvalidTransID,
      sk.offset(),
      initSpOffset,
      sk.resumed(),
      sk.func()
    };

    while (result == Translator::Retry) {
      m_tx.traceStart(transContext);

      if (!region) {
        m_tx.setMode(TransKind::Interp);
        m_tx.traceFree();
        break;
      }

      try {
        assertCleanState();
        result = m_tx.translateRegion(*region, bcControlFlow,
                                      regionInterps, args.m_flags);

        // If we're profiling, grab the postconditions so we can
        // use them in region selection whenever we decide to retranslate.
        if (m_tx.mode() == TransKind::Profile &&
            result == Translator::Success &&
            RuntimeOption::EvalJitPGOUsePostConditions) {
          pconds = m_tx.irTrans()->hhbcTrans().unit().postConditions();
        }

        FTRACE(2, "translateRegion finished with result {}\n",
               Translator::translateResultName(result));
      } catch (ControlFlowFailedExc& cfe) {
        FTRACE(2, "translateRegion with control flow failed: '{}'\n",
               cfe.what());
        always_assert(bcControlFlow &&
                      "control flow translation failed, but control flow not "
                      "enabled");
        bcControlFlow = false;
        result = Translator::Retry;
      } catch (const std::exception& e) {
        FTRACE(1, "translateRegion failed with '{}'\n", e.what());
        result = Translator::Failure;
      }

      if (result != Translator::Success) {
        // Translation failed or will be retried. Free resources for this
        // trace, rollback the translation cache frontiers, and discard any
        // pending fixups.
        resetState();
      }

      if (result == Translator::Failure) {
        // If the region translator failed for an Optimize translation, it's OK
        // to do a Live translation for the function entry. Otherwise, fall
        // back to Interp.
        if (m_tx.mode() == TransKind::Optimize) {
          if (sk.getFuncId() == liveFunc()->getFuncId() &&
              liveUnit()->contains(vmpc()) &&
              sk.offset() == liveUnit()->offsetOf(vmpc()) &&
              sk.resumed() == liveResumed()) {
            m_tx.setMode(TransKind::Live);
            RegionContext rContext { sk.func(), sk.offset(), liveSpOff(),
                sk.resumed() };
            FTRACE(2, "populating live context for region after failed optimize"
                   "translation\n");
            populateLiveContext(rContext);
            region = selectRegion(rContext, m_tx.mode());
          } else {
            region.reset();
          }
        }
      }

      m_tx.traceFree();
    }

    if (result == Translator::Success) {
      assert(m_tx.mode() == TransKind::Live    ||
             m_tx.mode() == TransKind::Profile ||
             m_tx.mode() == TransKind::Optimize);
      transKindToRecord = m_tx.mode();
    }
  }

  if (args.m_dryRun) {
    resetState();
    return;
  }

  if (transKindToRecord == TransKind::Interp) {
    assertCleanState();
    FTRACE(1, "emitting dispatchBB interp request for failed translation\n");
    mcg->backEnd().emitInterpReq(code.main(), code.cold(), sk);
    // Fall through.
  }

  recordGdbTranslation(sk, sk.func(), code.main(), start,
                       false, false);
  recordGdbTranslation(sk, sk.func(), code.cold(), coldStart,
                       false, false);
  if (RuntimeOption::EvalJitPGO) {
    if (transKindToRecord == TransKind::Profile) {
      always_assert(region);
      m_tx.profData()->addTransProfile(region, pconds);
    } else {
      m_tx.profData()->addTransNonProf(transKindToRecord, sk);
    }
  }

  TransRec tr(sk, transKindToRecord,
              start,           code.main().frontier()       - start,
              realColdStart,   code.realCold().frontier()   - realColdStart,
              realFrozenStart, code.realFrozen().frontier() - realFrozenStart,
              region, m_fixups.m_bcMap);
  m_tx.addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(sk.unit(), sk.func(), tr);
  }

  m_fixups.process();

  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: (func %d, bcOff %d)\n",
        start, sk.getFuncId(), sk.offset());
  srcRec.newTranslation(start);
  TRACE(1, "mcg: %zd-byte tracelet\n", code.main().frontier() - start);
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getUsage().c_str());
  }
}

void MCGenerator::traceCodeGen() {
  HhbcTranslator& ht = m_tx.irTrans()->hhbcTrans();
  auto& unit = ht.unit();

  auto finishPass = [&](const char* msg, int level) {
    printUnit(level, unit, msg, nullptr, nullptr, ht.irBuilder().guards());
    assert(checkCfg(unit));
  };

  finishPass(" after initial translation ", kIRLevel);

  // Task #4075847: enable optimizations with loops
  if (!(RuntimeOption::EvalJitLoops && m_tx.mode() == TransKind::Optimize)) {
    optimize(unit, ht.irBuilder(), m_tx.mode());
    finishPass(" after optimizing ", kOptLevel);
  }
  if (m_tx.mode() == TransKind::Profile &&
      RuntimeOption::EvalJitPGOUsePostConditions) {
    unit.collectPostConditions();
  }

  auto regs = allocateRegs(unit);
  assert(checkRegisters(unit, regs)); // calls checkCfg internally.

  recordBCInstr(OpTraceletGuard, code.main(), code.main().frontier(), false);
  genCode(unit, this, regs);

  m_numHHIRTrans++;
}

MCGenerator::MCGenerator()
  : m_backEnd(newBackEnd())
  , m_numNativeTrampolines(0)
  , m_numHHIRTrans(0)
  , m_catchTraceMap(128)
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
}

void MCGenerator::initUniqueStubs() {
  // Put the following stubs into ahot, rather than a.
  CodeCache::Selector cbSel(CodeCache::Selector::Args(code).
                            hot(m_tx.useAHot()));
  m_tx.uniqueStubs = mcg->backEnd().emitUniqueStubs();
}

void MCGenerator::registerCatchBlock(CTCA ip, TCA block) {
  FTRACE(1, "registerCatchBlock: afterCall: {} block: {}\n", ip, block);
  m_fixups.m_pendingCatchTraces.emplace_back(ip, block);
}

folly::Optional<TCA> MCGenerator::getCatchTrace(CTCA ip) const {
  TCA* found = m_catchTraceMap.find(ip);
  if (found) return *found;
  return folly::none;
}

void MCGenerator::codeEmittedThisRequest(size_t& requestEntry,
                                         size_t& now) const {
  requestEntry = s_initialTCSize;
  now = code.totalUsed();
}

void MCGenerator::requestInit() {
  tl_regState = VMRegState::CLEAN;
  Timer::RequestInit();
  Treadmill::startRequest();
  memset(&s_perfCounters, 0, sizeof(s_perfCounters));
  Stats::init();
  s_initialTCSize = code.totalUsed();
}

void MCGenerator::requestExit() {
  if (Translator::WriteLease().amOwner()) {
    Translator::WriteLease().drop();
  }
  TRACE_MOD(txlease, 2, "%" PRIx64 " write lease stats: %15" PRId64
            " kept, %15" PRId64 " grabbed\n",
            Process::GetThreadIdForTrace(), Translator::WriteLease().m_hintKept,
            Translator::WriteLease().m_hintGrabbed);
  Treadmill::finishRequest();
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
}

bool
MCGenerator::isPseudoEvent(const char* event) {
  for (auto name : kPerfCounterNames) {
    if (!strcmp(event, name)) {
      return true;
    }
  }
  return false;
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
}

MCGenerator::~MCGenerator() {
}

static Debug::TCRange rangeFrom(const CodeBlock& cb, const TCA addr,
                                bool isAcold) {
  assert(cb.contains(addr));
  return Debug::TCRange(addr, cb.frontier(), isAcold);
}

void MCGenerator::recordBCInstr(uint32_t op,
                                const CodeBlock& cb,
                                const TCA addr,
                                bool cold) {
  if (addr != cb.frontier()) {
    m_debugInfo.recordBCInstr(Debug::TCRange(addr, cb.frontier(),
                                             cold), op);
  }
}

void MCGenerator::recordGdbTranslation(SrcKey sk,
                                       const Func* srcFunc,
                                       const CodeBlock& cb,
                                       const TCA start,
                                       bool exit,
                                       bool inPrologue) {
  if (start != cb.frontier()) {
    assert(Translator::WriteLease().amOwner());
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
                                const TCA start, const char* name) {
  if (!RuntimeOption::EvalJitNoGdb) {
    m_debugInfo.recordStub(rangeFrom(cb, start, &cb == &code.cold()),
                           name);
  }
}

std::string MCGenerator::getUsage() {
  std::string usage;
  size_t totalBlockSize = 0;
  size_t totalBlockCapacity = 0;

  auto addRow = [&](const std::string& name, size_t used, size_t capacity) {
    totalBlockSize += used;
    totalBlockCapacity += capacity;
    auto percent = capacity ? 100 * used / capacity : 0;
    usage += folly::format("mcg: {:9} bytes ({}%) in {}\n",
                           used, percent, name).str();
  };
  code.forEachBlock([&](const char* name, const CodeBlock& a) {
    addRow(std::string("code.") + name, a.used(), a.capacity());
  });
  // Report code.stubs usage = code.cold + code.frozen usage, so
  // ODS doesn't break.
  auto const stubsUsed = code.realCold().used() + code.realFrozen().used();
  auto const stubsCapacity = code.realCold().capacity() +
    code.realFrozen().capacity();
  addRow(std::string("code.stubs"), stubsUsed, stubsCapacity);

  addRow("data", code.data().used(), code.data().capacity());
  addRow("RDS", RDS::usedBytes(),
         RuntimeOption::EvalJitTargetCacheSize * 3 / 4);
  addRow("RDSLocal", RDS::usedLocalBytes(),
         RuntimeOption::EvalJitTargetCacheSize * 3 / 4);
  addRow("persistentRDS", RDS::usedPersistentBytes(),
         RuntimeOption::EvalJitTargetCacheSize / 4);
  addRow("total",
         totalBlockSize + code.data().used() +
         RDS::usedBytes() + RDS::usedPersistentBytes(),
         totalBlockCapacity + code.data().capacity() +
         RuntimeOption::EvalJitTargetCacheSize);

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
  // It grabs the write lease and iterating through whole SrcDB...
  bool locked = Translator::WriteLease().acquire(true);
  if (!locked) {
    return false;
  }
  assert(mcg->cgFixups().empty());
  struct timespec tsBegin, tsEnd;
  HPHP::Timer::GetMonotonicTime(tsBegin);
  // Doc says even find _could_ invalidate iterator, in pactice it should
  // be very rare, so go with it now.
  for (SrcDB::const_iterator it = m_tx.getSrcDB().begin();
       it != m_tx.getSrcDB().end(); ++it) {
    SrcKey const sk = SrcKey::fromAtomicInt(it->first);
    // We may have a SrcKey to a deleted function. NB: this may miss a
    // race with deleting a Func. See task #2826313.
    if (!Func::isFuncIdValid(sk.getFuncId())) continue;
    SrcRec* sr = it->second;
    if (sr->unitMd5() == unit->md5() &&
        !sr->hasDebuggerGuard() &&
        m_tx.isSrcKeyInBL(sk)) {
      addDbgGuardImpl(sk, sr);
    }
  }
  mcg->cgFixups().process();
  Translator::WriteLease().drop();
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
  bool locked = Translator::WriteLease().acquire(true);
  if (!locked) {
    return false;
  }
  assert(mcg->cgFixups().empty());
  {
    if (SrcRec* sr = m_tx.getSrcDB().find(sk)) {
      addDbgGuardImpl(sk, sr);
    }
  }
  mcg->cgFixups().process();
  Translator::WriteLease().drop();
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
  if (result) {
    for (auto const& pair : m_trampolineMap) {
      void* helperAddr = pair.first;
      void* trampAddr = pair.second;
      auto functionName = getNativeFunctionName(helperAddr);
      fprintf(helperAddrFile,"%10p %10p %s\n",
              trampAddr, helperAddr,
              functionName.c_str());
    }
  }
  return result;
}

// Returns true on success
bool MCGenerator::dumpTC(bool ignoreLease) {
  if (!ignoreLease && !Translator::WriteLease().acquire(true)) return false;
  bool success = dumpTCData();
  if (success) {
    success = dumpTCCode("/tmp/tc_dump");
  }
  if (!ignoreLease) Translator::WriteLease().drop();
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
  assert(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  assert(Translator::WriteLease().amOwner());
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  SrcRec* sr = m_tx.getSrcDB().find(sk);
  assert(sr);
  /*
   * Since previous translations aren't reachable from here, we know we
   * just created some garbage in the TC. We currently have no mechanism
   * to reclaim this.
   */
  sr->replaceOldTranslations();
}

void MCGenerator::setJmpTransID(TCA jmp) {
  if (m_tx.mode() != TransKind::Profile) return;

  TransID transId = m_tx.profData()->curTransID();
  FTRACE(5, "setJmpTransID: adding {} => {}\n", jmp, transId);
  m_fixups.m_pendingJmpTransIDs.emplace_back(jmp, transId);
}

void RelocationInfo::recordAddress(TCA src, TCA dest, int range) {
  assert(m_destSize == size_t(-1) || dest - m_dest >= m_destSize);
  m_destSize = dest - m_dest;
  m_adjustedAddresses.emplace(src, std::make_pair(dest, range));
}

TCA RelocationInfo::adjustedAddressAfter(TCA addr) const {
  if (size_t(addr - m_start) > size_t(m_end - m_start)) {
    return nullptr;
  }

  auto it = m_adjustedAddresses.find(addr);
  if (it == m_adjustedAddresses.end()) return nullptr;

  return it->second.first + it->second.second;
}

TCA RelocationInfo::adjustedAddressBefore(TCA addr) const {
  if (size_t(addr - m_start) > size_t(m_end - m_start)) {
    return nullptr;
  }

  auto it = m_adjustedAddresses.find(addr);
  if (it == m_adjustedAddresses.end()) return nullptr;

  return it->second.first;
}

void
emitIncStat(CodeBlock& cb, uint64_t* tl_table, uint index, int n, bool force) {
  if (!force && !Stats::enabled()) return;
  intptr_t disp = uintptr_t(&tl_table[index]) - tlsBase();

  mcg->backEnd().emitIncStat(cb, disp, n);
}

} // HPHP::JIT

} // HPHP
