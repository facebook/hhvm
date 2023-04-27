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
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <thrift/lib/cpp2/type/UniversalHashAlgorithm.h>

#include "rust/cxx.h" // @manual=fbsource//third-party/rust:cxx-core

namespace fbthrift_conformance::rust {

std::unique_ptr<std::string> getUniversalHash(
    apache::thrift::type::UniversalHashAlgorithm alg, const std::string& uri);

std::unique_ptr<std::string> getUniversalHashPrefix(
    const std::string& universalHash, int8_t hashBytes);

bool matchesUniversalHash(
    const std::string& universalHash, const std::string& prefix);

} // namespace fbthrift_conformance::rust
