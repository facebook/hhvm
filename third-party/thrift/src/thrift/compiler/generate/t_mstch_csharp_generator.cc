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

#include <string>

#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/generate/csharp/util.h>
#include <thrift/compiler/generate/t_whisker_generator.h>
#include <thrift/compiler/generate/templates.h>

namespace apache::thrift::compiler {

namespace {

namespace w = whisker::make;

// Oncall: content_dre
class t_csharp_EXPERIMENTAL_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  std::string template_prefix() const override { return "csharp"; }

  whisker::source_manager template_source_manager() const final {
    return whisker::source_manager{
        std::make_unique<in_memory_source_manager_backend>(
            create_templates_by_path())};
  }

  void generate_program() override {
    out_dir_base_ = "gen-csharp";
    auto filename = std::string(get_program()->name()) + ".cs";
    render_to_file(
        /*output_file=*/filename,
        /*template_file=*/"module",
        /*context=*/w::map());
  }

 private:
  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);

    def.property("csharp_namespace", [this](const t_program& self) {
      return csharp::get_csharp_namespace(self, diags_);
    });

    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def = whisker::dsl::prototype_builder<h_field>::extends(base);

    def.property("csharp_property_name", [](const t_field& self) {
      return "@" + csharp::get_csharp_property_name(std::string(self.name()));
    });

    def.property("is_message?", [](const t_field& self) {
      return self.name() == "message";
    });

    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    def.property("csharp_ttype_byte", [](const t_type& self) {
      return static_cast<int64_t>(csharp::get_csharp_ttype(&self));
    });

    def.property("is_nullable?", [](const t_type& self) {
      return csharp::is_csharp_nullable_type(&self);
    });

    // C# value-type primitive (excludes string/binary which are reference
    // types in C#). Distinct from base primitive? which includes all
    // t_primitive_type.
    def.property("csharp_primitive?", [](const t_type& self) {
      const auto* true_type = self.get_true_type();
      if (const auto* prim = true_type->try_as<t_primitive_type>()) {
        return !prim->is_string() && !prim->is_binary();
      }
      return false;
    });

    return std::move(def).make();
  }

  prototype<t_include>::ptr make_prototype_for_include(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_include(proto);
    auto def = whisker::dsl::prototype_builder<h_include>::extends(base);

    def.property("csharp_namespace", [this](const t_include& self) {
      return csharp::get_csharp_namespace(*self.get_program(), diags_);
    });

    def.property("alias", [](const t_include& self) {
      return std::string(self.get_program()->name());
    });

    def.property("is_useful_alias?", [this](const t_include& self) {
      std::string ns =
          csharp::get_csharp_namespace(*self.get_program(), diags_);
      std::string alias = std::string(self.get_program()->name());
      return ns != alias;
    });

    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);

    def.property("csharp_name", [](const t_named& self) {
      return "@" + std::string(self.name());
    });

    // Namespace-qualified name for cross-program type references.
    // Returns "Namespace.@TypeName" for external types, "@TypeName" for local.
    def.property("csharp_qualified_name", [this](const t_named& self) {
      if (self.program() != nullptr && self.program() != get_program()) {
        return csharp::get_csharp_namespace(*self.program(), diags_) + ".@" +
            std::string(self.name());
      }
      return "@" + std::string(self.name());
    });

    return std::move(def).make();
  }

  prototype<t_typedef>::ptr make_prototype_for_typedef(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_typedef(proto);
    auto def = whisker::dsl::prototype_builder<h_typedef>::extends(base);

    def.property("defined_kind?", [](const t_typedef& self) {
      return self.typedef_kind() == t_typedef::kind::defined;
    });

    return std::move(def).make();
  }

  prototype<t_const>::ptr make_prototype_for_const(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const(proto);
    auto def = whisker::dsl::prototype_builder<h_const>::extends(base);
    return std::move(def).make();
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    // Override string_value to use C#-specific escaping
    // The base generator uses get_escaped_string() which outputs octal escapes
    // that C# doesn't understand. We need \t, \n, \r, etc.
    def.property("string_value", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_STRING
          ? w::string(csharp::escape_csharp_string(self.get_string()))
          : w::null;
    });

    // Returns the string value as a comma-separated list of byte values.
    // For binary default values like "7", returns "55" (ASCII code for '7').
    // Used in templates for generating C# byte array initializers.
    def.property("byte_array_elements", [](const t_const_value& self) {
      if (self.kind() != t_const_value::CV_STRING) {
        return w::null;
      }
      const std::string& str = self.get_string();
      whisker::array::raw result;
      result.reserve(str.size());
      for (unsigned char c : str) {
        result.emplace_back(w::i64(static_cast<int64_t>(c)));
      }
      return w::array(std::move(result));
    });

    // Whisker's strict_printable_types only allows i64 and string to be
    // printed via {{...}} interpolation. f64, bool, and null require
    // explicit string conversion with language-specific formatting.
    def.property("bool_string", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_BOOL
          ? w::string(self.get_bool() ? "true" : "false")
          : w::null;
    });

    def.property("double_string", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_DOUBLE
          ? w::string(fmt::format("{}", self.get_double()))
          : w::null;
    });

    return std::move(def).make();
  }
};

} // namespace

THRIFT_REGISTER_GENERATOR(
    csharp_EXPERIMENTAL,
    "C#",
    "EXPERIMENTAL support for C# - use at your own risk. APIs can change without prior notice. All behavior should be considered undefined, unless explicitly specified otherwise.");

} // namespace apache::thrift::compiler
