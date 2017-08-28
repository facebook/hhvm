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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/minstr-state.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgCall(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const extra = inst->extra<Call>();
  auto const callee = extra->callee;
  auto const argc = extra->numParams;

  auto& v = vmain(env);
  auto& vc = vcold(env);
  auto const catchBlock = label(env, inst->taken());

  auto const calleeSP = sp[cellsToBytes(extra->spOffset.offset)];
  auto const calleeAR = calleeSP + cellsToBytes(argc);

  v << store{fp, calleeAR + AROFF(m_sfp)};
  v << storeli{safe_cast<int32_t>(extra->after), calleeAR + AROFF(m_soff)};

  if (extra->fcallAwait) {
    // This clobbers any flags that might have already been set on the callee
    // AR (e.g., by SpillFrame), but this is okay because there should never be
    // any conflicts; see the documentation in act-rec.h.
    auto const imm = static_cast<int32_t>(
      ActRec::encodeNumArgsAndFlags(argc, ActRec::Flags::IsFCallAwait)
    );
    v << storeli{imm, calleeAR + AROFF(m_numArgsAndFlags)};
  }

  auto const isNativeImplCall = callee &&
                                callee->builtinFuncPtr() &&
                                !callee->nativeFuncPtr() &&
                                argc == callee->numParams();
  if (isNativeImplCall) {
    // The assumption here is that for builtins, the generated func contains
    // only a single opcode (NativeImpl), and there are no non-argument locals.
    if (do_assert) {
      assertx(argc == callee->numLocals());
      assertx(callee->numIterators() == 0);
      assertx(callee->numClsRefSlots() == 0);

      auto addr = callee->getEntry();
      while (peek_op(addr) == Op::AssertRATL) {
        addr += instrLen(addr);
      }
      assertx(peek_op(addr) == Op::NativeImpl);
      assertx(addr + instrLen(addr) ==
              callee->unit()->entry() + callee->past());
    }

    v << store{v.cns(tc::ustubs().retHelper), calleeAR + AROFF(m_savedRip)};
    if (callee->attrs() & AttrMayUseVV) {
      v << storeqi{0, calleeAR + AROFF(m_invName)};
    }
    v << lea{calleeAR, rvmfp()};

    emitCheckSurpriseFlagsEnter(v, vc, fp, Fixup(0, argc), catchBlock);

    auto const builtinFuncPtr = callee->builtinFuncPtr();
    TRACE(2, "Calling builtin preClass %p func %p\n",
          callee->preClass(), builtinFuncPtr);

    // We sometimes call this while curFunc() isn't really the builtin, so make
    // sure to record the sync point as if we are inside the builtin.
    if (FixupMap::eagerRecord(callee)) {
      auto const syncSP = v.makeReg();
      v << lea{calleeSP, syncSP};
      emitEagerSyncPoint(v, callee->getEntry(), rvmtl(), rvmfp(), syncSP);
    }

    // Call the native implementation.  This will free the locals for us in the
    // normal case.  In the case where an exception is thrown, the VM unwinder
    // will handle it for us.
    auto const done = v.makeBlock();
    v << vinvoke{CallSpec::direct(builtinFuncPtr), v.makeVcallArgs({{rvmfp()}}),
                 v.makeTuple({}), {done, catchBlock}, Fixup(0, argc)};

    v = done;
    // The native implementation already put the return value on the stack for
    // us, and handled cleaning up the arguments.  We have to update the frame
    // pointer and the stack pointer, and load the return value into the return
    // register so the trace we are returning to has it where it expects.
    // TODO(#1273094): We should probably modify the actual builtins to return
    // values via registers using the C ABI and do a reg-to-reg move.
    loadTV(v, inst->dst(), dstLoc(env, inst, 0), rvmfp()[kArRetOff], true);
    v << load{rvmfp()[AROFF(m_sfp)], rvmfp()};
    emitRB(v, Trace::RBTypeFuncExit, callee->fullName()->data());
    return;
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << syncvmsp{v.cns(0x42)};

    constexpr uint64_t kUninitializedRIP = 0xba5eba11acc01ade;
    emitImmStoreq(v, kUninitializedRIP, calleeAR + AROFF(m_savedRip));
  }

  // A few vasm passes depend on the particular instruction sequence here:
  //  - vasm-copy expects this lea{} to be immediately followed by the
  //    callphp{} below.
  //  - vasm-prof-branch requires that this lea{} fall through straight to the
  //    callphp{}, with no intervening control flow (though it doesn't care if
  //    they're contiguous).
  v << lea{calleeAR, rvmfp()};

  // Emit a smashable call that initially calls a recyclable service request
  // stub.  The stub and the eventual targets take rvmfp() as an argument,
  // pointing to the callee ActRec.
  auto const target = callee
    ? tc::ustubs().immutableBindCallStub
    : tc::ustubs().bindCallStub;

  auto const done = v.makeBlock();
  v << callphp{target, php_call_regs(), {{done, catchBlock}}};
  v = done;

  auto const dst = dstLoc(env, inst, 0);
  auto const type = inst->dst()->type();
  if (!type.admitsSingleVal()) {
    v << defvmretdata{dst.reg(0)};
  }
  if (type.needsReg()) {
    v << defvmrettype{dst.reg(1)};
  }
}

void cgCallArray(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<CallArray>();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const syncSP = v.makeReg();
  v << lea{sp[cellsToBytes(extra->spOffset.offset)], syncSP};
  v << syncvmsp{syncSP};

  auto const target = extra->numParams == 0
    ? tc::ustubs().fcallArrayHelper
    : tc::ustubs().fcallUnpackHelper;

  auto const pc = v.cns(extra->pc);
  auto const after = v.cns(extra->after);
  auto const args = extra->numParams == 0
    ? v.makeTuple({pc, after})
    : v.makeTuple({pc, after, v.cns(extra->numParams)});

  auto const done = v.makeBlock();
  v << vcallarray{target, fcall_array_regs(), args,
                  {done, label(env, inst->taken())}};
  v = done;

  auto const dst = dstLoc(env, inst, 0);
  auto const type = inst->dst()->type();
  if (!type.admitsSingleVal()) {
    v << defvmretdata{dst.reg(0)};
  }
  if (type.needsReg()) {
    v << defvmrettype{dst.reg(1)};
  }
}

void cgCallBuiltin(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<CallBuiltin>();
  auto const callee = extra->callee;
  auto const funcReturnType = callee->hniReturnType();
  auto const returnByValue = callee->isReturnByValue();

  auto const dstData = dstLoc(env, inst, 0).reg(0);
  auto const dstType = dstLoc(env, inst, 0).reg(1);

  auto returnType = inst->dst()->type();
  // Subtract out the null possibility from the return type if it would be a
  // reference type otherwise. Don't do this if the type is nothing but a null
  // (which would give us TBottom. This makes it easier to test what kind of
  // return we need to generate below.
  if (returnType.maybe(TNull) && !(returnType <= TNull)) {
    if ((returnType - TNull).isReferenceType()) {
      returnType -= TNull;
    }
  }

  auto& v = vmain(env);

  // Whether `t' is passed in/out of C++ as String&/Array&/Object&.
  auto const isReqPtrRef = [] (MaybeDataType t) {
    return isStringType(t) || isArrayLikeType(t) ||
           t == KindOfObject || t == KindOfResource;
  };

  if (FixupMap::eagerRecord(callee)) {
    auto const sp = srcLoc(env, inst, 1).reg();
    auto const spOffset = cellsToBytes(extra->spOffset.offset);
    auto const& marker = inst->marker();
    auto const pc = marker.fixupSk().unit()->entry() + marker.fixupBcOff();

    auto const synced_sp = v.makeReg();
    v << lea{sp[spOffset], synced_sp};
    emitEagerSyncPoint(v, pc, rvmtl(), srcLoc(env, inst, 0).reg(), synced_sp);
  }

  auto args = argGroup(env, inst);

  int returnOffset = 0;

  if (!returnByValue) {
    if (isBuiltinByRef(funcReturnType)) {
      returnOffset = rds::kVmMInstrStateOff +
        (isReqPtrRef(funcReturnType) ? offsetof(MInstrState, ptrBuiltinReturn) :
         offsetof(MInstrState, tvBuiltinReturn));
      // Pass the address of tvBuiltinReturn or ptrBuiltinReturn to the
      // native function as the location where it can construct the return
      // Array, String, Object, or Variant.
      args.indRet(rvmtl(), returnOffset);
    }
  }

  // The srcs past the first two (sp and fp) are the arguments to the callee.
  auto srcNum = uint32_t{2};

  // Add the this_ or self_ argument for HNI builtins.
  if (callee->isMethod()) {
    if (callee->isStatic()) {
      args.ssa(srcNum);
      ++srcNum;
    } else {
      // Note that we don't support objects with vtables here (if they may need
      // a $this pointer adjustment).  This should be filtered out during irgen
      // or before.
      args.ssa(srcNum);
      ++srcNum;
    }
  }

  // Add the func_num_args() value if needed.
  if (callee->attrs() & AttrNumArgs) {
    // If `numNonDefault' is negative, this is passed as an src.
    if (extra->numNonDefault >= 0) {
      args.imm((int64_t)extra->numNonDefault);
    } else {
      args.ssa(srcNum);
      ++srcNum;
    }
  }

  // Add the positional arguments.
  for (uint32_t i = 0; i < callee->numParams(); ++i, ++srcNum) {
    auto const& pi = callee->params()[i];

    // Non-pointer and NativeArg args are passed by value.  String, Array,
    // Object, and Variant are passed by const&, i.e. a pointer to stack memory
    // holding the value, so we expect PtrToT types for these.  Pointers to
    // req::ptr types (String, Array, Object) need adjusting to point to
    // &ptr->m_data.
    if (TVOFF(m_data) && !pi.nativeArg && isReqPtrRef(pi.builtinType)) {
      assertx(inst->src(srcNum)->type() <= TPtrToGen);
      args.addr(srcLoc(env, inst, srcNum).reg(), TVOFF(m_data));
    } else if (pi.nativeArg && !pi.builtinType && !callee->byRef(i)) {
      // This condition indicates a MixedTV (i.e., TypedValue-by-value) arg.
      args.typedValue(srcNum);
    } else {
      args.ssa(srcNum, pi.builtinType == KindOfDouble);
    }
  }

  auto dest = [&] () -> CallDest {
    if (isBuiltinByRef(funcReturnType)) {
      if (!returnByValue) return kVoidDest; // indirect return
      return funcReturnType
        ? callDest(dstData) // String, Array, or Object
        : callDest(dstData, dstType); // Variant
    }
    return funcReturnType == KindOfDouble
      ? callDestDbl(env, inst)
      : callDest(env, inst);
  }();

  cgCallHelper(v, env, CallSpec::direct(callee->nativeFuncPtr()),
               dest, SyncOptions::Sync, args);

  // For primitive return types (int, bool, double) and returnByValue, the
  // return value is already in dstData/dstType.
  if (returnType.isSimpleType() || returnByValue) return;

  // For return by reference (String, Object, Array, Variant), the builtin
  // writes the return value into MInstrState::tvBuiltinReturn or
  // ptrBuiltinReturn, from where it has to be tested and copied.

  if (returnType.isReferenceType()) {
    // The return type is String, Array, or Object; fold nullptr to KindOfNull.
    assertx(isBuiltinByRef(funcReturnType) && isReqPtrRef(funcReturnType));

    v << load{rvmtl()[returnOffset], dstData};

    if (dstType.isValid()) {
      auto const sf = v.makeReg();
      auto const rtype = v.cns(returnType.toDataType());
      auto const nulltype = v.cns(KindOfNull);
      v << testq{dstData, dstData, sf};
      v << cmovb{CC_Z, sf, rtype, nulltype, dstType};
    }
    return;
  }

  if (returnType <= TCell || returnType <= TBoxedCell) {
    // The return type is Variant; fold KindOfUninit to KindOfNull.
    assertx(isBuiltinByRef(funcReturnType) && !isReqPtrRef(funcReturnType));
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");

    v << load{rvmtl()[returnOffset + TVOFF(m_data)], dstData};

    if (dstType.isValid()) {
      auto const rtype = v.makeReg();
      v << loadb{rvmtl()[returnOffset + TVOFF(m_type)], rtype};

      auto const sf = v.makeReg();
      auto const nulltype = v.cns(KindOfNull);
      v << testb{rtype, rtype, sf};
      v << cmovb{CC_Z, sf, rtype, nulltype, dstType};
    }
    return;
  }

  not_reached();
}

void cgNativeImpl(IRLS& env, const IRInstruction* inst) {
  auto fp = srcLoc(env, inst, 0).reg();
  auto sp = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const func = inst->marker().func();

  if (FixupMap::eagerRecord(func)) {
    emitEagerSyncPoint(v, func->getEntry(), rvmtl(), fp, sp);
  }
  v << vinvoke{
    CallSpec::direct(func->builtinFuncPtr()),
    v.makeVcallArgs({{fp}}),
    v.makeTuple({}),
    {label(env, inst->next()), label(env, inst->taken())},
    makeFixup(inst->marker(), SyncOptions::Sync)
  };
}

static void traceCallback(ActRec* fp, Cell* sp, Offset bcOff) {
  if (Trace::moduleEnabled(Trace::hhirTracelets)) {
    FTRACE(0, "{} {} {} {} {}\n",
           fp->m_func->fullName()->data(), bcOff, fp, sp,
           __builtin_return_address(0));
  }
  checkFrame(fp, sp, true /* fullCheck */, bcOff);
}

void cgDbgTraceCall(IRLS& env, const IRInstruction* inst) {
  auto const spOff = inst->extra<DbgTraceCall>()->offset;

  auto const args = argGroup(env, inst)
    .ssa(0)
    .addr(srcLoc(env, inst, 1).reg(), cellsToBytes(spOff.offset))
    .imm(inst->marker().bcOff());

  cgCallHelper(vmain(env), env, CallSpec::direct(traceCallback),
               callDest(env, inst), SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////

void cgEnterFrame(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << phplogue{fp};
}

///////////////////////////////////////////////////////////////////////////////

void cgCheckRefs(IRLS& env, const IRInstruction* inst)  {
  auto const func = srcLoc(env, inst, 0).reg();
  auto const nparams = srcLoc(env, inst, 1).reg();

  auto const extra = inst->extra<CheckRefs>();
  auto const mask64 = extra->mask;
  auto const vals64 = extra->vals;
  assertx(mask64);
  assertx((vals64 & mask64) == vals64);

  auto& v = vmain(env);

  auto const thenBody = [&] (Vout& v) {
    auto const sf = v.makeReg();

    auto bitsOff = sizeof(uint64_t) * (extra->firstBit / 64);
    auto bitsPtr = v.makeReg();
    auto cond = CC_NE;

    if (extra->firstBit == 0) {
      bitsOff = Func::refBitValOff();
      bitsPtr = func;
    } else {
      v << load{func[Func::sharedOff()], bitsPtr};
      bitsOff -= sizeof(uint64_t);
    }

    if (vals64 == 0 || (mask64 & (mask64 - 1)) == 0) {
      // If vals64 is zero, or we're testing a single bit, we can get away with
      // a single test, rather than mask-and-compare.  The use of testbim{} and
      // testlim{} here is little-endian specific but it's "ok" for now as long
      // as nothing else is read or written using the same pointer.
      if (mask64 <= 0xff) {
        v << testbim{(int8_t)mask64, bitsPtr[bitsOff], sf};
      } else if (mask64 <= 0xffffffff) {
        v << testlim{(int32_t)mask64, bitsPtr[bitsOff], sf};
      } else {
        v << testqm{v.cns(mask64), bitsPtr[bitsOff], sf};
      }
      if (vals64) cond = CC_E;
    } else {
      auto const bits = v.makeReg();
      auto const maskedBits = v.makeReg();

      if (mask64 <= 0xff && vals64 <= 0xff) {
        v << loadtqb{bitsPtr[bitsOff], bits};
        v << andbi{(int8_t)mask64, bits, maskedBits, v.makeReg()};
        v << cmpbi{(int8_t)vals64, maskedBits, sf};
      } else if (mask64 <= 0xffffffff && vals64 <= 0xffffffff) {
        v << loadtql{bitsPtr[bitsOff], bits};
        v << andli{(int32_t)mask64, bits, maskedBits, v.makeReg()};
        v << cmpli{(int32_t)vals64, maskedBits, sf};
      } else {
        v << load{bitsPtr[bitsOff], bits};
        v << andq{v.cns(mask64), bits, maskedBits, v.makeReg()};
        v << cmpq{v.cns(vals64), maskedBits, sf};
      }
    }
    fwdJcc(v, env, cond, sf, inst->taken());
  };

  if (extra->firstBit == 0) {
    assertx(inst->src(1)->hasConstVal());
    // This is the first 64 bits.  No need to check nparams.
    thenBody(v);
  } else {
    // Check number of args...
    auto const sf = v.makeReg();
    v << cmpqi{extra->firstBit, nparams, sf};

    if (vals64 != 0 && vals64 != mask64) {
      // If we're beyond nparams, then either all params are refs, or all
      // params are non-refs, so if vals64 isn't 0 and isnt mask64, there's no
      // possibility of a match.
      fwdJcc(v, env, CC_LE, sf, inst->taken());
      thenBody(v);
    } else {
      ifThenElse(v, CC_NLE, sf, thenBody, [&] (Vout& v) {
        // If not special builtin...
        auto const sf = v.makeReg();
        v << testlim{AttrVariadicByRef, func[Func::attrsOff()], sf};
        fwdJcc(v, env, vals64 ? CC_Z : CC_NZ, sf, inst->taken());
      });
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
