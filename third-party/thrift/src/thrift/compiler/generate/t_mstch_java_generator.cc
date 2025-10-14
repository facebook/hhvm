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

#include <openssl/evp.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/java/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/whisker/mstch_compat.h>

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
  // Save an initalized context.
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

std::string str_type_list;
std::string type_list_hash;

struct type_mapping {
  std::string uri;
  std::string className;
};

std::vector<type_mapping> type_list;

set<std::string> adapterDefinitionSet;
std::string prevTypeName;

bool is_annotation_map_field_equal(
    const t_const* annotation, const char* field, const int value) {
  for (const auto& item : annotation->value()->get_map()) {
    if (item.first->get_string() == field) {
      return item.second->get_integer() == value;
    }
  }
  return annotation->value()->get_map().empty();
}

} // namespace

class t_mstch_java_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "type:typedef_type", // in UnionWrite.mustache
        "struct:isUnion?", // in WriteResponseType.mustache
        "struct:asBean?", // in DefaultValue.mustache
        "field:hasAdapter?", // in BoxedType.mustache
    };
    return opts;
  }

  std::string template_prefix() const override { return "java"; }

  void generate_program() override;

 private:
  void set_mstch_factories();

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

  /*
   * Generate multiple Java items according to the given template. Writes
   * output to package_dir underneath the global output directory.
   */
  void generate_rpc_interfaces() {
    const t_program* program = get_program();
    const auto& id = program->path();
    if (!mstch_context_.program_cache.count(id)) {
      mstch_context_.program_cache[id] =
          mstch_context_.program_factory->make_mstch_object(
              program, mstch_context_);
    }
    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = has_option("separate_data_type_from_services")
        ? "services" / raw_package_dir
        : raw_package_dir;

    for (const t_service* service : program->services()) {
      auto& cache = mstch_context_.service_cache;
      auto filename = java::mangle_java_name(service->name(), true) + ".java";
      const auto& item_id = id + service->name();
      if (!cache.count(item_id)) {
        cache[item_id] = mstch_context_.service_factory->make_mstch_object(
            service, mstch_context_);
      }

      render_to_file(cache[item_id], "Service", package_dir / filename);
    }
  }

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
    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = has_option("separate_data_type_from_services")
        ? "data-type" / raw_package_dir
        : raw_package_dir;

    std::string package_name = get_namespace_or_default(*program_);

    for (const T* item : items) {
      auto classname = java::mangle_java_name(item->name(), true);
      auto filename = classname + ".java";
      const auto& item_id = id + item->name();
      if (!c.count(item_id)) {
        c[item_id] = factory.make_mstch_object(item, mstch_context_);
      }

      render_to_file(c[item_id], tpl_path, package_dir / filename);

      auto uri = item->uri();
      if (!uri.empty()) {
        type_list.push_back(type_mapping{uri, package_name + "." + classname});
        str_type_list += uri;
      }
    }
  }

  /*
   * Generate Service Client implementation - Sync & Async. Writes
   * output to package_dir
   */
  void generate_services() {
    const auto& service_factory = *mstch_context_.service_factory;
    auto& cache = mstch_context_.service_cache;
    const t_program* program = get_program();
    const auto& id = program->path();
    if (!mstch_context_.program_cache.count(id)) {
      mstch_context_.program_cache[id] =
          mstch_context_.program_factory->make_mstch_object(
              program, mstch_context_);
    }

    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};

    auto package_dir = has_option("separate_data_type_from_services")
        ? "services" / raw_package_dir
        : raw_package_dir;

    // Iterate through services
    for (const t_service* service : program->services()) {
      auto service_name = java::mangle_java_name(service->name(), true);
      // NOTE: generation of ClientImpl and AsyncClientImpl is deprecated and
      // only customers who are still using netty3 for some reason should use it
      // for backward compatibility reasons.
      if (has_option("deprecated_allow_leagcy_reflection_client")) {
        // Generate deprecated sync client
        auto sync_filename = service_name + "ClientImpl.java";
        const auto& sync_service_id = id + service->name() + "Client";
        if (!cache.count(sync_service_id)) {
          cache[sync_service_id] =
              service_factory.make_mstch_object(service, mstch_context_);
        }

        render_to_file(
            cache[sync_service_id],
            "deprecated/ServiceClient",
            package_dir / sync_filename);

        // Generate deprecated async client
        auto async_filename = service_name + "AsyncClientImpl.java";
        const auto& async_service_id = id + service->name() + "AsyncClient";
        if (!cache.count(async_service_id)) {
          cache[async_service_id] =
              service_factory.make_mstch_object(service, mstch_context_);
        }

        render_to_file(
            cache[async_service_id],
            "deprecated/ServiceAsyncClient",
            package_dir / async_filename);
      }

      // Generate Async to Reactive Wrapper
      auto async_reactive_wrapper_filename =
          service_name + "AsyncReactiveWrapper.java";
      const auto& async_reactive_wrapper_id =
          id + service->name() + "AsyncReactiveWrapper";
      if (!cache.count(async_reactive_wrapper_id)) {
        cache[async_reactive_wrapper_id] =
            service_factory.make_mstch_object(service, mstch_context_);
      }

      render_to_file(
          cache[async_reactive_wrapper_id],
          "AsyncReactiveWrapper",
          package_dir / async_reactive_wrapper_filename);

      // Generate Blocking to Reactive Wrapper
      auto blocking_reactive_wrapper_filename =
          service_name + "BlockingReactiveWrapper.java";
      const auto& blocking_reactive_wrapper_id =
          id + service->name() + "BlockingReactiveWrapper";
      if (!cache.count(blocking_reactive_wrapper_id)) {
        cache[blocking_reactive_wrapper_id] =
            service_factory.make_mstch_object(service, mstch_context_);
      }

      render_to_file(
          cache[blocking_reactive_wrapper_id],
          "BlockingReactiveWrapper",
          package_dir / blocking_reactive_wrapper_filename);

      // Generate Reactive to Async Wrapper
      auto reactive_async_wrapper_filename =
          service_name + "ReactiveAsyncWrapper.java";
      const auto& reactive_async_wrapper_id =
          id + service->name() + "ReactiveAsyncWrapper";
      if (!cache.count(reactive_async_wrapper_id)) {
        cache[reactive_async_wrapper_id] =
            service_factory.make_mstch_object(service, mstch_context_);
      }

      render_to_file(
          cache[reactive_async_wrapper_id],
          "ReactiveAsyncWrapper",
          package_dir / reactive_async_wrapper_filename);

      // Generate Reactive to Blocking Wrapper
      auto reactive_blocking_wrapper_filename =
          service_name + "ReactiveBlockingWrapper.java";
      const auto& reactive_blocking_wrapper_id =
          id + service->name() + "ReactiveBlockingWrapper";
      if (!cache.count(reactive_blocking_wrapper_id)) {
        cache[reactive_blocking_wrapper_id] =
            service_factory.make_mstch_object(service, mstch_context_);
      }

      render_to_file(
          cache[reactive_blocking_wrapper_id],
          "ReactiveBlockingWrapper",
          package_dir / reactive_blocking_wrapper_filename);

      // Generate Reactive Client
      auto reactive_client_filename = service_name + "ReactiveClient.java";
      const auto& reactive_client_wrapper_id =
          id + service->name() + "ReactiveClient";
      if (!cache.count(reactive_client_wrapper_id)) {
        cache[reactive_client_wrapper_id] =
            service_factory.make_mstch_object(service, mstch_context_);
      }

      render_to_file(
          cache[reactive_client_wrapper_id],
          "ReactiveClient",
          package_dir / reactive_client_filename);

      // Generate RpcServerHandler
      auto rpc_server_handler_filename = service_name + "RpcServerHandler.java";
      const auto& rpc_server_handler_id =
          id + service->name() + "RpcServerHandler";
      if (!cache.count(rpc_server_handler_id)) {
        cache[rpc_server_handler_id] =
            service_factory.make_mstch_object(service, mstch_context_);
      }

      render_to_file(
          cache[rpc_server_handler_id],
          "RpcServerHandler",
          package_dir / rpc_server_handler_filename);
    }
  }

  void generate_constants(const t_program* program) {
    if (program->consts().empty()) {
      // Only generate Constants.java if we actually have constants
      return;
    }
    const auto& prog = cached_program(program);

    auto raw_package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto package_dir = has_option("separate_data_type_from_services")
        ? "data-type" / raw_package_dir
        : raw_package_dir;

    auto constant_file_name = get_constants_class_name(*program) + ".java";
    render_to_file(prog, "Constants", package_dir / constant_file_name);
  }

  void generate_placeholder(const t_program* program) {
    auto package_dir = std::filesystem::path{
        java::package_to_path(get_namespace_or_default(*program))};
    auto placeholder_file_name = ".generated_" + program->name();
    if (has_option("separate_data_type_from_services")) {
      write_output("data-type" / package_dir / placeholder_file_name, "");
      write_output("services" / package_dir / placeholder_file_name, "");
    } else {
      write_output(package_dir / placeholder_file_name, "");
    }
  }

  void generate_type_list(const t_program* program) {
    // NOTE: ideally, we do not really need to generated type list if
    // `type_list.size() == 0`, but due to build system limitations.
    // all java_library need at least one .java file.

    auto java_namespace = get_namespace_or_default(*program);
    auto raw_package_dir =
        std::filesystem::path{java::package_to_path(java_namespace)};
    auto package_dir = has_option("separate_data_type_from_services")
        ? "data-type" / raw_package_dir
        : raw_package_dir;

    const auto& program_name = program->name();
    type_list_hash = toHex(hash(program_name + java_namespace + str_type_list));

    std::string file_name = "__fbthrift_TypeList_" + type_list_hash + ".java";

    const auto& prog = cached_program(program);
    render_to_file(prog, "TypeList", package_dir / file_name);
  }
};

class mstch_java_program : public mstch_program {
  static constexpr int32_t BATCH_SIZE = 512;

 public:
  mstch_java_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:javaPackage", &mstch_java_program::java_package},
            {"program:constantClassName",
             &mstch_java_program::constant_class_name},
            {"program:typeList", &mstch_java_program::list},
            {"program:typeListHash", &mstch_java_program::list_hash},
        });
  }
  mstch::node java_package() { return get_namespace_or_default(*program_); }
  mstch::node constant_class_name() {
    return get_constants_class_name(*program_);
  }
  mstch::node list_hash() { return type_list_hash; }
  mstch::node list() {
    int32_t size = type_list.size();
    mstch::array arr;
    int n = 0;
    while (n * BATCH_SIZE < size) {
      mstch::array a;
      for (int32_t i = n * BATCH_SIZE; i < std::min((n + 1) * BATCH_SIZE, size);
           i++) {
        const auto& m = type_list[i];
        a.emplace_back(mstch::map{
            {"typeList:uri", m.uri},
            {"typeList:className", m.className},
        });
      }
      arr.emplace_back(mstch::map{
          {"typeList:index", n++},
          {"typeList:batch", a},
      });
    }
    return arr;
  }
};

class mstch_java_struct : public mstch_struct {
  // A struct is a "big struct" if it contains > 127 members. The reason for
  // this limit is that we generate exhaustive constructor for Thrift struct
  // but Java methods are limited to 255 arguments (and since long/double
  // types count twice, 127 is a safe threshold).
  static constexpr uint64_t bigStructThreshold = 127;

 public:
  mstch_java_struct(
      const t_structured* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos) {
    register_methods(
        this,
        {
            {"struct:javaPackage", &mstch_java_struct::java_package},
            {"struct:unionFieldTypeUnique?",
             &mstch_java_struct::is_union_field_type_unique},
            {"struct:asBean?", &mstch_java_struct::is_as_bean},
            {"struct:isBigStruct?", &mstch_java_struct::is_BigStruct},
            {"struct:javaCapitalName", &mstch_java_struct::java_capital_name},
            {"struct:javaAnnotations?",
             &mstch_java_struct::has_java_annotations},
            {"struct:isUnion?", &mstch_java_struct::is_struct_union},
            {"struct:javaAnnotations", &mstch_java_struct::java_annotations},
            {"struct:exceptionMessage", &mstch_java_struct::exception_message},
            {"struct:needsExceptionMessage?",
             &mstch_java_struct::needs_exception_message},
            {"struct:hasTerseField?", &mstch_java_struct::has_terse_field},
            {"struct:hasWrapper?", &mstch_java_struct::has_wrapper},
            {"struct:clearAdapter", &mstch_java_struct::clear_adapter},
        });
  }
  mstch::node java_package() {
    return get_namespace_or_default(*struct_->program());
  }
  mstch::node is_struct_union() { return struct_->is<t_union>(); }
  mstch::node is_union_field_type_unique() {
    std::set<std::string> field_types;
    for (const auto& field : struct_->fields()) {
      auto type_name = field.type()->get_true_type()->get_full_name();
      std::string type_with_erasure = type_name.substr(0, type_name.find('<'));
      if (field_types.find(type_with_erasure) != field_types.end()) {
        return false;
      } else {
        field_types.insert(type_with_erasure);
      }
    }
    return true;
  }
  mstch::node has_terse_field() {
    for (const auto& field : struct_->fields()) {
      if (field.qualifier() == t_field_qualifier::terse) {
        return true;
      }
    }
    return false;
  }
  mstch::node clear_adapter() {
    adapterDefinitionSet.clear();
    return mstch::node();
  }
  mstch::node has_wrapper() {
    for (const auto& field : struct_->fields()) {
      if (field.has_structured_annotation(kJavaWrapperUri)) {
        return true;
      }
    }
    return false;
  }
  mstch::node is_as_bean() {
    if (!struct_->is<t_exception>() && !struct_->is<t_union>()) {
      return struct_->get_unstructured_annotation("java.swift.mutable") ==
          "true" ||
          struct_->has_structured_annotation(kJavaMutableUri);
    } else {
      return false;
    }
  }

  mstch::node is_BigStruct() {
    return (
        (struct_->is<t_struct>() || struct_->is<t_union>()) &&
        struct_->fields().size() > bigStructThreshold);
  }

  mstch::node java_capital_name() {
    return java::mangle_java_name(struct_->name(), true);
  }
  mstch::node has_java_annotations() {
    return struct_->has_unstructured_annotation("java.swift.annotations") ||
        struct_->has_structured_annotation(kJavaAnnotationUri);
  }
  mstch::node java_annotations() {
    if (struct_->has_unstructured_annotation("java.swift.annotations")) {
      return struct_->get_unstructured_annotation("java.swift.annotations");
    }

    return get_structed_annotation_attribute(
        struct_, kJavaAnnotationUri, "java_annotation");
  }
  mstch::node exception_message() {
    const auto* message_field =
        dynamic_cast<const t_exception&>(*struct_).get_message_field();
    return get_java_swift_name(message_field);
  }
  // we can only override Throwable's getMessage() if:
  // 1 - there is either provided `'message'` annotation or
  //  `@thrift.ErrorMessage` annotation.
  // 2 - there is no struct field named 'message' (since it
  //  will generate `getMessage()` method)
  mstch::node needs_exception_message() {
    return struct_->is<t_exception>() &&
        dynamic_cast<const t_exception&>(*struct_).get_message_field() !=
        nullptr &&
        struct_->get_field_by_name("message") == nullptr;
  }
};

class mstch_java_service : public mstch_service {
 public:
  mstch_java_service(
      const t_service* s,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service = nullptr)
      : mstch_service(s, ctx, pos, containing_service) {
    register_methods(
        this,
        {
            {"service:javaPackage", &mstch_java_service::java_package},
            {"service:javaCapitalName", &mstch_java_service::java_capital_name},
            {"service:onewayFunctions",
             &mstch_java_service::get_oneway_functions},
            {"service:requestResponseFunctions",
             &mstch_java_service::get_request_response_functions},
            {"service:singleRequestFunctions",
             &mstch_java_service::get_single_request_functions},
            {"service:streamingFunctions",
             &mstch_java_service::get_streaming_functions},
            {"service:sinkFunctions", &mstch_java_service::get_sink_functions},
        });
  }
  mstch::node java_package() {
    return get_namespace_or_default(*(service_->program()));
  }
  mstch::node java_capital_name() {
    return java::mangle_java_name(service_->name(), true);
  }
  mstch::node get_oneway_functions() {
    std::vector<t_function*> funcs;
    for (auto func : service_->get_functions()) {
      if (func->qualifier() == t_function_qualifier::oneway) {
        funcs.push_back(func);
      }
    }
    return make_mstch_functions(funcs);
  }
  mstch::node get_request_response_functions() {
    std::vector<t_function*> funcs;
    for (auto func : service_->get_functions()) {
      if (!func->sink_or_stream() && !func->is_interaction_constructor() &&
          func->qualifier() != t_function_qualifier::oneway) {
        funcs.push_back(func);
      }
    }
    return make_mstch_functions(funcs);
  }
  mstch::node get_single_request_functions() {
    std::vector<t_function*> funcs;
    for (auto func : service_->get_functions()) {
      if (!func->sink_or_stream() && !func->is_interaction_constructor()) {
        funcs.push_back(func);
      }
    }
    return make_mstch_functions(funcs);
  }

  mstch::node get_streaming_functions() {
    std::vector<t_function*> funcs;
    for (auto func : service_->get_functions()) {
      if (func->stream()) {
        funcs.push_back(func);
      }
    }
    return make_mstch_functions(funcs);
  }

  mstch::node get_sink_functions() {
    std::vector<t_function*> funcs;
    for (auto func : service_->get_functions()) {
      if (func->sink()) {
        funcs.push_back(func);
      }
    }
    return make_mstch_functions(funcs);
  }
};

class mstch_java_interaction : public mstch_java_service {
 public:
  using ast_type = t_interaction;

  mstch_java_interaction(
      const t_interaction* interaction,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service)
      : mstch_java_service(interaction, ctx, pos, containing_service) {
    register_methods(
        this,
        {{"interaction:javaParentCapitalName",
          &mstch_java_interaction::java_parent_capital_name}});
  }

  mstch::node java_parent_capital_name() {
    return java::mangle_java_name(containing_service_->name(), true);
  }
};

class mstch_java_function : public mstch_function {
 public:
  mstch_java_function(
      const t_function* f, mstch_context& ctx, mstch_element_position pos)
      : mstch_function(f, ctx, pos) {
    register_methods(
        this,
        {
            {"function:javaName", &mstch_java_function::java_name},
            {"function:voidType", &mstch_java_function::is_void_type},
            {"function:nestedDepth",
             {with_no_caching, &mstch_java_function::get_nested_depth}},
            {"function:nestedDepth++",
             {with_no_caching, &mstch_java_function::increment_nested_depth}},
            {"function:nestedDepth--",
             {with_no_caching, &mstch_java_function::decrement_nested_depth}},
            {"function:isFirstDepth?",
             {with_no_caching, &mstch_java_function::is_first_depth}},
            {"function:prevNestedDepth",
             {with_no_caching, &mstch_java_function::preceding_nested_depth}},
            {"function:isNested?",
             {with_no_caching,
              &mstch_java_function::get_nested_container_flag}},
            {"function:setIsNested",
             {with_no_caching,
              &mstch_java_function::set_nested_container_flag}},
            {"function:unsetIsNested",
             {with_no_caching,
              &mstch_java_function::unset_nested_container_flag}},
        });
  }

  int32_t nestedDepth = 0;
  bool isNestedContainerFlag = false;

  mstch::node get_nested_depth() { return nestedDepth; }
  mstch::node preceding_nested_depth() { return (nestedDepth - 1); }
  mstch::node is_first_depth() { return (nestedDepth == 1); }
  mstch::node get_nested_container_flag() { return isNestedContainerFlag; }
  mstch::node set_nested_container_flag() {
    isNestedContainerFlag = true;
    return mstch::node();
  }
  mstch::node unset_nested_container_flag() {
    isNestedContainerFlag = false;
    return mstch::node();
  }
  mstch::node increment_nested_depth() {
    nestedDepth++;
    return mstch::node();
  }
  mstch::node decrement_nested_depth() {
    nestedDepth--;
    return mstch::node();
  }

  mstch::node java_name() {
    return java::mangle_java_name(function_->name(), false);
  }

  mstch::node is_void_type() {
    return function_->return_type()->is_void() && !function_->stream();
  }
};

class mstch_java_field : public mstch_field {
 public:
  mstch_java_field(
      const t_field* f, mstch_context& ctx, mstch_element_position pos)
      : mstch_field(f, ctx, pos) {
    register_methods(
        this,
        {
            {"field:javaName", &mstch_java_field::java_name},
            {"field:javaCapitalName", &mstch_java_field::java_capital_name},
            {"field:javaConstantName", &mstch_java_field::java_constant_name},
            {"field:javaDefaultValue", &mstch_java_field::java_default_value},
            {"field:javaAllCapsName", &mstch_java_field::java_all_caps_name},
            {"field:recursive?", &mstch_java_field::is_recursive_reference},
            {"field:negativeId?", &mstch_java_field::is_negative_id},
            {"field:javaAnnotations?", &mstch_java_field::has_java_annotations},
            {"field:javaAnnotations", &mstch_java_field::java_annotations},
            {"field:javaTFieldName", &mstch_java_field::java_tfield_name},
            {"field:isNullableOrOptionalNotEnum?",
             &mstch_java_field::is_nullable_or_optional_not_enum},
            {"field:isEnum?", &mstch_java_field::is_enum},
            {"field:isObject?", &mstch_java_field::is_object},
            {"field:isUnion?", &mstch_java_field::is_union},
            {"field:isContainer?", &mstch_java_field::is_container},
            {"field:typeFieldName", &mstch_java_field::type_field_name},
            {"field:isSensitive?", &mstch_java_field::is_sensitive},
            {"field:hasInitialValue?", &mstch_java_field::has_initial_value},
            {"field:isNumericOrVoid?", &mstch_java_field::is_numeric_or_void},
            {"field:hasWrapper?", &mstch_java_field::has_wrapper},
            {"field:wrapper",
             &mstch_java_field::get_structured_wrapper_class_name},
            {"field:wrapperType",
             &mstch_java_field::get_structured_wrapper_type_class_name},
            {"field:hasAdapterOrWrapper?",
             &mstch_java_field::has_adapter_or_wrapper},
            {"field:hasAdapter?", &mstch_java_field::has_adapter},
            {"field:hasTypeAdapter?", &mstch_java_field::has_type_adapter},
            {"field:typeAdapterTypeClassName",
             &mstch_java_field::get_typedef_adapter_type_class_name},
            {"field:typeAdapterClassName",
             &mstch_java_field::get_typedef_adapter_class_name},
            {"field:hasFieldAdapter?", &mstch_java_field::has_field_adapter},
            {"field:fieldAdapterTypeClassName",
             &mstch_java_field::get_field_adapter_type_class_name},
            {"field:fieldAdapterClassName",
             &mstch_java_field::get_field_adapter_class_name},
            {"field:FieldNameUnmangled?",
             &mstch_java_field::is_field_name_unmangled},

            {"field:nestedDepth",
             {with_no_caching, &mstch_java_field::get_nested_depth}},
            {"field:nestedDepth++",
             {with_no_caching, &mstch_java_field::increment_nested_depth}},
            {"field:nestedDepth--",
             {with_no_caching, &mstch_java_field::decrement_nested_depth}},
            {"field:isFirstDepth?",
             {with_no_caching, &mstch_java_field::is_first_depth}},
            {"field:prevNestedDepth",
             {with_no_caching, &mstch_java_field::preceding_nested_depth}},
            {"field:isNested?",
             {with_no_caching, &mstch_java_field::get_nested_container_flag}},
            {"field:setIsNested",
             {with_no_caching, &mstch_java_field::set_nested_container_flag}},

            {"field:java_strings_compat?",
             &mstch_java_field::is_strings_compat},
            {"field:java_coding_error_action_report?",
             &mstch_java_field::is_coding_error_action_report},
        });
  }

  int32_t nestedDepth = 0;
  bool isNestedContainerFlag = false;

  bool _has_wrapper() {
    return field_->has_structured_annotation(kJavaWrapperUri);
  }

  mstch::node has_wrapper() { return _has_wrapper(); }

  mstch::node get_structured_wrapper_class_name() {
    return get_structed_annotation_attribute(
        field_, kJavaWrapperUri, "wrapperClassName");
  }

  mstch::node get_structured_wrapper_type_class_name() {
    return get_structed_annotation_attribute(
        field_, kJavaWrapperUri, "typeClassName");
  }

  mstch::node has_adapter_or_wrapper() {
    return _has_field_adapter() || _has_type_adapter() || _has_wrapper();
  }

  mstch::node has_adapter() {
    return _has_field_adapter() || _has_type_adapter();
  }

  bool _has_type_adapter() {
    auto type = field_->get_type();
    if (type->is<t_typedef>()) {
      if (t_typedef::get_first_structured_annotation_or_null(
              type, kJavaAdapterUri)) {
        return true;
      }
    }
    return false;
  }

  mstch::node has_type_adapter() { return _has_type_adapter(); }

  mstch::node get_typedef_adapter_type_class_name() {
    return get_typedef_structed_annotation_attribute(
        kJavaAdapterUri, "typeClassName");
  }

  mstch::node get_typedef_adapter_class_name() {
    return get_typedef_structed_annotation_attribute(
        kJavaAdapterUri, "adapterClassName");
  }

  mstch::node get_typedef_structed_annotation_attribute(
      const char* uri, const std::string& field) {
    auto type = field_->get_type();
    if (type->is<t_typedef>()) {
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

  bool _has_field_adapter() {
    return field_->has_structured_annotation(kJavaAdapterUri);
  }

  mstch::node has_field_adapter() { return _has_field_adapter(); }

  mstch::node get_field_adapter_type_class_name() {
    return get_structed_annotation_attribute(
        field_, kJavaAdapterUri, "typeClassName");
  }

  mstch::node get_field_adapter_class_name() {
    return get_structed_annotation_attribute(
        field_, kJavaAdapterUri, "adapterClassName");
  }

  mstch::node has_initial_value() {
    if (field_->get_req() == t_field::e_req::optional) {
      // default values are ignored for optional fields
      return false;
    }
    return !!field_->get_value();
  }
  mstch::node get_nested_depth() { return nestedDepth; }
  mstch::node preceding_nested_depth() { return (nestedDepth - 1); }
  mstch::node is_first_depth() { return (nestedDepth == 1); }
  mstch::node get_nested_container_flag() { return isNestedContainerFlag; }
  mstch::node set_nested_container_flag() {
    isNestedContainerFlag = true;
    return mstch::node();
  }
  mstch::node increment_nested_depth() {
    nestedDepth++;
    return mstch::node();
  }
  mstch::node decrement_nested_depth() {
    nestedDepth--;
    return mstch::node();
  }
  mstch::node is_numeric_or_void() {
    auto type = field_->get_type()->get_true_type();
    return type->is_void() || type->is_bool() || type->is_byte() ||
        type->is_i16() || type->is_i32() || type->is_i64() ||
        type->is_double() || type->is_float();
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
        field_type->is_double() || field_type->is<t_enum>());
  }

  mstch::node is_enum() {
    const t_type* field_type = field_->get_type()->get_true_type();
    return field_type->is<t_enum>();
  }

  mstch::node is_object() {
    const t_type* field_type = field_->get_type()->get_true_type();
    return field_type->is<t_structured>();
  }

  mstch::node is_union() {
    const t_type* field_type = field_->get_type()->get_true_type();
    return field_type->is<t_union>();
  }

  mstch::node is_container() {
    return field_->get_type()->get_true_type()->is<t_container>();
  }
  mstch::node java_name() { return get_java_swift_name(field_); }

  mstch::node type_field_name() {
    auto type_name = field_->get_type()->get_true_type()->get_full_name();
    return java::mangle_java_name(type_name, true);
  }

  mstch::node java_tfield_name() {
    return constant_name(get_java_swift_name(field_)) + "_FIELD_DESC";
  }
  mstch::node java_capital_name() {
    return java::mangle_java_name(get_java_swift_name(field_), true);
  }
  mstch::node java_constant_name() {
    return constant_name(get_java_swift_name(field_));
  }
  mstch::node java_all_caps_name() {
    auto field_name = field_->name();
    boost::to_upper(field_name);
    return field_name;
  }
  mstch::node java_default_value() { return default_value_for_field(field_); }
  mstch::node is_recursive_reference() {
    return field_->get_unstructured_annotation("swift.recursive_reference") ==
        "true" ||
        field_->has_structured_annotation(kJavaRecursiveUri);
  }
  mstch::node is_negative_id() { return field_->get_key() < 0; }
  std::string default_value_for_field(const t_field* field) {
    if (field_->get_req() == t_field::e_req::optional) {
      return "null";
    }
    return default_value_for_type(field->get_type());
  }
  std::string default_value_for_type(const t_type* type) {
    if (const t_typedef* typedef_ = type->try_as<t_typedef>()) {
      auto typedef_type = typedef_->get_type();
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
      } else if (type->is<t_enum>()) {
        // we use fromInteger(0) as default value as it may be null or the enum
        // entry for 0.
        auto javaNamespace = get_namespace_or_default(*(type->program()));
        auto enumType = java::mangle_java_name(type->name(), true);
        return javaNamespace + "." + enumType + ".fromInteger(0)";
      }
      return "null";
    }
  }

  mstch::node is_sensitive() {
    return field_->has_unstructured_annotation("java.sensitive");
  }
  std::string constant_name(string name) {
    string constant_str;

    bool is_first = true;
    bool was_previous_char_upper = false;
    for (string::iterator iter = name.begin(); iter != name.end(); ++iter) {
      string::value_type character = (*iter);
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
  mstch::node has_java_annotations() {
    return field_->has_unstructured_annotation("java.swift.annotations") ||
        field_->has_structured_annotation(kJavaAnnotationUri);
  }
  mstch::node java_annotations() {
    if (field_->has_unstructured_annotation("java.swift.annotations")) {
      return field_->get_unstructured_annotation("java.swift.annotations");
    }

    return get_structed_annotation_attribute(
        field_, kJavaAnnotationUri, "java_annotation");
  }

  mstch::node is_field_name_unmangled() {
    return field_->has_structured_annotation(kJavaFieldUseUnmangledNameUri);
  }

  mstch::node is_strings_compat() {
    if (field_->has_structured_annotation(kStringsUri)) {
      return true;
    }
    auto type = field_->get_type();
    if (type->is<t_typedef>() &&
        t_typedef::get_first_structured_annotation_or_null(type, kStringsUri) !=
            nullptr) {
      return true;
    }
    const t_structured* parent = whisker_context().get_field_parent(field_);
    return parent != nullptr &&
        (parent->has_structured_annotation(kStringsUri) ||
         parent->program()->has_structured_annotation(kStringsUri));
  }
  mstch::node is_coding_error_action_report() {
    constexpr auto kOnInvalidUtf8 = "onInvalidUtf8";
    constexpr int kActionReport = 1;

    auto type = field_->get_type();
    if (type->is<t_typedef>()) {
      if (const t_const* annotation =
              t_typedef::get_first_structured_annotation_or_null(
                  type, kStringsUri)) {
        return is_annotation_map_field_equal(
            annotation, kOnInvalidUtf8, kActionReport);
      }
    }
    if (const t_const* annotation =
            field_->find_structured_annotation_or_null(kStringsUri)) {
      return is_annotation_map_field_equal(
          annotation, kOnInvalidUtf8, kActionReport);
    }
    const t_structured* parent = whisker_context().get_field_parent(field_);
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
  }
};

class mstch_java_enum : public mstch_enum {
 public:
  mstch_java_enum(
      const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum(e, ctx, pos) {
    register_methods(
        this,
        {
            {"enum:javaPackage", &mstch_java_enum::java_package},
            {"enum:javaCapitalName", &mstch_java_enum::java_capital_name},
            {"enum:skipEnumNameMap?",
             &mstch_java_enum::java_skip_enum_name_map},
            {"enum:numValues", &mstch_java_enum::num_values},
            {"enum:useIntrinsicDefault?",
             &mstch_java_enum::use_intrinsic_default},
            {"enum:findValueZero", &mstch_java_enum::find_value_zero},
        });
  }
  mstch::node java_package() {
    return get_namespace_or_default(*enum_->program());
  }
  mstch::node java_capital_name() {
    return java::mangle_java_name(enum_->name(), true);
  }
  mstch::node java_skip_enum_name_map() {
    return enum_->has_unstructured_annotation("java.swift.skip_enum_name_map");
  }
  mstch::node num_values() { return enum_->get_enum_values().size(); }
  mstch::node use_intrinsic_default() {
    if (enum_->has_structured_annotation(kJavaUseIntrinsicDefaultUri)) {
      if (enum_->find_value(0) == nullptr) {
        throw std::runtime_error(
            "Enum " + enum_->name() +
            " does not have a value for 0! You have to have value for 0 to use intrinsic default annotation.");
      }
      return true;
    }
    return false;
  }
  mstch::node find_value_zero() {
    if (enum_->has_structured_annotation(kJavaUseIntrinsicDefaultUri)) {
      return java::mangle_java_constant_name(enum_->find_value(0)->name());
    }
    return mstch::node();
  }
};

class mstch_java_const : public mstch_const {
 public:
  mstch_java_const(
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
            {"constant:javaCapitalName", &mstch_java_const::java_capital_name},
            {"constant:javaFieldName", &mstch_java_const::java_field_name},
            {"constant:javaIgnoreConstant?",
             &mstch_java_const::java_ignore_constant},
        });
  }
  mstch::node java_capital_name() {
    return java::mangle_java_constant_name(const_->name());
  }
  mstch::node java_field_name() {
    return java::mangle_java_name(field_->name(), true);
  }
  mstch::node java_ignore_constant() {
    // we have to ignore constants if they are enums that we handled as ints, as
    // we don't have the constant values to work with.
    if (const_->type()->is<t_map>()) {
      t_map* map = (t_map*)const_->type();
      if (map->key_type()->is<t_enum>()) {
        return map->key_type()->has_unstructured_annotation(
            "java.swift.skip_enum_name_map");
      }
    }
    if (const_->type()->is<t_list>()) {
      t_list* list = (t_list*)const_->type();
      if (list->elem_type().get_type()->is<t_enum>()) {
        return list->elem_type().get_type()->has_unstructured_annotation(
            "java.swift.skip_enum_name_map");
      }
    }
    if (const_->type()->is<t_set>()) {
      t_set* set = (t_set*)const_->type();
      if (set->elem_type().get_type()->is<t_enum>()) {
        return set->elem_type().get_type()->has_unstructured_annotation(
            "java.swift.skip_enum_name_map");
      }
    }
    if (const_->generated()) {
      // T194272441 generated schema const is rendered incorrectly.
      return true;
    }
    return mstch::node();
  }
};

class mstch_java_const_value : public mstch_const_value {
 public:
  mstch_java_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type) {
    register_methods(
        this,
        {
            {"value:quotedString", &mstch_java_const_value::quote_java_string},
            {"value:javaEnumValueName",
             &mstch_java_const_value::java_enum_value_name},
        });
  }
  mstch::node quote_java_string() {
    return java::quote_java_string(const_value_->get_string());
  }
  mstch::node java_enum_value_name() {
    if (type_ == cv::CV_INTEGER && const_value_->is_enum()) {
      const t_enum_value* enum_value = const_value_->get_enum_value();
      if (enum_value != nullptr) {
        return java::mangle_java_constant_name(enum_value->name());
      }
      return "fromInteger(" + std::to_string(const_value_->get_integer()) + ")";
    }
    return mstch::node();
  }
  bool same_type_as_expected() const override { return true; }
};

class mstch_java_type : public mstch_type {
 public:
  mstch_java_type(
      const t_type* t, mstch_context& ctx, mstch_element_position pos)
      : mstch_type(t, ctx, pos) {
    register_methods(
        this,
        {
            {"type:isContainer?", &mstch_java_type::is_container_type},
            {"type:javaType", &mstch_java_type::java_type},
            {"type:isBinaryString?", &mstch_java_type::is_binary_string},
            {"type:hasAdapter?", &mstch_java_type::has_type_adapter},
            {"type:adapterClassName",
             &mstch_java_type::get_structured_adapter_class_name},
            {"type:typeClassName",
             &mstch_java_type::get_structured_type_class_name},

            {"type:setIsMapKey",
             {with_no_caching, &mstch_java_type::set_is_map_key}},
            {"type:isMapKey?",
             {with_no_caching, &mstch_java_type::get_map_key_flag}},
            {"type:setIsMapValue",
             {with_no_caching, &mstch_java_type::set_is_map_value}},
            {"type:isMapValue?",
             {with_no_caching, &mstch_java_type::get_map_value_flag}},
            {"type:setIsNotMap",
             {with_no_caching, &mstch_java_type::set_is_not_map}},
            {"type:setAdapter",
             {with_no_caching, &mstch_java_type::set_adapter}},
            {"type:unsetAdapter",
             {with_no_caching, &mstch_java_type::unset_adapter}},
            {"type:isAdapterSet?",
             {with_no_caching, &mstch_java_type::is_adapter_set}},
            {"type:addAdapter",
             {with_no_caching, &mstch_java_type::add_type_adapter}},
            {"type:adapterDefined?",
             {with_no_caching, &mstch_java_type::is_type_adapter_defined}},
            {"type:lastAdapter?",
             {with_no_caching, &mstch_java_type::is_last_type_adapter}},
            {"type:setTypeName",
             {with_no_caching, &mstch_java_type::set_type_name}},
            {"type:getTypeName",
             {with_no_caching, &mstch_java_type::get_type_name}},
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
    return type_->get_true_type()->is<t_container>();
  }

  mstch::node java_type() {
    return type_->get_true_type()->get_unstructured_annotation(
        "java.swift.type");
  }
  mstch::node is_binary_string() {
    return type_->get_true_type()->get_unstructured_annotation(
        "java.swift.binary_string");
  }

  mstch::node has_type_adapter() {
    return has_structured_annotation(kJavaAdapterUri);
  }

  mstch::node has_structured_annotation(const char* uri) {
    if (type_->is<t_typedef>()) {
      if (t_typedef::get_first_structured_annotation_or_null(type_, uri)) {
        return true;
      }
    }
    return false;
  }

  mstch::node get_structured_adapter_class_name() {
    return get_structed_annotation_attribute(
        kJavaAdapterUri, "adapterClassName");
  }

  mstch::node get_structured_type_class_name() {
    return get_structed_annotation_attribute(kJavaAdapterUri, "typeClassName");
  }

  mstch::node get_structed_annotation_attribute(
      const char* uri, const std::string& field) {
    if (type_->is<t_typedef>()) {
      if (auto annotation =
              t_typedef::get_first_structured_annotation_or_null(type_, uri)) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == field) {
            return item.second->get_string();
          }
        }
      }
    }

    return nullptr;
  }

  mstch::node set_adapter() {
    hasTypeAdapter = true;
    return mstch::node();
  }

  mstch::node unset_adapter() {
    hasTypeAdapter = false;
    return mstch::node();
  }

  mstch::node is_adapter_set() { return hasTypeAdapter; }

  mstch::node add_type_adapter() {
    adapterDefinitionSet.insert(type_->name());
    return mstch::node();
  }

  mstch::node is_type_adapter_defined() {
    return adapterDefinitionSet.find(type_->name()) !=
        adapterDefinitionSet.end();
  }

  int32_t nested_adapter_count() {
    int32_t count = 0;
    auto type = type_;
    while (type) {
      if (type_->is<t_typedef>() &&
          type->has_structured_annotation(kJavaAdapterUri)) {
        count++;
        if (const auto* as_typedef = dynamic_cast<const t_typedef*>(type)) {
          type = as_typedef->get_type();
        } else {
          type = nullptr;
        }
      } else {
        type = nullptr;
      }
    }

    return count;
  }

  mstch::node is_last_type_adapter() { return nested_adapter_count() < 2; }

  mstch::node set_type_name() {
    if (nested_adapter_count() > 0) {
      prevTypeName = type_->name();
    }

    return mstch::node();
  }

  mstch::node get_type_name() { return prevTypeName; }
};

void t_mstch_java_generator::generate_program() {
  out_dir_base_ = "gen-java";
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
      get_program()->structured_definitions(),
      "Object");
  generate_rpc_interfaces();
  generate_services();
  generate_items(
      *mstch_context_.enum_factory,
      mstch_context_.enum_cache,
      get_program(),
      get_program()->enums(),
      "Enum");
  generate_constants(get_program());
  generate_placeholder(get_program());
  generate_type_list(get_program());
}

void t_mstch_java_generator::set_mstch_factories() {
  mstch_context_.add<mstch_java_program>();
  mstch_context_.add<mstch_java_service>();
  mstch_context_.add<mstch_java_interaction>();
  mstch_context_.add<mstch_java_function>();
  mstch_context_.add<mstch_java_type>();
  mstch_context_.add<mstch_java_struct>();
  mstch_context_.add<mstch_java_field>();
  mstch_context_.add<mstch_java_enum>();
  mstch_context_.add<mstch_java_const>();
  mstch_context_.add<mstch_java_const_value>();
}

THRIFT_REGISTER_GENERATOR(mstch_java, "Java", "");

} // namespace apache::thrift::compiler
