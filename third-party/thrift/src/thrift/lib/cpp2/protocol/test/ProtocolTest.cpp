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

#include <thrift/lib/cpp2/protocol/Protocol.h>

#include <algorithm>
#include <memory>
#include <random>
#include <gtest/gtest.h>
#include <folly/String.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;

class ProtocolTest : public testing::Test {};

static constexpr size_t kTestingProtocolMaxDepth = 4;

namespace {

struct StructuredExpectedTagTestStruct {
  using __fbthrift_cpp2_type = StructuredExpectedTagTestStruct;
  static constexpr bool __fbthrift_cpp2_is_union = false;
};

struct StructuredExpectedTagTestUnion {
  using __fbthrift_cpp2_type = StructuredExpectedTagTestUnion;
  static constexpr bool __fbthrift_cpp2_is_union = true;
};

} // namespace

TEST(TTypeTest, Format) {
  EXPECT_EQ(fmt::format("{}", T_BOOL), "BOOL");
  EXPECT_EQ(fmt::format("{}", T_I64), "I64");

  auto uint8Max = std::numeric_limits<uint8_t>::max();
  EXPECT_EQ(
      fmt::format("{}", static_cast<TType>(uint8Max)),
      fmt::format("UNKNOWN({})", uint8Max));
}

TEST_F(ProtocolTest, integral_expected_tag_width) {
  struct TestAdapter {};
  using apache::thrift::detail::pm::matches_integral_wire_tag_v;

  EXPECT_TRUE((matches_integral_wire_tag_v<type::bool_t, type::bool_t>));
  EXPECT_TRUE((matches_integral_wire_tag_v<type::byte_t, type::byte_t>));
  EXPECT_TRUE((matches_integral_wire_tag_v<type::i16_t, type::i16_t>));
  EXPECT_TRUE((matches_integral_wire_tag_v<type::i32_t, type::i32_t>));
  EXPECT_TRUE((matches_integral_wire_tag_v<type::i64_t, type::i64_t>));
  EXPECT_TRUE((matches_integral_wire_tag_v<
               type::cpp_type<std::uint16_t, type::i16_t>,
               type::i16_t>));
  EXPECT_TRUE((matches_integral_wire_tag_v<
               type::adapted<TestAdapter, type::i32_t>,
               type::i32_t>));

  EXPECT_FALSE((matches_integral_wire_tag_v<type::byte_t, type::i16_t>));
  EXPECT_FALSE((matches_integral_wire_tag_v<type::i16_t, type::i32_t>));
  EXPECT_FALSE((matches_integral_wire_tag_v<type::i32_t, type::i64_t>));
  EXPECT_FALSE((matches_integral_wire_tag_v<type::i64_t, type::i32_t>));
}

TEST_F(ProtocolTest, floating_point_expected_tag_width) {
  struct TestAdapter {};
  using apache::thrift::detail::pm::matches_floating_point_wire_tag_v;

  EXPECT_TRUE(
      (matches_floating_point_wire_tag_v<type::float_t, type::float_t>));
  EXPECT_TRUE(
      (matches_floating_point_wire_tag_v<type::double_t, type::double_t>));
  EXPECT_TRUE((matches_floating_point_wire_tag_v<
               type::cpp_type<float, type::float_t>,
               type::float_t>));
  EXPECT_TRUE((matches_floating_point_wire_tag_v<
               type::adapted<TestAdapter, type::double_t>,
               type::double_t>));

  EXPECT_FALSE(
      (matches_floating_point_wire_tag_v<type::float_t, type::double_t>));
  EXPECT_FALSE(
      (matches_floating_point_wire_tag_v<type::double_t, type::float_t>));
}

TEST_F(ProtocolTest, string_and_binary_expected_tag) {
  struct TestAdapter {};
  using apache::thrift::detail::pm::matches_wire_tag_v;

  EXPECT_TRUE((matches_wire_tag_v<type::string_t, type::string_t>));
  EXPECT_TRUE((matches_wire_tag_v<type::binary_t, type::binary_t>));
  EXPECT_TRUE((matches_wire_tag_v<
               type::cpp_type<folly::fbstring, type::string_t>,
               type::string_t>));
  EXPECT_TRUE((matches_wire_tag_v<
               type::adapted<TestAdapter, type::binary_t>,
               type::binary_t>));

  EXPECT_FALSE((matches_wire_tag_v<type::string_t, type::binary_t>));
  EXPECT_FALSE((matches_wire_tag_v<type::binary_t, type::string_t>));
}

TEST_F(ProtocolTest, container_expected_tag) {
  struct TestAdapter {};
  using apache::thrift::detail::pm::matches_wire_tag_v;

  EXPECT_TRUE((matches_wire_tag_v<type::list<type::i32_t>, type::list_c>));
  EXPECT_TRUE((matches_wire_tag_v<type::set<type::string_t>, type::set_c>));
  EXPECT_TRUE(
      (matches_wire_tag_v<type::map<type::i16_t, type::bool_t>, type::map_c>));
  EXPECT_TRUE(
      (matches_wire_tag_v<
          type::cpp_type<std::vector<std::int32_t>, type::list<type::i32_t>>,
          type::list_c>));
  EXPECT_TRUE((matches_wire_tag_v<
               type::adapted<TestAdapter, type::set<type::double_t>>,
               type::set_c>));

  EXPECT_FALSE((matches_wire_tag_v<type::list<type::i32_t>, type::set_c>));
  EXPECT_FALSE((matches_wire_tag_v<type::set<type::i32_t>, type::map_c>));
  EXPECT_FALSE((matches_wire_tag_v<
                type::map<type::i32_t, type::string_t>,
                type::list_c>));
}

TEST_F(ProtocolTest, structured_expected_tag) {
  struct TestAdapter {};
  using TestStruct = StructuredExpectedTagTestStruct;
  using TestUnion = StructuredExpectedTagTestUnion;
  using apache::thrift::detail::pm::matches_structured_wire_tag_v;

  EXPECT_TRUE(
      (matches_structured_wire_tag_v<type::struct_t<TestStruct>, TestStruct>));
  EXPECT_TRUE(
      (matches_structured_wire_tag_v<type::union_t<TestUnion>, TestUnion>));
  EXPECT_TRUE((matches_structured_wire_tag_v<
               type::cpp_type<TestStruct, type::struct_t<TestStruct>>,
               TestStruct>));
  EXPECT_TRUE((matches_structured_wire_tag_v<
               type::adapted<TestAdapter, type::union_t<TestUnion>>,
               TestUnion>));

  EXPECT_FALSE((matches_structured_wire_tag_v<type::i32_t, TestStruct>));
  EXPECT_FALSE(
      (matches_structured_wire_tag_v<type::list<type::i32_t>, TestStruct>));
  EXPECT_FALSE(
      (matches_structured_wire_tag_v<type::struct_t<TestStruct>, TestUnion>));
  EXPECT_FALSE(
      (matches_structured_wire_tag_v<type::union_t<TestUnion>, TestStruct>));
}

template <typename ProtocolWriter>
void makeNestedWriteInner(
    ProtocolWriter& writer, const size_t levels, const TType& type) {
  switch (type) {
    case TType::T_STRUCT: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeStructBegin("");
        writer.writeFieldBegin("fn", TType::T_STRUCT, 0);
      }
      writer.writeStructBegin("");
      writer.writeFieldBegin("fn", TType::T_BYTE, 0);
      writer.writeByte(7);
      writer.writeFieldEnd();
      writer.writeFieldStop();
      writer.writeStructEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeFieldEnd();
        writer.writeFieldStop();
        writer.writeStructEnd();
      }
      break;
    }
    case TType::T_LIST: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeListBegin(TType::T_LIST, 1);
      }
      writer.writeListBegin(TType::T_BYTE, 1);
      writer.writeByte(7);
      writer.writeListEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeListEnd();
      }
      break;
    }
    case TType::T_SET: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeSetBegin(TType::T_SET, 1);
      }
      writer.writeSetBegin(TType::T_BYTE, 1);
      writer.writeByte(7);
      writer.writeSetEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeSetEnd();
      }
      break;
    }
    case TType::T_MAP: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeMapBegin(TType::T_BYTE, TType::T_MAP, 1);
        writer.writeByte(7);
      }
      writer.writeMapBegin(TType::T_BYTE, TType::T_BYTE, 1);
      writer.writeByte(7);
      writer.writeByte(7);
      writer.writeMapEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeMapEnd();
      }
      break;
    }
    default:
      break;
  }
}

template <typename ProtocolWriter>
folly::IOBufQueue makeNested(
    const size_t height, const size_t levels, const TType type) {
  CHECK_GE(levels, 3);
  folly::IOBufQueue q;
  ProtocolWriter writer;
  writer.setOutput(&q);
  writer.setHeight(height);
  writer.writeStructBegin("");

  for (size_t j = 0; j < height; ++j) {
    writer.writeFieldBegin("fn", type, 0);
    makeNestedWriteInner(writer, levels - 3, type);
    writer.writeFieldEnd();
  }

  writer.writeFieldBegin("fn", type, 0);
  makeNestedWriteInner(writer, levels - 2, type);
  writer.writeFieldEnd();

  writer.writeFieldStop();
  writer.writeStructEnd();
  return q;
}

template <typename ProtocolReader>
size_t doSkip(const size_t height, const folly::IOBufQueue& input) {
  std::string inputs;
  input.appendToString(inputs);
  VLOG(1) << folly::hexlify<std::string>(inputs);
  ProtocolReader reader;
  reader.setHeight(height);
  reader.setInput(input.front());
  const auto a = reader.getCursorPosition();
  reader.skip(TType::T_STRUCT);
  const auto b = reader.getCursorPosition();
  return b - a;
}

template <typename ProtocolWriter, typename ProtocolReader>
void runSkipCheckDepth(
    folly::tag_t<ProtocolWriter, ProtocolReader>, const TType type) {
  const size_t height = kTestingProtocolMaxDepth;
  {
    const auto q = makeNested<ProtocolWriter>(height, height - 1, type);
    const auto s = doSkip<ProtocolReader>(height, q);
    EXPECT_EQ(q.front()->computeChainDataLength(), s);
  }
  {
    try {
      makeNested<ProtocolWriter>(height, height + 1, type);
      ADD_FAILURE() << "expected TProtocolException";
    } catch (const TProtocolException& e) {
      EXPECT_EQ(TProtocolException::DEPTH_LIMIT, e.getType());
    }
  }
  {
    auto q = makeNested<ProtocolWriter>(height + 1, height + 1, type);
    try {
      doSkip<ProtocolReader>(height, q);
      ADD_FAILURE() << "expected TProtocolException";
    } catch (const TProtocolException& e) {
      EXPECT_EQ(TProtocolException::DEPTH_LIMIT, e.getType());
    }
  }
}

template <
    typename ProtocolWriter,
    typename ProtocolReader,
    typename ArithmeticType>
void runBigListTest(
    folly::tag_t<ProtocolWriter, ProtocolReader>, ArithmeticType) {
  for (int randomInit = 0; randomInit <= 1; ++randomInit) {
    for (int i = 1; i < 256; ++i) {
      auto w = ProtocolWriter();
      auto q = folly::IOBufQueue();
      w.setOutput(&q);

      // Make sure we exercise the multi-IOBuf case
      const size_t intListSize = randomInit ? i : (i + 128 * 1024);
      std::vector<ArithmeticType> intList(intListSize);

      // Specify the engine and distribution.
      if (randomInit) {
        std::mt19937 mersenne_engine(1337); // Generates random integers
        if constexpr (std::is_floating_point_v<ArithmeticType>) {
          std::uniform_real_distribution<ArithmeticType> dist{};
          std::generate(intList.begin(), intList.end(), [&]() {
            return dist(mersenne_engine);
          });
        } else {
          std::uniform_int_distribution<ArithmeticType> dist{};
          std::generate(intList.begin(), intList.end(), [&]() {
            return dist(mersenne_engine);
          });
        }

      } else {
        ArithmeticType t = (intList.size() / 2) * (-1);
        std::generate_n(intList.begin(), intList.size(), [&]() { return ++t; });
      }
      using prot_method_integral =
          ::apache::thrift::detail::pm::protocol_methods<
              ::apache::thrift::type_class::list<
                  ::apache::thrift::type_class::integral>,
              ::std::vector<ArithmeticType>,
              ::apache::thrift::type::list<
                  ::apache::thrift::type::infer_tag<ArithmeticType>>>;
      using prot_method_float = ::apache::thrift::detail::pm::protocol_methods<
          ::apache::thrift::type_class::list<
              ::apache::thrift::type_class::floating_point>,
          ::std::vector<ArithmeticType>,
          ::apache::thrift::type::list<
              ::apache::thrift::type::infer_tag<ArithmeticType>>>;

      if constexpr (std::is_floating_point_v<ArithmeticType>) {
        prot_method_float::write(w, intList);
      } else {
        prot_method_integral::write(w, intList);
      }

      auto r = ProtocolReader();
      r.setInput(q.front());
      std::vector<ArithmeticType> outList;
      outList.resize(intList.size());
      if constexpr (std::is_floating_point_v<ArithmeticType>) {
        prot_method_float::read(r, outList);
      } else {
        prot_method_integral::read(r, outList);
      }
      ASSERT_EQ(intList.size(), outList.size());
      size_t len = std::min(intList.size(), outList.size());
      for (size_t j = 0; j < len; ++j) {
        ASSERT_EQ(intList[j], outList[j]);
      }
    }
  }
}

using BinaryProtocol = folly::tag_t<BinaryProtocolWriter, BinaryProtocolReader>;
using CompactProtocol =
    folly::tag_t<CompactProtocolWriter, CompactProtocolReader>;
using JSONProtocol = folly::tag_t<JSONProtocolWriter, JSONProtocolReader>;
using SimpleJSONProtocol =
    folly::tag_t<SimpleJSONProtocolWriter, SimpleJSONProtocolReader>;

TEST_F(ProtocolTest, skip_check_depth_binary) {
  runSkipCheckDepth(BinaryProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(BinaryProtocol{}, TType::T_LIST);
  runSkipCheckDepth(BinaryProtocol{}, TType::T_SET);
  runSkipCheckDepth(BinaryProtocol{}, TType::T_MAP);
}

TEST_F(ProtocolTest, skip_check_depth_compact) {
  runSkipCheckDepth(CompactProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(CompactProtocol{}, TType::T_LIST);
  runSkipCheckDepth(CompactProtocol{}, TType::T_SET);
  runSkipCheckDepth(CompactProtocol{}, TType::T_MAP);
}

TEST_F(ProtocolTest, skip_check_depth_json) {
  runSkipCheckDepth(JSONProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(JSONProtocol{}, TType::T_LIST);
  runSkipCheckDepth(JSONProtocol{}, TType::T_SET);
  runSkipCheckDepth(JSONProtocol{}, TType::T_MAP);
}

TEST_F(ProtocolTest, skip_check_depth_simple_json) {
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_LIST);
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_SET);
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_MAP);
}

TEST_F(ProtocolTest, big_arithmetic_list_binary_shard0) {
  runBigListTest(BinaryProtocol{}, int64_t{});
  runBigListTest(BinaryProtocol{}, uint64_t{});
  runBigListTest(BinaryProtocol{}, int32_t{});
  runBigListTest(BinaryProtocol{}, uint32_t{});
}

TEST_F(ProtocolTest, big_arithmetic_list_binary_shard1) {
  runBigListTest(BinaryProtocol{}, int16_t{});
  runBigListTest(BinaryProtocol{}, uint16_t{});
  runBigListTest(BinaryProtocol{}, int8_t{});
  runBigListTest(BinaryProtocol{}, uint8_t{});
  runBigListTest(BinaryProtocol{}, float{});
  runBigListTest(BinaryProtocol{}, double{});
}

TEST_F(ProtocolTest, big_arithmetic_list_compact_shard0) {
  runBigListTest(CompactProtocol{}, int64_t{});
  runBigListTest(CompactProtocol{}, uint64_t{});
  runBigListTest(CompactProtocol{}, int32_t{});
  runBigListTest(CompactProtocol{}, uint32_t{});
}

TEST_F(ProtocolTest, big_arithmetic_list_compact_shard1) {
  runBigListTest(CompactProtocol{}, int16_t{});
  runBigListTest(CompactProtocol{}, uint16_t{});
  runBigListTest(CompactProtocol{}, int8_t{});
  runBigListTest(CompactProtocol{}, uint8_t{});
  runBigListTest(CompactProtocol{}, float{});
  runBigListTest(CompactProtocol{}, double{});
}

namespace {

template <typename Writer>
std::string serializeBinaryField(folly::StringPiece payload) {
  Writer writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  writer.writeBinary(payload);
  std::string wire;
  queue.appendToString(wire);
  return wire;
}

enum class ChunkOwnership { AllUnmanaged, AllManaged, ManagedHeadOnly };

// One IOBuf per chunk, so that reads have to walk the chain. Unmanaged chunks
// wrap `data` in place, managed ones own a copy of it.
std::unique_ptr<folly::IOBuf> makeChain(
    folly::StringPiece data, size_t chunkSize, ChunkOwnership ownership) {
  std::unique_ptr<folly::IOBuf> chain;
  for (size_t offset = 0; offset < data.size(); offset += chunkSize) {
    const size_t length = std::min(chunkSize, data.size() - offset);
    const bool managed = ownership == ChunkOwnership::AllManaged ||
        (ownership == ChunkOwnership::ManagedHeadOnly && offset == 0);
    auto buf = managed ? folly::IOBuf::copyBuffer(data.data() + offset, length)
                       : folly::IOBuf::wrapBuffer(data.data() + offset, length);
    if (chain) {
      chain->appendToChain(std::move(buf));
    } else {
      chain = std::move(buf);
    }
  }
  return chain;
}

std::string toString(folly::IOBuf& buf) {
  const auto bytes = buf.coalesce();
  return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

// Lays the input out as a single buffer, rather than as a chain.
constexpr size_t kUnchunked = std::numeric_limits<size_t>::max();

template <typename Writer, typename Reader>
void runReadBinaryIOBufTest(folly::tag_t<Writer, Reader>, size_t chunkSize) {
  std::string payload(4096, '\0');
  for (size_t i = 0; i < payload.size(); ++i) {
    payload[i] = static_cast<char>(i);
  }
  const std::string wire = serializeBinaryField<Writer>(payload);
  const bool chained = chunkSize < wire.size();

  auto readBinary = [](const folly::IOBuf* input,
                       ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    Reader reader(sharing);
    reader.setInput(input);
    folly::IOBuf out;
    reader.readBinary(out);
    return out;
  };

  // Reads from a private copy of the wire bytes, then overwrites that copy: a
  // result that aliases the input shows up as changed content, rather than as
  // a use-after-free that only a sanitizer would catch.
  std::string storage;
  auto readBinaryFromPoisonedInput = [&](ChunkOwnership ownership) {
    storage = wire;
    auto chain = makeChain(storage, chunkSize, ownership);
    auto out = readBinary(chain.get());
    chain.reset();
    std::fill(storage.begin(), storage.end(), '\xff');
    return out;
  };

  {
    // An unmanaged input is copied into a single buffer owned by the result.
    auto out = readBinaryFromPoisonedInput(ChunkOwnership::AllUnmanaged);
    EXPECT_TRUE(out.isManaged());
    EXPECT_FALSE(out.isChained());
    EXPECT_EQ(payload, toString(out));
  }

  if (chained) {
    // Only the unmanaged part of the input is copied out; the managed buffer
    // it starts in is still shared, so the result is a chain of the two.
    auto out = readBinaryFromPoisonedInput(ChunkOwnership::ManagedHeadOnly);
    EXPECT_TRUE(out.isManaged());
    EXPECT_EQ(2, out.countChainElements());
    EXPECT_EQ(payload, toString(out));
  }

  {
    // A managed input is shared with the result, not copied.
    auto chain = makeChain(wire, chunkSize, ChunkOwnership::AllManaged);
    auto out = readBinary(chain.get());
    EXPECT_EQ(chained, out.isChained());
    EXPECT_TRUE(out.isShared());
    EXPECT_EQ(payload, toString(out));
  }

  {
    // SHARE_EXTERNAL_BUFFER shares an unmanaged input as well.
    auto chain = makeChain(wire, chunkSize, ChunkOwnership::AllUnmanaged);
    auto out = readBinary(chain.get(), SHARE_EXTERNAL_BUFFER);
    EXPECT_EQ(chained, out.isChained());
    EXPECT_FALSE(out.isManaged());
    EXPECT_EQ(payload, toString(out));
  }

  const std::string emptyWire = serializeBinaryField<Writer>("");

  {
    auto chain = makeChain(emptyWire, chunkSize, ChunkOwnership::AllUnmanaged);
    auto out = readBinary(chain.get());
    EXPECT_EQ("", toString(out));
  }

  {
    // An empty field at a buffer boundary: the buffer the cursor sits at is
    // exhausted, so what a clone would share is the buffer after it.
    const std::string trailing(16, 'y');
    auto chain = folly::IOBuf::copyBuffer(emptyWire.data(), emptyWire.size());
    chain->appendToChain(
        folly::IOBuf::wrapBuffer(trailing.data(), trailing.size()));
    auto out = readBinary(chain.get());
    EXPECT_EQ("", toString(out));
  }

  const folly::StringPiece truncated{wire.data(), wire.size() - 1};
  for (auto ownership :
       {ChunkOwnership::AllUnmanaged, ChunkOwnership::AllManaged}) {
    auto chain = makeChain(truncated, chunkSize, ownership);
    EXPECT_THROW(readBinary(chain.get()), std::out_of_range);
  }
}

} // namespace

TEST_F(ProtocolTest, read_binary_iobuf_binary) {
  runReadBinaryIOBufTest(BinaryProtocol{}, kUnchunked);
  runReadBinaryIOBufTest(BinaryProtocol{}, 100);
}

TEST_F(ProtocolTest, read_binary_iobuf_compact) {
  runReadBinaryIOBufTest(CompactProtocol{}, kUnchunked);
  runReadBinaryIOBufTest(CompactProtocol{}, 100);
}
