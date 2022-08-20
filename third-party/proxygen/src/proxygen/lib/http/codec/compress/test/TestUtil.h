/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <memory>
#include <proxygen/lib/http/codec/compress/HPACKDecoder.h>
#include <proxygen/lib/http/codec/compress/HPACKEncoder.h>
#include <proxygen/lib/http/codec/compress/QPACKDecoder.h>
#include <proxygen/lib/http/codec/compress/QPACKEncoder.h>
#include <string>

namespace proxygen { namespace hpack {

void dumpToFile(const std::string& filename, const folly::IOBuf* buf);

std::unique_ptr<folly::IOBuf> encodeDecode(std::vector<HPACKHeader>& headers,
                                           HPACKEncoder& encoder,
                                           HPACKDecoder& decoder);

void encodeDecode(std::vector<HPACKHeader>& headers,
                  QPACKEncoder& encoder,
                  QPACKDecoder& decoder);

std::unique_ptr<HPACKDecoder::headers_t> decode(HPACKDecoder& decoder,
                                                const folly::IOBuf* buffer);

std::vector<compress::Header> headersFromArray(
    std::vector<std::vector<std::string>>& a);

std::vector<compress::Header> basicHeaders();

class TestHeaderCodecStats : public HeaderCodec::Stats {

 public:
  explicit TestHeaderCodecStats(HeaderCodec::Type type) : type_(type) {
  }

  void recordEncode(HeaderCodec::Type type, HTTPHeaderSize& size) override {
    EXPECT_EQ(type, type_);
    encodes++;
    encodedBytesCompr += size.compressed;
    encodedBytesComprBlock += size.compressedBlock;
    encodedBytesUncompr += size.uncompressed;
  }

  void recordDecode(HeaderCodec::Type type, HTTPHeaderSize& size) override {
    EXPECT_EQ(type, type_);
    decodes++;
    decodedBytesCompr += size.compressed;
    decodedBytesUncompr += size.uncompressed;
  }

  void recordDecodeError(HeaderCodec::Type type) override {
    EXPECT_EQ(type, type_);
    errors++;
  }

  void recordDecodeTooLarge(HeaderCodec::Type type) override {
    EXPECT_EQ(type, type_);
    tooLarge++;
  }

  void reset() {
    encodes = 0;
    decodes = 0;
    encodedBytesCompr = 0;
    encodedBytesComprBlock = 0;
    encodedBytesUncompr = 0;
    decodedBytesCompr = 0;
    decodedBytesUncompr = 0;
    errors = 0;
    tooLarge = 0;
  }

  HeaderCodec::Type type_;
  uint32_t encodes{0};
  uint32_t encodedBytesCompr{0};
  uint32_t encodedBytesComprBlock{0};
  uint32_t encodedBytesUncompr{0};
  uint32_t decodes{0};
  uint32_t decodedBytesCompr{0};
  uint32_t decodedBytesUncompr{0};
  uint32_t errors{0};
  uint32_t tooLarge{0};
};

}} // namespace proxygen::hpack
