/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/stats/HeaderCodecStats.h>

using facebook::fb303::AVG;
using facebook::fb303::SUM;
using std::string;

namespace {

std::array<const char*, 3> kCompressionTypes = {"gzip", "hpack", "qpack"};

}

namespace proxygen {

TLHeaderCodecStats::TLHeaderCodecStats(const string& prefix) {
  encodeCompr_.reserve(kCompressionTypes.size());
  encodeUncompr_.reserve(kCompressionTypes.size());
  decodeCompr_.reserve(kCompressionTypes.size());
  decodeUncompr_.reserve(kCompressionTypes.size());
  encodes_.reserve(kCompressionTypes.size());
  decodes_.reserve(kCompressionTypes.size());
  decodeErrors_.reserve(kCompressionTypes.size());
  decodeTooLarge_.reserve(kCompressionTypes.size());
  for (auto comprType : kCompressionTypes) {
    encodeCompr_.emplace_back(std::make_unique<StatsWrapper::TLHistogram>(
        prefix + "_" + comprType + "_encode_compr",
        100,
        0,
        10000,
        SUM,
        AVG,
        10,
        50,
        90));
    encodeUncompr_.emplace_back(std::make_unique<StatsWrapper::TLHistogram>(
        prefix + "_" + comprType + "_encode_uncompr",
        100,
        0,
        10000,
        SUM,
        AVG,
        10,
        50,
        90));
    decodeCompr_.emplace_back(std::make_unique<StatsWrapper::TLHistogram>(
        prefix + "_" + comprType + "_decode_compr",
        100,
        0,
        10000,
        SUM,
        AVG,
        10,
        50,
        90));
    decodeUncompr_.emplace_back(std::make_unique<StatsWrapper::TLHistogram>(
        prefix + "_" + comprType + "_decode_uncompr",
        100,
        0,
        10000,
        SUM,
        AVG,
        10,
        50,
        90));
    encodes_.emplace_back(prefix + "_" + comprType + "_encodes", SUM);
    decodes_.emplace_back(prefix + "_" + comprType + "_decodes", SUM);
    decodeErrors_.emplace_back(prefix + "_" + comprType + "_decode_errors",
                               SUM);
    decodeTooLarge_.emplace_back(prefix + "_" + comprType + "_decode_too_large",
                                 SUM);
  }
}

void TLHeaderCodecStats::recordEncode(HeaderCodec::Type type,
                                      HTTPHeaderSize& size) {
  uint32_t i = (uint32_t)type;
  CHECK(i < encodes_.size());
  encodes_[i].add(1);
  CHECK(i < encodeCompr_.size());
  encodeCompr_[i]->add(size.compressed);
  CHECK(i < encodeUncompr_.size());
  encodeUncompr_[i]->add(size.uncompressed);
}

void TLHeaderCodecStats::recordDecode(HeaderCodec::Type type,
                                      HTTPHeaderSize& size) {
  uint32_t i = (uint32_t)type;
  CHECK(i < decodes_.size());
  decodes_[i].add(1);
  CHECK(i < decodeCompr_.size());
  decodeCompr_[i]->add(size.compressed);
  CHECK(i < decodeUncompr_.size());
  decodeUncompr_[i]->add(size.uncompressed);
}

void TLHeaderCodecStats::recordDecodeError(HeaderCodec::Type type) {
  uint32_t i = (uint32_t)type;
  CHECK(i < decodeErrors_.size());
  decodeErrors_[i].add(1);
}

void TLHeaderCodecStats::recordDecodeTooLarge(HeaderCodec::Type type) {
  uint32_t i = (uint32_t)type;
  CHECK(i < decodeTooLarge_.size());
  decodeTooLarge_[i].add(1);
}

} // namespace proxygen
