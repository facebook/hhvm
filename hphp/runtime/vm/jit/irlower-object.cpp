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

#include "hphp/runtime/base/memory-manager.h"
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
void cgLdRecDesc(IRLS& env, const IRInstruction* inst) {
  auto dst = dstLoc(env, inst, 0).reg();
  auto val = srcLoc(env, inst, 0).reg();
  emitLdRecDesc(vmain(env), val, dst);
}

void cgLdObjClass(IRLS& env, const IRInstruction* inst) {
  auto dst = dstLoc(env, inst, 0).reg();
  auto obj = srcLoc(env, inst, 0).reg();
  emitLdObjClass(vmain(env), obj, dst);
}

IMPL_OPCODE_CALL(AllocObj)
IMPL_OPCODE_CALL(AllocObjReified)

namespace {

void objectPropsRawInit(Vout& v, Vreg base, size_t props) {
  if (props == 0) return;

  auto const last_type_word_offset =
    sizeof(ObjectData) +
    8 * sizeof(uint64_t) * ((props - 1) / 7);

  v << storeqi{0, base[last_type_word_offset]};
}

}

void cgNewInstanceRaw(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = inst->extra<NewInstanceRaw>()->cls;

  assertx(!cls->getNativeDataInfo());
  auto const memoSize =
    cls->hasMemoSlots() ? ObjectData::objOffFromMemoNode(cls) : 0;
  auto const size =
    ObjectData::sizeForNProps(cls->numDeclProperties()) + memoSize;
  auto const index = MemoryManager::size2Index(size);
  auto const size_class = MemoryManager::sizeIndex2Size(index);

  auto const target = [&]{
    if (memoSize > 0) {
      return size <= kMaxSmallSize
        ? CallSpec::direct(&ObjectData::newInstanceRawMemoSmall)
        : CallSpec::direct(&ObjectData::newInstanceRawMemoBig);
    } else {
      return size <= kMaxSmallSize
        ? CallSpec::direct(&ObjectData::newInstanceRawSmall)
        : CallSpec::direct(&ObjectData::newInstanceRawBig);
    }
  }();

  auto args = argGroup(env, inst).immPtr(cls);
  size <= kMaxSmallSize
    ? args.imm(size_class).imm(index)
    : args.imm(size);
  if (memoSize > 0) args.imm(memoSize);

  cgCallHelper(
    v,
    env,
    target,
    callDest(dst),
    SyncOptions::Sync,
    args
  );
  objectPropsRawInit(v, dst, cls->numDeclProperties());

}

void cgConstructInstance(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = inst->extra<ConstructInstance>()->cls;

  auto const args = argGroup(env, inst).immPtr(cls);
  cgCallHelper(vmain(env), env,
               CallSpec::direct(cls->instanceCtor<true>().get()),
               callDest(dst), SyncOptions::Sync, args);
}

void cgConstructClosure(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cls = inst->extra<ConstructClosure>()->cls;
  assertx(cls);
  auto& v = vmain(env);

  // c_Closure is not allowed to use the fast path, as the constructor will
  // throw immediately.  No other closure's constructor is able to throw, so we
  // are able to make some optimizations that would be unsafe if unwinding.
  if (!RuntimeOption::RepoAuthoritative || cls == c_Closure::classof()) {
    auto const args = argGroup(env, inst).immPtr(cls);
    cgCallHelper(vmain(env), env, CallSpec::direct(createClosure),
                 callDest(dst), SyncOptions::None, args);
  } else {
    auto const size = c_Closure::size(cls);

    auto const index = MemoryManager::size2Index(size);
    auto const size_class = MemoryManager::sizeIndex2Size(index);

    // We don't specialize the large allocation case since it is unlikely.
    auto const target = index < kNumSmallSizes
                        ? CallSpec::direct(&createClosureRepoAuthRawSmall)
                        : CallSpec::direct(&createClosureRepoAuth);

    auto args = argGroup(env, inst).immPtr(cls);
    if (index < kNumSmallSizes) {
      args.imm(size_class).imm(index);
    }

    cgCallHelper(vmain(env), env, target,
                 callDest(dst), SyncOptions::None, args);

    objectPropsRawInit(v, dst, cls->numDeclProperties());
  }

  if (inst->src(0)->isA(TNullptr)) {
    v << storeqi{0, dst[c_Closure::ctxOffset()]};
  } else {
    auto const ctx = srcLoc(env, inst, 0).reg();
    v << store{ctx, dst[c_Closure::ctxOffset()]};
  }
}

IMPL_OPCODE_CALL(Clone)

IMPL_OPCODE_CALL(FuncCred);

///////////////////////////////////////////////////////////////////////////////

namespace {

static void initObjMemoSlots(MemoSlot* slots, size_t n) {
  for (auto i = 0; i < n; i++) {
    slots[i].init();
  }
}

void implInitObjPropsFast(Vout& v, IRLS& env, const IRInstruction* inst,
                          Vreg dst, const Class* cls, size_t nprops) {
  // memcpy the values from the class property init vec.
  auto args = argGroup(env, inst)
    .addr(dst, safe_cast<int32_t>(sizeof(ObjectData)))
    .imm(reinterpret_cast<uintptr_t>(cls->declPropInit().data()))
    .imm(ObjectProps::sizeFor(nprops));

  cgCallHelper(v, env, CallSpec::direct(memcpy),
               kVoidDest, SyncOptions::None, args);
}

void implInitObjMemoSlots(Vout& v, IRLS& env, const IRInstruction* inst,
                          const Class* cls, Vreg obj) {
  assertx(cls->hasMemoSlots());
  assertx(!cls->getNativeDataInfo());

  auto const nslots = cls->numMemoSlots();
  if (nslots < 8) {
    for (Slot i = 0; i < nslots; ++i) {
      static_assert(sizeof(MemoSlot) == 16, "");
      auto const offset = -(sizeof(MemoSlot) * (nslots - i));
      auto const dt = static_cast<data_type_t>(KindOfUninit);
      emitImmStoreq(v, 0, obj[offset + TVOFF(m_data)]);
      v << storebi{dt, obj[offset + TVOFF(m_type)]};
    }
    return;
  }

  auto const args = argGroup(env, inst)
    .addr(obj, -safe_cast<int32_t>(sizeof(MemoSlot) * nslots))
    .imm(nslots);
  cgCallHelper(v, env, CallSpec::direct(initObjMemoSlots),
               kVoidDest, SyncOptions::None, args);
}

}

void cgInitObjMemoSlots(IRLS& env, const IRInstruction* inst) {
  auto const cls = inst->extra<InitObjMemoSlots>()->cls;
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  implInitObjMemoSlots(v, env, inst, cls, obj);
}

void cgInitObjProps(IRLS& env, const IRInstruction* inst) {
  auto const cls = inst->extra<InitObjProps>()->cls;
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  if (cls->hasMemoSlots()) implInitObjMemoSlots(v, env, inst, cls, obj);

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

      auto args = argGroup(env, inst)
        .addr(obj, safe_cast<int32_t>(sizeof(ObjectData)));

      if (!cls->hasDeepInitProps()) {
        v << load{propInitVec[Class::PropInitVec::dataOff()], propData};
        cgCallHelper(v, env, CallSpec::direct(memcpy), kVoidDest,
                     SyncOptions::None,
                     args
                     .reg(propData)
                     .imm(ObjectProps::sizeFor(nprops)));
      } else {
        cgCallHelper(v, env, CallSpec::direct(deepInitHelper),
                     kVoidDest, SyncOptions::None,
                     args.reg(propInitVec).imm(nprops));
      }
    }
  }
}

void cgLockObj(IRLS& env, const IRInstruction* inst) {
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const mask = ~static_cast<int8_t>(ObjectData::IsBeingConstructed);
  v << andbim{mask, obj[HeaderAuxOffset], v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////

}}}
