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
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/packed-array.h"
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
#include "hphp/runtime/vm/jit/code-gen-internal.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

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

  // ArrayData::m_size is a uint32_t but we need to do a 64-bit comparison
  // since idx is KindOfInt64.
  auto const size = v.makeReg();
  auto const sf = v.makeReg();
  v << loadzlq{arr[ArrayData::offsetofSize()], size};
  v << cmpq{idx, size, sf};
  v << jcc{CC_BE, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

IMPL_OPCODE_CALL(ArrayAdd);

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
      cgCallHelper(v, env, CallSpec::array(&g_array_funcs.vsize),
                   callDest(d), SyncOptions::None,
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

namespace {

template <bool intishWarn>
ALWAYS_INLINE
bool ak_exist_string_impl(const ArrayData* arr, const StringData* key) {
  int64_t n;
  if (arr->convertKey(key, n, intishWarn)) {
    return arr->exists(n);
  }
  return arr->exists(key);
}

}

template <bool intishWarn>
bool ak_exist_string(const ArrayData* arr, const StringData* key) {
  return ak_exist_string_impl<intishWarn>(arr, key);
}

bool ak_exist_int_obj(ObjectData* obj, int64_t key) {
  if (obj->isCollection()) {
    return collections::contains(obj, key);
  }
  auto const arr = obj->toArray();
  return arr.get()->exists(key);
}

bool ak_exist_string_obj(ObjectData* obj, StringData* key) {
  if (obj->isCollection()) {
    return collections::contains(obj, Variant{key});
  }
  auto const arr = obj->toArray();
  return ak_exist_string_impl<false>(arr.get(), key);
}

void cgAKExistsArr(IRLS& env, const IRInstruction* inst) {
  auto const arrTy = inst->src(0)->type();
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain(env);

  auto const keyInfo = checkStrictlyInteger(arrTy, keyTy);
  auto const target =
    keyInfo.checkForInt
      ? (RuntimeOption::EvalHackArrCompatNotices
         ? CallSpec::direct(ak_exist_string<true>)
         : CallSpec::direct(ak_exist_string<false>))
      : (keyInfo.type == KeyType::Int
         ? CallSpec::array(&g_array_funcs.existsInt)
         : CallSpec::array(&g_array_funcs.existsStr));

  auto args = argGroup(env, inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }

  cgCallHelper(
    v, env, target, callDest(env, inst),
    RuntimeOption::EvalHackArrCompatNotices
      ? SyncOptions::Sync : SyncOptions::None,
    args
  );
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

IMPL_OPCODE_CALL(NewArray)
IMPL_OPCODE_CALL(NewMixedArray)
IMPL_OPCODE_CALL(NewLikeArray)
IMPL_OPCODE_CALL(NewDictArray)
IMPL_OPCODE_CALL(AllocPackedArray)
IMPL_OPCODE_CALL(AllocVecArray)

void cgNewStructArray(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<NewStructData>();
  auto& v = vmain(env);

  auto table = v.allocData<const StringData*>(extra->numKeys);
  memcpy(table, extra->keys, extra->numKeys * sizeof(*extra->keys));

  MixedArray* (*f)(uint32_t, const StringData* const*, const TypedValue*) =
    &MixedArray::MakeStruct;

  auto const args = argGroup(env, inst)
    .imm(extra->numKeys)
    .dataPtr(table)
    .addr(sp, cellsToBytes(extra->offset.offset));

  cgCallHelper(v, env, CallSpec::direct(f), callDest(env, inst),
               SyncOptions::None, args);
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
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
