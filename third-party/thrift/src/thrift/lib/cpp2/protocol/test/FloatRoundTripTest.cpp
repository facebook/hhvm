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
#include <thrift/lib/cpp2/protocol/detail/JsonWriter.h>

#include <bit>
#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

namespace apache::thrift::json5::detail {
namespace {

// There are 2^32 possible float bit patterns. To avoid test timeout, we split
// them into kNumShards interleaved shards: shard i tests bit patterns
// i, i + kNumShards, i + 2*kNumShards, ..., collectively covering all 2^32.
constexpr std::uint32_t kNumShards = folly::kIsDebug ? 1000 : 100;

class FloatRoundTripTest : public ::testing::TestWithParam<std::uint32_t> {};

TEST_P(FloatRoundTripTest, Shard) {
  const std::uint32_t max = std::numeric_limits<std::uint32_t>::max();
  for (std::uint64_t bits = GetParam(); bits <= max; bits += kNumShards) {
    auto original = std::bit_cast<float>(static_cast<std::uint32_t>(bits));
    if (!std::isfinite(original)) {
      continue;
    }

    folly::IOBufQueue queue;
    JsonWriter writer;
    writer.setOutput(folly::io::QueueAppender(&queue, 20));
    writer.writeFloat(original);

    auto buf = queue.move();
    Json5Reader reader;
    reader.setCursor(folly::io::Cursor(buf.get()));
    auto v = reader.readPrimitive(Json5Reader::FloatingPointPrecision::Single);
    EXPECT_EQ(original, std::get<float>(v)) << bits;
  }
}

INSTANTIATE_TEST_SUITE_P(
    /*InstantiationName=*/,
    /*TestSuiteName=*/FloatRoundTripTest,
    /*param_generator=*/::testing::Range(std::uint32_t{0}, kNumShards));

} // namespace
} // namespace apache::thrift::json5::detail
