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

#include <stdexcept>

#include <folly/File.h>
#include <folly/portability/GFlags.h>
#include <folly/system/MemoryMapping.h>
#include <thrift/lib/cpp2/frozen/Frozen.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/thrift/gen-cpp2/frozen_constants.h>

FOLLY_GFLAGS_DECLARE_bool(thrift_frozen_util_disable_mlock);
FOLLY_GFLAGS_DECLARE_bool(thrift_frozen_util_mlock_on_fault);

namespace apache {
namespace thrift {
namespace frozen {

class FrozenFileForwardIncompatible : public std::runtime_error {
 public:
  explicit FrozenFileForwardIncompatible(int fileVersion);

  int fileVersion() const { return fileVersion_; }
  int supportedVersion() const {
    return schema::frozen_constants::kCurrentFrozenFileVersion();
  }

 private:
  int fileVersion_;
};

/**
 * A FreezeRoot that mallocs buffers as needed.
 */
class MallocFreezer final : public FreezeRoot {
 public:
  explicit MallocFreezer() {}

  template <class T>
  void freeze(const Layout<T>& layout, const T& root) {
    doFreeze(layout, root);
  }

  void appendTo(std::string& out) const {
    out.reserve(size_ + out.size());
    for (auto& segment : segments_) {
      out.append(segment.buffer, segment.buffer + segment.size);
    }
  }

 private:
  size_t distanceToEnd(const byte* origin) const;
  size_t offsetOf(const byte* origin) const;

  folly::MutableByteRange appendBuffer(size_t size);

  void doAppendBytes(
      byte* origin,
      size_t n,
      folly::MutableByteRange& range,
      size_t& distance,
      size_t alignment) override;

  struct Segment {
    explicit Segment(size_t size);
    Segment(Segment&& other) : size(other.size), buffer(other.buffer) {
      other.buffer = nullptr;
    }

    ~Segment();

    size_t size{0};
    byte* buffer{nullptr}; // owned
  };
  std::map<const byte*, size_t> offsets_;
  std::vector<Segment> segments_;
  size_t size_{0};
};

/**
 * Returns an upper bound estimate of the number of bytes required to freeze
 * this object with a minimal layout. Actual bytes required will depend on the
 * alignment of the freeze buffer.
 */
template <class T>
size_t frozenSize(const T& v) {
  Layout<T> layout;
  return LayoutRoot::layout(v, layout) - LayoutRoot::kPaddingBytes;
}

/**
 * Returns an upper bound estimate of the number of bytes required to freeze
 * this object with a given layout. Actual bytes required will depend on on
 * the alignment of the freeze buffer.
 */
template <class T>
size_t frozenSize(const T& v, const Layout<T>& fixedLayout) {
  Layout<T> layout = fixedLayout;
  size_t size;
  bool changed;
  LayoutRoot::layout(v, layout, changed, size);
  if (changed) {
    throw LayoutException();
  }
  return size;
}

template <class T>
void serializeRootLayout(const Layout<T>& layout, std::string& out) {
  schema::MemorySchema memSchema;
  schema::Schema schema;
  saveRoot(layout, memSchema);
  schema::convert(memSchema, schema);

  *schema.fileVersion() = schema::frozen_constants::kCurrentFrozenFileVersion();
  out.clear();
  CompactSerializer::serialize(schema, &out);
}

template <class T>
void deserializeRootLayout(folly::ByteRange& range, Layout<T>& layoutOut) {
  schema::Schema schema;
  size_t schemaSize = CompactSerializer::deserialize(range, schema);

  if (*schema.fileVersion() >
      schema::frozen_constants::kCurrentFrozenFileVersion()) {
    throw FrozenFileForwardIncompatible(*schema.fileVersion());
  }

  schema::MemorySchema memSchema;
  schema::convert(std::move(schema), memSchema);
  loadRoot(layoutOut, memSchema);
  range.advance(schemaSize);
}

template <class T>
void freezeToFile(const T& x, folly::File file) {
  std::string schemaStr;
  auto layout = std::make_unique<Layout<T>>();
  auto contentSize = LayoutRoot::layout(x, *layout);

  serializeRootLayout(*layout, schemaStr);

  size_t initialBufferSize = contentSize + schemaStr.size();
  folly::MemoryMapping mapping(
      file.dup(), 0, initialBufferSize, folly::MemoryMapping::writable());
  auto mappingRange = mapping.writableRange();
  auto writeRange = mapping.writableRange();
  std::copy(schemaStr.begin(), schemaStr.end(), writeRange.begin());
  writeRange.advance(schemaStr.size());
  ByteRangeFreezer::freeze(*layout, x, writeRange);
  size_t finalBufferSize = writeRange.begin() - mappingRange.begin();
  folly::checkUnixError(
      ftruncate(file.fd(), finalBufferSize),
      "MallocFreezer: ftruncate() failed");
}

template <class T>
void freezeToString(const T& x, std::string& out) {
  out.clear();
  Layout<T> layout;
  size_t contentSize = LayoutRoot::layout(x, layout);
  serializeRootLayout(layout, out);

  size_t schemaSize = out.size();
  size_t bufferSize = schemaSize + contentSize;
  out.resize(bufferSize, 0);
  folly::MutableByteRange writeRange(
      reinterpret_cast<byte*>(&out[schemaSize]), contentSize);
  ByteRangeFreezer::freeze(layout, x, writeRange);
  out.resize(out.size() - writeRange.size());
}

template <class T>
std::string freezeToString(const T& x) {
  std::string result;
  freezeToString(x, result);
  return result;
}

template <class T>
std::string freezeDataToString(const T& x, const Layout<T>& layout) {
  std::string out;
  MallocFreezer freezer;
  freezer.freeze(layout, x);
  freezer.appendTo(out);
  return out;
}

template <class T>
void freezeToStringMalloc(const T& x, std::string& out) {
  Layout<T> layout;
  LayoutRoot::layout(x, layout);
  out.clear();
  serializeRootLayout(layout, out);
  MallocFreezer freezer;
  freezer.freeze(layout, x);
  freezer.appendTo(out);
}

/**
 * mapFrozen<T>() returns an owned reference to a frozen object which can be
 * used as a Frozen view of the object. All overloads of this function return
 * MappedFrozen<T>, which is an alias for a type which bundles the view with its
 * associated resources. This type may be used directly to hold references to
 * mapped frozen objects.
 *
 * Depending on which overload is used, this bundle will hold references to
 * different associated data:
 *
 *  - mapFrozen<T>(ByteRange): Only the layout tree associated with the object.
 *  - mapFrozen<T>(StringPiece): Same as mapFrozen<T>(ByteRange).
 *  - mapFrozen<T>(MemoryMapping): Takes ownership of the memory mapping
 *      in addition to the layout tree.
 *  - mapFrozen<T>(File): Owns the memory mapping created from the File (which,
 *      in turn, takes ownership of the File) in addition to the layout tree.
 */
template <class T>
using MappedFrozen = Bundled<typename Layout<T>::View>;

template <class T>
MappedFrozen<T> mapFrozen(folly::ByteRange range) {
  auto layout = std::make_unique<Layout<T>>();
  deserializeRootLayout(range, *layout);
  MappedFrozen<T> ret(layout->view({range.begin(), 0}));
  ret.hold(std::move(layout));
  return ret;
}

template <class T>
MappedFrozen<T> mapFrozen(folly::StringPiece range) {
  return mapFrozen<T>(folly::ByteRange(range));
}

template <class T>
MappedFrozen<T> mapFrozen(folly::MemoryMapping mapping) {
  auto ret = mapFrozen<T>(mapping.range());
  ret.hold(std::move(mapping));
  return ret;
}

/**
 * Maps from the given string, taking ownership of it and bundling it with the
 * return object to ensure its lifetime.
 * @param trim Trims the serialized layout from the input string.
 */
template <class T>
MappedFrozen<T> mapFrozen(std::string&& str, bool trim = true) {
  auto layout = std::make_unique<Layout<T>>();
  auto holder = std::make_unique<HolderImpl<std::string>>(std::move(str));
  auto& ownedStr = holder->t_;
  folly::ByteRange rangeBefore = folly::StringPiece(ownedStr);
  folly::ByteRange range = rangeBefore;
  deserializeRootLayout(range, *layout);
  if (trim) {
    size_t trimSize = range.begin() - rangeBefore.begin();
    ownedStr.erase(ownedStr.begin(), ownedStr.begin() + trimSize);
    ownedStr.shrink_to_fit();
    range = folly::StringPiece(ownedStr);
  }
  MappedFrozen<T> ret(layout->view({range.begin(), 0}));
  ret.holdImpl(std::move(holder));
  ret.hold(std::move(layout));
  return ret;
}

template <class T>
[[deprecated(
    "std::string values must be passed by move with std::move(str) or "
    "passed through non-owning StringPiece")]] MappedFrozen<T>
mapFrozen(const std::string& str) = delete;

template <class T>
MappedFrozen<T> mapFrozen(
    folly::File file, folly::MemoryMapping::LockMode lockMode) {
  folly::MemoryMapping mapping(std::move(file), 0);
  folly::MemoryMapping::LockFlags flags{};
  flags.lockOnFault = FLAGS_thrift_frozen_util_mlock_on_fault;
  mapping.mlock(lockMode, flags);
  return mapFrozen<T>(std::move(mapping));
}

template <class T>
MappedFrozen<T> mapFrozen(folly::File file) {
  return mapFrozen<T>(
      std::move(file), folly::MemoryMapping::LockMode::TRY_LOCK);
}

} // namespace frozen
} // namespace thrift
} // namespace apache
