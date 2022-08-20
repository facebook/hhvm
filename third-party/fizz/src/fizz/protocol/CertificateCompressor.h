/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>

namespace fizz {
class CertificateCompressor {
 public:
  virtual ~CertificateCompressor() = default;
  /*
   * Return what algorithm this class encodes/decodes.
   */
  virtual CertificateCompressionAlgorithm getAlgorithm() const = 0;

  /*
   * Compress a given certificate message. Throws an exception if the
   * compression fails.
   */
  virtual CompressedCertificate compress(const CertificateMsg&) = 0;
};

class CertificateDecompressor {
 public:
  virtual ~CertificateDecompressor() = default;
  /*
   * Return what algorithm this class encodes/decodes.
   */
  virtual CertificateCompressionAlgorithm getAlgorithm() const = 0;

  /*
   * Decompress a given compressed certificate message. Throws an exception
   * if decompression fails or if it fails parse the CertificateMessage.
   */
  virtual CertificateMsg decompress(const CompressedCertificate&) = 0;
};
} // namespace fizz
