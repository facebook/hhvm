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

#include <thrift/lib/cpp2/protocol/Json5Protocol.h>

#include <limits>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_constants.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types_custom_protocol.h>

namespace apache::thrift {

using facebook::thrift::json5::Example;
using facebook::thrift::json5::TestCase;
using namespace facebook::thrift::json5::json5_test_constants;
using json5::detail::Json5ProtocolReader;
using json5::detail::Json5ProtocolWriter;
using json5::detail::kJson5Options;

namespace {

Example readExample(std::string_view json) {
  auto buf = folly::IOBuf::copyBuffer(json);
  Json5ProtocolReader reader;
  reader.setInput(buf.get());
  Example example;
  example.read(&reader);
  return example;
}

std::string writeExample(
    const Example& example, const Json5ProtocolWriter::Options& options) {
  folly::IOBufQueue queue;
  Json5ProtocolWriter writer(COPY_EXTERNAL_BUFFER, options);
  writer.setOutput(&queue);
  example.write(&writer);
  return queue.moveAsValue().toString();
}

} // namespace

// ── Struct decoding tests driven by thrift test data ─────────────────────────

class Json5CustomProtocolDecodeTest
    : public ::testing::TestWithParam<TestCase> {};

TEST_P(Json5CustomProtocolDecodeTest, DecodeJson) {
  auto out = readExample(*GetParam().json());
  EXPECT_EQ(out, *GetParam().example()) << *GetParam().json();
}

TEST_P(Json5CustomProtocolDecodeTest, DecodeJson5) {
  auto out = readExample(*GetParam().json5());
  EXPECT_EQ(out, *GetParam().example()) << *GetParam().json5();
}

INSTANTIATE_TEST_SUITE_P(
    Decode,
    Json5CustomProtocolDecodeTest,
    ::testing::ValuesIn(testCases()),
    [](const auto& info) { return *info.param.name(); });

// ── Struct encoding tests driven by thrift test data ─────────────────────────

class Json5CustomProtocolEncodeTest
    : public ::testing::TestWithParam<TestCase> {};

TEST_P(Json5CustomProtocolEncodeTest, EncodeJson) {
  auto out =
      writeExample(*GetParam().example(), {.writer = {.indentWidth = 2}});
  EXPECT_EQ(out, *GetParam().json());
}

TEST_P(Json5CustomProtocolEncodeTest, EncodeJson5) {
  auto opts = kJson5Options;
  opts.indentWidth = 2;
  auto out = writeExample(*GetParam().example(), {.writer = opts});
  EXPECT_EQ(out, *GetParam().json5());
}

INSTANTIATE_TEST_SUITE_P(
    Encode,
    Json5CustomProtocolEncodeTest,
    ::testing::ValuesIn(testCases()),
    [](const auto& info) { return *info.param.name(); });

} // namespace apache::thrift
