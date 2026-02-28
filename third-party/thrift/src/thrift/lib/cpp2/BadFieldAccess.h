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
#include <string>

namespace apache::thrift {

// Base exception type for field access errors
class bad_field_access : public std::runtime_error {
 public:
  bad_field_access() = delete;
  explicit bad_field_access(const char* what) : std::runtime_error(what) {}
  explicit bad_field_access(const std::string& what)
      : std::runtime_error(what) {}
};

// An exception thrown when accessing an unset optional field value
class bad_optional_field_access : public bad_field_access {
 public:
  bad_optional_field_access()
      : bad_field_access("accessing unset optional value") {}
};

// An exception thrown when accessing an unset union field value or using union
// field accessors that do not match the union's type
class bad_union_field_access : public bad_field_access {
 public:
  bad_union_field_access()
      : bad_field_access("accessing unset or mismatched type union value") {}
  explicit bad_union_field_access(const std::string& what)
      : bad_field_access(what) {}
};

} // namespace apache::thrift
