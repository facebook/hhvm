/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#include <folly/init/Init.h>

#include <thrift/test/reflection/gen-cpp2/simple_reflection_fatal_types.h>
#include <thrift/test/reflection/gen-cpp2/simple_reflection_types_custom_protocol.h>

#include <thrift/lib/cpp2/reflection/populator.h>
#include <thrift/test/reflection/fatal_serialization_common.h>

#include <folly/portability/GFlags.h>

#include <random>

using namespace test_cpp2::simple_cpp_reflection;
using namespace apache::thrift;
using namespace apache::thrift::test;

using BinaryPair = RWPair<BinaryProtocolReader, BinaryProtocolWriter, false>;
using CompactPair = RWPair<CompactProtocolReader, CompactProtocolWriter, false>;

DEFINE_int32(
    seed,
    0,
    "Specify random seed to run benchmarks with. Random seed used by default");

template <typename PairType>
struct harness {
  struct7 a;
  MultiProtocolTestConcrete<PairType> rw;
  std::mt19937 gen;

  harness() : gen(std::mt19937(FLAGS_seed)) {
    populator::populator_opts opts;
    populator::populate(a, opts, gen);
  }
};

BENCHMARK(Binary_OldSerialiserWriter, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<BinaryPair> h;

    braces.dismissing([&] { //
      h.a.write(&h.rw.writer);
    });
  }
}

BENCHMARK_RELATIVE(Binary_NewSerializerWriter, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<BinaryPair> h;

    braces.dismissing([&] { //
      serializer_write(h.a, h.rw.writer);
    });
  }
}

BENCHMARK(Binary_OldSerialiserReader, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<BinaryPair> h;
    h.a.write(&h.rw.writer);
    h.rw.prep_read();

    braces.dismissing([&] { //
      h.a.read(&h.rw.reader);
    });
  }
}

BENCHMARK_RELATIVE(Binary_NewSerializerReader, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<BinaryPair> h;
    serializer_write(h.a, h.rw.writer);
    h.rw.prep_read();

    braces.dismissing([&] { //
      serializer_read(h.a, h.rw.reader);
    });
  }
}

BENCHMARK(Compact_OldSerialiserWriter, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<CompactPair> h;

    braces.dismissing([&] { //
      h.a.write(&h.rw.writer);
    });
  }
}

BENCHMARK_RELATIVE(Compact_NewSerializerWriter, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<CompactPair> h;

    braces.dismissing([&] { //
      serializer_write(h.a, h.rw.writer);
    });
  }
}

BENCHMARK(Compact_OldSerialiserReader, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<CompactPair> h;
    h.a.write(&h.rw.writer);
    h.rw.prep_read();

    braces.dismissing([&] { //
      h.a.read(&h.rw.reader);
    });
  }
}

BENCHMARK_RELATIVE(Compact_NewSerializerReader, iters) {
  folly::BenchmarkSuspender braces;

  while (iters--) {
    harness<CompactPair> h;
    serializer_write(h.a, h.rw.writer);
    h.rw.prep_read();

    braces.dismissing([&] { //
      serializer_read(h.a, h.rw.reader);
    });
  }
}

int main(int argc, char** argv) {
  std::random_device r;
  FLAGS_seed = r();
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::cout << "using seed " << FLAGS_seed << std::endl;

  folly::init(&argc, &argv, true);
  folly::runBenchmarks();

  return 0;
}
