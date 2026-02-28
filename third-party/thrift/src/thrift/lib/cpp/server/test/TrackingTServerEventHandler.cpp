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

#include <thrift/lib/cpp/server/test/TrackingTServerEventHandler.h>

#include <fmt/core.h>
#include <folly/ExceptionString.h>

namespace apache::thrift::server::test {

void TrackingTServerEventHandler::preStart(
    const folly::SocketAddress* /* address */) {
  history_.wlock()->emplace_back("preStart()");
}

void TrackingTServerEventHandler::preServe(
    const folly::SocketAddress* /* address */) {
  history_.wlock()->emplace_back("preStart()");
}

void TrackingTServerEventHandler::handleServeError(const std::exception& ex) {
  history_.wlock()->emplace_back(
      fmt::format("handleServeError('{}')", folly::exceptionStr(ex)));
}

void TrackingTServerEventHandler::newConnection(
    TConnectionContext* /* context */) {
  history_.wlock()->emplace_back("newConnection()");
}

void TrackingTServerEventHandler::connectionDestroyed(
    TConnectionContext* /* context */) {
  history_.wlock()->emplace_back("connectionDestroyed()");
}

void TrackingTServerEventHandler::postStop() {
  history_.wlock()->emplace_back("postStop()");
}

} // namespace apache::thrift::server::test
