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

#include <cstdlib>
#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/t_concat_generator.h>

#include <fmt/ranges.h>

namespace apache::thrift::compiler {
namespace {

class t_name_generator {
 public:
  std::string operator()(const char* prefix) {
    return prefix + std::to_string(counter_++);
  }

  void decrement_counter() {
    if (counter_ == 0) {
      throw std::runtime_error(
          "Name generator error, counter value is already 0");
    }
    counter_--;
  }

 private:
  int counter_ = 0;
};

std::string unescape(std::string s) {
  boost::replace_all(s, "\\\\", "\\");
  return s;
}

class t_result_struct final : public t_structured {
 public:
  t_result_struct(
      t_program* program, std::string name, std::string result_return_type)
      : t_structured(program, std::move(name)),
        result_return_type{std::move(result_return_type)} {}

  std::string getResultReturnType() const { return result_return_type; }

  // Both get_type_value() and is_struct_or_union() are implemented below for
  // historical reasons: t_result_struct used to be a subclass of t_struct. It
  // was moved to this anonymous namespace because it is only used in this
  // file, and made to inherit from t_structured (as we are decoupling all
  // other types from t_struct).
  t_type::type get_type_value() const override {
    return t_type::type::t_structured;
  }

 private:
  std::string result_return_type;
};

// Returns true iff type can be used as a set or map key in Hack.
bool is_type_arraykey(const t_type& type) {
  const t_type* true_type = type.get_true_type();
  return true_type->is_string_or_binary() || true_type->is_any_int() ||
      true_type->is_byte() || true_type->is<t_enum>();
}

/**
 * Hack code generator.
 */
class t_hack_generator : public t_concat_generator {
 public:
  using t_concat_generator::t_concat_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    json_ = option_is_specified(options, "json");
    phps_ = option_is_specified(options, "server");
    strict_types_ = option_is_specified(options, "stricttypes");
    auto explicit_arraysets = option_is_specified(options, "arraysets");
    no_nullables_ = option_is_specified(options, "nonullables");
    from_map_construct_ = option_is_specified(options, "frommap_construct");
    shapes_ = option_is_specified(options, "shapes");
    shape_arraykeys_ = option_is_specified(options, "shape_arraykeys");
    shapes_allow_unknown_fields_ =
        option_is_specified(options, "shapes_allow_unknown_fields");
    array_migration_ = option_is_specified(options, "array_migration");
    legacy_arrays_ = option_is_specified(options, "legacy_arrays");
    auto explicit_hack_collections =
        option_is_specified(options, "hack_collections");

    nullable_everything_ = option_is_specified(options, "nullable_everything");
    const_collections_ = option_is_specified(options, "const_collections");
    enum_extratype_ = option_is_specified(options, "enum_extratype");
    enum_transparenttype_ =
        option_is_specified(options, "enum_transparenttype");
    soft_attribute_ = option_is_specified(options, "soft_attribute");
    strict_unions_ = option_is_specified(options, "strict_unions");
    protected_unions_ =
        option_is_specified(options, "protected_unions") || strict_unions_;
    legacy_union_json_serialization_ =
        option_is_specified(options, "legacy_union_json_serialization");
    mangled_services_ = option_is_set(options, "mangledsvcs", false);
    typedef_ = option_is_specified(options, "typedef");
    server_stream_ = option_is_specified(options, "server_stream");

    union_logger_rollout_ =
        option_is_specified(options, "__union_logger_rollout");

    auto [_, ns_type_] = get_namespace(program_);
    has_hack_namespace = ns_type_ == HackThriftNamespaceType::HACK ||
        ns_type_ == HackThriftNamespaceType::PACKAGE;
    has_nested_ns = false;

    if (legacy_arrays_ && explicit_arraysets) {
      throw std::runtime_error(
          "Don't use arraysets with legacy_arrays, because legacy_arrays implies arraysets.");
    }
    arraysets_ = explicit_arraysets || legacy_arrays_;

    if (const_collections_ && explicit_hack_collections) {
      throw std::runtime_error(
          "Don't use hack_collections with const_collections, because const_collections implies hack_collections.");
    }
    hack_collections_ = explicit_hack_collections || const_collections_;

    // legacy_arrays_ is only used to migrate away from php gen
    if (legacy_arrays_ && strict_types_) {
      throw std::runtime_error("Don't use legacy_arrays with strict_types");
    } else if (!legacy_arrays_ && arraysets_ && !hack_collections_) {
      throw std::runtime_error("Don't use arraysets without hack_collections");
    } else if (mangled_services_ && has_hack_namespace) {
      throw std::runtime_error(
          "Don't use mangledsvcs with hack namespaces or package.");
    }

    out_dir_base_ = "gen-hack";
  }

  /**
   * Init and close methods
   */

  void init_generator() override;
  void close_generator() override;
  void init_codegen_file(
      std::ofstream& file_,
      const std::string& file_name_,
      bool skip_ns = false);

  /**
   * Program-level generation functions
   */

  void generate_typedef(const t_typedef* ttypedef) override;
  void generate_enum(const t_enum* tenum) override;
  void generate_const(const t_const* tconst) override;
  void generate_struct(const t_structured* tstruct) override;
  void generate_xception(const t_structured* txception) override;
  void generate_service(const t_service* tservice) override;

  std::string render_const_value(
      const t_type* type,
      const t_const_value* value,
      bool immutable_collections,
      bool ignore_wrapper,
      bool force_arrays);
  std::string render_const_value_helper(
      const t_type* type,
      const t_const_value* value,
      std::ostream& temp_var_initializations_out,
      t_name_generator& namer,
      bool immutable_collections,
      bool ignore_wrapper,
      bool structured_annotations,
      bool force_arrays,
      bool exclude_from_fixtures = false);
  std::string render_default_value(const t_type* type);

  /**
   * Metadata Types functions
   */

  /**
   * Keep synced with : thrift/lib/thrift/metadata.thrift
   */
  enum ThriftPrimitiveType {
    THRIFT_BOOL_TYPE = 1,
    THRIFT_BYTE_TYPE = 2,
    THRIFT_I16_TYPE = 3,
    THRIFT_I32_TYPE = 4,
    THRIFT_I64_TYPE = 5,
    THRIFT_FLOAT_TYPE = 6,
    THRIFT_DOUBLE_TYPE = 7,
    THRIFT_BINARY_TYPE = 8,
    THRIFT_STRING_TYPE = 9,
    THRIFT_VOID_TYPE = 10,
  };

  ThriftPrimitiveType base_to_t_primitive(const t_primitive_type* base_type);

  std::unique_ptr<t_const_value> type_to_tmeta(const t_type* type);
  std::unique_ptr<t_const_value> field_to_tmeta(const t_field* field);
  std::unique_ptr<t_const_value> function_to_tmeta(const t_function* function);
  std::unique_ptr<t_const_value> service_to_tmeta(const t_service* service);
  std::unique_ptr<t_const_value> enum_to_tmeta(const t_enum* tenum);
  std::unique_ptr<t_const_value> struct_to_tmeta(
      const t_structured* tstruct, bool is_exception);

  void append_to_t_enum(
      t_enum* tenum, t_program* program, ThriftPrimitiveType value);

  const t_type* tmeta_ThriftType_type();
  const t_type* tmeta_ThriftField_type();
  const t_type* tmeta_ThriftFunction_type();
  const t_type* tmeta_ThriftService_type();
  const t_type* tmeta_ThriftEnum_type();
  const t_type* tmeta_ThriftStruct_type();
  const t_type* tmeta_ThriftException_type();

  /**
   * Structs!
   */

  enum class ThriftStructType {
    STRUCT,
    EXCEPTION,
    ARGS,
    RESULT,
  };

  enum class ThriftShapishStructType {
    SYNC = 1,
    ASYNC = 2,
    VISITED = 3,
  };

  enum class ThriftAsyncStructCreationMethod {
    FROM_CONSTRUCTOR_SHAPE = 1,
    FROM_MAP = 2,
    FROM_SHAPE = 3,
  };

  enum class HackThriftNamespaceType {
    HACK,
    PHP,
    PACKAGE,
    EMPTY,
  };

  bool is_async_struct(const t_structured* tstruct);

  // Only use this to determine if struct uses IThriftShapishAsyncStruct
  bool is_async_shapish_struct(const t_structured* tstruct);
  bool is_async_type(const t_type* type, bool check_nested_structs);
  bool is_async_field(const t_field& field, bool check_nested_structs);
  bool has_clear_terse_fields(const t_structured* tstruct);

  void generate_php_struct_definition(
      std::ofstream& out,
      const t_structured* tstruct,
      ThriftStructType type = ThriftStructType::STRUCT,
      const std::string& name = "");
  void _generate_php_struct_definition(
      std::ofstream& out,
      const t_structured* tstruct,
      ThriftStructType type,
      const std::string& name);
  void generate_php_function_result_helpers(
      const t_function* tfunction,
      const t_type* ttype,
      const t_throws* ex,
      const std::string& prefix,
      const std::string& suffix,
      bool is_void);
  void generate_php_struct_definition_result_helpers(
      t_structured* result,
      const t_type* ttype,
      const t_throws* ex,
      bool is_void);
  void generate_php_function_args_helpers(
      const t_function* tfunction, const std::string& prefix);
  void generate_php_function_helpers(
      const t_service* tservice, const t_function* tfunction);
  void generate_php_stream_function_helpers(
      const t_function* tfunction, const std::string& prefix);
  void generate_php_sink_function_helpers(
      const t_function* tfunction, const std::string& prefix);
  void generate_php_interaction_function_helpers(
      const t_service* tservice,
      const t_service* interaction,
      const t_function* tfunction);

  void generate_php_union_enum(
      std::ofstream& out, const t_structured* tstruct, const std::string& name);
  void generate_php_union_methods(
      std::ofstream& out,
      const t_structured* tstruct,
      const std::string& struct_hack_name_with_ns);
  void generate_php_struct_fields(
      std::ofstream& out,
      const t_structured* tstruct,
      const std::string& struct_hack_name_with_ns,
      ThriftStructType type = ThriftStructType::STRUCT);

  void generate_php_struct_field_methods(
      std::ofstream& out, const t_field* field, bool is_exception);
  void generate_php_field_wrapper_methods(
      std::ofstream& out,
      const t_field& field,
      bool is_union,
      bool nullable,
      const std::string& struct_class_name);
  void generate_php_struct_methods(
      std::ofstream& out,
      const t_structured* tstruct,
      ThriftStructType type,
      const std::string& struct_hack_name,
      bool is_async_struct,
      bool is_async_shapish_struct,
      bool add_clear_terse_fields_interface,
      const std::string& struct_hack_name_with_ns);
  void generate_php_struct_constructor(
      std::ofstream& out,
      const t_structured* tstruct,
      ThriftStructType type,
      const std::string& struct_hack_name_with_ns);
  void generate_php_struct_default_constructor(
      std::ofstream& out,
      const t_structured* tstruct,
      ThriftStructType type,
      const std::string& struct_hack_name_with_ns);
  void generate_php_struct_withDefaultValues_method(std::ofstream& out);

  void generate_php_struct_clear_terse_fields(
      std::ofstream& out,
      const t_structured* tstruct,
      ThriftStructType type,
      const std::string& struct_hack_name_with_ns);
  void generate_php_struct_constructor_field_assignment(
      std::ofstream& out,
      const t_field& field,
      const t_structured* tstruct,
      ThriftStructType type,
      const std::string& name = "",
      bool is_default_assignment = false,
      bool skip_custom_default = false,
      bool first_field = false);
  void generate_php_struct_metadata_method(
      std::ofstream& out, const t_structured* tstruct);
  void generate_php_struct_structured_annotations_method(
      std::ofstream& out, const t_structured* tstruct);
  void generate_php_struct_shape_spec(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_constructor_shape = false);
  void generate_php_struct_shape_collection_value_lambda(
      std::ostream& out, t_name_generator& namer, const t_type* t);
  void generate_hack_array_from_shape_lambda(
      std::ostream& out, t_name_generator& namer, const t_type* t);
  void generate_hack_array_from_shape_lambda(
      std::ostream& out, t_name_generator& namer, const t_map* t);
  void generate_hack_array_from_shape_lambda(
      std::ostream& out, t_name_generator& namer, const t_list* t);
  void generate_hack_array_from_shape_lambda(
      std::ostream& out, t_name_generator& namer, const t_set* t);
  void generate_hack_array_from_shape_lambda(
      std::ostream& out, t_name_generator& namer, const t_structured* t);
  void generate_shape_from_hack_array_lambda(
      std::ostream& out, t_name_generator& namer, const t_type* t);
  void generate_php_struct_from_shape(
      std::ofstream& out, const t_structured* tstruct);
  void generate_php_struct_from_map(
      std::ofstream& out, const t_structured* tstruct);
  void generate_php_struct_async_struct_creation_method(
      std::ofstream& out,
      const t_structured* tstruct,
      const std::string& struct_hack_name_with_ns,
      ThriftAsyncStructCreationMethod method_type);
  void generate_php_struct_async_struct_creation_method_header(
      std::ofstream& out, ThriftAsyncStructCreationMethod method_type);
  void generate_php_struct_async_struct_creation_method_footer(
      std::ofstream& out);
  void generate_php_struct_async_struct_creation_method_field_assignment(
      std::ofstream& out,
      const t_structured* tstruct,
      const t_field& tfield,
      const std::string& field_ref,
      const std::string& struct_hack_name_with_ns,
      t_name_generator& namer,
      bool is_shape = false,
      bool uses_thrift_only_methods = false,
      const std::string& obj_ref = "$obj");
  bool generate_php_struct_async_struct_creation_method_field_assignment_helper(
      std::ostream& out,
      const t_type* ttype,
      t_name_generator& namer,
      const std::string& val,
      bool is_shape_method,
      bool uses_thrift_only_methods = false);

  bool type_has_nested_struct(const t_type* t);
  bool field_is_nullable(const t_structured* tstruct, const t_field* field);
  void generate_php_struct_shape_methods(
      std::ofstream& out, const t_structured* tstruct);
  void generate_php_struct_stringifyMapKeys_method(std::ofstream& out);

  void generate_php_struct_async_shape_methods(
      std::ofstream& out,
      const t_structured* tstruct,
      const std::string& struct_hack_name_with_ns);
  bool generate_php_struct_async_toShape_method_helper(
      std::ostream& out,
      const t_type* ttype,
      t_name_generator& namer,
      const std::string& val);

  void generate_hack_attributes(
      std::ofstream& out, const t_named* type, bool include_user_defined);
  void generate_adapter_type_checks(
      std::ofstream& out, const t_structured* tstruct);

  void generate_php_type_spec(
      std::ofstream& out, const t_type* t, uint32_t depth);
  void generate_php_type_spec_shape_elt_helper(
      std::ofstream& out,
      const std::string& field_name,
      const t_type* t,
      uint32_t depth);
  void generate_php_struct_spec(
      std::ofstream& out, const t_structured* tstruct);
  void generate_php_struct_struct_trait(
      std::ofstream& out,
      const t_structured* tstruct,
      const std::string& struct_hack_ref);
  void generate_php_structural_id(
      std::ofstream& out, const t_structured* tstruct, bool asFunction);
  bool skip_codegen(const t_field* field);
  bool skip_codegen(const t_function* function);

  bool is_valid_hack_type(const t_type* type, const t_type_ref& top_level_type);
  bool is_valid_hack_type(const t_type_ref& type) {
    return is_valid_hack_type(&*type, type);
  }

  /**
   * Service-level generation functions
   */

  void _generate_args(
      std::ofstream& out,
      const t_service* tservice,
      const t_function* tfunction);
  void _generate_current_seq_id(
      std::ofstream& out,
      const t_service* tservice,
      const t_function* tfunction);
  void _generate_sendImplHelper(
      std::ofstream& out,
      const t_function* tfunction,
      const t_service* tservice);
  void generate_service(const t_service* tservice, bool mangle);
  void generate_service_helpers(const t_service* tservice, bool mangle);
  void generate_service_interactions(const t_service* tservice, bool mangle);
  void generate_service_interface(
      const t_service* tservice, bool mangle, bool async, bool client);
  void generate_service_client(const t_service* tservice, bool mangle);
  void _generate_service_client(
      std::ofstream& out, const t_service* tservice, bool mangle);
  void _generate_sendImpl(
      std::ofstream& out,
      const t_service* tservice,
      const t_function* tfunction);
  void _generate_sendImpl_arg(
      std::ofstream& out,
      t_name_generator& namer,
      const std::string& var,
      const t_type* t);
  void _generate_service_client_children(
      std::ofstream& out, const t_service* tservice, bool mangle, bool async);
  void _generate_service_client_child_fn(
      std::ofstream& out,
      const t_service* tservice,
      const t_function* tfunction,
      bool legacy_arrays = false);
  void _generate_service_client_stream_child_fn(
      std::ofstream& out,
      const t_service* tservice,
      const t_function* tfunction,
      bool legacy_arrays = false);
  void _generate_service_client_sink_child_fn(
      std::ofstream& out,
      const t_service* tservice,
      const t_function* tfunction,
      bool legacy_arrays = false);
  void generate_service_processor(const t_service* tservice, bool mangle);
  void generate_process_function(
      const t_service* tservice, const t_function* tfunction, bool async);
  void generate_process_metadata_function(
      const t_service* tservice, bool mangle);

  /**
   * Read thrift object from JSON string, generated using the
   * TSimpleJSONProtocol.
   */

  void generate_json_enum(
      std::ofstream& out,
      t_name_generator& namer,
      const t_enum* tenum,
      const std::string& prefix_thrift,
      const std::string& prefix_json);

  void generate_json_struct(
      std::ofstream& out,
      t_name_generator& namer,
      const t_structured* tstruct,
      const std::string& prefix_thrift,
      const std::string& prefix_json);

  void generate_json_field(
      std::ofstream& out,
      t_name_generator& namer,
      const t_field* tfield,
      const std::string& prefix_thrift = "",
      const std::string& suffix_thrift = "",
      const std::string& prefix_json = "");

  void generate_json_container(
      std::ofstream& out,
      t_name_generator& namer,
      const t_type* ttype,
      const std::string& prefix_thrift = "",
      const std::string& prefix_json = "");

  void generate_json_set_element(
      std::ofstream& out,
      t_name_generator& namer,
      const t_set* tset,
      const std::string& value,
      const std::string& prefix_thrift);

  void generate_json_list_element(
      std::ofstream& out,
      t_name_generator& namer,
      const t_list* list,
      const std::string& value,
      const std::string& prefix_thrift);

  void generate_json_map_element(
      std::ofstream& out,
      t_name_generator& namer,
      const t_map* tmap,
      const std::string& key,
      const std::string& value,
      const std::string& prefix_thrift);

  void generate_json_reader(std::ofstream& out, const t_structured* tstruct);

  void generate_instance_key(std::ofstream& out);

  void generate_exception_method(
      std::ofstream& out, const t_structured* tstruct);

  /**
   * Helper rendering functions
   */

  enum class PhpFunctionNameSuffix {
    ARGS = 0,
    RESULT = 1,
    STREAM_RESPONSE = 2,
    FIRST_RESPONSE = 3,
    SINK_PAYLOAD = 4,
    SINK_FINAL_RESPONSE = 5,
  };

  enum class TypeToTypehintVariations {
    IS_SHAPE = 1,
    IS_ANY_SHAPE = 2,
    IMMUTABLE_COLLECTIONS = 3,
    IGNORE_WRAPPER = 6,
    RECURSIVE_IGNORE_WRAPPER = 7,
    IGNORE_TYPEDEF_OPTION = 8,
  };

  std::string declare_field(
      const t_field* tfield,
      bool init = false,
      bool obj = false,
      bool thrift = false);
  std::string function_signature(
      const t_function* tfunction,
      const std::string& more_tail_parameters = "",
      std::string typehint = "");
  std::string argument_list(
      const t_paramlist& tparamlist,
      const std::string& more_tail_parameters = "",
      bool typehints = true,
      bool force_nullable = false);
  std::string generate_rpc_function_name(
      const t_service* tservice, const t_function* tfunction) const;
  std::string generate_function_helper_name(
      const t_service* tservice,
      const t_function* tfunction,
      PhpFunctionNameSuffix suffix);
  std::string type_to_enum(const t_type* ttype);

  static std::string render_string(const std::string& value);

  std::string field_to_typehint(
      const t_field& tfield,
      const std::string& struct_class_name,
      bool is_field_nullable = false);
  std::string get_stream_function_return_typehint(const t_function* function);
  std::string get_sink_function_return_typehint(const t_function* function);
  std::string get_container_keyword(
      const t_type* ttype, std::map<TypeToTypehintVariations, bool> variations);
  std::string type_to_typehint(
      const t_type* ttype,
      std::map<TypeToTypehintVariations, bool> variations = {
          {TypeToTypehintVariations::IS_SHAPE, false},
          {TypeToTypehintVariations::IMMUTABLE_COLLECTIONS, false},
          {TypeToTypehintVariations::IS_ANY_SHAPE, false},
          {TypeToTypehintVariations::IGNORE_WRAPPER, false},
          {TypeToTypehintVariations::RECURSIVE_IGNORE_WRAPPER, false},
          {TypeToTypehintVariations::IGNORE_TYPEDEF_OPTION, false}});
  std::string typedef_to_typehint(
      const t_typedef* ttypedef,
      std::map<TypeToTypehintVariations, bool> variations);
  std::string type_to_param_typehint(
      const t_type* ttype, bool nullable = false);

  std::string union_enum_name(
      const std::string& name, const t_program* program, bool decl = false) {
    return union_enum_name(hack_name(name, program, decl));
  }

  std::string union_enum_name(const std::string& name) {
    // <StructName>Type
    return name + "Enum";
  }

  std::string union_field_to_enum(
      const t_structured* tstruct,
      const t_field* tfield,
      const std::string& name) {
    // If null is passed,  it refer to empty;
    if (tfield) {
      return union_enum_name(name, tstruct->program()) + "::" + tfield->name();
    } else {
      return union_enum_name(name, tstruct->program()) + "::" + UNION_EMPTY;
    }
  }

  std::string union_field_to_enum(
      const t_field* tfield, const std::string& name) {
    // If null is passed,  it refer to empty;
    if (tfield) {
      return name + "Enum::" + tfield->name();
    } else {
      return name + "Enum::" + UNION_EMPTY;
    }
  }

  bool is_bitmask_enum(const t_enum* tenum) {
    return tenum->has_unstructured_annotation("bitmask") ||
        tenum->has_structured_annotation(kBitmaskEnumUri);
  }

  std::optional<std::string> find_hack_adapter(const t_type* type) {
    if (!is_transitive_annotation(*type)) {
      if (const auto annotation =
              t_typedef::get_first_structured_annotation_or_null(
                  type, kHackAdapterUri)) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == "name") {
            return unescape(item.second->get_string());
          }
        }
      }
    }
    return {};
  }

  std::optional<std::string> find_hack_wrapper(const t_field& node) {
    if (!is_transitive_annotation(node)) {
      auto annotation =
          node.find_structured_annotation_or_null(kHackFieldWrapperUri);
      if (!annotation) {
        annotation = node.find_structured_annotation_or_null(kHackWrapperUri);
      }
      if (annotation) {
        for (const auto& item : annotation->value()->get_map()) {
          if (item.first->get_string() == "name") {
            return unescape(item.second->get_string());
          }
        }
      }
    }
    return {};
  }

  struct wrapper_info {
    std::optional<std::string> name;
    std::optional<std::string> underlying_name;
    std::optional<std::string> extra_namespace;
  };

  wrapper_info find_hack_wrapper(
      const t_type* ttype, bool look_up_through_hierarchy = true) {
    if (is_transitive_annotation(*ttype)) {
      return {};
    }

    const t_const* annotation = look_up_through_hierarchy
        ? t_typedef::get_first_structured_annotation_or_null(
              ttype, kHackWrapperUri)
        : ttype->find_structured_annotation_or_null(kHackWrapperUri);
    if (!annotation) {
      return {};
    }

    wrapper_info info;
    info.underlying_name = "";
    info.extra_namespace = "thrift_adapted_types";
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == "name") {
        info.name = unescape(item.second->get_string());
      } else if (item.first->get_string() == "underlyingName") {
        info.underlying_name = hack_name(
            unescape(item.second->get_string()), ttype->program(), true);
      } else if (item.first->get_string() == "extraNamespace") {
        info.extra_namespace = unescape(item.second->get_string());
      }
    }
    if (!info.name) {
      return {};
    }

    // If both name and ns are not provided,
    // then we need to nest the namespace
    if (info.underlying_name->empty()) {
      info.underlying_name = hack_name(ttype, true);
    }

    auto [ns, ns_type] = get_namespace(ttype->program());
    if (ns_type == HackThriftNamespaceType::HACK ||
        ns_type == HackThriftNamespaceType::PACKAGE) {
      info.extra_namespace = ns + "\\" + *info.extra_namespace;
    }

    return info;
  }

  std::optional<std::string> find_hack_field_adapter(const t_field& node) {
    if (const auto annotation =
            node.find_structured_annotation_or_null(kHackAdapterUri)) {
      for (const auto& item : annotation->value()->get_map()) {
        if (item.first->get_string() == "name") {
          return unescape(item.second->get_string());
        }
      }
    }
    return {};
  }

  const std::string& find_hack_name(const t_named* tnamed) const {
    return find_hack_name(tnamed, tnamed->name());
  }

  const std::string& find_hack_name(
      const t_named* tnamed, const std::string& default_name) const {
    if (const auto annotation =
            tnamed->find_structured_annotation_or_null(kHackNameUri)) {
      return annotation->get_value_from_structured_annotation("name")
          .get_string();
    }
    if (const std::string* annotation =
            tnamed->find_unstructured_annotation_or_null("hack.name")) {
      return *annotation;
    }
    return default_name;
  }

  const std::string find_union_enum_attributes(const t_structured& tstruct) {
    if (const auto annotation = tstruct.find_structured_annotation_or_null(
            kHackUnionEnumAttributesUri)) {
      for (const auto& item : annotation->value()->get_map()) {
        if (item.first->get_string() == "attributes") {
          std::string result;
          bool first = true;
          for (const auto& attribute : item.second->get_list()) {
            if (first) {
              first = false;
            } else {
              result += ", ";
            }
            result += attribute->get_string();
          }
          return result;
        }
      }
    }
    return "";
  }

  const std::string find_attributes(
      const t_named& tnamed, bool include_unstructured) {
    if (include_unstructured) {
      if (const std::string* annotation =
              tnamed.find_unstructured_annotation_or_null("hack.attributes")) {
        return *annotation;
      }
    }
    if (const auto annotation =
            tnamed.find_structured_annotation_or_null(kHackAttributeUri)) {
      for (const auto& item : annotation->value()->get_map()) {
        if (item.first->get_string() == "attributes") {
          std::string result;
          bool first = true;
          for (const auto& attribute : item.second->get_list()) {
            if (first) {
              first = false;
            } else {
              result += ", ";
            }
            result += attribute->get_string();
          }
          return result;
        }
      }
    }
    return "";
  }

  std::pair<bool, const std::string> find_hack_struct_trait(
      const t_structured* tstruct) const {
    if (const auto annotation =
            tstruct->find_structured_annotation_or_null(kHackStructTraitUri)) {
      for (const auto& item : annotation->value()->get_map()) {
        if (item.first->get_string() == "name") {
          return std::make_pair(true, item.second->get_string());
        }
      }
      return std::make_pair(true, "");
    }
    return std::make_pair(false, "");
  }

  bool has_hack_struct_as_trait(const t_structured* tstruct) const {
    return tstruct->has_structured_annotation(kHackStructAsTraitUri);
  }

  bool has_hack_module_internal(const t_named* tnamed) const {
    return tnamed->has_structured_annotation(kHackModuleInternalUri);
  }

  bool should_generate_client_header_methods(const t_named* tnamed) const {
    return tnamed->has_structured_annotation(
        kHackGenerateClientMethodsWithHeaders);
  }

  std::string php_namespace(const t_program* p) const {
    std::string php_ns = p->get_namespace("php");
    if (!php_ns.empty()) {
      std::replace(php_ns.begin(), php_ns.end(), '.', '_');
      php_ns.push_back('_');
    }
    return php_ns;
  }

  std::string package_namespace(const t_program* p) const {
    auto pkg_path = p->package().path();
    if (!p->package().domain().empty()) {
      pkg_path.insert(
          pkg_path.begin(),
          p->package().domain().begin(),
          p->package().domain().end() - 1);
    }
    if (!pkg_path.empty()) {
      std::string pkg_ns = fmt::format("{}", fmt::join(pkg_path, "\\"));
      return pkg_ns;
    }
    return "";
  }

  std::tuple<std::string, HackThriftNamespaceType> get_namespace(
      const t_program* p) const {
    std::string ns;
    HackThriftNamespaceType type_ = HackThriftNamespaceType::EMPTY;
    const auto& namespaces = p->namespaces();

    auto php_ns = php_namespace(p);
    auto pkg_ns = package_namespace(p);
    if (auto hack_ns = namespaces.find("hack"); hack_ns != namespaces.end()) {
      // Provided hack namespace can be empty
      // Empty Hack namespace should be treated as no namespace
      if (!hack_ns->second.empty()) {
        ns = hack_ns->second;
        std::replace(ns.begin(), ns.end(), '.', '\\');
        type_ = HackThriftNamespaceType::HACK;
      }
    } else if (!php_ns.empty()) {
      ns = php_ns;
      type_ = HackThriftNamespaceType::PHP;
    } else if (!pkg_ns.empty()) {
      ns = pkg_ns;
      type_ = HackThriftNamespaceType::PACKAGE;
    }
    return {ns, type_};
  }

  std::tuple<std::string, HackThriftNamespaceType> get_namespace(
      const t_service* s) const {
    return get_namespace(s->program());
  }

  std::string hack_name(
      std::string name, const t_program* prog, bool decl = false) {
    auto [ns, ns_type] = get_namespace(prog);
    if (ns_type == HackThriftNamespaceType::HACK ||
        ns_type == HackThriftNamespaceType::PACKAGE) {
      if (decl) {
        return name;
      }
      return "\\" + ns + "\\" + name;
    }
    return (!decl && (has_hack_namespace || has_nested_ns) ? "\\" : "") +
        (ns_type == HackThriftNamespaceType::PHP ? ns : "") + name;
  }

  std::string hack_name(const t_type* t, bool decl = false) {
    return hack_name(find_hack_name(t), t->program(), decl);
  }

  std::string hack_wrapped_type_name(
      const std::optional<std::string>& underlying_name,
      const std::optional<std::string>& underlying_ns,
      bool decl = false) {
    if (underlying_name.has_value()) {
      if (decl) {
        return *underlying_name;
      }

      if (underlying_ns.has_value()) {
        return "\\" + *underlying_ns + "\\" + *underlying_name;
      } else if (has_hack_namespace || has_nested_ns) {
        return "\\" + *underlying_name;
      }

      return *underlying_name;
    }

    throw std::runtime_error("`underlying_name` doesn't have a value!");
  }

  const char* UNION_EMPTY = "_EMPTY_";

  bool is_base_exception_property(const t_field*);

  std::string render_service_metadata_response(
      const t_service* service, const bool mangle);
  std::string render_structured_annotations(
      node_list_view<const t_const> annotations,
      std::ostream& temp_var_initializations_out,
      t_name_generator& namer);

 private:
  void generate_php_docstring(std::ofstream& out, const t_named* named_node);
  void generate_php_docstring(std::ofstream& out, const t_enum* tenum);
  void generate_php_docstring(std::ofstream& out, const t_service* tservice);
  void generate_php_docstring(std::ofstream& out, const t_const* tconst);
  void generate_php_docstring(std::ofstream& out, const t_function* tfunction);
  void generate_php_docstring(std::ofstream& out, const t_field* tfield);
  void generate_php_docstring(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_exception = false);
  void generate_php_docstring_args(
      std::ofstream& out, size_t start_pos, const t_structured* arg_list);
  void generate_php_docstring_stream_exceptions(
      std::ofstream& out, const t_throws* ex);

  /**
   * Generate the namespace mangled string, if necessary
   */
  std::string php_servicename_mangle(
      bool mangle,
      const t_service* svc,
      const std::string& name,
      bool extends = false) {
    auto [ns, ns_type] = get_namespace(svc);
    const std::string& s_name =
        !name.empty() ? find_hack_name(svc, name) : find_hack_name(svc);
    if (extends &&
        (ns_type == HackThriftNamespaceType::HACK ||
         ns_type == HackThriftNamespaceType::PACKAGE)) {
      return hack_name(s_name, svc->program());
    }
    return (extends && has_hack_namespace ? "\\" : "") + (mangle ? ns : "") +
        s_name;
  }

  std::string php_servicename_mangle(
      bool mangle, const t_service* svc, bool extends = false) {
    return php_servicename_mangle(mangle, svc, svc->name(), extends);
  }

  /**
   * Return the correct function to be used on a Hack Collection, only when
   * generating shape structures.
   * - If array_migration_ is set and its not a list, then use darray.
   * - If we're on a list, we'll want to use vec over dict
   */
  std::string generate_to_array_method(
      const t_type* t, const std::string& array) {
    if (!t->is<t_container>()) {
      throw std::logic_error("not a container");
    }
    if (array_migration_ && !t->is<t_list>()) {
      return "ThriftUtil::toDArray(" + array + ", static::class)";
    } else {
      return t->is<t_list>() ? "vec(" + array + ")" : "dict(" + array + ")";
    }
  }

  bool is_hack_const_type(const t_type* type);

  std::vector<const t_function*> get_supported_server_functions(
      const t_service* tservice, bool async) {
    std::vector<const t_function*> funcs;
    for (auto func : tservice->get_functions()) {
      if (is_function_supported(func, false, async)) {
        funcs.push_back(func);
      }
    }
    return funcs;
  }

  std::vector<const t_function*> get_supported_client_functions(
      const t_service* tservice) {
    std::vector<const t_function*> funcs;
    for (auto func : tservice->get_functions()) {
      if (is_function_supported(func, true)) {
        funcs.push_back(func);
      }
    }
    return funcs;
  }

  bool is_function_supported(
      const t_function* func, bool client, bool async = true) {
    if (func->interaction()) {
      return false;
    }
    if (client) {
      return true;
    }
    if (func->is_interaction_constructor() || func->sink()) {
      return false;
    }
    if (func->stream()) {
      return server_stream_ && async;
    }
    return true;
  }

  std::vector<const t_service*> get_interactions(
      const t_service* tservice) const {
    std::vector<const t_service*> interactions;
    for (const auto& func : tservice->get_functions()) {
      if (func->is_interaction_constructor()) {
        interactions.push_back(
            dynamic_cast<const t_service*>(func->interaction().get_type()));
      }
    }
    return interactions;
  }

  /**
   * File streams
   */
  std::ofstream f_types_;
  std::ofstream f_adapted_types_;
  std::ofstream f_consts_;
  std::ofstream f_helpers_;
  std::ofstream f_service_;

  /**
   * True iff we should generate a function parse json to thrift object.
   */
  bool json_;

  /**
   * Generate stubs for a PHP server
   */
  bool phps_;

  /**
   * * Whether to use collection classes everywhere vs KeyedContainer
   */
  bool strict_types_;

  /**
   * Whether to generate protected members for thrift unions
   */
  bool protected_unions_;

  /**
   * Whether to generate enforce single set field for unions.
   *
   * Setting this also implies protected_unions_
   */
  bool strict_unions_;

  /**
   * Control rollout of incorrect field access in union logging
   */
  bool union_logger_rollout_;

  /**
   * Preserve original union json serialization behavior with fb_json_encode
   */
  bool legacy_union_json_serialization_;

  /**
   * Whether to generate array sets or Set objects
   */
  bool arraysets_;

  /**
   * memory of the values of the constants in array initialisation form
   * for use with generate_const
   */
  std::vector<std::string> constants_values_;

  /**
   * True iff mangled service classes should be emitted
   */
  bool mangled_services_;

  /**
   * True if struct fields within structs should be instantiated rather than
   * nullable typed
   */
  bool no_nullables_;

  /**
   * True if struct should generate fromMap_DEPRECATED method with a semantic
   * of a legacy map constructor.
   */
  bool from_map_construct_;

  /**
   * True if we should generate Shape types for the generated structs
   */
  bool shapes_;

  /**
   * True if we should generate array<arraykey, TValue> instead of array<string,
   * TValue>
   */
  bool shape_arraykeys_;

  /**
   * True if we should allow implicit subtyping for shapes (i.e. '...')
   */
  bool shapes_allow_unknown_fields_;

  /**
   * True to use darrays instead of dicts for internal constructs
   */
  bool array_migration_;

  /**
   * True to never use hack collection objects. Only used for migrations
   */
  bool legacy_arrays_;

  /**
   * True to use hack collection objects.
   */
  bool hack_collections_;

  /**
   * True to force client methods to accept null arguments. Only used for
   * migrations
   */
  bool nullable_everything_;

  /**
   * True to force hack collection members be const collection objects
   */
  bool const_collections_;

  /**
   * True to generate explicit types for Hack enums: 'type FooType = Foo'
   */
  bool enum_extratype_;

  /**
   * True to use transparent typing for Hack enums: 'enum FooBar: int as int'.
   */
  bool enum_transparenttype_;

  /**
   * True to generate soft typehints as __Soft instead of @
   */
  bool soft_attribute_;

  /**
   * True to generate type aliases for typedefs defined
   */
  bool typedef_;

  /**
   * True to generate service code for streaming methods
   */
  bool server_stream_;

  bool has_hack_namespace;

  bool has_nested_ns;

  std::map<std::string, ThriftShapishStructType> struct_async_type_;

  /**
   * When to start emitting UNSAFE_CAST in $_TSPEC shape initializers.
   */
  uint32_t min_depth_for_unsafe_cast_ = 4;

  std::unordered_map<const t_type*, bool> type_validity_;
};

void t_hack_generator::generate_json_enum(
    std::ofstream& out,
    t_name_generator& /* namer */,
    const t_enum* tenum,
    const std::string& prefix_thrift,
    const std::string& prefix_json) {
  indent(out) << prefix_thrift << " = " << hack_name(tenum) << "::coerce("
              << prefix_json << ");\n";
}

void t_hack_generator::generate_json_struct(
    std::ofstream& out,
    t_name_generator& namer,
    const t_structured* tstruct,
    const std::string& prefix_thrift,
    const std::string& prefix_json) {
  std::string enc = namer("$_tmp");
  indent(out) << enc << " = " << "\\json_encode(" << prefix_json << ");\n";
  std::string tmp = namer("$_tmp");
  t_field felem(*tstruct, tmp);
  indent(out) << declare_field(&felem, true, true, true).substr(1) << "\n";
  indent(out) << tmp << "->readFromJson(" << enc << ");\n";
  indent(out) << prefix_thrift << " = " << tmp << ";\n";
}

void t_hack_generator::generate_json_field(
    std::ofstream& out,
    t_name_generator& namer,
    const t_field* tfield,
    const std::string& prefix_thrift,
    const std::string& suffix_thrift,
    const std::string& prefix_json) {
  if (skip_codegen(tfield)) {
    return;
  }
  const t_type* type = tfield->get_type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT READ JSON FIELD WITH void TYPE: " + prefix_thrift +
        tfield->name());
  }

  std::string name = prefix_thrift + tfield->name() + suffix_thrift;

  if (const auto* tstruct = dynamic_cast<const t_structured*>(type)) {
    generate_json_struct(out, namer, tstruct, name, prefix_json);
  } else if (const auto* tconatiner = dynamic_cast<const t_container*>(type)) {
    generate_json_container(out, namer, tconatiner, name, prefix_json);
  } else if (const auto* tenum = dynamic_cast<const t_enum*>(type)) {
    generate_json_enum(out, namer, tenum, name, prefix_json);
  } else if (const auto* tbase_type = type->try_as<t_primitive_type>()) {
    std::string typeConversionString;
    std::string number_limit;
    switch (tbase_type->primitive_type()) {
      case t_primitive_type::type::t_void:
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
      case t_primitive_type::type::t_bool:
      case t_primitive_type::type::t_i64:
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        break;
      case t_primitive_type::type::t_byte:
        number_limit = "0x7f";
        typeConversionString = "(int)";
        break;
      case t_primitive_type::type::t_i16:
        number_limit = "0x7fff";
        typeConversionString = "(int)";
        break;
      case t_primitive_type::type::t_i32:
        number_limit = "0x7fffffff";
        typeConversionString = "(int)";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no PHP reader for base type " +
            t_primitive_type::type_name(tbase_type->primitive_type()) + name);
    }

    if (number_limit.empty()) {
      indent(out) << name << " = " << typeConversionString << prefix_json
                  << ";\n";
    } else {
      std::string temp = namer("$_tmp");
      indent(out) << temp << " = (int)" << prefix_json << ";\n";
      indent(out) << "if (" << temp << " > " << number_limit << ") {\n";
      indent_up();
      indent(out) << "throw new \\TProtocolException(\"number exceeds "
                  << "limit in field\");\n";
      indent_down();
      indent(out) << "} else {\n";
      indent_up();
      indent(out) << name << " = " << typeConversionString << temp << ";\n";
      indent_down();
      indent(out) << "}\n";
    }
  }
}

void t_hack_generator::generate_json_container(
    std::ofstream& out,
    t_name_generator& namer,
    const t_type* ttype,
    const std::string& prefix_thrift,
    const std::string& prefix_json) {
  std::string size = namer("$_size");
  std::string key = namer("$_key");
  std::string value = namer("$_value");
  std::string json = namer("$_json");
  std::string container = namer("$_container");

  indent(out) << json << " = " << prefix_json << ";\n";
  if (ttype->is<t_map>()) {
    if (hack_collections_) {
      indent(out) << container << " = Map {};\n";
    } else {
      indent(out) << container << " = dict[];\n";
    }
  } else if (ttype->is<t_list>()) {
    if (hack_collections_) {
      indent(out) << container << " = Vector {};\n";
    } else {
      indent(out) << container << " = vec[];\n";
    }
  } else if (ttype->is<t_set>()) {
    if (arraysets_) {
      indent(out) << container << " = dict[];\n";
    } else if (hack_collections_) {
      indent(out) << container << " = Set {};\n";
    } else {
      indent(out) << container << " = keyset[];\n";
    }
  }
  indent(out) << "foreach(" << json << " as " << key << " => " << value
              << ") {\n";
  indent_up();

  if (const auto* tlist = dynamic_cast<const t_list*>(ttype)) {
    generate_json_list_element(out, namer, tlist, value, container);
  } else if (const auto* tset = dynamic_cast<const t_set*>(ttype)) {
    generate_json_set_element(out, namer, tset, value, container);
  } else if (const auto* tmap = dynamic_cast<const t_map*>(ttype)) {
    generate_json_map_element(out, namer, tmap, key, value, container);
  } else {
    throw std::runtime_error("compiler error: no PHP reader for this type.");
  }
  indent_down();
  indent(out) << "}\n";
  indent(out) << prefix_thrift << " = " << container << ";\n";
}

void t_hack_generator::generate_json_list_element(
    std::ofstream& out,
    t_name_generator& namer,
    const t_list* tlist,
    const std::string& value,
    const std::string& prefix_thrift) {
  std::string elem = namer("$_elem");
  t_field felem(*tlist->get_elem_type(), elem);
  indent(out) << declare_field(&felem, true, true, true).substr(1) << "\n";
  generate_json_field(out, namer, &felem, "", "", value);
  indent(out) << prefix_thrift << " []= " << elem << ";\n";
}

void t_hack_generator::generate_json_set_element(
    std::ofstream& out,
    t_name_generator& namer,
    const t_set* tset,
    const std::string& value,
    const std::string& prefix_thrift) {
  std::string elem = namer("$_elem");
  t_field felem(*tset->get_elem_type(), elem);
  indent(out) << declare_field(&felem, true, true, true).substr(1) << "\n";
  generate_json_field(out, namer, &felem, "", "", value);
  if (arraysets_) {
    indent(out) << prefix_thrift << "[" << elem << "] = true;\n";
  } else if (hack_collections_) {
    indent(out) << prefix_thrift << "->add(" << elem << ");\n";
  } else {
    indent(out) << prefix_thrift << " []= " << elem << ";\n";
  }
}

void t_hack_generator::generate_json_map_element(
    std::ofstream& out,
    t_name_generator& namer,
    const t_map* tmap,
    const std::string& key,
    const std::string& value,
    const std::string& prefix_thrift) {
  const t_type* keytype = tmap->key_type()->get_true_type();
  auto error = std::runtime_error(
      "compiler error: Thrift Hack compiler"
      "does not support complex types as the key of a map.");

  if (!keytype->is<t_enum>() && !keytype->is<t_primitive_type>()) {
    throw error;
  }
  if (const auto* tbase_type = keytype->try_as<t_primitive_type>()) {
    switch (tbase_type->primitive_type()) {
      case t_primitive_type::type::t_void:
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        throw error;
      default:
        break;
    }
  }
  std::string _value = namer("$_value");
  t_field vfelem(*tmap->get_val_type(), _value);
  indent(out) << declare_field(&vfelem, true, true, true).substr(1) << "\n";
  generate_json_field(out, namer, &vfelem, "", "", value);
  indent(out) << prefix_thrift << "[" << key << "] = " << _value << ";\n";
}

void t_hack_generator::generate_json_reader(
    std::ofstream& out, const t_structured* tstruct) {
  if (!json_) {
    return;
  }
  t_name_generator namer;

  indent(out) << "public function readFromJson(string $jsonText): void {\n";
  indent_up();
  if (tstruct->is<t_union>()) {
    indent(out) << "$this->_type = "
                << union_field_to_enum(tstruct, nullptr, tstruct->name())
                << ";\n";
  }
  indent(out) << "$parsed = json_decode($jsonText, true);\n\n";

  indent(out)
      << "if ($parsed === null || !($parsed is KeyedContainer<_, _>)) {\n";
  indent_up();
  indent(out) << "throw new \\TProtocolException(\"Cannot parse the given json"
              << " string.\");\n";
  indent_down();
  indent(out) << "}\n\n";
  for (const auto& tf : tstruct->fields()) {
    if (skip_codegen(&tf)) {
      continue;
    }
    indent(out) << "if (idx($parsed, '" << tf.name() << "') !== null) {\n";
    indent_up();
    std::string typehint = type_to_typehint(tf.get_type());
    generate_json_field(
        out,
        namer,
        &tf,
        "$this->",
        "",
        "HH\\FIXME\\UNSAFE_CAST<mixed, " + typehint + ">($parsed['" +
            tf.name() + "'])");
    if (tstruct->is<t_union>()) {
      indent(out) << "$this->_type = "
                  << union_field_to_enum(tstruct, &tf, tstruct->name())
                  << ";\n";
    }
    indent_down();
    indent(out) << "}";
    if (tf.get_req() == t_field::e_req::required) {
      out << " else {\n";
      indent_up();
      indent(out) << "throw new \\TProtocolException(\"Required field "
                  << tf.name() << " cannot be found.\");\n";
      indent_down();
      indent(out) << "}";
    }
    out << "\n";
  }
  indent_down();
  indent(out) << "}\n\n";
}

void t_hack_generator::generate_instance_key(std::ofstream& out) {
  indent(out) << "public function getInstanceKey()[write_props]: string {\n";
  indent_up();
  indent(out) << "return \\TCompactSerializer::serialize($this);\n";
  indent_down();
  indent(out) << "}\n\n";
}

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_hack_generator::init_generator() {
  // Make output directory.
  std::filesystem::create_directory(get_out_dir());
  init_codegen_file(
      f_types_, get_out_dir() + get_program()->name() + "_types.php");

  // Print header.
  if (!program_->consts().empty()) {
    init_codegen_file(
        f_consts_, get_out_dir() + get_program()->name() + "_constants.php");
    constants_values_.clear();
    auto [ns, ns_type] = get_namespace(program_);
    auto class_name = get_program()->name() + "_";
    f_consts_ << "class "
              << (ns_type != HackThriftNamespaceType::PHP ? class_name : ns)
              << "CONSTANTS implements \\IThriftConstants {\n";
  }

  if (!program_->structs_and_unions().empty()) {
    bool codegen_file_open = false;
    for (const t_structured* tstruct : program_->structs_and_unions()) {
      auto [wrapper, name, ns] = find_hack_wrapper(tstruct, false);
      if (wrapper) {
        if (!codegen_file_open) {
          init_codegen_file(
              f_adapted_types_,
              get_out_dir() + get_program()->name() + "_adapted_types.php",
              true);
          codegen_file_open = true;
        }
        f_types_ << "type " << hack_name(tstruct, true) << " = " << *wrapper
                 << "<" << hack_wrapped_type_name(name, ns) << ">;\n";
      }
    }
  }
}

void t_hack_generator::init_codegen_file(
    std::ofstream& file_, const std::string& file_name_, bool skip_ns) {
  file_.open(file_name_.c_str());
  record_genfile(file_name_);

  // Print header
  file_ << "<?hh\n" << autogen_comment() << "\n";

  if (skip_ns) {
    return;
  } else {
    auto [ns, _] = get_namespace(program_);
    if (has_hack_namespace) {
      file_ << "namespace " << ns << ";\n\n";
    }
  }
  const std::string module = program_->get_namespace("hack.module");
  if (!module.empty()) {
    file_ << "module " << module << ";\n";
  }
}

/**
 * Close up (or down) some filez.
 */
void t_hack_generator::close_generator() {
  // Close types file
  f_types_.close();

  if (!program_->consts().empty()) {
    // write out the values array
    indent_up();
    f_consts_ << "\n";
    // write structured annotations
    f_consts_
        << indent()
        << "public static function getAllStructuredAnnotations()[write_props]: "
           "dict<string, dict<string, \\IThriftStruct>> {\n";
    indent_up();

    std::stringstream annotations_out;
    std::stringstream annotations_temp_var_initializations_out;
    t_name_generator namer;

    annotations_out << indent() << "return dict[\n";
    indent_up();
    for (const t_const* tconst : program_->consts()) {
      if (tconst->structured_annotations().empty()) {
        continue;
      }
      annotations_out << indent() << "'" << tconst->name() << "' => "
                      << render_structured_annotations(
                             tconst->structured_annotations(),
                             annotations_temp_var_initializations_out,
                             namer)
                      << ",\n";
    }

    f_consts_ << annotations_temp_var_initializations_out.str();
    f_consts_ << annotations_out.str();
    indent_down();
    f_consts_ << indent() << "];\n";
    indent_down();
    f_consts_ << indent() << "}\n";
    indent_down();
    // close constants class
    f_consts_ << "}\n\n";
    f_consts_.close();
  }
  if (f_adapted_types_.is_open()) {
    f_adapted_types_.close();
  }
}

/**
 * Generates a typedef.
 *
 * @param ttypedef The type definition
 */
void t_hack_generator::generate_typedef(const t_typedef* ttypedef) {
  if (!typedef_) {
    return;
  }
  if (t_typedef::get_first_structured_annotation_or_null(
          ttypedef, kHackSkipCodegenUri)) {
    return;
  }
  bool is_mod_int = has_hack_module_internal(ttypedef);
  auto typedef_name = hack_name(ttypedef, true);
  auto [wrapper, name, ns] = find_hack_wrapper(ttypedef, false);
  if (wrapper) {
    // For wrapped typedef, typehint is generated in nested ns.
    // Set this flag to true to ensure that any references to other typedefs or
    // structs is prefixed with "\"
    has_nested_ns = true;
  }
  std::string typehint;
  if (auto adapter = find_hack_adapter(ttypedef)) {
    typehint = *adapter + "::THackType";
  } else {
    typehint = type_to_typehint(ttypedef->get_type());
  }
  if (wrapper && name.has_value()) {
    f_types_ << (is_mod_int ? "internal " : "") << "type " << typedef_name
             << " = " << *wrapper << "<" << hack_wrapped_type_name(name, ns)
             << ">;\n";

    if (!f_adapted_types_.is_open()) {
      init_codegen_file(
          f_adapted_types_,
          get_out_dir() + get_program()->name() + "_adapted_types.php");
    }
    if (ns) {
      f_adapted_types_ << "namespace " << *ns << " {\n";
    }
    f_adapted_types_ << (is_mod_int ? "internal " : "") << "type " << *name
                     << " = " << typehint << ";\n";

    if (ns) {
      f_adapted_types_ << "}\n";
    }
  } else {
    if (typedef_name == typehint) {
      return;
    }
    f_types_ << (is_mod_int ? "internal " : "") << "type " << typedef_name
             << " = " << typehint << ";\n";
  }
  // Reset the flag
  has_nested_ns = false;
}

/**
 * Generates code for an enumerated type. Since define is expensive to lookup
 * in PHP, we use a global array for this.
 *
 * @param tenum The enumeration
 */
void t_hack_generator::generate_enum(const t_enum* tenum) {
  std::string typehint;
  generate_php_docstring(f_types_, tenum);
  bool hack_enum = false;
  if (is_bitmask_enum(tenum)) {
    typehint = "int";
    f_types_ << "final class " << hack_name(tenum, true)
             << " extends \\Flags {\n";
  } else {
    hack_enum = true;
    typehint = hack_name(tenum, true);
    generate_hack_attributes(f_types_, tenum, /*include_user_defined*/ true);
    f_types_ << "enum " << hack_name(tenum, true) << ": int"
             << (enum_transparenttype_ ? " as int" : "") << " {\n";
  }

  indent_up();

  for (const t_enum_value* enum_value : tenum->get_enum_values()) {
    int32_t value = enum_value->get_value();

    generate_php_docstring(f_types_, enum_value);
    if (!hack_enum) {
      indent(f_types_) << "const " << typehint << " ";
    }
    indent(f_types_) << find_hack_name(enum_value) << " = " << value << ";\n";
  }

  indent_down();
  f_types_ << "}\n";
  if (hack_enum && enum_extratype_) {
    f_types_ << "type " << typehint << "Type = " << typehint << ";\n";
  }
  f_types_ << "\n";

  f_types_ << indent() << "class " << hack_name(tenum, true)
           << "_TEnumStaticMetadata implements \\IThriftEnumStaticMetadata {\n";
  indent_up();

  // Expose enum metadata
  f_types_ << indent() << "public static function getEnumMetadata()[]: "
           << "\\tmeta_ThriftEnum {\n";
  indent_up();

  f_types_ << indent() << "return "
           << render_const_value(
                  tmeta_ThriftEnum_type(),
                  enum_to_tmeta(tenum).get(),
                  /*immutable_collections*/ false,
                  /*ignore_wrapper*/ false,
                  /*force_arrays*/ true)
           << ";\n";

  indent_down();
  f_types_ << indent() << "}\n\n";

  // Structured annotations
  f_types_
      << indent()
      << "public static function getAllStructuredAnnotations()[write_props]: "
         "\\TEnumAnnotations {\n";
  indent_up();

  std::stringstream annotations_out;
  std::stringstream annotations_temp_var_initializations_out;
  t_name_generator namer;

  annotations_out << indent() << "return shape(\n";
  indent_up();
  annotations_out << indent() << "'enum' => "
                  << render_structured_annotations(
                         tenum->structured_annotations(),
                         annotations_temp_var_initializations_out,
                         namer)
                  << ",\n";
  annotations_out << indent() << "'constants' => dict[\n";
  indent_up();
  for (const t_enum_value* constant : tenum->get_enum_values()) {
    if (constant->structured_annotations().empty()) {
      continue;
    }
    annotations_out << indent() << "'" << constant->name() << "' => "
                    << render_structured_annotations(
                           constant->structured_annotations(),
                           annotations_temp_var_initializations_out,
                           namer)
                    << ",\n";
  }

  f_types_ << annotations_temp_var_initializations_out.str();
  f_types_ << annotations_out.str();
  indent_down();
  f_types_ << indent() << "],\n";
  indent_down();
  f_types_ << indent() << ");\n";
  indent_down();
  f_types_ << indent() << "}\n";
  indent_down();
  f_types_ << indent() << "}\n\n";
}

/**
 * Generate a constant value
 */
void t_hack_generator::generate_const(const t_const* tconst) {
  const t_type* type = tconst->type();
  const std::string& name = tconst->name();
  const t_const_value* value = tconst->value();

  indent_up();
  generate_php_docstring(f_consts_, tconst);
  bool is_hack_const = is_hack_const_type(type);
  f_consts_ << indent();

  std::stringstream consts_out;
  std::stringstream consts_temp_var_initializations_out;
  t_name_generator namer;

  // for base hack types, use const (guarantees optimization in hphp)
  if (is_hack_const) {
    f_consts_ << "const " << type_to_typehint(type) << " " << name << " = ";
    // cannot use const for objects (incl arrays). use static
  } else {
    f_consts_ << "<<__Memoize>>\n"
              << indent() << "public static function " << name
              << "()[write_props]: "
              << type_to_typehint(
                     type,
                     {{TypeToTypehintVariations::IMMUTABLE_COLLECTIONS, true}})
              << "{\n";
    indent_up();
    consts_out << indent() << "return ";
  }
  consts_out << render_const_value_helper(
                    type,
                    value,
                    consts_temp_var_initializations_out,
                    namer,
                    /*immutable_collections*/ true,
                    /*ignore_wrapper*/ false,
                    /*structured_annotations*/ false,
                    /*force_arrays*/ false,
                    /*exclude_from_fixtures*/ tconst->generated())
             << ";\n";

  f_consts_ << consts_temp_var_initializations_out.str();
  f_consts_ << consts_out.str();
  if (!is_hack_const) {
    indent_down();
    f_consts_ << indent() << "}\n";
  }
  f_consts_ << "\n";
  indent_down();
}

bool t_hack_generator::is_hack_const_type(const t_type* type) {
  auto [wrapper, name, ns] = find_hack_wrapper(type);
  if (wrapper) {
    return false;
  }
  if (const auto* ttypedef = dynamic_cast<const t_typedef*>(type)) {
    return is_hack_const_type(ttypedef->get_type());
  }
  type = type->get_true_type();
  if (type->is<t_primitive_type>() || type->is<t_enum>()) {
    return true;
  } else if (type->is<t_container>()) {
    if (legacy_arrays_ || hack_collections_) {
      return false;
    } else if (const auto* tlist = dynamic_cast<const t_list*>(type)) {
      return is_hack_const_type(tlist->get_elem_type());
    } else if (const auto* tset = dynamic_cast<const t_set*>(type)) {
      return is_hack_const_type(tset->get_elem_type());
    } else if (const auto* tmap = dynamic_cast<const t_map*>(type)) {
      return is_hack_const_type(&tmap->key_type().deref()) &&
          is_hack_const_type(tmap->get_val_type());
    }
  }
  return false;
}

std::string t_hack_generator::render_string(const std::string& value) {
  return fmt::format("\"{}\"", compiler::get_escaped_string(value));
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
std::string t_hack_generator::render_const_value(
    const t_type* type,
    const t_const_value* value,
    bool immutable_collections,
    bool ignore_wrapper,
    bool force_arrays) {
  std::ostringstream out;
  std::ostringstream initialization_out;
  t_name_generator namer;
  auto const_val = render_const_value_helper(
      type,
      value,
      initialization_out,
      namer,
      immutable_collections,
      ignore_wrapper,
      /*structured_annotations*/ false,
      force_arrays);
  out << initialization_out.str();
  out << const_val;
  return out.str();
}

std::string t_hack_generator::render_const_value_helper(
    const t_type* type,
    const t_const_value* value,
    std::ostream& temp_var_initializations_out,
    t_name_generator& namer,
    bool immutable_collections,
    bool ignore_wrapper,
    bool structured_annotations,
    bool force_arrays,
    bool exclude_from_fixtures) {
  std::ostringstream out;
  if (const auto* ttypedef = dynamic_cast<const t_placeholder_typedef*>(type)) {
    type = ttypedef->get_type();
  }
  if (const auto* ttypedef = dynamic_cast<const t_typedef*>(type)) {
    type = ttypedef->get_type();
    auto val = render_const_value_helper(
        type,
        value,
        temp_var_initializations_out,
        namer,
        immutable_collections,
        /*ignore_wrapper*/ false,
        /*structured_annotations*/ false,
        force_arrays);
    if (ignore_wrapper) {
      return val;
    }
    auto [wrapper, name, ns] = find_hack_wrapper(ttypedef);
    if (wrapper) {
      out << *wrapper << "::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<";
      out << type_to_typehint(
                 ttypedef, {{TypeToTypehintVariations::IGNORE_WRAPPER, true}})
          << ">(" << val << ")";
      return out.str();
    }
    return val;
  }

  if (const auto* tbase_type = type->try_as<t_primitive_type>()) {
    auto exclude_delimiter =
        exclude_from_fixtures ? "/*@fbthrift_strip_from_fixtures*/" : "";
    out << exclude_delimiter;
    switch (tbase_type->primitive_type()) {
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        out << render_string(value->get_string());
        break;
      case t_primitive_type::type::t_bool:
        out << (value->get_integer() > 0 ? "true" : "false");
        break;
      case t_primitive_type::type::t_byte:
      case t_primitive_type::type::t_i16:
      case t_primitive_type::type::t_i32:
      case t_primitive_type::type::t_i64:
        out << value->get_integer();
        break;
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        if (value->kind() == t_const_value::CV_INTEGER) {
          out << value->get_integer();
        } else {
          out << value->get_double();
        }
        if (out.str().find('.') == std::string::npos &&
            out.str().find('e') == std::string::npos) {
          out << ".0";
        }
        break;
      default:
        throw std::runtime_error(
            "compiler error: no const of base type " +
            t_primitive_type::type_name(tbase_type->primitive_type()));
    }
    out << exclude_delimiter;
  } else if (const auto* tenum = dynamic_cast<const t_enum*>(type)) {
    const t_enum_value* val = tenum->find_value(value->get_integer());
    if (val != nullptr) {
      out << hack_name(tenum) << "::" << val->name();
    } else {
      out << hack_name(tenum) << "::coerce(" << value->get_integer() << ")";
    }
  } else if (const auto* tstruct = dynamic_cast<const t_structured*>(type)) {
    std::string struct_name = hack_name(type);
    auto [wrapper, name, ns] = find_hack_wrapper(type);
    if (wrapper) {
      struct_name = hack_wrapped_type_name(name, ns);
      if (!ignore_wrapper) {
        out << *wrapper << "::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<";
        out << struct_name << ">(";
      }
    }
    if (is_async_struct(tstruct)) {
      auto temp_val = namer(("$" + hack_name(type, true)).c_str());
      int preserved_indent_size = get_indent();
      set_indent(2);
      temp_var_initializations_out << indent() << temp_val << " = "
                                   << struct_name << "::withDefaultValues();\n";
      for (const auto& entry : value->get_map()) {
        const auto* field =
            tstruct->get_field_by_name(entry.first->get_string());
        if (field == nullptr) {
          throw std::runtime_error(
              "type error: " + type->name() + " has no field " +
              entry.first->get_string());
        }
        t_const_value* v = entry.second;
        std::stringstream inner;
        if (v) {
          auto field_wrapper = find_hack_wrapper(*field);
          if (field_wrapper) {
            inner << indent() << temp_val << "->get_" << field->name()
                  << "()->setValue_DO_NOT_USE_THRIFT_INTERNAL(";
          } else {
            inner << indent() << temp_val << "->" << field->name() << " = ";
          }
          inner << render_const_value_helper(
              field->get_type(),
              v,
              temp_var_initializations_out,
              namer,
              false,
              false,
              structured_annotations,
              force_arrays);
          if (field_wrapper) {
            inner << ");\n";
          } else {
            inner << ";\n";
          }
        }
        temp_var_initializations_out << inner.str() << "\n";
      }
      out << temp_val;

      set_indent(preserved_indent_size);
    } else {
      out << struct_name << "::fromShape(\n";
      indent_up();
      indent(out) << "shape(\n";
      indent_up();

      for (const auto& entry : value->get_map()) {
        const auto* field =
            tstruct->get_field_by_name(entry.first->get_string());
        if (field == nullptr) {
          throw std::runtime_error(
              "type error: " + type->name() + " has no field " +
              entry.first->get_string());
        }
      }
      for (const auto& field : tstruct->fields()) {
        t_const_value* k = nullptr;
        t_const_value* v = nullptr;
        for (const auto& entry : value->get_map()) {
          if (field.name() == entry.first->get_string()) {
            k = entry.first;
            v = entry.second;
          }
        }
        if (v != nullptr) {
          assert(k != nullptr);
          indent(out) << render_const_value_helper(
                             &t_primitive_type::t_string(),
                             k,
                             temp_var_initializations_out,
                             namer,
                             /*immutable_collections*/ false,
                             /*ignore_wrapper*/ false,
                             structured_annotations,
                             force_arrays)
                      << " => "
                      << render_const_value_helper(
                             field.get_type(),
                             v,
                             temp_var_initializations_out,
                             namer,
                             /*immutable_collections*/ false,
                             /*ignore_wrapper*/ false,
                             structured_annotations,
                             force_arrays)
                      << ",\n";
        }
      }
      indent_down();
      indent(out) << ")\n";
      indent_down();
      indent(out) << ")";
    }
    if (wrapper && !ignore_wrapper) {
      out << ")";
    }
  } else if (const auto* tmap = dynamic_cast<const t_map*>(type)) {
    const t_type* ktype = &tmap->key_type().deref();
    const t_type* vtype = tmap->get_val_type();
    if (!hack_collections_ || structured_annotations || force_arrays) {
      out << "dict[\n";
    } else {
      out << (immutable_collections ? "Imm" : "") << "Map {\n";
    }
    indent_up();
    // Workaround to cover the cases where map is initialized with an empty
    // list, e.g. map<string, i32> m = []
    if (!value->is_empty()) {
      for (const auto& entry : value->get_map()) {
        out << indent();
        out << render_const_value_helper(
            ktype,
            entry.first,
            temp_var_initializations_out,
            namer,
            immutable_collections,
            /*ignore_wrapper*/ false,
            structured_annotations,
            force_arrays);
        out << " => ";
        out << render_const_value_helper(
            vtype,
            entry.second,
            temp_var_initializations_out,
            namer,
            immutable_collections,
            /*ignore_wrapper*/ false,
            structured_annotations,
            force_arrays);
        out << ",\n";
      }
    }
    indent_down();
    if (!hack_collections_ || structured_annotations || force_arrays) {
      indent(out) << "]";
    } else {
      indent(out) << "}";
    }
  } else if (const auto* tlist = dynamic_cast<const t_list*>(type)) {
    const t_type* etype = tlist->get_elem_type();
    if (!hack_collections_ || structured_annotations || force_arrays) {
      out << "vec[\n";
    } else {
      out << (immutable_collections ? "Imm" : "") << "Vector {\n";
    }
    indent_up();
    for (const auto* val : value->get_list()) {
      out << indent();
      out << render_const_value_helper(
          etype,
          val,
          temp_var_initializations_out,
          namer,
          immutable_collections,
          /*ignore_wrapper*/ false,
          structured_annotations,
          force_arrays);
      out << ",\n";
    }
    indent_down();
    if (!hack_collections_ || structured_annotations || force_arrays) {
      indent(out) << "]";
    } else {
      indent(out) << "}";
    }
  } else if (const auto* tset = dynamic_cast<const t_set*>(type)) {
    const t_type* etype = tset->get_elem_type();
    indent_up();
    const auto& vals = value->get_list_or_empty_map();
    if ((!legacy_arrays_ && !hack_collections_) || structured_annotations ||
        force_arrays) {
      out << "keyset[\n";
      for (const auto* val : vals) {
        out << indent();
        out << render_const_value_helper(
            etype,
            val,
            temp_var_initializations_out,
            namer,
            immutable_collections,
            /*ignore_wrapper*/ false,
            structured_annotations,
            force_arrays);
        out << ",\n";
      }
      indent_down();
      indent(out) << "]";
    } else if (arraysets_) {
      out << "dict[\n";
      for (const auto* val : vals) {
        out << indent();
        out << render_const_value_helper(
            etype,
            val,
            temp_var_initializations_out,
            namer,
            immutable_collections,
            /*ignore_wrapper*/ false,
            structured_annotations,
            force_arrays);
        out << " => true";
        out << ",\n";
      }
      indent_down();
      indent(out) << "]";
    } else {
      out << (immutable_collections ? "Imm" : "") << "Set {\n";
      for (const auto* val : vals) {
        out << indent();
        out << render_const_value_helper(
            etype,
            val,
            temp_var_initializations_out,
            namer,
            immutable_collections,
            /*ignore_wrapper*/ false,
            structured_annotations,
            force_arrays);
        out << ",\n";
      }
      indent_down();
      indent(out) << "}";
    }
  }
  return out.str();
}

std::string t_hack_generator::render_default_value(const t_type* type) {
  std::string dval;
  type = type->get_true_type();
  if (const auto* tbase_type = type->try_as<t_primitive_type>()) {
    t_primitive_type::type tbase = tbase_type->primitive_type();
    switch (tbase) {
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        dval = "''";
        break;
      case t_primitive_type::type::t_bool:
        dval = "false";
        break;
      case t_primitive_type::type::t_byte:
      case t_primitive_type::type::t_i16:
      case t_primitive_type::type::t_i32:
      case t_primitive_type::type::t_i64:
        dval = "0";
        break;
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        dval = "0.0";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no const of base type " +
            t_primitive_type::type_name(tbase));
    }
  } else if (type->is<t_enum>()) {
    dval = "null";
  } else if (const auto* tstruct = dynamic_cast<const t_structured*>(type)) {
    if (no_nullables_) {
      dval = hack_name(tstruct) + "::withDefaultValues()";
    } else {
      dval = "null";
    }
  } else if (type->is<t_map>()) {
    if (hack_collections_) {
      dval = "Map {}";
    } else {
      dval = "dict[]";
    }
  } else if (type->is<t_list>()) {
    if (hack_collections_) {
      dval = "Vector {}";
    } else {
      dval = "vec[]";
    }
  } else if (type->is<t_set>()) {
    if (arraysets_) {
      dval = "dict[]";
    } else if (hack_collections_) {
      dval = "Set {}";
    } else {
      dval = "keyset[]";
    }
  }
  return dval;
}

t_hack_generator::ThriftPrimitiveType t_hack_generator::base_to_t_primitive(
    const t_primitive_type* tbase) {
  switch (tbase->primitive_type()) {
    case t_primitive_type::type::t_bool:
      return ThriftPrimitiveType::THRIFT_BOOL_TYPE;
    case t_primitive_type::type::t_byte:
      return ThriftPrimitiveType::THRIFT_BYTE_TYPE;
    case t_primitive_type::type::t_i16:
      return ThriftPrimitiveType::THRIFT_I16_TYPE;
    case t_primitive_type::type::t_i32:
      return ThriftPrimitiveType::THRIFT_I32_TYPE;
    case t_primitive_type::type::t_i64:
      return ThriftPrimitiveType::THRIFT_I64_TYPE;
    case t_primitive_type::type::t_float:
      return ThriftPrimitiveType::THRIFT_FLOAT_TYPE;
    case t_primitive_type::type::t_double:
      return ThriftPrimitiveType::THRIFT_DOUBLE_TYPE;
    case t_primitive_type::type::t_binary:
      return ThriftPrimitiveType::THRIFT_BINARY_TYPE;
    case t_primitive_type::type::t_string:
      return ThriftPrimitiveType::THRIFT_STRING_TYPE;
    case t_primitive_type::type::t_void:
      return ThriftPrimitiveType::THRIFT_VOID_TYPE;
    default:
      throw std::invalid_argument(
          "compiler error: no ThriftPrimitiveType mapped to base type " +
          t_primitive_type::type_name(tbase->primitive_type()));
  }
}

std::unique_ptr<t_const_value> t_hack_generator::type_to_tmeta(
    const t_type* type) {
  auto tmeta_ThriftType = t_const_value::make_map();

  if (const auto* tbase_type = type->try_as<t_primitive_type>()) {
    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_primitive"),
        std::make_unique<t_const_value>(base_to_t_primitive(tbase_type)));
  } else if (const auto* tlist = dynamic_cast<const t_list*>(type)) {
    auto tlist_tmeta = t_const_value::make_map();
    tlist_tmeta->add_map(
        std::make_unique<t_const_value>("valueType"),
        type_to_tmeta(tlist->get_elem_type()));

    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_list"), std::move(tlist_tmeta));
  } else if (const auto* tset = dynamic_cast<const t_set*>(type)) {
    auto tset_tmeta = t_const_value::make_map();
    tset_tmeta->add_map(
        std::make_unique<t_const_value>("valueType"),
        type_to_tmeta(tset->get_elem_type()));

    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_set"), std::move(tset_tmeta));
  } else if (const auto* tmap = dynamic_cast<const t_map*>(type)) {
    auto tmap_tmeta = t_const_value::make_map();
    tmap_tmeta->add_map(
        std::make_unique<t_const_value>("keyType"),
        type_to_tmeta(&tmap->key_type().deref()));
    tmap_tmeta->add_map(
        std::make_unique<t_const_value>("valueType"),
        type_to_tmeta(tmap->get_val_type()));

    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_map"), std::move(tmap_tmeta));
  } else if (type->is<t_enum>()) {
    auto tenum_tmeta = t_const_value::make_map();
    tenum_tmeta->add_map(
        std::make_unique<t_const_value>("name"),
        std::make_unique<t_const_value>(type->get_scoped_name()));

    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_enum"), std::move(tenum_tmeta));
  } else if (type->is<t_structured>()) {
    auto tstruct_tmeta = t_const_value::make_map();
    tstruct_tmeta->add_map(
        std::make_unique<t_const_value>("name"),
        std::make_unique<t_const_value>(type->get_scoped_name()));

    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_struct"), std::move(tstruct_tmeta));
  } else if (type->is<t_union>()) {
    auto tunion_tmeta = t_const_value::make_map();
    tunion_tmeta->add_map(
        std::make_unique<t_const_value>("name"),
        std::make_unique<t_const_value>(type->get_scoped_name()));

    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_union"), std::move(tunion_tmeta));
  } else if (const auto* ttypedef = dynamic_cast<const t_typedef*>(type)) {
    auto ttypedef_tmeta = t_const_value::make_map();
    ttypedef_tmeta->add_map(
        std::make_unique<t_const_value>("name"),
        std::make_unique<t_const_value>(ttypedef->get_scoped_name()));
    ttypedef_tmeta->add_map(
        std::make_unique<t_const_value>("underlyingType"),
        type_to_tmeta(ttypedef->get_type()));

    tmeta_ThriftType->add_map(
        std::make_unique<t_const_value>("t_typedef"),
        std::move(ttypedef_tmeta));
  } else {
    // Unsupported type
  }

  return tmeta_ThriftType;
}

std::unique_ptr<t_const_value> t_hack_generator::field_to_tmeta(
    const t_field* field) {
  auto tmeta_ThriftField = t_const_value::make_map();

  tmeta_ThriftField->add_map(
      std::make_unique<t_const_value>("id"),
      std::make_unique<t_const_value>(field->id()));

  tmeta_ThriftField->add_map(
      std::make_unique<t_const_value>("type"),
      type_to_tmeta(field->get_type()));

  tmeta_ThriftField->add_map(
      std::make_unique<t_const_value>("name"),
      std::make_unique<t_const_value>(field->name()));

  if (field->get_req() == t_field::e_req::optional) {
    auto is_optional = std::make_unique<t_const_value>();
    is_optional->set_bool(true);
    tmeta_ThriftField->add_map(
        std::make_unique<t_const_value>("is_optional"), std::move(is_optional));
  }

  return tmeta_ThriftField;
}

std::unique_ptr<t_const_value> t_hack_generator::function_to_tmeta(
    const t_function* function) {
  auto tmeta_ThriftFunction = t_const_value::make_map();

  tmeta_ThriftFunction->add_map(
      std::make_unique<t_const_value>("name"),
      std::make_unique<t_const_value>(find_hack_name(function)));

  auto return_tmeta = std::unique_ptr<t_const_value>();
  if (const auto* sink = function->sink()) {
    auto sink_tmeta = t_const_value::make_map();
    sink_tmeta->add_map(
        std::make_unique<t_const_value>("elemType"),
        type_to_tmeta(sink->get_elem_type()));
    sink_tmeta->add_map(
        std::make_unique<t_const_value>("finalResponseType"),
        type_to_tmeta(sink->get_final_response_type()));
    if (!function->has_void_initial_response()) {
      sink_tmeta->add_map(
          std::make_unique<t_const_value>("initialResponseType"),
          type_to_tmeta(function->return_type().get_type()));
    } else {
      auto first_response_type = t_const_value::make_map();
      first_response_type->add_map(
          std::make_unique<t_const_value>("t_primitive"),
          std::make_unique<t_const_value>(
              ThriftPrimitiveType::THRIFT_VOID_TYPE));
      sink_tmeta->add_map(
          std::make_unique<t_const_value>("initialResponseType"),
          std::move(first_response_type));
    }
    return_tmeta = t_const_value::make_map();
    return_tmeta->add_map(
        std::make_unique<t_const_value>("t_sink"), std::move(sink_tmeta));
  } else if (const t_stream* stream = function->stream()) {
    auto stream_tmeta = t_const_value::make_map();
    stream_tmeta->add_map(
        std::make_unique<t_const_value>("elemType"),
        type_to_tmeta(stream->elem_type().get_type()));
    if (!function->has_void_initial_response()) {
      stream_tmeta->add_map(
          std::make_unique<t_const_value>("initialResponseType"),
          type_to_tmeta(function->return_type().get_type()));
    } else {
      auto first_response_type = t_const_value::make_map();
      first_response_type->add_map(
          std::make_unique<t_const_value>("t_primitive"),
          std::make_unique<t_const_value>(
              ThriftPrimitiveType::THRIFT_VOID_TYPE));
      stream_tmeta->add_map(
          std::make_unique<t_const_value>("initialResponseType"),
          std::move(first_response_type));
    }
    return_tmeta = t_const_value::make_map();
    return_tmeta->add_map(
        std::make_unique<t_const_value>("t_stream"), std::move(stream_tmeta));
  } else {
    return_tmeta = type_to_tmeta(function->return_type().get_type());
  }
  tmeta_ThriftFunction->add_map(
      std::make_unique<t_const_value>("return_type"), std::move(return_tmeta));

  if (function->params().has_fields()) {
    auto arguments = t_const_value::make_list();
    for (const auto& field : function->params().fields()) {
      arguments->add_list(field_to_tmeta(&field));
    }
    tmeta_ThriftFunction->add_map(
        std::make_unique<t_const_value>("arguments"), std::move(arguments));
  }

  if (!t_throws::is_null_or_empty(function->exceptions())) {
    auto exceptions = t_const_value::make_list();
    for (auto&& field : function->exceptions()->fields()) {
      exceptions->add_list(field_to_tmeta(&field));
    }
    tmeta_ThriftFunction->add_map(
        std::make_unique<t_const_value>("exceptions"), std::move(exceptions));
  }

  if (function->qualifier() == t_function_qualifier::oneway) {
    auto is_oneway = std::make_unique<t_const_value>();
    is_oneway->set_bool(true);
    tmeta_ThriftFunction->add_map(
        std::make_unique<t_const_value>("is_oneway"), std::move(is_oneway));
  }

  return tmeta_ThriftFunction;
}

std::unique_ptr<t_const_value> t_hack_generator::service_to_tmeta(
    const t_service* service) {
  auto tmeta_ThriftService = t_const_value::make_map();

  tmeta_ThriftService->add_map(
      std::make_unique<t_const_value>("name"),
      std::make_unique<t_const_value>(service->get_scoped_name()));

  auto functions = get_supported_client_functions(service);
  if (!functions.empty()) {
    auto tmeta_functions = t_const_value::make_list();
    for (const auto& function : functions) {
      if (!skip_codegen(function)) {
        tmeta_functions->add_list(function_to_tmeta(function));
      }
    }
    tmeta_ThriftService->add_map(
        std::make_unique<t_const_value>("functions"),
        std::move(tmeta_functions));
  }

  const t_service* parent = service->extends();
  if (parent) {
    tmeta_ThriftService->add_map(
        std::make_unique<t_const_value>("parent"),
        std::make_unique<t_const_value>(parent->get_scoped_name()));
  }
  return tmeta_ThriftService;
}

std::unique_ptr<t_const_value> t_hack_generator::enum_to_tmeta(
    const t_enum* tenum) {
  auto tmeta_ThriftEnum = t_const_value::make_map();

  tmeta_ThriftEnum->add_map(
      std::make_unique<t_const_value>("name"),
      std::make_unique<t_const_value>(tenum->get_scoped_name()));

  auto const& enum_values = tenum->get_enum_values();
  if (!enum_values.empty()) {
    auto tmeta_elements = t_const_value::make_map();

    for (const auto& constant : tenum->get_enum_values()) {
      tmeta_elements->add_map(
          std::make_unique<t_const_value>(constant->get_value()),
          std::make_unique<t_const_value>(constant->name()));
    }

    tmeta_ThriftEnum->add_map(
        std::make_unique<t_const_value>("elements"), std::move(tmeta_elements));
  }

  return tmeta_ThriftEnum;
}

std::unique_ptr<t_const_value> t_hack_generator::struct_to_tmeta(
    const t_structured* tstruct, bool is_exception) {
  auto tmeta = t_const_value::make_map();

  tmeta->add_map(
      std::make_unique<t_const_value>("name"),
      std::make_unique<t_const_value>(tstruct->get_scoped_name()));

  auto fields = tstruct->get_members();
  if (!fields.empty()) {
    auto tmeta_fields = t_const_value::make_list();
    for (const auto& field : fields) {
      if (!skip_codegen(field)) {
        tmeta_fields->add_list(field_to_tmeta(field));
      }
    }
    tmeta->add_map(
        std::make_unique<t_const_value>("fields"), std::move(tmeta_fields));
  }

  if (!is_exception) {
    auto is_union = std::make_unique<t_const_value>();
    is_union->set_bool(tstruct->is<t_union>());
    tmeta->add_map(
        std::make_unique<t_const_value>("is_union"), std::move(is_union));
  }

  return tmeta;
}

void t_hack_generator::append_to_t_enum(
    t_enum* tenum, t_program* program, ThriftPrimitiveType value) {
  std::unique_ptr<t_enum_value> enum_value;
  switch (value) {
    case ThriftPrimitiveType::THRIFT_BOOL_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_BOOL_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_BYTE_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_BYTE_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_I16_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_I16_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_I32_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_I32_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_I64_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_I64_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_FLOAT_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_FLOAT_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_DOUBLE_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_DOUBLE_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_BINARY_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_BINARY_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_STRING_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_STRING_TYPE");
      break;
    case ThriftPrimitiveType::THRIFT_VOID_TYPE:
      enum_value = std::make_unique<t_enum_value>("THRIFT_VOID_TYPE");
      break;
  }
  enum_value->set_value(value);
  auto const_value = std::make_unique<t_const>(
      program,
      t_type_ref::from_req_ptr(tenum),
      enum_value->name(),
      std::make_unique<t_const_value>(value));
  tenum->append(std::move(enum_value), std::move(const_value));
}

const t_type* t_hack_generator::tmeta_ThriftType_type() {
  static t_program empty_program("", "");
  static t_union type(&empty_program, "tmeta_ThriftType");
  static t_enum primitive_type(&empty_program, "");
  static t_struct list_type(&empty_program, "tmeta_ThriftListType");
  static t_struct set_type(&empty_program, "tmeta_ThriftSetType");
  static t_struct map_type(&empty_program, "tmeta_ThriftMapType");
  static t_struct enum_type(&empty_program, "tmeta_ThriftEnumType");
  static t_struct struct_type(&empty_program, "tmeta_ThriftStructType");
  static t_struct union_type(&empty_program, "tmeta_ThriftUnionType");
  static t_struct typedef_type(&empty_program, "tmeta_ThriftTypedefType");
  static t_struct stream_type(&empty_program, "tmeta_ThriftStreamType");
  static t_struct sink_type(&empty_program, "tmeta_ThriftSinkType");
  if (type.has_fields()) {
    return &type;
  }

  primitive_type.set_name("tmeta_ThriftPrimitiveType");
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_BOOL_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_BYTE_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_I16_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_I32_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_I64_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_FLOAT_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_BINARY_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_STRING_TYPE);
  append_to_t_enum(
      &primitive_type, &empty_program, ThriftPrimitiveType::THRIFT_VOID_TYPE);

  list_type.append(std::make_unique<t_field>(type, "valueType"));
  set_type.append(std::make_unique<t_field>(type, "valueType"));
  map_type.append(std::make_unique<t_field>(type, "keyType"));
  map_type.append(std::make_unique<t_field>(type, "valueType"));
  enum_type.append(
      std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  struct_type.append(
      std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  union_type.append(
      std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  typedef_type.append(
      std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  typedef_type.append(std::make_unique<t_field>(type, "underlyingType"));
  stream_type.append(std::make_unique<t_field>(type, "elemType"));
  stream_type.append(std::make_unique<t_field>(type, "initialResponseType"));
  sink_type.append(std::make_unique<t_field>(type, "elemType"));
  sink_type.append(std::make_unique<t_field>(type, "finalResponseType"));
  sink_type.append(std::make_unique<t_field>(type, "initialResponseType"));

  type.append(std::make_unique<t_field>(primitive_type, "t_primitive"));
  type.append(std::make_unique<t_field>(list_type, "t_list"));
  type.append(std::make_unique<t_field>(set_type, "t_set"));
  type.append(std::make_unique<t_field>(map_type, "t_map"));
  type.append(std::make_unique<t_field>(enum_type, "t_enum"));
  type.append(std::make_unique<t_field>(struct_type, "t_struct"));
  type.append(std::make_unique<t_field>(union_type, "t_union"));
  type.append(std::make_unique<t_field>(typedef_type, "t_typedef"));
  type.append(std::make_unique<t_field>(stream_type, "t_stream"));
  type.append(std::make_unique<t_field>(sink_type, "t_sink"));
  return &type;
}

const t_type* t_hack_generator::tmeta_ThriftField_type() {
  static t_program empty_program("", "");
  static t_struct type(&empty_program, "tmeta_ThriftField");
  if (type.has_fields()) {
    return &type;
  }

  type.append(std::make_unique<t_field>(t_primitive_type::t_i64(), "id"));
  type.append(std::make_unique<t_field>(*tmeta_ThriftType_type(), "type"));
  type.append(std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  type.append(
      std::make_unique<t_field>(t_primitive_type::t_bool(), "is_optional"));
  return &type;
}

const t_type* t_hack_generator::tmeta_ThriftFunction_type() {
  static t_program empty_program("", "");
  static t_struct type(&empty_program, "tmeta_ThriftFunction");
  static t_list tlist(*tmeta_ThriftField_type());
  if (type.has_fields()) {
    return &type;
  }

  type.append(std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  type.append(
      std::make_unique<t_field>(*tmeta_ThriftType_type(), "return_type"));
  type.append(std::make_unique<t_field>(tlist, "arguments"));
  type.append(std::make_unique<t_field>(tlist, "exceptions"));
  type.append(
      std::make_unique<t_field>(t_primitive_type::t_bool(), "is_oneway"));
  return &type;
}

const t_type* t_hack_generator::tmeta_ThriftService_type() {
  static t_program empty_program("", "");
  static t_struct type(&empty_program, "tmeta_ThriftService");
  static t_list tlist(*tmeta_ThriftFunction_type());
  if (type.has_fields()) {
    return &type;
  }

  type.append(std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  type.append(std::make_unique<t_field>(tlist, "functions"));
  type.append(
      std::make_unique<t_field>(t_primitive_type::t_string(), "parent"));
  return &type;
}

const t_type* t_hack_generator::tmeta_ThriftEnum_type() {
  static t_program empty_program("", "");
  static t_struct type(&empty_program, "tmeta_ThriftEnum");
  static t_map tmap(t_primitive_type::t_i32(), t_primitive_type::t_string());
  if (type.has_fields()) {
    return &type;
  }

  type.append(std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  type.append(std::make_unique<t_field>(tmap, "elements"));
  return &type;
}

const t_type* t_hack_generator::tmeta_ThriftStruct_type() {
  static t_program empty_program("", "");
  static t_struct type(&empty_program, "tmeta_ThriftStruct");
  static t_list tlist(*tmeta_ThriftField_type());
  if (type.has_fields()) {
    return &type;
  }

  type.append(std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  type.append(std::make_unique<t_field>(tlist, "fields"));
  type.append(
      std::make_unique<t_field>(t_primitive_type::t_bool(), "is_union"));
  return &type;
}

const t_type* t_hack_generator::tmeta_ThriftException_type() {
  static t_program empty_program("", "");
  static t_struct type(&empty_program, "tmeta_ThriftException");
  static t_list tlist(*tmeta_ThriftField_type());
  if (type.has_fields()) {
    return &type;
  }

  type.append(std::make_unique<t_field>(t_primitive_type::t_string(), "name"));
  type.append(std::make_unique<t_field>(tlist, "fields"));
  return &type;
}

/**
 * Make a struct
 */
void t_hack_generator::generate_struct(const t_structured* tstruct) {
  auto [wrapper, name, ns] = find_hack_wrapper(tstruct, false);
  if (wrapper) {
    // For wrapped typedef, typehint is generated in nested ns.
    // Set this flag to true to ensure that any references to other typedefs or
    // structs is prefixed with "\"
    has_nested_ns = true;
    if (ns) {
      f_adapted_types_ << "namespace " << *ns << " {\n\n";
    }
    indent_up();
    generate_php_struct_definition(
        f_adapted_types_, tstruct, ThriftStructType::STRUCT);
    indent_down();
    if (ns) {
      f_adapted_types_ << "\n}\n\n";
    }
    // reset the flag
    has_nested_ns = false;
  } else {
    generate_php_struct_definition(f_types_, tstruct);
  }
}

bool t_hack_generator::is_valid_hack_type(
    const t_type* type, const t_type_ref& top_level_type) {
  type = type->get_true_type();
  if (!type->is<t_container>()) {
    return true;
  }

  auto it = type_validity_.find(type);
  if (it != type_validity_.end()) {
    return it->second;
  }

  auto report_invalid_type = [&](const char* what, const t_type& type) {
    diags_.error(
        top_level_type,
        "`{}` cannot be used as a {} in Hack because it is not "
        "integer, string, binary or enum",
        type.name(),
        what);
  };

  bool valid = false;
  if (const t_list* list = dynamic_cast<const t_list*>(type)) {
    valid = is_valid_hack_type(&*list->elem_type(), top_level_type);
  } else if (const t_set* set = dynamic_cast<const t_set*>(type)) {
    const t_type& elem_type = *set->elem_type();
    valid = is_type_arraykey(elem_type);
    if (!valid) {
      report_invalid_type("set element", elem_type);
    }
  } else {
    const t_map* map = dynamic_cast<const t_map*>(type);
    assert(map);
    const t_type& key_type = *map->key_type();
    valid = is_type_arraykey(key_type);
    if (!valid) {
      report_invalid_type("map key", key_type);
    }
    valid = valid && is_valid_hack_type(&*map->val_type(), top_level_type);
  }
  type_validity_[type] = valid;
  return valid;
}

bool t_hack_generator::skip_codegen(const t_field* field) {
  auto skip_codegen = field->has_structured_annotation(kHackSkipCodegenUri);
  if (!skip_codegen) {
    skip_codegen = t_typedef::get_first_structured_annotation_or_null(
        field->get_type(), kHackSkipCodegenUri);
  }
  return skip_codegen || !is_valid_hack_type(field->type());
}

bool t_hack_generator::skip_codegen(const t_function* function) {
  if (function->has_structured_annotation(kHackSkipCodegenUri)) {
    return true;
  }
  bool valid = is_valid_hack_type(function->return_type());
  for (const auto& field : function->params().fields()) {
    valid &= is_valid_hack_type(field.type());
  }
  return !valid;
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_hack_generator::generate_xception(const t_structured* txception) {
  generate_php_struct_definition(
      f_types_, txception, ThriftStructType::EXCEPTION);
}

void t_hack_generator::generate_php_type_spec(
    std::ofstream& out, const t_type* t, uint32_t depth) {
  // Check the adapter before resolving typedefs.
  if (std::optional<std::string> adapter = find_hack_adapter(t)) {
    indent(out) << "'adapter' => " << *adapter << "::class,\n";
  }

  // Check the wrapper before resolving typedefs.
  auto [owrapper, oname, ons] = find_hack_wrapper(t);
  if (owrapper) {
    indent(out) << "'is_type_wrapped' => true,\n";
  }
  t = t->get_true_type();
  indent(out) << "'type' => " << type_to_enum(t) << ",\n";
  if (const auto* tbase_type = t->try_as<t_primitive_type>()) {
    if (tbase_type->primitive_type() == t_primitive_type::type::t_binary) {
      indent(out) << "'is_binary' => true,\n";
    }
  }

  if (t->is<t_primitive_type>()) {
    // Noop, type is all we need
  } else if (t->is<t_enum>()) {
    indent(out) << "'enum' => " << hack_name(t) << "::class,\n";
  } else if (const auto* tstruct = dynamic_cast<const t_structured*>(t)) {
    auto sname = hack_name(t);
    auto [wrapper, name, ns] = find_hack_wrapper(tstruct);
    if (wrapper) {
      sname = hack_wrapped_type_name(name, ns);
    }
    indent(out) << "'class' => " << sname << "::class,\n";
  } else if (const auto* tmap = dynamic_cast<const t_map*>(t)) {
    const t_type* ktype = &tmap->key_type().deref();
    const t_type* vtype = tmap->get_val_type();
    if (find_hack_adapter(ktype)) {
      throw std::runtime_error(
          "using hack.Adapter annotation with map keys is not supported yet");
    }
    auto [wrapper, name, ns] = find_hack_wrapper(ktype);
    if (wrapper) {
      throw std::runtime_error(
          "using hack.Wrapper annotation with map keys is not supported yet");
    }
    indent(out) << "'ktype' => " << type_to_enum(ktype) << ",\n";
    indent(out) << "'vtype' => " << type_to_enum(vtype) << ",\n";
    generate_php_type_spec_shape_elt_helper(out, "key", ktype, depth);
    generate_php_type_spec_shape_elt_helper(out, "val", vtype, depth);
    if (legacy_arrays_) {
      indent(out) << "'format' => 'array',\n";
    } else if (hack_collections_) {
      indent(out) << "'format' => 'collection',\n";
    } else {
      indent(out) << "'format' => 'harray',\n";
    }
  } else if (const auto* tlist = dynamic_cast<const t_list*>(t)) {
    const t_type* etype = tlist->get_elem_type();
    indent(out) << "'etype' => " << type_to_enum(etype) << ",\n";
    generate_php_type_spec_shape_elt_helper(out, "elem", etype, depth);
    if (legacy_arrays_) {
      indent(out) << "'format' => 'array',\n";
    } else if (hack_collections_) {
      indent(out) << "'format' => 'collection',\n";
    } else {
      indent(out) << "'format' => 'harray',\n";
    }
  } else if (const auto* tset = dynamic_cast<const t_set*>(t)) {
    const t_type* etype = tset->get_elem_type();
    if (find_hack_adapter(etype)) {
      throw std::runtime_error(
          "using hack.Adapter annotation with set keys is not supported yet");
    }
    auto [wrapper, name, ns] = find_hack_wrapper(etype);
    if (wrapper) {
      throw std::runtime_error(
          "using hack.Wrapper annotation with set keys is not supported yet");
    }
    indent(out) << "'etype' => " << type_to_enum(etype) << ",\n";
    generate_php_type_spec_shape_elt_helper(out, "elem", etype, depth);
    if (arraysets_) {
      indent(out) << "'format' => 'array',\n";
    } else if (hack_collections_) {
      indent(out) << "'format' => 'collection',\n";
    } else {
      indent(out) << "'format' => 'harray',\n";
    }
  } else {
    throw std::runtime_error(
        "compiler error: no type for php struct spec field");
  }
}

void t_hack_generator::generate_php_type_spec_shape_elt_helper(
    std::ofstream& out,
    const std::string& field_name,
    const t_type* t,
    uint32_t depth) {
  indent(out) << "'" << field_name << "' => ";
  if (depth >= min_depth_for_unsafe_cast_) {
    out << R"(\HH\FIXME\UNSAFE_CAST<mixed, \HH_FIXME\NON_DENOTABLE_TYPE>()";
  }
  out << "shape(\n";
  indent_up();
  generate_php_type_spec(out, t, depth + 1);
  indent_down();
  if (depth >= min_depth_for_unsafe_cast_) {
    indent(out) << ")),\n";
  } else {
    indent(out) << "),\n";
  }
}

/**
 * Generates the struct specification structure, which fully qualifies enough
 * type information to generalize serialization routines.
 */
void t_hack_generator::generate_php_struct_spec(
    std::ofstream& out, const t_structured* tstruct) {
  indent(out) << "const \\ThriftStructTypes::TSpec SPEC = dict[\n";
  indent_up();
  const auto fields =
      tstruct->has_structured_annotation(kSerializeInFieldIdOrderUri)
      ? tstruct->fields_id_order()
      : tstruct->fields().copy();
  for (const auto* field_ptr : fields) {
    if (skip_codegen(field_ptr)) {
      continue;
    }
    const auto& field = *field_ptr;
    const t_type& t = *field.type();
    indent(out) << field.id() << " => shape(\n";
    indent_up();
    out << indent() << "'var' => '" << field.name() << "',\n";
    if (tstruct->is<t_union>()) {
      // Optimally, we shouldn't set this per field but rather per struct.
      // However, the tspec is a field_id => data array, and if we set it
      // at the top level people might think the 'union' key is a field
      // id, which isn't cool. It's safer and more bc to instead set this
      // key on all fields.
      out << indent() << "'union' => true,\n";
    }
    if (find_hack_wrapper(field)) {
      indent(out) << "'is_wrapped' => true,\n";
    }
    if (std::optional<std::string> adapter = find_hack_field_adapter(field)) {
      indent(out) << "'adapter' => " << *adapter << "::class,\n";
    }
    if (field.qualifier() == t_field_qualifier::terse) {
      indent(out) << "'is_terse' => true,\n";
    }
    generate_php_type_spec(out, &t, /* depth */ 0);
    indent_down();
    indent(out) << "),\n";
  }
  indent_down();
  indent(out) << "];\n";

  indent(out) << "const dict<string, int> FIELDMAP = dict[\n";
  indent_up();
  for (const auto& field : fields) {
    if (!skip_codegen(field)) {
      indent(out) << "'" << field->name() << "' => " << field->id() << ",\n";
    }
  }
  indent_down();
  indent(out) << "];\n";
}

void t_hack_generator::generate_php_struct_struct_trait(
    std::ofstream& out, const t_structured* tstruct, const std::string& name) {
  std::string traitName;
  auto struct_trait = find_hack_struct_trait(tstruct);
  if (struct_trait.first) {
    if (struct_trait.second.empty()) {
      traitName = hack_name(name, tstruct->program()) + "Trait";
    } else {
      traitName = unescape(struct_trait.second);
      if (struct_trait.second.find('\\') == std::string::npos) {
        traitName = hack_name(traitName, tstruct->program());
      }
    }
  }

  if (!traitName.empty()) {
    indent(out) << "use " << traitName << ";\n\n";
  }
}

void t_hack_generator::generate_php_struct_shape_spec(
    std::ofstream& out,
    const t_structured* tstruct,
    bool is_constructor_shape) {
  indent(out) << "const type "
              << (is_constructor_shape ? "TConstructorShape" : "TShape")
              << " = shape(\n";
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const t_type* t = field.get_type();
    // Compute typehint before resolving typedefs to avoid missing any adapter
    // annotations.
    std::string typehint;
    if (const auto field_adapter = find_hack_field_adapter(field)) {
      typehint = *field_adapter + "::THackType";
    } else {
      typehint = type_to_typehint(
          t,
          {{TypeToTypehintVariations::IS_SHAPE, !is_constructor_shape},
           {TypeToTypehintVariations::IS_ANY_SHAPE, true},
           {TypeToTypehintVariations::IGNORE_WRAPPER, true},
           {TypeToTypehintVariations::RECURSIVE_IGNORE_WRAPPER, true}});
    }
    bool nullable = nullable_everything_ || field_is_nullable(tstruct, &field);

    std::string prefix = nullable || is_constructor_shape ? "?" : "";

    if (tstruct->is<t_result_struct>() && field.name() == "success") {
      if (!is_async_field(field, false)) {
        typehint = "this::TResult";
      }
    }

    indent(out) << "  " << prefix << "'" << field.name() << "' => " << prefix
                << typehint << ",\n";
  }
  if (!is_constructor_shape && shapes_allow_unknown_fields_) {
    indent(out) << "  ...\n";
  }
  indent(out) << ");\n";
}

/**
 * Generate a Lambda on a Collection Value.
 *
 * For example, if our structure is:
 *
 * 1: map<string, list<i32>> map_of_string_to_list_of_i32;
 *
 * Then our __toShape() routine results in:
 *
 *   'map_of_string_to_list_of_i32' =>
 * $this->map_of_string_to_list_of_i32->map(
 *     $_val0 ==> $_val0->toVArray(),
 *   )
 *     |> ThriftUtil::toDArray($$),
 *
 * And this method here will get called with
 *
 *  generate_php_struct_shape_collection_value_lambda(..., list<i32>)
 *
 * And returns the string:
 *
 *   "  $_val0 ==> $_val0->toVArray(),"
 *
 * This method operates via recursion on complex types.
 */
void t_hack_generator::generate_php_struct_shape_collection_value_lambda(
    std::ostream& out, t_name_generator& namer, const t_type* t) {
  std::string tmp = namer("_val");
  indent(out);
  out << "($" << tmp << ") ==> ";
  if (t->is<t_struct>() || t->is<t_union>()) {
    out << "$" << tmp << "->__toShape(),\n";
  } else if (t->is<t_set>()) {
    if (arraysets_ || legacy_arrays_) {
      out << "dict($" << tmp << "),\n";
    } else {
      out << "ThriftUtil::toDArray(Dict\\fill_keys($" << tmp
          << ", true), static::class),\n";
    }
  } else if (t->is<t_map>() || t->is<t_list>()) {
    const t_type* val_type = nullptr;
    if (const t_map* map = t->try_as<t_map>()) {
      val_type = map->get_val_type();
    } else {
      val_type = static_cast<const t_list*>(t)->get_elem_type();
    }
    val_type = val_type->get_true_type();

    if (!val_type->is<t_container>() && !val_type->is<t_struct>() &&
        !val_type->is<t_union>()) {
      out << generate_to_array_method(t, "$" + tmp) << ",\n";
      return;
    }

    out << "$" << tmp << "->map(\n";
    indent_up();
    generate_php_struct_shape_collection_value_lambda(out, namer, val_type);
    indent_down();
    indent(out) << ")\n";
    indent_up();
    indent(out) << "|> " << generate_to_array_method(t, "$$") << ",\n";
    indent_down();
  }
}

void t_hack_generator::generate_hack_array_from_shape_lambda(
    std::ostream& out, t_name_generator& namer, const t_map* t) {
  bool stringify_map_keys = false;
  if (shape_arraykeys_) {
    const t_type* key_type = t->key_type()->get_true_type();
    if (key_type->is<t_primitive_type>() && key_type->is_string_or_binary()) {
      stringify_map_keys = true;
    }
  }
  std::stringstream inner;
  const t_type* val_type = t->get_val_type()->get_true_type();
  indent_up();
  indent_up();
  generate_hack_array_from_shape_lambda(inner, namer, val_type);
  indent_down();
  indent_down();
  auto str = inner.str();

  if (str.empty()) {
    if (stringify_map_keys) {
      out << "\n";
      indent_up();
      out << indent() << "|> self::__stringifyMapKeys($$)";
      indent_down();
    }
  } else {
    out << "\n";
    indent_up();
    out << indent() << "|> Dict\\map(\n";
    indent_up();
    out << indent()
        << (stringify_map_keys ? "self::__stringifyMapKeys($$)" : "$$")
        << ",\n";
    std::string tmp = namer("_val");
    indent(out) << "$" << tmp << " ==> $" << tmp;
    out << str << ",\n";
    indent_down();
    indent(out) << ")";
    indent_down();
  }
  if (hack_collections_) {
    out << " |> new Map($$)";
  }
}

void t_hack_generator::generate_hack_array_from_shape_lambda(
    std::ostream& out, t_name_generator& namer, const t_list* t) {
  std::stringstream inner;
  const t_type* val_type = t->get_elem_type()->get_true_type();
  indent_up();
  indent_up();
  generate_hack_array_from_shape_lambda(inner, namer, val_type);
  indent_down();
  indent_down();
  auto str = inner.str();
  if (!str.empty()) {
    out << "\n";
    indent_up();
    out << indent() << "|> Vec\\map(\n";
    indent_up();
    out << indent() << "$$,\n";
    std::string tmp = namer("_val");
    indent(out) << "$" << tmp << " ==> $" << tmp;
    indent_up();
    out << str << ",\n";
    indent_down();
    indent_down();
    indent(out) << ")";
    indent_down();
  }
  if (hack_collections_) {
    out << " |> new Vector($$)";
  }
}

void t_hack_generator::generate_hack_array_from_shape_lambda(
    std::ostream& out, t_name_generator&, const t_set*) {
  if (hack_collections_ && !arraysets_) {
    out << "\n";
    indent_up();
    indent(out) << "|> new Set(Keyset\\keys($$))";
    indent_down();
  }
}

void t_hack_generator::generate_hack_array_from_shape_lambda(
    std::ostream& out, t_name_generator&, const t_structured* t) {
  out << "\n";
  indent_up();
  indent(out) << "|> ";
  std::string type = hack_name(t);
  out << type << "::__fromShape($$)";
  indent_down();
}

void t_hack_generator::generate_hack_array_from_shape_lambda(
    std::ostream& out, t_name_generator& namer, const t_type* t) {
  if (const t_map* map = t->try_as<t_map>()) {
    generate_hack_array_from_shape_lambda(out, namer, map);
  } else if (const t_list* list = t->try_as<t_list>()) {
    generate_hack_array_from_shape_lambda(out, namer, list);
  } else if (const t_structured* structured = t->try_as<t_structured>()) {
    generate_hack_array_from_shape_lambda(out, namer, structured);
  } else if (const t_set* set = t->try_as<t_set>()) {
    generate_hack_array_from_shape_lambda(out, namer, set);
  }
}

void t_hack_generator::generate_shape_from_hack_array_lambda(
    std::ostream& out, t_name_generator& namer, const t_type* t) {
  if (t->is<t_map>()) {
    out << "Dict\\map(\n";
  } else {
    out << "Vec\\map(\n";
  }
  indent_up();
  indent(out) << "$$,\n";
  std::string tmp = namer("_val");
  indent(out);
  out << "($" << tmp << ") ==> $" << tmp;

  const t_type* val_type = nullptr;
  if (const t_map* map = t->try_as<t_map>()) {
    val_type = map->get_val_type();
  } else {
    val_type = static_cast<const t_list*>(t)->get_elem_type();
  }
  val_type = val_type->get_true_type();

  if (val_type->is<t_struct>() || val_type->is<t_union>()) {
    out << "->__toShape(),\n";
  } else if (val_type->is<t_map>() || val_type->is<t_list>()) {
    indent_up();
    out << "\n";
    indent(out) << "|> ";
    generate_shape_from_hack_array_lambda(out, namer, val_type);
    indent_down();
  } else {
    out << ",\n";
  }

  indent_down();
  indent(out) << "),\n";
}

bool t_hack_generator::type_has_nested_struct(const t_type* t) {
  bool has_struct = false;
  const t_type* val_type = t;
  while (true) {
    if (const t_map* map = val_type->try_as<t_map>()) {
      val_type = map->get_val_type();
    } else {
      val_type = static_cast<const t_list*>(val_type)->get_elem_type();
    }
    val_type = val_type->get_true_type();

    if (!(val_type->is<t_map>() || val_type->is<t_list>())) {
      if (val_type->is<t_struct>() || val_type->is<t_union>()) {
        has_struct = true;
      }
      break;
    }
  }

  return has_struct;
}

/**
 * Determine whether a field should be marked nullable.
 */
bool t_hack_generator::field_is_nullable(
    const t_structured* tstruct, const t_field* field) {
  std::string dval;
  const t_type* t = field->type()->get_true_type();
  if (field->default_value() != nullptr && !(t->is<t_structured>())) {
    dval = render_const_value(
        t,
        field->default_value(),
        /*immutable_collections*/ false,
        /*ignore_wrapper*/ false,
        /*force_arrays*/ false);
  } else {
    dval = render_default_value(t);
  }

  return (dval == "null") || tstruct->is<t_union>() ||
      (field->get_req() == t_field::e_req::optional &&
       field->default_value() == nullptr) ||
      (t->is<t_enum>() && field->get_req() != t_field::e_req::required);
}

void t_hack_generator::generate_php_struct_stringifyMapKeys_method(
    std::ofstream& out) {
  if (!shape_arraykeys_) {
    return;
  }

  indent(out) << "public static function __stringifyMapKeys<T>("
              << "dict<arraykey, T> $m)[]: " << "dict<string, T> {\n";
  indent_up();
  indent(out) << "return Dict\\map_keys($m, $key ==> (string)$key);\n";
  indent_down();
  indent(out) << "}\n\n";
}

void t_hack_generator::generate_php_struct_shape_methods(
    std::ofstream& out, const t_structured* tstruct) {
  generate_php_struct_stringifyMapKeys_method(out);

  indent(out)
      << "public static function __fromShape(self::TShape $shape)[]: this {\n";
  indent_up();
  indent(out) << "return new static(\n";
  indent_up();

  t_name_generator namer;
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const t_type* t = field.type()->get_true_type();

    bool nullable = field_is_nullable(tstruct, &field) || nullable_everything_;

    bool use_pipe = tstruct->is<t_union>() || nullable;

    std::stringstream source;
    if (use_pipe) {
      source << "$$";
    } else {
      source << "$shape['" << field.name() << "']";
    }

    std::stringstream inner;
    bool is_simple_shape_index = true;
    if (find_hack_field_adapter(field) || find_hack_adapter(field.get_type())) {
      inner << source.str();
    } else if (t->is<t_set>()) {
      if (arraysets_ || !hack_collections_) {
        inner << source.str();
      } else {
        is_simple_shape_index = false;
        inner << "new Set(Keyset\\keys(" << source.str() << "))";
      }
    } else if (t->is<t_map>() || t->is<t_list>()) {
      inner << source.str();
      std::stringstream inner_;
      generate_hack_array_from_shape_lambda(inner_, namer, t);
      auto str = inner_.str();
      if (!str.empty()) {
        inner << str;
        is_simple_shape_index = false;
      }
    } else if (t->is<t_struct>() || t->is<t_union>()) {
      is_simple_shape_index = false;
      std::string type = hack_name(t);
      inner << type << "::__fromShape(" << source.str() << ")";
    } else {
      inner << source.str();
    }

    std::stringstream val;
    indent(val);
    if (use_pipe) {
      val << "Shapes::idx($shape, '" << field.name() << "')";
      if (!is_simple_shape_index) {
        val << " |> $$";
        val << " === null ? null : (" << inner.str() << ")";
      }
    } else {
      val << inner.str();
    }
    val << ",\n";
    out << val.str();
  }
  indent_down();
  indent(out) << ");\n";
  indent_down();
  indent(out) << "}\n";
  out << "\n";

  indent(out) << "public function __toShape()[]: self::TShape {\n";
  indent_up();

  indent(out) << "return shape(\n";
  indent_up();

  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const t_type* t = field.type()->get_true_type();
    t_name_generator ngen;

    indent(out) << "'" << field.name() << "' => ";

    std::stringstream val;

    bool nullable = field_is_nullable(tstruct, &field) || nullable_everything_;
    auto fieldRef = "$this->" + field.name();
    if (find_hack_field_adapter(field) || find_hack_adapter(field.get_type())) {
      val << fieldRef << ",\n";
    } else if (t->is<t_container>()) {
      if (t->is<t_map>() || t->is<t_list>()) {
        if (hack_collections_) {
          const t_type* val_type = nullptr;
          if (const t_map* map = t->try_as<t_map>()) {
            val_type = map->get_val_type();
          } else {
            val_type = static_cast<const t_list*>(t)->get_elem_type();
          }
          val_type = val_type->get_true_type();

          if (val_type->is<t_container>() || val_type->is<t_struct>() ||
              val_type->is<t_union>() || nullable) {
            val << fieldRef;
            if (val_type->is<t_container>() || val_type->is<t_struct>() ||
                val_type->is<t_union>()) {
              val << (nullable ? "?" : "") << "->map(\n";
              indent_up();
              generate_php_struct_shape_collection_value_lambda(
                  val, ngen, val_type);
              indent_down();
              indent(val) << ")";
            }
            val << '\n';
            indent_up();
            indent(val) << "|> " << (nullable ? "$$ === null ? null : " : "")
                        << generate_to_array_method(t, "$$") << ",\n";
            indent_down();
          } else {
            val << generate_to_array_method(t, fieldRef) << ",\n";
          }
        } else {
          val << fieldRef;
          if (type_has_nested_struct(t)) {
            val << "\n";
            indent_up();
            indent(val) << "|> ";
            if (nullable) {
              val << "$$ === null \n";
              indent_up();
              indent(val) << "? null \n";
              indent(val) << ": ";
            }
            generate_shape_from_hack_array_lambda(val, ngen, t);
            if (nullable) {
              indent_down();
            }
            indent_down();
          } else {
            val << ",\n";
          }
        }
      } else if (arraysets_ || !hack_collections_) {
        val << fieldRef << ",\n";
      } else {
        if (nullable) {
          val << fieldRef << "\n";
          indent_up();
          indent(val) << "|> $$ === null ? null : ";
        }
        val << "ThriftUtil::toDArray(Dict\\fill_keys(";
        if (nullable) {
          val << "$$";
        } else {
          val << fieldRef;
        }
        val << "->toValuesArray(), true), static::class),\n";
        if (nullable) {
          indent_down();
        }
      }
    } else if (t->is<t_struct>() || t->is<t_union>()) {
      val << fieldRef;
      val << (nullable ? "?" : "") << "->__toShape(),\n";
    } else {
      val << fieldRef << ",\n";
    }

    out << val.str();
  }
  indent_down();
  indent(out) << ");\n";
  indent_down();
  indent(out) << "}\n";
}

bool t_hack_generator::
    generate_php_struct_async_struct_creation_method_field_assignment_helper(
        std::ostream& out,
        const t_type* ttype,
        t_name_generator& namer,
        const std::string& val,
        bool is_shape_method,
        bool uses_thrift_only_methods) {
  if (const auto* ttypedef =
          dynamic_cast<const t_placeholder_typedef*>(ttype)) {
    ttype = ttypedef->get_type();
  }
  if (std::optional<std::string> adapter = find_hack_adapter(ttype)) {
    out << val;
    return false;
  }
  if (is_shape_method) {
    if (const auto* tstruct = dynamic_cast<const t_structured*>(ttype)) {
      bool is_async = is_async_shapish_struct(tstruct);
      auto [wrapper, name, ns] = find_hack_wrapper(tstruct);
      std::string struct_name;
      if (wrapper) {
        struct_name = hack_wrapped_type_name(name, ns);
      } else {
        struct_name = hack_name(tstruct);
      }
      if (is_async) {
        out << "await " << struct_name << "::__genFromShape(" << val << ")";
      } else {
        out << struct_name << "::__fromShape(" << val << ")";
      }
      return is_async;
    }
  }
  if (ttype->is<t_container>()) {
    if (ttype->is<t_set>()) {
      if (arraysets_ || !hack_collections_) {
        out << val;
      } else {
        out << "new Set(Keyset\\keys(" << val << "))";
      }
      return false;
    }
    std::string prefix;
    std::string suffix;
    std::string container_type;
    bool stringify_map_keys = false;
    const t_type* val_type = nullptr;

    if (const t_map* map = ttype->try_as<t_map>()) {
      container_type = "Dict\\";
      if (shape_arraykeys_) {
        const t_type* key_type = map->key_type().deref().get_true_type();
        if (is_shape_method && key_type->is<t_primitive_type>() &&
            key_type->is_string_or_binary()) {
          stringify_map_keys = true;
          indent_up();
          out << "self::__stringifyMapKeys(\n" << indent();
        }
      }
      if (hack_collections_) {
        prefix = prefix + "new Map(";
        suffix = ")" + suffix;
      }
      val_type = map->get_val_type();
    } else {
      container_type = "Vec\\";
      if (hack_collections_) {
        prefix = "new Vector(";
        suffix = ")";
      }
      val_type = static_cast<const t_list*>(ttype)->get_elem_type();
    }

    if (const auto* ttypedef =
            dynamic_cast<const t_placeholder_typedef*>(val_type)) {
      val_type = ttypedef->get_type();
    }
    auto [wrapper, name, ns] = find_hack_wrapper(val_type);
    std::stringstream inner;
    std::string inner_val = namer("$val");
    indent_up();
    indent_up();
    bool is_async_val =
        generate_php_struct_async_struct_creation_method_field_assignment_helper(
            inner,
            val_type,
            namer,
            inner_val,
            is_shape_method,
            uses_thrift_only_methods);
    indent_down();
    auto inner_str = inner.str();
    if (!wrapper && inner_val == inner_str) {
      // Since value doesn't need mapping, 'inner_val' will be unused.
      // Decrement the counter so that we don't skip namer values.
      namer.decrement_counter();
      indent_down();
      if (is_shape_method) {
        out << prefix << val << suffix;
      } else {
        out << val;
      }
      if (stringify_map_keys) {
        indent_down();
        out << "\n" << indent() << ")";
      }
      return false;
    }
    bool use_map_async = (wrapper && !uses_thrift_only_methods) || is_async_val;

    out << prefix << (use_map_async ? "await " : "") << container_type
        << (use_map_async ? "map_async" : "map") << "(\n";

    out << indent() << val << ",\n"
        << indent() << (use_map_async ? "async " : "") << inner_val
        << " ==> \n";

    indent_up();
    if (wrapper) {
      std::string typehint = type_to_typehint(
          val_type, {{TypeToTypehintVariations::IGNORE_WRAPPER, true}});
      auto wrapper_method = "await " + *wrapper + "::genFromThrift<";
      if (uses_thrift_only_methods) {
        wrapper_method = *wrapper + "::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<";
      }

      if (is_async_val && name.has_value()) {
        out << indent() << " {\n";
        indent_up();
        out << indent() << "$" << *name << " = " << inner_str << ";\n";
        out << indent() << "return " << wrapper_method << typehint << ">($"
            << *name << ");\n";
        indent_down();
        out << indent() << "}";
      } else {
        out << indent() << wrapper_method << typehint << ">(" << inner_str
            << ")\n";
      }
    } else {
      out << indent() << inner_str << "\n";
    }
    indent_down();
    indent_down();
    out << indent() << ")" << suffix;

    if (stringify_map_keys) {
      indent_down();
      out << "\n" << indent() << ")";
    }
    return use_map_async;
  } else {
    out << val;
    return false;
  }
}

bool t_hack_generator::generate_php_struct_async_toShape_method_helper(
    std::ostream& out,
    const t_type* ttype,
    t_name_generator& namer,
    const std::string& val) {
  if (find_hack_adapter(ttype)) {
    out << val;
    return false;
  }
  if (const auto* tstruct = dynamic_cast<const t_structured*>(ttype)) {
    if (is_async_shapish_struct(tstruct)) {
      out << val << "->__genToShape()";
      return true;
    } else {
      out << val << "->__toShape()";
      return false;
    }
  } else if (ttype->is<t_container>()) {
    if (ttype->is<t_set>()) {
      if (arraysets_ || !hack_collections_) {
        out << val;
      } else {
        out << "ThriftUtil::toDArray(Dict\\fill_keys(";
        out << val;
        out << "->toValuesArray(), true), static::class)";
      }
      return false;
    }
    std::string container_prefix;
    bool use_to_darray_conv = false;
    const t_type* val_type = nullptr;

    if (const t_map* map = ttype->try_as<t_map>()) {
      container_prefix = "Dict\\";
      if (array_migration_ && hack_collections_) {
        use_to_darray_conv = true;
        indent_up();
        out << "ThriftUtil::toDArray(\n" << indent();
      }
      val_type = map->get_val_type();
    } else {
      container_prefix = "Vec\\";
      val_type = static_cast<const t_list*>(ttype)->get_elem_type();
    }

    if (const auto* ttypedef =
            dynamic_cast<const t_placeholder_typedef*>(val_type)) {
      val_type = ttypedef->get_type();
    }
    auto [wrapper, name, ns] = find_hack_wrapper(val_type);
    if (val_type->is<t_container>() || val_type->is<t_struct>() ||
        val_type->is<t_union>()) {
      std::stringstream inner;
      std::string inner_val = namer("$val");
      indent_up();
      indent_up();
      bool is_async_val = generate_php_struct_async_toShape_method_helper(
          inner, val_type, namer, inner_val);
      indent_down();
      auto inner_str = (is_async_val ? "await " : "") + inner.str();
      if (!wrapper && inner_val == inner_str) {
        // Since value doesn't need mapping, 'inner_val' will be unused.
        // Decrement the counter so that we don't skip namer values.
        namer.decrement_counter();
        indent_down();
        out << val;
        if (use_to_darray_conv) {
          indent_down();
          out << "\n" << indent() << ")";
        }
        return false;
      }
      bool use_map_async = wrapper || is_async_val;
      out << container_prefix << (use_map_async ? "map_async" : "map") << "(\n";

      out << indent() << val << ",\n"
          << indent() << (use_map_async ? "async " : "") << inner_val
          << " ==> \n";

      indent_up();

      if (wrapper) {
        out << indent() << "{\n";
        indent_up();
        out << indent() << inner_val << " = await " << inner_val
            << "->genUnwrap();\n";
        out << indent() << "return " << inner_str << ";\n";
        indent_down();
        out << indent() << "}\n";
      } else {
        out << indent() << inner_str << "\n";
      }

      indent_down();
      indent_down();
      out << indent() << ")";
      if (use_to_darray_conv) {
        indent_down();
        out << "\n" << indent() << ")";
      }
      return use_map_async;
    } else {
      out << generate_to_array_method(ttype, val);
      return false;
    }
  } else {
    out << val;
    return false;
  }
}

void t_hack_generator::generate_php_struct_async_shape_methods(
    std::ofstream& out,
    const t_structured* tstruct,
    const std::string& struct_hack_name_with_ns) {
  generate_php_struct_stringifyMapKeys_method(out);
  generate_php_struct_async_struct_creation_method_header(
      out, ThriftAsyncStructCreationMethod::FROM_SHAPE);
  t_name_generator namer;
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const std::string& name = field.name();

    bool nullable = field_is_nullable(tstruct, &field) || nullable_everything_;

    std::string field_ref;
    if (tstruct->is<t_union>() || nullable) {
      field_ref = "$" + name;
      out << indent() << field_ref << " = Shapes::idx($shape, '" << name
          << "');\n";
      out << indent() << "if (" << field_ref << " !== null) {\n";
      indent_up();
    } else {
      field_ref = "$shape['" + name + "']";
    }

    generate_php_struct_async_struct_creation_method_field_assignment(
        out, tstruct, field, field_ref, struct_hack_name_with_ns, namer, true);
    if (tstruct->is<t_union>() || nullable) {
      indent_down();
      out << indent() << "}\n";
    }
  }
  generate_php_struct_async_struct_creation_method_footer(out);
  out << "\n";
  indent(out)
      << "public async function __genToShape(): Awaitable<self::TShape> {\n";
  indent_up();
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    auto fieldRef = "$this->" + field.name();
    if (find_hack_wrapper(field)) {
      out << indent() << "$" << field.name() << " = await "
          << (tstruct->is<t_union>() ? "" : "(") << fieldRef
          << (tstruct->is<t_union>() ? "?" : " as nonnull)")
          << "->genUnwrap();\n";
      fieldRef = "$" + field.name();
    }
    auto [wrapper, name, ns] = find_hack_wrapper(field.get_type());
    if (wrapper) {
      bool nullable =
          field_is_nullable(tstruct, &field) || nullable_everything_;
      out << indent() << "$" << field.name() << " = await " << fieldRef
          << (nullable ? "?" : "") << "->genUnwrap();\n";
    }
  }
  indent(out) << "return shape(\n";
  indent_up();

  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const t_type* t = field.type()->get_true_type();
    t_name_generator ngen;

    indent(out) << "'" << field.name() << "' => ";

    std::stringstream val;

    bool nullable = field_is_nullable(tstruct, &field) || nullable_everything_;

    auto fieldRef = "$this->" + field.name();
    if (find_hack_wrapper(field)) {
      fieldRef = "$" + field.name();
    } else {
      auto [wrapper, name, ns] = find_hack_wrapper(field.get_type());
      if (wrapper) {
        fieldRef = "$" + field.name();
      }
    }
    if (std::optional<std::string> adapter = find_hack_field_adapter(field)) {
      out << fieldRef << ",\n";
    } else if (
        !t->is<t_container>() && !t->is<t_struct>() && !t->is<t_union>()) {
      out << fieldRef << ",\n";
    } else {
      bool is_async = generate_php_struct_async_toShape_method_helper(
          val, t, ngen, fieldRef);
      out << (is_async ? "await " : "");
      if (nullable) {
        out << "(" << fieldRef << " === null \n";
        indent_up();
        indent(out) << "? null \n";
        indent(out) << ": ";
        out << "(\n" << indent() << val.str() << "\n" << indent() << ")\n";
        indent_down();
        indent(out) << "),\n";
      } else {
        out << val.str() << ",\n";
      }
    }
  }
  indent_down();
  indent(out) << ");\n";
  indent_down();
  indent(out) << "}\n";
}

/**
 * Generates the structural ID definition, see generate_structural_id()
 * for information about the structural ID.
 */
void t_hack_generator::generate_php_structural_id(
    std::ofstream& out, const t_structured* tstruct, bool asFunction) {
  if (asFunction) {
    indent(out) << "static function getStructuralID()[]: int {\n";
    indent_up();
    indent(out) << "return " << generate_structural_id(tstruct) << ";\n";
    indent_down();
    indent(out) << "}\n";
  } else {
    indent(out) << "const int STRUCTURAL_ID = "
                << generate_structural_id(tstruct) << ";\n";
  }
}

bool t_hack_generator::is_async_struct(const t_structured* tstruct) {
  for (const auto& field : tstruct->fields()) {
    if (!skip_codegen(&field)) {
      if (is_async_field(field, false)) {
        return true;
      }
    }
  }
  return false;
}

bool t_hack_generator::is_async_shapish_struct(const t_structured* tstruct) {
  std::string parent_struct_name = hack_name(tstruct);
  switch (struct_async_type_[parent_struct_name]) {
    case ThriftShapishStructType::ASYNC:
      return true;
    case ThriftShapishStructType::SYNC:
      return false;
    case ThriftShapishStructType::VISITED:
      // 'shapes' option cannot be used with recursive structs
      // We should ideally throw an error here.
      // But there are certain files that are able to bypass,
      // so simply returning false.
      return false;
    default:
      struct_async_type_[parent_struct_name] = ThriftShapishStructType::VISITED;
  }

  for (const auto& field : tstruct->fields()) {
    if (!skip_codegen(&field)) {
      if (is_async_field(field, true)) {
        struct_async_type_[parent_struct_name] = ThriftShapishStructType::ASYNC;
        return true;
      }
    }
  }
  struct_async_type_[parent_struct_name] = ThriftShapishStructType::SYNC;
  return false;
}

bool t_hack_generator::is_async_field(
    const t_field& field, bool check_nested_structs) {
  auto [wrapper, name, ns] = find_hack_wrapper(field.get_type());
  return find_hack_wrapper(field) /* Check for field wrapper */ ||
      wrapper /* Check for typedef wrapper*/ ||
      is_async_type(
             field.get_type()->get_true_type(),
             check_nested_structs) /* Check for struct wrapper and containers
                                    */
      ;
}

bool t_hack_generator::is_async_type(
    const t_type* type, bool check_nested_structs) {
  auto [wrapper, name, ns] = find_hack_wrapper(type);
  if (wrapper) {
    return true;
  }
  type = type->get_true_type();
  if (type->is<t_primitive_type>() || type->is<t_enum>()) {
    return false;
  } else if (type->is<t_container>()) {
    if (const auto* tlist = dynamic_cast<const t_list*>(type)) {
      return is_async_type(tlist->get_elem_type(), check_nested_structs);
    } else if (const auto* tset = dynamic_cast<const t_set*>(type)) {
      return is_async_type(tset->get_elem_type(), check_nested_structs);
    } else if (const auto* tmap = dynamic_cast<const t_map*>(type)) {
      return is_async_type(tmap->get_val_type(), check_nested_structs);
    }
  } else if (const auto* tstruct = dynamic_cast<const t_structured*>(type)) {
    if (check_nested_structs) {
      return is_async_shapish_struct(tstruct);
    }
  }
  return false;
}

void t_hack_generator::generate_php_struct_definition(
    std::ofstream& out,
    const t_structured* tstruct,
    ThriftStructType type,
    const std::string& name) {
  const std::string& real_name = !name.empty() ? name : find_hack_name(tstruct);
  if (tstruct->is<t_union>()) {
    // Generate enum for union before the actual class
    generate_php_union_enum(out, tstruct, real_name);
  }
  _generate_php_struct_definition(out, tstruct, type, real_name);
}

void t_hack_generator::generate_php_union_methods(
    std::ofstream& out,
    const t_structured* tstruct,
    const std::string& struct_hack_name_with_ns) {
  auto enumName = union_enum_name(struct_hack_name_with_ns);

  indent(out) << "public function getType()[]: " << enumName << " {\n";
  indent(out) << indent() << "return $this->_type;\n";
  indent(out) << "}\n\n";
  out << indent() << "public function reset()[write_props]: void {\n";
  indent_up();
  out << indent() << "switch ($this->_type) {\n";
  indent_up();
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const auto& fieldName = field.name();
    out << indent() << "case " << enumName << "::" << fieldName << ":\n";
    out << indent(get_indent() + 1) << "$this->" << fieldName << " = null;\n";
    out << indent(get_indent() + 1) << "break;\n";
  }
  out << indent() << "case " << enumName << "::_EMPTY_:\n";
  out << indent(get_indent() + 1) << "break;\n";
  indent_down();
  out << indent() << "}\n";
  out << indent() << "$this->_type = " << enumName << "::_EMPTY_;\n";
  indent_down();
  out << indent() << "}\n\n";

  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const auto& fieldName = field.name();
    std::string typehint;
    if (std::optional<std::string> adapter = find_hack_field_adapter(field)) {
      typehint = *adapter + "::THackType";
    } else {
      typehint = type_to_typehint(field.get_type());
    }

    // set_<fieldName>()
    indent(out) << "public function set_" << fieldName << "(" << typehint
                << " $" << fieldName << ")[write_props]: this {\n";
    indent_up();
    indent(out) << "$this->reset();\n";
    t_name_generator namer;
    generate_php_struct_async_struct_creation_method_field_assignment(
        out,
        tstruct,
        field,
        "$" + fieldName,
        struct_hack_name_with_ns,
        namer,
        false,
        true,
        "$this");

    indent(out) << "return $this;\n";
    indent_down();
    indent(out) << "}\n\n";

    typehint = field_to_typehint(
        field,
        struct_hack_name_with_ns,
        /* is_field_nullable */ false);

    // get_<fieldName>()
    indent(out) << "public function get_" << fieldName << "()[]: ?" << typehint
                << " {\n";
    indent_up();
    indent(out) << "$this->logIncorrectFieldAccessed(\n";
    indent_up();
    indent(out) << "$this->_type,\n";
    indent(out) << enumName << "::" << fieldName << ",\n";

    if (union_logger_rollout_) {
      indent(out) << "$this->" << fieldName << " === null,\n";
    }

    indent_down();
    indent(out) << ");\n";
    indent(out) << "return $this->" << fieldName << ";\n";
    indent_down();
    indent(out) << "}\n\n";

    // getx_<fieldName>()
    indent(out) << "public function getx_" << fieldName << "()[]: " << typehint
                << " {\n";
    indent_up();
    indent(out) << "invariant(\n";
    indent_up();
    indent(out) << "$this->_type === " << enumName << "::" << fieldName
                << ",\n";
    indent(out) << "'get_" << fieldName << " called on an instance of "
                << tstruct->name() << " whose current type is %s',\n";
    indent(out) << "(string)$this->_type,\n";
    indent_down();
    indent(out) << ");\n";
    indent(out) << "return $this->" << fieldName << " as nonnull;\n";
    indent_down();
    indent(out) << "}\n\n";
  }
}

void t_hack_generator::generate_php_struct_fields(
    std::ofstream& out,
    const t_structured* tstruct,
    const std::string& struct_hack_name_with_ns,
    ThriftStructType type) {
  for (const t_field& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const auto& fieldName = field.name();
    bool is_base_exception_field = type == ThriftStructType::EXCEPTION &&
        is_base_exception_property(&field);

    std::optional<std::string> field_wrapper = find_hack_wrapper(field);

    if (is_base_exception_field && field_wrapper) {
      throw std::runtime_error(
          tstruct->name() + "::" + fieldName +
          " has a wrapped type. FieldWrapper annotation is not" +
          " allowed for base exception properties.");
    }

    const t_type* t = field.get_type();
    bool is_async_typ = is_async_type(t, false);

    t = t->get_true_type();

    if (t->is<t_enum>() && is_bitmask_enum(static_cast<const t_enum*>(t))) {
      throw std::runtime_error(
          "Enum " + t->name() +
          " is actually a bitmask, cannot generate a field of this enum type");
    }

    // result structs only contain fields: success and e.
    // success is whatever type the method returns, but must be nullable
    // regardless, since if there is an exception we expect it to be null
    bool nullable =
        (type == ThriftStructType::RESULT ||
         field_is_nullable(tstruct, &field) || nullable_everything_) &&
        !is_base_exception_field;

    // Compute typehint before resolving typedefs to avoid missing any adapter
    // annotations.
    std::string typehint = field_to_typehint(
        field,
        struct_hack_name_with_ns,
        nullable && !tstruct->is<t_union>() /* is_field_nullable */);

    if (tstruct->is<t_result_struct>() && fieldName == "success") {
      typehint = "this::TResult";
    }

    if (nullable || field_wrapper) {
      typehint.insert(0, "?");
    }

    if (type != ThriftStructType::RESULT && type != ThriftStructType::ARGS) {
      generate_php_docstring(out, &field);
    }

    const std::string field_attributes = find_attributes(field, true);
    if (!field_attributes.empty()) {
      indent(out) << "<<" << field_attributes << ">>\n";
    }

    if (type == ThriftStructType::EXCEPTION && fieldName == "code") {
      if (!(t->is_any_int() || t->is<t_enum>())) {
        throw std::runtime_error(
            tstruct->name() + "::code defined to be a non-integral type. " +
            "code fields for Exception classes must be integral");
      } else if (const t_enum* enum_ = t->try_as<t_enum>();
                 enum_ != nullptr && enum_->get_enum_values().empty()) {
        throw std::runtime_error(
            "Enum " + t->name() + " is the type for the code property of " +
            tstruct->name() + ", but it has no values.");
      }
      if (t->is<t_enum>()) {
        typehint.insert(0, "/* Originally defined as ");
        typehint += " */ int";
      }
    }

    std::string visibility = field_wrapper
        ? "private"
        : ((protected_unions_ && tstruct->is<t_union>()) ? "protected"
                                                         : "public");

    indent(out) << visibility << " " << typehint << " $" << fieldName << ";\n";
    generate_php_struct_field_methods(
        out, &field, type == ThriftStructType::EXCEPTION);

    if (field_wrapper) {
      generate_php_field_wrapper_methods(
          out,
          field,
          tstruct->is<t_union>(),
          nullable,
          struct_hack_name_with_ns);
    }
    if (!tstruct->is<t_union>() && is_async_typ) {
      out << "\n";

      // set_<fieldName>_DO_NOT_USE_THRIFT_INTERNAL()
      indent(out) << "public function set_" << fieldName
                  << "_DO_NOT_USE_THRIFT_INTERNAL("
                  << type_to_typehint(
                         t,
                         {{TypeToTypehintVariations::RECURSIVE_IGNORE_WRAPPER,
                           true}})
                  << " $" << fieldName << ")[write_props]: void {\n";
      indent_up();

      t_name_generator namer;
      generate_php_struct_async_struct_creation_method_field_assignment(
          out,
          tstruct,
          field,
          "$" + fieldName,
          struct_hack_name_with_ns,
          namer,
          false,
          true,
          "$this");
      indent_down();
      indent(out) << "}\n\n";
    }
  }
}

void t_hack_generator::generate_php_field_wrapper_methods(
    std::ofstream& out,
    const t_field& field,
    bool is_union,
    bool nullable,
    const std::string& struct_hack_name_with_ns) {
  if (is_union) {
    return;
  }
  const auto& fieldName = field.name();
  out << "\n";

  // get_<fieldName>()
  indent(out) << "public function get_" << fieldName << "()[]: "
              << field_to_typehint(
                     field,
                     struct_hack_name_with_ns,
                     /*is_field_nullable*/ nullable)
              << " {\n";
  indent_up();
  indent(out) << "return $this->" << fieldName << " as nonnull;\n";
  indent_down();
  indent(out) << "}\n\n";
}

void t_hack_generator::generate_php_struct_field_methods(
    std::ofstream& out, const t_field* field, bool is_exception) {
  const t_type* t = field->get_type()->get_true_type();
  if (is_exception && field->name() == "code" && t->is<t_enum>()) {
    std::string enum_type = type_to_typehint(field->get_type());
    out << "\n";
    out << indent() << "public function setCodeAsEnum(" << enum_type
        << " $code)[write_props]: void {\n";
    std::string code_expr = "$code";
    if (!enum_transparenttype_) {
      code_expr =
          "HH\\FIXME\\UNSAFE_CAST<arraykey, int>($code, 'nontransparent enum')";
    }
    out << indent() << "  $this->code = " << code_expr << ";\n"
        << indent() << "}\n\n";

    out << indent() << "public function getCodeAsEnum()[]: " << enum_type
        << " {\n"
        << indent() << "  return "
        << "HH\\FIXME\\UNSAFE_CAST<int, " + enum_type + ">" +
            "($this->code, 'retain HHVM enforcement semantics')"
        << ";\n"
        << indent() << "}\n";
  }
}

void t_hack_generator::generate_php_struct_methods(
    std::ofstream& out,
    const t_structured* tstruct,
    ThriftStructType type,
    const std::string& struct_hack_name,
    bool is_async_struct,
    bool is_async_shapish_struct,
    bool add_clear_terse_fields_interface,
    const std::string& struct_hack_name_with_ns) {
  if (is_async_struct) {
    generate_php_struct_default_constructor(
        out, tstruct, type, struct_hack_name_with_ns);
    generate_php_struct_withDefaultValues_method(out);
    generate_php_struct_async_struct_creation_method(
        out,
        tstruct,
        struct_hack_name_with_ns,
        ThriftAsyncStructCreationMethod::FROM_CONSTRUCTOR_SHAPE);
    out << "\n";
    if (from_map_construct_) {
      generate_php_struct_async_struct_creation_method(
          out,
          tstruct,
          struct_hack_name_with_ns,
          ThriftAsyncStructCreationMethod::FROM_MAP);
      out << "\n";
    }
  } else {
    generate_php_struct_constructor(
        out, tstruct, type, struct_hack_name_with_ns);
    generate_php_struct_withDefaultValues_method(out);
    generate_php_struct_from_shape(out, tstruct);
    out << "\n";

    if (from_map_construct_) {
      generate_php_struct_from_map(out, tstruct);
      out << "\n";
    }
  }

  out << indent() << "public function getName()[]: string {\n"
      << indent() << "  return '" << struct_hack_name << "';\n"
      << indent() << "}\n\n";
  if (tstruct->is<t_union>()) {
    generate_php_union_methods(out, tstruct, struct_hack_name_with_ns);
  }
  if (type == ThriftStructType::EXCEPTION) {
    const auto* message_field =
        dynamic_cast<const t_exception&>(*tstruct).get_message_field();
    if (message_field && message_field->name() != "message") {
      if (const auto field_wrapper = find_hack_wrapper(*message_field)) {
        throw std::runtime_error(
            tstruct->name() + "::" + message_field->name() +
            " has a wrapped type. FieldWrapper annotation is not allowed for "
            "base exception properties.");
      }

      out << indent() << "<<__Override>>\n"
          << indent() << "public function getMessage()[]: string {\n";
      out << indent() << "  return $this->" << message_field->name();
      if (message_field->get_req() != t_field::e_req::required) {
        out << " ?? ''";
      }
      out << ";\n" << indent() << "}\n\n";
    }
  }
  if (add_clear_terse_fields_interface) {
    generate_php_struct_clear_terse_fields(
        out, tstruct, type, struct_hack_name_with_ns);
  }
  generate_php_struct_metadata_method(out, tstruct);
  generate_php_struct_structured_annotations_method(out, tstruct);

  if (shapes_ && !tstruct->generated() && type != ThriftStructType::EXCEPTION &&
      type != ThriftStructType::RESULT) {
    if (is_async_shapish_struct) {
      generate_php_struct_async_shape_methods(
          out, tstruct, struct_hack_name_with_ns);
    } else {
      generate_php_struct_shape_methods(out, tstruct);
    }
  }
  generate_instance_key(out);
  generate_json_reader(out, tstruct);
  generate_adapter_type_checks(out, tstruct);

  if (tstruct->is<t_result_struct>()) {
    generate_exception_method(out, tstruct);
  }
}

void t_hack_generator::generate_exception_method(
    std::ofstream& out, const t_structured* tstruct) {
  const auto& fields = tstruct->fields();
  if (fields.size() == 0 ||
      (fields.size() == 1 && fields[0].name() == "success")) {
    return;
  }
  auto it = fields.begin() + (fields[0].name() == "success");

  indent(out) << "public function checkForException(): ?\\TException {\n";
  indent_up();
  for (; it != fields.end(); ++it) {
    indent(out) << "if ($this->" << it[0].name() << " !== null) {\n";
    indent_up();
    indent(out) << "return $this->" << it[0].name() << ";\n";
    indent_down();
    indent(out) << "}\n";
  }
  indent(out) << "return null;\n";
  indent_down();
  indent(out) << "}\n";

  indent(out) << "\n";
  indent(out) << "public function setException(\\Exception $e): bool {\n";
  indent_up();
  it = fields.begin() + (fields[0].name() == "success");
  for (; it != fields.end(); ++it) {
    const auto& field = it[0];
    if (it[0].type()->get_true_type()->is<t_exception>()) {
      indent(out) << "if ($e is "
                  << type_to_typehint(
                         field.get_type(),
                         // Type aliases are not supported in `catch` clauses.
                         // Hack expects the name of a class.
                         {{TypeToTypehintVariations::IGNORE_TYPEDEF_OPTION,
                           true}})
                  << ") {\n";
      indent_up();
      indent(out) << "$this->" << field.name() << " = " << "$e;\n";
      indent(out) << "return true;\n";
      indent_down();
      indent(out) << "}\n";
    }
  }
  indent(out) << "return false;\n";
  indent_down();
  indent(out) << "}\n";
}

void t_hack_generator::generate_php_struct_constructor_field_assignment(
    std::ofstream& out,
    const t_field& field,
    const t_structured* tstruct,
    ThriftStructType type,
    const std::string& struct_hack_name_with_ns,
    bool is_default_assignment,
    bool skip_custom_default,
    bool first_field) {
  if (skip_codegen(&field)) {
    return;
  }
  std::string adapter;
  if (auto annotation = find_hack_field_adapter(field)) {
    adapter = *annotation;
  } else if (auto annotation_2 = find_hack_adapter(field.get_type())) {
    adapter = *annotation_2;
  }

  const t_type* t = field.type()->get_true_type();
  std::string hack_typehint;
  if (!adapter.empty()) {
    hack_typehint = adapter + "::THackType";
  } else {
    hack_typehint = type_to_typehint(
        field.get_type(), {{TypeToTypehintVariations::IGNORE_WRAPPER, true}});
  }
  std::string dval;
  bool is_exception = tstruct->is<t_exception>();
  if (field.default_value() != nullptr &&
      !(t->is<t_structured>() || skip_custom_default)) {
    dval = render_const_value(
        t,
        field.default_value(),
        /*immutable_collections*/ false,
        /*ignore_wrapper*/ false,
        /*force_arrays*/ false);
  } else if (
      tstruct->is<t_exception>() &&
      (field.name() == "code" || field.name() == "line")) {
    if (t->is_any_int()) {
      dval = "0";
    } else {
      // just use the lowest value
      const t_enum* tenum = static_cast<const t_enum*>(t);
      dval =
          hack_name(tenum) + "::" + (*tenum->get_enum_values().begin())->name();
    }
  } else if (
      is_exception && (field.name() == "message" || field.name() == "file")) {
    dval = "''";
  } else if (tstruct->is<t_union>() || nullable_everything_) {
    dval = "null";
  } else {
    dval = render_default_value(t);
  }
  if (dval != "null") {
    if (!adapter.empty()) {
      dval = adapter + "::fromThrift(" + dval + ")";
    }
  }

  // result structs only contain fields: success and e.
  // success is whatever type the method returns, but must be nullable
  // regardless, since if there is an exception we expect it to be null
  // TODO(ckwalsh) Extract this logic into a helper function
  bool nullable = type == ThriftStructType::RESULT ||
      (!(is_exception && is_base_exception_property(&field)) &&
       (dval == "null" ||
        (field.get_req() == t_field::e_req::optional &&
         field.default_value() == nullptr)));

  const std::string& field_name = field.name();
  bool need_enum_code_fixme = is_exception && field_name == "code" &&
      t->is<t_enum>() && !enum_transparenttype_;
  if (is_default_assignment) {
    auto [type_wrapper, underlying_name, ns] =
        find_hack_wrapper(field.get_type());
    if (type_wrapper) {
      if (!nullable) {
        dval = *type_wrapper + "::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<" +
            hack_typehint + ">(" + dval + ")";
      }
      hack_typehint = type_to_typehint(field.get_type());
    }
    if (std::optional<std::string> field_wrapper = find_hack_wrapper(field)) {
      static const std::string_view null = "null";
      out << indent() << "$this->" << field_name << " = " << *field_wrapper
          << "::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<" << (nullable ? "?" : "")
          << hack_typehint << ", " << struct_hack_name_with_ns << ">("
          << (nullable ? null : dval) << ", " << field.get_key()
          << ", $this);\n";
    } else if (!nullable || skip_custom_default) {
      out << indent() << "$this->" << field_name << " = " << dval << ";\n";
    }
  } else {
    if (tstruct->is<t_union>()) {
      // Capture value from constructor and update _type field
      if (strict_unions_ && !first_field) {
        out << " else ";
      } else {
        out << indent();
      }
      out << "if ($" << field_name << " !== null) {\n";
    }

    std::string field_expr =
        "$" + field_name + (!nullable ? " ?? " + dval : "");
    if (need_enum_code_fixme) {
      field_expr = "HH\\FIXME\\UNSAFE_CAST<arraykey, int>(" + field_expr +
          ", 'nontransparent Enum')";
    }
    out << indent() << (tstruct->is<t_union>() ? "  " : "")
        << "$this->" + field_name + " = " << field_expr << ";\n";

    if (tstruct->is<t_union>()) {
      out << indent() << "  $this->_type = "
          << union_field_to_enum(&field, struct_hack_name_with_ns) << ";\n"
          << indent() << "}";
      if (!strict_unions_) {
        out << "\n";
      }
    }
  }
}

void t_hack_generator::generate_php_struct_constructor(
    std::ofstream& out,
    const t_structured* tstruct,
    ThriftStructType type,
    const std::string& struct_hack_name_with_ns) {
  out << indent() << "public function __construct(";
  auto delim = "";

  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    std::string typehint;
    if (std::optional<std::string> adapter = find_hack_field_adapter(field)) {
      typehint = *adapter + "::THackType";
    } else {
      typehint = type_to_typehint(field.get_type());
    }
    if (tstruct->is<t_result_struct>() && field.name() == "success") {
      typehint = "this::TResult";
    }
    out << delim << "?" << typehint << " $" << field.name() << " = null";
    delim = ", ";
  }
  out << ")[] {\n";
  indent_up();

  if (type == ThriftStructType::EXCEPTION) {
    out << indent() << "parent::__construct();\n";
  }
  if (tstruct->is<t_union>()) {
    out << indent() << "$this->_type = "
        << union_field_to_enum(nullptr, struct_hack_name_with_ns) << ";\n";
  }

  if (strict_unions_ && tstruct->is<t_union>()) {
    bool is_first_field = true;
    for (auto iter = tstruct->fields().rbegin();
         iter != tstruct->fields().rend();
         iter++) {
      if (!skip_codegen(&*iter)) {
        generate_php_struct_constructor_field_assignment(
            out,
            *iter,
            tstruct,
            type,
            struct_hack_name_with_ns,
            false,
            false,
            is_first_field);
        is_first_field = false;
      }
    }
    out << "\n";
  } else {
    for (const auto& field : tstruct->fields()) {
      if (!skip_codegen(&field)) {
        generate_php_struct_constructor_field_assignment(
            out, field, tstruct, type, struct_hack_name_with_ns);
      }
    }
  }

  scope_down(out);
  out << "\n";
}

void t_hack_generator::generate_php_struct_withDefaultValues_method(
    std::ofstream& out) {
  indent(out) << "public static function withDefaultValues()[]: this {\n";
  indent_up();
  indent(out) << "return new static();\n";
  scope_down(out);
  out << "\n";
}

void t_hack_generator::generate_php_struct_default_constructor(
    std::ofstream& out,
    const t_structured* tstruct,
    ThriftStructType type,
    const std::string& struct_hack_name_with_ns) {
  out << indent() << "public function __construct()[] {\n";
  indent_up();
  if (!tstruct->is<t_union>()) {
    if (type == ThriftStructType::EXCEPTION) {
      out << indent() << "parent::__construct();\n";
    }

    std::vector<const t_field*> wrapped_fields_or_types;
    if (type != ThriftStructType::RESULT) {
      for (const auto& field : tstruct->fields()) {
        if (skip_codegen(&field)) {
          continue;
        }
        // Fields with FieldWrappr annotation are nullable and need to be set
        // at the end after all non-nullable fields are set.
        if (find_hack_wrapper(field)) {
          wrapped_fields_or_types.push_back(&field);
        } else {
          generate_php_struct_constructor_field_assignment(
              out, field, tstruct, type, struct_hack_name_with_ns, true);
        }
      }
      for (const auto& field : wrapped_fields_or_types) {
        generate_php_struct_constructor_field_assignment(
            out, *field, tstruct, type, struct_hack_name_with_ns, true);
      }
    }
  }
  scope_down(out);
  out << "\n";
}

bool t_hack_generator::has_clear_terse_fields(const t_structured* tstruct) {
  for (const auto& field : tstruct->fields()) {
    if (!skip_codegen(&field) &&
        field.qualifier() == t_field_qualifier::terse) {
      return true;
    }
  }
  return false;
}

void t_hack_generator::generate_php_struct_clear_terse_fields(
    std::ofstream& out,
    const t_structured* tstruct,
    ThriftStructType type,
    const std::string& struct_hack_name_with_ns) {
  assert(has_clear_terse_fields(tstruct));

  out << indent()
      << "public function clearTerseFields()[write_props]: void {\n";
  indent_up();
  for (const auto& field : tstruct->fields()) {
    if (!skip_codegen(&field) &&
        field.qualifier() == t_field_qualifier::terse) {
      generate_php_struct_constructor_field_assignment(
          out, field, tstruct, type, struct_hack_name_with_ns, true, true);
    }
  }
  scope_down(out);
  out << "\n";
}

void t_hack_generator::generate_php_struct_metadata_method(
    std::ofstream& out, const t_structured* tstruct) {
  bool is_exception = tstruct->is<t_exception>();
  // Metadata
  out << indent() << "public static function get"
      << (is_exception ? "Exception" : "Struct") << "Metadata()[]: "
      << (is_exception ? "\\tmeta_ThriftException" : "\\tmeta_ThriftStruct")
      << " {\n";
  indent_up();

  out << indent() << "return "
      << render_const_value(
             is_exception ? tmeta_ThriftException_type()
                          : tmeta_ThriftStruct_type(),
             struct_to_tmeta(tstruct, is_exception).get(),
             /*immutable_collections*/ false,
             /*ignore_wrapper*/ false,
             /*force_arrays*/ true)
      << ";\n";

  indent_down();
  out << indent() << "}\n\n";
}

void t_hack_generator::generate_php_struct_structured_annotations_method(
    std::ofstream& out, const t_structured* tstruct) {
  indent(out)
      << "public static function getAllStructuredAnnotations()[write_props]: "
         "\\TStructAnnotations {\n";
  indent_up();

  std::stringstream annotations_out;
  std::stringstream annotations_temp_var_initializations_out;
  t_name_generator namer;

  indent(annotations_out) << "return shape(\n";
  indent_up();
  indent(annotations_out) << "'struct' => "
                          << render_structured_annotations(
                                 tstruct->structured_annotations(),
                                 annotations_temp_var_initializations_out,
                                 namer)
                          << ",\n";
  indent(annotations_out) << "'fields' => dict[\n";
  indent_up();
  for (auto&& field : tstruct->fields()) {
    if ((field.structured_annotations().empty() &&
         field.type()->structured_annotations().empty()) ||
        skip_codegen(&field)) {
      continue;
    }
    indent(annotations_out) << "'" << field.name() << "' => shape(\n";
    indent_up();
    indent(annotations_out) << "'field' => "
                            << render_structured_annotations(
                                   field.structured_annotations(),
                                   annotations_temp_var_initializations_out,
                                   namer)
                            << ",\n";
    indent(annotations_out) << "'type' => "
                            << render_structured_annotations(
                                   field.type()->structured_annotations(),
                                   annotations_temp_var_initializations_out,
                                   namer)
                            << ",\n";
    indent_down();
    indent(annotations_out) << "),\n";
  }

  out << annotations_temp_var_initializations_out.str();
  out << annotations_out.str();
  indent_down();
  indent(out) << "],\n";
  indent_down();
  indent(out) << ");\n";
  indent_down();
  indent(out) << "}\n\n";
}

void t_hack_generator::generate_php_union_enum(
    std::ofstream& out, const t_structured* tstruct, const std::string& name) {
  // Generate enum class with this pattern
  // enum <UnionName>Enum: int {
  //   __EMPTY__ = 0;
  //   field1 = 1;
  // }
  const std::string union_enum_attributes =
      find_union_enum_attributes(*tstruct);
  if (!union_enum_attributes.empty()) {
    indent(out) << "<<" << union_enum_attributes << ">>\n";
  }
  out << "enum " << union_enum_name(name, tstruct->program(), true)
      << ": int {\n";

  indent_up();
  // If no field is set
  indent(out) << UNION_EMPTY << " = 0;\n";
  for (const auto& field : tstruct->fields()) {
    if (!skip_codegen(&field)) {
      indent(out) << field.name() << " = " << field.id() << ";\n";
    }
  }
  indent_down();
  out << "}\n\n";
}

bool t_hack_generator::is_base_exception_property(const t_field* field) {
  static const std::unordered_set<std::string> kBaseExceptionProperties{
      "code", "message", "line", "file"};
  return kBaseExceptionProperties.find(field->name()) !=
      kBaseExceptionProperties.end();
}

std::string t_hack_generator::render_service_metadata_response(
    const t_service* service, const bool mangle) {
  std::vector<const t_enum*> enums;
  std::vector<const t_structured*> structs;
  std::vector<const t_exception*> exceptions;
  std::vector<const t_service*> services;

  std::queue<const t_node*> queue;
  std::set<const t_node*> visited;

  queue.push(service);

  while (!queue.empty()) {
    auto next = queue.front();
    queue.pop();

    if (next == nullptr || visited.find(next) != visited.end()) {
      continue;
    }
    visited.emplace(next);

    if (const t_type* type = dynamic_cast<const t_type*>(next)) {
      next = type->get_true_type();
    }

    if (const auto* tlist = dynamic_cast<const t_list*>(next)) {
      queue.push(tlist->get_elem_type());
    } else if (const auto* tset = dynamic_cast<const t_set*>(next)) {
      queue.push(tset->get_elem_type());
    } else if (const auto* tmap = dynamic_cast<const t_map*>(next)) {
      queue.push(&tmap->key_type().deref());
      queue.push(tmap->get_val_type());
    } else if (dynamic_cast<const t_interaction*>(next)) {
      continue;
    } else if (const auto* tservice = dynamic_cast<const t_service*>(next)) {
      if (tservice != service) {
        services.push_back(tservice);
      }
      queue.push(tservice->extends());

      for (const auto& function : tservice->functions()) {
        if (skip_codegen(&function)) {
          continue;
        }
        if (function.sink_or_stream()) {
          queue.push(&function);
        } else {
          queue.push(function.return_type().get_type());
        }
        queue.push(&function.params());
        queue.push(function.exceptions());
      }
    } else if (const auto* tenum = dynamic_cast<const t_enum*>(next)) {
      enums.push_back(tenum);
    } else if (const auto* tstruct = dynamic_cast<const t_structured*>(next)) {
      for (const auto& field : tstruct->fields()) {
        queue.push(field.get_type());
      }

      if (!tstruct->program() || tstruct->is<t_paramlist>()) {
        continue;
      }

      if (const t_exception* exception = tstruct->try_as<t_exception>()) {
        exceptions.push_back(exception);
      } else {
        structs.push_back(tstruct);
      }
    } else if (const auto* ttypedef = dynamic_cast<const t_typedef*>(next)) {
      queue.push(ttypedef->get_type());
    } else if (const auto* fun = dynamic_cast<const t_function*>(next)) {
      if (const t_sink* sink = fun->sink()) {
        queue.push(sink->get_elem_type());
        queue.push(fun->return_type().get_type());
        queue.push(sink->get_final_response_type());
      } else {
        queue.push(fun->stream()->elem_type().get_type());
        queue.push(fun->return_type().get_type());
      }
    } else {
      // Unsupported node kind.
    }
  }

  std::ostringstream out;
  out << indent()
      << "return \\tmeta_ThriftServiceMetadataResponse::fromShape(\n";
  indent_up();
  out << indent() << "shape(\n";
  indent_up();

  out << indent() << "'context' => \\tmeta_ThriftServiceContext::fromShape(\n";
  indent_up();
  out << indent() << "shape(\n";
  indent_up();

  out << indent() << "'service_info' => self::getServiceMetadata(),\n"
      << indent() << "'module' => \\tmeta_ThriftModuleContext::fromShape(\n";

  indent_up();
  out << indent() << "shape(\n";
  indent_up();
  out << indent() << "'name' => '" << service->program()->name() << "',\n";
  indent_down();
  out << indent() << ")\n";
  indent_down();

  out << indent() << "),\n";
  indent_down();
  out << indent() << ")\n";
  indent_down();
  out << indent() << "),\n";

  out << indent() << "'metadata' => \\tmeta_ThriftMetadata::fromShape(\n";
  indent_up();
  out << indent() << "shape(\n";
  indent_up();

  // Enums

  out << indent() << "'enums' => dict[\n";
  indent_up();

  for (const auto* tenum : enums) {
    out << indent() << "'" << tenum->get_scoped_name() << "' => "
        << hack_name(tenum) << "_TEnumStaticMetadata::getEnumMetadata(),\n";
  }

  indent_down();
  out << indent() << "],\n";

  // Structs

  out << indent() << "'structs' => dict[\n";
  indent_up();

  for (const auto* tstruct : structs) {
    out << indent() << "'" << tstruct->get_scoped_name() << "' => ";
    auto [wrapper, name, ns] = find_hack_wrapper(tstruct);
    if (wrapper) {
      out << hack_wrapped_type_name(name, ns);
    } else {
      out << hack_name(tstruct);
    }
    out << "::getStructMetadata(),\n";
  }

  indent_down();
  out << indent() << "],\n";

  // Exceptions

  out << indent() << "'exceptions' => dict[\n";
  indent_up();

  for (const auto* exception : exceptions) {
    out << indent() << "'" << exception->get_scoped_name() << "' => "
        << hack_name(exception) << "::getExceptionMetadata(),\n";
  }

  indent_down();
  out << indent() << "],\n";

  // Services

  out << indent() << "'services' => dict[\n";
  indent_up();

  for (const auto* tservice : services) {
    out << indent() << "'" << tservice->get_scoped_name() << "' => "
        << php_servicename_mangle(mangle, tservice, true)
        << "StaticMetadata::getServiceMetadata(),\n";
  }

  indent_down();
  out << indent() << "],\n";

  indent_down();
  out << indent() << ")\n";
  indent_down();
  out << indent() << "),\n";
  indent_down();
  out << indent() << ")\n";
  indent_down();
  out << indent() << ");\n";

  return out.str();
}

std::string t_hack_generator::render_structured_annotations(
    node_list_view<const t_const> annotations,
    std::ostream& temp_var_initializations_out,
    t_name_generator& namer) {
  std::ostringstream out;
  out << "dict[";
  if (!annotations.empty()) {
    out << "\n";
    indent_up();
    for (const auto& annotation : annotations) {
      indent(out) << "'" << hack_name(annotation.type()) << "' => "
                  << render_const_value_helper(
                         annotation.type(),
                         annotation.value(),
                         temp_var_initializations_out,
                         namer,
                         /*immutable_collections*/ false,
                         /*ignore_wrapper*/ true,
                         /*structured_annotations*/ true,
                         /*force_arrays*/ false)
                  << ",\n";
    }
    indent_down();
    indent(out);
  }
  out << "]";
  return out.str();
}

/**
 * Combines the user-provided Hack attributes with those intrinsic to the
 * Thrift compiler.
 */
void t_hack_generator::generate_hack_attributes(
    std::ofstream& out, const t_named* type, bool include_user_defined) {
  std::vector<std::string> attributes_parts;

  const std::string& uri = type->uri();
  if (!uri.empty()) {
    attributes_parts.emplace_back(
        "\\ThriftTypeInfo(shape('uri' => '" + uri + "'))");
  }

  if (include_user_defined) {
    const std::string user_attributes = find_attributes(*type, true);
    if (!user_attributes.empty()) {
      attributes_parts.emplace_back(user_attributes);
    }
  }

  if (attributes_parts.empty()) {
    return;
  }

  out << "<<";

  auto delim = "";
  for (const auto& part : attributes_parts) {
    out << delim << part;
    delim = ",";
  }

  out << ">>\n";
}

void t_hack_generator::generate_adapter_type_checks(
    std::ofstream& out, const t_structured* tstruct) {
  // Adapter name -> original type of the field that the adapter is for.
  std::set<std::pair<std::string, std::string>> adapter_types_;
  for (const auto* t : collect_types(tstruct)) {
    if (std::optional<std::string> adapter = find_hack_adapter(t)) {
      adapter_types_.emplace(*adapter, type_to_typehint(t->get_true_type()));
    }
  }

  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    if (std::optional<std::string> adapter = find_hack_field_adapter(field)) {
      adapter_types_.emplace(*adapter, type_to_typehint(field.get_type()));
    }
  }

  if (adapter_types_.empty()) {
    return;
  }

  indent(out)
      << "private static function __hackAdapterTypeChecks()[]: void {\n";
  indent_up();
  for (const auto& kv : adapter_types_) {
    indent(out) << "\\ThriftUtil::requireSameType<" << kv.first
                << "::TThriftType, " << kv.second << ">();\n";
  }
  indent_down();
  indent(out) << "}\n\n";
}

/**
 * Generates a struct definition for a thrift data type. This is nothing in
 * PHP where the objects are all just associative arrays (unless of course we
 * decide to start using objects for them...)
 *
 * @param tstruct The struct definition
 */
void t_hack_generator::_generate_php_struct_definition(
    std::ofstream& out,
    const t_structured* tstruct,
    ThriftStructType type,
    const std::string& name) {
  bool generateAsTrait = has_hack_struct_as_trait(tstruct);

  if (type != ThriftStructType::ARGS && type != ThriftStructType::RESULT &&
      (type == ThriftStructType::EXCEPTION || !generateAsTrait)) {
    generate_php_docstring(out, tstruct, type == ThriftStructType::EXCEPTION);
  }

  generate_hack_attributes(out, tstruct, /*include_user_defined*/ true);
  auto [wrapper, underlying_name, ns] = find_hack_wrapper(tstruct, false);
  std::string struct_hack_name_with_ns;
  std::string struct_hack_decl;
  if (wrapper && underlying_name.has_value()) {
    struct_hack_name_with_ns = hack_wrapped_type_name(underlying_name, ns);
    struct_hack_decl = *underlying_name;
  } else {
    struct_hack_name_with_ns = hack_name(name, tstruct->program());
    struct_hack_decl = hack_name(name, tstruct->program(), true);
  }

  out << (generateAsTrait ? "trait " : "class ") << struct_hack_decl;

  if (generateAsTrait) {
    out << "Trait";
  } else if (tstruct->is<t_exception>()) {
    out << " extends \\TException";
  }
  bool is_async = is_async_struct(tstruct);
  const t_result_struct* result_struct =
      dynamic_cast<const t_result_struct*>(tstruct);
  bool is_result = result_struct != nullptr;
  if (is_result) {
    out << " extends \\Thrift" << (is_async ? "Async" : "Sync");
    if (result_struct->getResultReturnType() == "void") {
      out << "StructWithoutResult";
    } else {
      out << "StructWithResult";
    }
  } else if (is_async) {
    out << " implements \\IThriftAsyncStruct";
  } else {
    out << " implements \\IThriftSyncStruct";
  }
  if (tstruct->is<t_exception>()) {
    out << ", \\IThriftExceptionMetadata";
  } else {
    out << (is_result ? " implements" : ",");
    out << " \\IThriftStructMetadata";
  }

  // Wrapper is not supported on unions.
  if (tstruct->is<t_union>() && !wrapper) {
    std::string_view interface;
    if (strict_unions_) {
      interface = ", \\IThriftStrictUnion<";
    } else if (protected_unions_) {
      interface = ", \\IThriftProtectedUnion<";
    } else {
      interface = ", \\IThriftUnion<";
    }
    out << interface << union_enum_name(struct_hack_name_with_ns) << ">";
  }

  bool gen_shapes = shapes_ && !tstruct->generated() &&
      type != ThriftStructType::EXCEPTION && type != ThriftStructType::RESULT;

  bool is_async_shapish = false;
  if (gen_shapes) {
    is_async_shapish = is_async_shapish_struct(tstruct);
    if (is_async_shapish) {
      out << ", \\IThriftShapishAsyncStruct";
    } else {
      out << ", \\IThriftShapishSyncStruct";
    }
  }

  bool add_clear_terse_fields_interface = has_clear_terse_fields(tstruct);
  if (add_clear_terse_fields_interface) {
    out << ", \\IThriftStructWithClearTerseFields";
  }

  out << " {\n";
  indent_up();

  if (tstruct->is<t_union>()) {
    indent(out) << "use \\ThriftUnionSerializationTrait;\n\n";
    if (tstruct->has_structured_annotation(
            kHackMigrationBlockingLegacyJSONSerialization) ||
        legacy_union_json_serialization_) {
      indent(out) << "use \\ThriftLegacyJSONSerializationTrait;\n\n";
    }
  } else {
    indent(out) << "use \\ThriftSerializationTrait;\n\n";
  }

  if (result_struct != nullptr &&
      result_struct->getResultReturnType() != "void") {
    indent(out) << "const type TResult = ";
    out << result_struct->getResultReturnType() << ";\n\n";
  }

  if (generateAsTrait && type == ThriftStructType::EXCEPTION) {
    indent(out) << "require extends \\TException;\n";
  }

  generate_php_struct_struct_trait(out, tstruct, name);
  generate_php_struct_spec(out, tstruct);
  out << "\n";
  generate_php_struct_shape_spec(out, tstruct, true);
  out << "\n";
  if (gen_shapes) {
    generate_php_struct_shape_spec(out, tstruct);
  }

  generate_php_structural_id(out, tstruct, generateAsTrait);
  generate_php_struct_fields(out, tstruct, struct_hack_name_with_ns, type);

  if (tstruct->is<t_union>()) {
    // Generate _type to store which field is set and initialize it to _EMPTY_
    indent(out) << "protected " << union_enum_name(struct_hack_name_with_ns)
                << " $_type = "
                << union_field_to_enum(nullptr, struct_hack_name_with_ns)
                << ";\n";
  }

  out << "\n";

  generate_php_struct_methods(
      out,
      tstruct,
      type,
      name,
      is_async,
      is_async_shapish,
      add_clear_terse_fields_interface,
      struct_hack_name_with_ns);

  indent_down();
  out << indent() << "}\n\n";
}

void t_hack_generator::generate_php_struct_from_shape(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "public static function fromShape"
      << "(self::TConstructorShape $shape)[]: this {\n";
  indent_up();
  out << indent() << "return new static(\n";
  indent_up();
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const std::string& name = field.name();
    out << indent() << "Shapes::idx($shape, '" << name << "'),\n";
  }
  indent_down();
  out << indent() << ");\n";
  indent_down();
  out << indent() << "}\n";
}

void t_hack_generator::generate_php_struct_from_map(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "public static function fromMap_DEPRECATED(";
  if (strict_types_) {
    // Generate constructor from Map
    out << (const_collections_ ? "\\Const" : "") << "Map<string, mixed> $map";
  } else {
    // Generate constructor from KeyedContainer
    out << (soft_attribute_ ? "<<__Soft>> " : "@")
        << "KeyedContainer<string, mixed> $map";
  }
  out << ")[]: this {\n";
  indent_up();
  out << indent() << "return new static(\n";
  indent_up();
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }

    std::string typehint = type_to_typehint(field.get_type());
    out << indent() << "HH\\FIXME\\UNSAFE_CAST<mixed, " << typehint
        << ">(idx($map, '" << field.name() << "'), 'map value is mixed'),\n";
  }
  indent_down();
  out << indent() << ");\n";
  indent_down();
  out << indent() << "}\n";
}

void t_hack_generator::_generate_args(
    std::ofstream& out,
    const t_service* tservice,
    const t_function* tfunction) {
  const std::string& argsname = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::ARGS);

  const t_paramlist& params_struct = tfunction->params();
  const auto& fields = params_struct.fields();
  bool is_async = is_async_struct(&params_struct);
  out << indent() << "$args = " << argsname;
  if (is_async) {
    out << "::withDefaultValues();\n";
    if (!fields.empty()) {
      // Loop through the fields and assign to the args struct
      for (auto&& field : fields) {
        const std::string& name = field.name();
        const t_type* ftype = field.type()->get_true_type();
        bool nullable = nullable_everything_ || !no_nullables_ ||
            (ftype->is<t_enum>() &&
             (field.default_value() == nullptr ||
              field.get_req() != t_field::e_req::required));
        if (nullable) {
          out << indent() << "if ($" << name << " !== null) {\n";
          indent_up();
          out << indent() << "$args->" << name << " = $" << name << ";\n";
          indent_down();
          out << indent() << "}\n";
        } else {
          out << indent() << "$args->" << name << " = $" << name << ";\n";
        }
      }
    }
  } else if (!fields.empty()) {
    out << "::fromShape(shape(\n";
    indent_up();
    // Loop through the fields and assign to the args struct
    for (auto&& field : fields) {
      indent(out);
      std::string name = "$" + field.name();
      out << "'" << field.name() << "' => ";
      if (nullable_everything_) {
        // just passthrough null
        out << name << " === null ? null : ";
      }
      t_name_generator namer;
      this->_generate_sendImpl_arg(out, namer, name, field.get_type());
      out << ",\n";
    }
    indent_down();
    indent(out) << "));\n";
  } else {
    out << "::withDefaultValues();\n";
  }
}

void t_hack_generator::generate_php_struct_async_struct_creation_method_header(
    std::ofstream& out, ThriftAsyncStructCreationMethod method_type) {
  indent(out) << "public static async function ";
  switch (method_type) {
    case ThriftAsyncStructCreationMethod::FROM_MAP:
      out << "genFromMap_DEPRECATED(";
      if (strict_types_) {
        // Generate constructor from Map
        out << (const_collections_ ? "\\Const" : "")
            << "Map<string, mixed> $map)";
      } else {
        // Generate constructor from KeyedContainer
        out << (soft_attribute_ ? "<<__Soft>> " : "@")
            << "KeyedContainer<string, mixed> $map)";
      }
      break;
    case ThriftAsyncStructCreationMethod::FROM_CONSTRUCTOR_SHAPE:
      out << "genFromShape(self::TConstructorShape $shape)[zoned_shallow]";
      break;
    case ThriftAsyncStructCreationMethod::FROM_SHAPE:
      out << "__genFromShape(self::TShape $shape)";
      break;
  }

  out << ": Awaitable<this> {\n";
  indent_up();
  out << indent() << "$obj = new static();\n";
}

void t_hack_generator::generate_php_struct_async_struct_creation_method_footer(
    std::ofstream& out) {
  out << indent() << "return $obj;\n";

  indent_down();
  out << indent() << "}\n";
}

void t_hack_generator::
    generate_php_struct_async_struct_creation_method_field_assignment(
        std::ofstream& out,
        const t_structured* tstruct,
        const t_field& field,
        const std::string& field_ref,
        const std::string& struct_hack_name_with_ns,
        t_name_generator& namer,
        bool is_shape,
        bool uses_thrift_only_methods,
        const std::string& obj_ref) {
  if (tstruct->is<t_union>()) {
    out << indent() << obj_ref
        << "->_type = " << union_field_to_enum(&field, struct_hack_name_with_ns)
        << ";\n";
  }
  const auto& name = field.name();

  bool is_async = false;
  std::string source_str;
  if (std::optional<std::string> adapter = find_hack_field_adapter(field)) {
    is_async = false;
    source_str = field_ref;
  } else {
    std::stringstream source;
    is_async =
        generate_php_struct_async_struct_creation_method_field_assignment_helper(
            source,
            field.get_type(),
            namer,
            field_ref,
            is_shape,
            uses_thrift_only_methods);
    source_str = source.str();
    auto [type_wrapper, struct_name, ns] = find_hack_wrapper(field.get_type());
    if (type_wrapper) {
      if (is_async) {
        out << indent() << "$" << name << " = " << source_str << ";\n";
        source_str = "$" + name;
      }
      std::string val;
      if (uses_thrift_only_methods) {
        is_async = false;
        val = *type_wrapper + "::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<";
      } else {
        is_async = true;
        val = "await " + *type_wrapper + "::genFromThrift<";
      }
      val = val +
          type_to_typehint(
                field.get_type(),
                {{TypeToTypehintVariations::IGNORE_WRAPPER, true}}) +
          ">(" + source_str + ")";
      source_str = val;
    }
  }

  std::optional<std::string> field_wrapper = find_hack_wrapper(field);
  if (field_wrapper) {
    // await statements need to be in separate line,
    // so we need to assign the value to a temp variable
    // and then pass it to the wrapper for assignment
    if (is_async || source_str != field_ref) {
      out << indent() << "$" << name << " = " << source_str << ";\n";
      source_str = "$" + name;
    }
    if (tstruct->is<t_union>()) {
      out << indent() << obj_ref << "->" << name << " = ";
      if (uses_thrift_only_methods) {
        out << *field_wrapper << "::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<";
      } else {
        out << "await " << *field_wrapper << "::genFromThrift<";
      }
      out << type_to_typehint(field.get_type()) << ", "
          << struct_hack_name_with_ns << ">(" << source_str << ", "
          << field.get_key() << ", " << obj_ref << ");\n";
    } else {
      if (uses_thrift_only_methods) {
        out << indent() << obj_ref << "->get_" << name
            << "()->setValue_DO_NOT_USE_THRIFT_INTERNAL(";
      } else {
        out << indent() << "await " << obj_ref << "->get_" << name
            << "()->genWrap(";
      }
      out << source_str << ");\n";
    }
  } else {
    out << indent() << obj_ref << "->" << name << " = " << source_str << ";\n";
  }
}

void t_hack_generator::generate_php_struct_async_struct_creation_method(
    std::ofstream& out,
    const t_structured* tstruct,
    const std::string& struct_hack_name_with_ns,
    ThriftAsyncStructCreationMethod method_type) {
  generate_php_struct_async_struct_creation_method_header(out, method_type);
  std::string idx_prefix = "Shapes::idx($shape, '";
  if (method_type == ThriftAsyncStructCreationMethod::FROM_MAP) {
    idx_prefix = "idx($map, '";
  }
  for (const auto& field : tstruct->fields()) {
    if (skip_codegen(&field)) {
      continue;
    }
    const std::string& name = field.name();
    std::string field_ref = "$" + name;
    out << indent() << field_ref << " = " << idx_prefix << name << "');\n";
    out << indent() << "if (" << field_ref << " !== null) {\n";
    indent_up();

    if (method_type == ThriftAsyncStructCreationMethod::FROM_MAP) {
      std::string typehint = type_to_typehint(
          field.get_type(),
          {{TypeToTypehintVariations::RECURSIVE_IGNORE_WRAPPER, true}});
      field_ref.insert(0, "HH\\FIXME\\UNSAFE_CAST<mixed, " + typehint + ">(");
      field_ref.append(", 'Map value is mixed')");
    }

    t_name_generator namer;
    generate_php_struct_async_struct_creation_method_field_assignment(
        out, tstruct, field, field_ref, struct_hack_name_with_ns, namer);

    indent_down();
    out << indent() << "}\n";
  }
  generate_php_struct_async_struct_creation_method_footer(out);
}

void t_hack_generator::_generate_sendImplHelper(
    std::ofstream& out,
    const t_function* tfunction,
    const t_service* tservice) {
  std::string long_name = php_servicename_mangle(mangled_services_, tservice);
  const std::string& tservice_name =
      (tservice->is<t_interaction>()
           ? "\"" + service_name_ + "\""
           : long_name + "StaticMetadata::THRIFT_SVC_NAME");
  out << "$this->sendImplHelper($args, " << "\"" << find_hack_name(tfunction)
      << "\", "
      << (tfunction->qualifier() == t_function_qualifier::oneway ? "true"
                                                                 : "false")
      << ", " << tservice_name << " );\n";
}

/**
 * Generates a thrift service.
 *
 * @param tservice The service definition
 */
void t_hack_generator::generate_service(const t_service* tservice) {
  if (mangled_services_) {
    auto [_, ns_type] = get_namespace(tservice);
    if (ns_type != HackThriftNamespaceType::PHP) {
      throw std::runtime_error(
          "cannot generate mangled services for " + tservice->name() +
          "; no php namespace found");
    }
    // Note: Because calling generate_service again "uses up" tmp variables,
    //   generating a mangled service has the effect of changing the files of
    //   unmangled services declared in the same thrift file (i.e., with
    //   different tmp variables). Thus we store/restore the tmp_ counter so
    //   that unmangled service files are not affected.
    int orig_tmp = get_tmp_counter();
    // generate new files for mangled services, if requested
    generate_service(tservice, true);
    set_tmp_counter(orig_tmp);
  } else {
    generate_service(tservice, false);
  }
}

void t_hack_generator::generate_service(
    const t_service* tservice, bool mangle) {
  std::string f_base_name = php_servicename_mangle(mangle, tservice);
  init_codegen_file(f_service_, get_out_dir() + f_base_name + ".php");

  // Generate the main parts of the service
  generate_service_interface(
      tservice,
      mangle,
      /*async*/ true,
      /*client*/ false);
  generate_service_interface(
      tservice,
      mangle,
      /*async*/ true,
      /*client*/ true);
  generate_service_interface(
      tservice,
      mangle,
      /*async*/ false,
      /*client*/ true);
  generate_service_client(tservice, mangle);
  generate_service_interactions(tservice, mangle);
  if (phps_) {
    generate_service_processor(tservice, mangle);
  }
  // Generate the structures passed around and helper functions
  generate_service_helpers(tservice, mangle);

  // Close service file
  f_service_.close();
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_hack_generator::generate_service_processor(
    const t_service* tservice, bool mangle) {
  // Generate the dispatch methods
  std::string extends;
  std::string extends_processor = std::string("\\ThriftAsyncProcessor");
  if (tservice->extends() != nullptr) {
    extends = php_servicename_mangle(mangle, tservice->extends(), true);
    extends_processor = extends + "AsyncProcessorBase";
  }

  std::string long_name = php_servicename_mangle(mangle, tservice);

  // I hate to make this abstract, but Hack doesn't support overriding const
  // types. Thus, we will have an inheritance change that does not define the
  // const type, then branch off at each service with the processor that does
  // define the const type.

  f_service_ << indent() << "abstract class " << long_name
             << "AsyncProcessorBase extends " << extends_processor << " {\n"
             << indent() << "  use \\GetThriftServiceMetadata;\n"
             << indent() << "  abstract const type TThriftIf as " << long_name
             << "AsyncIf;\n"
             << indent()
             << "  const class<\\IThriftServiceStaticMetadata> "
                "SERVICE_METADATA_CLASS = "
             << long_name << "StaticMetadata::class;\n"
             << indent() << "  const string THRIFT_SVC_NAME = " << long_name
             << "StaticMetadata::THRIFT_SVC_NAME;\n\n";

  indent_up();

  // Generate the process subfunctions
  for (const auto* function : get_supported_server_functions(tservice, true)) {
    if (!skip_codegen(function)) {
      generate_process_function(tservice, function, true);
    }
  }
  generate_process_metadata_function(tservice, mangle);

  indent_down();
  f_service_ << "}\n";

  f_service_ << indent() << "class " << long_name << "AsyncProcessor extends "
             << long_name << "AsyncProcessorBase {\n"
             << indent() << "  const type TThriftIf = " << long_name
             << "AsyncIf;\n"
             << indent() << "}\n";

  f_service_ << "\n";
}

void t_hack_generator::generate_process_metadata_function(
    const t_service* tservice, bool mangle) {
  // Open function
  indent(f_service_) << "protected async function "
                        "process_getThriftServiceMetadata(int $seqid, "
                        "\\TProtocol $input, \\TProtocol $output): "
                        "Awaitable<void> {\n";
  indent_up();

  f_service_
      << indent()
      << "$this->process_getThriftServiceMetadataHelper($seqid, $input, $output, "
      << php_servicename_mangle(mangle, tservice) << "StaticMetadata::class"
      << ");\n";

  // Close function
  indent_down();
  f_service_ << indent() << "}\n";
}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_hack_generator::generate_process_function(
    const t_service* tservice, const t_function* tfunction, bool async) {
  // Open function
  indent(f_service_)
      << "protected" << (async ? " async" : "") << " function process_"
      << find_hack_name(tfunction)
      << "(int $seqid, \\TProtocol $input, \\TProtocol $output): "
      << (async ? "Awaitable<void>" : "void") << " {\n";
  indent_up();

  auto is_stream = tfunction->stream() != nullptr;

  std::string service_name = hack_name(tservice);
  std::string argsname = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::ARGS);
  std::string resultname = generate_function_helper_name(
      tservice,
      tfunction,
      is_stream ? PhpFunctionNameSuffix::FIRST_RESPONSE
                : PhpFunctionNameSuffix::RESULT);
  const std::string& fn_name = find_hack_name(tfunction);

  f_service_ << indent()
             << "$handler_ctx = $this->eventHandler_->getHandlerContext('"
             << fn_name << "');\n"
             << indent() << "$reply_type = \\TMessageType::REPLY;\n";

  // Declare result for non oneway function
  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    f_service_ << indent() << "$result = " << resultname
               << "::withDefaultValues();\n";
  }

  // Try block for a function with exceptions
  f_service_ << indent() << "try {\n";
  indent_up();
  f_service_ << indent() << "$args = $this->readHelper(" << argsname
             << "::class, $input, '" << fn_name << "', $handler_ctx);\n";

  // Generate the function call
  indent(f_service_) << "$this->eventHandler_->preExec($handler_ctx, '"
                     << service_name << "', '" << fn_name << "', $args);\n";

  f_service_ << indent();
  auto is_void = tfunction->return_type()->is_void();
  if (is_stream) {
    f_service_ << "$response_and_stream = ";
  } else if (
      tfunction->qualifier() != t_function_qualifier::oneway && !is_void) {
    f_service_ << "$result->success = ";
  }
  f_service_ << (async ? "await " : "") << "$this->handler->"
             << find_hack_name(tfunction) << "(";
  auto delim = "";
  for (const auto& param : tfunction->params().fields()) {
    f_service_ << delim << "$args->" << param.name();
    delim = ", ";
  }
  f_service_ << ");\n";

  if (is_stream) {
    if (!is_void) {
      f_service_ << indent()
                 << "$result->success = $response_and_stream->response;\n";
    }

    std::string stream_response_type = generate_function_helper_name(
        tservice, tfunction, PhpFunctionNameSuffix::STREAM_RESPONSE);
    f_service_ << indent() << "$this->eventHandler_->postExec($handler_ctx, '"
               << fn_name << "', $result);\n"
               << indent() << "$this->writeHelper($result, '" << fn_name
               << "', $seqid, $handler_ctx, $output, $reply_type);\n"
               << indent()
               << "await $this->genExecuteStream($response_and_stream->stream, "
               << stream_response_type << "::class, $output);\n"
               << indent() << "return;\n";
  } else if (tfunction->qualifier() != t_function_qualifier::oneway) {
    indent(f_service_) << "$this->eventHandler_->postExec($handler_ctx, '"
                       << fn_name << "', $result);\n";
  }
  indent_down();
  f_service_ << indent() << "} catch (\\Exception $ex) {\n";
  indent_up();
  if (tfunction->qualifier() != t_function_qualifier::oneway &&
      !get_elems(tfunction->exceptions()).empty()) {
    f_service_ << indent() << "if ($result->setException($ex)) {\n";
    indent_up();
    f_service_ << indent()
               << "$this->eventHandler_->handlerException($handler_ctx, '"
               << fn_name << "', $ex);\n";
    indent_down();
    f_service_ << indent() << "} else {\n";
    indent_up();
    // If $ex is not a declared exception, wrap it in an application exception
    f_service_ << indent() << "$reply_type = \\TMessageType::EXCEPTION;\n"
               << indent()
               << "$this->eventHandler_->handlerError($handler_ctx, '"
               << fn_name << "', $ex);\n"
               << indent()
               << "$result = new "
                  "\\TApplicationException($ex->getMessage().\"\\n\".$ex->"
                  "getTraceAsString());\n";
    indent_down();
    f_service_ << indent() << "}\n";
  } else {
    f_service_ << indent() << "$reply_type = \\TMessageType::EXCEPTION;\n"
               << indent()
               << "$this->eventHandler_->handlerError($handler_ctx, '"
               << fn_name << "', $ex);\n"
               << indent()
               << "$result = new "
                  "\\TApplicationException($ex->getMessage().\"\\n\".$ex->"
                  "getTraceAsString());\n";
  }
  indent_down();
  f_service_ << indent() << "}\n";

  // Shortcut out here for oneway functions
  if (tfunction->qualifier() == t_function_qualifier::oneway) {
    f_service_ << indent() << "return;\n";
    indent_down();
    f_service_ << indent() << "}\n";
    return;
  }

  f_service_ << indent() << "$this->writeHelper($result, '" << fn_name
             << "', $seqid, $handler_ctx, $output, $reply_type);\n";

  // Close function
  indent_down();
  f_service_ << indent() << "}\n";
}

/**
 * Generates helper functions for a service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_hack_generator::generate_service_helpers(
    const t_service* tservice, bool mangle) {
  f_service_ << "// HELPER FUNCTIONS AND STRUCTURES\n\n";

  for (const auto* function : get_supported_client_functions(tservice)) {
    if (!skip_codegen(function)) {
      generate_php_function_helpers(tservice, function);
    }
  }

  for (const auto& interaction : get_interactions(tservice)) {
    for (const auto& function : get_supported_client_functions(interaction)) {
      generate_php_interaction_function_helpers(
          tservice, interaction, function);
    }
  }

  f_service_ << indent() << "class " << php_servicename_mangle(mangle, tservice)
             << "StaticMetadata implements \\IThriftServiceStaticMetadata {\n";
  indent_up();

  f_service_ << indent() << "const string THRIFT_SVC_NAME = '"
             << tservice->name() << "';\n\n";

  // Expose service metadata
  f_service_ << indent() << "public static function getServiceMetadata()[]: "
             << "\\tmeta_ThriftService {\n";
  indent_up();

  f_service_ << indent() << "return "
             << render_const_value(
                    tmeta_ThriftService_type(),
                    service_to_tmeta(tservice).get(),
                    /*immutable_collections*/ false,
                    /*ignore_wrapper*/ false,
                    /*force_arrays*/ true)
             << ";\n";

  indent_down();
  f_service_ << indent() << "}\n\n";

  // Expose all metadata
  f_service_ << indent()
             << "public static function getServiceMetadataResponse()[]: "
             << "\\tmeta_ThriftServiceMetadataResponse {\n";
  indent_up();

  f_service_ << render_service_metadata_response(tservice, mangle);

  indent_down();
  f_service_ << indent() << "}\n\n";

  // Structured annotations
  f_service_
      << indent()
      << "public static function getAllStructuredAnnotations()[write_props]: "
         "\\TServiceAnnotations {\n";
  indent_up();

  std::stringstream annotations_out;
  std::stringstream annotations_temp_var_initializations_out;
  t_name_generator namer;

  annotations_out << indent() << "return shape(\n";
  indent_up();
  annotations_out << indent() << "'service' => "
                  << render_structured_annotations(
                         tservice->structured_annotations(),
                         annotations_temp_var_initializations_out,
                         namer)
                  << ",\n";
  annotations_out << indent() << "'functions' => dict[\n";
  indent_up();
  for (const auto& function : get_supported_client_functions(tservice)) {
    if (function->structured_annotations().empty()) {
      continue;
    }
    annotations_out << indent() << "'" << find_hack_name(function) << "' => "
                    << render_structured_annotations(
                           function->structured_annotations(),
                           annotations_temp_var_initializations_out,
                           namer)
                    << ",\n";
  }

  f_service_ << annotations_temp_var_initializations_out.str();
  f_service_ << annotations_out.str();

  indent_down();
  f_service_ << indent() << "],\n";
  indent_down();
  f_service_ << indent() << ");\n";
  indent_down();
  f_service_ << indent() << "}\n";

  indent_down();
  f_service_ << indent() << "}\n\n";
}

void t_hack_generator::generate_service_interactions(
    const t_service* tservice, bool mangle) {
  const std::vector<const t_service*>& interactions =
      get_interactions(tservice);
  if (interactions.empty()) {
    return;
  }

  f_service_ << "// INTERACTION HANDLERS\n\n";

  const std::string& service_name = tservice->name();
  for (const auto* interaction : interactions) {
    f_service_ << indent() << "class "
               << php_servicename_mangle(
                      mangle,
                      interaction,
                      service_name + "_" + interaction->name())
               << " extends \\ThriftClientBase {\n";
    indent_up();
    f_service_ << indent() << "const string THRIFT_SVC_NAME = "
               << php_servicename_mangle(mangle, tservice)
               << "StaticMetadata::THRIFT_SVC_NAME;\n\n";

    f_service_ << indent() << "private \\InteractionId $interactionId;\n\n";

    f_service_
        << indent() << "public function __construct(" << "\\TProtocol $input, "
        << "?\\TProtocol $output = null, "
        << "?\\IThriftMigrationAsyncChannel $channel = null)[leak_safe] {\n";
    indent_up();
    f_service_ << indent()
               << "parent::__construct($input, $output, $channel);\n";
    f_service_ << indent() << "if ($this->channel_ is nonnull) {\n";
    indent_up();
    f_service_ << indent()
               << "$this->interactionId = $this->channel_->createInteraction("
               << render_string(interaction->name()) << ");\n";
    indent_down();
    f_service_ << indent() << "} else {\n";
    indent_up();
    f_service_ << indent() << "throw new \\Exception("
               << render_string(
                      "The channel must be nonnull to create interactions.")
               << ");\n";
    indent_down();
    f_service_ << indent() << "}\n";
    indent_down();
    f_service_ << indent() << "}\n\n";

    // Generate interaction method implementations
    for (const auto& function : get_supported_client_functions(interaction)) {
      if (skip_codegen(function)) {
        continue;
      }
      _generate_service_client_child_fn(f_service_, interaction, function);
      _generate_sendImpl(f_service_, interaction, function);
    }

    indent_down();
    f_service_ << indent() << "}\n\n";
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_hack_generator::generate_php_function_helpers(
    const t_service* tservice, const t_function* tfunction) {
  const std::string& service_name = tservice->name();
  if (tfunction->stream()) {
    generate_php_stream_function_helpers(tfunction, service_name);
    return;
  } else if (tfunction->sink()) {
    generate_php_sink_function_helpers(tfunction, service_name);
    return;
  }

  generate_php_function_args_helpers(tfunction, service_name);

  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    generate_php_function_result_helpers(
        tfunction,
        tfunction->return_type().get_type(),
        tfunction->exceptions(),
        service_name,
        "_result",
        tfunction->return_type()->is_void());
  }
}

void t_hack_generator::generate_php_function_result_helpers(
    const t_function* tfunction,
    const t_type* ttype,
    const t_throws* ex,
    const std::string& prefix,
    const std::string& suffix,
    bool is_void) {
  bool is_result_struct =
      (suffix == "_result" || suffix == "_StreamResponse" ||
       suffix == "_FirstResponse" || suffix == "_SinkPayload" ||
       suffix == "_FinalResponse");
  if (!is_result_struct) {
    t_struct result(
        program_, prefix + "_" + find_hack_name(tfunction) + suffix);

    generate_php_struct_definition_result_helpers(&result, ttype, ex, is_void);
  } else {
    t_result_struct result_struct(
        program_,
        prefix + "_" + find_hack_name(tfunction) + suffix,
        ttype != nullptr ? type_to_typehint(ttype) : "void");
    generate_php_struct_definition_result_helpers(
        &result_struct, ttype, ex, is_void);
  }
}

void t_hack_generator::generate_php_struct_definition_result_helpers(
    t_structured* result,
    const t_type* ttype,
    const t_throws* ex,
    bool is_void) {
  if (ttype) {
    auto success = std::make_unique<t_field>(*ttype, "success", 0);
    if (!is_void) {
      result->append(std::move(success));
    }
  }
  if (ex != nullptr) {
    for (const auto& x : ex->fields()) {
      result->append(x.clone_DO_NOT_USE());
    }
  }
  generate_php_struct_definition(f_service_, result, ThriftStructType::RESULT);
}

void t_hack_generator::generate_php_function_args_helpers(
    const t_function* tfunction, const std::string& prefix) {
  const t_paramlist& params = tfunction->params();
  std::string params_name =
      prefix + "_" + find_hack_name(tfunction, params.name());
  generate_php_struct_definition(
      f_service_, &params, ThriftStructType::ARGS, params_name);
}

// Generates a struct and helpers for a stream function.
void t_hack_generator::generate_php_stream_function_helpers(
    const t_function* function, const std::string& prefix) {
  generate_php_function_args_helpers(function, prefix);

  const t_stream* stream = function->stream();
  generate_php_function_result_helpers(
      function,
      stream->elem_type().get_type(),
      stream->exceptions(),
      prefix,
      "_StreamResponse",
      false);

  generate_php_function_result_helpers(
      function,
      function->return_type().get_type(),
      function->exceptions(),
      prefix,
      "_FirstResponse",
      function->has_void_initial_response());
}

/**
 * Generates a struct and helpers for a sink function.
 *
 * @param tfunction The function
 */
void t_hack_generator::generate_php_sink_function_helpers(
    const t_function* function, const std::string& prefix) {
  generate_php_function_args_helpers(function, prefix);

  const t_sink* sink = function->sink();

  generate_php_function_result_helpers(
      function,
      function->return_type().get_type(),
      function->exceptions(),
      prefix,
      "_FirstResponse",
      function->has_void_initial_response());

  generate_php_function_result_helpers(
      function,
      sink->get_elem_type(),
      sink->sink_exceptions(),
      prefix,
      "_SinkPayload",
      /* is_void */ false);

  generate_php_function_result_helpers(
      function,
      sink->get_final_response_type(),
      sink->final_response_exceptions(),
      prefix,
      "_FinalResponse",
      /* is_void */ false);
}

/**
 * Generates a struct and helpers for an interaction function
 */
void t_hack_generator::generate_php_interaction_function_helpers(
    const t_service* tservice,
    const t_service* interaction,
    const t_function* tfunction) {
  const std::string& prefix = tservice->name() + "_" + interaction->name();
  if (tfunction->stream()) {
    generate_php_stream_function_helpers(tfunction, prefix);
    return;
  } else if (tfunction->sink()) {
    generate_php_sink_function_helpers(tfunction, prefix);
    return;
  }
  generate_php_function_args_helpers(tfunction, prefix);

  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    generate_php_function_result_helpers(
        tfunction,
        tfunction->return_type().get_type(),
        tfunction->exceptions(),
        prefix,
        "_result",
        tfunction->return_type()->is_void());
  }
}

/**
 * Generates the docstring for a generic object.
 */
void t_hack_generator::generate_php_docstring(
    std::ofstream& out, const t_named* named_node) {
  if (named_node->has_doc()) {
    generate_docstring_comment(
        out, // out
        "/**\n", // comment_start
        " * ", // line_prefix
        named_node->doc(),
        " */\n"); // comment_end
  }
}

/**
 * Generates the docstring for a function.
 *
 * This is how the generated docstring looks like:-
 *
 * <Original docstring goes here>
 *
 * Original thrift definition:-
 * return_type
 *   functionName(1: argType1 arg_name1,
 *                2: argType2 arg_name2)
 *   throws (1: exceptionType1 ex_name1,
 *           2: exceptionType2 ex_name2);
 */
void t_hack_generator::generate_php_docstring(
    std::ofstream& out, const t_function* tfunction) {
  indent(out) << "/**\n";
  // Copy the doc.
  if (tfunction->has_doc()) {
    generate_docstring_comment(
        out, // out
        "", // comment_start
        " * ", // line_prefix
        tfunction->doc(),
        ""); // comment_end
  }

  // Also write the original thrift function definition.
  if (tfunction->has_doc()) {
    indent(out) << " * \n";
  }
  indent(out) << " * " << "Original thrift definition:-\n";
  // Return type.
  indent(out) << " * ";
  if (tfunction->qualifier() == t_function_qualifier::oneway) {
    out << "oneway ";
  }
  if (const t_stream* stream = tfunction->stream()) {
    if (!tfunction->has_void_initial_response()) {
      out << thrift_type_name(tfunction->return_type().get_type()) << ", ";
    } else {
      out << "void, ";
    }
    out << "stream<" << thrift_type_name(stream->elem_type().get_type());
    generate_php_docstring_stream_exceptions(out, stream->exceptions());
    out << ">\n";
  } else if (const t_sink* sink = tfunction->sink()) {
    if (!tfunction->has_void_initial_response()) {
      out << thrift_type_name(tfunction->return_type().get_type()) << ", ";
    } else {
      out << "void, ";
    }
    out << "sink<" << thrift_type_name(sink->get_elem_type());
    generate_php_docstring_stream_exceptions(out, sink->sink_exceptions());

    out << ", " << thrift_type_name(sink->get_final_response_type());
    generate_php_docstring_stream_exceptions(
        out, sink->final_response_exceptions());
    out << ">\n";
  } else {
    out << thrift_type_name(tfunction->return_type().get_type()) << "\n";
  }

  // Function name.
  indent(out) << " * " << indent(1) << find_hack_name(tfunction) << "(";
  // Find the position after the " * " from where the function arguments
  // should be rendered.
  auto start_pos = get_indent_size() + find_hack_name(tfunction).size() + 1;

  // Parameters.
  generate_php_docstring_args(out, start_pos, &tfunction->params());
  out << ")";

  // Exceptions.
  if (!get_elems(tfunction->exceptions()).empty()) {
    out << "\n" << indent() << " * " << indent(1) << "throws (";
    // Find the position after the " * " from where the exceptions should be
    // rendered.
    start_pos = get_indent_size() + strlen("throws (");
    generate_php_docstring_args(out, start_pos, tfunction->exceptions());
    out << ")";
  }
  out << ";\n";
  indent(out) << " */\n";
}

/**
 * Generates the docstring for a field.
 *
 * This is how the generated docstring looks like:-
 *
 * <Original docstring goes here>
 *
 * Original thrift field:-
 * argNumber: argType argName
 */
void t_hack_generator::generate_php_docstring(
    std::ofstream& out, const t_field* tfield) {
  indent(out) << "/**\n";
  // Copy the doc.
  if (tfield->has_doc()) {
    generate_docstring_comment(
        out, // out
        "", // comment_start
        " * ", // line_prefix
        tfield->doc(),
        ""); // comment_end
    indent(out) << " * \n";
  }
  indent(out) << " * " << "Original thrift field:-\n";
  indent(out) << " * " << tfield->get_key() << ": "
              << tfield->get_type()->get_full_name() << " " << tfield->name()
              << "\n";
  indent(out) << " */\n";
}

/**
 * Generates the docstring for a struct.
 *
 * This is how the generated docstring looks like:-
 *
 * <Original docstring goes here>
 *
 * Original thrift struct/exception:-
 * Name
 */
void t_hack_generator::generate_php_docstring(
    std::ofstream& out, const t_structured* tstruct, bool is_exception) {
  indent(out) << "/**\n";
  // Copy the doc.
  if (tstruct->has_doc()) {
    generate_docstring_comment(
        out, // out
        "", // comment_start
        " * ", // line_prefix
        tstruct->doc(),
        ""); // comment_end
    indent(out) << " *\n";
  }
  indent(out) << " * " << "Original thrift ";
  if (is_exception) {
    out << "exception";
  } else if (tstruct->is<t_union>()) {
    out << "union";
  } else {
    out << "struct";
  };
  out << ":-\n";
  indent(out) << " * " << tstruct->name() << "\n";
  indent(out) << " */\n";
}

/**
 * Generates the docstring for an enum.
 *
 * This is how the generated docstring looks like:-
 *
 * <Original docstring goes here>
 *
 * Original thrift enum:-
 * Name
 */
void t_hack_generator::generate_php_docstring(
    std::ofstream& out, const t_enum* tenum) {
  indent(out) << "/**\n";
  // Copy the doc.
  if (tenum->has_doc()) {
    generate_docstring_comment(
        out, // out
        "", // comment_start
        " * ", // line_prefix
        tenum->doc(),
        ""); // comment_end
    indent(out) << " * \n";
  }
  indent(out) << " * " << "Original thrift enum:-\n";
  indent(out) << " * " << tenum->name() << "\n";
  indent(out) << " */\n";
}

/**
 * Generates the docstring for a service.
 *
 * This is how the generated docstring looks like:-
 *
 * <Original docstring goes here>
 *
 * Original thrift service:-
 * Name
 */
void t_hack_generator::generate_php_docstring(
    std::ofstream& out, const t_service* tservice) {
  indent(out) << "/**\n";
  // Copy the doc.
  if (tservice->has_doc()) {
    generate_docstring_comment(
        out, // out
        "", // comment_start
        " * ", // line_prefix
        tservice->doc(),
        ""); // comment_end
    indent(out) << " * \n";
  }
  indent(out) << " * " << "Original thrift service:-\n";
  indent(out) << " * " << tservice->name() << "\n";
  indent(out) << " */\n";
}

/**
 * Generates the docstring for a constant.
 *
 * This is how the generated docstring looks like:-
 *
 * <Original docstring goes here>
 *
 * Original thrift constant:-
 * TYPE NAME
 */
void t_hack_generator::generate_php_docstring(
    std::ofstream& out, const t_const* tconst) {
  indent(out) << "/**\n";
  // Copy the doc.
  if (tconst->has_doc()) {
    generate_docstring_comment(
        out, // out
        "", // comment_start
        " * ", // line_prefix
        tconst->doc(),
        ""); // comment_end
    indent(out) << " * \n";
  }
  indent(out) << " * " << "Original thrift constant:-\n";
  indent(out) << " * " << tconst->type()->get_full_name() << " "
              << tconst->name() << "\n";
  // no value because it could have characters that mess up the comment
  indent(out) << " */\n";
}

/**
 * Generates the docstring for function arguments and exceptions.
 *
 * @param size_t start_pos the position (after " * ") from which the rendering
 * of arguments should start. In other words, we put that many space after " * "
 * and then render the argument.
 */
void t_hack_generator::generate_php_docstring_args(
    std::ofstream& out, size_t start_pos, const t_structured* arg_list) {
  if (arg_list) {
    bool first = true;
    for (const auto& param : arg_list->fields()) {
      if (first) {
        first = false;
      } else {
        out << ",\n" << indent() << " * " << std::string(start_pos, ' ');
      }
      out << param.id() << ": " << thrift_type_name(param.get_type()) << " "
          << param.name();
    }
  }
}

void t_hack_generator::generate_php_docstring_stream_exceptions(
    std::ofstream& out, const t_throws* ex) {
  auto params = get_elems(ex);
  if (params.empty()) {
    return;
  }
  out << ", throws (";
  auto first = true;
  for (const t_field& param : params) {
    if (first) {
      first = false;
    } else {
      out << ", ";
    }
    out << param.id() << ": " << thrift_type_name(param.get_type()) << " "
        << param.name();
  }
  out << ")";
}

std::string t_hack_generator::get_container_keyword(
    const t_type* ttype, std::map<TypeToTypehintVariations, bool> variations) {
  std::string hack_collection, hack_array;
  bool immutable_collections =
      variations[TypeToTypehintVariations::IMMUTABLE_COLLECTIONS] ||
      const_collections_;
  if (ttype->is<t_map>()) {
    hack_array = "dict";
    hack_collection = immutable_collections ? "\\ConstMap" : "Map";
  } else if (ttype->is<t_list>()) {
    hack_array = "vec";
    hack_collection = immutable_collections ? "\\ConstVector" : "Vector";
  }

  if (!hack_collections_ || variations[TypeToTypehintVariations::IS_SHAPE]) {
    return hack_array;
  }
  return hack_collection;
}

std::string t_hack_generator::typedef_to_typehint(
    const t_typedef* ttypedef,
    std::map<TypeToTypehintVariations, bool> variations) {
  auto [wrapper, name, ns] = find_hack_wrapper(ttypedef, false);
  std::string typehint;

  if (auto adapter = find_hack_adapter(ttypedef)) {
    typehint = *adapter + "::THackType";
  } else {
    typehint = type_to_typehint(ttypedef->get_type(), variations);
    if (!wrapper && variations[TypeToTypehintVariations::IS_SHAPE]) {
      return typehint;
    }
  }
  bool typedef_option =
      !variations[TypeToTypehintVariations::IGNORE_TYPEDEF_OPTION] && typedef_;
  if (wrapper) {
    bool ignore_wrapper =
        variations[TypeToTypehintVariations::RECURSIVE_IGNORE_WRAPPER] ||
        variations[TypeToTypehintVariations::IGNORE_WRAPPER];
    if (ignore_wrapper) {
      return typedef_option ? hack_wrapped_type_name(name, ns) : typehint;
    } else {
      return typedef_option ? hack_name(ttypedef)
                            : (*wrapper + "<" + typehint + ">");
    }
  } else {
    return typedef_option ? hack_name(ttypedef) : typehint;
  }
}

/**
 * Generate an appropriate string for a php typehint
 */
std::string t_hack_generator::type_to_typehint(
    const t_type* ttype, std::map<TypeToTypehintVariations, bool> variations) {
  if (const auto* ttypedef =
          dynamic_cast<const t_placeholder_typedef*>(ttype)) {
    ttype = ttypedef->get_type();
  }
  if (const auto* ttypedef = dynamic_cast<const t_typedef*>(ttype)) {
    return typedef_to_typehint(ttypedef, variations);
  }

  if (ttype->is<t_structured>()) {
    std::string struct_name = hack_name(ttype);

    auto [wrapper, name, ns] = find_hack_wrapper(ttype, false);

    if (wrapper) {
      if (variations[TypeToTypehintVariations::RECURSIVE_IGNORE_WRAPPER] ||
          variations[TypeToTypehintVariations::IGNORE_WRAPPER]) {
        // If wrapper is ignored, then use the underlying_type
        struct_name = hack_wrapped_type_name(name, ns);
      }
    }
    return struct_name +
        (variations[TypeToTypehintVariations::IS_SHAPE] ? "::TShape" : "");
  }

  ttype = ttype->get_true_type();
  if (const auto* primitive = ttype->try_as<t_primitive_type>()) {
    switch (primitive->primitive_type()) {
      case t_primitive_type::type::t_void:
        return "void";
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        return "string";
      case t_primitive_type::type::t_bool:
        return "bool";
      case t_primitive_type::type::t_byte:
      case t_primitive_type::type::t_i16:
      case t_primitive_type::type::t_i32:
      case t_primitive_type::type::t_i64:
        return "int";
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        return "float";
      default:
        return "mixed";
    }
  } else if (const auto* tenum = dynamic_cast<const t_enum*>(ttype)) {
    if (is_bitmask_enum(tenum)) {
      return "int";
    } else {
      return hack_name(ttype);
    }
  } else if (const auto* tlist = dynamic_cast<const t_list*>(ttype)) {
    std::string prefix = get_container_keyword(ttype, variations);
    return prefix + "<" + type_to_typehint(tlist->get_elem_type(), variations) +
        ">";
  } else if (const auto* tmap = dynamic_cast<const t_map*>(ttype)) {
    std::string prefix = get_container_keyword(ttype, variations);
    std::string key_type =
        type_to_typehint(&tmap->key_type().deref(), variations);
    if (variations[TypeToTypehintVariations::IS_SHAPE] && shape_arraykeys_ &&
        key_type == "string") {
      key_type = "arraykey";
    }
    return prefix + "<" + key_type + ", " +
        type_to_typehint(tmap->get_val_type(), variations) + ">";
  } else if (const auto* tset = dynamic_cast<const t_set*>(ttype)) {
    std::string prefix;
    std::string suffix = ">";
    if (!legacy_arrays_ && !hack_collections_) {
      prefix = "keyset";
    } else if (arraysets_ || variations[TypeToTypehintVariations::IS_SHAPE]) {
      prefix = "dict";
      suffix = ", bool>";
    } else {
      prefix = variations[TypeToTypehintVariations::IMMUTABLE_COLLECTIONS] ||
              const_collections_
          ? "\\ConstSet"
          : "Set";
    }
    return prefix + "<" + type_to_typehint(tset->get_elem_type(), variations) +
        suffix;
  } else {
    return "mixed";
  }
}

std::string t_hack_generator::field_to_typehint(
    const t_field& tfield,
    const std::string& struct_class_name,
    bool is_field_nullable) {
  std::string typehint;
  // Check the adapter before resolving typedefs.
  if (std::optional<std::string> adapter = find_hack_field_adapter(tfield)) {
    typehint = *adapter + "::THackType";
  }

  if (typehint.empty()) {
    typehint = type_to_typehint(tfield.get_type());
  }
  if (std::optional<std::string> field_wrapper = find_hack_wrapper(tfield)) {
    typehint = *field_wrapper + "<" + (is_field_nullable ? "?" : "") +
        typehint + ", " + struct_class_name + ">";
  }
  return typehint;
}

std::string t_hack_generator::get_stream_function_return_typehint(
    const t_function* function) {
  // Finally, the function declaration.
  std::string return_typehint;

  auto stream_response_type_hint =
      type_to_typehint(function->stream()->elem_type().get_type()) + ">";

  if (!function->has_void_initial_response()) {
    auto first_response_type_hint =
        type_to_typehint(function->return_type().get_type());
    return_typehint = "\\ResponseAndStream<" + first_response_type_hint + ", " +
        stream_response_type_hint;
  } else {
    return_typehint = "\\ResponseAndStream<null, " + stream_response_type_hint;
  }
  return return_typehint;
}

std::string t_hack_generator::get_sink_function_return_typehint(
    const t_function* function) {
  // Finally, the function declaration.
  const t_sink* sink = function->sink();
  std::string return_typehint = type_to_typehint(sink->get_elem_type()) + ", " +
      type_to_typehint(sink->get_final_response_type()) + ">";

  if (function->has_void_initial_response()) {
    return "\\ResponseAndClientSink<null, " + return_typehint;
  }
  auto first_response_type_hint =
      type_to_typehint(function->return_type().get_type());
  return "\\ResponseAndClientSink<" + first_response_type_hint + ", " +
      return_typehint;
}
/**
 * Generate an appropriate string for a parameter typehint.
 * The difference from type_to_typehint() is for parameters we should accept
 * an array or a collection type, so we return KeyedContainer
 */
std::string t_hack_generator::type_to_param_typehint(
    const t_type* ttype, bool nullable) {
  std::string prefix = (nullable ? "?" : "");
  if (const auto* tlist = dynamic_cast<const t_list*>(ttype)) {
    if (strict_types_) {
      return prefix + type_to_typehint(ttype);
    } else {
      return prefix + "KeyedContainer<int, " +
          type_to_param_typehint(tlist->get_elem_type()) + ">";
    }
  } else if (const auto* tmap = dynamic_cast<const t_map*>(ttype)) {
    if (strict_types_) {
      return prefix + type_to_typehint(ttype);
    } else {
      const t_type* key_type = &tmap->key_type().deref();
      return prefix + "KeyedContainer<" +
          (!is_type_arraykey(*key_type) ? "arraykey"
                                        : type_to_param_typehint(key_type)) +
          ", " + type_to_param_typehint(tmap->get_val_type()) + ">";
    }
  } else {
    return prefix + type_to_typehint(ttype);
  }
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 * @param mangle Generate mangled service classes
 */
void t_hack_generator::generate_service_interface(
    const t_service* tservice, bool mangle, bool async, bool client) {
  generate_php_docstring(f_service_, tservice);
  std::string suffix =
      std::string(async ? "Async" : "") + (client ? "Client" : "");
  std::string extends_if =
      std::string("\\IThrift") + (async ? "Async" : "Sync") + "If";

  std::string long_name = php_servicename_mangle(mangle, tservice);
  if (async && client) {
    extends_if = long_name + "Async" + "If";
    if (tservice->extends() != nullptr) {
      std::string ext_prefix =
          php_servicename_mangle(mangle, tservice->extends(), true);
      extends_if = extends_if + ", " + ext_prefix + suffix + "If";
    }
  } else if (tservice->extends() != nullptr) {
    std::string ext_prefix =
        php_servicename_mangle(mangle, tservice->extends(), true);
    extends_if = ext_prefix + suffix + "If";
  }

  generate_hack_attributes(
      f_service_,
      tservice,
      /*include_user_defined*/ false);
  f_service_ << "interface " << long_name << suffix << "If extends "
             << extends_if << " {\n";
  indent_up();
  auto delim = "";
  auto functions = client ? get_supported_client_functions(tservice)
                          : get_supported_server_functions(tservice, async);

  auto svc_mod_int =
      tservice->has_structured_annotation(kHackModuleInternalUri);

  for (const t_function* function : functions) {
    if (skip_codegen(function) ||
        (async && client && is_function_supported(function, false, true))) {
      continue;
    }
    // Add a blank line before the start of a new function definition
    f_service_ << delim;
    delim = "\n";

    // Add the doxygen style comments.
    generate_php_docstring(f_service_, function);

    // Finally, the function declaration.
    std::string return_typehint;

    if (function->stream() != nullptr) {
      return_typehint = get_stream_function_return_typehint(function);
    } else if (function->sink() != nullptr) {
      return_typehint = get_sink_function_return_typehint(function);
    } else {
      return_typehint = type_to_typehint(function->return_type().get_type());
    }

    if (async || client) {
      return_typehint.insert(0, "Awaitable<");
      return_typehint.append(">");
    }
    auto fqlfr = (svc_mod_int || has_hack_module_internal(function))
        ? "internal"
        : "public";
    if (nullable_everything_) {
      const std::string& funname = find_hack_name(function);
      indent(f_service_) << fqlfr << " function " << funname << "("
                         << argument_list(function->params(), "", true, true)
                         << "): " << return_typehint << ";\n";
    } else {
      indent(f_service_) << fqlfr << " function "
                         << function_signature(function, "", return_typehint)
                         << ";\n";
    }
  }
  indent_down();
  f_service_ << "}\n\n";
}

void t_hack_generator::generate_service_client(
    const t_service* tservice, bool mangle) {
  _generate_service_client(f_service_, tservice, mangle);
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_hack_generator::_generate_service_client(
    std::ofstream& out, const t_service* tservice, bool mangle) {
  generate_php_docstring(out, tservice);

  std::string long_name = php_servicename_mangle(mangle, tservice);

  const std::string module = program_->get_namespace("hack.module");
  if (!module.empty()) {
    out << "internal ";
  }
  out << "trait " << long_name << "ClientBase {\n"
      << "  require extends \\ThriftClientBase;\n\n";
  indent_up();

  // Generate factory method for interactions
  const std::vector<const t_service*>& interactions =
      get_interactions(tservice);
  if (!interactions.empty()) {
    out << indent() << "/* interaction handlers factory methods */\n";
    const std::string& service_name = tservice->name();
    for (const auto& interaction : interactions) {
      const std::string& handle_name = php_servicename_mangle(
          mangle, interaction, service_name + "_" + interaction->name());
      out << indent() << "public function create" << interaction->name()
          << "(): ";
      out << handle_name << " {\n";
      indent_up();

      out << indent() << "$interaction = new " << handle_name
          << "($this->input_, $this->output_, $this->channel_);\n";
      out << indent() << "$interaction->setAsyncHandler($this->asyncHandler_)"
          << "->setEventHandler($this->eventHandler_);\n";
      out << indent() << "return $interaction;\n";

      indent_down();
      out << indent() << "}\n\n";
    }
  }

  // Generate functions as necessary.
  for (const auto* function : get_supported_client_functions(tservice)) {
    if (skip_codegen(function)) {
      continue;
    }
    _generate_service_client_child_fn(out, tservice, function);
    if (legacy_arrays_) {
      _generate_service_client_child_fn(
          out,
          tservice,
          function,
          /*legacy_arrays*/ true);
    }
  }

  scope_down(out);
  out << "\n";

  _generate_service_client_children(out, tservice, mangle, /*async*/ true);
  _generate_service_client_children(out, tservice, mangle, /*async*/ false);
}

void t_hack_generator::_generate_current_seq_id(
    std::ofstream& out,
    const t_service* tservice,
    const t_function* tfunction) {
  if (tservice->is<t_interaction>()) {
    indent(out) << "$currentseqid = $this->sendImpl_" << tfunction->name()
                << "(";

    auto delim = "";
    for (const auto& param : tfunction->params().fields()) {
      out << delim << "$" << param.name();
      delim = ", ";
    }
    out << ");\n";
  } else {
    out << indent() << "$currentseqid = ";
    _generate_sendImplHelper(out, tfunction, tservice);
  }
}

void t_hack_generator::_generate_sendImpl(
    std::ofstream& out,
    const t_service* tservice,
    const t_function* tfunction) {
  const std::string& funname = tfunction->name();
  const std::string& rpc_function_name =
      generate_rpc_function_name(tservice, tfunction);
  const std::string& tservice_name =
      (tservice->is<t_interaction>() ? service_name_ : tservice->name());

  if (nullable_everything_) {
    indent(out) << "protected function sendImpl_" << funname << "("
                << argument_list(tfunction->params(), "", true, true)
                << "): int {\n";
  } else {
    indent(out) << "protected function sendImpl_"
                << function_signature(tfunction, "", "int") << " {\n";
  }
  indent_up();

  out << indent() << "$currentseqid = $this->getNextSequenceID();\n";
  _generate_args(out, tservice, tfunction);

  out << indent() << "try {\n";
  indent_up();
  out << indent() << "$this->eventHandler_->preSend('" << rpc_function_name
      << "', $args, $currentseqid, '" << tservice_name << "');\n";
  out << indent() << "if ($this->output_ is \\TBinaryProtocolAccelerated)\n";
  scope_up(out);

  out << indent() << "\\thrift_protocol_write_binary($this->output_, '"
      << rpc_function_name << "', "
      << "\\TMessageType::CALL, $args, $currentseqid, "
      << "$this->output_->isStrictWrite(), "
      << (tfunction->qualifier() == t_function_qualifier::oneway ? "true"
                                                                 : "false")
      << ");\n";

  scope_down(out);
  out << indent()
      << "else if ($this->output_ is \\TCompactProtocolAccelerated)\n";
  scope_up(out);

  out << indent() << "\\thrift_protocol_write_compact2($this->output_, '"
      << rpc_function_name << "', "
      << "\\TMessageType::CALL, $args, $currentseqid, "
      << (tfunction->qualifier() == t_function_qualifier::oneway ? "true"
                                                                 : "false")
      << ", \\TCompactProtocolBase::VERSION);\n";

  scope_down(out);
  out << indent() << "else\n";
  scope_up(out);

  // Serialize the request header
  out << indent() << "$this->output_->writeMessageBegin('" << rpc_function_name
      << "', \\TMessageType::CALL, $currentseqid);\n";

  // Write to the stream
  out << indent() << "$args->write($this->output_);\n"
      << indent() << "$this->output_->writeMessageEnd();\n";
  if (tfunction->qualifier() == t_function_qualifier::oneway) {
    out << indent() << "$this->output_->getTransport()->onewayFlush();\n";
  } else {
    out << indent() << "$this->output_->getTransport()->flush();\n";
  }

  scope_down(out);

  indent_down();
  indent(out) << "} catch (\\THandlerShortCircuitException $ex) {\n";
  indent_up();
  out << indent() << "switch ($ex->resultType) {\n"
      << indent() << "  case \\THandlerShortCircuitException::R_EXPECTED_EX:\n"
      << indent()
      << "  case \\THandlerShortCircuitException::R_UNEXPECTED_EX:\n"
      << indent() << "    $this->eventHandler_->sendError('"
      << rpc_function_name << "', $args, $currentseqid, $ex->result);" << "\n"
      << indent() << "    throw $ex->result;\n"
      << indent() << "  case \\THandlerShortCircuitException::R_SUCCESS:\n"
      << indent() << "  default:\n"
      << indent() << "    $this->eventHandler_->postSend('" << rpc_function_name
      << "', $args, $currentseqid);\n"
      << indent() << "    return $currentseqid;\n"
      << indent() << "}\n";
  indent_down();
  indent(out) << "} catch (\\Exception $ex) {\n";
  indent_up();
  out << indent() << "$this->eventHandler_->sendError('" << rpc_function_name
      << "', $args, $currentseqid, $ex);\n"
      << indent() << "throw $ex;\n";
  indent_down();
  indent(out) << "}\n";

  out << indent() << "$this->eventHandler_->postSend('" << rpc_function_name
      << "', $args, $currentseqid);\n";

  indent(out) << "return $currentseqid;\n";

  scope_down(out);
}

// If !strict_types, containers are typehinted as KeyedContainer<Key, Value>
// to better support passing in arrays/dicts/maps/vecs/vectors and
// handle backwards compatibility. However, structs are typehinted as
// the actual container (ex: Map<Key, Val>), and we need to safely
// convert the typehints.
//
// This isn't as simple as dict($param) or new Map($param). If there is
// a nested container, that also needs to have its typehints converted.
// This iterates through the type object and generates the appropriate
// code to convert all the nested typehints.
void t_hack_generator::_generate_sendImpl_arg(
    std::ofstream& out,
    t_name_generator& namer,
    const std::string& var,
    const t_type* t) {
  const t_type* val_type = nullptr;
  if (strict_types_ || !t->is<t_container>()) {
    out << var;
    return;
  }

  if (const auto* tmap = dynamic_cast<const t_map*>(t)) {
    val_type = tmap->get_val_type();
  } else if (const auto* tlist = dynamic_cast<const t_list*>(t)) {
    val_type = tlist->get_elem_type();
  } else if (const auto* tset = dynamic_cast<const t_set*>(t)) {
    val_type = tset->get_elem_type();
  } else {
    throw std::runtime_error("Unknown container type");
  }

  val_type = val_type->get_true_type();
  if (val_type->is<t_container>() && !val_type->is<t_set>()) {
    if (t->is<t_map>()) {
      if (hack_collections_) {
        out << "(new Map(" << var << "))->map(";
      } else {
        out << "Dict\\map(" << var << ", ";
      }
    } else if (t->is<t_list>()) {
      if (hack_collections_) {
        out << "(new Vector(" << var << "))->map(";
      } else {
        out << "Vec\\map(" << var << ", ";
      }
    } else if (t->is<t_set>()) {
      throw std::runtime_error("Sets can't have nested containers");
    } else {
      throw std::runtime_error("Unknown container type");
    }
    indent_up();
    out << "\n" << indent();
    // Update var to what it will be next, since we no longer need the old
    // value.
    std::string new_var = "$" + namer("_val");
    out << new_var << " ==> ";
    this->_generate_sendImpl_arg(out, namer, new_var, val_type);
    indent_down();
    out << "\n" << indent();
    out << ")";
  } else {
    // the parens around the collections are unnecessary but I'm leaving them
    // so that I don't end up changing literally all files
    if (t->is<t_map>()) {
      if (hack_collections_) {
        out << "new Map(" << var << ")";
      } else {
        out << "dict(" << var << ")";
      }
    } else if (t->is<t_list>()) {
      if (hack_collections_) {
        out << "new Vector(" << var << ")";
      } else {
        out << "vec(" << var << ")";
      }
    } else if (t->is<t_set>()) {
      out << var;
    } else {
      throw std::runtime_error("Unknown container type");
    }
  }
}

void t_hack_generator::_generate_service_client_children(
    std::ofstream& out, const t_service* tservice, bool mangle, bool async) {
  auto attributes = find_attributes(*tservice, false);
  if (!attributes.empty()) {
    indent(out) << "<<" << attributes << ">>\n";
  }
  std::string long_name = php_servicename_mangle(mangle, tservice);
  std::string class_suffix = std::string(async ? "Async" : "");
  std::string interface_suffix = std::string(async ? "AsyncClient" : "Client");
  std::string extends = "\\ThriftClientBase";
  bool root = tservice->extends() == nullptr;
  if (!root) {
    extends = php_servicename_mangle(mangle, tservice->extends(), true) +
        class_suffix + "Client";
  }

  out << "class " << long_name << class_suffix << "Client extends " << extends
      << " implements " << long_name << interface_suffix << "If {\n"
      << "  use " << long_name << "ClientBase;\n\n";
  indent_up();
  out << indent() << "const string THRIFT_SVC_NAME = " << long_name
      << "StaticMetadata::THRIFT_SVC_NAME;\n\n";
  indent_down();
  out << "}\n\n";
}

void t_hack_generator::_generate_service_client_child_fn(
    std::ofstream& out,
    const t_service* tservice,
    const t_function* tfunction,
    bool legacy_arrays) {
  if (tfunction->stream()) {
    _generate_service_client_stream_child_fn(
        out, tservice, tfunction, legacy_arrays);
    return;
  }

  if (tfunction->sink()) {
    _generate_service_client_sink_child_fn(
        out, tservice, tfunction, legacy_arrays);
    return;
  }

  bool is_oneway = tfunction->qualifier() == t_function_qualifier::oneway;
  bool gen_header_method = !is_oneway &&
      (should_generate_client_header_methods(tfunction) ||
       should_generate_client_header_methods(tservice));
  bool is_void = tfunction->return_type()->is_void();
  std::string funname =
      find_hack_name(tfunction) + (legacy_arrays ? "__LEGACY_ARRAYS" : "");
  std::string long_name = php_servicename_mangle(mangled_services_, tservice);
  const std::string& tservice_name =
      (tservice->is<t_interaction>()
           ? "\"" + service_name_ + "\""
           : long_name + "StaticMetadata::THRIFT_SVC_NAME");
  std::string return_typehint =
      type_to_typehint(tfunction->return_type().get_type());

  generate_php_docstring(out, tfunction);
  auto generate_method_decl = [&](bool is_header_method) {
    indent(out) << (has_hack_module_internal(tservice) ||
                            has_hack_module_internal(tfunction)
                        ? "internal"
                        : "public")
                << " async function " << (is_header_method ? "header_" : "")
                << funname << "("
                << argument_list(
                       tfunction->params(), "", true, nullable_everything_)
                << "): Awaitable<";

    if (is_header_method) {
      if (!is_void) {
        out << "(" << return_typehint << ", ?dict<string,string>)";
      } else {
        out << "?dict<string,string>";
      }
    } else {
      out << return_typehint;
    }
    out << "> {\n";
  };

  generate_method_decl(false);
  indent_up();
  if (gen_header_method) {
    indent(out) << (!is_void ? "return (" : "") << "await $this->header_"
                << funname << "("
                << argument_list(tfunction->params(), "", false) << ")";
    if (!is_void) {
      out << ")[0]";
    }
    out << ";\n";
    scope_down(out);
    out << "\n";
    generate_method_decl(true);
    indent_up();
  }

  indent(out) << "$rpc_options = $this->getAndResetOptions() ?? "
              << (tservice->is<t_interaction>()
                      ? "new \\RpcOptions()"
                      : "\\ThriftClientBase::defaultOptions()")
              << ";\n";
  if (tservice->is<t_interaction>()) {
    indent(out) << "$rpc_options = "
                   "$rpc_options->setInteractionId($this->interactionId);\n";
  }

  _generate_args(out, tservice, tfunction);
  indent(out) << "await $this->asyncHandler_->genBefore(" << tservice_name
              << ", \"" << generate_rpc_function_name(tservice, tfunction)
              << "\", $args);\n";
  _generate_current_seq_id(out, tservice, tfunction);

  if (is_oneway) {
    out << indent() << "await $this->genAwaitNoResponse($rpc_options);\n";
    scope_down(out);
    out << "\n";
    return;
  }

  std::string resultname = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::RESULT);

  std::string rpc_call = fmt::format(
      "await $this->genAwaitResponse({}::class, \"{}\", {}, $currentseqid, $rpc_options{})",
      resultname,
      tfunction->name(),
      is_void ? "true" : "false",
      legacy_arrays ? ", shape('read_options' => \\THRIFT_MARK_LEGACY_ARRAYS)"
                    : "");

  if (gen_header_method) {
    if (is_void) {
      // return headers only if void
      indent(out) << "return (" << rpc_call << ")[1];\n";
    } else {
      // return both response and headers
      indent(out) << "return " << rpc_call << ";\n";
    }
  } else {
    // return nothing if void
    if (is_void) {
      indent(out) << rpc_call << ";\n";
    } else {
      // return response only
      indent(out) << "return (" << rpc_call << ")[0];\n";
    }
  }
  scope_down(out);
  out << "\n";
}

void t_hack_generator::_generate_service_client_stream_child_fn(
    std::ofstream& out,
    const t_service* tservice,
    const t_function* tfunction,
    bool legacy_arrays) {
  std::string funname =
      tfunction->name() + (legacy_arrays ? "__LEGACY_ARRAYS" : "");
  const std::string& tservice_name =
      (tservice->is<t_interaction>() ? service_name_ : tservice->name());
  std::string return_typehint = get_stream_function_return_typehint(tfunction);

  generate_php_docstring(out, tfunction);
  indent(out) << "public async function " << funname << "("
              << argument_list(
                     tfunction->params(), "", true, nullable_everything_)
              << "): Awaitable<" + return_typehint + "> {\n";

  indent_up();

  indent(out) << "$hh_frame_metadata = $this->getHHFrameMetadata();\n";
  indent(out) << "if ($hh_frame_metadata !== null) {\n";
  indent_up();
  indent(out) << "\\HH\\set_frame_metadata($hh_frame_metadata);\n";
  indent_down();
  indent(out) << "}\n";

  indent(out) << "$rpc_options = $this->getAndResetOptions() ?? "
              << (tservice->is<t_interaction>()
                      ? "new \\RpcOptions()"
                      : "\\ThriftClientBase::defaultOptions()")
              << ";\n";
  if (tservice->is<t_interaction>()) {
    indent(out) << "$rpc_options = "
                   "$rpc_options->setInteractionId($this->interactionId);\n";
  }

  _generate_args(out, tservice, tfunction);
  indent(out) << "await $this->asyncHandler_->genBefore(\"" << tservice_name
              << "\", \"" << generate_rpc_function_name(tservice, tfunction)
              << "\", $args);\n";
  _generate_current_seq_id(out, tservice, tfunction);

  std::string first_response_type = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::FIRST_RESPONSE);
  std::string stream_response_type = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::STREAM_RESPONSE);
  out << indent() << "return await $this->genAwaitStreamResponse("
      << first_response_type << "::class, " << stream_response_type
      << "::class, " << "\"" << tfunction->name() << "\", "
      << (tfunction->has_void_initial_response() ? "true" : "false")
      << ", $currentseqid, $rpc_options";
  if (legacy_arrays) {
    out << ", shape('read_options' => \\THRIFT_MARK_LEGACY_ARRAYS)";
  }
  out << ");\n";
  scope_down(out);
  out << "\n";
}

void t_hack_generator::_generate_service_client_sink_child_fn(
    std::ofstream& out,
    const t_service* tservice,
    const t_function* tfunction,
    bool legacy_arrays) {
  std::string funname =
      tfunction->name() + (legacy_arrays ? "__LEGACY_ARRAYS" : "");
  const std::string& tservice_name =
      (tservice->is<t_interaction>() ? service_name_ : tservice->name());

  generate_php_docstring(out, tfunction);
  std::string return_typehint = get_sink_function_return_typehint(tfunction);
  indent(out) << "public async function " << funname << "("
              << argument_list(
                     tfunction->params(), "", true, nullable_everything_)
              << "): Awaitable<" + return_typehint + "> {\n";

  indent_up();

  indent(out) << "$hh_frame_metadata = $this->getHHFrameMetadata();\n";
  indent(out) << "if ($hh_frame_metadata !== null) {\n";
  indent_up();
  indent(out) << "\\HH\\set_frame_metadata($hh_frame_metadata);\n";
  indent_down();
  indent(out) << "}\n";

  indent(out) << "$rpc_options = $this->getAndResetOptions() ?? "
              << (tservice->is<t_interaction>()
                      ? "new \\RpcOptions()"
                      : "\\ThriftClientBase::defaultOptions()")
              << ";\n";
  if (tservice->is<t_interaction>()) {
    out << "$rpc_options->setInteractionId($this->interactionId);\n";
  }

  _generate_args(out, tservice, tfunction);
  indent(out) << "await $this->asyncHandler_->genBefore(\"" << tservice_name
              << "\", \"" << generate_rpc_function_name(tservice, tfunction)
              << "\", $args);\n";
  _generate_current_seq_id(out, tservice, tfunction);

  std::string first_response_type = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::FIRST_RESPONSE);
  std::string sink_payload_type = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::SINK_PAYLOAD);
  std::string final_response_type = generate_function_helper_name(
      tservice, tfunction, PhpFunctionNameSuffix::SINK_FINAL_RESPONSE);
  out << indent() << "return await $this->genAwaitSinkResponse("
      << first_response_type << "::class, " << sink_payload_type << "::class, "
      << final_response_type << "::class, " << "\"" << tfunction->name()
      << "\", " << (tfunction->has_void_initial_response() ? "true" : "false")
      << ", $currentseqid, $rpc_options";
  if (legacy_arrays) {
    out << ", shape('read_options' => \\THRIFT_MARK_LEGACY_ARRAYS)";
  }
  out << ");\n";
  scope_down(out);
  out << "\n";
}

/**
 * Declares a field, which may include initialization as necessary.
 *
 * @param ttype The type
 * @param init iff initialize the field
 * @param obj iff the field is an object
 * @param thrift iff the object is a thrift object
 */
std::string t_hack_generator::declare_field(
    const t_field* tfield, bool init, bool obj, bool /*thrift*/) {
  std::string result = "$" + tfield->name();
  if (init) {
    const t_type* type = tfield->get_type()->get_true_type();
    if (const auto* tbase_type = type->try_as<t_primitive_type>()) {
      switch (tbase_type->primitive_type()) {
        case t_primitive_type::type::t_void:
          break;
        case t_primitive_type::type::t_string:
        case t_primitive_type::type::t_binary:
          result += " = ''";
          break;
        case t_primitive_type::type::t_bool:
          result += " = false";
          break;
        case t_primitive_type::type::t_byte:
        case t_primitive_type::type::t_i16:
        case t_primitive_type::type::t_i32:
        case t_primitive_type::type::t_i64:
          result += " = 0";
          break;
        case t_primitive_type::type::t_double:
        case t_primitive_type::type::t_float:
          result += " = 0.0";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no Hack initializer for base type " +
              t_primitive_type::type_name(tbase_type->primitive_type()));
      }
    } else if (type->is<t_enum>()) {
      result += " = null";
    } else if (type->is<t_map>()) {
      if (hack_collections_) {
        result += " = Map {}";
      } else {
        result += " = dict[]";
      }
    } else if (type->is<t_list>()) {
      if (hack_collections_) {
        result += " = Vector {}";
      } else {
        result += " = vec[]";
      }
    } else if (type->is<t_set>()) {
      if (arraysets_) {
        result += " = dict[]";
      } else if (hack_collections_) {
        result += " = Set {}";
      } else {
        result += " = keyset[]";
      }
    } else if (type->is<t_structured>()) {
      if (obj) {
        result += " = " + hack_name(type) + "::withDefaultValues()";
      } else {
        result += " = null";
      }
    }
  }
  return result + ";";
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
std::string t_hack_generator::function_signature(
    const t_function* tfunction,
    const std::string& more_tail_parameters,
    std::string typehint) {
  if (typehint.empty()) {
    typehint = type_to_typehint(tfunction->return_type().get_type());
  }

  return find_hack_name(tfunction) + "(" +
      argument_list(tfunction->params(), more_tail_parameters) +
      "): " + typehint;
}

/**
 * Renders a field list
 */
std::string t_hack_generator::argument_list(
    const t_paramlist& tparamlist,
    const std::string& more_tail_parameters,
    bool typehints,
    bool force_nullable) {
  std::string result;
  auto delim = "";
  for (const auto& field : tparamlist.fields()) {
    result += delim;
    delim = ", ";
    if (typehints) {
      auto true_type = field.get_type()->get_true_type();
      /*
       * force_nullable sets everything to null
       * Structs are nullable unless no_nullables_ is set.
       * Enums are always nullable
       */
      auto is_param_nullable = force_nullable || true_type->is<t_enum>() ||
          (!no_nullables_ &&
           (true_type->is<t_struct>() || true_type->is<t_union>()));
      result +=
          type_to_param_typehint(field.get_type(), is_param_nullable) + " ";
    }
    result += "$" + find_hack_name(&field);
  }

  if (more_tail_parameters.length() > 0) {
    result += delim;
    result += more_tail_parameters;
  }
  return result;
}

/**
 * Renders the function name to be used in RPC
 */
std::string t_hack_generator::generate_rpc_function_name(
    const t_service* tservice, const t_function* tfunction) const {
  std::string prefix =
      tservice->is<t_interaction>() ? tservice->name() + "." : "";
  return prefix + find_hack_name(tfunction);
}

/**
 * Generate function's helper structures name
 * @param suffix defines the suffix
 */
std::string t_hack_generator::generate_function_helper_name(
    const t_service* tservice,
    const t_function* tfunction,
    PhpFunctionNameSuffix suffix) {
  std::string prefix;
  if (tservice->is<t_interaction>()) {
    prefix = hack_name(service_name_, program_) + "_" + tservice->name();
  } else {
    prefix = hack_name(tservice);
  }
  std::string fname = find_hack_name(tfunction);
  switch (suffix) {
    case PhpFunctionNameSuffix::ARGS:
      return prefix + "_" + fname + "_args";
    case PhpFunctionNameSuffix::RESULT:
      return prefix + "_" + fname + "_result";
    case PhpFunctionNameSuffix::STREAM_RESPONSE:
      return prefix + "_" + fname + "_StreamResponse";
    case PhpFunctionNameSuffix::FIRST_RESPONSE:
      return prefix + "_" + fname + "_FirstResponse";
    case PhpFunctionNameSuffix::SINK_PAYLOAD:
      return prefix + "_" + fname + "_SinkPayload";
    case PhpFunctionNameSuffix::SINK_FINAL_RESPONSE:
      return prefix + "_" + fname + "_FinalResponse";
    default:
      throw std::runtime_error("Invalid php function name suffix");
  }
}

/**
 * Converts the parse type to a C++ enum string for the given type.
 */
std::string t_hack_generator::type_to_enum(const t_type* type) {
  type = type->get_true_type();

  if (const auto* tbase_type = type->try_as<t_primitive_type>()) {
    switch (tbase_type->primitive_type()) {
      case t_primitive_type::type::t_void:
        throw std::runtime_error("NO T_VOID CONSTRUCT");
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        return "\\TType::STRING";
      case t_primitive_type::type::t_bool:
        return "\\TType::BOOL";
      case t_primitive_type::type::t_byte:
        return "\\TType::BYTE";
      case t_primitive_type::type::t_i16:
        return "\\TType::I16";
      case t_primitive_type::type::t_i32:
        return "\\TType::I32";
      case t_primitive_type::type::t_i64:
        return "\\TType::I64";
      case t_primitive_type::type::t_double:
        return "\\TType::DOUBLE";
      case t_primitive_type::type::t_float:
        return "\\TType::FLOAT";
    }
  } else if (type->is<t_enum>()) {
    return "\\TType::I32";
  } else if (type->is<t_structured>()) {
    return "\\TType::STRUCT";
  } else if (type->is<t_map>()) {
    return "\\TType::MAP";
  } else if (type->is<t_set>()) {
    return "\\TType::SET";
  } else if (type->is<t_list>()) {
    return "\\TType::LST";
  }

  throw std::runtime_error("INVALID TYPE IN type_to_enum: " + type->name());
}

THRIFT_REGISTER_GENERATOR(
    hack,
    "HACK",
    "    server:          Generate Hack server stubs.\n"
    "    rest:            Generate Hack REST processors.\n"
    "    json:            Generate functions to parse JSON into thrift "
    "struct.\n"
    "    mangledsvcs      Generate services with namespace mangling.\n"
    "    stricttypes      Use Collection classes everywhere rather than "
    "KeyedContainer.\n"
    "    arraysets        Use legacy arrays for sets rather than objects.\n"
    "                     Either legacy_arrays or hack_collections must be present.\n"
    "    nonullables      Instantiate struct fields within structs, rather "
    "than nullable\n"
    "    structtrait      Add 'use [StructName]Trait;' to generated classes\n"
    "    shapes           Generate Shape definitions for structs\n"
    "    protected_unions Generate protected members for thrift unions\n"
    "    strict_unions    Only allow single set field for thrift unions\n"
    "    legacy_union_json_serialization Preserve existing json serialization \n"
    "                                       for thrift unions\n"
    "    shape_arraykeys  When generating Shape definition for structs:\n"
    "                        replace array<string, TValue> with "
    "array<arraykey, TValue>\n"
    "    shapes_allow_unknown_fields Allow unknown fields and implicit "
    "subtyping for shapes \n"
    "    frommap_construct Generate fromMap_DEPRECATED method.\n"
    "    hack_collections Generate hack collections instead of hack arrays.\n"
    "    const_collections Use ConstCollection objects rather than their "
    "mutable counterparts.\n"
    "    typedef          Generate type aliases for all the types defined\n"
    "    enum_transparenttype Use transparent typing for Hack enums: 'enum "
    "FooBar: int as int'.\n"
    "    server_stream Generate service code for streaming methods'.\n");

} // namespace
} // namespace apache::thrift::compiler
