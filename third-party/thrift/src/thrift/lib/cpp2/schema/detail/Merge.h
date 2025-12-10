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

#include <thrift/lib/cpp2/schema/SchemaV1.h>

#ifdef THRIFT_SCHEMA_AVAILABLE

#include <unordered_set>

namespace apache::thrift::schema::detail {

// Helpers for working with schemas.
type::Schema mergeSchemas(folly::Range<const std::string_view*> schemas);
type::Schema mergeSchemas(std::vector<type::Schema>&& schemas);

// Includes omnibus schema (e.g. thrift/lib/thrift/*) if available.
type::Schema loadBundledSchema(folly::Range<const std::string_view*> schemas);

void mergeInto(
    type::Schema& dst,
    type::Schema&& src,
    std::unordered_set<type::ProgramId>& includedPrograms,
    bool allowDuplicateDefinitionKeys);
std::optional<type::Schema> readSchema(std::string_view data);

} // namespace apache::thrift::schema::detail

#endif
