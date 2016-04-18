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

#include "hphp/runtime/vm/jit/code-gen-x64.h"

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-internal.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/mixed-array-offset-profile.h"

#include "hphp/util/trace.h"

// This file does ugly things with macros so include last.
#include "hphp/runtime/vm/jit/minstr-helpers.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(hhir);

///////////////////////////////////////////////////////////////////////////////

void CodeGenerator::cgBaseG(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto const flags = inst->extra<MOpFlagsData>()->flags;
  BUILD_OPTAB(BASE_G_HELPER_TABLE, flags);
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .typedValue(0)
  );
}

///////////////////////////////////////////////////////////////////////////////

void CodeGenerator::cgPropImpl(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto const flags   = inst->extra<MOpFlagsData>()->flags;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(PROP_HELPER_TABLE, flags, keyType, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .memberKeyS(1)
      .ssa(2)
  );
}

void CodeGenerator::cgPropX(IRInstruction* i)     { cgPropImpl(i); }
void CodeGenerator::cgPropDX(IRInstruction* i)    { cgPropImpl(i); }

void CodeGenerator::cgPropQ(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto args =
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .ssa(1)
      .ssa(2);

  auto helper = inst->src(0)->isA(TObj)
    ? CallSpec::direct(propCOQ)
    : CallSpec::direct(propCQ);

  cgCallHelper(
    vmain(),
    helper,
    callDest(inst),
    SyncOptions::Sync,
    args
  );
}

void CodeGenerator::cgCGetProp(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto const flags   = inst->extra<MOpFlagsData>()->flags;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(CGETPROP_HELPER_TABLE, keyType, base->isA(TObj), flags);
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDestTV(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .memberKeyS(1)
  );
}

void CodeGenerator::cgCGetPropQ(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto args =
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .ssa(1);

  if (inst->src(0)->isA(TObj)) {
    cgCallHelper(
      vmain(),
      CallSpec::direct(cGetPropSOQ),
      callDestTV(inst),
      SyncOptions::Sync,
      args
    );
    return;
  }

  cgCallHelper(
    vmain(),
    CallSpec::direct(cGetPropSQ),
    callDestTV(inst),
    SyncOptions::Sync,
    args
  );
}

void CodeGenerator::cgVGetProp(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(VGETPROP_HELPER_TABLE, keyType, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .memberKeyS(1)
  );
}

void CodeGenerator::cgBindProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  BUILD_OPTAB(BINDPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
      .ssa(2)
  );
}

void CodeGenerator::cgSetProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(SETPROP_HELPER_TABLE, keyType, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .memberKeyS(1)
      .typedValue(2)
  );
}

void CodeGenerator::cgUnsetProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  BUILD_OPTAB(UNSETPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
  );
}

void CodeGenerator::cgSetOpProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<SetOpProp>();
  BUILD_OPTAB(SETOPPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDestTV(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
      .typedValue(2)
      .imm(static_cast<int32_t>(extra->op))
  );
}

void CodeGenerator::cgIncDecProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<IncDecProp>();
  BUILD_OPTAB(INCDECPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDestTV(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
      .imm(static_cast<int32_t>(extra->op))
  );
}

void CodeGenerator::cgIssetEmptyPropImpl(IRInstruction* inst) {
  auto const isEmpty = inst->op() == EmptyProp;
  auto const base = inst->src(0);
  auto const key = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(ISSET_EMPTY_PROP_HELPER_TABLE, keyType, isEmpty, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .memberKeyS(1)
  );
}

void CodeGenerator::cgIssetProp(IRInstruction* i) { cgIssetEmptyPropImpl(i); }
void CodeGenerator::cgEmptyProp(IRInstruction* i) { cgIssetEmptyPropImpl(i); }

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Make an ArgGroup for array elem instructions that take the ArrayData* (or
 * pointer to a KindOfArray TV) as the first argument, and the index key as the
 * second.
 */
ArgGroup elemArgs(IRLS& env, const IRInstruction* inst,
                  const ArrayKeyInfo& keyInfo) {
  auto args = argGroup(env, inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }
  return args;
}

ptrdiff_t elmOff(uint32_t pos) {
  return sizeof(MixedArray) + pos * sizeof(MixedArray::Elm);
}

/*
 * JIT helper for profiling MixedArray offsets.
 */
template<bool checkForInt>
void profileMixedArrayOffsetHelper(const ArrayData* ad, int64_t i,
                                   MixedArrayOffsetProfile* prof) {
  prof->update(ad, i);
}
template<bool checkForInt>
void profileMixedArrayOffsetHelper(const ArrayData* ad, const StringData* sd,
                                   MixedArrayOffsetProfile* prof) {
  prof->update(ad, sd, checkForInt);
}

///////////////////////////////////////////////////////////////////////////////

}}

#define PROFILE_MIXED_ARRAY_OFFSET_HELPER_TABLE(m)          \
  /* name                       keyType     checkForInt */  \
  m(profileMixedArrayOffsetS,  KeyType::Str,   false)       \
  m(profileMixedArrayOffsetSi, KeyType::Str,    true)       \
  m(profileMixedArrayOffsetI,  KeyType::Int,   false)       \

#define X(nm, keyType, checkForInt)                               \
static inline void nm(const ArrayData* a, key_type<keyType> k,    \
                      MixedArrayOffsetProfile* p) {               \
  irlower::profileMixedArrayOffsetHelper<checkForInt>(a, k, p);   \
}
namespace MInstrHelpers {
PROFILE_MIXED_ARRAY_OFFSET_HELPER_TABLE(X)
}
#undef X

namespace irlower {

///////////////////////////////////////////////////////////////////////////////

void CodeGenerator::cgProfileMixedArrayOffset(IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());

  BUILD_OPTAB(PROFILE_MIXED_ARRAY_OFFSET_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt);
  auto& v = vmain();

  auto const rprof = v.makeReg();
  v << lea{rvmtl()[inst->extra<ProfileMixedArrayOffset>()->handle], rprof};

  cgCallHelper(v, CallSpec::direct(opFunc), kVoidDest, SyncOptions::Sync,
               elemArgs(m_state, inst, keyInfo).reg(rprof));
}

void CodeGenerator::cgCheckMixedArrayOffset(IRInstruction* inst) {
  auto const arr = srcLoc(inst, 0).reg();
  auto const key = srcLoc(inst, 1).reg();
  auto const branch = label(inst->taken());
  auto const pos = inst->extra<CheckMixedArrayOffset>()->index;
  auto& v = vmain();

  { // Also fail if our predicted position exceeds bounds.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(pos), arr[MixedArray::usedOff()], sf};
    ifThen(v, CC_LE, sf, branch);
  }
  { // Fail if the Elm key value doesn't match.
    auto const sf = v.makeReg();
    v << cmpqm{key, arr[elmOff(pos) + MixedArray::Elm::keyOff()], sf};
    ifThen(v, CC_NE, sf, branch);
  }
  auto const dataOff = elmOff(pos) + MixedArray::Elm::dataOff();

  { // Fail if the Elm key type doesn't match.
    auto const sf = v.makeReg();
    v << cmplim{0, arr[dataOff + TVOFF(m_aux)], sf};

    auto const key_info = checkStrictlyInteger(
      inst->src(0)->type(), // arr
      inst->src(1)->type()  // key
    );
    assertx(key_info.type != KeyType::Any);

    // Note that if `key' actually is an integer-ish string, we'd fail this
    // check (and most likely would have failed the previous check also), but
    // this false negative is allowed.
    auto const is_str_key = key_info.type == KeyType::Str;
    ifThen(v, is_str_key ? CC_L : CC_GE, sf, branch);
  }
  { // Fail if the Elm is a tombstone.  See MixedArray::isTombstone().
    auto const sf = v.makeReg();
    v << cmpbim{KindOfUninit, arr[dataOff + TVOFF(m_type)], sf};
    ifThen(v, CC_L, sf, branch);
  }
}

void CodeGenerator::cgCheckArrayCOW(IRInstruction* inst) {
  auto const arr = srcLoc(inst, 0).reg();
  auto& v = vmain();

  auto const sf = v.makeReg();
  v << cmplim{1, arr[FAST_REFCOUNT_OFFSET], sf};
  ifThen(v, CC_NE, sf, label(inst->taken()));
}

void CodeGenerator::cgElemImpl(IRInstruction* inst) {
  auto const flags = inst->extra<MOpFlagsData>()->flags;
  auto const key   = inst->src(1);

  BUILD_OPTAB(ELEM_HELPER_TABLE, getKeyType(key), flags);
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
      .ssa(2)
  );
}

void CodeGenerator::cgElemX(IRInstruction* i)     { cgElemImpl(i); }
void CodeGenerator::cgElemDX(IRInstruction* i)    { cgElemImpl(i); }
void CodeGenerator::cgElemUX(IRInstruction* i)    { cgElemImpl(i); }

void CodeGenerator::cgElemArrayImpl(IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const flags = inst->op() == ElemArrayW ? MOpFlags::Warn : MOpFlags::None;
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());

  BUILD_OPTAB(
    ELEM_ARRAY_HELPER_TABLE,
    keyInfo.type,
    keyInfo.checkForInt,
    flags
  );

  cgCallHelper(vmain(), CallSpec::direct(opFunc), callDest(inst),
               SyncOptions::Sync, elemArgs(m_state, inst, keyInfo));
}

void CodeGenerator::cgElemArray(IRInstruction* i)  { cgElemArrayImpl(i); }
void CodeGenerator::cgElemArrayW(IRInstruction* i) { cgElemArrayImpl(i); }

void CodeGenerator::cgElemArrayD(IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_D_HELPER_TABLE, keyInfo.type);

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    elemArgs(m_state, inst, keyInfo)
  );
}

void CodeGenerator::cgElemArrayU(IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_U_HELPER_TABLE, keyInfo.type);

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    elemArgs(m_state, inst, keyInfo)
  );
}

void CodeGenerator::cgElemMixedArrayK(IRInstruction* inst) {
  auto const arr = srcLoc(inst, 0).reg();
  auto const dst = dstLoc(inst, 0);
  auto const pos = inst->extra<ElemMixedArrayK>()->index;
  auto const off = elmOff(pos) + MixedArray::Elm::dataOff();

  assertx(dst.numAllocated() == 1);
  vmain() << lea{arr[off], dst.reg()};
}

void CodeGenerator::cgArrayGet(IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());

  BUILD_OPTAB(ARRAYGET_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt);

  cgCallHelper(vmain(), CallSpec::direct(opFunc), callDestTV(inst),
               SyncOptions::Sync, elemArgs(m_state, inst, keyInfo));
}

void CodeGenerator::cgMixedArrayGetK(IRInstruction* inst) {
  auto const arr = srcLoc(inst, 0).reg();
  auto const pos = inst->extra<MixedArrayGetK>()->index;
  auto const off = elmOff(pos) + MixedArray::Elm::dataOff();

  loadTV(vmain(), inst->dst(0), dstLoc(inst, 0), arr[off]);
}

void CodeGenerator::cgMapGet(IRInstruction* inst) {
  auto const target =
    inst->src(1)->isA(TInt)
      ? CallSpec::direct(MInstrHelpers::mapGetImpl<KeyType::Int>)
      : CallSpec::direct(MInstrHelpers::mapGetImpl<KeyType::Str>);
  cgCallHelper(
    vmain(),
    target,
    callDestTV(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
  );
}

void CodeGenerator::cgMapSet(IRInstruction* inst) {
  auto const target =
    inst->src(1)->isA(TInt)
      ? CallSpec::direct(MInstrHelpers::mapSetImpl<KeyType::Int>)
      : CallSpec::direct(MInstrHelpers::mapSetImpl<KeyType::Str>);
  cgCallHelper(
    vmain(),
    target,
    kVoidDest,
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
      .typedValue(2)
  );
}

void CodeGenerator::cgMapIsset(IRInstruction* inst) {
  auto const target =
    inst->src(1)->isA(TInt)
      ? CallSpec::direct(MInstrHelpers::mapIssetImpl<KeyType::Int>)
      : CallSpec::direct(MInstrHelpers::mapIssetImpl<KeyType::Str>);
  cgCallHelper(
    vmain(),
    target,
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
  );
}

void CodeGenerator::cgCGetElem(IRInstruction* inst) {
  auto const flags = inst->extra<MOpFlagsData>()->flags;
  auto const key   = inst->src(1);
  BUILD_OPTAB(CGETELEM_HELPER_TABLE, getKeyType(key), flags);
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDestTV(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
  );
}

void CodeGenerator::cgVGetElem(IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(VGETELEM_HELPER_TABLE, getKeyType(key));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
  );
}

void CodeGenerator::cgArraySetImpl(IRInstruction* inst) {
  bool const setRef  = inst->op() == ArraySetRef;
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ARRAYSET_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt,
              setRef);

  auto args = elemArgs(m_state, inst, keyInfo);
  args.typedValue(2);
  if (setRef) {
    args.ssa(3);
  }

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    args
  );
}

void CodeGenerator::cgArraySet(IRInstruction* i)    { cgArraySetImpl(i); }
void CodeGenerator::cgArraySetRef(IRInstruction* i) { cgArraySetImpl(i); }

void CodeGenerator::cgSetElem(IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(SETELEM_HELPER_TABLE, getKeyType(key));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
      .typedValue(2)
  );
}

void CodeGenerator::cgArrayIsset(IRInstruction* inst) {
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ARRAY_ISSET_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt);

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    elemArgs(m_state, inst, keyInfo)
  );
}

void CodeGenerator::cgUnsetElem(IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(UNSET_ELEM_HELPER_TABLE, getKeyType(key));
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
  );
}

void CodeGenerator::cgIssetEmptyElemImpl(IRInstruction* inst) {
  auto const isEmpty = inst->op() == EmptyElem;
  auto const key     = inst->src(1);
  BUILD_OPTAB(ISSET_EMPTY_ELEM_HELPER_TABLE, getKeyType(key), isEmpty);
  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
  );
}

void CodeGenerator::cgIssetElem(IRInstruction* i) { cgIssetEmptyElemImpl(i); }
void CodeGenerator::cgEmptyElem(IRInstruction* i) { cgIssetEmptyElemImpl(i); }

///////////////////////////////////////////////////////////////////////////////

void CodeGenerator::cgArrayIdx(IRInstruction* inst) {
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());

  auto const target = [&] () -> CallSpec {
    if (keyInfo.checkForInt) {
      return CallSpec::direct(arrayIdxSi);
    }
    if (keyInfo.type == KeyType::Int) {
      return CallSpec::direct(arrayIdxI);
    }
    return CallSpec::direct(arrayIdxS);
  }();

  cgCallHelper(
    vmain(),
    target,
    callDestTV(inst),
    SyncOptions::Sync,
    elemArgs(m_state, inst, keyInfo).typedValue(2)
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
