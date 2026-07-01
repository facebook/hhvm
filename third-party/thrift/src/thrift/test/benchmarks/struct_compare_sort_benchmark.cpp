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

#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/test/gen-cpp2/ThriftTest_types.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

#include <algorithm>
#include <compare>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace {

enum class DataShape {
  HighEntropyPrefix,
  SharedPrefix,
};

constexpr std::size_t kStructCount = 1024;
constexpr std::size_t kContainerSize = 8;

std::string randomString(std::mt19937_64& rng) {
  constexpr char kAlphabet[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::uniform_int_distribution<std::size_t> dist(0, sizeof(kAlphabet) - 2);

  std::string result;
  result.reserve(12);
  for (std::size_t i = 0; i < 12; ++i) {
    result.push_back(kAlphabet[dist(rng)]);
  }
  return result;
}

thrift::test::VersioningTestV2 makeVersioningTestV2(
    std::mt19937_64& rng, DataShape shape) {
  thrift::test::VersioningTestV2 value;
  std::uniform_int_distribution<int32_t> i32Dist(0, 1'000'000);
  std::uniform_int_distribution<int64_t> i64Dist(0, 1'000'000);
  std::uniform_real_distribution<double> doubleDist(0.0, 1'000'000.0);

  if (shape == DataShape::HighEntropyPrefix) {
    *value.begin_in_both() = i32Dist(rng);
    *value.newint() = i32Dist(rng);
    *value.newbyte() = static_cast<int8_t>(i32Dist(rng));
    *value.newshort() = static_cast<int16_t>(i32Dist(rng));
    *value.newlong() = i64Dist(rng);
    *value.newdouble() = doubleDist(rng);
  } else {
    *value.begin_in_both() = 1;
    *value.newint() = 2;
    *value.newbyte() = 3;
    *value.newshort() = 4;
    *value.newlong() = 5;
    *value.newdouble() = 6.0;
  }

  value.newstruct().emplace();
  *value.newstruct()->message() = randomString(rng);
  *value.newstruct()->type() = i32Dist(rng);

  value.newlist()->clear();
  value.newset()->clear();
  value.newmap()->clear();
  for (std::size_t i = 0; i < kContainerSize; ++i) {
    const auto random = i32Dist(rng);
    value.newlist()->push_back(random);
    value.newset()->insert(random);
    value.newmap()->emplace(random, i32Dist(rng));
  }

  *value.newunicodestring() = randomString(rng);
  *value.newstring() = randomString(rng);
  *value.newbool() = (i32Dist(rng) & 1) != 0;
  *value.end_in_both() = i32Dist(rng);
  return value;
}

std::vector<thrift::test::VersioningTestV2> makeDataSet(DataShape shape) {
  std::mt19937_64 rng(static_cast<uint64_t>(shape));
  std::vector<thrift::test::VersioningTestV2> values;
  values.reserve(kStructCount);
  for (std::size_t i = 0; i < kStructCount; ++i) {
    values.push_back(makeVersioningTestV2(rng, shape));
  }
  return values;
}

struct OldGeneratedCompareLess {
  bool operator()(
      const thrift::test::VersioningTestV2& lhs,
      const thrift::test::VersioningTestV2& rhs) const {
    if (apache::thrift::op::detail::StructEquality{}(lhs, rhs)) {
      return false;
    }
    return apache::thrift::op::detail::StructLessThan{}(lhs, rhs);
  }
};

struct OpCompareLess {
  bool operator()(
      const thrift::test::VersioningTestV2& lhs,
      const thrift::test::VersioningTestV2& rhs) const {
    return std::is_lt(
        apache::thrift::op::compare<thrift::test::VersioningTestV2>(lhs, rhs));
  }
};

struct OperatorLess {
  bool operator()(
      const thrift::test::VersioningTestV2& lhs,
      const thrift::test::VersioningTestV2& rhs) const {
    return lhs < rhs;
  }
};

template <class Less>
void sortVersioningTestV2Benchmark(std::size_t iters, DataShape shape) {
  folly::BenchmarkSuspender suspender;
  const auto input = makeDataSet(shape);
  std::vector<thrift::test::VersioningTestV2> values;

  while (iters-- > 0) {
    values = input;
    suspender.dismiss();
    std::sort(values.begin(), values.end(), Less{});
    folly::doNotOptimizeAway(values);
    suspender.rehire();
  }
}

BENCHMARK(SortVersioningTestV2OldCompare_HighEntropyPrefix, iters) {
  sortVersioningTestV2Benchmark<OldGeneratedCompareLess>(
      iters, DataShape::HighEntropyPrefix);
}

BENCHMARK_RELATIVE(SortVersioningTestV2OpCompare_HighEntropyPrefix, iters) {
  sortVersioningTestV2Benchmark<OpCompareLess>(
      iters, DataShape::HighEntropyPrefix);
}

BENCHMARK_RELATIVE(SortVersioningTestV2OperatorLess_HighEntropyPrefix, iters) {
  sortVersioningTestV2Benchmark<OperatorLess>(
      iters, DataShape::HighEntropyPrefix);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(SortVersioningTestV2OldCompare_SharedPrefix, iters) {
  sortVersioningTestV2Benchmark<OldGeneratedCompareLess>(
      iters, DataShape::SharedPrefix);
}

BENCHMARK_RELATIVE(SortVersioningTestV2OpCompare_SharedPrefix, iters) {
  sortVersioningTestV2Benchmark<OpCompareLess>(iters, DataShape::SharedPrefix);
}

BENCHMARK_RELATIVE(SortVersioningTestV2OperatorLess_SharedPrefix, iters) {
  sortVersioningTestV2Benchmark<OperatorLess>(iters, DataShape::SharedPrefix);
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
