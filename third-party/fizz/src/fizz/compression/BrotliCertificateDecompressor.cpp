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
Status brotliDecompressImpl(
    size_t& ret,
    Error& err,
    folly::ByteRange input,
    uint8_t* output,
    size_t outputSize) {
  auto status =
      BrotliDecoderDecompress(input.size(), input.data(), &outputSize, output);
  if (status != BrotliDecoderResult::BROTLI_DECODER_RESULT_SUCCESS) {
    return err.error("Decompressing certificate failed");
  }
  ret = outputSize;
  return Status::Success;
}
} // namespace

CertificateCompressionAlgorithm BrotliCertificateDecompressor::getAlgorithm()
    const {
  return CertificateCompressionAlgorithm::brotli;
}

Status BrotliCertificateDecompressor::decompress(
    CertificateMsg& ret,
    Error& err,
    const CompressedCertificate& cc) {
  if (cc.algorithm != getAlgorithm()) {
    return err.error(
        "Compressed certificate uses non-brotli algorithm: " +
        toString(cc.algorithm));
  }

  if (cc.uncompressed_length > kMaxHandshakeSize) {
    return err.error(
        "Compressed certificate exceeds maximum certificate message size");
  }

  auto rawCertMessage = IOBuf::create(cc.uncompressed_length);
  auto compRange = cc.compressed_certificate_message->coalesce();
  size_t decompressedSize = 0;
  FIZZ_RETURN_ON_ERROR(brotliDecompressImpl(
      decompressedSize,
      err,
      compRange,
      rawCertMessage->writableData(),
      cc.uncompressed_length));
  if (decompressedSize != cc.uncompressed_length) {
    return err.error("Uncompressed length incorrect");
  }

  rawCertMessage->append(decompressedSize);
  FIZZ_RETURN_ON_ERROR(
      decode<CertificateMsg>(ret, err, std::move(rawCertMessage)));
  return Status::Success;
}

} // namespace fizz
