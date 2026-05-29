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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * HandlerTag is a compile-time wrapper for handler IDs.
 *
 * It provides type safety and enables implicit conversion to HandlerId
 * for use in pipeline operations.
 *
 * Usage:
 *   HANDLER_TAG(decoder);
 *   HANDLER_TAG(encoder);
 *
 *   auto* ctx = pipeline->context(decoder_tag);
 */
template <HandlerId Id>
struct HandlerTag {
  static constexpr HandlerId id = Id;
  constexpr operator HandlerId() const noexcept { return id; }
};

/**
 * Macro for defining handler tags with compile-time IDs.
 *
 * The tag name will be the macro argument with "_tag" suffix.
 * The ID is computed at compile time using FNV-1a hash of the name.
 *
 * Example:
 *   HANDLER_TAG(codec);           // Creates codec_tag
 *   HANDLER_TAG(thrift_processor); // Creates thrift_processor_tag
 */
// clang-format off
#define HANDLER_TAG(name) \
    inline constexpr auto name##_tag = \
        ::apache::thrift::fast_thrift::channel_pipeline::HandlerTag<::apache::thrift::fast_thrift::channel_pipeline::fnv1a_hash(#name)>{}
// clang-format on

} // namespace apache::thrift::fast_thrift::channel_pipeline
