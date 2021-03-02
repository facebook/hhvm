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

#ifndef HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_
#define HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/vm/jit/irgen.h"

#include <folly/portability/GMock.h>

namespace HPHP{
namespace bespoke {
namespace testing {

std::atomic<uint16_t> s_num_abstract_layouts;
std::atomic<uint16_t> s_num_concrete_layouts;

struct MockLayout : public Layout {
  MockLayout(const std::string& description, LayoutSet&& parents,
             LayoutIndex idx, bool concrete)
    : Layout(idx, description, std::move(parents), nullptr)
    , m_concrete(concrete)
  {}

  bool isConcrete() const override { return m_concrete; }

private:
  bool m_concrete;
};

inline Layout* makeDummyLayout(const std::string& name,
                               std::vector<jit::ArrayLayout> parents,
                               bool concrete = true) {
  using ::testing::Mock;

  Layout::LayoutSet indices;
  std::transform(
    parents.cbegin(), parents.cend(),
    std::inserter(indices, indices.end()),
    [&](jit::ArrayLayout parent) {
      always_assert(parent.bespoke());
      return *parent.layoutIndex();
    }
  );

  // In order to support type tests, we use a 1-hot encoding to encode leaf
  // concrete layout indices. Abstract layout indices aren't constrained.
  auto const index = [&]() -> LayoutIndex {
    auto constexpr base = kLoggingLayoutByte << 8;
    if (concrete) {
      auto const index = s_num_concrete_layouts++;
      return {safe_cast<uint16_t>(base + (1 << index))};
    }
    auto const index = s_num_abstract_layouts++;
    return {safe_cast<uint16_t>(base + 0xff - index)};
  }();

  auto const ret = new MockLayout(name, std::move(indices), index, concrete);
  Mock::AllowLeak(ret);
  return ret;
}

inline Layout* makeDummyAbstractLayout(const std::string& name,
                                       std::vector<jit::ArrayLayout> parents) {
  return makeDummyLayout(name, parents, false);
}


} // namespace testing
} // namespace bespoke
} // namespace HPHP

#endif // HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_
