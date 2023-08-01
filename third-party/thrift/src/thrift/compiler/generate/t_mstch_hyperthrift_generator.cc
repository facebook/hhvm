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
#include <iomanip>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <thrift/compiler/lib/java/util.h>

#include <openssl/evp.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/gen/cpp/type_resolver.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

/**
 * Gets the java namespace, throws a runtime error if not found.
 */
std::string get_namespace_or_default(const t_program& prog) {
  auto hyperthrift_namespace = boost::algorithm::join(
      prog.gen_namespace_or_default("hyperthrift", {}), ".");
  if (hyperthrift_namespace != "") {
    return hyperthrift_namespace;
  }
  // Fallback to java namespace if hyperthrift namespace is not found.
  auto java_namespace =
      boost::algorithm::join(prog.gen_namespace_or_default("java", {}), ".");
  if (java_namespace != "") {
    return java_namespace;
  }

  throw std::runtime_error{
      "No namespace 'hyperthrift' or 'java' in " + prog.name()};
}

template <typename Node>
mstch::node get_structed_annotation_attribute(
    const Node* node, const char* uri, const std::string& key) {
  if (auto annotation = node->find_structured_annotation_or_null(uri)) {
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == key) {
        return item.second->get_string();
      }
    }
  }

  return nullptr;
}

template <typename Node>
std::string get_java_swift_name(const Node* node) {
  return node->get_annotation(
      "java.swift.name", java::mangle_java_name(node->get_name(), false));
}

std::string str_type_list = "";
std::string type_list_hash = "";

struct type_mapping {
  std::string uri;
  std::string className;
};

std::vector<type_mapping> type_list;

std::string prevTypeName;

} // namespace

class t_mstch_hyperthrift_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "hyperthrift"; }

  void generate_program() override;

 private:
  void set_mstch_factories();

  template <typename T, typename Factory, typename Cache>
  void generate_items(
      const Factory& factory,
      Cache& c,
      const t_program* program,
      const std::vector<T*>& items,
      const std::string& tpl_path) {
    const auto& id = program->path();
    if (!mstch_context_.program_cache.count(id)) {
      mstch_context_.program_cache[id] =
          mstch_context_.program_factory->make_mstch_object(
              program, mstch_context_);
    }
    auto raw_package_dir = boost::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = "java" / raw_package_dir;
    std::string package_name = get_namespace_or_default(*program_);

    for (const T* item : items) {
      auto classname = java::mangle_java_name(item->get_name(), true);
      auto filename = classname + ".java";
      const auto& item_id = id + item->get_name();
      if (!c.count(item_id)) {
        c[item_id] = factory.make_mstch_object(item, mstch_context_);
      }

      render_to_file(c[item_id], tpl_path, package_dir / filename);
    }
  }

  void generate_constants(const t_program* program) {
    if (program->consts().empty()) {
      // Only generate Constants.java if we actually have constants
      return;
    }
    auto name = program->name();
    const auto& prog = cached_program(program);

    auto raw_package_dir = boost::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = "java" / raw_package_dir;
    auto constant_file_name = "Constants.java";
    render_to_file(prog, "constants", package_dir / constant_file_name);
  }
};

class mstch_hyperthrift_program : public mstch_program {
  static constexpr int32_t BATCH_SIZE = 512;

 public:
  mstch_hyperthrift_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:javaPackage", &mstch_hyperthrift_program::java_package},
        });
  }
  mstch::node java_package() { return get_namespace_or_default(*program_); }
};

class mstch_hyperthrift_struct : public mstch_struct {
  // A struct is a "big struct" if it contains > 127 members. The reason for
  // this limit is that we generate exhaustive constructor for Thrift struct
  // but Java methods are limited to 255 arguments (and since long/double
  // types count twice, 127 is a safe threshold).
  static constexpr uint64_t bigStructThreshold = 127;

 public:
  mstch_hyperthrift_struct(
      const t_struct* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos) {
    register_methods(
        this,
        {
            {"struct:javaPackage", &mstch_hyperthrift_struct::java_package},
            {"struct:javaCapitalName",
             &mstch_hyperthrift_struct::java_capital_name},
            {"struct:shouldGenerateBuilder",
             &mstch_hyperthrift_struct::has_builder},
            {"struct:numFields", &mstch_hyperthrift_struct::num_fields},
        });
  }
  mstch::node java_package() {
    return get_namespace_or_default(*struct_->program());
  }
  mstch::node java_capital_name() {
    return java::mangle_java_name(struct_->get_name(), true);
  }
  // default generating builder for now
  mstch::node has_builder() { return true; }
  mstch::node num_fields() { return (int)struct_->get_members().size(); }
};

class mstch_hyperthrift_field : public mstch_field {
 public:
  mstch_hyperthrift_field(
      const t_field* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context)
      : mstch_field(f, ctx, pos, field_context) {
    register_methods(
        this,
        {
            {"field:javaName", &mstch_hyperthrift_field::java_name},
            {"field:javaCapitalName",
             &mstch_hyperthrift_field::java_capital_name},
            {"field:javaDefaultValue",
             &mstch_hyperthrift_field::java_default_value},
            {"field:javaAllCapsName",
             &mstch_hyperthrift_field::java_all_caps_name},
            {"field:javaTFieldName",
             &mstch_hyperthrift_field::java_tfield_name},
            {"field:isNullableOrOptionalNotEnum?",
             &mstch_hyperthrift_field::is_nullable_or_optional_not_enum},
            {"field:isEnum?", &mstch_hyperthrift_field::is_enum},
            {"field:isObject?", &mstch_hyperthrift_field::is_object},
            {"field:isUnion?", &mstch_hyperthrift_field::is_union},
            {"field:isContainer?", &mstch_hyperthrift_field::is_container},
            {"field:typeFieldName", &mstch_hyperthrift_field::type_field_name},
            {"field:isSensitive?", &mstch_hyperthrift_field::is_sensitive},
            {"field:hasInitialValue?",
             &mstch_hyperthrift_field::has_initial_value},
        });
  }
  mstch::node get_typedef_structed_annotation_attribute(
      const char* uri, const std::string& field) {
    auto type = field_->get_type();
    if (type->is_typedef()) {
      if (auto annotation =
              t_typedef::get_first_structured_annotation_or_null(type, uri)) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == field) {
            return item.second->get_string();
          }
        }
      }
    }

    return nullptr;
  }

  mstch::node has_initial_value() {
    if (field_->get_req() == t_field::e_req::optional) {
      // default values are ignored for optional fields
      return false;
    }
    return field_->get_value();
  }
  mstch::node is_nullable_or_optional_not_enum() {
    if (field_->get_req() == t_field::e_req::optional) {
      return true;
    }
    const t_type* field_type = field_->get_type()->get_true_type();
    return !(
        field_type->is_bool() || field_type->is_byte() ||
        field_type->is_float() || field_type->is_i16() ||
        field_type->is_i32() || field_type->is_i64() ||
        field_type->is_double() || field_type->is_enum());
  }

  mstch::node is_enum() {
    const t_type* field_type = field_->get_type()->get_true_type();
    return field_type->is_enum();
  }

  mstch::node is_object() {
    const t_type* field_type = field_->get_type()->get_true_type();
    return field_type->is_struct() || field_type->is_exception() ||
        field_type->is_union();
  }

  mstch::node is_union() {
    const t_type* field_type = field_->get_type()->get_true_type();
    return field_type->is_union();
  }

  mstch::node is_container() {
    return field_->get_type()->get_true_type()->is_container();
  }
  mstch::node java_name() { return get_java_swift_name(field_); }

  mstch::node type_field_name() {
    auto type_name = field_->get_type()->get_true_type()->get_full_name();
    return java::mangle_java_name(type_name, true);
  }

  mstch::node java_tfield_name() {
    return constant_name(field_->get_name()) + "_FIELD_DESC";
  }
  mstch::node java_capital_name() {
    return java::mangle_java_name(
        field_->get_annotation("java.swift.name", &field_->get_name()), true);
  }
  mstch::node java_all_caps_name() {
    auto field_name = field_->get_name();
    boost::to_upper(field_name);
    return field_name;
  }
  mstch::node java_default_value() { return default_value_for_field(field_); }
  std::string default_value_for_field(const t_field* field) {
    if (field_->get_req() == t_field::e_req::optional) {
      return "null";
    }
    return default_value_for_type(field->get_type());
  }
  std::string default_value_for_type(const t_type* type) {
    if (type->is_typedef()) {
      auto typedef_type = dynamic_cast<const t_typedef*>(type)->get_type();
      return default_value_for_type(typedef_type);
    } else {
      if (type->is_byte() || type->is_i16() || type->is_i32()) {
        return "0";
      } else if (type->is_i64()) {
        return "0L";
      } else if (type->is_float()) {
        return "0.f";
      } else if (type->is_double()) {
        return "0.";
      } else if (type->is_bool()) {
        return "false";
      } else if (type->is_enum()) {
        // we use fromInteger(0) as default value as it may be null or the enum
        // entry for 0.
        auto javaNamespace = get_namespace_or_default(*(type->program()));
        auto enumType = java::mangle_java_name(type->get_name(), true);
        return javaNamespace + "." + enumType + ".fromInteger(0)";
      }
      return "null";
    }
  }

  mstch::node is_sensitive() {
    return field_->has_annotation("java.sensitive");
  }
  std::string constant_name(std::string name) {
    std::string constant_str;

    bool is_first = true;
    bool was_previous_char_upper = false;
    for (std::string::iterator iter = name.begin(); iter != name.end();
         ++iter) {
      std::string::value_type character = (*iter);
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
};

class mstch_hyperthrift_enum : public mstch_enum {
 public:
  mstch_hyperthrift_enum(
      const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum(e, ctx, pos) {
    register_methods(
        this,
        {
            {"enum:javaPackage", &mstch_hyperthrift_enum::java_package},
            {"enum:javaCapitalName",
             &mstch_hyperthrift_enum::java_capital_name},
        });
  }
  mstch::node java_package() {
    return get_namespace_or_default(*enum_->program());
  }
  mstch::node java_capital_name() {
    return java::mangle_java_name(enum_->get_name(), true);
  }
};

class mstch_hyperthrift_enum_value : public mstch_enum_value {
 public:
  mstch_hyperthrift_enum_value(
      const t_enum_value* ev, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum_value(ev, ctx, pos) {
    register_methods(
        this,
        {
            {"enum_value:javaConstantName",
             &mstch_hyperthrift_enum_value::java_constant_name},
        });
  }
  mstch::node java_constant_name() {
    return java::mangle_java_constant_name(enum_value_->get_name());
  }
};

class mstch_hyperthrift_const : public mstch_const {
 public:
  mstch_hyperthrift_const(
      const t_const* c,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      const t_field* field)
      : mstch_const(c, ctx, pos, current_const, expected_type, field) {
    register_methods(
        this,
        {
            {"constant:javaCapitalName",
             &mstch_hyperthrift_const::java_capital_name},
            {"constant:javaFieldName",
             &mstch_hyperthrift_const::java_field_name},
        });
  }
  mstch::node java_capital_name() {
    return java::mangle_java_constant_name(const_->get_name());
  }
  mstch::node java_field_name() {
    return java::mangle_java_name(field_->get_name(), true);
  }
};

class mstch_hyperthrift_const_value : public mstch_const_value {
 public:
  mstch_hyperthrift_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type) {
    register_methods(
        this,
        {
            {"value:quotedString",
             &mstch_hyperthrift_const_value::quote_java_string},
            {"value:javaEnumValueName",
             &mstch_hyperthrift_const_value::java_enum_value_name},
        });
  }
  mstch::node quote_java_string() {
    return java::quote_java_string(const_value_->get_string());
  }
  mstch::node java_enum_value_name() {
    if (type_ == cv::CV_INTEGER && const_value_->is_enum()) {
      const t_enum_value* enum_value = const_value_->get_enum_value();
      if (enum_value != nullptr) {
        return java::mangle_java_constant_name(enum_value->get_name());
      }
      return "fromInteger(" + std::to_string(const_value_->get_integer()) + ")";
    }
    return mstch::node();
  }
};

class mstch_hyperthrift_type : public mstch_type {
 public:
  mstch_hyperthrift_type(
      const t_type* t, mstch_context& ctx, mstch_element_position pos)
      : mstch_type(t, ctx, pos) {
    register_methods(
        this,
        {
            {"type:isContainer?", &mstch_hyperthrift_type::is_container_type},
            {"type:javaType", &mstch_hyperthrift_type::java_type},
            {"type:setIsMapKey", &mstch_hyperthrift_type::set_is_map_key},
            {"type:isMapKey?", &mstch_hyperthrift_type::get_map_key_flag},
            {"type:setIsMapValue", &mstch_hyperthrift_type::set_is_map_value},
            {"type:isMapValue?", &mstch_hyperthrift_type::get_map_value_flag},
            {"type:isBinaryString?", &mstch_hyperthrift_type::is_binary_string},
            {"type:setIsNotMap", &mstch_hyperthrift_type::set_is_not_map},
        });
  }
  bool isMapValueFlag = false;
  bool isMapKeyFlag = false;
  bool hasTypeAdapter = false;

  mstch::node set_is_not_map() {
    isMapValueFlag = false;
    isMapKeyFlag = false;
    return mstch::node();
  }
  mstch::node get_map_value_flag() { return isMapValueFlag; }
  mstch::node get_map_key_flag() { return isMapKeyFlag; }
  mstch::node set_is_map_value() {
    isMapValueFlag = true;
    return mstch::node();
  }
  mstch::node set_is_map_key() {
    isMapKeyFlag = true;
    return mstch::node();
  }

  mstch::node is_container_type() {
    return type_->get_true_type()->is_container();
  }

  mstch::node java_type() {
    return type_->get_true_type()->get_annotation("java.swift.type");
  }
  mstch::node is_binary_string() {
    return type_->get_true_type()->get_annotation("java.swift.binary_string");
  }

  mstch::node has_structured_annotation(const char* uri) {
    if (type_->is_typedef()) {
      if (auto annotation =
              t_typedef::get_first_structured_annotation_or_null(type_, uri)) {
        return true;
      }
    }
    return false;
  }
};

void t_mstch_hyperthrift_generator::generate_program() {
  out_dir_base_ = "gen-hyperthrift";
  set_mstch_factories();

  auto name = get_program()->name();
  const auto& id = get_program()->path();
  if (!mstch_context_.program_cache.count(id)) {
    mstch_context_.program_cache[id] =
        mstch_context_.program_factory->make_mstch_object(
            get_program(), mstch_context_);
  }

  str_type_list = "";
  type_list_hash = "";
  generate_items(
      *mstch_context_.struct_factory,
      mstch_context_.struct_cache,
      get_program(),
      get_program()->objects(),
      "struct");
  generate_items(
      *mstch_context_.enum_factory,
      mstch_context_.enum_cache,
      get_program(),
      get_program()->enums(),
      "enum");
  generate_constants(get_program());
}

void t_mstch_hyperthrift_generator::set_mstch_factories() {
  mstch_context_.add<mstch_hyperthrift_program>();
  mstch_context_.add<mstch_hyperthrift_type>();
  mstch_context_.add<mstch_hyperthrift_struct>();
  mstch_context_.add<mstch_hyperthrift_field>();
  mstch_context_.add<mstch_hyperthrift_enum>();
  mstch_context_.add<mstch_hyperthrift_enum_value>();
  mstch_context_.add<mstch_hyperthrift_const>();
  mstch_context_.add<mstch_hyperthrift_const_value>();
}

THRIFT_REGISTER_GENERATOR(mstch_hyperthrift, "HyperThrift", "");

} // namespace compiler
} // namespace thrift
} // namespace apache
