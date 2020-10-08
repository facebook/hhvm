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

#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke/monotype-vec.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/punt.h"

#include <atomic>
#include <array>
#include <folly/lang/Bits.h>

namespace HPHP { namespace bespoke {

using namespace jit;
using namespace jit::irgen;

namespace {
std::atomic<size_t> s_layoutTableIndex;
std::array<Layout*, Layout::kMaxIndex.raw + 1> s_layoutTable;
}

Layout::Layout(const std::string& description, const LayoutFunctions* vtable)
    : m_index(ReserveIndices(1)), m_description(description), m_vtable(vtable) {
  assertx(s_layoutTable[m_index.raw] == nullptr);
  s_layoutTable[m_index.raw] = this;
}

Layout::Layout(LayoutIndex index, const std::string& description,
               const LayoutFunctions* vtable)
    : m_index(index), m_description(description), m_vtable(vtable) {
  assertx(s_layoutTable[m_index.raw] == nullptr);
  s_layoutTable[m_index.raw] = this;
}

LayoutIndex Layout::ReserveIndices(size_t count) {
  assertx(folly::isPowTwo(count));
  auto const padded_count = 2 * count - 1;
  auto const base = s_layoutTableIndex.fetch_add(padded_count);
  always_assert(base + padded_count <= kMaxIndex.raw + 1);
  for (auto i = 0; i < padded_count; i++) {
    s_layoutTable[base + i] = nullptr;
  }
  auto const result = (base + count - 1) & ~(count - 1);
  assertx(result % count == 0);
  return {safe_cast<uint16_t>(result)};
}

const Layout* Layout::FromIndex(LayoutIndex index) {
  auto const layout = s_layoutTable[index.raw];
  assertx(layout != nullptr);
  assertx(layout->index() == index);
  return layout;
}

SSATmp* Layout::emitGet(
    IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const {
  PUNT(unimpl_bespoke_emitGet);
}

SSATmp* Layout::emitSet(
    IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const {
  PUNT(unimpl_bespoke_emitSet);
}

SSATmp* Layout::emitAppend(IRGS& env, SSATmp* arr, SSATmp* val) const {
  PUNT(unimpl_bespoke_emitAppend);
}

struct Layout::Initializer {
  Initializer() {
    assertx(s_layoutTableIndex.load(std::memory_order_relaxed) == 0);
    LoggingArray::InitializeLayouts();
    MonotypeVec::InitializeLayouts();
  }
};
Layout::Initializer Layout::s_initializer;

}}
