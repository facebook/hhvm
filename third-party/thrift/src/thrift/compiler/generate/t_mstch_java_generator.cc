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
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <fmt/core.h>

#include <openssl/evp.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/java/util.h>
#include <thrift/compiler/generate/t_whisker_generator.h>
#include <thrift/compiler/generate/templates.h>

using namespace std;

namespace apache::thrift::compiler {

namespace {

/**
 * Gets the java namespace, throws a runtime error if not found.
 */
std::string get_namespace_or_default(const t_program& prog) {
  auto prog_namespace = fmt::format(
      "{}", fmt::join(prog.gen_namespace_or_default("java.swift", {}), "."));
  if (prog_namespace != "") {
    return prog_namespace;
  } else {
    throw std::runtime_error{"No namespace 'java.swift' in " + prog.name()};
  }
}

std::string get_constants_class_name(const t_program& prog) {
  const auto& constant_name = prog.get_namespace("java.swift.constants");
  if (constant_name == "") {
    return "Constants";
  } else {
    auto java_name_space = get_namespace_or_default(prog);
    std::string java_class_name;
    if (constant_name.rfind(java_name_space) == 0) {
      java_class_name = constant_name.substr(java_name_space.length() + 1);
    } else {
      java_class_name = constant_name;
    }

    if (java_class_name == "" ||
        java_class_name.find('.') != std::string::npos) {
      throw std::runtime_error{
          "Java Constants Class Name `" + java_class_name +
          "` is not well formatted."};
    }

    return java_class_name;
  }
}

template <typename Node>
std::string get_java_swift_name(const Node* node) {
  return node->get_unstructured_annotation(
      "java.swift.name", java::mangle_java_name(node->name(), false));
}

struct MdCtxDeleter {
  void operator()(EVP_MD_CTX* ctx) const { EVP_MD_CTX_free(ctx); }
};
using ctx_ptr = std::unique_ptr<EVP_MD_CTX, MdCtxDeleter>;

ctx_ptr newMdContext() {
  auto* ctx = EVP_MD_CTX_new();
  return ctx_ptr(ctx);
}

std::string hash(std::string st) {
  // Save an initialized context.
  static EVP_MD_CTX* kBase = []() {
    auto ctx = newMdContext();
    EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr);
    return ctx.release(); // Leaky singleton.
  }();

  // Copy the base context.
  auto ctx = newMdContext();
  EVP_MD_CTX_copy_ex(ctx.get(), kBase);
  // Digest the st.
  EVP_DigestUpdate(ctx.get(), st.data(), st.size());

  // Get the result.
  std::string result(EVP_MD_CTX_size(ctx.get()), 0);
  uint32_t size;
  EVP_DigestFinal_ex(
      ctx.get(), reinterpret_cast<uint8_t*>(result.data()), &size);
  assert(size == result.size()); // Should already be the correct size.
  result.resize(size);
  return result;
}

string toHex(const string& s) {
  ostringstream ret;

  unsigned int c;
  for (string::size_type i = 0; i < s.length(); ++i) {
    c = (unsigned int)(unsigned char)s[i];
    ret << hex << setfill('0') << setw(2) << c;
  }
  return ret.str().substr(0, 8);
}

// Cycle detection needed: e.g. GenericMap -> GenericMapValue -> GenericMap.
void collect_referenced_types(
    const t_type* type,
    std::set<std::string>& visited,
    std::vector<const t_structured*>& structs,
    std::vector<const t_enum*>& enums) {
  type = type->get_true_type();

  if (const auto* list = type->try_as<t_list>()) {
    collect_referenced_types(
        &list->elem_type().deref(), visited, structs, enums);
  } else if (const auto* set = type->try_as<t_set>()) {
    collect_referenced_types(
        &set->elem_type().deref(), visited, structs, enums);
  } else if (const auto* map = type->try_as<t_map>()) {
    collect_referenced_types(&map->key_type().deref(), visited, structs, enums);
    collect_referenced_types(&map->val_type().deref(), visited, structs, enums);
  } else if (const auto* enm = type->try_as<t_enum>()) {
    std::string fqn = fmt::format(
        "{}.{}",
        get_namespace_or_default(*type->program()),
        java::mangle_java_name(type->name(), true));
    if (visited.insert(fqn).second) {
      enums.push_back(enm);
    }
  } else if (const auto* strct = type->try_as<t_structured>()) {
    std::string fqn = fmt::format(
        "{}.{}",
        get_namespace_or_default(*type->program()),
        java::mangle_java_name(type->name(), true));
    if (visited.insert(fqn).second) {
      structs.push_back(strct);
      for (const auto& field : strct->fields()) {
        collect_referenced_types(
            field.type().get_type(), visited, structs, enums);
      }
    }
  }
}

void collect_types_from_service_functions(
    const t_interface& service,
    std::set<std::string>& visited,
    std::vector<const t_structured*>& structs,
    std::vector<const t_enum*>& enums) {
  for (const auto& func : service.functions()) {
    if (func.is_interaction_constructor()) {
      continue;
    }
    collect_referenced_types(
        func.return_type().get_type(), visited, structs, enums);
    for (const auto& field : func.params().fields()) {
      collect_referenced_types(
          field.type().get_type(), visited, structs, enums);
    }
    if (func.exceptions()) {
      for (const auto& field : func.exceptions()->fields()) {
        collect_referenced_types(
            field.type().get_type(), visited, structs, enums);
      }
    }
  }
}

whisker::object batch_whisker_array(
    whisker::array::raw items, size_t batch_size) {
  if (items.empty()) {
    return whisker::make::array(whisker::array::raw{});
  }
  whisker::array::raw batches;
  for (size_t i = 0; i < items.size(); i += batch_size) {
    whisker::array::raw batch;
    auto end = std::min(i + batch_size, items.size());
    for (size_t j = i; j < end; ++j) {
      batch.emplace_back(std::move(items[j]));
    }
    batches.emplace_back(whisker::make::array(std::move(batch)));
  }
  return whisker::make::array(std::move(batches));
}

constexpr size_t kValueBatchSize = 200;

struct service_referenced_types {
  std::vector<const t_structured*> structs;
  std::vector<const t_enum*> enums;
  std::vector<const t_structured*> exceptions;
};

service_referenced_types collect_service_referenced_types(
    const t_interface& service) {
  std::set<std::string> visited;
  std::vector<const t_structured*> all_structs;
  std::vector<const t_enum*> all_enums;
  collect_types_from_service_functions(
      service, visited, all_structs, all_enums);

  service_referenced_types result;
  for (const auto* s : all_structs) {
    if (s->is<t_exception>()) {
      result.exceptions.push_back(s);
    } else {
      result.structs.push_back(s);
    }
  }
  result.enums = std::move(all_enums);
  return result;
}

std::string compute_type_list_hash(const t_program& program) {
  std::string uri_concat;
  for (const auto* s : program.structured_definitions()) {
    auto uri = s->uri();
    if (!uri.empty()) {
      uri_concat += uri;
    }
  }
  for (const auto* e : program.enums()) {
    auto uri = e->uri();
    if (!uri.empty()) {
      uri_concat += uri;
    }
  }
  return toHex(
      hash(program.name() + get_namespace_or_default(program) + uri_concat));
}

std::string java_constant_name(std::string name) {
  std::string constant_str;
  bool is_first = true;
  bool was_previous_char_upper = false;
  for (auto iter = name.begin(); iter != name.end(); ++iter) {
    auto character = *iter;
    bool is_upper = isupper(character);
    if (is_upper && !is_first && !was_previous_char_upper) {
      constant_str += '_';
    }
    constant_str += toupper(character);
    is_first = false;
    was_previous_char_upper = is_upper;
  }
  return constant_str;
}

void collect_adapted_typedefs(
    const t_type* type,
    std::set<std::string>& seen,
    whisker::array::raw& result,
    const whisker::prototype_database& proto) {
  if (!type) {
    return;
  }
  if (const auto* td = type->try_as<t_typedef>()) {
    if (t_typedef::get_first_structured_annotation_or_null(
            type, kJavaAdapterUri) != nullptr) {
      if (seen.insert(td->name()).second) {
        result.emplace_back(resolve_derived_t_type(proto, *td));
      }
    }
    collect_adapted_typedefs(&td->type().deref(), seen, result, proto);
  } else if (const auto* list = type->try_as<t_list>()) {
    collect_adapted_typedefs(&list->elem_type().deref(), seen, result, proto);
  } else if (const auto* set = type->try_as<t_set>()) {
    collect_adapted_typedefs(&set->elem_type().deref(), seen, result, proto);
  } else if (const auto* map = type->try_as<t_map>()) {
    collect_adapted_typedefs(&map->key_type().deref(), seen, result, proto);
    collect_adapted_typedefs(&map->val_type().deref(), seen, result, proto);
  }
}

bool is_annotation_map_field_equal(
    const t_const* annotation, const char* field, const int value) {
  for (const auto& item : annotation->value()->get_map()) {
    if (item.first->get_string() == field) {
      return item.second->get_integer() == value;
    }
  }
  return annotation->value()->get_map().empty();
}

void validate_java_enum_intrinsic_default(
    sema_context& ctx, const t_enum& enm) {
  if (enm.has_structured_annotation(kJavaUseIntrinsicDefaultUri) &&
      enm.find_value(0) == nullptr) {
    ctx.error(
        enm,
        "Enum {} does not have a value for 0! You have to have value for 0 "
        "to use intrinsic default annotation.",
        enm.name());
  }
}

class t_mstch_java_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  void generate_program() override;

  void fill_validator_visitors(ast_validator& validator) const override {
    validator.add_enum_visitor(validate_java_enum_intrinsic_default);
  }

 private:
  std::string template_prefix() const override { return "java"; }

  whisker::source_manager template_source_manager() const final {
    return whisker::source_manager{
        std::make_unique<in_memory_source_manager_backend>(
            create_templates_by_path())};
  }

  strictness_options strictness() const override {
    return strictness_options{
        .boolean_conditional = false,
        .printable_types = false,
        .undefined_variables = true,
    };
  }

  void initialize_context(context_visitor& visitor) override {
    visitor.add_service_visitor([this](
                                    const whisker_generator_visitor_context&,
                                    const t_service& service) {
      service_types_map_[&service] = collect_service_referenced_types(service);
    });
  }

  const service_referenced_types& get_service_types(
      const t_interface& service) const {
    auto it = service_types_map_.find(&service);
    assert(it != service_types_map_.end());
    return it->second;
  }

  std::unordered_map<const t_interface*, service_referenced_types>
      service_types_map_;

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);

    def.property("java_name", [](const t_named& self) {
      return java::mangle_java_name(self.name(), true);
    });
    def.property("java_qualified_name", [](const t_named& self) {
      return fmt::format(
          "{}.{}",
          get_namespace_or_default(*self.program()),
          java::mangle_java_name(self.name(), true));
    });

    return std::move(def).make();
  }

  prototype<t_const>::ptr make_prototype_for_const(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const(proto);
    auto def = whisker::dsl::prototype_builder<h_const>::extends(base);

    def.property("javaCapitalName", [](const t_const& self) {
      return java::mangle_java_constant_name(self.name());
    });
    def.property("javaIgnoreConstant?", [](const t_const& self) {
      // we have to ignore constants if they are enums that we handled as ints,
      // as we don't have the constant values to work with.
      if (const t_map* map = self.type()->try_as<t_map>();
          map != nullptr && map->key_type()->is<t_enum>()) {
        return map->key_type()->has_unstructured_annotation(
            "java.swift.skip_enum_name_map");
      }
      if (const t_list* list = self.type()->try_as<t_list>();
          list != nullptr && list->elem_type()->is<t_enum>()) {
        return list->elem_type()->has_unstructured_annotation(
            "java.swift.skip_enum_name_map");
      }
      if (const t_set* set = self.type()->try_as<t_set>();
          set != nullptr && set->elem_type()->is<t_enum>()) {
        return set->elem_type()->has_unstructured_annotation(
            "java.swift.skip_enum_name_map");
      }
      // T194272441 generated schema const is rendered incorrectly.
      return self.generated();
    });

    return std::move(def).make();
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    def.property("quotedString", [](const t_const_value& self) {
      return java::quote_java_string(self.get_string());
    });

    return std::move(def).make();
  }

  prototype<t_enum>::ptr make_prototype_for_enum(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_enum(proto);
    auto def = whisker::dsl::prototype_builder<h_enum>::extends(base);
    def.property("java_enums_compat?", [](const t_enum& self) {
      return self.has_structured_annotation(kEnumsUri) ||
          self.program()->has_structured_annotation(kEnumsUri);
    });
    def.property("java_enum_type_open?", [&](const t_enum& self) {
      constexpr auto kEnumType = "type";
      constexpr int kOpenEnum = 1;
      if (const t_const* annotation =
              self.find_structured_annotation_or_null(kEnumsUri)) {
        return is_annotation_map_field_equal(annotation, kEnumType, kOpenEnum);
      }
      if (const t_const* annotation =
              self.program()->find_structured_annotation_or_null(kEnumsUri)) {
        return is_annotation_map_field_equal(annotation, kEnumType, kOpenEnum);
      }
      return false;
    });
    def.property("skipEnumNameMap?", [](const t_enum& self) {
      return self.has_unstructured_annotation("java.swift.skip_enum_name_map");
    });
    def.property("useIntrinsicDefault?", [](const t_enum& self) {
      if (self.has_structured_annotation(kJavaUseIntrinsicDefaultUri)) {
        if (self.find_value(0) == nullptr) {
          throw std::runtime_error(
              "Enum " + self.name() +
              " does not have a value for 0! You have to have value for 0 to use intrinsic default annotation.");
        }
        return true;
      }
      return false;
    });
    def.property("findValueZero", [](const t_enum& self) -> whisker::object {
      if (self.has_structured_annotation(kJavaUseIntrinsicDefaultUri)) {
        return whisker::make::string(
            java::mangle_java_constant_name(self.find_value(0)->name()));
      }
      return whisker::make::null;
    });
    def.property("value_batches", [&proto](const t_enum& self) {
      whisker::array::raw all_values;
      for (const auto& ev : self.values()) {
        all_values.emplace_back(proto.create<t_enum_value>(ev));
      }
      return batch_whisker_array(std::move(all_values), kValueBatchSize);
    });
    def.property("has_multiple_value_batches?", [](const t_enum& self) {
      return self.values().size() > kValueBatchSize;
    });
    return std::move(def).make();
  }

  prototype<t_enum_value>::ptr make_prototype_for_enum_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_enum_value(proto);
    auto def = whisker::dsl::prototype_builder<h_enum_value>::extends(base);
    def.property("javaConstantName", [](const t_enum_value& self) {
      return java::mangle_java_constant_name(self.name());
    });
    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def = whisker::dsl::prototype_builder<h_field>::extends(base);

    def.property("javaCapitalName", [](const t_field& self) {
      return java::mangle_java_name(get_java_swift_name(&self), true);
    });
    def.property(
        "negativeId?", [](const t_field& self) { return self.id() < 0; });
    def.property("isSensitive?", [](const t_field& self) {
      return self.has_unstructured_annotation("java.sensitive");
    });
    def.property("recursive?", [](const t_field& self) {
      return self.get_unstructured_annotation("swift.recursive_reference") ==
          "true" ||
          self.has_structured_annotation(kJavaRecursiveUri);
    });
    def.property("FieldNameUnmangled?", [](const t_field& self) {
      return self.has_structured_annotation(kJavaFieldUseUnmangledNameUri);
    });
    def.property("isNumericOrVoid?", [](const t_field& self) {
      auto type = self.type()->get_true_type();
      return type->is_void() || type->is_bool() || type->is_byte() ||
          type->is_i16() || type->is_i32() || type->is_i64() ||
          type->is_double() || type->is_float();
    });
    def.property("isNullableOrOptionalNotEnum?", [](const t_field& self) {
      if (self.qualifier() == t_field_qualifier::optional) {
        return true;
      }
      const t_type* field_type = self.type()->get_true_type();
      return !(
          field_type->is_bool() || field_type->is_byte() ||
          field_type->is_float() || field_type->is_i16() ||
          field_type->is_i32() || field_type->is_i64() ||
          field_type->is_double() || field_type->is<t_enum>());
    });
    def.property("hasInitialValue?", [](const t_field& self) {
      if (self.qualifier() == t_field_qualifier::optional) {
        return false;
      }
      return self.default_value() != nullptr;
    });
    def.property("javaAnnotations?", [](const t_field& self) {
      return self.has_structured_annotation(kJavaAnnotationUri);
    });
    def.property("javaAnnotations", [](const t_field& self) -> whisker::object {
      if (auto annotation =
              self.find_structured_annotation_or_null(kJavaAnnotationUri)) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == "java_annotation") {
            return whisker::make::string(item.second->get_string());
          }
        }
      }
      return whisker::make::null;
    });
    def.property("javaName", [](const t_field& self) {
      return get_java_swift_name(&self);
    });
    def.property("hasWrapper?", [](const t_field& self) {
      return self.has_structured_annotation(kJavaWrapperUri);
    });
    def.property("wrapper", [](const t_field& self) -> whisker::object {
      if (auto annotation =
              self.find_structured_annotation_or_null(kJavaWrapperUri)) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == "wrapperClassName") {
            return whisker::make::string(item.second->get_string());
          }
        }
      }
      return whisker::make::null;
    });
    def.property("wrapperType", [](const t_field& self) -> whisker::object {
      if (auto annotation =
              self.find_structured_annotation_or_null(kJavaWrapperUri)) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == "typeClassName") {
            return whisker::make::string(item.second->get_string());
          }
        }
      }
      return whisker::make::null;
    });
    def.property("hasFieldAdapter?", [](const t_field& self) {
      return self.has_structured_annotation(kJavaAdapterUri);
    });
    def.property(
        "fieldAdapterTypeClassName",
        [](const t_field& self) -> whisker::object {
          if (auto annotation =
                  self.find_structured_annotation_or_null(kJavaAdapterUri)) {
            for (const auto& item : annotation->value()->get_map()) {
              if (item.first->get_string() == "typeClassName") {
                return whisker::make::string(item.second->get_string());
              }
            }
          }
          return whisker::make::null;
        });
    def.property(
        "fieldAdapterClassName", [](const t_field& self) -> whisker::object {
          if (auto annotation =
                  self.find_structured_annotation_or_null(kJavaAdapterUri)) {
            for (const auto& item : annotation->value()->get_map()) {
              if (item.first->get_string() == "adapterClassName") {
                return whisker::make::string(item.second->get_string());
              }
            }
          }
          return whisker::make::null;
        });
    def.property("hasAdapter?", [](const t_field& self) {
      if (self.has_structured_annotation(kJavaAdapterUri)) {
        return true;
      }
      auto type = self.type().get_type();
      return type->is<t_typedef>() &&
          t_typedef::get_first_structured_annotation_or_null(
              type, kJavaAdapterUri) != nullptr;
    });
    def.property("hasAdapterOrWrapper?", [](const t_field& self) {
      if (self.has_structured_annotation(kJavaAdapterUri) ||
          self.has_structured_annotation(kJavaWrapperUri)) {
        return true;
      }
      auto type = self.type().get_type();
      return type->is<t_typedef>() &&
          t_typedef::get_first_structured_annotation_or_null(
              type, kJavaAdapterUri) != nullptr;
    });
    def.property("javaAllCapsName", [](const t_field& self) {
      auto field_name = self.name();
      boost::to_upper(field_name);
      return field_name;
    });
    def.property("javaConstantName", [](const t_field& self) {
      return java_constant_name(get_java_swift_name(&self));
    });
    def.property("javaTFieldName", [](const t_field& self) {
      return java_constant_name(get_java_swift_name(&self)) + "_FIELD_DESC";
    });
    def.property("typeFieldName", [](const t_field& self) {
      auto type_name = self.type()->get_true_type()->get_full_name();
      return java::mangle_java_name(type_name, true);
    });
    def.property("java_strings_compat?", [this](const t_field& self) {
      if (self.has_structured_annotation(kStringsUri) ||
          t_typedef::get_first_structured_annotation_or_null(
              self.type().get_type(), kStringsUri) != nullptr) {
        return true;
      }
      const t_structured* parent = context().get_field_parent(&self);
      assert(parent != nullptr);
      return parent->has_structured_annotation(kStringsUri) ||
          parent->program()->has_structured_annotation(kStringsUri);
    });
    def.property(
        "java_coding_error_action_report?", [this](const t_field& self) {
          constexpr auto kOnInvalidUtf8 = "onInvalidUtf8";
          constexpr int kActionReport = 1;

          if (const t_const* annotation =
                  self.find_structured_annotation_or_null(kStringsUri)) {
            return is_annotation_map_field_equal(
                annotation, kOnInvalidUtf8, kActionReport);
          }
          if (const t_const* annotation =
                  t_typedef::get_first_structured_annotation_or_null(
                      self.type().get_type(), kStringsUri)) {
            return is_annotation_map_field_equal(
                annotation, kOnInvalidUtf8, kActionReport);
          }
          const t_structured* parent = context().get_field_parent(&self);
          if (parent == nullptr) {
            return false;
          }
          if (const t_const* annotation =
                  parent->find_structured_annotation_or_null(kStringsUri)) {
            return is_annotation_map_field_equal(
                annotation, kOnInvalidUtf8, kActionReport);
          }
          if (const t_const* annotation =
                  parent->program()->find_structured_annotation_or_null(
                      kStringsUri)) {
            return is_annotation_map_field_equal(
                annotation, kOnInvalidUtf8, kActionReport);
          }
          return false;
        });

    return std::move(def).make();
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);
    def.property("javaPackage", &get_namespace_or_default);
    def.property("constantClassName", &get_constants_class_name);
    def.property("type_list_batches", [&proto](const t_program& self) {
      constexpr int32_t BATCH_SIZE = 512;
      whisker::array::raw all_types;
      for (const auto* s : self.structured_definitions()) {
        if (!s->uri().empty()) {
          all_types.emplace_back(resolve_derived_t_type(proto, *s));
        }
      }
      for (const auto* e : self.enums()) {
        if (!e->uri().empty()) {
          all_types.emplace_back(resolve_derived_t_type(proto, *e));
        }
      }
      whisker::array::raw batches;
      for (size_t i = 0; i < all_types.size(); i += BATCH_SIZE) {
        whisker::array::raw batch;
        auto end =
            std::min(i + static_cast<size_t>(BATCH_SIZE), all_types.size());
        for (size_t j = i; j < end; ++j) {
          batch.emplace_back(std::move(all_types[j]));
        }
        batches.emplace_back(whisker::make::array(std::move(batch)));
      }
      return whisker::make::array(std::move(batches));
    });
    def.property("type_list_hash", [](const t_program& self) {
      return compute_type_list_hash(self);
    });
    return std::move(def).make();
  }

  prototype<t_interface>::ptr make_prototype_for_interface(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interface(proto);
    auto def = whisker::dsl::prototype_builder<h_interface>::extends(base);
    def.property("function_batches", [&proto](const t_interface& self) {
      constexpr int32_t BATCH_SIZE = 100;
      whisker::array::raw all_funcs;
      for (const auto& func : self.functions()) {
        if (!func.is_interaction_constructor()) {
          all_funcs.emplace_back(proto.create<t_function>(func));
        }
      }
      whisker::array::raw batches;
      for (size_t i = 0; i < all_funcs.size(); i += BATCH_SIZE) {
        whisker::array::raw batch;
        auto end =
            std::min(i + static_cast<size_t>(BATCH_SIZE), all_funcs.size());
        for (size_t j = i; j < end; ++j) {
          batch.emplace_back(std::move(all_funcs[j]));
        }
        batches.emplace_back(whisker::make::array(std::move(batch)));
      }
      return whisker::make::array(std::move(batches));
    });

    def.property("referenced_structs", [this, &proto](const t_interface& self) {
      const auto& types = get_service_types(self);
      whisker::array::raw result;
      for (const auto* s : types.structs) {
        result.emplace_back(proto.create<t_structured>(*s));
      }
      return whisker::make::array(std::move(result));
    });

    def.property("referenced_enums", [this, &proto](const t_interface& self) {
      const auto& types = get_service_types(self);
      whisker::array::raw result;
      for (const auto* e : types.enums) {
        result.emplace_back(proto.create<t_enum>(*e));
      }
      return whisker::make::array(std::move(result));
    });

    def.property(
        "referenced_exceptions", [this, &proto](const t_interface& self) {
          const auto& types = get_service_types(self);
          whisker::array::raw result;
          for (const auto* s : types.exceptions) {
            result.emplace_back(proto.create<t_structured>(*s));
          }
          return whisker::make::array(std::move(result));
        });

    def.property(
        "referenced_struct_batches", [this, &proto](const t_interface& self) {
          const auto& types = get_service_types(self);
          whisker::array::raw items;
          for (const auto* s : types.structs) {
            items.emplace_back(proto.create<t_structured>(*s));
          }
          return batch_whisker_array(std::move(items), 20);
        });

    def.property(
        "referenced_enum_batches", [this, &proto](const t_interface& self) {
          const auto& types = get_service_types(self);
          whisker::array::raw items;
          for (const auto* e : types.enums) {
            items.emplace_back(proto.create<t_enum>(*e));
          }
          return batch_whisker_array(std::move(items), 1);
        });

    def.property(
        "referenced_exception_batches",
        [this, &proto](const t_interface& self) {
          const auto& types = get_service_types(self);
          whisker::array::raw items;
          for (const auto* s : types.exceptions) {
            items.emplace_back(proto.create<t_structured>(*s));
          }
          return batch_whisker_array(std::move(items), 50);
        });

    def.property("oneway_functions", [&proto](const t_interface& self) {
      whisker::array::raw funcs;
      for (const auto& func : self.functions()) {
        if (func.qualifier() == t_function_qualifier::oneway) {
          funcs.emplace_back(proto.create<t_function>(func));
        }
      }
      return whisker::make::array(std::move(funcs));
    });

    def.property(
        "request_response_functions", [&proto](const t_interface& self) {
          whisker::array::raw funcs;
          for (const auto& func : self.functions()) {
            if (!func.sink_or_stream() && !func.is_interaction_constructor() &&
                func.qualifier() != t_function_qualifier::oneway) {
              funcs.emplace_back(proto.create<t_function>(func));
            }
          }
          return whisker::make::array(std::move(funcs));
        });

    def.property("single_request_functions", [&proto](const t_interface& self) {
      whisker::array::raw funcs;
      for (const auto& func : self.functions()) {
        if (!func.sink_or_stream() && !func.is_interaction_constructor()) {
          funcs.emplace_back(proto.create<t_function>(func));
        }
      }
      return whisker::make::array(std::move(funcs));
    });

    def.property("streaming_functions", [&proto](const t_interface& self) {
      whisker::array::raw funcs;
      for (const auto& func : self.functions()) {
        if (func.stream() && !func.is_bidirectional_stream()) {
          funcs.emplace_back(proto.create<t_function>(func));
        }
      }
      return whisker::make::array(std::move(funcs));
    });

    def.property("sink_functions", [&proto](const t_interface& self) {
      whisker::array::raw funcs;
      for (const auto& func : self.functions()) {
        if (func.sink() && !func.is_bidirectional_stream()) {
          funcs.emplace_back(proto.create<t_function>(func));
        }
      }
      return whisker::make::array(std::move(funcs));
    });

    return std::move(def).make();
  }

  prototype<t_function>::ptr make_prototype_for_function(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_function(proto);
    auto def = whisker::dsl::prototype_builder<h_function>::extends(base);
    def.property("java_name", [](const t_function& self) {
      return java::mangle_java_name(self.name(), false);
    });
    def.property("voidType", [](const t_function& self) {
      return self.return_type()->is_void() && !self.stream();
    });
    return std::move(def).make();
  }

  prototype<t_structured>::ptr make_prototype_for_structured(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_structured(proto);
    auto def = whisker::dsl::prototype_builder<h_structured>::extends(base);

    def.property("unionFieldTypeUnique?", [](const t_structured& self) {
      std::set<std::string> field_types;
      for (const auto& field : self.fields()) {
        std::string type_name = field.type()->get_true_type()->get_full_name();
        std::string_view type_with_erasure =
            std::string_view{type_name}.substr(0, type_name.find('<'));
        if (!field_types.insert(std::string(type_with_erasure)).second) {
          return false;
        }
      }
      return true;
    });

    def.property("unique_adapted_typedefs", [&proto](const t_structured& self) {
      std::set<std::string> seen;
      whisker::array::raw result;
      for (const auto& field : self.fields()) {
        collect_adapted_typedefs(&field.type().deref(), seen, result, proto);
      }
      return whisker::make::array(std::move(result));
    });

    def.property("asBean?", [](const t_structured& self) {
      return self.is<t_struct>() &&
          (self.get_unstructured_annotation("java.swift.mutable") == "true" ||
           self.has_structured_annotation(kJavaMutableUri));
    });
    def.property("hasTerseField?", [](const t_structured& self) {
      for (const auto& field : self.fields()) {
        if (field.qualifier() == t_field_qualifier::terse) {
          return true;
        }
      }
      return false;
    });
    def.property("hasWrapper?", [](const t_structured& self) {
      for (const auto& field : self.fields()) {
        if (field.has_structured_annotation(kJavaWrapperUri)) {
          return true;
        }
      }
      return false;
    });

    def.property("isBigStruct?", [](const t_structured& self) {
      static constexpr uint64_t bigStructThreshold = 127;
      return (self.is<t_struct>() || self.is<t_union>()) &&
          self.fields().size() > bigStructThreshold;
    });

    def.property("javaAnnotations?", [](const t_structured& self) {
      return self.has_structured_annotation(kJavaAnnotationUri);
    });
    def.property(
        "javaAnnotations", [](const t_structured& self) -> std::string {
          if (auto annotation =
                  self.find_structured_annotation_or_null(kJavaAnnotationUri)) {
            for (const auto& item : annotation->value()->get_map()) {
              if (item.first->get_string() == "java_annotation") {
                return item.second->get_string();
              }
            }
          }
          return "";
        });

    def.property("needsExceptionMessage?", [](const t_structured& self) {
      return self.is<t_exception>() &&
          dynamic_cast<const t_exception&>(self).get_message_field() !=
          nullptr &&
          self.get_field_by_name("message") == nullptr;
    });
    def.property(
        "exceptionMessage", [](const t_structured& self) -> std::string {
          const auto* message_field =
              dynamic_cast<const t_exception&>(self).get_message_field();
          return get_java_swift_name(message_field);
        });

    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    def.property("javaCapitalName", [](const t_type& self) {
      return java::mangle_java_name(self.name(), true);
    });
    def.property("hasAdapter?", [](const t_type& self) {
      return self.is<t_typedef>() &&
          t_typedef::get_first_structured_annotation_or_null(
              &self, kJavaAdapterUri) != nullptr;
    });
    def.property("java_type?", [](const t_type& self) {
      auto* val = t_typedef::get_first_unstructured_annotation_or_null(
          &self, {"java.swift.type"});
      return val != nullptr && !val->empty();
    });
    def.property("java_type", [](const t_type& self) {
      auto* val = t_typedef::get_first_unstructured_annotation_or_null(
          &self, {"java.swift.type"});
      return val ? *val : "";
    });
    def.property("isBinaryString?", [](const t_type& self) {
      if (t_typedef::get_first_structured_annotation_or_null(
              &self, kJavaBinaryStringUri) != nullptr) {
        return true;
      }
      return t_typedef::get_first_unstructured_annotation_or_null(
                 &self, {"java.swift.binary_string"}) != nullptr;
    });
    def.property("adapterClassName", [](const t_type& self) {
      if (const t_const* annotation =
              t_typedef::get_first_structured_annotation_or_null(
                  &self, kJavaAdapterUri);
          annotation != nullptr && self.is<t_typedef>()) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == "adapterClassName") {
            return whisker::make::string(item.second->get_string());
          }
        }
      }
      return whisker::make::null;
    });
    def.property("adapterTypeClassName", [](const t_type& self) {
      if (const t_const* annotation =
              t_typedef::get_first_structured_annotation_or_null(
                  &self, kJavaAdapterUri);
          annotation != nullptr && self.is<t_typedef>()) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == "typeClassName") {
            return whisker::make::string(item.second->get_string());
          }
        }
      }
      return whisker::make::null;
    });
    def.property("enumSkipNameMap?", [](const t_type& self) {
      if (self.get_true_type()->is<t_enum>()) {
        return self.get_true_type()->has_unstructured_annotation(
            "java.swift.skip_enum_name_map");
      }
      return false;
    });
    def.property("lastAdapter?", [](const t_type& self) {
      int32_t count = 0;
      auto* type = &self;
      while (type) {
        if (type->is<t_typedef>() &&
            type->has_structured_annotation(kJavaAdapterUri)) {
          count++;
          if (const auto* as_typedef = type->try_as<t_typedef>()) {
            type = &as_typedef->type().deref();
          } else {
            break;
          }
        } else {
          break;
        }
      }
      return count < 2;
    });

    return std::move(def).make();
  }

  /*
   * Generate multiple Java items according to the given template. Writes
   * output to package_dir underneath the global output directory.
   */
  void generate_rpc_interfaces() {
    const t_program* program = get_program();
    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = has_compiler_option("separate_data_type_from_services")
        ? "services" / raw_package_dir
        : raw_package_dir;

    for (const t_service* service : program->services()) {
      auto filename = java::mangle_java_name(service->name(), true) + ".java";
      whisker::object context = whisker::make::map({
          {"service",
           whisker::make::native_handle(
               render_state().prototypes->create<t_service>(*service))},
      });
      render_to_file(package_dir / filename, "Service", context);
    }
  }

  template <typename T>
  void generate_items(
      const t_program* program,
      const std::vector<T*>& items,
      const std::string& tpl_path,
      const std::string& context_key) {
    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = has_compiler_option("separate_data_type_from_services")
        ? "data-type" / raw_package_dir
        : raw_package_dir;

    for (const T* item : items) {
      auto classname = java::mangle_java_name(item->name(), true);
      auto filename = classname + ".java";
      whisker::object context = whisker::make::map({
          {context_key,
           resolve_derived_t_type(*render_state().prototypes, *item)},
      });
      render_to_file(package_dir / filename, tpl_path, context);
    }
  }

  /*
   * Generate Service Client implementation - Sync & Async. Writes
   * output to package_dir
   */
  void render_service_to_file(
      const t_service& service,
      const std::filesystem::path& package_dir,
      const std::string& tpl_path,
      const std::string& filename) {
    whisker::object context = whisker::make::map({
        {"service",
         whisker::make::native_handle(
             render_state().prototypes->create<t_service>(service))},
    });
    render_to_file(package_dir / filename, tpl_path, context);
  }

  void generate_services() {
    const t_program* program = get_program();
    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};

    auto package_dir = has_compiler_option("separate_data_type_from_services")
        ? "services" / raw_package_dir
        : raw_package_dir;

    for (const t_service* service : program->services()) {
      auto service_name = java::mangle_java_name(service->name(), true);
      if (has_compiler_option("deprecated_allow_leagcy_reflection_client")) {
        render_service_to_file(
            *service,
            package_dir,
            "deprecated/ServiceClient",
            service_name + "ClientImpl.java");
        render_service_to_file(
            *service,
            package_dir,
            "deprecated/ServiceAsyncClient",
            service_name + "AsyncClientImpl.java");
      }

      render_service_to_file(
          *service,
          package_dir,
          "AsyncReactiveWrapper",
          service_name + "AsyncReactiveWrapper.java");
      render_service_to_file(
          *service,
          package_dir,
          "BlockingReactiveWrapper",
          service_name + "BlockingReactiveWrapper.java");
      render_service_to_file(
          *service,
          package_dir,
          "ReactiveAsyncWrapper",
          service_name + "ReactiveAsyncWrapper.java");
      render_service_to_file(
          *service,
          package_dir,
          "ReactiveBlockingWrapper",
          service_name + "ReactiveBlockingWrapper.java");
      render_service_to_file(
          *service,
          package_dir,
          "ReactiveClient",
          service_name + "ReactiveClient.java");
      render_service_to_file(
          *service,
          package_dir,
          "RpcServerHandler",
          service_name + "RpcServerHandler.java");
      render_service_to_file(
          *service,
          package_dir,
          "ThriftMetadataHandler",
          service_name + "ThriftMetadataHandler.java");
      render_service_to_file(
          *service,
          package_dir,
          "ThriftMetadataHandlerEnums",
          service_name + "ThriftMetadataHandlerEnums.java");
      render_service_to_file(
          *service,
          package_dir,
          "ThriftMetadataHandlerStructs",
          service_name + "ThriftMetadataHandlerStructs.java");
      render_service_to_file(
          *service,
          package_dir,
          "ThriftMetadataHandlerExceptions",
          service_name + "ThriftMetadataHandlerExceptions.java");
    }
  }

  void generate_constants(const t_program* program) {
    if (program->consts().empty()) {
      return;
    }
    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = has_compiler_option("separate_data_type_from_services")
        ? "data-type" / raw_package_dir
        : raw_package_dir;

    auto constant_file_name = get_constants_class_name(*program) + ".java";
    render_to_file(
        package_dir / constant_file_name, "Constants", whisker::make::null);
  }

  void generate_placeholder(const t_program* program) {
    auto package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto placeholder_file_name = ".generated_" + program->name();
    if (has_compiler_option("separate_data_type_from_services")) {
      write_to_file("data-type" / package_dir / placeholder_file_name, "");
      write_to_file("services" / package_dir / placeholder_file_name, "");
    } else {
      write_to_file(package_dir / placeholder_file_name, "");
    }
  }

  void generate_type_list(const t_program* program) {
    auto java_namespace = get_namespace_or_default(*program);
    auto raw_package_dir =
        std::filesystem::path{java::package_to_path(java_namespace)};
    auto package_dir = has_compiler_option("separate_data_type_from_services")
        ? "data-type" / raw_package_dir
        : raw_package_dir;

    auto list_hash = compute_type_list_hash(*program);
    std::string file_name =
        fmt::format("__fbthrift_TypeList_{}.java", list_hash);
    render_to_file(package_dir / file_name, "TypeList", whisker::make::null);
  }
};

void t_mstch_java_generator::generate_program() {
  out_dir_base_ = "gen-java";

  generate_items(
      get_program(), get_program()->structured_definitions(), "Object", "self");
  generate_rpc_interfaces();
  generate_services();
  generate_items(get_program(), get_program()->enums(), "Enum", "self");
  generate_constants(get_program());
  generate_placeholder(get_program());
  generate_type_list(get_program());
}

} // namespace

THRIFT_REGISTER_GENERATOR(mstch_java, "Java", "");

} // namespace apache::thrift::compiler
