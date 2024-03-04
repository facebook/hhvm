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

#include <thrift/lib/cpp/util/VarintUtils.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/util/test/VarintUtilsTestUtil.h>

using namespace apache::thrift::util;

FOLLY_CREATE_QUAL_INVOKER_SUITE(write_unrolled, writeVarintUnrolled);
#ifdef __BMI2__
FOLLY_CREATE_QUAL_INVOKER_SUITE(write_bmi2, writeVarintBMI2);
#endif

template <typename Case, typename Fn>
void bench_write(size_t iters, Fn fn, Case) {
  folly::BenchmarkSuspender braces;
  folly::IOBufQueue iobufQueue(folly::IOBufQueue::cacheChainLength());
  auto ints = Case::gen();
  while (iters--) {
    constexpr size_t kDesiredGrowth = 1 << 14;
    folly::io::QueueAppender c(&iobufQueue, kDesiredGrowth);
    c.ensure(ints.size() * 10);
    braces.dismissing([&] {
      for (auto v : ints) {
        fn(c, v);
      }
    });
    iobufQueue.clearAndTryReuseLargestBuffer();
  }
}

#define BM_WRITE_LOOP(kind) \
  BENCHMARK_NAMED_PARAM(bench_write, kind##_unrolled, write_unrolled, kind())

#ifdef __BMI2__
#define BM_REL_WRITE_BMI2(kind) \
  BENCHMARK_RELATIVE_NAMED_PARAM(bench_write, kind##_bmi2, write_bmi2, kind())

#define BM_WRITE(kind) \
  BM_WRITE_LOOP(kind)  \
  BM_REL_WRITE_BMI2(kind)
#else
#define BM_WRITE(kind) BM_WRITE_LOOP(kind)
#endif

BM_WRITE(u8_any)
BM_WRITE(u16_any)
BM_WRITE(u32_any)
BM_WRITE(u64_any)

BM_WRITE(u8_1b)
BM_WRITE(u8_2b)

BM_WRITE(u16_1b)
BM_WRITE(u16_2b)
BM_WRITE(u16_3b)

BM_WRITE(u32_1b)
BM_WRITE(u32_2b)
BM_WRITE(u32_3b)
BM_WRITE(u32_4b)
BM_WRITE(u32_5b)

BM_WRITE(u64_1b)
BM_WRITE(u64_2b)
BM_WRITE(u64_3b)
BM_WRITE(u64_4b)
BM_WRITE(u64_5b)
BM_WRITE(u64_6b)
BM_WRITE(u64_7b)
BM_WRITE(u64_8b)
BM_WRITE(u64_9b)
BM_WRITE(u64_10b)

template <typename Case>
void bench_read(size_t iters, Case) {
  folly::IOBufQueue iobufQueue(folly::IOBufQueue::cacheChainLength());
  const size_t kDesiredGrowth = sysconf(_SC_PAGE_SIZE);
  folly::io::QueueAppender c(&iobufQueue, kDesiredGrowth);
  BENCHMARK_SUSPEND {
    auto ints = Case::gen();
    c.ensure(ints.size() * 10);
    for (auto v : ints) {
#ifdef __BMI2__
      writeVarintBMI2(c, v);
#else
      writeVarintUnrolled(c, v);
#endif
    }
  }
  folly::io::Cursor rcursor(iobufQueue.front());
  while (iters--) {
    typename Case::integer_type val = 0;
    if (rcursor.isAtEnd()) {
      rcursor = folly::io::Cursor(iobufQueue.front());
    }
    readVarint(rcursor, val);
    folly::doNotOptimizeAway(val);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(bench_read, u8_any, u8_any())
BENCHMARK_NAMED_PARAM(bench_read, u8_1b, u8_1b())
BENCHMARK_NAMED_PARAM(bench_read, u8_2b, u8_2b())

BENCHMARK_NAMED_PARAM(bench_read, u16_any, u16_any())
BENCHMARK_NAMED_PARAM(bench_read, u16_1b, u16_1b())
BENCHMARK_NAMED_PARAM(bench_read, u16_2b, u16_2b())
BENCHMARK_NAMED_PARAM(bench_read, u16_3b, u16_3b())

BENCHMARK_NAMED_PARAM(bench_read, u32_any, u32_any())
BENCHMARK_NAMED_PARAM(bench_read, u32_1b, u32_1b())
BENCHMARK_NAMED_PARAM(bench_read, u32_2b, u32_2b())
BENCHMARK_NAMED_PARAM(bench_read, u32_3b, u32_3b())
BENCHMARK_NAMED_PARAM(bench_read, u32_4b, u32_4b())
BENCHMARK_NAMED_PARAM(bench_read, u32_5b, u32_5b())

BENCHMARK_NAMED_PARAM(bench_read, u64_any, u64_any())
BENCHMARK_NAMED_PARAM(bench_read, u64_1b, u64_1b())
BENCHMARK_NAMED_PARAM(bench_read, u64_2b, u64_2b())
BENCHMARK_NAMED_PARAM(bench_read, u64_3b, u64_3b())
BENCHMARK_NAMED_PARAM(bench_read, u64_4b, u64_4b())
BENCHMARK_NAMED_PARAM(bench_read, u64_5b, u64_5b())
BENCHMARK_NAMED_PARAM(bench_read, u64_6b, u64_6b())
BENCHMARK_NAMED_PARAM(bench_read, u64_7b, u64_7b())
BENCHMARK_NAMED_PARAM(bench_read, u64_8b, u64_8b())
BENCHMARK_NAMED_PARAM(bench_read, u64_9b, u64_9b())
BENCHMARK_NAMED_PARAM(bench_read, u64_10b, u64_10b())

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv, true);
  folly::runBenchmarks();
}

#if 0
$ buck run @mode/opt-clang-thinlto --config=cxx.use_default_autofdo_profile=false \
    //thrift/lib/cpp/util/test:varint_utils_bench -- --bm_min_iters=1000000
============================================================================
[...]ib/cpp/util/test/VarintUtilsBench.cpp     relative  time/iter   iters/s
============================================================================
bench_write(u8_any_unrolled)                                2.14us   466.65K
bench_write(u8_any_bmi2)                        140.85%     1.52us   657.30K
bench_write(u16_any_unrolled)                               4.50us   222.06K
bench_write(u16_any_bmi2)                       278.76%     1.62us   619.01K
bench_write(u32_any_unrolled)                               4.12us   242.54K
bench_write(u32_any_bmi2)                       260.58%     1.58us   632.02K
bench_write(u64_any_unrolled)                               9.45us   105.82K
bench_write(u64_any_bmi2)                       480.55%     1.97us   508.53K
bench_write(u8_1b_unrolled)                                 1.36us   736.86K
bench_write(u8_1b_bmi2)                         104.09%     1.30us   767.00K
bench_write(u8_2b_unrolled)                                 2.09us   478.46K
bench_write(u8_2b_bmi2)                         127.52%     1.64us   610.10K
bench_write(u16_1b_unrolled)                                1.40us   713.68K
bench_write(u16_1b_bmi2)                        97.217%     1.44us   693.82K
bench_write(u16_2b_unrolled)                                2.38us   420.66K
bench_write(u16_2b_bmi2)                        118.56%     2.01us   498.72K
bench_write(u16_3b_unrolled)                                2.48us   403.41K
bench_write(u16_3b_bmi2)                        161.58%     1.53us   651.82K
bench_write(u32_1b_unrolled)                                1.31us   760.87K
bench_write(u32_1b_bmi2)                        96.028%     1.37us   730.65K
bench_write(u32_2b_unrolled)                                2.51us   398.21K
bench_write(u32_2b_bmi2)                        162.89%     1.54us   648.65K
bench_write(u32_3b_unrolled)                                3.12us   321.02K
bench_write(u32_3b_bmi2)                        203.37%     1.53us   652.85K
bench_write(u32_4b_unrolled)                                3.16us   315.97K
bench_write(u32_4b_bmi2)                        205.97%     1.54us   650.80K
bench_write(u32_5b_unrolled)                                2.98us   335.28K
bench_write(u32_5b_bmi2)                        178.06%     1.68us   596.99K
bench_write(u64_1b_unrolled)                                1.34us   745.74K
bench_write(u64_1b_bmi2)                        99.655%     1.35us   743.17K
bench_write(u64_2b_unrolled)                                3.19us   313.53K
bench_write(u64_2b_bmi2)                        175.14%     1.82us   549.12K
bench_write(u64_3b_unrolled)                                3.39us   294.73K
bench_write(u64_3b_bmi2)                        179.66%     1.89us   529.52K
bench_write(u64_4b_unrolled)                                4.42us   226.01K
bench_write(u64_4b_bmi2)                        234.87%     1.88us   530.82K
bench_write(u64_5b_unrolled)                                4.51us   221.88K
bench_write(u64_5b_bmi2)                        238.21%     1.89us   528.54K
bench_write(u64_6b_unrolled)                                4.96us   201.47K
bench_write(u64_6b_bmi2)                        266.08%     1.87us   536.07K
bench_write(u64_7b_unrolled)                                5.83us   171.60K
bench_write(u64_7b_bmi2)                        298.78%     1.95us   512.70K
bench_write(u64_8b_unrolled)                                6.07us   164.76K
bench_write(u64_8b_bmi2)                        326.97%     1.86us   538.73K
bench_write(u64_9b_unrolled)                                6.09us   164.13K
bench_write(u64_9b_bmi2)                        319.45%     1.91us   524.31K
bench_write(u64_10b_unrolled)                               6.38us   156.72K
bench_write(u64_10b_bmi2)                       323.96%     1.97us   507.70K
----------------------------------------------------------------------------
bench_read(u8_any)                                          5.35ns   187.05M
bench_read(u8_1b)                                           1.10ns   909.75M
bench_read(u8_2b)                                           1.63ns   614.17M
bench_read(u16_any)                                         3.69ns   271.01M
bench_read(u16_1b)                                          1.06ns   940.09M
bench_read(u16_2b)                                          2.19ns   456.05M
bench_read(u16_3b)                                          1.87ns   534.97M
bench_read(u32_any)                                         4.84ns   206.57M
bench_read(u32_1b)                                          2.41ns   414.36M
bench_read(u32_2b)                                          3.25ns   308.15M
bench_read(u32_3b)                                          3.50ns   285.52M
bench_read(u32_4b)                                          3.78ns   264.38M
bench_read(u32_5b)                                          4.36ns   229.21M
bench_read(u64_any)                                         4.80ns   208.21M
bench_read(u64_1b)                                          1.02ns   985.09M
bench_read(u64_2b)                                          1.50ns   666.79M
bench_read(u64_3b)                                          4.78ns   209.16M
bench_read(u64_4b)                                          4.81ns   207.78M
bench_read(u64_5b)                                          4.78ns   209.00M
bench_read(u64_6b)                                          4.78ns   209.27M
bench_read(u64_7b)                                          4.78ns   209.35M
bench_read(u64_8b)                                          5.07ns   197.39M
bench_read(u64_9b)                                          5.17ns   193.38M
bench_read(u64_10b)                                         5.13ns   194.97M
#endif
