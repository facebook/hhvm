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

#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/Customer.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/CustomerLookupService.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/DataLoader.h>

DEFINE_int32(port, 7788, "Server port");
DEFINE_int32(io_threads, 0, "Number of IO threads (0 == number of cores)");
DEFINE_int32(cpu_threads, 0, "Number of CPU threads (0 == number of cores)");
DEFINE_int32(
    max_requests,
    -1,
    "Configures max requests, 0 will disable max request limit");

namespace facebook::sbe::test {

std::shared_ptr<CustomerLookupHandler> createCustomerLookupHandler(
    const std::string& path) {
  folly::F14FastMap<std::string, Customer> customers;
  DataLoader loader;
  loader.loadIntoMap(path, customers);
  return std::make_shared<CustomerLookupHandler>(std::move(customers));
}

std::shared_ptr<apache::thrift::ThriftServer> createCustomerLookupService(
    std::shared_ptr<apache::thrift::ServiceHandler<CustomerLookupHandler>>) {
  return nullptr;
  // if (!handler) {
  //   handler = createCustomerLookupHandler();
  // }
  // auto server = std::make_shared<ThriftServer>();
  // server->setInterface(handler);
  // server->setPort(port);
  // server->setPreferIoUring(io_threads);
  // server->setNumIOWorkerThreads(cpu_threads);
  // server->setMaxRequests(max_requests);
  // return server;
}
} // namespace facebook::sbe::test

// int main(int argc, char** argv) {
//   LOG(INFO) << "Starting CustomerLookupService";
//   folly::Init init(&argc, &argv);
//   auto CustomerLookupHandler = createCustomerLookupHandler();
//   auto service = createCustomerLookupService(CustomerLookupHandler);
//   service.start();
//   service.waitForStop();
//   LOG(INFO) << "CustomerLookupService stopped";
//   return 0;
// }
