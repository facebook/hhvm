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

#include <thrift/lib/cpp/TProcessorEventHandler.h>

namespace apache::thrift::test {

class TrackingTProcessorEventHandler
    : public apache::thrift::TProcessorEventHandler {
 private:
  std::vector<std::string> history_;

 public:
  const std::vector<std::string> getHistory() const { return history_; }

  void* getServiceContext(
      const char* serviceName,
      const char* functionName,
      TConnectionContext* /* connectionContext */) override;

  void freeContext(void* ctx, const char* functionName) override;

  void preRead(void* ctx, const char* functionName) override;

  void onReadData(
      void* ctx,
      const char* functionName,
      const SerializedMessage& /* msg */) override;

  void postRead(
      void* ctx,
      const char* functionName,
      apache::thrift::transport::THeader* /* header */,
      uint32_t /* bytes */) override;

  void preWrite(void* ctx, const char* functionName) override;

  void onWriteData(
      void* ctx,
      const char* functionName,
      const SerializedMessage& /* msg */) override;

  void postWrite(
      void* ctx, const char* functionName, uint32_t /* bytes */) override;
};

} // namespace apache::thrift::test
