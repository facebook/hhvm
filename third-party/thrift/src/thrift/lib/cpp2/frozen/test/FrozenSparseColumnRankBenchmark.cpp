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

// Motivation:
//
// Sparse columnar storage stores only the non-null values of a column in a
// compact array. A parallel validity bitmap (frozen list<bool> of length N)
// records which logical row indices are non-null.
//
// To look up the value at logical row i, the consumer:
//
//   1. Checks validity[i]. If 0 (null), returns null — NO rank needed.
//   2. Otherwise computes rank(validity, i) = popcount of validity[0..i] to
//      find the physical index in the compact value array.
//
// At validity density d, only fraction d of lookups pay the rank cost. The
// expected per-lookup cost is therefore (bit_test_cost) + d * (rank_cost).
//
// Without bulk popcount support, rank(i) requires iterating validity[k]
// for k in [0, i) via operator[] — a per-bit virtual call into
// folly::Bits<uint8_t>::test, with no opportunity for hardware POPCNT.
// This makes per-lookup rank cost O(i) with a high constant factor.
//
// With ArrayLayout::View::popcount(size_t until), rank(i) becomes
// O(i / 64) word-popcount operations plus a masked partial byte. For the
// bounded N=512 case (one cache line — the natural block size for a
// "prefix sum" rank acceleration scheme), per-lookup rank cost is bounded
// by 8 std::popcount calls regardless of i.
//
// The benchmarks below model the realistic lookup shape (with null
// short-circuit). Each iteration performs 100 random lookups against the
// same fixed bitmap (same indices for both paths, same fixture, fixed seed
// for reproducibility). Three densities are exercised:
//
//   - density 0.05: 95% short-circuit — bit-test-dominated regime, where
//     speedup degrades toward 1× because almost no rank work happens.
//   - density 0.5:  50% short-circuit; 50% pay the rank.
//   - density 0.9:  only 10% short-circuit; 90% pay the rank — worst case
//     for the iterator path (most lookups walk the full prefix).
//
// Sizes:
//   - N=512:  bounded prefix-sum hot path (one cache line of bitmap).
//   - N=8K:   intermediate (shows scaling).
//   - N=100K: unoptimized hot path (where the iterator collapses).

#include <cstdint>
#include <random>
#include <vector>

#include <folly/Benchmark.h>
#include <thrift/lib/cpp2/frozen/Frozen.h>

namespace apache::thrift::frozen {
namespace {

constexpr size_t kBatch = 100;
constexpr uint32_t kSeed = 0xC0FFEEu;

std::vector<bool> makeBitmap(size_t n, double density) {
  std::mt19937 rng(kSeed);
  std::bernoulli_distribution dist(density);
  std::vector<bool> v(n);
  for (size_t i = 0; i < n; ++i) {
    v[i] = dist(rng);
  }
  return v;
}

std::vector<size_t> makeRandomIndices(size_t n, size_t batch) {
  // Use a different seed than the bitmap so the index sequence isn't
  // correlated with the bit pattern. Generate in [0, n) so all indices
  // are valid.
  std::mt19937 rng(kSeed ^ 0x9E3779B9u);
  std::uniform_int_distribution<size_t> dist(0, n - 1);
  std::vector<size_t> v(batch);
  for (size_t i = 0; i < batch; ++i) {
    v[i] = dist(rng);
  }
  return v;
}

// Slow path lookup: short-circuit on null, otherwise iterate operator[]
// for k in [0, target) to compute rank.
template <class FrozenView>
size_t lookupIterator(const FrozenView& bitmap, size_t target) {
  if (!bitmap[target]) {
    return SIZE_MAX;
  }
  size_t r = 0;
  for (size_t k = 0; k < target; ++k) {
    if (bitmap[k]) {
      ++r;
    }
  }
  return r;
}

// Fast path lookup: short-circuit on null, otherwise call frozen
// View::popcount(target) which uses std::popcount over the byte range.
template <class FrozenView>
size_t lookupPopcount(const FrozenView& bitmap, size_t target) {
  if (!bitmap[target]) {
    return SIZE_MAX;
  }
  return bitmap.popcount(target);
}

template <bool UsePopcount>
void runBench(size_t iters, size_t n, double density) {
  folly::BenchmarkSuspender setup;
  auto original = makeBitmap(n, density);
  auto frozen = freeze(original);
  auto indices = makeRandomIndices(n, kBatch);
  setup.dismiss();

  size_t sink = 0;
  while (iters--) {
    for (size_t i = 0; i < kBatch; ++i) {
      if constexpr (UsePopcount) {
        sink += lookupPopcount(frozen, indices[i]);
      } else {
        sink += lookupIterator(frozen, indices[i]);
      }
    }
  }
  folly::doNotOptimizeAway(sink);
}

// ----------------------------------------------------------------------------
// density = 0.05 — 95% of lookups short-circuit on null. Bit-test-dominated
// regime: speedup degrades toward 1× because almost no rank work happens
// on either path.
// ----------------------------------------------------------------------------

BENCHMARK(lookup_iterator_n512_d05, iters) {
  runBench<false>(iters, 512, 0.05);
}
BENCHMARK_RELATIVE(lookup_popcount_n512_d05, iters) {
  runBench<true>(iters, 512, 0.05);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(lookup_iterator_n8k_d05, iters) {
  runBench<false>(iters, 8 * 1024, 0.05);
}
BENCHMARK_RELATIVE(lookup_popcount_n8k_d05, iters) {
  runBench<true>(iters, 8 * 1024, 0.05);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(lookup_iterator_n100k_d05, iters) {
  runBench<false>(iters, 100 * 1000, 0.05);
}
BENCHMARK_RELATIVE(lookup_popcount_n100k_d05, iters) {
  runBench<true>(iters, 100 * 1000, 0.05);
}

BENCHMARK_DRAW_LINE();

// ----------------------------------------------------------------------------
// density = 0.5 — 50% of lookups short-circuit on null.
// ----------------------------------------------------------------------------

BENCHMARK(lookup_iterator_n512_d50, iters) {
  runBench<false>(iters, 512, 0.5);
}
BENCHMARK_RELATIVE(lookup_popcount_n512_d50, iters) {
  runBench<true>(iters, 512, 0.5);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(lookup_iterator_n8k_d50, iters) {
  runBench<false>(iters, 8 * 1024, 0.5);
}
BENCHMARK_RELATIVE(lookup_popcount_n8k_d50, iters) {
  runBench<true>(iters, 8 * 1024, 0.5);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(lookup_iterator_n100k_d50, iters) {
  runBench<false>(iters, 100 * 1000, 0.5);
}
BENCHMARK_RELATIVE(lookup_popcount_n100k_d50, iters) {
  runBench<true>(iters, 100 * 1000, 0.5);
}

BENCHMARK_DRAW_LINE();

// ----------------------------------------------------------------------------
// density = 0.9 — only 10% of lookups short-circuit; 90% pay the rank.
// Worst case for the iterator (most lookups walk the full prefix).
// ----------------------------------------------------------------------------

BENCHMARK(lookup_iterator_n512_d90, iters) {
  runBench<false>(iters, 512, 0.9);
}
BENCHMARK_RELATIVE(lookup_popcount_n512_d90, iters) {
  runBench<true>(iters, 512, 0.9);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(lookup_iterator_n8k_d90, iters) {
  runBench<false>(iters, 8 * 1024, 0.9);
}
BENCHMARK_RELATIVE(lookup_popcount_n8k_d90, iters) {
  runBench<true>(iters, 8 * 1024, 0.9);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(lookup_iterator_n100k_d90, iters) {
  runBench<false>(iters, 100 * 1000, 0.9);
}
BENCHMARK_RELATIVE(lookup_popcount_n100k_d90, iters) {
  runBench<true>(iters, 100 * 1000, 0.9);
}

} // namespace
} // namespace apache::thrift::frozen

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
