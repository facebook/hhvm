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

#include <thrift/lib/cpp/protocol/TType.h>

#include <fmt/format.h>

namespace apache::thrift::protocol {

std::string debugStringForTType(TType type) {
  switch (type) {
    case T_STOP:
      return "STOP";
    case T_VOID:
      return "VOID";
    case T_BOOL:
      return "BOOL";
    case T_BYTE:
      return "BYTE";
    case T_DOUBLE:
      return "DOUBLE";
    case T_I16:
      return "I16";
    case T_I32:
      return "I32";
    case T_U64:
      return "U64";
    case T_I64:
      return "I64";
    case T_STRING:
      return "STRING";
    case T_STRUCT:
      return "STRUCT";
    case T_MAP:
      return "MAP";
    case T_SET:
      return "SET";
    case T_LIST:
      return "LIST";
    case T_UTF8:
      return "UTF8";
    case T_UTF16:
      return "UTF16";
    case T_STREAM:
      return "STREAM";
    case T_FLOAT:
      return "FLOAT";
    default:
      return fmt::format("UNKNOWN({})", static_cast<uint8_t>(type));
  }
}

} // namespace apache::thrift::protocol

auto fmt::formatter<apache::thrift::protocol::TType>::format(
    apache::thrift::protocol::TType type, format_context& ctx) const
    -> format_context::iterator {
  return formatter<std::string>::format(
      apache::thrift::protocol::debugStringForTType(type), ctx);
}
