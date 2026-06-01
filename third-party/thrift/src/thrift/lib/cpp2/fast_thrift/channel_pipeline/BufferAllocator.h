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

#include <cstddef>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * Abstraction to create and copy BytesPtrs.
 */
template <typename B>
concept BufferAllocator = requires(B b, size_t size, const void* data) {
  { b.allocate(size) } -> std::same_as<BytesPtr>;
  { b.copyBuffer(data, size) } -> std::same_as<BytesPtr>;
};

struct SimpleBufferAllocator {
  BytesPtr allocate(size_t size) { return folly::IOBuf::create(size); }

  BytesPtr copyBuffer(const void* data, size_t size) {
    return folly::IOBuf::copyBuffer(data, size);
  }
};

} // namespace apache::thrift::fast_thrift::channel_pipeline
