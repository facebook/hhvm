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

// The mappings from Thrift type tags to native c++ types.
#pragma once

#include <thrift/lib/cpp2/type/detail/NativeType.h>

namespace apache::thrift::type {

// The default standard type associated with the given concrete ThriftType.
//
// This is the type that is used by default, to represent the given ThriftType.
template <typename Tag>
using standard_type = typename detail::NativeTypes<Tag>::standard_type;

// The native type associated with the given concrete ThriftType.
//
// This is actual used by thrift to represent a value, taking
// into account any IDL annotations that modify the c++ type.
template <typename Tag>
using native_type = typename detail::NativeTypes<Tag>::native_type;

// Infer the Thrift type tag from a standard type.
// If GuessStringTag is true, then std::string will be mapped to binary_t.
// Otherwise the mapping is undefined (i.e. compiler error).
template <typename T, bool GuessStringTag = false>
using infer_tag =
    typename detail::InferTag<folly::remove_cvref_t<T>, GuessStringTag>::type;

} // namespace apache::thrift::type
