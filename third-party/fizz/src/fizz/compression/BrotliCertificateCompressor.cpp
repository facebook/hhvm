/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <brotli/encode.h>
#include <fizz/compression/BrotliCertificateCompressor.h>

using namespace folly;

namespace fizz {

BrotliCertificateCompressor::BrotliCertificateCompressor(
    int compressLevel,
    int windowSize)
    : level_(compressLevel), windowSize_(windowSize) {}

BrotliCertificateCompressor::BrotliCertificateCompressor()
    : level_(kDefaultCompressionLevel), windowSize_(kDefaultWindowSize) {}

CertificateCompressionAlgorithm BrotliCertificateCompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::brotli;
}

namespace {
namespace brotli1 {
std::unique_ptr<IOBuf>
brotliCompressImpl(int level, int windowSize, folly::ByteRange input) {
  size_t upperBound = ::BrotliEncoderMaxCompressedSize(input.size());
  if (upperBound == 0) {
    throw std::runtime_error(
        "Failed to compress certificate: could not calculate upper bound");
  }

  size_t size = input.size();
  auto compressed = IOBuf::create(upperBound);
  if (!BrotliEncoderCompress(
          level,
          windowSize,
          BROTLI_MODE_GENERIC,
          input.size(),
          input.data(),
          &size,
          compressed->writableTail())) {
    throw std::runtime_error("Failed to compress certificate");
  }

  // |size|, if the BrotliEncoderCompress call succeeds, is modified to contain
  // the compressed size.
  compressed->append(size);
  return compressed;
}
} // namespace brotli1
using brotli1::brotliCompressImpl;
} // namespace

CompressedCertificate BrotliCertificateCompressor::compress(
    const CertificateMsg& cert) {
  auto encoded = encode(cert);
  auto encodedRange = encoded->coalesce();
  auto compressedCert = brotliCompressImpl(level_, windowSize_, encodedRange);

  CompressedCertificate cc;
  cc.uncompressed_length = encodedRange.size();
  cc.algorithm = getAlgorithm();
  cc.compressed_certificate_message = std::move(compressedCert);
  return cc;
}

} // namespace fizz
