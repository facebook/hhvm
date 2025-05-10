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

#include <limits>

#include <gtest/gtest.h>
#include <folly/json.h>

#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/thrift/gen-cpp2/dynamic_types.h>
#include <thrift/test/gen-cpp2/DynamicTestService.h>

using namespace cpp2;
using namespace std;
using namespace folly;
using namespace apache::thrift;

static dynamic kDynamics[] = {
    // NULL
    nullptr,

    // BOOL
    false,
    true,

    // INT
    numeric_limits<int64_t>::min(),
    -1,
    0,
    1,
    numeric_limits<int64_t>::max(),

    // DOUBLE
    numeric_limits<double>::lowest(),
    numeric_limits<double>::min(),
    -1.0,
    0.0,
    1.0,
    numeric_limits<double>::max(),
    numeric_limits<double>::epsilon(),

    // STRING
    "",
    std::string(10, '\0'),
    "Hello World",

    // ARRAY
    dynamic::array,
    dynamic::array(nullptr),
    dynamic::array(0),
    dynamic::array(
        nullptr, 0, false, 0.0, "", dynamic::array(), dynamic::object()),
    dynamic::array(
        1,
        true,
        3.14,
        "Goodnight Moon",
        dynamic::array(1),
        dynamic::object("a", "A")),

    // OBJECT
    dynamic::object,

    dynamic::object //
    (std::string(10, '\0'), nullptr),

    dynamic::object //
    ("a", nullptr) //
    ("b", false) //
    ("c", 0) //
    ("d", 0.0) //
    ("e", "") //
    ("f", dynamic::array) //
    ("g", dynamic::object),

    dynamic::object //
    ("b", true) //
    ("c", 1) //
    ("d", 3.14) //
    ("e", "Goodnight Moon") //
    ("f", dynamic::array(1)) //
    ("g", dynamic::object("a", "A")),
};

class TestServiceHandler
    : public apache::thrift::ServiceHandler<DynamicTestService> {
 public:
  void echo(SerializableDynamic& out, const SerializableDynamic& in) override {
    out = in;
  }
};

class RoundtripTestFixture : public ::testing::TestWithParam<dynamic> {};

////////////////////////////////////////////////////////////////////////////////

TEST_P(RoundtripTestFixture, RoundtripDynamics) {
  const SerializableDynamic expected = GetParam();
  BinaryProtocolWriter protWriter;
  size_t bufSize =
      Cpp2Ops<SerializableDynamic>::serializedSize(&protWriter, &expected);
  folly::IOBufQueue queue;
  protWriter.setOutput(&queue, bufSize);
  Cpp2Ops<SerializableDynamic>::write(&protWriter, &expected);

  auto buf = queue.move();
  BinaryProtocolReader protReader;
  protReader.setInput(buf.get());
  SerializableDynamic actual;
  Cpp2Ops<SerializableDynamic>::read(&protReader, &actual);
  EXPECT_EQ(expected, actual)
      << "Expected: " << toJson(*expected) << " Actual: " << toJson(*actual);
}

TEST_P(RoundtripTestFixture, RoundtripContainer) {
  Container expected;
  *expected.data() = GetParam();

  BinaryProtocolWriter protWriter;
  size_t bufSize = Cpp2Ops<Container>::serializedSize(&protWriter, &expected);
  folly::IOBufQueue queue;
  protWriter.setOutput(&queue, bufSize);
  Cpp2Ops<Container>::write(&protWriter, &expected);

  auto buf = queue.move();
  BinaryProtocolReader protReader;
  protReader.setInput(buf.get());
  Container actual;
  Cpp2Ops<Container>::read(&protReader, &actual);
  EXPECT_EQ(expected, actual) << "Expected: " << toJson(**expected.data())
                              << " Actual: " << toJson(**actual.data());
}

TEST_P(RoundtripTestFixture, SerializeOverHandler) {
  ScopedServerInterfaceThread runner(std::make_shared<TestServiceHandler>());
  auto client = runner.newClient<DynamicTestServiceAsyncClient>();

  const SerializableDynamic expected = GetParam();
  SerializableDynamic actual;
  client->sync_echo(actual, expected);
  EXPECT_EQ(expected, actual)
      << "Expected: " << toJson(*expected) << " Actual: " << toJson(*actual);
}

INSTANTIATE_TEST_CASE_P(
    All, RoundtripTestFixture, ::testing::ValuesIn(kDynamics));
