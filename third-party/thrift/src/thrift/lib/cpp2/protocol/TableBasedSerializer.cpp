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

#include <thrift/lib/cpp2/protocol/TableBasedSerializerImpl.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

namespace apache::thrift::detail {

#define THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(                          \
    TypeClass, Type, ThriftType, TTypeValue)                           \
  const TypeInfo TypeToInfo<type_class::TypeClass, Type>::typeInfo = { \
      protocol::TType::TTypeValue,                                     \
      get<ThriftType, Type>,                                           \
      eraseFuncPtr(set<Type, ThriftType>),                             \
      nullptr,                                                         \
  }

// Specialization for numbers.
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int8_t, std::int8_t, T_BYTE);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int16_t, std::int16_t, T_I16);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int32_t, std::int32_t, T_I32);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::int64_t, std::int64_t, T_I64);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint8_t, std::int8_t, T_BYTE);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint16_t, std::int16_t, T_I16);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint32_t, std::int32_t, T_I32);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(
    integral, std::uint64_t, std::int64_t, T_I64);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(integral, bool, bool, T_BOOL);

THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(floating_point, float, float, T_FLOAT);
THRIFT_DEFINE_PRIMITIVE_TYPE_TO_INFO(floating_point, double, double, T_DOUBLE);

// Specialization for string.
#define THRIFT_DEFINE_STRING_TYPE_TO_INFO(TypeClass, ActualType, ExtVal)     \
  const StringFieldType TypeToInfo<type_class::TypeClass, ActualType>::ext = \
      ExtVal;                                                                \
  const TypeInfo TypeToInfo<type_class::TypeClass, ActualType>::typeInfo = { \
      /* .type */ protocol::TType::T_STRING,                                 \
      /* .get */ nullptr,                                                    \
      /* .set */ nullptr,                                                    \
      /* .typeExt */ &ext,                                                   \
  }

THRIFT_DEFINE_STRING_TYPE_TO_INFO(string, std::string, StringFieldType::String);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(
    string, folly::fbstring, StringFieldType::String);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(binary, std::string, StringFieldType::Binary);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(
    binary, folly::fbstring, StringFieldType::Binary);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(binary, folly::IOBuf, StringFieldType::IOBuf);
THRIFT_DEFINE_STRING_TYPE_TO_INFO(
    binary, std::unique_ptr<folly::IOBuf>, StringFieldType::IOBufPtr);

template void read<CompactProtocolReader>(
    CompactProtocolReader* iprot, const StructInfo& structInfo, void* object);
template size_t write<CompactProtocolWriter>(
    CompactProtocolWriter* iprot,
    const StructInfo& structInfo,
    const void* object);
template void read<BinaryProtocolReader>(
    BinaryProtocolReader* iprot, const StructInfo& structInfo, void* object);
template size_t write<BinaryProtocolWriter>(
    BinaryProtocolWriter* iprot,
    const StructInfo& structInfo,
    const void* object);
template void read<SimpleJSONProtocolReader>(
    SimpleJSONProtocolReader* iprot,
    const StructInfo& structInfo,
    void* object);
template size_t write<SimpleJSONProtocolWriter>(
    SimpleJSONProtocolWriter* iprot,
    const StructInfo& structInfo,
    const void* object);
template void read<JSONProtocolReader>(
    JSONProtocolReader* iprot, const StructInfo& structInfo, void* object);
template size_t write<JSONProtocolWriter>(
    JSONProtocolWriter* iprot,
    const StructInfo& structInfo,
    const void* object);

} // namespace apache::thrift::detail
