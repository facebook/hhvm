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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressor.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressorFactory.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressorRegistry.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using apache::thrift::rocket::CustomCompressorFactory;
using apache::thrift::rocket::CustomCompressorRegistry;

namespace apache::thrift::rocket {

namespace {

class MockCompressor : public CustomCompressor {
 public:
  ~MockCompressor() override = default;

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      compressBuffer,
      (std::unique_ptr<folly::IOBuf>&&),
      (override));

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      uncompressBuffer,
      (std::unique_ptr<folly::IOBuf>&&),
      (override));
};

class MockCompressorFactory : public CustomCompressorFactory {
 public:
  MockCompressorFactory(
      std::string const& name, std::string const& responsePayload)
      : name_(name), responsePayload_(responsePayload) {}

  std::string getCompressorName() const override { return name_; }

  std::optional<CustomCompressionSetupResponse>
  createCustomCompressorNegotiationResponse(
      CustomCompressionSetupRequest const& /*request*/) const override {
    CustomCompressionSetupResponse r;
    r.payload() = responsePayload_;
    return r;
  }

  std::shared_ptr<CustomCompressor> make(
      CustomCompressionSetupRequest const& /*request*/,
      CustomCompressionSetupResponse const& /*response*/,
      CompressorLocation /*location*/) const override {
    return nullptr;
  }

 private:
  std::string name_;
  std::string responsePayload_;
};

const std::string kName1 = "test_1";
const std::string kName2 = "test_2";

} // anonymous namespace

class CompressionTest : public TestSetup {};

TEST_F(CompressionTest, customRegistryAddRemoveGet) {
  EXPECT_EQ(CustomCompressorRegistry::get(kName1), nullptr);
  EXPECT_EQ(CustomCompressorRegistry::get(kName2), nullptr);

  {
    const auto res = CustomCompressorRegistry::registerFactory(
        std::make_shared<MockCompressorFactory>(kName1, "payload1"));
    EXPECT_TRUE(res);

    EXPECT_NE(CustomCompressorRegistry::get(kName1), nullptr);
    EXPECT_EQ(
        *CustomCompressorRegistry::get(kName1)
             ->createCustomCompressorNegotiationResponse({})
             ->payload(),
        "payload1");
    EXPECT_EQ(CustomCompressorRegistry::get(kName2), nullptr);
  }

  {
    const auto res = CustomCompressorRegistry::registerFactory(
        std::make_shared<MockCompressorFactory>(kName2, "payload2"));
    EXPECT_FALSE(res);
  }

  {
    CustomCompressorRegistry::unregister(kName1);
    const auto res = CustomCompressorRegistry::registerFactory(
        std::make_shared<MockCompressorFactory>(kName2, "payload2"));
    EXPECT_TRUE(res);

    EXPECT_EQ(CustomCompressorRegistry::get(kName1), nullptr);
    EXPECT_NE(CustomCompressorRegistry::get(kName2), nullptr);
    EXPECT_EQ(
        *CustomCompressorRegistry::get(kName2)
             ->createCustomCompressorNegotiationResponse({})
             ->payload(),
        "payload2");
  }

  {
    CustomCompressorRegistry::unregister(kName1);
    CustomCompressorRegistry::unregister(kName2);
    EXPECT_EQ(CustomCompressorRegistry::get(kName1), nullptr);
    EXPECT_EQ(CustomCompressorRegistry::get(kName2), nullptr);
  }
}

} // namespace apache::thrift::rocket
