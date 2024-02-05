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

#include <memory>

#include <fmt/core.h>

#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/gen/cpp/namespace_resolver.h>
#include <thrift/compiler/lib/cpp2/util.h>
#include <thrift/compiler/lib/uri.h>
#include <thrift/compiler/sema/standard_mutator_stage.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

t_field& rust_box(t_field& node) {
  node.set_annotation("rust.box");
  return node;
}

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

std::string getFieldIdPatchSuffix(
    const t_field_id& field_id, size_t traversal_order) {
  if (traversal_order == 0) {
    return fmt::format("Field{}Patch", field_id);
  }
  return fmt::format("Field{}Patch{}", field_id, traversal_order);
}

t_field_id getNormalizedFieldId(const t_field& field) {
  if (field.id() < 0) {
    return -field.id();
  }
  return field.id();
}

// A fluent function to set the doc string on a given node.
t_field& doc(std::string txt, t_field& node) {
  node.set_doc(std::move(txt) + "\n", node.src_range());
  return node;
}

// A fluent function to set the cpp.template string on a given node.
template <typename N>
std::unique_ptr<N> cpp_template(
    const std::string& cls, std::unique_ptr<N> node) {
  node->set_annotation("cpp.template", cls);
  return std::move(node);
}
template <typename N>
std::unique_ptr<N> rust_type(const std::string& cls, std::unique_ptr<N> node) {
  node->set_annotation("rust.type", cls);
  return std::move(node);
}

inline std::unique_ptr<t_set> unordered(std::unique_ptr<t_set> node) {
  return cpp_template(
      "::std::unordered_set", rust_type("HashSet", std::move(node)));
}
inline std::unique_ptr<t_map> unordered(std::unique_ptr<t_map> node) {
  return cpp_template(
      "::std::unordered_map", rust_type("HashMap", std::move(node)));
}

// Helper for generating a struct.
struct StructGen {
  // The annotation we are generating for.
  const t_node& annot;
  // The struct to add fields to.
  t_struct& generated;
  t_program& program_;

  // Add a new field to generated, and return it.
  t_field& field(t_field_id id, t_type_ref type, std::string name) {
    generated.append_field(
        std::make_unique<t_field>(type, std::move(name), id));
    t_field& result = generated.fields().back();
    result.set_qualifier(t_field_qualifier::terse);
    result.set_src_range(annot.src_range());
    return result;
  }

  t_struct* operator->() { return &generated; }
  operator t_struct&() { return generated; }
  operator t_type_ref() { return generated; }

  void add_frozen_exclude() {
    const t_type* annotation = dynamic_cast<const t_type*>(
        program_.scope()->find_by_uri(kCppFrozen2ExcludeUri));
    if (!annotation) {
      return;
    }
    auto value = t_const_value::make_map();
    value->set_ttype(*annotation);
    auto frozen_exclude =
        std::make_unique<t_const>(&program_, annotation, "", std::move(value));
    generated.add_structured_annotation(std::move(frozen_exclude));
  }

  void set_adapter(std::string name) {
    const t_type* annotation = dynamic_cast<const t_type*>(
        program_.scope()->find_by_uri(kCppAdapterUri));
    assert(annotation); // transitive include from patch.thrift
    auto value = t_const_value::make_map();
    auto ns = gen::cpp::namespace_resolver::gen_namespace(program_);
    value->add_map(
        std::make_unique<t_const_value>("name"),
        std::make_unique<t_const_value>(
            "::apache::thrift::op::detail::" + std::move(name) + "<" +
            std::move(ns) + "::" + generated.name() + "Struct" + ">"));
    value->add_map(
        std::make_unique<t_const_value>("underlyingName"),
        std::make_unique<t_const_value>(generated.name() + "Struct"));
    value->add_map(
        std::make_unique<t_const_value>("extraNamespace"),
        std::make_unique<t_const_value>(""));
    value->set_ttype(*annotation);
    auto adapter =
        std::make_unique<t_const>(&program_, annotation, "", std::move(value));
    generated.add_structured_annotation(std::move(adapter));
  }

  t_field& box(t_field& node) {
    node.set_qualifier(t_field_qualifier::optional);
    // Box the field, if the underlying type is a struct.
    if (!dynamic_cast<const t_struct*>(node.type()->get_true_type())) {
      return node;
    }

    const t_type* annotation =
        dynamic_cast<const t_type*>(program_.scope()->find_by_uri(kBoxUri));
    assert(annotation);

    auto value = t_const_value::make_map();
    value->set_ttype(*annotation);
    auto box_annot =
        std::make_unique<t_const>(&program_, annotation, "", std::move(value));
    node.add_structured_annotation(std::move(box_annot));
    return node;
  }

  t_field& intern_box(t_field& field) {
    const t_type* annotation = dynamic_cast<const t_type*>(
        program_.scope()->find_by_uri(kInternBoxUri));

    // Skip @thrift.InternBox optimization if the annotation is not found.
    if (!annotation) {
      return field;
    }

    auto value = t_const_value::make_map();
    value->set_ttype(*annotation);
    auto intern_box =
        std::make_unique<t_const>(&program_, annotation, "", std::move(value));
    field.add_structured_annotation(std::move(intern_box));
    return field;
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
  t_field& assign(t_type_ref type) {
    return box(
        doc("Assigns to a (set) value.\n\n"
            "If set, all other operations are ignored.\n\n"
            "Note: Optional and union fields must be set before assigned.\n",
            field(kAssignId, type, "assign")));
  }

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
    return intern_box(doc(
        "Assigns the value, if not already set to the same field. Applies third.",
        field(kEnsureUnionId, type, "ensure")));
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
    generator.add_struct_patch(*annot, node);
  }
}

void generate_union_patch(
    diagnostic_context& ctx, mutator_context& mctx, t_union& node) {
  if (auto* annot =
          ctx.program().inherit_annotation_or_null(node, kGeneratePatchUri)) {
    // Add a 'field patch' and 'union patch' using it.
    auto& generator = patch_generator::get_for(ctx, mctx);
    generator.add_union_patch(*annot, node);
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

const t_const* patch_generator::get_assign_only_annotation_or_null(
    const t_named& node) {
  return ctx_.program().inherit_annotation_or_null(node, kAssignOnlyPatchUri);
}

const t_const& patch_generator::get_field_annotation(
    const t_const& annot, const t_field& field) {
  if (auto* field_annot = get_assign_only_annotation_or_null(field)) {
    return *field_annot;
  }
  return annot;
}

t_struct& patch_generator::add_ensure_struct(
    const t_const& annot, t_structured& orig) {
  StructGen gen{
      annot, gen_suffix_struct(annot, orig, "EnsureStruct"), program_};
  gen.add_frozen_exclude();
  for (const t_field* field : orig.fields_id_order()) {
    if (!field->type().resolved()) {
      continue;
    }
    gen.box(gen.field(field->id(), field->type(), field->name()));
  }
  return gen;
}

t_struct& patch_generator::add_field_patch(
    const t_const& annot, t_structured& orig) {
  // Resolve (and maybe create) all the field patch types, strictly before
  // creating FieldPatch.
  std::map<t_field_id, t_type_ref> types; // Ordered by field id.
  for (const auto& field : orig.fields()) {
    if (!field.type().resolved()) {
      continue;
    }
    // Skip generating field patch for cpp.ref fields with cpp.RefType.Shared
    // and cpp.RefType.SharedMutable
    auto ref_type = gen::cpp::find_ref_type(field);
    if (ref_type == gen::cpp::reference_type::shared_const ||
        ref_type == gen::cpp::reference_type::shared_mutable) {
      continue;
    }

    if (t_type_ref patch_type =
            find_patch_type(get_field_annotation(annot, field), orig, field)) {
      types[field.id()] = patch_type;
    } else {
      ctx_.warning(field, "Could not resolve patch type for field.");
    }
  }
  StructGen gen{annot, gen_suffix_struct(annot, orig, "FieldPatch"), program_};
  for (const auto& entry : types) {
    gen.intern_box(gen.field(
        entry.first, entry.second, orig.get_field_by_id(entry.first)->name()));
  }
  gen.set_adapter("FieldPatchAdapter");
  return gen;
}

t_struct& patch_generator::add_union_patch(
    const t_const& annot, t_union& value_type) {
  PatchGen gen{
      {annot, gen_suffix_struct(annot, value_type, "Patch"), program_}};
  gen.assign(value_type);
  gen.clearUnion();
  if (get_assign_only_annotation_or_null(value_type)) {
    gen.set_adapter("AssignPatchAdapter");
    return gen;
  }
  t_type_ref patch_type = add_field_patch(annot, value_type);
  gen.patchPrior(patch_type);
  gen.ensureUnion(value_type);
  gen.patchAfter(patch_type);
  gen.set_adapter("UnionPatchAdapter");
  return gen;
}

t_struct& patch_generator::add_struct_patch(
    const t_const& annot, t_structured& value_type) {
  PatchGen gen{
      {annot, gen_suffix_struct(annot, value_type, "Patch"), program_}};
  gen.assign(value_type);
  gen.clear();
  if (get_assign_only_annotation_or_null(value_type)) {
    gen.set_adapter("AssignPatchAdapter");
    return gen;
  }
  t_type_ref patch_type = add_field_patch(annot, value_type);
  rust_box(gen.patchPrior(patch_type));
  rust_box(gen.ensureStruct(add_ensure_struct(annot, value_type)));
  rust_box(gen.patchAfter(patch_type));
  if (const auto* p = program_.scope()->find_type("patch.FieldIdList")) {
    // Box it in rust to avoid stack-overflow
    rust_box(gen.remove(*p));
  }
  gen.set_adapter("StructPatchAdapter");
  return gen;
}

t_type_ref patch_generator::find_patch_type(
    const t_const& annot,
    const t_structured& parent,
    t_type_ref type,
    const t_field_id& field_id,
    size_t traversal_order) {
  if (auto custom = find_patch_override(type)) {
    return custom;
  }

  // Look for a patch for the underlying type.
  const auto* ttype = type->get_true_type();

  // Base types use a shared representation defined in patch.thrift.
  if (auto* base_type = dynamic_cast<const t_base_type*>(ttype)) {
    const char* name = getPatchTypeName(base_type->base_type());
    if (const auto* result = program_.scope()->find_type(name)) {
      return t_type_ref::from_ptr(result);
    }

    // TODO(afuller): This look up hack only works for 'built-in' patch types.
    // Use a shared uri type registry instead.
    t_type_ref result = annot.type()->program()->scope()->ref_type(
        const_cast<t_program&>(*annot.type()->program()),
        name,
        parent.src_range());
    if (auto* ph = result.get_unresolved_type()) {
      // Set the location info, in case the type can't be resolved later.
      ph->set_src_range(parent.src_range());
      ph->set_generated();
    }

    return result;
  }

  // Structured types use generated patch types..
  if (auto* structured = dynamic_cast<const t_structured*>(ttype)) {
    if (!structured->uri().empty()) { // Try to look up by URI.
      if (auto* result = dynamic_cast<const t_type*>(
              program_.scope()->find_by_uri(structured->uri() + "Patch"))) {
        return t_type_ref::from_ptr(result);
      }
    }

    // Try to look up by Name.
    // Look for it in the same program as the type itself.
    t_type_ref result = program_.scope()->ref_type(
        const_cast<t_program&>(*structured->program()),
        structured->name() + "Patch",
        parent.src_range());
    if (auto* ph = result.get_unresolved_type()) {
      // Set the location info, in case the type can't be resolved later.
      ph->set_src_range(parent.src_range());
      ph->set_generated();
    }
    return result;
  }

  return gen_patch(annot, parent, field_id, type, traversal_order);
}

t_type_ref patch_generator::find_patch_type(
    const t_const& annot, const t_structured& parent, const t_field& field) {
  if (auto custom = find_patch_override(field)) {
    return custom;
  }

  // Could not resolve a shared patch type, so generate a field specific one.
  return find_patch_type(
      annot, parent, field.type(), getNormalizedFieldId(field));
}

t_type_ref patch_generator::find_patch_override(t_type_ref type) const {
  if (auto* uri = t_typedef::get_first_annotation_or_null(
          type.get_type(), {"thrift.patch.uri"})) {
    return find_patch_override(*type, *uri);
  }
  return {};
}

t_type_ref patch_generator::find_patch_override(const t_named& node) const {
  if (auto* uri = node.find_annotation_or_null("thrift.patch.uri")) {
    return find_patch_override(node, *uri);
  }
  return {};
}

t_type_ref patch_generator::find_patch_override(
    const t_node& node, const std::string& uri) const {
  if (const auto* result =
          dynamic_cast<const t_type*>(program_.scope()->find_by_uri(uri))) {
    return t_type_ref::from_ptr(result);
  }
  ctx_.warning(node, "Could not find custom type: {}", uri);
  return {};
}

t_struct& patch_generator::gen_struct(
    const t_node& annot, std::string name, std::string uri) {
  auto generated = std::make_unique<t_struct>(&program_, std::move(name));
  generated->set_generated();
  generated->set_uri(std::move(uri));
  // Attribute the new struct to the anntation, but give it a unique offset so
  // thrift/compiler/lib/cpp2/util.cc sorts it correctly.
  generated->set_src_range(
      {annot.src_range().begin + (++count_), annot.src_range().end});
  program_.scope()->add_definition(
      program_.scope_name(*generated), generated.get());
  return program_.add_def(std::move(generated));
}

t_struct& patch_generator::gen_suffix_struct(
    const t_node& annot, const t_named& orig, const char* suffix) {
  ctx_.check(!orig.uri().empty(), annot, "URI required to support patching.");
  t_struct& generated =
      gen_struct(annot, orig.name() + suffix, orig.uri() + suffix);
  if (const auto& cpp_name = gen::cpp::namespace_resolver::get_cpp_name(orig);
      cpp_name != orig.name()) {
    generated.set_annotation("cpp.name", cpp_name + suffix);
  }
  return generated;
}

t_struct& patch_generator::gen_patch(
    const t_const& annot,
    const t_structured& orig,
    const t_field_id& field_id,
    t_type_ref type,
    size_t traversal_order) {
  auto suffix = getFieldIdPatchSuffix(field_id, traversal_order);
  PatchGen gen{
      {annot, gen_suffix_struct(annot, orig, suffix.c_str()), program_}};
  // All value patches have an assign and clear field.
  gen.assign(type);
  gen.clear();
  const auto* ttype = type->get_true_type();

  if (annot.type()->uri() == kAssignOnlyPatchUri) {
    if (dynamic_cast<const t_list*>(ttype)) {
      ctx_.error("List patch can not be AssignOnly");
      return gen;
    }
    if (dynamic_cast<const t_set*>(ttype)) {
      ctx_.error("Set patch can not be AssignOnly");
      return gen;
    }
    gen.set_adapter("AssignPatchAdapter");
    return gen;
  }

  if (auto* list = dynamic_cast<const t_list*>(ttype)) {
    // TODO(afuller): support 'replace' op.
    gen.prepend(type);
    gen.append(type);
    gen.set_adapter("ListPatchAdapter");
  } else if (auto* set = dynamic_cast<const t_set*>(ttype)) {
    // TODO(afuller): support 'replace' op.
    gen.remove(type);
    gen.addSet(type);
    gen.set_adapter("SetPatchAdapter");
  } else if (auto* map = dynamic_cast<const t_map*>(ttype)) {
    // TODO(afuller): support 'removeIf' op.
    // TODO(afuller): support 'replace' op.
    if (map->key_type().resolved() && map->val_type().resolved()) {
      auto val_patch_type = find_patch_type(
          annot, orig, map->val_type(), field_id, traversal_order + 1);
      gen.patchPrior(inst_map(map->key_type(), val_patch_type));
      gen.addMap(type);
      gen.patchAfter(inst_map(map->key_type(), val_patch_type));
      gen.remove(inst_set(map->key_type()));
      gen.put(type);
      gen.set_adapter("MapPatchAdapter");
    }
  } else {
    gen.set_adapter("AssignPatchAdapter");
  }
  return gen;
}

t_type_ref patch_generator::inst_list(t_type_ref val) {
  // TODO(afuller): Consider caching.
  return program_.add_type_instantiation(std::make_unique<t_list>(val));
}
t_type_ref patch_generator::inst_set(t_type_ref key) {
  // TODO(afuller): Consider caching.
  return program_.add_type_instantiation(
      unordered(std::make_unique<t_set>(key)));
}
t_type_ref patch_generator::inst_map(t_type_ref key, t_type_ref val) {
  // TODO(afuller): Consider caching.
  return program_.add_type_instantiation(
      unordered(std::make_unique<t_map>(key, val)));
}

} // namespace compiler
} // namespace thrift
} // namespace apache
