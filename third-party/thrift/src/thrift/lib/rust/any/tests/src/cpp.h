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
#include "rust/cxx.h"

namespace thrift::rust::thrift_any {

std::unique_ptr<std::string> basic_to_any(const std::string& basic);
std::unique_ptr<std::string> any_to_basic(const std::string& any);

std::unique_ptr<std::string> simple_union_to_any(
    const std::string& simple_union);
std::unique_ptr<std::string> any_to_simple_union(const std::string& any);

std::unique_ptr<std::string> compress_any(const std::string& any);
std::unique_ptr<std::string> decompress_any(const std::string& any);

} // namespace thrift::rust::thrift_any
