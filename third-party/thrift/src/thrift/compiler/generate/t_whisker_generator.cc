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

#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/type_visitor.h>
#include <thrift/compiler/detail/system.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/util.h>
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
        ? i64(source_mgr().resolve_location(self.src_range().begin).line())
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
  def.property("docs?", mem_fn(&t_named::has_doc));
  def.property("program", mem_fn(&t_named::program, proto.of<t_program>()));
  def.property(
      "structured_annotations",
      mem_fn(&t_named::structured_annotations, proto.of<t_const>()));
  def.property("uri", mem_fn(&t_named::uri));

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
  return self.visit(
      [&](const t_typedef& typedef_) -> object {
        return object(proto.create<t_typedef>(typedef_));
      },
      [&](const t_primitive_type& primitive) -> object {
        return object(proto.create<t_primitive_type>(primitive));
      },
      [&](const t_list& list) -> object {
        return object(proto.create<t_list>(list));
      },
      [&](const t_set& set) -> object {
        return object(proto.create<t_set>(set));
      },
      [&](const t_map& map) -> object {
        return object(proto.create<t_map>(map));
      },
      [&](const t_enum& enum_) -> object {
        return object(proto.create<t_enum>(enum_));
      },
      [&](const t_union& union_) -> object {
        return object(proto.create<t_union>(union_));
      },
      [&](const t_exception& exception_) -> object {
        return object(proto.create<t_exception>(exception_));
      },
      [&](const t_struct& struct_) -> object {
        // All other t_struct subtypes (t_throws, t_paramlist) should be opaque
        // to Whisker to avoid additional tech debt.
        return object(proto.create<t_struct>(struct_));
      },
      [&](const t_service&) -> object {
        // This is tech debt from a time before t_interaction was moved out of
        // t_type (for return types). This case should no longer happen in
        // practice.
        throw std::logic_error("t_type -> t_service is not supported");
      });
}

// Compute the set of types that appear anywhere in the service
// definition as input or output types. This presents maps, lists etc as seen
// in declarations, but unpacks the payloads of sinks and streams.
whisker::array::ptr build_user_type_footprint(
    const t_service& service, const prototype_database& prototype_database) {
  std::vector<const t_type*> types;
  std::unordered_set<const t_type*> seen;

  // Helper to extract the necesary types from a single type identified in
  // some component of a method declaration.  Deals with maps, lists, streams
  // that surround actual types.
  auto extract_type = [&](const t_type* type) -> void {
    // Maintain insertion order for stable output.
    // Insert into types in order of detection (parsing), use "seen"
    // to avoid duplicates.
    if (seen.count(type) == 0) {
      types.emplace_back(type);
      seen.insert(type);
    }
  };

  // Go through each method declaration and identfiy the places that could
  // contain user defined types.
  std::deque<const t_function*> pending;
  for (const auto& function : service.functions()) {
    pending.emplace_back(&function);
  }
  while (!pending.empty()) {
    const auto& function = *pending.front();
    pending.erase(pending.begin());
    for (const auto& param : function.params().fields()) {
      extract_type(param.get_type());
    }
    if (const auto& excs = function.exceptions();
        !t_throws::is_null_or_empty(excs)) {
      for (auto& ex : excs->fields()) {
        extract_type(ex.get_type());
      }
    }

    extract_type(&function.return_type().deref());
    if (auto& type = function.interaction()) {
      if (auto* srv_type =
              dynamic_cast<const t_service*>(type->get_true_type())) {
        for (const auto& intfunc : srv_type->functions()) {
          pending.emplace_back(&intfunc);
        }
        continue;
      }
    }
    if (function.sink()) {
      extract_type(&function.sink()->elem_type().deref());
      if (!function.sink()->final_response_type().empty()) {
        extract_type(&function.sink()->final_response_type().deref());
      }
    }
    if (function.stream()) {
      extract_type(&function.stream()->elem_type().deref());
    }
  }
  whisker::array::raw ret;
  for (const t_type* typeptr : types) {
    // This line below should be this:
    // auto obj = resolve_derived_t_type(prototype_database, *typeptr);
    //
    // resolve_derived_t_type() does not produce the correct result for
    // right now - the right result being a match of the type names
    // used in service stub definitions, due to problems with inconsistent
    // behavior of the cpp_type property across different implementations/types.
    auto obj = prototype_database.create<t_type>(*typeptr);
    ret.emplace_back(std::move(obj));
  }
  return whisker::array::of(std::move(ret));
}

// mem_fn wrapper checking whether a collection is non-empty
template <typename Self>
auto is_non_empty_collection(auto (Self::*function)() const) {
  return [function](const Self& self) -> whisker::object {
    return whisker::object(!(self.*function)().empty());
  };
}

// mem_fn over a t_type, but after resolving any typedefs first
template <typename R>
static auto true_type_mem_fn(R (t_type::*function)() const) {
  return [function](const t_type& self) -> whisker::object {
    return whisker::object(
        std::decay_t<R>((self.get_true_type()->*function)()));
  };
}

} // namespace

prototype<t_type>::ptr t_whisker_generator::make_prototype_for_type(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_type>::extends(proto.of<t_named>());
  // clang-format off
  def.property("typedef?",          mem_fn(&t_type::is<t_typedef>));

  // Operations which resolve any typedefs first, before evaluating
  def.property("void?",             true_type_mem_fn(&t_type::is_void));
  def.property("primitive?",        true_type_mem_fn(&t_type::is<t_primitive_type>));
  def.property("string?",           true_type_mem_fn(&t_type::is_string));
  def.property("bool?",             true_type_mem_fn(&t_type::is_bool));
  def.property("byte?",             true_type_mem_fn(&t_type::is_byte));
  def.property("i16?",              true_type_mem_fn(&t_type::is_i16));
  def.property("i32?",              true_type_mem_fn(&t_type::is_i32));
  def.property("i64?",              true_type_mem_fn(&t_type::is_i64));
  def.property("float?",            true_type_mem_fn(&t_type::is_float));
  def.property("double?",           true_type_mem_fn(&t_type::is_double));
  def.property("enum?",             true_type_mem_fn(&t_type::is<t_enum>));
  def.property("structured?",       true_type_mem_fn(&t_type::is<t_structured>));
  def.property("struct?",           true_type_mem_fn(&t_type::is<t_struct>));
  def.property("union?",            true_type_mem_fn(&t_type::is<t_union>));
  def.property("exception?",        true_type_mem_fn(&t_type::is<t_exception>));
  def.property("container?",        true_type_mem_fn(&t_type::is<t_container>));
  def.property("list?",             true_type_mem_fn(&t_type::is<t_list>));
  def.property("set?",              true_type_mem_fn(&t_type::is<t_set>));
  def.property("map?",              true_type_mem_fn(&t_type::is<t_map>));
  def.property("binary?",           true_type_mem_fn(&t_type::is_binary));
  def.property("string_or_binary?", true_type_mem_fn(&t_type::is_string_or_binary));
  def.property("any_int?",          true_type_mem_fn(&t_type::is_any_int));
  def.property("floating_point?",   true_type_mem_fn(&t_type::is_floating_point));
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
  def.property("fields?", mem_fn(&t_structured::has_fields));
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
  def.property("blame", [](const t_exception& self) -> whisker::string {
    switch (self.blame()) {
      case t_error_blame::server:
        return "SERVER";
      case t_error_blame::client:
        return "CLIENT";
      default:
        return "UNSPECIFIED";
    }
  });
  def.property("kind", [](const t_exception& self) -> whisker::string {
    switch (self.kind()) {
      case t_error_kind::transient:
        return "TRANSIENT";
      case t_error_kind::stateful:
        return "STATEFUL";
      case t_error_kind::permanent:
        return "PERMANENT";
      default:
        return "UNSPECIFIED";
    }
  });
  def.property("safety", [](const t_exception& self) -> whisker::string {
    switch (self.safety()) {
      case t_error_safety::safe:
        return "SAFE";
      default:
        return "UNSPECIFIED";
    }
  });
  def.property(
      "message_field", [&proto](const t_exception& self) -> whisker::object {
        return proto.create_nullable<t_field>(self.get_message_field());
      });
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
  def.property("unused", [](const t_enum& self) { return i64(self.unused()); });
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
  def.property("generated?", mem_fn(&t_const::generated));
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
  using cv = t_const_value::t_const_value_kind;
  def.property("bool?", [](const t_const_value& self) {
    return self.kind() == cv::CV_BOOL;
  });
  def.property("double?", [](const t_const_value& self) {
    return self.kind() == cv::CV_DOUBLE;
  });
  def.property("integer?", [](const t_const_value& self) {
    return self.kind() == cv::CV_INTEGER && !self.is_enum();
  });
  def.property("enum?", [](const t_const_value& self) {
    return self.kind() == cv::CV_INTEGER && self.is_enum();
  });
  def.property("enum_value?", [](const t_const_value& self) {
    return self.get_enum_value() != nullptr;
  });
  def.property("string?", [](const t_const_value& self) {
    return self.kind() == cv::CV_STRING;
  });
  def.property("map?", [](const t_const_value& self) {
    return self.kind() == cv::CV_MAP;
  });
  def.property("list?", [](const t_const_value& self) {
    return self.kind() == cv::CV_LIST;
  });
  def.property("container?", [](const t_const_value& self) {
    return self.kind() == cv::CV_MAP || self.kind() == cv::CV_LIST;
  });

  def.property("empty_container?", [](const t_const_value& self) {
    return (self.kind() == cv::CV_MAP && self.get_map().empty()) ||
        (self.kind() == cv::CV_LIST && self.get_list().empty());
  });
  def.property("const_struct?", [](const t_const_value& self) {
    return w::boolean(
        !self.ttype().empty() &&
        self.ttype()->get_true_type()->is<t_structured>());
  });
  def.property("nonzero?", [](const t_const_value& self) {
    switch (self.kind()) {
      case cv::CV_DOUBLE:
        return w::boolean(self.get_double() != 0.0);
      case cv::CV_BOOL:
        return w::boolean(self.get_bool());
      case cv::CV_INTEGER:
        return w::boolean(self.get_integer() != 0);
      default:
        return w::null;
    }
  });

  def.property("integer_value", [](const t_const_value& self) {
    return self.kind() == cv::CV_INTEGER ? w::i64(self.get_integer()) : w::null;
  });
  def.property("double_value", [](const t_const_value& self) {
    return self.kind() == cv::CV_DOUBLE ? w::f64(self.get_double()) : w::null;
  });
  def.property("bool_value", [](const t_const_value& self) {
    return self.kind() == cv::CV_BOOL ? w::boolean(self.get_bool()) : w::null;
  });
  def.property("string_value", [](const t_const_value& self) {
    return self.kind() == cv::CV_STRING
        ? w::string(get_escaped_string(self.get_string()))
        : w::null;
  });
  def.property("string_length", [](const t_const_value& self) {
    return self.kind() == cv::CV_STRING
        ? w::i64((long)self.get_string().length())
        : w::null;
  });

  def.property("enum_name", [](const t_const_value& self) {
    return self.kind() == cv::CV_INTEGER && self.is_enum()
        ? w::string(self.get_enum()->name())
        : w::null;
  });

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
  def.property("autogen_path", [](const t_program& self) {
    std::string path = self.path();
    // use posix path separators, even on windows, for autogen comment
    // to avoid spurious fixture regen diffs
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
  });
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

  def.property("constants?", is_non_empty_collection(&t_program::consts));
  def.property("enums?", is_non_empty_collection(&t_program::enums));
  def.property(
      "interactions?", is_non_empty_collection(&t_program::interactions));
  def.property("services?", is_non_empty_collection(&t_program::services));
  def.property(
      "structs?", is_non_empty_collection(&t_program::structured_definitions));
  def.property("typedefs?", is_non_empty_collection(&t_program::typedefs));
  def.property("unions?", [](const t_program& self) {
    return std::any_of(
        self.structs_and_unions().begin(),
        self.structs_and_unions().end(),
        std::mem_fn(&t_structured::is<t_union>));
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
  def.property("exceptions", [proto](const t_sink& self) {
    return to_array(get_elems(self.sink_exceptions()), proto.of<t_field>());
  });
  def.property("final_response_exceptions", [proto](const t_sink& self) {
    return to_array(
        get_elems(self.final_response_exceptions()), proto.of<t_field>());
  });
  return std::move(def).make();
}

prototype<t_stream>::ptr t_whisker_generator::make_prototype_for_stream(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_stream>::extends(proto.of<t_node>());
  def.property("exceptions", [proto](const t_stream& self) {
    return to_array(get_elems(self.exceptions()), proto.of<t_field>());
  });
  return std::move(def).make();
}

prototype<t_function>::ptr t_whisker_generator::make_prototype_for_function(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_function>::extends(proto.of<t_named>());

  def.property("params", mem_fn(&t_function::params, proto.of<t_paramlist>()));
  def.property("sink", mem_fn(&t_function::sink, proto.of<t_sink>()));
  def.property("stream", mem_fn(&t_function::stream, proto.of<t_stream>()));

  def.property("return_type", [&](const t_function& function) {
    const t_type* type = function.return_type().get_type();
    return resolve_derived_t_type(proto, *type);
  });

  def.property("oneway?", [](const t_function& self) {
    return self.qualifier() == t_function_qualifier::oneway;
  });
  def.property("void?", [](const t_function& self) {
    return self.return_type()->is_void() && !self.interaction() &&
        !self.sink_or_stream();
  });
  def.property("exceptions?", [](const t_function& self) {
    return !get_elems(self.exceptions()).empty();
  });
  def.property("priority", [](const t_function& self) -> whisker::string {
    if (const t_const* val =
            self.find_structured_annotation_or_null(kPriorityUri)) {
      return val->get_value_from_structured_annotation("level")
          .get_enum_value()
          ->name();
    }
    return self.get_unstructured_annotation("priority", "NORMAL");
  });
  def.property("qualifier", [](const t_function& self) -> whisker::string {
    switch (self.qualifier()) {
      case t_function_qualifier::oneway:
        return "OneWay";
      case t_function_qualifier::idempotent:
        return "Idempotent";
      case t_function_qualifier::readonly:
        return "ReadOnly";
      default:
        return "Unspecified";
    }
  });

  // Interaction methods
  def.property(
      "starts_interaction?", mem_fn(&t_function::is_interaction_constructor));
  def.property("creates_interaction?", [](const t_function& self) {
    return !self.interaction().empty();
  });

  // Sink methods
  def.property("sink_or_stream?", [](const t_function& self) {
    return self.sink_or_stream() != nullptr;
  });
  def.property(
      "sink?", [](const t_function& self) { return self.sink() != nullptr; });
  def.property("sink_has_first_response?", [](const t_function& self) {
    return !self.has_void_initial_response() && self.sink() != nullptr;
  });

  // Stream methods
  def.property("stream?", [](const t_function& self) {
    return self.stream() != nullptr;
  });
  def.property("bidirectional_stream?", [](const t_function& self) {
    return self.sink() && self.stream();
  });
  def.property("stream_has_first_response?", [](const t_function& self) {
    return !self.has_void_initial_response() && self.stream() != nullptr;
  });
  def.property("initial_response?", [](const t_function& self) {
    return !self.return_type()->is_void();
  });

  return std::move(def).make();
}

prototype<t_interface>::ptr t_whisker_generator::make_prototype_for_interface(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_interface>::extends(proto.of<t_type>());
  def.property(
      "functions", mem_fn(&t_interface::functions, proto.of<t_function>()));
  def.property("interaction?", mem_fn(&t_interface::is_interaction));

  // This property retrieves the interactions of on this interface alone
  def.property("interactions", [&proto](const t_interface& interface) {
    if (interface.is_interaction()) {
      // Interactions cannot contain other interactions
      return whisker::array::of({});
    }

    std::vector<const t_interaction*> interactions;
    interactions.reserve(interface.functions().size());
    for (const auto& function : interface.functions()) {
      if (const auto& interaction = function.interaction()) {
        auto* ptr = &interaction->as<t_interaction>();
        if (std::find(interactions.begin(), interactions.end(), ptr) !=
            interactions.end()) {
          continue; // Already seen this interaction.
        }
        interactions.push_back(ptr);
      }
    }

    whisker::array::raw refs;
    refs.reserve(interactions.size());
    for (const auto* interaction : interactions) {
      refs.emplace_back(proto.create<t_interaction>(*interaction));
    }
    return whisker::array::of(std::move(refs));
  });
  return std::move(def).make();
}

prototype<t_service>::ptr t_whisker_generator::make_prototype_for_service(
    const prototype_database& proto) const {
  auto def = prototype_builder<h_service>::extends(proto.of<t_interface>());

  def.property("extends", mem_fn(&t_service::extends, proto.of<t_service>()));

  def.property("user_type_footprint", [&](const t_service& service) {
    return build_user_type_footprint(service, proto);
  });

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

    std::optional<source_view> source_code = src_manager_.get_file(path);
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
          src_manager_.resolve_location(include_from).file_name();
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

void t_whisker_generator::initialize_context() {
  context_visitor visitor;
  context_.register_visitors(visitor);
  initialize_context(visitor);
  for (const t_program& p : program_bundle_.programs()) {
    visitor(p);
  }
}

t_whisker_generator::cached_render_state& t_whisker_generator::render_state() {
  if (!cached_render_state_) {
    // Ideally we'd call initialize_context in the constructor, but the derived
    // initializer initialize_context(visitor) is virtual, and calling it from
    // the constructor would not call the derived class's implementation
    initialize_context();

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
