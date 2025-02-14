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

#include <thrift/lib/cpp2/server/overload/QueueConcurrencyOverloadChecker.h>

namespace apache::thrift {
folly::Optional<OverloadResult> QueueConcurrencyOverloadChecker::checkOverload(
    const CheckOverloadParams params) {
  if (auto maxRequests = server_.getMaxRequests(); maxRequests > 0 &&
      (params.method == nullptr ||
       !server_.getMethodsBypassMaxRequestsLimit().contains(*params.method)) &&
      static_cast<uint32_t>(server_.getActiveRequests()) >= maxRequests) {
    LoadShedder loadShedder = LoadShedder::MAX_REQUESTS;
    if (auto* cpuConcurrencyController = server_.getCPUConcurrencyController();
        cpuConcurrencyController->requestShed(
            CPUConcurrencyController::Method::MAX_REQUESTS)) {
      loadShedder = LoadShedder::CPU_CONCURRENCY_CONTROLLER;
    } else if (server_.getAdaptiveConcurrencyController().enabled()) {
      loadShedder = LoadShedder::ADAPTIVE_CONCURRENCY_CONTROLLER;
    }
    return OverloadResult{
        kOverloadedErrorCode,
        "load shedding due to max request limit",
        loadShedder};
  }

  return folly::none;
}

} // namespace apache::thrift
