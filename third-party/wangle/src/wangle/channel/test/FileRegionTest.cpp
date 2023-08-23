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

#include <folly/io/async/test/AsyncSocketTest.h>
#include <folly/portability/GTest.h>
#include <wangle/channel/FileRegion.h>

#ifdef SPLICE_F_NONBLOCK
using namespace folly;
using namespace wangle;
using namespace testing;

struct FileRegionTest : public Test {
  FileRegionTest() {
    // Connect
    socket = AsyncSocket::newSocket(&evb);
    socket->connect(&ccb, server.getAddress(), 30);

    // Accept the connection
    acceptedSocket = server.acceptAsync(&evb);
    acceptedSocket->setReadCB(&rcb);

    // Create temp file
    char path[] = "/tmp/AsyncSocketTest.WriteFile.XXXXXX";
    fd = mkostemp(path, O_RDWR);
    EXPECT_TRUE(fd > 0);
    EXPECT_EQ(0, unlink(path));
  }

  ~FileRegionTest() override {
    // Close up shop
    close(fd);
    acceptedSocket->close();
    socket->close();
  }

  folly::test::TestServer server;
  EventBase evb;
  std::shared_ptr<AsyncSocket> socket;
  std::shared_ptr<AsyncSocket> acceptedSocket;
  folly::test::ConnCallback ccb;
  folly::test::ReadCallback rcb;
  int fd;
};

TEST_F(FileRegionTest, Basic) {
  const size_t count = 1000000000; // 1 GB
  std::unique_ptr<uint8_t[]> zeroBuf = std::make_unique<uint8_t[]>(count);
  write(fd, zeroBuf.get(), count);

  FileRegion fileRegion(fd, 0, count);
  auto f = fileRegion.transferTo(socket);
  try {
    std::move(f).getVia(&evb);
  } catch (std::exception& e) {
    LOG(FATAL) << exceptionStr(e);
  }

  // Let the reads run to completion
  socket->shutdownWrite();
  evb.loopIgnoreKeepAlive();

  ASSERT_EQ(rcb.state, folly::test::STATE_SUCCEEDED);

  size_t receivedBytes = 0;
  for (auto& buf : rcb.buffers) {
    receivedBytes += buf.length;
    ASSERT_EQ(memcmp(buf.buffer, zeroBuf.get(), buf.length), 0);
  }
  ASSERT_EQ(receivedBytes, count);
}

TEST_F(FileRegionTest, Repeated) {
  const size_t count = 1000000;
  std::unique_ptr<uint8_t[]> zeroBuf = std::make_unique<uint8_t[]>(count);
  write(fd, zeroBuf.get(), count);

  int sendCount = 1000;

  FileRegion fileRegion(fd, 0, count);
  std::vector<Future<Unit>> fs;
  for (int i = 0; i < sendCount; i++) {
    fs.push_back(fileRegion.transferTo(socket));
  }
  auto f = collect(fs).via(&evb);
  ASSERT_NO_THROW(std::move(f).getVia(&evb));

  // Let the reads run to completion
  socket->shutdownWrite();
  evb.loopIgnoreKeepAlive();

  ASSERT_EQ(rcb.state, folly::test::STATE_SUCCEEDED);

  size_t receivedBytes = 0;
  for (auto& buf : rcb.buffers) {
    receivedBytes += buf.length;
  }
  ASSERT_EQ(receivedBytes, sendCount * count);
}
#endif
