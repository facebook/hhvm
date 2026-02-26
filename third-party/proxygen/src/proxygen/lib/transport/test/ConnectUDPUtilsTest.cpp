/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/ConnectUDPUtils.h>

#include <folly/portability/GTest.h>

using namespace proxygen;

TEST(ConnectUDPUtilsTest, ExpandTemplateBasic) {
  auto target = expandConnectUDPTemplate(
      "https://127.0.0.1:4443/masque?h={target_host}&p={target_port}",
      "192.0.2.6",
      443);
  EXPECT_EQ(target.scheme, "https");
  EXPECT_EQ(target.authority, "127.0.0.1:4443");
  EXPECT_EQ(target.path, "/masque?h=192.0.2.6&p=443");
}

TEST(ConnectUDPUtilsTest, ExpandTemplateIPv6) {
  auto target = expandConnectUDPTemplate(
      "https://proxy.example:4443/masque?h={target_host}&p={target_port}",
      "2001:db8::1",
      8080);
  EXPECT_EQ(target.scheme, "https");
  EXPECT_EQ(target.authority, "proxy.example:4443");
  EXPECT_EQ(target.path, "/masque?h=2001%3Adb8%3A%3A1&p=8080");
}

TEST(ConnectUDPUtilsTest, ExpandTemplateWellKnown) {
  auto target = expandConnectUDPTemplate(
      "https://proxy.example:443/.well-known/masque/udp/{target_host}/"
      "{target_port}/",
      "target.example.com",
      443);
  EXPECT_EQ(target.scheme, "https");
  EXPECT_EQ(target.authority, "proxy.example:443");
  EXPECT_EQ(target.path, "/.well-known/masque/udp/target.example.com/443/");
}

TEST(ConnectUDPUtilsTest, ExpandTemplateNoPath) {
  auto target =
      expandConnectUDPTemplate("https://proxy.example:443", "host", 80);
  EXPECT_EQ(target.scheme, "https");
  EXPECT_EQ(target.authority, "proxy.example:443");
  EXPECT_EQ(target.path, "/");
}

TEST(ConnectUDPUtilsTest, PrependContextId) {
  auto payload = folly::IOBuf::copyBuffer("hello");
  auto result = prependContextId(std::move(payload));
  ASSERT_NE(result, nullptr);
  result->coalesce();
  EXPECT_EQ(result->length(), 6);
  EXPECT_EQ(result->data()[0], 0x00);
  EXPECT_EQ(std::string((const char*)result->data() + 1, 5), "hello");
}

TEST(ConnectUDPUtilsTest, StripContextIdZero) {
  // Build datagram with context ID 0 + payload "hello"
  auto buf = folly::IOBuf::create(6);
  buf->append(6);
  buf->writableData()[0] = 0x00;
  memcpy(buf->writableData() + 1, "hello", 5);

  auto result = stripContextId(std::move(buf));
  ASSERT_TRUE(result.has_value());
  result.value()->coalesce();
  EXPECT_EQ(std::string((const char*)result.value()->data(),
                        result.value()->length()),
            "hello");
}

TEST(ConnectUDPUtilsTest, StripContextIdNonZero) {
  // Build datagram with context ID 1 + payload
  auto buf = folly::IOBuf::create(6);
  buf->append(6);
  buf->writableData()[0] = 0x01;
  memcpy(buf->writableData() + 1, "hello", 5);

  auto result = stripContextId(std::move(buf));
  EXPECT_FALSE(result.has_value());
}

TEST(ConnectUDPUtilsTest, StripContextIdEmpty) {
  auto result = stripContextId(nullptr);
  EXPECT_FALSE(result.has_value());
}

TEST(ConnectUDPUtilsTest, StripContextIdEmptyPayload) {
  // Context ID 0 with no payload
  auto buf = folly::IOBuf::create(1);
  buf->append(1);
  buf->writableData()[0] = 0x00;

  auto result = stripContextId(std::move(buf));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value()->computeChainDataLength(), 0);
}
