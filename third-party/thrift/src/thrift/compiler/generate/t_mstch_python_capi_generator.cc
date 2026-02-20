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
#include <optional>
#include <set>
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
                            field->type().get_type(), uri);
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
    return field->type().get_type();
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
    const auto* elem_type = list->elem_type().get_type();
    return format_unary_type(node, elem_type, "list", type_override);
  } else if (const t_set* set = true_type->try_as<t_set>()) {
    const auto* elem_type = set->elem_type().get_type();
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

bool is_capi_eligible_type_annotation(const t_const* annotation) {
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

bool is_capi_eligible_type(const t_type* type) {
  if (type->has_structured_annotation(kCppAdapterUri)) {
    return false;
  }

  if (const auto* cpp_type_anno =
          type->find_structured_annotation_or_null(kCppTypeUri)) {
    if (!is_capi_eligible_type_annotation(cpp_type_anno)) {
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
  if (const std::string* type_anno = type->find_unstructured_annotation_or_null(
          {"cpp.type", "cpp2.type"})) {
    return is_type_iobuf(*type_anno);
  }
  if (const t_list* list = type->try_as<t_list>();
      list != nullptr && !is_capi_eligible_type(list->elem_type().get_type())) {
    return false;
  } else if (const t_set* set = type->try_as<t_set>(); set != nullptr &&
             !is_capi_eligible_type(set->elem_type().get_type())) {
    return false;
  } else if (const t_map* map = type->try_as<t_map>(); map != nullptr &&
             (!is_capi_eligible_type(&map->key_type().deref()) ||
              !is_capi_eligible_type(&map->val_type().deref()))) {
    return false;
  }
  if (const t_typedef* tdef = type->try_as<t_typedef>()) {
    return is_capi_eligible_type(&tdef->type().deref());
  }
  return true;
}

bool is_capi_eligible_field(const t_field& field) {
  if (field.has_structured_annotation(kCppAdapterUri)) {
    return false;
  }
  if (const auto* cpp_type_anno =
          field.find_structured_annotation_or_null(kCppTypeUri)) {
    return is_capi_eligible_type_annotation(cpp_type_anno);
  }
  return true;
}

bool has_types(const t_program& program) {
  return !program.structured_definitions().empty() || !program.enums().empty();
}

class python_capi_generator_context {
 public:
  python_capi_generator_context(
      const t_program* root_program, bool serialize_python_capi)
      : root_program_{root_program} {
    has_marshal_types_ = !serialize_python_capi && has_types(*root_program);
  }

  const std::set<std::string>& capi_includes() const { return capi_includes_; }

  void register_visitors(t_whisker_generator::context_visitor& visitor) {
    if (!has_marshal_types_) {
      return;
    }

    using context = t_whisker_generator::whisker_generator_visitor_context;
    // Gather capi includes for included programs
    visitor.add_program_visitor([this](const context&, const t_program& p) {
      if (&p != root_program_) {
        return;
      }
      for (const t_program* included_program : p.get_includes_for_codegen()) {
        if (!included_program->structured_definitions().empty() ||
            !included_program->enums().empty() ||
            !included_program->typedefs().empty()) {
          capi_includes_.insert(get_capi_include(included_program, &p));
        }
      }
    });
    visitor.add_field_visitor([&](const context& ctx, const t_field& f) {
      if (&ctx.program() == root_program_) {
        visit_type(*f.type(), false);
      }
    });
    visitor.add_typedef_visitor([&](const context& ctx, const t_typedef& td) {
      if (&ctx.program() == root_program_) {
        visit_type(*td.type(), false);
      }
    });
  }

 private:
  const t_program* root_program_;
  bool has_marshal_types_;
  // Ordered set to ensure deterministic output/builds
  std::set<std::string> capi_includes_;

  void visit_type(const t_type& orig_type, bool has_typedef) {
    const t_type& true_type = *orig_type.get_true_type();
    has_typedef = has_typedef || orig_type.is<t_typedef>();
    if (has_typedef && true_type.program() != nullptr &&
        true_type.program() != root_program_) {
      capi_includes_.insert(
          get_capi_include(true_type.program(), root_program_));
    }

    if (const t_list* list = true_type.try_as<t_list>()) {
      visit_type(*list->elem_type(), has_typedef);
    } else if (const t_set* set = true_type.try_as<t_set>()) {
      visit_type(*set->elem_type(), has_typedef);
    } else if (const t_map* map = true_type.try_as<t_map>()) {
      visit_type(*map->key_type(), has_typedef);
      visit_type(*map->val_type(), has_typedef);
    }
  }
};

class t_mstch_python_capi_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "python_capi"; }

  void generate_program() override { generate_types(); }

 private:
  void generate_file(
      const std::string& file, const std::filesystem::path& base);
  void generate_types();
  std::filesystem::path package_to_path();

  std::filesystem::path generate_root_path_;

  // Mutable as it contains caches, but needs to be accessed from `const`
  // contexts
  mutable cpp_name_resolver cpp_resolver_;
  std::unique_ptr<python_capi_generator_context> python_capi_context_;

  void initialize_context(context_visitor& visitor) override {
    generate_root_path_ = package_to_path();
    out_dir_base_ = "gen-python-capi";
    if (std::string_view include_prefix =
            get_compiler_option("include_prefix").value_or("");
        !include_prefix.empty()) {
      program_->set_include_prefix(std::string(include_prefix));
    }

    python_capi_context_ = std::make_unique<python_capi_generator_context>(
        program_,
        /*serialize_python_capi=*/has_compiler_option("serialize_python_capi"));
    python_capi_context_->register_visitors(visitor);
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def =
        whisker::dsl::prototype_builder<h_named>::extends(std::move(base));

    def.property(
        "cpp_name", [](const t_named& self) { return cpp2::get_name(&self); });
    def.property("py_name", &python::get_py3_name);

    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def =
        whisker::dsl::prototype_builder<h_field>::extends(std::move(base));

    def.property("marshal_type", [](const t_field& self) {
      return format_marshal_type(self);
    });
    def.property("iobuf?", [](const t_field& self) {
      const t_type* ttype = self.type()->get_true_type();
      return ttype->is_binary() && is_type_iobuf(ttype);
    });

    return std::move(def).make();
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def =
        whisker::dsl::prototype_builder<h_program>::extends(std::move(base));

    def.property("capi_includes", [this](const t_program& self) {
      if (&self != program_) {
        throw whisker::eval_error(
            "capi_includes is only valid on root program");
      }
      return whisker::array::of(
          {python_capi_context_->capi_includes().begin(),
           python_capi_context_->capi_includes().end()});
    });
    def.property("capi_module_prefix", [](const t_program& self) {
      return gen_capi_module_prefix(&self);
    });
    def.property("cpp_namespace", &cpp2::get_gen_namespace);
    def.property("generate_capi?", &has_types);
    def.property("module_path", [this](const t_program& self) {
      return get_py3_namespace_with_name_and_prefix(
          &self,
          std::string(get_compiler_option("root_module_prefix").value_or("")));
    });

    return std::move(def).make();
  }

  prototype<t_structured>::ptr make_prototype_for_structured(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_structured(proto);
    auto def =
        whisker::dsl::prototype_builder<h_structured>::extends(std::move(base));

    // Override of `cpp_name` for structured types
    def.property("cpp_name", [this](const t_structured& self) {
      return cpp_resolver_.get_underlying_namespaced_name(self);
    });
    def.property("num_fields", [](const t_structured& self) {
      return whisker::make::i64(static_cast<int64_t>(self.fields().size()));
    });
    def.property("cpp_adapter_name", [](const t_structured& self) {
      if (const t_const* adapter_annotation =
              self.find_structured_annotation_or_null(kCppAdapterUri)) {
        return whisker::make::string(
            get_annotation_property(adapter_annotation, "name"));
      }
      return whisker::make::null;
    });

    def.property("marshal_capi?", [this](const t_structured& self) {
      const t_const* marshal_override =
          self.find_structured_annotation_or_null(kPythonUseCAPIUri);
      bool force_serialize = false;
      if (marshal_override != nullptr) {
        if (const t_const_value* serialize_field =
                marshal_override->get_value_from_structured_annotation_or_null(
                    "serialize")) {
          force_serialize = serialize_field->get_bool();
        }
      }

      if (force_serialize || self.generated() ||
          has_compiler_option("serialize_python_capi")) {
        return false;
      }
      if (marshal_override != nullptr ||
          has_compiler_option("marshal_python_capi")) {
        return true;
      }
      for (const t_field& f : self.fields()) {
        if (!is_capi_eligible_field(f) ||
            !is_capi_eligible_type(&f.type().deref())) {
          return false;
        }
      }
      return true;
    });

    def.property("tuple_positions", [](const t_structured& self) {
      std::unordered_map<const t_field*, int64_t> field_tuple_indexes;
      // Determine 1-based (0 is isset indicator) thrift-python tuple index for
      // each field
      int64_t tuple_index = 1;
      for (const t_field* f : self.fields_id_order()) {
        field_tuple_indexes[f] = tuple_index++;
      }

      // Now populate the tuple position array in definition order (to match C++
      // index)
      whisker::array::raw a;
      for (const t_field& f : self.fields()) {
        a.emplace_back(whisker::make::i64(field_tuple_indexes.at(&f)));
      }
      return whisker::make::array(std::move(a));
    });

    return std::move(def).make();
  }
};

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
