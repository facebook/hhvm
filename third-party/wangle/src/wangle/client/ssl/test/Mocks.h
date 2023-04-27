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

#include <folly/portability/GMock.h>
#include <wangle/client/ssl/SSLSessionCallbacks.h>

namespace wangle {

class MockSSLSessionCallbacks : public SSLSessionCallbacks {
 public:
  MOCK_METHOD2(setSSLSessionInternal, void(const std::string&, SSL_SESSION*));

  MOCK_CONST_METHOD1(getSSLSessionInternal, SSL_SESSION*(const std::string&));

  MOCK_METHOD1(removeSSLSessionInternal, bool(const std::string&));

  folly::ssl::SSLSessionUniquePtr getSSLSession(
      const std::string& host) const noexcept override {
    return folly::ssl::SSLSessionUniquePtr(getSSLSessionInternal(host));
  }

  void setSSLSession(
      const std::string& host,
      folly::ssl::SSLSessionUniquePtr session) noexcept override {
    setSSLSessionInternal(host, session.release());
  }

  bool removeSSLSession(const std::string& identity) noexcept override {
    return removeSSLSessionInternal(identity);
  }
};

} // namespace wangle
