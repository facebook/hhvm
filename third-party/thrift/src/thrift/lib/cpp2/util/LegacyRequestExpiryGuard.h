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

#include <thrift/lib/cpp2/async/ResponseChannel.h>

namespace apache::thrift {

struct LegacyRequestExpiryGuard {
  apache::thrift::ResponseChannelRequest::UniquePtr req;
  folly::EventBase* eb;

  LegacyRequestExpiryGuard(
      apache::thrift::ResponseChannelRequest::UniquePtr r, folly::EventBase* e)
      : req{std::move(r)}, eb{e} {}
  LegacyRequestExpiryGuard(LegacyRequestExpiryGuard&&) = default;
  LegacyRequestExpiryGuard& operator=(LegacyRequestExpiryGuard&& other) {
    if (this != &other) {
      expire();
    }
    req = std::move(other.req);
    eb = other.eb;
    return *this;
  }
  ~LegacyRequestExpiryGuard() { expire(); }

 private:
  void expire() {
    if (req) {
      eb->runInEventBaseThread([req = std::move(req)] {
        req->sendErrorWrapped(
            apache::thrift::TApplicationException{
                "Task expired without processing"},
            kTaskExpiredErrorCode);
      });
    }
  }
};

} // namespace apache::thrift
