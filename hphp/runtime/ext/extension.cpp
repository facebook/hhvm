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
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/extension-registry.h"

#include <cstdio>

#include "hphp/util/exception.h"
#include "hphp/util/assertions.h"
#include "hphp/runtime/ext/apache/ext_apache.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/vm/builtin-symbol-map.h"
#include "hphp/runtime/vm/native-func-table.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/system/systemlib.h"

#include <map>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Extension::Extension(const char* name, const char* version, const char* oncall)
    : m_name(name)
    , m_version(version)
    , m_oncall(oncall) {
  ExtensionRegistry::registerExtension(this);
}

const static std::string s_systemlibPhpName("systemlib.php");

bool Extension::IsSystemlibPath(const std::string& name) {
  return !name.compare(0, 2, "/:");
}

void Extension::CompileSystemlib(const std::string &slib,
                                 const std::string &name,
                                 const Extension* extension) {
  // TODO (t3443556) Bytecode repo compilation expects that any errors
  // encountered during systemlib compilation have valid filename pointers
  // which won't be the case for now unless these pointers are long-lived.
  auto const moduleName = makeStaticString("/:" + name);
  auto const unit = compile_systemlib_string(slib.c_str(), slib.size(),
                                             moduleName->data(), extension);
  always_assert_flog(unit, "No unit created for systemlib `{}'", moduleName);

  if (auto const info = unit->getFatalInfo()) {
    std::fprintf(stderr, "Systemlib `%s' contains a fataling unit: %s, %d\n",
                 name.c_str(),
                 info->m_fatalMsg.c_str(),
                 info->m_fatalLoc.line1);
    _Exit(HPHP_EXIT_FAILURE);
  }

  unit->merge();
  SystemLib::addPersistentUnit(unit);
}

namespace {

std::string get_section(std::string_view name) {
  assertx(!name.empty());
  std::string section("ext.");
  if (name.length() > 12) {
    section += HHVM_FN(md5)(std::string(name), false).substr(0, 12).data();
  } else {
    section += name;
  }
  return get_systemlib(section);
}

}

/**
 * Loads a named systemlib section from the main binary (or DSO)
 * using the label "ext.{hash(name)}"
 *
 * If {name} is not passed, then {m_name} is assumed.
 */
void Extension::loadSystemlib(const std::string& name) {
  auto const slib = get_section(name);
  if (!slib.empty()) {
    std::string phpname = s_systemlibPhpName + name;
    CompileSystemlib(slib, phpname, this);
  }
}

void Extension::moduleLoad(const IniSetting::Map& /*ini*/, Hdf /*hdf*/)
{}

void Extension::moduleInit()
{}

void Extension::moduleRegisterNative()
{}

void Extension::moduleInfo(Array &info) {
  info.set(String(m_name), true);
}

void Extension::cliClientInit()
{}

void Extension::moduleShutdown()
{}

void Extension::threadInit()
{}

void Extension::threadShutdown()
{}

void Extension::requestInit()
{}

void Extension::requestShutdown()
{}

// override this to control extension_loaded() return value
bool Extension::moduleEnabled() const {
  return true;
}

const Extension::DependencySet Extension::getDeps() const {
  // No dependencies by default
  return DependencySet();
}

void Extension::registerExtensionFunction(const String& name) {
  assertx(name.get()->isStatic());
  m_functions.push_back(name.get());
}

const std::vector<StringData*>& Extension::getExtensionFunctions() const {
  return m_functions;
}

std::vector<std::string> Extension::hackFiles() const {
  return {toLower(m_name)};
}

void Extension::loadEmitters() {
  for (auto const& name : hackFiles()) {
    loadSystemlib(name);
  }
}

void Extension::loadDecls() {
  // Look for "ext.{namehash}" in the binary and grab its decls
  for (auto const& name : hackFiles()) {
    loadDeclsFrom(name);
  }
}

void Extension::loadDeclsFrom(std::string_view name) {
  auto const slib = get_section(name);
  // We *really* ought to assert that `slib` is non-empty here, but there are
  // some extensions that don't have any source code, such as the ones created by
  // `IMPLEMENT_DEFAULT_EXTENSION_VERSION`
  Native::registerBuiltinSymbols(std::string(name), slib);
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
