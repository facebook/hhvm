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

#include <thrift/lib/cpp2/protocol/detail/JsonWriter.h>

#include <limits>
#include <gtest/gtest.h>
#include <folly/io/IOBufQueue.h>

namespace apache::thrift::json5::detail {
namespace {

enum class JsonFormat { Json, Json5 };

class JsonWriterTest : public ::testing::TestWithParam<JsonFormat> {
 protected:
  bool isJson5() const { return GetParam() == JsonFormat::Json5; }

  JsonWriterOptions options() const {
    auto opts = isJson5() ? kJson5Options : JsonWriterOptions{};
    opts.indentWidth = 2;
    return opts;
  }

  JsonWriterOptions compactOptions() const {
    auto opts = options();
    opts.indentWidth = 0;
    return opts;
  }
};

INSTANTIATE_TEST_SUITE_P(
    /*InstantiationName=*/,
    /*TestSuiteName=*/JsonWriterTest,
    /*param_generator=*/::testing::Values(JsonFormat::Json, JsonFormat::Json5),
    /*name_generator=*/[](const ::testing::TestParamInfo<JsonFormat>& info) {
      return info.param == JsonFormat::Json5 ? "Json5" : "Json";
    });

TEST_P(JsonWriterTest, Bool) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeBool(true);
  EXPECT_EQ(queue.move()->toString(), "true");
}

TEST_P(JsonWriterTest, StringWithEscapes) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeString("Say \"\nHello\"!");
  EXPECT_EQ(queue.move()->toString(), "\"Say \\\"\\nHello\\\"!\"");
}

TEST_P(JsonWriterTest, EmptyContainers) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  w.writeObjectName("empty");
  w.writeString("");
  w.writeObjectName("emptyObj");
  w.writeObjectBegin();
  w.writeObjectEnd();
  w.writeObjectName("emptyList");
  w.writeListBegin();
  w.writeListEnd();
  w.writeObjectEnd();
  if (isJson5()) {
    EXPECT_EQ(
        queue.move()->toString(),
        R"({
  empty: "",
  emptyObj: {},
  emptyList: [],
})");
  } else {
    EXPECT_EQ(queue.move()->toString(), R"({
  "empty": "",
  "emptyObj": {},
  "emptyList": []
})");
  }
}

TEST_P(JsonWriterTest, PrimitiveTypes) {
  folly::IOBufQueue queue;
  std::size_t size = 0;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  size += w.writeObjectBegin();
  size += w.writeObjectName("bool");
  size += w.writeBool(false);
  size += w.writeObjectName("integer");
  size += w.writeI64(42);
  size += w.writeObjectName("$float");
  size += w.writeDouble(3.14);
  size += w.writeObjectEnd();
  auto output = queue.move()->toString();
  EXPECT_EQ(size, output.size());
  if (isJson5()) {
    EXPECT_EQ(output, R"({
  bool: false,
  integer: 42,
  $float: 3.14,
})");
  } else {
    EXPECT_EQ(output, R"({
  "bool": false,
  "integer": 42,
  "$float": 3.14
})");
  }
}

TEST_P(JsonWriterTest, MoreFloats) {
  folly::IOBufQueue queue;
  std::size_t size = 0;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  size += w.writeObjectBegin();
  size += w.writeObjectName("-Zero");
  size += w.writeDouble(-0.0);
  size += w.writeObjectName("0.1");
  size += w.writeFloat(0.100000001490116119384765625F);
  size += w.writeObjectName("0.10000001");
  size += w.writeFloat(0.10000000894069671630859375F);
  size += w.writeObjectEnd();
  auto output = queue.move()->toString();
  EXPECT_EQ(size, output.size());
  if (isJson5()) {
    EXPECT_EQ(
        output,
        R"({
  "-Zero": -0.0,
  "0.1": 0.1,
  "0.10000001": 0.10000001,
})");
  } else {
    EXPECT_EQ(
        output,
        R"({
  "-Zero": -0.0,
  "0.1": 0.1,
  "0.10000001": 0.10000001
})");
  }
}

TEST_P(JsonWriterTest, Containers) {
  folly::IOBufQueue queue;
  std::size_t size = 0;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  size += w.writeObjectBegin();
  size += w.writeObjectName("list");
  size += w.writeListBegin();
  size += w.writeI32(1);
  size += w.writeI32(3);
  size += w.writeI32(2);
  size += w.writeListEnd();
  size += w.writeObjectName("object");
  size += w.writeObjectBegin();
  size += w.writeObjectName("1");
  size += w.writeI32(3);
  size += w.writeObjectName("2");
  size += w.writeI32(4);
  size += w.writeObjectEnd();
  size += w.writeObjectEnd();
  auto output = queue.move()->toString();
  EXPECT_EQ(output.size(), size);
  if (isJson5()) {
    EXPECT_EQ(output, R"({
  list: [
    1,
    3,
    2,
  ],
  object: {
    "1": 3,
    "2": 4,
  },
})");
  } else {
    EXPECT_EQ(output, R"({
  "list": [
    1,
    3,
    2
  ],
  "object": {
    "1": 3,
    "2": 4
  }
})");
  }
}

TEST_P(JsonWriterTest, CompactList) {
  folly::IOBufQueue queue;
  JsonWriter w(compactOptions());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeListBegin();
  w.writeI32(1);
  w.writeI32(2);
  w.writeListEnd();
  if (isJson5()) {
    EXPECT_EQ(queue.move()->toString(), "[1,2,]");
  } else {
    EXPECT_EQ(queue.move()->toString(), "[1,2]");
  }
}

TEST_P(JsonWriterTest, CompactObject) {
  folly::IOBufQueue queue;
  JsonWriter w(compactOptions());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  w.writeObjectName("name");
  w.writeString("Alice");
  w.writeObjectEnd();
  if (isJson5()) {
    EXPECT_EQ(queue.move()->toString(), "{name:\"Alice\",}");
  } else {
    EXPECT_EQ(queue.move()->toString(), R"({"name":"Alice"})");
  }
}

TEST_P(JsonWriterTest, ListBeginThrowsAsObjectKey) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  EXPECT_THROW(w.writeListBegin(), std::runtime_error);
}

TEST_P(JsonWriterTest, ObjectBeginThrowsAsObjectKey) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  EXPECT_THROW(w.writeObjectBegin(), std::runtime_error);
}

TEST_P(JsonWriterTest, CloseWrongContainerType) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeListBegin();
  EXPECT_THROW(w.writeObjectEnd(), std::runtime_error);
}

TEST_P(JsonWriterTest, CloseObjectAsListThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  w.writeObjectName("k");
  w.writeI32(1);
  EXPECT_THROW(w.writeListEnd(), std::runtime_error);
}

TEST_P(JsonWriterTest, CloseListWhenNoneOpen) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  EXPECT_THROW(w.writeListEnd(), std::runtime_error);
}

TEST_P(JsonWriterTest, CloseObjectWhenNoneOpen) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  EXPECT_THROW(w.writeObjectEnd(), std::runtime_error);
}

TEST_P(JsonWriterTest, WriteObjectNameInList) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeListBegin();
  EXPECT_THROW(w.writeObjectName("k"), std::runtime_error);
}

TEST_P(JsonWriterTest, InvalidUtf8StringThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  EXPECT_THROW(w.writeString("\x80\x81\x82"), std::runtime_error);
}

TEST_P(JsonWriterTest, IntegralAsObjectKeyThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  EXPECT_THROW(w.writeI32(1), std::runtime_error);
}

TEST_P(JsonWriterTest, BoolAsObjectKeyThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  EXPECT_THROW(w.writeBool(true), std::runtime_error);
}

TEST_P(JsonWriterTest, FloatAsObjectKeyThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  EXPECT_THROW(w.writeFloat(1.0F), std::runtime_error);
}

TEST_P(JsonWriterTest, DoubleAsObjectKeyThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  EXPECT_THROW(w.writeDouble(1.0), std::runtime_error);
}

TEST_P(JsonWriterTest, WriteStringAsObjectKeyThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  EXPECT_THROW(w.writeString("key"), std::runtime_error);
}

TEST_P(JsonWriterTest, ConsecutiveObjectNameThrows) {
  folly::IOBufQueue queue;
  JsonWriter w(options());
  w.setOutput(folly::io::QueueAppender(&queue, 4096));
  w.writeObjectBegin();
  w.writeObjectName("a");
  EXPECT_THROW(w.writeObjectName("b"), std::runtime_error);
}

TEST_P(JsonWriterTest, NanInf) {
  if (isJson5()) {
    folly::IOBufQueue queue;
    JsonWriter w(options());
    w.setOutput(folly::io::QueueAppender(&queue, 4096));
    w.writeListBegin();
    w.writeFloat(std::numeric_limits<float>::quiet_NaN());
    w.writeFloat(std::numeric_limits<float>::infinity());
    w.writeFloat(-std::numeric_limits<float>::infinity());
    w.writeDouble(std::numeric_limits<double>::quiet_NaN());
    w.writeDouble(std::numeric_limits<double>::infinity());
    w.writeDouble(-std::numeric_limits<double>::infinity());
    w.writeListEnd();
    EXPECT_EQ(
        queue.move()->toString(),
        R"([
  NaN,
  Infinity,
  -Infinity,
  NaN,
  Infinity,
  -Infinity,
])");
  } else {
    folly::IOBufQueue queue;
    JsonWriter w(options());
    w.setOutput(folly::io::QueueAppender(&queue, 4096));
    w.writeListBegin();
    EXPECT_THROW(
        w.writeFloat(std::numeric_limits<float>::quiet_NaN()),
        std::invalid_argument);
    EXPECT_THROW(
        w.writeFloat(std::numeric_limits<float>::infinity()),
        std::invalid_argument);
    EXPECT_THROW(
        w.writeFloat(-std::numeric_limits<float>::infinity()),
        std::invalid_argument);
    EXPECT_THROW(
        w.writeDouble(std::numeric_limits<double>::quiet_NaN()),
        std::invalid_argument);
    EXPECT_THROW(
        w.writeDouble(std::numeric_limits<double>::infinity()),
        std::invalid_argument);
    EXPECT_THROW(
        w.writeDouble(-std::numeric_limits<double>::infinity()),
        std::invalid_argument);
    w.writeListEnd();
  }
}

TEST_P(JsonWriterTest, NoSetOutput) {
  JsonWriter w(options());
  EXPECT_THROW(w.writeObjectBegin(), std::exception);
}

} // namespace
} // namespace apache::thrift::json5::detail
