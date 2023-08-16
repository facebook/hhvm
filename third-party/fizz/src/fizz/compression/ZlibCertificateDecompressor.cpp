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

CertificateMsg ZlibCertificateDecompressor::decompress(
    const CompressedCertificate& cc) {
  if (cc.algorithm != getAlgorithm()) {
    throw std::runtime_error(
        "Compressed certificate uses non-zlib algorithm: " +
        toString(cc.algorithm));
  }

  if (cc.uncompressed_length > kMaxHandshakeSize) {
    throw std::runtime_error(
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
        throw std::runtime_error("Uncompressed length incorrect");
      }
      break;
    case Z_MEM_ERROR:
      throw std::runtime_error("Insufficient memory to decompress cert");
    case Z_BUF_ERROR:
      throw std::runtime_error(
          "The uncompressed length given is too small to hold uncompressed data");
    case Z_DATA_ERROR:
      throw std::runtime_error(
          "The compressed certificate data was incomplete or invalid");
    default:
      throw std::runtime_error(
          "Failed to decompress: " + to<std::string>(status));
  }
  rawCertMessage->append(size);
  return decode<CertificateMsg>(std::move(rawCertMessage));
}

} // namespace fizz
