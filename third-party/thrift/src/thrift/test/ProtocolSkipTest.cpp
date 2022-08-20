/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderStructReadState.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/reflection/populator.h>
#include <thrift/lib/cpp2/reflection/reflection.h>
#include <thrift/test/testset/Testing.h>
#include <thrift/test/testset/gen-cpp2/testset_fatal_types.h>
#include <thrift/test/testset/gen-cpp2/testset_types_custom_protocol.h>

namespace apache::thrift::test {
namespace {

TEST(ProtocolSkipTest, SkipInt) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeI32(123);
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_I32);
}

TEST(ProtocolSkipTest, SkipStop) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeFieldStop();
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  bool thrown = false;
  try {
    reader.skip(TType::T_STOP);
  } catch (const TProtocolException& ex) {
    EXPECT_EQ(TProtocolException::INVALID_DATA, ex.getType());
    thrown = true;
  }
  EXPECT_TRUE(thrown);
}

TEST(ProtocolSkipTest, SkipStopInContainer) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeListBegin(TType::T_STOP, 1u << 30);
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  bool thrown = false;
  try {
    reader.skip(TType::T_LIST);
  } catch (const TProtocolException& ex) {
    EXPECT_EQ(TProtocolException::INVALID_DATA, ex.getType());
    thrown = true;
  }
  EXPECT_TRUE(thrown);
}

template <typename T>
class SpecializedSkipTest : public testing::Test {
 private:
  template <typename Reader>
  static auto serialized() {
    std::mt19937 rng;
    T obj;
    populator::populate(obj, {}, rng);
    using Writer = typename Reader::ProtocolWriter;
    auto buf = Serializer<Reader, Writer>::template serialize<std::string>(obj);
    return folly::IOBuf::copyBuffer(buf);
  }

 public:
  template <typename Reader>
  void test() {
    struct SlowReader : Reader {
      /* implicit */ SlowReader(const Reader& reader) : Reader(reader) {}
      static size_t fixedSizeInContainer(TType) { return 0; }
    };

    auto buf = serialized<Reader>();

    Reader prot;
    prot.setInput(buf.get());
    apache::thrift::detail::ProtocolReaderStructReadState<Reader> state;

    state.readStructBegin(&prot);
    state.readFieldBeginNoInline(&prot);
    while (!state.atStop()) {
      SlowReader protCopy = prot;
      protCopy.skip(state.fieldType);
      prot.skip(state.fieldType);
      EXPECT_EQ(prot.getCursor(), protCopy.getCursor());
      state.readFieldEnd(&prot);
      state.readFieldBeginNoInline(&prot);
    }

    state.readStructEnd(&prot);
  }
};

TYPED_TEST_CASE_P(SpecializedSkipTest);

TYPED_TEST_P(SpecializedSkipTest, Compact) {
  this->template test<CompactProtocolReader>();
}

TYPED_TEST_P(SpecializedSkipTest, Binary) {
  this->template test<BinaryProtocolReader>();
}

TYPED_TEST_P(SpecializedSkipTest, JSON) {
  this->template test<JSONProtocolReader>();
}

TYPED_TEST_P(SpecializedSkipTest, SimpleJSON) {
  this->template test<SimpleJSONProtocolReader>();
}

REGISTER_TYPED_TEST_CASE_P(
    SpecializedSkipTest, Compact, Binary, JSON, SimpleJSON);
THRIFT_INST_TESTSET_ALL(SpecializedSkipTest);

} // namespace
} // namespace apache::thrift::test
