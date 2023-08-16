/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/compression/CertDecompressionManager.h>

namespace fizz {

CertDecompressionManager::CertDecompressionManager() {}

CertDecompressionManager::CertDecompressionManager(
    std::vector<std::shared_ptr<CertificateDecompressor>> decompressors) {
  setDecompressors(std::move(decompressors));
}

void CertDecompressionManager::setDecompressors(
    std::vector<std::shared_ptr<CertificateDecompressor>> decompressors) {
  decompressors_.clear();
  for (const auto& decompressor : decompressors) {
    decompressors_[decompressor->getAlgorithm()] = decompressor;
  }
  supportedAlgos_.clear();
  for (const auto& pair : decompressors_) {
    supportedAlgos_.push_back(pair.first);
  }
}

std::vector<CertificateCompressionAlgorithm>
CertDecompressionManager::getSupportedAlgorithms() const {
  return supportedAlgos_;
}

std::shared_ptr<CertificateDecompressor>
CertDecompressionManager::getDecompressor(
    CertificateCompressionAlgorithm algo) const {
  try {
    return decompressors_.at(algo);
  } catch (const std::out_of_range&) {
    VLOG(4) << "Requested unknown algorithm: " << toString(algo);
    return nullptr;
  }
}

} // namespace fizz
