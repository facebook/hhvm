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
#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <fmt/format.h>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/cpp/util.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/python/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/whisker/mstch_compat.h>

namespace apache::thrift::compiler {

namespace {

std::string_view remove_global_scope(std::string_view symbol) {
  if (symbol.size() >= 2 && symbol.find("::", 0, 2) == 0) {
    symbol.remove_prefix(2);
  }
  return symbol;
}

bool is_supported_template(std::string_view symbol) {
  symbol = remove_global_scope(symbol);
  static const std::unordered_set<std::string_view> kSupportedTemplates = {
      "folly::F14FastMap",
      "folly::F14FastSet",
      "folly::F14NodeMap",
      "folly::F14NodeSet",
      "folly::F14ValueMap",
      "folly::F14ValueSet",
      "folly::F14VectorMap",
      "folly::F14VectorSet",
      "folly::fbvector",
      "folly::small_vector",
      "folly::sorted_vector_map",
      "folly::sorted_vector_set",
      "std::deque",
      "std::map",
      "std::set",
      "std::unordered_map",
      "std::unordered_set",
      "std::vector",
  };
  return kSupportedTemplates.count(symbol);
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
    if (const auto* _template =
            type->get_true_type()->find_unstructured_annotation_or_null(
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

inline std::string get_capi_include(
    const t_program* prog, const t_program* this_prog) {
  const std::string& prefix = prog->include_prefix();
  return fmt::format(
      "{}gen-python-capi/{}/thrift_types_capi.h",
      prefix.empty() ? this_prog->include_prefix() : prefix,
      prog->name());
}

const std::string& gen_capi_module_prefix(const t_program* program) {
  static std::unordered_map<const t_program*, std::string> cache;
  auto it = cache.find(program);
  if (it != cache.end()) {
    return it->second;
  }
  auto inserted =
      cache.emplace(program, python::gen_capi_module_prefix_impl(program));
  return inserted.first->second;
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
  } else if (true_type->is_string()) {
    return "::apache::thrift::python::capi::FallibleString";
  } else if (true_type->is_binary()) {
    return "Bytes";
  } else if (true_type->is<t_enum>()) {
    return fmt::format(
        "::apache::thrift::python::capi::ComposedEnum<{}::{}>",
        cpp2::get_gen_namespace(*true_type->program()),
        cpp2::get_name(true_type));
  } else if (true_type->is<t_structured>()) {
    return fmt::format(
        "::apache::thrift::python::capi::ComposedStruct<{}::{}, ::{}::NamespaceTag>",
        cpp2::get_gen_namespace(*true_type->program()),
        cpp2::get_name(true_type),
        gen_capi_module_prefix(true_type->program()));

  } else if (const t_list* list = true_type->try_as<t_list>()) {
    const auto* elem_type = list->get_elem_type();
    return format_unary_type(node, elem_type, "list", type_override);
  } else if (const t_set* set = true_type->try_as<t_set>()) {
    const auto* elem_type = set->get_elem_type();
    return format_unary_type(node, elem_type, "set", type_override);
  } else if (const t_map* map = true_type->try_as<t_map>()) {
    return format_map_type(
        node,
        &map->key_type().deref(),
        &map->val_type().deref(),
        type_override);
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
            {"program:generate_capi?",
             &python_capi_mstch_program::has_types_node},
            {"program:module_path", &python_capi_mstch_program::module_path},
        });
    has_marshal_types_ = check_has_marshal_types();
    if (has_marshal_types_) {
      gather_capi_includes();
    }
    visit_types_for_objects();
    visit_types_for_typedefs();
  }

  mstch::node has_types_node() { return has_types(); }

  mstch::node capi_includes() {
    std::vector<const CapiInclude*> namespaces;
    namespaces.reserve(capi_includes_.size());
    for (const auto& it : capi_includes_) {
      namespaces.push_back(&it.second);
    }
    std::sort(
        namespaces.begin(), namespaces.end(), [](const auto* m, const auto* n) {
          return m->include_prefix < n->include_prefix;
        });
    mstch::array a;
    for (const auto& it : namespaces) {
      a.emplace_back(mstch::map{{"header_include", it->include_prefix}});
    }
    return a;
  }

  mstch::node capi_module_prefix() { return gen_capi_module_prefix(program_); }

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

  bool has_types() const {
    return !program_->structured_definitions().empty() ||
        !program_->enums().empty();
  }

  bool check_has_marshal_types() {
    return has_types() && !has_option("serialize_python_capi");
  }

  void gather_capi_includes() {
    for (const t_program* included_program :
         program_->get_includes_for_codegen()) {
      if (included_program->structured_definitions().empty() &&
          included_program->enums().empty() &&
          included_program->typedefs().empty()) {
        continue;
      }
      capi_includes_[included_program->path()] = CapiInclude{
          get_capi_include(included_program, program_),
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
            CapiInclude{get_capi_include(prog, program_)};
      }
    }
  }

  // visit structs and exceptions
  void visit_types_for_objects() {
    for (const t_structured* object : program_->structured_definitions()) {
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
    is_typedef = is_typedef == TypeDef::HasTypedef || orig_type->is<t_typedef>()
        ? TypeDef::HasTypedef
        : TypeDef::NoTypedef;
    if (is_typedef == TypeDef::HasTypedef) {
      add_typedef_namespace(true_type);
    }
    if (const t_list* list = true_type->try_as<t_list>()) {
      visit_type_with_typedef(list->get_elem_type(), is_typedef);
    } else if (const t_set* set = true_type->try_as<t_set>()) {
      visit_type_with_typedef(set->get_elem_type(), is_typedef);
    } else if (const t_map* map = true_type->try_as<t_map>()) {
      visit_type_with_typedef(&map->key_type().deref(), is_typedef);
      visit_type_with_typedef(&map->val_type().deref(), is_typedef);
    }
  }

  std::unordered_map<std::string_view, CapiInclude> capi_includes_;
  std::unordered_set<const t_type*> seen_types_;
  bool has_marshal_types_ = false;
};

class python_capi_mstch_struct : public mstch_struct {
 public:
  python_capi_mstch_struct(
      const t_structured* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos) {
    register_methods(
        this,
        {
            {"struct:py_name", &python_capi_mstch_struct::py_name},
            {"struct:marshal_capi?", &python_capi_mstch_struct::marshal_capi},
            {"struct:cpp_name", &python_capi_mstch_struct::cpp_name},
            {"struct:cpp_adapter?", &python_capi_mstch_struct::cpp_adapter},
            {"struct:num_fields", &python_capi_mstch_struct::num_fields},
            {"struct:tuple_positions",
             &python_capi_mstch_struct::tuple_positions},
        });
  }

  mstch::node py_name() { return python::get_py3_name(*struct_); }

  mstch::node tuple_positions() {
    std::vector<std::pair<int, int>> index_keys;
    size_t cpp_index = 0;
    for (const auto& f : struct_->fields()) {
      index_keys.emplace_back(f.get_key(), cpp_index++);
    }
    // sort by key to match thrift-python tuple ordering
    std::sort(index_keys.begin(), index_keys.end());
    // replace key with python tuple index
    for (size_t i = 0; i < index_keys.size(); ++i) {
      // offset by 1 because tuple position 0 is isset indicator
      index_keys[i].first = i + 1;
    }
    // now sort by cpp index
    std::sort(
        index_keys.begin(),
        index_keys.end(),
        [](const auto& tup_cpp1, const auto& tup_cpp2) {
          return tup_cpp1.second < tup_cpp2.second;
        });
    mstch::array a;
    for (size_t i = 0; i < index_keys.size(); ++i) {
      a.emplace_back(mstch::map{
          {"tuple:index", index_keys[i].first},
          {"tuple:comma", std::string_view(i == 0 ? "" : ", ")}});
    }
    return a;
  }

  bool capi_eligible_type_annotation(const t_const* annotation) {
    if (const auto* type_name =
            annotation->get_value_from_structured_annotation_or_null("name")) {
      return is_type_iobuf(type_name->get_string());
    }
    if (const auto* template_name =
            annotation->get_value_from_structured_annotation_or_null(
                "template")) {
      return is_supported_template(template_name->get_string());
    }
    return false;
  }

  bool capi_eligible_type(const t_type* type) {
    if (type->has_structured_annotation(kCppAdapterUri)) {
      return false;
    }

    if (const auto* cpp_type_anno =
            type->find_structured_annotation_or_null(kCppTypeUri)) {
      if (!capi_eligible_type_annotation(cpp_type_anno)) {
        return false;
      }
    }
    // thrift currently lowers structured annotations to unstructured
    // annotations so this will always be non-null if @cpp.Type annotation
    // used on type or field
    // TODO: delete these if structured annotation migration completed
    if (const std::string* template_anno =
            type->find_unstructured_annotation_or_null(
                {"cpp.template", "cpp2.template"})) {
      if (!is_supported_template(*template_anno)) {
        return false;
      }
    }
    if (const std::string* type_anno =
            type->find_unstructured_annotation_or_null(
                {"cpp.type", "cpp2.type"})) {
      return is_type_iobuf(*type_anno);
    }
    if (const t_list* list = type->try_as<t_list>();
        list != nullptr && !capi_eligible_type(list->get_elem_type())) {
      return false;
    } else if (const t_set* set = type->try_as<t_set>();
               set != nullptr && !capi_eligible_type(set->get_elem_type())) {
      return false;
    } else if (const t_map* map = type->try_as<t_map>(); map != nullptr &&
               (!capi_eligible_type(&map->key_type().deref()) ||
                !capi_eligible_type(&map->val_type().deref()))) {
      return false;
    }
    if (const t_typedef* tdef = type->try_as<t_typedef>()) {
      return capi_eligible_type(tdef->get_type());
    }
    return true;
  }

  bool capi_eligible_field(const t_field& field) {
    if (field.has_structured_annotation(kCppAdapterUri)) {
      return false;
    }
    if (const auto* cpp_type_anno =
            field.find_structured_annotation_or_null(kCppTypeUri)) {
      return capi_eligible_type_annotation(cpp_type_anno);
    }
    return true;
  }

  mstch::node marshal_capi() {
    const auto marshal_override =
        struct_->find_structured_annotation_or_null(kPythonUseCAPIUri);
    auto force_serialize = [marshal_override]() {
      const auto serialize_field = marshal_override
          ? marshal_override->get_value_from_structured_annotation_or_null(
                "serialize")
          : nullptr;
      return serialize_field && serialize_field->get_bool();
    };

    if (struct_->generated() || has_option("serialize_python_capi") ||
        force_serialize()) {
      return false;
    }
    if (has_option("marshal_python_capi") || marshal_override) {
      return true;
    }
    for (const auto& f : struct_->fields()) {
      if (!capi_eligible_field(f) || !capi_eligible_type(f.get_type())) {
        return false;
      }
    }
    return true;
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

  mstch::node num_fields() { return struct_->fields().size(); }

 private:
  cpp_name_resolver cpp_resolver_;
};

class python_capi_mstch_field : public mstch_field {
 public:
  python_capi_mstch_field(
      const t_field* field, mstch_context& ctx, mstch_element_position pos)
      : mstch_field(field, ctx, pos), py_name_(python::get_py3_name(*field)) {
    register_methods(
        this,
        {
            {"field:cpp_name", &python_capi_mstch_field::cpp_name},
            {"field:marshal_type", &python_capi_mstch_field::marshal_type},
            {"field:iobuf?", &python_capi_mstch_field::iobuf},
        });
  }

  mstch::node cpp_name() { return cpp2::get_name(field_); }

  mstch::node marshal_type() { return format_marshal_type(*field_); }

  mstch::node iobuf() {
    const auto* ttype = field_->get_type()->get_true_type();
    return ttype->is_binary() && is_type_iobuf(ttype);
  }

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
  void set_mstch_factories();
  void generate_file(
      const std::string& file, const std::filesystem::path& base);
  void generate_types();
  std::filesystem::path package_to_path();

  std::filesystem::path generate_root_path_;
};

void t_mstch_python_capi_generator::set_mstch_factories() {
  mstch_context_.add<python_capi_mstch_program>();
  mstch_context_.add<python_capi_mstch_struct>();
  mstch_context_.add<python_capi_mstch_field>();
  mstch_context_.add<python_capi_mstch_enum>();
}

std::filesystem::path t_mstch_python_capi_generator::package_to_path() {
  auto package = get_py3_namespace(get_program());
  return fmt::format("{}", fmt::join(package, "/"));
}

void t_mstch_python_capi_generator::generate_file(
    const std::string& file, const std::filesystem::path& base = {}) {
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
  generate_file("thrift_converter.pxd", generate_root_path_);
  generate_file("thrift_converter.pyx", generate_root_path_);
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_python_capi,
    "Python Capi",
    "    include_prefix:  Use full include paths in generated files.\n");

} // namespace apache::thrift::compiler
