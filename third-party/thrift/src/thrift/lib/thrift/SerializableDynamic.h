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
#include <utility>

#include <folly/dynamic.h>

namespace apache::thrift {

namespace detail {

template <typename ThriftType>
folly::dynamic fromThriftDynamic(ThriftType value) {
  if constexpr (requires { value.get_boolean(); }) {
    switch (value.getType()) {
      case ThriftType::Type::__EMPTY__:
        return nullptr;
      case ThriftType::Type::boolean:
        return value.get_boolean();
      case ThriftType::Type::integer:
        return value.get_integer();
      case ThriftType::Type::doubl:
        return value.get_doubl();
      case ThriftType::Type::str:
        return value.move_str();
      case ThriftType::Type::arr: {
        folly::dynamic result = folly::dynamic::array;
        for (auto& item : value.move_arr()) {
          result.push_back(fromThriftDynamic(std::move(item)));
        }
        return result;
      }
      case ThriftType::Type::object: {
        folly::dynamic result = folly::dynamic::object;
        for (auto& [key, item] : value.move_object()) {
          result[std::move(key)] = fromThriftDynamic(std::move(item));
        }
        return result;
      }
    }
  } else {
    switch (value.getType()) {
      case ThriftType::Type::__EMPTY__:
        return nullptr;
      case ThriftType::Type::b:
        return value.get_b();
      case ThriftType::Type::i:
        return value.get_i();
      case ThriftType::Type::d:
        return value.get_d();
      case ThriftType::Type::s:
        return value.move_s();
      case ThriftType::Type::l: {
        folly::dynamic result = folly::dynamic::array;
        for (auto& item : value.move_l()) {
          result.push_back(fromThriftDynamic(std::move(item)));
        }
        return result;
      }
      case ThriftType::Type::o: {
        folly::dynamic result = folly::dynamic::object;
        for (auto& [key, item] : value.move_o()) {
          result[std::move(key)] = fromThriftDynamic(std::move(item));
        }
        return result;
      }
    }
  }
  throw std::runtime_error("Invalid Dynamic type");
}

} // namespace detail

class SerializableDynamic {
 public:
  SerializableDynamic() : value_(nullptr) {}

  /* implicit */ SerializableDynamic(folly::dynamic value)
      : value_(std::move(value)) {}

  template <typename ThriftType>
    requires(
        requires(ThriftType& value) { value.get_boolean(); } ||
        requires(ThriftType& value) { value.get_b(); })
  /* implicit */ SerializableDynamic(ThriftType value)
      : value_(detail::fromThriftDynamic(std::move(value))) {}

  SerializableDynamic& operator=(folly::dynamic value) {
    value_ = std::move(value);
    return *this;
  }

  const folly::dynamic& operator*() const { return value_; }
  folly::dynamic& operator*() { return value_; }

  const folly::dynamic& value() const { return value_; }
  folly::dynamic& value() { return value_; }

  const folly::dynamic* operator->() const { return &value_; }
  folly::dynamic* operator->() { return &value_; }

  bool operator==(const SerializableDynamic& other) const {
    return value_ == other.value_;
  }

  bool operator<(const SerializableDynamic& other) const {
    return value_ < other.value_;
  }

  void __fbthrift_clear() { value_ = nullptr; }

 private:
  folly::dynamic value_;
};

} // namespace apache::thrift

#include <thrift/lib/thrift/SerializableDynamicAdapter.h>
