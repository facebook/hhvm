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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/record-data.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/array-kind-profile.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void profileArrayKindHelper(ArrayKindProfile* profile, ArrayData* arr) {
  profile->report(arr->kind());
}

void cgProfileArrayKind(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RDSHandleData>();
  assertx(!rds::isPersistentHandle(extra->handle));
  auto& v = vmain(env);

  auto const profile = v.makeReg();
  v << lea{rvmtl()[extra->handle], profile};
  cgCallHelper(v, env, CallSpec::direct(profileArrayKindHelper), kVoidDest,
               SyncOptions::None, argGroup(env, inst).reg(profile).ssa(0));
}

void cgCheckPackedArrayDataBounds(IRLS& env, const IRInstruction* inst) {
  static_assert(ArrayData::sizeofSize() == 4, "");

  // We may check packed array bounds on profiled arrays that we do not
  // statically know have kPackedKind.
  assertx(inst->taken());
  auto arr = srcLoc(env, inst, 0).reg();
  auto idx = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const size = [&]{
    auto const arrTmp = inst->src(0);
    if (arrTmp->hasConstVal()) return v.cns(arrTmp->arrLikeVal()->size());
    auto const at = arrTmp->type().arrSpec().type();
    using A = RepoAuthType::Array;
    if (at && at->tag() == A::Tag::Packed && at->emptiness() == A::Empty::No) {
      return v.cns(at->size());
    }
    // ArrayData::m_size is a uint32_t but we need to do a 64-bit comparison
    // since idx is KindOfInt64.
    auto const size = v.makeReg();
    v << loadzlq{arr[ArrayData::offsetofSize()], size};
    return size;
  }();

  auto const sf = v.makeReg();
  v << cmpq{idx, size, sf};
  v << jcc{CC_BE, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

IMPL_OPCODE_CALL(ArrayAdd);

////////////////////////////////////////////////////////////////////////////////

namespace {

ArrayData* setLegacyHelper(ArrayData* arr) {
  if (arr->cowCheck()) {
    auto ad = arr->copy();
    arr->decRefCount();
    ad->setLegacyArray(true);
    return ad;
  } else {
    arr->setLegacyArray(true);
    return arr;
  }
}

void setLegacyImpl(IRLS& env, const IRInstruction* inst) {
  auto const args = argGroup(env, inst).ssa(0);

  cgCallHelper(vmain(env),
               env,
               CallSpec::direct(setLegacyHelper),
               callDest(env, inst),
               SyncOptions::None,
               args);
}

}

void cgSetLegacyVec(IRLS& env, const IRInstruction* inst) {
  setLegacyImpl(env, inst);
}

void cgSetLegacyDict(IRLS& env, const IRInstruction* inst) {
  setLegacyImpl(env, inst);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void implCountArrayLike(IRLS& env, const IRInstruction* inst) {
  static_assert(ArrayData::sizeofSize() == 4, "");
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  vmain(env) << loadzlq{src[ArrayData::offsetofSize()], dst};
}

}

IMPL_OPCODE_CALL(Count)

void cgCountArray(IRLS& env, const IRInstruction* inst) {
  auto const& arr_type = inst->src(0)->type();
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const d = v.makeReg();
  auto const sf = v.makeReg();
  v << loadl{src[ArrayData::offsetofSize()], d};
  v << testl{d, d, sf};

  unlikelyCond(
    v, vcold(env), CC_S, sf, dst,
    [&](Vout& v) {
      auto const d = v.makeReg();
      auto const call = CallSpec::array(
        arr_type, &g_array_funcs.vsize, &ArrayData::size);
      cgCallHelper(v, env, call, callDest(d), SyncOptions::None,
                   argGroup(env, inst).ssa(0));
      return d;
    },
    [&](Vout& /*v*/) { return d; });
}

void cgCountArrayFast(IRLS& env, const IRInstruction* inst) {
  implCountArrayLike(env, inst);
}

void cgCountVec(IRLS& env, const IRInstruction* inst) {
  implCountArrayLike(env, inst);
}

void cgCountDict(IRLS& env, const IRInstruction* inst) {
  implCountArrayLike(env, inst);
}

void cgCountKeyset(IRLS& env, const IRInstruction* inst) {
  implCountArrayLike(env, inst);
}

///////////////////////////////////////////////////////////////////////////////
// AKExists.

bool ak_exist_int_obj(ObjectData* obj, int64_t key) {
  if (obj->isCollection()) {
    return collections::contains(obj, key);
  } else if (obj->instanceof(c_Closure::classof())) {
    return false;
  }
  auto const arr = obj->toArray(false, true);
  return arr.get()->exists(key);
}

bool ak_exist_string_obj(ObjectData* obj, StringData* key) {
  if (obj->isCollection()) {
    return collections::contains(obj, Variant{key});
  } else if (obj->instanceof(c_Closure::classof())) {
    return false;
  }
  auto const arr = obj->toArray(false, true);
  return arr.get()->exists(key);
}

void cgAKExistsArr(IRLS& env, const IRInstruction* inst) {
  auto const& arr_type = inst->src(0)->type();
  auto const& key_type = inst->src(1)->type();
  auto& v = vmain(env);

  using ExistsInt = bool (ArrayData::*)(int64_t) const;
  using ExistsStr = bool (ArrayData::*)(const StringData*) const;

  assertx(key_type.subtypeOfAny(TInt, TStr));
  auto const target = (key_type <= TInt)
    ? CallSpec::array(arr_type, &g_array_funcs.existsInt,
                      static_cast<ExistsInt>(&ArrayData::exists))
    : CallSpec::array(arr_type, &g_array_funcs.existsStr,
                      static_cast<ExistsStr>(&ArrayData::exists));

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None,
               argGroup(env, inst).ssa(0).ssa(1));
}

void cgAKExistsDict(IRLS& env, const IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain(env);

  auto const target = (keyTy <= TInt)
    ? CallSpec::direct(MixedArray::ExistsIntDict)
    : CallSpec::direct(MixedArray::ExistsStrDict);

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None,
               argGroup(env, inst).ssa(0).ssa(1));
}

void cgAKExistsKeyset(IRLS& env, const IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain(env);

  auto const target = (keyTy <= TInt)
    ? CallSpec::direct(SetArray::ExistsInt)
    : CallSpec::direct(SetArray::ExistsStr);

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None,
               argGroup(env, inst).ssa(0).ssa(1));
}

void cgAKExistsObj(IRLS& env, const IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain(env);

  auto const target = (keyTy <= TInt)
    ? CallSpec::direct(ak_exist_int_obj)
    : CallSpec::direct(ak_exist_string_obj);

  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync,
               argGroup(env, inst).ssa(0).ssa(1));
}

///////////////////////////////////////////////////////////////////////////////
// Array creation.

namespace {

using MakeArrayFn = ArrayData*(uint32_t);

void implNewArray(IRLS& env, const IRInstruction* inst, MakeArrayFn target,
                  SyncOptions sync = SyncOptions::None) {
  cgCallHelper(vmain(env), env, CallSpec::direct(target), callDest(env, inst),
               sync, argGroup(env, inst).ssa(0));
}

void implAllocArray(IRLS& env, const IRInstruction* inst, MakeArrayFn target,
                    SyncOptions sync = SyncOptions::None) {
  auto const extra = inst->extra<PackedArrayData>();
  cgCallHelper(vmain(env), env, CallSpec::direct(target), callDest(env, inst),
               sync, argGroup(env, inst).imm(extra->size));
}

}

void cgNewArray(IRLS& env, const IRInstruction* inst) {
  implNewArray(env, inst, PackedArray::MakeReserve);
}
void cgNewMixedArray(IRLS& env, const IRInstruction* inst) {
  implNewArray(env, inst, MixedArray::MakeReserveMixed);
}
void cgNewDictArray(IRLS& env, const IRInstruction* inst) {
  implNewArray(env, inst, MixedArray::MakeReserveDict);
}
void cgNewDArray(IRLS& env, const IRInstruction* inst) {
  implNewArray(env, inst, MixedArray::MakeReserveDArray);
}

void cgAllocPackedArray(IRLS& env, const IRInstruction* inst) {
  implAllocArray(env, inst, PackedArray::MakeUninitialized);
}
void cgAllocVecArray(IRLS& env, const IRInstruction* inst) {
  implAllocArray(env, inst, PackedArray::MakeUninitializedVec);
}
void cgAllocVArray(IRLS& env, const IRInstruction* inst) {
  implAllocArray(env, inst, PackedArray::MakeUninitializedVArray);
}

void cgNewLikeArray(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(vmain(env), env, CallSpec::direct(MixedArray::MakeReserveLike),
               callDest(env, inst), SyncOptions::None,
               argGroup(env, inst).ssa(0).ssa(1));
}

namespace {

void newStructImpl(
  IRLS& env,
  const IRInstruction* inst,
  MixedArray* (*f)(uint32_t, const StringData* const*, const TypedValue*),
  SyncOptions sync = SyncOptions::None
) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<NewStructData>();
  auto& v = vmain(env);

  auto table = v.allocData<const StringData*>(extra->numKeys);
  memcpy(table, extra->keys, extra->numKeys * sizeof(*extra->keys));

  auto const args = argGroup(env, inst)
    .imm(extra->numKeys)
    .dataPtr(table)
    .addr(sp, cellsToBytes(extra->offset.offset));

  cgCallHelper(v, env, CallSpec::direct(f), callDest(env, inst), sync, args);
}

}

void cgNewStructArray(IRLS& env, const IRInstruction* inst) {
  newStructImpl(env, inst, MixedArray::MakeStruct);
}

void cgNewStructDArray(IRLS& env, const IRInstruction* inst) {
  newStructImpl(env, inst, MixedArray::MakeStructDArray);
}

void cgNewStructDict(IRLS& env, const IRInstruction* inst) {
  newStructImpl(
    env, inst, MixedArray::MakeStructDict,
    RuntimeOption::EvalArrayProvenance ? SyncOptions::Sync : SyncOptions::None
  );
}

void cgNewKeysetArray(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<NewKeysetArray>();
  auto& v = vmain(env);

  auto const args = argGroup(env, inst)
    .imm(extra->size)
    .addr(sp, cellsToBytes(extra->offset.offset));

  cgCallHelper(v, env, CallSpec::direct(SetArray::MakeSet),
               callDest(env, inst), SyncOptions::Sync, args);
}

namespace {

template<typename ArrayInit>
void allocStructImpl(
  IRLS& env,
  const IRInstruction* inst,
  MixedArray* (*f)(uint32_t, const int32_t*),
  SyncOptions sync = SyncOptions::None
) {
  arrprov::TagOverride ap_override{arrprov::tagFromSK(inst->marker().sk())};
  auto const extra = inst->extra<NewStructData>();
  auto init = ArrayInit{extra->numKeys};
  for (auto i = 0; i < extra->numKeys; ++i) {
    init.set(extra->keys[i], make_tv<KindOfNull>());
  }
  auto const array = init.toArray();
  auto const ad = MixedArray::asMixed(array.get());

  auto const scale = MixedArray::computeScaleFromSize(extra->numKeys);
  always_assert(MixedArray::HashSize(scale) == ad->hashSize());

  using HashTableEntry = std::remove_pointer_t<decltype(ad->hashTab())>;

  auto& v = vmain(env);
  auto table = v.allocData<HashTableEntry>(ad->hashSize());
  memcpy(table, ad->hashTab(), ad->hashSize() * sizeof(HashTableEntry));

  auto const args = argGroup(env, inst).imm(extra->numKeys).dataPtr(table);
  cgCallHelper(v, env, CallSpec::direct(f), callDest(env, inst), sync, args);
}

}

void cgAllocStructArray(IRLS& env, const IRInstruction* inst) {
  allocStructImpl<MixedArrayInit>(env, inst, MixedArray::AllocStruct);
}

void cgAllocStructDArray(IRLS& env, const IRInstruction* inst) {
  allocStructImpl<DArrayInit>(env, inst, MixedArray::AllocStructDArray);
}

void cgAllocStructDict(IRLS& env, const IRInstruction* inst) {
  allocStructImpl<DictInit>(
    env, inst, MixedArray::AllocStructDict,
    RuntimeOption::EvalArrayProvenance ? SyncOptions::Sync : SyncOptions::None
  );
}

void cgInitMixedLayoutArray(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const key = inst->extra<InitMixedLayoutArray>()->key;
  auto const idx = inst->extra<InitMixedLayoutArray>()->index;

  auto const elm_off  = MixedArray::elmOff(idx);
  auto const key_ptr  = arr[elm_off + MixedArrayElm::keyOff()];
  auto const data_ptr = arr[elm_off + MixedArrayElm::dataOff()];
  auto const hash_ptr = arr[elm_off + MixedArrayElm::hashOff()];

  auto& v = vmain(env);
  storeTV(v, data_ptr, srcLoc(env, inst, 1), inst->src(1));
  v << storeli { key->hash(), hash_ptr };
  v << store { v.cns(key), key_ptr };
}

void cgInitPackedLayoutArray(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const index = inst->extra<InitPackedLayoutArray>()->index;

  auto const slot_off = PackedArray::entriesOffset() +
                        index * sizeof(TypedValue);
  storeTV(vmain(env), arr[slot_off], srcLoc(env, inst, 1), inst->src(1));
}

void cgInitPackedLayoutArrayLoop(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const spIn = srcLoc(env, inst, 1).reg();
  auto const extra = inst->extra<InitPackedLayoutArrayLoop>();
  auto const count = safe_cast<int>(extra->size);
  auto& v = vmain(env);

  auto const sp = v.makeReg();
  v << lea{spIn[cellsToBytes(extra->offset.offset)], sp};

  auto const i = v.cns(0);
  auto const j = v.cns((count - 1) * 2);

  // We know that we have at least one element in the array so we don't have to
  // do an initial bounds check.
  assertx(count);

  doWhile(v, CC_GE, {i, j},
    [&] (const VregList& in, const VregList& out) {
      auto const i1 = in[0],  j1 = in[1];
      auto const i2 = out[0], j2 = out[1];
      auto const sf = v.makeReg();
      auto const value = v.makeReg();

      // Load the value from the stack and store into the array.  It's safe to
      // copy all 16 bytes of the TV because packed arrays don't use m_aux.
      v << loadups{sp[j1 * 8], value};
      v << storeups{value, arr[i1 * 8] + PackedArray::entriesOffset()};

      // Add 2 to the loop variable because we can only scale by at most 8.
      v << lea{i1[2], i2};
      v << subqi{2, j1, j2, sf};
      return sf;
    },
    count
  );
}

namespace {

template<typename Fn>
void newRecordImpl(IRLS& env, const IRInstruction* inst, Fn creatorFn) {
  auto const rec = srcLoc(env, inst, 0).reg();
  auto const sp = srcLoc(env, inst, 1).reg();
  auto const extra = inst->extra<NewStructData>();
  auto& v = vmain(env);

  auto table = v.allocData<const StringData*>(extra->numKeys);
  memcpy(table, extra->keys, extra->numKeys * sizeof(*extra->keys));

  auto const args = argGroup(env, inst)
    .reg(rec)
    .imm(extra->numKeys)
    .dataPtr(table)
    .addr(sp, cellsToBytes(extra->offset.offset));

  cgCallHelper(v, env, CallSpec::direct(creatorFn),
               callDest(env, inst),
               SyncOptions::Sync, args);
}

}

void cgNewRecord(IRLS& env, const IRInstruction* inst) {
  newRecordImpl(env, inst, RecordData::newRecord);
}

void cgNewRecordArray(IRLS& env, const IRInstruction* inst) {
  newRecordImpl(env, inst, RecordArray::newRecordArray);
}

///////////////////////////////////////////////////////////////////////////////

}}}
