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

#include <iostream>
#include <limits>

#include <boost/program_options.hpp>

#include "hphp/tools/debug-parser/debug-parser.h"

/*
 * Simple program which provides a command-line interface to the debug-info
 * printer. Mainly for debugging.
 */

namespace {

const std::string kProgramDescription =
  "Print out debug-information in textual format";

}

int main(int argc, char** argv) {
  namespace po = boost::program_options;

  std::size_t begin = 0;
  std::size_t end = std::numeric_limits<std::size_t>::max();
  bool dwp = false;

  po::options_description desc{"Allowed options"};
  desc.add_options()
    ("help", "produce help message")
    ("begin",
     po::value<std::size_t>(&begin),
     "offset to begin printing at")
    ("end",
     po::value<std::size_t>(&end),
     "offset to stop printing at")
    ("dwp",
     po::bool_switch(&dwp),
     "print the dwp file instead of the binary");

  try {
    po::variables_map vm;
    const auto parsed = po::command_line_parser(argc, argv).options(desc).run();
    po::store(parsed, vm);

    if (vm.count("help")) {
      std::cout << kProgramDescription << "\n\n"
                << desc << std::endl;
      return 0;
    }

    po::notify(vm);

    using namespace debug_parser;

    const auto filenames =
      po::collect_unrecognized(parsed.options, po::include_positional);
    for (const auto& filename : filenames) {
      try {
        const auto printer = Printer::make(filename);
        if (!printer) {
          std::cerr << "ERROR: Platform doesn't have a debug-info parser"
                    << std::endl;
          return 1;
        }
        (*printer)(std::cout, begin, end, dwp);
      } catch (const Exception& exn) {
        std::cerr << "ERROR: " << exn.what() << std::endl;
        return 1;
      }
    }
  } catch (const po::error& e) {
    std::cerr << e.what() << "\n\n"
              << kProgramDescription << "\n\n"
              << desc << std::endl;
    return 1;
  }

  return 0;
}
