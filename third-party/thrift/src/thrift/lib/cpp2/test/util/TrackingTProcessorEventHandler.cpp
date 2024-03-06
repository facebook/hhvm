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

#include <thrift/lib/cpp2/test/util/TrackingTProcessorEventHandler.h>

namespace apache::thrift::test {

void* TrackingTProcessorEventHandler::getServiceContext(
    const char* serviceName,
    const char* functionName,
    TConnectionContext* /* connectionContext */) {
  history_.emplace_back(
      fmt::format("getServiceContext('{}', '{}')", serviceName, functionName));
  return this;
}

void TrackingTProcessorEventHandler::freeContext(
    void* /* ctx */, const char* functionName) {
  history_.emplace_back(fmt::format("freeContext('{}')", functionName));
}

void TrackingTProcessorEventHandler::preRead(
    void* /* ctx */, const char* functionName) {
  history_.emplace_back(fmt::format("preRead('{}')", functionName));
}

void TrackingTProcessorEventHandler::onReadData(
    void* /* ctx */,
    const char* functionName,
    const SerializedMessage& /* msg */) {
  history_.emplace_back(fmt::format("onReadData('{}')", functionName));
}

void TrackingTProcessorEventHandler::postRead(
    void* /* ctx */,
    const char* functionName,
    apache::thrift::transport::THeader* /* header */,
    uint32_t /*bytes*/) {
  history_.emplace_back(fmt::format("postRead('{}')", functionName));
}

void TrackingTProcessorEventHandler::preWrite(
    void* /* ctx */, const char* functionName) {
  history_.emplace_back(fmt::format("preWrite('{}')", functionName));
}

void TrackingTProcessorEventHandler::onWriteData(
    void* /* ctx */,
    const char* functionName,
    const SerializedMessage& /* msg */) {
  history_.emplace_back(fmt::format("onWriteData('{}')", functionName));
}

void TrackingTProcessorEventHandler::postWrite(
    void* /* ctx */, const char* functionName, uint32_t /* bytes */) {
  history_.emplace_back(fmt::format("postWrite('{}')", functionName));
}

} // namespace apache::thrift::test
