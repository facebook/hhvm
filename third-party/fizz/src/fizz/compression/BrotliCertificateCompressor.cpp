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
Status brotliCompressImpl(
    std::unique_ptr<IOBuf>& ret,
    Error& err,
    int level,
    int windowSize,
    folly::ByteRange input) {
  size_t upperBound = ::BrotliEncoderMaxCompressedSize(input.size());
  if (upperBound == 0) {
    return err.error(
        "Failed to compress certificate: could not calculate upper bound");
  }

  size_t size = upperBound;
  auto compressed = IOBuf::create(upperBound);
  if (!BrotliEncoderCompress(
          level,
          windowSize,
          BROTLI_MODE_GENERIC,
          input.size(),
          input.data(),
          &size,
          compressed->writableTail())) {
    return err.error("Failed to compress certificate");
  }

  // |size|, if the BrotliEncoderCompress call succeeds, is modified to contain
  // the compressed size.
  compressed->append(size);
  ret = std::move(compressed);
  return Status::Success;
}
} // namespace brotli1
using brotli1::brotliCompressImpl;
} // namespace

Status BrotliCertificateCompressor::compress(
    CompressedCertificate& ret,
    Error& err,
    const CertificateMsg& cert) {
  Buf encoded;
  FIZZ_RETURN_ON_ERROR(encode(encoded, err, cert));
  auto encodedRange = encoded->coalesce();
  std::unique_ptr<IOBuf> compressedCert;
  FIZZ_RETURN_ON_ERROR(brotliCompressImpl(
      compressedCert, err, level_, windowSize_, encodedRange));

  ret.uncompressed_length = encodedRange.size();
  ret.algorithm = getAlgorithm();
  ret.compressed_certificate_message = std::move(compressedCert);
  return Status::Success;
}

} // namespace fizz
