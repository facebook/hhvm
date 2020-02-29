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
#include "hphp/runtime/vm/call-flags.h"
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
#include "hphp/runtime/vm/jit/call-target-profile.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/target-profile.h"
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
  auto const callee = srcLoc(env, inst, 2).reg();
  auto const ctx = srcLoc(env, inst, 3).reg();
  auto const extra = inst->extra<Call>();
  auto const numArgsInclUnpack = extra->numArgs + (extra->hasUnpack ? 1 : 0);

  auto& v = vmain(env);

  auto const callFlags = CallFlags(
    extra->hasGenerics,
    extra->dynamicCall,
    extra->asyncEagerReturn,
    extra->callOffset,
    extra->genericsBitmap
  );

  v << copy{v.cns(callFlags.value()), r_php_call_flags()};
  v << copy{callee, r_php_call_func()};
  v << copy{v.cns(numArgsInclUnpack), r_php_call_num_args()};

  auto withCtx = false;
  assertx(inst->src(3)->isA(TObj) || inst->src(3)->isA(TCls) ||
          inst->src(3)->isA(TNullptr));
  if (inst->src(3)->isA(TObj) || inst->src(3)->isA(TCls)) {
    withCtx = true;
    v << copy{ctx, r_php_call_ctx()};
  } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
    withCtx = true;
    v << copy{v.cns(ActRec::kTrashedThisSlot), r_php_call_ctx()};
  }

  // Make vmsp() point to the future vmfp().
  auto const ssp = v.makeReg();
  v << lea{sp[cellsToBytes(extra->spOffset.offset + extra->numInputs())], ssp};
  v << syncvmsp{ssp};

  auto const done = v.makeBlock();
  if (inst->src(2)->hasConstVal(TFunc) &&
      extra->numArgs <= inst->src(2)->funcVal()->numNonVariadicParams()) {
    // Emit a smashable call that initially calls a recyclable service request
    // stub.  The stub and the eventual targets take rvmfp() as an argument,
    // pointing to the callee ActRec.
    auto const func = inst->src(2)->funcVal();
    assertx(!extra->hasUnpack ||
            extra->numArgs == func->numNonVariadicParams());
    v << callphp{tc::ustubs().immutableBindCallStub, php_call_regs(withCtx),
                 func, numArgsInclUnpack};
  } else if (extra->skipNumArgsCheck) {
    // If we've statically determined the provided number of arguments
    // doesn't exceed what the target expects, we can skip the stub
    // and call the prologue directly.
    assertx(!extra->hasUnpack);
    auto const pTabOff = safe_cast<int32_t>(Func::prologueTableOff());
    auto const ptrSize = safe_cast<int32_t>(sizeof(LowPtr<uint8_t>));
    auto const dest = v.makeReg();
    emitLdLowPtr(v, r_php_call_func()[extra->numArgs * ptrSize + pTabOff],
                 dest, sizeof(LowPtr<uint8_t>));
    v << callphpr{dest, php_call_regs(withCtx)};
  } else {
    // If the callee is not known statically, or it was determined later, but
    // we have too many arguments for prologue to handle, go to the redispatch
    // stub that will pack any extra arguments and transfer control to the
    // appropriate prologue.
    assertx(!extra->hasUnpack);
    v << callphp{tc::ustubs().funcPrologueRedispatch, php_call_regs(withCtx),
                 nullptr, extra->numArgs};
  }

  // The prologue is responsible for unwinding all inputs. We could have
  // optimized away Uninit stores for ActRec and inouts, so skip them as well.
  auto const marker = inst->marker();
  auto const fixupBcOff = marker.fixupBcOff() - marker.fixupFunc()->base();
  auto const fixupSpOff =
    marker.spOff() - extra->numInputs() - kNumActRecCells - extra->numOut;
  v << syncpoint{Fixup{fixupBcOff, fixupSpOff.offset}};
  v << unwind{done, label(env, inst->taken())};
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

void cgCallUnpack(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<CallUnpack>();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const callee = srcLoc(env, inst, 2).reg();
  auto const ctx = srcLoc(env, inst, 3).reg();
  auto& v = vmain(env);

  auto const calleeSP = sp[cellsToBytes(extra->spOffset.offset)];
  auto const calleeAR = calleeSP + cellsToBytes(extra->numInputs());
  v << store{callee, calleeAR + AROFF(m_func)};
  v << storeli{safe_cast<int32_t>(extra->numArgs) + 1,
               calleeAR + AROFF(m_numArgs)};

  assertx(inst->src(3)->isA(TObj) || inst->src(3)->isA(TCls) ||
          inst->src(3)->isA(TNullptr));
  if (inst->src(3)->isA(TObj) || inst->src(3)->isA(TCls)) {
    v << store{ctx, calleeAR + AROFF(m_thisUnsafe)};
  } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, ActRec::kTrashedThisSlot, calleeAR + AROFF(m_thisUnsafe));
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, ActRec::kTrashedVarEnvSlot, calleeAR + AROFF(m_varEnv));
  }

  auto const syncSP = v.makeReg();
  v << lea{sp[cellsToBytes(extra->spOffset.offset)], syncSP};
  v << syncvmsp{syncSP};

  auto const callFlags = CallFlags(
    extra->hasGenerics,
    extra->dynamicCall,
    false,  // async eager return unsupported with unpack
    0, // call offset passed differently to unpack
    0  // generics bitmap not used with unpack
  );

  auto const target = tc::ustubs().fcallUnpackHelper;
  auto const callOff = v.cns(extra->callOffset);
  auto const args = v.makeTuple(
    {callOff, v.cns(extra->numInputs()), v.cns(callFlags.value())});

  auto const done = v.makeBlock();
  v << vcallunpack{target, fcall_unpack_regs(), args,
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

  auto& v = vmain(env);

  // We don't write to the true dst registers until the very end of the
  // instruction sequence, in case we have to perform option-dependent fixups.
  auto const dstData = dstLoc(env, inst, 0).reg(0);
  auto const dstType = dstLoc(env, inst, 0).reg(1);
  auto const tmpData = v.makeReg();
  auto const tmpType = v.makeReg();

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

  int returnOffset = rds::kVmMInstrStateOff +
                     offsetof(MInstrState, tvBuiltinReturn);
  auto args = argGroup(env, inst);

  if (!returnByValue) {
    if (isBuiltinByRef(funcReturnType)) {
      if (isReqPtrRef(funcReturnType)) {
        returnOffset += TVOFF(m_data);
      }
      // Pass the address of tvBuiltinReturn to the native function as the
      // location where it can construct the return Array, String, Object, or
      // Variant.
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

  // Add the positional arguments.
  for (uint32_t i = 0; i < callee->numParams(); ++i, ++srcNum) {
    auto const& pi = callee->params()[i];

    // Non-pointer and NativeArg args are passed by value.  String, Array,
    // Object, and Variant are passed by const&, i.e. a pointer to stack memory
    // holding the value, so we expect PtrToT types for these.  Pointers to
    // req::ptr types (String, Array, Object) need adjusting to point to
    // &ptr->m_data.
    if (TVOFF(m_data) && !pi.isNativeArg() && isReqPtrRef(pi.builtinType)) {
      assertx(inst->src(srcNum)->type() <= TPtrToCell);
      args.addr(srcLoc(env, inst, srcNum).reg(), TVOFF(m_data));
    } else if (pi.isNativeArg() && !pi.builtinType) {
      // This condition indicates a MixedTV (i.e., TypedValue-by-value) arg.
      args.typedValue(srcNum);
    } else {
      args.ssa(srcNum);
    }
  }

  auto dest = [&] () -> CallDest {
    if (isBuiltinByRef(funcReturnType)) {
      if (!returnByValue) return kVoidDest; // indirect return
      return funcReturnType
        ? callDest(dstData) // String, Array, or Object
        : callDest(dstData, dstType); // Variant
    }
    return callDest(env, inst);
  }();
  if (dest.reg0.isValid()) dest.reg0 = tmpData;
  if (dest.reg1.isValid()) dest.reg1 = tmpType;

  auto const isInlined = env.unit.context().initSrcKey.func() != callee;
  if (isInlined) v << inlinestart{callee, 0};

  // Call epilogue: handle builtin return types and inlining accounting.
  auto const end = [&] (Vout& v) {
    v << copy{tmpData, dstData};
    if (dstType.isValid()) v << copy{tmpType, dstType};
    if (isInlined) v << inlineend{};
  };

  cgCallHelper(v, env, CallSpec::direct(callee->nativeFuncPtr(), nullptr),
               dest, SyncOptions::Sync, args);

  // For primitive return types (int, bool, double) and returnByValue, the
  // return value is already in dstData/dstType.
  if (returnType.isSimpleType() || returnByValue) return end(v);

  // For return by reference (String, Object, Array, Variant), the builtin
  // writes the return value into MInstrState::tvBuiltinReturn, from where it
  // has to be tested and copied.

  if (returnType.isReferenceType()) {
    // The return type is String, Array, or Object; fold nullptr to KindOfNull.
    assertx(isBuiltinByRef(funcReturnType) && isReqPtrRef(funcReturnType));

    v << load{rvmtl()[returnOffset], tmpData};

    if (dstType.isValid()) {
      auto const sf = v.makeReg();
      auto const rtype = v.cns(returnType.toDataType());
      auto const nulltype = v.cns(KindOfNull);
      v << testq{tmpData, tmpData, sf};
      v << cmovb{CC_Z, sf, rtype, nulltype, tmpType};
    }
    return end(v);
  }

  if (returnType <= TCell) {
    // The return type is Variant; fold KindOfUninit to KindOfNull.
    assertx(isBuiltinByRef(funcReturnType) && !isReqPtrRef(funcReturnType));
    static_assert(KindOfUninit == static_cast<DataType>(0),
                  "KindOfUninit must be 0 for test");

    v << load{rvmtl()[returnOffset + TVOFF(m_data)], tmpData};

    if (dstType.isValid()) {
      auto const rtype = v.makeReg();
      v << loadb{rvmtl()[returnOffset + TVOFF(m_type)], rtype};

      auto const sf = v.makeReg();
      auto const nulltype = v.cns(KindOfNull);
      static_assert(KindOfUninit == static_cast<DataType>(0),
                    "Codegen assumes KindOfUninit == 0");
      v << testb{rtype, rtype, sf};
      v << cmovb{CC_Z, sf, rtype, nulltype, tmpType};
    }
    return end(v);
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
    CallSpec::direct(func->arFuncPtr(), nullptr),
    v.makeVcallArgs({{fp}}),
    v.makeTuple({}),
    {label(env, inst->next()),
     label(env, inst->taken())},
    makeFixup(inst->marker(), SyncOptions::Sync)
  };
}

static void traceCallback(ActRec* fp, TypedValue* sp, Offset bcOff) {
  if (Trace::moduleEnabled(Trace::hhirTracelets)) {
    FTRACE(0, "{} {} {} {} {}\n",
           fp->m_func->fullName()->data(), bcOff, fp, sp,
           __builtin_return_address(0));
  }
  checkFrame(fp, sp, true /* fullCheck */);
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

void cgProfileCall(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ProfileCallTargetData>();

  auto const args = argGroup(env, inst)
    .addr(rvmtl(), safe_cast<int32_t>(extra->handle))
    .ssa(0);

  cgCallHelper(vmain(env), env, CallSpec::method(&CallTargetProfile::report),
               kVoidDest, SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////

void cgEnterPrologue(IRLS& env, const IRInstruction*) {
  vmain(env) << stublogue{false};
}

///////////////////////////////////////////////////////////////////////////////

void cgCheckInOuts(IRLS& env, const IRInstruction* inst)  {
  auto const func = srcLoc(env, inst, 0).reg();
  auto const nparams = srcLoc(env, inst, 1).reg();

  auto const extra = inst->extra<CheckInOuts>();
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
      bitsOff = Func::inoutBitValOff();
      bitsPtr = func;
    } else {
      auto const shared = v.makeReg();
      v << load{func[Func::sharedOff()], shared};
      v << load{shared[Func::sharedInOutBitPtrOff()], bitsPtr};
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

    if (vals64 != 0) {
      // If we're beyond nparams, then all params are non-refs, so if vals64
      // isn't 0, there's no possibility of a match.
      fwdJcc(v, env, CC_LE, sf, inst->taken());
      thenBody(v);
    } else {
      ifThen(v, CC_NLE, sf, thenBody);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////


}}}
