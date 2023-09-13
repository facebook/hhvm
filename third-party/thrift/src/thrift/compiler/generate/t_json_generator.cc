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

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/generate/json.h>
#include <thrift/compiler/generate/t_concat_generator.h>

using namespace std;

namespace apache {
namespace thrift {
namespace compiler {

/**
 * JSON code generator.
 */
class t_json_generator : public t_concat_generator {
 public:
  using t_concat_generator::t_concat_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    out_dir_base_ = "gen-json";
    annotate_ = options.find("annotate") != options.end();
  }

  void generate_program() override;

  /**
   * Program-level generation functions
   */

  void generate_include(const t_program* included_program);
  void generate_typedef(const t_typedef* ttypedef) override;
  void generate_enum(const t_enum* tenum) override;
  void generate_const(const t_const* tconst) override;
  void generate_consts(vector<t_const*> consts) override;
  void generate_struct(const t_struct* tstruct) override;
  void generate_service(const t_service* tservice) override;
  void generate_xception(const t_struct* txception) override;

  void print_type(const t_type* ttype);
  void print_name(const string& name);
  void print_const_value(const t_const_value* tvalue);
  void print_const_key(t_const_value* tvalue);
  void print_lineno(const t_node& node);
  string type_to_string(const t_type* type);
  string type_to_spec_args(const t_type* ttype);
  string type_name(const t_type* ttype);

  bool should_resolve_to_true_type(const t_type* ttype);

  std::ofstream f_out_;

 private:
  void print_annotations(
      const std::map<std::string, annotation_value>& annotations);
  void print_structured_annotations(node_list_view<const t_const> annotations);
  void print_node_annotations(
      const t_named& node, bool add_heading_comma, bool add_trailing_comma);
  void print_source_range(const source_range& range);

  // True if we should generate annotations in json representation.
  bool annotate_;
};

/**
 * Prepares for file generation by opening up the necessary file output
 * stream.
 */
void t_json_generator::generate_program() {
  // Make output directory
  boost::filesystem::create_directory(get_out_dir());
  string module_name = program_->get_namespace("json");
  string fname = get_out_dir();
  if (module_name.empty()) {
    module_name = program_->name();
  }
  string mangled_module_name = module_name;
  boost::filesystem::create_directory(fname);
  for (string::size_type pos = mangled_module_name.find('.');
       pos != string::npos;
       pos = mangled_module_name.find('.')) {
    fname += '/';
    fname += mangled_module_name.substr(0, pos);
    mangled_module_name.erase(0, pos + 1);
    boost::filesystem::create_directory(fname);
  }

  fname += '/';
  fname += mangled_module_name;
  fname += ".json";
  f_out_.open(fname.c_str());
  indent(f_out_) << "{" << endl;
  indent_up();
  indent(f_out_) << "\"__fbthrift\": {\"@"
                 << "generated\": 0}," << endl;
  indent(f_out_) << "\"thrift_module\" : \"" << module_name << "\"";

  if (!program_->consts().empty()) {
    f_out_ << "," << endl << indent() << "\"constants\" : {" << endl;
    indent_up();
    auto consts = program_->consts();
    generate_consts(consts);
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}";
  }

  if (!program_->includes().empty()) {
    f_out_ << "," << endl << indent() << "\"includes\": {" << endl;
    indent_up();
    // Generate includes
    auto includes = program_->get_included_programs();
    for (auto in_iter = includes.begin(); in_iter != includes.end();
         ++in_iter) {
      if (in_iter != includes.begin()) {
        f_out_ << "," << endl;
      }
      generate_include(*in_iter);
    }
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}";
  }

  if (!program_->enums().empty()) {
    f_out_ << "," << endl << indent() << "\"enumerations\" : {" << endl;
    indent_up();
    // Generate enums
    auto enums = program_->enums();
    for (auto en_iter = enums.begin(); en_iter != enums.end(); ++en_iter) {
      if (en_iter != enums.begin()) {
        f_out_ << "," << endl;
      }
      generate_enum(*en_iter);
    }
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}";
  }

  if (!program_->typedefs().empty()) {
    f_out_ << "," << endl << indent() << "\"typedefs\" : {" << endl;
    indent_up();
    // Generate typedefs
    auto typedefs = program_->typedefs();
    for (auto td_iter = typedefs.begin(); td_iter != typedefs.end();
         ++td_iter) {
      if (td_iter != typedefs.begin()) {
        f_out_ << "," << endl;
      }
      generate_typedef(*td_iter);
    }
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}";
  }

  if (!program_->objects().empty()) {
    f_out_ << "," << endl << indent() << "\"structs\" : {" << endl;
    indent_up();
    // Generate structs and exceptions in declared order
    auto objects = program_->objects();
    for (auto o_iter = objects.begin(); o_iter != objects.end(); ++o_iter) {
      if (o_iter != objects.begin()) {
        f_out_ << "," << endl;
      }
      if ((*o_iter)->is_exception()) {
        generate_xception(*o_iter);
      } else {
        generate_struct(*o_iter);
      }
    }
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}";
  }

  if (!program_->services().empty()) {
    f_out_ << "," << endl << indent() << "\"services\" : {" << endl;
    indent_up();
    // Generate services
    auto services = program_->services();
    for (auto sv_iter = services.begin(); sv_iter != services.end();
         ++sv_iter) {
      service_name_ = get_service_name(*sv_iter);
      if (sv_iter != services.begin()) {
        f_out_ << "," << endl;
      }
      generate_service(*sv_iter);
    }
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}";
  }

  f_out_ << endl;
  indent_down();
  indent(f_out_) << "}" << endl;
  f_out_.close();
}

/**
 * Converts the parse type to a string
 */
string t_json_generator::type_to_string(const t_type* type) {
  if (should_resolve_to_true_type(type)) {
    type = type->get_true_type();
  }

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        return "VOID";
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        return "STRING";
      case t_base_type::TYPE_BOOL:
        return "BOOL";
      case t_base_type::TYPE_BYTE:
        return "BYTE";
      case t_base_type::TYPE_I16:
        return "I16";
      case t_base_type::TYPE_I32:
        return "I32";
      case t_base_type::TYPE_I64:
        return "I64";
      case t_base_type::TYPE_DOUBLE:
        return "DOUBLE";
      case t_base_type::TYPE_FLOAT:
        return "FLOAT";
    }
  } else if (type->is_enum()) {
    return "ENUM";
  } else if (type->is_struct() || type->is_exception()) {
    return "STRUCT";
  } else if (type->is_map()) {
    return "MAP";
  } else if (type->is_set()) {
    return "SET";
  } else if (type->is_list()) {
    return "LIST";
  } else if (type->is_service()) {
    return "SERVICE";
  } else if (type->is_typedef()) {
    return "TYPEDEF";
  }

  throw std::runtime_error(
      "INVALID TYPE IN type_to_string: " + type->get_name());
}

/**
 * Returns a string containing a type spec for the provided type.
 * The specification has the following structure:
 *   type_enum -> STRING | BOOL | BYTE | ...
 *   tuple_spec -> { "type_enum" : type_enum, "spec_args" : spec_args }
 *   spec_args -> null  // (for base types)
 *              | tuple_spec  // (for lists and sets)
 *              | { "key_type" : tuple_spec, "val_type" : tuple_spec}  // (maps)
 */
string t_json_generator::type_to_spec_args(const t_type* ttype) {
  if (should_resolve_to_true_type(ttype)) {
    ttype = ttype->get_true_type();
  }

  if (ttype->is_base_type()) {
    return "null";
  } else if (
      ttype->is_struct() || ttype->is_exception() || ttype->is_service() ||
      ttype->is_enum() || ttype->is_typedef()) {
    string module = "";
    if (ttype->program() != program_) {
      module = ttype->program()->name() + ".";
    }
    return "\"" + module + ttype->get_name() + "\"";
  } else if (ttype->is_map()) {
    return "{ \"key_type\" : { \"type_enum\" : \"" +
        type_to_string(((t_map*)ttype)->get_key_type()) +
        "\", \"spec_args\" : " +
        type_to_spec_args(((t_map*)ttype)->get_key_type()) +
        " }, \"val_type\" : { \"type_enum\" : \"" +
        type_to_string(((t_map*)ttype)->get_val_type()) +
        "\", \"spec_args\" : " +
        type_to_spec_args(((t_map*)ttype)->get_val_type()) + "} } ";
  } else if (ttype->is_set()) {
    return "{ \"type_enum\" : \"" +
        type_to_string(((t_set*)ttype)->get_elem_type()) +
        "\", \"spec_args\" : " +
        type_to_spec_args(((t_set*)ttype)->get_elem_type()) + "} ";
  } else if (ttype->is_list()) {
    return "{ \"type_enum\" : \"" +
        type_to_string(((t_list*)ttype)->get_elem_type()) +
        "\", \"spec_args\" : " +
        type_to_spec_args(((t_list*)ttype)->get_elem_type()) + "} ";
  }

  throw std::runtime_error(
      "INVALID TYPE IN type_to_spec_args: " + ttype->get_name());
}

/**
 * Return the type name, based on the namespace and the module (when
 * applicable).
 */
string t_json_generator::type_name(const t_type* ttype) {
  const t_program* program = ttype->program();
  if (program != nullptr && program != program_) {
    const string& json_namespace = program->get_namespace("json");
    return (!json_namespace.empty() ? json_namespace : program->name()) + "." +
        ttype->get_name();
  }
  return ttype->get_name();
}

/**
 * Prints out the provided type spec
 */
void t_json_generator::print_type(const t_type* ttype) {
  indent(f_out_) << "\"type_enum\" : \"" << type_to_string(ttype) << "\","
                 << endl;
  indent(f_out_) << "\"spec_args\" : " << type_to_spec_args(ttype);
}

void t_json_generator::print_name(const string& name) {
  f_out_ << indent() << "\"name\" : "
         << "\"" << name << "\"," << endl;
}

/**
 * Prints out a JSON representation of the provided constant map key.
 * The JSON spec allows for strings, and nothing else.
 * TODO - support serialization of complex keys...
 */
void t_json_generator::print_const_key(t_const_value* tvalue) {
  switch (tvalue->get_type()) {
    case t_const_value::CV_INTEGER:
      f_out_ << "\"" << tvalue->get_integer() << "\"";
      break;
    case t_const_value::CV_DOUBLE:
      f_out_ << "\"" << tvalue->get_double() << "\"";
      break;
    case t_const_value::CV_STRING:
      json_quote_ascii(f_out_, tvalue->get_string());
      break;
    case t_const_value::CV_MAP:
    case t_const_value::CV_LIST:
    default: {
      std::ostringstream msg;
      msg << "INVALID TYPE IN print_const_key: " << tvalue->get_type();
      throw msg.str();
    }
  }
}

/**
 * Prints out a JSON representation of the provided constant value
 */
void t_json_generator::print_const_value(const t_const_value* tvalue) {
  bool first = true;
  switch (tvalue->get_type()) {
    case t_const_value::CV_INTEGER:
      f_out_ << tvalue->get_integer();
      break;
    case t_const_value::CV_DOUBLE:
      f_out_ << tvalue->get_double();
      break;
    case t_const_value::CV_STRING:
      json_quote_ascii(f_out_, tvalue->get_string());
      break;
    case t_const_value::CV_BOOL:
      f_out_ << (tvalue->get_bool() ? "true" : "false");
      break;
    case t_const_value::CV_MAP: {
      f_out_ << "{ ";
      const vector<pair<t_const_value*, t_const_value*>>& map_elems =
          tvalue->get_map();
      vector<pair<t_const_value*, t_const_value*>>::const_iterator map_iter;
      for (map_iter = map_elems.begin(); map_iter != map_elems.end();
           map_iter++) {
        if (!first) {
          f_out_ << ", ";
        }
        first = false;
        print_const_key(map_iter->first);
        f_out_ << " : ";
        print_const_value(map_iter->second);
      }
      f_out_ << " }";
    } break;
    case t_const_value::CV_LIST: {
      f_out_ << "[ ";
      vector<t_const_value*> list_elems = tvalue->get_list();
      ;
      vector<t_const_value*>::iterator list_iter;
      for (list_iter = list_elems.begin(); list_iter != list_elems.end();
           list_iter++) {
        if (!first) {
          f_out_ << ", ";
        }
        first = false;
        print_const_value(*list_iter);
      }
      f_out_ << " ]";
    } break;
    default:
      f_out_ << "UNKNOWN";
      break;
  }
}

void t_json_generator::print_lineno(const t_node& node) {
  auto loc = node.src_range().begin;
  unsigned line =
      loc != source_location() ? resolved_location(loc, source_mgr_).line() : 0;
  indent(f_out_) << "\"lineno\" : " << line << ",\n";
}

void t_json_generator::print_annotations(
    const std::map<std::string, annotation_value>& annotations) {
  indent(f_out_) << "\"annotations\" : {";
  indent_up();
  bool first = true;
  std::map<std::string, annotation_value>::const_iterator iter;
  for (iter = annotations.begin(); iter != annotations.end(); ++iter) {
    if (!first) {
      f_out_ << ",";
    }
    f_out_ << endl;
    first = false;
    indent(f_out_) << "\"" << iter->first << "\" : {" << endl;
    indent_up();

    indent(f_out_) << "\"value\" : ";
    json_quote_ascii(f_out_, iter->second.value);
    f_out_ << "," << endl;

    print_source_range(iter->second.src_range);

    indent_down();
    indent(f_out_) << "}" << endl;
  }
  indent_down();
  indent(f_out_) << "}";
}

void t_json_generator::print_structured_annotations(
    node_list_view<const t_const> annotations) {
  indent(f_out_) << "\"structured_annotations\" : {";
  indent_up();
  bool first = true;

  for (const auto& annotation : annotations) {
    if (!std::exchange(first, false)) {
      f_out_ << ",";
    }
    f_out_ << endl;
    indent(f_out_) << "\"" << type_name(annotation.type()) << "\" : ";
    print_const_value(annotation.value());
  }
  f_out_ << endl;
  indent_down();
  indent(f_out_) << "}";
}

void t_json_generator::print_node_annotations(
    const t_named& node, bool add_heading_comma, bool add_trailing_comma) {
  if (annotate_) {
    if (add_heading_comma &&
        (!node.annotations().empty() ||
         !node.structured_annotations().empty())) {
      f_out_ << "," << endl;
    }
    if (!node.annotations().empty()) {
      print_annotations(node.annotations());
    }
    if (!node.structured_annotations().empty()) {
      if (!node.annotations().empty()) {
        f_out_ << "," << endl;
      }
      print_structured_annotations(node.structured_annotations());
    }
    if (add_trailing_comma &&
        (!node.annotations().empty() ||
         !node.structured_annotations().empty())) {
      f_out_ << "," << endl;
    }
  }
}

/**
 * Generates an include.
 *
 * @param tinclude The include statement
 */
void t_json_generator::generate_include(const t_program* included_program) {
  indent(f_out_) << "\"" << included_program->get_name() << "\" : {" << endl;
  indent_up();
  indent(f_out_) << "\"path\" : \""
                 << included_program->include_prefix() +
          included_program->name() + ".thrift"
                 << "\"" << endl;
  indent_down();
  indent(f_out_) << "}";
}

/**
 * Generates a typedef.
 *
 * @param ttypedef The type definition
 */
void t_json_generator::generate_typedef(const t_typedef* ttypedef) {
  indent(f_out_) << "\"" << ttypedef->get_name() << "\" : {" << endl;
  indent_up();
  print_lineno(*ttypedef);
  print_type(ttypedef->get_type());
  print_node_annotations(
      *ttypedef,
      /*add_heading_comma=*/true,
      /*add_trailing_comma=*/false);
  f_out_ << "," << endl;
  print_source_range(ttypedef->src_range());
  indent_down();
  indent(f_out_) << "}";
}

/**
 * Generates code for an enumerated type.
 *
 * @param tenum The enumeration
 */
void t_json_generator::generate_enum(const t_enum* tenum) {
  indent(f_out_) << "\"" << tenum->get_name() << "\" : {" << endl;
  indent_up();
  print_lineno(*tenum);
  print_node_annotations(
      *tenum,
      /*add_heading_comma=*/false,
      /*add_trailing_comma=*/true);
  indent(f_out_) << "\"constants\" : {" << endl;
  indent_up();
  vector<t_enum_value*> values = tenum->get_enum_values();
  vector<t_enum_value*>::iterator val_iter;
  for (val_iter = values.begin(); val_iter != values.end(); ++val_iter) {
    if (val_iter != values.begin()) {
      f_out_ << "," << endl;
    }
    // TODO (partisan): Find a good way to expose enumerator annotations.
    // Modifying a value from a scalar to the JSON would fit the general
    // approach of compartmentalization, but may be backwards-incompatible.
    // Adding annotations as a separate top-level enum list/map would go
    // against this general approach.
    indent(f_out_) << "\"" << (*val_iter)->get_name() << "\""
                   << " : " << (*val_iter)->get_value();
  }
  f_out_ << endl;
  indent_down();
  indent(f_out_) << "}," << endl;
  print_source_range(tenum->src_range());
  indent_down();
  indent(f_out_) << "}";
}

/**
 * Generate constants
 */
void t_json_generator::generate_consts(vector<t_const*> consts) {
  for (auto c_iter = consts.begin(); c_iter != consts.end(); ++c_iter) {
    if (c_iter != consts.begin()) {
      f_out_ << "," << endl;
    }
    generate_const(*c_iter);
  }
}

/**
 * Generates a constant value
 */
void t_json_generator::generate_const(const t_const* tconst) {
  string name = tconst->get_name();
  indent(f_out_) << "\"" << name << "\" : {" << endl;
  indent_up();
  print_lineno(*tconst);
  indent(f_out_) << "\"value\" : ";
  print_const_value(tconst->value());
  f_out_ << "," << endl;
  print_type(tconst->type());
  print_node_annotations(
      *tconst,
      /*add_heading_comma=*/true,
      /*add_trailing_comma=*/false);
  f_out_ << "," << endl;
  print_source_range(tconst->src_range());
  indent_down();
  indent(f_out_) << "}";
}

/**
 * Generates a struct definition for a thrift data type.
 *
 * @param tstruct The struct definition
 */
void t_json_generator::generate_struct(const t_struct* tstruct) {
  const string& name = tstruct->get_name();
  indent(f_out_) << "\"" << name << "\" : {" << endl;
  indent_up();
  print_lineno(*tstruct);
  indent(f_out_) << "\"is_exception\" : "
                 << (tstruct->is_exception() ? "true" : "false") << "," << endl;
  indent(f_out_) << "\"is_union\" : "
                 << (tstruct->is_union() ? "true" : "false") << "," << endl;
  print_node_annotations(
      *tstruct,
      /*add_heading_comma=*/false,
      /*add_trailing_comma=*/true);
  vector<t_field*> members = tstruct->get_members();
  vector<t_field*>::iterator mem_iter = members.begin();
  indent(f_out_) << "\"fields\" : {" << endl;
  indent_up();
  for (; mem_iter != members.end(); mem_iter++) {
    if (mem_iter != members.begin()) {
      f_out_ << "," << endl;
    }
    indent(f_out_) << "\"" << (*mem_iter)->get_name() << "\" : {" << endl;
    indent_up();
    print_type((*mem_iter)->get_type());
    f_out_ << "," << endl
           << indent() << "\"required\" : "
           << ((*mem_iter)->get_req() != t_field::e_req::optional ? "true"
                                                                  : "false");
    const t_const_value* default_val = (*mem_iter)->get_value();
    if (default_val != nullptr) {
      f_out_ << "," << endl << indent() << "\"default_value\" : ";
      print_const_value(default_val);
    }
    print_node_annotations(
        **mem_iter,
        /*add_heading_comma=*/true,
        /*add_trailing_comma=*/false);
    f_out_ << "," << endl;
    print_source_range((*mem_iter)->src_range());
    indent_down();
    indent(f_out_) << "}";
  }
  f_out_ << endl;
  indent_down();
  indent(f_out_) << "}," << endl;
  print_source_range(tstruct->src_range());
  indent_down();
  indent(f_out_) << "}";
}

/**
 * Exceptions are special structs
 *
 * @param exception The struct definition
 */
void t_json_generator::generate_xception(const t_struct* exception) {
  generate_struct(exception);
}

/**
 * Generates the JSON object for a Thrift service.
 *
 * @param tservice The service definition
 */
void t_json_generator::generate_service(const t_service* tservice) {
  indent(f_out_) << "\"" << service_name_ << "\" : {" << endl;
  indent_up();

  bool first = true;

  if (tservice->get_extends()) {
    indent(f_out_) << "\"extends\" : {" << endl;
    indent_up();
    print_type(tservice->get_extends());
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}";
    first = false;
  }
  if (!first) {
    f_out_ << "," << endl;
  }
  print_lineno(*tservice);
  print_node_annotations(
      *tservice,
      /*add_heading_comma=*/false,
      /*add_trailing_comma=*/true);
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator fn_iter = functions.begin();
  f_out_ << indent() << "\"functions\" : {" << endl;
  indent_up();
  for (; fn_iter != functions.end(); fn_iter++) {
    if (fn_iter != functions.begin()) {
      f_out_ << "," << endl;
    }
    string fn_name = (*fn_iter)->get_name();
    indent(f_out_) << "\"" << service_name_ << "." << fn_name << "\" : {"
                   << endl;
    indent_up();
    indent(f_out_) << "\"return_type\" : {" << endl;
    indent_up();
    print_type((*fn_iter)->return_type());
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}," << endl;

    indent(f_out_) << "\"args\" : [";
    vector<t_field*> args = (*fn_iter)->get_paramlist()->get_members();
    vector<t_field*>::iterator arg_iter = args.begin();
    if (arg_iter != args.end()) {
      f_out_ << endl;
      indent_up();
      for (; arg_iter != args.end(); arg_iter++) {
        if (arg_iter != args.begin()) {
          f_out_ << "," << endl;
        }
        indent(f_out_) << "{" << endl;
        indent_up();
        print_name((*arg_iter)->get_name());
        print_type((*arg_iter)->get_type());
        if ((*arg_iter)->get_value() != nullptr) {
          f_out_ << "," << endl << indent() << "\"value\" : ";
          print_const_value((*arg_iter)->get_value());
        }
        print_node_annotations(
            **arg_iter,
            /*add_heading_comma=*/true,
            /*add_trailing_comma=*/false);
        f_out_ << endl;
        indent_down();
        indent(f_out_) << "}";
      }
      f_out_ << endl;
      indent_down();
      indent(f_out_);
    }
    f_out_ << "]," << endl;

    indent(f_out_) << "\"throws\" : [";
    auto exceptions = get_elems((*fn_iter)->exceptions());
    if (!exceptions.empty()) {
      f_out_ << endl;
      indent_up();
      auto ex_iter = exceptions.begin();
      for (; ex_iter != exceptions.end(); ++ex_iter) {
        if (ex_iter != exceptions.begin()) {
          f_out_ << "," << endl;
        }
        indent(f_out_) << type_to_spec_args((*ex_iter).get_type());
      }
      f_out_ << endl;
      indent_down();
      indent(f_out_);
    }
    f_out_ << "]";
    print_node_annotations(
        **fn_iter,
        /*add_heading_comma=*/true,
        /*add_trailing_comma=*/false);
    f_out_ << "," << endl;
    print_source_range((*fn_iter)->src_range());
    indent_down();
    indent(f_out_) << "}";
  }
  f_out_ << endl;
  indent_down();
  indent(f_out_) << "}," << endl;

  print_source_range(tservice->src_range());

  f_out_ << endl;
  indent_down();
  indent(f_out_) << "}";
}

bool t_json_generator::should_resolve_to_true_type(const t_type* ttype) {
  // Only resolve undefined typedefs as they were used for undeclared types
  return dynamic_cast<const t_placeholder_typedef*>(ttype) != nullptr;
}

/**
 * Prints source range information of a given source_range class.
 *
 * @param range The source range.
 */
void t_json_generator::print_source_range(const source_range& range) {
  indent(f_out_) << "\"source_range\" : {" << endl;
  indent_up();

  struct line_column {
    unsigned line = 0;
    unsigned column = 0;

    line_column(source_location loc, const source_manager& sm) {
      if (loc != source_location()) {
        auto resolved_loc = resolved_location(loc, sm);
        line = resolved_loc.line();
        column = resolved_loc.column();
      }
    }
  };

  auto begin_loc = line_column(range.begin, source_mgr_);
  indent(f_out_) << "\"begin\" : {" << endl;
  indent_up();
  indent(f_out_) << "\"line\" : " << begin_loc.line << "," << endl;
  indent(f_out_) << "\"column\" : " << begin_loc.column << endl;
  indent_down();
  indent(f_out_) << "}," << endl;

  auto end_loc = line_column(range.end, source_mgr_);
  indent(f_out_) << "\"end\" : {" << endl;
  indent_up();
  indent(f_out_) << "\"line\" : " << end_loc.line << "," << endl;
  indent(f_out_) << "\"column\" : " << end_loc.column << endl;
  indent_down();
  indent(f_out_) << "}" << endl;

  indent_down();
  indent(f_out_) << "}" << endl;
}

THRIFT_REGISTER_GENERATOR(
    json,
    "JSON",
    "    annotate:        Generate annotations in json representation\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
