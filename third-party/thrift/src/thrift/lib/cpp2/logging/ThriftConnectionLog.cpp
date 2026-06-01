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

#include <thrift/lib/cpp2/logging/ThriftConnectionLog.h>

namespace apache::thrift {

ThriftConnectionLog::ThriftConnectionLog(
    IThriftServerCounters* counters, IThriftRequestLogging* logging)
    : counters_(counters), logging_(logging) {}

std::unique_ptr<ThriftStreamLog> ThriftConnectionLog::createStreamLog(
    std::string_view methodName) const {
  if (!counters_ && !logging_) {
    return nullptr;
  }
  return std::make_unique<ThriftStreamLog>(methodName, counters_, logging_);
}

std::unique_ptr<ThriftSinkLog> ThriftConnectionLog::createSinkLog(
    std::string_view methodName) const {
  if (!counters_ && !logging_) {
    return nullptr;
  }
  return std::make_unique<ThriftSinkLog>(methodName, counters_, logging_);
}

std::shared_ptr<ThriftBiDiLog> ThriftConnectionLog::createBiDiLog(
    std::string_view methodName) const {
  if (!counters_ && !logging_) {
    return nullptr;
  }
  return std::make_shared<ThriftBiDiLog>(methodName, counters_, logging_);
}

} // namespace apache::thrift
