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

#include <thrift/lib/cpp2/transport/http2/common/testutil/FakeResponseHandler.h>

#include <glog/logging.h>
#include <folly/portability/GTest.h>

namespace apache {
namespace thrift {

using folly::IOBuf;
using proxygen::HTTPMessage;
using std::string;
using std::unordered_map;
using namespace testing;

FakeResponseHandler::FakeResponseHandler(folly::EventBase* evb)
    : evb_(getKeepAliveToken(evb)),
      txn_(
          proxygen::TransportDirection::UPSTREAM,
          proxygen::HTTPCodec::StreamID(1),
          0,
          dummyEgressQueue_) {
  EXPECT_CALL(txn_, sendHeaders(testing::_))
      .WillRepeatedly(Invoke([this](const HTTPMessage& msg) mutable {
        EXPECT_TRUE(evb_->inRunningEventBaseThread());
        auto copyHeaders = [&](const string& key, const string& val) {
          headers_.insert(make_pair(key, val));
        };
        msg.getHeaders().forEach(copyHeaders);
      }));
  EXPECT_CALL(txn_, sendBody(testing::_))
      .WillRepeatedly(Invoke([this](std::shared_ptr<IOBuf> body) mutable {
        EXPECT_TRUE(evb_->inRunningEventBaseThread());
        if (body_) {
          body_->prependChain(body->clone());
        } else {
          body_ = body->clone();
        }
      }));
  EXPECT_CALL(txn_, sendEOM()).WillRepeatedly(Invoke([this] {
    EXPECT_TRUE(evb_->inRunningEventBaseThread());
    eomReceived_ = true;
    // Tests that use this class are expected to be done at this point.
    evb_.reset();
  }));
}

unordered_map<string, string>* FakeResponseHandler::getHeaders() {
  return &headers_;
}

folly::IOBuf* FakeResponseHandler::getBodyBuf() {
  return body_.get();
}

string FakeResponseHandler::getBody() {
  if (body_) {
    // Clone so we do not destroy the IOBuf - just in case.
    return body_->clone()->moveToFbString().toStdString();
  } else {
    return "";
  }
}

bool FakeResponseHandler::eomReceived() {
  return eomReceived_;
}

} // namespace thrift
} // namespace apache
