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

#include <thrift/compiler/generate/t_java_deprecated_generator.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

using namespace std;

namespace apache {
namespace thrift {
namespace compiler {
namespace {
std::string upcase_string(std::string original) {
  std::transform(original.begin(), original.end(), original.begin(), ::toupper);
  return original;
}
} // namespace

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_java_deprecated_generator::init_generator() {
  out_dir_base_ = "gen-javadeprecated";

  // Make output directory
  boost::filesystem::create_directory(get_out_dir());
  namespace_key_ = "java";
  package_name_ = program_->get_namespace(namespace_key_);

  string dir = package_name_;
  string subdir = get_out_dir();
  string::size_type loc;
  while ((loc = dir.find('.')) != string::npos) {
    subdir = subdir + "/" + dir.substr(0, loc);
    boost::filesystem::create_directory(subdir);
    dir = dir.substr(loc + 1);
  }
  if (dir.size() > 0) {
    subdir = subdir + "/" + dir;
    boost::filesystem::create_directory(subdir);
  }

  package_dir_ = subdir;
}

/**
 * Packages the generated file
 *
 * @return String of the package, i.e. "package com.facebook.thriftdemo;"
 */
string t_java_deprecated_generator::java_package() {
  if (!package_name_.empty()) {
    return string("package ") + package_name_ + ";\n\n";
  }
  return "";
}

/**
 * @return String indicating the parent class for the generated struct
 */
boost::optional<string> t_java_deprecated_generator::java_struct_parent_class(
    const t_struct* /* unused */, StructGenParams params) {
  return boost::make_optional(params.is_exception, std::string("Exception"));
}

/**
 * Prints standard java imports for a thrift service
 *
 * @return List of imports for Java types that are used in here
 */
string t_java_deprecated_generator::java_service_imports() {
  return java_struct_imports() +
      "import org.slf4j.Logger;\nimport org.slf4j.LoggerFactory;\n\n";
}

/**
 * Prints standard java imports for a thrift structure
 *
 * @return List of imports for Java types that are used in here
 */
string t_java_deprecated_generator::java_struct_imports() {
  return string() + "import java.util.List;\n" +
      "import java.util.ArrayList;\nimport java.util.Map;\n" +
      "import java.util.HashMap;\nimport java.util.Set;\n" +
      "import java.util.HashSet;\nimport java.util.Collections;\n" +
      "import java.util.BitSet;\nimport java.util.Arrays;\n";
}

/**
 * Prints standard java imports
 *
 * @return List of imports necessary for thrift
 */
string t_java_deprecated_generator::java_thrift_imports() {
  return string() + "import com.facebook.thrift.*;\n" +
      "import com.facebook.thrift.annotations.*;\n" +
      "import com.facebook.thrift.async.*;\n" +
      "import com.facebook.thrift.meta_data.*;\n" +
      "import com.facebook.thrift.server.*;\n" +
      "import com.facebook.thrift.transport.*;\n" +
      "import com.facebook.thrift.protocol.*;\n\n";
}

string t_java_deprecated_generator::java_suppress_warnings_enum() {
  return string() + "@SuppressWarnings({ \"unused\" })\n";
}

string t_java_deprecated_generator::java_suppress_warnings_consts() {
  return string() + "@SuppressWarnings({ \"unused\" })\n";
}

string t_java_deprecated_generator::java_suppress_warnings_union() {
  return string() +
      "@SuppressWarnings({ \"unused\", \"serial\", \"unchecked\" })\n";
}

string t_java_deprecated_generator::java_suppress_warnings_struct() {
  return string() + "@SuppressWarnings({ \"unused\", \"serial\" })\n";
}

string t_java_deprecated_generator::java_suppress_warnings_service() {
  return string() + "@SuppressWarnings({ \"unused\", \"serial\" })\n";
}

/**
 * Nothing in Java
 */
void t_java_deprecated_generator::close_generator() {}

/**
 * Generates a typedef. This is not done in Java, since it does
 * not support arbitrary name replacements, and it'd be a wacky waste
 * of overhead to make wrapper classes.
 *
 * @param ttypedef The type definition
 */
void t_java_deprecated_generator::generate_typedef(
    const t_typedef* /*ttypedef*/) {}

/**
 * Enums are Java enums with a set of static constants.
 *
 * @param tenum The enumeration
 */
void t_java_deprecated_generator::generate_enum(const t_enum* tenum) {
  // Make output file
  string f_enum_name = get_package_dir() + "/" + (tenum->get_name()) + ".java";
  ofstream f_enum;
  f_enum.open(f_enum_name.c_str());
  record_genfile(f_enum_name);

  // Comment and package it
  f_enum << autogen_comment() << java_package() << endl;

  // Add java imports
  f_enum << string() + "import com.facebook.thrift.IntRangeSet;\n" +
          +"import java.util.Map;\n" + "import java.util.HashMap;\n"
         << endl;

  f_enum << java_suppress_warnings_enum() << "public enum " << tenum->get_name()
         << " implements com.facebook.thrift.TEnum ";
  scope_up(f_enum);

  const auto& enums = tenum->get_enum_values();
  bool first = true;
  for (auto t_enum_value : enums) {
    auto value = t_enum_value->get_value();
    if (!first) {
      f_enum << ",\n";
    }
    generate_java_doc(f_enum, t_enum_value);
    indent(f_enum) << t_enum_value->get_name() << "(" << value << ")";
    first = false;
  }
  f_enum << ";" << endl << endl;
  const uint32_t LARGE_ENUMS_THRESHOLD = 16;

  if (enums.size() > LARGE_ENUMS_THRESHOLD) {
    f_enum << indent() << "private static final Map<Integer, "
           << tenum->get_name() << "> INDEXED_VALUES = new HashMap<Integer, "
           << tenum->get_name() << ">();" << endl
           << endl
           << indent() << "static {" << endl
           << indent() << "  for (" << tenum->get_name() << " e: values()) {"
           << endl
           << indent() << "    INDEXED_VALUES.put(e.getValue(), e);" << endl
           << indent() << "  }" << endl
           << indent() << "}" << endl
           << endl;
  }

  // Field for thriftCode
  indent(f_enum) << "private final int value;" << endl << endl;

  indent(f_enum) << "private " << tenum->get_name() << "(int value) {" << endl;
  indent(f_enum) << "  this.value = value;" << endl;
  indent(f_enum) << "}" << endl << endl;

  indent(f_enum) << "/**" << endl;
  indent(f_enum) << " * Get the integer value of this enum value, as defined "
                    "in the Thrift IDL."
                 << endl;
  indent(f_enum) << " */" << endl;
  indent(f_enum) << "public int getValue() {" << endl;
  indent(f_enum) << "  return value;" << endl;
  indent(f_enum) << "}" << endl << endl;

  indent(f_enum) << "/**" << endl;
  indent(f_enum) << " * Find a the enum type by its integer value, as defined "
                    "in the Thrift IDL."
                 << endl;
  indent(f_enum) << " * @return null if the value is not found." << endl;
  indent(f_enum) << " */" << endl;
  indent(f_enum) << "public static " + tenum->get_name() +
          " findByValue(int value) { "
                 << endl;

  indent_up();

  if (enums.size() > LARGE_ENUMS_THRESHOLD) {
    indent(f_enum) << "return INDEXED_VALUES.get(value);" << endl;
  } else {
    indent(f_enum) << "switch (value) {" << endl;
    indent_up();

    for (auto t_enum_value : enums) {
      auto value = t_enum_value->get_value();
      indent(f_enum) << "case " << value << ":" << endl;
      indent(f_enum) << "  return " << t_enum_value->get_name() << ";" << endl;
    }

    indent(f_enum) << "default:" << endl;
    indent(f_enum) << "  return null;" << endl;

    indent_down();
    indent(f_enum) << "}" << endl;
  }

  indent_down();

  indent(f_enum) << "}" << endl;

  scope_down(f_enum);

  f_enum.close();
}

/**
 * Generates a class that holds all the constants.
 */
void t_java_deprecated_generator::generate_consts(
    std::vector<t_const*> consts) {
  if (consts.empty()) {
    return;
  }

  string f_consts_name = get_package_dir() + "/Constants.java";
  ofstream f_consts;
  f_consts.open(f_consts_name.c_str());
  record_genfile(f_consts_name);

  // Print header
  f_consts << autogen_comment() << java_package() << java_struct_imports();

  f_consts << java_suppress_warnings_consts() << "public class Constants {"
           << endl
           << endl;
  indent_up();
  for (const auto* tconst : consts) {
    print_const_value(
        f_consts, tconst->get_name(), tconst->type(), tconst->value(), false);
  }
  indent_down();
  indent(f_consts) << "}" << endl;
  f_consts.close();
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
void t_java_deprecated_generator::print_const_value(
    std::ostream& out,
    string name,
    const t_type* type,
    const t_const_value* value,
    bool in_static,
    bool defval) {
  type = type->get_true_type();

  indent(out);
  if (!defval) {
    out << (in_static ? "" : "public static final ") << type_name(type) << " ";
  }
  if (type->is_base_type()) {
    string v2 = render_const_value(out, name, type, value);
    out << name << " = " << v2 << ";" << endl << endl;
  } else if (type->is_enum()) {
    out << name << " = " << render_const_value(out, name, type, value) << ";"
        << endl
        << endl;
  } else if (type->is_struct() || type->is_exception()) {
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
    out << name << " = new " << type_name(type, false, true) << "();" << endl;
    if (!in_static) {
      indent(out) << "static {" << endl;
      indent_up();
    }
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
      string val = render_const_value(out, name, field_type, v_iter->second);
      indent(out) << name << ".";
      std::string cap_name = get_cap_name(v_iter->first->get_string());
      out << "set" << cap_name << "(" << val << ");" << endl;
    }
    if (!in_static) {
      indent_down();
      indent(out) << "}" << endl;
    }
    out << endl;
  } else if (type->is_map()) {
    out << name << " = new " << type_name(type, false, true) << "();" << endl;
    if (!in_static) {
      indent(out) << "static {" << endl;
      indent_up();
    }
    const t_type* ktype = ((t_map*)type)->get_key_type();
    const t_type* vtype = ((t_map*)type)->get_val_type();
    const vector<pair<t_const_value*, t_const_value*>>& val = value->get_map();
    vector<pair<t_const_value*, t_const_value*>>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      string key = render_const_value(out, name, ktype, v_iter->first);
      string val = render_const_value(out, name, vtype, v_iter->second);
      indent(out) << name << ".put(" << key << ", " << val << ");" << endl;
    }
    if (!in_static) {
      indent_down();
      indent(out) << "}" << endl;
    }
    out << endl;
  } else if (type->is_list() || type->is_set()) {
    out << name << " = new " << type_name(type, false, true) << "();" << endl;
    if (!in_static) {
      indent(out) << "static {" << endl;
      indent_up();
    }
    const t_type* etype;
    if (type->is_list()) {
      etype = ((t_list*)type)->get_elem_type();
    } else {
      etype = ((t_set*)type)->get_elem_type();
    }
    const vector<t_const_value*>& val = value->get_list();
    vector<t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      string val = render_const_value(out, name, etype, *v_iter);
      indent(out) << name << ".add(" << val << ");" << endl;
    }
    if (!in_static) {
      indent_down();
      indent(out) << "}" << endl;
    }
    out << endl;
  } else {
    throw std::runtime_error(
        "compiler error: no const of type " + type->get_name());
  }
}

string t_java_deprecated_generator::render_const_value(
    ostream& out,
    string /* unused */,
    const t_type* type,
    const t_const_value* value) {
  type = type->get_true_type();
  std::ostringstream render;
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        render << '"';
        for (unsigned char c : value->get_string()) {
          switch (c) {
            case '\\':
              render << "\\\\";
              break;
            case '"':
              render << "\\\"";
              break;
            case '\n':
              render << "\\n";
              break;
            default:
              if (c < 0x20) {
                render << fmt::format("\\x{:02x}", c);
              } else {
                render << c;
              }
              break;
          }
        }
        render << '"';
        if (tbase == t_base_type::TYPE_BINARY) {
          render << ".getBytes()";
        }
        break;
      case t_base_type::TYPE_BOOL:
        render << ((value->get_integer() > 0) ? "true" : "false");
        break;
      case t_base_type::TYPE_BYTE:
        render << "(byte)" << value->get_integer();
        break;
      case t_base_type::TYPE_I16:
        render << "(short)" << value->get_integer();
        break;
      case t_base_type::TYPE_I32:
        render << value->get_integer();
        break;
      case t_base_type::TYPE_I64:
        render << value->get_integer() << "L";
        break;
      case t_base_type::TYPE_DOUBLE:
        if (value->get_type() == t_const_value::CV_INTEGER) {
          render << "(double)" << value->get_integer();
        } else {
          render << value->get_double();
        }
        break;
      case t_base_type::TYPE_FLOAT:
        if (value->get_type() == t_const_value::CV_INTEGER) {
          render << "(float)" << value->get_integer();
        } else {
          render << "(float)" << value->get_double();
        }
        break;
      default:
        throw std::runtime_error(
            "compiler error: no const of base type " +
            t_base_type::t_base_name(tbase));
    }
  } else if (type->is_enum()) {
    std::string namespace_prefix =
        type->program()->get_namespace(namespace_key_);
    if (namespace_prefix.length() > 0) {
      namespace_prefix += ".";
    }
    if (value->get_enum_value() == nullptr) {
      render << namespace_prefix << type->get_name() << ".findByValue("
             << value->get_integer() << ")";
    } else {
      render << namespace_prefix << value->get_enum()->get_name() << "."
             << value->get_enum_value()->get_name();
    }
  } else {
    string t = tmp("tmp");
    print_const_value(out, t, type, value, true);
    render << t;
  }

  return render.str();
}

/**
 * Generates a struct definition for a thrift data type. This will be a TBase
 * implementor.
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_struct(const t_struct* tstruct) {
  if (tstruct->is_union()) {
    generate_java_union(tstruct);
  } else {
    generate_java_struct(tstruct, false);
  }
}

/**
 * Exceptions are structs, but they inherit from Exception
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_xception(const t_struct* txception) {
  generate_java_struct(txception, true);
}

/**
 * Java struct definition.
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_struct(
    const t_struct* tstruct, bool is_exception) {
  // Make output file
  string f_struct_name =
      get_package_dir() + "/" + (tstruct->get_name()) + ".java";
  ofstream f_struct;
  f_struct.open(f_struct_name.c_str());
  record_genfile(f_struct_name);

  f_struct << autogen_comment() << java_package() << java_struct_imports()
           << java_thrift_imports();

  StructGenParams params;
  params.is_exception = is_exception;
  params.gen_immutable = generate_immutable_structs_;
  params.gen_builder = generate_builder;

  generate_java_struct_definition(f_struct, tstruct, params);
  f_struct.close();
}

/**
 * Java union definition.
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_union(const t_struct* tstruct) {
  // Make output file
  string f_struct_name =
      get_package_dir() + "/" + (tstruct->get_name()) + ".java";
  ofstream f_struct;
  f_struct.open(f_struct_name.c_str());
  record_genfile(f_struct_name);

  f_struct << autogen_comment() << java_package() << java_struct_imports()
           << java_thrift_imports();

  generate_java_doc(f_struct, tstruct);

  bool is_final = tstruct->has_annotation("final");

  indent(f_struct) << java_suppress_warnings_union() << "public "
                   << (is_final ? "final " : "") << "class "
                   << tstruct->get_name()
                   << " extends TUnion<" + tstruct->get_name() + "> ";

  if (is_comparable(tstruct)) {
    f_struct << "implements Comparable<" << type_name(tstruct) << "> ";
  }

  scope_up(f_struct);

  generate_struct_desc(f_struct, tstruct);
  generate_field_descs(f_struct, tstruct);

  f_struct << endl;

  generate_field_name_constants(f_struct, tstruct);

  f_struct << endl;

  if (generate_field_metadata_) {
    generate_java_meta_data_map(f_struct, tstruct);
  } else {
    indent(f_struct) << "public static final Map<Integer, FieldMetaData> "
                        "metaDataMap = new HashMap<>();"
                     << endl
                     << endl;
  }

  generate_union_constructor(f_struct, tstruct);

  f_struct << endl;

  generate_union_abstract_methods(f_struct, tstruct);

  f_struct << endl;

  generate_union_getters_and_setters(f_struct, tstruct);

  f_struct << endl;

  generate_union_comparisons(f_struct, tstruct);

  f_struct << endl;

  generate_union_hashcode(f_struct, tstruct);

  f_struct << endl;

  scope_down(f_struct);

  f_struct.close();
}

void t_java_deprecated_generator::generate_union_constructor(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "public " << type_name(tstruct) << "() {" << endl;
  indent(out) << "  super();" << endl;
  indent(out) << "}" << endl << endl;

  indent(out) << "public " << type_name(tstruct)
              << "(int setField, Object __value) {" << endl;
  indent(out) << "  super(setField, __value);" << endl;
  indent(out) << "}" << endl << endl;

  indent(out) << "public " << type_name(tstruct) << "(" << type_name(tstruct)
              << " other) {" << endl;
  indent(out) << "  super(other);" << endl;
  indent(out) << "}" << endl << endl;

  indent(out) << "public " << tstruct->get_name() << " deepCopy() {" << endl;
  indent(out) << "  return new " << tstruct->get_name() << "(this);" << endl;
  indent(out) << "}" << endl << endl;

  // generate "constructors" for each field
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    indent(out) << "public static " << type_name(tstruct) << " "
                << (*m_iter)->get_name() << "("
                << type_name((*m_iter)->get_type()) << " __value) {" << endl;
    indent(out) << "  " << type_name(tstruct) << " x = new "
                << type_name(tstruct) << "();" << endl;
    indent(out) << "  x.set" << get_cap_name((*m_iter)->get_name())
                << "(__value);" << endl;
    indent(out) << "  return x;" << endl;
    indent(out) << "}" << endl << endl;
  }
}

void t_java_deprecated_generator::generate_union_getters_and_setters(
    ofstream& out, const t_struct* tstruct) {
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  indent(out) << "private Object __getValue(int expectedFieldId) {" << endl;
  indent(out) << "  if (getSetField() == expectedFieldId) {" << endl;
  indent(out) << "    return getFieldValue();" << endl;
  indent(out) << "  } else {" << endl;
  indent(out) << "    throw new RuntimeException(\"Cannot get field '\" + "
                 "getFieldDesc(expectedFieldId).name + \""
              << "' because union is currently set to \" + "
                 "getFieldDesc(getSetField()).name);"
              << endl;
  indent(out) << "  }" << endl;
  indent(out) << "}" << endl << endl;

  indent(out) << "private void __setValue(int fieldId, Object __value) {"
              << endl;
  indent(out) << "  if (__value == null) throw new NullPointerException();"
              << endl;
  indent(out) << "  setField_ = fieldId;" << endl;
  indent(out) << "  value_ = __value;" << endl;
  indent(out) << "}" << endl << endl;

  bool first = true;
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    if (first) {
      first = false;
    } else {
      out << endl;
    }

    const t_field* field = (*m_iter);
    generate_java_doc(out, field);
    if (is_field_sensitive(field)) {
      indent(out) << "@Sensitive" << endl;
    }
    indent(out) << "public " << type_name(field->get_type()) << " "
                << get_simple_getter_name(field) << "() {" << endl;
    indent(out) << "  return (" << type_name(field->get_type(), true)
                << ") __getValue(" << upcase_string(field->get_name()) << ");"
                << endl;
    indent(out) << "}" << endl;

    out << endl;

    generate_java_doc(out, field);
    indent(out) << "public void set" << get_cap_name(field->get_name()) << "("
                << type_name(field->get_type()) << " __value) {" << endl;
    if (type_can_be_null(field->get_type())) {
      indent(out) << "  __setValue(" << upcase_string(field->get_name())
                  << ", __value);" << endl;
    } else {
      indent(out) << "  setField_ = " << upcase_string(field->get_name()) << ";"
                  << endl;
      indent(out) << "  value_ = __value;" << endl;
    }
    indent(out) << "}" << endl;
  }
}

void t_java_deprecated_generator::generate_union_abstract_methods(
    ofstream& out, const t_struct* tstruct) {
  if (!generate_immutable_structs_) {
    // immutable structs use generic `checkType` (defined in parent class
    // `TUnion`)
    generate_check_type(out, tstruct);
    out << endl;
    generate_union_reader(out, tstruct);
    out << endl;
  }

  generate_read_value(out, tstruct);
  out << endl;
  generate_write_value(out, tstruct);
  out << endl;
  generate_get_field_desc(out, tstruct);
  out << endl;
  generate_get_struct_desc(out, tstruct);
  out << endl;

  indent(out) << "@Override" << endl;
  indent(out) << "protected Map<Integer, FieldMetaData> getMetaDataMap() { "
                 "return metaDataMap; }"
              << endl;
}

void t_java_deprecated_generator::generate_check_type(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "@Override" << endl;
  indent(out) << "protected void checkType(short setField, Object __value) "
                 "throws ClassCastException {"
              << endl;
  indent_up();

  indent(out) << "switch (setField) {" << endl;
  indent_up();

  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_field* field = (*m_iter);

    indent(out) << "case " << upcase_string(field->get_name()) << ":" << endl;
    indent(out) << "  if (__value instanceof "
                << type_name(field->get_type(), true, false, true) << ") {"
                << endl;
    indent(out) << "    break;" << endl;
    indent(out) << "  }" << endl;
    indent(out)
        << "  throw new ClassCastException(\"Was expecting value of type "
        << type_name(field->get_type(), true, false) << " for field '"
        << field->get_name()
        << "', but got \" + __value.getClass().getSimpleName());" << endl;
    // do the real check here
  }

  indent(out) << "default:" << endl;
  indent(out) << "  throw new IllegalArgumentException(\"Unknown field id \" + "
                 "setField);"
              << endl;

  indent_down();
  indent(out) << "}" << endl;

  indent_down();
  indent(out) << "}" << endl;
}

/**
 * Generates a function to read a union.
 *
 * @param out The output stream
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_union_reader(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "@Override" << endl;
  indent(out) << "public void read(TProtocol iprot) throws TException {"
              << endl;
  indent_up();

  indent(out) << "setField_ = 0;" << endl;
  indent(out) << "value_ = null;" << endl;

  // metaDataMap must be passed to iprot.readStructBegin() for
  // deserialization to work.
  indent(out) << "iprot.readStructBegin("
              << (generate_field_metadata_ ? "metaDataMap" : "") << ");"
              << endl;

  indent(out) << "TField __field = iprot.readFieldBegin();" << endl;

  indent(out) << "if (__field.type != TType.STOP)" << endl;
  scope_up(out);

  indent(out) << "value_ = readValue(iprot, __field);" << endl;
  indent(out) << "if (value_ != null)" << endl;
  scope_up(out);

  indent(out) << "switch (__field.id) {" << endl;
  indent_up();

  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_field* field = (*m_iter);
    indent(out) << "case " << upcase_string(field->get_name()) << ":" << endl;
    indent_up();
    indent(out) << "if (__field.type == " << constant_name(field->get_name())
                << "_FIELD_DESC.type) {" << endl;
    indent_up();
    indent(out) << "setField_ = __field.id;" << endl;
    indent_down();
    indent(out) << "}" << endl;
    indent(out) << "break;" << endl;
    indent_down();
  }

  indent_down();
  indent(out) << "}" << endl;
  indent_down();
  indent(out) << "}" << endl;
  indent(out) << "iprot.readFieldEnd();" << endl;
  // Eat stop byte
  indent(out) << "TField __stopField = iprot.readFieldBegin();" << endl;
  indent(out) << "if (__stopField.type != TType.STOP) {" << endl;
  indent(out)
      << "  throw new TProtocolException(TProtocolException.INVALID_DATA, "
         "\"Union '" +
          tstruct->get_name() + "' is missing a STOP byte\");"
      << endl;
  indent(out) << "}" << endl;
  indent_down();
  indent(out) << "}" << endl;

  indent(out) << "iprot.readStructEnd();" << endl;

  indent_down();
  indent(out) << "}" << endl;
}

void t_java_deprecated_generator::generate_read_value(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "@Override" << endl;
  indent(out) << "protected Object readValue(TProtocol iprot, TField __field) "
                 "throws TException {"
              << endl;

  indent_up();

  indent(out) << "switch (__field.id) {" << endl;
  indent_up();

  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_field* field = (*m_iter);

    indent(out) << "case " << upcase_string(field->get_name()) << ":" << endl;
    indent_up();
    indent(out) << "if (__field.type == " << constant_name(field->get_name())
                << "_FIELD_DESC.type) {" << endl;
    indent_up();
    indent(out) << type_name(field->get_type(), true, false) << " "
                << field->get_name() << ";" << endl;
    generate_deserialize_field(out, field, "");
    indent(out) << "return " << field->get_name() << ";" << endl;
    indent_down();
    indent(out) << "}" << endl;
    indent(out) << "break;" << endl;
    indent_down();
  }

  indent_down();
  indent(out) << "}" << endl;
  indent_down();

  indent(out) << "  TProtocolUtil.skip(iprot, __field.type);" << endl;
  indent(out) << "  return null;" << endl;
  indent(out) << "}" << endl;
}

void t_java_deprecated_generator::generate_write_value(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "@Override" << endl;
  indent(out) << "protected void writeValue(TProtocol oprot, short setField, "
                 "Object __value) throws TException {"
              << endl;

  indent_up();

  indent(out) << "switch (setField) {" << endl;
  indent_up();

  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_field* field = (*m_iter);

    indent(out) << "case " << upcase_string(field->get_name()) << ":" << endl;
    indent_up();
    indent(out) << type_name(field->get_type(), true, false) << " "
                << field->get_name() << " = ("
                << type_name(field->get_type(), true, false)
                << ")getFieldValue();" << endl;
    generate_serialize_field(out, field, "");
    indent(out) << "return;" << endl;
    indent_down();
  }

  indent(out) << "default:" << endl;
  indent(out) << "  throw new IllegalStateException(\"Cannot write union with "
                 "unknown field \" + setField);"
              << endl;

  indent_down();
  indent(out) << "}" << endl;

  indent_down();

  indent(out) << "}" << endl;
}

void t_java_deprecated_generator::generate_get_field_desc(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "@Override" << endl;
  indent(out) << "protected TField getFieldDesc(int setField) {" << endl;
  indent_up();

  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  indent(out) << "switch (setField) {" << endl;
  indent_up();

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_field* field = (*m_iter);
    indent(out) << "case " << upcase_string(field->get_name()) << ":" << endl;
    indent(out) << "  return " << constant_name(field->get_name())
                << "_FIELD_DESC;" << endl;
  }

  indent(out) << "default:" << endl;
  indent(out) << "  throw new IllegalArgumentException(\"Unknown field id \" + "
                 "setField);"
              << endl;

  indent_down();
  indent(out) << "}" << endl;

  indent_down();
  indent(out) << "}" << endl;
}

void t_java_deprecated_generator::generate_get_struct_desc(
    ofstream& out, const t_struct* /*tstruct*/) {
  indent(out) << "@Override" << endl;
  indent(out) << "protected TStruct getStructDesc() {" << endl;
  indent(out) << "  return STRUCT_DESC;" << endl;
  indent(out) << "}" << endl;
}

void t_java_deprecated_generator::generate_union_comparisons(
    ofstream& out, const t_struct* tstruct) {
  // equality
  indent(out) << "public boolean equals(Object other) {" << endl;
  indent(out) << "  if (other instanceof " << tstruct->get_name() << ") {"
              << endl;
  indent(out) << "    return equals((" << tstruct->get_name() << ")other);"
              << endl;
  indent(out) << "  } else {" << endl;
  indent(out) << "    return false;" << endl;
  indent(out) << "  }" << endl;
  indent(out) << "}" << endl;

  out << endl;

  indent(out) << "public boolean equals(" << tstruct->get_name() << " other) {"
              << endl;
  if (struct_has_naked_binary_fields(tstruct)) {
    indent(out) << "  return equalsSlowImpl(other);" << endl;
  } else {
    indent(out) << "  return equalsNobinaryImpl(other);" << endl;
  }
  indent(out) << "}" << endl;
  out << endl;

  if (is_comparable(tstruct)) {
    indent(out) << "@Override" << endl;
    indent(out) << "public int compareTo(" << type_name(tstruct) << " other) {"
                << endl;
    indent(out) << "  return compareToImpl(other);" << endl;
    indent(out) << "}" << endl;
    out << endl;
  }
}

void t_java_deprecated_generator::generate_union_hashcode(
    ofstream& out, const t_struct* /*tstruct*/) {
  indent(out) << "@Override" << endl;
  indent(out) << "public int hashCode() {" << endl;
  indent(out) << "  return Arrays.deepHashCode(new Object[] {"
              << "getSetField(), getFieldValue()});" << endl;
  indent(out) << "}" << endl;
}

/**
 * Generates code for a constructor for a tstruct, given the fields that
 * the constructor should take as parameters to initialize the struct.
 */
void t_java_deprecated_generator::generate_java_constructor(
    ofstream& out, const t_struct* tstruct, const vector<t_field*>& fields) {
  vector<t_field*>::const_iterator m_iter;
  indent(out) << "public " << tstruct->get_name() << "(";
  if (!fields.empty()) {
    out << endl;
    indent_up();
    indent_up();
    for (m_iter = fields.begin(); m_iter != fields.end();) {
      indent(out) << type_name((*m_iter)->get_type()) << " "
                  << (*m_iter)->get_name();
      ++m_iter;
      if (m_iter != fields.end()) {
        out << "," << endl;
      }
    }
    indent_down();
    indent_down();
  }
  out << ") {" << endl;
  indent_up();
  if (generate_immutable_structs_) {
    if (fields.size() != tstruct->get_members().size()) {
      construct_constant_fields(out, tstruct);
    }
  } else {
    indent(out) << "this();" << endl;
  }
  for (m_iter = fields.begin(); m_iter != fields.end(); ++m_iter) {
    indent(out) << "this." << (*m_iter)->get_name() << " = "
                << (*m_iter)->get_name() << ";" << endl;
    if (!generate_immutable_structs_) {
      generate_isset_set(out, (*m_iter));
    }
  }
  indent_down();
  indent(out) << "}" << endl << endl;
}

/**
 * Generates code for a constructor for a tstruct, using the Builder pattern.
 */
void t_java_deprecated_generator::generate_java_constructor_using_builder(
    ofstream& out,
    const t_struct* tstruct,
    const vector<t_field*>& fields,
    uint32_t bitset_size,
    bool useDefaultConstructor) {
  vector<t_field*>::const_iterator m_iter;

  // BEGIN Create inner Builder class
  indent(out) << "public static class Builder {" << endl;
  indent_up();
  // - List builder fields
  for (m_iter = fields.begin(); m_iter != fields.end(); ++m_iter) {
    indent(out) << "private " << type_name((*m_iter)->get_type()) << " "
                << (*m_iter)->get_name() << ";" << endl;
  }
  out << endl;

  if (bitset_size > 0) {
    indent(out) << "BitSet __optional_isset = new BitSet(" << bitset_size
                << ");" << endl
                << endl;
  }

  // - Define Builder constructor
  indent(out) << "public Builder() {" << endl;
  if (!useDefaultConstructor) {
    for (m_iter = fields.begin(); m_iter != fields.end(); ++m_iter) {
      const t_type* t = (*m_iter)->get_type()->get_true_type();
      if ((*m_iter)->get_value() != nullptr) {
        print_const_value(
            indent(out),
            "this." + (*m_iter)->get_name(),
            t,
            (*m_iter)->get_value(),
            true,
            true);
      }
    }
  }
  indent(out) << "}" << endl;
  out << endl;

  // - Define field builder methods
  for (m_iter = fields.begin(); m_iter != fields.end(); ++m_iter) {
    auto methodName = (*m_iter)->get_name();
    methodName[0] = toupper(methodName[0]);
    indent(out) << "public Builder set" << methodName << "(final "
                << type_name((*m_iter)->get_type()) << " "
                << (*m_iter)->get_name() << ") {" << endl;
    indent_up();
    indent(out) << "this." << (*m_iter)->get_name() << " = "
                << (*m_iter)->get_name() << ";" << endl;
    if (!type_can_be_null((*m_iter)->get_type())) {
      indent(out) << "__optional_isset.set(" << isset_field_id(*m_iter)
                  << ", true);" << endl;
    }
    indent(out) << "return this;" << endl;
    indent_down();
    indent(out) << "}" << endl << endl;
  }
  // - Define build() method
  indent(out) << "public " << tstruct->get_name() << " build() {" << endl;
  indent_up();
  if (useDefaultConstructor) {
    indent(out) << tstruct->get_name() << " result = new "
                << tstruct->get_name() << "();" << endl;

    for (m_iter = fields.begin(); m_iter != fields.end(); ++m_iter) {
      auto methodName = (*m_iter)->get_name();
      methodName[0] = toupper(methodName[0]);
      if (!type_can_be_null((*m_iter)->get_type())) {
        // if field is set (check the bitset), set the field value
        indent(out) << "if (__optional_isset.get(" << isset_field_id(*m_iter)
                    << ")) {" << endl;
        indent_up();
      }
      indent(out) << "result.set" << methodName << "(this."
                  << (*m_iter)->get_name() << ");" << endl;
      if (!type_can_be_null((*m_iter)->get_type())) {
        indent_down();
        indent(out) << "}" << endl;
      }
    }
    indent(out) << "return result;" << endl;
  } else {
    // android

    indent(out) << "return new " << tstruct->get_name() << "(" << endl;
    indent_up();
    for (m_iter = fields.begin(); m_iter != fields.end(); ++m_iter) {
      auto isLast = m_iter + 1 == fields.end();
      indent(out) << "this." << (*m_iter)->get_name() << (isLast ? "" : ",")
                  << endl;
    }
    indent_down();
    indent(out) << ");" << endl;
  }
  indent_down();
  indent(out) << "}" << endl;
  // END Create inner Builder class
  indent_down();
  indent(out) << "}" << endl << endl;

  // Create static builder() method for ease of use
  indent(out) << "public static Builder builder() {" << endl;
  indent_up();
  indent(out) << "return new Builder();" << endl;
  indent_down();
  indent(out) << "}" << endl << endl;
}

/**
 * Java struct definition. This has various parameters, as it could be
 * generated standalone or inside another class as a helper. If it
 * is a helper than it is a static class.
 *
 * @param tstruct      The struct definition
 * @param is_exception Is this an exception?
 * @param in_class     If inside a class, needs to be static class
 * @param is_result    If this is a result it needs a different writer
 */
void t_java_deprecated_generator::generate_java_struct_definition(
    ofstream& out, const t_struct* tstruct, StructGenParams params) {
  generate_java_doc(out, tstruct);

  bool is_final = tstruct->has_annotation("final");

  indent(out) << (params.in_class ? string() : java_suppress_warnings_struct())
              << "public " << (is_final ? "final " : "")
              << (params.in_class ? "static " : "") << "class "
              << tstruct->get_name() << " ";

  boost::optional<string> parent = java_struct_parent_class(tstruct, params);
  if (parent) {
    out << "extends " << *parent << " ";
  }

  out << "implements TBase, java.io.Serializable, Cloneable";

  if (is_comparable(tstruct)) {
    out << ", Comparable<" << type_name(tstruct) << ">";
  }

  out << " ";

  scope_up(out);

  generate_struct_desc(out, tstruct);

  // Members are public for -java, private for -javabean
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  generate_field_descs(out, tstruct);

  out << endl;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    generate_java_doc(out, *m_iter);
    indent(out) << "public ";
    if (params.gen_immutable) {
      out << "final ";
    }
    out << declare_field(*m_iter, false) << endl;
  }
  generate_field_name_constants(out, tstruct);

  uint32_t bitset_size = 0;
  if (!params.gen_immutable) {
    // isset data
    if (members.size() > 0) {
      out << endl;

      indent(out) << "// isset id assignments" << endl;

      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        if (!type_can_be_null((*m_iter)->get_type())) {
          indent(out) << "private static final int " << isset_field_id(*m_iter)
                      << " = " << bitset_size << ";" << endl;
          bitset_size++;
        }
      }

      if (bitset_size > 0) {
        indent(out) << "private BitSet __isset_bit_vector = new BitSet("
                    << bitset_size << ");" << endl;
      }

      out << endl;
    }

    if (generate_field_metadata_) {
      generate_java_meta_data_map(out, tstruct);

      // Static initializer to populate global class to struct metadata map
      indent(out) << "static {" << endl;
      indent_up();
      indent(out) << "FieldMetaData.addStructMetaDataMap(" << type_name(tstruct)
                  << ".class, metaDataMap);" << endl;
      indent_down();
      indent(out) << "}" << endl;
    }
  }
  out << endl;

  vector<t_field*> required_members;
  vector<t_field*> non_optional_members;
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    if ((*m_iter)->get_req() == t_field::e_req::required) {
      required_members.push_back(*m_iter);
    }
    if ((*m_iter)->get_req() != t_field::e_req::optional) {
      non_optional_members.push_back(*m_iter);
    }
  }

  if (params.gen_immutable &&
      members.size() <= MAX_NUM_JAVA_CONSTRUCTOR_PARAMS) {
    // Constructor for all fields
    generate_java_constructor(out, tstruct, members);
  } else {
    // Default constructor
    indent(out) << "public " << tstruct->get_name() << "() {" << endl;
    indent_up();
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      const t_type* t = (*m_iter)->get_type()->get_true_type();
      if ((*m_iter)->get_value() != nullptr) {
        print_const_value(
            out,
            "this." + (*m_iter)->get_name(),
            t,
            (*m_iter)->get_value(),
            true,
            true);
      }
    }
    indent_down();
    indent(out) << "}" << endl << endl;

    if (!required_members.empty() &&
        required_members.size() <= MAX_NUM_JAVA_CONSTRUCTOR_PARAMS) {
      // Constructor for all required fields
      generate_java_constructor(out, tstruct, required_members);
    }

    if (non_optional_members.size() > required_members.size() &&
        non_optional_members.size() <= MAX_NUM_JAVA_CONSTRUCTOR_PARAMS) {
      // Constructor for all non-optional fields
      generate_java_constructor(out, tstruct, non_optional_members);
    }

    if (members.size() > non_optional_members.size() &&
        members.size() <= MAX_NUM_JAVA_CONSTRUCTOR_PARAMS) {
      // Constructor for all fields
      generate_java_constructor(out, tstruct, members);
    }
  }

  auto forceBuilderGeneration =
      tstruct->has_annotation("android.generate_builder");

  if (forceBuilderGeneration || params.gen_builder ||
      members.size() > MAX_NUM_JAVA_CONSTRUCTOR_PARAMS) {
    generate_java_constructor_using_builder(
        out, tstruct, members, bitset_size, !params.gen_immutable);
  }

  // copy constructor
  indent(out) << "/**" << endl;
  indent(out) << " * Performs a deep copy on <i>other</i>." << endl;
  indent(out) << " */" << endl;
  indent(out) << "public " << tstruct->get_name() << "(" << tstruct->get_name()
              << " other) {" << endl;
  indent_up();

  if (!params.gen_immutable && has_bit_vector(tstruct)) {
    indent(out) << "__isset_bit_vector.clear();" << endl;
    indent(out) << "__isset_bit_vector.or(other.__isset_bit_vector);" << endl;
  }

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_field* field = (*m_iter);
    std::string field_name = field->get_name();
    const t_type* type = field->get_type();

    bool can_be_null = type_can_be_null(type);

    if (can_be_null) {
      indent(out) << "if (other." << generate_isset_check(field) << ") {"
                  << endl;
      indent_up();
    }

    indent(out) << "this." << field_name << " = TBaseHelper.deepCopy(other."
                << field_name << ");" << endl;

    if (can_be_null) {
      indent_down();
      if (params.gen_immutable) {
        indent(out) << "} else {" << endl;
        indent(out) << "  this." << field_name << " = null;" << endl;
      }
      indent(out) << "}" << endl;
    }
  }

  indent_down();
  indent(out) << "}" << endl << endl;

  // clone method, so that you can deep copy an object when you don't know its
  // class.
  indent(out) << "public " << tstruct->get_name() << " deepCopy() {" << endl;
  indent(out) << "  return new " << tstruct->get_name() << "(this);" << endl;
  indent(out) << "}" << endl << endl;

  generate_java_bean_boilerplate(out, tstruct, params.gen_immutable);

  generate_generic_field_getters_setters(out, tstruct);

  generate_java_struct_equality(out, tstruct);
  if (is_comparable(tstruct)) {
    generate_java_struct_compare_to(out, tstruct);
  }

  generate_java_struct_reader(out, tstruct);
  if (params.is_result) {
    generate_java_struct_result_writer(out, tstruct);
  } else {
    generate_java_struct_writer(out, tstruct);
  }
  generate_java_struct_tostring(out, tstruct);
  generate_java_validator(out, tstruct);
  scope_down(out);
  out << endl;
}

void t_java_deprecated_generator::construct_constant_fields(
    ofstream& out, const t_struct* tstruct) {
  auto& members = tstruct->get_members();
  for (auto m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_type* t = (*m_iter)->get_type()->get_true_type();
    if ((*m_iter)->get_value() != nullptr) {
      print_const_value(
          out,
          "this." + (*m_iter)->get_name(),
          t,
          (*m_iter)->get_value(),
          true,
          true);
    }
  }
}

/**
 * Generates equals methods and a hashCode method for a structure.
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_struct_equality(
    ofstream& out, const t_struct* tstruct) {
  out << indent() << "@Override" << endl
      << indent() << "public boolean equals(Object _that) {" << endl;
  indent_up();
  out << indent() << "if (_that == null)" << endl
      << indent() << "  return false;" << endl;

  out << indent() << "if (this == _that)" << endl
      << // to make up for an otherwise
      indent() << "  return true;" << endl; // slow equals and a bad hashCode

  out << indent() << "if (!(_that instanceof " << tstruct->get_name() << "))"
      << endl
      << indent() << "  return false;" << endl;

  out << indent() << tstruct->get_name() << " that = (" << tstruct->get_name()
      << ")_that;" << endl;
  const vector<t_field*>& members = tstruct->get_members();
  for (auto m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    out << endl;

    const t_type* t = (*m_iter)->get_type()->get_true_type();
    // Most existing Thrift code does not use isset or optional/required,
    // so we treat "default" fields as required.
    bool is_optional = (*m_iter)->get_req() == t_field::e_req::optional;
    bool can_be_null = type_can_be_null(t);
    string name = (*m_iter)->get_name();
    string equalMethodName = "equalsNobinary";
    if (type_has_naked_binary(t)) {
      equalMethodName = "equalsSlow";
    }

    if (is_optional || can_be_null) {
      out << indent() << "if (!TBaseHelper." << equalMethodName << "("
          << "this." + generate_isset_check(*m_iter) << ", "
          << "that." + generate_isset_check(*m_iter) << ", this." << name
          << ", that." << name << ")) { return false; }" << endl;
    } else {
      out << indent() << "if (!TBaseHelper." << equalMethodName << "(this."
          << name << ", that." << name << ")) { return false; }" << endl;
    }
  }
  out << endl;
  indent(out) << "return true;" << endl;
  scope_down(out);
  out << endl;

  out << indent() << "@Override" << endl
      << indent() << "public int hashCode() {" << endl;
  indent_up();
  indent(out) << "return Arrays.deepHashCode(new Object[] {";
  bool first = true;
  for (auto m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    if (!first) {
      out << ", ";
    }
    out << (*m_iter)->get_name();
    first = false;
  }
  out << "});" << endl;

  indent_down();
  indent(out) << "}" << endl << endl;
}

void t_java_deprecated_generator::generate_java_struct_compare_to(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "@Override" << endl;
  indent(out) << "public int compareTo(" << type_name(tstruct) << " other) {"
              << endl;
  indent_up();

  indent(out) << "if (other == null) {" << endl;
  indent(out) << "  // See java.lang.Comparable docs" << endl;
  indent(out) << "  throw new NullPointerException();" << endl;
  indent(out) << "}" << endl;
  out << endl;

  indent(out) << "if (other == this) {" << endl;
  indent(out) << "  return 0;" << endl;
  indent(out) << "}" << endl;

  indent(out) << "int lastComparison = 0;" << endl;
  out << endl;

  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    const t_field* field = *m_iter;
    indent(out) << "lastComparison = Boolean.valueOf("
                << generate_isset_check(field) << ").compareTo(other."
                << generate_isset_check(field) << ");" << endl;
    indent(out) << "if (lastComparison != 0) {" << endl;
    indent(out) << "  return lastComparison;" << endl;
    indent(out) << "}" << endl;

    indent(out) << "lastComparison = TBaseHelper.compareTo("
                << field->get_name() << ", other." << field->get_name() << ");"
                << endl;
    indent(out) << "if (lastComparison != 0) { " << endl;
    indent(out) << "  return lastComparison;" << endl;
    indent(out) << "}" << endl;
  }

  indent(out) << "return 0;" << endl;

  indent_down();
  indent(out) << "}" << endl << endl;
}

/**
 * Generates a function to read all the fields of the struct.
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_struct_reader(
    ofstream& out, const t_struct* tstruct) {
  if (generate_immutable_structs_) {
    out << indent()
        << "// This is required to satisfy the TBase interface, but can't be "
           "implemented on immutable struture."
        << endl;
    out << indent() << "public void read(TProtocol iprot) throws TException {"
        << endl;
    out << indent()
        << "  throw new TException(\"unimplemented in android immutable "
           "structure\");"
        << endl;
    out << indent() << "}" << endl << endl;

    out << indent() << "public static " << tstruct->get_name()
        << " deserialize(TProtocol iprot) throws TException {" << endl;
    indent_up();
  } else {
    out << indent() << "public void read(TProtocol iprot) throws TException {"
        << endl;
    indent_up();
  }

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  if (generate_immutable_structs_) {
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      string tmp_name = "tmp_" + (*f_iter)->get_name();
      const t_type* field_type = (*f_iter)->get_type();
      indent(out) << type_name(field_type) << " " << tmp_name;
      if (type_can_be_null(field_type)) {
        out << " = null";
      } else if (
          field_type->is_any_int() || field_type->is_double() ||
          field_type->is_float()) {
        out << " = 0";
      } else if (field_type->is_bool()) {
        out << " = false";
      }
      out << ";" << endl;
    }
  }

  // Declare stack tmp variables and read struct header
  out << indent() << "TField __field;" << endl
      << indent() << "iprot.readStructBegin("
      << (generate_field_metadata_ ? "metaDataMap" : "") << ");" << endl;

  // Loop over reading in fields
  indent(out) << "while (true)" << endl;
  scope_up(out);

  // Read beginning field marker
  indent(out) << "__field = iprot.readFieldBegin();" << endl;

  // Check for field STOP marker and break
  indent(out) << "if (__field.type == TType.STOP) {" << endl;
  indent_up();
  indent(out) << "break;" << endl;
  indent_down();
  indent(out) << "}" << endl;

  // Switch statement on the field we are reading
  indent(out) << "switch (__field.id)" << endl;

  scope_up(out);

  // Generate deserialization code for known cases
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    indent(out) << "case " << upcase_string((*f_iter)->get_name()) << ":"
                << endl;
    indent_up();
    indent(out) << "if (__field.type == " << type_to_enum((*f_iter)->get_type())
                << ") {" << endl;
    indent_up();

    if (generate_immutable_structs_) {
      generate_deserialize_field(out, *f_iter, "tmp_");
    } else {
      generate_deserialize_field(out, *f_iter, "this.");
      generate_isset_set(out, *f_iter);
    }
    indent_down();
    out << indent() << "} else {" << endl
        << indent() << "  TProtocolUtil.skip(iprot, __field.type);" << endl
        << indent() << "}" << endl
        << indent() << "break;" << endl;
    indent_down();
  }

  // In the default case we skip the field
  out << indent() << "default:" << endl
      << indent() << "  TProtocolUtil.skip(iprot, __field.type);" << endl
      << indent() << "  break;" << endl;

  scope_down(out);

  // Read field end marker
  indent(out) << "iprot.readFieldEnd();" << endl;

  scope_down(out);

  out << indent() << "iprot.readStructEnd();" << endl << endl;

  if (generate_immutable_structs_) {
    indent(out) << tstruct->get_name() << " _that;" << endl;
    indent(out) << "_that = new " << tstruct->get_name() << "(" << endl;
    bool first = true;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      string tmp_name = "tmp_" + (*f_iter)->get_name();
      if (first) {
        indent(out) << "  " << tmp_name << endl;
      } else {
        indent(out) << "  ," << tmp_name << endl;
      }
      first = false;
    }
    indent(out) << ");" << endl;
    indent(out) << "_that.validate();" << endl;
    indent(out) << "return _that;" << endl;
  } else {
    // Check for required fields of primitive type
    // (which can be checked here but not in the general validate method)
    out << endl
        << indent()
        << "// check for required fields of primitive type, which can't be "
           "checked in the validate method"
        << endl;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      if ((*f_iter)->get_req() == t_field::e_req::required &&
          !type_can_be_null((*f_iter)->get_type())) {
        out << indent() << "if (!" << generate_isset_check(*f_iter) << ") {"
            << endl
            << indent() << "  throw new TProtocolException(\"Required field '"
            << (*f_iter)->get_name()
            << "' was not found in serialized data! Struct: \" + toString());"
            << endl
            << indent() << "}" << endl;
      }
    }

    // performs various checks (e.g. check that all required fields are set)
    indent(out) << "validate();" << endl;
  }

  indent_down();
  out << indent() << "}" << endl << endl;
}

// generates java method to perform various checks
// (e.g. check that all required fields are set)
void t_java_deprecated_generator::generate_java_validator(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "public void validate() throws TException {" << endl;
  indent_up();

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  out << indent() << "// check for required fields" << endl;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if ((*f_iter)->get_req() == t_field::e_req::required) {
      if (type_can_be_null((*f_iter)->get_type())) {
        indent(out) << "if (" << (*f_iter)->get_name() << " == null) {" << endl;
        indent(out) << "  throw new TProtocolException("
                    << "TProtocolException.MISSING_REQUIRED_FIELD, "
                    << "\"Required field '" << (*f_iter)->get_name()
                    << "' was not present! Struct: \" + toString());" << endl;
        indent(out) << "}" << endl;
      } else {
        indent(out) << "// alas, we cannot check '" << (*f_iter)->get_name()
                    << "' because it's a primitive and you chose the non-beans "
                    << "generator." << endl;
      }
    }
  }

  indent_down();
  indent(out) << "}" << endl << endl;
}

/**
 * Generates a function to write all the fields of the struct
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_struct_writer(
    ofstream& out, const t_struct* tstruct) {
  out << indent() << "public void write(TProtocol oprot) throws TException {"
      << endl;
  indent_up();

  const vector<t_field*>& fields = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator f_iter;

  // performs various checks (e.g. check that all required fields are set)
  indent(out) << "validate();" << endl << endl;

  indent(out) << "oprot.writeStructBegin(STRUCT_DESC);" << endl;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    bool null_allowed = type_can_be_null((*f_iter)->get_type());
    if (null_allowed) {
      out << indent() << "if (this." << (*f_iter)->get_name() << " != null) {"
          << endl;
      indent_up();
    }
    bool optional = (*f_iter)->get_req() == t_field::e_req::optional;
    if (optional) {
      indent(out) << "if (" << generate_isset_check((*f_iter)) << ") {" << endl;
      indent_up();
    }

    indent(out) << "oprot.writeFieldBegin("
                << constant_name((*f_iter)->get_name()) << "_FIELD_DESC);"
                << endl;

    // Write field contents
    generate_serialize_field(out, *f_iter, "this.");

    // Write field closer
    indent(out) << "oprot.writeFieldEnd();" << endl;

    if (optional) {
      indent_down();
      indent(out) << "}" << endl;
    }
    if (null_allowed) {
      indent_down();
      indent(out) << "}" << endl;
    }
  }
  // Write the struct map
  out << indent() << "oprot.writeFieldStop();" << endl
      << indent() << "oprot.writeStructEnd();" << endl;

  indent_down();
  out << indent() << "}" << endl << endl;
}

/**
 * Generates a function to write all the fields of the struct,
 * which is a function result. These fields are only written
 * if they are set in the Isset array, and only one of them
 * can be set at a time.
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_struct_result_writer(
    ofstream& out, const t_struct* tstruct) {
  out << indent() << "public void write(TProtocol oprot) throws TException {"
      << endl;
  indent_up();

  const vector<t_field*>& fields = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator f_iter;

  indent(out) << "oprot.writeStructBegin(STRUCT_DESC);" << endl;

  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
      out << endl << indent() << "if ";
    } else {
      out << " else if ";
    }
    out << "(this." << generate_isset_check(*f_iter) << ") {" << endl;

    indent_up();

    indent(out) << "oprot.writeFieldBegin("
                << constant_name((*f_iter)->get_name()) << "_FIELD_DESC);"
                << endl;

    // Write field contents
    generate_serialize_field(out, *f_iter, "this.");

    // Write field closer
    indent(out) << "oprot.writeFieldEnd();" << endl;

    indent_down();
    indent(out) << "}";
  }
  // Write the struct map
  out << endl
      << indent() << "oprot.writeFieldStop();" << endl
      << indent() << "oprot.writeStructEnd();" << endl;

  indent_down();
  out << indent() << "}" << endl << endl;
}

void t_java_deprecated_generator::generate_reflection_getters(
    ostringstream& out,
    const t_type* type,
    string field_name,
    string cap_name) {
  indent(out) << "case " << upcase_string(field_name) << ":" << endl;
  indent_up();

  if (type->is_base_type() && !type->is_string_or_binary()) {
    t_base_type* base_type = (t_base_type*)type;

    indent(out) << "return new " << type_name(type, true, false) << "("
                << (base_type->is_bool() ? "is" : "get") << cap_name << "());"
                << endl
                << endl;
  } else {
    indent(out) << "return get" << cap_name << "();" << endl << endl;
  }

  indent_down();
}

void t_java_deprecated_generator::generate_reflection_setters(
    ostringstream& out,
    const t_type* type,
    string field_name,
    string cap_name) {
  indent(out) << "case " << upcase_string(field_name) << ":" << endl;
  indent_up();
  indent(out) << "if (__value == null) {" << endl;
  indent(out) << "  unset" << get_cap_name(field_name) << "();" << endl;
  indent(out) << "} else {" << endl;
  indent(out) << "  set" << cap_name << "((" << type_name(type, true, false)
              << ")__value);" << endl;
  indent(out) << "}" << endl;
  indent(out) << "break;" << endl << endl;

  indent_down();
}

void t_java_deprecated_generator::generate_generic_field_getters_setters(
    std::ofstream& out, const t_struct* tstruct) {
  std::ostringstream getter_stream;
  std::ostringstream setter_stream;

  // build up the bodies of both the getter and setter at once
  bool needs_suppress_warnings = false;
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    const t_field* field = *f_iter;
    const t_type* type = field->get_type()->get_true_type();
    const std::string& field_name = field->get_name();
    std::string cap_name = get_cap_name(field_name);

    if (type->is_list() || type->is_map() || type->is_set())
      needs_suppress_warnings = true;

    indent_up();
    generate_reflection_setters(setter_stream, type, field_name, cap_name);
    generate_reflection_getters(getter_stream, type, field_name, cap_name);
    indent_down();
  }

  // create the setter
  if (!generate_immutable_structs_) {
    if (needs_suppress_warnings) {
      indent(out) << "@SuppressWarnings(\"unchecked\")" << endl;
    }
    indent(out) << "public void setFieldValue(int fieldID, Object __value) {"
                << endl;
    indent_up();

    indent(out) << "switch (fieldID) {" << endl;

    out << setter_stream.str();

    indent(out) << "default:" << endl;
    indent(out) << "  throw new IllegalArgumentException(\"Field \" + fieldID "
                   "+ \" doesn't exist!\");"
                << endl;

    indent(out) << "}" << endl;

    indent_down();
    indent(out) << "}" << endl << endl;

    // create the getter
    indent(out) << "public Object getFieldValue(int fieldID) {" << endl;
    indent_up();

    indent(out) << "switch (fieldID) {" << endl;

    out << getter_stream.str();

    indent(out) << "default:" << endl;
    indent(out) << "  throw new IllegalArgumentException(\"Field \" + fieldID "
                   "+ \" doesn't exist!\");"
                << endl;

    indent(out) << "}" << endl;

    indent_down();

    indent(out) << "}" << endl << endl;
  }
}

std::string t_java_deprecated_generator::get_simple_getter_name(
    const t_field* field) {
  const std::string& field_name = field->get_name();
  std::string cap_name = get_cap_name(field_name);
  const t_type* type = field->get_type()->get_true_type();

  if (type->is_base_type() &&
      ((t_base_type*)type)->get_base() == t_base_type::TYPE_BOOL) {
    return "is" + cap_name;
  } else {
    return "get" + cap_name;
  }
}

/**
 * Generates a set of Java Bean boilerplate functions (setters, getters, etc.)
 * for the given struct.
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_bean_boilerplate(
    ofstream& out, const t_struct* tstruct, bool gen_immutable) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    const t_field* field = *f_iter;
    const t_type* type = field->get_type()->get_true_type();
    std::string field_name = field->get_name();
    std::string cap_name = get_cap_name(field_name);

    // Simple getter
    std::string getter_name = get_simple_getter_name(field);
    generate_java_doc(out, field);
    if (is_field_sensitive(field)) {
      indent(out) << "@Sensitive" << endl;
    }
    indent(out) << "public " << type_name(type) << " " << getter_name << "() {"
                << endl;
    indent_up();
    indent(out) << "return this." << field_name << ";" << endl;
    indent_down();
    indent(out) << "}" << endl << endl;

    if (!gen_immutable) {
      // Simple setter
      generate_java_doc(out, field);
      indent(out) << "public " << type_name(tstruct) << " set" << cap_name
                  << "(" << type_name(type) << " " << field_name << ") {"
                  << endl;
      indent_up();
      indent(out) << "this." << field_name << " = " << field_name << ";"
                  << endl;
      generate_isset_set(out, field);
      indent(out) << "return this;" << endl;

      indent_down();
      indent(out) << "}" << endl << endl;

      // Unsetter
      indent(out) << "public void unset" << cap_name << "() {" << endl;
      indent_up();
      if (type_can_be_null(type)) {
        indent(out) << "this." << field_name << " = null;" << endl;
      } else {
        indent(out) << "__isset_bit_vector.clear(" << isset_field_id(field)
                    << ");" << endl;
      }
      indent_down();
      indent(out) << "}" << endl << endl;
    }

    // isSet method
    indent(out) << "// Returns true if field " << field_name
                << " is set (has been assigned a value) and false otherwise"
                << endl;
    indent(out) << "public boolean is" << get_cap_name("set") << cap_name
                << "() {" << endl;
    indent_up();
    if (type_can_be_null(type)) {
      indent(out) << "return this." << field_name << " != null;" << endl;
    } else {
      if (!gen_immutable) {
        indent(out) << "return __isset_bit_vector.get(" << isset_field_id(field)
                    << ");" << endl;
      } else {
        // Values must be set in the contructor for immutable structs
        indent(out) << "return true;" << endl;
      }
    }
    indent_down();
    indent(out) << "}" << endl << endl;

    if (!gen_immutable) {
      indent(out) << "public void set" << cap_name << get_cap_name("isSet")
                  << "(boolean __value) {" << endl;
      indent_up();
      if (type_can_be_null(type)) {
        indent(out) << "if (!__value) {" << endl;
        indent(out) << "  this." << field_name << " = null;" << endl;
        indent(out) << "}" << endl;
      } else {
        indent(out) << "__isset_bit_vector.set(" << isset_field_id(field)
                    << ", __value);" << endl;
      }
      indent_down();
      indent(out) << "}" << endl << endl;
    }
  }
}

void t_java_deprecated_generator::generate_default_toString(
    ofstream& out, const t_struct* /*tstruct*/) {
  out << indent() << "@Override" << endl
      << indent() << "public String toString() {" << endl;
  indent_up();
  out << indent() << "return toString(1, true);" << endl;
  indent_down();
  out << indent() << "}" << endl << endl;
}

/**
 * Generates a toString() method for the given struct
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_struct_tostring(
    ofstream& out, const t_struct* tstruct) {
  generate_default_toString(out, tstruct);
  out << indent() << "@Override" << endl;
  out << indent() << "public String toString(int indent, boolean prettyPrint) {"
      << endl;
  indent_up();
  if (generate_immutable_structs_) {
    out << indent()
        << "return TBaseHelper.toStringHelper(this, indent, prettyPrint);"
        << endl;
  } else {
    out << indent()
        << "String indentStr = prettyPrint ? "
           "TBaseHelper.getIndentedString(indent) "
        << ": \"\";" << endl;
    out << indent() << "String newLine = prettyPrint ? \"\\n\" : \"\";" << endl;
    out << indent() << "String space = prettyPrint ? \" \" : \"\";" << endl;
    out << indent() << "StringBuilder sb = new StringBuilder(\""
        << tstruct->get_name() << "\");" << endl;
    out << indent() << "sb.append(space);" << endl;
    out << indent() << "sb.append(\"(\");" << endl;
    out << indent() << "sb.append(newLine);" << endl;
    out << indent() << "boolean first = true;" << endl << endl;

    const vector<t_field*>& fields = tstruct->get_members();
    vector<t_field*>::const_iterator f_iter;
    bool first = true;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      const t_field* field = (*f_iter);
      string fname = field->get_name();
      const t_type* ftype = field->get_type()->get_true_type();

      if (tstruct->is_union()) {
        indent(out) << "// Only print this field if it is the set field"
                    << endl;
        indent(out) << "if (" << generate_setfield_check(field) << ")" << endl;
        scope_up(out);
      }

      bool could_be_unset = field->get_req() == t_field::e_req::optional;
      if (could_be_unset) {
        indent(out) << "if (" << generate_isset_check(field) << ")" << endl;
        scope_up(out);
      }

      if (!first) {
        indent(out) << "if (!first) sb.append(\",\" + newLine);" << endl;
      }
      indent(out) << "sb.append(indentStr);" << endl;
      indent(out) << "sb.append(\"" << fname << "\");" << endl;
      indent(out) << "sb.append(space);" << endl;
      indent(out) << "sb.append(\":\").append(space);" << endl;

      if (is_field_sensitive(field)) {
        indent(out) << "sb.append(\"<SENSITIVE FIELD>\");" << endl;
      } else {
        bool can_be_null = type_can_be_null(field->get_type());
        std::string field_getter =
            "this." + get_simple_getter_name(field) + "()";
        if (can_be_null) {
          indent(out) << "if (" << field_getter << " == null) {" << endl;
          indent(out) << "  sb.append(\"null\");" << endl;
          indent(out) << "} else {" << endl;
          indent_up();
        }

        if (ftype->is_base_type() && ftype->is_binary()) {
          indent(out) << "  int __" << fname << "_size = Math.min("
                      << field_getter << ".length, 128);" << endl;
          indent(out) << "  for (int i = 0; i < __" << fname << "_size; i++) {"
                      << endl;
          indent(out) << "    if (i != 0) sb.append(\" \");" << endl;
          indent(out) << "    sb.append(Integer.toHexString(" << field_getter
                      << "[i]).length() > 1 ? Integer.toHexString("
                      << field_getter << "[i]).substring(Integer.toHexString("
                      << field_getter
                      << "[i]).length() - 2).toUpperCase() : \"0\" + "
                         "Integer.toHexString("
                      << field_getter << "[i]).toUpperCase());" << endl;
          indent(out) << "  }" << endl;
          indent(out) << "  if (" << field_getter
                      << ".length > 128) sb.append(\" ...\");" << endl;
        } else if (ftype->is_enum()) {
          indent(out) << "String " << fname << "_name = " << field_getter
                      << " == null ? \"null\" : " << field_getter << ".name();"
                      << endl;
          indent(out) << "if (" << fname << "_name != null) {" << endl;
          indent(out) << "  sb.append(" << fname << "_name);" << endl;
          indent(out) << "  sb.append(\" (\");" << endl;
          indent(out) << "}" << endl;
          indent(out) << "sb.append(" << field_getter << ");" << endl;
          indent(out) << "if (" << fname << "_name != null) {" << endl;
          indent(out) << "  sb.append(\")\");" << endl;
          indent(out) << "}" << endl;
        } else {
          indent(out) << "sb.append(TBaseHelper.toString(" << field_getter
                      << ", indent + 1, prettyPrint));" << endl;
        }

        if (can_be_null) {
          indent_down();
          indent(out) << "}" << endl;
        }
      }
      indent(out) << "first = false;" << endl;

      if (could_be_unset) {
        scope_down(out);
      }

      if (tstruct->is_union()) {
        scope_down(out);
      }

      first = false;
    }
    out << indent()
        << "sb.append(newLine + TBaseHelper.reduceIndent(indentStr));" << endl;
    out << indent() << "sb.append(\")\");" << endl
        << indent() << "return sb.toString();" << endl;
  }
  indent_down();
  indent(out) << "}" << endl << endl;
}

/**
 * Generates a static map with meta data to store information such as fieldID
 * to fieldName mapping
 *
 * @param tstruct The struct definition
 */
void t_java_deprecated_generator::generate_java_meta_data_map(
    ofstream& out, const t_struct* tstruct) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  // Static Map with fieldID -> FieldMetaData mappings
  indent(out) << "public static final Map<Integer, FieldMetaData> metaDataMap;"
              << endl
              << endl;

  // Populate map
  indent(out) << "static {" << endl;
  indent_up();
  indent(out) << "Map<Integer, FieldMetaData> tmpMetaDataMap = "
                 "new HashMap<Integer, FieldMetaData>();"
              << endl;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    const t_field* field = *f_iter;
    const std::string& field_name = field->get_name();
    indent(out) << "tmpMetaDataMap.put(" << upcase_string(field_name)
                << ", new FieldMetaData(\"" << field_name << "\", ";

    // Set field requirement type (required, optional, etc.)
    if (field->get_req() == t_field::e_req::required) {
      out << "TFieldRequirementType.REQUIRED, ";
    } else if (field->get_req() == t_field::e_req::optional) {
      out << "TFieldRequirementType.OPTIONAL, ";
    } else {
      out << "TFieldRequirementType.DEFAULT, ";
    }

    // Create value meta data
    generate_field_value_meta_data(out, field->get_type());
    out << "));" << endl;
  }
  indent(out) << "metaDataMap = Collections.unmodifiableMap(tmpMetaDataMap);"
              << endl;
  indent_down();
  indent(out) << "}" << endl << endl;
}

/**
 * Returns a string with the java representation of the given thrift type
 * (e.g. for the type struct it returns "TType.STRUCT")
 */
std::string t_java_deprecated_generator::get_java_type_string(
    const t_type* type) {
  if (type->is_list()) {
    return "TType.LIST";
  } else if (type->is_map()) {
    return "TType.MAP";
  } else if (type->is_set()) {
    return "TType.SET";
  } else if (type->is_struct() || type->is_exception()) {
    return "TType.STRUCT";
  } else if (type->is_enum()) {
    return "TType.I32";
  } else if (type->is_typedef()) {
    return get_java_type_string(((t_typedef*)type)->get_type());
  } else if (type->is_base_type()) {
    switch (((t_base_type*)type)->get_base()) {
      case t_base_type::TYPE_VOID:
        return "TType.VOID";
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
      default:
        throw std::runtime_error(
            "Unknown thrift type \"" + type->get_name() +
            "\" passed to t_java_deprecated_generator::get_java_type_string!");
    }
  } else {
    throw std::runtime_error(
        "Unknown thrift type \"" + type->get_name() +
        "\" passed to t_java_deprecated_generator::get_java_type_string!");
  }
}

void t_java_deprecated_generator::generate_field_value_meta_data(
    std::ofstream& out, const t_type* type) {
  type = type->get_true_type();
  out << endl;
  indent_up();
  indent_up();
  if (type->is_struct()) {
    indent(out) << "new StructMetaData(TType.STRUCT, " << type_name(type)
                << ".class";
  } else if (type->is_container()) {
    if (type->is_list()) {
      indent(out) << "new ListMetaData(TType.LIST, ";
      const t_type* elem_type = ((t_list*)type)->get_elem_type();
      generate_field_value_meta_data(out, elem_type);
    } else if (type->is_set()) {
      indent(out) << "new SetMetaData(TType.SET, ";
      const t_type* elem_type = ((t_set*)type)->get_elem_type();
      generate_field_value_meta_data(out, elem_type);
    } else { // map
      indent(out) << "new MapMetaData(TType.MAP, ";
      const t_type* key_type = ((t_map*)type)->get_key_type();
      const t_type* val_type = ((t_map*)type)->get_val_type();
      generate_field_value_meta_data(out, key_type);
      out << ", ";
      generate_field_value_meta_data(out, val_type);
    }
  } else {
    indent(out) << "new FieldValueMetaData(" << get_java_type_string(type);
  }
  out << ")";
  indent_down();
  indent_down();
}

/**
 * Generates a thrift service. In C++, this comprises an entirely separate
 * header and source file. The header file defines the methods and includes
 * the data types defined in the main header file, and the implementation
 * file contains implementations of the basic printer and default interfaces.
 *
 * @param tservice The service definition
 */
void t_java_deprecated_generator::generate_service(const t_service* tservice) {
  // Make output file
  string f_service_name = get_package_dir() + "/" + service_name_ + ".java";
  f_service_.open(f_service_name.c_str());
  record_genfile(f_service_name);

  f_service_ << autogen_comment() << java_package() << java_service_imports()
             << java_thrift_imports();

  f_service_ << java_suppress_warnings_service() << "public class "
             << service_name_ << " {" << endl
             << endl;
  indent_up();

  // Generate the three main parts of the service
  generate_service_interface(tservice);
  generate_service_async_interface(tservice);
  generate_service_client(tservice);
  generate_service_async_client(tservice);
  generate_service_server(tservice);
  generate_service_helpers(tservice);

  indent_down();
  f_service_ << "}" << endl;
  f_service_.close();
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 */
void t_java_deprecated_generator::generate_service_interface(
    const t_service* tservice) {
  string extends = "";
  string extends_iface = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    extends_iface = " extends " + extends + ".Iface";
  }

  generate_java_doc(f_service_, tservice);
  f_service_ << indent() << "public interface Iface" << extends_iface << " {"
             << endl
             << endl;
  indent_up();
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (!can_generate_method(*f_iter)) {
      continue;
    }

    generate_java_doc(f_service_, *f_iter);
    indent(f_service_) << "public " << function_signature(*f_iter) << ";"
                       << endl
                       << endl;
  }
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

void t_java_deprecated_generator::generate_service_async_interface(
    const t_service* tservice) {
  string extends = "";
  string extends_iface = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    extends_iface = " extends " + extends + ".AsyncIface";
  }

  f_service_ << indent() << "public interface AsyncIface" << extends_iface
             << " {" << endl
             << endl;
  indent_up();
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (!can_generate_method(*f_iter)) {
      continue;
    }
    indent(f_service_) << "public "
                       << function_signature_async(
                              *f_iter, "resultHandler", true)
                       << " throws TException;" << endl
                       << endl;
  }
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

/**
 * Generates structs for all the service args and return types
 *
 * @param tservice The service
 */
void t_java_deprecated_generator::generate_service_helpers(
    const t_service* tservice) {
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (!can_generate_method(*f_iter)) {
      continue;
    }
    const t_struct* ts = (*f_iter)->get_paramlist();
    StructGenParams params;
    params.in_class = true;
    generate_java_struct_definition(f_service_, ts, params);
    generate_function_helpers(*f_iter);
  }
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_java_deprecated_generator::generate_service_client(
    const t_service* tservice) {
  string extends = "";
  string extends_client = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    extends_client = " extends " + extends + ".Client";
  } else {
    // Client base class
    extends_client = " extends EventHandlerBase";
  }

  indent(f_service_) << "public static class Client" << extends_client
                     << " implements Iface, TClientIf {" << endl;
  indent_up();

  indent(f_service_) << "public Client(TProtocol prot)" << endl;
  scope_up(f_service_);
  indent(f_service_) << "this(prot, prot);" << endl;
  scope_down(f_service_);
  f_service_ << endl;

  indent(f_service_) << "public Client(TProtocol iprot, TProtocol oprot)"
                     << endl;
  scope_up(f_service_);
  if (extends.empty()) {
    f_service_ << indent() << "iprot_ = iprot;" << endl
               << indent() << "oprot_ = oprot;" << endl;
  } else {
    f_service_ << indent() << "super(iprot, oprot);" << endl;
  }
  scope_down(f_service_);
  f_service_ << endl;

  if (extends.empty()) {
    f_service_ << indent() << "protected TProtocol iprot_;" << endl
               << indent() << "protected TProtocol oprot_;" << endl
               << endl
               << indent() << "protected int seqid_;" << endl
               << endl;

    f_service_ << indent() << "@Override" << endl
               << indent() << "public TProtocol getInputProtocol()" << endl;
    scope_up(f_service_);
    indent(f_service_) << "return this.iprot_;" << endl;
    scope_down(f_service_);
    f_service_ << endl;

    f_service_ << indent() << "@Override" << endl
               << indent() << "public TProtocol getOutputProtocol()" << endl;
    scope_up(f_service_);
    indent(f_service_) << "return this.oprot_;" << endl;
    scope_down(f_service_);
    f_service_ << endl;
  }

  // Generate client method implementations
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::const_iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (!can_generate_method(*f_iter)) {
      continue;
    }
    string funname = (*f_iter)->get_name();
    string service_func_name =
        "\"" + tservice->get_name() + "." + (*f_iter)->get_name() + "\"";
    // Open function
    indent(f_service_) << "public " << function_signature(*f_iter) << endl;
    scope_up(f_service_);

    f_service_ << indent() << "ContextStack ctx = getContextStack("
               << service_func_name << ", null);" << endl
               << indent() << "this.setContextStack(ctx);" << endl
               << indent() << "send_" << funname << "(";

    // Get the struct of function call params
    const t_struct* arg_struct = (*f_iter)->get_paramlist();

    // Declare the function arguments
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator fld_iter;
    bool first = true;
    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      if (first) {
        first = false;
      } else {
        f_service_ << ", ";
      }
      f_service_ << (*fld_iter)->get_name();
    }
    f_service_ << ");" << endl;

    if ((*f_iter)->qualifier() != t_function_qualifier::one_way) {
      f_service_ << indent();
      if (!(*f_iter)->return_type()->is_void()) {
        f_service_ << "return ";
      }
      f_service_ << "recv_" << funname << "();" << endl;
    }

    scope_down(f_service_);
    f_service_ << endl;

    t_function send_function(
        &t_base_type::t_void(),
        string("send_") + (*f_iter)->get_name(),
        t_struct::clone_DO_NOT_USE((*f_iter)->get_paramlist()));

    string argsname = (*f_iter)->get_name() + "_args";

    // Open function
    indent(f_service_) << "public " << function_signature(&send_function)
                       << endl;
    scope_up(f_service_);

    // Serialize the request
    indent(f_service_) << "ContextStack ctx = "
                       << "this.getContextStack();" << endl;
    indent(f_service_) << "super.preWrite(ctx, " << service_func_name
                       << ", null);" << endl;

    f_service_ << indent() << "oprot_.writeMessageBegin(new TMessage(\""
               << funname << "\", TMessageType.CALL, seqid_));" << endl
               << indent() << argsname << " args = new " << argsname << "();"
               << endl;

    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      f_service_ << indent() << "args." << (*fld_iter)->get_name() << " = "
                 << (*fld_iter)->get_name() << ";" << endl;
    }

    string bytes = tmp("_bytes");

    string flush = (*f_iter)->qualifier() == t_function_qualifier::one_way
        ? "onewayFlush"
        : "flush";
    f_service_ << indent() << "args.write(oprot_);" << endl
               << indent() << "oprot_.writeMessageEnd();" << endl
               << indent() << "oprot_.getTransport()." << flush << "();" << endl
               << indent() << "super.postWrite(ctx, " << service_func_name
               << ", args);" << endl
               << indent() << "return;" << endl;

    scope_down(f_service_);
    f_service_ << endl;

    // Generate recv function only if not a oneway function
    if ((*f_iter)->qualifier() != t_function_qualifier::one_way) {
      string resultname = (*f_iter)->get_name() + "_result";

      const t_throws* exceptions = (*f_iter)->exceptions();
      t_function recv_function(
          (*f_iter)->return_type(),
          string("recv_") + (*f_iter)->get_name(),
          std::make_unique<t_paramlist>(program_),
          exceptions ? t_struct::clone_DO_NOT_USE(exceptions) : nullptr);
      // Open the recv function
      indent(f_service_) << "public " << function_signature(&recv_function)
                         << endl;
      scope_up(f_service_);

      indent(f_service_) << "ContextStack ctx = "
                         << "super.getContextStack();" << endl; // newversion
      indent(f_service_) << "long bytes;" << endl;

      indent(f_service_) << "TMessageType mtype;" << endl
                         << indent() << "super.preRead(ctx, "
                         << service_func_name << ");" << endl;

      // TODO(mcslee): Message validation here, was the seqid etc ok?

      f_service_
          << indent() << "TMessage msg = iprot_.readMessageBegin();" << endl
          << indent() << "if (msg.type == TMessageType.EXCEPTION) {" << endl
          << indent()
          << "  TApplicationException x = TApplicationException.read(iprot_);"
          << endl
          << indent() << "  iprot_.readMessageEnd();" << endl
          << indent() << "  throw x;" << endl
          << indent() << "}" << endl;

      if (generate_immutable_structs_) {
        f_service_ << indent() << resultname << " result = " << resultname
                   << ".deserialize(iprot_);" << endl;
      } else {
        f_service_ << indent() << resultname << " result = new " << resultname
                   << "();" << endl
                   << indent() << "result.read(iprot_);" << endl;
      }

      f_service_ << indent() << "iprot_.readMessageEnd();" << endl
                 << indent() << "super.postRead(ctx, " << service_func_name
                 << ", result);" << endl
                 << endl;

      // Careful, only return _result if not a void function
      if (!(*f_iter)->return_type()->is_void()) {
        f_service_ << indent() << "if (result."
                   << generate_isset_check("success") << ") {" << endl
                   << indent() << "  return result.success;" << endl
                   << indent() << "}" << endl;
      }

      for (const auto& x : get_elems((*f_iter)->exceptions())) {
        f_service_ << indent() << "if (result." << x.get_name() << " != null) {"
                   << endl
                   << indent() << "  throw result." << x.get_name() << ";"
                   << endl
                   << indent() << "}" << endl;
      }

      // If you get here it's an exception, unless a void function
      if ((*f_iter)->return_type()->is_void()) {
        indent(f_service_) << "return;" << endl;
      } else {
        f_service_
            << indent()
            << "throw new "
               "TApplicationException(TApplicationException.MISSING_RESULT, \""
            << (*f_iter)->get_name() << " failed: unknown result\");" << endl;
      }

      // Close function
      scope_down(f_service_);
      f_service_ << endl;
    }
  }

  indent_down();
  indent(f_service_) << "}" << endl;
}

void t_java_deprecated_generator::generate_service_async_client(
    const t_service* tservice) {
  string extends = "TAsyncClient";
  string extends_client = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends()) + ".AsyncClient";
  }

  indent(f_service_) << "public static class AsyncClient extends " << extends
                     << " implements AsyncIface {" << endl;
  indent_up();

  // Factory method
  indent(f_service_) << "public static class Factory implements "
                        "TAsyncClientFactory<AsyncClient> {"
                     << endl;
  indent(f_service_) << "  private TAsyncClientManager clientManager;" << endl;
  indent(f_service_) << "  private TProtocolFactory protocolFactory;" << endl;
  indent(f_service_) << "  public Factory(TAsyncClientManager clientManager, "
                        "TProtocolFactory protocolFactory) {"
                     << endl;
  indent(f_service_) << "    this.clientManager = clientManager;" << endl;
  indent(f_service_) << "    this.protocolFactory = protocolFactory;" << endl;
  indent(f_service_) << "  }" << endl;
  indent(f_service_) << "  public AsyncClient "
                        "getAsyncClient(TNonblockingTransport transport) {"
                     << endl;
  indent(f_service_) << "    return new AsyncClient(protocolFactory, "
                        "clientManager, transport);"
                     << endl;
  indent(f_service_) << "  }" << endl;
  indent(f_service_) << "}" << endl << endl;

  indent(f_service_)
      << "public AsyncClient(TProtocolFactory protocolFactory, "
         "TAsyncClientManager clientManager, TNonblockingTransport transport) {"
      << endl;
  indent(f_service_) << "  super(protocolFactory, clientManager, transport);"
                     << endl;
  indent(f_service_) << "}" << endl << endl;

  // Generate client method implementations
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::const_iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (!can_generate_method(*f_iter)) {
      continue;
    }
    string funname = (*f_iter)->get_name();
    const t_type* ret_type = (*f_iter)->return_type();
    const t_struct* arg_struct = (*f_iter)->get_paramlist();
    string funclassname = funname + "_call";
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator fld_iter;
    string args_name = (*f_iter)->get_name() + "_args";
    string result_name = (*f_iter)->get_name() + "_result";
    string result_handler_symbol;
    string client_sybmol = tmp("client");
    string protocol_factory_symbol = tmp("protocolFactory");
    string transport_symbol = tmp("transport");

    // Main method body
    result_handler_symbol = tmp("resultHandler");
    f_service_ << indent() << "public "
               << function_signature_async(
                      *f_iter, result_handler_symbol, false)
               << " throws TException {" << endl
               << indent() << "  checkReady();" << endl
               << indent() << "  " << funclassname << " method_call = new "
               << funclassname << "("
               << async_argument_list(
                      *f_iter, arg_struct, result_handler_symbol)
               << ", this, ___protocolFactory, ___transport);" << endl
               << indent() << "  this.___currentMethod = method_call;" << endl
               << indent() << "  ___manager.call(method_call);" << endl
               << indent() << "}" << endl
               << endl;

    // TAsyncMethod object for this function call
    indent(f_service_) << "public static class " + funclassname +
            " extends TAsyncMethodCall {"
                       << endl;
    indent_up();

    // Member variables
    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      indent(f_service_) << "private " + type_name((*fld_iter)->get_type()) +
              " " + (*fld_iter)->get_name() +
              ";" << endl;
    }

    // NOTE since we use a new Client instance to deserialize, let's keep seqid
    // to 0 for now indent(f_service_) << "private int seqid;" << endl << endl;

    // Constructor
    result_handler_symbol = tmp("resultHandler");
    indent(f_service_) << "public " + funclassname + "(" +
            async_argument_list(
                              *f_iter, arg_struct, result_handler_symbol, true)
                       << ", TAsyncClient " << client_sybmol
                       << ", TProtocolFactory " << protocol_factory_symbol
                       << ", TNonblockingTransport " << transport_symbol
                       << ") throws TException {" << endl;
    indent(f_service_) << "  super(" << client_sybmol << ", "
                       << protocol_factory_symbol << ", " << transport_symbol
                       << ", " << result_handler_symbol << ", "
                       << ((*f_iter)->qualifier() ==
                                   t_function_qualifier::one_way
                               ? "true"
                               : "false")
                       << ");" << endl;

    // Assign member variables
    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      indent(f_service_) << "  this." + (*fld_iter)->get_name() + " = " +
              (*fld_iter)->get_name() +
              ";" << endl;
    }

    indent(f_service_) << "}" << endl << endl;

    indent(f_service_)
        << "public void write_args(TProtocol prot) throws TException {" << endl;
    indent_up();

    // Serialize request
    // NOTE we are leaving seqid as 0, for now (see above)
    string pservice_func_name =
        "\"" + tservice->get_name() + "." + funname + "\"";
    f_service_ << indent() << "prot.writeMessageBegin(new TMessage(\""
               << funname << "\", TMessageType.CALL, 0));" << endl
               << indent() << args_name << " args = new " << args_name << "();"
               << endl;

    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      f_service_ << indent() << "args.set"
                 << get_cap_name((*fld_iter)->get_name()) << "("
                 << (*fld_iter)->get_name() << ");" << endl;
    }

    f_service_ << indent() << "args.write(prot);" << endl
               << indent() << "prot.writeMessageEnd();" << endl;

    indent_down();
    indent(f_service_) << "}" << endl << endl;

    // Return method
    indent(f_service_) << "public " + type_name(ret_type) +
            " getResult() throws ";
    for (const t_field& x : get_elems((*f_iter)->exceptions())) {
      f_service_ << type_name(x.get_type(), false, false) + ", ";
    }
    f_service_ << "TException {" << endl;

    indent_up();
    f_service_
        << indent() << "if (getState() != State.RESPONSE_READ) {" << endl
        << indent()
        << "  throw new IllegalStateException(\"Method call not finished!\");"
        << endl
        << indent() << "}" << endl
        << indent()
        << "TMemoryInputTransport memoryTransport = new "
           "TMemoryInputTransport(getFrameBuffer().array());"
        << endl
        << indent()
        << "TProtocol prot = "
           "super.client.getProtocolFactory().getProtocol(memoryTransport);"
        << endl;
    if ((*f_iter)->qualifier() != t_function_qualifier::one_way) {
      indent(f_service_);
      if (!ret_type->is_void()) {
        f_service_ << "return ";
      }
      f_service_ << "(new Client(prot)).recv_" + funname + "();" << endl;
    }

    // Close function
    indent_down();
    indent(f_service_) << "}" << endl;

    // Close class
    indent_down();
    indent(f_service_) << "}" << endl << endl;
  }

  // Close AsyncClient
  scope_down(f_service_);
  f_service_ << endl;
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_java_deprecated_generator::generate_service_server(
    const t_service* tservice) {
  // Generate the dispatch methods
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  // Extends stuff
  string extends = "";
  string extends_processor = "";
  if (tservice->get_extends() != nullptr) {
    extends = type_name(tservice->get_extends());
    extends_processor = " extends " + extends + ".Processor";
  }

  // Generate the header portion
  indent(f_service_) << "public static class Processor" << extends_processor
                     << " implements TProcessor {" << endl;
  indent_up();

  indent(f_service_) << "private static final Logger LOGGER = "
                        "LoggerFactory.getLogger(Processor.class.getName());"
                     << endl;

  indent(f_service_) << "public Processor(Iface iface)" << endl;
  scope_up(f_service_);
  if (!extends.empty()) {
    f_service_ << indent() << "super(iface);" << endl;
  }
  f_service_ << indent() << "iface_ = iface;" << endl;

  if (extends.empty()) {
    f_service_
        << indent()
        << "event_handler_ = new TProcessorEventHandler(); // Empty handler"
        << endl;
  }

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (!can_generate_method(*f_iter)) {
      continue;
    }
    f_service_ << indent() << "processMap_.put(\"" << (*f_iter)->get_name()
               << "\", new " << (*f_iter)->get_name() << "());" << endl;
  }

  scope_down(f_service_);
  f_service_ << endl;

  if (extends.empty()) {
    f_service_ << indent() << "protected static interface ProcessFunction {"
               << endl
               << indent()
               << "  public void process(int seqid, TProtocol iprot, TProtocol "
                  "oprot, TConnectionContext server_ctx) throws TException;"
               << endl
               << indent() << "}" << endl
               << endl;

    f_service_
        << indent()
        << "public void setEventHandler(TProcessorEventHandler handler) {"
        << endl
        << indent() << "  this.event_handler_ = handler;" << endl
        << indent() << "}" << endl
        << endl;
  }

  f_service_ << indent() << "private Iface iface_;" << endl;

  if (extends.empty()) {
    f_service_ << indent() << "protected TProcessorEventHandler event_handler_;"
               << endl;
  }

  if (extends.empty()) {
    f_service_ << indent()
               << "protected final HashMap<String,ProcessFunction> processMap_ "
                  "= new HashMap<String,ProcessFunction>();"
               << endl;
  }

  f_service_ << endl;

  // Generate the server implementation
  indent(f_service_)
      << "public boolean process(TProtocol iprot, TProtocol oprot, "
         "TConnectionContext server_ctx) throws TException"
      << endl;
  scope_up(f_service_);

  f_service_ << indent() << "TMessage msg = iprot.readMessageBegin();" << endl;

  // TODO(mcslee): validate message, was the seqid etc. legit?

  f_service_ << indent() << "ProcessFunction fn = processMap_.get(msg.name);"
             << endl
             << indent() << "if (fn == null) {" << endl
             << indent() << "  TProtocolUtil.skip(iprot, TType.STRUCT);" << endl
             << indent() << "  iprot.readMessageEnd();" << endl
             << indent()
             << "  TApplicationException x = new "
                "TApplicationException(TApplicationException.UNKNOWN_METHOD, "
                "\"Invalid method name: '\"+msg.name+\"'\");"
             << endl
             << indent()
             << "  oprot.writeMessageBegin(new TMessage(msg.name, "
                "TMessageType.EXCEPTION, msg.seqid));"
             << endl
             << indent() << "  x.write(oprot);" << endl
             << indent() << "  oprot.writeMessageEnd();" << endl
             << indent() << "  oprot.getTransport().flush();" << endl
             << indent() << "  return true;" << endl
             << indent() << "}" << endl
             << indent() << "fn.process(msg.seqid, iprot, oprot, server_ctx);"
             << endl;

  f_service_ << indent() << "return true;" << endl;

  scope_down(f_service_);
  f_service_ << endl;

  // Generate the process subfunctions
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    if (!can_generate_method(*f_iter)) {
      continue;
    }
    generate_process_function(tservice, *f_iter);
  }

  indent_down();
  indent(f_service_) << "}" << endl << endl;
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_java_deprecated_generator::generate_function_helpers(
    const t_function* tfunction) {
  if (tfunction->qualifier() == t_function_qualifier::one_way) {
    return;
  }

  t_struct result(program_, tfunction->get_name() + "_result");
  auto success =
      std::make_unique<t_field>(tfunction->return_type(), "success", 0);
  if (!tfunction->return_type()->is_void()) {
    result.append(std::move(success));
  }

  for (const t_field& x : get_elems(tfunction->exceptions())) {
    result.append(x.clone_DO_NOT_USE());
  }

  StructGenParams params;
  params.in_class = true;
  params.is_result = true;
  generate_java_struct_definition(f_service_, &result, params);
}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_java_deprecated_generator::generate_process_function(
    const t_service* tservice, const t_function* tfunction) {
  // Open class
  indent(f_service_) << "private class " << tfunction->get_name()
                     << " implements ProcessFunction {" << endl;
  indent_up();

  // Open function
  indent(f_service_)
      << "public void process(int seqid, TProtocol iprot, TProtocol oprot, "
         "TConnectionContext server_ctx) throws TException"
      << endl;
  scope_up(f_service_);

  string argsname = tfunction->get_name() + "_args";
  string resultname = tfunction->get_name() + "_result";
  string pservice_fn_name =
      "\"" + tservice->get_name() + "." + tfunction->get_name() + "\"";
  f_service_ << indent() << "Object handler_ctx = event_handler_.getContext("
             << pservice_fn_name << ", server_ctx);" << endl
             << indent() << argsname << " args = new " << argsname << "();"
             << endl
             << indent() << "event_handler_.preRead(handler_ctx, "
             << pservice_fn_name << ");" << endl
             << indent() << "args.read(iprot);" << endl
             << indent() << "iprot.readMessageEnd();" << endl
             << indent() << "event_handler_.postRead(handler_ctx, "
             << pservice_fn_name << ", args);" << endl;

  // Declare result for non oneway function
  if (tfunction->qualifier() != t_function_qualifier::one_way) {
    f_service_ << indent() << resultname << " result = new " << resultname
               << "();" << endl;
  }

  // Emit a try block for a function with exceptions.
  auto exceptions = get_elems(tfunction->exceptions());
  if (!exceptions.empty()) {
    f_service_ << indent() << "try {" << endl;
    indent_up();
  }

  // Generate the function call
  const t_struct* arg_struct = tfunction->get_paramlist();
  const std::vector<t_field*>& fields = arg_struct->get_members();
  vector<t_field*>::const_iterator f_iter;

  f_service_ << indent();
  if (tfunction->qualifier() != t_function_qualifier::one_way &&
      !tfunction->return_type()->is_void()) {
    f_service_ << "result.success = ";
  }
  f_service_ << "iface_." << tfunction->get_name() << "(";
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      f_service_ << ", ";
    }
    f_service_ << "args." << (*f_iter)->get_name();
  }
  f_service_ << ");" << endl;

  // Set isset on success field
  if (tfunction->qualifier() != t_function_qualifier::one_way &&
      !tfunction->return_type()->is_void() &&
      !type_can_be_null(tfunction->return_type())) {
    f_service_ << indent() << "result.set" << get_cap_name("success")
               << get_cap_name("isSet") << "(true);" << endl;
  }

  if (tfunction->qualifier() != t_function_qualifier::one_way &&
      !exceptions.empty()) {
    string pservice_func_name =
        "\"" + tservice->get_name() + "." + tfunction->get_name() + "\"";
    string pservice_func_name_error =
        tservice->get_name() + "." + tfunction->get_name();
    indent_down();
    f_service_ << indent() << "}";
    for (const t_field& x : exceptions) {
      f_service_ << " catch (" << type_name(x.get_type(), false, false) << " "
                 << x.get_name() << ") {" << endl;
      if (tfunction->qualifier() != t_function_qualifier::one_way) {
        indent_up();
        f_service_ << indent() << "result." << x.get_name() << " = "
                   << x.get_name() << ";" << endl
                   << indent()
                   << "event_handler_.declaredUserException(handler_ctx, "
                   << pservice_func_name << ", " << x.get_name() << ");"
                   << endl;
        indent_down();
        f_service_ << indent() << "}";
      } else {
        f_service_ << "}";
      }
    }
    f_service_ << " catch (Throwable th) {" << endl;
    indent_up();
    f_service_ << indent() << "LOGGER.error(\"Internal error processing "
               << pservice_func_name_error << "\", th);" << endl
               << indent() << "event_handler_.handlerError(handler_ctx, "
               << pservice_func_name << ", th);" << endl
               << indent()
               << "TApplicationException x = new "
                  "TApplicationException(TApplicationException.INTERNAL_ERROR, "
                  "\"Internal error processing "
               << pservice_func_name_error << "\");" << endl
               << indent() << "event_handler_.preWrite(handler_ctx, \""
               << pservice_func_name_error << "\", null);" << endl
               << indent() << "oprot.writeMessageBegin(new TMessage("
               << pservice_func_name << ", TMessageType.EXCEPTION, seqid));"
               << endl
               << indent() << "x.write(oprot);" << endl
               << indent() << "oprot.writeMessageEnd();" << endl
               << indent() << "oprot.getTransport().flush();" << endl
               << indent() << "event_handler_.postWrite(handler_ctx, "
               << pservice_func_name << ", null);" << endl
               << indent() << "return;" << endl;
    indent_down();
    f_service_ << indent() << "}" << endl;
  }

  // Shortcut out here for oneway functions
  if (tfunction->qualifier() == t_function_qualifier::one_way) {
    f_service_ << indent() << "return;" << endl;
    scope_down(f_service_);

    // Close class
    indent_down();
    f_service_ << indent() << "}" << endl << endl;
    return;
  }

  // string pservice_fn_name = "\"" + tservice->get_name() + "." +
  // tfunction->get_name() + "\"";
  f_service_ << indent() << "event_handler_.preWrite(handler_ctx, "
             << pservice_fn_name << ", result);" << endl
             << indent() << "oprot.writeMessageBegin(new TMessage(\""
             << tfunction->get_name() << "\", TMessageType.REPLY, seqid));"
             << endl
             << indent() << "result.write(oprot);" << endl
             << indent() << "oprot.writeMessageEnd();" << endl
             << indent() << "oprot.getTransport().flush();" << endl
             << indent() << "event_handler_.postWrite(handler_ctx, "
             << pservice_fn_name << ", result);" << endl;

  // Close function
  scope_down(f_service_);
  f_service_ << endl;

  // Close class
  indent_down();
  f_service_ << indent() << "}" << endl << endl;
}

/**
 * Deserializes a field of any type.
 *
 * @param tfield The field
 * @param prefix The variable name or container for this field
 */
void t_java_deprecated_generator::generate_deserialize_field(
    ofstream& out, const t_field* tfield, string prefix) {
  const t_type* type = tfield->get_type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->get_name());
  }

  string name = prefix + tfield->get_name();

  if (type->is_struct() || type->is_exception()) {
    generate_deserialize_struct(out, (t_struct*)type, name);
  } else if (type->is_container()) {
    generate_deserialize_container(out, type, name);
  } else if (type->is_enum()) {
    indent(out) << name << " = "
                << get_enum_class_name(tfield->get_type()->get_true_type())
                << ".findByValue(iprot.readI32());" << endl;
  } else if (type->is_base_type()) {
    indent(out) << name << " = iprot.";
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error(
            "compiler error: cannot serialize void field in a struct: " + name);
      case t_base_type::TYPE_STRING:
        out << "readString();";
        break;
      case t_base_type::TYPE_BINARY:
        out << "readBinary();";
        break;
      case t_base_type::TYPE_BOOL:
        out << "readBool();";
        break;
      case t_base_type::TYPE_BYTE:
        out << "readByte();";
        break;
      case t_base_type::TYPE_I16:
        out << "readI16();";
        break;
      case t_base_type::TYPE_I32:
        out << "readI32();";
        break;
      case t_base_type::TYPE_I64:
        out << "readI64();";
        break;
      case t_base_type::TYPE_DOUBLE:
        out << "readDouble();";
        break;
      case t_base_type::TYPE_FLOAT:
        out << "readFloat();";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no Java name for base type " +
            t_base_type::t_base_name(tbase));
    }
    out << endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n",
        tfield->get_name().c_str(),
        type_name(type).c_str());
    throw std::runtime_error("compiler error");
  }
}

/**
 * Generates an unserializer for a struct, invokes read()
 */
void t_java_deprecated_generator::generate_deserialize_struct(
    ofstream& out, const t_struct* tstruct, string prefix) {
  if (generate_immutable_structs_ && !tstruct->is_union()) {
    out << indent() << prefix << " = " << type_name(tstruct)
        << ".deserialize(iprot);" << endl;
  } else {
    out << indent() << prefix << " = new " << type_name(tstruct) << "();"
        << endl
        << indent() << prefix << ".read(iprot);" << endl;
  }
}

/**
 * Deserializes a container by reading its size and then iterating
 */
void t_java_deprecated_generator::generate_deserialize_container(
    ofstream& out, const t_type* ttype, string prefix) {
  scope_up(out);

  string obj;

  if (ttype->is_map()) {
    obj = tmp("_map");
  } else if (ttype->is_set()) {
    obj = tmp("_set");
  } else if (ttype->is_list()) {
    obj = tmp("_list");
  }

  // Declare variables, read header
  if (ttype->is_map()) {
    indent(out) << "TMap " << obj << " = iprot.readMapBegin();" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "TSet " << obj << " = iprot.readSetBegin();" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "TList " << obj << " = iprot.readListBegin();" << endl;
  }

  // Protocol may have explicit non-negative size for a collection,
  // or specify -1 for size, and expect user to peek into collection
  // for one element at a time.

  indent(out) << prefix << " = new "
              << type_name(ttype, false, true)
              // size the collection correctly,
              // use initial capacity of 0 if there is no explicit size
              << "(Math.max(0, " << (ttype->is_list() ? "" : "2*") << obj
              << ".size"
              << "));" << endl;

  // For loop iterates over elements
  // Use explicit size, if provided, or peek for one element at a time if not
  string i = tmp("_i");
  indent(out) << "for (int " << i << " = 0; " << endl;

  indent(out) << "     (" << obj << ".size < 0) ? ";

  if (ttype->is_map()) {
    out << "iprot.peekMap()";
  } else if (ttype->is_set()) {
    out << "iprot.peekSet()";
  } else if (ttype->is_list()) {
    out << "iprot.peekList()";
  }

  out << " : (" << i << " < " << obj << ".size"
      << "); " << endl;
  indent(out) << "     ++" << i << ")" << endl;

  scope_up(out);

  if (ttype->is_map()) {
    generate_deserialize_map_element(out, (t_map*)ttype, prefix);
  } else if (ttype->is_set()) {
    generate_deserialize_set_element(out, (t_set*)ttype, prefix);
  } else if (ttype->is_list()) {
    generate_deserialize_list_element(out, (t_list*)ttype, prefix);
  }

  scope_down(out);

  // Read container end
  if (ttype->is_map()) {
    indent(out) << "iprot.readMapEnd();" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "iprot.readSetEnd();" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "iprot.readListEnd();" << endl;
  }

  scope_down(out);
}

/**
 * Generates code to deserialize a map
 */
void t_java_deprecated_generator::generate_deserialize_map_element(
    ofstream& out, const t_map* tmap, string prefix) {
  string key = tmp("_key");
  string val = tmp("_val");
  t_field fkey(tmap->get_key_type(), key);
  t_field fval(tmap->get_val_type(), val);

  indent(out) << declare_field(&fkey) << endl;
  indent(out) << declare_field(&fval) << endl;

  generate_deserialize_field(out, &fkey);
  generate_deserialize_field(out, &fval);

  indent(out) << prefix << ".put(" << key << ", " << val << ");" << endl;
}

/**
 * Deserializes a set element
 */
void t_java_deprecated_generator::generate_deserialize_set_element(
    ofstream& out, const t_set* tset, string prefix) {
  string elem = tmp("_elem");
  t_field felem(tset->get_elem_type(), elem);

  indent(out) << declare_field(&felem) << endl;

  generate_deserialize_field(out, &felem);

  indent(out) << prefix << ".add(" << elem << ");" << endl;
}

/**
 * Deserializes a list element
 */
void t_java_deprecated_generator::generate_deserialize_list_element(
    ofstream& out, const t_list* tlist, string prefix) {
  string elem = tmp("_elem");
  t_field felem(tlist->get_elem_type(), elem);

  indent(out) << declare_field(&felem) << endl;

  generate_deserialize_field(out, &felem);

  indent(out) << prefix << ".add(" << elem << ");" << endl;
}

/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param prefix Name to prepend to field name
 */
void t_java_deprecated_generator::generate_serialize_field(
    ofstream& out, const t_field* tfield, string prefix) {
  const t_type* type = tfield->get_type()->get_true_type();

  // Do nothing for void types
  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + prefix +
        tfield->get_name());
  }

  if (type->is_struct() || type->is_exception()) {
    generate_serialize_struct(
        out, (t_struct*)type, prefix + tfield->get_name());
  } else if (type->is_container()) {
    generate_serialize_container(out, type, prefix + tfield->get_name());
  } else if (type->is_enum()) {
    auto enumName = prefix + tfield->get_name();
    indent(out) << "oprot.writeI32(" << enumName
                << " == null ? 0 : " << enumName << ".getValue());" << endl;
  } else if (type->is_base_type()) {
    string name = prefix + tfield->get_name();
    indent(out) << "oprot.";

    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error(
            "compiler error: cannot serialize void field in a struct: " + name);
      case t_base_type::TYPE_STRING:
        out << "writeString(" << name << ");";
        break;
      case t_base_type::TYPE_BINARY:
        out << "writeBinary(" << name << ");";
        break;
      case t_base_type::TYPE_BOOL:
        out << "writeBool(" << name << ");";
        break;
      case t_base_type::TYPE_BYTE:
        out << "writeByte(" << name << ");";
        break;
      case t_base_type::TYPE_I16:
        out << "writeI16(" << name << ");";
        break;
      case t_base_type::TYPE_I32:
        out << "writeI32(" << name << ");";
        break;
      case t_base_type::TYPE_I64:
        out << "writeI64(" << name << ");";
        break;
      case t_base_type::TYPE_DOUBLE:
        out << "writeDouble(" << name << ");";
        break;
      case t_base_type::TYPE_FLOAT:
        out << "writeFloat(" << name << ");";
        break;
      default:
        throw std::runtime_error(
            "compiler error: no Java name for base type " +
            t_base_type::t_base_name(tbase));
    }
    out << endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO SERIALIZE FIELD '%s%s' TYPE '%s'\n",
        prefix.c_str(),
        tfield->get_name().c_str(),
        type_name(type).c_str());
    throw std::runtime_error("compiler error");
  }
}

/**
 * Serializes all the members of a struct.
 *
 * @param tstruct The struct to serialize
 * @param prefix  String prefix to attach to all fields
 */
void t_java_deprecated_generator::generate_serialize_struct(
    ofstream& out, const t_struct* /*tstruct*/, string prefix) {
  out << indent() << prefix << ".write(oprot);" << endl;
}

/**
 * Serializes a container by writing its size then the elements.
 *
 * @param ttype  The type of container
 * @param prefix String prefix for fields
 */
void t_java_deprecated_generator::generate_serialize_container(
    ofstream& out, const t_type* ttype, string prefix) {
  scope_up(out);

  if (ttype->is_map()) {
    indent(out) << "oprot.writeMapBegin(new TMap("
                << type_to_enum(((t_map*)ttype)->get_key_type()) << ", "
                << type_to_enum(((t_map*)ttype)->get_val_type()) << ", "
                << prefix << ".size()));" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "oprot.writeSetBegin(new TSet("
                << type_to_enum(((t_set*)ttype)->get_elem_type()) << ", "
                << prefix << ".size()));" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "oprot.writeListBegin(new TList("
                << type_to_enum(((t_list*)ttype)->get_elem_type()) << ", "
                << prefix << ".size()));" << endl;
  }

  string iter = tmp("_iter");
  if (ttype->is_map()) {
    indent(out) << "for (Map.Entry<"
                << type_name(((t_map*)ttype)->get_key_type(), true, false)
                << ", "
                << type_name(((t_map*)ttype)->get_val_type(), true, false)
                << "> " << iter << " : " << prefix << ".entrySet())";
  } else if (ttype->is_set()) {
    indent(out) << "for (" << type_name(((t_set*)ttype)->get_elem_type()) << " "
                << iter << " : " << prefix << ")";
  } else if (ttype->is_list()) {
    indent(out) << "for (" << type_name(((t_list*)ttype)->get_elem_type())
                << " " << iter << " : " << prefix << ")";
  }

  scope_up(out);

  if (ttype->is_map()) {
    generate_serialize_map_element(out, (t_map*)ttype, iter, prefix);
  } else if (ttype->is_set()) {
    generate_serialize_set_element(out, (t_set*)ttype, iter);
  } else if (ttype->is_list()) {
    generate_serialize_list_element(out, (t_list*)ttype, iter);
  }

  scope_down(out);

  if (ttype->is_map()) {
    indent(out) << "oprot.writeMapEnd();" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "oprot.writeSetEnd();" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "oprot.writeListEnd();" << endl;
  }

  scope_down(out);
}

/**
 * Serializes the members of a map.
 */
void t_java_deprecated_generator::generate_serialize_map_element(
    ofstream& out, const t_map* tmap, string iter, string /*map*/) {
  t_field kfield(tmap->get_key_type(), iter + ".getKey()");
  generate_serialize_field(out, &kfield, "");
  t_field vfield(tmap->get_val_type(), iter + ".getValue()");
  generate_serialize_field(out, &vfield, "");
}

/**
 * Serializes the members of a set.
 */
void t_java_deprecated_generator::generate_serialize_set_element(
    ofstream& out, const t_set* tset, string iter) {
  t_field efield(tset->get_elem_type(), iter);
  generate_serialize_field(out, &efield, "");
}

/**
 * Serializes the members of a list.
 */
void t_java_deprecated_generator::generate_serialize_list_element(
    ofstream& out, const t_list* tlist, string iter) {
  t_field efield(tlist->get_elem_type(), iter);
  generate_serialize_field(out, &efield, "");
}

/**
 * Returns a Java type name
 *
 * @param ttype The type
 * @param container Is the type going inside a container?
 * @return Java type name, i.e. HashMap<Key,Value>
 */
string t_java_deprecated_generator::type_name(
    const t_type* ttype, bool in_container, bool in_init, bool skip_generic) {
  // In Java typedefs are just resolved to their real type
  ttype = ttype->get_true_type();
  string prefix;

  if (ttype->is_base_type()) {
    return base_type_name((t_base_type*)ttype, in_container);
  } else if (ttype->is_map()) {
    const t_map* tmap = (t_map*)ttype;
    if (in_init) {
      prefix = "HashMap";
    } else {
      prefix = "Map";
    }
    return prefix +
        (skip_generic ? ""
                      : "<" + type_name(tmap->get_key_type(), true) + "," +
                 type_name(tmap->get_val_type(), true) + ">");
  } else if (ttype->is_set()) {
    const t_set* tset = (t_set*)ttype;
    if (in_init) {
      prefix = "HashSet";
    } else {
      prefix = "Set";
    }
    return prefix +
        (skip_generic ? ""
                      : "<" + type_name(tset->get_elem_type(), true) + ">");
  } else if (ttype->is_list()) {
    const t_list* tlist = (t_list*)ttype;
    if (in_init) {
      prefix = "ArrayList";
    } else {
      prefix = "List";
    }
    return prefix +
        (skip_generic ? ""
                      : "<" + type_name(tlist->get_elem_type(), true) + ">");
  }

  // Check for namespacing
  const t_program* program = ttype->program();
  if (program != nullptr && program != program_) {
    const string& package = program->get_namespace(namespace_key_);
    if (!package.empty()) {
      return package + "." + ttype->get_name();
    }
  }

  return ttype->get_name();
}

/**
 * Returns the C++ type that corresponds to the thrift type.
 *
 * @param tbase The base type
 * @param container Is it going in a Java container?
 */
string t_java_deprecated_generator::base_type_name(
    t_base_type* type, bool in_container) {
  t_base_type::t_base tbase = type->get_base();
  bool boxedPrimitive = in_container || generate_boxed_primitive;

  switch (tbase) {
    case t_base_type::TYPE_VOID:
      return "void";
    case t_base_type::TYPE_STRING:
      return "String";
    case t_base_type::TYPE_BINARY:
      return "byte[]";
    case t_base_type::TYPE_BOOL:
      return (boxedPrimitive ? "Boolean" : "boolean");
    case t_base_type::TYPE_BYTE:
      return (boxedPrimitive ? "Byte" : "byte");
    case t_base_type::TYPE_I16:
      return (boxedPrimitive ? "Short" : "short");
    case t_base_type::TYPE_I32:
      return (boxedPrimitive ? "Integer" : "int");
    case t_base_type::TYPE_I64:
      return (boxedPrimitive ? "Long" : "long");
    case t_base_type::TYPE_DOUBLE:
      return (boxedPrimitive ? "Double" : "double");
    case t_base_type::TYPE_FLOAT:
      return (boxedPrimitive ? "Float" : "float");
    default:
      throw std::runtime_error(
          "compiler error: no C++ name for base type " +
          t_base_type::t_base_name(tbase));
  }
}

/**
 * Declares a field, which may include initialization as necessary.
 *
 * @param ttype The type
 */
string t_java_deprecated_generator::declare_field(
    const t_field* tfield, bool init) {
  // TODO(mcslee): do we ever need to initialize the field?
  string result = type_name(tfield->get_type()) + " " + tfield->get_name();
  if (init) {
    const t_type* ttype = tfield->get_type()->get_true_type();
    if (ttype->is_base_type() && tfield->get_value() != nullptr) {
      ofstream dummy;
      result += " = " +
          render_const_value(
                    dummy, tfield->get_name(), ttype, tfield->get_value());
    } else if (ttype->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw std::runtime_error("NO T_VOID CONSTRUCT");
        case t_base_type::TYPE_STRING:
        case t_base_type::TYPE_BINARY:
          result += " = null";
          break;
        case t_base_type::TYPE_BOOL:
          result += " = false";
          break;
        case t_base_type::TYPE_BYTE:
        case t_base_type::TYPE_I16:
        case t_base_type::TYPE_I32:
        case t_base_type::TYPE_I64:
          result += " = 0";
          break;
        case t_base_type::TYPE_DOUBLE:
          result += " = (double)0";
          break;
        case t_base_type::TYPE_FLOAT:
          result += " = (float)0";
          break;
      }

    } else if (ttype->is_enum()) {
      result += " = 0";
    } else if (ttype->is_container()) {
      result += " = new " + type_name(ttype, false, true) + "()";
    } else {
      result += " = new " + type_name(ttype, false, true) + "()";
      ;
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
string t_java_deprecated_generator::function_signature(
    const t_function* tfunction, string prefix) {
  const t_type* ttype = tfunction->return_type();
  std::string result = type_name(ttype) + " " + prefix + tfunction->get_name() +
      "(" + argument_list(tfunction->get_paramlist()) + ") throws ";
  for (const t_field& x : get_elems(tfunction->exceptions())) {
    result += type_name(x.get_type(), false, false) + ", ";
  }
  result += "TException";
  return result;
}

/**
 * Renders a function signature of the form 'void name(args, resultHandler)'
 *
 * @params tfunction Function definition
 * @return String of rendered function definition
 */
string t_java_deprecated_generator::function_signature_async(
    const t_function* tfunction,
    string result_handler_symbol,
    bool use_base_method,
    string prefix) {
  std::string arglist = async_function_call_arglist(
      tfunction, result_handler_symbol, use_base_method, true);
  std::string result =
      prefix + "void " + tfunction->get_name() + "(" + arglist + ")";
  return result;
}

string t_java_deprecated_generator::async_function_call_arglist(
    const t_function* tfunc,
    string result_handler_symbol,
    bool /*use_base_method*/,
    bool include_types) {
  std::string arglist = "";
  if (tfunc->get_paramlist()->get_members().size() > 0) {
    arglist = argument_list(tfunc->get_paramlist(), include_types) + ", ";
  }

  if (include_types) {
    arglist += "AsyncMethodCallback ";
  }
  arglist += result_handler_symbol;

  return arglist;
}

/**
 * Renders a comma separated field list, with type names
 */
string t_java_deprecated_generator::argument_list(
    const t_struct* tstruct, bool include_types) {
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
    if (include_types) {
      result += type_name((*f_iter)->get_type()) + " ";
    }
    result += (*f_iter)->get_name();
  }
  return result;
}

string t_java_deprecated_generator::async_argument_list(
    const t_function* /*tfunct*/,
    const t_struct* tstruct,
    string result_handler_symbol,
    bool include_types) {
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
    if (include_types) {
      result += type_name((*f_iter)->get_type()) + " ";
    }
    result += (*f_iter)->get_name();
  }
  if (!first) {
    result += ", ";
  }
  if (include_types) {
    result += "AsyncMethodCallback ";
  }
  result += result_handler_symbol;
  return result;
}

/**
 * Converts the parse type to a C++ enum string for the given type.
 */
string t_java_deprecated_generator::type_to_enum(const t_type* type) {
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

/**
 * Applies the correct style to a string
 */
std::string t_java_deprecated_generator::get_cap_name(std::string name) {
  name[0] = toupper(name[0]);
  return name;
}

string t_java_deprecated_generator::constant_name(string name) {
  string constant_name;

  bool is_first = true;
  bool was_previous_char_upper = false;
  for (string::iterator iter = name.begin(); iter != name.end(); ++iter) {
    string::value_type character = (*iter);

    bool is_upper = isupper(character);

    if (is_upper && !is_first && !was_previous_char_upper) {
      constant_name += '_';
    }
    constant_name += toupper(character);

    is_first = false;
    was_previous_char_upper = is_upper;
  }

  return constant_name;
}

void t_java_deprecated_generator::generate_java_docstring_comment(
    ofstream& out, string contents) {
  generate_docstring_comment(out, "/**\n", " * ", contents, " */\n");
}

void t_java_deprecated_generator::generate_java_doc(
    ofstream& out, const t_field* field) {
  if (field->get_type()->is_enum()) {
    string combined_message =
        field->doc() + "\n@see " + get_enum_class_name(field->get_type());
    generate_java_docstring_comment(out, combined_message);
  } else {
    generate_java_doc(out, (t_named*)field);
  }
}

/**
 * Emits a JavaDoc comment if the provided object has a doc in Thrift
 */
void t_java_deprecated_generator::generate_java_doc(
    ofstream& out, const t_named* named_node) {
  if (named_node->has_doc()) {
    generate_java_docstring_comment(out, named_node->doc());
  }
}

/**
 * Emits a JavaDoc comment if the provided function object has a doc in Thrift
 */
void t_java_deprecated_generator::generate_java_doc(
    ofstream& out, const t_function* tfunction) {
  if (tfunction->has_doc()) {
    stringstream ss;
    ss << tfunction->doc();
    const vector<t_field*>& fields = tfunction->get_paramlist()->get_members();
    vector<t_field*>::const_iterator p_iter;
    for (p_iter = fields.begin(); p_iter != fields.end(); ++p_iter) {
      const t_field* p = *p_iter;
      ss << "\n@param " << p->get_name();
      if (p->has_doc()) {
        ss << " " << p->doc();
      }
    }
    generate_docstring_comment(out, "/**\n", " * ", ss.str(), " */\n");
  }
}

std::string t_java_deprecated_generator::isset_field_id(const t_field* field) {
  return "__" + upcase_string(field->get_name() + "_isset_id");
}

std::string t_java_deprecated_generator::generate_isset_check(
    const t_field* field) {
  return generate_isset_check(field->get_name());
}

std::string t_java_deprecated_generator::generate_isset_check(
    std::string field_name) {
  return "is" + get_cap_name("set") + get_cap_name(field_name) + "()";
}

std::string t_java_deprecated_generator::generate_setfield_check(
    const t_field* field) {
  return generate_setfield_check(field->get_name());
}

std::string t_java_deprecated_generator::generate_setfield_check(
    std::string field_name) {
  return "getSetField() == " + upcase_string(field_name);
}

void t_java_deprecated_generator::generate_isset_set(
    ofstream& out, const t_field* field) {
  if (!type_can_be_null(field->get_type())) {
    indent(out) << "set" << get_cap_name(field->get_name())
                << get_cap_name("isSet") << "(true);" << endl;
  }
}

std::string t_java_deprecated_generator::get_enum_class_name(
    const t_type* type) {
  string package = "";
  const t_program* program = type->program();
  if (program != nullptr && program != program_) {
    package = program->get_namespace(namespace_key_);
    if (package != "") {
      package += ".";
    }
  }
  return package + type->get_name();
}

void t_java_deprecated_generator::generate_struct_desc(
    ofstream& out, const t_struct* tstruct) {
  indent(out) << "private static final TStruct STRUCT_DESC = new TStruct(\""
              << tstruct->get_name() << "\");" << endl;
}

void t_java_deprecated_generator::generate_field_descs(
    ofstream& out, const t_struct* tstruct) {
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    indent(out) << "private static final TField "
                << constant_name((*m_iter)->get_name())
                << "_FIELD_DESC = new TField(\"" << (*m_iter)->get_name()
                << "\", " << type_to_enum((*m_iter)->get_type()) << ", "
                << "(short)" << (*m_iter)->get_key();
    if (is_field_sensitive(*m_iter)) {
      out << ", new HashMap<String, Object>() {{ put(\"sensitive\", true); }}";
    }
    out << ");" << endl;
  }
}

void t_java_deprecated_generator::generate_field_name_constants(
    ofstream& out, const t_struct* tstruct) {
  // Members are public for -java, private for -javabean
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    indent(out) << "public static final int "
                << upcase_string((*m_iter)->get_name()) << " = "
                << (*m_iter)->get_key() << ";" << endl;
  }
}

bool t_java_deprecated_generator::is_comparable(
    const t_type* type, vector<const t_type*>* enclosing) {
  type = type->get_true_type();

  if (type->is_base_type()) {
    return true;
  } else if (type->is_enum()) {
    return true;
  } else if (type->is_struct()) {
    vector<const t_type*> enclosing2;
    enclosing = enclosing ? enclosing : &enclosing2;
    for (auto iter = enclosing->begin(); iter != enclosing->end(); iter++) {
      if (*iter == type) {
        return false;
      }
    }
    enclosing->push_back(type);
    bool ret = struct_has_all_comparable_fields((t_struct*)type, enclosing);
    enclosing->pop_back();
    return ret;
  } else if (type->is_exception()) {
    // There's no particular reason this wouldn't work exactly the same
    // as it does for structs. I'm not sure we actually want exceptions
    // to be Comparable though: in addition to the fields we have, which
    // we know how to compare, they also have stack trace info etc.
    // inherited from Throwable, which we'd be ignoring. (OTOH, I suppose
    // we already ignore it for equals.) Let's leave it off for now; we
    // can always change this to `true` later if somebody has a use case.
    return false;
  } else if (type->is_map()) {
    return is_comparable(((t_map*)type)->get_key_type(), enclosing) &&
        is_comparable(((t_map*)type)->get_val_type(), enclosing);
  } else if (type->is_set()) {
    return is_comparable(((t_set*)type)->get_elem_type(), enclosing);
  } else if (type->is_list()) {
    return is_comparable(((t_list*)type)->get_elem_type(), enclosing);
  } else {
    throw std::runtime_error("Don't know how to handle this ttype");
  }
}

bool t_java_deprecated_generator::struct_has_all_comparable_fields(
    const t_struct* tstruct, vector<const t_type*>* enclosing) {
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    if (!is_comparable((*m_iter)->get_type(), enclosing)) {
      return false;
    }
  }
  return true;
}

bool t_java_deprecated_generator::type_has_naked_binary(const t_type* type) {
  type = type->get_true_type();

  if (type->is_base_type()) {
    return type->is_binary();
  } else if (type->is_enum()) {
    return false;
  } else if (type->is_struct() || type->is_exception()) {
    return false;
  } else if (type->is_map()) {
    return type_has_naked_binary(((t_map*)type)->get_key_type()) ||
        type_has_naked_binary(((t_map*)type)->get_val_type());
  } else if (type->is_set()) {
    return type_has_naked_binary(((t_set*)type)->get_elem_type());
  } else if (type->is_list()) {
    return type_has_naked_binary(((t_list*)type)->get_elem_type());
  } else {
    throw std::runtime_error("Don't know how to handle this ttype");
  }
}

bool t_java_deprecated_generator::struct_has_naked_binary_fields(
    const t_struct* tstruct) {
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    if (type_has_naked_binary((*m_iter)->get_type()))
      return true;
  }

  return false;
}

bool t_java_deprecated_generator::has_bit_vector(const t_struct* tstruct) {
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    if (!type_can_be_null((*m_iter)->get_type())) {
      return true;
    }
  }
  return false;
}

THRIFT_REGISTER_GENERATOR(java_deprecated, "Java Deprecated", "");

} // namespace compiler
} // namespace thrift
} // namespace apache
