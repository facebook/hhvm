/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/base/program_functions.h"
#include "hhvm/process_init.h"
#include "compiler/compiler.h"

#include "util/embedded_data.h"
#include "util/embedded_vfs.h"
#include "util/util.h"

int main(int argc, char** argv) {
  if (!argc) return 0;
  if (argc > 1 && !strcmp(argv[1], "--hphp")) {
    argv[1] = argv[0];
    return HPHP::compiler_main(argc - 1, argv + 1);
  }
  HPHP::register_process_init();
  HPHP::Util::embedded_data data;
  if (!HPHP::Util::get_embedded_data("repo", &data)) {
    return HPHP::execute_program(argc, argv);
  }
  std::string repo;
  HPHP::Util::string_printf(repo, "%s:%u:%u",
                            data.m_filename.c_str(),
                            (unsigned)data.m_start, (unsigned)data.m_len);
  HPHP::sqlite3_embedded_initialize(nullptr, true);

  std::vector<char*> args;
  args.push_back(argv[0]);
  args.push_back("-vRepo.Authoritative=true");
  args.push_back("-vRepo.Local.Mode=r-");
  repo = "-vRepo.Local.Path=" + repo;
  args.push_back(const_cast<char*>(repo.c_str()));
  if (argc > 1) {
    args.insert(args.end(), argv + 1, argv + argc);
  }
  return HPHP::execute_program(args.size(), &args[0]);
}
