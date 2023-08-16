/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/compression/CertificateCompressor.h>

namespace fizz {
class BrotliCertificateCompressor : public CertificateCompressor {
 public:
  BrotliCertificateCompressor();
  explicit BrotliCertificateCompressor(int compressLevel, int windowSize);
  ~BrotliCertificateCompressor() override = default;

  CertificateCompressionAlgorithm getAlgorithm() const override;

  CompressedCertificate compress(const CertificateMsg&) override;

  static constexpr int kDefaultCompressionLevel = 5;
  static constexpr int kDefaultWindowSize = 22;

 private:
  const int level_;
  const int windowSize_;
};
} // namespace fizz
