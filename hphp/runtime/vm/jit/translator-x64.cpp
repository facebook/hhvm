/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/translator-x64.h"

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
#ifdef __FreeBSD__
#include <sys/ucontext.h>
#endif

#ifdef __FreeBSD__
#define RIP_REGISTER(v) (v).mc_rip
#elif defined(__APPLE__)
#define RIP_REGISTER(v) (v)->__ss.__rip
#elif defined(__x86_64__)
#define RIP_REGISTER(v) (v).gregs[REG_RIP]
#elif defined(__AARCH64EL__)
#define RIP_REGISTER(v) (v).pc
#else
#error How is rip accessed on this architecture?
#endif

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/scoped_ptr.hpp>

#include "folly/Format.h"
#include "folly/String.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/bitops.h"
#include "hphp/util/debug.h"
#include "hphp/util/disasm.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/rank.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"
#include "hphp/util/meta.h"
#include "hphp/util/process.h"
#include "hphp/util/util.h"
#include "hphp/util/repo-schema.h"
#include "hphp/util/cycles.h"

#include "hphp/vixl/a64/decoder-a64.h"
#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/vixl/a64/simulator-a64.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/arch.h"
#include "hphp/runtime/vm/jit/unique-stubs-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/php-debug.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/pendq.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/func-prologues.h"
#include "hphp/runtime/vm/jit/func-prologues-x64.h"
#include "hphp/runtime/vm/jit/func-prologues-arm.h"
#include "hphp/runtime/vm/jit/debug-guards.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/translator-x64-internal.h"

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(tx64);

using namespace reg;
using namespace Util;
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

static const char* const kInstrCountTx64Name = "instr_tx64";
static const char* const kInstrCountIRName = "instr_hhir";

#define TPC(n) "trans_" #n,
static const char* const kPerfCounterNames[] = {
  TRANS_PERF_COUNTERS
  kInstrCountTx64Name,
  kInstrCountIRName,
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

// The global TranslatorX64 object.
TranslatorX64* tx64;

// Register dirtiness: thread-private.
__thread VMRegState tl_regState = VMRegState::CLEAN;

// stubBlock --
//   Used to emit a bunch of outlined code that is unconditionally jumped to.
template <typename L>
void stubBlock(X64Assembler& hot, X64Assembler& cold, const L& body) {
  hot.  jmp(cold.frontier());
  guardDiamond(cold, body);
  cold. jmp(hot.frontier());
}

JIT::CppCall TranslatorX64::getDtorCall(DataType type) {
  switch (type) {
  case BitwiseKindOfString:
    return JIT::CppCall(getMethodPtr(&StringData::release));
  case KindOfArray:
    return JIT::CppCall(getMethodPtr(&ArrayData::release));
  case KindOfObject:
    return JIT::CppCall(getMethodPtr(&ObjectData::release));
  case KindOfResource:
    return JIT::CppCall(getMethodPtr(&ResourceData::release));
  case KindOfRef:
    return JIT::CppCall(getMethodPtr(&RefData::release));
  default:
    assert(false);
    NOT_REACHED();
  }
}

bool TranslatorX64::profileSrcKey(const SrcKey& sk) const {
  if (!sk.func()->shouldPGO()) return false;

  if (profData()->optimized(sk.getFuncId())) return false;

  // If we've hit EvalJitProfileRequests, then don't emit profiling
  // translations that would trigger an optimizing retranslation.
  // This limits the duration of profiling.  For
  // non-retranslate-triggering SrcKeys, whose profiling translations
  // only increment a counter, it's OK to emit them past the
  // EvalJitProfileRequests threshold as long as we're already
  // profiling this function (next check below) but haven't
  // retranslated this function yet (checked above).
  bool triggersRetrans = sk.func()->isEntry(sk.offset());
  if (triggersRetrans &&
      requestCount() > RuntimeOption::EvalJitProfileRequests) {
    return false;
  }

  // For translations that don't trigger a retranslation, only emit
  // them if we've already generated a retranslation-triggering
  // translation for its function.
  if (!triggersRetrans &&
      !profData()->profiling(sk.getFuncId())) {
    return false;
  }

  return true;
}

/*
 * Invalidate the SrcDB entries for func's SrcKeys that have any
 * Profile translation.
 */
void TranslatorX64::invalidateFuncProfSrcKeys(const Func* func) {
  assert(RuntimeOption::EvalJitPGO);
  FuncId funcId = func->getFuncId();
  for (auto tid : m_profData->funcProfTransIDs(funcId)) {
    invalidateSrcKey(m_profData->transSrcKey(tid));
  }
}

TCA TranslatorX64::retranslate(const TranslArgs& args) {
  SrcRec* sr = m_srcDB.find(args.m_sk);

  bool locked = sr->tryLock();
  SCOPE_EXIT {
    if (locked) sr->freeLock();
  };
  if (isDebuggerAttachedProcess() && isSrcKeyInBL(args.m_sk)) {
    // We are about to translate something known to be blacklisted by
    // debugger, exit early
    SKTRACE(1, args.m_sk, "retranslate abort due to debugger\n");
    return nullptr;
  }
  LeaseHolder writer(s_writeLease);
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
  if (m_mode == TransInvalid) {
    m_mode = profileSrcKey(args.m_sk) ? TransProfile : TransLive;
  }
  return translate(args);
}

TCA TranslatorX64::retranslateOpt(TransID transId, bool align) {
  LeaseHolder writer(s_writeLease);
  if (!writer || !shouldTranslate()) return nullptr;
  if (isDebuggerAttachedProcess()) return nullptr;

  TRACE(1, "retranslateOpt: transId = %u\n", transId);

  SCOPE_EXIT{ m_mode = TransInvalid; };

  always_assert(m_profData->transRegion(transId) != nullptr);

  Func*       func = m_profData->transFunc(transId);
  FuncId    funcId = func->getFuncId();
  const SrcKey& sk = m_profData->transSrcKey(transId);

  if (m_profData->optimized(funcId)) return nullptr;
  m_profData->setOptimized(funcId);

  bool setFuncBody = func->getDVFunclets().size() == 0;

  func->setFuncBody(uniqueStubs.funcBodyHelperThunk);

  // Invalidate SrcDB's entries for all func's SrcKeys.
  invalidateFuncProfSrcKeys(func);

  // Regenerate the prologues and DV funclets before the actual function body.
  TCA start = regeneratePrologues(func, sk);

  // Regionize func and translate all its regions.
  std::vector<JIT::RegionDescPtr> regions;
  JIT::regionizeFunc(func, this, regions);

  for (auto region : regions) {
    m_mode = TransOptimize;
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
TranslatorX64::getTranslation(const TranslArgs& args) {
  auto sk = args.m_sk;
  sk.func()->validate();
  SKTRACE(2, sk,
          "getTranslation: curUnit %s funcId %x offset %d\n",
          sk.unit()->filepath()->data(),
          sk.getFuncId(),
          sk.offset());
  SKTRACE(2, sk, "   funcId: %x \n", sk.func()->getFuncId());

  if (Translator::liveFrameIsPseudoMain()) {
    SKTRACE(2, sk, "punting on pseudoMain\n");
    return nullptr;
  }
  if (const SrcRec* sr = m_srcDB.find(sk)) {
    TCA tca = sr->getTopTranslation();
    if (tca) {
      SKTRACE(2, sk, "getTranslation: found %p\n", tca);
      return tca;
    }
  }
  return createTranslation(args);
}

int
TranslatorX64::numTranslations(SrcKey sk) const {
  if (const SrcRec* sr = m_srcDB.find(sk)) {
    return sr->translations().size();
  }
  return 0;
}

static void populateLiveContext(JIT::RegionContext& ctx) {
  typedef JIT::RegionDesc::Location L;

  const ActRec*     const fp {g_vmContext->getFP()};
  const TypedValue* const sp {g_vmContext->getStack().top()};

  for (uint32_t i = 0; i < fp->m_func->numLocals(); ++i) {
    ctx.liveTypes.push_back(
      { L::Local{i}, JIT::liveTVType(frame_local(fp, i)) }
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
        { L::Stack{stackOff, ctx.spOffset - stackOff}, JIT::liveTVType(tv) }
      );
      stackOff++;
      FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
    }
  );
}

TCA
TranslatorX64::createTranslation(const TranslArgs& args) {
  if (!shouldTranslate()) return nullptr;

  /*
   * Try to become the writer. We delay this until we *know* we will have
   * a need to create new translations, instead of just trying to win the
   * lottery at the dawn of time. Hopefully lots of requests won't require
   * any new translation.
   */
  auto sk = args.m_sk;
  LeaseHolder writer(s_writeLease);
  if (!writer) return nullptr;

  if (SrcRec* sr = m_srcDB.find(sk)) {
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
  TCA astart = mainCode.frontier();
  TCA stubstart = stubsCode.frontier();
  TCA req = emitServiceReq(stubsCode, JIT::REQ_RETRANSLATE, sk.offset());
  SKTRACE(1, sk, "inserting anchor translation for (%p,%d) at %p\n",
          sk.unit(), sk.offset(), req);
  SrcRec* sr = m_srcDB.insert(sk);
  sr->setFuncInfo(sk.func());
  sr->setAnchorTranslation(req);

  size_t asize = mainCode.frontier() - astart;
  size_t stubsize = stubsCode.frontier() - stubstart;
  assert(asize == 0);
  if (stubsize && RuntimeOption::EvalDumpTCAnchors) {
    addTranslation(TransRec(sk, sk.unit()->md5(), TransAnchor,
                            astart, asize, stubstart, stubsize));
    if (m_profData) {
      m_profData->addTransNonProf(TransAnchor, sk);
    }
    assert(!isTransDBEnabled() || getTransRec(stubstart)->kind == TransAnchor);
  }

  return retranslate(args);
}

TCA
TranslatorX64::lookupTranslation(SrcKey sk) const {
  if (SrcRec* sr = m_srcDB.find(sk)) {
    return sr->getTopTranslation();
  }
  return nullptr;
}

TCA
TranslatorX64::translate(const TranslArgs& args) {
  INC_TPC(translate);
  assert(((uintptr_t)vmsp() & (sizeof(Cell) - 1)) == 0);
  assert(((uintptr_t)vmfp() & (sizeof(Cell) - 1)) == 0);
  assert(m_mode != TransInvalid);
  SCOPE_EXIT{ m_mode = TransInvalid; };

  if (!args.m_interp) {
    if (m_numHHIRTrans == RuntimeOption::EvalJitGlobalTranslationLimit) {
      RuntimeOption::EvalJit = false;
      ThreadInfo::s_threadInfo->m_reqInjectionData.updateJit();
      return nullptr;
    }
  }

  Func* func = const_cast<Func*>(args.m_sk.func());
  CodeBlockSelector asmSel(CodeBlockSelector::Args(this)
                           .profile(m_mode == TransProfile)
                           .hot(func->attrs() & AttrHot));

  if (args.m_align) {
    JIT::X64::moveToAlign(mainCode, kNonFallthroughAlign);
  }

  TCA start = mainCode.frontier();

  translateWork(args);

  if (args.m_setFuncBody) {
    func->setFuncBody(start);
  }
  SKTRACE(1, args.m_sk, "translate moved head from %p to %p\n",
          getTopTranslation(args.m_sk), start);
  return start;
}

void TranslatorX64::protectCode() {
  mprotect(tx64->hotCode.base(),
           tx64->stubsCode.base() - tx64->hotCode.base() +
           tx64->stubsCode.capacity(), PROT_READ | PROT_EXEC);
}

void TranslatorX64::unprotectCode() {
  mprotect(tx64->hotCode.base(),
           tx64->stubsCode.base() - tx64->hotCode.base() +
           tx64->stubsCode.capacity(),
           PROT_READ | PROT_WRITE | PROT_EXEC);
}

TCA
TranslatorX64::getCallArrayPrologue(Func* func) {
  TCA tca = func->getFuncBody();
  if (tca != uniqueStubs.funcBodyHelperThunk) return tca;

  DVFuncletsVec dvs = func->getDVFunclets();

  if (dvs.size()) {
    LeaseHolder writer(s_writeLease);
    if (!writer) return nullptr;
    tca = func->getFuncBody();
    if (tca != uniqueStubs.funcBodyHelperThunk) return tca;
    if (JIT::arch() == JIT::Arch::X64) {
      tca = JIT::X64::emitCallArrayPrologue(func, dvs);
    } else if (JIT::arch() == JIT::Arch::ARM) {
      tca = JIT::ARM::emitCallArrayPrologue(func, dvs);
    } else {
      not_implemented();
    }
    func->setFuncBody(tca);
  } else {
    SrcKey sk(func, func->base());
    tca = tx64->getTranslation(TranslArgs(sk, false).setFuncBody());
  }

  return tca;
}

void
TranslatorX64::smashPrologueGuards(TCA* prologues, int numPrologues,
                                   const Func* func) {
  DEBUG_ONLY std::unique_ptr<LeaseHolder> writer;
  for (int i = 0; i < numPrologues; i++) {
    if (prologues[i] != uniqueStubs.fcallHelperThunk
        && JIT::funcPrologueHasGuard(prologues[i], func)) {
      if (debug) {
        /*
         * Unit's are sometimes created racily, in which case all
         * but the first are destroyed immediately. In that case,
         * the Funcs of the destroyed Units never need their
         * prologues smashing, and it would be a lock rank violation
         * to take the write lease here.
         * In all other cases, Funcs are destroyed via a delayed path
         * (treadmill) and the rank violation isn't an issue.
         *
         * Also note that we only need the write lease because we
         * mprotect the translation cache in debug builds.
         */
        if (!writer) {
          writer.reset(new LeaseHolder(s_writeLease, LeaseAcquire::BLOCKING));
        }
      }
      if (JIT::arch() == JIT::Arch::X64) {
        JIT::X64::funcPrologueSmashGuard(prologues[i], func);
      } else if (JIT::arch() == JIT::Arch::ARM) {
        JIT::ARM::funcPrologueSmashGuard(prologues[i], func);
      } else {
        not_implemented();
      }
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
TranslatorX64::checkCachedPrologue(const Func* func, int paramIdx,
                                   TCA& prologue) const {
  prologue = (TCA)func->getPrologue(paramIdx);
  if (prologue != uniqueStubs.fcallHelperThunk) {
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
  vmfp() = (Cell*)ar;
  vmsp() = sp;
  vmpc() = ar->unit()->at(pcOff);
}

TCA
TranslatorX64::getFuncPrologue(Func* func, int nPassed, ActRec* ar) {
  func->validate();
  TRACE(1, "funcPrologue %s(%d)\n", func->fullName()->data(), nPassed);
  int numParams = func->numParams();
  int paramIndex = nPassed <= numParams ? nPassed : numParams + 1;

  bool const funcIsMagic = func->isMagic();

  // Do a quick test before grabbing the write lease
  TCA prologue;
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  Offset entry = func->getEntryForNumArgs(nPassed);
  SrcKey funcBody(func, entry);

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

  LeaseHolder writer(s_writeLease);
  if (!writer || !shouldTranslate()) return nullptr;

  // Double check the prologue array now that we have the write lease
  // in case another thread snuck in and set the prologue already.
  if (checkCachedPrologue(func, paramIndex, prologue)) return prologue;

  // We're comming from a BIND_CALL service request, so enable
  // profiling if we haven't optimized the function entry yet.
  assert(m_mode == TransInvalid || m_mode == TransPrologue);
  if (m_mode == TransInvalid && profileSrcKey(funcBody)) {
    m_mode = TransProflogue;
  } else {
    m_mode = TransPrologue;
  }
  SCOPE_EXIT{ m_mode = TransInvalid; };

  CodeBlockSelector asmSel(CodeBlockSelector::Args(this)
                           .profile(m_mode == TransProflogue)
                           .hot(func->attrs() & AttrHot));

  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  if (((uintptr_t)mainCode.frontier() & kX64CacheLineMask) >= 32) {
    JIT::X64::moveToAlign(mainCode, kX64CacheLineSize);
  }

  // Careful: this isn't necessarily the real entry point. For funcIsMagic
  // prologues, this is just a possible prologue.
  TCA aStart    = mainCode.frontier();
  TCA start     = aStart;
  TCA stubStart = stubsCode.frontier();

  auto const skFuncBody = [&] {
    switch (JIT::arch()) {
    case JIT::Arch::X64:
      return funcIsMagic
        ? JIT::X64::emitMagicFuncPrologue(func, nPassed, start)
        : JIT::X64::emitFuncPrologue(func, nPassed, start);
    case JIT::Arch::ARM:
      return JIT::ARM::emitFuncPrologue(
        mainCode, stubsCode, func, funcIsMagic, nPassed, start, aStart
      );
    }
    not_reached();
  }();

  assert(JIT::funcPrologueHasGuard(start, func));
  TRACE(2, "funcPrologue tx64 %p %s(%d) setting prologue %p\n",
        this, func->fullName()->data(), nPassed, start);
  assert(isValidCodeAddress(start));
  func->setPrologue(paramIndex, start);

  assert(m_mode == TransPrologue || m_mode == TransProflogue);
  addTranslation(TransRec(skFuncBody, func->unit()->md5(),
                          m_mode, aStart, mainCode.frontier() - aStart,
                          stubStart, stubsCode.frontier() - stubStart));

  if (m_profData) {
    m_profData->addTransPrologue(m_mode, skFuncBody, paramIndex);
  }

  recordGdbTranslation(skFuncBody, func,
                       mainCode, aStart,
                       false, true);
  recordBCInstr(OpFuncPrologue, mainCode, start);

  return start;
}

/**
 * Given the proflogueTransId for a TransProflogue translation,
 * regenerate the prologue (as a TransPrologue).  Returns the starting
 * address for the translation corresponding to triggerSk, if such
 * translation is generated; otherwise returns nullptr.
 */
TCA TranslatorX64::regeneratePrologue(TransID prologueTransId,
                                      SrcKey triggerSk) {
  Func* func = m_profData->transFunc(prologueTransId);
  int  nArgs = m_profData->prologueArgs(prologueTransId);

  // Regenerate the prologue.
  func->resetPrologue(nArgs);
  m_mode = TransPrologue;
  SCOPE_EXIT { m_mode = TransInvalid; };
  TCA start = getFuncPrologue(func, nArgs);
  func->setPrologue(nArgs, start);

  // Smash callers of the old prologue with the address of the new one.
  JIT::PrologueCallersRec* pcr = m_profData->prologueCallers(prologueTransId);
  for (TCA toSmash : pcr->mainCallers()) {
    JIT::smashCall(toSmash, start);
  }
  // If the prologue has a guard, then smash its guard-callers as well.
  if (JIT::funcPrologueHasGuard(start, func)) {
    TCA guard = JIT::funcPrologueToGuard(start, func);
    for (TCA toSmash : pcr->guardCallers()) {
      JIT::smashCall(toSmash, guard);
    }
  }
  pcr->clearAllCallers();

  // If this prologue has a DV funclet, then generate a translation
  // for the DV funclet right after the prologue.  However, skip
  // cloned closures because their prologues are actually the DV
  // funclets already.
  TCA triggerSkStart = nullptr;
  if (nArgs < func->numParams() && !func->isClonedClosure()) {
    auto paramInfo = func->params()[nArgs];
    if (paramInfo.hasDefaultValue()) {
      m_mode = TransOptimize;
      SrcKey  funcletSK(func, paramInfo.funcletOff());
      TransID funcletTransId = m_profData->dvFuncletTransId(func, nArgs);
      if (funcletTransId != InvalidID) {
        invalidateSrcKey(funcletSK);
        TCA dvStart = retranslate(TranslArgs(funcletSK, false).
                                  transId(funcletTransId));
        if (dvStart && !triggerSkStart && funcletSK == triggerSk) {
          triggerSkStart = dvStart;
        }
        // Flag that this translation has been retranslated, so that
        // it's not retranslated again along with the function body.
        m_profData->setOptimized(funcletSK);
      }
    }
  }

  return triggerSkStart;;
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
TCA TranslatorX64::regeneratePrologues(Func* func, SrcKey triggerSk) {
  TCA triggerStart = nullptr;
  std::vector<TransID> prologTransIDs;

  for (int nArgs = 0; nArgs <= func->numParams() + 1; nArgs++) {
    TransID tid = profData()->prologueTransId(func, nArgs);
    assert(IMPLIES(func->getPrologue(nArgs) != uniqueStubs.fcallHelperThunk,
                   tid != InvalidID));
    if (tid != InvalidID) {
      prologTransIDs.push_back(tid);
    }
  }

  std::sort(prologTransIDs.begin(), prologTransIDs.end(),
          [&](TransID t1, TransID t2) -> bool {
            // This will sort in ascending order. Note that transCounters start
            // at JitPGOThreshold and count down.
            return m_profData->transCounter(t1) > m_profData->transCounter(t2);
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
TranslatorX64::bindJmp(TCA toSmash, SrcKey destSk,
                       JIT::ServiceRequest req, bool& smashed) {
  TCA tDest = getTranslation(TranslArgs(destSk, false));
  if (!tDest) return nullptr;
  LeaseHolder writer(s_writeLease);
  if (!writer) return tDest;
  SrcRec* sr = getSrcRec(destSk);
  if (req == JIT::REQ_BIND_ADDR) {
    auto addr = reinterpret_cast<TCA*>(toSmash);
    if (*addr == tDest) {
      // Already smashed
      return tDest;
    }
    sr->chainFrom(IncomingBranch::addr(addr));
  } else if (req == JIT::REQ_BIND_JCC || req == JIT::REQ_BIND_SIDE_EXIT) {
    auto jt = JIT::jccTarget(toSmash);
    assert(jt);
    if (jt == tDest) {
      // Already smashed
      return tDest;
    }
    sr->chainFrom(IncomingBranch::jccFrom(toSmash));
  } else {
    assert(!JIT::jccTarget(toSmash));
    if (!JIT::jmpTarget(toSmash) || JIT::jmpTarget(toSmash) == tDest) {
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
TranslatorX64::bindJmpccFirst(TCA toSmash,
                              Offset offTaken, Offset offNotTaken,
                              bool taken,
                              ConditionCode cc,
                              bool& smashed) {
  const Func* f = liveFunc();
  LeaseHolder writer(s_writeLease);
  if (!writer) return nullptr;
  Offset offWillExplore = taken ? offTaken : offNotTaken;
  Offset offWillDefer = taken ? offNotTaken : offTaken;
  SrcKey dest(f, offWillExplore);
  TRACE(3, "bindJmpccFirst: explored %d, will defer %d; overwriting cc%02x "
        "taken %d\n",
        offWillExplore, offWillDefer, cc, taken);

  // We want the branch to point to whichever side has not been explored
  // yet.
  if (taken) {
    cc = ccNegate(cc);
  }

  auto& cb = codeBlockFor(toSmash);
  Asm as { cb };
  // Its not clear where chainFrom should go to if as is astubs
  assert(&cb != &stubsCode);

  using namespace JIT::X64;

  // can we just directly fall through?
  // a jmp + jz takes 5 + 6 = 11 bytes
  bool fallThru = toSmash + kJmpccLen + kJmpLen == cb.frontier() &&
    !m_srcDB.find(dest);

  TCA tDest;
  tDest = getTranslation(TranslArgs(dest, !fallThru));
  if (!tDest) {
    return 0;
  }

  if (JIT::jmpTarget(toSmash + kJmpccLen) != JIT::jccTarget(toSmash)) {
    // someone else already smashed this one. Ideally we would
    // just re-execute from toSmash - except the flags will have
    // been trashed.
    return tDest;
  }

  TCA stub = emitEphemeralServiceReq(stubsCode, getFreeStub(),
                                     JIT::REQ_BIND_JMPCC_SECOND, toSmash,
                                     offWillDefer, cc);

  smashed = true;
  assert(s_writeLease.amOwner());
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
  getSrcRec(dest)->chainFrom(IncomingBranch::jmpFrom(cb.frontier()));
  TRACE(5, "bindJmpccFirst: overwrote with cc%02x taken %d\n", cc, taken);
  return tDest;
}

// smashes a jcc to point to a new destination
TCA
TranslatorX64::bindJmpccSecond(TCA toSmash, const Offset off,
                               ConditionCode cc, bool& smashed) {
  const Func* f = liveFunc();
  SrcKey dest(f, off);
  TCA branch = getTranslation(TranslArgs(dest, true));
  if (branch) {
    LeaseHolder writer(s_writeLease);
    if (writer) {
      if (branch == JIT::jccTarget(toSmash)) {
        // already smashed
        return branch;
      } else {
        smashed = true;
        SrcRec* destRec = getSrcRec(dest);
        destRec->chainFrom(IncomingBranch::jccFrom(toSmash));
      }
    }
  }
  return branch;
}

void TranslatorX64::emitResolvedDeps(const ChangeMap& resolvedDeps) {
  for (const auto dep : resolvedDeps) {
    m_irTrans->assertType(dep.first, dep.second->rtt);
  }
}

void
TranslatorX64::checkRefs(SrcKey sk,
                         const RefDeps& refDeps,
                         SrcRec& fail) {
  if (refDeps.size() == 0) {
    return;
  }

  // Set up guards for each pushed ActRec that we've made reffiness
  // assumptions about
  for (RefDeps::ArMap::const_iterator it = refDeps.m_arMap.begin();
       it != refDeps.m_arMap.end(); ++it) {
    // Be careful! The actual Func might have fewer refs than the number
    // of args we're passing. To forestall this, we always prepare at
    // least 64 bits in the Func, and always fill out the refBitVec
    // to a multiple of 64 bits

    int entryArDelta = it->first;

    m_irTrans->hhbcTrans().guardRefs(entryArDelta,
                                     it->second.m_mask,
                                     it->second.m_vals);
  }
}

class FreeRequestStubTrigger : public Treadmill::WorkItem {
  TCA m_stub;
 public:
  explicit FreeRequestStubTrigger(TCA stub) : m_stub(stub) {
    TRACE(3, "FreeStubTrigger @ %p, stub %p\n", this, m_stub);
  }
  virtual void operator()() {
    TRACE(3, "FreeStubTrigger: Firing @ %p , stub %p\n", this, m_stub);
    if (tx64->freeRequestStub(m_stub) != true) {
      // If we can't free the stub, enqueue again to retry.
      TRACE(3, "FreeStubTrigger: write lease failed, requeueing %p\n", m_stub);
      enqueue(new FreeRequestStubTrigger(m_stub));
    }
  }
};

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

/*
 * enterTCHelper does not save callee-saved registers except %rbp. This means
 * when we call it from C++, we have to tell gcc to clobber all the other
 * callee-saved registers.
 */
#if defined(__x86_64__)
#  define CALLEE_SAVED_BARRIER() \
  asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15")
#elif defined(__AARCH64EL__)
#  define CALLEE_SAVED_BARRIER() \
  asm volatile("" : : : "x19", "x20", "x21", "x22", "x23", "x24", "x25", \
               "x26", "x27", "x28")
#else
#  error What are the callee-saved registers on your system?
#endif

/*
 * enterTCHelper is a handwritten assembly function that transfers control in
 * and out of the TC.
 */
static_assert(X64::rVmSp == rbx &&
              X64::rVmFp == rbp &&
              X64::rVmTl == r12 &&
              X64::rStashedAR == r15,
              "__enterTCHelper needs to be modified to use the correct ABI");
static_assert(JIT::REQ_BIND_CALL == 0x1,
              "Update assembly test for REQ_BIND_CALL in __enterTCHelper");
extern "C" void enterTCHelper(Cell* vm_sp,
                              Cell* vm_fp,
                              TCA start,
                              TReqInfo* infoPtr,
                              ActRec* firstAR,
                              void* targetCacheBase);


struct TReqInfo {
  uintptr_t requestNum;
  uintptr_t args[5];

  // Some TC registers need to be preserved across service requests.
  uintptr_t saved_rStashedAr;

  // Stub addresses are passed back to allow us to recycle used stubs.
  TCA stubAddr;
};


void
TranslatorX64::enterTC(TCA start, void* data) {
  if (debug) {
    fflush(stdout);
    fflush(stderr);
  }
  DepthGuard d;
  TReqInfo info;
  SrcKey sk;

  if (LIKELY(start != nullptr)) {
    info.requestNum = data ? JIT::REQ_BIND_CALL : -1;
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

    s_writeLease.gremlinUnlock();
    // Keep dispatching until we end up somewhere the translator
    // recognizes, or we luck out and the leaseholder exits.
    while (!start) {
      TRACE(2, "enterTC forwarding BB to interpreter\n");
      g_vmContext->m_pc = sk.unit()->at(sk.offset());
      INC_TPC(interp_bb);
      g_vmContext->dispatchBB();
      PC newPc = g_vmContext->getPC();
      if (!newPc) { g_vmContext->m_fp = 0; return; }
      sk = SrcKey(liveFunc(), newPc);
      start = getTranslation(TranslArgs(sk, true));
    }
    assert(start == uniqueStubs.funcBodyHelperThunk ||
           isValidCodeAddress(start) ||
           (start == uniqueStubs.fcallHelperThunk &&
            info.saved_rStashedAr == (uintptr_t)data));
    assert(!s_writeLease.amOwner());
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

    if (RuntimeOption::EvalSimulateARM) {
      vixl::PrintDisassembler disasm(std::cout);
      vixl::Decoder decoder;
      if (getenv("ARM_DISASM")) {
        decoder.AppendVisitor(&disasm);
      }
      vixl::Simulator sim(&decoder, std::cout);
      sim.set_exception_hook(
        [] (vixl::Simulator* s) {
          if (tl_regState == VMRegState::DIRTY) {
            // This is a pseudo-copy of the logic in sync_regstate.
            tx64->fixupMap().fixupWorkSimulated(g_vmContext);
            tl_regState = VMRegState::CLEAN;
          }
        }
      );

      g_vmContext->m_activeSims.push_back(&sim);
      SCOPE_EXIT { g_vmContext->m_activeSims.pop_back(); };

      sim.   set_xreg(JIT::ARM::rGContextReg.code(), g_vmContext);
      sim.   set_xreg(JIT::ARM::rVmFp.code(), vmfp());
      sim.   set_xreg(JIT::ARM::rVmSp.code(), vmsp());
      sim.   set_xreg(JIT::ARM::rVmTl.code(), RDS::tl_base);
      sim.   set_xreg(JIT::ARM::rStashedAR.code(), info.saved_rStashedAr);

      std::cout.flush();
      sim.RunFrom(vixl::Instruction::Cast(start));
      std::cout.flush();

      info.requestNum = sim.xreg(0);
      info.args[0] = sim.xreg(1);
      info.args[1] = sim.xreg(2);
      info.args[2] = sim.xreg(3);
      info.args[3] = sim.xreg(4);
      info.args[4] = sim.xreg(5);
      info.saved_rStashedAr = sim.xreg(JIT::ARM::rStashedAR.code());

      info.stubAddr = reinterpret_cast<TCA>(sim.xreg(JIT::ARM::rAsm.code()));
    } else {
      // We have to force C++ to spill anything that might be in a callee-saved
      // register (aside from rbp). enterTCHelper does not save them.
      CALLEE_SAVED_BARRIER();
      enterTCHelper(vmsp(), vmfp(), start, &info, vmFirstAR(),
                    RDS::tl_base);
      CALLEE_SAVED_BARRIER();
    }

    assert(g_vmContext->m_stack.isValidAddress((uintptr_t)vmsp()));

    tl_regState = VMRegState::CLEAN; // Careful: pc isn't sync'ed yet.
    TRACE(1, "enterTC: %p fp%p sp%p } return\n", start,
          vmfp(), vmsp());

    if (debug) {
      // Debugging code: cede the write lease half the time.
      if (RuntimeOption::EvalJitStressLease) {
        if (d.depthOne() && (rand() % 2) == 0) {
          s_writeLease.gremlinLock();
        }
      }
      // Ensure that each case either returns, or drives start to a valid
      // value.
      start = TCA(0xbee5face);
    }

    TRACE(2, "enterTC: request(%s) args: %" PRIxPTR " %" PRIxPTR " %"
             PRIxPTR " %" PRIxPTR " %" PRIxPTR "\n",
          JIT::serviceReqName(info.requestNum),
          info.args[0], info.args[1], info.args[2], info.args[3],
          info.args[4]);

    if (LIKELY(info.requestNum == JIT::REQ_EXIT)) {
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
bool TranslatorX64::handleServiceRequest(TReqInfo& info,
                                         TCA& start,
                                         SrcKey& sk) {
  const JIT::ServiceRequest requestNum =
    static_cast<JIT::ServiceRequest>(info.requestNum);
  auto* const args = info.args;
  assert(requestNum != JIT::REQ_EXIT);
  INC_TPC(service_req);

  bool smashed = false;
  switch (requestNum) {
  case JIT::REQ_BIND_CALL: {
    JIT::ReqBindCall* req = reinterpret_cast<JIT::ReqBindCall*>(args[0]);
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
      dest = JIT::funcPrologueToGuard(dest, func);
    } else {
      TRACE(2, "enterTC: bindCall immutably %s -> %p\n",
            func->fullName()->data(), dest);
    }
    if (dest) {
      LeaseHolder writer(s_writeLease);
      if (writer && JIT::callTarget(toSmash) != dest) {
        TRACE(2, "enterTC: bindCall smash %p -> %p\n", toSmash, dest);
        JIT::smashCall(toSmash, dest);
        smashed = true;
        // For functions to be PGO'ed, if their prologues haven't been
        // regenerated yet, then save toSmash as a caller to the
        // prologue, so that it can later be smashed to call a new
        // prologue when it's generated.
        int calleeNumParams = func->numParams();
        int calledPrologNumArgs = (nArgs <= calleeNumParams ?
                                   nArgs :  calleeNumParams + 1);
        SrcKey calleeSK = {func, func->getEntryForNumArgs(calledPrologNumArgs)};
        if (profileSrcKey(calleeSK)) {
          if (isImmutable) {
            m_profData->addPrologueMainCaller(func, calledPrologNumArgs,
                                              toSmash);
          } else {
            m_profData->addPrologueGuardCaller(func, calledPrologNumArgs,
                                               toSmash);
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
      info.requestNum = ~JIT::REQ_BIND_CALL;
    }
    start = dest;
  } break;

  case JIT::REQ_BIND_SIDE_EXIT:
  case JIT::REQ_BIND_JMP:
  case JIT::REQ_BIND_JCC:
  case JIT::REQ_BIND_ADDR:
  {
    TCA toSmash = (TCA)args[0];
    Offset off = args[1];
    sk = SrcKey(liveFunc(), off);
    if (requestNum == JIT::REQ_BIND_SIDE_EXIT) {
      SKTRACE(3, sk, "side exit taken!\n");
    }
    start = bindJmp(toSmash, sk, requestNum, smashed);
  } break;

  case JIT::REQ_BIND_JMPCC_FIRST: {
    TCA toSmash = (TCA)args[0];
    Offset offTaken = (Offset)args[1];
    Offset offNotTaken = (Offset)args[2];
    ConditionCode cc = ConditionCode(args[3]);
    bool taken = int64_t(args[4]) & 1;
    start = bindJmpccFirst(toSmash, offTaken, offNotTaken,
                           taken, cc, smashed);
    // SrcKey: we basically need to emulate the fail
    sk = SrcKey(liveFunc(), taken ? offTaken : offNotTaken);
  } break;

  case JIT::REQ_BIND_JMPCC_SECOND: {
    TCA toSmash = (TCA)args[0];
    Offset off = (Offset)args[1];
    ConditionCode cc = ConditionCode(args[2]);
    start = bindJmpccSecond(toSmash, off, cc, smashed);
    sk = SrcKey(liveFunc(), off);
  } break;

  case JIT::REQ_RETRANSLATE_OPT: {
    FuncId  funcId  = (FuncId) args[0];
    Offset  offset  = (Offset) args[1];
    TransID transId = (TransID)args[2];
    sk = SrcKey(funcId, offset);
    start = retranslateOpt(transId, false);
    SKTRACE(2, sk, "retranslated-OPT: transId = %d  start: @%p\n", transId,
            start);
    break;
  }

  case JIT::REQ_RETRANSLATE: {
    INC_TPC(retranslate);
    sk = SrcKey(liveFunc(), (Offset)args[0]);
    start = retranslate(TranslArgs(sk, true));
    SKTRACE(2, sk, "retranslated @%p\n", start);
  } break;

  case JIT::REQ_INTERPRET: {
    Offset off = args[0];
    int numInstrs = args[1];
    g_vmContext->m_pc = liveUnit()->at(off);
    /*
     * We know the compilation unit has not changed; basic blocks do
     * not span files. I claim even exceptions do not violate this
     * axiom.
     */
    assert(numInstrs >= 0);
    SKTRACE(5, SrcKey(liveFunc(), off), "interp: enter\n");
    if (numInstrs) {
      s_perfCounters[tpc_interp_instr] += numInstrs;
      g_vmContext->dispatchN(numInstrs);
    } else {
      // numInstrs == 0 means it wants to dispatch until BB ends
      INC_TPC(interp_bb);
      g_vmContext->dispatchBB();
    }
    PC newPc = g_vmContext->getPC();
    if (!newPc) { g_vmContext->m_fp = 0; return false; }
    SrcKey newSk(liveFunc(), newPc);
    SKTRACE(5, newSk, "interp: exit\n");
    sk = newSk;
    start = getTranslation(TranslArgs(newSk, true));
  } break;

  case JIT::REQ_POST_INTERP_RET: {
    // This is only responsible for the control-flow aspect of the Ret:
    // getting to the destination's translation, if any.
    ActRec* ar = (ActRec*)args[0];
    ActRec* caller = (ActRec*)args[1];
    assert((Cell*) caller == vmfp());
    Unit* destUnit = caller->m_func->unit();
    // Set PC so logging code in getTranslation doesn't get confused.
    vmpc() = destUnit->at(caller->m_func->base() + ar->m_soff);
    SrcKey dest(caller->m_func, vmpc());
    sk = dest;
    start = getTranslation(TranslArgs(dest, true));
    TRACE(3, "REQ_POST_INTERP_RET: from %s to %s\n",
          ar->m_func->fullName()->data(),
          caller->m_func->fullName()->data());
  } break;

  case JIT::REQ_RESUME: {
    if (UNLIKELY(vmpc() == 0)) {
      g_vmContext->m_fp = 0;
      return false;
    }
    SrcKey dest(liveFunc(), vmpc());
    sk = dest;
    start = getTranslation(TranslArgs(dest, true));
  } break;

  case JIT::REQ_STACK_OVERFLOW:
    if (((ActRec*)info.saved_rStashedAr)->m_savedRbp == (uintptr_t)vmfp()) {
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
      assert(isFCallStar(toOp(*vmpc())));
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

  case JIT::REQ_EXIT:
    not_reached();
  }

  if (smashed && info.stubAddr) {
    Treadmill::WorkItem::enqueue(new FreeRequestStubTrigger(info.stubAddr));
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
TranslatorX64::freeRequestStub(TCA stub) {
  LeaseHolder writer(s_writeLease);
  /*
   * If we can't acquire the write lock, the caller
   * (FreeRequestStubTrigger) retries
   */
  if (!writer) return false;
  assert(stubsCode.contains(stub));
  m_freeStubs.push(stub);
  return true;
}

TCA TranslatorX64::getFreeStub() {
  TCA ret = m_freeStubs.maybePop();
  if (ret) {
    Stats::inc(Stats::Astubs_Reused);
    assert(m_freeStubs.m_list == nullptr ||
           stubsCode.contains(TCA(m_freeStubs.m_list)));
    TRACE(1, "recycle stub %p\n", ret);
  } else {
    ret = stubsCode.frontier();
    Stats::inc(Stats::Astubs_New);
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
VMExecutionContext*                                                     \
interpOne##opcode(ActRec* ar, Cell* sp, Offset pcOff) {                 \
  interp_set_regs(ar, sp, pcOff);                                       \
  SKTRACE(5, SrcKey(liveFunc(), vmpc()), "%40s %p %p\n",                \
          "interpOne" #opcode " before (fp,sp)",                        \
          vmfp(), vmsp());                                              \
  assert(toOp(*vmpc()) == Op::opcode);                                  \
  VMExecutionContext* ec = g_vmContext;                                 \
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

TCA TranslatorX64::getTranslatedCaller() const {
  DECLARE_FRAME_POINTER(fp);
  ActRec* framePtr = fp;  // can't directly mutate the register-mapped one
  for (; framePtr; framePtr = (ActRec*)framePtr->m_savedRbp) {
    TCA rip = (TCA)framePtr->m_savedRip;
    if (isValidCodeAddress(rip)) {
      return rip;
    }
  }
  return nullptr;
}

void
TranslatorX64::syncWork() {
  assert(tl_regState == VMRegState::DIRTY);
  m_fixupMap.fixup(g_vmContext);
  tl_regState = VMRegState::CLEAN;
  Stats::inc(Stats::TC_Sync);
}

// could be static but used in hopt/codegen.cpp
void raiseUndefVariable(StringData* nm) {
  raise_notice(Strings::UNDEFINED_VARIABLE, nm->data());
  // FIXME: do we need to decref the string if an exception is propagating?
  decRefStr(nm);
}

TCA
TranslatorX64::emitNativeTrampoline(TCA helperAddr) {
  if (!trampolinesCode.canEmit(kExpectedPerTrampolineSize)) {
    // not enough space to emit a trampoline, so just return the
    // helper address and emitCall will the emit the right sequence
    // to call it indirectly
    TRACE(1, "Ran out of space to emit a trampoline for %p\n", helperAddr);
    always_assert(false);
    return helperAddr;
  }

  uint32_t index = m_numNativeTrampolines++;
  TCA trampAddr = trampolinesCode.frontier();
  if (Stats::enabled()) {
    Stats::emitInc(trampolinesCode, &Stats::tl_helper_counters[0], index);
    char* name = getNativeFunctionName(helperAddr);
    const size_t limit = 50;
    if (strlen(name) > limit) {
      name[limit] = '\0';
    }
    Stats::helperNames[index].store(name, std::memory_order_release);
  }

  Asm a { trampolinesCode };
  a.    jmp    (helperAddr);
  a.    ud2    ();

  trampolineMap[helperAddr] = trampAddr;
  recordBCInstr(OpNativeTrampoline, trampolinesCode, trampAddr);
  return trampAddr;
}

TCA
TranslatorX64::getNativeTrampoline(TCA helperAddr) {
  if (!RuntimeOption::EvalJitTrampolines && !Stats::enabled()) {
    return helperAddr;
  }
  TCA trampAddr = (TCA)mapGet<PointerMap>(trampolineMap, helperAddr);
  if (trampAddr) {
    return trampAddr;
  }
  return emitNativeTrampoline(helperAddr);
}

bool
TranslatorX64::reachedTranslationLimit(SrcKey sk,
                                       const SrcRec& srcRec) const {
  if (srcRec.translations().size() == RuntimeOption::EvalJitMaxTranslations) {
    INC_TPC(max_trans);
    if (debug && Trace::moduleEnabled(Trace::tx64, 2)) {
      const auto& tns = srcRec.translations();
      TRACE(1, "Too many (%zd) translations: %s, BC offset %d\n",
            tns.size(), sk.unit()->filepath()->data(),
            sk.offset());
      SKTRACE(2, sk, "{\n");
      TCA topTrans = srcRec.getTopTranslation();
      for (size_t i = 0; i < tns.size(); ++i) {
        const TransRec* rec = getTransRec(tns[i]);
        assert(rec);
        SKTRACE(2, sk, "%zd %p\n", i, tns[i]);
        if (tns[i] == topTrans) {
          SKTRACE(2, sk, "%zd: *Top*\n", i);
        }
        if (rec->kind == TransAnchor) {
          SKTRACE(2, sk, "%zd: Anchor\n", i);
        } else {
          SKTRACE(2, sk, "%zd: guards {\n", i);
          for (unsigned j = 0; j < rec->dependencies.size(); ++j) {
            TRACE(2, rec->dependencies[j]);
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
TranslatorX64::emitGuardChecks(SrcKey sk,
                               const ChangeMap& dependencies,
                               const RefDeps& refDeps,
                               SrcRec& fail) {
  if (Trace::moduleEnabled(Trace::stats, 2)) {
    Stats::emitInc(mainCode, Stats::TraceletGuard_enter);
  }

  m_irTrans->hhbcTrans().emitRB(RBTypeTraceletGuards, sk);
  bool checkOuterTypeOnly = m_mode != TransProfile;
  for (auto const& dep : dependencies) {
    m_irTrans->checkType(dep.first, dep.second->rtt, checkOuterTypeOnly);
  }

  checkRefs(sk, refDeps, fail);

  if (Trace::moduleEnabled(Trace::stats, 2)) {
    Stats::emitInc(mainCode, Stats::TraceletGuard_execute);
  }
}


void dumpTranslationInfo(const Tracelet& t, TCA postGuards) {
  if (!debug) return;

  SrcKey sk = t.m_sk;
  DEBUG_ONLY auto unit = sk.unit();

  TRACE(3, "----------------------------------------------\n");
  TRACE(3, "  Translating from file %s:%d %s at %p:\n",
        unit->filepath()->data(),
        unit->getLineNumber(sk.offset()),
        sk.func()->name()->data(),
        postGuards);
  TRACE(3, "  preconds:\n");
  TRACE(3, "    types:\n");
  for (DepMap::const_iterator i = t.m_dependencies.begin();
       i != t.m_dependencies.end(); ++i) {
    TRACE(3, "      %-5s\n", i->second->pretty().c_str());
  }
  if (t.m_refDeps.size() != 0) {
    TRACE(3, "    refs:\n");
    for (RefDeps::ArMap::const_iterator i = t.m_refDeps.m_arMap.begin();
        i != t.m_refDeps.m_arMap.end();
        ++i) {
      TRACE(3, "      (ActRec %" PRId64 " : %-5s)\n", i->first,
        i->second.pretty().c_str());
    }
  }
  TRACE(3, "  postconds:\n");
  for (ChangeMap::const_iterator i = t.m_changes.begin();
       i != t.m_changes.end(); ++i) {
    TRACE(3, "    %-5s\n", i->second->pretty().c_str());
  }
  for (auto ni = t.m_instrStream.first; ni; ni = ni->next) {
    TRACE(3, "  %6d: %s\n", ni->source.offset(),
      instrToString((Op*)ni->pc()).c_str());
    if (ni->breaksTracelet) break;
  }
  TRACE(3, "----------------------------------------------\n");
  if (Trace::moduleEnabled(Trace::tx64, 5)) {
    // prettyStack() expects to use vmpc(). Leave it in the state we
    // found it since this code is debug-only, and we don't want behavior
    // to vary across the optimized/debug builds.
    PC oldPC = vmpc();
    vmpc() = unit->at(sk.offset());
    TRACE(3, g_vmContext->prettyStack(string(" tx64 ")));
    vmpc() = oldPC;
    TRACE(3, "----------------------------------------------\n");
  }
}

void
TranslatorX64::translateWork(const TranslArgs& args) {
  auto sk = args.m_sk;
  std::unique_ptr<Tracelet> tp;

  SKTRACE(1, sk, "translateWork\n");
  assert(m_srcDB.find(sk));

  TCA        start = mainCode.frontier();
  TCA        stubStart = stubsCode.frontier();
  SrcRec&    srcRec = *getSrcRec(sk);
  TransKind  transKind = TransInterp;
  UndoMarker undoA(mainCode);
  UndoMarker undoAstubs(stubsCode);

  auto resetState = [&] {
    undoA.undo();
    undoAstubs.undo();
    m_fixupMap.clearPendingFixups();
    m_pendingCatchTraces.clear();
    m_bcMap.clear();
    srcRec.clearInProgressTailJumps();
  };

  auto assertCleanState = [&] {
    assert(mainCode.frontier() == start);
    assert(stubsCode.frontier() == stubStart);
    assert(m_fixupMap.pendingFixupsEmpty());
    assert(m_pendingCatchTraces.empty());
    assert(m_bcMap.empty());
    assert(srcRec.inProgressTailJumps().empty());
  };

  JIT::PostConditions pconds;
  if (!args.m_interp && !reachedTranslationLimit(sk, srcRec)) {
    // Attempt to create a region at this SrcKey
    JIT::RegionDescPtr region;
    if (RuntimeOption::EvalJitPGO) {
      if (m_mode == TransOptimize) {
        region = args.m_region;
        if (region) {
          assert(region->blocks.size() > 0);
        } else {
          TransID transId = args.m_transId;
          assert(transId != InvalidID);
          region = JIT::selectHotRegion(transId, this);
          assert(region);
          if (region && region->blocks.size() == 0) region = nullptr;
        }
      } else {
        assert(m_mode == TransProfile || m_mode == TransLive);
        tp = analyze(sk);
      }
    } else {
      tp = analyze(sk);
      JIT::RegionContext rContext { sk.func(), sk.offset(), liveSpOff() };
      FTRACE(2, "populating live context for region\n");
      populateLiveContext(rContext);
      region = JIT::selectRegion(rContext, tp.get());
    }

    TranslateResult result = Retry;
    RegionBlacklist regionInterps;
    Offset initSpOffset = region ? region->blocks[0]->initialSpOffset()
                                 : liveSpOff();
    while (result == Retry) {
      traceStart(sk.offset(), initSpOffset, sk.func());

      // Try translating a region if we have one, then fall back to using the
      // Tracelet.
      if (region) {
        try {
          assertCleanState();
          result = translateRegion(*region, regionInterps);
          FTRACE(2, "translateRegion finished with result {}\n",
                 translateResultName(result));
        } catch (const std::exception& e) {
          FTRACE(1, "translateRegion failed with '{}'\n", e.what());
          result = Failure;
        }
        if (result == Failure) {
          traceFree();
          traceStart(sk.offset(), liveSpOff(), sk.func());
          resetState();
        }
      }
      if (!region || result == Failure) {
        // If the region translator failed for an Optimize
        // translation, it's OK to do a Live translation for the
        // function entry.  We lazily create the tracelet here in this
        // case.
        if (m_mode == TransOptimize) {
          if (sk.getFuncId() == liveFunc()->getFuncId() &&
              liveUnit()->contains(vmpc()) &&
              sk.offset() == liveUnit()->offsetOf(vmpc())) {
            m_mode = TransLive;
            tp = analyze(sk);
          } else {
            m_mode = TransInterp;
            traceFree();
            break;
          }
        }
        FTRACE(1, "trying translateTracelet\n");
        assertCleanState();
        result = translateTracelet(*tp);

        // If we're profiling, grab the postconditions so we can
        // use them in region selection whenever we decide to
        // retranslate.
        if (m_mode == TransProfile && result == Success &&
            RuntimeOption::EvalJitPGOUsePostConditions) {
          pconds = m_irTrans->hhbcTrans().traceBuilder().getKnownTypes();
        }
      }

      if (result != Success) {
        // Translation failed. Free resources for this trace, rollback the
        // translation cache frontiers, and discard any pending fixups.
        resetState();
      }
      traceFree();
    }

    if (result == Success) {
      assert(m_mode == TransLive    ||
             m_mode == TransProfile ||
             m_mode == TransOptimize);
      transKind = m_mode;
    }
  }

  if (transKind == TransInterp) {
    assertCleanState();
    TRACE(1,
          "emitting %d-instr interp request for failed translation\n",
          int(tp->m_numOpcodes));
    if (JIT::arch() == JIT::Arch::X64) {
      Asm a { mainCode };
      // Add a counter for the translation if requested
      if (RuntimeOption::EvalJitTransCounters) {
        JIT::X64::emitTransCounterInc(a);
      }
      a.    jmp(emitServiceReq(stubsCode, JIT::REQ_INTERPRET,
                               sk.offset(), tp ? tp->m_numOpcodes : 1));
    } else if (JIT::arch() == JIT::Arch::ARM) {
      if (RuntimeOption::EvalJitTransCounters) {
        vixl::MacroAssembler a { mainCode };
        JIT::ARM::emitTransCounterInc(a);
      }
      // This jump won't be smashed, but a far jump on ARM requires the same
      // code sequence.
      JIT::emitSmashableJump(
        mainCode,
        emitServiceReq(stubsCode, JIT::REQ_INTERPRET,
                       sk.offset(), tp ? tp->m_numOpcodes : 1),
        CC_None
      );
    }
    // Fall through.
  }

  m_fixupMap.processPendingFixups();
  processPendingCatchTraces();

  addTranslation(TransRec(sk, sk.unit()->md5(), transKind, tp.get(), start,
                          mainCode.frontier() - start, stubStart,
                          stubsCode.frontier() - stubStart,
                          m_bcMap));
  m_bcMap.clear();

  recordGdbTranslation(sk, sk.func(), mainCode, start,
                       false, false);
  recordGdbTranslation(sk, sk.func(), stubsCode, stubStart,
                       false, false);
  if (RuntimeOption::EvalJitPGO) {
    if (transKind == TransProfile) {
      m_profData->addTransProfile(*tp, liveSpOff(), pconds);
    } else {
      m_profData->addTransNonProf(transKind, sk);
    }
  }
  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: (func %d, bcOff %d)\n",
        start, sk.getFuncId(), sk.offset());
  srcRec.newTranslation(start);
  TRACE(1, "tx64: %zd-byte tracelet\n", mainCode.frontier() - start);
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getUsage().c_str());
  }
}

TranslatorX64::TranslateResult
TranslatorX64::translateTracelet(Tracelet& t) {
  FTRACE(2, "attempting to translate tracelet:\n{}\n", t.toString());
  assert(!Translator::liveFrameIsPseudoMain());
  const SrcKey &sk = t.m_sk;
  SrcRec& srcRec = *getSrcRec(sk);
  HhbcTranslator& ht = m_irTrans->hhbcTrans();
  bool profilingFunc = false;

  assert(srcRec.inProgressTailJumps().size() == 0);
  try {
    emitResolvedDeps(t.m_resolvedDeps);
    {
      emitGuardChecks(sk, t.m_dependencies, t.m_refDeps, srcRec);

      dumpTranslationInfo(t, mainCode.frontier());

      // after guards, add a counter for the translation if requested
      if (RuntimeOption::EvalJitTransCounters) {
        ht.emitIncTransCounter();
      }

      if (m_mode == TransProfile) {
        if (t.func()->isEntry(sk.offset())) {
          ht.emitCheckCold(m_profData->curTransID());
          profilingFunc = true;
        } else {
          ht.emitIncProfCounter(m_profData->curTransID());
        }
      }

      m_irTrans->hhbcTrans().emitRB(RBTypeTraceletBody, t.m_sk);
      Stats::emitInc(mainCode, Stats::Instr_TC, t.m_numOpcodes);
    }

    // Profiling on function entry.
    if (t.m_sk.offset() == t.func()->base()) {
      ht.profileFunctionEntry("Normal");
    }

    /*
     * Profiling on the shapes of tracelets that are whole functions.
     * (These are the things we might consider trying to support
     * inlining.)
     */
    [&]{
      static const bool enabled = Stats::enabledAny() &&
                                  getenv("HHVM_STATS_FUNCSHAPE");
      if (!enabled) return;
      if (t.m_sk.offset() != t.func()->base()) return;
      if (auto last = t.m_instrStream.last) {
        if (last->op() != OpRetC && last->op() != OpRetV) {
          return;
        }
      }
      ht.profileSmallFunctionShape(traceletShape(t));
    }();

    Unit::MetaHandle metaHand;
    // Translate each instruction in the tracelet
    for (auto* ni = t.m_instrStream.first; ni && !ht.hasExit();
         ni = ni->next) {
      ht.setBcOff(ni->source.offset(), ni->breaksTracelet && !ht.isInlining());
      readMetaData(metaHand, *ni, m_irTrans->hhbcTrans(), MetaMode::Legacy);

      try {
        SKTRACE(1, ni->source, "HHIR: translateInstr\n");
        assert(!(m_mode == TransProfile && ni->outputPredicted && ni->next));
        m_irTrans->translateInstr(*ni);
      } catch (JIT::FailedIRGen& fcg) {
        always_assert(!ni->interp);
        ni->interp = true;
        FTRACE(1, "HHIR: RETRY Translation {}: will interpOne BC instr {} "
               "after failing to generate ir: {} \n\n",
               getCurrentTransID(), ni->toString(), fcg.what());
        return Retry;
      }
      assert(ni->source.offset() >= t.func()->base());
      // We sometimes leave the tail of a truncated tracelet in place to aid
      // analysis, but breaksTracelet is authoritative.
      if (ni->breaksTracelet || m_irTrans->hhbcTrans().hasExit()) break;
    }
    traceEnd();

    try {
      traceCodeGen();
      TRACE(1, "HHIR: SUCCEEDED to generate code for Translation %d\n\n\n",
            getCurrentTransID());
      if (profilingFunc) profData()->setProfiling(t.func()->getFuncId());
      return Success;
    } catch (JIT::FailedCodeGen& fcg) {
      // Code-gen failed. Search for the bytecode instruction that caused the
      // problem, flag it to be interpreted, and retranslate the tracelet.
      for (auto ni = t.m_instrStream.first; ni; ni = ni->next) {
        if (ni->source.offset() == fcg.bcOff) {
          always_assert(!ni->interp);
          ni->interp = true;
          FTRACE(1, "HHIR: RETRY Translation {}: will interpOne BC instr {} "
                 "after failing to code-gen \n\n",
                 getCurrentTransID(), ni->toString(), fcg.what());
          return Retry;
        }
      }
      throw fcg;
    }
  } catch (JIT::FailedCodeGen& fcg) {
    TRACE(1, "HHIR: FAILED to generate code for Translation %d "
          "@ %s:%d (%s)\n", getCurrentTransID(),
          fcg.file, fcg.line, fcg.func);
    // HHIR:TODO Remove extra TRACE and adjust tools
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          fcg.file, fcg.line, fcg.func);
  } catch (JIT::FailedIRGen& x) {
    TRACE(1, "HHIR: FAILED to translate @ %s:%d (%s)\n",
          x.file, x.line, x.func);
  } catch (const FailedAssertion& fa) {
    fa.print();
    StackTraceNoHeap::AddExtraLogging(
      "Assertion failure",
      folly::format("{}\n\nActive Trace:\n{}\n",
                    fa.summary, ht.trace()->toString()).str());
    abort();
  } catch (const JIT::FailedTraceGen& e) {
    FTRACE(1, "HHIR: FAILED to translate whole trace: {}\n",
           e.what());
  }
  return Failure;
}

void TranslatorX64::traceCodeGen() {

  HhbcTranslator& ht = m_irTrans->hhbcTrans();
  auto& unit = ht.unit();

  auto finishPass = [&](const char* msg, int level) {
    dumpTrace(level, unit, msg, nullptr, nullptr, nullptr,
              ht.traceBuilder().guards());
    assert(checkCfg(unit));
  };

  finishPass(" after initial translation ", kIRLevel);

  optimize(unit, ht.traceBuilder());
  finishPass(" after optimizing ", kOptLevel);

  auto regs = RuntimeOption::EvalHHIRXls ? allocateRegs(unit) :
              allocRegsForUnit(unit);
  assert(checkRegisters(unit, regs)); // calls checkCfg internally.

  recordBCInstr(OpTraceletGuard, mainCode, mainCode.frontier());
  genCode(mainCode, stubsCode, unit, &m_bcMap, this, regs);

  m_numHHIRTrans++;
}

TranslatorX64::TranslatorX64()
  : m_numNativeTrampolines(0)
  , m_numHHIRTrans(0)
  , m_catchTraceMap(128)
{
  static const size_t kRoundUp = 2 << 20;

  auto ru = [=] (size_t sz) { return sz + (-sz & (kRoundUp - 1)); };

  const size_t kAHotSize   = ru(RuntimeOption::RepoAuthoritative ?
                                RuntimeOption::EvalJitAHotSize : 0);
  const size_t kASize      = ru(RuntimeOption::EvalJitASize);
  const size_t kAProfSize  = ru(RuntimeOption::EvalJitPGO ?
                                RuntimeOption::EvalJitAProfSize : 0);
  const size_t kAStubsSize = ru(RuntimeOption::EvalJitAStubsSize);
  const size_t kGDataSize  = ru(RuntimeOption::EvalJitGlobalDataSize);
  m_totalSize = kAHotSize + kASize + kAStubsSize + kAProfSize + kGDataSize;

  TRACE(1, "TranslatorX64@%p startup\n", this);
  tx64 = this;

  if ((kASize < (10 << 20)) ||
      (kAStubsSize < (10 << 20)) ||
      (kGDataSize < (2 << 20))) {
    fprintf(stderr, "Allocation sizes ASize, AStubsSize, and GlobalDataSize "
                    "are too small.\n");
    exit(1);
  }

  if (m_totalSize > (2ul << 30)) {
    fprintf(stderr,"Combined size of ASize, AStubSize, and GlobalDataSize "
                   "must be < 2GiB to support 32-bit relative addresses\n");
    exit(1);
  }

  static bool profileUp = false;
  if (!profileUp) {
    profileInit();
    profileUp = true;
  }

  auto enhugen = [&](void* base, int numMB) {
    if (RuntimeOption::EvalMapTCHuge) {
      assert((uintptr_t(base) & (kRoundUp - 1)) == 0);
      hintHuge(base, numMB << 20);
    }
  };

  // We want to ensure that the block for "a", "astubs",
  // "atrampolines", and "m_globalData" are nearby so that we can
  // short jump/point between them. Thus we allocate one slab and
  // divide it between "a", "astubs", and "atrampolines".

  // Using sbrk to ensure its in the bottom 2G, so we avoid
  // the need for trampolines, and get to use shorter
  // instructions for tc addresses.
  size_t allocationSize = m_totalSize;
  uint8_t* base = (uint8_t*)sbrk(0);
  if (base != (uint8_t*)-1) {
    assert(!(allocationSize & (kRoundUp - 1)));
    // Make sure that we have space to round up to the start
    // of a huge page
    allocationSize += -(uint64_t)base & (kRoundUp - 1);
    base = (uint8_t*)sbrk(allocationSize);
  }
  if (base == (uint8_t*)-1) {
    allocationSize = m_totalSize + kRoundUp - 1;
    base = (uint8_t*)low_malloc(allocationSize);
    if (!base) {
      base = (uint8_t*)malloc(allocationSize);
    }
    if (!base) {
      fprintf(stderr, "could not allocate %zd bytes for translation cache\n",
              allocationSize);
      exit(1);
    }
  } else {
    low_malloc_skip_huge(base, base + allocationSize - 1);
  }
  assert(base);
  tcStart = base;
  base += -(uint64_t)base & (kRoundUp - 1);

  m_unwindRegistrar = register_unwind_region(base, m_totalSize - kGDataSize);
  Util::numa_interleave(base, m_totalSize);

  TRACE(1, "init atrampolines @%p\n", base);

  trampolinesCode.init(base, kTrampolinesBlockSize);

  auto misalign = kTrampolinesBlockSize;

  if (kAHotSize) {
    TRACE(1, "init ahot @%p\n", base);
    hotCode.init(base, kAHotSize);
    enhugen(base, kAHotSize >> 20);
    base += kAHotSize;
    hotCode.skip(misalign);
    misalign = 0;
  }

  TRACE(1, "init a @%p\n", base);

  mainCode.init(base, kASize);
  enhugen(base, RuntimeOption::EvalTCNumHugeHotMB);
  aStart = base;
  base += kASize;
  mainCode.skip(misalign);
  misalign = 0;

  TRACE(1, "init aprof @%p\n", base);
  profCode.init(base, kAProfSize);
  base += kAProfSize;

  TRACE(1, "init astubs @%p\n", base);
  stubsCode.init(base, kAStubsSize);
  enhugen(base, RuntimeOption::EvalTCNumHugeColdMB);
  base += kAStubsSize;

  TRACE(1, "init gdata @%p\n", base);
  m_globalData.init(base, kGDataSize);
  base += kGDataSize;

  assert(base - tcStart <= allocationSize);
  assert(base - tcStart + kRoundUp > allocationSize);
}

void TranslatorX64::initUniqueStubs() {
  // Put the following stubs into ahot, rather than a.
  CodeBlockSelector asmSel(CodeBlockSelector::Args(this).hot(true));
  if (JIT::arch() == JIT::Arch::ARM) {
    uniqueStubs = JIT::ARM::emitUniqueStubs();
  } else {
    uniqueStubs = JIT::X64::emitUniqueStubs();
  }
}

void TranslatorX64::registerCatchTrace(CTCA ip, TCA trace) {
  FTRACE(1, "registerCatchTrace: afterCall: {} trace: {}\n", ip, trace);
  m_pendingCatchTraces.emplace_back(ip, trace);
}

void TranslatorX64::processPendingCatchTraces() {
  for (auto const& pair : m_pendingCatchTraces) {
    m_catchTraceMap.insert(pair.first, pair.second);
  }
  m_pendingCatchTraces.clear();
}

TCA TranslatorX64::getCatchTrace(CTCA ip) const {
  TCA* found = m_catchTraceMap.find(ip);
  return found ? *found : nullptr;
}

void
TranslatorX64::requestInit() {
  TRACE(1, "in requestInit(%" PRId64 ")\n", g_vmContext->m_currentThreadIdx);
  tl_regState = VMRegState::CLEAN;
  PendQ::drain();
  requestResetHighLevelTranslator();
  Treadmill::startRequest(g_vmContext->m_currentThreadIdx);
  memset(&s_perfCounters, 0, sizeof(s_perfCounters));
  Stats::init();
}

void
TranslatorX64::requestExit() {
  if (s_writeLease.amOwner()) {
    s_writeLease.drop();
  }
  TRACE_MOD(txlease, 2, "%" PRIx64 " write lease stats: %15" PRId64
            " kept, %15" PRId64 " grabbed\n",
            Process::GetThreadIdForTrace(), s_writeLease.m_hintKept,
            s_writeLease.m_hintGrabbed);
  PendQ::drain();
  Treadmill::finishRequest(g_vmContext->m_currentThreadIdx);
  TRACE(1, "done requestExit(%" PRId64 ")\n", g_vmContext->m_currentThreadIdx);
  Stats::dump();
  Stats::clear();

  if (Trace::moduleEnabledRelease(Trace::tx64stats, 1)) {
    Trace::traceRelease("TranslatorX64 perf counters for %s:\n",
                        g_context->getRequestUrl(50).c_str());
    for (int i = 0; i < tpc_num_counters; i++) {
      Trace::traceRelease("%-20s %10" PRId64 "\n",
                          kPerfCounterNames[i], s_perfCounters[i]);
    }
    Trace::traceRelease("\n");
  }
}

bool
TranslatorX64::isPseudoEvent(const char* event) {
  for (auto name : kPerfCounterNames) {
    if (!strcmp(event, name)) {
      return true;
    }
  }
  return false;
}

void
TranslatorX64::getPerfCounters(Array& ret) {
  for (int i = 0; i < tpc_num_counters; i++) {
    // Until Perflab can automatically scale the values we give it to
    // an appropriate range, we have to fudge these numbers so they
    // look more like reasonable hardware counter values.
    ret.set(String::FromCStr(kPerfCounterNames[i]),
            s_perfCounters[i] * 1000);
  }

  if (RuntimeOption::EnableInstructionCounts) {
    auto doCounts = [&](unsigned begin, const char* const name) {
      int64_t count = 0;
      for (; begin < Stats::Instr_InterpOneHighInvalid;
           begin += STATS_PER_OPCODE) {
        count += Stats::tl_counters[Stats::StatCounter(begin)];
      }
      ret.set(String::FromCStr(name), count);
    };

    doCounts(Stats::Instr_TranslLowInvalid + STATS_PER_OPCODE,
             kInstrCountTx64Name);
    doCounts(Stats::Instr_TranslIRPostLowInvalid + STATS_PER_OPCODE,
             kInstrCountIRName);
  }
}

TranslatorX64::~TranslatorX64() {
  int result = munmap(trampolinesCode.base(), m_totalSize);
  if (result != 0) {
    perror("freeSlab: munmap");
  }
}

static Debug::TCRange rangeFrom(const CodeBlock& cb, const TCA addr,
                                bool isAstubs) {
  assert(cb.contains(addr));
  return Debug::TCRange(addr, cb.frontier(), isAstubs);
}

void TranslatorX64::recordBCInstr(uint32_t op,
                                  const CodeBlock& cb,
                                  const TCA addr) {
  if (addr != cb.frontier()) {
    m_debugInfo.recordBCInstr(Debug::TCRange(addr, cb.frontier(),
                                             &cb == &stubsCode), op);
  }
}

void TranslatorX64::recordGdbTranslation(SrcKey sk,
                                         const Func* srcFunc,
                                         const CodeBlock& cb,
                                         const TCA start,
                                         bool exit,
                                         bool inPrologue) {
  if (start != cb.frontier()) {
    assert(s_writeLease.amOwner());
    if (!RuntimeOption::EvalJitNoGdb) {
      m_debugInfo.recordTracelet(rangeFrom(cb, start, &cb == &stubsCode),
                                 srcFunc,
                                 srcFunc->unit() ?
                                   srcFunc->unit()->at(sk.offset()) : nullptr,
                                 exit, inPrologue);
    }
    if (RuntimeOption::EvalPerfPidMap) {
      m_debugInfo.recordPerfMap(rangeFrom(cb, start, &cb == &stubsCode),
                                srcFunc, exit, inPrologue);
    }
  }
}

void TranslatorX64::recordGdbStub(const CodeBlock& cb,
                                  const TCA start, const char* name) {
  if (!RuntimeOption::EvalJitNoGdb) {
    m_debugInfo.recordStub(rangeFrom(cb, start, &cb == &stubsCode),
                           name);
  }
}

size_t TranslatorX64::getHotCodeSize() const {
  return hotCode.base() == aStart ? mainCode.used()
                                  : hotCode.used();
}

size_t TranslatorX64::getCodeSize() const {
  return mainCode.base() == aStart ? mainCode.used()
       : hotCode.base() == aStart ? hotCode.used()
       : profCode.used();
}

size_t TranslatorX64::getProfCodeSize() const {
  return profCode.base() == aStart ? mainCode.used()
                                   : profCode.used();
}

size_t TranslatorX64::getStubSize() const {
  return stubsCode.used();
}

std::string TranslatorX64::getUsage() {
  std::string usage;
  size_t aHotUsage  = hotCode.used();
  size_t aProfUsage = profCode.used();
  size_t aUsage     = mainCode.used();
  size_t stubsUsage = stubsCode.used();
  size_t dataUsage  = m_globalData.used();

  size_t rdsUsage    = RDS::usedBytes();
  size_t persistentUsage = RDS::usedPersistentBytes();

  auto const rdsPct = 400 * rdsUsage /
    RuntimeOption::EvalJitTargetCacheSize / 3;
  auto const persistentPct = 400 * persistentUsage /
    RuntimeOption::EvalJitTargetCacheSize;

  Util::string_printf(
    usage,
    "tx64: %9zd bytes (%zd%%) in ahot.code\n"
    "tx64: %9zd bytes (%zd%%) in a.code\n"
    "tx64: %9zd bytes (%zd%%) in aprof.code\n"
    "tx64: %9zd bytes (%zd%%) in astubs.code\n"
    "tx64: %9zd bytes (%zd%%) in m_globalData\n"
    "tx64: %9zd bytes (%zd%%) in targetCache\n"
    "tx64: %9zd bytes (%zd%%) in persistentCache\n"
    "tx64: %9zd bytes (%zd%%) in RDS\n"
    "tx64: %9zd bytes (%zd%%) in persistentRDS\n",
    aHotUsage,  100 * aHotUsage / hotCode.capacity(),
    aUsage,     100 * aUsage / mainCode.capacity(),
    aProfUsage, (profCode.capacity() != 0
                 ? 100 * aProfUsage / profCode.capacity() : 0),
    stubsUsage, 100 * stubsUsage / stubsCode.capacity(),
    dataUsage,  100 * dataUsage / m_globalData.capacity(),
    rdsUsage, rdsPct,
    persistentUsage, persistentPct,
    // TODO(#2966387): temporarily double logging targetcache under new names
    rdsUsage, rdsPct,
    persistentUsage, persistentPct);
  return usage;
}

bool TranslatorX64::addDbgGuards(const Unit* unit) {
  // TODO refactor
  // It grabs the write lease and iterating through whole SrcDB...
  bool locked = s_writeLease.acquire(true);
  if (!locked) {
    return false;
  }
  struct timespec tsBegin, tsEnd;
  Timer::GetMonotonicTime(tsBegin);
  // Doc says even find _could_ invalidate iterator, in pactice it should
  // be very rare, so go with it now.
  for (SrcDB::iterator it = m_srcDB.begin(); it != m_srcDB.end(); ++it) {
    SrcKey const sk = SrcKey::fromAtomicInt(it->first);
    // We may have a SrcKey to a deleted function. NB: this may miss a
    // race with deleting a Func. See task #2826313.
    if (!Func::isFuncIdValid(sk.getFuncId())) continue;
    SrcRec* sr = it->second;
    if (sr->unitMd5() == unit->md5() &&
        !sr->hasDebuggerGuard() &&
        isSrcKeyInBL(sk)) {
      JIT::addDbgGuardImpl(sk, sr);
    }
  }
  s_writeLease.drop();
  Timer::GetMonotonicTime(tsEnd);
  int64_t elapsed = gettime_diff_us(tsBegin, tsEnd);
  if (Trace::moduleEnabledRelease(Trace::tx64, 5)) {
    Trace::traceRelease("addDbgGuards got lease for %" PRId64 " us\n", elapsed);
  }
  return true;
}

bool TranslatorX64::addDbgGuard(const Func* func, Offset offset) {
  SrcKey sk(func, offset);
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
    if (!isSrcKeyInBL(sk)) {
      TRACE(5, "calling addDbgGuard on PC that is not in blacklist");
      return false;
    }
  }
  bool locked = s_writeLease.acquire(true);
  if (!locked) {
    return false;
  }
  {
    if (SrcRec* sr = m_srcDB.find(sk)) {
      JIT::addDbgGuardImpl(sk, sr);
    }
  }
  s_writeLease.drop();
  return true;
}

bool TranslatorX64::dumpTCCode(const char* filename) {
#define OPEN_FILE(F, SUFFIX)                            \
  string F ## name = string(filename).append(SUFFIX);   \
  FILE* F = fopen(F ## name .c_str(),"wb");             \
  if (F == nullptr) return false;                       \
  SCOPE_EXIT{ fclose(F); };

  OPEN_FILE(aFile,          "_a");
  OPEN_FILE(aprofFile,      "_aprof");
  OPEN_FILE(astubFile,      "_astub");
  OPEN_FILE(helperAddrFile, "_helpers_addrs.txt");

#undef OPEN_FILE

  // dump starting from the trampolines; this assumes processInit() places
  // trampolines before the translation cache
  size_t count = mainCode.frontier() - trampolinesCode.base();
  bool result = (fwrite(trampolinesCode.base(), 1, count, aFile) == count);
  if (result) {
    count = profCode.used();
    result = (fwrite(profCode.base(), 1, count, aprofFile) == count);
  }
  if (result) {
    count = stubsCode.used();
    result = (fwrite(stubsCode.base(), 1, count, astubFile) == count);
  }
  if (result) {
    for(PointerMap::iterator iter = trampolineMap.begin();
        iter != trampolineMap.end();
        iter++) {
      void* helperAddr = iter->first;
      void* trampAddr = iter->second;
      char* functionName = getNativeFunctionName(helperAddr);
      fprintf(helperAddrFile,"%10p %10p %s\n",
              trampAddr, helperAddr,
              functionName);
      free(functionName);
    }
  }
  return result;
}

// Returns true on success
bool TranslatorX64::dumpTC(bool ignoreLease) {
  if (!ignoreLease && !s_writeLease.acquire(true)) return false;
  bool success = dumpTCData();
  if (success) {
    success = dumpTCCode("/tmp/tc_dump");
  }
  if (!ignoreLease) s_writeLease.drop();
  return success;
}

// Returns true on success
bool tc_dump(void) {
  return tx64 && tx64->dumpTC();
}

// Returns true on success
bool TranslatorX64::dumpTCData() {
  gzFile tcDataFile = gzopen("/tmp/tc_data.txt.gz", "w");
  if (!tcDataFile) return false;

  if (!gzprintf(tcDataFile,
                "repo_schema     = %s\n"
                "a.base          = %p\n"
                "a.frontier      = %p\n"
                "aprof.base      = %p\n"
                "aprof.frontier  = %p\n"
                "astubs.base     = %p\n"
                "astubs.frontier = %p\n\n",
                kRepoSchemaId,
                trampolinesCode.base(), mainCode.frontier(),
                profCode.base(), profCode.frontier(),
                stubsCode.base(), stubsCode.frontier())) {
    return false;
  }

  if (!gzprintf(tcDataFile, "total_translations = %zu\n\n",
                m_translations.size())) {
    return false;
  }

  for (size_t t = 0; t < m_translations.size(); t++) {
    if (gzputs(tcDataFile,
               m_translations[t].print(getTransCounter(t)).c_str()) == -1) {
      return false;
    }
  }

  gzclose(tcDataFile);
  return true;
}

void TranslatorX64::invalidateSrcKey(SrcKey sk) {
  assert(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  assert(s_writeLease.amOwner());
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  SrcRec* sr = m_srcDB.find(sk);
  assert(sr);
  /*
   * Since previous translations aren't reachable from here, we know we
   * just created some garbage in the TC. We currently have no mechanism
   * to reclaim this.
   */
  sr->replaceOldTranslations();
}

void TranslatorX64::setJmpTransID(TCA jmp) {
  if (m_mode != TransProfile) return;

  TransID transId = m_profData->curTransID();
  FTRACE(5, "setJmpTransID: adding {} => {}\n", jmp, transId);
  m_jmpToTransID[jmp] = transId;
}

TranslatorX64::CodeBlockSelector::CodeBlockSelector(const Args& args)
    : m_tx(args.getTranslator())
    , m_select(args.getSelection()) {

  // If an assembler other an 'a' has already been selected, then just
  // keep that selection.
  if (m_tx->mainCode.base() != m_tx->aStart) {
    m_select = CodeBlockSelection::Default;
  }

  swap();
}

/*
 * Swap 'a' with 'ahot' or 'aprof'. Note that, although we don't write to either
 * tx->ahot or tx->aprof directly, we still need to make sure that all assembler
 * code areas are available in a, astubs, aprof, and ahot, for example when we
 * call codeBlockChoose(addr, ...).
 */
void TranslatorX64::CodeBlockSelector::swap() {
  switch (m_select) {
    case CodeBlockSelection::Profile:
      std::swap(m_tx->mainCode, m_tx->profCode);
      break;
    case CodeBlockSelection::Hot:
      std::swap(m_tx->mainCode, m_tx->hotCode);
      break;
    case CodeBlockSelection::Default:
      break; // nothing to do
  }
}

TranslatorX64::CodeBlockSelector::~CodeBlockSelector() {
  swap();
}

TranslatorX64::CodeBlockSelector::Args::Args(TranslatorX64* tx)
    : m_tx(tx)
    , m_select(CodeBlockSelection::Default) {
  assert(m_tx != nullptr);
}

static const int kMaxTranslationBytes = 8192;

TranslatorX64::CodeBlockSelector::Args&
TranslatorX64::CodeBlockSelector::Args::hot(bool isHot) {
  // Profile has precedence over Hot.
  if (m_select == CodeBlockSelection::Profile) return *this;

  // Make sure there's enough room left in ahot.
  if (isHot && m_tx->hotCode.available() > kMaxTranslationBytes) {
    m_select = CodeBlockSelection::Hot;
  } else {
    m_select = CodeBlockSelection::Default;
  }
  return *this;
}

TranslatorX64::CodeBlockSelector::Args&
TranslatorX64::CodeBlockSelector::Args::profile(bool isProf) {
  if (isProf) {
    m_select = CodeBlockSelection::Profile;
  } else if (m_select == CodeBlockSelection::Profile) {
    m_select = CodeBlockSelection::Default;
  }
  return *this;
}

TranslatorX64::CodeBlockSelection
TranslatorX64::CodeBlockSelector::Args::getSelection() const {
  return m_select;
}

TranslatorX64*
TranslatorX64::CodeBlockSelector::Args::getTranslator() const {
  return m_tx;
}

} // HPHP::JIT

} // HPHP
