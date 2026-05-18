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
  MOCK_METHOD(CompressedCertificate, _compress, (const CertificateMsg&));
  Status compress(
      CompressedCertificate& ret,
      Error& err,
      const CertificateMsg& cert) override {
    FIZZ_THROW_TO_ERROR(ret, _compress(cert));
  }
  void setDefaults() {
    ON_CALL(*this, getAlgorithm()).WillByDefault(InvokeWithoutArgs([]() {
      return CertificateCompressionAlgorithm::zlib;
    }));
  }
};

class MockCertificateDecompressor : public CertificateDecompressor {
 public:
  MOCK_METHOD(CertificateCompressionAlgorithm, getAlgorithm, (), (const));
  MOCK_METHOD(CertificateMsg, _decompress, (const CompressedCertificate&));
  Status decompress(
      CertificateMsg& ret,
      Error& err,
      const CompressedCertificate& cc) override {
    FIZZ_THROW_TO_ERROR(ret, _decompress(cc));
  }
  void setDefaults() {
    ON_CALL(*this, getAlgorithm()).WillByDefault(InvokeWithoutArgs([]() {
      return CertificateCompressionAlgorithm::zlib;
    }));
  }
};
} // namespace test
} // namespace fizz
