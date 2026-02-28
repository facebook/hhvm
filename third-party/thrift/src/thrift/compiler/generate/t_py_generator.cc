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

#include <stdlib.h>
#include <sys/types.h>
#include <stdexcept>

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/detail/system.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/python/util.h>
#include <thrift/compiler/generate/t_concat_generator.h>
#include <thrift/compiler/generate/t_generator.h>

using namespace std;

namespace apache::thrift::compiler {

namespace {

const std::string* get_py_adapter(const t_type* type) {
  if (!type->get_true_type()->is<t_struct>() &&
      !type->get_true_type()->is<t_union>()) {
    return nullptr;
  }
  return t_typedef::get_first_unstructured_annotation_or_null(
      type, {"py.adapter"});
}

void mark_file_executable(const std::filesystem::path& path) {
  namespace fs = std::filesystem;
  fs::permissions(
      path,
      fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
      fs::perm_options::add);
}

string prefix_temporary(const string& name) {
  return "_fbthrift_" + name;
}

bool is_hidden(const t_field& field) {
  return field.has_structured_annotation(kPythonPyDeprecatedHiddenUri);
}

/**
 * Mapping to legacy enum integer values, to preserve Thrift spec
 * compatibility.
 */
int get_thrift_spec_req(t_field_qualifier qualifier) {
  switch (qualifier) {
    case t_field_qualifier::none:
      return 2;
    case t_field_qualifier::required:
      return 0;
    case t_field_qualifier::optional:
      return 1;
    case t_field_qualifier::terse:
      return 3;
  }
}

/**
 * Python code generator.
 */
class t_py_generator : public t_concat_generator {
 public:
  using t_concat_generator::t_concat_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    gen_json_ = options.find("json") != options.end();
    gen_slots_ = options.find("slots") != options.end();
    gen_asyncio_ = options.find("asyncio") != options.end();
    gen_future_ = options.find("future") != options.end();
    gen_utf8strings_ = options.find("utf8strings") != options.end();
    gen_cpp_transport_ = options.find("cpp_transport") != options.end();
    if (gen_cpp_transport_ && gen_asyncio_) {
      throw std::runtime_error(
          "compiler error: can't use cpp transport together with asyncio yet");
    }

    sort_keys_ = options.find("sort_keys") != options.end();

    auto iter = options.find("thrift_port");
    if (iter != options.end()) {
      default_port_ = iter->second;
    } else {
      default_port_ = "9090";
    }

    compare_t_fields_only_ =
        options.find("compare_t_fields_only") != options.end();

    out_dir_base_ = "gen-py";
  }

  /**
   * Init and close methods
   */

  void init_generator() override;
  void close_generator() override;

  /**
   * Program-level generation functions
   */

  void generate_typedef(const t_typedef* ttypedef) override;
  void generate_enum(const t_enum* tenum) override;
  void generate_const(const t_const* tconst) override;
  void generate_struct(const t_structured* tstruct) override;
  void generate_forward_declaration(const t_structured* tstruct) override;
  void generate_xception(const t_structured* txception) override;
  void generate_service(const t_service* tservice) override;

  std::string render_const_value(
      const t_type* type, const t_const_value* value);

  /**
   * Struct generation code
   */

  void generate_py_struct(const t_structured* tstruct, bool is_exception);
  void generate_py_thrift_spec(
      std::ofstream& out, const t_structured* tstruct, bool is_exception);
  void generate_py_annotation_dict(
      std::ofstream& out, const deprecated_annotation_map& fields);
  void generate_py_annotations(std::ofstream& out, const t_structured* tstruct);
  void generate_py_union(std::ofstream& out, const t_structured* tstruct);
  void generate_py_struct_definition(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_exception = false,
      bool is_result = false);
  void generate_py_struct_reader(
      std::ofstream& out, const t_structured* tstruct);
  void generate_py_struct_writer(
      std::ofstream& out, const t_structured* tstruct);
  void generate_py_function_helpers(const t_function* tfunction);
  void generate_py_converter_helpers(
      std::ofstream& out, const t_structured* tstruct);

  /**
   * Service-level generation functions
   */

  void generate_service_helpers(const t_service* tservice);
  void generate_service_interface(const t_service* tservice, bool with_context);
  void generate_service_client(const t_service* tservice);
  void generate_service_remote(const t_service* tservice);
  void generate_service_fuzzer(const t_service* tservice);
  void generate_service_server(const t_service* tservice, bool with_context);
  void generate_process_function(
      const t_service* tservice,
      const t_function* tfunction,
      bool with_context,
      bool future);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field(
      std::ofstream& out,
      const t_field* tfield,
      const std::string& prefix = "",
      bool inclass = false,
      const std::string& actual_type = "");

  void generate_deserialize_struct(
      std::ofstream& out,
      const t_struct* tstruct,
      const std::string& prefix = "");

  void generate_deserialize_container(
      std::ofstream& out, const t_type* ttype, const std::string& prefix = "");

  void generate_deserialize_set_element(
      std::ofstream& out, const t_set* tset, const std::string& prefix = "");

  void generate_deserialize_map_element(
      std::ofstream& out,
      const t_map* tmap,
      const std::string& prefix = "",
      std::string key_actual_type = "",
      std::string value_actual_type = "");

  void generate_deserialize_list_element(
      std::ofstream& out, const t_list* tlist, const std::string& prefix = "");

  void generate_serialize_field(
      std::ofstream& out,
      const t_field* tfield,
      const std::string& prefix = "");

  void generate_serialize_struct(
      std::ofstream& out,
      const t_struct* tstruct,
      const std::string& prefix = "");

  void generate_serialize_container(
      std::ofstream& out, const t_type* ttype, const std::string& prefix = "");

  void generate_serialize_map_element(
      std::ofstream& out,
      const t_map* tmap,
      std::string kiter,
      std::string viter);

  void generate_serialize_set_element(
      std::ofstream& out, const t_set* tmap, std::string iter);

  void generate_serialize_list_element(
      std::ofstream& out, const t_list* tlist, std::string iter);

  void generate_json_enum(
      std::ofstream& out,
      const t_enum* tenum,
      const string& prefix_thrift,
      const string& prefix_json);
  void generate_json_struct(
      std::ofstream& out,
      const t_struct* tstruct,
      const string& prefix_thrift,
      const string& prefix_json);
  void generate_json_field(
      std::ofstream& out,
      const t_field* tfield,
      const string& prefix_thrift = "",
      const string& suffix_thrift = "",
      const string& prefix_json = "",
      bool generate_assignment = true);
  void generate_json_container(
      std::ofstream& out,
      const t_type* ttype,
      const string& prefix_thrift = "",
      const string& prefix_json = "");

  void generate_json_collection_element(
      ofstream& out,
      const t_type* type,
      const string& collection,
      const string& elem,
      const string& action_prefix,
      const string& action_suffix,
      const string& prefix_json);

  void generate_json_map_key(
      ofstream& out,
      const t_type* type,
      const string& parsed_key,
      const string& string_key);

  void generate_json_reader(std::ofstream& out, const t_structured* tstruct);

  void generate_fastproto_read(std::ofstream& out, const t_structured* tstruct);
  void generate_fastproto_write(
      std::ofstream& out, const t_structured* tstruct);

  /**
   * Helper rendering functions
   */

  std::string py_autogen_comment();
  std::string py_par_warning(const string& service_tool_name);
  std::string py_imports();
  std::string rename_reserved_keywords(const std::string& value);
  std::string render_includes();
  std::string render_fastproto_includes();
  std::string declare_argument(
      const t_structured* tstruct, const t_field* tfield);
  std::string render_field_default_value(const t_field* tfield);
  std::string type_name(const t_type* ttype);
  std::string service_name(const t_service* service) {
    return get_real_py_module(service->program()) + "." +
        rename_reserved_keywords(service->name());
  }
  std::string function_signature(
      const t_function* tfunction, const std::string& prefix = "");
  std::string function_signature_if(
      const t_function* tfunction,
      bool with_context,
      const std::string& prefix = "");
  std::string argument_list(const t_paramlist& tparamlist);
  std::string type_to_enum(const t_type* ttype);
  std::string type_to_spec_args(const t_type* ttype);
  std::string get_real_py_module(const t_program* program);

  std::string render_string(const std::string& value);
  std::string render_ttype_declarations(const char* delimiter);

  std::string get_priority(
      const t_named* obj, const std::string& def = "NORMAL");
  const std::vector<const t_function*>& get_functions(
      const t_service* tservice);

 private:
  /**
   * True iff we should generate a function parse json to thrift object.
   */
  bool gen_json_;

  /**
   * True iff we should generate __slots__ for thrift structs.
   */
  bool gen_slots_;

  /**
   * True iff we should generate code for asyncio server in Python 3.
   */
  bool gen_asyncio_;

  /**
   * True iff we should generate services supporting concurrent.futures.
   */
  bool gen_future_;

  /**
   * True iff strings should be encoded using utf-8.
   */
  bool gen_utf8strings_;

  /**
   * True if we should generate new clients using C++ transport.
   */
  bool gen_cpp_transport_;

  /**
   * True iff we serialize maps sorted by key and sets by value
   */
  bool sort_keys_;

  /**
   * True iff we compare thrift classes using their spec fields only
   */
  bool compare_t_fields_only_;

  /**
   * Default port to use.
   */
  std::string default_port_;

  /**
   * File streams
   */

  std::ofstream f_types_;
  std::ofstream f_consts_;
  std::ofstream f_service_;

  std::filesystem::path package_dir_;

  std::map<std::string, const std::vector<const t_function*>> func_map_;

  void generate_json_reader_fn_signature(ofstream& out);
  static int32_t get_thrift_spec_key(const t_structured*, const t_field*);

  void generate_python_docstring(
      std::ofstream& out, const t_structured* tstruct);

  void generate_python_docstring(
      std::ofstream& out, const t_function* tfunction);

  void generate_python_docstring(
      std::ofstream& out,
      const t_named* named_node,
      const t_structured* tstruct,
      const char* subheader);

  void generate_python_docstring(std::ofstream& out, const t_named* named_node);
};

std::string t_py_generator::get_real_py_module(const t_program* program) {
  if (gen_asyncio_) {
    std::string asyncio_module = program->get_namespace("py.asyncio");
    if (!asyncio_module.empty()) {
      return asyncio_module;
    }
  }

  std::string real_module = program->get_namespace("py");
  if (real_module.empty()) {
    return program->name();
  }
  return real_module;
}

void t_py_generator::generate_json_field(
    ofstream& out,
    const t_field* tfield,
    const string& prefix_thrift,
    const string& suffix_thrift,
    const string& prefix_json,
    bool generate_assignment) {
  const t_type* type = tfield->type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT READ JSON FIELD WITH void TYPE: " + prefix_thrift +
        tfield->name());
  }

  string name =
      prefix_thrift + rename_reserved_keywords(tfield->name()) + suffix_thrift;

  if (type->is<t_structured>()) {
    generate_json_struct(out, (t_struct*)type, name, prefix_json);
  } else if (const t_container* container = type->try_as<t_container>()) {
    generate_json_container(out, container, name, prefix_json);
  } else if (const t_enum* enum_ = type->try_as<t_enum>()) {
    generate_json_enum(out, enum_, name, prefix_json);
  } else if (const auto* primitive = type->try_as<t_primitive_type>()) {
    string conversion_function;
    string number_limit;
    string number_negative_limit;
    switch (primitive->primitive_type()) {
      case t_primitive_type::type::t_void:
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
      case t_primitive_type::type::t_bool:
        break;
      case t_primitive_type::type::t_byte:
        number_limit = "0x7f";
        number_negative_limit = "-0x80";
        break;
      case t_primitive_type::type::t_i16:
        number_limit = "0x7fff";
        number_negative_limit = "-0x8000";
        break;
      case t_primitive_type::type::t_i32:
        number_limit = "0x7fffffff";
        number_negative_limit = "-0x80000000";
        break;
      case t_primitive_type::type::t_i64:
        conversion_function = "long";
        break;
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        conversion_function = "float";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no python reader for base type " +
            primitive->name() + name);
    }

    string value = prefix_json;
    if (!conversion_function.empty()) {
      value = conversion_function + "(" + value + ")";
    }

    if (generate_assignment) {
      indent(out) << name << " = " << value << endl;
    }

    if (!number_limit.empty()) {
      indent(out) << "if " << name << " > " << number_limit << " or " << name
                  << " < " << number_negative_limit << ":" << endl;
      indent_up();
      indent(out) << "raise TProtocolException(TProtocolException.INVALID_DATA,"
                  << " 'number exceeds limit in field')" << endl;
      indent_down();
    }
  } else {
    throw std::runtime_error("Compiler did not generate_json_field reader");
  }
}

void t_py_generator::generate_json_struct(
    ofstream& out,
    const t_struct* tstruct,
    const string& prefix_thrift,
    const string& prefix_json) {
  indent(out) << prefix_thrift << " = " << type_name(tstruct) << "()" << endl;
  indent(out) << prefix_thrift << ".readFromJson(" << prefix_json
              << ", is_text=False, **kwargs)" << endl;
}

void t_py_generator::generate_json_enum(
    ofstream& out,
    const t_enum* tenum,
    const string& prefix_thrift,
    const string& prefix_json) {
  indent(out) << prefix_thrift << " = " << prefix_json << endl;
  indent(out) << "if not " << prefix_thrift << " in " << type_name(tenum)
              << "._VALUES_TO_NAMES:" << endl;
  indent_up();
  indent(out)
      << "msg = 'Integer value ''%s'' is not a recognized value of enum type "
      << type_name(tenum) << "' % " << prefix_thrift << endl;
  indent(out) << "if relax_enum_validation:" << endl;
  indent(out) << "    warnings.warn(msg)" << endl;
  indent(out) << "else:" << endl;
  indent(out) << "    raise TProtocolException("
              << "TProtocolException.INVALID_DATA, msg)" << endl;
  indent_down();
  indent(out) << "if wrap_enum_constants:" << endl;
  indent_up();
  indent(out) << prefix_thrift << " = ThriftEnumWrapper(" << type_name(tenum)
              << ", " << prefix_thrift << ")" << endl;
  indent_down();
}

void t_py_generator::generate_json_container(
    ofstream& out,
    const t_type* ttype,
    const string& prefix_thrift,
    const string& prefix_json) {
  if (const t_list* list = ttype->try_as<t_list>()) {
    string e = tmp("_tmp_e");
    indent(out) << prefix_thrift << " = []" << endl;

    indent(out) << "for " << e << " in " << prefix_json << ":" << endl;
    indent_up();
    generate_json_collection_element(
        out,
        list->elem_type().get_type(),
        prefix_thrift,
        e,
        ".append(",
        ")",
        prefix_json);
    indent_down();
  } else if (const t_set* set = ttype->try_as<t_set>()) {
    string e = tmp("_tmp_e");
    indent(out) << prefix_thrift << " = set_cls()" << endl;

    indent(out) << "for " << e << " in " << prefix_json << ":" << endl;
    indent_up();
    generate_json_collection_element(
        out,
        set->elem_type().get_type(),
        prefix_thrift,
        e,
        ".add(",
        ")",
        prefix_json);
    indent_down();
  } else if (const t_map* map = ttype->try_as<t_map>()) {
    string k = tmp("_tmp_k");
    string v = tmp("_tmp_v");
    string kp = tmp("_tmp_kp");
    indent(out) << prefix_thrift << " = dict_cls()" << endl;

    indent(out) << "for " << k << ", " << v << " in " << prefix_json
                << ".items():" << endl;
    indent_up();

    generate_json_map_key(out, &map->key_type().deref(), kp, k);

    generate_json_collection_element(
        out,
        &map->val_type().deref(),
        prefix_thrift,
        v,
        "[" + kp + "] = ",
        "",
        prefix_json + "[" + kp + "]");
    indent_down();
  }
}

void t_py_generator::generate_json_collection_element(
    ofstream& out,
    const t_type* type,
    const string& collection,
    const string& elem,
    const string& action_prefix,
    const string& action_suffix,
    const string& prefix_json) {
  string to_act_on = elem;
  string to_parse = prefix_json;
  type = type->get_true_type();

  if (const auto* primitive = type->try_as<t_primitive_type>()) {
    switch (primitive->primitive_type()) {
      // Explicitly cast into float because there is an asymetry
      // between serializing and deserializing NaN.
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        to_act_on = "float(" + to_act_on + ")";
        break;
      default:
        break;
    }
  } else if (type->is<t_enum>()) {
    to_parse = elem;
    to_act_on = tmp("_enum");
  } else if (type->is<t_list>()) {
    to_parse = elem;
    to_act_on = tmp("_list");
  } else if (type->is<t_map>()) {
    to_parse = elem;
    to_act_on = tmp("_map");
  } else if (type->is<t_set>()) {
    to_parse = elem;
    to_act_on = tmp("_set");
  } else if (type->is<t_struct>() || type->is<t_union>()) {
    to_parse = elem;
    to_act_on = tmp("_struct");
  }

  t_field felem(*type, to_act_on);
  generate_json_field(out, &felem, "", "", to_parse, false);
  indent(out) << collection << action_prefix << to_act_on << action_suffix
              << endl;
}

void t_py_generator::generate_json_map_key(
    ofstream& out,
    const t_type* type,
    const string& parsed_key,
    const string& raw_key) {
  type = type->get_true_type();
  if (type->is<t_enum>()) {
    indent(out) << parsed_key << " = int(" << raw_key << ")" << endl;
    indent(out) << "if wrap_enum_constants:" << endl;
    indent_up();
    indent(out) << parsed_key << " = ThriftEnumWrapper(" << type_name(type)
                << ", " << parsed_key << ")" << endl;
    indent_down();
  } else if (const auto* primitive = type->try_as<t_primitive_type>()) {
    string conversion_function;
    string number_limit;
    string number_negative_limit;
    bool generate_assignment = true;
    switch (primitive->primitive_type()) {
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        break;
      case t_primitive_type::type::t_bool:
        indent(out) << "if " << raw_key << " == 'true':" << endl;
        indent_up();
        indent(out) << parsed_key << " = True" << endl;
        indent_down();
        indent(out) << "elif " << raw_key << " == 'false':" << endl;
        indent_up();
        indent(out) << parsed_key << " = False" << endl;
        indent_down();
        indent(out) << "else:" << endl;
        indent_up();
        indent(out) << "raise TProtocolException(TProtocolException."
                    << "INVALID_DATA, 'invalid boolean value' + " << raw_key
                    << ")" << endl;
        indent_down();
        generate_assignment = false;
        break;
      case t_primitive_type::type::t_byte:
        conversion_function = "int";
        number_limit = "0x7f";
        number_negative_limit = "-0x80";
        break;
      case t_primitive_type::type::t_i16:
        conversion_function = "int";
        number_limit = "0x7fff";
        number_negative_limit = "-0x8000";
        break;
      case t_primitive_type::type::t_i32:
        conversion_function = "int";
        number_limit = "0x7fffffff";
        number_negative_limit = "-0x80000000";
        break;
      case t_primitive_type::type::t_i64:
        conversion_function = "long";
        break;
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        conversion_function = "float";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no C++ reader for base type " + primitive->name());
    }

    string value = raw_key;
    if (!conversion_function.empty()) {
      value = conversion_function + "(" + value + ")";
    }

    if (generate_assignment) {
      indent(out) << parsed_key << " = " << value << endl;
    }

    if (!number_limit.empty()) {
      indent(out) << "if " << parsed_key << " > " << number_limit << " or "
                  << parsed_key << " < " << number_negative_limit << ":"
                  << endl;
      indent_up();
      indent(out) << "raise TProtocolException(TProtocolException.INVALID_DATA,"
                  << " 'number exceeds the limit in key ' + " << raw_key << ")"
                  << endl;
      indent_down();
    }
  } else {
    throw string("compiler error: invalid key type");
  }
}

void t_py_generator::generate_json_reader_fn_signature(ofstream& out) {
  indent(out) << "def readFromJson(self, json, is_text=True, **kwargs):"
              << endl;
  indent_up();
  indent(out) << "kwargs_copy = dict(kwargs)" << endl;
  indent(out)
      << "wrap_enum_constants = kwargs_copy.pop('wrap_enum_constants', False)"
      << endl;
  indent(out)
      << "relax_enum_validation = "
         "bool(kwargs_copy.pop('relax_enum_validation', not wrap_enum_constants))"
      << endl;
  indent(out) << "set_cls = kwargs_copy.pop('custom_set_cls', set)" << endl;
  indent(out) << "dict_cls = kwargs_copy.pop('custom_dict_cls', dict)" << endl;
  indent(out) << "if wrap_enum_constants and relax_enum_validation:" << endl;
  indent(out) << "    raise ValueError(" << endl;
  indent(out)
      << "        'wrap_enum_constants cannot be used together with relax_enum_validation'"
      << endl;
  indent(out) << "    )" << endl;
  indent(out) << "if kwargs_copy:" << endl;
  indent(out) << "    extra_kwargs = ', '.join(kwargs_copy.keys())" << endl;
  indent(out) << "    raise ValueError(" << endl;
  indent(out) << "        'Unexpected keyword arguments: ' + extra_kwargs"
              << endl;
  indent(out) << "    )" << endl;
}

void t_py_generator::generate_json_reader(
    ofstream& out, const t_structured* tstruct) {
  if (!gen_json_) {
    return;
  }

  generate_json_reader_fn_signature(out);
  indent(out) << "json_obj = json" << endl;
  indent(out) << "if is_text:" << endl;
  indent_up();
  indent(out) << "json_obj = loads(json)" << endl;
  indent_down();

  for (const t_field& f_iter : tstruct->fields()) {
    if (is_hidden(f_iter)) {
      continue;
    }
    string field = f_iter.name();
    indent(out) << "if '" << field << "' in json_obj " << "and json_obj['"
                << field << "'] is not None:" << endl;
    indent_up();
    generate_json_field(
        out, &f_iter, "self.", "", "json_obj['" + f_iter.name() + "']");

    indent_down();
  }
  indent_down();
  out << endl;
}

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_py_generator::init_generator() {
  // Make output directory structure
  string module = get_real_py_module(program_);
  package_dir_ =
      add_gen_dir() ? detail::format_abs_path(get_out_dir()) : get_out_path();
  std::filesystem::create_directory(package_dir_);
  while (true) {
    std::filesystem::create_directory(package_dir_);
    std::ofstream init_py(package_dir_ / "__init__.py");
    init_py << py_autogen_comment();
    init_py.close();
    if (module.empty()) {
      break;
    }
    string::size_type pos = module.find('.');
    if (pos == string::npos) {
      package_dir_ /= module;
      module.clear();
    } else {
      package_dir_ /= module.substr(0, pos);
      module.erase(0, pos + 1);
    }
  }

  // Make output file
  auto f_types_path = package_dir_ / "ttypes.py";
  f_types_.open(f_types_path);
  record_genfile(f_types_path);

  auto f_consts_path = package_dir_ / "constants.py";
  f_consts_.open(f_consts_path);
  record_genfile(f_consts_path);

  auto f_init_path = package_dir_ / "__init__.py";
  std::ofstream f_init;
  f_init.open(f_init_path);
  record_genfile(f_init_path);
  f_init << py_autogen_comment() << "__all__ = ['ttypes', 'constants'";
  for (const auto* tservice : program_->services()) {
    f_init << ", '" << tservice->name() << "'";
  }
  f_init << "]" << endl;
  f_init.close();

  // Print header
  f_types_ << py_autogen_comment() << endl
           << py_imports() << endl
           << render_includes() << endl
           << render_fastproto_includes() << "all_structs = []" << endl
           << "UTF8STRINGS = bool(" << gen_utf8strings_ << ") or "
           << "sys.version_info.major >= 3" << endl
           << endl;

  // Define __all__ for ttypes
  f_types_ << "__all__ = [" << render_ttype_declarations("'") << "]" << endl
           << endl;

  f_consts_ << py_autogen_comment() << endl
            << py_imports() << endl
            << render_includes() << endl
            << "from .ttypes import " << render_ttype_declarations("") << endl
            << endl;
}

string t_py_generator::render_ttype_declarations(const char* delimiter) {
  std::ostringstream out;
  out << delimiter << "UTF8STRINGS" << delimiter;
  for (const auto& en : program_->enums()) {
    out << ", " << delimiter << rename_reserved_keywords(en->name())
        << delimiter;
  }
  for (const t_structured* object : program_->structured_definitions()) {
    out << ", " << delimiter << rename_reserved_keywords(object->name())
        << delimiter;
  }
  for (const auto& td : program_->typedefs()) {
    out << ", " << delimiter << rename_reserved_keywords(td->name())
        << delimiter;
  }
  return out.str();
}

/**
 * Ensures the string is not a reserved Python keyword.
 */
string t_py_generator::rename_reserved_keywords(const string& value) {
  const auto& reserved_keywords = get_python_reserved_names();
  if (reserved_keywords.find(value) != reserved_keywords.end()) {
    return value + "_PY_RESERVED_KEYWORD";
  } else {
    return value;
  }
}

/**
 * Renders all the imports necessary for including another Thrift program
 */
string t_py_generator::render_includes() {
  const vector<t_program*>& includes = program_->get_includes_for_codegen();
  string result;
  for (size_t i = 0; i < includes.size(); ++i) {
    result += "import " + get_real_py_module(includes[i]) + ".ttypes\n";
  }
  if (includes.size() > 0) {
    result += "\n";
  }

  set<string> modules;
  for (const auto* strct : program_->structs_and_unions()) {
    for (const auto* t : collect_types(strct)) {
      if (const auto* adapter = get_py_adapter(t)) {
        modules.emplace(adapter->substr(0, adapter->find_last_of('.')));
      }
    }
  }
  for (const auto* type : program_->typedefs()) {
    if (const auto* adapter = get_py_adapter(type)) {
      modules.emplace(adapter->substr(0, adapter->find_last_of('.')));
    }
  }
  for (const auto& module : modules) {
    result += "import " + module + "\n";
  }
  if (modules.size() > 0) {
    result += "\n";
  }

  return result;
}

/**
 * Renders all the imports necessary to use fastproto.
 */
string t_py_generator::render_fastproto_includes() {
  return R"(import pprint
import warnings
from thrift import Thrift
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TCompactProtocol
from thrift.protocol import THeaderProtocol
fastproto = None
try:
  from thrift.protocol import fastproto
except ImportError:
  pass
)"
         /*
    Given a sparse thrift_spec generate a full thrift_spec as expected by
    fastproto. The old form is a tuple where every position is the same as the
    thrift field id. The new form is just a tuple of the used field ids without
    all the None padding, but its cheaper bytecode wise. There is a bug in
    python 3.10 that causes large tuples to use more memory and generate larger
    .pyc than <=3.9. See: https://github.com/python/cpython/issues/109036
          */
         R"(
def __EXPAND_THRIFT_SPEC(spec):
    next_id = 0
    for item in spec:
        item_id = item[0]
        if next_id >= 0 and item_id < 0:
            next_id = item_id
        if item_id != next_id:
            for _ in range(next_id, item_id):
                yield None
        yield item
        next_id = item_id + 1

class ThriftEnumWrapper(int):
  def __new__(cls, enum_class, value):
    return super().__new__(cls, value)
  def __init__(self, enum_class, value):    self.enum_class = enum_class
  def __repr__(self):
    return self.enum_class.__name__ + '.' + self.enum_class._VALUES_TO_NAMES[self]

)";
}

/**
 * Autogen'd comment
 */
string t_py_generator::py_autogen_comment() {
  return std::string("#\n") + "# Autogenerated by Thrift\n" + "#\n" +
      "# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE "
      "DOING\n" +
      "#  @"
      "generated\n" +
      "#\n";
}

/**
 * Print out warning message in the case a *.py is running instead of *.par
 */
string t_py_generator::py_par_warning(const string& service_tool_name) {
  return "if (not sys.argv[0].endswith(\"par\") and\n"
         "    not sys.argv[0].endswith(\"xar\") and\n"
         "    os.getenv('PAR_UNPACK_TMP') == None):\n"
         "\n"
         "    f = open(sys.argv[0], \"r\")\n"
         "\n"
         "    f.readline() # This will be #!/bin/bash\n"
         "    line = f.readline()\n"
         "    f.close()\n"
         "\n"
         "    # The par generator tool always has '# This par was made' as "
         "the\n"
         "    # second line. See fbcode/tools/make_par/make_par.py\n"
         "    if (not line.startswith('# This par was made')):\n"
         "        print(\"\"\"WARNING\n"
         "        You are trying to run *-" +
      service_tool_name +
      ".py which is\n"
      "        incorrect as the paths are not set up correctly.\n"
      "        Instead, you should generate your thrift file with\n"
      "        thrift_library and then run the resulting\n"
      "        *-" +
      service_tool_name +
      ".par.\n"
      "        For more information, please read\n"
      "        http://fburl.com/python-remotes\"\"\")\n"
      "        exit()\n";
}

/**
 * Prints standard thrift imports
 */
string t_py_generator::py_imports() {
  string imports = "from __future__ import absolute_import\n";

  imports += "import sys\n";
  imports += "from thrift.util.Recursive import fix_spec\n";
  imports += "from thrift.Thrift import TType, TMessageType, TPriority";
  imports += ", TRequestContext, TProcessorEventHandler, TServerInterface";
  imports +=
      ", TProcessor, TException, TApplicationException, UnimplementedTypedef\n";
  imports += "from thrift.protocol.TProtocol import TProtocolException\n";
  if (compare_t_fields_only_) {
    imports += "from thrift.util import parse_struct_spec\n\n";
  } else {
    imports += "\n";
  }

  if (gen_json_) {
    imports += "from json import loads\n";
    imports += "import sys\n";
    imports += "if sys.version_info[0] >= 3:\n";
    imports += "  long = int\n";
  }

  return imports;
}

/**
 * Closes the type files
 */
void t_py_generator::close_generator() {
  // Close types file
  f_types_ << "fix_spec(all_structs)" << endl << "del all_structs" << endl;
  f_types_.close();
  f_consts_.close();
}

/**
 * Print typedefs of typedefs, enums, exceptions, and structs as "Name = Type".
 * Unsupported types get `UnimplementedTypedef()` as the l-value.
 */
void t_py_generator::generate_typedef(const t_typedef* ttypedef) {
  const auto varname = rename_reserved_keywords(ttypedef->name());
  const auto* type = &ttypedef->type().deref();
  // Typedefs of user-defined types are useful as aliases.  On the other
  // hand, base types are implicit, so it is not as helpful to support
  // creating aliases to their Python analogs.  That said, if you need it,
  // add an `else if` below.
  if (const auto* adapter = get_py_adapter(type)) {
    f_types_ << varname << " = " << *adapter << ".Type" << endl;
  } else if (
      type->is<t_typedef>() || type->is<t_enum>() || type->is<t_structured>()) {
    f_types_ << varname << " = " << type_name(type) << endl;
  } else {
    // Emit dummy symbols for other type names, because otherwise a typedef
    // to a typedef to a base type would result in non-importable code.
    //
    // The dummy is a proper object instance rather than None because:
    //  - Some questionable files, e.g. PythonReservedKeywords.thrift
    //    shadow a struct with a typedef, and the parser accepts it.
    //  - This generator splits struct-like object creation into two passes,
    //    forward_declaration (reality: class definition), which happens
    //    before typedefs are instantiated, and definition (reality:
    //    mutation of the class object) , which happens after.  If the
    //    typedef shadowed a struct, and its value were None, all of the
    //    mutations would fail at import time with mysterious messages.  By
    //    substituting an UnimplementedTypedef(), we instead let this blow
    //    up at runtime, as the author of the shadowing file richly deseres.
    f_types_ << varname << " = UnimplementedTypedef()" << endl;
  }
}

/**
 * Generates code for an enumerated type. Done using a class to scope
 * the values.
 *
 * @param tenum The enumeration
 */
void t_py_generator::generate_enum(const t_enum* tenum) {
  std::ostringstream names_list, values_list;

  string name = rename_reserved_keywords(tenum->name());

  f_types_ << "class " << name << ":" << endl;

  indent_up();
  generate_python_docstring(f_types_, tenum);

  // Make Pylint not blow up on access of enum members
  f_types_ << indent()
           << "def __getattr__(self, name): raise AttributeError(name)" << endl;

  names_list << "(" << endl;
  values_list << "(" << endl;

  node_list_view<const t_enum_value> constants = tenum->values();
  node_list_view<const t_enum_value>::iterator c_iter;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int32_t value = (*c_iter).get_value();

    // Build up lists of values and names to avoid large dict literals
    names_list << indent(2) << "\"" << (*c_iter).name() << "\"," << endl;
    values_list << indent(2) << value << "," << endl;
  }
  names_list << ")";
  values_list << indent() << ")";

  f_types_ << endl;
  f_types_ << indent() << "_NAMES_TO_VALUES = dict(zip(" << names_list.str()
           << "," << endl
           << values_list.str() << "))" << endl;
  f_types_ << indent() << "_VALUES_TO_NAMES = {}" << endl;

  indent_down();
  f_types_ << endl;
  f_types_ << "for k, v in " << name << "._NAMES_TO_VALUES.items():" << endl
           << indent(2) << "setattr(" << name << ", k, v)" << endl
           << indent(2) << name << "._VALUES_TO_NAMES[v] = k" << endl
           << endl;
}

/**
 * Generate a constant value
 */
void t_py_generator::generate_const(const t_const* tconst) {
  if (tconst->generated()) {
    return;
  }
  const t_type* type = tconst->type();
  string name = rename_reserved_keywords(tconst->name());
  const t_const_value* value = tconst->value();

  indent(f_consts_) << name << " = " << render_const_value(type, value);
  f_consts_ << endl << endl;
}

string t_py_generator::render_string(const string& value) {
  std::string escaped;
  escaped.reserve(value.size());
  for (unsigned char c : value) {
    if (c == '\n') {
      escaped.push_back(c);
    } else if (c < 0x20 || c >= 0xf8) {
      escaped.append(fmt::format("\\x{:02x}", c));
    } else if (c == '"') {
      escaped.append("\\\"");
    } else if (c == '\\') {
      escaped.append("\\\\");
    } else {
      escaped.push_back(c);
    }
  }

  std::ostringstream out;
  // If string contains multiple lines, then wrap it in triple quotes """
  std::string wrap(escaped.find('\n') == std::string::npos ? "\"" : R"(""")");
  out << wrap << escaped << wrap;
  return out.str();
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
string t_py_generator::render_const_value(
    const t_type* type, const t_const_value* value) {
  type = type->get_true_type();
  std::ostringstream out;

  if (const auto* primitive = type->try_as<t_primitive_type>()) {
    switch (primitive->primitive_type()) {
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        out << render_string(value->get_string());
        break;
      case t_primitive_type::type::t_bool:
        out << (value->get_integer() > 0 ? "True" : "False");
        break;
      case t_primitive_type::type::t_byte:
      case t_primitive_type::type::t_i16:
      case t_primitive_type::type::t_i32:
      case t_primitive_type::type::t_i64:
        out << value->get_integer();
        break;
      case t_primitive_type::type::t_double:
      case t_primitive_type::type::t_float:
        out << std::showpoint;
        if (value->kind() == t_const_value::CV_INTEGER) {
          out << value->get_integer();
        } else {
          out << value->get_double();
        }
        break;
      default:
        throw std::runtime_error(
            "compiler error: no const of base type " + primitive->name());
    }
  } else if (type->is<t_enum>()) {
    indent(out) << value->get_integer();
  } else if (const t_structured* structured = type->try_as<t_structured>()) {
    out << rename_reserved_keywords(type_name(type)) << "(**{" << endl;
    indent_up();
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      const t_type* field_type = nullptr;
      for (const t_field& f_iter : structured->fields()) {
        if (f_iter.name() == v_iter->first->get_string()) {
          field_type = f_iter.type().get_type();
        }
      }
      if (field_type == nullptr) {
        throw std::runtime_error(
            "type error: " + type->name() + " has no field " +
            v_iter->first->get_string());
      }
      out << indent();
      out << render_const_value(&t_primitive_type::t_string(), v_iter->first);
      out << " : ";
      out << render_const_value(field_type, v_iter->second);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "})";
  } else if (const t_map* map = type->try_as<t_map>()) {
    const t_type* ktype = &map->key_type().deref();
    const t_type* vtype = &map->val_type().deref();
    out << "{" << endl;
    indent_up();
    if (value->kind() == t_const_value::CV_MAP) {
      const vector<pair<t_const_value*, t_const_value*>>& val =
          value->get_map();
      vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
      for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
        out << indent();
        out << render_const_value(ktype, v_iter->first);
        out << " : ";
        out << render_const_value(vtype, v_iter->second);
        out << "," << endl;
      }
    }
    indent_down();
    indent(out) << "}";
  } else if (type->is<t_list>() || type->is<t_set>()) {
    const t_type* etype;
    if (const t_list* list = type->try_as<t_list>()) {
      etype = list->elem_type().get_type();
    } else {
      etype = ((t_set*)type)->elem_type().get_type();
    }
    if (type->is<t_set>()) {
      out << "set(";
    }
    out << "[" << endl;
    indent_up();
    for (const t_const_value* elem : value->get_list_or_empty_map()) {
      out << indent();
      out << render_const_value(etype, elem);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "]";
    if (type->is<t_set>()) {
      out << ")";
    }
  } else {
    throw std::runtime_error(
        "CANNOT GENERATE CONSTANT FOR TYPE: " + type->name());
  }

  return out.str();
}

void t_py_generator::generate_forward_declaration(const t_structured* tstruct) {
  if (!tstruct->is<t_union>()) {
    generate_py_struct(tstruct, tstruct->is<t_exception>());
  } else {
    generate_py_union(f_types_, tstruct);
  }
}

/**
 * Generates a python struct
 */
void t_py_generator::generate_struct(const t_structured* tstruct) {
  generate_py_thrift_spec(f_types_, tstruct, false /* is_exception */);
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_py_generator::generate_xception(const t_structured* txception) {
  generate_py_thrift_spec(f_types_, txception, true /* is_exception */);
}

/**
 * Generates a python struct
 */
void t_py_generator::generate_py_struct(
    const t_structured* tstruct, bool is_exception) {
  generate_py_struct_definition(f_types_, tstruct, is_exception);
  generate_py_converter_helpers(f_types_, tstruct);
}

/**
 * Generates a python union definition. We just keep a variable `value`
 * which holds the current value and to know type we use instanceof.
 */
void t_py_generator::generate_py_union(
    ofstream& out, const t_structured* tstruct) {
  node_list_view<const t_field> members = tstruct->fields();
  const vector<const t_field*>& sorted_members = tstruct->fields_id_order();

  out << "class " << rename_reserved_keywords(tstruct->name())
      << "(object):" << endl;

  indent_up();
  generate_python_docstring(out, tstruct);

  out << endl;

  indent(out) << "thrift_spec = None" << endl;
  if (members.size() != 0) {
    indent(out) << "__init__ = None" << endl << endl;
  }

  // Generate some class level identifiers (similar to enum)
  indent(out) << "__EMPTY__ = 0" << endl;
  for (auto& member : sorted_members) {
    if (is_hidden(*member)) {
      continue;
    }
    indent(out) << uppercase(member->name()) << " = " << member->id() << endl;
  }
  indent(out) << endl;

  // Generate `isUnion` method
  indent(out) << "@staticmethod" << endl;
  indent(out) << "def isUnion():" << endl;
  indent(out) << "  return True" << endl << endl;

  // Generate `get_` methods
  for (auto& member : members) {
    if (is_hidden(member)) {
      continue;
    }
    indent(out) << "def get_" << member.name() << "(self):" << endl;
    indent(out) << "  assert self.field == " << member.id() << endl;
    indent(out) << "  return self.value" << endl << endl;
  }

  // Generate `set_` methods
  for (auto& member : members) {
    if (is_hidden(member)) {
      continue;
    }
    indent(out) << "def set_" << member.name() << "(self, value):" << endl;
    indent(out) << "  self.field = " << member.id() << endl;
    indent(out) << "  self.value = value" << endl << endl;
  }

  // Method to get the stored type
  indent(out) << "def getType(self):" << endl;
  indent(out) << "  return self.field" << endl << endl;

  // According to Python doc, __repr__() "should" return a valid expression
  // such that `object == eval(repr(object))` is true.
  out << indent() << "def __repr__(self):" << endl
      << indent() << "  value = pprint.pformat(self.value)" << endl
      << indent() << "  member = ''" << endl;
  for (auto& member : sorted_members) {
    if (is_hidden(*member)) {
      continue;
    }
    auto key = rename_reserved_keywords(member->name());
    out << indent() << "  if self.field == " << member->id() << ":" << endl
        << indent() << "    padding = ' ' * " << key.size() + 1 << endl
        << indent() << "    value = padding.join(value.splitlines(True))"
        << endl
        << indent() << "    member = '\\n    %s=%s' % ('" << key << "', value)"
        << endl;
  }
  // This will generate
  //   UnionClass()  or
  //   UnionClass(
  //       key=value)
  out << indent() << "  return \"%s(%s)\" % (self.__class__.__name__, member)"
      << endl
      << endl;

  // Generate `read` method
  indent(out) << "def read(self, iprot):" << endl;
  indent_up();

  indent(out) << "self.field = 0" << endl;
  indent(out) << "self.value = None" << endl;

  generate_fastproto_read(out, tstruct);

  indent(out) << "iprot.readStructBegin()" << endl;
  indent(out) << "while True:" << endl;
  indent_up();
  indent(out) << "(fname, ftype, fid) = iprot.readFieldBegin()" << endl;
  indent(out) << "if ftype == TType.STOP:" << endl;
  indent_up();
  indent(out) << "break" << endl << endl;
  indent_down();

  bool first = true;
  for (auto& member : sorted_members) {
    if (is_hidden(*member)) {
      continue;
    }
    auto t = type_to_enum(member->type().get_type());
    const auto& n = member->name();
    auto k = member->id();
    indent(out) << (first ? "" : "el") << "if fid == " << k << ":" << endl;
    indent_up();
    indent(out) << "if ftype == " << t << ":" << endl;
    indent_up();
    generate_deserialize_field(out, member, prefix_temporary(""));
    indent(out) << "assert self.field == 0 and self.value is None" << endl;
    indent(out) << "self.set_" << n << "("
                << prefix_temporary(rename_reserved_keywords(n)) << ")" << endl;
    indent_down();
    indent(out) << "else:" << endl;
    indent(out) << "  iprot.skip(ftype)" << endl;
    indent_down();

    first = false;
  }

  indent(out) << "else:" << endl;
  indent(out) << "  iprot.skip(ftype)" << endl;
  indent(out) << "iprot.readFieldEnd()" << endl;
  indent_down();

  indent(out) << "iprot.readStructEnd()" << endl << endl;
  indent_down();

  // Generate `write` method
  indent(out) << "def write(self, oprot):" << endl;
  indent_up();

  generate_fastproto_write(out, tstruct);

  indent(out) << "oprot.writeUnionBegin('" << tstruct->name() << "')" << endl;

  first = true;
  for (auto& member : sorted_members) {
    if (is_hidden(*member)) {
      continue;
    }
    auto t = type_to_enum(member->type().get_type());
    const auto& n = member->name();
    auto k = member->id();

    indent(out) << (first ? "" : "el") << "if self.field == " << k << ":"
                << endl;
    indent_up();
    indent(out) << "oprot.writeFieldBegin('" << n << "', " << t << ", " << k
                << ")" << endl;

    indent(out) << rename_reserved_keywords(n) << " = self.value" << endl;
    generate_serialize_field(out, member, "");
    indent(out) << "oprot.writeFieldEnd()" << endl;
    indent_down();
  }
  indent(out) << "oprot.writeFieldStop()" << endl;
  indent(out) << "oprot.writeUnionEnd()" << endl;
  indent_down();
  indent(out) << endl;

  // Generate json reader
  if (gen_json_) {
    generate_json_reader_fn_signature(out);
    indent(out) << "self.field = 0" << endl;
    indent(out) << "self.value = None" << endl;
    indent(out) << "obj = json" << endl;
    indent(out) << "if is_text:" << endl;
    indent_up();
    indent(out) << "obj = loads(json)" << endl;
    indent_down();

    indent(out) << "if not isinstance(obj, dict) or len(obj) > 1:" << endl;
    indent(out) << "  raise TProtocolException("
                << "TProtocolException.INVALID_DATA, 'Can not parse')" << endl;
    indent(out) << endl;

    for (auto& member : members) {
      if (is_hidden(member)) {
        continue;
      }
      const auto& n = member.name();
      indent(out) << "if '" << n << "' in obj:" << endl;
      indent_up();
      generate_json_field(
          out, &member, prefix_temporary(""), "", "obj['" + n + "']");
      indent(out) << "self.set_" << n << "("
                  << prefix_temporary(rename_reserved_keywords(n)) << ")"
                  << endl;
      indent_down();
    }
    indent_down();
    out << endl;
  }

  // Equality and inequality methods that compare by value
  out << indent() << "def __eq__(self, other):" << endl;
  indent_up();
  out << indent() << "if not isinstance(other, self.__class__):" << endl;
  indent_up();
  out << indent() << "return False" << endl;
  indent_down();
  out << endl;
  if (compare_t_fields_only_) {
    out << indent() << "return " << "self.field == other.field and "
        << "self.value == other.value" << endl;
  } else {
    out << indent() << "return " << "self.__dict__ == other.__dict__" << endl;
  }

  indent_down();
  out << endl;

  out << indent() << "def __ne__(self, other):" << endl;
  indent_up();
  out << indent() << "return not (self == other)" << endl;
  indent_down();
  out << endl;

  indent_down();
  generate_py_converter_helpers(out, tstruct);
}

void t_py_generator::generate_py_thrift_spec(
    ofstream& out, const t_structured* tstruct, bool /*is_exception*/) {
  const vector<const t_field*> members = tstruct->fields().copy();
  const vector<const t_field*>& sorted_members = tstruct->fields_id_order();
  vector<const t_field*>::const_iterator m_iter;

  indent(out) << "all_structs.append("
              << rename_reserved_keywords(tstruct->name()) << ")" << endl
              << rename_reserved_keywords(tstruct->name())
              << ".thrift_spec = tuple(__EXPAND_THRIFT_SPEC((" << endl;

  indent_up();

  for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
       ++m_iter) {
    if (is_hidden(**m_iter)) {
      continue;
    }
    indent(out) << "(" << (*m_iter)->id() << ", "
                << type_to_enum((*m_iter)->type().get_type()) << ", " << "'"
                << rename_reserved_keywords((*m_iter)->name()) << "'" << ", "
                << type_to_spec_args((*m_iter)->type().get_type()) << ", "
                << render_field_default_value(*m_iter) << ", "
                << get_thrift_spec_req((*m_iter)->qualifier()) << ", ),"
                << " # " << (*m_iter)->id() << endl;
  }

  indent_down();
  indent(out) << ")))" << endl << endl;

  generate_py_annotations(out, tstruct);

  if (members.size() > 0) {
    out << indent() << "def " << rename_reserved_keywords(tstruct->name())
        << "__init__(self,";
    if (members.size() > 255) {
      out << " **kwargs";
    } else {
      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        if (is_hidden(**m_iter)) {
          continue;
        }
        // This fills in default values, as opposed to nulls
        out << " " << declare_argument(tstruct, *m_iter) << ",";
      }
    }
    out << "):" << endl;

    indent_up();

    if (members.size() > 255) {
      for (const auto& member : members) {
        if (is_hidden(*member)) {
          continue;
        }
        indent(out) << rename_reserved_keywords(member->name())
                    << " = kwargs.pop(\n";
        indent(out) << "  \"" << rename_reserved_keywords(member->name())
                    << "\",\n";
        if (member->default_value() != nullptr) {
          indent(out) << "  " << rename_reserved_keywords(tstruct->name())
                      << ".thrift_spec[" << get_thrift_spec_key(tstruct, member)
                      << "][4],\n";
        } else {
          indent(out) << "  None,\n";
        }
        indent(out) << ")\n";
      }
      indent(out) << "if kwargs:\n";
      indent(out) << "  key, _value = kwargs.popitem()\n";
      indent(out) << "  raise TypeError(\"{}() got an unexpected keyword "
                     "argument '{}'\".format(\""
                  << rename_reserved_keywords(tstruct->name())
                  << "__init__\", key))\n";
    }

    if (tstruct->is<t_union>()) {
      indent(out) << "self.field = 0" << endl;
      indent(out) << "self.value = None" << endl;

      for (auto& member : sorted_members) {
        indent(out) << "if " << rename_reserved_keywords(member->name())
                    << " is not None:" << endl;
        indent(out) << "  assert self.field == 0 and self.value is None"
                    << endl;
        indent(out) << "  self.field = " << member->id() << endl;
        indent(out) << "  self.value = "
                    << rename_reserved_keywords(member->name()) << endl;
      }
    } else {
      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        if (is_hidden(**m_iter)) {
          continue;
        }
        // Initialize fields
        const t_type* type = (*m_iter)->type().get_type();
        if (!type->is<t_primitive_type>() && !type->is<t_enum>() &&
            (*m_iter)->default_value() != nullptr) {
          indent(out) << "if " << rename_reserved_keywords((*m_iter)->name())
                      << " is self.thrift_spec["
                      << get_thrift_spec_key(tstruct, *m_iter)
                      << "][4]:" << endl;
          indent(out) << "  " << rename_reserved_keywords((*m_iter)->name())
                      << " = " << render_field_default_value(*m_iter) << endl;
        }
        indent(out) << "self." << rename_reserved_keywords((*m_iter)->name())
                    << " = " << rename_reserved_keywords((*m_iter)->name())
                    << endl;
      }
    }
    indent_down();

    out << endl;
    out << indent() << rename_reserved_keywords(tstruct->name())
        << ".__init__ = " << rename_reserved_keywords(tstruct->name())
        << "__init__" << endl
        << endl;
  }

  // ThriftStruct.__setstate__: Ensure that unpickled objects have all expected
  // fields.
  if (members.size() > 0 && !tstruct->is<t_union>() && !gen_slots_) {
    out << indent() << "def " << rename_reserved_keywords(tstruct->name())
        << "__setstate__(self, state):" << endl;

    indent_up();
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      if (is_hidden(**m_iter)) {
        continue;
      }
      indent(out) << "state.setdefault('"
                  << rename_reserved_keywords((*m_iter)->name()) << "', "
                  << render_field_default_value(*m_iter) << ")" << endl;
    }
    indent(out) << "self.__dict__ = state" << endl;
    indent_down();

    out << endl;
    out << indent() << rename_reserved_keywords(tstruct->name())
        << ".__getstate__ = lambda self: self.__dict__.copy()" << endl;
    out << indent() << rename_reserved_keywords(tstruct->name())
        << ".__setstate__ = " << rename_reserved_keywords(tstruct->name())
        << "__setstate__" << endl
        << endl;
  }
}

void t_py_generator::generate_py_annotation_dict(
    std::ofstream& out, const deprecated_annotation_map& fields) {
  indent_up();
  for (auto a_iter = fields.begin(); a_iter != fields.end(); ++a_iter) {
    indent(out) << render_string(a_iter->first) << ": "
                << render_string(a_iter->second.value) << ",\n";
  }
  indent_down();
}

void t_py_generator::generate_py_annotations(
    std::ofstream& out, const t_structured* tstruct) {
  const vector<const t_field*>& sorted_members = tstruct->fields_id_order();
  vector<const t_field*>::const_iterator m_iter;

  indent(out) << rename_reserved_keywords(tstruct->name())
              << ".thrift_struct_annotations = {" << endl;
  generate_py_annotation_dict(out, tstruct->unstructured_annotations());
  indent(out) << "}" << endl;

  indent(out) << rename_reserved_keywords(tstruct->name())
              << ".thrift_field_annotations = {" << endl;
  indent_up();

  for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
       ++m_iter) {
    const t_field* field = *m_iter;
    if (field->unstructured_annotations().empty()) {
      continue;
    }
    indent(out) << field->id() << ": {" << endl;
    generate_py_annotation_dict(out, field->unstructured_annotations());
    indent(out) << "}," << endl;
  }
  indent_down();
  indent(out) << "}" << endl << endl;
}

/**
 * Generates a struct definition for a thrift data type.
 *
 * @param tstruct The struct definition
 */
void t_py_generator::generate_py_struct_definition(
    ofstream& out,
    const t_structured* tstruct,
    bool is_exception,
    bool /*is_result*/) {
  node_list_view<const t_field> members = tstruct->fields();
  const vector<const t_field*>& sorted_members = tstruct->fields_id_order();
  vector<const t_field*>::const_iterator m_iter;

  out << "class " << rename_reserved_keywords(tstruct->name());
  if (is_exception) {
    out << "(TException)";
  }
  out << ":" << endl;
  indent_up();
  generate_python_docstring(out, tstruct);

  out << endl;

  /*
     Here we generate the structure specification for the fastproto codec.
     These specifications have the following structure:
     thrift_spec -> tuple of item_spec
     item_spec -> None | (tag, type_enum, name, spec_args, default)
     tag -> integer
     type_enum -> TType.I32 | TType.STRING | TType.STRUCT | ...
     name -> string_literal
     default -> None  # Handled by __init__
     spec_args -> None  # For simple types
                | True/False for Text/Binary Strings
                | (type_enum, spec_args)  # Value type for list/set
                | (type_enum, spec_args, type_enum, spec_args)
                  # Key and value for map
                | [class_name, spec_args_ptr, is_union] # For struct/exception
                | class_name for Enums
     class_name -> identifier  # Basically a pointer to the class
     spec_args_ptr -> expression  # just class_name.spec_args
  */

  if (gen_slots_) {
    indent(out) << "__slots__ = [ " << endl;
    indent_up();
    for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
         ++m_iter) {
      if (is_hidden(**m_iter)) {
        continue;
      }
      indent(out) << "'" << rename_reserved_keywords((*m_iter)->name()) << "',"
                  << endl;
    }
    indent_down();
    indent(out) << " ]" << endl << endl;
  }

  // TODO(dreiss): Test encoding of structs where some inner structs
  // don't have thrift_spec.
  indent(out) << "thrift_spec = None" << endl;
  indent(out) << "thrift_field_annotations = None" << endl;
  indent(out) << "thrift_struct_annotations = None" << endl;
  if (members.size() != 0) {
    indent(out) << "__init__ = None" << endl;
  }

  // Generate `isUnion` method to distinguish union
  indent(out) << "@staticmethod" << endl;
  indent(out) << "def isUnion():" << endl;
  indent(out) << "  return False" << endl << endl;

  generate_py_struct_reader(out, tstruct);
  generate_py_struct_writer(out, tstruct);
  generate_json_reader(out, tstruct);

  // For exceptions only, generate a __str__ method. Use the message annotation
  // if available, otherwise default to __repr__ explicitly. See python bug
  // #5882
  if (is_exception) {
    out << indent() << "def __str__(self):" << endl;
    if (const auto* message_field =
            dynamic_cast<const t_exception&>(*tstruct).get_message_field()) {
      out << indent() << "  if self." << message_field->name() << ":" << endl
          << indent() << "    return self." << message_field->name() << endl
          << indent() << "  else:" << endl
          << indent() << "    return repr(self)" << endl;
    } else {
      out << indent() << "  return repr(self)" << endl;
    }
    out << endl;
  }

  if (!gen_slots_) {
    // According to Python doc, __repr__() "should" return a valid expression
    // such that `object == eval(repr(object))` is true.
    out << indent() << "def __repr__(self):" << endl
        << indent() << "  L = []" << endl
        << indent() << "  padding = ' ' * 4" << endl;
    for (const auto& member : members) {
      if (is_hidden(member)) {
        continue;
      }
      auto key = rename_reserved_keywords(member.name());
      auto has_double_underscore = key.find("__") == 0;
      if (has_double_underscore) {
        out << indent() << "  if getattr(self, \"" << key
            << "\", None) is not None:" << endl;
      } else {
        out << indent() << "  if self." << key << " is not None:" << endl;
      }

      indent_up();
      if (has_double_underscore) {
        out << indent() << "  value = pprint.pformat(getattr(self, \"" << key
            << "\", None), indent=0)" << endl;
      } else {
        out << indent() << "  value = pprint.pformat(self." << key
            << ", indent=0)" << endl;
      }
      out << indent() << "  value = padding.join(value.splitlines(True))"
          << endl
          << indent() << "  L.append('    " << key << "=%s' % (value))" << endl;
      indent_down();
    }

    // For exceptions only, force message attribute to be included in
    // __repr__(). This is because BaseException.message has been deprecated as
    // of Python 2.6 so python refuses to include the message attribute in
    // __dict__ of an Exception object which is used for generating return
    // value of __repr__.
    if (is_exception) {
      out << indent() << "  if 'message' not in self.__dict__:" << endl
          << indent() << "    message = getattr(self, 'message', None)" << endl
          << indent() << "    if message:" << endl
          << indent() << "      L.append('message=%r' % message)" << endl;
    }

    out << indent() << "  return \"%s(%s)\" % (self.__class__.__name__, "
        << R"("\n" + ",\n".join(L) if L else ''))" << endl
        << endl;

    // Equality and inequality methods that compare by value
    out << indent() << "def __eq__(self, other):" << endl;
    indent_up();
    out << indent() << "if not isinstance(other, self.__class__):" << endl;
    indent_up();
    out << indent() << "return False" << endl;
    indent_down();
    out << endl;
    if (compare_t_fields_only_) {
      out << indent() << "spec_t_fields = parse_struct_spec(self)" << endl;
      out << indent() << "return "
          << "all(getattr(self, field.name, field.default) "
          << "== getattr(other, field.name, field.default)"
          << " for field in spec_t_fields)" << endl;
    } else {
      out << indent() << "return " << "self.__dict__ == other.__dict__ "
          << endl;
    }
    indent_down();
    out << endl;

    out << indent() << "def __ne__(self, other):" << endl;
    indent_up();

    out << indent() << "return not (self == other)" << endl;
    indent_down();
    out << endl;
  } else {
    // Use __slots__ instead of __dict__ for implementing
    // __eq__, __repr__, __ne__
    out << indent() << "def __repr__(self):" << endl
        << indent() << "  L = []" << endl
        << indent() << "  padding = ' ' * 4" << endl
        << indent() << "  for key in self.__slots__:" << endl
        << indent() << "    value = getattr(self, key)" << endl
        << indent() << "    if value is None:" << endl
        << indent() << "        continue" << endl
        << indent() << "    value = pprint.pformat(value)" << endl
        << indent() << "    value = padding.join(value.splitlines(True))"
        << endl
        << indent() << "    L.append('    %s=%s' % (key, value))" << endl
        << indent() << "  return \"%s(\\n%s)\" % (self.__class__.__name__, "
        << R"("\n" + ",\n".join(L) if L else ''))" << endl
        << endl;

    // Equality method that compares each attribute by value and type,
    // walking __slots__
    out << indent() << "def __eq__(self, other):" << endl
        << indent() << "  if not isinstance(other, self.__class__):" << endl
        << indent() << "    return False" << endl
        << indent() << "  for attr in self.__slots__:" << endl
        << indent() << "    my_val = getattr(self, attr)" << endl
        << indent() << "    other_val = getattr(other, attr)" << endl
        << indent() << "    if my_val != other_val:" << endl
        << indent() << "      return False" << endl
        << indent() << "  return True" << endl
        << endl;

    out << indent() << "def __ne__(self, other):" << endl
        << indent() << "  return not (self == other)" << endl
        << endl;
  }

  out << indent() << "def __dir__(self):" << endl;
  indent_up();
  out << indent() << "return (" << endl;
  indent_up();
  for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
       ++m_iter) {
    if (is_hidden(**m_iter)) {
      continue;
    }
    indent(out) << "'" << rename_reserved_keywords((*m_iter)->name()) << "',"
                << endl;
  }
  indent_down();
  indent(out) << ")" << endl << endl;
  indent_down();

  indent_down();

  indent_up();
  out << indent() << "__hash__ = object.__hash__" << endl;
  out << endl;

  indent_down();
}

void t_py_generator::generate_fastproto_write(
    ofstream& out, const t_structured* tstruct) {
  indent(out)
      << "if (isinstance(oprot, TBinaryProtocol.TBinaryProtocolAccelerated) "
         "or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and "
         "oprot.get_protocol_id() == "
         "THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) "
         "and self.thrift_spec is not None "
         "and fastproto is not None:"
      << endl;
  indent_up();

  indent(out) << "oprot.trans.write(fastproto.encode(self, "
              << "[self.__class__, self.thrift_spec, "
              << (tstruct->is<t_union>() ? "True" : "False") << "], "
              << "utf8strings=UTF8STRINGS, protoid=0))" << endl;
  indent(out) << "return" << endl;

  indent_down();
  indent(out)
      << "if (isinstance(oprot, TCompactProtocol.TCompactProtocolAccelerated) "
         "or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and "
         "oprot.get_protocol_id() == "
         "THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) "
         "and self.thrift_spec is not None "
         "and fastproto is not None:"
      << endl;
  indent_up();

  indent(out) << "oprot.trans.write(fastproto.encode(self, "
              << "[self.__class__, self.thrift_spec, "
              << (tstruct->is<t_union>() ? "True" : "False") << "], "
              << "utf8strings=UTF8STRINGS, protoid=2))" << endl;
  indent(out) << "return" << endl;
  indent_down();
}

void t_py_generator::generate_fastproto_read(
    ofstream& out, const t_structured* tstruct) {
  indent(out)
      << "if (isinstance(iprot, TBinaryProtocol.TBinaryProtocolAccelerated) "
         "or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and "
         "iprot.get_protocol_id() == "
         "THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) "
         "and isinstance(iprot.trans, TTransport.CReadableTransport) "
         "and self.thrift_spec is not None "
         "and fastproto is not None:"
      << endl;
  indent_up();

  indent(out) << "fastproto.decode(self, iprot.trans, "
              << "[self.__class__, self.thrift_spec, "
              << (tstruct->is<t_union>() ? "True" : "False") << "], "
              << "utf8strings=UTF8STRINGS, protoid=0)" << endl;
  indent(out) << "return" << endl;
  indent_down();

  indent(out)
      << "if (isinstance(iprot, TCompactProtocol.TCompactProtocolAccelerated) "
         "or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and "
         "iprot.get_protocol_id() == "
         "THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) "
         "and isinstance(iprot.trans, TTransport.CReadableTransport) "
         "and self.thrift_spec is not None "
         "and fastproto is not None:"
      << endl;
  indent_up();

  indent(out) << "fastproto.decode(self, iprot.trans, "
              << "[self.__class__, self.thrift_spec, "
              << (tstruct->is<t_union>() ? "True" : "False") << "], "
              << "utf8strings=UTF8STRINGS, protoid=2)" << endl;
  indent(out) << "return" << endl;
  indent_down();
}

/**
 * Generates the read method for a struct
 */
void t_py_generator::generate_py_struct_reader(
    ofstream& out, const t_structured* tstruct) {
  indent(out) << "def read(self, iprot):" << endl;
  indent_up();

  generate_fastproto_read(out, tstruct);

  indent(out) << "iprot.readStructBegin()" << endl;

  // Loop over reading in fields
  indent(out) << "while True:" << endl;
  indent_up();

  // Read beginning field marker
  indent(out) << "(fname, ftype, fid) = iprot.readFieldBegin()" << endl;

  // Check for field STOP marker and break
  indent(out) << "if ftype == TType.STOP:" << endl;
  indent_up();
  indent(out) << "break" << endl;
  indent_down();

  // Switch statement on the field we are reading
  bool first = true;

  // Generate deserialization code for known cases
  for (const t_field& f_iter : tstruct->fields()) {
    if (is_hidden(f_iter)) {
      continue;
    }
    if (first) {
      first = false;
      out << indent() << "if ";
    } else {
      out << indent() << "elif ";
    }
    out << "fid == " << f_iter.id() << ":" << endl;
    indent_up();
    indent(out) << "if ftype == " << type_to_enum(f_iter.type().get_type())
                << ":" << endl;
    indent_up();
    generate_deserialize_field(out, &f_iter, "self.");
    indent_down();
    out << indent() << "else:" << endl
        << indent() << "  iprot.skip(ftype)" << endl;
    indent_down();
  }

  // In the default case we skip the field
  out << indent() << "else:" << endl
      << indent() << "  iprot.skip(ftype)" << endl;

  // Read field end marker
  indent(out) << "iprot.readFieldEnd()" << endl;

  indent_down();

  indent(out) << "iprot.readStructEnd()" << endl;
  indent_down();

  out << endl;
}

void t_py_generator::generate_py_struct_writer(
    ofstream& out, const t_structured* tstruct) {
  const string& name = tstruct->name();
  const vector<const t_field*>& fields = tstruct->fields_id_order();
  vector<const t_field*>::const_iterator f_iter;

  indent(out) << "def write(self, oprot):" << endl;
  indent_up();

  generate_fastproto_write(out, tstruct);

  indent(out) << "oprot.writeStructBegin('" << name << "')" << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (is_hidden(**f_iter)) {
      continue;
    }
    // Write field header
    indent(out) << "if self." << rename_reserved_keywords((*f_iter)->name())
                << " != None";
    if ((*f_iter)->qualifier() == t_field_qualifier::optional &&
        (*f_iter)->default_value() != nullptr) {
      // An optional field with a value set should not be serialized if
      // the value equals the default value
      out << " and self." << rename_reserved_keywords((*f_iter)->name())
          << " != " << "self.thrift_spec["
          << get_thrift_spec_key(tstruct, *f_iter) << "][4]";
    }
    out << ":" << endl;
    indent_up();
    indent(out) << "oprot.writeFieldBegin(" << "'" << (*f_iter)->name() << "', "
                << type_to_enum((*f_iter)->type().get_type()) << ", "
                << (*f_iter)->id() << ")" << endl;

    // Write field contents
    generate_serialize_field(out, *f_iter, "self.");

    // Write field closer
    indent(out) << "oprot.writeFieldEnd()" << endl;

    indent_down();
  }

  // Write the struct map
  out << indent() << "oprot.writeFieldStop()" << endl
      << indent() << "oprot.writeStructEnd()" << endl;

  indent_down();
  out << endl;
}

/**
 * Generates a thrift service.
 *
 * @param tservice The service definition
 */
void t_py_generator::generate_service(const t_service* tservice) {
  string f_service_filename = rename_reserved_keywords(service_name_) + ".py";
  auto f_service_path = package_dir_ / f_service_filename;
  f_service_.open(f_service_path);
  record_genfile(f_service_path);

  f_service_ << py_autogen_comment() << endl << py_imports() << endl;

  if (tservice->extends() != nullptr) {
    f_service_ << "import "
               << get_real_py_module(tservice->extends()->program()) << "."
               << rename_reserved_keywords(tservice->extends()->name()) << endl;
  }

  f_service_ << "from .ttypes import " << render_ttype_declarations("") << endl
             << render_includes() << "from thrift.Thrift import TProcessor"
             << endl
             << render_fastproto_includes() << endl
             << "all_structs = []" << endl
             << "UTF8STRINGS = bool(" << gen_utf8strings_ << ") or "
             << "sys.version_info.major >= 3" << endl
             << endl;

  if (gen_future_) {
    f_service_ << "from concurrent.futures import Future, ThreadPoolExecutor"
               << endl;
  }

  if (gen_asyncio_) {
    f_service_ << "import asyncio" << endl
               << "from thrift.util.asyncio import call_as_future" << endl;
  }
  f_service_ << "from thrift.util.Decorators import (" << endl
             << "  future_process_main," << endl
             << "  future_process_method," << endl
             << "  process_main as thrift_process_main," << endl
             << "  process_method as thrift_process_method," << endl
             << "  should_run_on_thread," << endl
             << "  write_results_after_future," << endl
             << ")" << endl;

  f_service_ << endl;

  // Generate the three main parts of the service (well, two for now in PHP)
  generate_service_interface(tservice, false);
  if (!gen_future_) {
    generate_service_interface(tservice, true);
  }

  generate_service_helpers(tservice);
  generate_service_client(tservice);
  generate_service_server(tservice, false);
  if (!gen_future_) {
    generate_service_server(tservice, true);
  }
  generate_service_remote(tservice);
  generate_service_fuzzer(tservice);

  // Close service file
  f_service_ << "fix_spec(all_structs)" << endl
             << "del all_structs" << endl
             << endl;
  f_service_.close();
}

/**
 * Generates helper functions for a service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_py_generator::generate_service_helpers(const t_service* tservice) {
  const auto& functions = get_functions(tservice);

  f_service_ << "# HELPER FUNCTIONS AND STRUCTURES" << endl << endl;

  for (auto f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    const t_paramlist& ts = (*f_iter)->params();
    generate_py_struct_definition(f_service_, &ts, false);
    generate_py_thrift_spec(f_service_, &ts, false);
    generate_py_function_helpers(*f_iter);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_py_generator::generate_py_function_helpers(const t_function* tfunction) {
  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    t_struct result(
        program_, rename_reserved_keywords(tfunction->name()) + "_result");
    if (!tfunction->return_type()->is_void()) {
      result.create_field(tfunction->return_type(), "success", 0);
    }
    if (tfunction->exceptions() != nullptr) {
      legacy_copy_exception_fields(*tfunction->exceptions(), result);
    }
    generate_py_struct_definition(f_service_, &result, false, true);
    generate_py_thrift_spec(f_service_, &result, false);
  }
}

void t_py_generator::generate_py_converter_helpers(
    std::ofstream& out, const t_structured* tstruct) {
  indent_up();
  // TODO: accommodate root_module_prefix
  auto python_namespace = get_py3_namespace_with_name_and_prefix(program_, "");
  out << indent() << "def _to_python(self):" << endl;
  indent_up();
  out << indent() << "import importlib" << endl
      << indent() << "import thrift.python.converter" << endl
      << indent() << "python_types = importlib.import_module(\""
      << python_namespace << ".thrift_types\")" << endl
      << indent()
      << "return thrift.python.converter.to_python_struct(python_types."
      << rename_reserved_keywords(tstruct->name()) << ", self" << ")" << endl
      << endl;
  indent_down();

  out << indent() << "def _to_mutable_python(self):" << endl;
  indent_up();
  out << indent() << "import importlib" << endl
      << indent() << "import thrift.python.mutable_converter" << endl
      << indent() << "python_mutable_types = importlib.import_module(\""
      << python_namespace << ".thrift_mutable_types\")" << endl
      << indent() << "return "
      << "thrift.python.mutable_converter.to_mutable_python_struct_or_union("
      << "python_mutable_types." << rename_reserved_keywords(tstruct->name())
      << ", self" << ")" << endl
      << endl;
  indent_down();

  auto py3_namespace = get_py3_namespace_with_name_and_prefix(program_, "");
  out << indent() << "def _to_py3(self):" << endl;
  indent_up();
  out << indent() << "import importlib" << endl
      << indent() << "import thrift.py3.converter" << endl
      << indent() << "py3_types = importlib.import_module(\"" << py3_namespace
      << ".types\")" << endl
      << indent() << "return thrift.py3.converter.to_py3_struct(py3_types."
      << rename_reserved_keywords(tstruct->name()) << ", self" << ")" << endl
      << endl;
  indent_down();

  out << indent() << "def _to_py_deprecated(self):" << endl
      << indent() << "  return self" << endl
      << endl;

  indent_down();
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 */
void t_py_generator::generate_service_interface(
    const t_service* tservice, bool with_context) {
  string iface_prefix = with_context ? "Context" : "";
  string extends;
  string extends_if;
  if (tservice->extends() != nullptr) {
    extends = service_name(tservice->extends());
    extends_if = "(" + extends + "." + iface_prefix + "Iface)";
  }

  f_service_ << "class " << iface_prefix << "Iface" << extends_if << ":"
             << endl;
  indent_up();
  generate_python_docstring(f_service_, tservice);
  if (!tservice->unstructured_annotations().empty()) {
    f_service_ << indent() << "annotations = {" << endl;
    generate_py_annotation_dict(
        f_service_, tservice->unstructured_annotations());
    f_service_ << indent() << "}" << endl << endl;
  }
  std::string service_priority = get_priority(tservice);
  const std::vector<const t_function*>& functions = get_functions(tservice);
  if (functions.empty()) {
    f_service_ << indent() << "pass" << endl;
  } else {
    for (auto f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
      f_service_ << indent() << "def "
                 << function_signature_if(*f_iter, with_context) << ":" << endl;
      indent_up();
      generate_python_docstring(f_service_, (*f_iter));
      f_service_ << indent() << "pass" << endl << endl;
      indent_down();

      if (gen_future_) {
        f_service_ << indent() << "def future_"
                   << function_signature_if(*f_iter, false) << ":" << endl;
        indent_up();
        generate_python_docstring(f_service_, (*f_iter));
        f_service_ << indent() << "fut = Future()" << endl
                   << indent() << "try:" << endl;
        indent_up();
        f_service_ << indent() << "fut.set_result(self." << (*f_iter)->name()
                   << "(";
        for (const t_field& it : (*f_iter)->params().fields()) {
          f_service_ << rename_reserved_keywords(it.name()) << ",";
        }
        f_service_ << "))" << endl;
        indent_down();
        f_service_ << indent() << "except:" << endl;
        indent_up();
        f_service_ << indent() << "fut.set_exception(sys.exc_info()[1])"
                   << endl;
        indent_down();
        f_service_ << indent() << "return fut" << endl << endl;
        indent_down();
      }
    }
  }

  indent_down();
  f_service_ << endl;
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_py_generator::generate_service_client(const t_service* tservice) {
  string extends;
  string extends_client;
  if (tservice->extends() != nullptr) {
    extends = service_name(tservice->extends());
    extends_client = extends + ".Client, ";
  }

  f_service_ << "class Client(" << extends_client << "Iface):" << endl;
  indent_up();
  generate_python_docstring(f_service_, tservice);

  f_service_ << indent() << "_fbthrift_force_cpp_transport = "
             << (gen_cpp_transport_ ? "True" : "False") << endl
             << endl;

  // Context Handlers
  if (!gen_asyncio_) {
    f_service_ << indent() << "def __enter__(self):" << endl
               << indent() << "  if self._fbthrift_cpp_transport:" << endl
               << indent() << "    self._fbthrift_cpp_transport.__enter__()"
               << endl
               << indent() << "  return self" << endl
               << endl;
    f_service_ << indent() << "def __exit__(self, type, value, tb):" << endl
               << indent() << "  if self._fbthrift_cpp_transport:" << endl
               << indent()
               << "    self._fbthrift_cpp_transport.__exit__(type, value, tb)"
               << endl
               << indent() << "  if self._iprot:" << endl
               << indent() << "    self._iprot.trans.close()" << endl
               << indent()
               << "  if self._oprot and self._iprot is not self._oprot:" << endl
               << indent() << "    self._oprot.trans.close()" << endl
               << endl;
  }

  // Constructor function
  if (gen_asyncio_) {
    f_service_
        << indent()
        << "def __init__(self, oprot=None, loop=None, cpp_transport=None):"
        << endl;
  } else {
    f_service_
        << indent()
        << "def __init__(self, iprot=None, oprot=None, cpp_transport=None):"
        << endl;
  }
  if (extends.empty()) {
    if (gen_asyncio_) {
      f_service_ << indent() << "  self._oprot = oprot" << endl
                 << indent()
                 << "  self._loop = loop or asyncio.get_event_loop()" << endl
                 << indent() << "  self._seqid = 0" << endl
                 << indent() << "  self._futures = {}" << endl
                 << indent() << "  self._fbthrift_cpp_transport = None" << endl
                 << endl;
    } else {
      f_service_ << indent() << "  self._iprot = self._oprot = iprot" << endl
                 << indent() << "  if oprot != None:" << endl
                 << indent() << "    self._oprot = oprot" << endl
                 << indent() << "  self._seqid = 0" << endl
                 << indent() << "  self._fbthrift_cpp_transport = cpp_transport"
                 << endl
                 << endl;
    }
  } else {
    if (gen_asyncio_) {
      f_service_ << indent() << "  " << extends
                 << ".Client.__init__(self, oprot, loop)" << endl
                 << endl;
    } else {
      f_service_ << indent() << "  " << extends
                 << ".Client.__init__(self, iprot, oprot, cpp_transport)"
                 << endl
                 << endl;
    }
  }

  // Helpers
  f_service_
      << indent() << "def set_persistent_header(self, key, value):" << endl
      << indent() << "  if self._fbthrift_cpp_transport:" << endl
      << indent()
      << "    self._fbthrift_cpp_transport.set_persistent_header(key, value)"
      << endl
      << indent() << "  else:" << endl
      << indent() << "    try:" << endl
      << indent() << "      self._oprot.trans.set_persistent_header(key, value)"
      << endl
      << indent() << "    except AttributeError:" << endl
      << indent() << "      pass" << endl
      << endl;

  f_service_
      << indent() << "def get_persistent_headers(self):" << endl
      << indent() << "  if self._fbthrift_cpp_transport:" << endl
      << indent()
      << "    return self._fbthrift_cpp_transport.get_persistent_headers()"
      << endl
      << indent() << "  try:" << endl
      << indent()
      << "    return self._oprot.trans.get_write_persistent_headers()" << endl
      << indent() << "  except AttributeError:" << endl
      << indent() << "    return {}" << endl
      << endl;

  f_service_ << indent() << "def clear_persistent_headers(self):" << endl
             << indent() << "  if self._fbthrift_cpp_transport:" << endl
             << indent()
             << "    self._fbthrift_cpp_transport.clear_persistent_headers()"
             << endl
             << indent() << "  else:" << endl
             << indent() << "    try:" << endl
             << indent() << "      self._oprot.trans.clear_persistent_headers()"
             << endl
             << indent() << "    except AttributeError:" << endl
             << indent() << "      pass" << endl
             << endl;

  f_service_
      << indent() << "def set_onetime_header(self, key, value):" << endl
      << indent() << "  if self._fbthrift_cpp_transport:" << endl
      << indent()
      << "    self._fbthrift_cpp_transport.set_onetime_header(key, value)"
      << endl
      << indent() << "  else:" << endl
      << indent() << "    try:" << endl
      << indent() << "      self._oprot.trans.set_header(key, value)" << endl
      << indent() << "    except AttributeError:" << endl
      << indent() << "      pass" << endl
      << endl;

  f_service_
      << indent() << "def get_last_response_headers(self):" << endl
      << indent() << "  if self._fbthrift_cpp_transport:" << endl
      << indent()
      << "    return self._fbthrift_cpp_transport.get_last_response_headers()"
      << endl
      << indent() << "  try:" << endl
      << indent() << "    return self._iprot.trans.get_headers()" << endl
      << indent() << "  except AttributeError:" << endl
      << indent() << "    return {}" << endl
      << endl;

  f_service_ << indent() << "def set_max_frame_size(self, size):" << endl
             << indent() << "  if self._fbthrift_cpp_transport:" << endl
             << indent() << "    pass" << endl
             << indent() << "  else:" << endl
             << indent() << "    try:" << endl
             << indent() << "      self._oprot.trans.set_max_frame_size(size)"
             << endl
             << indent() << "    except AttributeError:" << endl
             << indent() << "      pass" << endl
             << endl;

  // Generate client method implementations
  const auto& functions = get_functions(tservice);
  vector<const t_function*>::const_iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    const t_paramlist& arg_struct = (*f_iter)->params();
    node_list_view<const t_field> fields = arg_struct.fields();
    string funname = rename_reserved_keywords((*f_iter)->name());
    string argsname = (*f_iter)->name() + "_args";

    // Open function
    indent(f_service_) << "def " << function_signature(*f_iter) << ":" << endl;
    indent_up();
    generate_python_docstring(f_service_, (*f_iter));

    // CPP transport
    if (!gen_asyncio_) {
      indent(f_service_) << "if (self._fbthrift_cpp_transport):" << endl;
      indent_up();
      f_service_ << indent() << "args = " << argsname << "()" << endl;
      for (const t_field& fld_iter : fields) {
        f_service_ << indent() << "args."
                   << rename_reserved_keywords(fld_iter.name()) << " = "
                   << rename_reserved_keywords(fld_iter.name()) << endl;
      }
      f_service_ << indent()
                 << "result = self._fbthrift_cpp_transport._send_request(\""
                 << tservice->name() << "\", \"" << (*f_iter)->name()
                 << "\", args, " << (*f_iter)->name() << "_result)" << endl;
      if (!(*f_iter)->return_type()->is_void()) {
        f_service_ << indent() << "if result.success is not None:" << endl
                   << indent() << "  return result.success" << endl;
      }

      for (const t_field& ex : get_elems((*f_iter)->exceptions())) {
        auto name = rename_reserved_keywords(ex.name());
        f_service_ << indent() << "if result." << name
                   << " is not None:" << endl
                   << indent() << "  raise result." << name << endl;
      }

      if ((*f_iter)->return_type()->is_void()) {
        f_service_ << indent() << "return None" << endl;
      } else {
        f_service_
            << indent()
            << "raise "
               "TApplicationException(TApplicationException.MISSING_RESULT)"
            << endl;
      }
      indent_down();
    }

    if (gen_asyncio_) {
      indent(f_service_) << "self._seqid += 1" << endl;
      indent(f_service_) << "fut = self._futures[self._seqid] = "
                            "asyncio.Future(loop=self._loop)"
                         << endl;
    }

    indent(f_service_) << "self.send_" << funname << "(";

    bool first = true;
    for (const t_field& fld_iter : fields) {
      if (first) {
        first = false;
      } else {
        f_service_ << ", ";
      }
      f_service_ << rename_reserved_keywords(fld_iter.name());
    }
    f_service_ << ")" << endl;

    if ((*f_iter)->qualifier() != t_function_qualifier::oneway) {
      f_service_ << indent();
      if (gen_asyncio_) {
        f_service_ << "return fut" << endl;
      } else {
        if (!(*f_iter)->return_type()->is_void()) {
          f_service_ << "return ";
        }
        f_service_ << "self.recv_" << funname << "()" << endl;
      }
    } else {
      if (gen_asyncio_) {
        f_service_ << indent() << "fut.set_result(None)" << endl
                   << indent() << "return fut" << endl;
      }
    }
    indent_down();
    f_service_ << endl;

    indent(f_service_) << "def send_" << function_signature(*f_iter) << ":"
                       << endl;

    indent_up();

    // Serialize the request header
    f_service_ << indent() << "self._oprot.writeMessageBegin('"
               << (*f_iter)->name() << "', TMessageType.CALL, self._seqid)"
               << endl;

    f_service_ << indent() << "args = " << argsname << "()" << endl;

    for (const t_field& fld_iter : fields) {
      f_service_ << indent() << "args."
                 << rename_reserved_keywords(fld_iter.name()) << " = "
                 << rename_reserved_keywords(fld_iter.name()) << endl;
    }

    std::string flush = (*f_iter)->qualifier() == t_function_qualifier::oneway
        ? "onewayFlush"
        : "flush";
    // Write to the stream
    f_service_ << indent() << "args.write(self._oprot)" << endl
               << indent() << "self._oprot.writeMessageEnd()" << endl
               << indent() << "self._oprot.trans." << flush << "()" << endl;

    indent_down();

    if ((*f_iter)->qualifier() != t_function_qualifier::oneway) {
      std::string resultname = (*f_iter)->name() + "_result";
      // Open function
      f_service_ << endl;
      if (gen_asyncio_) {
        f_service_ << indent() << "def recv_" << (*f_iter)->name()
                   << "(self, iprot, mtype, rseqid):" << endl;
      } else {
        t_function recv_function(
            program_,
            (*f_iter)->return_type(),
            string("recv_") + (*f_iter)->name());
        f_service_ << indent() << "def " << function_signature(&recv_function)
                   << ":" << endl;
      }
      indent_up();

      // TODO(mcslee): Validate message reply here, seq ids etc.

      if (gen_asyncio_) {
        f_service_ << indent() << "try:" << endl;
        f_service_ << indent() << indent() << "fut = self._futures.pop(rseqid)"
                   << endl;
        f_service_ << indent() << "except KeyError:" << endl;
        f_service_ << indent() << indent() << "return   # request timed out"
                   << endl;
      } else {
        f_service_ << indent() << "(fname, mtype, rseqid) = "
                   << "self._iprot.readMessageBegin()" << endl;
      }

      f_service_ << indent() << "if mtype == TMessageType.EXCEPTION:" << endl
                 << indent() << "  x = TApplicationException()" << endl;

      if (gen_asyncio_) {
        f_service_ << indent() << "  x.read(iprot)" << endl
                   << indent() << "  iprot.readMessageEnd()" << endl
                   << indent() << "  fut.set_exception(x)" << endl
                   << indent() << "  return" << endl
                   << indent() << "result = " << resultname << "()" << endl
                   << indent() << "try:" << endl
                   << indent() << "  result.read(iprot)" << endl
                   << indent() << "except Exception as e:" << endl
                   << indent() << "  fut.set_exception(e)" << endl
                   << indent() << "  return" << endl
                   << indent() << "iprot.readMessageEnd()" << endl;
      } else {
        f_service_ << indent() << "  x.read(self._iprot)" << endl
                   << indent() << "  self._iprot.readMessageEnd()" << endl
                   << indent() << "  raise x" << endl
                   << indent() << "result = " << resultname << "()" << endl
                   << indent() << "result.read(self._iprot)" << endl
                   << indent() << "self._iprot.readMessageEnd()" << endl;
      }

      // Careful, only return _result if not a void function
      if (!(*f_iter)->return_type()->is_void()) {
        f_service_ << indent() << "if result.success != None:" << endl;
        if (gen_asyncio_) {
          f_service_ << indent() << "  fut.set_result(result.success)" << endl
                     << indent() << "  return" << endl;
        } else {
          f_service_ << indent() << "  return result.success" << endl;
        }
      }

      for (const t_field& x : get_elems((*f_iter)->exceptions())) {
        f_service_ << indent() << "if result."
                   << rename_reserved_keywords(x.name()) << " != None:" << endl;
        if (gen_asyncio_) {
          f_service_ << indent() << "  fut.set_exception(result."
                     << rename_reserved_keywords(x.name()) << ")" << endl
                     << indent() << "  return" << endl;
        } else {
          f_service_ << indent() << "  raise result."
                     << rename_reserved_keywords(x.name()) << endl;
        }
      }

      // Careful, only return _result if not a void function
      if ((*f_iter)->return_type()->is_void()) {
        if (gen_asyncio_) {
          f_service_ << indent() << "fut.set_result(None)" << endl
                     << indent() << "return" << endl;
        } else {
          indent(f_service_) << "return" << endl;
        }
      } else {
        if (gen_asyncio_) {
          f_service_ << indent() << "fut.set_exception(TApplicationException("
                     << "TApplicationException.MISSING_RESULT, \""
                     << (*f_iter)->name() << " failed: unknown result\"))"
                     << endl
                     << indent() << "return" << endl;
        } else {
          f_service_ << indent() << "raise TApplicationException("
                     << "TApplicationException.MISSING_RESULT, \""
                     << (*f_iter)->name() << " failed: unknown result\");"
                     << endl;
        }
      }

      // Close function
      indent_down();
      f_service_ << endl;
    }
  }

  indent_down();
  f_service_ << endl;
}

/**
 * Generates a command line tool for making remote requests
 *
 * @param tservice The service to generate a remote for.
 */
void t_py_generator::generate_service_remote(const t_service* tservice) {
  string f_remote_filename = service_name_ + "-remote";
  auto f_remote_path = package_dir_ / f_remote_filename;
  std::ofstream f_remote;
  f_remote.open(f_remote_path);
  record_genfile(f_remote_path);

  f_remote << "#!/usr/bin/env python\n"
           << py_autogen_comment()
           << "\n"
              "from __future__ import print_function\n"
              "from __future__ import absolute_import\n"
              "\n"
              "import os\n"
              "import sys\n"
              "\n"
           <<
      // This has to be before thrift definitions
      // in case the environment is not correct.
      py_par_warning("remote") <<
      // Import the service module and types
      "\n" << "from . import " << rename_reserved_keywords(service_name_)
           << "\n"
           << "from . import ttypes\n"
              "\n"
              "from thrift.util.remote import Function\n"
              "from thrift.remote import Remote\n"
           << "\n";

  // Emit a list of objects describing the service functions.
  // The library code will use this to print the usage message and
  // perform function argument processing
  f_remote << "FUNCTIONS = {\n";

  set<string> processed_fns;
  for (const t_service* cur_service = tservice; cur_service != nullptr;
       cur_service = cur_service->extends()) {
    const string& svc_name = cur_service->name();
    const auto& functions = get_functions(cur_service);
    for (vector<const t_function*>::const_iterator it = functions.begin();
         it != functions.end();
         ++it) {
      const t_function* fn = *it;
      const string& fn_name = fn->name();
      pair<set<string>::iterator, bool> ret = processed_fns.insert(fn_name);
      if (!ret.second) {
        // A child class has overridden this function, so we've listed it
        // already.
        continue;
      }

      f_remote << "    '" << fn_name << "': Function('" << fn_name << "', '"
               << svc_name << "', ";
      if (fn->qualifier() == t_function_qualifier::oneway) {
        f_remote << "None, ";
      } else {
        f_remote << "'" << thrift_type_name(fn->return_type().get_type())
                 << "', ";
      }

      f_remote << "[";
      bool first = true;
      for (const t_field& it_2 : fn->params().fields()) {
        if (first) {
          first = false;
        } else {
          f_remote << ", ";
        }
        f_remote << "('" << thrift_type_name(it_2.type().get_type()) << "', '"
                 << it_2.name() << "', '"
                 << thrift_type_name(it_2.type()->get_true_type()) << "')";
      }
      f_remote << "]),\n";
    }
  }
  f_remote << "}\n\n";

  // Similar, but for service names
  f_remote << "SERVICE_NAMES = [";
  for (const t_service* cur_service = tservice; cur_service != nullptr;
       cur_service = cur_service->extends()) {
    f_remote << "'" << cur_service->name() << "', ";
  }
  f_remote << "]\n\n";

  f_remote << "if __name__ == '__main__':\n"
              "    Remote.run(FUNCTIONS, SERVICE_NAMES, "
           << rename_reserved_keywords(service_name_)
           << ", ttypes, sys.argv, default_port=" << default_port_ << ")\n";

  // Close the remote file
  f_remote.close();

  // Make file executable
  mark_file_executable(f_remote_path);
}

/**
 * Generates a commandline tool for fuzz testing
 *
 * @param tservice The service to generate a fuzzer for.
 */
void t_py_generator::generate_service_fuzzer(const t_service* /*tservice*/) {
  string f_fuzzer_filename = service_name_ + "-fuzzer";
  auto f_fuzzer_path = package_dir_ / f_fuzzer_filename;
  std::ofstream f_fuzzer;
  f_fuzzer.open(f_fuzzer_path);
  record_genfile(f_fuzzer_path);

  f_fuzzer << "#!/usr/bin/env python\n"
           << py_autogen_comment()
           << "\n"
              "from __future__ import absolute_import\n"
              "from __future__ import division\n"
              "from __future__ import print_function\n"
              "from __future__ import unicode_literals\n"
              "\n"
              "import os\n"
              "import sys\n"
              "\n"
           << py_par_warning("fuzzer") << "\n"
           << "from . import " << rename_reserved_keywords(service_name_)
           << "\n"
           << "from . import ttypes\n"
           << "from . import constants\n"
           << "\n"
              "import thrift.util.fuzzer"
              "\n"
              "thrift.util.fuzzer.fuzz_service("
           << rename_reserved_keywords(service_name_)
           << ", ttypes, constants)\n";
  f_fuzzer.close();
  mark_file_executable(f_fuzzer_path);
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_py_generator::generate_service_server(
    const t_service* tservice, bool with_context) {
  string class_prefix = with_context ? "Context" : "";

  // Generate the dispatch methods
  const auto& functions = get_functions(tservice);

  string extends;
  string extends_processor;
  if (tservice->extends() != nullptr) {
    extends = service_name(tservice->extends());
    extends_processor = extends + "." + class_prefix + "Processor, ";
  }

  // Generate the header portion
  f_service_ << "class " << class_prefix << "Processor(" << extends_processor
             << class_prefix << "Iface, TProcessor):" << endl;

  indent_up();

  f_service_ << indent() << "_onewayMethods = (";
  for (auto f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if ((*f_iter)->qualifier() == t_function_qualifier::oneway) {
      f_service_ << "\"" << (*f_iter)->name() << "\",";
    }
  }
  f_service_ << ")" << endl << endl;

  if (gen_future_) {
    indent(f_service_) << "def __init__(self, handler, executor=None):" << endl;
  } else if (gen_asyncio_) {
    indent(f_service_) << "def __init__(self, handler, loop=None):" << endl;
  } else {
    indent(f_service_) << "def __init__(self, handler):" << endl;
  }
  indent_up();
  if (extends.empty()) {
    f_service_ << indent() << "TProcessor.__init__(self)" << endl;
    f_service_ << indent() << "self._handler = handler" << endl;

    if (gen_future_) {
      f_service_ << indent() << "self._executor = executor or "
                 << "ThreadPoolExecutor(max_workers=32)" << endl;
    }
    if (gen_asyncio_) {
      f_service_ << indent() << "self._loop = loop or asyncio.get_event_loop()"
                 << endl;
    }

    f_service_ << indent() << "self._processMap = {}" << endl
               << indent() << "self._priorityMap = {}" << endl;
  } else {
    if (gen_asyncio_) {
      f_service_ << indent() << extends << "." << class_prefix
                 << "Processor.__init__(self, handler, loop)" << endl;
    } else {
      f_service_ << indent() << extends << "." << class_prefix
                 << "Processor.__init__(self, handler)" << endl;
    }
  }
  auto service_priority = get_priority(tservice);
  for (auto f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    auto function_prio = get_priority(*f_iter, service_priority);
    f_service_ << indent() << "self._processMap["
               << render_string((*f_iter)->name()) << "] = " << class_prefix
               << "Processor." << (gen_future_ ? "future_process_" : "process_")
               << (*f_iter)->name() << endl
               << indent() << "self._priorityMap["
               << render_string((*f_iter)->name()) << "] = " << "TPriority."
               << function_prio << endl;
  }
  indent_down();
  f_service_ << endl;

  f_service_ << indent() << "def onewayMethods(self):" << endl;
  indent_up();
  f_service_ << indent() << "l = []" << endl;
  if (!extends.empty()) {
    f_service_ << indent() << "l.extend(" << extends << "." << class_prefix
               << "Processor.onewayMethods(self))" << endl;
  }
  f_service_ << indent() << "l.extend(" << class_prefix
             << "Processor._onewayMethods)" << endl
             << indent() << "return tuple(l)" << endl
             << endl;
  indent_down();

  // Generate the server implementation
  if (gen_asyncio_) {
    indent(f_service_) << "@thrift_process_main(asyncio=True)" << endl;
  } else if (gen_future_) {
    indent(f_service_) << "@future_process_main()" << endl;
  } else {
    indent(f_service_) << "@thrift_process_main()" << endl;
  }
  indent(f_service_) << "def process(self,): pass" << endl << endl;

  // Generate the process subfunctions
  for (auto f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (gen_future_) {
      generate_process_function(tservice, *f_iter, false, true);
    } else {
      generate_process_function(tservice, *f_iter, with_context, false);
    }
  }

  indent_down();

  f_service_ << indent() << class_prefix
             << "Iface._processor_type = " << class_prefix << "Processor"
             << endl;

  f_service_ << endl;
}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_py_generator::generate_process_function(
    const t_service* /*tservice*/,
    const t_function* tfunction,
    bool with_context,
    bool future) {
  const string& fn_name = tfunction->name();

  // Open function
  if (future) {
    indent(f_service_) << "def then_" << fn_name
                       << "(self, args, handler_ctx):" << endl;
  } else {
    indent(f_service_) << "@thrift_process_method(" << fn_name << "_args, "
                       << "oneway="
                       << (tfunction->qualifier() ==
                                   t_function_qualifier::oneway
                               ? "True"
                               : "False")
                       << (gen_asyncio_ ? ", asyncio=True" : "") << ")" << endl;

    f_service_ << indent() << "def process_" << fn_name
               << "(self, args, handler_ctx"
               << (gen_asyncio_ ? ", seqid, oprot, fn_name):" : "):") << endl;
  }
  indent_up();

  // Declare result for non oneway function
  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    f_service_ << indent() << "result = " << fn_name + "_result()" << endl;
  }

  if (gen_asyncio_) {
    const t_paramlist& arg_struct = tfunction->params();
    string handler =
        "self._handler." + rename_reserved_keywords(tfunction->name());

    string args_list;
    bool first = true;
    if (with_context) {
      args_list += "handler_ctx";
      first = false;
    }
    for (const t_field& f_iter : arg_struct.fields()) {
      if (first) {
        first = false;
      } else {
        args_list += ", ";
      }
      args_list += "args.";
      args_list += rename_reserved_keywords(f_iter.name());
    }

    f_service_ << indent() << "if should_run_on_thread(" << handler
               << "):" << endl
               << indent() << "  fut = self._loop.run_in_executor(None, "
               << handler << ", " << args_list << ")" << endl
               << indent() << "else:" << endl
               << indent() << "  fut = call_as_future(" << handler
               << ", self._loop, " << args_list << ")" << endl;

    if (tfunction->qualifier() != t_function_qualifier::oneway) {
      string known_exceptions = "{";
      int exc_num = 0;
      for (const t_field& x : get_elems(tfunction->exceptions())) {
        if (exc_num++ > 0) {
          known_exceptions += ", ";
        }
        known_exceptions += "'";
        known_exceptions += rename_reserved_keywords(x.name());
        known_exceptions += "': ";
        known_exceptions += type_name(x.type().get_type());
      }
      known_exceptions += "}";

      f_service_
          << indent() << "fut.add_done_callback("
          << "lambda f: write_results_after_future("
          << "result, self._event_handler, handler_ctx, seqid, oprot, fn_name, "
          << known_exceptions + ", f))" << endl;
    }
    f_service_ << indent() << "return fut" << endl;
    indent_down();
    f_service_ << endl;
  } else {
    // Try block to wrap call to handler
    f_service_ << indent() << "try:" << endl;
    indent_up();

    // Generate the function call
    const t_paramlist& arg_struct = tfunction->params();
    string handler = (future ? "self._handler.future_" : "self._handler.") +
        rename_reserved_keywords(tfunction->name());

    f_service_ << indent();

    if (tfunction->qualifier() != t_function_qualifier::oneway &&
        !tfunction->return_type()->is_void()) {
      f_service_ << "result.success = ";
    }
    f_service_ << handler << "(";
    bool first = true;
    if (with_context) {
      f_service_ << "handler_ctx";
      first = false;
    }
    for (const t_field& f_iter : arg_struct.fields()) {
      if (first) {
        first = false;
      } else {
        f_service_ << ", ";
      }
      f_service_ << "args." << rename_reserved_keywords(f_iter.name());
    }
    f_service_ << ")" << (future ? ".result()" : "") << endl;

    indent_down();
    int exc_num = 0;
    for (const t_field& x : get_elems(tfunction->exceptions())) {
      f_service_ << indent() << "except " << type_name(x.type().get_type())
                 << " as exc" << exc_num << ":" << endl;
      if (tfunction->qualifier() != t_function_qualifier::oneway) {
        indent_up();
        f_service_ << indent()
                   << "self._event_handler.handlerException(handler_ctx, '"
                   << fn_name << "', exc" << exc_num << ")" << endl
                   << indent() << "result."
                   << rename_reserved_keywords(x.name()) << " = exc" << exc_num
                   << endl;
        indent_down();
      } else {
        f_service_ << indent() << "pass" << endl;
      }
      ++exc_num;
    }
    f_service_ << indent() << "except:" << endl
               << indent() << "  ex = sys.exc_info()[1]" << endl
               << indent()
               << "  self._event_handler.handlerError(handler_ctx, '" << fn_name
               << "', ex)" << endl
               << indent() << "  result = Thrift.TApplicationException(message="
               << "repr(ex))" << endl;
    if (tfunction->qualifier() != t_function_qualifier::oneway) {
      f_service_ << indent() << "return result" << endl;
    }

    // Close function
    indent_down();

    if (future) {
      f_service_ << endl;

      f_service_ << indent() << "@future_process_method(" << fn_name
                 << "_args, oneway="
                 << (tfunction->qualifier() == t_function_qualifier::oneway
                         ? "True"
                         : "False")
                 << ")" << endl;

      f_service_ << indent() << "def future_process_" << fn_name
                 << "(self, args, handler_ctx):" << endl;

      indent_up();
      f_service_ << indent() << "return self._executor.submit(self.then_"
                 << fn_name << ", args, handler_ctx)" << endl;
      indent_down();
    }

    f_service_ << endl;
  }
}

/**
 * Deserializes a field of any type.
 */
void t_py_generator::generate_deserialize_field(
    ofstream& out,
    const t_field* tfield,
    const string& prefix,
    bool /*inclass*/,
    const string& /* actual_type */) {
  const t_type* type = tfield->type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->name());
  }

  string name = prefix + rename_reserved_keywords(tfield->name());

  if (type->is<t_structured>()) {
    generate_deserialize_struct(out, (t_struct*)type, name);
  } else if (type->is<t_container>()) {
    generate_deserialize_container(out, type, name);
  } else if (type->is<t_primitive_type>() || type->is<t_enum>()) {
    indent(out) << name << " = iprot.";

    if (const auto* primitive = type->try_as<t_primitive_type>()) {
      switch (primitive->primitive_type()) {
        case t_primitive_type::type::t_void:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);
        case t_primitive_type::type::t_string:
          out << "readString().decode('utf-8') "
              << "if UTF8STRINGS else iprot.readString()";
          break;
        case t_primitive_type::type::t_binary:
          out << "readString()";
          break;
        case t_primitive_type::type::t_bool:
          out << "readBool()";
          break;
        case t_primitive_type::type::t_byte:
          out << "readByte()";
          break;
        case t_primitive_type::type::t_i16:
          out << "readI16()";
          break;
        case t_primitive_type::type::t_i32:
          out << "readI32()";
          break;
        case t_primitive_type::type::t_i64:
          out << "readI64()";
          break;
        case t_primitive_type::type::t_double:
          out << "readDouble()";
          break;
        case t_primitive_type::type::t_float:
          out << "readFloat()";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no Python name for base type " +
              primitive->name());
      }
    } else if (type->is<t_enum>()) {
      out << "readI32()";
    }
    out << endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n",
        tfield->name().c_str(),
        type->name().c_str());
  }
  if (const auto* adapter = get_py_adapter(tfield->type().get_type())) {
    indent(out) << name << " = " << *adapter << ".from_thrift(" << name << ")"
                << endl;
  }
}

/**
 * Generates an unserializer for a struct, calling read()
 */
void t_py_generator::generate_deserialize_struct(
    ofstream& out, const t_struct* tstruct, const string& prefix) {
  out << indent() << prefix << " = " << type_name(tstruct) << "()" << endl
      << indent() << prefix << ".read(iprot)" << endl;
}

/**
 * Serialize a container by writing out the header followed by
 * data and then a footer.
 */
void t_py_generator::generate_deserialize_container(
    ofstream& out, const t_type* ttype, const string& prefix) {
  string size = tmp("_size");
  string ktype = tmp("_ktype");
  string vtype = tmp("_vtype");
  string etype = tmp("_etype");

  t_field fsize(t_primitive_type::t_i32(), size);
  t_field fktype(t_primitive_type::t_byte(), ktype);
  t_field fvtype(t_primitive_type::t_byte(), vtype);
  t_field fetype(t_primitive_type::t_byte(), etype);

  // Declare variables, read header
  if (ttype->is<t_map>()) {
    out << indent() << prefix << " = {}" << endl
        << indent() << "(" << ktype << ", " << vtype << ", " << size
        << " ) = iprot.readMapBegin() " << endl;
  } else if (ttype->is<t_set>()) {
    out << indent() << prefix << " = set()" << endl
        << indent() << "(" << etype << ", " << size
        << ") = iprot.readSetBegin()" << endl;
  } else if (ttype->is<t_list>()) {
    out << indent() << prefix << " = []" << endl
        << indent() << "(" << etype << ", " << size
        << ") = iprot.readListBegin()" << endl;
  }

  // For loop iterates over elements
  string i = tmp("_i");
  indent(out) << "if " << size << " >= 0:" << endl
              << indent() << "  for " << i << " in range(" << size
              << "):" << endl;

  indent_up();
  indent_up();

  if (const t_map* map = ttype->try_as<t_map>()) {
    generate_deserialize_map_element(out, map, prefix, ktype, vtype);
  } else if (const t_set* set = ttype->try_as<t_set>()) {
    generate_deserialize_set_element(out, set, prefix);
  } else if (const t_list* list = ttype->try_as<t_list>()) {
    generate_deserialize_list_element(out, list, prefix);
  }

  indent_down();
  indent_down();

  indent(out) << "else: " << endl;
  if (ttype->is<t_map>()) {
    out << indent() << "  while iprot.peekMap():" << endl;
  } else if (ttype->is<t_set>()) {
    out << indent() << "  while iprot.peekSet():" << endl;
  } else if (ttype->is<t_list>()) {
    out << indent() << "  while iprot.peekList():" << endl;
  }

  indent_up();
  indent_up();

  if (const t_map* map = ttype->try_as<t_map>()) {
    generate_deserialize_map_element(out, map, prefix, ktype, vtype);
  } else if (const t_set* set = ttype->try_as<t_set>()) {
    generate_deserialize_set_element(out, set, prefix);
  } else if (const t_list* list = ttype->try_as<t_list>()) {
    generate_deserialize_list_element(out, list, prefix);
  }

  indent_down();
  indent_down();

  // Read container end
  if (ttype->is<t_map>()) {
    indent(out) << "iprot.readMapEnd()" << endl;
  } else if (ttype->is<t_set>()) {
    indent(out) << "iprot.readSetEnd()" << endl;
  } else if (ttype->is<t_list>()) {
    indent(out) << "iprot.readListEnd()" << endl;
  }
}

/**
 * Generates code to deserialize a map
 */
void t_py_generator::generate_deserialize_map_element(
    ofstream& out,
    const t_map* tmap,
    const string& prefix,
    string key_actual_type,
    string value_actual_type) {
  string key = tmp("_key");
  string val = tmp("_val");
  t_field fkey(tmap->key_type().deref(), key);
  t_field fval(tmap->val_type().deref(), val);

  generate_deserialize_field(out, &fkey, "", false, std::move(key_actual_type));
  generate_deserialize_field(
      out, &fval, "", false, std::move(value_actual_type));

  indent(out) << prefix << "[" << key << "] = " << val << endl;
}

/**
 * Write a set element
 */
void t_py_generator::generate_deserialize_set_element(
    ofstream& out, const t_set* tset, const string& prefix) {
  string elem = tmp("_elem");
  t_field felem(*tset->elem_type(), elem);

  generate_deserialize_field(out, &felem);

  indent(out) << prefix << ".add(" << elem << ")" << endl;
}

/**
 * Write a list element
 */
void t_py_generator::generate_deserialize_list_element(
    ofstream& out, const t_list* tlist, const string& prefix) {
  string elem = tmp("_elem");
  t_field felem(*tlist->elem_type(), elem);

  generate_deserialize_field(out, &felem);

  indent(out) << prefix << ".append(" << elem << ")" << endl;
}

/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param prefix Name to prepend to field name
 */
void t_py_generator::generate_serialize_field(
    ofstream& out, const t_field* tfield, const string& prefix) {
  const t_type* type = tfield->type()->get_true_type();

  // Do nothing for void types
  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->name());
  }
  string name = prefix + rename_reserved_keywords(tfield->name());
  if (const auto* adapter = get_py_adapter(tfield->type().get_type())) {
    string real_name = std::move(name);
    name = tmp("adpt");
    indent(out) << name << " = " << *adapter << ".to_thrift(" << real_name
                << ")" << endl;
  }
  if (type->is<t_structured>()) {
    generate_serialize_struct(out, (t_struct*)type, name);
  } else if (type->is<t_container>()) {
    generate_serialize_container(out, type, name);
  } else if (type->is<t_primitive_type>() || type->is<t_enum>()) {
    indent(out) << "oprot.";

    if (const auto* primitive = type->try_as<t_primitive_type>()) {
      switch (primitive->primitive_type()) {
        case t_primitive_type::type::t_void:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);
        case t_primitive_type::type::t_string:
          out << "writeString(" << name << ".encode('utf-8')) "
              << "if UTF8STRINGS and not isinstance(" << name << ", bytes) "
              << "else oprot.writeString(" << name << ")";
          break;
        case t_primitive_type::type::t_binary:
          out << "writeString(" << name << ")";
          break;
        case t_primitive_type::type::t_bool:
          out << "writeBool(" << name << ")";
          break;
        case t_primitive_type::type::t_byte:
          out << "writeByte(" << name << ")";
          break;
        case t_primitive_type::type::t_i16:
          out << "writeI16(" << name << ")";
          break;
        case t_primitive_type::type::t_i32:
          out << "writeI32(" << name << ")";
          break;
        case t_primitive_type::type::t_i64:
          out << "writeI64(" << name << ")";
          break;
        case t_primitive_type::type::t_double:
          out << "writeDouble(" << name << ")";
          break;
        case t_primitive_type::type::t_float:
          out << "writeFloat(" << name << ")";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no Python name for base type " +
              primitive->name());
      }
    } else if (type->is<t_enum>()) {
      out << "writeI32(" << name << ")";
    }
    out << endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO SERIALIZE FIELD '%s%s' TYPE '%s'\n",
        prefix.c_str(),
        tfield->name().c_str(),
        type->name().c_str());
  }
}

/**
 * Serializes all the members of a struct.
 *
 * @param tstruct The struct to serialize
 * @param prefix  String prefix to attach to all fields
 */
void t_py_generator::generate_serialize_struct(
    ofstream& out, const t_struct* /*tstruct*/, const string& prefix) {
  indent(out) << prefix << ".write(oprot)" << endl;
}

void t_py_generator::generate_serialize_container(
    ofstream& out, const t_type* ttype, const string& prefix) {
  if (const t_map* map = ttype->try_as<t_map>()) {
    indent(out) << "oprot.writeMapBegin("
                << type_to_enum(&map->key_type().deref()) << ", "
                << type_to_enum(&map->val_type().deref()) << ", " << "len("
                << prefix << "))" << endl;
  } else if (const t_set* set = ttype->try_as<t_set>()) {
    indent(out) << "oprot.writeSetBegin("
                << type_to_enum(set->elem_type().get_type()) << ", " << "len("
                << prefix << "))" << endl;
  } else if (const t_list* list = ttype->try_as<t_list>()) {
    indent(out) << "oprot.writeListBegin("
                << type_to_enum(list->elem_type().get_type()) << ", " << "len("
                << prefix << "))" << endl;
  }

  if (const t_map* map = ttype->try_as<t_map>()) {
    string kiter = tmp("kiter");
    string viter = tmp("viter");
    if (sort_keys_) {
      string sorted = tmp("sorted");
      indent(out) << sorted << " = " << prefix << ".items()" << endl;
      string tuple = tmp("tuple");
      indent(out) << sorted << " = sorted(" << sorted << ", key=lambda "
                  << tuple << ": " << tuple << "[0])" << endl;
      indent(out) << "for " << kiter << "," << viter << " in " << sorted << ":"
                  << endl;
    } else {
      indent(out) << "for " << kiter << "," << viter << " in " << prefix
                  << ".items():" << endl;
    }
    indent_up();
    generate_serialize_map_element(out, map, kiter, viter);
    indent_down();
  } else if (const t_set* set = ttype->try_as<t_set>()) {
    string iter = tmp("iter");
    if (sort_keys_) {
      indent(out) << "for " << iter << " in sorted(" << prefix << "):" << endl;
    } else {
      indent(out) << "for " << iter << " in " << prefix << ":" << endl;
    }
    indent_up();
    generate_serialize_set_element(out, set, iter);
    indent_down();
  } else if (const t_list* list = ttype->try_as<t_list>()) {
    string iter = tmp("iter");
    indent(out) << "for " << iter << " in " << prefix << ":" << endl;
    indent_up();
    generate_serialize_list_element(out, list, iter);
    indent_down();
  }

  if (ttype->is<t_map>()) {
    indent(out) << "oprot.writeMapEnd()" << endl;
  } else if (ttype->is<t_set>()) {
    indent(out) << "oprot.writeSetEnd()" << endl;
  } else if (ttype->is<t_list>()) {
    indent(out) << "oprot.writeListEnd()" << endl;
  }
}

/**
 * Serializes the members of a map.
 *
 */
void t_py_generator::generate_serialize_map_element(
    ofstream& out, const t_map* tmap, string kiter, string viter) {
  t_field kfield(tmap->key_type().deref(), std::move(kiter));
  generate_serialize_field(out, &kfield, "");

  t_field vfield(tmap->val_type().deref(), std::move(viter));
  generate_serialize_field(out, &vfield, "");
}

/**
 * Serializes the members of a set.
 */
void t_py_generator::generate_serialize_set_element(
    ofstream& out, const t_set* tset, string iter) {
  t_field efield(*tset->elem_type(), std::move(iter));
  generate_serialize_field(out, &efield, "");
}

/**
 * Serializes the members of a list.
 */
void t_py_generator::generate_serialize_list_element(
    ofstream& out, const t_list* tlist, string iter) {
  t_field efield(*tlist->elem_type(), std::move(iter));
  generate_serialize_field(out, &efield, "");
}

/**
 * Generates the docstring for a given struct.
 */
void t_py_generator::generate_python_docstring(
    ofstream& out, const t_structured* tstruct) {
  generate_python_docstring(out, tstruct, tstruct, "Attributes");
}

/**
 * Generates the docstring for a given function.
 */
void t_py_generator::generate_python_docstring(
    ofstream& out, const t_function* tfunction) {
  generate_python_docstring(out, tfunction, &tfunction->params(), "Parameters");
}

/**
 * Generates the docstring for a struct or function.
 */
void t_py_generator::generate_python_docstring(
    ofstream& out,
    const t_named* named_node,
    const t_structured* tstruct,
    const char* subheader) {
  bool has_doc = false;
  stringstream ss;
  if (named_node->has_doc()) {
    has_doc = true;
    ss << named_node->doc();
  }

  if (tstruct->has_fields()) {
    if (has_doc) {
      ss << endl;
    }
    has_doc = true;
    ss << subheader << ":\n";
    for (const t_field& p : tstruct->fields()) {
      if (is_hidden(p)) {
        continue;
      }
      ss << " - " << rename_reserved_keywords(p.name());
      if (p.has_doc()) {
        ss << ": " << p.doc();
      } else {
        ss << endl;
      }
    }
  }

  if (has_doc) {
    generate_docstring_comment(out, "r\"\"\"\n", "", ss.str(), "\"\"\"\n");
  }
}

/**
 * Generates the docstring for a generic object.
 */
void t_py_generator::generate_python_docstring(
    ofstream& out, const t_named* named_node) {
  if (named_node->has_doc()) {
    generate_docstring_comment(
        out, "r\"\"\"\n", "", named_node->doc(), "\"\"\"\n");
  }
}

/**
 * Declares an argument, which may include initialization as necessary.
 *
 * @param tfield The field
 */
string t_py_generator::declare_argument(
    const t_structured* tstruct, const t_field* tfield) {
  std::ostringstream result;
  result << rename_reserved_keywords(tfield->name()) << "=";
  if (tfield->default_value() != nullptr) {
    result << rename_reserved_keywords(tstruct->name()) << ".thrift_spec["
           << get_thrift_spec_key(tstruct, tfield) << "][4]";
  } else {
    result << "None";
  }
  return result.str();
}

/**
 * Renders a field default value, returns None otherwise.
 *
 * @param tfield The field
 */
string t_py_generator::render_field_default_value(const t_field* tfield) {
  const t_type* type = tfield->type()->get_true_type();
  if (tfield->default_value() != nullptr) {
    return render_const_value(type, tfield->default_value());
  } else {
    return "None";
  }
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_py_generator::function_signature(
    const t_function* tfunction, const string& prefix) {
  // TODO(mcslee): Nitpicky, no ',' if argument_list is empty
  return prefix + rename_reserved_keywords(tfunction->name()) + "(self, " +
      argument_list(tfunction->params()) + ")";
}

/**
 * Renders an interface function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_py_generator::function_signature_if(
    const t_function* tfunction, bool with_context, const string& prefix) {
  // TODO(mcslee): Nitpicky, no ',' if argument_list is empty
  string signature = prefix + rename_reserved_keywords(tfunction->name()) + "(";
  signature += "self, ";
  if (with_context) {
    signature += "handler_ctx, ";
  }
  signature += argument_list(tfunction->params()) + ")";
  return signature;
}

/**
 * Renders a field list
 */
string t_py_generator::argument_list(const t_paramlist& tparamlist) {
  string result;
  bool first = true;
  for (const t_field& f_iter : tparamlist.fields()) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }
    result += rename_reserved_keywords(f_iter.name());
    result += "=" + render_field_default_value(&f_iter);
  }
  return result;
}

string t_py_generator::type_name(const t_type* ttype) {
  const t_program* program = ttype->program();
  if (program != nullptr && program != program_ &&
      !program_->includes().empty()) {
    return get_real_py_module(program) + ".ttypes." +
        rename_reserved_keywords(ttype->name());
  }
  return rename_reserved_keywords(ttype->name());
}

/**
 * Converts the parse type to a Python type
 */
string t_py_generator::type_to_enum(const t_type* type) {
  type = type->get_true_type();

  if (const auto* primitive = type->try_as<t_primitive_type>()) {
    switch (primitive->primitive_type()) {
      case t_primitive_type::type::t_void:
        throw std::runtime_error("NO T_VOID CONSTRUCT");
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        return "TType.STRING";
      case t_primitive_type::type::t_bool:
        return "TType.BOOL";
      case t_primitive_type::type::t_byte:
        return "TType.BYTE";
      case t_primitive_type::type::t_i16:
        return "TType.I16";
      case t_primitive_type::type::t_i32:
        return "TType.I32";
      case t_primitive_type::type::t_i64:
        return "TType.I64";
      case t_primitive_type::type::t_double:
        return "TType.DOUBLE";
      case t_primitive_type::type::t_float:
        return "TType.FLOAT";
    }
  } else if (type->is<t_enum>()) {
    return "TType.I32";
  } else if (type->is<t_structured>()) {
    return "TType.STRUCT";
  } else if (type->is<t_map>()) {
    return "TType.MAP";
  } else if (type->is<t_set>()) {
    return "TType.SET";
  } else if (type->is<t_list>()) {
    return "TType.LIST";
  }

  throw std::runtime_error("INVALID TYPE IN type_to_enum: " + type->name());
}

/** See the comment inside generate_py_struct_definition for what this is. */
string t_py_generator::type_to_spec_args(const t_type* ttype) {
  const auto* adapter = get_py_adapter(ttype); // Do this before get_true_type.
  ttype = ttype->get_true_type();

  if (const auto* primitive = ttype->try_as<t_primitive_type>()) {
    t_primitive_type::type tbase = primitive->primitive_type();
    if (tbase == t_primitive_type::type::t_string) {
      return "True";
    } else if (tbase == t_primitive_type::type::t_binary) {
      return "False";
    }
    return "None";
  } else if (ttype->is<t_enum>()) {
    return type_name(ttype);
  } else if (ttype->is<t_exception>()) {
    return "[" + type_name(ttype) + ", " + type_name(ttype) +
        ".thrift_spec, False]";
  } else if (ttype->is<t_structured>()) {
    string ret = "[" + type_name(ttype) + ", " + type_name(ttype) +
        ".thrift_spec, " + (ttype->is<t_union>() ? "True" : "False");
    if (adapter) {
      ret += ", " + *adapter;
    }
    return ret + "]";
  } else if (const t_map* tmap = ttype->try_as<t_map>()) {
    return std::string("(") + type_to_enum(&tmap->key_type().deref()) + "," +
        type_to_spec_args(&tmap->key_type().deref()) + "," +
        type_to_enum(&tmap->val_type().deref()) + "," +
        type_to_spec_args(&tmap->val_type().deref()) + ")";

  } else if (const t_set* set = ttype->try_as<t_set>()) {
    return "(" + type_to_enum(set->elem_type().get_type()) + "," +
        type_to_spec_args(set->elem_type().get_type()) + ")";

  } else if (const t_list* list = ttype->try_as<t_list>()) {
    return "(" + type_to_enum(list->elem_type().get_type()) + "," +
        type_to_spec_args(list->elem_type().get_type()) + ")";
  }

  throw std::runtime_error(
      "INVALID TYPE IN type_to_spec_args: " + ttype->name());
}

/**
 * Gets the priority annotation of an object (service / function)
 */
std::string t_py_generator::get_priority(
    const t_named* obj, const std::string& def) {
  if (obj) {
    if (auto* val = obj->find_structured_annotation_or_null(kPriorityUri)) {
      return val->get_value_from_structured_annotation("level")
          .get_enum_value()
          ->name();
    }
    return obj->get_unstructured_annotation("priority", &def);
  }
  return def;
}

/**
 * Returns the functions that are supported, leaving unsupported functions
 * (e.g. stream and sink functions).
 */
const std::vector<const t_function*>& t_py_generator::get_functions(
    const t_service* tservice) {
  auto name = tservice->get_full_name();
  auto found = func_map_.find(name);
  if (found != func_map_.end()) {
    return found->second;
  }
  std::vector<const t_function*> funcs;
  for (const auto& func : tservice->functions()) {
    if (!func.sink_or_stream() && !func.is_interaction_constructor()) {
      funcs.push_back(&func);
    }
  }
  auto inserted = func_map_.emplace(name, std::move(funcs));
  return inserted.first->second;
}

int32_t t_py_generator::get_thrift_spec_key(
    const t_structured* s, const t_field* f) {
  // If struct contains negative ids, e.g. for following struct
  //
  // struct NegativeId {
  //   -2: i32 field1 = 1;
  //   2: i32 field2 = 2;
  // }
  //
  // The generated thrift_sepc looks like this
  //
  // NegativeId.thrift_spec = (
  //   (-2, TType.I32, 'field2', None, 2, 2, ), # -2
  //   None, # -1,
  //   None, # 0
  //   None, # 1,
  //   (2, TType.I32, 'field3', None, 3, 2, ), # 2
  // )
  //
  // In this case, to get thrift_spec of corresponding field, we need to add
  // offset to field id: `thrift_spec[id + offset]`
  const int32_t smallest_id = s->fields_id_order()[0]->id();
  const int32_t offset = -std::min(smallest_id, 0);
  return f->id() + offset;
}

THRIFT_REGISTER_GENERATOR(
    py,
    "Python",
    "    json:            Generate function to parse entity from json\n"
    "    slots:           Generate code using slots for instance members.\n"
    "    sort_keys:       Serialize maps sorted by key and sets by value.\n"
    "    thrift_port=NNN: Default port to use in remote client (default "
    "9090).\n"
    "    asyncio:         Generate asyncio-friendly RPC services.\n"
    "    utf8strings:     Encode/decode strings using utf8 in the generated "
    "code.\n");

} // namespace

} // namespace apache::thrift::compiler
