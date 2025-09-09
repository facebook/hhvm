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

#include <cstddef>

namespace apache::thrift::compiler {

// Total number of templates
extern const std::size_t templates_size;

// Lengths of each template's name, e.g. `lang/xyz.mustache` -> 17
extern const std::size_t templates_name_sizes[];

// Names of each template, e.g. `lang/xyz.mustache`
extern const char* const templates_name_datas[];

// Lengths of each template's content, e.g. `...` -> 3
extern const std::size_t templates_content_sizes[];

// Raw string content of each template, e.g. `...`
extern const char* const templates_content_datas[];

} // namespace apache::thrift::compiler
