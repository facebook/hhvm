/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/builtin-symbol-map.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/build-info.h"
#include "hphp/util/logger.h"

#include <folly/portability/Libgen.h>

#include <string>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

#define STRINGIZE_CLASS_NAME(cls) #cls
#define pinitSentinel __pinitSentinel
#define resource __resource

#define SYSTEM_CLASS_STRING(cls)                        \
  const StaticString s_##cls(STRINGIZE_CLASS_NAME(cls));
SYSTEMLIB_CLASSES(SYSTEM_CLASS_STRING)
#undef SYSTEM_CLASS_STRING

#undef resource
#undef pinitSentinel
#undef STRINGIZE_CLASS_NAME

#define STRINGIZE_HH_CLASS_NAME(cls) "HH\\" #cls
#define SYSTEM_HH_CLASS_STRING(cls) \
  const StaticString s_HH_##cls(STRINGIZE_HH_CLASS_NAME(cls));
SYSTEMLIB_HH_CLASSES(SYSTEM_HH_CLASS_STRING)
#undef SYSTEM_HH_CLASS_STRING
#undef STRINGIZE_HH_CLASS_NAME

namespace {
const StaticString s_Throwable("Throwable");
const StaticString s_BaseException("\\__SystemLib\\BaseException");
const StaticString s_Error("Error");
const StaticString s_ArithmeticError("ArithmeticError");
const StaticString s_ArgumentCountError("ArgumentCountError");
const StaticString s_AssertionError("AssertionError");
const StaticString s_DivisionByZeroError("DivisionByZeroError");
const StaticString s_ParseError("ParseError");
const StaticString s_TypeError("TypeError");
const StaticString s_MethCallerHelper("\\__SystemLib\\MethCallerHelper");
const StaticString s_DynMethCallerHelper("\\__SystemLib\\DynMethCallerHelper");
}

void ProcessInitNoSystemLib() {
  assertx(!RuntimeOption::RepoAuthoritative);
  Stack::ValidateStackSize();
}

std::string get_and_check_systemlib() {
  auto const slib = get_systemlib();

  if (slib.empty()) {
    // Die a horrible death.
    Logger::Error("Unable to find/load systemlib.php, check /proc is mounted"
                  " or export HHVM_SYSTEMLIB to your ENV");
    _exit(HPHP_EXIT_FAILURE);
  }
  return slib;
}

void ProcessInit() {
  // Save the current options, and set things up so that
  // systemlib.php can be read from and stored in the
  // normal repo.
  int db = RuntimeOption::EvalDumpBytecode;
  bool rp = RuntimeOption::AlwaysUseRelativePath;
  bool sf = RuntimeOption::SafeFileAccess;
  bool ah = RuntimeOption::EvalAllowHhas;
  RuntimeOption::EvalDumpBytecode &= ~1;
  RuntimeOption::AlwaysUseRelativePath = false;
  RuntimeOption::SafeFileAccess = false;
  RuntimeOption::EvalAllowHhas = true;

  if (RuntimeOption::RepoAuthoritative) {
    RepoFile::loadGlobalTables();
    RepoFile::globalData().load();
  }

  rds::requestInit();
  auto const slib = get_and_check_systemlib();

  // Save this in case the debugger needs it. Once we know if this
  // process does not have debugger support, we'll clear it.
  SystemLib::s_source = slib;

  SystemLib::s_unit = compile_systemlib_string(slib.c_str(), slib.size(),
                                               "/:systemlib.php",
                                               Native::s_systemNativeFuncs);

  if (auto const info = SystemLib::s_unit->getFatalInfo()) {
    Logger::Error("An error has been introduced into the systemlib, "
                  "but we cannot give you a file and line number right now.");
    Logger::Error("Check all of your changes to hphp/system/php");
    Logger::Error("HipHop Parse Error: %s %d",
                  info->m_fatalMsg.c_str(), info->m_fatalLoc.line1);
    _exit(HPHP_EXIT_FAILURE);
  }

  // Load the systemlib unit to build the Class objects
  SystemLib::s_unit->merge();
  SystemLib::s_nullFunc =
    Func::lookup(makeStaticString("__SystemLib\\__86null"));

#define INIT_SYSTEMLIB_CLASS_FIELD(cls)                                 \
  {                                                                     \
    Class *cls = NamedType::get(s_##cls.get())->clsList();              \
    assert(cls);                                                        \
    SystemLib::s_##cls##Class = cls;                                    \
  }

  INIT_SYSTEMLIB_CLASS_FIELD(Throwable)
  INIT_SYSTEMLIB_CLASS_FIELD(BaseException)
  INIT_SYSTEMLIB_CLASS_FIELD(Error)
  INIT_SYSTEMLIB_CLASS_FIELD(ArithmeticError)
  INIT_SYSTEMLIB_CLASS_FIELD(ArgumentCountError)
  INIT_SYSTEMLIB_CLASS_FIELD(AssertionError)
  INIT_SYSTEMLIB_CLASS_FIELD(DivisionByZeroError)
  INIT_SYSTEMLIB_CLASS_FIELD(DivisionByZeroException)
  INIT_SYSTEMLIB_CLASS_FIELD(ParseError)
  INIT_SYSTEMLIB_CLASS_FIELD(TypeError)
  INIT_SYSTEMLIB_CLASS_FIELD(MethCallerHelper)
  INIT_SYSTEMLIB_CLASS_FIELD(DynMethCallerHelper)

  // Stash a pointer to the VM Classes for stdClass, Exception,
  // pinitSentinel and resource
  SYSTEMLIB_CLASSES(INIT_SYSTEMLIB_CLASS_FIELD)

#undef INIT_SYSTEMLIB_CLASS_FIELD

#define INIT_SYSTEMLIB_HH_CLASS_FIELD(cls)                              \
  {                                                                     \
    Class *cls = NamedType::get(s_HH_##cls.get())->clsList();           \
    assert(cls);                                                        \
    SystemLib::s_HH_##cls##Class = cls;                                 \
  }

  // Stash a pointer to the VM Classes for various HH namespace classes
  SYSTEMLIB_HH_CLASSES(INIT_SYSTEMLIB_HH_CLASS_FIELD)

#undef INIT_SYSTEMLIB_HH_CLASS_FIELD

  Stack::ValidateStackSize();

  RuntimeOption::AlwaysUseRelativePath = rp;
  RuntimeOption::SafeFileAccess = sf;
  RuntimeOption::EvalDumpBytecode = db;
  RuntimeOption::EvalAllowHhas = ah;
}

///////////////////////////////////////////////////////////////////////////////
}
