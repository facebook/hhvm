/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/compression/CertificateCompressor.h>
#include <zlib.h>

namespace fizz {
class ZlibCertificateCompressor : public CertificateCompressor {
 public:
  explicit ZlibCertificateCompressor(int compressLevel);
  ~ZlibCertificateCompressor() override = default;

  CertificateCompressionAlgorithm getAlgorithm() const override;

  CompressedCertificate compress(const CertificateMsg&) override;

 private:
  const int level_;
};
} // namespace fizz
