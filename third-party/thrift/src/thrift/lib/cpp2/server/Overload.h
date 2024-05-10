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

#include <string>

#pragma once

namespace apache::thrift {

enum class LoadShedder : uint8_t {
  CUSTOM,
  MAX_REQUESTS,
  MAX_QPS,
  CPU_CONCURRENCY_CONTROLLER,
  ADAPTIVE_CONCURRENCY_CONTROLLER,
};

struct OverloadResult {
  std::string errorCode;
  std::string errorMessage;
  LoadShedder loadShedder;
};

} // namespace apache::thrift
