/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/system/systemlib.h"
#include "hphp/runtime/vm/decl-provider.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/base/execution-context.h"

TRACE_SET_MOD(decl);

namespace HPHP {

namespace {

  // Class
  const StaticString
    s_name("name"),
    s_kind("kind"),
    s_type("type"),
    s_module("module"),
    s_as_constraint("as_constraint"),
    s_super_constraint("super_constraint"),
    s_docs_url("docs_url"),
    s_extends("extends"),
    s_uses("uses"),
    s_implements("implements"),
    s_where_constraints("where_constraints"),
    s_xhp_attr_uses("xhp_attributed_uses"),
    s_visibility("visibility"),
    s_base_types("base_types"),
    s_base("base"),
    s_constraint("constraint"),
    s_attributes("attributes"),
    s_require_class("require_class"),
    s_require_extends("require_extends"),
    s_require_implements("require_implements"),
    s_enum_type("enum_type"),
    s_constructor("constructor"),
    s_consts("consts"),
    s_typeconsts("typeconsts"),
    s_static_methods("static_methods"),
    s_variance("variance"),
    s_includes("includes"),
    s_tparams("tparams"),
    s_params("params"),
    s_implicit_params("implicit_params"),
    s_cross_package("cross_package"),
    s_constraints("constraints"),
    s_reified("reified"),
    s_signature_type("signature_type"),
    s_signature("signature"),
    s_return_type("return_type"),
    s_props("props"),
    s_sprops("static_props"),
    s_imports("imports"),
    s_exports("exports"),
    s_methods("methods"),
    s_args("args");

  // Bools
  const StaticString
    s_is_abstract("is_abstract"),
    s_is_final("is_final"),
    s_is_xhp("is_xhp"),
    s_has_xhp("has_xhp_keyword"),
    s_is_xhp_marked_empty("is_xhp_marked_empty"),
    s_is_internal("is_internal"),
    s_is_strict("is_strict"),
    s_disable_xhp_element_mangling("disable_xhp_element_mangling"),
    s_has_first_pass_parse_errors("has_first_pass_parse_errors"),
    s_is_multiple("is_multiple_declarations"),
    s_is_override("is_override"),
    s_is_dyncall("is_dynamically_callable"),
    s_is_ctx("is_ctx"),
    s_is_enforceable("is_enforceable"),
    s_is_reifiable("is_reifiable"),
    s_is_soft_type("is_soft_type"),
    s_is_soft_return_type("is_soft_return_type");

  // Method signature Bools
  const StaticString
    s_is_return_disposable("is_return_disposable"),
    s_is_coroutine("is_coroutine"),
    s_is_async("is_async"),
    s_is_generator("is_generator"),
    s_is_instantiated_targs("is_instantiated_targs"),
    s_is_function_pointer("is_function_pointer"),
    s_is_returns_readonly("is_returns_readonly"),
    s_is_readonly_this("is_readonly_this"),
    s_is_support_dynamic_type("is_support_dynamic_type"),
    s_is_memoized("is_memoized"),
    s_is_variadic("is_variadic"),
    s_is_accept_disposable("is_accept_disposable"),
    s_is_inout("is_inout"),
    s_has_default("has_default"),
    s_is_ifc_external("is_ifc_external"),
    s_is_ifc_can_call("is_ifc_can_call"),
    s_is_readonly("is_readonly"),
    s_is_const("is_const"),
    s_is_lateinit("is_lateinit"),
    s_is_lsb("is_lsb"),
    s_is_needs_init("is_needs_init"),
    s_is_php_std_lib("is_php_std_lib"),
    s_is_safe_global_variable("is_safe_global_variable"),
    s_is_no_auto_likes("is_no_auto_likes"),
    s_is_no_dynamic("is_no_auto_dynamic");

  std::optional<hackc::DeclParserConfig> m_config;

  void assertEnv() {
    if (RuntimeOption::RepoAuthoritative) {
      SystemLib::throwInvalidOperationExceptionObject(
        "Cannot use FileDecls in RepoAuthoritative mode");
    }
  }

  void initDeclConfig() {
    if (!m_config.has_value()) {
      const RepoOptions& options = g_context->getRepoOptionsForCurrentFrame();
      hackc::DeclParserConfig config;
      options.flags().initDeclConfig(config);
      m_config = config;
    }
  }

  String rustToString(const rust::String& str) {
    return String(str.data(), str.size(), CopyStringMode::CopyString);
  }

  Array populateStringArray(const rust::Vec<rust::String>& vec) {
    if (vec.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& str : vec) {
      arr.append(String(str.data(), str.size(), CopyStringMode::CopyString));
    }
    return arr;
  }

  #define MAYBE_SET(fval, fname, fn) \
    if (!fval.empty()) { \
      info.set(fname, fn(fval)); \
    }

  #define MAYBE_SET_FIRST(fval, fname, fn) \
    if (!fval.empty()) { \
      info.set(fname, fn(fval)[0]); \
    }

  #define MAYBE_SET_BOOL(fval, fname) \
    if (fval) { \
      info.set(fname, fval); \
    }

  #define MAYBE_FLAG(fval, fname, flag) \
      if ((fval & flag) != 0) { \
        info.set(fname, true); \
      }

  /*
   * Maps the rust type constraints to their hack shape equivalent
   */
  Array populateTypeConstraints(const rust::Vec<hackc::ExtDeclTypeConstraint>& constraints) {
    if (constraints.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& c : constraints) {
      Array info = Array::CreateDict();
      info.set(s_kind, rustToString(c.kind));
      info.set(s_type, rustToString(c.type_));
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust enum types to their hack shape equivalent
   */
  Array populateEnumType(const rust::Vec<hackc::ExtDeclEnumType>& entypes) {
    if (entypes.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& et : entypes) {
      Array info = Array::CreateDict();
      info.set(s_base, rustToString(et.base));
      info.set(s_constraint, rustToString(et.constraint));
      MAYBE_SET(et.includes, s_includes, populateStringArray)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust attributes to their hack shape equivalent
   */
  Array populateAttributes(const rust::Vec<hackc::ExtDeclAttribute>& attrs) {
    if (attrs.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& attr : attrs) {
      Array info = Array::CreateDict();
      info.set(s_name, rustToString(attr.name));
      MAYBE_SET(attr.args, s_args, populateStringArray)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust TParams to their hack shape equivalent
   */
  Array populateTParams(const rust::Vec<hackc::ExtDeclTparam>& tparams) {
    if (tparams.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& tp : tparams) {
      Array info = Array::CreateDict();
      info.set(s_name, rustToString(tp.name));
      info.set(s_variance, rustToString(tp.variance));
      info.set(s_reified, rustToString(tp.reified));
      MAYBE_SET(tp.user_attributes, s_attributes, populateAttributes)
      MAYBE_SET(tp.tparams, s_tparams, populateTParams)
      MAYBE_SET(tp.constraints, s_constraints, populateTypeConstraints)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust class constants to their hack shape equivalent
   */
  Array populateConstants(const rust::Vec<hackc::ExtDeclClassConst>& consts) {
    if (consts.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& c : consts) {
      Array info = Array::CreateDict();
      info.set(s_name, rustToString(c.name));
      info.set(s_type, rustToString(c.type_));
      MAYBE_SET_BOOL(c.is_abstract, s_is_abstract)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust class type consts to their hack shape equivalent
   */
  Array populateTypeConstants(const rust::Vec<hackc::ExtDeclClassTypeConst>& typeconsts) {
    if (typeconsts.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& tc : typeconsts) {
      Array info = Array::CreateDict();
      info.set(s_name, rustToString(tc.name));
      info.set(s_kind, rustToString(tc.kind));
      MAYBE_SET_BOOL(tc.is_ctx, s_is_ctx)
      MAYBE_SET_BOOL(tc.enforceable, s_is_enforceable)
      MAYBE_SET_BOOL(tc.reifiable, s_is_reifiable)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust properties to their hack shape equivalent
   */
  Array populateProps(const rust::Vec<::HPHP::hackc::ExtDeclProp>& props) {
    if (props.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& prop : props) {
      Array info = Array::CreateDict();
      info.set(s_name, rustToString(prop.name));
      info.set(s_type, rustToString(prop.type_));
      info.set(s_visibility, rustToString(prop.visibility));
      MAYBE_FLAG(prop.flags, s_is_abstract, 1 << 0)
      MAYBE_FLAG(prop.flags, s_is_const, 1 << 1)
      MAYBE_FLAG(prop.flags, s_is_lateinit, 1 << 2)
      MAYBE_FLAG(prop.flags, s_is_lsb, 1 << 3)
      MAYBE_FLAG(prop.flags, s_is_needs_init, 1 << 4)
      MAYBE_FLAG(prop.flags, s_is_php_std_lib, 1 << 5)
      MAYBE_FLAG(prop.flags, s_is_readonly, 1 << 6)
      MAYBE_FLAG(prop.flags, s_is_safe_global_variable, 1 << 7)
      MAYBE_FLAG(prop.flags, s_is_no_auto_likes, 1 << 8)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust methods to their hack shape equivalent
   */
   Array populateMethodParams(const rust::Vec<::HPHP::hackc::ExtDeclMethodParam>& params) {
    if (params.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& param : params) {
      Array info = Array::CreateDict();
      info.set(s_name, rustToString(param.name));
      info.set(s_type, rustToString(param.type_));
      MAYBE_SET_BOOL(!param.enforced_type, s_is_soft_type)
      MAYBE_FLAG(param.flags, s_is_accept_disposable, 1 << 0)
      MAYBE_FLAG(param.flags, s_is_inout, 1 << 1)
      MAYBE_FLAG(param.flags, s_has_default, 1 << 2)
      MAYBE_FLAG(param.flags, s_is_ifc_external, 1 << 3)
      MAYBE_FLAG(param.flags, s_is_ifc_can_call, 1 << 4)
      MAYBE_FLAG(param.flags, s_is_readonly, 1 << 8)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps the rust method signatures (TFun) to their hack shape equivalent
   */
  Array populateSignature(const rust::Vec<::HPHP::hackc::ExtDeclSignature>& sigs) {
    for (auto const& sig : sigs) {
      // There would always be at most one here
      Array info = Array::CreateDict();

      info.set(s_return_type, rustToString(sig.return_type));
      MAYBE_SET(sig.tparams, s_tparams, populateTParams)
      MAYBE_SET(sig.where_constraints, s_where_constraints, populateTypeConstraints)
      MAYBE_SET_BOOL(!sig.return_enforced, s_is_soft_return_type)
      MAYBE_SET(sig.params, s_params, populateMethodParams)
      MAYBE_SET(sig.implicit_params, s_implicit_params, rustToString)
      MAYBE_SET(sig.cross_package, s_cross_package, rustToString)
      MAYBE_FLAG(sig.flags, s_is_return_disposable, 1 << 0)
      MAYBE_FLAG(sig.flags, s_is_coroutine, 1 << 3)
      MAYBE_FLAG(sig.flags, s_is_async, 1 << 4)
      MAYBE_FLAG(sig.flags, s_is_generator, 1 << 5)
      MAYBE_FLAG(sig.flags, s_is_instantiated_targs, 1 << 8)
      MAYBE_FLAG(sig.flags, s_is_function_pointer, 1 << 9)
      MAYBE_FLAG(sig.flags, s_is_returns_readonly, 1 << 10)
      MAYBE_FLAG(sig.flags, s_is_readonly_this, 1 << 11)
      MAYBE_FLAG(sig.flags, s_is_support_dynamic_type, 1 << 12)
      MAYBE_FLAG(sig.flags, s_is_memoized, 1 << 13)
      MAYBE_FLAG(sig.flags, s_is_variadic, 1 << 14)

      Array arr = Array::CreateVec();
      arr.append(info);
      return arr;
    }

    return empty_vec_array();
  }

  /*
   * Maps the rust methods to their hack shape equivalent
   */
  Array populateMethods(const rust::Vec<::HPHP::hackc::ExtDeclMethod>& meths) {
    if (meths.empty()) {
      return empty_vec_array();
    }

    Array arr = Array::CreateVec();
    for (auto const& meth : meths) {
      Array info = Array::CreateDict();
      info.set(s_name, rustToString(meth.name));
      info.set(s_signature_type, rustToString(meth.type_));
      info.set(s_visibility, rustToString(meth.visibility));
      MAYBE_SET(meth.attributes, s_attributes, populateAttributes)
      MAYBE_SET_FIRST(meth.signature, s_signature, populateSignature)
      MAYBE_FLAG(meth.flags, s_is_abstract, 1 << 0)
      MAYBE_FLAG(meth.flags, s_is_final, 1 << 1)
      MAYBE_FLAG(meth.flags, s_is_override, 1 << 2)
      MAYBE_FLAG(meth.flags, s_is_dyncall, 1 << 3)
      MAYBE_FLAG(meth.flags, s_is_php_std_lib, 1 << 4)
      MAYBE_FLAG(meth.flags, s_is_support_dynamic_type, 1 << 5)
      arr.append(info);
    }
    return arr;
  }

  /*
   * Maps a rust class to its hack shape equivalent
   */
  Array populateClass(const hackc::ExtDeclClass& kls) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(kls.name));

    // Derived from ClassishKind
    // class / interface / trait / enum / enum_class
    info.set(s_kind, rustToString(kls.kind));

    MAYBE_SET(kls.module, s_module, rustToString)
    MAYBE_SET(kls.docs_url, s_docs_url, rustToString)

    MAYBE_SET_BOOL(kls.final_, s_is_final)
    MAYBE_SET_BOOL(kls.abstract_, s_is_abstract)
    MAYBE_SET_BOOL(kls.internal, s_is_internal)
    MAYBE_SET_BOOL(kls.is_strict, s_is_strict)
    MAYBE_SET_BOOL(kls.support_dynamic_type, s_is_support_dynamic_type)

    MAYBE_SET(kls.extends, s_extends, populateStringArray)
    MAYBE_SET(kls.uses, s_uses, populateStringArray)
    MAYBE_SET(kls.implements, s_implements, populateStringArray)
    MAYBE_SET(kls.require_extends, s_require_extends, populateStringArray)
    MAYBE_SET(kls.require_implements, s_require_implements, populateStringArray)
    MAYBE_SET(kls.require_class, s_require_class, populateStringArray)

    // XHP related
    MAYBE_SET_BOOL(kls.is_xhp, s_is_xhp)
    MAYBE_SET_BOOL(kls.has_xhp_keyword, s_has_xhp)
    MAYBE_SET_BOOL(kls.xhp_marked_empty, s_is_xhp_marked_empty)
    MAYBE_SET(kls.xhp_attr_uses, s_xhp_attr_uses, populateStringArray)

    // Complex Types
    MAYBE_SET(kls.user_attributes, s_attributes, populateAttributes)
    MAYBE_SET(kls.methods, s_methods, populateMethods)
    MAYBE_SET(kls.static_methods, s_static_methods, populateMethods)
    MAYBE_SET_FIRST(kls.constructor, s_constructor, populateMethods)
    MAYBE_SET(kls.typeconsts, s_typeconsts, populateTypeConstants)
    MAYBE_SET(kls.consts, s_consts, populateConstants)
    MAYBE_SET(kls.where_constraints, s_where_constraints, populateTypeConstraints)
    MAYBE_SET(kls.tparams, s_tparams, populateTParams)
    MAYBE_SET_FIRST(kls.enum_type, s_enum_type, populateEnumType)
    MAYBE_SET(kls.props, s_props, populateProps)
    MAYBE_SET(kls.sprops, s_sprops, populateProps)

    return info;
  }
}

/*
 * Maps the rust classes to their hack shape equivalent
 */
Array populateClasses(const rust::Vec<::HPHP::hackc::ExtDeclClass>& classes) {
  if (classes.empty()) {
    return empty_vec_array();
  }

  Array arr = Array::CreateVec();
  for (auto const& kls : classes) {
    arr.append(populateClass(kls));
  }
  return arr;
}

/*
 * Maps the rust file consts to their hack shape equivalent
 */
Array populateFileConsts(const rust::Vec<::HPHP::hackc::ExtDeclFileConst>& consts) {
  if (consts.empty()) {
    return empty_vec_array();
  }

  Array arr = Array::CreateVec();
  for (auto const& c : consts) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(c.name));
    info.set(s_type, rustToString(c.type_));
    arr.append(info);
  }
  return arr;
}

/*
 * Maps the rust functions to their hack shape equivalent
 */
Array populateFileFuncs(const rust::Vec<::HPHP::hackc::ExtDeclFileFunc>& funs) {
  if (funs.empty()) {
    return empty_vec_array();
  }

  Array arr = Array::CreateVec();
  for (auto const& fun : funs) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(fun.name));
    info.set(s_signature_type, rustToString(fun.type_));
    MAYBE_SET(fun.module, s_module, rustToString)
    MAYBE_SET_BOOL(fun.internal, s_is_internal)
    MAYBE_SET_BOOL(fun.php_std_lib, s_is_php_std_lib)
    MAYBE_SET_BOOL(fun.support_dynamic_type, s_is_support_dynamic_type)
    MAYBE_SET_BOOL(fun.no_auto_dynamic, s_is_no_dynamic)
    MAYBE_SET_BOOL(fun.no_auto_likes, s_is_no_auto_likes)
    MAYBE_SET_FIRST(fun.signature, s_signature, populateSignature)
    arr.append(info);
  }
  return arr;
}

/*
 * Maps the rust module definitions to their hack shape equivalent
 */
Array populateModules(const rust::Vec<::HPHP::hackc::ExtDeclModule>& modules) {
  if (modules.empty()) {
    return empty_vec_array();
  }

  Array arr = Array::CreateVec();
  for (auto const& module : modules) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(module.name));
    MAYBE_SET(module.imports, s_imports, populateStringArray)
    MAYBE_SET(module.exports, s_exports, populateStringArray)
    arr.append(info);
  }
  return arr;
}

/*
 * Maps the rust type definitions to their hack shape equivalent
 */
Array populateTypedefs(const rust::Vec<::HPHP::hackc::ExtDeclTypeDef>& typedefs) {
  if (typedefs.empty()) {
    return empty_vec_array();
  }

  Array arr = Array::CreateVec();
  for (auto const& td : typedefs) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(td.name));
    info.set(s_type, rustToString(td.type_));
    info.set(s_visibility, rustToString(td.visibility));
    MAYBE_SET(td.module, s_module, rustToString)
    MAYBE_SET(td.tparams, s_tparams, populateTParams)
    MAYBE_SET(td.as_constraint, s_as_constraint, rustToString)
    MAYBE_SET(td.super_constraint, s_super_constraint, rustToString)
    MAYBE_SET_BOOL(td.is_ctx, s_is_ctx)
    MAYBE_SET_BOOL(td.internal, s_is_internal)
    MAYBE_SET(td.docs_url, s_docs_url, rustToString)
    MAYBE_SET(td.attributes, s_attributes, populateAttributes)
    arr.append(info);
  }
  return arr;
}

/*
 * Maps the rust file to its hack shape equivalent
 */
Array populateFile(const ::HPHP::hackc::ExtDeclFile& file) {
  Array info = Array::CreateDict();
  MAYBE_SET(file.typedefs, s_attributes, populateTypedefs)
  MAYBE_SET(file.functions, s_attributes, populateFileFuncs)
  MAYBE_SET(file.constants, s_attributes, populateFileConsts)
  MAYBE_SET(file.file_attributes, s_attributes, populateAttributes)
  MAYBE_SET(file.modules, s_attributes, populateModules)
  MAYBE_SET(file.classes, s_attributes, populateClasses)
  MAYBE_SET_BOOL(file.disable_xhp_element_mangling, s_disable_xhp_element_mangling)
  MAYBE_SET_BOOL(file.has_first_pass_parse_errors, s_has_first_pass_parse_errors)
  MAYBE_SET_BOOL(file.is_strict, s_is_strict)
  return info;
}

///////////////////////////////////////////////////////////////////////////////

/*
  The native implementation of the FileDecls class. The class is accessible
  through Hack and exposes the methods of ext_decl extension to retrieve info.

  Every instance holds the results of the parsing in the rust DeclsHolder.
  The methods use the declsHolder to access the parsed information.
*/
struct FileDecls : SystemLib::ClassLoader<"FileDecls"> {
  FileDecls(){}
  FileDecls& operator=(const FileDecls& /*that_*/) {
    throw_not_implemented("FileDecls::operator=");
  }
  ~FileDecls() {}

  void validateState() {
    if (
      !this->declsHolder.has_value() ||
      this->error != empty_string()
    ) {
      SystemLib::throwInvalidOperationExceptionObject("FileDecls is in erroneous state");
    }
  }

  String error = empty_string();
  std::optional<rust::Box<hackc::DeclsHolder>> declsHolder;

  // Filename to DeclsHolder for hackc::parse_decls() results
  static hphp_hash_map<std::string, FileDecls> m_cache;
};

///////////////////////////////////////////////////////////////////////////////
// API

/*
 * Not implemented yet. Parses the content in the given path and returns
 * a new instance of FileDecls. This method may use a cache instead of parsing.
 */
Object HHVM_STATIC_METHOD(FileDecls, parsePath, const String& path) {
  assertEnv();
  Object obj{FileDecls::classof()};
  auto data = Native::data<FileDecls>(obj);
  initDeclConfig();
  data->error = String("Not implemented yet");
  return obj;
}

/*
 * Parses the provided text and returns a new instance of FileDecls.
 */
Object HHVM_STATIC_METHOD(FileDecls, parseText, const String& text) {
  assertEnv();
  Object obj{FileDecls::classof()};
  auto data = Native::data<FileDecls>(obj);
  initDeclConfig();
  try {
    data->declsHolder = hackc::parse_decls(
      m_config.value(),
      "",
      {(const uint8_t*)text.data(), (size_t)text.size()}
    );
  } catch (const std::exception& ex) {
    data->error = ex.what();
  }

  return obj;
}

/*
 * Returns a non empty string if the instance is in an erroneous state.
 * This can happen if parsing failed.
 */
static String HHVM_METHOD(FileDecls, getError) {
  auto data = Native::data<FileDecls>(this_);
  return data->error;
}

/*
 * Returns true if the parsed content contains a type or a class with
 * the given name.
 */
static bool HHVM_METHOD(FileDecls, hasType, const String& name) {
  if (name.empty()) {
    return false;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return hackc::type_exists(
    *data->declsHolder.value(),
    rust::String{name.data()}
  );
}

static Variant HHVM_METHOD(FileDecls, getFile) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decl = hackc::get_file(*data->declsHolder.value());
  return Variant(populateFile(decl));
}
static Array HHVM_METHOD(FileDecls, getClasses) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateClasses(hackc::get_classes(*data->declsHolder.value()).vec);
}
static Variant HHVM_METHOD(FileDecls, getClass, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class(*data->declsHolder.value(), rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateClass(decls.vec[0]);
}
static Array HHVM_METHOD(FileDecls, getFileAttributes) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateAttributes(hackc::get_file_attributes(*data->declsHolder.value()).vec);
}
static Variant HHVM_METHOD(FileDecls, getFileAttribute, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_file_attribute(*data->declsHolder.value(), rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateAttributes(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getFileConsts) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateFileConsts(hackc::get_file_consts(*data->declsHolder.value()).vec);
}
static Variant HHVM_METHOD(FileDecls, getFileConst, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_file_const(*data->declsHolder.value(), rust::String{name.data()});
    return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateFileConsts(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getFileFuncs) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateFileFuncs(hackc::get_file_funcs(*data->declsHolder.value()).vec);
}
static Variant HHVM_METHOD(FileDecls, getFileFunc, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_file_func(*data->declsHolder.value(), rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateFileFuncs(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getFileModules) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateModules(hackc::get_file_modules(*data->declsHolder.value()).vec);
}
static Variant HHVM_METHOD(FileDecls, getFileModule, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_file_module(*data->declsHolder.value(), rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateModules(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getFileTypedefs) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateTypedefs(hackc::get_file_typedefs(*data->declsHolder.value()).vec);
}
static Variant HHVM_METHOD(FileDecls, getFileTypedef, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_file_typedef(*data->declsHolder.value(), rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateTypedefs(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getMethods, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateMethods(hackc::get_class_methods(*data->declsHolder.value(), rust::String{kls.data()}).vec);
}
static Variant HHVM_METHOD(FileDecls, getMethod, const String& kls, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_method(*data->declsHolder.value(), rust::String{kls.data()}, rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateMethods(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getStaticMethods, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateMethods(hackc::get_class_smethods(*data->declsHolder.value(), rust::String{kls.data()}).vec);
}
static Variant HHVM_METHOD(FileDecls, getStaticMethod, const String& kls, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_smethod(*data->declsHolder.value(), rust::String{kls.data()}, rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateMethods(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getConsts, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateConstants(hackc::get_class_consts(*data->declsHolder.value(), rust::String{kls.data()}).vec);
}
static Variant HHVM_METHOD(FileDecls, getConst, const String& kls, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_const(*data->declsHolder.value(), rust::String{kls.data()}, rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateConstants(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getTypeconsts, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateTypeConstants(hackc::get_class_typeconsts(*data->declsHolder.value(), rust::String{kls.data()}).vec);
}
static Variant HHVM_METHOD(FileDecls, getTypeconst, const String& kls, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_typeconst(*data->declsHolder.value(), rust::String{kls.data()}, rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateTypeConstants(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getProps, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateProps(hackc::get_class_props(*data->declsHolder.value(), rust::String{kls.data()}).vec);
}
static Variant HHVM_METHOD(FileDecls, getProp, const String& kls, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_prop(*data->declsHolder.value(), rust::String{kls.data()}, rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateProps(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getStaticProps, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateProps(hackc::get_class_sprops(*data->declsHolder.value(), rust::String{kls.data()}).vec);
}
static Variant HHVM_METHOD(FileDecls, getStaticProp, const String& kls, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();

  // Static properties are prefixed with dollars, unlike instance properties.
  // This can cause some confusion in the querying API.
  String norm_name = name;
  if (name.charAt(0) != '$') {
    norm_name = String::FromChar('$')  + name;
  }

  auto const decls = hackc::get_class_sprop(*data->declsHolder.value(), rust::String{kls.data()}, rust::String{norm_name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateProps(decls.vec)[0];
}
static Array HHVM_METHOD(FileDecls, getAttributes, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateAttributes(hackc::get_class_attributes(*data->declsHolder.value(), rust::String{kls.data()}).vec);
}
static Variant HHVM_METHOD(FileDecls, getAttribute, const String& kls, const String& name) {
  if (name.empty()) {
    return Variant(Variant::NullInit());
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_attribute(*data->declsHolder.value(), rust::String{kls.data()}, rust::String{name.data()});
  return decls.vec.empty()
    ? Variant(Variant::NullInit())
    : populateAttributes(decls.vec)[0];
}

///////////////////////////////////////////////////////////////////////////////
// Extension

struct DeclExtension final : Extension {
  DeclExtension() : Extension("decl", "0.1", NO_ONCALL_YET) {}

  const DependencySet getDeps() const override {
    // ensure ext_facts initializes and loads first.
    return DependencySet({"facts"});
  }

  void moduleInit() override {
    HHVM_ME(FileDecls, getError);
    HHVM_ME(FileDecls, hasType);
    HHVM_ME(FileDecls, getClass);
    HHVM_STATIC_ME(FileDecls, parseText);
    HHVM_STATIC_ME(FileDecls, parsePath);

    HHVM_ME(FileDecls, getMethods);
    HHVM_ME(FileDecls, getMethod);
    HHVM_ME(FileDecls, getStaticMethods);
    HHVM_ME(FileDecls, getStaticMethod);
    HHVM_ME(FileDecls, getConsts);
    HHVM_ME(FileDecls, getConst);
    HHVM_ME(FileDecls, getTypeconsts);
    HHVM_ME(FileDecls, getTypeconst);
    HHVM_ME(FileDecls, getProps);
    HHVM_ME(FileDecls, getProp);
    HHVM_ME(FileDecls, getStaticProps);
    HHVM_ME(FileDecls, getStaticProp);
    HHVM_ME(FileDecls, getAttributes);
    HHVM_ME(FileDecls, getAttribute);
    HHVM_ME(FileDecls, getClasses);
    HHVM_ME(FileDecls, getClass);
    HHVM_ME(FileDecls, getFileAttributes);
    HHVM_ME(FileDecls, getFileAttribute);
    HHVM_ME(FileDecls, getFileConsts);
    HHVM_ME(FileDecls, getFileConst);
    HHVM_ME(FileDecls, getFileFuncs);
    HHVM_ME(FileDecls, getFileFunc);
    HHVM_ME(FileDecls, getFileModules);
    HHVM_ME(FileDecls, getFileModule);
    HHVM_ME(FileDecls, getFileTypedefs);
    HHVM_ME(FileDecls, getFileTypedef);
    HHVM_ME(FileDecls, getFile);


    Native::registerNativeDataInfo<FileDecls>();
  }

  std::vector<std::string> hackFiles() const override {
    return {
      "decl",
    };
  }
} s_decl_extension;

}
