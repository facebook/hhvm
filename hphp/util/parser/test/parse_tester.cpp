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

#include "hphp/util/parser/test/parser.h"

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
  HPHP::Test::g_verifyMode = true;

  char* filename = nullptr;
  for (int i = 1; i < argc - 1; i++) {
    if (!strcmp(argv[i], "--file")) {
      filename = argv[i+1];
    }
  }
  if (filename == nullptr) {
    std::cerr << argv[0] << ": no --file 'filename' passed" << '\n';
    return 1;
  }

  std::ifstream in(filename);
  if (!in.is_open()) {
    std::cerr << argv[0] << ": couldn't open file " << filename << " "
              << strerror(errno) << '\n';
    return 1;
  }

  using HPHP::Scanner;
  using HPHP::Test::Parser;
  Scanner scan(in, Scanner::AllowShortTags |
                   Scanner::AllowHipHopSyntax);
  Parser parser(scan, argv[1]);
  parser.parse();
  std::cout << "FORCE PASS";
}

catch (const std::exception& e) {
  std::cerr << argv[0] << ": " << e.what() << '\n';
  return 1;
}
