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

#include <thrift/lib/cpp2/async/FramingHandler.h>

namespace apache {
namespace thrift {

void FramingHandler::read(Context* ctx, folly::IOBufQueue& q) {
  // Remaining for this packet.  Will update the class member
  // variable below for the next call to getReadBuffer
  size_t remaining = 0;

  // Loop as long as there are deframed messages to read.
  // Partial frames are stored inside the handlers between calls

  // On the last iteration, remaining_ is updated to the anticipated remaining
  // frame length (if we're in the middle of a frame) or to readBufferSize_
  // (if we are exactly between frames)
  while (!closing_) {
    std::unique_ptr<folly::IOBuf> unframed;
    std::unique_ptr<apache::thrift::transport::THeader> header;
    auto ex = folly::try_and_catch([&]() {
      // got a decrypted message
      std::tie(unframed, remaining, header) = removeFrame(&q);
    });

    if (ex) {
      VLOG(5) << "Failed to read a message header";
      ctx->fireReadException(std::move(ex));
      ctx->fireClose();
      return;
    }

    if (!unframed) {
      ctx->setReadBufferSettings(
          readBufferSize_, remaining ? remaining : readBufferSize_);
      return;
    } else {
      ctx->fireRead(std::make_pair(std::move(unframed), std::move(header)));
    }
  }
}

folly::Future<folly::Unit> FramingHandler::write(
    Context* ctx,
    std::pair<
        std::unique_ptr<folly::IOBuf>,
        apache::thrift::transport::THeader*> bufAndHeader) {
  return ctx->fireWrite(
      addFrame(std::move(bufAndHeader.first), bufAndHeader.second));
}

folly::Future<folly::Unit> FramingHandler::close(Context* ctx) {
  closing_ = true;
  return ctx->fireClose();
}

} // namespace thrift
} // namespace apache
