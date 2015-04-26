/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/runtime.h"
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
  return !name.compare(0, s_systemlibPhpName.length(), s_systemlibPhpName) ||
         !name.compare(0, s_systemlibHhasName.length(), s_systemlibHhasName);
}

void Extension::CompileSystemlib(const std::string &slib,
                                 const std::string &name) {
  // TODO (t3443556) Bytecode repo compilation expects that any errors
  // encountered during systemlib compilation have valid filename pointers
  // which won't be the case for now unless these pointers are long-lived.
  auto const moduleName = makeStaticString(name.c_str());
  auto const unit = compile_systemlib_string(slib.c_str(), slib.size(),
                                             moduleName->data());
  always_assert_flog(unit, "No unit created for systemlib `{}'", name);

  const StringData* msg;
  int line;
  if (unit->compileTimeFatal(msg, line) ||
      unit->parseFatal(msg, line)) {
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
  std::string n = name.empty() ?
    std::string(m_name.data(), m_name.size()) : name;
  std::string section("ext.");
  section += HHVM_FN(md5)(n, false).substr(0, 12).data();
  std::string hhas;
  std::string slib = get_systemlib(&hhas, section, m_dsoName);
  if (!slib.empty()) {
    std::string phpname = s_systemlibPhpName + n;
    CompileSystemlib(slib, phpname);
  }
  if (!hhas.empty()) {
    std::string hhasname = s_systemlibHhasName + n;
    CompileSystemlib(hhas, hhasname);
  }
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
