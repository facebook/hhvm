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

#include <string>

#include <folly/container/F14Map.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

namespace apache::thrift::rocket {

/**
 * Abstract base class for logging used by RocketClient.
 */
class RocketClientLogger {
 public:
  virtual ~RocketClientLogger() = default;

  /**
   * Logs security policies received from the server.
   * @param policies Map of security policy names to their enforcement status.
   * For example: {{"authWall", "ENFORCED"}, {"authorization", "ENABLED"}}
   */
  virtual void logSecurityPolicies(
      const folly::F14FastMap<std::string, std::string>& policies) = 0;

  /**
   * Logs an exception with the associated transport exception type.
   * @param exception The transport exception to log.
   */
  virtual void logException(
      const apache::thrift::transport::TTransportException& exception) = 0;
};

} // namespace apache::thrift::rocket
