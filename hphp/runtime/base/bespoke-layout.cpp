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

#include "hphp/runtime/base/bespoke-layout.h"

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP {

using namespace jit;
using namespace jit::irgen;

BespokeLayout BespokeLayout::FromIndex(uint16_t index) {
  return BespokeLayout{bespoke::Layout::FromIndex({index})};
}

uint16_t BespokeLayout::index() const {
  return m_layout->index().raw;
}

const std::string& BespokeLayout::describe() const {
  return m_layout->describe();
}

namespace {
bool checkLayoutMatches(const bespoke::Layout* layout, SSATmp* arr) {
  auto const DEBUG_ONLY layoutType =
    arr->type().unspecialize().narrowToBespokeLayout(BespokeLayout(layout));
  assertx(arr->type() <= layoutType);

  return true;
}
}

SSATmp* BespokeLayout::emitGet(
    IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const {
  assertx(checkLayoutMatches(m_layout, arr));
  return m_layout->emitGet(env, arr, key, taken);
}

SSATmp* BespokeLayout::emitSet(
    IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const {
  assertx(checkLayoutMatches(m_layout, arr));
  return m_layout->emitSet(env, arr, key, val);
}

SSATmp* BespokeLayout::emitAppend(IRGS& env, SSATmp* arr, SSATmp* val) const {
  assertx(checkLayoutMatches(m_layout, arr));
  return m_layout->emitAppend(env, arr, val);
}

}

