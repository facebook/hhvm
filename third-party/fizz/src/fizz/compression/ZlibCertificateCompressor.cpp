/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/compression/ZlibCertificateCompressor.h>

using namespace folly;

namespace fizz {

ZlibCertificateCompressor::ZlibCertificateCompressor(int compressLevel)
    : level_(compressLevel) {
  if (compressLevel != Z_DEFAULT_COMPRESSION &&
      !(compressLevel >= Z_NO_COMPRESSION &&
        compressLevel <= Z_BEST_COMPRESSION)) {
    throw std::runtime_error(
        "Invalid compression level requested:" +
        to<std::string>(compressLevel));
  }
}

CertificateCompressionAlgorithm ZlibCertificateCompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::zlib;
}

CompressedCertificate ZlibCertificateCompressor::compress(
    const CertificateMsg& cert) {
  auto encoded = encode(cert);
  auto certRange = encoded->coalesce();
  auto compressedCert = IOBuf::create(compressBound(certRange.size()));
  unsigned long size = compressedCert->capacity();

  int status = ::compress2(
      compressedCert->writableData(),
      &size,
      certRange.data(),
      certRange.size(),
      level_);
  switch (status) {
    case Z_OK:
      compressedCert->append(size);
      break;
    case Z_MEM_ERROR:
      throw std::runtime_error("Insufficient memory to compress cert");
    case Z_BUF_ERROR:
      throw std::runtime_error("Buffer too small for compressed cert");
    case Z_STREAM_ERROR:
      throw std::runtime_error(
          "Compression level invalid: " + to<std::string>(level_));
    default:
      throw std::runtime_error(
          "Failed to compress: " + to<std::string>(status));
  }

  CompressedCertificate cc;
  cc.uncompressed_length = certRange.size();
  cc.algorithm = getAlgorithm();
  cc.compressed_certificate_message = std::move(compressedCert);
  return cc;
}

} // namespace fizz
