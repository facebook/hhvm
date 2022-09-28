/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_TEST_MOCKTASYNCSSLSOCKET_H_
#define THRIFT_TEST_MOCKTASYNCSSLSOCKET_H_ 1

#include <folly/portability/GMock.h>

#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>

namespace apache {
namespace thrift {
namespace test {

class MockTAsyncSSLSocket : public apache::thrift::async::TAsyncSSLSocket {
 public:
  using UniquePtr = std::unique_ptr<MockTAsyncSSLSocket, Destructor>;

  MockTAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext> ctx, folly::EventBase* base)
      : TAsyncSSLSocket(ctx, base) {}

  static MockTAsyncSSLSocket::UniquePtr newSocket(
      const std::shared_ptr<folly::SSLContext> ctx, folly::EventBase* base) {
    return MockTAsyncSSLSocket::UniquePtr(new MockTAsyncSSLSocket(ctx, base));
  }

  MOCK_METHOD(
      void,
      connect,
      (AsyncSocket::ConnectCallback*,
       const folly::SocketAddress&,
       int,
       const folly::SocketOptionMap&,
       const folly::SocketAddress&,
       const std::string&),
      (noexcept, override));

  MOCK_METHOD(
      void,
      connect,
      (AsyncSocket::ConnectCallback*,
       const folly::SocketAddress&,
       std::chrono::milliseconds,
       std::chrono::milliseconds,
       const folly::SocketOptionMap&,
       const folly::SocketAddress&,
       const std::string&),
      (noexcept, override));

  MOCK_METHOD(
      void, getLocalAddress, (folly::SocketAddress*), (const, override));
  MOCK_METHOD(void, getPeerAddress, (folly::SocketAddress*), (const, override));
  MOCK_METHOD(void, closeNow, (), (override));
  MOCK_METHOD(bool, good, (), (const, override));
  MOCK_METHOD(bool, readable, (), (const, override));
  MOCK_METHOD(bool, hangup, (), (const, override));
  MOCK_METHOD(
      void,
      getSelectedNextProtocol,
      (const unsigned char**, unsigned*),
      (const, override));
  MOCK_METHOD(
      bool,
      getSelectedNextProtocolNoThrow,
      (const unsigned char**, unsigned*),
      (const, override));
};
} // namespace test
} // namespace thrift
} // namespace apache

#endif
