/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/code-gen-helpers.h"

// This file does ugly things with macros so include last.
#include "hphp/runtime/vm/jit/minstr-helpers.h"

namespace HPHP { namespace jit { namespace x64 {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgBaseG(IRInstruction* inst) {
  using namespace MInstrHelpers;
  using F = TypedValue* (*)(TypedValue);
  static const F opFuncs[] = { baseG, baseGW, baseGD, baseGWD };
  auto const mia = inst->extra<BaseG>()->mia;
  cgCallHelper(
    vmain(),
    CppCall::direct(opFuncs[mia & MIA_base]),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .typedValue(0)
  );
}

void CodeGenerator::cgPropImpl(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto const mia = inst->extra<MInstrAttrData>()->mia;
  BUILD_OPTAB(PROP_HELPER_TABLE, mia, inst->src(0)->isA(TObj));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
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

  if (inst->src(0)->isA(TObj)) {
    cgCallHelper(
      vmain(),
      CppCall::direct(propCOQ),
      callDest(inst),
      SyncOptions::kSyncPoint,
      args
    );
    return;
  }

  cgCallHelper(
    vmain(),
    CppCall::direct(propCQ),
    callDest(inst),
    SyncOptions::kSyncPoint,
    args
  );
}


void CodeGenerator::cgCGetProp(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto const base    = inst->src(0);
  auto const key     = inst->src(1);
  auto const keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(CGETPROP_HELPER_TABLE, keyType, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .memberKeyS(1)
      .ssa(2)
  );
}

void CodeGenerator::cgCGetPropQ(IRInstruction* inst) {
  using namespace MInstrHelpers;
  auto args =
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .ssa(1)
      .ssa(2);

  if (inst->src(0)->isA(TObj)) {
    cgCallHelper(
      vmain(),
      CppCall::direct(cGetPropSOQ),
      callDestTV(inst),
      SyncOptions::kSyncPoint,
      args
    );
    return;
  }

  cgCallHelper(
    vmain(),
    CppCall::direct(cGetPropSQ),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
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
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .memberKeyS(1)
      .ssa(2)
  );
}

void CodeGenerator::cgBindProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  BUILD_OPTAB(BINDPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
      .ssa(2)
      .ssa(3)
  );
}

void CodeGenerator::cgSetProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  BUILD_OPTAB(SETPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    kVoidDest,
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
      .typedValue(2)
  );
}

void CodeGenerator::cgUnsetProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  BUILD_OPTAB(UNSETPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    kVoidDest,
    SyncOptions::kSyncPoint,
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
    CppCall::direct(opFunc),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
      .typedValue(2)
      .ssa(3)
      .imm(static_cast<int32_t>(extra->op))
  );
}

void CodeGenerator::cgIncDecProp(IRInstruction* inst) {
  auto const base = inst->src(0);
  auto const extra = inst->extra<IncDecProp>();
  BUILD_OPTAB(INCDECPROP_HELPER_TABLE, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
      .imm(static_cast<int32_t>(extra->op))
  );
}

void CodeGenerator::cgIssetEmptyPropImpl(IRInstruction* inst) {
  bool const isEmpty = inst->op() == EmptyProp;
  auto const base = inst->src(0);
  BUILD_OPTAB(ISSET_EMPTY_PROP_HELPER_TABLE, isEmpty, base->isA(TObj));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .immPtr(getClass(inst->marker()))
      .ssa(0)
      .typedValue(1)
  );
}

void CodeGenerator::cgIssetProp(IRInstruction* i) { cgIssetEmptyPropImpl(i); }
void CodeGenerator::cgEmptyProp(IRInstruction* i) { cgIssetEmptyPropImpl(i); }

void CodeGenerator::cgElemImpl(IRInstruction* inst) {
  auto const mia = inst->extra<MInstrAttrData>()->mia;
  auto const key = inst->src(1);
  BUILD_OPTAB(ELEM_HELPER_TABLE, getKeyType(key), mia);
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
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
  auto const key     = inst->src(1);
  bool const warn    = inst->op() == ElemArrayW;
  auto const keyInfo = checkStrictlyInteger(key->type());
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
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    args
  );
}

void CodeGenerator::cgElemArray(IRInstruction* i)  { cgElemArrayImpl(i); }
void CodeGenerator::cgElemArrayW(IRInstruction* i) { cgElemArrayImpl(i); }

void CodeGenerator::cgArrayGet(IRInstruction* inst) {
  auto const key         = inst->src(1);
  auto const keyInfo     = checkStrictlyInteger(key->type());
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
    CppCall::direct(opFunc),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    args
  );
}

void CodeGenerator::cgMapGet(IRInstruction* inst) {
  auto const target =
    inst->src(1)->isA(TInt)
      ? CppCall::direct(MInstrHelpers::mapGetImpl<KeyType::Int>)
      : CppCall::direct(MInstrHelpers::mapGetImpl<KeyType::Str>);
  cgCallHelper(
    vmain(),
    target,
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
  );
}

void CodeGenerator::cgMapSet(IRInstruction* inst) {
  auto const target =
    inst->src(1)->isA(TInt)
      ? CppCall::direct(MInstrHelpers::mapSetImpl<KeyType::Int>)
      : CppCall::direct(MInstrHelpers::mapSetImpl<KeyType::Str>);
  cgCallHelper(
    vmain(),
    target,
    kVoidDest,
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
      .typedValue(2)
  );
}

void CodeGenerator::cgMapIsset(IRInstruction* inst) {
  auto const target =
    inst->src(1)->isA(TInt)
      ? CppCall::direct(MInstrHelpers::mapIssetImpl<KeyType::Int>)
      : CppCall::direct(MInstrHelpers::mapIssetImpl<KeyType::Str>);
  cgCallHelper(
    vmain(),
    target,
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
  );
}

void CodeGenerator::cgCGetElem(IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(CGETELEM_HELPER_TABLE, getKeyType(key));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
      .ssa(2)
  );
}

void CodeGenerator::cgVGetElem(IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(VGETELEM_HELPER_TABLE, getKeyType(key));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
      .ssa(2)
  );
}

void CodeGenerator::cgArraySetImpl(IRInstruction* inst) {
  bool const setRef  = inst->op() == ArraySetRef;
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(key->type());
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
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
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
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
      .typedValue(2)
  );
}

void CodeGenerator::cgArrayIsset(IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(key->type());
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
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    args
  );
}

void CodeGenerator::cgUnsetElem(IRInstruction* inst) {
  auto const key = inst->src(1);
  BUILD_OPTAB(UNSET_ELEM_HELPER_TABLE, getKeyType(key));
  cgCallHelper(
    vmain(),
    CppCall::direct(opFunc),
    kVoidDest,
    SyncOptions::kSyncPoint,
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
    CppCall::direct(opFunc),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .memberKeyIS(1)
      .ssa(2)
  );
}

void CodeGenerator::cgIssetElem(IRInstruction* i) { cgIssetEmptyElemImpl(i); }
void CodeGenerator::cgEmptyElem(IRInstruction* i) { cgIssetEmptyElemImpl(i); }

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgArrayIdx(IRInstruction* inst) {
  auto const key     = inst->src(1);
  auto const keyInfo = checkStrictlyInteger(key->type());

  auto const target = [&] () -> CppCall {
    if (keyInfo.checkForInt) {
      return CppCall::direct(arrayIdxSi);
    }
    if (keyInfo.type == KeyType::Int) {
      return CppCall::direct(arrayIdxI);
    }
    return CppCall::direct(arrayIdxS);
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
    SyncOptions::kSyncPoint,
    args
  );
}

//////////////////////////////////////////////////////////////////////

}}}
