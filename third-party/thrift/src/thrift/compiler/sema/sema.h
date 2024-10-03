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

#include <thrift/compiler/sema/sema_context.h>

namespace apache::thrift::compiler {

class t_program_bundle;

// Thrift semantic analyzer consisting of a sequence of mutation and validation
// stages.
struct sema {
 private:
  bool use_legacy_type_ref_resolution_;

  // Tries to resolve any unresolved type references, returning true if
  // successful and reporting any errors via diags.
  bool resolve_all_types(sema_context& diags, t_program_bundle& bundle);

 public:
  explicit sema(bool use_legacy_type_ref_resolution)
      : use_legacy_type_ref_resolution_(use_legacy_type_ref_resolution) {}

  struct result {
    bool unresolved_types = false;
  };

  result run(sema_context& ctx, t_program_bundle& bundle);

  // Adds schema to the to the root program.
  static void add_schema(sema_context& ctx, t_program_bundle& bundle);
};

} // namespace apache::thrift::compiler
