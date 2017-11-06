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
#include "hphp/runtime/vm/jit/phys-reg.h"
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
  align(cb, nullptr, Alignment::JmpTarget, AlignContext::Dead);
}

///////////////////////////////////////////////////////////////////////////////

using AFWH = c_AsyncFunctionWaitHandle;

/*
 * Convert an AsyncFunctionWaitHandle-relative offset to an offset relative to
 * either its contained ActRec or AsioBlockable.
 */
constexpr ptrdiff_t ar_rel(ptrdiff_t off) {
  return off - AFWH::arOff();
}
constexpr ptrdiff_t bl_rel(ptrdiff_t off) {
  return off - AFWH::childrenOff() - AFWH::Node::blockableOff();
}

/*
 * Store the async function's return value to the AsyncFunctionWaitHandle.
 */
void storeAFWHResult(Vout& v, PhysReg data, PhysReg type) {
  auto const resultOff = ar_rel(AFWH::resultOff());
  v << store{data, rvmfp()[resultOff + TVOFF(m_data)]};
  // This store must preserve the kind bits in the WaitHandle for correctness.
  v << storeb{type, rvmfp()[resultOff + TVOFF(m_type)]};
}

/*
 * In a cold path, call into native code to unblock every member of an async
 * function's dependency chain, if it has any.
 */
void unblockParents(Vout& v, Vout& vc, Vreg parent) {
  auto const sf = v.makeReg();
  v << testq{parent, parent, sf};

  unlikelyIfThen(v, vc, CC_NZ, sf, [&] (Vout& v) {
    v << vcall{CallSpec::direct(AsioBlockableChain::UnblockJitHelper),
               v.makeVcallArgs({{rvmfp(), rvmsp(), parent}}), v.makeTuple({})};
  });
}

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

TCA emitAsyncSwitchCtrl(CodeBlock& cb, DataBlock& data, TCA* inner) {
  alignJmpTarget(cb);

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
      Fixup{},
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
        c_WaitHandle::Kind::AsyncFunction,
        AFWH::STATE_RUNNING
      ),
      afwh[AFWH::stateOff()]
    };

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

    emitIncRefWork(v, data, type);

    // Now decref `child', which may free it---but note that the WaitHandle's
    // destructor has no risk of reentry.
    emitDecRefWorkObj(v, child);

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

TCA emitAsyncRetCtrl(CodeBlock& cb, DataBlock& data, TCA switchCtrl) {
  alignJmpTarget(cb);

  return vwrap2(cb, cb, data, [&] (Vout& v, Vout& vc) {
    auto const data = rarg(0);
    auto const type = rarg(1);

    auto const slow_path = Vlabel(v.makeBlock());

    // Load the parent chain.
    auto const parentBl = v.makeReg();
    v << load{rvmfp()[ar_rel(AFWH::parentChainOff())], parentBl};

    // Set state to succeeded.
    v << storebi{
      c_WaitHandle::toKindState(
        c_WaitHandle::Kind::AsyncFunction,
        c_WaitHandle::STATE_SUCCEEDED
      ),
      rvmfp()[ar_rel(c_WaitHandle::stateOff())]
    };

    // Load the WaitHandle*.
    auto const wh = v.makeReg();
    v << lea{rvmfp()[Resumable::dataOff() - Resumable::arOff()], wh};

    // Check if there's any parent.
    auto const hasParent = v.makeReg();
    v << testq{parentBl, parentBl, hasParent};
    ifThen(v, CC_Z, hasParent, slow_path);

    // Check parentBl->getKind() == AFWH.
    static_assert(
      uint8_t(AsioBlockable::Kind::AsyncFunctionWaitHandleNode) == 0,
      "AFWH kind must be 0."
    );
    auto const isAFWH = v.makeReg();
    v << testbim{
      int8_t(AsioBlockable::kKindMask),
      parentBl[AsioBlockable::bitsOff()],
      isAFWH
    };
    ifThen(v, CC_NZ, isAFWH, slow_path);

    // Check parentBl->getBWH()->getKindState() == {Async, BLOCKED}.
    auto const blockedState = AFWH::toKindState(
      c_WaitHandle::Kind::AsyncFunction,
      AFWH::STATE_BLOCKED
    );
    auto const isBlocked = v.makeReg();
    v << cmpbim{blockedState, parentBl[bl_rel(AFWH::stateOff())], isBlocked};
    ifThen(v, CC_NE, isBlocked, slow_path);

    // Check parentBl->getBWH()->resumable()->resumeAddr() != nullptr.
    auto const isNullAddr = v.makeReg();
    v << cmpqim{0, parentBl[bl_rel(AFWH::resumeAddrOff())], isNullAddr};
    ifThen(v, CC_E, isNullAddr, slow_path);

    // Check parentBl->getContextIdx() == child->getContextIdx().
    auto const childContextIdx = v.makeReg();
    auto const parentContextIdx = v.makeReg();
    auto const inSameContext = v.makeReg();

    v << loadb{rvmfp()[ar_rel(AFWH::contextIdxOff())], childContextIdx};
    v << loadb{parentBl[bl_rel(AFWH::contextIdxOff())], parentContextIdx};
    v << cmpb{parentContextIdx, childContextIdx, inSameContext};
    ifThen(v, CC_NE, inSameContext, slow_path);

    /*
     * Fast path.
     *
     * Handle the return value, unblock any additional parents, release the
     * WaitHandle, and transfer control to the parent.
     */
    // Incref the return value.  In addition to pushing it onto the stack, we
    // are also storing it in the AFWH object.
    emitIncRefWork(v, data, type);

    // Write the return value to the stack and the AFWH object.
    v << store{data, rvmsp()[TVOFF(m_data)]};
    v << storeb{type, rvmsp()[TVOFF(m_type)]};
    storeAFWHResult(v, data, type);

    // Load the next parent in the chain, and unblock the whole chain.
    auto const nextParent = v.makeReg();
    auto const tmp = v.makeReg();
    v << load{parentBl[AsioBlockable::bitsOff()], tmp};
    v << andqi{
      int32_t(AsioBlockable::kParentMask),
      tmp,
      nextParent,
      v.makeReg()
    };
    unblockParents(v, vc, nextParent);

    // Set up PHP frame linkage for our parent by copying our ActRec's sfp.
    auto const sfp = v.makeReg();
    v << load{rvmfp()[AROFF(m_sfp)], sfp};
    v << store{sfp, parentBl[bl_rel(AFWH::arOff()) + AROFF(m_sfp)]};

    // Drop the reference to the current AFWH twice:
    //  - it is no longer being executed
    //  - it is no longer referenced by the parent
    //
    // The first time we don't need to check for release.  The second time, we
    // do, but we can type-specialize.
    emitDecRef(v, wh);
    emitDecRefWorkObj(v, wh);

    // Update vmfp() and vmFirstAR().
    v << lea{parentBl[bl_rel(AFWH::arOff())], rvmfp()};
    v << store{rvmfp(), rvmtl()[rds::kVmFirstAROff]};

    // setState(STATE_RUNNING)
    auto const runningState = c_WaitHandle::toKindState(
      c_WaitHandle::Kind::AsyncFunction,
      c_ResumableWaitHandle::STATE_RUNNING
    );
    v << storebi{runningState, parentBl[bl_rel(AFWH::stateOff())]};

    // Transfer control to the resume address.
    v << jmpm{rvmfp()[ar_rel(AFWH::resumeAddrOff())], vm_regs_with_sp()};

    /*
     * Slow path: unblock all parents, and return to the scheduler.
     */
    v = slow_path;

    // Store result into the AFWH object and unblock all parents.
    //
    // Storing the result into the AFWH overwrites contextIdx (they share a
    // union), so it has to be done after the checks in the fast path (but
    // before unblocking parents).
    storeAFWHResult(v, data, type);
    unblockParents(v, vc, parentBl);

    // Load the saved frame pointer from the ActRec.
    v << load{rvmfp()[AROFF(m_sfp)], rvmfp()};

    // Decref the WaitHandle.  We only do it once here (unlike in the fast
    // path) since we're not also resuming a parent that we've unblocked.
    emitDecRefWorkObj(v, wh);

    v << jmpi{switchCtrl, vm_regs_with_sp()};
  });
}

///////////////////////////////////////////////////////////////////////////////

}

void UniqueStubs::emitAllResumable(CodeCache& code, Debug::DebugInfo& dbg) {
  auto view = code.view();
  auto& main = view.main();
  auto& hotBlock = code.view(TransKind::Optimize).main();
  auto& data = view.data();

  auto const hot = [&]() -> CodeBlock& {
    return hotBlock.available() > 512 ? hotBlock : main;
  };

#define ADD(name, stub) name = add(#name, (stub), code, dbg)
  TCA inner_stub;
  ADD(asyncSwitchCtrl,  emitAsyncSwitchCtrl(main, data, &inner_stub));
  ADD(asyncRetCtrl,     emitAsyncRetCtrl(hot(), data, inner_stub));
#undef ADD
}

///////////////////////////////////////////////////////////////////////////////

}}
