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

#include <string>
#include <vector>

#include <glog/logging.h>

#include <gtest/gtest.h>
#include <folly/Benchmark.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ProtocolBenchmark_types.h>

namespace apache::thrift::test {

constexpr size_t kElementCount = 10000;
constexpr size_t kLargeElementCount = 10;

BenchmarkObject intStructs;
BenchmarkObject smallStringStructs;
BenchmarkObject largeStringStructs;
BenchmarkObject ints;
BenchmarkObject smallStrings;
BenchmarkObject largeStrings;

void initData() {
  intStructs.intStructs()->reserve(kElementCount);
  for (size_t i = 0; i < kElementCount; ++i) {
    IntOnly x;
    *x.x() = i;
    intStructs.intStructs()->push_back(std::move(x));
  }

  smallStringStructs.stringStructs()->reserve(kElementCount);
  for (size_t i = 0; i < kElementCount; ++i) {
    StringOnly x;
    x.x()->assign(100, 'x');
    smallStringStructs.stringStructs()->push_back(std::move(x));
  }

  largeStringStructs.stringStructs()->reserve(kLargeElementCount);
  for (size_t i = 0; i < kLargeElementCount; ++i) {
    StringOnly x;
    x.x()->assign(1 << 20, 'x');
    largeStringStructs.stringStructs()->push_back(std::move(x));
  }

  ints.ints()->reserve(kElementCount);
  for (size_t i = 0; i < kElementCount; ++i) {
    ints.ints()->push_back(i);
  }

  smallStrings.strings()->reserve(kElementCount);
  for (size_t i = 0; i < kElementCount; ++i) {
    smallStrings.strings()->emplace_back(100, 'x');
  }

  largeStrings.strings()->reserve(kLargeElementCount);
  for (size_t i = 0; i < kLargeElementCount; ++i) {
    largeStrings.strings()->emplace_back(1 << 20, 'x');
  }
}

template <class Writer>
void writerBenchmark(const BenchmarkObject& obj, int iters) {
  while (iters--) {
    folly::IOBufQueue queue;
    Writer writer;
    writer.setOutput(&queue);

    Cpp2Ops<BenchmarkObject>::write(&writer, &obj);
  }
}

#define X1(proto, kind)                      \
  BENCHMARK(proto##_##kind, n) {             \
    writerBenchmark<proto##Writer>(kind, n); \
  }

#define X(proto)                \
  X1(proto, ints)               \
  X1(proto, smallStrings)       \
  X1(proto, largeStrings)       \
  X1(proto, intStructs)         \
  X1(proto, smallStringStructs) \
  X1(proto, largeStringStructs)

X(BinaryProtocol)
X(CompactProtocol)

#undef X
#undef X1

} // namespace apache::thrift::test

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  auto ret = RUN_ALL_TESTS();
  if (!ret) {
    apache::thrift::test::initData();
    folly::runBenchmarksOnFlag();
  }
  return ret;
}
