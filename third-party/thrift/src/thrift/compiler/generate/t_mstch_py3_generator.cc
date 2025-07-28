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

#include <filesystem>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fmt/format.h>

#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>
#include <thrift/compiler/generate/cpp/reference_type.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/python/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache::thrift::compiler {

namespace {

std::vector<t_function*> lifecycleFunctions() {
  static t_function onStartServing_{
      nullptr, t_primitive_type::t_void(), "onStartServing"};
  static t_function onStopRequested_{
      nullptr, t_primitive_type::t_void(), "onStopRequested"};

  return {&onStartServing_, &onStopRequested_};
}

// TO-DO: remove duplicate in pyi
mstch::array create_string_array(const std::vector<std::string>& values) {
  mstch::array mstch_array;
  for (auto it = values.begin(); it != values.end(); ++it) {
    mstch_array.push_back(mstch::map{
        {"value", *it},
        {"first?", it == values.begin()},
        {"last?", std::next(it) == values.end()},
    });
  }

  return mstch_array;
}

// TO-DO: remove duplicate in pyi
bool has_types(const t_program* program) {
  assert(program != nullptr);

  return !(
      program->structured_definitions().empty() && program->enums().empty() &&
      program->typedefs().empty() && program->consts().empty());
}

std::vector<std::string> get_py3_namespace_with_name(const t_program* program) {
  auto ns = get_py3_namespace(program);
  ns.push_back(program->name());
  return ns;
}

const t_type* get_list_elem_type(const t_type& type) {
  assert(type.is<t_list>());
  return dynamic_cast<const t_list&>(type).get_elem_type();
}

const t_type* get_set_elem_type(const t_type& type) {
  assert(type.is<t_set>());
  return dynamic_cast<const t_set&>(type).get_elem_type();
}

const t_type* get_map_key_type(const t_type& type) {
  assert(type.is<t_map>());
  return dynamic_cast<const t_map&>(type).get_key_type();
}

const t_type* get_map_val_type(const t_type& type) {
  assert(type.is<t_map>());
  return dynamic_cast<const t_map&>(type).get_val_type();
}

bool type_needs_convert(const t_type* type) {
  // NB: float32 has to be rounded by cython to maintain old py3 behavior
  return type->is<t_structured>() || type->is<t_container>() ||
      type->is_float();
}

bool container_needs_convert(const t_type* type) {
  const t_type* true_type = type->get_true_type();

  if (const t_map* map_type = dynamic_cast<const t_map*>(true_type)) {
    return container_needs_convert(map_type->get_key_type()) ||
        container_needs_convert(map_type->get_val_type());
  } else if (const t_list* list_type = dynamic_cast<const t_list*>(true_type)) {
    return container_needs_convert(list_type->get_elem_type());
  } else if (const t_set* set_type = dynamic_cast<const t_set*>(true_type)) {
    return container_needs_convert(set_type->get_elem_type());
  } else if (true_type->is<t_structured>()) {
    return true;
  }
  return false;
}

std::string get_cpp_template(const t_type& type) {
  if (const auto* val = type.find_unstructured_annotation_or_null(
          {"cpp.template", "cpp2.template"})) {
    return *val;
  }
  if (type.is<t_list>()) {
    return "std::vector";
  }
  if (type.is<t_set>()) {
    return "std::set";
  }
  if (type.is<t_map>()) {
    return "std::map";
  }

  return {};
}

bool is_hidden(const t_named& node) {
  return node.has_unstructured_annotation("py3.hidden") ||
      node.has_structured_annotation(kPythonPy3HiddenUri);
}
bool is_hidden(const t_typedef& node) {
  return node.generated() || node.has_unstructured_annotation("py3.hidden") ||
      node.has_structured_annotation(kPythonPy3HiddenUri) ||
      is_hidden(*node.get_true_type());
}
bool is_hidden(const t_type& node) {
  return node.generated() || node.has_unstructured_annotation("py3.hidden") ||
      node.has_structured_annotation(kPythonPy3HiddenUri) ||
      cpp_name_resolver::is_directly_adapted(node);
}

bool is_func_supported(bool no_stream, const t_function* func) {
  return !is_hidden(*func) && !(no_stream && func->stream()) && !func->sink() &&
      !func->is_interaction_constructor();
}

std::vector<std::string> get_type_py3_namespace(
    const t_program* prog, const std::string& suffix) {
  auto ns = get_py3_namespace_with_name(prog);
  ns.push_back(suffix);
  return ns;
}

class py3_mstch_program : public mstch_program {
 public:
  py3_mstch_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:unique_functions_by_return_type",
             &py3_mstch_program::unique_functions_by_return_type},
            {"program:has_types?", &py3_mstch_program::program_has_types},
            {"program:needs_container_converters?",
             &py3_mstch_program::needs_container_converters},
            {"program:cppNamespaces", &py3_mstch_program::getCpp2Namespace},
            {"program:py3Namespaces", &py3_mstch_program::getPy3Namespace},
            {"program:includeNamespaces",
             &py3_mstch_program::includeNamespaces},
            {"program:cppIncludes", &py3_mstch_program::getCppIncludes},
            {"program:containerTypes", &py3_mstch_program::getContainerTypes},
            {"program:hasConstants", &py3_mstch_program::hasConstants},
            {"program:hasContainerTypes",
             &py3_mstch_program::hasContainerTypes},
            {"program:hasEnumTypes", &py3_mstch_program::hasEnumTypes},
            {"program:hasUnionTypes", &py3_mstch_program::hasUnionTypes},
            {"program:customTemplates", &py3_mstch_program::getCustomTemplates},
            {"program:customTypes", &py3_mstch_program::getCustomTypes},
            {"program:has_stream?", &py3_mstch_program::hasStream},
            {"program:python_capi_converter?",
             &py3_mstch_program::capi_converter},
            {"program:capi_module_prefix",
             &py3_mstch_program::capi_module_prefix},
            {"program:intercompatible?", &py3_mstch_program::intercompatible},
            {"program:auto_migrate?", &py3_mstch_program::auto_migrate},
            {"program:gen_legacy_container_converters?",
             &py3_mstch_program::legacy_container_converters},
            {"program:stream_types", &py3_mstch_program::getStreamTypes},
            {"program:response_and_stream_functions",
             &py3_mstch_program::response_and_stream_functions},
            {"program:stream_exceptions",
             &py3_mstch_program::getStreamExceptions},
            {"program:cpp_gen_path", &py3_mstch_program::getCppGenPath},
            {"program:py_deprecated_module_path",
             &py3_mstch_program::py_deprecated_module_path},
            {"program:filtered_structs", &py3_mstch_program::filtered_objects},
            {"program:filtered_typedefs",
             &py3_mstch_program::filtered_typedefs},
            {"program:inplace_migrate?", &py3_mstch_program::inplace_migrate},
            {"program:gen_py3_cython?", &py3_mstch_program::gen_py3_cython},
        });
    gather_included_program_namespaces();
    visit_types_for_services_and_interactions();
    visit_types_for_objects();
    visit_types_for_constants();
    visit_types_for_typedefs();

    for (const t_function* func : lifecycleFunctions()) {
      add_function_by_unique_return_type(
          func, visit_type(func->return_type().get_type()));
    }
  }

  mstch::node getCppGenPath() {
    return std::string(has_option("py3cpp") ? "gen-py3cpp" : "gen-cpp2");
  }

  mstch::node program_has_types() { return has_types(program_); }

  mstch::node getContainerTypes() { return make_mstch_types(containers_); }

  mstch::node hasContainerTypes() { return !containers_.empty(); }

  mstch::node needs_container_converters() {
    return !containers_.empty() && !program_->services().empty();
  }

  mstch::node hasConstants() {
    for (auto constant : program_->consts()) {
      if (!is_hidden(*constant)) {
        return true;
      }
    }
    return false;
  }

  mstch::node hasEnumTypes() { return !program_->enums().empty(); }

  mstch::node hasUnionTypes() {
    for (const auto* ttype : objects_) {
      if (ttype->is<t_union>()) {
        return true;
      }
    }
    return false;
  }

  mstch::node getCppIncludes() {
    mstch::array a;
    if (program_->language_includes().count("cpp")) {
      for (const auto& include : program_->language_includes().at("cpp")) {
        a.push_back(include);
      }
    }
    return a;
  }

  mstch::node unique_functions_by_return_type() {
    std::vector<const t_function*> functions;
    bool no_stream = has_option("no_stream");
    for (const auto& kvp : uniqueFunctionsByReturnType_) {
      if (is_func_supported(no_stream, kvp.second)) {
        functions.push_back(kvp.second);
      }
    }

    return make_mstch_functions(functions, nullptr);
  }

  mstch::node getCustomTemplates() {
    return make_mstch_types(customTemplates_);
  }

  mstch::node getCustomTypes() { return make_mstch_types(customTypes_); }

  mstch::node response_and_stream_functions() {
    return make_mstch_functions(response_and_stream_functions_, nullptr);
  }

  mstch::node getStreamExceptions() {
    std::vector<const t_type*> types;
    for (const auto& kvp : streamExceptions_) {
      types.push_back(kvp.second);
    }
    return make_mstch_types(types);
  }

  mstch::node getStreamTypes() {
    std::vector<const t_type*> types;
    if (!has_option("no_stream")) {
      for (const auto& kvp : streamTypes_) {
        types.push_back(kvp.second);
      }
    }
    return make_mstch_types(types);
  }

  mstch::node includeNamespaces() {
    mstch::array mstch_array;
    for (const auto& kvp : includeNamespaces_) {
      mstch_array.push_back(mstch::map{
          {"includeNamespace", create_string_array(kvp.second.ns)},
          {"hasServices?", kvp.second.hasServices},
          {"hasTypes?", kvp.second.hasTypes}});
    }
    return mstch_array;
  }

  mstch::node getCpp2Namespace() {
    return create_string_array(cpp2::get_gen_namespace_components(*program_));
  }

  mstch::node getPy3Namespace() {
    return create_string_array(get_py3_namespace(program_));
  }

  mstch::node hasStream() {
    return !has_option("no_stream") && !streamTypes_.empty();
  }

  mstch::node capi_converter() { return has_option("python_capi_converter"); }

  mstch::node capi_module_prefix() {
    return python::gen_capi_module_prefix_impl(program_);
  }

  mstch::node intercompatible() { return has_option("intercompatible"); }

  mstch::node auto_migrate() { return has_option("auto_migrate"); }

  mstch::node gen_py3_cython() {
    return !(has_option("auto_migrate") || has_option("inplace_migrate"));
  }

  // this option triggers generation of py3 structs as wrappers around
  // thrift-python structs
  mstch::node inplace_migrate() { return has_option("inplace_migrate"); }

  mstch::node legacy_container_converters() {
    return has_option("gen_legacy_container_converters");
  }

  mstch::node py_deprecated_module_path() {
    const std::string& module_path = program_->get_namespace("py");
    if (module_path.empty()) {
      return program_->name();
    }
    return module_path;
  }

 protected:
  struct Namespace {
    std::vector<std::string> ns;
    bool hasServices;
    bool hasTypes;
  };

  void gather_included_program_namespaces() {
    for (const auto* program : program_->get_includes_for_codegen()) {
      includeNamespaces_[program->path()] = Namespace{
          get_py3_namespace_with_name(program),
          !program->services().empty(),
          has_types(program)};
    }
  }

  void add_typedef_namespace(const t_type* type) {
    const auto* prog = type->program();
    if ((prog != nullptr) && (prog != program_)) {
      const auto& path = prog->path();
      if (includeNamespaces_.find(path) != includeNamespaces_.end()) {
        return;
      }
      includeNamespaces_[path] =
          Namespace{get_py3_namespace_with_name(prog), false, true};
    }
  }

  void add_function_by_unique_return_type(
      const t_function* function, std::string return_type_name) {
    auto sa = cpp2::is_stack_arguments(context_.options, *function);
    uniqueFunctionsByReturnType_.insert(
        {{std::move(return_type_name), sa}, function});
  }

  void visit_type_single_service(const t_service* service);

  void visit_types_for_services_and_interactions() {
    for (const auto* service : program_->services()) {
      visit_type_single_service(service);
    }
    for (const auto* interaction : program_->interactions()) {
      visit_type_single_service(interaction);
    }
  }

  void visit_types_for_objects() {
    for (t_structured* object : program_->structured_definitions()) {
      if (is_hidden(*object)) {
        continue;
      }
      for (const auto& field : object->fields()) {
        if (is_hidden(field)) {
          continue;
        }
        visit_type(field.get_type());
      }
      objects_.push_back(object);
    }
  }

  void visit_types_for_constants() {
    for (const auto& constant : program_->consts()) {
      visit_type(constant->type());
    }
  }

  void visit_types_for_typedefs() {
    for (const auto* typedef_def : program_->typedefs()) {
      if (is_hidden(*typedef_def) || is_hidden(*typedef_def->get_true_type())) {
        continue;
      }
      visit_type(typedef_def);
      typedefs_.push_back(typedef_def);
    }
  }

  std::string visit_type_impl(const t_type* orig_type, bool fromTypeDef);

  std::string visit_type(const t_type* orig_type) {
    return visit_type_impl(orig_type, orig_type->is<t_typedef>());
  }

  mstch::node filtered_objects() {
    std::string id = program_->name() + get_program_namespace(program_);
    return make_mstch_array_cached(
        objects_, *context_.struct_factory, context_.struct_cache, id);
  }

  mstch::node filtered_typedefs() { return make_mstch_typedefs(typedefs_); }

  std::vector<const t_type*> containers_;
  std::vector<const t_type*> customTemplates_;
  std::vector<const t_type*> customTypes_;
  std::unordered_set<std::string> seenTypeNames_;
  std::map<std::string, Namespace> includeNamespaces_;
  std::map<std::tuple<std::string, bool>, const t_function*>
      uniqueFunctionsByReturnType_;
  std::map<std::string, const t_type*> streamTypes_;
  std::map<std::string, const t_type*> streamExceptions_;
  std::vector<const t_structured*> objects_;
  std::vector<const t_typedef*> typedefs_;

  // Functions with a stream and an initial response.
  std::vector<const t_function*> response_and_stream_functions_;
};

class py3_mstch_service : public mstch_service {
 public:
  py3_mstch_service(
      const t_service* service,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_program* prog,
      const t_service* containing_service = nullptr)
      : mstch_service(service, ctx, pos, containing_service), prog_{prog} {
    register_methods(
        this,
        {
            {"service:externalProgram?", &py3_mstch_service::isExternalProgram},
            {"service:cppNamespaces", &py3_mstch_service::cppNamespaces},
            {"service:py3Namespaces", &py3_mstch_service::py3Namespaces},
            {"service:programName", &py3_mstch_service::programName},
            {"service:includePrefix", &py3_mstch_service::includePrefix},
            {"service:cpp_name", &py3_mstch_service::cpp_name},
            {"service:qualified_name", &py3_mstch_service::qualified_name},
            {"service:supportedFunctions",
             &py3_mstch_service::get_supported_functions},
            {"service:lifecycleFunctions",
             &py3_mstch_service::get_lifecycle_functions},
            {"service:supportedFunctionsWithLifecycle",
             &py3_mstch_service::get_supported_functions_with_lifecycle},
            {"service:supportedInteractions",
             &py3_mstch_service::get_supported_interactions},
        });

    // Collect supported interactions
    for (const auto* function : get_functions()) {
      if (function->is_interaction_constructor()) {
        supported_interactions_.insert(dynamic_cast<const t_interaction*>(
            function->interaction().get_type()));
      }
    }
  }

  mstch::node isExternalProgram() { return prog_ != service_->program(); }

  mstch::node cppNamespaces() {
    return create_string_array(
        cpp2::get_gen_namespace_components(*service_->program()));
  }

  mstch::node py3Namespaces() {
    return create_string_array(get_py3_namespace(service_->program()));
  }

  mstch::node programName() { return service_->program()->name(); }

  mstch::node includePrefix() { return service_->program()->include_prefix(); }

  mstch::node cpp_name() { return cpp2::get_name(service_); }

  mstch::node qualified_name() {
    return cpp2::get_gen_namespace(*service_->program()) +
        "::" + cpp2::get_name(service_);
  }

  std::vector<t_function*> supportedFunctions() {
    std::vector<t_function*> funcs;
    bool no_stream = has_option("no_stream");
    for (auto* func : service_->get_functions()) {
      if (is_func_supported(no_stream, func)) {
        funcs.push_back(func);
      }
    }
    return funcs;
  }

  mstch::node get_lifecycle_functions() {
    return make_mstch_functions(lifecycleFunctions(), service_);
  }

  mstch::node get_supported_functions() {
    return make_mstch_functions(supportedFunctions(), service_);
  }

  mstch::node get_supported_functions_with_lifecycle() {
    auto funcs = supportedFunctions();
    for (auto* func : lifecycleFunctions()) {
      funcs.push_back(func);
    }
    return make_mstch_functions(funcs, service_);
  }

  mstch::node get_supported_interactions() {
    return make_mstch_interactions(supported_interactions_, service_);
  }

 protected:
  const t_program* prog_;
  std::set<const t_interaction*> supported_interactions_;
};

class py3_mstch_interaction : public py3_mstch_service {
 public:
  using ast_type = t_interaction;

  py3_mstch_interaction(
      const t_interaction* interaction,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service,
      const t_program* prog)
      : py3_mstch_service(interaction, ctx, pos, prog, containing_service) {
    register_methods(
        this,
        {{"interaction:parent_service_cpp_name",
          &py3_mstch_interaction::parent_service_cpp_name}});
  }

  mstch::node parent_service_cpp_name() {
    return cpp2::get_name(containing_service_);
  }
};

class py3_mstch_function : public mstch_function {
 public:
  py3_mstch_function(
      const t_function* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_interface* iface)
      : mstch_function(f, ctx, pos, iface), cppName_(cpp2::get_name(f)) {
    register_methods(
        this,
        {
            {"function:eb", &py3_mstch_function::event_based},
            {"function:stack_arguments?", &py3_mstch_function::stack_arguments},
            {"function:cppName", &py3_mstch_function::cppName},
            {"function:modulePath", &py3_mstch_function::modulePath},
        });
  }

  mstch::node cppName() { return cppName_; }

  mstch::node event_based() {
    return function_->get_unstructured_annotation("thread") == "eb" ||
        function_->has_structured_annotation(kCppProcessInEbThreadUri) ||
        interface_->has_unstructured_annotation("process_in_event_base") ||
        interface_->has_structured_annotation(kCppProcessInEbThreadUri);
  }

  mstch::node stack_arguments() {
    return cpp2::is_stack_arguments(context_.options, *function_);
  }

  mstch::node modulePath() {
    return fmt::format(
        "_{}",
        fmt::join(get_type_py3_namespace(function_->program(), "types"), "_"));
  }

 protected:
  const std::string cppName_;
};

class py3_mstch_type : public mstch_type {
 public:
  using cached_properties = apache::thrift::compiler::python::cached_properties;

  struct context {
    const t_program* program;
    std::unordered_map<const t_type*, cached_properties>* cache;
    cpp_name_resolver* name_resolver;
  };

  cached_properties& get_cached_props(const t_type* type, const context& c);

  py3_mstch_type(
      const t_type* type,
      mstch_context& ctx,
      mstch_element_position pos,
      context c)
      : mstch_type(type->get_true_type(), ctx, pos),
        prog_(c.program),
        cached_props_(get_cached_props(type, c)) {
    register_methods(
        this,
        {
            {"type:modulePath", &py3_mstch_type::modulePath},
            {"type:module_path_period_separated",
             &py3_mstch_type::module_path_period_separated},
            {"type:module_auto_migrate_path",
             &py3_mstch_type::moduleAutoMigratePath},
            {"type:cbinding_path", &py3_mstch_type::cbinding_path},
            {"type:capi_converter_path", &py3_mstch_type::capi_converter_path},
            {"type:flat_name", &py3_mstch_type::flatName},
            {"type:cppNamespaces", &py3_mstch_type::cppNamespaces},
            {"type:cppTemplate", &py3_mstch_type::cppTemplate},
            {"type:cythonTemplate", &py3_mstch_type::cythonTemplate},
            {"type:defaultTemplate?", &py3_mstch_type::isDefaultTemplate},
            {"type:customCppType", &py3_mstch_type::customCppType},
            {"type:customCppType?", &py3_mstch_type::isCustomCppType},
            {"type:cythonCustomType", &py3_mstch_type::cythonType},
            {"type:number?", &py3_mstch_type::isNumber},
            {"type:integer?", &py3_mstch_type::isInteger},
            {"type:containerOfString?", &py3_mstch_type::isContainerOfString},
            {"type:cythonTypeNoneable?", &py3_mstch_type::cythonTypeNoneable},
            {"type:hasCythonType?", &py3_mstch_type::hasCythonType},
            {"type:iobuf?", &py3_mstch_type::isIOBuf},
            {"type:iobufRef?", &py3_mstch_type::isIOBufRef},
            {"type:iobufWrapper?", &py3_mstch_type::isIOBufWrapper},
            {"type:flexibleBinary?", &py3_mstch_type::isFlexibleBinary},
            {"type:customBinaryType?", &py3_mstch_type::isCustomBinaryType},
            {"type:simple?", &py3_mstch_type::isSimple},
            {"type:needs_convert?", &py3_mstch_type::needs_convert},
            {"type:is_container_of_struct?",
             &py3_mstch_type::is_container_of_struct},
            {"type:elem_needs_convert?",
             &py3_mstch_type::element_needs_convert},
            {"type:key_needs_convert?", &py3_mstch_type::map_key_needs_convert},
            {"type:val_needs_convert?",
             &py3_mstch_type::map_value_needs_convert},
            {"type:resolves_to_complex_return?",
             &py3_mstch_type::resolves_to_complex_return},

            {"type:need_module_path?",
             {with_no_caching, &py3_mstch_type::need_module_path}},
            {"type:need_cbinding_path?",
             {with_no_caching, &py3_mstch_type::need_cbinding_path}},
        });
  }

  mstch::node need_module_path() { return need_import_path("is_types_file"); }

  mstch::node need_cbinding_path() {
    return need_import_path("is_cbindings_file");
  }

  mstch::node modulePath() {
    return fmt::format(
        "_{}",
        fmt::join(get_type_py3_namespace(get_type_program(), "types"), "_"));
  }

  mstch::node module_path_period_separated() {
    return fmt::format(
        "{}",
        fmt::join(get_type_py3_namespace(get_type_program(), "types"), "."));
  }

  mstch::node cbinding_path() {
    return fmt::format(
        "_{}",
        fmt::join(
            get_type_py3_namespace(get_type_program(), "cbindings"), "_"));
  }

  mstch::node capi_converter_path() {
    return fmt::format(
        "_{}",
        fmt::join(
            get_type_py3_namespace(get_type_program(), "thrift_converter"),
            "_"));
  }

  mstch::node moduleAutoMigratePath() {
    return fmt::format(
        "_{}",
        fmt::join(
            get_type_py3_namespace(get_type_program(), "thrift_types"), "_"));
  }

  mstch::node flatName() { return cached_props_.flat_name(); }

  mstch::node cppNamespaces() {
    return create_string_array(get_type_cpp2_namespace());
  }

  mstch::node cppTemplate() { return cached_props_.cpp_template(); }

  mstch::node cythonTemplate() { return to_cython_template(); }

  mstch::node isDefaultTemplate() { return is_default_template(); }

  mstch::node customCppType() { return cached_props_.cpp_type(); }

  mstch::node cythonType() { return to_cython_type(); }

  mstch::node isCustomCppType() { return is_custom_cpp_type(); }

  mstch::node isNumber() { return is_number(); }

  mstch::node isInteger() { return is_integer(); }

  mstch::node isContainerOfString() {
    return is_list_of_string() || is_set_of_string();
  }

  mstch::node cythonTypeNoneable() {
    return !(is_number() || type_->is<t_container>());
  }

  mstch::node hasCythonType() { return has_cython_type(); }

  mstch::node isIOBuf() { return is_iobuf(); }

  mstch::node isIOBufRef() { return is_iobuf_ref(); }

  mstch::node isIOBufWrapper() { return is_iobuf() || is_iobuf_ref(); }

  mstch::node isFlexibleBinary() { return is_flexible_binary(); }

  mstch::node isCustomBinaryType() { return is_custom_binary_type(); }

  // types that don't have an underlying C++ type
  // i.e., structs, unions, exceptions all enclose a C++ type
  mstch::node isSimple() {
    return (type_->is<t_primitive_type>() || type_->is<t_enum>() ||
            type_->is<t_container>()) &&
        !is_custom_binary_type();
  }

  // types that need conversion to py3 if accessed from thrift-python struct
  // fields
  mstch::node needs_convert() { return type_needs_convert(type_); }

  mstch::node is_container_of_struct() {
    return type_->is<t_container>() && container_needs_convert(type_);
  }

  // type:list_elem_type etc. is defined in mstch_objects, so the returned
  // type node doesn't define type:needs_convert
  mstch::node element_needs_convert() {
    if (type_->is<t_list>()) {
      return type_needs_convert(get_list_elem_type(*type_));
    } else if (type_->is<t_set>()) {
      return type_needs_convert(get_set_elem_type(*type_));
    }
    return false;
  }

  mstch::node map_key_needs_convert() {
    if (type_->is<t_map>()) {
      return type_needs_convert(get_map_key_type(*type_));
    }
    return false;
  }

  mstch::node map_value_needs_convert() {
    if (type_->is<t_map>()) {
      return type_needs_convert(get_map_val_type(*type_));
    }
    return false;
  }

  mstch::node resolves_to_complex_return() {
    return resolved_type_->is<t_container>() ||
        resolved_type_->is_string_or_binary() ||
        resolved_type_->is<t_structured>();
  }

  const std::string& get_flat_name() const { return cached_props_.flat_name(); }

  void set_flat_name(const std::string& extra) {
    cached_props_.set_flat_name(prog_, type_, extra);
  }

  bool is_default_template() const {
    return cached_props_.is_default_template(type_);
  }

  bool is_custom_cpp_type() const { return cached_props_.cpp_type() != ""; }

 protected:
  const t_program* get_type_program() const {
    if (const t_program* p = type_->program()) {
      return p;
    }
    return prog_;
  }

  bool need_import_path(const std::string& option) {
    if (!has_option(option)) {
      return true;
    }
    if (const t_program* prog = type_->program()) {
      if (prog != prog_) {
        return true;
      }
    }
    return false;
  }

  std::vector<std::string> get_type_cpp2_namespace() const {
    return cpp2::get_gen_namespace_components(*get_type_program());
  }

  std::string to_cython_template() const {
    return cached_props_.to_cython_template();
  }

  std::string to_cython_type() const { return cached_props_.to_cython_type(); }

  bool is_integer() const { return type_->is_any_int() || type_->is_byte(); }

  bool is_number() const { return is_integer() || type_->is_floating_point(); }

  bool is_list_of_string() {
    if (!type_->is<t_list>()) {
      return false;
    }
    return get_list_elem_type(*type_)->is_string_or_binary();
  }

  bool is_set_of_string() {
    if (!type_->is<t_set>()) {
      return false;
    }
    return get_set_elem_type(*type_)->is_string_or_binary();
  }

  bool has_cython_type() const {
    return has_option("inplace_migrate")
        ? !(type_->is<t_container>() || type_->is<t_struct>() ||
            type_->is<t_union>())
        : !type_->is<t_container>();
  }

  bool is_iobuf() const { return cached_props_.cpp_type() == "folly::IOBuf"; }

  bool is_iobuf_ref() const {
    return cached_props_.cpp_type() == "std::unique_ptr<folly::IOBuf>";
  }

  bool is_flexible_binary() const {
    return type_->is_binary() && is_custom_cpp_type() && !is_iobuf() &&
        !is_iobuf_ref() &&
        // We know that folly::fbstring is completely substitutable for
        // std::string and it's a common-enough type to special-case:
        cached_props_.cpp_type() != "folly::fbstring" &&
        cached_props_.cpp_type() != "::folly::fbstring";
  }

  bool is_custom_binary_type() const {
    return is_iobuf() || is_iobuf_ref() || is_flexible_binary();
  }

  const t_program* prog_;
  cached_properties& cached_props_;
};

class py3_mstch_typedef : public mstch_typedef {
 public:
  py3_mstch_typedef(
      const t_typedef* type, mstch_context& ctx, mstch_element_position pos)
      : mstch_typedef(type, ctx, pos) {
    register_methods(
        this,
        {
            {"typedef:asType", &py3_mstch_typedef::asType},
        });
  }

  mstch::node asType() {
    return context_.type_factory->make_mstch_object(typedef_, context_);
  }
};

class py3_mstch_struct : public mstch_struct {
 public:
  py3_mstch_struct(
      const t_structured* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos) {
    register_methods(
        this,
        {
            {"struct:size", &py3_mstch_struct::getSize},
            {"struct:is_struct_orderable?",
             &py3_mstch_struct::isStructOrderable},
            {"struct:cpp_noncomparable", &py3_mstch_struct::cppNonComparable},
            {"struct:cpp_noncopyable?", &py3_mstch_struct::cppNonCopyable},
            {"struct:exception_message?",
             &py3_mstch_struct::hasExceptionMessage},
            {"struct:exception_message", &py3_mstch_struct::exceptionMessage},
            {"struct:py3_fields", &py3_mstch_struct::py3_fields},
            {"struct:py3_fields?", &py3_mstch_struct::has_py3_fields},
            {"struct:has_hidden_fields?", &py3_mstch_struct::has_hidden_fields},
            {"struct:has_defaulted_field?",
             &py3_mstch_struct::has_defaulted_field},
            {"struct:allow_inheritance?", &py3_mstch_struct::allow_inheritance},
        });
    py3_fields_ = struct_->fields().copy();
    py3_fields_.erase(
        std::remove_if(
            py3_fields_.begin(),
            py3_fields_.end(),
            [this](const t_field* field) {
              bool hidden = field->has_unstructured_annotation("py3.hidden") ||
                  field->has_structured_annotation(kPythonPy3HiddenUri);
              this->hidden_fields |= hidden;
              return hidden;
            }),
        py3_fields_.end());
  }

  mstch::node getSize() { return py3_fields_.size(); }

  mstch::node allow_inheritance() {
    return struct_->has_structured_annotation(
        kPythonMigrationBlockingAllowInheritanceUri);
  }

  mstch::node isStructOrderable() {
    return cpp2::OrderableTypeUtils::is_orderable(
               *struct_,
               !context_.options.count(
                   "disable_custom_type_ordering_if_structure_has_uri")) &&
        !struct_->has_unstructured_annotation("no_default_comparators");
  }

  mstch::node cppNonComparable() {
    return struct_->has_unstructured_annotation(
        {"cpp.noncomparable", "cpp2.noncomparable"});
  }

  mstch::node cppNonCopyable() {
    return struct_->has_unstructured_annotation(
        {"cpp.noncopyable", "cpp2.noncopyable"});
  }

  mstch::node hasExceptionMessage() {
    return !!dynamic_cast<const t_exception&>(*struct_).get_message_field();
  }

  mstch::node exceptionMessage() {
    const auto* message_field =
        dynamic_cast<const t_exception&>(*struct_).get_message_field();
    return message_field ? python::get_py3_name(*message_field) : "";
  }

  mstch::node py3_fields() { return make_mstch_fields(py3_fields_); }

  mstch::node has_py3_fields() { return !py3_fields_.empty(); }

  mstch::node has_hidden_fields() { return hidden_fields; }

  mstch::node has_defaulted_field() {
    if (struct_->is<t_union>()) {
      return false;
    }
    for (const auto& field : py3_fields_) {
      if (field->get_value()) {
        return true;
      }
    }
    return false;
  }

 private:
  std::vector<const t_field*> py3_fields_;
  bool hidden_fields = false;
};

class py3_mstch_field : public mstch_field {
 public:
  enum class RefType : uint8_t {
    NotRef,
    Unique,
    Shared,
    SharedConst,
    IOBuf,
  };
  py3_mstch_field(
      const t_field* field,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context)
      : mstch_field(field, ctx, pos, field_context),
        pyName_(python::get_py3_name(*field)),
        cppName_(cpp2::get_name(field)) {
    register_methods(
        this,
        {
            {"field:py_name", &py3_mstch_field::pyName},
            {"field:reference?", &py3_mstch_field::isRef},
            {"field:unique_ref?", &py3_mstch_field::isUniqueRef},
            {"field:shared_ref?", &py3_mstch_field::isSharedRef},
            {"field:shared_const_ref?", &py3_mstch_field::isSharedConstRef},
            {"field:iobuf_ref?", &py3_mstch_field::isIOBufRef},
            {"field:has_ref_accessor?", &py3_mstch_field::hasRefAccessor},
            {"field:hasDefaultValue?", &py3_mstch_field::hasDefaultValue},
            {"field:optional_default?",
             &py3_mstch_field::has_optional_default_value},
            {"field:user_default_value", &py3_mstch_field::user_default_value},
            {"field:PEP484Optional?", &py3_mstch_field::isPEP484Optional},
            {"field:isset?", &py3_mstch_field::isSet},
            {"field:cppName", &py3_mstch_field::cppName},
            {"field:hasModifiedName?", &py3_mstch_field::hasModifiedName},
            {"field:hasPyName?", &py3_mstch_field::hasPyName},
            {"field:boxed_ref?", &py3_mstch_field::boxed_ref},
            {"field:has_ref_api?", &py3_mstch_field::hasRefApi},
        });
  }

  mstch::node isRef() { return is_ref(); }

  mstch::node isUniqueRef() { return get_ref_type() == RefType::Unique; }

  mstch::node isSharedRef() { return get_ref_type() == RefType::Shared; }

  mstch::node isSharedConstRef() {
    return get_ref_type() == RefType::SharedConst;
  }

  mstch::node isIOBufRef() { return get_ref_type() == RefType::IOBuf; }

  mstch::node hasRefAccessor() {
    auto ref_type = get_ref_type();
    return (ref_type == RefType::NotRef || ref_type == RefType::IOBuf);
  }

  mstch::node hasDefaultValue() { return has_default_value(); }

  mstch::node isPEP484Optional() { return !has_default_value(); }

  mstch::node isSet() {
    auto ref_type = get_ref_type();
    return (ref_type == RefType::NotRef || ref_type == RefType::IOBuf) &&
        field_->get_req() != t_field::e_req::required;
  }

  mstch::node pyName() { return pyName_; }
  mstch::node cppName() { return cppName_; }
  mstch::node hasModifiedName() { return pyName_ != cppName_; }
  mstch::node hasPyName() { return pyName_ != field_->name(); }

  bool has_default_value() {
    return !is_ref() && (field_->get_value() != nullptr || !is_optional_());
  }

  bool has_optional_default_value() {
    return is_optional_() && field_->get_value() != nullptr;
  }

  mstch::node user_default_value() {
    const t_const_value* value = field_->get_value();
    if (!value) {
      return mstch::node();
    }
    if (value->is_empty()) {
      auto true_type = field_->get_type()->get_true_type();
      if ((true_type->is<t_list>() || true_type->is<t_set>()) &&
          value->kind() != t_const_value::CV_LIST) {
        const_cast<t_const_value*>(value)->convert_empty_map_to_list();
      }
      if (true_type->is<t_map>() && value->kind() != t_const_value::CV_MAP) {
        const_cast<t_const_value*>(value)->convert_empty_list_to_map();
      }
    }
    return context_.const_value_factory->make_mstch_object(
        value, context_, pos_, nullptr, nullptr);
  }

  mstch::node boxed_ref() {
    return gen::cpp::find_ref_type(*field_) == gen::cpp::reference_type::boxed;
  }

  mstch::node hasRefApi() {
    const t_structured* parentStruct = mstch_field::field_context_->strct;
    return generate_legacy_api(*parentStruct);
  }

 protected:
  RefType get_ref_type() {
    if (ref_type_cached_) {
      return ref_type_;
    }
    ref_type_cached_ = true;
    switch (gen::cpp::find_ref_type(*field_)) {
      case gen::cpp::reference_type::unique: {
        return ref_type_ = RefType::Unique;
      }
      case gen::cpp::reference_type::shared_const: {
        return ref_type_ = RefType::SharedConst;
      }
      case gen::cpp::reference_type::shared_mutable: {
        return ref_type_ = RefType::Shared;
      }
      case gen::cpp::reference_type::boxed_intern:
      case gen::cpp::reference_type::boxed: {
        return ref_type_ = RefType::NotRef;
      }
      case gen::cpp::reference_type::none: {
        const t_type* resolved_type = field_->get_type()->get_true_type();
        if (cpp2::get_type(resolved_type) == "std::unique_ptr<folly::IOBuf>") {
          return ref_type_ = RefType::IOBuf;
        }
        return ref_type_ = RefType::NotRef;
      }
    }
    // Suppress "control reaches end of non-void function" warning
    throw std::logic_error{"Unhandled ref_type"};
  }

  bool is_ref() { return get_ref_type() != RefType::NotRef; }

  RefType ref_type_{RefType::NotRef};
  bool ref_type_cached_ = false;
  const std::string pyName_;
  const std::string cppName_;
};

class py3_mstch_enum : public mstch_enum {
 public:
  py3_mstch_enum(
      const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum(e, ctx, pos) {
    register_methods(
        this,
        {
            {"enum:flags?", &py3_mstch_enum::hasFlags},
            {"enum:cpp_name", &py3_mstch_enum::cpp_name},
        });
  }

  mstch::node hasFlags() {
    return enum_->has_unstructured_annotation("py3.flags") ||
        enum_->has_structured_annotation(kPythonFlagsUri);
  }

  mstch::node cpp_name() { return cpp2::get_name(enum_); }
};

class py3_mstch_enum_value : public mstch_enum_value {
 public:
  py3_mstch_enum_value(
      const t_enum_value* ev, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum_value(ev, ctx, pos) {
    register_methods(
        this,
        {
            {"enum_value:py_name", &py3_mstch_enum_value::pyName},
            {"enum_value:cppName", &py3_mstch_enum_value::cppName},
            {"enum_value:hasPyName?", &py3_mstch_enum_value::hasPyName},
        });
  }

  mstch::node pyName() { return python::get_py3_name(*enum_value_); }

  mstch::node cppName() { return cpp2::get_name(enum_value_); }

  mstch::node hasPyName() {
    return python::get_py3_name(*enum_value_) != enum_value_->name();
  }
};

class py3_mstch_const_value : public mstch_const_value {
 public:
  py3_mstch_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type) {
    register_methods(
        this,
        {
            {"value:value_for_bool?", &py3_mstch_const_value::is_bool_value},
            {"value:value_for_floating_point?",
             &py3_mstch_const_value::is_float_value},
            {"value:py3_binary?", &py3_mstch_const_value::is_binary},
            {"value:unicode_value", &py3_mstch_const_value::unicode_value},
            {"value:const_enum_type", &py3_mstch_const_value::const_enum_type},
            {"value:py3_enum_value_name",
             &py3_mstch_const_value::py3_enum_value_name},
            {"value:const_enum_type", &py3_mstch_const_value::const_enum_type},
            {"value:const_container_type",
             &py3_mstch_const_value::const_container_type},
        });
  }

  mstch::node is_bool_value() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is_bool();
    }
    return false;
  }

  mstch::node is_float_value() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is_floating_point();
    }
    return false;
  }

  mstch::node is_binary() {
    auto& ttype = const_value_->ttype();
    return type_ == cv::CV_STRING && ttype &&
        ttype->get_true_type()->is_binary();
  }

  /*
   * Use this function (instead of the version used by C++) to render unicode
   * strings, i.e., normal python strings "".
   * For binary bytes b"", use string_value, which has octal escapes for
   * unicode characters.
   */
  mstch::node unicode_value() {
    if (type_ != cv::CV_STRING) {
      return {};
    }
    return get_escaped_string<nonascii_handling::no_escape>(
        const_value_->get_string());
  }

  mstch::node py3_enum_value_name() {
    if (!const_value_->is_enum() || const_value_->get_enum_value() == nullptr) {
      return mstch::node();
    }
    const auto& enum_name = const_value_->get_enum()->name();
    return python::get_py3_name_class_scope(
        *const_value_->get_enum_value(), enum_name);
  }

  mstch::node const_enum_type() {
    if (!const_value_->ttype() || type_ != cv::CV_INTEGER ||
        !const_value_->is_enum()) {
      return {};
    }
    const auto* type = const_value_->ttype()->get_true_type();
    if (type->is<t_enum>()) {
      return context_.type_factory->make_mstch_object(type, context_);
    }
    return {};
  }

  mstch::node const_container_type() {
    if (!const_value_->ttype()) {
      return {};
    }
    const auto* type = const_value_->ttype()->get_true_type();
    if (type->is<t_container>()) {
      return context_.type_factory->make_mstch_object(type, context_);
    }
    return {};
  }
};

class py3_mstch_deprecated_annotation : public mstch_deprecated_annotation {
 public:
  py3_mstch_deprecated_annotation(
      const t_annotation* a, mstch_context& ctx, mstch_element_position pos)
      : mstch_deprecated_annotation(a, ctx, pos) {
    register_methods(
        this,
        {
            {"annotation:value?", &py3_mstch_deprecated_annotation::has_value},
            {"annotation:py_quoted_key",
             &py3_mstch_deprecated_annotation::py_quoted_key},
            {"annotation:py_quoted_value",
             &py3_mstch_deprecated_annotation::py_quoted_value},
        });
  }

  mstch::node has_value() { return !val_.value.empty(); }
  mstch::node py_quoted_key() { return to_python_string_literal(key_); }
  mstch::node py_quoted_value() { return to_python_string_literal(val_.value); }

 protected:
  std::string to_python_string_literal(std::string val) const {
    std::string quotes = R"(""")";
    boost::algorithm::replace_all(val, "\\", "\\\\");
    boost::algorithm::replace_all(val, "\"", "\\\"");
    return quotes + val + quotes;
  }
};

std::string py3_mstch_program::visit_type_impl(
    const t_type* orig_type, bool fromTypeDef) {
  bool hasPy3EnableCppAdapterAnnot =
      orig_type->has_structured_annotation(kPythonPy3EnableCppAdapterUri);
  auto trueType = orig_type->get_true_type();
  auto baseType = context_.type_factory->make_mstch_object(orig_type, context_);
  py3_mstch_type* type = dynamic_cast<py3_mstch_type*>(baseType.get());
  const std::string& flatName = type->get_flat_name();
  // Import all types either beneath a typedef, even if the current type is
  // not directly a typedef
  fromTypeDef = fromTypeDef || orig_type->is<t_typedef>();
  if (flatName.empty()) {
    std::string extra;
    if (trueType->is<t_list>()) {
      extra = "List__" +
          visit_type_impl(get_list_elem_type(*trueType), fromTypeDef);
    } else if (trueType->is<t_set>()) {
      extra =
          "Set__" + visit_type_impl(get_set_elem_type(*trueType), fromTypeDef);
    } else if (trueType->is<t_map>()) {
      extra = "Map__" +
          visit_type_impl(get_map_key_type(*trueType), fromTypeDef) + "_" +
          visit_type_impl(get_map_val_type(*trueType), fromTypeDef);
    } else if (trueType->is_binary()) {
      extra = "binary";
    } else {
      extra = trueType->name();
    }
    type->set_flat_name(std::move(extra));
  }
  assert(!flatName.empty());
  // If this type or a parent of this type is a typedef,
  // then add the namespace of the *resolved* type:
  // (parent matters if you have eg. typedef list<list<type>>)
  if (fromTypeDef) {
    add_typedef_namespace(trueType);
  }
  bool inserted = seenTypeNames_.insert(flatName).second;
  if (inserted) {
    if (trueType->is<t_container>()) {
      containers_.push_back(hasPy3EnableCppAdapterAnnot ? orig_type : trueType);
    }
    if (!type->is_default_template()) {
      customTemplates_.push_back(trueType);
    }
    if (type->is_custom_cpp_type()) {
      customTypes_.push_back(
          hasPy3EnableCppAdapterAnnot ? orig_type : trueType);
    }
  }
  return flatName;
}

// Generator-specific validator that enforces that a reserved key is not used
// as a namespace component.
void validate_no_reserved_key_in_namespace(
    sema_context& ctx, const t_program& prog) {
  auto namespace_tokens = get_py3_namespace(&prog);
  if (namespace_tokens.empty()) {
    return;
  }
  for (const auto& component : namespace_tokens) {
    if (get_python_reserved_names().find(component) !=
        get_python_reserved_names().end()) {
      ctx.report(
          prog,
          "reserved-keyword-in-namespace-rule",
          diagnostic_level::error,
          "Namespace '{}' contains reserved keyword '{}'",
          fmt::join(namespace_tokens, "."),
          component);
    }
  }

  std::vector<std::string> components;
  boost::split(components, prog.path(), boost::is_any_of("\\/."));
  for (const auto& component : components) {
    if (component == "include") {
      ctx.report(
          prog,
          "no-reserved-keyword-in-namespace",
          diagnostic_level::error,
          "Path '{}' contains reserved keyword 'include'",
          prog.path());
    }
  }
}

// Generator-specific validator that enforces "name" and "value" are not used
// as enum member or union field names (thrift-py3).
namespace enum_member_union_field_names_validator {
void validate(const t_named& node, const std::string& name, sema_context& ctx) {
  auto pyname = node.get_unstructured_annotation("py3.name", &name);
  if (const t_const* annot =
          node.find_structured_annotation_or_null(kPythonNameUri)) {
    if (auto annotation_name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      pyname = annotation_name->get_string();
    }
  }
  if (pyname == "name" || pyname == "value") {
    ctx.report(
        node,
        "enum-member-union-field-names-rule",
        diagnostic_level::error,
        "'{}' should not be used as an enum/union field name in thrift-py3. "
        "Use a different name or annotate the field with "
        "`(py3.name=\"<new_py_name>\")`",
        pyname);
  }
}
bool validate_enum(sema_context& ctx, const t_enum& enm) {
  for (const t_enum_value& ev : enm.values()) {
    validate(ev, ev.name(), ctx);
  }
  return true;
}

bool validate_union(sema_context& ctx, const t_union& s) {
  for (const t_field& f : s.fields()) {
    validate(f, f.name(), ctx);
  }
  return true;
}

} // namespace enum_member_union_field_names_validator

void py3_mstch_program::visit_type_single_service(const t_service* service) {
  for (const auto& function : service->functions()) {
    if (is_hidden(function)) {
      continue;
    }

    for (const auto& field : function.params().fields()) {
      visit_type(field.get_type());
    }
    const t_stream* stream = function.stream();
    if (const t_throws* exceptions = stream ? stream->exceptions() : nullptr) {
      for (const t_field& field : exceptions->fields()) {
        const t_type* exType = field.get_type();
        streamExceptions_.emplace(visit_type(field.get_type()), exType);
      }
    }
    for (const t_field& field : get_elems(function.exceptions())) {
      visit_type(field.get_type());
    }

    std::string return_type_name;
    if (stream && !function.is_interaction_constructor()) {
      return_type_name = "Stream__";
      const t_type* elem_type = stream->elem_type().get_type();
      if (function.has_return_type()) {
        return_type_name = "ResponseAndStream__" +
            visit_type(function.return_type().get_type()) + "_";
      }
      std::string elem_type_name = visit_type(elem_type);
      return_type_name += elem_type_name;
      auto base_type = context_.type_factory->make_mstch_object(
          stream->elem_type().get_type(), context_);
      streamTypes_.emplace(elem_type_name, elem_type);
      bool inserted = seenTypeNames_.insert(return_type_name).second;
      if (inserted && function.has_return_type()) {
        response_and_stream_functions_.push_back(&function);
      }
    } else if (!function.sink()) {
      const auto type = function.is_interaction_constructor()
          ? function.interaction()
          : function.return_type();
      return_type_name = visit_type(type.get_type());
    }
    add_function_by_unique_return_type(&function, std::move(return_type_name));
  }
}

class t_mstch_py3_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "service:autogen_path", "function:stream?"};
    return opts;
  }

  std::string template_prefix() const override { return "py3"; }

  void generate_program() override {
    generateRootPath_ = package_to_path();
    out_dir_base_ = "gen-py3";
    auto include_prefix = get_option("include_prefix").value_or("");
    if (!include_prefix.empty()) {
      program_->set_include_prefix(std::move(include_prefix));
    }
    set_mstch_factories();
    generate_init_files();
    generate_types();
    generate_services();
  }

  void fill_validator_visitors(ast_validator& validator) const override {
    validator.add_program_visitor(validate_no_reserved_key_in_namespace);
    validator.add_enum_visitor(
        enum_member_union_field_names_validator::validate_enum);
    validator.add_union_visitor(
        enum_member_union_field_names_validator::validate_union);
  }

  enum class FileType { CBindingsFile, TypesFile, NotTypesFile };

 private:
  bool should_resolve_typedefs() const override { return true; }
  void set_mstch_factories();
  void generate_init_files();
  void generate_file(
      const std::string& template_name,
      FileType file_type,
      const std::filesystem::path& base);
  void generate_types();
  void generate_services();
  std::filesystem::path package_to_path();

  std::filesystem::path generateRootPath_;
  std::unordered_map<const t_type*, py3_mstch_type::cached_properties>
      type_props_cache_;
  cpp_name_resolver cpp_name_resolver_;
};

py3_mstch_type::cached_properties& py3_mstch_type::get_cached_props(
    const t_type* type, const py3_mstch_type::context& c) {
  // @python.Py3EnableCppAdapter treats C++ Adapter on typedef as a custom
  // cpp.type.
  auto true_type = type->get_true_type();
  if (type->has_structured_annotation(kPythonPy3EnableCppAdapterUri)) {
    return c.cache
        ->emplace(
            type,
            py3_mstch_type::cached_properties{
                get_cpp_template(*true_type),
                c.name_resolver->get_native_type(*type),
                {}})
        .first->second;
  }
  auto it = c.cache->find(true_type);
  if (it == c.cache->end()) {
    it = c.cache
             ->emplace(
                 true_type,
                 py3_mstch_type::cached_properties{
                     get_cpp_template(*true_type),
                     fmt::to_string(cpp2::get_type(true_type)),
                     {}})
             .first;
  }
  return it->second;
}

void t_mstch_py3_generator::set_mstch_factories() {
  mstch_context_.add<py3_mstch_program>();
  mstch_context_.add<py3_mstch_service>(program_);
  mstch_context_.add<py3_mstch_interaction>(program_);
  mstch_context_.add<py3_mstch_function>();
  mstch_context_.add<py3_mstch_type>(py3_mstch_type::context{
      program_, &type_props_cache_, &cpp_name_resolver_});
  mstch_context_.add<py3_mstch_typedef>();
  mstch_context_.add<py3_mstch_struct>();
  mstch_context_.add<py3_mstch_field>();
  mstch_context_.add<py3_mstch_enum>();
  mstch_context_.add<py3_mstch_enum_value>();
  mstch_context_.add<py3_mstch_const_value>();
  mstch_context_.add<py3_mstch_deprecated_annotation>();
}

void t_mstch_py3_generator::generate_init_files() {
  std::filesystem::path p = generateRootPath_;
  auto mstch_program = make_mstch_program_cached(get_program(), mstch_context_);
  while (!p.empty()) {
    render_to_file(
        mstch_program, "common/auto_generated_py", p / "__init__.py");
    p = p.parent_path();
  }
}

std::filesystem::path t_mstch_py3_generator::package_to_path() {
  auto package = get_py3_namespace(get_program());
  std::filesystem::path path;
  for (const auto& path_part : package) {
    path /= path_part;
  }
  return path;
}

void t_mstch_py3_generator::generate_file(
    const std::string& template_name,
    FileType file_type,
    const std::filesystem::path& base = {}) {
  t_program* program = get_program();
  const std::string& program_name = program->name();

  mstch_context_.set_or_erase_option(
      (file_type == FileType::TypesFile) ||
          (file_type == FileType::CBindingsFile),
      "is_types_file",
      "");

  mstch_context_.set_or_erase_option(
      file_type == FileType::CBindingsFile, "is_cbindings_file", "");

  std::shared_ptr<mstch_base> mstch_program =
      make_mstch_program_cached(program, mstch_context_);
  render_to_file(
      mstch_program,
      template_name,
      base / program_name / template_name // (output) path
  );
}

void t_mstch_py3_generator::generate_types() {
  std::vector<std::string> autoMigrateFilesWithTypeContext{
      "types.py",
      "types_auto_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "types_auto_migrated.py",
  };

  std::vector<std::string> autoMigrateFilesNoTypeContext{
      "metadata.py",
  };

  std::vector<std::string> converterFiles{
      "converter.pxd",
      "converter.pyx",
  };

  std::vector<std::string> cythonFilesWithTypeContext{
      "types.pyx",
      "types.pxd",
      "types.pyi",
  };

  std::vector<std::string> cythonFilesNoTypeContext{
      "builders.py",
      "constants_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "containers_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "metadata.pxd",
      "metadata.pyi",
      "metadata.pyx",
      "types_auto_migrated.py",
      "types_empty.pyx",
      "types_fields.pxd",
      "types_fields.pyx",
      "types_impl_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "types_reflection.py",
  };

  std::vector<std::string> cppFilesWithTypeContext{
      "types.h",
  };

  std::vector<std::string> cppFilesWithNoTypeContext{
      "metadata.h",
      "metadata.cpp",
  };

  generate_file("cbindings.pxd", FileType::CBindingsFile, generateRootPath_);

  if (has_option("enable_container_pickling_DO_NOT_USE")) {
    generate_file("__init__.py", FileType::TypesFile, generateRootPath_);
  }
  if (has_option("inplace_migrate")) {
    generate_file(
        "types_inplace_FBTHRIFT_ONLY_DO_NOT_USE.py",
        FileType::TypesFile,
        generateRootPath_);
  }
  for (const auto& file : converterFiles) {
    generate_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  // - if auto_migrate is present, generate types.pxd, and types.py
  // - else, just generate normal cython files
  for (const auto& file : autoMigrateFilesWithTypeContext) {
    generate_file(file, FileType::TypesFile, generateRootPath_);
  }
  for (const auto& file : autoMigrateFilesNoTypeContext) {
    generate_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cythonFilesWithTypeContext) {
    generate_file(file, FileType::TypesFile, generateRootPath_);
  }
  for (const auto& file : cppFilesWithTypeContext) {
    generate_file(file, FileType::TypesFile);
  }
  for (const auto& file : cythonFilesNoTypeContext) {
    generate_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cppFilesWithNoTypeContext) {
    generate_file(file, FileType::NotTypesFile);
  }
}

void t_mstch_py3_generator::generate_services() {
  if (get_program()->services().empty() && !has_option("single_file_service")) {
    // There is no need to generate empty / broken code for non existent
    // services. However, in single_file_service mode, the build system may
    // not know ahead of time if these files can exist - so we should always
    // generate them.
    return;
  }

  std::vector<std::string> pythonFiles{
      "clients.py",
      "services.py",
  };

  std::vector<std::string> normalCythonFiles{
      "clients.pxd",
      "clients.pyx",
      "clients.pyi",
      "services.pxd",
      "services.pyx",
      "services.pyi",
  };

  std::vector<std::string> cythonFiles{
      "clients_wrapper.pxd",
      "services_wrapper.pxd",
      "services_interface.pxd",
  };

  std::vector<std::string> cppFiles{
      "clients_wrapper.h",
      "clients_wrapper.cpp",
      "services_wrapper.h",
      "services_wrapper.cpp",
  };

  // TODO this logic is a complete mess and I intend to clean it up later
  // the gist is:
  // - if auto_migrate is present, generate py3_clients and clients.px
  // - if auto_migrate isn't present, just generate all the normal files

  for (const auto& file : pythonFiles) {
    generate_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : normalCythonFiles) {
    generate_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cppFiles) {
    generate_file(file, FileType::NotTypesFile);
  }
  for (const auto& file : cythonFiles) {
    generate_file(file, FileType::NotTypesFile, generateRootPath_);
  }
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_py3,
    "Python 3",
    "    include_prefix:  Use full include paths in generated files.\n");

} // namespace apache::thrift::compiler
