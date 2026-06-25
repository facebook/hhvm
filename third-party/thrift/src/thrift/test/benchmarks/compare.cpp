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

#include <compare>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <folly/Benchmark.h>
#include <folly/Conv.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/benchmarks/gen-cpp2/compare_types.h>
#include <thrift/test/testset/gen-cpp2/testset_types.h>

namespace apache::thrift::test {

std::mt19937 rng;
constexpr int N = 20;
MyStruct structs[N];
MyUnion unions[N];

SubSubStruct subSubStruct;
SubStruct subStruct;
SubSubStruct diffSubSubStructs[N];
SubStruct diffSubStructs[N];
MyNestedStruct nestedStructs[N];

constexpr int kTestsetPairCount = 64;
constexpr int kContainerSize = 32;

template <class T>
struct ComparePair {
  T lhs;
  T rhs;
};

std::string stringValue(int pair, int outer, int inner) {
  return folly::to<std::string>(
      "value_", pair, "_", outer, "_", inner, "_", std::string(24, 'x'));
}

ComparePair<testset::struct_list_list_string> makeListListStringPair(int pair) {
  ComparePair<testset::struct_list_list_string> result;
  result.lhs.field_1() = std::vector<std::vector<std::string>>{};
  result.rhs.field_1() = std::vector<std::vector<std::string>>{};

  auto& lhs = *result.lhs.field_1();
  auto& rhs = *result.rhs.field_1();
  lhs.reserve(kContainerSize);
  rhs.reserve(kContainerSize);
  for (int outer = 0; outer < kContainerSize; ++outer) {
    std::vector<std::string> lhsInner;
    std::vector<std::string> rhsInner;
    lhsInner.reserve(kContainerSize);
    rhsInner.reserve(kContainerSize);
    for (int inner = 0; inner < kContainerSize; ++inner) {
      auto value = stringValue(pair, outer, inner);
      lhsInner.push_back(value);
      rhsInner.push_back(std::move(value));
    }
    lhs.push_back(std::move(lhsInner));
    rhs.push_back(std::move(rhsInner));
  }
  rhs.back().back().push_back('z');
  return result;
}

ComparePair<testset::struct_map_string_set_i64> makeMapStringSetI64Pair(
    int pair) {
  ComparePair<testset::struct_map_string_set_i64> result;
  result.lhs.field_1() = std::map<std::string, std::set<int64_t>>{};
  result.rhs.field_1() = std::map<std::string, std::set<int64_t>>{};

  auto& lhs = *result.lhs.field_1();
  auto& rhs = *result.rhs.field_1();
  for (int outer = 0; outer < kContainerSize; ++outer) {
    std::set<int64_t> lhsInner;
    std::set<int64_t> rhsInner;
    for (int inner = 0; inner < kContainerSize; ++inner) {
      auto value = static_cast<int64_t>(pair * 100'000 + outer * 1'000 + inner);
      lhsInner.insert(value);
      rhsInner.insert(value);
    }
    auto key = folly::to<std::string>("key_", pair, "_", outer);
    lhs.emplace(key, lhsInner);
    rhs.emplace(std::move(key), std::move(rhsInner));
  }

  auto& lastSet = rhs.rbegin()->second;
  const auto last = *lastSet.rbegin();
  lastSet.erase(last);
  lastSet.insert(last + 1);
  return result;
}

template <class T, class MakePair>
std::vector<ComparePair<T>> makeComparePairs(MakePair makePair) {
  std::vector<ComparePair<T>> pairs;
  pairs.reserve(kTestsetPairCount);
  for (int i = 0; i < kTestsetPairCount; ++i) {
    pairs.push_back(makePair(i));
  }
  return pairs;
}

const std::vector<ComparePair<testset::struct_list_list_string>>&
listListStringPairs() {
  static const auto pairs = makeComparePairs<testset::struct_list_list_string>(
      makeListListStringPair);
  return pairs;
}

const std::vector<ComparePair<testset::struct_map_string_set_i64>>&
mapStringSetI64Pairs() {
  static const auto pairs =
      makeComparePairs<testset::struct_map_string_set_i64>(
          makeMapStringSetI64Pair);
  return pairs;
}

struct EqualThenLessCompare {
  template <class T>
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    if (op::equal<T>(lhs, rhs)) {
      return std::partial_ordering::equivalent;
    }
    if (op::less<T>(lhs, rhs)) {
      return std::partial_ordering::less;
    }
    return std::partial_ordering::greater;
  }
};

struct OpCompare {
  template <class T>
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return op::compare<T>(lhs, rhs);
  }
};

template <class Compare, class T>
void runCompareBenchmark(
    const std::vector<ComparePair<T>>& pairs, std::size_t iters) {
  std::uint64_t result = 0;
  while (iters-- > 0) {
    for (const auto& pair : pairs) {
      auto ordering = Compare{}(pair.lhs, pair.rhs);
      result += std::is_lt(ordering) ? 1 : std::is_gt(ordering) ? 2 : 3;
    }
  }
  folly::doNotOptimizeAway(result);
}

void initTestsetCompareInputs() {
  for (const auto& pair : listListStringPairs()) {
    CHECK(std::is_lt(EqualThenLessCompare{}(pair.lhs, pair.rhs)));
    CHECK(
        std::is_lt(
            op::compare<testset::struct_list_list_string>(pair.lhs, pair.rhs)));
  }
  for (const auto& pair : mapStringSetI64Pairs()) {
    CHECK(std::is_lt(EqualThenLessCompare{}(pair.lhs, pair.rhs)));
    CHECK(
        std::is_lt(
            op::compare<testset::struct_map_string_set_i64>(
                pair.lhs, pair.rhs)));
  }
}

void init() {
  subSubStruct.subsubfield_1() = 4207849484;
  subSubStruct.subsubfield_2() = std::numeric_limits<long>::max();
  subSubStruct.subsubfield_3() = -3.5650879601311513E283;
  subSubStruct.subsubfield_4() = std::numeric_limits<double>::max();
  subSubStruct.subsubfield_5() = 251;
  subSubStruct.subsubfield_6() = std::numeric_limits<int>::max();
  subSubStruct.subsubfield_7() = true;

  op::for_each_ordinal<SubStruct>(
      [&]<class Ord>(Ord) { op::get<Ord>(subStruct) = subSubStruct; });

  for (int i = 0; i < N; ++i) {
    constexpr size_t size = op::num_fields<MyStruct>;
    op::for_each_ordinal<MyStruct>([&]<class Ord>(Ord) {
      if (folly::to_underlying(Ord::value) == size) {
        // Make the last field different
        op::get<Ord>(structs[i]) = std::string(100, '0') + char(rng() % 10);
        op::get<Ord>(unions[i]) = std::string(100, '0') + char(rng() % 10);
      } else {
        op::get<Ord>(structs[i]) = std::string(100, '0');
      }
    });

    diffSubSubStructs[i].subsubfield_1() = 4207849484;
    diffSubSubStructs[i].subsubfield_2() = std::numeric_limits<long>::max();
    diffSubSubStructs[i].subsubfield_3() = -3.5650879601311513E283;
    diffSubSubStructs[i].subsubfield_4() = std::numeric_limits<double>::max();
    diffSubSubStructs[i].subsubfield_5() = 251;
    diffSubSubStructs[i].subsubfield_6() = std::numeric_limits<int>::max();
    diffSubSubStructs[i].subsubfield_7() = i % 2 == 0;

    op::for_each_ordinal<SubStruct>([&]<class Ord>(Ord) {
      if (folly::to_underlying(Ord::value) == op::num_fields<SubStruct>) {
        // Make the last field different
        op::get<Ord>(diffSubStructs[i]) = diffSubSubStructs[i];
      } else {
        op::get<Ord>(diffSubStructs[i]) = subSubStruct;
      }
    });

    op::for_each_ordinal<MyNestedStruct>([&]<class Ord>(Ord) {
      if (folly::to_underlying(Ord::value) == op::num_fields<MyNestedStruct>) {
        // Make the last field different
        op::get<Ord>(nestedStructs[i]) = diffSubStructs[i];
      } else {
        op::get<Ord>(nestedStructs[i]) = subStruct;
      }
    });
  }

  // Sanity check
  for (int i = 1; i < N; i++) {
    CHECK_EQ(
        structs[i - 1] < structs[i],
        apache::thrift::op::detail::StructLessThan{}(
            structs[i - 1], structs[i]));
    CHECK_EQ(
        unions[i - 1] < unions[i],
        apache::thrift::op::detail::UnionLessThan{}(unions[i - 1], unions[i]));
    CHECK_EQ(
        structs[i - 1] == structs[i],
        apache::thrift::op::detail::StructEquality{}(
            structs[i - 1], structs[i]));
    CHECK_EQ(
        unions[i - 1] == unions[i],
        apache::thrift::op::detail::UnionEquality{}(unions[i - 1], unions[i]));
    CHECK_EQ(
        nestedStructs[i - 1] < nestedStructs[i],
        apache::thrift::op::detail::StructLessThan{}(
            nestedStructs[i - 1], nestedStructs[i]));
    CHECK_EQ(
        nestedStructs[i - 1] == nestedStructs[i],
        apache::thrift::op::detail::StructEquality{}(
            nestedStructs[i - 1], nestedStructs[i]));
  }

  initTestsetCompareInputs();
}

BENCHMARK(StructLess) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= structs[i - 1] < structs[i];
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK_RELATIVE(StructOpLess) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= apache::thrift::op::detail::StructLessThan{}(
        structs[i - 1], structs[i]);
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK(NestedStructLess) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= nestedStructs[i - 1] < nestedStructs[i];
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK_RELATIVE(NestedStructOpLess) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= apache::thrift::op::detail::StructLessThan{}(
        nestedStructs[i - 1], nestedStructs[i]);
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK(UnionLess) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= unions[i - 1] < unions[i];
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK_RELATIVE(UnionOpLess) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= apache::thrift::op::detail::UnionLessThan{}(unions[i - 1], unions[i]);
  }
  folly::doNotOptimizeAway(b);
}

BENCHMARK(StructEqual) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= structs[i - 1] == structs[i];
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK_RELATIVE(StructOpEqual) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= apache::thrift::op::detail::StructEquality{}(
        structs[i - 1], structs[i]);
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK(NestedStructEqual) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= nestedStructs[i - 1] == nestedStructs[i];
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK_RELATIVE(NestedStructOpEqual) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= apache::thrift::op::detail::StructEquality{}(
        nestedStructs[i - 1], nestedStructs[i]);
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK(UnionEqual) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= unions[i - 1] == unions[i];
  }
  folly::doNotOptimizeAway(b);
}
BENCHMARK_RELATIVE(UnionOpEqual) {
  bool b = true;
  for (int i = 1; i < N; i++) {
    b ^= apache::thrift::op::detail::UnionEquality{}(unions[i - 1], unions[i]);
  }
  folly::doNotOptimizeAway(b);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(TestsetListListStringEqualThenLessCompare, iters) {
  runCompareBenchmark<EqualThenLessCompare>(listListStringPairs(), iters);
}
BENCHMARK_RELATIVE(TestsetListListStringOpCompare, iters) {
  runCompareBenchmark<OpCompare>(listListStringPairs(), iters);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(TestsetMapStringSetI64EqualThenLessCompare, iters) {
  runCompareBenchmark<EqualThenLessCompare>(mapStringSetI64Pairs(), iters);
}
BENCHMARK_RELATIVE(TestsetMapStringSetI64OpCompare, iters) {
  runCompareBenchmark<OpCompare>(mapStringSetI64Pairs(), iters);
}
} // namespace apache::thrift::test

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  apache::thrift::test::init();
  folly::runBenchmarks();
}
