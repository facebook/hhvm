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

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"

#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"

namespace HPHP { namespace jit { namespace irlower {

//////////////////////////////////////////////////////////////////////////////

void cgLogArrayReach(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<LogArrayReach>();

  auto& v = vmain(env);
  auto const args = argGroup(env, inst).imm(data->profile).ssa(0);
  auto const target = CallSpec::method(&bespoke::SinkProfile::update);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgNewLoggingArray(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<NewLoggingArray>();

  auto const target = [&] {
    using Fn = ArrayData*(*)(ArrayData*, bespoke::LoggingProfile*);
    return shouldTestBespokeArrayLikes()
      ? CallSpec::direct(static_cast<Fn>(bespoke::makeBespokeForTesting))
      : CallSpec::direct(static_cast<Fn>(bespoke::maybeMakeLoggingArray));
  }();

  cgCallHelper(vmain(env), env, target, callDest(env, inst), SyncOptions::Sync,
               argGroup(env, inst).ssa(0).immPtr(data->profile));
}

void cgProfileArrLikeProps(IRLS& env, const IRInstruction* inst) {
  auto const target = CallSpec::direct(bespoke::profileArrLikeProps);
  cgCallHelper(vmain(env), env, target, callDest(env, inst), SyncOptions::Sync,
               argGroup(env, inst).ssa(0));
}

//////////////////////////////////////////////////////////////////////////////

void cgBespokeSet(IRLS& env, const IRInstruction* inst) {
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    auto const key = inst->src(1);
    if (layout) {
      auto const vtable = layout->vtable();
      return key->isA(TStr) ? CallSpec::direct(vtable->fnSetStrMove)
                            : CallSpec::direct(vtable->fnSetIntMove);
    } else {
     return key->isA(TStr) ? CallSpec::direct(BespokeArray::SetStrMove)
                           : CallSpec::direct(BespokeArray::SetIntMove);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).ssa(1).typedValue(2);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgBespokeAppend(IRLS& env, const IRInstruction* inst) {
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    if (layout) {
      return CallSpec::direct(layout->vtable()->fnAppendMove);
    } else {
      return CallSpec::direct(BespokeArray::AppendMove);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).typedValue(1);
  auto& v = vmain(env);
  cgCallHelper(v, env, target, callDest(env, inst), SyncOptions::Sync, args);
}

void cgBespokeGet(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dst = dstLoc(env, inst, 0);
  auto const retElem = dst.reg(0);
  auto const retType = dst.reg(1);
  auto const dest = callDest(retElem, retType);
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    auto const key = inst->src(1);
    if (layout) {
      auto const vtable = layout->vtable();
      return key->isA(TStr) ? CallSpec::direct(vtable->fnGetStr)
                            : CallSpec::direct(vtable->fnGetInt);
    } else {
     return key->isA(TStr) ? CallSpec::direct(BespokeArray::NvGetStr)
                           : CallSpec::direct(BespokeArray::NvGetInt);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);

  emitTypeTest(v, env, TUninit, retType, retElem, v.makeReg(),
    [&] (ConditionCode cc, Vreg sf) {
      fwdJcc(v, env, cc, sf, inst->taken());
    }
  );
}

void cgBespokeElem(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDest(env, inst);
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    auto const key = inst->src(1);
    if (layout) {
      auto const vtable = layout->vtable();
      return key->isA(TStr) ? CallSpec::direct(vtable->fnElemStr)
                            : CallSpec::direct(vtable->fnElemInt);
    } else {
      return key->isA(TStr) ? CallSpec::direct(BespokeArray::ElemStr)
                            : CallSpec::direct(BespokeArray::ElemInt);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).ssa(1).ssa(2);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

void cgBespokeIterFirstPos(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDest(env, inst);
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    if (layout) {
      auto const vtable = layout->vtable();
      return CallSpec::direct(vtable->fnIterBegin);
    } else {
      return CallSpec::direct(BespokeArray::IterBegin);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

void cgBespokeIterLastPos(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDest(env, inst);
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    if (layout) {
      auto const vtable = layout->vtable();
      return CallSpec::direct(vtable->fnIterLast);
    } else {
      return CallSpec::direct(BespokeArray::IterLast);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

void cgBespokeIterAdvancePos(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDest(env, inst);
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    if (layout) {
      auto const vtable = layout->vtable();
      return CallSpec::direct(vtable->fnIterAdvance);
    } else {
      return CallSpec::direct(BespokeArray::IterAdvance);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

void cgBespokeIterGetKey(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDestTV(env, inst);
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    if (layout) {
      auto const vtable = layout->vtable();
      return CallSpec::direct(vtable->fnGetPosKey);
    } else {
      return CallSpec::direct(BespokeArray::GetPosKey);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

void cgBespokeIterGetVal(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDestTV(env, inst);
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    if (layout) {
      auto const vtable = layout->vtable();
      return CallSpec::direct(vtable->fnGetPosVal);
    } else {
      return CallSpec::direct(BespokeArray::GetPosVal);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).ssa(1);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

void cgBespokeEscalateToVanilla(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  auto const dest = callDest(env, inst);
  auto const reason = inst->src(1)->strVal()->data();
  auto const target = [&] {
    auto const layout = inst->extra<BespokeLayoutData>()->layout;
    if (layout) {
      auto const vtable = layout->vtable();
      return CallSpec::direct(vtable->fnEscalateToVanilla);
    } else {
      return CallSpec::direct(BespokeArray::ToVanilla);
    }
  }();
  auto const args = argGroup(env, inst).ssa(0).imm(reason);
  cgCallHelper(v, env, target, dest, SyncOptions::Sync, args);
}

//////////////////////////////////////////////////////////////////////////////

}}}
