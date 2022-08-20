/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <proxygen/lib/http/codec/TransportDirection.h>
#include <proxygen/lib/http/codec/compress/CompressionInfo.h>
#include <proxygen/lib/http/codec/compress/HPACKDecoder.h>
#include <proxygen/lib/http/codec/compress/HPACKEncoder.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>
#include <proxygen/lib/http/codec/compress/HeaderIndexingStrategy.h>
#include <string>
#include <vector>

namespace folly { namespace io {
class Cursor;
}} // namespace folly::io

namespace proxygen {

class HPACKHeader;
class HTTPMessage;

namespace compress {
uint32_t prepareHeaders(const std::vector<Header>& headers,
                        std::vector<HPACKHeader>& prepared);
}

/*
 * Current version of the wire protocol. When we're making changes to the wire
 * protocol we need to change this version and the NPN string so that old
 * clients will not be able to negotiate it anymore.
 */

class HPACKCodec : public HeaderCodec {
 public:
  explicit HPACKCodec(TransportDirection direction);
  ~HPACKCodec() override {
  }

  std::unique_ptr<folly::IOBuf> encode(
      std::vector<compress::Header>& headers) noexcept;

  void encode(std::vector<compress::Header>& headers,
              folly::IOBufQueue& writeBuf) noexcept;

  void encodeHTTP(
      const HTTPMessage& msg,
      folly::IOBufQueue& writeBuf,
      bool includeDate,
      const folly::Optional<HTTPHeaders>& extraHeaders = folly::none) noexcept;

  void decodeStreaming(folly::io::Cursor& cursor,
                       uint32_t length,
                       HPACK::StreamingCallback* streamingCb) noexcept;

  void setEncoderHeaderTableSize(uint32_t size) {
    encoder_.setHeaderTableSize(size);
  }

  void setDecoderHeaderTableMaxSize(uint32_t size) {
    decoder_.setHeaderTableMaxSize(size);
  }

  void describe(std::ostream& os) const;

  void setMaxUncompressed(uint64_t maxUncompressed) override {
    HeaderCodec::setMaxUncompressed(maxUncompressed);
    decoder_.setMaxUncompressed(maxUncompressed);
  }

  CompressionInfo getCompressionInfo() const {
    return CompressionInfo(encoder_.getTableSize(),
                           encoder_.getBytesStored(),
                           encoder_.getHeadersStored(),
                           encoder_.getInsertCount(),
                           0,
                           0,
                           encoder_.getStaticRefs(),
                           decoder_.getTableSize(),
                           decoder_.getBytesStored(),
                           decoder_.getHeadersStored(),
                           decoder_.getInsertCount(),
                           0,
                           0,
                           decoder_.getStaticRefs());
  }

  void setHeaderIndexingStrategy(const HeaderIndexingStrategy* indexingStrat) {
    encoder_.setHeaderIndexingStrategy(indexingStrat);
  }
  const HeaderIndexingStrategy* getHeaderIndexingStrategy() const {
    return encoder_.getHeaderIndexingStrategy();
  }

 protected:
  HPACKEncoder encoder_;
  HPACKDecoder decoder_;

 private:
  void recordCompressedSize(const folly::IOBuf* buf);
  void recordCompressedSize(size_t size);

  std::vector<HPACKHeader> decodedHeaders_;
};

std::ostream& operator<<(std::ostream& os, const HPACKCodec& codec);
} // namespace proxygen
