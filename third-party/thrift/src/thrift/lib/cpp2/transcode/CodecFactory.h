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

#include <thrift/lib/cpp2/transcode/Codec.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>

#include <span>
#include <string_view>

namespace apache::thrift::type_system {
class StructNode;
class UnionNode;
} // namespace apache::thrift::type_system

namespace apache::thrift::transcode {

/**
 * Produce codecs for specific protocols from Thrift TypeSystem schemas.
 */
Codec makeThriftCompactCodec(const type_system::StructNode& node);
Codec makeThriftCompactCodec(const type_system::UnionNode& node);
Codec makeThriftCompactCodec(
    std::string_view name,
    std::span<const type_system::FieldDefinition> fields);
Codec makeThriftBinaryCodec(const type_system::StructNode& node);
Codec makeThriftBinaryCodec(const type_system::UnionNode& node);
Codec makeThriftBinaryCodec(
    std::string_view name,
    std::span<const type_system::FieldDefinition> fields);
Codec makeProtobufBinaryCodec(const type_system::StructNode& node);
Codec makeProtobufBinaryCodec(const type_system::UnionNode& node);
Codec makeProtobufBinaryCodec(
    std::string_view name,
    std::span<const type_system::FieldDefinition> fields);
Codec makeJsonCodec(const type_system::StructNode& node);
Codec makeJsonCodec(const type_system::UnionNode& node);
Codec makeJsonCodec(
    std::string_view name,
    std::span<const type_system::FieldDefinition> fields);

} // namespace apache::thrift::transcode
