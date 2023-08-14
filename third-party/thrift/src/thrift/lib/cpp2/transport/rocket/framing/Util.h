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

#include <array>
#include <cstdint>
#include <exception>
#include <string>

#include <folly/Memory.h>
#include <folly/io/Cursor.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>

namespace apache {
namespace thrift {
namespace rocket {

namespace detail {
[[noreturn]] void throwUnexpectedFrameType(uint8_t frameType);
} // namespace detail

StreamId readStreamId(folly::io::Cursor& cursor);

size_t readFrameOrMetadataSize(folly::io::Cursor& cursor);

size_t readFrameOrMetadataSize(std::array<uint8_t, 3> bytes);

std::pair<uint8_t, Flags> readFrameTypeAndFlagsUnsafe(
    folly::io::Cursor& cursor);

std::pair<FrameType, Flags> readFrameTypeAndFlags(folly::io::Cursor& cursor);

ExtFrameType readExtFrameType(folly::io::Cursor& cursor);

bool alignTo4k(folly::IOBuf& buffer, size_t startOffset, size_t frameSize);
// Used in the new buffer logic in rocket parser. alignTo4k will be deprecated
// once the new buffer logic is stable.
bool alignTo4kBufQueue(
    folly::IOBufQueue& bufQueue, size_t startOffset, size_t frameSize);

std::unique_ptr<folly::IOBuf> get4kAlignedBuf(
    size_t numBytes, size_t startOffset, size_t trimLength);

// Has both false positives and false negatives
bool isMaybeRocketFrame(const folly::IOBuf& data);

} // namespace rocket
} // namespace thrift
} // namespace apache
