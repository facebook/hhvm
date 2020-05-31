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

#include "folly/SharedMutex.h"
#include "folly/Synchronized.h"

#include <atomic>
#include <limits>
#include <vector>

namespace HPHP { namespace bespoke {

namespace {
auto constexpr kMaxNumLayouts = 1 << 16;
folly::Synchronized<std::vector<Layout*>, folly::SharedMutex> s_layoutTable;
}

Layout::Layout() {
  auto table = s_layoutTable.wlock();
  m_index = table->size();
  always_assert(m_index < kMaxNumLayouts);
  table->push_back(this);
}

const Layout* layoutForIndex(uint32_t index) {
  auto const layout = s_layoutTable.rlock()->at(index);
  assertx(layout->index() == index);
  return layout;
}

}}
