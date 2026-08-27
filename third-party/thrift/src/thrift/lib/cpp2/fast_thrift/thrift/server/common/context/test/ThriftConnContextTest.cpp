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

#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>

namespace apache::thrift::fast_thrift::thrift {

// Stands in for the security layer's per-connection fields.
struct TestConnFields {
  int value{0};
};

TEST(ThriftConnContextTest, DefaultsAreEmpty) {
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
  EXPECT_TRUE(ctx->getPeerAddress().empty());
  EXPECT_TRUE(ctx->getSecurityProtocol().empty());
  EXPECT_EQ(ctx->getPeerCertificate(), nullptr);
  EXPECT_EQ(ctx->getUserData(), nullptr);
}

TEST(ThriftConnContextTest, SettersAreReflectedInGetters) {
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
  folly::SocketAddress addr("127.0.0.1", 1234);

  ctx->setPeerAddress(addr);
  ctx->setSecurityProtocol("TLS1.3");

  EXPECT_EQ(ctx->getPeerAddress(), addr);
  EXPECT_EQ(ctx->getSecurityProtocol(), "TLS1.3");
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

// A server with no security layer never fills the slot, and the context must
// not invent one.
TEST(ThriftConnContextTest, InternalFieldsAreEmptyUntilFilled) {
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
  EXPECT_FALSE(ctx->hasInternalFields());
}

// The context owns the fields outright: the object the constructor was handed
// is moved in, and what comes back out is that same object, not a copy.
TEST(ThriftConnContextTest, InternalFieldsAreOwnedByTheContext) {
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext(
      detail::InternalFieldsT{std::in_place_type<TestConnFields>})};
  ASSERT_TRUE(ctx->hasInternalFields());

  ctx->getInternalFields<TestConnFields>().value = 9;
  EXPECT_EQ(std::as_const(*ctx).getInternalFields<TestConnFields>().value, 9);
}

// Each connection has its own fields, never a shared one.
TEST(ThriftConnContextTest, InternalFieldsAreNotSharedBetweenConnections) {
  boost::intrusive_ptr<ThriftConnContext> first{new ThriftConnContext(
      detail::InternalFieldsT{std::in_place_type<TestConnFields>})};
  boost::intrusive_ptr<ThriftConnContext> second{new ThriftConnContext(
      detail::InternalFieldsT{std::in_place_type<TestConnFields>})};

  first->getInternalFields<TestConnFields>().value = 1;
  second->getInternalFields<TestConnFields>().value = 2;

  EXPECT_EQ(first->getInternalFields<TestConnFields>().value, 1);
  EXPECT_EQ(second->getInternalFields<TestConnFields>().value, 2);
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
