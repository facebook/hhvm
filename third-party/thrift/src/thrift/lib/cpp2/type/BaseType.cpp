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

#include <thrift/lib/cpp2/type/BaseType.h>

namespace apache::thrift::type {

std::string_view getBaseTypeName(BaseType type) noexcept {
  switch (type) {
    case BaseType::Void:
      return "void";
    case BaseType::Bool:
      return "bool";
    case BaseType::Byte:
      return "byte";
    case BaseType::I16:
      return "i16";
    case BaseType::Enum:
      return "enum";
    case BaseType::I32:
      return "i32";
    case BaseType::I64:
      return "i64";
    case BaseType::Double:
      return "double";
    case BaseType::Float:
      return "float";
    case BaseType::String:
      return "string";
    case BaseType::Binary:
      return "binary";

    case BaseType::List:
      return "list";
    case BaseType::Set:
      return "set";
    case BaseType::Map:
      return "map";

    case BaseType::Struct:
      return "struct";
    case BaseType::Union:
      return "union";
    case BaseType::Exception:
      return "exception";
    default:
      return "{unrecognized}";
  }
}

} // namespace apache::thrift::type
