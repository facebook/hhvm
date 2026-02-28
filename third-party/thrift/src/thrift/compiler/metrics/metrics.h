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

#include <map>
#include <memory>
#include <thrift/compiler/metrics/metric.h>

namespace apache::thrift::compiler::detail {

// A holder class for metrics generated during a compiler run.
class metrics {
 public:
  enum class StringValue { HOSTNAME, INPUT_FILENAME, LAST };

  static std::string_view to_string(StringValue value) {
    switch (value) {
      case StringValue::HOSTNAME:
        return "hostname";
      case StringValue::INPUT_FILENAME:
        return "input_filename";
      case StringValue::LAST:
        return "StringValueLast";
    }
  }

  value<std::string>& get(StringValue value) {
    return get<detail::value<std::string>>(to_string(value));
  }

  enum class IntValue {
    RUNTIME,
    LAST,
  };

  static std::string_view to_string(IntValue value) {
    switch (value) {
      case IntValue::RUNTIME:
        return "runtime";
      case IntValue::LAST:
        return "IntValueLast";
    }
  }

  value<int>& get(IntValue value) {
    return get<detail::value<int>>(to_string(value));
  }

  enum class EventString { PARSING_PARAMS, GEN_PARAMS, SEMA_PARAMS, LAST };

  static std::string_view to_string(EventString value) {
    switch (value) {
      case EventString::PARSING_PARAMS:
        return "parsing_params";
      case EventString::GEN_PARAMS:
        return "gen_params";
      case EventString::SEMA_PARAMS:
        return "sema_params";
      case EventString::LAST:
        return "EventStringLast";
    }
  }

  event<std::string>& get(EventString value) {
    return get<detail::event<std::string>>(to_string(value));
  }

 private:
  std::map<std::string_view, std::unique_ptr<metric_base>> values_;

  template <typename T>
  T& get(std::string_view metric_name) {
    auto it = values_.find(metric_name);
    if (it == values_.end()) {
      values_[metric_name] = std::make_unique<T>();
    }
    return dynamic_cast<T&>(*values_[metric_name]);
  }
};

} // namespace apache::thrift::compiler::detail
