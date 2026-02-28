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

#include <string>

#include <boost/operators.hpp>

#include <folly/Range.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>

namespace test {

class Buffer : boost::totally_ordered<Buffer> {
 public:
  Buffer() { buf_.unshare(); }
  explicit Buffer(std::string&& str) : Buffer() {
    clear();
    append(str.data(), str.size());
    std::memset(const_cast<char*>(str.data()), 0, str.size());
    str.clear();
  }
  Buffer(const Buffer& that) = default;
  Buffer(Buffer&& that) noexcept = default;
  Buffer& operator=(const Buffer& that) = default;
  Buffer& operator=(Buffer&& that) noexcept = default;
  size_t size() const { return buf_.length(); }
  size_t length() const { return buf_.length(); }
  const uint8_t* data() const { return buf_.data(); }
  void clear() { buf_.trimEnd(buf_.length()); }
  void reserve(size_t desiredCapacity) { buf_.reserve(0, desiredCapacity); }
  Buffer& append(const void* data, size_t len) {
    if (len > 0) {
      reserve(buf_.length() + len); // make sure there's enough capacity
      std::memcpy(buf_.writableTail(), data, len);
      buf_.append(len);
    }
    return *this;
  }
  bool operator==(const Buffer& that) const {
    return folly::IOBufEqualTo{}(buf_, that.buf_);
  }
  bool operator<(const Buffer& that) const {
    return folly::IOBufLess{}(buf_, that.buf_);
  }
  operator folly::ByteRange() const {
    return folly::ByteRange(buf_.data(), buf_.length());
  }

 private:
  folly::IOBuf buf_;
};

} // namespace test

namespace apache::thrift {

template <>
class Cpp2Ops<::test::Buffer> {
 public:
  using Type = ::test::Buffer;
  static constexpr protocol::TType thriftType() { return protocol::T_STRING; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeBinary(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readBinary(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeBinary(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeZCBinary(*value);
  }
};

} // namespace apache::thrift
