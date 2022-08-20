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

#include <fizz/test/HandshakeTest.h>

#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache {
namespace thrift {
namespace test {

class ThriftTLSHandshakeTest : public fizz::test::HandshakeTest {};

TEST_F(ThriftTLSHandshakeTest, TestExtensionsThriftParameters) {
  auto context = std::make_shared<apache::thrift::ThriftParametersContext>();
  auto clientThriftParams =
      std::make_shared<apache::thrift::ThriftParametersClientExtension>(
          context);
  auto serverThriftParams =
      std::make_shared<apache::thrift::ThriftParametersServerExtension>(
          context);
  clientExtensions_ = clientThriftParams;
  serverExtensions_ = serverThriftParams;
  resetTransports();
  doHandshake();
  EXPECT_TRUE(clientThriftParams->getThriftCompressionAlgorithm().has_value());
  EXPECT_TRUE(serverThriftParams->getThriftCompressionAlgorithm().has_value());
  EXPECT_EQ(
      *clientThriftParams->getThriftCompressionAlgorithm(),
      apache::thrift::CompressionAlgorithm::ZSTD);
  EXPECT_EQ(
      *serverThriftParams->getThriftCompressionAlgorithm(),
      apache::thrift::CompressionAlgorithm::ZSTD);
}
} // namespace test
} // namespace thrift
} // namespace apache
