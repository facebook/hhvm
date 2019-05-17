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
#include "hphp/runtime/vm/native-func-table.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/system/systemlib.h"

#include <map>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Global systemlib extensions implemented entirely in PHP

IMPLEMENT_DEFAULT_EXTENSION_VERSION(redis, NO_EXTENSION_VERSION_YET);

///////////////////////////////////////////////////////////////////////////////

Extension::Extension(const char* name, const char* version /* = "" */)
    : m_name(name)
    , m_version(version ? version : "") {
  ExtensionRegistry::registerExtension(this);
}

const static std::string
  s_systemlibPhpName("systemlib.php"),
  s_systemlibHhasName("systemlib.hhas.");

bool Extension::IsSystemlibPath(const std::string& name) {
  return !name.compare(0, 2, "/:");
}

void Extension::CompileSystemlib(const std::string &slib,
                                 const std::string &name,
                                 const Native::FuncTable& nativeFuncs) {
  // TODO (t3443556) Bytecode repo compilation expects that any errors
  // encountered during systemlib compilation have valid filename pointers
  // which won't be the case for now unless these pointers are long-lived.
  auto const moduleName = makeStaticString("/:" + name);
  auto const unit = compile_systemlib_string(slib.c_str(), slib.size(),
                                             moduleName->data(),
                                             nativeFuncs);
  always_assert_flog(unit, "No unit created for systemlib `{}'", moduleName);

  const StringData* msg;
  int line;
  if (unit->compileTimeFatal(msg, line) || unit->parseFatal(msg, line)) {
    std::fprintf(stderr, "Systemlib `%s' contains a fataling unit: %s, %d\n",
                 name.c_str(),
                 msg->data(),
                 line);
    _Exit(0);
  }

  unit->merge();
  SystemLib::addPersistentUnit(unit);
}

/**
 * Loads a named systemlib section from the main binary (or DSO)
 * using the label "ext.{hash(name)}"
 *
 * If {name} is not passed, then {m_name} is assumed.
 */
void Extension::loadSystemlib(const std::string& name) {
  assertx(!name.empty());
#ifdef _MSC_VER
  std::string section("ext_");
#else
  std::string section("ext.");
#endif
  section += HHVM_FN(md5)(name, false).substr(0, 12).data();
  std::string hhas;
  std::string slib = get_systemlib(&hhas, section, m_dsoName);
  if (!slib.empty()) {
    std::string phpname = s_systemlibPhpName + name;
    CompileSystemlib(slib, phpname, m_nativeFuncs);
  }
  if (!hhas.empty()) {
    std::string hhasname = s_systemlibHhasName + name;
    CompileSystemlib(hhas, hhasname, m_nativeFuncs);
  }
}

void Extension::moduleLoad(const IniSetting::Map& /*ini*/, Hdf /*hdf*/)
{}

void Extension::moduleInfo(Array &info) {
  info.set(String(m_name), true);
}

void Extension::moduleInit()
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

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
