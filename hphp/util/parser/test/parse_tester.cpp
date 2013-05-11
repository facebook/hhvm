/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <iostream>
#include <cstdlib>
#include <string.h>

#include "util/parser/test/parser.h"

namespace HPHP { namespace Test {

  bool g_verifyMode = false;

}}


/*
 * This program parses a file with the hphp php parser, and dumps
 * every callback the parser makes to stdout.
 *
 * If a parse error occurs, it says why.
 */
int main(int argc, char** argv) try {
  if (argc >= 2 && !strcmp(argv[1], "--verify")) {
    HPHP::Test::g_verifyMode = true;
    --argc, ++argv;
  }

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " [--verify] filename\n";
    std::exit(1);
  }

  std::ifstream in(argv[1]);
  if (!in.is_open()) {
    std::cerr << argv[0] << ": couldn't open file: "
              << strerror(errno) << '\n';
  }

  std::cout << "1..1\n";

  try {
    using HPHP::Scanner;
    using HPHP::Test::Parser;
    Scanner scan(in, Scanner::AllowShortTags |
                     Scanner::EnableHipHopKeywords);
    Parser parser(scan, argv[1]);
    parser.parse();
  } catch (const std::exception& e) {
    if (HPHP::Test::g_verifyMode) {
      std::cout << "not ";
    } else {
      throw;
    }
  }
  std::cout << "ok 1\n";
}

catch (const std::runtime_error& e) {
  std::cerr << argv[0] << ": " << e.what() << '\n';
  return 1;
}
