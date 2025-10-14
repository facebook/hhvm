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
#include <list>
#include <string>
#include <utility>
#include <vector>

#include <stdlib.h>
#include <sstream>
#include <thrift/compiler/generate/t_concat_generator.h>

using namespace std;

namespace apache::thrift::compiler {

/**
 * JS code generator.
 */
class t_js_generator : public t_concat_generator {
 public:
  using t_concat_generator::t_concat_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    gen_node_ = options.find("node") != options.end();
    gen_jquery_ = options.find("jquery") != options.end();
    out_dir_base_ = gen_node_ ? "gen-nodejs" : "gen-js";
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

  std::string render_recv_throw(const std::string& var);
  std::string render_recv_return(const std::string& var);

  std::string render_const_value(
      const t_type* type, const t_const_value* value);

  /**
   * Structs!
   */
  void generate_js_struct(const t_structured* tstruct, bool is_exception);
  void generate_js_struct_definition(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_exception = false,
      bool is_exported = true,
      const std::string& namePrefix = "");
  void generate_js_struct_reader(
      std::ofstream& out, const t_structured* tstruct, const std::string& name);
  void generate_js_struct_writer(
      std::ofstream& out, const t_structured* tstruct, const std::string& name);
  void generate_js_function_helpers(const t_function* tfunction);

  /**
   * Service-level generation functions
   */
  void generate_service_helpers(const t_service* tservice);
  void generate_service_interface(const t_service* tservice);
  void generate_service_rest(const t_service* tservice);
  void generate_service_client(const t_service* tservice);
  void generate_service_processor(const t_service* tservice);
  void generate_process_function(
      const t_service* tservice, const t_function* tfunction);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field(
      std::ofstream& out,
      const t_field* tfield,
      const std::string& prefix = "",
      bool inclass = false);

  void generate_deserialize_struct(
      std::ofstream& out,
      const t_struct* tstruct,
      const std::string& prefix = "");

  void generate_deserialize_container(
      std::ofstream& out, const t_type* ttype, const std::string& prefix = "");

  void generate_deserialize_set_element(
      std::ofstream& out, const t_set* tset, const std::string& prefix = "");

  void generate_deserialize_map_element(
      std::ofstream& out, const t_map* tmap, const std::string& prefix = "");

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

  /**
   * Helper rendering functions
   */

  std::string js_includes();
  std::string render_includes();
  std::string declare_field(
      const t_field* tfield, bool init = false, bool obj = false);
  std::string function_signature(
      const t_function* tfunction,
      const std::string& prefix = "",
      bool include_callback = false);
  std::string argument_list(
      const t_paramlist& tparamlist, bool include_callback = false);
  std::string type_to_enum(const t_type* ttype);

  std::string autogen_comment() {
    return std::string("//\n") + "// Autogenerated by Thrift Compiler\n" +
        "//\n" +
        "// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE "
        "DOING\n" +
        "//\n";
  }

  std::vector<std::string> js_namespace_pieces(const t_program* p) {
    std::string ns = p->get_namespace("js");

    std::string::size_type loc;
    std::vector<std::string> pieces;

    if (ns.size() > 0) {
      while ((loc = ns.find('.')) != std::string::npos) {
        pieces.push_back(ns.substr(0, loc));
        ns = ns.substr(loc + 1);
      }
    }

    if (ns.size() > 0) {
      pieces.push_back(ns);
    }

    return pieces;
  }

  std::string js_type_namespace(const t_program* p) {
    if (gen_node_) {
      if (p != nullptr && p != program_) {
        return p->name() + "_ttypes.";
      }
      return "ttypes.";
    }
    return js_namespace(p);
  }

  std::string js_export_namespace(const t_program* p) {
    if (gen_node_) {
      return "exports.";
    }
    return js_namespace(p);
  }

  std::string js_namespace(const t_program* p) {
    std::string ns = p->get_namespace("js");
    if (ns.size() > 0) {
      ns += ".";
    }

    return ns;
  }

  std::string js_node_module(const t_program* p) {
    std::string node_module = p->get_namespace("node_module");
    if (node_module.size() == 0) {
      node_module = ".";
    }
    return node_module;
  }

 private:
  /**
   * True iff we should generate NodeJS-friendly RPC services.
   */
  bool gen_node_;

  /**
   * True if we should generate services that use jQuery ajax (async/sync).
   */
  bool gen_jquery_;

  /**
   * File streams
   */
  std::ofstream f_types_;
  std::ofstream f_service_;
};

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_js_generator::init_generator() {
  // Make output directory
  std::filesystem::create_directory(get_out_dir());

  string outdir = get_out_dir();

  // Make output file
  string f_types_name = outdir + program_->name() + "_types.js";
  f_types_.open(f_types_name.c_str());

  // Print header
  f_types_ << autogen_comment() << js_includes() << endl;

  if (gen_node_) {
    f_types_ << "var ttypes = module.exports = {};" << endl;
  }

  string pns;

  // setup the namespace
  // TODO should the namespace just be in the directory structure for node?
  vector<string> ns_pieces = js_namespace_pieces(program_);
  if (ns_pieces.size() > 0) {
    for (size_t i = 0; i < ns_pieces.size(); ++i) {
      pns += ((i == 0) ? "" : ".") + ns_pieces[i];
      f_types_ << "if (typeof " << pns << " === 'undefined') {" << endl;
      f_types_ << "  " << pns << " = {};" << endl;
      f_types_ << "}" << endl;
    }
  }
}

/**
 * Prints standard js imports
 */
string t_js_generator::js_includes() {
  if (gen_node_) {
    return string("var Thrift = require('thrift').Thrift;");
  }
  string inc;

  return inc;
}

/**
 * Renders all the imports necessary for including another Thrift program
 */
string t_js_generator::render_includes() {
  if (gen_node_) {
    const vector<t_program*>& includes = program_->get_includes_for_codegen();
    string result;
    for (size_t i = 0; i < includes.size(); ++i) {
      result += "var " + includes[i]->name() + "_ttypes = require('" +
          js_node_module(includes[i]) + "/" + includes[i]->name() +
          "_types')\n";
    }
    if (includes.size() > 0) {
      result += "\n";
    }
    return result;
  }
  string inc;

  return inc;
}

/**
 * Close up (or down) some filez.
 */
void t_js_generator::close_generator() {
  // Close types file

  f_types_.close();
}

/**
 * Generates a typedef. This is not done in JS, types are all implicit.
 *
 * @param ttypedef The type definition
 */
void t_js_generator::generate_typedef(const t_typedef* ttypedef) {
  (void)ttypedef;
}

/**
 * Generates code for an enumerated type. Since define is expensive to lookup
 * in JS, we use a global array for this.
 *
 * @param tenum The enumeration
 */
void t_js_generator::generate_enum(const t_enum* tenum) {
  f_types_ << js_type_namespace(tenum->program()) << tenum->name() << " = {"
           << endl;

  vector<t_enum_value*> constants = tenum->get_enum_values();
  vector<t_enum_value*>::iterator c_iter;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int value = (*c_iter)->get_value();
    f_types_ << "'" << (*c_iter)->name() << "' : " << value;
    if (c_iter != constants.end() - 1) {
      f_types_ << ",";
    }
    f_types_ << endl;
  }

  f_types_ << "};" << endl;
}

/**
 * Generate a constant value
 */
void t_js_generator::generate_const(const t_const* tconst) {
  const t_type* type = tconst->type();
  const string& name = tconst->name();
  const t_const_value* value = tconst->value();

  f_types_ << js_type_namespace(program_) << name << " = ";
  f_types_ << render_const_value(type, value) << ";" << endl;
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
string t_js_generator::render_const_value(
    const t_type* type, const t_const_value* value) {
  std::ostringstream out;

  type = type->get_true_type();

  if (const auto* primitive = type->try_as<t_primitive_type>()) {
    t_primitive_type::type tbase = primitive->primitive_type();
    switch (tbase) {
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        out << "'" << value->get_string() << "'";
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
        if (value->kind() == t_const_value::CV_INTEGER) {
          out << value->get_integer();
        } else {
          out << value->get_double();
        }
        break;
      default:
        throw std::runtime_error(
            "compiler error: no const of base type " +
            t_primitive_type::type_name(tbase));
    }
  } else if (type->is<t_enum>()) {
    out << value->get_integer();
  } else if (type->is<t_structured>()) {
    out << "new " << js_type_namespace(type->program()) << type->name() << "({"
        << endl;
    indent_up();
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      const t_type* field_type = nullptr;
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if ((*f_iter)->name() == v_iter->first->get_string()) {
          field_type = (*f_iter)->get_type();
        }
      }
      if (field_type == nullptr) {
        throw std::runtime_error(
            "type error: " + type->name() + " has no field " +
            v_iter->first->get_string());
      }
      if (v_iter != val.begin()) {
        out << ",";
      }
      out << render_const_value(&t_primitive_type::t_string(), v_iter->first);
      out << " : ";
      out << render_const_value(field_type, v_iter->second);
    }

    out << "})";
  } else if (const t_map* map = type->try_as<t_map>()) {
    const t_type* ktype = &map->key_type().deref();
    const t_type* vtype = &map->val_type().deref();
    out << "{";

    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      if (v_iter != val.begin()) {
        out << "," << endl;
      }

      out << render_const_value(ktype, v_iter->first);

      out << " : ";
      out << render_const_value(vtype, v_iter->second);
    }

    out << endl << "}";
  } else if (type->is<t_list>() || type->is<t_set>()) {
    const t_type* etype;
    if (const t_list* list = type->try_as<t_list>()) {
      etype = list->get_elem_type();
    } else {
      etype = ((t_set*)type)->get_elem_type();
    }
    out << "[";
    const vector<t_const_value*>& val = value->get_list();
    vector<t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      if (v_iter != val.begin()) {
        out << ",";
      }
      out << render_const_value(etype, *v_iter);
    }
    out << "]";
  }
  return out.str();
}

/**
 * Make a struct
 */
void t_js_generator::generate_struct(const t_structured* tstruct) {
  generate_js_struct(tstruct, false /* is_exception */);
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_js_generator::generate_xception(const t_structured* txception) {
  generate_js_struct(txception, true /* is_exception */);
}

/**
 * Structs can be normal or exceptions.
 */
void t_js_generator::generate_js_struct(
    const t_structured* tstruct, bool is_exception) {
  generate_js_struct_definition(f_types_, tstruct, is_exception);
}

/**
 * Generates a struct definition for a thrift data type. This is nothing in JS
 * where the objects are all just associative arrays (unless of course we
 * decide to start using objects for them...)
 *
 * @param tstruct The struct definition
 */
void t_js_generator::generate_js_struct_definition(
    ofstream& out,
    const t_structured* tstruct,
    bool is_exception,
    bool is_exported,
    const std::string& name_prefix) {
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;
  std::string name = name_prefix + tstruct->name();
  indent_up();

  if (gen_node_) {
    if (is_exported) {
      out << js_namespace(tstruct->program()) << name << " = "
          << "module.exports." << name << " = function(args) {\n";
    } else {
      out << js_namespace(tstruct->program()) << name
          << " = function(args) {\n";
    }
  } else {
    out << js_namespace(tstruct->program()) << name << " = function(args) {\n";
  }

  if (gen_node_ && is_exception) {
    out << indent() << "Thrift.TException.call(this, \""
        << js_namespace(tstruct->program()) << name << "\")" << endl;
    out << indent() << "this.name = \"" << js_namespace(tstruct->program())
        << name << "\"" << endl;
  }

  // members with arguments
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string dval = declare_field(*m_iter, false, true);
    const t_type* t = (*m_iter)->get_type()->get_true_type();
    if ((*m_iter)->get_value() != nullptr && !(t->is<t_structured>())) {
      dval = render_const_value((*m_iter)->get_type(), (*m_iter)->get_value());
      out << indent() << "this." << (*m_iter)->name() << " = " << dval << ";"
          << endl;
    } else {
      out << indent() << dval << ";" << endl;
    }
  }

  // Generate constructor from array
  if (members.size() > 0) {
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      const t_type* t = (*m_iter)->get_type()->get_true_type();
      if ((*m_iter)->get_value() != nullptr && (t->is<t_structured>())) {
        indent(out) << "this." << (*m_iter)->name() << " = "
                    << render_const_value(t, (*m_iter)->get_value()) << ";"
                    << endl;
      }
    }

    // Early returns for exceptions
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      const t_type* t = (*m_iter)->get_type()->get_true_type();
      if (t->is<t_exception>()) {
        out << indent() << "if (args instanceof "
            << js_type_namespace(t->program()) << t->name() << ") {" << endl
            << indent() << indent() << "this." << (*m_iter)->name()
            << " = args;" << endl
            << indent() << indent() << "return;" << endl
            << indent() << "}" << endl;
      }
    }

    out << indent() << "if (args) {" << endl;

    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      out << indent() << indent() << "if (args." << (*m_iter)->name()
          << " !== undefined) {" << endl
          << indent() << indent() << indent() << "this." << (*m_iter)->name()
          << " = args." << (*m_iter)->name() << ";" << endl
          << indent() << indent() << "}" << endl;
    }

    out << indent() << "}" << endl;
  }

  indent_down();
  out << "};\n";

  if (is_exception) {
    out << "Thrift.inherits(" << js_namespace(tstruct->program()) << name
        << ", Thrift.TException);" << endl;
    out << js_namespace(tstruct->program()) << name << ".prototype.name = '"
        << name << "';" << endl;
  } else {
    // init prototype
    out << js_namespace(tstruct->program()) << name << ".prototype = {};\n";
  }

  generate_js_struct_reader(out, tstruct, name);
  generate_js_struct_writer(out, tstruct, name);
}

/**
 * Generates the read() method for a struct
 */
void t_js_generator::generate_js_struct_reader(
    ofstream& out, const t_structured* tstruct, const std::string& name) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  out << js_namespace(tstruct->program()) << name
      << ".prototype.read = function(input) {" << endl;

  indent_up();

  indent(out) << "input.readStructBegin();" << endl;

  // Loop over reading in fields
  indent(out) << "while (true)" << endl;

  scope_up(out);

  indent(out) << "var ret = input.readFieldBegin();" << endl;
  indent(out) << "var fname = ret.fname;" << endl;
  indent(out) << "var ftype = ret.ftype;" << endl;
  indent(out) << "var fid = ret.fid;" << endl;

  // Check for field STOP marker and break
  indent(out) << "if (ftype == Thrift.Type.STOP) {" << endl;
  indent_up();
  indent(out) << "break;" << endl;
  indent_down();
  indent(out) << "}" << endl;
  if (!fields.empty()) {
    // Switch statement on the field we are reading
    indent(out) << "switch (fid)" << endl;

    scope_up(out);

    // Generate deserialization code for known cases
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      indent(out) << "case " << (*f_iter)->get_key() << ":" << endl;
      indent(out) << "if (ftype == " << type_to_enum((*f_iter)->get_type())
                  << ") {" << endl;

      indent_up();
      generate_deserialize_field(out, *f_iter, "this.");
      indent_down();

      indent(out) << "} else {" << endl;

      indent(out) << "  input.skip(ftype);" << endl;

      out << indent() << "}" << endl << indent() << "break;" << endl;
    }
    if (fields.size() == 1) {
      // pseudo case to make jslint happy
      indent(out) << "case 0:" << endl;
      indent(out) << "  input.skip(ftype);" << endl;
      indent(out) << "  break;" << endl;
    }
    // In the default case we skip the field
    indent(out) << "default:" << endl;
    indent(out) << "  input.skip(ftype);" << endl;

    scope_down(out);
  } else {
    indent(out) << "input.skip(ftype);" << endl;
  }

  indent(out) << "input.readFieldEnd();" << endl;

  scope_down(out);

  indent(out) << "input.readStructEnd();" << endl;

  indent(out) << "return;" << endl;

  indent_down();
  out << indent() << "};" << endl << endl;
}

/**
 * Generates the write() method for a struct
 */
void t_js_generator::generate_js_struct_writer(
    ofstream& out, const t_structured* tstruct, const std::string& name) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  out << js_namespace(tstruct->program()) << name
      << ".prototype.write = function(output) {" << endl;

  indent_up();

  indent(out) << "output.writeStructBegin('" << name << "');" << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    out << indent() << "if (this." << (*f_iter)->name() << " !== null && this."
        << (*f_iter)->name() << " !== undefined) {" << endl;
    indent_up();

    indent(out) << "output.writeFieldBegin(" << "'" << (*f_iter)->name()
                << "', " << type_to_enum((*f_iter)->get_type()) << ", "
                << (*f_iter)->get_key() << ");" << endl;

    // Write field contents
    generate_serialize_field(out, *f_iter, "this.");

    indent(out) << "output.writeFieldEnd();" << endl;

    indent_down();
    indent(out) << "}" << endl;
  }

  out << indent() << "output.writeFieldStop();" << endl
      << indent() << "output.writeStructEnd();" << endl;

  out << indent() << "return;" << endl;

  indent_down();
  out << indent() << "};" << endl << endl;
}

/**
 * Generates a thrift service.
 *
 * @param tservice The service definition
 */
void t_js_generator::generate_service(const t_service* tservice) {
  string f_service_name = get_out_dir() + service_name_ + ".js";
  f_service_.open(f_service_name.c_str());

  f_service_ << autogen_comment() << js_includes() << endl
             << render_includes() << endl;

  if (gen_node_) {
    if (tservice->extends() != nullptr) {
      f_service_ << "var " << tservice->extends()->name() << " = require('"
                 << js_node_module(tservice->extends()->program()) << "/"
                 << tservice->extends()->name() << "')" << endl
                 << "var " << tservice->extends()->name()
                 << "Client = " << tservice->extends()->name() << ".Client"
                 << endl
                 << "var " << tservice->extends()->name()
                 << "Processor = " << tservice->extends()->name()
                 << ".Processor" << endl;
    }

    f_service_ << "var ttypes = require('./" + program_->name() + "_types');"
               << endl;
  }

  generate_service_helpers(tservice);
  generate_service_interface(tservice);
  generate_service_client(tservice);

  if (gen_node_) {
    generate_service_processor(tservice);
  }

  f_service_.close();
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_js_generator::generate_service_processor(const t_service* tservice) {
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  f_service_ << js_namespace(tservice->program()) << service_name_
             << "Processor = " << "exports.Processor = function(handler) ";

  scope_up(f_service_);

  f_service_ << indent() << "this._handler = handler" << endl;

  scope_down(f_service_);

  if (tservice->extends() != nullptr) {
    indent(f_service_) << "Thrift.inherits("
                       << js_namespace(tservice->program()) << service_name_
                       << "Processor, " << tservice->extends()->name()
                       << "Processor)" << endl;
  }

  // Generate the server implementation
  indent(f_service_)
      << js_namespace(tservice->program()) << service_name_
      << "Processor.prototype.process = function(input, output) ";

  scope_up(f_service_);

  f_service_ << indent() << "var r = input.readMessageBegin();" << endl
             << indent() << "if (this['process_' + r.fname]) {" << endl
             << indent()
             << "  return this['process_' + r.fname].call(this, r.rseqid, "
                "input, output);"
             << endl
             << indent() << "} else {" << endl
             << indent() << "  input.skip(Thrift.Type.STRUCT);" << endl
             << indent() << "  input.readMessageEnd();" << endl
             << indent()
             << "  var x = new "
                "Thrift.TApplicationException(Thrift.TApplicationExceptionType."
                "UNKNOWN_METHOD, 'Unknown function ' + r.fname);"
             << endl
             << indent()
             << "  output.writeMessageBegin(r.fname, "
                "Thrift.MessageType.Exception, r.rseqid);"
             << endl
             << indent() << "  x.write(output);" << endl
             << indent() << "  output.writeMessageEnd();" << endl
             << indent() << "  output.flush();" << endl
             << indent() << "}" << endl;

  scope_down(f_service_);
  f_service_ << endl;

  // Generate the process subfunctions
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    generate_process_function(tservice, *f_iter);
  }
}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_js_generator::generate_process_function(
    const t_service* tservice, const t_function* tfunction) {
  indent(f_service_) << js_namespace(tservice->program()) << service_name_
                     << "Processor.prototype.process_" + tfunction->name() +
          " = function(seqid, input, output) ";

  scope_up(f_service_);

  string argsname = js_namespace(program_) + service_name_ + "_" +
      tfunction->name() + "_args";
  string resultname = js_namespace(program_) + service_name_ + "_" +
      tfunction->name() + "_result";

  f_service_ << indent() << "var args = new " << argsname << "();" << endl
             << indent() << "args.read(input);" << endl
             << indent() << "input.readMessageEnd();" << endl;

  // Generate the function call
  const t_paramlist& arg_struct = tfunction->params();
  const std::vector<t_field*>& fields = arg_struct.get_members();
  vector<t_field*>::const_iterator f_iter;

  f_service_ << indent() << "this._handler." << tfunction->name() << "(";

  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      f_service_ << ", ";
    }
    f_service_ << "args." << (*f_iter)->name();
  }

  // Shortcut out here for oneway functions
  if (tfunction->qualifier() == t_function_qualifier::oneway) {
    f_service_ << ")" << endl;
    scope_down(f_service_);
    f_service_ << endl;
    return;
  }

  if (!first) {
    f_service_ << ", ";
  }
  f_service_ << "function (err, result) {" << endl;
  indent_up();

  f_service_ << indent() << "var result = new " << resultname
             << "((err != null ? err : {success: result}));" << endl
             << indent() << "output.writeMessageBegin(\"" << tfunction->name()
             << "\", Thrift.MessageType.REPLY, seqid);" << endl
             << indent() << "result.write(output);" << endl
             << indent() << "output.writeMessageEnd();" << endl
             << indent() << "output.flush();" << endl;

  indent_down();
  indent(f_service_) << "})" << endl;

  scope_down(f_service_);
  f_service_ << endl;
}

/**
 * Generates helper functions for a service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_js_generator::generate_service_helpers(const t_service* tservice) {
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  f_service_ << "//HELPER FUNCTIONS AND STRUCTURES" << endl << endl;
  std::string prefix = service_name_ + "_";
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    const t_paramlist& ts = (*f_iter)->params();
    generate_js_struct_definition(f_service_, &ts, false, false, prefix);
    generate_js_function_helpers(*f_iter);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_js_generator::generate_js_function_helpers(const t_function* tfunction) {
  t_struct result(
      program_, service_name_ + "_" + tfunction->name() + "_result");
  auto success =
      std::make_unique<t_field>(tfunction->return_type(), "success", 0);
  if (!tfunction->return_type()->is_void()) {
    result.append(std::move(success));
  }

  for (const t_field& x : get_elems(tfunction->exceptions())) {
    result.append(x.clone_DO_NOT_USE());
  }

  generate_js_struct_definition(f_service_, &result, false, false);
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 */
void t_js_generator::generate_service_interface(const t_service* tservice) {
  (void)tservice;
}

/**
 * Generates a REST interface
 */
void t_js_generator::generate_service_rest(const t_service* tservice) {
  (void)tservice;
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_js_generator::generate_service_client(const t_service* tservice) {
  string extends;

  if (gen_node_) {
    f_service_ << js_namespace(tservice->program()) << service_name_
               << "Client = " << "exports.Client = function(output, pClass) {"
               << endl;
  } else {
    f_service_ << js_namespace(tservice->program()) << service_name_
               << "Client = function(input, output) {" << endl;
  }

  indent_up();

  if (gen_node_) {
    f_service_ << indent() << "  this.output = output;" << endl
               << indent() << "  this.pClass = pClass;" << endl
               << indent() << "  this.seqid = 0;" << endl
               << indent() << "  this._reqs = {};" << endl;
  } else {
    f_service_ << indent() << "  this.input = input;" << endl
               << indent() << "  this.output = (!output) ? input : output;"
               << endl
               << indent() << "  this.seqid = 0;" << endl;
  }

  indent_down();

  f_service_ << indent() << "};" << endl;

  if (tservice->extends() != nullptr) {
    indent(f_service_) << "Thrift.inherits("
                       << js_namespace(tservice->program()) << service_name_
                       << "Client, " << tservice->extends()->name() << "Client)"
                       << endl;
  } else {
    // init prototype
    indent(f_service_) << js_namespace(tservice->program()) << service_name_
                       << "Client.prototype = {};" << endl;
  }

  // Generate client method implementations
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::const_iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    const t_paramlist& arg_struct = (*f_iter)->params();
    const vector<t_field*>& fields = arg_struct.get_members();
    vector<t_field*>::const_iterator fld_iter;
    string funname = (*f_iter)->name();
    string arglist = argument_list(arg_struct);

    // Open function
    f_service_ << js_namespace(tservice->program()) << service_name_
               << "Client.prototype."
               << function_signature(*f_iter, "", gen_node_ || gen_jquery_)
               << " {" << endl;

    indent_up();

    if (gen_node_) {
      f_service_ << indent() << "this.seqid += 1;" << endl
                 << indent() << "this._reqs[this.seqid] = callback;" << endl;
    } else if (gen_jquery_) {
      f_service_ << indent() << "if (callback === undefined) {" << endl;
      indent_up();
    }

    f_service_ << indent() << "this.send_" << funname << "(" << arglist << ");"
               << endl;

    if (!gen_node_ && (*f_iter)->qualifier() != t_function_qualifier::oneway) {
      f_service_ << indent();
      if (!(*f_iter)->return_type()->is_void()) {
        f_service_ << "return ";
      }
      f_service_ << "this.recv_" << funname << "();" << endl;
    }

    if (gen_jquery_) {
      indent_down();
      f_service_ << indent() << "} else {" << endl;
      indent_up();
      f_service_ << indent() << "var postData = this.send_" << funname << "("
                 << arglist << (arglist.empty() ? "" : ", ") << "true);"
                 << endl;
      f_service_ << indent() << "return this.output.getTransport()" << endl;
      indent_up();
      f_service_ << indent()
                 << ".jqRequest(this, postData, arguments, this.recv_"
                 << funname << ");" << endl;
      indent_down();
      indent_down();
      f_service_ << indent() << "}" << endl;
    }

    indent_down();

    f_service_ << "};" << endl << endl;

    // Send function
    f_service_ << js_namespace(tservice->program()) << service_name_
               << "Client.prototype.send_"
               << function_signature(*f_iter, "", gen_jquery_) << " {" << endl;

    indent_up();

    std::string outputVar;
    if (gen_node_) {
      f_service_ << indent() << "var output = new this.pClass(this.output);"
                 << endl;
      outputVar = "output";
    } else {
      outputVar = "this.output";
    }

    std::string argsname = js_namespace(program_) + service_name_ + "_" +
        (*f_iter)->name() + "_args";

    // Serialize the request header
    f_service_ << indent() << outputVar << ".writeMessageBegin('"
               << (*f_iter)->name()
               << "', Thrift.MessageType.CALL, this.seqid);" << endl;

    f_service_ << indent() << "var args = new " << argsname << "();" << endl;

    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      f_service_ << indent() << "args." << (*fld_iter)->name() << " = "
                 << (*fld_iter)->name() << ";" << endl;
    }

    // Write to the stream
    f_service_ << indent() << "args.write(" << outputVar << ");" << endl
               << indent() << outputVar << ".writeMessageEnd();" << endl;

    if (gen_node_) {
      f_service_ << indent() << "return this.output.flush();" << endl;
    } else {
      if (gen_jquery_) {
        f_service_ << indent()
                   << "return this.output.getTransport().flush(callback);"
                   << endl;
      } else {
        f_service_ << indent() << "return this.output.getTransport().flush();"
                   << endl;
      }
    }

    indent_down();

    f_service_ << "};" << endl;

    if ((*f_iter)->qualifier() != t_function_qualifier::oneway) {
      std::string resultname = js_namespace(tservice->program()) +
          service_name_ + "_" + (*f_iter)->name() + "_result";

      if (gen_node_) {
        // Open function
        f_service_ << endl
                   << js_namespace(tservice->program()) << service_name_
                   << "Client.prototype.recv_" << (*f_iter)->name()
                   << " = function(input,mtype,rseqid) {" << endl;
      } else {
        t_function recv_function(
            program_, (*f_iter)->return_type(), "recv_" + (*f_iter)->name());
        // Open function
        f_service_ << endl
                   << js_namespace(tservice->program()) << service_name_
                   << "Client.prototype." << function_signature(&recv_function)
                   << " {" << endl;
      }

      indent_up();

      std::string inputVar;
      if (gen_node_) {
        inputVar = "input";
      } else {
        inputVar = "this.input";
      }

      if (gen_node_) {
        f_service_ << indent()
                   << "var callback = this._reqs[rseqid] || function() {};"
                   << endl
                   << indent() << "delete this._reqs[rseqid];" << endl;
      } else {
        f_service_ << indent() << "var ret = this.input.readMessageBegin();"
                   << endl
                   << indent() << "var fname = ret.fname;" << endl
                   << indent() << "var mtype = ret.mtype;" << endl
                   << indent() << "var rseqid = ret.rseqid;" << endl;
      }

      f_service_ << indent() << "if (mtype == Thrift.MessageType.EXCEPTION) {"
                 << endl
                 << indent() << "  var x = new Thrift.TApplicationException();"
                 << endl
                 << indent() << "  x.read(" << inputVar << ");" << endl
                 << indent() << "  " << inputVar << ".readMessageEnd();" << endl
                 << indent() << "  " << render_recv_throw("x") << endl
                 << indent() << "}" << endl;

      f_service_ << indent() << "var result = new " << resultname << "();"
                 << endl
                 << indent() << "result.read(" << inputVar << ");" << endl;

      f_service_ << indent() << inputVar << ".readMessageEnd();" << endl
                 << endl;

      for (const t_field& x : get_elems((*f_iter)->exceptions())) {
        f_service_ << indent() << "if (null !== result." << x.name() << ") {"
                   << endl
                   << indent() << "  "
                   << render_recv_throw("result." + x.name()) << endl
                   << indent() << "}" << endl;
      }

      // Careful, only return result if not a void function
      if (!(*f_iter)->return_type()->is_void()) {
        f_service_ << indent() << "if (null !== result.success) {" << endl
                   << indent() << "  " << render_recv_return("result.success")
                   << endl
                   << indent() << "}" << endl;
        f_service_ << indent()
                   << render_recv_throw(
                          "'" + (*f_iter)->name() + " failed: unknown result'")
                   << endl;
      } else {
        if (gen_node_) {
          indent(f_service_) << "callback(null)" << endl;
        } else {
          indent(f_service_) << "return;" << endl;
        }
      }

      // Close function
      indent_down();
      f_service_ << "};" << endl;
    }
  }
}

std::string t_js_generator::render_recv_throw(const std::string& var) {
  if (gen_node_) {
    return "return callback(" + var + ");";
  } else {
    return "throw " + var + ";";
  }
}

std::string t_js_generator::render_recv_return(const std::string& var) {
  if (gen_node_) {
    return "return callback(null, " + var + ");";
  } else {
    return "return " + var + ";";
  }
}

/**
 * Deserializes a field of any type.
 */
void t_js_generator::generate_deserialize_field(
    ofstream& out, const t_field* tfield, const string& prefix, bool inclass) {
  (void)inclass;
  const t_type* type = tfield->get_type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->name());
  }

  string name = prefix + tfield->name();

  if (type->is<t_structured>()) {
    generate_deserialize_struct(out, (t_struct*)type, name);
  } else if (type->is<t_container>()) {
    generate_deserialize_container(out, type, name);
  } else if (type->is<t_primitive_type>() || type->is<t_enum>()) {
    indent(out) << name << " = input.";

    if (const auto* primitive = type->try_as<t_primitive_type>()) {
      t_primitive_type::type tbase = primitive->primitive_type();
      switch (tbase) {
        case t_primitive_type::type::t_void:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);
        case t_primitive_type::type::t_string:
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
        default:
          throw std::runtime_error(
              "compiler error: no JS name for base type " +
              t_primitive_type::type_name(tbase));
      }
    } else if (type->is<t_enum>()) {
      out << "readI32()";
    }

    if (!gen_node_) {
      out << ".value";
    }

    out << ";" << endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n",
        tfield->name().c_str(),
        type->name().c_str());
  }
}

/**
 * Generates an unserializer for a variable. This makes two key assumptions,
 * first that there is a const char* variable named data that points to the
 * buffer for deserialization, and that there is a variable protocol which
 * is a reference to a TProtocol serialization object.
 */
void t_js_generator::generate_deserialize_struct(
    ofstream& out, const t_struct* tstruct, const string& prefix) {
  out << indent() << prefix << " = new "
      << js_type_namespace(tstruct->program()) << tstruct->name() << "();"
      << endl
      << indent() << prefix << ".read(input);" << endl;
}

void t_js_generator::generate_deserialize_container(
    ofstream& out, const t_type* ttype, const string& prefix) {
  string size = tmp("_size");
  string ktype = tmp("_ktype");
  string vtype = tmp("_vtype");
  string etype = tmp("_etype");
  string rtmp3 = tmp("_rtmp3");

  t_field fsize(t_primitive_type::t_i32(), size);
  t_field fktype(t_primitive_type::t_byte(), ktype);
  t_field fvtype(t_primitive_type::t_byte(), vtype);
  t_field fetype(t_primitive_type::t_byte(), etype);

  out << indent() << "var " << size << " = 0;" << endl;
  out << indent() << "var " << rtmp3 << ";" << endl;

  // Declare variables, read header
  if (ttype->is<t_map>()) {
    out << indent() << prefix << " = {};" << endl
        << indent() << "var " << ktype << " = 0;" << endl
        << indent() << "var " << vtype << " = 0;" << endl;

    out << indent() << rtmp3 << " = input.readMapBegin();" << endl;
    out << indent() << ktype << " = " << rtmp3 << ".ktype;" << endl;
    out << indent() << vtype << " = " << rtmp3 << ".vtype;" << endl;
    out << indent() << size << " = " << rtmp3 << ".size;" << endl;

  } else if (ttype->is<t_set>()) {
    out << indent() << prefix << " = [];" << endl
        << indent() << "var " << etype << " = 0;" << endl
        << indent() << rtmp3 << " = input.readSetBegin();" << endl
        << indent() << etype << " = " << rtmp3 << ".etype;" << endl
        << indent() << size << " = " << rtmp3 << ".size;" << endl;

  } else if (ttype->is<t_list>()) {
    out << indent() << prefix << " = [];" << endl
        << indent() << "var " << etype << " = 0;" << endl
        << indent() << rtmp3 << " = input.readListBegin();" << endl
        << indent() << etype << " = " << rtmp3 << ".etype;" << endl
        << indent() << size << " = " << rtmp3 << ".size;" << endl;
  }

  // For loop iterates over elements
  string i = tmp("_i");
  indent(out) << "for (var " << i << " = 0; " << i << " < " << size << "; ++"
              << i << ")" << endl;

  scope_up(out);

  if (const t_map* map = ttype->try_as<t_map>()) {
    if (!gen_node_) {
      out << indent() << "if (" << i << " > 0 ) {" << endl
          << indent()
          << "  if (input.rstack.length > input.rpos[input.rpos.length -1] + "
             "1) {"
          << endl
          << indent() << "    input.rstack.pop();" << endl
          << indent() << "  }" << endl
          << indent() << "}" << endl;
    }

    generate_deserialize_map_element(out, map, prefix);
  } else if (const t_set* set = ttype->try_as<t_set>()) {
    generate_deserialize_set_element(out, set, prefix);
  } else if (const t_list* list = ttype->try_as<t_list>()) {
    generate_deserialize_list_element(out, list, prefix);
  }

  scope_down(out);

  // Read container end
  if (ttype->is<t_map>()) {
    indent(out) << "input.readMapEnd();" << endl;
  } else if (ttype->is<t_set>()) {
    indent(out) << "input.readSetEnd();" << endl;
  } else if (ttype->is<t_list>()) {
    indent(out) << "input.readListEnd();" << endl;
  }
}

/**
 * Generates code to deserialize a map
 */
void t_js_generator::generate_deserialize_map_element(
    ofstream& out, const t_map* tmap, const string& prefix) {
  string key = tmp("key");
  string val = tmp("val");
  t_field fkey(tmap->key_type().deref(), key);
  t_field fval(tmap->val_type().deref(), val);

  indent(out) << declare_field(&fkey, false, false) << ";" << endl;
  indent(out) << declare_field(&fval, false, false) << ";" << endl;

  generate_deserialize_field(out, &fkey);
  generate_deserialize_field(out, &fval);

  indent(out) << prefix << "[" << key << "] = " << val << ";" << endl;
}

void t_js_generator::generate_deserialize_set_element(
    ofstream& out, const t_set* tset, const string& prefix) {
  string elem = tmp("elem");
  t_field felem(*tset->get_elem_type(), elem);

  indent(out) << "var " << elem << " = null;" << endl;

  generate_deserialize_field(out, &felem);

  indent(out) << prefix << ".push(" << elem << ");" << endl;
}

void t_js_generator::generate_deserialize_list_element(
    ofstream& out, const t_list* tlist, const string& prefix) {
  string elem = tmp("elem");
  t_field felem(*tlist->get_elem_type(), elem);

  indent(out) << "var " << elem << " = null;" << endl;

  generate_deserialize_field(out, &felem);

  indent(out) << prefix << ".push(" << elem << ");" << endl;
}

/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param prefix Name to prepend to field name
 */
void t_js_generator::generate_serialize_field(
    ofstream& out, const t_field* tfield, const string& prefix) {
  const t_type* type = tfield->get_type()->get_true_type();

  // Do nothing for void types
  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->name());
  }

  if (type->is<t_structured>()) {
    generate_serialize_struct(out, (t_struct*)type, prefix + tfield->name());
  } else if (type->is<t_container>()) {
    generate_serialize_container(out, type, prefix + tfield->name());
  } else if (type->is<t_primitive_type>() || type->is<t_enum>()) {
    string name = tfield->name();

    // Hack for when prefix is defined (always a hash ref)
    if (!prefix.empty()) {
      name = prefix + tfield->name();
    }

    indent(out) << "output.";

    if (const auto* primitive = type->try_as<t_primitive_type>()) {
      t_primitive_type::type tbase = primitive->primitive_type();
      switch (tbase) {
        case t_primitive_type::type::t_void:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              name);
        case t_primitive_type::type::t_string:
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
        default:
          throw std::runtime_error(
              "compiler error: no JS name for base type " +
              t_primitive_type::type_name(tbase));
      }
    } else if (type->is<t_enum>()) {
      out << "writeI32(" << name << ")";
    }
    out << ";" << endl;

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
void t_js_generator::generate_serialize_struct(
    ofstream& out, const t_struct* tstruct, const string& prefix) {
  (void)tstruct;
  indent(out) << prefix << ".write(output);" << endl;
}

/**
 * Writes out a container
 */
void t_js_generator::generate_serialize_container(
    ofstream& out, const t_type* ttype, const string& prefix) {
  if (const t_map* map = ttype->try_as<t_map>()) {
    indent(out) << "output.writeMapBegin("
                << type_to_enum(&map->key_type().deref()) << ", "
                << type_to_enum(&map->val_type().deref()) << ", "
                << "Thrift.objectLength(" << prefix << "));" << endl;
  } else if (const t_set* set = ttype->try_as<t_set>()) {
    indent(out) << "output.writeSetBegin(" << type_to_enum(set->get_elem_type())
                << ", " << prefix << ".length);" << endl;

  } else if (const t_list* list = ttype->try_as<t_list>()) {
    indent(out) << "output.writeListBegin("
                << type_to_enum(list->get_elem_type()) << ", " << prefix
                << ".length);" << endl;
  }

  if (const t_map* map = ttype->try_as<t_map>()) {
    string kiter = tmp("kiter");
    string viter = tmp("viter");
    indent(out) << "for (var " << kiter << " in " << prefix << ")" << endl;
    scope_up(out);
    indent(out) << "if (" << prefix << ".hasOwnProperty(" << kiter << "))"
                << endl;
    scope_up(out);
    indent(out) << "var " << viter << " = " << prefix << "[" << kiter << "];"
                << endl;
    generate_serialize_map_element(out, map, kiter, viter);
    scope_down(out);
    scope_down(out);

  } else if (const t_set* set = ttype->try_as<t_set>()) {
    string iter = tmp("iter");
    indent(out) << "for (var " << iter << " in " << prefix << ")" << endl;
    scope_up(out);
    indent(out) << "if (" << prefix << ".hasOwnProperty(" << iter << "))"
                << endl;
    scope_up(out);
    indent(out) << iter << " = " << prefix << "[" << iter << "];" << endl;
    generate_serialize_set_element(out, set, iter);
    scope_down(out);
    scope_down(out);

  } else if (const t_list* list = ttype->try_as<t_list>()) {
    string iter = tmp("iter");
    indent(out) << "for (var " << iter << " in " << prefix << ")" << endl;
    scope_up(out);
    indent(out) << "if (" << prefix << ".hasOwnProperty(" << iter << "))"
                << endl;
    scope_up(out);
    indent(out) << iter << " = " << prefix << "[" << iter << "];" << endl;
    generate_serialize_list_element(out, list, iter);
    scope_down(out);
    scope_down(out);
  }

  if (ttype->is<t_map>()) {
    indent(out) << "output.writeMapEnd();" << endl;
  } else if (ttype->is<t_set>()) {
    indent(out) << "output.writeSetEnd();" << endl;
  } else if (ttype->is<t_list>()) {
    indent(out) << "output.writeListEnd();" << endl;
  }
}

/**
 * Serializes the members of a map.
 *
 */
void t_js_generator::generate_serialize_map_element(
    ofstream& out, const t_map* tmap, string kiter, string viter) {
  t_field kfield(tmap->key_type().deref(), std::move(kiter));
  generate_serialize_field(out, &kfield);

  t_field vfield(tmap->val_type().deref(), std::move(viter));
  generate_serialize_field(out, &vfield);
}

/**
 * Serializes the members of a set.
 */
void t_js_generator::generate_serialize_set_element(
    ofstream& out, const t_set* tset, string iter) {
  t_field efield(*tset->get_elem_type(), std::move(iter));
  generate_serialize_field(out, &efield);
}

/**
 * Serializes the members of a list.
 */
void t_js_generator::generate_serialize_list_element(
    ofstream& out, const t_list* tlist, string iter) {
  t_field efield(*tlist->get_elem_type(), std::move(iter));
  generate_serialize_field(out, &efield);
}

/**
 * Declares a field, which may include initialization as necessary.
 *
 * @param ttype The type
 */
string t_js_generator::declare_field(
    const t_field* tfield, bool init, bool obj) {
  string result = "this." + tfield->name();

  if (!obj) {
    result = "var " + tfield->name();
  }

  if (init) {
    const t_type* type = tfield->get_type()->get_true_type();
    if (const auto* primitive = type->try_as<t_primitive_type>()) {
      t_primitive_type::type tbase = primitive->primitive_type();
      switch (tbase) {
        case t_primitive_type::type::t_void:
          break;
        case t_primitive_type::type::t_string:
        case t_primitive_type::type::t_binary:
        case t_primitive_type::type::t_bool:
        case t_primitive_type::type::t_byte:
        case t_primitive_type::type::t_i16:
        case t_primitive_type::type::t_i32:
        case t_primitive_type::type::t_i64:
        case t_primitive_type::type::t_double:
          result += " = null";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no JS initializer for base type " +
              t_primitive_type::type_name(tbase));
      }
    } else if (type->is<t_enum>()) {
      result += " = null";
    } else if (type->is<t_map>()) {
      result += " = null";
    } else if (type->is<t_container>()) {
      result += " = null";
    } else if (type->is<t_structured>()) {
      if (obj) {
        result += " = new " + js_type_namespace(type->program()) +
            type->name() + "()";
      } else {
        result += " = null";
      }
    }
  } else {
    result += " = null";
  }
  return result;
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_js_generator::function_signature(
    const t_function* tfunction, const string& prefix, bool include_callback) {
  string str;

  str = prefix + tfunction->name() + " = function(";

  str += argument_list(tfunction->params(), include_callback);

  str += ")";
  return str;
}

/**
 * Renders a field list
 */
string t_js_generator::argument_list(
    const t_paramlist& tparamlist, bool include_callback) {
  string result;

  const vector<t_field*>& fields = tparamlist.get_members();
  vector<t_field*>::const_iterator f_iter;
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }
    result += (*f_iter)->name();
  }

  if (include_callback) {
    if (!fields.empty()) {
      result += ", ";
    }
    result += "callback";
  }

  return result;
}

/**
 * Converts the parse type to a C++ enum string for the given type.
 */
string t_js_generator ::type_to_enum(const t_type* type) {
  type = type->get_true_type();

  if (const auto* primitive = type->try_as<t_primitive_type>()) {
    t_primitive_type::type tbase = primitive->primitive_type();
    switch (tbase) {
      case t_primitive_type::type::t_void:
        throw std::runtime_error("NO T_VOID CONSTRUCT");
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        return "Thrift.Type.STRING";
      case t_primitive_type::type::t_bool:
        return "Thrift.Type.BOOL";
      case t_primitive_type::type::t_byte:
        return "Thrift.Type.BYTE";
      case t_primitive_type::type::t_i16:
        return "Thrift.Type.I16";
      case t_primitive_type::type::t_i32:
        return "Thrift.Type.I32";
      case t_primitive_type::type::t_i64:
        return "Thrift.Type.I64";
      case t_primitive_type::type::t_double:
        return "Thrift.Type.DOUBLE";
      case t_primitive_type::type::t_float:
        throw std::runtime_error("Float type is not supported");
    }
  } else if (type->is<t_enum>()) {
    return "Thrift.Type.I32";
  } else if (type->is<t_structured>()) {
    return "Thrift.Type.STRUCT";
  } else if (type->is<t_map>()) {
    return "Thrift.Type.MAP";
  } else if (type->is<t_set>()) {
    return "Thrift.Type.SET";
  } else if (type->is<t_list>()) {
    return "Thrift.Type.LIST";
  }

  throw std::runtime_error("INVALID TYPE IN type_to_enum: " + type->name());
}

THRIFT_REGISTER_GENERATOR(
    js,
    "Javascript",
    "    jquery:          Generate jQuery compatible code.\n"
    "    node:            Generate node.js compatible code.\n");

} // namespace apache::thrift::compiler
