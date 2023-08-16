// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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
