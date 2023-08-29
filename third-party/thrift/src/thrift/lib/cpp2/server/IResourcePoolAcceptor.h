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

#include <optional>

namespace folly {
class Executor;
}

namespace apache::thrift {

class ServerRequestRejection;
class ServerRequest;

class IResourcePoolAcceptor {
 public:
  virtual ~IResourcePoolAcceptor() = default;

  // Accept a request into the resource pool. If an empty optional is returned
  // the request is consumed (moved from), otherwise it is left intact.
  //
  // Once a request has been accepted the only outcomes should be
  // timeout/expired or executed.
  virtual std::optional<ServerRequestRejection> accept(
      ServerRequest&& request) = 0;
};

} // namespace apache::thrift
