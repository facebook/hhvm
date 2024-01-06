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

#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/type/Any.h>

namespace apache::thrift {

namespace detail {

class AnyDebugWriter : public DebugProtocolWriter {
 public:
  explicit AnyDebugWriter(bool try_unregistered_structs_as_any)
      : try_unregistered_structs_as_any_(try_unregistered_structs_as_any) {}
  uint32_t write(const type::AnyStruct& any);

  uint32_t write(const type::AnyData& any);

 private:
  uint32_t writeUnregisteredAny(const type::AnyStruct& any);

  template <class ProtocolReader>
  uint32_t writeUnregisteredAnyImpl(
      ProtocolReader& reader, const type::BaseType& type);

  template <class ProtocolReader, typename Tag>
  type::native_type<Tag> decode(ProtocolReader& reader);

  template <class ProtocolReader>
  bool tryAsAny(ProtocolReader& reader);

  bool try_unregistered_structs_as_any_;
};
} // namespace detail

/*
 * Returns string representation of the Thrift Any object that is
 * human readable and nicely indented. Binary data inside the AnyStruct is
 * deserialized without schema, based on the Type information provided in
 * AnyStruct.
 *
 * `try_unregistered_structs_as_any` : When enabled, this flag triggers an
 * attempt to convert each struct into an AnyStruct during schemaless
 * deserialization of a struct. If a valid, non-empty AnyStruct is identified,
 * it is treated as a nested AnyStruct and it's binary data will then
 * recursively be deserialized into a human-readable format.
 *
 * It is write-only now and cannot deserialize from such a string. There is no
 * guarantee that the format won't change, it might be evolved in a
 * non-backward compatible way. It should only be used for logging.
 *
 * Use this instead of DebugProtocol (or debugStringViaEncode method)
 * if you need to debug the data stored within Thrift Any.
 * The DebugProtocol will print a complex and lengthy string containing
 * binary blob, which can be difficult to read.
 *
 */
std::string anyDebugString(
    const type::AnyStruct& obj, bool try_unregistered_structs_as_any = false);

std::string anyDebugString(
    const type::AnyData& obj, bool try_unregistered_structs_as_any = false);
} // namespace apache::thrift
