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

#include <thrift/lib/cpp2/protocol/detail/JsonReader.h>

#include <cmath>
#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>

namespace apache::thrift::json5::detail {
namespace {

using enum Json5Reader::FloatingPointPrecision;

class Json5ReaderTest : public ::testing::Test {
 protected:
  std::unique_ptr<folly::IOBuf> buf_;

  Json5Reader reader(std::string_view input) {
    buf_ = folly::IOBuf::copyBuffer(input);
    Json5Reader r;
    r.setCursor(folly::io::Cursor(buf_.get()));
    return r;
  }
};

TEST_F(Json5ReaderTest, Bool) {
  auto r = reader("true");
  EXPECT_TRUE(std::get<bool>(r.readPrimitive(Double)));
}

TEST_F(Json5ReaderTest, StringWithEscapes) {
  auto r = reader(R"("line1\n\"line2\"\
line3")");
  EXPECT_EQ(
      std::get<std::string>(r.readPrimitive(Double)),
      "line1\n\"line2\"\nline3");
}

TEST_F(Json5ReaderTest, EmptyContainers) {
  auto verify = [this](std::string_view input) {
    auto r = reader(input);
    r.readObjectBegin();

    EXPECT_EQ(r.readObjectName(), "empty");
    EXPECT_EQ(std::get<std::string>(r.readPrimitive(Double)), "");

    EXPECT_EQ(r.readObjectName(), "emptyObj");
    r.readObjectBegin();
    r.readObjectEnd();

    EXPECT_EQ(r.readObjectName(), "emptyList");
    r.readListBegin();
    r.readListEnd();

    r.readObjectEnd();
  };

  // Standard JSON
  verify(R"({
  "empty": "",
  "emptyObj": {},
  "emptyList": []
})");

  // JSON5: unquoted keys and trailing comma
  verify(R"({
  empty: "",
  emptyObj: {},
  emptyList: [],
})");
}

TEST_F(Json5ReaderTest, PrimitiveTypes) {
  auto verify = [this](std::string_view input) {
    auto r = reader(input);
    r.readObjectBegin();

    EXPECT_EQ(r.readObjectName(), "bool");
    EXPECT_FALSE(std::get<bool>(r.readPrimitive(Double)));

    EXPECT_EQ(r.readObjectName(), "integer");
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 42);

    EXPECT_EQ(r.readObjectName(), "$float");
    EXPECT_DOUBLE_EQ(std::get<double>(r.readPrimitive(Double)), 3.14);

    r.readObjectEnd();
  };

  // Standard JSON
  verify(R"({
  "bool": false,
  "integer": 42,
  "$float": 3.14
})");

  // JSON5: unquoted keys and trailing comma
  verify(R"({
  bool: false,
  integer: 42,
  $float: 3.14,
})");
}

TEST_F(Json5ReaderTest, MoreFloats) {
  auto verify = [this](std::string_view input) {
    auto r = reader(input);
    r.readObjectBegin();

    EXPECT_EQ(r.readObjectName(), "-Zero");
    auto negZero = std::get<float>(r.readPrimitive(Single));
    EXPECT_EQ(negZero, 0.0F);
    EXPECT_TRUE(std::signbit(negZero));

    EXPECT_EQ(r.readObjectName(), "0.1");
    EXPECT_EQ(
        std::get<float>(r.readPrimitive(Single)),
        0.100000001490116119384765625F);

    EXPECT_EQ(r.readObjectName(), "0.10000001");
    EXPECT_EQ(
        std::get<float>(r.readPrimitive(Single)),
        0.10000000894069671630859375F);

    r.readObjectEnd();
  };

  // Standard JSON (keys must be quoted since they contain special chars)
  verify(R"({
  "-Zero": -0.0,
  "0.1": 0.1,
  "0.10000001": 0.10000001
})");

  // JSON5: trailing comma
  verify(R"({
  "-Zero": -0.0,
  "0.1": 0.1,
  "0.10000001": 0.10000001,
})");
}

TEST_F(Json5ReaderTest, Containers) {
  auto verify = [this](std::string_view input) {
    auto r = reader(input);
    r.readObjectBegin();

    EXPECT_EQ(r.readObjectName(), "list");
    r.readListBegin();
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 1);
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 3);
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 2);
    r.readListEnd();

    EXPECT_EQ(r.readObjectName(), "object");
    r.readObjectBegin();
    EXPECT_EQ(r.readObjectName(), "1");
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 3);
    EXPECT_EQ(r.readObjectName(), "2");
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 4);
    r.readObjectEnd();

    r.readObjectEnd();
  };

  // Standard JSON
  verify(R"({
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

  // JSON5: unquoted keys and trailing commas
  verify(R"({
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
}

TEST_F(Json5ReaderTest, CompactList) {
  auto verify = [this](std::string_view input) {
    auto r = reader(input);
    r.readListBegin();
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 1);
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 2);
    r.readListEnd();
  };

  // Standard JSON
  verify("[1,2]");

  // JSON5: trailing comma
  verify("[1,2,]");
}

TEST_F(Json5ReaderTest, CompactObject) {
  auto verify = [this](std::string_view input) {
    auto r = reader(input);
    r.readObjectBegin();
    EXPECT_EQ(r.readObjectName(), "name");
    EXPECT_EQ(std::get<std::string>(r.readPrimitive(Double)), "Alice");
    r.readObjectEnd();
  };

  // Standard JSON
  verify(R"({"name":"Alice"})");

  // JSON5: unquoted key and trailing comma
  verify(R"({name:"Alice",})");
}

TEST_F(Json5ReaderTest, NanInf) {
  auto verify = [&]<class FloatType>(auto fp, FloatType) {
    auto r = reader("[NaN, +NaN, -NaN, Infinity, +Infinity, -Infinity]");
    r.readListBegin();

    auto positiveNan = std::get<FloatType>(r.readPrimitive(fp));
    EXPECT_TRUE(std::isnan(positiveNan));
    EXPECT_FALSE(std::signbit(positiveNan));

    auto positiveNan2 = std::get<FloatType>(r.readPrimitive(fp));
    EXPECT_TRUE(std::isnan(positiveNan2));
    EXPECT_FALSE(std::signbit(positiveNan2));

    auto negativeNan = std::get<FloatType>(r.readPrimitive(fp));
    EXPECT_TRUE(std::isnan(negativeNan));
    EXPECT_TRUE(std::signbit(negativeNan));

    EXPECT_EQ(
        std::get<FloatType>(r.readPrimitive(fp)),
        std::numeric_limits<FloatType>::infinity());

    EXPECT_EQ(
        std::get<FloatType>(r.readPrimitive(fp)),
        std::numeric_limits<FloatType>::infinity());

    EXPECT_EQ(
        std::get<FloatType>(r.readPrimitive(fp)),
        -std::numeric_limits<FloatType>::infinity());

    r.readListEnd();
  };
  verify(Single, float{});
  verify(Double, double{});
}

TEST_F(Json5ReaderTest, HexNumbers) {
  auto readInt = [this](std::string_view input) {
    return std::get<std::int64_t>(reader(input).readPrimitive(Double));
  };

  EXPECT_EQ(readInt("0x0"), 0x0);
  EXPECT_EQ(readInt("0x00"), 0x00);
  EXPECT_EQ(readInt("0xFF"), 0xFF);
  EXPECT_EQ(readInt("0xdead"), 0xDEAD);
  EXPECT_EQ(readInt("0XDEAD"), 0xDEAD);
  EXPECT_EQ(readInt("0xDeAd"), 0xDEAD);
  EXPECT_EQ(readInt("+0xDEAD"), 0xDEAD);
  EXPECT_EQ(readInt("-0xDEAD"), -0xDEAD);

  EXPECT_THROW(readInt("0x"), std::exception);
  EXPECT_THROW(readInt("x0"), std::exception);
  EXPECT_THROW(readInt("00x0"), std::exception);
  EXPECT_THROW(readInt("0 x0"), std::exception);
  EXPECT_THROW(readInt("0x 0"), std::exception);
  EXPECT_THROW(readInt("0Y0"), std::exception);
  EXPECT_THROW(readInt("0xfg"), std::exception);
  EXPECT_THROW(readInt("0x1.0"), std::exception);
  EXPECT_THROW(readInt("0x1.0p1"), std::exception);

  {
    // In array/object
    auto r = reader("[0x1, 0xA, {color: 0xFF00FF}]");
    r.readListBegin();
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 1);
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 0xA);
    r.readObjectBegin();
    r.readObjectName();
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 0xFF00FF);
    r.readObjectEnd();
    r.readListEnd();
  }

  std::uint64_t maxI64 = std::numeric_limits<std::int64_t>::max();

  std::string maxHex = fmt::format("0x{:x}", maxI64);
  std::string minHex = fmt::format("-0x{:x}", maxI64 + 1);
  std::string maxHexPlusOne = fmt::format("0x{:x}", maxI64 + 1);
  std::string minHexMinusOne = fmt::format("-0x{:x}", maxI64 + 2);

  EXPECT_EQ(readInt(maxHex), std::numeric_limits<std::int64_t>::max());
  EXPECT_EQ(readInt(minHex), std::numeric_limits<std::int64_t>::min());
  EXPECT_THROW(readInt(maxHexPlusOne), std::exception);
  EXPECT_THROW(readInt(minHexMinusOne), std::exception);
}

TEST_F(Json5ReaderTest, ScientificNotation) {
  auto verify = [&]<class FloatType>(auto fp, FloatType) {
    for (auto s :
         {"3.14e0", "314e-2", "0.314E+1", ".314e1", "0.00314e3", "31400E-4"}) {
      EXPECT_EQ(
          std::get<FloatType>(reader(s).readPrimitive(fp)), FloatType(3.14));
    }
  };
  verify(Single, float{});
  verify(Double, double{});
}

TEST_F(Json5ReaderTest, UnicodeEscape) {
  auto readStr = [this](std::string_view input) {
    return std::get<std::string>(reader(input).readPrimitive(Double));
  };

  EXPECT_EQ(readStr(R"("\u20ac")"), "€");
  EXPECT_EQ(readStr(R"("\u20AC")"), "€"); // case insensitive
  EXPECT_EQ(readStr(R"("caf\u00e9")"), "café");
  EXPECT_EQ(readStr(R"("\u00482\u004F")"), "H2O");
  EXPECT_EQ(readStr(R"("\u0000")"), std::string(1, '\0'));
  EXPECT_EQ(readStr(R"("\u0048\u0065\u006C\u006C\u006F")"), "Hello");
  EXPECT_EQ(readStr("'\\u0041'"), "A"); // single-quoted

  EXPECT_THROW(readStr(R"("\u00")"), std::exception); // truncated
  EXPECT_THROW(readStr(R"("\u00GZ")"), std::exception); // invalid hex
}

TEST_F(Json5ReaderTest, PeekToken) {
  using Token = Json5Reader::Token;

  auto verify = [this](std::string_view input) {
    auto r = reader(input);

    // Object begin
    EXPECT_EQ(r.peekToken(), Token::ObjectBegin);
    EXPECT_EQ(r.peekToken(), Token::ObjectBegin); // Does not consume
    r.readObjectBegin();

    // string value
    EXPECT_EQ(r.readObjectName(), "string");
    EXPECT_EQ(r.peekToken(), Token::Primitive);
    EXPECT_EQ(std::get<std::string>(r.readPrimitive(Double)), "hello");

    // array value
    EXPECT_EQ(r.readObjectName(), "array");
    EXPECT_EQ(r.peekToken(), Token::ListBegin);
    r.readListBegin();
    EXPECT_EQ(r.peekToken(), Token::Primitive);
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), -1);
    EXPECT_EQ(r.peekToken(), Token::Primitive);
    EXPECT_EQ(std::get<std::string>(r.readPrimitive(Double)), "two");
    EXPECT_EQ(r.peekToken(), Token::Primitive);
    EXPECT_FALSE(std::get<bool>(r.readPrimitive(Double)));
    EXPECT_EQ(r.peekToken(), Token::ListEnd);
    r.readListEnd();

    // nested object value
    EXPECT_EQ(r.readObjectName(), "nested");
    EXPECT_EQ(r.peekToken(), Token::ObjectBegin);
    r.readObjectBegin();
    EXPECT_EQ(r.readObjectName(), "inner");
    EXPECT_EQ(r.peekToken(), Token::Primitive);
    EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 99);
    EXPECT_EQ(r.peekToken(), Token::ObjectEnd);
    r.readObjectEnd();

    // End of outer object
    EXPECT_EQ(r.peekToken(), Token::ObjectEnd);
    r.readObjectEnd();
  };

  // Standard JSON
  verify(R"({
    "string": "hello",
    "array": [-1, "two", false],
    "nested": {"inner": 99}
  })");

  // JSON5
  verify(R"({
    string: 'hello', // comment
    array: [-1, "two", false,],
    nested: {inner: /* comment */ +99,},
  })");
}

TEST_F(Json5ReaderTest, Comments) {
  for (auto s : {
           "// line comment\n42",
           "42// trailing",
           "/* block */42",
           "/* multi\n   line */42",
           "/*a*//*b*/42/**/",
           "// line\n/* block */42",
           "/*/* stars inside */42",
       }) {
    EXPECT_EQ(std::get<std::int64_t>(reader(s).readPrimitive(Double)), 42);
  }
}

TEST_F(Json5ReaderTest, CommentsInStringsAndContainers) {
  auto r = reader(R"({
    // a comment
    "/* comment inside string */" /* inline */: /* before value */ [
      // first element
      "/* multi-line \
      string1 */",
      "// multi-line \
      string2"
    ]
    // trailing
  })");
  r.readObjectBegin();
  EXPECT_EQ(r.readObjectName(), "/* comment inside string */");
  r.readListBegin();
  EXPECT_EQ(
      std::get<std::string>(r.readPrimitive(Double)),
      "/* multi-line \n      string1 */");
  EXPECT_EQ(
      std::get<std::string>(r.readPrimitive(Double)),
      "// multi-line \n      string2");
  r.readListEnd();
  r.readObjectEnd();
}

TEST_F(Json5ReaderTest, PeekTokenAfterEOF) {
  auto r = reader("42");
  EXPECT_EQ(std::get<std::int64_t>(r.readPrimitive(Double)), 42);
  EXPECT_THROW((void)r.peekToken(), std::runtime_error);
}

TEST_F(Json5ReaderTest, ErrorHandling) {
  // Unexpected end of input
  {
    auto r = reader("");
    EXPECT_THROW((void)r.peekToken(), std::runtime_error);
  }

  // Expected object begin, got something else
  {
    auto r = reader("123");
    EXPECT_THROW(r.readObjectBegin(), std::runtime_error);
  }

  // Expected list begin, got something else
  {
    auto r = reader("{}");
    EXPECT_THROW(r.readListBegin(), std::runtime_error);
  }

  // Expected object end, got something else
  {
    auto r = reader("{\"a\": 1");
    r.readObjectBegin();
    r.readObjectName();
    r.readPrimitive(Double);
    EXPECT_THROW(r.readObjectEnd(), std::runtime_error);
  }

  // Expected list end, got something else
  {
    auto r = reader("[1, 2");
    r.readListBegin();
    r.readPrimitive(Double);
    r.readPrimitive(Double);
    EXPECT_THROW(r.readListEnd(), std::runtime_error);
  }

  // Consecutive commas
  {
    auto r = reader("[1,,2]");
    r.readListBegin();
    EXPECT_THROW(r.readPrimitive(Double), std::runtime_error);
  }

  // Missing comma between elements
  {
    auto r = reader("[1 2]");
    r.readListBegin();
    EXPECT_THROW(r.readPrimitive(Double), std::runtime_error);
  }

  // Missing comma between containers
  {
    auto r = reader("[[][]]");
    r.readListBegin();
    r.readListBegin();
    auto f = [&] {
      r.readListEnd();
      r.readListBegin();
    };
    EXPECT_THROW(f(), std::runtime_error);
  }

  // Unknown escape sequence in string
  {
    auto r = reader(R"("\q")");
    EXPECT_THROW(r.readPrimitive(Double), std::runtime_error);
  }

  // Unescaped newline in string
  {
    auto r = reader("\"hello\nworld\"");
    EXPECT_THROW(r.readPrimitive(Double), std::runtime_error);
  }

  // Unexpected identifier
  {
    auto r = reader("undefined");
    EXPECT_THROW(r.readPrimitive(Double), std::runtime_error);
  }

  // Invalid object name (starts with digit)
  {
    auto r = reader("{123: 456}");
    r.readObjectBegin();
    EXPECT_THROW(r.readObjectName(), std::runtime_error);
  }

  // Missing colon after object name
  {
    auto r = reader("{\"key\" 123}");
    r.readObjectBegin();
    EXPECT_THROW(r.readObjectName(), std::runtime_error);
  }

  // Invalid number (just a sign)
  {
    auto r = reader("+");
    EXPECT_THROW(r.readPrimitive(Double), std::runtime_error);
  }

  // Invalid Infinity/NaN spelling
  {
    auto r = reader("Inf");
    EXPECT_THROW(r.readPrimitive(Double), std::runtime_error);
  }

  // Missing digits after exponent
  {
    auto r = reader("1e");
    EXPECT_THROW(r.readPrimitive(Double), std::exception);
  }

  // Missing digits after exponent sign
  {
    auto r = reader("1e+");
    EXPECT_THROW(r.readPrimitive(Double), std::exception);
  }

  // Missing digits after negative exponent sign
  {
    auto r = reader("2.5E-");
    EXPECT_THROW(r.readPrimitive(Double), std::exception);
  }

  // No cursor set
  {
    Json5Reader r;
    EXPECT_THROW((void)r.getCursor(), std::exception);
  }

  // Unterminated block comment
  {
    auto r = reader("42/* unterminated");
    EXPECT_THROW((void)r.readPrimitive(Double), std::exception);
  }

  // Comment-only input
  {
    auto r = reader("// Comment-only, no value");
    EXPECT_THROW((void)r.peekToken(), std::exception);
  }

  // Leading zeroes are not valid (not valid JSON or JSON5)
  {
    EXPECT_THROW(reader("00").readPrimitive(Double), std::exception);
    EXPECT_THROW(reader("01").readPrimitive(Double), std::exception);
    EXPECT_THROW(reader("007").readPrimitive(Double), std::exception);
    EXPECT_THROW(reader("0123").readPrimitive(Double), std::exception);
  }
}

} // namespace
} // namespace apache::thrift::json5::detail
