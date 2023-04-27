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
#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_layouts.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Example_types.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace apache::thrift;
using namespace apache::thrift::frozen;
using namespace apache::thrift::test;

EveryLayout stressValue2 = [] {
  EveryLayout x;
  *x.aBool() = true;
  *x.aInt() = 2;
  *x.aList() = {3, 5};
  *x.aSet() = {7, 11};
  *x.aHashSet() = {13, 17};
  *x.aMap() = {{19, 23}, {29, 31}};
  *x.aHashMap() = {{37, 41}, {43, 47}};
  x.optInt() = 53;
  *x.aFloat() = 59.61;
  x.optMap() = {{2, 4}, {3, 9}};
  return x;
}();

BENCHMARK(CompactSerialize, iters) {
  size_t s = 0;

  while (iters--) {
    std::string out;
    CompactSerializer::serialize(stressValue2, &out);
    s += out.size();
  }
  folly::doNotOptimizeAway(s);
}

BENCHMARK_RELATIVE(FrozenFreeze, iters) {
  size_t s = 0;
  folly::BenchmarkSuspender setup;
  Layout<EveryLayout> layout;
  LayoutRoot::layout(stressValue2, layout);
  setup.dismiss();
  while (iters--) {
    std::string out = freezeDataToString(stressValue2, layout);
    s += out.size();
  }
  folly::doNotOptimizeAway(s);
}

BENCHMARK_RELATIVE(FrozenFreezePreallocate, iters) {
  size_t s = 0;
  folly::BenchmarkSuspender setup;
  Layout<EveryLayout> layout;
  LayoutRoot::layout(stressValue2, layout);
  setup.dismiss();
  while (iters--) {
    std::array<byte, 1024> buffer;
    auto write = folly::MutableByteRange(buffer.begin(), buffer.end());
    ByteRangeFreezer::freeze(layout, stressValue2, write);
    s += buffer.size() - write.size();
  }
  folly::doNotOptimizeAway(s);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(CompactDeserialize, iters) {
  size_t s = 0;
  folly::BenchmarkSuspender setup;
  std::string out;
  CompactSerializer::serialize(stressValue2, &out);
  setup.dismiss();

  while (iters--) {
    EveryLayout copy;
    CompactSerializer::deserialize(out, copy);
    s += out.size();
  }
  folly::doNotOptimizeAway(s);
}

BENCHMARK_RELATIVE(FrozenThaw, iters) {
  size_t s = 0;
  folly::BenchmarkSuspender setup;
  Layout<EveryLayout> layout;
  LayoutRoot::layout(stressValue2, layout);
  std::string out = freezeDataToString(stressValue2, layout);
  setup.dismiss();
  while (iters--) {
    EveryLayout copy;
    layout.thaw(ViewPosition{reinterpret_cast<byte*>(&out[0]), 0}, copy);
    s += out.size();
  }
  folly::doNotOptimizeAway(s);
}

#if 0
============================================================================
                                                relative  time/iter  iters/s
============================================================================
CompactSerialize                                           439.75ns    2.27M
FrozenFreeze                                      21.53%     2.04us  489.59K
FrozenFreezePreallocate                           62.33%   705.48ns    1.42M
----------------------------------------------------------------------------
CompactDeserialize                                         787.59ns    1.27M
FrozenThaw                                        98.97%   795.77ns    1.26M
============================================================================
#endif
int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
