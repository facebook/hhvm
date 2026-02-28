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

#include <list>

namespace apache::thrift::compiler::detail {

class metric_base {
 public:
  metric_base() = default;
  virtual ~metric_base() = default;
};

template <typename T>
class metric : public metric_base {
 public:
  const T& get() const { return value_; }

 protected:
  T value_;
};

template <typename T>
class value : public metric<T> {
 public:
  void set(T new_value) { this->value_ = new_value; }
};

template <typename K>
class event : public metric<std::list<K>> {
 public:
  void add(K new_value) { this->value_.push_back(new_value); }
};

class counter : public metric<int> {
 public:
  counter() { this->value_ = 0; }

  void increment() { ++this->value_; }
};
} // namespace apache::thrift::compiler::detail
