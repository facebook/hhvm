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

#include <thrift/lib/cpp2/server/MonitoringMethodNames.h>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    const folly::sorted_vector_set<std::string_view>&,
    getMonitoringMethodNames) {
  static const auto& kEmpty = *new folly::sorted_vector_set<std::string_view>{};
  return kEmpty;
}
} // namespace detail

const folly::sorted_vector_set<std::string_view>& getMonitoringMethodNames() {
  static auto& methodNames = detail::getMonitoringMethodNames();
  return methodNames;
}

} // namespace apache::thrift
