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

#include <thrift/lib/cpp2/gen/module_metadata_cpp.h>

namespace apache::thrift::detail::md {

ThriftConstValue cvBool(bool value) {
  ThriftConstValue ret;
  ret.cv_bool() = value;
  return ret;
}

ThriftConstValue cvInteger(int64_t value) {
  ThriftConstValue ret;
  ret.cv_integer() = value;
  return ret;
}

ThriftConstValue cvDouble(double value) {
  ThriftConstValue ret;
  ret.cv_double() = value;
  return ret;
}

ThriftConstValue cvString(const char* value) {
  ThriftConstValue ret;
  ret.cv_string() = value;
  return ret;
}

ThriftConstValue cvMap(std::vector<ThriftConstValuePair>&& value) {
  ThriftConstValue ret;
  ret.cv_map() = std::move(value);
  return ret;
}

ThriftConstValue cvList(std::initializer_list<ThriftConstValue> value) {
  ThriftConstValue ret;
  ret.cv_list().emplace().assign(
      std::make_move_iterator(value.begin()),
      std::make_move_iterator(value.end()));
  return ret;
}

ThriftConstValue cvStruct(
    const char* name, std::map<std::string, ThriftConstValue>&& fields) {
  ThriftConstValue ret;
  ThriftConstStruct s;
  s.type()->name() = name;
  s.fields() = std::move(fields);
  ret.cv_struct() = std::move(s);
  return ret;
}

ThriftConstValuePair cvPair(ThriftConstValue&& key, ThriftConstValue&& value) {
  ThriftConstValuePair pair;
  pair.key() = std::move(key);
  pair.value() = std::move(value);
  return pair;
}

void genStructFieldMetadata(
    const syntax_graph::StructuredNode&,
    metadata::ThriftField& field,
    const EncodedThriftField& f,
    size_t) {
  DCHECK_EQ(*field.id(), f.id);
  DCHECK_EQ(*field.name(), f.name);
  DCHECK_EQ(*field.is_optional(), f.is_optional);

  auto newAnnotations = std::move(*field.structured_annotations());
  field.structured_annotations().emplace().assign(
      f.structured_annotations.begin(), f.structured_annotations.end());
}
} // namespace apache::thrift::detail::md
