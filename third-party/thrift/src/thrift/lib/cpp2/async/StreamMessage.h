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

#include <folly/ExceptionWrapper.h>
#include <folly/Try.h>
#include <thrift/lib/cpp2/async/StreamPayload.h>

namespace apache::thrift {

/**
 * StreamMessage is simply a containing struct - the inner structs here
 * represent the actual message types passed around the server/client
 * state of stream, sink, or bidi rpcs. Most of these correspond to transport
 * frames, but there are a few exceptions e.g control messages.
 */
struct StreamMessage {
  /**
   * A PayloadOrError{} message is contains a folly::Try<StreamPayload>.
   * Eventually this will be split, but for ease of refactoring they are kept
   * together for now. Notably this is never expected to contain an empty Try
   * indicating completion - that is now it's own Complete{} message.
   */
  struct PayloadOrError {
    folly::Try<StreamPayload> streamPayloadTry;
  };

  /**
   * RequestN{} are flow control messages indicating the sender is ready to
   * process an additional n elements. It corresponds to a RequestN frame in
   * transport and onRequestN() for the various server callbacks. Note that
   * (signed) int32 is the representation, since that is what is supported by
   * Rocket and RSocket.
   */
  struct RequestN {
    int32_t n;
  };
  /**
   * Cancel{} is an explicit cancellation signal from the sender. Cancel{} is
   * a terminal message.
   */
  struct Cancel {};

  /**
   * Complete is a sentinel value to indicate that the stream has terminated
   * gracefully. This is terminal message.
   */
  struct Complete {};

  /**
   * Pause{} indicates to a queue consumer that it can suspend until
   * a Resume{} is received. Streams support pausing and resuming based on
   * certain transport level load metrics.
   */
  struct Pause {};

  /**
   * Resume{} indicates to a queue consumer that it can resume computation after
   * pausing. Streams support pausing and resuming based on certain transport
   * level load metrics.
   */
  struct Resume {};
};

} // namespace apache::thrift
