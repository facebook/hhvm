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
#include <type_traits>

namespace apache::thrift {

template <class>
struct IsString : std::false_type {};
template <class>
struct IsHashMap : std::false_type {};
template <class>
struct IsHashSet : std::false_type {};
template <class>
struct IsOrderedMap : std::false_type {};
template <class>
struct IsOrderedSet : std::false_type {};
template <class>
struct IsList : std::false_type {};
template <class>
struct HasSortedUniqueCtor : std::false_type {};

namespace frozen {
template <class>
struct IsExcluded : std::false_type {};
} // namespace frozen

} // namespace apache::thrift

#define THRIFT_DECLARE_TRAIT(Trait, ...)         \
  namespace apache {                             \
  namespace thrift {                             \
  template <>                                    \
  struct Trait<__VA_ARGS__> : std::true_type {}; \
  }                                              \
  }

#define THRIFT_DECLARE_TRAIT_TEMPLATE(Trait, ...)         \
  namespace apache {                                      \
  namespace thrift {                                      \
  template <class... Args>                                \
  struct Trait<__VA_ARGS__<Args...>> : std::true_type {}; \
  }                                                       \
  }
