/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/compression/ZstdCertificateDecompressor.h>
#include <zstd.h>

using namespace folly;

namespace fizz {

CertificateCompressionAlgorithm ZstdCertificateDecompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::zstd;
}

CertificateMsg ZstdCertificateDecompressor::decompress(
    const CompressedCertificate& cc) {
  if (cc.algorithm != getAlgorithm()) {
    throw std::runtime_error(
        "Compressed certificate uses non-zstd algorithm: " +
        toString(cc.algorithm));
  }

  if (cc.uncompressed_length > kMaxHandshakeSize) {
    throw std::runtime_error(
        "Compressed certificate exceeds maximum certificate message size");
  }

  auto rawCertMessage = IOBuf::create(cc.uncompressed_length);
  auto compRange = cc.compressed_certificate_message->coalesce();
  auto status = ZSTD_decompress(
      rawCertMessage->writableData(),
      rawCertMessage->tailroom(),
      compRange.data(),
      compRange.size());

  if (ZSTD_isError(status)) {
    std::string errorMsg("Failed to decompress cert with zstd: ");
    errorMsg += ZSTD_getErrorName(status);
    throw std::runtime_error(std::move(errorMsg));
  }

  if (status != cc.uncompressed_length) {
    throw std::runtime_error("Uncompressed length incorrect");
  }

  if (status == 0) {
    throw std::runtime_error("Compressed certificate is zero-length");
  }

  // with successful decompression, status holds uncompressed size
  rawCertMessage->append(status);
  return decode<CertificateMsg>(std::move(rawCertMessage));
}

} // namespace fizz
