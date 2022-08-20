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

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp/util/THttpParser.h>

#include <fmt/core.h>

#include <folly/String.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

namespace {

class THttpClientParserTest : public testing::Test {};

using HeaderMap = apache::thrift::transport::THeader::StringToStringMap;

void write(apache::thrift::util::THttpParser& parser, folly::StringPiece text) {
  while (!text.empty()) {
    void* buf = nullptr;
    size_t len = 0;
    parser.getReadBuffer(&buf, &len);
    len = std::min(len, text.size());
    std::memcpy(buf, text.data(), len);
    text.advance(len);
    parser.readDataAvailable(len);
  }
}
} // namespace

TEST_F(THttpClientParserTest, too_many_headers) {
  apache::thrift::util::THttpClientParser parser;
  HeaderMap header;
  for (int i = 0; i < 100; i++) {
    header[fmt::format("testing_header{}", i)] = "test_header";
  }
  HeaderMap header_persistent;
  auto answer = std::string("{'testing': 'this is a test'}");
  auto pre = folly::IOBuf::copyBuffer(answer.c_str(), answer.size());
  auto buf = parser.constructHeader(
      std::move(pre), header, header_persistent, nullptr);
  auto fbs = buf->moveToFbString();
  std::string output = std::string(fbs.c_str(), fbs.size());
  for (int i = 0; i < 100; i++) {
    std::string s = fmt::format("testing_header{}: test_header\r\n", i);
    EXPECT_THAT(output, ::testing::HasSubstr(s));
  }
  std::vector<std::string> o;
  folly::split("\r\n", output, o);
  if (o.at(o.size() - 1) != answer) {
    FAIL() << fmt::format(
        "Final line should be {} not {}", answer, o.at(o.size() - 1));
  }
}

TEST_F(THttpClientParserTest, read_encapsulated_status_line) {
  apache::thrift::util::THttpClientParser parser;
  write(parser, "HTTP/1.1 200 OK\r\n");
  SUCCEED() << "did not crash";
}
