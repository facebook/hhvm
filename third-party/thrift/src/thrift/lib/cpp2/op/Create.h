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

// Operations supported by all ThriftType values.
#pragma once

#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/detail/Create.h>

namespace apache {
namespace thrift {
namespace op {

/// Returns the default for the given type.
///
/// Some adapted types might not have default constructor. This allows
/// default constructing those adapted types.
///
/// For example:
/// * create<type::i32_t>() -> 0
/// * create<adapted<Adapter, type::i32_t>>() -> Adapted<int32_t>{}
/// * create<field_t<FieldId, adapted<FieldAdapter, type::i32_t>>>(Struct)
///    -> AdaptedWithContext<int32_t>{};
template <typename Tag>
inline constexpr detail::Create<Tag> create{};

/// Ensures the given field. If the field doesn't exist, emplaces the field.
/// For example:
/// * ensure<field_tag>(foo)
///   // calls foo.field_ref().ensure()
template <typename Id = void, typename Tag = void>
inline constexpr detail::Ensure<Id, Tag> ensure{};

} // namespace op
} // namespace thrift
} // namespace apache
