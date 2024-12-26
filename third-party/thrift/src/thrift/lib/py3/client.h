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
#include <cstdint>
#include <functional>

#include <folly/Range.h>
#include <folly/SocketAddress.h>
#include <folly/Try.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/py3/client_wrapper.h>

namespace thrift::py3 {
using RequestChannel_ptr = apache::thrift::RequestChannel::Ptr;

/*
 * T is the cpp2 async client class
 * U is the py3 clientwraper class
 */
template <class T, class U>
std::unique_ptr<ClientWrapper> makeClientWrapper(RequestChannel_ptr&& channel) {
  std::shared_ptr<apache::thrift::RequestChannel> channel_ = std::move(channel);
  auto client = std::make_unique<T>(channel_);
  return std::make_unique<U>(std::move(client), std::move(channel_));
}

inline void destroyInEventBaseThread(RequestChannel_ptr&& ptr) {
  auto eb = ptr->getEventBase();
  eb->runInEventBaseThread([ptr = std::move(ptr)] {});
}

} // namespace thrift::py3
