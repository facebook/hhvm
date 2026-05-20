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

/**
 * ThriftServerConnectionContextHandler Microbenchmarks
 *
 * The handler's only per-request work on the inbound path is stamping the
 * per-connection ThriftConnContext onto the request (one intrusive_ptr bump)
 * before forwarding. This benchmark guards that hot path against regressions
 * — e.g., switching the context to std::shared_ptr (atomic + heap control
 * block) or making the intrusive refcount atomic.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

#include <boost/intrusive_ptr.hpp>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionContextHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::thrift;

namespace {

using apache::thrift::fast_thrift::rocket::bench::BenchContext;

ThriftServerRequestMessage makeRequest(uint32_t streamId) {
  ThriftServerRequestMessage req;
  req.streamId = streamId;
  return req;
}

BENCHMARK(Read_StampConnContext, iters) {
  BenchmarkSuspender suspender;

  boost::intrusive_ptr<ThriftConnContext> connContext{new ThriftConnContext()};
  ThriftServerConnectionContextHandler<BenchContext> handler{connContext};
  BenchContext ctx;

  std::vector<TypeErasedBox> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(erase_and_box(makeRequest(static_cast<uint32_t>(i))));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, std::move(requests[i]));
    doNotOptimizeAway(result);
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
