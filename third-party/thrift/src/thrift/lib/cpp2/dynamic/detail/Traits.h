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

#include <folly/Traits.h>
#include <folly/memory/not_null.h>

#include <cstddef>
#include <variant>

namespace apache::thrift::type_system::detail {

template <typename Variant, typename T>
struct IndexOfImpl;

template <typename T, typename... Types>
struct IndexOfImpl<std::variant<Types...>, T> {
  static constexpr std::size_t value = folly::type_pack_find_v<T, Types...>;
};

/**
 * Given a std::variant<Types...> and a type T, returns the index of T in the
 * type parameter pack of the variant. If T is not in the type parameter pack,
 * then the result is sizeof...(Types).
 *
 * Examples:
 *
 *     IndexOf<std::variant<int, float, std::string>, int>         == 0
 *     IndexOf<std::variant<int, float, std::string>, float>       == 1
 *     IndexOf<std::variant<int, float, std::string>, std::string> == 2
 *     IndexOf<std::variant<int, float, std::string>, double>      == 3
 */
template <typename Variant, typename T>
inline constexpr std::size_t IndexOf = IndexOfImpl<Variant, T>::value;

template <typename T>
T& maybe_deref(T& t) {
  return t;
}
template <typename T>
T& maybe_deref(folly::not_null<T*> t) {
  return *t;
}

} // namespace apache::thrift::type_system::detail
