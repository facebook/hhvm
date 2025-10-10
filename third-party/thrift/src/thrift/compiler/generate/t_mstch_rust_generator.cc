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

#include <cassert>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/rust/uri.h>
#include <thrift/compiler/generate/rust/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/sema/ast_validator.h>
#include <thrift/compiler/sema/sema_context.h>

namespace apache::thrift::compiler::rust {

struct rust_codegen_options {
  // Crate names used for referring to sibling crates within the same Thrift
  // library.
  std::string types_crate;
  std::string clients_crate;

  // Buck target label that identifies the Rust library built from the sources
  // currently being generated.
  std::string label;

  // Index that can resolve a Thrift t_program to a Rust crate name.
  rust_crate_index crate_index;

  // Whether to emit derive(Serialize, Deserialize).
  // Enabled by `--gen rust:serde`.
  bool serde = false;

  // Whether to emit derive(Valuable).
  // Enabled by `--gen rust:valuable`
  bool valuable = false;

  // Whether fields w/optional values of None should
  // be skipped during serialization. Enabled w/ `--gen
  // rust:skip_none_serialization` Note: `rust:serde` must also be set for this
  // to affect codegen.
  bool skip_none_serialization = false;

  // True if we are generating a submodule rather than the whole crate.
  bool multifile_mode = false;

  // List of extra sources to include at top level of crate.
  mstch::array types_include_srcs;
  mstch::array clients_include_srcs;
  mstch::array services_include_srcs;

  // Markdown file to include in front of crate-level documentation.
  std::string include_docs;

  // The current program being generated and its Rust module path.
  const t_program* current_program;
  std::string current_crate;

  // Whether to generate Thrift metadata for structs or not.
  bool gen_metadata = true;

  // Determines whether or not to generate `fbthrift::conformance::AnyRegistry`
  // initialization functions (c.f. `t_mstch_cpp2_generator::generate_program()`
  // https://fburl.com/code/np814p6y). Enabled by `--gen rust:any`.
  bool any_registry_initialization_enabled = false;

  // split independent types into different compilation units.
  int types_split_count = 0;
};

enum class FieldKind { Box, Arc, Inline };

class mstch_rust_value;

namespace {

const std::string_view kRustCratePrefix = "crate::";
const std::string_view kRustCrateTypesPrefix = "crate::types::";

std::string quoted_rust_doc(const t_named* named_node) {
  const std::string& doc = named_node->doc();

  // strip leading/trailing whitespace
  static const std::string whitespace = "\n\r\t ";
  const auto first = doc.find_first_not_of(whitespace);
  if (first == std::string::npos) {
    // empty string
    return "\"\"";
  }

  const auto last = doc.find_last_not_of(whitespace);
  return quote(doc.substr(first, last - first + 1), true);
}

std::string get_type_annotation(const t_named* node) {
  if (const t_const* annot =
          node->find_structured_annotation_or_null(kRustTypeUri)) {
    return get_annotation_property_string(annot, "name");
  }
  return "";
}

bool has_type_annotation(const t_named* node) {
  return !get_type_annotation(node).empty();
}

bool has_nonstandard_type_annotation(const t_named* node) {
  return get_type_annotation(node).find("::") != std::string::npos;
}

bool has_newtype_annotation(const t_named* node) {
  const t_typedef* type = dynamic_cast<const t_typedef*>(node);
  return type && node->has_structured_annotation(kRustNewTypeUri);
}

bool can_derive_ord(const t_type* type) {
  bool has_custom_type_annotation = has_type_annotation(type);

  type = type->get_true_type();
  if (type->is_string() || type->is_binary() || type->is_bool() ||
      type->is_byte() || type->is_i16() || type->is_i32() || type->is_i64() ||
      type->is<t_enum>() || type->is_void()) {
    return true;
  }
  if (type->has_structured_annotation(kRustOrdUri)) {
    return true;
  }
  if (const t_list* list = type->try_as<t_list>()) {
    auto elem_type = list->get_elem_type();
    return elem_type && can_derive_ord(elem_type);
  }
  // We can implement Ord on BTreeMap (the default map type) if both the key and
  // value implement Eq.
  if (type->is<t_map>() && !has_custom_type_annotation) {
    auto map_type = dynamic_cast<const t_map*>(type);
    auto key_elem_type = &map_type->key_type().deref();
    auto val_elem_type = &map_type->val_type().deref();

    return key_elem_type && val_elem_type && can_derive_ord(key_elem_type) &&
        can_derive_ord(val_elem_type);
  }
  return false;
}

bool rust_serde_enabled(
    const rust_codegen_options& options, const t_named& node) {
  if (const t_const* annot =
          node.find_structured_annotation_or_null(kRustSerdeUri)) {
    return get_annotation_property_bool(annot, "enabled", true);
  }

  return options.serde;
}

std::string get_types_import_name(
    const t_program* program, const rust_codegen_options& options) {
  if (program == options.current_program) {
    return options.current_crate + "::types";
  }

  auto crate = options.crate_index.find(program);
  if (!crate) {
    return program->name() + "__types";
  } else if (crate->dependency_path.empty()) {
    return crate->import_name(program) + "::types";
  } else {
    return crate->import_name(program);
  }
}

std::string get_client_import_name(
    const t_program* program, const rust_codegen_options& options) {
  if (program == options.current_program) {
    return options.current_crate + "::client";
  }

  auto crate = options.crate_index.find(program);
  if (!crate) {
    return program->name() + "__clients";
  } else if (crate->dependency_path.empty()) {
    return crate->import_name(program) + "::client";
  }

  std::string path;
  for (const auto& dep : crate->dependency_path) {
    path += path.empty() ? "::" : "::__dependencies::";
    path += dep + "_clients";
  }

  if (crate->multifile) {
    path += "::" + multifile_module_name(program);
  }

  return path;
}

std::string get_server_import_name(
    const t_program* program, const rust_codegen_options& options) {
  if (program == options.current_program) {
    return options.current_crate;
  }

  auto crate = options.crate_index.find(program);
  if (!crate) {
    return program->name() + "__services";
  } else if (crate->dependency_path.empty()) {
    return crate->import_name(program);
  }

  std::string path;
  for (const auto& dep : crate->dependency_path) {
    path += path.empty() ? "::" : "::__dependencies::";
    path += dep + "_services";
  }

  if (crate->multifile) {
    path += "::" + multifile_module_name(program);
  }

  return path;
}

std::string get_mock_import_name(
    const t_program* program, const rust_codegen_options& options) {
  if (program == options.current_program) {
    return options.current_crate;
  }

  auto crate = options.crate_index.find(program);
  if (!crate) {
    return program->name() + "__mocks";
  } else if (crate->dependency_path.empty()) {
    return crate->import_name(program);
  }

  std::string path;
  for (const auto& dep : crate->dependency_path) {
    path += path.empty() ? "::" : "::__dependencies::";
    path += dep + "_mocks";
  }

  if (crate->multifile) {
    path += "::" + multifile_module_name(program);
  }

  return path;
}

// Path to the crate root of the given service's mocks crate. Unlike
// `get_mock_import_name`, for multifile Thrift libraries the module name is
// not included here.
std::string get_mock_crate(
    const t_program* program, const rust_codegen_options& options) {
  if (program == options.current_program) {
    return "crate";
  }

  auto crate = options.crate_index.find(program);
  if (!crate) {
    return program->name() + "__mocks";
  } else if (crate->dependency_path.empty()) {
    return "crate";
  }

  std::string path;
  for (const auto& dep : crate->dependency_path) {
    path += path.empty() ? "::" : "::__dependencies::";
    path += dep + "_mocks";
  }
  return path;
}

FieldKind field_kind(const t_named& node) {
  if (node.has_structured_annotation(kRustBoxUri)) {
    // @rust.Box
    return FieldKind::Box;
  }
  if (node.has_structured_annotation(kRustArcUri)) {
    // @rust.Arc takes priority over @thrift.Box, field will be an Arc in Rust
    return FieldKind::Arc;
  }
  if (node.has_unstructured_annotation("thrift.box") ||
      node.has_structured_annotation(kBoxUri)) {
    // @thrift.Box
    return FieldKind::Box;
  }
  return FieldKind::Inline;
}

int checked_stoi(const std::string& s, const std::string& msg) {
  try {
    std::size_t pos = 0;
    int ret = std::stoi(s, &pos);
    if (pos != s.size()) {
      throw std::runtime_error(msg);
    }
    return ret;
  } catch (...) {
    throw std::runtime_error(msg);
  }
}

void parse_include_srcs(
    mstch::array& elements, std::optional<std::string> const& include_srcs) {
  if (!include_srcs) {
    return;
  }
  const auto& paths = *include_srcs;
  std::string::size_type pos = 0;
  while (pos != std::string::npos && pos < paths.size()) {
    std::string::size_type next_pos = paths.find(':', pos);
    elements.emplace_back(paths.substr(pos, next_pos - pos));
    pos = ((next_pos == std::string::npos) ? next_pos : next_pos + 1);
  }
}

const t_const* find_structured_adapter_annotation(const t_named& node) {
  return node.find_structured_annotation_or_null(kRustAdapterUri);
}

const t_const* find_structured_derive_annotation(const t_named& node) {
  return node.find_structured_annotation_or_null(kRustDeriveUri);
}

const t_const* find_structured_service_exn_annotation(const t_named& node) {
  return node.find_structured_annotation_or_null(kRustServiceExnUri);
}

bool node_has_adapter(const t_named& node) {
  return find_structured_adapter_annotation(node) != nullptr;
}

bool type_has_transitive_adapter(
    const t_type* type, bool step_through_newtypes) {
  if (const t_typedef* typedef_type = type->try_as<t_typedef>()) {
    // Currently the only "type" that can have an adapter is a typedef.
    if (node_has_adapter(*typedef_type)) {
      return true;
    }

    if (!step_through_newtypes && has_newtype_annotation(typedef_type)) {
      return false;
    }

    return type_has_transitive_adapter(
        typedef_type->get_type(), step_through_newtypes);

  } else if (const t_list* list_type = type->try_as<t_list>()) {
    return type_has_transitive_adapter(
        list_type->get_elem_type(), step_through_newtypes);

  } else if (const t_set* set_type = type->try_as<t_set>()) {
    return type_has_transitive_adapter(
        set_type->get_elem_type(), step_through_newtypes);

  } else if (const t_map* map_type = type->try_as<t_map>()) {
    return type_has_transitive_adapter(
               &map_type->key_type().deref(), step_through_newtypes) ||
        type_has_transitive_adapter(
               &map_type->val_type().deref(), step_through_newtypes);

  } else if (type->is<t_struct>() || type->is<t_union>()) {
    auto struct_type = dynamic_cast<const t_structured*>(type);
    if (struct_type) {
      return node_has_adapter(*struct_type);
    }
  }

  return false;
}

const t_type* step_through_typedefs(const t_type* t, bool break_on_adapter) {
  while (t->is<t_typedef>()) {
    auto typedef_type = dynamic_cast<const t_typedef*>(t);
    if (!typedef_type) {
      return t;
    }

    if (has_newtype_annotation(typedef_type)) {
      return t;
    }

    if (break_on_adapter && node_has_adapter(*typedef_type)) {
      return t;
    }

    t = typedef_type->get_type();
  }

  return t;
}

bool typedef_has_constructor_expression(const t_typedef* t) {
  do {
    if (has_newtype_annotation(t)) {
      // Outermost typedef is a newtype or refers to a newtype.
      return true;
    }
    if (node_has_adapter(*t)) {
      // Outermost typedef refers to an associated type projection through the
      // ThriftAdapter trait, which is only in the type namespace.
      break;
    }
    if (t->find_structured_annotation_or_null(kRustTypeUri)) {
      // Outermost typedef refers to a non-Thrift-generated type.
      break;
    }
    const t_type* definition = t->get_type();
    if (definition->is<t_enum>()) {
      // Outermost typedef refers to an enum.
      return true;
    }
    t = dynamic_cast<const t_typedef*>(definition);
  } while (t);
  return false;
}

bool node_has_custom_rust_type(const t_named& node) {
  return node.has_structured_annotation(kRustTypeUri) ||
      node.has_structured_annotation(kRustNewTypeUri);
}

// NOTE: a transitive _adapter_ is different from a transitive _annotation_. A
// transitive adapter is defined as one applied transitively through types.
// E.g.
//
// ```
// @rust.Adapter{name = "Foo"}
// typedef string AdaptedString
//
// struct Bar {
//   1: AdaptedString field1;
// }
// ```
//
// `Bar.field1` has a transitive adapter due to its type being an adapted
// type.
//
// A transitive annotation is one that is applied through `@scope.Transitive`.
// E.g.
//
// ```
// @rust.Adapter{name = "Foo"}
// @scope.Transitive
// struct SomeAnnotation {}
//
// struct Bar {
//   @SomeAnnotation
//   1: string field1;
// }
// ```
//
// `Bar.field1` has an non-transitive adapter applied through a transitive
// annotation in this example.
mstch::node adapter_node(
    // The field or typedef's adapter (if there is one).
    const t_const* adapter_annotation,
    const t_type* type,
    bool ignore_transitive_check,
    mstch_context& context,
    mstch_element_position pos,
    const rust_codegen_options& options) {
  // Step through typedefs.
  const t_type* curr_type = step_through_typedefs(type, true);
  const t_type* transitive_type = nullptr;

  if (!ignore_transitive_check &&
      type_has_transitive_adapter(curr_type, false)) {
    transitive_type = curr_type;
  }

  if (!adapter_annotation && !transitive_type) {
    return false;
  }

  mstch::node name;
  bool is_generic = false;
  if (adapter_annotation != nullptr) {
    // Always replace `crate::` with the package name of where this annotation
    // originated to support adapters applied with `@scope.Transitive`.
    // If the annotation originates from the same module, this will just
    // return `crate::` anyways to be a no-op.
    std::string package =
        get_types_import_name(adapter_annotation->program(), options);

    auto adapter_name =
        get_annotation_property_string(adapter_annotation, "name");

    is_generic = boost::algorithm::ends_with(adapter_name, "<>");
    if (is_generic) {
      adapter_name = adapter_name.substr(0, adapter_name.length() - 2);
    }

    if (!package.empty() &&
        boost::algorithm::starts_with(adapter_name, kRustCrateTypesPrefix)) {
      adapter_name =
          package + "::" + adapter_name.substr(kRustCrateTypesPrefix.length());
    } else if (
        !package.empty() &&
        boost::algorithm::starts_with(adapter_name, kRustCratePrefix)) {
      adapter_name =
          package + "::" + adapter_name.substr(kRustCratePrefix.length());
    } else if (!(boost::algorithm::starts_with(adapter_name, "::") ||
                 boost::algorithm::starts_with(
                     adapter_name, kRustCratePrefix))) {
      adapter_name = "::" + adapter_name;
    }

    name = adapter_name;
  }

  bool layered = adapter_annotation != nullptr && transitive_type != nullptr;
  bool transitive_only =
      adapter_annotation == nullptr && transitive_type != nullptr;

  auto underlying_type =
      context.type_factory->make_mstch_object(type, context, pos);

  mstch::node transitive_type_node;
  if (transitive_type) {
    transitive_type_node =
        context.type_factory->make_mstch_object(transitive_type, context, pos);
  }

  mstch::map node{
      // Only populated if this adapter node was generated from a typedef
      // or a field with an adapter.
      {"adapter:name?", name},
      {"adapter:is_generic?", is_generic},
      {"adapter:underlying_type", underlying_type},
      {"adapter:layered?", layered},
      {"adapter:transitive_only?", transitive_only},
      {"adapter:transitive_type?", transitive_type_node},
  };

  return node;
}

mstch::node structured_annotations_node(
    const t_named& named,
    unsigned depth,
    mstch_context& context,
    mstch_element_position pos,
    const rust_codegen_options& options) {
  std::vector<mstch::node> annotations;

  // Note, duplicate annotations are not allowed per the Thrift spec.
  for (const t_const& annotation : named.structured_annotations()) {
    auto direct_annotation = mstch::make_shared_node<mstch_rust_value>(
        annotation.value(), annotation.type(), depth, context, pos, options);

    mstch::node transitive;
    const t_type* annotation_type = annotation.type();
    if (annotation_type->has_structured_annotation(kTransitiveUri)) {
      transitive = context.type_factory->make_mstch_object(
          annotation_type, context, pos);
    }

    annotations.emplace_back(mstch::map{
        {"structured_annotation:direct", direct_annotation},
        {"structured_annotation:transitive?", transitive},
    });
  }

  return annotations;
}

std::string get_resolved_name(const t_type* t) {
  t = t->get_true_type();
  if (auto c = dynamic_cast<const t_list*>(t)) {
    return fmt::format("list<{}>", get_resolved_name(c->get_elem_type()));
  }
  if (auto c = dynamic_cast<const t_set*>(t)) {
    return fmt::format("set<{}>", get_resolved_name(c->get_elem_type()));
  }
  if (auto c = dynamic_cast<const t_map*>(t)) {
    return fmt::format(
        "map<{},{}>",
        get_resolved_name(&c->key_type().deref()),
        get_resolved_name(&c->val_type().deref()));
  }
  return t->get_full_name();
}

std::string get_resolved_name(const t_field* field) {
  return get_resolved_name(&field->type().deref());
}

} // namespace

class t_mstch_rust_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "typedef:newtype?",
        "function:name",
    };
    return opts;
  }

  std::string template_prefix() const override { return "rust"; }

  void generate_program() override;
  void generate_split_types();
  void fill_validator_visitors(ast_validator&) const override;

 private:
  void set_mstch_factories();
  rust_codegen_options options_;
  whisker::map::raw globals() const override {
    whisker::map::raw globals = t_mstch_generator::globals();
    globals["rust_annotation_name"] = whisker::dsl::make_function(
        "rust_annotation_name",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(boost::algorithm::replace_all_copy(
              ctx.argument<whisker::string>(0), ".", "_"));
        });
    globals["rust_quote"] = whisker::dsl::make_function(
        "rust_quote",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({"escape_backslashes"});
          ctx.declare_arity(1);
          std::optional<bool> escape_backslashes = ctx.named_argument<bool>(
              "escape_backslashes",
              whisker::dsl::function::context::named_argument_presence::
                  optional);
          return whisker::make::string(quote(
              ctx.argument<whisker::string>(0),
              escape_backslashes.value_or(false)));
        });
    return globals;
  }

  prototype<t_enum>::ptr make_prototype_for_enum(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_enum(proto);
    auto def = whisker::dsl::prototype_builder<h_enum>::extends(base);
    def.property("variants_by_name", [&proto](const t_enum& self) {
      std::vector<const t_enum_value*> variants = self.values().copy();
      std::sort(variants.begin(), variants.end(), [](auto a, auto b) {
        return a->name() < b->name();
      });
      return to_array(std::move(variants), proto.of<t_enum_value>());
    });
    def.property("variants_by_number", [&proto](const t_enum& self) {
      std::vector<const t_enum_value*> variants = self.values().copy();
      std::sort(variants.begin(), variants.end(), [](auto a, auto b) {
        return a->get_value() < b->get_value();
      });
      return to_array(std::move(variants), proto.of<t_enum_value>());
    });
    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);
    def.property("rust_name", [](const t_named& self) {
      return named_rust_name(&self);
    });
    def.property(
        "docs", [](const t_named& self) { return quoted_rust_doc(&self); });
    return std::move(def).make();
  }
};

class rust_mstch_program : public mstch_program {
 public:
  rust_mstch_program(
      const t_program* program,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options,
      const int split_id = 0)
      : mstch_program(program, ctx, pos),
        options_(*options),
        split_id_(split_id) {
    register_methods(
        this,
        {
            {"program:direct_dependencies?",
             &rust_mstch_program::rust_has_direct_dependencies},
            {"program:direct_dependencies",
             &rust_mstch_program::rust_direct_dependencies},
            {"program:types", &rust_mstch_program::rust_types},
            {"program:clients", &rust_mstch_program::rust_clients},
            {"program:nonexhaustiveStructs?",
             &rust_mstch_program::rust_nonexhaustive_structs},
            {"program:serde?", &rust_mstch_program::rust_serde},
            {"program:valuable?", &rust_mstch_program::rust_valuable},
            {"program:skip_none_serialization?",
             &rust_mstch_program::rust_skip_none_serialization},
            {"program:multifile?", &rust_mstch_program::rust_multifile},
            {"program:crate", &rust_mstch_program::rust_crate},
            {"program:client_package",
             &rust_mstch_program::rust_client_package},
            {"program:includes", &rust_mstch_program::rust_includes},
            {"program:label", &rust_mstch_program::rust_label},
            {"program:nonstandardTypes",
             &rust_mstch_program::rust_nonstandard_types},
            {"program:nonstandardFields",
             &rust_mstch_program::rust_nonstandard_fields},
            {"program:types_include_srcs",
             &rust_mstch_program::rust_types_include_srcs},
            {"program:clients_include_srcs",
             &rust_mstch_program::rust_clients_include_srcs},
            {"program:services_include_srcs",
             &rust_mstch_program::rust_services_include_srcs},
            {"program:include_docs", &rust_mstch_program::rust_include_docs},
            {"program:has_default_tests?",
             &rust_mstch_program::rust_has_default_tests},
            {"program:structs_for_default_test",
             &rust_mstch_program::rust_structs_for_default_test},
            {"program:has_adapted_structs?",
             &rust_mstch_program::rust_has_adapted_structs},
            {"program:adapted_structs",
             &rust_mstch_program::rust_adapted_structs},
            {"program:has_adapters?", &rust_mstch_program::rust_has_adapters},
            {"program:adapters", &rust_mstch_program::rust_adapters},
            {"program:has_const_tests?",
             &rust_mstch_program::rust_has_const_tests},
            {"program:consts_for_test",
             &rust_mstch_program::rust_consts_for_test},
            {"program:rust_gen_native_metadata?",
             &rust_mstch_program::rust_gen_native_metadata},
            {"program:types_with_constructors",
             &rust_mstch_program::rust_types_with_constructors},
            {"program:current_split_structs",
             &rust_mstch_program::current_split_structs},
            {"program:current_split_typedefs",
             &rust_mstch_program::current_split_typedefs},
            {"program:current_split_enums",
             &rust_mstch_program::current_split_enums},
            {"program:split_mode_enabled?",
             &rust_mstch_program::split_mode_enabled},
            {"program:type_splits", &rust_mstch_program::type_splits},
        });

    // Generate type split data if split count option is provided.
    if (options_.types_split_count) {
      initialize_type_split();
      generate_split_data();
    }
  }

  mstch::node rust_has_direct_dependencies() {
    return !options_.crate_index.direct_dependencies().empty();
  }

  mstch::node rust_direct_dependencies() {
    mstch::array direct_dependencies;
    for (auto crate : options_.crate_index.direct_dependencies()) {
      mstch::map dependency;
      dependency["dependency:name"] =
          mangle_crate_name(crate->dependency_path[0]);
      dependency["dependency:name_unmangled"] = crate->dependency_path[0];
      dependency["dependency:label"] = crate->label;
      direct_dependencies.emplace_back(std::move(dependency));
    }
    return direct_dependencies;
  }

  mstch::node rust_types() {
    auto types = "::" + options_.types_crate;
    if (options_.multifile_mode) {
      types += "::" + multifile_module_name(program_);
    }
    return types;
  }

  mstch::node rust_clients() {
    auto clients = "::" + options_.clients_crate;
    if (options_.multifile_mode) {
      clients += "::" + multifile_module_name(program_);
    }
    return clients;
  }

  mstch::node rust_nonexhaustive_structs() {
    for (t_structured* strct : program_->structs_and_unions()) {
      // The is_union is because `union` are also in this collection.
      if (!strct->is<t_union>() &&
          !strct->has_structured_annotation(kRustExhaustiveUri)) {
        return true;
      }
    }
    for (t_exception* strct : program_->exceptions()) {
      if (!strct->has_structured_annotation(kRustExhaustiveUri)) {
        return true;
      }
    }
    return false;
  }
  mstch::node rust_serde() { return options_.serde; }
  mstch::node rust_skip_none_serialization() {
    return options_.skip_none_serialization;
  }
  mstch::node rust_valuable() { return options_.valuable; }
  mstch::node rust_multifile() { return options_.multifile_mode; }
  mstch::node rust_crate() {
    if (options_.multifile_mode) {
      return "crate::" + multifile_module_name(program_);
    }
    return std::string("crate");
  }
  mstch::node rust_client_package() {
    return get_client_import_name(program_, options_);
  }
  mstch::node rust_includes() {
    mstch::array includes;
    for (auto* program : program_->get_includes_for_codegen()) {
      includes.emplace_back(
          context_.program_factory->make_mstch_object(program, context_, pos_));
    }
    return includes;
  }
  mstch::node rust_label() {
    if (program_ == options_.current_program) {
      return options_.label;
    }
    auto crate = options_.crate_index.find(program_);
    if (crate) {
      return crate->label;
    }
    return false;
  }
  template <typename F>
  void foreach_field(F&& f) const {
    for (const t_structured* strct : program_->structs_and_unions()) {
      for (const auto& field : strct->fields()) {
        f(&field);
      }
    }
  }
  template <typename F>
  void foreach_type(F&& f) const {
    for (const t_structured* strct : program_->structs_and_unions()) {
      for (const auto& field : strct->fields()) {
        f(field.get_type());
      }
    }
    for (const auto* service : program_->services()) {
      for (const auto& function : service->functions()) {
        for (const auto& param : function.params().fields()) {
          f(param.get_type());
        }
        f(function.return_type().get_type());
      }
    }
    for (auto typedf : program_->typedefs()) {
      f(typedf);
    }
  }
  mstch::node rust_nonstandard_types() {
    types_set_t types = nonstandard_types();
    return make_mstch_types(
        std::vector<const t_type*>(types.begin(), types.end()));
  }
  mstch::node rust_nonstandard_fields() {
    fields_set_t fields = nonstandard_fields();
    return make_mstch_fields(
        std::vector<const t_field*>(fields.begin(), fields.end()));
  }
  mstch::node rust_types_include_srcs() { return options_.types_include_srcs; }
  mstch::node rust_clients_include_srcs() {
    return options_.clients_include_srcs;
  }
  mstch::node rust_services_include_srcs() {
    return options_.services_include_srcs;
  }
  mstch::node rust_include_docs() { return options_.include_docs; }
  mstch::node rust_has_default_tests() {
    for (const t_structured* strct : program_->structs_and_unions()) {
      for (const t_field& field : strct->fields()) {
        if (node_has_adapter(field) ||
            type_has_transitive_adapter(field.get_type(), true)) {
          return true;
        }
      }
    }

    return false;
  }
  mstch::node rust_structs_for_default_test() {
    mstch::array strcts;

    for (const t_structured* strct : program_->structs_and_unions()) {
      for (const t_field& field : strct->fields()) {
        if (node_has_adapter(field) ||
            type_has_transitive_adapter(field.get_type(), true)) {
          strcts.emplace_back(context_.struct_factory->make_mstch_object(
              strct, context_, pos_));
          break;
        }
      }
    }

    return strcts;
  }

  mstch::node rust_has_adapted_structs() {
    for (const t_structured* strct : program_->structs_and_unions()) {
      if (node_has_adapter(*strct)) {
        return true;
      }
    }

    return false;
  }

  mstch::node rust_adapted_structs() {
    mstch::array strcts;

    for (const t_structured* strct : program_->structs_and_unions()) {
      if (node_has_adapter(*strct)) {
        strcts.emplace_back(
            context_.struct_factory->make_mstch_object(strct, context_, pos_));
      }
    }

    return strcts;
  }

  mstch::node rust_has_adapters() {
    for (const t_structured* strct : program_->structs_and_unions()) {
      if (node_has_adapter(*strct)) {
        return true;
      }
    }

    for (const t_typedef* t : program_->typedefs()) {
      if (node_has_adapter(*t)) {
        return true;
      }
    }

    return false;
  }

  mstch::node rust_adapters() {
    mstch::array types_with_direct_adapters;

    for (const t_structured* strct : program_->structs_and_unions()) {
      if (node_has_adapter(*strct)) {
        types_with_direct_adapters.emplace_back(
            context_.type_factory->make_mstch_object(strct, context_, pos_));
      }
    }

    for (const t_typedef* t : program_->typedefs()) {
      if (node_has_adapter(*t)) {
        types_with_direct_adapters.emplace_back(
            context_.type_factory->make_mstch_object(t, context_, pos_));
      }
    }

    return types_with_direct_adapters;
  }

  mstch::node rust_has_const_tests() {
    for (const t_const* c : program_->consts()) {
      if (type_has_transitive_adapter(c->type(), true)) {
        return true;
      }
    }

    return false;
  }
  mstch::node rust_consts_for_test() {
    mstch::array consts;

    for (const t_const* c : program_->consts()) {
      if (type_has_transitive_adapter(c->type(), true)) {
        consts.emplace_back(c->name());
      }
    }

    return consts;
  }
  mstch::node rust_gen_native_metadata() { return options_.gen_metadata; }
  mstch::node rust_types_with_constructors() {
    // Names that this Thrift crate defines in both the type namespace and value
    // namespace. This is the case for enums, where `E` is a type and `E(0)` is
    // an expression, and for newtype typedefs, where `T` is a type and
    // `T(inner)` is an expression, and also for non-newtype typedefs referring
    // to these.
    std::set<std::string> types;
    std::vector<t_enum*> enums;
    std::vector<t_typedef*> typedefs;
    if (options_.types_split_count > 0) {
      enums = enum_split_assignments_[split_id_];
      typedefs = typedef_split_assignments_[split_id_];
    } else {
      enums = program_->enums();
      typedefs = program_->typedefs();
    }

    for (const t_enum* t : enums) {
      types.insert(type_rust_name(t));
    }
    for (const t_typedef* t : typedefs) {
      if (typedef_has_constructor_expression(t)) {
        types.insert(type_rust_name(t));
      }
    }
    return mstch::array(types.begin(), types.end());
  }

  mstch::node current_split_structs() {
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
    return make_mstch_array_cached(
        struct_split_assignments_[split_id_],
        *context_.struct_factory,
        context_.struct_cache,
        id);
  }

  mstch::node current_split_typedefs() {
    return make_mstch_typedefs(typedef_split_assignments_[split_id_]);
  }

  mstch::node current_split_enums() {
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
    return make_mstch_array_cached(
        enum_split_assignments_[split_id_],
        *context_.enum_factory,
        context_.enum_cache,
        id);
  }

  mstch::node type_splits() {
    mstch::array split_indices(options_.types_split_count + 1);
    for (int i = 0; i < options_.types_split_count + 1; i++) {
      split_indices[i] = i;
    }
    return split_indices;
  }

  mstch::node split_mode_enabled() {
    return static_cast<bool>(options_.types_split_count);
  }

 private:
  const rust_codegen_options& options_;
  const int split_id_;
  std::map<int, std::vector<t_structured*>> struct_split_assignments_;
  std::map<int, std::vector<t_typedef*>> typedef_split_assignments_;
  std::map<int, std::vector<t_enum*>> enum_split_assignments_;
  std::set<const t_named*> all_types;
  std::set<const t_named*> dependent_types;

  void initialize_type_split() {
    // Collect all types in this program
    for (const t_enum* enm : program_->enums()) {
      all_types.insert(enm);
    }
    for (const t_typedef* typedf : program_->typedefs()) {
      all_types.insert(typedf);
    }
    for (const t_structured* strct : program_->structured_definitions()) {
      all_types.insert(strct);
    }

    // Identify dependencies within types
    for (t_typedef* typedf : program_->typedefs()) {
      if (generate_reference_set(typedf->get_type()) ||
          (typedef_has_constructor_expression(typedf))) {
        dependent_types.insert(typedf);
      }
    }
    for (t_structured* strct : program_->structured_definitions()) {
      for (const t_field& field : strct->fields()) {
        if (generate_reference_set(field.get_type())) {
          dependent_types.insert(strct);
        }
      }
    }
  }

  void generate_split_data() {
    auto next = [counter = 0, this]() mutable {
      return ((counter++) % options_.types_split_count) + 1;
    };
    for (t_typedef* typedf : program_->typedefs()) {
      if (dependent_types.count(typedf)) {
        // place all dependent types in the first chunk
        typedef_split_assignments_[0].emplace_back(typedf);
      } else {
        // place independent type in a shard that isn't zero
        typedef_split_assignments_[next()].emplace_back(typedf);
      }
    }

    for (t_structured* strct : program_->structured_definitions()) {
      if (dependent_types.count(strct)) {
        // place all dependent types in the first chunk
        struct_split_assignments_[0].emplace_back(strct);
      } else {
        // place independent type in a shard that isn't zero
        struct_split_assignments_[next()].emplace_back(strct);
      }
    }

    for (t_enum* enm : program_->enums()) {
      if (dependent_types.count(enm)) {
        // place all dependent types in the first chunk
        enum_split_assignments_[0].emplace_back(enm);
      } else {
        // place independent type in a shard that isn't zero
        enum_split_assignments_[next()].emplace_back(enm);
      }
    }
  }

  // This function checks to see if a type is defined within the current crate
  // by checking the `all_types` set for membership. If the type is within the
  // `all_types` set, it adds the type to the `dependent_types` set and returns
  // true. This function's return value is used to determine if the parent type
  // needs to be added to the `dependent_types` set
  bool generate_reference_set(const t_type* type) {
    if (!type) {
      return false;
    }

    bool dependent = false;

    // Check if the type is defined within the current crate. If it is, add the
    // type to the dependent type set.
    if (all_types.count(type)) {
      dependent_types.insert(type);
      dependent = true;
    }
    // Recursively check container types
    else if (const t_list* list_type = dynamic_cast<const t_list*>(type)) {
      dependent = generate_reference_set(list_type->get_elem_type());
    } else if (const t_set* set_type = dynamic_cast<const t_set*>(type)) {
      dependent = generate_reference_set(set_type->get_elem_type());
    } else if (const t_map* map_type = dynamic_cast<const t_map*>(type)) {
      bool dependent_map_key =
          generate_reference_set(&map_type->key_type().deref());
      bool dependent_map_val =
          generate_reference_set(&map_type->val_type().deref());
      dependent = dependent_map_key || dependent_map_val;
    }
    return dependent;
  }

 private:
  template <class T>
  struct rust_type_less {
    bool operator()(const T* lhs, const T* rhs) const {
      std::string lhs_annotation = get_type_annotation(lhs);
      std::string rhs_annotation = get_type_annotation(rhs);
      if (lhs_annotation != rhs_annotation) {
        return lhs_annotation < rhs_annotation;
      }
      return get_resolved_name(lhs) < get_resolved_name(rhs);
    }
  };
  using strings_set_t = std::set<std::string>;
  using types_set_t = std::set<const t_type*, rust_type_less<t_type>>;
  using fields_set_t = std::set<const t_field*, rust_type_less<t_field>>;

 private:
  types_set_t nonstandard_types() {
    types_set_t types;
    foreach_type([&](const t_type* type) {
      if (has_nonstandard_type_annotation(type)) {
        types.insert(type);
      }
    });
    return types;
  }
  // Collect fields with nonstandard types not contained by
  // `nonstandard_types()` (avoid generating multiple definitions for the same
  // type).
  fields_set_t nonstandard_fields() {
    fields_set_t fields;
    strings_set_t names;
    types_set_t types = nonstandard_types();
    std::transform(
        types.begin(),
        types.end(),
        std::inserter(names, names.end()),
        [](const t_type* t) { return get_resolved_name(t); });
    foreach_field([&](const t_field* field) {
      if (has_nonstandard_type_annotation(field)) {
        if (names.find(get_resolved_name(&field->type().deref())) ==
            names.end()) {
          fields.insert(field);
        }
      }
    });
    return fields;
  }
};

class rust_mstch_service : public mstch_service {
 public:
  rust_mstch_service(
      const t_service* service,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options,
      const t_service* containing_service = nullptr)
      : mstch_service(service, ctx, pos, containing_service),
        options_(*options) {
    for (auto function : service->get_functions()) {
      function_upcamel_names_.insert(camelcase(function->name()));
    }
    register_methods(
        this,
        {{"service:rustFunctions", &rust_mstch_service::rust_functions},
         {"service:rust_exceptions", &rust_mstch_service::rust_all_exceptions},
         {"service:client_package", &rust_mstch_service::rust_client_package},
         {"service:server_package", &rust_mstch_service::rust_server_package},
         {"service:mock_package", &rust_mstch_service::rust_mock_package},
         {"service:mock_crate", &rust_mstch_service::rust_mock_crate},
         {"service:snake", &rust_mstch_service::rust_snake},
         {"service:requestContext?", &rust_mstch_service::rust_request_context},
         {"service:extendedClients",
          &rust_mstch_service::rust_extended_clients},
         {"service:program_name", &rust_mstch_service::program_name}});
  }
  mstch::node rust_functions();
  mstch::node rust_client_package() {
    return get_client_import_name(service_->program(), options_);
  }
  mstch::node rust_server_package() {
    return get_server_import_name(service_->program(), options_);
  }
  mstch::node rust_mock_package() {
    return get_mock_import_name(service_->program(), options_);
  }
  mstch::node rust_mock_crate() {
    return get_mock_crate(service_->program(), options_);
  }
  mstch::node rust_snake() {
    if (const t_const* annot_mod =
            service_->find_structured_annotation_or_null(kRustModUri)) {
      return get_annotation_property_string(annot_mod, "name");
    } else if (
        const t_const* annot_name =
            service_->find_structured_annotation_or_null(kRustNameUri)) {
      return snakecase(get_annotation_property_string(annot_name, "name"));
    } else {
      return mangle_type(snakecase(service_->name()));
    }
  }
  mstch::node rust_request_context() {
    return service_->has_structured_annotation(kRustRequestContextUri);
  }
  mstch::node rust_extended_clients() {
    mstch::array extended_services;
    const t_service* service = service_;
    std::string as_ref_impl = "&self.parent";
    while (const t_service* parent_service = service->extends()) {
      mstch::map node;
      node["extendedService:packagePrefix"] =
          get_client_import_name(parent_service->program(), options_);
      node["extendedService:asRefImpl"] = as_ref_impl;
      node["extendedService:service"] =
          make_mstch_extended_service_cached(parent_service);
      extended_services.emplace_back(node);
      as_ref_impl = "self.parent.as_ref()";
      service = parent_service;
    }
    return extended_services;
  }
  mstch::node program_name() { return service_->program()->name(); }

  mstch::node rust_all_exceptions();

 private:
  std::unordered_multiset<std::string> function_upcamel_names_;
  const rust_codegen_options& options_;
};

class rust_mstch_interaction : public rust_mstch_service {
 public:
  using ast_type = t_interaction;

  rust_mstch_interaction(
      const t_interaction* interaction,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service,
      const rust_codegen_options* options)
      : rust_mstch_service(interaction, ctx, pos, options, containing_service) {
  }
};

class rust_mstch_function : public mstch_function {
 public:
  rust_mstch_function(
      const t_function* function,
      mstch_context& ctx,
      mstch_element_position pos,
      const std::unordered_multiset<std::string>& function_upcamel_names)
      : mstch_function(function, ctx, pos),
        function_upcamel_names_(function_upcamel_names) {
    register_methods(
        this,
        {{"function:upcamel", &rust_mstch_function::rust_upcamel},
         {"function:index", &rust_mstch_function::rust_index},
         {"function:uniqueExceptions",
          &rust_mstch_function::rust_unique_exceptions},
         {"function:uniqueStreamExceptions",
          &rust_mstch_function::rust_unique_stream_exceptions},
         {"function:uniqueSinkExceptions",
          &rust_mstch_function::rust_unique_sink_exceptions},
         {"function:uniqueSinkFinalExceptions",
          &rust_mstch_function::rust_unique_sink_final_exceptions},
         {"function:args_by_name", &rust_mstch_function::rust_args_by_name},
         {"function:returns_by_name",
          &rust_mstch_function::rust_returns_by_name},
         {"function:interaction_name",
          &rust_mstch_function::rust_interaction_name},
         {"function:void_or_void_stream?",
          &rust_mstch_function::rust_void_or_void_stream},
         {"function:enable_anyhow_to_application_exn",
          &rust_mstch_function::rust_anyhow_to_application_exn}});
  }
  mstch::node rust_upcamel() {
    auto upcamel_name = camelcase(function_->name());
    if (function_upcamel_names_.count(upcamel_name) > 1) {
      // If a service contains a pair of methods that collide converted to
      // CamelCase, like a service containing both create_shard and
      // createShard, then we name the exception types without any case
      // conversion; instead of a CreateShardExn they'll get create_shardExn
      // and createShardExn.
      return function_->name();
    }
    return upcamel_name;
  }
  mstch::node rust_index() { return pos_.index; }
  mstch::node rust_unique_exceptions() {
    return rust_make_unique_exceptions(function_->exceptions());
  }
  mstch::node rust_unique_stream_exceptions() {
    const t_stream* stream = function_->stream();
    return rust_make_unique_exceptions(stream ? stream->exceptions() : nullptr);
  }
  mstch::node rust_unique_sink_exceptions() {
    const t_sink* sink = function_->sink();
    return rust_make_unique_exceptions(
        sink ? sink->sink_exceptions() : nullptr);
  }
  mstch::node rust_unique_sink_final_exceptions() {
    const t_sink* sink = function_->sink();
    return rust_make_unique_exceptions(
        sink ? sink->final_response_exceptions() : nullptr);
  }
  mstch::node rust_make_unique_exceptions(const t_structured* s) {
    // When generating From<> impls for an error type, we must not generate
    // one where more than one variant contains the same type of exception.
    // Find only those exceptions that map uniquely to a variant.

    std::vector<const t_field*> unique_exceptions;
    if (s) {
      const auto& exceptions = s->fields();
      std::map<const t_type*, unsigned> type_count;
      for (const auto& x : exceptions) {
        type_count[x.get_type()] += 1;
      }

      for (const auto& x : exceptions) {
        if (type_count.at(x.get_type()) == 1) {
          unique_exceptions.emplace_back(&x);
        }
      }
    }

    return make_mstch_fields(unique_exceptions);
  }
  mstch::node rust_args_by_name() {
    auto params = function_->params().fields().copy();
    std::sort(params.begin(), params.end(), [](auto a, auto b) {
      return a->name() < b->name();
    });
    return make_mstch_fields(params);
  }

  mstch::node rust_returns_by_name() {
    auto returns = std::vector<std::string>();
    auto add_return =
        [&](std::string_view name, std::string_view type, int id) {
          returns.push_back(fmt::format(
              "::fbthrift::Field::new(\"{}\", ::fbthrift::TType::{}, {})",
              name,
              type,
              id));
        };
    auto get_ttype = [](const t_type& type) {
      const t_type* true_type = type.get_true_type();
      if (const auto* primitive = true_type->try_as<t_primitive_type>()) {
        switch (primitive->primitive_type()) {
          case t_primitive_type::type::t_void:
            return "Void";
          case t_primitive_type::type::t_bool:
            return "Bool";
          case t_primitive_type::type::t_byte:
            return "Byte";
          case t_primitive_type::type::t_i16:
            return "I16";
          case t_primitive_type::type::t_i32:
            return "I32";
          case t_primitive_type::type::t_i64:
            return "I64";
          case t_primitive_type::type::t_float:
            return "Float";
          case t_primitive_type::type::t_double:
            return "Double";
          case t_primitive_type::type::t_string:
            return "String";
          case t_primitive_type::type::t_binary:
            return "String";
        }
      } else if (true_type->is<t_list>()) {
        return "List";
      } else if (true_type->is<t_set>()) {
        return "Set";
      } else if (true_type->is<t_map>()) {
        return "Map";
      } else if (true_type->is<t_enum>()) {
        return "I32";
      } else if (true_type->is<t_structured>()) {
        return "Struct";
      }
      return "";
    };
    for (const t_field& field : get_elems(function_->exceptions())) {
      add_return(field.name(), get_ttype(*field.type()), field.id());
    }
    auto type_name = std::string();
    if (function_->stream()) {
      type_name = "Stream";
    } else if (function_->sink()) {
      type_name = "Void";
    } else {
      type_name = get_ttype(*function_->return_type());
    }
    add_return("Success", type_name, 0);
    std::sort(returns.begin(), returns.end());
    auto array = mstch::array();
    for (const std::string& ret : returns) {
      array.emplace_back(ret);
    }
    return array;
  }

  mstch::node rust_interaction_name() {
    const auto& interaction = function_->interaction();
    return interaction ? interaction->name() : function_->return_type()->name();
  }
  mstch::node rust_void_or_void_stream() {
    return function_->has_void_initial_response();
  }
  mstch::node rust_anyhow_to_application_exn() {
    // First look for annotation on the function.
    if (const t_const* annot =
            find_structured_service_exn_annotation(*function_)) {
      for (const auto& item : annot->value()->get_map()) {
        if (item.first->get_string() == "anyhow_to_application_exn") {
          return get_annotation_property_bool(
              annot, "anyhow_to_application_exn");
        }
      }
    }

    // If not present on function, look at service annotations.
    if (const t_const* annot =
            find_structured_service_exn_annotation(interface())) {
      return get_annotation_property_bool(annot, "anyhow_to_application_exn");
    }

    return false;
  }

 private:
  const std::unordered_multiset<std::string>& function_upcamel_names_;
};

class rust_mstch_function_factory {
 public:
  std::shared_ptr<mstch_base> make_mstch_object(
      const t_function* function,
      mstch_context& ctx,
      mstch_element_position pos,
      const std::unordered_multiset<std::string>& function_upcamel_names)
      const {
    return std::make_shared<rust_mstch_function>(
        function, ctx, pos, function_upcamel_names);
  }
};

class rust_mstch_struct : public mstch_struct {
 public:
  rust_mstch_struct(
      const t_structured* s,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options)
      : mstch_struct(s, ctx, pos),
        options_(*options),
        adapter_annotation_(find_structured_adapter_annotation(*s)) {
    register_methods(
        this,
        {
            {"struct:rust_name", &rust_mstch_struct::rust_name},
            {"struct:package", &rust_mstch_struct::rust_package},
            {"struct:ord?", &rust_mstch_struct::rust_is_ord},
            {"struct:copy?", &rust_mstch_struct::rust_is_copy},
            {"struct:exhaustive?", &rust_mstch_struct::rust_is_exhaustive},
            {"struct:fields_by_name", &rust_mstch_struct::rust_fields_by_name},
            {"struct:fields_reversed",
             &rust_mstch_struct::rust_fields_reversed},
            {"struct:derive", &rust_mstch_struct::rust_derive},
            {"struct:has_exception_message?",
             &rust_mstch_struct::has_exception_message},
            {"struct:is_exception_message_optional?",
             &rust_mstch_struct::is_exception_message_optional},
            {"struct:exception_message", &rust_mstch_struct::exception_message},
            {"struct:serde?", &rust_mstch_struct::rust_serde},
            {"struct:has_adapter?", &rust_mstch_struct::has_adapter},
            {"struct:rust_structured_annotations",
             &rust_mstch_struct::rust_structured_annotations},
            {"struct:generated?", &rust_mstch_struct::rust_generated_struct},
            {"struct:all_optional?", &rust_mstch_struct::rust_all_optional},
        });
  }
  mstch::node rust_name() { return type_rust_name(struct_); }
  mstch::node rust_package() {
    return get_types_import_name(struct_->program(), options_);
  }
  mstch::node rust_is_ord() {
    if (struct_->has_structured_annotation(kRustOrdUri)) {
      return true;
    }

    for (const auto& field : struct_->fields()) {
      if (!can_derive_ord(field.get_type())) {
        return false;
      }

      // Assume we cannot derive `Ord` on the adapted type.
      if (node_has_adapter(field) ||
          type_has_transitive_adapter(field.get_type(), true)) {
        return false;
      }

      if (field.has_structured_annotation(kRustTypeUri)) {
        return false;
      }
    }
    return true;
  }
  mstch::node rust_is_copy() {
    return struct_->has_structured_annotation(kRustCopyUri);
  }
  mstch::node rust_is_exhaustive() {
    return struct_->has_structured_annotation(kRustExhaustiveUri);
  }
  mstch::node rust_fields_by_name() {
    auto fields = struct_->fields().copy();
    std::sort(fields.begin(), fields.end(), [](auto a, auto b) {
      return a->name() < b->name();
    });
    return make_mstch_fields(fields);
  }
  mstch::node rust_fields_reversed() {
    auto fields = struct_->fields().copy();
    std::reverse(fields.begin(), fields.end());
    return make_mstch_fields(fields);
  }
  mstch::node rust_derive() {
    if (auto annotation = find_structured_derive_annotation(*struct_)) {
      // Always replace `crate::` with the package name of where this
      // annotation originated to support derives applied with
      // `@scope.Transitive`. If the annotation originates from the same
      // module, this will just return `crate::` anyways to be a no-op.
      std::string package =
          get_types_import_name(annotation->program(), options_);

      std::string ret;
      std::string delimiter;

      for (const auto& item : annotation->value()->get_map()) {
        if (item.first->get_string() == "derives") {
          for (const t_const_value* val : item.second->get_list()) {
            auto str_val = val->get_string();

            if (!package.empty() &&
                boost::algorithm::starts_with(str_val, kRustCratePrefix)) {
              str_val =
                  package + "::" + str_val.substr(kRustCratePrefix.length());
            }

            ret = ret + delimiter + str_val;
            delimiter = ", ";
          }
        }
      }

      return ret;
    }

    return nullptr;
  }
  mstch::node has_exception_message() {
    return !!dynamic_cast<const t_exception&>(*struct_).get_message_field();
  }
  mstch::node is_exception_message_optional() {
    if (const auto* message_field =
            dynamic_cast<const t_exception&>(*struct_).get_message_field()) {
      return message_field->get_req() == t_field::e_req::optional;
    }
    return {};
  }
  mstch::node exception_message() {
    const auto* message_field =
        dynamic_cast<const t_exception&>(*struct_).get_message_field();
    return message_field ? message_field->name() : "";
  }
  mstch::node rust_serde() { return rust_serde_enabled(options_, *struct_); }
  mstch::node has_adapter() {
    // Structs cannot have transitive types, so we ignore the transitive type
    // check.
    return adapter_node(
        adapter_annotation_, struct_, true, context_, pos_, options_);
  }
  mstch::node rust_structured_annotations() {
    return structured_annotations_node(*struct_, 1, context_, pos_, options_);
  }
  mstch::node rust_generated_struct() { return (*struct_).generated(); }
  mstch::node rust_all_optional() {
    for (const auto& field : struct_->fields()) {
      if (field.get_req() != t_field::e_req::optional) {
        return false;
      }
    }
    return true;
  }

 private:
  const rust_codegen_options& options_;
  const t_const* adapter_annotation_;
};

class rust_mstch_enum : public mstch_enum {
 public:
  rust_mstch_enum(
      const t_enum* e,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options)
      : mstch_enum(e, ctx, pos), options_(*options) {
    register_methods(
        this,
        {
            {"enum:rust_name", &rust_mstch_enum::rust_name},
            {"enum:package", &rust_mstch_enum::rust_package},
            {"enum:serde?", &rust_mstch_enum::rust_serde},
            {"enum:derive", &rust_mstch_enum::rust_derive},
        });
  }
  mstch::node rust_derive() {
    if (auto annotation = find_structured_derive_annotation(*enum_)) {
      // Always replace `crate::` with the package name of where this
      // annotation originated to support derives applied with
      // `@scope.Transitive`. If the annotation originates from the same
      // module, this will just return `crate::` anyways to be a no-op.
      std::string package =
          get_types_import_name(annotation->program(), options_);

      std::string ret;
      std::string delimiter;

      for (const auto& item : annotation->value()->get_map()) {
        if (item.first->get_string() == "derives") {
          for (const t_const_value* val : item.second->get_list()) {
            auto str_val = val->get_string();

            if (!package.empty() &&
                boost::algorithm::starts_with(str_val, kRustCratePrefix)) {
              str_val =
                  package + "::" + str_val.substr(kRustCratePrefix.length());
            }

            ret = ret + delimiter + str_val;
            delimiter = ", ";
          }
        }
      }

      return ret;
    }
    return mstch::node();
  }
  mstch::node rust_name() { return type_rust_name(enum_); }
  mstch::node rust_package() {
    return get_types_import_name(enum_->program(), options_);
  }
  mstch::node rust_serde() { return rust_serde_enabled(options_, *enum_); }

 private:
  const rust_codegen_options& options_;
};

class rust_mstch_type : public mstch_type {
 public:
  rust_mstch_type(
      const t_type* type,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options)
      : mstch_type(type, ctx, pos), options_(*options) {
    register_methods(
        this,
        {
            {"type:rust_name", &rust_mstch_type::rust_name},
            {"type:rust_name_snake", &rust_mstch_type::rust_name_snake},
            {"type:package", &rust_mstch_type::rust_package},
            {"type:rust", &rust_mstch_type::rust_type},
            {"type:type_annotation?", &rust_mstch_type::rust_type_annotation},
            {"type:nonstandard?", &rust_mstch_type::rust_nonstandard},
            {"type:has_adapter?", &rust_mstch_type::adapter},
        });
  }
  mstch::node rust_name() { return type_rust_name(type_); }
  mstch::node rust_name_snake() {
    return snakecase(mangle_type(type_->name()));
  }
  mstch::node rust_package() {
    return get_types_import_name(type_->program(), options_);
  }
  mstch::node rust_type() {
    auto rust_type = get_type_annotation(type_);
    if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
      return "fbthrift::builtin_types::" + rust_type;
    }
    return rust_type;
  }
  mstch::node rust_type_annotation() {
    return has_type_annotation(type_) && !has_newtype_annotation(type_);
  }
  mstch::node rust_nonstandard() {
    return has_nonstandard_type_annotation(type_) &&
        !has_newtype_annotation(type_);
  }
  mstch::node adapter() {
    return adapter_node(nullptr, type_, false, context_, pos_, options_);
  }

 private:
  const rust_codegen_options& options_;
};

class mstch_rust_value : public mstch_base {
 public:
  using value_type = t_const_value::t_const_value_kind;
  mstch_rust_value(
      const t_const_value* const_value,
      const t_type* type,
      unsigned depth,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options& options)
      : mstch_base(ctx, pos),
        const_value_(const_value),
        local_type_(type),
        underlying_type_(step_through_typedefs(type, false)),
        depth_(depth),
        options_(options) {
    register_methods(
        this,
        {
            {"value:underlying_type", &mstch_rust_value::underlying_type},
            {"value:local_type", &mstch_rust_value::local_type},
            {"value:newtype?", &mstch_rust_value::is_newtype},
            {"value:inner", &mstch_rust_value::inner},
            {"value:bool?", &mstch_rust_value::is_bool},
            {"value:bool_value", &mstch_rust_value::bool_value},
            {"value:integer?", &mstch_rust_value::is_integer},
            {"value:integer_value", &mstch_rust_value::integer_value},
            {"value:floatingPoint?", &mstch_rust_value::is_floating_point},
            {"value:floatingPointValue",
             &mstch_rust_value::floating_point_value},
            {"value:string?", &mstch_rust_value::is_string},
            {"value:binary?", &mstch_rust_value::is_binary},
            {"value:quoted", &mstch_rust_value::string_quoted},
            {"value:list?", &mstch_rust_value::is_list},
            {"value:list_elements", &mstch_rust_value::list_elements},
            {"value:set?", &mstch_rust_value::is_set},
            {"value:setMembers", &mstch_rust_value::set_members},
            {"value:map?", &mstch_rust_value::is_map},
            {"value:mapEntries", &mstch_rust_value::map_entries},
            {"value:struct?", &mstch_rust_value::is_struct},
            {"value:structFields", &mstch_rust_value::struct_fields},
            {"value:exhaustive?", &mstch_rust_value::is_exhaustive},
            {"value:union?", &mstch_rust_value::is_union},
            {"value:unionVariant", &mstch_rust_value::union_variant},
            {"value:unionValue", &mstch_rust_value::union_value},
            {"value:enum?", &mstch_rust_value::is_enum},
            {"value:enumVariant", &mstch_rust_value::enum_variant},
            {"value:empty?", &mstch_rust_value::is_empty},
            {"value:indent", &mstch_rust_value::indent},
            {"value:simpleLiteral?", &mstch_rust_value::simple_literal},
        });
  }
  mstch::node underlying_type() {
    return context_.type_factory->make_mstch_object(
        underlying_type_, context_, pos_);
  }
  mstch::node local_type() {
    return context_.type_factory->make_mstch_object(
        local_type_, context_, pos_);
  }
  mstch::node is_newtype() { return has_newtype_annotation(underlying_type_); }
  mstch::node inner() {
    auto typedef_type = dynamic_cast<const t_typedef*>(underlying_type_);
    if (typedef_type) {
      auto inner_type = typedef_type->get_type();
      return std::make_shared<mstch_rust_value>(
          const_value_, inner_type, depth_, context_, pos_, options_);
    }
    return mstch::node();
  }
  mstch::node is_bool() { return underlying_type_->is_bool(); }
  mstch::node bool_value() {
    if (const_value_->kind() == value_type::CV_INTEGER) {
      return const_value_->get_integer() != 0;
    }
    return const_value_->get_bool();
  }
  mstch::node is_integer() {
    return underlying_type_->is_byte() || underlying_type_->is_i16() ||
        underlying_type_->is_i32() || underlying_type_->is_i64();
  }
  mstch::node integer_value() {
    return std::to_string(const_value_->get_integer());
  }
  mstch::node is_floating_point() {
    return underlying_type_->is_float() || underlying_type_->is_double();
  }
  mstch::node floating_point_value() {
    auto str = fmt::format(
        "{}",
        const_value_->kind() == value_type::CV_INTEGER
            ? const_value_->get_integer()
            : const_value_->get_double());

    if (str.find('.') == std::string::npos &&
        str.find('e') == std::string::npos &&
        str.find('E') == std::string::npos) {
      str += ".0";
    }
    return str;
  }
  mstch::node is_string() { return underlying_type_->is_string(); }
  mstch::node is_binary() { return underlying_type_->is_binary(); }
  mstch::node string_quoted() {
    return quote(const_value_->get_string(), false);
  }
  mstch::node is_list() {
    return underlying_type_->is<t_list>() &&
        (const_value_->kind() == value_type::CV_LIST ||
         (const_value_->kind() == value_type::CV_MAP &&
          const_value_->get_map().empty()));
  }
  mstch::node list_elements() {
    const t_type* elem_type;
    if (underlying_type_->is<t_set>()) {
      auto set_type = dynamic_cast<const t_set*>(underlying_type_);
      if (!set_type) {
        return mstch::node();
      }
      elem_type = set_type->get_elem_type();
    } else {
      auto list_type = dynamic_cast<const t_list*>(underlying_type_);
      if (!list_type) {
        return mstch::node();
      }
      elem_type = list_type->get_elem_type();
    }

    mstch::array elements;
    for (auto elem : const_value_->get_list()) {
      elements.emplace_back(std::make_shared<mstch_rust_value>(
          elem, elem_type, depth_ + 1, context_, pos_, options_));
    }
    return elements;
  }
  mstch::node is_set() {
    return underlying_type_->is<t_set>() &&
        (const_value_->kind() == value_type::CV_LIST ||
         (const_value_->kind() == value_type::CV_MAP &&
          const_value_->get_map().empty()));
  }
  mstch::node set_members() { return list_elements(); }
  mstch::node is_map() {
    return underlying_type_->is<t_map>() &&
        (const_value_->kind() == value_type::CV_MAP ||
         (const_value_->kind() == value_type::CV_LIST &&
          const_value_->get_list().empty()));
  }
  mstch::node map_entries();
  mstch::node is_struct() {
    return underlying_type_->is<t_structured>() &&
        !underlying_type_->is<t_union>() &&
        const_value_->kind() == value_type::CV_MAP;
  }
  mstch::node struct_fields();
  mstch::node is_exhaustive();
  mstch::node is_union() {
    if (!underlying_type_->is<t_union>() ||
        const_value_->kind() != value_type::CV_MAP) {
      return false;
    }
    if (const_value_->get_map().empty()) {
      // value will be the union's Default
      return true;
    }
    return const_value_->get_map().size() == 1 &&
        const_value_->get_map().at(0).first->kind() == value_type::CV_STRING;
  }
  mstch::node union_variant() {
    auto struct_type = dynamic_cast<const t_structured*>(underlying_type_);
    if (!struct_type) {
      return mstch::node();
    }

    if (const_value_->get_map().empty()) {
      return mstch::node();
    }

    const auto& entry = const_value_->get_map().at(0);
    const auto& variant = entry.first->get_string();

    for (auto&& field : struct_type->fields()) {
      if (field.name() == variant) {
        if (const t_const* annot =
                field.find_structured_annotation_or_null(kRustNameUri)) {
          return get_annotation_property_string(annot, "name");
        } else {
          return variant;
        }
      }
    }
    return mstch::node();
  }
  mstch::node union_value() {
    auto struct_type = dynamic_cast<const t_structured*>(underlying_type_);
    if (!struct_type) {
      return mstch::node();
    }

    const auto& entry = const_value_->get_map().at(0);
    const auto& variant = entry.first->get_string();
    const auto* content = entry.second;

    for (auto&& field : struct_type->fields()) {
      if (field.name() == variant) {
        return std::make_shared<mstch_rust_value>(
            content, field.get_type(), depth_ + 1, context_, pos_, options_);
      }
    }
    return mstch::node();
  }
  mstch::node is_enum() { return underlying_type_->is<t_enum>(); }
  mstch::node enum_variant() {
    if (const_value_->is_enum()) {
      auto enum_value = const_value_->get_enum_value();
      if (enum_value) {
        return mangle(enum_value->name());
      }
    }
    return mstch::node();
  }
  mstch::node is_empty() {
    auto kind = const_value_->kind();
    if (kind == value_type::CV_LIST) {
      return const_value_->get_list().empty();
    }
    if (kind == value_type::CV_MAP) {
      return const_value_->get_map().empty();
    }
    if (kind == value_type::CV_STRING) {
      return const_value_->get_string().empty();
    }
    return false;
  }
  mstch::node indent() { return std::string(4 * depth_, ' '); }
  mstch::node simple_literal() {
    // Primitives have simple literals
    if (underlying_type_->is_bool() || underlying_type_->is_byte() ||
        underlying_type_->is_any_int() ||
        underlying_type_->is_floating_point()) {
      return true;
    }
    // Enum variants as well
    if (underlying_type_->is<t_enum>()) {
      return enum_variant();
    }
    return false;
  }

 private:
  const t_const_value* const_value_;

  // The type (potentially a typedef) by which the value's type is known to
  // the current crate.
  const t_type* local_type_;

  // The underlying type of the value after stepping through any non-newtype
  // typedefs.
  const t_type* underlying_type_;

  unsigned depth_;
  const rust_codegen_options& options_;
};

class mstch_rust_map_entry : public mstch_base {
 public:
  mstch_rust_map_entry(
      const t_const_value* key,
      const t_type* key_type,
      const t_const_value* value,
      const t_type* value_type,
      unsigned depth,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options& options)
      : mstch_base(ctx, pos),
        key_(key),
        key_type_(key_type),
        value_(value),
        value_type_(value_type),
        depth_(depth),
        options_(options) {
    register_methods(
        this,
        {
            {"entry:key", &mstch_rust_map_entry::key},
            {"entry:value", &mstch_rust_map_entry::value},
        });
  }
  mstch::node key() {
    return std::make_shared<mstch_rust_value>(
        key_, key_type_, depth_, context_, pos_, options_);
  }
  mstch::node value() {
    return std::make_shared<mstch_rust_value>(
        value_, value_type_, depth_, context_, pos_, options_);
  }

 private:
  const t_const_value* key_;
  const t_type* key_type_;
  const t_const_value* value_;
  const t_type* value_type_;
  unsigned depth_;
  const rust_codegen_options& options_;
};

class mstch_rust_struct_field : public mstch_base {
 public:
  mstch_rust_struct_field(
      const t_field* field,
      const t_const_value* explicit_value,
      unsigned depth,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options& options)
      : mstch_base(ctx, pos),
        field_(field),
        explicit_value_(explicit_value),
        depth_(depth),
        options_(options),
        adapter_annotation_(find_structured_adapter_annotation(*field)) {
    register_methods(
        this,
        {
            {"field:self", &mstch_rust_struct_field::self},
            {"field:explicit_value", &mstch_rust_struct_field::explicit_value},
            {"field:default", &mstch_rust_struct_field::rust_default},
            {"field:type", &mstch_rust_struct_field::type},
            {"field:box?", &mstch_rust_struct_field::is_boxed},
            {"field:arc?", &mstch_rust_struct_field::is_arc},
            {"field:docs?", &mstch_rust_struct_field::rust_has_docs},
            {"field:has_adapter?", &mstch_rust_struct_field::has_adapter},
        });
  }
  whisker::object self() { return make_self(*field_); }
  mstch::node explicit_value() {
    if (explicit_value_) {
      auto type = field_->get_type();
      return std::make_shared<mstch_rust_value>(
          explicit_value_, type, depth_, context_, pos_, options_);
    }
    return mstch::node();
  }
  mstch::node rust_default() {
    if (auto default_value = field_->get_value()) {
      return std::make_shared<mstch_rust_value>(
          default_value, field_->get_type(), depth_, context_, pos_, options_);
    }
    return mstch::node();
  }
  mstch::node type() {
    auto type = field_->get_type();
    return context_.type_factory->make_mstch_object(type, context_, pos_);
  }
  mstch::node is_boxed() { return field_kind(*field_) == FieldKind::Box; }
  mstch::node is_arc() { return field_kind(*field_) == FieldKind::Arc; }
  mstch::node rust_has_docs() { return field_->has_doc(); }
  mstch::node has_adapter() {
    return adapter_node(
        adapter_annotation_,
        field_->get_type(),
        false,
        context_,
        pos_,
        options_);
  }

 private:
  const t_field* field_;
  const t_const_value* explicit_value_;
  unsigned depth_;
  const rust_codegen_options& options_;
  const t_const* adapter_annotation_;
};

mstch::node mstch_rust_value::map_entries() {
  auto map_type = dynamic_cast<const t_map*>(underlying_type_);
  if (!map_type) {
    return mstch::node();
  }
  auto key_type = &map_type->key_type().deref();
  auto value_type = &map_type->val_type().deref();

  mstch::array entries;
  for (auto entry : const_value_->get_map()) {
    entries.emplace_back(std::make_shared<mstch_rust_map_entry>(
        entry.first,
        key_type,
        entry.second,
        value_type,
        depth_ + 3,
        context_,
        pos_,
        options_));
  }
  return entries;
}

mstch::node mstch_rust_value::struct_fields() {
  auto struct_type = dynamic_cast<const t_structured*>(underlying_type_);
  if (!struct_type) {
    return mstch::node();
  }

  std::map<std::string, const t_const_value*> map_entries;
  for (auto entry : const_value_->get_map()) {
    auto key = entry.first;
    if (key->kind() == value_type::CV_STRING) {
      map_entries[key->get_string()] = entry.second;
    }
  }

  mstch::array fields;
  for (auto&& field : struct_type->fields()) {
    auto explicit_value = map_entries[field.name()];
    fields.emplace_back(std::make_shared<mstch_rust_struct_field>(
        &field, explicit_value, depth_ + 1, context_, pos_, options_));
  }
  return fields;
}

mstch::node mstch_rust_value::is_exhaustive() {
  auto struct_type = dynamic_cast<const t_structured*>(underlying_type_);
  return struct_type &&
      struct_type->has_structured_annotation(kRustExhaustiveUri);
}

class rust_mstch_const : public mstch_const {
 public:
  rust_mstch_const(
      const t_const* c,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      const t_field* field,
      const rust_codegen_options* options)
      : mstch_const(c, ctx, pos, current_const, expected_type, field),
        options_(*options) {
    register_methods(
        this,
        {
            {"constant:lazy?", &rust_mstch_const::rust_lazy},
            {"constant:rust", &rust_mstch_const::rust_typed_value},
        });
  }
  mstch::node rust_lazy() {
    if (type_has_transitive_adapter(const_->type(), true)) {
      return true;
    }

    auto type = const_->type()->get_true_type();
    return type->is<t_list>() || type->is<t_map>() || type->is<t_set>() ||
        type->is<t_struct>() || type->is<t_union>();
  }
  mstch::node rust_typed_value() {
    unsigned depth = 0;
    return std::make_shared<mstch_rust_value>(
        const_->value(), const_->type(), depth, context_, pos_, options_);
  }

 private:
  const rust_codegen_options& options_;
};

class rust_mstch_field : public mstch_field {
 public:
  rust_mstch_field(
      const t_field* field,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options)
      : mstch_field(field, ctx, pos),
        options_(*options),
        adapter_annotation_(find_structured_adapter_annotation(*field)) {
    register_methods(
        this,
        {
            {"field:primitive?", &rust_mstch_field::rust_primitive},
            {"field:rename?", &rust_mstch_field::rust_rename},
            {"field:default", &rust_mstch_field::rust_default},
            {"field:box?", &rust_mstch_field::rust_is_boxed},
            {"field:arc?", &rust_mstch_field::rust_is_arc},
            {"field:type_annotation?", &rust_mstch_field::rust_type_annotation},
            {"field:type_nonstandard?",
             &rust_mstch_field::rust_type_nonstandard},
            {"field:type_rust", &rust_mstch_field::rust_type},
            {"field:has_adapter?", &rust_mstch_field::has_adapter},
            {"field:rust_structured_annotations",
             &rust_mstch_field::rust_structured_annotations},
        });
  }
  mstch::node rust_type_annotation() { return has_type_annotation(field_); }
  mstch::node rust_type_nonstandard() {
    return has_nonstandard_type_annotation(field_);
  }
  mstch::node rust_type() {
    auto rust_type = get_type_annotation(field_);
    if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
      return "fbthrift::builtin_types::" + rust_type;
    }
    return rust_type;
  }
  mstch::node rust_primitive() {
    auto type = field_->get_type();
    return type->is_bool() || type->is_any_int() || type->is_floating_point();
  }
  mstch::node rust_rename() { return field_->name() != mangle(field_->name()); }
  mstch::node rust_default() {
    auto value = field_->get_value();
    if (value) {
      unsigned depth = 2; // impl Default + fn default
      auto type = field_->get_type();
      return std::make_shared<mstch_rust_value>(
          value, type, depth, context_, pos_, options_);
    }
    return mstch::node();
  }
  mstch::node rust_is_boxed() { return field_kind(*field_) == FieldKind::Box; }
  mstch::node rust_is_arc() { return field_kind(*field_) == FieldKind::Arc; }
  mstch::node has_adapter() {
    return adapter_node(
        adapter_annotation_,
        field_->get_type(),
        false,
        context_,
        pos_,
        options_);
  }
  mstch::node rust_structured_annotations() {
    return structured_annotations_node(*field_, 3, context_, pos_, options_);
  }

 private:
  const rust_codegen_options& options_;
  const t_const* adapter_annotation_;
};

class rust_mstch_typedef : public mstch_typedef {
 public:
  rust_mstch_typedef(
      const t_typedef* t,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options)
      : mstch_typedef(t, ctx, pos),
        options_(*options),
        adapter_annotation_(find_structured_adapter_annotation(*t)) {
    register_methods(
        this,
        {
            {"typedef:rust_name", &rust_mstch_typedef::rust_name},
            {"typedef:newtype?", &rust_mstch_typedef::rust_newtype},
            {"typedef:ord?", &rust_mstch_typedef::rust_ord},
            {"typedef:copy?", &rust_mstch_typedef::rust_copy},
            {"typedef:rust_type", &rust_mstch_typedef::rust_type},
            {"typedef:nonstandard?", &rust_mstch_typedef::rust_nonstandard},
            {"typedef:serde?", &rust_mstch_typedef::rust_serde},
            {"typedef:has_adapter?", &rust_mstch_typedef::has_adapter},
            {"typedef:constructor?", &rust_mstch_typedef::constructor},
        });
  }
  mstch::node rust_name() { return type_rust_name(typedef_); }
  mstch::node rust_newtype() { return has_newtype_annotation(typedef_); }
  mstch::node rust_type() {
    // See 'typedef.mustache'. The context is writing a newtype: e.g. `pub
    // struct T(pub X)`. If `X` has a `rust.Type` annotation `A` we should
    // write `struct T(pub A)` If it does not, we should write `pub struct T
    // (pub X)`.
    std::string rust_type;
    if (const t_const* annot =
            typedef_->find_structured_annotation_or_null(kRustTypeUri)) {
      rust_type = get_annotation_property_string(annot, "name");
    }
    if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
      return "fbthrift::builtin_types::" + rust_type;
    }
    return rust_type;
  }
  mstch::node rust_ord() {
    return typedef_->has_structured_annotation(kRustOrdUri) ||
        (can_derive_ord(typedef_) &&
         !type_has_transitive_adapter(typedef_->get_type(), true));
  }
  mstch::node rust_copy() {
    if (!type_has_transitive_adapter(typedef_->get_type(), true)) {
      auto inner = typedef_->get_true_type();
      if (inner->is_bool() || inner->is_byte() || inner->is_i16() ||
          inner->is_i32() || inner->is_i64() || inner->is<t_enum>() ||
          inner->is_void()) {
        return true;
      }
    }
    return false;
  }
  mstch::node rust_nonstandard() {
    // See 'typedef.mustache'. The context is writing serialization functions
    // for a newtype `pub struct T(pub X)`.
    // If `X` has a type annotation `A` that is non-standard we should emit
    // the phrase `crate::r#impl::write(&self.0, p)`. If `X` does not have an
    // annotation or does but it is not non-standard we should write
    // `self.0.write(p)`.
    std::string rust_type;
    if (const t_const* annot =
            typedef_->find_structured_annotation_or_null(kRustTypeUri)) {
      rust_type = get_annotation_property_string(annot, "name");
    }
    return rust_type.find("::") != std::string::npos;
  }
  mstch::node rust_serde() { return rust_serde_enabled(options_, *typedef_); }
  mstch::node has_adapter() {
    return adapter_node(
        adapter_annotation_,
        typedef_->get_type(),
        false,
        context_,
        pos_,
        options_);
  }
  mstch::node constructor() {
    return typedef_has_constructor_expression(typedef_);
  }

 private:
  const rust_codegen_options& options_;
  const t_const* adapter_annotation_;
};

mstch::node rust_mstch_service::rust_functions() {
  return make_mstch_array(
      service_->get_functions(),
      rust_mstch_function_factory(),
      function_upcamel_names_);
}

mstch::node rust_mstch_service::rust_all_exceptions() {
  struct name_less {
    bool operator()(const t_type* lhs, const t_type* rhs) const {
      return lhs->get_scoped_name() < rhs->get_scoped_name();
    }
  };
  using fields_t = std::vector<const t_field*>;
  using functions_t = std::vector<const t_function*>;
  using field_map_t = std::map<const t_type*, fields_t>;
  using function_map_t = std::map<const t_type*, functions_t, name_less>;

  field_map_t field_map;
  function_map_t function_map;
  for (const auto& fun : service_->functions()) {
    for (const t_field& fld : get_elems(fun.exceptions())) {
      function_map[&fld.type().deref()].push_back(&fun);
      field_map[&fld.type().deref()].push_back(&fld);
    }
  }

  mstch::array output;
  for (const auto& funcs : function_map) {
    mstch::map data;
    data["rust_exception:type"] =
        context_.type_factory->make_mstch_object(funcs.first, context_, {});

    auto functions = make_mstch_array(
        funcs.second, rust_mstch_function_factory(), function_upcamel_names_);
    auto fields = make_mstch_fields(field_map[funcs.first]);

    mstch::array function_data;
    for (size_t i = 0; i < fields.size(); i++) {
      mstch::map inner;
      inner["rust_exception_function:function"] = std::move(functions[i]);
      inner["rust_exception_function:field"] = std::move(fields[i]);
      function_data.emplace_back(std::move(inner));
    }

    data["rust_exception:functions"] = std::move(function_data);
    output.emplace_back(data);
  }

  return output;
}

void t_mstch_rust_generator::generate_program() {
  if (auto types_crate_flag = get_option("types_crate")) {
    options_.types_crate =
        boost::algorithm::replace_all_copy(*types_crate_flag, "-", "_");
  }

  if (auto clients_crate_flag = get_option("clients_crate")) {
    options_.clients_crate =
        boost::algorithm::replace_all_copy(*clients_crate_flag, "-", "_");
  }

  if (auto cratemap_flag = get_option("cratemap")) {
    auto cratemap = load_crate_map(*cratemap_flag);
    options_.multifile_mode = cratemap.multifile_mode;
    options_.label = std::move(cratemap.label);
    options_.crate_index =
        rust_crate_index{program_, std::move(cratemap.cratemap)};
  }

  options_.serde = has_option("serde");
  options_.skip_none_serialization = has_option("skip_none_serialization");
  if (options_.skip_none_serialization) {
    assert(options_.serde);
  }

  options_.valuable = has_option("valuable");

  options_.any_registry_initialization_enabled = has_option("any");

  std::optional<std::string> maybe_number = get_option("types_split_count");
  if (maybe_number) {
    options_.types_split_count = checked_stoi(
        maybe_number.value(),
        "Invalid types_split_count '" + maybe_number.value() + "'");
  }

  parse_include_srcs(
      options_.types_include_srcs, get_option("types_include_srcs"));
  parse_include_srcs(
      options_.clients_include_srcs, get_option("clients_include_srcs"));
  parse_include_srcs(
      options_.services_include_srcs, get_option("services_include_srcs"));

  if (auto include_docs = get_option("include_docs")) {
    options_.include_docs = include_docs.value();
  }

  if (options_.multifile_mode) {
    options_.current_crate = "crate::" + multifile_module_name(program_);
  } else {
    options_.current_crate = "crate";
  }

  options_.current_program = program_;
  out_dir_base_ = "gen-rust";

  if (auto gen_metadata = get_option("gen_metadata")) {
    options_.gen_metadata = gen_metadata.value() == "true";
  }

  std::optional<std::string> crate_name_option = get_option("crate_name");
  std::string namespace_rust = program_->get_namespace("rust");
  if (!namespace_rust.empty()) {
    std::vector<std::string> pieces;
    boost::split(pieces, namespace_rust, boost::is_any_of("."));
    if (options_.multifile_mode) {
      if (pieces.size() > 2) {
        throw std::runtime_error(fmt::format(
            "`namespace rust {}`: namespace for multi-file Thrift library must have 1 piece, or 2 pieces separated by '.'",
            namespace_rust));
      }
    } else {
      if (pieces.size() != 1) {
        throw std::runtime_error(fmt::format(
            "`namespace rust {}`: namespace for single-file Thrift library must not contain '.'",
            namespace_rust));
      }
    }
    if (crate_name_option && pieces[0] != *crate_name_option) {
      throw std::runtime_error(fmt::format(
          R"(`namespace rust` disagrees with rust_crate_name option: "{}" vs "{}")",
          pieces[0],
          *crate_name_option));
    }
  } else if (crate_name_option) {
    namespace_rust = *crate_name_option;
  } else if (
      std::optional<std::string> default_crate_name_option =
          get_option("default_crate_name")) {
    namespace_rust = *default_crate_name_option;
  } else {
    namespace_rust = program_->name();
  }

  std::string namespace_cpp2 = cpp_name_resolver::gen_namespace(*program_);

  std::string service_names;
  for (const t_service* service : program_->services()) {
    service_names += named_rust_name(service);
    service_names += '\n';
  }

  set_mstch_factories();

  if (options_.types_split_count > 0) {
    generate_split_types();
  }
  const auto& prog = cached_program(program_);
  render_to_file(prog, "types.rs", "types.rs");
  render_to_file(prog, "services.rs", "services.rs");
  render_to_file(prog, "errors.rs", "errors.rs");
  render_to_file(prog, "consts.rs", "consts.rs");
  render_to_file(prog, "client.rs", "client.rs");
  render_to_file(prog, "server.rs", "server.rs");
  render_to_file(prog, "mock.rs", "mock.rs");
  write_output("namespace-rust", namespace_rust + '\n');
  write_output("namespace-cpp2", namespace_cpp2 + '\n');
  write_output("service-names", service_names);
}

void t_mstch_rust_generator::generate_split_types() {
  // Generate individual split files
  for (int split_id = 0; split_id <= options_.types_split_count; ++split_id) {
    auto split_program = std::make_shared<rust_mstch_program>(
        program_,
        mstch_context_,
        mstch_element_position(),
        &options_,
        split_id);

    render_to_file(
        std::shared_ptr<mstch_base>(split_program),
        "lib/types_split",
        fmt::format("types_{}.rs", split_id));
  }
}

void t_mstch_rust_generator::set_mstch_factories() {
  mstch_context_.add<rust_mstch_program>(&options_);
  mstch_context_.add<rust_mstch_service>(&options_);
  mstch_context_.add<rust_mstch_interaction>(&options_);
  mstch_context_.add<rust_mstch_type>(&options_);
  mstch_context_.add<rust_mstch_typedef>(&options_);
  mstch_context_.add<rust_mstch_struct>(&options_);
  mstch_context_.add<rust_mstch_field>(&options_);
  mstch_context_.add<rust_mstch_enum>(&options_);
  mstch_context_.add<rust_mstch_const>(&options_);
}

namespace {

void validate_struct_annotations(
    sema_context& ctx,
    const t_structured& s,
    const rust_codegen_options& options) {
  for (auto& field : s.fields()) {
    if (field.has_structured_annotation(kRustBoxUri) &&
        field.has_structured_annotation(kRustArcUri)) {
      ctx.report(
          field,
          "rust-field-box-arc-rule",
          diagnostic_level::error,
          "Field `{}` cannot be both Box'ed and Arc'ed",
          field.name());
    }

    if (node_has_adapter(field) &&
        (node_has_custom_rust_type(field) ||
         rust_serde_enabled(options, field))) {
      ctx.report(
          field,
          "rust-field-adapter-rule",
          diagnostic_level::error,
          "Field `{}` cannot have both an adapter and `rust.type` or `rust.newtype` or `rust.serde = true`",
          field.name());
    }
  }
}

bool validate_program_annotations(sema_context& ctx, const t_program& program) {
  for (auto t : program.typedefs()) {
    if (node_has_adapter(*t)) {
      if (node_has_custom_rust_type(*t)) {
        ctx.report(
            *t,
            "rust-typedef-adapter-rule",
            diagnostic_level::error,
            "Typedef `{}` cannot have both an adapter and `rust.type` or `rust.newtype`",
            t->name());
      }

      // To be spec compliant, adapted typedefs can be composed only if they
      // are wrapped. For example, the following is not allowed:
      //
      // ```
      // @rust.Adapter{name = "Foo"}
      // typedef string Foo
      //
      // @rust.Adapter{name = "Bar"}
      // typedef Foo Bar
      // ```
      // But the following is allowed:
      // ```
      // @rust.Adapter{name = "Foo"}
      // typedef string Foo
      //
      // @rust.Adapter{name = "Bar"}
      // typedef list<Foo> Bar
      // ```
      const t_type* typedef_stepped =
          step_through_typedefs(t->get_type(), true);

      if (node_has_adapter(*typedef_stepped)) {
        ctx.report(
            *t,
            "rust-typedef-transitive-adapter-rule",
            diagnostic_level::error,
            "Typedef `{}` cannot have a direct transitive adapter",
            t->name());
      }
    }
  }

  return true;
}

} // namespace

void t_mstch_rust_generator::fill_validator_visitors(
    ast_validator& validator) const {
  validator.add_structured_definition_visitor(std::bind(
      validate_struct_annotations,
      std::placeholders::_1,
      std::placeholders::_2,
      options_));
  validator.add_program_visitor(validate_program_annotations);
}

THRIFT_REGISTER_GENERATOR(
    mstch_rust,
    "Rust",
    "    serde:           Derive serde Serialize/Deserialize traits for types\n"
    "    valuable:        Derive valuable Valuable trait for types\n"
    "    any:             Generate \"any registry\" initialization functions\n"
    "    deprecated_default_enum_min_i32:\n"
    "                     Default enum value is i32::MIN. Deprecated, to be removed in future versions\n"
    "    deprecated_optional_with_default_is_some:\n"
    "                     Optionals with defaults initialized to `Some`. Deprecated, to be removed in future versions\n"
    "    include_prefix=: Set program:include_prefix.\n"
    "    types_include_srcs=, clients_include_srcs=, services_include_srcs=:\n"
    "                     Additional Rust source files to include in each crate, `:` separated\n"
    "    include_docs=:   Markdown to include in front of crate-level documentation.\n"
    "    cratemap=map:    Mapping file from services to crate names\n"
    "    types_crate=:    Name that the main crate uses to refer to its dependency on the types crate\n"
    "    types_split_count=:    Number of compilation units to split the independent types into\n");
} // namespace apache::thrift::compiler::rust
