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

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/class-meth-data.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/preclass.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"
#include "hphp/util/low-ptr.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdClsName(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const preclass = v.makeReg();
  v << load{src[Class::preClassOff()], preclass};
  emitLdLowPtr(v, preclass[PreClass::nameOffset()],
               dst, sizeof(LowStringPtr));
}

void cgLdLazyCls(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const preclass = v.makeReg();
  v << load{src[Class::preClassOff()], preclass};
  emitLdLowPtr(v, preclass[PreClass::nameOffset()],
               dst, sizeof(LowStringPtr));
}

void cgLdLazyClsName(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const lazyClsData = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << copy{lazyClsData, dst};
}

void cgMethodExists(IRLS& env, const IRInstruction* inst) {
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(vmain(env), env, CallSpec::direct(methodExistsHelper),
               callDest(env, inst), SyncOptions::None, args);
}

void cgLdClsMethod(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto const idx = inst->src(1)->intVal();
  assertx(idx >= 0);

  auto& v = vmain(env);
  auto const methOff = -(idx + 1) * sizeof(LowPtr<Func>);
  emitLdLowPtr(v, cls[methOff], dst, sizeof(LowPtr<Func>));
}

void cgLdIfaceMethod(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdIfaceMethod>();
  auto const func = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const vtable_vec = v.makeReg();
  auto const vtable = v.makeReg();

  v << load{cls[Class::vtableVecOff()], vtable_vec};
  auto const vtableOff = extra->vtableIdx * sizeof(Class::VtableVecSlot) +
    offsetof(Class::VtableVecSlot, vtable);
  v << load{vtable_vec[vtableOff], vtable};
  emitLdLowPtr(v, vtable[extra->methodIdx * sizeof(LowPtr<Func>)],
               func, sizeof(LowPtr<Func>));
}

void cgLdObjInvoke(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  emitLdLowPtr(v, cls[Class::invokeOff()], dst, sizeof(LowPtr<Func>));
}

IMPL_OPCODE_CALL(HasToString);

void cgLdFuncVecLen(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const off = Class::funcVecLenOff();
  static_assert(sizeof(Class::veclen_t) == 2 || sizeof(Class::veclen_t) == 4,
                "Class::veclen_t must be 2 or 4 bytes wide");
  if (sizeof(Class::veclen_t) == 2) {
    auto const tmp = v.makeReg();
    v << loadw{cls[off], tmp};
    v << movzwq{tmp, dst};
  } else {
    v << loadzlq{cls[off], dst};
  }
}

///////////////////////////////////////////////////////////////////////////////

void cgLdClsInitData(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = srcLoc(env, inst, 0).reg();
  auto const offset = Class::propDataCacheOff() +
    rds::Link<Class::PropInitVec*, rds::Mode::Normal>::handleOff();
  auto& v = vmain(env);

  auto const handle = v.makeReg();
  auto const vec = v.makeReg();
  v << loadzlq{cls[offset], handle};
  markRDSAccess(v, handle);
  v << load{Vreg(rvmtl())[handle], vec};
  v << load{vec[Class::PropInitVec::dataOff()], dst};
}

void cgPropTypeRedefineCheck(IRLS& env, const IRInstruction* inst) {
  auto const cls = inst->src(0)->clsVal();
  auto const slot = inst->src(1)->intVal();
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(cls->maybeRedefinesPropTypes());
  assertx(slot != kInvalidSlot);
  assertx(slot < cls->numDeclProperties());
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::method(&Class::checkPropTypeRedefinition),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst).immPtr(cls).imm(slot)
  );
}

IMPL_OPCODE_CALL(InitProps)
IMPL_OPCODE_CALL(InitSProps)

///////////////////////////////////////////////////////////////////////////////

void cgLookupSPropSlot(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::method(&Class::lookupSProp),
    callDest(env, inst),
    SyncOptions::None,
    argGroup(env, inst).ssa(0).ssa(1)
  );
}

///////////////////////////////////////////////////////////////////////////////

void cgLdFuncInOutBits(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const func = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << loadzlq{func[Func::inoutBitsOff()], dst};
}

void cgLdFuncNumParams(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const func = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const tmp = v.makeReg();
  // See Func::finishedEmittingParams and Func::numParams.
  v << loadzlq{func[Func::paramCountsOff()], tmp};
  v << shrqi{1, tmp, dst, v.makeReg()};
}

void cgLdFuncName(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const func = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  emitLdLowPtr(v, func[Func::nameOff()], dst, sizeof(LowStringPtr));
}

void cgLdMethCallerName(IRLS& env, const IRInstruction* inst) {
  static_assert(Func::kMethCallerBit == 1,
                "Fix the decq if you change kMethCallerBit");
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const func = srcLoc(env, inst, 0).reg();
  auto const isCls = inst->extra<MethCallerData>()->isCls;
  auto& v = vmain(env);
  auto const off = isCls ?
    Func::methCallerClsNameOff() : Func::methCallerMethNameOff();
  auto const tmp = v.makeReg();
  emitLdLowPtr(v, func[off], tmp, sizeof(Func::low_storage_t));
  v << decq{tmp, dst, v.makeReg()};
}

void cgLdFuncCls(IRLS& env, const IRInstruction* inst) {
  auto const func = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  emitLdLowPtr(v, func[Func::clsOff()], dst, sizeof(Func::low_storage_t));
}

void cgFuncHasAttr(IRLS& env, const IRInstruction* inst) {
  auto const func = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const attr = inst->extra<AttrData>()->attr;

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testlim{attr, func[Func::attrsOff()], sf};
  v << setcc{CC_NZ, sf, dst};
}

void cgClassHasAttr(IRLS& env, const IRInstruction* inst) {
  auto const cls = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const attr = inst->extra<AttrData>()->attr;

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testlim{attr, cls[Class::attrCopyOff()], sf};
  v << setcc{CC_NZ, sf, dst};
}

///////////////////////////////////////////////////////////////////////////////

void cgLdFuncRequiredCoeffects(IRLS& env, const IRInstruction* inst) {
  auto const func = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  vmain(env) << loadzwq{func[Func::requiredCoeffectsOff()], dst};
}

///////////////////////////////////////////////////////////////////////////////

void cgLdClsFromClsMeth(IRLS& env, const IRInstruction* inst) {
  auto const clsMethDataRef = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  if (use_lowptr) {
#ifdef USE_LOWPTR
    static_assert(ClsMethData::clsOffset() == 0, "Class offset must be 0");
#endif
    auto const tmp = v.makeReg();
    v << movtql{clsMethDataRef, tmp};
    v << movzlq{tmp, dst};
  } else {
    v << load{clsMethDataRef[ClsMethData::clsOffset()], dst};
  }
}

void cgLdFuncFromClsMeth(IRLS& env, const IRInstruction* inst) {
  auto const clsMethDataRef = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  if (use_lowptr) {
#ifdef USE_LOWPTR
    static_assert(ClsMethData::funcOffset() == 4, "Func offset must be 4");
#endif
    v << shrqi{32, clsMethDataRef, dst, v.makeReg()};
  } else {
    v << load{clsMethDataRef[ClsMethData::funcOffset()], dst};
  }
}

void cgNewClsMeth(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(ClsMethDataRef::create),
    callDest(env, inst),
    SyncOptions::None,
    argGroup(env, inst).ssa(0).ssa(1)
  );
}

///////////////////////////////////////////////////////////////////////////////

}
