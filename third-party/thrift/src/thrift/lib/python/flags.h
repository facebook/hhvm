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

#include <set>
#include <unordered_map>

#include <folly/Indestructible.h>
#include <folly/Synchronized.h>
#include <thrift/lib/cpp2/Flags.h>

namespace apache::thrift::python {

template <typename T>
using enable_if_bool_or_int = std::
    enable_if_t<std::is_same_v<bool, T> || std::is_same_v<int64_t, T>, int>;

template <typename T, enable_if_bool_or_int<T> = true>
class FlagWrapperWithIndestructibleName {
 public:
  FlagWrapperWithIndestructibleName(const std::string& name, T defaultValue)
      : name_(name), wrapper_(name_, defaultValue) {}
  T get() { return wrapper_.get(); }

 private:
  std::string name_;
  apache::thrift::detail::FlagWrapper<T> wrapper_;
};

template <typename T, enable_if_bool_or_int<T> = true>
class FlagRegistry final {
 public:
  FlagRegistry() = delete;
  static void define(const std::string& name, T defaultValue) {
    allFlags().wlock()->emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(name, defaultValue));
  }
  static T get(const std::string& name) {
    return allFlags().wlock()->at(name).get();
  }

 private:
  static auto& allFlags() {
    static folly::Indestructible<folly::Synchronized<
        std::unordered_map<std::string, FlagWrapperWithIndestructibleName<T>>>>
        flags;
    return *flags;
  }
};

} // namespace apache::thrift::python
