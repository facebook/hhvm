/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/HeaderCodec.h>
#include <proxygen/lib/stats/BaseStats.h>
#include <string>
#include <vector>

namespace proxygen {

class TLHeaderCodecStats : public HeaderCodec::Stats {
 public:
  explicit TLHeaderCodecStats(const std::string& prefix);
  ~TLHeaderCodecStats() override {
  }
  TLHeaderCodecStats(const TLHeaderCodecStats&) = delete;
  TLHeaderCodecStats& operator=(const TLHeaderCodecStats&) = delete;

  void recordEncode(HeaderCodec::Type type, HTTPHeaderSize& size) override;
  void recordDecode(HeaderCodec::Type type, HTTPHeaderSize& size) override;
  void recordDecodeError(HeaderCodec::Type type) override;
  void recordDecodeTooLarge(HeaderCodec::Type type) override;

 private:
  std::vector<std::unique_ptr<BaseStats::TLHistogram>> encodeCompr_;
  std::vector<std::unique_ptr<BaseStats::TLHistogram>> encodeUncompr_;
  std::vector<std::unique_ptr<BaseStats::TLHistogram>> decodeCompr_;
  std::vector<std::unique_ptr<BaseStats::TLHistogram>> decodeUncompr_;
  std::vector<BaseStats::TLTimeseries> encodes_;
  std::vector<BaseStats::TLTimeseries> decodes_;
  std::vector<BaseStats::TLTimeseries> decodeErrors_;
  std::vector<BaseStats::TLTimeseries> decodeTooLarge_;
};

} // namespace proxygen
