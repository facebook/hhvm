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

#include <folly/Benchmark.h>
#include <folly/Random.h>
#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/frozen/HintTypes.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_layouts.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_types.h>

namespace apache::thrift::frozen {
namespace {

constexpr int kRows = 490;
constexpr int kCols = 51;

template <class T, class Inner = std::vector<T>>
std::vector<Inner> makeMatrix() {
  std::vector<Inner> vals;
  for (int y = 0; y < kRows; ++y) {
    vals.emplace_back();
    auto& row = vals.back();
    for (int x = 0; x < kCols; ++x) {
      row.push_back(folly::Random::rand32(12000) - 6000);
    }
  }
  return vals;
}

template <class F>
void benchmarkSum(size_t iters, const F& matrix) {
  int s = 0;
  while (iters--) {
    for (auto& row : matrix) {
      for (auto val : row) {
        s += val;
      }
    }
  }
  folly::doNotOptimizeAway(s);
}

template <class F>
void benchmarkSumCols(size_t iters, const F& matrix) {
  int s = 0;
  while (iters--) {
    for (size_t x = 0; x < kCols; ++x) {
      for (size_t y = x; y < kRows; ++y) {
        s += matrix[y][x];
      }
    }
  }
  folly::doNotOptimizeAway(s);
}

template <class F>
void benchmarkSumSavedCols(size_t iters, const F& matrix) {
  folly::BenchmarkSuspender setup;
  std::vector<typename F::value_type> rows;
  for (size_t y = 0; y < kRows; ++y) {
    rows.push_back(matrix[y]);
  }
  setup.dismiss();

  int s = 0;
  while (iters--) {
    for (size_t x = 0; x < kCols; ++x) {
      for (size_t y = x; y < kRows; ++y) {
        s += rows[y][x];
      }
    }
  }
  folly::doNotOptimizeAway(s);
}

auto vvi16 = makeMatrix<int16_t>();
auto vvi32 = makeMatrix<int32_t>();
auto vvi64 = makeMatrix<int64_t>();
auto fvvi16 = freeze(vvi16);
auto fvvi32 = freeze(vvi32);
auto fvvi64 = freeze(vvi64);
auto fuvvi16 = freeze(makeMatrix<int16_t, VectorUnpacked<int16_t>>());
auto fuvvi32 = freeze(makeMatrix<int32_t, VectorUnpacked<int32_t>>());
auto fuvvi64 = freeze(makeMatrix<int64_t, VectorUnpacked<int64_t>>());
auto vvf32 = makeMatrix<float>();
auto fvvf32 = freeze(vvf32);

BENCHMARK_PARAM(benchmarkSum, vvi16)
BENCHMARK_RELATIVE_PARAM(benchmarkSum, fvvi16)
BENCHMARK_RELATIVE_PARAM(benchmarkSum, fuvvi16)
BENCHMARK_PARAM(benchmarkSum, vvi32)
BENCHMARK_RELATIVE_PARAM(benchmarkSum, fvvi32)
BENCHMARK_RELATIVE_PARAM(benchmarkSum, fuvvi32)
BENCHMARK_PARAM(benchmarkSum, vvi64)
BENCHMARK_RELATIVE_PARAM(benchmarkSum, fvvi64)
BENCHMARK_RELATIVE_PARAM(benchmarkSum, fuvvi64)
BENCHMARK_PARAM(benchmarkSum, vvf32)
BENCHMARK_RELATIVE_PARAM(benchmarkSum, fvvf32)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(benchmarkSumCols, vvi32)
BENCHMARK_RELATIVE_PARAM(benchmarkSumCols, fvvi32)
BENCHMARK_RELATIVE_PARAM(benchmarkSumCols, fuvvi32)
BENCHMARK_RELATIVE_PARAM(benchmarkSumSavedCols, fvvi32)
BENCHMARK_RELATIVE_PARAM(benchmarkSumSavedCols, fuvvi32)

constexpr size_t kEntries = 1000000;
constexpr size_t kChunkSize = 1000;

template <typename T>
FixedSizeString<sizeof(T)> copyToFixedSizeStr(T value) {
  FixedSizeString<sizeof(T)> valueStr;
  valueStr.resize(sizeof(T));
  memcpy(&valueStr[0], reinterpret_cast<const void*>(&value), sizeof(T));
  return valueStr;
}

template <typename K>
K makeKey() {
  return folly::to<K>(folly::Random::rand32(kEntries));
}

template <>
FixedSizeString<8> makeKey<FixedSizeString<8>>() {
  return copyToFixedSizeStr(
      folly::to<int64_t>(folly::Random::rand32(kEntries)));
}

template <typename K>
const std::vector<K>& makeKeys() {
  // static to reuse storage across iterations.
  static std::vector<K> keys(kChunkSize);
  for (auto& key : keys) {
    key = makeKey<K>();
  }
  return keys;
}

template <class Map>
Map makeMap() {
  Map hist;
  using K = typename Map::key_type;
  for (size_t y = 0; y < kEntries; ++y) {
    ++hist[makeKey<K>()];
  }
  return hist;
}

template <
    typename K,
    typename V,
    template <typename, typename>
    class ContainerType>
auto convertToFixedSizeMap(const ContainerType<K, V>& map) {
  ContainerType<FixedSizeString<sizeof(K)>, FixedSizeString<sizeof(V)>> strMap;
  strMap.reserve(map.size());
  for (const auto& [key, value] : map) {
    strMap[copyToFixedSizeStr(key)] = copyToFixedSizeStr(value);
  }
  return strMap;
}

template <typename K, typename V>
using UnorderedMapType = std::unordered_map<K, V>;

template <typename K>
struct OwnedKey {
  using type = K;
};

template <>
struct OwnedKey<folly::StringPiece> {
  using type = std::string;
};

template <>
struct OwnedKey<folly::ByteRange> {
  using type = FixedSizeString<8>;
};

template <>
struct OwnedKey<
    typename detail::FixedSizeStringLayout<FixedSizeString<8>>::View> {
  using type = FixedSizeString<8>;
};

template <
    typename M,
    typename T,
    std::enable_if_t<
        !std::is_same<
            typename M::key_type,
            detail::FixedSizeStringLayout<FixedSizeString<8>>::View>::value,
        bool> = true>
typename M::const_iterator mapFind(const M& map, const T& key) {
  return map.find(key);
}

// Enabled for hashmaps with FixedSizeString as the key_type, where the
// corresponding view map would have folly::ByteRange as the key_type.
template <
    typename M,
    typename T,
    std::enable_if_t<
        std::is_same<
            typename M::key_type,
            detail::FixedSizeStringLayout<FixedSizeString<8>>::View>::value,
        bool> = true>
typename M::const_iterator mapFind(const M& map, const T& key) {
  static_assert(std::is_same<T, FixedSizeString<8>>::value);
  auto keyView = folly::ByteRange{
      reinterpret_cast<const uint8_t*>(key.data()), key.size()};
  return map.find(keyView);
}

template <class Map>
void benchmarkLookup(size_t iters, const Map& map) {
  using K = typename OwnedKey<typename Map::key_type>::type;
  int s = 0;
  for (;;) {
    folly::BenchmarkSuspender setup;
    auto& keys = makeKeys<K>();
    setup.dismiss();

    for (auto& key : keys) {
      if (iters-- == 0) {
        folly::doNotOptimizeAway(s);
        return;
      }
      auto found = mapFind(map, key);
      if (found != map.end()) {
        ++s;
      }
    }
  }
  folly::doNotOptimizeAway(s);
}

BENCHMARK_DRAW_LINE();

auto hashMap_f32 = makeMap<std::unordered_map<float, int>>();
auto hashMap_i32 = makeMap<std::unordered_map<int32_t, int>>();
auto hashMap_i64 = makeMap<std::unordered_map<int64_t, int>>();
auto hashMap_str = makeMap<std::unordered_map<std::string, int>>();
auto hashMap_fixedStr =
    convertToFixedSizeMap<int64_t, int, UnorderedMapType>(hashMap_i64);
auto frozenHashMap_f32 = freeze(hashMap_f32);
auto frozenHashMap_i32 = freeze(hashMap_i32);
auto frozenHashMap_i64 = freeze(hashMap_i64);
auto frozenHashMap_str = freeze(hashMap_str);
auto frozenHashMap_fixedStr = freeze(hashMap_fixedStr);

BENCHMARK_PARAM(benchmarkLookup, hashMap_f32)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenHashMap_f32)
BENCHMARK_PARAM(benchmarkLookup, hashMap_i32)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenHashMap_i32)
BENCHMARK_PARAM(benchmarkLookup, hashMap_i64)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenHashMap_i64)
BENCHMARK_PARAM(benchmarkLookup, hashMap_str)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenHashMap_str)
BENCHMARK_PARAM(benchmarkLookup, hashMap_fixedStr)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenHashMap_fixedStr)

BENCHMARK_DRAW_LINE();

auto map_f32 = std::map<float, int>(hashMap_f32.begin(), hashMap_f32.end());
auto map_i32 = std::map<int32_t, int>(hashMap_i32.begin(), hashMap_i32.end());
auto map_i64 = std::map<int64_t, int>(hashMap_i64.begin(), hashMap_i64.end());
auto frozenMap_f32 = freeze(map_f32);
auto frozenMap_i32 = freeze(map_i32);
auto frozenMap_i64 = freeze(map_i64);

BENCHMARK_PARAM(benchmarkLookup, map_f32)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenMap_f32)
BENCHMARK_PARAM(benchmarkLookup, map_i32)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenMap_i32)
BENCHMARK_PARAM(benchmarkLookup, map_i64)
BENCHMARK_RELATIVE_PARAM(benchmarkLookup, frozenMap_i64)

BENCHMARK_DRAW_LINE();

template <class T>
void benchmarkOldFreezeDataToString(size_t iters, const T& data) {
  const auto layout = maximumLayout<T>();
  size_t s = 0;
  while (iters--) {
    std::string out;
    out.resize(frozenSize(data, layout));
    folly::MutableByteRange writeRange(
        reinterpret_cast<byte*>(&out[0]), out.size());
    ByteRangeFreezer::freeze(layout, data, writeRange);
    out.resize(out.size() - writeRange.size());
    s += out.size();
  }
  folly::doNotOptimizeAway(s);
}

template <class T>
void benchmarkFreezeDataToString(size_t iters, const T& data) {
  const auto layout = maximumLayout<T>();
  size_t s = 0;
  while (iters--) {
    s += freezeDataToString(data, layout).size();
  }
  folly::doNotOptimizeAway(s);
}

auto strings = std::vector<std::string>{"one", "two", "three", "four", "five"};
auto tuple = std::make_pair(std::make_pair(1.3, 2.4), std::make_pair(4.5, 'x'));

BENCHMARK_PARAM(benchmarkFreezeDataToString, vvf32)
BENCHMARK_RELATIVE_PARAM(benchmarkOldFreezeDataToString, vvf32)
BENCHMARK_PARAM(benchmarkFreezeDataToString, tuple)
BENCHMARK_RELATIVE_PARAM(benchmarkOldFreezeDataToString, tuple)
BENCHMARK_PARAM(benchmarkFreezeDataToString, strings)
BENCHMARK_RELATIVE_PARAM(benchmarkOldFreezeDataToString, strings)
BENCHMARK_PARAM(benchmarkFreezeDataToString, hashMap_i32)
BENCHMARK_RELATIVE_PARAM(benchmarkOldFreezeDataToString, hashMap_i32)

#if 0
============================================================================
thrift/lib/cpp2/frozen/test/FrozenBench.cpp     relative  time/iter  iters/s
============================================================================
benchmarkSum(vvi16)                                          4.32us  231.24K
benchmarkSum(fvvi16)                               3.52%   122.88us    8.14K
benchmarkSum(fuvvi16)                             47.29%     9.15us  109.35K
benchmarkSum(vvi32)                                          3.56us  280.96K
benchmarkSum(fvvi32)                               3.18%   112.05us    8.92K
benchmarkSum(fuvvi32)                             45.79%     7.77us  128.64K
benchmarkSum(vvi64)                                          6.46us  154.76K
benchmarkSum(fvvi64)                               5.16%   125.33us    7.98K
benchmarkSum(fuvvi64)                             59.78%    10.81us   92.51K
benchmarkSum(vvf32)                                         83.51us   11.97K
benchmarkSum(fvvf32)                              99.09%    84.28us   11.87K
----------------------------------------------------------------------------
benchmarkSumCols(vvi32)                                     18.26us   54.77K
benchmarkSumCols(fvvi32)                           5.11%   357.29us    2.80K
benchmarkSumCols(fuvvi32)                          9.18%   198.89us    5.03K
benchmarkSumSavedCols(fvvi32)                     14.73%   123.94us    8.07K
benchmarkSumSavedCols(fuvvi32)                    92.12%    19.82us   50.46K
----------------------------------------------------------------------------
benchmarkLookup(hashMap_f32)                               163.78ns    6.11M
benchmarkLookup(frozenHashMap_f32)               299.82%    54.63ns   18.31M
benchmarkLookup(hashMap_i32)                                43.95ns   22.75M
benchmarkLookup(frozenHashMap_i32)                97.45%    45.10ns   22.17M
benchmarkLookup(hashMap_i64)                                61.90ns   16.15M
benchmarkLookup(frozenHashMap_i64)               144.47%    42.85ns   23.34M
benchmarkLookup(hashMap_str)                               389.26ns    2.57M
benchmarkLookup(frozenHashMap_str)               347.28%   112.09ns    8.92M
benchmarkLookup(hashMap_fixedStr)                          132.96ns    7.52M
benchmarkLookup(frozenHashMap_fixedStr)          273.92%    48.54ns   20.60M
----------------------------------------------------------------------------
benchmarkLookup(map_f32)                                   627.60ns    1.59M
benchmarkLookup(frozenMap_f32)                   265.51%   236.37ns    4.23M
benchmarkLookup(map_i32)                                   486.12ns    2.06M
benchmarkLookup(frozenMap_i32)                   146.07%   332.81ns    3.00M
benchmarkLookup(map_i64)                                   492.17ns    2.03M
benchmarkLookup(frozenMap_i64)                   126.09%   390.34ns    2.56M
----------------------------------------------------------------------------
benchmarkFreezeDataToString(vvf32)                         133.03us    7.52K
benchmarkOldFreezeDataToString(vvf32)             81.62%   162.99us    6.14K
benchmarkFreezeDataToString(tuple)                         204.38ns    4.89M
benchmarkOldFreezeDataToString(tuple)            184.05%   111.05ns    9.01M
benchmarkFreezeDataToString(strings)                       800.70ns    1.25M
benchmarkOldFreezeDataToString(strings)          215.42%   371.69ns    2.69M
benchmarkFreezeDataToString(hashMap_i32)                   115.49ms     8.66
benchmarkOldFreezeDataToString(hashMap_i32)       62.11%   185.93ms     5.38
============================================================================
#endif
} // namespace
} // namespace apache::thrift::frozen

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
