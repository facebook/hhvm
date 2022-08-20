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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/record/Extensions.h>
#include <fizz/record/test/ExtensionTestsBase.h>
#include <thrift/lib/cpp2/security/extensions/Types.h>

using namespace fizz::test;

namespace apache {
namespace thrift {
namespace test {

TEST_F(ExtensionsTest, TestEmptyThriftParameters) {
  NegotiationParameters params;
  std::vector<fizz::Extension> exts;
  ThriftParametersExt paramsExt;
  paramsExt.params = params;
  exts.push_back(encodeThriftExtension(paramsExt));

  auto ext = getThriftExtension(exts);

  EXPECT_TRUE(ext.has_value());
  EXPECT_EQ(ext.value().params, paramsExt.params);
}

TEST_F(ExtensionsTest, TestThriftParameters) {
  NegotiationParameters params;
  std::uint64_t compressions = 1ull << (int(CompressionAlgorithm::ZSTD) - 1) |
      1ull << (int(CompressionAlgorithm::ZLIB) - 1);
  params.compressionAlgos_ref() = compressions;
  params.useStopTLS_ref() = true;
  std::vector<fizz::Extension> exts;
  ThriftParametersExt paramsExt;
  paramsExt.params = params;
  exts.push_back(encodeThriftExtension(paramsExt));

  auto ext = getThriftExtension(exts);

  EXPECT_TRUE(ext.has_value());
  EXPECT_EQ(ext.value().params, paramsExt.params);
  EXPECT_TRUE(ext->params.compressionAlgos_ref());
  EXPECT_EQ(ext->params.compressionAlgos_ref().value(), compressions);
  EXPECT_TRUE(ext->params.useStopTLS_ref().value());
}

} // namespace test
} // namespace thrift
} // namespace apache
