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

#include <thrift/compiler/sema/patch_mutator.h>

#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/lib/uri.h>
#include <thrift/compiler/sema/standard_mutator_stage.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

constexpr auto kGeneratePatchUri = "facebook.com/thrift/op/GeneratePatch";

// TODO(afuller): Index all types by uri, and find them that way.
const char* getPatchTypeName(t_base_type::type base_type) {
  switch (base_type) {
    case t_base_type::type::t_bool:
      return "patch.BoolPatch";
    case t_base_type::type::t_byte:
      return "patch.BytePatch";
    case t_base_type::type::t_i16:
      return "patch.I16Patch";
    case t_base_type::type::t_i32:
      return "patch.I32Patch";
    case t_base_type::type::t_i64:
      return "patch.I64Patch";
    case t_base_type::type::t_float:
      return "patch.FloatPatch";
    case t_base_type::type::t_double:
      return "patch.DoublePatch";
    case t_base_type::type::t_string:
      return "patch.StringPatch";
    case t_base_type::type::t_binary:
      return "patch.BinaryPatch";
    default:
      return "";
  }
}

std::string getSibName(const std::string& sibling, const std::string& name) {
  return sibling.substr(0, sibling.find_last_of("/")) + name;
}

// A fluent function to set the doc string on a given node.
template <typename N>
N& doc(std::string txt, N& node) {
  node.set_doc(std::move(txt) + "\n");
  return node;
}

// A fluent function to box a given field.
t_field& box(t_field& node) {
  node.set_qualifier(t_field_qualifier::optional);
  // Box the field, if the underlying type is a struct.
  if (dynamic_cast<const t_struct*>(node.type()->get_true_type())) {
    node.set_annotation("thrift.box");
  }
  return node;
}

// Helper for generating a struct.
struct StructGen {
  // The annotation we are generating for.
  const t_node& annot;
  // The struct to add fields to.
  t_struct& generated;

  // Add a new field to generated, and return it.
  t_field& field(t_field_id id, t_type_ref type, std::string name) {
    generated.append_field(
        std::make_unique<t_field>(type, std::move(name), id));
    t_field& result = generated.fields().back();
    // TODO(afuller): Make terse when supported.
    // result.set_qualifier(t_field_qualifier::terse);
    result.set_src_range(annot.src_range());
    return result;
  }

  t_struct* operator->() { return &generated; }
  operator t_struct&() { return generated; }
  operator t_type_ref() { return generated; }

  void set_adapter(std::string name, t_program& program) {
    auto annotation =
        dynamic_cast<const t_type*>(program.scope()->find_def(kCppAdapterUri));
    assert(annotation); // transitive include from patch.thrift
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    value->add_map(
        std::make_unique<t_const_value>("name"),
        std::make_unique<t_const_value>(
            "::apache::thrift::op::detail::" + std::move(name)));
    value->add_map(
        std::make_unique<t_const_value>("underlyingName"),
        std::make_unique<t_const_value>(generated.name() + "Struct"));
    value->add_map(
        std::make_unique<t_const_value>("extraNamespace"),
        std::make_unique<t_const_value>(""));
    value->set_ttype(*annotation);
    auto adapter =
        std::make_unique<t_const>(&program, annotation, "", std::move(value));
    generated.add_structured_annotation(std::move(adapter));
  }
};

// Helper for generating patch structs.
struct PatchGen : StructGen {
  // Standardized patch field ids.
  enum t_patch_field_id : t_field_id {
    // Common ops.
    kAssignId = 1,
    kClearId = 2,

    // Patch ops.
    kPatchPriorId = 3, // Union, Struct, Map, List
    kPatchAfterId = 6, // Union, Struct, Map, List

    // Initialization ops.
    kEnsureUnionId = 4, // Union
    kEnsureStructId = 5, // Struct

    // Container ops.
    kRemoveId = 7, // Set, Map, List
    kAddSetId = 8, // Set
    kAddMapId = 5, // Map
    kPutId = 9, // Map(, Set)

    // Sequence operators.
    kPrependId = 8, // List, String, Binary
    kAppendId = 9, // List, String, Binary
  };

  // {kAssignId}: {type} assign;
  t_field& assignUnion(t_type_ref type) {
    return doc(
        "Assigns a value. If set, all other operations are ignored.",
        field(kAssignId, type, "assign"));
  }
  // {kAssignId}: optional {type} assign (thrift.box);
  t_field& assign(t_type_ref type) { return box(assignUnion(type)); }

  // {kClearId}: bool clear;
  t_field& clear() {
    return doc(
        "Clears a value. Applies first.",
        field(kClearId, t_base_type::t_bool(), "clear"));
  }
  t_field& clearUnion() {
    return doc("Clears any set value. Applies first.", clear());
  }

  // {kPatchPriorId}: {patch_type} patch;
  t_field& patchPrior(t_type_ref patch_type) {
    return doc(
        "Patches any previously set values. Applies second.",
        field(kPatchPriorId, patch_type, "patchPrior"));
  }

  // {kRemoveId}: {type} remove;
  t_field& remove(t_type_ref type) {
    return doc(
        "Removes entries, if present. Applies third.",
        field(kRemoveId, type, "remove"));
  }

  // {kEnsureUnionId}: {type} ensure;
  t_field& ensureUnion(t_type_ref type) {
    return doc(
        "Assigns the value, if not already set to the same field. Applies third.",
        field(kEnsureUnionId, type, "ensure"));
  }

  // {kEnsureStructId}: {type} ensure;
  t_field& ensureStruct(t_type_ref type) {
    return doc(
        "Initialize fields, using the given defaults. Applies third.",
        field(kEnsureStructId, type, "ensure"));
  }

  // {kAddMapId}: {type} ensure;
  t_field& addMap(t_type_ref type) {
    return doc(
        "Add the given values, if the keys are not already present. Applies forth.",
        field(kAddMapId, type, "add"));
  }

  // {kAddId}: {type} add;
  t_field& addSet(t_type_ref type) {
    return doc(
        "Adds entries, if not already present. Applies fourth.",
        field(kAddSetId, type, "add"));
  }

  // {kPutId}: {type} put;
  t_field& put(t_type_ref type) {
    return doc(
        "Adds or replaces the given key/value pairs. Applies fifth.",
        field(kPutId, type, "put"));
  }

  // {kPrependId}: {type} prepend;
  t_field& prepend(t_type_ref type) {
    return doc(
        "Prepends to the front of a given list.",
        field(kPrependId, type, "prepend"));
  }

  // {kAppendId}: {type} append;
  t_field& append(t_type_ref type) {
    return doc(
        "Appends to the back of a given list.",
        field(kAppendId, type, "append"));
  }

  // {kPatchAfterId}: {patch_type} patch;
  t_field& patchAfter(t_type_ref patch_type) {
    return doc(
        "Patches any set value, including newly set values. Applies last.",
        field(kPatchAfterId, patch_type, "patch"));
  }
};

// Generates a patch representation for any struct with the @patch.GeneratePatch
// annotation.
void generate_struct_patch(
    diagnostic_context& ctx, mutator_context& mctx, t_struct& node) {
  if (auto* annot =
          ctx.program().inherit_annotation_or_null(node, kGeneratePatchUri)) {
    // Add a 'field patch' and 'struct patch' using it.
    auto& generator = patch_generator::get_for(ctx, mctx);
    generator.add_struct_patch(
        *annot, node, generator.add_field_patch(*annot, node));
  }
}

void generate_union_patch(
    diagnostic_context& ctx, mutator_context& mctx, t_union& node) {
  if (auto* annot =
          ctx.program().inherit_annotation_or_null(node, kGeneratePatchUri)) {
    // Add a 'field patch' and 'union patch' using it.
    auto& generator = patch_generator::get_for(ctx, mctx);
    generator.add_union_patch(
        *annot, node, generator.add_field_patch(*annot, node));
  }
}

} // namespace

void add_patch_mutators(ast_mutators& mutators) {
  auto& mutator = mutators[standard_mutator_stage::plugin];
  mutator.add_struct_visitor(&generate_struct_patch);
  mutator.add_union_visitor(&generate_union_patch);
}

patch_generator& patch_generator::get_for(
    diagnostic_context& ctx, mutator_context& mctx) {
  t_program& program = dynamic_cast<t_program&>(*mctx.root());
  return ctx.cache().get(program, [&]() {
    return std::make_unique<patch_generator>(ctx, program);
  });
}

t_struct& patch_generator::add_field_patch(
    const t_const& annot, t_structured& orig) {
  StructGen gen{annot, gen_suffix_struct(annot, orig, "FieldPatch")};
  for (const auto& field : orig.fields()) {
    if (t_type_ref patch_type = find_patch_type(annot, orig, field)) {
      gen.field(field.id(), patch_type, field.name());
    } else {
      ctx_.warning(field, "Could not resolve patch type for field.");
    }
  }
  gen.set_adapter("FieldPatchAdapter", program_);
  return gen;
}

t_struct& patch_generator::add_union_patch(
    const t_node& annot, t_union& value_type, t_type_ref patch_type) {
  PatchGen gen{{annot, gen_suffix_struct(annot, value_type, "Patch")}};
  gen.assignUnion(value_type);
  gen.clearUnion();
  gen.patchPrior(patch_type);
  gen.ensureUnion(value_type);
  gen.patchAfter(patch_type);
  gen.set_adapter("UnionPatchAdapter", program_);
  return gen;
}

t_struct& patch_generator::add_struct_patch(
    const t_node& annot, t_struct& value_type, t_type_ref patch_type) {
  PatchGen gen{{annot, gen_suffix_struct(annot, value_type, "Patch")}};
  gen.assign(value_type);
  gen.clear();
  gen.patchPrior(patch_type);
  gen.ensureStruct(value_type);
  gen.patchAfter(patch_type);
  gen.set_adapter("StructPatchAdapter", program_);
  return gen;
}

t_type_ref patch_generator::find_patch_type(
    const t_const& annot, const t_structured& parent, const t_field& field) {
  // Base types use a shared representation defined in patch.thrift.
  const auto* type = field.type()->get_true_type();
  if (auto* base_type = dynamic_cast<const t_base_type*>(type)) {
    const char* name = getPatchTypeName(base_type->base_type());
    if (const auto* result = program_.scope()->find_type(name)) {
      return t_type_ref::from_ptr(result);
    }

    // TODO(afuller): This look up hack only works for 'built-in' patch types.
    // Use a shared uri type registry instead.
    t_type_ref result = annot.type()->program()->scope()->ref_type(
        *annot.type()->program(), name, field.src_range());
    if (auto* ph = result.get_unresolved_type()) {
      // Set the location info, in case the type can't be resolved later.
      ph->set_src_range(field.src_range());
      ph->set_generated();
    }

    return result;
  }

  // Check the field for a custom patch type.
  if (auto* custom = field.find_annotation_or_null("thrift.patch.uri")) {
    if (const auto* result =
            dynamic_cast<const t_type*>(program_.scope()->find_def(*custom))) {
      return t_type_ref::from_ptr(result);
    }
    ctx_.warning(field, "Could not find custom type: {}", *custom);
  }

  // Check the field type for a custom patch type.
  if (auto* custom = t_typedef::get_first_annotation_or_null(
          field.type().get_type(), {"thrift.patch.uri"})) {
    if (const auto* result =
            dynamic_cast<const t_type*>(program_.scope()->find_def(*custom))) {
      return t_type_ref::from_ptr(result);
    }
    ctx_.warning(*field.type(), "Could not find custom type: {}", *custom);
  }

  if (auto* structured = dynamic_cast<const t_structured*>(type)) {
    std::string name = structured->name() + "Patch";
    if (!structured->uri().empty()) { // Try to look up by URI.
      if (auto* result = dynamic_cast<const t_type*>(program_.scope()->find_def(
              getSibName(structured->uri(), name)))) {
        return t_type_ref::from_ptr(result);
      }
    }

    // Try to look up by Name.
    // Look for it in the same program as the type itself.
    t_type_ref result = program_.scope()->ref_type(
        *structured->program(), name, field.src_range());
    if (auto* ph = result.get_unresolved_type()) {
      // Set the location info, in case the type can't be resolved later.
      ph->set_src_range(field.src_range());
      ph->set_generated();
    }
    return result;
  }

  // Could not resolve a shared patch type, so generate a field specific one.
  // Give it a stable name.
  std::string suffix = "Field" + std::to_string(field.id()) + "Patch";
  PatchGen gen{{annot, gen_suffix_struct(annot, parent, suffix.c_str())}};
  // All value patches have an assign and clear field.
  gen.assign(field.type());
  gen.clear();
  if (auto* container = dynamic_cast<const t_container*>(type)) {
    switch (container->container_type()) {
      case t_container::type::t_list:
        // TODO(afuller): support 'patch'.
        // TODO(afuller): support 'remove' op.
        // TODO(afuller): support 'replace' op.
        gen.prepend(field.type());
        gen.append(field.type());
        gen.set_adapter("ListPatchAdapter", program_);
        break;
      case t_container::type::t_set:
        // TODO(afuller): support 'replace' op.
        gen.remove(field.type());
        gen.addSet(field.type());
        gen.set_adapter("SetPatchAdapter", program_);
        break;
      case t_container::type::t_map:
        // TODO(afuller): support 'patch' op.
        // TODO(afuller): support 'remove' op.
        // TODO(afuller): support 'removeIf' op.
        // TODO(afuller): support 'replace' op.
        gen.addMap(field.type());
        gen.put(field.type());
        gen.set_adapter("MapPatchAdapter", program_);
        break;
    }
  } else {
    gen.set_adapter("AssignPatchAdapter", program_);
  }
  return gen;
}

t_struct& patch_generator::gen_struct(
    const t_node& annot, std::string name, std::string uri) {
  auto generated = std::make_unique<t_struct>(&program_, std::move(name));
  generated->set_generated();
  generated->set_uri(std::move(uri));
  // Attribute the new struct to the anntation.
  generated->set_src_range(annot.src_range());
  program_.scope()->add_type(program_.scope_name(*generated), generated.get());
  return program_.add_def(std::move(generated));
}

t_struct& patch_generator::gen_suffix_struct(
    const t_node& annot, const t_named& orig, const char* suffix) {
  ctx_.check(!orig.uri().empty(), annot, "URI required to support patching.");
  t_struct& generated =
      gen_struct(annot, orig.name() + suffix, orig.uri() + suffix);
  if (const auto* cpp_name = orig.find_annotation_or_null("cpp.name")) {
    generated.set_annotation("cpp.name", *cpp_name + suffix);
  }
  return generated;
}

t_struct& patch_generator::gen_prefix_struct(
    const t_node& annot, const t_named& orig, const char* prefix) {
  t_struct& generated = gen_struct(
      annot, prefix + orig.name(), prefix_uri_name(orig.uri(), prefix));
  if (const auto* cpp_name = orig.find_annotation_or_null("cpp.name")) {
    generated.set_annotation("cpp.name", prefix + *cpp_name);
  }
  return generated;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
