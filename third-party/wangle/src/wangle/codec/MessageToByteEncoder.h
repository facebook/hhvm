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

#include <wangle/channel/Handler.h>

namespace wangle {

/**
 * An OutboundHandler which encodes message in a stream-like fashion from one
 * message to IOBuf. Inverse of ByteToMessageDecoder.
 */
template <typename M>
class MessageToByteEncoder
    : public OutboundHandler<M, std::unique_ptr<folly::IOBuf>> {
 public:
  using Context =
      typename OutboundHandler<M, std::unique_ptr<folly::IOBuf>>::Context;

  virtual std::unique_ptr<folly::IOBuf> encode(M& msg) = 0;

  folly::Future<folly::Unit> write(Context* ctx, M msg) override {
    auto buf = encode(msg);
    return buf ? ctx->fireWrite(std::move(buf)) : folly::makeFuture();
  }
};

} // namespace wangle
