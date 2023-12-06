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

#include <assert.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <thrift/compiler/generate/t_concat_generator.h>

namespace apache {
namespace thrift {
namespace compiler {

// static const std::string std::endl = "\n";  // avoid std::ostream <<
// std::endl flushes
static const std::string kFieldPrefix = "__thrift_";
static const std::string kStructInheritanceRootObjectName = "TBaseStruct";
static const std::string kExceptionInheritanceRootObjectName = "TBaseException";
static const std::string kSetPostfix = "_set";
static const std::string kToStringPostfix = "ToString";
static const std::string kFromStringPostfix = "FromString";

/**
 * Objective-C code generator.
 */
class t_cocoa_generator : public t_concat_generator {
 public:
  using t_concat_generator::t_concat_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    log_unexpected_ = options.find("log_unexpected") != options.end();
    validate_required_ = options.find("validate_required") != options.end();
    nullability_ = options.find("nullability") != options.end();
    simple_value_equality_ =
        options.find("simple_value_equality") != options.end();

    auto iter = options.find("import_path");
    if (iter != options.end()) {
      import_path_ = iter->second;
      if (import_path_.at(import_path_.length() - 1) != '/') {
        import_path_ += '/';
      }
    }

    out_dir_base_ = "gen-cocoa";
  }

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
  void generate_enum_from_string_function(const t_enum* tenum);
  void generate_struct(const t_structured* tstruct) override;
  void generate_xception(const t_structured* txception) override;
  void generate_service(const t_service* tservice) override;

  void print_const_value(
      std::ofstream& out,
      const std::string& name,
      const t_type* type,
      const t_const_value* value,
      bool defval = false,
      bool is_property = false);
  std::string render_const_value(
      std::ofstream& out,
      const t_type* type,
      const t_const_value* value,
      bool containerize_it = false);

  void generate_cocoa_struct(const t_struct* tstruct, bool is_exception);
  void generate_cocoa_struct_interface(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_exception = false);
  void generate_cocoa_struct_implementation(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_exception = false,
      bool is_result = false);
  void generate_cocoa_struct_initializer_signature(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_init_with_coder_method(
      std::ofstream& out, const t_structured* tstruct, bool is_exception);
  void generate_cocoa_struct_encode_with_coder_method(
      std::ofstream& out, const t_structured* tstruct, bool is_exception);
  void generate_cocoa_struct_hash_method(std::ofstream& out);
  void generate_cocoa_struct_is_equal_method(
      std::ofstream& out, const t_structured* tstruct, bool is_exception);
  void generate_cocoa_struct_field_accessor_declarations(
      std::ofstream& out,
      const t_structured* tstruct,
      bool is_declare_getter,
      bool is_declare_setter,
      bool is_declare_isset_getter,
      bool is_exception);
  void generate_cocoa_struct_field_accessor_implementations(
      std::ofstream& out, const t_structured* tstruct, bool is_exception);
  void generate_cocoa_struct_reader(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_result_writer(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_writer(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_validator(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_description(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_toDict(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_makeImmutable(
      std::ofstream& out, const t_structured* tstruct);
  void generate_cocoa_struct_mutableCopyWithZone(
      std::ofstream& out, const t_structured* tstruct);

  std::string function_result_helper_struct_type(const t_function* tfunction);
  std::string function_args_helper_struct_type(const t_function* tfunction);
  void generate_function_helpers(const t_function* tfunction);

  /**
   * Service-level generation functions
   */

  void generate_cocoa_service_protocol(
      std::ofstream& out, const t_service* tservice);
  void generate_cocoa_service_client_interface(
      std::ofstream& out, const t_service* tservice);
  void generate_cocoa_service_client_implementation(
      std::ofstream& out, const t_service* tservice);
  void generate_cocoa_service_server_interface(
      std::ofstream& out, const t_service* tservice);
  void generate_cocoa_service_server_implementation(
      std::ofstream& out, const t_service* tservice);
  void generate_cocoa_service_helpers(const t_service* tservice);
  void generate_service_client(const t_service* tservice);
  void generate_service_server(const t_service* tservice);
  void generate_process_function(
      const t_service* tservice, const t_function* tfunction);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field(
      std::ofstream& out, const t_field* tfield, const std::string& fieldName);

  void generate_deserialize_struct(
      std::ofstream& out, const t_type* tstruct, const std::string& fieldName);

  void generate_deserialize_container(
      std::ofstream& out, const t_type* ttype, const std::string& fieldName);

  void generate_deserialize_set_element(
      std::ofstream& out, const t_set* tset, const std::string& fieldName);

  void generate_deserialize_map_element(
      std::ofstream& out, const t_map* tmap, const std::string& fieldName);

  void generate_deserialize_list_element(
      std::ofstream& out, const t_list* tlist, const std::string& fieldName);

  void generate_serialize_field(
      std::ofstream& out, const t_field* tfield, const std::string& fieldName);

  void generate_serialize_struct(
      std::ofstream& out,
      const t_struct* tstruct,
      const std::string& fieldName);

  void generate_serialize_container(
      std::ofstream& out, const t_type* ttype, const std::string& fieldName);

  void generate_serialize_map_element(
      std::ofstream& out,
      const t_map* tmap,
      const std::string& iter,
      const std::string& map);

  void generate_serialize_set_element(
      std::ofstream& out, const t_set* tmap, const std::string& iter);

  void generate_serialize_list_element(
      std::ofstream& out,
      const t_list* tlist,
      const std::string& index,
      const std::string& listName);

  /**
   * Helper rendering functions
   */

  std::string cocoa_prefix();
  std::string cocoa_imports();
  std::string cocoa_thrift_imports();
  std::string custom_thrift_marker();
  std::string type_name(const t_type* ttype, bool class_ref = false);
  std::string base_type_name(t_base_type* tbase);
  std::string declare_field(const t_field* tfield);
  std::string declare_property(const t_field* tfield);
  std::string function_signature(const t_function* tfunction);
  std::string argument_list(const t_paramlist& tparamlist);
  std::string type_to_enum(const t_type* ttype);
  std::string format_string_for_type(const t_type* type);
  std::string call_field_setter(
      const t_field* tfield, const std::string& fieldName);
  std::string containerize(const t_type* ttype, const std::string& fieldName);
  std::string decontainerize(
      const t_field* tfield, const std::string& fieldName);
  std::string get_cocoa_property_name(const t_field* tfield);

  bool type_can_be_null(const t_type* ttype) {
    ttype = ttype->get_true_type();

    return ttype->is_container() || ttype->is_struct() ||
        ttype->is_exception() || ttype->is_string_or_binary();
  }

 private:
  std::string cocoa_prefix_;
  std::string constants_declarations_;

  /**
   * File streams
   */

  std::ofstream f_header_;
  std::ofstream f_impl_;

  bool log_unexpected_;
  bool validate_required_;
  bool nullability_;
  bool simple_value_equality_;
  std::string import_path_;
};

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 */
void t_cocoa_generator::init_generator() {
  // Make output directory
  boost::filesystem::create_directory(get_out_dir());
  cocoa_prefix_ = program_->get_namespace("cocoa");

  // we have a .h header file...
  std::string f_header_name = get_program()->name() + ".h";
  std::string f_header_fullname = get_out_dir() + f_header_name;
  f_header_.open(f_header_fullname.c_str());

  f_header_ << autogen_comment() << std::endl;

  f_header_ << custom_thrift_marker() << cocoa_imports()
            << cocoa_thrift_imports();
  if (nullability_) {
    f_header_ << "NS_ASSUME_NONNULL_BEGIN\n\n";
  }

  // ...and a .m implementation file
  std::string f_impl_name = get_out_dir() + get_program()->name() + ".m";
  f_impl_.open(f_impl_name.c_str());

  f_impl_ << autogen_comment() << std::endl;

  f_impl_ << custom_thrift_marker() << cocoa_imports() << cocoa_thrift_imports()
          << "#import \"" << f_header_name << "\"" << std::endl
          << std::endl;
}

/**
 * Prints standard Cocoa imports
 *
 * @return List of imports for Cocoa libraries
 */
std::string t_cocoa_generator::cocoa_imports() {
  return std::string() + "#import <Foundation/Foundation.h>\n" + "\n";
}

/**
 * Add a marker for this generator
 *
 */
std::string t_cocoa_generator::custom_thrift_marker() {
  return std::string("/**\n") +
      std::string(
             "* @"
             "generated by Thrift\n") +
      "*/\n\n";
}

/**
 * Prints thrift runtime imports
 *
 * @return List of imports necessary for thrift runtime
 */
std::string t_cocoa_generator::cocoa_thrift_imports() {
  std::string systemImports[] = {
      "TProtocol",
      "TApplicationException",
      "TProtocolException",
      "TProtocolUtil",
      "TProcessor",
      "TObjective-C",
      "TBase",
      kStructInheritanceRootObjectName,
      kExceptionInheritanceRootObjectName,
  };

  std::string result = "";
  for (const auto& systemImport : systemImports) {
    if (import_path_ == "") {
      result += "#import \"" + systemImport + ".h\"\n";
    } else {
      result += "#import <" + import_path_ + systemImport + ".h>\n";
    }
  }

  // Include other Thrift includes
  const std::vector<t_program*>& includes =
      program_->get_includes_for_codegen();
  for (size_t i = 0; i < includes.size(); ++i) {
    result += "#import \"" + includes[i]->name() + ".h\"" + "\n";
  }
  result += "\n";

  return result;
}

/**
 * Finish up generation.
 */
void t_cocoa_generator::close_generator() {
  // stick our constants declarations at the end of the header file
  // since they refer to things we are defining.
  f_header_ << constants_declarations_ << std::endl;
  if (nullability_) {
    f_header_ << std::endl << "NS_ASSUME_NONNULL_END\n" << std::endl;
  }
}

/**
 * Generates a typedef. This is just a simple 1-liner in objective-c
 *
 * @param ttypedef The type definition
 */
void t_cocoa_generator::generate_typedef(const t_typedef* ttypedef) {
  f_header_ << indent() << "typedef " << type_name(ttypedef->get_type()) << " "
            << cocoa_prefix_ << ttypedef->name() << ";" << std::endl
            << std::endl;
}

/**
 * Generates code for an enumerated type. In Objective-C, this is
 * essentially the same as the thrift definition itself, using the
 * enum keyword in Objective-C.  For namespace purposes, the name of
 * the enum plus an underscore is prefixed onto each element.
 *
 * @param tenum The enumeration
 */
void t_cocoa_generator::generate_enum(const t_enum* tenum) {
  f_header_ << indent() << "typedef NS_ENUM(int, " << cocoa_prefix_
            << tenum->name() << ") {" << std::endl;
  indent_up();

  std::vector<t_enum_value*> constants = tenum->get_enum_values();
  std::vector<t_enum_value*>::iterator c_iter;
  bool first = true;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    if (first) {
      first = false;
    } else {
      f_header_ << "," << std::endl;
    }
    f_header_ << indent() << cocoa_prefix_ << tenum->name() << "_"
              << (*c_iter)->name();
    f_header_ << " = " << (*c_iter)->get_value();
  }

  indent_down();
  f_header_ << std::endl << "};" << std::endl << std::endl;

  // toString

  const std::string toStringFunctionDeclaration = std::string("NSString* ") +
      cocoa_prefix_ + tenum->name() + kToStringPostfix + "(const " +
      cocoa_prefix_ + tenum->name() + " value)";

  f_header_ << indent() << toStringFunctionDeclaration << ";" << std::endl
            << std::endl;

  // implementation:
  f_impl_ << indent() << toStringFunctionDeclaration << std::endl
          << "{" << std::endl;
  indent_up();

  f_impl_ << indent() << "switch(value) {" << std::endl;
  indent_up();
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    std::string itemName =
        cocoa_prefix_ + tenum->name() + "_" + (*c_iter)->name();
    f_impl_ << indent() << "case " << itemName
            << ": return @\"" + itemName + "\";" << std::endl;
  }
  indent_down();
  f_impl_ << indent() << "}" << std::endl;
  f_impl_ << indent() << "return [NSString stringWithFormat:@\""
          << cocoa_prefix_ << tenum->name() << "_"
          << "%d\", (int)value];" << std::endl;
  indent_down();
  f_impl_ << indent() << "}" << std::endl;
  f_impl_ << std::endl;

  // fromString

  if (tenum->has_annotation("cocoa.enum_conversion")) {
    generate_enum_from_string_function(tenum);
  }
}

void t_cocoa_generator::generate_enum_from_string_function(
    const t_enum* tenum) {
  const std::string fromStringFunctionDeclaration = cocoa_prefix_ +
      tenum->name() + " " + cocoa_prefix_ + tenum->name() + kFromStringPostfix +
      "(NSString *str, int fallbackValue)";

  f_header_ << indent() << fromStringFunctionDeclaration << ";" << std::endl
            << std::endl;

  // implementation:
  f_impl_ << indent() << fromStringFunctionDeclaration << std::endl
          << "{" << std::endl;

  indent_up();

  f_impl_ << indent()
          << "static NSDictionary<NSString *, NSNumber *> *mapping = nil;"
          << std::endl;
  f_impl_ << indent() << "static dispatch_once_t onceToken;" << std::endl;
  f_impl_ << indent() << "dispatch_once(&onceToken, ^{" << std::endl;

  indent_up();

  f_impl_ << indent() << "mapping = @{" << std::endl;

  indent_up();

  for (auto constant = tenum->get_enum_values().begin();
       constant != tenum->get_enum_values().end();
       ++constant) {
    std::string namespacedConstantName =
        cocoa_prefix_ + tenum->name() + "_" + (*constant)->name();
    f_impl_ << indent() << "@\"" << namespacedConstantName << "\": @("
            << namespacedConstantName << ")," << std::endl;
  }

  indent_down();

  f_impl_ << indent() << "};" << std::endl;

  indent_down();

  f_impl_ << indent() << "});" << std::endl;
  f_impl_ << indent()
          << "return mapping[str] ? [mapping[str] intValue] : fallbackValue;"
          << std::endl;

  indent_down();

  f_impl_ << indent() << "}" << std::endl;
  f_impl_ << std::endl;
}

/**
 * Generates a class that holds all the constants.  Primitive values
 * could have been placed outside this class, but I just put
 * everything in for consistency.
 */
void t_cocoa_generator::generate_consts(std::vector<t_const*> consts) {
  // don't create constants class if no constants are present
  if (consts.empty()) {
    return;
  }

  std::ostringstream const_interface;
  std::string constants_class_name =
      cocoa_prefix_ + get_program()->name() + "Constants";

  const_interface << "@interface " << constants_class_name << " : "
                  << kStructInheritanceRootObjectName << " ";
  scope_up(const_interface);
  scope_down(const_interface);

  // getter method for each constant defined.
  for (const auto* tconst : consts) {
    std::string name = tconst->name();
    const t_type* type = tconst->type();
    const_interface << "+ (" << type_name(type) << ") " << name << ";"
                    << std::endl;
  }

  const_interface << "@end";

  // this gets spit into the header file in ::close_generator
  constants_declarations_ = const_interface.str();

  // static variables in the .m hold all constant values
  for (const auto* tconst : consts) {
    std::string name = tconst->name();
    const t_type* type = tconst->type();
    f_impl_ << "static " << type_name(type) << " " << cocoa_prefix_ << name;
    if (!type->is_container() && !type->is_struct()) {
      f_impl_ << " = " << render_const_value(f_impl_, type, tconst->value());
    }
    f_impl_ << ";" << std::endl;
  }
  f_impl_ << std::endl;

  f_impl_ << "@implementation " << constants_class_name << std::endl;

  // check if initialize method is needed to initialize complex constants
  // when the class is initialized
  bool should_have_initialize =
      std::any_of(consts.begin(), consts.end(), [](const t_const* c) {
        return c->type()->is_container() || c->type()->is_struct();
      });
  if (should_have_initialize) {
    f_impl_ << "+ (void) initialize ";
    scope_up(f_impl_);

    for (const auto* tconst : consts) {
      if (tconst->type()->is_container() || tconst->type()->is_struct()) {
        print_const_value(
            f_impl_,
            cocoa_prefix_ + tconst->name(),
            tconst->type(),
            tconst->value(),
            false,
            false);
        f_impl_ << ";" << std::endl;
      }
    }
    scope_down(f_impl_);
  }

  // getter method for each constant
  for (const auto* tconst : consts) {
    std::string name = tconst->name();
    const t_type* type = tconst->type();
    f_impl_ << "+ (" << type_name(type) << ") " << name;
    scope_up(f_impl_);
    indent(f_impl_) << "return " << cocoa_prefix_ << name << ";" << std::endl;
    scope_down(f_impl_);
  }

  f_impl_ << "@end" << std::endl << std::endl;
}

/**
 * Generates a struct definition for a thrift data type. This is a class
 * with protected data members, read(), write(), and getters and setters.
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_struct(const t_structured* tstruct) {
  generate_cocoa_struct_interface(f_header_, tstruct, false);
  generate_cocoa_struct_implementation(f_impl_, tstruct, false);
}

/**
 * Exceptions are structs, but they inherit from NSException
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_xception(const t_structured* txception) {
  generate_cocoa_struct_interface(f_header_, txception, true);
  generate_cocoa_struct_implementation(f_impl_, txception, true);
}

/**
 * Generate the interface for a struct
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_cocoa_struct_interface(
    std::ofstream& out, const t_structured* tstruct, bool is_exception) {
  out << "@interface " << cocoa_prefix_ << tstruct->name() << " : ";

  if (is_exception) {
    out << kExceptionInheritanceRootObjectName;
  } else {
    out << kStructInheritanceRootObjectName;
  }
  out << " <TBase, NSCoding> ";

  scope_up(out);

  // members are protected.  this is redundant, but explicit.
  //  f_header_ << std::endl << "@protected:" << std::endl;

  // member variables
  for (const auto& field : tstruct->fields()) {
    out << indent() << declare_field(&field) << std::endl;
  }

  if (tstruct->has_fields()) {
    out << std::endl;
    // isset fields
    for (const auto& field : tstruct->fields()) {
      indent(out) << "BOOL " << kFieldPrefix << field.name() << kSetPostfix
                  << ";" << std::endl;
    }
  }

  scope_down(out);
  out << std::endl;

  // properties
  if (tstruct->has_fields()) {
    // out << "#if TARGET_OS_IPHONE || (MAC_OS_X_VERSION_MAX_ALLOWED >=
    // MAC_OS_X_VERSION_10_5)" << std::endl;
    for (const auto& field : tstruct->fields()) {
      out << indent() << declare_property(&field) << std::endl;
    }
    // out << "#endif" << std::endl;
    out << std::endl;
  }

  // default initializer
  out << indent() << "- (id) init NS_DESIGNATED_INITIALIZER;" << std::endl;

  // although we conform to NSCoding, we need to declare -initWithCoder:
  // explicitly, because the implementation invokes super, and is thus
  // a designated initializer
  out << indent()
      << "- (id) initWithCoder:(NSCoder *)decoder NS_DESIGNATED_INITIALIZER;"
      << std::endl;

  // initializer for all fields
  if (tstruct->has_fields()) {
    generate_cocoa_struct_initializer_signature(out, tstruct);
    out << " NS_DESIGNATED_INITIALIZER;" << std::endl;
  }
  // read and write
  out << "- (void) read: (id <TProtocol>) inProtocol;" << std::endl;
  out << "- (void) write: (id <TProtocol>) outProtocol;" << std::endl;
  // validator
  out << "- (void) validate;" << std::endl << std::endl;

  // getters and setters
  generate_cocoa_struct_field_accessor_declarations(
      out, tstruct, false, false, true, is_exception);

  out << "@end" << std::endl << std::endl;
}

/**
 * Generate signature for initializer of struct with a parameter for
 * each field.
 */
void t_cocoa_generator::generate_cocoa_struct_initializer_signature(
    std::ofstream& out, const t_structured* tstruct) {
  indent(out) << "- (id) initWith";
  bool first = true;
  for (const auto& field : tstruct->fields()) {
    if (first) {
      out << capitalize(field.name());
    } else {
      out << " " << field.name();
    }
    out << ": (";
    if (nullability_ && type_can_be_null((field.get_type()))) {
      out
          << (field.get_req() == t_field::e_req::required ? "nonnull "
                                                          : "nullable ");
    }
    out << type_name(field.get_type()) << ") " << field.name();
    first = false;
  }
}

/**
 * Generate getter and setter declarations for all fields, plus an
 * IsSet getter.
 */
void t_cocoa_generator::generate_cocoa_struct_field_accessor_declarations(
    std::ofstream& out,
    const t_structured* tstruct,
    bool is_declare_getter,
    bool is_declare_setter,
    bool is_declare_isset_getter,
    bool is_exception) {
  (void)is_exception;
  for (const auto& field : tstruct->fields()) {
    // out << indent() << "#if !__has_feature(objc_arc)" << std::endl
    if (is_declare_getter) {
      out << indent() << "- (" << type_name(field.get_type()) << ") "
          << decapitalize(get_cocoa_property_name(&field)) << ";" << std::endl;
    }

    if (is_declare_setter) {
      out << indent() << "- (void) set"
          << capitalize(get_cocoa_property_name(&field)) << ": ("
          << type_name(field.get_type()) << ") " << field.name() << ";"
          << std::endl;
    }
    // out << indent() << "#endif" << std::endl;
    if (is_declare_isset_getter) {
      out << indent() << "- (BOOL) " << field.name() << "IsSet"
          << ";" << std::endl;
    }
  }
}

/**
 * Generate the initWithCoder method for this struct so it's compatible with
 * the NSCoding protocol
 */
void t_cocoa_generator::generate_cocoa_struct_init_with_coder_method(
    std::ofstream& out, const t_structured* tstruct, bool is_exception) {
  indent(out) << "- (id) initWithCoder: (NSCoder *) decoder" << std::endl;
  scope_up(out);
  if (is_exception) {
    // NSExceptions conform to NSCoding, so we can call super
    out << indent() << "self = [super initWithCoder: decoder];" << std::endl;
  } else {
    out << indent() << "self = [super init];" << std::endl;
  }

  for (const auto& field : tstruct->fields()) {
    const t_type* t = field.type()->get_true_type();
    out << indent() << "if ([decoder containsValueForKey: @\"" << field.name()
        << "\"])" << std::endl;
    scope_up(out);
    out << indent() << kFieldPrefix << field.name() << " = ";
    if (type_can_be_null(t)) {
      out << "[[decoder decodeObjectForKey: @\"" << field.name()
          << "\"] retain_stub];" << std::endl;
    } else if (t->is_enum()) {
      out << "[decoder decodeIntForKey: @\"" << field.name() << "\"];"
          << std::endl;
    } else {
      t_base_type::t_base tbase = ((t_base_type*)t)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_BOOL:
          out << "[decoder decodeBoolForKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_BYTE:
          out << "[decoder decodeIntForKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_I16:
          out << "[decoder decodeIntForKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_I32:
          out << "[decoder decodeInt32ForKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_I64:
          out << "[decoder decodeInt64ForKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_DOUBLE:
          out << "[decoder decodeDoubleForKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        default:
          throw std::runtime_error(
              "compiler error: don't know how to decode thrift type: " +
              t_base_type::t_base_name(tbase));
      }
    }
    out << indent() << kFieldPrefix << field.name() << kSetPostfix << " = YES;"
        << std::endl;
    scope_down(out);
  }

  out << indent() << "return self;" << std::endl;
  scope_down(out);
  out << std::endl;
}

/**
 * Generate the encodeWithCoder method for this struct so it's compatible with
 * the NSCoding protocol
 */
void t_cocoa_generator::generate_cocoa_struct_encode_with_coder_method(
    std::ofstream& out, const t_structured* tstruct, bool is_exception) {
  indent(out) << "- (void) encodeWithCoder: (NSCoder *) encoder" << std::endl;
  scope_up(out);
  if (is_exception) {
    // NSExceptions conform to NSCoding, so we can call super
    out << indent() << "[super encodeWithCoder: encoder];" << std::endl;
  }

  for (const auto& field : tstruct->fields()) {
    const t_type* t = field.type()->get_true_type();
    out << indent() << "if (" << kFieldPrefix << field.name() << kSetPostfix
        << ")" << std::endl;
    scope_up(out);
    // out << indent() << kFieldPrefix << field.name() << " = ";
    if (type_can_be_null(t)) {
      out << indent() << "[encoder encodeObject: " << kFieldPrefix
          << field.name() << " forKey: @\"" << field.name() << "\"];"
          << std::endl;
    } else if (t->is_enum()) {
      out << indent() << "[encoder encodeInt: " << kFieldPrefix << field.name()
          << " forKey: @\"" << field.name() << "\"];" << std::endl;
    } else {
      t_base_type::t_base tbase = ((t_base_type*)t)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_BOOL:
          out << indent() << "[encoder encodeBool: " << kFieldPrefix
              << field.name() << " forKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_BYTE:
          out << indent() << "[encoder encodeInt: " << kFieldPrefix
              << field.name() << " forKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_I16:
          out << indent() << "[encoder encodeInt: " << kFieldPrefix
              << field.name() << " forKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_I32:
          out << indent() << "[encoder encodeInt32: " << kFieldPrefix
              << field.name() << " forKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_I64:
          out << indent() << "[encoder encodeInt64: " << kFieldPrefix
              << field.name() << " forKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        case t_base_type::TYPE_DOUBLE:
          out << indent() << "[encoder encodeDouble: " << kFieldPrefix
              << field.name() << " forKey: @\"" << field.name() << "\"];"
              << std::endl;
          break;
        default:
          throw std::runtime_error(
              "compiler error: don't know how to encode thrift type: " +
              t_base_type::t_base_name(tbase));
      }
    }
    scope_down(out);
  }

  scope_down(out);
  out << std::endl;
}

/**
 * Generate the hash method for this struct
 */
void t_cocoa_generator::generate_cocoa_struct_hash_method(std::ofstream& out) {
  indent(out) << "- (NSUInteger) hash" << std::endl;
  scope_up(out);
  out << indent() << "return 0;" << std::endl;
  scope_down(out);
  out << std::endl;
}

/**
 * Generate the isEqual method for this struct
 */
void t_cocoa_generator::generate_cocoa_struct_is_equal_method(
    std::ofstream& out, const t_structured* tstruct, bool is_exception) {
  indent(out) << "- (BOOL) isEqual: (id) anObject" << std::endl;
  scope_up(out);

  indent(out) << "if (self == anObject) {" << std::endl;
  indent_up();
  indent(out) << "return YES;" << std::endl;
  indent_down();
  indent(out) << "}" << std::endl;

  std::string class_name = cocoa_prefix_ + tstruct->name();

  if (is_exception) {
    indent(out) << "if (![super isEqual:anObject]) {" << std::endl;
    indent_up();
    indent(out) << "return NO;" << std::endl;
    indent_down();
    indent(out) << "}" << std::endl << std::endl;
  } else {
    indent(out) << "if (![anObject isKindOfClass:[" << class_name
                << " class]]) {" << std::endl;
    indent_up();
    indent(out) << "return NO;" << std::endl;
    indent_down();
    indent(out) << "}" << std::endl;
  }

  out << indent() << "return [[self toDict] isEqual:[anObject toDict]];"
      << std::endl;
  scope_down(out);
  out << std::endl;
}

/**
 * Generate struct implementation.
 *
 * @param tstruct      The struct definition
 * @param is_exception Is this an exception?
 * @param is_result    If this is a result it needs a different writer
 */
void t_cocoa_generator::generate_cocoa_struct_implementation(
    std::ofstream& out,
    const t_structured* tstruct,
    bool is_exception,
    bool is_result) {
  indent(out) << "@implementation " << cocoa_prefix_ << tstruct->name()
              << std::endl
              << std::endl;

  // exceptions need to call the designated initializer on NSException
  if (is_exception) {
    out << indent() << "- (instancetype) init" << std::endl;
    scope_up(out);
    out << indent() << "return [super initWithName: @\"" << cocoa_prefix_
        << tstruct->name() << "\" reason: @\"unknown\" userInfo: nil];"
        << std::endl;
    scope_down(out);
    out << std::endl;
  } else {
    // struct

    // default initializer
    // setup instance variables with default values
    indent(out) << "- (instancetype) init" << std::endl;
    scope_up(out);
    indent(out) << "self = [super init];" << std::endl;
    size_t num_members_with_values = 0;
    for (const auto& field : tstruct->fields()) {
      if (field.default_value() != nullptr) {
        if (num_members_with_values == 0) {
          // out << "#if TARGET_OS_IPHONE || (MAC_OS_X_VERSION_MAX_ALLOWED >=
          // MAC_OS_X_VERSION_10_5)" << std::endl;
        }
        num_members_with_values++;
        print_const_value(
            out,
            "self." + field.name(),
            field.type()->get_true_type(),
            field.default_value(),
            false,
            true);
      }
    }
    if (num_members_with_values != 0) {
      // out << "#endif" << std::endl;
    }

    indent(out) << "return self;" << std::endl;
    scope_down(out);
    out << std::endl;
  }

  // initializer with all fields as params
  if (tstruct->has_fields()) {
    generate_cocoa_struct_initializer_signature(out, tstruct);
    out << std::endl;
    scope_up(out);
    if (is_exception) {
      out << indent() << "self = [self init];" << std::endl;
    } else {
      out << indent() << "self = [super init];" << std::endl;
    }

    for (const auto& field : tstruct->fields()) {
      const t_type* t = field.type()->get_true_type();
      out << indent() << kFieldPrefix << field.name() << " = ";
      if (type_can_be_null(t)) {
        // out << "[" << field.name() << " retain_stub];" << std::endl;
        out << field.name() << ";" << std::endl;
      } else {
        out << field.name() << ";" << std::endl;
      }
      out << indent() << kFieldPrefix << field.name() << kSetPostfix
          << " = YES;" << std::endl;
    }

    out << indent() << "return self;" << std::endl;
    scope_down(out);
    out << std::endl;
  }

  // initWithCoder for NSCoding
  generate_cocoa_struct_init_with_coder_method(out, tstruct, is_exception);
  // encodeWithCoder for NSCoding
  generate_cocoa_struct_encode_with_coder_method(out, tstruct, is_exception);

  // hash and isEqual for NSObject
  if (simple_value_equality_) {
    generate_cocoa_struct_hash_method(out);
    generate_cocoa_struct_is_equal_method(out, tstruct, is_exception);
  }

  // dealloc
  // if (!members.empty()) {
  //   out << "- (void) dealloc" << std::endl;
  //   scope_up(out);
  //   for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
  //     const t_type* t = (*m_iter)->get_type()->get_true_type();
  //     if (type_can_be_null(t)) {
  //       indent(out) << "[" << kFieldPrefix << (*m_iter)->name() << "
  //       release_stub];" << std::endl;
  //     }
  //   }
  //   // out << indent() << "[super dealloc_stub];" << std::endl;
  //   scope_down(out);
  //   out << std::endl;
  // }

  // the rest of the methods
  generate_cocoa_struct_field_accessor_implementations(
      out, tstruct, is_exception);
  generate_cocoa_struct_reader(out, tstruct);
  if (is_result) {
    generate_cocoa_struct_result_writer(out, tstruct);
  } else {
    generate_cocoa_struct_writer(out, tstruct);
  }
  generate_cocoa_struct_validator(out, tstruct);
  generate_cocoa_struct_description(out, tstruct);
  generate_cocoa_struct_toDict(out, tstruct);
  generate_cocoa_struct_makeImmutable(out, tstruct);
  generate_cocoa_struct_mutableCopyWithZone(out, tstruct);

  out << "@end" << std::endl << std::endl;
}

/**
 * Generates a function to read all the fields of the struct.
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_cocoa_struct_reader(
    std::ofstream& out, const t_structured* tstruct) {
  out << "- (void) read: (id <TProtocol>) inProtocol" << std::endl;
  scope_up(out);

  // Declare stack tmp variables
  indent(out) << "NSString * fieldName;" << std::endl;
  indent(out) << "int fieldType;" << std::endl;
  indent(out) << "int fieldID;" << std::endl;
  out << std::endl;

  indent(out) << "[inProtocol readStructBeginReturningName: NULL];"
              << std::endl;

  // Loop over reading in fields
  indent(out) << "while (true)" << std::endl;
  scope_up(out);

  // Read beginning field marker
  indent(out) << "[inProtocol readFieldBeginReturningName: &fieldName type: "
                 "&fieldType fieldID: &fieldID];"
              << std::endl;

  // Check for field STOP marker and break
  indent(out) << "if (fieldType == TType_STOP) { " << std::endl;
  indent_up();
  indent(out) << "break;" << std::endl;
  indent_down();
  indent(out) << "}" << std::endl;

  // Switch statement on the field we are reading
  indent(out) << "switch (fieldID)" << std::endl;

  scope_up(out);

  // Generate deserialization code for known cases
  for (const auto& field : tstruct->fields()) {
    indent(out) << "case " << field.id() << ":" << std::endl;
    indent_up();
    indent(out) << "if (fieldType == " << type_to_enum(field.get_type())
                << ") {" << std::endl;
    indent_up();

    generate_deserialize_field(out, &field, "fieldValue");
    indent(out) << call_field_setter(&field, "fieldValue") << std::endl;
    // if this is an allocated field, release it since the struct
    // is now retaining it
    if (type_can_be_null(field.get_type())) {
      // deserialized strings are autorelease, so don't release them
      if (!field.type()->get_true_type()->is_string_or_binary()) {
        indent(out) << "[fieldValue release_stub];" << std::endl;
      }
    }

    indent_down();
    out << indent() << "} else {" << std::endl;
    if (log_unexpected_) {
      out << indent()
          << "  NSLog(@\"%s: field ID %i has unexpected type %i.  Skipping.\", "
             "__PRETTY_FUNCTION__, fieldID, fieldType);"
          << std::endl;
    }
    out << indent()
        << "  [TProtocolUtil skipType: fieldType onProtocol: inProtocol];"
        << std::endl
        << indent() << "}" << std::endl
        << indent() << "break;" << std::endl;
    indent_down();
  }

  // In the default case we skip the field
  out << indent() << "default:" << std::endl;
  if (log_unexpected_) {
    out << indent()
        << "  NSLog(@\"%s: unexpected field ID %i with type %i.  Skipping.\", "
           "__PRETTY_FUNCTION__, fieldID, fieldType);"
        << std::endl;
  }
  out << indent()
      << "  [TProtocolUtil skipType: fieldType onProtocol: inProtocol];"
      << std::endl
      << indent() << "  break;" << std::endl;

  scope_down(out);

  // Read field end marker
  indent(out) << "[inProtocol readFieldEnd];" << std::endl;

  scope_down(out);

  out << indent() << "[inProtocol readStructEnd];" << std::endl;

  // performs various checks (e.g. check that all required fields are set)
  if (validate_required_) {
    out << indent() << "[self validate];" << std::endl;
  }

  indent_down();
  out << indent() << "}" << std::endl << std::endl;
}

/**
 * Generates a function to write all the fields of the struct
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_cocoa_struct_writer(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "- (void) write: (id <TProtocol>) outProtocol {"
      << std::endl;
  indent_up();

  const std::string& name = tstruct->name();
  out << indent() << "[outProtocol writeStructBeginWithName: @\"" << name
      << "\"];" << std::endl;

  for (const auto& field : tstruct->fields()) {
    out << indent() << "if (" << kFieldPrefix << field.name() << kSetPostfix
        << ") {" << std::endl;
    indent_up();
    bool null_allowed = type_can_be_null(field.get_type());
    if (null_allowed) {
      out << indent() << "if (" << kFieldPrefix << field.name() << " != nil) {"
          << std::endl;
      indent_up();
    }

    indent(out) << "[outProtocol writeFieldBeginWithName: @\"" << field.name()
                << "\" type: " << type_to_enum(field.get_type())
                << " fieldID: " << field.id() << "];" << std::endl;

    // Write field contents
    generate_serialize_field(out, &field, kFieldPrefix + field.name());

    // Write field closer
    indent(out) << "[outProtocol writeFieldEnd];" << std::endl;

    if (null_allowed) {
      scope_down(out);
    }
    scope_down(out);
  }
  // Write the struct std::map
  out << indent() << "[outProtocol writeFieldStop];" << std::endl
      << indent() << "[outProtocol writeStructEnd];" << std::endl;

  indent_down();
  out << indent() << "}" << std::endl << std::endl;
}

/**
 * Generates a function to write all the fields of the struct, which
 * is a function result. These fields are only written if they are
 * set, and only one of them can be set at a time.
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_cocoa_struct_result_writer(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "- (void) write: (id <TProtocol>) outProtocol {"
      << std::endl;
  indent_up();

  const std::string& name = tstruct->name();

  out << indent() << "[outProtocol writeStructBeginWithName: @\"" << name
      << "\"];" << std::endl;

  bool first = true;
  for (const auto& field : tstruct->fields()) {
    if (first) {
      first = false;
      out << std::endl << indent() << "if ";
    } else {
      out << " else if ";
    }

    out << "(" << kFieldPrefix << field.name() << kSetPostfix << ") {"
        << std::endl;
    indent_up();

    bool null_allowed = type_can_be_null(field.get_type());
    if (null_allowed) {
      out << indent() << "if (" << kFieldPrefix << field.name() << " != nil) {"
          << std::endl;
      indent_up();
    }

    indent(out) << "[outProtocol writeFieldBeginWithName: @\"" << field.name()
                << "\" type: " << type_to_enum(field.get_type())
                << " fieldID: " << field.id() << "];" << std::endl;

    // Write field contents
    generate_serialize_field(out, &field, kFieldPrefix + field.name());

    // Write field closer
    indent(out) << "[outProtocol writeFieldEnd];" << std::endl;

    if (null_allowed) {
      indent_down();
      indent(out) << "}" << std::endl;
    }

    indent_down();
    indent(out) << "}";
  }
  // Write the struct std::map
  out << std::endl
      << indent() << "[outProtocol writeFieldStop];" << std::endl
      << indent() << "[outProtocol writeStructEnd];" << std::endl;

  indent_down();
  out << indent() << "}" << std::endl << std::endl;
}

/**
 * Generates a function to perform various checks
 * (e.g. check that all required fields are set)
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_cocoa_struct_validator(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "- (void) validate {" << std::endl;
  indent_up();

  out << indent() << "// check for required fields" << std::endl;
  for (const auto& field : tstruct->fields()) {
    if (field.get_req() == t_field::e_req::required) {
      out << indent() << "if (!" << kFieldPrefix << field.name() << kSetPostfix
          << ") {" << std::endl
          << indent()
          << "  @throw [TProtocolException exceptionWithName: "
             "@\"TProtocolException\""
          << std::endl
          << indent()
          << "                             reason: @\"Required field '"
          << field.name() << "' is not set.\"];" << std::endl
          << indent() << "}" << std::endl;
    }
  }

  indent_down();
  out << indent() << "}" << std::endl << std::endl;
}

/**
 * Generate property accessor methods for all fields in the struct.
 * getter, setter, isset getter.
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_cocoa_struct_field_accessor_implementations(
    std::ofstream& out, const t_structured* tstruct, bool is_exception) {
  (void)is_exception;
  for (const auto& field : tstruct->fields()) {
    const t_type* type = field.type()->get_true_type();
    const std::string& field_name = field.name();
    std::string getter_selector = get_cocoa_property_name(&field);
    std::string cap_name = getter_selector;
    cap_name[0] = toupper(cap_name[0]);

    // Simple getter
    indent(out) << "- (" << type_name(type) << ") ";
    out << getter_selector << " {" << std::endl;
    indent_up();
    if (!type_can_be_null(type)) {
      indent(out) << "return " << kFieldPrefix << field_name << ";"
                  << std::endl;
    } else {
      indent(out) << "return " << kFieldPrefix << field_name << ";"
                  << std::endl;
    }
    indent_down();
    indent(out) << "}" << std::endl << std::endl;

    // Simple setter
    indent(out) << "- (void) set" << cap_name << ": (" << type_name(type)
                << ") " << field_name << " {" << std::endl;
    indent_up();
    indent(out) << "[self throwExceptionIfImmutable];" << std::endl;
    if (!type_can_be_null(type)) {
      indent(out) << kFieldPrefix << field_name << " = " << field_name << ";"
                  << std::endl;
    } else {
      // indent(out) << "[" << field_name << " retain_stub];" << std::endl;
      // indent(out) << "[" << kFieldPrefix << field_name << " release_stub];"
      // << std::endl;
      indent(out) << kFieldPrefix << field_name << " = " << field_name << ";"
                  << std::endl;
    }
    indent(out) << kFieldPrefix << field_name << kSetPostfix << " = YES;"
                << std::endl;
    indent_down();
    indent(out) << "}" << std::endl << std::endl;

    // IsSet
    indent(out) << "- (BOOL) " << field_name << "IsSet {" << std::endl;
    indent_up();
    indent(out) << "return " << kFieldPrefix << field_name << kSetPostfix << ";"
                << std::endl;
    indent_down();
    indent(out) << "}" << std::endl << std::endl;

    // Unsetter - do we need this?
    indent(out) << "- (void) unset" << cap_name << " {" << std::endl;
    indent_up();
    if (type_can_be_null(type)) {
      // indent(out) << "[" << kFieldPrefix << field_name << " release_stub];"
      // << std::endl;
      indent(out) << kFieldPrefix << field_name << " = nil;" << std::endl;
    }
    indent(out) << kFieldPrefix << field_name << kSetPostfix << " = NO;"
                << std::endl;
    indent_down();
    indent(out) << "}" << std::endl << std::endl;
  }
}

/**
 * Generates a description method for the given struct
 *
 * @param tstruct The struct definition
 */
void t_cocoa_generator::generate_cocoa_struct_description(
    std::ofstream& out, const t_structured* /*tstruct*/) {
  out << indent() << "- (NSString *) description {" << std::endl;
  indent_up();
  indent(out) << "return [[self toDict] description];" << std::endl;
  indent_down();
  indent(out) << "}" << std::endl << std::endl;
}

/**
 * Recursively call [makeImmutable] on all the fields
 */
void t_cocoa_generator::generate_cocoa_struct_makeImmutable(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "- (BOOL) makeImmutable {" << std::endl;
  indent_up();
  out << indent() << "const BOOL wasImmutable = [self isImmutable];"
      << std::endl;

  out << indent() << "if (!wasImmutable) {" << std::endl;
  indent_up();

  for (const auto& field : tstruct->fields()) {
    const t_type* ttype = field.get_type();
    std::string field_name = kFieldPrefix + field.name();
    if (ttype->is_typedef()) {
      ttype = ttype->get_true_type();
    }
    if (ttype->is_struct()) {
      out << indent() << "if (" << field_name << " && "
          << "![" << field_name << " isImmutable]"
          << ") {" << std::endl;
      indent_up();
      out << indent() << "[" << field_name << " makeImmutable];" << std::endl;
      indent_down();
      out << indent() << "}" << std::endl;
    } else if (ttype->is_base_type()) {
      // nothing.
    } else if (ttype->is_enum()) {
      // nothing
    } else if (ttype->is_list() || ttype->is_set()) {
      out << indent() << "if (" << field_name << ") {" << std::endl;
      indent_up();
      out << indent() << "for (id item in " << field_name << ") {" << std::endl;
      indent_up();
      out << indent() << "if ([item isKindOfClass:["
          << kStructInheritanceRootObjectName << " class]]) {[(("
          << kStructInheritanceRootObjectName << "*)item) makeImmutable];}"
          << std::endl;
      // TODO:: can item be a list / std::map / set, in which case need to do
      // [copy] on it
      indent_down();
      out << indent() << "}" << std::endl;
      out << indent() << field_name << " = "
          << "[" << field_name << " copy];" << std::endl;
      indent_down();
      out << indent() << "}" << std::endl;
    } else if (ttype->is_map()) {
      out << indent() << "if (" << field_name << ") {" << std::endl;
      indent_up();
      out << indent() << "for (NSString* k in " << field_name << ") {"
          << std::endl;
      indent_up();
      out << indent() << "id item = " << field_name << "[k];" << std::endl;
      out << indent() << "if ([item isKindOfClass:["
          << kStructInheritanceRootObjectName << " class]]) {[(("
          << kStructInheritanceRootObjectName << "*)item) makeImmutable];}"
          << std::endl;
      // TODO:: can item be a list / std::map / set, in which case need to do
      // [copy] on it
      indent_down();
      out << indent() << "}" << std::endl;
      out << indent() << field_name << " = "
          << "[" << field_name << " copy];" << std::endl;
      indent_down();
      out << indent() << "}" << std::endl;
    } else {
      std::cout << "WAT?! " << ttype->name() << std::endl;
      assert(false);
    }
  }
  out << indent() << "[super makeImmutable];" << std::endl;
  indent_down();
  out << indent() << "}" << std::endl;

  out << indent() << "return YES;" << std::endl;
  indent_down();
  indent(out) << "}" << std::endl << std::endl;
}

/**
 * Generates a toDict method
 *
 */
void t_cocoa_generator::generate_cocoa_struct_toDict(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "- (NSDictionary *) toDict {" << std::endl;
  indent_up();

  out << indent()
      << "NSMutableDictionary *ret = [NSMutableDictionary dictionary];"
      << std::endl;
  indent(out) << "ret[@\"" << kFieldPrefix << "struct_name\"]"
              << " = "
              << "@\"" + tstruct->name() + "\";" << std::endl;

  for (const auto& field : tstruct->fields()) {
    const t_type* ttype = field.get_type();
    std::string field_name = kFieldPrefix + field.name();
    std::string ret_equals = "ret[@\"" + field.name() + "\"] = ";
    if (ttype->is_typedef()) {
      ttype = ttype->get_true_type();
    }

    const bool check_for_null = ttype->is_struct() ||
        ttype->is_string_or_binary() || ttype->is_container();

    if (check_for_null) {
      out << indent() << "if (" << field_name << ") {" << std::endl;
      indent_up();
    }

    if (ttype->is_struct()) {
      out << indent() << ret_equals << "[" << field_name << " toDict];"
          << std::endl;
    } else if (ttype->is_string_or_binary()) {
      out << indent() << ret_equals << field_name << ";" << std::endl;
    } else if (ttype->is_base_type() || ttype->is_enum()) {
      out << indent() << ret_equals << "@(" << field_name << ");" << std::endl;
      if (ttype->is_enum()) {
        const t_program* program = ttype->program();
        std::string ToStringFunctionName = program
            ? (program->get_namespace("cocoa") + ttype->name() +
               kToStringPostfix)
            : cocoa_prefix_ + ttype->name() + kToStringPostfix;
        out << indent()
            << "ret[@\"" + field.name() + "_str\"] = " << ToStringFunctionName
            << "(" << field_name << ");" << std::endl;
      }
    } else if (ttype->is_list() || ttype->is_set()) {
      out << indent() << "NSMutableArray* a = [NSMutableArray array];"
          << std::endl;
      out << indent() << "for (id item in " << field_name << ") {" << std::endl;
      indent_up();
      out << indent() << "if ([item isKindOfClass:["
          << kStructInheritanceRootObjectName
          << " class]]) {[a addObject:[item toDict]];}" << std::endl;
      out << indent() << "else {[a addObject:item];}" << std::endl;
      indent_down();
      out << indent() << "}" << std::endl;
      out << indent() << ret_equals << "[a copy];" << std::endl;
    } else if (ttype->is_map()) {
      out << indent()
          << "NSMutableDictionary* d = [NSMutableDictionary dictionary];"
          << std::endl;
      out << indent() << "for (NSString* k in " << field_name << ") {"
          << std::endl;
      indent_up();
      out << indent() << "id item = " << field_name << "[k];" << std::endl;
      out << indent() << "if ([item isKindOfClass:["
          << kStructInheritanceRootObjectName
          << " class]]) {d[k] = [item toDict];}" << std::endl;
      out << indent() << "else {d[k] = item;}" << std::endl;
      indent_down();
      out << indent() << "}" << std::endl;
      out << indent() << ret_equals << "[d copy];" << std::endl;
    } else {
      std::cout << "WAT?! " << ttype->name() << std::endl;
      assert(false);
    }

    if (check_for_null) {
      indent_down();
      out << indent() << "}" << std::endl;
    }
  }

  out << indent() << "return [ret copy];" << std::endl;
  indent_down();
  indent(out) << "}" << std::endl << std::endl;
}

/**
 * Generate mutableCopyWithZone
 *
 */
void t_cocoa_generator::generate_cocoa_struct_mutableCopyWithZone(
    std::ofstream& out, const t_structured* tstruct) {
  out << indent() << "- (id) mutableCopyWithZone:(NSZone *)zone {" << std::endl;
  indent_up();
  out << indent() << cocoa_prefix_ << tstruct->name()
      << " *newCopy = [[[self class] alloc] init];;" << std::endl;

  for (const auto& field : tstruct->fields()) {
    const t_type* ttype = field.get_type();
    std::string field_name = kFieldPrefix + field.name();
    if (ttype->is_typedef()) {
      ttype = ttype->get_true_type();
    }

    const bool check_for_null = ttype->is_struct() ||
        ttype->is_string_or_binary() || ttype->is_container();

    if (check_for_null) {
      out << indent() << "if (" << field_name << ") {" << std::endl;
      indent_up();
    }

    if (ttype->is_struct() || ttype->is_list() || ttype->is_set() ||
        ttype->is_map()) {
      out << indent() << "newCopy->" << field_name << " = "
          << "[self->" << field_name << " mutableCopyWithZone:zone];"
          << std::endl;
    } else if (
        ttype->is_string_or_binary() || ttype->is_base_type() ||
        ttype->is_enum()) {
      out << indent() << "newCopy->" << field_name << " = "
          << "self->" << field_name << ";" << std::endl;
    } else {
      std::cout << "WAT?! " << ttype->name() << std::endl;
      assert(false);
    }
    if (check_for_null) {
      indent_down();
      out << indent() << "}" << std::endl;
    }
    out << indent() << "newCopy->" << field_name << kSetPostfix << " = "
        << "self->" << field_name << kSetPostfix << ";" << std::endl;
  }

  out << indent() << "return newCopy;" << std::endl;
  indent_down();
  indent(out) << "}" << std::endl << std::endl;
}

/**
 * Generates a thrift service.  In Objective-C this consists of a
 * protocol definition, a client interface and a client implementation.
 *
 * @param tservice The service definition
 */
void t_cocoa_generator::generate_service(const t_service* tservice) {
  generate_cocoa_service_protocol(f_header_, tservice);
  generate_cocoa_service_client_interface(f_header_, tservice);
  generate_cocoa_service_server_interface(f_header_, tservice);
  generate_cocoa_service_helpers(tservice);
  generate_cocoa_service_client_implementation(f_impl_, tservice);
  generate_cocoa_service_server_implementation(f_impl_, tservice);
}

/**
 * Generates structs for all the service return types
 *
 * @param tservice The service
 */
void t_cocoa_generator::generate_cocoa_service_helpers(
    const t_service* tservice) {
  std::vector<t_function*> functions = tservice->get_functions();
  std::vector<t_function*>::iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    const t_paramlist& ts = (*f_iter)->params();
    generate_cocoa_struct_interface(f_impl_, &ts, false);
    generate_cocoa_struct_implementation(f_impl_, &ts, false, false);
    generate_function_helpers(*f_iter);
  }
}

std::string t_cocoa_generator::function_result_helper_struct_type(
    const t_function* tfunction) {
  if (tfunction->qualifier() == t_function_qualifier::oneway) {
    return capitalize(tfunction->name());
  } else {
    return capitalize(tfunction->name()) + "_result";
  }
}

std::string t_cocoa_generator::function_args_helper_struct_type(
    const t_function* tfunction) {
  return tfunction->name() + "_args";
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_cocoa_generator::generate_function_helpers(const t_function* tfunction) {
  if (tfunction->qualifier() == t_function_qualifier::oneway) {
    return;
  }

  // create a result struct with a success field of the return type,
  // and a field for each type of exception thrown
  t_struct result(program_, function_result_helper_struct_type(tfunction));
  auto success =
      std::make_unique<t_field>(tfunction->return_type(), "success", 0);
  if (!tfunction->return_type()->is_void()) {
    result.append(std::move(success));
  }

  if (tfunction->exceptions() != nullptr) {
    for (const auto& x : tfunction->exceptions()->fields()) {
      result.append(x.clone_DO_NOT_USE());
    }
  }

  // generate the result struct
  generate_cocoa_struct_interface(f_impl_, &result, false);
  generate_cocoa_struct_implementation(f_impl_, &result, false, true);
}

/**
 * Generates a service protocol definition.
 *
 * @param tservice The service to generate a protocol definition for
 */
void t_cocoa_generator::generate_cocoa_service_protocol(
    std::ofstream& out, const t_service* tservice) {
  out << "@protocol " << cocoa_prefix_ << tservice->name() << " <NSObject>"
      << std::endl;

  for (const auto* function : tservice->get_functions()) {
    out << "- " << function_signature(function) << ";"
        << "  // throws ";
    for (const t_field& x : get_elems(function->exceptions())) {
      out << type_name(x.get_type()) + ", ";
    }
    out << "TException" << std::endl;
  }
  out << "@end" << std::endl << std::endl;
}

/**
 * Generates a service client interface definition.
 *
 * @param tservice The service to generate a client interface definition for
 */
void t_cocoa_generator::generate_cocoa_service_client_interface(
    std::ofstream& out, const t_service* tservice) {
  out << "@interface " << cocoa_prefix_ << tservice->name()
      << "Client : NSObject <" << cocoa_prefix_ << tservice->name() << "> ";

  scope_up(out);
  out << indent() << "id <TProtocol> inProtocol;" << std::endl;
  out << indent() << "id <TProtocol> outProtocol;" << std::endl;
  scope_down(out);

  out << "- (id) initWithProtocol: (id <TProtocol>) protocol;" << std::endl;
  out << "- (id) initWithInProtocol: (id <TProtocol>) inProtocol outProtocol: "
         "(id <TProtocol>) outProtocol;"
      << std::endl;
  out << "@end" << std::endl << std::endl;
}

/**
 * Generates a service server interface definition. In other words, the TProcess
 * implementation for the service definition.
 *
 * @param tservice The service to generate a client interface definition for
 */
void t_cocoa_generator::generate_cocoa_service_server_interface(
    std::ofstream& out, const t_service* tservice) {
  out << "@interface " << cocoa_prefix_ << tservice->name()
      << "Processor : NSObject <TProcessor> ";

  scope_up(out);
  out << indent() << "id <" << cocoa_prefix_ << tservice->name()
      << "> mService;" << std::endl;
  out << indent() << "NSDictionary * mMethodMap;" << std::endl;
  scope_down(out);

  out << "- (id) initWith" << tservice->name() << ": (id <" << cocoa_prefix_
      << tservice->name() << ">) service;" << std::endl;
  out << "- (id<" << cocoa_prefix_ << tservice->name() << ">) service;"
      << std::endl;

  out << "@end" << std::endl << std::endl;
}

/**
 * Generates a service client implementation.
 *
 * @param tservice The service to generate an implementation for
 */
void t_cocoa_generator::generate_cocoa_service_client_implementation(
    std::ofstream& out, const t_service* tservice) {
  out << "@implementation " << cocoa_prefix_ << tservice->name() << "Client"
      << std::endl;

  // initializers
  out << "- (id) initWithProtocol: (id <TProtocol>) protocol" << std::endl;
  scope_up(out);
  out << indent()
      << "return [self initWithInProtocol: protocol outProtocol: protocol];"
      << std::endl;
  scope_down(out);
  out << std::endl;

  out << "- (id) initWithInProtocol: (id <TProtocol>) anInProtocol "
         "outProtocol: (id <TProtocol>) anOutProtocol"
      << std::endl;
  scope_up(out);
  out << indent() << "self = [super init];" << std::endl;
  out << indent() << "inProtocol = [anInProtocol retain_stub];" << std::endl;
  out << indent() << "outProtocol = [anOutProtocol retain_stub];" << std::endl;
  out << indent() << "return self;" << std::endl;
  scope_down(out);
  out << std::endl;

  // dealloc
  out << "- (void) dealloc" << std::endl;
  scope_up(out);
  out << indent() << "[inProtocol release_stub];" << std::endl;
  out << indent() << "[outProtocol release_stub];" << std::endl;
  out << indent() << "[super dealloc_stub];" << std::endl;
  scope_down(out);
  out << std::endl;

  // generate client method implementations
  for (const auto* function : tservice->get_functions()) {
    const std::string& funname = function->name();

    t_function send_function(
        nullptr,
        t_type_ref::from_req_ptr(&t_base_type::t_void()),
        "send_" + function->name(),
        t_struct::clone_DO_NOT_USE(&function->params()));

    std::string argsname = function->name() + "_args";

    // Open function
    indent(out) << "- " << function_signature(&send_function) << std::endl;
    scope_up(out);

    // Serialize the request
    out << indent() << "[outProtocol writeMessageBeginWithName: @\"" << funname
        << "\""
        << " type: TMessageType_CALL"
        << " sequenceID: 0];" << std::endl;

    out << indent() << "[outProtocol writeStructBeginWithName: @\"" << argsname
        << "\"];" << std::endl;

    // write out function parameters
    for (const auto& param : function->params().fields()) {
      const std::string& fieldName = param.name();
      if (type_can_be_null(param.get_type())) {
        out << indent() << "if (" << fieldName << " != nil)";
        scope_up(out);
      }
      out << indent() << "[outProtocol writeFieldBeginWithName: @\""
          << fieldName
          << "\""
             " type: "
          << type_to_enum(param.get_type()) << " fieldID: " << param.get_key()
          << "];" << std::endl;

      generate_serialize_field(out, &param, fieldName);

      out << indent() << "[outProtocol writeFieldEnd];" << std::endl;

      if (type_can_be_null(param.get_type())) {
        scope_down(out);
      }
    }

    out << indent() << "[outProtocol writeFieldStop];" << std::endl;
    out << indent() << "[outProtocol writeStructEnd];" << std::endl;

    out << indent() << "[outProtocol writeMessageEnd];" << std::endl
        << indent() << "[[outProtocol transport] flush];" << std::endl;

    scope_down(out);
    out << std::endl;

    if (function->qualifier() != t_function_qualifier::oneway) {
      t_function recv_function(
          program_, function->return_type(), "recv_" + function->name());
      if (const t_throws* exceptions = function->exceptions()) {
        recv_function.set_exceptions(t_struct::clone_DO_NOT_USE(exceptions));
      }
      // Open function
      indent(out) << "- " << function_signature(&recv_function) << std::endl;
      scope_up(out);

      // TODO(mcslee): Message validation here, was the seqid etc ok?

      // check for an exception
      out << indent() << "int msgType = 0;" << std::endl
          << indent()
          << "[inProtocol readMessageBeginReturningName: nil type: &msgType "
             "sequenceID: NULL];"
          << std::endl
          << indent() << "if (msgType == TMessageType_EXCEPTION) {" << std::endl
          << indent()
          << "  TApplicationException * x = [TApplicationException read: "
             "inProtocol];"
          << std::endl
          << indent() << "  [inProtocol readMessageEnd];" << std::endl
          << indent() << "  @throw x;" << std::endl
          << indent() << "}" << std::endl;

      // FIXME - could optimize here to reduce creation of temporary objects.
      std::string resultname = function_result_helper_struct_type(function);
      out << indent() << cocoa_prefix_ << resultname << " * result = [[["
          << cocoa_prefix_ << resultname << " alloc] init] autorelease_stub];"
          << std::endl;
      indent(out) << "[result read: inProtocol];" << std::endl;
      indent(out) << "[inProtocol readMessageEnd];" << std::endl;

      // Careful, only return _result if not a void function
      if (!function->return_type()->is_void()) {
        out << indent() << "if ([result successIsSet]) {" << std::endl
            << indent() << "  return [result success];" << std::endl
            << indent() << "}" << std::endl;
      }

      if (function->exceptions() != nullptr) {
        for (const auto& x : function->exceptions()->fields()) {
          out << indent() << "if ([result " << x.name() << "IsSet]) {"
              << std::endl
              << indent() << "  @throw [result " << x.name() << "];"
              << std::endl
              << indent() << "}" << std::endl;
        }
      }

      // If you get here it's an exception, unless a void function
      if (function->return_type()->is_void()) {
        indent(out) << "return;" << std::endl;
      } else {
        out << indent()
            << "@throw [TApplicationException exceptionWithType: "
               "TApplicationException_MISSING_RESULT"
            << std::endl
            << indent()
            << "                                         reason: @\""
            << function->name() << " failed: unknown result\"];" << std::endl;
      }

      // Close function
      scope_down(out);
      out << std::endl;
    }

    // Open function
    indent(out) << "- " << function_signature(function) << std::endl;
    scope_up(out);
    indent(out) << "[self send_" << funname;

    // Declare the function arguments
    bool first = true;
    for (const auto& param : function->params().fields()) {
      const std::string& fieldName = param.name();
      out << " ";
      if (first) {
        first = false;
        out << ": " << fieldName;
      } else {
        out << fieldName << ": " << fieldName;
      }
    }
    out << "];" << std::endl;

    if (function->qualifier() != t_function_qualifier::oneway) {
      out << indent();
      if (!function->return_type()->is_void()) {
        out << "return ";
      }
      out << "[self recv_" << funname << "];" << std::endl;
    }
    scope_down(out);
    out << std::endl;
  }

  indent_down();

  out << "@end" << std::endl << std::endl;
}

/**
 * Generates a service server implementation.  In other words the actual
 * TProcessor implementation for the service.
 *
 * @param tservice The service to generate an implementation for
 */
void t_cocoa_generator::generate_cocoa_service_server_implementation(
    std::ofstream& out, const t_service* tservice) {
  out << "@implementation " << cocoa_prefix_ << tservice->name() << "Processor"
      << std::endl;
  indent_up();

  // initializer
  out << std::endl;
  out << "- (id) initWith" << tservice->name() << ": (id <" << cocoa_prefix_
      << tservice->name() << ">) service" << std::endl;
  scope_up(out);
  out << indent() << "self = [super init];" << std::endl;
  out << indent() << "if (!self) {" << std::endl;
  out << indent() << "  return nil;" << std::endl;
  out << indent() << "}" << std::endl;
  // out << indent() << "mService = [service retain_stub];" << std::endl;
  // out << indent() << "mMethodMap = [[NSMutableDictionary dictionary]
  // retain_stub];" << std::endl;

  // generate method std::map for routing incoming calls
  for (const auto* function : tservice->get_functions()) {
    const std::string& funname = function->name();
    scope_up(out);
    out << indent() << "SEL s = @selector(process_" << funname
        << "_withSequenceID:inProtocol:outProtocol:);" << std::endl;
    out << indent()
        << "NSMethodSignature * sig = [self methodSignatureForSelector: s];"
        << std::endl;
    out << indent()
        << "NSInvocation * invocation = [NSInvocation "
           "invocationWithMethodSignature: sig];"
        << std::endl;
    out << indent() << "[invocation setSelector: s];" << std::endl;
    out << indent() << "[invocation retainArguments];" << std::endl;
    out << indent() << "[mMethodMap setValue: invocation forKey: @\"" << funname
        << "\"];" << std::endl;
    scope_down(out);
  }
  out << indent() << "return self;" << std::endl;
  scope_down(out);

  // implementation of the 'service' method which returns the service associated
  // with this processor
  out << std::endl;
  out << indent() << "- (id<" << cocoa_prefix_ << tservice->name()
      << ">) service" << std::endl;
  out << indent() << "{" << std::endl;
  out << indent() << "  return [[mService retain_stub] autorelease_stub];"
      << std::endl;
  out << indent() << "}" << std::endl;

  // implementation of the TProcess method, which dispatches the incoming call
  // using the method std::map
  out << std::endl;
  out << indent()
      << "- (BOOL) processOnInputProtocol: (id <TProtocol>) inProtocol"
      << std::endl;
  out << indent()
      << "                 outputProtocol: (id <TProtocol>) outProtocol"
      << std::endl;
  out << indent() << "{" << std::endl;
  out << indent() << "  NSString * messageName;" << std::endl;
  out << indent() << "  int messageType;" << std::endl;
  out << indent() << "  int seqID;" << std::endl;
  out << indent() << "  [inProtocol readMessageBeginReturningName: &messageName"
      << std::endl;
  out << indent() << "                                       type: &messageType"
      << std::endl;
  out << indent() << "                                 sequenceID: &seqID];"
      << std::endl;
  out << indent()
      << "  NSInvocation * invocation = [mMethodMap valueForKey: messageName];"
      << std::endl;
  out << indent() << "  if (invocation == nil) {" << std::endl;
  out << indent()
      << "    [TProtocolUtil skipType: TType_STRUCT onProtocol: inProtocol];"
      << std::endl;
  out << indent() << "    [inProtocol readMessageEnd];" << std::endl;
  out << indent()
      << "    TApplicationException * x = [TApplicationException "
         "exceptionWithType: TApplicationException_UNKNOWN_METHOD reason: "
         "[NSString stringWithFormat: @\"Invalid method name: '%@'\", "
         "messageName]];"
      << std::endl;
  out << indent() << "    [outProtocol writeMessageBeginWithName: messageName"
      << std::endl;
  out << indent()
      << "                                      type: TMessageType_EXCEPTION"
      << std::endl;
  out << indent() << "                                sequenceID: seqID];"
      << std::endl;
  out << indent() << "    [x write: outProtocol];" << std::endl;
  out << indent() << "    [outProtocol writeMessageEnd];" << std::endl;
  out << indent() << "    [[outProtocol transport] flush];" << std::endl;
  out << indent() << "    return YES;" << std::endl;
  out << indent() << "  }" << std::endl;
  out << indent() << "  // NSInvocation does not conform to NSCopying protocol"
      << std::endl;
  out << indent()
      << "  NSInvocation * i = [NSInvocation invocationWithMethodSignature: "
         "[invocation methodSignature]];"
      << std::endl;
  out << indent() << "  [i setSelector: [invocation selector]];" << std::endl;
  out << indent() << "  [i setArgument: &seqID atIndex: 2];" << std::endl;
  out << indent() << "  [i setArgument: &inProtocol atIndex: 3];" << std::endl;
  out << indent() << "  [i setArgument: &outProtocol atIndex: 4];" << std::endl;
  out << indent() << "  [i setTarget: self];" << std::endl;
  out << indent() << "  [i invoke];" << std::endl;
  out << indent() << "  return YES;" << std::endl;
  out << indent() << "}" << std::endl;

  // generate a process_XXXX method for each service function, which reads args,
  // calls the service, and writes results
  for (const auto* function : tservice->get_functions()) {
    out << std::endl;
    const std::string& funname = function->name();
    out << indent() << "- (void) process_" << funname
        << "_withSequenceID: (int32_t) seqID inProtocol: (id<TProtocol>) "
           "inProtocol outProtocol: (id<TProtocol>) outProtocol"
        << std::endl;
    scope_up(out);
    std::string argstype =
        cocoa_prefix_ + function_args_helper_struct_type(function);
    out << indent() << argstype << " * args = [[" << argstype
        << " alloc] init];" << std::endl;
    out << indent() << "[args read: inProtocol];" << std::endl;
    out << indent() << "[inProtocol readMessageEnd];" << std::endl;

    // prepare the result if not oneway
    if (function->qualifier() != t_function_qualifier::oneway) {
      std::string resulttype =
          cocoa_prefix_ + function_result_helper_struct_type(function);
      out << indent() << resulttype << " * result = [[" << resulttype
          << " alloc] init];" << std::endl;
    }

    // make the call to the actual service object
    out << indent();
    if (!function->return_type()->is_void()) {
      out << "[result setSuccess: ";
    }
    out << "[mService " << funname;
    bool first = true;
    for (const auto& param : function->params().fields()) {
      const std::string& fieldName = param.name();
      if (first) {
        first = false;
        out << ": [args " << fieldName << "]";
      } else {
        out << " " << fieldName << ": [args " << fieldName << "]";
      }
    }
    out << "]";
    if (!function->return_type()->is_void()) {
      out << "]";
    }
    out << ";" << std::endl;

    // write out the result if not oneway
    if (function->qualifier() != t_function_qualifier::oneway) {
      out << indent() << "[outProtocol writeMessageBeginWithName: @\""
          << funname << "\"" << std::endl;
      out << indent()
          << "                                  type: TMessageType_REPLY"
          << std::endl;
      out << indent() << "                            sequenceID: seqID];"
          << std::endl;
      out << indent() << "[result write: outProtocol];" << std::endl;
      out << indent() << "[outProtocol writeMessageEnd];" << std::endl;
      out << indent() << "[[outProtocol transport] flush];" << std::endl;
      out << indent() << "[result release_stub];" << std::endl;
    }
    out << indent() << "[args release_stub];" << std::endl;

    scope_down(out);
  }

  // dealloc
  out << std::endl;
  out << "- (void) dealloc" << std::endl;
  scope_up(out);
  out << indent() << "[mService release_stub];" << std::endl;
  out << indent() << "[mMethodMap release_stub];" << std::endl;
  out << indent() << "[super dealloc_stub];" << std::endl;
  scope_down(out);
  out << std::endl;

  indent_down();

  out << "@end" << std::endl << std::endl;
}

/**
 * Deserializes a field of any type.
 *
 * @param tfield The field
 * @param fieldName The variable name for this field
 */
void t_cocoa_generator::generate_deserialize_field(
    std::ofstream& out, const t_field* tfield, const std::string& fieldName) {
  const t_type* type = tfield->type()->get_true_type();

  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + tfield->name());
  }

  if (type->is_struct() || type->is_exception()) {
    generate_deserialize_struct(out, type, fieldName);
  } else if (type->is_container()) {
    generate_deserialize_container(out, type, fieldName);
  } else if (type->is_base_type() || type->is_enum()) {
    indent(out) << type_name(type) << " " << fieldName << " = [inProtocol ";

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              tfield->name());
        case t_base_type::TYPE_STRING:
          out << "readString];";
          break;
        case t_base_type::TYPE_BINARY:
          out << "readBinary];";
          break;
        case t_base_type::TYPE_BOOL:
          out << "readBool];";
          break;
        case t_base_type::TYPE_BYTE:
          out << "readByte];";
          break;
        case t_base_type::TYPE_I16:
          out << "readI16];";
          break;
        case t_base_type::TYPE_I32:
          out << "readI32];";
          break;
        case t_base_type::TYPE_I64:
          out << "readI64];";
          break;
        case t_base_type::TYPE_DOUBLE:
          out << "readDouble];";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no Objective-C name for base type " +
              t_base_type::t_base_name(tbase));
      }
    } else if (type->is_enum()) {
      out << "readI32];";
    }
    out << std::endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n",
        tfield->name().c_str(),
        type_name(type).c_str());
  }
}

/**
 * Generates an unserializer for a struct, allocates the struct and invokes
 * read:
 */
void t_cocoa_generator::generate_deserialize_struct(
    std::ofstream& out, const t_type* tstruct, const std::string& fieldName) {
  indent(out) << type_name(tstruct) << fieldName << " = [["
              << type_name(tstruct, true) << " alloc] init];" << std::endl;
  indent(out) << "[" << fieldName << " read: inProtocol];" << std::endl;
}

/**
 * Deserializes a container by reading its size and then iterating
 */
void t_cocoa_generator::generate_deserialize_container(
    std::ofstream& out, const t_type* ttype, const std::string& fieldName) {
  std::string size = tmp("_size");
  indent(out) << "int " << size << ";" << std::endl;

  // Declare variables, read header
  if (ttype->is_map()) {
    indent(out) << "[inProtocol readMapBeginReturningKeyType: NULL valueType: "
                   "NULL size: &"
                << size << "];" << std::endl;
    indent(out) << "NSMutableDictionary * " << fieldName
                << " = [[NSMutableDictionary alloc] initWithCapacity: " << size
                << "];" << std::endl;
  } else if (ttype->is_set()) {
    indent(out) << "[inProtocol readSetBeginReturningElementType: NULL size: &"
                << size << "];" << std::endl;
    indent(out) << "NSMutableSet * " << fieldName
                << " = [[NSMutableSet alloc] initWithCapacity: " << size << "];"
                << std::endl;
  } else if (ttype->is_list()) {
    indent(out) << "[inProtocol readListBeginReturningElementType: NULL size: &"
                << size << "];" << std::endl;
    indent(out) << "NSMutableArray * " << fieldName
                << " = [[NSMutableArray alloc] initWithCapacity: " << size
                << "];" << std::endl;
  }
  // FIXME - the code above does not verify that the element types of
  // the containers being read match the element types of the
  // containers we are reading into.  Does that matter?

  // For loop iterates over elements
  std::string i = tmp("_i");
  indent(out) << "int " << i << ";" << std::endl
              << indent() << "for (" << i << " = 0; " << i << " < " << size
              << "; "
              << "++" << i << ")" << std::endl;

  scope_up(out);

  if (ttype->is_map()) {
    generate_deserialize_map_element(out, (t_map*)ttype, fieldName);
  } else if (ttype->is_set()) {
    generate_deserialize_set_element(out, (t_set*)ttype, fieldName);
  } else if (ttype->is_list()) {
    generate_deserialize_list_element(out, (t_list*)ttype, fieldName);
  }

  scope_down(out);

  // Read container end
  if (ttype->is_map()) {
    indent(out) << "[inProtocol readMapEnd];" << std::endl;
  } else if (ttype->is_set()) {
    indent(out) << "[inProtocol readSetEnd];" << std::endl;
  } else if (ttype->is_list()) {
    indent(out) << "[inProtocol readListEnd];" << std::endl;
  }
}

/**
 * Take a variable of a given type and wrap it in code to make it
 * suitable for putting into a container, if necessary.  Basically,
 * wrap scaler primitives in NSNumber objects.
 */
std::string t_cocoa_generator::containerize(
    const t_type* ttype, const std::string& fieldName) {
  // FIXME - optimize here to avoid autorelease pool?
  ttype = ttype->get_true_type();
  if (ttype->is_enum()) {
    return "[NSNumber numberWithInt: " + fieldName + "]";
  } else if (ttype->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("can't containerize void");
      case t_base_type::TYPE_BOOL:
        return "[NSNumber numberWithBool: " + fieldName + "]";
      case t_base_type::TYPE_BYTE:
        return "[NSNumber numberWithUnsignedChar: " + fieldName + "]";
      case t_base_type::TYPE_I16:
        return "[NSNumber numberWithShort: " + fieldName + "]";
      case t_base_type::TYPE_I32:
        return "[NSNumber numberWithLong: " + fieldName + "]";
      case t_base_type::TYPE_I64:
        return "[NSNumber numberWithLongLong: " + fieldName + "]";
      case t_base_type::TYPE_DOUBLE:
        return "[NSNumber numberWithDouble: " + fieldName + "]";
      default:
        break;
    }
  }

  // do nothing
  return fieldName;
}

/**
 * Generates code to deserialize a std::map element
 */
void t_cocoa_generator::generate_deserialize_map_element(
    std::ofstream& out, const t_map* tmap, const std::string& fieldName) {
  std::string key = tmp("_key");
  std::string val = tmp("_val");
  const t_type* key_type = tmap->get_key_type();
  const t_type* valType = tmap->get_val_type();
  t_field fkey(key_type, key);
  t_field fval(valType, val);

  generate_deserialize_field(out, &fkey, key);
  generate_deserialize_field(out, &fval, val);

  indent(out) << "[" << fieldName
              << " setObject: " << containerize(valType, val)
              << " forKey: " << containerize(key_type, key) << "];"
              << std::endl;

  if (type_can_be_null(key_type)) {
    if (!(key_type->get_true_type()->is_string_or_binary())) {
      indent(out) << "[" << containerize(key_type, key) << " release_stub];"
                  << std::endl;
    }
  }

  if (type_can_be_null(valType)) {
    if (!(valType->get_true_type()->is_string_or_binary())) {
      indent(out) << "[" << containerize(valType, val) << " release_stub];"
                  << std::endl;
    }
  }
}

/**
 * Deserializes a set element
 */
void t_cocoa_generator::generate_deserialize_set_element(
    std::ofstream& out, const t_set* tset, const std::string& fieldName) {
  std::string elem = tmp("_elem");
  const t_type* type = tset->get_elem_type();
  t_field felem(type, elem);

  generate_deserialize_field(out, &felem, elem);

  indent(out) << "[" << fieldName << " addObject: " << containerize(type, elem)
              << "];" << std::endl;

  if (type_can_be_null(type)) {
    // deserialized strings are autorelease, so don't release them
    if (!(type->get_true_type()->is_string_or_binary())) {
      indent(out) << "[" << containerize(type, elem) << " release_stub];"
                  << std::endl;
    }
  }
}

/**
 * Deserializes a list element
 */
void t_cocoa_generator::generate_deserialize_list_element(
    std::ofstream& out, const t_list* tlist, const std::string& fieldName) {
  std::string elem = tmp("_elem");
  const t_type* type = tlist->get_elem_type();
  t_field felem(type, elem);

  generate_deserialize_field(out, &felem, elem);

  indent(out) << "[" << fieldName << " addObject: " << containerize(type, elem)
              << "];" << std::endl;

  if (type_can_be_null(type)) {
    if (!(type->get_true_type()->is_string_or_binary())) {
      indent(out) << "[" << containerize(type, elem) << " release_stub];"
                  << std::endl;
    }
  }
}

/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param fieldName Name to of the variable holding the field
 */
void t_cocoa_generator::generate_serialize_field(
    std::ofstream& out, const t_field* tfield, const std::string& fieldName) {
  const t_type* type = tfield->type()->get_true_type();

  // Do nothing for void types
  if (type->is_void()) {
    throw std::runtime_error(
        "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + tfield->name());
  }

  if (type->is_struct() || type->is_exception()) {
    generate_serialize_struct(out, (t_struct*)type, fieldName);
  } else if (type->is_container()) {
    generate_serialize_container(out, type, fieldName);
  } else if (type->is_base_type() || type->is_enum()) {
    indent(out) << "[outProtocol ";

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw std::runtime_error(
              "compiler error: cannot serialize void field in a struct: " +
              fieldName);
        case t_base_type::TYPE_STRING:
          out << "writeString: " << fieldName << "];";
          break;
        case t_base_type::TYPE_BINARY:
          out << "writeBinary: " << fieldName << "];";
          break;
        case t_base_type::TYPE_BOOL:
          out << "writeBool: " << fieldName << "];";
          break;
        case t_base_type::TYPE_BYTE:
          out << "writeByte: " << fieldName << "];";
          break;
        case t_base_type::TYPE_I16:
          out << "writeI16: " << fieldName << "];";
          break;
        case t_base_type::TYPE_I32:
          out << "writeI32: " << fieldName << "];";
          break;
        case t_base_type::TYPE_I64:
          out << "writeI64: " << fieldName << "];";
          break;
        case t_base_type::TYPE_DOUBLE:
          out << "writeDouble: " << fieldName << "];";
          break;
        default:
          throw std::runtime_error(
              "compiler error: no Objective-C name for base type " +
              t_base_type::t_base_name(tbase));
      }
    } else if (type->is_enum()) {
      out << "writeI32: " << fieldName << "];";
    }
    out << std::endl;
  } else {
    printf(
        "DO NOT KNOW HOW TO SERIALIZE FIELD '%s' TYPE '%s'\n",
        tfield->name().c_str(),
        type_name(type).c_str());
  }
}

/**
 * Serialize a struct.
 *
 * @param tstruct The struct to serialize
 * @param fieldName Name of variable holding struct
 */
void t_cocoa_generator::generate_serialize_struct(
    std::ofstream& out, const t_struct* tstruct, const std::string& fieldName) {
  (void)tstruct;
  out << indent() << "[" << fieldName << " write: outProtocol];" << std::endl;
}

/**
 * Serializes a container by writing its size then the elements.
 *
 * @param ttype  The type of container
 * @param fieldName Name of variable holding container
 */
void t_cocoa_generator::generate_serialize_container(
    std::ofstream& out, const t_type* ttype, const std::string& fieldName) {
  scope_up(out);

  if (ttype->is_map()) {
    indent(out) << "[outProtocol writeMapBeginWithKeyType: "
                << type_to_enum(((t_map*)ttype)->get_key_type())
                << " valueType: "
                << type_to_enum(((t_map*)ttype)->get_val_type())
                << " size: (int)[" << fieldName << " count]];" << std::endl;
  } else if (ttype->is_set()) {
    indent(out) << "[outProtocol writeSetBeginWithElementType: "
                << type_to_enum(((t_set*)ttype)->get_elem_type())
                << " size: (int)[" << fieldName << " count]];" << std::endl;
  } else if (ttype->is_list()) {
    indent(out) << "[outProtocol writeListBeginWithElementType: "
                << type_to_enum(((t_list*)ttype)->get_elem_type())
                << " size: (int)[" << fieldName << " count]];" << std::endl;
  }

  std::string iter = tmp("_iter");
  std::string key;
  if (ttype->is_map()) {
    key = tmp("key");
    indent(out) << "NSEnumerator * " << iter << " = [" << fieldName
                << " keyEnumerator];" << std::endl;
    indent(out) << "id " << key << ";" << std::endl;
    indent(out) << "while ((" << key << " = [" << iter << " nextObject]))"
                << std::endl;
  } else if (ttype->is_set()) {
    key = tmp("obj");
    indent(out) << "NSEnumerator * " << iter << " = [" << fieldName
                << " objectEnumerator];" << std::endl;
    indent(out) << "id " << key << ";" << std::endl;
    indent(out) << "while ((" << key << " = [" << iter << " nextObject]))"
                << std::endl;
  } else if (ttype->is_list()) {
    key = tmp("i");
    indent(out) << "int " << key << ";" << std::endl;
    indent(out) << "for (" << key << " = 0; " << key << " < [" << fieldName
                << " count]; " << key << "++)" << std::endl;
  }

  scope_up(out);

  if (ttype->is_map()) {
    generate_serialize_map_element(out, (t_map*)ttype, key, fieldName);
  } else if (ttype->is_set()) {
    generate_serialize_set_element(out, (t_set*)ttype, key);
  } else if (ttype->is_list()) {
    generate_serialize_list_element(out, (t_list*)ttype, key, fieldName);
  }

  scope_down(out);

  if (ttype->is_map()) {
    indent(out) << "[outProtocol writeMapEnd];" << std::endl;
  } else if (ttype->is_set()) {
    indent(out) << "[outProtocol writeSetEnd];" << std::endl;
  } else if (ttype->is_list()) {
    indent(out) << "[outProtocol writeListEnd];" << std::endl;
  }

  scope_down(out);
}

/**
 * Given a field variable name, wrap it in code that converts it to a
 * primitive type, if necessary.
 */
std::string t_cocoa_generator::decontainerize(
    const t_field* tfield, const std::string& fieldName) {
  const t_type* ttype = tfield->type()->get_true_type();
  if (ttype->is_enum()) {
    return "[" + fieldName + " intValue]";
  } else if (ttype->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("can't decontainerize void");
      case t_base_type::TYPE_BOOL:
        return "[" + fieldName + " boolValue]";
      case t_base_type::TYPE_BYTE:
        return "[" + fieldName + " unsignedCharValue]";
      case t_base_type::TYPE_I16:
        return "[" + fieldName + " shortValue]";
      case t_base_type::TYPE_I32:
        return "[" + fieldName + " longValue]";
      case t_base_type::TYPE_I64:
        return "[" + fieldName + " longLongValue]";
      case t_base_type::TYPE_DOUBLE:
        return "[" + fieldName + " doubleValue]";
      default:
        break;
    }
  }

  // do nothing
  return fieldName;
}

/**
 * Serializes the members of a std::map.
 */
void t_cocoa_generator::generate_serialize_map_element(
    std::ofstream& out,
    const t_map* tmap,
    const std::string& key,
    const std::string& mapName) {
  t_field kfield(tmap->get_key_type(), key);
  generate_serialize_field(out, &kfield, decontainerize(&kfield, key));
  t_field vfield(
      tmap->get_val_type(), "[" + mapName + " objectForKey: " + key + "]");
  generate_serialize_field(
      out, &vfield, decontainerize(&vfield, vfield.name()));
}

/**
 * Serializes the members of a set.
 */
void t_cocoa_generator::generate_serialize_set_element(
    std::ofstream& out, const t_set* tset, const std::string& elementName) {
  t_field efield(tset->get_elem_type(), elementName);
  generate_serialize_field(out, &efield, decontainerize(&efield, elementName));
}

/**
 * Serializes the members of a list.
 */
void t_cocoa_generator::generate_serialize_list_element(
    std::ofstream& out,
    const t_list* tlist,
    const std::string& index,
    const std::string& listName) {
  t_field efield(
      tlist->get_elem_type(),
      "[" + listName + " objectAtIndex: " + index + "]");
  generate_serialize_field(
      out, &efield, decontainerize(&efield, efield.name()));
}

/**
 * Returns an Objective-C name
 *
 * @param ttype The type
 * @param class_ref Do we want a Class reference istead of a type reference?
 * @return Java type name, i.e. HashMap<Key,Value>
 */
std::string t_cocoa_generator::type_name(const t_type* ttype, bool class_ref) {
  if (ttype->is_typedef() || ttype->is_enum()) {
    const t_program* program = ttype->program();
    return program ? (program->get_namespace("cocoa") + ttype->name())
                   : ttype->name();
  }

  std::string result;
  if (ttype->is_base_type()) {
    return base_type_name((t_base_type*)ttype);
  } else if (ttype->is_map()) {
    result = "TBaseStructDictionary";
  } else if (ttype->is_set()) {
    result = "TBaseStructSet";
  } else if (ttype->is_list()) {
    result = "TBaseStructArray";
  } else {
    // Check for prefix
    const t_program* program = ttype->program();
    result = program ? (program->get_namespace("cocoa") + ttype->name())
                     : ttype->name();
  }

  if (!class_ref) {
    result += " *";
  }
  return result;
}

/**
 * Returns the Objective-C type that corresponds to the thrift type.
 *
 * @param tbase The base type
 */
std::string t_cocoa_generator::base_type_name(t_base_type* type) {
  t_base_type::t_base tbase = type->get_base();

  switch (tbase) {
    case t_base_type::TYPE_VOID:
      return "void";
    case t_base_type::TYPE_STRING:
      return "NSString *";
    case t_base_type::TYPE_BINARY:
      return "NSData *";
    case t_base_type::TYPE_BOOL:
      return "BOOL";
    case t_base_type::TYPE_BYTE:
      return "uint8_t";
    case t_base_type::TYPE_I16:
      return "int16_t";
    case t_base_type::TYPE_I32:
      return "int32_t";
    case t_base_type::TYPE_I64:
      return "int64_t";
    case t_base_type::TYPE_DOUBLE:
      return "double";
    default:
      throw std::runtime_error(
          "compiler error: no Objective-C name for base type " +
          t_base_type::t_base_name(tbase));
  }
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
void t_cocoa_generator::print_const_value(
    std::ofstream& out,
    const std::string& name,
    const t_type* type,
    const t_const_value* value,
    bool defval,
    bool is_property) {
  type = type->get_true_type();

  indent(out);
  if (type->is_base_type()) {
    std::string v2 = render_const_value(out, type, value);
    if (defval)
      out << type_name(type) << " ";
    out << name << " = " << v2 << ";" << std::endl << std::endl;
  } else if (type->is_enum()) {
    if (defval)
      out << type_name(type) << " ";
    out << name << " = " << render_const_value(out, type, value) << ";"
        << std::endl
        << std::endl;
  } else if (type->is_struct() || type->is_exception()) {
    const auto* as_struct = static_cast<const t_struct*>(type);
    if (defval) {
      out << type_name(type) << " ";
    }
    if (defval || is_property) {
      out << name << " = [[[" << type_name(type, true)
          << " alloc] init] autorelease_stub];" << std::endl;
    } else {
      out << name << " = [[" << type_name(type, true) << " alloc] init];"
          << std::endl;
    }
    for (const auto& entry : value->get_map()) {
      const auto* field =
          as_struct->get_field_by_name(entry.first->get_string());
      if (field == nullptr) {
        throw std::runtime_error(
            "type error: " + type->name() + " has no field " +
            entry.first->get_string());
      }
      std::string val =
          render_const_value(out, field->get_type(), entry.second);
      std::string cap_name = capitalize(entry.first->get_string());
      indent(out) << "[" << name << " set" << cap_name << ":" << val << "];"
                  << std::endl;
    }
    out << std::endl;
  } else if (type->is_map()) {
    const t_type* ktype = ((t_map*)type)->get_key_type();
    const t_type* vtype = ((t_map*)type)->get_val_type();
    const std::vector<std::pair<t_const_value*, t_const_value*>>& val =
        value->get_map();
    std::vector<std::pair<t_const_value*, t_const_value*>>::const_iterator
        v_iter;
    if (defval)
      out << "NSMutableDictionary *";
    if (defval || is_property)
      out << name
          << " = [[[NSMutableDictionary alloc] initWithCapacity:" << val.size()
          << "] autorelease_stub]; " << std::endl;
    else
      out << name
          << " = [[NSMutableDictionary alloc] initWithCapacity:" << val.size()
          << "]; " << std::endl;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      std::string key = render_const_value(out, ktype, v_iter->first, true);
      std::string val = render_const_value(out, vtype, v_iter->second, true);
      indent(out) << "[" << name << " setObject:" << val << " forKey:" << key
                  << "];" << std::endl;
    }
    out << std::endl;
  } else if (type->is_list()) {
    const t_type* etype = ((t_list*)type)->get_elem_type();
    const std::vector<t_const_value*>& val = value->get_list();
    std::vector<t_const_value*>::const_iterator v_iter;
    if (defval)
      out << "NSMutableArray *";
    if (defval || is_property)
      out << name
          << " = [[[NSMutableArray alloc] initWithCapacity:" << val.size()
          << "] autorelease_stub];" << std::endl;
    else
      out << name
          << " = [[NSMutableArray alloc] initWithCapacity:" << val.size()
          << "];" << std::endl;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      std::string val = render_const_value(out, etype, *v_iter, true);
      indent(out) << "[" << name << " addObject:" << val << "];" << std::endl;
    }
    out << std::endl;
  } else if (type->is_set()) {
    const t_type* etype = ((t_set*)type)->get_elem_type();
    const std::vector<t_const_value*>& val = value->get_list();
    std::vector<t_const_value*>::const_iterator v_iter;
    if (defval)
      out << "NSMutableSet *";
    if (defval || is_property)
      out << name << " = [[[NSMutableSet alloc] initWithCapacity:" << val.size()
          << "] autorelease_stub];" << std::endl;
    else
      out << name << " = [[NSMutableSet alloc] initWithCapacity:" << val.size()
          << "];" << std::endl;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      std::string val = render_const_value(out, etype, *v_iter, true);
      indent(out) << "[" << name << " addObject:" << val << "];" << std::endl;
    }
    out << std::endl;
  } else {
    throw std::runtime_error(
        "compiler error: no const of type " + type->name());
  }
}

std::string t_cocoa_generator::render_const_value(
    std::ofstream& out,
    const t_type* type,
    const t_const_value* value,
    bool containerize_it) {
  type = type->get_true_type();
  std::ostringstream render;

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        // We must handle binary constant but the syntax of IDL defines
        // nothing about binary constant.
        render << "@\"" << get_escaped_string(value) << '"';
        break;
      case t_base_type::TYPE_BOOL:
        render << ((value->get_integer() > 0) ? "YES" : "NO");
        break;
      case t_base_type::TYPE_BYTE:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
        render << value->get_integer();
        break;
      case t_base_type::TYPE_DOUBLE:
        if (value->kind() == t_const_value::CV_INTEGER) {
          render << value->get_integer();
        } else {
          render << value->get_double();
        }
        break;
      default:
        throw std::runtime_error(
            "compiler error: no const of base type " +
            t_base_type::t_base_name(tbase));
    }
  } else if (type->is_enum()) {
    render << value->get_integer();
  } else {
    std::string t = tmp("tmp");
    print_const_value(out, t, type, value, true, false);
    render << t;
  }

  if (containerize_it) {
    return containerize(type, render.str());
  }
  return render.str();
}

/**
 * Declares a field.
 *
 * @param ttype The type
 */
std::string t_cocoa_generator::declare_field(const t_field* tfield) {
  return type_name(tfield->get_type()) + " " + kFieldPrefix + tfield->name() +
      ";";
}

/**
 * Declares an Objective-C 2.0 property.
 *
 * @param tfield The field to declare a property for
 */
std::string t_cocoa_generator::declare_property(const t_field* tfield) {
  std::ostringstream render;
  render << "@property (nonatomic";
  if (type_can_be_null(tfield->get_type())) {
    render << ", retain";
    if (nullability_) {
      render
          << (tfield->get_req() == t_field::e_req::required ? ", nonnull"
                                                            : ", nullable");
    }
  }
  render
      // << ", getter=" << decapitalize(get_cocoa_property_name(tfield))
      // << ", setter=set" << capitalize(get_cocoa_property_name(tfield)) +
      // ":"
      << ")"
      << " " << type_name(tfield->get_type()) << " "
      << get_cocoa_property_name(tfield) << ";";
  return render.str();
}

/**
 * Renders a function signature
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
std::string t_cocoa_generator::function_signature(const t_function* tfunction) {
  const t_type& type = *tfunction->return_type();
  std::string result = "(" + type_name(&type) + ") " + tfunction->name() +
      argument_list(tfunction->params());
  return result;
}

/**
 * Renders a colon separated list of types and names, suitable for an
 * objective-c parameter list
 */
std::string t_cocoa_generator::argument_list(const t_paramlist& tparamlist) {
  std::string result = "";

  bool first = true;
  for (const auto& field : tparamlist.fields()) {
    std::string argPrefix = "";
    if (first) {
      first = false;
    } else {
      argPrefix = field.name();
      result += " ";
    }
    result +=
        argPrefix + ": (" + type_name(field.get_type()) + ") " + field.name();
  }
  return result;
}

/**
 * Converts the parse type to an Objective-C enum std::string for the given
 * type.
 */
std::string t_cocoa_generator::type_to_enum(const t_type* type) {
  type = type->get_true_type();

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("NO T_VOID CONSTRUCT");
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        return "TType_STRING";
      case t_base_type::TYPE_BOOL:
        return "TType_BOOL";
      case t_base_type::TYPE_BYTE:
        return "TType_BYTE";
      case t_base_type::TYPE_I16:
        return "TType_I16";
      case t_base_type::TYPE_I32:
        return "TType_I32";
      case t_base_type::TYPE_I64:
        return "TType_I64";
      case t_base_type::TYPE_DOUBLE:
        return "TType_DOUBLE";
      case t_base_type::TYPE_FLOAT:
        return "TType_FLOAT";
    }
  } else if (type->is_enum()) {
    return "TType_I32";
  } else if (type->is_struct() || type->is_exception()) {
    return "TType_STRUCT";
  } else if (type->is_map()) {
    return "TType_MAP";
  } else if (type->is_set()) {
    return "TType_SET";
  } else if (type->is_list()) {
    return "TType_LIST";
  }

  throw std::runtime_error("INVALID TYPE IN type_to_enum: " + type->name());
}

/**
 * Returns a format std::string specifier for the supplied parse type.
 */
std::string t_cocoa_generator::format_string_for_type(const t_type* type) {
  type = type->get_true_type();

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw std::runtime_error("NO T_VOID CONSTRUCT");
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_BINARY:
        return "\\\"%@\\\"";
      case t_base_type::TYPE_BOOL:
        return "%i";
      case t_base_type::TYPE_BYTE:
        return "%i";
      case t_base_type::TYPE_I16:
        return "%hi";
      case t_base_type::TYPE_I32:
        return "%i";
      case t_base_type::TYPE_I64:
        return "%qi";
      case t_base_type::TYPE_DOUBLE:
        return "%f";
      case t_base_type::TYPE_FLOAT:
        return "%f";
    }
  } else if (type->is_enum()) {
    return "%i";
  } else if (type->is_struct() || type->is_exception()) {
    return "%@";
  } else if (type->is_map()) {
    return "%@";
  } else if (type->is_set()) {
    return "%@";
  } else if (type->is_list()) {
    return "%@";
  }

  throw std::runtime_error(
      "INVALID TYPE IN format_string_for_type: " + type->name());
}

/**
 * Generate a call to a field's setter.
 *
 * @param tfield Field the setter is being called on
 * @param fieldName Name of variable to pass to setter
 */

std::string t_cocoa_generator::call_field_setter(
    const t_field* tfield, const std::string& fieldName) {
  return "[self set" + capitalize(get_cocoa_property_name(tfield)) + ": " +
      fieldName + "];";
}

std::string t_cocoa_generator::get_cocoa_property_name(const t_field* tfield) {
  return tfield->get_annotation("cocoa.name", &tfield->name());
}

THRIFT_REGISTER_GENERATOR(
    cocoa,
    "Cocoa",
    "    import_path=XYZ: Override thrift package import path\n"
    "    log_unexpected:  Log every time an unexpected field ID or type is "
    "encountered.\n"
    "    nullability:     Use annotations to ensure required fields are "
    "present.\n"
    "    validate_required:\n"
    "                     Throws exception if any required field is not "
    "set.\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
