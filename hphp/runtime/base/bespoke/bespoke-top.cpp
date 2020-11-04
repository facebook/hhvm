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

#include "hphp/runtime/base/bespoke/bespoke-top.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/punt.h"

namespace HPHP { namespace bespoke {

using namespace jit;
using namespace jit::irgen;

namespace {
BespokeTop* s_layout;
}

BespokeTop::BespokeTop(): Layout("BespokeTop", {}, true) {}

void BespokeTop::InitializeLayouts() {
  s_layout = new BespokeTop();
}

LayoutIndex BespokeTop::GetLayoutIndex() {
  return s_layout->index();
}

SSATmp* BespokeTop::emitElem(
    IRGS& env, SSATmp* lval, SSATmp* key, bool throwOnMissing) const {
  auto const data = BespokeLayoutData { nullptr };
  return gen(env, BespokeElem, TCell, data, lval, key, cns(env,throwOnMissing));
}

SSATmp* BespokeTop::emitGet(IRGS& env, SSATmp* arr, SSATmp* key,
                            Block* taken) const {
  return gen(env, BespokeGet, TCell, BespokeLayoutData { nullptr }, taken, arr,
             key);
}

SSATmp* BespokeTop::emitSet(IRGS& env, SSATmp* arr, SSATmp* key,
                            SSATmp* val) const {
  auto const outputType = arr->type().unspecialize().modified();
  return gen(env, BespokeSet, outputType, BespokeLayoutData { nullptr }, arr,
             key, val);
}

SSATmp* BespokeTop::emitAppend(IRGS& env, SSATmp* arr,
                               SSATmp* val) const {
  auto const outputType = arr->type().unspecialize().modified();
  return gen(env, BespokeAppend, outputType, BespokeLayoutData { nullptr }, arr,
             val);
}

SSATmp* BespokeTop::emitIterFirstPos(IRGS& env, SSATmp* arr) const {
  auto const data = BespokeLayoutData { nullptr };
  return gen(env, BespokeIterFirstPos, data, arr);
}

SSATmp* BespokeTop::emitIterLastPos(IRGS& env, SSATmp* arr) const {
  auto const data = BespokeLayoutData { nullptr };
  return gen(env, BespokeIterLastPos, data, arr);
}

SSATmp* BespokeTop::emitIterPos(IRGS& env, SSATmp* arr, SSATmp* idx) const {
  PUNT(unimpl_bespoke_iterpos);
}

SSATmp* BespokeTop::emitIterAdvancePos(
    IRGS& env, SSATmp* arr, SSATmp* pos) const {
  auto const data = BespokeLayoutData { nullptr };
  return gen(env, BespokeIterAdvancePos, data, arr, pos);
}

SSATmp* BespokeTop::emitIterElm(IRGS& env, SSATmp* arr, SSATmp* pos) const {
  return pos;
}

SSATmp* BespokeTop::emitIterGetKey(IRGS& env, SSATmp* arr, SSATmp* elm) const {
  auto const data = BespokeLayoutData { nullptr };
  auto const retType = arr->type().subtypeOfAny(TVec, TVArr) ? TInt : TInt|TStr;
  return gen(env, BespokeIterGetKey, retType, data, arr, elm);
}

SSATmp* BespokeTop::emitIterGetVal(IRGS& env, SSATmp* arr, SSATmp* elm) const {
  auto const data = BespokeLayoutData { nullptr };
  return gen(env, BespokeIterGetVal, TCell, data, arr, elm);
}

}}
