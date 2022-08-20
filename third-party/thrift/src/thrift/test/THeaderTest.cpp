/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <folly/io/IOBufQueue.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

using namespace apache::thrift::transport;

TEST(THeaderTest, removeBadHeaderStringSize) {
  uint8_t badHeader[] = {
      0x00, 0x00, 0x00, 0x13, // Frame size is corrupted here
      0x0F, 0xFF, 0x00, 0x00, // THRIFT_HEADER_CLIENT_TYPE
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // Header size
      0x00, // Proto ID
      0x00, // Num transforms
      0x01, // Info ID Key value
      0x01, // Num headers
      0xFF, 0xFF, 0xFF, 0xFF, // Malformed varint32 string size
      0x00 // String should go here
  };
  folly::IOBufQueue queue;
  queue.append(folly::IOBuf::wrapBuffer(badHeader, sizeof(badHeader)));
  // Try to remove the bad header
  THeader header;
  size_t needed;
  THeader::StringToStringMap persistentHeaders;
  EXPECT_THROW(
      auto buf = header.removeHeader(&queue, needed, persistentHeaders),
      TTransportException);
}
