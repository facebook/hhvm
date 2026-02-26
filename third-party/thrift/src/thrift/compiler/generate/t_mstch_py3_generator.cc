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
    mstch_array.emplace_back(
        mstch::map{
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

bool type_needs_convert(const t_type* type) {
  return type->is<t_structured>() || type->is<t_container>();
}

bool container_needs_convert(const t_type* type) {
  const t_type* true_type = type->get_true_type();

  if (const t_map* map_type = dynamic_cast<const t_map*>(true_type)) {
    return container_needs_convert(&map_type->key_type().deref()) ||
        container_needs_convert(&map_type->val_type().deref());
  } else if (const t_list* list_type = dynamic_cast<const t_list*>(true_type)) {
    return container_needs_convert(list_type->elem_type().get_type());
  } else if (const t_set* set_type = dynamic_cast<const t_set*>(true_type)) {
    return container_needs_convert(set_type->elem_type().get_type());
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

enum class field_cpp_kind : uint8_t {
  /** Plain value or boxed ref with no special handling */
  value,
  unique_ptr,
  shared_ptr_mutable,
  shared_ptr_const,
  iobuf,
};

bool field_has_default_value(
    const t_field& field, const field_cpp_kind cpp_kind) {
  return cpp_kind == field_cpp_kind::value &&
      (field.default_value() != nullptr ||
       field.qualifier() != t_field_qualifier::optional);
}

std::vector<std::string> get_type_py3_namespace(
    const t_program* prog, const std::string& suffix) {
  auto ns = get_py3_namespace_with_name(prog);
  ns.push_back(suffix);
  return ns;
}

enum class FileType { CBindingsFile, TypesFile, NotTypesFile };

class py3_generator_context {
 public:
  using cached_type_properties =
      apache::thrift::compiler::python::cached_properties;

  py3_generator_context(const t_program* program, const FileType* file_type)
      : program_(program), file_type_(file_type) {}

  cached_type_properties& get_cached_type_props(const t_type* type) const;

  field_cpp_kind get_field_cpp_kind(const t_field& field) const {
    assert(find_cpp_kinds_.contains(&field));
    return find_cpp_kinds_.at(&field);
  }

  const t_program* program() const { return program_; }
  const FileType* file_type() const { return file_type_; }

  void register_visitors(t_whisker_generator::context_visitor& visitor) {
    using context = t_whisker_generator::whisker_generator_visitor_context;
    visitor.add_field_visitor([this](const context&, const t_field& field) {
      switch (gen::cpp::find_ref_type(field)) {
        case gen::cpp::reference_type::unique: {
          find_cpp_kinds_[&field] = field_cpp_kind::unique_ptr;
          return;
        }
        case gen::cpp::reference_type::shared_const: {
          find_cpp_kinds_[&field] = field_cpp_kind::shared_ptr_const;
          return;
        }
        case gen::cpp::reference_type::shared_mutable: {
          find_cpp_kinds_[&field] = field_cpp_kind::shared_ptr_mutable;
          return;
        }
        case gen::cpp::reference_type::boxed_intern:
        case gen::cpp::reference_type::boxed: {
          find_cpp_kinds_[&field] = field_cpp_kind::value;
          return;
        }
        case gen::cpp::reference_type::none: {
          const t_type* resolved_type = field.type()->get_true_type();
          find_cpp_kinds_[&field] =
              cpp2::get_type(resolved_type) == "std::unique_ptr<folly::IOBuf>"
              ? field_cpp_kind::iobuf
              : field_cpp_kind::value;
          return;
        }
        default:
          throw std::logic_error{"Unhandled ref_type"};
      }
    });
  }

 private:
  const t_program* program_;
  const FileType* file_type_;

  std::unordered_map<const t_field*, field_cpp_kind> find_cpp_kinds_;

  // These properties are mutable as they are (or contain) caches which must be
  // accessed from a const method context
  mutable cpp_name_resolver name_resolver_;
  mutable std::unordered_map<const t_type*, cached_type_properties>
      type_properties_;
};

class py3_mstch_program : public mstch_program {
 public:
  py3_mstch_program(
      const t_program* p,
      mstch_context& ctx,
      mstch_element_position pos,
      py3_generator_context* context)
      : mstch_program(p, ctx, pos), generator_context_(*context) {
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
    return !containers_.empty() &&
        (!program_->services().empty() ||
         has_option("gen_legacy_container_converters"));
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
        a.emplace_back(include);
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

  mstch::node response_and_stream_functions() {
    return make_mstch_functions(response_and_stream_functions_);
  }

  mstch::node getStreamExceptions() {
    std::vector<const t_type*> types;
    types.reserve(streamExceptions_.size());
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
      mstch_array.emplace_back(
          mstch::map{
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

  mstch::node auto_migrate() { return has_option("auto_migrate"); }

  mstch::node gen_py3_cython() {
    return !(has_option("auto_migrate") || has_option("inplace_migrate"));
  }

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
    auto sa = cpp2::is_stack_arguments(*context_.options, *function);
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
        visit_type(field.type().get_type());
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
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
    return make_mstch_array_cached(
        objects_, *context_.struct_factory, context_.struct_cache, id);
  }

  mstch::node filtered_typedefs() { return make_mstch_typedefs(typedefs_); }

  py3_generator_context& generator_context_;
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

struct interaction_name_less {
  bool operator()(const t_interaction* lhs, const t_interaction* rhs) const {
    return lhs->name() < rhs->name();
  }
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
    for (const auto& function : service_->functions()) {
      if (function.is_interaction_constructor()) {
        supported_interactions_.insert(
            &function.interaction()->as<t_interaction>());
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

  mstch::node qualified_name() {
    return cpp2::get_gen_namespace(*service_->program()) +
        "::" + cpp2::get_name(service_);
  }

  std::vector<const t_function*> supportedFunctions() {
    std::vector<const t_function*> funcs;
    bool no_stream = has_option("no_stream");
    for (const auto& func : service_->functions()) {
      if (is_func_supported(no_stream, &func)) {
        funcs.push_back(&func);
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

  mstch::node get_supported_interactions() {
    return make_mstch_interactions(supported_interactions_, service_);
  }

 protected:
  const t_program* prog_;
  std::set<const t_interaction*, interaction_name_less> supported_interactions_;
};

class py3_mstch_type : public mstch_type {
 public:
  py3_mstch_type(
      const t_type* type,
      mstch_context& ctx,
      mstch_element_position pos,
      py3_generator_context* c)
      : mstch_type(type, ctx, pos),
        prog_(c->program()),
        file_type_(c->file_type()),
        cached_props_(c->get_cached_type_props(type)) {
    register_methods(
        this,
        {
            {"type:module_auto_migrate_path",
             &py3_mstch_type::moduleAutoMigratePath},
            {"type:cbinding_path", &py3_mstch_type::cbinding_path},
            {"type:capi_converter_path", &py3_mstch_type::capi_converter_path},
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

            {"type:need_cbinding_path?",
             {with_no_caching, &py3_mstch_type::need_cbinding_path}},
        });
  }

  mstch::node need_cbinding_path() {
    // Need import if in a different declaration file, or type originated in a
    // different Thrift program
    return *file_type_ != FileType::CBindingsFile ||
        (resolved_type_->program() != nullptr &&
         resolved_type_->program() != prog_);
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

  mstch::node cppNamespaces() {
    return create_string_array(get_type_cpp2_namespace());
  }

  mstch::node cppTemplate() { return cached_props_.cpp_template(); }

  mstch::node cythonTemplate() { return to_cython_template(); }

  mstch::node isDefaultTemplate() {
    return cached_props_.is_default_template(resolved_type_);
  }

  mstch::node customCppType() { return cached_props_.cpp_type(); }

  mstch::node cythonType() { return to_cython_type(); }

  mstch::node isCustomCppType() { return is_custom_cpp_type(); }

  mstch::node isNumber() { return is_number(); }

  mstch::node isInteger() { return is_integer(); }

  mstch::node isContainerOfString() {
    return is_list_of_string() || is_set_of_string();
  }

  mstch::node cythonTypeNoneable() {
    return !(is_number() || resolved_type_->is<t_container>());
  }

  mstch::node hasCythonType() { return has_cython_type(); }

  mstch::node isIOBuf() { return is_iobuf(); }

  mstch::node isIOBufRef() { return is_iobuf_ref(); }

  mstch::node isFlexibleBinary() { return is_flexible_binary(); }

  mstch::node isCustomBinaryType() { return is_custom_binary_type(); }

  // types that don't have an underlying C++ type
  // i.e., structs, unions, exceptions all enclose a C++ type
  mstch::node isSimple() {
    return (resolved_type_->is<t_primitive_type>() ||
            resolved_type_->is<t_enum>() ||
            resolved_type_->is<t_container>()) &&
        !is_custom_binary_type();
  }

  // types that need conversion to py3 if accessed from thrift-python struct
  // fields
  mstch::node needs_convert() { return type_needs_convert(resolved_type_); }

  mstch::node is_container_of_struct() {
    return resolved_type_->is<t_container>() &&
        container_needs_convert(resolved_type_);
  }

  // type:list_elem_type etc. is defined in mstch_objects, so the returned
  // type node doesn't define type:needs_convert
  mstch::node element_needs_convert() {
    if (const t_list* list = resolved_type_->try_as<t_list>()) {
      return type_needs_convert(list->elem_type().get_type());
    } else if (const t_set* set = resolved_type_->try_as<t_set>()) {
      return type_needs_convert(set->elem_type().get_type());
    }
    return false;
  }

  mstch::node map_key_needs_convert() {
    if (const t_map* map = resolved_type_->try_as<t_map>()) {
      return type_needs_convert(map->key_type().get_type());
    }
    return false;
  }

  mstch::node map_value_needs_convert() {
    if (const t_map* map = resolved_type_->try_as<t_map>()) {
      return type_needs_convert(map->val_type().get_type());
    }
    return false;
  }

  bool is_custom_cpp_type() const { return cached_props_.cpp_type() != ""; }

 protected:
  const t_program* get_type_program() const {
    if (const t_program* p = resolved_type_->program()) {
      return p;
    }
    return prog_;
  }

  std::vector<std::string> get_type_cpp2_namespace() const {
    return cpp2::get_gen_namespace_components(*get_type_program());
  }

  std::string to_cython_template() const {
    return cached_props_.to_cython_template();
  }

  std::string to_cython_type() const { return cached_props_.to_cython_type(); }

  bool is_integer() const {
    return resolved_type_->is_any_int() || resolved_type_->is_byte();
  }

  bool is_number() const {
    return is_integer() || resolved_type_->is_floating_point();
  }

  bool is_list_of_string() {
    if (const t_list* list = resolved_type_->try_as<t_list>()) {
      return list->elem_type()->is_string_or_binary();
    }
    return false;
  }

  bool is_set_of_string() {
    if (const t_set* set = resolved_type_->try_as<t_set>()) {
      return set->elem_type()->is_string_or_binary();
    }
    return false;
  }

  bool has_cython_type() const {
    return has_option("inplace_migrate")
        ? !(resolved_type_->is<t_container>() ||
            resolved_type_->is<t_struct>() || resolved_type_->is<t_union>())
        : !resolved_type_->is<t_container>();
  }

  bool is_iobuf() const { return cached_props_.cpp_type() == "folly::IOBuf"; }

  bool is_iobuf_ref() const {
    return cached_props_.cpp_type() == "std::unique_ptr<folly::IOBuf>";
  }

  bool is_flexible_binary() const {
    return resolved_type_->is_binary() && is_custom_cpp_type() && !is_iobuf() &&
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
  const FileType* file_type_;
  py3_generator_context::cached_type_properties& cached_props_;
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
    return cpp2::OrderableTypeUtils::is_orderable(*struct_) &&
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
      if (field->default_value()) {
        return true;
      }
    }
    return false;
  }

 private:
  std::vector<const t_field*> py3_fields_;
  bool hidden_fields = false;
};

std::string py3_mstch_program::visit_type_impl(
    const t_type* orig_type, bool fromTypeDef) {
  bool hasPy3EnableCppAdapterAnnot =
      orig_type->has_structured_annotation(kPythonPy3EnableCppAdapterUri);
  auto trueType = orig_type->get_true_type();
  py3_generator_context::cached_type_properties& props =
      generator_context_.get_cached_type_props(orig_type);
  const std::string& flatName = props.flat_name();
  // Import all types either beneath a typedef, even if the current type is
  // not directly a typedef
  fromTypeDef = fromTypeDef || orig_type->is<t_typedef>();
  if (flatName.empty()) {
    std::string extra;
    if (const t_list* list = trueType->try_as<t_list>()) {
      extra =
          "List__" + visit_type_impl(list->elem_type().get_type(), fromTypeDef);
    } else if (const t_set* set = trueType->try_as<t_set>()) {
      extra =
          "Set__" + visit_type_impl(set->elem_type().get_type(), fromTypeDef);
    } else if (const t_map* map = trueType->try_as<t_map>()) {
      extra = "Map__" +
          visit_type_impl(map->key_type().get_type(), fromTypeDef) + "_" +
          visit_type_impl(map->val_type().get_type(), fromTypeDef);
    } else if (trueType->is_binary()) {
      extra = "binary";
    } else {
      extra = trueType->name();
    }
    props.set_flat_name(generator_context_.program(), trueType, extra);
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
    if (!props.is_default_template(trueType)) {
      customTemplates_.push_back(trueType);
    }
    if (!props.cpp_type().empty()) {
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
      visit_type(field.type().get_type());
    }
    const t_stream* stream = function.stream();
    if (const t_throws* exceptions = stream ? stream->exceptions() : nullptr) {
      for (const t_field& field : exceptions->fields()) {
        const t_type* exType = field.type().get_type();
        streamExceptions_.emplace(visit_type(field.type().get_type()), exType);
      }
    }
    for (const t_field& field : get_elems(function.exceptions())) {
      visit_type(field.type().get_type());
    }

    std::string return_type_name;
    if (stream && !function.is_interaction_constructor()) {
      return_type_name = "Stream__";
      const t_type* elem_type = stream->elem_type().get_type();
      if (!function.has_void_initial_response()) {
        return_type_name = "ResponseAndStream__" +
            visit_type(function.return_type().get_type()) + "_";
      }
      std::string elem_type_name = visit_type(elem_type);
      return_type_name += elem_type_name;
      streamTypes_.emplace(elem_type_name, elem_type);
      bool inserted = seenTypeNames_.insert(return_type_name).second;
      if (inserted && !function.has_void_initial_response()) {
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
    opts.allowed_undefined_variables = {"function:stream?"};
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

 private:
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
  FileType file_type_ = FileType::NotTypesFile;
  py3_generator_context context_{program_, &file_type_};

  void initialize_context(t_whisker_generator::context_visitor& visitor) final {
    context_.register_visitors(visitor);
  }

  whisker::map::raw globals(prototype_database& proto) const override {
    whisker::map::raw globals = t_mstch_generator::globals(proto);
    globals["py_string_literal"] = whisker::dsl::make_function(
        "py_string_literal",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(
              python::to_python_string_literal(
                  ctx.argument<whisker::string>(0)));
        });
    return globals;
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    def.property("py3_enum_value_name", [](const t_const_value& self) {
      if (!self.is_enum() || self.get_enum_value() == nullptr) {
        return whisker::make::null;
      }
      const auto& enum_name = self.get_enum()->name();
      return whisker::make::string(
          python::get_py3_name_class_scope(*self.get_enum_value(), enum_name));
    });
    /*
     * Use this function (instead of the version used by C++) to render unicode
     * strings, i.e., normal python strings "".
     * For binary bytes b"", use string_value, which has octal escapes for
     * unicode characters.
     */
    def.property("unicode_value", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_STRING
          ? get_escaped_string<nonascii_handling::no_escape>(self.get_string())
          : "";
    });

    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def =
        whisker::dsl::prototype_builder<h_field>::extends(std::move(base));

    def.property("reference?", [this](const t_field& self) {
      return context_.get_field_cpp_kind(self) != field_cpp_kind::value;
    });
    def.property("unique_ref?", [this](const t_field& self) {
      return context_.get_field_cpp_kind(self) == field_cpp_kind::unique_ptr;
    });
    def.property("shared_ref?", [this](const t_field& self) {
      return context_.get_field_cpp_kind(self) ==
          field_cpp_kind::shared_ptr_mutable;
    });
    def.property("shared_const_ref?", [this](const t_field& self) {
      return context_.get_field_cpp_kind(self) ==
          field_cpp_kind::shared_ptr_const;
    });
    def.property("iobuf_ref?", [this](const t_field& self) {
      return context_.get_field_cpp_kind(self) == field_cpp_kind::iobuf;
    });
    def.property("has_ref_accessor?", [this](const t_field& self) {
      const field_cpp_kind cpp_kind = context_.get_field_cpp_kind(self);
      return cpp_kind == field_cpp_kind::value ||
          cpp_kind == field_cpp_kind::iobuf;
    });
    def.property("hasDefaultValue?", [this](const t_field& self) {
      return field_has_default_value(self, context_.get_field_cpp_kind(self));
    });
    def.property("optional_default?", [](const t_field& self) {
      return self.qualifier() == t_field_qualifier::optional &&
          self.default_value() != nullptr;
    });
    def.property("PEP484Optional?", [this](const t_field& self) {
      return !field_has_default_value(self, context_.get_field_cpp_kind(self));
    });
    def.property("isset?", [this](const t_field& self) {
      const field_cpp_kind cpp_kind = context_.get_field_cpp_kind(self);
      return (cpp_kind == field_cpp_kind::value ||
              cpp_kind == field_cpp_kind::iobuf) &&
          self.qualifier() != t_field_qualifier::required;
    });
    def.property("boxed_ref?", [](const t_field& self) {
      return gen::cpp::find_ref_type(self) == gen::cpp::reference_type::boxed;
    });

    return std::move(def).make();
  }

  prototype<t_function>::ptr make_prototype_for_function(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_function(proto);
    auto def =
        whisker::dsl::prototype_builder<h_function>::extends(std::move(base));

    def.property("eb?", [this](const t_function& self) {
      if (self.get_unstructured_annotation("thread") == "eb" ||
          self.has_structured_annotation(kCppProcessInEbThreadUri)) {
        return true;
      }
      const t_interface* parent = context().get_function_parent(&self);
      assert(parent != nullptr);
      return parent->has_unstructured_annotation("process_in_event_base") ||
          parent->has_structured_annotation(kCppProcessInEbThreadUri);
    });
    def.property("stack_arguments?", [this](const t_function& self) {
      return cpp2::is_stack_arguments(compiler_options(), self);
    });

    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);
    def.property("py_name", &python::get_py3_name);
    def.property("hasPyName?", [](const t_named& self) {
      return python::get_py3_name(self) != self.name();
    });
    def.property(
        "cpp_name", [](const t_named& self) { return cpp2::get_name(&self); });
    def.property("modulePath", [this](const t_named& self) {
      const t_program* program =
          self.program() == nullptr ? get_program() : self.program();
      return fmt::format(
          "_{}", fmt::join(get_type_py3_namespace(program, "types"), "_"));
    });
    return std::move(def).make();
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);

    // this option triggers generation of py3 structs as wrappers around
    // thrift-python structs
    def.property("inplace_migrate?", [this](const t_program&) {
      return has_compiler_option("inplace_migrate");
    });

    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    // Overrides for `t_named` properties, resolving typedefs before computing
    // the value. This is necessary because the py3 generator previously used to
    // erase typedefs when initializing mstch_type, but still used the
    // unresolved type in the constructor to resolve context properties.
    // To resolve context properties in Whisker, we need to use the unresolved
    // type as the key, but that requires us to resolve typedefs for all other
    // properties to maintain compatibility.
    def.property("py_name", [](const t_type& self) {
      return python::get_py3_name(*self.get_true_type());
    });
    def.property("hasPyName?", [](const t_type& self) {
      const t_type* true_type = self.get_true_type();
      return python::get_py3_name(*true_type) != true_type->name();
    });
    def.property("cpp_name", [](const t_type& self) {
      return cpp2::get_name(self.get_true_type());
    });
    def.property("modulePath", [this](const t_type& self) {
      const t_type* true_type = self.get_true_type();
      const t_program* program = true_type->program() == nullptr
          ? get_program()
          : true_type->program();
      return fmt::format(
          "_{}", fmt::join(get_type_py3_namespace(program, "types"), "_"));
    });

    def.property("module_path_period_separated", [this](const t_type& self) {
      const t_type* true_type = self.get_true_type();
      const t_program* program = true_type->program() == nullptr
          ? get_program()
          : true_type->program();
      return fmt::format(
          "{}", fmt::join(get_type_py3_namespace(program, "types"), "."));
    });

    def.property("need_module_path?", [this](const t_type& self) {
      if (file_type_ == FileType::NotTypesFile) {
        return true;
      }
      const t_type* true_type = self.get_true_type();
      return (
          true_type->program() != nullptr &&
          true_type->program() != get_program());
    });

    def.property("iobufWrapper?", [this](const t_type& self) {
      const py3_generator_context::cached_type_properties& cached_props =
          context_.get_cached_type_props(&self);
      return cached_props.cpp_type() == "folly::IOBuf" ||
          cached_props.cpp_type() == "std::unique_ptr<folly::IOBuf>";
    });

    def.property("flat_name", [this](const t_type& self) {
      return context_.get_cached_type_props(&self).flat_name();
    });

    return std::move(def).make();
  }
};

py3_generator_context::cached_type_properties&
py3_generator_context::get_cached_type_props(const t_type* type) const {
  // @python.Py3EnableCppAdapter treats C++ Adapter on typedef as a custom
  // cpp.type.
  auto true_type = type->get_true_type();
  if (type->has_structured_annotation(kPythonPy3EnableCppAdapterUri)) {
    return type_properties_
        .emplace(
            type,
            cached_type_properties{
                get_cpp_template(*true_type),
                name_resolver_.get_native_type(*type),
                {}})
        .first->second;
  }
  auto it = type_properties_.find(true_type);
  if (it == type_properties_.end()) {
    it = type_properties_
             .emplace(
                 true_type,
                 cached_type_properties{
                     get_cpp_template(*true_type),
                     fmt::to_string(cpp2::get_type(true_type)),
                     {}})
             .first;
  }
  return it->second;
}

void t_mstch_py3_generator::set_mstch_factories() {
  mstch_context_.add<py3_mstch_program>(&context_);
  mstch_context_.add<py3_mstch_service>(program_);
  mstch_context_.add<py3_mstch_type>(&context_);
  mstch_context_.add<py3_mstch_struct>();
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
  file_type_ = file_type;

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
