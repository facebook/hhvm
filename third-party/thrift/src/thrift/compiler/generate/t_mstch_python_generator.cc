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
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/detail/mustache/mstch.h>
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

bool is_type_iobuf(const std::string& name) {
  return name == "folly::IOBuf" || name == "std::unique_ptr<folly::IOBuf>";
}

const t_const* find_structured_adapter_annotation(const t_named& node) {
  return node.find_structured_annotation_or_null(kPythonAdapterUri);
}

const t_const* find_structured_adapter_annotation(const t_type& type) {
  // Traverse typedefs and find first adapter if any.
  return t_typedef::get_first_structured_annotation_or_null(
      &type, kPythonAdapterUri);
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

const t_const* get_transitive_annotation_of_adapter_or_null(
    const t_named& node) {
  for (const auto* annotation : node.structured_annotations()) {
    const t_type& annotation_type = *annotation->type();
    if (is_transitive_annotation(annotation_type)) {
      if (annotation_type.find_structured_annotation_or_null(
              kPythonAdapterUri)) {
        return annotation;
      }
    }
  }
  return nullptr;
}

const std::string extract_module_path(const std::string& fully_qualified_name) {
  auto tokens = split_namespace(fully_qualified_name);
  if (tokens.size() <= 1) {
    return "";
  }
  tokens.pop_back();
  return boost::algorithm::join(tokens, ".");
}

class python_mstch_const_value;

mstch::node adapter_node(
    const t_const* adapter_annotation,
    const t_const* transitive_adapter_annotation,
    mstch_context& context,
    mstch_element_position pos) {
  if (!adapter_annotation) {
    return false;
  }
  auto type_hint = get_annotation_property(adapter_annotation, "typeHint");
  bool is_generic = boost::algorithm::ends_with(type_hint, "[]");
  if (is_generic) {
    type_hint = type_hint.substr(0, type_hint.size() - 2);
  }
  bool is_transitive = (transitive_adapter_annotation != nullptr);
  mstch::map node{
      {"adapter:name", get_annotation_property(adapter_annotation, "name")},
      {"adapter:type_hint", type_hint},
      {"adapter:is_generic?", is_generic},
      {"adapter:is_transitive?", is_transitive},
  };
  if (is_transitive) {
    node["adapter:transitive_annotation"] =
        std::make_shared<python_mstch_const_value>(
            transitive_adapter_annotation->value(),
            context,
            pos,
            transitive_adapter_annotation,
            &*transitive_adapter_annotation->type());
  }
  return node;
}

class python_mstch_program : public mstch_program {
 public:
  python_mstch_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:module_path", &python_mstch_program::module_path},
            {"program:py_deprecated_module_path",
             &python_mstch_program::py_deprecated_module_path},
            {"program:is_types_file?", &python_mstch_program::is_types_file},
            {"program:include_namespaces",
             &python_mstch_program::include_namespaces},
            {"program:base_library_package",
             &python_mstch_program::base_library_package},
            {"program:root_module_prefix",
             &python_mstch_program::root_module_prefix},
            {"program:adapter_modules", &python_mstch_program::adapter_modules},
            {"program:adapter_type_hint_modules",
             &python_mstch_program::adapter_type_hint_modules},
        });
    register_has_option("program:import_static?", "import_static");
    gather_included_program_namespaces();
    visit_types_for_services_and_interactions();
    visit_types_for_objects();
    visit_types_for_constants();
    visit_types_for_typedefs();
    visit_types_for_mixin_fields();
    visit_types_for_adapters();
  }

  mstch::node is_types_file() { return has_option("is_types_file"); }

  mstch::node include_namespaces() {
    std::vector<const Namespace*> namespaces;
    for (const auto& it : include_namespaces_) {
      namespaces.push_back(&it.second);
    }
    std::sort(
        namespaces.begin(), namespaces.end(), [](const auto* m, const auto* n) {
          return m->ns < n->ns;
        });
    mstch::array a;
    for (const auto& it : namespaces) {
      a.push_back(mstch::map{
          {"included_module_path", it->ns},
          {"has_services?", it->has_services},
          {"has_types?", it->has_types}});
    }
    return a;
  }

  mstch::node module_path() {
    return get_py3_namespace_with_name_and_prefix(
        program_, get_option("root_module_prefix"));
  }

  mstch::node py_deprecated_module_path() {
    std::string module_path = program_->get_namespace("py");
    if (module_path.empty()) {
      return program_->name();
    }
    return module_path;
  }

  mstch::node base_library_package() {
    auto option = get_option("base_library_package");
    return option.empty() ? "thrift.python" : option;
  }

  mstch::node root_module_prefix() {
    auto prefix = get_option("root_module_prefix");
    return prefix.empty() ? "" : prefix + ".";
  }

  mstch::node adapter_modules() { return module_path_array(adapter_modules_); }

  mstch::node adapter_type_hint_modules() {
    return module_path_array(adapter_type_hint_modules_);
  }

 protected:
  struct Namespace {
    std::string ns;
    bool has_services;
    bool has_types;
  };

  void gather_included_program_namespaces() {
    for (const t_program* included_program :
         program_->get_included_programs()) {
      bool has_types =
          !(included_program->objects().empty() &&
            included_program->enums().empty() &&
            included_program->typedefs().empty() &&
            included_program->consts().empty());
      include_namespaces_[included_program->path()] = Namespace{
          get_py3_namespace_with_name_and_prefix(
              included_program, get_option("root_module_prefix")),
          !included_program->services().empty(),
          has_types,
      };
    }
  }

  void add_typedef_namespace(const t_type* type) {
    auto prog = type->program();
    if (prog && prog != program_) {
      const auto& path = prog->path();
      if (include_namespaces_.find(path) != include_namespaces_.end()) {
        return;
      }
      auto ns = Namespace();
      ns.ns = get_py3_namespace_with_name_and_prefix(
          prog, get_option("root_module_prefix"));
      ns.has_services = false;
      ns.has_types = true;
      include_namespaces_[path] = std::move(ns);
    }
  }

  void visit_type_single_service(const t_service* service) {
    for (const auto& function : service->functions()) {
      for (const auto& field : function.get_paramlist()->fields()) {
        visit_type(field.get_type());
      }
      for (const auto& field : function.get_xceptions()->fields()) {
        visit_type(field.get_type());
      }
      for (const auto& field : function.get_stream_xceptions()->fields()) {
        visit_type(field.get_type());
      }
      visit_type(function.get_returntype());
    }
  }

  void visit_types_for_services_and_interactions() {
    for (const auto* service : program_->services()) {
      visit_type_single_service(service);
    }
    for (const auto* interaction : program_->interactions()) {
      visit_type_single_service(interaction);
    }
  }

  void visit_types_for_objects() {
    for (const auto& object : program_->objects()) {
      for (auto&& field : object->fields()) {
        visit_type(field.get_type());
      }
    }
  }

  void visit_types_for_constants() {
    for (const auto& constant : program_->consts()) {
      visit_type(constant->get_type());
    }
  }

  void visit_types_for_typedefs() {
    for (const auto typedef_def : program_->typedefs()) {
      visit_type(typedef_def->get_type());
    }
  }

  void visit_types_for_mixin_fields() {
    for (const auto& strct : program_->structs()) {
      for (const auto& m : cpp2::get_mixins_and_members(*strct)) {
        visit_type(m.member->get_type());
      }
    }
  }

  void visit_types_for_adapters() {
    for (const auto& strct : program_->structs()) {
      if (auto annotation = find_structured_adapter_annotation(*strct)) {
        extract_module_and_insert_to(
            get_annotation_property(annotation, "name"), adapter_modules_);
        extract_module_and_insert_to(
            get_annotation_property(annotation, "typeHint"), adapter_modules_);
        extract_module_and_insert_to(
            get_annotation_property(annotation, "typeHint"),
            adapter_type_hint_modules_);
      }
      for (const auto& field : strct->fields()) {
        if (auto annotation = find_structured_adapter_annotation(field)) {
          extract_module_and_insert_to(
              get_annotation_property(annotation, "name"), adapter_modules_);
          extract_module_and_insert_to(
              get_annotation_property(annotation, "typeHint"),
              adapter_type_hint_modules_);
        }
      }
    }
    for (const auto typedef_def : program_->typedefs()) {
      if (auto annotation = find_structured_adapter_annotation(*typedef_def)) {
        extract_module_and_insert_to(
            get_annotation_property(annotation, "name"), adapter_modules_);
        extract_module_and_insert_to(
            get_annotation_property(annotation, "typeHint"), adapter_modules_);
        extract_module_and_insert_to(
            get_annotation_property(annotation, "typeHint"),
            adapter_type_hint_modules_);
      }
    }
  }

  void extract_module_and_insert_to(
      const std::string& name, std::unordered_set<std::string>& modules) {
    auto module_path = extract_module_path(name);
    if (module_path != "") {
      modules.insert(module_path);
    }
  }

  enum TypeDef { NoTypedef, HasTypedef };

  void visit_type(const t_type* orig_type) {
    return visit_type_with_typedef(orig_type, TypeDef::NoTypedef);
  }

  void visit_type_with_typedef(const t_type* orig_type, TypeDef is_typedef) {
    auto true_type = orig_type->get_true_type();
    if (!seen_types_.insert(true_type).second) {
      return;
    }
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
      visit_type_with_typedef(
          dynamic_cast<const t_map&>(*true_type).get_key_type(), is_typedef);
      visit_type_with_typedef(
          dynamic_cast<const t_map&>(*true_type).get_val_type(), is_typedef);
    } else if (true_type->is_streamresponse()) {
      const t_type* resp_type =
          dynamic_cast<const t_stream_response&>(*true_type)
              .get_first_response_type();
      const t_type* elem_type =
          dynamic_cast<const t_stream_response&>(*true_type).get_elem_type();
      if (resp_type) {
        visit_type_with_typedef(resp_type, is_typedef);
      }
      visit_type_with_typedef(elem_type, is_typedef);
    }
  }

  mstch::node module_path_array(
      const std::unordered_set<std::string>& modules) {
    mstch::array a;
    for (const auto& m : modules) {
      a.push_back(mstch::map{{"module_path", m}});
    }
    return a;
  }

  std::unordered_map<std::string, Namespace> include_namespaces_;
  std::unordered_set<const t_type*> seen_types_;
  std::unordered_set<std::string> adapter_modules_;
  std::unordered_set<std::string> adapter_type_hint_modules_;
};

class python_mstch_service : public mstch_service {
 public:
  python_mstch_service(
      const t_service* s,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_program* prog)
      : mstch_service(s, ctx, pos), prog_(prog) {
    register_methods(
        this,
        {
            {"service:module_path", &python_mstch_service::module_path},
            {"service:program_name", &python_mstch_service::program_name},
            {"service:parent_service_name",
             &python_mstch_service::parent_service_name},
            {"service:supported_functions",
             &python_mstch_service::supported_functions},
            {"service:supported_service_functions",
             &python_mstch_service::supported_service_functions},
            {"service:external_program?",
             &python_mstch_service::is_external_program},
        });
  }

  mstch::node module_path() {
    return get_py3_namespace_with_name_and_prefix(
        service_->program(), get_option("root_module_prefix"));
  }

  mstch::node program_name() { return service_->program()->name(); }

  mstch::node parent_service_name() {
    return context_.options.at("parent_service_name");
  }

  std::vector<t_function*> get_supported_functions(
      std::function<bool(const t_function*)> func_filter) {
    std::vector<t_function*> funcs;
    for (auto func : service_->get_functions()) {
      if (func_filter(func)) {
        funcs.push_back(func);
      }
    }
    return funcs;
  }

  mstch::node supported_functions() {
    return make_mstch_functions(
        get_supported_functions([](const t_function* func) -> bool {
          return !func->returns_sink() && !func->get_returntype()->is_service();
        }));
  }

  mstch::node supported_service_functions() {
    return make_mstch_functions(
        get_supported_functions([](const t_function* func) -> bool {
          return !func->returns_stream() && !func->returns_sink() &&
              !func->get_returntype()->is_service();
        }));
  }

  mstch::node is_external_program() { return prog_ != service_->program(); }

 protected:
  const t_program* prog_;
};

// Generator-specific validator that enforces that a reserved key is not used as
// a namespace component.
class no_reserved_key_in_namespace_validator : virtual public validator {
 public:
  using validator::visit;

  bool visit(t_program* prog) override {
    validate(prog);
    return true;
  }

 private:
  void validate(t_program* prog) {
    auto namespace_tokens = get_py3_namespace(prog);
    if (namespace_tokens.empty()) {
      return;
    }
    for (const auto& field_name : namespace_tokens) {
      if (get_python_reserved_names().find(field_name) !=
          get_python_reserved_names().end()) {
        report_error(
            *prog,
            "Namespace '{}' contains reserved keyword '{}'",
            fmt::join(namespace_tokens, "."),
            field_name);
      }
    }

    std::vector<std::string> fields;
    boost::split(fields, prog->path(), boost::is_any_of("\\/."));
    for (const auto& field : fields) {
      if (field == "include") {
        report_error(
            *prog,
            "Path '{}' contains reserved keyword 'include'",
            prog->path());
      }
    }
  }
};
class python_mstch_function : public mstch_function {
 public:
  python_mstch_function(
      const t_function* f, mstch_context& ctx, mstch_element_position pos)
      : mstch_function(f, ctx, pos) {
    register_methods(
        this,
        {
            {"function:created_interaction",
             &python_mstch_function::created_interaction},
            {"function:returns_tuple?", &python_mstch_function::returns_tuple},
            {"function:early_client_return?",
             &python_mstch_function::early_client_return},
            {"function:regular_response_type",
             &python_mstch_function::regular_response_type},
            {"function:return_stream_elem_type",
             &python_mstch_function::return_stream_elem_type},
            {"function:async_only?", &python_mstch_function::async_only},
        });
  }

  mstch::node created_interaction() {
    const auto* interaction = function_->returned_interaction().get_type();
    if (interaction != nullptr) {
      return interaction->get_name();
    }
    return "";
  }

  mstch::node returns_tuple() {
    // TOOD add in sinks, etc
    const auto& rettype = *function_->return_type();
    auto stream = dynamic_cast<const t_stream_response*>(&rettype);
    return (stream && !stream->first_response_type().empty()) ||
        (!function_->returned_interaction().empty() && !rettype.is_void());
  }

  mstch::node early_client_return() {
    // TOOD add in sinks, etc
    const auto& rettype = *function_->return_type();
    auto stream = dynamic_cast<const t_stream_response*>(&rettype);
    return !(
        rettype.is_void() || (stream && stream->first_response_type().empty()));
  }

  mstch::node regular_response_type() {
    if (function_->is_oneway()) {
      return {};
    }
    const t_type* rettype = function_->return_type()->get_true_type();
    if (rettype->is_streamresponse()) {
      auto stream = dynamic_cast<const t_stream_response*>(rettype);
      rettype = stream->has_first_response() ? stream->get_first_response_type()
                                             : &t_base_type::t_void();
    }
    return context_.type_factory->make_mstch_object(rettype, context_, pos_);
  }

  mstch::node return_stream_elem_type() {
    if (function_->is_oneway()) {
      return {};
    }
    const t_type* rettype = function_->return_type()->get_true_type();
    if (!rettype->is_streamresponse()) {
      return {};
    }
    return context_.type_factory->make_mstch_object(
        dynamic_cast<const t_stream_response*>(rettype)->get_elem_type(),
        context_,
        pos_);
  }

  mstch::node async_only() {
    return function_->returns_stream() || function_->returns_sink() ||
        function_->get_returntype()->is_service() ||
        function_->is_interaction_member() ||
        !function_->returned_interaction().empty();
  }

 protected:
  const std::string cppName_;
};

class python_mstch_type : public mstch_type {
 public:
  python_mstch_type(
      const t_type* type,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_program* prog)
      : mstch_type(type->get_true_type(), ctx, pos),
        prog_(prog),
        adapter_annotation_(find_structured_adapter_annotation(*type)),
        transitive_adapter_annotation_(
            get_transitive_annotation_of_adapter_or_null(*type)) {
    register_methods(
        this,
        {
            {"type:module_path", &python_mstch_type::module_path},
            {"type:program_name", &python_mstch_type::program_name},
            {"type:metadata_path", &python_mstch_type::metadata_path},
            {"type:py3_namespace", &python_mstch_type::py3_namespace},
            {"type:need_module_path?", &python_mstch_type::need_module_path},
            {"type:external_program?", &python_mstch_type::is_external_program},
            {"type:integer?", &python_mstch_type::is_integer},
            {"type:iobuf?", &python_mstch_type::is_iobuf},
            {"type:has_adapter?", &python_mstch_type::adapter},
            {"type:with_regular_response?",
             &python_mstch_type::with_regular_response},
        });
  }

  mstch::node module_path() {
    return get_py3_namespace_with_name_and_prefix(
               get_type_program(), get_option("root_module_prefix")) +
        ".thrift_types";
  }

  mstch::node program_name() { return get_type_program()->name(); }

  mstch::node metadata_path() {
    return get_py3_namespace_with_name_and_prefix(
               get_type_program(), get_option("root_module_prefix")) +
        ".thrift_metadata";
  }

  mstch::node py3_namespace() {
    std::ostringstream ss;
    for (const auto& path : get_py3_namespace(get_type_program())) {
      ss << path << ".";
    }
    return ss.str();
  }

  mstch::node need_module_path() {
    if (!has_option("is_types_file")) {
      return true;
    }
    if (const t_program* prog = type_->program()) {
      if (prog != prog_) {
        return true;
      }
    }
    return false;
  }

  mstch::node is_external_program() {
    auto p = type_->program();
    return p && p != prog_;
  }

  mstch::node is_integer() { return type_->is_any_int() || type_->is_byte(); }

  // Supporting legacy py3 cpp.type iobuf declaration here
  mstch::node is_iobuf() {
    return type_->has_annotation("py3.iobuf") ||
        is_type_iobuf(type_->get_annotation("cpp2.type")) ||
        is_type_iobuf(type_->get_annotation("cpp.type"));
  }

  mstch::node adapter() {
    return adapter_node(
        adapter_annotation_, transitive_adapter_annotation_, context_, pos_);
  }

  mstch::node with_regular_response() {
    if (!resolved_type_->is_streamresponse()) {
      return !resolved_type_->is_void();
    }
    auto stream = dynamic_cast<const t_stream_response*>(resolved_type_);
    return stream && !stream->first_response_type().empty();
  }

 protected:
  const t_program* get_type_program() const {
    if (const t_program* p = type_->program()) {
      return p;
    }
    return prog_;
  }

  const t_program* prog_;
  const t_const* adapter_annotation_;
  const t_const* transitive_adapter_annotation_;
};

class python_mstch_typedef : public mstch_typedef {
 public:
  python_mstch_typedef(
      const t_typedef* t, mstch_context& ctx, mstch_element_position pos)
      : mstch_typedef(t, ctx, pos),
        adapter_annotation_(find_structured_adapter_annotation(*t)) {
    register_methods(
        this,
        {
            {"typedef:has_adapter?", &python_mstch_typedef::adapter},
        });
  }

  mstch::node adapter() {
    return adapter_node(adapter_annotation_, nullptr, context_, pos_);
  }

 private:
  const t_const* adapter_annotation_;
};

class python_mstch_struct : public mstch_struct {
 public:
  python_mstch_struct(
      const t_struct* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos),
        adapter_annotation_(find_structured_adapter_annotation(*s)) {
    register_methods(
        this,
        {
            {"struct:fields_and_mixin_fields",
             &python_mstch_struct::fields_and_mixin_fields},
            {"struct:exception_message?",
             &python_mstch_struct::has_exception_message},
            {"struct:exception_message",
             &python_mstch_struct::exception_message},
            {"struct:has_adapter?", &python_mstch_struct::adapter},
            {"struct:legacy_api?", &python_mstch_struct::legacy_api},
        });
  }

  mstch::node fields_and_mixin_fields() {
    std::vector<const t_field*> fields = struct_->fields().copy();
    for (auto m : cpp2::get_mixins_and_members(*struct_)) {
      fields.push_back(m.member);
    }
    std::sort(fields.begin(), fields.end(), [](const auto* m, const auto* n) {
      return m->id() < n->id();
    });
    return make_mstch_fields(fields);
  }

  mstch::node has_exception_message() {
    return struct_->has_annotation("message");
  }
  mstch::node exception_message() { return struct_->get_annotation("message"); }

  mstch::node adapter() {
    return adapter_node(adapter_annotation_, nullptr, context_, pos_);
  }

  mstch::node legacy_api() {
    return ::apache::thrift::compiler::generate_legacy_api(*struct_);
  }

 private:
  const t_const* adapter_annotation_;
};

class python_mstch_field : public mstch_field {
 public:
  python_mstch_field(
      const t_field* field,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context)
      : mstch_field(field, ctx, pos, field_context),
        py_name_(py3::get_py3_name(*field)),
        adapter_annotation_(find_structured_adapter_annotation(*field)),
        transitive_adapter_annotation_(
            get_transitive_annotation_of_adapter_or_null(*field)) {
    register_methods(
        this,
        {
            {"field:py_name", &python_mstch_field::py_name},
            {"field:tablebased_qualifier",
             &python_mstch_field::tablebased_qualifier},
            {"field:user_default_value",
             &python_mstch_field::user_default_value},
            {"field:has_adapter?", &python_mstch_field::adapter},
        });
  }

  mstch::node py_name() { return py_name_; }
  mstch::node tablebased_qualifier() {
    const std::string enum_type = "FieldQualifier.";
    switch (field_->qualifier()) {
      case t_field_qualifier::none:
      case t_field_qualifier::required:
        return enum_type + "Unqualified";
      case t_field_qualifier::optional:
        return enum_type + "Optional";
      case t_field_qualifier::terse:
        return enum_type + "Terse";
      default:
        throw std::runtime_error("unknown qualifier");
    }
  }
  mstch::node user_default_value() {
    const t_const_value* value = field_->get_value();
    if (!value) {
      return mstch::node();
    }
    if (value->is_empty()) {
      auto true_type = field_->get_type()->get_true_type();
      if ((true_type->is_list() || true_type->is_set()) &&
          value->get_type() != t_const_value::CV_LIST) {
        const_cast<t_const_value*>(value)->set_list();
      }
      if (true_type->is_map() && value->get_type() != t_const_value::CV_MAP) {
        const_cast<t_const_value*>(value)->set_map();
      }
    }
    return context_.const_value_factory->make_mstch_object(
        value, context_, pos_, nullptr, nullptr);
  }
  mstch::node adapter() {
    return adapter_node(
        adapter_annotation_, transitive_adapter_annotation_, context_, pos_);
  }

 private:
  const std::string py_name_;
  const t_const* adapter_annotation_;
  const t_const* transitive_adapter_annotation_;
};

class python_mstch_enum : public mstch_enum {
 public:
  python_mstch_enum(
      const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum(e, ctx, pos) {
    register_methods(
        this,
        {
            {"enum:flags?", &python_mstch_enum::has_flags},
            {"enum:legacy_api?", &python_mstch_enum::legacy_api},
        });
  }

  mstch::node has_flags() { return enum_->has_annotation("py3.flags"); }

  mstch::node legacy_api() {
    return ::apache::thrift::compiler::generate_legacy_api(*enum_);
  }
};

class python_mstch_enum_value : public mstch_enum_value {
 public:
  python_mstch_enum_value(
      const t_enum_value* ev, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum_value(ev, ctx, pos) {
    register_methods(
        this,
        {
            {"enum_value:py_name", &python_mstch_enum_value::py_name},
        });
  }

  mstch::node py_name() { return py3::get_py3_name(*enum_value_); }
};

// Generator-specific validator that enforces "name" and "value" are not used as
// enum member or union field names (thrift-py3).
class enum_member_union_field_names_validator : virtual public validator {
 public:
  using validator::visit;

  bool visit(t_enum* enm) override {
    for (const t_enum_value* ev : enm->get_enum_values()) {
      validate(ev, ev->get_name());
    }
    return true;
  }

  bool visit(t_struct* s) override {
    if (!s->is_union()) {
      return false;
    }
    for (const t_field& f : s->fields()) {
      validate(&f, f.name());
    }
    return true;
  }

 private:
  void validate(const t_named* node, const std::string& name) {
    const auto& pyname = node->get_annotation("py3.name", &name);
    if (pyname == "name" || pyname == "value") {
      report_error(
          *node,
          "'{}' should not be used as an enum/union field name in thrift-py3. "
          "Use a different name or annotate the field with "
          "`(py3.name=\"<new_py_name>\")`",
          pyname);
    }
  }
};

class t_mstch_python_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "python"; }

  void generate_program() override {
    generate_root_path_ = package_to_path();
    out_dir_base_ = "gen-python";
    auto include_prefix = get_option("include_prefix").value_or("");
    if (!include_prefix.empty()) {
      program_->set_include_prefix(std::move(include_prefix));
    }
    set_mstch_factories();
    generate_types();
    generate_metadata();
    generate_clients();
    generate_services();
  }

  void fill_validator_list(validator_list& vl) const override {
    vl.add<no_reserved_key_in_namespace_validator>();
    vl.add<enum_member_union_field_names_validator>();
  }

  enum TypesFile { IsTypesFile, NotTypesFile };

 protected:
  bool should_resolve_typedefs() const override { return true; }
  void set_mstch_factories();
  void generate_file(
      const std::string& file,
      TypesFile is_types_file,
      const boost::filesystem::path& base);
  void set_types_file(bool val);
  void generate_types();
  void generate_metadata();
  void generate_clients();
  void generate_services();
  boost::filesystem::path package_to_path();

  boost::filesystem::path generate_root_path_;
};

class python_mstch_const : public mstch_const {
 public:
  python_mstch_const(
      const t_const* c,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      const t_field* field)
      : mstch_const(c, ctx, pos, current_const, expected_type, field),
        adapter_annotation_(find_structured_adapter_annotation(*c)),
        transitive_adapter_annotation_(
            get_transitive_annotation_of_adapter_or_null(*c)) {
    register_methods(
        this,
        {
            {"constant:has_adapter?", &python_mstch_const::has_adapter},
            {"constant:adapter_name", &python_mstch_const::adapter_name},
            {"constant:adapter_type_hint",
             &python_mstch_const::adapter_type_hint},
            {"constant:is_adapter_transitive?",
             &python_mstch_const::is_adapter_transitive},
            {"constant:transitive_adapter_annotation",
             &python_mstch_const::transitive_adapter_annotation},
        });
  }

  mstch::node has_adapter() { return adapter_annotation_ != nullptr; }

  mstch::node adapter_name() {
    return get_annotation_property(adapter_annotation_, "name");
  }

  mstch::node adapter_type_hint() {
    return get_annotation_property(adapter_annotation_, "typeHint");
  }

  mstch::node is_adapter_transitive() {
    return transitive_adapter_annotation_ != nullptr;
  }

  mstch::node transitive_adapter_annotation() {
    return std::make_shared<python_mstch_const_value>(
        transitive_adapter_annotation_->value(),
        context_,
        pos_,
        transitive_adapter_annotation_,
        &*transitive_adapter_annotation_->type());
  }

 private:
  const t_const* adapter_annotation_;
  const t_const* transitive_adapter_annotation_;
};

class python_mstch_const_value : public mstch_const_value {
 public:
  python_mstch_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type) {
    register_methods(
        this,
        {
            {"value:py3_enum_value_name",
             &python_mstch_const_value::py3_enum_value_name},
            {"value:py3_binary?", &python_mstch_const_value::is_binary},
            {"value:const_enum_type",
             &python_mstch_const_value::const_enum_type},
            {"value:value_for_bool?",
             &python_mstch_const_value::value_for_bool},
            {"value:value_for_floating_point?",
             &python_mstch_const_value::value_for_floating_point},
            {"value:list_elem_type", &python_mstch_const_value::list_elem_type},
            {"value:value_for_set?", &python_mstch_const_value::value_for_set},
            {"value:map_key_type", &python_mstch_const_value::map_key_type},
            {"value:map_val_type", &python_mstch_const_value::map_val_type},
        });
  }

  mstch::node is_binary() {
    auto& ttype = const_value_->ttype();
    return type_ == cv::CV_STRING && ttype &&
        ttype->get_true_type()->is_binary();
  }

  mstch::node const_enum_type() {
    if (!const_value_->ttype() || type_ != cv::CV_INTEGER ||
        !const_value_->is_enum()) {
      return {};
    }
    const auto* type = const_value_->ttype()->get_true_type();
    if (type->is_enum()) {
      return context_.type_factory->make_mstch_object(type, context_);
    }
    return {};
  }

  mstch::node value_for_bool() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is_bool();
    }
    return false;
  }

  mstch::node value_for_floating_point() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is_floating_point();
    }
    return false;
  }

  mstch::node py3_enum_value_name() {
    if (const_value_->is_enum() && const_value_->get_enum_value() != nullptr) {
      return py3::get_py3_name(*const_value_->get_enum_value());
    }
    return mstch::node();
  }

  mstch::node list_elem_type() {
    if (auto ttype = const_value_->ttype()) {
      const auto* type = ttype->get_true_type();
      const t_type* elem_type = nullptr;
      if (type->is_list()) {
        elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
      } else if (type->is_set()) {
        elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
      } else {
        return {};
      }
      return context_.type_factory->make_mstch_object(
          elem_type, context_, pos_);
    }
    return {};
  }

  mstch::node value_for_set() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is_set();
    }
    return false;
  }

  mstch::node map_key_type() {
    if (auto ttype = const_value_->ttype()) {
      const auto* type = ttype->get_true_type();
      if (type->is_map()) {
        return context_.type_factory->make_mstch_object(
            dynamic_cast<const t_map*>(type)->get_key_type(), context_, pos_);
      }
    }
    return {};
  }

  mstch::node map_val_type() {
    if (auto ttype = const_value_->ttype()) {
      const auto* type = ttype->get_true_type();
      if (type->is_map()) {
        return context_.type_factory->make_mstch_object(
            dynamic_cast<const t_map*>(type)->get_val_type(), context_, pos_);
      }
    }
    return {};
  }
};

class python_mstch_deprecated_annotation : public mstch_deprecated_annotation {
 public:
  python_mstch_deprecated_annotation(
      const t_annotation* a, mstch_context& ctx, mstch_element_position pos)
      : mstch_deprecated_annotation(a, ctx, pos) {
    register_methods(
        this,
        {
            {"annotation:value?",
             &python_mstch_deprecated_annotation::has_value},
            {"annotation:py_quoted_key",
             &python_mstch_deprecated_annotation::py_quoted_key},
            {"annotation:py_quoted_value",
             &python_mstch_deprecated_annotation::py_quoted_value},
        });
  }

  mstch::node has_value() { return !val_.value.empty(); }
  mstch::node py_quoted_key() { return to_python_string_literal(key_); }
  mstch::node py_quoted_value() { return to_python_string_literal(val_.value); }

 protected:
  std::string to_python_string_literal(std::string val) const {
    std::string quotes = "\"\"\"";
    boost::algorithm::replace_all(val, "\\", "\\\\");
    boost::algorithm::replace_all(val, "\"", "\\\"");
    return quotes + val + quotes;
  }
};

void t_mstch_python_generator::set_mstch_factories() {
  mstch_context_.add<python_mstch_program>();
  mstch_context_.add<python_mstch_service>(program_);
  mstch_context_.add<python_mstch_function>();
  mstch_context_.add<python_mstch_type>(program_);
  mstch_context_.add<python_mstch_typedef>();
  mstch_context_.add<python_mstch_struct>();
  mstch_context_.add<python_mstch_field>();
  mstch_context_.add<python_mstch_enum>();
  mstch_context_.add<python_mstch_enum_value>();
  mstch_context_.add<python_mstch_const>();
  mstch_context_.add<python_mstch_const_value>();
  mstch_context_.add<python_mstch_deprecated_annotation>();
}

boost::filesystem::path t_mstch_python_generator::package_to_path() {
  auto package = get_py3_namespace(get_program());
  return boost::algorithm::join(package, "/");
}

void t_mstch_python_generator::generate_file(
    const std::string& file,
    TypesFile is_types_file,
    const boost::filesystem::path& base = {}) {
  auto program = get_program();
  const auto& name = program->name();
  if (is_types_file == IsTypesFile) {
    mstch_context_.options["is_types_file"] = "";
  } else {
    mstch_context_.options.erase("is_types_file");
  }
  auto mstch_program = make_mstch_program_cached(program, mstch_context_);
  render_to_file(mstch_program, file, base / name / file);
}

void t_mstch_python_generator::generate_types() {
  generate_file("thrift_types.py", IsTypesFile, generate_root_path_);
  generate_file("thrift_types.pyi", IsTypesFile, generate_root_path_);
}

void t_mstch_python_generator::generate_metadata() {
  generate_file("thrift_metadata.py", IsTypesFile, generate_root_path_);
}

void t_mstch_python_generator::generate_clients() {
  if (get_program()->services().empty()) {
    // There is no need to generate empty / broken code for non existent
    // services.
    return;
  }

  generate_file("thrift_clients.py", NotTypesFile, generate_root_path_);
}

void t_mstch_python_generator::generate_services() {
  if (get_program()->services().empty()) {
    // There is no need to generate empty / broken code for non existent
    // services.
    return;
  }
  generate_file("thrift_services.py", NotTypesFile, generate_root_path_);
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_python,
    "Python",
    "    include_prefix:  Use full include paths in generated files.\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
