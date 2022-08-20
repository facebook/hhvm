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

#include <memory>
#include <folly/Random.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <folly/portability/GTest.h>

using namespace apache::thrift;
using namespace folly;
using namespace apache::thrift::transport;

namespace apache {
namespace thrift {
namespace transport {

TEST(THeaderTest, largetransform) {
  THeader header;
  header.setTransform(THeader::ZLIB_TRANSFORM); // ZLib flag

  size_t buf_size = 10000000;
  std::unique_ptr<IOBuf> buf(IOBuf::create(buf_size));
  // Make the data compressible, but not totally random
  for (size_t i = 0; i < buf_size / 4; i++) {
    buf->writableData()[i] = (int8_t)folly::Random::rand32(256);
    buf->writableData()[i + 1] = buf->writableData()[i];
    buf->writableData()[i + 2] = buf->writableData()[i];
    buf->writableData()[i + 3] = (int8_t)folly::Random::rand32(256);
  }
  buf->append(buf_size);

  THeader::StringToStringMap persistentHeaders;
  buf = header.addHeader(std::move(buf), persistentHeaders);
  buf_size = buf->computeChainDataLength();
  buf->gather(buf_size);
  std::unique_ptr<IOBufQueue> queue(new IOBufQueue);
  std::unique_ptr<IOBufQueue> queue2(new IOBufQueue);
  queue->append(std::move(buf));
  queue2->append(queue->split(buf_size / 4));
  queue2->append(queue->split(buf_size / 4));
  queue2->append(IOBuf::create(0)); // Empty buffer should work
  queue2->append(queue->split(buf_size / 4));
  queue2->append(queue->move());

  size_t needed;

  buf = header.removeHeader(queue2.get(), needed, persistentHeaders);
  EXPECT_EQ(buf->computeChainDataLength(), 10000000);
}

TEST(THeaderTest, http_clear_header) {
  THeader header;
  header.setClientType(THRIFT_HTTP_CLIENT_TYPE);
  auto parser = std::make_shared<apache::thrift::util::THttpClientParser>(
      "testhost", "testuri");
  header.setHttpClientParser(parser);
  header.setHeader("WriteHeader", "foo");

  size_t buf_size = 1000000;
  std::unique_ptr<IOBuf> buf(IOBuf::create(buf_size));
  THeader::StringToStringMap persistentHeaders;
  buf = header.addHeader(std::move(buf), persistentHeaders);

  EXPECT_TRUE(header.isWriteHeadersEmpty());
}

TEST(THeaderTest, transform) {
  // Simple test for TRANSFORMS enum to string conversion
  EXPECT_EQ(
      THeader::getStringTransform(THeader::TRANSFORMS::ZLIB_TRANSFORM), "zlib");
}

TEST(THeaderTest, eraseReadHeader) {
  THeader header;
  header.setReadHeaders({{"foo", "v"}, {"bar", "v"}, {"moo", "v"}});
  EXPECT_EQ(3, header.getHeaders().size());
  header.eraseReadHeader("bar");
  EXPECT_EQ(2, header.getHeaders().size());
}

TEST(THeaderTest, removeHeaderNullptrQueue) {
  THeader header;
  size_t needed;
  THeader::StringToStringMap persistentHeaders;
  EXPECT_EQ(nullptr, header.removeHeader(nullptr, needed, persistentHeaders));
  EXPECT_EQ(4, needed);
}

TEST(THeaderTest, removeHeaderEmptyQueue) {
  THeader header;
  size_t needed;
  THeader::StringToStringMap persistentHeaders;
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  EXPECT_EQ(nullptr, header.removeHeader(&queue, needed, persistentHeaders));
  EXPECT_EQ(4, needed);
}

TEST(THeaderTest, explicitTimeoutAndPriority) {
  THeader header;
  header.setClientTimeout(std::chrono::milliseconds(100));
  header.setClientQueueTimeout(std::chrono::milliseconds(200));
  header.setCallPriority(concurrency::PRIORITY::IMPORTANT);
  THeader::StringToStringMap headerMap;
  headerMap[THeader::CLIENT_TIMEOUT_HEADER] = "10";
  headerMap[THeader::QUEUE_TIMEOUT_HEADER] = "20";
  headerMap[THeader::PRIORITY_HEADER] = "3";
  header.setReadHeaders(std::move(headerMap));
  EXPECT_EQ(std::chrono::milliseconds(100), header.getClientTimeout());
  EXPECT_EQ(std::chrono::milliseconds(200), header.getClientQueueTimeout());
  EXPECT_EQ(concurrency::PRIORITY::IMPORTANT, header.getCallPriority());
}

TEST(THeaderTest, headerTimeoutAndPriority) {
  THeader header;
  THeader::StringToStringMap headerMap;
  headerMap[THeader::CLIENT_TIMEOUT_HEADER] = "10";
  headerMap[THeader::QUEUE_TIMEOUT_HEADER] = "20";
  headerMap[THeader::PRIORITY_HEADER] = "3";
  header.setReadHeaders(std::move(headerMap));
  EXPECT_EQ(std::chrono::milliseconds(10), header.getClientTimeout());
  EXPECT_EQ(std::chrono::milliseconds(20), header.getClientQueueTimeout());
  EXPECT_EQ(concurrency::PRIORITY::NORMAL, header.getCallPriority());
}

TEST(THeaderTest, defaultTimeoutAndPriority) {
  THeader header;
  EXPECT_EQ(std::chrono::milliseconds(0), header.getClientTimeout());
  EXPECT_EQ(std::chrono::milliseconds(0), header.getClientQueueTimeout());
  EXPECT_EQ(concurrency::PRIORITY::N_PRIORITIES, header.getCallPriority());
}

TEST(THeaderTest, serverQueueingMetadataTest) {
  static constexpr std::chrono::milliseconds kQueueTimeout(10);
  static constexpr std::chrono::milliseconds kTimeQueued(6);
  THeader hdr1, hdr2;
  hdr1.setServerQueueTimeout(kQueueTimeout);
  hdr1.setProcessDelay(kTimeQueued);
  EXPECT_EQ(kQueueTimeout, hdr1.getServerQueueTimeout().value());
  EXPECT_EQ(kTimeQueued, hdr1.getProcessDelay().value());
  EXPECT_FALSE(hdr2.getServerQueueTimeout().hasValue());
  EXPECT_FALSE(hdr2.getProcessDelay().hasValue());
}

void testAsciiHeaderData(const std::string& data, const std::string& expected) {
  auto buf = folly::IOBuf::copyBuffer(data);
  THeader::StringToStringMap persistentHeaders;
  THeader header;
  std::unique_ptr<IOBufQueue> queue(new IOBufQueue);
  queue->append(std::move(buf));

  size_t needed;
  try {
    buf = header.removeHeader(queue.get(), needed, persistentHeaders);
    ASSERT_TRUE(false);
  } catch (TTransportException& e) {
    ASSERT_EQ(e.what(), expected);
  }
}

TEST(THeaderTest, asciiData1) {
  std::string data = "llocations statistics!";
  auto expected = "The Thrift server received an ASCII request '" + data +
      "' and safely ignored it. In all likelihood, "
      "this isn't the reason of your problem (probably a local daemon "
      "sending HTTP content to all listening ports).";

  testAsciiHeaderData(data, expected);
}

TEST(THeaderTest, asciiData2) {
  std::string data = "ap Here for more details";
  auto expected = "The Thrift server received an ASCII request '" + data +
      "' and safely ignored it. In all likelihood, "
      "this isn't the reason of your problem (probably a local daemon "
      "sending HTTP content to all listening ports).";
  testAsciiHeaderData(data, expected);
}

TEST(THeaderTest, asciiData3) {
  auto expected =
      "Header transport frame is too large: 1918987876 "
      "(hex 0x72616e64, ascii 'rand')";
  testAsciiHeaderData("random data", expected);
}

} // namespace transport
} // namespace thrift
} // namespace apache
