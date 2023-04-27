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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace test {

class MockRocketServerConnectionObserver
    : public RocketServerConnection::ManagedObserver {
 public:
  using RocketServerConnection::ManagedObserver::ManagedObserver;
  MOCK_METHOD(
      void,
      writeReady,
      (RocketServerConnection*, const WriteEvent&),
      (override));
  MOCK_METHOD(
      void,
      writeSuccess,
      (RocketServerConnection*, const WriteEvent&),
      (override));
};

} // namespace test
} // namespace rocket
} // namespace thrift
} // namespace apache
