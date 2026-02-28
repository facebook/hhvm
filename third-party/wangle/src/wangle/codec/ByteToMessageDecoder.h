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
 * A Handler which decodes bytes in a stream-like fashion from
 * IOBufQueue to a  Message type.
 *
 * Frame detection
 *
 * Generally frame detection should be handled earlier in the pipeline
 * by adding a DelimiterBasedFrameDecoder, FixedLengthFrameDecoder,
 * LengthFieldBasedFrameDecoder, LineBasedFrameDecoder.
 *
 * If a custom frame decoder is required, then one needs to be careful
 * when implementing one with {@link ByteToMessageDecoder}. Ensure
 * there are enough bytes in the buffer for a complete frame by
 * checking {@link ByteBuf#readableBytes()}. If there are not enough
 * bytes for a complete frame, return without modify the reader index
 * to allow more bytes to arrive.
 *
 * To check for complete frames without modify the reader index, use
 * IOBufQueue.front(), without split() or pop_front().
 */
template <typename M>
class ByteToMessageDecoder : public InboundHandler<folly::IOBufQueue&, M> {
 public:
  using Context = typename InboundHandler<folly::IOBufQueue&, M>::Context;

  /**
   * Decode bytes from buf into result.
   *
   * @return bool - Return true if decoding is successful, false if buf
   *                has insufficient bytes.
   */
  virtual bool
  decode(Context* ctx, folly::IOBufQueue& buf, M& result, size_t&) = 0;

  void transportActive(Context* ctx) override {
    transportActive_ = true;
    ctx->fireTransportActive();
  }

  void transportInactive(Context* ctx) override {
    transportActive_ = false;
    ctx->fireTransportInactive();
  }

  void read(Context* ctx, folly::IOBufQueue& q) override {
    bool success = true;
    while (success && transportActive_) {
      M result;
      size_t needed = 0;
      success = decode(ctx, q, result, needed);
      if (success) {
        ctx->fireRead(std::move(result));
      }
    }
  }

 private:
  bool transportActive_ = true;
};

using ByteToByteDecoder = ByteToMessageDecoder<std::unique_ptr<folly::IOBuf>>;

} // namespace wangle
