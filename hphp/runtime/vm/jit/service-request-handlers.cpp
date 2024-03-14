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

#include "hphp/runtime/vm/jit/service-request-handlers.h"

#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/jit-resume-addr-defs.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/workload-stats.h"

#include "hphp/vixl/a64/decoder-a64.h"

#include "hphp/util/arch.h"
#include "hphp/util/configs/debugger.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP::jit::svcreq {

namespace {

///////////////////////////////////////////////////////////////////////////////

RegionContext getContext(SrcKey sk, bool profiling) {
  RegionContext ctx { sk, liveSpOff() };

  auto const func = sk.func();
  auto const fp = vmfp();
  auto const sp = vmsp();

  always_assert(func == fp->func());
  auto const ctxClass = func->cls();
  auto const addLiveType = [&](Location loc, tv_rval tv) {
    auto const type = typeFromTV(tv, ctxClass);
    ctx.liveTypes.push_back({loc, type});

    FTRACE(2, "Added live type: {}\n", show(ctx.liveTypes.back()));
  };

  // Track local types.
  auto const numInitedLocals = sk.funcEntry()
    ? func->numFuncEntryInputs() : func->numLocals();
  for (uint32_t i = 0; i < numInitedLocals; ++i) {
    addLiveType(Location::Local{i}, frame_local(fp, i));
  }

  // Track stack types.
  int32_t stackOff = 0;
  visitStackElems(
    fp, sp,
    [&] (const TypedValue* tv) {
      addLiveType(Location::Stack{ctx.spOffset - stackOff}, tv);
      stackOff++;
    }
  );

  // Track the mbase type.  The member base register is only valid after a
  // member base op and before a member final op---and only AssertRAT*'s are
  // allowed to intervene in a sequence of bytecode member operations.
  if (!sk.funcEntry()) {
    // Get the bytecode for `ctx', skipping Asserts.
    auto const op = [&] {
      auto pc = func->at(sk.offset());
      while (isTypeAssert(peek_op(pc))) {
        pc += instrLen(pc);
      }
      return peek_op(pc);
    }();
    assertx(!isTypeAssert(op));

    if (isMemberDimOp(op) || isMemberFinalOp(op)) {
      auto const mbase = vmMInstrState().base;
      assertx(mbase != nullptr);
      addLiveType(Location::MBase{}, mbase);
    }
  }

  return ctx;
}

/*
 * Create a translation for the SrcKey specified in args.
 *
 * If a translation for this SrcKey already exists it will be returned. The kind
 * of translation created will be selected based on the SrcKey specified.
 */
TranslationResult getTranslation(SrcKey sk) {
  sk.func()->validate();

  if (!RID().getJit()) {
    SKTRACE(2, sk, "punting because jit was disabled\n");
    return TranslationResult::failTransiently();
  }

  if (auto const sr = tc::findSrcRec(sk)) {
    if (auto const tca = sr->getTopTranslation()) {
      SKTRACE(2, sk, "getTranslation: found %p\n", tca);
      return TranslationResult{tca};
    }
  }

  if (UNLIKELY(RID().isJittingDisabled())) {
    SKTRACE(2, sk, "punting because jitting code was disabled\n");
    return TranslationResult::failTransiently();
  }

  if (UNLIKELY(Cfg::Debugger::EnableVSDebugger && RO::EvalEmitDebuggerIntrCheck)) {
    assertx(!RO::RepoAuthoritative);
    sk.func()->ensureDebuggerIntrSetLinkBound();
  }

  if (UNLIKELY(!RO::RepoAuthoritative && sk.unit()->isCoverageEnabled())) {
    assertx(RO::EvalEnablePerFileCoverage);
    SKTRACE(2, sk, "punting because per file code coverage is enabled\n");
    return TranslationResult::failTransiently();
  }

  auto args = TransArgs{sk};
  args.kind = tc::profileFunc(args.sk.func()) ?
    TransKind::Profile : TransKind::Live;

  if (auto const s = tc::shouldTranslate(args.sk, args.kind);
      s != TranslationResult::Scope::Success) {
    return TranslationResult{s};
  }

  LeaseHolder writer(sk.func(), args.kind);
  if (!writer) return TranslationResult::failTransiently();

  if (auto const s = tc::shouldTranslate(args.sk, args.kind);
      s != TranslationResult::Scope::Success) {
    return TranslationResult{s};
  }

  if (tc::createSrcRec(sk, liveSpOff()) == nullptr) {
    // ran out of TC space
    return TranslationResult::failForProcess();
  }

  if (auto const tca = tc::findSrcRec(sk)->getTopTranslation()) {
    // Handle extremely unlikely race; someone may have just added the first
    // translation for this SrcRec while we did a non-blocking wait on the
    // write lease in createSrcRec().
    return TranslationResult{tca};
  }

  return mcgen::retranslate(
    args,
    getContext(args.sk, args.kind == TransKind::Profile)
  );
}

///////////////////////////////////////////////////////////////////////////////

}

JitResumeAddr getFuncEntry(const Func* func) {
  if (auto const addr = func->getFuncEntry()) {
    return JitResumeAddr::transFuncEntry(addr);
  }

  LeaseHolder writer(func, TransKind::Profile);
  if (!writer) {
    return JitResumeAddr::helper(tc::ustubs().resumeHelperFuncEntryFromInterp);
  }

  if (auto const addr = func->getFuncEntry()) {
    return JitResumeAddr::transFuncEntry(addr);
  }

  auto const numParams = func->numNonVariadicParams();
  if (func->numRequiredParams() != numParams) {
    const_cast<Func*>(func)
      ->setFuncEntry(tc::ustubs().resumeHelperFuncEntryFromTC);
    return JitResumeAddr::helper(tc::ustubs().resumeHelperFuncEntryFromInterp);
  }

  SrcKey sk{func, numParams, SrcKey::FuncEntryTag{}};
  auto const trans = getTranslation(sk);

  if (auto const addr = trans.addr()) {
    const_cast<Func*>(func)->setFuncEntry(addr);
    return JitResumeAddr::transFuncEntry(addr);
  }

  if (trans.isProcessPersistentFailure()) {
    const_cast<Func*>(func)
      ->setFuncEntry(tc::ustubs().interpHelperNoTranslateFuncEntryFromTC);
    // implies request persistent failure below
  }

  if (trans.isRequestPersistentFailure()) {
    return JitResumeAddr::helper(
      tc::ustubs().interpHelperNoTranslateFuncEntryFromInterp);
  }

  return JitResumeAddr::helper(tc::ustubs().resumeHelperFuncEntryFromInterp);
}

namespace {

TCA resume(SrcKey sk, TranslationResult transResult) noexcept {
  auto const start = [&] {
    if (auto const addr = transResult.addr()) return addr;
    if (sk.funcEntry()) {
      sk.advance();
      vmpc() = sk.pc();
      return transResult.scope() == TranslationResult::Scope::Transient
        ? tc::ustubs().interpHelperFuncEntryFromTC
        : tc::ustubs().interpHelperNoTranslateFuncEntryFromTC;
    }
    vmpc() = sk.pc();
    return transResult.scope() == TranslationResult::Scope::Transient
      ? tc::ustubs().interpHelperFromTC
      : tc::ustubs().interpHelperNoTranslateFromTC;
  }();

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = sk.valid() ? sk.toAtomicInt() : uint64_t(-1LL);
    Trace::ringbufferEntry(Trace::RBTypeResumeTC, skData, (uint64_t)start);
  }

  regState() = VMRegState::DIRTY;
  return start;
}

void syncRegs(SBInvOffset spOff) noexcept {
  assert_native_stack_aligned();

  // This is a lie, only vmfp() is synced. We will sync vmsp() below and vmpc()
  // later if we are going to use the resume helper.
  regState() = VMRegState::CLEAN;
  vmsp() = Stack::anyFrameStackBase(vmfp()) - spOff.offset;
  vmJitReturnAddr() = nullptr;
}

}

void uninitDefaultArgs(ActRec* fp, uint32_t numEntryArgs,
                       uint32_t numNonVariadicParams) noexcept {
  // JIT may optimize away uninit writes for default arguments. Write them, as
  // we may inspect them or continue execution in the interpreter.
  for (auto param = numEntryArgs; param < numNonVariadicParams; ++param) {
    tvWriteUninit(frame_local(fp, param));
  }
}

TCA handleTranslate(Offset bcOff, SBInvOffset spOff) noexcept {
  syncRegs(spOff);
  FTRACE(1, "handleTranslate {}\n", vmfp()->func()->fullName()->data());

  auto const sk = SrcKey { liveFunc(), bcOff, liveResumeMode() };
  return resume(sk, getTranslation(sk));
}

TCA handleTranslateFuncEntry(uint32_t numArgs) noexcept {
  syncRegs(SBInvOffset{0});
  uninitDefaultArgs(vmfp(), numArgs, liveFunc()->numNonVariadicParams());
  FTRACE(1, "handleTranslateFuncEntry {}\n",
         vmfp()->func()->fullName()->data());

  auto const sk = SrcKey { liveFunc(), numArgs, SrcKey::FuncEntryTag {} };
  return resume(sk, getTranslation(sk));
}

TCA handleRetranslate(Offset bcOff, SBInvOffset spOff) noexcept {
  syncRegs(spOff);
  FTRACE(1, "handleRetranslate {}\n", vmfp()->func()->fullName()->data());

  INC_TPC(retranslate);
  auto const sk = SrcKey { liveFunc(), bcOff, liveResumeMode() };
  auto const context = getContext(sk, tc::profileFunc(sk.func()));
  auto const transResult = mcgen::retranslate(TransArgs{sk}, context);
  SKTRACE(2, sk, "retranslated @%p\n", transResult.addr());
  return resume(sk, transResult);
}

TCA handleRetranslateFuncEntry(uint32_t numArgs) noexcept {
  syncRegs(SBInvOffset{0});
  uninitDefaultArgs(vmfp(), numArgs, liveFunc()->numNonVariadicParams());
  FTRACE(1, "handleRetranslateFuncEntry {}\n",
         vmfp()->func()->fullName()->data());

  INC_TPC(retranslate);
  auto const sk = SrcKey { liveFunc(), numArgs, SrcKey::FuncEntryTag {} };
  auto const context = getContext(sk, tc::profileFunc(sk.func()));
  auto const transResult = mcgen::retranslate(TransArgs{sk}, context);
  SKTRACE(2, sk, "retranslated @%p\n", transResult.addr());
  return resume(sk, transResult);
}

TCA handleRetranslateOpt(uint32_t numArgs) noexcept {
  syncRegs(SBInvOffset{0});
  uninitDefaultArgs(vmfp(), numArgs, liveFunc()->numNonVariadicParams());
  FTRACE(1, "handleRetranslateOpt {}\n", vmfp()->func()->fullName()->data());

  auto const sk = SrcKey { liveFunc(), numArgs, SrcKey::FuncEntryTag {} };
  auto const translated = mcgen::retranslateOpt(sk.funcID());
  vmpc() = sk.advanced().pc();
  regState() = VMRegState::DIRTY;

  if (translated) {
    // Retranslation was successful. Resume execution at the new optimized
    // translation.
    return tc::ustubs().resumeHelperFuncEntryFromTC;
  } else {
    // Retranslation failed, probably because we couldn't get the write
    // lease. Interpret a BB before running more Profile translations, to
    // avoid spinning through this path repeatedly.
    return tc::ustubs().interpHelperFuncEntryFromTC;
  }
}

TCA handlePostInterpRet(uint32_t callOffAndFlags) noexcept {
  assert_native_stack_aligned();
  regState() = VMRegState::CLEAN; // partially a lie: vmpc() isn't synced

  auto const callOffset = callOffAndFlags >> ActRec::CallOffsetStart;
  auto const isAER = callOffAndFlags & (1 << ActRec::AsyncEagerRet);

  FTRACE(3, "handlePostInterpRet to {}@{}{}\n",
         vmfp()->func()->fullName()->data(), callOffset, isAER ? "a" : "");

  // Reconstruct PC so that the subsequent logic have clean reg state.
  vmpc() = skipCall(vmfp()->func()->at(callOffset));
  vmJitReturnAddr() = nullptr;

  if (isAER) {
    // When returning to the interpreted FCall, the execution continues at
    // the next opcode, not honoring the request for async eager return.
    // If the callee returned eagerly, we need to wrap the result into
    // StaticWaitHandle.
    assertx(tvIsPlausible(vmsp()[0]));
    assertx(vmsp()[0].m_aux.u_asyncEagerReturnFlag + 1 < 2);
    if (vmsp()[0].m_aux.u_asyncEagerReturnFlag) {
      auto const waitHandle = c_StaticWaitHandle::CreateSucceeded(vmsp()[0]);
      vmsp()[0] = make_tv<KindOfObject>(waitHandle);
    }
  }

  auto const sk = liveSK();
  return resume(sk, getTranslation(sk));
}

TCA handleBindCall(TCA toSmash, Func* func, int32_t numArgs) {
  TRACE(2, "bindCall %s, numArgs %d\n", func->fullName()->data(), numArgs);
  auto const trans = mcgen::getFuncPrologue(func, numArgs);
  TRACE(2, "bindCall immutably %s -> %p\n",
        func->fullName()->data(), trans.addr());

  if (trans.isProcessPersistentFailure()) {
    // We can't get a translation for this and we can't create any new
    // ones any longer. Smash the call site with a stub which will
    // interp the prologue, then run resumeHelperNoTranslateFromInterp.
    tc::bindCall(
      toSmash,
      tc::ustubs().fcallHelperNoTranslateThunk,
      func,
      numArgs
    );
    return tc::ustubs().fcallHelperNoTranslateThunk;
  } else if (trans.addr()) {
    // Using start is racy but bindCall will recheck the start address after
    // acquiring a lock on the ProfTransRec
    tc::bindCall(toSmash, trans.addr(), func, numArgs);
    return trans.addr();
  } else {
    // We couldn't get a prologue address. Return a stub that will finish
    // entering the callee frame in C++, then call handleResume at the callee's
    // entry point.
    return trans.isRequestPersistentFailure()
      ? tc::ustubs().fcallHelperNoTranslateThunk
      : tc::ustubs().fcallHelperThunk;
  }
}

std::string ResumeFlags::show() const {
  std::vector<std::string> flags;
  if (m_noTranslate) flags.emplace_back("noTranslate");
  if (m_interpFirst) flags.emplace_back("interpFirst");
  if (m_funcEntry) flags.emplace_back("funcEntry");
  return folly::join(", ", flags);
}

JitResumeAddr handleResume(ResumeFlags flags) {
  assert_native_stack_aligned();
  FTRACE(1, "handleResume({})\n", flags.show());

  regState() = VMRegState::CLEAN;
  assertx(vmpc());

  auto sk = liveSK();
  if (flags.m_funcEntry) {
    assertx(sk.resumeMode() == ResumeMode::None);
    auto const func = sk.func();
    auto numArgs = func->numNonVariadicParams();
    while (numArgs > 0 && !frame_local(vmfp(), numArgs - 1)->is_init()) {
      --numArgs;
    }
    DEBUG_ONLY auto const entryOffset = sk.offset();
    sk = SrcKey { sk.func(), numArgs, SrcKey::FuncEntryTag {} };
    assertx(sk.entryOffset() == entryOffset);

    vmsp() = Stack::frameStackBase(vmfp());
  }
  FTRACE(2, "handleResume: sk: {}\n", showShort(sk));

  auto const findOrTranslate = [&] () -> JitResumeAddr {
    if (!flags.m_noTranslate) {
      auto const trans = getTranslation(sk);
      if (auto const addr = trans.addr()) {
        if (sk.funcEntry()) return JitResumeAddr::transFuncEntry(addr);
        return JitResumeAddr::trans(addr);
      }
      if (!trans.isRequestPersistentFailure()) return JitResumeAddr::none();
      return JitResumeAddr::helper(
        sk.funcEntry()
          ? tc::ustubs().interpHelperNoTranslateFuncEntryFromInterp
          : tc::ustubs().interpHelperNoTranslateFromInterp
      );
    }

    if (auto const sr = tc::findSrcRec(sk)) {
      if (auto const tca = sr->getTopTranslation()) {
        if (LIKELY(RID().getJit())) {
          SKTRACE(2, sk, "handleResume: found %p\n", tca);
          if (sk.funcEntry()) return JitResumeAddr::transFuncEntry(tca);
          return JitResumeAddr::trans(tca);
        }
      }
    }
    return JitResumeAddr::none();
  };

  auto start = JitResumeAddr::none();
  if (!flags.m_interpFirst) start = findOrTranslate();
  if (!flags.m_noTranslate && flags.m_interpFirst) INC_TPC(interp_bb_force);

  vmJitReturnAddr() = nullptr;
  vmJitCalledFrame() = vmfp();
  SCOPE_EXIT { vmJitCalledFrame() = nullptr; };

  // If we can't get a translation at the current SrcKey, interpret basic
  // blocks until we end up somewhere with a translation (which we may have
  // created, if the lease holder dropped it).
  if (!start) {
    WorkloadStats guard(WorkloadStats::InInterp);
    tracing::BlockNoTrace _{"dispatch-bb"};

    if (sk.funcEntry()) {
      auto const savedRip = vmfp()->m_savedRip;
      if (!funcEntry()) {
        FTRACE(2, "handleResume: returning to rip={} pc={} after intercept\n",
               TCA(savedRip), vmpc());
        // The callee was intercepted and should be skipped. In that case,
        // return to the caller. If we entered this frame from the interpreter,
        // use the resumeHelper, as the retHelper logic has been already
        // performed and the frame has been overwritten by the return value.
        regState() = VMRegState::DIRTY;
        if (isReturnHelper(savedRip)) {
          return
            JitResumeAddr::helper(jit::tc::ustubs().resumeHelperFromInterp);
        }
        return JitResumeAddr::ret(TCA(savedRip));
      }
      sk.advance();
    }

    do {
      INC_TPC(interp_bb);
      if (auto const retAddr = HPHP::dispatchBB()) {
        start = retAddr;
      } else {
        assertx(vmpc());
        sk = liveSK();
        start = findOrTranslate();
      }
    } while (!start);
  }

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = sk.valid() ? sk.toAtomicInt() : uint64_t(-1LL);
    Trace::ringbufferEntry(Trace::RBTypeResumeTC, skData,
                           (uint64_t)start.handler);
  }

  regState() = VMRegState::DIRTY;
  return start;
}

///////////////////////////////////////////////////////////////////////////////

}
