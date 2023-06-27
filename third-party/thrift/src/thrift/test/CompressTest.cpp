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

#include <cstdlib>
#include <ctime>
#include <folly/Benchmark.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

#include <thrift/test/gen-cpp2/ThriftTest.h>

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace thrift::test;

void testMessage(
    uint8_t flag, int iters, bool easyMessage, bool binary = false) {
  Bonk b;
  Bonk bin;
  *b.message() = "";

  THeader header;
  if (flag) {
    header.setTransform(flag);
  }
  if (binary) {
    header.setProtocolId(T_BINARY_PROTOCOL);
  }
  size_t neaded;

  for (int i = 0; i < iters; i++) {
    if (easyMessage) {
      *b.message() += "t";
    } else {
      *b.message() += 66 + rand() % 24;
    }
    folly::IOBufQueue out;
    if (binary) {
      BinarySerializer::serialize(b, &out);
    } else {
      CompactSerializer::serialize(b, &out);
    }

    auto withHeader = header.addHeader(out.move());
    out.append(std::move(withHeader));
    THeader::StringToStringMap strMap;
    auto buf = header.removeHeader(&out, neaded, strMap);

    if (binary) {
      BinarySerializer::deserialize(buf.get(), bin);
    } else {
      CompactSerializer::deserialize(buf.get(), bin);
    }
  }
}

void testChainedCompression(uint8_t flag, int iters) {
  THeader header;
  transport::THeader::StringToStringMap persistentHeaders;
  if (flag) {
    header.setTransform(flag);
  }

  auto head = folly::IOBuf::create(0);

  for (int i = 0; i < iters; i++) {
    auto buf = folly::IOBuf::create(1);
    buf->append(1);
    *(buf->writableData()) = 66 + rand() % 24;
    head->prependChain(std::move(buf));
  }

  auto cloned = head->clone();

  auto compressed = header.addHeader(std::move(head));
  EXPECT_NE(compressed, nullptr);
  printf("%i\n", (int)compressed->length());

  size_t needed = 0;
  folly::IOBufQueue q;
  q.append(std::move(compressed));

  auto uncompressed = header.removeHeader(&q, needed, persistentHeaders);
  EXPECT_NE(uncompressed, nullptr);
  EXPECT_EQ(needed, 0);
  EXPECT_TRUE(q.empty());

  cloned->coalesce();
  uncompressed->coalesce();
  printf("%i, %i\n", (int)cloned->length(), (int)uncompressed->length());
  EXPECT_EQ(cloned->length(), uncompressed->length());
  EXPECT_EQ(0, memcmp(cloned->data(), uncompressed->data(), cloned->length()));
}

BENCHMARK(BM_UncompressedBinary, iters) {
  testMessage(0, iters, true, true);
}

BENCHMARK(BM_Uncompressed, iters) {
  testMessage(0, iters, true);
}

BENCHMARK(BM_Zlib, iters) {
  testMessage(0x01, iters, true);
}

BENCHMARK(BM_Zstd, iters) {
  testMessage(5, iters, true);
}

// Test a 'hard' to compress message, more random.

BENCHMARK(BM_UncompressedBinaryHard, iters) {
  testMessage(0, iters, false, true);
}

BENCHMARK(BM_UncompressedHard, iters) {
  testMessage(0, iters, false);
}

BENCHMARK(BM_ZlibHard, iters) {
  testMessage(0x01, iters, false);
}

BENCHMARK(BM_ZstdHard, iters) {
  testMessage(5, iters, false);
}

TEST(chained, none) {
  testChainedCompression(0, 1000);
}

TEST(chained, zlib) {
  testChainedCompression(1, 1000);
}

TEST(chained, zstd) {
  testChainedCompression(5, 1000);
}

TEST(sdf, sdfsd) {
  Bonk b;
  Bonk bin;
  *b.message() = "";
  for (int i = 0; i < 10000; i++) {
    *b.message() += 66 + rand() % 24;
  }

  THeader header;
  folly::IOBufQueue out;
  CompactSerializer::serialize(b, &out);
  auto serialized = out.move();

  auto uncompressedSize =
      header.addHeader(serialized->clone())->computeChainDataLength();

  header.setTransform(THeader::ZLIB_TRANSFORM);
  auto compressed = header.addHeader(serialized->clone());
  auto compressedSize = compressed->computeChainDataLength();

  // Should compress.
  EXPECT_LT(compressedSize, uncompressedSize);
  // Exactly one transform should be applied.
  folly::io::Cursor cursor(compressed.get());
  cursor.skip(15);
  EXPECT_EQ(cursor.read<uint8_t>(), 1);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  srand(time(0));

  auto ret = RUN_ALL_TESTS();

  // Run the benchmarks
  if (!ret) {
    folly::runBenchmarksOnFlag();
  }

  return 0;
}
