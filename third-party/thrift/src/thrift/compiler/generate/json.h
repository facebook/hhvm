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

#include <iosfwd>
#include <string>

namespace apache {
namespace thrift {
namespace compiler {

class t_const_value;

//  json_quote_ascii
//
//  Emits a json quoted-string given an input ascii string.
std::string json_quote_ascii(const std::string& s);
std::ostream& json_quote_ascii(std::ostream& o, const std::string& s);

/**
 * Serialize t_const_value to JSON string.
 * The t_const_value is usually thrift annotation or metadata
 *
 * Example:
 * @MyAnnotation{
 *   my_bool = true,
 *   my_string = "hello",
 * }
 * struct MyStruct{}
 *
 * The annotation above will be serialized to:
 * "{\"my_bool\": true, \"my_string\": \"hello\"}"
 *
 * The output is a JSON string that may not contain newlines and indents.
 */
std::string to_json(const t_const_value* value);

} // namespace compiler
} // namespace thrift
} // namespace apache
