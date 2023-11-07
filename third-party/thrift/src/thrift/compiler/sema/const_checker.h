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

#include <thrift/compiler/ast/t_const_value.h>

namespace apache {
namespace thrift {
namespace compiler {

class diagnostic_context;
class t_base_type;
class t_named;
class t_type;

// (ffrancet) I managed to trace this comment all the way back to 2008 when
// thrift was migrated to the fbcode repo. True piece of history here
/**
 * You know, when I started working on Thrift I really thought it wasn't going
 * to become a programming language because it was just a generator and it
 * wouldn't need runtime type information and all that jazz. But then we
 * decided to add constants, and all of a sudden that means runtime type
 * validation and inference, except the "runtime" is the code generator
 * runtime. Shit. I've been had.
 */
void check_const_rec(
    diagnostic_context& ctx,
    const t_named& node,
    const t_type* type,
    const t_const_value* value);

} // namespace compiler
} // namespace thrift
} // namespace apache
