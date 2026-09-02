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

// Deserializes a `list<string>` of 32-character names into an allocator-aware
// Thrift struct, and into a plain one from the same bytes for reference. Every
// element goes through emplace_back_default, so per-element construction cost
// shows up here.

#include <cstddef>
#include <memory>
#include <memory_resource>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/benchmarks/gen-cpp2/AllocatorEmplaceBenchData_types.h>

namespace {

namespace data = ::thrift::benchmark::allocator_emplace;

// Past the small-string buffer, so an element is a real allocation.
constexpr std::size_t kNameLen = 32;

// Both structs have the same wire layout, so one buffer feeds either.
std::unique_ptr<folly::IOBuf> serializedNames(std::size_t count) {
  data::Names golden;
  for (std::size_t i = 0; i < count; ++i) {
    golden.names()->emplace_back(kNameLen, 'x');
  }
  folly::IOBufQueue q;
  apache::thrift::CompactSerializer::serialize(golden, &q);
  auto buf = q.move();
  buf->coalesce();
  return buf;
}

struct PlainTarget {
  data::Names value;
};

// The arena is a member so it outlives `value`. Default-constructing the struct
// instead would silently fall back to the new_delete resource and measure pmr
// indirection with none of its benefit.
struct ArenaTarget {
  std::pmr::monotonic_buffer_resource arena;
  data::AllocatorAwareNames value{
      std::pmr::polymorphic_allocator<std::byte>{&arena}};
};

template <typename Target>
void runDeserialize(unsigned iters, std::size_t count) {
  std::unique_ptr<folly::IOBuf> bytes;
  BENCHMARK_SUSPEND {
    bytes = serializedNames(count);
  }
  while (iters--) {
    Target target;
    apache::thrift::CompactSerializer::deserialize(bytes.get(), target.value);
    folly::doNotOptimizeAway(target.value);
  }
}

void deserializePlain(unsigned iters, std::size_t count) {
  runDeserialize<PlainTarget>(iters, count);
}

void deserializeAllocatorAware(unsigned iters, std::size_t count) {
  runDeserialize<ArenaTarget>(iters, count);
}

BENCHMARK_PARAM(deserializePlain, 64)
BENCHMARK_RELATIVE_PARAM(deserializeAllocatorAware, 64)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(deserializePlain, 512)
BENCHMARK_RELATIVE_PARAM(deserializeAllocatorAware, 512)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(deserializePlain, 4096)
BENCHMARK_RELATIVE_PARAM(deserializeAllocatorAware, 4096)
BENCHMARK_DRAW_LINE();
BENCHMARK_PARAM(deserializePlain, 8192)
BENCHMARK_RELATIVE_PARAM(deserializeAllocatorAware, 8192)

} // namespace

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
