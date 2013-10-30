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
#include "hphp/hhbbc/debug.h"

#include <string>
#include <utility>
#include <cstdlib>
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/parallel.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

void debug_dump_program(const php::Program& program) {
  namespace fs = boost::filesystem;
  trace_time tracer("debug dump");

  char dirBuf[] = "/tmp/hhbbcXXXXXX";
  auto const dir = mkdtemp(dirBuf);
  fs::create_directory(fs::path(dir));
  std::cout << "debug dump going to " << dir << '\n';

  parallel_for_each(
    program.units,
    [&] (const std::unique_ptr<php::Unit>& u) {
      auto const file = fs::path(dir) / fs::path(u->filename->data());
      fs::create_directories(fs::path(file).remove_filename());
      fs::ofstream out(file);
      if (!out.is_open()) {
        throw std::runtime_error(std::string("failed to open file ") +
          file.native());
      }
      out << show(*u);
      if (out.bad()) {
        throw std::runtime_error(std::string("couldn't write file ") +
          file.native());
      }
    }
  );

  std::cout << "debug dump done\n";
}

//////////////////////////////////////////////////////////////////////

}}

