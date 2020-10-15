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

#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"

namespace HPHP { namespace jit { namespace irlower {

void cgBespokeSet(IRLS& env, const IRInstruction* inst) {
  // TODO(mcolavita): layout-based dispatch when we have move layout methods
  auto const target = [&] {
    if (inst->src(1)->isA(TStr)) {
      return CallSpec::direct(BespokeArray::SetStrMove);
    } else {
      assertx(inst->src(1)->isA(TInt));
      return CallSpec::direct(BespokeArray::SetIntMove);
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
      return CallSpec::direct(layout->vtable()->fnAppend);
    } else {
      return CallSpec::direct(BespokeArray::Append);
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
    if (inst->src(1)->isA(TStr)) {
      return CallSpec::direct(BespokeArray::NvGetStr);
    } else {
      assertx(inst->src(1)->isA(TInt));
      return CallSpec::direct(BespokeArray::NvGetInt);
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

}}}
