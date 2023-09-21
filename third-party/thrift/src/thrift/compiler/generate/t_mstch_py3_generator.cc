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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <fmt/format.h>

#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/gen/cpp/reference_type.h>
#include <thrift/compiler/gen/cpp/type_resolver.h>
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

std::vector<t_function*> lifecycleFunctions() {
  static t_function onStartServing_{
      nullptr, t_base_type::t_void(), "onStartServing"};
  static t_function onStopRequested_{
      nullptr, t_base_type::t_void(), "onStopRequested"};

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
  assert(type.is_list());
  return dynamic_cast<const t_list&>(type).get_elem_type();
}

const t_type* get_set_elem_type(const t_type& type) {
  assert(type.is_set());
  return dynamic_cast<const t_set&>(type).get_elem_type();
}

const t_type* get_map_key_type(const t_type& type) {
  assert(type.is_map());
  return dynamic_cast<const t_map&>(type).get_key_type();
}

const t_type* get_map_val_type(const t_type& type) {
  assert(type.is_map());
  return dynamic_cast<const t_map&>(type).get_val_type();
}

std::string get_cpp_template(const t_type& type) {
  if (const auto* val =
          type.find_annotation_or_null({"cpp.template", "cpp2.template"})) {
    return *val;
  }
  if (type.is_list()) {
    return "std::vector";
  }
  if (type.is_set()) {
    return "std::set";
  }
  if (type.is_map()) {
    return "std::map";
  }

  return {};
}

bool is_hidden(const t_named& node) {
  return node.has_annotation("py3.hidden") ||
      node.find_structured_annotation_or_null(kPythonPy3HiddenUri);
}

bool is_func_supported(bool no_stream, const t_function* func) {
  return !is_hidden(*func) && !(no_stream && func->stream()) && !func->sink() &&
      !func->return_type()->is_service();
}

bool is_hidden(const t_type& node) {
  return node.generated() ||
      gen::cpp::type_resolver::is_directly_adapted(node) ||
      node.has_annotation("py3.hidden") ||
      node.find_structured_annotation_or_null(kPythonPy3HiddenUri);
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
            {"program:cppNamespaces", &py3_mstch_program::getCpp2Namespace},
            {"program:py3Namespaces", &py3_mstch_program::getPy3Namespace},
            {"program:includeNamespaces",
             &py3_mstch_program::includeNamespaces},
            {"program:cppIncludes", &py3_mstch_program::getCppIncludes},
            {"program:containerTypes", &py3_mstch_program::getContainerTypes},
            {"program:customTemplates", &py3_mstch_program::getCustomTemplates},
            {"program:customTypes", &py3_mstch_program::getCustomTypes},
            {"program:moveContainerTypes",
             &py3_mstch_program::getMoveContainerTypes},
            {"program:has_stream?", &py3_mstch_program::hasStream},
            {"program:python_capi_converter?",
             &py3_mstch_program::capi_converter},
            {"program:intercompatible?", &py3_mstch_program::intercompatible},
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
        });
    gather_included_program_namespaces();
    visit_types_for_services_and_interactions();
    visit_types_for_objects();
    visit_types_for_constants();
    visit_types_for_typedefs();
    visit_types_for_mixin_fields();

    for (const t_function* func : lifecycleFunctions()) {
      add_function_by_unique_return_type(func, visit_type(func->return_type()));
    }
  }

  mstch::node getCppGenPath() {
    return std::string(has_option("py3cpp") ? "gen-py3cpp" : "gen-cpp2");
  }

  mstch::node getContainerTypes() { return make_mstch_types(containers_); }

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

    return make_mstch_functions(functions);
  }

  mstch::node getCustomTemplates() {
    return make_mstch_types(customTemplates_);
  }

  mstch::node getCustomTypes() { return make_mstch_types(customTypes_); }

  mstch::node getMoveContainerTypes() {
    std::vector<const t_type*> types;
    for (const auto& kvp : moveContainers_) {
      types.push_back(kvp.second);
    }
    return make_mstch_types(types);
  }

  mstch::node response_and_stream_functions() {
    return make_mstch_functions(response_and_stream_functions_);
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

  mstch::node intercompatible() { return has_option("intercompatible"); }

  mstch::node py_deprecated_module_path() {
    std::string module_path = program_->get_namespace("py");
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
      visit_type(typedef_def->get_type());
      typedefs_.push_back(typedef_def);
    }
  }

  void visit_types_for_mixin_fields() {
    for (const auto* strct : program_->structs_and_unions()) {
      if (is_hidden(*strct)) {
        continue;
      }
      for (const auto& m : cpp2::get_mixins_and_members(*strct)) {
        visit_type(m.member->get_type());
      }
    }
  }

  enum TypeDef { NoTypedef, HasTypedef };

  std::string visit_type(const t_type* orig_type) {
    return visit_type_with_typedef(orig_type, TypeDef::NoTypedef);
  }

  std::string visit_type_with_typedef(
      const t_type* orig_type, TypeDef isTypedef);

  mstch::node filtered_objects() {
    std::string id = program_->name() + get_program_namespace(program_);
    return make_mstch_array_cached(
        objects_, *context_.struct_factory, context_.struct_cache, id);
  }

  mstch::node filtered_typedefs() { return make_mstch_typedefs(typedefs_); }

  std::vector<const t_type*> containers_;
  std::map<std::string, const t_type*> moveContainers_;
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
      const t_program* prog)
      : mstch_service(service, ctx, pos), prog_{prog} {
    register_methods(
        this,
        {
            {"service:externalProgram?", &py3_mstch_service::isExternalProgram},
            {"service:cppNamespaces", &py3_mstch_service::cppNamespaces},
            {"service:py3Namespaces", &py3_mstch_service::py3Namespaces},
            {"service:programName", &py3_mstch_service::programName},
            {"service:includePrefix", &py3_mstch_service::includePrefix},
            {"service:cpp_name", &py3_mstch_service::cpp_name},
            {"service:parent_service_name",
             &py3_mstch_service::parent_service_name},
            {"service:parent_service_cpp_name",
             &py3_mstch_service::parent_service_cpp_name},
            {"service:qualified_name", &py3_mstch_service::qualified_name},
            {"service:supportedFunctions",
             &py3_mstch_service::get_supported_functions},
            {"service:lifecycleFunctions",
             &py3_mstch_service::get_lifecycle_functions},
            {"service:supportedFunctionsWithLifecycle",
             &py3_mstch_service::get_supported_functions_with_lifecycle},
        });
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

  mstch::node parent_service_name() {
    return context_.options.at("parent_service_name");
  }

  mstch::node parent_service_cpp_name() {
    return context_.options.at("parent_service_cpp_name");
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
    return make_mstch_functions(lifecycleFunctions());
  }

  mstch::node get_supported_functions() {
    return make_mstch_functions(supportedFunctions());
  }

  mstch::node get_supported_functions_with_lifecycle() {
    auto funcs = supportedFunctions();
    for (auto* func : lifecycleFunctions()) {
      funcs.push_back(func);
    }
    return make_mstch_functions(funcs);
  }

 protected:
  const t_program* prog_;
};

class py3_mstch_function : public mstch_function {
 public:
  py3_mstch_function(
      const t_function* f, mstch_context& ctx, mstch_element_position pos)
      : mstch_function(f, ctx, pos), cppName_(cpp2::get_name(f)) {
    register_methods(
        this,
        {{"function:eb", &py3_mstch_function::event_based},
         {"function:stack_arguments?", &py3_mstch_function::stack_arguments},
         {"function:cppName", &py3_mstch_function::cppName}});
  }

  mstch::node cppName() { return cppName_; }

  mstch::node event_based() {
    return function_->get_annotation("thread") == "eb";
  }

  mstch::node stack_arguments() {
    return cpp2::is_stack_arguments(context_.options, *function_);
  }

 protected:
  const std::string cppName_;
};

class py3_mstch_type : public mstch_type {
 public:
  struct CachedProperties {
    const std::string cppTemplate;
    std::string cppType;
    std::string flatName;
  };

  struct data {
    const t_program* program;
    std::unordered_map<const t_type*, CachedProperties>* cache;
  };

  CachedProperties& get_cached_props(const t_type* type, const data& d);

  py3_mstch_type(
      const t_type* type,
      mstch_context& ctx,
      mstch_element_position pos,
      data d)
      : mstch_type(type->get_true_type(), ctx, pos),
        prog_(d.program),
        cached_props_(get_cached_props(type, d)) {
    strip_cpp_comments_and_newlines(cached_props_.cppType);
    register_methods(
        this,
        {
            {"type:modulePath", &py3_mstch_type::modulePath},
            {"type:need_module_path?", &py3_mstch_type::need_module_path},
            {"type:flat_name", &py3_mstch_type::flatName},
            {"type:cppNamespaces", &py3_mstch_type::cppNamespaces},
            {"type:cppTemplate", &py3_mstch_type::cppTemplate},
            {"type:cythonTemplate", &py3_mstch_type::cythonTemplate},
            {"type:defaultTemplate?", &py3_mstch_type::isDefaultTemplate},
            {"type:cppCustomType", &py3_mstch_type::cppType},
            {"type:cythonCustomType", &py3_mstch_type::cythonType},
            {"type:hasCustomType?", &py3_mstch_type::hasCustomType},
            {"type:number?", &py3_mstch_type::isNumber},
            {"type:integer?", &py3_mstch_type::isInteger},
            {"type:containerOfString?", &py3_mstch_type::isContainerOfString},
            {"type:cythonTypeNoneable?", &py3_mstch_type::cythonTypeNoneable},
            {"type:hasCythonType?", &py3_mstch_type::hasCythonType},
            {"type:iobuf?", &py3_mstch_type::isIOBuf},
            {"type:iobufRef?", &py3_mstch_type::isIOBufRef},
            {"type:iobufWrapper?", &py3_mstch_type::isIOBufWrapper},
            {"type:flexibleBinary?", &py3_mstch_type::isFlexibleBinary},
            {"type:hasCustomTypeBehavior?",
             &py3_mstch_type::hasCustomTypeBehavior},
            {"type:simple?", &py3_mstch_type::isSimple},
            {"type:resolves_to_complex_return?",
             &py3_mstch_type::resolves_to_complex_return},
        });
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

  mstch::node modulePath() {
    return "_" + boost::algorithm::join(get_type_py3_namespace(), "_");
  }

  mstch::node flatName() { return cached_props_.flatName; }

  mstch::node cppNamespaces() {
    return create_string_array(get_type_cpp2_namespace());
  }

  mstch::node cppTemplate() { return cached_props_.cppTemplate; }

  mstch::node cythonTemplate() { return to_cython_template(); }

  mstch::node isDefaultTemplate() { return is_default_template(); }

  mstch::node cppType() { return cached_props_.cppType; }

  mstch::node cythonType() { return to_cython_type(); }

  mstch::node hasCustomType() { return has_custom_cpp_type(); }

  mstch::node isNumber() { return is_number(); }

  mstch::node isInteger() { return is_integer(); }

  mstch::node isContainerOfString() {
    return is_list_of_string() || is_set_of_string();
  }

  mstch::node cythonTypeNoneable() { return !is_number() && has_cython_type(); }

  mstch::node hasCythonType() { return has_cython_type(); }

  mstch::node isIOBuf() { return is_iobuf(); }

  mstch::node isIOBufRef() { return is_iobuf_ref(); }

  mstch::node isIOBufWrapper() { return is_iobuf() || is_iobuf_ref(); }

  mstch::node isFlexibleBinary() { return is_flexible_binary(); }

  mstch::node hasCustomTypeBehavior() { return has_custom_type_behavior(); }

  mstch::node isSimple() {
    return (type_->is_base_type() || type_->is_enum()) &&
        !has_custom_type_behavior();
  }

  mstch::node resolves_to_complex_return() {
    return resolved_type_->is_container() ||
        resolved_type_->is_string_or_binary() || resolved_type_->is_struct() ||
        resolved_type_->is_exception();
  }

  const std::string& get_flat_name() const { return cached_props_.flatName; }

  void set_flat_name(std::string extra) {
    std::string custom_prefix;
    if (!is_default_template()) {
      custom_prefix = to_cython_template() + "__";
    } else {
      if (cached_props_.cppType != "") {
        custom_prefix = to_cython_type() + "__";
      }
    }
    const t_program* typeProgram = type_->program();
    if (typeProgram && typeProgram != prog_) {
      custom_prefix += typeProgram->name() + "_";
    }
    custom_prefix += extra;
    cached_props_.flatName = std::move(custom_prefix);
  }

  bool is_default_template() const {
    return (!type_->is_container() && cached_props_.cppTemplate == "") ||
        (type_->is_list() && cached_props_.cppTemplate == "std::vector") ||
        (type_->is_set() && cached_props_.cppTemplate == "std::set") ||
        (type_->is_map() && cached_props_.cppTemplate == "std::map");
  }

  bool has_custom_cpp_type() const { return cached_props_.cppType != ""; }

 protected:
  const t_program* get_type_program() const {
    if (const t_program* p = type_->program()) {
      return p;
    }
    return prog_;
  }

  std::vector<std::string> get_type_py3_namespace() const {
    auto ns = get_py3_namespace_with_name(get_type_program());
    ns.push_back("types");
    return ns;
  }

  std::vector<std::string> get_type_cpp2_namespace() const {
    return cpp2::get_gen_namespace_components(*get_type_program());
  }

  std::string to_cython_template() const {
    // handle special built-ins first:
    if (cached_props_.cppTemplate == "std::vector") {
      return "vector";
    } else if (cached_props_.cppTemplate == "std::set") {
      return "cset";
    } else if (cached_props_.cppTemplate == "std::map") {
      return "cmap";
    }
    // then default handling:
    return boost::algorithm::replace_all_copy(
        cached_props_.cppTemplate, "::", "_");
  }

  std::string to_cython_type() const {
    if (cached_props_.cppType == "") {
      return "";
    }
    std::string cython_type = cached_props_.cppType;
    boost::algorithm::replace_all(cython_type, "::", "_");
    boost::algorithm::replace_all(cython_type, "<", "_");
    boost::algorithm::replace_all(cython_type, ">", "");
    boost::algorithm::replace_all(cython_type, " ", "");
    boost::algorithm::replace_all(cython_type, ", ", "_");
    boost::algorithm::replace_all(cython_type, ",", "_");
    return cython_type;
  }

  bool is_integer() const { return type_->is_any_int() || type_->is_byte(); }

  bool is_number() const { return is_integer() || type_->is_floating_point(); }

  bool is_list_of_string() {
    if (!type_->is_list()) {
      return false;
    }
    return get_list_elem_type(*type_)->is_string_or_binary();
  }

  bool is_set_of_string() {
    if (!type_->is_set()) {
      return false;
    }
    return get_set_elem_type(*type_)->is_string_or_binary();
  }

  bool has_cython_type() const { return !type_->is_container(); }

  bool is_iobuf() const { return cached_props_.cppType == "folly::IOBuf"; }

  bool is_iobuf_ref() const {
    return cached_props_.cppType == "std::unique_ptr<folly::IOBuf>";
  }

  bool is_flexible_binary() const {
    return type_->is_binary() && has_custom_cpp_type() && !is_iobuf() &&
        !is_iobuf_ref() &&
        // We know that folly::fbstring is completely substitutable for
        // std::string and it's a common-enough type to special-case:
        cached_props_.cppType != "folly::fbstring" &&
        cached_props_.cppType != "::folly::fbstring";
  }

  bool has_custom_type_behavior() const {
    return is_iobuf() || is_iobuf_ref() || is_flexible_binary();
  }

  const t_program* prog_;
  CachedProperties& cached_props_;
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
            {"struct:fields_and_mixin_fields",
             &py3_mstch_struct::fields_and_mixin_fields},
            {"struct:py3_fields", &py3_mstch_struct::py3_fields},
            {"struct:py3_fields?", &py3_mstch_struct::has_py3_fields},
            {"struct:has_hidden_fields?", &py3_mstch_struct::has_hidden_fields},
        });
    py3_fields_ = struct_->fields().copy();
    py3_fields_.erase(
        std::remove_if(
            py3_fields_.begin(),
            py3_fields_.end(),
            [this](const t_field* field) {
              bool hidden = field->has_annotation("py3.hidden") ||
                  field->find_structured_annotation_or_null(
                      kPythonPy3HiddenUri);
              this->hidden_fields |= hidden;
              return hidden;
            }),
        py3_fields_.end());
  }

  mstch::node getSize() { return std::to_string(py3_fields_.size()); }

  mstch::node isStructOrderable() {
    return cpp2::is_orderable(*struct_) &&
        !struct_->has_annotation("no_default_comparators");
  }

  mstch::node cppNonComparable() {
    return struct_->has_annotation({"cpp.noncomparable", "cpp2.noncomparable"});
  }

  mstch::node cppNonCopyable() {
    return struct_->has_annotation({"cpp.noncopyable", "cpp2.noncopyable"});
  }

  mstch::node hasExceptionMessage() {
    return struct_->has_annotation("message");
  }

  mstch::node exceptionMessage() { return struct_->get_annotation("message"); }

  mstch::node py3_fields() { return make_mstch_fields(py3_fields_); }

  mstch::node has_py3_fields() { return !py3_fields_.empty(); }

  mstch::node has_hidden_fields() { return hidden_fields; }

  mstch::node fields_and_mixin_fields() {
    std::vector<const t_field*> fields = py3_fields_;
    for (const auto& m : cpp2::get_mixins_and_members(*struct_)) {
      fields.push_back(m.member);
    }

    return make_mstch_fields(fields);
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
        pyName_(py3::get_py3_name(*field)),
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
  mstch::node hasPyName() { return pyName_ != field_->get_name(); }

  bool has_default_value() {
    return !is_ref() && (field_->get_value() != nullptr || !is_optional_());
  }

  mstch::node boxed_ref() {
    return gen::cpp::find_ref_type(*field_) == gen::cpp::reference_type::boxed;
  }

  mstch::node hasRefApi() {
    // Mixin is a special case (T126232678) because it does not contain
    // a valid pointer to the top level struct
    bool isMixin =
        ((mstch_field::field_context_ == nullptr) ||
         (mstch_field::field_context_->strct == nullptr));
    if (isMixin) {
      return false;
    }

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
    return enum_->has_annotation("py3.flags") ||
        enum_->find_structured_annotation_or_null(kPythonFlagsUri);
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

  mstch::node pyName() { return py3::get_py3_name(*enum_value_); }

  mstch::node cppName() { return cpp2::get_name(enum_value_); }

  mstch::node hasPyName() {
    return py3::get_py3_name(*enum_value_) != enum_value_->get_name();
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
    std::string quotes = "\"\"\"";
    boost::algorithm::replace_all(val, "\\", "\\\\");
    boost::algorithm::replace_all(val, "\"", "\\\"");
    return quotes + val + quotes;
  }
};

std::string py3_mstch_program::visit_type_with_typedef(
    const t_type* orig_type, TypeDef isTypedef) {
  auto trueType = orig_type->get_true_type();
  auto baseType = context_.type_factory->make_mstch_object(trueType, context_);
  py3_mstch_type* type = dynamic_cast<py3_mstch_type*>(baseType.get());
  const std::string& flatName = type->get_flat_name();
  // Import all types either beneath a typedef, even if the current type is
  // not directly a typedef
  isTypedef = isTypedef == TypeDef::HasTypedef || orig_type->is_typedef()
      ? TypeDef::HasTypedef
      : TypeDef::NoTypedef;
  if (flatName.empty()) {
    std::string extra;
    if (trueType->is_list()) {
      extra = "List__" +
          visit_type_with_typedef(get_list_elem_type(*trueType), isTypedef);
    } else if (trueType->is_set()) {
      extra = "Set__" +
          visit_type_with_typedef(get_set_elem_type(*trueType), isTypedef);
    } else if (trueType->is_map()) {
      extra = "Map__" +
          visit_type_with_typedef(get_map_key_type(*trueType), isTypedef) +
          "_" + visit_type_with_typedef(get_map_val_type(*trueType), isTypedef);
    } else if (trueType->is_binary()) {
      extra = "binary";
    } else {
      extra = trueType->get_name();
    }
    type->set_flat_name(std::move(extra));
  }
  assert(!flatName.empty());
  // If this type or a parent of this type is a typedef,
  // then add the namespace of the *resolved* type:
  // (parent matters if you have eg. typedef list<list<type>>)
  if (isTypedef == TypeDef::HasTypedef) {
    add_typedef_namespace(trueType);
  }
  bool inserted = seenTypeNames_.insert(flatName).second;
  if (inserted) {
    if (trueType->is_container()) {
      containers_.push_back(trueType);
      moveContainers_.emplace(
          boost::algorithm::replace_all_copy(flatName, "binary", "string"),
          trueType);
    }
    if (!type->is_default_template()) {
      customTemplates_.push_back(trueType);
    }
    if (type->has_custom_cpp_type()) {
      customTypes_.push_back(trueType);
    }
  }
  return flatName;
}

// Generator-specific validator that enforces that a reserved key is not used as
// a namespace component.
class no_reserved_key_in_namespace_validator : public validator {
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
    for (const auto& component : namespace_tokens) {
      if (get_python_reserved_names().find(component) !=
          get_python_reserved_names().end()) {
        report_error(
            *prog,
            "Namespace '{}' contains reserved keyword '{}'",
            fmt::join(namespace_tokens, "."),
            component);
      }
    }

    std::vector<std::string> components;
    boost::split(components, prog->path(), boost::is_any_of("\\/."));
    for (const auto& component : components) {
      if (component == "include") {
        report_error(
            *prog,
            "Path '{}' contains reserved keyword 'include'",
            prog->path());
      }
    }
  }
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
    auto pyname = node->get_annotation("py3.name", &name);
    if (const t_const* annot =
            node->find_structured_annotation_or_null(kPythonNameUri)) {
      if (auto annotation_name =
              annot->get_value_from_structured_annotation_or_null("name")) {
        pyname = annotation_name->get_string();
      }
    }
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

void py3_mstch_program::visit_type_single_service(const t_service* service) {
  for (const auto& function : service->functions()) {
    if (is_hidden(function)) {
      continue;
    }

    for (const auto& field : function.get_paramlist()->fields()) {
      visit_type(field.get_type());
    }
    const t_stream_response* stream = function.stream();
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
    if (const t_stream_response* stream = function.stream()) {
      return_type_name = "Stream__";
      const t_type* resp_type = stream->get_first_response_type();
      const t_type* elem_type = stream->get_elem_type();
      if (resp_type) {
        return_type_name = "ResponseAndStream__" + visit_type(resp_type) + "_";
      }
      std::string elem_type_name = visit_type(elem_type);
      return_type_name += elem_type_name;
      auto base_type =
          context_.type_factory->make_mstch_object(stream, context_);
      py3_mstch_type* type = dynamic_cast<py3_mstch_type*>(base_type.get());
      type->set_flat_name(return_type_name);
      streamTypes_.emplace(elem_type_name, elem_type);
      bool inserted = seenTypeNames_.insert(return_type_name).second;
      if (inserted && resp_type) {
        response_and_stream_functions_.push_back(&function);
      }
    } else if (!function.sink()) {
      return_type_name = visit_type(function.return_type());
    }
    add_function_by_unique_return_type(&function, std::move(return_type_name));
  }
}

class t_mstch_py3_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

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

  void fill_validator_list(validator_list& vl) const override {
    vl.add<no_reserved_key_in_namespace_validator>();
    vl.add<enum_member_union_field_names_validator>();
  }

  enum TypesFile { IsTypesFile, NotTypesFile };

 private:
  bool should_resolve_typedefs() const override { return true; }
  void set_mstch_factories();
  void generate_init_files();
  void generate_file(
      const std::string& file,
      TypesFile is_types_file,
      const boost::filesystem::path& base);
  void generate_types();
  void generate_services();
  boost::filesystem::path package_to_path();

  boost::filesystem::path generateRootPath_;
  std::unordered_map<const t_type*, py3_mstch_type::CachedProperties>
      type_props_cache_;
};

py3_mstch_type::CachedProperties& py3_mstch_type::get_cached_props(
    const t_type* type, const data& d) {
  auto true_type = type->get_true_type();
  auto it = d.cache->find(true_type);
  if (it == d.cache->end()) {
    it = d.cache
             ->emplace(
                 true_type,
                 py3_mstch_type::CachedProperties{
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
  mstch_context_.add<py3_mstch_function>();
  mstch_context_.add<py3_mstch_type>(
      py3_mstch_type::data{program_, &type_props_cache_});
  mstch_context_.add<py3_mstch_struct>();
  mstch_context_.add<py3_mstch_field>();
  mstch_context_.add<py3_mstch_enum>();
  mstch_context_.add<py3_mstch_enum_value>();
  mstch_context_.add<py3_mstch_deprecated_annotation>();
}

void t_mstch_py3_generator::generate_init_files() {
  boost::filesystem::path p = generateRootPath_;
  auto mstch_program = make_mstch_program_cached(get_program(), mstch_context_);
  while (!p.empty()) {
    render_to_file(
        mstch_program, "common/auto_generated_py", p / "__init__.py");
    p = p.parent_path();
  }
}

boost::filesystem::path t_mstch_py3_generator::package_to_path() {
  auto package = get_py3_namespace(get_program());
  boost::filesystem::path path;
  for (const auto& path_part : package) {
    path /= path_part;
  }
  return path;
}

void t_mstch_py3_generator::generate_file(
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

void t_mstch_py3_generator::generate_types() {
  std::vector<std::string> cythonFilesWithTypeContext{
      "types.pyx",
      "types.pxd",
      "types.pyi",
  };

  std::vector<std::string> cythonFilesNoTypeContext{
      "types_reflection.pxd",
      "types_reflection.pyx",
      "types_fields.pxd",
      "types_fields.pyx",
      "builders.pxd",
      "builders.pyx",
      "builders.pyi",
      "metadata.pxd",
      "metadata.pyi",
      "metadata.pyx",
  };

  std::vector<std::string> cppFilesWithTypeContext{
      "types.h",
  };

  std::vector<std::string> cppFilesWithNoTypeContext{
      "metadata.h",
      "metadata.cpp",
  };

  for (const auto& file : cythonFilesWithTypeContext) {
    generate_file(file, IsTypesFile, generateRootPath_);
  }
  for (const auto& file : cppFilesWithTypeContext) {
    generate_file(file, IsTypesFile);
  }
  for (const auto& file : cythonFilesNoTypeContext) {
    generate_file(file, NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cppFilesWithNoTypeContext) {
    generate_file(file, NotTypesFile);
  }
}

void t_mstch_py3_generator::generate_services() {
  if (get_program()->services().empty() && !has_option("single_file_service")) {
    // There is no need to generate empty / broken code for non existent
    // services. However, in single_file_service mode, the build system may not
    // know ahead of time if these files can exist - so we should always
    // generate them.
    return;
  }

  std::vector<std::string> cythonFiles{
      "clients.pxd",
      "clients.pyx",
      "clients.pyi",
      "clients_wrapper.pxd",
      "services.pxd",
      "services.pyx",
      "services.pyi",
      "services_wrapper.pxd",
      "services_reflection.pxd",
      "services_reflection.pyx",
  };

  std::vector<std::string> cppFiles{
      "clients_wrapper.h",
      "clients_wrapper.cpp",
      "services_wrapper.h",
      "services_wrapper.cpp",
  };

  for (const auto& file : cythonFiles) {
    generate_file(file, NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cppFiles) {
    generate_file(file, NotTypesFile);
  }
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_py3,
    "Python 3",
    "    include_prefix:  Use full include paths in generated files.\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
