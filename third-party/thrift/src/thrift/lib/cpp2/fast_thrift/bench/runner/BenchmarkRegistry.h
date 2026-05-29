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

#pragma once

#include <folly/CPortability.h>

#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientInterface.h>

#include <folly/Preprocessor.h>
#include <folly/coro/Task.h>

#include <functional>
#include <string>
#include <vector>

namespace apache::thrift::fast_thrift::bench {

/**
 * Benchmark function type.
 * The function receives a Client reference for sending requests.
 * Returns a Task<void> to allow async operations with co_await.
 */
using BenchmarkFn = std::function<folly::coro::Task<void>(Client&)>;

/**
 * A registered benchmark with its configuration.
 */
struct BenchmarkEntry {
  std::string name;
  BenchmarkFn fn;
};

/**
 * Registry for benchmarks.
 * Benchmarks are registered against the Client base class.
 */
class BenchmarkRegistry {
 public:
  FOLLY_EXPORT static BenchmarkRegistry& instance() {
    static BenchmarkRegistry registry;
    return registry;
  }

  void registerBenchmark(const std::string& name, BenchmarkFn fn) {
    benchmarks_.push_back({name, std::move(fn)});
  }

  const std::vector<BenchmarkEntry>& benchmarks() const { return benchmarks_; }

 private:
  BenchmarkRegistry() = default;
  std::vector<BenchmarkEntry> benchmarks_;
};

/**
 * Helper for static registration of benchmarks.
 */
struct BenchmarkRegistrar {
  BenchmarkRegistrar(const std::string& name, BenchmarkFn fn) {
    BenchmarkRegistry::instance().registerBenchmark(name, std::move(fn));
  }
};

} // namespace apache::thrift::fast_thrift::bench

/**
 * Macro to define a benchmark suite.
 *
 * Usage:
 *   BENCHMARK_SUITE(Payload_64B) {
 *     co_await client.echo(...);
 *   }
 *
 * Parameters:
 *   name - Unique name for the benchmark
 */
#define BENCHMARK_SUITE(name)                                     \
  static ::folly::coro::Task<void> benchSuiteFn_##name(           \
      ::apache::thrift::fast_thrift::bench::Client& client);      \
  static ::apache::thrift::fast_thrift::bench::BenchmarkRegistrar \
      benchSuiteReg_##name(#name, benchSuiteFn_##name);           \
  static ::folly::coro::Task<void> benchSuiteFn_##name(           \
      [[maybe_unused]] ::apache::thrift::fast_thrift::bench::Client& client)
