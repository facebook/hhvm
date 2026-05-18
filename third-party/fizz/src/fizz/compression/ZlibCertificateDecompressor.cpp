/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/compression/ZlibCertificateDecompressor.h>

using namespace folly;

namespace fizz {

CertificateCompressionAlgorithm ZlibCertificateDecompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::zlib;
}

Status ZlibCertificateDecompressor::decompress(
    CertificateMsg& ret,
    Error& err,
    const CompressedCertificate& cc) {
  if (cc.algorithm != getAlgorithm()) {
    return err.error(
        "Compressed certificate uses non-zlib algorithm: " +
        toString(cc.algorithm));
  }

  if (cc.uncompressed_length > kMaxHandshakeSize) {
    return err.error(
        "Compressed certificate exceeds maximum certificate message size");
  }

  auto rawCertMessage = IOBuf::create(cc.uncompressed_length);
  unsigned long size = cc.uncompressed_length;
  auto compRange = cc.compressed_certificate_message->coalesce();
  auto status = ::uncompress(
      rawCertMessage->writableData(),
      &size,
      compRange.data(),
      compRange.size());
  switch (status) {
    case Z_OK:
      if (size != cc.uncompressed_length) {
        return err.error("Uncompressed length incorrect");
      }
      break;
    case Z_MEM_ERROR:
      return err.error("Insufficient memory to decompress cert");
    case Z_BUF_ERROR:
      return err.error(
          "The uncompressed length given is too small to hold uncompressed data");
    case Z_DATA_ERROR:
      return err.error(
          "The compressed certificate data was incomplete or invalid");
    default:
      return err.error("Failed to decompress: " + to<std::string>(status));
  }
  rawCertMessage->append(size);
  FIZZ_RETURN_ON_ERROR(
      decode<CertificateMsg>(ret, err, std::move(rawCertMessage)));
  return Status::Success;
}

} // namespace fizz
