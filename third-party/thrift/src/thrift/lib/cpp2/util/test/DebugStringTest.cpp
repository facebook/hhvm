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

#include <thrift/lib/cpp2/util/DebugString.h>

#include <folly/String.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/util/test/gen-cpp2/DebugString_types.h>

using apache::thrift::BinaryProtocolReader;
using apache::thrift::BinaryProtocolWriter;
using apache::thrift::CompactProtocolReader;
using apache::thrift::CompactProtocolWriter;
using apache::thrift::DebugStringParams;
using apache::thrift::test::MixedStruct;
using apache::thrift::test::Struct1;
using apache::thrift::test::Struct2;

template <class Writer, class Struct>
std::unique_ptr<folly::IOBuf> serialize(Struct& ms) {
  Writer writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  ms.write(&writer);
  return queue.move();
}

template <class Writer, class Reader, class Struct>
std::string serializeThenToDebugString(Struct& ms, DebugStringParams p = {}) {
  auto buf = serialize<Writer>(ms);
  Reader r;
  r.setInput(buf.get());
  return apache::thrift::toDebugString(r, p);
}

template <class Struct>
std::string serializeThenToDebugStringBinaryAndCompact(
    Struct& ms, DebugStringParams p = {}) {
  auto res1 =
      serializeThenToDebugString<CompactProtocolWriter, CompactProtocolReader>(
          ms, p);
  auto res2 =
      serializeThenToDebugString<BinaryProtocolWriter, BinaryProtocolReader>(
          ms, p);
  EXPECT_EQ(res1, res2);
  return res1;
}

template <class Writer, class Reader, class Struct>
void toDebugStringAndBackSingle(Struct& ms, DebugStringParams p = {}) {
  // Serialize the struct, convert serialized data to text format
  std::unique_ptr<folly::IOBuf> serializedFormat1 = serialize<Writer>(ms);
  Reader r;
  r.setInput(serializedFormat1.get());
  std::string debugString = apache::thrift::toDebugString(r, p);

  // Convert text format back from text to serialized format.
  Writer wr;
  folly::IOBufQueue queue;
  wr.setOutput(&queue);
  apache::thrift::fromDebugString(debugString, wr);
  std::unique_ptr<folly::IOBuf> serializedFormat2 = queue.move();
  // Compare the bits
  folly::IOBufEqualTo cmp;
  EXPECT_TRUE(cmp(serializedFormat1, serializedFormat2)) << debugString;
}

template <class Struct>
void toDebugStringAndBack(Struct& ms) {
  DebugStringParams defaultp, oneLinep;
  oneLinep.oneLine = true;
  toDebugStringAndBackSingle<CompactProtocolWriter, CompactProtocolReader>(
      ms, defaultp);
  toDebugStringAndBackSingle<CompactProtocolWriter, CompactProtocolReader>(
      ms, oneLinep);
  toDebugStringAndBackSingle<BinaryProtocolWriter, BinaryProtocolReader>(
      ms, defaultp);
  toDebugStringAndBackSingle<BinaryProtocolWriter, BinaryProtocolReader>(
      ms, oneLinep);
}

std::string stripNewLinesAndSpace(const std::string& input) {
  std::string ret;
  char last = 0;
  for (char x : input) {
    if (x == '\n') {
      x = ' ';
    }
    if (x == ' ' && last == x) {
      continue;
    }
    ret += x;
    last = x;
  }
  return ret;
}

TEST(DebugString, SimpleTypes) {
  DebugStringParams defaultp, oneLinep;
  oneLinep.oneLine = true;

  MixedStruct ms;
  // empty
  EXPECT_EQ(
      "struct {\n}", serializeThenToDebugStringBinaryAndCompact(ms, defaultp));
  EXPECT_EQ(
      "struct { }", serializeThenToDebugStringBinaryAndCompact(ms, oneLinep));
  toDebugStringAndBack(ms);

  // A couple of simple values
  ms.myI16() = 123;
  ms.myI32() = 456789;
  EXPECT_EQ(
      folly::stripLeftMargin(R"(
      struct {
        3: i16 = 123
        4: i32 = 456789
      })"),
      serializeThenToDebugStringBinaryAndCompact(ms, defaultp));
  EXPECT_EQ(
      "struct { 3: i16 = 123 4: i32 = 456789 }",
      serializeThenToDebugStringBinaryAndCompact(ms, oneLinep));
  toDebugStringAndBack(ms);

  // Add string/binary
  ms.myString() = "thrift";
  ms.myBinary() = "fb\xff";
  std::string expected = folly::stripLeftMargin(R"(
      struct {
        3: i16 = 123
        4: i32 = 456789
        8: string = "thrift"
        9: string = "fb\xff"
      })");
  EXPECT_EQ(expected, serializeThenToDebugStringBinaryAndCompact(ms, defaultp));
  EXPECT_EQ(
      stripNewLinesAndSpace(expected),
      serializeThenToDebugStringBinaryAndCompact(ms, oneLinep));
  toDebugStringAndBack(ms);

  // Set all the simple fields
  ms.myBool() = true;
  ms.myByte() = 127;
  ms.myI16() = 32767;
  ms.myI32() = 2000111222;
  ms.myI64() = 8000111222;
  ms.myDouble() = 123.1;
  ms.myFloat() = -123.1;
  ms.myString() = "hi";
  ms.myBinary() = "bin";

  expected = folly::stripLeftMargin(R"(
      struct {
        1: bool = true
        2: byte = 127
        3: i16 = 32767
        4: i32 = 2000111222
        5: i64 = 8000111222
        6: double = 123.1
        7: float = -123.1
        8: string = "hi"
        9: string = "bin"
      })");
  EXPECT_EQ(expected, serializeThenToDebugStringBinaryAndCompact(ms, defaultp));
  EXPECT_EQ(
      stripNewLinesAndSpace(expected),
      serializeThenToDebugStringBinaryAndCompact(ms, oneLinep));
  toDebugStringAndBack(ms);
}

TEST(DebugString, Containers) {
  DebugStringParams defaultp, oneLinep;
  oneLinep.oneLine = true;

  MixedStruct ms;
  ms.myMap() = {{123, "onetwothree"}, {456, "fourfivesix"}};
  ms.myList() = {"item1", "item2", "item3"};
  ms.mySet() = {}; // empty, but set.

  std::string expected = folly::stripLeftMargin(R"(
      struct {
        10: map<i32, string> = {
          123 : "onetwothree",
          456 : "fourfivesix",
        }
        11: list<string> = [
          "item1", "item2", "item3",
        ]
        12: set<i64> = {
        }
      })");
  EXPECT_EQ(expected, serializeThenToDebugStringBinaryAndCompact(ms, defaultp));
  EXPECT_EQ(
      stripNewLinesAndSpace(expected),
      serializeThenToDebugStringBinaryAndCompact(ms, oneLinep));
  toDebugStringAndBack(ms);

  // Wrap-around with list.
  MixedStruct ms2;
  ms2.myList() = {
      "item1",
      "item2",
      "item3",
      "item4",
      "item5",
      "item6",
      "item7",
      "item8",
      "item9",
      "item10",
      "item11",
      "item12"};
  expected = folly::stripLeftMargin(R"(
      struct {
        11: list<string> = [
          "item1", "item2", "item3", "item4", "item5", "item6", "item7", "item8",
          "item9", "item10", "item11", "item12",
        ]
      })");
  EXPECT_EQ(
      expected, serializeThenToDebugStringBinaryAndCompact(ms2, defaultp));
  EXPECT_EQ(
      stripNewLinesAndSpace(expected),
      serializeThenToDebugStringBinaryAndCompact(ms2, oneLinep));
  toDebugStringAndBack(ms);
}

TEST(DebugString, Structs) {
  DebugStringParams defaultp, oneLinep;
  oneLinep.oneLine = true;

  // Simple struct
  MixedStruct ms;
  Struct1 s1;
  s1.str() = "hi";
  ms.struct1() = s1;

  std::string expected = folly::stripLeftMargin(R"(
      struct {
        13: struct = {
          1: string = "hi"
        }
      })");
  EXPECT_EQ(expected, serializeThenToDebugStringBinaryAndCompact(ms, defaultp));
  EXPECT_EQ(
      stripNewLinesAndSpace(expected),
      serializeThenToDebugStringBinaryAndCompact(ms, oneLinep));
  toDebugStringAndBack(ms);

  // Nested struct
  Struct2 s2;
  std::vector<Struct1> vec;
  s1.str() = "a";
  vec.push_back(s1);
  s1.str() = "b";
  vec.push_back(s1);
  s1.str() = "c";
  vec.push_back(s1);
  s2.structs() = vec;
  ms.struct2() = s2;
  expected = folly::stripLeftMargin(R"(
      struct {
        13: struct = {
          1: string = "hi"
        }
        14: struct = {
          1: list<struct> = [
            {
              1: string = "a"
            }, {
              1: string = "b"
            },
            {
              1: string = "c"
            },
          ]
        }
      })");
  EXPECT_EQ(expected, serializeThenToDebugStringBinaryAndCompact(ms, defaultp));
  EXPECT_EQ(
      stripNewLinesAndSpace(expected),
      serializeThenToDebugStringBinaryAndCompact(ms, oneLinep));
  toDebugStringAndBack(ms);
}
