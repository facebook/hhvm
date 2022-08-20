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
#include <thrift/lib/cpp2/op/detail/Ensure.h>

namespace apache {
namespace thrift {
namespace op {

// TODO: move to Create.h
// Ensures the given field. If the field doesn't exist, emplaces the field.
// For example:
//   // calls foo.field_ref().ensure()
//   ensure<field_tag>(foo.field_ref(), foo)
//   // constructs a smart pointer if doesn't exist.
//   ensure<field_tag>(foo.smart_ptr_ref(), foo)
template <typename Tag>
FOLLY_INLINE_VARIABLE constexpr detail::Ensure<Tag> ensure{};

} // namespace op
} // namespace thrift
} // namespace apache
