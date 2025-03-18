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

#include <folly/String.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

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
