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

#include <thrift/conformance/cpp2/Any.h>

#include <gtest/gtest.h>
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/cpp2/Testing.h>

namespace apache::thrift::conformance {
namespace {

TEST(AnyTest, SetProtocol) {
  Any any;
  Protocol customProtocol("Hi");
  setProtocol(customProtocol, any);
  EXPECT_TRUE(hasProtocol(any, customProtocol));
  EXPECT_EQ(getProtocol(any), customProtocol);
  EXPECT_TRUE(any.customProtocol().has_value());
  EXPECT_TRUE(any.protocol().has_value());

  setProtocol(getStandardProtocol<StandardProtocol::Compact>(), any);
  EXPECT_TRUE(
      hasProtocol(any, getStandardProtocol<StandardProtocol::Compact>()));
  EXPECT_EQ(getProtocol(any), getStandardProtocol<StandardProtocol::Compact>());
  EXPECT_FALSE(any.customProtocol().has_value());
  EXPECT_FALSE(any.protocol().has_value());

  setProtocol(customProtocol, any);
  EXPECT_TRUE(any.customProtocol().has_value());
  EXPECT_TRUE(any.protocol().has_value());

  setProtocol(getStandardProtocol<StandardProtocol::Binary>(), any);
  EXPECT_TRUE(
      hasProtocol(any, getStandardProtocol<StandardProtocol::Binary>()));
  EXPECT_EQ(getProtocol(any), getStandardProtocol<StandardProtocol::Binary>());
  EXPECT_FALSE(any.customProtocol().has_value());
  EXPECT_TRUE(any.protocol().has_value());
}

TEST(AnyTest, None) {
  Any any;
  validateAny(any);
  EXPECT_EQ(getProtocol(any), getStandardProtocol<StandardProtocol::Compact>());
  any.protocol() = StandardProtocol::Custom;
  validateAny(any);
  EXPECT_EQ(getProtocol(any), getStandardProtocol<StandardProtocol::Custom>());
  any.customProtocol() = "";
  validateAny(any);
  EXPECT_EQ(getProtocol(any), getStandardProtocol<StandardProtocol::Custom>());

  any.customProtocol() = "Custom";
  EXPECT_NE(getProtocol(any), getStandardProtocol<StandardProtocol::Custom>());
  EXPECT_THROW(validateAny(any), std::invalid_argument);
}

TEST(AnyTest, Standard) {
  Any any;
  any.protocol() = StandardProtocol::Binary;
  EXPECT_EQ(getProtocol(any), getStandardProtocol<StandardProtocol::Binary>());
  EXPECT_TRUE(
      hasProtocol(any, getStandardProtocol<StandardProtocol::Binary>()));

  // Junk in the customProtocol.
  any.customProtocol() = "Ignored";
  EXPECT_EQ(getProtocol(any), getStandardProtocol<StandardProtocol::Binary>());
  EXPECT_TRUE(
      hasProtocol(any, getStandardProtocol<StandardProtocol::Binary>()));

  // Unnormalize name.
  any.protocol() = StandardProtocol::Custom;
  any.customProtocol() = "Binary";
  EXPECT_NE(getProtocol(any), getStandardProtocol<StandardProtocol::Binary>());
  EXPECT_THROW(validateAny(any), std::invalid_argument);
}

TEST(AnyTest, Custom) {
  Any any;
  any.protocol() = StandardProtocol::Custom;
  any.customProtocol() = "Hi";
  EXPECT_EQ(getProtocol(any), Protocol("Hi"));
  EXPECT_TRUE(hasProtocol(any, Protocol("Hi")));
  EXPECT_NE(getProtocol(any), Protocol("Bye"));
  EXPECT_FALSE(hasProtocol(any, Protocol("Bye")));
  EXPECT_NE(getProtocol(any), Protocol(StandardProtocol::Custom));
  EXPECT_FALSE(hasProtocol(any, Protocol(StandardProtocol::Custom)));
  EXPECT_NE(getProtocol(any), Protocol(StandardProtocol::Binary));
  EXPECT_FALSE(hasProtocol(any, Protocol(StandardProtocol::Binary)));
}

TEST(AnyTest, Unknown) {
  Any any;
  any.protocol() = kUnknownStdProtocol;
  EXPECT_EQ(getProtocol(any), UnknownProtocol());
  EXPECT_TRUE(hasProtocol(any, UnknownProtocol()));
  EXPECT_EQ(getProtocol(any).name(), "");
}

TEST(AnyTest, Custom_EmptyString) {
  Any any;
  // Empty string protocol is the same as None
  any.protocol() = StandardProtocol::Custom;
  any.customProtocol() = "";
  EXPECT_TRUE(any.customProtocol().has_value());
  EXPECT_EQ(getProtocol(any), kNoProtocol);
  EXPECT_TRUE(hasProtocol(any, kNoProtocol));
}

TEST(AnyTest, ValidateAny) {
  const auto bad = "foo.com:42/my/type";
  const auto good = "foo.com/my/type";
  Any any;
  validateAny(any);
  any.type() = "";
  any.customProtocol() = "";
  validateAny(any);
  any.type().ensure() = bad;
  EXPECT_THROW(validateAny(any), std::invalid_argument);
  any.type() = good;
  any.customProtocol().ensure() = bad;
  EXPECT_THROW(validateAny(any), std::invalid_argument);
  any.customProtocol() = good;
  validateAny(any);
}

} // namespace
} // namespace apache::thrift::conformance
