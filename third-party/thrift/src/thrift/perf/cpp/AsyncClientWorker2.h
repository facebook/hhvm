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

#ifndef THRIFT_TEST_PERF_ASYNCCLIENTWORKER2_H_
#define THRIFT_TEST_PERF_ASYNCCLIENTWORKER2_H_ 1

#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <folly/ssl/SSLSession.h>
#include <thrift/lib/cpp/test/loadgen/Worker.h>
#include <thrift/perf/cpp/AsyncIntervalTimer.h>
#include <thrift/perf/cpp/ClientLoadConfig.h>
#include <thrift/perf/if/gen-cpp2/LoadTest.h>

using apache::thrift::loadgen::Worker;
using apache::thrift::test::ClientLoadConfig;

namespace apache {
namespace thrift {

// Before updating for header format LoadTestClientT was specialized on
// TBinaryProtocolT<TBufferBase>, but in practice it didn't seem to
// affect the timing.

class AsyncClientWorker2
    : public Worker<test::LoadTestAsyncClient, ClientLoadConfig> {
 public:
  AsyncClientWorker2()
      : eb_(), asyncTimer_(0), sslContext_(new folly::SSLContext()) {
    setupSSLContext();
  }

  typedef test::LoadTestAsyncClient Client;
  typedef Worker<Client, ClientLoadConfig> Parent;

  std::shared_ptr<Client> createConnection() override;
  // this is now a no-op, AsyncClientWorker::run works differently
  // from Worker::run
  void performOperation(
      const std::shared_ptr<Client>& /* client */,
      uint32_t /* opType */) override {}
  void run() override;

 private:
  void setupSSLContext();
  folly::EventBase eb_;
  AsyncIntervalTimer asyncTimer_;
  std::shared_ptr<folly::SSLContext> sslContext_;
  std::shared_ptr<folly::ssl::SSLSession> session_;
};

} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_PERF_CLIENTWORKER_H_
