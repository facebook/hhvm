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

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/debug-guards.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/recycle-tc.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc-info.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"

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

///////////////////////////////////////////////////////////////////////////////

bool dumpTCAnnotation(const Func& func, TransKind transKind) {
  return RuntimeOption::EvalDumpTCAnnotationsForAllTrans ||
    (transKind == TransKind::Optimize && (func.attrs() & AttrHot));
}

int MCGenerator::numTranslations(SrcKey sk) const {
  if (const SrcRec* sr = m_srcDB.find(sk)) {
    return sr->translations().size();
  }
  return 0;
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
  auto tDest = getTranslation(args);
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
      start = retranslate(args);
      SKTRACE(2, sk, "retranslated @%p\n", start);
      break;
    }

    case REQ_RETRANSLATE_OPT: {
      sk = SrcKey::fromAtomicInt(info.args[0].sk);
      auto transID = info.args[1].transID;
      start = retranslateOpt(sk, transID);
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
      start = getTranslation(TransArgs{sk});
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
      start = getTranslation(TransArgs{sk});
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
    start = getTranslation(TransArgs(sk));
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
    start = getTranslation(TransArgs{sk});
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

MCGenerator::MCGenerator()
  : m_catchTraceMap(128)
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

///////////////////////////////////////////////////////////////////////////////

}}
