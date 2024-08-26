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

namespace apache::thrift {

class StreamMetricCallback {
 public:
  virtual ~StreamMetricCallback() = default;
  virtual void onFirstResponse(std::string_view /* methodName */) = 0;
  virtual void onFirstResponseError(std::string_view /* methodName */) = 0;
  virtual void onStreamNext(std::string_view /* methodName */) = 0;
  virtual void onStreamError(std::string_view /* methodName */) = 0;
  virtual void onStreamComplete(std::string_view /* methodName */) = 0;
  virtual void onStreamCancel(std::string_view /* methodName */) = 0;
  virtual void onStreamRequestN(
      std::string_view /* methodName */, uint32_t /* credits */) = 0;
};

class NoopStreamMetricCallback final : public StreamMetricCallback {
  void onFirstResponse(std::string_view) override {}
  void onFirstResponseError(std::string_view) override {}
  void onStreamNext(std::string_view) override {}
  void onStreamError(std::string_view) override {}
  void onStreamComplete(std::string_view) override {}
  void onStreamCancel(std::string_view) override {}
  void onStreamRequestN(std::string_view, uint32_t) override {}
};

} // namespace apache::thrift
