/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/ProxyStatus.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersDecoder.h>

#include <folly/portability/GTest.h>

#include <string>

using namespace proxygen;

TEST(ProxyStatusTest, TestUpdatingStatusType) {
  ProxyStatus proxy_status{};
  EXPECT_EQ(proxy_status.getStatusType(), StatusType::ENUM_COUNT);
  EXPECT_TRUE(proxy_status.isEmpty());

  proxy_status.setStatusType(StatusType::connection_timeout);
  EXPECT_EQ(proxy_status.getStatusType(), StatusType::connection_timeout);
  EXPECT_FALSE(proxy_status.isEmpty());

  proxy_status.setStatusType(StatusType::http_protocol_error);
  EXPECT_EQ(proxy_status.getStatusType(), StatusType::http_protocol_error);
  EXPECT_FALSE(proxy_status.isEmpty());

  proxy_status.setStatusType(StatusType::proxy_internal_error);
  EXPECT_EQ(proxy_status.getStatusType(), StatusType::proxy_internal_error);
  EXPECT_FALSE(proxy_status.isEmpty());

  proxy_status.setStatusType(StatusType::ENUM_COUNT);
  EXPECT_EQ(proxy_status.getStatusType(), StatusType::ENUM_COUNT);
  EXPECT_TRUE(proxy_status.isEmpty());
}

TEST(ProxyStatusTest, TestStatusSerialization) {
  ProxyStatus proxy_status{StatusType::connection_timeout};

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_STREQ(str.c_str(), "connection_timeout");

  EXPECT_EQ(parameterisedList.size(), 1);
  EXPECT_EQ(parameterisedList[0].identifier, "connection_timeout");
  EXPECT_EQ(parameterisedList[0].parameterMap.size(), 0);
}

TEST(ProxyStatusTest, TestEmpty) {
  ProxyStatus proxy_status{};
  EXPECT_TRUE(proxy_status.isEmpty());

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_STREQ(str.c_str(), "");
  EXPECT_EQ(parameterisedList.size(), 0);
}

TEST(ProxyStatusTest, TestMissingStatus) {
  ProxyStatus proxy_status{};
  EXPECT_TRUE(proxy_status.isEmpty());
  proxy_status.setProxyStatusParameter("a", "1");

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_STREQ(str.c_str(), "");
  EXPECT_EQ(parameterisedList.size(), 0);
}

TEST(ProxyStatusTest, TestUpstreamIP) {
  ProxyStatus proxy_status{StatusType::http_protocol_error};
  EXPECT_FALSE(proxy_status.hasUpstreamIP());
  proxy_status.setUpstreamIP("upstreamIP");
  EXPECT_TRUE(proxy_status.hasUpstreamIP());

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_EQ(parameterisedList.size(), 1);
  EXPECT_EQ(parameterisedList[0].identifier, "http_protocol_error");
  EXPECT_EQ(parameterisedList[0].parameterMap.size(), 1);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_upip"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_upip"],
            std::string("upstreamIP"));
}

TEST(ProxyStatusTest, TestProxy) {
  ProxyStatus proxy_status{StatusType::proxy_internal_error};
  proxy_status.setProxy("proxy");

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_EQ(parameterisedList.size(), 1);
  EXPECT_EQ(parameterisedList[0].identifier, "proxy_internal_error");
  EXPECT_EQ(parameterisedList[0].parameterMap.size(), 1);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_proxy"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_proxy"], std::string("proxy"));
}

TEST(ProxyStatusTest, TestSerialization) {
  ProxyStatus proxy_status{StatusType::proxy_internal_error};
  EXPECT_FALSE(proxy_status.hasUpstreamIP());
  proxy_status.setUpstreamIP("upstreamIP");
  EXPECT_TRUE(proxy_status.hasUpstreamIP());
  proxy_status.setProxy("proxy");
  proxy_status.setProxyStatusParameter("a", "1");
  proxy_status.setProxyStatusParameter("b", "");
  proxy_status.setStatusType(StatusType::http_protocol_error);

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_EQ(parameterisedList.size(), 1);
  EXPECT_EQ(parameterisedList[0].identifier, "http_protocol_error");
  EXPECT_EQ(parameterisedList[0].parameterMap.size(), 3);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_upip"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_proxy"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["a"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_upip"],
            std::string("upstreamIP"));
  EXPECT_EQ(parameterisedList[0].parameterMap["e_proxy"], std::string("proxy"));
  EXPECT_EQ(parameterisedList[0].parameterMap["a"], std::string("1"));
}

TEST(ProxyStatusTest, TestSetProxyError) {
  ProxyStatus proxy_status{StatusType::proxy_internal_error};
  proxy_status.setProxyError(false);

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_EQ(parameterisedList.size(), 1);
  EXPECT_EQ(parameterisedList[0].identifier, "proxy_internal_error");
  EXPECT_EQ(parameterisedList[0].parameterMap.size(), 1);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_isproxyerr"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_isproxyerr"],
            std::string("false"));
}

TEST(ProxyStatusTest, TestSetServerError) {
  ProxyStatus proxy_status{StatusType::proxy_internal_error};
  proxy_status.setServerError(true);

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_EQ(parameterisedList.size(), 1);
  EXPECT_EQ(parameterisedList[0].identifier, "proxy_internal_error");
  EXPECT_EQ(parameterisedList[0].parameterMap.size(), 1);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_isservererr"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_isservererr"],
            std::string("true"));
}

TEST(ProxyStatusTest, TestSetClientError) {
  ProxyStatus proxy_status{StatusType::proxy_internal_error};
  proxy_status.setClientError(true);

  auto str = proxy_status.toString();
  StructuredHeadersDecoder decoder(str);
  StructuredHeaders::ParameterisedList parameterisedList;
  decoder.decodeParameterisedList(parameterisedList);

  EXPECT_EQ(parameterisedList.size(), 1);
  EXPECT_EQ(parameterisedList[0].identifier, "proxy_internal_error");
  EXPECT_EQ(parameterisedList[0].parameterMap.size(), 1);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_isclienterr"].tag,
            StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(parameterisedList[0].parameterMap["e_isclienterr"],
            std::string("true"));
}
