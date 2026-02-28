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

#include <wangle/codec/FixedLengthFrameDecoder.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/test/CodecTestUtils.h>

using namespace folly;
using namespace wangle;
using namespace folly::io;

namespace {
auto createZeroedBuffer(size_t size) {
  auto ret = IOBuf::create(size);
  ret->append(size);
  std::memset(ret->writableData(), 0x00, size);
  return ret;
}
} // namespace

TEST(FixedLengthFrameDecoder, FailWhenLengthFieldEndOffset) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(FixedLengthFrameDecoder(10))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 10);
      }))
      .finalize();

  auto buf3 = createZeroedBuffer(3);
  auto buf11 = createZeroedBuffer(11);
  auto buf16 = createZeroedBuffer(16);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(buf3));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(buf11));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  q.append(std::move(buf16));
  pipeline->read(q);
  EXPECT_EQ(called, 3);
}

TEST(LengthFieldFramePipeline, SimpleTest) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(test::BytesReflector())
      .addBack(LengthFieldPrepender())
      .addBack(LengthFieldBasedFrameDecoder())
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 2);
      }))
      .finalize();

  auto buf = createZeroedBuffer(2);
  pipeline->write(std::move(buf));
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFramePipeline, LittleEndian) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(test::BytesReflector())
      .addBack(LengthFieldBasedFrameDecoder(4, 100, 0, 0, 4, false))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 1);
      }))
      .addBack(LengthFieldPrepender(4, 0, false, false))
      .finalize();

  auto buf = createZeroedBuffer(1);
  pipeline->write(std::move(buf));
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, Simple) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder())
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 1);
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(4);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint32_t)1);
  auto bufData = createZeroedBuffer(1);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(bufData));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, NoStrip) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(2, 10, 0, 0, 0))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 3);
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(2);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint16_t)1);
  auto bufData = createZeroedBuffer(1);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(bufData));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, Adjustment) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(2, 10, 0, -2, 0))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 3);
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(2);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint16_t)3); // includes frame size
  auto bufData = createZeroedBuffer(1);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(bufData));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, PreHeader) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(2, 10, 2, 0, 0))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 5);
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(4);
  RWPrivateCursor c(bufFrame.get());
  c.write((uint16_t)100); // header
  c.writeBE((uint16_t)1); // frame size
  auto bufData = createZeroedBuffer(1);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(bufData));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, PostHeader) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(2, 10, 0, 2, 0))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 5);
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(4);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint16_t)1); // frame size
  c.write((uint16_t)100); // header
  auto bufData = createZeroedBuffer(1);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(bufData));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoderStrip, PrePostHeader) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(2, 10, 2, 2, 4))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 3);
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(6);
  RWPrivateCursor c(bufFrame.get());
  c.write((uint16_t)100); // pre header
  c.writeBE((uint16_t)1); // frame size
  c.write((uint16_t)100); // post header
  auto bufData = createZeroedBuffer(1);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(bufData));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, StripPrePostHeaderFrameInclHeader) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(2, 10, 2, -2, 4))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 3);
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(6);
  RWPrivateCursor c(bufFrame.get());
  c.write((uint16_t)100); // pre header
  c.writeBE((uint16_t)5); // frame size
  c.write((uint16_t)100); // post header
  auto bufData = createZeroedBuffer(1);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  q.append(std::move(bufData));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, FailTestLengthFieldEndOffset) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(4, 10, 4, -2, 4))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        ASSERT_EQ(nullptr, buf);
        called++;
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(8);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint32_t)0); // frame size
  c.write((uint32_t)0); // crap

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, FailTestLengthFieldFrameSize) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(4, 10, 0, 0, 4))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        ASSERT_EQ(nullptr, buf);
        called++;
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(16);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint32_t)12); // frame size
  c.write((uint32_t)0); // nothing
  c.write((uint32_t)0); // nothing
  c.write((uint32_t)0); // nothing

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, FailTestLengthFieldInitialBytes) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(4, 10, 0, 0, 10))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        ASSERT_EQ(nullptr, buf);
        called++;
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(16);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint32_t)4); // frame size
  c.write((uint32_t)0); // nothing
  c.write((uint32_t)0); // nothing
  c.write((uint32_t)0); // nothing

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LengthFieldFrameDecoder, FailTestNotEnoughBytes) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LengthFieldBasedFrameDecoder(4, 10, 0, 0, 0))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        ASSERT_EQ(nullptr, buf);
        called++;
      }))
      .finalize();

  auto bufFrame = createZeroedBuffer(16);
  RWPrivateCursor c(bufFrame.get());
  c.writeBE((uint32_t)7); // frame size - 1 byte too large (7 > 10 - 4)
  c.write((uint32_t)0); // nothing
  c.write((uint32_t)0); // nothing
  c.write((uint32_t)0); // nothing

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(bufFrame));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LineBasedFrameDecoder, Simple) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LineBasedFrameDecoder(10))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 3);
      }))
      .finalize();

  auto buf = createZeroedBuffer(3);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  buf = createZeroedBuffer(1);
  RWPrivateCursor c(buf.get());
  c.write<char>('\n');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  buf = createZeroedBuffer(4);
  RWPrivateCursor c1(buf.get());
  c1.write(' ');
  c1.write(' ');
  c1.write(' ');

  c1.write('\r');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  buf = createZeroedBuffer(1);
  RWPrivateCursor c2(buf.get());
  c2.write('\n');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 2);
}

TEST(LineBasedFrameDecoder, SaveDelimiter) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LineBasedFrameDecoder(10, false))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 4);
      }))
      .finalize();

  auto buf = createZeroedBuffer(3);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 0);

  buf = createZeroedBuffer(1);
  RWPrivateCursor c(buf.get());
  c.write<char>('\n');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  buf = createZeroedBuffer(3);
  RWPrivateCursor c1(buf.get());
  c1.write(' ');
  c1.write(' ');
  c1.write('\r');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  buf = createZeroedBuffer(1);
  RWPrivateCursor c2(buf.get());
  c2.write('\n');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 2);
}

TEST(LineBasedFrameDecoder, Fail) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LineBasedFrameDecoder(10))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        ASSERT_EQ(nullptr, buf);
        called++;
      }))
      .finalize();

  auto buf = createZeroedBuffer(11);

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  buf = createZeroedBuffer(1);
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  buf = createZeroedBuffer(2);
  RWPrivateCursor c(buf.get());
  c.write(' ');
  c.write<char>('\n');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);

  buf = createZeroedBuffer(12);
  RWPrivateCursor c2(buf.get());
  for (int i = 0; i < 11; i++) {
    c2.write(' ');
  }
  c2.write<char>('\n');
  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 2);
}

TEST(LineBasedFrameDecoder, NewLineOnly) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LineBasedFrameDecoder(
          10, true, LineBasedFrameDecoder::TerminatorType::NEWLINE))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 1);
      }))
      .finalize();

  auto buf = createZeroedBuffer(2);
  RWPrivateCursor c(buf.get());
  c.write<char>('\r');
  c.write<char>('\n');

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LineBasedFrameDecoder, CarriageNewLineOnly) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LineBasedFrameDecoder(
          10, true, LineBasedFrameDecoder::TerminatorType::CARRIAGENEWLINE))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 1);
      }))
      .finalize();

  auto buf = createZeroedBuffer(3);
  RWPrivateCursor c(buf.get());
  c.write<char>('\n');
  c.write<char>('\r');
  c.write<char>('\n');

  IOBufQueue q(IOBufQueue::cacheChainLength());

  q.append(std::move(buf));
  pipeline->read(q);
  EXPECT_EQ(called, 1);
}

TEST(LineBasedFrameDecoder, CarriageOnly) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();

  (*pipeline)
      .addBack(LineBasedFrameDecoder(
          10, true, LineBasedFrameDecoder::TerminatorType::CARRIAGENEWLINE))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf>) { FAIL(); }))
      .finalize();

  IOBufQueue q(IOBufQueue::cacheChainLength());
  q.append(IOBuf::copyBuffer("\raa"));
  pipeline->read(q);
}

TEST(LineBasedFrameDecoder, DoubleCarriage) {
  auto pipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>::create();
  int called = 0;

  (*pipeline)
      .addBack(LineBasedFrameDecoder(
          10, true, LineBasedFrameDecoder::TerminatorType::CARRIAGENEWLINE))
      .addBack(test::FrameTester([&](std::unique_ptr<IOBuf> buf) {
        auto sz = buf->computeChainDataLength();
        called++;
        EXPECT_EQ(sz, 1);
      }))
      .finalize();

  IOBufQueue q(IOBufQueue::cacheChainLength());
  q.append(IOBuf::copyBuffer("\r\r\na\r\n"));
  pipeline->read(q);
  EXPECT_EQ(called, 2);
}
