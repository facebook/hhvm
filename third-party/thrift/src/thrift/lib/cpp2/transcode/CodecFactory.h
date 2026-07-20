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
Codec makeThriftBinaryCodec(const type_system::StructNode& node);
Codec makeThriftBinaryCodec(const type_system::UnionNode& node);
Codec makeProtobufBinaryCodec(const type_system::StructNode& node);
Codec makeProtobufBinaryCodec(const type_system::UnionNode& node);
Codec makeJsonCodec(const type_system::StructNode& node);
Codec makeJsonCodec(const type_system::UnionNode& node);

} // namespace apache::thrift::transcode
