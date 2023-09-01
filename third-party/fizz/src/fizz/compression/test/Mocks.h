/*
 *  Copyright (c) 2023-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/compression/CertificateCompressor.h>
#include <folly/portability/GMock.h>

namespace fizz {
namespace test {

using namespace testing;

class MockCertificateCompressor : public CertificateCompressor {
 public:
  MOCK_METHOD(CertificateCompressionAlgorithm, getAlgorithm, (), (const));
  MOCK_METHOD(CompressedCertificate, compress, (const CertificateMsg&));
  void setDefaults() {
    ON_CALL(*this, getAlgorithm()).WillByDefault(InvokeWithoutArgs([]() {
      return CertificateCompressionAlgorithm::zlib;
    }));
  }
};

class MockCertificateDecompressor : public CertificateDecompressor {
 public:
  MOCK_METHOD(CertificateCompressionAlgorithm, getAlgorithm, (), (const));
  MOCK_METHOD(CertificateMsg, decompress, (const CompressedCertificate&));
  void setDefaults() {
    ON_CALL(*this, getAlgorithm()).WillByDefault(InvokeWithoutArgs([]() {
      return CertificateCompressionAlgorithm::zlib;
    }));
  }
};
} // namespace test
} // namespace fizz
