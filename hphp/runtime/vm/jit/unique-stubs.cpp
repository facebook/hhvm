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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-tls.h"
#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/phys-reg-saver.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/service-request-handlers.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/stack-overflow.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs-ppc64.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/trace.h"

#include <folly/Format.h>

#include <algorithm>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void alignJmpTarget(CodeBlock& cb) {
  align(cb, nullptr, Alignment::JmpTarget, AlignContext::Dead);
}

void assertNativeStackAligned(Vout& v) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << call{TCA(assert_native_stack_aligned)};
  }
}

/*
 * Load and store the VM registers from/to RDS.
 */
void loadVMRegs(Vout& v) {
  v << load{rvmtl()[rds::kVmfpOff], rvmfp()};
  v << load{rvmtl()[rds::kVmspOff], rvmsp()};
}
void storeVMRegs(Vout& v) {
  v << store{rvmfp(), rvmtl()[rds::kVmfpOff]};
  v << store{rvmsp(), rvmtl()[rds::kVmspOff]};
}

/*
 * Load and store the PHP return registers from/to the top of the VM stack.
 *
 * Note that we don't do loadb{}/storeb{} for the type register, because we
 * sometimes need to preserve the m_aux field across returns.
 */
void loadReturnRegs(Vout& v) {
  v << load{rvmsp()[TVOFF(m_data)], rret_data()};
  v << load{rvmsp()[TVOFF(m_type)], rret_type()};
}
void storeReturnRegs(Vout& v) {
  v << store{rret_data(), rvmsp()[TVOFF(m_data)]};
  v << store{rret_type(), rvmsp()[TVOFF(m_type)]};
}

/*
 * Convenience wrapper around a simple vcall to `helper', with a single `arg'
 * and a return value in `d'.
 */
template<class F>
Vinstr simplecall(Vout& v, F helper, Vreg arg, Vreg d) {
  return vcall{
    CallSpec::direct(helper),
    v.makeVcallArgs({{arg}}),
    v.makeTuple({d}),
    Fixup{},
    DestType::SSA
  };
}

/*
 * Emit a catch trace that unwinds a stub context back to the PHP context that
 * called it.
 */
template<class GenFn>
void emitStubCatch(Vout& v, const UniqueStubs& us, GenFn gen) {
  always_assert(us.endCatchHelper);
  v << landingpad{};
  gen(v);
  v << stubunwind{};
  v << jmpi{us.endCatchHelper};
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFreeLocalsHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  return ARCH_SWITCH_CALL(emitFreeLocalsHelpers, cb, data, us);
}

TCA emitCallToExit(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  return ARCH_SWITCH_CALL(emitCallToExit, cb, data, us);
}

///////////////////////////////////////////////////////////////////////////////

TCA fcallHelper(ActRec* ar) {
  assert_native_stack_aligned();
  assertx(!ar->resumed());

  if (LIKELY(!RuntimeOption::EvalFailJitPrologs)) {
    auto const tca = mcgen::getFuncPrologue(
      const_cast<Func*>(ar->func()),
      ar->numArgs()
    );
    if (tca) return tca;
  }

  // Check for stack overflow in the same place func prologues make their
  // StackCheck::Early check (see irgen-func-prologue.cpp).  This handler also
  // cleans and syncs vmRegs for us.
  if (checkCalleeStackOverflow(ar)) handleStackOverflow(ar);

  try {
    VMRegAnchor _(ar);
    if (doFCall(ar, vmpc())) {
      return tc::ustubs().resumeHelperRet;
    }
    // We've been asked to skip the function body (fb_intercept).  The vmregs
    // have already been fixed; indicate this with a nullptr return.
    return nullptr;
  } catch (...) {
    // The VMRegAnchor above took care of us, but we need to tell the unwinder
    // (since ~VMRegAnchor() will have reset tl_regState).
    tl_regState = VMRegState::CLEAN;
    throw;
  }
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFuncPrologueRedispatch(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [] (Vout& v) {
    auto const func = v.makeReg();
    v << load{rvmfp()[AROFF(m_func)], func};

    auto const argc = v.makeReg();
    auto const naaf = v.makeReg();
    v << loadl{rvmfp()[AROFF(m_numArgsAndFlags)], naaf};
    v << andli{ActRec::kNumArgsMask, naaf, argc, v.makeReg()};

    auto const nparams = v.makeReg();
    auto const pcounts = v.makeReg();
    v << loadl{func[Func::paramCountsOff()], pcounts};
    v << shrli{0x1, pcounts, nparams, v.makeReg()};

    auto const sf = v.makeReg();
    v << cmpl{argc, nparams, sf};

    auto const pTabOff = safe_cast<int32_t>(Func::prologueTableOff());
    auto const ptrSize = safe_cast<int32_t>(sizeof(LowPtr<uint8_t>));

    // If we passed more args than declared, we need to dispatch to the
    // "too many arguments" prologue.
    ifThen(v, CC_L, sf, [&] (Vout& v) {
      auto const dest = v.makeReg();

      auto const nargs = v.makeReg();
      v << movzlq{nparams, nargs};

      emitLdLowPtr(v, func[nargs * ptrSize + (pTabOff + ptrSize)],
                   dest, sizeof(LowPtr<uint8_t>));
      v << jmpr{dest};
    });

    auto const nargs = v.makeReg();
    v << movzlq{argc, nargs};

    auto const dest = v.makeReg();
    emitLdLowPtr(v, func[nargs * ptrSize + pTabOff],
                 dest, sizeof(LowPtr<uint8_t>));
    v << jmpr{dest};
  });
}

TCA emitFCallHelperThunk(CodeBlock& main, CodeBlock& cold, DataBlock& data) {
  alignJmpTarget(main);

  return vwrap2(main, cold, data, [] (Vout& v, Vout& vc) {
    v << phplogue{rvmfp()};

    // fcallHelper asserts native stack alignment for us.
    TCA (*helper)(ActRec*) = &fcallHelper;
    auto const dest = v.makeReg();
    v << simplecall(v, helper, rvmfp(), dest);

    // Clobber rvmsp in debug builds.
    if (debug) v << copy{v.cns(0x1), rvmsp()};

    auto const sf = v.makeReg();
    v << testq{dest, dest, sf};

    unlikelyIfThen(v, vc, CC_Z, sf, [&] (Vout& v) {
      // A nullptr dest means the callee was intercepted and should be skipped.
      // Make a copy of the current rvmfp(), which belongs to the callee,
      // before syncing VM regs and return regs.
      auto const callee_fp = v.makeReg();
      v << copy{rvmfp(), callee_fp};
      loadVMRegs(v);
      loadReturnRegs(v);

      // Do a PHP return to the caller---i.e., relative to the callee's frame.
      // Note that if intercept skips the callee, it tears down its frame but
      // guarantees that m_savedRip remains valid, so this is safe (and is the
      // only way to get the return address).
      //
      // TODO(#8908075): We've been fishing the m_savedRip out of the callee's
      // logically-trashed frame for a while now, but we really ought to
      // respect that the frame is freed and not touch it.
      v << phpret{callee_fp, rvmfp(), php_return_regs(), true};
    });

    // Jump to the func prologue.
    v << tailcallphp{dest, rvmfp(), php_call_regs()};
  });
}

TCA emitFuncBodyHelperThunk(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [] (Vout& v) {
    TCA (*helper)(ActRec*) = &svcreq::funcBodyHelper;
    auto const dest = v.makeReg();
    v << simplecall(v, helper, rvmfp(), dest);
    v << jmpr{dest};
  });
}

TCA emitFunctionEnterHelper(CodeBlock& main, CodeBlock& cold,
                            DataBlock& data, UniqueStubs& us) {
  alignJmpTarget(main);

  CGMeta meta;

  auto const start = vwrap2(main, cold, data, meta, [&] (Vout& v, Vout& vc) {
    auto const ar = v.makeReg();

    v << copy{rvmfp(), ar};

    // Fully set up the call frame for the stub.  We can't skip this like we do
    // in other stubs because we need the return IP for this frame in the %rbp
    // chain, in order to find the proper fixup for the VMRegAnchor in the
    // intercept handler.
    v << stublogue{true};
    v << copy{rsp(), rvmfp()};

    // When we call the event hook, it might tell us to skip the callee
    // (because of fb_intercept).  If that happens, we need to return to the
    // caller, but the handler will have already popped the callee's frame.
    // So, we need to save these values for later.
    v << pushpm{ar[AROFF(m_savedRip)], ar[AROFF(m_sfp)]};

    v << copy2{ar, v.cns(EventHook::NormalFunc), rarg(0), rarg(1)};

    auto const done = v.makeBlock();
    auto const ctch = vc.makeBlock();
    auto const should_continue = v.makeReg();
    bool (*hook)(const ActRec*, int) = &EventHook::onFunctionCall;

    v << vinvoke{
      CallSpec::direct(hook),
      v.makeVcallArgs({{ar, v.cns(EventHook::NormalFunc)}}),
      v.makeTuple({should_continue}),
      {done, ctch},
      Fixup{},
      DestType::SSA
    };

    vc = ctch;
    emitStubCatch(vc, us, [] (Vout& v) {
      // Skip past the stuff we saved for the intercept case.
      v << lea{rsp()[16], rsp()};
      // Undo our stub frame, so that rvmfp() points to the parent VM frame.
      v << load{rsp()[AROFF(m_sfp)], rvmfp()};
    });

    v = done;

    auto const sf = v.makeReg();
    v << testb{should_continue, should_continue, sf};

    unlikelyIfThen(v, vc, CC_Z, sf, [&] (Vout& v) {
      auto const saved_rip = v.makeReg();

      // The event hook has already cleaned up the stack and popped the
      // callee's frame, so we're ready to continue from the original call
      // site.  We just need to grab the fp/rip of the original frame that we
      // saved earlier, and sync rvmsp().
      v << popp{rvmfp(), saved_rip};

      // Drop our call frame; the stublogue{} instruction guarantees that this
      // is exactly 16 bytes.
      v << lea{rsp()[kNativeFrameSize], rsp()};

      // Sync vmsp and the return regs.
      v << load{rvmtl()[rds::kVmspOff], rvmsp()};
      v << load{rvmsp()[TVOFF(m_data)], rret_data()};
      v << load{rvmsp()[TVOFF(m_type)], rret_type()};

      // Return to the caller.  This unbalances the return stack buffer, but if
      // we're intercepting, we probably don't care.
      v << jmpr{saved_rip};
    });

    // Skip past the stuff we saved for the intercept case.
    v << lea{rsp()[16], rsp()};

    // Restore rvmfp() and return to the callee's func prologue.
    v << stubret{RegSet(), true};
  });

  meta.process(nullptr);
  return start;
}

TCA emitFunctionSurprisedOrStackOverflow(CodeBlock& main,
                                         CodeBlock& cold,
                                         DataBlock& data,
                                         const UniqueStubs& us) {
  alignJmpTarget(main);

  CGMeta meta;

  auto const start = vwrap2(main, cold, data, meta, [&] (Vout& v, Vout& vc) {
    v << stublogue{};

    auto const done = v.makeBlock();
    auto const ctch = vc.makeBlock();

    v << vinvoke{CallSpec::direct(handlePossibleStackOverflow),
                 v.makeVcallArgs({{rvmfp()}}), v.makeTuple({}),
                 {done, ctch}};
    vc = ctch;
    emitStubCatch(vc, us, [](Vout& /*v*/) {});

    v = done;
    v << tailcallstub{us.functionEnterHelper};
  });

  meta.process(nullptr);
  return start;
}

///////////////////////////////////////////////////////////////////////////////

template<bool async>
void loadGenFrame(Vout& v, Vreg d) {
  auto const arOff = BaseGenerator::arOff() -
    (async ? AsyncGenerator::objectOff() : Generator::objectOff());

  auto const gen = v.makeReg();

  // We have to get the Generator object from the current frame's $this, then
  // load the embedded frame.
  v << load{rvmfp()[AROFF(m_thisUnsafe)], gen};
  v << lea{gen[arOff], d};
}

void debuggerRetImpl(Vout& v, Vreg ar) {
  auto const soff = v.makeReg();

  v << loadl{ar[AROFF(m_soff)], soff};
  v << storel{soff, rvmtl()[unwinderDebuggerReturnOffOff()]};
  v << store{rvmsp(), rvmtl()[unwinderDebuggerReturnSPOff()]};

  auto const ret = v.makeReg();
  v << simplecall(v, unstashDebuggerCatch, ar, ret);

  v << jmpr{ret};
}

TCA emitInterpRet(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  auto const start = vwrap(cb, data, [] (Vout& v) {
    // Sync return regs before calling native assert function.
    storeReturnRegs(v);
    assertNativeStackAligned(v);

    v << lea{rvmsp()[-kArRetOff], r_svcreq_arg(0)};
    v << copy{rvmfp(), r_svcreq_arg(1)};
  });
  svcreq::emit_persistent(cb, data, folly::none, REQ_POST_INTERP_RET);
  return start;
}

template<bool async>
TCA emitInterpGenRet(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  auto const start = vwrap(cb, data, [] (Vout& v) {
    // Sync return regs before calling native assert function.
    storeReturnRegs(v);
    assertNativeStackAligned(v);

    loadGenFrame<async>(v, r_svcreq_arg(0));
    v << copy{rvmfp(), r_svcreq_arg(1)};
  });
  svcreq::emit_persistent(cb, data, folly::none, REQ_POST_INTERP_RET);
  return start;
}

TCA emitDebuggerInterpRet(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [] (Vout& v) {
    // Sync return regs before calling native assert function.
    storeReturnRegs(v);
    assertNativeStackAligned(v);

    auto const ar = v.makeReg();
    v << lea{rvmsp()[-kArRetOff], ar};
    debuggerRetImpl(v, ar);
  });
}

template<bool async>
TCA emitDebuggerInterpGenRet(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [] (Vout& v) {
    assertNativeStackAligned(v);

    auto const ar = v.makeReg();
    loadGenFrame<async>(v, ar);
    debuggerRetImpl(v, ar);
  });
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
    v << jmpm{afwh[AFWH::resumeAddrOff()], php_return_regs()};

    v = slow_path;
    v << leavetc{php_return_regs()};
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
    v << jmpm{rvmfp()[ar_rel(AFWH::resumeAddrOff())], php_return_regs()};

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

    v << jmpi{switchCtrl, php_return_regs()};
  });
}

///////////////////////////////////////////////////////////////////////////////

template<bool immutable>
TCA emitBindCallStub(CodeBlock& cb, DataBlock& data) {
  return vwrap(cb, data, [] (Vout& v) {
    v << phplogue{rvmfp()};

    auto args = VregList { v.makeReg(), v.makeReg(), v.makeReg() };

    // Reconstruct the address of the call from the saved RIP.
    auto const savedRIP = v.makeReg();
    auto const callLen = safe_cast<int>(smashableCallLen());
    v << load{rvmfp()[AROFF(m_savedRip)], savedRIP};
    v << subqi{callLen, savedRIP, args[0], v.makeReg()};

    v << copy{rvmfp(), args[1]};
    v << movb{v.cns(immutable), args[2]};

    auto const ret = v.makeReg();

    v << vcall{
      CallSpec::direct(svcreq::handleBindCall),
      v.makeVcallArgs({args}),
      v.makeTuple({ret}),
      Fixup{},
      DestType::SSA
    };

    v << tailcallphp{ret, rvmfp(), php_call_regs()};
  });
}

TCA emitFCallArrayHelper(CodeBlock& main, CodeBlock& cold,
                         DataBlock& data, UniqueStubs& us) {
  align(main, nullptr, Alignment::CacheLine, AlignContext::Dead);

  CGMeta meta;

  auto const ret = vwrap(main, data, [] (Vout& v) {
    v << movl{v.cns(0), rarg(2)};
  });

  us.fcallUnpackHelper = vwrap2(main, cold, data, meta,
                                [&] (Vout& v, Vout& vc) {
    // We reach fcallArrayHelper in the same context as a func prologue, so
    // this should really be a phplogue{}---but we don't need the return
    // address in the ActRec until later, and in the event the callee is
    // intercepted, we must save it on the stack because the callee frame will
    // already have been popped.  So use a stublogue and "convert" it manually
    // later.
    v << stublogue{};

    storeVMRegs(v);

    auto const func = v.makeReg();
    auto const unit = v.makeReg();
    auto const bc = v.makeReg();

    // Load fp->m_func->m_unit->m_bc.
    v << load{rvmfp()[AROFF(m_func)], func};
    v << load{func[Func::unitOff()], unit};
    v << load{unit[Unit::bcOff()], bc};

    auto const pc = v.makeReg();
    auto const next = v.makeReg();

    // Convert offsets into PCs, and sync the PC.
    v << addq{bc, rarg(0), pc, v.makeReg()};
    v << store{pc, rvmtl()[rds::kVmpcOff]};
    v << addq{bc, rarg(1), next, v.makeReg()};

    auto const retAddr = v.makeReg();
    v << loadstubret{retAddr};

    auto const done = v.makeBlock();
    auto const ctch = vc.makeBlock();
    auto const should_continue = v.makeReg();
    bool (*helper)(PC, int32_t, void*) = &doFCallArrayTC;

    v << vinvoke{
      CallSpec::direct(helper),
      v.makeVcallArgs({{next, rarg(2), retAddr}}),
      v.makeTuple({should_continue}),
      {done, ctch},
      Fixup{},
      DestType::SSA
    };
    vc = ctch;
    emitStubCatch(vc, us, [] (Vout& v) { loadVMRegs(v); });

    v = done;

    // Load only rvmsp(); we need to wait to make sure we aren't skipping the
    // callee before loading rvmfp().
    v << load{rvmtl()[rds::kVmspOff], rvmsp()};

    auto const sf = v.makeReg();
    v << testb{should_continue, should_continue, sf};

    unlikelyIfThen(v, vc, CC_Z, sf, [&] (Vout& v) {
      // If false was returned, we should skip the callee.  The interpreter
      // will have popped the pre-live ActRec already, so we can just return to
      // the caller after syncing the return regs.
      loadReturnRegs(v);
      v << stubret{};
    });
    v << load{rvmtl()[rds::kVmfpOff], rvmfp()};

    // If true was returned, we're calling the callee, so undo the stublogue{}
    // and convert to a phplogue{}.
    v << stubtophp{rvmfp()};

    auto const callee = v.makeReg();
    auto const body = v.makeReg();

    v << load{rvmfp()[AROFF(m_func)], callee};
    emitLdLowPtr(v, callee[Func::funcBodyOff()], body, sizeof(LowPtr<uint8_t>));

    // We jmp directly to the func body---this keeps the return stack buffer
    // balanced between the call to this stub and the ret from the callee.
    v << jmpr{body};
  });

  meta.process(nullptr);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

struct ResumeHelperEntryPoints {
  TCA resumeHelperRet;
  TCA resumeHelper;
  TCA handleResume;
  TCA reenterTC;
};

ResumeHelperEntryPoints emitResumeHelpers(CodeBlock& cb, DataBlock& data) {
  ResumeHelperEntryPoints rh;

  rh.resumeHelperRet = vwrap(cb, data, [] (Vout& v) {
    v << phplogue{rvmfp()};
  });
  rh.resumeHelper = vwrap(cb, data, [] (Vout& v) {
    v << ldimmb{0, rarg(0)};
  });

  rh.handleResume = vwrap(cb, data, [] (Vout& v) {
    v << load{rvmtl()[rds::kVmfpOff], rvmfp()};

    auto const handler = reinterpret_cast<TCA>(svcreq::handleResume);
    v << call{handler, arg_regs(2)};
  });

  rh.reenterTC = vwrap(cb, data, [] (Vout& v) {
    // Save the return of handleResume(), then sync regs.
    auto const target = v.makeReg();
    v << copy{rret(), target};

    loadVMRegs(v);
    loadReturnRegs(v);  // spurious load if we're not returning

    v << jmpr{target};
  });

  return rh;
}

TCA emitResumeInterpHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us,
                            ResumeHelperEntryPoints& rh) {
  alignJmpTarget(cb);

  rh = emitResumeHelpers(cb, data);

  us.resumeHelperRet = rh.resumeHelperRet;
  us.resumeHelper = rh.resumeHelper;

  us.interpHelper = vwrap(cb, data, [] (Vout& v) {
    v << store{rarg(0), rvmtl()[rds::kVmpcOff]};
  });
  us.interpHelperSyncedPC = vwrap(cb, data, [&] (Vout& v) {
    storeVMRegs(v);
    v << ldimmb{1, rarg(0)};
    v << jmpi{rh.handleResume, RegSet(rarg(0))};
  });

  us.fcallAwaitSuspendHelper = vwrap(cb, data, [&] (Vout& v) {
    v << load{rvmtl()[rds::kVmfpOff], rvmfp()};

    auto const handler = reinterpret_cast<TCA>(svcreq::handleFCallAwaitSuspend);
    v << call{handler, arg_regs(2)};
    v << jmpi{rh.reenterTC, RegSet()};
  });

  return us.resumeHelperRet;
}

TCA emitInterpOneCFHelper(CodeBlock& cb, DataBlock& data, Op op,
                          const ResumeHelperEntryPoints& rh) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    v << copy2{rvmfp(), rvmsp(), rarg(0), rarg(1)};
    // rarg(2) is set at the stub callsite.

    auto const handler = reinterpret_cast<TCA>(
      interpOneEntryPoints[static_cast<size_t>(op)]
    );
    v << call{handler, arg_regs(3)};

    auto const sf = v.makeReg();
    auto const next = v.makeBlock();

    v << testq{rret(), rret(), sf};
    v << jcci{CC_NZ, sf, next, rh.reenterTC};
    v = next;
    v << jmpi{rh.resumeHelper};
  });
}

void emitInterpOneCFHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us,
                            const ResumeHelperEntryPoints& rh,
                            const CodeCache& code, Debug::DebugInfo& dbg) {
  alignJmpTarget(cb);

  auto const emit = [&] (Op op, const char* name) {
    auto const stub = emitInterpOneCFHelper(cb, data, op, rh);
    us.interpOneCFHelpers[op] = stub;
    us.add(name, stub, code, dbg);
  };

#define O(name, imm, in, out, flags)          \
  if (((flags) & CF) || ((flags) & TF)) {     \
    emit(Op::name, "interpOneCFHelper"#name); \
  }
  OPCODES
#undef O

  // Exit is a very special snowflake.  Because it can appear in PHP
  // expressions, the emitter pretends that it pushed a value on the eval stack
  // (and iopExit actually does push Null right before throwing).  Marking it
  // as TF would mess up any bytecodes that want to consume its output value,
  // so we can't do that.  But we also don't want to extend regions past it, so
  // the JIT treats it as terminal and uses InterpOneCF to execute it.
  emit(Op::Exit, "interpOneCFHelperExit");
}

///////////////////////////////////////////////////////////////////////////////

TCA emitDecRefGeneric(CodeBlock& cb, DataBlock& data) {
  CGMeta meta;

  auto const start = vwrap(cb, data, meta, [] (Vout& v) {
    v << vregrestrict{};
    v << stublogue{};

    auto const rdata = rarg(0);
    auto const rtype = rarg(1);

    auto const destroy = [&] (Vout& v) {
      // decRefGeneric is called via callfaststub, whose ABI claims that all
      // registers are preserved.  This is true in the fast path, but in the
      // slow path we need to manually save caller-saved registers.
      auto const callerSaved = abi().gpUnreserved - abi().calleeSaved;
      PhysRegSaver prs{v, callerSaved};

      // As a consequence of being called via callfaststub, we can't safely use
      // any Vregs here except for status flags registers, at least not with
      // the default vwrap() ABI.  Just use the argument registers instead.
      assertx(callerSaved.contains(rdata));
      assertx(callerSaved.contains(rtype));

      auto const dtor = lookupDestructor(v, rtype);
      v << callm{dtor, arg_regs(1)};

      // The stub frame's saved RIP is at %rsp[8] before we saved the
      // caller-saved registers.
      v << syncpoint{makeIndirectFixup(prs.dwordsPushed())};
    };

    emitDecRefWork(v, v, rdata, destroy, false);

    v << stubret{};
  });

  meta.process(nullptr);
  return start;
}

///////////////////////////////////////////////////////////////////////////////

TCA emitEnterTCExit(CodeBlock& cb, DataBlock& data, UniqueStubs& /*us*/) {
  return vwrap(cb, data, [&] (Vout& v) {
    // Eagerly save VM regs.
    storeVMRegs(v);

    // Realign the native stack.
    switch (arch()) {
      case Arch::X64:
      case Arch::PPC64:
        v << lea{rsp()[8], rsp()};
        break;
      case Arch::ARM:
        break;
    }

    // Store the return value on the top of the eval stack.  Whenever we get to
    // enterTCExit, we're semantically executing some PHP construct that sends
    // a return value out of a function (either a RetC, or a Yield, or an Await
    // that's suspending, etc), and moreover, we must be executing the return
    // that leaves this level of VM reentry (i.e. the only way we get here is
    // by coming from the callToExit stub or by a phpret{} or leavetc{} that
    // undoes the calltc{} or resumetc{} in enterTCHelper).
    //
    // Either way, we have a live PHP return value in the return registers,
    // which we need to put on the top of the evaluation stack.
    storeReturnRegs(v);

    // Perform a native return.
    //
    // On PPC64, as there is no new frame created when entering the VM, the FP
    // must not be saved.
    v << stubret{RegSet(), arch() != Arch::PPC64};
  });
}

TCA emitEnterTCHelper(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  alignJmpTarget(cb);

  auto const sp       = rarg(0);
  auto const fp       = rarg(1);
  auto const start    = rarg(2);
  auto const firstAR  = rarg(3);
#ifdef _MSC_VER
  auto const tl       = reg::r10;
  auto const calleeAR = reg::r11;
#else
  auto const tl       = rarg(4);
  auto const calleeAR = rarg(5);
#endif

  return vwrap2(cb, cb, data, [&] (Vout& v, Vout& vc) {
    // Architecture-specific setup for entering the TC.
    v << inittc{};

    // Native func prologue.
    v << stublogue{arch() != Arch::PPC64};

#ifdef _MSC_VER
    // Windows hates argument registers.
    v << load{rsp()[0x28], reg::r10};
    v << load{rsp()[0x30], reg::r11};
#endif

    // Set up linkage with the top VM frame in this nesting.
    v << store{rsp(), firstAR[AROFF(m_sfp)]};

    // Set up the VM registers.
    v << copy{fp, rvmfp()};
    v << copy{sp, rvmsp()};
    v << copy{tl, rvmtl()};

    // Unalign the native stack.
    switch (arch()) {
      case Arch::X64:
      case Arch::PPC64:
        v << lea{rsp()[-8], rsp()};
        break;
      case Arch::ARM:
        break;
    }

    // Check if `calleeAR' was set.
    auto const sf = v.makeReg();
    v << testq{calleeAR, calleeAR, sf};

    // We mark this block as unlikely in order to coax the emitter into
    // ordering this block last.  This is an important optimization for x64;
    // without it, both the jcc for the branch and the jmp for the resumetc{}
    // will end up in the same 16-byte extent of code, which messes up the
    // branch predictor.
    unlikelyIfThen(v, vc, CC_Z, sf, [&] (Vout& v) {
      // No callee means we're resuming in the middle of a TC function.
      v << resumetc{start, us.enterTCExit, vm_regs_with_sp()};
    });

    // We have a callee; set rvmfp() and call it.
    v << copy{calleeAR, rvmfp()};
    v << calltc{start, rvmfp(), us.enterTCExit, vm_regs_with_sp()};
  });
}

TCA emitHandleSRHelper(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [] (Vout& v) {
    storeVMRegs(v);

    // Pack the service request args into a svcreq::ReqInfo on the stack.
    assertx(!(svcreq::kMaxArgs & 1));
    for (auto i = svcreq::kMaxArgs; i >= 2; i -= 2) {
      v << pushp{r_svcreq_arg(i - 1), r_svcreq_arg(i - 2)};
    }
    v << pushp{r_svcreq_stub(), r_svcreq_req()};

    // Call mcg->handleServiceRequest(rsp()).
    auto const sp = v.makeReg();
    v << copy{rsp(), sp};

    auto const ret = v.makeReg();

    v << vcall{
      CallSpec::direct(svcreq::handleServiceRequest),
      v.makeVcallArgs({{sp}}),
      v.makeTuple({ret}),
      Fixup{},
      DestType::SSA
    };

    // Pop the ReqInfo off the stack.
    auto const reqinfo_sz = static_cast<int>(sizeof(svcreq::ReqInfo));
    v << lea{rsp()[reqinfo_sz], rsp()};

    // rvmtl() was preserved by the callee, but rvmsp() and rvmfp() might've
    // changed if we interpreted anything.  Reload them.  Also load the return
    // regs; if we're not returning, it's a spurious load.
    loadVMRegs(v);
    loadReturnRegs(v);

    v << jmpr{ret};
  });
}

///////////////////////////////////////////////////////////////////////////////

TCA emitEndCatchHelper(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  auto const udrspo = rvmtl()[unwinderDebuggerReturnSPOff()];

  auto const debuggerReturn = vwrap(cb, data, [&] (Vout& v) {
    v << load{udrspo, rvmsp()};
    v << storeqi{0, udrspo};
  });
  svcreq::emit_persistent(cb, data, folly::none, REQ_POST_DEBUGGER_RET);

  CGMeta meta;

  auto const resumeCPPUnwind = vwrap(cb, data, meta, [&] (Vout& v) {
    static_assert(sizeof(tl_regState) == 8,
                  "The following store must match the size of tl_regState.");
    auto const regstate = emitTLSAddr(v, tls_datum(tl_regState));
    v << storeqi{static_cast<int32_t>(VMRegState::CLEAN), regstate};

    v << load{rvmtl()[unwinderExnOff()], rarg(0)};
    v << call{TCA(_Unwind_Resume), arg_regs(1), &us.endCatchHelperPast};
    v << ud2{};
  });
  meta.process(nullptr);

  alignJmpTarget(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    auto const done1 = v.makeBlock();
    auto const sf1 = v.makeReg();

    v << cmpqim{0, udrspo, sf1};
    v << jcci{CC_NE, sf1, done1, debuggerReturn};
    v = done1;

    // Normal end catch situation: call back to tc_unwind_resume, which returns
    // the catch trace (or null) in the first return register, and the new vmfp
    // in the second.
    v << copy{rvmfp(), rarg(0)};
    v << call{TCA(tc_unwind_resume), arg_regs(1)};
    v << copy{rret(1), rvmfp()};

    auto const done2 = v.makeBlock();
    auto const sf2 = v.makeReg();

    v << testq{rret(0), rret(0), sf2};
    v << jcci{CC_Z, sf2, done2, resumeCPPUnwind};
    v = done2;

    v << jmpr{rret(0)};
  });
}

TCA emitUnknownExceptionHandler(CodeBlock& cb,
                                DataBlock& data,
                                UniqueStubs& us) {
  alignJmpTarget(cb);

  CGMeta meta;
  auto const ret = vwrap(cb, data, meta, [&] (Vout& v) {
    v << call{
      TCA(unknownExceptionHandler), {}, &us.unknownExceptionHandlerPast
    };
  });
  meta.process(nullptr);

  return ret;
}

TCA emitThrowSwitchMode(CodeBlock& cb, DataBlock& data) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [] (Vout& v) {
    v << call{TCA(throwSwitchMode)};
    v << ud2{};
  });
}

template<class F>
TCA emitHelperThunk(CodeCache& code, CodeBlock& cb, DataBlock& data, F* func) {
  // we only emit these calls into hot, main and cold.
  if (deltaFits(code.base() - (TCA)func, sz::dword) &&
      deltaFits(code.frozen().base() - (TCA)func, sz::dword)) {
    return (TCA)func;
  }
  alignJmpTarget(cb);
  return vwrap(cb, data, [&] (Vout& v) {
      v << jmpi{(TCA)func};
  });
}

///////////////////////////////////////////////////////////////////////////////

}

void UniqueStubs::emitAll(CodeCache& code, Debug::DebugInfo& dbg) {
  auto view = code.view();
  auto& main = view.main();
  auto& cold = view.cold();
  auto& frozen = view.frozen();
  auto& hotBlock = code.view(TransKind::Optimize).main();
  auto& data = view.data();

  auto const hot = [&]() -> CodeBlock& {
    return hotBlock.available() > 512 ? hotBlock : main;
  };

#define ADD(name, stub) name = add(#name, (stub), code, dbg)
  ADD(enterTCExit,   emitEnterTCExit(main, data, *this));
  enterTCHelper =
    decltype(enterTCHelper)(add("enterTCHelper",
                                emitEnterTCHelper(main, data, *this),
                                code,
                                dbg));

  // These guys are required by a number of other stubs.
  ADD(handleSRHelper, emitHandleSRHelper(hot(), data));
  ADD(endCatchHelper, emitEndCatchHelper(frozen, data, *this));
  ADD(unknownExceptionHandler, emitUnknownExceptionHandler(cold, data, *this));

  ADD(funcPrologueRedispatch, emitFuncPrologueRedispatch(hot(), data));
  ADD(fcallHelperThunk,       emitFCallHelperThunk(cold, frozen, data));
  ADD(funcBodyHelperThunk,    emitFuncBodyHelperThunk(cold, data));
  ADD(functionEnterHelper, emitFunctionEnterHelper(cold, frozen, data, *this));
  ADD(functionSurprisedOrStackOverflow,
      emitFunctionSurprisedOrStackOverflow(cold, frozen, data, *this));

  ADD(retHelper,                  emitInterpRet(hot(), data));
  ADD(genRetHelper,               emitInterpGenRet<false>(cold, data));
  ADD(asyncGenRetHelper,          emitInterpGenRet<true>(hot(), data));
  ADD(retInlHelper,               emitInterpRet(hot(), data));
  ADD(debuggerRetHelper,          emitDebuggerInterpRet(cold, data));
  ADD(debuggerGenRetHelper,       emitDebuggerInterpGenRet<false>(cold, data));
  ADD(debuggerAsyncGenRetHelper,  emitDebuggerInterpGenRet<true>(cold, data));

  TCA inner_stub;
  ADD(asyncSwitchCtrl,  emitAsyncSwitchCtrl(main, data, &inner_stub));
  ADD(asyncRetCtrl,     emitAsyncRetCtrl(hot(), data, inner_stub));

  ADD(bindCallStub,           emitBindCallStub<false>(cold, data));
  ADD(immutableBindCallStub,  emitBindCallStub<true>(cold, data));
  ADD(fcallArrayHelper,       emitFCallArrayHelper(hot(), frozen, data, *this));

  ADD(decRefGeneric,  emitDecRefGeneric(cold, data));

  ADD(callToExit,         emitCallToExit(main, data, *this));
  ADD(throwSwitchMode,    emitThrowSwitchMode(frozen, data));

  ADD(handlePrimeCacheInit,
      emitHelperThunk(code, cold, data,
                      MethodCache::handlePrimeCacheInit<false>));
  ADD(handlePrimeCacheInitFatal,
      emitHelperThunk(code, cold, data,
                      MethodCache::handlePrimeCacheInit<true>));
  ADD(handleSlowPath,
      emitHelperThunk(code, main, data,
                      MethodCache::handleSlowPath<false>));
  ADD(handleSlowPathFatal,
      emitHelperThunk(code, main, data,
                      MethodCache::handleSlowPath<true>));

#undef ADD

  add("freeLocalsHelpers",
      emitFreeLocalsHelpers(hot(), data, *this), code, dbg);

  ResumeHelperEntryPoints rh;
  add("resumeInterpHelpers",
      emitResumeInterpHelpers(hot(), data, *this, rh),
      code, dbg);
  emitInterpOneCFHelpers(cold, data, *this, rh, code, dbg);
}

///////////////////////////////////////////////////////////////////////////////

TCA UniqueStubs::add(const char* name, TCA start,
                     const CodeCache& code, Debug::DebugInfo& dbg) {
  if (!code.isValidCodeAddress(start)) return start;

  auto& cb = code.blockFor(start);
  auto const end = cb.frontier();

  FTRACE(1, "unique stub: {} @ {} -- {:4} bytes: {}\n",
         cb.name(),
         static_cast<void*>(start),
         static_cast<size_t>(end - start),
         name);

  ONTRACE(2,
          [&]{
            std::ostringstream os;
            disasmRange(os, start, end);
            FTRACE(2, "{}\n", os.str());
          }()
         );

  if (!RuntimeOption::EvalJitNoGdb) {
    dbg.recordStub(Debug::TCRange(start, end, &cb == &code.cold()),
                   folly::sformat("HHVM::{}", name));
  }
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportHelperToVtune(folly::sformat("HHVM::{}", name).c_str(),
                        start,
                        end);
  }
  if (RuntimeOption::EvalPerfPidMap) {
    dbg.recordPerfMap(Debug::TCRange(start, end, &cb == &code.cold()),
                      SrcKey{},
                      nullptr,
                      false,
                      false,
                      folly::sformat("HHVM::{}", name));
  }

  auto const newStub = StubRange{name, start, end};
  auto lower = std::lower_bound(m_ranges.begin(), m_ranges.end(), newStub);

  // We assume ranges are non-overlapping.
  assertx(lower == m_ranges.end() || newStub.end <= lower->start);
  assertx(lower == m_ranges.begin() || (lower - 1)->end <= newStub.start);
  m_ranges.insert(lower, newStub);
  return start;
}

std::string UniqueStubs::describe(TCA address) const {
  auto raw = [address] { return folly::sformat("{}", address); };
  if (m_ranges.empty()) return raw();

  auto const dummy = StubRange{"", address, nullptr};
  auto lower = std::upper_bound(m_ranges.begin(), m_ranges.end(), dummy);
  if (lower == m_ranges.begin()) return raw();

  --lower;
  if (lower->contains(address)) {
    return folly::sformat("{}+{:#x}", lower->name, address - lower->start);
  }
  return raw();
}

///////////////////////////////////////////////////////////////////////////////

RegSet interp_one_cf_regs() {
  return vm_regs_with_sp() | rarg(2);
}

void emitInterpReq(Vout& v, SrcKey sk, FPInvOffset spOff) {
  if (!sk.resumed()) {
    v << lea{rvmfp()[-cellsToBytes(spOff.offset)], rvmsp()};
  }
  v << copy{v.cns(sk.pc()), rarg(0)};
  v << jmpi{tc::ustubs().interpHelper, arg_regs(1)};
}

///////////////////////////////////////////////////////////////////////////////

void enterTCImpl(TCA start, ActRec* stashedAR) {
  // We have to force C++ to spill anything that might be in a callee-saved
  // register (aside from rvmfp()), since enterTCHelper does not save them.
  CALLEE_SAVED_BARRIER();
  auto& regs = vmRegsUnsafe();
  tc::ustubs().enterTCHelper(regs.stack.top(), regs.fp, start,
                              vmFirstAR(), rds::tl_base, stashedAR);
  CALLEE_SAVED_BARRIER();
}

///////////////////////////////////////////////////////////////////////////////

}}
