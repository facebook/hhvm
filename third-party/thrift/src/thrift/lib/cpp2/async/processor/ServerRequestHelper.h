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

#include <thrift/lib/cpp2/async/processor/ServerRequest.h>

namespace apache::thrift::detail {

class ServerRequestHelper : public ServerRequest {
 public:
  using ServerRequest::asyncProcessor;
  using ServerRequest::compressedRequest;
  using ServerRequest::eventBase;
  using ServerRequest::executor;
  using ServerRequest::internalPriority;
  using ServerRequest::moveConcurrencyControllerNotification;
  using ServerRequest::moveRequestPileNotification;
  using ServerRequest::protocol;
  using ServerRequest::queueObserverPayload;
  using ServerRequest::request;
  using ServerRequest::requestContext;
  using ServerRequest::resourcePool;
  using ServerRequest::setAsyncProcessor;
  using ServerRequest::setExecutor;
  using ServerRequest::setInternalPriority;
  using ServerRequest::setMethodMetadata;
  using ServerRequest::setResourcePool;
};

} // namespace apache::thrift::detail
