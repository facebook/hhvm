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
#include <string_view>
#include <unordered_map>
#include <utility>
#include <fatal/type/array.h>
#include <fatal/type/foreach.h>
#include <folly/Indestructible.h>
#include <folly/MapUtil.h>
#include <folly/Traits.h>

namespace apache::thrift::reflection {
namespace detail {
using FieldNameToIndex = std::unordered_map<std::string_view, std::size_t>;

template <class Members>
const FieldNameToIndex& fieldNameToIndex() {
  static const folly::Indestructible<FieldNameToIndex> ret = std::invoke([] {
    FieldNameToIndex ret;
    std::size_t index = 0;
    fatal::foreach<Members>([&]<class Member>(Member) {
      ret[fatal::z_data<typename Member::type::name>()] = index++;
    });
    return ret;
  });
  return *ret;
}

template <template <class...> class Members, class... Member, class F>
void invokeByFieldIndex(
    folly::tag_t<Members<Member...>>, F f, std::size_t index) {
  using Func = void (*)(F);
  static constexpr Func funcs[] = {[](F f) { f(fatal::tag<Member>{}); }...};
  funcs[index](f);
}
} // namespace detail

// Members is a list of legacy reflection fields that can be obtained by
//
//    apache::thrift::reflect_struct<MyStruct>::members
//
// This function invokes f(field, args...) by a given field name if such field
// exists. Otherwise it returns `false` without invoking `f`. This is used to
// migrate legacy reflection to always-on reflection.
//
// Semantically it has the same behavior as the following code:
//
//   template <class Members, class... Args>
//   bool invokeByFieldName(std::string_view name, Args&&... args) {
//     return fatal::trie_find<Members, fatal::get_type::name>(
//         name.begin(), name.end(), std::forward<Args>(args)...);
//   }
template <class Members, class F, class... Args>
bool invokeByFieldName(std::string_view name, F&& f, Args&&... args) {
  auto idx = folly::get_ptr(detail::fieldNameToIndex<Members>(), name);
  if (!idx) {
    return false;
  }

  detail::invokeByFieldIndex(
      folly::tag_t<Members>{},
      [&](auto tag) { std::forward<F>(f)(tag, std::forward<Args>(args)...); },
      *idx);

  return true;
}
} // namespace apache::thrift::reflection
