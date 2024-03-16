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
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/str-key-table.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/unaligned-typed-value.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-vec-defs.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/cow-profile.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/irlower-bespoke.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

#include "hphp/util/immed.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/trace.h"

// This file does ugly things with macros so include last.
#include "hphp/runtime/vm/jit/irlower-minstr-internal.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgBaseG(IRLS& env, const IRInstruction* inst) {
  auto const mode = inst->extra<MOpModeData>()->mode;
  BUILD_OPTAB(BASE_G_HELPER_TABLE, mode);

  auto const args = argGroup(env, inst).typedValue(0);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgFinishMemberOp(IRLS&, const IRInstruction*) {}

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Make an ArgGroup for prop instructions that takes:
 *    1/ the context Class*
 *    2/ the ObjectData* or a TypedValue, depending on the base type
 */
ArgGroup propArgs(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto args = argGroup(env, inst).immPtr(inst->marker().func());
  if (base->isA(TObj)) return args.ssa(0);
  return args.typedValue(0);
}

void implProp(IRLS& env, const IRInstruction* inst) {
  auto const mode    = inst->extra<PropData>()->mode;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  auto const args = propArgs(env, inst)
    .memberKeyS(1)
    .ssa(2)
    .imm(static_cast<int32_t>(inst->extra<PropData>()->op));

  auto const target = [&] {
    if (inst->is(PropDX)) {
      BUILD_OPTAB2(base->isA(TObj),
                   PROPD_OBJ_HELPER_TABLE,
                   PROPD_HELPER_TABLE,
                   keyType);
      return target;
    } else {
      BUILD_OPTAB2(base->isA(TObj),
                   PROP_OBJ_HELPER_TABLE,
                   PROP_HELPER_TABLE,
                   mode, keyType);
      return target;
    }
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

}

///////////////////////////////////////////////////////////////////////////////

void cgPropX(IRLS& env, const IRInstruction* i)  { implProp(env, i); }
void cgPropDX(IRLS& env, const IRInstruction* i) { implProp(env, i); }

void cgPropQ(IRLS& env, const IRInstruction* inst) {
  using namespace MInstrHelpers;

  auto const args = propArgs(env, inst)
    .ssa(1)
    .ssa(2)
    .imm(static_cast<int32_t>(inst->extra<PropData>()->op));

  auto& v = vmain(env);
  if (inst->src(0)->isA(TObj)) {
    return cgCallHelper(v, env, CallSpec::direct(propCOQ), callDest(env, inst),
                        SyncOptions::Sync, args);
  }

  auto const mode = inst->extra<PropData>()->mode;
  BUILD_OPTAB(PROPQ_HELPER_TABLE, mode);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgCGetProp(IRLS& env, const IRInstruction* inst) {
  auto const mode    = inst->extra<PropData>()->mode;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  BUILD_OPTAB2(base->isA(TObj),
               CGET_OBJ_PROP_HELPER_TABLE,
               CGET_PROP_HELPER_TABLE,
               keyType, mode);

  auto const args = propArgs(env, inst)
    .memberKeyS(1)
    .imm(static_cast<int32_t>(inst->extra<PropData>()->op));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgCGetPropQ(IRLS& env, const IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto const mode = inst->extra<PropData>()->mode;

  auto args = propArgs(env, inst)
    .ssa(1)
    .imm(static_cast<int32_t>(inst->extra<PropData>()->op));

  auto& v = vmain(env);

  if (inst->src(0)->isA(TObj)) {
    return cgCallHelper(v, env, CallSpec::direct(cGetPropSOQ),
                        callDestTV(env, inst), SyncOptions::Sync, args);
  }

  BUILD_OPTAB(CGET_PROPQ_HELPER_TABLE, mode);
  cgCallHelper(v, env, target, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgSetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  auto args = propArgs(env, inst)
    .memberKeyS(1)
    .typedValue(2)
    .imm(static_cast<int32_t>(inst->extra<ReadonlyData>()->op));

  auto const target = [&] {
    if (base->isA(TObj)) {
      BUILD_OPTAB(SETPROP_OBJ_HELPER_TABLE, keyType);
      return target;
    } else {
      BUILD_OPTAB(SETPROP_HELPER_TABLE, keyType);
      return target;
    }
  }();

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgUnsetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::unsetPropCO)
    : CallSpec::direct(MInstrHelpers::unsetPropC);

  auto const args = propArgs(env, inst).typedValue(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, kVoidDest, SyncOptions::Sync, args);
}

void cgSetOpProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<SetOpProp>();

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::setOpPropCO)
    : CallSpec::direct(MInstrHelpers::setOpPropC);

  auto args = propArgs(env, inst)
    .typedValue(1)
    .typedValue(2)
    .imm(static_cast<int32_t>(extra->op));

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDestTV(env, inst), SyncOptions::Sync, args);
}

void cgIncDecProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<IncDecProp>();

  auto helper = base->isA(TObj)
    ? CallSpec::direct(MInstrHelpers::incDecPropCO)
    : CallSpec::direct(MInstrHelpers::incDecPropC);

  auto args = propArgs(env, inst)
    .typedValue(1)
    .imm(static_cast<int32_t>(extra->op));

  auto& v = vmain(env);
  cgCallHelper(v, env, helper, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgIssetProp(IRLS& env, const IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);

  BUILD_OPTAB2(base->isA(TObj),
               ISSET_OBJ_PROP_HELPER_TABLE,
               ISSET_PROP_HELPER_TABLE,
               keyType);

  auto const args = propArgs(env, inst).memberKeyS(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

IMPL_OPCODE_CALL(ProfileProp);

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Make an ArgGroup for elem instructions that takes:
 *    1/ the pointer to an array-like TV
 *    2/ the index key, as a raw value or a TypedValue depending on whether
 *       the type is known
 */
ArgGroup elemArgs(IRLS& env, const IRInstruction* inst) {
  auto args = argGroup(env, inst);
  if (inst->src(0)->isA(TMem)) {
    args.ssa(0);
  } else {
    args.typedValue(0);
  }
  args.memberKeyIS(1);
  return args;
}

void implElem(IRLS& env, const IRInstruction* inst) {
  auto const mode = inst->extra<MOpModeData>()->mode;
  auto const key  = inst->src(1);
  auto const sync = SyncOptions::Sync;
  auto args = elemArgs(env, inst);

  if (inst->is(ElemDX)) {
    assertx(mode == MOpMode::Define);
    BUILD_OPTAB(ELEMD_HELPER_TABLE, getKeyType(key));
    cgCallHelper(vmain(env), env, target, callDest(env, inst), sync, args);
    return;
  }

  if (inst->is(ElemUX)) {
    assertx(mode == MOpMode::Unset);
    BUILD_OPTAB(ELEMU_HELPER_TABLE, getKeyType(key));
    cgCallHelper(vmain(env), env, target, callDest(env, inst), sync, args);
    return;
  }

  BUILD_OPTAB(ELEM_HELPER_TABLE, getKeyType(key), mode);
  cgCallHelper(vmain(env), env, target, callDestTV(env, inst), sync, args);
}

}

///////////////////////////////////////////////////////////////////////////////

void cgElemX(IRLS& env, const IRInstruction* i)  { implElem(env, i); }
void cgElemDX(IRLS& env, const IRInstruction* i) { implElem(env, i); }
void cgElemUX(IRLS& env, const IRInstruction* i) { implElem(env, i); }

void cgCGetElem(IRLS& env, const IRInstruction* inst) {
  auto const mode  = inst->extra<MOpModeData>()->mode;
  auto const key   = inst->src(1);
  BUILD_OPTAB(CGETELEM_HELPER_TABLE, getKeyType(key), mode);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgSetElem(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(SETELEM_HELPER_TABLE, getKeyType(inst->src(1)));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst).typedValue(2));
}

void cgSetRange(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->is(SetRangeRev) ?
    CallSpec::direct(HPHP::SetRange<true>) :
    CallSpec::direct(HPHP::SetRange<false>);
  cgCallHelper(
    vmain(env), env, target, callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst).ssa(0).ssa(1).typedValue(2).ssa(3).ssa(4)
  );
}

void cgSetRangeRev(IRLS& env, const IRInstruction* inst) {
  cgSetRange(env, inst);
}

void cgSetOpElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = CallSpec::direct(MInstrHelpers::setOpElem);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .typedValue(2)
    .imm(uint32_t(inst->extra<SetOpElem>()->op));

  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgIncDecElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const target = CallSpec::direct(MInstrHelpers::incDecElem);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .typedValue(1)
    .imm(uint32_t(inst->extra<IncDecElem>()->op));

  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgUnsetElem(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(UNSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest,
               SyncOptions::Sync, elemArgs(env, inst));
}

void cgIssetElem(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(ISSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::Sync, elemArgs(env, inst));
}

///////////////////////////////////////////////////////////////////////////////
// Offset profiling.

namespace {

void implProfileHackArrayAccess(IRLS& env, const IRInstruction* inst,
                                const CallSpec& target) {
  auto& v = vmain(env);

  auto const extra = inst->extra<RDSHandlePairData>();
  auto args = argGroup(env, inst)
                .ssa(0)
                .ssa(1)
                .addr(rvmtl(), safe_cast<int32_t>(extra->handle));
  if (extra->extra == rds::kUninitHandle) {
    args.immPtr(nullptr);
  } else {
    args.addr(rvmtl(), safe_cast<int32_t>(extra->extra));
  }
  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

}

void cgProfileDictAccess(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(PROFILE_DICT_ACCESS_HELPER_TABLE, getKeyType(inst->src(1)));
  implProfileHackArrayAccess(env, inst, target);
}

void cgProfileKeysetAccess(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(PROFILE_KEYSET_ACCESS_HELPER_TABLE, getKeyType(inst->src(1)));
  implProfileHackArrayAccess(env, inst, target);
}

void cgCheckDictOffset(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const key = srcLoc(env, inst, 1).reg();
  auto const branch = label(env, inst->taken());
  auto const pos = inst->extra<IndexData>()->index;
  auto& v = vmain(env);

  auto const elmOff = VanillaDict::elmOff(pos);
  using Elm = VanillaDict::Elm;

  auto const key_type = getKeyType(inst->src(1));
  auto const is_str_key = key_type == KeyType::Str;
  assertx(key_type != KeyType::Any);

  { // Also fail if our predicted position exceeds bounds. The layout-agnostic
    // variant does a test on the (m_size, m_extra) quadword here; for vanilla
    // array-likes, m_extra is always 0, and for bespoke array-likes, the full
    // quadword is always a negative int64_t.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(pos), arr[VanillaDict::usedOff()], sf};
    ifThen(v, CC_LE, sf, branch);
  }
  { // Fail if the Elm key value doesn't match.
    auto const sf = v.makeReg();
    v << cmpqm{key, arr[elmOff + Elm::keyOff()], sf};
    ifThen(v, CC_NE, sf, branch);
  }
  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, arr[elmOff + Elm::dataOff() + TVOFF(m_aux)], sf};

    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See VanillaDict::isTombstone().
    // We set the key type to look like an int when setting it be a tombstone.
    // This is originally to simplify the type scan, however we can use this
    // to elide this check when looking for string keys.
    if (!is_str_key) {
      auto const sf = v.makeReg();
      v << cmpbim{static_cast<data_type_t>(kInvalidDataType),
                  arr[elmOff + Elm::dataOff() + TVOFF(m_type)], sf};
      ifThen(v, CC_E, sf, branch);
    }
  }
}

void cgCheckKeysetOffset(IRLS& env, const IRInstruction* inst) {
  auto const keyset = srcLoc(env, inst, 0).reg();
  auto const key = srcLoc(env, inst, 1).reg();
  auto const branch = label(env, inst->taken());
  auto const pos = inst->extra<CheckKeysetOffset>()->index;
  auto& v = vmain(env);
  auto const tvOff = pos * (int) sizeof(VanillaKeyset::Elm) +
                     VanillaKeyset::dataOff() + VanillaKeyset::tvOff();

  { // Fail if our predicted position exceeds bounds.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(pos), keyset[VanillaKeyset::usedOff()], sf};
    ifThen(v, CC_LE, sf, branch);
  }
  { // Fail if the Elm key value doesn't match.
    auto const sf = v.makeReg();
    v << cmpqm{key, keyset[tvOff + TVOFF(m_data)], sf};
    ifThen(v, CC_NE, sf, branch);
  }
  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, keyset[tvOff + TVOFF(m_aux)], sf};

    auto const key_type = getKeyType(inst->src(1));
    assertx(key_type != KeyType::Any);

    auto const is_str_key = key_type == KeyType::Str;
    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See VanillaKeyset::isTombstone().
    auto const sf = v.makeReg();
    v << cmpbim{static_cast<data_type_t>(kInvalidDataType),
                keyset[tvOff + TVOFF(m_type)], sf};
    ifThen(v, CC_E, sf, branch);
  }
}

void cgCheckDictKeys(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const mask = ArrayKeyTypes::getMask(inst->typeParam());
  always_assert_flog(mask, "Invalid VanillaDict key check: {}",
                     inst->typeParam().toString());

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << testbim{int8_t(*mask), src[VanillaDict::kKeyTypesOffset], sf};
  v << jcc{CC_NZ, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

namespace {
void traceCheckNotInStrKeyTable(ArrayData* ad, StringData* sd) {
  FTRACE_MOD(Trace::idx, 1,
             "Key: {}, HashMasked: {}, ad: {}\n",
             sd->data(),
             sd->hash() & StrKeyTable::kStrKeyTableMask,
             ad);
  FTRACE_MOD(Trace::idx, 1,
             "HasStrKeyTable: {}\nMay be in table: {}\n",
             ad->hasStrKeyTable(),
             ad->missingKeySideTable().mayContain(sd));
}
} // namespace

void cgCheckMissingKeyInArrLike(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const key = srcLoc(env, inst, 1).reg();
  auto const branch = label(env, inst->taken());
  auto& v = vmain(env);

  if (Trace::moduleEnabled(Trace::idx, 1)) {
    v << copy2{arr, srcLoc(env, inst, 1).reg(), rarg(0), rarg(1)};
    v << call{TCA(traceCheckNotInStrKeyTable), arg_regs(2)};
  }

  // If the array doesn't have the table, jump to branch.
  auto const sf = v.makeReg();
  v << testbim{ArrayData::kHasStrKeyTable, arr[HeaderAuxOffset], sf};
  ifThen(v, CC_Z, sf, branch);

  auto const mask = StrKeyTable::kStrKeyTableMask;
  auto const tableSize = safe_cast<int32_t>(sizeof(StrKeyTable));
  static_assert(sizeof(StrKeyTable) % 8 == 0,
                "Size of StrKeyTable to be a multiple of 8 bytes");
  if (inst->src(1)->hasConstVal(TStaticStr)) {
    // If the key is constant, so is the hash.
    // So, emit a direct 1-byte test for the corresponding bit.
    auto const sfmap = v.makeReg();
    auto const maskedHash = inst->src(1)->strVal()->hash() & mask;
    v << testqim{
      1 << (maskedHash & 7),
      arr[(maskedHash >> 3) - tableSize],
      sfmap
    };
    ifThen(v, CC_NZ, sfmap, branch);
    return;
  }
  auto const hash = v.makeReg();       // sd->hash
  auto const maskedHash = v.makeReg(); // hash & mask
  auto const test = v.makeReg();       // 1 << maskedHash
  auto const sfmap = v.makeReg();
  // Mask the hash to get an index into the StrKeyTable.
  v << load{key[StringData::hashOff()], hash};
  v << andqi{mask, hash, maskedHash, v.makeReg()};
  auto const indexBits = [&] {
    if (tableSize == 8 /* bytes */) return maskedHash;
    auto const reg = v.makeReg();
    // Extract the bottom 6 bits to index into the quadword
    v << andqi{0x3F, maskedHash, reg, v.makeReg()};
    return reg;
  }();
  v << shl{indexBits, v.cns(1), test, v.makeReg()};

  if (tableSize == 8 /* bytes */) {
    // If the bitset only has 64 bits, we can use a single quadword test.
    v << testqm{test, arr[-tableSize], sfmap};
  } else {
    // Otherwise, find the right quadword and apply the test.
    auto const offset = v.makeReg(); // maskedHash >> 6
    v << shrqi{6, maskedHash, offset, v.makeReg()};
    v << testqm{test, arr[offset * 8 - tableSize], sfmap};
  }
  // If the bit is set(i.e. the key may exist in the array), jump to the branch.
  ifThen(v, CC_NZ, sfmap, branch);
}

///////////////////////////////////////////////////////////////////////////////
// COW

void cgCheckArrayCOW(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = emitCmpRefCount(v, OneReference, arr);
  ifThen(v, CC_NE, sf, label(env, inst->taken()));
  v << copy{arr, dst};
}

void cgCopyArray(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const copy = copyFuncForArrayLike(inst->src(0)->type());
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(v, env, copy, callDest(env, inst), SyncOptions::None, args);
}

void cgProfileArrayCOW(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const extra = inst->extra<RDSHandleData>();
  auto const args =
    argGroup(env, inst).addr(rvmtl(), safe_cast<int32_t>(extra->handle)).ssa(0);
  cgCallHelper(
    v,
    env,
    CallSpec::method(&COWProfile::update),
    kVoidDest,
    SyncOptions::None,
    args
  );
}

///////////////////////////////////////////////////////////////////////////////
// Array.

namespace {

VscaledDisp getDictLayoutOffset(IRLS& env, const IRInstruction* inst) {
  auto const pos = srcLoc(env, inst, 2).reg();
  auto& v = vmain(env);
  // We want to index by VanillaDict::Elm but VScaled doesn't let us scale by 24
  // So let's use (3 * pos) * 8 to get sizeof(VanillaDict::Elm) * pos
  auto pos3 = v.makeReg();
  v << lea{pos[pos * 2], pos3};
  static_assert(sizeof(VanillaDict::Elm) == 24);
  return pos3 * 8 + VanillaDict::dataOff() + VanillaDict::Elm::dataOff();
}

VscaledDisp getKeysetLayoutOffset(IRLS& env, const IRInstruction* inst) {
  auto const pos = srcLoc(env, inst, 2).reg();
  auto& v = vmain(env);
  // We want to index by VanillaDict::Elm but VScaled doesn't let us scale by 16
  // So let's use (2 * pos) * 8 to get sizeof(VanillaKeyset::Elm) * pos
  auto pos2 = v.makeReg();
  v << lea{pos[pos], pos2};
  static_assert(sizeof(VanillaKeyset::Elm) == 16);
  return pos2 * 8 + VanillaKeyset::dataOff() + VanillaKeyset::tvOff();
}

} // namespace

void cgGetDictPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const pos_tmp = inst->src(1);
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const pos = srcLoc(env, inst, 1).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  if (pos_tmp->hasConstVal(TInt)) {
    auto const offset = VanillaDict::elmOff(pos_tmp->intVal());
    if (deltaFits(offset, sz::dword)) {
      v << addqi{safe_cast<int32_t>(offset), arr, dst, v.makeReg()};
      return;
    }
  }

  auto const px3 = v.makeReg();
  v << lea{pos[pos * 2], px3};
  v << lea{arr[px3 * 8 + VanillaDict::dataOff()], dst};
}

void cgAdvanceDictPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const extra = inst->extra<AdvanceDictPtrIter>();
  auto const delta = extra->offset * int32_t(sizeof(VanillaDictElm));
  v << addqi{delta, src, dst, v.makeReg()};
}

void cgGetVecPtrIter(IRLS& env, const IRInstruction* inst) {
  assertx(VanillaVec::stores_unaligned_typed_values);

  auto const pos_tmp = inst->src(1);
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const pos = srcLoc(env, inst, 1).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  if (pos_tmp->hasConstVal(TInt)) {
    auto const offset = VanillaVec::entryOffset(pos_tmp->intVal());
    if (deltaFits(offset.data_offset, sz::dword)) {
      v << addqi{safe_cast<int32_t>(offset.data_offset), arr, dst, v.makeReg()};
      return;
    }
  }

  auto const px9 = v.makeReg();
  v << lea{pos[pos * 8], px9};
  v << lea{arr[px9 + VanillaVec::entriesOffset()], dst};
}

void cgAdvanceVecPtrIter(IRLS& env, const IRInstruction* inst) {
  assertx(VanillaVec::stores_unaligned_typed_values);

  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const extra = inst->extra<AdvanceVecPtrIter>();
  auto const delta = extra->offset * int32_t(sizeof(UnalignedTypedValue));
  v << addqi{delta, src, dst, v.makeReg()};
}

void cgLdPtrIterKey(IRLS& env, const IRInstruction* inst) {
  static_assert(sizeof(VanillaDictElm::hash_t) == 4, "");
  auto const elm = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);

  auto& v = vmain(env);
  if (inst->dst(0)->type().needsReg()) {
    assertx(dst.hasReg(1));
    auto const sf = v.makeReg();
    v << cmplim{0, elm[VanillaDictElm::hashOff()], sf};
    v << cmovb{CC_L, sf, v.cns(KindOfString), v.cns(KindOfInt64), dst.reg(1)};
  }
  v << load{elm[VanillaDictElm::keyOff()], dst.reg(0)};
}

void cgLdPtrIterVal(IRLS& env, const IRInstruction* inst) {
  static_assert(VanillaDictElm::dataOff() == 0, "");
  static_assert(TVOFF(m_data) == 0, "");
  auto const elm = srcLoc(env, inst, 0).reg();
  loadTV(vmain(env), inst->dst(0), dstLoc(env, inst, 0), elm[0]);
}

void cgEqPtrIter(IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();
  v << cmpq{s1, s0, sf};
  v << setcc{CC_E, sf, d};
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(SetNewElem);
IMPL_OPCODE_CALL(SetNewElemVec);
IMPL_OPCODE_CALL(SetNewElemDict);

IMPL_OPCODE_CALL(AddNewElemVec);
IMPL_OPCODE_CALL(AddNewElemKeyset);

template <TypedValue (*f)(ArrayData*)>
void containerFirstLastHelper(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(f),
              callDestTV(env, inst), SyncOptions::None,
              argGroup(env, inst).ssa(0));
}

void cgVecFirst(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<vecFirstLast<true>>(env, inst);
}

void cgVecLast(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<vecFirstLast<false>>(env, inst);
}

void cgDictFirst(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<true, false>>(env, inst);
}

void cgDictLast(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<false, false>>(env, inst);
}

void cgDictFirstKey(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<true, true>>(env, inst);
}

void cgDictLastKey(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<false, true>>(env, inst);
}

void cgKeysetFirst(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<true, false>>(env, inst);
}

void cgKeysetLast(IRLS& env, const IRInstruction* inst) {
  containerFirstLastHelper<arrFirstLast<false, false>>(env, inst);
}

///////////////////////////////////////////////////////////////////////////////
// Vec.

irlower::LvalPtrs implVecElemLval(IRLS& env, Vreg rarr,
                                  Vreg ridx, const SSATmp* idx) {
  auto& v = vmain(env);

  static_assert(sizeof(TypedValue) == 16, "");
  static_assert(TVOFF(m_data) == 0, "");

  if (idx && idx->hasConstVal()) {
    auto const offset = VanillaVec::entryOffset(idx->intVal());
    if (deltaFits(offset.type_offset, sz::dword) &&
        deltaFits(offset.data_offset, sz::dword)) {
      return {rarr[offset.type_offset], rarr[offset.data_offset]};
    }
  }

  if constexpr (VanillaVec::stores_unaligned_typed_values) {
    // Compute `rarr + ridx * sizeof(UnalignedTypedValue) + VanillaVec::entriesOffset().
    static_assert(IMPLIES(VanillaVec::stores_unaligned_typed_values, sizeof(UnalignedTypedValue) == 9));
    auto ridx_times_9 = v.makeReg();
    v << lea{ridx[ridx * 8], ridx_times_9};
    auto const type_offset = VanillaVec::entriesOffset() + offsetof(UnalignedTypedValue, m_type);
    auto const data_offset = VanillaVec::entriesOffset() + offsetof(UnalignedTypedValue, m_data);
    return {rarr[ridx_times_9] + type_offset, rarr[ridx_times_9] + data_offset};
  } else {
    // See PackedBlock::LvalAt for an explanation of this math.
    auto const x = v.makeReg();
    auto const a = v.makeReg();
    auto const b = v.makeReg();
    v << andqi{-8, ridx, x, v.makeReg()};
    v << lea{rarr[x    * 8] + VanillaVec::entriesOffset(), a};
    v << lea{rarr[ridx * 8] + VanillaVec::entriesOffset(), b};
    return {a[ridx], b[x] + 8};
  }
}

/*
 * Thread-local RDS packed array access sampling counter.
 */
rds::Link<uint32_t, rds::Mode::Local> s_counter;

void cgLdVecElemAddr(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const idx = srcLoc(env, inst, 1).reg();
  auto const dstLoc = irlower::dstLoc(env, inst, 0);
  auto const addr = implVecElemLval(env, arr, idx, inst->src(1));
  vmain(env) << lea{addr.val, dstLoc.reg(tv_lval::val_idx)};
  vmain(env) << lea{addr.type, dstLoc.reg(tv_lval::type_idx)};
}

void cgLdVecElem(IRLS& env, const IRInstruction* inst) {
  auto const arr = srcLoc(env, inst, 0).reg();
  auto const idx = srcLoc(env, inst, 1).reg();
  auto const addr = implVecElemLval(env, arr, idx, inst->src(1));

  loadTV(vmain(env), inst->dst()->type(), dstLoc(env, inst, 0),
         addr.type, addr.val);
}

void cgReserveVecNewElem(IRLS& env, const IRInstruction* i) {
  static_assert(ArrayData::sizeofSize() == 4, "");

  auto& v = vmain(env);
  auto arrayData = srcLoc(env, i, 0).reg();
  auto const sizePtr = arrayData[ArrayData::offsetofSize()];

  // If the check below succeeds, we'll end up returning the original size
  // so just use the destination register to hold the orignal size
  auto const size = dstLoc(env, i, 0).reg();
  v << loadzlq{sizePtr, size};

  { // Bail out unless size < cap
    auto const indexb = v.makeReg();
    v << loadb{arrayData[VanillaVec::SizeIndexOffset], indexb};
    auto const index = v.makeReg();
    v << movzbq{indexb, index};
    auto const cap = v.makeReg();
    auto const table =
      reinterpret_cast<uintptr_t>(kSizeIndex2VanillaVecCapacity);
    if (table < std::numeric_limits<int>::max()) {
      v << loadzlq{baseless(index * 4 + table), cap};
    } else {
      auto const base = v.cns(table);
      v << loadzlq{base[index * 4], cap};
    }

    auto const sf = v.makeReg();
    v << cmpq{size, cap, sf};
    ifThen(v, CC_BE, sf, label(env, i->taken()));
  }

  v << inclm{sizePtr, v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////
// Dict.

namespace {

void implDictGet(IRLS& env, const IRInstruction* inst, MOpMode mode) {
  assertx(mode == MOpMode::None || mode == MOpMode::Warn);
  BUILD_OPTAB(DICTGET_HELPER_TABLE, getKeyType(inst->src(1)), mode);
  auto const sync = mode == MOpMode::None
    ? SyncOptions::None
    : SyncOptions::Sync;
  auto const args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), sync, args);
}

void implDictSet(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(DICTSET_HELPER_TABLE, getKeyType(inst->src(1)));
  auto const args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

}

void cgElemDictD(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(ELEM_DICT_D_HELPER_TABLE, getKeyType(inst->src(1)));
  auto const args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgElemDictU(IRLS& env, const IRInstruction* inst) {
  BUILD_OPTAB(ELEM_DICT_U_HELPER_TABLE, getKeyType(inst->src(1)));
  auto const args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgElemDictK(IRLS& env, const IRInstruction* inst) {
  auto const dict = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0);

  auto& v = vmain(env);
  auto const off = getDictLayoutOffset(env, inst);

  v << lea{dict[off], dst.reg(tv_lval::val_idx)};
  static_assert(TVOFF(m_data) == 0, "");
  v << lea{dict[off + TVOFF(m_type)], dst.reg(tv_lval::type_idx)};
}

void cgDictGet(IRLS& env, const IRInstruction* inst) {
  implDictGet(env, inst, MOpMode::Warn);
}
void cgDictGetQuiet(IRLS& env, const IRInstruction* inst) {
  implDictGet(env, inst, MOpMode::None);
}

void cgDictGetK(IRLS& env, const IRInstruction* inst) {
  auto const dict = srcLoc(env, inst, 0).reg();
  auto const off = getDictLayoutOffset(env, inst);
  loadTV(vmain(env), inst->dst(0), dstLoc(env, inst, 0), dict[off]);
}

void cgDictSet(IRLS& env, const IRInstruction* i)    { implDictSet(env, i); }

void cgDictIsset(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(DICT_ISSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgDictIdx(IRLS& env, const IRInstruction* inst) {
  auto const def = inst->src(2);
  if (def->isA(TInitNull)) return implDictGet(env, inst, MOpMode::None);
  auto const keyType = getKeyType(inst->src(1));

  auto const target = [&] () -> CallSpec {
    if (keyType == KeyType::Int) return CallSpec::direct(dictIdxI);
    assertx(keyType == KeyType::Str);
    auto const scan =
      inst->extra<SizeHintData>()->hint == SizeHintData::SmallStatic &&
      inst->src(1)->isA(TStaticStr);
    return CallSpec::direct(scan ? dictIdxScan : dictIdxS);
  }();

  auto args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////
// Keyset.

namespace {

void implKeysetGet(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  auto const mode = inst->op() == KeysetGetQuiet
    ? MOpMode::None
    : MOpMode::Warn;
  auto const sync = inst->op() == KeysetGetQuiet
    ? SyncOptions::None
    : SyncOptions::Sync;
  BUILD_OPTAB(KEYSETGET_HELPER_TABLE, getKeyType(key), mode);

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst), sync, args);
}

}

void cgKeysetGet(IRLS& env, const IRInstruction* inst) {
  implKeysetGet(env, inst);
}
void cgKeysetGetQuiet(IRLS& env, const IRInstruction* inst) {
  implKeysetGet(env, inst);
}

void cgKeysetGetK(IRLS& env, const IRInstruction* inst) {
  auto const keyset = srcLoc(env, inst, 0).reg();
  auto const off = getKeysetLayoutOffset(env, inst);
  loadTV(vmain(env), inst->dst(0), dstLoc(env, inst, 0), keyset[off]);
}

void cgSetNewElemKeyset(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  BUILD_OPTAB(KEYSET_SETNEWELEM_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgKeysetIsset(IRLS& env, const IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(KEYSET_ISSET_ELEM_HELPER_TABLE, getKeyType(key));

  auto args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::None, args);
}

void cgKeysetIdx(IRLS& env, const IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const target  = getKeyType(key) == KeyType::Int
    ? CallSpec::direct(keysetIdxI)
    : CallSpec::direct(keysetIdxS);
  auto args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////
// Collections.

IMPL_OPCODE_CALL(PairIsset);
IMPL_OPCODE_CALL(VectorIsset);

void cgVectorSet(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::vectorSetImplI)
    : CallSpec::direct(MInstrHelpers::vectorSetImplS);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .ssa(1)
    .typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgMapGet(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::mapGetImpl<KeyType::Int>)
    : CallSpec::direct(MInstrHelpers::mapGetImpl<KeyType::Str>);

  auto const args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDestTV(env, inst),
               SyncOptions::Sync, args);
}

void cgMapSet(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::mapSetImpl<KeyType::Int>)
    : CallSpec::direct(MInstrHelpers::mapSetImpl<KeyType::Str>);

  auto const args = argGroup(env, inst)
    .ssa(0)
    .ssa(1)
    .typedValue(2);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgMapIsset(IRLS& env, const IRInstruction* inst) {
  auto const target = inst->src(1)->isA(TInt)
    ? CallSpec::direct(MInstrHelpers::mapIssetImpl<KeyType::Int>)
    : CallSpec::direct(MInstrHelpers::mapIssetImpl<KeyType::Str>);

  auto const args = argGroup(env, inst).ssa(0).ssa(1);

  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst),
               SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////

}
