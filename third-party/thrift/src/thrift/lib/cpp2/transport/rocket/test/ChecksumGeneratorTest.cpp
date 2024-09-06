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
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>

using namespace ::apache::thrift::rocket;

class ChecksumTest : public testing::Test {
 protected:
  std::vector<std::unique_ptr<folly::IOBuf>> buffers;

  std::unique_ptr<folly::IOBuf> create(size_t bytes) {
    auto buf = folly::IOBuf::create(bytes);
    for (size_t i = 0; i < bytes; i++) {
      buf->writableData()[i] = folly::Random::rand32() % 256;
    }
    buf->append(bytes);
    FOLLY_SAFE_DCHECK(buf->computeChainDataLength() == bytes);
    return buf;
  }

  void setup() {
    buffers.resize(6);
    buffers[0] = create(1 << 10);
    buffers[1] = create(1 << 16);
    buffers[2] = create(1 << 20);
    buffers[3] = create(1 << 23);

    {
      std::unique_ptr<folly::IOBuf> buf = create(1 << 10);
      for (int i = 0; i < 1023; i++) {
        buf->appendToChain(create(1 << 10));
      }

      FOLLY_SAFE_DCHECK(buf->countChainElements() == 1024);
      FOLLY_SAFE_DCHECK(buf->computeChainDataLength() == 1024 * 1024);

      buffers[4] = std::move(buf);
    }

    {
      std::unique_ptr<folly::IOBuf> buf = create(1 << 10);
      for (int i = 0; i < 1023; i++) {
        buf->appendToChain(create(1 << 10));
      }

      FOLLY_SAFE_DCHECK(buf->countChainElements() == 1024);
      FOLLY_SAFE_DCHECK(buf->computeChainDataLength() == 1024 * 1024);

      buffers[5] = std::move(buf);
    }
  }

  ChecksumTest() { setup(); }
};

template <typename Algorithm>
void doChecksumTest(std::vector<std::unique_ptr<folly::IOBuf>>& buffers) {
  for (auto& buf : buffers) {
    folly::IOBuf& bufRef = *buf;
    ChecksumGenerator<Algorithm> generator;
    auto resp = generator.calculateChecksumFromIOBuf(bufRef);
    auto verify =
        generator.validateChecksumFromIOBuf(resp.checksum, resp.salt, bufRef);

    if (!verify) {
      LOG(ERROR) << "Checksum failed for buffer of size " << buf->length();
      FAIL();
    }
  }
}

TEST_F(ChecksumTest, TestXXH3) {
  doChecksumTest<XXH3_64>(buffers);
}

TEST_F(ChecksumTest, TestCRC32) {
  doChecksumTest<CRC32C>(buffers);
}
