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

#pragma once
#include <thrift/lib/cpp2/reflection/populator.h>
#include <thrift/test/testset/Testing.h>

namespace apache::thrift::test {
namespace detail {
template <class T>
constexpr bool has_adapter() {
  constexpr std::string_view name = folly::pretty_name<T>();
  return name.find("_adapted_") != name.npos;
}
} // namespace detail

template <class T>
T populated_if_not_adapted(
    std::mt19937& rng, populator::populator_opts opt = {}) {
  T result;
  if constexpr (!detail::has_adapter<T>()) {
    populator::populate(result, opt, rng);
  }
  return result;
}
} // namespace apache::thrift::test
