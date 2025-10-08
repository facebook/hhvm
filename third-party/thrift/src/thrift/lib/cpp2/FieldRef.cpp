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

#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/FieldRef.h>

#include <fmt/core.h>

[[noreturn]] void apache::thrift::detail::throw_on_bad_optional_field_access() {
  throw bad_optional_field_access();
}

[[noreturn]] void apache::thrift::detail::throw_on_bad_union_field_access() {
  throw bad_union_field_access();
}

[[noreturn]] void apache::thrift::detail::throw_on_bad_union_field_access(
    std::string_view union_name,
    int16_t accessed_field_id,
    int16_t active_field_id) {
  throw bad_union_field_access(fmt::format(
      "bad union field access: attempted to access field '{}' in union '{}', but active field is '{}'",
      accessed_field_id,
      union_name,
      active_field_id));
}

[[noreturn]] void apache::thrift::detail::throw_on_nullptr_dereferencing() {
  throw std::logic_error(
      "Trying to dereference a nullptr in union_field_ref. "
      "This can only happen if user used setter to clear cpp.ref pointer. "
      "It won't happen if user didn't use deprecated setter API at all.");
}
