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

#include <memory>
#include <string_view>

#include <thrift/lib/cpp2/logging/IThriftRequestLogging.h>
#include <thrift/lib/cpp2/logging/IThriftServerCounters.h>
#include <thrift/lib/cpp2/logging/ThriftBiDiLog.h>
#include <thrift/lib/cpp2/logging/ThriftSinkLog.h>
#include <thrift/lib/cpp2/logging/ThriftStreamLog.h>

namespace apache::thrift {

/**
 * Per-connection factory that creates stream/sink logs with injected backends.
 *
 * Stored on Cpp2ConnContext for the lifetime of the connection. Each stream
 * or sink request creates its own ThriftStreamLog / ThriftSinkLog via the
 * factory methods, inheriting the connection-level backend references.
 */
class ThriftConnectionLog {
 public:
  ThriftConnectionLog(
      IThriftServerCounters* counters, IThriftRequestLogging* logging);

  /**
   * Create a new stream log for the given method.
   */
  std::unique_ptr<ThriftStreamLog> createStreamLog(
      std::string_view methodName) const;

  /**
   * Create a new sink log for the given method.
   */
  std::unique_ptr<ThriftSinkLog> createSinkLog(
      std::string_view methodName) const;

  /**
   * Create a new bidi log for the given method.
   * Returns shared_ptr because the bidi log is shared across bridges + stapler.
   */
  std::shared_ptr<ThriftBiDiLog> createBiDiLog(
      std::string_view methodName) const;

 private:
  IThriftServerCounters* counters_;
  IThriftRequestLogging* logging_;
};

} // namespace apache::thrift
