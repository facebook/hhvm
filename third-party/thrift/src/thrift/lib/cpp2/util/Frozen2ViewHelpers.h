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

#include <thrift/lib/cpp2/frozen/Frozen.h>

#include <type_traits>
#include <utility>

namespace apache::thrift::frozen {

template <typename ViewT, typename = void>
struct ViewHelper {
  using ViewType = ViewT;
  using ObjectType = decltype(std::declval<ViewT>().thaw());

  static ObjectType thaw(ViewType v) { return v.thaw(); }
};

template <typename ViewT>
struct ViewHelper<
    ViewT,
    std::enable_if_t<std::is_arithmetic_v<ViewT> || std::is_enum_v<ViewT>>> {
  using ViewType = ViewT;
  using ObjectType = ViewT;

  static ObjectType thaw(ViewType v) { return v; }
};

template <>
struct ViewHelper<folly::Range<const char*>> {
  using ViewType = folly::Range<const char*>;
  using ObjectType = std::string;

  static ObjectType thaw(ViewType v) { return std::string(v.begin(), v.end()); }
};
} // namespace apache::thrift::frozen
