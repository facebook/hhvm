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

#include <thrift/lib/cpp2/protocol/JSONProtocolCommon.h>

#include <random>
#include <string>

#include <gtest/gtest.h>
#include <folly/String.h>
#include <folly/io/IOBuf.h>

using namespace std::literals;

class JSONProtocolCommonTest : public testing::Test {};

namespace {

auto quote(std::string_view sv) {
  return "\"" + folly::cEscape<std::string>(sv) + "\"";
}

template <typename Fn>
void each_split(std::string_view sv, Fn fn) {
  for (size_t i = 0; i <= sv.size(); ++i) {
    fn(i, std::array{sv.substr(0, i), sv.substr(i)});
  }
}

template <typename Vec>
std::unique_ptr<folly::IOBuf> from_split(Vec const& vec) {
  //  cursor transparently handles empty bufs
  auto buf = folly::IOBuf::create(0);
  for (auto sv : vec) {
    buf->appendToChain(folly::IOBuf::copyBuffer(folly::StringPiece(sv)));
  }
  return buf;
}

} // namespace

TEST_F(JSONProtocolCommonTest, whitespace_combinatorics) {
  //  test behavior of member readWhitespace with a large suite of inputs

  struct Reader : apache::thrift::JSONProtocolReaderCommon {
    using apache::thrift::JSONProtocolReaderCommon::readWhitespace;
  };

  constexpr size_t big = 40;

  std::mt19937 rng;

  auto const spaces = [](size_t const len) { //
    return std::string(len, ' ');
  };
  auto const whitespaces = [&](size_t const len) {
    std::string s;
    for (size_t i = 0; i < len; ++i) {
      s += " \r\n\t"[rng() % 4];
    }
    return s;
  };
  auto const blackspace = "[[[blackspace]]]";

  auto read = [&](auto const& svs) {
    auto const buf = from_split(svs);
    Reader r;
    r.setInput(buf.get());
    auto const pos0 = r.getCursorPosition();
    auto const len = r.readWhitespace();
    auto const pos1 = r.getCursorPosition();
    EXPECT_EQ(len, pos1 - pos0);
    return len;
  };

  //  spaces
  for (size_t i = 0; i <= big; ++i) {
    auto const s = spaces(i);
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i, read(vec)) //
          << "split[" << split << "] input: " << quote(s);
    });
  }

  //  spaces then text
  for (size_t i = 0; i <= big; ++i) {
    auto const s = spaces(i) + blackspace;
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i, read(vec)) //
          << "split[" << split << "] input: " << quote(s);
    });
  }

  //  newline then spaces
  for (size_t i = 0; i <= big; ++i) {
    auto const s = "\n" + spaces(i);
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i + 1, read(vec))
          << "split[" << split << "] input: " << quote(s);
    });
  }

  //  newline then spaces then text
  for (size_t i = 0; i <= big; ++i) {
    auto const s = "\n" + spaces(i) + blackspace;
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i + 1, read(vec))
          << "split[" << split << "] input: " << quote(s);
    });
  }

  //  whitespaces
  for (size_t i = 0; i <= big; ++i) {
    auto const s = whitespaces(i);
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i, read(vec)) //
          << "split[" << split << "] input: " << quote(s);
    });
  }

  //  whitespaces then text
  for (size_t i = 0; i <= big; ++i) {
    auto const s = whitespaces(i) + blackspace;
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i, read(vec)) //
          << "split[" << split << "] input: " << quote(s);
    });
  }

  //  newline then whitespaces
  for (size_t i = 0; i <= big; ++i) {
    auto const s = "\n" + whitespaces(i);
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i + 1, read(vec))
          << "split[" << split << "] input: " << quote(s);
    });
  }

  //  newline then whitespaces then text
  for (size_t i = 0; i <= big; ++i) {
    auto const s = "\n" + whitespaces(i) + blackspace;
    each_split(s, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(i + 1, read(vec))
          << "split[" << split << "] input: " << quote(s);
    });
  }
}

TEST_F(JSONProtocolCommonTest, string_ascii) {
  struct Reader : apache::thrift::JSONProtocolReaderCommon {
    using apache::thrift::JSONProtocolReaderCommon::readJSONString;
  };
  struct Writer : apache::thrift::JSONProtocolWriterCommon {
    using apache::thrift::JSONProtocolWriterCommon::writeJSONString;
  };

  auto write = [](std::string_view sv) {
    folly::IOBufQueue q;
    Writer w;
    w.setOutput(&q);
    w.writeJSONString(sv);
    return std::string(folly::StringPiece(q.move()->coalesce()));
  };

  auto read = [](auto const& svs) {
    auto const buf = from_split(svs);
    Reader r;
    r.setInput(buf.get());
    std::string out;
    r.readJSONString(out);
    // NOLINTNEXTLINE(clang-diagnostic-nrvo) false-positive?
    return out;
  };

  for (size_t i = 0; i < 40; ++i) {
    auto const s = std::invoke([&] {
      std::string r;
      r += std::string(i, 'x');
      return r;
    });
    EXPECT_EQ(i, s.size());
    auto v = write(s);
    EXPECT_EQ(i + 2, v.size());
    each_split(v, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(s, read(vec)) //
          << "split[" << split << "] input: " << quote(s);
    });
  }

  for (size_t i = 0; i < 40; ++i) {
    auto const s = std::invoke([&] {
      std::string r;
      for (size_t j = 0; j < i; ++j) {
        r += std::string("x\"y\"z");
      }
      return r;
    });
    EXPECT_EQ(5 * i, s.size());
    auto v = write(s);
    EXPECT_EQ(7 * i + 2, v.size());
    each_split(v, [&](auto const& split, auto const& vec) {
      EXPECT_EQ(s, read(vec)) //
          << "split[" << split << "] input: " << quote(s);
    });
  }
}

TEST_F(JSONProtocolCommonTest, string_ascii_underflow) {
  struct Reader : apache::thrift::JSONProtocolReaderCommon {
    using apache::thrift::JSONProtocolReaderCommon::readJSONString;
  };

  auto read = [](auto const& svs) {
    auto const buf = from_split(svs);
    Reader r;
    r.setInput(buf.get());
    std::string out;
    r.readJSONString(out);
    // NOLINTNEXTLINE(clang-diagnostic-nrvo) false-positive?
    return out;
  };

  {
    auto v = "\"hello"s; // no terminal "
    each_split(v, [&](auto, auto const& vec) {
      EXPECT_THROW(read(vec), std::out_of_range);
    });
  }
}

TEST_F(JSONProtocolCommonTest, string_ascii_stress) {
  struct Reader : apache::thrift::JSONProtocolReaderCommon {
    using apache::thrift::JSONProtocolReaderCommon::readJSONString;
  };
  struct Writer : apache::thrift::JSONProtocolWriterCommon {
    using apache::thrift::JSONProtocolWriterCommon::writeJSONString;
  };

  std::mt19937 rng;
  for (size_t i = 0; i < 1024; ++i) {
    std::uniform_int_distribution<size_t> length_d{0, 16};
    std::uniform_int_distribution<uint16_t> char_d{0x00, 0x7f};
    std::string e(length_d(rng), 0);
    std::generate(e.begin(), e.end(), [&] { return char(char_d(rng)); });

    folly::IOBufQueue q;
    Writer w;
    w.setOutput(&q);
    w.writeJSONString(e);

    std::string f;
    Reader r;
    r.setInput(q.front());
    r.readJSONString(f);

    EXPECT_EQ(e, f);
  }
}
