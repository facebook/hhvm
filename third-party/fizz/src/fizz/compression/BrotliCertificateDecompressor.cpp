/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/compression/BrotliCertificateDecompressor.h>

#include <brotli/decode.h>

using namespace folly;

namespace fizz {

namespace {
size_t brotliDecompressImpl(
    folly::ByteRange input,
    uint8_t* output,
    size_t outputSize) {
  auto status =
      BrotliDecoderDecompress(input.size(), input.data(), &outputSize, output);
  if (status != BrotliDecoderResult::BROTLI_DECODER_RESULT_SUCCESS) {
    throw std::runtime_error("Decompressing certificate failed");
  }
  return outputSize;
}
} // namespace

CertificateCompressionAlgorithm BrotliCertificateDecompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::brotli;
}

CertificateMsg BrotliCertificateDecompressor::decompress(
    const CompressedCertificate& cc) {
  if (cc.algorithm != getAlgorithm()) {
    throw std::runtime_error(
        "Compressed certificate uses non-brotli algorithm: " +
        toString(cc.algorithm));
  }

  if (cc.uncompressed_length > kMaxHandshakeSize) {
    throw std::runtime_error(
        "Compressed certificate exceeds maximum certificate message size");
  }

  auto rawCertMessage = IOBuf::create(cc.uncompressed_length);
  auto compRange = cc.compressed_certificate_message->coalesce();
  auto decompressedSize = brotliDecompressImpl(
      compRange, rawCertMessage->writableData(), cc.uncompressed_length);
  if (decompressedSize != cc.uncompressed_length) {
    throw std::runtime_error("Uncompressed length incorrect");
  }

  rawCertMessage->append(decompressedSize);
  return decode<CertificateMsg>(std::move(rawCertMessage));
}

} // namespace fizz
