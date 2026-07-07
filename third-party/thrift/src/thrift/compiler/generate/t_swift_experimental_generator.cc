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

#include <stdexcept>
#include <string>

#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/generate/swift/util.h>
#include <thrift/compiler/generate/t_whisker_generator.h>
#include <thrift/compiler/generate/templates.h>

namespace apache::thrift::compiler {

namespace {

namespace w = whisker::make;

// EXPERIMENTAL Swift (Apple Swift) data-types generator. Modeled on
// t_csharp_experimental_generator.cc: protocol-agnostic, data-types only,
// Whisker-based, targeting the vendored runtime at xplat/thrift/lib/swift/.
// Oncall: thrift_qna
class t_swift_EXPERIMENTAL_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  std::string template_prefix() const override { return "swift"; }

  whisker::source_manager template_source_manager() const final {
    return whisker::source_manager{
        std::make_unique<in_memory_source_manager_backend>(
            create_templates_by_path())};
  }

  void generate_program() override {
    out_dir_base_ = "gen-swift";
    const t_program* program = get_program();
    if (program == nullptr) {
      throw std::runtime_error("generate_program called without a program");
    }
    validate_no_synthesized_name_collisions(*program);
    auto filename = std::string(program->name()) + ".swift";
    render_to_file(
        /*output_file=*/filename,
        /*template_file=*/"module",
        /*context=*/w::map());
  }

 private:
  // The generator synthesizes Swift members that are not present in the Thrift
  // IDL and share the generated type's member namespace. A user-declared name
  // that matches one is a legal Thrift identifier but yields a duplicate Swift
  // declaration (a compile error), so reject such inputs up front with a clear
  // message instead of generating broken Swift. Synthesized members by kind:
  //   - enum: the `unknown(Int32)` case + the `name`/`rawValue` properties
  //   - union: the `_empty` case (other members are methods/initializers or
  //     overload cleanly with payload-carrying cases)
  //   - struct/exception: the `write(to:)` method (a stored property with the
  //     same base name conflicts; `init`/`init(from:)` are initializers)
  // At file scope, a `Constants` enum is synthesized when the program declares
  // constants, so a user type named `Constants` would also collide.
  // Only the root program is emitted; included types are referenced by
  // qualified name, so checking the root program suffices.
  static void validate_no_synthesized_name_collisions(
      const t_program& program) {
    for (const t_enum* e : program.enums()) {
      for (const t_enum_value& value : e->values()) {
        const std::string_view name = value.name();
        if (name == "unknown" || name == "name" || name == "rawValue") {
          throw std::runtime_error(
              "swift: enum '" + std::string(e->name()) +
              "' declares a value '" + std::string(name) +
              "' that collides with a synthesized Swift member "
              "(the 'unknown(Int32)' case, or the 'name'/'rawValue' "
              "properties); rename the enum value.");
        }
      }
    }
    for (const t_structured* s : program.structured_definitions()) {
      const bool is_union = s->is<t_union>();
      for (const t_field& field : s->fields()) {
        const std::string_view name = field.name();
        if (is_union && name == "_empty") {
          throw std::runtime_error(
              "swift: union '" + std::string(s->name()) +
              "' declares a field named '_empty', which collides with the "
              "synthesized '_empty' case; rename the field.");
        }
        if (!is_union && name == "write") {
          throw std::runtime_error(
              "swift: " +
              std::string(s->is<t_exception>() ? "exception" : "struct") +
              " '" + std::string(s->name()) +
              "' declares a field named 'write', which collides with the "
              "synthesized 'write(to:)' method; rename the field.");
        }
      }
    }
    if (!program.consts().empty()) {
      auto check_type_name = [](const t_named& type) {
        if (type.name() == "Constants") {
          throw std::runtime_error(
              "swift: type 'Constants' collides with the synthesized "
              "'Constants' enum that holds the program's constants; rename the "
              "type or move it to a program without constants.");
        }
      };
      for (const t_typedef* td : program.typedefs()) {
        check_type_name(*td);
      }
      for (const t_enum* e : program.enums()) {
        check_type_name(*e);
      }
      for (const t_structured* s : program.structured_definitions()) {
        check_type_name(*s);
      }
    }
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);

    def.property("swift_module", [](const t_program& self) {
      return swift::get_swift_module(self);
    });

    return std::move(def).make();
  }

  prototype<t_include>::ptr make_prototype_for_include(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_include(proto);
    auto def = whisker::dsl::prototype_builder<h_include>::extends(base);

    // The Swift module (namespace) of an included program, used to emit
    // `import <Module>` for cross-file references.
    def.property("swift_module", [](const t_include& self) {
      const t_program* prog = self.get_program();
      if (prog == nullptr) {
        return std::string();
      }
      return swift::get_swift_module(*prog);
    });

    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);

    // Type and enum-case names keep their original Thrift spelling, escaped
    // with backticks only when they collide with a Swift keyword.
    def.property("swift_name", [](const t_named& self) {
      return swift::get_swift_name(std::string(self.name()));
    });

    // Module-qualified name for cross-program (include) references: a type
    // defined in another Thrift program is referenced as `<Module>.<Name>`,
    // where <Module> is that program's Swift module (its `namespace swift`).
    // Same-program references stay unqualified.
    def.property("swift_qualified_name", [this](const t_named& self) {
      const t_program* prog = self.program();
      if (prog != nullptr && prog != get_program()) {
        return swift::get_swift_module(*prog) + "." +
            swift::get_swift_name(std::string(self.name()));
      }
      return swift::get_swift_name(std::string(self.name()));
    });

    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def = whisker::dsl::prototype_builder<h_field>::extends(base);

    def.property("swift_property_name", [](const t_field& self) {
      return swift::get_swift_property_name(std::string(self.name()));
    });

    return std::move(def).make();
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    // Swift-specific string escaping (\t, \n, \u{..}) — the base generator
    // emits octal escapes that Swift does not understand. Exposed under a
    // Swift-specific name rather than shadowing the base `string_value` so no
    // shared template accidentally receives Swift-escaped output.
    def.property("swift_string_value", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_STRING
          ? w::string(swift::escape_swift_string(self.get_string()))
          : w::null;
    });

    // A binary default value's bytes as an array of integers, for a Swift
    // `Data([...])` literal.
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

    return std::move(def).make();
  }
};

} // namespace

THRIFT_REGISTER_GENERATOR(
    swift_EXPERIMENTAL,
    "Swift",
    "EXPERIMENTAL support for Swift - use at your own risk. APIs can change without prior notice. All behavior should be considered undefined, unless explicitly specified otherwise.");

} // namespace apache::thrift::compiler
