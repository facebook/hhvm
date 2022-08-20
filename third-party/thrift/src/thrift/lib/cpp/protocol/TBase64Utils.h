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

#ifndef _THRIFT_PROTOCOL_TBASE64UTILS_H_
#define _THRIFT_PROTOCOL_TBASE64UTILS_H_

#include <stdint.h>
#include <string>

#include <folly/io/IOBuf.h>

namespace apache {
namespace thrift {
namespace protocol {

// in must be at least len bytes
// len must be 1, 2, or 3
// buf must be a buffer of at least 4 bytes and may not overlap in
// the data is not padded with '='; the caller can do this if desired
void base64_encode(const uint8_t* in, uint32_t len, uint8_t* buf);

// buf must be a buffer of at least 4 bytes and contain base64 encoded values
// buf will be changed to contain output bytes
// len is number of bytes to consume from input (must be 2, 3, or 4)
// no '=' padding should be included in the input
void base64_decode(uint8_t* buf, uint32_t len);

std::string base64Encode(folly::ByteRange binary);

std::unique_ptr<folly::IOBuf> base64Decode(folly::StringPiece base64);

} // namespace protocol
} // namespace thrift
} // namespace apache

#endif // #define _THRIFT_PROTOCOL_TBASE64UTILS_H_
