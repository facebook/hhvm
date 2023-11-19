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
#include <thrift/lib/cpp2/op/detail/Copy.h>

namespace apache {
namespace thrift {
namespace op {

// TODO: move to Create.h
/// Copies from the src field to the dst field.
/// For example:
/// * copy(src.field_ref(), dst.field_ref())
///   // calls dst.field_ref().copy_from(src.field_ref())
/// * copy(src.unique_ptr_ref(), dst.unique_ptr_ref())
///   // If src is nullptr, it sets dst to nullptr, otherwise constructs a new
///   // unique ptr with the same value.
/// * copy(src.shared_ptr_ref(), dst.shared_ptr_ref())
///   // If src is nullptr, it sets dst to nullptr, otherwise shares the
///   pointer.
inline constexpr detail::Copy copy{};

} // namespace op
} // namespace thrift
} // namespace apache
