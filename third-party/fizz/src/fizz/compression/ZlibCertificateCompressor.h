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
  static Status create(
      std::unique_ptr<ZlibCertificateCompressor>& ret,
      Error& err,
      int compressLevel);

  ~ZlibCertificateCompressor() override = default;

  CertificateCompressionAlgorithm getAlgorithm() const override;

  Status compress(CompressedCertificate& ret, Error& err, const CertificateMsg&)
      override;

 private:
  explicit ZlibCertificateCompressor(int compressLevel);
  const int level_;
};
} // namespace fizz
