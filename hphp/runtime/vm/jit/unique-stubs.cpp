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
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/packed-array.h"
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
#include "hphp/runtime/vm/resumable.h"
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
void loadVmfp(Vout& v) { v << load{rvmtl()[rds::kVmfpOff], rvmfp()}; }
void loadVmsp(Vout& v) { v << load{rvmtl()[rds::kVmspOff], rvmsp()}; }
void loadVMRegs(Vout& v) { loadVmfp(v); loadVmsp(v); }
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
  always_assert(us.endCatchStublogueHelper);
  v << landingpad{};
  gen(v);
  v << jmpi{us.endCatchStublogueHelper, vm_regs_no_sp()};
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

bool fcallHelper(CallFlags callFlags, Func* func, int32_t numArgs, void* ctx,
                 TCA savedRip) {
  assert_native_stack_aligned();
  assertx(numArgs <= func->numNonVariadicParams() + 1);

  // The stub already synced the vmfp() and vmsp() registers, but the vmsp() is
  // pointing to the space reserved for the ActRec. This space together with
  // the space reserved for output args may contain random garbage, as the Call
  // opcode promises to kill/write this space, so store-elim is allowed to
  // optimize away these writes. So, write NullUninits over this space, adjust
  // vmsp() and set vmpc()/vmJitReturnAddr().
  assertx(tl_regState == VMRegState::DIRTY);
  auto& unsafeRegs = vmRegsUnsafe();
  auto const calleeFP = unsafeRegs.stack.top();
  for (auto i = kNumActRecCells + func->numInOutParams(); i--;) {
    tvWriteUninit(calleeFP[i]);
  }

  unsafeRegs.stack.nalloc(numArgs + (callFlags.hasGenerics() ? 1 : 0));
  unsafeRegs.pc = unsafeRegs.fp->unit()->at(
    unsafeRegs.fp->func()->base() + callFlags.callOffset());
  unsafeRegs.jitReturnAddr = savedRip;
  tl_regState = VMRegState::CLEAN;

  // Check for stack overflow in the same place func prologues make their
  // StackCheck::Early check (see irgen-func-prologue.cpp).
  if (checkCalleeStackOverflow(calleeFP, func)) {
    throw_stack_overflow();
  }

  // Write ActRec.
  ActRec* ar = reinterpret_cast<ActRec*>(calleeFP);
  ar->m_sfp = vmfp();
  ar->setJitReturn(savedRip);
  ar->m_func = func;
  ar->m_callOffAndFlags = ActRec::encodeCallOffsetAndFlags(
    callFlags.callOffset(),
    callFlags.asyncEagerReturn() ? (1 << ActRec::AsyncEagerRet) : 0
  );
  ar->setNumArgs(numArgs);
  ar->m_thisUnsafe = reinterpret_cast<ObjectData*>(ctx);
  ar->trashVarEnv();

  // If doFCall() returns false, we've been asked to skip the function body due
  // to fb_intercept, so indicate that via the return value. If we did not
  // throw, we are going to reenter TC, so set the registers as dirty.
  auto const hasUnpack = numArgs == func->numNonVariadicParams() + 1;
  if (hasUnpack) --numArgs;
  auto const notIntercepted = doFCall(ar, numArgs, hasUnpack, callFlags);
  tl_regState = VMRegState::DIRTY;
  return notIntercepted;
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFuncPrologueRedispatch(CodeBlock& cb, DataBlock& data) {
  alignCacheLine(cb);

  return vwrap(cb, data, [] (Vout& v) {
    auto const func = v.makeReg();
    auto const numArgs = v.makeReg();
    v << copy{r_php_call_func(), func};
    v << copy{r_php_call_num_args(), numArgs};

    auto const paramCounts = v.makeReg();
    auto const paramCountsMinusOne = v.makeReg();
    auto const numNonVariadicParams = v.makeReg();
    v << loadl{func[Func::paramCountsOff()], paramCounts};
    v << decl{paramCounts, paramCountsMinusOne, v.makeReg()};
    v << shrli{1, paramCountsMinusOne, numNonVariadicParams, v.makeReg()};

    auto const sf = v.makeReg();
    v << cmpl{numNonVariadicParams, numArgs, sf};

    auto const pTabOff = safe_cast<int32_t>(Func::prologueTableOff());
    auto const ptrSize = safe_cast<int32_t>(sizeof(LowPtr<uint8_t>));

    ifThen(v, CC_LE, sf, [&] (Vout& v) {
      // Fast path (numArgs <= numNonVariadicParams). Call the numArgs prologue.
      auto const dest = v.makeReg();
      emitLdLowPtr(v, func[numArgs * ptrSize + pTabOff],
                   dest, sizeof(LowPtr<uint8_t>));
      v << jmpr{dest, php_call_regs(true)};
    });

    // Slow path: we passed more arguments than declared. Need to pack the extra
    // args and dispatch to the "too many arguments" prologue.

    // We are going to do a C++ call, need to ensure native stack is aligned.
    v << stublogue{false};

    // Figure out the range of stack values to pack.
    auto const stackTopOff = v.makeReg();
    auto const stackTopPtr = v.makeReg();
    auto const numToPack = v.makeReg();
    assertx(sizeof(TypedValue) == (1 << 4));
    v << shlqi{4, numArgs, stackTopOff, v.makeReg()};
    v << subq{stackTopOff, rvmsp(), stackTopPtr, v.makeReg()};
    v << subl{numNonVariadicParams, numArgs, numToPack, v.makeReg()};

    // Pack the extra args into a vec/varray.
    auto const helper = [](uint32_t count,
                           TypedValue* values) -> ArrayData* {
      // TODO(jgriego) surely we have a better way of instrumenting this ...
      ARRPROV_USE_RUNTIME_LOCATION();
      if (RO::EvalHackArrDVArrs) {
        return PackedArray::MakeVec(count, values);
      } else {
        return PackedArray::MakeVArray(count, values);
      }
    };
    auto const packedArr = v.makeReg();
    {
      auto const save = r_php_call_flags()|r_php_call_func()|r_php_call_ctx();
      PhysRegSaver prs{v, save};
      v << vcall{
        CallSpec::direct(static_cast<ArrayData* (*)(uint32_t, TypedValue*)>(helper)),
        v.makeVcallArgs({{numToPack, stackTopPtr}}),
        v.makeTuple({packedArr}),
        Fixup{},
        DestType::SSA
      };
    }

    // Calculate the new number of arguments.
    auto const numNewArgs32 = v.makeReg();
    auto const numNewArgs = v.makeReg();
    v << incl{numNonVariadicParams, numNewArgs32, v.makeReg()};
    v << movzlq{numNewArgs32, numNewArgs};

    // Figure out where to store the packed array.
    auto const unpackCellOff = v.makeReg();
    auto const unpackCellPtr = v.makeReg();
    assertx(sizeof(TypedValue) == (1 << 4));
    v << shlqi{4, numNewArgs, unpackCellOff, v.makeReg()};
    v << subq{unpackCellOff, rvmsp(), unpackCellPtr, v.makeReg()};

    // Store it.
    auto const type = RuntimeOption::EvalHackArrDVArrs
      ? DataType::Vec
      : DataType::Array;
    v << store{packedArr, unpackCellPtr + TVOFF(m_data)};
    v << storeb{v.cns(type), unpackCellPtr + TVOFF(m_type)};

    // Move generics to the correct place.
    auto const generics = v.makeReg();
    v << loadups{stackTopPtr[-int32_t(sizeof(TypedValue))], generics};
    v << storeups{generics, unpackCellPtr[-int32_t(sizeof(TypedValue))]};

    // Restore all inputs.
    v << copy{numNewArgs, r_php_call_num_args()};

    // Call the numNonVariadicParams + 1 prologue.
    auto const dest = v.makeReg();
    emitLdLowPtr(v, Vreg(r_php_call_func())[numNewArgs * ptrSize + pTabOff],
                 dest, sizeof(LowPtr<uint8_t>));
    v << tailcallstubr{dest, php_call_regs(true)};
  });
}

TCA emitFCallHelperThunk(CodeBlock& main, CodeBlock& cold, DataBlock& data,
                         UniqueStubs& us) {
  alignJmpTarget(main);
  CGMeta meta;

  auto const start = vwrap2(main, cold, data, meta, [&] (Vout& v, Vout& vc) {
    v << stublogue{false};

    // Save all inputs.
    auto const func = v.makeReg();
    auto const numArgs = v.makeReg();
    v << copy{r_php_call_func(), func};
    v << copy{r_php_call_num_args(), numArgs};

    // Try to JIT the prologue first.
    auto const target = v.makeReg();
    {
      PhysRegSaver prs{v, r_php_call_flags()|r_php_call_ctx()};
      v << vcall{
        CallSpec::direct(mcgen::getFuncPrologue),
        v.makeVcallArgs({{func, numArgs}}),
        v.makeTuple({target}),
        Fixup{},
        DestType::SSA
      };
    }

    auto const targetSF = v.makeReg();
    v << testq{target, target, targetSF};
    ifThen(v, CC_NZ, targetSF, [&] (Vout& v) {
      // Restore all inputs and call the resolved prologue.
      v << copy{func, r_php_call_func()};
      v << copy{numArgs, r_php_call_num_args()};
      v << tailcallstubr{target, php_call_regs(true)};
    });

    auto const flags = v.makeReg();
    auto const ctx = v.makeReg();
    auto const savedRip = v.makeReg();
    v << copy{r_php_call_flags(), flags};
    v << copy{r_php_call_ctx(), ctx};
    v << loadstubret{savedRip};

    // Call C++ helper to perform the equivalent of the func prologue logic.
    auto const done = v.makeBlock();
    auto const ctch = vc.makeBlock();
    auto const notIntercepted = v.makeReg();
    storeVMRegs(v);
    v << vinvoke{
      CallSpec::direct(fcallHelper),
      v.makeVcallArgs({{flags, func, numArgs, ctx, savedRip}}),
      v.makeTuple({notIntercepted}),
      {done, ctch},
      Fixup{},
      DestType::SSA
    };

    vc = ctch;
    emitStubCatch(vc, us, [] (Vout& v) { loadVmfp(v); });

    v = done;

    auto const notInterceptedSF = v.makeReg();
    v << testb{notIntercepted, notIntercepted, notInterceptedSF};

    unlikelyIfThen(v, vc, CC_Z, notInterceptedSF, [&] (Vout& v) {
      // The callee was intercepted and should be skipped. In that case, sync
      // the registers and return to the caller.
      loadVMRegs(v);
      loadReturnRegs(v);
      v << stubret{php_return_regs(), false};
    });

    // Use resumeHelper stub to resume the execution. It assumes phplogue{}
    // context, so convert the context first. Note that the VM registers are
    // not synced yet, but that's fine as resumeHelper operates on TLS data.
    v << stubtophp{};
    v << jmpi{us.resumeHelper, RegSet(rvmtl())};
  });

  meta.process(nullptr);
  return start;
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
    emitStubCatch(vc, us, [] (Vout& v) { loadVmfp(v); });

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

///////////////////////////////////////////////////////////////////////////////

TCA emitBindCallStub(CodeBlock& cb, DataBlock& data) {
  return vwrap(cb, data, [] (Vout& v) {
    v << stublogue{false};

    // Save all inputs.
    auto const func = v.makeReg();
    auto const numArgs = v.makeReg();
    v << copy{r_php_call_func(), func};
    v << copy{r_php_call_num_args(), numArgs};

    // Reconstruct the address of the call from the saved RIP.
    auto const savedRip = v.makeReg();
    auto const toSmash = v.makeReg();
    auto const callLen = safe_cast<int>(smashableCallLen());
    v << loadstubret{savedRip};
    v << subqi{callLen, savedRip, toSmash, v.makeReg()};

    // Call C++ helper to bind the call.
    auto const target = v.makeReg();
    {
      PhysRegSaver prs{v, r_php_call_flags()|r_php_call_ctx()};
      v << vcall{
        CallSpec::direct(svcreq::handleBindCall),
        v.makeVcallArgs({{toSmash, func, numArgs}}),
        v.makeTuple({target}),
        Fixup{},
        DestType::SSA
      };
    }

    // Restore all inputs and call the resolved prologue.
    v << copy{func, r_php_call_func()};
    v << copy{numArgs, r_php_call_num_args()};
    v << tailcallstubr{target, php_call_regs(true)};
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
    bool (*helper)(PC, int32_t, CallFlags, void*) = &doFCallUnpackTC;

    v << vinvoke{
      CallSpec::direct(helper),
      v.makeVcallArgs({{pc, rarg(1), rarg(2), retAddr}}),
      v.makeTuple({should_continue}),
      {done, ctch},
      Fixup{},
      DestType::SSA
    };
    vc = ctch;
    emitStubCatch(vc, us, [] (Vout& v) { loadVmfp(v); });

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
    loadVmfp(v);

    // If true was returned, we're calling the callee, so undo the stublogue{}
    // and convert to a phplogue{}.
    v << stubtophp{};

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
  TCA resumeHelper;
  TCA handleResume;
  TCA reenterTC;
};

ResumeHelperEntryPoints emitResumeHelpers(CodeBlock& cb, DataBlock& data) {
  ResumeHelperEntryPoints rh;

  rh.resumeHelper = vwrap(cb, data, [] (Vout& v) {
    v << ldimmb{0, rarg(0)};
    v << fallthru{arg_regs(1)};
  });

  rh.handleResume = vwrap(cb, data, [] (Vout& v) {
    loadVmfp(v);

    auto const handler = reinterpret_cast<TCA>(svcreq::handleResume);
    v << call{handler, arg_regs(1)};
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

  us.resumeHelper = rh.resumeHelper;

  us.interpHelper = vwrap(cb, data, [] (Vout& v) {
    v << store{rarg(0), rvmtl()[rds::kVmpcOff]};
  });
  us.interpHelperSyncedPC = vwrap(cb, data, [&] (Vout& v) {
    storeVMRegs(v);
    v << ldimmb{1, rarg(0)};
    v << jmpi{rh.handleResume, RegSet(rarg(0))};
  });

  return us.resumeHelper;
}

TCA emitInterpOneCFHelper(CodeBlock& cb, DataBlock& data, Op op,
                          const ResumeHelperEntryPoints& rh) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    v << copyargs{
      v.makeTuple({rvmfp(), rvmsp()}),
      v.makeTuple({rarg(0), rarg(1)})
    };
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

    // Store the return value on the top of the eval stack.  Whenever we get to
    // enterTCExit, we're semantically executing some PHP construct that sends
    // a return value out of a function (either a RetC, or a Yield, or an Await
    // that's suspending, etc), and moreover, we must be executing the return
    // that leaves this level of VM reentry (i.e. the only way we get here is
    // by coming from the callToExit stub or by a phpret{} or leavetc{} that
    // undoes the resumetc{} in enterTCHelper).
    //
    // Either way, we have a live PHP return value in the return registers,
    // which we need to put on the top of the evaluation stack.
    storeReturnRegs(v);

    // Restore the registers that need to be saved across enterTC
    auto cross_jit = cross_jit_save();
    if (!(cross_jit.size() & 1)) alignNativeStack(v, true);
    cross_jit.forEachR(
      [&] (PhysReg r) {
        v << pop{r};
      }
    );

    // Perform a native return.
    //
    // On PPC64, as there is no new frame created when entering the VM, the FP
    // must not be restored.
    v << stubret{cross_jit, arch() != Arch::PPC64};
  });
}

TCA emitEnterTCHelper(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  alignCacheLine(cb);

  auto const start    = rarg(0);
  auto const fp       = rarg(1);
  auto const tl       = rarg(2);
  auto const sp       = rarg(3);
  auto const firstAR  = rarg(4);

  return vwrap2(cb, cb, data, [&] (Vout& v, Vout& vc) {
    // Architecture-specific setup for entering the TC.
    v << inittc{};

    // Native func prologue.
    v << stublogue{arch() != Arch::PPC64};

    // Set up linkage with the top VM frame in this nesting.
    v << store{rsp(), firstAR[AROFF(m_sfp)]};

    // Save the registers that need to be saved across enterTC
    auto cross_jit = cross_jit_save();
    cross_jit.forEach(
      [&] (PhysReg r) {
        v << push{r};
      }
    );

    // Set up the VM registers.
    v << copy{fp, rvmfp()};
    v << copy{tl, rvmtl()};
    v << copy{sp, rvmsp()};

    // Unalign the native stack.
    if (!(cross_jit.size() & 1)) alignNativeStack(v, false);

    v << resumetc{start, us.enterTCExit, vm_regs_with_sp()};
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

  CGMeta meta;

  us.resumeCPPUnwind = vwrap(cb, data, meta, [&] (Vout& v) {
    static_assert(sizeof(tl_regState) == 8,
                  "The following store must match the size of tl_regState.");
    auto const regstate = emitTLSAddr(v, tls_datum(tl_regState));
    v << storeqi{static_cast<int32_t>(VMRegState::CLEAN), regstate};
    // We were about to return to callToExit, so drop the return
    // address, then do what enterTCExit would have done.
    alignNativeStack(v, true);
    auto cross_jit = cross_jit_save();
    if (!(cross_jit.size() & 1)) alignNativeStack(v, true);
    cross_jit.forEachR(
      [&] (PhysReg r) {
        v << pop{r};
      }
    );
    v << call{
      TCA(__cxxabiv1::__cxa_rethrow),
      arg_regs(0) | cross_jit,
      &us.endCatchHelperPast
    };
    v << trap{TRAP_REASON};
  });
  meta.process(nullptr);

  alignJmpTarget(cb);

  auto const teardownEnter = vwrap(cb, data, [&] (Vout& v) {
    v << copy{v.cns(true), rarg(1)};
    v << fallthru{RegSet{} | rarg(1)};
  });

  auto const body = vwrap(cb, data, [&] (Vout& v) {
    // Normal end catch situation: call back to tc_unwind_resume, which returns
    // the catch trace (or null) in the first return register, and the new vmfp
    // in the second.
    v << copy{rvmfp(), rarg(0)};
    v << call{TCA(tc_unwind_resume), arg_regs(2)};
    v << copy{rret(1), rvmfp()};

    auto const done = v.makeBlock();
    auto const sf = v.makeReg();

    v << testq{rret(0), rret(0), sf};
    v << jcci{CC_Z, sf, done, us.resumeCPPUnwind};
    v = done;

    v << jmpr{rret(0), vm_regs_with_sp()};
  });

  us.endCatchTeardownThisHelper = vwrap(cb, data, meta, [&] (Vout& v) {
    auto const thiz = v.makeReg();
    v << load{rvmfp()[AROFF(m_thisUnsafe)], thiz};
    emitDecRefWorkObj(v, thiz, TRAP_REASON);
    v << fallthru{RegSet{}};
  });

  us.endCatchSkipTeardownHelper = vwrap(cb, data, meta, [&] (Vout& v) {
    if (debug) {
      // We can't call trashVarEnv() (rather, write kTrashedVarEnv to the slot)
      // here because the punned tailFrameIds field is needed for backtracing.
      emitImmStoreq(v, ActRec::kTrashedThisSlot, rvmfp()[AROFF(m_thisUnsafe)]);
    }
    v << copy{v.cns(false), rarg(1)};
    v << jmpi{body, RegSet{} | rarg(1)};
  });

  return teardownEnter;
}

TCA emitEndCatchStublogueHelper(CodeBlock& cb, DataBlock& data,
                                UniqueStubs& us) {
  alignJmpTarget(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    // End catch situation in stublogue context: pop the native frame and
    // pass the curent rvmfp() and saved RIP from the native frame to
    // tc_unwind_resume_stublogue(),which returns the catch trace (or null)
    // in the first return register, and the new vmfp in the second.
    v << copy{rvmfp(), rarg(0)};
    v << stubunwind{rarg(1)};
    v << call{TCA(tc_unwind_resume_stublogue), arg_regs(2)};
    v << copy{rret(1), rvmfp()};

    auto const done = v.makeBlock();
    auto const sf = v.makeReg();

    v << testq{rret(0), rret(0), sf};
    v << jcci{CC_Z, sf, done, us.resumeCPPUnwind};
    v = done;

    v << jmpr{rret(0), vm_regs_no_sp()};
  });
}

TCA emitUnwinderAsyncRet(CodeBlock& cb, DataBlock& data) {
  alignCacheLine(cb);
  alignJmpTarget(cb);
  return vwrap(cb, data, [&] (Vout& v) {
    v << load{rvmtl()[unwinderFSWHOff()], rret_data()};
    v << movzbq{v.cns(KindOfObject), rret_type()};
    v << pushm{rvmtl()[unwinderSavedRipOff()]};
    v << load{rvmtl()[rds::kVmspOff], rvmsp()};
    v << ret{php_return_regs()};
  });
}

TCA emitUnwinderAsyncNullRet(CodeBlock& cb, DataBlock& data) {
  alignCacheLine(cb);
  alignJmpTarget(cb);
  return vwrap(cb, data, [&] (Vout& v) {
    v << movzbq{v.cns(KindOfUninit), rret_type()};
    v << pushm{rvmtl()[unwinderSavedRipOff()]};
    v << load{rvmtl()[rds::kVmspOff], rvmsp()};
    v << ret{php_return_regs()};
  });
}

namespace {

[[noreturn]] static void throw_exception_while_unwinding() {
  assert_native_stack_aligned();
  assertx(g_unwind_rds->exn.left());
  throw req::root<Object>(Object::attach(g_unwind_rds->exn.left()));
}

} // namespace

TCA emitThrowExceptionWhileUnwinding(CodeBlock& cb, DataBlock& data) {
  alignCacheLine(cb);
  alignJmpTarget(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    // The saved rip is the caller is callToExit. We want to skip over
    // callToExit and enterTCExit, so move the stack pointer and set the rbp
    // to be the next frame in the rbp chain.
    alignNativeStack(v, true);
    auto cross_jit = cross_jit_save();
    if (!(cross_jit.size() & 1)) alignNativeStack(v, true);
    cross_jit.forEachR(
      [&] (PhysReg r) {
        v << pop{r};
      }
    );
    v << load{rsp()[0], rvmfp()};
    v << tailcallstub{TCA(throw_exception_while_unwinding), cross_jit};
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

  // These guys are required by a number of other stubs.
  ADD(handleSRHelper, hotView(), emitHandleSRHelper(hot(), data));
  ADD(endCatchHelper, hotView(), emitEndCatchHelper(hot(), data, *this));
  ADD(endCatchStublogueHelper,
      hotView(),
      emitEndCatchStublogueHelper(hot(), data, *this));
  ADD(unwinderAsyncRet, hotView(), emitUnwinderAsyncRet(hot(), data));
  ADD(unwinderAsyncNullRet, hotView(), emitUnwinderAsyncNullRet(hot(), data));
  ADD(throwExceptionWhileUnwinding,
      hotView(),
      emitThrowExceptionWhileUnwinding(hot(), data));

  ADD(funcPrologueRedispatch,
      hotView(),
      emitFuncPrologueRedispatch(hot(), data));
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

  ADD(immutableBindCallStub, view, emitBindCallStub(cold, data));
  ADD(fcallUnpackHelper,
      hotView(),
      emitFCallUnpackHelper(hot(), cold, data, *this));

  ADD(decRefGeneric,  hotView(), emitDecRefGeneric(hot(), data));

  ADD(callToExit,         hotView(), emitCallToExit(hot(), data, *this));

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

  ADD(fcallHelperThunk,
      view,
      emitFCallHelperThunk(cold, frozen, data, *this));
#undef ADD

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

}}
