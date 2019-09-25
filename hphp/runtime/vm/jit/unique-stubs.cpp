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
#include "hphp/runtime/vm/call-flags.h"
#include "hphp/runtime/vm/cti.h"
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
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs-arm.h"
#include "hphp/runtime/vm/jit/unique-stubs-ppc64.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/runtime/ext/asio/ext_async-generator.h"
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
  if (RuntimeOption::EvalJitAlignUniqueStubs) {
    align(cb, nullptr, Alignment::JmpTarget, AlignContext::Dead);
  }
}

void alignCacheLine(CodeBlock& cb) {
  if (RuntimeOption::EvalJitAlignUniqueStubs) {
    align(cb, nullptr, Alignment::CacheLine, AlignContext::Dead);
  }
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
    CallSpec::direct(helper, nullptr),
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
  auto const args = gen(v);
  v << stubunwind{};
  v << jmpi{us.endCatchHelper, args};
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFreeLocalsHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  alignCacheLine(cb);
  return ARCH_SWITCH_CALL(emitFreeLocalsHelpers, cb, data, us);
}

TCA emitCallToExit(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  alignJmpTarget(cb);
  return ARCH_SWITCH_CALL(emitCallToExit, cb, data, us);
}

///////////////////////////////////////////////////////////////////////////////

struct FCallHelperRet {
  TCA destAddr;
  TCA retAddr;
};

FCallHelperRet fcallHelper(CallFlags callFlags, ActRec* ar) {
  assert_native_stack_aligned();
  assertx(!ar->resumed());

  if (LIKELY(!RuntimeOption::EvalFailJitPrologs)) {
    auto const tca = mcgen::getFuncPrologue(
      const_cast<Func*>(ar->func()),
      ar->numArgs()
    );
    if (tca) return { tca, nullptr };
  }

  // Check for stack overflow in the same place func prologues make their
  // StackCheck::Early check (see irgen-func-prologue.cpp).  This handler also
  // cleans and syncs vmRegs for us.
  if (checkCalleeStackOverflow(ar)) handleStackOverflow(ar);

  // If doFCall indicates that the function was intercepted and should be
  // skipped, it will have already torn down the callee's frame. So, we need to
  // save the return value thats in it.
  auto const retAddr = (TCA)ar->m_savedRip;

  try {
    VMRegAnchor _(callFlags, ar);
    if (doFCall(ar, ar->numArgs(), false, callFlags.hasGenerics())) {
      return { tc::ustubs().resumeHelperRet, nullptr };
    }
    // We've been asked to skip the function body (fb_intercept).  The vmregs
    // have already been fixed; indicate this with a nullptr return.
    return { nullptr, retAddr };
  } catch (...) {
    // The VMRegAnchor above took care of us, but we need to tell the unwinder
    // (since ~VMRegAnchor() will have reset tl_regState).
    tl_regState = VMRegState::CLEAN;
    throw;
  }
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFuncPrologueRedispatch(CodeBlock& cb, DataBlock& data) {
  alignCacheLine(cb);

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
      v << jmpr{dest, php_call_regs()};
    });

    auto const nargs = v.makeReg();
    v << movzlq{argc, nargs};

    auto const dest = v.makeReg();
    emitLdLowPtr(v, func[nargs * ptrSize + pTabOff],
                 dest, sizeof(LowPtr<uint8_t>));
    v << jmpr{dest, php_call_regs()};
  });
}

TCA emitFCallHelperThunk(CodeBlock& main, CodeBlock& cold, DataBlock& data) {
  alignJmpTarget(main);

  return vwrap2(main, cold, data, [] (Vout& v, Vout& vc) {
    v << phplogue{rvmfp()};

    // fcallHelper asserts native stack alignment for us.
    FCallHelperRet (*helper)(CallFlags, ActRec*) = &fcallHelper;
    auto const callFlags = v.makeReg();
    auto const dest = v.makeReg();
    auto const saved_rip = v.makeReg();
    v << copy{r_php_call_flags(), callFlags};
    v << vcall{
      CallSpec::direct(helper, nullptr),
      v.makeVcallArgs({{callFlags, rvmfp()}}),
      v.makeTuple({dest, saved_rip}),
      Fixup{},
      DestType::SSA
    };

    // Clobber rvmsp in debug builds.
    if (debug) v << copy{v.cns(0x1), rvmsp()};

    auto const sf = v.makeReg();
    v << testq{dest, dest, sf};

    unlikelyIfThen(v, vc, CC_Z, sf, [&] (Vout& v) {
      // A nullptr dest means the callee was intercepted and should be
      // skipped. In that case, saved_rip will contain the return address that
      // was in the callee's ActRec before it was torn down by the intercept.
      loadVMRegs(v);
      loadReturnRegs(v);

      // Return to the caller. This unbalances the return stack buffer, but if
      // we're intercepting, we probably don't care.
      v << jmpr{saved_rip, php_return_regs()};
    });

    // Jump to the func prologue.
    v << copy{callFlags, r_php_call_flags()};
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
  alignCacheLine(main);

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
      return rsp() | rvmfp();
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
      v << jmpr{saved_rip, php_return_regs()};
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
    emitStubCatch(vc, us, [](Vout&) { return RegSet{}; });

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
  auto const callOff = v.makeReg();

  v << loadl{ar[AROFF(m_callOff)], callOff};
  v << storel{callOff, rvmtl()[unwinderDebuggerCallOffOff()]};
  v << store{rvmsp(), rvmtl()[unwinderDebuggerReturnSPOff()]};

  auto const ret = v.makeReg();
  v << simplecall(v, unstashDebuggerCatch, ar, ret);

  v << jmpr{ret};
}

TCA emitInterpRet(CodeBlock& cb, DataBlock& data) {
  alignCacheLine(cb);

  auto const start = vwrap(cb, data, [] (Vout& v) {
    // Sync return regs before calling native assert function.
    storeReturnRegs(v);
    assertNativeStackAligned(v);

    v << lea{rvmsp()[-kArRetOff], r_svcreq_arg(0)};
    v << copy{rvmfp(), r_svcreq_arg(1)};
    v << fallthru{r_svcreq_arg(0) | r_svcreq_arg(1)};
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
    v << fallthru{r_svcreq_arg(0) | r_svcreq_arg(1)};
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

TCA emitBindCallStub(CodeBlock& cb, DataBlock& data) {
  return vwrap(cb, data, [] (Vout& v) {
    v << phplogue{rvmfp()};

    auto args = VregList { v.makeReg(), v.makeReg() };

    // Reconstruct the address of the call from the saved RIP.
    auto const callFlags = v.makeReg();
    auto const savedRIP = v.makeReg();
    auto const callLen = safe_cast<int>(smashableCallLen());
    v << copy{r_php_call_flags(), callFlags};
    v << load{rvmfp()[AROFF(m_savedRip)], savedRIP};
    v << subqi{callLen, savedRIP, args[0], v.makeReg()};

    v << copy{rvmfp(), args[1]};

    auto const ret = v.makeReg();

    v << vcall{
      CallSpec::direct(svcreq::handleBindCall),
      v.makeVcallArgs({args}),
      v.makeTuple({ret}),
      Fixup{},
      DestType::SSA
    };

    v << copy{callFlags, r_php_call_flags()};
    v << tailcallphp{ret, rvmfp(), php_call_regs()};
  });
}

TCA emitFCallUnpackHelper(CodeBlock& main, CodeBlock& cold,
                          DataBlock& data, UniqueStubs& us) {
  alignCacheLine(main);

  CGMeta meta;

  auto const ret = vwrap2(main, cold, data, meta, [&] (Vout& v, Vout& vc) {
    // We reach fcallUnpackHelper in the same context as a func prologue, so
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

    // Convert offsets into PCs, and sync the PC.
    v << addq{bc, rarg(0), pc, v.makeReg()};
    v << store{pc, rvmtl()[rds::kVmpcOff]};

    auto const retAddr = v.makeReg();
    v << loadstubret{retAddr};

    auto const done = v.makeBlock();
    auto const ctch = vc.makeBlock();
    auto const should_continue = v.makeReg();
    bool (*helper)(PC, int32_t, bool, void*) = &doFCallUnpackTC;

    v << vinvoke{
      CallSpec::direct(helper),
      v.makeVcallArgs({{pc, rarg(1), rarg(2), retAddr}}),
      v.makeTuple({should_continue}),
      {done, ctch},
      Fixup{},
      DestType::SSA
    };
    vc = ctch;
    emitStubCatch(
      vc, us,
      [] (Vout& v) {
        loadVMRegs(v);
        return php_return_regs();
      }
    );

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
      v << stubret{php_return_regs()};
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
    v << fallthru{RegSet{rarg(0)}};
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

    v << jmpr{target, php_return_regs()};
  });

  return rh;
}

TCA emitResumeInterpHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us,
                            ResumeHelperEntryPoints& rh) {
  alignCacheLine(cb);

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
                            CodeCache::View view,
                            const ResumeHelperEntryPoints& rh,
                            const CodeCache& code, Debug::DebugInfo& dbg) {
  alignJmpTarget(cb);

  auto const emit = [&] (Op op, const char* name) {
    tc::TransLocMaker maker{view};
    maker.markStart();
    auto const stub = emitInterpOneCFHelper(cb, data, op, rh);
    us.interpOneCFHelpers[op] = stub;
    us.add(name, code, stub, view, maker.markEnd().loc(), dbg);
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
  alignCacheLine(cb);

  auto const start = vwrap(cb, data, meta, [] (Vout& v) {
    v << vregrestrict{};
    auto const fullFrame = [&] {
      switch (arch()) {
        case Arch::ARM:
        case Arch::PPC64:
          return true;
        case Arch::X64:
          return false;
      }
      not_reached();
    }();
    v << stublogue{fullFrame};
    if (fullFrame) {
      v << copy{rsp(), rvmfp()};
    }

    auto const rdata = rarg(0);
    auto const rtype = rarg(1);

    auto const destroy = [&] (Vout& v) {
      // decRefGeneric is called via callfaststub, whose ABI claims that all
      // registers are preserved.  This is true in the fast path, but in the
      // slow path we need to manually save caller-saved registers.
      auto const callerSaved = abi().gpUnreserved - abi().calleeSaved;
      PhysRegSaver prs{v, callerSaved};

      // Since we've manually saved the caller saved registers, we can
      // use those for Vregs. We use the helper ABI for this stub
      // which only allows caller saved registers.
      assertx(callerSaved.contains(rdata));
      assertx(callerSaved.contains(rtype));
      assertx(
        (callerSaved & abi(CodeKind::Helper).gpUnreserved) ==
        abi(CodeKind::Helper).gpUnreserved
      );

      auto const dtor = lookupDestructor(v, rtype);
      v << callm{dtor, arg_regs(1)};

      if (!fullFrame) {
        // The stub frame's saved RIP is at %rsp[8] before we saved the
        // caller-saved registers.
        v << syncpoint{makeIndirectFixup(prs.dwordsPushed())};
      }
    };

    emitDecRefWork(v, v, rdata, destroy, false, TRAP_REASON);

    v << stubret{{}, fullFrame};
  }, CodeKind::Helper);

  meta.process(nullptr);
  return start;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void alignNativeStack(Vout& v, bool exit) {
  switch (arch()) {
    case Arch::X64:
    case Arch::PPC64:
      v << lea{rsp()[exit ? 8 : -8], rsp()};
      break;
    case Arch::ARM:
      break;
  }
}

}

TCA emitEnterTCExit(CodeBlock& cb, DataBlock& data, UniqueStubs& /*us*/) {
  alignCacheLine(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    // Eagerly save VM regs.
    storeVMRegs(v);

    // Realign the native stack.
    alignNativeStack(v, true);

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
  alignCacheLine(cb);

  auto const start    = rarg(0);
  auto const fp       = rarg(1);
  auto const tl       = rarg(2);
  auto const sp       = rarg(3);
#ifdef _MSC_VER
  auto const firstAR  = reg::r10;
#else
  auto const firstAR  = rarg(4);
#endif

  return vwrap2(cb, cb, data, [&] (Vout& v, Vout& vc) {
    // Architecture-specific setup for entering the TC.
    v << inittc{};

    // Native func prologue.
    v << stublogue{arch() != Arch::PPC64};

#ifdef _MSC_VER
    // Windows hates argument registers.
    v << load{rsp()[0x28], reg::r10};
#endif

    // Set up linkage with the top VM frame in this nesting.
    v << store{rsp(), firstAR[AROFF(m_sfp)]};

    // Set up the VM registers.
    v << copy{fp, rvmfp()};
    v << copy{tl, rvmtl()};
    v << copy{sp, rvmsp()};

    // Unalign the native stack.
    alignNativeStack(v, false);

    v << resumetc{start, us.enterTCExit, vm_regs_with_sp()};
  });
}

TCA
emitEnterTCAtPrologueHelper(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  alignCacheLine(cb);

  UNUSED auto const callFlags = rarg(0);
  auto const start     = rarg(1);
  auto const fp        = rarg(2);
  auto const tl        = rarg(3);
  assertx(callFlags == r_php_call_flags());

  return vwrap2(cb, cb, data, [&] (Vout& v, Vout& vc) {
    // Architecture-specific setup for entering the TC.
    v << inittc{};

    // Native func prologue.
    v << stublogue{arch() != Arch::PPC64};

    // Set up linkage with the top VM frame in this nesting.
    v << store{rsp(), fp[AROFF(m_sfp)]};

    // Set up the VM registers.
    v << copy{fp, rvmfp()};
    v << copy{tl, rvmtl()};

    // Unalign the native stack.
    alignNativeStack(v, false);

    // Set rvmfp() to the callee and call the prologue.
    v << calltc{start, rvmfp(), us.enterTCExit, php_call_regs()};
  });
}

TCA emitHandleSRHelper(CodeBlock& cb, DataBlock& data) {
  alignCacheLine(cb);

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

    v << jmpr{ret, php_return_regs()};
  });
}

///////////////////////////////////////////////////////////////////////////////

TCA emitEndCatchHelper(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  alignCacheLine(cb);

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
    v << trap{TRAP_REASON};
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
    v << trap{TRAP_REASON};
  });
}

///////////////////////////////////////////////////////////////////////////////

}

void UniqueStubs::emitAll(CodeCache& code, Debug::DebugInfo& dbg) {
  auto view = code.view();
  auto& main = view.main();
  auto& cold = view.cold();
  auto& frozen = view.frozen();
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
  ADD(enterTCExit,   hotView(), emitEnterTCExit(hot(), data, *this));
  enterTCHelper = decltype(enterTCHelper)(
    EMIT("enterTCHelper", view,
      [&] { return emitEnterTCHelper(main, data, *this); }));
  enterTCAtPrologueHelper = decltype(enterTCAtPrologueHelper)(
    EMIT("enterTCAtPrologueHelper", view,
      [&] { return emitEnterTCAtPrologueHelper(main, data, *this); }));

  // These guys are required by a number of other stubs.
  ADD(handleSRHelper, hotView(), emitHandleSRHelper(hot(), data));
  ADD(endCatchHelper, hotView(), emitEndCatchHelper(hot(), data, *this));
  ADD(unknownExceptionHandler,
      view,
      emitUnknownExceptionHandler(cold, data, *this));

  ADD(funcPrologueRedispatch,
      hotView(),
      emitFuncPrologueRedispatch(hot(), data));
  ADD(fcallHelperThunk,       view, emitFCallHelperThunk(cold, frozen, data));
  ADD(funcBodyHelperThunk,    view, emitFuncBodyHelperThunk(cold, data));
  ADD(functionEnterHelper,
      hotView(),
      emitFunctionEnterHelper(hot(), cold, data, *this));
  ADD(functionSurprisedOrStackOverflow,
      hotView(),
      emitFunctionSurprisedOrStackOverflow(hot(), cold, data, *this));

  ADD(retHelper, hotView(), emitInterpRet(hot(), data));
  ADD(genRetHelper, view, emitInterpGenRet<false>(cold, data));
  ADD(asyncGenRetHelper, hotView(), emitInterpGenRet<true>(hot(), data));
  ADD(retInlHelper, hotView(), emitInterpRet(hot(), data));
  ADD(debuggerRetHelper, view, emitDebuggerInterpRet(cold, data));
  ADD(debuggerGenRetHelper, view, emitDebuggerInterpGenRet<false>(cold, data));
  ADD(debuggerAsyncGenRetHelper,
      view,
      emitDebuggerInterpGenRet<true>(cold, data));

  ADD(immutableBindCallStub, view, emitBindCallStub(cold, data));
  ADD(fcallUnpackHelper,
      hotView(),
      emitFCallUnpackHelper(hot(), cold, data, *this));

  ADD(decRefGeneric,  hotView(), emitDecRefGeneric(hot(), data));

  ADD(callToExit,         hotView(), emitCallToExit(hot(), data, *this));
  ADD(throwSwitchMode,    view, emitThrowSwitchMode(frozen, data));
#undef ADD

  EMIT(
    "freeLocalsHelpers",
    hotView(),
    [&] { return emitFreeLocalsHelpers(hot(), data, *this); }
  );

  ResumeHelperEntryPoints rh;
  EMIT(
    "resumeInterpHelpers",
    hotView(),
    [&] { return emitResumeInterpHelpers(hot(), data, *this, rh); }
  );
  emitInterpOneCFHelpers(cold, data, *this, view, rh, code, dbg);

  emitAllResumable(code, dbg);
  if (cti_enabled()) compile_cti_stubs();
}

///////////////////////////////////////////////////////////////////////////////

void UniqueStubs::add(const char* name,
                      const CodeCache& code,
                      TCA mainStart,
                      CodeCache::View view,
                      TransLoc loc,
                      Debug::DebugInfo& dbg) {
  if (!code.isValidCodeAddress(mainStart)) return;

  auto const& startBlock = code.blockFor(mainStart);

  auto const process = [&] (const CodeBlock& cb, Address start, Address end) {
    if (start == end) return;

    // We may have inserted padding at the beginning, so adjust past it (using
    // the start address).
    if (&cb == &startBlock) start = mainStart;

    FTRACE(1, "unique stub: {} @ {} -- {:4} bytes: {}\n",
           cb.name(),
           static_cast<void*>(start),
           static_cast<size_t>(end - start),
           name);

    ONTRACE(2,
            [&]{
              std::ostringstream os;
              disasmRange(os, TransKind::Optimize, start, end);
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
  };
  process(view.main(), loc.mainStart(), loc.mainEnd());
  process(view.cold(), loc.coldCodeStart(), loc.coldEnd());
  if (&view.cold() != &view.frozen()) {
    process(view.frozen(), loc.frozenCodeStart(), loc.frozenEnd());
  }
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
  if (sk.resumeMode() == ResumeMode::None) {
    v << lea{rvmfp()[-cellsToBytes(spOff.offset)], rvmsp()};
  }
  v << copy{v.cns(sk.pc()), rarg(0)};
  v << jmpi{tc::ustubs().interpHelper, arg_regs(1)};
}

///////////////////////////////////////////////////////////////////////////////

// see T39604764: inlining-specific issue that might also affect GCC
#ifdef __clang__
NEVER_INLINE
#endif
void enterTCImpl(TCA start) {
  assert_flog(tc::isValidCodeAddress(start), "start = {} ; func = {} ({})\n",
              start, vmfp()->func(), vmfp()->func()->fullName());

  // We have to force C++ to spill anything that might be in a callee-saved
  // register (aside from rvmfp()), since enterTCHelper does not save them.
  CALLEE_SAVED_BARRIER();
  auto& regs = vmRegsUnsafe();
  tc::ustubs().enterTCHelper(start, regs.fp, rds::tl_base, regs.stack.top(),
                             vmFirstAR());
  CALLEE_SAVED_BARRIER();
}

// see T39604764: inlining-specific issue that might also affect GCC
#ifdef __clang__
NEVER_INLINE
#endif
void enterTCAtPrologueImpl(CallFlags callFlags, TCA start, ActRec* calleeAR) {
  assert_flog(tc::isValidCodeAddress(start), "start = {} ; func = {} ({})\n",
              start, calleeAR->func(), calleeAR->func()->fullName());

  // We have to force C++ to spill anything that might be in a callee-saved
  // register (aside from rvmfp()), since enterTCHelper does not save them.
  CALLEE_SAVED_BARRIER();
  tc::ustubs().enterTCAtPrologueHelper(callFlags, start, calleeAR,
                                       rds::tl_base);
  CALLEE_SAVED_BARRIER();
}

///////////////////////////////////////////////////////////////////////////////

}}
