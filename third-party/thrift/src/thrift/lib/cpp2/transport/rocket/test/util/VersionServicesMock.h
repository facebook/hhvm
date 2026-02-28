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

#include <gmock/gmock.h>

#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/gen-cpp2/NewVersion.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/gen-cpp2/OldVersion.h>

namespace testutil::testservice {

class OldServiceMock : public apache::thrift::ServiceHandler<OldVersion> {
 public:
  OldServiceMock() {}

  int32_t AddOne(int32_t i) override { return i + 1; }

  void DeletedMethod() override {}

  apache::thrift::ServerStream<Message> DeletedStreamMethod() override {
    return apache::thrift::ServerStream<Message>::createEmpty();
  }

  apache::thrift::ResponseAndServerStream<Message, Message>
  DeletedResponseAndStreamMethod() override {
    return {{}, apache::thrift::ServerStream<Message>::createEmpty()};
  }

  apache::thrift::ServerStream<int32_t> Range(
      int32_t from, int32_t length) override {
    auto [stream, publisher] =
        apache::thrift::ServerStream<int32_t>::createPublisher();
    for (int i = from; i < from + length; ++i) {
      publisher.next(i);
    }
    std::move(publisher).complete();
    return std::move(stream);
  }

  apache::thrift::ResponseAndServerStream<int32_t, int32_t> RangeAndAddOne(
      int32_t from, int32_t length, int32_t number) override {
    return {number + 1, Range(from, length)};
  }

  apache::thrift::ServerStream<Message> StreamToRequestResponse() override {
    return apache::thrift::ServerStream<Message>::createEmpty();
  }

  apache::thrift::ResponseAndServerStream<Message, Message>
  ResponseandStreamToRequestResponse() override {
    Message response;
    *response.message() = "Message";
    response.message().ensure();
    return {
        std::move(response),
        apache::thrift::ServerStream<Message>::createEmpty()};
  }

  void RequestResponseToStream(Message& response) override {
    *response.message() = "Message";
    response.message().ensure();
  }

  void RequestResponseToResponseandStream(Message& response) override {
    *response.message() = "Message";
    response.message().ensure();
  }

 protected:
  folly::ScopedEventBaseThread executor_;
};

class NewServiceMock : public apache::thrift::ServiceHandler<NewVersion> {
 public:
  NewServiceMock() {}

  int32_t AddOne(int32_t i) override { return i + 1; }

  apache::thrift::ServerStream<int32_t> Range(
      int32_t from, int32_t length) override {
    auto [stream, publisher] =
        apache::thrift::ServerStream<int32_t>::createPublisher();
    for (int i = from; i < from + length; ++i) {
      publisher.next(i);
    }
    std::move(publisher).complete();
    return std::move(stream);
  }

  apache::thrift::ResponseAndServerStream<int32_t, int32_t> RangeAndAddOne(
      int32_t from, int32_t length, int32_t number) override {
    return {number + 1, Range(from, length)};
  }

 protected:
  void StreamToRequestResponse() override {
    LOG(DFATAL) << "StreamToRequestResponse should not be executed";
  }

  void ResponseandStreamToRequestResponse() override {
    LOG(DFATAL) << "ResponseandStreamToRequestResponse should not be executed";
  }

  apache::thrift::ServerStream<Message> RequestResponseToStream() override {
    LOG(DFATAL) << "RequestResponseToStream should not be executed";

    return apache::thrift::ServerStream<Message>::createEmpty();
  }

  apache::thrift::ResponseAndServerStream<Message, Message>
  RequestResponseToResponseandStream() override {
    LOG(DFATAL) << "RequestResponseToStream should not be executed";

    return {Message{}, apache::thrift::ServerStream<Message>::createEmpty()};
  }

 protected:
  folly::ScopedEventBaseThread executor_;
};
} // namespace testutil::testservice
