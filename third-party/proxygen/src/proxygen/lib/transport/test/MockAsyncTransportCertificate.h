/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/AsyncTransportCertificate.h>
#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>
#include <folly/portability/GMock.h>

namespace proxygen {

class MockAsyncTransportCertificate : public folly::AsyncTransportCertificate {
 public:
  MOCK_METHOD(std::string, getIdentity, (), (const, override));
};

class MockOpenSSLTransportCertificate
    : public folly::OpenSSLTransportCertificate {
 public:
  MOCK_METHOD(folly::ssl::X509UniquePtr, getX509, (), (const, override));
  MOCK_METHOD(std::string, getIdentity, (), (const, override));
};

} // namespace proxygen
