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

#include "hphp/test/ext/test.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/program-functions.h"

#include "hphp/hhvm/process-init.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  HPHP::StaticString::CreateAll();
  HPHP::register_process_init();

#ifdef HHVM_FBMAKE
  // In an fbmake build, override the locations of hphp and hhvm that
  // dirs.mk will think we're using.  (This is used in test.mk when
  // building/running tests.)
  setenv("TEST_OVERRIDE_HHVM", "_bin/hphp/hhvm/hhvm", true);
  setenv("TEST_OVERRIDE_HPHP", "_bin/hphp/hhvm/hphp", true);
#endif

  std::string suite, which, set;
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

  Test::logger.log_url = nullptr;
  if (argc >= 6) {
    Test::logger.log_url = argv[5];
  }

  rds::local::init();
  SCOPE_EXIT { rds::local::fini(); };

  // Initialize the runtime options with their default values
  {
    IniSetting::Map ini = IniSetting::Map::object;
    Hdf empty;
    RuntimeOption::Load(ini, empty);
    // This one's default value changed recently
    Cfg::Server::AlwaysPopulateRawPostData = true;
  }

  hphp_process_init();
  SCOPE_EXIT { hphp_process_exit(); };
  Test test;
  return test.RunTests(suite, which, set) ? 0 : -1;
}
