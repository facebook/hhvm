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

#include <string_view>

#include <folly/sorted_vector_types.h>

#include <thrift/lib/cpp2/PluggableFunction.h>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    const folly::sorted_vector_set<std::string_view>&,
    getMonitoringMethodNames);
}

/**
 * Returns the names of monitoring methods known to the server.
 * For example, those defined in fb303:
 *   https://github.com/facebook/fb303/blob/053b0bec7c3d8a17b6de1d18b91f59f88d93b060/fb303/thrift/fb303_core.thrift#L45
 */
const folly::sorted_vector_set<std::string_view>& getMonitoringMethodNames();

inline bool isMonitoringMethodName(std::string_view methodName) {
  return getMonitoringMethodNames().count(methodName) > 0;
}

} // namespace apache::thrift
