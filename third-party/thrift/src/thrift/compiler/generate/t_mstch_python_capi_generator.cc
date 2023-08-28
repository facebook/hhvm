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

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <fmt/format.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/gen/cpp/type_resolver.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/lib/cpp2/util.h>
#include <thrift/compiler/lib/py3/util.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

bool is_type_iobuf(std::string_view name) {
  return name == "folly::IOBuf" || name == "std::unique_ptr<folly::IOBuf>";
}

// t_node must be t_type or t_field
const t_const* find_structured_annotation(const t_node& node, const char* uri) {
  if (auto field = dynamic_cast<const t_field*>(&node)) {
    const t_const* annotation = field->find_structured_annotation_or_null(uri);
    return annotation ? annotation
                      : t_typedef::get_first_structured_annotation_or_null(
                            field->get_type(), uri);
  } else if (auto type = dynamic_cast<const t_type*>(&node)) {
    return t_typedef::get_first_structured_annotation_or_null(type, uri);
  }
  return nullptr;
}

const t_const_value* structured_type_override(
    const t_node& node, const char* key = "name") {
  if (auto annotation = find_structured_annotation(node, kCppTypeUri)) {
    return annotation->get_value_from_structured_annotation_or_null(key);
  }
  return nullptr;
}

const t_type* type_of_node(const t_node& node) {
  if (auto* field = dynamic_cast<const t_field*>(&node)) {
    return field->get_type();
  } else if (auto* type = dynamic_cast<const t_type*>(&node)) {
    return type;
  }
  return nullptr;
}

std::string_view cpp_type_override(const t_node& node) {
  if (auto typedef_override = structured_type_override(node)) {
    return typedef_override->get_string();
  }
  if (auto type = type_of_node(node)) {
    // finds unstructured annotation type, if present
    return cpp2::get_type(type->get_true_type());
  }
  return "";
}

std::string_view get_cpp_template(const t_node& node) {
  if (auto template_override = structured_type_override(node, "template")) {
    return template_override->get_string();
  }
  if (auto type = type_of_node(node)) {
    // finds unstructured annotation template, if present
    if (const auto* _template = type->get_true_type()->find_annotation_or_null(
            {"cpp.template", "cpp2.template"})) {
      return *_template;
    }
  }
  return "";
}

std::string_view get_annotation_property(
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

bool marshal_capi_override_annotation(const t_named& node) {
  return node.find_structured_annotation_or_null(kMarshalCapiUri) != nullptr;
}

inline std::string get_capi_include_namespace(const t_program* prog) {
  return fmt::format(
      "{}gen-python-capi/{}/thrift_types_capi.h",
      prog->include_prefix(),
      prog->name());
}

// Formats a field's type, or a sub-type
// of a compound type (e.g., a container)
// Hence, assumes t_node is a t_field or t_type
std::string format_marshal_type(
    const t_node& node, std::string_view type_override = "");

// Assumes t_node is a t_field or t_type
std::string format_unary_type(
    const t_node& node,
    const t_type* elem_type,
    const char* container,
    std::string_view type_override) {
  if (type_override.empty()) {
    type_override = cpp_type_override(node);
  }
  if (!type_override.empty()) {
    std::string elem_thrift_type = format_marshal_type(
        *elem_type, fmt::format("{}::value_type", type_override));
    return fmt::format(
        "{}<{}, {}>", container, elem_thrift_type, type_override);
  }
  auto template_override = get_cpp_template(node);
  std::string elem_thrift_type = format_marshal_type(*elem_type);
  if (!template_override.empty()) {
    return fmt::format(
        "{}<{}, {}<native_t<{}>>>",
        container,
        elem_thrift_type,
        template_override,
        elem_thrift_type);
  }
  return fmt::format("{}<{}>", container, elem_thrift_type);
}

// Assumes t_node is a t_field or t_type
std::string format_map_type(
    const t_node& node,
    const t_type* key_type,
    const t_type* val_type,
    std::string_view type_override) {
  if (type_override.empty()) {
    type_override = cpp_type_override(node);
  }
  if (!type_override.empty()) {
    std::string key_thrift_type = format_marshal_type(
        *key_type, fmt::format("{}::key_type", type_override));
    std::string val_thrift_type = format_marshal_type(
        *val_type, fmt::format("{}::mapped_type", type_override));
    return fmt::format(
        "map<{}, {}, {}>", key_thrift_type, val_thrift_type, type_override);
  }
  auto template_override = get_cpp_template(node);
  std::string key_thrift_type = format_marshal_type(*key_type);
  std::string val_thrift_type = format_marshal_type(*val_type);
  if (!template_override.empty()) {
    return fmt::format(
        "map<{}, {}, {}<native_t<{}>, native_t<{}>>>",
        key_thrift_type,
        val_thrift_type,
        template_override,
        key_thrift_type,
        val_thrift_type);
  }
  return fmt::format("map<{}, {}>", key_thrift_type, val_thrift_type);
}

// Assumes t_node is a t_field or t_type
std::string format_marshal_type_unadapted(
    const t_node& node, std::string_view type_override);

std::string format_adapted(const t_const* adapter, const t_type* type) {
  auto adaptedType = get_annotation_property(adapter, "adaptedType");
  if (adaptedType.empty()) {
    return fmt::format(
        "::apache::thrift::python::capi::AdaptedThrift<{}, {}>",
        get_annotation_property(adapter, "name"),
        format_marshal_type_unadapted(*type, ""));
  }
  return fmt::format(
      "::apache::thrift::python::capi::CircularlyAdaptedThrift<{}, {}>",
      get_annotation_property(adapter, "name"),
      adaptedType);
}

std::string format_marshal_type(
    const t_node& node, std::string_view type_override) {
  if (auto adapter = find_structured_annotation(node, kCppAdapterUri)) {
    return format_adapted(adapter, type_of_node(node));
  }
  return format_marshal_type_unadapted(node, type_override);
}

std::string format_marshal_type_unadapted(
    const t_node& node, std::string_view type_override) {
  const t_type* true_type = type_of_node(node)->get_true_type();
  std::string_view t_override = cpp_type_override(node);
  auto override_or = [&](const char* default_) {
    return t_override.empty() ? std::string(default_) : std::string(t_override);
  };
  if (true_type->is_bool()) {
    return override_or("bool");
  } else if (true_type->is_byte()) {
    return override_or("int8_t");
  } else if (true_type->is_i16()) {
    return override_or("int16_t");
  } else if (true_type->is_i32()) {
    return override_or("int32_t");
  } else if (true_type->is_i64()) {
    return override_or("int64_t");
  } else if (true_type->is_float()) {
    return override_or("float");
  } else if (true_type->is_double()) {
    return override_or("double");
  } else if (true_type->is_binary() && is_type_iobuf(t_override)) {
    return std::string(t_override);
  } else if (true_type->is_string_or_binary()) {
    // unicode's internal_data representation is binary
    return "Bytes";
  } else if (true_type->is_enum()) {
    return fmt::format(
        "::apache::thrift::python::capi::ComposedEnum<{}::{}>",
        cpp2::get_gen_namespace(*true_type->program()),
        cpp2::get_name(true_type));
  } else if (true_type->is_struct() || true_type->is_exception()) {
    return fmt::format(
        "::apache::thrift::python::capi::ComposedStruct<{}::{}>",
        cpp2::get_gen_namespace(*true_type->program()),
        cpp2::get_name(true_type));
  } else if (true_type->is_list()) {
    const auto* elem_type =
        dynamic_cast<const t_list*>(true_type)->get_elem_type();
    return format_unary_type(node, elem_type, "list", type_override);
  } else if (true_type->is_set()) {
    const auto* elem_type =
        dynamic_cast<const t_set*>(true_type)->get_elem_type();
    return format_unary_type(node, elem_type, "set", type_override);
  } else if (true_type->is_map()) {
    const auto* map = dynamic_cast<const t_map*>(true_type);
    return format_map_type(
        node, map->get_key_type(), map->get_val_type(), type_override);
  }
  // if not supported, empty string to cause compile error
  return "";
}

class python_capi_mstch_program : public mstch_program {
 public:
  python_capi_mstch_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:capi_includes",
             &python_capi_mstch_program::capi_includes},
            {"program:capi_module_prefix",
             &python_capi_mstch_program::capi_module_prefix},
            {"program:cpp_namespaces",
             &python_capi_mstch_program::get_cpp2_namespace},
            {"program:generate_capi?", &python_capi_mstch_program::has_types},
            {"program:module_path", &python_capi_mstch_program::module_path},
            {"program:marshal_capi?",
             &python_capi_mstch_program::has_marshal_types},
        });
    has_marshal_types_ = check_has_marshal_types();
    if (has_marshal_types_) {
      gather_capi_includes();
    }
    visit_types_for_objects();
    visit_types_for_typedefs();
  }

  mstch::node has_types() {
    return program_->objects().size() > 0 || program_->enums().size() > 0;
  }

  mstch::node capi_includes() {
    std::vector<const CapiInclude*> namespaces;
    for (const auto& it : capi_includes_) {
      namespaces.push_back(&it.second);
    }
    std::sort(
        namespaces.begin(), namespaces.end(), [](const auto* m, const auto* n) {
          return m->include_prefix < n->include_prefix;
        });
    mstch::array a;
    for (const auto& it : namespaces) {
      a.push_back(mstch::map{{"include_prefix", it->include_prefix}});
    }
    return a;
  }

  mstch::node capi_module_prefix() {
    std::string prefix = get_py3_namespace_with_name_and_prefix(
        program_, get_option("root_module_prefix"), "__");
    // kebab is not kosher in cpp fn names
    std::replace(prefix.begin(), prefix.end(), '-', '_');
    return prefix;
  }

  mstch::node has_marshal_types() { return has_marshal_types_; }

  mstch::node module_path() {
    return get_py3_namespace_with_name_and_prefix(
        program_, get_option("root_module_prefix"));
  }

  mstch::node get_cpp2_namespace() {
    return cpp2::get_gen_namespace(*program_);
  }

 protected:
  struct CapiInclude {
    std::string include_prefix;
  };

  bool check_has_marshal_types() {
    if (has_option("marshal_python_capi") &&
        (!program_->structs().empty() || !program_->exceptions().empty())) {
      return true;
    }
    for (const t_struct* s : program_->objects()) {
      if (marshal_capi_override_annotation(*s)) {
        return true;
      }
    }
    return false;
  }

  void gather_capi_includes() {
    for (const t_program* included_program :
         program_->get_includes_for_codegen()) {
      if (included_program->objects().empty() &&
          included_program->enums().empty() &&
          included_program->typedefs().empty()) {
        continue;
      }
      capi_includes_[included_program->path()] = CapiInclude{
          get_capi_include_namespace(included_program),
      };
    }
  }

  void add_typedef_namespace(const t_type* type) {
    auto prog = type->program();
    if (prog && prog != program_) {
      const auto& path = prog->path();
      if (capi_includes_.find(path) != capi_includes_.end()) {
        return;
      }

      if (has_marshal_types_) {
        capi_includes_[prog->path()] =
            CapiInclude{get_capi_include_namespace(prog)};
      }
    }
  }

  // visit structs and exceptions
  void visit_types_for_objects() {
    for (const auto& object : program_->objects()) {
      for (auto&& field : object->fields()) {
        visit_type(field.get_type());
      }
    }
  }

  void visit_types_for_typedefs() {
    for (const auto typedef_def : program_->typedefs()) {
      visit_type(typedef_def->get_type());
    }
  }

  enum TypeDef { NoTypedef, HasTypedef };

  void visit_type(const t_type* orig_type) {
    return visit_type_with_typedef(orig_type, TypeDef::NoTypedef);
  }

  void visit_type_with_typedef(const t_type* orig_type, TypeDef is_typedef) {
    if (!seen_types_.insert(orig_type).second) {
      return;
    }
    auto true_type = orig_type->get_true_type();
    is_typedef = is_typedef == TypeDef::HasTypedef || orig_type->is_typedef()
        ? TypeDef::HasTypedef
        : TypeDef::NoTypedef;
    if (is_typedef == TypeDef::HasTypedef) {
      add_typedef_namespace(true_type);
    }
    if (true_type->is_list()) {
      visit_type_with_typedef(
          dynamic_cast<const t_list&>(*true_type).get_elem_type(), is_typedef);
    } else if (true_type->is_set()) {
      visit_type_with_typedef(
          dynamic_cast<const t_set&>(*true_type).get_elem_type(), is_typedef);
    } else if (true_type->is_map()) {
      const auto* map = dynamic_cast<const t_map*>(true_type);
      visit_type_with_typedef(map->get_key_type(), is_typedef);
      visit_type_with_typedef(map->get_val_type(), is_typedef);
    }
  }

  std::unordered_map<std::string_view, CapiInclude> capi_includes_;
  std::unordered_set<const t_type*> seen_types_;
  bool has_marshal_types_ = false;
};

class python_capi_mstch_struct : public mstch_struct {
 public:
  python_capi_mstch_struct(
      const t_struct* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos) {
    register_methods(
        this,
        {
            {"struct:marshal_capi?", &python_capi_mstch_struct::marshal_capi},
            {"struct:cpp_name", &python_capi_mstch_struct::cpp_name},
            {"struct:cpp_adapter?", &python_capi_mstch_struct::cpp_adapter},
            {"struct:fields_size", &python_capi_mstch_struct::fields_size},
        });
  }

  mstch::node marshal_capi() {
    return !struct_->generated() &&
        (has_option("marshal_python_capi") ||
         marshal_capi_override_annotation(*struct_) ||
         struct_->fields().empty());
  }

  mstch::node cpp_adapter() {
    if (auto adapter_annotation =
            struct_->find_structured_annotation_or_null(kCppAdapterUri)) {
      return mstch::map{
          {"cpp_adapter:name",
           get_annotation_property(adapter_annotation, "name")},
      };
    }
    return false;
  }

  mstch::node cpp_name() {
    return cpp_resolver_.get_underlying_namespaced_name(*struct_);
  }

  mstch::node fields_size() { return std::to_string(struct_->fields().size()); }

 private:
  gen::cpp::type_resolver cpp_resolver_;
};

class python_capi_mstch_field : public mstch_field {
 public:
  python_capi_mstch_field(
      const t_field* field,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context)
      : mstch_field(field, ctx, pos, field_context),
        py_name_(py3::get_py3_name(*field)) {
    register_methods(
        this,
        {
            {"field:cpp_name", &python_capi_mstch_field::cpp_name},
            {"field:marshal_type", &python_capi_mstch_field::marshal_type},
        });
  }

  mstch::node cpp_name() { return cpp2::get_name(field_); }

  mstch::node marshal_type() { return format_marshal_type(*field_); }

 private:
  const std::string py_name_;
};

class python_capi_mstch_enum : public mstch_enum {
 public:
  python_capi_mstch_enum(
      const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum(e, ctx, pos) {
    register_methods(
        this,
        {
            {"enum:cpp_name", &python_capi_mstch_enum::cpp_name},
        });
  }

  mstch::node cpp_name() { return cpp2::get_name(enum_); }
};

class t_mstch_python_capi_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "python_capi"; }

  void generate_program() override {
    generate_root_path_ = package_to_path();
    out_dir_base_ = "gen-python-capi";
    auto include_prefix = get_option("include_prefix").value_or("");
    if (!include_prefix.empty()) {
      program_->set_include_prefix(std::move(include_prefix));
    }
    set_mstch_factories();
    generate_types();
  }

 protected:
  bool should_resolve_typedefs() const override { return true; }
  void set_mstch_factories();
  void generate_file(
      const std::string& file, const boost::filesystem::path& base);
  void generate_types();
  boost::filesystem::path package_to_path();

  boost::filesystem::path generate_root_path_;
};

void t_mstch_python_capi_generator::set_mstch_factories() {
  mstch_context_.add<python_capi_mstch_program>();
  mstch_context_.add<python_capi_mstch_struct>();
  mstch_context_.add<python_capi_mstch_field>();
  mstch_context_.add<python_capi_mstch_enum>();
}

boost::filesystem::path t_mstch_python_capi_generator::package_to_path() {
  auto package = get_py3_namespace(get_program());
  return boost::algorithm::join(package, "/");
}

void t_mstch_python_capi_generator::generate_file(
    const std::string& file, const boost::filesystem::path& base = {}) {
  auto program = get_program();
  const auto& name = program->name();
  auto mstch_program = make_mstch_program_cached(program, mstch_context_);
  render_to_file(mstch_program, file, base / name / file);
}

void t_mstch_python_capi_generator::generate_types() {
  generate_file("thrift_types_capi.pxd", generate_root_path_);
  generate_file("thrift_types_capi.pyx", generate_root_path_);
  generate_file("thrift_types_capi.h", "");
  generate_file("thrift_types_capi.cpp", "");
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_python_capi,
    "Python Capi",
    "    include_prefix:  Use full include paths in generated files.\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
