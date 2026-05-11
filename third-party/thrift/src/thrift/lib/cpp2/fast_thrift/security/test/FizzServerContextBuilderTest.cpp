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

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

namespace apache::thrift::fast_thrift::security::test {

class FizzServerContextBuilderTest : public ::testing::Test {
 protected:
  TestCert cert_ = makeTestCert();
  ThriftTlsConfig thriftConfig_{};
};

TEST_F(FizzServerContextBuilderTest, BuildsFromPemBuffers) {
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;

  auto ctx = buildFizzServerContext(config, thriftConfig_);
  ASSERT_NE(ctx, nullptr);
  EXPECT_EQ(ctx->getSupportedAlpns(), std::vector<std::string>{"rs"});
  EXPECT_EQ(ctx->getClientAuthMode(), fizz::server::ClientAuthMode::Required);
}

TEST_F(FizzServerContextBuilderTest, AppliesCustomAlpnAndAuthMode) {
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;
  config.alpnProtocols = {"h2", "rs"};
  config.clientAuth = fizz::server::ClientAuthMode::None;

  auto ctx = buildFizzServerContext(config, thriftConfig_);
  ASSERT_NE(ctx, nullptr);
  EXPECT_EQ(ctx->getSupportedAlpns(), (std::vector<std::string>{"h2", "rs"}));
  EXPECT_EQ(ctx->getClientAuthMode(), fizz::server::ClientAuthMode::None);
}

TEST_F(FizzServerContextBuilderTest, RejectsBothPathAndBufferSet) {
  FizzServerCertConfig config;
  config.certPath = "/dev/null";
  config.keyPath = "/dev/null";
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;

  EXPECT_THROW(
      buildFizzServerContext(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, RejectsNeitherPathNorBufferSet) {
  FizzServerCertConfig config;
  EXPECT_THROW(
      buildFizzServerContext(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, RejectsCertPathWithoutKeyPath) {
  FizzServerCertConfig config;
  config.certPath = "/tmp/whatever.pem";
  EXPECT_THROW(
      buildFizzServerContext(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, RejectsCertPemWithoutKeyPem) {
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  EXPECT_THROW(
      buildFizzServerContext(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, ThrowsOnUnreadableCertPath) {
  FizzServerCertConfig config;
  config.certPath = "/this/path/does/not/exist.pem";
  config.keyPath = "/this/path/does/not/exist.key";
  EXPECT_THROW(
      buildFizzServerContext(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, ThrowsOnMalformedPem) {
  FizzServerCertConfig config;
  config.certPem = "not actually a PEM";
  config.keyPem = "also not a PEM";
  EXPECT_THROW(
      buildFizzServerContext(config, thriftConfig_), std::runtime_error);
}

} // namespace apache::thrift::fast_thrift::security::test
