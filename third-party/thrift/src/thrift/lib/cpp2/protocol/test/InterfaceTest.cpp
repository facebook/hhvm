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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactV1Protocol.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/VirtualProtocol.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;

namespace {

constexpr auto strvalue = StringPiece("hello world");
const IOBuf iobuf(IOBuf::WRAP_BUFFER, strvalue);

using UniquePtrIOBuf = unique_ptr<IOBuf>;
using ConstIOBufPtr = const IOBuf*;

template <typename R>
struct maker {
  static R make();
};

template <>
struct maker<StringPiece> {
  static StringPiece make() { return strvalue; }
};

template <>
struct maker<ByteRange> {
  static ByteRange make() { return ByteRange(strvalue); }
};

template <>
struct maker<IOBuf> {
  static IOBuf make() { return IOBuf(IOBuf::COPY_BUFFER, strvalue); }
};

template <>
struct maker<UniquePtrIOBuf> {
  static UniquePtrIOBuf make() { return IOBuf::copyBuffer(strvalue); }
};

template <>
struct maker<ConstIOBufPtr> {
  static ConstIOBufPtr make() { return &iobuf; }
};

template <>
struct maker<Cursor> {
  static Cursor make() { return Cursor(&iobuf); }
};

using Writers = testing::Types<
    BinaryProtocolWriter,
    CompactProtocolWriter,
    CompactV1ProtocolWriter,
    DebugProtocolWriter,
    JSONProtocolWriter,
    SimpleJSONProtocolWriter>;

using Readers = testing::Types<
    BinaryProtocolReader,
    CompactProtocolReader,
    CompactV1ProtocolReader,
    JSONProtocolReader,
    SimpleJSONProtocolReader,
    VirtualReader<BinaryProtocolReader>,
    VirtualReader<CompactProtocolReader>,
    VirtualReader<JSONProtocolReader>,
    VirtualReader<SimpleJSONProtocolReader>>;

template <typename Writer>
class WriterInterfaceTest : public testing::Test {
 protected:
  IOBufQueue queue;
  Writer writer;
  void SetUp() override { writer.setOutput(&queue); }
};

template <typename Reader>
class ReaderInterfaceTest : public testing::Test {
 protected:
  IOBufQueue queue;
  typename Reader::ProtocolWriter writer;
  void SetUp() override { writer.setOutput(&queue); }

  Reader reader() {
    Reader reader;
    reader.setInput(queue.front());
    return reader;
  }
};

} // namespace

TYPED_TEST_CASE_P(WriterInterfaceTest);

template <typename Reader>
struct WriterOfReader {
  using type = typename Reader::ProtocolWriter;
};
template <>
struct WriterOfReader<void> {
  using type = void;
};

TYPED_TEST_P(WriterInterfaceTest, reader_writer_cycle) {
  using W = decltype(this->writer);
  using W2 = typename WriterOfReader<typename W::ProtocolReader>::type;
  EXPECT_TRUE((std::is_same<W, W2>::value || std::is_same<void, W2>::value));
}

#define WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(method, argtype)      \
  TYPED_TEST_P(WriterInterfaceTest, method_sig_##method##_##argtype) { \
    this->writer.method(maker<argtype>::make());                       \
  }

WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(writeString, StringPiece)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(writeBinary, StringPiece)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(writeBinary, ByteRange)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(writeBinary, UniquePtrIOBuf)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(writeBinary, IOBuf)

WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeString, StringPiece)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeBinary, StringPiece)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeBinary, ByteRange)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeBinary, UniquePtrIOBuf)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeBinary, IOBuf)

WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeZCBinary, StringPiece)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeZCBinary, ByteRange)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeZCBinary, UniquePtrIOBuf)
WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P(serializedSizeZCBinary, IOBuf)

#undef WRITER_INTERFACE_METHOD_SIG_TYPED_TEST_P

REGISTER_TYPED_TEST_CASE_P(
    WriterInterfaceTest,
    reader_writer_cycle,
    method_sig_writeString_StringPiece,
    method_sig_writeBinary_StringPiece,
    method_sig_writeBinary_ByteRange,
    method_sig_writeBinary_UniquePtrIOBuf,
    method_sig_writeBinary_IOBuf,
    method_sig_serializedSizeString_StringPiece,
    method_sig_serializedSizeBinary_StringPiece,
    method_sig_serializedSizeBinary_ByteRange,
    method_sig_serializedSizeBinary_UniquePtrIOBuf,
    method_sig_serializedSizeBinary_IOBuf,
    method_sig_serializedSizeZCBinary_StringPiece,
    method_sig_serializedSizeZCBinary_ByteRange,
    method_sig_serializedSizeZCBinary_UniquePtrIOBuf,
    method_sig_serializedSizeZCBinary_IOBuf);

INSTANTIATE_TYPED_TEST_CASE_P(Thrift, WriterInterfaceTest, Writers);

TYPED_TEST_CASE_P(ReaderInterfaceTest);

TYPED_TEST_P(ReaderInterfaceTest, readString_fbstring) {
  this->writer.writeString("hello");
  folly::fbstring out;
  this->reader().readString(out);
  EXPECT_EQ("hello", out);
}

#define READER_INTERFACE_METHOD_SIG_TYPED_TEST_P(method, argtype)      \
  TYPED_TEST_P(ReaderInterfaceTest, method_sig_##method##_##argtype) { \
    this->reader().method(maker<argtype>::make());                     \
  }

READER_INTERFACE_METHOD_SIG_TYPED_TEST_P(setInput, ConstIOBufPtr)
READER_INTERFACE_METHOD_SIG_TYPED_TEST_P(setInput, Cursor)

#undef READER_INTERFACE_METHOD_SIG_TYPED_TEST_P

REGISTER_TYPED_TEST_CASE_P(
    ReaderInterfaceTest,
    readString_fbstring,
    method_sig_setInput_ConstIOBufPtr,
    method_sig_setInput_Cursor);

INSTANTIATE_TYPED_TEST_CASE_P(Thrift, ReaderInterfaceTest, Readers);
