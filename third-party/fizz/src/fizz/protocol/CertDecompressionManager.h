/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/compression/CertificateCompressor.h>

#include <map>
#include <vector>

namespace fizz {

class CertDecompressionManager {
 public:
  /**
   * Can be constructed empty with no arguments or initialized with the
   * decompressors provided.
   */
  CertDecompressionManager();
  explicit CertDecompressionManager(
      std::vector<std::shared_ptr<CertificateDecompressor>> decompressors);

  /**
   * Explicitly sets the decompressors. Clears the internal storage before
   * using the new list.
   */
  void setDecompressors(
      std::vector<std::shared_ptr<CertificateDecompressor>> decompressors);

  /**
   * Return supported decompression algorithms.
   */
  std::vector<CertificateCompressionAlgorithm> getSupportedAlgorithms() const;

  /**
   * Returns the decompressor for a given algorithm. If not found, returns
   * nullptr;
   */
  std::shared_ptr<CertificateDecompressor> getDecompressor(
      CertificateCompressionAlgorithm algo) const;

 private:
  std::map<
      CertificateCompressionAlgorithm,
      std::shared_ptr<CertificateDecompressor>>
      decompressors_;

  std::vector<CertificateCompressionAlgorithm> supportedAlgos_;
};

} // namespace fizz
