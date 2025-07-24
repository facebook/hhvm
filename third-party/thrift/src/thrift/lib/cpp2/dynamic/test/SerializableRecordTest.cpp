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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>

#include <limits>
#include <type_traits>

namespace apache::thrift::type_system {

static_assert(std::is_copy_constructible_v<SerializableRecord>);
static_assert(std::is_copy_assignable_v<SerializableRecord>);
static_assert(std::is_move_constructible_v<SerializableRecord>);
static_assert(std::is_move_assignable_v<SerializableRecord>);

namespace {
SerializableRecord::ByteArray makeByteArray(std::string_view str) {
  return folly::IOBuf::copyBuffer(str);
}

SerializableRecord decode(SerializableRecordUnion&& raw) {
  return SerializableRecord::fromThrift(std::move(raw));
}

SerializableRecordUnion encode(const SerializableRecord& record) {
  return SerializableRecord::toThrift(record);
}

} // namespace

TEST(SerializableRecordTest, Bool) {
  SerializableRecord r = SerializableRecord::Bool(true);
  EXPECT_TRUE(r.isBool());
  EXPECT_TRUE(r.isType<SerializableRecord::Bool>());
  EXPECT_FALSE(r.isInt8());
  EXPECT_FALSE(r.isType<SerializableRecord::Int8>());

  EXPECT_TRUE(r.asBool());
  EXPECT_EQ(r, SerializableRecord::Bool(true));
  EXPECT_NE(r, SerializableRecord::Bool(false));
  EXPECT_NE(r, SerializableRecord::Int32(0));

  EXPECT_EQ(toDebugString(r), "Bool(true)\n");

  EXPECT_EQ(encode(r).boolDatum().value(), true);
}

TEST(SerializableRecordTest, Int8) {
  SerializableRecord r = SerializableRecord::Int8(42);
  EXPECT_TRUE(r.isInt8());
  EXPECT_FALSE(r.isInt16());

  EXPECT_EQ(r.asInt8(), 42);
  EXPECT_EQ(r, SerializableRecord::Int8(42));
  EXPECT_NE(r, SerializableRecord::Int8(0));
  EXPECT_NE(r, SerializableRecord::Int16(42));

  EXPECT_EQ(toDebugString(r), "Int8(42)\n");

  EXPECT_EQ(encode(r).int8Datum().value(), 42);
}

TEST(SerializableRecordTest, Int16) {
  SerializableRecord r = SerializableRecord::Int16(42);
  EXPECT_TRUE(r.isInt16());
  EXPECT_FALSE(r.isInt32());

  EXPECT_EQ(r.asInt16(), 42);
  EXPECT_EQ(r, SerializableRecord::Int16(42));
  EXPECT_NE(r, SerializableRecord::Int16(0));
  EXPECT_NE(r, SerializableRecord::Int32(42));

  EXPECT_EQ(toDebugString(r), "Int16(42)\n");

  EXPECT_EQ(encode(r).int16Datum().value(), 42);
}

TEST(SerializableRecordTest, Int32) {
  SerializableRecord r = SerializableRecord::Int32(42);
  EXPECT_TRUE(r.isInt32());
  EXPECT_FALSE(r.isInt64());

  EXPECT_EQ(r.asInt32(), 42);
  EXPECT_EQ(r, SerializableRecord::Int32(42));
  EXPECT_NE(r, SerializableRecord::Int32(0));
  EXPECT_NE(r, SerializableRecord::Int64(42));

  EXPECT_EQ(toDebugString(r), "Int32(42)\n");

  EXPECT_EQ(encode(r).int32Datum().value(), 42);
}

TEST(SerializableRecordTest, Int64) {
  SerializableRecord r = SerializableRecord::Int64(42);
  EXPECT_TRUE(r.isInt64());
  EXPECT_FALSE(r.isFloat32());

  EXPECT_EQ(r.asInt64(), 42);
  EXPECT_EQ(r, SerializableRecord::Int64(42));
  EXPECT_NE(r, SerializableRecord::Int64(0));
  EXPECT_NE(r, SerializableRecord::Float32(42));

  EXPECT_EQ(toDebugString(r), "Int64(42)\n");

  EXPECT_EQ(encode(r).int64Datum().value(), 42);
}

TEST(SerializableRecordTest, Float32) {
  SerializableRecord r = SerializableRecord::Float32(42);
  EXPECT_TRUE(r.isFloat32());
  EXPECT_FALSE(r.isFloat64());

  EXPECT_EQ(r.asFloat32(), 42);
  EXPECT_EQ(r, SerializableRecord::Float32(42));
  EXPECT_NE(r, SerializableRecord::Float32(0));
  EXPECT_NE(r, SerializableRecord::Float64(42));

  EXPECT_EQ(toDebugString(r), "Float32(42)\n");

  EXPECT_EQ(encode(r).float32Datum().value(), 42);
}

TEST(SerializableRecordTest, Float64) {
  SerializableRecord r = SerializableRecord::Float64(42);
  EXPECT_TRUE(r.isFloat64());
  EXPECT_FALSE(r.isText());

  EXPECT_EQ(r.asFloat64(), 42);
  EXPECT_EQ(r, SerializableRecord::Float64(42));
  EXPECT_NE(r, SerializableRecord::Float64(0));
  EXPECT_NE(r, SerializableRecord::Float32(42));
  EXPECT_NE(r, SerializableRecord::Text("hello"));

  EXPECT_EQ(toDebugString(r), "Float64(42)\n");

  EXPECT_EQ(encode(r).float64Datum().value(), 42);
}

TEST(SerializableRecordTest, Text) {
  SerializableRecord r = SerializableRecord::text("hello");
  EXPECT_TRUE(r.isText());
  EXPECT_FALSE(r.isByteArray());

  EXPECT_EQ(r.asText(), "hello");
  EXPECT_EQ(r, SerializableRecord::Text("hello"));
  EXPECT_NE(r, SerializableRecord::Text("world"));
  EXPECT_NE(r, SerializableRecord::Float32(42));

  EXPECT_EQ(toDebugString(r), "Text(\"hello\")\n");

  EXPECT_EQ(encode(r).textDatum().value(), "hello");
}

TEST(SerializableRecordTest, ByteArray) {
  SerializableRecord r = makeByteArray("hello");
  EXPECT_TRUE(r.isByteArray());
  EXPECT_FALSE(r.isText());

  EXPECT_EQ(r, makeByteArray("hello"));
  EXPECT_NE(r, makeByteArray("world"));
  EXPECT_NE(r, SerializableRecord::Text("hello"));

  EXPECT_EQ(toDebugString(r), "ByteArray(\"aGVsbG8=\")\n");

  EXPECT_TRUE(folly::IOBufEqualTo{}(
      encode(r).byteArrayDatum().value(), *makeByteArray("hello")));
}

TEST(SerializableRecordTest, Kind) {
  SerializableRecord r = SerializableRecord::Int64(42);
  EXPECT_THAT(
      [&] { r.asType<SerializableRecord::Int32>(); },
      testing::ThrowsMessage<std::runtime_error>(
          testing::HasSubstr("int64Datum")));
}

TEST(SerializableRecordTest, CopyAndMove) {
  SerializableRecord original = SerializableRecord(makeByteArray("hello"));
  SerializableRecord copy = original;
  EXPECT_EQ(copy, original);

  SerializableRecord moved = std::move(original);
  EXPECT_EQ(moved, makeByteArray("hello"));
}

TEST(SerializableRecordTest, FieldSet) {
  SerializableRecord r = SerializableRecord::FieldSet({
      {FieldId(1), SerializableRecord::Int32(-17)},
      {FieldId(2),
       SerializableRecord::Map({
           {
               SerializableRecord::FieldSet({
                   {FieldId(1), SerializableRecord::Int32(42)},
               }),
               SerializableRecord::List(
                   {SerializableRecord::Int32(1),
                    SerializableRecord::Int32(2)}),
           },
       })},
      {FieldId(3),
       SerializableRecord::Map({
           {
               SerializableRecord::Bool(true),
               SerializableRecord::Set({SerializableRecord::text("hello")}),
           },
       })},
  });

  EXPECT_TRUE(r.isFieldSet());
  const SerializableRecord::FieldSet& fields = r.asFieldSet();
  EXPECT_EQ(fields.size(), 3);
  EXPECT_EQ(fields.at(FieldId(1)).asInt32(), -17);

  const SerializableRecord::Map& field2 = fields.at(FieldId(2)).asMap();
  EXPECT_EQ(field2.size(), 1);
  const SerializableRecord::List& field2Entry1 =
      field2
          .at(SerializableRecord::FieldSet({
              {FieldId(1), SerializableRecord::Int32(42)},
          }))
          .asType<SerializableRecord::List>();
  EXPECT_EQ(
      field2Entry1,
      SerializableRecord::List(
          {SerializableRecord::Int32(1), SerializableRecord::Int32(2)}));

  const SerializableRecord::Map& field3 = fields.at(FieldId(3)).asMap();
  const SerializableRecord::Set& field3Entry1 =
      field3.at(SerializableRecord::Bool(true))
          .asType<SerializableRecord::Set>();
  EXPECT_EQ(
      field3Entry1,
      SerializableRecord::Set({SerializableRecord::text("hello")}));

  EXPECT_EQ(
      toDebugString(r),
      "FieldSet(size=3)\n"
      "├─ 1 → Int32(-17)\n"
      "├─ 2 → Map(size=1)\n"
      "│  ├─ key = FieldSet(size=1)\n"
      "│  │  ╰─ 1 → Int32(42)\n"
      "│  ╰─ value = List(size=2)\n"
      "│     ├─ [0] → Int32(1)\n"
      "│     ╰─ [1] → Int32(2)\n"
      "╰─ 3 → Map(size=1)\n"
      "   ├─ key = Bool(true)\n"
      "   ╰─ value = Set(size=1)\n"
      "      ╰─ Text(\"hello\")\n");

  EXPECT_EQ(decode(encode(r)), r);
}

TEST(SerializableRecordTest, List) {
  SerializableRecord r = SerializableRecord::List(
      {SerializableRecord::Int32(1),
       SerializableRecord::Int64(2),
       SerializableRecord::text("hello")});

  EXPECT_TRUE(r.isList());
  const SerializableRecord::List& list = r.asList();
  EXPECT_EQ(list.size(), 3);
  EXPECT_EQ(list[0].asInt32(), 1);
  EXPECT_EQ(list[1].asInt64(), 2);
  EXPECT_EQ(list[2].asText(), SerializableRecord::Text("hello"));

  EXPECT_EQ(
      toDebugString(r),
      "List(size=3)\n"
      "├─ [0] → Int32(1)\n"
      "├─ [1] → Int64(2)\n"
      "╰─ [2] → Text(\"hello\")\n");

  EXPECT_EQ(decode(encode(r)), r);
}

TEST(SerializableRecordTest, Set) {
  SerializableRecord r = SerializableRecord::Set(
      {SerializableRecord::Int32(1),
       SerializableRecord::Int64(1),
       SerializableRecord::Int64(2)});

  EXPECT_TRUE(r.isSet());
  const SerializableRecord::Set& set = r.asSet();
  EXPECT_EQ(set.size(), 3);
  EXPECT_TRUE(set.contains(SerializableRecord::Int32(1)));
  EXPECT_TRUE(set.contains(SerializableRecord::Int64(1)));
  EXPECT_TRUE(set.contains(SerializableRecord::Int64(2)));

  EXPECT_EQ(decode(encode(r)), r);
}

TEST(SerializableRecordTest, Map) {
  SerializableRecord r = SerializableRecord::Map({
      {SerializableRecord::Int32(1), SerializableRecord::Text("one")},
      {SerializableRecord::Int32(2), SerializableRecord::Text("two")},
  });

  EXPECT_TRUE(r.isMap());
  const SerializableRecord::Map& map = r.asMap();
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.at(SerializableRecord::Int32(1)).asText(), "one");
  EXPECT_EQ(map.at(SerializableRecord::Int32(2)).asText(), "two");

  EXPECT_EQ(decode(encode(r)), r);
}

TEST(SerializableRecordTest, ComplexNestedRecords) {
  SerializableRecord r = SerializableRecord::FieldSet({{
      FieldId(1),
      SerializableRecord::Map({
          {
              SerializableRecord::Int32(1),
              SerializableRecord::FieldSet({
                  {
                      FieldId(2),
                      SerializableRecord::List({
                          SerializableRecord::Int64(100),
                          SerializableRecord::Map({
                              {SerializableRecord::Text("key1"),
                               SerializableRecord::Set({
                                   SerializableRecord::Float32(3.14),
                                   SerializableRecord::Float64(2.718),
                               })},
                              {SerializableRecord::Text("key2"),
                               SerializableRecord::FieldSet({
                                   {
                                       FieldId(3),
                                       makeByteArray("nested"),
                                   },
                               })},
                          }),
                      }),
                  },
              }),
          },
      }),
  }});

  EXPECT_TRUE(r.isFieldSet());
  const SerializableRecord::FieldSet& level1 = r.asFieldSet();
  EXPECT_EQ(level1.size(), 1);

  const SerializableRecord::Map& level1Map = level1.at(FieldId(1)).asMap();
  EXPECT_EQ(level1Map.size(), 1);

  const SerializableRecord::FieldSet& level2FieldSet =
      level1Map.at(SerializableRecord::Int32(1)).asFieldSet();
  EXPECT_EQ(level2FieldSet.size(), 1);

  const SerializableRecord::List& level2List =
      level2FieldSet.at(FieldId(2)).asList();
  EXPECT_EQ(level2List.size(), 2);
  EXPECT_EQ(level2List[0].asInt64(), 100);

  const SerializableRecord::Map& level2Map = level2List[1].asMap();
  EXPECT_EQ(level2Map.size(), 2);

  const SerializableRecord::Set& key1Set =
      level2Map.at(SerializableRecord::Text("key1")).asSet();
  EXPECT_EQ(key1Set.size(), 2);
  EXPECT_TRUE(key1Set.contains(SerializableRecord::Float32(3.14)));
  EXPECT_TRUE(key1Set.contains(SerializableRecord::Float64(2.718)));

  const SerializableRecord::FieldSet& key2FieldSet =
      level2Map.at(SerializableRecord::Text("key2")).asFieldSet();
  EXPECT_EQ(key2FieldSet.size(), 1);
  EXPECT_EQ(key2FieldSet.at(FieldId(3)), makeByteArray("nested"));

  EXPECT_EQ(decode(encode(r)), r);
}

TEST(SerializableRecordTest, InvalidUTF8Text) {
  EXPECT_THROW(
      { SerializableRecord r = SerializableRecord::text("\xFF\xFE\xFD"); },
      std::invalid_argument);
}

TEST(SerializableRecordTest, InvalidFloat) {
  using FloatLimits = std::numeric_limits<float>;
  using DoubleLimits = std::numeric_limits<double>;

  // NaN is invalid.
  EXPECT_THROW(
      {
        SerializableRecord r =
            SerializableRecord::Float32(FloatLimits::quiet_NaN());
      },
      std::invalid_argument);
  EXPECT_THROW(
      {
        SerializableRecord r =
            SerializableRecord::Float64(DoubleLimits::quiet_NaN());
      },
      std::invalid_argument);

  // Negative zero is invalid.
  EXPECT_THROW(
      { SerializableRecord r = SerializableRecord::Float32(-0.0f); },
      std::invalid_argument);
  EXPECT_THROW(
      { SerializableRecord r = SerializableRecord::Float64(-0.0); },
      std::invalid_argument);

  // Positive zero is valid.
  EXPECT_NO_THROW(
      { SerializableRecord r = SerializableRecord::Float32(0.0f); });
  EXPECT_NO_THROW({ SerializableRecord r = SerializableRecord::Float64(0.0); });

  // Infinity is valid.
  EXPECT_NO_THROW({
    SerializableRecord r = SerializableRecord::Float32(FloatLimits::infinity());
  });
  EXPECT_NO_THROW({
    SerializableRecord r =
        SerializableRecord::Float64(DoubleLimits::infinity());
  });

  // -Infinity is valid.
  EXPECT_NO_THROW({
    SerializableRecord r =
        SerializableRecord::Float32(-FloatLimits::infinity());
  });
  EXPECT_NO_THROW({
    SerializableRecord r =
        SerializableRecord::Float64(-DoubleLimits::infinity());
  });
}

TEST(SerializableRecordTest, InvalidDatumSerde) {
  EXPECT_NO_THROW({
    SerializableRecordUnion raw;
    raw.float64Datum() = 0.0;
    decode(std::move(raw));
  });

  EXPECT_THROW(
      {
        SerializableRecordUnion raw;
        raw.float64Datum() = -0.0;
        decode(std::move(raw));
      },
      std::invalid_argument);
  EXPECT_THROW(
      {
        SerializableRecordUnion raw;
        raw.float32Datum() = std::numeric_limits<float>::quiet_NaN();
        decode(std::move(raw));
      },
      std::invalid_argument);
  EXPECT_THROW(
      {
        SerializableRecordUnion raw;
        raw.textDatum() = "\xFF\xFE\xFD";
        decode(std::move(raw));
      },
      std::invalid_argument);
}

} // namespace apache::thrift::type_system
