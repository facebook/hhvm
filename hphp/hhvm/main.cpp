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

#include <vector>

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/emulate-zend.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/compiler/compiler.h"
#include "hphp/hhbbc/hhbbc.h"

#include "hphp/util/embedded-data.h"
#include "hphp/util/embedded-vfs.h"
#include "hphp/util/text-util.h"

#include <folly/Singleton.h>

/*
 * These are here to work around a gcc-5 lto bug. Without them,
 * certain symbols don't get defined, even though they're referenced,
 * but the build succeeds, and the references get set to nullptr (so
 * calls to vector<string>::~vector() end up as a call to 0.
 *
 * See t15096405
 */
std::vector<std::string> dummy_vec { "hello", "foo" };
std::set<std::string> dummy_set { "hello" };

int main(int argc, char** argv) {
  // Also for t15096405
  std::string (*ptr1)(std::string&&, const char*) = std::operator+;
  std::string (*ptr2)(const char*, std::string&&) = std::operator+;
  if (!argc) {
    return intptr_t(ptr1) + intptr_t(ptr2);
  }
  int len = strlen(argv[0]);
  if (len >= 4 && !strcmp(argv[0] + len - 4, "hphp")) {
    return HPHP::compiler_main(argc, argv);
  }
  if (argc > 1 && !strcmp(argv[1], "--hphp")) {
    argv[1] = argv[0];
    return HPHP::compiler_main(argc - 1, argv + 1);
  }

  if (len >= 5 && !strcmp(argv[0] + len - 5, "hhbbc")) {
    return HPHP::HHBBC::main(argc, argv);
  }
  if (argc > 1 && !strcmp(argv[1], "--hhbbc")) {
    argv[1] = "hhbbc";
    return HPHP::HHBBC::main(argc - 1, argv + 1);
  }

  HPHP::register_process_init();
  if (len >= 3 && !strcmp(argv[0] + len - 3, "php")) {
    return HPHP::emulate_zend(argc, argv);
  }
  if (argc > 1 && !strcmp(argv[1], "--php")) {
    argv[1] = argv[0];
    return HPHP::emulate_zend(argc - 1, argv + 1);
  }

  if (argc > 1 && !strcmp(argv[1], "--info")) {
    // prepare a command line of "<command> --php -r 'phpinfo();'"
    std::vector<char*> args;
    args.push_back(argv[0]);
    args.push_back("-r");
    args.push_back("phpinfo();");
    return HPHP::emulate_zend(args.size(), args.data());
  }

  HPHP::embedded_data data;
  if (!HPHP::get_embedded_data("repo", &data)) {
    return HPHP::execute_program(argc, argv);
  }
  std::string repo;
  HPHP::string_printf(repo, "%s:%u:%u",
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
