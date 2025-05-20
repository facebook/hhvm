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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types_custom_protocol.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;

namespace {

class JSONProtocolTest : public testing::Test {};

using W = JSONProtocolWriter;
using R = JSONProtocolReader;
using S = JSONSerializer;

template <typename T>
struct action_traits_impl;
template <typename C, typename A>
struct action_traits_impl<void (C::*)(A&) const> {
  using arg_type = A;
};
template <typename C, typename A>
struct action_traits_impl<void (C::*)(A&)> {
  using arg_type = A;
};
template <typename F>
using action_traits = action_traits_impl<decltype(&F::operator())>;
template <typename F>
using arg = typename action_traits<F>::arg_type;

string writing_cpp2(function<void(W&)> f) {
  IOBufQueue queue;
  W writer;
  writer.setOutput(&queue);
  f(writer);
  string _return;
  queue.appendToString(_return);
  return _return;
}

template <typename F>
arg<F> returning(F&& f) {
  arg<F> ret;
  f(ret);
  return ret;
}

template <typename T>
T reading_cpp2(const vector<StringPiece>& input, function<T(R&)> f) {
  IOBufQueue queue;
  for (const auto& data : input) {
    queue.wrapBuffer(data.data(), data.size());
  }
  auto buf = queue.move();
  R reader;
  reader.setInput(buf.get());
  return f(reader);
}

template <typename T>
T reading_cpp2(ByteRange input, function<T(R&)> f) {
  IOBuf buf(IOBuf::WRAP_BUFFER, input);
  R reader;
  reader.setInput(&buf);
  return f(reader);
}

template <typename T>
T reading_cpp2(StringPiece input, function<T(R&)>&& f) {
  using F = typename std::remove_reference<decltype(f)>::type;
  return reading_cpp2(ByteRange(input), std::forward<F>(f));
}

template <typename TInput>
struct ProtocolWriteTestCase {
  TInput input;
  string expected;
};

} // namespace

TEST_F(JSONProtocolTest, writeBool_false) {
  auto expected = "0";
  EXPECT_EQ(expected, writing_cpp2([](W& p) { p.writeBool(false); }));
}

TEST_F(JSONProtocolTest, writeBool_true) {
  auto expected = "1";
  EXPECT_EQ(expected, writing_cpp2([](W& p) { p.writeBool(true); }));
}

TEST_F(JSONProtocolTest, writeByte) {
  auto expected = "17";
  EXPECT_EQ(expected, writing_cpp2([](W& p) { p.writeByte(17); }));
}

TEST_F(JSONProtocolTest, writeI16) {
  auto expected = "1017";
  EXPECT_EQ(expected, writing_cpp2([](W& p) { p.writeI16(1017); }));
}

TEST_F(JSONProtocolTest, writeI32) {
  auto expected = "100017";
  EXPECT_EQ(expected, writing_cpp2([](W& p) { p.writeI32(100017); }));
}

TEST_F(JSONProtocolTest, writeI64) {
  auto expected = "5000000017";
  EXPECT_EQ(expected, writing_cpp2([](W& p) { p.writeI64(5000000017); }));
}

TEST_F(JSONProtocolTest, writeDouble) {
  const vector<ProtocolWriteTestCase<double>> kTests = {
      {5.25, "5.25"},
      {3.142, "3.142"},
      {3.1415, "3.1415"},
      {3.14159, "3.14159"},
      {3.141593, "3.141593"},
      {3.1415927, "3.1415927"},
      {3.14159265, "3.14159265"},
      {3.141592653, "3.141592653"},
      {0.0, "0"},
      {-0.0, "-0"},
      {123.0, "123"},
      {-1234.0, "-1234"},
      {1234567.0, "1234567"},
      {numeric_limits<double>::max(), "1.7976931348623157E308"},
      {nextafter(numeric_limits<double>::max(), 0.0), "1.7976931348623155E308"},
      {numeric_limits<double>::lowest(), "-1.7976931348623157E308"},
      {numeric_limits<double>::quiet_NaN(), "\"NaN\""},
      {-numeric_limits<double>::quiet_NaN(), "\"NaN\""},
      {numeric_limits<double>::infinity(), "\"Infinity\""},
      {-numeric_limits<double>::infinity(), "\"-Infinity\""},
  };

  for (const auto& test : kTests) {
    EXPECT_EQ(test.expected, writing_cpp2([&test](W& p) {
                p.writeDouble(test.input);
              }));
  }
}

TEST_F(JSONProtocolTest, writeFloat) {
  const vector<ProtocolWriteTestCase<float>> kTests = {
      {5.25f, "5.25"},
      {3.142f, "3.142"},
      {3.1415f, "3.1415"},
      {3.14159f, "3.14159"},
      {3.141593f, "3.141593"},
      {3.1415927f, "3.1415927"},
      {3.14159265f, "3.1415927"},
      {3.141592653f, "3.1415927"},
      {0.f, "0"},
      {-0.f, "-0"},
      {123.f, "123"},
      {-1234.f, "-1234"},
      {1234567.f, "1234567"},
      {numeric_limits<float>::max(), "3.4028235E38"},
      {nextafter(numeric_limits<float>::max(), 0.f), "3.4028233E38"},
      {numeric_limits<float>::lowest(), "-3.4028235E38"},
      {numeric_limits<float>::quiet_NaN(), "\"NaN\""},
      {-numeric_limits<float>::quiet_NaN(), "\"NaN\""},
      {numeric_limits<float>::infinity(), "\"Infinity\""},
      {-numeric_limits<float>::infinity(), "\"-Infinity\""},
  };

  for (const auto& test : kTests) {
    EXPECT_EQ(test.expected, writing_cpp2([&test](W& p) {
                p.writeFloat(test.input);
              }));
  }
}

TEST_F(JSONProtocolTest, writeString) {
  auto expected = R"("foobar")";
  EXPECT_EQ(
      expected, writing_cpp2([](W& p) { p.writeString(string("foobar")); }));
  EXPECT_EQ(expected, writing_cpp2([](W& p) { p.writeString("foobar"); }));
}

TEST_F(JSONProtocolTest, writeBigString) {
  string expected;
  expected.resize(2000000);
  expected[0] = '"';
  expected.back() = '"';
  for (size_t i = 1; i < expected.size() - 1; ++i) {
    expected[i] = '0' + (i % 32);
  }
  string input(expected.data() + 1, 2000000 - 2);
  EXPECT_EQ(expected, writing_cpp2([&input](W& p) { p.writeString(input); }));
}

TEST_F(JSONProtocolTest, writeBinary) {
  auto expected = R"("Zm9vYmFy")";
  EXPECT_EQ(
      expected, writing_cpp2([](W& p) { p.writeBinary(string("foobar")); }));
  EXPECT_EQ(
      expected, writing_cpp2([](W& p) {
        p.writeBinary(IOBuf::wrapBuffer(ByteRange(StringPiece("foobar"))));
      }));
  EXPECT_EQ(
      expected, writing_cpp2([](W& p) {
        p.writeBinary(*IOBuf::wrapBuffer(ByteRange(StringPiece("foobar"))));
      }));
}

TEST_F(JSONProtocolTest, writeMessage) {
  auto expected = R"([1,"foobar",1,3])";
  EXPECT_EQ(expected, writing_cpp2([](W& p) {
              p.writeMessageBegin("foobar", MessageType::T_CALL, 3);
              p.writeMessageEnd();
            }));
}

TEST_F(JSONProtocolTest, writeStruct) {
  auto expected = R"({"3":{"i64":17},"12":{"str":"some-data"}})";
  EXPECT_EQ(expected, writing_cpp2([](W& p) {
              p.writeStructBegin("foobar");
              p.writeFieldBegin("i64-field", TType::T_I64, 3);
              p.writeI64(17);
              p.writeFieldEnd();
              p.writeFieldBegin("str-field", TType::T_STRING, 12);
              p.writeString(string("some-data"));
              p.writeFieldEnd();
              p.writeFieldStop();
              p.writeStructEnd();
            }));
}

TEST_F(JSONProtocolTest, writeMap_string_i64) {
  auto expected = R"(["str","i64",3,{"foo":13,"bar":17,"baz":19}])";
  EXPECT_EQ(expected, writing_cpp2([](W& p) {
              p.writeMapBegin(TType::T_STRING, TType::T_I64, 3);
              p.writeString(string("foo"));
              p.writeI64(13);
              p.writeString(string("bar"));
              p.writeI64(17);
              p.writeString(string("baz"));
              p.writeI64(19);
              p.writeMapEnd();
            }));
}

TEST_F(JSONProtocolTest, writeList_string) {
  auto expected = R"(["str",3,"foo","bar","baz"])";
  EXPECT_EQ(expected, writing_cpp2([](W& p) {
              p.writeListBegin(TType::T_STRING, 3);
              p.writeString(string("foo"));
              p.writeString(string("bar"));
              p.writeString(string("baz"));
              p.writeListEnd();
            }));
}

TEST_F(JSONProtocolTest, writeSet_string) {
  auto expected = R"(["str",3,"foo","bar","baz"])";
  EXPECT_EQ(expected, writing_cpp2([](W& p) {
              p.writeSetBegin(TType::T_STRING, 3);
              p.writeString(string("foo"));
              p.writeString(string("bar"));
              p.writeString(string("baz"));
              p.writeSetEnd();
            }));
}

TEST_F(JSONProtocolTest, serializedSizeBool_false) {
  EXPECT_EQ(2, W().serializedSizeBool(false));
}

TEST_F(JSONProtocolTest, serializedSizeBool_true) {
  EXPECT_EQ(2, W().serializedSizeBool(true));
}

TEST_F(JSONProtocolTest, serializedSizeByte) {
  EXPECT_EQ(6, W().serializedSizeByte(17));
}

TEST_F(JSONProtocolTest, serializedSizeI16) {
  EXPECT_EQ(8, W().serializedSizeI16(1017));
}

TEST_F(JSONProtocolTest, serializedSizeI32) {
  EXPECT_EQ(13, W().serializedSizeI32(100017));
}

TEST_F(JSONProtocolTest, serializedSizeI64) {
  EXPECT_EQ(25, W().serializedSizeI64(5000000017));
}

TEST_F(JSONProtocolTest, serializedSizeDouble) {
  EXPECT_EQ(25, W().serializedSizeDouble(5.25));
}

TEST_F(JSONProtocolTest, serializedSizeFloat) {
  EXPECT_EQ(25, W().serializedSizeFloat(5.25f));
}

TEST_F(JSONProtocolTest, serializedSizeStop) {
  EXPECT_EQ(0, W().serializedSizeStop());
}

TEST_F(JSONProtocolTest, readBool_false) {
  auto input = "0";
  auto expected = false;
  EXPECT_EQ(expected, reading_cpp2<bool>(input, [](R& p) {
              return returning([&](bool& _) { p.readBool(_); });
            }));
}

TEST_F(JSONProtocolTest, readBool_true) {
  auto input = "1";
  auto expected = true;
  EXPECT_EQ(expected, reading_cpp2<bool>(input, [](R& p) {
              return returning([&](bool& _) { p.readBool(_); });
            }));
}

TEST_F(JSONProtocolTest, readByte) {
  auto input = "17";
  auto expected = int8_t(17);
  EXPECT_EQ(expected, reading_cpp2<int8_t>(input, [](R& p) {
              return returning([&](int8_t& _) { p.readByte(_); });
            }));
}

TEST_F(JSONProtocolTest, readI16) {
  auto input = "1017";
  auto expected = int16_t(1017);
  EXPECT_EQ(expected, reading_cpp2<int16_t>(input, [](R& p) {
              return returning([&](int16_t& _) { p.readI16(_); });
            }));
}

TEST_F(JSONProtocolTest, readI32) {
  auto input = "100017";
  auto expected = int32_t(100017);
  EXPECT_EQ(expected, reading_cpp2<int32_t>(input, [](R& p) {
              return returning([&](int32_t& _) { p.readI32(_); });
            }));
}

TEST_F(JSONProtocolTest, readI64) {
  auto input = "5000000017";
  auto expected = int64_t(5000000017);
  EXPECT_EQ(expected, reading_cpp2<int64_t>(input, [](R& p) {
              return returning([&](int64_t& _) { p.readI64(_); });
            }));
}

TEST_F(JSONProtocolTest, readDouble) {
  auto input = "5.25";
  auto expected = 5.25;
  EXPECT_EQ(expected, reading_cpp2<double>(input, [](R& p) {
              return returning([&](double& _) { p.readDouble(_); });
            }));
}

TEST_F(JSONProtocolTest, readDouble_malformed) {
  auto input = "\"nondouble\"";
  EXPECT_ANY_THROW(reading_cpp2<double>(input, [](R& p) {
    return returning([&](double& _) { p.readDouble(_); });
  }));
}

TEST_F(JSONProtocolTest, readDouble_empty) {
  auto input = StringPiece("5.25").subpiece(0, 0);
  EXPECT_ANY_THROW(reading_cpp2<double>(input, [](R& p) {
    return returning([&](double& _) { p.readDouble(_); });
  }));
}

TEST_F(JSONProtocolTest, readDouble_null) {
  auto input = StringPiece();
  EXPECT_ANY_THROW(reading_cpp2<double>(input, [](R& p) {
    return returning([&](double& _) { p.readDouble(_); });
  }));
}

TEST_F(JSONProtocolTest, readDouble_split) {
  vector<StringPiece> input = {" ", "\t \r \n ", "   5", ".2", "5 "};
  auto expected = 5.25;
  EXPECT_EQ(expected, reading_cpp2<double>(input, [](R& p) {
              return returning([&](double& _) { p.readDouble(_); });
            }));
}

TEST_F(JSONProtocolTest, readDouble_split_malformed) {
  vector<StringPiece> input = {" ", "\t \r \n ", "  \"nondouble\" "};
  EXPECT_ANY_THROW(reading_cpp2<double>(input, [](R& p) {
    return returning([&](double& _) { p.readDouble(_); });
  }));
}

TEST_F(JSONProtocolTest, readDouble_split_empty) {
  vector<StringPiece> input = {" ", "\t \r \n ", "   "};
  EXPECT_ANY_THROW(reading_cpp2<double>(input, [](R& p) {
    return returning([&](double& _) { p.readDouble(_); });
  }));
}

TEST_F(JSONProtocolTest, readFloat) {
  auto input = "5.25";
  auto expected = 5.25f;
  EXPECT_EQ(expected, reading_cpp2<float>(input, [](R& p) {
              return returning([&](float& _) { p.readFloat(_); });
            }));
}

TEST_F(JSONProtocolTest, readFloat_numeric_limits) {
  EXPECT_EQ(
      numeric_limits<float>::infinity(),
      reading_cpp2<float>("3.4028236E38", [](R& p) {
        return returning([&](float& _) { p.readFloat(_); });
      }));
  EXPECT_EQ(
      numeric_limits<float>::max(),
      reading_cpp2<float>("3.4028235E38", [](R& p) {
        return returning([&](float& _) { p.readFloat(_); });
      }));
  EXPECT_EQ(
      numeric_limits<float>::max(),
      reading_cpp2<float>("3.4028234E38", [](R& p) {
        return returning([&](float& _) { p.readFloat(_); });
      }));
  EXPECT_EQ(
      -numeric_limits<float>::infinity(),
      reading_cpp2<float>("-3.4028236E38", [](R& p) {
        return returning([&](float& _) { p.readFloat(_); });
      }));
  EXPECT_EQ(
      numeric_limits<float>::lowest(),
      reading_cpp2<float>("-3.4028235E38", [](R& p) {
        return returning([&](float& _) { p.readFloat(_); });
      }));
  EXPECT_EQ(
      numeric_limits<float>::lowest(),
      reading_cpp2<float>("-3.4028234E38", [](R& p) {
        return returning([&](float& _) { p.readFloat(_); });
      }));
}

TEST_F(JSONProtocolTest, readString) {
  auto input = R"("foobar")";
  auto expected = "foobar";
  EXPECT_EQ(expected, reading_cpp2<string>(input, [](R& p) {
              return returning([&](string& _) { p.readString(_); });
            }));
}

TEST_F(JSONProtocolTest, readString_raw) {
  auto input = R"("\u0019\u0002\u0000\u0000")";
  auto expected = string("\x19\x02\x00\x00", 4);
  EXPECT_EQ(expected, reading_cpp2<string>(input, [](R& p) {
              p.setAllowDecodeUTF8(false);
              return returning([&](string& _) { p.readString(_); });
            }));
  EXPECT_EQ(expected, reading_cpp2<string>(input, [](R& p) {
              return returning([&](string& _) { p.readString(_); });
            }));
}

TEST_F(JSONProtocolTest, readString_utf8) {
  auto input = R"("\u263A")";
  EXPECT_EQ("\u263A", reading_cpp2<string>(input, [](R& p) {
              return returning([&](string& _) { p.readString(_); });
            }));
}

TEST_F(JSONProtocolTest, readString_utf8_surrogate_pair) {
  auto input = R"("\uD989\uDE3A")";
  EXPECT_EQ("\U0007263A", reading_cpp2<string>(input, [](R& p) {
              return returning([&](string& _) { p.readString(_); });
            }));
}

TEST_F(JSONProtocolTest, readBinary) {
  auto input = R"("Zm9vYmFy")";
  auto expected = "foobar";
  EXPECT_EQ(expected, reading_cpp2<string>(input, [](R& p) {
              return returning([&](string& _) { p.readBinary(_); });
            }));
  EXPECT_EQ(expected, reading_cpp2<string>(input, [](R& p) {
              auto buf = IOBuf::create(0);
              p.readBinary(buf);
              return StringPiece(buf->coalesce()).str();
            }));
  EXPECT_EQ(expected, reading_cpp2<string>(input, [](R& p) {
              IOBuf buf(IOBuf::CREATE, 0);
              p.readBinary(buf);
              return StringPiece(buf.coalesce()).str();
            }));
}

TEST_F(JSONProtocolTest, readMap_string_i64) {
  using concrete_map_t = map<string, int64_t>;
  auto input = R"(["str","i64",3,{"foo":13,"bar":17,"baz":19}])";
  auto expected = concrete_map_t{{"foo", 13}, {"bar", 17}, {"baz", 19}};
  EXPECT_EQ(expected, reading_cpp2<concrete_map_t>(input, [](R& p) {
              return returning([&](concrete_map_t& _) {
                TType keyType = TType::T_STOP;
                TType valType = TType::T_STOP;
                uint32_t size = 0;
                p.readMapBegin(keyType, valType, size);
                EXPECT_EQ(TType::T_STRING, keyType);
                EXPECT_EQ(TType::T_I64, valType);
                EXPECT_EQ(3, size);
                while (size--) {
                  string key;
                  int64_t val;
                  p.readString(key);
                  p.readI64(val);
                  _[key] = val;
                }
                p.readMapEnd();
              });
            }));
}

TEST_F(JSONProtocolTest, readList_string) {
  using concrete_list_t = vector<string>;
  auto input = R"(["str",3,"foo","bar","baz"])";
  auto expected = concrete_list_t{"foo", "bar", "baz"};
  EXPECT_EQ(expected, reading_cpp2<concrete_list_t>(input, [](R& p) {
              return returning([&](concrete_list_t& _) {
                TType elemType = TType::T_STOP;
                uint32_t size = 0;
                p.readListBegin(elemType, size);
                EXPECT_EQ(TType::T_STRING, elemType);
                EXPECT_EQ(3, size);
                while (size--) {
                  string elem;
                  p.readString(elem);
                  _.push_back(elem);
                }
                p.readListEnd();
              });
            }));
}

TEST_F(JSONProtocolTest, readSet_string) {
  using concrete_set_t = set<string>;
  auto input = R"(["str",3,"foo","bar","baz"])";
  auto expected = concrete_set_t{"foo", "bar", "baz"};
  EXPECT_EQ(expected, reading_cpp2<concrete_set_t>(input, [](R& p) {
              return returning([&](concrete_set_t& _) {
                TType elemType = TType::T_STOP;
                uint32_t size = 0;
                p.readListBegin(elemType, size);
                EXPECT_EQ(TType::T_STRING, elemType);
                EXPECT_EQ(3, size);
                while (size--) {
                  string elem;
                  p.readString(elem);
                  _.insert(elem);
                }
                p.readListEnd();
              });
            }));
}

template <typename P, typename F>
static void readStruct(P& p, F f) {
  string name_;
  p.readStructBegin(name_);
  EXPECT_EQ("", name_);
  f();
  p.readStructEnd();
}

template <typename P, typename F>
static void readField(P& p, TType fieldType, int16_t fieldId, F f) {
  string name_;
  TType fieldType_ = TType::T_STOP;
  int16_t fieldId_ = numeric_limits<int16_t>::max();
  p.readFieldBegin(name_, fieldType_, fieldId_);
  EXPECT_EQ("", name_);
  EXPECT_EQ(fieldType, fieldType_);
  EXPECT_EQ(fieldId, fieldId_);
  f();
  p.readFieldEnd();
}

TEST_F(JSONProtocolTest, readStruct) {
  using concrete_struct_t = tuple<int64_t, string>;
  auto input = R"({"3":{"i64":17},"12":{"str":"some-data"}})";
  auto expected = concrete_struct_t{17, "some-data"};
  EXPECT_EQ(expected, reading_cpp2<concrete_struct_t>(input, [](R& p) {
              return returning([&](concrete_struct_t& _) {
                readStruct(p, [&] {
                  readField(p, TType::T_I64, 3, [&] { p.readI64(get<0>(_)); });
                  readField(
                      p, TType::T_STRING, 12, [&] { p.readString(get<1>(_)); });
                });
              });
            }));
}

TEST_F(JSONProtocolTest, readMessage) {
  auto input = R"([1,"foobar",1,3])";
  struct Unit {
    bool operator==(Unit) const { return true; }
  };
  auto expected = Unit{};
  EXPECT_EQ(expected, reading_cpp2<Unit>(input, [](R& p) {
              string name;
              MessageType messageType = MessageType(-1);
              int32_t seqId = -1;
              p.readMessageBegin(name, messageType, seqId);
              EXPECT_EQ("foobar", name);
              EXPECT_EQ(MessageType::T_CALL, messageType);
              EXPECT_EQ(3, seqId);
              p.readMessageEnd();
              return Unit{};
            }));
}

TEST_F(JSONProtocolTest, roundtrip) {
  using type = OneOfEach;
  const auto orig = type{};
  const auto serialized = S::serialize<string>(orig);
  type deserialized;
  const auto size = S::deserialize(serialized, deserialized);
  EXPECT_EQ(serialized.size(), size);
  EXPECT_EQ(orig, deserialized);
}
