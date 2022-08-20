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

#include <memory>

#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <wangle/channel/Handler.h>

namespace wangle {
namespace test {

class FrameTester
    : public wangle::InboundHandler<std::unique_ptr<folly::IOBuf>> {
 public:
  explicit FrameTester(
      folly::Function<void(std::unique_ptr<folly::IOBuf>)> test)
      : test_(std::move(test)) {}

  void read(Context*, std::unique_ptr<folly::IOBuf> buf) override {
    test_(std::move(buf));
  }

  void readException(Context*, folly::exception_wrapper) override {
    test_(nullptr);
  }

 private:
  folly::Function<void(std::unique_ptr<folly::IOBuf>)> test_;
};

class BytesReflector : public wangle::BytesToBytesHandler {
 public:
  folly::Future<folly::Unit> write(
      Context* ctx,
      std::unique_ptr<folly::IOBuf> buf) override {
    folly::IOBufQueue q_(folly::IOBufQueue::cacheChainLength());
    q_.append(std::move(buf));
    ctx->fireRead(q_);

    return folly::makeFuture();
  }
};
} // namespace test
} // namespace wangle
