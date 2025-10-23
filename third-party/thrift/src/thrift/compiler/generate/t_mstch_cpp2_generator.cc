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
#include <cassert>
#include <filesystem>
#include <memory>
#include <queue>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fmt/core.h>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/type_visitor.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/sema/ast_validator.h>
#include <thrift/compiler/sema/schematizer.h>
#include <thrift/compiler/sema/sema_context.h>
#include <thrift/compiler/sema/standard_validator.h>

using apache::thrift::compiler::detail::schematizer;

namespace apache::thrift::compiler {
namespace {

// A compiler counterpart of cpp.EnumUnderlyingType that avoids dependency on
// the generated code and follows the compiler naming conventions.
enum class enum_underlying_type {
  i8 = 0,
  u8 = 1,
  i16 = 2,
  u16 = 3,
  u32 = 4,
};

const std::string& get_cpp_template(const t_type* type) {
  return type->get_unstructured_annotation({"cpp.template", "cpp2.template"});
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
  return type->is<t_container>() || type->is_string_or_binary() ||
      type->is<t_structured>();
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

  // Check if both types are the same kind and for primitives, the same type
  if (const t_primitive_type *prim_a = resolved_a->try_as<t_primitive_type>(),
      *prim_b = resolved_b->try_as<t_primitive_type>();
      prim_a != nullptr && prim_b != nullptr) {
    // Both are primitives, check they are the same primitive type
    if (prim_a->primitive_type() != prim_b->primitive_type()) {
      return false;
    }
  } else if (typeid(*resolved_a) != typeid(*resolved_b)) {
    // Use typeid for other types to check they are the same kind
    return false;
  }

  if (const t_list* list_a = resolved_a->try_as<t_list>()) {
    const auto* list_b = static_cast<const t_list*>(resolved_b);
    return same_types(
        list_a->elem_type().get_type(), list_b->elem_type().get_type());
  } else if (const t_set* set_a = resolved_a->try_as<t_set>()) {
    const auto* set_b = static_cast<const t_set*>(resolved_b);
    return same_types(
        set_a->elem_type().get_type(), set_b->elem_type().get_type());
  } else if (const t_map* map_a = resolved_a->try_as<t_map>()) {
    const auto* map_b = static_cast<const t_map*>(resolved_b);
    return same_types(&map_a->key_type().deref(), &map_b->key_type().deref()) &&
        same_types(&map_a->val_type().deref(), &map_b->val_type().deref());
  }
  return true;
}

std::vector<t_annotation> get_fatal_annotations(
    const deprecated_annotation_map& annotations) {
  std::vector<t_annotation> fatal_annotations;
  for (const auto& iter : annotations) {
    if (is_annotation_blacklisted_in_fatal(iter.first)) {
      continue;
    }
    fatal_annotations.emplace_back(iter.first, iter.second);
  }

  return fatal_annotations;
}

std::string get_fatal_string_short_id(const std::string& key) {
  return boost::algorithm::replace_all_copy(
      boost::algorithm::replace_all_copy(key, ".", "_"), "/", "_");
}
std::string get_fatal_string_short_id(const t_named* node) {
  // Use the unmodified cpp name.
  return cpp2::get_name(node);
}

std::string get_fatal_namespace_name_short_id(
    const std::string& lang, const std::string& ns) {
  std::string replacement = lang == "cpp" || lang == "cpp2" ? "__" : "_";
  std::string result = boost::algorithm::replace_all_copy(ns, ".", replacement);
  result = boost::algorithm::replace_all_copy(result, "/", "_");
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
    const t_mstch_generator::compiler_options_map& options) {
  return options.find("py3cpp") != options.end() ? "gen-py3cpp" : "gen-cpp2";
}

std::string mangle_field_name(const std::string& name) {
  return "__fbthrift_field_" + name;
}

bool should_mangle_field_storage_name_in_struct(const t_structured& s) {
  // We don't mangle field name if cpp.methods exist
  return !s.has_unstructured_annotation({"cpp.methods", "cpp2.methods"});
}

bool resolves_to_container_or_struct(const t_type* type) {
  return type->is<t_container>() || type->is<t_structured>();
}

bool has_schema(source_manager& sm, const t_program& program) {
  return program.find({schematizer::name_schema(sm, program), source_range{}});
}

bool generate_reduced_client(const t_interface& i) {
  return i.is<t_interaction>();
}

// Compute the set of types that appear anywhere in the service
// definition as input or output types. This presents maps, lists etc as seen
// in declarations, but unpacks the payloads of sinks and streams.
whisker::array::ptr build_user_type_footprint(
    const t_service& service,
    const whisker::prototype_database& prototype_database) {
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

struct cpp2_field_generator_context {
  const t_field* serialization_prev = nullptr;
  const t_field* serialization_next = nullptr;
  int isset_index = -1;
};

class cpp2_generator_context {
 public:
  static cpp2_generator_context create() { return cpp2_generator_context(); }

  cpp2_generator_context(cpp2_generator_context&&) = default;
  cpp2_generator_context& operator=(cpp2_generator_context&&) = default;

  bool is_orderable(
      const t_structured& structured_type,
      bool enableCustomTypeOrderingIfStructureHasUri) {
    return cpp2::OrderableTypeUtils::is_orderable(
        is_orderable_memo_,
        structured_type,
        enableCustomTypeOrderingIfStructureHasUri);
  }

  cpp_name_resolver& resolver() { return resolver_; }

  const cpp2_field_generator_context* get_field_context(
      const t_field* field) const {
    auto it = field_context_map_.find(field);
    return it == field_context_map_.end() ? nullptr : &it->second;
  }

  void register_visitors(t_whisker_generator::context_visitor& visitor) {
    // Compute field isset indexes and serialization order, which requires a
    // back-reference to the parent structured definition
    visitor.add_structured_definition_visitor([this](const t_structured& node) {
      cpp2_field_generator_context field_ctx;
      for (const t_field& field : node.fields()) {
        if (cpp2::field_has_isset(&field)) {
          field_ctx.isset_index++;
        }
        field_context_map_[&field] = field_ctx;
      }

      const std::vector<t_field*>& serialization_order =
          node.has_structured_annotation(kSerializeInFieldIdOrderUri)
          ? node.get_sorted_members()
          : node.get_members();
      const t_field* prev = nullptr;
      for (const t_field* curr : serialization_order) {
        if (prev) {
          field_context_map_[prev].serialization_next = curr;
          field_context_map_[curr].serialization_prev = prev;
        }
        prev = curr;
      }
    });
  }

 private:
  cpp2_generator_context() = default;

  std::unordered_map<const t_type*, bool> is_orderable_memo_;
  cpp_name_resolver resolver_;

  // Although generator fields can be in a different order than the IDL
  // order, field_generator_context should be always computed in the IDL order,
  // as the context does not change by reordering. Without the map, each
  // different reordering recomputes field_generator_context, and each
  // field takes O(N) to loop through node_list_view<t_field> or
  // std::vector<t_field*> to find the exact t_field to compute
  // field_generator_context.
  std::unordered_map<const t_field*, cpp2_field_generator_context>
      field_context_map_;
};

int checked_stoi(const std::string& s, const std::string& msg) {
  std::size_t pos = 0;
  int ret = std::stoi(s, &pos);
  if (pos != s.size()) {
    throw std::runtime_error(msg);
  }
  return ret;
}

int get_split_count(const t_mstch_generator::compiler_options_map& options) {
  auto iter = options.find("types_cpp_splits");
  if (iter == options.end()) {
    return 0;
  }
  return checked_stoi(
      iter->second, "Invalid types_cpp_splits value: `" + iter->second + "`");
}

whisker::object make_whisker_annotations(
    const std::vector<t_annotation>& annotations) {
  whisker::array::raw result;
  result.reserve(annotations.size());
  for (const t_annotation& a : annotations) {
    result.push_back(whisker::make::map({
        {"key", whisker::make::string(a.first)},
        {"value", whisker::make::string(a.second.value)},
    }));
  }
  return whisker::make::array(std::move(result));
}

class t_mstch_cpp2_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "program:autogen_path",
        "service:autogen_path",
        "field:fatal_annotations?",
    };
    return opts;
  }

  std::string template_prefix() const override { return "cpp2"; }

  void process_options(
      const std::map<std::string, std::string>& options) override {
    t_mstch_generator::process_options(options);
    client_name_to_split_count_ = get_client_name_to_split_count();
    out_dir_base_ = get_out_dir_base(this->options());
  }

  void generate_program() override;
  void fill_validator_visitors(ast_validator&) const override;
  static std::string get_cpp2_namespace(const t_program* program);
  static std::string get_cpp2_unprefixed_namespace(const t_program* program);
  static mstch::array cpp_includes(const t_program* program);
  static mstch::node include_prefix(
      const t_program* program, compiler_options_map& options);

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

  void initialize_context(context_visitor& visitor) override {
    cpp_context_ = std::make_shared<cpp2_generator_context>(
        cpp2_generator_context::create());
    cpp_context_->register_visitors(visitor);
  }

  whisker::map::raw globals() const override {
    whisker::map::raw globals = t_mstch_generator::globals();
    // Provide global default for cpp_enable_same_program_const_referencing?
    // Only the template for module_types.h overrides this, setting it to FALSE
    // Controls whether references to consts in the current Thrift file can be
    // emitted as references, or must be inlined. By default, we can emit all
    // const usage as references.
    // module_types.h is an exception, because module_constants.h (where const
    // accessors are declared) depends on module_types.h, so module_types.h
    // cannot include module_constants.h without a circular dependency.
    // This restriction only applies within the same program - module_types.h
    // CAN include and reference consts from other programs.
    globals["cpp_enable_same_program_const_referencing?"] =
        whisker::make::true_value;
    globals["cpp_fatal_string_id"] = whisker::dsl::make_function(
        "cpp_fatal_string_id",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(
              get_fatal_string_short_id(ctx.argument<whisker::string>(0)));
        });
    globals["cpp_render_fatal"] = whisker::dsl::make_function(
        "cpp_render_fatal",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(
              render_fatal_string(ctx.argument<whisker::string>(0)));
        });
    return globals;
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);
    def.property(
        "cpp_qualified_namespace", &cpp2::get_gen_unprefixed_namespace);
    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);
    def.property("cpp_name", [](const t_named& named) {
      return cpp2::get_name(&named);
    });
    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    def.property("cpp_qualified_underlying_name", [this](const t_type& type) {
      return cpp_context_->resolver().get_underlying_namespaced_name(type);
    });

    def.property("resolves_to_complex_return?", [](const t_type& type) {
      return is_complex_return(type.get_true_type());
    });

    def.property("cpp_type", [&](const t_type& type) {
      return cpp_context_->resolver().get_native_type(type);
    });

    def.property("cpp_standard_type", [&](const t_type& type) {
      return cpp_context_->resolver().get_standard_type(type);
    });

    return std::move(def).make();
  }

  prototype<t_typedef>::ptr make_prototype_for_typedef(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_typedef(proto);
    auto def = whisker::dsl::prototype_builder<h_typedef>::extends(base);

    def.property("cpp_type", [&](const t_typedef& t) {
      return cpp_context_->resolver().get_underlying_type_name(t);
    });

    return std::move(def).make();
  }

  prototype<t_enum>::ptr make_prototype_for_enum(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_enum(proto);
    auto def = whisker::dsl::prototype_builder<h_enum>::extends(base);

    def.property("cpp_min", [](const t_enum& e) -> whisker::object {
      auto values = e.values();
      if (values.empty()) {
        return {};
      }
      auto min = std::min_element(
          values.begin(),
          values.end(),
          [](const t_enum_value& a, const t_enum_value& b) {
            return a.get_value() < b.get_value();
          });
      return whisker::object(cpp2::get_name(&*min));
    });

    def.property("cpp_max", [](const t_enum& e) -> whisker::object {
      auto values = e.values();
      if (values.empty()) {
        return {};
      }
      auto max = std::max_element(
          values.begin(),
          values.end(),
          [](const t_enum_value& a, const t_enum_value& b) {
            return a.get_value() < b.get_value();
          });
      return whisker::object(cpp2::get_name(&*max));
    });

    def.property("cpp_enum_type", [](const t_enum& e) -> std::string {
      if (const auto* annot =
              e.find_structured_annotation_or_null(kCppEnumTypeUri)) {
        const auto& type = annot->get_value_from_structured_annotation("type");
        switch (static_cast<enum_underlying_type>(type.get_integer())) {
          case enum_underlying_type::i8:
            return "::std::int8_t";
          case enum_underlying_type::u8:
            return "::std::uint8_t";
          case enum_underlying_type::i16:
            return "::std::int16_t";
          case enum_underlying_type::u16:
            return "::std::uint16_t";
          case enum_underlying_type::u32:
            return "::std::uint32_t";
          default:
            throw std::runtime_error("unknown enum underlying type");
        }
      }
      if (const std::string* type =
              e.find_unstructured_annotation_or_null("cpp.enum_type")) {
        return *type;
      }
      return e.has_unstructured_annotation("cpp.deprecated_enum_unscoped")
          ? "int"
          : "";
    });

    def.property("cpp_is_unscoped", [](const t_enum& e) {
      return e.get_unstructured_annotation("cpp.deprecated_enum_unscoped");
    });

    def.property("cpp_declare_bitwise_ops", [](const t_enum& e) {
      return e.has_unstructured_annotation("cpp.declare_bitwise_ops") ||
          e.has_structured_annotation(kBitmaskEnumUri);
    });

    return std::move(def).make();
  }

  prototype<t_function>::ptr make_prototype_for_function(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_function(proto);
    auto def = whisker::dsl::prototype_builder<h_function>::extends(base);

    def.property("cpp_return_type", [&](const t_function& f) -> std::string {
      return cpp_context_->resolver().get_return_type(f);
    });

    // Specifies if the generated recv_* functions have an additional argument
    // representing the return value.
    def.property("cpp_recv_arg?", [&](const t_function& f) {
      return !f.return_type()->is_void() || f.sink_or_stream();
    });

    def.property("stack_arguments?", [this](const t_function& function) {
      return cpp2::is_stack_arguments(compiler_options(), function);
    });

    return std::move(def).make();
  }

  prototype<t_service>::ptr make_prototype_for_service(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_service(proto);
    auto def = whisker::dsl::prototype_builder<h_service>::extends(base);

    def.property(
        "cpp_requires_method_decorator?", [](const t_service& service) {
          return service.has_structured_annotation(
              apache::thrift::compiler::kCppGenerateServiceMethodDecorator);
        });
    def.property("qualified_name", &cpp2::get_service_qualified_name);
    def.property("user_type_footprint", [&](const t_service& service) {
      return build_user_type_footprint(service, proto);
    });

    return std::move(def).make();
  }

  prototype<t_interaction>::ptr make_prototype_for_interaction(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interaction(proto);
    auto def = whisker::dsl::prototype_builder<h_interaction>::extends(base);
    def.property("event_base?", [](const t_interaction& self) {
      return self.has_unstructured_annotation("process_in_event_base") ||
          self.has_structured_annotation(kCppProcessInEbThreadUri);
    });
    def.property("serial?", [](const t_interaction& self) {
      return self.has_unstructured_annotation("serial") ||
          self.has_structured_annotation(kSerialUri);
    });
    return std::move(def).make();
  }

  prototype<t_const>::ptr make_prototype_for_const(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const(proto);
    auto def = whisker::dsl::prototype_builder<h_const>::extends(base);
    def.property("external?", [this](const t_const& self) {
      return self.program() != program_;
    });
    return std::move(def).make();
  }

  std::unordered_map<std::string, int> get_client_name_to_split_count() const;

  std::shared_ptr<cpp2_generator_context> cpp_context_;
  std::unordered_map<std::string, int32_t> client_name_to_split_count_;
};

class cpp_mstch_program : public mstch_program {
  // transitive_schema_initializers depends on consistent order.
  struct program_less {
    bool operator()(const t_program* a, const t_program* b) const {
      return a->path() < b->path();
    }
  };
  using transitive_include_map =
      std::map<const t_program*, std::string, program_less>;

 public:
  cpp_mstch_program(
      const t_program* program,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager& sm,
      std::optional<int32_t> split_id = std::nullopt,
      std::optional<std::vector<t_structured*>> split_structs = std::nullopt)
      : mstch_program(program, ctx, pos),
        split_id_(split_id),
        split_structs_(std::move(split_structs)),
        sm_(sm) {
    register_methods(
        this,
        {{"program:cpp_includes", &cpp_mstch_program::cpp_includes},
         {"program:qualified_namespace",
          &cpp_mstch_program::qualified_namespace},
         {"program:include_prefix", &cpp_mstch_program::include_prefix},
         {"program:cpp_declare_hash?", &cpp_mstch_program::cpp_declare_hash},
         {"program:thrift_includes", &cpp_mstch_program::thrift_includes},
         {"program:transitive_schema_initializers",
          &cpp_mstch_program::transitive_schema_initializers},
         {"program:num_transitive_thrift_includes",
          &cpp_mstch_program::num_transitive_thrift_includes},
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
         {"program:has_schema?", &cpp_mstch_program::has_schema},
         {"program:schema_name", &cpp_mstch_program::schema_name},
         {"program:schema_includes_const?",
          &cpp_mstch_program::schema_includes_const},
         {"program:structs_and_typedefs",
          &cpp_mstch_program::structs_and_typedefs}});
  }
  std::string get_program_namespace(const t_program* program) override {
    return t_mstch_cpp2_generator::get_cpp2_namespace(program);
  }

  std::vector<const t_typedef*> alias_to_struct() {
    std::vector<const t_typedef*> result;
    for (const t_typedef* i : program_->typedefs()) {
      const t_type* alias = i->get_type();
      if (alias->is<t_typedef>() &&
          alias->has_unstructured_annotation("cpp.type")) {
        const t_type* ttype = i->get_type()->get_true_type();
        if ((ttype->is<t_structured>()) &&
            !cpp_name_resolver::find_first_adapter(*ttype)) {
          result.push_back(i);
        }
      }
    }
    return result;
  }
  template <typename Node>
  void collect_fatal_string_annotated(
      std::map<std::string, std::string>& fatal_strings, const Node* node) {
    fatal_strings.emplace(get_fatal_string_short_id(node), node->name());
    auto hash = cpp2::sha256_hex(node->name());
    fatal_strings.emplace("__fbthrift_hash_" + hash, node->name());
    for (const auto& a : node->unstructured_annotations()) {
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
      if (obj->is<t_union>()) {
        result.push_back(get_fatal_string_short_id(obj));
      }
    }
    return result;
  }
  std::vector<std::string> get_fatal_struct_names() {
    std::vector<std::string> result;
    for (const t_structured* obj : program_->structured_definitions()) {
      if (!obj->is<t_union>() && !cpp_name_resolver::find_first_adapter(*obj)) {
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
      a.emplace_back(mstch::map{
          {"fatal_string:name", vec.at(i)},
          {"last?", i == vec.size() - 1},
      });
    }
    return mstch::map{{"fatal_strings:items", a}};
  }
  mstch::node qualified_namespace() {
    return t_mstch_cpp2_generator::get_cpp2_unprefixed_namespace(program_);
  }
  mstch::node cpp_includes() {
    mstch::array includes = t_mstch_cpp2_generator::cpp_includes(program_);
    auto it = context_.options.find("includes");
    if (it != context_.options.end()) {
      std::vector<std::string> extra_includes;
      boost::split(extra_includes, it->second, [](char c) { return c == ':'; });
      for (auto& include : extra_includes) {
        includes.emplace_back(mstch::map{{"cpp_include", std::move(include)}});
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
          return strct->has_unstructured_annotation(
              {"cpp.declare_hash", "cpp2.declare_hash"});
        });
    bool cpp_declare_in_typedefs = std::any_of(
        program_->typedefs().begin(),
        program_->typedefs().end(),
        [](const auto* typedf) {
          return typedf->get_type()->has_unstructured_annotation(
              {"cpp.declare_hash", "cpp2.declare_hash"});
        });
    return cpp_declare_in_structs || cpp_declare_in_typedefs;
  }
  mstch::node thrift_includes() {
    mstch::array a;
    for (const auto* program : program_->get_includes_for_codegen()) {
      a.emplace_back(make_mstch_program_cached(program, context_));
    }
    return a;
  }
  /**
   * To reduce build time, the generated constants code only includes the
   * headers for direct thrift includes in the .cpp file and not in the .h
   * file, which means constants from transitive includes are not visible. To
   * allow constructing a flattened array of schemas for transitive
   * dependencies without undoing this optimization we indirect through the
   * flattened array of one of the direct includes to reach the schema of the
   * transitive include. Constructing this in mustache would be horrible, so
   * we build up the code here instead.
   */
  std::unique_ptr<transitive_include_map> gen_transitive_include_map(
      const t_program* program) {
    auto includes = std::make_unique<transitive_include_map>();
    auto local_includes = program->get_includes_for_codegen();
    for (const auto* include : local_includes) {
      if (include->has_structured_annotation(kDisableSchemaConstUri)) {
        continue;
      }

      includes->emplace(
          include,
          fmt::format(
              "::apache::thrift::detail::mc::readSchema({}::{}_constants::{})",
              t_mstch_cpp2_generator::get_cpp2_namespace(include),
              include->name(),
              schematizer::name_schema(sm_, *include)));
      const auto& recursive_includes = context_.cache().get(
          *include, [&] { return gen_transitive_include_map(include); });
      // Transitive includes begin at 1 because every programs' list of includes
      // has itself as the first entry.
      size_t i = 1;
      for (const auto& [recursive_include, _] : recursive_includes) {
        if (includes->count(recursive_include) == 0) {
          includes->emplace(
              recursive_include,
              fmt::format(
                  "::apache::thrift::detail::mc::readSchemaInclude({}::{}_constants::{}_includes, {})",
                  t_mstch_cpp2_generator::get_cpp2_namespace(include),
                  include->name(),
                  schematizer::name_schema(sm_, *include),
                  i));
        }
        ++i;
      }
    }
    return includes;
  }
  mstch::node transitive_schema_initializers() {
    const auto& includes = context_.cache().get(
        *program_, [&] { return gen_transitive_include_map(program_); });
    mstch::array initializers = {
        fmt::format("{}()", schematizer::name_schema(sm_, *program_))};
    initializers.reserve(includes.size() + 1);
    for (const auto& [_, include] : includes) {
      initializers.emplace_back(include);
    }
    return initializers;
  }
  mstch::node num_transitive_thrift_includes() {
    const auto& includes = context_.cache().get(
        *program_, [&] { return gen_transitive_include_map(program_); });
    // Codegen includes the root program but transitive_include_map does not.
    return includes.size() + 1;
  }
  mstch::node frozen_packed() { return get_option("frozen") == "packed"; }
  mstch::node legacy_api() { return true; }
  mstch::node fatal_languages() {
    mstch::array a;
    for (const auto& pair : program_->namespaces()) {
      if (!pair.second.empty()) {
        a.emplace_back(mstch::map{
            {"language:safe_name", get_fatal_string_short_id(pair.first)},
            {"language:safe_namespace",
             get_fatal_namespace_name_short_id(pair.first, pair.second)},
            {"last?", false},
        });
      }
    }
    if (!a.empty()) {
      std::get<mstch::map>(a.back())["last?"] = true;
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
      unique_names.emplace(get_fatal_string_short_id(enm), enm->name());
      for (const auto& i : enm->values()) {
        collect_fatal_string_annotated(unique_names, &i);
      }
    }
    // structs, unions and exceptions
    for (const t_structured* obj : program_->structured_definitions()) {
      if (obj->is<t_union>()) {
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
      unique_names.emplace(get_fatal_string_short_id(cnst), cnst->name());
    }
    // services
    for (const auto* service : program_->services()) {
      // function annotations are not currently included.
      unique_names.emplace(get_fatal_string_short_id(service), service->name());
      for (const auto* f : service->get_functions()) {
        unique_names.emplace(get_fatal_string_short_id(f), f->name());
        for (const auto& p : f->params().fields()) {
          unique_names.emplace(get_fatal_string_short_id(&p), p.name());
        }
      }
    }
    // typedefs resolve to struct
    for (const t_typedef* i : alias_to_struct()) {
      unique_names.emplace(get_fatal_string_short_id(i), i->name());
    }

    mstch::array a;
    for (const auto& name : unique_names) {
      a.emplace_back(mstch::map{
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
      if (s->is<t_union>()) {
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
      a.emplace_back(*f);
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
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
    std::transform(
        sorted.begin(),
        sorted.end(),
        std::back_inserter(ret),
        [&](const t_type* node) -> mstch::node {
          if (auto typedf = node->try_as<t_typedef>()) {
            return context_.typedef_factory->make_mstch_object(
                typedf, context_);
          }
          return make_mstch_element_cached(
              static_cast<const t_structured*>(node),
              *context_.struct_factory,
              context_.struct_cache,
              id,
              0,
              0);
        });
    return ret;
  }

  mstch::node split_structs() {
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
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
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
    return make_mstch_array_cached(
        split_id_ ? *split_enums_ : program_->enums(),
        *context_.enum_factory,
        context_.enum_cache,
        id);
  }

  mstch::node has_schema() {
    return ::apache::thrift::compiler::has_schema(sm_, *program_);
  }

  mstch::node schema_name() { return schematizer::name_schema(sm_, *program_); }

  mstch::node schema_includes_const() {
    return supports_schema_includes(program_);
  }

 private:
  bool supports_schema_includes(const t_program* program) {
    enum class strong_bool {};
    strong_bool supports = context_.cache().get(*program, [&] {
      bool ret =
          // If you don't have a schema const you don't need schema includes.
          ::apache::thrift::compiler::has_schema(sm_, *program_) &&
          // Opting out of schema const should disable all of its failure modes.
          !program->has_structured_annotation(kDisableSchemaConstUri);
      return std::make_unique<strong_bool>(static_cast<strong_bool>(ret));
    });
    return static_cast<bool>(supports);
  }

  const std::optional<int32_t> split_id_;
  const std::optional<std::vector<t_structured*>> split_structs_;
  std::optional<std::vector<t_enum*>> split_enums_;
  source_manager& sm_;
};

class cpp_mstch_service : public mstch_service {
 public:
  cpp_mstch_service(
      const t_service* service,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager& sm,
      const t_service* containing_service = nullptr,
      int32_t split_id = 0,
      int32_t split_count = 1)
      : mstch_service(service, ctx, pos, containing_service), sm_(sm) {
    register_methods(
        this,
        {
            {"service:program_name", &cpp_mstch_service::program_name},
            {"service:program_qualified_name",
             &cpp_mstch_service::program_qualified_name},
            {"service:autogen_path", &cpp_mstch_service::autogen_path},
            {"service:include_prefix", &cpp_mstch_service::include_prefix},
            {"service:thrift_includes", &cpp_mstch_service::thrift_includes},
            {"service:qualified_namespace",
             &cpp_mstch_service::qualified_namespace},
            {"service:oneway_functions", &cpp_mstch_service::oneway_functions},
            {"service:oneways?", &cpp_mstch_service::has_oneway},
            {"service:cpp_includes", &cpp_mstch_service::cpp_includes},
            {"service:metadata_name", &cpp_mstch_service::metadata_name},
            {"service:parent_service_cpp_name",
             &cpp_mstch_service::parent_service_cpp_name},
            {"service:parent_service_qualified_name",
             &cpp_mstch_service::parent_service_qualified_name},
            {"service:thrift_uri_or_service_name",
             &cpp_mstch_service::thrift_uri_or_service_name},
            {"service:has_service_schema",
             &cpp_mstch_service::has_service_schema},
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
      a.emplace_back(make_mstch_program_cached(program, context_));
    }
    return a;
  }
  mstch::node qualified_namespace() {
    return t_mstch_cpp2_generator::get_cpp2_unprefixed_namespace(
        service_->program());
  }
  mstch::node oneway_functions() {
    std::vector<const t_function*> oneway_functions;
    for (const auto* function : get_functions()) {
      if (function->qualifier() == t_function_qualifier::oneway) {
        oneway_functions.push_back(function);
      }
    }
    return make_mstch_functions(oneway_functions);
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
    return service_->program()->name() + "_" + service_->name();
  }
  mstch::node parent_service_cpp_name() {
    return cpp2::get_name(parent_service());
  }
  mstch::node parent_service_qualified_name() {
    return cpp2::get_service_qualified_name(*parent_service());
  }
  mstch::node reduced_client() { return generate_reduced_client(*service_); }
  mstch::node thrift_uri_or_service_name() {
    return service_->uri().empty() ? parent_service_name() : service_->uri();
  }
  mstch::node has_service_schema() {
    return has_schema(sm_, *service_->program());
  }

 private:
  const std::vector<t_function*>& get_functions() const override {
    return functions_;
  }

  std::vector<t_function*> functions_;
  source_manager& sm_;
};

class cpp_mstch_interaction : public cpp_mstch_service {
 public:
  using ast_type = t_interaction;

  cpp_mstch_interaction(
      const t_interaction* interaction,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service,
      source_manager& sm)
      : cpp_mstch_service(interaction, ctx, pos, sm, containing_service) {}
};

class cpp_mstch_function : public mstch_function {
 public:
  cpp_mstch_function(
      const t_function* function,
      mstch_context& ctx,
      mstch_element_position pos,
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_function(function, ctx, pos), cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"function:eb", &cpp_mstch_function::event_based},
            {"function:stack_arguments?", &cpp_mstch_function::stack_arguments},
            {"function:sync_returns_by_outparam?",
             &cpp_mstch_function::sync_returns_by_outparam},
            {"function:prefixed_name", &cpp_mstch_function::prefixed_name},
            {"function:has_deprecated_header_client_methods",
             &cpp_mstch_function::has_deprecated_header_client_methods},
            {"function:virtual_client_methods?",
             &cpp_mstch_function::virtual_client_methods},
            {"function:legacy_client_methods?",
             &cpp_mstch_function::legacy_client_methods},
        });
  }
  mstch::node event_based() {
    return function_->get_unstructured_annotation("thread") == "eb" ||
        function_->has_structured_annotation(kCppProcessInEbThreadUri) ||
        interface().has_unstructured_annotation("process_in_event_base") ||
        interface().has_structured_annotation(kCppProcessInEbThreadUri);
  }
  mstch::node stack_arguments() {
    return cpp2::is_stack_arguments(context_.options, *function_);
  }
  mstch::node sync_returns_by_outparam() {
    return is_complex_return(function_->return_type()->get_true_type()) &&
        !function_->interaction() && !function_->sink_or_stream();
  }

  mstch::node prefixed_name() {
    const std::string& name = cpp2::get_name(function_);
    return interface().is<t_interaction>()
        ? fmt::format("{}_{}", interface().name(), name)
        : name;
  }

  mstch::node has_deprecated_header_client_methods() {
    return function_->has_structured_annotation(
               kCppGenerateDeprecatedHeaderClientMethodsUri) ||
        function_->has_unstructured_annotation(
            "cpp.generate_deprecated_header_client_methods") ||
        interface().has_structured_annotation(
            kCppGenerateDeprecatedHeaderClientMethodsUri) ||
        interface().has_unstructured_annotation(
            "cpp.generate_deprecated_header_client_methods");
  }

  mstch::node virtual_client_methods() {
    return !generate_reduced_client(interface()) && !function_->interaction() &&
        !function_->is_bidirectional_stream();
  }

  mstch::node legacy_client_methods() {
    return !generate_reduced_client(interface()) && !function_->interaction() &&
        !function_->is_bidirectional_stream();
  }

 private:
  std::shared_ptr<cpp2_generator_context> cpp_context_;
};

bool needs_op_encode(const t_type& type);

bool check_container_needs_op_encode(const t_type& type) {
  const auto* true_type = type.get_true_type();
  if (auto list_container = true_type->try_as<t_list>()) {
    return needs_op_encode(*list_container->elem_type());
  } else if (auto set_container = true_type->try_as<t_set>()) {
    return needs_op_encode(*set_container->elem_type());
  } else if (auto map_container = true_type->try_as<t_map>()) {
    return needs_op_encode(*map_container->key_type()) ||
        needs_op_encode(*map_container->val_type());
  }
  return false;
}

bool needs_op_encode(const t_type& type) {
  return (type.program() &&
          type.program()->inherit_annotation_or_null(
              type, kCppUseOpEncodeUri)) ||
      t_typedef::get_first_structured_annotation_or_null(
             &type, kCppUseOpEncodeUri) ||
      cpp_name_resolver::find_first_adapter(type) ||
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
      strct.has_structured_annotation(kCppUseOpEncodeUri) ||
      cpp_name_resolver::find_first_adapter(field) ||
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
            {"type:resolves_to_fixed_size?",
             &cpp_mstch_type::resolves_to_fixed_size},
            {"type:resolves_to_enum?", &cpp_mstch_type::resolves_to_enum},
            {"type:transitively_refers_to_struct?",
             &cpp_mstch_type::transitively_refers_to_struct},
            {"type:cpp_fullname", &cpp_mstch_type::cpp_fullname},
            {"type:cpp_standard_type", &cpp_mstch_type::cpp_standard_type},
            {"type:cpp_adapter", &cpp_mstch_type::cpp_adapter},
            {"type:string_or_binary?", &cpp_mstch_type::is_string_or_binary},
            {"type:non_empty_struct?", &cpp_mstch_type::is_non_empty_struct},
            {"type:qualified_namespace", &cpp_mstch_type::qualified_namespace},
            {"type:cpp_declare_hash", &cpp_mstch_type::cpp_declare_hash},
            {"type:cpp_declare_equal_to",
             &cpp_mstch_type::cpp_declare_equal_to},
            {"type:type_class", &cpp_mstch_type::type_class},
            {"type:type_tag", &cpp_mstch_type::type_tag},
            {"type:program_name", &cpp_mstch_type::program_name},
            {"type:cpp_use_allocator?", &cpp_mstch_type::cpp_use_allocator},
            {"type:use_op_encode?", &cpp_mstch_type::use_op_encode},
        });
  }
  std::string get_type_namespace(const t_program* program) override {
    return cpp2::get_gen_namespace(*program);
  }
  mstch::node resolves_to_base() {
    return resolved_type_->is<t_primitive_type>();
  }
  mstch::node resolves_to_integral() {
    return resolved_type_->is_byte() || resolved_type_->is_any_int();
  }
  mstch::node resolves_to_base_or_enum() {
    return resolved_type_->is<t_primitive_type>() ||
        resolved_type_->is<t_enum>();
  }
  mstch::node resolves_to_container() {
    return resolved_type_->is<t_container>();
  }
  mstch::node resolves_to_container_or_struct() {
    return ::apache::thrift::compiler::resolves_to_container_or_struct(
        resolved_type_);
  }
  mstch::node resolves_to_container_or_enum() {
    return resolved_type_->is<t_container>() || resolved_type_->is<t_enum>();
  }
  mstch::node resolves_to_fixed_size() {
    return resolved_type_->is_bool() || resolved_type_->is_byte() ||
        resolved_type_->is_any_int() || resolved_type_->is<t_enum>() ||
        resolved_type_->is_floating_point();
  }
  mstch::node resolves_to_enum() { return resolved_type_->is<t_enum>(); }
  mstch::node transitively_refers_to_struct() {
    // fast path is unnecessary but may avoid allocations
    if (resolved_type_->is<t_struct>() || resolved_type_->is<t_union>()) {
      return true;
    }
    if (!resolved_type_->is<t_container>()) {
      return false;
    }
    // type is a container: traverse (breadthwise, but could be depthwise)
    std::queue<const t_type*> queue;
    queue.push(resolved_type_);
    while (!queue.empty()) {
      auto next = queue.front();
      queue.pop();
      if (next->is<t_struct>() || next->is<t_union>()) {
        return true;
      }
      if (!next->is<t_container>()) {
        continue;
      }
      if (const t_list* list = next->try_as<t_list>()) {
        queue.push(list->elem_type().get_type());
      } else if (const t_set* set = next->try_as<t_set>()) {
        queue.push(set->elem_type().get_type());
      } else if (const t_map* map = next->try_as<t_map>()) {
        queue.push(&map->key_type().deref());
        queue.push(&map->val_type().deref());
      } else {
        assert(false);
      }
    }
    return false;
  }
  mstch::node cpp_fullname() {
    return cpp_context_->resolver().get_namespaced_name(
        *type_->program(), *type_);
  }
  mstch::node cpp_standard_type() {
    return cpp_context_->resolver().get_standard_type(*type_);
  }
  mstch::node cpp_adapter() {
    if (const auto* adapter = cpp_name_resolver::find_first_adapter(*type_)) {
      return *adapter;
    }
    return {};
  }
  mstch::node is_string_or_binary() {
    return resolved_type_->is_string_or_binary();
  }
  mstch::node cpp_declare_hash() {
    return resolved_type_->has_unstructured_annotation(
        {"cpp.declare_hash", "cpp2.declare_hash"});
  }
  mstch::node cpp_declare_equal_to() {
    return resolved_type_->has_unstructured_annotation(
        {"cpp.declare_equal_to", "cpp2.declare_equal_to"});
  }
  mstch::node cpp_use_allocator() {
    return !!t_typedef::get_first_unstructured_annotation_or_null(
        type_, {"cpp.use_allocator"});
  }
  mstch::node is_non_empty_struct() {
    auto as_struct = resolved_type_->try_as<t_structured>();
    return as_struct && as_struct->has_fields();
  }
  mstch::node qualified_namespace() {
    return t_mstch_cpp2_generator::get_cpp2_unprefixed_namespace(
        type_->program());
  }
  mstch::node type_class() { return cpp2::get_gen_type_class(*resolved_type_); }
  mstch::node type_tag() {
    return cpp_context_->resolver().get_type_tag(*type_);
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
            {"struct:num_fields", &cpp_mstch_struct::num_fields},
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
            {"struct:cpp_fullname", &cpp_mstch_struct::cpp_fullname},
            {"struct:cpp_methods", &cpp_mstch_struct::cpp_methods},
            {"struct:cpp_declare_hash", &cpp_mstch_struct::cpp_declare_hash},
            {"struct:cpp_declare_equal_to",
             &cpp_mstch_struct::cpp_declare_equal_to},
            {"struct:cpp_noncopyable", &cpp_mstch_struct::cpp_noncopyable},
            {"struct:cpp_noncomparable", &cpp_mstch_struct::cpp_noncomparable},
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
            {"struct:min_union_member",
             &cpp_mstch_struct::get_min_union_member},
            {"struct:max_union_member",
             &cpp_mstch_struct::get_max_union_member},
            {"struct:cpp_allocator", &cpp_mstch_struct::cpp_allocator},
            {"struct:cpp_allocator_via", &cpp_mstch_struct::cpp_allocator_via},
            {"struct:cpp_frozen2_exclude?",
             &cpp_mstch_struct::cpp_frozen2_exclude},
            {"struct:has_non_optional_and_non_terse_field?",
             &cpp_mstch_struct::has_non_optional_and_non_terse_field},
            {"struct:fields_with_runtime_annotation?",
             &cpp_mstch_struct::has_fields_with_runtime_annotation},
            {"struct:fields_with_runtime_annotation",
             &cpp_mstch_struct::fields_with_runtime_annotation},
            {"struct:structured_runtime_annotations",
             &cpp_mstch_struct::structured_runtime_annotations},
            {"struct:any?", &cpp_mstch_struct::any},
            {"struct:extra_namespace", &cpp_mstch_struct::extra_namespace},
            {"struct:type_tag", &cpp_mstch_struct::type_tag},
            {"struct:is_trivially_destructible?",
             &cpp_mstch_struct::is_trivially_destructible},
        });
  }
  mstch::node num_fields() { return struct_->fields().size(); }
  mstch::node explicitly_constructed_fields() {
    // Filter fields according to the following criteria:
    // Get all enums
    // Get all base_types but empty strings
    // Get all non-empty structs and containers
    // Get all non-optional references with basetypes, enums,
    // non-empty structs, and containers
    std::vector<const t_field*> filtered_fields;
    for (const auto* field : get_members_in_layout_order()) {
      const t_type* type = field->type()->get_true_type();
      // Filter out all optional references.
      if (cpp2::is_explicit_ref(field) &&
          field->qualifier() == t_field_qualifier::optional) {
        continue;
      }
      if (type->is<t_enum>() ||
          (type->is<t_primitive_type>() && !type->is_string_or_binary()) ||
          (type->is_string_or_binary() && field->default_value() != nullptr) ||
          (type->is<t_container>() && field->default_value() != nullptr &&
           !field->default_value()->is_empty()) ||
          ((type->is<t_struct>() || type->is<t_union>()) &&
           (struct_ != type->try_as<t_struct>()) &&
           ((field->default_value() && !field->default_value()->is_empty()) ||
            (cpp2::is_explicit_ref(field) &&
             field->qualifier() != t_field_qualifier::optional))) ||
          (type->is<t_container>() && cpp2::is_explicit_ref(field) &&
           field->qualifier() != t_field_qualifier::optional) ||
          (type->is<t_primitive_type>() && cpp2::is_explicit_ref(field) &&
           field->qualifier() != t_field_qualifier::optional)) {
        filtered_fields.push_back(field);
      }
    }
    return make_mstch_fields(filtered_fields);
  }

  mstch::node mixin_fields() {
    mstch::array fields;
    for (auto i : cpp2::get_mixins_and_members(*struct_)) {
      const auto suffix = "_ref";
      fields.emplace_back(mstch::map{
          {"mixin:name", i.mixin->name()},
          {"mixin:field_name", i.member->name()},
          {"mixin:accessor", i.member->name() + suffix}});
    }
    return fields;
  }

  mstch::node is_struct_orderable() {
    return cpp_context_->is_orderable(
               *struct_,
               !context_.options.count(
                   "disable_custom_type_ordering_if_structure_has_uri")) &&
        !struct_->has_unstructured_annotation("no_default_comparators");
  }
  mstch::node nondefault_copy_ctor_and_assignment() {
    if (struct_->has_unstructured_annotation("cpp.allocator")) {
      return true;
    }
    for (const auto& f : struct_->fields()) {
      if (cpp2::field_transitively_refers_to_unique(&f) || cpp2::is_lazy(&f) ||
          cpp_name_resolver::find_first_adapter(f)) {
        return true;
      }
    }
    return false;
  }
  mstch::node cpp_fullname() {
    return cpp_context_->resolver().get_underlying_namespaced_name(*struct_);
  }
  mstch::node cpp_underlying_name() {
    return cpp_name_resolver::get_underlying_name(*struct_);
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
    return struct_->get_unstructured_annotation(
        {"cpp.methods", "cpp2.methods"});
  }
  mstch::node cpp_declare_hash() {
    return struct_->has_unstructured_annotation(
        {"cpp.declare_hash", "cpp2.declare_hash"});
  }
  mstch::node cpp_declare_equal_to() {
    return struct_->has_unstructured_annotation(
        {"cpp.declare_equal_to", "cpp2.declare_equal_to"});
  }
  mstch::node cpp_noncopyable() {
    if (struct_->has_unstructured_annotation(
            {"cpp.noncopyable", "cpp2.noncopyable"})) {
      return true;
    }

    bool result = false;
    cpp2::for_each_transitive_field(struct_, [&result](const t_field* field) {
      if (!field->get_type()->has_unstructured_annotation(
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
    return struct_->has_unstructured_annotation(
        {"cpp.noncomparable", "cpp2.noncomparable"});
  }
  mstch::node is_eligible_for_constexpr() {
    return is_eligible_for_constexpr_(struct_) ||
        struct_->has_unstructured_annotation({"cpp.methods", "cpp2.methods"});
  }
  mstch::node cpp_virtual() {
    return struct_->has_unstructured_annotation(
        {"cpp.virtual", "cpp2.virtual"});
  }
  mstch::node message() {
    const t_exception* exception = struct_->try_as<t_exception>();
    const t_field* message_field =
        exception == nullptr ? nullptr : exception->get_message_field();
    if (!message_field) {
      return {};
    }
    if (!should_mangle_field_storage_name_in_struct(*struct_)) {
      return message_field->name();
    }
    return mangle_field_name(message_field->name());
  }
  mstch::node cpp_allocator() {
    return struct_->get_unstructured_annotation("cpp.allocator");
  }
  mstch::node cpp_frozen2_exclude() {
    // TODO(dokwon): Fix frozen2 compatibility with adapter.
    return struct_->has_unstructured_annotation("cpp.frozen2_exclude") ||
        struct_->has_structured_annotation(kCppFrozen2ExcludeUri) ||
        cpp_context_->resolver().is_directly_adapted(*struct_);
  }
  mstch::node cpp_allocator_via() {
    if (const auto* name = struct_->find_unstructured_annotation_or_null(
            "cpp.allocator_via")) {
      for (const auto& field : struct_->fields()) {
        if (cpp2::get_name(&field) == *name) {
          return mangle_field_name(*name);
        }
      }
      throw std::runtime_error("No cpp.allocator_via field \"" + *name + "\"");
    }
    return "";
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
    if (struct_->has_structured_annotation(kCppDisableLazyChecksumUri)) {
      return "false";
    }

    return "true";
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
    return size;
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
    if (struct_->is<t_exception>()) {
      return true;
    }
    for (const auto& field : struct_->fields()) {
      const auto* resolved_typedef = field.type()->get_true_type();
      if (cpp2::is_ref(&field) || resolved_typedef->is_string_or_binary() ||
          resolved_typedef->is<t_container>()) {
        return true;
      }
    }
    return false;
  }
  mstch::node has_fatal_annotations() {
    return get_fatal_annotations(struct_->unstructured_annotations()).size() >
        0;
  }
  whisker::object fatal_annotations() {
    return make_whisker_annotations(
        get_fatal_annotations(struct_->unstructured_annotations()));
  }
  mstch::node get_legacy_type_id() {
    return std::to_string(struct_->get_type_id());
  }
  mstch::node legacy_api() { return true; }
  mstch::node metadata_name() {
    return struct_->program()->name() + "_" + struct_->name();
  }

  mstch::node get_num_union_members() {
    if (!struct_->is<t_union>()) {
      throw std::runtime_error("not a union struct");
    }
    return struct_->fields().size();
  }

  template <class Comp>
  mstch::node get_extremal_union_member(Comp comp) {
    if (!struct_->is<t_union>()) {
      throw std::runtime_error("not a union struct");
    }
    auto iter = std::min_element(
        struct_->fields().cbegin(),
        struct_->fields().cend(),
        [&comp](const t_field& a, const t_field& b) {
          return comp(a.id(), b.id());
        });
    if (iter == struct_->fields().cend()) {
      throw std::runtime_error("empty union struct");
    }

    return cpp2::get_name(&*iter);
  }

  mstch::node get_min_union_member() {
    return get_extremal_union_member(std::less<>{});
  }

  mstch::node get_max_union_member() {
    return get_extremal_union_member(std::greater<>{});
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
              !field.has_structured_annotation(kCppDeprecatedTerseWriteUri) &&
              field.qualifier() != t_field_qualifier::optional &&
              field.qualifier() != t_field_qualifier::terse;
        });
  }
  mstch::node has_fields_with_runtime_annotation() {
    const auto& fields = struct_->fields();
    return std::any_of(fields.begin(), fields.end(), has_runtime_annotation);
  }

  mstch::node fields_with_runtime_annotation() {
    return make_mstch_fields(get_fields_with_runtime_annotation());
  }

  const std::vector<const t_field*>& get_fields_with_runtime_annotation() {
    static std::unordered_map<const t_structured*, std::vector<const t_field*>>
        cache;

    auto it = cache.find(struct_);
    if (it != cache.end()) {
      return it->second;
    }

    std::vector<const t_field*> result;
    for (const auto* field : struct_->get_members()) {
      if (has_runtime_annotation(*field)) {
        result.push_back(field);
      }
    }

    return cache.emplace(struct_, std::move(result)).first->second;
  }

  mstch::node structured_runtime_annotations() {
    std::vector<const t_const*> runtime_annotations;
    for (const auto& annotation : struct_->structured_annotations()) {
      if (is_runtime_annotation(*annotation.type())) {
        runtime_annotations.push_back(&annotation);
      }
    }

    return mstch_base::structured_annotations(runtime_annotations);
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
  // Throws exception if cannot compute the alignment .
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

    const t_type* type = field->type()->get_true_type();

    size_t result = type->visit(
        [&](const t_primitive_type& primitive) -> size_t {
          switch (primitive.primitive_type()) {
            case t_primitive_type::type::t_bool:
            case t_primitive_type::type::t_byte:
              return 1;
            case t_primitive_type::type::t_i16:
              return 2;
            case t_primitive_type::type::t_i32:
            case t_primitive_type::type::t_float:
              return 4;
            case t_primitive_type::type::t_i64:
            case t_primitive_type::type::t_double:
            case t_primitive_type::type::t_string:
            case t_primitive_type::type::t_binary:
              return 8;
            default:
              throw std::logic_error(
                  "Computing alignment of unknown primitive type");
          }
        },
        [&](const t_enum& enm) -> size_t { return compute_alignment(enm); },
        [&](const t_container&) -> size_t { return 8; },
        [&](const t_structured& structured) {
          // The type member of a union is an int
          // The __isset member generated in presence of non-required fields of
          // structs/exns only has bool fields so its alignment is 1
          size_t align = structured.is<t_union>() ? 4 : 1;
          for (const auto& field_2 : structured.fields()) {
            size_t field_align = compute_alignment(&field_2, memo);
            align = std::max(align, field_align);
            if (align == kMaxAlign) {
              // No need to continue because the structured already has the
              // maximum alignment.
              break;
            }
          }
          return align;
        },
        [&](const t_service&) -> size_t {
          throw std::logic_error("Computing alignment of service");
        },
        [&](const t_typedef&) -> size_t {
          throw std::logic_error("Unreachable: typedefs resolved above");
        });

    return ret = result;
  }

  static size_t compute_alignment(const t_enum& e) {
    if (const auto* annot =
            e.find_structured_annotation_or_null(kCppEnumTypeUri)) {
      const auto& type = annot->get_value_from_structured_annotation("type");
      switch (static_cast<enum_underlying_type>(type.get_integer())) {
        case enum_underlying_type::i8:
        case enum_underlying_type::u8:
          return 1;
        case enum_underlying_type::i16:
        case enum_underlying_type::u16:
          return 2;
        case enum_underlying_type::u32:
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

    if (!struct_->has_unstructured_annotation("cpp.minimize_padding") &&
        !struct_->has_structured_annotation(kCppMinimizePaddingUri)) {
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
    return make_mstch_fields(struct_->fields_id_order());
  }

  mstch::node any() {
    return struct_->uri() != "" &&
        !struct_->has_unstructured_annotation("cpp.detail.no_any");
  }

  mstch::node is_trivially_destructible() {
    for (const auto& field : struct_->fields()) {
      const t_type* type = field.type()->get_true_type();
      if (cpp2::is_ref(&field) || cpp2::is_custom_type(field) ||
          !is_scalar(*type)) {
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
      std::shared_ptr<cpp2_generator_context> cpp_ctx)
      : mstch_field(f, ctx, pos), cpp_context_(std::move(cpp_ctx)) {
    register_methods(
        this,
        {
            {"field:name_hash", &cpp_mstch_field::name_hash},
            {"field:index_plus_one", &cpp_mstch_field::index_plus_one},
            {"field:ordinal", &cpp_mstch_field::ordinal},
            {"field:has_isset?", &cpp_mstch_field::has_isset},
            {"field:isset_index", &cpp_mstch_field::isset_index},
            {"field:cpp_type", &cpp_mstch_field::cpp_type},
            {"field:cpp_storage_name", &cpp_mstch_field::cpp_storage_name},
            {"field:cpp_storage_type", &cpp_mstch_field::cpp_storage_type},
            {"field:cpp_standard_type", &cpp_mstch_field::cpp_standard_type},
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
            {"field:terse_cpp_ref?", &cpp_mstch_field::terse_cpp_ref},
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
            {"field:deprecated_terse_writes?",
             &cpp_mstch_field::deprecated_terse_writes},
            {"field:deprecated_terse_writes_with_non_redundant_custom_default?",
             &cpp_mstch_field::
                 deprecated_terse_writes_with_non_redundant_custom_default},
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
            {"field:use_op_encode?", &cpp_mstch_field::use_op_encode},
            {"field:fill?", &cpp_mstch_field::fill},
            {"field:structured_runtime_annotations",
             &cpp_mstch_field::structured_runtime_annotations},
        });
  }
  mstch::node name_hash() {
    return "__fbthrift_hash_" + cpp2::sha256_hex(field_->name());
  }
  mstch::node index_plus_one() { return pos_.index + 1; }
  mstch::node ordinal() { return index_plus_one(); }
  mstch::node isset_index() {
    const cpp2_field_generator_context* field_context =
        cpp_context_->get_field_context(field_);
    assert(field_context);
    return field_context->isset_index;
  }
  mstch::node cpp_type() {
    const t_structured* parent = whisker_context().get_field_parent(field_);
    assert(parent != nullptr);
    return cpp_context_->resolver().get_native_type(*field_, *parent);
  }
  mstch::node cpp_storage_name() {
    if (!is_eligible_for_storage_name_mangling()) {
      return cpp2::get_name(field_);
    }

    return mangle_field_name(cpp2::get_name(field_));
  }
  mstch::node cpp_storage_type() {
    const t_structured* parent = whisker_context().get_field_parent(field_);
    assert(parent != nullptr);
    return cpp_context_->resolver().get_storage_type(*field_, *parent);
  }
  mstch::node cpp_standard_type() {
    return cpp_context_->resolver().get_standard_type(*field_);
  }
  mstch::node eligible_for_storage_name_mangling() {
    return is_eligible_for_storage_name_mangling();
  }
  mstch::node has_deprecated_accessors() {
    return !cpp2::is_explicit_ref(field_) && !cpp2::is_lazy(field_) &&
        !cpp_name_resolver::find_first_adapter(*field_) &&
        !cpp_name_resolver::find_field_interceptor(*field_) &&
        !has_option("no_getters_setters") &&
        field_->qualifier() != t_field_qualifier::terse;
  }
  mstch::node cpp_ref() { return cpp2::is_explicit_ref(field_); }
  mstch::node opt_cpp_ref() {
    return cpp2::is_explicit_ref(field_) &&
        field_->qualifier() == t_field_qualifier::optional;
  }
  mstch::node non_opt_cpp_ref() {
    return cpp2::is_explicit_ref(field_) &&
        field_->qualifier() != t_field_qualifier::optional;
  }
  mstch::node terse_cpp_ref() {
    return cpp2::is_explicit_ref(field_) &&
        field_->qualifier() == t_field_qualifier::terse;
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
    // There are downstream plugin code generators which take the address of
    // field accessor functions. They need to be able to accurately determine if
    // an accessor is emitted as a template or regular function, because taking
    // the address of a template function requires that their codegen emit
    // "&f<>" and a non-template function requires them to emit "&f".

    // If the code-generator or template implementation changes which accessors
    // are emitted as templates, is_field_accessor_template must be kept in sync
    return gen::cpp::is_field_accessor_template(*field_);
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
            cpp_name_resolver::find_first_adapter(*field_)) {
      return *adapter;
    }
    return {};
  }
  mstch::node cpp_exactly_one_adapter() {
    bool hasFieldAdapter =
        cpp_name_resolver::find_structured_adapter_annotation(*field_);
    bool hasTypeAdapter =
        cpp_name_resolver::find_first_adapter(*field_->type());
    return hasFieldAdapter != hasTypeAdapter;
  }
  mstch::node cpp_field_interceptor() {
    if (const std::string* interceptor =
            cpp_name_resolver::find_field_interceptor(*field_)) {
      return *interceptor;
    }
    return {};
  }

  // The field accessor is inlined and erased by default, unless 'noinline' is
  // specified in FieldInterceptor.
  mstch::node cpp_accessor_attribute() {
    if (const t_const* annotation = field_->find_structured_annotation_or_null(
            kCppFieldInterceptorUri)) {
      if (annotation->get_value_from_structured_annotation_or_null(
              "noinline")) {
        return "FOLLY_NOINLINE";
      }
    }
    return "FOLLY_ERASE";
  }

  mstch::node cpp_adapter() {
    // Only find a structured adapter on the field.
    if (const std::string* adapter =
            cpp_name_resolver::find_structured_adapter_annotation(*field_)) {
      return *adapter;
    }
    return {};
  }
  mstch::node cpp_noncopyable() {
    return field_->get_type()->has_unstructured_annotation(
        {"cpp.noncopyable", "cpp2.noncopyable"});
  }
  mstch::node serialization_prev_field_key() {
    const cpp2_field_generator_context* field_context =
        cpp_context_->get_field_context(field_);
    assert(field_context && field_context->serialization_prev);
    return field_context->serialization_prev->id();
  }
  mstch::node serialization_next_field_key() {
    const cpp2_field_generator_context* field_context =
        cpp_context_->get_field_context(field_);
    assert(field_context && field_context->serialization_next);
    return field_context->serialization_next->id();
  }
  mstch::node serialization_next_field_type() {
    const cpp2_field_generator_context* field_context =
        cpp_context_->get_field_context(field_);
    assert(field_context && field_context->serialization_next);
    return field_context->serialization_next
        ? context_.type_factory->make_mstch_object(
              field_context->serialization_next->get_type(), context_, pos_)
        : mstch::node("");
  }
  mstch::node deprecated_terse_writes() {
    return field_->has_structured_annotation(kCppDeprecatedTerseWriteUri) ||
        (has_option("deprecated_terse_writes") &&
         cpp2::deprecated_terse_writes(field_));
  }
  mstch::node deprecated_terse_writes_with_non_redundant_custom_default() {
    return std::get<bool>(deprecated_terse_writes()) &&
        field_->default_value() &&
        !detail::is_initializer_default_value(
               field_->type().deref(), *field_->default_value());
  }
  mstch::node zero_copy_arg() {
    return zero_copy_arg_impl(*field_->type()) ? "true" : "false";
  }
  mstch::node has_fatal_annotations() {
    return get_fatal_annotations(field_->unstructured_annotations()).size() > 0;
  }
  mstch::node has_isset() { return cpp2::field_has_isset(field_); }
  whisker::object fatal_annotations() {
    return make_whisker_annotations(
        get_fatal_annotations(field_->unstructured_annotations()));
  }
  mstch::node fatal_required_qualifier() {
    switch (field_->qualifier()) {
      case t_field_qualifier::required:
        return "required";
      case t_field_qualifier::optional:
        return "optional";
      case t_field_qualifier::none:
        return "required_of_writer";
      case t_field_qualifier::terse:
        return "terse";
    }
    throw std::runtime_error("unknown required qualifier");
  }

  mstch::node visibility() { return is_private() ? "private" : "public"; }

  mstch::node metadata_name() {
    auto key = field_->id();
    auto suffix = key >= 0 ? std::to_string(key) : "_" + std::to_string(-key);
    return field_->name() + "_" + suffix;
  }

  mstch::node type_tag() {
    const t_structured* parent = whisker_context().get_field_parent(field_);
    assert(parent != nullptr);
    return cpp_context_->resolver().get_type_tag(*field_, *parent);
  }

  mstch::node raw_binary() {
    return field_->type()->get_true_type()->is_binary() &&
        !cpp_name_resolver::find_first_adapter(*field_);
  }

  mstch::node raw_string_or_binary() {
    return field_->type()->get_true_type()->is_string_or_binary() &&
        !cpp_name_resolver::find_first_adapter(*field_);
  }

  mstch::node use_op_encode() {
    const t_structured* parent = whisker_context().get_field_parent(field_);
    assert(parent != nullptr);
    return needs_op_encode(*field_, *parent);
  }

  // Not optional, terse, or deprecated terse.
  mstch::node fill() {
    return (field_->qualifier() == t_field_qualifier::none ||
            field_->qualifier() == t_field_qualifier::required) &&
        !std::get<bool>(deprecated_terse_writes());
  }

  mstch::node structured_runtime_annotations() {
    std::vector<const t_const*> runtime_annotations;
    for (const auto& annotation : field_->structured_annotations()) {
      if (is_runtime_annotation(*annotation.type())) {
        runtime_annotations.push_back(&annotation);
      }
    }

    return mstch_base::structured_annotations(runtime_annotations);
  }

 private:
  bool zero_copy_arg_impl(const t_type& type) {
    const auto& true_type = *type.get_true_type();
    if (true_type.is_binary() || true_type.is<t_structured>()) {
      return true;
    } else if (const t_list* list = true_type.try_as<t_list>()) {
      return zero_copy_arg_impl(*list->elem_type());
    } else if (const t_set* set = true_type.try_as<t_set>()) {
      return zero_copy_arg_impl(*set->elem_type());
    } else if (const t_map* map = true_type.try_as<t_map>()) {
      return zero_copy_arg_impl(map->key_type().deref()) ||
          zero_copy_arg_impl(map->val_type().deref());
    }
    return false;
  }

  bool is_private() const {
    auto req = field_->qualifier();
    bool isPrivate = true;
    if (cpp2::is_lazy(field_)) {
      // Lazy field has to be private.
    } else if (cpp2::is_ref(field_)) {
      // cpp.ref field is always private
    } else if (req == t_field_qualifier::required) {
      isPrivate = !has_option("deprecated_public_required_fields");
    }
    return isPrivate;
  }

  bool is_eligible_for_storage_name_mangling() const {
    const t_structured* strct = whisker_context().get_field_parent(field_);
    assert(strct != nullptr);

    if (strct->is<t_union>()) {
      return false;
    }

    if (!should_mangle_field_storage_name_in_struct(*strct)) {
      return false;
    }

    return is_private();
  }

  std::shared_ptr<cpp2_generator_context> cpp_context_;
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
            {"constant:cpp_adapter", &cpp_mstch_const::cpp_adapter},
            {"constant:cpp_type", &cpp_mstch_const::cpp_type},
            {"constant:has_extra_arg?", &cpp_mstch_const::has_extra_arg},
            {"constant:extra_arg", &cpp_mstch_const::extra_arg},
            {"constant:extra_arg_type", &cpp_mstch_const::extra_arg_type},
            {"constant:outline_init?", &cpp_mstch_const::outline_init},
        });
  }
  mstch::node enum_value() {
    if (const_->type()->is<t_enum>()) {
      const auto* enm = static_cast<const t_enum*>(const_->type());
      const auto* enum_val = enm->find_value(const_->value()->get_integer());
      if (enum_val) {
        return enum_val->name();
      } else {
        return std::to_string(const_->value()->get_integer());
      }
    }
    return mstch::node();
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
    return is_empty_container() &&
        !cpp_name_resolver::find_first_adapter(
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

  bool is_empty_container() const {
    return (const_value_->kind() == cv::CV_MAP &&
            const_value_->get_map().empty()) ||
        (const_value_->kind() == cv::CV_LIST &&
         const_value_->get_list().empty());
  }
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
  mstch_context_.add<cpp_mstch_program>(std::ref(source_mgr_));
  mstch_context_.add<cpp_mstch_service>(std::ref(source_mgr_));
  mstch_context_.add<cpp_mstch_interaction>(std::ref(source_mgr_));
  mstch_context_.add<cpp_mstch_function>(cpp_context_);
  mstch_context_.add<cpp_mstch_type>(cpp_context_);
  mstch_context_.add<cpp_mstch_struct>(cpp_context_);
  mstch_context_.add<cpp_mstch_field>(cpp_context_);
  mstch_context_.add<cpp_mstch_const>(cpp_context_);
  mstch_context_.add<cpp_mstch_const_value>();
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
          source_mgr_,
          split_id,
          shards.at(split_id));
      render_to_file(
          std::shared_ptr<mstch_base>(split_program),
          "module_types.cpp",
          name + "_types." + s + ".split.cpp");
      render_to_file(
          std::shared_ptr<mstch_base>(split_program),
          "module_types_binary.cpp",
          name + "_types_binary." + s + ".split.cpp");
      render_to_file(
          std::shared_ptr<mstch_base>(split_program),
          "module_types_compact.cpp",
          name + "_types_compact." + s + ".split.cpp");
      render_to_file(
          std::shared_ptr<mstch_base>(split_program),
          "module_types_serialization.cpp",
          name + "_types_serialization." + s + ".split.cpp");
    }
  } else {
    render_to_file(prog, "module_types.cpp", name + "_types.cpp");
    render_to_file(prog, "module_types_binary.cpp", name + "_types_binary.cpp");
    render_to_file(
        prog, "module_types_compact.cpp", name + "_types_compact.cpp");
    render_to_file(
        prog,
        "module_types_serialization.cpp",
        name + "_types_serialization.cpp");
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
  const auto& name = service->name();
  auto mstch_service =
      make_mstch_service_cached(get_program(), service, mstch_context_);

  mstch::map context = {
      {"program", cached_program(get_program())},
      {"service", mstch_service},
  };

  render_to_file(mstch_service, "ServiceAsyncClient.h", name + "AsyncClient.h");
  render_to_file(context, "service.cpp", name + ".cpp");
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
          source_mgr_,
          nullptr,
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

  for (const char* protocol : {"binary", "compact"}) {
    render_to_file(
        mstch_service,
        "service_processmap_protocol.cpp",
        name + "_processmap_" + protocol + ".cpp");
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
    mstch_services.emplace_back(
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
    mstch_services.emplace_back(
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
  auto has_method_decorator = std::any_of(
      services.cbegin(), services.cend(), [&](const t_service* service) {
        return service->has_structured_annotation(
            apache::thrift::compiler::kCppGenerateServiceMethodDecorator);
      });

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
      {"any_method_decorators?", has_method_decorator},
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

mstch::array t_mstch_cpp2_generator::cpp_includes(const t_program* program) {
  mstch::array a;
  if (program->language_includes().count("cpp")) {
    for (auto include : program->language_includes().at("cpp")) {
      mstch::map cpp_include;
      if (include.at(0) != '<') {
        include = fmt::format("\"{}\"", include);
      }
      cpp_include.emplace("cpp_include", std::move(include));
      a.emplace_back(std::move(cpp_include));
    }
  }
  return a;
}

mstch::node t_mstch_cpp2_generator::include_prefix(
    const t_program* program, compiler_options_map& options) {
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
  if (std::filesystem::path(prefix).has_root_directory()) {
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
  for (const auto& kv : split(map, ',')) {
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
    sema_context& ctx,
    const t_structured& s,
    const t_mstch_generator::compiler_options_map& options) {
  if (cpp2::packed_isset(s)) {
    if (options.count("tablebased") != 0) {
      ctx.report(
          s,
          "tablebased-isset-bitpacking-rule",
          diagnostic_level::error,
          "Tablebased serialization is incompatible with isset bitpacking for struct `{}`",
          s.name());
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

  void operator()(sema_context& ctx, const t_program& program) {
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
      const int32_t object_count, sema_context& ctx, const t_program& program) {
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
      const std::vector<t_service*>& services, sema_context& ctx) {
    if (client_name_to_split_count_.empty()) {
      return;
    }
    for (const t_service* s : services) {
      auto iter = client_name_to_split_count_.find(s->name());
      if (iter != client_name_to_split_count_.end() &&
          iter->second > static_cast<int32_t>(s->get_functions().size())) {
        ctx.report(
            *s,
            "more-splits-than-functions-rule",
            diagnostic_level::error,
            "`client_cpp_splits={}` (For service {}) is misconfigured: it "
            "can not be greater than the number of functions, which is {}.",
            iter->second,
            s->name(),
            s->get_functions().size());
      }
    }
  }
};

void forbid_deprecated_terse_writes_ref(
    sema_context& ctx,
    const t_structured& strct,
    const t_mstch_generator::compiler_options_map& options) {
  for (auto& field : strct.fields()) {
    const bool isUniqueRef =
        gen::cpp::find_ref_type(field) == gen::cpp::reference_type::unique;
    const bool isDeprecatedTerseWrites =
        field.qualifier() == t_field_qualifier::none &&
        (options.count("deprecated_terse_writes") ||
         field.has_structured_annotation(kCppDeprecatedTerseWriteUri));

    if (field.has_structured_annotation(
            kCppAllowLegacyDeprecatedTerseWritesRefUri)) {
      if (!isUniqueRef) {
        ctx.report(
            field,
            diagnostic_level::error,
            "@cpp.AllowLegacyDeprecatedTerseWritesRef can not be applied to `{}`"
            " since it's not cpp.Ref{{Unique}} field.",
            field.name());
      }
      if (!isDeprecatedTerseWrites) {
        ctx.report(
            field,
            diagnostic_level::error,
            "@cpp.AllowLegacyDeprecatedTerseWritesRef can not be applied to `{}`"
            " since it's not cpp.DeprecatedTerseWrite field.",
            field.name());
      }
      continue;
    }

    if (!isUniqueRef || !isDeprecatedTerseWrites) {
      continue;
    }

    ctx.report(
        field,
        diagnostic_level::error,
        "@cpp.Ref{{Unique}} can not be applied to `{}`"
        " since it's cpp.DeprecatedTerseWrite field.",
        field.name());
  }
}

void validate_lazy_fields(sema_context& ctx, const t_field& field) {
  if (cpp2::is_lazy(&field)) {
    auto t = field.type()->get_true_type();
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
          field.name());
    }
  }
}

// TODO(dokwon): Remove this validation once `deprecated_terse_writes` cpp2
// options are completely removed.
void validate_deprecated_terse_writes(
    sema_context& ctx,
    const t_field& field,
    const t_mstch_generator::compiler_options_map& options) {
  if (options.count("deprecated_terse_writes") != 0 &&
      field.has_structured_annotation(kCppDeprecatedTerseWriteUri)) {
    ctx.error(
        "Cannot use thrift_cpp2_options `deprecated_terse_writes` with @cpp.DeprecatedTerseWrite.");
  }
}

void t_mstch_cpp2_generator::fill_validator_visitors(
    ast_validator& validator) const {
  validator.add_structured_definition_visitor(std::bind(
      validate_struct_annotations,
      std::placeholders::_1,
      std::placeholders::_2,
      options()));
  validator.add_struct_visitor(std::bind(
      forbid_deprecated_terse_writes_ref,
      std::placeholders::_1,
      std::placeholders::_2,
      options()));
  validator.add_program_visitor(
      validate_splits(get_split_count(options()), client_name_to_split_count_));
  validator.add_field_visitor(validate_lazy_fields);
  validator.add_field_visitor(std::bind(
      validate_deprecated_terse_writes,
      std::placeholders::_1,
      std::placeholders::_2,
      options()));
}

THRIFT_REGISTER_GENERATOR(
    mstch_cpp2, "cpp2", R"(    (NOTE: the list below may not be exhaustive)
    any
      Register types with the AnyRegistry.
    client_cpp_splits={[<service name:str>:<split count:int>[,...]*]}
      Enable splitting of client method .cpp files (into N
      *.async_client_split.cpp" files). The given split count cannot be greater
      than the number of methods in the corresponding service. See also
      types_cpp_splits below.
    deprecated_clear
      Use the deprecated semantics for "clearing" Thrift structs, which assigns
      the *standard* default value instead of the *intrinsic* defaults (see
      https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#default-values).
    deprecated_enforce_required
      Enforce required fields (deprecated since 2019).
    deprecated_public_required_fields
      Make member variables corresponding to required fields public instead of
      private. In addition to exposing directly the field (which is unsafe to
      begin with), this prevents the generation of the reference accessors
      that do not have the _ref() suffix.
    deprecated_terse_writes
      Enable deprecated terse writes, which are discouraged in favor of
      @thrift.TerseWrite. See:
      https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/field-qualifiers.md#terse-writes-compiler-option
    disable_custom_type_ordering_if_structure_has_uri
      Without this option, custom set/map are considered orderable if parent structure has uri.
    frozen[=packed]
      Enable frozen structs. If the packed parameter is given, structure members
      will be packed with an alignment of 1 (i.e., #pragma pack(push, 1)).
      NOTE: this capability is not actively maintained. Use at your own risks.
    frozen2
      Enable frozen2 (see https://fburl.com/thrift_frozen2).
      NOTE: this capability is not actively maintained. Use at your own risks.
    includes=<extra_include:str>:...
      Add cpp_include for each of the given values.
    include_prefix
      Override the "include prefix" for all generated files, i.e. the directory
      from which application code should include headers, typically:
      <include_prefix>/gen-cpp2/...
    json
      Enable SimpleJson serialization.
    no_getters_setters
      Do not generate (deprecated) field getter and setter methods, even when
      it would be possible to do so. This is enouraged, and eventually will be
      enabled by default as getters and setters are deprecated in favor of field
      references (i.e., field() or field_ref() methods). Other conditions that
      would prevent getters/setters from being generated (even if this option is
      not enabled) include if the corresponding field: is a reference field
      (@cpp.Ref, cpp[2].ref_[type]), is adapted (@cpp.Adapter), is lazy
      (@cpp.Lazy), has a @cpp.FieldIntercaptor or is terse.
    no_metadata
      Generate empty metadata, do not generate _metadata.cpp.
    py3cpp
      if specified, output folder is "gen-py3cpp" instead of "gen-cpp2".
    reflection
      Enable the generation of "old-type" (a.k.a. "fatal") reflection for Thrift
      types. This is deprecated in favor of always-on reflection (see
      https://fburl.com/thrift-cpp-reflection). Note: the name "fatal" comes
      from the name of the Facebook Template Library, see:
      https://github.com/facebook/fatal/blob/main/README.md
    single_file_service
      Generate all RPC services and client code in a single file, respectively.
    sync_methods_return_try
      Generate (deprecated) sync code for RPC methods that returns a folly::Try.
    tablebased
      Enable the table-based serialization.
    types_cpp_splits=<split_count:int>
      Enable splitting of type .cpp files (into the given number of files).
      Cannot be greater than the number of objects. See also client_cpp_splits
      above.
)"

);

} // namespace
} // namespace apache::thrift::compiler
