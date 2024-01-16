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
#include <array>
#include <cassert>
#include <memory>
#include <queue>
#include <set>
#include <string_view>
#include <vector>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fmt/core.h>

#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/gen/cpp/type_resolver.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/lib/cpp2/util.h>
#include <thrift/compiler/lib/uri.h>
#include <thrift/compiler/sema/ast_validator.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

// Since we can not include `thrift/annotation/cpp.thrift`,
// this is a copy of cpp.EnumUnderlyingType.
enum class EnumUnderlyingType {
  I8 = 0,
  U8 = 1,
  I16 = 2,
  U16 = 3,
  U32 = 4,
};

const std::string& get_cpp_template(const t_type* type) {
  return type->get_annotation({"cpp.template", "cpp2.template"});
}

bool is_annotation_blacklisted_in_fatal(const std::string& key) {
  const static std::set<std::string> black_list{
      "cpp.methods",
      "cpp.name",
      "cpp.ref",
      "cpp.ref_type",
      "cpp.template",
      "cpp.type",
      "cpp2.methods",
      "cpp2.ref",
      "cpp2.ref_type",
      "cpp2.template",
      "cpp2.type",
  };
  return black_list.find(key) != black_list.end();
}

bool is_complex_return(const t_type* type) {
  return type->is_container() || type->is_string_or_binary() ||
      type->is_struct() || type->is_exception();
}

bool same_types(const t_type* a, const t_type* b) {
  if (!a || !b) {
    return false;
  }

  if (get_cpp_template(a) != get_cpp_template(b) ||
      cpp2::get_type(a) != cpp2::get_type(b)) {
    return false;
  }

  const auto* resolved_a = a->get_true_type();
  const auto* resolved_b = b->get_true_type();

  if (resolved_a->get_type_value() != resolved_b->get_type_value()) {
    return false;
  }

  switch (resolved_a->get_type_value()) {
    case t_type::type::t_list: {
      const auto* list_a = static_cast<const t_list*>(resolved_a);
      const auto* list_b = static_cast<const t_list*>(resolved_b);
      return same_types(list_a->get_elem_type(), list_b->get_elem_type());
    }
    case t_type::type::t_set: {
      const auto* set_a = static_cast<const t_set*>(resolved_a);
      const auto* set_b = static_cast<const t_set*>(resolved_b);
      return same_types(set_a->get_elem_type(), set_b->get_elem_type());
    }
    case t_type::type::t_map: {
      const auto* map_a = static_cast<const t_map*>(resolved_a);
      const auto* map_b = static_cast<const t_map*>(resolved_b);
      return same_types(map_a->get_key_type(), map_b->get_key_type()) &&
          same_types(map_a->get_val_type(), map_b->get_val_type());
    }
    default:;
  }
  return true;
}

std::vector<t_annotation> get_fatal_annotations(
    deprecated_annotation_map annotations) {
  std::vector<t_annotation> fatal_annotations;
  for (const auto& iter : annotations) {
    if (is_annotation_blacklisted_in_fatal(iter.first)) {
      continue;
    }
    fatal_annotations.push_back({iter.first, iter.second});
  }

  return fatal_annotations;
}

std::string get_fatal_string_short_id(const std::string& key) {
  return boost::algorithm::replace_all_copy(key, ".", "_");
}
std::string get_fatal_string_short_id(const t_named* node) {
  // Use the unmodified cpp name.
  return cpp2::get_name(node);
}

std::string get_fatal_namespace_name_short_id(
    const std::string& lang, const std::string& ns) {
  std::string replacement = lang == "cpp" || lang == "cpp2" ? "__" : "_";
  std::string result = boost::algorithm::replace_all_copy(ns, ".", replacement);
  return result;
}

std::string get_fatal_namespace(
    const std::string& lang, const std::string& ns) {
  if (lang == "cpp" || lang == "cpp2") {
    return boost::algorithm::replace_all_copy(ns, ".", "::");
  } else if (lang == "php") {
    return boost::algorithm::replace_all_copy(ns, ".", "_");
  }
  return ns;
}

std::string render_fatal_string(const std::string& normal_string) {
  const static std::map<char, std::string> substition{
      {'\0', "\\0"},
      {'\n', "\\n"},
      {'\r', "\\r"},
      {'\t', "\\t"},
      {'\'', "\\\'"},
      {'\\', "\\\\"},
  };
  std::ostringstream res;
  res << "::fatal::sequence<char";
  for (const char& c : normal_string) {
    res << ", '";
    auto found = substition.find(c);
    if (found != substition.end()) {
      res << found->second;
    } else {
      res << c;
    }
    res << "'";
  }
  res << ">";
  return res.str();
}

std::string get_out_dir_base(
    const std::map<std::string, std::string>& options) {
  return options.find("py3cpp") != options.end() ? "gen-py3cpp" : "gen-cpp2";
}

std::string mangle_field_name(const std::string& name) {
  return "__fbthrift_field_" + name;
}

bool should_mangle_field_storage_name_in_struct(const t_structured& s) {
  // We don't mangle field name if cpp.methods exist
  return !s.has_annotation({"cpp.methods", "cpp2.methods"});
}

bool resolves_to_container_or_struct(const t_type* type) {
  return type->is_container() || type->is_struct() || type->is_exception();
}

bool is_runtime_annotation(const t_named& named) {
  return named.find_structured_annotation_or_null(kCppRuntimeAnnotation);
}

bool has_runtime_annotation(const t_named& named) {
  return std::any_of(
      named.structured_annotations().begin(),
      named.structured_annotations().end(),
      [](const t_const& cnst) { return is_runtime_annotation(*cnst.type()); });
}

class cpp2_generator_context {
 public:
  static cpp2_generator_context create() { return cpp2_generator_context(); }

  cpp2_generator_context(cpp2_generator_context&&) = default;
  cpp2_generator_context& operator=(cpp2_generator_context&&) = default;

  bool is_orderable(const t_type& type) {
    auto& memo = is_orderable_memo_;
    return cpp2::is_orderable(memo, type);
  }

  gen::cpp::type_resolver& resolver() { return resolver_; }

 private:
  cpp2_generator_context() = default;

  std::unordered_map<const t_type*, bool> is_orderable_memo_;
  gen::cpp::type_resolver resolver_;
};

int checked_stoi(const std::string& s, std::string msg) {
  std::size_t pos = 0;
  int ret = std::stoi(s, &pos);
  if (pos != s.size()) {
    throw std::runtime_error(msg);
  }
  return ret;
}

int get_split_count(const std::map<std::string, std::string>& options) {
  auto iter = options.find("types_cpp_splits");
  if (iter == options.end()) {
    return 0;
  }
  return checked_stoi(
      iter->second, "Invalid types_cpp_splits value: `" + iter->second + "`");
}

class t_mstch_cpp2_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "cpp2"; }
  bool convert_delimiter() const override { return true; }

  void process_options(
      const std::map<std::string, std::string>& options) override {
    t_mstch_generator::process_options(options);
    cpp_context_ = std::make_shared<cpp2_generator_context>(
        cpp2_generator_context::create());
    client_name_to_split_count_ = get_client_name_to_split_count();
    out_dir_base_ = get_out_dir_base(options);
  }

  void generate_program() override;
  void fill_validator_visitors(ast_validator&) const override;
  static std::string get_cpp2_namespace(const t_program* program);
  static std::string get_cpp2_unprefixed_namespace(const t_program* program);
  static mstch::array get_namespace_array(const t_program* program);
  static mstch::array cpp_includes(const t_program* program);
  static mstch::node include_prefix(
      const t_program* program, std::map<std::string, std::string>& options);

 private:
  void set_mstch_factories();
  void generate_sinit(const t_program* program);
  void generate_reflection(const t_program* program);
  void generate_visitation(const t_program* program);
  void generate_constants(const t_program* program);
  void generate_metadata(const t_program* program);
  void generate_structs(const t_program* program);
  void generate_out_of_line_service(const t_service* service);
  void generate_out_of_line_services(const std::vector<t_service*>& services);
  void generate_inline_services(const std::vector<t_service*>& services);

  std::unordered_map<std::string, int> get_client_name_to_split_count() const;

  std::shared_ptr<cpp2_generator_context> cpp_context_;
  std::unordered_map<std::string, int32_t> client_name_to_split_count_;
};

class cpp_mstch_program : public mstch_program {
 public:
  cpp_mstch_program(
      const t_program* program,
      mstch_context& ctx,
      mstch_element_position pos,
      boost::optional<int32_t> split_id = boost::none,
      boost::optional<std::vector<t_structured*>> split_structs = boost::none)
      : mstch_program(program, ctx, pos),
        split_id_(split_id),
        split_structs_(split_structs) {
    register_methods(
        this,
        {{"program:cpp_includes", &cpp_mstch_program::cpp_includes},
         {"program:namespace_cpp2", &cpp_mstch_program::namespace_cpp2},
         {"program:include_prefix", &cpp_mstch_program::include_prefix},
         {"program:cpp_declare_hash?", &cpp_mstch_program::cpp_declare_hash},
         {"program:thrift_includes", &cpp_mstch_program::thrift_includes},
         {"program:frozen_packed?", &cpp_mstch_program::frozen_packed},
         {"program:legacy_api?", &cpp_mstch_program::legacy_api},
         {"program:fatal_languages", &cpp_mstch_program::fatal_languages},
         {"program:fatal_enums", &cpp_mstch_program::fatal_enums},
         {"program:fatal_unions", &cpp_mstch_program::fatal_unions},
         {"program:fatal_structs", &cpp_mstch_program::fatal_structs},
         {"program:fatal_constants", &cpp_mstch_program::fatal_constants},
         {"program:fatal_services", &cpp_mstch_program::fatal_services},
         {"program:fatal_identifiers", &cpp_mstch_program::fatal_identifiers},
         {"program:fatal_data_member", &cpp_mstch_program::fatal_data_member},
         {"program:split_structs", &cpp_mstch_program::split_structs},
         {"program:split_enums", &cpp_mstch_program::split_enums},
         {"program:structs_and_typedefs",
          &cpp_mstch_program::structs_and_typedefs}});
    register_has_option("program:tablebased?", "tablebased");
    register_has_option("program:no_metadata?", "no_metadata");
    register_has_option(
        "program:enforce_required?", "deprecated_enforce_required");
    register_has_option("program:interning?", "interning");
  }
  std::string get_program_namespace(const t_program* program) override {
    return t_mstch_cpp2_generator::get_cpp2_namespace(program);
  }

  std::vector<const t_typedef*> alias_to_struct() {
    std::vector<const t_typedef*> result;
    for (const t_typedef* i : program_->typedefs()) {
      const t_type* alias = i->get_type();
      if (alias->is_typedef() && alias->has_annotation("cpp.type")) {
        const t_type* ttype = i->get_type()->get_true_type();
        if ((ttype->is_struct() || ttype->is_exception()) &&
            !gen::cpp::type_resolver::find_first_adapter(*ttype)) {
          result.push_back(i);
        }
      }
    }
    return result;
  }
  template <typename Node>
  void collect_fatal_string_annotated(
      std::map<std::string, std::string>& fatal_strings, const Node* node) {
    fatal_strings.emplace(get_fatal_string_short_id(node), node->get_name());
    auto hash = cpp2::sha256_hex(node->get_name());
    fatal_strings.emplace("__fbthrift_hash_" + hash, node->get_name());
    for (const auto& a : node->annotations()) {
      if (!is_annotation_blacklisted_in_fatal(a.first)) {
        fatal_strings.emplace(get_fatal_string_short_id(a.first), a.first);
      }
    }
  }
  std::vector<std::string> get_fatal_enum_names() {
    std::vector<std::string> result;
    for (const auto* enm : program_->enums()) {
      result.push_back(get_fatal_string_short_id(enm));
    }
    return result;
  }
  std::vector<std::string> get_fatal_union_names() {
    std::vector<std::string> result;
    for (const t_structured* obj : program_->structured_definitions()) {
      if (obj->is_union()) {
        result.push_back(get_fatal_string_short_id(obj));
      }
    }
    return result;
  }
  std::vector<std::string> get_fatal_struct_names() {
    std::vector<std::string> result;
    for (const t_structured* obj : program_->structured_definitions()) {
      if (!obj->is_union() &&
          !gen::cpp::type_resolver::find_first_adapter(*obj)) {
        result.push_back(get_fatal_string_short_id(obj));
      }
    }
    // typedefs resolve to struct
    for (const t_typedef* i : alias_to_struct()) {
      result.push_back(get_fatal_string_short_id(i));
    }
    return result;
  }
  std::vector<std::string> get_fatal_constant_names() {
    std::vector<std::string> result;
    for (const auto* cnst : program_->consts()) {
      result.push_back(get_fatal_string_short_id(cnst));
    }
    return result;
  }
  std::vector<std::string> get_fatal_service_names() {
    std::vector<std::string> result;
    for (const auto* service : program_->services()) {
      result.push_back(get_fatal_string_short_id(service));
    }
    return result;
  }
  mstch::node to_fatal_string_array(const std::vector<std::string>&& vec) {
    mstch::array a;
    for (size_t i = 0; i < vec.size(); i++) {
      a.push_back(mstch::map{
          {"fatal_string:name", vec.at(i)},
          {"last?", i == vec.size() - 1},
      });
    }
    return mstch::map{{"fatal_strings:items", a}};
  }

  mstch::node namespace_cpp2() {
    return t_mstch_cpp2_generator::get_namespace_array(program_);
  }
  mstch::node cpp_includes() {
    mstch::array includes = t_mstch_cpp2_generator::cpp_includes(program_);
    auto it = context_.options.find("includes");
    if (it != context_.options.end()) {
      std::vector<std::string> extra_includes;
      boost::split(extra_includes, it->second, [](char c) { return c == ':'; });
      for (auto& include : extra_includes) {
        includes.push_back(mstch::map{{"cpp_include", std::move(include)}});
      }
    }
    return includes;
  }
  mstch::node include_prefix() {
    return t_mstch_cpp2_generator::include_prefix(program_, context_.options);
  }
  mstch::node cpp_declare_hash() {
    bool cpp_declare_in_structs = std::any_of(
        program_->structs_and_unions().begin(),
        program_->structs_and_unions().end(),
        [](const t_structured* strct) {
          return strct->has_annotation(
              {"cpp.declare_hash", "cpp2.declare_hash"});
        });
    bool cpp_declare_in_typedefs = std::any_of(
        program_->typedefs().begin(),
        program_->typedefs().end(),
        [](const auto* typedf) {
          return typedf->get_type()->has_annotation(
              {"cpp.declare_hash", "cpp2.declare_hash"});
        });
    return cpp_declare_in_structs || cpp_declare_in_typedefs;
  }
  mstch::node thrift_includes() {
    mstch::array a;
    for (const auto* program : program_->get_includes_for_codegen()) {
      a.push_back(make_mstch_program_cached(program, context_));
    }
    return a;
  }
  mstch::node frozen_packed() { return get_option("frozen") == "packed"; }
  mstch::node legacy_api() {
    return ::apache::thrift::compiler::generate_legacy_api(*program_);
  }
  mstch::node fatal_languages() {
    mstch::array a;
    for (const auto& pair : program_->namespaces()) {
      if (!pair.second.empty()) {
        a.push_back(mstch::map{
            {"language:safe_name", get_fatal_string_short_id(pair.first)},
            {"language:safe_namespace",
             get_fatal_namespace_name_short_id(pair.first, pair.second)},
            {"last?", false},
        });
      }
    }
    if (!a.empty()) {
      boost::get<mstch::map>(a.back())["last?"] = true;
    }
    return mstch::map{{"fatal_languages:items", a}};
  }
  mstch::node fatal_enums() {
    return to_fatal_string_array(get_fatal_enum_names());
  }
  mstch::node fatal_unions() {
    return to_fatal_string_array(get_fatal_union_names());
  }
  mstch::node fatal_structs() {
    return to_fatal_string_array(get_fatal_struct_names());
  }
  mstch::node fatal_constants() {
    return to_fatal_string_array(get_fatal_constant_names());
  }
  mstch::node fatal_services() {
    return to_fatal_string_array(get_fatal_service_names());
  }
  mstch::node fatal_identifiers() {
    std::map<std::string, std::string> unique_names;
    unique_names.emplace(get_fatal_string_short_id(program_), program_->name());
    // languages and namespaces
    for (const auto& pair : program_->namespaces()) {
      if (!pair.second.empty()) {
        unique_names.emplace(get_fatal_string_short_id(pair.first), pair.first);
        unique_names.emplace(
            get_fatal_namespace_name_short_id(pair.first, pair.second),
            get_fatal_namespace(pair.first, pair.second));
      }
    }
    // enums
    for (const auto* enm : program_->enums()) {
      collect_fatal_string_annotated(unique_names, enm);
      unique_names.emplace(get_fatal_string_short_id(enm), enm->get_name());
      for (const auto& i : enm->get_enum_values()) {
        collect_fatal_string_annotated(unique_names, i);
      }
    }
    // structs, unions and exceptions
    for (const t_structured* obj : program_->structured_definitions()) {
      if (obj->is_union()) {
        // When generating <program_name>_fatal_union.h, we will generate
        // <union_name>_Type_enum_traits
        unique_names.emplace("Type", "Type");
      }
      collect_fatal_string_annotated(unique_names, obj);
      for (const auto& m : obj->fields()) {
        collect_fatal_string_annotated(unique_names, &m);
      }
    }
    // consts
    for (const auto* cnst : program_->consts()) {
      unique_names.emplace(get_fatal_string_short_id(cnst), cnst->get_name());
    }
    // services
    for (const auto* service : program_->services()) {
      // function annotations are not currently included.
      unique_names.emplace(
          get_fatal_string_short_id(service), service->get_name());
      for (const auto* f : service->get_functions()) {
        unique_names.emplace(get_fatal_string_short_id(f), f->get_name());
        for (const auto& p : f->params().fields()) {
          unique_names.emplace(get_fatal_string_short_id(&p), p.name());
        }
      }
    }
    // typedefs resolve to struct
    for (const t_typedef* i : alias_to_struct()) {
      unique_names.emplace(get_fatal_string_short_id(i), i->get_name());
    }

    mstch::array a;
    for (const auto& name : unique_names) {
      a.push_back(mstch::map{
          {"identifier:name", name.first},
          {"identifier:fatal_string", render_fatal_string(name.second)},
      });
    }
    return a;
  }
  mstch::node fatal_data_member() {
    std::unordered_set<std::string> fields;
    std::vector<const std::string*> ordered_fields;
    for (const t_structured* s : program_->structured_definitions()) {
      if (s->is_union()) {
        continue;
      }
      for (const t_field& f : s->fields()) {
        auto result = fields.insert(cpp2::get_name(&f));
        if (result.second) {
          ordered_fields.push_back(&*result.first);
        }
      }
    }
    mstch::array a;
    for (const auto& f : ordered_fields) {
      a.push_back(*f);
    }
    return a;
  }
  mstch::node structs_and_typedefs() {
    // We combine these because the adapter trait used in typedefs requires the
    // typedefed struct to be complete, and the typedefs themselves cannot be
    // forward declared.
    // Topo sort the combined set to fulfill these requirements.
    // As in other parts of this codebase, structs includes unions and
    // exceptions.
    std::vector<const t_type*> nodes;
    nodes.reserve(
        program_->structured_definitions().size() +
        program_->typedefs().size());
    nodes.insert(
        nodes.end(), program_->typedefs().begin(), program_->typedefs().end());
    nodes.insert(
        nodes.end(),
        program_->structured_definitions().begin(),
        program_->structured_definitions().end());
    auto deps = cpp2::gen_dependency_graph(program_, nodes);
    auto sorted = cpp2::topological_sort<const t_type*>(
        nodes.begin(), nodes.end(), deps, true);

    // Generate the sorted nodes
    mstch::array ret;
    ret.reserve(sorted.size());
    std::string id = program_->name() + get_program_namespace(program_);
    std::transform(
        sorted.begin(),
        sorted.end(),
        std::back_inserter(ret),
        [&](const t_type* node) -> mstch::node {
          if (auto typedf = dynamic_cast<t_typedef const*>(node)) {
            return context_.typedef_factory->make_mstch_object(
                typedf, context_);
          }
          return make_mstch_element_cached(
              static_cast<const t_struct*>(node),
              *context_.struct_factory,
              context_.struct_cache,
              id,
              0,
              0);
        });
    return ret;
  }

  mstch::node split_structs() {
    std::string id = program_->name() + get_program_namespace(program_);
    return make_mstch_array_cached(
        split_id_ ? *split_structs_ : program_->structured_definitions(),
        *context_.struct_factory,
        context_.struct_cache,
        id);
  }

  mstch::node split_enums() {
    if (split_id_) {
      if (!split_enums_) {
        int split_count = std::max(get_split_count(context_.options), 1);
        const size_t cnt = program_->enums().size();
        split_enums_.emplace();
        for (size_t i = *split_id_; i < cnt; i += split_count) {
          split_enums_->push_back(program_->enums()[i]);
        }
      }
    }
    std::string id = program_->name() + get_program_namespace(program_);
    return make_mstch_array_cached(
        split_id_ ? *split_enums_ : program_->enums(),
        *context_.enum_factory,
        context_.enum_cache,
        id);
  }

 private:
  const boost::optional<int32_t> split_id_;
  const boost::optional<std::vector<t_structured*>> split_structs_;
  boost::optional<std::vector<t_enum*>> split_enums_;
};

class cpp_mstch_service : public mstch_service {
 public:
  cpp_mstch_service(
      const t_service* service,
      mstch_context& ctx,
      mstch_element_position pos,
      int32_t split_id = 0,
      int32_t split_count = 1)
      : mstch_service(service, ctx, pos) {
    register_methods(
        this,
        {
            {"service:program_name", &cpp_mstch_service::program_name},
            {"service:program_qualified_name",
             &cpp_mstch_service::program_qualified_name},
            {"service:autogen_path", &cpp_mstch_service::autogen_path},
            {"service:include_prefix", &cpp_mstch_service::include_prefix},
            {"service:thrift_includes", &cpp_mstch_service::thrift_includes},
            {"service:namespace_cpp2", &cpp_mstch_service::namespace_cpp2},
            {"service:oneway_functions", &cpp_mstch_service::oneway_functions},
            {"service:oneways?", &cpp_mstch_service::has_oneway},
            {"service:cpp_includes", &cpp_mstch_service::cpp_includes},
            {"service:metadata_name", &cpp_mstch_service::metadata_name},
            {"service:cpp_name", &cpp_mstch_service::cpp_name},
            {"service:qualified_name", &cpp_mstch_service::qualified_name},
            {"service:parent_service_name",
             &cpp_mstch_service::parent_service_name},
            {"service:parent_service_cpp_name",
             &cpp_mstch_service::parent_service_cpp_name},
            {"service:parent_service_qualified_name",
             &cpp_mstch_service::parent_service_qualified_name},
            {"service:thrift_uri_or_service_name",
             &cpp_mstch_service::thrift_uri_or_service_name},
            {"service:service_schema_name",
             &cpp_mstch_service::service_schema_name},
            {"service:has_service_schema",
             &cpp_mstch_service::has_service_schema},
            {"service:cpp_typed_interceptor?",
             &cpp_mstch_service::generate_typed_interceptor},
            {"service:reduced_client?", &cpp_mstch_service::reduced_client},
        });

    const auto all_functions = mstch_service::get_functions();
    for (size_t id = split_id; id < all_functions.size(); id += split_count) {
      functions_.push_back(all_functions[id]);
    }
  }
  std::string get_service_namespace(const t_program* program) override {
    return t_mstch_cpp2_generator::get_cpp2_namespace(program);
  }
  mstch::node program_name() { return service_->program()->name(); }
  mstch::node program_qualified_name() {
    return get_service_namespace(service_->program()) +
        "::" + service_->program()->name();
  }
  mstch::node autogen_path() {
    std::string path = service_->program()->path();
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
  }
  mstch::node cpp_includes() {
    return t_mstch_cpp2_generator::cpp_includes(service_->program());
  }
  mstch::node include_prefix() {
    return t_mstch_cpp2_generator::include_prefix(
        service_->program(), context_.options);
  }
  mstch::node thrift_includes() {
    mstch::array a;
    for (const auto* program :
         service_->program()->get_includes_for_codegen()) {
      a.push_back(make_mstch_program_cached(program, context_));
    }
    return a;
  }
  mstch::node namespace_cpp2() {
    return t_mstch_cpp2_generator::get_namespace_array(service_->program());
  }
  mstch::node oneway_functions() {
    std::vector<const t_function*> oneway_functions;
    for (const auto* function : get_functions()) {
      if (function->qualifier() == t_function_qualifier::oneway) {
        oneway_functions.push_back(function);
      }
    }
    return make_mstch_functions(oneway_functions, service_);
  }
  mstch::node has_oneway() {
    for (const auto* function : get_functions()) {
      if (function->qualifier() == t_function_qualifier::oneway) {
        return true;
      }
    }
    return false;
  }
  mstch::node metadata_name() {
    return service_->program()->name() + "_" + service_->get_name();
  }
  mstch::node cpp_name() {
    return service_->is_interaction() ? service_->name()
                                      : cpp2::get_name(service_);
  }
  mstch::node qualified_name() {
    return cpp2::get_service_qualified_name(*service_);
  }
  virtual mstch::node parent_service_name() { return service_->get_name(); }
  virtual mstch::node parent_service_cpp_name() { return cpp_name(); }
  virtual mstch::node parent_service_qualified_name() {
    return qualified_name();
  }
  mstch::node reduced_client() {
    return service_->is_interaction() || !generate_legacy_api(*service_);
  }
  mstch::node thrift_uri_or_service_name() {
    return service_->uri().empty() ? parent_service_name() : service_->uri();
  }
  mstch::node has_service_schema() {
    const t_const* annotation =
        service_->find_structured_annotation_or_null(kGenerateRuntimeSchemaUri);
    return annotation ? true : false;
  }

  mstch::node service_schema_name() {
    const t_const* annotation =
        service_->find_structured_annotation_or_null(kGenerateRuntimeSchemaUri);
    if (!annotation) {
      return "";
    }

    std::string name;
    if (auto nameOverride = annotation
            ? annotation->get_value_from_structured_annotation_or_null("name")
            : nullptr) {
      name = nameOverride->get_string();
    } else {
      name = fmt::format("schema{}", service_->name());
    }
    return name;
  }
  mstch::node generate_typed_interceptor() {
    if (service_->find_structured_annotation_or_null(
            kCppGenerateTypedInterceptor)) {
      return true;
    }
    for (const auto* function : get_functions()) {
      if (function->find_structured_annotation_or_null(
              kCppGenerateTypedInterceptor)) {
        return true;
      }
    }
    for (const auto* interaction : interactions_) {
      if (interaction->find_structured_annotation_or_null(
              kCppGenerateTypedInterceptor)) {
        return true;
      }
    }
    return false;
  }

 private:
  const std::vector<t_function*>& get_functions() const override {
    return functions_;
  }

  std::vector<t_function*> functions_;
};

class cpp_mstch_interaction : public cpp_mstch_service {
 public:
  using ast_type = t_interaction;

  cpp_mstch_interaction(
      const t_interaction* interaction,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service)
      : cpp_mstch_service(interaction, ctx, pos),
        containing_service_(containing_service) {}

  mstch::node parent_service_name() override {
    return containing_service_->get_name();
  }
  mstch::node parent_service_cpp_name() override {
    return cpp2::get_name(containing_service_);
  }
  mstch::node parent_service_qualified_name() override {
    return cpp2::get_service_qualified_name(*containing_service_);
  }

 private:
  const t_service* containing_service_ = nullptr;
};

class cpp_mstch_function : public mstch_function {
 public:
  cpp_mstch_function(
      const t_function* function,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_interface* iface,
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_function(function, ctx, pos, iface),
        cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"function:eb", &cpp_mstch_function::event_based},
            {"function:cpp_name", &cpp_mstch_function::cpp_name},
            {"function:cpp_return_type", &cpp_mstch_function::cpp_return_type},
            {"function:cpp_void?", &cpp_mstch_function::is_cpp_void},
            {"function:stack_arguments?", &cpp_mstch_function::stack_arguments},
            {"function:created_interaction",
             &cpp_mstch_function::created_interaction},
            {"function:cpp_typed_interceptor?",
             &cpp_mstch_function::generate_typed_interceptor},
            {"function:sync_returns_by_outparam?",
             &cpp_mstch_function::sync_returns_by_outparam},
            {"function:prefixed_name", &cpp_mstch_function::prefixed_name},
        });
  }
  mstch::node event_based() {
    return function_->get_annotation("thread") == "eb" ||
        function_->find_structured_annotation_or_null(
            kCppProcessInEbThreadUri) ||
        interface_->find_annotation_or_null("process_in_event_base") ||
        interface_->find_structured_annotation_or_null(
            kCppProcessInEbThreadUri);
  }
  mstch::node cpp_name() { return cpp2::get_name(function_); }
  mstch::node cpp_return_type() {
    return cpp_context_->resolver().get_return_type(*function_);
  }
  // Specifies if the generated C++ function is void.
  mstch::node is_cpp_void() {
    return function_->return_type()->is_void() && !function_->sink_or_stream();
  }
  mstch::node stack_arguments() {
    return cpp2::is_stack_arguments(context_.options, *function_);
  }
  mstch::node created_interaction() {
    return cpp2::get_name(function_->interaction().get_type());
  }
  mstch::node sync_returns_by_outparam() {
    return is_complex_return(function_->return_type()->get_true_type()) &&
        !function_->interaction() && !function_->sink_or_stream();
  }
  mstch::node generate_typed_interceptor() {
    // Functions that are an interaction constructor, don't create an RPC. hence
    // we don't generate interceptors for them.
    return !function_->is_interaction_constructor() &&
        (function_->find_structured_annotation_or_null(
             kCppGenerateTypedInterceptor) ||
         interface_->find_structured_annotation_or_null(
             kCppGenerateTypedInterceptor));
  }

  mstch::node prefixed_name() {
    return interface_->is_interaction()
        ? fmt::format(
              "{}_{}", interface_->get_name(), cpp2::get_name(function_))
        : cpp_name();
  }

 private:
  std::shared_ptr<cpp2_generator_context> cpp_context_;
};

bool needs_op_encode(const t_type& type);

bool check_container_needs_op_encode(const t_type& type) {
  const auto* true_type = type.get_true_type();
  if (auto container = dynamic_cast<const t_list*>(true_type)) {
    return needs_op_encode(*container->elem_type());
  } else if (auto container_2 = dynamic_cast<const t_set*>(true_type)) {
    return needs_op_encode(*container_2->elem_type());
  } else if (auto container_2 = dynamic_cast<const t_map*>(true_type)) {
    return needs_op_encode(*container_2->key_type()) ||
        needs_op_encode(*container_2->val_type());
  }
  return false;
}

bool needs_op_encode(const t_type& type) {
  return (type.program() &&
          type.program()->inherit_annotation_or_null(
              type, kCppUseOpEncodeUri)) ||
      t_typedef::get_first_structured_annotation_or_null(
             &type, kCppUseOpEncodeUri) ||
      gen::cpp::type_resolver::find_first_adapter(type) ||
      check_container_needs_op_encode(type);
}

// Enable `@cpp.UseOpEncode` for following fields:
// - A package is annotated with `@cpp.UseOpEncode`
// - A parent struct is annotated with `@cpp.UseOpEncode`
// - A container has a key or element type marked with `@cpp.UseOpEncode`
// - A container has an adapted key or element type.
// - A field is adapted.
bool needs_op_encode(const t_field& field, const t_structured& strct) {
  return (strct.program() &&
          strct.program()->inherit_annotation_or_null(
              strct, kCppUseOpEncodeUri)) ||
      strct.find_structured_annotation_or_null(kCppUseOpEncodeUri) ||
      gen::cpp::type_resolver::find_first_adapter(field) ||
      check_container_needs_op_encode(*field.type());
}

class cpp_mstch_type : public mstch_type {
 public:
  cpp_mstch_type(
      const t_type* type,
      mstch_context& ctx,
      mstch_element_position pos,
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_type(type, ctx, pos), cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"type:resolves_to_base?", &cpp_mstch_type::resolves_to_base},
            {"type:resolves_to_integral?",
             &cpp_mstch_type::resolves_to_integral},
            {"type:resolves_to_base_or_enum?",
             &cpp_mstch_type::resolves_to_base_or_enum},
            {"type:resolves_to_container?",
             &cpp_mstch_type::resolves_to_container},
            {"type:resolves_to_container_or_struct?",
             &cpp_mstch_type::resolves_to_container_or_struct},
            {"type:resolves_to_container_or_enum?",
             &cpp_mstch_type::resolves_to_container_or_enum},
            {"type:resolves_to_complex_return?",
             &cpp_mstch_type::resolves_to_complex_return},
            {"type:resolves_to_fixed_size?",
             &cpp_mstch_type::resolves_to_fixed_size},
            {"type:resolves_to_enum?", &cpp_mstch_type::resolves_to_enum},
            {"type:transitively_refers_to_struct?",
             &cpp_mstch_type::transitively_refers_to_struct},
            {"type:cpp_name", &cpp_mstch_type::cpp_name},
            {"type:cpp_fullname", &cpp_mstch_type::cpp_fullname},
            {"type:cpp_type", &cpp_mstch_type::cpp_type},
            {"type:cpp_standard_type", &cpp_mstch_type::cpp_standard_type},
            {"type:cpp_adapter", &cpp_mstch_type::cpp_adapter},
            {"type:string_or_binary?", &cpp_mstch_type::is_string_or_binary},
            {"type:resolved_cpp_type", &cpp_mstch_type::resolved_cpp_type},
            {"type:cpp_template", &cpp_mstch_type::cpp_template},
            {"type:cpp_indirection?", &cpp_mstch_type::cpp_indirection},
            {"type:non_empty_struct?", &cpp_mstch_type::is_non_empty_struct},
            {"type:namespace_cpp2", &cpp_mstch_type::namespace_cpp2},
            {"type:cpp_declare_hash", &cpp_mstch_type::cpp_declare_hash},
            {"type:cpp_declare_equal_to",
             &cpp_mstch_type::cpp_declare_equal_to},
            {"type:type_class", &cpp_mstch_type::type_class},
            {"type:type_tag", &cpp_mstch_type::type_tag},
            {"type:type_class_with_indirection",
             &cpp_mstch_type::type_class_with_indirection},
            {"type:program_name", &cpp_mstch_type::program_name},
            {"type:cpp_use_allocator?", &cpp_mstch_type::cpp_use_allocator},
            {"type:use_op_encode?", &cpp_mstch_type::use_op_encode},
        });
    register_has_option(
        "type:sync_methods_return_try?", "sync_methods_return_try");
  }
  std::string get_type_namespace(const t_program* program) override {
    return cpp2::get_gen_namespace(*program);
  }
  mstch::node resolves_to_base() { return resolved_type_->is_base_type(); }
  mstch::node resolves_to_integral() {
    return resolved_type_->is_byte() || resolved_type_->is_any_int();
  }
  mstch::node resolves_to_base_or_enum() {
    return resolved_type_->is_base_type() || resolved_type_->is_enum();
  }
  mstch::node resolves_to_container() { return resolved_type_->is_container(); }
  mstch::node resolves_to_container_or_struct() {
    return ::apache::thrift::compiler::resolves_to_container_or_struct(
        resolved_type_);
  }
  mstch::node resolves_to_container_or_enum() {
    return resolved_type_->is_container() || resolved_type_->is_enum();
  }
  mstch::node resolves_to_complex_return() {
    return is_complex_return(resolved_type_) && !resolved_type_->is_service();
  }
  mstch::node resolves_to_fixed_size() {
    return resolved_type_->is_bool() || resolved_type_->is_byte() ||
        resolved_type_->is_any_int() || resolved_type_->is_enum() ||
        resolved_type_->is_floating_point();
  }
  mstch::node resolves_to_enum() { return resolved_type_->is_enum(); }
  mstch::node transitively_refers_to_struct() {
    // fast path is unnecessary but may avoid allocations
    if (resolved_type_->is_struct()) {
      return true;
    }
    if (!resolved_type_->is_container()) {
      return false;
    }
    // type is a container: traverse (breadthwise, but could be depthwise)
    std::queue<const t_type*> queue;
    queue.push(resolved_type_);
    while (!queue.empty()) {
      auto next = queue.front();
      queue.pop();
      if (next->is_struct()) {
        return true;
      }
      if (!next->is_container()) {
        continue;
      }
      if (false) {
      } else if (next->is_list()) {
        queue.push(static_cast<const t_list*>(next)->get_elem_type());
      } else if (next->is_set()) {
        queue.push(static_cast<const t_set*>(next)->get_elem_type());
      } else if (next->is_map()) {
        queue.push(static_cast<const t_map*>(next)->get_key_type());
        queue.push(static_cast<const t_map*>(next)->get_val_type());
      } else {
        assert(false);
      }
    }
    return false;
  }
  mstch::node cpp_name() { return cpp2::get_name(type_); }
  mstch::node cpp_fullname() {
    return cpp_context_->resolver().get_namespaced_name(
        *type_->get_program(), *type_);
  }
  mstch::node cpp_type() {
    return cpp_context_->resolver().get_native_type(*type_);
  }
  mstch::node cpp_standard_type() {
    return cpp_context_->resolver().get_standard_type(*type_);
  }
  mstch::node cpp_adapter() {
    if (const auto* adapter =
            gen::cpp::type_resolver::find_first_adapter(*type_)) {
      return *adapter;
    }
    return {};
  }
  mstch::node resolved_cpp_type() {
    return fmt::to_string(cpp2::get_type(resolved_type_));
  }
  mstch::node is_string_or_binary() {
    return resolved_type_->is_string_or_binary();
  }
  mstch::node cpp_template() { return get_cpp_template(type_); }
  mstch::node cpp_indirection() {
    return resolved_type_->has_annotation("cpp.indirection");
  }
  mstch::node cpp_declare_hash() {
    return resolved_type_->has_annotation(
        {"cpp.declare_hash", "cpp2.declare_hash"});
  }
  mstch::node cpp_declare_equal_to() {
    return resolved_type_->has_annotation(
        {"cpp.declare_equal_to", "cpp2.declare_equal_to"});
  }
  mstch::node cpp_use_allocator() {
    return t_typedef::get_first_annotation_or_null(
        type_, {"cpp.use_allocator"});
  }
  mstch::node is_non_empty_struct() {
    auto as_struct = dynamic_cast<const t_struct*>(resolved_type_);
    return as_struct && as_struct->has_fields();
  }
  mstch::node namespace_cpp2() {
    return t_mstch_cpp2_generator::get_namespace_array(type_->program());
  }
  mstch::node type_class() { return cpp2::get_gen_type_class(*resolved_type_); }
  mstch::node type_tag() {
    return cpp_context_->resolver().get_type_tag(*type_);
  }
  mstch::node type_class_with_indirection() {
    return cpp2::get_gen_type_class_with_indirection(*resolved_type_);
  }
  mstch::node program_name() {
    std::string name;
    if (auto prog = type_->program()) {
      name = prog->name();
    }
    return name;
  }
  mstch::node use_op_encode() { return needs_op_encode(*type_); }

 private:
  std::shared_ptr<cpp2_generator_context> cpp_context_;
};

class cpp_mstch_typedef : public mstch_typedef {
 public:
  cpp_mstch_typedef(
      const t_typedef* t,
      mstch_context& ctx,
      mstch_element_position pos,
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_typedef(t, ctx, pos), cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"typedef:cpp_type", &cpp_mstch_typedef::cpp_type},
        });
  }
  mstch::node cpp_type() {
    return cpp_context_->resolver().get_underlying_type_name(*typedef_);
  }

 private:
  std::shared_ptr<cpp2_generator_context> cpp_context_;
};

class cpp_mstch_struct : public mstch_struct {
 public:
  cpp_mstch_struct(
      const t_structured* s,
      mstch_context& ctx,
      mstch_element_position pos,
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_struct(s, ctx, pos), cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"struct:fields_size", &cpp_mstch_struct::fields_size},
            {"struct:explicitly_constructed_fields",
             &cpp_mstch_struct::explicitly_constructed_fields},
            {"struct:fields_in_key_order",
             &cpp_mstch_struct::fields_in_key_order},
            {"struct:fields_in_layout_order",
             &cpp_mstch_struct::fields_in_layout_order},
            {"struct:is_struct_orderable?",
             &cpp_mstch_struct::is_struct_orderable},
            {"struct:nondefault_copy_ctor_and_assignment?",
             &cpp_mstch_struct::nondefault_copy_ctor_and_assignment},
            {"struct:cpp_underlying_name",
             &cpp_mstch_struct::cpp_underlying_name},
            {"struct:cpp_underlying_type",
             &cpp_mstch_struct::cpp_underlying_type},
            {"struct:is_directly_adapted?",
             &cpp_mstch_struct::is_directly_adapted},
            {"struct:dependent_direct_adapter?",
             &cpp_mstch_struct::dependent_direct_adapter},
            {"struct:cpp_name", &cpp_mstch_struct::cpp_name},
            {"struct:cpp_fullname", &cpp_mstch_struct::cpp_fullname},
            {"struct:cpp_methods", &cpp_mstch_struct::cpp_methods},
            {"struct:cpp_declare_hash", &cpp_mstch_struct::cpp_declare_hash},
            {"struct:cpp_declare_equal_to",
             &cpp_mstch_struct::cpp_declare_equal_to},
            {"struct:cpp_noncopyable", &cpp_mstch_struct::cpp_noncopyable},
            {"struct:cpp_noncomparable", &cpp_mstch_struct::cpp_noncomparable},
            {"struct:cpp_trivially_relocatable",
             &cpp_mstch_struct::cpp_trivially_relocatable},
            {"struct:cpp_runtime_annotation?",
             &cpp_mstch_struct::cpp_runtime_annotation},
            {"struct:is_eligible_for_constexpr?",
             &cpp_mstch_struct::is_eligible_for_constexpr},
            {"struct:virtual", &cpp_mstch_struct::cpp_virtual},
            {"struct:message", &cpp_mstch_struct::message},
            {"struct:isset_fields?", &cpp_mstch_struct::has_isset_fields},
            {"struct:isset_fields", &cpp_mstch_struct::isset_fields},
            {"struct:isset_fields_size", &cpp_mstch_struct::isset_fields_size},
            {"struct:isset_bitset_option",
             &cpp_mstch_struct::isset_bitset_option},
            {"struct:lazy_fields?", &cpp_mstch_struct::has_lazy_fields},
            {"struct:indexing?", &cpp_mstch_struct::indexing},
            {"struct:write_lazy_field_checksum",
             &cpp_mstch_struct::write_lazy_field_checksum},
            {"struct:is_large?", &cpp_mstch_struct::is_large},
            {"struct:fatal_annotations?",
             &cpp_mstch_struct::has_fatal_annotations},
            {"struct:fatal_annotations", &cpp_mstch_struct::fatal_annotations},
            {"struct:legacy_type_id", &cpp_mstch_struct::get_legacy_type_id},
            {"struct:legacy_api?", &cpp_mstch_struct::legacy_api},
            {"struct:metadata_name", &cpp_mstch_struct::metadata_name},
            {"struct:mixin_fields", &cpp_mstch_struct::mixin_fields},
            {"struct:num_union_members",
             &cpp_mstch_struct::get_num_union_members},
            {"struct:cpp_allocator", &cpp_mstch_struct::cpp_allocator},
            {"struct:cpp_allocator_via", &cpp_mstch_struct::cpp_allocator_via},
            {"struct:cpp_frozen2_exclude?",
             &cpp_mstch_struct::cpp_frozen2_exclude},
            {"struct:has_non_optional_and_non_terse_field?",
             &cpp_mstch_struct::has_non_optional_and_non_terse_field},
            {"struct:has_field_with_runtime_annotation?",
             &cpp_mstch_struct::has_field_with_runtime_annotation},
            {"struct:any?", &cpp_mstch_struct::any},
            {"struct:scoped_enum_as_union_type?",
             &cpp_mstch_struct::scoped_enum_as_union_type},
            {"struct:extra_namespace", &cpp_mstch_struct::extra_namespace},
            {"struct:type_tag", &cpp_mstch_struct::type_tag},
            {"struct:patch?", &cpp_mstch_struct::patch},
            {"struct:is_trivially_destructible?",
             &cpp_mstch_struct::is_trivially_destructible},
        });
  }
  mstch::node fields_size() { return std::to_string(struct_->fields().size()); }
  mstch::node explicitly_constructed_fields() {
    // Filter fields according to the following criteria:
    // Get all enums
    // Get all base_types but empty strings
    // Get all non-empty structs and containers
    // Get all non-optional references with basetypes, enums,
    // non-empty structs, and containers
    std::vector<const t_field*> filtered_fields;
    for (const auto* field : get_members_in_layout_order()) {
      const t_type* type = field->get_type()->get_true_type();
      // Filter out all optional references.
      if (cpp2::is_explicit_ref(field) &&
          field->get_req() == t_field::e_req::optional) {
        continue;
      }
      if (type->is_enum() ||
          (type->is_base_type() && !type->is_string_or_binary()) ||
          (type->is_string_or_binary() && field->get_value() != nullptr) ||
          (type->is_container() && field->get_value() != nullptr &&
           !field->get_value()->is_empty()) ||
          (type->is_struct() &&
           (struct_ != dynamic_cast<const t_struct*>(type)) &&
           ((field->get_value() && !field->get_value()->is_empty()) ||
            (cpp2::is_explicit_ref(field) &&
             field->get_req() != t_field::e_req::optional))) ||
          (type->is_container() && cpp2::is_explicit_ref(field) &&
           field->get_req() != t_field::e_req::optional) ||
          (type->is_base_type() && cpp2::is_explicit_ref(field) &&
           field->get_req() != t_field::e_req::optional)) {
        filtered_fields.push_back(field);
      }
    }
    return make_mstch_fields(filtered_fields);
  }

  mstch::node mixin_fields() {
    mstch::array fields;
    for (auto i : cpp2::get_mixins_and_members(*struct_)) {
      const auto suffix =
          ::apache::thrift::compiler::generate_legacy_api(*struct_) ||
              i.mixin->type()->is_union()
          ? "_ref"
          : "";
      fields.push_back(mstch::map{
          {"mixin:name", i.mixin->get_name()},
          {"mixin:field_name", i.member->get_name()},
          {"mixin:accessor", i.member->get_name() + suffix}});
    }
    return fields;
  }

  mstch::node is_struct_orderable() {
    return cpp_context_->is_orderable(*struct_) &&
        !struct_->has_annotation("no_default_comparators");
  }
  mstch::node nondefault_copy_ctor_and_assignment() {
    if (struct_->has_annotation("cpp.allocator")) {
      return true;
    }
    for (const auto& f : struct_->fields()) {
      if (cpp2::field_transitively_refers_to_unique(&f) || cpp2::is_lazy(&f) ||
          gen::cpp::type_resolver::find_first_adapter(f)) {
        return true;
      }
    }
    return false;
  }
  mstch::node cpp_name() { return cpp2::get_name(struct_); }
  mstch::node cpp_fullname() {
    return cpp_context_->resolver().get_underlying_namespaced_name(*struct_);
  }
  mstch::node cpp_underlying_name() {
    return cpp_context_->resolver().get_underlying_name(*struct_);
  }
  mstch::node cpp_underlying_type() {
    return cpp_context_->resolver().get_underlying_type_name(*struct_);
  }
  mstch::node is_directly_adapted() {
    return cpp_context_->resolver().is_directly_adapted(*struct_);
  }
  mstch::node dependent_direct_adapter() {
    auto adapter =
        cpp_context_->resolver().find_nontransitive_adapter(*struct_);
    return adapter &&
        !adapter->get_value_from_structured_annotation_or_null("adaptedType");
  }

  mstch::node cpp_methods() {
    return struct_->get_annotation({"cpp.methods", "cpp2.methods"});
  }
  mstch::node cpp_declare_hash() {
    return struct_->has_annotation({"cpp.declare_hash", "cpp2.declare_hash"});
  }
  mstch::node cpp_declare_equal_to() {
    return struct_->has_annotation(
        {"cpp.declare_equal_to", "cpp2.declare_equal_to"});
  }
  mstch::node cpp_noncopyable() {
    if (struct_->has_annotation({"cpp.noncopyable", "cpp2.noncopyable"})) {
      return true;
    }

    bool result = false;
    cpp2::for_each_transitive_field(struct_, [&result](const t_field* field) {
      if (!field->get_type()->has_annotation(
              {"cpp.noncopyable", "cpp2.noncopyable"})) {
        return true;
      }
      switch (gen::cpp::find_ref_type(*field)) {
        case gen::cpp::reference_type::shared_const:
        case gen::cpp::reference_type::shared_mutable: {
          return true;
        }
        case gen::cpp::reference_type::boxed_intern:
        case gen::cpp::reference_type::boxed:
        case gen::cpp::reference_type::none:
        case gen::cpp::reference_type::unique:
          break;
      }
      result = true;
      return false;
    });
    return result;
  }
  mstch::node cpp_noncomparable() {
    return struct_->has_annotation({"cpp.noncomparable", "cpp2.noncomparable"});
  }
  mstch::node cpp_trivially_relocatable() {
    return nullptr !=
        struct_->find_structured_annotation_or_null(
            kCppTriviallyRelocatableUri);
  }
  mstch::node cpp_runtime_annotation() {
    return is_runtime_annotation(*struct_);
  }
  mstch::node is_eligible_for_constexpr() {
    return is_eligible_for_constexpr_(struct_) ||
        struct_->has_annotation({"cpp.methods", "cpp2.methods"});
  }
  mstch::node cpp_virtual() {
    return struct_->has_annotation({"cpp.virtual", "cpp2.virtual"});
  }
  mstch::node message() {
    if (!struct_->is_exception()) {
      return {};
    }
    const auto* message_field =
        dynamic_cast<const t_exception&>(*struct_).get_message_field();
    if (!message_field) {
      return {};
    }
    if (!should_mangle_field_storage_name_in_struct(*struct_)) {
      return message_field->name();
    }
    return mangle_field_name(message_field->name());
  }
  mstch::node cpp_allocator() {
    return struct_->get_annotation("cpp.allocator");
  }
  mstch::node cpp_frozen2_exclude() {
    // TODO(dokwon): Fix frozen2 compatibility with adapter.
    return struct_->has_annotation("cpp.frozen2_exclude") ||
        struct_->find_structured_annotation_or_null(kCppFrozen2ExcludeUri) ||
        cpp_context_->resolver().is_directly_adapted(*struct_);
  }
  mstch::node cpp_allocator_via() {
    if (const auto* name =
            struct_->find_annotation_or_null("cpp.allocator_via")) {
      for (const auto& field : struct_->fields()) {
        if (cpp2::get_name(&field) == *name) {
          return mangle_field_name(*name);
        }
      }
      throw std::runtime_error("No cpp.allocator_via field \"" + *name + "\"");
    }
    return std::string();
  }
  mstch::node has_lazy_fields() {
    for (const auto& field : struct_->get_members()) {
      if (cpp2::is_lazy(field)) {
        return true;
      }
    }
    return false;
  }
  mstch::node indexing() { return has_lazy_fields(); }
  mstch::node write_lazy_field_checksum() {
    if (struct_->find_structured_annotation_or_null(
            kCppDisableLazyChecksumUri)) {
      return std::string("false");
    }

    return std::string("true");
  }
  mstch::node has_isset_fields() {
    for (const auto& field : struct_->fields()) {
      if (cpp2::field_has_isset(&field)) {
        return true;
      }
    }
    return false;
  }
  mstch::node isset_fields() {
    std::vector<const t_field*> fields;
    for (const auto& field : struct_->fields()) {
      if (cpp2::field_has_isset(&field)) {
        fields.push_back(&field);
      }
    }
    if (fields.empty()) {
      return mstch::node();
    }
    return make_mstch_fields(fields);
  }
  mstch::node isset_fields_size() {
    std::size_t size = 0;
    for (const auto& field : struct_->fields()) {
      if (cpp2::field_has_isset(&field)) {
        size++;
      }
    }
    return std::to_string(size);
  }
  mstch::node isset_bitset_option() {
    static const std::string kPrefix =
        "apache::thrift::detail::IssetBitsetOption::";
    if (const auto* anno = cpp2::packed_isset(*struct_)) {
      for (const auto& kv : anno->value()->get_map()) {
        if (kv.first->get_string() == "atomic") {
          if (!kv.second->get_bool()) {
            return kPrefix + "Packed";
          }
        }
      }
      return kPrefix + "PackedWithAtomic";
    }
    return kPrefix + "Unpacked";
  }

  mstch::node is_large() {
    // Outline constructors and destructors if the struct has at least one
    // member with a non-trivial destructor (involving at least a branch and a
    // likely deallocation).
    // TODO(ott): Support unions.
    if (struct_->is_exception()) {
      return true;
    }
    for (const auto& field : struct_->fields()) {
      const auto* resolved_typedef = field.type()->get_true_type();
      if (cpp2::is_ref(&field) || resolved_typedef->is_string_or_binary() ||
          resolved_typedef->is_container()) {
        return true;
      }
    }
    return false;
  }
  mstch::node has_fatal_annotations() {
    return get_fatal_annotations(struct_->annotations()).size() > 0;
  }
  mstch::node fatal_annotations() {
    return make_mstch_annotations(
        get_fatal_annotations(struct_->annotations()));
  }
  mstch::node get_legacy_type_id() {
    return std::to_string(struct_->get_type_id());
  }
  mstch::node legacy_api() {
    return ::apache::thrift::compiler::generate_legacy_api(*struct_);
  }
  mstch::node metadata_name() {
    return struct_->program()->name() + "_" + struct_->get_name();
  }

  mstch::node get_num_union_members() {
    if (!struct_->is_union()) {
      throw std::runtime_error("not a union struct");
    }
    return std::to_string(struct_->fields().size());
  }
  mstch::node has_non_optional_and_non_terse_field() {
    const auto& fields = struct_->fields();
    return std::any_of(
        fields.begin(),
        fields.end(),
        [enabled_terse_write =
             has_option("deprecated_terse_writes")](auto& field) {
          return (!enabled_terse_write ||
                  !cpp2::deprecated_terse_writes(&field)) &&
              field.get_req() != t_field::e_req::optional &&
              field.get_req() != t_field::e_req::terse;
        });
  }
  mstch::node has_field_with_runtime_annotation() {
    const auto& fields = struct_->fields();
    return std::any_of(fields.begin(), fields.end(), has_runtime_annotation);
  }

  mstch::node scoped_enum_as_union_type() {
    return struct_->find_structured_annotation_or_null(
        kCppScopedEnumAsUnionTypeUri);
  }

  mstch::node extra_namespace() {
    auto* extra = cpp_context_->resolver().get_extra_namespace(*struct_);
    return extra ? *extra : mstch::node{};
  }

  mstch::node type_tag() {
    return cpp_context_->resolver().get_type_tag(*struct_);
  }

 protected:
  // Computes the alignment of field on the target platform.
  // Returns max alignment if cannot compute the alignment.
  static size_t compute_alignment(
      const t_field* field, std::unordered_map<const t_field*, size_t>& memo) {
    const size_t kMaxAlign = alignof(std::max_align_t);
    auto find = memo.emplace(field, 0);
    auto& ret = find.first->second;
    if (!find.second) {
      return ret;
    }
    if (cpp2::is_ref(field)) {
      return ret = 8;
    }
    if (cpp2::is_custom_type(*field)) {
      return ret = kMaxAlign;
    }

    const t_type* type = field->get_type();
    switch (type->get_type_value()) {
      case t_type::type::t_bool:
      case t_type::type::t_byte:
        return ret = 1;
      case t_type::type::t_i16:
        return ret = 2;
      case t_type::type::t_i32:
      case t_type::type::t_float:
        return ret = 4;
      case t_type::type::t_enum:
        return ret = compute_alignment(
                   *dynamic_cast<const t_enum*>(type->get_true_type()));
      case t_type::type::t_i64:
      case t_type::type::t_double:
      case t_type::type::t_string:
      case t_type::type::t_binary:
      case t_type::type::t_list:
      case t_type::type::t_set:
      case t_type::type::t_map:
        return ret = 8;
      case t_type::type::t_structured: {
        size_t align = 1;
        const t_struct* strct =
            dynamic_cast<const t_struct*>(type->get_true_type());
        assert(strct);
        for (const auto& field_2 : strct->fields()) {
          size_t field_align = compute_alignment(&field_2, memo);
          align = std::max(align, field_align);
          if (align == kMaxAlign) {
            // No need to continue because the struct already has the maximum
            // alignment.
            return ret = align;
          }
        }
        // The __isset member that is generated in the presence of non-required
        // fields doesn't affect the alignment, because, having only bool
        // fields, it has the alignments of 1.
        return ret = align;
      }
      default:
        return ret = kMaxAlign;
    }
  }

  static size_t compute_alignment(const t_enum& e) {
    if (const auto* annot =
            e.find_structured_annotation_or_null(kCppEnumTypeUri)) {
      const auto& type = annot->get_value_from_structured_annotation("type");
      switch (static_cast<EnumUnderlyingType>(type.get_integer())) {
        case EnumUnderlyingType::I8:
        case EnumUnderlyingType::U8:
          return 1;
        case EnumUnderlyingType::I16:
        case EnumUnderlyingType::U16:
          return 2;
        case EnumUnderlyingType::U32:
          return 4;
        default:
          throw std::runtime_error("unknown enum underlying type");
      }
    }
    return 4;
  }

  // Returns the struct members reordered to minimize padding if the
  // @cpp.MinimizePadding annotation is specified.
  const std::vector<const t_field*>& get_members_in_layout_order() {
    if (struct_->fields().size() == fields_in_layout_order_.size()) {
      // Already reordered.
      return fields_in_layout_order_;
    }

    if (!struct_->has_annotation("cpp.minimize_padding") &&
        !struct_->find_structured_annotation_or_null(kCppMinimizePaddingUri)) {
      return fields_in_layout_order_ = struct_->fields().copy();
    }

    // Compute field alignments.
    struct FieldAlign {
      const t_field* field = nullptr;
      size_t align = 0;
    };
    std::vector<FieldAlign> field_alignments;
    field_alignments.reserve(struct_->fields().size());
    std::unordered_map<const t_field*, size_t> memo;
    for (const auto& field : struct_->fields()) {
      size_t align = compute_alignment(&field, memo);
      assert(align);
      field_alignments.push_back(FieldAlign{&field, align});
    }

    // Sort by decreasing alignment using stable sort to avoid unnecessary
    // reordering.
    std::stable_sort(
        field_alignments.begin(),
        field_alignments.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.align > rhs.align; });

    // Construct the reordered field vector.
    fields_in_layout_order_.reserve(struct_->fields().size());
    std::transform(
        field_alignments.begin(),
        field_alignments.end(),
        std::back_inserter(fields_in_layout_order_),
        [](const FieldAlign& fa) { return fa.field; });
    return fields_in_layout_order_;
  }

  mstch::node fields_in_layout_order() {
    return make_mstch_fields(get_members_in_layout_order());
  }

  mstch::node fields_in_key_order() {
    return make_mstch_fields(get_members_in_key_order());
  }

  mstch::node any() {
    return struct_->uri() != "" &&
        !struct_->has_annotation("cpp.detail.no_any");
  }

  mstch::node patch() {
    return !struct_->is_exception() &&
        struct_->program()->inherit_annotation_or_null(
            *struct_, kGeneratePatchUri) != nullptr;
  }

  mstch::node is_trivially_destructible() {
    for (const auto& field : struct_->fields()) {
      const t_type* type = field.get_type()->get_true_type();
      if (cpp2::is_ref(&field) || cpp2::is_custom_type(field) ||
          !type->is_scalar()) {
        return false;
      }
    }
    return true;
  }

  std::shared_ptr<cpp2_generator_context> cpp_context_;

  std::vector<const t_field*> fields_in_layout_order_;
  cpp2::is_eligible_for_constexpr is_eligible_for_constexpr_;
};

class cpp_mstch_field : public mstch_field {
 public:
  cpp_mstch_field(
      const t_field* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context,
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_field(f, ctx, pos, field_context),
        cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"field:name_hash", &cpp_mstch_field::name_hash},
            {"field:index_plus_one", &cpp_mstch_field::index_plus_one},
            {"field:ordinal", &cpp_mstch_field::ordinal},
            {"field:has_isset?", &cpp_mstch_field::has_isset},
            {"field:isset_index", &cpp_mstch_field::isset_index},
            {"field:cpp_name", &cpp_mstch_field::cpp_name},
            {"field:cpp_type", &cpp_mstch_field::cpp_type},
            {"field:cpp_storage_name", &cpp_mstch_field::cpp_storage_name},
            {"field:cpp_storage_type", &cpp_mstch_field::cpp_storage_type},
            {"field:has_deprecated_accessors?",
             &cpp_mstch_field::has_deprecated_accessors},
            {"field:serialization_next_field_key",
             &cpp_mstch_field::serialization_next_field_key},
            {"field:serialization_prev_field_key",
             &cpp_mstch_field::serialization_prev_field_key},
            {"field:serialization_next_field_type",
             &cpp_mstch_field::serialization_next_field_type},
            {"field:non_opt_cpp_ref?", &cpp_mstch_field::non_opt_cpp_ref},
            {"field:opt_cpp_ref?", &cpp_mstch_field::opt_cpp_ref},
            {"field:cpp_ref?", &cpp_mstch_field::cpp_ref},
            {"field:cpp_ref_unique?", &cpp_mstch_field::cpp_ref_unique},
            {"field:cpp_ref_shared?", &cpp_mstch_field::cpp_ref_shared},
            {"field:cpp_ref_shared_const?",
             &cpp_mstch_field::cpp_ref_shared_const},
            {"field:cpp_ref_not_boxed?", &cpp_mstch_field::cpp_ref_not_boxed},
            {"field:cpp_adapter", &cpp_mstch_field::cpp_adapter},
            {"field:cpp_first_adapter", &cpp_mstch_field::cpp_first_adapter},
            {"field:cpp_exactly_one_adapter?",
             &cpp_mstch_field::cpp_exactly_one_adapter},
            {"field:cpp_field_interceptor",
             &cpp_mstch_field::cpp_field_interceptor},
            {"field:cpp_accessor_attribute",
             &cpp_mstch_field::cpp_accessor_attribute},
            {"field:zero_copy_arg", &cpp_mstch_field::zero_copy_arg},
            {"field:cpp_noncopyable?", &cpp_mstch_field::cpp_noncopyable},
            {"field:enum_has_value", &cpp_mstch_field::enum_has_value},
            {"field:deprecated_terse_writes?",
             &cpp_mstch_field::deprecated_terse_writes},
            {"field:fatal_annotations?",
             &cpp_mstch_field::has_fatal_annotations},
            {"field:fatal_annotations", &cpp_mstch_field::fatal_annotations},
            {"field:fatal_required_qualifier",
             &cpp_mstch_field::fatal_required_qualifier},
            {"field:visibility", &cpp_mstch_field::visibility},
            {"field:metadata_name", &cpp_mstch_field::metadata_name},
            {"field:lazy?", &cpp_mstch_field::lazy},
            {"field:lazy_ref?", &cpp_mstch_field::lazy_ref},
            {"field:boxed_ref?", &cpp_mstch_field::boxed_ref},
            {"field:intern_boxed_ref?", &cpp_mstch_field::intern_boxed_ref},
            {"field:use_field_ref?", &cpp_mstch_field::use_field_ref},
            {"field:field_ref_type", &cpp_mstch_field::field_ref_type},
            {"field:transitively_refers_to_unique?",
             &cpp_mstch_field::transitively_refers_to_unique},
            {"field:eligible_for_storage_name_mangling?",
             &cpp_mstch_field::eligible_for_storage_name_mangling},
            {"field:type_tag", &cpp_mstch_field::type_tag},
            {"field:tablebased_qualifier",
             &cpp_mstch_field::tablebased_qualifier},
            {"field:raw_binary?", &cpp_mstch_field::raw_binary},
            {"field:raw_string_or_binary?",
             &cpp_mstch_field::raw_string_or_binary},
            {"field:cpp_has_runtime_annotation?",
             &cpp_mstch_field::cpp_has_runtime_annotation},
            {"field:use_op_encode?", &cpp_mstch_field::use_op_encode},
        });
    register_has_option("field:deprecated_clear?", "deprecated_clear");
  }
  mstch::node name_hash() {
    return "__fbthrift_hash_" + cpp2::sha256_hex(field_->get_name());
  }
  mstch::node index_plus_one() { return std::to_string(pos_.index + 1); }
  mstch::node ordinal() { return index_plus_one(); }
  mstch::node isset_index() {
    assert(field_context_);
    return field_context_->isset_index;
  }
  mstch::node cpp_name() { return cpp2::get_name(field_); }
  mstch::node cpp_type() {
    assert(field_context_->strct);
    return cpp_context_->resolver().get_native_type(
        *field_, *field_context_->strct);
  }
  mstch::node cpp_storage_name() {
    if (!is_eligible_for_storage_name_mangling()) {
      return cpp2::get_name(field_);
    }

    return mangle_field_name(cpp2::get_name(field_));
  }
  mstch::node cpp_storage_type() {
    assert(field_context_->strct);
    return cpp_context_->resolver().get_storage_type(
        *field_, *field_context_->strct);
  }
  mstch::node eligible_for_storage_name_mangling() {
    return is_eligible_for_storage_name_mangling();
  }
  mstch::node has_deprecated_accessors() {
    return !cpp2::is_explicit_ref(field_) && !cpp2::is_lazy(field_) &&
        !gen::cpp::type_resolver::find_first_adapter(*field_) &&
        !gen::cpp::type_resolver::find_field_interceptor(*field_) &&
        !has_option("no_getters_setters");
  }
  mstch::node cpp_ref() { return cpp2::is_explicit_ref(field_); }
  mstch::node opt_cpp_ref() {
    return cpp2::is_explicit_ref(field_) &&
        field_->get_req() == t_field::e_req::optional;
  }
  mstch::node non_opt_cpp_ref() {
    return cpp2::is_explicit_ref(field_) &&
        field_->get_req() != t_field::e_req::optional;
  }
  mstch::node lazy() { return cpp2::is_lazy(field_); }
  mstch::node lazy_ref() { return cpp2::is_lazy_ref(field_); }
  mstch::node boxed_ref() {
    return gen::cpp::find_ref_type(*field_) == gen::cpp::reference_type::boxed;
  }
  mstch::node intern_boxed_ref() {
    return gen::cpp::find_ref_type(*field_) ==
        gen::cpp::reference_type::boxed_intern;
  }
  mstch::node use_field_ref() {
    auto ref_type = gen::cpp::find_ref_type(*field_);
    return ref_type == gen::cpp::reference_type::none ||
        ref_type == gen::cpp::reference_type::boxed ||
        ref_type == gen::cpp::reference_type::boxed_intern;
  }
  mstch::node field_ref_type() {
    return cpp_context_->resolver().get_reference_type(*field_);
  }

  mstch::node tablebased_qualifier() {
    const std::string enum_type = "::apache::thrift::detail::FieldQualifier::";
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

  mstch::node transitively_refers_to_unique() {
    return cpp2::field_transitively_refers_to_unique(field_);
  }
  mstch::node cpp_ref_unique() { return cpp2::is_unique_ref(field_); }
  mstch::node cpp_ref_shared() {
    return gen::cpp::find_ref_type(*field_) ==
        gen::cpp::reference_type::shared_mutable;
  }
  mstch::node cpp_ref_shared_const() {
    return gen::cpp::find_ref_type(*field_) ==
        gen::cpp::reference_type::shared_const;
  }
  mstch::node cpp_ref_not_boxed() {
    auto ref_type = gen::cpp::find_ref_type(*field_);
    return ref_type != gen::cpp::reference_type::none &&
        ref_type != gen::cpp::reference_type::boxed &&
        ref_type != gen::cpp::reference_type::boxed_intern;
  }
  mstch::node cpp_first_adapter() {
    if (const std::string* adapter =
            gen::cpp::type_resolver::find_first_adapter(*field_)) {
      return *adapter;
    }
    return {};
  }
  mstch::node cpp_exactly_one_adapter() {
    bool hasFieldAdapter =
        gen::cpp::type_resolver::find_structured_adapter_annotation(*field_);
    bool hasTypeAdapter =
        gen::cpp::type_resolver::find_first_adapter(*field_->type());
    return hasFieldAdapter != hasTypeAdapter;
  }
  mstch::node cpp_field_interceptor() {
    if (const std::string* interceptor =
            gen::cpp::type_resolver::find_field_interceptor(*field_)) {
      return *interceptor;
    }
    return {};
  }

  // The field accessor is inlined and erased by default, unless 'noinline' is
  // specified in FieldInterceptor.
  mstch::node cpp_accessor_attribute() {
    if (const t_const* annotation = field_->find_structured_annotation_or_null(
            kCppFieldInterceptorUri)) {
      if (const auto* val =
              annotation->get_value_from_structured_annotation_or_null(
                  "noinline")) {
        return std::string("FOLLY_NOINLINE");
      }
    }
    return std::string("FOLLY_ERASE");
  }

  mstch::node cpp_adapter() {
    // Only find a structured adapter on the field.
    if (const std::string* adapter =
            gen::cpp::type_resolver::find_structured_adapter_annotation(
                *field_)) {
      return *adapter;
    }
    return {};
  }
  mstch::node cpp_noncopyable() {
    return field_->get_type()->has_annotation(
        {"cpp.noncopyable", "cpp2.noncopyable"});
  }
  mstch::node enum_has_value() {
    if (auto enm = dynamic_cast<const t_enum*>(field_->get_type())) {
      const auto* const_value = field_->get_value();
      using cv = t_const_value::t_const_value_kind;
      if (const_value->kind() == cv::CV_INTEGER) {
        if (auto* enum_value = enm->find_value(const_value->get_integer())) {
          return context_.enum_value_factory->make_mstch_object(
              enum_value, context_, pos_);
        }
      }
    }
    return mstch::node();
  }
  mstch::node serialization_prev_field_key() {
    assert(field_context_ && field_context_->serialization_prev);
    return field_context_->serialization_prev->get_key();
  }
  mstch::node serialization_next_field_key() {
    assert(field_context_ && field_context_->serialization_next);
    return field_context_->serialization_next->get_key();
  }
  mstch::node serialization_next_field_type() {
    assert(field_context_ && field_context_->serialization_next);
    return field_context_->serialization_next
        ? context_.type_factory->make_mstch_object(
              field_context_->serialization_next->get_type(), context_, pos_)
        : mstch::node("");
  }
  mstch::node deprecated_terse_writes() {
    return has_option("deprecated_terse_writes") &&
        cpp2::deprecated_terse_writes(field_);
  }
  mstch::node zero_copy_arg() {
    switch (field_->get_type()->get_type_value()) {
      case t_type::type::t_binary:
      case t_type::type::t_structured:
        return std::string("true");
      default:
        return std::string("false");
    }
  }
  mstch::node has_fatal_annotations() {
    return get_fatal_annotations(field_->annotations()).size() > 0;
  }
  mstch::node has_isset() { return cpp2::field_has_isset(field_); }
  mstch::node fatal_annotations() {
    return make_mstch_annotations(get_fatal_annotations(field_->annotations()));
  }
  mstch::node fatal_required_qualifier() {
    switch (field_->get_req()) {
      case t_field::e_req::required:
        return std::string("required");
      case t_field::e_req::optional:
        return std::string("optional");
      case t_field::e_req::opt_in_req_out:
        return std::string("required_of_writer");
      case t_field::e_req::terse:
        return std::string("terse");
    }
    throw std::runtime_error("unknown required qualifier");
  }

  mstch::node visibility() {
    return std::string(is_private() ? "private" : "public");
  }

  mstch::node metadata_name() {
    auto key = field_->get_key();
    auto suffix = key >= 0 ? std::to_string(key) : "_" + std::to_string(-key);
    return field_->get_name() + "_" + suffix;
  }

  mstch::node type_tag() {
    return cpp_context_->resolver().get_type_tag(*field_);
  }

  mstch::node raw_binary() {
    return field_->type()->get_true_type()->is_binary() &&
        !gen::cpp::type_resolver::find_first_adapter(*field_);
  }

  mstch::node raw_string_or_binary() {
    return field_->type()->get_true_type()->is_string_or_binary() &&
        !gen::cpp::type_resolver::find_first_adapter(*field_);
  }

  mstch::node cpp_has_runtime_annotation() {
    return has_runtime_annotation(*field_);
  }

  mstch::node use_op_encode() {
    assert(field_context_->strct);
    return needs_op_encode(*field_, *field_context_->strct);
  }

 private:
  bool is_private() const {
    auto req = field_->get_req();
    bool isPrivate = true;
    if (cpp2::is_lazy(field_)) {
      // Lazy field has to be private.
    } else if (cpp2::is_ref(field_)) {
      // cpp.ref field is always private
    } else if (req == t_field::e_req::required) {
      isPrivate = !has_option("deprecated_public_required_fields");
    }
    return isPrivate;
  }

  bool is_eligible_for_storage_name_mangling() const {
    const auto* strct = field_context_->strct;

    if (strct->is_union()) {
      return false;
    }

    if (!should_mangle_field_storage_name_in_struct(*strct)) {
      return false;
    }

    return is_private();
  }

  std::shared_ptr<cpp2_generator_context> cpp_context_;
};

class cpp_mstch_enum : public mstch_enum {
 public:
  cpp_mstch_enum(
      const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum(e, ctx, pos) {
    register_methods(
        this,
        {
            {"enum:empty?", &cpp_mstch_enum::is_empty},
            {"enum:size", &cpp_mstch_enum::size},
            {"enum:min", &cpp_mstch_enum::min},
            {"enum:max", &cpp_mstch_enum::max},
            {"enum:cpp_is_unscoped", &cpp_mstch_enum::cpp_is_unscoped},
            {"enum:cpp_name", &cpp_mstch_enum::cpp_name},
            {"enum:cpp_enum_type", &cpp_mstch_enum::cpp_enum_type},
            {"enum:cpp_declare_bitwise_ops",
             &cpp_mstch_enum::cpp_declare_bitwise_ops},
            {"enum:has_zero", &cpp_mstch_enum::has_zero},
            {"enum:fatal_annotations?", &cpp_mstch_enum::has_fatal_annotations},
            {"enum:fatal_annotations", &cpp_mstch_enum::fatal_annotations},
            {"enum:legacy_type_id", &cpp_mstch_enum::get_legacy_type_id},
            {"enum:legacy_api?", &cpp_mstch_enum::legacy_api},
        });
  }
  mstch::node is_empty() { return enum_->get_enum_values().empty(); }
  mstch::node size() { return std::to_string(enum_->get_enum_values().size()); }
  mstch::node min() {
    if (!enum_->get_enum_values().empty()) {
      auto e_min = std::min_element(
          enum_->get_enum_values().begin(),
          enum_->get_enum_values().end(),
          [](t_enum_value* a, t_enum_value* b) {
            return a->get_value() < b->get_value();
          });
      return cpp2::get_name(*e_min);
    }
    return mstch::node();
  }
  mstch::node max() {
    if (!enum_->get_enum_values().empty()) {
      auto e_max = std::max_element(
          enum_->get_enum_values().begin(),
          enum_->get_enum_values().end(),
          [](t_enum_value* a, t_enum_value* b) {
            return a->get_value() < b->get_value();
          });
      return cpp2::get_name(*e_max);
    }
    return mstch::node();
  }
  mstch::node cpp_is_unscoped() { return cpp_is_unscoped_(); }
  mstch::node cpp_name() { return cpp2::get_name(enum_); }
  mstch::node cpp_enum_type() { return fmt::to_string(cpp_enum_type(*enum_)); }
  mstch::node cpp_declare_bitwise_ops() {
    return enum_->find_annotation_or_null(
               {"cpp.declare_bitwise_ops", "cpp2.declare_bitwise_ops"}) ||
        enum_->find_structured_annotation_or_null(kBitmaskEnumUri);
  }
  mstch::node has_zero() {
    auto* enum_value = enum_->find_value(0);
    if (enum_value != nullptr) {
      return context_.enum_value_factory->make_mstch_object(
          enum_value, context_, pos_);
    }
    return mstch::node();
  }
  mstch::node has_fatal_annotations() {
    return get_fatal_annotations(enum_->annotations()).size() > 0;
  }
  mstch::node fatal_annotations() {
    return make_mstch_annotations(get_fatal_annotations(enum_->annotations()));
  }
  mstch::node get_legacy_type_id() {
    return std::to_string(enum_->get_type_id());
  }
  mstch::node legacy_api() {
    return ::apache::thrift::compiler::generate_legacy_api(*enum_);
  }

 private:
  const std::string& cpp_is_unscoped_() {
    return enum_->get_annotation(
        {"cpp2.deprecated_enum_unscoped", "cpp.deprecated_enum_unscoped"});
  }

  std::string_view cpp_enum_type(const t_enum& e) {
    if (const auto* annot =
            e.find_structured_annotation_or_null(kCppEnumTypeUri)) {
      const auto& type = annot->get_value_from_structured_annotation("type");
      switch (static_cast<EnumUnderlyingType>(type.get_integer())) {
        case EnumUnderlyingType::I8:
          return "::std::int8_t";
        case EnumUnderlyingType::U8:
          return "::std::uint8_t";
        case EnumUnderlyingType::I16:
          return "::std::int16_t";
        case EnumUnderlyingType::U16:
          return "::std::uint16_t";
        case EnumUnderlyingType::U32:
          return "::std::uint32_t";
        default:
          throw std::runtime_error("unknown enum underlying type");
      }
    }
    // TODO(dokwon): Deprecate unstructured 'cpp.enum_type' annotation.
    static std::string kInt = "int";
    if (const std::string& type = e.get_annotation(
            {"cpp.enum_type", "cpp2.enum_type"},
            cpp_is_unscoped_().empty() ? nullptr : &kInt);
        !type.empty()) {
      return type;
    }
    return "";
  }
};

class cpp_mstch_enum_value : public mstch_enum_value {
 public:
  cpp_mstch_enum_value(
      const t_enum_value* ev, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum_value(ev, ctx, pos) {
    register_methods(
        this,
        {
            {"enum_value:name_hash", &cpp_mstch_enum_value::name_hash},
            {"enum_value:cpp_name", &cpp_mstch_enum_value::cpp_name},
            {"enum_value:fatal_annotations?",
             &cpp_mstch_enum_value::has_fatal_annotations},
            {"enum_value:fatal_annotations",
             &cpp_mstch_enum_value::fatal_annotations},
        });
  }
  mstch::node name_hash() {
    return "__fbthrift_hash_" + cpp2::sha256_hex(enum_value_->get_name());
  }
  mstch::node cpp_name() { return cpp2::get_name(enum_value_); }
  mstch::node has_fatal_annotations() {
    return get_fatal_annotations(enum_value_->annotations()).size() > 0;
  }
  mstch::node fatal_annotations() {
    return make_mstch_annotations(
        get_fatal_annotations(enum_value_->annotations()));
  }
};

class cpp_mstch_const : public mstch_const {
 public:
  cpp_mstch_const(
      const t_const* c,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      const t_field* field,
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_const(c, ctx, pos, current_const, expected_type, field),
        cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"constant:enum_value", &cpp_mstch_const::enum_value},
            {"constant:cpp_name", &cpp_mstch_const::cpp_name},
            {"constant:cpp_adapter", &cpp_mstch_const::cpp_adapter},
            {"constant:cpp_type", &cpp_mstch_const::cpp_type},
            {"constant:cpp_runtime_annotation?",
             &cpp_mstch_const::cpp_runtime_annotation},
            {"constant:uri", &cpp_mstch_const::uri},
            {"constant:has_extra_arg?", &cpp_mstch_const::has_extra_arg},
            {"constant:extra_arg", &cpp_mstch_const::extra_arg},
            {"constant:extra_arg_type", &cpp_mstch_const::extra_arg_type},
            {"constant:outline_init?", &cpp_mstch_const::outline_init},
        });
  }
  mstch::node enum_value() {
    if (const_->type()->is_enum()) {
      const auto* enm = static_cast<const t_enum*>(const_->type());
      const auto* enum_val = enm->find_value(const_->value()->get_integer());
      if (enum_val) {
        return enum_val->get_name();
      } else {
        return std::to_string(const_->value()->get_integer());
      }
    }
    return mstch::node();
  }
  mstch::node cpp_name() { return cpp2::get_name(field_); }
  mstch::node cpp_runtime_annotation() {
    return is_runtime_annotation(*const_->type());
  }
  mstch::node cpp_adapter() {
    if (const std::string* adapter =
            cpp_context_->resolver().find_structured_adapter_annotation(
                *const_)) {
      return *adapter;
    }
    return {};
  }
  mstch::node cpp_type() {
    return cpp_context_->resolver().get_native_type(*const_);
  }
  mstch::node uri() { return const_->uri(); }
  mstch::node has_extra_arg() {
    return cpp2::get_transitive_annotation_of_adapter_or_null(*const_) !=
        nullptr;
  }
  mstch::node extra_arg() {
    auto anno = cpp2::get_transitive_annotation_of_adapter_or_null(*const_);
    return std::shared_ptr<mstch_base>(std::make_shared<mstch_const_value>(
        anno->value(), context_, pos_, anno, &*anno->type()));
  }
  mstch::node extra_arg_type() {
    auto anno = cpp2::get_transitive_annotation_of_adapter_or_null(*const_);
    return std::shared_ptr<mstch_base>(std::make_shared<cpp_mstch_type>(
        &*anno->type(), context_, pos_, cpp_context_));
  }
  mstch::node outline_init() {
    return resolves_to_container_or_struct(const_->type()->get_true_type()) ||
        cpp_context_->resolver().find_structured_adapter_annotation(*const_) ||
        cpp_context_->resolver().find_first_adapter(*const_->type());
  }

 private:
  std::shared_ptr<cpp2_generator_context> cpp_context_;
};

class cpp_mstch_const_value : public mstch_const_value {
 public:
  cpp_mstch_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type) {
    register_methods(
        this,
        {
            {"value:default_construct?",
             &cpp_mstch_const_value::default_construct},
            {"value:enum_value_cpp_name",
             &cpp_mstch_const_value::enum_value_cpp_name},
        });
  }

 private:
  mstch::node default_construct() {
    return boost::get<bool>(is_empty_container()) &&
        !gen::cpp::type_resolver::find_first_adapter(
               *const_value_->get_owner()->type());
  }

  mstch::node enum_value_cpp_name() {
    // reference: mstch_const_value::enum_value_name
    if (type_ == cv::CV_INTEGER && const_value_->is_enum() &&
        const_value_->get_enum_value() != nullptr) {
      return cpp2::get_name(const_value_->get_enum_value());
    }
    return mstch::node();
  }

  bool same_type_as_expected() const override {
    return const_value_->get_owner() &&
        same_types(expected_type_, const_value_->get_owner()->type());
  }
};

class cpp_mstch_deprecated_annotation : public mstch_deprecated_annotation {
 public:
  cpp_mstch_deprecated_annotation(
      const t_annotation* a, mstch_context& ctx, mstch_element_position pos)
      : mstch_deprecated_annotation(a, ctx, pos) {
    register_methods(
        this,
        {
            {"annotation:safe_key", &cpp_mstch_deprecated_annotation::safe_key},
            {"annotation:fatal_string",
             &cpp_mstch_deprecated_annotation::fatal_string},
        });
  }
  mstch::node safe_key() { return get_fatal_string_short_id(key_); }
  mstch::node fatal_string() { return render_fatal_string(val_.value); }
};

void t_mstch_cpp2_generator::generate_program() {
  const auto* program = get_program();
  set_mstch_factories();

  if (has_option("any")) {
    generate_sinit(program);
  }
  if (has_option("reflection")) {
    generate_reflection(program);
  }
  generate_structs(program);
  generate_constants(program);
  if (has_option("single_file_service")) {
    generate_inline_services(program->services());
  } else {
    generate_out_of_line_services(program->services());
  }
  generate_metadata(program);
  generate_visitation(program);
}

void t_mstch_cpp2_generator::set_mstch_factories() {
  mstch_context_.add<cpp_mstch_program>();
  mstch_context_.add<cpp_mstch_service>();
  mstch_context_.add<cpp_mstch_interaction>();
  mstch_context_.add<cpp_mstch_function>(cpp_context_);
  mstch_context_.add<cpp_mstch_type>(cpp_context_);
  mstch_context_.add<cpp_mstch_typedef>(cpp_context_);
  mstch_context_.add<cpp_mstch_struct>(cpp_context_);
  mstch_context_.add<cpp_mstch_field>(cpp_context_);
  mstch_context_.add<cpp_mstch_enum>();
  mstch_context_.add<cpp_mstch_enum_value>();
  mstch_context_.add<cpp_mstch_const>(cpp_context_);
  mstch_context_.add<cpp_mstch_const_value>();
  mstch_context_.add<cpp_mstch_deprecated_annotation>();
}

void t_mstch_cpp2_generator::generate_constants(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_constants.h", name + "_constants.h");
  render_to_file(prog, "module_constants.cpp", name + "_constants.cpp");
}

void t_mstch_cpp2_generator::generate_metadata(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_metadata.h", name + "_metadata.h");
  if (!has_option("no_metadata")) {
    render_to_file(prog, "module_metadata.cpp", name + "_metadata.cpp");
  }
}

void t_mstch_cpp2_generator::generate_sinit(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_sinit.cpp", name + "_sinit.cpp");
}

void t_mstch_cpp2_generator::generate_reflection(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  // Combo include: all
  render_to_file(prog, "module_fatal_all.h", name + "_fatal_all.h");
  // Combo include: types
  render_to_file(prog, "module_fatal_types.h", name + "_fatal_types.h");
  // Unique Compile-time Strings, Metadata tags and Metadata registration
  render_to_file(prog, "module_fatal.h", name + "_fatal.h");

  render_to_file(prog, "module_fatal_enum.h", name + "_fatal_enum.h");
  render_to_file(prog, "module_fatal_union.h", name + "_fatal_union.h");
  render_to_file(prog, "module_fatal_struct.h", name + "_fatal_struct.h");
  render_to_file(prog, "module_fatal_constant.h", name + "_fatal_constant.h");
  render_to_file(prog, "module_fatal_service.h", name + "_fatal_service.h");
}

void t_mstch_cpp2_generator::generate_visitation(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_visitation.h", name + "_visitation.h");
  render_to_file(prog, "module_for_each_field.h", name + "_for_each_field.h");
  render_to_file(prog, "module_visit_union.h", name + "_visit_union.h");
  render_to_file(
      prog,
      "module_visit_by_thrift_field_metadata.h",
      name + "_visit_by_thrift_field_metadata.h");
}

void t_mstch_cpp2_generator::generate_structs(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_data.h", name + "_data.h");
  render_to_file(prog, "module_data.cpp", name + "_data.cpp");
  render_to_file(prog, "module_types.h", name + "_types.h");
  render_to_file(prog, "module_types_fwd.h", name + "_types_fwd.h");
  render_to_file(prog, "module_types.tcc", name + "_types.tcc");

  if (int split_count = get_split_count(options())) {
    auto digit = std::to_string(split_count - 1).size();
    auto shards = cpp2::lpt_split(
        program->structured_definitions(), split_count, [](auto t) {
          return t->fields().size();
        });
    for (int split_id = 0; split_id < split_count; ++split_id) {
      auto s = std::to_string(split_id);
      s = std::string(digit - s.size(), '0') + s;
      auto split_program = std::make_shared<cpp_mstch_program>(
          program,
          mstch_context_,
          mstch_element_position(),
          split_id,
          shards.at(split_id));
      render_to_file(
          std::shared_ptr<mstch_base>(split_program),
          "module_types.cpp",
          name + "_types." + s + ".split.cpp");
    }
  } else {
    render_to_file(prog, "module_types.cpp", name + "_types.cpp");
  }

  render_to_file(
      prog,
      "module_types_custom_protocol.h",
      name + "_types_custom_protocol.h");
  if (has_option("frozen2")) {
    render_to_file(prog, "module_layouts.h", name + "_layouts.h");
    render_to_file(prog, "module_layouts.cpp", name + "_layouts.cpp");
  }
}

void t_mstch_cpp2_generator::generate_out_of_line_service(
    const t_service* service) {
  const auto& name = service->get_name();
  auto mstch_service =
      make_mstch_service_cached(get_program(), service, mstch_context_);

  render_to_file(mstch_service, "ServiceAsyncClient.h", name + "AsyncClient.h");
  render_to_file(mstch_service, "service.cpp", name + ".cpp");
  render_to_file(mstch_service, "service.h", name + ".h");
  render_to_file(mstch_service, "service.tcc", name + ".tcc");
  render_to_file(
      mstch_service, "types_custom_protocol.h", name + "_custom_protocol.h");

  auto iter = client_name_to_split_count_.find(name);
  if (iter != client_name_to_split_count_.end()) {
    auto split_count = iter->second;
    auto digit = std::to_string(split_count - 1).size();
    for (int split_id = 0; split_id < split_count; ++split_id) {
      auto s = std::to_string(split_id);
      s = std::string(digit - s.size(), '0') + s;
      auto split_service = std::make_shared<cpp_mstch_service>(
          service,
          mstch_context_,
          mstch_element_position(),
          split_id,
          split_count);
      render_to_file(
          std::shared_ptr<mstch_base>(split_service),
          "ServiceAsyncClient.cpp",
          name + "." + s + ".async_client_split.cpp");
    }
  } else {
    render_to_file(
        mstch_service, "ServiceAsyncClient.cpp", name + "AsyncClient.cpp");
  }

  std::vector<std::array<std::string, 3>> protocols = {
      {{"binary", "BinaryProtocol", "T_BINARY_PROTOCOL"}},
      {{"compact", "CompactProtocol", "T_COMPACT_PROTOCOL"}},
  };
  for (const auto& protocol : protocols) {
    render_to_file(
        mstch_service,
        "service_processmap_protocol.cpp",
        name + "_processmap_" + protocol.at(0) + ".cpp");
  }
}

void t_mstch_cpp2_generator::generate_out_of_line_services(
    const std::vector<t_service*>& services) {
  for (const auto* service : services) {
    generate_out_of_line_service(service);
  }

  mstch::array mstch_services;
  mstch_services.reserve(services.size());
  for (const t_service* service : services) {
    mstch_services.push_back(
        make_mstch_service_cached(get_program(), service, mstch_context_));
  }
  mstch::map context{
      {"services", std::move(mstch_services)},
  };
  const auto& module_name = get_program()->name();
  render_to_file(
      context, "module_handlers_out_of_line.h", module_name + "_handlers.h");
  render_to_file(
      context, "module_clients_out_of_line.h", module_name + "_clients.h");
}

void t_mstch_cpp2_generator::generate_inline_services(
    const std::vector<t_service*>& services) {
  mstch::array mstch_services;
  mstch_services.reserve(services.size());
  for (const t_service* service : services) {
    mstch_services.push_back(
        make_mstch_service_cached(get_program(), service, mstch_context_));
  }
  auto any_service_has_any_function = [&](auto&& predicate) -> bool {
    return std::any_of(
        services.cbegin(), services.cend(), [&](const t_service* service) {
          auto funcs = service->functions();
          return std::any_of(
              funcs.cbegin(), funcs.cend(), [&](auto const& func) {
                return predicate(func);
              });
        });
  };
  auto any_service = [&](auto&& predicate) -> bool {
    return std::any_of(
        services.cbegin(), services.cend(), [&](const t_service* service) {
          return predicate(service);
        });
  };

  mstch::map context = {
      {"program", cached_program(get_program())},
      {"any_sinks?",
       any_service_has_any_function(std::mem_fn(&t_function::sink))},
      {"any_streams?",
       any_service_has_any_function(std::mem_fn(&t_function::stream))},
      {"any_interactions?",
       any_service_has_any_function([](const t_function& func) {
         return func.is_interaction_constructor() || func.interaction();
       })},
      {"any_interceptors?",
       any_service_has_any_function([](const t_function& func) {
         return func.find_structured_annotation_or_null(
             kCppGenerateTypedInterceptor);
       }) ||
           any_service([](const t_service* service) {
             return service->find_structured_annotation_or_null(
                 kCppGenerateTypedInterceptor);
           })},
      {"services", std::move(mstch_services)},
  };
  const auto& module_name = get_program()->name();
  render_to_file(context, "module_clients.h", module_name + "_clients.h");
  render_to_file(context, "module_clients.cpp", module_name + "_clients.cpp");
  render_to_file(
      context, "module_handlers-inl.h", module_name + "_handlers-inl.h");
  render_to_file(context, "module_handlers.h", module_name + "_handlers.h");
  render_to_file(context, "module_handlers.cpp", module_name + "_handlers.cpp");
}

std::string t_mstch_cpp2_generator::get_cpp2_namespace(
    const t_program* program) {
  return cpp2::get_gen_namespace(*program);
}

/* static */ std::string t_mstch_cpp2_generator::get_cpp2_unprefixed_namespace(
    const t_program* program) {
  return cpp2::get_gen_unprefixed_namespace(*program);
}

mstch::array t_mstch_cpp2_generator::get_namespace_array(
    const t_program* program) {
  const auto v = cpp2::get_gen_namespace_components(*program);
  mstch::array a;
  for (auto it = v.begin(); it != v.end(); ++it) {
    mstch::map m;
    m.emplace("namespace:name", *it);
    a.push_back(m);
  }
  for (auto itr = a.begin(); itr != a.end(); ++itr) {
    boost::get<mstch::map>(*itr).emplace("first?", itr == a.begin());
    boost::get<mstch::map>(*itr).emplace("last?", std::next(itr) == a.end());
  }
  return a;
}

mstch::array t_mstch_cpp2_generator::cpp_includes(const t_program* program) {
  mstch::array a;
  if (program->language_includes().count("cpp")) {
    for (auto include : program->language_includes().at("cpp")) {
      mstch::map cpp_include;
      if (include.at(0) != '<') {
        include = fmt::format("\"{}\"", include);
      }
      cpp_include.emplace("cpp_include", std::move(include));
      a.push_back(std::move(cpp_include));
    }
  }
  return a;
}

mstch::node t_mstch_cpp2_generator::include_prefix(
    const t_program* program, std::map<std::string, std::string>& options) {
  auto prefix = program->include_prefix();
  auto include_prefix = options["include_prefix"];
  auto out_dir_base = get_out_dir_base(options);
  if (prefix.empty()) {
    if (include_prefix.empty()) {
      return prefix;
    } else {
      return include_prefix + "/" + out_dir_base + "/";
    }
  }
  if (boost::filesystem::path(prefix).has_root_directory()) {
    return include_prefix + "/" + out_dir_base + "/";
  }
  return prefix + out_dir_base + "/";
}

static auto split(const std::string& s, char delimiter) {
  std::vector<std::string> ret;
  boost::algorithm::split(ret, s, [&](char c) { return c == delimiter; });
  return ret;
}

std::unordered_map<std::string, int>
t_mstch_cpp2_generator::get_client_name_to_split_count() const {
  auto client_cpp_splits = get_option("client_cpp_splits");
  if (!client_cpp_splits) {
    return {};
  }

  auto map = *client_cpp_splits;
  if (map.size() < 2 || map[0] != '{' || *map.rbegin() != '}') {
    throw std::runtime_error("Invalid client_cpp_splits value: `" + map + "`");
  }
  map = map.substr(1, map.size() - 2);
  if (map.empty()) {
    return {};
  }
  std::unordered_map<std::string, int> ret;
  for (auto kv : split(map, ',')) {
    auto a = split(kv, ':');
    if (a.size() != 2) {
      throw std::runtime_error(
          "Invalid pair `" + kv + "` in client_cpp_splits value: `" + map +
          "`");
    }
    ret[a[0]] = checked_stoi(
        a[1],
        "Invalid pair `" + kv + "` in client_cpp_splits value: `" + map + "`");
  }
  return ret;
}

// Make sure there is no incompatible annotation.
void validate_struct_annotations(
    diagnostic_context& ctx,
    const t_structured& s,
    const std::map<std::string, std::string>& options) {
  if (cpp2::packed_isset(s)) {
    if (options.count("tablebased") != 0) {
      ctx.report(
          s,
          "tablebased-isset-bitpacking-rule",
          diagnostic_level::error,
          "Tablebased serialization is incompatible with isset bitpacking for struct `{}`",
          s.get_name());
    }
  }

  for (const auto& field : s.fields()) {
    if (cpp2::is_mixin(field)) {
      // Mixins cannot be refs
      if (cpp2::is_explicit_ref(&field)) {
        ctx.report(
            field,
            "mixin-ref-rule",
            diagnostic_level::error,
            "Mixin field `{}` can not be a ref in cpp.",
            field.name());
      }
    }
  }
}

class validate_splits {
 public:
  explicit validate_splits(
      int split_count,
      const std::unordered_map<std::string, int>& client_name_to_split_count)
      : split_count_(split_count),
        client_name_to_split_count_(client_name_to_split_count) {}

  void operator()(diagnostic_context& ctx, const t_program& program) {
    validate_type_cpp_splits(
        program.structured_definitions().size() + program.enums().size(),
        ctx,
        program);
    validate_client_cpp_splits(program.services(), ctx);
  }

 private:
  int split_count_ = 0;
  std::unordered_map<std::string, int> client_name_to_split_count_;

  void validate_type_cpp_splits(
      const int32_t object_count,
      diagnostic_context& ctx,
      const t_program& program) {
    if (split_count_ > object_count) {
      ctx.report(
          program,
          "more-splits-than-objects-rule",
          diagnostic_level::error,
          "`types_cpp_splits={}` is misconfigured: it can not be greater "
          "than the number of objects, which is {}.",
          split_count_,
          object_count);
    }
  }

  void validate_client_cpp_splits(
      const std::vector<t_service*>& services, diagnostic_context& ctx) {
    if (client_name_to_split_count_.empty()) {
      return;
    }
    for (const t_service* s : services) {
      auto iter = client_name_to_split_count_.find(s->get_name());
      if (iter != client_name_to_split_count_.end() &&
          iter->second > static_cast<int32_t>(s->get_functions().size())) {
        ctx.report(
            *s,
            "more-splits-than-functions-rule",
            diagnostic_level::error,
            "`client_cpp_splits={}` (For service {}) is misconfigured: it "
            "can not be greater than the number of functions, which is {}.",
            iter->second,
            s->get_name(),
            s->get_functions().size());
      }
    }
  }
};

void validate_lazy_fields(diagnostic_context& ctx, const t_field& field) {
  if (cpp2::is_lazy(&field)) {
    auto t = field.get_type()->get_true_type();
    const char* field_type = nullptr;
    if (t->is_any_int() || t->is_bool() || t->is_byte()) {
      field_type = "Integral field";
    }
    if (t->is_floating_point()) {
      field_type = "Floating point field";
    }
    if (field_type) {
      ctx.report(
          field,
          "no-lazy-int-float-field-rule",
          diagnostic_level::error,
          "{} `{}` can not be marked as lazy, since doing so won't bring "
          "any benefit.",
          field_type,
          field.get_name());
    }
  }
}

void t_mstch_cpp2_generator::fill_validator_visitors(
    ast_validator& validator) const {
  validator.add_structured_definition_visitor(std::bind(
      validate_struct_annotations,
      std::placeholders::_1,
      std::placeholders::_2,
      options()));
  validator.add_program_visitor(
      validate_splits(get_split_count(options()), client_name_to_split_count_));
  validator.add_field_visitor(validate_lazy_fields);
}

THRIFT_REGISTER_GENERATOR(mstch_cpp2, "cpp2", "");

} // namespace
} // namespace compiler
} // namespace thrift
} // namespace apache
