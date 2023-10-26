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

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>

namespace apache {
namespace thrift {
namespace rocket {

class FakeOwner : public folly::DelayedDestruction {
 public:
  void handleFrame(std::unique_ptr<folly::IOBuf>) {}
  void close(folly::exception_wrapper) noexcept {}
  void scheduleTimeout(
      folly::HHWheelTimer::Callback*, const std::chrono::milliseconds&) {}
  bool incMemoryUsage(uint32_t) { return true; }
  void decMemoryUsage(uint32_t) {}
};

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
TEST(ParserTest, resizeBufferTest) {
  FakeOwner owner;
  Parser<FakeOwner> parser(owner);
  parser.setReadBufferSize(Parser<FakeOwner>::kMaxBufferSize * 2);

  folly::IOBuf iobuf{folly::IOBuf::CreateOp(), parser.getReadBufferSize()};
  // pretend there is something written into the buffer
  iobuf.append(Parser<FakeOwner>::kMinBufferSize);

  parser.setReadBuffer(std::move(iobuf));
  parser.resizeBuffer();

  EXPECT_EQ(parser.getReadBufferSize(), Parser<FakeOwner>::kMaxBufferSize);
}

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
TEST(ParserTest, noResizeBufferReadBufGtMaxTest) {
  FakeOwner owner;
  Parser<FakeOwner> parser(owner);
  parser.setReadBufferSize(Parser<FakeOwner>::kMaxBufferSize * 2);

  folly::IOBuf iobuf{folly::IOBuf::CreateOp(), parser.getReadBufferSize()};
  // pretend there is something written into the buffer, but with size > max
  iobuf.append(Parser<FakeOwner>::kMaxBufferSize * 1.5);

  parser.setReadBuffer(std::move(iobuf));
  parser.resizeBuffer();

  EXPECT_EQ(parser.getReadBufferSize(), Parser<FakeOwner>::kMaxBufferSize * 2);
}

// TODO: This should be removed once the new buffer logic controlled by
// THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
TEST(ParserTest, noResizeBufferReadBufEqMaxTest) {
  FakeOwner owner;
  Parser<FakeOwner> parser(owner);
  parser.setReadBufferSize(Parser<FakeOwner>::kMaxBufferSize * 2);

  folly::IOBuf iobuf{folly::IOBuf::CreateOp(), parser.getReadBufferSize()};
  // pretend there is something written into the buffer, but with size = max
  iobuf.append(Parser<FakeOwner>::kMaxBufferSize);

  parser.setReadBuffer(std::move(iobuf));
  parser.resizeBuffer();

  EXPECT_EQ(parser.getReadBufferSize(), Parser<FakeOwner>::kMaxBufferSize * 2);
}

} // namespace rocket
} // namespace thrift
} // namespace apache
