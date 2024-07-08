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

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Runtime.h>
#include <thrift/lib/cpp2/type/TypeRegistry.h>
#include <thrift/lib/rust/any/tests/if/gen-cpp2/test_structs_types.h>
#include <thrift/lib/rust/any/tests/src/cpp.h>
#include "icsp/lib/Compression.h"

namespace thrift::rust::thrift_any {

template <typename T>
std::unique_ptr<std::string> to_any(const std::string& compact_serialized_t) {
  //
  // deserialize T from compact
  //
  T t_obj =
      apache::thrift::CompactSerializer::deserialize<T>(compact_serialized_t);
  //
  // convert Basic to Any
  //
  apache::thrift::type::AnyStruct any =
      apache::thrift::type::TypeRegistry::generated()
          .store<apache::thrift::type::StandardProtocol::Compact>(t_obj)
          .toThrift();
  //
  // serialize Any to compact
  //
  std::string compact_any =
      apache::thrift::CompactSerializer::serialize<std::string>(any);

  return std::make_unique<std::string>(std::move(compact_any));
}

template <typename T>
std::unique_ptr<std::string> from_any(const std::string& any) {
  //
  // deserialize Any from compact
  //
  const apache::thrift::type::AnyStruct any_obj =
      apache::thrift::CompactSerializer::deserialize<
          apache::thrift::type::AnyStruct>(any);
  apache::thrift::type::AnyData anydata(any_obj);
  //
  // convert Any to T
  //
  T t_obj;
  apache::thrift::type::TypeRegistry::generated().load(anydata, t_obj);
  //
  // serialize T to compact
  //
  std::string compact_t =
      apache::thrift::CompactSerializer::serialize<std::string>(t_obj);

  return std::make_unique<std::string>(std::move(compact_t));
}

std::unique_ptr<std::string> basic_to_any(const std::string& basic) {
  return to_any<cpp2::Basic>(basic);
}

std::unique_ptr<std::string> any_to_basic(const std::string& any) {
  return from_any<cpp2::Basic>(any);
}

std::unique_ptr<std::string> simple_union_to_any(
    const std::string& simple_union) {
  return to_any<cpp2::SimpleUnion>(simple_union);
}

std::unique_ptr<std::string> any_to_simple_union(const std::string& any) {
  return from_any<cpp2::SimpleUnion>(any);
}

std::unique_ptr<std::string> compress_any(const std::string& any) {
  const apache::thrift::type::AnyStruct any_obj =
      apache::thrift::CompactSerializer::deserialize<
          apache::thrift::type::AnyStruct>(any);
  apache::thrift::type::AnyData anydata(any_obj);

  anydata = facebook::icsp::compressAny(anydata);

  std::string compact_t =
      apache::thrift::CompactSerializer::serialize<std::string>(
          anydata.toThrift());

  return std::make_unique<std::string>(std::move(compact_t));
}
std::unique_ptr<std::string> decompress_any(const std::string& any) {
  const apache::thrift::type::AnyStruct any_obj =
      apache::thrift::CompactSerializer::deserialize<
          apache::thrift::type::AnyStruct>(any);
  apache::thrift::type::AnyData anydata(any_obj);

  anydata = facebook::icsp::decompressAny(anydata);

  std::string compact_t =
      apache::thrift::CompactSerializer::serialize<std::string>(
          anydata.toThrift());

  return std::make_unique<std::string>(std::move(compact_t));
}

} // namespace thrift::rust::thrift_any
