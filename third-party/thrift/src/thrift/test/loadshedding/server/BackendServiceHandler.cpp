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

#include <cstdlib>
#include <vector>

#include <thrift/test/loadshedding/server/BackendServiceHandler.h>

using namespace facebook::thrift::test;

BackendServiceHandler::BackendServiceHandler() {}

void BackendServiceHandler::doWork(
    BackendResponse& response, std::unique_ptr<BackendRequest> request) {
  auto wakeup = std::chrono::steady_clock::now() +
      std::chrono::microseconds(*request->time_per_request());

  if (*request->consumeCPU()) {
    auto cycle = 0;
    while (std::chrono::steady_clock::now() < wakeup) {
      cycle++;
    }
    *response.status() =
        "Consumed CPU for " + std::to_string(cycle) + " cycles.";
  } else {
    std::this_thread::sleep_for(
        std::chrono::microseconds(*request->time_per_request()));
    *response.status() =
        "Slept for " + std::to_string(*request->time_per_request()) + " ms.";
  }
  requestCount_++;
}
