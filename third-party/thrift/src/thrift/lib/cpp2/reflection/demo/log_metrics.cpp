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

#include <iostream>

#include <fatal/type/cat.h>
#include <folly/Conv.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/metrics_fatal_types.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

using namespace apache::thrift;
using namespace static_reflection::demo;

struct metrics_client {
  template <typename Metric, typename Value>
  void add(const Metric& name, const Value& value) {
    metrics_[name] += value;
  }

  void report() {
    for (const auto& i : metrics_) {
      std::cout << i.first << ": " << i.second << '\n';
    }
  }

 private:
  std::unordered_map<std::string, std::intmax_t> metrics_;
};

// dynamic //

struct export_metric_dynamic {
  template <typename Member, std::size_t Index, typename T>
  void operator()(
      fatal::indexed<Member, Index>,
      metrics_client& sink,
      T const& metrics,
      const std::string& prefix) const {
    const auto key = prefix + fatal::z_data<typename Member::name>();
    const auto& value = typename Member::getter{}(metrics);

    sink.add(key, value);
  }
};

std::string const kPrefix = "my_app.";

template <typename T>
void export_metrics_dynamic(metrics_client& sink, T const& metrics) {
  using info = reflect_struct<T>;

  fatal::foreach<typename info::members>(
      export_metric_dynamic(),
      sink,
      metrics,
      folly::to<std::string>(
          kPrefix, fatal::z_data<typename info::name>(), '.'));
}

// static //

struct export_metric_static {
  FATAL_S(prefix, "my_app");
  FATAL_S(dot, ".");

  template <typename Member, std::size_t Index, typename T>
  void operator()(
      fatal::indexed<Member, Index>,
      metrics_client& sink,
      T const& metrics) const {
    using info = reflect_struct<T>;
    using key = fatal::
        cat<prefix, dot, typename info::name, dot, typename Member::name>;

    const auto& value = typename Member::getter{}(metrics);
    sink.add(fatal::z_data<key>(), value);
  }
};

template <typename T>
void export_metrics_static(metrics_client& sink, T const& metrics) {
  fatal::foreach<typename reflect_struct<T>::members>(
      export_metric_static(), sink, metrics);
}

// driver //

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  metrics_client sink;

  host_info metric;
  *metric.cpu() = 1;
  *metric.memory() = 12;
  *metric.disk() = 1234;
  *metric.network_bytes_sent() = 123;
  *metric.network_bytes_received() = 321;
  *metric.network_retransmits() = 2;
  *metric.network_connection_reset() = 3;
  *metric.network_packets_dropped() = 1;

  int iterations = argc > 2 ? atoi(argv[2]) : 1000000;

  // try running this binary through `perf stat` for both versions
  if (argc > 1 && argv[1] == std::string("dynamic")) {
    std::cout << "dynamic version, " << iterations << " iterations\n";
    while (iterations--) {
      export_metrics_dynamic(sink, metric);
    }
  } else {
    std::cout << "static version, " << iterations << " iterations\n";
    while (iterations--) {
      export_metrics_static(sink, metric);
    }
  }

  sink.report();

  return 0;
}
