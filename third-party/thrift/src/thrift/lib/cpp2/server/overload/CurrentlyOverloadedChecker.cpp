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

#include <thrift/lib/cpp2/server/overload/CurrentlyOverloadedChecker.h>

namespace apache::thrift {

folly::Optional<OverloadResult> CurrentlyOverloadedChecker::checkOverload(
    const CheckOverloadParams params) {
  if (UNLIKELY(
          isOverloaded_ &&
          (params.method == nullptr ||
           !config_.getMethodsBypassMaxRequestsLimit().contains(
               *params.method)) &&
          isOverloaded_(params.readHeaders, params.method))) {
    return OverloadResult{
        kAppOverloadedErrorCode,
        fmt::format(
            "Host {} is load shedding due to custom isOverloaded() callback.",
            server_.getAddressAsString()),
        LoadShedder::CUSTOM};
  }

  return folly::none;
}

} // namespace apache::thrift
