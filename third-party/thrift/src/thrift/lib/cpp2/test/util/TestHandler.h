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

#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>

extern const std::string kEchoSuffix;

class TestHandler
    : public apache::thrift::ServiceHandler<apache::thrift::test::TestService> {
  void sendResponse(std::string& ret, int64_t size) override;
  void noResponse(int64_t size) override;
  void voidResponse() override;

  void echoRequest(std::string& ret, std::unique_ptr<std::string> req) override;
  using StringCob =
      apache::thrift::HandlerCallback<std::unique_ptr<std::string>>;
  void async_tm_serializationTest(
      StringCob::Ptr callback, bool inEventBase) override;

  void async_eb_eventBaseAsync(StringCob::Ptr callback) override;

  void async_tm_notCalledBack(
      apache::thrift::HandlerCallbackPtr<void> cb) override;

  void echoIOBuf(
      std::unique_ptr<folly::IOBuf>& ret,
      std::unique_ptr<folly::IOBuf> buf) override;

  int32_t processHeader() override;
  void throwsHandlerException() override;
};
