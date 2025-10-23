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
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/generate/json.h>
#include <thrift/compiler/generate/t_concat_generator.h>

using namespace std;

namespace apache::thrift::compiler {

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
  void generate_struct(const t_structured* tstruct) override;
  void generate_service(const t_service* tservice) override;
  void generate_xception(const t_structured* txception) override;

  template <typename T>
  void print_spec(const T* type_or_service);
  void print_name(const string& name);
  void print_const_value(const t_const_value* tvalue);
  void print_const_key(t_const_value* tvalue);
  void print_lineno(const t_node& node);

  string to_string(const t_type* type);
  string to_string(const t_service* service);

  string to_spec_args(const t_type* type);
  string to_spec_args(const t_service* service);
  string to_spec_args_named(const t_named* named);

  string type_name(const t_type* ttype);

  bool should_resolve_to_true_type(const t_type* ttype);

  std::ofstream f_out_;

 private:
  void print_annotations(const deprecated_annotation_map& annotations);
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
  std::filesystem::create_directory(get_out_dir());
  string module_name = program_->get_namespace("json");
  string fname = get_out_dir();
  if (module_name.empty()) {
    module_name = program_->name();
  }
  string mangled_module_name = module_name;
  std::filesystem::create_directory(fname);
  for (string::size_type pos = mangled_module_name.find('.');
       pos != string::npos;
       pos = mangled_module_name.find('.')) {
    fname += '/';
    fname += mangled_module_name.substr(0, pos);
    mangled_module_name.erase(0, pos + 1);
    std::filesystem::create_directory(fname);
  }

  fname += '/';
  fname += mangled_module_name;
  fname += ".json";
  f_out_.open(fname.c_str());
  indent(f_out_) << "{" << endl;
  indent_up();
  indent(f_out_) << R"("__fbthrift": {"@)" << "generated\": 0}," << endl;
  indent(f_out_) << R"("thrift_module" : ")" << module_name << "\"";

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

  if (!program_->structured_definitions().empty()) {
    f_out_ << "," << endl << indent() << "\"structs\" : {" << endl;
    indent_up();
    // Generate structs and exceptions in declared order
    auto objects = program_->structured_definitions();
    for (auto o_iter = objects.begin(); o_iter != objects.end(); ++o_iter) {
      if (o_iter != objects.begin()) {
        f_out_ << "," << endl;
      }
      if ((*o_iter)->is<t_exception>()) {
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
 * Converts the type to a string.
 */
string t_json_generator::to_string(const t_type* type) {
  if (should_resolve_to_true_type(type)) {
    type = type->get_true_type();
  }

  if (const auto* primitive = type->try_as<t_primitive_type>()) {
    t_primitive_type::type tbase = primitive->primitive_type();
    switch (tbase) {
      case t_primitive_type::type::t_void:
        return "VOID";
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        return "STRING";
      case t_primitive_type::type::t_bool:
        return "BOOL";
      case t_primitive_type::type::t_byte:
        return "BYTE";
      case t_primitive_type::type::t_i16:
        return "I16";
      case t_primitive_type::type::t_i32:
        return "I32";
      case t_primitive_type::type::t_i64:
        return "I64";
      case t_primitive_type::type::t_double:
        return "DOUBLE";
      case t_primitive_type::type::t_float:
        return "FLOAT";
    }
  } else if (type->is<t_enum>()) {
    return "ENUM";
  } else if (type->is<t_structured>()) {
    return "STRUCT";
  } else if (type->is<t_map>()) {
    return "MAP";
  } else if (type->is<t_set>()) {
    return "SET";
  } else if (type->is<t_list>()) {
    return "LIST";
  } else if (type->is<t_typedef>()) {
    return "TYPEDEF";
  }

  throw std::runtime_error("INVALID TYPE IN to_string: " + type->name());
}

string t_json_generator::to_string(const t_service*) {
  return "SERVICE";
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
string t_json_generator::to_spec_args(const t_type* type) {
  if (should_resolve_to_true_type(type)) {
    type = type->get_true_type();
  }

  if (type->is<t_primitive_type>()) {
    return "null";
  } else if (
      type->is<t_structured>() || type->is<t_enum>() || type->is<t_typedef>()) {
    return to_spec_args_named(type);
  } else if (const t_map* map = type->try_as<t_map>()) {
    return R"({ "key_type" : { "type_enum" : ")" +
        to_string(&map->key_type().deref()) + R"(", "spec_args" : )" +
        to_spec_args(&map->key_type().deref()) +
        R"( }, "val_type" : { "type_enum" : ")" +
        to_string(&map->val_type().deref()) + R"(", "spec_args" : )" +
        to_spec_args(&map->val_type().deref()) + "} } ";
  } else if (const t_set* set = type->try_as<t_set>()) {
    return R"({ "type_enum" : ")" + to_string(set->elem_type().get_type()) +
        R"(", "spec_args" : )" + to_spec_args(set->elem_type().get_type()) +
        "} ";
  } else if (const t_list* list = type->try_as<t_list>()) {
    return R"({ "type_enum" : ")" + to_string(list->elem_type().get_type()) +
        R"(", "spec_args" : )" + to_spec_args(list->elem_type().get_type()) +
        "} ";
  }

  throw std::runtime_error("INVALID TYPE IN to_spec_args: " + type->name());
}

string t_json_generator::to_spec_args(const t_service* service) {
  return to_spec_args_named(service);
}

string t_json_generator::to_spec_args_named(const t_named* named) {
  std::string module;
  if (named->program() != program_) {
    module = named->program()->name() + ".";
  }
  return "\"" + module + named->name() + "\"";
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
        ttype->name();
  }
  return ttype->name();
}

/**
 * Prints out the provided type or service spec.
 */
template <typename T>
void t_json_generator::print_spec(const T* type_or_service) {
  indent(f_out_) << R"("type_enum" : ")" << to_string(type_or_service)
                 << "\",\n";
  indent(f_out_) << "\"spec_args\" : " << to_spec_args(type_or_service);
}

void t_json_generator::print_name(const string& name) {
  f_out_ << indent() << "\"name\" : " << "\"" << name << "\"," << endl;
}

/**
 * Prints out a JSON representation of the provided constant map key.
 * The JSON spec allows for strings, and nothing else.
 * TODO - support serialization of complex keys...
 */
void t_json_generator::print_const_key(t_const_value* tvalue) {
  switch (tvalue->kind()) {
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
      msg << "INVALID TYPE IN print_const_key: " << tvalue->kind();
      throw msg.str();
    }
  }
}

/**
 * Prints out a JSON representation of the provided constant value
 */
void t_json_generator::print_const_value(const t_const_value* tvalue) {
  bool first = true;
  switch (tvalue->kind()) {
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
      loc != source_location() ? source_mgr_.resolve_location(loc).line() : 0;
  indent(f_out_) << "\"lineno\" : " << line << ",\n";
}

void t_json_generator::print_annotations(
    const deprecated_annotation_map& annotations) {
  indent(f_out_) << "\"annotations\" : {";
  indent_up();
  bool first = true;
  deprecated_annotation_map::const_iterator iter;
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
        (!node.unstructured_annotations().empty() ||
         !node.structured_annotations().empty())) {
      f_out_ << "," << endl;
    }
    if (!node.unstructured_annotations().empty()) {
      print_annotations(node.unstructured_annotations());
    }
    if (!node.structured_annotations().empty()) {
      if (!node.unstructured_annotations().empty()) {
        f_out_ << "," << endl;
      }
      print_structured_annotations(node.structured_annotations());
    }
    if (add_trailing_comma &&
        (!node.unstructured_annotations().empty() ||
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
  indent(f_out_) << "\"" << included_program->name() << "\" : {" << endl;
  indent_up();
  indent(f_out_) << R"("path" : ")"
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
  indent(f_out_) << "\"" << ttypedef->name() << "\" : {" << endl;
  indent_up();
  print_lineno(*ttypedef);
  print_spec(ttypedef->get_type());
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
  indent(f_out_) << "\"" << tenum->name() << "\" : {" << endl;
  indent_up();
  print_lineno(*tenum);
  print_node_annotations(
      *tenum,
      /*add_heading_comma=*/false,
      /*add_trailing_comma=*/true);
  indent(f_out_) << "\"constants\" : {" << endl;
  indent_up();
  node_list_view<const t_enum_value> values = tenum->values();
  node_list_view<const t_enum_value>::iterator val_iter;
  for (val_iter = values.begin(); val_iter != values.end(); ++val_iter) {
    if (val_iter != values.begin()) {
      f_out_ << "," << endl;
    }
    // TODO (partisan): Find a good way to expose enumerator annotations.
    // Modifying a value from a scalar to the JSON would fit the general
    // approach of compartmentalization, but may be backwards-incompatible.
    // Adding annotations as a separate top-level enum list/map would go
    // against this general approach.
    indent(f_out_) << "\"" << (*val_iter).name() << "\"" << " : "
                   << (*val_iter).get_value();
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
  const string& name = tconst->name();
  indent(f_out_) << "\"" << name << "\" : {" << endl;
  indent_up();
  print_lineno(*tconst);
  indent(f_out_) << "\"value\" : ";
  print_const_value(tconst->value());
  f_out_ << "," << endl;
  print_spec(tconst->type());
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
void t_json_generator::generate_struct(const t_structured* tstruct) {
  const string& name = tstruct->name();
  indent(f_out_) << "\"" << name << "\" : {" << endl;
  indent_up();
  print_lineno(*tstruct);
  indent(f_out_) << "\"is_exception\" : "
                 << (tstruct->is<t_exception>() ? "true" : "false") << ","
                 << endl;
  indent(f_out_) << "\"is_union\" : "
                 << (tstruct->is<t_union>() ? "true" : "false") << "," << endl;
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
    indent(f_out_) << "\"" << (*mem_iter)->name() << "\" : {" << endl;
    indent_up();
    print_spec((*mem_iter)->type().get_type());
    f_out_ << "," << endl
           << indent() << "\"required\" : "
           << ((*mem_iter)->qualifier() != t_field_qualifier::optional
                   ? "true"
                   : "false");
    const t_const_value* default_val = (*mem_iter)->default_value();
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
void t_json_generator::generate_xception(const t_structured* exception) {
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

  if (tservice->extends()) {
    indent(f_out_) << "\"extends\" : {" << endl;
    indent_up();
    print_spec(tservice->extends());
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
    string fn_name = (*fn_iter)->name();
    indent(f_out_) << "\"" << service_name_ << "." << fn_name << "\" : {"
                   << endl;
    indent_up();
    indent(f_out_) << "\"return_type\" : {" << endl;
    indent_up();
    const t_function* fun = *fn_iter;
    if (fun->is_interaction_constructor()) {
      print_spec(static_cast<const t_service*>(fun->interaction().get_type()));
    } else {
      print_spec(fun->return_type().get_type());
    }
    f_out_ << endl;
    indent_down();
    indent(f_out_) << "}," << endl;

    indent(f_out_) << "\"args\" : [";
    vector<t_field*> args = (*fn_iter)->params().get_members();
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
        print_name((*arg_iter)->name());
        print_spec((*arg_iter)->type().get_type());
        if ((*arg_iter)->default_value() != nullptr) {
          f_out_ << "," << endl << indent() << "\"value\" : ";
          print_const_value((*arg_iter)->default_value());
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
        indent(f_out_) << to_spec_args((*ex_iter).type().get_type());
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
  return ttype->is<t_placeholder_typedef>();
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
        resolved_location resolved_loc = sm.resolve_location(loc);
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

} // namespace apache::thrift::compiler
