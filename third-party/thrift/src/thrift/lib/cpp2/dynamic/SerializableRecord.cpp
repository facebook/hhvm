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

#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>

#include <thrift/shared/tree_printer.h>

#include <folly/base64.h>

#include <fmt/core.h>

#include <ostream>

namespace apache::thrift::dynamic {

namespace {

void printTo(tree_printer::scope& scope, const SerializableRecord& record) {
  record.visit(
      [&](SerializableRecord::Bool value) {
        scope.print("Bool({})", value ? "true" : "false");
      },
      [&](SerializableRecord::Int8 value) { scope.print("Int8({})", value); },
      [&](SerializableRecord::Int16 value) { scope.print("Int16({})", value); },
      [&](SerializableRecord::Int32 value) { scope.print("Int32({})", value); },
      [&](SerializableRecord::Int64 value) { scope.print("Int64({})", value); },
      [&](SerializableRecord::Float32 value) {
        scope.print("Float32({})", value);
      },
      [&](SerializableRecord::Float64 value) {
        scope.print("Float64({})", value);
      },
      [&](const SerializableRecord::Text& value) {
        scope.print("Text(\"{}\")", tree_printer::escape(value));
      },
      [&](const SerializableRecord::ByteArray& value) {
        scope.print(
            "ByteArray(\"{}\")", folly::base64Encode(value->toString()));
      },
      [&](const SerializableRecord::FieldSet& value) {
        scope.print("FieldSet(size={})", value.size());
        for (const auto& [fieldId, field] : value) {
          printTo(scope.make_child("{} → ", fieldId), field);
        }
      },
      [&](const SerializableRecord::List& value) {
        scope.print("List(size={})", value.size());
        for (std::size_t i = 0; i < value.size(); ++i) {
          printTo(scope.make_child("[{}] → ", i), value[i]);
        }
      },
      [&](const SerializableRecord::Set& value) {
        // The order of elements is indeterminate because it's a hashset.
        scope.print("Set(size={})", value.size());
        for (const auto& element : value) {
          printTo(scope.make_child(), element);
        }
      },
      [&](const SerializableRecord::Map& value) {
        // The order of elements is indeterminate because it's a hashmap.
        scope.print("Map(size={})", value.size());
        for (const auto& [k, v] : value) {
          tree_printer::scope& keyScope = scope.make_child("key = ");
          printTo(keyScope, k);
          tree_printer::scope& valueScope = scope.make_child("value = ");
          printTo(valueScope, v);
        }
      });
}

} // namespace

std::string toDebugString(const SerializableRecord& record) {
  auto scope = tree_printer::scope::make_root();
  printTo(scope, record);
  return tree_printer::to_string(scope);
}

std::ostream& operator<<(std::ostream& out, const SerializableRecord& record) {
  auto scope = tree_printer::scope::make_root();
  printTo(scope, record);
  return out << scope;
}

} // namespace apache::thrift::dynamic
