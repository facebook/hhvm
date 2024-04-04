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

#ifndef THRIFT_TEST_HANDLERS_ASYNCLOADHANDLER2_H_
#define THRIFT_TEST_HANDLERS_ASYNCLOADHANDLER2_H_ 1

#include <thrift/perf/if/gen-cpp2/LoadTest.h>

namespace apache {
namespace thrift {

class AsyncLoadHandler2
    : public apache::thrift::ServiceHandler<test::LoadTest> {
 public:
  void async_eb_noop(HandlerCallback<void>::Ptr callback) override;
  void async_eb_onewayNoop(HandlerCallbackBase::Ptr callback) override;
  void async_eb_asyncNoop(HandlerCallback<void>::Ptr callback) override;
  void async_eb_sleep(
      HandlerCallback<void>::Ptr callback, int64_t microseconds) override;
  void async_eb_onewaySleep(
      HandlerCallbackBase::Ptr callback, int64_t microseconds) override;
  void sync_burn(int64_t microseconds) override;
  folly::Future<folly::Unit> future_burn(int64_t microseconds) override;
  void sync_onewayBurn(int64_t microseconds) override;
  folly::Future<folly::Unit> future_onewayBurn(int64_t microseconds) override;
  void async_eb_badSleep(
      HandlerCallback<void>::Ptr callback, int64_t microseconds) override;
  void async_eb_badBurn(
      HandlerCallback<void>::Ptr callback, int64_t microseconds) override;
  void async_eb_throwError(
      HandlerCallback<void>::Ptr callback, int32_t code) override;
  void async_eb_throwUnexpected(
      HandlerCallback<void>::Ptr callback, int32_t code) override;
  void async_eb_onewayThrow(
      HandlerCallbackBase::Ptr callback, int32_t code) override;
  void async_eb_send(
      HandlerCallback<void>::Ptr callback,
      std::unique_ptr<std::string> data) override;
  void async_eb_onewaySend(
      HandlerCallbackBase::Ptr callback,
      std::unique_ptr<std::string> data) override;
  void async_eb_recv(
      HandlerCallback<std::unique_ptr<std::string>>::Ptr callback,
      int64_t bytes) override;
  void async_eb_sendrecv(
      HandlerCallback<std::unique_ptr<std::string>>::Ptr callback,
      std::unique_ptr<std::string> data,
      int64_t recvBytes) override;
  void sync_echo(
      std::string& output, std::unique_ptr<std::string> data) override;
  folly::Future<std::unique_ptr<std::string>> future_echo(
      std::unique_ptr<std::string> data) override;
  void async_eb_add(
      HandlerCallback<int64_t>::Ptr, int64_t a, int64_t b) override;
  void async_eb_largeContainer(
      HandlerCallback<void>::Ptr callback,
      std::unique_ptr<std::vector<test::BigStruct>> items) override;

  void async_eb_iterAllFields(
      HandlerCallback<std::unique_ptr<std::vector<test::BigStruct>>>::Ptr
          callback,
      std::unique_ptr<std::vector<test::BigStruct>> items) override;
};

} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_HANDLERS_ASYNCLOADHANDLER2_H_
