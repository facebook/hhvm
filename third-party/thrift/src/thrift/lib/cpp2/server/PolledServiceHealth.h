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

#include <folly/Portability.h>
#include <folly/experimental/coro/Task.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift {
/**
 * An interface that service handlers should implement to dynamically
 * communicate to ThriftServer about the health of the service.
 *
 * ThriftServer will periodically poll getServiceHealth and adjust its
 * internally stored status accordingly.
 */
class PolledServiceHealth : public virtual ServiceHandlerBase {
 public:
  enum class ServiceHealth : std::uint8_t {
    // 0 is used to represent an uninitialized value
    OK = 1,
    ERROR,
  };

  /**
   * Gets the current health of the service.
   *
   * Note that a service may have multiple ServiceHandler's (for example, from a
   * custom AsyncProcessorFactory) which means that there may be multiple
   * PolledServiceHealth instances. In that case, the "most alarming"
   * ServiceHealth value is the one picked by ThriftServer.
   */
  virtual folly::coro::Task<ServiceHealth> co_getServiceHealth() = 0;
};
} // namespace apache::thrift

#endif
