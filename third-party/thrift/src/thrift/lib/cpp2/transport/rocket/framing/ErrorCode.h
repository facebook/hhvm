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

#include <folly/Range.h>

namespace apache {
namespace thrift {
namespace rocket {

enum class ErrorCode : uint32_t {
  // Reserved
  RESERVED = 0x00000000,
  // The Setup frame is invalid for the server (it could be that the client is
  // too recent for the old server). Stream ID MUST be 0.
  INVALID_SETUP = 0x00000001,
  // Some (or all) of the parameters specified by the client are unsupported by
  // the server. Stream ID MUST be 0.
  UNSUPPORTED_SETUP = 0x00000002,
  // The server rejected the setup, it can specify the reason in the payload.
  // Stream ID MUST be 0.
  REJECTED_SETUP = 0x00000003,
  // The server rejected the resume, it can specify the reason in the payload.
  // Stream ID MUST be 0.
  REJECTED_RESUME = 0x00000004,
  // The connection is being terminated. Stream ID MUST be 0. Sender or Receiver
  // of this frame MAY close the connection immediately without waiting for
  // outstanding streams to terminate.
  CONNECTION_ERROR = 0x00000101,
  //  The connection is being terminated. Stream ID MUST be 0. Sender or
  //  Receiver of this frame MUST wait for outstanding streams to terminate
  //  before closing the connection. New requests MAY not be accepted.
  CONNECTION_CLOSE = 0x00000102,
  // Application layer logic generating a Reactive Streams onError event. Stream
  // ID MUST be > 0.
  APPLICATION_ERROR = 0x00000201,
  // Despite being a valid request, the Responder decided to reject it. The
  // Responder guarantees that it didn't process the request. The reason for the
  // rejection is explained in the Error Data section. Stream ID MUST be > 0.
  REJECTED = 0x00000202,
  // The Responder canceled the request but may have started processing it
  // (similar to REJECTED but doesn't guarantee lack of side-effects). Stream ID
  // MUST be > 0.
  CANCELED = 0x00000203,
  // The request is invalid. Stream ID MUST be > 0.
  INVALID = 0x00000204,
  // Reserved for extension use.
  RESERVED_EXT = 0xFFFFFFFF,
};

folly::StringPiece toString(ErrorCode ec);

} // namespace rocket
} // namespace thrift
} // namespace apache
