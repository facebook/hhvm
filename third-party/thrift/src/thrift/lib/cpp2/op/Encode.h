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

#include <thrift/lib/cpp2/op/detail/Encode.h>

namespace apache {
namespace thrift {
namespace op {

/// Returns the serialized size of the avlue using the type tag.
/// For example: serialized_size<false, type::int16_t>(prot, 1);
template <bool ZeroCopy, typename Tag>
inline constexpr detail::SerializedSize<ZeroCopy, Tag> serialized_size{};

/// Encodes the given value to the given protocol using the type tag.
/// This handles adapted type.
/// For example: encode<type::int16_t>(prot, 1);
template <typename Tag>
inline constexpr detail::Encode<Tag> encode{};

/// Decodes the value from the given protocol using the type tag.
/// This handles adapted type.
/// For example: decode<type::int16_t>(prot, i); // decode to variable i
template <typename Tag>
inline constexpr detail::Decode<Tag> decode{};

} // namespace op
} // namespace thrift
} // namespace apache
