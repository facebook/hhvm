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
class ZstdCertificateCompressor : public CertificateCompressor {
 public:
  explicit ZstdCertificateCompressor(int compressLevel);
  ~ZstdCertificateCompressor() override = default;

  CertificateCompressionAlgorithm getAlgorithm() const override;

  CompressedCertificate compress(const CertificateMsg&) override;

 private:
  const int level_;
};
} // namespace fizz
