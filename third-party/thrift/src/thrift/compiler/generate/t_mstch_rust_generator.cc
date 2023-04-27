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
#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fmt/core.h>

#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/lib/rust/util.h>
#include <thrift/compiler/lib/uri.h>

using namespace std;

namespace apache {
namespace thrift {
namespace compiler {

namespace rust {
const std::string_view kRustCratePrefix = "crate::";

std::string quoted_rust_doc(const t_node* node) {
  const std::string doc = node->get_doc();

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

bool can_derive_ord(const t_type* type) {
  type = type->get_true_type();
  if (type->is_string() || type->is_binary() || type->is_bool() ||
      type->is_byte() || type->is_i16() || type->is_i32() || type->is_i64() ||
      type->is_enum() || type->is_void()) {
    return true;
  }
  if (type->has_annotation("rust.ord")) {
    return true;
  }
  if (type->is_list()) {
    auto elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
    return elem_type && can_derive_ord(elem_type);
  }
  return false;
}

struct rust_codegen_options {
  // Key: package name according to Thrift.
  // Value: rust_crate_name to use in generated code.
  std::map<std::string, std::string> cratemap;

  // Whether to emit derive(Serialize, Deserialize).
  // Enabled by `--gen rust:serde`.
  bool serde = false;

  // Whether fields w/optional values of None should
  // be skipped during serialization. Enabled w/ `--gen
  // rust:skip_none_serialization` Note: `rust:serde` must also be set for this
  // to affect codegen.
  bool skip_none_serialization = false;

  // Whether to skip server stubs. Server stubs are built by default, but can
  // be turned off via `--gen rust:noserver`.
  bool noserver = false;

  // True if we are generating a submodule rather than the whole crate.
  bool multifile_mode = false;

  // List of extra sources to include at top-level of the crate.
  std::vector<std::string> include_srcs;

  // The current program being generated and its Rust module path.
  const t_program* current_program;
  std::string current_crate;

  // Whether to generate Thrift metadata for structs or not.
  bool gen_metadata = true;
};

static bool validate_rust_serde(const t_node& node) {
  const std::string* ann = node.find_annotation_or_null("rust.serde");

  return ann == nullptr || *ann == "true" || *ann == "false";
}

static bool rust_serde_enabled(
    const rust_codegen_options& options, const t_node& node) {
  const std::string* ann = node.find_annotation_or_null("rust.serde");

  if (ann == nullptr) {
    return options.serde;
  } else if (*ann == "true") {
    return true;
  } else if (*ann == "false") {
    return false;
  } else {
    throw std::runtime_error("rust.serde should be `true` or `false`");
  }
}

std::string get_import_name(
    const t_program* program, const rust_codegen_options& options) {
  if (program == options.current_program) {
    return options.current_crate;
  }

  auto program_name = program->name();
  auto crate_name = options.cratemap.find(program_name);
  if (crate_name != options.cratemap.end()) {
    return crate_name->second;
  }
  return program_name;
}

enum class FieldKind { Box, Arc, Inline };

bool node_is_boxed(const t_named& node) {
  return node.has_annotation("rust.box") || node.has_annotation("thrift.box") ||
      node.find_structured_annotation_or_null(kBoxUri);
}

bool node_is_arced(const t_named& node) {
  return node.has_annotation("rust.arc");
}

FieldKind field_kind(const t_named& node) {
  if (node_is_arced(node)) {
    return FieldKind::Arc;
  }
  if (node_is_boxed(node)) {
    return FieldKind::Box;
  }
  return FieldKind::Inline;
}

// For example `set<Value> (rust.type = "indexmap::IndexSet")` or `map<string,
// Value> (rust.type = "indexmap::IndexMap")`. Unlike for standard library
// collections, serialization impls for these types are not provided in the
// fbthrift Rust runtime library and instead that logic will need to be emitted
// into the generated crate.
bool has_nonstandard_type_annotation(const t_type* type) {
  return type->get_annotation("rust.type").find("::") != string::npos;
}

const t_const* find_structured_adapter_annotation(const t_named& node) {
  return node.find_structured_annotation_or_null(
      "facebook.com/thrift/annotation/rust/Adapter");
}

const t_const* find_structured_derive_annotation(const t_named& node) {
  return node.find_structured_annotation_or_null(
      "facebook.com/thrift/annotation/rust/Derive");
}

bool node_has_adapter(const t_named& node) {
  return find_structured_adapter_annotation(node) != nullptr;
}

bool type_has_transitive_adapter(
    const t_type* type, bool step_through_newtypes) {
  if (type->is_typedef()) {
    auto typedef_type = dynamic_cast<const t_typedef*>(type);
    if (typedef_type) {
      // Currently the only "type" that can have an adapter is a typedef.
      if (node_has_adapter(*typedef_type)) {
        return true;
      }

      if (!step_through_newtypes &&
          (*typedef_type).has_annotation("rust.newtype")) {
        return false;
      }

      return type_has_transitive_adapter(
          typedef_type->get_type(), step_through_newtypes);
    }
  } else if (type->is_list()) {
    auto list_type = dynamic_cast<const t_list*>(type);
    if (list_type) {
      return type_has_transitive_adapter(
          list_type->get_elem_type(), step_through_newtypes);
    }
  } else if (type->is_set()) {
    auto set_type = dynamic_cast<const t_set*>(type);
    if (set_type) {
      return type_has_transitive_adapter(
          set_type->get_elem_type(), step_through_newtypes);
    }
  } else if (type->is_map()) {
    auto map_type = dynamic_cast<const t_map*>(type);
    if (map_type) {
      return type_has_transitive_adapter(
                 map_type->get_key_type(), step_through_newtypes) ||
          type_has_transitive_adapter(
                 map_type->get_val_type(), step_through_newtypes);
    }
  } else if (type->is_struct()) {
    auto struct_type = dynamic_cast<const t_struct*>(type);
    if (struct_type) {
      return node_has_adapter(*struct_type);
    }
  }

  return false;
}

const t_type* step_through_typedefs(const t_type* t, bool break_on_adapter) {
  while (t->is_typedef()) {
    auto typedef_type = dynamic_cast<const t_typedef*>(t);
    if (!typedef_type) {
      return t;
    }

    if ((*typedef_type).has_annotation("rust.newtype")) {
      return t;
    }

    if (break_on_adapter && node_has_adapter(*typedef_type)) {
      return t;
    }

    t = typedef_type->get_type();
  }

  return t;
}

bool node_has_custom_rust_type(const t_named& node) {
  return node.has_annotation("rust.type") ||
      node.has_annotation("rust.newtype");
}

const std::string get_annotation_property(
    const t_const* annotation, const std::string& key) {
  if (annotation) {
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == key) {
        return item.second->get_string();
      }
    }
  }
  return "";
}

// NOTE: a transitive _adapter_ is different from a transitive _annotation_. A
// transitive adapter is defined as one applied transitively through types. E.g.
// ```
// @rust.Adapter{ name = "Foo" }
// typedef string AdaptedString
//
// struct Bar {
//    1: AdaptedString field1;
// }
// ```
//
// `Bar.field1` has a transitive adapter due to its type being an adapted type.
//
// A transitive annotation is one that is applied through `@scope.Transitive`.
// E.g.
// ```
// @rust.Adapter{ name = "Foo" }
// @scope.Transitive
// struct SomeAnnotation {}
//
// struct Bar {
//    @SomeAnnotation
//    1: string field1;
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
    // If the annotation originates from the same module, this will just return
    // `crate::` anyways to be a no-op.
    std::string package =
        get_import_name(adapter_annotation->program(), options);

    auto adapter_name = get_annotation_property(adapter_annotation, "name");

    is_generic = boost::algorithm::ends_with(adapter_name, "<>");
    if (is_generic) {
      adapter_name = adapter_name.substr(0, adapter_name.length() - 2);
    }

    if (!(boost::algorithm::starts_with(adapter_name, "::") ||
          boost::algorithm::starts_with(adapter_name, kRustCratePrefix))) {
      adapter_name = "::" + adapter_name;
    } else if (
        !package.empty() &&
        boost::algorithm::starts_with(adapter_name, kRustCratePrefix)) {
      adapter_name =
          package + "::" + adapter_name.substr(kRustCratePrefix.length());
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

class mstch_rust_value;

mstch::node structured_annotations_node(
    const t_named& named,
    unsigned depth,
    mstch_context& context,
    mstch_element_position pos,
    const rust_codegen_options& options) {
  std::vector<mstch::node> annotations;

  // Note, duplicate annotations are not allowed per the Thrift spec.
  for (const t_const* annotation : named.structured_annotations()) {
    auto direct_annotation = std::make_shared<mstch_rust_value>(
        annotation->get_value(),
        annotation->get_type(),
        depth,
        context,
        pos,
        options);

    mstch::node transitive;
    const t_type* annotation_type = annotation->get_type();
    if ((*annotation_type).find_structured_annotation_or_null(kTransitiveUri)) {
      transitive = context.type_factory->make_mstch_object(
          annotation_type, context, pos);
    }

    annotations.push_back(mstch::map{
        {"structured_annotation:direct", direct_annotation},
        {"structured_annotation:transitive?", transitive},
    });
  }

  return annotations;
}

class t_mstch_rust_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "rust"; }

  void generate_program() override;
  void fill_validator_list(validator_list&) const override;

 private:
  void set_mstch_factories();
  rust_codegen_options options_;
};

class rust_mstch_program : public mstch_program {
 public:
  rust_mstch_program(
      const t_program* program,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options)
      : mstch_program(program, ctx, pos), options_(*options) {
    register_methods(
        this,
        {
            {"program:types?", &rust_mstch_program::rust_has_types},
            {"program:structsOrEnums?",
             &rust_mstch_program::rust_structs_or_enums},
            {"program:nonexhaustiveStructs?",
             &rust_mstch_program::rust_nonexhaustive_structs},
            {"program:serde?", &rust_mstch_program::rust_serde},
            {"program:skip_none_serialization?",
             &rust_mstch_program::rust_skip_none_serialization},
            {"program:server?", &rust_mstch_program::rust_server},
            {"program:multifile?", &rust_mstch_program::rust_multifile},
            {"program:crate", &rust_mstch_program::rust_crate},
            {"program:package", &rust_mstch_program::rust_package},
            {"program:includes", &rust_mstch_program::rust_includes},
            {"program:anyServiceWithoutParent?",
             &rust_mstch_program::rust_any_service_without_parent},
            {"program:nonstandardTypes?",
             &rust_mstch_program::rust_has_nonstandard_types},
            {"program:nonstandardTypes",
             &rust_mstch_program::rust_nonstandard_types},
            {"program:docs?", &rust_mstch_program::rust_has_docs},
            {"program:docs", &rust_mstch_program::rust_docs},
            {"program:include_srcs", &rust_mstch_program::rust_include_srcs},
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
        });
    register_has_option("program:default_enum_zero?", "default_enum_zero");
  }
  mstch::node rust_has_types() {
    return !program_->structs().empty() || !program_->enums().empty() ||
        !program_->typedefs().empty() || !program_->xceptions().empty();
  }
  mstch::node rust_structs_or_enums() {
    return !program_->structs().empty() || !program_->enums().empty() ||
        !program_->xceptions().empty();
  }
  mstch::node rust_nonexhaustive_structs() {
    for (auto& strct : program_->structs()) {
      // The is_union is because `union` are also in this collection.
      if (!strct->is_union() && !strct->has_annotation("rust.exhaustive")) {
        return true;
      }
    }
    for (auto& strct : program_->xceptions()) {
      if (!strct->has_annotation("rust.exhaustive")) {
        return true;
      }
    }
    return false;
  }
  mstch::node rust_serde() { return options_.serde; }
  mstch::node rust_skip_none_serialization() {
    return options_.skip_none_serialization;
  }
  mstch::node rust_server() { return !options_.noserver; }
  mstch::node rust_multifile() { return options_.multifile_mode; }
  mstch::node rust_crate() {
    if (options_.multifile_mode) {
      return "crate::" + mangle(program_->name());
    }
    return std::string("crate");
  }
  mstch::node rust_package() { return get_import_name(program_, options_); }
  mstch::node rust_includes() {
    mstch::array includes;
    for (auto* program : program_->get_included_programs()) {
      includes.push_back(
          context_.program_factory->make_mstch_object(program, context_, pos_));
    }
    return includes;
  }
  mstch::node rust_any_service_without_parent() {
    for (const t_service* service : program_->services()) {
      if (service->get_extends() == nullptr) {
        return true;
      }
    }
    return false;
  }
  template <typename F>
  void foreach_type(F&& f) const {
    for (const auto* strct : program_->structs()) {
      for (const auto& field : strct->fields()) {
        f(field.get_type());
      }
    }
    for (const auto* service : program_->services()) {
      for (const auto& function : service->functions()) {
        for (const auto& param : function.get_paramlist()->fields()) {
          f(param.get_type());
        }
        f(function.get_returntype());
      }
    }
    for (auto typedf : program_->typedefs()) {
      f(typedf);
    }
  }
  mstch::node rust_has_nonstandard_types() {
    bool has_nonstandard_types = false;
    foreach_type([&](const t_type* type) {
      if (has_nonstandard_type_annotation(type)) {
        has_nonstandard_types = true;
      }
    });
    return has_nonstandard_types;
  }
  mstch::node rust_nonstandard_types() {
    // Sort/deduplicate by value of `rust.type` annotation.
    struct rust_type_less {
      bool operator()(const t_type* lhs, const t_type* rhs) const {
        auto& lhs_annotation = lhs->get_annotation("rust.type");
        auto& rhs_annotation = rhs->get_annotation("rust.type");
        if (lhs_annotation != rhs_annotation) {
          return lhs_annotation < rhs_annotation;
        }

        std::function<std::string(const t_type*)> get_resolved_name =
            [&](const t_type* t) {
              t = t->get_true_type();
              if (auto c = dynamic_cast<const t_list*>(t)) {
                return fmt::format(
                    "list<{}>", get_resolved_name(c->get_elem_type()));
              }
              if (auto c = dynamic_cast<const t_set*>(t)) {
                return fmt::format(
                    "set<{}>", get_resolved_name(c->get_elem_type()));
              }
              if (auto c = dynamic_cast<const t_map*>(t)) {
                return fmt::format(
                    "map<{},{}>",
                    get_resolved_name(c->get_key_type()),
                    get_resolved_name(c->get_val_type()));
              }
              return t->get_full_name();
            };
        return get_resolved_name(lhs) < get_resolved_name(rhs);
      }
    };
    std::set<const t_type*, rust_type_less> nonstandard_types;
    foreach_type([&](const t_type* type) {
      if (has_nonstandard_type_annotation(type)) {
        nonstandard_types.insert(type);
      }
    });
    std::vector<const t_type*> elements(
        nonstandard_types.begin(), nonstandard_types.end());
    return make_mstch_types(elements);
  }
  mstch::node rust_has_docs() { return program_->has_doc(); }
  mstch::node rust_docs() { return quoted_rust_doc(program_); }
  mstch::node rust_include_srcs() {
    mstch::array elements;
    for (auto elem : options_.include_srcs) {
      mstch::map node;
      node["program:include_src"] = elem;
      elements.push_back(node);
    }
    return elements;
  }
  mstch::node rust_has_default_tests() {
    for (const t_struct* strct : program_->structs()) {
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

    for (const t_struct* strct : program_->structs()) {
      for (const t_field& field : strct->fields()) {
        if (node_has_adapter(field) ||
            type_has_transitive_adapter(field.get_type(), true)) {
          strcts.push_back(context_.struct_factory->make_mstch_object(
              strct, context_, pos_));
          break;
        }
      }
    }

    return strcts;
  }

  mstch::node rust_has_adapted_structs() {
    for (const t_struct* strct : program_->structs()) {
      if (node_has_adapter(*strct)) {
        return true;
      }
    }

    return false;
  }

  mstch::node rust_adapted_structs() {
    mstch::array strcts;

    for (const t_struct* strct : program_->structs()) {
      if (node_has_adapter(*strct)) {
        strcts.push_back(
            context_.struct_factory->make_mstch_object(strct, context_, pos_));
      }
    }

    return strcts;
  }

  mstch::node rust_has_adapters() {
    for (const t_struct* strct : program_->structs()) {
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

    for (const t_struct* strct : program_->structs()) {
      if (node_has_adapter(*strct)) {
        types_with_direct_adapters.push_back(
            context_.type_factory->make_mstch_object(strct, context_, pos_));
      }
    }

    for (const t_typedef* t : program_->typedefs()) {
      if (node_has_adapter(*t)) {
        types_with_direct_adapters.push_back(
            context_.type_factory->make_mstch_object(t, context_, pos_));
      }
    }

    return types_with_direct_adapters;
  }

  mstch::node rust_has_const_tests() {
    for (const t_const* c : program_->consts()) {
      if (type_has_transitive_adapter(c->get_type(), true)) {
        return true;
      }
    }

    return false;
  }
  mstch::node rust_consts_for_test() {
    mstch::array consts;

    for (const t_const* c : program_->consts()) {
      if (type_has_transitive_adapter(c->get_type(), true)) {
        consts.push_back(c->name());
      }
    }

    return consts;
  }
  mstch::node rust_gen_native_metadata() { return options_.gen_metadata; }

 private:
  const rust_codegen_options& options_;
};

class rust_mstch_service : public mstch_service {
 public:
  rust_mstch_service(
      const t_service* service,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options* options)
      : mstch_service(service, ctx, pos), options_(*options) {
    for (auto function : service->get_functions()) {
      function_upcamel_names_.insert(camelcase(function->get_name()));
    }
    register_methods(
        this,
        {
            {"service:rustFunctions", &rust_mstch_service::rust_functions},
            {"service:rust_exceptions",
             &rust_mstch_service::rust_all_exceptions},
            {"service:package", &rust_mstch_service::rust_package},
            {"service:snake", &rust_mstch_service::rust_snake},
            {"service:requestContext?",
             &rust_mstch_service::rust_request_context},
            {"service:extendedServices",
             &rust_mstch_service::rust_extended_services},
            {"service:docs?", &rust_mstch_service::rust_has_doc},
            {"service:docs", &rust_mstch_service::rust_doc},
            {"service:parent_service_name",
             &rust_mstch_service::parent_service_name},
        });
  }
  mstch::node rust_functions();
  mstch::node rust_package() {
    return get_import_name(service_->program(), options_);
  }
  mstch::node rust_snake() {
    return service_->get_annotation(
        "rust.mod", mangle_type(snakecase(service_->get_name())));
  }
  mstch::node rust_request_context() {
    return service_->has_annotation("rust.request_context");
  }
  mstch::node rust_extended_services() {
    mstch::array extended_services;
    const t_service* service = service_;
    std::string type_prefix = get_import_name(service_->program(), options_);
    std::string as_ref_impl = "&self.parent";
    while (true) {
      const t_service* parent_service = service->get_extends();
      if (parent_service == nullptr) {
        break;
      }
      if (parent_service->program() != service->program()) {
        type_prefix += "::dependencies::" + parent_service->program()->name();
      }
      mstch::map node;
      node["extendedService:packagePrefix"] = type_prefix;
      node["extendedService:asRefImpl"] = as_ref_impl;
      node["extendedService:service"] =
          make_mstch_extended_service_cached(parent_service);
      extended_services.push_back(node);
      as_ref_impl = "self.parent.as_ref()";
      service = parent_service;
    }
    return extended_services;
  }
  virtual mstch::node parent_service_name() { return service_->get_name(); }

  mstch::node rust_all_exceptions();
  mstch::node rust_has_doc() { return service_->has_doc(); }
  mstch::node rust_doc() { return quoted_rust_doc(service_); }

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
      : rust_mstch_service(interaction, ctx, pos, options),
        containing_service_(containing_service) {}

  mstch::node parent_service_name() override {
    return containing_service_->get_name();
  }

 private:
  const t_service* containing_service_ = nullptr;
};

class rust_mstch_function : public mstch_function {
 public:
  rust_mstch_function(
      const t_function* function,
      mstch_context& ctx,
      mstch_element_position pos,
      const std::unordered_multiset<std::string>& function_upcamel_names)
      : mstch_function(function, ctx, pos),
        function_upcamel_names_(function_upcamel_names),
        success_return(function->get_returntype(), "Success", 0) {
    register_methods(
        this,
        {
            {"function:rust_name", &rust_mstch_function::rust_name},
            {"function:upcamel", &rust_mstch_function::rust_upcamel},
            {"function:index", &rust_mstch_function::rust_index},
            {"function:uniqueExceptions",
             &rust_mstch_function::rust_unique_exceptions},
            {"function:uniqueStreamExceptions",
             &rust_mstch_function::rust_unique_stream_exceptions},
            {"function:args_by_name", &rust_mstch_function::rust_args_by_name},
            {"function:returns_by_name",
             &rust_mstch_function::rust_returns_by_name},
            {"function:docs?", &rust_mstch_function::rust_has_doc},
            {"function:docs", &rust_mstch_function::rust_doc},
            {"function:interaction_name",
             &rust_mstch_function::rust_interaction_name},
            {"function:void_excluding_interaction?",
             &rust_mstch_function::rust_void_excluding_interaction},
        });
  }
  mstch::node rust_name() {
    if (!function_->has_annotation("rust.name")) {
      return mangle(function_->get_name());
    }
    return function_->get_annotation("rust.name");
  }
  mstch::node rust_upcamel() {
    auto upcamel_name = camelcase(function_->get_name());
    if (function_upcamel_names_.count(upcamel_name) > 1) {
      // If a service contains a pair of methods that collide converted to
      // CamelCase, like a service containing both create_shard and createShard,
      // then we name the exception types without any case conversion; instead
      // of a CreateShardExn they'll get create_shardExn and createShardExn.
      return function_->get_name();
    }
    return upcamel_name;
  }
  mstch::node rust_index() { return pos_.index; }
  mstch::node rust_unique_exceptions() {
    return rust_make_unique_exceptions(function_->get_xceptions());
  }
  mstch::node rust_unique_stream_exceptions() {
    return rust_make_unique_exceptions(function_->get_stream_xceptions());
  }
  mstch::node rust_make_unique_exceptions(const t_struct* a) {
    // When generating From<> impls for an error type, we must not generate one
    // where more than one variant contains the same type of exception. Find
    // only those exceptions that map uniquely to a variant.

    const auto& exceptions = a->fields();
    std::map<const t_type*, unsigned> type_count;
    for (const auto& x : exceptions) {
      type_count[x.get_type()] += 1;
    }

    std::vector<const t_field*> unique_exceptions;
    for (const auto& x : exceptions) {
      if (type_count.at(x.get_type()) == 1) {
        unique_exceptions.emplace_back(&x);
      }
    }

    return make_mstch_fields(unique_exceptions);
  }
  mstch::node rust_args_by_name() {
    auto params = function_->get_paramlist()->fields().copy();
    std::sort(params.begin(), params.end(), [](auto a, auto b) {
      return a->get_name() < b->get_name();
    });
    return make_mstch_fields(params);
  }
  mstch::node rust_returns_by_name() {
    auto returns = function_->get_xceptions()->fields().copy();
    returns.push_back(&success_return);
    std::sort(returns.begin(), returns.end(), [](auto a, auto b) {
      return a->get_name() < b->get_name();
    });
    return make_mstch_fields(returns);
  }
  mstch::node rust_has_doc() { return function_->has_doc(); }
  mstch::node rust_doc() { return quoted_rust_doc(function_); }
  mstch::node rust_interaction_name() {
    if (!function_->returned_interaction().empty()) {
      return function_->returned_interaction()->get_name();
    }
    return function_->get_returntype()->get_name();
  }
  mstch::node rust_void_excluding_interaction() {
    return function_->return_type().deref().is_void();
  }

 private:
  const std::unordered_multiset<std::string>& function_upcamel_names_;
  t_field success_return;
};

class mstch_rust_value;

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
      const t_struct* s,
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
            {"struct:docs?", &rust_mstch_struct::rust_has_doc},
            {"struct:docs", &rust_mstch_struct::rust_doc},
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
        });
  }
  mstch::node rust_name() { return struct_rust_name(struct_); }
  mstch::node rust_package() {
    return get_import_name(struct_->program(), options_);
  }
  mstch::node rust_is_ord() {
    if (struct_->has_annotation("rust.ord")) {
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
    }
    return true;
  }
  mstch::node rust_is_copy() { return struct_->has_annotation("rust.copy"); }
  mstch::node rust_is_exhaustive() {
    return struct_->has_annotation("rust.exhaustive");
  }
  mstch::node rust_fields_by_name() {
    auto fields = struct_->fields().copy();
    std::sort(fields.begin(), fields.end(), [](auto a, auto b) {
      return a->get_name() < b->get_name();
    });
    return make_mstch_fields(fields);
  }
  mstch::node rust_has_doc() { return struct_->has_doc(); }
  mstch::node rust_doc() { return quoted_rust_doc(struct_); }
  mstch::node rust_derive() {
    if (auto annotation = find_structured_derive_annotation(*struct_)) {
      // Always replace `crate::` with the package name of where this annotation
      // originated to support derives applied with `@scope.Transitive`.
      // If the annotation originates from the same module, this will just
      // return `crate::` anyways to be a no-op.
      std::string package = get_import_name(annotation->program(), options_);

      std::string ret;
      std::string delimiter = "";

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

    if (!struct_->has_annotation("rust.derive")) {
      return nullptr;
    }
    return struct_->get_annotation("rust.derive");
  }
  mstch::node has_exception_message() {
    return struct_->has_annotation("message");
  }
  mstch::node is_exception_message_optional() {
    if (!struct_->has_annotation("message")) {
      return nullptr;
    }
    return struct_->get_field_by_name(struct_->get_annotation("message"))
               ->get_req() == t_field::e_req::optional;
  }
  mstch::node exception_message() { return struct_->get_annotation("message"); }
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

 private:
  const rust_codegen_options& options_;
  const t_const* adapter_annotation_;
};

class rust_mstch_enum_value : public mstch_enum_value {
 public:
  rust_mstch_enum_value(
      const t_enum_value* ev, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum_value(ev, ctx, pos) {
    register_methods(
        this,
        {
            {"enum_value:rust_name", &rust_mstch_enum_value::rust_name},
            {"enum_value:docs?", &rust_mstch_enum_value::rust_has_doc},
            {"enum_value:docs", &rust_mstch_enum_value::rust_doc},
        });
  }
  mstch::node rust_name() {
    if (!enum_value_->has_annotation("rust.name")) {
      return mangle(enum_value_->get_name());
    }
    return enum_value_->get_annotation("rust.name");
  }
  mstch::node rust_has_doc() { return enum_value_->has_doc(); }
  mstch::node rust_doc() { return quoted_rust_doc(enum_value_); }
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
            {"enum:variants_by_name", &rust_mstch_enum::variants_by_name},
            {"enum:variants_by_number", &rust_mstch_enum::variants_by_number},
            {"enum:docs?", &rust_mstch_enum::rust_has_doc},
            {"enum:docs", &rust_mstch_enum::rust_doc},
            {"enum:serde?", &rust_mstch_enum::rust_serde},
        });
  }
  mstch::node rust_name() {
    if (!enum_->has_annotation("rust.name")) {
      return mangle_type(enum_->get_name());
    }
    return enum_->get_annotation("rust.name");
  }
  mstch::node rust_package() {
    return get_import_name(enum_->program(), options_);
  }
  mstch::node variants_by_name() {
    std::vector<t_enum_value*> variants = enum_->get_enum_values();
    std::sort(variants.begin(), variants.end(), [](auto a, auto b) {
      return a->get_name() < b->get_name();
    });
    return make_mstch_enum_values(variants);
  }
  mstch::node variants_by_number() {
    std::vector<t_enum_value*> variants = enum_->get_enum_values();
    std::sort(variants.begin(), variants.end(), [](auto a, auto b) {
      return a->get_value() < b->get_value();
    });
    return make_mstch_enum_values(variants);
  }
  mstch::node rust_has_doc() { return enum_->has_doc(); }
  mstch::node rust_doc() { return quoted_rust_doc(enum_); }
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
            {"type:nonstandard?", &rust_mstch_type::rust_nonstandard},
            {"type:has_adapter?", &rust_mstch_type::adapter},
        });
  }
  mstch::node rust_name() {
    if (!type_->has_annotation("rust.name")) {
      return mangle_type(type_->get_name());
    }
    return type_->get_annotation("rust.name");
  }
  mstch::node rust_name_snake() {
    return snakecase(mangle_type(type_->get_name()));
  }
  mstch::node rust_package() {
    return get_import_name(type_->program(), options_);
  }
  mstch::node rust_type() {
    const std::string& rust_type = type_->get_annotation("rust.type");
    if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
      return "fbthrift::builtin_types::" + rust_type;
    }
    return rust_type;
  }
  mstch::node rust_nonstandard() {
    return has_nonstandard_type_annotation(type_) &&
        !(type_->is_typedef() && type_->has_annotation("rust.newtype"));
  }
  mstch::node adapter() {
    return adapter_node(nullptr, type_, false, context_, pos_, options_);
  }

 private:
  const rust_codegen_options& options_;
};

class mstch_rust_value : public mstch_base {
 public:
  using value_type = t_const_value::t_const_value_type;
  mstch_rust_value(
      const t_const_value* const_value,
      const t_type* type,
      unsigned depth,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options& options)
      : mstch_base(ctx, pos),
        const_value_(const_value),
        type_(type),
        depth_(depth),
        options_(options) {
    // Step through any non-newtype typedefs.
    type_ = step_through_typedefs(type_, false);

    register_methods(
        this,
        {
            {"value:type", &mstch_rust_value::type},
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
            {"value:enumPackage", &mstch_rust_value::enum_package},
            {"value:enumVariant", &mstch_rust_value::enum_variant},
            {"value:empty?", &mstch_rust_value::is_empty},
            {"value:indent", &mstch_rust_value::indent},
            {"value:simpleLiteral?", &mstch_rust_value::simple_literal},
        });
  }
  mstch::node type() {
    return context_.type_factory->make_mstch_object(type_, context_, pos_);
  }
  mstch::node is_newtype() {
    return type_->is_typedef() && type_->has_annotation("rust.newtype");
  }
  mstch::node inner() {
    auto typedef_type = dynamic_cast<const t_typedef*>(type_);
    if (typedef_type) {
      auto inner_type = typedef_type->get_type();
      return std::make_shared<mstch_rust_value>(
          const_value_, inner_type, depth_, context_, pos_, options_);
    }
    return mstch::node();
  }
  mstch::node is_bool() { return type_->is_bool(); }
  mstch::node bool_value() {
    if (const_value_->get_type() == value_type::CV_INTEGER) {
      return const_value_->get_integer() != 0;
    }
    return const_value_->get_bool();
  }
  mstch::node is_integer() {
    return type_->is_byte() || type_->is_i16() || type_->is_i32() ||
        type_->is_i64();
  }
  mstch::node integer_value() {
    return std::to_string(const_value_->get_integer());
  }
  mstch::node is_floating_point() {
    return type_->is_float() || type_->is_double();
  }
  mstch::node floating_point_value() {
    auto str = fmt::format(
        "{}",
        const_value_->get_type() == value_type::CV_INTEGER
            ? const_value_->get_integer()
            : const_value_->get_double());

    if (str.find('.') == std::string::npos &&
        str.find('e') == std::string::npos &&
        str.find('E') == std::string::npos) {
      str += ".0";
    }
    return str;
  }
  mstch::node is_string() { return type_->is_string(); }
  mstch::node is_binary() { return type_->is_binary(); }
  mstch::node string_quoted() {
    return quote(const_value_->get_string(), false);
  }
  mstch::node is_list() {
    return type_->is_list() &&
        (const_value_->get_type() == value_type::CV_LIST ||
         (const_value_->get_type() == value_type::CV_MAP &&
          const_value_->get_map().empty()));
  }
  mstch::node list_elements() {
    const t_type* elem_type;
    if (type_->is_set()) {
      auto set_type = dynamic_cast<const t_set*>(type_);
      if (!set_type) {
        return mstch::node();
      }
      elem_type = set_type->get_elem_type();
    } else {
      auto list_type = dynamic_cast<const t_list*>(type_);
      if (!list_type) {
        return mstch::node();
      }
      elem_type = list_type->get_elem_type();
    }

    mstch::array elements;
    for (auto elem : const_value_->get_list()) {
      elements.push_back(std::make_shared<mstch_rust_value>(
          elem, elem_type, depth_ + 1, context_, pos_, options_));
    }
    return elements;
  }
  mstch::node is_set() {
    return type_->is_set() &&
        (const_value_->get_type() == value_type::CV_LIST ||
         (const_value_->get_type() == value_type::CV_MAP &&
          const_value_->get_map().empty()));
  }
  mstch::node set_members() { return list_elements(); }
  mstch::node is_map() {
    return type_->is_map() &&
        (const_value_->get_type() == value_type::CV_MAP ||
         (const_value_->get_type() == value_type::CV_LIST &&
          const_value_->get_list().empty()));
  }
  mstch::node map_entries();
  mstch::node is_struct() {
    return (type_->is_struct() || type_->is_xception()) && !type_->is_union() &&
        const_value_->get_type() == value_type::CV_MAP;
  }
  mstch::node struct_fields();
  mstch::node is_exhaustive();
  mstch::node is_union() {
    if (!type_->is_union() || const_value_->get_type() != value_type::CV_MAP) {
      return false;
    }
    if (const_value_->get_map().empty()) {
      // value will be the union's Default
      return true;
    }
    return const_value_->get_map().size() == 1 &&
        const_value_->get_map().at(0).first->get_type() ==
        value_type::CV_STRING;
  }
  mstch::node union_variant() {
    if (const_value_->get_map().empty()) {
      return mstch::node();
    }
    return const_value_->get_map().at(0).first->get_string();
  }
  mstch::node union_value() {
    auto struct_type = dynamic_cast<const t_struct*>(type_);
    if (!struct_type) {
      return mstch::node();
    }

    auto entry = const_value_->get_map().at(0);
    auto variant = entry.first->get_string();
    auto content = entry.second;

    for (auto&& field : struct_type->fields()) {
      if (field.name() == variant) {
        return std::make_shared<mstch_rust_value>(
            content, field.get_type(), depth_ + 1, context_, pos_, options_);
      }
    }
    return mstch::node();
  }
  mstch::node is_enum() { return type_->is_enum(); }
  mstch::node enum_package() {
    if (type_->is_enum()) {
      return get_import_name(type_->program(), options_);
    }
    return mstch::node();
  }
  mstch::node enum_variant() {
    if (const_value_->is_enum()) {
      auto enum_value = const_value_->get_enum_value();
      if (enum_value) {
        return mangle(enum_value->get_name());
      }
    }
    return mstch::node();
  }
  mstch::node is_empty() {
    auto type = const_value_->get_type();
    if (type == value_type::CV_LIST) {
      return const_value_->get_list().empty();
    }
    if (type == value_type::CV_MAP) {
      return const_value_->get_map().empty();
    }
    if (type == value_type::CV_STRING) {
      return const_value_->get_string().empty();
    }
    return false;
  }
  mstch::node indent() { return std::string(4 * depth_, ' '); }
  mstch::node simple_literal() {
    // Primitives have simple literals
    if (type_->is_bool() || type_->is_any_int() || type_->is_floating_point()) {
      return true;
    }
    // Enum variants as well
    if (type_->is_enum()) {
      return enum_variant();
    }
    return false;
  }

 private:
  const t_const_value* const_value_;
  const t_type* type_;
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
      const t_const_value* value,
      unsigned depth,
      mstch_context& ctx,
      mstch_element_position pos,
      const rust_codegen_options& options)
      : mstch_base(ctx, pos),
        field_(field),
        value_(value),
        depth_(depth),
        options_(options),
        adapter_annotation_(find_structured_adapter_annotation(*field)) {
    register_methods(
        this,
        {
            {"field:key", &mstch_rust_struct_field::key},
            {"field:rust_name", &mstch_rust_struct_field::rust_name},
            {"field:optional?", &mstch_rust_struct_field::is_optional},
            {"field:value", &mstch_rust_struct_field::value},
            {"field:type", &mstch_rust_struct_field::type},
            {"field:box?", &mstch_rust_struct_field::is_boxed},
            {"field:arc?", &mstch_rust_struct_field::is_arc},
            {"field:docs?", &mstch_rust_struct_field::rust_has_docs},
            {"field:docs", &mstch_rust_struct_field::rust_docs},
            {"field:has_adapter?", &mstch_rust_struct_field::has_adapter},
        });
  }
  mstch::node key() { return std::to_string(field_->get_key()); }
  mstch::node rust_name() {
    if (!field_->has_annotation("rust.name")) {
      return mangle(field_->get_name());
    }
    return field_->get_annotation("rust.name");
  }
  mstch::node is_optional() {
    return field_->get_req() == t_field::e_req::optional;
  }
  mstch::node value() {
    if (value_) {
      auto type = field_->get_type();
      return std::make_shared<mstch_rust_value>(
          value_, type, depth_, context_, pos_, options_);
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
  mstch::node rust_docs() { return quoted_rust_doc(field_); }
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
  const t_const_value* value_;
  unsigned depth_;
  const rust_codegen_options& options_;
  const t_const* adapter_annotation_;
};

mstch::node mstch_rust_value::map_entries() {
  auto map_type = dynamic_cast<const t_map*>(type_);
  if (!map_type) {
    return mstch::node();
  }
  auto key_type = map_type->get_key_type();
  auto value_type = map_type->get_val_type();

  mstch::array entries;
  for (auto entry : const_value_->get_map()) {
    entries.push_back(std::make_shared<mstch_rust_map_entry>(
        entry.first,
        key_type,
        entry.second,
        value_type,
        depth_ + 1,
        context_,
        pos_,
        options_));
  }
  return entries;
}

mstch::node mstch_rust_value::struct_fields() {
  auto struct_type = dynamic_cast<const t_struct*>(type_);
  if (!struct_type) {
    return mstch::node();
  }

  std::map<std::string, const t_const_value*> map_entries;
  for (auto entry : const_value_->get_map()) {
    auto key = entry.first;
    if (key->get_type() == value_type::CV_STRING) {
      map_entries[key->get_string()] = entry.second;
    }
  }

  mstch::array fields;
  for (auto&& field : struct_type->fields()) {
    auto value = map_entries[field.name()];
    if (!value) {
      value = field.default_value();
    }
    fields.push_back(std::make_shared<mstch_rust_struct_field>(
        &field, value, depth_ + 1, context_, pos_, options_));
  }
  return fields;
}

mstch::node mstch_rust_value::is_exhaustive() {
  auto struct_type = dynamic_cast<const t_struct*>(type_);
  return struct_type && struct_type->has_annotation("rust.exhaustive");
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
            {"constant:package", &rust_mstch_const::rust_package},
            {"constant:lazy?", &rust_mstch_const::rust_lazy},
            {"constant:rust", &rust_mstch_const::rust_typed_value},
            {"constant:docs?", &rust_mstch_const::rust_has_docs},
            {"constant:docs", &rust_mstch_const::rust_docs},
        });
  }
  mstch::node rust_package() {
    return get_import_name(const_->get_program(), options_);
  }
  mstch::node rust_lazy() {
    if (type_has_transitive_adapter(const_->get_type(), true)) {
      return true;
    }

    auto type = const_->get_type()->get_true_type();
    return type->is_list() || type->is_map() || type->is_set() ||
        type->is_struct();
  }
  mstch::node rust_typed_value() {
    unsigned depth = 0;
    return std::make_shared<mstch_rust_value>(
        const_->get_value(),
        const_->get_type(),
        depth,
        context_,
        pos_,
        options_);
  }
  mstch::node rust_has_docs() { return const_->has_doc(); }
  mstch::node rust_docs() { return quoted_rust_doc(const_); }

 private:
  const rust_codegen_options& options_;
};

class rust_mstch_field : public mstch_field {
 public:
  rust_mstch_field(
      const t_field* field,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context,
      const rust_codegen_options* options)
      : mstch_field(field, ctx, pos, field_context),
        options_(*options),
        adapter_annotation_(find_structured_adapter_annotation(*field)) {
    register_methods(
        this,
        {
            {"field:rust_name", &rust_mstch_field::rust_name},
            {"field:primitive?", &rust_mstch_field::rust_primitive},
            {"field:rename?", &rust_mstch_field::rust_rename},
            {"field:default", &rust_mstch_field::rust_default},
            {"field:box?", &rust_mstch_field::rust_is_boxed},
            {"field:arc?", &rust_mstch_field::rust_is_arc},
            {"field:docs?", &rust_mstch_field::rust_has_docs},
            {"field:docs", &rust_mstch_field::rust_docs},
            {"field:has_adapter?", &rust_mstch_field::has_adapter},
            {"field:rust_structured_annotations",
             &rust_mstch_field::rust_structured_annotations},
        });
  }
  mstch::node rust_name() {
    if (!field_->has_annotation("rust.name")) {
      return mangle(field_->get_name());
    }
    return field_->get_annotation("rust.name");
  }
  mstch::node rust_primitive() {
    auto type = field_->get_type();
    return type->is_bool() || type->is_any_int() || type->is_floating_point();
  }
  mstch::node rust_rename() {
    return field_->get_name() != mangle(field_->get_name());
  }
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
  mstch::node rust_has_docs() { return field_->has_doc(); }
  mstch::node rust_docs() { return quoted_rust_doc(field_); }
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
            {"typedef:docs?", &rust_mstch_typedef::rust_has_docs},
            {"typedef:docs", &rust_mstch_typedef::rust_docs},
            {"typedef:serde?", &rust_mstch_typedef::rust_serde},
            {"typedef:has_adapter?", &rust_mstch_typedef::has_adapter},
        });
  }
  mstch::node rust_name() { return typedef_rust_name(typedef_); }
  mstch::node rust_newtype() {
    return typedef_->has_annotation("rust.newtype");
  }
  mstch::node rust_type() {
    const std::string& rust_type = typedef_->get_annotation("rust.type");
    if (!rust_type.empty() && rust_type.find("::") == std::string::npos) {
      return "fbthrift::builtin_types::" + rust_type;
    }
    return rust_type;
  }
  mstch::node rust_ord() {
    return typedef_->has_annotation("rust.ord") ||
        (can_derive_ord(typedef_->get_type()) &&
         !type_has_transitive_adapter(typedef_->get_type(), true));
  }
  mstch::node rust_copy() {
    if (!type_has_transitive_adapter(typedef_->get_type(), true)) {
      auto inner = typedef_->get_true_type();
      if (inner->is_bool() || inner->is_byte() || inner->is_i16() ||
          inner->is_i32() || inner->is_i64() || inner->is_enum() ||
          inner->is_void()) {
        return true;
      }
    }
    if (typedef_->has_annotation("rust.copy")) {
      return true;
    }
    return false;
  }
  mstch::node rust_nonstandard() {
    return typedef_->get_annotation("rust.type").find("::") != string::npos;
  }
  mstch::node rust_has_docs() { return typedef_->has_doc(); }
  mstch::node rust_docs() { return quoted_rust_doc(typedef_); }
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

 private:
  const rust_codegen_options& options_;
  const t_const* adapter_annotation_;
};

class rust_mstch_deprecated_annotation : public mstch_deprecated_annotation {
 public:
  rust_mstch_deprecated_annotation(
      const t_annotation* a, mstch_context& ctx, mstch_element_position pos)
      : mstch_deprecated_annotation(a, ctx, pos) {
    register_methods(
        this,
        {
            {"annotation:value?",
             &rust_mstch_deprecated_annotation::rust_has_value},
            {"annotation:rust_name",
             &rust_mstch_deprecated_annotation::rust_name},
            {"annotation:rust_value",
             &rust_mstch_deprecated_annotation::rust_value},
        });
  }
  mstch::node rust_has_value() { return !val_.value.empty(); }
  mstch::node rust_name() {
    return boost::algorithm::replace_all_copy(key_, ".", "_");
  }
  mstch::node rust_value() { return quote(val_.value, true); }
};

mstch::node rust_mstch_service::rust_functions() {
  return make_mstch_array(
      service_->get_functions(),
      rust_mstch_function_factory(),
      function_upcamel_names_);
}

struct name_less {
  bool operator()(const t_type* lhs, const t_type* rhs) const {
    return lhs->get_scoped_name() < rhs->get_scoped_name();
  }
};

mstch::node rust_mstch_service::rust_all_exceptions() {
  std::map<const t_type*, std::vector<const t_function*>, name_less>
      function_map;
  std::map<const t_type*, std::vector<const t_field*>> field_map;
  for (const auto& fun : service_->functions()) {
    for (const auto& fld : fun.get_xceptions()->fields()) {
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
      function_data.push_back(std::move(inner));
    }

    data["rust_exception:functions"] = std::move(function_data);
    output.push_back(data);
  }

  return output;
}

void t_mstch_rust_generator::generate_program() {
  if (auto cratemap_flag = get_option("cratemap")) {
    auto cratemap = load_crate_map(*cratemap_flag);
    options_.multifile_mode = cratemap.multifile_mode;
    options_.cratemap = std::move(cratemap.cratemap);
  }

  options_.serde = has_option("serde");
  options_.noserver = has_option("noserver");
  options_.skip_none_serialization = has_option("skip_none_serialization");
  if (options_.skip_none_serialization) {
    assert(options_.serde);
  }

  if (auto include_prefix_flag = get_option("include_prefix")) {
    program_->set_include_prefix(*include_prefix_flag);
  }

  if (auto include_srcs = get_option("include_srcs")) {
    auto paths = *include_srcs;

    string::size_type pos = 0;
    while (pos != string::npos && pos < paths.size()) {
      string::size_type next_pos = paths.find(':', pos);
      auto path = paths.substr(pos, next_pos - pos);
      options_.include_srcs.push_back(path);
      pos = ((next_pos == string::npos) ? next_pos : next_pos + 1);
    }
  }

  if (options_.multifile_mode) {
    options_.current_crate = "crate::" + mangle(program_->name());
  } else {
    options_.current_crate = "crate";
  }

  options_.current_program = program_;
  out_dir_base_ = "gen-rust";

  if (auto gen_metadata = get_option("gen_metadata")) {
    options_.gen_metadata = gen_metadata.value() == "true";
  }

  boost::optional<std::string> crate_name_option = get_option("crate_name");
  std::string namespace_rust = program_->get_namespace("rust");
  if (!namespace_rust.empty()) {
    std::vector<std::string> pieces;
    boost::split(pieces, namespace_rust, boost::is_any_of("."));
    if (options_.multifile_mode) {
      if (pieces.size() != 2) {
        throw std::runtime_error(fmt::format(
            "`namespace rust {}`: namespace for multi-file Thrift library must have 2 pieces separated by '.'",
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
          "`namespace rust` disagrees with rust_crate_name option: \"{}\" vs \"{}\"",
          pieces[0],
          *crate_name_option));
    }
  } else if (crate_name_option) {
    namespace_rust = *crate_name_option;
  } else {
    namespace_rust = program_->name();
  }

  set_mstch_factories();

  const auto& prog = cached_program(program_);
  render_to_file(prog, "lib.rs", "lib.rs");
  render_to_file(prog, "types.rs", "types.rs");
  write_output("namespace", namespace_rust);
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
  mstch_context_.add<rust_mstch_enum_value>();
  mstch_context_.add<rust_mstch_deprecated_annotation>();
  mstch_context_.add<rust_mstch_const>(&options_);
}

namespace {
class annotation_validator : public validator {
 public:
  explicit annotation_validator(const rust_codegen_options& options)
      : options_(options) {}
  using validator::visit;
  bool visit(t_struct*) override;
  bool visit(t_enum*) override;
  bool visit(t_program*) override;

 private:
  const rust_codegen_options& options_;
};

bool annotation_validator::visit(t_struct* s) {
  if (!validate_rust_serde(*s)) {
    report_error(*s, "`rust.serde` must be `true` or `false`");
  }

  for (auto& field : s->fields()) {
    FieldKind kind = field_kind(field);
    bool box = node_is_boxed(field) || kind == FieldKind::Box;
    bool arc = node_is_arced(field) || kind == FieldKind::Arc;
    if (box && arc) {
      report_error(
          field, "Field `{}` cannot be both Box'ed and Arc'ed", field.name());
    }

    if (node_has_adapter(field) &&
        (node_has_custom_rust_type(field) ||
         rust_serde_enabled(options_, field))) {
      report_error(
          field,
          "Field `{}` cannot have both an adapter and `rust.type` or `rust.newtype` or `rust.serde = true`",
          field.name());
    }
  }
  return true;
}

bool annotation_validator::visit(t_enum* e) {
  if (!validate_rust_serde(*e)) {
    report_error(*e, "`rust.serde` must be `true` or `false`");
  }
  return true;
}

bool annotation_validator::visit(t_program* p) {
  for (auto t : p->typedefs()) {
    if (node_has_adapter(*t)) {
      if (node_has_custom_rust_type(*t)) {
        report_error(
            *t,
            "Typedef `{}` cannot have both an adapter and `rust.type` or `rust.newtype`",
            (*t).name());
      }

      // To be spec compliant, adapted typedefs can be composed only if they are
      // wrapped. For example, the following is not allowed:
      // ```
      // @rust.Adapter{ name = "Foo" }
      // typedef string Foo
      //
      // @rust.Adapter{ name = "Bar" }
      // typedef Foo Bar
      // ```
      // But the following is allowed:
      // ```
      // @rust.Adapter{ name = "Foo" }
      // typedef string Foo
      //
      // @rust.Adapter{ name = "Bar" }
      // typedef list<Foo> Bar
      // ```
      const t_type* typedef_stepped =
          step_through_typedefs(t->get_type(), true);

      if (node_has_adapter(*typedef_stepped)) {
        report_error(
            *t,
            "Typedef `{}` cannot have a direct transitive adapter",
            (*t).name());
      }
    }
  }

  return true;
}

} // namespace

void t_mstch_rust_generator::fill_validator_list(validator_list& l) const {
  l.add<annotation_validator>(options_);
}

THRIFT_REGISTER_GENERATOR(
    mstch_rust,
    "Rust",
    "    serde:           Derive serde Serialize/Deserialize traits for types\n"
    "    noserver:        Don't emit server code\n"
    "    include_prefix=: Set program:include_prefix.\n"
    "    include_srcs=:   Additional Rust source file to include in output, "
    "`:` separated\n"
    "    cratemap=map:    Mapping file from services to crate names\n");

} // namespace rust
} // namespace compiler
} // namespace thrift
} // namespace apache
