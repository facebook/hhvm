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

#include <folly/Exception.h>
#include <folly/container/F14Map.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>

namespace apache::thrift::protocol::detail {

void validateTypeIsFull(const type::Type& type);
std::string debugFormatType(const type::Type& type);

// A wrapper around a map that validates that keys are full types
template <typename Mask>
class ValidatingTypeMap {
 public:
#if FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE
  using map_type = folly::F14VectorMap<type::Type, Mask>;
  void reserve(std::size_t n) { map_.reserve(n); }
#else
  // f14 map is not available in some platforms. Default to std::map which is
  // able to handle incomplete types
  using map_type = std::map<type::Type, Mask>;
  // std::map doesn't support reserve :(
  void reserve(std::size_t) {}
#endif

  using key_type = typename map_type::key_type;
  using value_type = Mask;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;
  using mapped_type = typename map_type::mapped_type;

  iterator begin() { return map_.begin(); }
  iterator end() { return map_.end(); }
  const_iterator begin() const { return map_.begin(); }
  const_iterator end() const { return map_.end(); }

  iterator find(const key_type& key) {
    validateTypeIsFull(key);
    return map_.find(key);
  }
  const_iterator find(const key_type& key) const {
    validateTypeIsFull(key);
    return map_.find(key);
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(const key_type& key, Args&&... args) {
    validateTypeIsFull(key);
    return map_.emplace(key, std::forward<Args>(args)...);
  }
  template <typename... Args>
  std::pair<iterator, bool> emplace(key_type&& key, Args&&... args) {
    validateTypeIsFull(key);
    return map_.emplace(std::move(key), std::forward<Args>(args)...);
  }

  mapped_type& operator[](const key_type& key) {
    validateTypeIsFull(key);
    return map_[key];
  }

  mapped_type& at(const key_type& key) {
    validateTypeIsFull(key);
    return map_.at(key);
  }

  const mapped_type& at(const key_type& key) const {
    validateTypeIsFull(key);
    return map_.at(key);
  }

  size_t size() const { return map_.size(); }

  bool empty() const { return map_.empty(); }

  bool operator==(const ValidatingTypeMap& rhs) const {
    return this->map_ == rhs.map_;
  }

 private:
  map_type map_;
};

/**
 * Adapter to convert a List<<Type, Mask>> into a Map<Type, Mask>
 *
 * Because structured keys are not well supported by some langauges, we cannot
 * use Map<Type, Mask> directly in the IDL.
 *
 * This Adapter attempts to maintain a sane user-experience in C++
 */

template <typename Entry, typename Mask>
class TypeToMaskAdapter {
  using ThriftType = std::vector<Entry>;
  using AdaptedType = ValidatingTypeMap<Mask>;

 public:
  static AdaptedType fromThrift(const ThriftType& thriftVal) {
    AdaptedType adaptedVal;
    adaptedVal.reserve(thriftVal.size());
    for (const Entry& e : thriftVal) {
      if (!adaptedVal.emplace(*e.type(), folly::copy(*e.mask())).second) {
        folly::throw_exception<std::runtime_error>(
            "type-mask has mulitple entries for the same type: " +
            debugFormatType(*e.type()));
      }
    }
    return adaptedVal;
  }

  static ThriftType toThrift(const AdaptedType& adaptedVal) {
    ThriftType thriftVal;
    thriftVal.reserve(adaptedVal.size());
    for (const auto& [type, mask] : adaptedVal) {
      thriftVal.emplace_back();
      thriftVal.back().type() = type;
      thriftVal.back().mask() = mask;
    }
    return thriftVal;
  }

  // TODO: Customize encode/decode to improve performance of serde (i.e. avoid
  // materializing intermediate thrift struct)
};

} // namespace apache::thrift::protocol::detail
