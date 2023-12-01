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

#include <thrift/lib/cpp2/transport/util/ConnectionManager.h>

#include <folly/portability/GFlags.h>

#include <folly/Singleton.h>

DEFINE_int32(
    num_client_connections,
    1,
    "Number of connections client will establish with each "
    "server (a specific address and port).");

namespace apache {
namespace thrift {

using std::string;

namespace {
folly::Singleton<ConnectionManager> factory;
}

std::shared_ptr<ConnectionManager> ConnectionManager::getInstance() {
  return factory.try_get();
}

ConnectionManager::ConnectionManager() {
  if (FLAGS_num_client_connections == 0) {
    FLAGS_num_client_connections = sysconf(_SC_NPROCESSORS_ONLN);
  }
  for (int32_t i = 0; i < FLAGS_num_client_connections; ++i) {
    threads_.push_back(std::make_unique<ConnectionThread>());
  }
  nextThreadToUse_ = 0;
}

std::shared_ptr<ClientConnectionIf> ConnectionManager::getConnection(
    const string& addr, uint16_t port) {
  ConnectionThread* thread = threads_[nextThreadToUse_].get();
  // The update of nextThreadToUse_ has race conditions, but regardless
  // its value will always be in the correct range.  The race condition
  // only causes the round-robin scheme to get a bit corrupted.
  nextThreadToUse_ = (nextThreadToUse_ + 1) % FLAGS_num_client_connections;
  return thread->getConnection(addr, port);
}

} // namespace thrift
} // namespace apache
