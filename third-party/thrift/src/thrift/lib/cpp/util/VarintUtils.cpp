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

#include <thrift/lib/cpp/util/VarintUtils.h>

#include <thrift/lib/cpp/TApplicationException.h>

#include <stdint.h>

namespace apache::thrift::util {

/**
 * Read an i16 from the wire as a varint. The MSB of each byte is set
 * if there is another byte to follow. This can read up to 3 bytes.
 */
uint32_t readVarint16(
    const uint8_t* ptr, int16_t* i16, const uint8_t* boundary) {
  int64_t val;
  uint32_t rsize = readVarint64(ptr, &val, boundary);
  *i16 = (int16_t)val;
  return rsize;
}

/**
 * Read an i32 from the wire as a varint. The MSB of each byte is set
 * if there is another byte to follow. This can read up to 5 bytes.
 */
uint32_t readVarint32(
    const uint8_t* ptr, int32_t* i32, const uint8_t* boundary) {
  int64_t val;
  uint32_t rsize = readVarint64(ptr, &val, boundary);
  *i32 = (int32_t)val;
  return rsize;
}

/**
 * Read an i64 from the wire as a proper varint. The MSB of each byte is set
 * if there is another byte to follow. This can read up to 10 bytes.
 * Caller is responsible for advancing ptr after call.
 */
uint32_t readVarint64(
    const uint8_t* ptr, int64_t* i64, const uint8_t* boundary) {
  uint32_t rsize = 0;
  uint64_t val = 0;
  int shift = 0;

  while (true) {
    if (ptr == boundary) {
      throw TApplicationException(
          TApplicationException::INVALID_MESSAGE_TYPE,
          "Trying to read past header boundary");
    }
    uint8_t byte = *(ptr++);
    rsize++;
    val |= (uint64_t)(byte & 0x7f) << shift;
    shift += 7;
    if (!(byte & 0x80)) {
      *i64 = val;
      return rsize;
    }
  }
}

/**
 * Write an i32 as a varint. Results in 1-5 bytes on the wire.
 */
uint32_t writeVarint32(uint32_t n, uint8_t* pkt) {
  uint8_t buf[5];
  uint32_t wsize = 0;

  while (true) {
    if ((n & ~0x7F) == 0) {
      buf[wsize++] = (int8_t)n;
      break;
    } else {
      buf[wsize++] = (int8_t)((n & 0x7F) | 0x80);
      n >>= 7;
    }
  }

  // Caller will advance pkt.
  for (auto i = 0u; i < wsize; i++) {
    pkt[i] = buf[i];
  }

  return wsize;
}

uint32_t writeVarint64(uint64_t n, uint8_t* pkt) {
  uint8_t buf[10];
  uint32_t wsize = 0;

  while (true) {
    if ((n & ~0x7F) == 0) {
      buf[wsize++] = (int8_t)n;
      break;
    } else {
      buf[wsize++] = (int8_t)((n & 0x7F) | 0x80);
      n >>= 7;
    }
  }

  // Caller will advance pkt.
  for (auto i = 0u; i < wsize; i++) {
    pkt[i] = buf[i];
  }

  return wsize;
}

namespace detail {
[[noreturn]] void throwInvalidVarint() {
  throw std::out_of_range("invalid varint read");
}
} // namespace detail

} // namespace apache::thrift::util
