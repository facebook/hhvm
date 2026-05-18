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
    : level_(compressLevel) {}

Status ZlibCertificateCompressor::create(
    std::unique_ptr<ZlibCertificateCompressor>& ret,
    Error& err,
    int compressLevel) {
  if (compressLevel != Z_DEFAULT_COMPRESSION &&
      !(compressLevel >= Z_NO_COMPRESSION &&
        compressLevel <= Z_BEST_COMPRESSION)) {
    return err.error(
        "Invalid compression level requested:" +
        to<std::string>(compressLevel));
  }
  ret = std::unique_ptr<ZlibCertificateCompressor>(
      new ZlibCertificateCompressor(compressLevel));
  return Status::Success;
}

CertificateCompressionAlgorithm ZlibCertificateCompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::zlib;
}

Status ZlibCertificateCompressor::compress(
    CompressedCertificate& ret,
    Error& err,
    const CertificateMsg& cert) {
  Buf encoded;
  FIZZ_RETURN_ON_ERROR(encode(encoded, err, cert));
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
      return err.error("Insufficient memory to compress cert");
    case Z_BUF_ERROR:
      return err.error("Buffer too small for compressed cert");
    case Z_STREAM_ERROR:
      return err.error("Compression level invalid: " + to<std::string>(level_));
    default:
      return err.error("Failed to compress: " + to<std::string>(status));
  }

  ret.uncompressed_length = certRange.size();
  ret.algorithm = getAlgorithm();
  ret.compressed_certificate_message = std::move(compressedCert);
  return Status::Success;
}

} // namespace fizz
