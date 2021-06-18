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
#ifndef incl_HPHP_HHVM_PROCESS_INIT_H_
#define incl_HPHP_HHVM_PROCESS_INIT_H_

#include "hphp/compiler/analysis/emitter.h"

#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/program-functions.h"

#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/treadmill.h"

namespace HPHP {

extern void (*g_vmProcessInit)();
void hphp_process_init();
void ProcessInit();

/*
 * This must be called before execute_program_impl in an hhvm build.
 */
inline void register_process_init() {
  g_vmProcessInit = &ProcessInit;
  g_hphp_compiler_parse = &HPHP::Compiler::hphp_compiler_parse;
}

/*
 * Initialize the runtime in a way that's appropriate for unit tests
 * that make partial use of libhphp_runtime.a.  (There will not be a
 * real execution context or anything like that.)
 */
inline void init_for_unit_test() {
  HPHP::StaticString::CreateAll();
  register_process_init();
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  RuntimeOption::Load(ini, config);
  hphp_compiler_init();
  hphp_process_init();
  hphp_session_init(Treadmill::SessionKind::UnitTests);
}
}
#endif
