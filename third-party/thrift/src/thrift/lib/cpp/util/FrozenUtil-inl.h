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

#ifndef THRIFT_LIB_CPP_UTIL_FROZENUTIL_H_
#error This must only be included via FrozenUtil.h
#endif

namespace apache {
namespace thrift {
namespace util {

template <class T, class Frozen>
const Frozen* freezeToFile(
    const T& value, const folly::MemoryMapping& mapping) {
  DCHECK_GE(mapping.range().size(), frozenSize(value));
  auto writableBytes = mapping.writableRange();
  byte* buffer = writableBytes.begin();
  const Frozen* frozen = freeze(value, buffer);
  size_t size = buffer - writableBytes.begin();
  ftruncate(mapping.fd(), size);
  return frozen;
}

template <class T>
void freezeToFile(const T& value, folly::File file) {
  size_t size = frozenSize(value);
  folly::MemoryMapping mapping(
      std::move(file), 0, size, folly::MemoryMapping::writable());

  auto writableBytes = mapping.writableRange();
  byte* start = writableBytes.begin();
  byte* buffer = start;

  freeze(value, buffer);

  CHECK_EQ(size, buffer - start);
  ftruncate(mapping.fd(), size);
}

template <class T>
void freezeToFile(const T& value, int fd) {
  freezeToFile(value, folly::File(fd));
}

template <class T>
void freezeToSparseFile(const T& value, folly::File file, size_t sparseSize) {
  // Make a sparse file of the requested size, freeze to it, then truncate the
  // file to the actual size when done.
  folly::MemoryMapping mapping(
      std::move(file), 0, sparseSize, folly::MemoryMapping::writable());

  auto writableBytes = mapping.writableRange();
  byte* start = writableBytes.begin();
  byte* buffer = start;

  freeze(value, buffer);

  size_t actualSize = buffer - start;
  ftruncate(mapping.fd(), actualSize);
}

template <class T, class Frozen>
const Frozen* mapFrozen(folly::ByteRange mapping) {
  return reinterpret_cast<const Frozen*>(mapping.data());
}

template <class T, class Frozen>
const Frozen* mapFrozen(const folly::MemoryMapping& mapping) {
  return mapping.asRange<Frozen>().data();
}

} // namespace util
} // namespace thrift
} // namespace apache
