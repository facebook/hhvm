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

#include <utility>
#include <variant>

namespace apache::thrift::compiler::detail {

template <typename...>
struct overload_t {};

template <typename Func, typename... Funcs>
struct overload_t<Func, Funcs...> : overload_t<Funcs...>, Func {
  explicit constexpr overload_t(Func f, Funcs... fs)
      : overload_t<Funcs...>(std::move(fs)...), Func(std::move(f)) {}

  using Func::operator();
  using overload_t<Funcs...>::operator();
};

template <typename Func>
struct overload_t<Func> : Func {
  explicit constexpr overload_t(Func c) : Func(std::move(c)) {}
  using Func::operator();
};

template <class... Funcs>
decltype(auto) overload(Funcs&&... funcs) {
  return overload_t<std::decay_t<Funcs>...>{std::forward<Funcs>(funcs)...};
}

template <class Variant, class... Funcs>
decltype(auto) variant_match(Variant&& variant, Funcs&&... funcs) {
  return std::visit(
      overload(std::forward<Funcs>(funcs)...), std::forward<Variant>(variant));
}

} // namespace apache::thrift::compiler::detail
