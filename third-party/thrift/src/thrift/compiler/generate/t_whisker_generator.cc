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

#include <thrift/compiler/generate/t_whisker_generator.h>

#include <thrift/compiler/detail/system.h>
#include <thrift/compiler/generate/templates.h>
#include <thrift/compiler/sema/schematizer.h>
#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/parser.h>
#include <thrift/compiler/whisker/source_location.h>
#include <thrift/compiler/whisker/standard_library.h>

#include <cassert>
#include <cstddef>
#include <fstream>

#include <fmt/ranges.h>

#include <boost/algorithm/string/split.hpp>

namespace w = whisker::make;
using whisker::array;
using whisker::i64;
using whisker::map;
using whisker::object;
using whisker::prototype;
using whisker::prototype_database;
using whisker::string;

namespace dsl = whisker::dsl;
using dsl::function;
using dsl::prototype_builder;

namespace apache::thrift::compiler {

prototype<t_node>::ptr t_whisker_generator::make_prototype_for_node(
    const prototype_database&) const {
  prototype_builder<h_node> def;
  def.property("lineno", [&](const t_node& self) {
    auto loc = self.src_range().begin;
    return loc != source_location()
        ? i64(resolved_location(self.src_range().begin, source_mgr()).line())
        : i64(0);
  });
  return std::move(def).make();
}

prototype<t_named>::ptr t_whisker_generator::make_prototype_for_named(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_named>::extends(proto.of<t_node>());
  def.property("name", mem_fn(&t_named::name));
  def.property("scoped_name", mem_fn(&t_named::get_scoped_name));
  def.property("doc", mem_fn(&t_named::doc));
  def.property("program", mem_fn(&t_named::program, proto.of<t_program>()));
  def.property(
      "structured_annotations",
      mem_fn(&t_named::structured_annotations, proto.of<t_const>()));

  def.property("definition_key", [this](const t_named& named) {
    map::raw m;
    detail::schematizer s(*named.program()->global_scope(), source_mgr_, {});
    auto unescaped = s.identify_definition(named);
    std::string escaped;
    for (unsigned char chr : unescaped) {
      fmt::format_to(std::back_inserter(escaped), "\\x{:02x}", chr);
    }
    m["buffer"] = escaped;
    // NOTE: this is not the same as `string.len self.definition_key` because
    // of escape sequences!
    m["length"] = i64(detail::schematizer::definition_identifier_length());
    return map::of(std::move(m));
  });

  return std::move(def).make();
}

namespace {

template <typename T>
object t_type_as(const prototype_database& proto, const t_type& self) {
  static_assert(std::is_base_of_v<t_type, T>);
  return proto.create_nullable<T>(self.try_as<T>());
}

// When a t_type is bound to a native_handle for use within Whisker templates,
// we want to make sure that we attach the prototype of the most-derived t_type
// subclass. This allows the following usage pattern:
//
//   {{#if type.struct?}}
//     {{type.fields}}
//     ...
//
// The object `type.fields` is accessible after the check for `type.struct?`
// because this function will attach the prototype of t_struct.
object resolve_derived_t_type(
    const prototype_database& proto, const t_type& self) {
  if (self.is_typedef()) {
    // t_typedef::get_type_value() returns the underlying type value, which is
    // not useful for detecting t_typedef. So we handle it separately.
    return object(proto.create<t_typedef>(*self.try_as<t_typedef>()));
  }
  switch (self.get_type_value()) {
    case t_type::type::t_void:
    case t_type::type::t_bool:
    case t_type::type::t_byte:
    case t_type::type::t_i16:
    case t_type::type::t_i32:
    case t_type::type::t_i64:
    case t_type::type::t_float:
    case t_type::type::t_double:
    case t_type::type::t_string:
    case t_type::type::t_binary:
      return t_type_as<t_primitive_type>(proto, self);

    case t_type::type::t_list:
      return t_type_as<t_list>(proto, self);
    case t_type::type::t_set:
      return t_type_as<t_set>(proto, self);
    case t_type::type::t_map:
      return t_type_as<t_map>(proto, self);

    case t_type::type::t_enum:
      return t_type_as<t_enum>(proto, self);
    case t_type::type::t_structured: {
      if (auto union_ = self.try_as<t_union>()) {
        return object(proto.create<t_union>(*union_));
      }
      if (auto exception_ = self.try_as<t_exception>()) {
        return object(proto.create<t_exception>(*exception_));
      }
      if (auto struct_ = self.try_as<t_struct>()) {
        // All other t_struct subtypes (t_throws, t_paramlist) should be opaque
        // to Whisker to avoid additional tech debt.
        return object(proto.create<t_struct>(*struct_));
      }
      throw std::logic_error("Unknown t_structured subtype");
    }
    case t_type::type::t_service:
      // This is tech debt from a time before t_interaction was moved out of
      // t_type (for return types). This case should no longer happen in
      // practice.
      throw std::logic_error("t_type -> t_service is not supported");
    default:
      break;
  }
  throw std::logic_error("Unknown t_type subtype");
}

} // namespace

prototype<t_type>::ptr t_whisker_generator::make_prototype_for_type(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_type>::extends(proto.of<t_named>());
  // clang-format off
  def.property("void?",             mem_fn(&t_type::is_void));
  def.property("primitive?",        mem_fn(&t_type::is_primitive_type));
  def.property("string?",           mem_fn(&t_type::is_string));
  def.property("bool?",             mem_fn(&t_type::is_bool));
  def.property("byte?",             mem_fn(&t_type::is_byte));
  def.property("i16?",              mem_fn(&t_type::is_i16));
  def.property("i32?",              mem_fn(&t_type::is_i32));
  def.property("i64?",              mem_fn(&t_type::is_i64));
  def.property("float?",            mem_fn(&t_type::is_float));
  def.property("double?",           mem_fn(&t_type::is_double));
  def.property("typedef?",          mem_fn(&t_type::is_typedef));
  def.property("enum?",             mem_fn(&t_type::is_enum));
  def.property("struct?",           mem_fn(&t_type::is_struct));
  def.property("union?",            mem_fn(&t_type::is_union));
  def.property("exception?",        mem_fn(&t_type::is_exception));
  def.property("container?",        mem_fn(&t_type::is<t_container>));
  def.property("list?",             mem_fn(&t_type::is<t_list>));
  def.property("set?",              mem_fn(&t_type::is<t_set>));
  def.property("map?",              mem_fn(&t_type::is<t_map>));
  def.property("binary?",           mem_fn(&t_type::is_binary));
  def.property("string_or_binary?", mem_fn(&t_type::is_string_or_binary));
  def.property("any_int?",          mem_fn(&t_type::is_any_int));
  def.property("floating_point?",   mem_fn(&t_type::is_floating_point));
  def.property("scalar?",           mem_fn(&t_type::is_scalar));
  def.property("int_or_enum?",      mem_fn(&t_type::is_int_or_enum));
  // clang-format on

  def.property("full_name", mem_fn(&t_type::get_full_name));

  return std::move(def).make();
}

prototype<t_typedef>::ptr t_whisker_generator::make_prototype_for_typedef(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_typedef>::extends(proto.of<t_type>());
  def.property("resolved", [&](const t_typedef& self) {
    return resolve_derived_t_type(proto, self.type().deref());
  });
  return std::move(def).make();
}

prototype<t_structured>::ptr t_whisker_generator::make_prototype_for_structured(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_structured>::extends(proto.of<t_type>());
  def.property("fields", mem_fn(&t_structured::fields, proto.of<t_field>()));
  return std::move(def).make();
}

prototype<t_struct>::ptr t_whisker_generator::make_prototype_for_struct(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_struct>::extends(proto.of<t_structured>());
  return std::move(def).make();
}

prototype<t_paramlist>::ptr t_whisker_generator::make_prototype_for_paramlist(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_paramlist>::extends(proto.of<t_structured>());
  return std::move(def).make();
}

prototype<t_throws>::ptr t_whisker_generator::make_prototype_for_throws(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_throws>::extends(proto.of<t_structured>());
  return std::move(def).make();
}

prototype<t_union>::ptr t_whisker_generator::make_prototype_for_union(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_union>::extends(proto.of<t_structured>());
  return std::move(def).make();
}

prototype<t_exception>::ptr t_whisker_generator::make_prototype_for_exception(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_exception>::extends(proto.of<t_structured>());
  return std::move(def).make();
}

prototype<t_primitive_type>::ptr
t_whisker_generator::make_prototype_for_primitive_type(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_primitive_type>::extends(proto.of<t_type>());
  return std::move(def).make();
}

prototype<t_field>::ptr t_whisker_generator::make_prototype_for_field(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_field>::extends(proto.of<t_named>());
  def.property("id", [](const t_field& self) { return i64(self.id()); });
  def.property("type", [&](const t_field& self) {
    return resolve_derived_t_type(proto, self.type().deref());
  });
  def.property("default_value", [&](const t_field& self) {
    return proto.create_nullable<t_const_value>(self.get_default_value());
  });
  def.property("unqualified?", [&](const t_field& self) {
    return self.qualifier() == t_field_qualifier::none;
  });
  def.property("required?", [&](const t_field& self) {
    return self.qualifier() == t_field_qualifier::required;
  });
  def.property("optional?", [&](const t_field& self) {
    return self.qualifier() == t_field_qualifier::optional;
  });
  def.property("terse?", [&](const t_field& self) {
    return self.qualifier() == t_field_qualifier::terse;
  });
  return std::move(def).make();
}

prototype<t_enum>::ptr t_whisker_generator::make_prototype_for_enum(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_enum>::extends(proto.of<t_type>());
  def.property("values", mem_fn(&t_enum::values, proto.of<t_enum_value>()));
  return std::move(def).make();
}

prototype<t_enum_value>::ptr t_whisker_generator::make_prototype_for_enum_value(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_enum_value>::extends(proto.of<t_named>());
  def.property(
      "value", [](const t_enum_value& self) { return i64(self.get_value()); });
  return std::move(def).make();
}

prototype<t_const>::ptr t_whisker_generator::make_prototype_for_const(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_const>::extends(proto.of<t_named>());
  def.property("type", [&](const t_const& self) {
    return resolve_derived_t_type(proto, self.type_ref().deref());
  });
  def.property("value", [&](const t_const& self) {
    return proto.create<t_const_value>(*self.value());
  });
  return std::move(def).make();
}

prototype<t_const_value>::ptr
t_whisker_generator::make_prototype_for_const_value(
    const prototype_database&) const {
  prototype_builder<h_const_value> def;
  return std::move(def).make();
}

prototype<t_container>::ptr t_whisker_generator::make_prototype_for_container(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_container>::extends(proto.of<t_type>());
  return std::move(def).make();
}

prototype<t_map>::ptr t_whisker_generator::make_prototype_for_map(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_map>::extends(proto.of<t_container>());
  def.property("key_type", [&](const t_map& self) {
    return resolve_derived_t_type(proto, self.key_type().deref());
  });
  def.property("val_type", [&](const t_map& self) {
    return resolve_derived_t_type(proto, self.val_type().deref());
  });
  return std::move(def).make();
}

prototype<t_set>::ptr t_whisker_generator::make_prototype_for_set(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_set>::extends(proto.of<t_container>());
  def.property("elem_type", [&](const t_set& self) {
    return resolve_derived_t_type(proto, self.elem_type().deref());
  });
  return std::move(def).make();
}

prototype<t_list>::ptr t_whisker_generator::make_prototype_for_list(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_list>::extends(proto.of<t_container>());
  def.property("elem_type", [&](const t_list& self) {
    return resolve_derived_t_type(proto, self.elem_type().deref());
  });
  return std::move(def).make();
}

prototype<t_program>::ptr t_whisker_generator::make_prototype_for_program(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_program>::extends(proto.of<t_named>());
  def.property("package", mem_fn(&t_program::package, proto.of<t_package>()));
  def.property("doc", mem_fn(&t_program::doc));
  def.property("include_prefix", mem_fn(&t_program::include_prefix));
  def.property("includes", mem_fn(&t_program::includes, proto.of<t_include>()));
  def.property("namespaces", [&](const t_program& self) -> map::ptr {
    map::raw result;
    for (const auto& [language, value] : self.namespaces()) {
      result[language] = string(value);
    }
    return map::of(std::move(result));
  });
  def.function(
      "namespace_of", [&](const t_program& self, function::context ctx) {
        ctx.declare_arity(0);
        ctx.declare_named_arguments({"language"});
        return self.get_namespace(*ctx.named_argument<string>("language"));
      });

  def.property(
      "structured_definitions",
      mem_fn(&t_program::structured_definitions, proto.of<t_structured>()));
  def.property("services", mem_fn(&t_program::services, proto.of<t_service>()));
  def.property("typedefs", mem_fn(&t_program::typedefs, proto.of<t_typedef>()));
  def.property("enums", mem_fn(&t_program::enums, proto.of<t_enum>()));
  def.property("consts", mem_fn(&t_program::consts, proto.of<t_const>()));

  def.property("definition_key", [this](const t_program& self) {
    map::raw m;
    detail::schematizer s(*self.global_scope(), source_mgr_, {});
    auto id = std::to_string(s.identify_program(self));
    // NOTE: this overrides a property on t_named which is not the strlen,
    // but this is the same as the strlen. Provided for consistency to avoid
    // bugs when using the base implementation.
    m["length"] = i64(id.length());
    m["buffer"] = std::move(id);
    return map::of(std::move(m));
  });
  def.property("schema_name", [this](const t_program& self) {
    auto name = detail::schematizer::name_schema(source_mgr_, self);
    if (self.find(name)) {
      return object(std::move(name));
    }
    return object();
  });
  def.property("uris", [](const t_program& self) {
    array::raw result;
    for (const auto& def : self.structured_definitions()) {
      if (!def->uri().empty()) {
        result.emplace_back(def->uri());
      }
    }
    for (const auto& def : self.enums()) {
      if (!def->uri().empty()) {
        result.emplace_back(def->uri());
      }
    }
    return array::of(std::move(result));
  });
  return std::move(def).make();
}

prototype<t_package>::ptr t_whisker_generator::make_prototype_for_package(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_package>::extends(proto.of<t_node>());
  def.property("explicit?", mem_fn(&t_package::is_explicit));
  def.property("empty?", mem_fn(&t_package::empty));
  def.property("name", mem_fn(&t_package::name));
  return std::move(def).make();
}

prototype<t_include>::ptr t_whisker_generator::make_prototype_for_include(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_include>::extends(proto.of<t_node>());
  def.property("program", [&](const t_include& self) {
    return proto.create<t_program>(*self.get_program());
  });
  return std::move(def).make();
}

prototype<t_sink>::ptr t_whisker_generator::make_prototype_for_sink(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_sink>::extends(proto.of<t_node>());
  return std::move(def).make();
}

prototype<t_stream>::ptr t_whisker_generator::make_prototype_for_stream(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_stream>::extends(proto.of<t_node>());
  return std::move(def).make();
}

prototype<t_function>::ptr t_whisker_generator::make_prototype_for_function(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_function>::extends(proto.of<t_named>());

  def.property("params", mem_fn(&t_function::params, proto.of<t_paramlist>()));

  def.property("return_type", [&](const t_function& function) {
    const t_type* type = function.return_type().get_type();
    // Override the return type for compatibility with old codegen.
    if (function.is_interaction_constructor()) {
      // The old syntax (performs) treats an interaction as a response.
      type = function.interaction().get_type();
    } else if (function.sink_or_stream()) {
      type = &t_primitive_type::t_void();
    }
    return resolve_derived_t_type(proto, *type);
  });

  return std::move(def).make();
}

prototype<t_interface>::ptr t_whisker_generator::make_prototype_for_interface(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_interface>::extends(proto.of<t_type>());
  def.property(
      "functions", mem_fn(&t_interface::functions, proto.of<t_function>()));
  def.property("interaction?", mem_fn(&t_interface::is_interaction));
  return std::move(def).make();
}

prototype<t_service>::ptr t_whisker_generator::make_prototype_for_service(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_service>::extends(proto.of<t_interface>());
  def.property("extends", mem_fn(&t_service::extends, proto.of<t_service>()));
  return std::move(def).make();
}

prototype<t_interaction>::ptr
t_whisker_generator::make_prototype_for_interaction(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_interaction>::extends(proto.of<t_service>());
  return std::move(def).make();
}

void t_whisker_generator::define_prototypes(prototype_database& db) const {
  // WARNING: the order of these calls must be sorted with base classes first.
  // The derived classes require the base class prototypes to be defined first.
  //
  // As a reference, the `make_prototype_for_*` family of functions are declared
  // in the same order.
  db.define(make_prototype_for_node(db));
  db.define(make_prototype_for_named(db));

  db.define(make_prototype_for_type(db));
  db.define(make_prototype_for_typedef(db));
  db.define(make_prototype_for_structured(db));
  db.define(make_prototype_for_struct(db));
  db.define(make_prototype_for_paramlist(db));
  db.define(make_prototype_for_throws(db));
  db.define(make_prototype_for_union(db));
  db.define(make_prototype_for_exception(db));

  db.define(make_prototype_for_primitive_type(db));
  db.define(make_prototype_for_field(db));
  db.define(make_prototype_for_enum(db));
  db.define(make_prototype_for_enum_value(db));
  db.define(make_prototype_for_const(db));
  db.define(make_prototype_for_const_value(db));

  db.define(make_prototype_for_container(db));
  db.define(make_prototype_for_map(db));
  db.define(make_prototype_for_set(db));
  db.define(make_prototype_for_list(db));

  db.define(make_prototype_for_program(db));

  db.define(make_prototype_for_package(db));
  db.define(make_prototype_for_include(db));
  db.define(make_prototype_for_sink(db));
  db.define(make_prototype_for_stream(db));
  db.define(make_prototype_for_function(db));

  db.define(make_prototype_for_interface(db));
  db.define(make_prototype_for_service(db));
  db.define(make_prototype_for_interaction(db));

  define_additional_prototypes(db);
}

using fs_path = std::filesystem::path;

namespace {

bool is_last_char(std::string_view data, char c) {
  return !data.empty() && data.back() == c;
}

void chomp_last_char(std::string* data, char c) {
  if (is_last_char(*data, c)) {
    data->pop_back();
  }
}

} // namespace

class t_whisker_generator::whisker_source_parser
    : public whisker::source_resolver {
 public:
  explicit whisker_source_parser(
      templates_map templates_by_path, std::string template_prefix)
      : template_prefix_(std::move(template_prefix)),
        src_manager_(std::make_unique<in_memory_source_manager_backend>(
            std::move(templates_by_path))) {}

  resolve_import_result resolve_import(
      std::string_view combined_path,
      source_location include_from,
      diagnostics_engine& diags) override {
    std::vector<std::string> path_parts;
    boost::algorithm::split(
        path_parts, combined_path, [](char c) { return c == '/'; });
    std::string path = normalize_path(path_parts, include_from);

    if (auto cached = cached_asts_.find(path); cached != cached_asts_.end()) {
      if (!cached->second.has_value()) {
        return whisker::unexpected(parsing_error());
      }
      return &cached->second.value();
    }

    std::optional<source> source_code = src_manager_.get_file(path);
    if (!source_code.has_value()) {
      return nullptr;
    }
    auto ast = whisker::parse(*source_code, diags);
    auto [result, inserted] =
        cached_asts_.insert({std::move(path), std::move(ast)});
    assert(inserted);
    if (!result->second.has_value()) {
      return whisker::unexpected(parsing_error());
    }
    return &result->second.value();
  }

  whisker::source_manager& source_manager() { return src_manager_; }

 private:
  std::string normalize_path(
      const std::vector<std::string>& macro_path,
      source_location include_from) const {
    // The template_prefix will be added to the partial path, e.g.,
    // "field/member" --> "cpp2/field/member"
    std::string template_prefix;

    auto start = macro_path.begin();
    if (include_from == source_location()) {
      // If include_from is empty, we use the stored template_prefix
      template_prefix = template_prefix_;
    } else if (*start != "..") {
      fs_path current_file_path =
          resolved_location(include_from, src_manager_).file_name();
      template_prefix = current_file_path.begin()->generic_string();
    } else {
      // If path starts with "..", the template_prefix will be the second
      // element, and the template_name starts at the 3rd element. e.g.,
      // "../cpp2/field/member": template_prefix = "cpp2"
      ++start;
      template_prefix = *start++;
    }

    // Whisker always breaks down the path into components. However, the
    // template_map stores them as one concatenated string.
    return fmt::format(
        "{}/{}", template_prefix, fmt::join(start, macro_path.end(), "/"));
  }

  std::string template_prefix_;
  whisker::source_manager src_manager_;
  std::unordered_map<std::string, std::optional<whisker::ast::root>>
      cached_asts_;
};

/* static */ t_whisker_generator::templates_map
t_whisker_generator::create_templates_by_path() {
  templates_map result;
  for (std::size_t i = 0; i < templates_size; ++i) {
    auto name = fs_path(
        templates_name_datas[i],
        templates_name_datas[i] + templates_name_sizes[i]);
    name = name.parent_path() / name.stem();

    auto tpl = std::string(
        templates_content_datas[i],
        templates_content_datas[i] + templates_content_sizes[i]);
    // Remove a single '\n' or '\r\n' or '\r' at end, if present.
    chomp_last_char(&tpl, '\n');
    chomp_last_char(&tpl, '\r');
    result.emplace(name.generic_string(), std::move(tpl));
  }
  return result;
}

t_whisker_generator::cached_render_state& t_whisker_generator::render_state() {
  if (!cached_render_state_) {
    whisker::render_options options;

    auto source_resolver = std::make_shared<whisker_source_parser>(
        create_templates_by_path(), template_prefix());
    options.src_resolver = source_resolver;

    strictness_options strict = strictness();
    const auto level_for = [](bool strict) {
      return strict ? diagnostic_level::error : diagnostic_level::debug;
    };
    options.strict_boolean_conditional = level_for(strict.boolean_conditional);
    options.strict_printable_types = level_for(strict.printable_types);
    options.strict_undefined_variables = level_for(strict.undefined_variables);

    whisker::load_standard_library(options.globals);
    options.globals.merge(globals());

    auto prototypes = std::make_unique<prototype_database>();
    define_prototypes(*prototypes);

    cached_render_state_ = cached_render_state{
        whisker::diagnostics_engine(
            source_resolver->source_manager(),
            [](const diagnostic& d) { fmt::print(stderr, "{}\n", d); },
            diagnostic_params::only_errors()),
        source_resolver,
        std::move(options),
        std::move(prototypes),
    };
  }

  assert(cached_render_state_.has_value());
  return *cached_render_state_;
}

std::string t_whisker_generator::render(
    std::string_view template_file, const whisker::object& context) {
  cached_render_state& state = render_state();
  const whisker::ast::root& ast = whisker::visit(
      state.source_resolver->resolve_import(
          template_file, {}, state.diagnostic_engine),
      [&](const whisker::ast::root* resolved) -> const whisker::ast::root& {
        if (resolved == nullptr) {
          throw std::runtime_error{
              fmt::format("Failed to find template '{}'", template_file)};
        }
        return *resolved;
      },
      [&](const whisker::source_resolver::parsing_error&)
          -> const whisker::ast::root& {
        throw std::runtime_error{
            fmt::format("Failed to parse template '{}'", template_file)};
      });

  std::ostringstream out;
  if (!whisker::render(
          out, ast, context, state.diagnostic_engine, state.render_options)) {
    throw std::runtime_error{
        fmt::format("Failed to render template '{}'", template_file)};
  }
  return out.str();
}

void t_whisker_generator::write_to_file(
    const std::filesystem::path& output_file, std::string_view data) {
  auto abs_path = detail::make_abs_path(fs_path(get_out_dir()), output_file);
  std::filesystem::create_directories(abs_path.parent_path());

  {
    std::ofstream output{abs_path.string()};
    if (!output) {
      throw std::runtime_error(
          fmt::format("Could not open '{}' for writing.", abs_path.string()));
    }
    output << data;
    if (!is_last_char(data, '\n')) {
      // Terminate with newline.
      output << '\n';
    }
  }
  record_genfile(abs_path.string());
}

void t_whisker_generator::render_to_file(
    const std::filesystem::path& output_file,
    std::string_view template_file,
    const whisker::object& context) {
  write_to_file(output_file, render(template_file, context));
}

} // namespace apache::thrift::compiler
