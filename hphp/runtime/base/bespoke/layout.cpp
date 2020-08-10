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

#include <atomic>
#include <array>

namespace HPHP { namespace bespoke {

namespace {
auto constexpr kMaxNumLayouts = 1 << 16;

std::array<Layout*, kMaxNumLayouts> s_layoutTable;
}

Layout::Layout() {
  static std::atomic<uint64_t> s_layoutTableIndex;
  m_index = s_layoutTableIndex ++;
  always_assert(m_index < kMaxNumLayouts);
  s_layoutTable[m_index] = this;
}

const Layout* layoutForIndex(uint32_t index) {
  auto const layout = s_layoutTable[index];
  assertx(layout->index() == index);
  return layout;
}

}}
