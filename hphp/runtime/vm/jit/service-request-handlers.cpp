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

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/workload-stats.h"

#include "hphp/ppc64-asm/decoded-instr-ppc64.h"
#include "hphp/vixl/a64/decoder-a64.h"

#include "hphp/util/arch.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace svcreq {

namespace {

///////////////////////////////////////////////////////////////////////////////

RegionContext getContext(SrcKey sk) {
  RegionContext ctx {
    sk.func(), sk.offset(), liveSpOff(),
    sk.resumed(), sk.hasThis()
  };

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
        !ar->func()->cls() ? TNullptr :
        (ar->hasThis()  ?
         Type::SubObj(ar->getThis()->getVMClass()) :
         Type::SubCls(ar->getClass()));

      ctx.preLiveARs.push_back({ stackOff, ar->func(), objOrCls });
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

  // Get the bytecode for `ctx', skipping Asserts.
  auto const op = [&] {
    auto pc = ctx.func->unit()->at(ctx.bcOffset);
    while (isTypeAssert(peek_op(pc))) {
      pc += instrLen(pc);
    }
    return peek_op(pc);
  }();
  assertx(!isTypeAssert(op));

  // Track the mbase type.  The member base register is only valid after a
  // member base op and before a member final op---and only AssertRAT*'s are
  // allowed to intervene in a sequence of bytecode member operations.
  if (isMemberDimOp(op) || isMemberFinalOp(op)) {
    auto const mbase = vmMInstrState().base;
    assertx(mbase != nullptr);

    ctx.liveTypes.push_back({ Location::MBase{}, typeFromTV(mbase, ctxClass) });
    FTRACE(2, "added live type {}\n", show(ctx.liveTypes.back()));
  }

  return ctx;
}

bool liveFrameIsPseudoMain() {
  auto const ar = vmfp();
  if (!(ar->func()->attrs() & AttrMayUseVV)) return false;
  return ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope();
}

/*
 * Create a translation for the SrcKey specified in args.
 *
 * If a translation for this SrcKey already exists it will be returned. The kind
 * of translation created will be selected based on the SrcKey specified.
 */
TCA getTranslation(TransArgs args) {
  auto sk = args.sk;
  sk.func()->validate();

  if (liveFrameIsPseudoMain() && !RuntimeOption::EvalJitPseudomain) {
    SKTRACE(2, sk, "punting on pseudoMain\n");
    return nullptr;
  }

  if (!RID().getJit()) {
    SKTRACE(2, sk, "punting because jitting was disabled\n");
    return nullptr;
  }

  if (auto const sr = tc::findSrcRec(sk)) {
    if (auto const tca = sr->getTopTranslation()) {
      SKTRACE(2, sk, "getTranslation: found %p\n", tca);
      return tca;
    }
  }

  args.kind = tc::profileFunc(args.sk.func()) ?
    TransKind::Profile : TransKind::Live;

  if (!tc::shouldTranslate(args.sk.func(), args.kind)) return nullptr;

  LeaseHolder writer(sk.func(), args.kind);
  if (!writer || !tc::shouldTranslate(sk.func(), args.kind)) return nullptr;

  if (RuntimeOption::EvalFailJitPrologs && sk.op() == Op::FCallAwait) {
    return nullptr;
  }

  tc::createSrcRec(sk, liveSpOff());

  auto sr = tc::findSrcRec(sk);
  always_assert(sr);

  if (auto const tca = sr->getTopTranslation()) {
    // Handle extremely unlikely race; someone may have just added the first
    // translation for this SrcRec while we did a non-blocking wait on the
    // write lease in createSrcRec().
    return tca;
  }

  return mcgen::retranslate(args, getContext(args.sk));
}

TCA getFuncBody(Func* func) {
  auto tca = func->getFuncBody();
  if (tca != tc::ustubs().funcBodyHelperThunk) return tca;

  LeaseHolder writer(func, TransKind::Profile);
  if (!writer) return nullptr;

  tca = func->getFuncBody();
  if (tca != tc::ustubs().funcBodyHelperThunk) return tca;

  auto const dvs = func->getDVFunclets();
  if (dvs.size() || func->hasThisVaries()) {
    auto const kind = tc::profileFunc(func) ? TransKind::Profile
                                            : TransKind::Live;
    tca = tc::emitFuncBodyDispatch(func, dvs, kind);
  } else {
    SrcKey sk(func, func->base(), false, func->mayHaveThis());
    tca = getTranslation(TransArgs{sk});
    if (tca) func->setFuncBody(tca);
  }

  return tca;
}

/*
 * Runtime service handler that patches a jmp to the translation of u:dest from
 * toSmash.
 */
TCA bindJmp(TCA toSmash, SrcKey destSk, ServiceRequest req, TransFlags trflags,
            bool& smashed) {
  auto args = TransArgs{destSk};
  args.flags = trflags;
  auto tDest = getTranslation(args);
  if (!tDest) return nullptr;

  if (req == REQ_BIND_ADDR) {
    return tc::bindAddr(toSmash, destSk, trflags, smashed);
  }

  return tc::bindJmp(toSmash, destSk, trflags, smashed);
}

void syncFuncBodyVMRegs(ActRec* fp, void* sp) {
  auto& regs = vmRegsUnsafe();
  regs.fp = fp;
  regs.stack.top() = (Cell*)sp;

  auto const nargs = fp->numArgs();
  auto const nparams = fp->func()->numNonVariadicParams();
  auto const& paramInfo = fp->func()->params();

  auto firstDVI = InvalidAbsoluteOffset;

  for (auto i = nargs; i < nparams; ++i) {
    auto const dvi = paramInfo[i].funcletOff;
    if (dvi != InvalidAbsoluteOffset) {
      firstDVI = dvi;
      break;
    }
  }
  if (firstDVI != InvalidAbsoluteOffset) {
    regs.pc = fp->m_func->unit()->entry() + firstDVI;
  } else {
    regs.pc = fp->m_func->getEntry();
  }
}

///////////////////////////////////////////////////////////////////////////////

}

TCA funcBodyHelper(ActRec* fp) {
  assert_native_stack_aligned();
  void* const sp = reinterpret_cast<Cell*>(fp) - fp->func()->numSlotsInFrame();
  syncFuncBodyVMRegs(fp, sp);
  tl_regState = VMRegState::CLEAN;

  auto const func = const_cast<Func*>(fp->m_func);
  auto tca = getFuncBody(func);
  if (!tca) {
    tca = tc::ustubs().resumeHelper;
  }

  tl_regState = VMRegState::DIRTY;
  return tca;
}

TCA handleServiceRequest(ReqInfo& info) noexcept {
  FTRACE(1, "handleServiceRequest {}\n", svcreq::to_name(info.req));

  assert_native_stack_aligned();
  tl_regState = VMRegState::CLEAN; // partially a lie: vmpc() isn't synced

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    Trace::ringbufferEntry(
      Trace::RBTypeServiceReq, (uint64_t)info.req, (uint64_t)info.args[0].tca
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
      sk = SrcKey{
        liveFunc(), info.args[0].offset, liveResumed(), liveHasThis()
      };
      auto trflags = info.args[1].trflags;
      auto args = TransArgs{sk};
      args.flags = trflags;
      start = mcgen::retranslate(args, getContext(args.sk));
      SKTRACE(2, sk, "retranslated @%p\n", start);
      break;
    }

    case REQ_RETRANSLATE_OPT: {
      sk = SrcKey::fromAtomicInt(info.args[0].sk);
      if (mcgen::retranslateOpt(sk.funcID())) {
        // Retranslation was successful. Resume execution at the new Optimize
        // translation.
        vmpc() = sk.unit()->at(sk.offset());
        start = tc::ustubs().resumeHelper;
      } else {
        // Retranslation failed, probably because we couldn't get the write
        // lease. Interpret a BB before running more Profile translations, to
        // avoid spinning through this path repeatedly.
        start = nullptr;
      }
      break;
    }

    case REQ_POST_INTERP_RET: {
      // This is only responsible for the control-flow aspect of the Ret:
      // getting to the destination's translation, if any.
      auto ar = info.args[0].ar;
      auto const caller = info.args[1].ar;
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
        assert(ar->retSlot()->m_aux.u_fcallAwaitFlag < 2);
        if (ar->retSlot()->m_aux.u_fcallAwaitFlag) {
          start = tc::ustubs().fcallAwaitSuspendHelper;
          break;
        }
      }
      assertx(caller == vmfp());
      sk = liveSK();
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
      sk = liveSK();
      start = getTranslation(TransArgs{sk});
      break;
    }
  }

  if (smashed && info.stub) {
    auto const stub = info.stub;
    FTRACE(3, "Freeing stub {} on treadmill\n", stub);
    Treadmill::enqueue([stub] { tc::freeTCStub(stub); });
  }

  if (start == nullptr) {
    vmpc() = sk.unit()->at(sk.offset());
    start = tc::ustubs().interpHelperSyncedPC;
  }

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = sk.valid() ? sk.toAtomicInt() : uint64_t(-1LL);
    Trace::ringbufferEntry(Trace::RBTypeResumeTC, skData, (uint64_t)start);
  }

  tl_regState = VMRegState::DIRTY;
  return start;
}

TCA handleBindCall(TCA toSmash, ActRec* calleeFrame, bool isImmutable) {
  Func* func = const_cast<Func*>(calleeFrame->m_func);
  int nArgs = calleeFrame->numArgs();
  TRACE(2, "bindCall %s, ActRec %p\n", func->fullName()->data(), calleeFrame);
  TCA start = mcgen::getFuncPrologue(func, nArgs);
  TRACE(2, "bindCall -> %p\n", start);
  if (start && !isImmutable) {
    // We dont know we're calling the right function, so adjust start to point
    // to the dynamic check of ar->m_func.
    start = funcGuardFromPrologue(start, func);
  } else {
    TRACE(2, "bindCall immutably %s -> %p\n", func->fullName()->data(), start);
  }

  if (start && !RuntimeOption::EvalFailJitPrologs) {
    // Using start is racy but bindCall will recheck the start address after
    // acquiring a lock on the ProfTransRec
    tc::bindCall(toSmash, start, func, nArgs, isImmutable);
  } else {
    // We couldn't get a prologue address. Return a stub that will finish
    // entering the callee frame in C++, then call handleResume at the callee's
    // entry point.
    start = tc::ustubs().fcallHelperThunk;
  }

  return start;
}

TCA handleFCallAwaitSuspend() {
  assert_native_stack_aligned();
  FTRACE(1, "handleFCallAwaitSuspend\n");

  tl_regState = VMRegState::CLEAN;

  vmJitCalledFrame() = vmfp();
  SCOPE_EXIT { vmJitCalledFrame() = nullptr; };

  auto start = suspendStack(vmpc());
  tl_regState = VMRegState::DIRTY;
  return start ? start : tc::ustubs().resumeHelper;
}

TCA handleResume(bool interpFirst) {
  assert_native_stack_aligned();
  FTRACE(1, "handleResume({})\n", interpFirst);

  if (!vmRegsUnsafe().pc) return tc::ustubs().callToExit;

  tl_regState = VMRegState::CLEAN;

  auto sk = liveSK();
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
  if (!start) {
    WorkloadStats guard(WorkloadStats::InInterp);

    while (!start) {
      INC_TPC(interp_bb);
      if (auto retAddr = HPHP::dispatchBB()) {
        start = retAddr;
        break;
      }

      assertx(vmpc());
      sk = liveSK();
      start = getTranslation(TransArgs{sk});
    }
  }

  if (Trace::moduleEnabled(Trace::ringbuffer, 1)) {
    auto skData = sk.valid() ? sk.toAtomicInt() : uint64_t(-1LL);
    Trace::ringbufferEntry(Trace::RBTypeResumeTC, skData, (uint64_t)start);
  }

  tl_regState = VMRegState::DIRTY;
  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}}
