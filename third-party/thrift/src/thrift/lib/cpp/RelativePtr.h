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

#ifndef THRIFT_RELATIVEPTR_H_
#define THRIFT_RELATIVEPTR_H_

#include <glog/logging.h>

namespace apache {
namespace thrift {

typedef uint8_t byte;

// Relative Ptr - The key to relocatable object graphs
// TODO: expose 'OffsetType' as a type parameter in freeze()
template <class T, class OffsetType = uint32_t>
class RelativePtr {
  OffsetType offset_;

 public:
  RelativePtr() { reset(nullptr); }

  RelativePtr(const RelativePtr&) = delete;
  RelativePtr& operator=(const RelativePtr&) = delete;

  explicit RelativePtr(T* ptr) { reset(ptr); }

  void reset(T* ptr = nullptr) {
    if (!ptr) {
      offset_ = 0;
      return;
    }
    const byte* target = reinterpret_cast<const byte*>(ptr);
    const byte* origin = reinterpret_cast<const byte*>(this);
    auto offset = target - origin;
    offset_ = offset;
    CHECK_EQ(offset_, offset) << "Structure too large to address using "
                              << (sizeof(OffsetType) * 8) << "-bit pointers";
  }

  T* get() const {
    if (!offset_) {
      return nullptr;
    }
    byte* origin = const_cast<byte*>(reinterpret_cast<const byte*>(this));
    byte* target = origin + offset_;
    return reinterpret_cast<T*>(target);
  }

  T& operator*() const { return *get(); }
};

} // namespace thrift
} // namespace apache

#endif // THRIFT_RELATIVEPTR_H_
