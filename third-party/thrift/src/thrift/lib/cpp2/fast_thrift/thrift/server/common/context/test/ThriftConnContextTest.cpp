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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>

#include <gtest/gtest.h>

#include <utility>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>

namespace apache::thrift::fast_thrift::thrift {

TEST(ThriftConnContextTest, DefaultsAreEmpty) {
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
  EXPECT_TRUE(ctx->getPeerAddress().empty());
  EXPECT_TRUE(ctx->getSecurityProtocol().empty());
  EXPECT_EQ(ctx->getPeerCertificate(), nullptr);
  EXPECT_EQ(ctx->getTransport(), nullptr);
  EXPECT_EQ(ctx->getPeerIdentities(), nullptr);
  EXPECT_EQ(ctx->getClientMetadata(), nullptr);
  EXPECT_EQ(ctx->getUserData(), nullptr);
}

TEST(ThriftConnContextTest, SettersAreReflectedInGetters) {
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
  folly::SocketAddress addr("127.0.0.1", 1234);

  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb);
  int peerIdentities = 0;

  ctx->setPeerAddress(addr);
  ctx->setSecurityProtocol("TLS1.3");
  ctx->setTransport(socket.get());
  ctx->setPeerIdentities(&peerIdentities);

  EXPECT_EQ(ctx->getPeerAddress(), addr);
  EXPECT_EQ(ctx->getSecurityProtocol(), "TLS1.3");
  EXPECT_EQ(ctx->getTransport(), socket.get());
  EXPECT_EQ(ctx->getPeerIdentities(), &peerIdentities);
}

// What the client sent about itself is kept whole, for triage to read back.
TEST(ThriftConnContextTest, ClientMetadataIsLatchedWhole) {
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};

  apache::thrift::ClientMetadata metadata;
  metadata.hostname() = "client.host";
  metadata.agent() = "test-agent";
  ctx->setClientMetadata(std::move(metadata));

  const auto* latched = ctx->getClientMetadata();
  ASSERT_NE(latched, nullptr);
  EXPECT_EQ(latched->hostname().value_or(""), "client.host");
  EXPECT_EQ(latched->agent().value_or(""), "test-agent");
}

TEST(ThriftConnContextTest, IntrusivePtrSharesOwnership) {
  boost::intrusive_ptr<ThriftConnContext> a{new ThriftConnContext()};
  EXPECT_EQ(a->use_count(), 1);

  auto b = a;
  EXPECT_EQ(a->use_count(), 2);
  EXPECT_EQ(a.get(), b.get());

  a.reset();
  EXPECT_EQ(b->use_count(), 1);
}

TEST(ThriftConnContextTest, IntrusivePtrDeletesOnLastReference) {
  // Probe destruction via the user-data deleter — it runs from
  // ~ThriftConnContext, which fires when the refcount drops to zero.
  bool destroyed = false;
  {
    boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
    ctx->setUserData(
        rocket::with_custom_deleter(
            &destroyed,
            +[](void* p) noexcept { *static_cast<bool*>(p) = true; }));
    EXPECT_FALSE(destroyed);
  }
  EXPECT_TRUE(destroyed);
}

} // namespace apache::thrift::fast_thrift::thrift
