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

#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/packed-array.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/collections/hash-collection.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/type-specialization.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

void implColTestSize(IRLS& env, const IRInstruction* inst, ConditionCode cc) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assertx(ty < TObj &&
          ty.clsSpec().cls() &&
          ty.clsSpec().cls()->isCollectionClass());

  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmplim{0, src[collections::FAST_SIZE_OFFSET], sf};
  v << setcc{cc, sf, dst};
}

}

void cgIsCol(IRLS& env, const IRInstruction* inst) {
  assertx(inst->src(0)->type() <= TObj);

  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << testwim{ObjectData::IsCollection, src[ObjectData::attributeOff()], sf};
  v << setcc{CC_NE, sf, dst};
}

void cgColIsEmpty(IRLS& env, const IRInstruction* inst) {
  implColTestSize(env, inst, CC_E);
}

void cgColIsNEmpty(IRLS& env, const IRInstruction* inst) {
  implColTestSize(env, inst, CC_NE);
}

void cgCountCollection(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst  = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << loadzlq{src[collections::FAST_SIZE_OFFSET], dst};
}

void cgNewCol(IRLS& env, const IRInstruction* inst) {
  auto const target = [&] {
    auto const col_type = inst->extra<NewCol>()->type;
    auto const helper = collections::allocEmptyFunc(col_type);
    return CallSpec::direct(helper);
  }();
  cgCallHelper(vmain(env), env, target, callDest(env, inst),
               SyncOptions::Sync, argGroup(env, inst));
}

IMPL_OPCODE_CALL(NewPair)

void cgNewColFromArray(IRLS& env, const IRInstruction* inst) {
  auto const target = [&] {
    auto const col_type = inst->extra<NewColFromArray>()->type;
    auto const helper = collections::allocFromArrayFunc(col_type);
    return CallSpec::direct(helper);
  }();
  cgCallHelper(vmain(env), env, target, callDest(env, inst),
               SyncOptions::Sync, argGroup(env, inst).ssa(0));
}

void cgLdColVec(IRLS& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();
  auto const cls = ty.clsSpec().cls();

  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  always_assert_flog(
    ty == TBottom ||
    collections::isType(cls, CollectionType::Vector, CollectionType::ImmVector),
    "LdColVec received an unsupported type: {}\n",
    ty.toString()
  );

  v << load{src[BaseVector::arrOffset()], dst};
}

void cgLdColDict(IRLS& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();
  auto const cls = ty.clsSpec().cls();

  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  always_assert_flog(
    ty == TBottom ||
    collections::isType(cls,
                        CollectionType::Map, CollectionType::ImmMap,
                        CollectionType::Set, CollectionType::ImmSet),
    "LdColDict received an unsupported type: {}\n",
    ty.toString()
  );

  v << load{src[HashCollection::arrOffset()], dst};
}

///////////////////////////////////////////////////////////////////////////////
// Vector

namespace {

void assertHasVectorSrc(const IRInstruction* inst, bool imm_allowed) {
  DEBUG_ONLY auto const vec = inst->src(0);
  assertx(vec->type() < TObj);

  if (imm_allowed) {
    assertx(collections::isType(vec->type().clsSpec().cls(),
                                CollectionType::Vector,
                                CollectionType::ImmVector));
  } else {
    assertx(collections::isType(vec->type().clsSpec().cls(),
                                CollectionType::Vector));
  }
}

}

void cgLdVectorSize(IRLS& env, const IRInstruction* inst) {
  assertHasVectorSrc(inst, true);

  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << loadzlq{src[BaseVector::sizeOffset()], dst};
}

void cgLdVectorBase(IRLS& env, const IRInstruction* inst) {
  assertHasVectorSrc(inst, true);

  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const arr = v.makeReg();
  v << load{src[BaseVector::arrOffset()], arr};
  v << lea{arr[PackedArray::entriesOffset()], dst};
}

void cgVectorHasImmCopy(IRLS& env, const IRInstruction* inst) {
  assertHasVectorSrc(inst, false);

  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const arr = v.makeReg();
  v << load{src[BaseVector::arrOffset()], arr};
  auto const sf = emitCmpRefCount(v, OneReference, arr);
  v << jcc{CC_NE, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgVectorDoCow(IRLS& env, const IRInstruction* inst) {
  assertHasVectorSrc(inst, false);

  cgCallHelper(vmain(env), env, CallSpec::direct(triggerCow),
               kVoidDest, SyncOptions::Sync, argGroup(env, inst).ssa(0));
}

///////////////////////////////////////////////////////////////////////////////
// Pair

void cgLdPairBase(IRLS& env, const IRInstruction* inst) {
  DEBUG_ONLY auto const pair = inst->src(0);
  assertx(pair->type() < TObj);
  assertx(collections::isType(pair->type().clsSpec().cls(),
                              CollectionType::Pair));

  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  vmain(env) << lea{src[c_Pair::dataOffset()], dst};
}

///////////////////////////////////////////////////////////////////////////////

}}}
