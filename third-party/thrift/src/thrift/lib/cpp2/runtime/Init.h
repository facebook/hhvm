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

#include <vector>

#include <folly/Range.h>

namespace apache::thrift {
class TProcessorEventHandler;
}

namespace apache::thrift::runtime {

struct InitOptions {
  std::vector<std::shared_ptr<apache::thrift::TProcessorEventHandler>>
      legacyClientEventHandlers;
};
void init(InitOptions);
bool wasInitialized() noexcept;

folly::Range<std::shared_ptr<apache::thrift::TProcessorEventHandler>*>
getGlobalLegacyClientEventHandlers();

} // namespace apache::thrift::runtime
