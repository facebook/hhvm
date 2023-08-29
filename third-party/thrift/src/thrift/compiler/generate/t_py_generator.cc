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
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/detail/system.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/t_concat_generator.h>
#include <thrift/compiler/generate/t_generator.h>
#include <thrift/compiler/lib/py3/util.h>

using namespace std;

namespace apache {
namespace thrift {
namespace compiler {

namespace {

const std::string* get_py_adapter(const t_type* type) {
  if (!type->get_true_type()->is_struct()) {
    return nullptr;
  }
  return t_typedef::get_first_annotation_or_null(type, {"py.adapter"});
}

void mark_file_executable(const boost::filesystem::path& path) {
  namespace fs = boost::filesystem;
  fs::permissions(
      path, fs::add_perms | fs::owner_exe | fs::group_exe | fs::others_exe);
}

string prefix_temporary(const string& name) {
  return "_fbthrift_" + name;
}
} // namespace

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
  void generate_struct(const t_struct* tstruct) override;
  void generate_forward_declaration(const t_struct* tstruct) override;
  void generate_xception(const t_struct* txception) override;
  void generate_service(const t_service* tservice) override;

  std::string render_const_value(
      const t_type* type, const t_const_value* value);

  /**
   * Struct generation code
   */

  void generate_py_struct(const t_struct* tstruct, bool is_exception);
  void generate_py_thrift_spec(
      std::ofstream& out, const t_struct* tstruct, bool is_exception);
  void generate_py_annotation_dict(
      std::ofstream& out,
      const std::map<std::string, annotation_value>& fields);
  void generate_py_annotations(std::ofstream& out, const t_struct* tstruct);
  void generate_py_union(std::ofstream& out, const t_struct* tstruct);
  void generate_py_struct_definition(
      std::ofstream& out,
      const t_struct* tstruct,
      bool is_exception = false,
      bool is_result = false);
  void generate_py_struct_reader(std::ofstream& out, const t_struct* tstruct);
  void generate_py_struct_writer(std::ofstream& out, const t_struct* tstruct);
  void generate_py_function_helpers(const t_function* tfunction);
  void generate_py_converter_helpers(
      std::ofstream& out, const t_struct* tstruct);

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
      std::string prefix = "",
      bool inclass = false,
      std::string actual_type = "");

  void generate_deserialize_struct(
      std::ofstream& out, const t_struct* tstruct, std::string prefix = "");

  void generate_deserialize_container(
      std::ofstream& out, const t_type* ttype, std::string prefix = "");

  void generate_deserialize_set_element(
      std::ofstream& out, const t_set* tset, std::string prefix = "");

  void generate_deserialize_map_element(
      std::ofstream& out,
      const t_map* tmap,
      std::string prefix = "",
      std::string key_actual_type = "",
      std::string value_actual_type = "");

  void generate_deserialize_list_element(
      std::ofstream& out, const t_list* tlist, std::string prefix = "");

  void generate_serialize_field(
      std::ofstream& out, const t_field* tfield, std::string prefix = "");

  void generate_serialize_struct(
      std::ofstream& out, const t_struct* tstruct, std::string prefix = "");

  void generate_serialize_container(
      std::ofstream& out, const t_type* ttype, std::string prefix = "");

  void generate_serialize_map_element(
      std::ofstream& out,
      const t_map* tmap,
      std::string kiter,
      std::string viter);

  void generate_serialize_set_element(
      std::ofstream& out, const t_set* tmap, std::string iter);

  void generate_serialize_list_element(
      std::ofstream& out, const t_list* tlist, std::string iter);

  void generate_python_docstring(std::ofstream& out, const t_struct* tstruct);

  void generate_python_docstring(
      std::ofstream& out, const t_function* tfunction);

  void generate_python_docstring(
      std::ofstream& out,
      const t_node* tdoc,
      const t_struct* tstruct,
      const char* subheader);

  void generate_python_docstring(std::ofstream& out, const t_node* tdoc);

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

  void generate_json_reader(std::ofstream& out, const t_struct* tstruct);

  void generate_fastproto_read(std::ofstream& out, const t_struct* tstruct);
  void generate_fastproto_write(std::ofstream& out, const t_struct* tstruct);

  /**
   * Helper rendering functions
   */

  std::string py_autogen_comment();
  std::string py_par_warning(string service_tool_name);
  std::string py_imports();
  std::string rename_reserved_keywords(const std::string& value);
  std::string render_includes();
  std::string render_fastproto_includes();
  std::string declare_argument(const t_struct* tstruct, const t_field* tfield);
  std::string render_field_default_value(const t_field* tfield);
  std::string type_name(const t_type* ttype);
  std::string function_signature(
      const t_function* tfunction, std::string prefix = "");
  std::string function_signature_if(
      const t_function* tfunction, bool with_context, std::string prefix = "");
  std::string argument_list(const t_struct* tstruct);
  std::string type_to_enum(const t_type* ttype);
  std::string type_to_spec_args(const t_type* ttype);
  std::string get_real_py_module(const t_program* program);

  std::string render_string(const std::string& value);
  std::string render_ttype_declarations(const char* delimiter);

  std::string get_priority(
      const t_named* obj, const std::string& def = "NORMAL");
  const std::vector<t_function*>& get_functions(const t_service* tservice);

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

  boost::filesystem::ofstream f_types_;
  boost::filesystem::ofstream f_consts_;
  boost::filesystem::ofstream f_service_;

  boost::filesystem::path package_dir_;

  std::map<std::string, const std::vector<t_function*>> func_map_;

  void generate_json_reader_fn_signature(ofstream& out);
  static int32_t get_thrift_spec_key(const t_struct*, const t_field*);
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
  const t_type* type = tfield->get_type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT READ JSON FIELD WITH void TYPE: " + prefix_thrift +
        tfield->get_name());
  }

  string name = prefix_thrift + rename_reserved_keywords(tfield->get_name()) +
      suffix_thrift;

  if (type->is_struct() || type->is_exception()) {
    generate_json_struct(out, (t_struct*)type, name, prefix_json);
  } else if (type->is_container()) {
    generate_json_container(out, (t_container*)type, name, prefix_json);
  } else if (type->is_enum()) {
    generate_json_enum(out, (t_enum*)type, name, prefix_json);
  } else if (type->is_base_type()) {
    string conversion_function = "";
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    string number_limit = "";
    string number_negative_limit = "";
    switch (tbase) {
      case t_base_type::TYPE_VOID:
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
      case t_base_type::TYPE_BOOL:
        break;
      case t_base_type::TYPE_BYTE:
        number_limit = "0x7f";
        number_negative_limit = "-0x80";
        break;
      case t_base_type::TYPE_I16:
        number_limit = "0x7fff";
        number_negative_limit = "-0x8000";
        break;
      case t_base_type::TYPE_I32:
        number_limit = "0x7fffffff";
        number_negative_limit = "-0x80000000";
        break;
      case t_base_type::TYPE_I64:
        conversion_function = "long";
        break;
      case t_base_type::TYPE_DOUBLE:
      case t_base_type::TYPE_FLOAT:
        conversion_function = "float";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no python reader for base type " +
            t_base_type::t_base_name(tbase) + name);
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
  indent(out)
      << prefix_thrift << ".readFromJson(" << prefix_json
      << ", is_text=False, relax_enum_validation=relax_enum_validation, "
      << "custom_set_cls=set_cls, custom_dict_cls=dict_cls)" << endl;
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
}

void t_py_generator::generate_json_container(
    ofstream& out,
    const t_type* ttype,
    const string& prefix_thrift,
    const string& prefix_json) {
  if (ttype->is_list()) {
    string e = tmp("_tmp_e");
    indent(out) << prefix_thrift << " = []" << endl;

    indent(out) << "for " << e << " in " << prefix_json << ":" << endl;
    indent_up();
    generate_json_collection_element(
        out,
        ((t_list*)ttype)->get_elem_type(),
        prefix_thrift,
        e,
        ".append(",
        ")",
        prefix_json);
    indent_down();
  } else if (ttype->is_set()) {
    string e = tmp("_tmp_e");
    indent(out) << prefix_thrift << " = set_cls()" << endl;

    indent(out) << "for " << e << " in " << prefix_json << ":" << endl;
    indent_up();
    generate_json_collection_element(
        out,
        ((t_set*)ttype)->get_elem_type(),
        prefix_thrift,
        e,
        ".add(",
        ")",
        prefix_json);
    indent_down();
  } else if (ttype->is_map()) {
    string k = tmp("_tmp_k");
    string v = tmp("_tmp_v");
    string kp = tmp("_tmp_kp");
    indent(out) << prefix_thrift << " = dict_cls()" << endl;

    indent(out) << "for " << k << ", " << v << " in " << prefix_json
                << ".items():" << endl;
    indent_up();

    generate_json_map_key(out, ((t_map*)ttype)->get_key_type(), kp, k);

    generate_json_collection_element(
        out,
        ((t_map*)ttype)->get_val_type(),
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

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      // Explicitly cast into float because there is an asymetry
      // between serializing and deserializing NaN.
      case t_base_type::TYPE_DOUBLE:
      case t_base_type::TYPE_FLOAT:
        to_act_on = "float(" + to_act_on + ")";
        break;
      default:
        break;
    }
  } else if (type->is_enum()) {
    to_parse = elem;
    to_act_on = tmp("_enum");
  } else if (type->is_list()) {
    to_parse = elem;
    to_act_on = tmp("_list");
  } else if (type->is_map()) {
    to_parse = elem;
    to_act_on = tmp("_map");
  } else if (type->is_set()) {
    to_parse = elem;
    to_act_on = tmp("_set");
  } else if (type->is_struct()) {
    to_parse = elem;
    to_act_on = tmp("_struct");
  }

  t_field felem(type, to_act_on);
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
  if (type->is_enum()) {
    indent(out) << parsed_key << " = int(" << raw_key << ")" << endl;
  } else if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    string conversion_function = "";
    string number_limit = "";
    string number_negative_limit = "";
    bool generate_assignment = true;
    switch (tbase) {
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        break;
      case t_base_type::TYPE_BOOL:
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
      case t_base_type::TYPE_BYTE:
        conversion_function = "int";
        number_limit = "0x7f";
        number_negative_limit = "-0x80";
        break;
      case t_base_type::TYPE_I16:
        conversion_function = "int";
        number_limit = "0x7fff";
        number_negative_limit = "-0x8000";
        break;
      case t_base_type::TYPE_I32:
        conversion_function = "int";
        number_limit = "0x7fffffff";
        number_negative_limit = "-0x80000000";
        break;
      case t_base_type::TYPE_I64:
        conversion_function = "long";
        break;
      case t_base_type::TYPE_DOUBLE:
      case t_base_type::TYPE_FLOAT:
        conversion_function = "float";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no C++ reader for base type " +
            t_base_type::t_base_name(tbase));
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
  indent(out) << "relax_enum_validation = "
                 "bool(kwargs.pop('relax_enum_validation', False))"
              << endl;
  indent(out) << "set_cls = kwargs.pop('custom_set_cls', set)" << endl;
  indent(out) << "dict_cls = kwargs.pop('custom_dict_cls', dict)" << endl;
  indent(out) << "if kwargs:" << endl;
  indent(out) << "    extra_kwargs = ', '.join(kwargs.keys())" << endl;
  indent(out) << "    raise ValueError(" << endl;
  indent(out) << "        'Unexpected keyword arguments: ' + extra_kwargs"
              << endl;
  indent(out) << "    )" << endl;
}

void t_py_generator::generate_json_reader(
    ofstream& out, const t_struct* tstruct) {
  if (!gen_json_) {
    return;
  }

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  generate_json_reader_fn_signature(out);
  indent(out) << "json_obj = json" << endl;
  indent(out) << "if is_text:" << endl;
  indent_up();
  indent(out) << "json_obj = loads(json)" << endl;
  indent_down();

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    string field = (*f_iter)->get_name();
    indent(out) << "if '" << field << "' in json_obj "
                << "and json_obj['" << field << "'] is not None:" << endl;
    indent_up();
    generate_json_field(
        out, *f_iter, "self.", "", "json_obj['" + (*f_iter)->get_name() + "']");

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
  boost::filesystem::create_directory(package_dir_);
  while (true) {
    boost::filesystem::create_directory(package_dir_);
    boost::filesystem::ofstream init_py(package_dir_ / "__init__.py");
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
  boost::filesystem::ofstream f_init;
  f_init.open(f_init_path);
  record_genfile(f_init_path);
  f_init << py_autogen_comment() << "__all__ = ['ttypes', 'constants'";
  for (const auto* tservice : program_->services()) {
    f_init << ", '" << tservice->get_name() << "'";
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
    out << ", " << delimiter << rename_reserved_keywords(en->get_name())
        << delimiter;
  }
  for (const auto& object : program_->objects()) {
    out << ", " << delimiter << rename_reserved_keywords(object->get_name())
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
  string result = "";
  for (size_t i = 0; i < includes.size(); ++i) {
    result += "import " + get_real_py_module(includes[i]) + ".ttypes\n";
  }
  if (includes.size() > 0) {
    result += "\n";
  }

  set<string> modules;
  for (const auto* strct : program_->structs()) {
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
  return "import pprint\n"
         "import warnings\n"
         "from thrift import Thrift\n"
         "from thrift.transport import TTransport\n"
         "from thrift.protocol import TBinaryProtocol\n"
         "from thrift.protocol import TCompactProtocol\n"
         "from thrift.protocol import THeaderProtocol\n"
         "fastproto = None\n"
         "try:\n"
         "  from thrift.protocol import fastproto\n"
         "except ImportError:\n"
         "  pass\n";
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
string t_py_generator::py_par_warning(string service_tool_name) {
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
  const auto* type = ttypedef->get_type();
  // Typedefs of user-defined types are useful as aliases.  On the other
  // hand, base types are implicit, so it is not as helpful to support
  // creating aliases to their Python analogs.  That said, if you need it,
  // add an `else if` below.
  if (const auto* adapter = get_py_adapter(type)) {
    f_types_ << varname << " = " << *adapter << ".Type" << endl;
  } else if (
      type->is_typedef() || type->is_enum() || type->is_struct() ||
      type->is_exception()) {
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
  std::ostringstream to_string_mapping, from_string_mapping;

  f_types_ << "class " << rename_reserved_keywords(tenum->get_name()) << ":"
           << endl;

  indent_up();
  generate_python_docstring(f_types_, tenum);

  to_string_mapping << indent() << "_VALUES_TO_NAMES = {" << endl;
  from_string_mapping << indent() << "_NAMES_TO_VALUES = {" << endl;

  vector<t_enum_value*> constants = tenum->get_enum_values();
  vector<t_enum_value*>::iterator c_iter;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int32_t value = (*c_iter)->get_value();

    f_types_ << indent() << rename_reserved_keywords((*c_iter)->get_name())
             << " = " << value << endl;

    // Dictionaries to/from string names of enums
    to_string_mapping << indent() << indent() << value << ": \""
                      << (*c_iter)->get_name() << "\"," << endl;
    from_string_mapping << indent() << indent() << '"' << (*c_iter)->get_name()
                        << "\": " << value << ',' << endl;
  }
  to_string_mapping << indent() << "}" << endl;
  from_string_mapping << indent() << "}" << endl;

  indent_down();
  f_types_ << endl;
  f_types_ << to_string_mapping.str() << endl
           << from_string_mapping.str() << endl;
}

/**
 * Generate a constant value
 */
void t_py_generator::generate_const(const t_const* tconst) {
  const t_type* type = tconst->get_type();
  string name = rename_reserved_keywords(tconst->get_name());
  t_const_value* value = tconst->get_value();

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
  std::string wrap(escaped.find("\n") == std::string::npos ? "\"" : "\"\"\"");
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

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        out << render_string(value->get_string());
        break;
      case t_base_type::TYPE_BOOL:
        out << (value->get_integer() > 0 ? "True" : "False");
        break;
      case t_base_type::TYPE_BYTE:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
        out << value->get_integer();
        break;
      case t_base_type::TYPE_DOUBLE:
      case t_base_type::TYPE_FLOAT:
        out << std::showpoint;
        if (value->get_type() == t_const_value::CV_INTEGER) {
          out << value->get_integer();
        } else {
          out << value->get_double();
        }
        break;
      default:
        throw std::runtime_error(
            "compiler error: no const of base type " +
            t_base_type::t_base_name(tbase));
    }
  } else if (type->is_enum()) {
    indent(out) << value->get_integer();
  } else if (type->is_struct() || type->is_exception()) {
    out << rename_reserved_keywords(type_name(type)) << "(**{" << endl;
    indent_up();
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      const t_type* field_type = nullptr;
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if ((*f_iter)->get_name() == v_iter->first->get_string()) {
          field_type = (*f_iter)->get_type();
        }
      }
      if (field_type == nullptr) {
        throw std::runtime_error(
            "type error: " + type->get_name() + " has no field " +
            v_iter->first->get_string());
      }
      out << indent();
      out << render_const_value(&t_base_type::t_string(), v_iter->first);
      out << " : ";
      out << render_const_value(field_type, v_iter->second);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "})";
  } else if (type->is_map()) {
    const t_type* ktype = ((t_map*)type)->get_key_type();
    const t_type* vtype = ((t_map*)type)->get_val_type();
    out << "{" << endl;
    indent_up();
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent();
      out << render_const_value(ktype, v_iter->first);
      out << " : ";
      out << render_const_value(vtype, v_iter->second);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "}";
  } else if (type->is_list() || type->is_set()) {
    const t_type* etype;
    if (type->is_list()) {
      etype = ((t_list*)type)->get_elem_type();
    } else {
      etype = ((t_set*)type)->get_elem_type();
    }
    if (type->is_set()) {
      out << "set(";
    }
    out << "[" << endl;
    indent_up();
    const vector<t_const_value*>& val = value->get_list();
    vector<t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent();
      out << render_const_value(etype, *v_iter);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "]";
    if (type->is_set()) {
      out << ")";
    }
  } else {
    throw std::runtime_error(
        "CANNOT GENERATE CONSTANT FOR TYPE: " + type->get_name());
  }

  return out.str();
}

void t_py_generator::generate_forward_declaration(const t_struct* tstruct) {
  if (!tstruct->is_union()) {
    generate_py_struct(tstruct, tstruct->is_exception());
  } else {
    generate_py_union(f_types_, tstruct);
  }
}

/**
 * Generates a python struct
 */
void t_py_generator::generate_struct(const t_struct* tstruct) {
  generate_py_thrift_spec(f_types_, tstruct, false);
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_py_generator::generate_xception(const t_struct* txception) {
  generate_py_thrift_spec(f_types_, txception, true);
}

/**
 * Generates a python struct
 */
void t_py_generator::generate_py_struct(
    const t_struct* tstruct, bool is_exception) {
  generate_py_struct_definition(f_types_, tstruct, is_exception);
  generate_py_converter_helpers(f_types_, tstruct);
}

/**
 * Generates a python union definition. We just keep a variable `value`
 * which holds the current value and to know type we use instanceof.
 */
void t_py_generator::generate_py_union(ofstream& out, const t_struct* tstruct) {
  const vector<t_field*>& members = tstruct->get_members();
  const vector<t_field*>& sorted_members = tstruct->get_sorted_members();

  out << "class " << rename_reserved_keywords(tstruct->get_name())
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
    indent(out) << uppercase(member->get_name()) << " = " << member->get_key()
                << endl;
  }
  indent(out) << endl;

  // Generate `isUnion` method
  indent(out) << "@staticmethod" << endl;
  indent(out) << "def isUnion():" << endl;
  indent(out) << "  return True" << endl << endl;

  // Generate `get_` methods
  for (auto& member : members) {
    indent(out) << "def get_" << member->get_name() << "(self):" << endl;
    indent(out) << "  assert self.field == " << member->get_key() << endl;
    indent(out) << "  return self.value" << endl << endl;
  }

  // Generate `set_` methods
  for (auto& member : members) {
    indent(out) << "def set_" << member->get_name() << "(self, value):" << endl;
    indent(out) << "  self.field = " << member->get_key() << endl;
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
    auto key = rename_reserved_keywords(member->get_name());
    out << indent() << "  if self.field == " << member->get_key() << ":" << endl
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
    auto t = type_to_enum(member->get_type());
    auto n = member->get_name();
    auto k = member->get_key();
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

  indent(out) << "oprot.writeUnionBegin('" << tstruct->get_name() << "')"
              << endl;

  first = true;
  for (auto& member : sorted_members) {
    auto t = type_to_enum(member->get_type());
    auto n = member->get_name();
    auto k = member->get_key();

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
      auto n = member->get_name();
      indent(out) << "if '" << n << "' in obj:" << endl;
      indent_up();
      generate_json_field(
          out, member, prefix_temporary(""), "", "obj['" + n + "']");
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
    out << indent() << "return "
        << "self.field == other.field and "
        << "self.value == other.value" << endl;
  } else {
    out << indent() << "return "
        << "self.__dict__ == other.__dict__" << endl;
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
    ofstream& out, const t_struct* tstruct, bool /*is_exception*/) {
  const vector<t_field*>& members = tstruct->get_members();
  const vector<t_field*>& sorted_members = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator m_iter;

  indent(out) << "all_structs.append("
              << rename_reserved_keywords(tstruct->get_name()) << ")" << endl
              << rename_reserved_keywords(tstruct->get_name())
              << ".thrift_spec = (" << endl;

  indent_up();

  int sorted_keys_pos = 0;
  for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
       ++m_iter) {
    if (sorted_keys_pos >= 0 && (*m_iter)->get_key() < 0) {
      sorted_keys_pos = (*m_iter)->get_key();
    }

    for (; sorted_keys_pos != (*m_iter)->get_key(); sorted_keys_pos++) {
      indent(out) << "None, # " << sorted_keys_pos << endl;
    }

    indent(out) << "(" << (*m_iter)->get_key() << ", "
                << type_to_enum((*m_iter)->get_type()) << ", "
                << "'" << rename_reserved_keywords((*m_iter)->get_name()) << "'"
                << ", " << type_to_spec_args((*m_iter)->get_type()) << ", "
                << render_field_default_value(*m_iter) << ", "
                << static_cast<int>((*m_iter)->get_req()) << ", ),"
                << " # " << sorted_keys_pos << endl;

    sorted_keys_pos++;
  }

  indent_down();
  indent(out) << ")" << endl << endl;

  generate_py_annotations(out, tstruct);

  if (members.size() > 0) {
    out << indent() << "def " << rename_reserved_keywords(tstruct->get_name())
        << "__init__(self,";
    if (members.size() > 255) {
      out << " **kwargs";
    } else {
      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        // This fills in default values, as opposed to nulls
        out << " " << declare_argument(tstruct, *m_iter) << ",";
      }
    }
    out << "):" << endl;

    indent_up();

    if (members.size() > 255) {
      for (const auto& member : members) {
        indent(out) << rename_reserved_keywords(member->get_name())
                    << " = kwargs.pop(\n";
        indent(out) << "  \"" << rename_reserved_keywords(member->get_name())
                    << "\",\n";
        if (member->get_value() != nullptr) {
          indent(out) << "  " << rename_reserved_keywords(tstruct->get_name())
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
                  << rename_reserved_keywords(tstruct->get_name())
                  << "__init__\", key))\n";
    }

    if (tstruct->is_union()) {
      indent(out) << "self.field = 0" << endl;
      indent(out) << "self.value = None" << endl;

      for (auto& member : sorted_members) {
        indent(out) << "if " << rename_reserved_keywords(member->get_name())
                    << " is not None:" << endl;
        indent(out) << "  assert self.field == 0 and self.value is None"
                    << endl;
        indent(out) << "  self.field = " << member->get_key() << endl;
        indent(out) << "  self.value = "
                    << rename_reserved_keywords(member->get_name()) << endl;
      }
    } else {
      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        // Initialize fields
        const t_type* type = (*m_iter)->get_type();
        if (!type->is_base_type() && !type->is_enum() &&
            (*m_iter)->get_value() != nullptr) {
          indent(out) << "if "
                      << rename_reserved_keywords((*m_iter)->get_name())
                      << " is self.thrift_spec["
                      << get_thrift_spec_key(tstruct, *m_iter)
                      << "][4]:" << endl;
          indent(out) << "  " << rename_reserved_keywords((*m_iter)->get_name())
                      << " = " << render_field_default_value(*m_iter) << endl;
        }
        indent(out) << "self."
                    << rename_reserved_keywords((*m_iter)->get_name()) << " = "
                    << rename_reserved_keywords((*m_iter)->get_name()) << endl;
      }
    }
    indent_down();

    out << endl;
    out << indent() << rename_reserved_keywords(tstruct->get_name())
        << ".__init__ = " << rename_reserved_keywords(tstruct->get_name())
        << "__init__" << endl
        << endl;
  }

  // ThriftStruct.__setstate__: Ensure that unpickled objects have all expected
  // fields.
  if (members.size() > 0 && !tstruct->is_union() && !gen_slots_) {
    out << indent() << "def " << rename_reserved_keywords(tstruct->get_name())
        << "__setstate__(self, state):" << endl;

    indent_up();
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      indent(out) << "state.setdefault('"
                  << rename_reserved_keywords((*m_iter)->get_name()) << "', "
                  << render_field_default_value(*m_iter) << ")" << endl;
    }
    indent(out) << "self.__dict__ = state" << endl;
    indent_down();

    out << endl;
    out << indent() << rename_reserved_keywords(tstruct->get_name())
        << ".__getstate__ = lambda self: self.__dict__.copy()" << endl;
    out << indent() << rename_reserved_keywords(tstruct->get_name())
        << ".__setstate__ = " << rename_reserved_keywords(tstruct->get_name())
        << "__setstate__" << endl
        << endl;
  }
}

void t_py_generator::generate_py_annotation_dict(
    std::ofstream& out, const std::map<std::string, annotation_value>& fields) {
  indent_up();
  for (auto a_iter = fields.begin(); a_iter != fields.end(); ++a_iter) {
    indent(out) << render_string(a_iter->first) << ": "
                << render_string(a_iter->second.value) << ",\n";
  }
  indent_down();
}

void t_py_generator::generate_py_annotations(
    std::ofstream& out, const t_struct* tstruct) {
  const vector<t_field*>& sorted_members = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator m_iter;

  indent(out) << rename_reserved_keywords(tstruct->get_name())
              << ".thrift_struct_annotations = {" << endl;
  generate_py_annotation_dict(out, tstruct->annotations());
  indent(out) << "}" << endl;

  indent(out) << rename_reserved_keywords(tstruct->get_name())
              << ".thrift_field_annotations = {" << endl;
  indent_up();

  for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
       ++m_iter) {
    const t_field* field = *m_iter;
    if (field->annotations().empty()) {
      continue;
    }
    indent(out) << field->get_key() << ": {" << endl;
    generate_py_annotation_dict(out, field->annotations());
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
    const t_struct* tstruct,
    bool is_exception,
    bool /*is_result*/) {
  const vector<t_field*>& members = tstruct->get_members();
  const vector<t_field*>& sorted_members = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator m_iter;

  out << "class " << rename_reserved_keywords(tstruct->get_name());
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
      indent(out) << "'" << rename_reserved_keywords((*m_iter)->get_name())
                  << "'," << endl;
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
    if (const auto* msg = tstruct->find_annotation_or_null("message")) {
      out << indent() << "  if self." << *msg << ":" << endl
          << indent() << "    return self." << *msg << endl
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
      auto key = rename_reserved_keywords(member->get_name());
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
        << "\"\\n\" + \",\\n\".join(L) if L else '')" << endl
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
      out << indent() << "return "
          << "self.__dict__ == other.__dict__ " << endl;
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
        << "\"\\n\" + \",\\n\".join(L) if L else '')" << endl
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
    indent(out) << "'" << rename_reserved_keywords((*m_iter)->get_name())
                << "'," << endl;
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
    ofstream& out, const t_struct* tstruct) {
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
              << (tstruct->is_union() ? "True" : "False") << "], "
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
              << (tstruct->is_union() ? "True" : "False") << "], "
              << "utf8strings=UTF8STRINGS, protoid=2))" << endl;
  indent(out) << "return" << endl;
  indent_down();
}

void t_py_generator::generate_fastproto_read(
    ofstream& out, const t_struct* tstruct) {
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
              << (tstruct->is_union() ? "True" : "False") << "], "
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
              << (tstruct->is_union() ? "True" : "False") << "], "
              << "utf8strings=UTF8STRINGS, protoid=2)" << endl;
  indent(out) << "return" << endl;
  indent_down();
}

/**
 * Generates the read method for a struct
 */
void t_py_generator::generate_py_struct_reader(
    ofstream& out, const t_struct* tstruct) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

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
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
      out << indent() << "if ";
    } else {
      out << indent() << "elif ";
    }
    out << "fid == " << (*f_iter)->get_key() << ":" << endl;
    indent_up();
    indent(out) << "if ftype == " << type_to_enum((*f_iter)->get_type()) << ":"
                << endl;
    indent_up();
    generate_deserialize_field(out, *f_iter, "self.");
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
    ofstream& out, const t_struct* tstruct) {
  string name = tstruct->get_name();
  const vector<t_field*>& fields = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator f_iter;

  indent(out) << "def write(self, oprot):" << endl;
  indent_up();

  generate_fastproto_write(out, tstruct);

  indent(out) << "oprot.writeStructBegin('" << name << "')" << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    // Write field header
    indent(out) << "if self." << rename_reserved_keywords((*f_iter)->get_name())
                << " != None";
    if ((*f_iter)->get_req() == t_field::e_req::optional &&
        (*f_iter)->get_value() != nullptr) {
      // An optional field with a value set should not be serialized if
      // the value equals the default value
      out << " and self." << rename_reserved_keywords((*f_iter)->get_name())
          << " != "
          << "self.thrift_spec[" << get_thrift_spec_key(tstruct, *f_iter)
          << "][4]";
    }
    out << ":" << endl;
    indent_up();
    indent(out) << "oprot.writeFieldBegin("
                << "'" << (*f_iter)->get_name() << "', "
                << type_to_enum((*f_iter)->get_type()) << ", "
                << (*f_iter)->get_key() << ")" << endl;

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

  if (tservice->get_extends() != nullptr) {
    f_service_ << "import "
               << get_real_py_module(tservice->get_extends()->program()) << "."
               << rename_reserved_keywords(tservice->get_extends()->get_name())
               << endl;
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
    const t_struct* ts = (*f_iter)->get_paramlist();
    generate_py_struct_definition(f_service_, ts, false);
    generate_py_thrift_spec(f_service_, ts, false);
    generate_py_function_helpers(*f_iter);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_py_generator::generate_py_function_helpers(const t_function* tfunction) {
  if (tfunction->qualifier() != t_function_qualifier::one_way) {
    t_struct result(
        program_, rename_reserved_keywords(tfunction->get_name()) + "_result");
    auto success =
        std::make_unique<t_field>(tfunction->return_type(), "success", 0);
    if (!tfunction->return_type()->is_void()) {
      result.append(std::move(success));
    }

    for (const t_field& x : get_elems(tfunction->exceptions())) {
      result.append(x.clone_DO_NOT_USE());
    }
    generate_py_struct_definition(f_service_, &result, false, true);
    generate_py_thrift_spec(f_service_, &result, false);
  }
}

void t_py_generator::generate_py_converter_helpers(
    std::ofstream& out, const t_struct* tstruct) {
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
      << rename_reserved_keywords(tstruct->get_name()) << ", self"
      << ")" << endl
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
      << rename_reserved_keywords(tstruct->get_name()) << ", self"
      << ")" << endl
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
  string extends = "";
  string extends_if = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    extends_if = "(" + extends + "." + iface_prefix + "Iface)";
  }

  f_service_ << "class " << iface_prefix << "Iface" << extends_if << ":"
             << endl;
  indent_up();
  generate_python_docstring(f_service_, tservice);
  if (!tservice->annotations().empty()) {
    f_service_ << indent() << "annotations = {" << endl;
    generate_py_annotation_dict(f_service_, tservice->annotations());
    f_service_ << indent() << "}" << endl << endl;
  }
  std::string service_priority = get_priority(tservice);
  const auto& functions = get_functions(tservice);
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
        f_service_ << indent() << "fut.set_result(self."
                   << (*f_iter)->get_name() << "(";
        const vector<t_field*>& fields =
            (*f_iter)->get_paramlist()->get_members();
        for (auto it = fields.begin(); it != fields.end(); ++it) {
          f_service_ << rename_reserved_keywords((*it)->get_name()) << ",";
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
  string extends = "";
  string extends_client = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
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
  vector<t_function*>::const_iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    const t_struct* arg_struct = (*f_iter)->get_paramlist();
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator fld_iter;
    string funname = rename_reserved_keywords((*f_iter)->get_name());
    string argsname = (*f_iter)->get_name() + "_args";

    // Open function
    indent(f_service_) << "def " << function_signature(*f_iter) << ":" << endl;
    indent_up();
    generate_python_docstring(f_service_, (*f_iter));

    // CPP transport
    if (!gen_asyncio_) {
      indent(f_service_) << "if (self._fbthrift_cpp_transport):" << endl;
      indent_up();
      f_service_ << indent() << "args = " << argsname << "()" << endl;
      for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
        f_service_ << indent() << "args."
                   << rename_reserved_keywords((*fld_iter)->get_name()) << " = "
                   << rename_reserved_keywords((*fld_iter)->get_name()) << endl;
      }
      f_service_ << indent()
                 << "result = self._fbthrift_cpp_transport._send_request(\""
                 << tservice->get_name() << "\", \"" << (*f_iter)->get_name()
                 << "\", args, " << (*f_iter)->get_name() << "_result)" << endl;
      if (!(*f_iter)->return_type()->is_void()) {
        f_service_ << indent() << "if result.success is not None:" << endl
                   << indent() << "  return result.success" << endl;
      }

      for (const t_field& ex : get_elems((*f_iter)->exceptions())) {
        auto name = rename_reserved_keywords(ex.get_name());
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
    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      if (first) {
        first = false;
      } else {
        f_service_ << ", ";
      }
      f_service_ << rename_reserved_keywords((*fld_iter)->get_name());
    }
    f_service_ << ")" << endl;

    if ((*f_iter)->qualifier() != t_function_qualifier::one_way) {
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
               << (*f_iter)->get_name() << "', TMessageType.CALL, self._seqid)"
               << endl;

    f_service_ << indent() << "args = " << argsname << "()" << endl;

    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      f_service_ << indent() << "args."
                 << rename_reserved_keywords((*fld_iter)->get_name()) << " = "
                 << rename_reserved_keywords((*fld_iter)->get_name()) << endl;
    }

    std::string flush = (*f_iter)->qualifier() == t_function_qualifier::one_way
        ? "onewayFlush"
        : "flush";
    // Write to the stream
    f_service_ << indent() << "args.write(self._oprot)" << endl
               << indent() << "self._oprot.writeMessageEnd()" << endl
               << indent() << "self._oprot.trans." << flush << "()" << endl;

    indent_down();

    if ((*f_iter)->qualifier() != t_function_qualifier::one_way) {
      std::string resultname = (*f_iter)->get_name() + "_result";
      // Open function
      f_service_ << endl;
      if (gen_asyncio_) {
        f_service_ << indent() << "def recv_" << (*f_iter)->get_name()
                   << "(self, iprot, mtype, rseqid):" << endl;
      } else {
        t_function recv_function(
            (*f_iter)->return_type(),
            string("recv_") + (*f_iter)->get_name(),
            std::make_unique<t_paramlist>(program_));
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
                   << rename_reserved_keywords(x.get_name())
                   << " != None:" << endl;
        if (gen_asyncio_) {
          f_service_ << indent() << "  fut.set_exception(result."
                     << rename_reserved_keywords(x.get_name()) << ")" << endl
                     << indent() << "  return" << endl;
        } else {
          f_service_ << indent() << "  raise result."
                     << rename_reserved_keywords(x.get_name()) << endl;
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
                     << (*f_iter)->get_name() << " failed: unknown result\"))"
                     << endl
                     << indent() << "return" << endl;
        } else {
          f_service_ << indent() << "raise TApplicationException("
                     << "TApplicationException.MISSING_RESULT, \""
                     << (*f_iter)->get_name() << " failed: unknown result\");"
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
  boost::filesystem::ofstream f_remote;
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
      "\n"
           << "from . import " << rename_reserved_keywords(service_name_)
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
       cur_service = cur_service->get_extends()) {
    const string& svc_name = cur_service->get_name();
    const auto& functions = get_functions(cur_service);
    for (vector<t_function*>::const_iterator it = functions.begin();
         it != functions.end();
         ++it) {
      const t_function* fn = *it;
      const string& fn_name = fn->get_name();
      pair<set<string>::iterator, bool> ret = processed_fns.insert(fn_name);
      if (!ret.second) {
        // A child class has overridden this function, so we've listed it
        // already.
        continue;
      }

      f_remote << "    '" << fn_name << "': Function('" << fn_name << "', '"
               << svc_name << "', ";
      if (fn->qualifier() == t_function_qualifier::one_way) {
        f_remote << "None, ";
      } else {
        f_remote << "'" << thrift_type_name(fn->return_type()) << "', ";
      }

      f_remote << "[";
      const vector<t_field*>& args = fn->get_paramlist()->get_members();
      bool first = true;
      for (vector<t_field*>::const_iterator it = args.begin(); it != args.end();
           ++it) {
        if (first) {
          first = false;
        } else {
          f_remote << ", ";
        }
        f_remote << "('" << thrift_type_name((*it)->get_type()) << "', '"
                 << (*it)->get_name() << "', '"
                 << thrift_type_name((*it)->get_type()->get_true_type())
                 << "')";
      }
      f_remote << "]),\n";
    }
  }
  f_remote << "}\n\n";

  // Similar, but for service names
  f_remote << "SERVICE_NAMES = [";
  for (const t_service* cur_service = tservice; cur_service != nullptr;
       cur_service = cur_service->get_extends()) {
    f_remote << "'" << cur_service->get_name() << "', ";
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
  boost::filesystem::ofstream f_fuzzer;
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

  string extends = "";
  string extends_processor = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    extends_processor = extends + "." + class_prefix + "Processor, ";
  }

  // Generate the header portion
  f_service_ << "class " << class_prefix << "Processor(" << extends_processor
             << class_prefix << "Iface, TProcessor):" << endl;

  indent_up();

  f_service_ << indent() << "_onewayMethods = (";
  for (auto f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if ((*f_iter)->qualifier() == t_function_qualifier::one_way) {
      f_service_ << "\"" << (*f_iter)->get_name() << "\",";
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
               << render_string((*f_iter)->get_name()) << "] = " << class_prefix
               << "Processor." << (gen_future_ ? "future_process_" : "process_")
               << (*f_iter)->get_name() << endl
               << indent() << "self._priorityMap["
               << render_string((*f_iter)->get_name()) << "] = "
               << "TPriority." << function_prio << endl;
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
  const string& fn_name = tfunction->get_name();

  // Open function
  if (future) {
    indent(f_service_) << "def then_" << fn_name
                       << "(self, args, handler_ctx):" << endl;
  } else {
    indent(f_service_) << "@thrift_process_method(" << fn_name << "_args, "
                       << "oneway="
                       << (tfunction->qualifier() ==
                                   t_function_qualifier::one_way
                               ? "True"
                               : "False")
                       << (gen_asyncio_ ? ", asyncio=True" : "") << ")" << endl;

    f_service_ << indent() << "def process_" << fn_name
               << "(self, args, handler_ctx"
               << (gen_asyncio_ ? ", seqid, oprot, fn_name):" : "):") << endl;
  }
  indent_up();

  // Declare result for non oneway function
  if (tfunction->qualifier() != t_function_qualifier::one_way) {
    f_service_ << indent() << "result = " << fn_name + "_result()" << endl;
  }

  if (gen_asyncio_) {
    const t_struct* arg_struct = tfunction->get_paramlist();
    const std::vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator f_iter;

    string handler =
        "self._handler." + rename_reserved_keywords(tfunction->get_name());

    string args_list = "";
    bool first = true;
    if (with_context) {
      args_list += "handler_ctx";
      first = false;
    }
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      if (first) {
        first = false;
      } else {
        args_list += ", ";
      }
      args_list += "args.";
      args_list += rename_reserved_keywords((*f_iter)->get_name());
    }

    f_service_ << indent() << "if should_run_on_thread(" << handler
               << "):" << endl
               << indent() << "  fut = self._loop.run_in_executor(None, "
               << handler << ", " << args_list << ")" << endl
               << indent() << "else:" << endl
               << indent() << "  fut = call_as_future(" << handler
               << ", self._loop, " << args_list << ")" << endl;

    if (tfunction->qualifier() != t_function_qualifier::one_way) {
      string known_exceptions = "{";
      int exc_num = 0;
      for (const t_field& x : get_elems(tfunction->exceptions())) {
        if (exc_num++ > 0) {
          known_exceptions += ", ";
        }
        known_exceptions += "'";
        known_exceptions += rename_reserved_keywords(x.get_name());
        known_exceptions += "': ";
        known_exceptions += type_name(x.get_type());
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
    const t_struct* arg_struct = tfunction->get_paramlist();
    const std::vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator f_iter;

    string handler = (future ? "self._handler.future_" : "self._handler.") +
        rename_reserved_keywords(tfunction->get_name());

    f_service_ << indent();

    if (tfunction->qualifier() != t_function_qualifier::one_way &&
        !tfunction->return_type()->is_void()) {
      f_service_ << "result.success = ";
    }
    f_service_ << handler << "(";
    bool first = true;
    if (with_context) {
      f_service_ << "handler_ctx";
      first = false;
    }
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      if (first) {
        first = false;
      } else {
        f_service_ << ", ";
      }
      f_service_ << "args." << rename_reserved_keywords((*f_iter)->get_name());
    }
    f_service_ << ")" << (future ? ".result()" : "") << endl;

    indent_down();
    int exc_num = 0;
    for (const t_field& x : get_elems(tfunction->exceptions())) {
      f_service_ << indent() << "except " << type_name(x.get_type())
                 << " as exc" << exc_num << ":" << endl;
      if (tfunction->qualifier() != t_function_qualifier::one_way) {
        indent_up();
        f_service_ << indent()
                   << "self._event_handler.handlerException(handler_ctx, '"
                   << fn_name << "', exc" << exc_num << ")" << endl
                   << indent() << "result."
                   << rename_reserved_keywords(x.get_name()) << " = exc"
                   << exc_num << endl;
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
    if (tfunction->qualifier() != t_function_qualifier::one_way) {
      f_service_ << indent() << "return result" << endl;
    }

    // Close function
    indent_down();

    if (future) {
      f_service_ << endl;

      f_service_ << indent() << "@future_process_method(" << fn_name
                 << "_args, oneway="
                 << (tfunction->qualifier() == t_function_qualifier::one_way
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
    string prefix,
    bool /*inclass*/,
    string /* actual_type */) {
  const t_type* type = tfield->get_type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->get_name());
  }

  string name = prefix + rename_reserved_keywords(tfield->get_name());

  if (type->is_struct() || type->is_exception()) {
    generate_deserialize_struct(out, (t_struct*)type, name);
  } else if (type->is_container()) {
    generate_deserialize_container(out, type, name);
  } else if (type->is_base_type() || type->is_enum()) {
    indent(out) << name << " = iprot.";

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);
        case t_base_type::TYPE_STRING:
          out << "readString().decode('utf-8') "
              << "if UTF8STRINGS else iprot.readString()";
          break;
        case t_base_type::TYPE_BINARY:
          out << "readString()";
          break;
        case t_base_type::TYPE_BOOL:
          out << "readBool()";
          break;
        case t_base_type::TYPE_BYTE:
          out << "readByte()";
          break;
        case t_base_type::TYPE_I16:
          out << "readI16()";
          break;
        case t_base_type::TYPE_I32:
          out << "readI32()";
          break;
        case t_base_type::TYPE_I64:
          out << "readI64()";
          break;
        case t_base_type::TYPE_DOUBLE:
          out << "readDouble()";
          break;
        case t_base_type::TYPE_FLOAT:
          out << "readFloat()";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no Python name for base type " +
              t_base_type::t_base_name(tbase));
      }
    } else if (type->is_enum()) {
      out << "readI32()";
    }
    out << endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n",
        tfield->get_name().c_str(),
        type->get_name().c_str());
  }
  if (const auto* adapter = get_py_adapter(tfield->get_type())) {
    indent(out) << name << " = " << *adapter << ".from_thrift(" << name << ")"
                << endl;
  }
}

/**
 * Generates an unserializer for a struct, calling read()
 */
void t_py_generator::generate_deserialize_struct(
    ofstream& out, const t_struct* tstruct, string prefix) {
  out << indent() << prefix << " = " << type_name(tstruct) << "()" << endl
      << indent() << prefix << ".read(iprot)" << endl;
}

/**
 * Serialize a container by writing out the header followed by
 * data and then a footer.
 */
void t_py_generator::generate_deserialize_container(
    ofstream& out, const t_type* ttype, string prefix) {
  string size = tmp("_size");
  string ktype = tmp("_ktype");
  string vtype = tmp("_vtype");
  string etype = tmp("_etype");

  t_field fsize(&t_base_type::t_i32(), size);
  t_field fktype(&t_base_type::t_byte(), ktype);
  t_field fvtype(&t_base_type::t_byte(), vtype);
  t_field fetype(&t_base_type::t_byte(), etype);

  // Declare variables, read header
  if (ttype->is_map()) {
    out << indent() << prefix << " = {}" << endl
        << indent() << "(" << ktype << ", " << vtype << ", " << size
        << " ) = iprot.readMapBegin() " << endl;
  } else if (ttype->is_set()) {
    out << indent() << prefix << " = set()" << endl
        << indent() << "(" << etype << ", " << size
        << ") = iprot.readSetBegin()" << endl;
  } else if (ttype->is_list()) {
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

  if (ttype->is_map()) {
    generate_deserialize_map_element(out, (t_map*)ttype, prefix, ktype, vtype);
  } else if (ttype->is_set()) {
    generate_deserialize_set_element(out, (t_set*)ttype, prefix);
  } else if (ttype->is_list()) {
    generate_deserialize_list_element(out, (t_list*)ttype, prefix);
  }

  indent_down();
  indent_down();

  indent(out) << "else: " << endl;
  if (ttype->is_map()) {
    out << indent() << "  while iprot.peekMap():" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "  while iprot.peekSet():" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "  while iprot.peekList():" << endl;
  }

  indent_up();
  indent_up();

  if (ttype->is_map()) {
    generate_deserialize_map_element(out, (t_map*)ttype, prefix, ktype, vtype);
  } else if (ttype->is_set()) {
    generate_deserialize_set_element(out, (t_set*)ttype, prefix);
  } else if (ttype->is_list()) {
    generate_deserialize_list_element(out, (t_list*)ttype, prefix);
  }

  indent_down();
  indent_down();

  // Read container end
  if (ttype->is_map()) {
    indent(out) << "iprot.readMapEnd()" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "iprot.readSetEnd()" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "iprot.readListEnd()" << endl;
  }
}

/**
 * Generates code to deserialize a map
 */
void t_py_generator::generate_deserialize_map_element(
    ofstream& out,
    const t_map* tmap,
    string prefix,
    string key_actual_type,
    string value_actual_type) {
  string key = tmp("_key");
  string val = tmp("_val");
  t_field fkey(tmap->get_key_type(), key);
  t_field fval(tmap->get_val_type(), val);

  generate_deserialize_field(out, &fkey, "", false, key_actual_type);
  generate_deserialize_field(out, &fval, "", false, value_actual_type);

  indent(out) << prefix << "[" << key << "] = " << val << endl;
}

/**
 * Write a set element
 */
void t_py_generator::generate_deserialize_set_element(
    ofstream& out, const t_set* tset, string prefix) {
  string elem = tmp("_elem");
  t_field felem(tset->get_elem_type(), elem);

  generate_deserialize_field(out, &felem);

  indent(out) << prefix << ".add(" << elem << ")" << endl;
}

/**
 * Write a list element
 */
void t_py_generator::generate_deserialize_list_element(
    ofstream& out, const t_list* tlist, string prefix) {
  string elem = tmp("_elem");
  t_field felem(tlist->get_elem_type(), elem);

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
    ofstream& out, const t_field* tfield, string prefix) {
  const t_type* type = tfield->get_type()->get_true_type();

  // Do nothing for void types
  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->get_name());
  }
  string name = prefix + rename_reserved_keywords(tfield->get_name());
  if (const auto* adapter = get_py_adapter(tfield->get_type())) {
    string real_name = std::move(name);
    name = tmp("adpt");
    indent(out) << name << " = " << *adapter << ".to_thrift(" << real_name
                << ")" << endl;
  }
  if (type->is_struct() || type->is_exception()) {
    generate_serialize_struct(out, (t_struct*)type, name);
  } else if (type->is_container()) {
    generate_serialize_container(out, type, name);
  } else if (type->is_base_type() || type->is_enum()) {
    indent(out) << "oprot.";

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);
        case t_base_type::TYPE_STRING:
          out << "writeString(" << name << ".encode('utf-8')) "
              << "if UTF8STRINGS and not isinstance(" << name << ", bytes) "
              << "else oprot.writeString(" << name << ")";
          break;
        case t_base_type::TYPE_BINARY:
          out << "writeString(" << name << ")";
          break;
        case t_base_type::TYPE_BOOL:
          out << "writeBool(" << name << ")";
          break;
        case t_base_type::TYPE_BYTE:
          out << "writeByte(" << name << ")";
          break;
        case t_base_type::TYPE_I16:
          out << "writeI16(" << name << ")";
          break;
        case t_base_type::TYPE_I32:
          out << "writeI32(" << name << ")";
          break;
        case t_base_type::TYPE_I64:
          out << "writeI64(" << name << ")";
          break;
        case t_base_type::TYPE_DOUBLE:
          out << "writeDouble(" << name << ")";
          break;
        case t_base_type::TYPE_FLOAT:
          out << "writeFloat(" << name << ")";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no Python name for base type " +
              t_base_type::t_base_name(tbase));
      }
    } else if (type->is_enum()) {
      out << "writeI32(" << name << ")";
    }
    out << endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO SERIALIZE FIELD '%s%s' TYPE '%s'\n",
        prefix.c_str(),
        tfield->get_name().c_str(),
        type->get_name().c_str());
  }
}

/**
 * Serializes all the members of a struct.
 *
 * @param tstruct The struct to serialize
 * @param prefix  String prefix to attach to all fields
 */
void t_py_generator::generate_serialize_struct(
    ofstream& out, const t_struct* /*tstruct*/, string prefix) {
  indent(out) << prefix << ".write(oprot)" << endl;
}

void t_py_generator::generate_serialize_container(
    ofstream& out, const t_type* ttype, string prefix) {
  if (ttype->is_map()) {
    indent(out) << "oprot.writeMapBegin("
                << type_to_enum(((t_map*)ttype)->get_key_type()) << ", "
                << type_to_enum(((t_map*)ttype)->get_val_type()) << ", "
                << "len(" << prefix << "))" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "oprot.writeSetBegin("
                << type_to_enum(((t_set*)ttype)->get_elem_type()) << ", "
                << "len(" << prefix << "))" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "oprot.writeListBegin("
                << type_to_enum(((t_list*)ttype)->get_elem_type()) << ", "
                << "len(" << prefix << "))" << endl;
  }

  if (ttype->is_map()) {
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
    generate_serialize_map_element(out, (t_map*)ttype, kiter, viter);
    indent_down();
  } else if (ttype->is_set()) {
    string iter = tmp("iter");
    if (sort_keys_) {
      indent(out) << "for " << iter << " in sorted(" << prefix << "):" << endl;
    } else {
      indent(out) << "for " << iter << " in " << prefix << ":" << endl;
    }
    indent_up();
    generate_serialize_set_element(out, (t_set*)ttype, iter);
    indent_down();
  } else if (ttype->is_list()) {
    string iter = tmp("iter");
    indent(out) << "for " << iter << " in " << prefix << ":" << endl;
    indent_up();
    generate_serialize_list_element(out, (t_list*)ttype, iter);
    indent_down();
  }

  if (ttype->is_map()) {
    indent(out) << "oprot.writeMapEnd()" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "oprot.writeSetEnd()" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "oprot.writeListEnd()" << endl;
  }
}

/**
 * Serializes the members of a map.
 *
 */
void t_py_generator::generate_serialize_map_element(
    ofstream& out, const t_map* tmap, string kiter, string viter) {
  t_field kfield(tmap->get_key_type(), kiter);
  generate_serialize_field(out, &kfield, "");

  t_field vfield(tmap->get_val_type(), viter);
  generate_serialize_field(out, &vfield, "");
}

/**
 * Serializes the members of a set.
 */
void t_py_generator::generate_serialize_set_element(
    ofstream& out, const t_set* tset, string iter) {
  t_field efield(tset->get_elem_type(), iter);
  generate_serialize_field(out, &efield, "");
}

/**
 * Serializes the members of a list.
 */
void t_py_generator::generate_serialize_list_element(
    ofstream& out, const t_list* tlist, string iter) {
  t_field efield(tlist->get_elem_type(), iter);
  generate_serialize_field(out, &efield, "");
}

/**
 * Generates the docstring for a given struct.
 */
void t_py_generator::generate_python_docstring(
    ofstream& out, const t_struct* tstruct) {
  generate_python_docstring(out, tstruct, tstruct, "Attributes");
}

/**
 * Generates the docstring for a given function.
 */
void t_py_generator::generate_python_docstring(
    ofstream& out, const t_function* tfunction) {
  generate_python_docstring(
      out, tfunction, tfunction->get_paramlist(), "Parameters");
}

/**
 * Generates the docstring for a struct or function.
 */
void t_py_generator::generate_python_docstring(
    ofstream& out,
    const t_node* tdoc,
    const t_struct* tstruct,
    const char* subheader) {
  bool has_doc = false;
  stringstream ss;
  if (tdoc->has_doc()) {
    has_doc = true;
    ss << tdoc->get_doc();
  }

  const vector<t_field*>& fields = tstruct->get_members();
  if (fields.size() > 0) {
    if (has_doc) {
      ss << endl;
    }
    has_doc = true;
    ss << subheader << ":\n";
    vector<t_field*>::const_iterator p_iter;
    for (p_iter = fields.begin(); p_iter != fields.end(); ++p_iter) {
      const t_field* p = *p_iter;
      ss << " - " << rename_reserved_keywords(p->get_name());
      if (p->has_doc()) {
        ss << ": " << p->get_doc();
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
    ofstream& out, const t_node* tdoc) {
  if (tdoc->has_doc()) {
    generate_docstring_comment(
        out, "r\"\"\"\n", "", tdoc->get_doc(), "\"\"\"\n");
  }
}

/**
 * Declares an argument, which may include initialization as necessary.
 *
 * @param tfield The field
 */
string t_py_generator::declare_argument(
    const t_struct* tstruct, const t_field* tfield) {
  std::ostringstream result;
  result << rename_reserved_keywords(tfield->get_name()) << "=";
  if (tfield->get_value() != nullptr) {
    result << rename_reserved_keywords(tstruct->get_name()) << ".thrift_spec["
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
  const t_type* type = tfield->get_type()->get_true_type();
  if (tfield->get_value() != nullptr) {
    return render_const_value(type, tfield->get_value());
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
    const t_function* tfunction, string prefix) {
  // TODO(mcslee): Nitpicky, no ',' if argument_list is empty
  return prefix + rename_reserved_keywords(tfunction->get_name()) + "(self, " +
      argument_list(tfunction->get_paramlist()) + ")";
}

/**
 * Renders an interface function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_py_generator::function_signature_if(
    const t_function* tfunction, bool with_context, string prefix) {
  // TODO(mcslee): Nitpicky, no ',' if argument_list is empty
  string signature =
      prefix + rename_reserved_keywords(tfunction->get_name()) + "(";
  signature += "self, ";
  if (with_context) {
    signature += "handler_ctx, ";
  }
  signature += argument_list(tfunction->get_paramlist()) + ")";
  return signature;
}

/**
 * Renders a field list
 */
string t_py_generator::argument_list(const t_struct* tstruct) {
  string result = "";

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }
    result += rename_reserved_keywords((*f_iter)->get_name());
    result += "=" + render_field_default_value(*f_iter);
  }
  return result;
}

string t_py_generator::type_name(const t_type* ttype) {
  const t_program* program = ttype->program();
  if (ttype->is_service()) {
    return get_real_py_module(program) + "." +
        rename_reserved_keywords(ttype->get_name());
  }
  if (program != nullptr && program != program_ &&
      !program_->includes().empty()) {
    return get_real_py_module(program) + ".ttypes." +
        rename_reserved_keywords(ttype->get_name());
  }
  return rename_reserved_keywords(ttype->get_name());
}

/**
 * Converts the parse type to a Python type
 */
string t_py_generator::type_to_enum(const t_type* type) {
  type = type->get_true_type();

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("NO T_VOID CONSTRUCT");
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        return "TType.STRING";
      case t_base_type::TYPE_BOOL:
        return "TType.BOOL";
      case t_base_type::TYPE_BYTE:
        return "TType.BYTE";
      case t_base_type::TYPE_I16:
        return "TType.I16";
      case t_base_type::TYPE_I32:
        return "TType.I32";
      case t_base_type::TYPE_I64:
        return "TType.I64";
      case t_base_type::TYPE_DOUBLE:
        return "TType.DOUBLE";
      case t_base_type::TYPE_FLOAT:
        return "TType.FLOAT";
    }
  } else if (type->is_enum()) {
    return "TType.I32";
  } else if (type->is_struct() || type->is_exception()) {
    return "TType.STRUCT";
  } else if (type->is_map()) {
    return "TType.MAP";
  } else if (type->is_set()) {
    return "TType.SET";
  } else if (type->is_list()) {
    return "TType.LIST";
  }

  throw std::runtime_error("INVALID TYPE IN type_to_enum: " + type->get_name());
}

/** See the comment inside generate_py_struct_definition for what this is. */
string t_py_generator::type_to_spec_args(const t_type* ttype) {
  const auto* adapter = get_py_adapter(ttype); // Do this before get_true_type.
  ttype = ttype->get_true_type();

  if (ttype->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
    if (tbase == t_base_type::TYPE_STRING) {
      return "True";
    } else if (tbase == t_base_type::TYPE_BINARY) {
      return "False";
    }
    return "None";
  } else if (ttype->is_enum()) {
    return type_name(ttype);
  } else if (ttype->is_struct()) {
    string ret = "[" + type_name(ttype) + ", " + type_name(ttype) +
        ".thrift_spec, " + (((t_struct*)ttype)->is_union() ? "True" : "False");
    if (adapter) {
      ret += ", " + *adapter;
    }
    return ret + "]";
  } else if (ttype->is_exception()) {
    return "[" + type_name(ttype) + ", " + type_name(ttype) +
        ".thrift_spec, False]";
  } else if (ttype->is_map()) {
    auto tmap = (t_map*)ttype;
    return std::string("(") + type_to_enum(tmap->get_key_type()) + "," +
        type_to_spec_args(tmap->get_key_type()) + "," +
        type_to_enum(tmap->get_val_type()) + "," +
        type_to_spec_args(tmap->get_val_type()) + ")";

  } else if (ttype->is_set()) {
    return "(" + type_to_enum(((t_set*)ttype)->get_elem_type()) + "," +
        type_to_spec_args(((t_set*)ttype)->get_elem_type()) + ")";

  } else if (ttype->is_list()) {
    return "(" + type_to_enum(((t_list*)ttype)->get_elem_type()) + "," +
        type_to_spec_args(((t_list*)ttype)->get_elem_type()) + ")";
  }

  throw std::runtime_error(
      "INVALID TYPE IN type_to_spec_args: " + ttype->get_name());
}

/**
 * Gets the priority annotation of an object (service / function)
 */
std::string t_py_generator::get_priority(
    const t_named* obj, const std::string& def) {
  if (obj) {
    return obj->get_annotation("priority", &def);
  }
  return def;
}

/**
 * Returns the functions that are supported, leaving unsupported functions
 * (e.g. stream and sink functions).
 */
const std::vector<t_function*>& t_py_generator::get_functions(
    const t_service* tservice) {
  auto name = tservice->get_full_name();
  auto found = func_map_.find(name);
  if (found != func_map_.end()) {
    return found->second;
  }
  std::vector<t_function*> funcs;
  for (auto func : tservice->get_functions()) {
    if (!func->sink_or_stream() && !func->return_type()->is_service()) {
      funcs.push_back(func);
    }
  }
  auto inserted = func_map_.emplace(name, std::move(funcs));
  return inserted.first->second;
}

int32_t t_py_generator::get_thrift_spec_key(
    const t_struct* s, const t_field* f) {
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
  const int32_t smallest_id = s->get_sorted_members()[0]->get_key();
  const int32_t offset = -std::min(smallest_id, 0);
  return f->get_key() + offset;
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

} // namespace compiler
} // namespace thrift
} // namespace apache
