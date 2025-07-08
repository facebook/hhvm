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

#include <thrift/lib/cpp2/async/processor/EventTask.h>

namespace apache::thrift {

EventTask::~EventTask() {
  expired();
}

void EventTask::expired() {
  // only expire req_ once
  if (!req_.request()) {
    return;
  }
  failWith(
      TApplicationException{"Task expired without processing"},
      kTaskExpiredErrorCode);
}

void EventTask::failWith(folly::exception_wrapper ex, std::string exCode) {
  auto cleanUp = [oneway = oneway_,
                  req = apache::thrift::detail::ServerRequestHelper::request(
                      std::move(req_)),
                  ex = std::move(ex),
                  exCode = std::move(exCode)]() mutable {
    // if oneway, skip sending back anything
    if (oneway) {
      return;
    }
    req->sendErrorWrapped(std::move(ex), std::move(exCode));
  };

  auto eb = apache::thrift::detail::ServerRequestHelper::eventBase(req_);

  if (eb->inRunningEventBaseThread()) {
    cleanUp();
  } else {
    eb->runInEventBaseThread(std::move(cleanUp));
  }
}

void EventTask::setTile(TilePtr&& tile) {
  req_.requestContext()->setTile(std::move(tile));
}

} // namespace apache::thrift
