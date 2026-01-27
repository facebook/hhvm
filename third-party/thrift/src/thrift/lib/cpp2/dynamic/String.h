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

#include <memory_resource>
#include <string>
#include <string_view>

namespace apache::thrift::dynamic {

/**
 * A String type backed by std::pmr::string for memory resource control.
 * Data is only exposed as std::string_view to avoid exposing the backing type.
 */
class String final {
 public:
  // Constructors
  String() : String(nullptr) {}
  explicit String(std::pmr::memory_resource* mr)
      : data_(mr ? mr : std::pmr::get_default_resource()) {}
  explicit String(std::string_view sv, std::pmr::memory_resource* mr = nullptr)
      : data_(sv, mr ? mr : std::pmr::get_default_resource()) {}

  // Copy and move
  String(const String&) = default;
  String(String&&) noexcept = default;
  String& operator=(const String&) = default;
  String& operator=(String&&) noexcept = default;
  ~String() = default;

  // Data access (read-only view)
  std::string_view view() const noexcept { return data_; }
  const char* data() const noexcept { return data_.data(); }
  size_t size() const noexcept { return data_.size(); }
  bool empty() const noexcept { return data_.empty(); }

  // Conversion operators
  operator std::string_view() const noexcept { return view(); }

  // Mutation methods
  void append(std::string_view sv) { data_.append(sv); }
  void append(const char* s, size_t n) { data_.append(s, n); }
  void clear() noexcept { data_.clear(); }
  void reserve(size_t n) { data_.reserve(n); }
  void resize(size_t n) { data_.resize(n); }
  void resize(size_t n, char c) { data_.resize(n, c); }

  // Assignment from string_view
  String& operator=(std::string_view sv) {
    data_ = sv;
    return *this;
  }

  // Comparison
  friend bool operator==(const String& lhs, const String& rhs) noexcept {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator==(const String& lhs, std::string_view rhs) noexcept {
    return lhs.data_ == rhs;
  }
  friend bool operator==(std::string_view lhs, const String& rhs) noexcept {
    return lhs == rhs.data_;
  }

 private:
  std::pmr::string data_;
};

} // namespace apache::thrift::dynamic
