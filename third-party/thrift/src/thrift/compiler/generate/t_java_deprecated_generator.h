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

#ifndef T_JAVA_GENERATOR_H
#define T_JAVA_GENERATOR_H

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <thrift/compiler/generate/t_concat_generator.h>

namespace apache {
namespace thrift {
namespace compiler {

struct StructGenParams {
  bool is_exception = false;
  bool in_class = false;
  bool is_result = false;
  bool gen_immutable = false;
  bool gen_builder = false;
};

/**
 * Java code generator.
 */
class t_java_deprecated_generator : public t_concat_generator {
 public:
  using t_concat_generator::t_concat_generator;

  /**
   * Init and close methods
   */

  void init_generator() override;
  void close_generator() override;

  void generate_consts(std::vector<t_const*> consts) override;

  /**
   * Program-level generation functions
   */

  void generate_typedef(const t_typedef* ttypedef) override;
  void generate_enum(const t_enum* tenum) override;
  void generate_struct(const t_structured* tstruct) override;
  void generate_union(const t_struct* tunion);
  void generate_xception(const t_structured* txception) override;
  void generate_service(const t_service* tservice) override;
  void generate_default_toString(std::ofstream&, const t_structured*);
  void generate_toString_prettyprint(std::ofstream&);

  virtual void print_const_value(
      std::ostream& out,
      std::string name,
      const t_type* type,
      const t_const_value* value,
      bool in_static,
      bool defval = false);
  virtual std::string render_const_value(
      std::ostream& out,
      std::string name,
      const t_type* type,
      const t_const_value* value);

  /**
   * Service-level generation functions
   */

  void generate_java_struct(const t_structured* tstruct, bool is_exception);

  void generate_java_constructor(
      std::ofstream& out,
      const t_structured* tstruct,
      const std::vector<t_field*>& fields);
  void generate_java_constructor_using_builder(
      std::ofstream& out,
      const t_structured* tstruct,
      const std::vector<t_field*>& fields,
      uint32_t bitset_size,
      bool useDefaultConstructor);
  void generate_java_struct_definition(
      std::ofstream& out, const t_structured* tstruct, StructGenParams params);
  void construct_constant_fields(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_struct_equality(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_struct_compare_to(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_struct_reader(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_validator(std::ofstream& out, const t_structured* tstruct);
  void generate_java_struct_result_writer(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_struct_writer(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_struct_tostring(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_meta_data_map(
      std::ofstream& out, const t_structured* tstruct);
  void generate_field_value_meta_data(std::ofstream& out, const t_type* type);
  std::string get_java_type_string(const t_type* type);
  void generate_reflection_setters(
      std::ostringstream& out,
      const t_type* type,
      std::string field_name,
      std::string cap_name);
  void generate_reflection_getters(
      std::ostringstream& out,
      const t_type* type,
      std::string field_name,
      std::string cap_name);
  void generate_generic_field_getters_setters(
      std::ofstream& out, const t_structured* tstruct);
  void generate_java_bean_boilerplate(
      std::ofstream& out, const t_structured* tstruct, bool gen_immutable);
  std::string get_simple_getter_name(const t_field* field);

  void generate_function_helpers(const t_function* tfunction);
  std::string get_cap_name(std::string name);
  std::string generate_isset_check(const t_field* field);
  std::string generate_isset_check(std::string field);
  std::string generate_setfield_check(const t_field* field);
  std::string generate_setfield_check(std::string field);
  void generate_isset_set(std::ofstream& out, const t_field* field);
  std::string isset_field_id(const t_field* field);

  void generate_service_interface(const t_service* tservice);
  void generate_service_async_interface(const t_service* tservice);
  void generate_service_helpers(const t_service* tservice);
  void generate_service_client(const t_service* tservice);
  void generate_service_async_client(const t_service* tservice);
  void generate_service_server(const t_service* tservice);
  void generate_process_function(
      const t_service* tservice, const t_function* tfunction);

  void generate_java_union(const t_structured* tstruct);
  void generate_union_constructor(
      std::ofstream& out, const t_structured* tstruct);
  void generate_union_getters_and_setters(
      std::ofstream& out, const t_structured* tstruct);
  void generate_union_abstract_methods(
      std::ofstream& out, const t_structured* tstruct);
  void generate_check_type(std::ofstream& out, const t_structured* tstruct);
  void generate_union_reader(std::ofstream& out, const t_structured* tstruct);
  void generate_read_value(std::ofstream& out, const t_structured* tstruct);
  void generate_write_value(std::ofstream& out, const t_structured* tstruct);
  void generate_get_field_desc(std::ofstream& out, const t_structured* tstruct);
  void generate_get_struct_desc(
      std::ofstream& out, const t_structured* tstruct);
  void generate_get_field_name(std::ofstream& out, const t_struct* tstruct);

  void generate_union_comparisons(
      std::ofstream& out, const t_structured* tstruct);
  void generate_union_hashcode(std::ofstream& out, const t_structured* tstruct);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field(
      std::ofstream& out, const t_field* tfield, std::string prefix = "");

  void generate_deserialize_struct(
      std::ofstream& out, const t_struct* tstruct, std::string prefix = "");

  void generate_deserialize_container(
      std::ofstream& out, const t_type* ttype, std::string prefix = "");

  void generate_deserialize_set_element(
      std::ofstream& out, const t_set* tset, std::string prefix = "");

  void generate_deserialize_map_element(
      std::ofstream& out, const t_map* tmap, std::string prefix = "");

  void generate_deserialize_list_element(
      std::ofstream& out, const t_list* tlist, std::string prefix = "");

  void generate_serialize_field(
      std::ofstream& out, const t_field* tfield, std::string prefix = "");

  void generate_serialize_struct(
      std::ofstream& out, const t_struct* tstruct, std::string prefix = "");

  void generate_serialize_container(
      std::ofstream& out, const t_type* ttype, std::string prefix = "");

  void generate_serialize_map_element(
      std::ofstream& out, const t_map* tmap, std::string iter, std::string map);

  void generate_serialize_set_element(
      std::ofstream& out, const t_set* tmap, std::string iter);

  void generate_serialize_list_element(
      std::ofstream& out, const t_list* tlist, std::string iter);

  virtual bool is_comparable(
      const t_type* type, std::vector<const t_type*>* enclosing = nullptr);
  bool struct_has_all_comparable_fields(
      const t_struct* tstruct, std::vector<const t_type*>* enclosing);

  bool type_has_naked_binary(const t_type* type);
  bool struct_has_naked_binary_fields(const t_structured* tstruct);

  virtual bool has_bit_vector(const t_structured* tstruct);

  /**
   * Helper rendering functions
   */

  std::string java_package();
  virtual std::string java_service_imports();
  virtual std::string java_struct_imports();
  std::string java_thrift_imports();
  std::string java_suppress_warnings_enum();
  std::string java_suppress_warnings_consts();
  std::string java_suppress_warnings_union();
  std::string java_suppress_warnings_struct();
  std::string java_suppress_warnings_service();
  virtual boost::optional<std::string> java_struct_parent_class(
      const t_structured* tstruct, StructGenParams params);

  virtual std::string type_name(
      const t_type* ttype,
      bool in_container = false,
      bool in_init = false,
      bool skip_generic = false);
  std::string base_type_name(t_base_type* tbase, bool in_container = false);
  std::string declare_field(const t_field* tfield, bool init = false);
  std::string function_signature(
      const t_function* tfunction, std::string prefix = "");
  std::string function_signature_async(
      const t_function* tfunction,
      std::string result_handler_symbol,
      bool use_base_method = false,
      std::string prefix = "");
  std::string argument_list(const t_struct* tstruct, bool include_types = true);
  std::string async_function_call_arglist(
      const t_function* tfunc,
      std::string result_handler_symbol,
      bool use_base_method = true,
      bool include_types = true);
  std::string async_argument_list(
      const t_function* tfunct,
      const t_struct* tstruct,
      std::string result_handler_symbol,
      bool include_types = false);
  std::string type_to_enum(const t_type* ttype);
  std::string get_enum_class_name(const t_type* type);
  void generate_struct_desc(std::ofstream& out, const t_structured* tstruct);
  void generate_field_descs(std::ofstream& out, const t_structured* tstruct);
  void generate_field_name_constants(
      std::ofstream& out, const t_structured* tstruct);

  virtual const std::string& get_package_dir() { return package_dir_; }

  bool type_can_be_null(const t_type* ttype) {
    ttype = ttype->get_true_type();

    return generate_boxed_primitive || ttype->is_container() ||
        ttype->is_struct() || ttype->is_exception() ||
        ttype->is_string_or_binary() || ttype->is_enum();
  }

  std::string constant_name(std::string name);

 protected:
  // indicate if we can generate the method
  // E.g. Java doesn't support streaming, so all streaming methods are skipped
  bool can_generate_method(const t_function* func) {
    return !func->sink_or_stream() && !func->return_type()->is_service();
  }

  bool is_field_sensitive(const t_field* field) {
    return field->has_annotation("java.sensitive");
  }

  std::string namespace_key_;
  bool generate_field_metadata_ = true;
  bool generate_immutable_structs_ = false;
  bool generate_boxed_primitive = false;
  bool generate_builder = true;

  /**
   * File streams
   */
  std::string package_name_;
  std::ofstream f_service_;
  std::string package_dir_;

  static constexpr size_t MAX_NUM_JAVA_CONSTRUCTOR_PARAMS = 127;

 private:
  void generate_java_doc(std::ofstream& out, const t_field* field);

  void generate_java_doc(std::ofstream& out, const t_named* named_node);

  void generate_java_doc(std::ofstream& out, const t_function* tdoc);

  void generate_java_docstring_comment(
      std::ofstream& out, std::string contents);
};

} // namespace compiler
} // namespace thrift
} // namespace apache

#endif
