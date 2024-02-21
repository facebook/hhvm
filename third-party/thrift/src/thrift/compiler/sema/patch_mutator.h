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

#include <string>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/sema/ast_mutator.h>
#include <thrift/compiler/sema/diagnostic_context.h>

namespace apache {
namespace thrift {
namespace compiler {

// Adds the patch mutators to the plugin stage of the given mutators.
void add_patch_mutators(ast_mutators& mutators);

// A class that can generate different types of patch representations
// and add them into the given program.
class patch_generator {
 public:
  // Gets the patch_generator for the given context.
  static patch_generator& get_for(
      diagnostic_context& ctx, mutator_context& mctx);

  explicit patch_generator(diagnostic_context& ctx, t_program& program)
      : ctx_(ctx), program_(program) {}

  // Add a struct with fields 1:1 with the given node, attributing the new node
  // to the given annotation, and return a reference to it.
  //
  // The fields in the generated struct have the same id and name as the
  // original field, but the type is replaced with an associated patch type. For
  // example, for
  //
  //   struct Bar {
  //     1: Foo myFoo;
  //   }
  //
  // the following struct is generated:
  //
  //   struct BarPatch {
  //     1: FooPatch myFoo;
  //   }
  //
  t_struct& add_field_patch(const t_const& annot, t_structured& node);

  // Add a value patch representation for the given struct and associate patch
  // type, and return a reference to it.
  //
  // The resulting struct has the form:
  //
  //   struct StructValuePatch<Value, Patch> {
  //     // Assigns a value. If set, all other operations are ignored.
  //     optional Value assign (thrift.box);
  //
  //     // Clears a value. Applies first.
  //     bool clear;
  //
  //     // Patches any set value. Applies second.
  //     Patch patch;
  //
  //     // The value to use for each field, if not present.
  //     Value ensure;
  //
  //     // Patches any set value, including newly set values. Applies fourth.
  //     Patch patchAfter;
  //   }
  //
  t_struct& add_struct_patch(const t_const& annot, t_structured& value_type);

  // Add a value patch representation for the given union and associate patch
  // type, and return a reference to it.
  //
  // The resulting struct has the form:
  //
  //   struct UnionValuePatch<Value, Patch> {
  //     // Clears any set value. Applies first.
  //     bool clear;
  //
  //     // Patches any set value. Applies second.
  //     Patch patch;
  //
  //     // Assigns the field, if not already set. Applies third.
  //     Value ensure;
  //
  //     // Patches any set value, including newly set values. Applies fourth.
  //     Patch patchAfter;
  //   }
  //
  t_struct& add_union_patch(const t_const& annot, t_union& value_type);

  t_struct& add_ensure_struct(const t_const& annot, t_structured& node);

 private:
  friend class PatchGeneratorTest;

  diagnostic_context& ctx_;
  t_program& program_;
  // TODO(afuller): Sort by decl order not offset in
  // thrift/compiler/lib/cpp2/util.cc and remove this hack.
  uint_least32_t count_ = 0;

  // Adds a new struct to the program, and return a reference to it.
  t_struct& gen_struct(const t_node& annot, std::string name, std::string uri);
  t_struct& gen_suffix_struct(
      const t_node& annot, const t_named& orig, const char* suffix);

  t_struct& gen_patch(
      const t_const& annot,
      const t_structured& orig,
      const t_field_id& field_id,
      t_type_ref type,
      size_t traversal_order = 0);

  // Attempts to resolve the associated patch type for the given field.
  //
  // If a shared representation cannot be found, a new one may be generated.
  // Otherwise an empty t_type_ref is returned.
  t_type_ref find_patch_type(
      const t_const& annot,
      const t_structured& parent,
      t_type_ref type,
      const t_field_id& field_id,
      size_t traversal_order = 0);
  t_type_ref find_patch_type(
      const t_const& annot, const t_structured& parent, const t_field& field);

  t_type_ref find_patch_override(t_type_ref type) const;
  t_type_ref find_patch_override(const t_named& node) const;
  t_type_ref find_patch_override(
      const t_node& node, const std::string& uri) const;

  // Injects prefix immediately after the last '/'
  //
  // For example: my/path/Name -> my/path/{prefix}Name
  static std::string prefix_uri_name(
      const std::string& uri, const char* prefix) {
    size_t pos = uri.rfind('/') + 1;
    return pos > 0 ? uri.substr(0, pos) + prefix + uri.substr(pos) : "";
  }

  t_type_ref inst_list(t_type_ref val);
  t_type_ref inst_set(t_type_ref key);
  t_type_ref inst_map(t_type_ref val, t_type_ref key);

  const t_const* get_assign_only_annotation_or_null(const t_named& node);

  const t_const& get_field_annotation(
      const t_const& annot, const t_field& field);
};

} // namespace compiler
} // namespace thrift
} // namespace apache
