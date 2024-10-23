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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache::thrift {

class MockCallback : public RequestCallback {
 public:
  MockCallback(bool clientError, bool serverError)
      : clientError_(clientError), serverError_(serverError) {}
  ~MockCallback() override { EXPECT_TRUE(callbackReceived_); }

  void requestSent() override {}

  void replyReceived(ClientReceiveState&& crs) override {
    EXPECT_FALSE(crs.isException());
    EXPECT_FALSE(callbackReceived_);
    EXPECT_FALSE(clientError_);
    auto reply =
        crs.serializedResponse().buffer->cloneAsValue().moveToFbString();
    bool expired = (reply.find("Task expired") != folly::fbstring::npos) ||
        (reply.find("Queue Timeout") != folly::fbstring::npos);
    EXPECT_EQ(serverError_, expired);
    callbackReceived_ = true;
  }
  void requestError(ClientReceiveState&& crs) override {
    EXPECT_TRUE(crs.isException());
    EXPECT_TRUE(
        crs.exception().is_compatible_with<transport::TTransportException>());
    EXPECT_FALSE(callbackReceived_);
    EXPECT_TRUE(clientError_ || serverError_);
    callbackReceived_ = true;
  }
  bool callbackReceived() { return callbackReceived_; }

 private:
  bool clientError_;
  bool serverError_;
  bool callbackReceived_{false};
};
} // namespace apache::thrift
