/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/analysis/emitter.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/ext/ext.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/ext_hhvm/ext_hhvm.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/logger.h"

#include <folly/Singleton.h>

#include <libgen.h> // For dirname(3).
#include <string>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

#define STRINGIZE_CLASS_NAME(cls) #cls
#define pinitSentinel __pinitSentinel
#define resource __resource

#define SYSTEM_CLASS_STRING(cls)                        \
  const StaticString s_##cls(LITSTR_INIT(STRINGIZE_CLASS_NAME(cls)));
SYSTEMLIB_CLASSES(SYSTEM_CLASS_STRING)

#undef resource
#undef pinitSentinel
#undef STRINGIZE_CLASS_NAME

void ProcessInit() {
  // Create the global mcg object
  jit::mcg = new jit::MCGenerator();
  jit::mcg->initUniqueStubs();

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

  rds::requestInit();
  string hhas;
  string slib = get_systemlib(&hhas);

  if (slib.empty()) {
    // Die a horrible death.
    Logger::Error("Unable to find/load systemlib.php");
    _exit(1);
  }

  LitstrTable::init();
  LitstrTable::get().setWriting();
  Repo::get().loadGlobalData();

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
    SystemLib::s_hhas_unit = compile_string(hhas.c_str(), hhas.size(),
                                            "systemlib.hhas");
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

  SystemLib::s_nativeFuncUnit = build_native_func_unit(hhbc_ext_funcs,
                                                       hhbc_ext_funcs_count);
  SystemLib::s_nativeFuncUnit->merge();
  SystemLib::s_nullFunc =
    Unit::lookupFunc(makeStaticString("86null"));

  // We call a special bytecode emitter function to build the native
  // unit which will contain all of our cppext functions and classes.
  // Each function and method will have a bytecode body that will thunk
  // to the native implementation.
  Unit* nativeClassUnit = build_native_class_unit(hhbc_ext_classes,
                                                  hhbc_ext_class_count);
  SystemLib::s_nativeClassUnit = nativeClassUnit;

  LitstrTable::get().setReading();

  // Load the nativelib unit to build the Class objects
  SystemLib::s_nativeClassUnit->merge();

#define INIT_SYSTEMLIB_CLASS_FIELD(cls)                                 \
  {                                                                     \
    Class *cls = NamedEntity::get(s_##cls.get())->clsList();       \
    assert(!hhbc_ext_class_count || cls);                               \
    SystemLib::s_##cls##Class = cls;                                    \
  }

  // Stash a pointer to the VM Classes for stdclass, Exception,
  // pinitSentinel and resource
  SYSTEMLIB_CLASSES(INIT_SYSTEMLIB_CLASS_FIELD)

#undef INIT_SYSTEMLIB_CLASS_FIELD

  // Retrieve all of the class pointers
  for (long long i = 0; i < hhbc_ext_class_count; ++i) {
    const HhbcExtClassInfo* info = hhbc_ext_classes + i;
    const StringData* name = makeStaticString(info->m_name);
    const NamedEntity* ne = NamedEntity::get(name);
    Class* cls = Unit::lookupClass(ne);
    assert(cls);
    *(info->m_clsPtr) = cls;
  }

  ClassInfo::InitializeSystemConstants();
  Stack::ValidateStackSize();
  SystemLib::s_inited = true;

  RuntimeOption::AlwaysUseRelativePath = rp;
  RuntimeOption::SafeFileAccess = sf;
  RuntimeOption::EvalDumpBytecode = db;
  RuntimeOption::EvalAllowHhas = ah;
  Option::WholeProgram = wp;

  void tweak_variant_dtors();
  tweak_variant_dtors();

  folly::SingletonVault::singleton()->registrationComplete();
}

///////////////////////////////////////////////////////////////////////////////
}
