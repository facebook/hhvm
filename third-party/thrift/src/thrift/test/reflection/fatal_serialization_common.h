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

#pragma once

#include <gtest/gtest.h>
#include <folly/Traits.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

namespace apache {
namespace thrift {
namespace test {

template <typename Reader, typename Writer, bool Printable>
struct RWPair {
  using reader = Reader;
  using writer = Writer;
  using printable = std::bool_constant<Printable>;
};

using protocol_type_pairs = ::testing::Types<
    RWPair<SimpleJSONProtocolReader, SimpleJSONProtocolWriter, true>,
    RWPair<JSONProtocolReader, JSONProtocolWriter, true>,
    RWPair<BinaryProtocolReader, BinaryProtocolWriter, false>,
    RWPair<CompactProtocolReader, CompactProtocolWriter, false>>;

template <bool printable>
void print_underlying(const folly::IOBuf& buffer, int vlog_level = 5) {
  if (VLOG_IS_ON(vlog_level)) {
    folly::ByteRange range(buffer.data(), buffer.length());
    if (printable) {
      VLOG(vlog_level) << "buffer: "
                       << std::string((const char*)range.data(), range.size());
    } else {
      std::ostringstream out;
      for (size_t i = 0; i < range.size(); i++) {
        out << std::setw(2) << std::setfill('0') << std::hex
            << (int)range.data()[i] << " ";
      }
      VLOG(vlog_level) << "buffer: " << out.str();
    }
  }
}

template <typename Pair>
struct TypedTestCommon : public ::testing::Test {
  typename Pair::reader reader;
  typename Pair::writer writer;
  folly::IOBufQueue buffer;
  std::unique_ptr<folly::IOBuf> underlying;

  TypedTestCommon() { this->writer.setOutput(&this->buffer, 4096); }

  void prep_read() {
    this->underlying = this->buffer.move();
    this->reader.setInput(this->underlying.get());
  }

  void debug_buffer() { print_underlying<Pair::printable::value>(*underlying); }
};

template <typename Pair>
struct MultiProtocolTest : public TypedTestCommon<Pair> {};

template <typename Pair>
struct MultiProtocolTestConcrete : public TypedTestCommon<Pair> {
  void TestBody() override { return; }
};

template <typename Pair>
struct CompareProtocolTest : public ::testing::Test {
  MultiProtocolTestConcrete<Pair> st1, st2;

  void prep_read() {
    st1.prep_read();
    st2.prep_read();
  }

  void debug_buffer() {
    st1.debug_buffer();
    st2.debug_buffer();
  }
};

} // namespace test
} // namespace thrift
} // namespace apache
