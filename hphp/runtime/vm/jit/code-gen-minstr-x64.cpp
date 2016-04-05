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

#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-internal.h"

#include "hphp/util/trace.h"

// This file does ugly things with macros so include last.
#include "hphp/runtime/vm/jit/minstr-helpers.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

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
  auto const mia     = inst->extra<MInstrAttrData>()->mia;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(CGETPROP_HELPER_TABLE, keyType, base->isA(TObj), mia);
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
  auto const arr     = inst->src(0);
  auto const key     = inst->src(1);
  bool const warn    = inst->op() == ElemArrayW;
  auto const keyInfo = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt,
              warn);

  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    args
  );
}

void CodeGenerator::cgElemArray(IRInstruction* i)  { cgElemArrayImpl(i); }
void CodeGenerator::cgElemArrayW(IRInstruction* i) { cgElemArrayImpl(i); }

void CodeGenerator::cgElemArrayD(IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_D_HELPER_TABLE, keyInfo.type);

  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    args
  );
}

void CodeGenerator::cgElemArrayU(IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(inst->typeParam(), key->type());
  BUILD_OPTAB(ELEM_ARRAY_U_HELPER_TABLE, keyInfo.type);

  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    args
  );
}

void CodeGenerator::cgArrayGet(IRInstruction* inst) {
  auto const arr         = inst->src(0);
  auto const key         = inst->src(1);
  auto const keyInfo     = checkStrictlyInteger(arr->type(), key->type());
  BUILD_OPTAB(ARRAYGET_HELPER_TABLE,
              keyInfo.type,
              keyInfo.checkForInt);

  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDestTV(inst),
    SyncOptions::Sync,
    args
  );
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
  auto const mia = inst->extra<MInstrAttrData>()->mia;
  auto const key = inst->src(1);
  BUILD_OPTAB(CGETELEM_HELPER_TABLE, getKeyType(key), mia);
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

  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }
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

  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }

  cgCallHelper(
    vmain(),
    CallSpec::direct(opFunc),
    callDest(inst),
    SyncOptions::Sync,
    args
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

//////////////////////////////////////////////////////////////////////

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

  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }
  args.typedValue(2);

  cgCallHelper(
    vmain(),
    target,
    callDestTV(inst),
    SyncOptions::Sync,
    args
  );
}

//////////////////////////////////////////////////////////////////////

}}}
