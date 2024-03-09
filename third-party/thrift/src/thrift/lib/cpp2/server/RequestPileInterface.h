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

#include <cstdint>
#include <optional>
#include <string>

#include <thrift/lib/cpp2/server/RequestCompletionCallback.h>
#include <thrift/lib/thrift/gen-cpp2/serverdbginfo_types.h>

namespace apache::thrift {

class ServerRequestRejection;
class ServerRequest;

// The common interface to all request piles. The details of the construction
// are left to implementations of this interface. A request pile is effectively
// a queue (or more normally multiple queues). The users of this interface
// add requests and remove requests and the request pile implementation handles
// any prioritization, resource balancing, load limiting etc amongst all the
// requests it is managing.
class RequestPileInterface : public RequestCompletionCallback {
 public:
  // Returns a snapshot of the total of current requests in this request pile.
  // This is only intended for monitoring. This is thread safe.
  virtual uint64_t requestCount() const = 0;

  // Try to enqueue a request. The RequestPile implementation will determine
  // how to queue it, or whether to reject it. This might be using logic
  // written directly in the enqueue method or a RequestPile implementation
  // might provide a customization point for users to route the request to
  // different queues within the request pile depending on its attributes.
  //
  // If this returns an empty optional enqueue has consumed the supplied
  // request, if this returns a non-empty optional the supplied request has not
  // been consumed.
  virtual std::optional<ServerRequestRejection> enqueue(
      ServerRequest&& request) = 0;

  // Return the next request to process from the request pile or nothing.
  //
  // A RequestPiles implementation can choose not to return a request even if it
  // has one on the specified pile which could be returned. For example, if the
  // service is maintaining a strict quota of active requests per client that
  // includes keeping space reserved for a potential future request from a
  // client instead of executing a request that would exceed the quota of a
  // different client.
  virtual std::optional<ServerRequest> dequeue() = 0;

  // If a callback was requested this will be called when the request processing
  // has finished.
  void onRequestFinished(ServerRequestData&) override;

  virtual std::string describe() const = 0;

  virtual serverdbginfo::RequestPileDbgInfo getDbgInfo() const { return {}; }
};

} // namespace apache::thrift
