/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/compression/ZstdCertificateCompressor.h>
#include <zstd.h>

using namespace folly;

namespace fizz {

ZstdCertificateCompressor::ZstdCertificateCompressor(int compressLevel)
    : level_(compressLevel) {}

CertificateCompressionAlgorithm ZstdCertificateCompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::zstd;
}

Status ZstdCertificateCompressor::compress(
    CompressedCertificate& ret,
    Error& err,
    const CertificateMsg& cert) {
  Buf encoded;
  FIZZ_RETURN_ON_ERROR(encode(encoded, err, cert));
  auto certRange = encoded->coalesce();
  auto compressedCert = IOBuf::create(ZSTD_compressBound(certRange.size()));

  auto status = ZSTD_compress(
      compressedCert->writableData(),
      compressedCert->tailroom(),
      certRange.data(),
      certRange.size(),
      level_);

  if (ZSTD_isError(status)) {
    std::string errorMsg("Failed to compress cert with zstd: ");
    errorMsg += ZSTD_getErrorName(status);
    return err.error(std::move(errorMsg));
  }

  // with successful compression, status holds compressed size
  compressedCert->append(status);

  ret.uncompressed_length = certRange.size();
  ret.algorithm = getAlgorithm();
  ret.compressed_certificate_message = std::move(compressedCert);
  return Status::Success;
}

} // namespace fizz
