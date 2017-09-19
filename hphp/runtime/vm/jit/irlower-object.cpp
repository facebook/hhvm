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

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/class.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdObjClass(IRLS& env, const IRInstruction* inst) {
  auto dst = dstLoc(env, inst, 0).reg();
  auto obj = srcLoc(env, inst, 0).reg();
  emitLdObjClass(vmain(env), obj, dst);
}

IMPL_OPCODE_CALL(AllocObj)

void cgNewInstanceRaw(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = inst->extra<NewInstanceRaw>()->cls;
  auto const size = ObjectData::sizeForNProps(cls->numDeclProperties());

  auto attrs = cls->getODAttrs();
  if (attrs != ObjectData::DefaultAttrs) {
    auto func = size <= kMaxSmallSize
      ? &ObjectData::newInstanceRawAttrs<false>
      : &ObjectData::newInstanceRawAttrs<true>;
    cgCallHelper(vmain(env), env, CallSpec::direct(func),
                 callDest(dst), SyncOptions::Sync, argGroup(env, inst)
                   .imm(reinterpret_cast<uintptr_t>(cls))
                   .imm(size)
                   .imm(attrs));
  } else {
    auto func = size <= kMaxSmallSize
      ? &ObjectData::newInstanceRaw<false>
      : &ObjectData::newInstanceRaw<true>;
    cgCallHelper(vmain(env), env, CallSpec::direct(func),
                 callDest(dst), SyncOptions::Sync, argGroup(env, inst)
                   .imm(reinterpret_cast<uintptr_t>(cls))
                   .imm(size));
  }
}

void cgConstructInstance(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = inst->extra<ConstructInstance>()->cls;

  auto const args = argGroup(env, inst).immPtr(cls);
  cgCallHelper(vmain(env), env, CallSpec::direct(cls->instanceCtor().get()),
               callDest(dst), SyncOptions::Sync, args);
}

IMPL_OPCODE_CALL(Clone)
IMPL_OPCODE_CALL(RegisterLiveObj)

///////////////////////////////////////////////////////////////////////////////

namespace {

void implInitObjPropsFast(Vout& v, IRLS& env, const IRInstruction* inst,
                          Vreg dst, const Class* cls, size_t nprops) {
  // If the object has a small number of properties, just emit stores inline.
  if (nprops < 8) {
    for (int i = 0; i < nprops; ++i) {
      auto const propOffset = sizeof(ObjectData) + sizeof(TypedValue) * i;
      auto const& initTV = cls->declPropInit()[i];

      if (!isNullType(initTV.m_type)) {
        emitImmStoreq(v, initTV.m_data.num, dst[propOffset + TVOFF(m_data)]);
      }
      v << storebi{initTV.m_type, dst[propOffset + TVOFF(m_type)]};
    }
    return;
  }

  // Use memcpy for large numbers of properties.
  auto args = argGroup(env, inst)
    .addr(dst, safe_cast<int32_t>(sizeof(ObjectData)))
    .imm(reinterpret_cast<uintptr_t>(&cls->declPropInit()[0]))
    .imm(cellsToBytes(nprops));

  cgCallHelper(v, env, CallSpec::direct(memcpy),
               kVoidDest, SyncOptions::None, args);
}

}

void cgInitObjProps(IRLS& env, const IRInstruction* inst) {
  auto const cls = inst->extra<InitObjProps>()->cls;
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  // Initialize the properties.
  auto const nprops = cls->numDeclProperties();
  if (nprops > 0) {
    if (cls->pinitVec().size() == 0) {
      // If the Class has no 86pinit property-initializer functions, we can
      // just copy the initial values from a data member on the Class.
      implInitObjPropsFast(v, env, inst, obj, cls, nprops);
    } else {
      // Load the Class's propInitVec from the target cache.  We know it's
      // already been initialized as a pre-condition on this op.
      auto const propHandle = cls->propHandle();
      assertx(rds::isNormalHandle(propHandle));

      auto const propInitVec = v.makeReg();
      auto const propData = v.makeReg();
      v << load{Vreg(rvmtl())[propHandle], propInitVec};
      v << load{propInitVec[Class::PropInitVec::dataOff()], propData};

      auto args = argGroup(env, inst)
        .addr(obj, safe_cast<int32_t>(sizeof(ObjectData)))
        .reg(propData);

      if (!cls->hasDeepInitProps()) {
        cgCallHelper(v, env, CallSpec::direct(memcpy), kVoidDest,
                     SyncOptions::None, args.imm(cellsToBytes(nprops)));
      } else {
        cgCallHelper(v, env, CallSpec::direct(deepInitHelper),
                     kVoidDest, SyncOptions::None, args.imm(nprops));
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
