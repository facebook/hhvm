/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/io/async/ssl/BasicTransportCertificate.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift::test {

class MockCpp2ConnContext : public apache::thrift::Cpp2ConnContext {
 public:
  MockCpp2ConnContext(
      const folly::SocketAddress* address = nullptr,
      const std::shared_ptr<X509> peerCert = nullptr,
      apache::thrift::ClientIdentityHook clientIdentityHook = nullptr)
      : apache::thrift::Cpp2ConnContext(
            address, nullptr, nullptr, peerCert, clientIdentityHook) {
    transport_ = std::make_unique<folly::test::MockAsyncTransport>();
    EXPECT_CALL(*this, getTransport())
        .WillRepeatedly(testing::Return(transport_.get()));

    if (peerCert) {
      auto x509raw = peerCert.get();
      X509_up_ref(x509raw);
      transportCert_ = std::make_unique<folly::ssl::BasicTransportCertificate>(
          "cert1", folly::ssl::X509UniquePtr(x509raw));
      EXPECT_CALL(*transport_, getPeerCertificate())
          .WillRepeatedly(testing::Return(transportCert_.get()));
    } else {
      EXPECT_CALL(*transport_, getPeerCertificate())
          .WillRepeatedly(testing::Return(nullptr));
    }
  }

  MOCK_CONST_METHOD0(getTransport, const folly::AsyncTransport*());

 private:
  std::unique_ptr<folly::test::MockAsyncTransport> transport_;
  std::unique_ptr<folly::ssl::BasicTransportCertificate> transportCert_;
};

} // namespace apache::thrift::test
