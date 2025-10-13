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
#include <thrift/compiler/sema/schematizer.h>
#include <thrift/lib/thrift/gen-cpp2/schema_types.h>

namespace apache::thrift::compiler {
class source_manager;
class t_program;
namespace detail {
type::Schema gen_schema(
    schematizer::options& schema_opts,
    source_manager& source_mgr,
    const t_program& root_program);
}
} // namespace apache::thrift::compiler
