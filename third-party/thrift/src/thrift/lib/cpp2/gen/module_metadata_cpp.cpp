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

namespace apache {
namespace thrift {
namespace detail {
namespace md {

ThriftConstValue cvBool(bool value) {
  ThriftConstValue ret;
  ret.cv_bool_ref() = value;
  return ret;
}

ThriftConstValue cvInteger(int64_t value) {
  ThriftConstValue ret;
  ret.cv_integer_ref() = value;
  return ret;
}

ThriftConstValue cvDouble(double value) {
  ThriftConstValue ret;
  ret.cv_double_ref() = value;
  return ret;
}

ThriftConstValue cvString(const char* value) {
  ThriftConstValue ret;
  ret.cv_string_ref() = value;
  return ret;
}

ThriftConstValue cvMap(std::vector<ThriftConstValuePair>&& value) {
  ThriftConstValue ret;
  ret.cv_map_ref() = std::move(value);
  return ret;
}

ThriftConstValue cvList(std::vector<ThriftConstValue>&& value) {
  ThriftConstValue ret;
  ret.cv_list_ref() = std::move(value);
  return ret;
}

ThriftConstValue cvStruct(
    const char* name, std::map<std::string, ThriftConstValue>&& fields) {
  ThriftConstValue ret;
  ThriftConstStruct s;
  s.type_ref()->name_ref() = name;
  s.fields_ref() = std::move(fields);
  ret.cv_struct_ref() = std::move(s);
  return ret;
}

ThriftConstValuePair cvPair(ThriftConstValue&& key, ThriftConstValue&& value) {
  ThriftConstValuePair pair;
  pair.key_ref() = std::move(key);
  pair.value_ref() = std::move(value);
  return pair;
}

} // namespace md
} // namespace detail
} // namespace thrift
} // namespace apache
