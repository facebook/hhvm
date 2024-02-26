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

/**
 * Loads a named systemlib section from the main binary (or DSO)
 * using the label "ext.{hash(name)}"
 *
 * If {name} is not passed, then {m_name} is assumed.
 */
void Extension::loadSystemlib(const std::string& name) {
  auto const moduleName = std::string("/:ext_"+name);
  auto const unit = get_systemlib(moduleName, this);
  always_assert_flog(unit, "No unit created for systemlib `{}'", moduleName);
  if (auto const info = unit->getFatalInfo()) {
    std::fprintf(stderr, "Systemlib `%s' contains a fataling unit: %s, %d\n",
                 moduleName.c_str(),
                 info->m_fatalMsg.c_str(),
                 info->m_fatalLoc.line1);
    _Exit(HPHP_EXIT_FAILURE);
  }

  unit->merge();
  SystemLib::addPersistentUnit(unit);
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
  return {toLower(std::string(m_name) + ".php")};
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

void Extension::loadDeclsFrom(const std::string& name) {
  auto serialized_decls = get_embedded_section("/:ext_" + name + ".decls");
  FTRACE_MOD(Trace::tmp0, 1, "Loading decls from {}\n", name.c_str());
  always_assert(serialized_decls.size() > 0);
  Native::registerBuiltinSymbols(serialized_decls);
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
