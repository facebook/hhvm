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
class BrotliCertificateDecompressor : public CertificateDecompressor {
 public:
  ~BrotliCertificateDecompressor() override = default;

  CertificateCompressionAlgorithm getAlgorithm() const override;

  Status decompress(
      CertificateMsg& ret,
      Error& err,
      const CompressedCertificate&) override;
};
} // namespace fizz
