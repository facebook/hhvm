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

#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/surprise-flags.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void alignJmpTarget(CodeBlock& cb) {
  if (RuntimeOption::EvalJitAlignUniqueStubs) {
    align(cb, nullptr, Alignment::JmpTarget, AlignContext::Dead);
  }
}

void alignCacheLine(CodeBlock& cb) {
  if (RuntimeOption::EvalJitAlignUniqueStubs) {
    align(cb, nullptr, Alignment::CacheLine, AlignContext::Dead);
  }
}

///////////////////////////////////////////////////////////////////////////////

using AFWH = c_AsyncFunctionWaitHandle;

/*
 * Convert an AsyncFunctionWaitHandle-relative offset to an offset relative to
 * either its contained ActRec or AsioBlockable.
 */
constexpr ptrdiff_t afwhToAr(ptrdiff_t off) {
  return off - AFWH::arOff();
}
constexpr ptrdiff_t afwhToBl(ptrdiff_t off) {
  return off - AFWH::childrenOff() - AFWH::Node::blockableOff();
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Try to pop a fast resumable off the current AsioContext's queue.  If there
 * is none (or if surprise flags are set), return nullptr.
 */
c_AsyncFunctionWaitHandle* getFastRunnableAFWH() {
  if (checkSurpriseFlags()) return nullptr;
  auto const ctx = AsioSession::Get()->getCurrentContext();

  auto const afwh = ctx->maybePopFast();
  assertx(!afwh || afwh->isFastResumable());
  return afwh;
}

/*
 * Free memory used by the AsyncFunctionWaitHandle.
 */
void freeAFWH(c_AsyncFunctionWaitHandle* wh) {
  auto const size = wh->resumable()->size();
  auto const base = reinterpret_cast<char*>(wh + 1) - size;
  tl_heap->objFree(base, size);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Store the async function's return value to the AsyncFunctionWaitHandle and
 * mark it as succeeded.
 *
 * Note that this overwrites parentChain and contextIdx, so these fields need
 * to be loaded before the result is stored.
 */
void storeAFWHResult(Vout& v, PhysReg data, PhysReg type) {
  auto const resultOff = afwhToAr(AFWH::resultOff());
  v << store{data, rvmfp()[resultOff + TVOFF(m_data)]};
  // This store must preserve the other bits in the WaitHandle for correctness.
  v << storeb{type, rvmfp()[resultOff + TVOFF(m_type)]};

  // Set state to succeeded.
  v << storebi{
    c_Awaitable::toKindState(
      c_Awaitable::Kind::AsyncFunction,
      c_Awaitable::STATE_SUCCEEDED
    ),
    rvmfp()[afwhToAr(c_Awaitable::stateOff())]
  };
}

/*
 * Unblock the chain of blockables. Calls into native code if the pointer to
 * the first blockable is non-null.
 */
void unblockParents(Vout& v, Vreg firstBl) {
  auto const sf = v.makeReg();
  v << testq{firstBl, firstBl, sf};

  ifThen(v, CC_NZ, sf, [&] (Vout& v) {
    v << vcall{CallSpec::direct(AsioBlockableChain::UnblockJitHelper),
               v.makeVcallArgs({{rvmfp(), rvmsp(), firstBl}}),
               v.makeTuple({}),
               Fixup::none()};
  });
}

TCA emitAsyncSwitchCtrl(CodeBlock& cb, DataBlock& data, TCA* inner) {
  alignCacheLine(cb);

  auto const ret = vwrap(cb, data, [] (Vout& v) {
    // Set rvmfp() to the suspending WaitHandle's parent frame.
    v << load{rvmfp()[AROFF(m_sfp)], rvmfp()};
  });

  *inner = vwrap(cb, data, [] (Vout& v) {
    auto const slow_path = Vlabel(v.makeBlock());

    auto const afwh = v.makeReg();
    v << vcall{
      CallSpec::direct(getFastRunnableAFWH),
      v.makeVcallArgs({{}}),
      v.makeTuple({afwh}),
      Fixup::none(),
      DestType::SSA
    };

    auto const isNull = v.makeReg();
    v << testq{afwh, afwh, isNull};
    ifThen(v, CC_Z, isNull, slow_path);

    // Transfer our frame linkage into the AFWH that we're resuming.
    v << store{rvmfp(), afwh[AFWH::arOff() + AROFF(m_sfp)]};

    // Set the AFHW's state to RUNNING.
    v << storebi{
      AFWH::toKindState(
        c_Awaitable::Kind::AsyncFunction,
        AFWH::STATE_RUNNING
      ),
      afwh[AFWH::stateOff()]
    };

    if (RO::EvalEnableImplicitContext) {
      markRDSAccess(v, ImplicitContext::activeCtx.handle());
      auto const implicitContext = v.makeReg();
      v << load {
        afwh[c_ResumableWaitHandle::implicitContextOff()],
        implicitContext
      };
      v << store{implicitContext, rvmtl()[ImplicitContext::activeCtx.handle()]};
    }

    auto const child = v.makeReg();
    v << load{afwh[AFWH::childrenOff() + AFWH::Node::childOff()], child};

    // Copy the child's result onto the stack and incref it.
    auto const data = v.makeReg();
    auto const type = v.makeReg();
    auto const resultOff = c_WaitableWaitHandle::resultOff();
    v << load {child[resultOff + TVOFF(m_data)], data};
    v << loadb{child[resultOff + TVOFF(m_type)], type};
    v << store {data, rvmsp()[TVOFF(m_data)]};
    v << storeb{type, rvmsp()[TVOFF(m_type)]};

    emitIncRefWork(v, data, type, TRAP_REASON);

    // Now decref `child', which may free it---but note that the WaitHandle's
    // destructor has no risk of reentry.
    emitDecRefWorkObj(v, child, TRAP_REASON);

    // Load the address of the ActRec for our AFWH into rvmfp(), and sync it to
    // vmFirstAR().  We don't need to sync any of the vmRegs(), since we're
    // jumping straight into the async function.
    v << lea{afwh[AFWH::arOff()], rvmfp()};
    v << store{rvmfp(), rvmtl()[rds::kVmFirstAROff]};

    // Jump to the AFWH's resume address.
    v << jmpm{afwh[AFWH::resumeAddrOff()], vm_regs_with_sp()};

    // Return control to the asio scheduler. The enterTCExit stub will deal with
    // populating top of the stack with the returned null.
    v = slow_path;
    v << syncvmrettype{v.cns(KindOfNull)};
    v << leavetc{vm_regs_with_sp() | rret_type()};
  });

  return ret;
}

/*
 * Attempt to resume a parent of a currently returning async function (with
 * rvmfp() pointing to its ActRec), where the parent is known to be an async
 * function (with parentBl pointing to its AsioBlockable inside AFWH).
 *
 * If the control is transferred to the parent, the result of the previously
 * running async function will be stored to its AFWH, the AFWH will be marked
 * as succeeded and all remaining parents in the blockable chain will be
 * unblocked.
 *
 * If the parent cannot be resumed, the cannotResume label will be taken
 * without any state changes.
 */
void asyncFuncMaybeRetToAsyncFunc(Vout& v, PhysReg rdata, PhysReg rtype,
                                  Vreg parentBl, Vreg nextBl,
                                  Vlabel cannotResume) {
  // Check parentBl->getWH()->resumable()->resumeAddr() != nullptr.
  auto const isNullAddr = v.makeReg();
  v << cmpqim{0, parentBl[afwhToBl(AFWH::resumeAddrOff())], isNullAddr};
  ifThen(v, CC_E, isNullAddr, cannotResume);

  // Check parentBl->getWH()->getContextIdx() == child->getContextIdx().
  auto const childContextIdx = v.makeReg();
  auto const parentContextIdx = v.makeReg();
  auto const inSameContext = v.makeReg();

  v << loadb{rvmfp()[afwhToAr(AFWH::contextIdxOff())], childContextIdx};
  v << loadb{parentBl[afwhToBl(AFWH::contextIdxOff())], parentContextIdx};
  v << cmpb{parentContextIdx, childContextIdx, inSameContext};
  ifThen(v, CC_NE, inSameContext, cannotResume);

  // Fast path.

  // Transfer the return value from return registers to the resumed async
  // function via stack.
  v << store{rdata, rvmsp()[TVOFF(m_data)]};
  v << storeb{rtype, rvmsp()[TVOFF(m_type)]};

  // Set up PHP frame linkage for our parent by copying our ActRec's sfp.
  auto const sfp = v.makeReg();
  v << load{rvmfp()[AROFF(m_sfp)], sfp};
  v << store{sfp, parentBl[afwhToBl(AFWH::arOff()) + AROFF(m_sfp)]};

  // The AFWH is referenced at least twice:
  //  - one for being in the running state
  //  - one for being referenced by the parent we are going to resume
  //
  // If it is referenced exactly twice, there can't be any other parents.
  // Since the WH is going to be destroyed anyway, avoid populating its
  // state and result and just free the memory directly.
  auto const wh = v.makeReg();
  v << lea{rvmfp()[Resumable::dataOff() - Resumable::arOff()], wh};
  emitDecRef(v, wh, TRAP_REASON);
  auto const shouldRelease = emitCmpRefCount(v, OneReference, wh);
  ifThenElse(
    v, CC_E, shouldRelease,
    [&] (Vout& v) {  // Free the memory.
      v << vcall{
        CallSpec::direct(freeAFWH),
        v.makeVcallArgs({{wh}}),
        v.makeTuple({}),
        Fixup::none()
      };
    },
    [&] (Vout& v) {  // Someone else still has a ref, do the work.
      // Store the return value into the AFWH and mark it as finished.
      emitIncRefWork(v, rdata, rtype, TRAP_REASON);
      storeAFWHResult(v, rdata, rtype);

      // Drop the second ref, but we have more.
      emitDecRef(v, wh, TRAP_REASON);

      // Unblock all remaining parents. This may free the wh.
      unblockParents(v, nextBl);
    }
  );

  // Update vmfp() and vmFirstAR().
  v << lea{parentBl[afwhToBl(AFWH::arOff())], rvmfp()};
  v << store{rvmfp(), rvmtl()[rds::kVmFirstAROff]};

  // setState(STATE_RUNNING)
  auto const runningState = c_Awaitable::toKindState(
    c_Awaitable::Kind::AsyncFunction,
    c_ResumableWaitHandle::STATE_RUNNING
  );
  v << storebi{runningState, parentBl[afwhToBl(AFWH::stateOff())]};

  if (RO::EvalEnableImplicitContext) {
    markRDSAccess(v, ImplicitContext::activeCtx.handle());
    auto const implicitContext = v.makeReg();
    v << load{parentBl[afwhToBl(AFWH::implicitContextOff())], implicitContext};
    v << store{implicitContext, rvmtl()[ImplicitContext::activeCtx.handle()]};
  }

  // Transfer control to the resume address.
  v << jmpm{rvmfp()[afwhToAr(AFWH::resumeAddrOff())], vm_regs_with_sp()};
}

/*
 * Store the return value into the wait handle, unblock its parents,
 * update the frame pointer and decref.
 */
void asyncFuncRetOnly(Vout& v, PhysReg data, PhysReg type, Vreg parentBl) {
  // Transfer the return value from return registers into the AFWH, mark
  // it as finished and unblock all parents.
  storeAFWHResult(v, data, type);
  unblockParents(v, parentBl);

  // Get the pointer to the AFWH before losing FP.
  auto const wh = v.makeReg();
  v << lea{rvmfp()[Resumable::dataOff() - Resumable::arOff()], wh};

  // Load the saved frame pointer from the ActRec.
  v << load{rvmfp()[AROFF(m_sfp)], rvmfp()};

  // Decref the AFWH for no longer being in the running state.
  emitDecRefWorkObj(v, wh, TRAP_REASON);
}

TCA emitAsyncFuncRet(CodeBlock& cb, DataBlock& data, TCA switchCtrl) {
  alignCacheLine(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    auto const slowPath = Vlabel(v.makeBlock());

    // Load ptr to the first parent's blockable.
    auto const parentBl = v.makeReg();
    v << load{rvmfp()[afwhToAr(AFWH::parentChainOff())], parentBl};

    // Check if there's any parent. Parents are always in a blocked state.
    auto const hasParent = v.makeReg();
    v << testq{parentBl, parentBl, hasParent};
    ifThen(v, CC_Z, hasParent, slowPath);

    // Load blockable bits.
    auto const parentBlBits = v.makeReg();
    v << load{parentBl[AsioBlockable::bitsOff()], parentBlBits};

    // Is our parent an AFWH? Check parentBl->getKind() == AFWHN.
    static_assert(
      uint8_t(AsioBlockable::Kind::AsyncFunctionWaitHandleNode) == 0,
      "AFWH kind must be 0."
    );
    auto const isAFWH = v.makeReg();
    v << testbi{int8_t(AsioBlockable::kKindMask), parentBlBits, isAFWH};
    ifThen(v, CC_NZ, isAFWH, slowPath);

    // Try to resume the parent AFWH. The parentBlBits is already decoded
    // as the next blockable, since the kind bits are zero.
    asyncFuncMaybeRetToAsyncFunc(v, rarg(0), rarg(1), parentBl, parentBlBits,
                                 slowPath);
    assertx(v.closed());

    // Slow path. Finish returning and try to use the asyncSwitchCtrl stub
    // to resume another ResumableWaitHandle.
    v = slowPath;
    asyncFuncRetOnly(v, rarg(0), rarg(1), parentBl);
    v << jmpi{switchCtrl, vm_regs_with_sp()};
  });
}

TCA emitAsyncFuncRetSlow(CodeBlock& cb, DataBlock& data, TCA asyncFuncRet) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    auto const slowPath = Vlabel(v.makeBlock());

    // Check for surprise *after* the return event hook was called.
    irlower::emitCheckSurpriseFlags(v, rvmsp(), slowPath);

    // The return event hook cleared the surprise, continue on the fast path.
    v << jmpi{asyncFuncRet, vm_regs_with_sp() | rarg(0) | rarg(1)};

    // Slow path.
    v = slowPath;

    // Load ptr to the first parent's blockable.
    auto const parentBl = v.makeReg();
    v << load{rvmfp()[afwhToAr(AFWH::parentChainOff())], parentBl};

    // Finish returning and transfer control to the asio scheduler, which will
    // properly deal with the surprise when resuming the next function.
    asyncFuncRetOnly(v, rarg(0), rarg(1), parentBl);
    v << syncvmrettype{v.cns(KindOfNull)};
    v << leavetc{vm_regs_with_sp() | rret_type()};
  });
}

///////////////////////////////////////////////////////////////////////////////

}

void UniqueStubs::emitAllResumable(CodeCache& code, Debug::DebugInfo& dbg) {
  auto view = code.view();
  auto& main = view.main();
  auto& cold = view.cold();
  auto optView = code.view(TransKind::Optimize);
  auto& hotBlock = optView.main();
  auto& data = view.data();

  auto const hot = [&]() -> CodeBlock& {
    return hotBlock.available() > 512 ? hotBlock : main;
  };
  auto const hotView = [&]() -> CodeCache::View& {
    return hotBlock.available() > 512 ? optView : view;
  };

#define EMIT(name, v_in, stub)                                     \
  [&] {                                                            \
    auto const& v = (v_in);                                        \
    tc::TransLocMaker maker{v};                                    \
    maker.markStart();                                             \
    auto const start = (stub)();                                   \
    add(name, code, start, v, maker.markEnd().loc(), dbg);         \
    return start;                                                  \
  }()

#define ADD(name, v, stub) name = EMIT(#name, v, [&] { return (stub); })
  TCA inner_stub;
  ADD(asyncSwitchCtrl,
      hotView(),
      emitAsyncSwitchCtrl(hot(), data, &inner_stub));
  ADD(asyncFuncRet,     hotView(), emitAsyncFuncRet(hot(), data, inner_stub));
  ADD(asyncFuncRetSlow, view, emitAsyncFuncRetSlow(cold, data, asyncFuncRet));
#undef ADD
}

///////////////////////////////////////////////////////////////////////////////

}}
