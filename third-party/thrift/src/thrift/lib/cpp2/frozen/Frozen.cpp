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

#include <thrift/lib/cpp2/frozen/Frozen.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

namespace apache {
namespace thrift {
namespace frozen {

std::ostream& operator<<(std::ostream& os, DebugLine dl) {
  os << '\n';
  for (int i = 0; i < dl.level; ++i) {
    os << ' ' << ' ';
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const LayoutBase& layout) {
  layout.print(os, 0);
  return os;
}

bool LayoutBase::resize(FieldPosition after, bool _inlined) {
  bool resized = false;
  inlined = (this->size == 0 && _inlined);
  if (!inlined) {
    if (static_cast<size_t>(after.offset) > this->size) {
      this->size = after.offset;
      resized = true;
    }
  }
  if (static_cast<size_t>(after.bitOffset) > this->bits) {
    this->bits = after.bitOffset;
    resized = true;
  }
  return resized;
}

void LayoutBase::print(std::ostream& os, int level) const {
  os << DebugLine(level);
  if (size) {
    os << size << " byte";
    if (bits) {
      os << " (with " << bits << " bits)";
    }
  } else if (bits) {
    os << bits << " bit";
  } else {
    os << "empty";
  }
  os << ' ';
}

void LayoutBase::clear() {
  size = 0;
  bits = 0;
  inlined = false;
}

void ByteRangeFreezer::doAppendBytes(
    byte* origin,
    size_t n,
    folly::MutableByteRange& range,
    size_t& distance,
    size_t alignment) {
  CHECK_LE(origin, write_.begin());
  if (!n) {
    distance = 0;
    range.reset(nullptr, 0);
    return;
  }
  auto start = reinterpret_cast<intptr_t>(write_.begin());
  auto aligned = alignBy(start, alignment);
  auto padding = aligned - start;
  if (padding + n > write_.size()) {
    throw std::length_error("Insufficient buffer allocated");
  }
  range.reset(write_.begin() + padding, n);
  write_.advance(padding + n);
  distance = range.begin() - origin;
}

namespace detail {

FieldPosition BlockLayout::maximize() {
  FieldPosition pos = startFieldPosition();
  FROZEN_MAXIMIZE_FIELD(mask);
  FROZEN_MAXIMIZE_FIELD(offset);
  return pos;
}

FieldPosition BlockLayout::layout(
    LayoutRoot& root, const T& x, LayoutPosition self) {
  FieldPosition pos = startFieldPosition();
  FROZEN_LAYOUT_FIELD_REQ(mask);
  FROZEN_LAYOUT_FIELD_REQ(offset);
  return pos;
}

void BlockLayout::freeze(
    FreezeRoot& root, const T& x, FreezePosition self) const {
  FROZEN_FREEZE_FIELD_REQ(mask);
  FROZEN_FREEZE_FIELD_REQ(offset);
}

void BlockLayout::print(std::ostream& os, int level) const {
  LayoutBase::print(os, level);
  os << folly::demangle(type.name());
  maskField.print(os, level + 1);
  offsetField.print(os, level + 1);
}

void BlockLayout::clear() {
  maskField.clear();
  offsetField.clear();
}

size_t BufferHelpers<std::unique_ptr<folly::IOBuf>>::size(
    const std::unique_ptr<folly::IOBuf>& src) {
  return src != nullptr ? src->computeChainDataLength() : 0;
}

void BufferHelpers<std::unique_ptr<folly::IOBuf>>::copyTo(
    const std::unique_ptr<folly::IOBuf>& src, folly::MutableByteRange dst) {
  if (src != nullptr) {
    folly::io::Cursor(src.get()).pull(dst.begin(), dst.size());
  }
}

void BufferHelpers<std::unique_ptr<folly::IOBuf>>::thawTo(
    folly::ByteRange src, std::unique_ptr<folly::IOBuf>& dst) {
  dst = folly::IOBuf::copyBuffer(src.begin(), src.size());
}

size_t BufferHelpers<folly::IOBuf>::size(const folly::IOBuf& src) {
  return src.computeChainDataLength();
}

void BufferHelpers<folly::IOBuf>::copyTo(
    const folly::IOBuf& src, folly::MutableByteRange dst) {
  folly::io::Cursor(&src).pull(dst.begin(), dst.size());
}

void BufferHelpers<folly::IOBuf>::thawTo(
    folly::ByteRange src, folly::IOBuf& dst) {
  dst = folly::IOBuf(folly::IOBuf::COPY_BUFFER, src);
}

} // namespace detail
} // namespace frozen
} // namespace thrift
} // namespace apache
