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

// Stands in for the security layer's per-connection fields. Owned outside the
// context, as the real ones are — they live on the Cpp2ConnContext.
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

// Nothing to read until something puts fields there: a server with no
// security layer has none, and asking must not invent one. An installed but
// empty slot reads the same way — there is no object to hand back.
TEST(ThriftConnContextTest, InternalFieldsAreNullUntilFilled) {
  detail::InternalFieldsT empty;
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
  EXPECT_EQ(ctx->getInternalFields<TestConnFields>(), nullptr);
  EXPECT_EQ(std::as_const(*ctx).getInternalFields<TestConnFields>(), nullptr);

  ctx->setInternalFields(&empty);
  EXPECT_EQ(ctx->getInternalFields<TestConnFields>(), nullptr);
  EXPECT_EQ(std::as_const(*ctx).getInternalFields<TestConnFields>(), nullptr);
}

// The point of the pointer: the context hands out the object its owner holds,
// so a write through the owner is the write this context reads.
TEST(ThriftConnContextTest, InternalFieldsAliasTheOwnersObject) {
  detail::InternalFieldsT owned{std::in_place_type<TestConnFields>};
  boost::intrusive_ptr<ThriftConnContext> ctx{new ThriftConnContext()};
  ctx->setInternalFields(&owned);

  owned.value<TestConnFields>().value = 7;
  ASSERT_NE(ctx->getInternalFields<TestConnFields>(), nullptr);
  EXPECT_EQ(ctx->getInternalFields<TestConnFields>()->value, 7);
  EXPECT_EQ(
      ctx->getInternalFields<TestConnFields>(), &owned.value<TestConnFields>());

  ctx->getInternalFields<TestConnFields>()->value = 9;
  EXPECT_EQ(owned.value<TestConnFields>().value, 9);
}

// Two connections point at their own owner's fields, never at each other's.
TEST(ThriftConnContextTest, InternalFieldsAreNotSharedBetweenConnections) {
  detail::InternalFieldsT firstFields{std::in_place_type<TestConnFields>};
  detail::InternalFieldsT secondFields{std::in_place_type<TestConnFields>};
  boost::intrusive_ptr<ThriftConnContext> first{new ThriftConnContext()};
  boost::intrusive_ptr<ThriftConnContext> second{new ThriftConnContext()};
  first->setInternalFields(&firstFields);
  second->setInternalFields(&secondFields);

  first->getInternalFields<TestConnFields>()->value = 1;
  second->getInternalFields<TestConnFields>()->value = 2;

  EXPECT_EQ(first->getInternalFields<TestConnFields>()->value, 1);
  EXPECT_EQ(second->getInternalFields<TestConnFields>()->value, 2);
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
