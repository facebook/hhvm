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

#include <thrift/lib/cpp2/fast_thrift/bench/runner/BenchmarkRegistry.h>
#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientInterface.h>

#include <folly/BenchmarkUtil.h>
#include <folly/coro/Task.h>

#include <cstring>

using apache::thrift::fast_thrift::bench::Client;

namespace {

std::unique_ptr<folly::IOBuf> createPayload(size_t size) {
  auto payload = folly::IOBuf::create(size);
  payload->append(size);
  std::memset(payload->writableData(), 'X', size);
  return payload;
}

} // namespace

BENCHMARK_SUITE(Payload_64B) {
  constexpr size_t size = 64;
  auto payload = createPayload(size);
  auto resp = co_await client.echo(std::move(payload));
  folly::doNotOptimizeAway(resp);
}

BENCHMARK_SUITE(Payload_4KB) {
  constexpr size_t size = 4 * 1024;
  auto payload = createPayload(size);
  auto resp = co_await client.echo(std::move(payload));
  folly::doNotOptimizeAway(resp);
}

BENCHMARK_SUITE(Payload_1MB) {
  constexpr size_t size = 1024 * 1024;
  auto payload = createPayload(size);
  auto resp = co_await client.echo(std::move(payload));
  folly::doNotOptimizeAway(resp);
}
