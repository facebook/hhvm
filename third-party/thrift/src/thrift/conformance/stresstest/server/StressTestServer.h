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

#include <thrift/conformance/stresstest/server/StressTestHandler.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

namespace apache {
namespace thrift {
namespace stress {

std::shared_ptr<StressTestHandler> createStressTestHandler();

std::shared_ptr<ThriftServer> createStressTestServer(
    std::shared_ptr<apache::thrift::ServiceHandler<StressTest>> handler =
        nullptr);

} // namespace stress
} // namespace thrift
} // namespace apache
