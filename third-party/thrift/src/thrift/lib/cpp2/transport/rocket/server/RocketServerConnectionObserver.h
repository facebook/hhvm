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

#include <sys/types.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>

namespace apache {
namespace thrift {
namespace rocket {

class RocketServerConnection;

class RocketServerConnectionObserver {
 public:
  enum class Events {
    WriteEvents = 1,
  };

  struct WriteEvent {
    // the stream id of the write
    const StreamId streamId;
    // the start offset of the write (relative to the write batch it is part
    // of)
    const size_t batchOffset;
    // the number of bytes in the write
    const size_t totalBytesInWrite;

    explicit WriteEvent(
        StreamId streamId, size_t batchOffset, size_t totalBytes)
        : streamId(streamId),
          batchOffset(batchOffset),
          totalBytesInWrite(totalBytes) {}
  };

  virtual ~RocketServerConnectionObserver() = default;

  /**
   * writeReady() is invoked when a new response is ready to be written to
   * the underlying transport
   */
  virtual void writeReady(
      RocketServerConnection* /* connection */,
      const WriteEvent& /* writeEvent */) {}

  /**
   * writeSuccess() is invoked when a response has been successfully
   * written to the underlying transport
   */
  virtual void writeSuccess(
      RocketServerConnection* /* connection */,
      const WriteEvent& /* writeEvent */) {}
};

} // namespace rocket
} // namespace thrift
} // namespace apache
