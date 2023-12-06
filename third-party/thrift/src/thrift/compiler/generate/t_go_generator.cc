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

/*
 * This file is programmatically sanitized for style:
 * astyle --style=1tbs -f -p -H -j -U t_go_generator.cc
 *
 * The output of astyle should not be taken unquestioningly, but it is a good
 * guide for ensuring uniformity and readability.
 */

#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>

#include <stdlib.h>
#include <sys/types.h>
#include <algorithm>
#include <clocale>
#include <sstream>
#include <thrift/compiler/generate/t_concat_generator.h>
#include <thrift/compiler/generate/t_generator.h>
#include <thrift/compiler/lib/go/util.h>

using namespace std;

namespace apache {
namespace thrift {
namespace compiler {

// static const string endl = "\n"; // avoid ostream << std::endl flushes

const string default_thrift_import =
    "github.com/facebook/fbthrift/thrift/lib/go/thrift";
static std::string package_flag;

/**
 * get_func_name returns the name of the method defined by thrift,
 * considering annotations (if any are set).
 */
const string get_func_name(const t_function* tfunction) {
  auto name_override = go::get_go_name_annotation(tfunction);
  if (name_override != nullptr) {
    return *name_override;
  }
  return tfunction->get_name();
}

/**
 * Go code generator.
 */
class t_go_generator : public t_concat_generator {
 public:
  using t_concat_generator::t_concat_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    out_dir_base_ = "gen-go";
    gen_thrift_import_ = default_thrift_import;

    auto iter = options.find("package_prefix");
    if (iter != options.end()) {
      gen_package_prefix_ = iter->second;
    }

    iter = options.find("thrift_import");
    if (iter != options.end()) {
      gen_thrift_import_ = iter->second;
    }

    iter = options.find("package");
    if (iter != options.end()) {
      package_flag = iter->second;
    }

    iter = options.find("use_context");
    if (iter != options.end()) {
      gen_use_context_ = true;
      gen_processor_func_ = "thrift.ProcessorFunctionContext";
    }
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
  void generate_xception(const t_structured* txception) override;
  void generate_service(const t_service* tservice) override;

  std::string render_const_value(
      const t_type* type,
      const t_const_value* value,
      const string& name,
      bool is_optional = false);

  /**
   * Struct generation code
   */

  void generate_go_struct(const t_structured* tstruct, bool is_exception);
  void generate_go_struct_definition(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_exception = false,
      bool is_result = false,
      bool is_args = false);
  void generate_go_struct_initializer(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_args_or_result = false);
  void generate_isset_helpers(
      std::ofstream& out,
      const t_structured* tstruct,
      const string& tstruct_name,
      bool is_result = false);
  void generate_go_struct_builder(
      std::ofstream& out,
      const t_structured* tstruct,
      const string& tstruct_name,
      bool is_result = false);
  void generate_go_struct_setters(
      std::ofstream& out,
      const t_structured* tstruct,
      const string& tstruct_name,
      bool is_result = false);
  void generate_countsetfields_helper(
      std::ofstream& out,
      const t_structured* tstruct,
      const string& tstruct_name,
      bool is_result = false);
  void generate_go_struct_reader(
      std::ofstream& out,
      const t_structured* tstruct,
      const string& tstruct_name,
      bool is_result = false);
  void generate_go_struct_writer(
      std::ofstream& out,
      const t_structured* tstruct,
      const string& tstruct_name,
      bool is_result = false,
      bool uses_countsetfields = false);
  void generate_go_function_helpers(const t_function* tfunction);
  void get_publicized_name_and_def_value(
      const t_field* tfield,
      string* OUT_pub_name,
      const t_const_value** OUT_def_value) const;
  string make_client_interface_name(const t_service* tservice);

  /**
   * Service-level generation functions
   */

  void generate_service_helpers(const t_service* tservice);
  void generate_service_interface(const t_service* tservice);
  void generate_service_client_interface(const t_service* tservice);
  void generate_service_client(const t_service* tservice, bool);
  void generate_service_client_channel(const t_service* tservice);
  void generate_service_client_common_methods(string&, bool);
  void generate_service_client_method(string&, const t_function*, bool);
  void generate_service_client_channel_method(string&, const t_function*);
  void generate_service_client_channel_call(const t_function* f_iter);
  void generate_service_client_send_msg_call(const t_function*);
  void generate_service_client_recv_method(string&, const t_function*);
  void generate_service_client_recv_method_exception_handling(
      const t_throws* exceptions);
  void generate_service_client_threadsafe(const t_service* tservice);
  void generate_service_server(const t_service* tservice);
  void generate_process_function_type(
      const t_service* tservice, const t_function* tfunction);
  void generate_run_function(
      const t_service* tservice, const t_function* tfunction);
  void generate_read_function(
      const t_service* tservice, const t_function* tfunction);
  void generate_write_function(
      const t_service* tservice, const t_function* tfunction);
  void generate_struct_error_result_fn(
      const t_service* tservice, const t_function* tfunction);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field(
      std::ofstream& out,
      const t_field* tfield,
      bool declare,
      std::string prefix = "",
      bool inclass = false,
      bool coerceData = false,
      bool inkey = false,
      bool in_container = false);

  void generate_deserialize_struct(
      std::ofstream& out,
      const t_struct* tstruct,
      bool is_pointer_field,
      bool declare,
      std::string prefix = "");

  void generate_deserialize_container(
      std::ofstream& out,
      const t_type* ttype,
      bool pointer_field,
      bool declare,
      std::string prefix = "");

  void generate_deserialize_set_element(
      std::ofstream& out,
      const t_set* tset,
      bool declare,
      std::string prefix = "");

  void generate_deserialize_map_element(
      std::ofstream& out,
      const t_map* tmap,
      bool declare,
      std::string prefix = "");

  void generate_deserialize_list_element(
      std::ofstream& out,
      const t_list* tlist,
      bool declare,
      std::string prefix = "");

  void generate_serialize_field(
      std::ofstream& out,
      const t_field* tfield,
      std::string prefix = "",
      bool inkey = false);

  void generate_serialize_struct(
      std::ofstream& out, const t_struct* tstruct, std::string prefix = "");

  void generate_serialize_container(
      std::ofstream& out,
      const t_type* ttype,
      bool pointer_field,
      std::string prefix = "");

  void generate_serialize_map_element(
      std::ofstream& out,
      const t_map* tmap,
      std::string kiter,
      std::string viter);

  void generate_serialize_set_element(
      std::ofstream& out, const t_set* tmap, std::string iter);

  void generate_serialize_list_element(
      std::ofstream& out, const t_list* tlist, std::string iter);

  /**
   * Helper rendering functions
   */

  std::string go_autogen_comment();
  std::string go_package();
  std::string go_imports_begin();
  std::string go_imports_end();
  std::string render_includes();
  std::string render_import_protection();
  std::string render_fastbinary_includes();
  std::string declare_argument(const t_field* tfield);
  std::string render_field_initial_value(
      const t_field* tfield, const string& name, bool optional_field);
  std::string type_name(const t_type* ttype);
  std::string function_signature(
      const t_function* tfunction, std::string prefix = "");
  std::string function_signature_if(
      const t_function* tfunction,
      std::string prefix = "",
      bool useContext = false);
  std::string argument_list(const t_paramlist& tparamlist);
  std::string type_to_enum(const t_type* ttype);
  std::string type_to_go_type(const t_type* ttype);
  std::string type_to_go_type_with_opt(
      const t_type* ttype, bool optional_field, bool from_typedef = false);
  std::string type_to_go_key_type(const t_type* ttype);
  std::string type_to_spec_args(const t_type* ttype);

  static std::string get_real_go_module(const t_program* program) {
    if (!package_flag.empty()) {
      return package_flag;
    }
    std::string real_module = program->get_namespace("go");
    if (!real_module.empty()) {
      return real_module;
    }

    return lowercase(program->name());
  }

 private:
  struct MethodDefinition {
    string name;
    string return_type;
  };

  vector<MethodDefinition> common_client_methods_;

  std::string gen_package_prefix_;
  std::string gen_thrift_import_;
  std::string gen_processor_func_ = "thrift.ProcessorFunction";
  bool gen_use_context_ = false;

  /**
   * File streams
   */

  std::ofstream f_types_;
  std::string f_types_name_;
  std::ofstream f_consts_;
  std::string f_consts_name_;
  std::ostringstream f_const_vars_;
  std::stringstream f_const_values_;
  std::ofstream f_service_;

  std::string package_name_;
  std::string package_dir_;
  unordered_map<std::string, std::string> package_identifiers;

  std::set<std::string> commonInitialisms;
  std::unordered_set<std::string> protectedMethods;

  void generate_go_docstring(std::ofstream& out, const t_structured* tstruct);

  void generate_go_docstring(std::ofstream& out, const t_function* tfunction);

  void generate_go_docstring(std::ofstream& out, const t_named* named_node);

  void generate_go_docstring(
      std::ofstream& out,
      const t_named* named_node,
      const t_structured* tstruct,
      const char* subheader);

  std::string camelcase(const std::string& value) const;
  std::string publicize(
      const std::string& value, bool is_args_or_result = false) const;
  std::string privatize(const std::string& value) const;
  static std::string variable_name_to_go_name(const std::string& value);
  static bool is_pointer_field(
      const t_field* tfield, bool in_container = false);
  static bool omit_initialization(const t_field* tfield);
  static bool type_need_reference(const t_type* type);
  static std::vector<const t_function*> get_supported_functions(
      const t_service*);
};

// we don't have support for thrift streaming in go yet, so skip methods that
// produce it.
std::vector<const t_function*> t_go_generator::get_supported_functions(
    const t_service* tservice) {
  std::vector<const t_function*> funcs;
  for (const auto* func : tservice->get_functions()) {
    if (!func->sink_or_stream() && !func->is_interaction_constructor()) {
      funcs.push_back(func);
    }
  }
  return funcs;
}

// returns true if field initialization can be omitted since it has
// corresponding go type zero value or default value is not set
bool t_go_generator::omit_initialization(const t_field* tfield) {
  const t_const_value* value = tfield->get_value();
  if (!value) {
    return true;
  }
  const t_type* type = tfield->get_type()->get_true_type();
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("Unsupported type: void");

      case t_base_type::TYPE_STRING:
        // strings are pointers if has no default
        return value->get_string().empty();

      case t_base_type::TYPE_BINARY:
        //[]byte are always inline
        return false;

      case t_base_type::TYPE_BOOL:
      case t_base_type::TYPE_BYTE:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
        return value->get_integer() == 0;
      case t_base_type::TYPE_DOUBLE:
      case t_base_type::TYPE_FLOAT:
        if (value->kind() == t_const_value::CV_INTEGER) {
          return value->get_integer() == 0;
        } else {
          return value->get_double() == 0.;
        }
    }
  }
  return false;
}

// Returns true if the type need a reference if used as optional without default
bool t_go_generator::type_need_reference(const t_type* type) {
  type = type->get_true_type();
  if (type->is_map() || type->is_set() || type->is_list() ||
      type->is_struct() || type->is_exception() || type->is_binary()) {
    return false;
  }
  return true;
}

// returns false if field could not use comparison to default value as !IsSet*
bool t_go_generator::is_pointer_field(
    const t_field* tfield, bool in_container_value) {
  (void)in_container_value;
  if (tfield->find_structured_annotation_or_null(kCppRefUri) != nullptr ||
      tfield->has_annotation("cpp.ref")) {
    return true;
  }
  const t_type* type = tfield->get_type()->get_true_type();
  // Structs in containers are pointers
  if (type->is_struct() || type->is_exception()) {
    return true;
  }
  if (!(tfield->get_req() == t_field::e_req::optional)) {
    return false;
  }

  bool has_default = tfield->get_value();
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("Unsupported type: void");

      case t_base_type::TYPE_STRING:
        // strings are pointers if has no default
        return !has_default;

      case t_base_type::TYPE_BINARY:
        //[]byte are always inline
        return false;

      case t_base_type::TYPE_BOOL:
      case t_base_type::TYPE_BYTE:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
      case t_base_type::TYPE_DOUBLE:
      case t_base_type::TYPE_FLOAT:
        return !has_default;
    }
  } else if (type->is_enum()) {
    return !has_default;
  } else if (type->is_struct() || type->is_exception()) {
    return true;
  } else if (type->is_map()) {
    return has_default;
  } else if (type->is_set()) {
    return has_default;
  } else if (type->is_list()) {
    return has_default;
  } else if (type->is_typedef()) {
    return has_default;
  }

  throw std::runtime_error(
      "INVALID TYPE IN type_to_go_type: " + type->get_name());
}

std::string t_go_generator::camelcase(const std::string& value) const {
  std::string value2(value);
  std::setlocale(LC_ALL, "C"); // set locale to classic

  if (value.empty()) {
    return "";
  }

  std::string::size_type i = 0;
  if (value2[0] == '_') {
    // Need a letter, replace '_' with 'x'
    value2[0] = 'x';
    if (value2.length() > 1) {
      value2[1] = toupper(value2[1]);
    }
    i++;
  }
  // as long as we are changing things, let's change _ followed by lowercase to
  // capital and fix common initialisms
  for (; i < value2.size() - 1; ++i) {
    if (value2[i] == '_') {
      if (islower(value2[i + 1])) {
        value2.replace(i, 2, 1, toupper(value2[i + 1]));
      }
      // NOTE: the substr call below has a bug - instead of giving it a desired
      // length of substring, we are giving it an index of the next underscore.
      // This breaks common initialism logic which follows. Too late to fix now.
      std::string word = value2.substr(i, value2.find('_', i));
      std::transform(word.begin(), word.end(), word.begin(), ::toupper);
      if (commonInitialisms.find(word) != commonInitialisms.end()) {
        value2.replace(i, word.length(), word);
      }
    }
  }

  return value2;
}

std::string t_go_generator::publicize(
    const std::string& value, bool is_args_or_result) const {
  if (value.size() <= 0) {
    return value;
  }

  std::string value2(value), prefix;

  string::size_type dot_pos = value.rfind('.');
  if (dot_pos != string::npos) {
    prefix = value.substr(0, dot_pos + 1) + prefix;
    value2 = value.substr(dot_pos + 1);
  }

  value2 = camelcase(value2);

  if (!isupper(value2[0])) {
    value2[0] = toupper(value2[0]);
  }

  // final length before further checks, the string may become longer
  size_t len_before = value2.length();

  if (
      // IDL identifiers may start with "New" which interferes with the CTOR
      // pattern Adding an extra underscore to all those identifiers solves this
      ((len_before >= 3) && (value2.substr(0, 3) == "New")) ||
      // We define certain methods on many structures (such as String, or
      // Error), which can cause certain fields to conflict. Prevent these here
      (protectedMethods.find(value2) != protectedMethods.end())) {
    value2 += '_';
  }

  // IDL identifiers may end with "Args"/"Result" which interferes with the
  // implicit service function structs Adding another extra underscore to all
  // those identifiers solves this Suppress this check for the actual helper
  // struct names
  if (!is_args_or_result) {
    bool ends_with_args =
        (len_before >= 4) && (value2.substr(len_before - 4, 4) == "Args");
    bool ends_with_rslt =
        (len_before >= 6) && (value2.substr(len_before - 6, 6) == "Result");
    if (ends_with_args || ends_with_rslt) {
      value2 += '_';
    }
  }

  // Avoid naming collisions with other services
  if (is_args_or_result) {
    prefix += publicize(service_name_);
  }

  return prefix + value2;
}

std::string t_go_generator::privatize(const std::string& value) const {
  if (value.size() <= 0) {
    return value;
  }

  std::string value2(value);

  value2 = camelcase(value2);

  if (!islower(value2[0])) {
    value2[0] = tolower(value2[0]);
  }

  return value2;
}

std::string t_go_generator::variable_name_to_go_name(const std::string& value) {
  if (value.size() <= 0) {
    return value;
  }

  std::string value2(value);
  std::transform(value2.begin(), value2.end(), value2.begin(), ::tolower);

  switch (value[0]) {
    case 'b':
    case 'B':
      if (value2 != "break") {
        return value;
      }

      break;

    case 'c':
    case 'C':
      if (value2 != "case" && value2 != "chan" && value2 != "const" &&
          value2 != "continue") {
        return value;
      }

      break;

    case 'd':
    case 'D':
      if (value2 != "default" && value2 != "defer") {
        return value;
      }

      break;

    case 'e':
    case 'E':
      if (value2 != "else" && value2 != "error") {
        return value;
      }

      break;

    case 'f':
    case 'F':
      if (value2 != "fallthrough" && value2 != "for" && value2 != "func") {
        return value;
      }

      break;

    case 'g':
    case 'G':
      if (value2 != "go" && value2 != "goto") {
        return value;
      }

      break;

    case 'i':
    case 'I':
      if (value2 != "if" && value2 != "import" && value2 != "interface") {
        return value;
      }

      break;

    case 'm':
    case 'M':
      if (value2 != "map") {
        return value;
      }

      break;

    case 'p':
    case 'P':
      if (value2 != "package") {
        return value;
      }

      break;

    case 'r':
    case 'R':
      if (value2 != "range" && value2 != "return") {
        return value;
      }

      break;

    case 's':
    case 'S':
      if (value2 != "select" && value2 != "struct" && value2 != "switch") {
        return value;
      }

      break;

    case 't':
    case 'T':
      if (value2 != "type") {
        return value;
      }

      break;

    case 'v':
    case 'V':
      if (value2 != "var") {
        return value;
      }

      break;

    default:
      return value;
  }

  return value2 + "_a1";
}

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_go_generator::init_generator() {
  // Make output directory
  string module = get_real_go_module(program_);
  string target = module;
  package_dir_ = get_out_dir();

  common_client_methods_ = {
      {"Open", "error"},
      {"Close", "error"},
      {"IsOpen", "bool"},
  };

  // This set is taken from
  // https://github.com/golang/lint/blob/master/lint.go#L692
  commonInitialisms.insert("API");
  commonInitialisms.insert("ASCII");
  commonInitialisms.insert("CPU");
  commonInitialisms.insert("CSS");
  commonInitialisms.insert("DNS");
  commonInitialisms.insert("EOF");
  commonInitialisms.insert("GUID");
  commonInitialisms.insert("HTML");
  commonInitialisms.insert("HTTP");
  commonInitialisms.insert("HTTPS");
  commonInitialisms.insert("ID");
  commonInitialisms.insert("IP");
  commonInitialisms.insert("JSON");
  commonInitialisms.insert("LHS");
  commonInitialisms.insert("QPS");
  commonInitialisms.insert("RAM");
  commonInitialisms.insert("RHS");
  commonInitialisms.insert("RPC");
  commonInitialisms.insert("SLA");
  commonInitialisms.insert("SMTP");
  commonInitialisms.insert("SSH");
  commonInitialisms.insert("TCP");
  commonInitialisms.insert("TLS");
  commonInitialisms.insert("TTL");
  commonInitialisms.insert("UDP");
  commonInitialisms.insert("UI");
  commonInitialisms.insert("UID");
  commonInitialisms.insert("UUID");
  commonInitialisms.insert("URI");
  commonInitialisms.insert("URL");
  commonInitialisms.insert("UTF8");
  commonInitialisms.insert("VM");
  commonInitialisms.insert("XML");
  commonInitialisms.insert("XSRF");
  commonInitialisms.insert("XSS");

  // Methods that we define on some structures
  protectedMethods.insert("Error");
  protectedMethods.insert("String");

  while (true) {
    // TODO: Do better error checking here.
    boost::filesystem::create_directory(package_dir_);

    if (module.empty()) {
      break;
    }

    string::size_type pos = module.find('.');

    if (pos == string::npos) {
      package_dir_ += "/";
      package_dir_ += module;
      package_name_ = module;
      // TODO(dokwon): Remove this after fixing package name colliding with
      // reserved keywords.
      // 'type' is a reserved keyword in go. Append '_' to bypass this issue.
      if (package_name_ == "type") {
        package_name_ += "_";
      }
      module.clear();
    } else {
      package_dir_ += "/";
      package_dir_ += module.substr(0, pos);
      module.erase(0, pos + 1);
    }
  }

  string::size_type loc;

  while ((loc = target.find('.')) != string::npos) {
    target.replace(loc, 1, 1, '/');
  }

  // Make output files
  f_types_name_ = package_dir_ + "/" + "ttypes.go";
  f_types_.open(f_types_name_.c_str());

  f_consts_name_ = package_dir_ + "/" + "constants.go";
  f_consts_.open(f_consts_name_.c_str());

  // Print header
  f_types_ << go_autogen_comment() << go_package() << render_includes()
           << render_import_protection();

  f_consts_ << go_autogen_comment() << go_package() << render_includes();

  f_const_values_ << endl << "func init() {" << endl;
}

/**
 * Renders all the imports necessary for including another Thrift program
 */
string t_go_generator::render_includes() {
  const vector<t_program*>& includes = program_->get_includes_for_codegen();
  string result = "";
  string unused_prot = "";

  for (size_t i = 0; i < includes.size(); ++i) {
    string go_module = get_real_go_module(includes[i]);
    string go_path = go_module;
    size_t found = 0;
    for (size_t j = 0; j < go_module.size(); j++) {
      // Import statement uses slashes ('/') in namespace
      if (go_module[j] == '.') {
        go_path[j] = '/';
        found = j + 1;
      }
    }

    auto it = package_identifiers.find(go_module);
    if (it == package_identifiers.end()) {
      auto value = tmp(go_module.substr(found));
      it = package_identifiers.emplace(go_module, std::move(value)).first;
    }
    const auto& package_identifier = it->second;

    result += "\t" + package_identifier + " \"" + gen_package_prefix_ +
        go_path + "\"\n";
    unused_prot += "var _ = " + package_identifier + ".GoUnusedProtection__\n";
  }

  if (includes.size() > 0) {
    result += "\n";
  }

  return go_imports_begin() + result + go_imports_end() + unused_prot;
}

string t_go_generator::render_import_protection() {
  return string("var GoUnusedProtection__ int;\n\n");
}

/**
 * Renders all the imports necessary to use the accelerated TBinaryProtocol
 */
string t_go_generator::render_fastbinary_includes() {
  return "";
}

/**
 * Autogen'd comment
 */
string t_go_generator::go_autogen_comment() {
  return std::string() +
      "// Autogenerated by Thrift Compiler (facebook)\n"
      "// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING\n"
      "// @"
      "generated\n\n";
}

/**
 * Prints standard thrift package
 */
string t_go_generator::go_package() {
  return string("package ") + package_name_ + "\n\n";
}

/**
 * Render the beginning of the import statement
 */
string t_go_generator::go_imports_begin() {
  return string(
      "import (\n"
      "\t\"bytes\"\n"
      "\t\"context\"\n"
      "\t\"sync\"\n"
      "\t\"fmt\"\n"
      "\tthrift \"" +
      gen_thrift_import_ + "\"\n");
}

/**
 * End the import statement, include undscore-assignments
 *
 * These "_ =" prevent the go compiler complaining about unused imports.
 * This will have to do in lieu of more intelligent import statement
 * construction
 */
string t_go_generator::go_imports_end() {
  return string(
      ")\n\n"
      "// (needed to ensure safety because of naive import list "
      "construction.)\n"
      "var _ = thrift.ZERO\n"
      "var _ = fmt.Printf\n"
      "var _ = sync.Mutex{}\n"
      "var _ = bytes.Equal\n"
      "var _ = context.Background\n\n");
}

/**
 * Closes the type files
 */
void t_go_generator::close_generator() {
  f_const_values_ << "}" << endl << endl;
  f_consts_ << f_const_vars_.str();
  f_consts_ << f_const_values_.str();

  // Close types and constants files
  f_consts_.close();
  f_types_.close();
}

/**
 * Generates a typedef.
 *
 * @param ttypedef The type definition
 */
void t_go_generator::generate_typedef(const t_typedef* ttypedef) {
  generate_go_docstring(f_types_, ttypedef);
  string new_type_name(publicize(ttypedef->name()));
  const t_type* tbasetype(ttypedef->get_type());
  const t_type* true_type(ttypedef->get_true_type());
  string base_type(type_to_go_type_with_opt(tbasetype, false, true));

  if (base_type == new_type_name) {
    return;
  }

  f_types_ << "type " << new_type_name << " = " << base_type << endl << endl;
  // Generate a convenience function that converts an instance of a type
  // (which may be a constant) into a pointer to an instance of a type.
  // Special case: If a newtype ends in "Ptr" this function might redeclare the
  // typename as a function. So if that would happen, name it with the suffix
  // ToPointer instead.
  int name_len = new_type_name.length();
  if (name_len >= 3 && new_type_name[name_len - 3] == 'P' &&
      new_type_name[name_len - 2] == 't' &&
      new_type_name[name_len - 1] == 'r') {
    f_types_ << "func " << new_type_name << "ToPointer(v " << new_type_name
             << ") *" << new_type_name << " { return &v }" << endl
             << endl;
  } else {
    f_types_ << "func " << new_type_name << "Ptr(v " << new_type_name << ") *"
             << new_type_name << " { return &v }" << endl
             << endl;
  }
  // Generate New* function
  if (true_type->is_struct() || true_type->is_exception()) {
    const t_program* program = tbasetype->program();
    // only declare a return with a pointer if the concrete type isn't
    // already a pointer
    const bool needsptr = !(base_type.at(0) == '*');
    string ctor;
    if (program != nullptr && program != program_) {
      string module(package_identifiers[get_real_go_module(program)]);
      ctor = module + ".New" + publicize(tbasetype->get_name());
    } else {
      ctor = "New" + publicize(tbasetype->get_name());
    }
    f_types_ << "func "
             << "New" << publicize(new_type_name) << "() "
             << (needsptr ? "*" : "") << new_type_name << " { return " << ctor
             << "() }" << endl
             << endl;
  }
}

/**
 * Generates code for an enumerated type. Done using a class to scope
 * the values.
 *
 * @param tenum The enumeration
 */
void t_go_generator::generate_enum(const t_enum* tenum) {
  std::ostringstream name_mapping, value_mapping, names_slice, values_slice;
  std::string tenum_name(publicize(tenum->get_name()));
  generate_go_docstring(f_types_, tenum);
  f_types_ << "type " << tenum_name << " int64" << endl << "const (" << endl;

  name_mapping << indent() << "var " << tenum_name << "ToName = map["
               << tenum_name << "]string {" << endl;

  value_mapping << indent() << "var " << tenum_name << "ToValue = map[string]"
                << tenum_name << " {" << endl;

  names_slice << indent() << "var " << tenum_name << "Names = []string {"
              << endl;

  values_slice << indent() << "var " << tenum_name << "Values = []"
               << tenum_name << " {" << endl;

  vector<t_enum_value*> constants = tenum->get_enum_values();
  vector<t_enum_value*>::iterator c_iter;
  set<int> seen;

  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int value = (*c_iter)->get_value();

    string iter_std_name(escape_string((*c_iter)->get_name()));
    string iter_name((*c_iter)->get_name());
    f_types_ << indent() << "  " << tenum_name << "_" << iter_name << ' '
             << tenum_name << " = " << value << endl;

    // Only add a to_string_mapping if there isn't a duplicate of the value
    if (seen.find(value) == seen.end()) {
      // Dictionaries to/from string names of enums
      name_mapping << indent() << "  " << tenum_name << "_" << iter_name
                   << ": \"" << iter_std_name << "\"," << endl;
    }

    value_mapping << indent() << "  \"" << iter_std_name << "\": " << tenum_name
                  << "_" << iter_name << "," << endl;

    names_slice << indent() << "  \"" << iter_std_name << "\"," << endl;

    values_slice << indent() << "  " << tenum_name << "_" << iter_name << ","
                 << endl;

    if (iter_std_name != escape_string(iter_name)) {
      value_mapping << indent() << "  \"" << escape_string(iter_std_name)
                    << "\": " << tenum_name << "_" << iter_name << "," << endl;
    }
    seen.insert(value);
  }

  name_mapping << indent() << "}" << endl;
  value_mapping << indent() << "}" << endl;
  names_slice << indent() << "}" << endl;
  values_slice << indent() << "}" << endl;

  f_types_ << ")" << endl << endl;
  f_types_ << name_mapping.str() << endl;
  f_types_ << value_mapping.str() << endl;
  f_types_ << names_slice.str() << endl;
  f_types_ << values_slice.str() << endl;
  f_types_ << "func (p " << tenum_name << ") String() string {" << endl
           << "  if v, ok := " << tenum_name << "ToName[p]; ok {" << endl
           << "    return v" << endl
           << "  }" << endl
           << "  return \"<UNSET>\"" << endl
           << "}" << endl
           << endl;
  f_types_ << "func " << tenum_name << "FromString(s string) (" << tenum_name
           << ", error) {" << endl
           << "  if v, ok := " << tenum_name << "ToValue[s]; ok {" << endl
           << "    return v, nil" << endl
           << "  }" << endl
           << "  return " << tenum_name << "(0),"
           << " fmt.Errorf(\"not a valid " << tenum_name << " string\")" << endl
           << "}" << endl
           << endl;

  // Generate a convenience function that converts an instance of an enum
  // (which may be a constant) into a pointer to an instance of that enum
  // type.
  f_types_ << "func " << tenum_name << "Ptr(v " << tenum_name << ") *"
           << tenum_name << " { return &v }" << endl
           << endl;
}

/**
 * Generate a constant value
 */
void t_go_generator::generate_const(const t_const* tconst) {
  const t_type* type = tconst->type();
  string name = publicize(tconst->get_name());
  const t_const_value* value = tconst->value();

  if (type->is_base_type() || type->is_enum()) {
    indent(f_consts_) << "const " << name << " = "
                      << render_const_value(type, value, name) << endl;
  } else {
    f_const_values_ << indent() << name << " = "
                    << render_const_value(type, value, name) << endl
                    << endl;

    f_consts_ << indent() << "var " << name << " " << type_to_go_type(type)
              << endl;
  }
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
string t_go_generator::render_const_value(
    const t_type* type,
    const t_const_value* value,
    const string& name,
    bool is_optional) {
  type = type->get_true_type();
  std::ostringstream out;

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
      case t_base_type::TYPE_STRING:
        if (is_optional) {
          f_const_vars_ << "var const_lit_" << name << " "
                        << type_to_go_type(type) << " = " << '"'
                        << get_escaped_string(value) << '"' << endl;
          out << "&const_lit_" << name;
        } else {
          out << '"' << get_escaped_string(value) << '"';
        }
        break;
      case t_base_type::TYPE_BINARY:
        out << "[]byte(\"" << get_escaped_string(value) << "\")";
        break;
      case t_base_type::TYPE_BOOL:
        out << (value->get_integer() > 0 ? "true" : "false");
        break;

      case t_base_type::TYPE_BYTE:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
        if (is_optional) {
          f_const_vars_ << "var const_lit_" << name << " "
                        << type_to_go_type(type) << " = "
                        << value->get_integer() << endl;
          out << "&const_lit_" << name;
        } else {
          out << value->get_integer();
        }
        break;

      case t_base_type::TYPE_DOUBLE:
      case t_base_type::TYPE_FLOAT: {
        if (is_optional) {
          f_const_vars_ << "var const_lit_" << name << " "
                        << type_to_go_type(type) << " = ";
        }

        std::ostringstream& value_out = is_optional ? f_const_vars_ : out;
        if (value->kind() == t_const_value::CV_INTEGER) {
          value_out << value->get_integer();
        } else {
          value_out << value->get_double();
        }

        if (is_optional) {
          f_const_vars_ << endl;
          out << "&const_lit_" << name;
        }

        break;
      }

      default:
        throw std::runtime_error(
            "compiler error: no const of base type " +
            t_base_type::t_base_name(tbase));
    }
  } else if (type->is_enum()) {
    if (is_optional) {
      f_const_vars_ << "var const_lit_" << name << " " << type_to_go_type(type)
                    << " = " << value->get_integer() << endl;
      out << "&const_lit_" << name;
    } else {
      out << value->get_integer();
    }
  } else if (type->is_struct() || type->is_exception()) {
    out << "&" << publicize(type_name(type)) << "{";
    indent_up();
    out << endl;
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;

    if (((t_struct*)type)->is_union()) {
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        (*f_iter)->set_req(t_field::e_req::optional);
      }
    }

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      const t_type* field_type = nullptr;
      bool is_field_optional = false;
      string field_name;

      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if ((*f_iter)->get_name() == v_iter->first->get_string()) {
          field_name = name + "_" + (*f_iter)->get_name();
          field_type = (*f_iter)->get_type();
          is_field_optional =
              ((*f_iter)->get_req() == t_field::e_req::optional);
          break;
        }
      }

      if (field_type == nullptr) {
        throw std::runtime_error(
            "type error: " + type->get_name() + " has no field " +
            v_iter->first->get_string());
      }

      out << indent() << publicize(v_iter->first->get_string()) << ": "
          << render_const_value(
                 field_type, v_iter->second, field_name, is_field_optional)
          << "," << endl;
    }
    indent_down();
    out << indent() << "}";

  } else if (type->is_map()) {
    const t_type* ktype = ((t_map*)type)->get_key_type();
    const t_type* vtype = ((t_map*)type)->get_val_type();
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    out << "map[" << type_to_go_key_type(ktype) << "]" << type_to_go_type(vtype)
        << "{" << endl;
    indent_up();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      auto idx = std::to_string(std::distance(val.begin(), v_iter));
      out << indent() << render_const_value(ktype, v_iter->first, name) << ": "
          << render_const_value(vtype, v_iter->second, name + "_" + idx) << ","
          << endl;
    }

    indent_down();
    out << indent() << "}";
  } else if (type->is_list()) {
    const t_type* etype = ((t_list*)type)->get_elem_type();
    const vector<t_const_value*>& val = value->get_list();
    out << "[]" << type_to_go_type(etype) << "{" << endl;
    indent_up();
    vector<t_const_value*>::const_iterator v_iter;

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      auto idx = std::to_string(std::distance(val.begin(), v_iter));
      out << indent() << render_const_value(etype, *v_iter, name + "_" + idx)
          << "," << endl;
    }

    indent_down();
    out << indent() << "}";
  } else if (type->is_set()) {
    const t_type* etype = ((t_set*)type)->get_elem_type();
    const vector<t_const_value*>& val = value->get_list();
    out << "[]" << type_to_go_key_type(etype) << "{" << endl;
    indent_up();
    vector<t_const_value*>::const_iterator v_iter;

    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      auto idx = std::to_string(std::distance(val.begin(), v_iter));
      out << indent() << render_const_value(etype, *v_iter, name + "_" + idx)
          << ", ";
    }

    indent_down();
    out << indent() << "}";
  } else {
    throw std::runtime_error(
        "CANNOT GENERATE CONSTANT FOR TYPE: " + type->get_name());
  }

  return out.str();
}

/**
 * Generates a go struct
 */
void t_go_generator::generate_struct(const t_structured* tstruct) {
  generate_go_struct(tstruct, false /* is_exception */);
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_go_generator::generate_xception(const t_structured* txception) {
  generate_go_struct(txception, true /* is_exception */);
}

/**
 * Generates a go struct
 */
void t_go_generator::generate_go_struct(
    const t_structured* tstruct, bool is_exception) {
  generate_go_struct_definition(f_types_, tstruct, is_exception);
}

void t_go_generator::get_publicized_name_and_def_value(
    const t_field* tfield,
    string* OUT_pub_name,
    const t_const_value** OUT_def_value) const {
  const string base_field_name = tfield->get_name();
  const string escaped_field_name = escape_string(base_field_name);
  *OUT_pub_name = publicize(escaped_field_name);
  *OUT_def_value = tfield->get_value();
}

void t_go_generator::generate_go_struct_initializer(
    ofstream& out, const t_structured* tstruct, bool is_args_or_result) {
  out << publicize(type_name(tstruct), is_args_or_result) << "{";
  indent_up();
  const vector<t_field*>& members = tstruct->get_members();
  // This boolean flag is used for formatting & indentation purposes
  bool member_has_been_initialized = false;
  for (vector<t_field*>::const_iterator m_iter = members.begin();
       m_iter != members.end();
       ++m_iter) {
    const t_type* ttype = (*m_iter)->get_type();
    const t_type* ttruetype = ttype->get_true_type();
    bool pointer_field = is_pointer_field(*m_iter);
    bool struct_field = ttruetype->is_struct() &&
        !(dynamic_cast<const t_struct*>(ttruetype)->is_union());
    string publicized_name;
    string module;
    if (auto&& program = ttype->program()) {
      if (program != program_) {
        module = package_identifiers[get_real_go_module(program)] + ".";
      }
    }

    const t_const_value* def_value;
    get_publicized_name_and_def_value(*m_iter, &publicized_name, &def_value);
    if (!pointer_field && def_value != nullptr &&
        !omit_initialization(*m_iter)) {
      if (!member_has_been_initialized) {
        out << endl;
      }
      out << indent() << publicized_name << ": "
          << render_field_initial_value(
                 *m_iter, (*m_iter)->get_name(), pointer_field)
          << "," << endl;
      member_has_been_initialized = true;
    } else if (
        struct_field && (*m_iter)->get_req() != t_field::e_req::optional) {
      if (!member_has_been_initialized) {
        out << endl;
      }
      out << indent() << publicized_name << ": " << module << "New"
          << publicize(ttype->get_name()) << "()"
          << "," << endl;
      member_has_been_initialized = true;
    }
  }

  indent_down();
  if (member_has_been_initialized) {
    out << indent();
  }
  out << "}" << endl;
}

/**
 * Generates a struct definition for a thrift data type.
 *
 * @param tstruct The struct definition
 */
void t_go_generator::generate_go_struct_definition(
    ofstream& out,
    const t_structured* tstruct,
    bool is_exception,
    bool is_result,
    bool is_args) {
  const vector<t_field*>& members = tstruct->get_members();
  const vector<t_field*>& sorted_members = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator m_iter;

  std::string tstruct_name(
      publicize(tstruct->get_name(), is_args || is_result));
  generate_go_docstring(out, tstruct);
  out << indent() << "type " << tstruct_name << " struct {" << endl;
  /*
     Here we generate the structure specification for the fastbinary codec.
     These specifications have the following structure:
     thrift_spec -> tuple of item_spec
     item_spec -> nil | (tag, type_enum, name, spec_args, default)
     tag -> integer
     type_enum -> TType.I32 | TType.STRING | TType.STRUCT | ...
     name -> string_literal
     default -> nil  # Handled by __init__
     spec_args -> nil  # For simple types
                | (type_enum, spec_args)  # Value type for list/set
                | (type_enum, spec_args, type_enum, spec_args)
                  # Key and value for map
                | (class_name, spec_args_ptr) # For struct/exception
     class_name -> identifier  # Basically a pointer to the class
     spec_args_ptr -> expression  # just class_name.spec_args

     TODO(dreiss): Consider making this work for structs with negative tags.
  */
  // TODO(dreiss): Look into generating an empty tuple instead of nil
  // for structures with no members.
  // TODO(dreiss): Test encoding of structs where some inner structs
  // don't have thrift_spec.
  indent_up();

  if (is_args) {
    out << indent() << "thrift.IRequest" << endl;
  }

  if (is_result) {
    out << indent() << "thrift.IResponse" << endl;
  }

  int num_setable = 0;
  if (sorted_members.empty() || (sorted_members[0]->get_key() >= 0)) {
    int sorted_keys_pos = 0;

    for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
         ++m_iter) {
      // Set field to optional if field is union, this is so we can get a
      // pointer to the field.
      if (tstruct->is_union())
        (*m_iter)->set_req(t_field::e_req::optional);
      if (sorted_keys_pos != (*m_iter)->get_key()) {
        int first_unused = std::max(1, sorted_keys_pos++);
        while (sorted_keys_pos != (*m_iter)->get_key()) {
          ++sorted_keys_pos;
        }
        int last_unused = sorted_keys_pos - 1;
        if (first_unused < last_unused) {
          indent(out) << "// unused fields # " << first_unused << " to "
                      << last_unused << endl;
        } else if (first_unused == last_unused) {
          indent(out) << "// unused field # " << first_unused << endl;
        }
      }

      const t_type* fieldType = (*m_iter)->get_type();
      string goType =
          type_to_go_type_with_opt(fieldType, is_pointer_field(*m_iter));
      string gotag;
      // Check for user override of db and json tags using "go.tag"
      if (const auto* val = go::get_go_tag_annotation(*m_iter)) {
        gotag = *val;
      } else {
        gotag = "db:\"" + escape_string((*m_iter)->get_name()) + "\" ";
        if ((*m_iter)->get_req() == t_field::e_req::optional) {
          gotag +=
              "json:\"" + escape_string((*m_iter)->get_name()) + ",omitempty\"";
        } else {
          gotag += "json:\"" + escape_string((*m_iter)->get_name()) + "\"";
        }
      }
      indent(out) << publicize((*m_iter)->get_name()) << " " << goType
                  << " `thrift:\"" << escape_string((*m_iter)->get_name())
                  << "," << sorted_keys_pos;

      if ((*m_iter)->get_req() == t_field::e_req::required) {
        out << ",required";
      } else if (((*m_iter)->get_req() == t_field::e_req::optional)) {
        out << ",optional";
      }

      out << "\" " << gotag << "`" << endl;
      sorted_keys_pos++;
    }
  } else {
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      // This fills in default values, as opposed to nulls
      out << indent() << publicize((*m_iter)->get_name()) << " "
          << type_to_go_type((*m_iter)->get_type()) << endl;
    }
  }

  indent_down();
  out << indent() << "}" << endl << endl;
  out << indent() << "func New" << tstruct_name << "() *" << tstruct_name
      << " {" << endl;
  indent_up();
  out << indent() << "return &";
  generate_go_struct_initializer(out, tstruct, is_result || is_args);
  indent_down();
  out << indent() << "}" << endl << endl;
  // Default values for optional fields
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string publicized_name;
    const t_const_value* def_value;
    get_publicized_name_and_def_value(*m_iter, &publicized_name, &def_value);
    const t_type* fieldType = (*m_iter)->get_type();
    string goType = type_to_go_type_with_opt(fieldType, false);
    string def_var_name = tstruct_name + "_" + publicized_name + "_DEFAULT";
    if ((*m_iter)->get_req() == t_field::e_req::optional ||
        is_pointer_field(*m_iter)) {
      out << indent() << "var " << def_var_name << " " << goType;
      if (def_value != nullptr) {
        out << " = "
            << render_const_value(fieldType, def_value, (*m_iter)->get_name());
      }
      out << endl;
    }
    if (is_pointer_field(*m_iter)) {
      string goOptType = type_to_go_type_with_opt(fieldType, true);
      string maybepointer = goOptType != goType ? "*" : "";
      out << indent() << "func (p *" << tstruct_name << ") Get"
          << publicized_name << "() " << goType << " {" << endl;
      out << indent() << "  if !p.IsSet" << publicized_name << "() {" << endl;
      out << indent() << "    return " << def_var_name << endl;
      out << indent() << "  }" << endl;
      out << indent() << "  return " << maybepointer << "p." << publicized_name
          << endl;
      out << indent() << "}" << endl;
      if (fieldType->is_struct()) {
        // Create "default" getter for structs
        string module;
        if (auto&& program = fieldType->program()) {
          if (program != program_) {
            module = package_identifiers[get_real_go_module(program)] + ".";
          }
        }
        out << indent() << "func (p *" << tstruct_name << ") DefaultGet"
            << publicized_name << "() " << goType << " {" << endl;
        out << indent() << "  if !p.IsSet" << publicized_name << "() {" << endl;
        out << indent() << "    return " << module << "New"
            << publicize(fieldType->get_name()) << "()" << endl;
        out << indent() << "  }" << endl;
        out << indent() << "  return " << maybepointer << "p."
            << publicized_name << endl;
        out << indent() << "}" << endl;
      }
      num_setable += 1;
    } else {
      out << endl;
      out << indent() << "func (p *" << tstruct_name << ") Get"
          << publicized_name << "() " << goType << " {" << endl;
      out << indent() << "  return p." << publicized_name << endl;
      out << indent() << "}" << endl;
    }
  }

  if (tstruct->is_union() && num_setable > 0) {
    generate_countsetfields_helper(out, tstruct, tstruct_name, is_result);
  }

  generate_isset_helpers(out, tstruct, tstruct_name, is_result);
  generate_go_struct_builder(out, tstruct, tstruct_name, is_result);
  generate_go_struct_setters(out, tstruct, tstruct_name, is_result);
  generate_go_struct_reader(out, tstruct, tstruct_name, is_result);
  generate_go_struct_writer(
      out, tstruct, tstruct_name, is_result, num_setable > 0);

  out << indent() << "func (p *" << tstruct_name << ") String() string {"
      << endl;
  out << indent() << "  if p == nil {" << endl;
  out << indent() << "    return \"<nil>\"" << endl;
  out << indent() << "  }" << endl << endl;

  std::vector<std::string> format;
  string values;
  for (m_iter = sorted_members.begin(); m_iter != sorted_members.end();
       ++m_iter) {
    if (is_pointer_field(*m_iter)) {
      out << indent() << "  var " << privatize((*m_iter)->get_name())
          << "Val string" << endl;
      out << indent() << "  if p." << publicize((*m_iter)->get_name())
          << " == nil {" << endl;
      out << indent() << "    " << privatize((*m_iter)->get_name())
          << "Val = \"<nil>\"" << endl;
      out << indent() << "  } else {" << endl;

      if (type_need_reference((*m_iter)->get_type())) {
        out << indent() << "    " << privatize((*m_iter)->get_name())
            << "Val = fmt.Sprintf(\"%v\", *p."
            << publicize((*m_iter)->get_name()) << ")" << endl;
      } else {
        out << indent() << "    " << privatize((*m_iter)->get_name())
            << "Val = fmt.Sprintf(\"%v\", p."
            << publicize((*m_iter)->get_name()) << ")" << endl;
      }

      out << indent() << "  }" << endl;
    } else {
      out << indent() << "  " << privatize((*m_iter)->get_name())
          << "Val := fmt.Sprintf(\"%v\", p." << publicize((*m_iter)->get_name())
          << ")" << endl;
    }
    format.push_back(publicize((*m_iter)->get_name()) + ":%s");
    values = values + ", " + privatize((*m_iter)->get_name()) + "Val";
  }
  out << indent() << "  return fmt.Sprintf(\"" << escape_string(tstruct_name)
      << "({" << boost::algorithm::join(format, " ") << "})\"" << values << ")"
      << endl;
  out << indent() << "}" << endl << endl;

  if (is_exception) {
    out << indent() << "func (p *" << tstruct_name << ") Error() string {"
        << endl;
    out << indent() << "  return p.String()" << endl;
    out << indent() << "}" << endl << endl;
  }
}

/**
 * Generates the IsSet helper methods for a struct
 */
void t_go_generator::generate_isset_helpers(
    ofstream& out,
    const t_structured* tstruct,
    const string& tstruct_name,
    bool is_result) {
  (void)is_result;
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  const string escaped_tstruct_name(escape_string(tstruct->get_name()));

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    const string field_name(publicize(escape_string((*f_iter)->get_name())));
    if ((*f_iter)->get_req() == t_field::e_req::optional ||
        is_pointer_field(*f_iter)) {
      out << indent() << "func (p *" << tstruct_name << ") IsSet" << field_name
          << "() bool {" << endl;
      indent_up();
      const t_type* ttype = (*f_iter)->get_type()->get_true_type();
      bool is_byteslice = ttype->is_base_type() && ttype->is_binary();
      bool compare_to_nil_only = ttype->is_set() || ttype->is_list() ||
          ttype->is_map() || (is_byteslice && !(*f_iter)->get_value());
      if (is_pointer_field(*f_iter) || compare_to_nil_only) {
        out << indent() << "return p != nil && p." << field_name << " != nil"
            << endl;
      } else {
        string def_var_name = tstruct_name + "_" + field_name + "_DEFAULT";
        if (is_byteslice) {
          out << indent() << "return p != nil && !bytes.Equal(p." << field_name
              << ", " << def_var_name << ")" << endl;
        } else {
          out << indent() << "return p != nil && p." << field_name
              << " != " << def_var_name << endl;
        }
      }
      indent_down();
      out << indent() << "}" << endl << endl;
    }
  }
}

/**
 * Generates Builder helper object for a struct
 */
void t_go_generator::generate_go_struct_builder(
    ofstream& out,
    const t_structured* tstruct,
    const string& tstruct_name,
    bool is_result) {
  (void)is_result;

  // Type definition
  out << indent() << "type " << tstruct_name << "Builder struct {" << endl;
  indent_up();
  out << indent() << "obj *" << tstruct_name << endl;
  indent_down();
  out << indent() << "}" << endl << endl;
  // End of type definition

  // New function
  out << indent() << "func New" << tstruct_name << "Builder() *" << tstruct_name
      << "Builder"
      << "{" << endl;
  indent_up();
  out << indent() << "return &" << tstruct_name << "Builder{" << endl;
  indent_up();
  out << indent() << "obj: New" << tstruct_name << "()," << endl;
  indent_down();
  out << indent() << "}" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;
  // End of New function

  // Emit function
  out << indent() << "func (p " << tstruct_name << "Builder) Emit() *"
      << tstruct_name << "{" << endl;
  indent_up();
  out << indent() << "return &" << tstruct_name << "{" << endl;
  indent_up();
  const auto members = tstruct->get_members();
  // Emit: copy values field-by-field into the new object to let Emit() issue
  // new object on every call which allows to reuse Builder
  for (auto m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string publicized_name;
    const t_const_value* def_value;
    get_publicized_name_and_def_value(*m_iter, &publicized_name, &def_value);
    out << indent() << publicized_name << ": p.obj." << publicized_name << ","
        << endl;
  }
  indent_down();
  out << indent() << "}" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;
  // End of Emit function

  // Builder functions
  for (auto m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string publicized_name;
    const t_const_value* def_value;
    get_publicized_name_and_def_value(*m_iter, &publicized_name, &def_value);

    string arg_name =
        variable_name_to_go_name(privatize((*m_iter)->get_name()));

    string receiver_name = lowercase(tstruct_name.substr(0, 1));
    if (arg_name == receiver_name) {
      receiver_name = lowercase(tstruct_name.substr(0, 2));
    }

    const t_type* ttype = (*m_iter)->get_type();
    string go_type = type_to_go_type(ttype);

    string maybepointer;
    if (is_pointer_field(*m_iter)) {
      string go_opt_type = type_to_go_type_with_opt(ttype, true);
      maybepointer = go_opt_type != go_type ? "*" : "";
    }

    out << indent() << "func (" << receiver_name << " *" << tstruct_name
        << "Builder) " << publicized_name << "(" << arg_name << " "
        << maybepointer << go_type << ") *" << tstruct_name << "Builder {"
        << endl;
    indent_up();
    out << indent() << receiver_name << ".obj." << publicized_name << " = "
        << arg_name << endl;
    out << indent() << "return " << receiver_name << endl;
    indent_down();
    out << indent() << "}" << endl << endl;
  }
  // End of Builder functions
}

/**
 * Generates Set* methods for Go struct
 */
void t_go_generator::generate_go_struct_setters(
    ofstream& out,
    const t_structured* tstruct,
    const string& tstruct_name,
    bool is_result) {
  (void)is_result;
  const auto members = tstruct->get_members();
  set<string> field_names;
  for (auto m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string publicized_name;
    const t_const_value* def_value;
    get_publicized_name_and_def_value(*m_iter, &publicized_name, &def_value);
    field_names.insert(publicized_name);
  }

  for (auto m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string publicized_name;
    const t_const_value* def_value;
    get_publicized_name_and_def_value(*m_iter, &publicized_name, &def_value);

    string arg_name =
        variable_name_to_go_name(privatize((*m_iter)->get_name()));

    string receiver_name = lowercase(tstruct_name.substr(0, 1));
    if (arg_name == receiver_name) {
      receiver_name = lowercase(tstruct_name.substr(0, 2));
    }

    string method_name = "Set" + publicized_name;
    while (field_names.count(method_name) != 0) {
      method_name += "_";
    }

    const t_type* ttype = (*m_iter)->get_type();
    string go_type = type_to_go_type(ttype);

    string maybepointer;
    if (is_pointer_field(*m_iter)) {
      string go_opt_type = type_to_go_type_with_opt(ttype, true);
      maybepointer = go_opt_type != go_type ? "*" : "";
    }

    out << indent() << "func (" << receiver_name << " *" << tstruct_name << ") "
        << method_name << "(" << arg_name << " " << maybepointer << go_type
        << ") *" << tstruct_name << " {" << endl;
    indent_up();
    out << indent() << receiver_name << "." << publicized_name << " = "
        << arg_name << endl;
    out << indent() << "return " << receiver_name << endl;
    indent_down();
    out << indent() << "}" << endl << endl;
  }
}

/**
 * Generates the CountSetFields helper method for a struct
 */
void t_go_generator::generate_countsetfields_helper(
    ofstream& out,
    const t_structured* tstruct,
    const string& tstruct_name,
    bool is_result) {
  (void)is_result;
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  const string escaped_tstruct_name(escape_string(tstruct->get_name()));

  out << indent() << "func (p *" << tstruct_name << ") CountSetFields"
      << tstruct_name << "() int {" << endl;
  indent_up();
  out << indent() << "count := 0" << endl;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if ((*f_iter)->get_req() != t_field::e_req::optional &&
        !is_pointer_field(*f_iter)) {
      continue;
    }

    const string field_name(publicize(escape_string((*f_iter)->get_name())));

    out << indent() << "if (p.IsSet" << field_name << "()) {" << endl;
    indent_up();
    out << indent() << "count++" << endl;
    indent_down();
    out << indent() << "}" << endl;
  }

  out << indent() << "return count" << endl << endl;
  indent_down();
  out << indent() << "}" << endl << endl;
}

/**
 * Generates the read method for a struct
 */
void t_go_generator::generate_go_struct_reader(
    ofstream& out,
    const t_structured* tstruct,
    const string& tstruct_name,
    bool is_result) {
  (void)is_result;
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  string escaped_tstruct_name(escape_string(tstruct->get_name()));
  out << indent() << "func (p *" << tstruct_name
      << ") Read(iprot thrift.Protocol) error {" << endl;
  indent_up();
  out << indent() << "if _, err := iprot.ReadStructBegin(); err != nil {"
      << endl;
  out << indent()
      << "  return thrift.PrependError(fmt.Sprintf(\"%T read error: \", p), "
         "err)"
      << endl;
  out << indent() << "}" << endl << endl;

  // Required variables does not have IsSet functions, so we need tmp vars to
  // check them.
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if ((*f_iter)->get_req() == t_field::e_req::required) {
      const string field_name(publicize(escape_string((*f_iter)->get_name())));
      indent(out) << "var isset" << field_name << " bool = false;" << endl;
    }
  }
  out << endl;

  // Loop over reading in fields
  indent(out) << "for {" << endl;
  indent_up();
  // Read beginning field marker
  out << indent() << "_, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()"
      << endl;
  out << indent() << "if err != nil {" << endl;
  indent_up();
  out << indent()
      << "return thrift.PrependError(fmt.Sprintf("
         "\"%T field %d read error: \", p, fieldId), err)"
      << endl;
  indent_down();
  out << indent() << "}" << endl;
  // Check for field STOP marker and break
  out << indent() << "if fieldTypeId == thrift.STOP { break; }" << endl;

  string thriftFieldTypeId;
  // Generate deserialization code for known cases
  set<int32_t> seen;

  // Switch statement on the field we are reading, false if no fields present
  bool have_switch = !fields.empty();
  if (have_switch) {
    indent(out) << "switch fieldId {" << endl;
  }

  // All the fields we know
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    int32_t field_id = (*f_iter)->get_key();

    // -1 -> ReadField_1, 1 -> ReadField1
    string field_method("ReadField");
    field_method += (field_id < 0 ? "_" : "");
    field_method += std::to_string(std::abs(field_id));

    if (seen.find(field_id) != seen.end()) {
      continue;
    }
    seen.insert(field_id);

    out << indent() << "case " << field_id << ":" << endl;
    indent_up();
    thriftFieldTypeId = type_to_enum((*f_iter)->get_type());

    out << indent() << "if err := p." << field_method << "(iprot); err != nil {"
        << endl;
    out << indent() << "  return err" << endl;
    out << indent() << "}" << endl;

    // Mark required field as read
    if ((*f_iter)->get_req() == t_field::e_req::required) {
      const string field_name(publicize(escape_string((*f_iter)->get_name())));
      out << indent() << "isset" << field_name << " = true" << endl;
    }

    indent_down();
  }

  // Begin switch default case
  if (have_switch) {
    out << indent() << "default:" << endl;
    indent_up();
  }

  // Skip unknown fields in either case
  out << indent() << "if err := iprot.Skip(fieldTypeId); err != nil {" << endl;
  out << indent() << "  return err" << endl;
  out << indent() << "}" << endl;

  // End switch default case
  if (have_switch) {
    indent_down();
    out << indent() << "}" << endl;
  }

  // Read field end marker
  out << indent() << "if err := iprot.ReadFieldEnd(); err != nil {" << endl;
  out << indent() << "  return err" << endl;
  out << indent() << "}" << endl;
  indent_down();
  out << indent() << "}" << endl;
  out << indent() << "if err := iprot.ReadStructEnd(); err != nil {" << endl;
  out << indent()
      << "  return thrift.PrependError(fmt.Sprintf("
         "\"%T read struct end error: \", p), err)"
      << endl;
  out << indent() << "}" << endl;

  // Return error if any required fields are missing.
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if ((*f_iter)->get_req() == t_field::e_req::required) {
      const string field_name(publicize(escape_string((*f_iter)->get_name())));
      out << indent() << "if !isset" << field_name << "{" << endl;
      out << indent()
          << "  return "
             "thrift.NewProtocolExceptionWithType(thrift.INVALID_DATA, "
             "fmt.Errorf(\"Required field "
          << field_name << " is not set\"));" << endl;
      out << indent() << "}" << endl;
    }
  }

  out << indent() << "return nil" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    string field_type_name(publicize((*f_iter)->get_type()->get_name()));
    string field_name(publicize((*f_iter)->get_name()));
    int32_t field_id = (*f_iter)->get_key();
    // -1 -> ReadField_1, 1 -> ReadField1
    string field_method("ReadField");
    field_method += (field_id < 0 ? "_" : "");
    field_method += std::to_string(std::abs(field_id));

    out << indent() << "func (p *" << tstruct_name << ")  " << field_method
        << "(iprot thrift.Protocol) error {" << endl;
    indent_up();
    generate_deserialize_field(out, *f_iter, false, "p.");
    indent_down();
    out << indent() << "  return nil" << endl;
    out << indent() << "}" << endl << endl;
  }
}

void t_go_generator::generate_go_struct_writer(
    ofstream& out,
    const t_structured* tstruct,
    const string& tstruct_name,
    bool is_result,
    bool uses_countsetfields) {
  (void)is_result;
  const string& name = tstruct->get_name();
  const vector<t_field*>& fields = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator f_iter;
  indent(out) << "func (p *" << tstruct_name
              << ") Write(oprot thrift.Protocol) error {" << endl;
  indent_up();
  if (tstruct->is_union() && uses_countsetfields) {
    std::string pub_tstruct_name(publicize(tstruct->get_name()));
    out << indent() << "if c := p.CountSetFields" << pub_tstruct_name
        << "(); c > 1 {" << endl
        << indent()
        << "  return fmt.Errorf(\"%T write union: no more than one field must "
           "be set (%d set).\", p, c)"
        << endl
        << indent() << "}" << endl;
  }
  out << indent() << "if err := oprot.WriteStructBegin(\"" << name
      << "\"); err != nil {" << endl;
  out << indent()
      << "  return thrift.PrependError(fmt.Sprintf("
         "\"%T write struct begin error: \", p), err) }"
      << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    int32_t field_id = (*f_iter)->get_key();
    const string& field_name = (*f_iter)->get_name();
    string escape_field_name = escape_string(field_name);
    // -1 -> writeField_1, 1 -> writeField1
    string field_method("writeField");
    field_method += (field_id < 0 ? "_" : "");
    field_method += std::to_string(std::abs(field_id));

    out << indent() << "if err := p." << field_method
        << "(oprot); err != nil { return err }" << endl;
  }

  // Write the struct map
  out << indent() << "if err := oprot.WriteFieldStop(); err != nil {" << endl;
  out << indent()
      << "  return thrift.PrependError(\"write field stop error: \", err) }"
      << endl;
  out << indent() << "if err := oprot.WriteStructEnd(); err != nil {" << endl;
  out << indent()
      << "  return thrift.PrependError(\"write struct stop error: \", err) }"
      << endl;
  out << indent() << "return nil" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    int32_t field_id = (*f_iter)->get_key();
    const string& field_name = (*f_iter)->get_name();
    string escape_field_name = escape_string(field_name);
    t_field::e_req field_required = (*f_iter)->get_req();
    // -1 -> writeField_1, 1 -> writeField1
    string field_method("writeField");
    field_method += (field_id < 0 ? "_" : "");
    field_method += std::to_string(std::abs(field_id));

    out << indent() << "func (p *" << tstruct_name << ") " << field_method
        << "(oprot thrift.Protocol) (err error) {" << endl;
    indent_up();

    if (field_required == t_field::e_req::optional) {
      out << indent() << "if p.IsSet" << publicize(field_name) << "() {"
          << endl;
      indent_up();
    }

    out << indent() << "if err := oprot.WriteFieldBegin(\"" << escape_field_name
        << "\", " << type_to_enum((*f_iter)->get_type()) << ", " << field_id
        << "); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(fmt.Sprintf(\"%T write field begin "
           "error "
        << field_id << ":" << escape_field_name << ": \", p), err) }" << endl;

    // Write field contents
    generate_serialize_field(out, *f_iter, "p.");

    // Write field closer
    out << indent() << "if err := oprot.WriteFieldEnd(); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(fmt.Sprintf(\"%T write field end "
           "error "
        << field_id << ":" << escape_field_name << ": \", p), err) }" << endl;

    if (field_required == t_field::e_req::optional) {
      indent_down();
      out << indent() << "}" << endl;
    }

    indent_down();
    out << indent() << "  return err" << endl;
    out << indent() << "}" << endl << endl;
  }
}

/**
 * Generates a thrift service.
 *
 * @param tservice The service definition
 */
void t_go_generator::generate_service(const t_service* tservice) {
  string test_suffix("_test");
  string filename = lowercase(service_name_);
  string f_service_name;

  size_t fname_len = filename.length();
  size_t suffix_len = test_suffix.length();

  if ((fname_len >= suffix_len) &&
      (filename.compare(fname_len - suffix_len, suffix_len, test_suffix) ==
       0)) {
    f_service_name = package_dir_ + "/" + filename + "_.go";
  } else {
    f_service_name = package_dir_ + "/" + filename + ".go";
  }
  f_service_.open(f_service_name.c_str());
  f_service_ << go_autogen_comment() << go_package() << render_includes();

  generate_service_interface(tservice);
  generate_service_client_interface(tservice);
  generate_service_client(tservice, false);
  generate_service_client(tservice, true); // threadsafe client
  generate_service_client_channel(tservice); // channel client
  generate_service_server(tservice);
  generate_service_helpers(tservice);
  // Close service file
  f_service_ << endl;
  f_service_.close();
}

/**
 * Generates helper functions for a service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_go_generator::generate_service_helpers(const t_service* tservice) {
  vector<const t_function*> functions = get_supported_functions(tservice);
  f_service_ << "// HELPER FUNCTIONS AND STRUCTURES" << endl << endl;

  for (auto func : functions) {
    const t_structured& ts = func->params();
    generate_go_struct_definition(f_service_, &ts, false, false, true);
    generate_go_function_helpers(func);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_go_generator::generate_go_function_helpers(const t_function* tfunction) {
  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    t_struct result(program_, tfunction->get_name() + "_result");
    auto success =
        std::make_unique<t_field>(tfunction->return_type(), "success", 0);
    success->set_req(t_field::e_req::optional);

    if (!tfunction->return_type()->is_void()) {
      result.append(std::move(success));
    }

    for (const t_field& x : get_elems(tfunction->exceptions())) {
      auto new_f = x.clone_DO_NOT_USE();
      new_f->set_req(t_field::e_req::optional);
      result.append(std::move(new_f));
    }

    generate_go_struct_definition(f_service_, &result, false, true);
  }
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 */
void t_go_generator::generate_service_interface(const t_service* tservice) {
  string extends = "";
  string extends_if = "";
  string serviceName(publicize(tservice->get_name()));
  const string& interfaceName = serviceName;

  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    size_t index = extends.rfind('.');

    if (index != string::npos) {
      extends_if = "\n" + indent() + "  " + extends.substr(0, index + 1) +
          publicize(extends.substr(index + 1)) + "\n";
    } else {
      extends_if = "\n" + indent() + publicize(extends) + "\n";
    }
  }

  f_service_ << indent() << "type " << interfaceName << " interface {"
             << extends_if;
  indent_up();
  generate_go_docstring(f_service_, tservice);
  vector<const t_function*> functions = get_supported_functions(tservice);

  if (!functions.empty()) {
    f_service_ << endl;

    for (const auto* func : functions) {
      generate_go_docstring(f_service_, func);

      f_service_ << indent()
                 << function_signature_if(func, "", gen_use_context_) << endl;
    }
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

void t_go_generator::generate_service_client_method(
    string& clientTypeName, const t_function* func, bool isThreadsafe) {
  string funname = publicize(func->get_name());
  // Open function
  generate_go_docstring(f_service_, func);
  f_service_ << indent() << "func (p *" << clientTypeName << ") "
             << function_signature_if(func, "", false) << " {" << endl;
  indent_up();
  /*
  f_service_ <<
    indent() << "p.SeqId += 1" << endl;
  if ((*f_iter)->qualifier() != t_function_qualifier::oneway) {
    f_service_ <<
      indent() << "d := defer.Deferred()" << endl <<
      indent() << "p.Reqs[p.SeqId] = d" << endl;
  }
  */

  if (isThreadsafe) {
    f_service_ << indent() << "p.Mu.Lock()" << endl;
    f_service_ << indent() << "defer p.Mu.Unlock()" << endl;
  }

  generate_service_client_send_msg_call(func);

  f_service_ << indent() << "if err != nil { return }" << endl;

  if (func->qualifier() != t_function_qualifier::oneway) {
    f_service_ << indent() << "return p.recv" << funname << "()" << endl;
  } else {
    f_service_ << indent() << "return" << endl;
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  if (func->qualifier() != t_function_qualifier::oneway) {
    generate_service_client_recv_method(clientTypeName, func);
  }
}

void t_go_generator::generate_service_client_channel_method(
    string& clientTypeName, const t_function* func) {
  string funname = publicize(func->get_name());
  // Open function
  generate_go_docstring(f_service_, func);
  f_service_ << indent() << "func (p *" << clientTypeName << ") "
             << function_signature_if(func, "", true) << " {" << endl;
  indent_up();

  generate_service_client_channel_call(func);
}

void t_go_generator::generate_service_client_channel_call(
    const t_function* func) {
  auto methodName = func->get_name();
  auto result_type_name = publicize(func->get_name() + "_result", true);
  auto argsType = publicize(methodName + "_args", true);
  auto isOneway = func->qualifier() == t_function_qualifier::oneway;
  auto returnsVoid = func->return_type()->is_void();

  const t_structured& arg_struct = func->params();
  const auto& fields = arg_struct.get_members();

  // Initialize request struct
  f_service_ << indent() << "args := " << argsType << "{" << endl;
  indent_up();
  for (auto fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
    f_service_ << indent() << publicize((*fld_iter)->get_name()) << " : "
               << variable_name_to_go_name((*fld_iter)->get_name()) << ","
               << endl;
  }
  indent_down();
  f_service_ << indent() << "}" << endl;

  if (!isOneway) {
    // Declare response struct
    f_service_ << indent() << "var __result " << result_type_name << endl;
  }

  if (isOneway) {
    f_service_ << indent() << "err = p.RequestChannel.Oneway(ctx, \""
               << methodName << "\", &args)" << endl;
  } else {
    f_service_ << indent() << "err = p.RequestChannel.Call(ctx, \""
               << methodName << "\", &args, &__result)" << endl;
  }

  f_service_ << indent() << "if err != nil { return }" << endl;

  // If there are no exceptions, no code is generated. So we can always call
  // this.
  generate_service_client_recv_method_exception_handling(func->exceptions());

  // Careful, only return _result if not a void function
  if (returnsVoid || isOneway) {
    f_service_ << indent() << "return nil" << endl;
  } else {
    f_service_ << indent() << "return __result.GetSuccess(), nil" << endl;
  }

  // Close function
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

void t_go_generator::generate_service_client_send_msg_call(
    const t_function* func) {
  auto methodName = func->get_name();
  auto argsType = publicize(methodName + "_args", true);
  auto messageType = func->qualifier() == t_function_qualifier::oneway
      ? "thrift.ONEWAY"
      : "thrift.CALL";

  const t_structured& arg_struct = func->params();
  const auto& fields = arg_struct.get_members();

  // If there are no fields in the struct, don't bother initializing anything
  if (fields.size() == 0) {
    f_service_ << indent() << "var args " << argsType << endl;
  } else {
    f_service_ << indent() << "args := " << argsType << "{" << endl;
    indent_up();
    for (auto fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      f_service_ << indent() << publicize((*fld_iter)->get_name()) << " : "
                 << variable_name_to_go_name((*fld_iter)->get_name()) << ","
                 << endl;
    }
    indent_down();
    f_service_ << indent() << "}" << endl;
  }

  f_service_ << indent() << "err = p.CC.SendMsg(\"" << methodName
             << "\", &args, " << messageType << ")" << endl;
}

void t_go_generator::generate_service_client_recv_method(
    string& clientTypeName, const t_function* func) {
  std::string result_type_name = publicize(func->get_name() + "_result", true);
  auto methodName = func->get_name();
  auto returnsVoid = func->return_type()->is_void();
  bool raisesExceptions = !get_elems(func->exceptions()).empty();

  // Open function
  f_service_ << endl
             << indent() << "func (p *" << clientTypeName << ") recv"
             << publicize(func->get_name()) << "() (";

  if (!returnsVoid) {
    f_service_ << "value " << type_to_go_type(func->return_type().get_type())
               << ", ";
  }

  f_service_ << "err error) {" << endl;
  indent_up();
  f_service_ << indent() << "var __result " << result_type_name << endl;

  if (returnsVoid && !raisesExceptions) {
    f_service_ << indent() << "return p.CC.RecvMsg(\"" << methodName
               << "\", &__result)" << endl;
  } else {
    f_service_ << indent() << "err = p.CC.RecvMsg(\"" << methodName
               << "\", &__result)" << endl;
    f_service_ << indent() << "if err != nil { return }" << endl;

    // If there are no exceptions, no code is generated. So we can always call
    // this.
    generate_service_client_recv_method_exception_handling(func->exceptions());

    // Careful, only return _result if not a void function
    if (!returnsVoid) {
      f_service_ << indent() << "return __result.GetSuccess(), nil" << endl;
    } else {
      f_service_ << indent() << "return nil" << endl;
    }
  }

  // Close function
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

void t_go_generator::generate_service_client_recv_method_exception_handling(
    const t_throws* exceptions) {
  // NOTE: Trick to avoid copying the entire string every iteration of
  // the for loop below. We just copy the pointer to the string instead of the
  // whole thing.
  auto indent_str = indent();
  auto else_delim = " else ";
  auto* delim = indent_str.c_str();

  for (const t_field& x : get_elems(exceptions)) {
    const std::string pubname = publicize(x.get_name());

    f_service_ << delim << "if __result." << pubname << " != nil {" << endl;
    f_service_ << indent() << "  err = __result." << pubname << endl;
    f_service_ << indent() << "  return " << endl;
    f_service_ << indent() << "}";
    delim = else_delim;
  }
  f_service_ << endl;
}

void t_go_generator::generate_service_client_common_methods(
    string& clientTypeName, bool isThreadsafe) {
  for (auto& method : common_client_methods_) {
    f_service_ << indent() << "func(client *" << clientTypeName << ") "
               << method.name << "() " << method.return_type << " {" << endl;

    indent_up();
    if (isThreadsafe) {
      f_service_ << indent() << "client.Mu.Lock()" << endl;
      f_service_ << indent() << "defer client.Mu.Unlock()" << endl;
    }

    f_service_ << indent() << "return client.CC." << method.name << "()"
               << endl;
    indent_down();
    f_service_ << indent() << "}" << endl << endl;
  }
}

string t_go_generator::make_client_interface_name(const t_service* tservice) {
  return publicize(tservice->get_name()) + "ClientInterface";
}

void t_go_generator::generate_service_client_interface(
    const t_service* tservice) {
  auto clientInterfaceName = make_client_interface_name(tservice);
  f_service_ << "type " << clientInterfaceName << " interface {" << endl;
  indent_up();

  // Embed the base ClientInterface into this one
  f_service_ << indent() << "thrift.ClientInterface" << endl;

  for (auto* function : get_supported_functions(tservice)) {
    generate_go_docstring(f_service_, function);
    f_service_ << indent() << function_signature_if(function, "", false)
               << endl;
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a client for.
 * @param isThreadsafe indicates if the generated client should be
 *     threadsafe.
 */
void t_go_generator::generate_service_client(
    const t_service* tservice, bool isThreadsafe) {
  string extends = "";
  string extends_field = "";
  string extends_client = "";
  string extends_client_new = "";
  string serviceName(publicize(tservice->get_name()));
  string clientTypeNameSuffix = (isThreadsafe) ? "ThreadsafeClient" : "Client";
  string clientTypeName = serviceName + clientTypeNameSuffix;

  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    size_t index = extends.rfind('.');

    if (index != string::npos) {
      extends_client = extends.substr(0, index + 1) +
          publicize(extends.substr(index + 1)) + clientTypeNameSuffix;
      extends_client_new = extends.substr(0, index + 1) + "New" +
          publicize(extends.substr(index + 1)) + clientTypeNameSuffix;
    } else {
      extends_client = publicize(extends) + clientTypeNameSuffix;
      extends_client_new = "New" + extends_client;
    }
  }

  extends_field = extends_client.substr(extends_client.find('.') + 1);

  generate_go_docstring(f_service_, tservice);
  f_service_ << indent() << "type " << clientTypeName << " struct {" << endl;
  indent_up();

  // Force the client struct to implement the expected interface.
  f_service_ << indent() << make_client_interface_name(tservice) << endl;

  if (!extends_client.empty()) {
    f_service_ << indent() << "*" << extends_client << endl;
  } else {
    f_service_ << indent() << "CC thrift.ClientConn" << endl;
    if (isThreadsafe) {
      f_service_ << indent() << "Mu sync.Mutex" << endl;
    }
    /*f_service_ << indent() << "reqs map[int32]Deferred" << endl*/;
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  generate_service_client_common_methods(clientTypeName, isThreadsafe);

  // Constructor function
  f_service_ << indent() << "func New" << clientTypeName
             << "Factory(t thrift.Transport, f thrift.ProtocolFactory) *"
             << clientTypeName << " {" << endl;
  indent_up();
  f_service_ << indent() << "return &" << clientTypeName;

  if (!extends.empty()) {
    f_service_ << "{" << extends_field << ": " << extends_client_new
               << "Factory(t, f)}" << endl;
  } else {
    f_service_ << "{ CC: thrift.NewClientConn(t, f) }" << endl;
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // Constructor function
  f_service_
      << indent() << "func New" << clientTypeName
      << "(t thrift.Transport, iprot thrift.Protocol, oprot thrift.Protocol) *"
      << clientTypeName << " {" << endl;
  indent_up();
  f_service_ << indent() << "return &" << clientTypeName;

  if (!extends.empty()) {
    f_service_ << "{" << extends_field << ": " << extends_client_new
               << "(t, iprot, oprot)}" << endl;
  } else {
    f_service_ << "{ CC: thrift.NewClientConnWithProtocols(t, iprot, oprot) }"
               << endl;
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // Constructor function with protocol only
  f_service_ << indent() << "func New" << clientTypeName << "Protocol"
             << "(prot thrift.Protocol) *" << clientTypeName << " {" << endl;
  indent_up();
  f_service_ << indent() << "return New" << clientTypeName
             << "(prot.Transport(), prot, prot)" << endl;

  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // Generate client method implementations
  for (auto* func : get_supported_functions(tservice)) {
    generate_service_client_method(clientTypeName, func, isThreadsafe);
  }

  // indent_down();
  f_service_ << endl;
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a client for.
 * @param isThreadsafe indicates if the generated client should be
 *     threadsafe.
 */
void t_go_generator::generate_service_client_channel(
    const t_service* tservice) {
  string extends = "";
  string extends_field = "";
  string extends_client = "";
  string extends_client_new = "";
  string serviceName(publicize(tservice->get_name()));
  string clientTypeNameSuffix = "ChannelClient";
  string clientTypeName = serviceName + clientTypeNameSuffix;

  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    size_t index = extends.rfind('.');

    if (index != string::npos) {
      extends_client = extends.substr(0, index + 1) +
          publicize(extends.substr(index + 1)) + clientTypeNameSuffix;
      extends_client_new = extends.substr(0, index + 1) + "New" +
          publicize(extends.substr(index + 1)) + clientTypeNameSuffix;
    } else {
      extends_client = publicize(extends) + clientTypeNameSuffix;
      extends_client_new = "New" + extends_client;
    }
  }

  extends_field = extends_client.substr(extends_client.find('.') + 1);

  generate_go_docstring(f_service_, tservice);
  f_service_ << indent() << "type " << clientTypeName << " struct {" << endl;
  indent_up();

  if (!extends_client.empty()) {
    f_service_ << indent() << "*" << extends_client << endl;
  } else {
    f_service_ << indent() << "RequestChannel thrift.RequestChannel" << endl;
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // Close method
  f_service_ << indent() << "func (c *" << clientTypeName << ") Close() error {"
             << endl;
  indent_up();
  f_service_ << indent() << "return c.RequestChannel.Close()" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // IsOpen method
  f_service_ << indent() << "func (c *" << clientTypeName << ") IsOpen() bool {"
             << endl;
  indent_up();
  f_service_ << indent() << "return c.RequestChannel.IsOpen()" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // Open method
  f_service_ << indent() << "func (c *" << clientTypeName << ") Open() error {"
             << endl;
  indent_up();
  f_service_ << indent() << "return c.RequestChannel.Open()" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // Constructor function
  f_service_ << indent() << "func New" << clientTypeName
             << "(channel thrift.RequestChannel) *" << clientTypeName << " {"
             << endl;
  indent_up();

  f_service_ << indent() << "return &" << clientTypeName;
  if (!extends.empty()) {
    f_service_ << "{" << extends_field << ": " << extends_client_new
               << "(channel)}" << endl;
  } else {
    f_service_ << "{RequestChannel: channel}" << endl;
  }

  indent_down();
  f_service_ << indent() << "}" << endl << endl;

  // Generate client method implementations
  for (const auto* func : get_supported_functions(tservice)) {
    generate_service_client_channel_method(clientTypeName, func);
  }

  // indent_down();
  f_service_ << endl;
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_go_generator::generate_service_server(const t_service* tservice) {
  // Generate the dispatch methods
  vector<const t_function*> functions = get_supported_functions(tservice);
  string extends = "";
  string extends_processor = "";
  string extends_processor_new = "";
  string serviceName(publicize(tservice->get_name()));

  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    size_t index = extends.rfind('.');

    if (index != string::npos) {
      extends_processor = extends.substr(0, index + 1) +
          publicize(extends.substr(index + 1)) + "Processor";
      extends_processor_new = extends.substr(0, index + 1) + "New" +
          publicize(extends.substr(index + 1)) + "Processor";
    } else {
      extends_processor = publicize(extends) + "Processor";
      extends_processor_new = "New" + extends_processor;
    }
  }

  string pServiceName(privatize(serviceName));
  // Generate the header portion
  string self(tmp("self"));

  // TODO should handle extends?
  if (extends_processor.empty()) {
    f_service_ << indent() << "type " << serviceName << "Processor struct {"
               << endl;
    f_service_ << indent() << "  processorMap map[string]"
               << gen_processor_func_ << endl;
    f_service_ << indent() << "  functionServiceMap map[string]string" << endl;
    f_service_ << indent() << "  handler " << serviceName << endl;
    f_service_ << indent() << "}" << endl << endl;

    f_service_ << indent() << "func (p *" << serviceName
               << "Processor) AddToProcessorMap(key string, processor "
               << gen_processor_func_ << ") {" << endl;
    f_service_ << indent() << "  p.processorMap[key] = processor" << endl;
    f_service_ << indent() << "}" << endl << endl;

    f_service_ << indent() << "func (p *" << serviceName
               << "Processor) AddToFunctionServiceMap(key, service string) {"
               << endl;
    f_service_ << indent() << "  p.functionServiceMap[key] = service" << endl;
    f_service_ << indent() << "}" << endl << endl;

    f_service_ << indent() << "func (p *" << serviceName << "Processor) ";
    if (gen_use_context_) {
      f_service_ << "GetProcessorFunctionContext(key string) ";
    } else {
      f_service_ << "GetProcessorFunction(key string) ";
    }
    f_service_ << "(processor " << gen_processor_func_ << ", err error) {"
               << endl;
    indent_up();
    f_service_ << indent() << "if processor, ok := p.processorMap[key]; ok {"
               << endl;
    f_service_ << indent() << "  return processor, nil" << endl;
    f_service_ << indent() << "}" << endl;
    f_service_ << indent()
               << "return nil, nil // generic error message will be sent"
               << endl;
    indent_down();
    f_service_ << indent() << "}" << endl << endl;
    f_service_ << indent() << "func (p *" << serviceName
               << "Processor) ProcessorMap() map[string]" << gen_processor_func_
               << " {" << endl;
    f_service_ << indent() << "  return p.processorMap" << endl;
    f_service_ << indent() << "}" << endl << endl;
    f_service_ << indent() << "func (p *" << serviceName
               << "Processor) FunctionServiceMap() map[string]string {" << endl;
    f_service_ << indent() << "  return p.functionServiceMap" << endl;
    f_service_ << indent() << "}" << endl << endl;
    f_service_ << indent() << "func New" << serviceName << "Processor(handler "
               << serviceName << ") *" << serviceName << "Processor {" << endl;
    f_service_ << indent() << "  " << self << " := &" << serviceName
               << "Processor{handler:handler, "
                  "processorMap:make(map[string]"
               << gen_processor_func_
               << "), functionServiceMap:make(map[string]string)}" << endl;

    indent_up();
    for (const auto* func : functions) {
      string escapedFuncName(escape_string(func->get_name()));
      f_service_ << indent() << self << ".processorMap[\"" << escapedFuncName
                 << "\"] = &" << pServiceName << "Processor"
                 << publicize(func->get_name()) << "{handler:handler}" << endl;
    }
    for (const auto* func : functions) {
      string escapedFuncName(escape_string(func->get_name()));
      f_service_ << indent() << self << ".functionServiceMap[\""
                 << escapedFuncName << "\"] = \"" << serviceName << "\""
                 << endl;
    }

    string x(tmp("x"));
    f_service_ << indent() << "return " << self << endl;
    indent_down();
    f_service_ << indent() << "}" << endl << endl;

  } else {
    f_service_ << indent() << "type " << serviceName << "Processor struct {"
               << endl;
    f_service_ << indent() << "  *" << extends_processor << endl;
    f_service_ << indent() << "}" << endl << endl;

    f_service_ << indent() << "func New" << serviceName << "Processor(handler "
               << serviceName << ") *" << serviceName << "Processor {" << endl;
    f_service_ << indent() << "  " << self << " := &" << serviceName
               << "Processor{" << extends_processor_new << "(handler)}" << endl;

    for (const auto* func : functions) {
      string escapedFuncName(escape_string(func->get_name()));
      f_service_ << indent() << "  " << self << ".AddToProcessorMap(\""
                 << escapedFuncName << "\", &" << pServiceName << "Processor"
                 << publicize((func)->get_name()) << "{handler:handler})"
                 << endl;
    }

    for (const auto* func : functions) {
      string escapedFuncName(escape_string(func->get_name()));
      f_service_ << indent() << "  " << self << ".AddToFunctionServiceMap(\""
                 << escapedFuncName << "\", \"" << serviceName << "\")" << endl;
    }

    f_service_ << indent() << "  return " << self << endl;
    f_service_ << indent() << "}" << endl << endl;
  }

  // Generate the process subfunctions
  for (const auto* func : functions) {
    generate_process_function_type(tservice, func);
    generate_struct_error_result_fn(tservice, func);
    generate_read_function(tservice, func);
    generate_write_function(tservice, func);
    generate_run_function(tservice, func);
  }

  f_service_ << endl;
}
void t_go_generator::generate_process_function_type(
    const t_service* tservice, const t_function* tfunction) {
  string processorName = privatize(tservice->get_name()) + "Processor" +
      publicize(tfunction->get_name());
  f_service_ << indent() << "type " << processorName << " struct {" << endl;
  f_service_ << indent() << "  handler " << publicize(tservice->get_name())
             << endl;
  f_service_ << indent() << "}" << endl << endl;
}

void t_go_generator::generate_read_function(
    const t_service* tservice, const t_function* tfunction) {
  // Open function
  string processorName = privatize(tservice->get_name()) + "Processor" +
      publicize(tfunction->get_name());
  string argsname = publicize(tfunction->get_name() + "_args", true);
  f_service_ << indent() << "func (p *" << processorName
             << ") Read(iprot thrift.Protocol) "
             << "(thrift.Struct, thrift.Exception) {" << endl;
  indent_up();
  f_service_ << indent() << "args := " << argsname << "{}" << endl;
  f_service_ << indent() << "if err := args.Read(iprot); err != nil {" << endl;
  indent_up();
  f_service_ << indent() << "return nil, err" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl;
  f_service_ << indent() << "iprot.ReadMessageEnd()" << endl;
  f_service_ << indent() << "return &args, nil" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

void t_go_generator::generate_struct_error_result_fn(
    const t_service* tservice, const t_function* tfunction) {
  // Open function
  string processorName = privatize(tservice->get_name()) + "Processor" +
      publicize(tfunction->get_name());
  string argsname = publicize(tfunction->get_name() + "_args", true);
  string resultname = publicize(tfunction->get_name() + "_result", true);

  if (tfunction->qualifier() == t_function_qualifier::oneway) {
    return;
  }

  f_service_ << indent() << "func (p *" << resultname
             << ") Exception() thrift.WritableException {" << endl;
  indent_up();
  f_service_ << indent() << "if p == nil { return nil }" << endl;
  for (const t_field& x : get_elems(tfunction->exceptions())) {
    indent(f_service_) << "if p." << publicize(x.get_name()) << " != nil {"
                       << endl;
    indent_up();
    indent(f_service_) << "return p." << publicize(x.get_name()) << endl;
    indent_down();
    f_service_ << indent() << "}" << endl;
  }
  f_service_ << indent() << "return nil" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

void t_go_generator::generate_write_function(
    const t_service* tservice, const t_function* tfunction) {
  // Open function
  string processorName = privatize(tservice->get_name()) + "Processor" +
      publicize(tfunction->get_name());
  string argsname = publicize(tfunction->get_name() + "_args", true);
  string resultname = publicize(tfunction->get_name() + "_result", true);
  f_service_ << indent() << "func (p *" << processorName
             << ") Write(seqId int32, result thrift.WritableStruct, oprot "
                "thrift.Protocol) (err "
                "thrift.Exception) {"
             << endl;
  indent_up();

  f_service_ << indent() << "var err2 error" << endl;
  f_service_ << indent() << "messageType := thrift.REPLY" << endl;

  auto exceptions = get_elems(tfunction->exceptions());
  f_service_ << indent() << "switch " << (exceptions.empty() ? "" : "v := ")
             << "result.(type) {" << endl;
  for (const t_field& x : exceptions) {
    indent(f_service_) << "case " << type_to_go_type(x.get_type()) << ":"
                       << endl;
    indent_up();
    indent(f_service_) << "msg := " << resultname << "{"
                       << publicize(x.get_name()) << ": v}" << endl;
    indent(f_service_) << "result = &msg" << endl;
    indent_down();
  }
  indent(f_service_) << "case thrift.ApplicationException:" << endl;
  indent_up();
  indent(f_service_) << "messageType = thrift.EXCEPTION" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl;

  f_service_ << indent() << "if err2 = oprot.WriteMessageBegin(\""
             << escape_string(tfunction->get_name())
             << "\", messageType, seqId); err2 != nil {" << endl;
  f_service_ << indent() << "  err = err2" << endl;
  f_service_ << indent() << "}" << endl;
  f_service_ << indent()
             << "if err2 = result.Write(oprot); err == nil && err2 != nil {"
             << endl;

  f_service_ << indent() << "  err = err2" << endl;
  f_service_ << indent() << "}" << endl;
  f_service_ << indent()
             << "if err2 = oprot.WriteMessageEnd(); err == nil && err2 != nil {"
             << endl;
  f_service_ << indent() << "  err = err2" << endl;
  f_service_ << indent() << "}" << endl;
  f_service_ << indent()
             << "if err2 = oprot.Flush(); err == nil && err2 != nil {" << endl;
  f_service_ << indent() << "  err = err2" << endl;
  f_service_ << indent() << "}" << endl;
  f_service_ << indent() << "return err" << endl;
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

void t_go_generator::generate_run_function(
    const t_service* tservice, const t_function* tfunction) {
  // Open function
  string processorName = privatize(tservice->get_name()) + "Processor" +
      publicize(tfunction->get_name());
  string argsname = publicize(tfunction->get_name() + "_args", true);
  string resultname = publicize(tfunction->get_name() + "_result", true);
  f_service_ << indent() << "func (p *" << processorName << ") ";
  if (gen_use_context_) {
    f_service_ << "RunContext(ctx context.Context, argStruct thrift.Struct)";
  } else {
    f_service_ << "Run(argStruct thrift.Struct)";
  }
  f_service_ << " (thrift.WritableStruct, thrift.ApplicationException) {"
             << endl;
  indent_up();
  const t_structured& arg_struct = tfunction->params();
  const std::vector<t_field*>& fields = arg_struct.get_members();
  if (!fields.empty()) {
    indent(f_service_) << "args := argStruct.(*" << argsname << ")" << endl;
  }

  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    indent(f_service_) << "var __result " << resultname << endl;
  }
  indent(f_service_) << "if ";

  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    if (!tfunction->return_type()->is_void()) {
      f_service_ << "retval, ";
    }
  }

  // Generate the function call
  f_service_ << "err := p.handler." << publicize(get_func_name(tfunction))
             << "(";
  if (gen_use_context_) {
    f_service_ << "ctx" << (fields.empty() ? "" : ", ");
  }
  bool first = true;
  vector<t_field*>::const_iterator f_iter;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      f_service_ << ", ";
    }

    f_service_ << "args." << publicize((*f_iter)->get_name());
  }

  f_service_ << "); err != nil {" << endl;
  indent_up();

  auto exceptions = get_elems(tfunction->exceptions());
  f_service_ << indent() << "switch " << (exceptions.empty() ? "" : "v := ")
             << "err.(type) {" << endl;

  for (const t_field& x : exceptions) {
    indent(f_service_) << "case " << type_to_go_type(x.get_type()) << ":"
                       << endl;
    indent_up();
    indent(f_service_) << "__result." << publicize(x.get_name()) << " = v"
                       << endl;
    indent_down();
  }

  indent(f_service_) << "default:" << endl;
  indent_up();
  indent(f_service_)
      << "x := thrift.NewApplicationExceptionCause(thrift.INTERNAL_ERROR, "
      << "\"Internal error processing " << escape_string(tfunction->get_name())
      << ": \" + err.Error(), err)" << endl;
  indent(f_service_) << "return x, x" << endl;
  indent_down();
  indent(f_service_) << "}" << endl;
  indent_down();
  indent(f_service_) << "}"; // closes err != nil

  bool need_reference =
      type_need_reference(tfunction->return_type().get_type());

  if (tfunction->qualifier() != t_function_qualifier::oneway &&
      !tfunction->return_type()->is_void()) {
    f_service_ << " else {"
               << endl; // make sure we set Success retval only on success
    indent_up();
    indent(f_service_) << "__result.Success = ";
    if (need_reference) {
      f_service_ << "&";
    }
    f_service_ << "retval" << endl;
    indent_down();
    indent(f_service_) << "}" << endl;
  } else {
    f_service_ << endl;
  }

  if (tfunction->qualifier() != t_function_qualifier::oneway) {
    indent(f_service_) << "return &__result, nil" << endl;
  } else {
    indent(f_service_) << "return nil, nil" << endl;
  }
  indent_down();
  indent(f_service_) << "}" << endl << endl;
}

/**
 * Deserializes a field of any type.
 */
void t_go_generator::generate_deserialize_field(
    ofstream& out,
    const t_field* tfield,
    bool declare,
    string prefix,
    bool inclass,
    bool coerceData,
    bool inkey,
    bool in_container_value) {
  (void)inclass;
  (void)coerceData;
  const t_type* orig_type = tfield->get_type();
  const t_type* type = orig_type->get_true_type();
  string name = tfield->get_name();
  if (prefix != "") {
    name = prefix + publicize(tfield->get_name());
  }

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + name);
  }

  if (type->is_struct() || type->is_exception()) {
    generate_deserialize_struct(
        out,
        (t_struct*)orig_type,
        is_pointer_field(tfield, in_container_value),
        declare,
        name);
  } else if (type->is_container()) {
    generate_deserialize_container(
        out, orig_type, is_pointer_field(tfield), declare, name);
  } else if (type->is_base_type() || type->is_enum()) {
    if (declare) {
      string type_name = inkey ? type_to_go_key_type(tfield->get_type())
                               : type_to_go_type(tfield->get_type());

      out << indent() << "var " << tfield->get_name() << " " << type_name
          << endl;
    }

    indent(out) << "if v, err := iprot.";

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

      switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);

        case t_base_type::TYPE_STRING:
          out << "ReadString()";
          break;

        case t_base_type::TYPE_BINARY:
          if (!inkey) {
            out << "ReadBinary()";
          } else {
            out << "ReadString()";
          }
          break;

        case t_base_type::TYPE_BOOL:
          out << "ReadBool()";
          break;

        case t_base_type::TYPE_BYTE:
          out << "ReadByte()";
          break;

        case t_base_type::TYPE_I16:
          out << "ReadI16()";
          break;

        case t_base_type::TYPE_I32:
          out << "ReadI32()";
          break;

        case t_base_type::TYPE_I64:
          out << "ReadI64()";
          break;

        case t_base_type::TYPE_DOUBLE:
          out << "ReadDouble()";
          break;

        case t_base_type::TYPE_FLOAT:
          out << "ReadFloat()";
          break;

        default:
          throw std::runtime_error(
              "compiler error: no Go name for base type " +
              t_base_type::t_base_name(tbase));
      }
    } else if (type->is_enum()) {
      out << "ReadI32()";
    }

    out << "; err != nil {" << endl;
    indent_up();
    out << indent() << "return thrift.PrependError(\"error reading field "
        << tfield->get_key() << ": \", err)" << endl;
    indent_down();

    out << indent() << "} else {" << endl;
    indent_up();
    string wrap;

    if (type->is_binary() && inkey) {
      wrap = "";
    } else if (type->is_enum() || orig_type->is_typedef()) {
      wrap = publicize(type_name(orig_type));
    } else if (((t_base_type*)type)->get_base() == t_base_type::TYPE_BYTE) {
      wrap = "int8";
    }

    string maybe_address = (is_pointer_field(tfield) ? "&" : "");
    if (wrap == "") {
      indent(out) << name << " = " << maybe_address << "v" << endl;
    } else {
      indent(out) << "temp := " << wrap << "(v)" << endl;
      indent(out) << name << " = " << maybe_address << "temp" << endl;
    }

    indent_down();
    out << indent() << "}" << endl;
  } else {
    throw std::runtime_error(
        "INVALID TYPE IN generate_deserialize_field '" + type->get_name() +
        "' for field '" + tfield->get_name() + "'");
  }
}

/**
 * Generates an unserializer for a struct, calling read()
 */
void t_go_generator::generate_deserialize_struct(
    ofstream& out,
    const t_struct* tstruct,
    bool pointer_field,
    bool declare,
    string prefix) {
  string eq(declare ? " := " : " = ");

  out << indent() << prefix << eq << (pointer_field ? "" : "*");

  const t_program* program = tstruct->program();
  if (program != nullptr && program != program_) {
    out << package_identifiers[get_real_go_module(program)] << ".";
  }
  out << "New" << publicize(tstruct->get_name()) << "()" << endl;

  out << indent() << "if err := " << prefix << ".Read(iprot); err != nil {"
      << endl;
  out << indent()
      << "  return thrift.PrependError(fmt.Sprintf(\"%T error reading struct: "
         "\", "
      << prefix << "), err)" << endl;
  out << indent() << "}" << endl;
}

/**
 * Serialize a container by writing out the header followed by
 * data and then a footer.
 */
void t_go_generator::generate_deserialize_container(
    ofstream& out,
    const t_type* orig_type,
    bool pointer_field,
    bool declare,
    string prefix) {
  const t_type* ttype = orig_type->get_true_type();
  string eq(" = ");

  if (declare) {
    eq = " := ";
  }

  // Declare variables, read header
  if (ttype->is_map()) {
    out << indent() << "_, _, size, err := iprot.ReadMapBegin()" << endl;
    out << indent() << "if err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error reading map begin: \", err)"
        << endl;
    out << indent() << "}" << endl;
    out << indent() << "tMap := make(" << type_to_go_type(orig_type)
        << ", size)" << endl;
    out << indent() << prefix << eq << " " << (pointer_field ? "&" : "")
        << "tMap" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "_, size, err := iprot.ReadSetBegin()" << endl;
    out << indent() << "if err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error reading set begin: \", err)"
        << endl;
    out << indent() << "}" << endl;
    out << indent() << "tSet := make(" << type_to_go_type(orig_type)
        << ", 0, size)" << endl;
    out << indent() << prefix << eq << " " << (pointer_field ? "&" : "")
        << "tSet" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "_, size, err := iprot.ReadListBegin()" << endl;
    out << indent() << "if err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error reading list begin: \", err)"
        << endl;
    out << indent() << "}" << endl;
    out << indent() << "tSlice := make(" << type_to_go_type(orig_type)
        << ", 0, size)" << endl;
    out << indent() << prefix << eq << " " << (pointer_field ? "&" : "")
        << "tSlice" << endl;
  } else {
    throw std::runtime_error(
        "INVALID TYPE IN generate_deserialize_container '" + ttype->get_name() +
        "' for prefix '" + prefix + "'");
  }

  // For loop iterates over elements
  out << indent() << "for i := 0; i < size; i ++ {" << endl;
  indent_up();

  if (pointer_field) {
    prefix = "(*" + prefix + ")";
  }
  if (ttype->is_map()) {
    generate_deserialize_map_element(out, (t_map*)ttype, declare, prefix);
  } else if (ttype->is_set()) {
    generate_deserialize_set_element(out, (t_set*)ttype, declare, prefix);
  } else if (ttype->is_list()) {
    generate_deserialize_list_element(out, (t_list*)ttype, declare, prefix);
  }

  indent_down();
  out << indent() << "}" << endl;

  // Read container end
  if (ttype->is_map()) {
    out << indent() << "if err := iprot.ReadMapEnd(); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error reading map end: \", err)"
        << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "if err := iprot.ReadSetEnd(); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error reading set end: \", err)"
        << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "if err := iprot.ReadListEnd(); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error reading list end: \", err)"
        << endl;
    out << indent() << "}" << endl;
  }
}

/**
 * Generates code to deserialize a map
 */
void t_go_generator::generate_deserialize_map_element(
    ofstream& out, const t_map* tmap, bool declare, string prefix) {
  (void)declare;
  string key = tmp("_key");
  string val = tmp("_val");

  t_field fkey(tmap->get_key_type(), key);
  t_field fval(tmap->get_val_type(), val);
  string derefs = is_pointer_field(&fkey) ? "*" : "";
  fkey.set_req(t_field::e_req::opt_in_req_out);
  fval.set_req(t_field::e_req::opt_in_req_out);
  generate_deserialize_field(out, &fkey, true, "", false, false, true);
  generate_deserialize_field(out, &fval, true, "", false, false, false, true);
  indent(out) << prefix << "[" << derefs << key << "] = " << val << endl;
}

/**
 * Write a set element
 */
void t_go_generator::generate_deserialize_set_element(
    ofstream& out, const t_set* tset, bool declare, string prefix) {
  (void)declare;
  string elem = tmp("_elem");
  t_field felem(tset->get_elem_type(), elem);
  felem.set_req(t_field::e_req::opt_in_req_out);
  generate_deserialize_field(out, &felem, true, "", false, false, false, true);
  indent(out) << prefix << " = append(" << prefix << ", " << elem << ")"
              << endl;
}

/**
 * Write a list element
 */
void t_go_generator::generate_deserialize_list_element(
    ofstream& out, const t_list* tlist, bool declare, string prefix) {
  (void)declare;
  string elem = tmp("_elem");
  t_field felem(((t_list*)tlist)->get_elem_type(), elem);
  felem.set_req(t_field::e_req::opt_in_req_out);
  generate_deserialize_field(out, &felem, true, "", false, false, false, true);
  indent(out) << prefix << " = append(" << prefix << ", " << elem << ")"
              << endl;
}

/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param prefix Name to prepend to field name
 */
void t_go_generator::generate_serialize_field(
    ofstream& out, const t_field* tfield, string prefix, bool inkey) {
  const t_type* type = tfield->get_type()->get_true_type();
  string name(prefix + publicize(tfield->get_name()));

  // Do nothing for void types
  if (type->is_void()) {
    throw std::runtime_error(
        "compiler error: cannot generate serialize for void type: " + name);
  }

  if (type->is_struct() || type->is_exception()) {
    generate_serialize_struct(out, (t_struct*)type, name);
  } else if (type->is_container()) {
    generate_serialize_container(out, type, is_pointer_field(tfield), name);
  } else if (type->is_base_type() || type->is_enum()) {
    indent(out) << "if err := oprot.";

    if (is_pointer_field(tfield)) {
      name = "*" + name;
    }

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

      switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);

        case t_base_type::TYPE_STRING:
          out << "WriteString(string(" << name << "))";
          break;

        case t_base_type::TYPE_BINARY:
          if (!inkey) {
            out << "WriteBinary(" << name << ")";
          } else {
            out << "WriteString(string(" << name << "))";
          }
          break;

        case t_base_type::TYPE_BOOL:
          out << "WriteBool(bool(" << name << "))";
          break;

        case t_base_type::TYPE_BYTE:
          out << "WriteByte(byte(" << name << "))";
          break;

        case t_base_type::TYPE_I16:
          out << "WriteI16(int16(" << name << "))";
          break;

        case t_base_type::TYPE_I32:
          out << "WriteI32(int32(" << name << "))";
          break;

        case t_base_type::TYPE_I64:
          out << "WriteI64(int64(" << name << "))";
          break;

        case t_base_type::TYPE_DOUBLE:
          out << "WriteDouble(float64(" << name << "))";
          break;

        case t_base_type::TYPE_FLOAT:
          out << "WriteFloat(float32(" << name << "))";
          break;

        default:
          throw std::runtime_error(
              "compiler error: no Go name for base type " +
              t_base_type::t_base_name(tbase));
      }
    } else if (type->is_enum()) {
      out << "WriteI32(int32(" << name << "))";
    }

    out << "; err != nil {" << endl;
    out << indent() << "return thrift.PrependError(fmt.Sprintf(\"%T."
        << escape_string(tfield->get_name()) << " (" << tfield->get_key()
        << ") field write error: \", p), err) }" << endl;
  } else {
    throw std::runtime_error(
        "compiler error: Invalid type in generate_serialize_field '" +
        type->get_name() + "' for field '" + name + "'");
  }
}

/**
 * Serializes all the members of a struct.
 *
 * @param tstruct The struct to serialize
 * @param prefix  String prefix to attach to all fields
 */
void t_go_generator::generate_serialize_struct(
    ofstream& out, const t_struct* tstruct, string prefix) {
  (void)tstruct;
  out << indent() << "if err := " << prefix << ".Write(oprot); err != nil {"
      << endl;
  out << indent()
      << "  return thrift.PrependError(fmt.Sprintf(\"%T error writing struct: "
         "\", "
      << prefix << "), err)" << endl;
  out << indent() << "}" << endl;
}

void t_go_generator::generate_serialize_container(
    ofstream& out, const t_type* ttype, bool pointer_field, string prefix) {
  if (pointer_field) {
    prefix = "*" + prefix;
  }
  if (ttype->is_map()) {
    out << indent() << "if err := oprot.WriteMapBegin("
        << type_to_enum(((t_map*)ttype)->get_key_type()) << ", "
        << type_to_enum(((t_map*)ttype)->get_val_type()) << ", "
        << "len(" << prefix << ")); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error writing map begin: \", err)"
        << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "if err := oprot.WriteSetBegin("
        << type_to_enum(((t_set*)ttype)->get_elem_type()) << ", "
        << "len(" << prefix << ")); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error writing set begin: \", err)"
        << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "if err := oprot.WriteListBegin("
        << type_to_enum(((t_list*)ttype)->get_elem_type()) << ", "
        << "len(" << prefix << ")); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error writing list begin: \", err)"
        << endl;
    out << indent() << "}" << endl;
  } else {
    throw std::runtime_error(
        "compiler error: Invalid type in generate_serialize_container '" +
        ttype->get_name() + "' for prefix '" + prefix + "'");
  }

  if (ttype->is_map()) {
    const t_map* tmap = (t_map*)ttype;
    out << indent() << "for k, v := range " << prefix << " {" << endl;
    indent_up();
    generate_serialize_map_element(out, tmap, "k", "v");
    indent_down();
    indent(out) << "}" << endl;
  } else if (ttype->is_set()) {
    const t_set* tset = (t_set*)ttype;
    out << indent() << "set := make(map["
        << type_to_go_type(tset->get_elem_type()) << "]bool, len(" << prefix
        << "))" << endl;
    out << indent() << "for _, v := range " << prefix << " {" << endl;
    out << indent() << "  if ok := set[v]; ok {" << endl;
    out << indent()
        << "    return thrift.PrependError(\"\", fmt.Errorf(\"%T error writing "
           "set field: slice is not unique\", v))"
        << endl;
    out << indent() << "  }" << endl;
    out << indent() << "  set[v] = true" << endl;
    out << indent() << "}" << endl;
    out << indent() << "for _, v := range " << prefix << " {" << endl;
    indent_up();
    generate_serialize_set_element(out, tset, "v");
    indent_down();
    indent(out) << "}" << endl;
  } else if (ttype->is_list()) {
    const t_list* tlist = (t_list*)ttype;
    out << indent() << "for _, v := range " << prefix << " {" << endl;

    indent_up();
    generate_serialize_list_element(out, tlist, "v");
    indent_down();
    indent(out) << "}" << endl;
  }

  if (ttype->is_map()) {
    out << indent() << "if err := oprot.WriteMapEnd(); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error writing map end: \", err)"
        << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_set()) {
    out << indent() << "if err := oprot.WriteSetEnd(); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error writing set end: \", err)"
        << endl;
    out << indent() << "}" << endl;
  } else if (ttype->is_list()) {
    out << indent() << "if err := oprot.WriteListEnd(); err != nil {" << endl;
    out << indent()
        << "  return thrift.PrependError(\"error writing list end: \", err)"
        << endl;
    out << indent() << "}" << endl;
  }
}

/**
 * Serializes the members of a map.
 *
 */
void t_go_generator::generate_serialize_map_element(
    ofstream& out, const t_map* tmap, string kiter, string viter) {
  t_field kfield(tmap->get_key_type(), "");
  t_field vfield(tmap->get_val_type(), "");
  kfield.set_req(t_field::e_req::opt_in_req_out);
  vfield.set_req(t_field::e_req::opt_in_req_out);
  generate_serialize_field(out, &kfield, kiter, true);
  generate_serialize_field(out, &vfield, viter);
}

/**
 * Serializes the members of a set.
 */
void t_go_generator::generate_serialize_set_element(
    ofstream& out, const t_set* tset, string prefix) {
  t_field efield(tset->get_elem_type(), "");
  efield.set_req(t_field::e_req::opt_in_req_out);
  generate_serialize_field(out, &efield, prefix);
}

/**
 * Serializes the members of a list.
 */
void t_go_generator::generate_serialize_list_element(
    ofstream& out, const t_list* tlist, string prefix) {
  t_field efield(tlist->get_elem_type(), "");
  efield.set_req(t_field::e_req::opt_in_req_out);
  generate_serialize_field(out, &efield, prefix);
}

/**
 * Generates the docstring for a given struct.
 */
void t_go_generator::generate_go_docstring(
    ofstream& out, const t_structured* tstruct) {
  generate_go_docstring(out, tstruct, tstruct, "Attributes");
}

/**
 * Generates the docstring for a given function.
 */
void t_go_generator::generate_go_docstring(
    ofstream& out, const t_function* tfunction) {
  generate_go_docstring(out, tfunction, &tfunction->params(), "Parameters");
}

/**
 * Generates the docstring for a struct or function.
 */
void t_go_generator::generate_go_docstring(
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
      ss << " - " << publicize(p->get_name());

      if (p->has_doc()) {
        ss << ": " << p->doc();
      } else {
        ss << endl;
      }
    }
  }

  if (has_doc) {
    generate_docstring_comment(out, "", "// ", ss.str(), "");
  }
}

/**
 * Generates the docstring for a generic object.
 */
void t_go_generator::generate_go_docstring(
    ofstream& out, const t_named* named_node) {
  if (named_node->has_doc()) {
    generate_docstring_comment(out, "", "//", named_node->doc(), "");
  }
}

/**
 * Declares an argument, which may include initialization as necessary.
 *
 * @param tfield The field
 */
string t_go_generator::declare_argument(const t_field* tfield) {
  std::ostringstream result;
  result << publicize(tfield->get_name()) << "=";

  if (tfield->get_value() != nullptr) {
    result << "thrift_spec[" << tfield->get_key() << "][4]";
  } else {
    result << "nil";
  }

  return result.str();
}

/**
 * Renders a struct field initial value.
 *
 * @param tfield The field, which must have `tfield->get_value() != nullptr`
 */
string t_go_generator::render_field_initial_value(
    const t_field* tfield, const string& name, bool optional_field) {
  const t_type* type = tfield->get_type()->get_true_type();

  if (optional_field) {
    // The caller will make a second pass for optional fields,
    // assigning the result of render_const_value to "*field_name". It
    // is maddening that Go syntax does not allow for a type-agnostic
    // way to initialize a pointer to a const value, but so it goes.
    // The alternative would be to write type specific functions that
    // convert from const values to pointer types, but given the lack
    // of overloading it would be messy.
    return "new(" + type_to_go_type(tfield->get_type()) + ")";
  } else {
    return render_const_value(type, tfield->get_value(), name);
  }
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_go_generator::function_signature(
    const t_function* tfunction, string prefix) {
  // TODO(mcslee): Nitpicky, no ',' if argument_list is empty
  return publicize(prefix + tfunction->get_name()) + "(" +
      argument_list(tfunction->params()) + ")";
}

/**
 * Renders an interface function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_go_generator::function_signature_if(
    const t_function* tfunction, string prefix, bool useContext) {
  string signature = publicize(prefix + get_func_name(tfunction)) + "(";
  string args = argument_list(tfunction->params());
  if (useContext) {
    signature += "ctx context.Context";
    signature += args.length() > 0 ? ", " : "";
  }
  signature += args + ") (";
  const t_type* ret = tfunction->return_type().get_type();

  if (!ret->is_void()) {
    signature += "_r " + type_to_go_type(ret) + ", ";
  }

  signature += "err error)";

  return signature;
}

/**
 * Renders a field list
 */
string t_go_generator::argument_list(const t_paramlist& tparamlist) {
  string result = "";
  const vector<t_field*>& fields = tparamlist.get_members();
  vector<t_field*>::const_iterator f_iter;
  bool first = true;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }

    result += variable_name_to_go_name((*f_iter)->get_name()) + " " +
        type_to_go_type((*f_iter)->get_type());
  }

  return result;
}

string t_go_generator::type_name(const t_type* ttype) {
  const t_program* program = ttype->program();

  if (program != nullptr && program != program_) {
    string module(get_real_go_module(program));
    return package_identifiers[module] + "." + ttype->get_name();
  }

  return ttype->get_name();
}

/**
 * Converts the parse type to a go tyoe
 */
string t_go_generator::type_to_enum(const t_type* type) {
  type = type->get_true_type();

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("NO T_VOID CONSTRUCT");

      case t_base_type::TYPE_STRING:
        return "thrift.STRING";

      case t_base_type::TYPE_BINARY:
        // binary is still a string type internally
        return "thrift.STRING";

      case t_base_type::TYPE_BOOL:
        return "thrift.BOOL";

      case t_base_type::TYPE_BYTE:
        return "thrift.BYTE";

      case t_base_type::TYPE_I16:
        return "thrift.I16";

      case t_base_type::TYPE_I32:
        return "thrift.I32";

      case t_base_type::TYPE_I64:
        return "thrift.I64";

      case t_base_type::TYPE_DOUBLE:
        return "thrift.DOUBLE";

      case t_base_type::TYPE_FLOAT:
        return "thrift.FLOAT";
    }
  } else if (type->is_enum()) {
    return "thrift.I32";
  } else if (type->is_struct() || type->is_exception()) {
    return "thrift.STRUCT";
  } else if (type->is_map()) {
    return "thrift.MAP";
  } else if (type->is_set()) {
    return "thrift.SET";
  } else if (type->is_list()) {
    return "thrift.LIST";
  }

  throw std::runtime_error("INVALID TYPE IN type_to_enum: " + type->get_name());
}

string strip_type_pointer(string tname) {
  if (!tname.empty() && tname.front() == '*') {
    return tname.substr(1, tname.size() - 1);
  }
  return tname;
}
/**
 * Converts the parse type to a go map type, will throw an exception if it will
 * not produce a valid go map type.
 */
string t_go_generator::type_to_go_key_type(const t_type* type) {
  const t_type* resolved_type = type;

  while (resolved_type->is_typedef()) {
    resolved_type = ((t_typedef*)resolved_type)->get_type()->get_true_type();
  }

  if (resolved_type->is_map() || resolved_type->is_list() ||
      resolved_type->is_set()) {
    throw std::runtime_error(
        "Cannot produce a valid type for a Go map key: " +
        type_to_go_type(type) + " - aborting.");
  }

  if (resolved_type->is_binary()) {
    return "string";
  }

  return strip_type_pointer(type_to_go_type(type));
}

/**
 * Converts the parse type to a go type
 */
string t_go_generator::type_to_go_type(const t_type* type) {
  return type_to_go_type_with_opt(type, false);
}

/**
 * Converts the parse type to a go type, taking into account whether the field
 * associated with the type is T_OPTIONAL.
 */
string t_go_generator::type_to_go_type_with_opt(
    const t_type* type, bool optional_field, bool from_typedef) {
  string maybe_pointer(optional_field ? "*" : "");

  if (auto ph = dynamic_cast<const t_placeholder_typedef*>(type)) {
    type = ph->get_true_type();
  }

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();

    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("Unsupported type: void");

      case t_base_type::TYPE_STRING:
        return maybe_pointer + "string";

      case t_base_type::TYPE_BINARY:
        return maybe_pointer + "[]byte";

      case t_base_type::TYPE_BOOL:
        return maybe_pointer + "bool";

      case t_base_type::TYPE_BYTE:
        return maybe_pointer + "int8";

      case t_base_type::TYPE_I16:
        return maybe_pointer + "int16";

      case t_base_type::TYPE_I32:
        return maybe_pointer + "int32";

      case t_base_type::TYPE_I64:
        return maybe_pointer + "int64";

      case t_base_type::TYPE_DOUBLE:
        return maybe_pointer + "float64";

      case t_base_type::TYPE_FLOAT:
        return maybe_pointer + "float32";
    }
  } else if (type->is_enum()) {
    return maybe_pointer + publicize(type_name(type));
  } else if (type->is_struct() || type->is_exception()) {
    if (from_typedef) {
      return publicize(type_name(type));
    }
    return "*" + publicize(type_name(type));
  } else if (type->is_map()) {
    const t_map* t = (t_map*)type;
    string key_type = type_to_go_key_type(t->get_key_type());
    string value_type = type_to_go_type(t->get_val_type());
    return maybe_pointer + string("map[") + key_type + "]" + value_type;
  } else if (type->is_set()) {
    const t_set* t = (t_set*)type;
    string elemType = type_to_go_type(t->get_elem_type());
    return maybe_pointer + string("[]") + elemType;
  } else if (type->is_list()) {
    const t_list* t = (t_list*)type;
    string elemType = type_to_go_type(t->get_elem_type());
    return maybe_pointer + string("[]") + elemType;
  } else if (type->is_typedef()) {
    auto true_type = type->get_true_type();
    if (true_type->is_struct() || true_type->is_exception()) {
      return "*" + publicize(type_name(type));
    }
    return maybe_pointer + publicize(type_name(type));
  }

  throw std::runtime_error(
      "INVALID TYPE IN type_to_go_type: " + type->get_name());
}

/** See the comment inside generate_go_struct_definition for what this is. */
string t_go_generator::type_to_spec_args(const t_type* ttype) {
  while (ttype->is_typedef()) {
    ttype = ((t_typedef*)ttype)->get_type();
  }

  if (ttype->is_base_type() || ttype->is_enum()) {
    return "nil";
  } else if (ttype->is_struct() || ttype->is_exception()) {
    return "(" + type_name(ttype) + ", " + type_name(ttype) + ".thrift_spec)";
  } else if (ttype->is_map()) {
    return "(" + type_to_enum(((t_map*)ttype)->get_key_type()) + "," +
        type_to_spec_args(((t_map*)ttype)->get_key_type()) + "," +
        type_to_enum(((t_map*)ttype)->get_val_type()) + "," +
        type_to_spec_args(((t_map*)ttype)->get_val_type()) + ")";
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

THRIFT_REGISTER_GENERATOR(
    go,
    "Go",
    "    package_prefix=  Package prefix for generated files.\n"
    "    thrift_import=   Override thrift package import path (default:" +
        default_thrift_import +
        ")\n"
        "    package=         Package name (default: inferred from thrift file "
        "name)\n"
        "    use_context      Generate code with context on all thrift server "
        "handlers\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
