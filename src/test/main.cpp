/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test.h>
#include <util/logger.h>
#include <runtime/base/program_functions.h>
#include <dlfcn.h>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv) {
  string suite, which, set;
  void (*compiler_hook_initialize)();
  compiler_hook_initialize =
    (void (*)())dlsym(NULL, "compiler_hook_initialize");
  if (compiler_hook_initialize) compiler_hook_initialize();
  if (argc >= 2) suite = argv[1];
  if (argc >= 3) which = argv[2];
  if (argc >= 4) set   = argv[3];
  if (argc >= 5) {
    if (strcmp(argv[4], "quiet") == 0) {
      Test::s_quiet = true;
    } else {
      Logger::LogLevel = (Logger::LogLevelType)atoi(argv[4]);
    }
  }
  hphp_process_init();
  Test test;
  return test.RunTests(suite, which, set) ? 0 : -1;
}
