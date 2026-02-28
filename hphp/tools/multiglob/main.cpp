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

#include "hphp/util/multiglob.h"

#include <iostream>

using namespace HPHP;
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "multiglob filename patterns" << std::endl;
    return 0;
  }

  std::set<std::string> patterns;
  for (auto i = 2; i < argc; i++) {
    patterns.insert(argv[i]);
  }

  auto paths = MultiGlob::matches(patterns, argv[1]);
  for (auto& path : paths) {
    std::cout << path.string() << std::endl;
  }

  return 0;
}
