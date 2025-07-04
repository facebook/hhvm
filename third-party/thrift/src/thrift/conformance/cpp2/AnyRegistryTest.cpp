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

#include <thrift/conformance/cpp2/AnyRegistry.h>

#include <gtest/gtest.h>

#include <thrift/conformance/cpp2/Any.h>
#include <thrift/conformance/cpp2/Testing.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache::thrift::conformance {
using type::kDisableUniversalHash;

namespace {

TEST(AnyRegistryTest, ShortType) {
  AnyRegistry registry;
  FollyToStringSerializer<int> intCodec;
  EXPECT_TRUE(registry.registerType<int>(shortThriftType(), {&intCodec}));

  // Should use the type uri because it is shorter than the id.
  Any any = registry.store(1, kFollyToStringProtocol);
  EXPECT_TRUE(any.type());
  EXPECT_FALSE(any.typeHashPrefixSha2_256());
  EXPECT_EQ(registry.load<int>(any), 1);
  EXPECT_EQ(*registry.tryGetTypeId(any), typeid(int));
  EXPECT_EQ(registry.getTypeId(any), typeid(int));
  EXPECT_EQ(registry.getTypeUri(any), shortThriftType().uri().value());
}

void checkLongType(int typeBytes, int expectedOutBytes) {
  AnyRegistry registry;
  FollyToStringSerializer<int> intCodec;
  auto longType = longThriftType();
  if (typeBytes == kTypeHashBytesNotSpecified) {
    longType.typeHashBytes().reset();
  } else {
    longType.typeHashBytes() = typeBytes;
  }
  EXPECT_TRUE(registry.registerType<int>(longType, {&intCodec}));

  // Should use the type id because it is shorter than the uri.
  Any any = registry.store(1, kFollyToStringProtocol);
  if (expectedOutBytes == kDisableUniversalHash) {
    EXPECT_FALSE(any.typeHashPrefixSha2_256());
    EXPECT_TRUE(any.type());
    EXPECT_EQ(
        registry.getSerializerByUri(*any.type(), intCodec.getProtocol()),
        &intCodec);
  } else {
    EXPECT_FALSE(any.type());
    ASSERT_TRUE(any.typeHashPrefixSha2_256());
    EXPECT_EQ(
        any.typeHashPrefixSha2_256().value_unchecked().size(),
        expectedOutBytes);
    EXPECT_EQ(
        registry.getSerializerByHash(
            type::UniversalHashAlgorithm::Sha2_256,
            *any.typeHashPrefixSha2_256(),
            intCodec.getProtocol()),
        &intCodec);
  }
  EXPECT_EQ(registry.load<int>(any), 1);
  EXPECT_EQ(*registry.tryGetTypeId(any), typeid(int));
  EXPECT_EQ(registry.getTypeId(any), typeid(int));
  EXPECT_EQ(registry.getTypeUri(any), longType.uri().value());
}

TEST(AnyRegistryTest, LongType) {
  // Disabled is respected.
  THRIFT_SCOPED_CHECK(
      checkLongType(kDisableUniversalHash, kDisableUniversalHash));

  // Unset uses default.
  THRIFT_SCOPED_CHECK(
      checkLongType(kTypeHashBytesNotSpecified, type::kDefaultTypeHashBytes));

  // Type can increase bytes used.
  THRIFT_SCOPED_CHECK(checkLongType(24, 24));
  THRIFT_SCOPED_CHECK(checkLongType(32, 32));
}

TEST(AnyRegistryTest, ShortTypeHash) {
  AnyRegistry registry;
  FollyToStringSerializer<int> intCodec;
  auto longType = longThriftType();
  EXPECT_TRUE(registry.registerType<int>(longType, {&intCodec}));
  Any any = registry.store(1, kFollyToStringProtocol);
  any.typeHashPrefixSha2_256() = any.typeHashPrefixSha2_256()->substr(0, 7);
  EXPECT_THROW(registry.load<int>(any), std::out_of_range);
}

TEST(AnyRegistryTest, TypeNotFound) {
  AnyRegistry registry;
  EXPECT_EQ(registry.getTypeUri<int>(), "");
  EXPECT_EQ((registry.getSerializer<int, StandardProtocol::Binary>()), nullptr);

  EXPECT_THROW(registry.store<StandardProtocol::Binary>(1), std::out_of_range);

  Any any;
  EXPECT_THROW(registry.load<int>(any), std::invalid_argument);
  any.type() = thriftType("int");
  EXPECT_THROW(registry.load<int>(any), std::out_of_range);
  any.protocol() = StandardProtocol::Binary;
  EXPECT_THROW(registry.load<int>(any), std::out_of_range);

  FollyToStringSerializer<int> intCodec;
  EXPECT_THROW(registry.registerSerializer<int>(&intCodec), std::out_of_range);
}

TEST(AnyRegistryTest, ProtocolNotFound) {
  AnyRegistry registry;
  EXPECT_TRUE(registry.registerType<int>(testThriftType("int")));
  EXPECT_EQ(registry.getTypeUri<int>(), thriftType("int"));
  EXPECT_EQ((registry.getSerializer<int, StandardProtocol::Binary>()), nullptr);

  EXPECT_THROW(registry.store<StandardProtocol::Binary>(1), std::out_of_range);

  Any any;
  EXPECT_THROW(registry.load<int>(any), std::invalid_argument);
  any.type() = thriftType("int");
  EXPECT_THROW(registry.load<int>(any), std::out_of_range);
  any.protocol() = StandardProtocol::Binary;
  EXPECT_THROW(registry.load<int>(any), std::out_of_range);
}

TEST(AnyRegistryTest, TypeHashToShort) {
  AnyRegistry registry;
  FollyToStringSerializer<int> intCodec;
  auto anyType = longThriftType();
  anyType.typeHashBytes() = 17;
  EXPECT_TRUE(registry.registerType<int>(anyType, {&intCodec}));
  Any any = registry.store(1, intCodec.getProtocol());
  ASSERT_TRUE(any.typeHashPrefixSha2_256());
  EXPECT_EQ(any.typeHashPrefixSha2_256()->size(), 17);
  EXPECT_EQ(registry.load<int>(any), 1);

  any.typeHashPrefixSha2_256() = any.typeHashPrefixSha2_256()->substr(0, 8);
  EXPECT_EQ(registry.load<int>(any), 1);

  any.typeHashPrefixSha2_256() = any.typeHashPrefixSha2_256()->substr(0, 7);
  EXPECT_THROW(registry.load<int>(any), std::out_of_range);
}

TEST(AnyRegistryTest, Behavior) {
  AnyRegistry registry;
  const AnyRegistry& cregistry = registry;
  EXPECT_EQ(cregistry.getTypeUri(typeid(int)), "");
  EXPECT_EQ(cregistry.getSerializer<int>(kFollyToStringProtocol), nullptr);

  FollyToStringSerializer<int> intCodec;

  // Type must be registered first.
  EXPECT_THROW(registry.registerSerializer<int>(&intCodec), std::out_of_range);

  // Empty string is rejected.
  EXPECT_THROW(
      registry.registerType<int>(testThriftType("")), std::invalid_argument);

  EXPECT_TRUE(registry.registerType<int>(testThriftType("int")));
  EXPECT_EQ(cregistry.getTypeUri(typeid(int)), thriftType("int"));
  EXPECT_EQ(cregistry.getSerializer<int>(kFollyToStringProtocol), nullptr);

  // Conflicting and duplicate registrations are rejected.
  EXPECT_FALSE(registry.registerType<int>(testThriftType("int")));
  EXPECT_FALSE(registry.registerType<int>(testThriftType("other-int")));
  EXPECT_FALSE(registry.registerType<double>(testThriftType("int")));

  EXPECT_TRUE(registry.registerSerializer<int>(&intCodec));
  EXPECT_EQ(cregistry.getTypeUri<int>(), thriftType("int"));
  EXPECT_EQ(cregistry.getSerializer<int>(kFollyToStringProtocol), &intCodec);

  // Duplicate registrations are rejected.
  EXPECT_FALSE(registry.registerSerializer<int>(&intCodec));

  Number1Serializer number1Codec;
  EXPECT_TRUE(registry.registerSerializer<int>(&number1Codec));

  EXPECT_TRUE(registry.registerType<double>(testThriftType("double")));

  // nullptr is rejected.
  EXPECT_FALSE(registry.registerSerializer<double>(nullptr));

  EXPECT_TRUE(registry.registerSerializer<double>(
      std::make_unique<FollyToStringSerializer<double>>()));

  Any value = cregistry.store(3, kFollyToStringProtocol);
  EXPECT_EQ(value.type().value_or(""), thriftType("int"));
  EXPECT_FALSE(value.typeHashPrefixSha2_256().has_value());
  EXPECT_EQ(toString(*value.data()), "3");
  EXPECT_TRUE(hasProtocol(value, kFollyToStringProtocol));
  EXPECT_EQ(std::any_cast<int>(cregistry.load(value)), 3);
  EXPECT_EQ(cregistry.load<int>(value), 3);

  // Storing an Any does nothing if the protocols match.
  Any original = value;
  value = cregistry.store(original, kFollyToStringProtocol);
  EXPECT_EQ(value.type().value_or(""), thriftType("int"));
  EXPECT_FALSE(value.typeHashPrefixSha2_256().has_value());
  EXPECT_EQ(toString(*value.data()), "3");
  EXPECT_TRUE(hasProtocol(value, kFollyToStringProtocol));
  value =
      cregistry.store(std::any(std::move(original)), kFollyToStringProtocol);
  EXPECT_EQ(value.type().value_or(""), thriftType("int"));
  EXPECT_FALSE(value.typeHashPrefixSha2_256().has_value());
  EXPECT_EQ(toString(*value.data()), "3");
  EXPECT_TRUE(hasProtocol(value, kFollyToStringProtocol));

  // Storing an Any with a different protocol does a conversion.
  original = value;
  value = cregistry.store(original, Number1Serializer::kProtocol);
  EXPECT_EQ(value.type().value_or(""), thriftType("int"));
  EXPECT_FALSE(value.typeHashPrefixSha2_256().has_value());
  EXPECT_EQ(toString(*value.data()), "number 1!!");
  EXPECT_TRUE(hasProtocol(value, Number1Serializer::kProtocol));
  value = cregistry.store(
      std::any(std::move(original)), Number1Serializer::kProtocol);
  EXPECT_EQ(value.type().value_or(""), thriftType("int"));
  EXPECT_FALSE(value.typeHashPrefixSha2_256().has_value());
  EXPECT_EQ(toString(*value.data()), "number 1!!");
  EXPECT_TRUE(hasProtocol(value, Number1Serializer::kProtocol));
  EXPECT_EQ(std::any_cast<int>(cregistry.load(value)), 1);
  EXPECT_EQ(cregistry.load<int>(value), 1);

  // Storing an unsupported type is an error.
  EXPECT_THROW(
      cregistry.store(2.5f, kFollyToStringProtocol), std::out_of_range);
  EXPECT_THROW(
      cregistry.store(std::any(2.5f), kFollyToStringProtocol),
      std::out_of_range);

  // Storing using an unsupported protocol throws an error
  EXPECT_THROW(
      cregistry.store(3, Protocol(StandardProtocol::Binary)),
      std::out_of_range);

  // Loading an empty Any value throws an error.
  value = {};
  EXPECT_FALSE(value.type().has_value());
  EXPECT_FALSE(value.typeHashPrefixSha2_256().has_value());
  EXPECT_EQ(toString(*value.data()), "");
  EXPECT_TRUE(
      hasProtocol(value, getStandardProtocol<StandardProtocol::Compact>()));
  EXPECT_THROW(cregistry.load(value), std::invalid_argument);
  EXPECT_THROW(cregistry.load<float>(value), std::invalid_argument);
  value.type() = "foo";
  EXPECT_THROW(cregistry.load(value), std::out_of_range);
  EXPECT_THROW(cregistry.load<float>(value), std::out_of_range);

  value = cregistry.store(2.5, kFollyToStringProtocol);
  EXPECT_EQ(*value.type(), thriftType("double"));
  EXPECT_FALSE(value.typeHashPrefixSha2_256().has_value());
  EXPECT_EQ(toString(*value.data()), "2.5");
  EXPECT_TRUE(hasProtocol(value, kFollyToStringProtocol));
  EXPECT_EQ(std::any_cast<double>(cregistry.load(value)), 2.5);
  EXPECT_EQ(cregistry.load<double>(value), 2.5);
  EXPECT_THROW(cregistry.load<int>(value), std::bad_any_cast);

  EXPECT_EQ(
      cregistry.debugString(),
      "AnyRegistry[\n"
      "  facebook.com/thrift/double (319b4d9a143e15bbf818e1fe4556f46a578cb58acda43d5f40cf9a22886dc9d8):\n"
      "    facebook.com/thrift/FollyToString,\n"
      "  facebook.com/thrift/int (3fc51d1641587f7d26c7ab0dcb97b69f6ea48f9ea15ea626ec34e6069fc4b136):\n"
      "    facebook.com/thrift/FollyToString,\n"
      "    facebook.com/thrift/Number1,\n"
      "]");
}

TEST(AnyRegistryTest, Aliases) {
  AnyRegistry registry;
  const AnyRegistry& cregistry = registry;
  FollyToStringSerializer<int> intCodec;
  Number1Serializer oneCodec;

  EXPECT_TRUE(registry.registerType<int>(
      testThriftType({"int", "Int", "Integer"}), {&oneCodec, &intCodec}));
  EXPECT_EQ(registry.getTypeUri<int>(), thriftType("int"));
  EXPECT_EQ(
      registry.getSerializerByUri(thriftType("int"), oneCodec.getProtocol()),
      &oneCodec);
  EXPECT_EQ(
      registry.getSerializerByUri(thriftType("Int"), oneCodec.getProtocol()),
      &oneCodec);
  EXPECT_EQ(
      registry.getSerializerByUri(
          thriftType("Integer"), oneCodec.getProtocol()),
      &oneCodec);

  auto any = cregistry.store(1, kFollyToStringProtocol);
  // Stored under the main type uri.
  EXPECT_EQ(any.type().value_or(""), thriftType("int"));
  EXPECT_EQ(cregistry.load<int>(any), 1);

  any.type() = thriftType("Int");
  EXPECT_EQ(cregistry.load<int>(any), 1);

  any.type() = thriftType("Integer");
  EXPECT_EQ(cregistry.load<int>(any), 1);

  any.type() = thriftType("Unknown");
  EXPECT_THROW(cregistry.load<int>(any), std::out_of_range);
}

TEST(AnyRegistryTest, ForwardCompat_Protocol) {
  AnyRegistry registry;
  Protocol invalidProtocol("invalid");
  EXPECT_THROW(validateProtocol(invalidProtocol), std::invalid_argument);
  // Lookup does not throw.
  EXPECT_EQ(registry.getSerializer<int>(invalidProtocol), nullptr);
}

TEST(AnyRegistryTest, ForwardCompat_Any) {
  AnyRegistry registry;
  const AnyRegistry& cregistry = registry;
  FollyToStringSerializer<int> intCodec;

  EXPECT_TRUE(registry.registerType<int>(testThriftType("int")));
  EXPECT_TRUE(registry.registerSerializer<int>(&intCodec));

  Any any = cregistry.store(1, kFollyToStringProtocol);

  validateAny(any);
  any.type() = "invalid";
  EXPECT_THROW(validateAny(any), std::invalid_argument);
  // Load does not throw std::invalid_argument.
  EXPECT_THROW(cregistry.load(any), std::out_of_range);
}

TEST(AnyRegistryTest, StdProtocol) {
  AnyRegistry registry;
  const AnyRegistry& cregistry = registry;
  registry.registerType<
      protocol::Value,
      StandardProtocol::Binary,
      StandardProtocol::Compact>(testThriftType("Value"));

  auto value = protocol::asValueStruct<type::i32_t>(1);
  auto any = cregistry.store<StandardProtocol::Compact>(value);
  ASSERT_TRUE(any.type());
  EXPECT_EQ(any.type().value_unchecked(), thriftType("Value"));
  EXPECT_EQ(cregistry.load<protocol::Value>(any), value);
}

TEST(AnyRegistryTest, Generated) {
  // Double register fails with a runtime error.
  EXPECT_THROW(
      detail::registerGeneratedStruct<protocol::Value>(), std::runtime_error);

  auto value = protocol::asValueStruct<type::i32_t>(1);
  auto any = AnyRegistry::generated().store<StandardProtocol::Compact>(value);
  EXPECT_EQ(AnyRegistry::generated().load<protocol::Value>(any), value);
  EXPECT_THROW(
      AnyRegistry::generated().store<StandardProtocol::Json>(value),
      std::out_of_range);
}

TEST(AnyRegistryTest, ForceRegister) {
  AnyRegistry registry;
  EXPECT_TRUE(registry.forceRegisterType(typeid(protocol::Value), "va"));
  EXPECT_TRUE(registry.registerSerializer<protocol::Value>(
      &getAnyStandardSerializer<protocol::Value, StandardProtocol::Compact>()));
  protocol::Value expected;
  expected.boolValue_ref() = true;
  Any any = registry.store(
      expected, getStandardProtocol<StandardProtocol::Compact>());
  EXPECT_EQ(any.type(), "va");
  protocol::Value actual = registry.load<protocol::Value>(any);
  EXPECT_EQ(actual, expected);
}

} // namespace
} // namespace apache::thrift::conformance
