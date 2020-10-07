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
#include "hphp/runtime/base/bespoke-array.h"
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

void cgCheckVecBounds(IRLS& env, const IRInstruction* inst) {
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

void cgCheckVecBoundsLA(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const idx = inst->src(1)->intVal();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmpqim{safe_cast<int32_t>(idx), arr[ArrayData::offsetofSize()], sf};
  v << jcc{CC_LE, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

////////////////////////////////////////////////////////////////////////////////

namespace {

ArrayData* setLegacyHelper(ArrayData* ad, bool legacy) {
  auto const result = ad->setLegacyArray(ad->cowCheck(), legacy);
  if (result != ad) decRefArr(ad);
  return result;
}

void setLegacyImpl(IRLS& env, const IRInstruction* inst, bool set) {
  auto const args = argGroup(env, inst).ssa(0).imm(set);
  auto const target = CallSpec::direct(setLegacyHelper);
  cgCallHelper(vmain(env), env, target, callDest(env, inst),
               SyncOptions::None, args);
}

}

void cgSetLegacyVec(IRLS& env, const IRInstruction* inst) {
  setLegacyImpl(env, inst, true);
}

void cgSetLegacyDict(IRLS& env, const IRInstruction* inst) {
  setLegacyImpl(env, inst, true);
}

void cgUnsetLegacyVec(IRLS& env, const IRInstruction* inst) {
  setLegacyImpl(env, inst, false);
}

void cgUnsetLegacyDict(IRLS& env, const IRInstruction* inst) {
  setLegacyImpl(env, inst, false);
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

void cgAKExistsDict(IRLS& env, const IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain(env);

  auto const target = (keyTy <= TInt)
    ? CallSpec::direct(MixedArray::ExistsInt)
    : CallSpec::direct(MixedArray::ExistsStr);

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

void cgNewLoggingArray(IRLS& env, const IRInstruction* inst) {
  auto const target = [&] {
    if (shouldTestBespokeArrayLikes()) {
      return CallSpec::direct(
        static_cast<ArrayData*(*)(ArrayData*)>(bespoke::makeBespokeForTesting));
    } else {
      return CallSpec::direct(
        static_cast<ArrayData*(*)(ArrayData*)>(bespoke::maybeMakeLoggingArray));
    }
  }();
  cgCallHelper(vmain(env), env, target, callDest(env, inst),
               SyncOptions::Sync, argGroup(env, inst).ssa(0));
}

namespace {

using MakeArrayFn = ArrayData*(uint32_t);

folly::Optional<arrprov::Tag> getProvenanceTag(const IRInstruction* inst) {
  if (!RO::EvalArrayProvenance) return folly::none;
  if (inst->marker().func()->isProvenanceSkipFrame()) return folly::none;
  return arrprov::tagFromSK(inst->marker().sk());
}

template <typename T, typename U>
T convertAsBytes(U u) {
  static_assert(std::is_trivially_copyable<T>::value);
  static_assert(std::is_trivially_copyable<U>::value);
  static_assert(sizeof(T) == sizeof(U));
  T t;
  std::memcpy(&t, &u, sizeof(T));
  return t;
}

ArrayData* newTaggedDArray(uint32_t size, uint32_t tagImm) {
  arrprov::TagOverride _(convertAsBytes<arrprov::Tag>(tagImm));
  return MixedArray::MakeReserveDArray(size);
}

ArrayData* allocTaggedVArray(uint32_t size, uint32_t tagImm) {
  arrprov::TagOverride _(convertAsBytes<arrprov::Tag>(tagImm));
  return PackedArray::MakeUninitializedVArray(size);
}

void implNewTaggedDArray(IRLS& env, const IRInstruction* inst,
                         arrprov::Tag tag) {
  auto const tagImm = convertAsBytes<uint32_t>(tag);
  cgCallHelper(vmain(env), env, CallSpec::direct(newTaggedDArray),
               callDest(env, inst), SyncOptions::None,
               argGroup(env, inst).ssa(0).imm(tagImm));
}

void implAllocTaggedVArray(IRLS& env, const IRInstruction* inst,
                           arrprov::Tag tag) {
  auto const extra = inst->extra<PackedArrayData>();
  auto const tagImm = convertAsBytes<uint32_t>(tag);
  cgCallHelper(vmain(env), env, CallSpec::direct(allocTaggedVArray),
               callDest(env, inst), SyncOptions::None,
               argGroup(env, inst).imm(extra->size).imm(tagImm));
}

void implNewArray(IRLS& env, const IRInstruction* inst, MakeArrayFn target) {
  cgCallHelper(vmain(env), env, CallSpec::direct(target), callDest(env, inst),
               SyncOptions::None, argGroup(env, inst).ssa(0));
}

void implAllocArray(IRLS& env, const IRInstruction* inst, MakeArrayFn target) {
  auto const extra = inst->extra<PackedArrayData>();
  cgCallHelper(vmain(env), env, CallSpec::direct(target), callDest(env, inst),
               SyncOptions::None, argGroup(env, inst).imm(extra->size));
}

}

void cgNewDictArray(IRLS& env, const IRInstruction* inst) {
  implNewArray(env, inst, MixedArray::MakeReserveDict);
}
void cgNewDArray(IRLS& env, const IRInstruction* inst) {
  if (auto const tag = getProvenanceTag(inst)) {
    return implNewTaggedDArray(env, inst, *tag);
  }
  implNewArray(env, inst, MixedArray::MakeReserveDArray);
}

void cgAllocVec(IRLS& env, const IRInstruction* inst) {
  implAllocArray(env, inst, PackedArray::MakeUninitializedVec);
}
void cgAllocVArray(IRLS& env, const IRInstruction* inst) {
  if (auto const tag = getProvenanceTag(inst)) {
    return implAllocTaggedVArray(env, inst, *tag);
  }
  implAllocArray(env, inst, PackedArray::MakeUninitializedVArray);
}

namespace {

ArrayData* newTaggedStructDArray(uint32_t size, const StringData* const* keys,
                                 const TypedValue* values, uint32_t tagImm) {
  arrprov::TagOverride _(convertAsBytes<arrprov::Tag>(tagImm));
  return MixedArray::MakeStructDArray(size, keys, values);
}

void newStructImpl(
  IRLS& env,
  const IRInstruction* inst,
  MixedArray* (*f)(uint32_t, const StringData* const*, const TypedValue*),
  folly::Optional<arrprov::Tag> tag = folly::none
) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<NewStructData>();
  auto& v = vmain(env);

  auto table = v.allocData<const StringData*>(extra->numKeys);
  memcpy(table, extra->keys, extra->numKeys * sizeof(*extra->keys));

  auto const sync = SyncOptions::None;
  auto args = argGroup(env, inst)
    .imm(extra->numKeys)
    .dataPtr(table)
    .addr(sp, cellsToBytes(extra->offset.offset));

  if (tag) {
    assertx(inst->is(NewStructDArray));
    args.imm(convertAsBytes<uint32_t>(*tag));
    auto const target = CallSpec::direct(newTaggedStructDArray);
    cgCallHelper(v, env, target, callDest(env, inst), sync, args);
    return;
  }

  cgCallHelper(v, env, CallSpec::direct(f), callDest(env, inst), sync, args);
}

}

void cgNewStructDArray(IRLS& env, const IRInstruction* inst) {
  auto const tag = getProvenanceTag(inst);
  newStructImpl(env, inst, MixedArray::MakeStructDArray, tag);
}

void cgNewStructDict(IRLS& env, const IRInstruction* inst) {
  newStructImpl(env, inst, MixedArray::MakeStructDict);
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

ArrayData* allocTaggedStructDArray(uint32_t size, const int32_t* hash,
                                   uint32_t tagImm) {
  arrprov::TagOverride _(convertAsBytes<arrprov::Tag>(tagImm));
  return MixedArray::AllocStructDArray(size, hash);
}

template<typename ArrayInit>
void allocStructImpl(
  IRLS& env,
  const IRInstruction* inst,
  MixedArray* (*f)(uint32_t, const int32_t*),
  folly::Optional<arrprov::Tag> tag = folly::none
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

  auto const sync = SyncOptions::None;
  auto args = argGroup(env, inst).imm(extra->numKeys).dataPtr(table);

  if (tag) {
    assertx(inst->is(AllocStructDArray));
    args.imm(convertAsBytes<uint32_t>(*tag));
    auto const target = CallSpec::direct(allocTaggedStructDArray);
    cgCallHelper(v, env, target, callDest(env, inst), sync, args);
    return;
  }

  cgCallHelper(v, env, CallSpec::direct(f), callDest(env, inst), sync, args);
}

}

void cgAllocStructDArray(IRLS& env, const IRInstruction* inst) {
  auto const tag = getProvenanceTag(inst);
  allocStructImpl<DArrayInit>(env, inst, MixedArray::AllocStructDArray, tag);
}

void cgAllocStructDict(IRLS& env, const IRInstruction* inst) {
  allocStructImpl<DictInit>(env, inst, MixedArray::AllocStructDict);
}

void cgInitDictElem(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const key = inst->extra<InitDictElem>()->key;
  auto const idx = inst->extra<InitDictElem>()->index;

  auto const elm_off  = MixedArray::elmOff(idx);
  auto const key_ptr  = arr[elm_off + MixedArrayElm::keyOff()];
  auto const data_ptr = arr[elm_off + MixedArrayElm::dataOff()];
  auto const hash_ptr = arr[elm_off + MixedArrayElm::hashOff()];

  auto& v = vmain(env);
  storeTV(v, data_ptr, srcLoc(env, inst, 1), inst->src(1));
  v << storeli { key->hash(), hash_ptr };
  v << store { v.cns(key), key_ptr };
}

void cgInitVecElem(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const index = inst->extra<InitVecElem>()->index;

  auto const slot_off = PackedArray::entriesOffset() +
                        index * sizeof(TypedValue);
  storeTV(vmain(env), arr[slot_off], srcLoc(env, inst, 1), inst->src(1));
}

void cgInitVecElemLoop(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const spIn = srcLoc(env, inst, 1).reg();
  auto const extra = inst->extra<InitVecElemLoop>();
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

namespace {
void arrayReach(ArrayData* ad, TransID transId, uint32_t guardIdx) {
  if (LIKELY(ad->isVanilla())) return;
  BespokeArray::asBespoke(ad)->logReachEvent(transId, guardIdx);
}
}

void cgLogArrayReach(IRLS& env, const IRInstruction* inst) {
  auto data = inst->extra<LogArrayReach>();

  auto& v = vmain(env);
  auto const args = argGroup(env, inst)
    .ssa(0).imm(data->transId).imm(data->guardIdx);

  auto const target = CallSpec::direct(arrayReach);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////

}}}
