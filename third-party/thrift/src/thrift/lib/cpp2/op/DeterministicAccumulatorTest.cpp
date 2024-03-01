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

#include <thrift/lib/cpp2/op/DeterministicAccumulator.h>
#include <thrift/lib/cpp2/op/Sha256Hasher.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/op/StdHasher.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types_custom_protocol.h>

namespace apache::thrift::op {
namespace {

struct MoveOnlyHahser : StdHasher {
  using StdHasher::StdHasher;
  explicit MoveOnlyHahser() noexcept = default;
  MoveOnlyHahser(const MoveOnlyHahser&) = delete;
  MoveOnlyHahser(MoveOnlyHahser&&) noexcept = default;
  MoveOnlyHahser& operator=(MoveOnlyHahser&&) noexcept = default;
  MoveOnlyHahser& operator=(const MoveOnlyHahser&) noexcept = delete;
};

TEST(DeterministicAccumulatorTest, SimpleHash) {
  auto accumulator = makeDeterministicAccumulator<MoveOnlyHahser>();
  accumulator.combine(0);
  EXPECT_EQ(accumulator.result().getResult(), 0);
}

TEST(DeterministicAccumulatorTest, Exceptions) {
  auto accumulator = makeDeterministicAccumulator<MoveOnlyHahser>();

  EXPECT_THROW(accumulator.endOrdered(), std::logic_error);
  EXPECT_THROW(accumulator.endUnordered(), std::logic_error);

  accumulator.beginOrdered();
  accumulator.combine(0);
  EXPECT_THROW(accumulator.endUnordered(), std::logic_error);
  EXPECT_THROW(accumulator.result(), std::logic_error);

  accumulator.beginUnordered();
  accumulator.combine(0);
  EXPECT_THROW(accumulator.endOrdered(), std::logic_error);
  EXPECT_THROW(accumulator.result(), std::logic_error);

  accumulator.endUnordered();
  accumulator.combine(0);
  EXPECT_THROW(accumulator.endUnordered(), std::logic_error);
  EXPECT_THROW(accumulator.result(), std::logic_error);

  accumulator.endOrdered();
  EXPECT_THROW(accumulator.endOrdered(), std::logic_error);
  EXPECT_THROW(accumulator.endUnordered(), std::logic_error);
  accumulator.result();
}

TEST(DeterministicAccumulatorTest, ArgumentDeduction) {
  DeterministicAccumulator accumulator([]() { return MoveOnlyHahser{}; });
  accumulator.beginOrdered();
  accumulator.combine(0);
  accumulator.endOrdered();
  accumulator.result();
}

TEST(DeterministicAccumulatorTest, ComplexArgumentDeduction) {
  DeterministicAccumulator accumulator([]() { return MoveOnlyHahser{}; });
  accumulator.beginOrdered();
  accumulator.combine(0);
  accumulator.beginUnordered();
  accumulator.combine(0);
  accumulator.beginOrdered();
  accumulator.combine(0);
  accumulator.endOrdered();
  accumulator.endUnordered();
  accumulator.endOrdered();
  accumulator.result();
}

TEST(DeterministicAccumulatorTest, UnorderedElementFinalized) {
  // Sha256Hasher throws if the hasher is not finalized.
  DeterministicAccumulator accumulator([]() { return Sha256Hasher{}; });
  accumulator.beginUnordered();
  accumulator.beginOrdered();
  accumulator.combine(1);
  accumulator.beginOrdered();
  accumulator.combine(1);
  accumulator.beginUnordered();
  accumulator.combine(1);
  accumulator.endUnordered();
  accumulator.endOrdered();
  accumulator.endOrdered();
  accumulator.endUnordered();
  accumulator.result();
}

} // namespace
} // namespace apache::thrift::op
