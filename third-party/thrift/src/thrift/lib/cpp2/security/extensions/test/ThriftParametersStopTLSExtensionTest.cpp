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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache::thrift {

// Test class for both StopTLS and StopTLSV2
class ThriftParametersExtensionStopTLSTest
    : public testing::TestWithParam<std::tuple<bool, bool>> {};

TEST_P(ThriftParametersExtensionStopTLSTest, testClientExtension) {
  auto [clientSupport, serverSupport] = GetParam();

  // set up client
  auto context = std::make_shared<ThriftParametersContext>();
  context->setUseStopTLS(clientSupport);
  auto extensions = std::make_shared<ThriftParametersClientExtension>(context);

  // set up server
  std::vector<fizz::Extension> serverExtensions;
  NegotiationParameters params;
  params.useStopTLS_ref() = serverSupport;
  ThriftParametersExt paramsExt;
  paramsExt.params = params;
  serverExtensions.push_back(encodeThriftExtension(paramsExt));

  extensions->onEncryptedExtensions(serverExtensions);

  if (clientSupport && serverSupport) {
    EXPECT_TRUE(extensions->getNegotiatedStopTLS());
  } else {
    EXPECT_FALSE(extensions->getNegotiatedStopTLS());
  }
}

TEST_P(ThriftParametersExtensionStopTLSTest, testServerExtension) {
  auto [clientSupport, serverSupport] = GetParam();

  // set up server
  auto context = std::make_shared<ThriftParametersContext>();
  context->setUseStopTLS(serverSupport);
  auto extensions = std::make_shared<ThriftParametersServerExtension>(context);

  // set up client
  ThriftParametersExt clientThriftParams;
  clientThriftParams.params.useStopTLS_ref() = clientSupport;

  fizz::ClientHello chlo;
  chlo.extensions.push_back(encodeThriftExtension(clientThriftParams));

  auto exts = extensions->getExtensions(chlo);
  EXPECT_EQ(exts.size(), 1);

  auto thriftParametersExtension = getThriftExtension(exts);
  EXPECT_TRUE(thriftParametersExtension.has_value());
  if (clientSupport && serverSupport) {
    EXPECT_TRUE(extensions->getNegotiatedStopTLS());
  } else {
    EXPECT_FALSE(extensions->getNegotiatedStopTLS());
  }
}

TEST_P(ThriftParametersExtensionStopTLSTest, testClientExtensionStopTLSV2) {
  auto [clientSupport, serverSupport] = GetParam();

  // set up client
  auto context = std::make_shared<ThriftParametersContext>();
  context->setUseStopTLSV2(clientSupport);
  auto extensions = std::make_shared<ThriftParametersClientExtension>(context);

  // set up server
  std::vector<fizz::Extension> serverExtensions;
  NegotiationParameters params;
  if (serverSupport) {
    params.useStopTLSV2_ref() = true;
  }
  ThriftParametersExt paramsExt;
  paramsExt.params = params;
  serverExtensions.push_back(encodeThriftExtension(paramsExt));

  extensions->onEncryptedExtensions(serverExtensions);

  if (clientSupport && serverSupport) {
    EXPECT_TRUE(extensions->getNegotiatedStopTLSV2());
  } else {
    EXPECT_FALSE(extensions->getNegotiatedStopTLSV2());
  }
}

TEST_P(ThriftParametersExtensionStopTLSTest, testServerExtensionStopTLSV2) {
  auto [clientSupport, serverSupport] = GetParam();

  // set up server
  auto context = std::make_shared<ThriftParametersContext>();
  context->setUseStopTLSV2(serverSupport);
  auto extensions = std::make_shared<ThriftParametersServerExtension>(context);

  // set up client
  ThriftParametersExt clientThriftParams;
  if (clientSupport) {
    clientThriftParams.params.useStopTLSV2_ref() = true;
  }
  fizz::ClientHello chlo;
  chlo.extensions.push_back(encodeThriftExtension(clientThriftParams));

  auto exts = extensions->getExtensions(chlo);
  EXPECT_EQ(exts.size(), 1);

  auto thriftParametersExtension = getThriftExtension(exts);
  EXPECT_TRUE(thriftParametersExtension.has_value());
  if (clientSupport && serverSupport) {
    EXPECT_TRUE(extensions->getNegotiatedStopTLSV2());
  } else {
    EXPECT_FALSE(extensions->getNegotiatedStopTLSV2());
  }
}

// This test verifies that both StopTLS and StopTLSV2 can be negotiated
// independently
TEST_F(ThriftParametersExtensionStopTLSTest, testCombinedNegotiation) {
  // Test all combinations of StopTLS and StopTLSV2 support
  for (bool clientStopTLS : {false, true}) {
    for (bool clientStopTLSV2 : {false, true}) {
      for (bool serverStopTLS : {false, true}) {
        for (bool serverStopTLSV2 : {false, true}) {
          // Set up client
          auto context = std::make_shared<ThriftParametersContext>();
          context->setUseStopTLS(clientStopTLS);
          context->setUseStopTLSV2(clientStopTLSV2);
          auto extensions =
              std::make_shared<ThriftParametersClientExtension>(context);

          // Set up server
          std::vector<fizz::Extension> serverExtensions;
          NegotiationParameters params;
          if (serverStopTLS) {
            params.useStopTLS_ref() = true;
          }
          if (serverStopTLSV2) {
            params.useStopTLSV2_ref() = true;
          }
          ThriftParametersExt paramsExt;
          paramsExt.params = params;
          serverExtensions.push_back(encodeThriftExtension(paramsExt));

          extensions->onEncryptedExtensions(serverExtensions);

          // Check StopTLS negotiation
          EXPECT_EQ(
              extensions->getNegotiatedStopTLS(),
              clientStopTLS && serverStopTLS);

          // Check StopTLSV2 negotiation
          EXPECT_EQ(
              extensions->getNegotiatedStopTLSV2(),
              clientStopTLSV2 && serverStopTLSV2);
        }
      }
    }
  }
}

INSTANTIATE_TEST_CASE_P(
    StopTLSNegotationTest,
    ThriftParametersExtensionStopTLSTest,
    testing::Combine(
        testing::Values(false, true), testing::Values(false, true)));
} // namespace apache::thrift
