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

#include <cstddef>
#include <type_traits>
#include <utility>
#include <folly/CPortability.h>

namespace apache {
namespace thrift {
namespace detail {

template <typename IntegerSequence>
struct foreach_;

template <std::size_t... I>
struct foreach_<std::index_sequence<I...>> {
  template <typename F, typename... O>
  FOLLY_ERASE static void go(F&& f, O&&... o) {
    using _ = int[];
    void(_{
        (void(f(std::integral_constant<std::size_t, I>{}, std::forward<O>(o))),
         0)...,
        0});
  }
};

template <typename F, typename... O>
FOLLY_ERASE void foreach(F&& f, O&&... o) {
  using seq = std::make_index_sequence<sizeof...(O)>;
  foreach_<seq>::go(std::forward<F>(f), std::forward<O>(o)...);
}

template <typename F, std::size_t... I>
FOLLY_ERASE void foreach_index_(F&& f, std::index_sequence<I...>) {
  foreach_<std::index_sequence<I...>>::go(std::forward<F>(f), I...);
}

template <std::size_t Size, typename F>
FOLLY_ERASE void foreach_index(F&& f) {
  using seq = std::make_index_sequence<Size>;
  foreach_index_([&](auto _, auto) { f(_); }, seq{});
}

} // namespace detail
} // namespace thrift
} // namespace apache
