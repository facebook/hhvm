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
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/rust/uri.h>
#include <thrift/compiler/generate/rust/util.h>
#include <thrift/compiler/generate/t_whisker_generator.h>
#include <thrift/compiler/generate/templates.h>
#include <thrift/compiler/sema/ast_validator.h>
#include <thrift/compiler/sema/sema_context.h>

namespace apache::thrift::compiler::rust {

namespace {

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
  std::vector<std::string> types_include_srcs;
  std::vector<std::string> clients_include_srcs;
  std::vector<std::string> services_include_srcs;

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
    auto elem_type = list->elem_type().get_type();
    return elem_type && can_derive_ord(elem_type);
  }
  // We can implement Ord on BTreeMap (the default map type) if both the key and
  // value implement Eq.
  if (const t_map* map_type = type->try_as<t_map>();
      map_type != nullptr && !has_custom_type_annotation) {
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
    std::vector<std::string>& elements,
    std::optional<std::string_view> include_srcs) {
  if (!include_srcs) {
    return;
  }
  const auto& paths = *include_srcs;
  std::string_view::size_type pos = 0;
  while (pos != std::string_view::npos && pos < paths.size()) {
    std::string_view::size_type next_pos = paths.find(':', pos);
    elements.emplace_back(paths.substr(pos, next_pos - pos));
    pos = ((next_pos == std::string_view::npos) ? next_pos : next_pos + 1);
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

// Used to create the part contained within the parentheses of the derived
// string "#[derive(Debug, Clone, Copy, PartialEq, Default)]" for structs and
// enums based on annotations within the thrift file
std::string compute_derive_string(
    const t_named& node, const rust_codegen_options& options) {
  if (auto annotation = find_structured_derive_annotation(node)) {
    std::string package = get_types_import_name(annotation->program(), options);
    std::string ret;
    std::string delimiter;
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == "derives") {
        for (const t_const_value* val : item.second->get_list()) {
          auto str_val = val->get_string();
          if (!package.empty() && str_val.starts_with(kRustCratePrefix)) {
            str_val = package.append("::").append(
                str_val.substr(kRustCratePrefix.length()));
          }
          ret = ret.append(delimiter).append(str_val);
          delimiter = ", ";
        }
      }
    }
    return ret;
  }
  return std::string();
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
        &typedef_type->type().deref(), step_through_newtypes);

  } else if (const t_list* list_type = type->try_as<t_list>()) {
    return type_has_transitive_adapter(
        list_type->elem_type().get_type(), step_through_newtypes);

  } else if (const t_set* set_type = type->try_as<t_set>()) {
    return type_has_transitive_adapter(
        set_type->elem_type().get_type(), step_through_newtypes);

  } else if (const t_map* map_type = type->try_as<t_map>()) {
    return type_has_transitive_adapter(
               &map_type->key_type().deref(), step_through_newtypes) ||
        type_has_transitive_adapter(
               &map_type->val_type().deref(), step_through_newtypes);

  } else if (const t_structured* struct_type = type->try_as<t_structured>()) {
    return node_has_adapter(*struct_type);
  }

  return false;
}

const t_type* step_through_typedefs(const t_type* t, bool break_on_adapter) {
  const t_type* stepped =
      // Find first type which is:
      // - not a typedef
      // - typedef with the NewType annotation
      // - typedef with an adapter, where break_on_adapter is true
      t_typedef::find_type_if(t, [break_on_adapter](const t_type* type) {
        const t_typedef* typedef_type = type->try_as<t_typedef>();
        return typedef_type == nullptr ||
            has_newtype_annotation(typedef_type) ||
            (break_on_adapter && node_has_adapter(*typedef_type));
      });
  return stepped == nullptr ? t : stepped;
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
    const t_type& definition = *t->type();
    if (definition.is<t_enum>()) {
      // Outermost typedef refers to an enum.
      return true;
    }
    t = definition.try_as<t_typedef>();
  } while (t);
  return false;
}

bool node_has_custom_rust_type(const t_named& node) {
  return node.has_structured_annotation(kRustTypeUri) ||
      node.has_structured_annotation(kRustNewTypeUri);
}

// Compute the Rust type name string for a type, replicating `lib/type.mustache`
// logic. This is needed for generic adapter type parameters.
std::string compute_type_name(
    const t_type* type, const rust_codegen_options& options);

// Forward declarations for adapter name computation.
std::string compute_transitive_adapter_name(
    const t_type* type,
    const rust_codegen_options& options,
    const t_field* field);

// Resolve the direct adapter annotation name, applying crate path fixups.
std::string resolve_adapter_annotation_name(
    const t_const* adapter_annotation, const rust_codegen_options& options) {
  std::string package =
      get_types_import_name(adapter_annotation->program(), options);
  auto adapter_name =
      get_annotation_property_string(adapter_annotation, "name");

  bool is_generic = adapter_name.ends_with("<>");
  if (is_generic) {
    adapter_name = adapter_name.substr(0, adapter_name.length() - 2);
  }

  if (!package.empty() && adapter_name.starts_with(kRustCrateTypesPrefix)) {
    adapter_name =
        package + "::" + adapter_name.substr(kRustCrateTypesPrefix.length());
  } else if (!package.empty() && adapter_name.starts_with(kRustCratePrefix)) {
    adapter_name =
        package + "::" + adapter_name.substr(kRustCratePrefix.length());
  } else if (!(adapter_name.starts_with("::") ||
               adapter_name.starts_with(kRustCratePrefix))) {
    adapter_name = "::" + adapter_name;
  }

  return adapter_name;
}

// Compute the Rust raw type name (like lib/rawtype.mustache).
std::string compute_rawtype_name(
    const t_type* type, const rust_codegen_options& options) {
  auto rust_type = get_type_annotation(type);
  if (!rust_type.empty()) {
    if (rust_type.find("::") == std::string::npos) {
      return "fbthrift::builtin_types::" + rust_type;
    }
    return "::" + rust_type;
  }

  if (type->is_void()) {
    return "()";
  }
  if (type->is_bool()) {
    return "::std::primitive::bool";
  }
  if (type->is_byte()) {
    return "::std::primitive::i8";
  }
  if (type->is_i16()) {
    return "::std::primitive::i16";
  }
  if (type->is_i32()) {
    return "::std::primitive::i32";
  }
  if (type->is_i64()) {
    return "::std::primitive::i64";
  }
  if (type->is_float()) {
    return "::std::primitive::f32";
  }
  if (type->is_double()) {
    return "::std::primitive::f64";
  }
  if (type->is_binary()) {
    return "::std::vec::Vec<::std::primitive::u8>";
  }
  if (type->is_string()) {
    return "::std::string::String";
  }
  if (type->is<t_enum>() || type->is<t_structured>()) {
    std::string prefix = get_types_import_name(type->program(), options);
    if (type->is<t_structured>() && node_has_adapter(*type)) {
      return prefix + "::unadapted::" + type_rust_name(type);
    }
    return prefix + "::" + type_rust_name(type);
  }
  if (const auto* list_type = type->try_as<t_list>()) {
    return "::std::vec::Vec<" +
        compute_type_name(list_type->elem_type().get_type(), options) + ">";
  }
  if (const auto* set_type = type->try_as<t_set>()) {
    return "::std::collections::BTreeSet<" +
        compute_type_name(set_type->elem_type().get_type(), options) + ">";
  }
  if (const auto* map_type = type->try_as<t_map>()) {
    return "::std::collections::BTreeMap<" +
        compute_type_name(&map_type->key_type().deref(), options) + ", " +
        compute_type_name(&map_type->val_type().deref(), options) + ">";
  }
  return type_rust_name(type);
}

// Compute the full type name, handling typedefs (like lib/type.mustache).
std::string compute_type_name(
    const t_type* type, const rust_codegen_options& options) {
  if (type->is<t_typedef>()) {
    if (has_type_annotation(type) && !has_newtype_annotation(type)) {
      return compute_rawtype_name(type, options);
    }
    return get_types_import_name(type->program(), options) +
        "::" + type_rust_name(type);
  }
  return compute_rawtype_name(type, options);
}

// Compute the adapter name string. This replicates the logic of
// `lib/adapter/name.mustache`.
//
// Parameters:
// - adapter_annotation: the direct adapter annotation on the field/typedef
//   (may be null)
// - type: the type (for typedef: the underlying type; for field: the field
//   type; for struct: the struct itself)
// - ignore_transitive_check: true for structs (which cannot have transitive
//   adapters)
// - options: codegen options
// - field: optional field pointer, used for struct-context adapter name
//   computation where field type annotations affect container wrappers
std::string compute_adapter_name(
    const t_const* adapter_annotation,
    const t_type* type,
    bool ignore_transitive_check,
    const rust_codegen_options& options,
    const t_field* field = nullptr) {
  const t_type* curr_type = step_through_typedefs(type, true);
  const t_type* transitive_type = nullptr;

  if (!ignore_transitive_check &&
      type_has_transitive_adapter(curr_type, false)) {
    transitive_type = curr_type;
  }

  if (!adapter_annotation && !transitive_type) {
    return "";
  }

  std::string direct_name;
  bool is_generic = false;
  if (adapter_annotation != nullptr) {
    direct_name = resolve_adapter_annotation_name(adapter_annotation, options);
    auto raw_name = get_annotation_property_string(adapter_annotation, "name");
    is_generic = raw_name.ends_with("<>");
  }

  bool layered = adapter_annotation != nullptr && transitive_type != nullptr;

  std::string result;

  if (layered) {
    result += "::fbthrift::adapter::LayeredThriftAdapter<";
  }

  if (!direct_name.empty()) {
    result += direct_name;
  }

  if (is_generic) {
    result += "<" + compute_type_name(type, options) + ">";
  }

  if (layered) {
    result += ", ";
  }

  if (transitive_type) {
    result += compute_transitive_adapter_name(transitive_type, options, field);
  }

  if (layered) {
    result += ">";
  }

  return result;
}

// Compute the transitive adapter name for a type.
// This handles typedef, struct, and container (list/set/map) cases.
std::string compute_transitive_adapter_name(
    const t_type* type,
    const rust_codegen_options& options,
    const t_field* field) {
  if (type->is<t_typedef>()) {
    return get_types_import_name(type->program(), options) +
        "::adapters::" + type_rust_name(type);
  }

  if (type->is<t_structured>()) {
    auto true_type = type->get_true_type();
    return get_types_import_name(true_type->program(), options) +
        "::adapters::" + type_rust_name(true_type);
  }

  if (const auto* list_type = type->try_as<t_list>()) {
    auto elem = list_type->elem_type().get_type();
    std::string wrapper;
    if (field && has_type_annotation(field)) {
      wrapper = "::" + get_type_annotation(field);
    } else {
      wrapper = "::fbthrift::adapter::ListMapAdapter";
    }
    // Recurse into elem for its adapter name
    std::string elem_adapter =
        compute_adapter_name(nullptr, elem, false, options, nullptr);
    return wrapper + "<" + elem_adapter + ">";
  }

  if (const auto* set_type = type->try_as<t_set>()) {
    auto elem = set_type->elem_type().get_type();
    std::string wrapper;
    if (field && has_type_annotation(field)) {
      wrapper = "::" + get_type_annotation(field);
    } else {
      wrapper = "::fbthrift::adapter::SetMapAdapter";
    }
    std::string elem_adapter =
        compute_adapter_name(nullptr, elem, false, options, nullptr);
    return wrapper + "<" + elem_adapter + ">";
  }

  if (const auto* map_type = type->try_as<t_map>()) {
    auto key_type = &map_type->key_type().deref();
    auto val_type = &map_type->val_type().deref();
    std::string wrapper;
    if (field && has_type_annotation(field)) {
      wrapper = "::" + get_type_annotation(field);
    } else {
      wrapper = "::fbthrift::adapter::MapMapAdapter";
    }
    std::string key_adapter;
    if (type_has_transitive_adapter(
            step_through_typedefs(key_type, true), false) ||
        node_has_adapter(*key_type)) {
      key_adapter =
          compute_adapter_name(nullptr, key_type, false, options, nullptr);
    } else {
      key_adapter = "::fbthrift::adapter::IdentityAdapter<" +
          compute_type_name(key_type, options) + ">";
    }
    std::string val_adapter;
    if (type_has_transitive_adapter(
            step_through_typedefs(val_type, true), false) ||
        node_has_adapter(*val_type)) {
      val_adapter =
          compute_adapter_name(nullptr, val_type, false, options, nullptr);
    } else {
      val_adapter = "::fbthrift::adapter::IdentityAdapter<" +
          compute_type_name(val_type, options) + ">";
    }
    return wrapper + "<" + key_adapter + ", " + val_adapter + ">";
  }

  return "";
}

// Compute the qualified adapter string:
// `<ADAPTER_NAME as ::fbthrift::adapter::ThriftAdapter>`
std::string compute_adapter_qualified(
    const t_const* adapter_annotation,
    const t_type* type,
    bool ignore_transitive_check,
    const rust_codegen_options& options,
    const t_field* field = nullptr) {
  std::string name = compute_adapter_name(
      adapter_annotation, type, ignore_transitive_check, options, field);
  if (name.empty()) {
    return "";
  }
  return "<" + name + " as ::fbthrift::adapter::ThriftAdapter>";
}

// Compute the struct-qualified adapter string:
// `<ADAPTER_NAME as ::fbthrift::adapter::ThriftAdapter>`
// Used in structimpl context for field-level adapter calls.
// The difference from compute_adapter_qualified is that this passes the field
// pointer, which affects how container adapter wrappers are chosen based on
// field type annotations.
std::string compute_adapter_struct_qualified(
    const t_const* adapter_annotation,
    const t_type* type,
    const rust_codegen_options& options,
    const t_field* field) {
  std::string name =
      compute_adapter_name(adapter_annotation, type, false, options, field);
  if (name.empty()) {
    return "";
  }
  return "<" + name + " as ::fbthrift::adapter::ThriftAdapter>";
}

std::string get_resolved_name(const t_type* t) {
  t = t->get_true_type();
  if (auto c = dynamic_cast<const t_list*>(t)) {
    return fmt::format(
        "list<{}>", get_resolved_name(c->elem_type().get_type()));
  }
  if (auto c = dynamic_cast<const t_set*>(t)) {
    return fmt::format("set<{}>", get_resolved_name(c->elem_type().get_type()));
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

struct rust_const_value_info {
  unsigned depth;
  const t_type* local_type;
  const t_type* underlying_type;
  // Pre-computed newtype typedef chain (outermost first).
  // Each entry is a newtype typedef that wraps the next layer.
  std::vector<const t_typedef*> newtype_chain;
};

class rust_generator_context {
 public:
  void register_visitors(t_whisker_generator::context_visitor& visitor) {
    visitor.add_const_visitor(
        [this](
            const t_whisker_generator::whisker_generator_visitor_context&,
            const t_const& node) {
          populate_value(node.value(), node.type(), 0);
        });
    visitor.add_field_visitor(
        [this](
            const t_whisker_generator::whisker_generator_visitor_context&,
            const t_field& node) {
          if (auto value = node.default_value()) {
            populate_value(value, node.type().get_type(), 2);
          }
        });
    visitor.add_structured_definition_visitor(
        [this](
            const t_whisker_generator::whisker_generator_visitor_context&,
            const t_structured& node) {
          for (const t_const& ann : node.structured_annotations()) {
            populate_value(ann.value(), ann.type(), 1);
          }
        });
    // Field annotations are visited separately since the field visitor
    // doesn't cover annotations.
    visitor.add_field_visitor(
        [this](
            const t_whisker_generator::whisker_generator_visitor_context&,
            const t_field& node) {
          for (const t_const& ann : node.structured_annotations()) {
            populate_value(ann.value(), ann.type(), 3);
          }
        });
  }

  const rust_const_value_info* get_info(const t_const_value* v) const {
    auto it = info_map_.find(v);
    return it == info_map_.end() ? nullptr : &it->second;
  }

 private:
  std::unordered_map<const t_const_value*, rust_const_value_info> info_map_;

  void populate_value(
      const t_const_value* value, const t_type* type, unsigned depth) {
    if (!value || info_map_.count(value)) {
      return;
    }

    const t_type* underlying = type->get_true_type();

    // Build the newtype chain: walk through typedefs that are newtypes
    // from the original type to the underlying type.
    std::vector<const t_typedef*> chain;
    {
      const t_type* cursor = type;
      while (cursor != underlying) {
        if (const auto* td = cursor->try_as<t_typedef>()) {
          if (has_newtype_annotation(td)) {
            chain.push_back(td);
          }
          cursor = &td->type().deref();
        } else {
          break;
        }
      }
    }

    info_map_[value] = rust_const_value_info{
        depth,
        type,
        underlying,
        std::move(chain),
    };

    // Recurse into children
    if (underlying->is<t_list>() || underlying->is<t_set>()) {
      const t_type* elem_type = nullptr;
      if (const auto* list_type = underlying->try_as<t_list>()) {
        elem_type = list_type->elem_type().get_type();
      } else if (const auto* set_type = underlying->try_as<t_set>()) {
        elem_type = set_type->elem_type().get_type();
      }
      if (elem_type) {
        for (auto elem : value->get_list()) {
          populate_value(elem, elem_type, depth + 3);
        }
      }
    } else if (const auto* map_type = underlying->try_as<t_map>()) {
      auto key_type = &map_type->key_type().deref();
      auto val_type = &map_type->val_type().deref();
      for (auto entry : value->get_map()) {
        populate_value(entry.first, key_type, depth + 3);
        populate_value(entry.second, val_type, depth + 3);
      }
    } else if (const auto* struct_type = underlying->try_as<t_structured>()) {
      if (underlying->is<t_union>()) {
        // Union: only one field is set
        if (!value->get_map().empty() &&
            value->get_map().at(0).first->kind() ==
                t_const_value::t_const_value_kind::CV_STRING) {
          const auto& entry = value->get_map().at(0);
          const auto& variant = entry.first->get_string();
          for (auto&& field : struct_type->fields()) {
            if (field.name() == variant) {
              populate_value(entry.second, field.type().get_type(), depth + 1);
              break;
            }
          }
        }
      } else {
        // Struct/exception: iterate all fields
        std::map<std::string, const t_const_value*> map_entries;
        for (auto entry : value->get_map()) {
          if (entry.first->kind() ==
              t_const_value::t_const_value_kind::CV_STRING) {
            map_entries[entry.first->get_string()] = entry.second;
          }
        }
        for (auto&& field : struct_type->fields()) {
          auto it = map_entries.find(field.name());
          if (it != map_entries.end() && it->second) {
            populate_value(it->second, field.type().get_type(), depth + 1);
          }
          // Also populate field defaults at depth+1
          if (auto default_val = field.default_value()) {
            populate_value(default_val, field.type().get_type(), depth + 1);
          }
        }
      }
    }
  }
};

class t_mstch_rust_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  std::string template_prefix() const override { return "rust"; }

  whisker::source_manager template_source_manager() const final {
    return whisker::source_manager{
        std::make_unique<in_memory_source_manager_backend>(
            create_templates_by_path())};
  }

  // t_whisker_generator defaults all strict; we relax for migration.
  strictness_options strictness() const override {
    strictness_options strict;
    strict.boolean_conditional = false;
    strict.printable_types = false;
    strict.undefined_variables = true;
    return strict;
  }

  void process_options(
      const std::map<std::string, std::string>& options) override;
  void generate_program() override;
  void generate_split_types();
  void fill_validator_visitors(ast_validator&) const override;

 protected:
  void initialize_context(context_visitor& visitor) override {
    rust_context_ = std::make_unique<rust_generator_context>();
    rust_context_->register_visitors(visitor);
    visitor.add_interface_visitor(
        [this](
            const whisker_generator_visitor_context& ctx,
            const t_interface& node) {
          // Only track interaction containers from the root program
          // being generated. Interactions shared across programs need
          // different parent_service_name depending on the program context.
          if (&ctx.program() != program_) {
            return;
          }
          if (auto* service = node.try_as<t_service>();
              service && !service->is<t_interaction>()) {
            for (const auto& function : service->functions()) {
              if (const auto& interaction = function.interaction()) {
                interaction_containers_[&interaction->as<t_interaction>()] =
                    service;
              }
            }
          }
        });
  }

 private:
  rust_codegen_options options_;
  std::unique_ptr<rust_generator_context> rust_context_;
  std::map<const t_interaction*, const t_service*> interaction_containers_;

  // Split mode state: per-render split_id set in generate_split_types().
  // Mutable because Whisker property lambdas capture `this` as const
  // (make_prototype_for_program is const override), but these are runtime
  // state set before each render call, not part of the prototype's logical
  // const-ness.
  mutable int current_split_id_ = 0;
  mutable std::map<int, std::vector<t_structured*>> struct_split_assignments_;
  mutable std::map<int, std::vector<t_typedef*>> typedef_split_assignments_;
  mutable std::map<int, std::vector<t_enum*>> enum_split_assignments_;

  void initialize_type_splits() {
    std::set<const t_named*> all_types;
    std::set<const t_named*> dependent_types;
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

    // Helper to check if a type is defined within the current crate
    auto generate_reference_set = [&all_types, &dependent_types](
                                      auto& self, const t_type* type) -> bool {
      if (!type) {
        return false;
      }
      bool dependent = false;
      if (all_types.count(type)) {
        dependent_types.insert(type);
        dependent = true;
      } else if (const t_list* list_type = dynamic_cast<const t_list*>(type)) {
        dependent = self(self, list_type->elem_type().get_type());
      } else if (const t_set* set_type = dynamic_cast<const t_set*>(type)) {
        dependent = self(self, set_type->elem_type().get_type());
      } else if (const t_map* map_type = dynamic_cast<const t_map*>(type)) {
        bool key = self(self, &map_type->key_type().deref());
        bool val = self(self, &map_type->val_type().deref());
        dependent = key || val;
      }
      return dependent;
    };

    // Identify dependencies within types
    for (t_typedef* typedf : program_->typedefs()) {
      if (generate_reference_set(
              generate_reference_set, &typedf->type().deref()) ||
          (typedef_has_constructor_expression(typedf))) {
        dependent_types.insert(typedf);
      }
    }
    for (t_structured* strct : program_->structured_definitions()) {
      for (const t_field& field : strct->fields()) {
        if (generate_reference_set(
                generate_reference_set, field.type().get_type())) {
          dependent_types.insert(strct);
        }
      }
    }

    // Generate split assignments
    auto next = [counter = 0, this]() mutable {
      return ((counter++) % options_.types_split_count) + 1;
    };
    for (t_typedef* typedf : program_->typedefs()) {
      if (dependent_types.count(typedf)) {
        typedef_split_assignments_[0].emplace_back(typedf);
      } else {
        typedef_split_assignments_[next()].emplace_back(typedf);
      }
    }
    for (t_structured* strct : program_->structured_definitions()) {
      if (dependent_types.count(strct)) {
        struct_split_assignments_[0].emplace_back(strct);
      } else {
        struct_split_assignments_[next()].emplace_back(strct);
      }
    }
    for (t_enum* enm : program_->enums()) {
      if (dependent_types.count(enm)) {
        enum_split_assignments_[0].emplace_back(enm);
      } else {
        enum_split_assignments_[next()].emplace_back(enm);
      }
    }
  }
  whisker::map::raw globals(prototype_database& proto) const override {
    whisker::map::raw globals = t_whisker_generator::globals(proto);
    // These variables are accessed inside {{> partial}} includes where the
    // enclosing scope's prototype chain may not include the typedef or
    // function handle. Setting them to null at the global level prevents
    // undefined variable errors when templates are rendered outside their
    // natural iteration context (e.g., in partials shared across contexts).
    globals.insert({"typedef:newtype?", whisker::make::null});
    globals.insert({"function:name", whisker::make::null});
    globals["rust_annotation_name"] = whisker::dsl::make_function(
        "rust_annotation_name",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(
              boost::algorithm::replace_all_copy(
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

  // Override interface prototype to add shared properties used by both
  // services and interactions. Both h_service and h_interaction extend
  // h_interface in their prototype chains, so properties defined here
  // are inherited by both.
  prototype<t_interface>::ptr make_prototype_for_interface(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interface(proto);
    auto def = whisker::dsl::prototype_builder<h_interface>::extends(base);
    def.property("client_package", [this](const t_interface& self) {
      return get_client_import_name(self.program(), options_);
    });
    def.property("server_package", [this](const t_interface& self) {
      return get_server_import_name(self.program(), options_);
    });
    def.property("mock_package", [this](const t_interface& self) {
      return get_mock_import_name(self.program(), options_);
    });
    def.property("mock_crate", [this](const t_interface& self) {
      return get_mock_crate(self.program(), options_);
    });
    def.property("snake", [](const t_interface& self) {
      if (const t_const* annot_mod =
              self.find_structured_annotation_or_null(kRustModUri)) {
        return get_annotation_property_string(annot_mod, "name");
      } else if (
          const t_const* annot_name =
              self.find_structured_annotation_or_null(kRustNameUri)) {
        return snakecase(get_annotation_property_string(annot_name, "name"));
      } else {
        return mangle_type(snakecase(self.name()));
      }
    });
    def.property("requestContext?", [](const t_interface& self) {
      return self.has_structured_annotation(kRustRequestContextUri);
    });
    def.property("rust_exceptions", [&proto](const t_interface& self) {
      struct name_less {
        bool operator()(const t_type* lhs, const t_type* rhs) const {
          return lhs->get_scoped_name() < rhs->get_scoped_name();
        }
      };
      enum exception_source_enum {
        FUNCTION = 1,
        SINK = 2,
        STREAM = 3,
      };
      struct exception_source {
        exception_source_enum source_enum;
        const t_function* function;
        const t_field* field;
      };
      using sources_t = std::vector<exception_source>;
      using source_map_t = std::map<const t_type*, sources_t, name_less>;

      source_map_t source_map;

      for (const auto& fun : self.functions()) {
        for (const t_field& fld : get_elems(fun.exceptions())) {
          source_map[&fld.type().deref()].push_back(
              exception_source{
                  .source_enum = FUNCTION, .function = &fun, .field = &fld});
        }
        if (fun.stream()) {
          for (const t_field& fld : get_elems(fun.stream()->exceptions())) {
            source_map[&fld.type().deref()].push_back(
                exception_source{
                    .source_enum = STREAM, .function = &fun, .field = &fld});
          }
        }
        if (fun.sink()) {
          for (const t_field& fld : get_elems(fun.sink()->sink_exceptions())) {
            source_map[&fld.type().deref()].push_back(
                exception_source{
                    .source_enum = SINK, .function = &fun, .field = &fld});
          }
        }
      }

      whisker::array::raw output;
      for (const auto& sources : source_map) {
        whisker::array::raw function_data;
        whisker::array::raw stream_data;
        whisker::array::raw sink_data;
        for (const auto& source : sources.second) {
          whisker::map::raw inner;
          inner["rust_exception_function:function"] =
              whisker::object(proto.create<t_function>(*source.function));
          inner["rust_exception_function:field"] =
              whisker::object(proto.create<t_field>(*source.field));
          auto entry = whisker::make::map(std::move(inner));
          if (source.source_enum == FUNCTION) {
            function_data.emplace_back(std::move(entry));
          } else if (source.source_enum == STREAM) {
            stream_data.emplace_back(std::move(entry));
          } else if (source.source_enum == SINK) {
            sink_data.emplace_back(std::move(entry));
          }
        }

        whisker::map::raw data;
        data["rust_exception:type"] =
            resolve_derived_t_type(proto, *sources.first);
        data["rust_exception:functions"] =
            whisker::make::array(std::move(function_data));
        data["rust_exception:streams"] =
            whisker::make::array(std::move(stream_data));
        data["rust_exception:sinks"] =
            whisker::make::array(std::move(sink_data));
        output.emplace_back(whisker::make::map(std::move(data)));
      }

      return whisker::make::array(std::move(output));
    });
    return std::move(def).make();
  }

  // Override interaction prototype to add interaction-specific properties
  // and a "service" qualifier level so that service:* lookups resolve
  // correctly on interactions. Most shared properties (client_package,
  // server_package, mock_package, mock_crate, snake, requestContext?,
  // rust_exceptions) are inherited from make_prototype_for_interface.
  //
  // The templates (lib/service.mustache) use `service:` qualified lookups
  // (e.g., service:rust_name, service:snake) on both services and
  // interactions. For services, the "service" qualifier is on h_service.
  // For interactions, we need to insert a prototype level with qualifier
  // "service" in the chain so these lookups resolve correctly.
  //
  // Chain: h_interaction("interaction") [0 own props]
  //          → basic_prototype("service") [0 own props, just qualifier]
  //            → prototype("interaction") [interaction-specific properties]
  //              → base interaction [0 props]
  //                → h_interface("") [shared props] → h_type("type") →
  //                h_named("") → ...
  prototype<t_interaction>::ptr make_prototype_for_interaction(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interaction(proto);
    // Build our custom service properties on an "interaction" qualified
    // prototype (the builder forces qualifier from the handle type).
    auto inner_def =
        whisker::dsl::prototype_builder<h_interaction>::extends(base);
    inner_def.property("extends?", [](const t_interaction&) {
      // Interactions don't extend other services
      return false;
    });
    inner_def.property("extends", [](const t_interaction&) {
      // Interactions don't extend other services
      return whisker::make::null;
    });
    inner_def.property("interactions?", [](const t_interaction&) {
      // Interactions don't have sub-interactions
      return false;
    });
    inner_def.property("interactions", [](const t_interaction&) {
      // Interactions don't have sub-interactions
      return whisker::make::array(whisker::array::raw{});
    });
    inner_def.property(
        "has_bidirectional_streams?", [](const t_interaction& self) {
          return std::any_of(
              self.functions().begin(),
              self.functions().end(),
              [](const t_function& function) {
                return function.is_bidirectional_stream();
              });
        });
    inner_def.property(
        "parent_service_name", [this](const t_interaction& self) {
          auto it = interaction_containers_.find(&self);
          if (it != interaction_containers_.end()) {
            return std::string(it->second->name());
          }
          return std::string(self.name());
        });
    inner_def.property("extendedClients", [](const t_interaction&) {
      // Interactions don't have extended clients
      return whisker::make::array(whisker::array::raw{});
    });
    auto inner_proto = std::move(inner_def).make();

    // Wrap with a "service" qualified prototype level so that
    // service:* lookups (e.g., service:rust_name, service:snake)
    // resolve correctly on interactions. The "service" qualifier
    // matches, then properties are inherited from the inner prototype
    // via the parent chain with empty qualifier.
    static const std::string_view service_qualifier = "service";
    whisker::prototype_ptr<t_interaction> service_alias =
        std::make_shared<whisker::basic_prototype<t_interaction>>(
            whisker::prototype<>::descriptors_map{},
            std::move(inner_proto),
            service_qualifier);

    // Final h_interaction prototype with qualifier "interaction" and
    // no own properties — it inherits everything from the chain.
    auto def = whisker::dsl::prototype_builder<h_interaction>::extends(
        std::move(service_alias));
    return std::move(def).make();
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
      return to_array(variants, proto.of<t_enum_value>());
    });
    def.property("variants_by_number", [&proto](const t_enum& self) {
      std::vector<const t_enum_value*> variants = self.values().copy();
      std::sort(variants.begin(), variants.end(), [](auto a, auto b) {
        return a->get_value() < b->get_value();
      });
      return to_array(variants, proto.of<t_enum_value>());
    });
    def.property("derive", [this](const t_enum& self) {
      return compute_derive_string(self, options_);
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

  prototype<t_const>::ptr make_prototype_for_const(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const(proto);
    auto def = whisker::dsl::prototype_builder<h_const>::extends(base);
    def.property("lazy?", [](const t_const& self) {
      if (type_has_transitive_adapter(self.type(), true)) {
        return true;
      }
      auto type = self.type()->get_true_type();
      return type->is<t_container>() || type->is<t_structured>();
    });
    return std::move(def).make();
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    using value_type = t_const_value::t_const_value_kind;
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    def.property(
        "underlying_type",
        [this, &proto](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::null;
          }
          return resolve_derived_t_type(proto, *info->underlying_type);
        });

    def.property(
        "local_type",
        [this, &proto](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::null;
          }
          return resolve_derived_t_type(proto, *info->local_type);
        });

    def.property("newtype?", [this](const t_const_value& self) {
      auto* info = rust_context_->get_info(&self);
      if (!info) {
        return false;
      }
      return !info->newtype_chain.empty();
    });

    def.property(
        "newtype_chain",
        [this, &proto](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::null;
          }
          whisker::array::raw chain;
          for (auto* td : info->newtype_chain) {
            chain.emplace_back(resolve_derived_t_type(proto, *td));
          }
          return whisker::make::array(std::move(chain));
        });

    def.property(
        "newtype_chain_reversed",
        [this, &proto](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::null;
          }
          whisker::array::raw chain;
          for (auto it = info->newtype_chain.rbegin();
               it != info->newtype_chain.rend();
               ++it) {
            chain.emplace_back(resolve_derived_t_type(proto, **it));
          }
          return whisker::make::array(std::move(chain));
        });

    def.property(
        "bool_value", [](const t_const_value& self) -> whisker::object {
          if (self.kind() == value_type::CV_INTEGER) {
            return whisker::make::boolean(self.get_integer() != 0);
          }
          return whisker::make::boolean(self.get_bool());
        });

    def.property("integer_value", [](const t_const_value& self) {
      return whisker::make::string(std::to_string(self.get_integer()));
    });

    def.property("floatingPoint?", [](const t_const_value& self) {
      return !self.type().empty() &&
          self.type()->get_true_type()->is_floating_point();
    });

    def.property("floatingPointValue", [](const t_const_value& self) {
      auto str = fmt::format(
          "{}",
          self.kind() == value_type::CV_INTEGER
              ? static_cast<double>(self.get_integer())
              : self.get_double());
      if (str.find('.') == std::string::npos &&
          str.find('e') == std::string::npos &&
          str.find('E') == std::string::npos) {
        str += ".0";
      }
      return whisker::make::string(std::move(str));
    });

    def.property("quoted", [](const t_const_value& self) {
      return whisker::make::string(quote(self.get_string(), false));
    });

    def.property(
        "structFields",
        [this, &proto](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::null;
          }
          auto struct_type =
              dynamic_cast<const t_structured*>(info->underlying_type);
          if (!struct_type) {
            return whisker::make::null;
          }
          std::map<std::string, const t_const_value*> map_entries;
          for (auto entry : self.get_map()) {
            auto key = entry.first;
            if (key->kind() == value_type::CV_STRING) {
              map_entries[key->get_string()] = entry.second;
            }
          }
          whisker::array::raw fields;
          for (auto&& field : struct_type->fields()) {
            auto explicit_value = map_entries[field.name()];
            whisker::object explicit_value_obj = whisker::make::null;
            if (explicit_value) {
              explicit_value_obj = whisker::make::native_handle(
                  proto.create<t_const_value>(*explicit_value));
            }
            whisker::object default_value_obj = whisker::make::null;
            if (auto default_val = field.default_value()) {
              default_value_obj = whisker::make::native_handle(
                  proto.create<t_const_value>(*default_val));
            }
            auto adapter_annotation = find_structured_adapter_annotation(field);
            fields.emplace_back(
                whisker::make::map(
                    whisker::map::raw{
                        {"field:rust_name",
                         whisker::make::string(named_rust_name(&field))},
                        {"field:id", whisker::make::i64(field.id())},
                        {"field:optional?",
                         whisker::make::boolean(
                             field.qualifier() == t_field_qualifier::optional)},
                        {"field:box?",
                         whisker::make::boolean(
                             field_kind(field) == FieldKind::Box)},
                        {"field:arc?",
                         whisker::make::boolean(
                             field_kind(field) == FieldKind::Arc)},
                        {"field:has_adapter?",
                         whisker::make::boolean(
                             adapter_annotation != nullptr ||
                             type_has_transitive_adapter(
                                 step_through_typedefs(
                                     field.type().get_type(), true),
                                 false))},
                        {"field:adapter_qualified",
                         whisker::make::string(compute_adapter_qualified(
                             adapter_annotation,
                             field.type().get_type(),
                             false,
                             options_))},
                        {"field:docs?",
                         whisker::make::boolean(field.has_doc())},
                        {"field:explicit_value", std::move(explicit_value_obj)},
                        {"field:default_value", std::move(default_value_obj)},
                    }));
          }
          return whisker::make::array(std::move(fields));
        });

    def.property("exhaustive?", [this](const t_const_value& self) {
      auto* info = rust_context_->get_info(&self);
      if (!info) {
        return false;
      }
      auto struct_type =
          dynamic_cast<const t_structured*>(info->underlying_type);
      return struct_type &&
          struct_type->has_structured_annotation(kRustExhaustiveUri);
    });

    def.property(
        "unionVariant", [this](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::null;
          }
          auto struct_type =
              dynamic_cast<const t_structured*>(info->underlying_type);
          if (!struct_type || self.get_map().empty()) {
            return whisker::make::null;
          }
          const auto& entry = self.get_map().at(0);
          const auto& variant = entry.first->get_string();
          for (auto&& field : struct_type->fields()) {
            if (field.name() == variant) {
              if (const t_const* annot =
                      field.find_structured_annotation_or_null(kRustNameUri)) {
                return whisker::make::string(
                    get_annotation_property_string(annot, "name"));
              } else {
                return whisker::make::string(std::string(variant));
              }
            }
          }
          return whisker::make::null;
        });

    def.property(
        "unionValue",
        [this, &proto](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::null;
          }
          auto struct_type =
              dynamic_cast<const t_structured*>(info->underlying_type);
          if (!struct_type) {
            return whisker::make::null;
          }
          const auto& entry = self.get_map().at(0);
          const auto& variant = entry.first->get_string();
          const auto* content = entry.second;
          for (auto&& field : struct_type->fields()) {
            if (field.name() == variant) {
              return whisker::make::native_handle(
                  proto.create<t_const_value>(*content));
            }
          }
          return whisker::make::null;
        });

    def.property(
        "enumVariant", [](const t_const_value& self) -> whisker::object {
          if (self.is_enum()) {
            auto enum_value = self.get_enum_value();
            if (enum_value) {
              return whisker::make::string(mangle(enum_value->name()));
            }
          }
          return whisker::make::null;
        });

    def.property("empty?", [](const t_const_value& self) {
      auto kind = self.kind();
      if (kind == value_type::CV_LIST) {
        return self.get_list().empty();
      }
      if (kind == value_type::CV_MAP) {
        return self.get_map().empty();
      }
      if (kind == value_type::CV_STRING) {
        return self.get_string().empty();
      }
      return false;
    });

    def.property("indent", [this](const t_const_value& self) {
      auto* info = rust_context_->get_info(&self);
      if (!info) {
        return whisker::make::string(std::string());
      }
      return whisker::make::string(std::string(4 * info->depth, ' '));
    });

    def.property(
        "simpleLiteral?", [this](const t_const_value& self) -> whisker::object {
          auto* info = rust_context_->get_info(&self);
          if (!info) {
            return whisker::make::boolean(false);
          }
          // Primitives have simple literals
          if (info->underlying_type->is_bool() ||
              info->underlying_type->is_byte() ||
              info->underlying_type->is_any_int() ||
              info->underlying_type->is_floating_point()) {
            return whisker::make::boolean(true);
          }
          // Enum variants as well
          if (info->underlying_type->is<t_enum>()) {
            if (self.is_enum()) {
              auto enum_value = self.get_enum_value();
              if (enum_value) {
                return whisker::make::string(mangle(enum_value->name()));
              }
            }
            return whisker::make::boolean(false);
          }
          return whisker::make::boolean(false);
        });

    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def =
        whisker::dsl::prototype_builder<h_type>::extends(std::move(base));
    def.property(
        "rust_name", [](const t_type& self) { return type_rust_name(&self); });
    def.property("rust_name_snake", [](const t_type& self) {
      return snakecase(mangle_type(self.name()));
    });
    def.property("package", [this](const t_type& self) {
      return get_types_import_name(self.program(), options_);
    });
    def.property("rust", [](const t_type& self) {
      auto rust_type = get_type_annotation(&self);
      if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
        rust_type = fmt::format("fbthrift::builtin_types::{}", rust_type);
      }
      return rust_type;
    });
    def.property("type_annotation?", [](const t_type& self) {
      return has_type_annotation(&self) && !has_newtype_annotation(&self);
    });
    def.property("nonstandard?", [](const t_type& self) {
      return has_nonstandard_type_annotation(&self) &&
          !has_newtype_annotation(&self);
    });
    def.property("serde?", [this](const t_type& self) {
      return rust_serde_enabled(options_, self);
    });
    def.property("has_adapter?", [](const t_type& self) {
      const t_type* curr_type = step_through_typedefs(&self, true);
      return type_has_transitive_adapter(curr_type, false);
    });
    def.property("adapter_name", [this](const t_type& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      if (auto* td = self.try_as<t_typedef>()) {
        return compute_adapter_name(
            adapter_annotation, &td->type().deref(), false, options_);
      }
      bool ignore_transitive = self.is<t_structured>();
      return compute_adapter_name(
          adapter_annotation, &self, ignore_transitive, options_);
    });
    def.property("adapter_qualified", [this](const t_type& self) {
      return compute_adapter_qualified(nullptr, &self, false, options_);
    });
    // Type sub-object accessors (migrated from mstch_type)
    def.property("typedef", [&proto](const t_type& self) -> whisker::object {
      if (auto* td = self.try_as<t_typedef>()) {
        return resolve_derived_t_type(proto, *td);
      }
      return whisker::make::null;
    });
    // These accessors follow typedefs (get_true_type) to match the mstch
    // behavior of mstch_type, which stores resolved_type_ = get_true_type().
    def.property(
        "list_elem_type", [&proto](const t_type& self) -> whisker::object {
          if (auto* list = self.get_true_type()->try_as<t_list>()) {
            return resolve_derived_t_type(proto, list->elem_type().deref());
          }
          return whisker::make::null;
        });
    def.property(
        "set_elem_type", [&proto](const t_type& self) -> whisker::object {
          if (auto* set = self.get_true_type()->try_as<t_set>()) {
            return resolve_derived_t_type(proto, set->elem_type().deref());
          }
          return whisker::make::null;
        });
    def.property("key_type", [&proto](const t_type& self) -> whisker::object {
      if (auto* map = self.get_true_type()->try_as<t_map>()) {
        return resolve_derived_t_type(proto, map->key_type().deref());
      }
      return whisker::make::null;
    });
    def.property("value_type", [&proto](const t_type& self) -> whisker::object {
      if (auto* map = self.get_true_type()->try_as<t_map>()) {
        return resolve_derived_t_type(proto, map->val_type().deref());
      }
      return whisker::make::null;
    });
    return std::move(def).make();
  }

  prototype<t_typedef>::ptr make_prototype_for_typedef(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_typedef(proto);
    auto def = whisker::dsl::prototype_builder<h_typedef>::extends(base);
    def.property("newtype?", [](const t_typedef& self) {
      return has_newtype_annotation(&self);
    });
    def.property("ord?", [](const t_typedef& self) {
      return self.has_structured_annotation(kRustOrdUri) ||
          (can_derive_ord(&self) &&
           !type_has_transitive_adapter(&self.type().deref(), true));
    });
    def.property("copy?", [](const t_typedef& self) {
      if (!type_has_transitive_adapter(&self.type().deref(), true)) {
        auto inner = self.get_true_type();
        if (inner->is_bool() || inner->is_byte() || inner->is_i16() ||
            inner->is_i32() || inner->is_i64() || inner->is<t_enum>() ||
            inner->is_void()) {
          return true;
        }
      }
      return false;
    });
    def.property("rust_type", [](const t_typedef& self) {
      // See 'typedef.mustache'. The context is writing a newtype: e.g. `pub
      // struct T(pub X)`. If `X` has a `rust.Type` annotation `A` we should
      // write `struct T(pub A)` If it does not, we should write `pub struct T
      // (pub X)`.
      std::string rust_type;
      if (const t_const* annot =
              self.find_structured_annotation_or_null(kRustTypeUri)) {
        rust_type = get_annotation_property_string(annot, "name");
      }
      if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
        rust_type = std::string("fbthrift::builtin_types::") + rust_type;
      }
      return rust_type;
    });
    def.property("nonstandard?", [](const t_typedef& self) {
      // See 'typedef.mustache'. The context is writing serialization functions
      // for a newtype `pub struct T(pub X)`.
      // If `X` has a type annotation `A` that is non-standard we should emit
      // the phrase `crate::r#impl::write(&self.0, p)`. If `X` does not have an
      // annotation or does but it is not non-standard we should write
      // `self.0.write(p)`.
      std::string rust_type;
      if (const t_const* annot =
              self.find_structured_annotation_or_null(kRustTypeUri)) {
        rust_type = get_annotation_property_string(annot, "name");
      }
      return rust_type.find("::") != std::string::npos;
    });
    def.property("constructor?", [](const t_typedef& self) {
      return typedef_has_constructor_expression(&self);
    });
    def.property("has_adapter?", [](const t_typedef& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      auto type = &self.type().deref();
      const t_type* curr_type = step_through_typedefs(type, true);
      return adapter_annotation != nullptr ||
          type_has_transitive_adapter(curr_type, false);
    });
    def.property("adapter_name", [this](const t_typedef& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      return compute_adapter_name(
          adapter_annotation, &self.type().deref(), false, options_);
    });
    def.property("adapter_qualified", [this](const t_typedef& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      return compute_adapter_qualified(
          adapter_annotation, &self.type().deref(), false, options_);
    });
    def.property("adapter_transitive_only?", [](const t_typedef& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      if (adapter_annotation != nullptr) {
        return false;
      }
      auto type = &self.type().deref();
      const t_type* curr_type = step_through_typedefs(type, true);
      return type_has_transitive_adapter(curr_type, false);
    });
    // mstch "type" = resolved (context-switch to resolved type).
    // Use proto.create<t_type> instead of resolve_derived_t_type to avoid
    // the resolved type having a "typedef" qualifier, which would shadow
    // the outer typedef's properties (e.g., typedef:rust_name) in templates.
    def.property("type", [&proto](const t_typedef& self) {
      return whisker::object(proto.create<t_type>(self.type().deref()));
    });
    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def = whisker::dsl::prototype_builder<h_field>::extends(base);
    def.property("primitive?", [](const t_field& self) {
      auto type = self.type().get_type();
      return type->is_bool() || type->is_any_int() || type->is_floating_point();
    });
    def.property("rename?", [](const t_field& self) {
      return self.name() != mangle(self.name());
    });
    def.property("box?", [](const t_field& self) {
      return field_kind(self) == FieldKind::Box;
    });
    def.property("arc?", [](const t_field& self) {
      return field_kind(self) == FieldKind::Arc;
    });
    def.property("type_annotation?", [](const t_field& self) {
      return has_type_annotation(&self);
    });
    def.property("type_nonstandard?", [](const t_field& self) {
      return has_nonstandard_type_annotation(&self);
    });
    def.property("type_rust", [](const t_field& self) {
      auto rust_type = get_type_annotation(&self);
      if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
        rust_type = std::string("fbthrift::builtin_types::") + rust_type;
      }
      return rust_type;
    });
    def.property("has_adapter?", [](const t_field& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      auto type = self.type().get_type();
      const t_type* curr_type = step_through_typedefs(type, true);
      return adapter_annotation != nullptr ||
          type_has_transitive_adapter(curr_type, false);
    });
    def.property("adapter_name", [this](const t_field& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      return compute_adapter_name(
          adapter_annotation, self.type().get_type(), false, options_);
    });
    def.property("adapter_qualified", [this](const t_field& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      return compute_adapter_qualified(
          adapter_annotation, self.type().get_type(), false, options_);
    });
    def.property("adapter_struct_qualified", [this](const t_field& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      return compute_adapter_struct_qualified(
          adapter_annotation, self.type().get_type(), options_, &self);
    });
    def.property("rust_structured_annotations", [&proto](const t_field& self) {
      whisker::array::raw annotations;
      for (const t_const& ann : self.structured_annotations()) {
        whisker::object direct = whisker::make::native_handle(
            proto.create<t_const_value>(*ann.value()));
        whisker::object transitive = whisker::make::null;
        if (ann.type()->has_structured_annotation(kTransitiveUri)) {
          transitive = resolve_derived_t_type(proto, *ann.type());
        }
        annotations.emplace_back(
            whisker::make::map(
                whisker::map::raw{
                    {"structured_annotation:direct", std::move(direct)},
                    {"structured_annotation:transitive?",
                     std::move(transitive)},
                }));
      }
      return whisker::make::array(std::move(annotations));
    });
    return std::move(def).make();
  }

  prototype<t_structured>::ptr make_prototype_for_structured(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_structured(proto);
    auto def = whisker::dsl::prototype_builder<h_structured>::extends(base);
    def.property("ord?", [](const t_structured& self) {
      if (self.has_structured_annotation(kRustOrdUri)) {
        return true;
      }
      for (const auto& field : self.fields()) {
        if (!can_derive_ord(field.type().get_type())) {
          return false;
        }
        // Assume we cannot derive `Ord` on the adapted type.
        if (node_has_adapter(field) ||
            type_has_transitive_adapter(field.type().get_type(), true)) {
          return false;
        }
        if (field.has_structured_annotation(kRustTypeUri)) {
          return false;
        }
      }
      return true;
    });
    def.property("copy?", [](const t_structured& self) {
      return self.has_structured_annotation(kRustCopyUri);
    });
    def.property("exhaustive?", [](const t_structured& self) {
      return self.has_structured_annotation(kRustExhaustiveUri);
    });
    def.property("derive", [this](const t_structured& self) {
      return compute_derive_string(self, options_);
    });
    def.property("has_exception_message?", [](const t_structured& self) {
      if (const auto* exc = dynamic_cast<const t_exception*>(&self)) {
        return exc->get_message_field() != nullptr;
      }
      return false;
    });
    def.property(
        "is_exception_message_optional?", [](const t_structured& self) {
          if (const auto* exc = dynamic_cast<const t_exception*>(&self)) {
            if (const auto* message_field = exc->get_message_field()) {
              return message_field->qualifier() == t_field_qualifier::optional;
            }
          }
          return false;
        });
    def.property("exception_message", [](const t_structured& self) {
      if (const auto* exc = dynamic_cast<const t_exception*>(&self)) {
        if (const auto* message_field = exc->get_message_field()) {
          return message_field->name();
        }
      }
      return std::string();
    });
    def.property("all_optional?", [](const t_structured& self) {
      for (const auto& field : self.fields()) {
        if (field.qualifier() != t_field_qualifier::optional) {
          return false;
        }
      }
      return true;
    });
    def.property("has_adapter?", [](const t_structured& self) {
      return node_has_adapter(self);
    });
    def.property("adapter_name", [this](const t_structured& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      // Structs cannot have transitive types, so ignore transitive check.
      return compute_adapter_name(adapter_annotation, &self, true, options_);
    });
    def.property("adapter_qualified", [this](const t_structured& self) {
      auto adapter_annotation = find_structured_adapter_annotation(self);
      return compute_adapter_qualified(
          adapter_annotation, &self, true, options_);
    });
    def.property("fields_by_name", [&proto](const t_structured& self) {
      std::vector<const t_field*> fields = self.fields().copy();
      std::sort(fields.begin(), fields.end(), [](auto a, auto b) {
        return a->name() < b->name();
      });
      return to_array(fields, proto.of<t_field>());
    });
    def.property(
        "rust_structured_annotations", [&proto](const t_structured& self) {
          whisker::array::raw annotations;
          for (const t_const& ann : self.structured_annotations()) {
            whisker::object direct = whisker::make::native_handle(
                proto.create<t_const_value>(*ann.value()));
            whisker::object transitive = whisker::make::null;
            if (ann.type()->has_structured_annotation(kTransitiveUri)) {
              transitive = resolve_derived_t_type(proto, *ann.type());
            }
            annotations.emplace_back(
                whisker::make::map(
                    whisker::map::raw{
                        {"structured_annotation:direct", std::move(direct)},
                        {"structured_annotation:transitive?",
                         std::move(transitive)},
                    }));
          }
          return whisker::make::array(std::move(annotations));
        });
    return std::move(def).make();
  }

  prototype<t_service>::ptr make_prototype_for_service(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_service(proto);
    auto def = whisker::dsl::prototype_builder<h_service>::extends(base);
    def.property("client_package", [this](const t_service& self) {
      return get_client_import_name(self.program(), options_);
    });
    def.property("server_package", [this](const t_service& self) {
      return get_server_import_name(self.program(), options_);
    });
    def.property("mock_package", [this](const t_service& self) {
      return get_mock_import_name(self.program(), options_);
    });
    def.property("mock_crate", [this](const t_service& self) {
      return get_mock_crate(self.program(), options_);
    });
    def.property("snake", [](const t_service& self) {
      if (const t_const* annot_mod =
              self.find_structured_annotation_or_null(kRustModUri)) {
        return get_annotation_property_string(annot_mod, "name");
      } else if (
          const t_const* annot_name =
              self.find_structured_annotation_or_null(kRustNameUri)) {
        return snakecase(get_annotation_property_string(annot_name, "name"));
      } else {
        return mangle_type(snakecase(self.name()));
      }
    });
    def.property("requestContext?", [](const t_service& self) {
      return self.has_structured_annotation(kRustRequestContextUri);
    });
    def.property("extendedClients", [this, &proto](const t_service& self) {
      whisker::array::raw extended_services;
      const t_service* service = &self;
      std::string as_ref_impl = "&self.parent";
      while (const t_service* parent_service = service->extends()) {
        whisker::map::raw node;
        node["extendedService:packagePrefix"] = whisker::make::string(
            get_client_import_name(parent_service->program(), options_));
        node["extendedService:asRefImpl"] = whisker::make::string(as_ref_impl);
        node["extendedService:service"] =
            whisker::object(proto.create<t_service>(*parent_service));
        extended_services.emplace_back(whisker::make::map(std::move(node)));
        as_ref_impl = "self.parent.as_ref()";
        service = parent_service;
      }
      return whisker::make::array(std::move(extended_services));
    });
    def.property("rust_exceptions", [&proto](const t_service& self) {
      struct name_less {
        bool operator()(const t_type* lhs, const t_type* rhs) const {
          return lhs->get_scoped_name() < rhs->get_scoped_name();
        }
      };
      enum exception_source_enum {
        FUNCTION = 1,
        SINK = 2,
        STREAM = 3,
      };
      struct exception_source {
        exception_source_enum source_enum;
        const t_function* function;
        const t_field* field;
      };
      using sources_t = std::vector<exception_source>;
      using source_map_t = std::map<const t_type*, sources_t, name_less>;

      source_map_t source_map;

      for (const auto& fun : self.functions()) {
        for (const t_field& fld : get_elems(fun.exceptions())) {
          source_map[&fld.type().deref()].push_back(
              exception_source{
                  .source_enum = FUNCTION, .function = &fun, .field = &fld});
        }
        if (fun.stream()) {
          for (const t_field& fld : get_elems(fun.stream()->exceptions())) {
            source_map[&fld.type().deref()].push_back(
                exception_source{
                    .source_enum = STREAM, .function = &fun, .field = &fld});
          }
        }
        if (fun.sink()) {
          for (const t_field& fld : get_elems(fun.sink()->sink_exceptions())) {
            source_map[&fld.type().deref()].push_back(
                exception_source{
                    .source_enum = SINK, .function = &fun, .field = &fld});
          }
        }
      }

      whisker::array::raw output;
      for (const auto& sources : source_map) {
        whisker::array::raw function_data;
        whisker::array::raw stream_data;
        whisker::array::raw sink_data;
        for (const auto& source : sources.second) {
          whisker::map::raw inner;
          inner["rust_exception_function:function"] =
              whisker::object(proto.create<t_function>(*source.function));
          inner["rust_exception_function:field"] =
              whisker::object(proto.create<t_field>(*source.field));
          auto entry = whisker::make::map(std::move(inner));
          if (source.source_enum == FUNCTION) {
            function_data.emplace_back(std::move(entry));
          } else if (source.source_enum == STREAM) {
            stream_data.emplace_back(std::move(entry));
          } else if (source.source_enum == SINK) {
            sink_data.emplace_back(std::move(entry));
          }
        }

        whisker::map::raw data;
        data["rust_exception:type"] =
            resolve_derived_t_type(proto, *sources.first);
        data["rust_exception:functions"] =
            whisker::make::array(std::move(function_data));
        data["rust_exception:streams"] =
            whisker::make::array(std::move(stream_data));
        data["rust_exception:sinks"] =
            whisker::make::array(std::move(sink_data));
        output.emplace_back(whisker::make::map(std::move(data)));
      }

      return whisker::make::array(std::move(output));
    });
    def.property("extends?", [](const t_service& self) {
      return self.extends() != nullptr;
    });
    def.property("interactions?", [](const t_service& self) {
      for (const auto& function : self.functions()) {
        if (function.interaction()) {
          return true;
        }
      }
      return false;
    });
    // parent_service_name: own name for services (interactions have their
    // own override in make_prototype_for_interaction)
    def.property("parent_service_name", [](const t_service& self) {
      return std::string(self.name());
    });
    // Override base service:interactions to wrap each interaction with a
    // per-service parent_service_name. This is needed because the same
    // interaction can be performed by multiple services, and the template
    // uses service:parent_service_name inside {{#service:interactions}} to
    // get the ENCLOSING service's name, not the interaction's global owner.
    def.property("interactions", [&proto](const t_service& self) {
      std::vector<const t_interaction*> interactions;
      interactions.reserve(self.functions().size());
      for (const auto& function : self.functions()) {
        if (const auto& interaction = function.interaction()) {
          auto* ptr = &interaction->as<t_interaction>();
          if (std::find(interactions.begin(), interactions.end(), ptr) !=
              interactions.end()) {
            continue;
          }
          interactions.push_back(ptr);
        }
      }

      // Create per-service interaction handles with a custom prototype
      // that overrides parent_service_name to return this service's name.
      std::string service_name(self.name());
      auto base_interaction_proto = proto.of<t_interaction>();

      // Create an override prototype with parent_service_name
      whisker::prototype<>::descriptors_map overrides;
      overrides.emplace(
          "parent_service_name",
          whisker::prototype<>::property(
              whisker::dsl::make_function(
                  [service_name](
                      whisker::dsl::function::context) -> whisker::object {
                    return whisker::make::string(service_name);
                  })));

      // Build chain: override("service") → base interaction proto → ...
      // The "service" qualifier ensures service:parent_service_name
      // finds the override before falling through to the inner prototype.
      static const std::string_view svc_qual = "service";
      auto override_proto =
          std::make_shared<whisker::basic_prototype<t_interaction>>(
              std::move(overrides), base_interaction_proto, svc_qual);

      whisker::array::raw result;
      result.reserve(interactions.size());
      for (const t_interaction* interaction : interactions) {
        result.emplace_back(
            whisker::object(
                whisker::native_handle<t_interaction>(
                    whisker::manage_as_static(*interaction), override_proto)));
      }
      return whisker::make::array(std::move(result));
    });
    return std::move(def).make();
  }

  prototype<t_function>::ptr make_prototype_for_function(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_function(proto);
    auto def = whisker::dsl::prototype_builder<h_function>::extends(base);
    def.property("interaction_name", [](const t_function& self) {
      const auto& interaction = self.interaction();
      return interaction ? interaction->name() : self.return_type()->name();
    });
    def.property("void_or_void_stream?", [](const t_function& self) {
      return self.has_void_initial_response();
    });
    def.property("returns_by_name", [](const t_function& self) {
      auto get_ttype = [](const t_type& type) -> const char* {
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
            default:
              break;
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
      std::vector<std::string> returns;
      for (const t_field& field : get_elems(self.exceptions())) {
        returns.push_back(
            fmt::format(
                "::fbthrift::Field::new(\"{}\", ::fbthrift::TType::{}, {})",
                field.name(),
                get_ttype(*field.type()),
                field.id()));
      }
      std::string type_name;
      if (self.stream()) {
        type_name = "Stream";
      } else if (self.sink()) {
        type_name = "Void";
      } else {
        type_name = get_ttype(*self.return_type());
      }
      returns.push_back(
          fmt::format(
              "::fbthrift::Field::new(\"Success\", ::fbthrift::TType::{}, 0)",
              type_name));
      std::sort(returns.begin(), returns.end());
      whisker::array::raw result;
      for (const std::string& ret : returns) {
        result.emplace_back(whisker::make::string(ret));
      }
      return whisker::make::array(std::move(result));
    });
    def.property("args_by_name", [&proto](const t_function& self) {
      std::vector<const t_field*> params = self.params().fields().copy();
      std::sort(params.begin(), params.end(), [](auto a, auto b) {
        return a->name() < b->name();
      });
      return to_array(params, proto.of<t_field>());
    });
    def.property(
        "enable_anyhow_to_application_exn", [this](const t_function& self) {
          // First look for annotation on the function.
          if (const t_const* annot =
                  find_structured_service_exn_annotation(self)) {
            for (const auto& item : annot->value()->get_map()) {
              if (item.first->get_string() == "anyhow_to_application_exn") {
                return get_annotation_property_bool(
                    annot, "anyhow_to_application_exn");
              }
            }
          }
          // If not present on function, look at service annotations.
          const t_interface* parent = context().get_function_parent(&self);
          if (parent) {
            if (const t_const* annot =
                    find_structured_service_exn_annotation(*parent)) {
              return get_annotation_property_bool(
                  annot, "anyhow_to_application_exn");
            }
          }
          return false;
        });
    def.property("upcamel", [this](const t_function& self) -> std::string {
      auto upcamel_name = camelcase(self.name());
      const t_interface* parent = context().get_function_parent(&self);
      // If a service contains a pair of methods that collide converted to
      // CamelCase, like a service containing both create_shard and
      // createShard, then we name the exception types without any case
      // conversion; instead of a CreateShardExn they'll get create_shardExn
      // and createShardExn.
      if (parent) {
        int count = 0;
        for (const auto& f : parent->functions()) {
          if (camelcase(f.name()) == upcamel_name) {
            count++;
          }
        }
        if (count > 1) {
          upcamel_name = self.name();
        }
      }
      return upcamel_name;
    });
    // Helper for unique exception field filtering — finds exceptions whose
    // type appears exactly once (used for From<> impls where duplicate types
    // would conflict).
    auto make_unique = [&proto](const t_structured* s) {
      std::vector<const t_field*> unique_exceptions;
      if (s) {
        const auto& exceptions = s->fields();
        std::map<const t_type*, unsigned> type_count;
        for (const auto& x : exceptions) {
          type_count[x.type().get_type()] += 1;
        }
        for (const auto& x : exceptions) {
          if (type_count.at(x.type().get_type()) == 1) {
            unique_exceptions.emplace_back(&x);
          }
        }
      }
      return to_array(unique_exceptions, proto.of<t_field>());
    };
    def.property("index", [this](const t_function& self) {
      const t_interface* parent = context().get_function_parent(&self);
      if (!parent) {
        return whisker::i64(0);
      }
      whisker::i64 idx = 0;
      for (const auto& f : parent->functions()) {
        if (&f == &self) {
          return idx;
        }
        ++idx;
      }
      return whisker::i64(0);
    });
    def.property("uniqueExceptions", [make_unique](const t_function& self) {
      return make_unique(self.exceptions());
    });
    def.property(
        "uniqueStreamExceptions", [make_unique](const t_function& self) {
          const t_stream* stream = self.stream();
          return make_unique(stream ? stream->exceptions() : nullptr);
        });
    def.property("uniqueSinkExceptions", [make_unique](const t_function& self) {
      const t_sink* sink = self.sink();
      return make_unique(sink ? sink->sink_exceptions() : nullptr);
    });
    def.property(
        "uniqueSinkFinalExceptions", [make_unique](const t_function& self) {
          const t_sink* sink = self.sink();
          return make_unique(
              sink ? sink->final_response_exceptions() : nullptr);
        });
    def.property("args", [&proto](const t_function& self) {
      return to_array(self.params().fields().copy(), proto.of<t_field>());
    });
    // Exception arrays for streams and sinks (mstch provides these; whisker
    // base only has boolean *_exceptions? properties)
    auto make_exception_fields = [&proto](const t_throws* exceptions) {
      if (exceptions) {
        return to_array(exceptions->fields().copy(), proto.of<t_field>());
      }
      return whisker::make::array(whisker::array::raw{});
    };
    def.property(
        "stream_exceptions",
        [make_exception_fields](const t_function& self) -> whisker::object {
          const t_stream* stream = self.stream();
          return stream ? make_exception_fields(stream->exceptions())
                        : whisker::make::null;
        });
    def.property(
        "sink_exceptions",
        [make_exception_fields](const t_function& self) -> whisker::object {
          const t_sink* sink = self.sink();
          return sink ? make_exception_fields(sink->sink_exceptions())
                      : whisker::make::null;
        });
    def.property(
        "sink_final_response_exceptions",
        [make_exception_fields](const t_function& self) -> whisker::object {
          const t_sink* sink = self.sink();
          return sink ? make_exception_fields(sink->final_response_exceptions())
                      : whisker::make::null;
        });
    // Element/response type context-switch properties for streams and sinks
    def.property(
        "stream_elem_type",
        [&proto](const t_function& self) -> whisker::object {
          const t_stream* stream = self.stream();
          return stream
              ? resolve_derived_t_type(proto, stream->elem_type().deref())
              : whisker::make::null;
        });
    def.property(
        "stream_first_response_type",
        [&proto](const t_function& self) -> whisker::object {
          const t_stream* stream = self.stream();
          if (stream && !self.has_void_initial_response()) {
            return resolve_derived_t_type(proto, self.return_type().deref());
          }
          return whisker::make::null;
        });
    def.property(
        "sink_elem_type", [&proto](const t_function& self) -> whisker::object {
          const t_sink* sink = self.sink();
          return sink ? resolve_derived_t_type(proto, sink->elem_type().deref())
                      : whisker::make::null;
        });
    def.property(
        "sink_first_response_type",
        [&proto](const t_function& self) -> whisker::object {
          const t_sink* sink = self.sink();
          if (sink && !self.has_void_initial_response()) {
            return resolve_derived_t_type(proto, self.return_type().deref());
          }
          return whisker::make::null;
        });
    def.property(
        "sink_final_response_type",
        [&proto](const t_function& self) -> whisker::object {
          const t_sink* sink = self.sink();
          return sink ? resolve_derived_t_type(
                            proto, sink->final_response_type().deref())
                      : whisker::make::null;
        });
    return std::move(def).make();
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);
    // mstch "structs" = structured_definitions (structs, unions, and
    // exceptions)
    def.property("structs", [&proto](const t_program& self) {
      whisker::array::raw result;
      for (const t_structured* s : self.structured_definitions()) {
        result.emplace_back(resolve_derived_t_type(proto, *s));
      }
      return whisker::make::array(std::move(result));
    });
    // mstch "constants" = consts
    def.property("constants", [&proto](const t_program& self) {
      return to_array(self.consts(), proto.of<t_const>());
    });
    def.property("has_const_tests?", [](const t_program& self) {
      for (const t_const* c : self.consts()) {
        if (type_has_transitive_adapter(c->type(), true)) {
          return true;
        }
      }
      return false;
    });
    def.property("consts_for_test", [](const t_program& self) {
      whisker::array::raw consts;
      for (const t_const* c : self.consts()) {
        if (type_has_transitive_adapter(c->type(), true)) {
          consts.emplace_back(whisker::make::string(c->name()));
        }
      }
      return whisker::make::array(std::move(consts));
    });
    def.property("serde?", [this](const t_program&) { return options_.serde; });
    def.property(
        "valuable?", [this](const t_program&) { return options_.valuable; });
    def.property("skip_none_serialization?", [this](const t_program&) {
      return options_.skip_none_serialization;
    });
    def.property("multifile?", [this](const t_program&) {
      return options_.multifile_mode;
    });
    def.property("rust_gen_native_metadata?", [this](const t_program&) {
      return options_.gen_metadata;
    });
    def.property("nonexhaustiveStructs?", [](const t_program& self) {
      for (t_structured* strct : self.structs_and_unions()) {
        if (!strct->is<t_union>() &&
            !strct->has_structured_annotation(kRustExhaustiveUri)) {
          return true;
        }
      }
      for (t_exception* strct : self.exceptions()) {
        if (!strct->has_structured_annotation(kRustExhaustiveUri)) {
          return true;
        }
      }
      return false;
    });
    def.property("direct_dependencies?", [this](const t_program&) {
      return !options_.crate_index.direct_dependencies().empty();
    });
    def.property("types", [this](const t_program& self) {
      auto types = "::" + options_.types_crate;
      if (options_.multifile_mode) {
        types += "::" + multifile_module_name(&self);
      }
      return types;
    });
    def.property("clients", [this](const t_program& self) {
      auto clients = "::" + options_.clients_crate;
      if (options_.multifile_mode) {
        clients += "::" + multifile_module_name(&self);
      }
      return clients;
    });
    def.property("crate", [this](const t_program& self) {
      if (options_.multifile_mode) {
        return std::string("crate::") + multifile_module_name(&self);
      }
      return std::string("crate");
    });
    def.property("split_mode_enabled?", [this](const t_program&) {
      return static_cast<bool>(options_.types_split_count);
    });
    def.property("include_docs", [this](const t_program&) {
      return options_.include_docs;
    });
    def.property("has_default_tests?", [](const t_program& self) {
      for (const t_structured* strct : self.structs_and_unions()) {
        for (const t_field& field : strct->fields()) {
          if (node_has_adapter(field) ||
              type_has_transitive_adapter(field.type().get_type(), true)) {
            return true;
          }
        }
      }
      return false;
    });
    def.property("has_adapted_structs?", [](const t_program& self) {
      for (const t_structured* strct : self.structs_and_unions()) {
        if (node_has_adapter(*strct)) {
          return true;
        }
      }
      return false;
    });
    def.property("has_adapters?", [](const t_program& self) {
      for (const t_structured* strct : self.structs_and_unions()) {
        if (node_has_adapter(*strct)) {
          return true;
        }
      }
      for (const t_typedef* t : self.typedefs()) {
        if (node_has_adapter(*t)) {
          return true;
        }
      }
      return false;
    });
    def.property("types_include_srcs", [this](const t_program&) {
      whisker::array::raw result;
      for (const auto& s : options_.types_include_srcs) {
        result.emplace_back(whisker::make::string(s));
      }
      return whisker::make::array(std::move(result));
    });
    def.property("clients_include_srcs", [this](const t_program&) {
      whisker::array::raw result;
      for (const auto& s : options_.clients_include_srcs) {
        result.emplace_back(whisker::make::string(s));
      }
      return whisker::make::array(std::move(result));
    });
    def.property("services_include_srcs", [this](const t_program&) {
      whisker::array::raw result;
      for (const auto& s : options_.services_include_srcs) {
        result.emplace_back(whisker::make::string(s));
      }
      return whisker::make::array(std::move(result));
    });
    def.property("direct_dependencies", [this](const t_program&) {
      whisker::array::raw deps;
      for (auto crate : options_.crate_index.direct_dependencies()) {
        whisker::map::raw dep;
        dep["name"] =
            whisker::make::string(mangle_crate_name(crate->dependency_path[0]));
        dep["name_unmangled"] =
            whisker::make::string(crate->dependency_path[0]);
        dep["label"] = whisker::make::string(crate->label);
        deps.emplace_back(whisker::make::map(std::move(dep)));
      }
      return whisker::make::array(std::move(deps));
    });
    def.property("structs_for_default_test", [&proto](const t_program& self) {
      std::vector<const t_structured*> structs;
      for (const t_structured* strct : self.structs_and_unions()) {
        for (const t_field& field : strct->fields()) {
          if (node_has_adapter(field) ||
              type_has_transitive_adapter(field.type().get_type(), true)) {
            structs.push_back(strct);
            break;
          }
        }
      }
      return to_array(structs, proto.of<t_structured>());
    });
    def.property("adapters", [&proto](const t_program& self) {
      whisker::array::raw result;
      for (const t_structured* strct : self.structs_and_unions()) {
        if (node_has_adapter(*strct)) {
          result.emplace_back(resolve_derived_t_type(proto, *strct));
        }
      }
      for (const t_typedef* t : self.typedefs()) {
        if (node_has_adapter(*t)) {
          result.emplace_back(resolve_derived_t_type(proto, *t));
        }
      }
      return whisker::make::array(std::move(result));
    });
    def.property("adapted_structs", [&proto](const t_program& self) {
      std::vector<const t_structured*> structs;
      for (const t_structured* strct : self.structs_and_unions()) {
        if (node_has_adapter(*strct)) {
          structs.push_back(strct);
        }
      }
      return to_array(structs, proto.of<t_structured>());
    });
    def.property("nonstandardTypes", [&proto](const t_program& self) {
      // Collect types with nonstandard type annotations (annotation contains
      // "::"), sorted by annotation then resolved name for deterministic
      // output.
      struct type_less {
        bool operator()(const t_type* lhs, const t_type* rhs) const {
          std::string la = get_type_annotation(lhs);
          std::string ra = get_type_annotation(rhs);
          if (la != ra) {
            return la < ra;
          }
          return get_resolved_name(lhs) < get_resolved_name(rhs);
        }
      };
      std::set<const t_type*, type_less> types;
      // Struct field types
      for (const t_structured* strct : self.structs_and_unions()) {
        for (const auto& field : strct->fields()) {
          const t_type* type = field.type().get_type();
          if (has_nonstandard_type_annotation(type)) {
            types.insert(type);
          }
        }
      }
      // Service function param/return types
      for (const auto* service : self.services()) {
        for (const auto& function : service->functions()) {
          for (const auto& param : function.params().fields()) {
            const t_type* type = param.type().get_type();
            if (has_nonstandard_type_annotation(type)) {
              types.insert(type);
            }
          }
          const t_type* ret = function.return_type().get_type();
          if (has_nonstandard_type_annotation(ret)) {
            types.insert(ret);
          }
        }
      }
      // Typedefs
      for (const t_typedef* t : self.typedefs()) {
        if (has_nonstandard_type_annotation(t)) {
          types.insert(t);
        }
      }
      whisker::array::raw result;
      for (const t_type* type : types) {
        result.emplace_back(resolve_derived_t_type(proto, *type));
      }
      return whisker::make::array(std::move(result));
    });
    def.property("nonstandardFields", [&proto](const t_program& self) {
      // Collect fields with nonstandard type annotations whose resolved
      // type name is not already covered by nonstandardTypes.
      struct type_less_t {
        bool operator()(const t_type* lhs, const t_type* rhs) const {
          std::string la = get_type_annotation(lhs);
          std::string ra = get_type_annotation(rhs);
          if (la != ra) {
            return la < ra;
          }
          return get_resolved_name(lhs) < get_resolved_name(rhs);
        }
      };
      struct field_less {
        bool operator()(const t_field* lhs, const t_field* rhs) const {
          std::string la = get_type_annotation(lhs);
          std::string ra = get_type_annotation(rhs);
          if (la != ra) {
            return la < ra;
          }
          return get_resolved_name(lhs) < get_resolved_name(rhs);
        }
      };
      // First, compute the set of resolved names from nonstandardTypes.
      std::set<const t_type*, type_less_t> types;
      for (const t_structured* strct : self.structs_and_unions()) {
        for (const auto& field : strct->fields()) {
          const t_type* type = field.type().get_type();
          if (has_nonstandard_type_annotation(type)) {
            types.insert(type);
          }
        }
      }
      for (const auto* service : self.services()) {
        for (const auto& function : service->functions()) {
          for (const auto& param : function.params().fields()) {
            const t_type* type = param.type().get_type();
            if (has_nonstandard_type_annotation(type)) {
              types.insert(type);
            }
          }
          const t_type* ret = function.return_type().get_type();
          if (has_nonstandard_type_annotation(ret)) {
            types.insert(ret);
          }
        }
      }
      for (const t_typedef* t : self.typedefs()) {
        if (has_nonstandard_type_annotation(t)) {
          types.insert(t);
        }
      }
      std::set<std::string> names;
      std::transform(
          types.begin(),
          types.end(),
          std::inserter(names, names.end()),
          [](const t_type* t) { return get_resolved_name(t); });
      // Now collect fields not covered by those names.
      std::set<const t_field*, field_less> fields;
      for (const t_structured* strct : self.structs_and_unions()) {
        for (const auto& field : strct->fields()) {
          if (has_nonstandard_type_annotation(&field)) {
            if (names.find(get_resolved_name(&field.type().deref())) ==
                names.end()) {
              fields.insert(&field);
            }
          }
        }
      }
      std::vector<const t_field*> result(fields.begin(), fields.end());
      return to_array(result, proto.of<t_field>());
    });
    def.property("type_splits", [this](const t_program&) {
      whisker::array::raw splits;
      for (int i = 0; i <= options_.types_split_count; i++) {
        splits.emplace_back(whisker::i64(i));
      }
      return whisker::make::array(std::move(splits));
    });
    def.property("types_with_constructors", [this](const t_program& self) {
      std::set<std::string> types;
      std::vector<t_enum*> enums;
      std::vector<t_typedef*> typedefs;
      if (options_.types_split_count > 0) {
        enums = enum_split_assignments_[current_split_id_];
        typedefs = typedef_split_assignments_[current_split_id_];
      } else {
        enums = self.enums();
        typedefs = self.typedefs();
      }
      for (const t_enum* t : enums) {
        types.insert(type_rust_name(t));
      }
      for (const t_typedef* t : typedefs) {
        if (typedef_has_constructor_expression(t)) {
          types.insert(type_rust_name(t));
        }
      }
      whisker::array::raw result;
      for (const auto& name : types) {
        result.emplace_back(whisker::make::string(name));
      }
      return whisker::make::array(std::move(result));
    });
    def.property("current_split_structs", [this, &proto](const t_program&) {
      return to_array(
          struct_split_assignments_[current_split_id_],
          proto.of<t_structured>());
    });
    def.property("current_split_typedefs", [this, &proto](const t_program&) {
      return to_array(
          typedef_split_assignments_[current_split_id_], proto.of<t_typedef>());
    });
    def.property("current_split_enums", [this, &proto](const t_program&) {
      return to_array(
          enum_split_assignments_[current_split_id_], proto.of<t_enum>());
    });
    return std::move(def).make();
  }
};

void t_mstch_rust_generator::process_options(
    const std::map<std::string, std::string>& options) {
  t_whisker_generator::process_options(options);

  if (auto types_crate_flag = get_compiler_option("types_crate")) {
    options_.types_crate = boost::algorithm::replace_all_copy(
        std::string{*types_crate_flag}, "-", "_");
  }

  if (auto clients_crate_flag = get_compiler_option("clients_crate")) {
    options_.clients_crate = boost::algorithm::replace_all_copy(
        std::string{*clients_crate_flag}, "-", "_");
  }

  if (auto cratemap_flag = get_compiler_option("cratemap")) {
    auto cratemap = load_crate_map(std::string{*cratemap_flag});
    options_.multifile_mode = cratemap.multifile_mode;
    options_.label = std::move(cratemap.label);
    options_.crate_index =
        rust_crate_index{program_, std::move(cratemap.cratemap)};
  }

  options_.serde = has_compiler_option("serde");
  options_.skip_none_serialization =
      has_compiler_option("skip_none_serialization");
  if (options_.skip_none_serialization) {
    assert(options_.serde);
  }

  options_.valuable = has_compiler_option("valuable");

  options_.any_registry_initialization_enabled = has_compiler_option("any");

  if (auto maybe_number = get_compiler_option("types_split_count")) {
    std::string number{*maybe_number};
    options_.types_split_count =
        checked_stoi(number, "Invalid types_split_count '" + number + "'");
  }

  parse_include_srcs(
      options_.types_include_srcs, get_compiler_option("types_include_srcs"));
  parse_include_srcs(
      options_.clients_include_srcs,
      get_compiler_option("clients_include_srcs"));
  parse_include_srcs(
      options_.services_include_srcs,
      get_compiler_option("services_include_srcs"));

  if (auto include_docs = get_compiler_option("include_docs")) {
    options_.include_docs = std::string{*include_docs};
  }

  if (options_.multifile_mode) {
    options_.current_crate = "crate::" + multifile_module_name(program_);
  } else {
    options_.current_crate = "crate";
  }

  options_.current_program = program_;
  out_dir_base_ = "gen-rust";

  if (auto gen_metadata = get_compiler_option("gen_metadata")) {
    options_.gen_metadata = *gen_metadata == "true";
  }
}

void t_mstch_rust_generator::generate_program() {
  std::string crate_name_option;
  bool has_crate_name = false;
  if (auto opt = get_compiler_option("crate_name")) {
    crate_name_option = std::string{*opt};
    has_crate_name = true;
  }
  std::string namespace_rust = program_->get_namespace("rust");
  if (!namespace_rust.empty()) {
    std::vector<std::string> pieces;
    boost::split(pieces, namespace_rust, boost::is_any_of("."));
    if (options_.multifile_mode) {
      if (pieces.size() > 2) {
        throw std::runtime_error(
            fmt::format(
                "`namespace rust {}`: namespace for multi-file Thrift library must have 1 piece, or 2 pieces separated by '.'",
                namespace_rust));
      }
    } else {
      if (pieces.size() != 1) {
        throw std::runtime_error(
            fmt::format(
                "`namespace rust {}`: namespace for single-file Thrift library must not contain '.'",
                namespace_rust));
      }
    }
    if (has_crate_name && !pieces.empty() && pieces[0] != crate_name_option) {
      throw std::runtime_error(
          fmt::format(
              R"(`namespace rust` disagrees with rust_crate_name option: "{}" vs "{}")",
              pieces[0],
              crate_name_option));
    }
  } else if (has_crate_name) {
    namespace_rust = crate_name_option;
  } else if (
      auto default_crate_name = get_compiler_option("default_crate_name")) {
    namespace_rust = std::string{*default_crate_name};
  } else {
    namespace_rust = program_->name();
  }

  std::string namespace_cpp2 = cpp_name_resolver::gen_namespace(*program_);

  std::string service_names;
  for (const t_service* service : program_->services()) {
    service_names += named_rust_name(service);
    service_names += '\n';
  }

  if (options_.types_split_count > 0) {
    generate_split_types();
  }

  auto& proto = *render_state().prototypes;
  whisker::object prog{proto.create<t_program>(*program_)};

  render_to_file("types.rs", "types.rs", prog);
  render_to_file("services.rs", "services.rs", prog);
  render_to_file("errors.rs", "errors.rs", prog);
  render_to_file("consts.rs", "consts.rs", prog);
  render_to_file("client.rs", "client.rs", prog);
  render_to_file("server.rs", "server.rs", prog);
  render_to_file("mock.rs", "mock.rs", prog);
  write_to_file("namespace-rust", namespace_rust + '\n');
  write_to_file("namespace-cpp2", namespace_cpp2 + '\n');
  write_to_file("service-names", service_names);
}

void t_mstch_rust_generator::generate_split_types() {
  initialize_type_splits();
  auto& proto = *render_state().prototypes;
  whisker::object prog{proto.create<t_program>(*program_)};
  for (int split_id = 0; split_id <= options_.types_split_count; ++split_id) {
    current_split_id_ = split_id;
    render_to_file(
        fmt::format("types_{}.rs", split_id), "lib/types_split", prog);
  }
}

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
          step_through_typedefs(&t->type().deref(), true);

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

void t_mstch_rust_generator::fill_validator_visitors(
    ast_validator& validator) const {
  validator.add_structured_definition_visitor(
      std::bind(
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

} // namespace

} // namespace apache::thrift::compiler::rust
