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

#include <folly/Executor.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/decl/decl-extractor.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/facts/thread-factory.h"
#include "hphp/runtime/vm/decl-provider.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/concurrent-lru-cache.h"
#include "hphp/zend/zend-string.h"

#include <fstream>
#include <memory>
#include <optional>

TRACE_SET_MOD(decl)

namespace HPHP {

namespace {
Decl::ExtractorConfig s_extractorConfig{};
std::unique_ptr<folly::IOThreadPoolExecutor> s_executor;
// File
const StaticString s_typedefs("typedefs");
const StaticString s_functions("functions");
const StaticString s_modules("modules");
const StaticString s_classes("classes");

// Class
const StaticString s_name("name");
const StaticString s_kind("kind");
const StaticString s_type("type");
const StaticString s_module("module");
const StaticString s_as_constraint("as_constraint");
const StaticString s_super_constraint("super_constraint");
const StaticString s_docs_url("docs_url");
const StaticString s_extends("extends");
const StaticString s_uses("uses");
const StaticString s_implements("implements");
const StaticString s_where_constraints("where_constraints");
const StaticString s_xhp_attr_uses("xhp_attributed_uses");
const StaticString s_visibility("visibility");
const StaticString s_base_types("base_types");
const StaticString s_base("base");
const StaticString s_constraint("constraint");
const StaticString s_attributes("attributes");
const StaticString s_require_class("require_class");
const StaticString s_require_extends("require_extends");
const StaticString s_require_implements("require_implements");
const StaticString s_enum_type("enum_type");
const StaticString s_constructor("constructor");
const StaticString s_consts("consts");
const StaticString s_typeconsts("typeconsts");
const StaticString s_static_methods("static_methods");
const StaticString s_variance("variance");
const StaticString s_includes("includes");
const StaticString s_tparams("tparams");
const StaticString s_params("params");
const StaticString s_implicit_params("implicit_params");
const StaticString s_cross_package("cross_package");
const StaticString s_constraints("constraints");
const StaticString s_reified("reified");
const StaticString s_signature_type("signature_type");
const StaticString s_signature("signature");
const StaticString s_return_type("return_type");
const StaticString s_props("props");
const StaticString s_sprops("static_props");
const StaticString s_imports("imports");
const StaticString s_exports("exports");
const StaticString s_methods("methods");
const StaticString s_args("args");
const StaticString s_raw_val("raw_val");
const StaticString s_subtypes("subtypes");
const StaticString s_value("value");

// Bools
const StaticString s_is_abstract("is_abstract");
const StaticString s_is_final("is_final");
const StaticString s_is_xhp("is_xhp");
const StaticString s_has_xhp("has_xhp_keyword");
const StaticString s_is_xhp_marked_empty("is_xhp_marked_empty");
const StaticString s_is_internal("is_internal");
const StaticString s_is_strict("is_strict");
const StaticString s_disable_xhp_element_mangling(
    "disable_xhp_element_mangling");
const StaticString s_has_first_pass_parse_errors("has_first_pass_parse_errors");
const StaticString s_is_multiple("is_multiple_declarations");
const StaticString s_is_override("is_override");
const StaticString s_is_dyncall("is_dynamically_callable");
const StaticString s_is_ctx("is_ctx");
const StaticString s_is_enforceable("is_enforceable");
const StaticString s_is_reifiable("is_reifiable");
const StaticString s_is_nullable("is_nullable");
const StaticString s_is_optional("is_optional");

// Method signature Bools
const StaticString s_is_return_disposable("is_return_disposable");
const StaticString s_is_coroutine("is_coroutine");
const StaticString s_is_async("is_async");
const StaticString s_is_generator("is_generator");
const StaticString s_is_instantiated_targs("is_instantiated_targs");
const StaticString s_is_function_pointer("is_function_pointer");
const StaticString s_is_returns_readonly("is_returns_readonly");
const StaticString s_is_readonly_this("is_readonly_this");
const StaticString s_is_support_dynamic_type("is_support_dynamic_type");
const StaticString s_is_memoized("is_memoized");
const StaticString s_is_variadic("is_variadic");
const StaticString s_is_accept_disposable("is_accept_disposable");
const StaticString s_is_inout("is_inout");
const StaticString s_has_default("has_default");
const StaticString s_default_value("default_value");
const StaticString s_is_readonly("is_readonly");
const StaticString s_is_const("is_const");
const StaticString s_is_lateinit("is_lateinit");
const StaticString s_is_lsb("is_lsb");
const StaticString s_is_needs_init("is_needs_init");
const StaticString s_is_php_std_lib("is_php_std_lib");
const StaticString s_is_safe_global_variable("is_safe_global_variable");
const StaticString s_is_no_auto_likes("is_no_auto_likes");
const StaticString s_is_no_dynamic("is_no_auto_dynamic");

void assertEnv() {
  if (Cfg::Repo::Authoritative) {
    SystemLib::throwInvalidOperationExceptionObject(
        "Cannot use FileDecls in RepoAuthoritative mode");
  }
}

std::string getRepoRootForFile(const String& path) {
  return RepoOptions::forFile(path.toCppString()).dir();
}

const folly::Executor::KeepAlive<folly::Executor> getExecutor() {
  if (s_executor == nullptr) {
    s_executor = std::make_unique<folly::IOThreadPoolExecutor>(
        1, Facts::make_thread_factory("DeclExtractor"));
  }
  return folly::getKeepAliveToken(s_executor.get());
}

hackc::DeclParserConfig initDeclConfig(std::string& path) {
  hackc::DeclParserConfig config;
  auto opts = RepoOptions::forFile(path);
  opts.flags().initDeclConfig(config);
  config.include_assignment_values = true;
  return config;
}

hackc::DeclParserConfig initDeclConfig() {
  hackc::DeclParserConfig config;
  auto opts = *g_context->getRepoOptionsForRequest();
  opts.flags().initDeclConfig(config);
  config.include_assignment_values = true;
  return config;
}

// Returns the SHA1 hash of the contents of the file at filePath.
// If we can supply the hash from Eden, do so.
// Else, take the slow path and compute the hash from the file contents.
SHA1 computeSHA1(const std::string& filePath, const std::string& root) {
  auto edenHash = getHashForFile(filePath, root);
  if (edenHash.has_value()) {
    return edenHash.value();
  }
  auto text = Decl::readFile(filePath);
  auto sha1 = HPHP::string_sha1(text);
  return SHA1{sha1};
}

// Allocate an HPHP::String copied from a rust::String
String rustToString(const rust::String& str) {
  return String(str.data(), str.size(), CopyStringMode::CopyString);
}

// Make a rust::Str slice pointing to the content of an HPHP::String
rust::Str toRustStr(const String& str) {
  return rust::Str{str.data(), (size_t)str.size()};
}

Array populateStringArray(const rust::Vec<rust::String>& vec) {
  if (vec.empty()) {
    return empty_vec_array();
  }

  VecInit arr{vec.size()};
  for (auto const& str : vec) {
    arr.append(String(str.data(), str.size(), CopyStringMode::CopyString));
  }
  return arr.toArray();
}

template <typename T, typename F>
void maybeSet(Array& info, const T& fval, const StaticString& fname, F fn) {
  if (!fval.empty()) {
    info.set(fname, fn(fval));
  }
}

template <typename T, typename F>
void maybeSetFirst(
    Array& info,
    const T& fval,
    const StaticString& fname,
    F fn) {
  if (!fval.empty()) {
    info.set(fname, fn(fval)[0]);
  }
}

void maybeSetBool(Array& info, bool fval, const StaticString& fname) {
  if (fval) {
    info.set(fname, true);
  }
}

/*
 * Maps the Rust type constraints to their Hack shape equivalent
 */
Array populateTypeConstraints(
    const rust::Vec<hackc::ExtDeclTypeConstraint>& constraints) {
  if (constraints.empty()) {
    return empty_vec_array();
  }

  VecInit arr{constraints.size()};
  for (auto const& c : constraints) {
    Array info = Array::CreateDict();
    info.set(s_kind, rustToString(c.kind));
    info.set(s_type, rustToString(c.type_));
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust enum types to their Hack shape equivalent
 */
Array populateEnumType(const rust::Vec<hackc::ExtDeclEnumType>& entypes) {
  if (entypes.empty()) {
    return empty_vec_array();
  }

  VecInit arr{entypes.size()};
  for (auto const& et : entypes) {
    Array info = Array::CreateDict();
    info.set(s_base, rustToString(et.base));
    info.set(s_constraint, rustToString(et.constraint));
    maybeSet(info, et.includes, s_includes, populateStringArray);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust attributes to their Hack shape equivalent
 */
Array populateAttributes(const rust::Vec<hackc::ExtDeclAttribute>& attrs) {
  if (attrs.empty()) {
    return empty_vec_array();
  }

  VecInit arr{attrs.size()};
  for (auto const& attr : attrs) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(attr.name));
    maybeSet(info, attr.args, s_args, populateStringArray);
    maybeSet(info, attr.raw_val, s_raw_val, rustToString);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust TParams to their Hack shape equivalent
 */
Array populateTParams(const rust::Vec<hackc::ExtDeclTparam>& tparams) {
  if (tparams.empty()) {
    return empty_vec_array();
  }

  VecInit arr{tparams.size()};
  for (auto const& tp : tparams) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(tp.name));
    info.set(s_variance, rustToString(tp.variance));
    info.set(s_reified, rustToString(tp.reified));
    maybeSet(info, tp.user_attributes, s_attributes, populateAttributes);
    maybeSet(info, tp.constraints, s_constraints, populateTypeConstraints);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust class constants to their Hack shape equivalent
 */
Array populateConstants(const rust::Vec<hackc::ExtDeclClassConst>& consts) {
  if (consts.empty()) {
    return empty_vec_array();
  }

  VecInit arr{consts.size()};
  for (auto const& c : consts) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(c.name));
    info.set(s_type, rustToString(c.type_));
    maybeSetBool(info, c.is_abstract, s_is_abstract);
    maybeSet(info, c.value, s_value, rustToString);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust class type consts to their Hack shape equivalent
 */
Array populateTypeConstants(
    const rust::Vec<hackc::ExtDeclClassTypeConst>& typeconsts) {
  if (typeconsts.empty()) {
    return empty_vec_array();
  }

  VecInit arr{typeconsts.size()};
  for (auto const& tc : typeconsts) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(tc.name));
    info.set(s_kind, rustToString(tc.kind));
    maybeSetBool(info, tc.is_ctx, s_is_ctx);
    maybeSetBool(info, tc.enforceable, s_is_enforceable);
    maybeSetBool(info, tc.reifiable, s_is_reifiable);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust properties to their Hack shape equivalent
 */
Array populateProps(const rust::Vec<hackc::ExtDeclProp>& props) {
  if (props.empty()) {
    return empty_vec_array();
  }

  VecInit arr{props.size()};
  for (auto const& prop : props) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(prop.name));
    info.set(s_type, rustToString(prop.type_));
    info.set(s_visibility, rustToString(prop.visibility));
    maybeSetBool(info, prop.is_abstract, s_is_abstract);
    maybeSetBool(info, prop.is_const, s_is_const);
    maybeSetBool(info, prop.is_lateinit, s_is_lateinit);
    maybeSetBool(info, prop.is_lsb, s_is_lsb);
    maybeSetBool(info, prop.needs_init, s_is_needs_init);
    maybeSetBool(info, prop.is_php_std_lib, s_is_php_std_lib);
    maybeSetBool(info, prop.is_readonly, s_is_readonly);
    maybeSetBool(info, prop.is_safe_global_variable, s_is_safe_global_variable);
    maybeSetBool(info, prop.no_auto_likes, s_is_no_auto_likes);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust methods to their Hack shape equivalent
 */
Array populateMethodParams(const rust::Vec<hackc::ExtDeclMethodParam>& params) {
  if (params.empty()) {
    return empty_vec_array();
  }

  VecInit arr{params.size()};
  for (auto const& param : params) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(param.name));
    info.set(s_type, rustToString(param.type_));
    maybeSetBool(info, param.accept_disposable, s_is_accept_disposable);
    maybeSetBool(info, param.is_inout, s_is_inout);
    maybeSetBool(info, param.has_default, s_has_default);
    maybeSetBool(info, param.is_readonly, s_is_readonly);
    maybeSetBool(info, param.is_optional, s_is_optional);
    maybeSet(info, param.def_value, s_default_value, rustToString);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust method signatures (TFun) to their Hack shape equivalent
 */
Array populateSignature(const rust::Vec<hackc::ExtDeclSignature>& sigs) {
  for (auto const& sig : sigs) {
    // There would always be at most one here
    Array info = Array::CreateDict();

    info.set(s_return_type, rustToString(sig.return_type));
    maybeSet(info, sig.tparams, s_tparams, populateTParams);
    maybeSet(
        info,
        sig.where_constraints,
        s_where_constraints,
        populateTypeConstraints);
    maybeSet(info, sig.params, s_params, populateMethodParams);
    maybeSet(info, sig.implicit_params, s_implicit_params, rustToString);
    maybeSet(info, sig.cross_package, s_cross_package, rustToString);
    maybeSetBool(info, sig.return_disposable, s_is_return_disposable);
    maybeSetBool(info, sig.is_coroutine, s_is_coroutine);
    maybeSetBool(info, sig.is_async, s_is_async);
    maybeSetBool(info, sig.is_generator, s_is_generator);
    maybeSetBool(info, sig.instantiated_targs, s_is_instantiated_targs);
    maybeSetBool(info, sig.is_function_pointer, s_is_function_pointer);
    maybeSetBool(info, sig.returns_readonly, s_is_returns_readonly);
    maybeSetBool(info, sig.readonly_this, s_is_readonly_this);
    maybeSetBool(info, sig.support_dynamic_type, s_is_support_dynamic_type);
    maybeSetBool(info, sig.is_memoized, s_is_memoized);
    maybeSetBool(info, sig.variadic, s_is_variadic);

    Array arr = Array::CreateVec();
    arr.append(info);
    return arr;
  }

  return empty_vec_array();
}

/*
 * Maps the Rust methods to their Hack shape equivalent
 */
Array populateMethods(const rust::Vec<hackc::ExtDeclMethod>& meths) {
  if (meths.empty()) {
    return empty_vec_array();
  }

  VecInit arr{meths.size()};
  for (auto const& meth : meths) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(meth.name));
    info.set(s_signature_type, rustToString(meth.type_));
    info.set(s_visibility, rustToString(meth.visibility));
    maybeSet(info, meth.attributes, s_attributes, populateAttributes);
    maybeSetFirst(info, meth.signature, s_signature, populateSignature);
    maybeSetBool(info, meth.is_abstract, s_is_abstract);
    maybeSetBool(info, meth.is_final, s_is_final);
    maybeSetBool(info, meth.is_override, s_is_override);
    maybeSetBool(info, meth.is_dynamicallycallable, s_is_dyncall);
    maybeSetBool(info, meth.is_php_std_lib, s_is_php_std_lib);
    maybeSetBool(info, meth.supports_dynamic_type, s_is_support_dynamic_type);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps a Rust class to its Hack shape equivalent
 */
Array populateClass(const hackc::ExtDeclClass& kls) {
  Array info = Array::CreateDict();
  info.set(s_name, rustToString(kls.name));

  // Derived from ClassishKind
  // class / interface / trait / enum / enum_class
  info.set(s_kind, rustToString(kls.kind));

  maybeSet(info, kls.module, s_module, rustToString);
  maybeSet(info, kls.docs_url, s_docs_url, rustToString);

  maybeSetBool(info, kls.final_, s_is_final);
  maybeSetBool(info, kls.abstract_, s_is_abstract);
  maybeSetBool(info, kls.internal, s_is_internal);
  maybeSetBool(info, kls.is_strict, s_is_strict);
  maybeSetBool(info, kls.support_dynamic_type, s_is_support_dynamic_type);

  maybeSet(info, kls.extends, s_extends, populateStringArray);
  maybeSet(info, kls.uses, s_uses, populateStringArray);
  maybeSet(info, kls.implements, s_implements, populateStringArray);
  maybeSet(info, kls.require_extends, s_require_extends, populateStringArray);
  maybeSet(
      info, kls.require_implements, s_require_implements, populateStringArray);
  maybeSet(info, kls.require_class, s_require_class, populateStringArray);

  // XHP related
  maybeSetBool(info, kls.is_xhp, s_is_xhp);
  maybeSetBool(info, kls.has_xhp_keyword, s_has_xhp);
  maybeSetBool(info, kls.xhp_marked_empty, s_is_xhp_marked_empty);
  maybeSet(info, kls.xhp_attr_uses, s_xhp_attr_uses, populateStringArray);

  // Complex Types
  maybeSet(info, kls.user_attributes, s_attributes, populateAttributes);
  maybeSet(info, kls.methods, s_methods, populateMethods);
  maybeSet(info, kls.static_methods, s_static_methods, populateMethods);
  maybeSetFirst(info, kls.constructor, s_constructor, populateMethods);
  maybeSet(info, kls.typeconsts, s_typeconsts, populateTypeConstants);
  maybeSet(info, kls.consts, s_consts, populateConstants);
  maybeSet(info, kls.tparams, s_tparams, populateTParams);
  maybeSetFirst(info, kls.enum_type, s_enum_type, populateEnumType);
  maybeSet(info, kls.props, s_props, populateProps);
  maybeSet(info, kls.sprops, s_sprops, populateProps);

  return info;
}
} // namespace

/*
 * Maps the Rust classes to their Hack shape equivalent
 */
Array populateClasses(const rust::Vec<hackc::ExtDeclClass>& classes) {
  if (classes.empty()) {
    return empty_vec_array();
  }

  VecInit arr{classes.size()};
  for (auto const& kls : classes) {
    arr.append(populateClass(kls));
  }
  return arr.toArray();
}

/*
 * Maps the rust file consts to their hack shape equivalent
 */
Array populateFileConsts(const rust::Vec<hackc::ExtDeclFileConst>& consts) {
  if (consts.empty()) {
    return empty_vec_array();
  }

  VecInit arr{consts.size()};
  for (auto const& c : consts) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(c.name));
    info.set(s_type, rustToString(c.type_));
    maybeSet(info, c.value, s_value, rustToString);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust functions to their Hack shape equivalent
 */
Array populateFileFuncs(const rust::Vec<hackc::ExtDeclFileFunc>& funs) {
  if (funs.empty()) {
    return empty_vec_array();
  }

  VecInit arr{funs.size()};
  for (auto const& fun : funs) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(fun.name));
    info.set(s_signature_type, rustToString(fun.type_));
    maybeSet(info, fun.module, s_module, rustToString);
    maybeSetBool(info, fun.internal, s_is_internal);
    maybeSetBool(info, fun.php_std_lib, s_is_php_std_lib);
    maybeSetBool(info, fun.support_dynamic_type, s_is_support_dynamic_type);
    maybeSetBool(info, fun.no_auto_dynamic, s_is_no_dynamic);
    maybeSetBool(info, fun.no_auto_likes, s_is_no_auto_likes);
    maybeSetFirst(info, fun.signature, s_signature, populateSignature);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust module definitions to their Hack shape equivalent
 */
Array populateModules(const rust::Vec<hackc::ExtDeclModule>& modules) {
  if (modules.empty()) {
    return empty_vec_array();
  }

  VecInit arr{modules.size()};
  for (auto const& module : modules) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(module.name));
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust type definitions to their Hack shape equivalent
 */
Array populateTypedefs(const rust::Vec<hackc::ExtDeclTypeDef>& typedefs) {
  if (typedefs.empty()) {
    return empty_vec_array();
  }

  VecInit arr{typedefs.size()};
  for (auto const& td : typedefs) {
    Array info = Array::CreateDict();
    info.set(s_name, rustToString(td.name));
    info.set(s_type, rustToString(td.type_));
    info.set(s_visibility, rustToString(td.visibility));
    maybeSet(info, td.module, s_module, rustToString);
    maybeSet(info, td.tparams, s_tparams, populateTParams);
    maybeSet(info, td.as_constraint, s_as_constraint, rustToString);
    maybeSet(info, td.super_constraint, s_super_constraint, rustToString);
    maybeSetBool(info, td.is_ctx, s_is_ctx);
    maybeSetBool(info, td.internal, s_is_internal);
    maybeSet(info, td.docs_url, s_docs_url, rustToString);
    maybeSet(info, td.attributes, s_attributes, populateAttributes);
    arr.append(info);
  }
  return arr.toArray();
}

/*
 * Maps the Rust file to its Hack shape equivalent
 */
Array populateFile(const hackc::ExtDeclFile& file) {
  Array info = Array::CreateDict();
  maybeSet(info, file.typedefs, s_typedefs, populateTypedefs);
  maybeSet(info, file.functions, s_functions, populateFileFuncs);
  maybeSet(info, file.constants, s_consts, populateFileConsts);
  maybeSet(info, file.file_attributes, s_attributes, populateAttributes);
  maybeSet(info, file.modules, s_modules, populateModules);
  maybeSet(info, file.classes, s_classes, populateClasses);
  maybeSetBool(
      info, file.disable_xhp_element_mangling, s_disable_xhp_element_mangling);
  maybeSetBool(
      info, file.has_first_pass_parse_errors, s_has_first_pass_parse_errors);
  maybeSetBool(info, file.is_strict, s_is_strict);
  return info;
}

Array populateTypeStructure(const hackc::ExtDeclTypeStructure& ts) {
  Array subtypes = Array::CreateVec();
  for (auto const& t : ts.subtypes) {
    Array info = Array::CreateDict();
    maybeSet(info, t.name, s_name, rustToString);
    maybeSetBool(info, t.optional, s_is_optional);
    info.set(s_type, populateTypeStructure(t.type_));
    subtypes.append(info);
  }

  Array info = Array::CreateDict();
  info.set(s_type, rustToString(ts.type_));
  info.set(s_kind, rustToString(ts.kind));
  maybeSetBool(info, ts.nullable, s_is_nullable);
  if (!subtypes.empty()) {
    info.set(s_subtypes, subtypes);
  }

  return info;
}

///////////////////////////////////////////////////////////////////////////////

// Mapping of SHA1 of file contents to the Decls from hackc::parse_decls().
// SHA1 is used as a cache key, invalidating if the file on disk changes.
// LRU cache means we eventually evict entries for files that have changed.
using FileDeclsCacheEntry =
    std::pair<SHA1, std::shared_ptr<rust::Box<hackc::DeclsHolder>>>;
using FileDeclsCache = ConcurrentLRUCache<std::string, FileDeclsCacheEntry>;
static std::unique_ptr<FileDeclsCache> declCachePtr;

/*
  The native implementation of the FileDecls class. The class is accessible
  through Hack and exposes the methods of ext_decl extension to retrieve info.

  Every instance holds the results of the parsing in the rust DeclsHolder.
  The methods use the declsHolder to access the parsed information.
*/
struct FileDecls : SystemLib::ClassLoader<"HH\\FileDecls"> {
  FileDecls() {}
  FileDecls& operator=(const FileDecls& /*that_*/) = delete;
  ~FileDecls() {}

  void validateState() {
    if (!this->declsHolder || this->error != empty_string()) {
      SystemLib::throwInvalidOperationExceptionObject(
          "FileDecls is in erroneous state");
    }
  }

  String error = empty_string();
  std::shared_ptr<rust::Box<hackc::DeclsHolder>> declsHolder;

  void sweep() {
    declsHolder.reset();
  }
};

namespace {
// Helper for async parsePath
class MemcacheHitEvent : public AsioExternalThreadEvent {
 public:
  explicit MemcacheHitEvent(
      folly::SemiFuture<rust::Box<hackc::DeclsHolder>>&& getFuture) {
    std::move(getFuture)
        .via(getExecutor())
        .thenValue([this](rust::Box<hackc::DeclsHolder>&& declsHolder) {
          holderPtr_ = std::make_shared<rust::Box<hackc::DeclsHolder>>(
              std::move(declsHolder));
        })
        .ensure([this]() { this->markAsFinished(); });
  }

 protected:
  void unserialize(TypedValue& output) override {
    if (holderPtr_ == nullptr) {
      SystemLib::throwInvalidOperationExceptionObject(
          "MemcacheHitEvent is in erroneous state");
    }
    Object obj{FileDecls::classof()};
    auto data = Native::data<FileDecls>(obj);
    data->declsHolder = holderPtr_;
    tvCopy(make_tv<KindOfObject>(obj.detach()), output);
  }

 private:
  std::shared_ptr<rust::Box<hackc::DeclsHolder>> holderPtr_;
};
} // namespace

///////////////////////////////////////////////////////////////////////////////
// API

/*
 * Parses the content in the given path and returns
 * a new instance of FileDecls. This method may use a cache instead of parsing.
 */
Object HHVM_STATIC_METHOD(FileDecls, parsePath, const String& path) {
  assertEnv();
  std::filesystem::path filePath{path.data()};
  auto root = getRepoRootForFile(path);
  auto sha1 = computeSHA1(filePath, root);
  {
    FileDeclsCache::ConstAccessor acc;
    if (declCachePtr && declCachePtr->find(acc, filePath.native())) {
      FileDeclsCacheEntry entry = *acc;
      auto& [expectedSha1, decls] = entry;
      if (expectedSha1 == sha1) {
        Object obj{FileDecls::classof()};
        auto data = Native::data<FileDecls>(obj);
        data->declsHolder = std::move(decls);
        return obj;
      }
    }
  }

  Facts::PathAndOptionalHash pathAndHash{
      filePath, Optional<std::string>(sha1.toString())};
  Object obj{FileDecls::classof()};
  auto data = Native::data<FileDecls>(obj);
  try {
    auto keepAliveToken = getExecutor();
    data->declsHolder =
        std::make_shared<rust::Box<hackc::DeclsHolder>>(Decl::decl_from_path(
            pathAndHash,
            keepAliveToken,
            s_extractorConfig.enableExternExtractor));
  } catch (const std::exception& ex) {
    data->error = ex.what();
  }

  if (declCachePtr) {
    auto ptr = data->declsHolder;
    declCachePtr->insert(filePath.native(), {sha1, ptr});
  }
  return obj;
}

String HHVM_STATIC_METHOD(FileDecls, getRepoOptionsHash) {
  auto opts = *g_context->getRepoOptionsForRequest();
  return String(opts.flags().cacheKeySha1().toString());
}

/*
 * Async version of parsePath.
 */
Object HHVM_STATIC_METHOD(FileDecls, genParsePath, const String& path) {
  assertEnv();
  std::filesystem::path filePath{path.data()};
  auto root = getRepoRootForFile(path);
  auto sha1 = computeSHA1(filePath, root);
  Facts::PathAndOptionalHash pathAndHash{
      filePath, Optional<std::string>(sha1.toString())};
  MemcacheHitEvent* event;
  try {
    auto keepAliveToken = getExecutor();
    auto declSemiFuture = Decl::decl_from_path_async(
        pathAndHash, keepAliveToken, s_extractorConfig.enableExternExtractor);
    event = new MemcacheHitEvent(std::move(declSemiFuture));
    return Object{event->getWaitHandle()};
  } catch (...) {
    if (event) {
      event->abandon();
    }
    SystemLib::throwRuntimeExceptionObject(
        std::string("Failed to fetch decls from Memcache or disk"));
  }
}

/*
 * Parses the provided text and returns a new instance of FileDecls.
 */
Object HHVM_STATIC_METHOD(FileDecls, parseText, const String& text) {
  assertEnv();
  Object obj{FileDecls::classof()};
  auto data = Native::data<FileDecls>(obj);
  auto config = initDeclConfig();
  try {
    data->declsHolder =
        std::make_shared<rust::Box<hackc::DeclsHolder>>(hackc::parse_decls(
            config, "", {(const uint8_t*)text.data(), (size_t)text.size()}));
  } catch (const std::exception& ex) {
    data->error = ex.what();
  }
  return obj;
}

/*
 * Parses a type expression and returns a type structure representation of it
 */
Variant HHVM_STATIC_METHOD(
    FileDecls,
    parseTypeExpression,
    const String& type_expression) {
  assertEnv();
  Object obj{FileDecls::classof()};
  auto data = Native::data<FileDecls>(obj);
  auto config = initDeclConfig();

  // The builtin sentinel
  String text = "type _TS_SENTINEL = " + type_expression + ";";

  try {
    data->declsHolder =
        std::make_shared<rust::Box<hackc::DeclsHolder>>(hackc::parse_decls(
            config, "", {(const uint8_t*)text.data(), (size_t)text.size()}));

    auto const decls =
        hackc::get_type_structure(**data->declsHolder, "_TS_SENTINEL");
    if (decls.empty()) {
      return init_null_variant;
    }
    return Variant(populateTypeStructure(decls.front()));
  } catch (const std::exception&) {
    return init_null_variant;
  }
}

/*
 * Returns a non empty string if the instance is in an erroneous state.
 * This can happen if parsing failed.
 */
static Variant HHVM_METHOD(FileDecls, getError) {
  auto data = Native::data<FileDecls>(this_);
  return data->error.empty() ? init_null_variant : data->error;
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
  return hackc::type_exists(**data->declsHolder, toRustStr(name));
}

static Variant HHVM_METHOD(FileDecls, getFile) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decl = hackc::get_file(**data->declsHolder);
  return Variant(populateFile(decl));
}

static Array HHVM_METHOD(FileDecls, getClasses) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateClasses(hackc::get_classes(**data->declsHolder));
}

static Variant HHVM_METHOD(FileDecls, getClass, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class(**data->declsHolder, toRustStr(name));
  return decls.empty() ? init_null_variant : populateClass(decls[0]);
}

static String HHVM_METHOD(FileDecls, getPublicApiForClass, const String& name) {
  if (name.empty()) {
    return String{};
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const api =
      hackc::get_public_api_for_class(**data->declsHolder, toRustStr(name));
  return api.empty() ? String{} : rustToString(api[0]);
}

static Array HHVM_METHOD(FileDecls, getFileAttributes) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateAttributes(hackc::get_file_attributes(**data->declsHolder));
}

static Variant HHVM_METHOD(FileDecls, getFileAttribute, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls =
      hackc::get_file_attribute(**data->declsHolder, toRustStr(name));
  return decls.empty() ? init_null_variant : populateAttributes(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getFileConsts) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateFileConsts(hackc::get_file_consts(**data->declsHolder));
}

static Variant HHVM_METHOD(FileDecls, getFileConst, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls =
      hackc::get_file_const(**data->declsHolder, toRustStr(name));
  return decls.empty() ? init_null_variant : populateFileConsts(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getFileFuncs) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateFileFuncs(hackc::get_file_funcs(**data->declsHolder));
}

static Variant HHVM_METHOD(FileDecls, getFileFunc, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_file_func(**data->declsHolder, toRustStr(name));
  return decls.empty() ? init_null_variant : populateFileFuncs(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getFileModules) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateModules(hackc::get_file_modules(**data->declsHolder));
}

static Variant HHVM_METHOD(FileDecls, getFileModule, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls =
      hackc::get_file_module(**data->declsHolder, toRustStr(name));
  return decls.empty() ? init_null_variant : populateModules(decls)[0];
}

static Variant HHVM_METHOD(FileDecls, getFileModuleMembership) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto result = hackc::get_file_module_membership(**data->declsHolder);
  return result.empty() ? init_null_variant : rustToString(result);
}

static Array HHVM_METHOD(FileDecls, getFileTypedefs) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateTypedefs(hackc::get_file_typedefs(**data->declsHolder));
}

static Variant HHVM_METHOD(FileDecls, getFileTypedef, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls =
      hackc::get_file_typedef(**data->declsHolder, toRustStr(name));
  return decls.empty() ? init_null_variant : populateTypedefs(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getShapeKeys, const String& name) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const keys = hackc::get_shape_keys(**data->declsHolder, toRustStr(name));
  return populateStringArray(keys);
}

static Array HHVM_METHOD(FileDecls, getMethods, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateMethods(
      hackc::get_class_methods(**data->declsHolder, toRustStr(kls)));
}

static Variant
HHVM_METHOD(FileDecls, getMethod, const String& kls, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_method(
      **data->declsHolder, toRustStr(kls), toRustStr(name));
  return decls.empty() ? init_null_variant : populateMethods(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getStaticMethods, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateMethods(
      hackc::get_class_smethods(**data->declsHolder, toRustStr(kls)));
}

static Variant
HHVM_METHOD(FileDecls, getStaticMethod, const String& kls, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_smethod(
      **data->declsHolder, toRustStr(kls), toRustStr(name));
  return decls.empty() ? init_null_variant : populateMethods(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getConsts, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateConstants(
      hackc::get_class_consts(**data->declsHolder, toRustStr(kls)));
}

static Variant
HHVM_METHOD(FileDecls, getConst, const String& kls, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_const(
      **data->declsHolder, toRustStr(kls), toRustStr(name));
  return decls.empty() ? init_null_variant : populateConstants(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getTypeconsts, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateTypeConstants(
      hackc::get_class_typeconsts(**data->declsHolder, toRustStr(kls)));
}

static Variant
HHVM_METHOD(FileDecls, getTypeconst, const String& kls, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_typeconst(
      **data->declsHolder, toRustStr(kls), toRustStr(name));
  return decls.empty() ? init_null_variant : populateTypeConstants(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getProps, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateProps(
      hackc::get_class_props(**data->declsHolder, toRustStr(kls)));
}

static Variant
HHVM_METHOD(FileDecls, getProp, const String& kls, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_prop(
      **data->declsHolder, toRustStr(kls), toRustStr(name));
  return decls.empty() ? init_null_variant : populateProps(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getStaticProps, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateProps(
      hackc::get_class_sprops(**data->declsHolder, toRustStr(kls)));
}

static Variant
HHVM_METHOD(FileDecls, getStaticProp, const String& kls, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();

  // Static properties are prefixed with dollars, unlike instance properties.
  // This can cause some confusion in the querying API.
  String norm_name = name;
  if (name.charAt(0) != '$') {
    norm_name = String::FromChar('$') + name;
  }

  auto const decls = hackc::get_class_sprop(
      **data->declsHolder, toRustStr(kls), toRustStr(norm_name));
  return decls.empty() ? init_null_variant : populateProps(decls)[0];
}

static Array HHVM_METHOD(FileDecls, getAttributes, const String& kls) {
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  return populateAttributes(
      hackc::get_class_attributes(**data->declsHolder, toRustStr(kls)));
}

static Variant
HHVM_METHOD(FileDecls, getAttribute, const String& kls, const String& name) {
  if (name.empty()) {
    return init_null_variant;
  }
  auto data = Native::data<FileDecls>(this_);
  data->validateState();
  auto const decls = hackc::get_class_attribute(
      **data->declsHolder, toRustStr(kls), toRustStr(name));
  return decls.empty() ? init_null_variant : populateAttributes(decls)[0];
}

///////////////////////////////////////////////////////////////////////////////
// Extension

struct DeclExtension final : Extension {
  DeclExtension() : Extension("decl", "0.1", NO_ONCALL_YET) {}

  const DependencySet getDeps() const override {
    // ensure ext_facts initializes and loads first.
    return DependencySet({"facts"});
  }

  void moduleLoad(const IniSetting::Map& ini, Hdf hdf) override {
    s_extractorConfig.enableExternExtractor =
        Config::GetBool(ini, hdf, "Ext.Decl.EnableExternExtractor", false);
    s_extractorConfig.cacheSize =
        Config::GetInt32(ini, hdf, "Ext.Decl.CacheSize", 500000);
    if (s_extractorConfig.cacheSize > 0) {
      declCachePtr.reset(new FileDeclsCache(s_extractorConfig.cacheSize));
    }
  }

  void moduleShutdown() override {
    // clear the executor to avoid the thread hanging around
    // after process exit. See D44596199 for similar issues.
    s_executor.reset();
  }

  void moduleRegisterNative() override {
    HHVM_STATIC_MALIAS(
        HH\\FileDecls, getRepoOptionsHash, FileDecls, getRepoOptionsHash);
    HHVM_STATIC_MALIAS(HH\\FileDecls, parseText, FileDecls, parseText);
    HHVM_STATIC_MALIAS(HH\\FileDecls, parsePath, FileDecls, parsePath);
    HHVM_STATIC_MALIAS(HH\\FileDecls, genParsePath, FileDecls, genParsePath);
    HHVM_STATIC_MALIAS(
        HH\\FileDecls, parseTypeExpression, FileDecls, parseTypeExpression);
    HHVM_MALIAS(HH\\FileDecls, getError, FileDecls, getError);
    HHVM_MALIAS(HH\\FileDecls, hasType, FileDecls, hasType);
    HHVM_MALIAS(HH\\FileDecls, getClass, FileDecls, getClass);
    HHVM_MALIAS(HH\\FileDecls, getMethods, FileDecls, getMethods);
    HHVM_MALIAS(HH\\FileDecls, getMethod, FileDecls, getMethod);
    HHVM_MALIAS(HH\\FileDecls, getStaticMethods, FileDecls, getStaticMethods);
    HHVM_MALIAS(HH\\FileDecls, getStaticMethod, FileDecls, getStaticMethod);
    HHVM_MALIAS(HH\\FileDecls, getConsts, FileDecls, getConsts);
    HHVM_MALIAS(HH\\FileDecls, getConst, FileDecls, getConst);
    HHVM_MALIAS(HH\\FileDecls, getTypeconsts, FileDecls, getTypeconsts);
    HHVM_MALIAS(HH\\FileDecls, getTypeconst, FileDecls, getTypeconst);
    HHVM_MALIAS(HH\\FileDecls, getProps, FileDecls, getProps);
    HHVM_MALIAS(HH\\FileDecls, getProp, FileDecls, getProp);
    HHVM_MALIAS(HH\\FileDecls, getStaticProps, FileDecls, getStaticProps);
    HHVM_MALIAS(HH\\FileDecls, getStaticProp, FileDecls, getStaticProp);
    HHVM_MALIAS(HH\\FileDecls, getAttributes, FileDecls, getAttributes);
    HHVM_MALIAS(HH\\FileDecls, getAttribute, FileDecls, getAttribute);
    HHVM_MALIAS(HH\\FileDecls, getClasses, FileDecls, getClasses);
    HHVM_MALIAS(HH\\FileDecls, getClass, FileDecls, getClass);
    HHVM_MALIAS(HH\\FileDecls, getFileAttributes, FileDecls, getFileAttributes);
    HHVM_MALIAS(HH\\FileDecls, getFileAttribute, FileDecls, getFileAttribute);
    HHVM_MALIAS(HH\\FileDecls, getFileConsts, FileDecls, getFileConsts);
    HHVM_MALIAS(HH\\FileDecls, getFileConst, FileDecls, getFileConst);
    HHVM_MALIAS(HH\\FileDecls, getFileFuncs, FileDecls, getFileFuncs);
    HHVM_MALIAS(HH\\FileDecls, getFileFunc, FileDecls, getFileFunc);
    HHVM_MALIAS(HH\\FileDecls, getFileModules, FileDecls, getFileModules);
    HHVM_MALIAS(HH\\FileDecls, getFileModule, FileDecls, getFileModule);
    HHVM_MALIAS(
        HH\\FileDecls,
        getFileModuleMembership,
        FileDecls,
        getFileModuleMembership);
    HHVM_MALIAS(HH\\FileDecls, getFileTypedefs, FileDecls, getFileTypedefs);
    HHVM_MALIAS(HH\\FileDecls, getFileTypedef, FileDecls, getFileTypedef);
    HHVM_MALIAS(HH\\FileDecls, getFile, FileDecls, getFile);
    HHVM_MALIAS(HH\\FileDecls, getShapeKeys, FileDecls, getShapeKeys);
    HHVM_MALIAS(
        HH\\FileDecls, getPublicApiForClass, FileDecls, getPublicApiForClass);

    Native::registerNativeDataInfo<FileDecls>();
  }
} s_decl_extension;

} // namespace HPHP
