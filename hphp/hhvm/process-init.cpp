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

#include "hphp/compiler/option.h"

#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
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

#undef resource
#undef pinitSentinel
#undef STRINGIZE_CLASS_NAME

namespace {
const StaticString s_Throwable("\\__SystemLib\\Throwable");
const StaticString s_BaseException("\\__SystemLib\\BaseException");
const StaticString s_Error("\\__SystemLib\\Error");
const StaticString s_ArithmeticError("\\__SystemLib\\ArithmeticError");
const StaticString s_AssertionError("\\__SystemLib\\AssertionError");
const StaticString s_DivisionByZeroError("\\__SystemLib\\DivisionByZeroError");
const StaticString s_ParseError("\\__SystemLib\\ParseError");
const StaticString s_TypeError("\\__SystemLib\\TypeError");
}

void tweak_variant_dtors();
void ProcessInit() {
  // Save the current options, and set things up so that
  // systemlib.php can be read from and stored in the
  // normal repo.
  int db = RuntimeOption::EvalDumpBytecode;
  bool rp = RuntimeOption::AlwaysUseRelativePath;
  bool sf = RuntimeOption::SafeFileAccess;
  bool ah = RuntimeOption::EvalAllowHhas;
  bool wp = Option::WholeProgram;
  RuntimeOption::EvalDumpBytecode &= ~1;
  RuntimeOption::AlwaysUseRelativePath = false;
  RuntimeOption::SafeFileAccess = false;
  RuntimeOption::EvalAllowHhas = true;
  Option::WholeProgram = false;

  if (RuntimeOption::RepoAuthoritative) {
    LitstrTable::init();
    Repo::get().loadGlobalData();
  }

  jit::mcgen::processInit();
  jit::processInitProfData();

  rds::requestInit();
  std::string hhas;
  auto const slib = get_systemlib(&hhas);

  if (slib.empty()) {
    // Die a horrible death.
    Logger::Error("Unable to find/load systemlib.php, check /proc is mounted"
                  " or export HHVM_SYSTEMLIB to your ENV");
    _exit(1);
  }

  // Save this in case the debugger needs it. Once we know if this
  // process does not have debugger support, we'll clear it.
  SystemLib::s_source = slib;

  SystemLib::s_unit = compile_systemlib_string(slib.c_str(), slib.size(),
                                               "systemlib.php");

  const StringData* msg;
  int line;
  if (SystemLib::s_unit->compileTimeFatal(msg, line)) {
    Logger::Error("An error has been introduced into the systemlib, "
                  "but we cannot give you a file and line number right now.");
    Logger::Error("Check all of your changes to hphp/system/php");
    Logger::Error("HipHop Parse Error: %s", msg->data());
    _exit(1);
  }

  if (!hhas.empty()) {
    SystemLib::s_hhas_unit = compile_systemlib_string(
      hhas.c_str(), hhas.size(), "systemlib.hhas");
    if (SystemLib::s_hhas_unit->compileTimeFatal(msg, line)) {
      Logger::Error("An error has been introduced in the hhas portion of "
                    "systemlib.");
      Logger::Error("Check all of your changes to hhas files in "
                    "hphp/system/php");
      Logger::Error("HipHop Parse Error: %s", msg->data());
      _exit(1);
    }
  }

  // Load the systemlib unit to build the Class objects
  SystemLib::s_unit->merge();
  if (SystemLib::s_hhas_unit) {
    SystemLib::s_hhas_unit->merge();
  }

  SystemLib::s_nullFunc =
    Unit::lookupFunc(makeStaticString("__SystemLib\\__86null"));

#define INIT_SYSTEMLIB_CLASS_FIELD(cls)                                 \
  {                                                                     \
    Class *cls = NamedEntity::get(s_##cls.get())->clsList();            \
    assert(cls);                                                        \
    SystemLib::s_##cls##Class = cls;                                    \
  }

  INIT_SYSTEMLIB_CLASS_FIELD(Throwable)
  INIT_SYSTEMLIB_CLASS_FIELD(BaseException)
  INIT_SYSTEMLIB_CLASS_FIELD(Error)
  INIT_SYSTEMLIB_CLASS_FIELD(ArithmeticError)
  INIT_SYSTEMLIB_CLASS_FIELD(AssertionError)
  INIT_SYSTEMLIB_CLASS_FIELD(DivisionByZeroError)
  INIT_SYSTEMLIB_CLASS_FIELD(ParseError)
  INIT_SYSTEMLIB_CLASS_FIELD(TypeError)

  // Stash a pointer to the VM Classes for stdclass, Exception,
  // pinitSentinel and resource
  SYSTEMLIB_CLASSES(INIT_SYSTEMLIB_CLASS_FIELD)

#undef INIT_SYSTEMLIB_CLASS_FIELD

  Stack::ValidateStackSize();
  SystemLib::s_inited = true;

  RuntimeOption::AlwaysUseRelativePath = rp;
  RuntimeOption::SafeFileAccess = sf;
  RuntimeOption::EvalDumpBytecode = db;
  RuntimeOption::EvalAllowHhas = ah;
  Option::WholeProgram = wp;

  tweak_variant_dtors();
}

///////////////////////////////////////////////////////////////////////////////
}
