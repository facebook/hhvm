/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THRIFT_FATAL_INTERNAL_TEST_HELPERS_H
#define THRIFT_FATAL_INTERNAL_TEST_HELPERS_H

#include <string>
#include <tuple>
#include <typeinfo>

#include <gtest/gtest.h>

#include <folly/Demangle.h>

namespace apache::thrift::detail {

struct expect_same {
  expect_same(const char* filename, std::size_t line)
      : filename_(filename), line_(line) {}

  template <typename LHS, typename RHS>
  void check() const {
    using type =
        std::tuple<std::string, const char*, std::size_t, const char*, bool>;
    const auto lhs_name = folly::demangle(typeid(LHS));
    const auto rhs_name = folly::demangle(typeid(RHS));
    const auto line_caption = "line: ";
    const type lhs(filename_, line_caption, line_, lhs_name.c_str(), true);
    const type rhs(
        filename_,
        line_caption,
        line_,
        lhs_name == rhs_name ? lhs_name.c_str() : rhs_name.c_str(),
        std::is_same_v<LHS, RHS>);
    EXPECT_EQ(lhs, rhs);
  }

 private:
  std::string const filename_;
  std::size_t const line_;
};

#define EXPECT_SAME \
  ::apache::thrift::detail::expect_same(__FILE__, __LINE__).check

} // namespace apache::thrift::detail

#endif // THRIFT_FATAL_INTERNAL_TEST_HELPERS_H
