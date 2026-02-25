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

#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/op/detail/PatchTraits.h>

namespace apache::thrift::op {

template <typename T, typename = void>
inline constexpr bool is_patch_v = false;

template <typename T>
inline constexpr bool
    is_patch_v<T, folly::void_t<typename T::underlying_type>> =
        std::is_base_of_v<detail::BasePatch<typename T::underlying_type, T>, T>;

template <typename Patch>
using patched_type_tag_t = detail::PatchedTypeTag<Patch>::type;
} // namespace apache::thrift::op
