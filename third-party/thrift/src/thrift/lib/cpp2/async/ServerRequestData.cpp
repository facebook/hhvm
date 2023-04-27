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

#include <thrift/lib/cpp2/async/ServerRequestData.h>

namespace apache::thrift {

void ServerRequestData::setRequestExecutionBegin(TimePoint now) {
  requestExecutionBegin = now;
}

void ServerRequestData::setRequestExecutionEnd(TimePoint now) {
  requestExecutionEnd = now;
}

ServerRequestData::Duration ServerRequestData::queuedDuration() const {
  return requestExecutionBegin - queueBegin;
}
ServerRequestData::Duration ServerRequestData::requestExecutionDuration()
    const {
  return requestExecutionEnd - requestExecutionBegin;
}
} // namespace apache::thrift
