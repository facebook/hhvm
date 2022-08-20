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

#ifndef THRIFT_TEST_PERF_CLIENTWORKER2_H_
#define THRIFT_TEST_PERF_CLIENTWORKER2_H_ 1

#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp/test/loadgen/Worker.h>
#include <thrift/perf/if/gen-cpp2/LoadTest.h>

namespace apache {
namespace thrift {

namespace test {

class ClientLoadConfig;

// Before updating for header format LoadTestClientT was specialized on
// TBinaryProtocolT<TBufferBase>, but in practice it didn't seem to
// affect the timing.

class ClientWorker2
    : public loadgen::Worker<LoadTestAsyncClient, ClientLoadConfig> {
 public:
  typedef LoadTestAsyncClient Client;
  typedef loadgen::Worker<Client, ClientLoadConfig> Parent;

  std::shared_ptr<Client> createConnection() override;
  void performOperation(
      const std::shared_ptr<Client>& client, uint32_t opType) override;

 private:
  void performNoop(const std::shared_ptr<Client>& client);
  void performOnewayNoop(const std::shared_ptr<Client>& client);
  void performAsyncNoop(const std::shared_ptr<Client>& client);
  void performSleep(const std::shared_ptr<Client>& client);
  void performOnewaySleep(const std::shared_ptr<Client>& client);
  void performBurn(const std::shared_ptr<Client>& client);
  void performOnewayBurn(const std::shared_ptr<Client>& client);
  void performBadSleep(const std::shared_ptr<Client>& client);
  void performBadBurn(const std::shared_ptr<Client>& client);
  void performThrowError(const std::shared_ptr<Client>& client);
  void performThrowUnexpected(const std::shared_ptr<Client>& client);
  void performOnewayThrow(const std::shared_ptr<Client>& client);
  void performSend(const std::shared_ptr<Client>& client);
  void performOnewaySend(const std::shared_ptr<Client>& client);
  void performRecv(const std::shared_ptr<Client>& client);
  void performSendrecv(const std::shared_ptr<Client>& client);
  void performEcho(const std::shared_ptr<Client>& client);
  void performAdd(const std::shared_ptr<Client>& client);
  void performLargeContainer(const std::shared_ptr<Client>& client);
  void performIterAllFields(const std::shared_ptr<Client>& client);

  folly::EventBaseManager ebm_;
};

} // namespace test
} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_PERF_CLIENTWORKER2_H_
