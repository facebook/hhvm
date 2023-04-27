/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertificateCompressor.h>
#include <zlib.h>

namespace fizz {
class ZlibCertificateDecompressor : public CertificateDecompressor {
 public:
  ~ZlibCertificateDecompressor() override = default;

  CertificateCompressionAlgorithm getAlgorithm() const override;

  CertificateMsg decompress(const CompressedCertificate&) override;
};
} // namespace fizz
