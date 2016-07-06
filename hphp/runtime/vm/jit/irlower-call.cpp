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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-helpers.h"
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
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
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

      auto addr = callee->getEntry();
      while (peek_op(addr) == Op::AssertRATL) {
        addr += instrLen(addr);
      }
      assertx(peek_op(addr) == Op::NativeImpl);
      assertx(addr + instrLen(addr) ==
              callee->unit()->entry() + callee->past());
    }

    v << store{v.cns(mcg->ustubs().retHelper), calleeAR + AROFF(m_savedRip)};
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
    env.catch_calls[inst->taken()] = CatchCall::CPP;

    v = done;
    // The native implementation already put the return value on the stack for
    // us, and handled cleaning up the arguments.  We have to update the frame
    // pointer and the stack pointer, and load the return value into the return
    // register so the trace we are returning to has it where it expects.
    // TODO(#1273094): We should probably modify the actual builtins to return
    // values via registers using the C ABI and do a reg-to-reg move.
    loadTV(v, inst->dst(), dstLoc(env, inst, 0), rvmfp()[AROFF(m_r)], true);
    v << load{rvmfp()[AROFF(m_sfp)], rvmfp()};
    emitRB(v, Trace::RBTypeFuncExit, callee->fullName()->data());
    return;
  }

  v << lea{calleeAR, rvmfp()};

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << syncvmsp{v.cns(0x42)};

    constexpr uint64_t kUninitializedRIP = 0xba5eba11acc01ade;
    emitImmStoreq(v, kUninitializedRIP, rvmfp()[AROFF(m_savedRip)]);
  }

  // Emit a smashable call that initially calls a recyclable service request
  // stub.  The stub and the eventual targets take rvmfp() as an argument,
  // pointing to the callee ActRec.
  auto const target = callee
    ? mcg->ustubs().immutableBindCallStub
    : mcg->ustubs().bindCallStub;

  auto const done = v.makeBlock();
  v << callphp{target, php_call_regs(), {{done, catchBlock}}};
  env.catch_calls[inst->taken()] = CatchCall::PHP;
  v = done;

  auto const dst = dstLoc(env, inst, 0);
  v << defvmret{dst.reg(0), dst.reg(1)};
}

void cgCallArray(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<CallArray>();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const syncSP = v.makeReg();
  v << lea{sp[cellsToBytes(extra->spOffset.offset)], syncSP};
  v << syncvmsp{syncSP};

  auto const target = extra->numParams == 0
    ? mcg->ustubs().fcallArrayHelper
    : mcg->ustubs().fcallUnpackHelper;

  auto const pc = v.cns(extra->pc);
  auto const after = v.cns(extra->after);
  auto const args = extra->numParams == 0
    ? v.makeTuple({pc, after})
    : v.makeTuple({pc, after, v.cns(extra->numParams)});

  auto const done = v.makeBlock();
  v << vcallarray{target, fcall_array_regs(), args,
                  {done, label(env, inst->taken())}};
  env.catch_calls[inst->taken()] = CatchCall::PHP;
  v = done;

  auto const dst = dstLoc(env, inst, 0);
  v << defvmret{dst.reg(0), dst.reg(1)};
}

void cgCallBuiltin(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<CallBuiltin>();
  auto const callee = extra->callee;
  auto const returnType = inst->typeParam();
  auto const funcReturnType = callee->returnType();
  auto const returnByValue = callee->isReturnByValue();

  auto const dstData = dstLoc(env, inst, 0).reg(0);
  auto const dstType = dstLoc(env, inst, 0).reg(1);

  auto& v = vmain(env);

  // Whether `t' is passed in/out of C++ as String&/Array&/Object&.
  auto const isReqPtrRef = [] (MaybeDataType t) {
    return isStringType(t) || isArrayType(t) ||
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
      args.addr(rvmtl(), returnOffset);
    }
  }

  // The srcs past the first two (sp and fp) are the arguments to the callee.
  auto srcNum = uint32_t{2};

  // Add the this_ or self_ argument for HNI builtins.
  if (callee->isMethod()) {
    if (callee->isStatic()) {
      if (callee->isNative()) {
        args.ssa(srcNum);
        ++srcNum;
      }
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
  // writes the return value into MInstrState::tvBuiltinReturn, from where it
  // has to be tested and copied.

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
  env.catch_calls[inst->taken()] = CatchCall::CPP;
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

}}}
