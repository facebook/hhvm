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

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace apache::thrift::fast_thrift::rocket::bench {

/**
 * Minimal mock context for rocket handler benchmarks.
 *
 * Provides the full context API surface so any handler bench can use it
 * without defining its own.
 */
class BenchContext {
 public:
  using Result = channel_pipeline::Result;
  using TypeErasedBox = channel_pipeline::TypeErasedBox;
  using BytesPtr = channel_pipeline::BytesPtr;

  Result fireRead(TypeErasedBox&& msg) noexcept {
    lastReadMsg_ = std::move(msg);
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    lastWriteMsg_ = std::move(msg);
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    lastException_ = std::move(e);
  }

  void deactivate() noexcept {}

  void close() noexcept {}

  BytesPtr allocate(size_t size) noexcept {
    return channel_pipeline::SimpleBufferAllocator{}.allocate(size);
  }

  BytesPtr copyBuffer(const void* data, size_t len) noexcept {
    auto buf = allocate(len);
    std::memcpy(buf->writableData(), data, len);
    buf->append(len);
    return buf;
  }

 private:
  TypeErasedBox lastReadMsg_;
  TypeErasedBox lastWriteMsg_;
  folly::exception_wrapper lastException_;
};

} // namespace apache::thrift::fast_thrift::rocket::bench
