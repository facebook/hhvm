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

#include <functional>
#include <memory>
#include <utility>

#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>

namespace apache::thrift::fast_thrift::thrift::server::test {

/**
 * TestServerConnection — per-accepted-client state used by fast_thrift's
 * test/bench server fixtures. Owns the thrift pipeline (which owns the
 * rocket::server::RocketServerConnection via the transport adapter).
 * Satisfies connection::Connection (close, drain, setCloseCallback) so it
 * can be returned directly from a connection factory.
 *
 * Tests/benches build the thrift pipeline themselves (handler set varies
 * per test) and assign into the fields here. The teardown semantics are
 * shared: close() tears down the pipeline → handlerRemoved propagates
 * into the transport adapter → adapter destroys the owned rocket
 * connection → close callback fires for ConnectionHandler bookkeeping.
 */
struct TestServerConnection {
  std::shared_ptr<ThriftServerChannel> serverChannel;
  channel_pipeline::SimpleBufferAllocator thriftAllocator;
  std::unique_ptr<ThriftServerTransportAdapter> transportAdapter;
  channel_pipeline::PipelineImpl::Ptr thriftPipeline;
  std::function<void()> closeCb;
  bool closed{false};

  TestServerConnection() = default;
  TestServerConnection(TestServerConnection&&) noexcept = default;
  TestServerConnection& operator=(TestServerConnection&&) noexcept = default;
  TestServerConnection(const TestServerConnection&) = delete;
  TestServerConnection& operator=(const TestServerConnection&) = delete;

  void start() noexcept {
    transportAdapter->rocketConnection().transportHandler->onConnect();
  }

  void close() noexcept {
    if (closed) {
      return;
    }
    closed = true;
    if (thriftPipeline) {
      thriftPipeline->close();
      thriftPipeline.reset();
    }
    transportAdapter.reset();
    if (closeCb) {
      auto cb = std::move(closeCb);
      cb();
    }
  }
  void drain() noexcept { close(); }
  void setCloseCallback(std::function<void()> cb) { closeCb = std::move(cb); }
};

/**
 * Type-erased connection::ConnectionFactory satisfying class that wraps
 * a std::function<TestServerConnection(socket)>. Tests bind the function
 * to a fixture-specific build callback (lambda capturing fixture state);
 * the factory delegates `getConnection` to it.
 *
 * Avoids each test having to write its own factory class.
 */
class TestServerConnectionFactory {
 public:
  using BuildFn =
      std::function<TestServerConnection(folly::AsyncTransport::UniquePtr)>;

  explicit TestServerConnectionFactory(BuildFn build) noexcept
      : build_(std::move(build)) {}

  TestServerConnection getConnection(folly::AsyncTransport::UniquePtr socket) {
    return build_(std::move(socket));
  }

 private:
  BuildFn build_;
};

} // namespace apache::thrift::fast_thrift::thrift::server::test
