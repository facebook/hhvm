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

/**
 * End-to-end test for FastThriftServerConfig::enableMetadataService.
 *
 * Boots a real FastThriftServer with the metadata service enabled and a real
 * RocketClientChannel ThriftMetadataService client, then proves that:
 *   - the user handler still serves user methods,
 *   - getThriftServiceMetadata() returns the schema for the user's service,
 *   - the response matches what the underlying
 *     detail::md::ServiceMetadata<ServiceHandler<S>>::gen produces (i.e. the
 *     same source of truth legacy ThriftServer uses), so introspection tools
 *     like Thrift Fiddle see what they would for a legacy server.
 *
 * The companion test verifies that with the flag OFF the RPC is rejected as
 * UNKNOWN_METHOD — i.e. the server is opt-in and doesn't accidentally expose
 * the metadata surface.
 */

#include <gtest/gtest.h>

#include <memory>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServerAsyncClient.h>
#include <thrift/lib/cpp2/gen/module_metadata_h.h>
#include <thrift/lib/thrift/gen-cpp2/ThriftMetadataServiceAsyncClient.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::test::integration::e2e {

namespace ftt = ::apache::thrift::fast_thrift::thrift;
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;
using ::apache::thrift::FastServiceHandler;
using ::apache::thrift::fast_thrift::thrift::test::integration::EchoResponse;

namespace {

class UserHandler : public FastServiceHandler<integration::FastThriftServer> {
 public:
  void async_eb_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    cb->done();
  }

  void async_eb_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb, int64_t a, int64_t b) override {
    cb->result(a + b);
  }

  void async_eb_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      std::unique_ptr<std::string> message) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = std::move(*message);
    cb->result(std::move(resp));
  }

  void async_eb_lookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/) override {
    cb->result(std::make_unique<EchoResponse>());
  }

  void async_eb_secureLookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/,
      std::unique_ptr<std::string> /*user*/) override {
    cb->result(std::make_unique<EchoResponse>());
  }
};

} // namespace
} // namespace apache::thrift::fast_thrift::thrift::test::integration::e2e

using namespace apache::thrift::fast_thrift::thrift::test::integration::
    e2e; // NOLINT
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;
namespace ftt = ::apache::thrift::fast_thrift::thrift;

class FastThriftServerMetadataE2ETest : public ::testing::Test {
 protected:
  void StartServer(bool enableMetadataService) {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);
    handler_ = std::make_shared<UserHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;
    config.enableMetadataService = enableMetadataService;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    if (server_) {
      server_->stop();
      server_.reset();
    }
  }

  template <typename Service>
  std::unique_ptr<apache::thrift::Client<Service>> createClient() {
    auto* evb = clientThread_->getEventBase();
    std::unique_ptr<apache::thrift::Client<Service>> client;
    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client =
          std::make_unique<apache::thrift::Client<Service>>(std::move(channel));
    });
    return client;
  }

  template <typename Client>
  void destroyClientOnEvb(std::unique_ptr<Client>& client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  std::shared_ptr<UserHandler> handler_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

// With enableMetadataService=true the metadata RPC must succeed AND return
// the same payload that detail::md::ServiceMetadata<ServiceHandler<S>>::gen
// produces — proving the cached response was built from the user handler's
// real schema, not stubbed.
TEST_F(FastThriftServerMetadataE2ETest, ServesGetThriftServiceMetadata) {
  StartServer(/*enableMetadataService=*/true);

  auto metadataClient =
      createClient<apache::thrift::metadata::ThriftMetadataService>();
  auto got = metadataClient->semifuture_getThriftServiceMetadata().get();

  apache::thrift::metadata::ThriftServiceMetadataResponse expected;
  apache::thrift::detail::md::ServiceMetadata<apache::thrift::ServiceHandler<
      integration::FastThriftServer>>::gen(expected);

  EXPECT_EQ(got, expected);
  // Sanity that user RPCs still flow through the composite alongside the
  // metadata RPC — earlier-child wins routing must keep user methods working.
  auto userClient = createClient<integration::FastThriftServer>();
  EXPECT_EQ(userClient->semifuture_add(2, 3).get(), 5);
  destroyClientOnEvb(userClient);
  destroyClientOnEvb(metadataClient);
}

// Default-off: with the flag unset, the metadata method must NOT be exposed.
// Composite is never mounted, so the request lands at the user adapter and
// fails as UNKNOWN_METHOD — proves the surface is genuinely opt-in.
TEST_F(FastThriftServerMetadataE2ETest, OptOutByDefault) {
  StartServer(/*enableMetadataService=*/false);

  auto metadataClient =
      createClient<apache::thrift::metadata::ThriftMetadataService>();
  EXPECT_THROW(
      metadataClient->semifuture_getThriftServiceMetadata().get(),
      apache::thrift::TApplicationException);

  destroyClientOnEvb(metadataClient);
}
